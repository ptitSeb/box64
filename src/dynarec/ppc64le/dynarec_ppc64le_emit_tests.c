#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "ppc64le_emitter.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"

#include "ppc64le_printer.h"
#include "dynarec_ppc64le_private.h"
#include "dynarec_ppc64le_functions.h"
#include "../dynarec_helper.h"


// emit CMP8 instruction, from cmp s1, s2, using s3, s4, s5 and s6 as scratch
void emit_cmp8(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5, int s6)
{
    IFX_PENDOR0 {
        STB(s1, offsetof(x64emu_t, op1), xEmu);
        STB(s2, offsetof(x64emu_t, op2), xEmu);
        SET_DF(s4, d_cmp8);
    } else {
        SET_DFNONE();
    }

    NAT_FLAGS_ENABLE_CARRY();
    NAT_FLAGS_ENABLE_SIGN();

    CLEAR_FLAGS(s3);
    IFX(X_AF | X_CF | X_OF) {
        // for later flag calculation
        NOT(s5, s1);
    }

    // It's a cmp, we can't store the result back to s1.
    SUB(s6, s1, s2);
    ANDId(s6, s6, 0xff);
    IFX_PENDOR0 {
        STB(s6, offsetof(x64emu_t, res), xEmu);
    }
    IFX(X_SF) {
        SRDI(s3, s6, 7);
        CMPDI(s3, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    CALC_SUB_FLAGS(s5, s2, s6, s3, s4, 8);
    IFX(X_ZF) {
        CMPDI(s6, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s6, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) {
        if (dyn->insts[ninst].nat_flags_needsign) {
            EXTSB(s3, s1);
            EXTSB(s4, s2);
            NAT_FLAGS_OPS(s3, s4, s5, xZR);
        } else {
            NAT_FLAGS_OPS(s1, s2, s3, xZR);
        }
    }
}

// emit CMP8 instruction, from cmp s1 , 0, using s3 and s4 as scratch
void emit_cmp8_0(dynarec_ppc64le_t* dyn, int ninst, int s1, int s3, int s4)
{
    IFX_PENDOR0 {
        STB(s1, offsetof(x64emu_t, op1), xEmu);
        LI(s3, 0);
        STB(s3, offsetof(x64emu_t, op2), xEmu);
        STB(s1, offsetof(x64emu_t, res), xEmu);
        SET_DF(s3, d_cmp8);
    } else {
        SET_DFNONE();
    }

    NAT_FLAGS_ENABLE_CARRY();
    NAT_FLAGS_ENABLE_SIGN();

    CLEAR_FLAGS(s3);
    IFX(X_SF) {
        SRDI(s3, s1, 7);
        CMPDI(s3, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX(X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) {
        if (dyn->insts[ninst].nat_flags_needsign) {
            EXTSB(s3, s1);
            NAT_FLAGS_OPS(s3, xZR, s4, xZR);
        } else {
            NAT_FLAGS_OPS(s1, xZR, s3, xZR);
        }
    }
}

// emit CMP16 instruction, from cmp s1, s2, using s3, s4, s5 and s6 as scratch
void emit_cmp16(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5, int s6)
{
    IFX_PENDOR0 {
        STH(s1, offsetof(x64emu_t, op1), xEmu);
        STH(s2, offsetof(x64emu_t, op2), xEmu);
        SET_DF(s4, d_cmp16);
    } else {
        SET_DFNONE();
    }

    NAT_FLAGS_ENABLE_CARRY();
    NAT_FLAGS_ENABLE_SIGN();

    CLEAR_FLAGS(s3);
    IFX(X_AF | X_CF | X_OF) {
        // for later flag calculation
        NOT(s5, s1);
    }

    // It's a cmp, we can't store the result back to s1.
    SUB(s6, s1, s2);
    IFX(X_ALL) {
        BF_EXTRACT(s6, s6, 15, 0);
    }
    IFX_PENDOR0 {
        STH(s6, offsetof(x64emu_t, res), xEmu);
    }
    IFX(X_SF) {
        SRDI(s3, s6, 15);
        CMPDI(s3, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    CALC_SUB_FLAGS(s5, s2, s6, s3, s4, 16);
    IFX(X_ZF) {
        CMPDI(s6, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s6, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) {
        if (dyn->insts[ninst].nat_flags_needsign) {
            EXTSH(s3, s1);
            EXTSH(s4, s2);
            NAT_FLAGS_OPS(s3, s4, s5, xZR);
        } else {
            NAT_FLAGS_OPS(s1, s2, s3, xZR);
        }
    }
}

// emit CMP16 instruction, from cmp s1 , #0, using s3 and s4 as scratch
void emit_cmp16_0(dynarec_ppc64le_t* dyn, int ninst, int s1, int s3, int s4)
{
    IFX_PENDOR0 {
        STH(s1, offsetof(x64emu_t, op1), xEmu);
        LI(s3, 0);
        STH(s3, offsetof(x64emu_t, op2), xEmu);
        STH(s1, offsetof(x64emu_t, res), xEmu);
        SET_DF(s3, d_cmp16);
    } else {
        SET_DFNONE();
    }

    NAT_FLAGS_ENABLE_CARRY();
    NAT_FLAGS_ENABLE_SIGN();

    CLEAR_FLAGS(s3);
    IFX(X_SF) {
        SRDI(s3, s1, 15);
        CMPDI(s3, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX(X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) {
        if (dyn->insts[ninst].nat_flags_needsign) {
            EXTSH(s3, s1);
            NAT_FLAGS_OPS(s3, xZR, s4, xZR);
        } else {
            NAT_FLAGS_OPS(s1, xZR, s3, xZR);
        }
    }
}

// emit CMP32 instruction, from cmp s1, s2, using s3, s4, s5 and s6 as scratch
void emit_cmp32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5, int s6)
{
    IFX_PENDOR0 {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SDxw(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, rex.w?d_cmp64:d_cmp32);
    } else {
        SET_DFNONE();
    }

    NAT_FLAGS_ENABLE_CARRY();
    NAT_FLAGS_ENABLE_SIGN();

    CLEAR_FLAGS(s3);
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
        // Check sign bit (bit 31 for 32-bit, bit 63 for 64-bit)
        // Can't use CMPDI after SUBxw because zero-extension makes result always >= 0
        SRDI(s3, s6, rex.w ? 63 : 31);
        CMPDI(s3, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s6);
    }
    CALC_SUB_FLAGS(s5, s2, s6, s3, s4, rex.w?64:32);
    IFX(X_ZF) {
        CMPDI(s6, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s6, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) {
        if (rex.w)
            NAT_FLAGS_OPS(s1, s2, s3, s4);
        else {
            if (dyn->insts[ninst].nat_flags_needsign) {
                SEXT_W(s3, s1);
                SEXT_W(s4, s2);
            } else {
                ZEROUP2(s3, s1);
                ZEROUP2(s4, s2);
            }
            NAT_FLAGS_OPS(s3, s4, s5, xZR);
        }
    }
}

// emit CMP32 instruction, from cmp s1, 0, using s3, s4 and s5 as scratch
void emit_cmp32_0(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, uint8_t nextop, int s1, int s3, int s4, int s5)
{
    IFX_PENDOR0 {
        STD(s1, offsetof(x64emu_t, op1), xEmu);
        LI(s3, 0);
        STD(s3, offsetof(x64emu_t, op2), xEmu);
        STD(s1, offsetof(x64emu_t, res), xEmu);
        SET_DF(s4, rex.w?d_cmp64:d_cmp32);
    } else {
        SET_DFNONE();
    }

    NAT_FLAGS_ENABLE_CARRY();
    NAT_FLAGS_ENABLE_SIGN();

    CLEAR_FLAGS(s3);
    IFX(X_SF) {
        if (rex.w) {
            CMPDI(s1, 0);
            BGE(8);
        } else {
            // Extract exactly bit 31 for SF — SRDI would be wrong if upper 32 bits are dirty
            BF_EXTRACT(s3, s1, 31, 31);
            CMPDI(s3, 0);
            BEQ(8);
        }
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    int res = s1;
    IFX (X_ZF | X_PF) {
        if (!rex.w && MODREG) {
            ZEROUP2(s5, s1);
            res = s5;
        }
    }
    IFX(X_ZF) {
        CMPDI(res, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, res, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) {
        if (rex.w)
            NAT_FLAGS_OPS(s1, xZR, s4, xZR);
        else {
            if (dyn->insts[ninst].nat_flags_needsign) {
                SEXT_W(s3, s1);
                NAT_FLAGS_OPS(s3, xZR, s4, xZR);
            } else if (res == s5) { // zero-up'd case
                NAT_FLAGS_OPS(res, xZR, s4, xZR);
            } else {
                ZEROUP2(s3, s1);
                NAT_FLAGS_OPS(s3, xZR, s4, xZR);
            }
        }
    }
}


// emit TEST8 instruction, from test s1, s2, using s3, s4 and s5 as scratch
void emit_test8(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    IFX_PENDOR0 {
        SET_DF(s3, d_tst8);
    } else {
        SET_DFNONE();
    }

    NAT_FLAGS_ENABLE_SIGN();

    CLEAR_FLAGS(s3);
    AND(s3, s1, s2); // res = s1 & s2

    IFX_PENDOR0 {
        STD(s3, offsetof(x64emu_t, res), xEmu);
    }
    IFX(X_SF) {
        SRDI(s4, s3, 7);
        CMPDI(s4, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX(X_ZF) {
        CMPDI(s3, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s3, s4, s5);
    }
    if (dyn->insts[ninst].nat_flags_fusion && dyn->insts[ninst].nat_flags_needsign) {
        EXTSB(s3, s3);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s3, xZR, s4, xZR);
}

// emit TEST8 instruction, from test s1, c, using s3, s4 and s5 as scratch
void emit_test8c(dynarec_ppc64le_t* dyn, int ninst, int s1, uint8_t c, int s3, int s4, int s5)
{
    IFX_PENDOR0 {
        SET_DF(s3, d_tst8);
    } else {
        SET_DFNONE();
    }

    NAT_FLAGS_ENABLE_SIGN();

    CLEAR_FLAGS(s3);
    ANDId(s3, s1, c); // res = s1 & c

    IFX_PENDOR0 {
        STD(s3, offsetof(x64emu_t, res), xEmu);
    }
    IFX (X_SF) {
        SRDI(s4, s3, 7);
        CMPDI(s4, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX (X_ZF) {
        CMPDI(s3, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s3, s4, s5);
    }
    if (dyn->insts[ninst].nat_flags_fusion && dyn->insts[ninst].nat_flags_needsign) {
        EXTSB(s3, s3);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s3, xZR, xZR, xZR);
}

// emit TEST16 instruction, from test s1, s2, using s3, s4 and s5 as scratch
void emit_test16(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    IFX_PENDOR0 {
        SET_DF(s3, d_tst16);
    } else {
        SET_DFNONE();
    }

    NAT_FLAGS_ENABLE_SIGN();

    CLEAR_FLAGS(s3);
    AND(s3, s1, s2); // res = s1 & s2

    IFX_PENDOR0 {
        STH(s3, offsetof(x64emu_t, res), xEmu);
    }

    IFX (X_SF) {
        SRDI(s4, s3, 15);
        CMPDI(s4, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX (X_ZF) {
        CMPDI(s3, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s3, s4, s5);
    }
    if (dyn->insts[ninst].nat_flags_fusion && dyn->insts[ninst].nat_flags_needsign) {
        EXTSH(s3, s3);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s3, xZR, xZR, xZR);
}

// emit TEST32 instruction, from test s1, s2, using s3, s4 and s5 as scratch
void emit_test32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    IFX_PENDOR0 {
        SET_DF(s3, rex.w?d_tst64:d_tst32);
    } else {
        SET_DFNONE();
    }

    NAT_FLAGS_ENABLE_SIGN();

    CLEAR_FLAGS(s3);
    AND(s3, s1, s2); // res = s1 & s2

    IFX_PENDOR0 {
        SDxw(s3, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_SF | X_ZF) {
        if (!rex.w) ZEROUP(s3);
    }
    IFX(X_SF) {
        SRDI(s4, s3, rex.w?63:31);
        CMPDI(s4, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX(X_ZF) {
        CMPDI(s3, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s3, s4, s5);
    }
    if (dyn->insts[ninst].nat_flags_fusion && !rex.w) {
        if (dyn->insts[ninst].nat_flags_needsign) {
            SEXT_W(s3, s3);
        } else {
            ZEROUP(s3);
        }
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s3, xZR, xZR, xZR);
}

// emit TEST32 instruction, from test s1, c, using s3, s4 and s5 as scratch
void emit_test32c(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4, int s5)
{
    IFX_PENDOR0 {
        SET_DF(s3, rex.w ? d_tst64 : d_tst32);
    } else {
        SET_DFNONE();
    }

    NAT_FLAGS_ENABLE_SIGN();

    CLEAR_FLAGS(s3);
    if (c >= 0 && c <= 65535) {
        ANDId(s3, s1, c);
    } else {
        MOV64xw(s3, c);
        AND(s3, s1, s3);
    }

    IFX_PENDOR0 {
        SDxw(s3, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_SF) {
        SRDI(s4, s3, rex.w ? 63 : 31);
        CMPDI(s4, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX(X_ZF) {
        CMPDI(s3, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s3, s4, s5);
    }
    if (dyn->insts[ninst].nat_flags_fusion && !rex.w) {
        if (dyn->insts[ninst].nat_flags_needsign) {
            SEXT_W(s3, s3);
        } else if (((uint64_t)c >> 32) != 0) {
            ZEROUP(s3);
        }
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s3, xZR, xZR, xZR);
}
