#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
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
#include "tools/bridge_private.h"
#include "x64run.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "emu/x87emu_private.h"
#include "x64trace.h"
#include "signals.h"
#include "dynarec_rv64.h"
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
// Free a FPU double reg
void fpu_free_reg(dynarec_rv64_t* dyn, int reg)
{
    int idx = EXTIDX(reg);
    // TODO: check upper limit?
    dyn->e.fpuused[idx] = 0;
    if(dyn->e.extcache[idx].t!=EXT_CACHE_ST_F && dyn->e.extcache[idx].t!=EXT_CACHE_ST_D)
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
// Get an XMM quad reg
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
void fpu_reset_reg(dynarec_rv64_t* dyn)
{
    dyn->e.fpu_reg = 0;
    for (int i=0; i<24; ++i) {
        dyn->e.fpuused[i]=0;
        dyn->e.extcache[i].v = 0;
    }
}

int extcache_get_st(dynarec_rv64_t* dyn, int ninst, int a)
{
    if (dyn->insts[ninst].e.swapped) {
        if(dyn->insts[ninst].e.combined1 == a)
            a = dyn->insts[ninst].e.combined2;
        else if(dyn->insts[ninst].e.combined2 == a)
            a = dyn->insts[ninst].e.combined1;
    }
    for(int i=0; i<24; ++i)
        if((dyn->insts[ninst].e.extcache[i].t==EXT_CACHE_ST_F
         || dyn->insts[ninst].e.extcache[i].t==EXT_CACHE_ST_D)
         && dyn->insts[ninst].e.extcache[i].n==a)
            return dyn->insts[ninst].e.extcache[i].t;
    // not in the cache yet, so will be fetched...
    return EXT_CACHE_ST_D;
}

int extcache_get_current_st(dynarec_rv64_t* dyn, int ninst, int a)
{
    (void)ninst;
    if(!dyn->insts)
        return EXT_CACHE_ST_D;
    for(int i=0; i<24; ++i)
        if((dyn->e.extcache[i].t==EXT_CACHE_ST_F
         || dyn->e.extcache[i].t==EXT_CACHE_ST_D)
         && dyn->e.extcache[i].n==a)
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
int extcache_get_st_f_noback(dynarec_rv64_t* dyn, int ninst, int a)
{
    for(int i=0; i<24; ++i)
        if(dyn->insts[ninst].e.extcache[i].t==EXT_CACHE_ST_F
         && dyn->insts[ninst].e.extcache[i].n==a)
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

static void extcache_promote_double_forward(dynarec_rv64_t* dyn, int ninst, int maxinst, int a);
static void extcache_promote_double_internal(dynarec_rv64_t* dyn, int ninst, int maxinst, int a);
static void extcache_promote_double_combined(dynarec_rv64_t* dyn, int ninst, int maxinst, int a)
{
    if(a == dyn->insts[ninst].e.combined1 || a == dyn->insts[ninst].e.combined2) {
        if(a == dyn->insts[ninst].e.combined1) {
            a = dyn->insts[ninst].e.combined2;
        } else 
            a = dyn->insts[ninst].e.combined1;
        int i = extcache_get_st_f_noback(dyn, ninst, a);
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
        int i = extcache_get_st_f(dyn, ninst, a);
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
        int i = extcache_get_st_f_noback(dyn, ninst, a);
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
    int i = extcache_get_current_st_f(dyn, a);
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

static int isCacheEmpty(dynarec_native_t* dyn, int ninst) {
    if(dyn->insts[ninst].e.stack_next) {
        return 0;
    }
    for(int i=0; i<24; ++i)
        if(dyn->insts[ninst].e.extcache[i].v) {       // there is something at ninst for i
            if(!(
            (dyn->insts[ninst].e.extcache[i].t==EXT_CACHE_ST_F || dyn->insts[ninst].e.extcache[i].t==EXT_CACHE_ST_D)
            && dyn->insts[ninst].e.extcache[i].n<dyn->insts[ninst].e.stack_pop))
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
                if(!(
                (dyn->insts[ninst].e.extcache[i].t==EXT_CACHE_ST_F || dyn->insts[ninst].e.extcache[i].t==EXT_CACHE_ST_D)
                && dyn->insts[ninst].e.extcache[i].n<dyn->insts[ninst].e.stack_pop))
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
                if(dyn->insts[ninst].e.extcache[i].n!=cache_i2.extcache[i].n) {   // not the same x64 reg
                    ret = 1;
                }
                else if(dyn->insts[ninst].e.extcache[i].t == EXT_CACHE_SS && cache_i2.extcache[i].t == EXT_CACHE_SD)
                    {/* nothing */ }
                else
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
            if((cache->extcache[j].t == EXT_CACHE_ST_D || cache->extcache[j].t == EXT_CACHE_ST_F)) {
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
        // reove the newly created extcache
        for(int i=0; i<24; ++i)
            if(cache->news&(1<<i))
                cache->extcache[i].v = 0;
        cache->news = 0;
    }
    if(cache->stack_push) {
        // unpush
        for(int j=0; j<24; ++j) {
            if((cache->extcache[j].t == EXT_CACHE_ST_D || cache->extcache[j].t == EXT_CACHE_ST_F)) {
                if(cache->extcache[j].n<cache->stack_push)
                    cache->extcache[j].v = 0;
                else
                    cache->extcache[j].n-=cache->stack_push;
            }
        }
        cache->x87stack-=cache->stack_push;
        cache->stack-=cache->stack_push;
        cache->stack_push = 0;
    }
    cache->x87stack+=cache->stack_pop;
    cache->stack_next = cache->stack;
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
                    cache->mmxcache[cache->extcache[i].n] = i;
                    ++cache->mmxcount;
                    ++cache->fpu_reg;
                    break;
                case EXT_CACHE_SS:
                    cache->ssecache[cache->extcache[i].n].reg = i;
                    cache->ssecache[cache->extcache[i].n].single = 1;
                    ++cache->fpu_reg;
                    break;
                case EXT_CACHE_SD:
                    cache->ssecache[cache->extcache[i].n].reg = i;
                    cache->ssecache[cache->extcache[i].n].single = 0;
                    ++cache->fpu_reg;
                    break;
                case EXT_CACHE_ST_F:
                case EXT_CACHE_ST_D:
                    cache->x87cache[x87reg] = cache->extcache[i].n;
                    cache->x87reg[x87reg] = i;
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


uint8_t extract_byte(uint32_t val, void* address){
    int idx = (((uintptr_t)address)&3)*8;
    return (val>>idx)&0xff;
}
uint32_t insert_byte(uint32_t val, uint8_t b, void* address){
    int idx = (((uintptr_t)address)&3)*8;
    val&=~(0xff<<idx);
    val|=(((uint32_t)b)<<idx);
    return val;
}

// will go badly if address is unaligned
uint16_t extract_half(uint32_t val, void* address){
    int idx = (((uintptr_t)address)&3)*8;
    return (val>>idx)&0xffff;
}
uint32_t insert_half(uint32_t val, uint16_t h, void* address){
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
        case EXT_CACHE_MM: sprintf(buff, "MM%d", n); break;
        case EXT_CACHE_SS: sprintf(buff, "SS%d", n); break;
        case EXT_CACHE_SD: sprintf(buff, "SD%d", n); break;
        case EXT_CACHE_SCR: sprintf(buff, "Scratch"); break;
        case EXT_CACHE_NONE: buff[0]='\0'; break;
    }
    return buff;
}
