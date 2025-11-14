#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>
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
#include "dynarec_native.h"
#include "dynarec_arm64_private.h"
#include "dynarec_arm64_functions.h"
#include "custommem.h"
#include "bridge.h"
#include "gdbjit.h"
#include "perfmap.h"

// Get a FPU scratch reg
int fpu_get_scratch(dynarec_arm_t* dyn, int ninst)
{
    int ret = SCRATCH0 + dyn->n.fpu_scratch++;
    if(dyn->n.ymm_used) {
        printf_log(LOG_INFO, "Warning, getting a scratch register after getting some YMM at inst=%d", ninst);
        uint8_t* addr = (uint8_t*)dyn->insts[ninst].x64.addr;
        printf_log_prefix(0, LOG_INFO, "(%hhX %hhX %hhX %hhX %hhX %hhX %hhX %hhX)\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]);
    }
    if(dyn->n.neoncache[ret].t==NEON_CACHE_YMMR || dyn->n.neoncache[ret].t==NEON_CACHE_YMMW) {
        // should only happens in step 0...
        dyn->insts[ninst].purge_ymm |= (1<<dyn->n.neoncache[ret].n); // mark as purged
        dyn->n.neoncache[ret].v = 0; // reset it
    }
    return ret;
}
// Get 2 consicutive FPU scratch reg
int fpu_get_double_scratch(dynarec_arm_t* dyn, int ninst)
{
    int ret = SCRATCH0 + dyn->n.fpu_scratch;
    if(dyn->n.ymm_used) {
        printf_log(LOG_INFO, "Warning, getting a double scratch register after getting some YMM at inst=%d", ninst);
        uint8_t* addr = (uint8_t*)dyn->insts[ninst].x64.addr;
        printf_log_prefix(0, LOG_INFO, "(%hhX %hhX %hhX %hhX %hhX %hhX %hhX %hhX)\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]);
    }
    if(dyn->n.neoncache[ret].t==NEON_CACHE_YMMR || dyn->n.neoncache[ret].t==NEON_CACHE_YMMW) {
        // should only happens in step 0...
        dyn->insts[ninst].purge_ymm |= (1<<dyn->n.neoncache[ret].n); // mark as purged
        dyn->n.neoncache[ret].v = 0; // reset it
    }
    if(dyn->n.neoncache[ret+1].t==NEON_CACHE_YMMR || dyn->n.neoncache[ret+1].t==NEON_CACHE_YMMW) {
        // should only happens in step 0...
        dyn->insts[ninst].purge_ymm |= (1<<dyn->n.neoncache[ret+1].n); // mark as purged
        dyn->n.neoncache[ret+1].v = 0; // reset it
    }
    dyn->n.fpu_scratch+=2;
    return ret;
}
// Reset scratch regs counter
void fpu_reset_scratch(dynarec_arm_t* dyn)
{
    dyn->n.fpu_scratch = 0;
    dyn->n.ymm_used = 0;
    dyn->n.ymm_regs = 0;
    dyn->n.ymm_write = 0;
    dyn->n.ymm_removed = 0;
    dyn->n.xmm_write = 0;
    dyn->n.xmm_used = 0;
    dyn->n.xmm_unneeded = 0;
    dyn->n.ymm_unneeded = 0;
    dyn->n.xmm_removed = 0;
}
// Get a x87 double reg
int fpu_get_reg_x87(dynarec_arm_t* dyn, int ninst, int t, int n)
{
    int i=X870;
    while (dyn->n.fpuused[i]) ++i;
    if(dyn->n.neoncache[i].t==NEON_CACHE_YMMR || dyn->n.neoncache[i].t==NEON_CACHE_YMMW) {
        // should only happens in step 0...
        dyn->insts[ninst].purge_ymm |= (1<<dyn->n.neoncache[i].n); // mark as purged
        dyn->n.neoncache[i].v = 0; // reset it
    }
    dyn->n.fpuused[i] = 1;
    dyn->n.neoncache[i].n = n;
    dyn->n.neoncache[i].t = t;
    dyn->n.news |= (1<<i);
    dyn->use_x87 = 1;
    return i; // return a Dx
}
// Free a FPU double reg
void fpu_free_reg(dynarec_arm_t* dyn, int reg)
{
    // TODO: check upper limit?
    dyn->n.fpuused[reg] = 0;
    if(dyn->n.neoncache[reg].t==NEON_CACHE_YMMR || dyn->n.neoncache[reg].t==NEON_CACHE_YMMW) {
        dyn->n.ymm_removed |= 1<<dyn->n.neoncache[reg].n;
        if(dyn->n.neoncache[reg].t==NEON_CACHE_YMMW)
            dyn->n.ymm_write |= 1<<dyn->n.neoncache[reg].n;
        if(reg>SCRATCH0)
            dyn->n.ymm_regs |= (8LL+reg-SCRATCH0)<<(dyn->n.neoncache[reg].n*4);
        else
            dyn->n.ymm_regs |= ((uint64_t)(reg-EMM0))<<(dyn->n.neoncache[reg].n*4);
    }
    if(dyn->n.neoncache[reg].t==NEON_CACHE_XMMR || dyn->n.neoncache[reg].t==NEON_CACHE_XMMW) {
        dyn->n.xmm_removed |= 1<<dyn->n.neoncache[reg].n;
        if(dyn->n.neoncache[reg].t==NEON_CACHE_XMMW)
            dyn->n.xmm_write |= 1<<dyn->n.neoncache[reg].n;
    }
    if(dyn->n.neoncache[reg].t!=NEON_CACHE_ST_F && dyn->n.neoncache[reg].t!=NEON_CACHE_ST_D && dyn->n.neoncache[reg].t!=NEON_CACHE_ST_I64)
        dyn->n.neoncache[reg].v = 0;
    if(dyn->n.fpu_scratch && reg==SCRATCH0+dyn->n.fpu_scratch-1)
        --dyn->n.fpu_scratch;
}
// Get an MMX double reg
int fpu_get_reg_emm(dynarec_arm_t* dyn, int ninst, int emm)
{
    int ret = EMM0 + emm;
    if(dyn->n.neoncache[ret].t==NEON_CACHE_YMMR || dyn->n.neoncache[ret].t==NEON_CACHE_YMMW) {
        // should only happens in step 0...
        dyn->insts[ninst].purge_ymm |= (1<<dyn->n.neoncache[ret].n); // mark as purged
        dyn->n.neoncache[ret].v = 0; // reset it
    }
    dyn->n.fpuused[ret] = 1;
    dyn->n.neoncache[ret].t = NEON_CACHE_MM;
    dyn->n.neoncache[ret].n = emm;
    dyn->n.news |= (1<<(ret));
    dyn->use_mmx = 1;
    return ret;
}
// Get an XMM quad reg
int fpu_get_reg_xmm(dynarec_arm_t* dyn, int t, int xmm)
{
    int i;
    if(xmm>7) {
        i = XMM8 + xmm - 8;
    } else {
        i = XMM0 + xmm;
    }
    dyn->n.fpuused[i] = 1;
    dyn->n.neoncache[i].t = t;
    dyn->n.neoncache[i].n = xmm;
    dyn->n.news |= (1<<i);
    dyn->use_xmm = 1;
    return i;
}
int internal_mark_ymm(dynarec_arm_t* dyn, int t, int ymm, int reg)
{
    dyn->use_ymm = 1;
    if((dyn->n.neoncache[reg].t==NEON_CACHE_YMMR) || (dyn->n.neoncache[reg].t==NEON_CACHE_YMMW)) {
        if(dyn->n.neoncache[reg].n == ymm) {
            // already there!
            if(t==NEON_CACHE_YMMW)
                dyn->n.neoncache[reg].t=t;
            return reg;
        }
    } else if(!dyn->n.neoncache[reg].v) {
        // found a slot!
        dyn->n.neoncache[reg].t=t;
        dyn->n.neoncache[reg].n=ymm;
        dyn->n.news |= (1<<reg);
        return reg;
    }
    return -1;
}
int is_ymm_to_keep(dynarec_arm_t* dyn, int reg, int k1, int k2, int k3)
{
    if((k1!=-1) && (dyn->n.neoncache[reg].n==k1))
        return 1;
    if((k2!=-1) && (dyn->n.neoncache[reg].n==k2))
        return 1;
    if((k3!=-1) && (dyn->n.neoncache[reg].n==k3))
        return 1;
    if((dyn->n.neoncache[reg].t==NEON_CACHE_YMMR || dyn->n.neoncache[reg].t==NEON_CACHE_YMMW) && (dyn->n.ymm_used&(1<<dyn->n.neoncache[reg].n)))
        return 1;
    return 0;
}

// Reset fpu regs counter
static void fpu_reset_reg_neoncache(neoncache_t* n)
{
    for (int i=0; i<32; ++i) {
        n->fpuused[i]=0;
        n->neoncache[i].v = 0;
    }
    n->ymm_regs = 0;
    n->ymm_removed = 0;
    n->ymm_used = 0;
    n->ymm_write = 0;
    n->xmm_removed = 0;
    n->xmm_write = 0;

}
void fpu_reset_reg(dynarec_arm_t* dyn)
{
    fpu_reset_reg_neoncache(&dyn->n);
}

int neoncache_no_i64(dynarec_arm_t* dyn, int ninst, int st, int a)
{
    if(a==NEON_CACHE_ST_I64) {
        neoncache_promote_double(dyn, ninst, st);
        return NEON_CACHE_ST_D;
    }
    return a;
}

int neoncache_get_st(dynarec_arm_t* dyn, int ninst, int a)
{
    if (dyn->insts[ninst].n.swapped) {
        if(dyn->insts[ninst].n.combined1 == a)
            a = dyn->insts[ninst].n.combined2;
        else if(dyn->insts[ninst].n.combined2 == a)
            a = dyn->insts[ninst].n.combined1;
    }
    for(int i=0; i<24; ++i)
        if((dyn->insts[ninst].n.neoncache[i].t==NEON_CACHE_ST_F
         || dyn->insts[ninst].n.neoncache[i].t==NEON_CACHE_ST_D
         || dyn->insts[ninst].n.neoncache[i].t==NEON_CACHE_ST_I64)
         && dyn->insts[ninst].n.neoncache[i].n==a)
            return dyn->insts[ninst].n.neoncache[i].t;
    // not in the cache yet, so will be fetched...
    return NEON_CACHE_ST_D;
}

int neoncache_get_current_st(dynarec_arm_t* dyn, int ninst, int a)
{
    (void)ninst;
    if(!dyn->insts)
        return NEON_CACHE_ST_D;
    for(int i=0; i<24; ++i)
        if((dyn->n.neoncache[i].t==NEON_CACHE_ST_F
         || dyn->n.neoncache[i].t==NEON_CACHE_ST_D
         || dyn->n.neoncache[i].t==NEON_CACHE_ST_I64)
         && dyn->n.neoncache[i].n==a)
            return dyn->n.neoncache[i].t;
    // not in the cache yet, so will be fetched...
    return NEON_CACHE_ST_D;
}

int neoncache_get_st_f(dynarec_arm_t* dyn, int ninst, int a)
{
    /*if(a+dyn->insts[ninst].n.stack_next-st<0)
        // The STx has been pushed at the end of instructon, so stop going back
        return -1;*/
    for(int i=0; i<24; ++i)
        if(dyn->insts[ninst].n.neoncache[i].t==NEON_CACHE_ST_F
         && dyn->insts[ninst].n.neoncache[i].n==a)
            return i;
    return -1;
}
int neoncache_get_st_f_i64(dynarec_arm_t* dyn, int ninst, int a)
{
    /*if(a+dyn->insts[ninst].n.stack_next-st<0)
        // The STx has been pushed at the end of instructon, so stop going back
        return -1;*/
    for(int i=0; i<24; ++i)
        if((dyn->insts[ninst].n.neoncache[i].t==NEON_CACHE_ST_I64 || dyn->insts[ninst].n.neoncache[i].t==NEON_CACHE_ST_F)
         && dyn->insts[ninst].n.neoncache[i].n==a)
            return i;
    return -1;
}
int neoncache_get_st_f_noback(dynarec_arm_t* dyn, int ninst, int a)
{
    for(int i=0; i<24; ++i)
        if(dyn->insts[ninst].n.neoncache[i].t==NEON_CACHE_ST_F
         && dyn->insts[ninst].n.neoncache[i].n==a)
            return i;
    return -1;
}
int neoncache_get_st_f_i64_noback(dynarec_arm_t* dyn, int ninst, int a)
{
    for(int i=0; i<24; ++i)
        if((dyn->insts[ninst].n.neoncache[i].t==NEON_CACHE_ST_I64 || dyn->insts[ninst].n.neoncache[i].t==NEON_CACHE_ST_F)
         && dyn->insts[ninst].n.neoncache[i].n==a)
            return i;
    return -1;
}
int neoncache_get_current_st_f(dynarec_arm_t* dyn, int a)
{
    for(int i=0; i<24; ++i)
        if(dyn->n.neoncache[i].t==NEON_CACHE_ST_F
         && dyn->n.neoncache[i].n==a)
            return i;
    return -1;
}
int neoncache_get_current_st_f_i64(dynarec_arm_t* dyn, int a)
{
    for(int i=0; i<24; ++i)
        if((dyn->n.neoncache[i].t==NEON_CACHE_ST_I64 || dyn->n.neoncache[i].t==NEON_CACHE_ST_F)
         && dyn->n.neoncache[i].n==a)
            return i;
    return -1;
}
static void neoncache_promote_double_forward(dynarec_arm_t* dyn, int ninst, int maxinst, int a);
static void neoncache_promote_double_internal(dynarec_arm_t* dyn, int ninst, int maxinst, int a);
static void neoncache_promote_double_combined(dynarec_arm_t* dyn, int ninst, int maxinst, int a)
{
    if(a == dyn->insts[ninst].n.combined1 || a == dyn->insts[ninst].n.combined2) {
        if(a == dyn->insts[ninst].n.combined1) {
            a = dyn->insts[ninst].n.combined2;
        } else
            a = dyn->insts[ninst].n.combined1;
        int i = neoncache_get_st_f_i64_noback(dyn, ninst, a);
        //if(dyn->need_dump) dynarec_log(LOG_NONE, "neoncache_promote_double_combined, ninst=%d combined%c %d i=%d (stack:%d/%d)\n", ninst, (a == dyn->insts[ninst].n.combined2)?'2':'1', a ,i, dyn->insts[ninst].n.stack_push, -dyn->insts[ninst].n.stack_pop);
        if(i>=0) {
            dyn->insts[ninst].n.neoncache[i].t = NEON_CACHE_ST_D;
            if(dyn->insts[ninst].x87precision) dyn->need_x87check = 2;
            if(!dyn->insts[ninst].n.barrier)
                neoncache_promote_double_internal(dyn, ninst-1, maxinst, a-dyn->insts[ninst].n.stack_push);
            // go forward is combined is not pop'd
            if(a-dyn->insts[ninst].n.stack_pop>=0)
                if(!((ninst+1<dyn->size) && dyn->insts[ninst+1].n.barrier))
                    neoncache_promote_double_forward(dyn, ninst+1, maxinst, a-dyn->insts[ninst].n.stack_pop);
        }
    }
}
static void neoncache_promote_double_internal(dynarec_arm_t* dyn, int ninst, int maxinst, int a)
{
    while(ninst>=0) {
        a+=dyn->insts[ninst].n.stack_pop;    // adjust Stack depth: add pop'd ST (going backward)
        int i = neoncache_get_st_f_i64(dyn, ninst, a);
        //if(dyn->need_dump) dynarec_log(LOG_NONE, "neoncache_promote_double_internal, ninst=%d, a=%d st=%d:%d, i=%d\n", ninst, a, dyn->insts[ninst].n.stack, dyn->insts[ninst].n.stack_next, i);
        if(i<0) return;
        dyn->insts[ninst].n.neoncache[i].t = NEON_CACHE_ST_D;
        if(dyn->insts[ninst].x87precision) dyn->need_x87check = 2;
        // check combined propagation too
        if(dyn->insts[ninst].n.combined1 || dyn->insts[ninst].n.combined2) {
            if(dyn->insts[ninst].n.swapped) {
                //if(dyn->need_dump) dynarec_log(LOG_NONE, "neoncache_promote_double_internal, ninst=%d swapped %d/%d vs %d with st %d\n", ninst, dyn->insts[ninst].n.combined1 ,dyn->insts[ninst].n.combined2, a, dyn->insts[ninst].n.stack);
                if (a==dyn->insts[ninst].n.combined1)
                    a = dyn->insts[ninst].n.combined2;
                else if (a==dyn->insts[ninst].n.combined2)
                    a = dyn->insts[ninst].n.combined1;
            } else {
                //if(dyn->need_dump) dynarec_log(LOG_NONE, "neoncache_promote_double_internal, ninst=%d combined %d/%d vs %d with st %d\n", ninst, dyn->insts[ninst].n.combined1 ,dyn->insts[ninst].n.combined2, a, dyn->insts[ninst].n.stack);
                neoncache_promote_double_combined(dyn, ninst, maxinst, a);
            }
        }
        a-=dyn->insts[ninst].n.stack_push;  // // adjust Stack depth: remove push'd ST (going backward)
        --ninst;
        if(ninst<0 || a<0 || dyn->insts[ninst].n.barrier)
            return;
    }
}

static void neoncache_promote_double_forward(dynarec_arm_t* dyn, int ninst, int maxinst, int a)
{
    while((ninst!=-1) && (ninst<maxinst) && (a>=0)) {
        a+=dyn->insts[ninst].n.stack_push;  // // adjust Stack depth: add push'd ST (going forward)
        if((dyn->insts[ninst].n.combined1 || dyn->insts[ninst].n.combined2) && dyn->insts[ninst].n.swapped) {
            //if(dyn->need_dump) dynarec_log(LOG_NONE, "neoncache_promote_double_forward, ninst=%d swapped %d/%d vs %d with st %d\n", ninst, dyn->insts[ninst].n.combined1 ,dyn->insts[ninst].n.combined2, a, dyn->insts[ninst].n.stack);
            if (a==dyn->insts[ninst].n.combined1)
                a = dyn->insts[ninst].n.combined2;
            else if (a==dyn->insts[ninst].n.combined2)
                a = dyn->insts[ninst].n.combined1;
        }
        int i = neoncache_get_st_f_i64_noback(dyn, ninst, a);
        //if(dyn->need_dump) dynarec_log(LOG_NONE, "neoncache_promote_double_forward, ninst=%d, a=%d st=%d:%d(%d/%d), i=%d\n", ninst, a, dyn->insts[ninst].n.stack, dyn->insts[ninst].n.stack_next, dyn->insts[ninst].n.stack_push, -dyn->insts[ninst].n.stack_pop, i);
        if(i<0) return;
        dyn->insts[ninst].n.neoncache[i].t = NEON_CACHE_ST_D;
        if(dyn->insts[ninst].x87precision) dyn->need_x87check = 2;
        // check combined propagation too
        if((dyn->insts[ninst].n.combined1 || dyn->insts[ninst].n.combined2) && !dyn->insts[ninst].n.swapped) {
            //if(dyn->need_dump) dynarec_log(LOG_NONE, "neoncache_promote_double_forward, ninst=%d combined %d/%d vs %d with st %d\n", ninst, dyn->insts[ninst].n.combined1 ,dyn->insts[ninst].n.combined2, a, dyn->insts[ninst].n.stack);
            neoncache_promote_double_combined(dyn, ninst, maxinst, a);
        }
        a-=dyn->insts[ninst].n.stack_pop;    // adjust Stack depth: remove pop'd ST (going forward)
        if(dyn->insts[ninst].x64.has_next && !dyn->insts[ninst].n.barrier)
            ++ninst;
        else
            ninst=-1;
    }
    if(ninst==maxinst)
        neoncache_promote_double(dyn, ninst, a);
}

void neoncache_promote_double(dynarec_arm_t* dyn, int ninst, int a)
{
    int i = neoncache_get_current_st_f_i64(dyn, a);
    //if(dyn->need_dump) dynarec_log(LOG_NONE, "neoncache_promote_double, ninst=%d a=%d st=%d i=%d\n", ninst, a, dyn->n.stack, i);
    if(i<0) return;
    dyn->n.neoncache[i].t = NEON_CACHE_ST_D;
    dyn->insts[ninst].n.neoncache[i].t = NEON_CACHE_ST_D;
    if(dyn->insts[ninst].x87precision) dyn->need_x87check = 2;
    // check combined propagation too
    if(dyn->n.combined1 || dyn->n.combined2) {
        if(dyn->n.swapped) {
            //if(dyn->need_dump) dynarec_log(LOG_NONE, "neoncache_promote_double, ninst=%d swapped! %d/%d vs %d\n", ninst, dyn->n.combined1 ,dyn->n.combined2, a);
            if(dyn->n.combined1 == a)
                a = dyn->n.combined2;
            else if(dyn->n.combined2 == a)
                a = dyn->n.combined1;
        } else {
            //if(dyn->need_dump) dynarec_log(LOG_NONE, "neoncache_promote_double, ninst=%d combined! %d/%d vs %d\n", ninst, dyn->n.combined1 ,dyn->n.combined2, a);
            if(dyn->n.combined1 == a)
                neoncache_promote_double(dyn, ninst, dyn->n.combined2);
            else if(dyn->n.combined2 == a)
                neoncache_promote_double(dyn, ninst, dyn->n.combined1);
        }
    }
    a-=dyn->insts[ninst].n.stack_push;  // // adjust Stack depth: remove push'd ST (going backward)
    if(!ninst || a<0) return;
    neoncache_promote_double_internal(dyn, ninst-1, ninst, a);
}

int neoncache_combine_st(dynarec_arm_t* dyn, int ninst, int a, int b)
{
    dyn->n.combined1=a;
    dyn->n.combined2=b;
    if( neoncache_get_current_st(dyn, ninst, a)==NEON_CACHE_ST_F
     && neoncache_get_current_st(dyn, ninst, b)==NEON_CACHE_ST_F )
        return NEON_CACHE_ST_F;
    // don't combine i64, it's only for load/store
    /*if( neoncache_get_current_st(dyn, ninst, a)==NEON_CACHE_ST_I64
     && neoncache_get_current_st(dyn, ninst, b)==NEON_CACHE_ST_I64 )
        return NEON_CACHE_ST_I64;*/
    return NEON_CACHE_ST_D;
}

static int isCacheEmpty(dynarec_native_t* dyn, int ninst) {
    if(dyn->insts[ninst].n.stack_next) {
        return 0;
    }
    for(int i=0; i<24; ++i)
        if(dyn->insts[ninst].n.neoncache[i].v) {       // there is something at ninst for i
            if(!(
            (dyn->insts[ninst].n.neoncache[i].t==NEON_CACHE_ST_F
             || dyn->insts[ninst].n.neoncache[i].t==NEON_CACHE_ST_D
             || dyn->insts[ninst].n.neoncache[i].t==NEON_CACHE_ST_I64)
            && dyn->insts[ninst].n.neoncache[i].n<dyn->insts[ninst].n.stack_pop))
                return 0;
        }
    return 1;

}

int fpuCacheNeedsTransform(dynarec_arm_t* dyn, int ninst) {
    int i2 = dyn->insts[ninst].x64.jmp_insts;
    if(i2<0)
        return 1;
    if((dyn->insts[i2].x64.barrier&BARRIER_FLOAT))
        // if the barrier as already been apply, no transform needed
        return ((dyn->insts[ninst].x64.barrier&BARRIER_FLOAT))?0:(isCacheEmpty(dyn, ninst)?0:1);
    int ret = 0;
    if(!i2) { // just purge
        if(dyn->insts[ninst].n.stack_next)
            return 1;
        if(dyn->insts[ninst].ymm0_out)
            return 1;
        for(int i=0; i<32 && !ret; ++i)
            if(dyn->insts[ninst].n.neoncache[i].v) {       // there is something at ninst for i
                int t = dyn->insts[ninst].n.neoncache[i].t;
                int n = dyn->insts[ninst].n.neoncache[i].n;
                if(!(
                (t==NEON_CACHE_ST_F
                || t==NEON_CACHE_ST_D
                || t==NEON_CACHE_ST_I64)
                && n<dyn->insts[ninst].n.stack_pop))
                    ret = 1;
            }
        return ret;
    }
    // Check if ninst can be compatible to i2
    if(dyn->insts[ninst].n.stack_next != dyn->insts[i2].n.stack-dyn->insts[i2].n.stack_push) {
        return 1;
    }
    if(dyn->insts[ninst].ymm0_out && (dyn->insts[ninst].ymm0_out&~dyn->insts[i2].ymm0_in))
        return 1;
    neoncache_t cache_i2 = dyn->insts[i2].n;
    neoncacheUnwind(&cache_i2);

    for(int i=0; i<32; ++i) {
        if(dyn->insts[ninst].n.neoncache[i].v) {       // there is something at ninst for i
            int t = dyn->insts[ninst].n.neoncache[i].t;
            int n = dyn->insts[ninst].n.neoncache[i].n;
            if(!cache_i2.neoncache[i].v) {    // but there is nothing at i2 for i
                if(((t==NEON_CACHE_XMMR) || (t==NEON_CACHE_XMMW)) && (cache_i2.xmm_unneeded&(1<<n))) { /* nothing*/}
                else if(((t==NEON_CACHE_YMMR) || (t==NEON_CACHE_YMMW)) && (cache_i2.ymm_unneeded&(1<<n))) { /* nothing*/}
                else 
                ret = 1;
            } else if(dyn->insts[ninst].n.neoncache[i].v!=cache_i2.neoncache[i].v) {  // there is something different
                if(n!=cache_i2.neoncache[i].n) {   // not the same x64 reg
                    ret = 1;
                }
                else if((t == NEON_CACHE_XMMR) && cache_i2.neoncache[i].t == NEON_CACHE_XMMW)
                    {/* nothing */ }
                else if((t == NEON_CACHE_YMMR) && cache_i2.neoncache[i].t == NEON_CACHE_YMMW)
                    {/* nothing */ }
                else
                    ret = 1;
            }
        } else if(cache_i2.neoncache[i].v)
            ret = 1;
    }
    return ret;
}

void neoncacheUnwind(neoncache_t* cache)
{
    if(cache->swapped) {
        // unswap
        int a = -1;
        int b = -1;
        // in neoncache
        for(int j=0; j<24 && ((a==-1) || (b==-1)); ++j)
            if((cache->neoncache[j].t == NEON_CACHE_ST_D || cache->neoncache[j].t == NEON_CACHE_ST_F || cache->neoncache[j].t == NEON_CACHE_ST_I64)) {
                if(cache->neoncache[j].n == cache->combined1)
                    a = j;
                else if(cache->neoncache[j].n == cache->combined2)
                    b = j;
            }
        if(a!=-1 && b!=-1) {
            int tmp = cache->neoncache[a].n;
            cache->neoncache[a].n = cache->neoncache[b].n;
            cache->neoncache[b].n = tmp;
        }
        // done
        cache->swapped = 0;
        cache->combined1 = cache->combined2 = 0;
    }
    if(cache->news) {
        // remove the newly created neoncache
        for(int i=0; i<32; ++i)
            if(cache->news&(1<<i))
                cache->neoncache[i].v = 0;
        cache->news = 0;
    }
    if(cache->stack_push) {
        // unpush
        for(int j=0; j<24; ++j) {
            if((cache->neoncache[j].t == NEON_CACHE_ST_D || cache->neoncache[j].t == NEON_CACHE_ST_F || cache->neoncache[j].t == NEON_CACHE_ST_I64)) {
                if(cache->neoncache[j].n<cache->stack_push)
                    cache->neoncache[j].v = 0;
                else
                    cache->neoncache[j].n-=cache->stack_push;
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
    // And now, rebuild the x87cache info with neoncache
    cache->mmxcount = 0;
    cache->fpu_scratch = 0;
    for(int i=0; i<8; ++i) {
        cache->x87cache[i] = -1;
        cache->mmxcache[i] = -1;
        cache->x87reg[i] = 0;
        cache->ssecache[i*2].v = -1;
        cache->ssecache[i*2+1].v = -1;
    }
    int x87reg = 0;
    for(int i=0; i<32; ++i) {
        if(cache->neoncache[i].v) {
            cache->fpuused[i] = 1;
            switch (cache->neoncache[i].t) {
                case NEON_CACHE_MM:
                    cache->mmxcache[cache->neoncache[i].n] = i;
                    ++cache->mmxcount;
                    break;
                case NEON_CACHE_XMMR:
                case NEON_CACHE_XMMW:
                    cache->ssecache[cache->neoncache[i].n].reg = i;
                    cache->ssecache[cache->neoncache[i].n].write = (cache->neoncache[i].t==NEON_CACHE_XMMW)?1:0;
                    break;
                case NEON_CACHE_YMMR:
                case NEON_CACHE_YMMW:
                    cache->fpuused[i] = 0;  // YMM does not mark the fpu reg as used
                    break;
                case NEON_CACHE_ST_F:
                case NEON_CACHE_ST_D:
                case NEON_CACHE_ST_I64:
                    cache->x87cache[x87reg] = cache->neoncache[i].n;
                    cache->x87reg[x87reg] = i;
                    ++x87reg;
                    break;
                case NEON_CACHE_SCR:
                    cache->fpuused[i] = 0;
                    cache->neoncache[i].v = 0;
                    break;
            }
        } else {
            cache->fpuused[i] = 0;
        }
    }
    // add back removed XMM
    if(cache->xmm_removed) {
        for(int i=0; i<16; ++i)
            if(cache->xmm_removed&(1<<i)) {
                int reg = (i<8)?(XMM0+i):(XMM8+i-8);
                cache->neoncache[reg].t = (cache->xmm_write&(1<<i))?NEON_CACHE_XMMW:NEON_CACHE_XMMR;
                cache->neoncache[reg].n = i;
                cache->ssecache[i].reg = reg;
                cache->ssecache[i].write = (cache->xmm_write&(1<<i))?1:0;
            }
        cache->xmm_write = cache->xmm_removed = 0;
    }
        // add back removed YMM
    if(cache->ymm_removed) {
        for(int i=0; i<16; ++i)
            if(cache->ymm_removed&(1<<i)) {
                int reg = cache->ymm_regs>>(i*4)&15;
                if(reg>7)
                    reg = reg - 8 + SCRATCH0;
                else
                    reg = reg + EMM0;
                //if(cache->neoncache[reg].v)   // this is normal when a ymm is purged to make space for another one
                //    printf_log(LOG_INFO, "Warning, recreating YMM%d on non empty slot %s", i, getCacheName(cache->neoncache[reg].t, cache->neoncache[reg].n));
                cache->neoncache[reg].t = (cache->ymm_write&(1<<i))?NEON_CACHE_YMMW:NEON_CACHE_YMMR;
                cache->neoncache[reg].n = i;
            }
        cache->ymm_regs = 0;
        cache->ymm_write = cache->ymm_removed = 0;
    }
    cache->ymm_used = 0;
}

#define F8      *(uint8_t*)(addr++)
#define F32S64  (uint64_t)(int64_t)*(int32_t*)(addr+=4, addr-4)
// Get if ED will have the correct parity. Not emitting anything. Parity is 2 for DWORD or 3 for QWORD
int getedparity(dynarec_arm_t* dyn, int ninst, uintptr_t addr, uint8_t nextop, int parity, int delta)
{
    (void)dyn; (void)ninst;

    uint32_t tested = (1<<parity)-1;
    if((nextop&0xC0)==0xC0)
        return 0;   // direct register, no parity...
    if(!(nextop&0xC0)) {
        if((nextop&7)==4) {
            uint8_t sib = F8;
            int sib_reg = (sib>>3)&7;
            if((sib&0x7)==5) {
                uint64_t tmp = F32S64;
                if (sib_reg!=4) {
                    // if XXXXXX+reg<<N then check parity of XXXXX and N should be enough
                    return ((tmp&tested)==0 && (sib>>6)>=parity)?1:0;
                } else {
                    // just a constant...
                    return (tmp&tested)?0:1;
                }
            } else {
                if(sib_reg==4 && parity<3)
                    return 0;   // simple [reg]
                // don't try [reg1 + reg2<<N], unless reg1 is ESP
                return ((sib&0x7)==4 && (sib>>6)>=parity)?1:0;
            }
        } else if((nextop&7)==5) {
            uint64_t tmp = F32S64;
            tmp+=addr+delta;
            return (tmp&tested)?0:1;
        } else {
            return 0;
        }
    } else {
        return 0; //Form [reg1 + reg2<<N + XXXXXX]
    }
}
#undef F8
#undef F32S64

const char* getCacheName(int t, int n)
{
    static char buff[20];
    switch(t) {
        case NEON_CACHE_ST_D: sprintf(buff, "ST%d", n); break;
        case NEON_CACHE_ST_F: sprintf(buff, "st%d", n); break;
        case NEON_CACHE_ST_I64: sprintf(buff, "STi%d", n); break;
        case NEON_CACHE_MM: sprintf(buff, "MM%d", n); break;
        case NEON_CACHE_XMMW: sprintf(buff, "XMM%d", n); break;
        case NEON_CACHE_XMMR: sprintf(buff, "xmm%d", n); break;
        case NEON_CACHE_YMMW: sprintf(buff, "YMM%d", n); break;
        case NEON_CACHE_YMMR: sprintf(buff, "ymm%d", n); break;
        case NEON_CACHE_SCR: sprintf(buff, "Scratch"); break;
        case NEON_CACHE_NONE: buff[0]='\0'; break;
    }
    return buff;
}

static register_mapping_t register_mappings[] = {
    { "rax", "x10" },
    { "eax", "w10" },
    { "ax", "x10" },
    { "ah", "x10" },
    { "al", "x10" },
    { "rcx", "x11" },
    { "ecx", "w11" },
    { "cx", "x11" },
    { "ch", "x11" },
    { "cl", "x11" },
    { "rdx", "x12" },
    { "edx", "w12" },
    { "dx", "x12" },
    { "dh", "x12" },
    { "dl", "x12" },
    { "rbx", "x13" },
    { "ebx", "w13" },
    { "bx", "x13" },
    { "bh", "x13" },
    { "bl", "x13" },
    { "rsi", "x14" },
    { "esi", "w14" },
    { "si", "x14" },
    { "sil", "x14" },
    { "rdi", "x15" },
    { "edi", "w15" },
    { "di", "x15" },
    { "dil", "x15" },
    { "rsp", "x16" },
    { "esp", "w16" },
    { "sp", "x16" },
    { "spl", "x16" },
    { "rbp", "x17" },
    { "ebp", "w17" },
    { "bp", "x17" },
    { "bpl", "x17" },
    { "r8", "x18" },
    { "r8d", "w18" },
    { "r8w", "x18" },
    { "r8b", "x18" },
    { "r9", "x19" },
    { "r9d", "w19" },
    { "r9w", "x19" },
    { "r9b", "x19" },
    { "r10", "x20" },
    { "r10d", "w20" },
    { "r10w", "x20" },
    { "r10b", "x20" },
    { "r11", "x21" },
    { "r11d", "w21" },
    { "r11w", "x21" },
    { "r11b", "x21" },
    { "r12", "x22" },
    { "r12d", "w22" },
    { "r12w", "x22" },
    { "r12b", "x22" },
    { "r13", "x23" },
    { "r13d", "w23" },
    { "r13w", "x23" },
    { "r13b", "x23" },
    { "r14", "x24" },
    { "r14d", "w24" },
    { "r14w", "x24" },
    { "r14b", "x24" },
    { "r15", "x25" },
    { "r15d", "w25" },
    { "r15w", "x25" },
    { "r15b", "x25" },
    { "rip", "x27" },
};

void printf_x64_instruction(dynarec_native_t* dyn, zydis_dec_t* dec, instruction_x64_t* inst, const char* name);
void inst_name_pass3(dynarec_native_t* dyn, int ninst, const char* name, rex_t rex)
{
    if (!dyn->need_dump && !BOX64ENV(dynarec_gdbjit) && !BOX64ENV(dynarec_perf_map)) {
        /*zydis_dec_t* dec = rex.is32bits ? my_context->dec32 : my_context->dec;
        if(dec && !OpcodeOK(dec, dyn->insts[ninst].x64.addr) && !strstr(name, "Illegal")) {
            uint8_t* p = (uint8_t*)dyn->insts[ninst].x64.addr;
            printf_log(LOG_INFO, "Warning: %p invalid opcode %02x %02x %02x %02x %02x %02x treated as valid\n", p, p[0], p[1], p[2], p[3], p[4], p[5]);
        }*/
        return;
    }

    static char buf[4096];
    int length = sprintf(buf, "barrier=%d state=%d/%d/%d(%d:%d->%d:%d), %s=%X/%X, use=%X, need=%X/%X, sm=%d(%d/%d/%d)",
        dyn->insts[ninst].x64.barrier,
        dyn->insts[ninst].x64.state_flags,
        dyn->f.pending,
        dyn->f.dfnone,
        dyn->insts[ninst].f_entry.pending,
        dyn->insts[ninst].f_entry.dfnone,
        dyn->insts[ninst].f_exit.pending,
        dyn->insts[ninst].f_exit.dfnone,
        dyn->insts[ninst].x64.may_set ? "may" : "set",
        dyn->insts[ninst].x64.set_flags,
        dyn->insts[ninst].x64.gen_flags,
        dyn->insts[ninst].x64.use_flags,
        dyn->insts[ninst].x64.need_before,
        dyn->insts[ninst].x64.need_after,
        dyn->smwrite, dyn->insts[ninst].will_write, dyn->insts[ninst].last_write, dyn->insts[ninst].lock);
    if (dyn->insts[ninst].nat_flags_op) {
        if (dyn->insts[ninst].nat_flags_op == NAT_FLAG_OP_TOUCH && dyn->insts[ninst].before_nat_flags)
            length += sprintf(buf + length, " NF:%d/read:%x", dyn->insts[ninst].nat_flags_op, dyn->insts[ninst].before_nat_flags);
        else
            length += sprintf(buf + length, " NF:%d", dyn->insts[ninst].nat_flags_op);
    }
    if (dyn->insts[ninst].use_nat_flags || dyn->insts[ninst].set_nat_flags || dyn->insts[ninst].need_nat_flags) {
        length += sprintf(buf + length, " nf:%hhx/%hhx/%hhx", dyn->insts[ninst].set_nat_flags, dyn->insts[ninst].use_nat_flags, dyn->insts[ninst].need_nat_flags);
    }
    if (dyn->insts[ninst].invert_carry)
        length += sprintf(buf + length, " CI");
    if (dyn->insts[ninst].gen_inverted_carry)
        length += sprintf(buf + length, " gic");
    if (dyn->insts[ninst].before_nat_flags & NF_CF) {
        length += sprintf(buf + length, " %ccb", dyn->insts[ninst].normal_carry_before ? 'n' : 'i');
    }
    if (dyn->insts[ninst].need_nat_flags & NF_CF) {
        length += sprintf(buf + length, " %cc", dyn->insts[ninst].normal_carry ? 'n' : 'i');
    }
    if (dyn->insts[ninst].pred_sz) {
        length += sprintf(buf + length, ", pred=");
        for (int ii = 0; ii < dyn->insts[ninst].pred_sz; ++ii)
            length += sprintf(buf + length, "%s%d", ii ? "/" : "", dyn->insts[ninst].pred[ii]);
    }
    if (!dyn->insts[ninst].x64.alive)
        length += sprintf(buf + length, "not executed");
    if (dyn->insts[ninst].x64.jmp && dyn->insts[ninst].x64.jmp_insts >= 0) {
        length += sprintf(buf + length, ", jmp=%d", dyn->insts[ninst].x64.jmp_insts);
    }
    if (dyn->insts[ninst].x64.jmp && dyn->insts[ninst].x64.jmp_insts == -1)
        length += sprintf(buf + length, ", jmp=out");
    if (dyn->insts[ninst].x64.has_callret)
        length += sprintf(buf + length, ", callret");
    if (dyn->last_ip) {
        length += sprintf(buf + length, ", last_ip=%p", (void*)dyn->last_ip);
    }
    for (int ii = 0; ii < 32; ++ii) {
        switch (dyn->insts[ninst].n.neoncache[ii].t) {
            case NEON_CACHE_ST_D: length += sprintf(buf + length, " D%d:%s", ii, getCacheName(dyn->insts[ninst].n.neoncache[ii].t, dyn->insts[ninst].n.neoncache[ii].n)); break;
            case NEON_CACHE_ST_F: length += sprintf(buf + length, " S%d:%s", ii, getCacheName(dyn->insts[ninst].n.neoncache[ii].t, dyn->insts[ninst].n.neoncache[ii].n)); break;
            case NEON_CACHE_ST_I64: length += sprintf(buf + length, " D%d:%s", ii, getCacheName(dyn->insts[ninst].n.neoncache[ii].t, dyn->insts[ninst].n.neoncache[ii].n)); break;
            case NEON_CACHE_MM: length += sprintf(buf + length, " D%d:%s", ii, getCacheName(dyn->insts[ninst].n.neoncache[ii].t, dyn->insts[ninst].n.neoncache[ii].n)); break;
            case NEON_CACHE_XMMW: length += sprintf(buf + length, " Q%d:%s", ii, getCacheName(dyn->insts[ninst].n.neoncache[ii].t, dyn->insts[ninst].n.neoncache[ii].n)); break;
            case NEON_CACHE_XMMR: length += sprintf(buf + length, " Q%d:%s", ii, getCacheName(dyn->insts[ninst].n.neoncache[ii].t, dyn->insts[ninst].n.neoncache[ii].n)); break;
            case NEON_CACHE_YMMW: length += sprintf(buf + length, " Q%d:%s", ii, getCacheName(dyn->insts[ninst].n.neoncache[ii].t, dyn->insts[ninst].n.neoncache[ii].n)); break;
            case NEON_CACHE_YMMR: length += sprintf(buf + length, " Q%d:%s", ii, getCacheName(dyn->insts[ninst].n.neoncache[ii].t, dyn->insts[ninst].n.neoncache[ii].n)); break;
            // case NEON_CACHE_SCR: length += sprintf(buf + length, " D%d:%s", ii, getCacheName(dyn->insts[ninst].n.neoncache[ii].t, dyn->insts[ninst].n.neoncache[ii].n)); break;
            case NEON_CACHE_NONE:
            default: break;
        }
    }
    if (memcmp(dyn->insts[ninst].n.neoncache, dyn->n.neoncache, sizeof(dyn->n.neoncache))) {
        length += sprintf(buf + length, " %s(Change:", (dyn->need_dump > 1) ? "\e[1;91m" : "");
        for (int ii = 0; ii < 32; ++ii)
            if (dyn->insts[ninst].n.neoncache[ii].v != dyn->n.neoncache[ii].v) {
                length += sprintf(buf + length, " V%d:%s", ii, getCacheName(dyn->n.neoncache[ii].t, dyn->n.neoncache[ii].n));
                length += sprintf(buf + length, "->%s", getCacheName(dyn->insts[ninst].n.neoncache[ii].t, dyn->insts[ninst].n.neoncache[ii].n));
            }
        length += sprintf(buf + length, ")%s", (dyn->need_dump > 1) ? "\e[0;32m" : "");
    }
    if (dyn->insts[ninst].n.xmm_used || dyn->insts[ninst].n.xmm_unneeded) {
        length += sprintf(buf + length, " xmmUsed=%04x/unneeded=%04x", dyn->insts[ninst].n.xmm_used, dyn->insts[ninst].n.xmm_unneeded);
    }
    if (dyn->insts[ninst].n.ymm_used || dyn->insts[ninst].n.ymm_unneeded) {
        length += sprintf(buf + length, " ymmUsed=%04x/unneeded=%04x", dyn->insts[ninst].n.ymm_used, dyn->insts[ninst].n.ymm_unneeded);
    }
    if (dyn->ymm_zero || dyn->insts[ninst].ymm0_add || dyn->insts[ninst].ymm0_sub || dyn->insts[ninst].ymm0_out) {
        length += sprintf(buf + length, " ymm0=(%04x/%04x+%04x-%04x=%04x)", dyn->ymm_zero, dyn->insts[ninst].ymm0_in, dyn->insts[ninst].ymm0_add, dyn->insts[ninst].ymm0_sub, dyn->insts[ninst].ymm0_out);
    }
    if (dyn->insts[ninst].purge_ymm) {
        length += sprintf(buf + length, " purgeYmm=%04x", dyn->insts[ninst].purge_ymm);
    }
    if (dyn->insts[ninst].preload_xmmymm) {
        length += sprintf(buf + length, " preload=%x/%d", dyn->insts[ninst].preload_xmmymm, dyn->insts[ninst].preload_from);
    }
    if (dyn->n.stack || dyn->insts[ninst].n.stack_next || dyn->insts[ninst].n.x87stack) {
        length += sprintf(buf + length, " X87:%d/%d(+%d/-%d)%d", dyn->n.stack, dyn->insts[ninst].n.stack_next, dyn->insts[ninst].n.stack_push, dyn->insts[ninst].n.stack_pop, dyn->insts[ninst].n.x87stack);
    }
    if (dyn->insts[ninst].n.combined1 || dyn->insts[ninst].n.combined2) {
        length += sprintf(buf + length, " %s:%d/%d", dyn->insts[ninst].n.swapped ? "SWP" : "CMB", dyn->insts[ninst].n.combined1, dyn->insts[ninst].n.combined2);
    }
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
    if(length>sizeof(buf)) printf_log(LOG_NONE, "Warning: buf to small in inst_name_pass3 (%d vs %zd)\n", length, sizeof(buf));
}

void print_opcode(dynarec_native_t* dyn, int ninst, uint32_t opcode)
{
    dynarec_log_prefix(0, LOG_NONE, "\t%08x\t%s\n", opcode, arm64_print(opcode, (uintptr_t)dyn->block));
}

static void x87_reset(neoncache_t* n)
{
    for (int i=0; i<8; ++i)
        n->x87cache[i] = -1;
    n->tags = 0;
    n->x87stack = 0;
    n->stack = 0;
    n->stack_next = 0;
    n->stack_pop = 0;
    n->stack_push = 0;
    n->combined1 = n->combined2 = 0;
    n->swapped = 0;
    n->barrier = 0;
    n->pushed = 0;
    n->poped = 0;

    for(int i=0; i<24; ++i)
        if(n->neoncache[i].t == NEON_CACHE_ST_F
         || n->neoncache[i].t == NEON_CACHE_ST_D
         || n->neoncache[i].t == NEON_CACHE_ST_I64)
            n->neoncache[i].v = 0;
}

static void mmx_reset(neoncache_t* n)
{
    n->mmxcount = 0;
    for (int i=0; i<8; ++i)
        n->mmxcache[i] = -1;
}

static void sse_reset(neoncache_t* n)
{
    for (int i=0; i<16; ++i)
        n->ssecache[i].v = -1;
    for (int i=0; i<32; ++i)
        if(n->neoncache[i].t==NEON_CACHE_YMMR || n->neoncache[i].t==NEON_CACHE_YMMW)
            n->neoncache[i].v = 0;
}

void fpu_reset(dynarec_native_t* dyn)
{
    x87_reset(&dyn->n);
    mmx_reset(&dyn->n);
    sse_reset(&dyn->n);
    fpu_reset_reg(dyn);
    dyn->ymm_zero = 0;
}

void fpu_reset_ninst(dynarec_native_t* dyn, int ninst)
{
    x87_reset(&dyn->insts[ninst].n);
    mmx_reset(&dyn->insts[ninst].n);
    sse_reset(&dyn->insts[ninst].n);
    fpu_reset_reg_neoncache(&dyn->insts[ninst].n);

}

int fpu_is_st_freed(dynarec_native_t* dyn, int ninst, int st)
{
    return (dyn->n.tags&(0b11<<(st*2)))?1:0;
}


uint8_t mark_natflag(dynarec_arm_t* dyn, int ninst, uint8_t flag, int before)
{
    if(dyn->insts[ninst].x64.set_flags && !before) {
        dyn->insts[ninst].set_nat_flags |= flag;
        //if(dyn->insts[ninst].x64.use_flags) {
        //    dyn->insts[ninst].use_nat_flags |= flag;
        //}
    } else {
        if(before)
            dyn->insts[ninst].use_nat_flags_before |= flag;
        else
            dyn->insts[ninst].use_nat_flags |= flag;
    }
    return flag;
}

uint8_t flag2native(uint8_t flags)
{
    uint8_t ret = 0;
    #ifdef ARM64
    if(flags&X_ZF) ret|=NF_EQ;
    if(flags&X_SF) ret|=NF_SF;
    if(flags&X_OF) ret|=NF_VF;
    if(flags&X_CF) ret|=NF_CF;
    #else
    // no native flags on rv64 or la64
    #endif
    return ret;
}

int flagIsNative(uint8_t flags)
{
    if(flags&(X_AF|X_PF)) return 0;
    return 1;
}

static uint8_t getNativeFlagsUsed(dynarec_arm_t* dyn, int start, uint8_t flags)
{
    // propagate and check wich flags are actually used
    uint8_t used_flags = 0;
    int nat_flags_used = 0;
    int ninst = start;
    while(ninst<dyn->size) {
//printf_log(LOG_INFO, "getNativeFlagsUsed ninst:%d/%d, flags=%x, used_flags=%x(%d), nat_flags_op_before:%x, nat_flags_op:%x, need_after:%x set_nat_flags:%x nat_flags_used:%x(%x)\n", ninst, start, flags, used_flags, nat_flags_used, dyn->insts[ninst].nat_flags_op_before, dyn->insts[ninst].nat_flags_op, flag2native(dyn->insts[ninst].x64.need_after),dyn->insts[ninst].set_nat_flags, dyn->insts[ninst].use_nat_flags, dyn->insts[ninst].use_nat_flags_before);
        // check if this is an opcode that generate flags but consume flags before
        if(dyn->insts[ninst].nat_flags_op_before)
            return 0;
        // check if nat flags are used "before"
        if(dyn->insts[ninst].use_nat_flags_before) {
            // check if the gen flags are compatible
            if(dyn->insts[ninst].use_nat_flags_before&~flags)
                return 0;
            nat_flags_used = 1;
            used_flags|=dyn->insts[ninst].use_nat_flags_before&flags;
        }
        // if the opcode generate flags, return
        if(dyn->insts[ninst].nat_flags_op==NAT_FLAG_OP_TOUCH && (start!=ninst)) {
            if(!nat_flags_used)
                return 0;
            if(used_flags&~dyn->insts[ninst].set_nat_flags) {
                // check partial changes that would destroy flag state
                if(dyn->insts[ninst].use_nat_flags_before&flags)
                    return used_flags;
                // check if flags are all refreshed, then it's ok
                if((used_flags&dyn->insts[ninst].set_nat_flags)==used_flags)
                    return used_flags;
                // incompatible
                return 0;
            }
            return used_flags;
        }
        // check if there is a callret barrier
        if(dyn->insts[ninst].x64.has_callret)
            return 0;
        // check if nat flags are used
        if(dyn->insts[ninst].use_nat_flags) {
            // check if the gen flags are compatible
            if(dyn->insts[ninst].use_nat_flags&~flags)
                return 0;
            nat_flags_used = 1;
            used_flags  |= dyn->insts[ninst].use_nat_flags&flags;
        }
        if(ninst!=start && dyn->insts[ninst].x64.use_flags) {
            // some flags not compatible with native, partial use not allowed
            if(flag2native(dyn->insts[ninst].x64.use_flags)!=dyn->insts[ninst].use_nat_flags)
                return 0;
            // check if flags are used, but not the natives ones
            //if(dyn->insts[ninst].use_nat_flags&~used_flags)
            //    return 0;
        }
        // check if flags are generated without native option
        if((start!=ninst) && dyn->insts[ninst].x64.gen_flags && (flag2native(dyn->insts[ninst].x64.gen_flags&dyn->insts[ninst].x64.need_after)&used_flags)) {
            if(used_flags&~flag2native(dyn->insts[ninst].x64.gen_flags&dyn->insts[ninst].x64.need_after))
                return 0;   // partial covert, not supported for now (TODO: this might be fixable)
            else
                return nat_flags_used?used_flags:0;  // full covert... End of propagation
        }
        // check if flags are still needed
        if(!(flag2native(dyn->insts[ninst].x64.need_after)&flags))
            return nat_flags_used?used_flags:0;
        // check if flags are destroyed, cancel the use then
        if(dyn->insts[ninst].nat_flags_op && (start!=ninst))
            return 0;
        // update used flags
        //used_flags |= (flag2native(dyn->insts[ninst].x64.need_after)&flags);

        // go next
        if(!dyn->insts[ninst].x64.has_next) {
            // check if it's a jump to an opcode with only 1 preds, then just follow the jump
            int jmp = dyn->insts[ninst].x64.jmp_insts;
            if(dyn->insts[ninst].x64.jmp && (jmp!=-1) && (getNominalPred(dyn, jmp)==ninst))
                ninst = jmp;
            else
                return nat_flags_used?used_flags:0;
        } else
            ++ninst;
    }
    return nat_flags_used?used_flags:0;
}

static void propagateNativeFlags(dynarec_arm_t* dyn, int start)
{
    int ninst = start;
    // those are the flags generated by the opcode and used later on
    uint8_t flags = dyn->insts[ninst].set_nat_flags&flag2native(dyn->insts[ninst].x64.need_after);
    //check if they are actualy used before starting
//printf_log(LOG_INFO, "propagateNativeFlags called for start=%d, flags=%x, will need:%x\n", start, flags, flag2native(dyn->insts[ninst].x64.need_after));
    if(!flags) return;
    // also check if some native flags are used but not genereated here
    if(flag2native(dyn->insts[ninst].x64.use_flags)&~flags) return;
    uint8_t used_flags = getNativeFlagsUsed(dyn, start, flags);
//printf_log(LOG_INFO, " will use:%x, carry:%d, generate inverted carry:%d\n", used_flags, used_flags&NF_CF, dyn->insts[ninst].gen_inverted_carry);
    if(!used_flags) return; // the flags wont be used, so just cancel
    int nc = dyn->insts[ninst].gen_inverted_carry?0:1;
    int carry = used_flags&NF_CF;
    // propagate
    while(ninst<dyn->size) {
        // check if this is an opcode that generate flags but consume flags before
        if((start!=ninst) && dyn->insts[ninst].nat_flags_op==NAT_FLAG_OP_TOUCH) {
            if(dyn->insts[ninst].use_nat_flags_before) {
                dyn->insts[ninst].before_nat_flags |= used_flags;
                if(carry) dyn->insts[ninst].normal_carry_before = nc;
            }
            // if the opcode generate flags, return
            return;
        }
        // check if flags are generated without native option
        if((start!=ninst) && dyn->insts[ninst].x64.gen_flags && (flag2native(dyn->insts[ninst].x64.gen_flags&dyn->insts[ninst].x64.need_after)&used_flags))
            return;
        // mark the opcode
        uint8_t use_flags = flag2native(dyn->insts[ninst].x64.need_before|dyn->insts[ninst].x64.need_after);
        if(dyn->insts[ninst].x64.use_flags) use_flags |= flag2native(dyn->insts[ninst].x64.use_flags);  // should not change anything
//printf_log(LOG_INFO, " marking ninst=%d with %x | %x&%x => %x\n", ninst, dyn->insts[ninst].need_nat_flags, used_flags, use_flags, dyn->insts[ninst].need_nat_flags | (used_flags&use_flags));
        dyn->insts[ninst].need_nat_flags |= used_flags&use_flags;
        if(carry) dyn->insts[ninst].normal_carry = nc;
        if(carry && dyn->insts[ninst].invert_carry) nc = 0;
        // check if flags are still needed
        if(!(flag2native(dyn->insts[ninst].x64.need_after)&used_flags))
            return;
        // go next
        if(!dyn->insts[ninst].x64.has_next) {
            // check if it's a jump to an opcode with only 1 preds, then just follow the jump
            int jmp = dyn->insts[ninst].x64.jmp_insts;
            if(dyn->insts[ninst].x64.jmp && (jmp!=-1) && (getNominalPred(dyn, jmp)==ninst))
                ninst = jmp;
            else
                return;
        } else
            ++ninst;
    }
}

void updateNativeFlags(dynarec_native_t* dyn)
{
    if(!BOX64ENV(dynarec_nativeflags))
        return;
    // forward check if native flags are used
    for(int ninst=0; ninst<dyn->size; ++ninst)
        if(flag2native(dyn->insts[ninst].x64.gen_flags) && (dyn->insts[ninst].nat_flags_op==NAT_FLAG_OP_TOUCH)) {
            propagateNativeFlags(dyn, ninst);
        }
}

void rasNativeState(dynarec_arm_t* dyn, int ninst)
{
    dyn->insts[ninst].nat_flags_op = dyn->insts[ninst].set_nat_flags = dyn->insts[ninst].use_nat_flags = dyn->insts[ninst].need_nat_flags = 0;
    dyn->insts[ninst].ymm0_in = dyn->insts[ninst].ymm0_sub = dyn->insts[ninst].ymm0_add = dyn->insts[ninst].ymm0_out = dyn->insts[ninst].purge_ymm = 0;
}

int nativeFlagsNeedsTransform(dynarec_arm_t* dyn, int ninst)
{
    int jmp = dyn->insts[ninst].x64.jmp_insts;
    if(jmp<0)
        return 0;
    if(!dyn->insts[ninst].x64.need_after || !dyn->insts[jmp].x64.need_before)
        return 0;
    if(dyn->insts[ninst].set_nat_flags)
        return 0;
    uint8_t flags_before = dyn->insts[ninst].need_nat_flags;
    uint8_t nc_before = dyn->insts[ninst].normal_carry;
    if(dyn->insts[ninst].invert_carry)
        nc_before = 0;
    uint8_t flags_after = dyn->insts[jmp].need_nat_flags;
    uint8_t nc_after = dyn->insts[jmp].normal_carry;
    if(dyn->insts[jmp].nat_flags_op==NAT_FLAG_OP_TOUCH) {
        flags_after = dyn->insts[jmp].before_nat_flags;
        nc_after = dyn->insts[jmp].normal_carry_before;
    }
    uint8_t flags_x86 = flag2native(dyn->insts[jmp].x64.need_before);
    flags_x86 &= ~flags_after;
    if((flags_before&NF_CF) && (flags_after&NF_CF) && (nc_before!=nc_after))
        return 1;
    // all flags_after should be present and none remaining flags_x86
    if(((flags_before&flags_after)!=flags_after) || (flags_before&flags_x86))
        return 1;
    return 0;
}

void fpu_save_and_unwind(dynarec_arm_t* dyn, int ninst, neoncache_t* cache)
{
    memcpy(cache, &dyn->insts[ninst].n, sizeof(neoncache_t));
    neoncacheUnwind(&dyn->insts[ninst].n);
}
void fpu_unwind_restore(dynarec_arm_t* dyn, int ninst, neoncache_t* cache)
{
    memcpy(&dyn->insts[ninst].n, cache, sizeof(neoncache_t));
}

static uint32_t getXYMMMask(dynarec_arm_t* dyn, int ninst)
{
    if(ninst<0) return 0;
    uint32_t ret = 0;
    for (int i=0; i<32; ++i) {
        switch(dyn->insts[ninst].n.neoncache[i].t) {
            case NEON_CACHE_XMMR:
            case NEON_CACHE_XMMW:
                ret |= 1<<dyn->insts[ninst].n.neoncache[i].n;
                break;
            case NEON_CACHE_YMMR:
            case NEON_CACHE_YMMW:
                ret |= 1<<(16+dyn->insts[ninst].n.neoncache[i].n);
                break;
        }
    }
    return ret;
}

static void propagateXYMMUneeded(dynarec_arm_t* dyn, int ninst, uint16_t mask_x, uint16_t mask_y)
{
    if(!ninst) return;
    ninst = getNominalPred(dyn, ninst);
    while(ninst>=0) {
        mask_x &= ~dyn->insts[ninst].n.xmm_used;
        mask_y &= ~dyn->insts[ninst].n.ymm_used;
        if(!mask_x && !mask_y) return; // used, value is needed
        if(dyn->insts[ninst].x64.barrier&BARRIER_FLOAT) return; // barrier, value is needed
        mask_x &= ~dyn->insts[ninst].n.xmm_unneeded;
        mask_y &= ~dyn->insts[ninst].n.ymm_unneeded;
        if(!mask_x && !mask_y) return; // already handled
        if(dyn->insts[ninst].x64.jmp) return;   // stop when a jump is detected, that gets too complicated
        dyn->insts[ninst].n.xmm_unneeded |= mask_x; // flags
        dyn->insts[ninst].n.ymm_unneeded |= mask_y; // flags
        ninst = getNominalPred(dyn, ninst); // continue
    }
}

void updateUneeded(dynarec_arm_t* dyn)
{
    if(!dyn->use_xmm && !dyn->use_ymm)
        return;
    for(int ninst=0; ninst<dyn->size; ++ninst) {
        if(dyn->insts[ninst].n.xmm_unneeded || dyn->insts[ninst].n.ymm_unneeded)
            propagateXYMMUneeded(dyn, ninst, dyn->insts[ninst].n.xmm_unneeded, dyn->insts[ninst].n.ymm_unneeded);
    }
    // try to add some preload of XYMM on jump were it would make sense
    for(int ninst=0; ninst<dyn->size; ++ninst)
        if((ninst && dyn->insts[ninst].pred_sz>1) || (!ninst && dyn->insts[ninst].pred_sz)) {
            int i2 = dyn->size+1;
            for(int j=0; j<dyn->insts[ninst].pred_sz; ++j)
                if(dyn->insts[ninst].pred[j]!=ninst-1 && dyn->insts[ninst].pred[j]<i2)
                    i2 = dyn->insts[ninst].pred[j];
            if(i2!=dyn->size+1 && i2>ninst) {
                uint32_t used = getXYMMMask(dyn, i2);
                if(used) {
                    int prev = getNominalPred(dyn, ninst);
                    if((prev==ninst-1) || !ninst) {
                        uint32_t dest = getXYMMMask(dyn, prev);
                        // removed unneeded
                        uint32_t unnedded = dyn->insts[ninst].n.xmm_unneeded | (dyn->insts[ninst].n.ymm_unneeded<<16);
                        // create the preload mask
                        uint32_t preload = (used & ~dest) & ~unnedded;
                        if(preload)
                            addSSEPreload(dyn, i2, ninst, preload);
                    }
                }
            }
        }
}

void tryEarlyFpuBarrier(dynarec_arm_t* dyn, int last_fpu_used, int ninst)
{
    // there is a barrier at ninst
    // check if, up to last fpu_used, if there is some suspicious jump that would prevent the barrier to be put earlier
    int usefull = 0;
    for(int i=ninst-1; i>last_fpu_used; --i)
    {
        if(!dyn->insts[i].x64.has_next)
            return; // break of chain, don't try to be smart for now
        if(dyn->insts[i].x64.barrier&BARRIER_FLOAT)
            return; // already done?
        if(dyn->insts[i].x64.jmp && dyn->insts[i].x64.jmp_insts==-1)
            usefull = 1;
        if(dyn->insts[i].x64.jmp && dyn->insts[i].x64.jmp_insts!=-1) {
            int i2 = dyn->insts[i].x64.jmp_insts;
            if(i2<last_fpu_used || i2>ninst) {
                // check if some xmm/ymm/x87 stack are used in landing point
                if(i2>ninst) {
                    if(dyn->insts[i2].n.xmm_used || dyn->insts[i2].n.ymm_used || dyn->insts[i2].n.stack)
                        return;
                }
                // we will stop there, not trying to guess too much thing
                if((usefull && (i+1)!=ninst)) {
                    if(BOX64ENV(dynarec_dump) || BOX64ENV(dynarec_log)>1) dynarec_log(LOG_NONE, "Putting early Float Barrier in %d for %d\n", i+1, ninst);
                    dyn->insts[i+1].x64.barrier|=BARRIER_FLOAT;
                }
                return;
            }
            usefull = 1;
        }
        for(int pred=0; pred<dyn->insts[i].pred_sz; ++pred) {
            if(dyn->insts[i].pred[pred]<=last_fpu_used) {
                if(usefull && ((i+1)!=ninst)) {
                    if(BOX64ENV(dynarec_dump) || BOX64ENV(dynarec_log)>1) dynarec_log(LOG_NONE, "Putting early Float Barrier in %d for %d\n", i+1, ninst);
                    dyn->insts[i+1].x64.barrier|=BARRIER_FLOAT;
                }
                return;
            }
        }
        if(dyn->insts[i].pred_sz>1)
            usefull = 1;
    }
    if(usefull) {
        if(BOX64ENV(dynarec_dump) || BOX64ENV(dynarec_log)>1) dynarec_log(LOG_NONE, "Putting early Float Barrier in %d for %d\n", last_fpu_used, ninst);
        dyn->insts[last_fpu_used+1].x64.barrier|=BARRIER_FLOAT;
    }
}

void propagateFpuBarrier(dynarec_arm_t* dyn)
{
    if(!dyn->use_x87)
        return;
    int last_fpu_used = -1;
    for(int ninst=0; ninst<dyn->size; ++ninst) {
        int fpu_used = dyn->insts[ninst].n.xmm_used || dyn->insts[ninst].n.ymm_used || dyn->insts[ninst].mmx_used || dyn->insts[ninst].x87_used;
        if(fpu_used) last_fpu_used = ninst;
        dyn->insts[ninst].fpu_used = fpu_used;
        if(dyn->insts[ninst].fpupurge && (last_fpu_used!=-1) && (last_fpu_used!=(ninst-1))) {
            tryEarlyFpuBarrier(dyn, last_fpu_used, ninst);
            last_fpu_used = -1;  // reset the last_fpu_used...
        }
    }
}


void updateYmm0s(dynarec_arm_t* dyn, int ninst, int max_ninst_reached)
{
    if(!dyn->use_ymm)
        return;
    int can_incr = ninst == max_ninst_reached; // Are we the top-level call?
    int ok = 1;
    while ((can_incr || ok) && ninst < dyn->size) {
        // if(dyn->need_dump) dynarec_log(LOG_NONE, "update ninst=%d (%d): can_incr=%d\n", ninst, max_ninst_reached, can_incr);
        uint16_t new_purge_ymm, new_ymm0_in, new_ymm0_out;

        if (dyn->insts[ninst].pred_sz && dyn->insts[ninst].x64.alive) {
            // The union of the empty set is empty (0), the intersection is the universe (-1)
            // The first instruction is the entry point, which has a virtual pred with ymm0_out = 0
            // Similarly, float barriers reset ymm0s
            uint16_t ymm0_union = 0;
            uint16_t ymm0_inter = (ninst && !(dyn->insts[ninst].x64.barrier & BARRIER_FLOAT)) ? ((uint16_t)-1) : (uint16_t)0;
            for (int i = 0; i < dyn->insts[ninst].pred_sz; ++i) {
                int pred = dyn->insts[ninst].pred[i];
                // if(dyn->need_dump) dynarec_log(LOG_NONE, "\twith pred[%d] = %d", i, pred);
                if (pred >= max_ninst_reached) {
                    // if(dyn->need_dump) dynarec_log(LOG_NONE, " (skipped)\n");
                    continue;
                }

                int pred_out = dyn->insts[pred].x64.has_callret ? 0 : dyn->insts[pred].ymm0_out;
                // if(dyn->need_dump) dynarec_log(LOG_NONE, " ~> %04X\n", pred_out);
                ymm0_union |= pred_out;
                ymm0_inter &= pred_out;
            }
            // if(dyn->need_dump) dynarec_log(LOG_NONE, "\t=> %04X,%04X\n", ymm0_union, ymm0_inter);
            //  Notice the default values yield something coherent here (if all pred are after ninst)
            new_purge_ymm = ymm0_union & ~ymm0_inter;
            new_ymm0_in = ymm0_inter;
            new_ymm0_out = (ymm0_inter | dyn->insts[ninst].ymm0_add) & ~dyn->insts[ninst].ymm0_sub;

            if ((dyn->insts[ninst].purge_ymm != new_purge_ymm) || (dyn->insts[ninst].ymm0_in != new_ymm0_in) || (dyn->insts[ninst].ymm0_out != new_ymm0_out)) {
                // Need to update self and next(s)
                dyn->insts[ninst].purge_ymm = new_purge_ymm;
                dyn->insts[ninst].ymm0_in = new_ymm0_in;
                dyn->insts[ninst].ymm0_out = new_ymm0_out;

                if (can_incr) {
                    // We always have ninst == max_ninst_reached when can_incr == 1
                    ++max_ninst_reached;
                } else {
                    // We need to stop here if the opcode has no "real" next or if we reached the ninst of the toplevel
                    ok = (max_ninst_reached - 1 != ninst) && dyn->insts[ninst].x64.has_next && !dyn->insts[ninst].x64.has_callret;
                }

                int jmp = (dyn->insts[ninst].x64.jmp) ? dyn->insts[ninst].x64.jmp_insts : -1;
                if ((jmp != -1) && (jmp < max_ninst_reached)) {
                    // if(dyn->need_dump) dynarec_log(LOG_NONE, "\t! jump to %d\n", jmp);
                    //  The jump goes before the last instruction reached, update the destination
                    //  If this is the top level call, this means the jump goes backward (jmp != ninst)
                    //  Otherwise, since we don't update all instructions, we may miss the update (don't use jmp < ninst)
                    updateYmm0s(dyn, jmp, max_ninst_reached);
                }
            } else {
                if (can_incr) {
                    // We always have ninst == max_ninst_reached when can_incr == 1
                    ++max_ninst_reached;

                    // Also update jumps to before (they are skipped otherwise)
                    int jmp = (dyn->insts[ninst].x64.jmp) ? dyn->insts[ninst].x64.jmp_insts : -1;
                    if ((jmp != -1) && (jmp < max_ninst_reached)) {
                        // if(dyn->need_dump) dynarec_log(LOG_NONE, "\t! jump to %d\n", jmp);
                        updateYmm0s(dyn, jmp, max_ninst_reached);
                    }
                } else {
                    // We didn't update anything, we can leave
                    ok = 0;
                }
            }
        } else if (can_incr) {
            // We always have ninst == max_ninst_reached when can_incr == 1
            ++max_ninst_reached;
        } else {
            // We didn't update anything, we can leave
            ok = 0;
        }
        ++ninst;
    }
}


// AVX helpers
void avx_mark_zero(dynarec_arm_t* dyn, int ninst, int reg)
{
    dyn->ymm_zero |= (1<<reg);
}

int is_avx_zero(dynarec_arm_t* dyn, int ninst, int reg)
{
    return (dyn->ymm_zero>>reg)&1;
}

int is_avx_zero_unset(dynarec_arm_t* dyn, int ninst, int reg)
{
    if((dyn->ymm_zero>>reg)&1) {
        dyn->ymm_zero &= ~(1<<reg);
        return 1;
    }
    return 0;
}

void avx_mark_zero_reset(dynarec_arm_t* dyn, int ninst)
{
    dyn->ymm_zero = 0;
}

static int xmm_preload_reg(dynarec_arm_t* dyn, int ninst, int last, int xmm)
{
    return (xmm>7)?(XMM8 + xmm - 8):(XMM0 + xmm);
}

static int ymm_preload_reg(dynarec_arm_t* dyn, int ninst, int last, int ymm)
{
    int i = -1;
    // search for when it will be loaded the first time
    int start = ninst;
    for(int ii=0; ii<32 && i==-1; ++ii)
        if(dyn->insts[last].n.neoncache[ii].n==ymm && (dyn->insts[last].n.neoncache[ii].t==NEON_CACHE_YMMR || dyn->insts[last].n.neoncache[ii].t==NEON_CACHE_YMMW))
            i = ii;
    while((ninst<last) && (i!=-1)) {
        // check if the reg is always free
        if(!(!dyn->insts[ninst].n.neoncache[i].v ||
            (dyn->insts[ninst].n.neoncache[i].n==ymm 
                && (dyn->insts[ninst].n.neoncache[i].t==NEON_CACHE_YMMR || dyn->insts[ninst].n.neoncache[i].t==NEON_CACHE_YMMW))))
            i = -1; // nope
        else
            ++ninst;
    }
    return i;
}

void addSSEPreload(dynarec_arm_t* dyn, int ninst, int i2, uint32_t preload)
{
    if((i2<0) || (i2>ninst) || !preload || dyn->insts[i2].preload_from)
        return;
    if(i2 && !dyn->insts[i2-1].x64.has_next)    // need a previous opcode to set the preload, unless it's the #0
        return;
    // check if there is jump out or outside this loop
    for(int i=i2; i<ninst; ++i) {
        if(!dyn->insts[i].x64.has_next || (dyn->insts[i].x64.barrier&BARRIER_FLOAT))
            return;
        // for now, unneeded stop at first jump, so this should also stop there
        if(dyn->insts[i].x64.jmp/* && ((dyn->insts[i].x64.jmp_insts>ninst) || (dyn->insts[i].x64.jmp_insts<i2))*/)
            return;
    }
    uint16_t ymm_mask = 0;
    for(int i=0; i<16; ++i) {
        if(preload&(1<<i)) {
            int v = xmm_preload_reg(dyn, i2?(i2-1):0, ninst, i);
            // propagate until not needed
            int i3 = i2;
            while(i3<ninst) {
                if(dyn->insts[i3].n.xmm_used&(1<<i))
                    i3 = ninst;
                else {
                    dyn->insts[i3].n.ssecache[i].write = 0;
                    dyn->insts[i3].n.ssecache[i].reg = v;
                    dyn->insts[i3].n.neoncache[v].t = NEON_CACHE_XMMR;
                    dyn->insts[i3].n.neoncache[v].n = i;
                    ++i3;
                }
            }
        }
        if(preload&(1<<(16+i))) {
            int v = ymm_preload_reg(dyn, i2?(i2-1):0, ninst, i);
            if(v!=-1) {
                ymm_mask |= 1<<i;
                // propagate until not needed
                int i3 = i2;
                while(i3<ninst) {
                    if(dyn->insts[i3].n.ymm_used&(1<<i))
                        i3 = ninst;
                    else {
                        dyn->insts[i3].n.neoncache[v].t = NEON_CACHE_YMMR;
                        dyn->insts[i3].n.neoncache[v].n = i;
                        ++i3;
                    }
                }
            }
        }
    }
    preload = (preload&0xffff) | (ymm_mask<<16);    // update ymm_mask
    //TODO: should check native calls too?
    dyn->insts[i2].preload_xmmymm = preload;
    dyn->insts[i2].preload_from = ninst;
    dyn->insts[i2].n.xmm_unneeded |= preload&0xffff;
    dyn->insts[i2].n.ymm_unneeded |= ymm_mask;
}