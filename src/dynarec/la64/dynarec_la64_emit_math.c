#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "la64_emitter.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"

#include "la64_printer.h"
#include "dynarec_la64_private.h"
#include "dynarec_la64_functions.h"
#include "../dynarec_helper.h"

// emit ADD32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_add32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    IFX (X_PEND) {
        if (rex.w) {
            ST_D(s1, xEmu, offsetof(x64emu_t, op1));
            ST_D(s2, xEmu, offsetof(x64emu_t, op2));
        } else {
            ST_W(s1, xEmu, offsetof(x64emu_t, op1));
            ST_W(s2, xEmu, offsetof(x64emu_t, op2));
        }
        SET_DF(s3, rex.w ? d_add64 : d_add32b);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    if (cpuext.lbt) {
        IFX (X_ALL) {
            if (rex.w)
                X64_ADD_D(s1, s2);
            else
                X64_ADD_W(s1, s2);
        }
        ADDxw(s1, s1, s2);
        if (!rex.w) ZEROUP(s1);

        IFX (X_PEND)
            SDxw(s1, xEmu, offsetof(x64emu_t, res));
        return;
    }

    CLEAR_FLAGS(s3);
    IFX (X_CF) {
        if (rex.w) {
            ZEROUP2(s5, s1);
            ZEROUP2(s4, s2);
            ADD_D(s5, s5, s4);
            SRLI_D(s3, s1, 0x20);
            SRLI_D(s4, s2, 0x20);
            ADD_D(s4, s4, s3);
            SRLI_D(s5, s5, 0x20);
            ADD_D(s5, s5, s4); // hi
            SRAI_D(s5, s5, 0x20);
            BEQZ(s5, 8);
            ORI(xFlags, xFlags, 1 << F_CF);
        } else {
            ZEROUP2(s3, s1);
            ZEROUP2(s4, s2);
            ADD_D(s5, s3, s4);
            SRLI_D(s5, s5, 0x20);
            BEQZ(s5, 8);
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
        ANDN(s3, s3, s1); // s3 = ~res & (op1 | op2)
        OR(s3, s3, s4);   // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX (X_AF) {
            ANDI(s4, s3, 0x08); // AF: cc & 0x08
            BEQZ(s4, 8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX (X_OF) {
            SRLI_D(s3, s3, rex.w ? 62 : 30);
            SRLI_D(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            BEQZ(s3, 8);
            ORI(xFlags, xFlags, 1 << F_OF);
        }
    }
    IFX (X_SF) {
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
}

// emit ADD32 instruction, from s1, constant c, store result in s1 using s3 and s4 as scratch
void emit_add32c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s2, int s3, int s4, int s5)
{
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    if ((s1 == xRSP) && (BOX64DRENV(dynarec_safeflags) < 2) && (!dyn->insts || (dyn->insts[ninst].x64.gen_flags == X_PEND) || (!BOX64ENV(dynarec_df) && (dyn->insts[ninst].x64.gen_flags == X_ALL)))) {
        // special case when doing math on ESP and only PEND is needed: ignoring it!
        if (c >= -2048 && c < 2048) {
            ADDI_D(s1, s1, c);
        } else {
            MOV64x(s2, c);
            ADD_D(s1, s1, s2);
        }
        if (!rex.w) { ZEROUP(s1); }
        return;
    }
    IFX (X_PEND | X_AF | X_CF | X_OF) {
        MOV64xw(s2, c);
    } else if (cpuext.lbt) {
        MOV64xw(s2, c);
    }
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SDxw(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, rex.w ? d_add64 : d_add32b);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    if (cpuext.lbt) {
        IFX (X_ALL) {
            if (rex.w)
                X64_ADD_D(s1, s2);
            else
                X64_ADD_W(s1, s2);
        }
        ADDxw(s1, s1, s2);
        if (!rex.w) ZEROUP(s1);

        IFX (X_PEND) SDxw(s1, xEmu, offsetof(x64emu_t, res));
        return;
    }

    CLEAR_FLAGS(s3);
    IFX (X_CF) {
        if (rex.w) {
            ZEROUP2(s5, s1);
            ZEROUP2(s4, s2);
            ADD_D(s5, s5, s4);
            SRLI_D(s3, s1, 0x20);
            SRLI_D(s4, s2, 0x20);
            ADD_D(s4, s4, s3);
            SRLI_D(s5, s5, 0x20);
            ADD_D(s5, s5, s4); // hi
            SRAI_D(s5, s5, 0x20);
            BEQZ(s5, 8);
            ORI(xFlags, xFlags, 1 << F_CF);
        } else {
            ZEROUP2(s3, s1);
            ZEROUP2(s4, s2);
            ADD_D(s5, s3, s4);
            SRLI_D(s5, s5, 0x20);
            BEQZ(s5, 8);
            ORI(xFlags, xFlags, 1 << F_CF);
        }
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
        ANDN(s3, s3, s1); // s3 = ~res & (op1 | op2)
        OR(s3, s3, s4);   // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX (X_AF) {
            ANDI(s4, s3, 0x08); // AF: cc & 0x08
            BEQZ(s4, 8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX (X_OF) {
            SRLI_D(s3, s3, rex.w ? 62 : 30);
            SRLI_D(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            BEQZ(s3, 8);
            ORI(xFlags, xFlags, 1 << F_OF);
        }
    }
    IFX (X_SF) {
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
}

// emit ADD8 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_add8(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    IFX (X_PEND) {
        ST_B(s1, xEmu, offsetof(x64emu_t, op1));
        ST_B(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, d_add8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    if (cpuext.lbt) {
        IFX (X_ALL) {
            X64_ADD_B(s1, s2);
        }
        ADD_D(s1, s1, s2);
        ANDI(s1, s1, 0xff);
        IFX (X_PEND)
            ST_H(s1, xEmu, offsetof(x64emu_t, res));
        return;
    }

    CLEAR_FLAGS(s3);
    IFX (X_AF | X_OF) {
        OR(s3, s1, s2);  // s3 = op1 | op2
        AND(s4, s1, s2); // s4 = op1 & op2
    }
    ADD_D(s1, s1, s2);

    IFX (X_AF | X_OF) {
        ANDN(s3, s3, s1); // s3 = ~res & (op1 | op2)
        OR(s3, s3, s4);   // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX (X_AF) {
            ANDI(s4, s3, 0x08); // AF: cc & 0x08
            BEQZ(s4, 8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX (X_OF) {
            SRLI_D(s3, s3, 6);
            SRLI_D(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            BEQZ(s3, 8);
            ORI(xFlags, xFlags, 1 << F_OF);
        }
    }
    IFX (X_CF) {
        SRLI_D(s3, s1, 8);
        BEQZ(s3, 8);
        ORI(xFlags, xFlags, 1 << F_CF);
    }
    IFX (X_PEND) {
        ST_H(s1, xEmu, offsetof(x64emu_t, res));
    }
    ANDI(s1, s1, 0xff);
    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_SF) {
        SRLI_D(s3, s1, 7);
        BEQZ(s3, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit ADD8 instruction, from s1, const c, store result in s1 using s3 and s4 as scratch
void emit_add8c(dynarec_la64_t* dyn, int ninst, int s1, int c, int s2, int s3, int s4)
{
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    IFX (X_PEND) {
        MOV32w(s4, c & 0xff);
        ST_B(s1, xEmu, offsetof(x64emu_t, op1));
        ST_B(s4, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, d_add8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    if (cpuext.lbt) {
        IFX (X_ALL) {
            IFX (X_PEND) {
            } else {
                MOV32w(s4, c & 0xff);
            }
            X64_ADD_B(s1, s4);
        }
        ADDI_D(s1, s1, c & 0xff);
        ANDI(s1, s1, 0xff);
        IFX (X_PEND)
            ST_H(s1, xEmu, offsetof(x64emu_t, res));
        return;
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
    ADDI_D(s1, s1, c & 0xff);

    IFX (X_AF | X_OF) {
        ANDN(s3, s3, s1); // s3 = ~res & (op1 | op2)
        OR(s3, s3, s4);   // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX (X_AF) {
            ANDI(s4, s3, 0x08); // AF: cc & 0x08
            BEQZ(s4, 8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX (X_OF) {
            SRLI_D(s3, s3, 6);
            SRLI_D(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            BEQZ(s3, 8);
            ORI(xFlags, xFlags, 1 << F_OF);
        }
    }
    IFX (X_CF) {
        SRLI_D(s3, s1, 8);
        BEQZ(s3, 8);
        ORI(xFlags, xFlags, 1 << F_CF);
    }
    IFX (X_PEND) {
        ST_H(s1, xEmu, offsetof(x64emu_t, res));
    }
    ANDI(s1, s1, 0xff);
    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_SF) {
        SRLI_D(s3, s1, 7);
        BEQZ(s3, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit ADD16 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_add16(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    IFX (X_PEND) {
        ST_H(s1, xEmu, offsetof(x64emu_t, op1));
        ST_H(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, d_add16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    IFXA (X_AF | X_OF, !cpuext.lbt) {
        OR(s3, s1, s2);  // s3 = op1 | op2
        AND(s4, s1, s2); // s4 = op1 & op2
    }

    IFXA (X_ALL, cpuext.lbt) {
        X64_ADD_H(s1, s2);
    }

    ADD_D(s1, s1, s2);

    IFX (X_PEND) {
        ST_W(s1, xEmu, offsetof(x64emu_t, res));
    }

    if (cpuext.lbt) {
        BSTRPICK_D(s1, s1, 15, 0);
        return;
    }

    CLEAR_FLAGS(s5);
    IFX (X_AF | X_OF) {
        ANDN(s3, s3, s1); // s3 = ~res & (op1 | op2)
        OR(s3, s3, s4);   // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX (X_AF) {
            ANDI(s4, s3, 0x08); // AF: cc & 0x08
            BEQZ(s4, 8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX (X_OF) {
            SRLI_D(s3, s3, 14);
            SRLI_D(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            BEQZ(s3, 8);
            ORI(xFlags, xFlags, 1 << F_OF);
        }
    }

    IFX (X_CF) {
        SRLI_D(s3, s1, 16);
        BEQZ(s3, 8);
        ORI(xFlags, xFlags, 1 << F_CF);
    }

    BSTRPICK_D(s1, s1, 15, 0);

    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_SF) {
        SRLI_D(s3, s1, 15);
        BEQZ(s3, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit SUB8 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_sub8(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    IFX (X_PEND) {
        ST_B(s1, xEmu, offsetof(x64emu_t, op1));
        ST_B(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, d_sub8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    if (cpuext.lbt) {
        IFX (X_ALL) {
            X64_SUB_B(s1, s2);
        }
        SUB_D(s1, s1, s2);
        ANDI(s1, s1, 0xff);
        IFX (X_PEND)
            ST_H(s1, xEmu, offsetof(x64emu_t, res));
        return;
    }

    CLEAR_FLAGS(s3);
    IFX (X_AF | X_CF | X_OF) {
        // for later flag calculation
        NOR(s5, xZR, s1);
    }

    SUB_D(s1, s1, s2);
    ANDI(s1, s1, 0xff);
    IFX (X_SF) {
        SRLI_D(s3, s1, 7);
        BEQZ(s3, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX (X_PEND) {
        ST_B(s1, xEmu, offsetof(x64emu_t, res));
    }
    CALC_SUB_FLAGS(s5, s2, s1, s3, s4, 8);
    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit SUB8 instruction, from s1, constant c, store result in s1 using s3 and s4 as scratch
void emit_sub8c(dynarec_la64_t* dyn, int ninst, int s1, int c, int s2, int s3, int s4, int s5)
{
    MOV32w(s2, c & 0xff);
    emit_sub8(dyn, ninst, s1, s2, s3, s4, s5);
}

// emit SUB16 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_sub16(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    IFX (X_PEND) {
        ST_H(s1, xEmu, offsetof(x64emu_t, op1));
        ST_H(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, d_sub16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFXA (X_AF | X_CF | X_OF, !cpuext.lbt) {
        // for later flag calculation
        NOR(s5, xZR, s1);
    }

    IFXA (X_ALL, cpuext.lbt) {
        X64_SUB_H(s1, s2);
    }

    SUB_W(s1, s1, s2);
    IFX (X_PEND) {
        ST_H(s1, xEmu, offsetof(x64emu_t, res));
    }

    if (cpuext.lbt) {
        BSTRPICK_D(s1, s1, 15, 0);
        return;
    }

    CLEAR_FLAGS(s3);
    SLLI_D(s1, s1, 48);
    IFX (X_SF) {
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    SRLI_D(s1, s1, 48);

    CALC_SUB_FLAGS(s5, s2, s1, s3, s4, 16);
    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit SUB32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_sub32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SDxw(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, rex.w ? d_sub64 : d_sub32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    if (cpuext.lbt) {
        IFX (X_ALL) {
            if (rex.w)
                X64_SUB_D(s1, s2);
            else
                X64_SUB_W(s1, s2);
        }
        SUBxw(s1, s1, s2);
        if (!rex.w) ZEROUP(s1);

        IFX (X_PEND)
            SDxw(s1, xEmu, offsetof(x64emu_t, res));
        return;
    }

    CLEAR_FLAGS(s3);
    IFX (X_AF | X_CF | X_OF) {
        // for later flag calculation
        NOR(s5, xZR, s1);
    }

    SUBxw(s1, s1, s2);
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_SF) {
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    CALC_SUB_FLAGS(s5, s2, s1, s3, s4, rex.w ? 64 : 32);
    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit SUB32 instruction, from s1, constant c, store result in s1 using s2, s3, s4 and s5 as scratch
void emit_sub32c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s2, int s3, int s4, int s5)
{
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    if ((s1 == xRSP) && (BOX64DRENV(dynarec_safeflags) < 2) && (!dyn->insts || (dyn->insts[ninst].x64.gen_flags == X_PEND) || (!BOX64ENV(dynarec_df) && (dyn->insts[ninst].x64.gen_flags == X_ALL)))) {
        // special case when doing math on RSP and only PEND is needed: ignoring it!
        if (c > -2048 && c <= 2048) {
            ADDI_D(s1, s1, -c);
        } else {
            MOV64xw(s2, c);
            SUBxw(s1, s1, s2);
        }
        if (!rex.w) { ZEROUP(s1); }
        return;
    }

    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        MOV64xw(s2, c);
        SDxw(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, rex.w ? d_sub64 : d_sub32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    if (cpuext.lbt) {
        IFX (X_PEND) {
        } else {
            MOV64xw(s2, c);
        }
        IFX (X_ALL) {
            if (rex.w) {
                X64_SUB_D(s1, s2);
            } else {
                X64_SUB_W(s1, s2);
            }
        }
        SUBxw(s1, s1, s2);
        if (!rex.w) ZEROUP(s1);

        IFX (X_PEND)
            SDxw(s1, xEmu, offsetof(x64emu_t, res));
        return;
    }

    CLEAR_FLAGS(s3);
    IFX (X_AF | X_CF | X_OF) {
        // for later flag calculation
        NOR(s5, xZR, s1);
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
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    CALC_SUB_FLAGS(s5, s2, s1, s3, s4, rex.w ? 64 : 32);
    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}


// emit SBB8 instruction, from s1, s2, store result in s1 using s3, s4 and s5 as scratch
void emit_sbb8(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    IFX (X_PEND) {
        ST_B(s1, xEmu, offsetof(x64emu_t, op1));
        ST_B(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, d_sbb8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }


    if (cpuext.lbt) {
        SBC_B(s3, s1, s2);
        IFX (X_ALL) {
            X64_SBC_B(s1, s2);
        }
        ANDI(s1, s3, 0xff);
        IFX (X_PEND)
            ST_B(s1, xEmu, offsetof(x64emu_t, res));
        return;
    }


    IFX (X_AF | X_CF | X_OF) {
        // for later flag calculation
        NOR(s5, xZR, s1);
    }

    SUB_W(s1, s1, s2);
    ANDI(s3, xFlags, 1 << F_CF);
    SUB_W(s1, s1, s3);
    ANDI(s1, s1, 0xff);

    CLEAR_FLAGS(s3);
    IFX (X_PEND) {
        ST_B(s1, xEmu, offsetof(x64emu_t, res));
    }

    CALC_SUB_FLAGS(s5, s2, s1, s3, s4, 8);
    IFX (X_SF) {
        SRLI_D(s3, s1, 7);
        BEQZ(s3, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit SBB8 instruction, from s1, constant c, store result in s1 using s3, s4, s5 and s6 as scratch
void emit_sbb8c(dynarec_la64_t* dyn, int ninst, int s1, int c, int s3, int s4, int s5, int s6)
{
    MOV32w(s6, c & 0xff);
    emit_sbb8(dyn, ninst, s1, s6, s3, s4, s5);
}

// emit SBB16 instruction, from s1, s2, store result in s1 using s3, s4 and s5 as scratch
void emit_sbb16(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    IFX (X_PEND) {
        ST_H(s1, xEmu, offsetof(x64emu_t, op1));
        ST_H(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, d_sbb16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    if (cpuext.lbt) {
        SBC_H(s3, s1, s2);

        IFX (X_ALL) {
            X64_SBC_H(s1, s2);
        }
        BSTRPICK_D(s1, s3, 15, 0);
        IFX (X_PEND)
            ST_H(s1, xEmu, offsetof(x64emu_t, res));
        return;
    }

    IFX (X_AF | X_CF | X_OF) {
        // for later flag calculation
        NOR(s5, xZR, s1);
    }

    SUB_W(s1, s1, s2);
    ANDI(s3, xFlags, 1 << F_CF);
    SUB_W(s1, s1, s3);

    CLEAR_FLAGS(s3);
    SLLI_W(s1, s1, 16);
    IFX (X_SF) {
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    SRLI_W(s1, s1, 16);

    IFX (X_PEND) {
        ST_H(s1, xEmu, offsetof(x64emu_t, res));
    }

    CALC_SUB_FLAGS(s5, s2, s1, s3, s4, 16);
    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit SBB32 instruction, from s1, s2, store result in s1 using s3, s4 and s5 as scratch
void emit_sbb32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SDxw(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, rex.w ? d_sbb64 : d_sbb32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    if (cpuext.lbt) {
        if (rex.w) {
            SBC_D(s3, s1, s2);
        } else {
            SBC_W(s3, s1, s2);
        }
        IFX (X_ALL) {
            if (rex.w)
                X64_SBC_D(s1, s2);
            else
                X64_SBC_W(s1, s2);
        }
        IFXA (X_AF, BOX64DRENV(dynarec_safeflags)) NOR(s5, xZR, s1);
        if (rex.w)
            MV(s1, s3);
        else
            ZEROUP2(s1, s3);
        IFXA (X_AF, BOX64DRENV(dynarec_safeflags)) {
            // bc = (res & (~op1 | op2)) | (~op1 & op2)
            OR(s3, s5, s2);
            AND(s4, s1, s3);
            AND(s5, s5, s2);
            OR(s4, s4, s5);
            // af = bc & 0x8
            SLLI_D(s3, s4, F_AF - 3);
            X64_SET_EFLAGS(s3, X_AF);
        }
        IFX (X_PEND)
            SDxw(s1, xEmu, offsetof(x64emu_t, res));
        return;
    }

    IFX (X_AF | X_CF | X_OF) {
        // for later flag calculation
        NOR(s5, xZR, s1);
    }

    SUBxw(s1, s1, s2);
    ANDI(s3, xFlags, 1 << F_CF);
    SUBxw(s1, s1, s3);

    CLEAR_FLAGS(s3);
    IFX (X_SF) {
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) ZEROUP(s1);
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    CALC_SUB_FLAGS(s5, s2, s1, s3, s4, rex.w ? 64 : 32);
    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}


// emit NEG8 instruction, from s1, store result in s1 using s2 and s3 as scratch
void emit_neg8(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3)
{
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    IFX (X_PEND) {
        ST_B(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s3, d_neg8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    IFX (X_AF | X_OF) {
        MV(s3, s1); // s3 = op1
    }

    IFXA (X_ALL, cpuext.lbt) {
        X64_SUB_B(xZR, s1);
    }

    NEG_D(s1, s1);
    ANDI(s1, s1, 0xff);
    IFX (X_PEND) {
        ST_B(s1, xEmu, offsetof(x64emu_t, res));
    }

    if (cpuext.lbt) return;

    CLEAR_FLAGS(s2);
    IFX (X_CF) {
        BEQZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_CF);
    }

    IFX (X_AF | X_OF) {
        OR(s3, s1, s3); // s3 = res | op1
        IFX (X_AF) {
            /* af = bc & 0x8 */
            ANDI(s2, s3, 8);
            BEQZ(s2, 8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX (X_OF) {
            /* of = ((bc >> (width-2)) ^ (bc >> (width-1))) & 0x1; */
            SRLI_D(s2, s3, 6);
            SRLI_D(s3, s2, 1);
            XOR(s2, s2, s3);
            ANDI(s2, s2, 1);
            BEQZ(s2, 8);
            ORI(xFlags, xFlags, 1 << F_OF);
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
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
}

// emit NEG32 instruction, from s1, store result in s1 using s2 and s3 as scratch
void emit_neg32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3)
{
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s3, rex.w ? d_neg64 : d_neg32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    if (!cpuext.lbt) {
        IFX (X_AF | X_OF) {
            MV(s3, s1); // s3 = op1
        }
    }

    IFXA (X_ALL, cpuext.lbt) {
        if (rex.w)
            X64_SUB_D(xZR, s1);
        else
            X64_SUB_W(xZR, s1);
    }

    NEGxw(s1, s1);
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }

    if (cpuext.lbt) {
        if (!rex.w) ZEROUP(s1);
        return;
    }

    CLEAR_FLAGS(s2);
    IFX (X_CF) {
        BEQZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_CF);
    }

    IFX (X_AF | X_OF) {
        OR(s3, s1, s3); // s3 = res | op1
        IFX (X_AF) {
            /* af = bc & 0x8 */
            ANDI(s2, s3, 8);
            BEQZ(s2, 8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX (X_OF) {
            /* of = ((bc >> (width-2)) ^ (bc >> (width-1))) & 0x1; */
            SRLI_D(s2, s3, (rex.w ? 64 : 32) - 2);
            SRLI_D(s3, s2, 1);
            XOR(s2, s2, s3);
            ANDI(s2, s2, 1);
            BEQZ(s2, 8);
            ORI(xFlags, xFlags, 1 << F_OF);
        }
    }
    IFX (X_SF) {
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s2);
    }
    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
}

// emit ADC8 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_adc8(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    IFX (X_PEND) {
        ST_H(s1, xEmu, offsetof(x64emu_t, op1));
        ST_H(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, d_adc8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    if (cpuext.lbt) {
        ADC_B(s3, s1, s2);

        IFX (X_ALL) {
            X64_ADC_B(s1, s2);
        }
        ANDI(s1, s3, 0xff);
        IFX (X_PEND) {
            ST_W(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }

    IFX (X_AF | X_OF) {
        OR(s4, s1, s2);  // s4 = op1 | op2
        AND(s5, s1, s2); // s5 = op1 & op2
    }

    ADD_D(s1, s1, s2);
    ANDI(s3, xFlags, 1 << F_CF);
    ADD_D(s1, s1, s3);

    CLEAR_FLAGS(s3);
    IFX (X_PEND) {
        ST_W(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_AF | X_OF) {
        ANDN(s3, s4, s1); // s3 = ~res & (op1 | op2)
        OR(s3, s3, s5);   // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX (X_AF) {
            ANDI(s4, s3, 0x08); // AF: cc & 0x08
            BEQZ(s4, 8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX (X_OF) {
            SRLI_D(s3, s3, 6);
            SRLI_D(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            BEQZ(s3, 8);
            ORI(xFlags, xFlags, 1 << F_OF);
        }
    }
    IFX (X_CF) {
        SRLI_D(s3, s1, 8);
        BEQZ(s3, 8);
        ORI(xFlags, xFlags, 1 << F_CF);
    }

    ANDI(s1, s1, 0xff);

    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_SF) {
        SRLI_D(s3, s1, 7);
        BEQZ(s3, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit ADC8 instruction, from s1, const c, store result in s1 using s3, s4, s5 and s6 as scratch
void emit_adc8c(dynarec_la64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4, int s5, int s6)
{
    MOV32w(s5, c & 0xff);
    emit_adc8(dyn, ninst, s1, s5, s3, s4, s6);
}

// emit ADC16 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_adc16(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    IFX (X_PEND) {
        ST_H(s1, xEmu, offsetof(x64emu_t, op1));
        ST_H(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, d_adc16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    if (cpuext.lbt) {
        ADC_H(s3, s1, s2);

        IFX (X_ALL) {
            X64_ADC_H(s1, s2);
        }
        BSTRPICK_D(s1, s3, 15, 0);
        IFX (X_PEND) {
            ST_W(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }

    IFX (X_AF | X_OF) {
        OR(s4, s1, s2);  // s4 = op1 | op2
        AND(s5, s1, s2); // s5 = op1 & op2
    }

    ADD_D(s1, s1, s2);
    ANDI(s3, xFlags, 1 << F_CF);
    ADD_D(s1, s1, s3);

    CLEAR_FLAGS(s3);
    IFX (X_PEND) {
        ST_W(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_AF | X_OF) {
        ANDN(s3, s4, s1); // s3 = ~res & (op1 | op2)
        OR(s3, s3, s5);   // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX (X_AF) {
            ANDI(s4, s3, 0x08); // AF: cc & 0x08
            BEQZ(s4, 8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX (X_OF) {
            SRLI_D(s3, s3, 14);
            SRLI_D(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            BEQZ(s3, 8);
            ORI(xFlags, xFlags, 1 << F_OF);
        }
    }
    IFX (X_CF) {
        SRLI_D(s3, s1, 16);
        BEQZ(s3, 8);
        ORI(xFlags, xFlags, 1 << F_CF);
    }

    BSTRPICK_D(s1, s1, 15, 0);

    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_SF) {
        SRLI_D(s3, s1, 15);
        BEQZ(s3, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit ADC32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_adc32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5, int s6)
{
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SDxw(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, rex.w ? d_adc64 : d_adc32b);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    if (cpuext.lbt) {
        if (rex.w)
            ADC_D(s3, s1, s2);
        else
            ADC_W(s3, s1, s2);

        IFX (X_ALL) {
            if (rex.w)
                X64_ADC_D(s1, s2);
            else
                X64_ADC_W(s1, s2);
        }
        if (rex.w)
            MV(s1, s3);
        else
            ZEROUP2(s1, s3);
        IFX (X_PEND) {
            SDxw(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }

    IFX (X_CF) {
        if (rex.w) {
            ZEROUP2(s5, s1);
            ZEROUP2(s4, s2);
            ADD_D(s5, s5, s4); // lo
            ANDI(s3, xFlags, 1);
            ADD_D(s5, s5, s3); // add carry
            SRLI_D(s3, s1, 0x20);
            SRLI_D(s4, s2, 0x20);
            ADD_D(s4, s4, s3);
            SRLI_D(s5, s5, 0x20);
            ADD_D(s5, s5, s4); // hi
            SRAI_D(s6, s5, 0x20);
        } else {
            ZEROUP2(s3, s1);
            ZEROUP2(s4, s2);
            ADD_D(s5, s3, s4);
            ANDI(s3, xFlags, 1);
            ADD_D(s5, s5, s3); // add carry
            SRLI_D(s6, s5, 0x20);
        }
    }
    IFX (X_AF | X_OF) {
        OR(s4, s1, s2);  // s4 = op1 | op2
        AND(s5, s1, s2); // s5 = op1 & op2
    }

    ADDxw(s1, s1, s2);
    ANDI(s3, xFlags, 1 << F_CF);
    ADDxw(s1, s1, s3);

    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }

    CLEAR_FLAGS(s3);
    IFX (X_CF) {
        BEQZ(s6, 8);
        ORI(xFlags, xFlags, 1 << F_CF);
    }
    IFX (X_AF | X_OF) {
        ANDN(s3, s4, s1); // s3 = ~res & (op1 | op2)
        OR(s3, s3, s5);   // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX (X_AF) {
            ANDI(s4, s3, 0x08); // AF: cc & 0x08
            BEQZ(s4, 8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX (X_OF) {
            SRLI_D(s3, s3, rex.w ? 62 : 30);
            SRLI_D(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            BEQZ(s3, 8);
            ORI(xFlags, xFlags, 1 << F_OF);
        }
    }
    IFX (X_SF) {
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
}

// emit INC8 instruction, from s1, store result in s1 using s2, s3 and s4 as scratch
void emit_inc8(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    IFX (X_PEND) {
        ST_B(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s3, d_inc8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFXA (X_ALL, !cpuext.lbt) {
        // preserving CF
        MOV64x(s4, (1UL << F_AF) | (1UL << F_OF) | (1UL << F_ZF) | (1UL << F_SF) | (1UL << F_PF));
        ANDN(xFlags, xFlags, s4);
    }

    IFXA (X_AF | X_OF, !cpuext.lbt) {
        ORI(s3, s1, 1);  // s3 = op1 | op2
        ANDI(s4, s1, 1); // s5 = op1 & op2
    }

    IFXA (X_ALL, cpuext.lbt) {
        X64_INC_B(s1);
    }

    ADDI_W(s1, s1, 1);

    IFX (X_PEND) {
        ST_B(s1, xEmu, offsetof(x64emu_t, res));
    }

    if (cpuext.lbt) {
        ANDI(s1, s1, 0xff);
        return;
    }

    IFX (X_AF | X_OF) {
        ANDN(s3, s3, s1); // s3 = ~res & (op1 | op2)
        OR(s3, s3, s4);   // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX (X_AF) {
            ANDI(s2, s3, 0x08); // AF: cc & 0x08
            BEQZ(s2, 8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX (X_OF) {
            SRLI_D(s3, s3, 6);
            SRLI_D(s2, s3, 1);
            XOR(s3, s3, s2);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            BEQZ(s3, 8);
            ORI(xFlags, xFlags, 1 << F_OF);
        }
    }
    IFX (X_SF) {
        ANDI(s2, s1, 0x80);
        BEQZ(s2, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    ANDI(s1, s1, 0xff);
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s2);
    }
    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
}

// emit INC16 instruction, from s1, store result in s1 using s3 and s4 as scratch
void emit_inc16(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    IFX (X_PEND) {
        ST_H(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s3, d_inc16);
    } else IFXORNAT (X_ZF | X_OF | X_AF | X_SF | X_PF) {
        SET_DFNONE();
    }

    IFXA (X_ALL, !cpuext.lbt) {
        // preserving CF
        MOV64x(s4, (1UL << F_AF) | (1UL << F_OF) | (1UL << F_ZF) | (1UL << F_SF) | (1UL << F_PF));
        ANDN(xFlags, xFlags, s4);
    }

    IFXA (X_AF | X_OF, !cpuext.lbt) {
        ORI(s3, s1, 1);  // s3 = op1 | op2
        ANDI(s4, s1, 1); // s4 = op1 & op2
    }

    IFXA (X_ALL, cpuext.lbt) {
        X64_INC_H(s1);
    }

    ADDI_D(s1, s1, 1);

    IFX (X_PEND) {
        ST_H(s1, xEmu, offsetof(x64emu_t, res));
    }

    if (cpuext.lbt) {
        BSTRPICK_D(s1, s1, 15, 0);
        return;
    }

    IFX (X_AF | X_OF) {
        ANDN(s3, s3, s1); // s3 = ~res & (op1 | op2)
        OR(s3, s3, s4);   // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX (X_AF) {
            ANDI(s4, s3, 0x08); // AF: cc & 0x08
            BEQZ(s4, 8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX (X_OF) {
            SRLI_D(s3, s3, 14);
            SRLI_D(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            BEQZ(s3, 8);
            ORI(xFlags, xFlags, 1 << F_OF);
        }
    }

    BSTRPICK_D(s1, s1, 15, 0);

    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_SF) {
        SRLI_D(s3, s1, 15);
        BEQZ(s3, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit INC32 instruction, from s1, store result in s1 using s3 and s4 as scratch
void emit_inc32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s3, rex.w ? d_inc64 : d_inc32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFXA (X_ALL, !cpuext.lbt) {
        // preserving CF
        MOV64x(s4, (1UL << F_AF) | (1UL << F_OF) | (1UL << F_ZF) | (1UL << F_SF) | (1UL << F_PF));
        ANDN(xFlags, xFlags, s4);
    }

    IFXA (X_AF | X_OF, !cpuext.lbt) {
        ORI(s3, s1, 1);  // s3 = op1 | op2
        ANDI(s5, s1, 1); // s5 = op1 & op2
    }

    IFXA (X_ALL, cpuext.lbt) {
        if (rex.w) {
            X64_INC_D(s1);
        } else {
            X64_INC_W(s1);
        }
    }

    ADDIxw(s1, s1, 1);

    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }

    if (cpuext.lbt) {
        if (!rex.w) ZEROUP(s1);
        return;
    }

    IFX (X_AF | X_OF) {
        ANDN(s3, s3, s1); // s3 = ~res & (op1 | op2)
        OR(s3, s3, s5);   // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX (X_AF) {
            ANDI(s2, s3, 0x08); // AF: cc & 0x08
            BEQZ(s2, 8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX (X_OF) {
            SRLI_D(s3, s3, rex.w ? 62 : 30);
            SRLI_D(s2, s3, 1);
            XOR(s3, s3, s2);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            BEQZ(s3, 8);
            ORI(xFlags, xFlags, 1 << F_OF);
        }
    }
    IFX (X_SF) {
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s2);
    }
    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
}

// emit DEC8 instruction, from s1, store result in s1 using s2, s3 and s4 as scratch
void emit_dec8(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    IFX (X_PEND) {
        ST_B(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s3, d_dec8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFXA (X_ALL, !cpuext.lbt) {
        // preserving CF
        MOV64x(s4, (1UL << F_AF) | (1UL << F_OF) | (1UL << F_ZF) | (1UL << F_SF) | (1UL << F_PF));
        ANDN(xFlags, xFlags, s4);
    }

    IFXA (X_AF | X_OF, !cpuext.lbt) {
        NOR(s4, xZR, s1); // s4 = ~op1
        ORI(s3, s4, 1);   // s3 = ~op1 | op2
        ANDI(s4, s4, 1);  // s4 = ~op1 & op2
    }

    IFXA (X_ALL, cpuext.lbt) {
        X64_DEC_B(s1);
    }

    ADDI_W(s1, s1, -1);

    IFX (X_PEND) {
        ST_B(s1, xEmu, offsetof(x64emu_t, res));
    }

    if (cpuext.lbt) {
        ANDI(s1, s1, 0xff);
        return;
    }

    IFX (X_AF | X_OF) {
        AND(s3, s1, s3); // s3 = res & (~op1 | op2)
        OR(s3, s3, s4);  // cc = (res & (~op1 | op2)) | (~op1 & op2)
        IFX (X_AF) {
            ANDI(s2, s3, 0x08); // AF: cc & 0x08
            BEQZ(s2, 8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX (X_OF) {
            SRLI_D(s3, s3, 6);
            SRLI_D(s2, s3, 1);
            XOR(s3, s3, s2);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            BEQZ(s3, 8);
            ORI(xFlags, xFlags, 1 << F_OF);
        }
    }
    IFX (X_SF) {
        ANDI(s2, s1, 0x80);
        BEQZ(s2, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    ANDI(s1, s1, 0xff);
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s2);
    }
    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
}

// emit DEC16 instruction, from s1, store result in s1 using s3 and s4 as scratch
void emit_dec16(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    IFX (X_PEND) {
        ST_H(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s3, d_dec16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    IFX (X_AF | X_OF) {
        NOR(s5, xZR, s1);
        ORI(s3, s5, 1);  // s3 = ~op1 | op2
        ANDI(s5, s5, 1); // s5 = ~op1 & op2
    }

    IFXA (X_ALL, cpuext.lbt) {
        X64_DEC_H(s1);
    }

    ADDI_W(s1, s1, -1);

    IFX (X_PEND) {
        ST_H(s1, xEmu, offsetof(x64emu_t, res));
    }

    if (cpuext.lbt) {
        BSTRPICK_D(s1, s1, 15, 0);
        return;
    }

    IFX (X_ALL) {
        // preserving CF
        MOV64x(s4, (1UL << F_AF) | (1UL << F_OF) | (1UL << F_ZF) | (1UL << F_SF) | (1UL << F_PF));
        ANDN(xFlags, xFlags, s4);
    }

    IFX (X_AF | X_OF) {
        AND(s3, s1, s3); // s3 = res & (~op1 | op2)
        OR(s3, s3, s5);  // cc = (res & (~op1 | op2)) | (~op1 & op2)
        IFX (X_AF) {
            ANDI(s2, s3, 0x08); // AF: cc & 0x08
            BEQZ(s2, 8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX (X_OF) {
            SRLI_D(s3, s3, 14);
            SRLI_D(s2, s3, 1);
            XOR(s3, s3, s2);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            BEQZ(s3, 8);
            ORI(xFlags, xFlags, 1 << F_OF);
        }
    }
    SLLI_W(s1, s1, 16);
    IFX (X_SF) {
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    SRLI_W(s1, s1, 16);
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s2);
    }
    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
}

// emit DEC32 instruction, from s1, store result in s1 using s3 and s4 as scratch
void emit_dec32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s3, rex.w ? d_dec64 : d_dec32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    IFXA (X_AF | X_OF, !cpuext.lbt) {
        NOR(s5, xZR, s1);
        ORI(s3, s5, 1);  // s3 = ~op1 | op2
        ANDI(s5, s5, 1); // s5 = ~op1 & op2
    }

    IFXA (X_ALL, cpuext.lbt) {
        if (rex.w) {
            X64_DEC_D(s1);
        } else {
            X64_DEC_W(s1);
        }
    }

    ADDIxw(s1, s1, -1);

    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }

    if (cpuext.lbt) {
        if (!rex.w) ZEROUP(s1);
        return;
    }

    IFX (X_ALL) {
        // preserving CF
        MOV64x(s4, (1UL << F_AF) | (1UL << F_OF) | (1UL << F_ZF) | (1UL << F_SF) | (1UL << F_PF));
        ANDN(xFlags, xFlags, s4);
    }
    IFX (X_AF | X_OF) {
        AND(s3, s1, s3); // s3 = res & (~op1 | op2)
        OR(s3, s3, s5);  // cc = (res & (~op1 | op2)) | (~op1 & op2)
        IFX (X_AF) {
            ANDI(s2, s3, 0x08); // AF: cc & 0x08
            BEQZ(s2, 8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX (X_OF) {
            SRLI_D(s3, s3, rex.w ? 62 : 30);
            SRLI_D(s2, s3, 1);
            XOR(s3, s3, s2);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            BEQZ(s3, 8);
            ORI(xFlags, xFlags, 1 << F_OF);
        }
    }
    IFX (X_SF) {
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s2);
    }
    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
}


// emit NEG16 instruction, from s1, store result in s1 using s2 and s3 as scratch
void emit_neg16(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3)
{
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    IFX (X_PEND) {
        ST_H(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s3, d_neg16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    IFX (X_AF | X_OF) {
        MV(s3, s1); // s3 = op1
    }

    NEG_D(s1, s1);
    BSTRPICK_D(s1, s1, 15, 0);
    IFX (X_PEND) {
        ST_H(s1, xEmu, offsetof(x64emu_t, res));
    }

    CLEAR_FLAGS(s3);
    IFX (X_CF) {
        BEQZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_CF);
    }

    IFX (X_AF | X_OF) {
        OR(s3, s1, s3); // s3 = res | op1
        IFX (X_AF) {
            /* af = bc & 0x8 */
            ANDI(s2, s3, 8);
            BEQZ(s2, 8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX (X_OF) {
            /* of = ((bc >> (width-2)) ^ (bc >> (width-1))) & 0x1; */
            SRLI_D(s2, s3, 14);
            SRLI_D(s3, s2, 1);
            XOR(s2, s2, s3);
            ANDI(s2, s2, 1);
            BEQZ(s2, 8);
            ORI(xFlags, xFlags, 1 << F_OF);
        }
    }
    IFX (X_SF) {
        SRLI_D(s3, s1, 15 - F_SF); // put sign bit in place
        ANDI(s3, s3, 1 << F_SF);   // 1<<F_SF is sign bit, so just mask
        OR(xFlags, xFlags, s3);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s2);
    }
    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }

    IFXA (X_ALL, cpuext.lbt) {
        SPILL_EFLAGS();
    }
}
