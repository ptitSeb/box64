#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include <errno.h>

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
#include "../tools/bridge_private.h"

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_functions.h"
#include "dynarec_rv64_helper.h"

// emit CMP8 instruction, from cmp s1, s2, using s3, s4, s5 and s6 as scratch
void emit_cmp8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5, int s6) {
    CLEAR_FLAGS();
    IFX_PENDOR0 {
        SB(s1, xEmu, offsetof(x64emu_t, op1));
        SB(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, d_cmp8);
    } else {
        SET_DFNONE(s4);
    }

    IFX(X_AF | X_CF | X_OF) {
        // for later flag calculation
        NOT(s5, s1);
    }

    // It's a cmp, we can't store the result back to s1.
    SUB(s6, s1, s2);
    ANDI(s6, s6, 0xff);
    IFX_PENDOR0 {
        SB(s6, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_SF) {
        SRLI(s3, s6, 7);
        BEQZ(s3, 4);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    CALC_SUB_FLAGS(s5, s2, s6, s3, s4, 8);
    IFX(X_ZF) {
        BEQZ(s6, 4);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s6, s3, s4);
    }
}

// emit CMP8 instruction, from cmp s1 , 0, using s3 and s4 as scratch
void emit_cmp8_0(dynarec_rv64_t* dyn, int ninst, int s1, int s3, int s4) {
    CLEAR_FLAGS();
    IFX_PENDOR0 {
        SB(s1, xEmu, offsetof(x64emu_t, op1));
        SB(xZR, xEmu, offsetof(x64emu_t, op2));
        SB(s1, xEmu, offsetof(x64emu_t, res));
        SET_DF(s3, d_cmp8);
    } else {
        SET_DFNONE(s4);
    }

    IFX(X_SF) {
        SRLI(s3, s1, 7);
        BEQZ(s3, 4);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX(X_ZF) {
        BEQZ(s1, 4);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit CMP32 instruction, from cmp s1, s2, using s3 and s4 as scratch
void emit_cmp32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5, int s6)
{
    CLEAR_FLAGS();
    IFX_PENDOR0 {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SDxw(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, rex.w?d_cmp64:d_cmp32);
    } else {
        SET_DFNONE(s4);
    }

    IFX(X_AF | X_CF | X_OF) {
        // for later flag calculation
        NOT(s5, s1);
    }

    // It's a cmp, we can't store the result back to s1.
    SUBxw(s6, s1, s2);
    IFX_PENDOR0 {
        SDxw(s6, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_SF) {
        BGE(s6, xZR, 0);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    CALC_SUB_FLAGS(s5, s2, s6, s3, s4, rex.w?64:32);
    IFX(X_ZF) {
        BEQZ(s6, 4);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s6, s3, s4);
    }
}

// emit TEST32 instruction, from test s1, s2, using s3 and s4 as scratch
void emit_test32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    CLEAR_FLAGS();
    IFX_PENDOR0 {
        SET_DF(s3, rex.w?d_tst64:d_tst32);
    } else {
        SET_DFNONE(s4);
    }

    AND(s3, s1, s2); // res = s1 & s2

    IFX_PENDOR0 {
        SDxw(s3, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_SF) {
        if (!rex.w) ZEROUP(s3);
        SRLI(s4, s3, rex.w?63:31);
        BEQZ(s4, 4);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX(X_ZF) {
        BNEZ(s3, 4);
        ORI(xFlags, xFlags, F_ZF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s3, s4, s5);
    }
}
