// SPDX-License-Identifier: MIT
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

int fpu_get_scratch(dynarec_rv64_t* dyn)
{
    if (dyn->e.fpu_scratch < 7) return SCRATCH0 + dyn->e.fpu_scratch++;
    abort();
}

static int rvv_scratch_group_width(int lmul)
{
    if (lmul < VECTOR_LMUL1 || lmul > VECTOR_LMUL8) abort();
    return 1 << lmul;
}

static int rvv_scratch_free(const dynarec_rv64_t* dyn, int base, int width)
{
    uint16_t group = (uint16_t)(((1u << width) - 1) << base);
    return !(dyn->e.vpu_scratch & group);
}

static int rvv_reserve_scratch(dynarec_rv64_t* dyn, int base, int width)
{
    if (base < 1 || base + width > 16 || (base & (width - 1)) || !rvv_scratch_free(dyn, base, width)) abort();
    uint16_t group = (uint16_t)(((1u << width) - 1) << base);
    dyn->e.vpu_scratch |= group;
    return base;
}

int vpu_get_scratch(dynarec_rv64_t* dyn)
{
    for (int reg = 1; reg < 16; ++reg)
        if (rvv_scratch_free(dyn, reg, 1)) return rvv_reserve_scratch(dyn, reg, 1);
    abort();
}

int vpu_get_scratch_lmul(dynarec_rv64_t* dyn, int lmul)
{
    int width = rvv_scratch_group_width(lmul);
    for (int reg = width; reg + width <= 16; reg += width)
        if (rvv_scratch_free(dyn, reg, width)) return rvv_reserve_scratch(dyn, reg, width);
    abort();
}

void fpu_reset_scratch(dynarec_rv64_t* dyn)
{
    dyn->e.fpu_scratch = 0;
    dyn->e.vpu_scratch = 0;
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
void extcache_free_reg(dynarec_rv64_t* dyn, int reg)
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
int simd_alloc_reg(dynarec_rv64_t* dyn, int t, int xmm)
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
    dyn->rvv.bits = 0;
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
    for (int a = 0; a < 16; ++a)
        if (dyn->insts[ninst].e.simd[a].dirty) return 0;
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
    // Check if ninst can be compatible to i2
    if (dyn->e.stack_next != dyn->insts[i2].e.stack - dyn->insts[i2].e.stack_push) {
        return 1;
    }
    extcache_t cache_i2 = dyn->insts[i2].e;
    extcacheUnwind(&cache_i2);

    for (int i = 0; i < 24; ++i) {
        if (i < 16
            && (!dyn->e.extcache[i].v || dyn->e.extcache[i].t == EXT_CACHE_SIMD)
            && (!cache_i2.extcache[i].v || cache_i2.extcache[i].t == EXT_CACHE_SIMD))
            continue;
        if (dyn->e.extcache[i].v) { // there is something at ninst for i
            if (!cache_i2.extcache[i].v) {       // but there is nothing at i2 for i
                ret = 1;
            } else if (dyn->e.extcache[i].v != cache_i2.extcache[i].v) { // there is something different
                if (dyn->e.extcache[i].n != cache_i2.extcache[i].n) {    // not the same x64 reg
                    ret = 1;
                } else
                    ret = 1;
            }
        } else if (cache_i2.extcache[i].v)
            ret = 1;
    }
    for (int a = 0; a < 16; ++a) {
        const simd_cache_t* src = &dyn->e.simd[a];
        const simd_cache_t* dst = &cache_i2.simd[a];
        unsigned kill = (dyn->insts[i2].simd_kill >> (a * 4)) & SIMD_YMM;
        if ((src->dirty & ~kill & ~dst->dirty) || (dst->valid & ~(src->valid & ~kill))) return 1;
    }
    return ret;
}

int rvv_needs_transform(dynarec_rv64_t* dyn, int ninst)
{
    int target = dyn->insts[ninst].x64.jmp_insts;
    return target >= 0 && dyn->insts[target].rvv_entry.vl && dyn->insts[target].rvv_entry.bits != dyn->rvv.bits;
}

void extcacheUnwind(extcache_t* cache)
{
    for (int a = 0; a < 16; ++a)
        if (cache->simd_saved & (1u << a)) cache->simd[a] = cache->simd_old[a];
    cache->simd_saved = 0;
    if (!cache->swapped && !cache->news && !cache->stack_push && !cache->stack_pop) {
        int dirty = 0;
        for (int i = 0; i < 24; ++i)
            if (cache->olds[i].v) {
                dirty = 1;
                break;
            }
        if (!dirty) {
            cache->barrier = 0;
            cache->fpu_scratch = 0;
            cache->vpu_scratch = 0;
            return;
        }
    }
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
            if (cache->news & (1u << i))
                cache->extcache[i].v = 0;
        cache->news = 0;
    }
    // add/change bad regs
    for (int i = 0; i < 24; ++i) {
        if (cache->olds[i].v) {
            cache->extcache[i] = cache->olds[i];
            // the undo log has been applied, don't reapply it on a second unwind
            cache->olds[i].v = 0;
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
    cache->vpu_scratch = 0;
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
                case EXT_CACHE_SIMD:
                    cache->ssecache[cache->extcache[i].n].reg = EXTREG(i);
                    cache->ssecache[cache->extcache[i].n].vector = 1;
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
        case EXT_CACHE_SIMD: sprintf(buff, "SIMD%d", n); break;
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

static const char* df_status[] = { "unknown", "set", "none_pending", "none" };

void printf_x64_instruction(dynarec_native_t* dyn, zydis_dec_t* dec, instruction_x64_t* inst, const char* name);
void inst_name_pass3(dynarec_native_t* dyn, int ninst, const char* name, rex_t rex)
{
    if (dyn->need_dump == 3) {
        printf_x64_instruction(dyn, rex.is32bits ? my_context->dec32 : my_context->dec, &dyn->insts[ninst].x64, name);
        if (!BOX64ENV(dynarec_gdbjit) && !BOX64ENV(dynarec_perf_map)) return;
    }
    if (!dyn->need_dump && !BOX64ENV(dynarec_gdbjit) && !BOX64ENV(dynarec_perf_map)) return;

    static __thread char buf[65536];
    int length = sprintf(buf,
        "barrier=%d state=%d/%s(%s->%s), set=%X/%X, use=%X, need=%X/%X, fuse=%d/%d, sm=%d(%d/%d), "
        "rvv@entry=(e%d,m%d,vl%d), rvv@exit=(e%d,m%d,vl%d)",
        dyn->insts[ninst].x64.barrier, dyn->insts[ninst].x64.state_flags, df_status[dyn->f], df_status[dyn->insts[ninst].f_entry], df_status[dyn->insts[ninst].f_exit],
        dyn->insts[ninst].x64.set_flags, dyn->insts[ninst].x64.gen_flags, dyn->insts[ninst].x64.use_flags, dyn->insts[ninst].x64.need_before, dyn->insts[ninst].x64.need_after,
        dyn->insts[ninst].nat_flags_fusion, dyn->insts[ninst].no_scratch_usage, dyn->smwrite, dyn->insts[ninst].will_write, dyn->insts[ninst].last_write,
        8 << dyn->insts[ninst].rvv_entry.sew, 1 << dyn->insts[ninst].rvv_entry.lmul, dyn->insts[ninst].rvv_entry.vl, 8 << dyn->insts[ninst].rvv_exit.sew,
        1 << dyn->insts[ninst].rvv_exit.lmul, dyn->insts[ninst].rvv_exit.vl);
    if (dyn->insts[ninst].simd_entry)
        length += sprintf(
            buf + length, ", simd@entry=(read=%016llx,carry=%016llx)", (unsigned long long)dyn->insts[ninst].simd_live, (unsigned long long)dyn->insts[ninst].simd_carry);
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
            case EXT_CACHE_SIMD: length += sprintf(buf + length, " v%d:%s", EXTREG(ii), getCacheName(EXT_CACHE_SIMD, dyn->insts[ninst].e.extcache[ii].n)); break;
            case EXT_CACHE_SCR: length += sprintf(buf + length, " f%d:%s", EXTREG(ii), getCacheName(dyn->insts[ninst].e.extcache[ii].t, dyn->insts[ninst].e.extcache[ii].n)); break;
            case EXT_CACHE_NONE:
            default: break;
        }
    }
    for (int a = 0; a < 16; ++a) {
        const simd_cache_t* c = &dyn->insts[ninst].e.simd[a];
        if (c->valid || c->zero || c->dirty)
            length += sprintf(buf + length, " ymm%d[v=%x,z=%x,d=%x]", a, c->valid, c->zero, c->dirty);
    }
    if (dyn->e.stack || dyn->insts[ninst].e.stack_next || dyn->insts[ninst].e.x87stack)
        length += sprintf(buf + length, " X87:%d/%d(+%d/-%d)%d", dyn->e.stack, dyn->insts[ninst].e.stack_next, dyn->insts[ninst].e.stack_push, dyn->insts[ninst].e.stack_pop, dyn->insts[ninst].e.x87stack);
    if (dyn->insts[ninst].e.combined1 || dyn->insts[ninst].e.combined2)
        length += sprintf(buf + length, " %s:%d/%d", dyn->insts[ninst].e.swapped ? "SWP" : "CMB", dyn->insts[ninst].e.combined1, dyn->insts[ninst].e.combined2);

    if (dyn->need_dump && dyn->need_dump != 3) {
        printf_x64_instruction(dyn, rex.is32bits ? my_context->dec32 : my_context->dec, &dyn->insts[ninst].x64, name);
        dynarec_log(LOG_NONE, "%s%p: %d emitted opcodes, inst=%d, %s%s\n",
            (dyn->need_dump > 1) ? "\e[32m" : "",
            (void*)(dyn->native_start + dyn->insts[ninst].address), dyn->insts[ninst].size / 4, ninst, buf, (dyn->need_dump > 1) ? "\e[m" : "");
    }
    if (BOX64ENV(dynarec_gdbjit)) {
        static __thread char buf2[4096 + 64];
        if (BOX64ENV(dynarec_gdbjit) > 1) {
            snprintf(buf2, sizeof(buf2), "; %d: %d opcodes, %.*s", ninst, dyn->insts[ninst].size / 4, (int)sizeof(buf2) - 64, buf);
            GdbJITBlockAddText(dyn->gdbjit_block, buf2);
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
    memset(e->simd, 0, sizeof(e->simd));
    e->simd_saved = 0;
    for (int i = 0; i < 16; ++i)
        e->ssecache[i].v = -1;
}

void fpu_reset(dynarec_rv64_t* dyn)
{
    x87_reset(&dyn->e);
    mmx_reset(&dyn->e);
    sse_reset(&dyn->e);
    fpu_reset_reg(dyn);
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

static int hasLinearPredecessor(const dynarec_rv64_t* dyn, int ninst);

void updateNativeFlags(dynarec_rv64_t* dyn)
{
    if (!BOX64ENV(dynarec_nativeflags))
        return;
    for (int i = 0; i < dyn->size; ++i)
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
                            while (*next)
                                next = &dyn->insts[*next].nat_next_inst;
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
}

void get_free_scratch(dynarec_rv64_t* dyn, int ninst, uint8_t* tmp1, uint8_t* tmp2, uint8_t* tmp3, uint8_t s1, uint8_t s2, uint8_t s3, uint8_t s4, uint8_t s5)
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

void fpu_save_and_unwind(dynarec_rv64_t* dyn, int ninst, extcache_t* cache)
{
    memcpy(cache, &dyn->insts[ninst].e, sizeof(extcache_t));
    extcacheUnwind(&dyn->insts[ninst].e);
}
void fpu_unwind_restore(dynarec_rv64_t* dyn, int ninst, extcache_t* cache)
{
    memcpy(&dyn->insts[ninst].e, cache, sizeof(extcache_t));
}

static int hasLinearPredecessor(const dynarec_rv64_t* dyn, int ninst)
{
    return ninst > 0 && dyn->insts[ninst].pred_sz == 1 && dyn->insts[ninst].pred[0] == ninst - 1;
}

int isUpper32Zero(dynarec_rv64_t* dyn, int ninst, int reg)
{
    if (!IS_GPR(reg))
        return 0;

    const uint16_t bit = (uint16_t)(1 << TO_X64(reg));
    int current = ninst;
    for (int depth = 0; depth < 4 && hasLinearPredecessor(dyn, current); ++depth) {
        const instruction_rv64_t* prev = &dyn->insts[current - 1];
        if (prev->x64.has_callret || prev->x64.barrier)
            return 0;
        if (prev->up32_write64 & bit) return prev->up32_zero & bit;
        --current;
    }
    return 0;
}

static void simd_analyze_zeros(dynarec_rv64_t* dyn)
{
    int n = dyn->size;
    uint64_t* out = malloc(n * sizeof(*out));
    if (!out) {
        for (int i = 0; i < n; ++i)
            dyn->insts[i].simd_zero_in = 0;
        return;
    }
    for (int i = 0; i < n; ++i)
        out[i] = UINT64_MAX;
    int changed;
    do {
        changed = 0;
        for (int i = 0; i < n; ++i) {
            instruction_rv64_t* inst = &dyn->insts[i];
            uint64_t in = i && inst->pred_sz ? UINT64_MAX : 0;
            for (int p = 0; p < inst->pred_sz; ++p)
                in &= out[inst->pred[p]];
            if (inst->x64.barrier & BARRIER_FLOAT) in = 0;
            inst->simd_zero_in = in;
            uint64_t next = (in & ~inst->simd_zero_kill) | inst->simd_zero_gen;
            if (inst->x64.has_callret || inst->simd_opaque) next = 0;
            if (out[i] != next) {
                out[i] = next;
                changed = 1;
            }
        }
    } while (changed);
    free(out);
}

static int simd_is_entry(const dynarec_rv64_t* dyn, int i)
{
    if (!dyn->insts[i].x64.alive) return 0;
    return !i || dyn->insts[i].pred_sz != 1 || dyn->insts[i].pred[0] != i - 1 || dyn->insts[i - 1].x64.jmp;
}

static int simd_boundary(const instruction_rv64_t* inst)
{
    return inst->simd_opaque || (inst->x64.barrier & BARRIER_FLOAT) || inst->x64.has_callret;
}

static int simd_successors(const dynarec_rv64_t* dyn, int i, int successors[2])
{
    const instruction_rv64_t* inst = &dyn->insts[i];
    int count = 0;
    if (inst->x64.has_next && i + 1 < dyn->size && dyn->insts[i + 1].x64.alive) successors[count++] = i + 1;
    int target = inst->x64.jmp_insts;
    if (inst->x64.jmp && target >= 0 && target < dyn->size && dyn->insts[target].x64.alive
        && (!count || successors[0] != target))
        successors[count++] = target;
    return count;
}

typedef struct simd_flow_s {
    uint64_t live;
    uint8_t reachable, loop, queued;
} simd_flow_t;

static void simd_analyze_loop(dynarec_rv64_t* dyn, int header, simd_flow_t* flow, int* work)
{
    instruction_rv64_t* entry = &dyn->insts[header];
    int has_backedge = 0;
    for (int p = 0; p < entry->pred_sz; ++p)
        if (entry->pred[p] >= header && dyn->insts[entry->pred[p]].x64.alive) has_backedge = 1;
    if (!has_backedge) return;

    memset(flow, 0, dyn->size * sizeof(*flow));
    int sp = 0;
    flow[header].reachable = 1;
    work[sp++] = header;
    while (sp) {
        int successors[2];
        int count = simd_successors(dyn, work[--sp], successors);
        for (int s = 0; s < count; ++s) {
            int next = successors[s];
            if (!flow[next].reachable) {
                flow[next].reachable = 1;
                work[sp++] = next;
            }
        }
    }

    flow[header].loop = 1;
    work[sp++] = header;
    has_backedge = 0;
    for (int p = 0; p < entry->pred_sz; ++p) {
        int latch = entry->pred[p];
        if (latch >= header && flow[latch].reachable) {
            has_backedge = 1;
            if (!flow[latch].loop) {
                flow[latch].loop = 1;
                work[sp++] = latch;
            }
        }
    }
    if (!has_backedge) return;
    uint64_t read = 0, written = 0, nonzero = 0;
    while (sp) {
        int i = work[--sp];
        const instruction_rv64_t* inst = &dyn->insts[i];
        if (simd_boundary(inst)) return;
        read |= inst->simd_read;
        written |= inst->simd_defs;
        nonzero |= inst->simd_defs & ~inst->simd_zero_gen;
        if (i == header) continue;
        for (int p = 0; p < inst->pred_sz; ++p) {
            int prev = inst->pred[p];
            if (prev >= header && flow[prev].reachable && !flow[prev].loop) {
                flow[prev].loop = 1;
                work[sp++] = prev;
            }
        }
    }

    for (int i = 0; i < dyn->size; ++i) {
        if (flow[i].loop) {
            flow[i].queued = 1;
            work[sp++] = i;
        }
    }
    while (sp) {
        int i = work[--sp];
        flow[i].queued = 0;
        const instruction_rv64_t* inst = &dyn->insts[i];
        int successors[2];
        int count = simd_successors(dyn, i, successors);
        uint64_t out = 0;
        for (int s = 0; s < count; ++s)
            if (flow[successors[s]].loop) out |= flow[successors[s]].live;
        uint64_t live = inst->simd_read | (out & ~inst->simd_defs);
        if (live == flow[i].live) continue;
        flow[i].live = live;
        for (int p = 0; p < inst->pred_sz; ++p) {
            int prev = inst->pred[p];
            if (flow[prev].loop && !flow[prev].queued) {
                flow[prev].queued = 1;
                work[sp++] = prev;
            }
        }
    }
    entry->simd_live |= flow[header].live;
    entry->simd_carry = written & (nonzero | read);
}

static void simd_analyze_entries(dynarec_rv64_t* dyn)
{
    simd_flow_t* flow = calloc(dyn->size, sizeof(*flow));
    int* work = malloc(dyn->size * sizeof(*work));
    for (int i = 0; i < dyn->size; ++i) {
        instruction_rv64_t* entry = &dyn->insts[i];
        entry->simd_entry = 0;
        entry->simd_live = entry->simd_carry = 0;
        entry->simd_kill = 0;
        entry->rvv_entry_config.bits = 0;
        if (!entry->x64.alive || !simd_is_entry(dyn, i) || (entry->x64.barrier & BARRIER_FLOAT)) continue;
        entry->simd_entry = 1;
        uint64_t read = 0, written = 0;
        for (int j = i; j < dyn->size; ++j) {
            instruction_rv64_t* inst = &dyn->insts[j];
            if (j != i && simd_is_entry(dyn, j)) break;
            if (!inst->x64.alive || simd_boundary(inst)) break;
            read |= inst->simd_read & ~written;
            written |= inst->simd_defs;
            if (!entry->rvv_entry_config.vl && inst->rvv_hint.vl) entry->rvv_entry_config = inst->rvv_hint;
            if (inst->x64.jmp || !inst->x64.has_next) break;
        }
        entry->simd_live = read;
        entry->simd_kill = written & ~read;
        if (flow && work) simd_analyze_loop(dyn, i, flow, work);
        entry->simd_carry &= ~entry->simd_kill;
        if (cpuext.vlen < 32) {
            entry->simd_live &= 0x7777777777777777ULL;
            entry->simd_carry &= 0x7777777777777777ULL;
        }
    }
    free(work);
    free(flow);
}

void updateUpperLiveness(dynarec_rv64_t* dyn)
{
    int n = dyn->size;
    if (n <= 0)
        return;

    simd_analyze_zeros(dyn);
    simd_analyze_entries(dyn);

    size_t live_size = (size_t)n * sizeof(uint16_t);
    size_t pending_size = (size_t)n * sizeof(uint16_t);
    size_t work_size = (size_t)n * sizeof(int);
    size_t list_size = (size_t)n * sizeof(uint8_t);
    void* buffer = calloc(1, 2 * live_size + pending_size + work_size + list_size);
    if (!buffer)
        return;
    uint16_t* live_in = (uint16_t*)buffer;
    uint16_t* live_out = (uint16_t*)((char*)live_in + live_size);
    uint16_t* pending_in = (uint16_t*)((char*)live_out + live_size);
    int* work = (int*)((char*)pending_in + pending_size);
    uint8_t* on_list = (uint8_t*)((char*)work + work_size);

    int sp = 0;
    for (int i = n - 1; i >= 0; --i) {
        if (dyn->insts[i].x64.alive) {
            work[sp++] = i;
            on_list[i] = 1;
        }
    }
    while (sp > 0) {
        int i = work[--sp];
        on_list[i] = 0;
        const instruction_rv64_t* inst = &dyn->insts[i];
        if (!inst->x64.alive)
            continue;
        uint16_t combined_out = 0;
        if (inst->x64.has_next && i + 1 < n && dyn->insts[i + 1].x64.alive)
            combined_out |= live_in[i + 1];
        if (inst->x64.jmp) {
            if (inst->x64.jmp_insts >= 0 && inst->x64.jmp_insts < n)
                combined_out |= live_in[inst->x64.jmp_insts];
            else
                combined_out = UINT16_MAX;
        }
        int has_internal_jump = inst->x64.jmp && inst->x64.jmp_insts >= 0 && inst->x64.jmp_insts < n;
        if ((inst->x64.has_next && i == n - 1) || (!inst->x64.has_next && !has_internal_jump))
            combined_out = UINT16_MAX;
        live_out[i] = combined_out;


        uint16_t live_in_val = inst->up32_read | (combined_out & (uint16_t)~inst->up32_write64);
        if (live_in_val != live_in[i]) {
            live_in[i] = live_in_val;
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
        instruction_rv64_t* inst = &dyn->insts[i];
        inst->up32_skip = inst->up32_write32 & (uint16_t)~live_out[i];
    }

    memset(pending_in, 0, pending_size);
    memset(on_list, 0, list_size);
    sp = 0;
    for (int i = 0; i < n; ++i) {
        if (dyn->insts[i].x64.alive) {
            work[sp++] = i;
            on_list[i] = 1;
        }
    }
    while (sp > 0) {
        int i = work[--sp];
        on_list[i] = 0;
        instruction_rv64_t* inst = &dyn->insts[i];
        if (!inst->x64.alive)
            continue;

        uint16_t merged_pending = 0;
        for (int p = 0; p < inst->pred_sz; ++p) {
            int j = inst->pred[p];
            const instruction_rv64_t* pred = &dyn->insts[j];
            merged_pending |= (pending_in[j] & (uint16_t)~pred->up32_write64) | pred->up32_skip;
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
        dyn->insts[i].up32_pending = pending_in[i];
    }

    free(buffer);
}

void updateRspMerge(dynarec_rv64_t* dyn, int is32bits)
{
    const int delta = is32bits ? 4 : 8;
    int pending = 0;
    int last_pushpop = -1;
    for (int i = 0; i < dyn->size; ++i) {
        instruction_rv64_t* inst = &dyn->insts[i];
        inst->rsp_entry = 0;
        inst->rsp_flush = 0;
        inst->rsp_merge = 0;
        int class = (inst->x64.alive && !BOX64ENV(dynarec_test)) ? inst->rsp_class : RSP_CLASS_BARRIER;
        if (pending && (class == RSP_CLASS_BARRIER || (i == 0) || (inst->pred_sz != 1) || (inst->pred[0] != i - 1))) {
            dyn->insts[last_pushpop].rsp_flush = pending;
            pending = 0;
            last_pushpop = -1;
        }
        switch (class) {
            case RSP_CLASS_PUSH:
                if (pending - (inst->rsp_span ? inst->rsp_span : delta) < -2048) {
                    dyn->insts[last_pushpop].rsp_flush = pending;
                    pending = 0;
                }
                inst->rsp_entry = pending;
                inst->rsp_merge = 1;
                pending -= (inst->rsp_span ? inst->rsp_span : delta);
                last_pushpop = i;
                break;
            case RSP_CLASS_POP:
                if (pending + (inst->rsp_span ? inst->rsp_span : delta) > 2047) {
                    dyn->insts[last_pushpop].rsp_flush = pending;
                    pending = 0;
                }
                inst->rsp_entry = pending;
                inst->rsp_merge = 1;
                pending += (inst->rsp_span ? inst->rsp_span : delta);
                last_pushpop = i;
                break;
            default:
                break;
        }
    }
    if (pending) dyn->insts[last_pushpop].rsp_flush = pending;
}
