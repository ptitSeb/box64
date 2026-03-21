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

// emit ADD32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_add32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SDxw(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, rex.w ? d_add64 : d_add32b);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    CLEAR_FLAGS(s3);
    IFX (X_CF) {
        if (rex.w) {
            ZEROUP2(s5, s1);
            ZEROUP2(s4, s2);
            ADD(s5, s5, s4);
            SRDI(s3, s1, 0x20);
            SRDI(s4, s2, 0x20);
            ADD(s4, s4, s3);
            SRDI(s5, s5, 0x20);
            ADD(s5, s5, s4); // hi
            SRADI(s5, s5, 0x20);
            CMPDI(s5, 0);
            BEQ(8);
            ORI(xFlags, xFlags, 1 << F_CF);
        } else {
            ZEROUP2(s3, s1);
            ZEROUP2(s4, s2);
            ADD(s5, s3, s4);
            SRDI(s5, s5, 0x20);
            CMPDI(s5, 0);
            BEQ(8);
            ORI(xFlags, xFlags, 1 << F_CF);
        }
    }
    IFX (X_AF | X_OF) {
        OR(s3, s1, s2);  // s3 = op1 | op2
        AND(s4, s1, s2); // s4 = op1 & op2
    }

    ADDxw(s1, s1, s2);

    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_AF | X_OF) {
        ANDC(s3, s3, s1); // s3 = ~res & (op1 | op2)
        OR(s3, s3, s4);   // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX (X_AF) {
            ANDId(s4, s3, 0x08); // AF: cc & 0x08
            CMPDI(s4, 0);
            BEQ(8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX (X_OF) {
            SRDI(s3, s3, rex.w ? 62 : 30);
            SRDI(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDId(s3, s3, 1); // OF: xor of two MSB's of cc
            BF_INSERT(xFlags, s3, F_OF, F_OF);
        }
    }
    IFX (X_SF) {
        SRDI(s3, s1, rex.w ? 63 : 31);
        CMPDI(s3, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit ADD32 instruction, from s1, constant c, store result in s1 using s3 and s4 as scratch
void emit_add32c(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s2, int s3, int s4, int s5)
{
    if ((s1 == xRSP) && (BOX64DRENV(dynarec_safeflags) < 2) && (!dyn->insts || (dyn->insts[ninst].x64.gen_flags == X_PEND) || (!BOX64ENV(dynarec_df) && (dyn->insts[ninst].x64.gen_flags == X_ALL)))) {
        // special case when doing math on ESP and only PEND is needed: ignoring it!
        if (c >= -32768 && c < 32768) {
            ADDI(s1, s1, c);
        } else {
            MOV64x(s2, c);
            ADD(s1, s1, s2);
        }
        if (!rex.w) { ZEROUP(s1); }
        return;
    }
    IFX (X_PEND | X_AF | X_CF | X_OF) {
        MOV64xw(s2, c);
    } else if (c < -32768 || c >= 32768) {
        MOV64xw(s2, c);
    }
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SDxw(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, rex.w ? d_add64 : d_add32b);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    CLEAR_FLAGS(s3);
    IFX (X_CF) {
        if (rex.w) {
            ZEROUP2(s5, s1);
            ZEROUP2(s4, s2);
            ADD(s5, s5, s4);
            SRDI(s3, s1, 0x20);
            SRDI(s4, s2, 0x20);
            ADD(s4, s4, s3);
            SRDI(s5, s5, 0x20);
            ADD(s5, s5, s4); // hi
            SRADI(s5, s5, 0x20);
            CMPDI(s5, 0);
            BEQ(8);
            ORI(xFlags, xFlags, 1 << F_CF);
        } else {
            ZEROUP2(s3, s1);
            ZEROUP2(s4, s2);
            ADD(s5, s3, s4);
            SRDI(s5, s5, 0x20);
            CMPDI(s5, 0);
            BEQ(8);
            ORI(xFlags, xFlags, 1 << F_CF);
        }
    }
    IFX (X_AF | X_OF) {
        OR(s3, s1, s2);  // s3 = op1 | op2
        AND(s4, s1, s2); // s4 = op1 & op2
    }

    if (c >= -32768 && c < 32768) {
        ADDIxw(s1, s1, c);
    } else {
        ADDxw(s1, s1, s2);
    }

    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_AF | X_OF) {
        ANDC(s3, s3, s1); // s3 = ~res & (op1 | op2)
        OR(s3, s3, s4);   // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX (X_AF) {
            ANDId(s4, s3, 0x08); // AF: cc & 0x08
            CMPDI(s4, 0);
            BEQ(8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX (X_OF) {
            SRDI(s3, s3, rex.w ? 62 : 30);
            SRDI(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDId(s3, s3, 1); // OF: xor of two MSB's of cc
            BF_INSERT(xFlags, s3, F_OF, F_OF);
        }
    }
    IFX (X_SF) {
        SRDI(s3, s1, rex.w ? 63 : 31);
        CMPDI(s3, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit ADD8 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_add8(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX (X_PEND) {
        STB(s1, offsetof(x64emu_t, op1), xEmu);
        STB(s2, offsetof(x64emu_t, op2), xEmu);
        SET_DF(s3, d_add8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    CLEAR_FLAGS(s3);
    IFX (X_AF | X_OF) {
        OR(s3, s1, s2);  // s3 = op1 | op2
        AND(s4, s1, s2); // s4 = op1 & op2
    }
    ADD(s1, s1, s2);

    IFX (X_AF | X_OF) {
        ANDC(s3, s3, s1); // s3 = ~res & (op1 | op2)
        OR(s3, s3, s4);   // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX (X_AF) {
            ANDId(s4, s3, 0x08); // AF: cc & 0x08
            CMPDI(s4, 0);
            BEQ(8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX (X_OF) {
            SRDI(s3, s3, 6);
            SRDI(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDId(s3, s3, 1); // OF: xor of two MSB's of cc
            BF_INSERT(xFlags, s3, F_OF, F_OF);
        }
    }
    IFX (X_CF) {
        SRDI(s3, s1, 8);
        CMPDI(s3, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_CF);
    }
    // d_add8 will use 16bits result to check for CF, so save 16bits and truncate after
    IFX (X_PEND) {
        STH(s1, offsetof(x64emu_t, res), xEmu);
    }
    ANDId(s1, s1, 0xff);
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_SF) {
        SRDI(s3, s1, 7);
        CMPDI(s3, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit ADD8 instruction, from s1, const c, store result in s1 using s3 and s4 as scratch
void emit_add8c(dynarec_ppc64le_t* dyn, int ninst, int s1, int32_t c, int s2, int s3, int s4)
{
    IFX (X_PEND) {
        MOV32w(s4, c & 0xff);
        STB(s1, offsetof(x64emu_t, op1), xEmu);
        STB(s4, offsetof(x64emu_t, op2), xEmu);
        SET_DF(s3, d_add8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    CLEAR_FLAGS(s3);
    IFX (X_AF | X_OF) {
        IFX (X_PEND) {
        } else {
            MOV32w(s4, c & 0xff);
        }
        OR(s3, s1, s4);  // s3 = op1 | op2
        AND(s4, s1, s4); // s4 = op1 & op2
    }
    ADDI(s1, s1, c & 0xff);

    IFX (X_AF | X_OF) {
        ANDC(s3, s3, s1); // s3 = ~res & (op1 | op2)
        OR(s3, s3, s4);   // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX (X_AF) {
            ANDId(s4, s3, 0x08); // AF: cc & 0x08
            CMPDI(s4, 0);
            BEQ(8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX (X_OF) {
            SRDI(s3, s3, 6);
            SRDI(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDId(s3, s3, 1); // OF: xor of two MSB's of cc
            BF_INSERT(xFlags, s3, F_OF, F_OF);
        }
    }
    IFX (X_CF) {
        SRDI(s3, s1, 8);
        CMPDI(s3, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_CF);
    }
    // d_add8 will use 16bits result to check for CF, so save 16bits and truncate after
    IFX (X_PEND) {
        STH(s1, offsetof(x64emu_t, res), xEmu);
    }
    ANDId(s1, s1, 0xff);
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_SF) {
        SRDI(s3, s1, 7);
        CMPDI(s3, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit ADD16 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_add16(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    IFX (X_PEND) {
        STH(s1, offsetof(x64emu_t, op1), xEmu);
        STH(s2, offsetof(x64emu_t, op2), xEmu);
        SET_DF(s3, d_add16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    IFX (X_AF | X_OF) {
        OR(s3, s1, s2);  // s3 = op1 | op2
        AND(s4, s1, s2); // s4 = op1 & op2
    }

    ADD(s1, s1, s2);

    // d_add16 will use 32bits result to check for CF
    IFX (X_PEND) {
        STW(s1, offsetof(x64emu_t, res), xEmu);
    }

    CLEAR_FLAGS(s5);
    IFX (X_AF | X_OF) {
        ANDC(s3, s3, s1); // s3 = ~res & (op1 | op2)
        OR(s3, s3, s4);   // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX (X_AF) {
            ANDId(s4, s3, 0x08); // AF: cc & 0x08
            CMPDI(s4, 0);
            BEQ(8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX (X_OF) {
            SRDI(s3, s3, 14);
            SRDI(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDId(s3, s3, 1); // OF: xor of two MSB's of cc
            BF_INSERT(xFlags, s3, F_OF, F_OF);
        }
    }

    IFX (X_CF) {
        SRDI(s3, s1, 16);
        CMPDI(s3, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_CF);
    }

    BF_EXTRACT(s1, s1, 15, 0);

    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_SF) {
        SRDI(s3, s1, 15);
        CMPDI(s3, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SUB8 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_sub8(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    IFX (X_PEND) {
        STB(s1, offsetof(x64emu_t, op1), xEmu);
        STB(s2, offsetof(x64emu_t, op2), xEmu);
        SET_DF(s3, d_sub8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    CLEAR_FLAGS(s3);
    IFX (X_AF | X_CF | X_OF) {
        // for later flag calculation
        NOT(s5, s1);
    }

    SUB(s1, s1, s2);
    ANDId(s1, s1, 0xff);
    IFX (X_SF) {
        SRDI(s3, s1, 7);
        CMPDI(s3, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX (X_PEND) {
        STB(s1, offsetof(x64emu_t, res), xEmu);
    }
    CALC_SUB_FLAGS(s5, s2, s1, s3, s4, 8);
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SUB8 instruction, from s1, constant c, store result in s1 using s3 and s4 as scratch
void emit_sub8c(dynarec_ppc64le_t* dyn, int ninst, int s1, int32_t c, int s2, int s3, int s4, int s5)
{
    MOV32w(s2, c & 0xff);
    emit_sub8(dyn, ninst, s1, s2, s3, s4, s5);
}

// emit SUB16 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_sub16(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    IFX (X_PEND) {
        STH(s1, offsetof(x64emu_t, op1), xEmu);
        STH(s2, offsetof(x64emu_t, op2), xEmu);
        SET_DF(s3, d_sub16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFX (X_AF | X_CF | X_OF) {
        // for later flag calculation
        NOT(s5, s1);
    }

    SUB(s1, s1, s2);
    IFX (X_PEND) {
        STH(s1, offsetof(x64emu_t, res), xEmu);
    }

    CLEAR_FLAGS(s3);
    // Check SF by extracting bit 15
    SLDI(s1, s1, 48);
    IFX (X_SF) {
        CMPDI(s1, 0);
        BGE(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    SRDI(s1, s1, 48);

    CALC_SUB_FLAGS(s5, s2, s1, s3, s4, 16);
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SUB32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_sub32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SDxw(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, rex.w ? d_sub64 : d_sub32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    CLEAR_FLAGS(s3);
    IFX (X_AF | X_CF | X_OF) {
        // for later flag calculation
        NOT(s5, s1);
    }

    SUBxw(s1, s1, s2);
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_SF) {
        SRDI(s3, s1, rex.w ? 63 : 31);
        CMPDI(s3, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    CALC_SUB_FLAGS(s5, s2, s1, s3, s4, rex.w ? 64 : 32);
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SUB32 instruction, from s1, constant c, store result in s1 using s2, s3, s4 and s5 as scratch
void emit_sub32c(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s2, int s3, int s4, int s5)
{
    if ((s1 == xRSP) && (BOX64DRENV(dynarec_safeflags) < 2) && (!dyn->insts || (dyn->insts[ninst].x64.gen_flags == X_PEND) || (!BOX64ENV(dynarec_df) && (dyn->insts[ninst].x64.gen_flags == X_ALL)))) {
        // special case when doing math on RSP and only PEND is needed: ignoring it!
        if (c > -32768 && c <= 32768) {
            ADDI(s1, s1, -c);
        } else {
            MOV64xw(s2, c);
            SUBxw(s1, s1, s2);
        }
        if (!rex.w) { ZEROUP(s1); }
        return;
    }

    IFX (X_PEND | X_AF | X_CF | X_OF) {
        MOV64xw(s2, c);
    } else if (c <= -32768 || c > 32768) {
        MOV64xw(s2, c);
    }
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SDxw(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, rex.w ? d_sub64 : d_sub32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    CLEAR_FLAGS(s3);
    IFX (X_AF | X_CF | X_OF) {
        // for later flag calculation
        NOT(s5, s1);
    }

    if (c > -32768 && c <= 32768) {
        ADDIxw(s1, s1, -c);
    } else {
        SUBxw(s1, s1, s2);
    }

    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_SF) {
        SRDI(s3, s1, rex.w ? 63 : 31);
        CMPDI(s3, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    CALC_SUB_FLAGS(s5, s2, s1, s3, s4, rex.w ? 64 : 32);
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}


// emit SBB8 instruction, from s1, s2, store result in s1 using s3, s4 and s5 as scratch
void emit_sbb8(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    IFX (X_PEND) {
        STB(s1, offsetof(x64emu_t, op1), xEmu);
        STB(s2, offsetof(x64emu_t, op2), xEmu);
        SET_DF(s3, d_sbb8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFX (X_AF | X_CF | X_OF) {
        // for later flag calculation
        NOT(s5, s1);
    }

    SUB(s1, s1, s2);
    ANDId(s3, xFlags, 1 << F_CF);
    SUB(s1, s1, s3);
    ANDId(s1, s1, 0xff);

    CLEAR_FLAGS(s3);
    IFX (X_PEND) {
        STB(s1, offsetof(x64emu_t, res), xEmu);
    }

    CALC_SUB_FLAGS(s5, s2, s1, s3, s4, 8);
    IFX (X_SF) {
        SRDI(s3, s1, 7);
        CMPDI(s3, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SBB8 instruction, from s1, constant c, store result in s1 using s3, s4, s5 and s6 as scratch
void emit_sbb8c(dynarec_ppc64le_t* dyn, int ninst, int s1, int32_t c, int s3, int s4, int s5, int s6)
{
    MOV32w(s6, c & 0xff);
    emit_sbb8(dyn, ninst, s1, s6, s3, s4, s5);
}

// emit SBB16 instruction, from s1, s2, store result in s1 using s3, s4 and s5 as scratch
void emit_sbb16(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    IFX (X_PEND) {
        STH(s1, offsetof(x64emu_t, op1), xEmu);
        STH(s2, offsetof(x64emu_t, op2), xEmu);
        SET_DF(s3, d_sbb16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFX (X_AF | X_CF | X_OF) {
        // for later flag calculation
        NOT(s5, s1);
    }

    SUB(s1, s1, s2);
    ANDId(s3, xFlags, 1 << F_CF);
    SUB(s1, s1, s3);

    CLEAR_FLAGS(s3);
    // Check SF: shift left 48, check sign, shift back
    SLDI(s1, s1, 48);
    IFX (X_SF) {
        CMPDI(s1, 0);
        BGE(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    SRDI(s1, s1, 48);

    IFX (X_PEND) {
        STH(s1, offsetof(x64emu_t, res), xEmu);
    }

    CALC_SUB_FLAGS(s5, s2, s1, s3, s4, 16);
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SBB32 instruction, from s1, s2, store result in s1 using s3, s4 and s5 as scratch
void emit_sbb32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SDxw(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, rex.w ? d_sbb64 : d_sbb32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFX (X_AF | X_CF | X_OF) {
        // for later flag calculation
        NOT(s5, s1);
    }

    SUBxw(s1, s1, s2);
    ANDId(s3, xFlags, 1 << F_CF);
    SUBxw(s1, s1, s3);

    CLEAR_FLAGS(s3);
    IFX (X_SF) {
        SRDI(s3, s1, rex.w ? 63 : 31);
        CMPDI(s3, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) ZEROUP(s1);
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    CALC_SUB_FLAGS(s5, s2, s1, s3, s4, rex.w ? 64 : 32);
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}


// emit NEG8 instruction, from s1, store result in s1 using s2 and s3 as scratch
void emit_neg8(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3)
{
    IFX (X_PEND) {
        STB(s1, offsetof(x64emu_t, op1), xEmu);
        SET_DF(s3, d_neg8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    IFX (X_AF | X_OF) {
        MV(s3, s1); // s3 = op1
    }

    NEG(s1, s1);
    ANDId(s1, s1, 0xff);
    IFX (X_PEND) {
        STB(s1, offsetof(x64emu_t, res), xEmu);
    }

    CLEAR_FLAGS(s2);
    IFX (X_CF) {
        CMPDI(s1, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_CF);
    }

    IFX (X_AF | X_OF) {
        OR(s3, s1, s3); // s3 = res | op1
        IFX (X_AF) {
            /* af = bc & 0x8 */
            ANDId(s2, s3, 8);
            CMPDI(s2, 0);
            BEQ(8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX (X_OF) {
            /* of = ((bc >> (width-2)) ^ (bc >> (width-1))) & 0x1; */
            SRDI(s2, s3, 6);
            SRDI(s3, s2, 1);
            XOR(s2, s2, s3);
            ANDId(s2, s2, 1);
            CMPDI(s2, 0);
            BEQ(8);
            ORI(xFlags, xFlags, 1 << F_OF);
        }
    }
    IFX (X_SF) {
        ANDId(s3, s1, 1 << F_SF); // 1<<F_SF is sign bit, so just mask
        OR(xFlags, xFlags, s3);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s2);
    }
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit NEG32 instruction, from s1, store result in s1 using s2 and s3 as scratch
void emit_neg32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3)
{
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s3, rex.w ? d_neg64 : d_neg32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFX (X_AF | X_OF) {
        MV(s3, s1); // s3 = op1
    }

    NEG(s1, s1);
    if (!rex.w) ZEROUP(s1);
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }

    CLEAR_FLAGS(s2);
    IFX (X_CF) {
        CMPDI(s1, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_CF);
    }

    IFX (X_AF | X_OF) {
        OR(s3, s1, s3); // s3 = res | op1
        IFX (X_AF) {
            /* af = bc & 0x8 */
            ANDId(s2, s3, 8);
            CMPDI(s2, 0);
            BEQ(8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX (X_OF) {
            /* of = ((bc >> (width-2)) ^ (bc >> (width-1))) & 0x1; */
            SRDI(s2, s3, (rex.w ? 64 : 32) - 2);
            SRDI(s3, s2, 1);
            XOR(s2, s2, s3);
            ANDId(s2, s2, 1);
            CMPDI(s2, 0);
            BEQ(8);
            ORI(xFlags, xFlags, 1 << F_OF);
        }
    }
    IFX (X_SF) {
        SRDI(s3, s1, rex.w ? 63 : 31);
        CMPDI(s3, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s2);
    }
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit NEG16 instruction, from s1, store result in s1 using s2 and s3 as scratch
void emit_neg16(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3)
{
    IFX (X_PEND) {
        STH(s1, offsetof(x64emu_t, op1), xEmu);
        SET_DF(s3, d_neg16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    CLEAR_FLAGS(s3);
    IFX (X_AF | X_OF) {
        MV(s3, s1); // s3 = op1
    }

    NEG(s1, s1);
    BF_EXTRACT(s1, s1, 15, 0);
    IFX (X_PEND) {
        STH(s1, offsetof(x64emu_t, res), xEmu);
    }

    IFX (X_CF) {
        CMPDI(s1, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_CF);
    }

    IFX (X_AF | X_OF) {
        OR(s3, s1, s3); // s3 = res | op1
        IFX (X_AF) {
            /* af = bc & 0x8 */
            ANDId(s2, s3, 8);
            CMPDI(s2, 0);
            BEQ(8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX (X_OF) {
            /* of = ((bc >> (width-2)) ^ (bc >> (width-1))) & 0x1; */
            SRDI(s2, s3, 14);
            SRDI(s3, s2, 1);
            XOR(s2, s2, s3);
            ANDId(s2, s2, 1);
            CMPDI(s2, 0);
            BEQ(8);
            ORI(xFlags, xFlags, 1 << F_OF);
        }
    }
    IFX (X_SF) {
        SRDI(s3, s1, 15 - F_SF); // put sign bit in place
        ANDId(s3, s3, 1 << F_SF);   // 1<<F_SF is sign bit, so just mask
        OR(xFlags, xFlags, s3);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s2);
    }
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit ADC8 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_adc8(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    IFX (X_PEND) {
        STB(s1, offsetof(x64emu_t, op1), xEmu);
        STB(s2, offsetof(x64emu_t, op2), xEmu);
        SET_DF(s3, d_adc8b);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFX (X_AF | X_OF) {
        OR(s4, s1, s2);  // s4 = op1 | op2
        AND(s5, s1, s2); // s5 = op1 & op2
    }

    ADD(s1, s1, s2);
    ANDId(s3, xFlags, 1 << F_CF);
    ADD(s1, s1, s3);

    CLEAR_FLAGS(s3);
    IFX (X_PEND) {
        // d_adc8 will use 16bits result to check for CF
        STB(s1, offsetof(x64emu_t, res), xEmu);
    }
    IFX (X_AF | X_OF) {
        ANDC(s3, s4, s1); // s3 = ~res & (op1 | op2)
        OR(s3, s3, s5);   // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX (X_AF) {
            ANDId(s4, s3, 0x08); // AF: cc & 0x08
            CMPDI(s4, 0);
            BEQ(8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX (X_OF) {
            SRDI(s3, s3, 6);
            SRDI(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDId(s3, s3, 1); // OF: xor of two MSB's of cc
            BF_INSERT(xFlags, s3, F_OF, F_OF);
        }
    }
    IFX (X_CF) {
        SRDI(s3, s1, 8);
        CMPDI(s3, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_CF);
    }

    ANDId(s1, s1, 0xff); // unneeded?

    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_SF) {
        SRDI(s3, s1, 7);
        CMPDI(s3, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit ADC8 instruction, from s1, const c, store result in s1 using s3, s4, s5 and s6 as scratch
void emit_adc8c(dynarec_ppc64le_t* dyn, int ninst, int s1, int32_t c, int s3, int s4, int s5, int s6)
{
    MOV32w(s5, c & 0xff);
    emit_adc8(dyn, ninst, s1, s5, s3, s4, s6);
}

// emit ADC16 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_adc16(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    IFX (X_PEND) {
        STH(s1, offsetof(x64emu_t, op1), xEmu);
        STH(s2, offsetof(x64emu_t, op2), xEmu);
        SET_DF(s3, d_adc16b);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFX (X_AF | X_OF) {
        OR(s4, s1, s2);  // s4 = op1 | op2
        AND(s5, s1, s2); // s5 = op1 & op2
    }

    ADD(s1, s1, s2);
    ANDId(s3, xFlags, 1 << F_CF);
    ADD(s1, s1, s3);

    CLEAR_FLAGS(s3);
    IFX (X_PEND) {
        // d_adc16 will use 32bits result to check for CF
        STH(s1, offsetof(x64emu_t, res), xEmu);
    }
    IFX (X_AF | X_OF) {
        ANDC(s3, s4, s1); // s3 = ~res & (op1 | op2)
        OR(s3, s3, s5);   // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX (X_AF) {
            ANDId(s4, s3, 0x08); // AF: cc & 0x08
            CMPDI(s4, 0);
            BEQ(8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX (X_OF) {
            SRDI(s3, s3, 14);
            SRDI(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDId(s3, s3, 1); // OF: xor of two MSB's of cc
            BF_INSERT(xFlags, s3, F_OF, F_OF);
        }
    }
    IFX (X_CF) {
        SRDI(s3, s1, 16);
        CMPDI(s3, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_CF);
    }

    BF_EXTRACT(s1, s1, 15, 0);  // unneeded?

    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_SF) {
        SRDI(s3, s1, 15);
        CMPDI(s3, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit ADC32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_adc32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5, int s6)
{
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SDxw(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, rex.w ? d_adc64 : d_adc32b);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFX (X_CF) {
        if (rex.w) {
            ZEROUP2(s5, s1);
            ZEROUP2(s4, s2);
            ADD(s5, s5, s4); // lo
            ANDId(s3, xFlags, 1);
            ADD(s5, s5, s3); // add carry
            SRDI(s3, s1, 0x20);
            SRDI(s4, s2, 0x20);
            ADD(s4, s4, s3);
            SRDI(s5, s5, 0x20);
            ADD(s5, s5, s4); // hi
            SRADI(s6, s5, 0x20);
        } else {
            ZEROUP2(s3, s1);
            ZEROUP2(s4, s2);
            ADD(s5, s3, s4);
            ANDId(s3, xFlags, 1);
            ADD(s5, s5, s3); // add carry
            SRDI(s6, s5, 0x20);
        }
    }
    IFX (X_AF | X_OF) {
        OR(s4, s1, s2);  // s4 = op1 | op2
        AND(s5, s1, s2); // s5 = op1 & op2
    }

    ADDxw(s1, s1, s2);
    ANDId(s3, xFlags, 1 << F_CF);
    ADDxw(s1, s1, s3);

    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }

    CLEAR_FLAGS(s3);
    IFX (X_CF) {
        CMPDI(s6, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_CF);
    }
    IFX (X_AF | X_OF) {
        ANDC(s3, s4, s1); // s3 = ~res & (op1 | op2)
        OR(s3, s3, s5);   // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX (X_AF) {
            ANDId(s4, s3, 0x08); // AF: cc & 0x08
            CMPDI(s4, 0);
            BEQ(8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX (X_OF) {
            SRDI(s3, s3, rex.w ? 62 : 30);
            SRDI(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDId(s3, s3, 1); // OF: xor of two MSB's of cc
            BF_INSERT(xFlags, s3, F_OF, F_OF);
        }
    }
    IFX (X_SF) {
        SRDI(s3, s1, rex.w ? 63 : 31);
        CMPDI(s3, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit INC8 instruction, from s1, store result in s1 using s2, s3 and s4 as scratch
void emit_inc8(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    SET_DFNONE();
    IFX (X_ALL) {
        // preserving CF
        MOV64x(s4, (1UL << F_AF) | (1UL << F_OF) | (1UL << F_ZF) | (1UL << F_SF) | (1UL << F_PF));
        ANDC(xFlags, xFlags, s4);
    }

    IFX (X_AF | X_OF) {
        ORI(s3, s1, 1);  // s3 = op1 | op2
        ANDId(s4, s1, 1); // s4 = op1 & op2
    }

    ADDI(s1, s1, 1);

    IFX (X_AF | X_OF) {
        ANDC(s3, s3, s1); // s3 = ~res & (op1 | op2)
        OR(s3, s3, s4);   // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX (X_AF) {
            ANDId(s2, s3, 0x08); // AF: cc & 0x08
            CMPDI(s2, 0);
            BEQ(8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX (X_OF) {
            SRDI(s3, s3, 6);
            SRDI(s2, s3, 1);
            XOR(s3, s3, s2);
            ANDId(s3, s3, 1); // OF: xor of two MSB's of cc
            BF_INSERT(xFlags, s3, F_OF, F_OF);
        }
    }
    IFX (X_SF) {
        ANDId(s2, s1, 0x80);
        CMPDI(s2, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    ANDId(s1, s1, 0xff);
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s2);
    }
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit INC16 instruction, from s1, store result in s1 using s3 and s4 as scratch
void emit_inc16(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    SET_DFNONE();
    IFX (X_ALL) {
        // preserving CF
        MOV64x(s4, (1UL << F_AF) | (1UL << F_OF) | (1UL << F_ZF) | (1UL << F_SF) | (1UL << F_PF));
        ANDC(xFlags, xFlags, s4);
    }

    IFX (X_AF | X_OF) {
        ORI(s3, s1, 1);  // s3 = op1 | op2
        ANDId(s4, s1, 1); // s4 = op1 & op2
    }

    ADDI(s1, s1, 1);

    IFX (X_AF | X_OF) {
        ANDC(s3, s3, s1); // s3 = ~res & (op1 | op2)
        OR(s3, s3, s4);   // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX (X_AF) {
            ANDId(s4, s3, 0x08); // AF: cc & 0x08
            CMPDI(s4, 0);
            BEQ(8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX (X_OF) {
            SRDI(s3, s3, 14);
            SRDI(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDId(s3, s3, 1); // OF: xor of two MSB's of cc
            BF_INSERT(xFlags, s3, F_OF, F_OF);
        }
    }

    BF_EXTRACT(s1, s1, 15, 0);

    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_SF) {
        SRDI(s3, s1, 15);
        CMPDI(s3, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit INC32 instruction, from s1, store result in s1 using s3 and s4 as scratch
void emit_inc32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    SET_DFNONE();
    IFX (X_ALL) {
        // preserving CF
        MOV64x(s4, (1UL << F_AF) | (1UL << F_OF) | (1UL << F_ZF) | (1UL << F_SF) | (1UL << F_PF));
        ANDC(xFlags, xFlags, s4);
    }

    IFX (X_AF | X_OF) {
        ORI(s3, s1, 1);  // s3 = op1 | op2
        ANDId(s5, s1, 1); // s5 = op1 & op2
    }

    ADDIxw(s1, s1, 1);

    IFX (X_AF | X_OF) {
        ANDC(s3, s3, s1); // s3 = ~res & (op1 | op2)
        OR(s3, s3, s5);   // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX (X_AF) {
            ANDId(s2, s3, 0x08); // AF: cc & 0x08
            CMPDI(s2, 0);
            BEQ(8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX (X_OF) {
            SRDI(s3, s3, rex.w ? 62 : 30);
            SRDI(s2, s3, 1);
            XOR(s3, s3, s2);
            ANDId(s3, s3, 1); // OF: xor of two MSB's of cc
            BF_INSERT(xFlags, s3, F_OF, F_OF);
        }
    }
    IFX (X_SF) {
        SRDI(s3, s1, rex.w ? 63 : 31);
        CMPDI(s3, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s2);
    }
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit DEC8 instruction, from s1, store result in s1 using s2, s3 and s4 as scratch
void emit_dec8(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    SET_DFNONE();
    IFX (X_ALL) {
        // preserving CF
        MOV64x(s4, (1UL << F_AF) | (1UL << F_OF) | (1UL << F_ZF) | (1UL << F_SF) | (1UL << F_PF));
        ANDC(xFlags, xFlags, s4);
    }

    IFX (X_AF | X_OF) {
        NOT(s4, s1);      // s4 = ~op1
        ORI(s3, s4, 1);   // s3 = ~op1 | op2
        ANDId(s4, s4, 1); // s4 = ~op1 & op2
    }

    ADDI(s1, s1, -1);

    IFX (X_AF | X_OF) {
        AND(s3, s1, s3); // s3 = res & (~op1 | op2)
        OR(s3, s3, s4);  // cc = (res & (~op1 | op2)) | (~op1 & op2)
        IFX (X_AF) {
            ANDId(s2, s3, 0x08); // AF: cc & 0x08
            CMPDI(s2, 0);
            BEQ(8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX (X_OF) {
            SRDI(s3, s3, 6);
            SRDI(s2, s3, 1);
            XOR(s3, s3, s2);
            ANDId(s3, s3, 1); // OF: xor of two MSB's of cc
            BF_INSERT(xFlags, s3, F_OF, F_OF);
        }
    }
    IFX (X_SF) {
        ANDId(s2, s1, 0x80);
        CMPDI(s2, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    ANDId(s1, s1, 0xff);
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s2);
    }
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit DEC16 instruction, from s1, store result in s1 using s3 and s4 as scratch
void emit_dec16(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    SET_DFNONE();

    IFX (X_AF | X_OF) {
        NOT(s5, s1);
        ORI(s3, s5, 1);   // s3 = ~op1 | op2
        ANDId(s5, s5, 1); // s5 = ~op1 & op2
    }

    ADDI(s1, s1, -1);
    IFX (X_ALL) {
        // preserving CF
        MOV64x(s4, (1UL << F_AF) | (1UL << F_OF) | (1UL << F_ZF) | (1UL << F_SF) | (1UL << F_PF));
        ANDC(xFlags, xFlags, s4);
    }

    IFX (X_AF | X_OF) {
        AND(s3, s1, s3); // s3 = res & (~op1 | op2)
        OR(s3, s3, s5);  // cc = (res & (~op1 | op2)) | (~op1 & op2)
        IFX (X_AF) {
            ANDId(s2, s3, 0x08); // AF: cc & 0x08
            CMPDI(s2, 0);
            BEQ(8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX (X_OF) {
            SRDI(s3, s3, 14);
            SRDI(s2, s3, 1);
            XOR(s3, s3, s2);
            ANDId(s3, s3, 1); // OF: xor of two MSB's of cc
            BF_INSERT(xFlags, s3, F_OF, F_OF);
        }
    }
    IFX (X_SF) {
        BF_EXTRACT(s5, s1, 15, 15);
        BF_INSERT(xFlags, s5, F_SF, F_SF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s2);
    }
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit DEC32 instruction, from s1, store result in s1 using s3 and s4 as scratch
void emit_dec32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    SET_DFNONE();

    IFX (X_AF | X_OF) {
        NOT(s5, s1);
        ORI(s3, s5, 1);   // s3 = ~op1 | op2
        ANDId(s5, s5, 1); // s5 = ~op1 & op2
    }

    ADDIxw(s1, s1, -1);
    IFX (X_ALL) {
        // preserving CF
        MOV64x(s4, (1UL << F_AF) | (1UL << F_OF) | (1UL << F_ZF) | (1UL << F_SF) | (1UL << F_PF));
        ANDC(xFlags, xFlags, s4);
    }
    IFX (X_AF | X_OF) {
        AND(s3, s1, s3); // s3 = res & (~op1 | op2)
        OR(s3, s3, s5);  // cc = (res & (~op1 | op2)) | (~op1 & op2)
        IFX (X_AF) {
            ANDId(s2, s3, 0x08); // AF: cc & 0x08
            CMPDI(s2, 0);
            BEQ(8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX (X_OF) {
            SRDI(s3, s3, rex.w ? 62 : 30);
            SRDI(s2, s3, 1);
            XOR(s3, s3, s2);
            ANDId(s3, s3, 1); // OF: xor of two MSB's of cc
            BF_INSERT(xFlags, s3, F_OF, F_OF);
        }
    }
    IFX (X_SF) {
        BF_EXTRACT(s5, s1, 31, 31);
        BF_INSERT(xFlags, s5, F_SF, F_SF);
    }
    if (!rex.w) ZEROUP(s1);
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s2);
    }
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}
