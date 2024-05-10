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
#include "dynarec.h"
#include "emu/x64emu_private.h"
#include "x64run.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "emu/x87emu_private.h"
#include "x64trace.h"
#include "signals.h"
#include "dynarec_native.h"
#include "dynarec_la64_private.h"
#include "dynarec_la64_functions.h"
#include "custommem.h"
#include "bridge.h"

#define XMM0 0
#define XMM8 16
#define X870 8
#define EMM0 8

// Get a FPU scratch reg
int fpu_get_scratch(dynarec_la64_t* dyn)
{
    return SCRATCH0 + dyn->lsx.fpu_scratch++; // return an Sx
}
// Reset scratch regs counter
void fpu_reset_scratch(dynarec_la64_t* dyn)
{
    dyn->lsx.fpu_scratch = 0;
}

// Free a FPU double reg
void fpu_free_reg(dynarec_la64_t* dyn, int reg)
{
    // TODO: check upper limit?
    dyn->lsx.fpuused[reg] = 0;
    if (dyn->lsx.lsxcache[reg].t != LSX_CACHE_ST_F && dyn->lsx.lsxcache[reg].t != LSX_CACHE_ST_D && dyn->lsx.lsxcache[reg].t != LSX_CACHE_ST_I64)
        dyn->lsx.lsxcache[reg].v = 0;
}

// Get an XMM quad reg
int fpu_get_reg_xmm(dynarec_la64_t* dyn, int t, int xmm)
{
    int i;
    if (xmm > 7) {
        i = XMM8 + xmm - 8;
    } else {
        i = XMM0 + xmm;
    }
    dyn->lsx.fpuused[i] = 1;
    dyn->lsx.lsxcache[i].t = t;
    dyn->lsx.lsxcache[i].n = xmm;
    dyn->lsx.news |= (1 << i);
    return i;
}

// Reset fpu regs counter
static void fpu_reset_reg_lsxcache(lsxcache_t* lsx)
{
    lsx->fpu_reg = 0;
    for (int i = 0; i < 24; ++i) {
        lsx->fpuused[i] = 0;
        lsx->lsxcache[i].v = 0;
    }
}

void fpu_reset_reg(dynarec_la64_t* dyn)
{
    fpu_reset_reg_lsxcache(&dyn->lsx);
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
    if (i2 < 0)
        return 1;
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
        cache->stack -= cache->stack_push;
        cache->stack_push = 0;
    }
    cache->x87stack += cache->stack_pop;
    cache->stack_next = cache->stack;
    cache->stack_pop = 0;
    cache->barrier = 0;
    // And now, rebuild the x87cache info with lsxcache
    cache->mmxcount = 0;
    cache->fpu_scratch = 0;
    cache->fpu_extra_qscratch = 0;
    cache->fpu_reg = 0;
    for (int i = 0; i < 8; ++i) {
        cache->x87cache[i] = -1;
        cache->mmxcache[i] = -1;
        cache->x87reg[i] = 0;
        cache->ssecache[i * 2].v = -1;
        cache->ssecache[i * 2 + 1].v = -1;
    }
    int x87reg = 0;
    for (int i = 0; i < 24; ++i) {
        if (cache->lsxcache[i].v) {
            cache->fpuused[i] = 1;
            switch (cache->lsxcache[i].t) {
                case LSX_CACHE_MM:
                    cache->mmxcache[cache->lsxcache[i].n] = i;
                    ++cache->mmxcount;
                    ++cache->fpu_reg;
                    break;
                case LSX_CACHE_XMMR:
                case LSX_CACHE_XMMW:
                    cache->ssecache[cache->lsxcache[i].n].reg = i;
                    cache->ssecache[cache->lsxcache[i].n].write = (cache->lsxcache[i].t == LSX_CACHE_XMMW) ? 1 : 0;
                    ++cache->fpu_reg;
                    break;
                case LSX_CACHE_ST_F:
                case LSX_CACHE_ST_D:
                case LSX_CACHE_ST_I64:
                    cache->x87cache[x87reg] = cache->lsxcache[i].n;
                    cache->x87reg[x87reg] = i;
                    ++x87reg;
                    ++cache->fpu_reg;
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
        case LSX_CACHE_SCR: sprintf(buff, "Scratch"); break;
        case LSX_CACHE_NONE: buff[0] = '\0'; break;
    }
    return buff;
}

void inst_name_pass3(dynarec_native_t* dyn, int ninst, const char* name, rex_t rex)
{
    if (box64_dynarec_dump) {
        printf_x64_instruction(rex.is32bits ? my_context->dec32 : my_context->dec, &dyn->insts[ninst].x64, name);
        dynarec_log(LOG_NONE, "%s%p: %d emitted opcodes, inst=%d, barrier=%d state=%d/%d(%d), %s=%X/%X, use=%X, need=%X/%X, sm=%d/%d",
            (box64_dynarec_dump > 1) ? "\e[32m" : "",
            (void*)(dyn->native_start + dyn->insts[ninst].address),
            dyn->insts[ninst].size / 4,
            ninst,
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
            dyn->smread, dyn->smwrite);
        if (dyn->insts[ninst].pred_sz) {
            dynarec_log(LOG_NONE, ", pred=");
            for (int ii = 0; ii < dyn->insts[ninst].pred_sz; ++ii)
                dynarec_log(LOG_NONE, "%s%d", ii ? "/" : "", dyn->insts[ninst].pred[ii]);
        }
        if (dyn->insts[ninst].x64.jmp && dyn->insts[ninst].x64.jmp_insts >= 0)
            dynarec_log(LOG_NONE, ", jmp=%d", dyn->insts[ninst].x64.jmp_insts);
        if (dyn->insts[ninst].x64.jmp && dyn->insts[ninst].x64.jmp_insts == -1)
            dynarec_log(LOG_NONE, ", jmp=out");
        if (dyn->last_ip)
            dynarec_log(LOG_NONE, ", last_ip=%p", (void*)dyn->last_ip);
        for (int ii = 0; ii < 24; ++ii) {
            switch (dyn->insts[ninst].lsx.lsxcache[ii].t) {
                case LSX_CACHE_ST_D: dynarec_log(LOG_NONE, " D%d:%s", ii, getCacheName(dyn->insts[ninst].lsx.lsxcache[ii].t, dyn->insts[ninst].lsx.lsxcache[ii].n)); break;
                case LSX_CACHE_ST_F: dynarec_log(LOG_NONE, " S%d:%s", ii, getCacheName(dyn->insts[ninst].lsx.lsxcache[ii].t, dyn->insts[ninst].lsx.lsxcache[ii].n)); break;
                case LSX_CACHE_ST_I64: dynarec_log(LOG_NONE, " D%d:%s", ii, getCacheName(dyn->insts[ninst].lsx.lsxcache[ii].t, dyn->insts[ninst].lsx.lsxcache[ii].n)); break;
                case LSX_CACHE_MM: dynarec_log(LOG_NONE, " D%d:%s", ii, getCacheName(dyn->insts[ninst].lsx.lsxcache[ii].t, dyn->insts[ninst].lsx.lsxcache[ii].n)); break;
                case LSX_CACHE_XMMW: dynarec_log(LOG_NONE, " Q%d:%s", ii, getCacheName(dyn->insts[ninst].lsx.lsxcache[ii].t, dyn->insts[ninst].lsx.lsxcache[ii].n)); break;
                case LSX_CACHE_XMMR: dynarec_log(LOG_NONE, " Q%d:%s", ii, getCacheName(dyn->insts[ninst].lsx.lsxcache[ii].t, dyn->insts[ninst].lsx.lsxcache[ii].n)); break;
                case LSX_CACHE_SCR: dynarec_log(LOG_NONE, " D%d:%s", ii, getCacheName(dyn->insts[ninst].lsx.lsxcache[ii].t, dyn->insts[ninst].lsx.lsxcache[ii].n)); break;
                case LSX_CACHE_NONE:
                default: break;
            }
        }
        if (dyn->lsx.stack || dyn->insts[ninst].lsx.stack_next || dyn->insts[ninst].lsx.x87stack)
            dynarec_log(LOG_NONE, " X87:%d/%d(+%d/-%d)%d", dyn->lsx.stack, dyn->insts[ninst].lsx.stack_next, dyn->insts[ninst].lsx.stack_push, dyn->insts[ninst].lsx.stack_pop, dyn->insts[ninst].lsx.x87stack);
        if (dyn->insts[ninst].lsx.combined1 || dyn->insts[ninst].lsx.combined2)
            dynarec_log(LOG_NONE, " %s:%d/%d", dyn->insts[ninst].lsx.swapped ? "SWP" : "CMB", dyn->insts[ninst].lsx.combined1, dyn->insts[ninst].lsx.combined2);
        dynarec_log(LOG_NONE, "%s\n", (box64_dynarec_dump > 1) ? "\e[m" : "");
    }
}

// will go badly if address is unaligned
static uint8_t extract_byte(uint32_t val, void* address)
{
    int idx = (((uintptr_t)address)&3)*8;
    return (val>>idx)&0xff;
}

static uint32_t insert_byte(uint32_t val, uint8_t b, void* address)
{
    int idx = (((uintptr_t)address)&3)*8;
    val&=~(0xff<<idx);
    val|=(((uint32_t)b)<<idx);
    return val;
}

static uint16_t extract_half(uint32_t val, void* address)
{
    int idx = (((uintptr_t)address)&3)*8;
    return (val>>idx)&0xffff;
}

static uint32_t insert_half(uint32_t val, uint16_t h, void* address)
{
    int idx = (((uintptr_t)address)&3)*8;
    val&=~(0xffff<<idx);
    val|=(((uint32_t)h)<<idx);
    return val;
}

uint8_t la64_lock_xchg_b_slow(void* addr, uint8_t val)
{
    uint32_t ret;
    uint32_t* aligned = (uint32_t*)(((uintptr_t)addr)&~3);
    do {
        ret = *aligned;
    } while(la64_lock_cas_d(aligned, ret, insert_byte(ret, val, addr)));
    return extract_byte(ret, addr);
}

int la64_lock_cas_b_slow(void* addr, uint8_t ref, uint8_t val)
{
    uint32_t* aligned = (uint32_t*)(((uintptr_t)addr)&~3);
    uint32_t tmp = *aligned;
    return la64_lock_cas_d(aligned, ref, insert_byte(tmp, val, addr));
}

int la64_lock_cas_h_slow(void* addr, uint16_t ref, uint16_t val)
{
    uint32_t* aligned = (uint32_t*)(((uintptr_t)addr)&~3);
    uint32_t tmp = *aligned;
    return la64_lock_cas_d(aligned, ref, insert_half(tmp, val, addr));
}

void print_opcode(dynarec_native_t* dyn, int ninst, uint32_t opcode)
{
    dynarec_log(LOG_NONE, "\t%08x\t%s\n", opcode, la64_print(opcode, (uintptr_t)dyn->block));
}

static void sse_reset(lsxcache_t* lsx)
{
    for (int i = 0; i < 16; ++i)
        lsx->ssecache[i].v = -1;
}

void fpu_reset(dynarec_la64_t* dyn)
{
    // TODO: x87 and mmx
    sse_reset(&dyn->lsx);
    fpu_reset_reg(dyn);
}

void fpu_reset_ninst(dynarec_la64_t* dyn, int ninst)
{
    // TODO: x87 and mmx
    sse_reset(&dyn->insts[ninst].lsx);
    fpu_reset_reg_lsxcache(&dyn->insts[ninst].lsx);
}