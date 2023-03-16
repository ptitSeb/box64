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

// emit ADD32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_add32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    CLEAR_FLAGS();
    IFX(X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SDxw(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, rex.w?d_add64:d_add32b);
    } else IFX(X_ALL) {
        SET_DFNONE(s3);
    }
    IFX(X_CF) {
        if (rex.w) {
            AND(s5, xMASK, s1);
            AND(s4, xMASK, s2);
            ADD(s5, s5, s4); // lo
            SRLI(s3, s1, 0x20);
            SRLI(s4, s2, 0x20);
            ADD(s4, s4, s3);
            SRLI(s5, s5, 0x20);
            ADD(s5, s5, s4); // hi
            SRAI(s5, s5, 0x20);
            BEQZ(s5, 4);
            ORI(xFlags, xFlags, 1 << F_CF);
        } else {
            ADD(s5, s1, s2);
            SRLI(s5, s5, 0x20);
            BEQZ(s5, 4);
            ORI(xFlags, xFlags, 1 << F_CF);
        }
    }
    IFX(X_AF | X_OF) {
        OR(s3, s1, s2);      // s3 = op1 | op2
        AND(s4, s1, s2);      // s4 = op1 & op2
    }

    ADDxw(s1, s1, s2);

    IFX(X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_AF | X_OF) {
        NOT(s2, s1);   // s2 = ~res
        AND(s3, s2, s3);   // s3 = ~res & (op1 | op2)
        OR(s3, s3, s4);   // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX(X_AF) {
            ANDI(s4, s3, 0x08); // AF: cc & 0x08
            BEQZ(s4, 4);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX(X_OF) {
            SRLI(s3, s3, rex.w?62:30);
            SRLI(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            BEQZ(s3, 4);
            ORI(xFlags, xFlags, 1 << F_OF2);
        }
    }
    IFX(X_SF) {
        BGE(s1, xZR, 4);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    IFX(X_ZF) {
        BNEZ(s1, 4);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
}

// emit ADD32 instruction, from s1, constant c, store result in s1 using s3 and s4 as scratch
void emit_add32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s2, int s3, int s4, int s5)
{
    CLEAR_FLAGS();
    if(s1==xRSP && (!dyn->insts || dyn->insts[ninst].x64.gen_flags==X_PEND))
    {
        // special case when doing math on ESP and only PEND is needed: ignoring it!
        if(c >= -2048 && c < 2048) {
            ADDIxw(s1, s1, c);
        } else {
            MOV64xw(s2, c);
            ADDxw(s1, s1, s2);
        }
        return;
    }
    IFX(X_PEND | X_AF | X_CF | X_OF) {
        MOV64xw(s2, c);
    }
    IFX(X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SDxw(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, rex.w?d_add64:d_add32b);
    } else IFX(X_ALL) {
        SET_DFNONE(s3);
    }
    IFX(X_CF) {
        if (rex.w) {
            AND(s5, xMASK, s1);
            AND(s4, xMASK, s2);
            ADD(s5, s5, s4); // lo
            SRLI(s3, s1, 0x20);
            SRLI(s4, s2, 0x20);
            ADD(s4, s4, s3);
            SRLI(s5, s5, 0x20);
            ADD(s5, s5, s4); // hi
            SRAI(s5, s5, 0x20);
            BEQZ(s5, 4);
            ORI(xFlags, xFlags, 1 << F_CF);
        } else {
            ADD(s5, s1, s2);
            SRLI(s5, s5, 0x20);
            BEQZ(s5, 4);
            ORI(xFlags, xFlags, 1 << F_CF);
        }
    }
    IFX(X_AF | X_OF) {
        OR(s3, s1, s2);      // s3 = op1 | op2
        AND(s4, s1, s2);      // s4 = op1 & op2
    }

    if(c >= -2048 && c < 2048) {
        ADDIxw(s1, s1, c);
    } else {
        IFX(X_PEND | X_AF | X_CF | X_OF) {} else {MOV64xw(s2, c);}
        ADDxw(s1, s1, s2);
    }

    IFX(X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_AF | X_OF) {
        NOT(s2, s1);   // s2 = ~res
        AND(s3, s2, s3);   // s3 = ~res & (op1 | op2)
        OR(s3, s3, s4);   // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX(X_AF) {
            ANDI(s4, s3, 0x08); // AF: cc & 0x08
            BEQZ(s4, 4);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX(X_OF) {
            SRLI(s3, s3, rex.w?62:30);
            SRLI(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            BEQZ(s3, 4);
            ORI(xFlags, xFlags, 1 << F_OF2);
        }
    }
    IFX(X_SF) {
        BGE(s1, xZR, 4);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    IFX(X_ZF) {
        BNEZ(s1, 4);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
}


// emit SUB32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_sub32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    CLEAR_FLAGS();
    IFX(X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SDxw(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, rex.w?d_sub64:d_sub32);
    } else IFX(X_ALL) {
        SET_DFNONE(s3);
    }

    IFX(X_AF | X_CF | X_OF) {
        // for later flag calculation
        NOT(s5, s1);
    }

    SUBxw(s1, s1, s2);
    IFX(X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_SF) {
        BGE(s1, xZR, 4);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    CALC_SUB_FLAGS(s5, s2, s1, s3, s4, rex.w?64:32);
    IFX(X_ZF) {
        BEQZ(s1, 4);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}


// emit SUB32 instruction, from s1, constant c, store result in s1 using s2, s3, s4 and s5 as scratch
void emit_sub32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s2, int s3, int s4, int s5)
{
    CLEAR_FLAGS();
    if(s1==xRSP && (!dyn->insts || dyn->insts[ninst].x64.gen_flags==X_PEND))
    {
        // special case when doing math on RSP and only PEND is needed: ignoring it!
        if (c > -2048 && c <= 2048) {
            ADDI(s1, s1, -c);
        } else {
            MOV64xw(s2, c);
            SUBxw(s1, s1, s2);
        }
        return;
    }

    IFX(X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        MOV64xw(s2, c);
        SDxw(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, rex.w?d_sub64:d_sub32);
    } else IFX(X_ALL) {
        SET_DFNONE(s3);
    }

    IFX(X_AF | X_CF | X_OF) {
        // for later flag calculation
        NOT(s5, s1);
    }

    if (c > -2048 && c <= 2048) {
        ADDIxw(s1, s1, -c);
    } else {
        IFX(X_PEND) {} else {MOV64xw(s2, c);}
        SUBxw(s1, s1, s2);
    }

    IFX(X_AF | X_CF | X_OF) {
        IFX(X_PEND) {}
        else if (c > -2048 && c <= 2048) {
            MOV64xw(s2, c);
        }
    }
    IFX(X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_SF) {
        BGE(s1, xZR, 4);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    CALC_SUB_FLAGS(s5, s2, s1, s3, s4, rex.w?64:32);
    IFX(X_ZF) {
        BEQZ(s1, 4);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}
