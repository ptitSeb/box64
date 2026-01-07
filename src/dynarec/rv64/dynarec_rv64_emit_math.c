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

// emit ADD32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_add32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    CLEAR_FLAGS();
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SDxw(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, rex.w ? d_add64 : d_add32b);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    IFX (X_CF) {
        if (rex.w) {
            ZEXTW2(s5, s1);
            if (cpuext.zba) // lo
                ADDUW(s5, s2, s5);
            else {
                ZEXTW2(s4, s2);
                ADD(s5, s5, s4);
            }
            SRLI(s3, s1, 0x20);
            SRLI(s4, s2, 0x20);
            ADD(s4, s4, s3);
            SRLI(s5, s5, 0x20);
            ADD(s5, s5, s4); // hi
            SRAI(s5, s5, 0x20);
        } else {
            ZEXTW2(s3, s1);
            ZEXTW2(s4, s2);
            ADD(s5, s3, s4);
            SRLI(s5, s5, 0x20);
        }
        SET_FLAGS_NEZ(s5, F_CF, s4);
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
        if (cpuext.zbb) {
            ANDN(s3, s3, s1); // s3 = ~res & (op1 | op2)
        } else {
            NOT(s5, s1);     // s5 = ~res
            AND(s3, s5, s3); // s3 = ~res & (op1 | op2)
        }
        OR(s3, s3, s4); // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX (X_AF) {
            ANDI(s4, s3, 0x08); // AF: cc & 0x08
            SET_FLAGS_NEZ(s4, F_AF, s5);
        }
        IFX (X_OF) {
            SRLI(s3, s3, rex.w ? 62 : 30);
            SRLI(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            SET_FLAGS_NEZ(s3, F_OF2, s5);
        }
    }
    IFX (X_SF) {
        SET_FLAGS_LTZ(s1, F_SF, s3, s5);
    }
    if (!rex.w && (IS_GPR(s1) || dyn->insts[ninst].nat_flags_fusion)) {
        ZEROUP(s1);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit ADD32 instruction, from s1, constant c, store result in s1 using s3 and s4 as scratch
void emit_add32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s2, int s3, int s4, int s5)
{
    if ((s1 == xRSP) && (BOX64DRENV(dynarec_safeflags) < 2) && (!dyn->insts || (dyn->insts[ninst].x64.gen_flags == X_PEND) || (!BOX64ENV(dynarec_df) && (dyn->insts[ninst].x64.gen_flags == X_ALL)))) {
        // special case when doing math on ESP and only PEND is needed: ignoring it!
        if (c >= -2048 && c < 2048) {
            ADDI(s1, s1, c);
        } else {
            MOV64x(s2, c);
            ADD(s1, s1, s2);
        }
        if (!rex.w) {
            ZEROUP(s1);
        }
        return;
    }
    CLEAR_FLAGS();
    IFX (X_PEND | X_AF | X_CF | X_OF) {
        MOV64xw(s2, c);
    }
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SDxw(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, rex.w ? d_add64 : d_add32b);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    IFX (X_CF) {
        if (rex.w) {
            ZEXTW2(s5, s1);
            if (cpuext.zba) // lo
                ADDUW(s5, s2, s5);
            else {
                ZEXTW2(s4, s2);
                ADD(s5, s5, s4);
            }
            SRLI(s3, s1, 0x20);
            SRLI(s4, s2, 0x20);
            ADD(s4, s4, s3);
            SRLI(s5, s5, 0x20);
            ADD(s5, s5, s4); // hi
            SRAI(s5, s5, 0x20);
        } else {
            ZEXTW2(s3, s1);
            ZEXTW2(s4, s2);
            ADD(s5, s3, s4);
            SRLI(s5, s5, 0x20);
        }
        SET_FLAGS_NEZ(s5, F_CF, s4);
    }
    IFX (X_AF | X_OF) {
        OR(s3, s1, s2);  // s3 = op1 | op2
        AND(s4, s1, s2); // s4 = op1 & op2
    }

    if (c >= -2048 && c < 2048) {
        ADDIxw(s1, s1, c);
    } else {
        IFX (X_PEND | X_AF | X_CF | X_OF) {
        } else {
            MOV64xw(s2, c);
        }
        ADDxw(s1, s1, s2);
    }

    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_AF | X_OF) {
        if (cpuext.zbb) {
            ANDN(s3, s3, s1); // s3 = ~res & (op1 | op2)
        } else {
            NOT(s2, s1);     // s2 = ~res
            AND(s3, s2, s3); // s3 = ~res & (op1 | op2)
        }
        OR(s3, s3, s4); // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX (X_AF) {
            ANDI(s4, s3, 0x08); // AF: cc & 0x08
            SET_FLAGS_NEZ(s4, F_AF, s5);
        }
        IFX (X_OF) {
            SRLI(s3, s3, rex.w ? 62 : 30);
            SRLI(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            SET_FLAGS_NEZ(s3, F_OF2, s5);
        }
    }
    IFX (X_SF) {
        SET_FLAGS_LTZ(s1, F_SF, s3, s5);
    }
    if (!rex.w && (IS_GPR(s1) || dyn->insts[ninst].nat_flags_fusion)) {
        ZEROUP(s1);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit ADD16 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_add16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    CLEAR_FLAGS();
    IFX (X_PEND) {
        SH(s1, xEmu, offsetof(x64emu_t, op1));
        SH(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, d_add16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    IFX (X_AF | X_OF) {
        OR(s3, s1, s2);  // s3 = op1 | op2
        AND(s4, s1, s2); // s4 = op1 & op2
    }
    ADD(s1, s1, s2);

    IFX (X_PEND) {
        SW(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_AF | X_OF) {
        if (cpuext.zbb) {
            ANDN(s3, s3, s1); // s3 = ~res & (op1 | op2)
        } else {
            NOT(s5, s1);     // s5 = ~res
            AND(s3, s5, s3); // s3 = ~res & (op1 | op2)
        }
        OR(s3, s3, s4); // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX (X_AF) {
            ANDI(s4, s3, 0x08); // AF: cc & 0x08
            SET_FLAGS_NEZ(s4, F_AF, s5);
        }
        IFX (X_OF) {
            SRLI(s3, s3, 14);
            SRLI(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            SET_FLAGS_NEZ(s3, F_OF2, s5);
        }
    }

    IFX (X_CF) {
        SRLI(s3, s1, 16);
        SET_FLAGS_NEZ(s3, F_CF, s4);
    }

    ZEXTH(s1, s1);

    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s4);
    }
    IFX (X_SF) {
        SRLI(s3, s1, 15);
        SET_FLAGS_NEZ(s3, F_SF, s4);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit ADD8 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_add8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    CLEAR_FLAGS();
    IFX (X_PEND) {
        SB(s1, xEmu, offsetof(x64emu_t, op1));
        SB(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, d_add8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    IFX (X_AF | X_OF) {
        OR(s3, s1, s2);  // s3 = op1 | op2
        AND(s4, s1, s2); // s4 = op1 & op2
    }
    ADD(s1, s1, s2);

    IFX (X_AF | X_OF) {
        if (cpuext.zbb) {
            ANDN(s3, s3, s1); // s3 = ~res & (op1 | op2)
        } else {
            NOT(s2, s1);     // s4 = ~res
            AND(s3, s2, s3); // s3 = ~res & (op1 | op2)
        }
        OR(s3, s3, s4); // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX (X_AF) {
            ANDI(s4, s3, 0x08); // AF: cc & 0x08
            SET_FLAGS_NEZ(s4, F_AF, s5);
        }
        IFX (X_OF) {
            SRLI(s3, s3, 6);
            SRLI(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            SET_FLAGS_NEZ(s3, F_OF2, s5);
        }
    }
    IFX (X_CF) {
        SRLI(s3, s1, 8);
        SET_FLAGS_NEZ(s3, F_CF, s4);
    }
    IFX (X_PEND) {
        SB(s1, xEmu, offsetof(x64emu_t, res));
    }
    ANDI(s1, s1, 0xff);
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s4);
    }
    IFX (X_SF) {
        SRLI(s3, s1, 7);
        SET_FLAGS_NEZ(s3, F_SF, s4);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit ADD8 instruction, from s1, const c, store result in s1 using s3 and s4 as scratch
void emit_add8c(dynarec_rv64_t* dyn, int ninst, int s1, int c, int s2, int s3, int s4, int s5)
{
    CLEAR_FLAGS();
    IFX (X_PEND) {
        MOV32w(s4, c & 0xff);
        SB(s1, xEmu, offsetof(x64emu_t, op1));
        SB(s4, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, d_add8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    IFX (X_AF | X_OF) {
        IFX (X_PEND) {
        } else {
            MOV32w(s4, c & 0xff);
        }
        OR(s3, s1, s4);  // s3 = op1 | op2
        AND(s4, s1, s4); // s4 = op1 & op2
    }
    ADDI(s1, s1, c);

    IFX (X_AF | X_OF) {
        if (cpuext.zbb) {
            ANDN(s3, s3, s1); // s3 = ~res & (op1 | op2)
        } else {
            NOT(s2, s1);     // s2 = ~res
            AND(s3, s2, s3); // s3 = ~res & (op1 | op2)
        }
        OR(s3, s3, s4); // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX (X_AF) {
            ANDI(s4, s3, 0x08); // AF: cc & 0x08
            SET_FLAGS_NEZ(s4, F_AF, s5);
        }
        IFX (X_OF) {
            SRLI(s3, s3, 6);
            SRLI(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            SET_FLAGS_NEZ(s3, F_OF2, s5);
        }
    }
    IFX (X_CF) {
        SRLI(s3, s1, 8);
        SET_FLAGS_NEZ(s3, F_CF, s4);
    }
    IFX (X_PEND) {
        SB(s1, xEmu, offsetof(x64emu_t, res));
    }
    ANDI(s1, s1, 0xff);
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s4);
    }
    IFX (X_SF) {
        SRLI(s3, s1, 7);
        SET_FLAGS_NEZ(s3, F_SF, s4);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SUB8 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_sub8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    CLEAR_FLAGS();
    IFX (X_PEND) {
        SB(s1, xEmu, offsetof(x64emu_t, op1));
        SB(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, d_sub8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFX (X_AF | X_CF | X_OF) {
        // for later flag calculation
        NOT(s5, s1);
    }

    SUB(s1, s1, s2);
    ANDI(s1, s1, 0xff);

    IFX (X_SF) {
        SRLI(s3, s1, 7);
        SET_FLAGS_NEZ(s3, F_SF, s4);
    }
    IFX (X_PEND) {
        SB(s1, xEmu, offsetof(x64emu_t, res));
    }
    CALC_SUB_FLAGS(s5, s2, s1, s3, s4, 8);
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s5);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SUB8 instruction, from s1, constant c, store result in s1 using s3 and s4 as scratch
void emit_sub8c(dynarec_rv64_t* dyn, int ninst, int s1, int c, int s2, int s3, int s4, int s5)
{
    MOV32w(s2, c & 0xff);
    emit_sub8(dyn, ninst, s1, s2, s3, s4, s5);
}

// emit SUB16 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_sub16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    CLEAR_FLAGS();
    IFX (X_PEND) {
        SH(s1, xEmu, offsetof(x64emu_t, op1));
        SH(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, d_sub16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFX (X_AF | X_CF | X_OF) {
        // for later flag calculation
        NOT(s5, s1);
    }

    SUBW(s1, s1, s2);

    IFX (X_PEND) {
        SH(s1, xEmu, offsetof(x64emu_t, res));
    }
    SLLI(s1, s1, 48);
    IFX (X_SF) {
        SET_FLAGS_LTZ(s1, F_SF, s3, s4);
    }
    SRLI(s1, s1, 48);

    CALC_SUB_FLAGS(s5, s2, s1, s3, s4, 16);
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s5);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}


// emit SUB32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_sub32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    CLEAR_FLAGS();
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SDxw(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, rex.w ? d_sub64 : d_sub32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFX (X_AF | X_CF | X_OF) {
        // for later flag calculation
        NOT(s5, s1);
    }

    SUBxw(s1, s1, s2);

    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_SF) {
        SET_FLAGS_LTZ(s1, F_SF, s3, s4);
    }
    if (!rex.w && (IS_GPR(s1) || dyn->insts[ninst].nat_flags_fusion)) {
        ZEROUP(s1);
    }
    CALC_SUB_FLAGS(s5, s2, s1, s3, s4, rex.w ? 64 : 32);
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s5);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}


// emit SUB32 instruction, from s1, constant c, store result in s1 using s2, s3, s4 and s5 as scratch
void emit_sub32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s2, int s3, int s4, int s5)
{
    if ((s1 == xRSP) && (BOX64DRENV(dynarec_safeflags) < 2) && (!dyn->insts || (dyn->insts[ninst].x64.gen_flags == X_PEND) || (!BOX64ENV(dynarec_df) && (dyn->insts[ninst].x64.gen_flags == X_ALL)))) {
        // special case when doing math on RSP and only PEND is needed: ignoring it!
        if (c > -2048 && c <= 2048) {
            SUBI(s1, s1, c);
        } else {
            MOV64xw(s2, c);
            SUBxw(s1, s1, s2);
        }
        if (!rex.w) {
            ZEROUP(s1);
        }
        return;
    }
    CLEAR_FLAGS();
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        MOV64xw(s2, c);
        SDxw(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, rex.w ? d_sub64 : d_sub32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFX (X_AF | X_CF | X_OF) {
        // for later flag calculation
        NOT(s5, s1);
    }

    if (c > -2048 && c <= 2048) {
        ADDIxw(s1, s1, -c);
    } else {
        IFX (X_PEND) {
        } else {
            MOV64xw(s2, c);
        }
        SUBxw(s1, s1, s2);
    }

    IFX (X_AF | X_CF | X_OF) {
        IFX (X_PEND) {
        } else if (c > -2048 && c <= 2048) {
            MOV64xw(s2, c);
        }
    }
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_SF) {
        SET_FLAGS_LTZ(s1, F_SF, s3, s4);
    }
    if (!rex.w && (IS_GPR(s1) || dyn->insts[ninst].nat_flags_fusion)) {
        ZEROUP(s1);
    }
    CALC_SUB_FLAGS(s5, s2, s1, s3, s4, rex.w ? 64 : 32);
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s5);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit INC8 instruction, from s1, store result in s1 using s2, s3 and s4 as scratch
void emit_inc8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFXORNAT (X_ALL) {
        ANDI(xFlags, xFlags, ~((1UL << F_AF) | (1UL << F_OF2) | (1UL << F_ZF) | (1UL << F_SF) | (1UL << F_PF)));
    }
    IFX (X_PEND) {
        SB(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s3, d_inc8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    IFX (X_AF | X_OF) {
        ORI(s3, s1, 1);  // s3 = op1 | op2
        ANDI(s4, s1, 1); // s5 = op1 & op2
    }

    ADDIW(s1, s1, 1);

    IFX (X_PEND) {
        SB(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_AF | X_OF) {
        if (cpuext.zbb) {
            ANDN(s3, s3, s1); // s3 = ~res & (op1 | op2)
        } else {
            NOT(s2, s1);     // s2 = ~res
            AND(s3, s2, s3); // s3 = ~res & (op1 | op2)
        }
        OR(s3, s3, s4); // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX (X_AF) {
            ANDI(s2, s3, 0x08); // AF: cc & 0x08
            SET_FLAGS_NEZ(s2, F_AF, s4);
        }
        IFX (X_OF) {
            SRLI(s3, s3, 6);
            SRLI(s2, s3, 1);
            XOR(s3, s3, s2);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            SET_FLAGS_NEZ(s3, F_OF2, s4);
        }
    }
    IFX (X_SF) {
        ANDI(s2, s1, 0x80);
        SET_FLAGS_NEZ(s2, F_SF, s4);
    }
    ANDI(s1, s1, 0xff);
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s2);
    }
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}


// emit DEC8 instruction, from s1, store result in s1 using s2, s3 and s4 as scratch
void emit_dec8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFXORNAT (X_ALL) {
        ANDI(xFlags, xFlags, ~((1UL << F_AF) | (1UL << F_OF2) | (1UL << F_ZF) | (1UL << F_SF) | (1UL << F_PF)));
    }
    IFX (X_PEND) {
        SB(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s3, d_dec8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    IFX (X_AF | X_OF) {
        NOT(s4, s1);     // s4 = ~op1
        ORI(s3, s4, 1);  // s3 = ~op1 | op2
        ANDI(s4, s4, 1); // s4 = ~op1 & op2
    }

    ADDIW(s1, s1, -1);

    IFX (X_PEND) {
        SB(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_AF | X_OF) {
        AND(s3, s1, s3); // s3 = res & (~op1 | op2)
        OR(s3, s3, s4);  // cc = (res & (~op1 | op2)) | (~op1 & op2)
        IFX (X_AF) {
            ANDI(s2, s3, 0x08); // AF: cc & 0x08
            SET_FLAGS_NEZ(s2, F_AF, s4);
        }
        IFX (X_OF) {
            SRLI(s3, s3, 6);
            SRLI(s2, s3, 1);
            XOR(s3, s3, s2);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            SET_FLAGS_NEZ(s3, F_OF2, s4);
        }
    }
    IFX (X_SF) {
        ANDI(s2, s1, 0x80);
        SET_FLAGS_NEZ(s2, F_SF, s4);
    }
    ANDI(s1, s1, 0xff);
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s2);
    }
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit INC32 instruction, from s1, store result in s1 using s3 and s4 as scratch
void emit_inc32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    IFXORNAT (X_ALL) {
        ANDI(xFlags, xFlags, ~((1UL << F_AF) | (1UL << F_OF2) | (1UL << F_ZF) | (1UL << F_SF) | (1UL << F_PF)));
    }
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s3, rex.w ? d_inc64 : d_inc32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    IFX (X_AF | X_OF) {
        ORI(s3, s1, 1);  // s3 = op1 | op2
        ANDI(s5, s1, 1); // s5 = op1 & op2
    }

    ADDIxw(s1, s1, 1);

    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_AF | X_OF) {
        if (cpuext.zbb) {
            ANDN(s3, s3, s1); // s3 = ~res & (op1 | op2)
        } else {
            NOT(s2, s1);     // s2 = ~res
            AND(s3, s2, s3); // s3 = ~res & (op1 | op2)
        }
        OR(s3, s3, s5); // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX (X_AF) {
            ANDI(s2, s3, 0x08); // AF: cc & 0x08
            SET_FLAGS_NEZ(s2, F_AF, s4);
        }
        IFX (X_OF) {
            SRLI(s3, s3, rex.w ? 62 : 30);
            SRLI(s2, s3, 1);
            XOR(s3, s3, s2);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            SET_FLAGS_NEZ(s3, F_OF2, s4);
        }
    }
    IFX (X_SF) {
        SET_FLAGS_LTZ(s1, F_SF, s4, s5);
    }
    if (!rex.w && (IS_GPR(s1) || dyn->insts[ninst].nat_flags_fusion)) {
        ZEROUP(s1);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s2);
    }
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit DEC32 instruction, from s1, store result in s1 using s3 and s4 as scratch
void emit_dec32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    IFXORNAT (X_ALL) {
        ANDI(xFlags, xFlags, ~((1UL << F_AF) | (1UL << F_OF2) | (1UL << F_ZF) | (1UL << F_SF) | (1UL << F_PF)));
    }
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s3, rex.w ? d_dec64 : d_dec32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    IFX (X_AF | X_OF) {
        NOT(s5, s1);
        ORI(s3, s5, 1);  // s3 = ~op1 | op2
        ANDI(s5, s5, 1); // s5 = ~op1 & op2
    }

    ADDIxw(s1, s1, -1);

    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_AF | X_OF) {
        AND(s3, s1, s3); // s3 = res & (~op1 | op2)
        OR(s3, s3, s5);  // cc = (res & (~op1 | op2)) | (~op1 & op2)
        IFX (X_AF) {
            ANDI(s2, s3, 0x08); // AF: cc & 0x08
            SET_FLAGS_NEZ(s2, F_AF, s4);
        }
        IFX (X_OF) {
            SRLI(s3, s3, rex.w ? 62 : 30);
            SRLI(s2, s3, 1);
            XOR(s3, s3, s2);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            SET_FLAGS_NEZ(s3, F_OF2, s4);
        }
    }
    IFX (X_SF) {
        SET_FLAGS_LTZ(s1, F_SF, s4, s5);
    }
    if (!rex.w && (IS_GPR(s1) || dyn->insts[ninst].nat_flags_fusion)) {
        ZEROUP(s1);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s2);
    }
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit INC16 instruction, from s1, store result in s1 using s3 and s4 as scratch
void emit_inc16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    IFXORNAT (X_ALL) {
        ANDI(xFlags, xFlags, ~((1UL << F_AF) | (1UL << F_OF2) | (1UL << F_ZF) | (1UL << F_SF) | (1UL << F_PF)));
    }
    IFX (X_PEND) {
        SH(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s3, d_inc16);
    } else IFXORNAT (X_ZF | X_OF | X_AF | X_SF | X_PF) {
        SET_DFNONE();
    }
    IFX (X_AF | X_OF) {
        ORI(s3, s1, 1);  // s3 = op1 | op2
        ANDI(s4, s1, 1); // s4 = op1 & op2
    }

    ADDI(s1, s1, 1);

    IFX (X_PEND) {
        SH(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_AF | X_OF) {
        if (cpuext.zbb) {
            ANDN(s3, s3, s1); // s3 = ~res & (op1 | op2)
        } else {
            NOT(s2, s1);     // s2 = ~res
            AND(s3, s2, s3); // s3 = ~res & (op1 | op2)
        }
        OR(s3, s3, s4); // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX (X_AF) {
            ANDI(s4, s3, 0x08); // AF: cc & 0x08
            SET_FLAGS_NEZ(s4, F_AF, s5);
        }
        IFX (X_OF) {
            SRLI(s3, s3, 14);
            SRLI(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            SET_FLAGS_NEZ(s3, F_OF2, s5);
        }
    }

    ZEXTH(s1, s1);

    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s5);
    }
    IFX (X_SF) {
        SRLI(s3, s1, 15);
        SET_FLAGS_NEZ(s3, F_SF, s5);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit DEC16 instruction, from s1, store result in s1 using s3 and s4 as scratch
void emit_dec16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    IFXORNAT (X_ALL) {
        ANDI(xFlags, xFlags, ~((1UL << F_AF) | (1UL << F_OF2) | (1UL << F_ZF) | (1UL << F_SF) | (1UL << F_PF)));
    }
    IFX (X_PEND) {
        SH(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s3, d_dec16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    IFX (X_AF | X_OF) {
        NOT(s5, s1);
        ORI(s3, s5, 1);  // s3 = ~op1 | op2
        ANDI(s5, s5, 1); // s5 = ~op1 & op2
    }

    ADDIW(s1, s1, -1);

    IFX (X_PEND) {
        SH(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_AF | X_OF) {
        AND(s3, s1, s3); // s3 = res & (~op1 | op2)
        OR(s3, s3, s5);  // cc = (res & (~op1 | op2)) | (~op1 & op2)
        IFX (X_AF) {
            ANDI(s2, s3, 0x08); // AF: cc & 0x08
            SET_FLAGS_NEZ(s2, F_AF, s5);
        }
        IFX (X_OF) {
            SRLI(s3, s3, 14);
            SRLI(s2, s3, 1);
            XOR(s3, s3, s2);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            SET_FLAGS_NEZ(s3, F_OF2, s5);
        }
    }
    SLLIW(s1, s1, 16);
    IFX (X_SF) {
        SET_FLAGS_LTZ(s1, F_SF, s4, s5);
    }
    SRLIW(s1, s1, 16);
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s2);
    }
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s5);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SBB8 instruction, from s1, s2, store result in s1 using s3, s4 and s5 as scratch
void emit_sbb8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    IFX (X_PEND) {
        SB(s1, xEmu, offsetof(x64emu_t, op1));
        SB(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, d_sbb8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFX (X_AF | X_CF | X_OF) {
        // for later flag calculation
        NOT(s5, s1);
    }

    SUBW(s1, s1, s2);
    ANDI(s3, xFlags, 1 << F_CF);
    SUBW(s1, s1, s3);
    ANDI(s1, s1, 0xff);

    CLEAR_FLAGS();
    IFX (X_PEND) {
        SB(s1, xEmu, offsetof(x64emu_t, res));
    }

    CALC_SUB_FLAGS(s5, s2, s1, s3, s4, 8);
    IFX (X_SF) {
        SRLI(s3, s1, 7);
        SET_FLAGS_NEZ(s3, F_SF, s5);
    }
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s5);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit ADC8 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_adc8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    IFX (X_PEND) {
        SH(s1, xEmu, offsetof(x64emu_t, op1));
        SH(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, d_adc8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    IFX (X_AF | X_OF) {
        OR(s4, s1, s2);  // s4 = op1 | op2
        AND(s5, s1, s2); // s5 = op1 & op2
    }

    ADD(s1, s1, s2);
    ANDI(s3, xFlags, 1 << F_CF);
    ADD(s1, s1, s3);

    CLEAR_FLAGS();
    IFX (X_PEND) {
        SW(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_AF | X_OF) {
        if (cpuext.zbb) {
            ANDN(s3, s4, s1); // s3 = ~res & (op1 | op2)
        } else {
            NOT(s2, s1);     // s2 = ~res
            AND(s3, s2, s4); // s3 = ~res & (op1 | op2)
        }
        OR(s3, s3, s5); // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX (X_AF) {
            ANDI(s4, s3, 0x08); // AF: cc & 0x08
            SET_FLAGS_NEZ(s4, F_AF, s5);
        }
        IFX (X_OF) {
            SRLI(s3, s3, 6);
            SRLI(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            SET_FLAGS_NEZ(s3, F_OF2, s5);
        }
    }
    IFX (X_CF) {
        SRLI(s3, s1, 8);
        SET_FLAGS_NEZ(s3, F_CF, s5);
    }

    ANDI(s1, s1, 0xff);

    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s5);
    }
    IFX (X_SF) {
        SRLI(s3, s1, 7);
        SET_FLAGS_NEZ(s3, F_SF, s5);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit ADC8 instruction, from s1, const c, store result in s1 using s3, s4, s5 and s6 as scratch
void emit_adc8c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4, int s5, int s6)
{
    MOV32w(s5, c & 0xff);
    emit_adc8(dyn, ninst, s1, s5, s3, s4, s6);
}

// emit SBB8 instruction, from s1, constant c, store result in s1 using s3, s4, s5 and s6 as scratch
void emit_sbb8c(dynarec_rv64_t* dyn, int ninst, int s1, int c, int s3, int s4, int s5, int s6)
{
    MOV32w(s6, c & 0xff);
    emit_sbb8(dyn, ninst, s1, s6, s3, s4, s5);
}

// emit SBB16 instruction, from s1, s2, store result in s1 using s3, s4 and s5 as scratch
void emit_sbb16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    IFX (X_PEND) {
        SH(s1, xEmu, offsetof(x64emu_t, op1));
        SH(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, d_sbb16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFX (X_AF | X_CF | X_OF) {
        // for later flag calculation
        NOT(s5, s1);
    }

    SUBW(s1, s1, s2);
    ANDI(s3, xFlags, 1 << F_CF);
    SUBW(s1, s1, s3);

    CLEAR_FLAGS();
    SLLIW(s1, s1, 16);
    IFX (X_SF) {
        SET_FLAGS_LTZ(s1, F_SF, s4, s5);
    }
    SRLIW(s1, s1, 16);

    IFX (X_PEND) {
        SH(s1, xEmu, offsetof(x64emu_t, res));
    }

    CALC_SUB_FLAGS(s5, s2, s1, s3, s4, 16);
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s5);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SBB32 instruction, from s1, s2, store result in s1 using s3, s4 and s5 as scratch
void emit_sbb32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
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
    ANDI(s3, xFlags, 1 << F_CF);
    SUBxw(s1, s1, s3);

    CLEAR_FLAGS();
    IFX (X_SF) {
        SET_FLAGS_LTZ(s1, F_SF, s4, s5);
    }
    if (!rex.w && (IS_GPR(s1) || dyn->insts[ninst].nat_flags_fusion)) {
        ZEROUP(s1);
    }

    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }

    CALC_SUB_FLAGS(s5, s2, s1, s3, s4, rex.w ? 64 : 32);
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s5);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit NEG32 instruction, from s1, store result in s1 using s2 and s3 as scratch
void emit_neg32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    CLEAR_FLAGS();
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s3, rex.w ? d_neg64 : d_neg32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    IFX (X_AF | X_OF) {
        MV(s3, s1); // s3 = op1
    }

    NEGxw(s1, s1);

    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }

    IFX (X_CF) {
        SET_FLAGS_NEZ(s1, F_CF, s4);
    }

    IFX (X_AF | X_OF) {
        OR(s3, s1, s3); // s3 = res | op1
        IFX (X_AF) {
            /* af = bc & 0x8 */
            ANDI(s2, s3, 8);
            SET_FLAGS_NEZ(s2, F_AF, s4);
        }
        IFX (X_OF) {
            /* of = ((bc >> (width-2)) ^ (bc >> (width-1))) & 0x1; */
            SRLI(s2, s3, (rex.w ? 64 : 32) - 2);
            SRLI(s3, s2, 1);
            XOR(s2, s2, s3);
            ANDI(s2, s2, 1);
            SET_FLAGS_NEZ(s2, F_OF2, s4);
        }
    }
    IFX (X_SF) {
        SET_FLAGS_LTZ(s1, F_SF, s4, s5);
    }
    if (!rex.w && (IS_GPR(s1) || dyn->insts[ninst].nat_flags_fusion)) {
        ZEROUP(s1);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s2);
    }
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit NEG16 instruction, from s1, store result in s1 using s2 and s3 as scratch
void emit_neg16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    CLEAR_FLAGS();
    IFX (X_PEND) {
        SH(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s3, d_neg16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    IFX (X_AF | X_OF) {
        MV(s3, s1); // s3 = op1
    }

    NEG(s1, s1);
    ZEXTH(s1, s1);

    IFX (X_PEND) {
        SH(s1, xEmu, offsetof(x64emu_t, res));
    }

    IFX (X_CF) {
        SET_FLAGS_NEZ(s1, F_CF, s4);
    }

    IFX (X_AF | X_OF) {
        OR(s3, s1, s3); // s3 = res | op1
        IFX (X_AF) {
            /* af = bc & 0x8 */
            ANDI(s2, s3, 8);
            SET_FLAGS_NEZ(s2, F_AF, s4);
        }
        IFX (X_OF) {
            /* of = ((bc >> (width-2)) ^ (bc >> (width-1))) & 0x1; */
            SRLI(s2, s3, 14);
            SRLI(s3, s2, 1);
            XOR(s2, s2, s3);
            ANDI(s2, s2, 1);
            SET_FLAGS_NEZ(s2, F_OF2, s4);
        }
    }
    IFX (X_SF) {
        SRLI(s3, s1, 15 - F_SF); // put sign bit in place
        ANDI(s3, s3, 1 << F_SF); // 1<<F_SF is sign bit, so just mask
        OR(xFlags, xFlags, s3);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s2);
    }
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit NEG8 instruction, from s1, store result in s1 using s2 and s3 as scratch
void emit_neg8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    CLEAR_FLAGS();
    IFX (X_PEND) {
        SB(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s3, d_neg8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    IFX (X_AF | X_OF) {
        MV(s3, s1); // s3 = op1
    }

    NEG(s1, s1);
    ANDI(s1, s1, 0xff);

    IFX (X_PEND) {
        SB(s1, xEmu, offsetof(x64emu_t, res));
    }

    IFX (X_CF) {
        SET_FLAGS_NEZ(s1, F_CF, s4);
    }

    IFX (X_AF | X_OF) {
        OR(s3, s1, s3); // s3 = res | op1
        IFX (X_AF) {
            /* af = bc & 0x8 */
            ANDI(s2, s3, 8);
            SET_FLAGS_NEZ(s2, F_AF, s4);
        }
        IFX (X_OF) {
            /* of = ((bc >> (width-2)) ^ (bc >> (width-1))) & 0x1; */
            SRLI(s2, s3, 6);
            SRLI(s3, s2, 1);
            XOR(s2, s2, s3);
            ANDI(s2, s2, 1);
            SET_FLAGS_NEZ(s2, F_OF2, s4);
        }
    }
    IFX (X_SF) {
        ANDI(s3, s1, 1 << F_SF); // 1<<F_SF is sign bit, so just mask
        OR(xFlags, xFlags, s3);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s2);
    }
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit ADC16 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_adc16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    IFX (X_PEND) {
        SH(s1, xEmu, offsetof(x64emu_t, op1));
        SH(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, d_adc16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    IFX (X_AF | X_OF) {
        OR(s4, s1, s2);  // s4 = op1 | op2
        AND(s5, s1, s2); // s5 = op1 & op2
    }

    ADD(s1, s1, s2);
    ANDI(s3, xFlags, 1 << F_CF);
    ADD(s1, s1, s3);

    CLEAR_FLAGS();
    IFX (X_PEND) {
        SW(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_AF | X_OF) {
        if (cpuext.zbb) {
            ANDN(s3, s4, s1); // s3 = ~res & (op1 | op2)
        } else {
            NOT(s2, s1);     // s2 = ~res
            AND(s3, s2, s4); // s3 = ~res & (op1 | op2)
        }
        OR(s3, s3, s5); // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX (X_AF) {
            ANDI(s4, s3, 0x08); // AF: cc & 0x08
            SET_FLAGS_NEZ(s4, F_AF, s5);
        }
        IFX (X_OF) {
            SRLI(s3, s3, 14);
            SRLI(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            SET_FLAGS_NEZ(s3, F_OF2, s5);
        }
    }
    IFX (X_CF) {
        SRLI(s3, s1, 16);
        SET_FLAGS_NEZ(s3, F_CF, s5);
    }

    ZEXTH(s1, s1);

    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s5);
    }
    IFX (X_SF) {
        SRLI(s3, s1, 15);
        SET_FLAGS_NEZ(s3, F_SF, s5);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit ADC32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_adc32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5, int s6)
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
            ZEXTW2(s5, s1);
            if (cpuext.zba)
                ADDUW(s5, s2, s5);
            else {
                ZEXTW2(s4, s2);
                ADD(s5, s5, s4);
            } // lo
            ANDI(s3, xFlags, 1);
            ADD(s5, s5, s3); // add carry
            SRLI(s3, s1, 0x20);
            SRLI(s4, s2, 0x20);
            ADD(s4, s4, s3);
            SRLI(s5, s5, 0x20);
            ADD(s5, s5, s4); // hi
            SRAI(s6, s5, 0x20);
        } else {
            ZEXTW2(s3, s1);
            ZEXTW2(s4, s2);
            ADD(s5, s3, s4);
            ANDI(s3, xFlags, 1);
            ADD(s5, s5, s3); // add carry
            SRLI(s6, s5, 0x20);
        }
    }
    IFX (X_AF | X_OF) {
        OR(s4, s1, s2);  // s4 = op1 | op2
        AND(s5, s1, s2); // s5 = op1 & op2
    }

    ADDxw(s1, s1, s2);
    ANDI(s3, xFlags, 1 << F_CF);
    ADDxw(s1, s1, s3);

    CLEAR_FLAGS();
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_AF | X_OF) {
        if (cpuext.zbb) {
            ANDN(s3, s4, s1); // s3 = ~res & (op1 | op2)
        } else {
            NOT(s3, s1);     // s2 = ~res
            AND(s3, s3, s4); // s3 = ~res & (op1 | op2)
        }
        OR(s3, s3, s5); // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX (X_AF) {
            ANDI(s4, s3, 0x08); // AF: cc & 0x08
            SET_FLAGS_NEZ(s4, F_AF, s5);
        }
        IFX (X_OF) {
            SRLI(s3, s3, rex.w ? 62 : 30);
            SRLI(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            SET_FLAGS_NEZ(s3, F_OF2, s5);
        }
    }
    IFX (X_CF) {
        SET_FLAGS_NEZ(s6, F_CF, s5);
    }
    IFX (X_SF) {
        SET_FLAGS_LTZ(s1, F_SF, s5, s6);
    }
    if (!rex.w && (IS_GPR(s1) || dyn->insts[ninst].nat_flags_fusion)) {
        ZEROUP(s1);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s5);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}
