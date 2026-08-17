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
#include "x64trace.h"
#include "signals.h"
#include "dynarec_native.h"
#include "la64_mapping.h"
#include "dynarec_la64_private.h"
#include "dynarec_la64_functions.h"
#include "custommem.h"
#include "bridge.h"
#include "gdbjit.h"
#include "perfmap.h"
#include "elfloader.h"

#define XMM0 0
#define X870 XMM0 + 16
#define EMM0 XMM0 + 16

// Get a FPU scratch reg
int fpu_get_scratch(dynarec_la64_t* dyn)
{
    int ret = SCRATCH0 + dyn->lsx.fpu_scratch++; // return an Sx
    if (ret >= SCRATCH_LIMIT) {
        dyn->abort = 1;
        ret = SCRATCH_LIMIT - 1;
    }
    return ret;
}
// Reset scratch regs counter
void fpu_reset_scratch(dynarec_la64_t* dyn)
{
    dyn->lsx.fpu_scratch = 0;
    dyn->lsx.ymm_used = 0;
    dyn->lsx.xmm_used = 0;
    dyn->lsx.ymm_load = 0;
    dyn->lsx.xmm_load = 0;
}
// Get a x87 double reg
int fpu_get_reg_x87(dynarec_la64_t* dyn, int t, int n)
{
    int i = X870;
    while (i < 24 && dyn->lsx.fpuused[i])
        ++i;
    if (i >= 24) {
        dyn->abort = 1;
        i = 23;
    }
    dyn->lsx.fpuused[i] = 1;
    dyn->lsx.lsxcache[i].n = n;
    dyn->lsx.lsxcache[i].t = t;
    dyn->lsx.news |= (1 << i);
    dyn->use_x87 = 1;
    return i; // return a Dx
}
// Free a FPU double reg
void fpu_free_reg(dynarec_la64_t* dyn, int reg)
{
    // TODO: check upper limit?
    dyn->lsx.fpuused[reg] = 0;
    if (dyn->lsx.lsxcache[reg].t != LSX_CACHE_ST_F && dyn->lsx.lsxcache[reg].t != LSX_CACHE_ST_D && dyn->lsx.lsxcache[reg].t != LSX_CACHE_ST_I64)
        dyn->lsx.lsxcache[reg].v = 0;
}
// Get an MMX double reg
int fpu_get_reg_emm(dynarec_la64_t* dyn, int emm)
{
    int ret = EMM0 + emm;
    dyn->lsx.fpuused[ret] = 1;
    dyn->lsx.lsxcache[ret].t = LSX_CACHE_MM;
    dyn->lsx.lsxcache[ret].n = emm;
    dyn->lsx.news |= (1 << (ret));
    dyn->use_mmx = 1;
    return ret;
}
// Get an XMM quad reg
int fpu_get_reg_xmm(dynarec_la64_t* dyn, int t, int xmm)
{
    int i;
    i = XMM0 + xmm;

    dyn->lsx.fpuused[i] = 1;
    dyn->lsx.lsxcache[i].t = t;
    dyn->lsx.lsxcache[i].n = xmm;
    dyn->lsx.news |= (1 << i);
    dyn->use_xmm = 1;
    return i;
}

int fpu_get_reg_xmm_scalar(dynarec_la64_t* dyn, int t, int xmm)
{
    if (dyn->use_x87 || dyn->use_mmx) return -1;
    int i = X870;
    while (i < 24 && dyn->lsx.fpuused[i])
        ++i;
    if (i >= 24) return -1;
    dyn->lsx.fpuused[i] = 1;
    dyn->lsx.lsxcache[i].t = t;
    dyn->lsx.lsxcache[i].n = xmm;
    dyn->lsx.scalarcache[xmm] = i;
    dyn->lsx.news |= 1u << i;
    dyn->use_xmm = 1;
    return i;
}

// Get an YMM quad reg
int fpu_get_reg_ymm(dynarec_la64_t* dyn, int t, int ymm)
{
    int i;
    i = XMM0 + ymm;

    dyn->lsx.fpuused[i] = 1;
    dyn->lsx.lsxcache[i].t = t;
    dyn->lsx.lsxcache[i].n = ymm;
    dyn->lsx.news |= (1 << i);
    dyn->use_ymm = 1;
    return i;
}

// Reset fpu regs counter
static void fpu_reset_reg_lsxcache(lsxcache_t* lsx)
{
    for (int i = 0; i < 24; ++i) {
        lsx->fpuused[i] = 0;
        lsx->lsxcache[i].v = 0;
    }
}

void fpu_reset_reg(dynarec_la64_t* dyn)
{
    fpu_reset_reg_lsxcache(&dyn->lsx);
}


int lsxcache_no_i64(dynarec_la64_t* dyn, int ninst, int st, int a)
{
    if (a == LSX_CACHE_ST_I64) {
        lsxcache_promote_double(dyn, ninst, st);
        return LSX_CACHE_ST_D;
    }
    return a;
}

int lsxcache_get_st(dynarec_la64_t* dyn, int ninst, int a)
{
    if (dyn->insts[ninst].lsx.swapped) {
        if (dyn->insts[ninst].lsx.combined1 == a)
            a = dyn->insts[ninst].lsx.combined2;
        else if (dyn->insts[ninst].lsx.combined2 == a)
            a = dyn->insts[ninst].lsx.combined1;
    }
    for (int i = 0; i < 24; ++i)
        if ((dyn->insts[ninst].lsx.lsxcache[i].t == LSX_CACHE_ST_F
                || dyn->insts[ninst].lsx.lsxcache[i].t == LSX_CACHE_ST_D
                || dyn->insts[ninst].lsx.lsxcache[i].t == LSX_CACHE_ST_I64)
            && dyn->insts[ninst].lsx.lsxcache[i].n == a)
            return dyn->insts[ninst].lsx.lsxcache[i].t;
    // not in the cache yet, so will be fetched...
    return LSX_CACHE_ST_D;
}

int lsxcache_get_current_st(dynarec_la64_t* dyn, int ninst, int a)
{
    (void)ninst;
    if (!dyn->insts)
        return LSX_CACHE_ST_D;
    for (int i = 0; i < 24; ++i)
        if ((dyn->lsx.lsxcache[i].t == LSX_CACHE_ST_F
                || dyn->lsx.lsxcache[i].t == LSX_CACHE_ST_D
                || dyn->lsx.lsxcache[i].t == LSX_CACHE_ST_I64)
            && dyn->lsx.lsxcache[i].n == a)
            return dyn->lsx.lsxcache[i].t;
    // not in the cache yet, so will be fetched...
    return LSX_CACHE_ST_D;
}

int lsxcache_get_st_f(dynarec_la64_t* dyn, int ninst, int a)
{
    for (int i = 0; i < 24; ++i)
        if (dyn->insts[ninst].lsx.lsxcache[i].t == LSX_CACHE_ST_F
            && dyn->insts[ninst].lsx.lsxcache[i].n == a)
            return i;
    return -1;
}

int lsxcache_get_st_f_i64(dynarec_la64_t* dyn, int ninst, int a)
{
    for (int i = 0; i < 24; ++i)
        if ((dyn->insts[ninst].lsx.lsxcache[i].t == LSX_CACHE_ST_I64 || dyn->insts[ninst].lsx.lsxcache[i].t == LSX_CACHE_ST_F)
            && dyn->insts[ninst].lsx.lsxcache[i].n == a)
            return i;
    return -1;
}

int lsxcache_get_current_st_f(dynarec_la64_t* dyn, int a)
{
    for (int i = 0; i < 24; ++i)
        if (dyn->lsx.lsxcache[i].t == LSX_CACHE_ST_F
            && dyn->lsx.lsxcache[i].n == a)
            return i;
    return -1;
}

int lsxcache_get_current_st_f_i64(dynarec_la64_t* dyn, int a)
{
    for (int i = 0; i < 24; ++i)
        if ((dyn->lsx.lsxcache[i].t == LSX_CACHE_ST_I64 || dyn->lsx.lsxcache[i].t == LSX_CACHE_ST_F)
            && dyn->lsx.lsxcache[i].n == a)
            return i;
    return -1;
}

static void lsxcache_promote_double_forward(dynarec_la64_t* dyn, int ninst, int maxinst, int a);
static void lsxcache_promote_double_internal(dynarec_la64_t* dyn, int ninst, int maxinst, int a);
static void lsxcache_promote_double_combined(dynarec_la64_t* dyn, int ninst, int maxinst, int a)
{
    if (a == dyn->insts[ninst].lsx.combined1 || a == dyn->insts[ninst].lsx.combined2) {
        if (a == dyn->insts[ninst].lsx.combined1) {
            a = dyn->insts[ninst].lsx.combined2;
        } else
            a = dyn->insts[ninst].lsx.combined1;
        int i = lsxcache_get_st_f_i64(dyn, ninst, a);
        if (i >= 0) {
            dyn->insts[ninst].lsx.lsxcache[i].t = LSX_CACHE_ST_D;
            if (dyn->insts[ninst].x87precision) dyn->need_x87check = 2;
            if (!dyn->insts[ninst].lsx.barrier)
                lsxcache_promote_double_internal(dyn, ninst - 1, maxinst, a - dyn->insts[ninst].lsx.stack_push);
            // go forward is combined is not pop'd
            if (a - dyn->insts[ninst].lsx.stack_pop >= 0)
                if (!dyn->insts[ninst + 1].lsx.barrier)
                    lsxcache_promote_double_forward(dyn, ninst + 1, maxinst, a - dyn->insts[ninst].lsx.stack_pop);
        }
    }
}
static void lsxcache_promote_double_internal(dynarec_la64_t* dyn, int ninst, int maxinst, int a)
{
    if (dyn->insts[ninst + 1].lsx.barrier)
        return;
    while (ninst >= 0) {
        a += dyn->insts[ninst].lsx.stack_pop; // adjust Stack depth: add pop'd ST (going backward)
        int i = lsxcache_get_st_f_i64(dyn, ninst, a);
        if (i < 0) return;
        dyn->insts[ninst].lsx.lsxcache[i].t = LSX_CACHE_ST_D;
        if (dyn->insts[ninst].x87precision) dyn->need_x87check = 2;
        // check combined propagation too
        if (dyn->insts[ninst].lsx.combined1 || dyn->insts[ninst].lsx.combined2) {
            if (dyn->insts[ninst].lsx.swapped) {
                // if(dyn->need_dump) dynarec_log(LOG_NONE, "lsxcache_promote_double_internal, ninst=%d swapped %d/%d vs %d with st %d\n", ninst, dyn->insts[ninst].e.combined1 ,dyn->insts[ninst].e.combined2, a, dyn->insts[ninst].e.stack);
                if (a == dyn->insts[ninst].lsx.combined1)
                    a = dyn->insts[ninst].lsx.combined2;
                else if (a == dyn->insts[ninst].lsx.combined2)
                    a = dyn->insts[ninst].lsx.combined1;
            } else {
                lsxcache_promote_double_combined(dyn, ninst, maxinst, a);
            }
        }
        a -= dyn->insts[ninst].lsx.stack_push; // // adjust Stack depth: remove push'd ST (going backward)
        --ninst;
        if (ninst < 0 || a < 0 || dyn->insts[ninst].lsx.barrier)
            return;
    }
}

static void lsxcache_promote_double_forward(dynarec_la64_t* dyn, int ninst, int maxinst, int a)
{
    while ((ninst != -1) && (ninst < maxinst) && (a >= 0)) {
        a += dyn->insts[ninst].lsx.stack_push; // // adjust Stack depth: add push'd ST (going forward)
        if ((dyn->insts[ninst].lsx.combined1 || dyn->insts[ninst].lsx.combined2) && dyn->insts[ninst].lsx.swapped) {
            // if(dyn->need_dump) dynarec_log(LOG_NONE, "lsxcache_promote_double_forward, ninst=%d swapped %d/%d vs %d with st %d\n", ninst, dyn->insts[ninst].e.combined1 ,dyn->insts[ninst].e.combined2, a, dyn->insts[ninst].e.stack);
            if (a == dyn->insts[ninst].lsx.combined1)
                a = dyn->insts[ninst].lsx.combined2;
            else if (a == dyn->insts[ninst].lsx.combined2)
                a = dyn->insts[ninst].lsx.combined1;
        }
        int i = lsxcache_get_st_f_i64(dyn, ninst, a);
        if (i < 0) return;
        dyn->insts[ninst].lsx.lsxcache[i].t = LSX_CACHE_ST_D;
        if (dyn->insts[ninst].x87precision) dyn->need_x87check = 2;
        // check combined propagation too
        if ((dyn->insts[ninst].lsx.combined1 || dyn->insts[ninst].lsx.combined2) && !dyn->insts[ninst].lsx.swapped) {
            // if(dyn->need_dump) dynarec_log(LOG_NONE, "lsxcache_promote_double_forward, ninst=%d combined %d/%d vs %d with st %d\n", ninst, dyn->insts[ninst].e.combined1 ,dyn->insts[ninst].e.combined2, a, dyn->insts[ninst].e.stack);
            lsxcache_promote_double_combined(dyn, ninst, maxinst, a);
        }
        a -= dyn->insts[ninst].lsx.stack_pop; // adjust Stack depth: remove pop'd ST (going forward)
        if (dyn->insts[ninst].x64.has_next && !dyn->insts[ninst].lsx.barrier)
            ++ninst;
        else
            ninst = -1;
    }
    if (ninst == maxinst)
        lsxcache_promote_double(dyn, ninst, a);
}

void lsxcache_promote_double(dynarec_la64_t* dyn, int ninst, int a)
{
    int i = lsxcache_get_current_st_f_i64(dyn, a);
    if (i < 0) return;
    dyn->lsx.lsxcache[i].t = LSX_CACHE_ST_D;
    dyn->insts[ninst].lsx.lsxcache[i].t = LSX_CACHE_ST_D;
    if (dyn->insts[ninst].x87precision) dyn->need_x87check = 2;
    // check combined propagation too
    if (dyn->lsx.combined1 || dyn->lsx.combined2) {
        if (dyn->lsx.swapped) {
            if (dyn->lsx.combined1 == a)
                a = dyn->lsx.combined2;
            else if (dyn->lsx.combined2 == a)
                a = dyn->lsx.combined1;
        } else {
            if (dyn->lsx.combined1 == a)
                lsxcache_promote_double(dyn, ninst, dyn->lsx.combined2);
            else if (dyn->lsx.combined2 == a)
                lsxcache_promote_double(dyn, ninst, dyn->lsx.combined1);
        }
    }
    a -= dyn->insts[ninst].lsx.stack_push; // // adjust Stack depth: remove push'd ST (going backward)
    if (!ninst || a < 0) return;
    lsxcache_promote_double_internal(dyn, ninst - 1, ninst, a);
}

int lsxcache_combine_st(dynarec_la64_t* dyn, int ninst, int a, int b)
{
    dyn->lsx.combined1 = a;
    dyn->lsx.combined2 = b;
    if (lsxcache_get_current_st(dyn, ninst, a) == LSX_CACHE_ST_F
        && lsxcache_get_current_st(dyn, ninst, b) == LSX_CACHE_ST_F)
        return LSX_CACHE_ST_F;
    return LSX_CACHE_ST_D;
}

static int isCacheEmpty(dynarec_native_t* dyn, int ninst)
{
    if (dyn->insts[ninst].lsx.stack_next) {
        return 0;
    }
    for (int i = 0; i < 24; ++i)
        if (dyn->insts[ninst].lsx.lsxcache[i].v) { // there is something at ninst for i
            if (!(
                    (dyn->insts[ninst].lsx.lsxcache[i].t == LSX_CACHE_ST_F
                        || dyn->insts[ninst].lsx.lsxcache[i].t == LSX_CACHE_ST_D
                        || dyn->insts[ninst].lsx.lsxcache[i].t == LSX_CACHE_ST_I64)
                    && dyn->insts[ninst].lsx.lsxcache[i].n < dyn->insts[ninst].lsx.stack_pop))
                return 0;
        }
    return 1;
}

int fpuCacheNeedsTransform(dynarec_la64_t* dyn, int ninst)
{
    int i2 = dyn->insts[ninst].x64.jmp_insts;
    if (i2 < 0) return 1;
    if ((dyn->insts[i2].x64.barrier & BARRIER_FLOAT))
        // if the barrier as already been apply, no transform needed
        return ((dyn->insts[ninst].x64.barrier & BARRIER_FLOAT)) ? 0 : (isCacheEmpty(dyn, ninst) ? 0 : 1);
    int ret = 0;
    if (!i2) { // just purge
        if (dyn->insts[ninst].lsx.stack_next) {
            return 1;
        }
        for (int i = 0; i < 24 && !ret; ++i)
            if (dyn->insts[ninst].lsx.lsxcache[i].v) { // there is something at ninst for i
                if (!(
                        (dyn->insts[ninst].lsx.lsxcache[i].t == LSX_CACHE_ST_F
                            || dyn->insts[ninst].lsx.lsxcache[i].t == LSX_CACHE_ST_D
                            || dyn->insts[ninst].lsx.lsxcache[i].t == LSX_CACHE_ST_I64)
                        && dyn->insts[ninst].lsx.lsxcache[i].n < dyn->insts[ninst].lsx.stack_pop))
                    ret = 1;
            }
        return ret;
    }
    // Check if ninst can be compatible to i2
    if (dyn->insts[ninst].lsx.stack_next != dyn->insts[i2].lsx.stack - dyn->insts[i2].lsx.stack_push) {
        return 1;
    }
    lsxcache_t cache_i2 = dyn->insts[i2].lsx;
    lsxcacheUnwind(&cache_i2);

    for (int i = 0; i < 24; ++i) {
        if (dyn->insts[ninst].lsx.lsxcache[i].v) { // there is something at ninst for i
            if (!cache_i2.lsxcache[i].v) {         // but there is nothing at i2 for i
                ret = 1;
            } else if (dyn->insts[ninst].lsx.lsxcache[i].v != cache_i2.lsxcache[i].v) { // there is something different
                if (dyn->insts[ninst].lsx.lsxcache[i].n != cache_i2.lsxcache[i].n) {    // not the same x64 reg
                    ret = 1;
                } else if (dyn->insts[ninst].lsx.lsxcache[i].t == LSX_CACHE_XMMR && cache_i2.lsxcache[i].t == LSX_CACHE_XMMW) { /* nothing */
                } else if (dyn->insts[ninst].lsx.lsxcache[i].t == LSX_CACHE_YMMR && cache_i2.lsxcache[i].t == LSX_CACHE_YMMW) { /* nothing */
                } else
                    ret = 1;
            }
        } else if (cache_i2.lsxcache[i].v)
            ret = 1;
    }
    return ret;
}

void lsxcacheUnwind(lsxcache_t* cache)
{
    if (cache->swapped) {
        // unswap
        int a = -1;
        int b = -1;
        for (int j = 0; j < 24 && ((a == -1) || (b == -1)); ++j)
            if ((cache->lsxcache[j].t == LSX_CACHE_ST_D || cache->lsxcache[j].t == LSX_CACHE_ST_F || cache->lsxcache[j].t == LSX_CACHE_ST_I64)) {
                if (cache->lsxcache[j].n == cache->combined1)
                    a = j;
                else if (cache->lsxcache[j].n == cache->combined2)
                    b = j;
            }
        if (a != -1 && b != -1) {
            int tmp = cache->lsxcache[a].n;
            cache->lsxcache[a].n = cache->lsxcache[b].n;
            cache->lsxcache[b].n = tmp;
        }
        cache->swapped = 0;
        cache->combined1 = cache->combined2 = 0;
    }
    if (cache->news) {
        // reove the newly created lsxcache
        for (int i = 0; i < 24; ++i)
            if (cache->news & (1 << i))
                cache->lsxcache[i].v = 0;
        cache->news = 0;
    }
    if (cache->stack_push) {
        // unpush
        for (int j = 0; j < 24; ++j) {
            if ((cache->lsxcache[j].t == LSX_CACHE_ST_D || cache->lsxcache[j].t == LSX_CACHE_ST_F || cache->lsxcache[j].t == LSX_CACHE_ST_I64)) {
                if (cache->lsxcache[j].n < cache->stack_push)
                    cache->lsxcache[j].v = 0;
                else
                    cache->lsxcache[j].n -= cache->stack_push;
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
    // And now, rebuild the x87cache info with lsxcache
    cache->mmxcount = 0;
    cache->fpu_scratch = 0;
    for (int i = 0; i < 8; ++i) {
        cache->x87cache[i] = -1;
        cache->mmxcache[i] = -1;
        cache->x87reg[i] = 0;
        cache->ssecache[i * 2].v = -1;
        cache->ssecache[i * 2 + 1].v = -1;
        cache->avxcache[i * 2].v = -1;
        cache->avxcache[i * 2 + 1].v = -1;
        cache->scalarcache[i * 2] = -1;
        cache->scalarcache[i * 2 + 1] = -1;
    }
    int x87reg = 0;
    for (int i = 0; i < 24; ++i) {
        if (cache->lsxcache[i].v) {
            cache->fpuused[i] = 1;
            switch (cache->lsxcache[i].t) {
                case LSX_CACHE_MM:
                    cache->mmxcache[cache->lsxcache[i].n] = i;
                    ++cache->mmxcount;
                    break;
                case LSX_CACHE_XMMR:
                case LSX_CACHE_XMMW:
                    cache->ssecache[cache->lsxcache[i].n].reg = i;
                    cache->ssecache[cache->lsxcache[i].n].write = (cache->lsxcache[i].t == LSX_CACHE_XMMW) ? 1 : 0;
                    break;
                case LSX_CACHE_XMM_S:
                case LSX_CACHE_XMM_D:
                    cache->scalarcache[cache->lsxcache[i].n] = i;
                    break;
                case LSX_CACHE_YMMR:
                case LSX_CACHE_YMMW:
                    cache->avxcache[cache->lsxcache[i].n].reg = i;
                    cache->avxcache[cache->lsxcache[i].n].upper_zero_pending = 0;
                    cache->avxcache[cache->lsxcache[i].n].write = (cache->lsxcache[i].t == LSX_CACHE_YMMW) ? 1 : 0;
                    break;
                case LSX_CACHE_ST_F:
                case LSX_CACHE_ST_D:
                case LSX_CACHE_ST_I64:
                    cache->x87cache[x87reg] = cache->lsxcache[i].n;
                    cache->x87reg[x87reg] = i;
                    ++x87reg;
                    break;
                case LSX_CACHE_SCR:
                    cache->fpuused[i] = 0;
                    cache->lsxcache[i].v = 0;
                    break;
            }
        } else {
            cache->fpuused[i] = 0;
        }
    }
}

const char* getCacheName(int t, int n)
{
    static char buff[20];
    switch (t) {
        case LSX_CACHE_ST_D: sprintf(buff, "ST%d", n); break;
        case LSX_CACHE_ST_F: sprintf(buff, "st%d", n); break;
        case LSX_CACHE_ST_I64: sprintf(buff, "STi%d", n); break;
        case LSX_CACHE_MM: sprintf(buff, "MM%d", n); break;
        case LSX_CACHE_XMMW: sprintf(buff, "XMM%d", n); break;
        case LSX_CACHE_XMMR: sprintf(buff, "xmm%d", n); break;
        case LSX_CACHE_XMM_S: sprintf(buff, "XMM%d.s", n); break;
        case LSX_CACHE_XMM_D: sprintf(buff, "XMM%d.d", n); break;
        case LSX_CACHE_YMMW: sprintf(buff, "YMM%d", n); break;
        case LSX_CACHE_YMMR: sprintf(buff, "ymm%d", n); break;
        case LSX_CACHE_SCR: sprintf(buff, "Scratch"); break;
        case LSX_CACHE_NONE: buff[0] = '\0'; break;
    }
    return buff;
}

static register_mapping_t register_mappings[] = {
    { "rax", "t0" },
    { "eax", "t0" },
    { "ax", "t0" },
    { "ah", "t0" },
    { "al", "t0" },
    { "rcx", "t1" },
    { "ecx", "t1" },
    { "cx", "t1" },
    { "ch", "t1" },
    { "cl", "t1" },
    { "rdx", "t2" },
    { "edx", "t2" },
    { "dx", "t2" },
    { "dh", "t2" },
    { "dl", "t2" },
    { "rbx", "t3" },
    { "ebx", "t3" },
    { "bx", "t3" },
    { "bh", "t3" },
    { "bl", "t3" },
    { "rsi", "t4" },
    { "esi", "t4" },
    { "si", "t4" },
    { "sil", "t4" },
    { "rdi", "t5" },
    { "edi", "t5" },
    { "di", "t5" },
    { "dil", "t5" },
    { "rsp", "t6" },
    { "esp", "t6" },
    { "sp", "t6" },
    { "spl", "t6" },
    { "rbp", "t7" },
    { "ebp", "t7" },
    { "bp", "t7" },
    { "bpl", "t7" },
    { "r8", "s0" },
    { "r8d", "s0" },
    { "r8w", "s0" },
    { "r8b", "s0" },
    { "r9", "s1" },
    { "r9d", "s1" },
    { "r9w", "s1" },
    { "r9b", "s1" },
    { "r10", "s2" },
    { "r10d", "s2" },
    { "r10w", "s2" },
    { "r10b", "s2" },
    { "r11", "s3" },
    { "r11d", "s3" },
    { "r11w", "s3" },
    { "r11b", "s3" },
    { "r12", "s4" },
    { "r12d", "s4" },
    { "r12w", "s4" },
    { "r12b", "s4" },
    { "r13", "s5" },
    { "r13d", "s5" },
    { "r13w", "s5" },
    { "r13b", "s5" },
    { "r14", "s6" },
    { "r14d", "s6" },
    { "r14w", "s6" },
    { "r14b", "s6" },
    { "r15", "s7" },
    { "r15d", "s7" },
    { "r15w", "s7" },
    { "r15b", "s7" },
    { "rip", "t8" },
};

static const char* Ft[] = { "fa0", "fa1", "fa2", "fa3", "fa4", "fa5", "fa6", "fa7", "ft0", "ft1", "ft2", "ft3", "ft4", "ft5", "ft6", "ft7", "ft8", "ft9", "ft10", "ft11", "ft12", "ft13", "ft14", "ft15", "fs0", "fs1", "fs2", "fs3", "fs4", "fs5", "fs6", "fs7" };
static const char* Vt[] = { "vra0", "vra1", "vra2", "vra3", "vra4", "vra5", "vra6", "vra7", "vrt0", "vrt1", "vrt2", "vrt3", "vrt4", "vrt5", "vrt6", "vrt7", "vrt8", "vrt9", "vrt10", "vrt11", "vrt12", "vrt13", "vrt14", "vrt15", "vrs0", "vrs1", "vrs2", "vrs3", "vrs4", "vrs5", "vrs6", "vrs7" };
static const char* XVt[] = { "xvra0", "xvra1", "xvra2", "xvra3", "xvra4", "xvra5", "xvra6", "xvra7", "xvrt0", "xvrt1", "xvrt2", "xvrt3", "xvrt4", "xvrt5", "xvrt6", "xvrt7", "xvrt8", "xvrt9", "xvrt10", "xvrt11", "xvrt12", "xvrt13", "xvrt14", "xvrt15", "xvrs0", "xvrs1", "xvrs2", "xvrs3", "xvrs4", "xvrs5", "xvrs6", "xvrs7" };

static const char* df_status[] = { "unknown", "set", "none_pending", "none" };

void printf_x64_instruction(dynarec_native_t* dyn, zydis_dec_t* dec, instruction_x64_t* inst, const char* name);
void inst_name_pass3(dynarec_native_t* dyn, int ninst, const char* name, rex_t rex)
{
    if (dyn->need_dump == 3) {
        printf_x64_instruction(dyn, rex.is32bits ? my_context->dec32 : my_context->dec, &dyn->insts[ninst].x64, name);
        if (!BOX64ENV(dynarec_gdbjit) && !BOX64ENV(dynarec_perf_map)) return;
    }
    if (!dyn->need_dump && !BOX64ENV(dynarec_gdbjit) && !BOX64ENV(dynarec_perf_map)) return;

    static char buf[4096];
    int length = sprintf(buf, "barrier=%d state=%d/%s(%s->%s), set=%X/%X, use=%X, need=%X/%X, fuse=%d/%d, sm=%d(%d/%d)",
        dyn->insts[ninst].x64.barrier,
        dyn->insts[ninst].x64.state_flags,
        df_status[dyn->f],
        df_status[dyn->insts[ninst].f_entry],
        df_status[dyn->insts[ninst].f_exit],
        dyn->insts[ninst].x64.set_flags,
        dyn->insts[ninst].x64.gen_flags,
        dyn->insts[ninst].x64.use_flags,
        dyn->insts[ninst].x64.need_before,
        dyn->insts[ninst].x64.need_after,
        dyn->insts[ninst].nat_flags_fusion,
        dyn->insts[ninst].no_scratch_usage,
        dyn->smwrite, dyn->insts[ninst].will_write, dyn->insts[ninst].last_write);
    if (dyn->insts[ninst].pred_sz) {
        length += sprintf(buf + length, ", pred=");
        for (int ii = 0; ii < dyn->insts[ninst].pred_sz; ++ii)
            length += sprintf(buf + length, "%s%d", ii ? "/" : "", dyn->insts[ninst].pred[ii]);
    }
    if (dyn->insts[ninst].x64.jmp && dyn->insts[ninst].x64.jmp_insts >= 0)
        length += sprintf(buf + length, ", jmp=%d", dyn->insts[ninst].x64.jmp_insts);
    if (dyn->insts[ninst].x64.jmp && dyn->insts[ninst].x64.jmp_insts == -1)
        length += sprintf(buf + length, ", jmp=out");
    if (dyn->insts[ninst].preload_xmmymm)
        length += sprintf(buf + length, ", preload=%x/%d", dyn->insts[ninst].preload_xmmymm, dyn->insts[ninst].preload_from);
    if (dyn->last_ip)
        length += sprintf(buf + length, ", last_ip=%p", (void*)dyn->last_ip);
    for (int ii = 0; ii < 24; ++ii) {
        switch (dyn->insts[ninst].lsx.lsxcache[ii].t) {
            case LSX_CACHE_ST_D: length += sprintf(buf + length, " D%d:%s", ii, getCacheName(dyn->insts[ninst].lsx.lsxcache[ii].t, dyn->insts[ninst].lsx.lsxcache[ii].n)); break;
            case LSX_CACHE_ST_F: length += sprintf(buf + length, " S%d:%s", ii, getCacheName(dyn->insts[ninst].lsx.lsxcache[ii].t, dyn->insts[ninst].lsx.lsxcache[ii].n)); break;
            case LSX_CACHE_ST_I64: length += sprintf(buf + length, " D%d:%s", ii, getCacheName(dyn->insts[ninst].lsx.lsxcache[ii].t, dyn->insts[ninst].lsx.lsxcache[ii].n)); break;
            case LSX_CACHE_MM: length += sprintf(buf + length, " %s:%s", Ft[ii], getCacheName(dyn->insts[ninst].lsx.lsxcache[ii].t, dyn->insts[ninst].lsx.lsxcache[ii].n)); break;
            case LSX_CACHE_XMMW: length += sprintf(buf + length, " %s:%s", Vt[ii], getCacheName(dyn->insts[ninst].lsx.lsxcache[ii].t, dyn->insts[ninst].lsx.lsxcache[ii].n)); break;
            case LSX_CACHE_XMMR: length += sprintf(buf + length, " %s:%s", Vt[ii], getCacheName(dyn->insts[ninst].lsx.lsxcache[ii].t, dyn->insts[ninst].lsx.lsxcache[ii].n)); break;
            case LSX_CACHE_XMM_S: length += sprintf(buf + length, " %s:%s", Ft[ii], getCacheName(dyn->insts[ninst].lsx.lsxcache[ii].t, dyn->insts[ninst].lsx.lsxcache[ii].n)); break;
            case LSX_CACHE_XMM_D: length += sprintf(buf + length, " %s:%s", Ft[ii], getCacheName(dyn->insts[ninst].lsx.lsxcache[ii].t, dyn->insts[ninst].lsx.lsxcache[ii].n)); break;
            case LSX_CACHE_YMMW: length += sprintf(buf + length, " %s:%s", XVt[ii], getCacheName(dyn->insts[ninst].lsx.lsxcache[ii].t, dyn->insts[ninst].lsx.lsxcache[ii].n)); break;
            case LSX_CACHE_YMMR: length += sprintf(buf + length, " %s:%s", XVt[ii], getCacheName(dyn->insts[ninst].lsx.lsxcache[ii].t, dyn->insts[ninst].lsx.lsxcache[ii].n)); break;
            case LSX_CACHE_SCR: length += sprintf(buf + length, " %s:%s", Ft[ii], getCacheName(dyn->insts[ninst].lsx.lsxcache[ii].t, dyn->insts[ninst].lsx.lsxcache[ii].n)); break;
            case LSX_CACHE_NONE:
            default: break;
        }
    }
    if (dyn->lsx.stack || dyn->insts[ninst].lsx.stack_next || dyn->insts[ninst].lsx.x87stack)
        length += sprintf(buf + length, " X87:%d/%d(+%d/-%d)%d", dyn->lsx.stack, dyn->insts[ninst].lsx.stack_next, dyn->insts[ninst].lsx.stack_push, dyn->insts[ninst].lsx.stack_pop, dyn->insts[ninst].lsx.x87stack);
    if (dyn->insts[ninst].lsx.combined1 || dyn->insts[ninst].lsx.combined2)
        length += sprintf(buf + length, " %s:%d/%d", dyn->insts[ninst].lsx.swapped ? "SWP" : "CMB", dyn->insts[ninst].lsx.combined1, dyn->insts[ninst].lsx.combined2);
    if(dyn->insts[ninst].x64.self_loop)
        length += sprintf(buf + length, " self-loop");

    if (dyn->need_dump && dyn->need_dump != 3) {
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
        writePerfMap(dyn->insts[ninst].x64.addr, dyn->native_start + dyn->insts[ninst].address, dyn->insts[ninst].size, name);
    }
    if (length > sizeof(buf)) printf_log(LOG_NONE, "Warning: buf to small in inst_name_pass3 (%d vs %zd)\n", length, sizeof(buf));
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

static uint32_t insert_half(uint32_t val, uint16_t h, void* address)
{
    int idx = (((uintptr_t)address) & 3) * 8;
    val &= ~((uint32_t)0xffff << idx);
    val |= (((uint32_t)h) << idx);
    return val;
}

uint8_t la64_lock_xchg_b_slow(void* addr, uint8_t val)
{
    uint32_t ret;
    uint32_t* aligned = (uint32_t*)(((uintptr_t)addr) & ~3);
    do {
        ret = *aligned;
    } while (la64_lock_cas_d(aligned, ret, insert_byte(ret, val, addr)));
    return extract_byte(ret, addr);
}

int la64_lock_cas_b_slow(void* addr, uint8_t ref, uint8_t val)
{
    uint32_t* aligned = (uint32_t*)(((uintptr_t)addr) & ~3);
    uint32_t tmp = *aligned;
    return la64_lock_cas_d(aligned, insert_byte(tmp, ref, addr), insert_byte(tmp, val, addr));
}

int la64_lock_cas_h_slow(void* addr, uint16_t ref, uint16_t val)
{
    if ((((uintptr_t)addr) & 7) == 7) {
        pthread_mutex_lock(&my_context->mutex_lock);
        uint16_t* p = (uint16_t*)addr;
        uint16_t old = *p;
        if (old == ref)
            *p = val;
        pthread_mutex_unlock(&my_context->mutex_lock);
        return old != ref;
    }
    if ((((uintptr_t)addr) & 3) != 3) {
        uint32_t* aligned = (uint32_t*)(((uintptr_t)addr) & ~3);
        uint32_t tmp = *aligned;
        return la64_lock_cas_d(aligned, insert_half(tmp, ref, addr), insert_half(tmp, val, addr));
    }
    uint64_t* aligned = (uint64_t*)(((uintptr_t)addr) & ~7ULL);
    int idx = (((uintptr_t)addr) & 7) * 8; // 24 here
    uint64_t mask = ~((uint64_t)0xffff << idx);
    uint64_t old = *aligned;
    uint64_t ref64 = (old & mask) | (((uint64_t)ref) << idx);
    uint64_t val64 = (old & mask) | (((uint64_t)val) << idx);
    return la64_lock_cas_dd(aligned, ref64, val64);
}

void print_opcode(dynarec_native_t* dyn, int ninst, uint32_t opcode)
{
    dynarec_log_prefix(0, LOG_NONE, "\t%08x\t%s\n", opcode, la64_print(opcode, (uintptr_t)dyn->block));
}

static void x87_reset(lsxcache_t* lsx)
{
    for (int i = 0; i < 8; ++i)
        lsx->x87cache[i] = -1;
    lsx->tags = 0;
    lsx->x87stack = 0;
    lsx->stack = 0;
    lsx->stack_next = 0;
    lsx->stack_pop = 0;
    lsx->stack_push = 0;
    lsx->combined1 = lsx->combined2 = 0;
    lsx->swapped = 0;
    lsx->barrier = 0;
    lsx->pushed = 0;
    lsx->poped = 0;

    for (int i = 0; i < 24; ++i)
        if (lsx->lsxcache[i].t == LSX_CACHE_ST_F
            || lsx->lsxcache[i].t == LSX_CACHE_ST_D
            || lsx->lsxcache[i].t == LSX_CACHE_ST_I64)
            lsx->lsxcache[i].v = 0;
}

static void mmx_reset(lsxcache_t* lsx)
{
    lsx->mmxcount = 0;
    for (int i = 0; i < 8; ++i)
        lsx->mmxcache[i] = -1;
}

static void sse_reset(lsxcache_t* lsx)
{
    for (int i = 0; i < 16; ++i) {
        lsx->ssecache[i].v = -1;
        lsx->scalarcache[i] = -1;
    }
}
static void avx_reset(lsxcache_t* lsx)
{
    for (int i = 0; i < 16; ++i)
        lsx->avxcache[i].v = -1;
}

void fpu_reset(dynarec_la64_t* dyn)
{
    x87_reset(&dyn->lsx);
    mmx_reset(&dyn->lsx);
    sse_reset(&dyn->lsx);
    avx_reset(&dyn->lsx);
    fpu_reset_reg(dyn);
}

int fpu_is_st_freed(dynarec_la64_t* dyn, int ninst, int st)
{
    return (dyn->lsx.tags & (0b11 << (st * 2))) ? 1 : 0;
}


void fpu_reset_ninst(dynarec_la64_t* dyn, int ninst)
{
    // TODO: x87 and mmx
    sse_reset(&dyn->insts[ninst].lsx);
    avx_reset(&dyn->insts[ninst].lsx);
    fpu_reset_reg_lsxcache(&dyn->insts[ninst].lsx);
}

void fpu_save_and_unwind(dynarec_la64_t* dyn, int ninst, lsxcache_t* cache)
{
    memcpy(cache, &dyn->insts[ninst].lsx, sizeof(lsxcache_t));
    lsxcacheUnwind(&dyn->insts[ninst].lsx);
}
void fpu_unwind_restore(dynarec_la64_t* dyn, int ninst, lsxcache_t* cache)
{
    memcpy(&dyn->insts[ninst].lsx, cache, sizeof(lsxcache_t));
}

static int hasLinearPredecessor(const dynarec_la64_t* dyn, int ninst)
{
    return ninst > 0 && dyn->insts[ninst].pred_sz == 1 && dyn->insts[ninst].pred[0] == ninst - 1;
}

int isUpper32Zero(dynarec_la64_t* dyn, int ninst, int reg)
{
    if (!IS_GPR(reg))
        return 0;

    const uint16_t bit = (uint16_t)(1 << TO_X64(reg));
    int current = ninst;
    for (int depth = 0; depth < 4 && hasLinearPredecessor(dyn, current); ++depth) {
        const instruction_la64_t* prev = &dyn->insts[current - 1];
        if (prev->x64.has_callret || prev->host_call || prev->x64.barrier)
            return 0;
        if (prev->up32_write64 & bit) return prev->up32_zero & bit;
        --current;
    }
    return 0;
}

void updateNativeFlags(dynarec_la64_t* dyn)
{
    if (!BOX64ENV(dynarec_nativeflags))
        return;
    for (int i = 0; i < dyn->size; ++i) {
        if (dyn->insts[i].nat_flags_fusion) {
            int j = i - 1;
            int found = 0;
            int last_fused = 0;
            if (hasLinearPredecessor(dyn, i)) {
                while (j >= 0) {
                    if (dyn->insts[j].x64.set_flags && (dyn->insts[i].x64.use_flags & dyn->insts[j].x64.set_flags) == dyn->insts[i].x64.use_flags) {
                        int needsign = (dyn->insts[i].x64.use_flags & X_SF) ? 1 : 0;
                        int needunsigned = (dyn->insts[i].x64.use_flags & X_CF) ? 1 : 0;
                        if (dyn->insts[j].nat_flags_fusion && ((needsign && dyn->insts[j].nat_flags_needunsigned) || (needunsigned && dyn->insts[j].nat_flags_needsign)))
                            break; // not compatible, stop here
                        dyn->insts[j].nat_flags_fusion = 1;
                        if (needsign)
                            dyn->insts[j].nat_flags_needsign = 1;
                        if (needunsigned)
                            dyn->insts[j].nat_flags_needunsigned = 1;
                        dyn->insts[i].x64.use_flags = 0;
                        if (last_fused) {
                            dyn->insts[last_fused].nat_next_inst = i;
                        } else {
                            uint16_t* next = &dyn->insts[j].nat_next_inst;
                            while (*next) next = &dyn->insts[*next].nat_next_inst;
                            *next = i;
                        }
                        dyn->insts[i].up32_read |= dyn->insts[j].up32_write32;
                        found = 1;
                        break;
                    } else if (hasLinearPredecessor(dyn, j) && dyn->insts[j].nat_flags_fusion && dyn->insts[j].x64.jmp && !dyn->insts[j].x64.set_flags && !dyn->insts[j].x64.use_flags) {
                        // already fused Jcc consumer
                        if (!last_fused) last_fused = j;
                        j -= 1;
                    } else if (hasLinearPredecessor(dyn, j) && dyn->insts[j].no_scratch_usage && !dyn->insts[j].x64.set_flags && !dyn->insts[j].x64.use_flags) {
                        j -= 1;
                    } else
                        break;
                }
            }
            if (!found) dyn->insts[i].nat_flags_fusion = 0;
        }

        int fusion = dyn->insts[i].comis_fusion;
        if (fusion < 0)
            continue;
        int found = 0;
        int j = i - 1;
        if (hasLinearPredecessor(dyn, i)) {
            while (j >= 0) {
                instruction_la64_t* inst = &dyn->insts[j];
                if (inst->comis_mark) {
                    inst->comis_fusion = fusion;
                    dyn->insts[i].x64.use_flags = 0;
                    found = 1;
                    break;
                }
                if (!hasLinearPredecessor(dyn, j) || inst->x64.jmp || inst->x64.barrier
                    || inst->x64.has_callret || inst->host_call
                    || inst->x64.set_flags || inst->x64.use_flags)
                    break;
                --j;
            }
        }
        if (!found) dyn->insts[i].comis_fusion = -1;
    }
}

void get_free_scratch(dynarec_la64_t* dyn, int ninst, uint8_t* tmp1, uint8_t* tmp2, uint8_t* tmp3, uint8_t s1, uint8_t s2, uint8_t s3, uint8_t s4, uint8_t s5)
{
    uint8_t n1 = dyn->insts[ninst].nat_flags_op1;
    uint8_t n2 = dyn->insts[ninst].nat_flags_op2;
    uint8_t tmp[5] = { 0 };
    int idx = 0;
#define GO(s) \
    if ((s != n1) && (s != n2)) tmp[idx++] = s
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

void tryEarlyFpuBarrier(dynarec_la64_t* dyn, int last_fpu_used, int ninst)
{
    // there is a barrier at ninst
    // check if, up to last fpu_used, if there is some suspicious jump that would prevent the barrier to be put earlier
    int usefull = 0;
    for (int i = ninst - 1; i > last_fpu_used; --i) {
        if (!dyn->insts[i].x64.has_next)
            return; // break of chain, don't try to be smart for now
        if (dyn->insts[i].x64.barrier & BARRIER_FLOAT)
            return; // already done?
        if (dyn->insts[i].x64.jmp && dyn->insts[i].x64.jmp_insts == -1)
            usefull = 1;
        if (dyn->insts[i].x64.jmp && dyn->insts[i].x64.jmp_insts != -1) {
            int i2 = dyn->insts[i].x64.jmp_insts;
            if (i2 < last_fpu_used || i2 > ninst) {
                // check if some xmm/ymm/x87 stack are used in landing point
                if (i2 > ninst) {
                    if (dyn->insts[i2].lsx.xmm_used || dyn->insts[i2].lsx.ymm_used || dyn->insts[i2].lsx.stack)
                        return;
                }
                // we will stop there, not trying to guess too much thing
                if ((usefull && (i + 1) != ninst)) {
                    if ((BOX64ENV(dynarec_dump) && BOX64ENV(dynarec_dump) != 3) || BOX64ENV(dynarec_log) > 1) dynarec_log(LOG_NONE, "Putting early Float Barrier in %d for %d\n", i + 1, ninst);
                    dyn->insts[i + 1].x64.barrier |= BARRIER_FLOAT;
                }
                return;
            }
            usefull = 1;
        }
        for (int pred = 0; pred < dyn->insts[i].pred_sz; ++pred) {
            if (dyn->insts[i].pred[pred] <= last_fpu_used) {
                if (usefull && ((i + 1) != ninst)) {
                    if ((BOX64ENV(dynarec_dump) && BOX64ENV(dynarec_dump) != 3) || BOX64ENV(dynarec_log) > 1) dynarec_log(LOG_NONE, "Putting early Float Barrier in %d for %d\n", i + 1, ninst);
                    dyn->insts[i + 1].x64.barrier |= BARRIER_FLOAT;
                }
                return;
            }
        }
        if (dyn->insts[i].pred_sz > 1)
            usefull = 1;
    }
    if (usefull) {
        if ((BOX64ENV(dynarec_dump) && BOX64ENV(dynarec_dump) != 3) || BOX64ENV(dynarec_log) > 1) dynarec_log(LOG_NONE, "Putting early Float Barrier in %d for %d\n", last_fpu_used + 1, ninst);
        dyn->insts[last_fpu_used + 1].x64.barrier |= BARRIER_FLOAT;
    }
}

void propagateFpuBarrier(dynarec_la64_t* dyn)
{
    if (!dyn->use_x87)
        return;
    int last_fpu_used = -1;
    for (int ninst = 0; ninst < dyn->size; ++ninst) {
        int fpu_used = dyn->insts[ninst].lsx.xmm_used || dyn->insts[ninst].lsx.ymm_used || dyn->insts[ninst].mmx_used || dyn->insts[ninst].x87_used;
        if (fpu_used) last_fpu_used = ninst;
        dyn->insts[ninst].fpu_used = fpu_used;
        if (dyn->insts[ninst].fpupurge && (last_fpu_used != -1) && (last_fpu_used != (ninst - 1))) {
            tryEarlyFpuBarrier(dyn, last_fpu_used, ninst);
            last_fpu_used = -1; // reset the last_fpu_used...
        }
    }
}

enum {
    UPPER_LIVENESS_VECTOR_SHIFT = 16,
    UPPER_PENDING_YMM_SHIFT = 16,
};

static vector_upper_t transferXmmCopyLiveness(vector_upper_t live_out, const vector_liveness_t* liveness)
{
    if (!liveness->xmm_copy_dst)
        return live_out;

    int dst = liveness->xmm_copy_dst - 1;
    int src = liveness->xmm_copy_src;
    uint16_t dst_bit = 1ULL << dst;
    uint16_t src_bit = 1ULL << src;
    int lane1_needed = live_out.xmm_lane1 & dst_bit;
    int lanes23_needed = live_out.xmm_lanes23 & dst_bit;
    live_out.xmm_lane1 &= (uint16_t)~dst_bit;
    live_out.xmm_lanes23 &= (uint16_t)~dst_bit;
    if (lane1_needed)
        live_out.xmm_lane1 |= src_bit;
    if (lanes23_needed)
        live_out.xmm_lanes23 |= src_bit;
    return live_out;
}

static uint32_t getFullXYMMMask(const dynarec_la64_t* dyn, int ninst)
{
    if (ninst < 0)
        return 0;
    uint32_t ret = 0;
    for (int i = 0; i < 24; ++i) {
        const lsx_cache_t cache = dyn->insts[ninst].lsx.lsxcache[i];
        if (cache.t == LSX_CACHE_XMMR || cache.t == LSX_CACHE_XMMW)
            ret |= 1u << cache.n;
        else if (cache.t == LSX_CACHE_YMMR || cache.t == LSX_CACHE_YMMW)
            ret |= 1u << (16 + cache.n);
    }
    return ret;
}

static uint16_t getCachedVectorMask(const dynarec_la64_t* dyn, int ninst)
{
    if (ninst < 0)
        return 0;
    uint16_t ret = 0;
    for (int i = 0; i < 24; ++i) {
        int type = dyn->insts[ninst].lsx.lsxcache[i].t;
        if (type == LSX_CACHE_XMMR || type == LSX_CACHE_XMMW
            || type == LSX_CACHE_XMM_S || type == LSX_CACHE_XMM_D
            || type == LSX_CACHE_YMMR || type == LSX_CACHE_YMMW)
            ret |= 1u << dyn->insts[ninst].lsx.lsxcache[i].n;
    }
    return ret;
}

static uint32_t filterPreloadLoads(const dynarec_la64_t* dyn, int first, int last, uint32_t preload)
{
    uint16_t pending_xmm = preload;
    uint16_t pending_ymm = preload >> 16;
    uint32_t ret = 0;
    for (int i = first; i <= last && (pending_xmm || pending_ymm); ++i) {
        const lsxcache_t* cache = &dyn->insts[i].lsx;
        uint16_t touched = cache->xmm_used | cache->ymm_used;
        uint16_t first_xmm = pending_xmm & touched;
        uint16_t first_ymm = pending_ymm & touched;
        ret |= first_xmm & cache->xmm_load;
        ret |= (uint32_t)(first_ymm & cache->ymm_load) << 16;
        pending_xmm &= ~touched;
        pending_ymm &= ~touched;
    }
    return ret;
}

static int tryApplyPreloadCache(dynarec_la64_t* dyn, int first, int last, int reg, int ymm)
{
    uint16_t bit = 1u << reg;
    int first_use = -1;
    for (int i = first; i <= last; ++i) {
        if ((dyn->insts[i].lsx.xmm_used | dyn->insts[i].lsx.ymm_used) & bit) {
            first_use = i;
            break;
        }
    }
    if (first_use < 0) return 0;

    for (int i = first; i < first_use; ++i)
        if (dyn->insts[i].lsx.lsxcache[reg].v)
            return 0;

    lsx_cache_t first_cache = dyn->insts[first_use].lsx.lsxcache[reg];
    if (first_cache.n != reg) return 0;
    if (ymm) {
        if (first_cache.t != LSX_CACHE_YMMR && first_cache.t != LSX_CACHE_YMMW)
            return 0;
    } else if (first_cache.t != LSX_CACHE_XMMR && first_cache.t != LSX_CACHE_XMMW) {
        return 0;
    }

    for (int i = first; i < first_use; ++i) {
        lsxcache_t* cache = &dyn->insts[i].lsx;
        cache->lsxcache[reg].t = ymm ? LSX_CACHE_YMMR : LSX_CACHE_XMMR;
        cache->lsxcache[reg].n = reg;
        cache->fpuused[reg] = 1;
        cache->news &= ~(1u << reg);
        if (ymm) {
            cache->ssecache[reg].v = -1;
            cache->avxcache[reg].reg = reg;
            cache->avxcache[reg].upper_zero_pending = 0;
            cache->avxcache[reg].write = 0;
        } else {
            cache->avxcache[reg].v = -1;
            cache->ssecache[reg].reg = reg;
            cache->ssecache[reg].write = 0;
        }
    }
    dyn->insts[first_use].lsx.news &= ~(1u << reg);
    return 1;
}

static void addSSEPreload(dynarec_la64_t* dyn, int last, int first, uint32_t preload)
{
    if (first < 0 || first > last || !preload || dyn->insts[first].preload_xmmymm)
        return;
    if (first && !dyn->insts[first - 1].x64.has_next)
        return;
    for (int i = first; i < last; ++i) {
        const instruction_la64_t* inst = &dyn->insts[i];
        if (!inst->x64.has_next || (inst->x64.barrier & BARRIER_FLOAT)
            || inst->x64.jmp || inst->x64.has_callret || inst->fpupurge || inst->host_call)
            return;
    }

    uint32_t accepted = 0;
    for (int reg = 0; reg < 16; ++reg) {
        if ((preload & (1u << reg)) && tryApplyPreloadCache(dyn, first, last, reg, 0))
            accepted |= 1u << reg;
        if ((preload & (1u << (16 + reg))) && tryApplyPreloadCache(dyn, first, last, reg, 1))
            accepted |= 1u << (16 + reg);
    }
    if (!accepted)
        return;
    dyn->insts[first].preload_xmmymm = accepted;
    dyn->insts[first].preload_from = last;
}

void updatePreloads(dynarec_la64_t* dyn)
{
    if (!dyn->use_xmm && !dyn->use_ymm)
        return;
    for (int first = 0; first < dyn->size; ++first) {
        if ((first && dyn->insts[first].pred_sz <= 1) || (!first && !dyn->insts[first].pred_sz))
            continue;
        int last = dyn->size + 1;
        for (int j = 0; j < dyn->insts[first].pred_sz; ++j) {
            int pred = dyn->insts[first].pred[j];
            if (pred != first - 1 && pred < last)
                last = pred;
        }
        if (last > dyn->size || last <= first)
            continue;
        int prev = getNominalPred(dyn, first);
        if (first && prev != first - 1)
            continue;

        uint32_t preload = getFullXYMMMask(dyn, last);
        uint16_t occupied = getCachedVectorMask(dyn, prev);
        preload &= ~(uint32_t)occupied;
        preload &= ~((uint32_t)occupied << 16);
        preload = filterPreloadLoads(dyn, first, last, preload);
        if (preload)
            addSSEPreload(dyn, last, first, preload);
    }
}

static int fpuCacheTransformStoresXmm(dynarec_la64_t* dyn, int ninst)
{
    const instruction_la64_t* inst = &dyn->insts[ninst];
    if (!inst->x64.jmp)
        return 0;
    int i2 = inst->x64.jmp_insts;
    if (i2 < 0 || i2 >= dyn->size)
        return 0; // out-of-block: the forced full live-out already covers the purge
    if (dyn->insts[i2].x64.barrier & BARRIER_FLOAT)
        return 0; // purge at the barrier target: already fully live there
    lsxcache_t cache_i2 = dyn->insts[i2].lsx;
    lsxcacheUnwind(&cache_i2);
    const lsxcache_t* cache = &inst->lsx;
    for (int i = 0; i < 24; ++i) {
        if (!cache->lsxcache[i].v)
            continue;
        int t = cache->lsxcache[i].t;
        if (t != LSX_CACHE_XMMW && t != LSX_CACHE_YMMW)
            continue;
        if (i2) {
            // transform: the entry is stored (unloaded or refreshed) unless the target holds the identical write
            if (cache_i2.lsxcache[i].v && cache_i2.lsxcache[i].n == cache->lsxcache[i].n && cache_i2.lsxcache[i].t == t)
                continue;
        }
        // i2 == 0: purge, every write is stored
        return 1;
    }
    return 0;
}

void updateUpperLiveness(dynarec_la64_t* dyn)
{
    int n = dyn->size;
    if (n <= 0)
        return;

    size_t live_size = (size_t)n * sizeof(uint64_t);
    size_t pending_size = (size_t)n * sizeof(uint32_t);
    size_t work_size = (size_t)n * sizeof(int);
    size_t list_size = (size_t)n * sizeof(uint8_t);
    void* buffer = calloc(1, 2 * live_size + pending_size + work_size + list_size);
    if (!buffer)
        return;
    uint64_t* live_in = (uint64_t*)buffer;
    uint64_t* live_out = (uint64_t*)((char*)live_in + live_size);
    uint32_t* pending_in = (uint32_t*)((char*)live_out + live_size);
    int* work = (int*)((char*)pending_in + pending_size);
    uint8_t* on_list = (uint8_t*)((char*)work + work_size);

    int sp = 0;
    for (int i = n - 1; i >= 0; --i) {
        if (dyn->insts[i].x64.alive) {
            work[sp++] = i;
            on_list[i] = 1;
        }
    }
    // backward analysis
    while (sp > 0) {
        int i = work[--sp];
        on_list[i] = 0;
        const instruction_la64_t* inst = &dyn->insts[i];
        if (!inst->x64.alive)
            continue;
        uint64_t combined_out = 0;
        if (inst->x64.has_next && i + 1 < n && dyn->insts[i + 1].x64.alive)
            combined_out |= live_in[i + 1];
        if (inst->x64.jmp) {
            if (inst->x64.jmp_insts >= 0 && inst->x64.jmp_insts < n)
                combined_out |= live_in[inst->x64.jmp_insts];
            else
                combined_out = UINT64_MAX;
        }
        int has_internal_jump = inst->x64.jmp && inst->x64.jmp_insts >= 0 && inst->x64.jmp_insts < n;
        if ((inst->x64.has_next && i == n - 1) || (!inst->x64.has_next && !has_internal_jump))
            combined_out = UINT64_MAX;
        live_out[i] = combined_out;

        uint16_t gpr_live_out = (uint16_t)combined_out;
        uint16_t gpr_live_in = inst->up32_read | (gpr_live_out & (uint16_t)~inst->up32_write64);
        vector_upper_t vector_live_in = {0};
        if ((inst->x64.barrier & BARRIER_FLOAT) || inst->x64.has_callret || inst->fpupurge || inst->host_call) {
            vector_live_in.xmm_lane1 = 0xFFFF;
            vector_live_in.xmm_lanes23 = 0xFFFF;
            vector_live_in.ymm_upper = 0xFFFF;
        } else {
            const vector_liveness_t* liveness = &inst->vector_liveness;
            vector_upper_t vector_live_out = {.raw = combined_out >> UPPER_LIVENESS_VECTOR_SHIFT};
            vector_live_out = transferXmmCopyLiveness(vector_live_out, liveness);
            vector_live_in.raw = liveness->use.raw | (vector_live_out.raw & ~liveness->def.raw);
            uint16_t touched = inst->lsx.xmm_used | inst->lsx.ymm_used;
            uint16_t untracked = touched & (uint16_t)~liveness->xmm_tracked;
            vector_live_in.xmm_lane1 |= untracked;
            vector_live_in.xmm_lanes23 |= untracked;
            if (fpuCacheTransformStoresXmm(dyn, i)) {
                vector_live_in.xmm_lane1 = 0xFFFF;
                vector_live_in.xmm_lanes23 = 0xFFFF;
            }
        }
        uint64_t combined_in = gpr_live_in | (vector_live_in.raw << UPPER_LIVENESS_VECTOR_SHIFT);
        if (combined_in != live_in[i]) {
            live_in[i] = combined_in;
            for (int p = 0; p < inst->pred_sz; ++p) {
                int j = inst->pred[p];
                if (!on_list[j]) {
                    work[sp++] = j;
                    on_list[j] = 1;
                }
            }
        }
    }

    for (int i = 0; i < n; ++i) {
        instruction_la64_t* inst = &dyn->insts[i];
        inst->up32_skip = inst->up32_write32 & (uint16_t)~live_out[i];
        inst->vector_liveness.live.raw = live_out[i] >> UPPER_LIVENESS_VECTOR_SHIFT;
    }

    memset(on_list, 0, list_size);
    sp = 0;
    for (int i = 0; i < n; ++i) {
        if (dyn->insts[i].x64.alive) {
            work[sp++] = i;
            on_list[i] = 1;
        }
    }
    // forward analysis

    // This is intentionally a may analysis. At a merge, adjust_arch cannot tell
    // which path supplied a deferred zero, so raw signal state can be ambiguous.
    while (sp > 0) {
        int i = work[--sp];
        on_list[i] = 0;
        instruction_la64_t* inst = &dyn->insts[i];
        if (!inst->x64.alive)
            continue;

        uint32_t merged_pending = 0;
        for (int p = 0; p < inst->pred_sz; ++p) {
            int j = inst->pred[p];
            const instruction_la64_t* pred = &dyn->insts[j];
            uint16_t ymm_write = pred->vector_liveness.def.ymm_upper;
            uint16_t ymm_dead = (uint16_t)~pred->vector_liveness.live.ymm_upper;
            uint32_t kill = pred->up32_write64 | ((uint32_t)ymm_write << UPPER_PENDING_YMM_SHIFT);
            uint32_t deferred_zero = pred->up32_skip | ((uint32_t)(pred->vector_liveness.ymm_zero & ymm_dead) << UPPER_PENDING_YMM_SHIFT);
            merged_pending |= (pending_in[j] & ~kill) | deferred_zero;
        }
        if (merged_pending != pending_in[i]) {
            pending_in[i] = merged_pending;
            if (inst->x64.has_next && i + 1 < n && dyn->insts[i + 1].x64.alive && !on_list[i + 1]) {
                work[sp++] = i + 1;
                on_list[i + 1] = 1;
            }
            if (inst->x64.jmp && inst->x64.jmp_insts >= 0 && inst->x64.jmp_insts < n) {
                int j = inst->x64.jmp_insts;
                if (dyn->insts[j].x64.alive && !on_list[j]) {
                    work[sp++] = j;
                    on_list[j] = 1;
                }
            }
        }
    }
    for (int i = 0; i < n; ++i) {
        dyn->insts[i].up32_pending = (uint16_t)pending_in[i];
        dyn->insts[i].vector_liveness.ymm_pending = (uint16_t)(pending_in[i] >> UPPER_PENDING_YMM_SHIFT);
    }

    free(buffer);
}

void updateRspMerge(dynarec_la64_t* dyn, int is32bits)
{
    const int delta = is32bits ? 4 : 8;
    int pending = 0;
    int last_pushpop = -1;
    for (int i = 0; i < dyn->size; ++i) {
        instruction_la64_t* inst = &dyn->insts[i];
        inst->rsp_entry = 0;
        inst->rsp_flush = 0;
        inst->rsp_merge = 0;
        // Note: trace with rsp merge is allowed on purpose.
        int class = (inst->x64.alive && !BOX64ENV(dynarec_test)) ? inst->rsp_class : RSP_CLASS_BARRIER;
        if (pending && (class == RSP_CLASS_BARRIER || (i == 0) || (inst->pred_sz != 1) || (inst->pred[0] != i - 1))) {
            dyn->insts[last_pushpop].rsp_flush = pending;
            pending = 0;
            last_pushpop = -1;
        }
        switch (class) {
            case RSP_CLASS_PUSH:
                if (pending - delta < -2048) {
                    dyn->insts[last_pushpop].rsp_flush = pending;
                    pending = 0;
                }
                inst->rsp_entry = pending;
                inst->rsp_merge = 1;
                pending -= delta;
                last_pushpop = i;
                break;
            case RSP_CLASS_POP:
                if (pending + delta > 2047) {
                    dyn->insts[last_pushpop].rsp_flush = pending;
                    pending = 0;
                }
                inst->rsp_entry = pending;
                inst->rsp_merge = 1;
                pending += delta;
                last_pushpop = i;
                break;
            default:
                break;
        }
    }
    if (pending) dyn->insts[last_pushpop].rsp_flush = pending;
}
