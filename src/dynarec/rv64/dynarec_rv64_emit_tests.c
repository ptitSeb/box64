#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
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

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_functions.h"
#include "dynarec_rv64_helper.h"

// emit CMP8 instruction, from cmp s1, s2, using s3, s4, s5 and s6 as scratch
void emit_cmp8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5, int s6)
{
    CLEAR_FLAGS();
    IFX_PENDOR0 {
        SB(s1, xEmu, offsetof(x64emu_t, op1));
        SB(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, d_cmp8);
    } else {
        SET_DFNONE();
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
        BEQZ(s3, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    CALC_SUB_FLAGS(s5, s2, s6, s3, s4, 8);
    IFX(X_ZF) {
        BNEZ(s6, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s6, s3, s4);
    }
}

// emit CMP8 instruction, from cmp s1 , 0, using s3 and s4 as scratch
void emit_cmp8_0(dynarec_rv64_t* dyn, int ninst, int s1, int s3, int s4)
{
    CLEAR_FLAGS();
    IFX_PENDOR0 {
        SB(s1, xEmu, offsetof(x64emu_t, op1));
        SB(xZR, xEmu, offsetof(x64emu_t, op2));
        SB(s1, xEmu, offsetof(x64emu_t, res));
        SET_DF(s3, d_cmp8);
    } else {
        SET_DFNONE();
    }

    IFX(X_SF) {
        SRLI(s3, s1, 7);
        BEQZ(s3, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX(X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit CMP16 instruction, from cmp s1, s2, using s3 and s4 as scratch
void emit_cmp16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5, int s6)
{
    CLEAR_FLAGS();
    IFX_PENDOR0 {
        SH(s1, xEmu, offsetof(x64emu_t, op1));
        SH(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, d_cmp16);
    } else {
        SET_DFNONE();
    }

    IFX(X_AF | X_CF | X_OF) {
        // for later flag calculation
        NOT(s5, s1);
    }

    // It's a cmp, we can't store the result back to s1.
    SUB(s6, s1, s2);
    IFX(X_ALL) {
        ZEXTH(s6, s6);
    }
    IFX_PENDOR0 {
        SH(s6, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_SF) {
        SRLI(s3, s6, 15);
        BEQZ(s3, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    CALC_SUB_FLAGS(s5, s2, s6, s3, s4, 16);
    IFX(X_ZF) {
        BNEZ(s6, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s6, s3, s4);
    }
}

// emit CMP16 instruction, from cmp s1 , #0, using s3 and s4 as scratch
void emit_cmp16_0(dynarec_rv64_t* dyn, int ninst, int s1, int s3, int s4)
{
    CLEAR_FLAGS();
    IFX_PENDOR0 {
        SH(s1, xEmu, offsetof(x64emu_t, op1));
        SH(xZR, xEmu, offsetof(x64emu_t, op2));
        SH(s1, xEmu, offsetof(x64emu_t, res));
        SET_DF(s3, d_cmp16);
    } else {
        SET_DFNONE();
    }

    IFX(X_SF) {
        SRLI(s3, s1, 15);
        BEQZ(s3, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX(X_ZF) {
        BNEZ(s1, 8);
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
        SET_DFNONE();
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
        BGE(s6, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s6);
    }
    CALC_SUB_FLAGS(s5, s2, s6, s3, s4, rex.w?64:32);
    IFX(X_ZF) {
        BNEZ(s6, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s6, s3, s4);
    }
}

// emit CMP32 instruction, from cmp s1, 0, using s3 and s4 as scratch
void emit_cmp32_0(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s3, int s4)
{
    CLEAR_FLAGS();
    IFX_PENDOR0 {
        SD(s1, xEmu, offsetof(x64emu_t, op1));
        SD(xZR, xEmu, offsetof(x64emu_t, op2));
        SD(s1, xEmu, offsetof(x64emu_t, res));
        SET_DF(s4, rex.w?d_cmp64:d_cmp32);
    } else {
        SET_DFNONE();
    }
    IFX(X_SF) {
        if (rex.w) {
            BGE(s1, xZR, 8);
        } else {
            SRLI(s3, s1, 31);
            BEQZ(s3, 8);
        }
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX(X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit TEST8 instruction, from test s1, s2, using s3, s4 and s5 as scratch
void emit_test8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5) {
    CLEAR_FLAGS();
    IFX_PENDOR0 {
        SET_DF(s3, d_tst8);
    } else {
        SET_DFNONE();
    }

    AND(s3, s1, s2); // res = s1 & s2

    IFX_PENDOR0 {
        SD(s3, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_SF) {
        SRLI(s4, s3, 7);
        BEQZ(s4, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX(X_ZF) {
        BNEZ(s3, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s3, s4, s5);
    }
}

// emit TEST16 instruction, from test s1, s2, using s3, s4 and s5 as scratch
void emit_test16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    CLEAR_FLAGS();
    IFX_PENDOR0 {
        SET_DF(s3, d_tst16);
    } else {
        SET_DFNONE();
    }

    AND(s3, s1, s2); // res = s1 & s2

    IFX_PENDOR0 {
        SH(s3, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_SF) {
        SRLI(s4, s3, 15);
        BEQZ(s4, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX(X_ZF) {
        BNEZ(s3, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s3, s4, s5);
    }
}

// emit TEST32 instruction, from test s1, s2, using s3 and s4 as scratch
void emit_test32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    CLEAR_FLAGS();
    IFX_PENDOR0 {
        SET_DF(s3, rex.w?d_tst64:d_tst32);
    } else {
        SET_DFNONE();
    }

    AND(s3, s1, s2); // res = s1 & s2

    IFX_PENDOR0 {
        SDxw(s3, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_SF|X_ZF) {
        if (!rex.w) ZEROUP(s3);
    }
    IFX(X_SF) {
        SRLI(s4, s3, rex.w?63:31);
        BEQZ(s4, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX(X_ZF) {
        BNEZ(s3, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s3, s4, s5);
    }
}

// emit TEST32 instruction, from test s1, s2, using s3 and s4 as scratch
void emit_test32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4, int s5)
{
    CLEAR_FLAGS();
    IFX_PENDOR0 {
        SET_DF(s3, rex.w?d_tst64:d_tst32);
    } else {
        SET_DFNONE();
    }

    if(c>=-2048 && c<=2047) {
        ANDI(s3, s1, c);
        IFX(X_SF|X_ZF) {
            if (!rex.w && c<0) ZEROUP(s3);
        }
    } else {
        MOV64xw(s3, c);
        AND(s3, s1, s3); // res = s1 & s2
    }

    IFX_PENDOR0 {
        SDxw(s3, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_SF) {
        SRLI(s4, s3, rex.w?63:31);
        BEQZ(s4, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX(X_ZF) {
        BNEZ(s3, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s3, s4, s5);
    }
}
