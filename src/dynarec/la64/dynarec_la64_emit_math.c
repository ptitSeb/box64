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

#include "la64_printer.h"
#include "dynarec_la64_private.h"
#include "dynarec_la64_functions.h"
#include "dynarec_la64_helper.h"

// emit ADD32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_add32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    IFX (X_PEND) {
        if (rex.w) {
            ST_D(s1, xEmu, offsetof(x64emu_t, op1));
            ST_D(s2, xEmu, offsetof(x64emu_t, op2));
        } else {
            ST_W(s1, xEmu, offsetof(x64emu_t, op1));
            ST_W(s2, xEmu, offsetof(x64emu_t, op2));
        }
        SET_DF(s3, rex.w ? d_add64 : d_add32b);
    } else IFX (X_ALL) {
        SET_DFNONE();
    }

    if (la64_lbt) {
        IFX (X_ALL) {
            X64_ADD_WU(s1, s2);
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
            AND(s5, xMASK, s1);
            AND(s4, xMASK, s2);
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
            ADD_D(s5, s1, s2);
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
    if (s1 == xRSP && (!dyn->insts || dyn->insts[ninst].x64.gen_flags == X_PEND)) {
        // special case when doing math on ESP and only PEND is needed: ignoring it!
        if (c >= -2048 && c < 2048) {
            ADDIxw(s1, s1, c);
        } else {
            MOV64xw(s2, c);
            ADDxw(s1, s1, s2);
        }
        return;
    }
    IFX (X_PEND | X_AF | X_CF | X_OF) {
        MOV64xw(s2, c);
    } else if (la64_lbt)
        MOV64xw(s2, c);
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SDxw(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, rex.w ? d_add64 : d_add32b);
    } else IFX (X_ALL) {
        SET_DFNONE();
    }

    if (la64_lbt) {
        IFX (X_ALL) {
            X64_ADD_WU(s1, s2);
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
            AND(s5, xMASK, s1);
            AND(s4, xMASK, s2);
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
            ADD_D(s5, s1, s2);
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
    IFX (X_PEND) {
        ST_B(s1, xEmu, offsetof(x64emu_t, op1));
        ST_B(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, d_add8);
    } else IFX (X_ALL) {
        SET_DFNONE();
    }

    if (la64_lbt) {
        IFX (X_ALL) {
            X64_ADD_B(s1, s2);
        }
        ADD_D(s1, s1, s2);
        IFX (X_PEND)
            ST_H(s1, xEmu, offsetof(x64emu_t, res));

        ANDI(s1, s1, 0xff);
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
        OR(s3, s3, s2);   // cc = (~res & (op1 | op2)) | (op1 & op2)
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
    IFX (X_PEND) {
        MOV32w(s4, c & 0xff);
        ST_B(s1, xEmu, offsetof(x64emu_t, op1));
        ST_B(s4, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, d_add8);
    } else IFX (X_ALL) {
        SET_DFNONE();
    }

    if (la64_lbt) {
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

// emit SUB8 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_sub8(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    IFX (X_PEND) {
        ST_B(s1, xEmu, offsetof(x64emu_t, op1));
        ST_B(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, d_sub8);
    } else IFX (X_ALL) {
        SET_DFNONE();
    }

    if (la64_lbt) {
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

// emit SUB32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_sub32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SDxw(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, rex.w ? d_sub64 : d_sub32);
    } else IFX (X_ALL) {
        SET_DFNONE();
    }

    if (la64_lbt) {
        IFX (X_ALL) {
            X64_SUB_WU(s1, s2);
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
    if (s1 == xRSP && (!dyn->insts || dyn->insts[ninst].x64.gen_flags == X_PEND)) {
        // special case when doing math on RSP and only PEND is needed: ignoring it!
        if (c > -2048 && c <= 2048) {
            ADDI_D(s1, s1, -c);
        } else {
            MOV64xw(s2, c);
            SUBxw(s1, s1, s2);
        }
        return;
    }

    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        MOV64xw(s2, c);
        SDxw(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, rex.w ? d_sub64 : d_sub32);
    } else IFX (X_ALL) {
        SET_DFNONE();
    }

    CLEAR_FLAGS(s3);
    if (la64_lbt) {
        IFX (X_PEND) {
        } else {
            MOV64xw(s2, c);
        }
        IFX (X_ALL) {
            X64_SUB_WU(s1, s2);
        }
        SUBxw(s1, s1, s2);
        if (!rex.w) ZEROUP(s1);

        IFX (X_PEND)
            SDxw(s1, xEmu, offsetof(x64emu_t, res));
        return;
    }

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
