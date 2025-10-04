#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "emu/x87emu_private.h"
#include "rv64_emitter.h"
#include "x64trace.h"
#include "signals.h"
#include "dynarec_native.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_functions.h"
#include "custommem.h"
#include "bridge.h"
#include "rv64_lock.h"
#include "gdbjit.h"
#include "perfmap.h"

#define XMM0 0
#define X870 XMM0 + 16
#define EMM0 XMM0 + 16

// Get a FPU scratch reg
int fpu_get_scratch(dynarec_rv64_t* dyn)
{
    return SCRATCH0 + dyn->e.fpu_scratch++; // return an Sx
}

// Get a FPU scratch reg aligned to LMUL
int fpu_get_scratch_lmul(dynarec_rv64_t* dyn, int lmul)
{
    int reg = SCRATCH0 + dyn->e.fpu_scratch;
    int skip = (reg % (1 << lmul)) ? (1 << lmul) - (reg % (1 << lmul)) : 0;
    dyn->e.fpu_scratch += skip + 1;
    return reg + skip;
}

// Reset scratch regs counter
void fpu_reset_scratch(dynarec_rv64_t* dyn)
{
    dyn->e.fpu_scratch = 0;
}
// Get a x87 double reg
int fpu_get_reg_x87(dynarec_rv64_t* dyn, int t, int n)
{
    int i = X870;
    while (dyn->e.fpuused[i])
        ++i;
    dyn->e.fpuused[i] = 1;
    dyn->e.extcache[i].n = n;
    dyn->e.extcache[i].t = t;
    dyn->e.news |= (1 << i);
    return EXTREG(i); // return a Dx
}
// Free a FPU reg
void fpu_free_reg(dynarec_rv64_t* dyn, int reg)
{
    int idx = EXTIDX(reg);
    // TODO: check upper limit?
    dyn->e.fpuused[idx] = 0;
    if (dyn->e.extcache[idx].t != EXT_CACHE_ST_F && dyn->e.extcache[idx].t != EXT_CACHE_ST_D && dyn->e.extcache[idx].t != EXT_CACHE_ST_I64)
        dyn->e.extcache[idx].v = 0;
}

// Get an MMX reg
int fpu_get_reg_emm(dynarec_rv64_t* dyn, int t, int emm)
{
    int i = EMM0 + emm;
    dyn->e.fpuused[i] = 1;
    dyn->e.extcache[i].t = t;
    dyn->e.extcache[i].n = emm;
    dyn->e.news |= (1 << (i));
    return EXTREG(i);
}

// Get an XMM reg
int fpu_get_reg_xmm(dynarec_rv64_t* dyn, int t, int xmm)
{
    int i = XMM0 + xmm;
    dyn->e.fpuused[i] = 1;
    dyn->e.extcache[i].t = t;
    dyn->e.extcache[i].n = xmm;
    dyn->e.news |= (1 << i);
    return EXTREG(i);
}
// Reset fpu regs counter
static void fpu_reset_reg_extcache(dynarec_rv64_t* dyn, extcache_t* e)
{
    for (int i = 0; i < 32; ++i) {
        e->fpuused[i] = 0;
        e->extcache[i].v = 0;
    }
    dyn->vector_sew = VECTOR_SEWNA;
}
void fpu_reset_reg(dynarec_rv64_t* dyn)
{
    fpu_reset_reg_extcache(dyn, &dyn->e);
}

int extcache_no_i64(dynarec_rv64_t* dyn, int ninst, int st, int a)
{
    if (a == EXT_CACHE_ST_I64) {
        extcache_promote_double(dyn, ninst, st);
        return EXT_CACHE_ST_D;
    }
    return a;
}

int extcache_get_st(dynarec_rv64_t* dyn, int ninst, int a)
{
    if (dyn->insts[ninst].e.swapped) {
        if (dyn->insts[ninst].e.combined1 == a)
            a = dyn->insts[ninst].e.combined2;
        else if (dyn->insts[ninst].e.combined2 == a)
            a = dyn->insts[ninst].e.combined1;
    }
    for (int i = 0; i < 24; ++i)
        if ((dyn->insts[ninst].e.extcache[i].t == EXT_CACHE_ST_F
                || dyn->insts[ninst].e.extcache[i].t == EXT_CACHE_ST_D
                || dyn->insts[ninst].e.extcache[i].t == EXT_CACHE_ST_I64)
            && dyn->insts[ninst].e.extcache[i].n == a)
            return dyn->insts[ninst].e.extcache[i].t;
    // not in the cache yet, so will be fetched...
    return EXT_CACHE_ST_D;
}

int extcache_get_current_st(dynarec_rv64_t* dyn, int ninst, int a)
{
    (void)ninst;
    if (!dyn->insts)
        return EXT_CACHE_ST_D;
    for (int i = 0; i < 24; ++i)
        if ((dyn->e.extcache[i].t == EXT_CACHE_ST_F
                || dyn->e.extcache[i].t == EXT_CACHE_ST_D
                || dyn->e.extcache[i].t == EXT_CACHE_ST_I64)
            && dyn->e.extcache[i].n == a)
            return dyn->e.extcache[i].t;
    // not in the cache yet, so will be fetched...
    return EXT_CACHE_ST_D;
}

int extcache_get_st_f(dynarec_rv64_t* dyn, int ninst, int a)
{
    for (int i = 0; i < 24; ++i)
        if (dyn->insts[ninst].e.extcache[i].t == EXT_CACHE_ST_F
            && dyn->insts[ninst].e.extcache[i].n == a)
            return i;
    return -1;
}

int extcache_get_st_f_i64(dynarec_rv64_t* dyn, int ninst, int a)
{
    for (int i = 0; i < 24; ++i)
        if ((dyn->insts[ninst].e.extcache[i].t == EXT_CACHE_ST_I64 || dyn->insts[ninst].e.extcache[i].t == EXT_CACHE_ST_F)
            && dyn->insts[ninst].e.extcache[i].n == a)
            return i;
    return -1;
}

int extcache_get_st_f_noback(dynarec_rv64_t* dyn, int ninst, int a)
{
    for (int i = 0; i < 24; ++i)
        if (dyn->insts[ninst].e.extcache[i].t == EXT_CACHE_ST_F
            && dyn->insts[ninst].e.extcache[i].n == a)
            return i;
    return -1;
}

int extcache_get_st_f_i64_noback(dynarec_rv64_t* dyn, int ninst, int a)
{
    for (int i = 0; i < 24; ++i)
        if ((dyn->insts[ninst].e.extcache[i].t == EXT_CACHE_ST_I64 || dyn->insts[ninst].e.extcache[i].t == EXT_CACHE_ST_F)
            && dyn->insts[ninst].e.extcache[i].n == a)
            return i;
    return -1;
}

int extcache_get_current_st_f(dynarec_rv64_t* dyn, int a)
{
    for (int i = 0; i < 24; ++i)
        if (dyn->e.extcache[i].t == EXT_CACHE_ST_F
            && dyn->e.extcache[i].n == a)
            return i;
    return -1;
}

int extcache_get_current_st_f_i64(dynarec_rv64_t* dyn, int a)
{
    for (int i = 0; i < 24; ++i)
        if ((dyn->e.extcache[i].t == EXT_CACHE_ST_I64 || dyn->e.extcache[i].t == EXT_CACHE_ST_F)
            && dyn->e.extcache[i].n == a)
            return i;
    return -1;
}

static void extcache_promote_double_forward(dynarec_rv64_t* dyn, int ninst, int maxinst, int a);
static void extcache_promote_double_internal(dynarec_rv64_t* dyn, int ninst, int maxinst, int a);
static void extcache_promote_double_combined(dynarec_rv64_t* dyn, int ninst, int maxinst, int a)
{
    if (a == dyn->insts[ninst].e.combined1 || a == dyn->insts[ninst].e.combined2) {
        if (a == dyn->insts[ninst].e.combined1) {
            a = dyn->insts[ninst].e.combined2;
        } else
            a = dyn->insts[ninst].e.combined1;
        int i = extcache_get_st_f_i64_noback(dyn, ninst, a);
        // if(dyn->need_dump) dynarec_log(LOG_NONE, "extcache_promote_double_combined, ninst=%d combined%c %d i=%d (stack:%d/%d)\n", ninst, (a == dyn->insts[ninst].e.combined2)?'2':'1', a ,i, dyn->insts[ninst].e.stack_push, -dyn->insts[ninst].e.stack_pop);
        if (i >= 0) {
            dyn->insts[ninst].e.extcache[i].t = EXT_CACHE_ST_D;
            if (dyn->insts[ninst].x87precision) dyn->need_x87check = 2;
            if (!dyn->insts[ninst].e.barrier)
                extcache_promote_double_internal(dyn, ninst - 1, maxinst, a - dyn->insts[ninst].e.stack_push);
            // go forward is combined is not pop'd
            if (a - dyn->insts[ninst].e.stack_pop >= 0)
                if (!dyn->insts[ninst + 1].e.barrier)
                    extcache_promote_double_forward(dyn, ninst + 1, maxinst, a - dyn->insts[ninst].e.stack_pop);
        }
    }
}
static void extcache_promote_double_internal(dynarec_rv64_t* dyn, int ninst, int maxinst, int a)
{
    if (dyn->insts[ninst + 1].e.barrier)
        return;
    while (ninst >= 0) {
        a += dyn->insts[ninst].e.stack_pop; // adjust Stack depth: add pop'd ST (going backward)
        int i = extcache_get_st_f_i64(dyn, ninst, a);
        // if(dyn->need_dump) dynarec_log(LOG_NONE, "extcache_promote_double_internal, ninst=%d, a=%d st=%d:%d, i=%d\n", ninst, a, dyn->insts[ninst].e.stack, dyn->insts[ninst].e.stack_next, i);
        if (i < 0) return;
        dyn->insts[ninst].e.extcache[i].t = EXT_CACHE_ST_D;
        if (dyn->insts[ninst].x87precision) dyn->need_x87check = 2;
        // check combined propagation too
        if (dyn->insts[ninst].e.combined1 || dyn->insts[ninst].e.combined2) {
            if (dyn->insts[ninst].e.swapped) {
                // if(dyn->need_dump) dynarec_log(LOG_NONE, "extcache_promote_double_internal, ninst=%d swapped %d/%d vs %d with st %d\n", ninst, dyn->insts[ninst].e.combined1 ,dyn->insts[ninst].e.combined2, a, dyn->insts[ninst].e.stack);
                if (a == dyn->insts[ninst].e.combined1)
                    a = dyn->insts[ninst].e.combined2;
                else if (a == dyn->insts[ninst].e.combined2)
                    a = dyn->insts[ninst].e.combined1;
            } else {
                // if(dyn->need_dump) dynarec_log(LOG_NONE, "extcache_promote_double_internal, ninst=%d combined %d/%d vs %d with st %d\n", ninst, dyn->insts[ninst].e.combined1 ,dyn->insts[ninst].e.combined2, a, dyn->insts[ninst].e.stack);
                extcache_promote_double_combined(dyn, ninst, maxinst, a);
            }
        }
        a -= dyn->insts[ninst].e.stack_push; // // adjust Stack depth: remove push'd ST (going backward)
        --ninst;
        if (ninst < 0 || a < 0 || dyn->insts[ninst].e.barrier)
            return;
    }
}

static void extcache_promote_double_forward(dynarec_rv64_t* dyn, int ninst, int maxinst, int a)
{
    while ((ninst != -1) && (ninst < maxinst) && (a >= 0)) {
        a += dyn->insts[ninst].e.stack_push; // // adjust Stack depth: add push'd ST (going forward)
        if ((dyn->insts[ninst].e.combined1 || dyn->insts[ninst].e.combined2) && dyn->insts[ninst].e.swapped) {
            // if(dyn->need_dump) dynarec_log(LOG_NONE, "extcache_promote_double_forward, ninst=%d swapped %d/%d vs %d with st %d\n", ninst, dyn->insts[ninst].e.combined1 ,dyn->insts[ninst].e.combined2, a, dyn->insts[ninst].e.stack);
            if (a == dyn->insts[ninst].e.combined1)
                a = dyn->insts[ninst].e.combined2;
            else if (a == dyn->insts[ninst].e.combined2)
                a = dyn->insts[ninst].e.combined1;
        }
        int i = extcache_get_st_f_i64_noback(dyn, ninst, a);
        // if(dyn->need_dump) dynarec_log(LOG_NONE, "extcache_promote_double_forward, ninst=%d, a=%d st=%d:%d(%d/%d), i=%d\n", ninst, a, dyn->insts[ninst].e.stack, dyn->insts[ninst].e.stack_next, dyn->insts[ninst].e.stack_push, -dyn->insts[ninst].e.stack_pop, i);
        if (i < 0) return;
        dyn->insts[ninst].e.extcache[i].t = EXT_CACHE_ST_D;
        if (dyn->insts[ninst].x87precision) dyn->need_x87check = 2;
        // check combined propagation too
        if ((dyn->insts[ninst].e.combined1 || dyn->insts[ninst].e.combined2) && !dyn->insts[ninst].e.swapped) {
            // if(dyn->need_dump) dynarec_log(LOG_NONE, "extcache_promote_double_forward, ninst=%d combined %d/%d vs %d with st %d\n", ninst, dyn->insts[ninst].e.combined1 ,dyn->insts[ninst].e.combined2, a, dyn->insts[ninst].e.stack);
            extcache_promote_double_combined(dyn, ninst, maxinst, a);
        }
        a -= dyn->insts[ninst].e.stack_pop; // adjust Stack depth: remove pop'd ST (going forward)
        if (dyn->insts[ninst].x64.has_next && !dyn->insts[ninst].e.barrier)
            ++ninst;
        else
            ninst = -1;
    }
    if (ninst == maxinst)
        extcache_promote_double(dyn, ninst, a);
}

void extcache_promote_double(dynarec_rv64_t* dyn, int ninst, int a)
{
    int i = extcache_get_current_st_f_i64(dyn, a);
    // if(dyn->need_dump) dynarec_log(LOG_NONE, "extcache_promote_double, ninst=%d a=%d st=%d i=%d\n", ninst, a, dyn->e.stack, i);
    if (i < 0) return;
    dyn->e.extcache[i].t = EXT_CACHE_ST_D;
    dyn->insts[ninst].e.extcache[i].t = EXT_CACHE_ST_D;
    if (dyn->insts[ninst].x87precision) dyn->need_x87check = 2;
    // check combined propagation too
    if (dyn->e.combined1 || dyn->e.combined2) {
        if (dyn->e.swapped) {
            // if(dyn->need_dump) dynarec_log(LOG_NONE, "extcache_promote_double, ninst=%d swapped! %d/%d vs %d\n", ninst, dyn->e.combined1 ,dyn->e.combined2, a);
            if (dyn->e.combined1 == a)
                a = dyn->e.combined2;
            else if (dyn->e.combined2 == a)
                a = dyn->e.combined1;
        } else {
            // if(dyn->need_dump) dynarec_log(LOG_NONE, "extcache_promote_double, ninst=%d combined! %d/%d vs %d\n", ninst, dyn->e.combined1 ,dyn->e.combined2, a);
            if (dyn->e.combined1 == a)
                extcache_promote_double(dyn, ninst, dyn->e.combined2);
            else if (dyn->e.combined2 == a)
                extcache_promote_double(dyn, ninst, dyn->e.combined1);
        }
    }
    a -= dyn->insts[ninst].e.stack_push; // // adjust Stack depth: remove push'd ST (going backward)
    if (!ninst || a < 0) return;
    extcache_promote_double_internal(dyn, ninst - 1, ninst, a);
}

int extcache_combine_st(dynarec_rv64_t* dyn, int ninst, int a, int b)
{
    dyn->e.combined1 = a;
    dyn->e.combined2 = b;
    if (extcache_get_current_st(dyn, ninst, a) == EXT_CACHE_ST_F
        && extcache_get_current_st(dyn, ninst, b) == EXT_CACHE_ST_F)
        return EXT_CACHE_ST_F;
    return EXT_CACHE_ST_D;
}

static int isCacheEmpty(dynarec_native_t* dyn, int ninst)
{
    if (dyn->insts[ninst].e.stack_next) {
        return 0;
    }
    for (int i = 0; i < 24; ++i)
        if (dyn->insts[ninst].e.extcache[i].v) { // there is something at ninst for i
            if (!(
                    (dyn->insts[ninst].e.extcache[i].t == EXT_CACHE_ST_F
                        || dyn->insts[ninst].e.extcache[i].t == EXT_CACHE_ST_D
                        || dyn->insts[ninst].e.extcache[i].t == EXT_CACHE_ST_I64)
                    && dyn->insts[ninst].e.extcache[i].n < dyn->insts[ninst].e.stack_pop))
                return 0;
        }
    return 1;
}

int fpuCacheNeedsTransform(dynarec_rv64_t* dyn, int ninst)
{
    int i2 = dyn->insts[ninst].x64.jmp_insts;
    if (i2 < 0)
        return 1;
    if ((dyn->insts[i2].x64.barrier & BARRIER_FLOAT))
        // if the barrier as already been apply, no transform needed
        return ((dyn->insts[ninst].x64.barrier & BARRIER_FLOAT)) ? 0 : (isCacheEmpty(dyn, ninst) ? 0 : 1);
    int ret = 0;
    if (!i2) { // just purge
        if (dyn->insts[ninst].e.stack_next) {
            return 1;
        }
        for (int i = 0; i < 24 && !ret; ++i)
            if (dyn->insts[ninst].e.extcache[i].v) { // there is something at ninst for i
                if (!(
                        (dyn->insts[ninst].e.extcache[i].t == EXT_CACHE_ST_F
                            || dyn->insts[ninst].e.extcache[i].t == EXT_CACHE_ST_D
                            || dyn->insts[ninst].e.extcache[i].t == EXT_CACHE_ST_I64)
                        && dyn->insts[ninst].e.extcache[i].n < dyn->insts[ninst].e.stack_pop))
                    ret = 1;
            }
        return ret;
    }
    // Check if ninst can be compatible to i2
    if (dyn->insts[ninst].e.stack_next != dyn->insts[i2].e.stack - dyn->insts[i2].e.stack_push) {
        return 1;
    }
    extcache_t cache_i2 = dyn->insts[i2].e;
    extcacheUnwind(&cache_i2);

    for (int i = 0; i < 24; ++i) {
        if (dyn->insts[ninst].e.extcache[i].v) { // there is something at ninst for i
            if (!cache_i2.extcache[i].v) {       // but there is nothing at i2 for i
                ret = 1;
            } else if (dyn->insts[ninst].e.extcache[i].v != cache_i2.extcache[i].v) { // there is something different
                if (dyn->insts[ninst].e.extcache[i].n != cache_i2.extcache[i].n) {    // not the same x64 reg
                    ret = 1;
                } else if (dyn->insts[ninst].e.extcache[i].t == EXT_CACHE_XMMR && cache_i2.extcache[i].t == EXT_CACHE_XMMW) { /* nothing */
                } else
                    ret = 1;
            }
        } else if (cache_i2.extcache[i].v)
            ret = 1;
    }
    return ret;
}

int sewNeedsTransform(dynarec_rv64_t* dyn, int ninst)
{
    int i2 = dyn->insts[ninst].x64.jmp_insts;

    if (dyn->insts[i2].vector_sew_entry == VECTOR_SEWNA)
        return 0;
    else if (dyn->insts[i2].vector_sew_entry == VECTOR_SEWANY && dyn->insts[ninst].vector_sew_exit != VECTOR_SEWNA)
        return 0;
    else if (dyn->insts[i2].vector_sew_entry == dyn->insts[ninst].vector_sew_exit)
        return 0;

    return 1;
}

void extcacheUnwind(extcache_t* cache)
{
    if (cache->swapped) {
        // unswap
        int a = -1;
        int b = -1;
        for (int j = 0; j < 24 && ((a == -1) || (b == -1)); ++j)
            if ((cache->extcache[j].t == EXT_CACHE_ST_D
                    || cache->extcache[j].t == EXT_CACHE_ST_F
                    || cache->extcache[j].t == EXT_CACHE_ST_I64)) {
                if (cache->extcache[j].n == cache->combined1)
                    a = j;
                else if (cache->extcache[j].n == cache->combined2)
                    b = j;
            }
        if (a != -1 && b != -1) {
            int tmp = cache->extcache[a].n;
            cache->extcache[a].n = cache->extcache[b].n;
            cache->extcache[b].n = tmp;
        }
        cache->swapped = 0;
        cache->combined1 = cache->combined2 = 0;
    }
    if (cache->news) {
        // remove the newly created extcache
        for (int i = 0; i < 24; ++i)
            if ((cache->news & (1 << i)) && !cache->olds[i].changed)
                cache->extcache[i].v = 0;
        cache->news = 0;
    }
    // add/change bad regs
    for (int i = 0; i < 16; ++i) {
        if (cache->olds[i].changed || cache->olds[i].purged) {
            if (cache->olds[i].type == EXT_CACHE_OLD_XMMR)
                cache->extcache[i].t = EXT_CACHE_XMMR;
            else if (cache->olds[i].type == EXT_CACHE_OLD_XMMW)
                cache->extcache[i].t = EXT_CACHE_XMMW;
            else if (cache->olds[i].type == EXT_CACHE_OLD_SS)
                cache->extcache[i].t = EXT_CACHE_SS;
            else if (cache->olds[i].type == EXT_CACHE_OLD_SD)
                cache->extcache[i].t = EXT_CACHE_SD;

            if (cache->olds[i].purged)
                cache->extcache[i].n = i;
        }
    }

    if (cache->stack_push) {
        // unpush
        for (int j = 0; j < 24; ++j) {
            if ((cache->extcache[j].t == EXT_CACHE_ST_D
                    || cache->extcache[j].t == EXT_CACHE_ST_F
                    || cache->extcache[j].t == EXT_CACHE_ST_I64)) {
                if (cache->extcache[j].n < cache->stack_push)
                    cache->extcache[j].v = 0;
                else
                    cache->extcache[j].n -= cache->stack_push;
            }
        }
        cache->x87stack -= cache->stack_push;
        cache->tags >>= (cache->stack_push * 2);
        cache->stack -= cache->stack_push;
        if (cache->pushed >= cache->stack_push)
            cache->pushed -= cache->stack_push;
        else
            cache->pushed = 0;
        cache->stack_push = 0;
    }
    cache->x87stack += cache->stack_pop;
    cache->stack_next = cache->stack;
    if (cache->stack_pop) {
        if (cache->poped >= cache->stack_pop)
            cache->poped -= cache->stack_pop;
        else
            cache->poped = 0;
        cache->tags <<= (cache->stack_pop * 2);
    }
    cache->stack_pop = 0;
    cache->barrier = 0;
    // And now, rebuild the x87cache info with extcache
    cache->mmxcount = 0;
    cache->fpu_scratch = 0;
    for (int i = 0; i < 8; ++i) {
        cache->x87cache[i] = -1;
        cache->mmxcache[i].v = -1;
        cache->x87reg[i] = 0;
        cache->ssecache[i * 2].v = -1;
        cache->ssecache[i * 2 + 1].v = -1;
    }
    int x87reg = 0;
    for (int i = 0; i < 32; ++i) {
        if (cache->extcache[i].v) {
            cache->fpuused[i] = 1;
            switch (cache->extcache[i].t) {
                case EXT_CACHE_MM:
                case EXT_CACHE_MMV:
                    cache->mmxcache[cache->extcache[i].n].reg = EXTREG(i);
                    cache->mmxcache[cache->extcache[i].n].vector = cache->extcache[i].t == EXT_CACHE_MMV;
                    ++cache->mmxcount;
                    break;
                case EXT_CACHE_SS:
                    cache->ssecache[cache->extcache[i].n].reg = EXTREG(i);
                    cache->ssecache[cache->extcache[i].n].vector = 0;
                    cache->ssecache[cache->extcache[i].n].single = 1;
                    break;
                case EXT_CACHE_SD:
                    cache->ssecache[cache->extcache[i].n].reg = EXTREG(i);
                    cache->ssecache[cache->extcache[i].n].vector = 0;
                    cache->ssecache[cache->extcache[i].n].single = 0;
                    break;
                case EXT_CACHE_XMMR:
                case EXT_CACHE_XMMW:
                    cache->ssecache[cache->extcache[i].n].reg = EXTREG(i);
                    cache->ssecache[cache->extcache[i].n].vector = 1;
                    cache->ssecache[cache->extcache[i].n].write = (cache->extcache[i].t == EXT_CACHE_XMMW) ? 1 : 0;
                    break;
                case EXT_CACHE_ST_F:
                case EXT_CACHE_ST_D:
                case EXT_CACHE_ST_I64:
                    cache->x87cache[x87reg] = cache->extcache[i].n;
                    cache->x87reg[x87reg] = EXTREG(i);
                    ++x87reg;
                    break;
                case EXT_CACHE_SCR:
                    cache->fpuused[i] = 0;
                    cache->extcache[i].v = 0;
                    break;
            }
        } else {
            cache->fpuused[i] = 0;
        }
    }
}

// will go badly if address is unaligned
static uint8_t extract_byte(uint32_t val, void* address)
{
    int idx = (((uintptr_t)address) & 3) * 8;
    return (val >> idx) & 0xff;
}

static uint32_t insert_byte(uint32_t val, uint8_t b, void* address)
{
    int idx = (((uintptr_t)address) & 3) * 8;
    val &= ~(0xff << idx);
    val |= (((uint32_t)b) << idx);
    return val;
}

static uint16_t extract_half(uint32_t val, void* address)
{
    int idx = (((uintptr_t)address) & 3) * 8;
    return (val >> idx) & 0xffff;
}
static uint32_t insert_half(uint32_t val, uint16_t h, void* address)
{
    int idx = (((uintptr_t)address) & 3) * 8;
    val &= ~(0xffff << idx);
    val |= (((uint32_t)h) << idx);
    return val;
}

uint8_t rv64_lock_xchg_b(void* addr, uint8_t val)
{
    uint32_t ret;
    uint32_t* aligned = (uint32_t*)(((uintptr_t)addr) & ~3);
    do {
        ret = *aligned;
    } while (rv64_lock_cas_d(aligned, ret, insert_byte(ret, val, addr)));
    return extract_byte(ret, addr);
}

int rv64_lock_cas_b(void* addr, uint8_t ref, uint8_t val)
{
    uint32_t* aligned = (uint32_t*)(((uintptr_t)addr) & ~3);
    uint32_t tmp = *aligned;
    return rv64_lock_cas_d(aligned, insert_byte(tmp, ref, addr), insert_byte(tmp, val, addr));
}

int rv64_lock_cas_h(void* addr, uint16_t ref, uint16_t val)
{
    uint32_t* aligned = (uint32_t*)(((uintptr_t)addr) & ~3);
    uint32_t tmp = *aligned;
    return rv64_lock_cas_d(aligned, insert_half(tmp, ref, addr), insert_half(tmp, val, addr));
}


const char* getCacheName(int t, int n)
{
    static char buff[20];
    switch (t) {
        case EXT_CACHE_ST_D: sprintf(buff, "ST%d", n); break;
        case EXT_CACHE_ST_F: sprintf(buff, "st%d", n); break;
        case EXT_CACHE_ST_I64: sprintf(buff, "STi%d", n); break;
        case EXT_CACHE_MM: sprintf(buff, "MM%d", n); break;
        case EXT_CACHE_MMV: sprintf(buff, "MMV%d", n); break;
        case EXT_CACHE_SS: sprintf(buff, "SS%d", n); break;
        case EXT_CACHE_SD: sprintf(buff, "SD%d", n); break;
        case EXT_CACHE_SCR: sprintf(buff, "Scratch"); break;
        case EXT_CACHE_XMMW: sprintf(buff, "XMM%d", n); break;
        case EXT_CACHE_XMMR: sprintf(buff, "xmm%d", n); break;
        case EXT_CACHE_NONE: buff[0] = '\0'; break;
    }
    return buff;
}

static register_mapping_t register_mappings[] = {
    { "rax", "a6" },
    { "eax", "a6" },
    { "ax", "a6" },
    { "ah", "a6" },
    { "al", "a6" },
    { "rcx", "a3" },
    { "ecx", "a3" },
    { "cx", "a3" },
    { "ch", "a3" },
    { "cl", "a3" },
    { "rdx", "a2" },
    { "edx", "a2" },
    { "dx", "a2" },
    { "dh", "a2" },
    { "dl", "a2" },
    { "rbx", "s8" },
    { "ebx", "s8" },
    { "bx", "s8" },
    { "bh", "s8" },
    { "bl", "s8" },
    { "rsi", "a1" },
    { "esi", "a1" },
    { "si", "a1" },
    { "sil", "a1" },
    { "rdi", "a0" },
    { "edi", "a0" },
    { "di", "a0" },
    { "dil", "a0" },
    { "rsp", "s1" },
    { "esp", "s1" },
    { "sp", "s1" },
    { "spl", "s1" },
    { "rbp", "s0" },
    { "ebp", "s0" },
    { "bp", "s0" },
    { "bpl", "s0" },
    { "r8", "a4" },
    { "r8d", "a4" },
    { "r8w", "a4" },
    { "r8b", "a4" },
    { "r9", "a5" },
    { "r9d", "a5" },
    { "r9w", "a5" },
    { "r9b", "a5" },
    { "r10", "s10" },
    { "r10d", "s10" },
    { "r10w", "s10" },
    { "r10b", "s10" },
    { "r11", "s11" },
    { "r11d", "s11" },
    { "r11w", "s11" },
    { "r11b", "s11" },
    { "r12", "s2" },
    { "r12d", "s2" },
    { "r12w", "s2" },
    { "r12b", "s2" },
    { "r13", "s3" },
    { "r13d", "s3" },
    { "r13w", "s3" },
    { "r13b", "s3" },
    { "r14", "s4" },
    { "r14d", "s4" },
    { "r14w", "s4" },
    { "r14b", "s4" },
    { "r15", "s5" },
    { "r15d", "s5" },
    { "r15w", "s5" },
    { "r15b", "s5" },
    { "rip", "s6" },
};

void printf_x64_instruction(dynarec_native_t* dyn, zydis_dec_t* dec, instruction_x64_t* inst, const char* name);
void inst_name_pass3(dynarec_native_t* dyn, int ninst, const char* name, rex_t rex)
{
    if (!dyn->need_dump && !BOX64ENV(dynarec_gdbjit) && !BOX64ENV(dynarec_perf_map)) return;

    static char buf[4096];
    int length = sprintf(buf, "barrier=%d state=%d/%d(%d), %s=%X/%X, use=%X, need=%X/%X, fuse=%d/%d, sm=%d(%d/%d), sew@entry=%d, sew@exit=%d",
        dyn->insts[ninst].x64.barrier,
        dyn->insts[ninst].x64.state_flags,
        dyn->f.pending,
        dyn->f.dfnone,
        dyn->insts[ninst].x64.may_set ? "may" : "set",
        dyn->insts[ninst].x64.set_flags,
        dyn->insts[ninst].x64.gen_flags,
        dyn->insts[ninst].x64.use_flags,
        dyn->insts[ninst].x64.need_before,
        dyn->insts[ninst].x64.need_after,
        dyn->insts[ninst].nat_flags_fusion,
        dyn->insts[ninst].no_scratch_usage,
        dyn->smwrite, dyn->insts[ninst].will_write, dyn->insts[ninst].last_write,
        dyn->insts[ninst].vector_sew_entry, dyn->insts[ninst].vector_sew_exit);
    if (dyn->insts[ninst].pred_sz) {
        length += sprintf(buf + length, ", pred=");
        for (int ii = 0; ii < dyn->insts[ninst].pred_sz; ++ii)
            length += sprintf(buf + length, "%s%d", ii ? "/" : "", dyn->insts[ninst].pred[ii]);
    }
    if (dyn->insts[ninst].x64.jmp && dyn->insts[ninst].x64.jmp_insts >= 0)
        length += sprintf(buf + length, ", jmp=%d", dyn->insts[ninst].x64.jmp_insts);
    if (dyn->insts[ninst].x64.jmp && dyn->insts[ninst].x64.jmp_insts == -1)
        length += sprintf(buf + length, ", jmp=out");
    if (dyn->last_ip)
        length += sprintf(buf + length, ", last_ip=%p", (void*)dyn->last_ip);
    for (int ii = 0; ii < 32; ++ii) {
        switch (dyn->insts[ninst].e.extcache[ii].t) {
            case EXT_CACHE_ST_D: length += sprintf(buf + length, " f%d:%s", EXTREG(ii), getCacheName(dyn->insts[ninst].e.extcache[ii].t, dyn->insts[ninst].e.extcache[ii].n)); break;
            case EXT_CACHE_ST_F: length += sprintf(buf + length, " f%d:%s", EXTREG(ii), getCacheName(dyn->insts[ninst].e.extcache[ii].t, dyn->insts[ninst].e.extcache[ii].n)); break;
            case EXT_CACHE_ST_I64: length += sprintf(buf + length, " f%d:%s", EXTREG(ii), getCacheName(dyn->insts[ninst].e.extcache[ii].t, dyn->insts[ninst].e.extcache[ii].n)); break;
            case EXT_CACHE_MM: length += sprintf(buf + length, " f%d:%s", EXTREG(ii), getCacheName(dyn->insts[ninst].e.extcache[ii].t, dyn->insts[ninst].e.extcache[ii].n)); break;
            case EXT_CACHE_MMV: length += sprintf(buf + length, " v%d:%s", EXTREG(ii), getCacheName(dyn->insts[ninst].e.extcache[ii].t, dyn->insts[ninst].e.extcache[ii].n)); break;
            case EXT_CACHE_SS: length += sprintf(buf + length, " f%d:%s", EXTREG(ii), getCacheName(dyn->insts[ninst].e.extcache[ii].t, dyn->insts[ninst].e.extcache[ii].n)); break;
            case EXT_CACHE_SD: length += sprintf(buf + length, " f%d:%s", EXTREG(ii), getCacheName(dyn->insts[ninst].e.extcache[ii].t, dyn->insts[ninst].e.extcache[ii].n)); break;
            case EXT_CACHE_XMMR: length += sprintf(buf + length, " v%d:%s", EXTREG(ii), getCacheName(dyn->insts[ninst].e.extcache[ii].t, dyn->insts[ninst].e.extcache[ii].n)); break;
            case EXT_CACHE_XMMW: length += sprintf(buf + length, " v%d:%s", EXTREG(ii), getCacheName(dyn->insts[ninst].e.extcache[ii].t, dyn->insts[ninst].e.extcache[ii].n)); break;
            case EXT_CACHE_SCR: length += sprintf(buf + length, " f%d:%s", EXTREG(ii), getCacheName(dyn->insts[ninst].e.extcache[ii].t, dyn->insts[ninst].e.extcache[ii].n)); break;
            case EXT_CACHE_NONE:
            default: break;
        }
    }
    if (dyn->ymm_zero)
        length += sprintf(buf + length, " ymm0_mask = %04x", dyn->ymm_zero);
    if (dyn->e.stack || dyn->insts[ninst].e.stack_next || dyn->insts[ninst].e.x87stack)
        length += sprintf(buf + length, " X87:%d/%d(+%d/-%d)%d", dyn->e.stack, dyn->insts[ninst].e.stack_next, dyn->insts[ninst].e.stack_push, dyn->insts[ninst].e.stack_pop, dyn->insts[ninst].e.x87stack);
    if (dyn->insts[ninst].e.combined1 || dyn->insts[ninst].e.combined2)
        length += sprintf(buf + length, " %s:%d/%d", dyn->insts[ninst].e.swapped ? "SWP" : "CMB", dyn->insts[ninst].e.combined1, dyn->insts[ninst].e.combined2);

    if (dyn->need_dump) {
        printf_x64_instruction(dyn, rex.is32bits ? my_context->dec32 : my_context->dec, &dyn->insts[ninst].x64, name);
        dynarec_log(LOG_NONE, "%s%p: %d emitted opcodes, inst=%d, %s%s\n",
            (dyn->need_dump > 1) ? "\e[32m" : "",
            (void*)(dyn->native_start + dyn->insts[ninst].address), dyn->insts[ninst].size / 4, ninst, buf, (dyn->need_dump > 1) ? "\e[m" : "");
    }
    if (BOX64ENV(dynarec_gdbjit)) {
        static char buf2[512];
        if (BOX64ENV(dynarec_gdbjit) > 1) {
            sprintf(buf2, "; %d: %d opcodes, %s", ninst, dyn->insts[ninst].size / 4, buf);
            dyn->gdbjit_block = GdbJITBlockAddLine(dyn->gdbjit_block, (dyn->native_start + dyn->insts[ninst].address), buf2);
        }
        zydis_dec_t* dec = rex.is32bits ? my_context->dec32 : my_context->dec;
        const char* inst_name = name;
        if (dec) {
            inst_name = DecodeX64Trace(dec, dyn->insts[ninst].x64.addr, 0);
            x64disas_add_register_mapping_annotations(buf2, inst_name, register_mappings, sizeof(register_mappings) / sizeof(register_mappings[0]));
            inst_name = buf2;
        }
        dyn->gdbjit_block = GdbJITBlockAddLine(dyn->gdbjit_block, (dyn->native_start + dyn->insts[ninst].address), inst_name);
    }
    if (BOX64ENV(dynarec_perf_map) && BOX64ENV(dynarec_perf_map_fd) != -1) {
        writePerfMap(dyn->insts[ninst].x64.addr, dyn->native_start + dyn->insts[ninst].address, dyn->insts[ninst].size / 4, name);
    }
    if (length > sizeof(buf)) printf_log(LOG_NONE, "Warning: buf to small in inst_name_pass3 (%d vs %zd)\n", length, sizeof(buf));
}

void print_opcode(dynarec_native_t* dyn, int ninst, uint32_t opcode)
{
    dynarec_log_prefix(0, LOG_NONE, "\t%08x\t%s\n", opcode, rv64_print(opcode, (uintptr_t)dyn->block));
}

// x87 stuffs
static void x87_reset(extcache_t* e)
{
    for (int i = 0; i < 8; ++i)
        e->x87cache[i] = -1;
    e->x87stack = 0;
    e->stack = 0;
    e->stack_next = 0;
    e->stack_pop = 0;
    e->stack_push = 0;
    e->combined1 = e->combined2 = 0;
    e->swapped = 0;
    e->barrier = 0;
    e->pushed = 0;
    e->poped = 0;

    for (int i = 0; i < 24; ++i)
        if (e->extcache[i].t == EXT_CACHE_ST_F
            || e->extcache[i].t == EXT_CACHE_ST_D
            || e->extcache[i].t == EXT_CACHE_ST_I64)
            e->extcache[i].v = 0;
}

static void mmx_reset(extcache_t* e)
{
    e->mmxcount = 0;
    for (int i = 0; i < 8; ++i)
        e->mmxcache[i].v = -1;
}

static void sse_reset(extcache_t* e)
{
    for (int i = 0; i < 16; ++i)
        e->ssecache[i].v = -1;
}

void fpu_reset(dynarec_rv64_t* dyn)
{
    x87_reset(&dyn->e);
    mmx_reset(&dyn->e);
    sse_reset(&dyn->e);
    fpu_reset_reg(dyn);
    dyn->ymm_zero = 0;
}

void fpu_reset_ninst(dynarec_rv64_t* dyn, int ninst)
{
    x87_reset(&dyn->insts[ninst].e);
    mmx_reset(&dyn->insts[ninst].e);
    sse_reset(&dyn->insts[ninst].e);
    fpu_reset_reg_extcache(dyn, &dyn->insts[ninst].e);
}

int fpu_is_st_freed(dynarec_rv64_t* dyn, int ninst, int st)
{
    return (dyn->e.tags & (0b11 << (st * 2))) ? 1 : 0;
}

void updateNativeFlags(dynarec_rv64_t* dyn)
{
    if (!BOX64ENV(dynarec_nativeflags))
        return;
    for (int i = 1; i < dyn->size; ++i)
        if (dyn->insts[i].nat_flags_fusion) {
            int j = i - 1;
            int found = 0;
            if (dyn->insts[i].pred_sz == 1 && dyn->insts[i].pred[0] == j) {
                while (j >= 0) {
                    if (dyn->insts[j].x64.set_flags && (dyn->insts[i].x64.use_flags & dyn->insts[j].x64.set_flags) == dyn->insts[i].x64.use_flags) {
                        dyn->insts[j].nat_flags_fusion = 1;
                        if (dyn->insts[i].x64.use_flags & X_SF) {
                            dyn->insts[j].nat_flags_needsign = 1;
                        }
                        dyn->insts[i].x64.use_flags = 0;
                        dyn->insts[j].nat_next_inst = i;
                        found = 1;
                        break;
                    } else if (j && dyn->insts[j].pred_sz == 1 && dyn->insts[j].pred[0] == j - 1
                        && dyn->insts[j].no_scratch_usage && !dyn->insts[j].x64.set_flags && !dyn->insts[j].x64.use_flags) {
                        j -= 1;
                    } else
                        break;
                }
            }
            if (!found) dyn->insts[i].nat_flags_fusion = 0;
        }
}

void get_free_scratch(dynarec_rv64_t* dyn, int ninst, uint8_t* tmp1, uint8_t* tmp2, uint8_t* tmp3, uint8_t s1, uint8_t s2, uint8_t s3, uint8_t s4, uint8_t s5)
{
    uint8_t n1 = dyn->insts[ninst].nat_flags_op1;
    uint8_t n2 = dyn->insts[ninst].nat_flags_op2;
    uint8_t tmp[5] = {0};
    int idx = 0;
    #define GO(s) if((s!=n1) && (s!=n2)) tmp[idx++] = s
    GO(s1);
    GO(s2);
    GO(s3);
    GO(s4);
    GO(s5);
    #undef GO
    *tmp1 = tmp[0];
    *tmp2 = tmp[1];
    *tmp3 = tmp[2];
}

void fpu_save_and_unwind(dynarec_rv64_t* dyn, int ninst, extcache_t* cache)
{
    memcpy(cache, &dyn->insts[ninst].e, sizeof(extcache_t));
    extcacheUnwind(&dyn->insts[ninst].e);
}
void fpu_unwind_restore(dynarec_rv64_t* dyn, int ninst, extcache_t* cache)
{
    memcpy(&dyn->insts[ninst].e, cache, sizeof(extcache_t));
}

extern const uint8_t ff_mult2[4][256];
extern const uint8_t ff_mult3[4][256];
extern const uint8_t invshiftrows[];
extern const uint8_t invsubbytes[256];
extern const uint8_t shiftrows[];
extern const uint8_t subbytes[256];
static const uint8_t mcshift[3][16] = {
    {3, 0, 1, 2, 7, 4, 5, 6, 11, 8, 9, 10, 15, 12, 13, 14},
    {2, 3, 0, 1, 6, 7, 4, 5, 10, 11, 8, 9, 14, 15, 12, 13},
    {1, 2, 3, 0, 5, 6, 7, 4, 9, 10, 11, 8, 13, 14, 15, 12},
};
__attribute__((target("arch=+v")))
void rvv_aesd(x64emu_t* emu, int xmm) {
    asm volatile(
        "li a7, 16\n\t"
        "vsetvli zero, a7, e8, m1\n\t"
        "vle8.v v4, (%[xmm])\n\t"
        "vle8.v v0, (%[shift])\n\t"
        "vle8.v v5, (%[mcshift])\n\t"
        "addi a7, %[mcshift], 16\n\t"
        "vle8.v v6, (a7)\n\t"
        "addi a7, %[mcshift], 32\n\t"
        "vle8.v v7, (a7)\n\t"
        "vrgather.vv v1, v4, v0\n\t"
        "vluxei8.v v4, (%[sub]), v1\n\t"
        "vluxei8.v v0, (%[ff]), v4\n\t"
        "addi a7, %[ff], 256\n\t"
        "vluxei8.v v1, (a7), v4\n\t"
        "addi a7, %[ff], 512\n\t"
        "vluxei8.v v2, (a7), v4\n\t"
        "addi a7, %[ff], 768\n\t"
        "vluxei8.v v3, (a7), v4\n\t"
        "vrgather.vv v9, v1, v5\n\t"
        "vrgather.vv v10, v2, v6\n\t"
        "vrgather.vv v11, v3, v7\n\t"
        "vxor.vv v0, v0, v9\n\t"
        "vxor.vv v1, v10, v11\n\t"
        "vxor.vv v0, v0, v1\n\t"
        "vse8.v v0, (%[xmm])"
        :
        : [xmm] "r" (&emu->xmm[xmm])
        , [ff] "r" (ff_mult2), [mcshift] "r" (mcshift)
        , [shift] "r" (invshiftrows), [sub] "r" (invsubbytes)
        : "memory", "a7", "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11");
}
__attribute__((target("arch=+v")))
void rvv_aese(x64emu_t* emu, int xmm) {
    asm volatile(
        "li a7, 16\n\t"
        "vsetvli zero, a7, e8, m1\n\t"
        "vle8.v v4, (%[xmm])\n\t"
        "vle8.v v0, (%[shift])\n\t"
        "vle8.v v5, (%[mcshift])\n\t"
        "addi a7, %[mcshift], 16\n\t"
        "vle8.v v6, (a7)\n\t"
        "addi a7, %[mcshift], 32\n\t"
        "vle8.v v7, (a7)\n\t"
        "vrgather.vv v1, v4, v0\n\t"
        "vluxei8.v v4, (%[sub]), v1\n\t"
        "vluxei8.v v0, (%[ff]), v4\n\t"
        "addi a7, %[ff], 256\n\t"
        "vluxei8.v v1, (a7), v4\n\t"
        "addi a7, %[ff], 512\n\t"
        "vluxei8.v v2, (a7), v4\n\t"
        "addi a7, %[ff], 768\n\t"
        "vluxei8.v v3, (a7), v4\n\t"
        "vrgather.vv v9, v1, v5\n\t"
        "vrgather.vv v10, v2, v6\n\t"
        "vrgather.vv v11, v3, v7\n\t"
        "vxor.vv v0, v0, v9\n\t"
        "vxor.vv v1, v10, v11\n\t"
        "vxor.vv v0, v0, v1\n\t"
        "vse8.v v0, (%[xmm])"
        :
        : [xmm] "r" (&emu->xmm[xmm])
        , [ff] "r" (ff_mult3), [mcshift] "r" (mcshift)
        , [shift] "r" (shiftrows), [sub] "r" (subbytes)
        : "memory", "a7", "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11");
}
__attribute__((target("arch=+v")))
void rvv_aesdlast(x64emu_t* emu, int xmm) {
    asm volatile(
        "li a7, 16\n\t"
        "vsetvli zero, a7, e8, m1\n\t"
        "vle8.v v4, (%[xmm])\n\t"
        "vle8.v v0, (%[shift])\n\t"
        "vrgather.vv v1, v4, v0\n\t"
        "vluxei8.v v4, (%[sub]), v1\n\t"
        "vse8.v v4, (%[xmm])"
        :
        : [xmm] "r" (&emu->xmm[xmm])
        , [shift] "r" (invshiftrows), [sub] "r" (invsubbytes)
        : "memory", "a7", "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11");
}
__attribute__((target("arch=+v")))
void rvv_aeselast(x64emu_t* emu, int xmm) {
    asm volatile(
        "li a7, 16\n\t"
        "vsetvli zero, a7, e8, m1\n\t"
        "vle8.v v4, (%[xmm])\n\t"
        "vle8.v v0, (%[shift])\n\t"
        "vrgather.vv v1, v4, v0\n\t"
        "vluxei8.v v4, (%[sub]), v1\n\t"
        "vse8.v v4, (%[xmm])"
        :
        : [xmm] "r" (&emu->xmm[xmm])
        , [shift] "r" (shiftrows), [sub] "r" (subbytes)
        : "memory", "a7", "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11");
}
