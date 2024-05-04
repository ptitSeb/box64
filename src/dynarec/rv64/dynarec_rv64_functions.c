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
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_functions.h"
#include "custommem.h"
#include "bridge.h"
#include "rv64_lock.h"

#define XMM0    0
#define X870    XMM0+16
#define EMM0    XMM0+16

// Get a FPU scratch reg
int fpu_get_scratch(dynarec_rv64_t* dyn)
{
    return SCRATCH0 + dyn->e.fpu_scratch++;  // return an Sx
}
// Reset scratch regs counter
void fpu_reset_scratch(dynarec_rv64_t* dyn)
{
    dyn->e.fpu_scratch = 0;
}
// Get a x87 double reg
int fpu_get_reg_x87(dynarec_rv64_t* dyn, int t, int n)
{
    int i=X870;
    while (dyn->e.fpuused[i]) ++i;
    dyn->e.fpuused[i] = 1;
    dyn->e.extcache[i].n = n;
    dyn->e.extcache[i].t = t;
    dyn->e.news |= (1<<i);
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
// Get an MMX double reg
int fpu_get_reg_emm(dynarec_rv64_t* dyn, int emm)
{
    dyn->e.fpuused[EMM0 + emm] = 1;
    dyn->e.extcache[EMM0 + emm].t = EXT_CACHE_MM;
    dyn->e.extcache[EMM0 + emm].n = emm;
    dyn->e.news |= (1<<(EMM0 + emm));
    return EXTREG(EMM0 + emm);
}
// Get an XMM reg
int fpu_get_reg_xmm(dynarec_rv64_t* dyn, int t, int xmm)
{
    int i = XMM0+xmm;
    dyn->e.fpuused[i] = 1;
    dyn->e.extcache[i].t = t;
    dyn->e.extcache[i].n = xmm;
    dyn->e.news |= (1<<i);
    return EXTREG(i);
}
// Reset fpu regs counter
void fpu_reset_reg_extcache(extcache_t* e)
{
    e->fpu_reg = 0;
    for (int i=0; i<24; ++i) {
        e->fpuused[i]=0;
        e->extcache[i].v = 0;
    }
}
void fpu_reset_reg(dynarec_rv64_t* dyn)
{
    fpu_reset_reg_extcache(&dyn->e);
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
    for(int i=0; i<24; ++i)
        if(dyn->insts[ninst].e.extcache[i].t==EXT_CACHE_ST_F
         && dyn->insts[ninst].e.extcache[i].n==a)
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
    for(int i=0; i<24; ++i)
        if(dyn->insts[ninst].e.extcache[i].t==EXT_CACHE_ST_F
         && dyn->insts[ninst].e.extcache[i].n==a)
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
    for(int i=0; i<24; ++i)
        if(dyn->e.extcache[i].t==EXT_CACHE_ST_F
         && dyn->e.extcache[i].n==a)
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
    if(a == dyn->insts[ninst].e.combined1 || a == dyn->insts[ninst].e.combined2) {
        if(a == dyn->insts[ninst].e.combined1) {
            a = dyn->insts[ninst].e.combined2;
        } else
            a = dyn->insts[ninst].e.combined1;
        int i = extcache_get_st_f_i64_noback(dyn, ninst, a);
        //if(box64_dynarec_dump) dynarec_log(LOG_NONE, "extcache_promote_double_combined, ninst=%d combined%c %d i=%d (stack:%d/%d)\n", ninst, (a == dyn->insts[ninst].e.combined2)?'2':'1', a ,i, dyn->insts[ninst].e.stack_push, -dyn->insts[ninst].e.stack_pop);
        if(i>=0) {
            dyn->insts[ninst].e.extcache[i].t = EXT_CACHE_ST_D;
            if(!dyn->insts[ninst].e.barrier)
                extcache_promote_double_internal(dyn, ninst-1, maxinst, a-dyn->insts[ninst].e.stack_push);
            // go forward is combined is not pop'd
            if(a-dyn->insts[ninst].e.stack_pop>=0)
                if(!dyn->insts[ninst+1].e.barrier)
                    extcache_promote_double_forward(dyn, ninst+1, maxinst, a-dyn->insts[ninst].e.stack_pop);
        }
    }
}
static void extcache_promote_double_internal(dynarec_rv64_t* dyn, int ninst, int maxinst, int a)
{
    if(dyn->insts[ninst+1].e.barrier)
        return;
    while(ninst>=0) {
        a+=dyn->insts[ninst].e.stack_pop;    // adjust Stack depth: add pop'd ST (going backward)
        int i = extcache_get_st_f_i64(dyn, ninst, a);
        //if(box64_dynarec_dump) dynarec_log(LOG_NONE, "extcache_promote_double_internal, ninst=%d, a=%d st=%d:%d, i=%d\n", ninst, a, dyn->insts[ninst].e.stack, dyn->insts[ninst].e.stack_next, i);
        if(i<0) return;
        dyn->insts[ninst].e.extcache[i].t = EXT_CACHE_ST_D;
        // check combined propagation too
        if(dyn->insts[ninst].e.combined1 || dyn->insts[ninst].e.combined2) {
            if(dyn->insts[ninst].e.swapped) {
                //if(box64_dynarec_dump) dynarec_log(LOG_NONE, "extcache_promote_double_internal, ninst=%d swapped %d/%d vs %d with st %d\n", ninst, dyn->insts[ninst].e.combined1 ,dyn->insts[ninst].e.combined2, a, dyn->insts[ninst].e.stack);
                if (a==dyn->insts[ninst].e.combined1)
                    a = dyn->insts[ninst].e.combined2;
                else if (a==dyn->insts[ninst].e.combined2)
                    a = dyn->insts[ninst].e.combined1;
            } else {
                //if(box64_dynarec_dump) dynarec_log(LOG_NONE, "extcache_promote_double_internal, ninst=%d combined %d/%d vs %d with st %d\n", ninst, dyn->insts[ninst].e.combined1 ,dyn->insts[ninst].e.combined2, a, dyn->insts[ninst].e.stack);
                extcache_promote_double_combined(dyn, ninst, maxinst, a);
            }
        }
        a-=dyn->insts[ninst].e.stack_push;  // // adjust Stack depth: remove push'd ST (going backward)
        --ninst;
        if(ninst<0 || a<0 || dyn->insts[ninst].e.barrier)
            return;
    }
}

static void extcache_promote_double_forward(dynarec_rv64_t* dyn, int ninst, int maxinst, int a)
{
    while((ninst!=-1) && (ninst<maxinst) && (a>=0)) {
        a+=dyn->insts[ninst].e.stack_push;  // // adjust Stack depth: add push'd ST (going forward)
        if((dyn->insts[ninst].e.combined1 || dyn->insts[ninst].e.combined2) && dyn->insts[ninst].e.swapped) {
            //if(box64_dynarec_dump) dynarec_log(LOG_NONE, "extcache_promote_double_forward, ninst=%d swapped %d/%d vs %d with st %d\n", ninst, dyn->insts[ninst].e.combined1 ,dyn->insts[ninst].e.combined2, a, dyn->insts[ninst].e.stack);
            if (a==dyn->insts[ninst].e.combined1)
                a = dyn->insts[ninst].e.combined2;
            else if (a==dyn->insts[ninst].e.combined2)
                a = dyn->insts[ninst].e.combined1;
        }
        int i = extcache_get_st_f_i64_noback(dyn, ninst, a);
        //if(box64_dynarec_dump) dynarec_log(LOG_NONE, "extcache_promote_double_forward, ninst=%d, a=%d st=%d:%d(%d/%d), i=%d\n", ninst, a, dyn->insts[ninst].e.stack, dyn->insts[ninst].e.stack_next, dyn->insts[ninst].e.stack_push, -dyn->insts[ninst].e.stack_pop, i);
        if(i<0) return;
        dyn->insts[ninst].e.extcache[i].t = EXT_CACHE_ST_D;
        // check combined propagation too
        if((dyn->insts[ninst].e.combined1 || dyn->insts[ninst].e.combined2) && !dyn->insts[ninst].e.swapped) {
            //if(box64_dynarec_dump) dynarec_log(LOG_NONE, "extcache_promote_double_forward, ninst=%d combined %d/%d vs %d with st %d\n", ninst, dyn->insts[ninst].e.combined1 ,dyn->insts[ninst].e.combined2, a, dyn->insts[ninst].e.stack);
            extcache_promote_double_combined(dyn, ninst, maxinst, a);
        }
        a-=dyn->insts[ninst].e.stack_pop;    // adjust Stack depth: remove pop'd ST (going forward)
        if(dyn->insts[ninst].x64.has_next && !dyn->insts[ninst].e.barrier)
            ++ninst;
        else
            ninst=-1;
    }
    if(ninst==maxinst)
        extcache_promote_double(dyn, ninst, a);
}

void extcache_promote_double(dynarec_rv64_t* dyn, int ninst, int a)
{
    int i = extcache_get_current_st_f_i64(dyn, a);
    //if(box64_dynarec_dump) dynarec_log(LOG_NONE, "extcache_promote_double, ninst=%d a=%d st=%d i=%d\n", ninst, a, dyn->e.stack, i);
    if(i<0) return;
    dyn->e.extcache[i].t = EXT_CACHE_ST_D;
    dyn->insts[ninst].e.extcache[i].t = EXT_CACHE_ST_D;
    // check combined propagation too
    if(dyn->e.combined1 || dyn->e.combined2) {
        if(dyn->e.swapped) {
            //if(box64_dynarec_dump) dynarec_log(LOG_NONE, "extcache_promote_double, ninst=%d swapped! %d/%d vs %d\n", ninst, dyn->e.combined1 ,dyn->e.combined2, a);
            if(dyn->e.combined1 == a)
                a = dyn->e.combined2;
            else if(dyn->e.combined2 == a)
                a = dyn->e.combined1;
        } else {
            //if(box64_dynarec_dump) dynarec_log(LOG_NONE, "extcache_promote_double, ninst=%d combined! %d/%d vs %d\n", ninst, dyn->e.combined1 ,dyn->e.combined2, a);
            if(dyn->e.combined1 == a)
                extcache_promote_double(dyn, ninst, dyn->e.combined2);
            else if(dyn->e.combined2 == a)
                extcache_promote_double(dyn, ninst, dyn->e.combined1);
        }
    }
    a-=dyn->insts[ninst].e.stack_push;  // // adjust Stack depth: remove push'd ST (going backward)
    if(!ninst || a<0) return;
    extcache_promote_double_internal(dyn, ninst-1, ninst, a);
}

int extcache_combine_st(dynarec_rv64_t* dyn, int ninst, int a, int b)
{
    dyn->e.combined1=a;
    dyn->e.combined2=b;
    if( extcache_get_current_st(dyn, ninst, a)==EXT_CACHE_ST_F
     && extcache_get_current_st(dyn, ninst, b)==EXT_CACHE_ST_F )
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

int fpuCacheNeedsTransform(dynarec_rv64_t* dyn, int ninst) {
    int i2 = dyn->insts[ninst].x64.jmp_insts;
    if(i2<0)
        return 1;
    if((dyn->insts[i2].x64.barrier&BARRIER_FLOAT))
        // if the barrier as already been apply, no transform needed
        return ((dyn->insts[ninst].x64.barrier&BARRIER_FLOAT))?0:(isCacheEmpty(dyn, ninst)?0:1);
    int ret = 0;
    if(!i2) { // just purge
        if(dyn->insts[ninst].e.stack_next) {
            return 1;
        }
        for(int i=0; i<24 && !ret; ++i)
            if(dyn->insts[ninst].e.extcache[i].v) {       // there is something at ninst for i
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
    if(dyn->insts[ninst].e.stack_next != dyn->insts[i2].e.stack-dyn->insts[i2].e.stack_push) {
        return 1;
    }
    extcache_t cache_i2 = dyn->insts[i2].e;
    extcacheUnwind(&cache_i2);

    for(int i=0; i<24; ++i) {
        if(dyn->insts[ninst].e.extcache[i].v) {       // there is something at ninst for i
            if(!cache_i2.extcache[i].v) {    // but there is nothing at i2 for i
                ret = 1;
            } else if(dyn->insts[ninst].e.extcache[i].v!=cache_i2.extcache[i].v) {  // there is something different
                ret = 1;
            }
        } else if(cache_i2.extcache[i].v)
            ret = 1;
    }
    return ret;
}

void extcacheUnwind(extcache_t* cache)
{
    if(cache->swapped) {
        // unswap
        int a = -1;
        int b = -1;
        for(int j=0; j<24 && ((a==-1) || (b==-1)); ++j)
            if ((cache->extcache[j].t == EXT_CACHE_ST_D
                    || cache->extcache[j].t == EXT_CACHE_ST_F
                    || cache->extcache[j].t == EXT_CACHE_ST_I64)) {
                if(cache->extcache[j].n == cache->combined1)
                    a = j;
                else if(cache->extcache[j].n == cache->combined2)
                    b = j;
            }
        if(a!=-1 && b!=-1) {
            int tmp = cache->extcache[a].n;
            cache->extcache[a].n = cache->extcache[b].n;
            cache->extcache[b].n = tmp;
        }
        cache->swapped = 0;
        cache->combined1 = cache->combined2 = 0;
    }
    if(cache->news) {
        // remove the newly created extcache
        for(int i=0; i<24; ++i)
            if(cache->news&(1<<i))
                cache->extcache[i].v = 0;
        cache->news = 0;
    }
    // add/change bad regs
    for(int i=0; i<16; ++i) {
        if(cache->olds[i].changed) {
            cache->extcache[i].t = cache->olds[i].single?EXT_CACHE_SS:EXT_CACHE_SD;
        } else if(cache->olds[i].purged) {
            cache->extcache[i].n = i;
            cache->extcache[i].t = cache->olds[i].single?EXT_CACHE_SS:EXT_CACHE_SD;
        }
    }
    if(cache->stack_push) {
        // unpush
        for(int j=0; j<24; ++j) {
            if ((cache->extcache[j].t == EXT_CACHE_ST_D
                    || cache->extcache[j].t == EXT_CACHE_ST_F
                    || cache->extcache[j].t == EXT_CACHE_ST_I64)) {
                if(cache->extcache[j].n<cache->stack_push)
                    cache->extcache[j].v = 0;
                else
                    cache->extcache[j].n-=cache->stack_push;
            }
        }
        cache->x87stack-=cache->stack_push;
        cache->tags>>=(cache->stack_push*2);
        cache->stack-=cache->stack_push;
        if(cache->pushed>=cache->stack_push)
            cache->pushed-=cache->stack_push;
        else
            cache->pushed = 0;
        cache->stack_push = 0;
    }
    cache->x87stack+=cache->stack_pop;
    cache->stack_next = cache->stack;
    if(cache->stack_pop) {
        if(cache->poped>=cache->stack_pop)
            cache->poped-=cache->stack_pop;
        else
            cache->poped = 0;
        cache->tags<<=(cache->stack_pop*2);
    }
    cache->stack_pop = 0;
    cache->barrier = 0;
    // And now, rebuild the x87cache info with extcache
    cache->mmxcount = 0;
    cache->fpu_scratch = 0;
    cache->fpu_extra_qscratch = 0;
    cache->fpu_reg = 0;
    for(int i=0; i<8; ++i) {
        cache->x87cache[i] = -1;
        cache->mmxcache[i] = -1;
        cache->x87reg[i] = 0;
        cache->ssecache[i*2].v = -1;
        cache->ssecache[i*2+1].v = -1;
    }
    int x87reg = 0;
    for(int i=0; i<24; ++i) {
        if(cache->extcache[i].v) {
            cache->fpuused[i] = 1;
            switch (cache->extcache[i].t) {
                case EXT_CACHE_MM:
                    cache->mmxcache[cache->extcache[i].n] = EXTREG(i);
                    ++cache->mmxcount;
                    ++cache->fpu_reg;
                    break;
                case EXT_CACHE_SS:
                    cache->ssecache[cache->extcache[i].n].reg = EXTREG(i);
                    cache->ssecache[cache->extcache[i].n].single = 1;
                    ++cache->fpu_reg;
                    break;
                case EXT_CACHE_SD:
                    cache->ssecache[cache->extcache[i].n].reg = EXTREG(i);
                    cache->ssecache[cache->extcache[i].n].single = 0;
                    ++cache->fpu_reg;
                    break;
                case EXT_CACHE_ST_F:
                case EXT_CACHE_ST_D:
                case EXT_CACHE_ST_I64:
                    cache->x87cache[x87reg] = cache->extcache[i].n;
                    cache->x87reg[x87reg] = EXTREG(i);
                    ++x87reg;
                    ++cache->fpu_reg;
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

uint8_t rv64_lock_xchg_b(void* addr, uint8_t val)
{
    uint32_t ret;
    uint32_t* aligned = (uint32_t*)(((uintptr_t)addr)&~3);
    do {
        ret = *aligned;
    } while(rv64_lock_cas_d(aligned, ret, insert_byte(ret, val, addr)));
    return extract_byte(ret, addr);
}

int rv64_lock_cas_b(void* addr, uint8_t ref, uint8_t val)
{
    uint32_t* aligned = (uint32_t*)(((uintptr_t)addr)&~3);
    uint32_t tmp = *aligned;
    return rv64_lock_cas_d(aligned, tmp, insert_byte(tmp, val, addr));
}

int rv64_lock_cas_h(void* addr, uint16_t ref, uint16_t val)
{
    uint32_t* aligned = (uint32_t*)(((uintptr_t)addr)&~3);
    uint32_t tmp = *aligned;
    return rv64_lock_cas_d(aligned, tmp, insert_half(tmp, val, addr));
}


const char* getCacheName(int t, int n)
{
    static char buff[20];
    switch(t) {
        case EXT_CACHE_ST_D: sprintf(buff, "ST%d", n); break;
        case EXT_CACHE_ST_F: sprintf(buff, "st%d", n); break;
        case EXT_CACHE_ST_I64: sprintf(buff, "STi%d", n); break;
        case EXT_CACHE_MM: sprintf(buff, "MM%d", n); break;
        case EXT_CACHE_SS: sprintf(buff, "SS%d", n); break;
        case EXT_CACHE_SD: sprintf(buff, "SD%d", n); break;
        case EXT_CACHE_SCR: sprintf(buff, "Scratch"); break;
        case EXT_CACHE_NONE: buff[0]='\0'; break;
    }
    return buff;
}

void inst_name_pass3(dynarec_native_t* dyn, int ninst, const char* name, rex_t rex)
{
    static const char* fnames[] = {
        "ft0", "ft1", "ft2", "ft3", "ft4", "ft5", "ft6", "ft7",
        "fs0", "fs1",
        "fa0", "fa1", "fa2", "fa3", "fa4", "fa5", "fa6", "fa7",
        "fs2", "fs3", "fs4", "fs5", "fs6", "fs7", "fs8", "fs9", "fs10", "fs11",
        "ft8", "ft9", "ft10", "ft11"
    };
    if(box64_dynarec_dump) {
        printf_x64_instruction(rex.is32bits?my_context->dec32:my_context->dec, &dyn->insts[ninst].x64, name);
        dynarec_log(LOG_NONE, "%s%p: %d emitted opcodes, inst=%d, barrier=%d state=%d/%d(%d), %s=%X/%X, use=%X, need=%X/%X, sm=%d/%d",
            (box64_dynarec_dump>1)?"\e[32m":"",
            (void*)(dyn->native_start+dyn->insts[ninst].address),
            dyn->insts[ninst].size/4,
            ninst,
            dyn->insts[ninst].x64.barrier,
            dyn->insts[ninst].x64.state_flags,
            dyn->f.pending,
            dyn->f.dfnone,
            dyn->insts[ninst].x64.may_set?"may":"set",
            dyn->insts[ninst].x64.set_flags,
            dyn->insts[ninst].x64.gen_flags,
            dyn->insts[ninst].x64.use_flags,
            dyn->insts[ninst].x64.need_before,
            dyn->insts[ninst].x64.need_after,
            dyn->smread, dyn->smwrite);
        if(dyn->insts[ninst].pred_sz) {
            dynarec_log(LOG_NONE, ", pred=");
            for(int ii=0; ii<dyn->insts[ninst].pred_sz; ++ii)
                dynarec_log(LOG_NONE, "%s%d", ii?"/":"", dyn->insts[ninst].pred[ii]);
        }
        if(dyn->insts[ninst].x64.jmp && dyn->insts[ninst].x64.jmp_insts>=0)
            dynarec_log(LOG_NONE, ", jmp=%d", dyn->insts[ninst].x64.jmp_insts);
        if(dyn->insts[ninst].x64.jmp && dyn->insts[ninst].x64.jmp_insts==-1)
            dynarec_log(LOG_NONE, ", jmp=out");
        if(dyn->last_ip)
            dynarec_log(LOG_NONE, ", last_ip=%p", (void*)dyn->last_ip);
        for(int ii=0; ii<24; ++ii) {
            switch(dyn->insts[ninst].e.extcache[ii].t) {
                case EXT_CACHE_ST_D: dynarec_log(LOG_NONE, " %s:%s", fnames[EXTREG(ii)], getCacheName(dyn->insts[ninst].e.extcache[ii].t, dyn->insts[ninst].e.extcache[ii].n)); break;
                case EXT_CACHE_ST_F: dynarec_log(LOG_NONE, " %s:%s", fnames[EXTREG(ii)], getCacheName(dyn->insts[ninst].e.extcache[ii].t, dyn->insts[ninst].e.extcache[ii].n)); break;
                case EXT_CACHE_ST_I64: dynarec_log(LOG_NONE, " %s:%s", fnames[EXTREG(ii)], getCacheName(dyn->insts[ninst].e.extcache[ii].t, dyn->insts[ninst].e.extcache[ii].n)); break;
                case EXT_CACHE_MM: dynarec_log(LOG_NONE, " %s:%s", fnames[EXTREG(ii)], getCacheName(dyn->insts[ninst].e.extcache[ii].t, dyn->insts[ninst].e.extcache[ii].n)); break;
                case EXT_CACHE_SS: dynarec_log(LOG_NONE, " %s:%s", fnames[EXTREG(ii)], getCacheName(dyn->insts[ninst].e.extcache[ii].t, dyn->insts[ninst].e.extcache[ii].n)); break;
                case EXT_CACHE_SD: dynarec_log(LOG_NONE, " %s:%s", fnames[EXTREG(ii)], getCacheName(dyn->insts[ninst].e.extcache[ii].t, dyn->insts[ninst].e.extcache[ii].n)); break;
                case EXT_CACHE_SCR: dynarec_log(LOG_NONE, " %s:%s", fnames[EXTREG(ii)], getCacheName(dyn->insts[ninst].e.extcache[ii].t, dyn->insts[ninst].e.extcache[ii].n)); break;
                case EXT_CACHE_NONE:
                default:    break;
            }
        }
        if(dyn->e.stack || dyn->insts[ninst].e.stack_next || dyn->insts[ninst].e.x87stack)
            dynarec_log(LOG_NONE, " X87:%d/%d(+%d/-%d)%d", dyn->e.stack, dyn->insts[ninst].e.stack_next, dyn->insts[ninst].e.stack_push, dyn->insts[ninst].e.stack_pop, dyn->insts[ninst].e.x87stack);
        if(dyn->insts[ninst].e.combined1 || dyn->insts[ninst].e.combined2)
            dynarec_log(LOG_NONE, " %s:%d/%d", dyn->insts[ninst].e.swapped?"SWP":"CMB", dyn->insts[ninst].e.combined1, dyn->insts[ninst].e.combined2);
        dynarec_log(LOG_NONE, "%s\n", (box64_dynarec_dump>1)?"\e[m":"");
    }
}

void print_opcode(dynarec_native_t* dyn, int ninst, uint32_t opcode)
{
    dynarec_log(LOG_NONE, "\t%08x\t%s\n", opcode, rv64_print(opcode, (uintptr_t)dyn->block));
}

void print_newinst(dynarec_native_t* dyn, int ninst)
{
    dynarec_log(LOG_NONE, "%sNew instruction %d, native=%p (0x%x)%s\n",
        (box64_dynarec_dump>1)?"\e[4;32m":"",
        ninst, dyn->block, dyn->native_size,
        (box64_dynarec_dump>1)?"\e[m":""
        );
}

// x87 stuffs
static void x87_reset(extcache_t* e)
{
    for (int i=0; i<8; ++i)
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

    for(int i=0; i<24; ++i)
        if (e->extcache[i].t == EXT_CACHE_ST_F
            || e->extcache[i].t == EXT_CACHE_ST_D
            || e->extcache[i].t == EXT_CACHE_ST_I64)
            e->extcache[i].v = 0;
}

static void mmx_reset(extcache_t* e)
{
    e->mmxcount = 0;
    for (int i=0; i<8; ++i)
        e->mmxcache[i] = -1;
}

static void sse_reset(extcache_t* e)
{
    for (int i=0; i<16; ++i)
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
    fpu_reset_reg_extcache(&dyn->insts[ninst].e);
}

int fpu_is_st_freed(dynarec_rv64_t* dyn, int ninst, int st)
{
    return (dyn->e.tags&(0b11<<(st*2)))?1:0;
}