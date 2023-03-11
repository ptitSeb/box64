#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include <errno.h>
#include <assert.h>

#include "debug.h"
#include "box64context.h"
#include "dynarec.h"
#include "emu/x64emu_private.h"
#include "emu/x64run_private.h"
#include "x64run.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"
#include "../dynablock_private.h"
#include "../tools/bridge_private.h"
#include "custommem.h"

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_functions.h"
#include "dynarec_rv64_helper.h"


void jump_to_epilog(dynarec_rv64_t* dyn, uintptr_t ip, int reg, int ninst)
{
    MAYUSE(dyn); MAYUSE(ip); MAYUSE(ninst);
    MESSAGE(LOG_DUMP, "Jump to epilog\n");

    if(reg) {
        if(reg!=xRIP) {
            MV(xRIP, reg);
        }
    } else {
        GETIP_(ip);
    }
    TABLE64(x2, (uintptr_t)rv64_epilog);
    SMEND();
    BR(x2);
}

void jump_to_next(dynarec_rv64_t* dyn, uintptr_t ip, int reg, int ninst)
{
    MAYUSE(dyn); MAYUSE(ninst);
    MESSAGE(LOG_DUMP, "Jump to next\n");

    if(reg) {
        if(reg!=xRIP) {
            MV(xRIP, reg);
        }
        uintptr_t tbl = getJumpTable64();
        MAYUSE(tbl);
        TABLE64(x3, tbl);
        SRLI(x2, xRIP, JMPTABL_START3);
        SLLI(x2, x2, 3);
        LD_I12(x3, x3, x2);
        LUI(x4, JMPTABLE_MASK2);    // x4 = mask
        SRLI(x2, xRIP, JMPTABL_START2);
        AND(x2, x2, x4);
        SLLI(x2, x2, 3);
        LD_I12(x3, x3, x2);
        if(JMPTABLE_MASK2!=JMPTABLE_MASK1) {
            LUI(x4, JMPTABLE_MASK1);    // x4 = mask
        }
        SRLI(x2, xRIP, JMPTABL_START1);
        AND(x2, x2, x4);
        SLLI(x2, x2, 3);
        LD_I12(x3, x3, x2);
        if(JMPTABLE_MASK1!=JMPTABLE_MASK0) {
            LUI(x4, JMPTABLE_MASK0);    // x4 = mask
        }
        AND(x2, x2, x4);
        SLLI(x2, x2, 3);
        LD_I12(x2, x3, x2);
    } else {
        uintptr_t p = getJumpTableAddress64(ip);
        MAYUSE(p);
        TABLE64(x3, p);
        GETIP_(ip);
        LD_I12(x2, x3, 0);
    }
    if(reg!=A1) {
        MV(A1, xRIP);
    }
    CLEARIP();
    #ifdef HAVE_TRACE
    //MOVx(x3, 15);    no access to PC reg 
    #endif
    SMEND();
    JALR(x2); // save LR...
}

void fpu_reset(dynarec_rv64_t* dyn)
{
    //TODO
}

void fpu_reset_cache(dynarec_rv64_t* dyn, int ninst, int reset_n)
{
    //TODO
}

void fpu_purgecache(dynarec_rv64_t* dyn, int ninst, int next, int s1, int s2, int s3)
{
    //TODO
}

// propagate ST stack state, especial stack pop that are defered
void fpu_propagate_stack(dynarec_rv64_t* dyn, int ninst)
{
    //TODO
}

void mmx_purgecache(dynarec_rv64_t* dyn, int ninst, int next, int s1)
{
    // TODO
}

void x87_purgecache(dynarec_rv64_t* dyn, int ninst, int next, int s1, int s2, int s3)
{
    //TODO
}

#ifdef HAVE_TRACE
void fpu_reflectcache(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3)
{
    //TODO
}
#endif
void fpu_pushcache(dynarec_rv64_t* dyn, int ninst, int s1, int not07)
{
    //TODO
}
void fpu_popcache(dynarec_rv64_t* dyn, int ninst, int s1, int not07)
{
    //TODO
}

void rv64_move64(dynarec_rv64_t* dyn, int ninst, int reg, int64_t val)
{
    if((val&0b111111111111)==val) {
        // simple 12bit value
        MOV_U12(reg, (val&0b111111111111));
        return;
    }
    if(((val>>32)<<32)==val) {
        // 32bits value
        LUI(reg, (val>>12));
        ORI(reg, reg, (val&0b111111111111));
        return;
    }
    //TODO: optimize that latter
    LUI(reg, (val>>44));
    ORI(reg, reg, ((val>>32)&0b111111111111));
    SLLI(reg, reg, 12);
    ORI(reg, reg, ((val>>20)&0b111111111111));
    SLLI(reg, reg, 12);
    ORI(reg, reg, ((val>>8)&0b111111111111));
    SLLI(reg, reg, 8);
    ORI(reg, reg, (val&0b111111111111));
}