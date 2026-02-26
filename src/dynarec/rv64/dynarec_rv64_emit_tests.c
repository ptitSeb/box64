#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_functions.h"
#include "../dynarec_helper.h"

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

    IFX (X_AF | X_CF | X_OF) {
        // for later flag calculation
        NOT(s5, s1);
    }

    IFX_PENDOR0 {
        SUB(s6, s1, s2);
        SB(s6, xEmu, offsetof(x64emu_t, res));
    } else IFX (X_ALL) {
        SUB(s6, s1, s2);
    }
    IFX (X_ALL) ANDI(s6, s6, 0xff);

    IFX (X_SF) {
        SRLI(s3, s6, 7);
        SET_FLAGS_NEZ(s3, F_SF, s4);
    }

    CALC_SUB_FLAGS(s5, s2, s6, s3, s4, 8);
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s6, F_ZF, s4);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s6, s3, s4);
    }

    NAT_FLAGS_ENABLE_CARRY();
    NAT_FLAGS_ENABLE_SIGN();
    if (dyn->insts[ninst].nat_flags_fusion) {
        if (dyn->insts[ninst].nat_flags_needsign) {
            SLLI(s3, s1, 56);
            SRAI(s3, s3, 56);
            SLLI(s4, s2, 56);
            SRAI(s4, s4, 56);
            NAT_FLAGS_OPS(s3, s4, s5, xZR);
        } else {
            NAT_FLAGS_OPS(s1, s2, s3, xZR);
        }
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

    IFX (X_SF) {
        SRLI(s3, s1, 7);
        SET_FLAGS_NEZ(s3, F_SF, s4);
    }
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s4);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    NAT_FLAGS_ENABLE_CARRY();
    NAT_FLAGS_ENABLE_SIGN();
    if (dyn->insts[ninst].nat_flags_fusion) {
        if (dyn->insts[ninst].nat_flags_needsign) {
            SLLI(s3, s1, 56);
            SRAI(s3, s3, 56);
            NAT_FLAGS_OPS(s3, xZR, s4, xZR);
        } else {
            NAT_FLAGS_OPS(s1, xZR, s3, xZR);
        }
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

    IFX (X_AF | X_CF | X_OF) {
        // for later flag calculation
        NOT(s5, s1);
    }

    IFX_PENDOR0 {
        SUB(s6, s1, s2);
        SH(s6, xEmu, offsetof(x64emu_t, res));
    } else IFX (X_ALL) {
        SUB(s6, s1, s2);
    }
    IFX (X_ALL) ZEXTH(s6, s6);

    IFX (X_SF) {
        SRLI(s3, s6, 15);
        SET_FLAGS_NEZ(s3, F_SF, s4);
    }

    CALC_SUB_FLAGS(s5, s2, s6, s3, s4, 16);
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s6, F_ZF, s4);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s6, s3, s4);
    }

    NAT_FLAGS_ENABLE_CARRY();
    NAT_FLAGS_ENABLE_SIGN();
    if (dyn->insts[ninst].nat_flags_fusion) {
        if (dyn->insts[ninst].nat_flags_needsign) {
            SLLI(s3, s1, 48);
            SRAI(s3, s3, 48);
            SLLI(s4, s2, 48);
            SRAI(s4, s4, 48);
            NAT_FLAGS_OPS(s3, s4, s5, xZR);
        } else {
            NAT_FLAGS_OPS(s1, s2, s3, xZR);
        }
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
    IFX (X_SF) {
        SRLI(s3, s1, 15);
        SET_FLAGS_NEZ(s3, F_SF, s4);
    }
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s4);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }

    NAT_FLAGS_ENABLE_CARRY();
    NAT_FLAGS_ENABLE_SIGN();
    if (dyn->insts[ninst].nat_flags_fusion) {
        if (dyn->insts[ninst].nat_flags_needsign) {
            SLLI(s3, s1, 48);
            SRAI(s3, s3, 48);
            NAT_FLAGS_OPS(s3, xZR, s4, xZR);
        } else {
            NAT_FLAGS_OPS(s1, xZR, s3, xZR);
        }
    }
}

// emit CMP32 instruction, from cmp s1, s2, using s3 and s4 as scratch
void emit_cmp32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5, int s6)
{
    CLEAR_FLAGS();
    IFX_PENDOR0 {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SDxw(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, rex.w ? d_cmp64 : d_cmp32);
    } else {
        SET_DFNONE();
    }

    IFX (X_AF | X_CF | X_OF) {
        // for later flag calculation
        NOT(s5, s1);
    }

    IFX_PENDOR0 {
        SUBxw(s6, s1, s2);
        SDxw(s6, xEmu, offsetof(x64emu_t, res));
    } else IFX (X_ALL) {
        SUBxw(s6, s1, s2);
    }

    IFX (X_SF) {
        SET_FLAGS_LTZ(s6, F_SF, s3, s4);
    }
    IFX (X_AF | X_CF | X_OF | X_ZF | X_PF) {
        if (!rex.w) ZEROUP(s6);
    }
    CALC_SUB_FLAGS(s5, s2, s6, s3, s4, rex.w ? 64 : 32);
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s6, F_ZF, s4);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s6, s3, s4);
    }
    NAT_FLAGS_ENABLE_CARRY();
    NAT_FLAGS_ENABLE_SIGN();
    if (dyn->insts[ninst].nat_flags_fusion) {
        if (rex.w) {
            NAT_FLAGS_OPS(s1, s2, s3, s4);
        } else {
            if (dyn->insts[ninst].nat_flags_needsign) {
                SEXT_W(s3, s1);
                SEXT_W(s4, s2);
            } else {
                ZEXTW2(s3, s1);
                ZEXTW2(s4, s2);
            }
            NAT_FLAGS_OPS(s3, s4, s5, xZR);
        }
    }
}

// emit CMP32 instruction, from cmp s1, 0, using s3 and s4 as scratch
void emit_cmp32_0(dynarec_rv64_t* dyn, int ninst, rex_t rex, uint8_t nextop, int s1, int s3, int s4, int s5)
{
    CLEAR_FLAGS();
    IFX_PENDOR0 {
        SD(s1, xEmu, offsetof(x64emu_t, op1));
        SD(xZR, xEmu, offsetof(x64emu_t, op2));
        SD(s1, xEmu, offsetof(x64emu_t, res));
        SET_DF(s4, rex.w ? d_cmp64 : d_cmp32);
    } else {
        SET_DFNONE();
    }

    IFX (X_SF) {
        if (rex.w) {
            SET_FLAGS_LTZ(s1, F_SF, s3, s4);
        } else {
            SRLIW(s3, s1, 31);
            SET_FLAGS_NEZ(s3, F_SF, s4);
        }
    }
    int res = s1;
    IFX (X_ZF | X_PF) {
        if (!rex.w && MODREG) {
            ZEXTW2(s5, s1);
            res = s5;
        }
    }
    IFX (X_ZF) {
        SET_FLAGS_EQZ(res, F_ZF, s3);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, res, s3, s4);
    }
    NAT_FLAGS_ENABLE_CARRY();
    NAT_FLAGS_ENABLE_SIGN();
    if (dyn->insts[ninst].nat_flags_fusion) {
        if (rex.w)
            NAT_FLAGS_OPS(s1, xZR, s3, xZR);
        else {
            if (dyn->insts[ninst].nat_flags_needsign) {
                SEXT_W(s3, s1);
                NAT_FLAGS_OPS(s3, xZR, s4, xZR);
            } else if (res == s5) { // zero-up'd case
                NAT_FLAGS_OPS(s5, xZR, s4, xZR);
            } else {
                ZEXTW2(s3, s1);
                NAT_FLAGS_OPS(s3, xZR, s4, xZR);
            }
        }
    }
}

// emit TEST8 instruction, from test s1, s2, using s3, s4 and s5 as scratch
void emit_test8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    CLEAR_FLAGS();
    IFX_PENDOR0 {
        SET_DF(s3, d_tst8);
    } else {
        SET_DFNONE();
    }

    if (s1 != s2) AND(s1, s1, s2); // res = s1 & s2

    IFX_PENDOR0 {
        SD(s1, xEmu, offsetof(x64emu_t, res));
    }

    IFX (X_SF) {
        SRLI(s4, s1, 7);
        SET_FLAGS_NEZ(s4, F_SF, s5);
    }
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s5);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s4, s5);
    }

    NAT_FLAGS_ENABLE_SIGN();
    if (dyn->insts[ninst].nat_flags_fusion && dyn->insts[ninst].nat_flags_needsign) {
        SLLI(s1, s1, 56);
        SRAI(s1, s1, 56);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit TEST8 instruction, from test s1, c, using s3, s4 and s5 as scratch
void emit_test8c(dynarec_rv64_t* dyn, int ninst, int s1, uint8_t c, int s3, int s4, int s5)
{
    CLEAR_FLAGS();
    IFX_PENDOR0 {
        SET_DF(s3, d_tst8);
    } else {
        SET_DFNONE();
    }

    ANDI(s3, s1, c); // res = s1 & c

    IFX_PENDOR0 {
        SD(s3, xEmu, offsetof(x64emu_t, res));
    }

    IFX (X_SF) {
        SRLI(s4, s3, 7);
        SET_FLAGS_NEZ(s4, F_SF, s5);
    }
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s3, F_ZF, s5);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s3, s4, s5);
    }

    NAT_FLAGS_ENABLE_SIGN();
    if (dyn->insts[ninst].nat_flags_fusion && dyn->insts[ninst].nat_flags_needsign) {
        SLLI(s3, s3, 56);
        SRAI(s3, s3, 56);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s3, xZR, xZR, xZR);
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

    IFX (X_SF) {
        SRLI(s4, s3, 15);
        SET_FLAGS_NEZ(s4, F_SF, s5);
    }
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s3, F_ZF, s5);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s3, s4, s5);
    }

    NAT_FLAGS_ENABLE_SIGN();
    if (dyn->insts[ninst].nat_flags_fusion && dyn->insts[ninst].nat_flags_needsign) {
        SLLI(s3, s3, 48);
        SRAI(s3, s3, 48);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s3, xZR, xZR, xZR);
}

// emit TEST32 instruction, from test s1, s2, using s3 and s4 as scratch
void emit_test32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    CLEAR_FLAGS();
    IFX_PENDOR0 {
        SET_DF(s3, rex.w ? d_tst64 : d_tst32);
    } else {
        SET_DFNONE();
    }

    AND(s3, s1, s2); // res = s1 & s2

    IFX_PENDOR0 {
        SDxw(s3, xEmu, offsetof(x64emu_t, res));
    }

    IFX (X_SF | X_ZF) {
        if (!rex.w) ZEROUP(s3);
    }

    IFX (X_SF) {
        SRLI(s4, s3, rex.w ? 63 : 31);
        SET_FLAGS_NEZ(s4, F_SF, s5);
    }
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s3, F_ZF, s5);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s3, s4, s5);
    }

    NAT_FLAGS_ENABLE_SIGN();
    if (dyn->insts[ninst].nat_flags_fusion && !rex.w) {
        if (dyn->insts[ninst].nat_flags_needsign) {
            SEXT_W(s3, s3);
        } else {
            IFX (X_SF | X_ZF) {
            } else
                ZEROUP(s3);
        }
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s3, xZR, xZR, xZR);
}

// emit TEST32 instruction, from test s1, s2, using s3 and s4 as scratch
void emit_test32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4, int s5)
{
    CLEAR_FLAGS();
    IFX_PENDOR0 {
        SET_DF(s3, rex.w ? d_tst64 : d_tst32);
    } else {
        SET_DFNONE();
    }

    if (c >= -2048 && c <= 2047) {
        ANDI(s3, s1, c);
    } else {
        MOV64xw(s3, c);
        AND(s3, s1, s3); // res = s1 & s2
    }

    IFX (X_SF | X_ZF) {
        if (!rex.w && c < 0) ZEROUP(s3);
    }

    IFX_PENDOR0 {
        SDxw(s3, xEmu, offsetof(x64emu_t, res));
    }

    IFX (X_SF) {
        SRLI(s4, s3, rex.w ? 63 : 31);
        SET_FLAGS_NEZ(s4, F_SF, s5);
    }
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s3, F_ZF, s5);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s3, s4, s5);
    }

    NAT_FLAGS_ENABLE_SIGN();
    if (dyn->insts[ninst].nat_flags_fusion && !rex.w) {
        if (dyn->insts[ninst].nat_flags_needsign) {
            SEXT_W(s3, s3);
        } else {
            IFX (X_SF | X_ZF) {
            } else if (c < 0)
                ZEROUP(s3);
        }
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s3, xZR, xZR, xZR);
}
