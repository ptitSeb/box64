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

// emit ADD32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_add32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    CLEAR_FLAGS();
    IFX(X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SDxw(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, rex.w?d_add64:d_add32b);
    } else IFX(X_ALL) {
        SET_DFNONE();
    }
    IFX(X_CF) {
        if (rex.w) {
            AND(s5, xMASK, s1);
            if(rv64_zba) ADDUW(s5, s2, s5); else {AND(s4, xMASK, s2); ADD(s5, s5, s4);} // lo
            SRLI(s3, s1, 0x20);
            SRLI(s4, s2, 0x20);
            ADD(s4, s4, s3);
            SRLI(s5, s5, 0x20);
            ADD(s5, s5, s4); // hi
            SRAI(s5, s5, 0x20);
            BEQZ(s5, 8);
            ORI(xFlags, xFlags, 1 << F_CF);
        } else {
            AND(s3, s1, xMASK);
            AND(s4, s2, xMASK);
            ADD(s5, s3, s4);
            SRLI(s5, s5, 0x20);
            BEQZ(s5, 8);
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
        if(rv64_zbb) {
            ANDN(s3, s3, s1);   // s3 = ~res & (op1 | op2)
        } else {
            NOT(s5, s1);   // s5 = ~res
            AND(s3, s5, s3);   // s3 = ~res & (op1 | op2)
        }
        OR(s3, s3, s4);   // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX(X_AF) {
            ANDI(s4, s3, 0x08); // AF: cc & 0x08
            BEQZ(s4, 8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX(X_OF) {
            SRLI(s3, s3, rex.w?62:30);
            SRLI(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            BEQZ(s3, 8);
            ORI(xFlags, xFlags, 1 << F_OF2);
        }
    }
    IFX(X_SF) {
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    IFX(X_ZF) {
        BNEZ(s1, 8);
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
        SET_DFNONE();
    }
    IFX(X_CF) {
        if (rex.w) {
            AND(s5, xMASK, s1);
            if(rv64_zba) ADDUW(s5, s2, s5); else {AND(s4, xMASK, s2); ADD(s5, s5, s4);} // lo
            SRLI(s3, s1, 0x20);
            SRLI(s4, s2, 0x20);
            ADD(s4, s4, s3);
            SRLI(s5, s5, 0x20);
            ADD(s5, s5, s4); // hi
            SRAI(s5, s5, 0x20);
            BEQZ(s5, 8);
            ORI(xFlags, xFlags, 1 << F_CF);
        } else {
            AND(s3, s1, xMASK);
            AND(s4, s2, xMASK);
            ADD(s5, s3, s4);
            SRLI(s5, s5, 0x20);
            BEQZ(s5, 8);
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
        if(rv64_zbb) {
            ANDN(s3, s3, s1);   // s3 = ~res & (op1 | op2)
        } else {
            NOT(s2, s1);   // s2 = ~res
            AND(s3, s2, s3);   // s3 = ~res & (op1 | op2)
        }
        OR(s3, s3, s4);   // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX(X_AF) {
            ANDI(s4, s3, 0x08); // AF: cc & 0x08
            BEQZ(s4, 8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX(X_OF) {
            SRLI(s3, s3, rex.w?62:30);
            SRLI(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            BEQZ(s3, 8);
            ORI(xFlags, xFlags, 1 << F_OF2);
        }
    }
    IFX(X_SF) {
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    IFX(X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
}

// emit ADD16 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_add16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    CLEAR_FLAGS();
    IFX(X_PEND) {
        SH(s1, xEmu, offsetof(x64emu_t, op1));
        SH(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, d_add16);
    } else IFX(X_ALL) {
        SET_DFNONE();
    }
    IFX(X_AF | X_OF) {
        OR(s3, s1, s2);    // s3 = op1 | op2
        AND(s4, s1, s2);    // s4 = op1 & op2
    }
    ADD(s1, s1, s2);

    IFX(X_PEND) {
        SW(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_AF | X_OF) {
        if(rv64_zbb) {
            ANDN(s3, s3, s1);    // s3 = ~res & (op1 | op2)
        } else {
            NOT(s5, s1);   // s5 = ~res
            AND(s3, s5, s3);   // s3 = ~res & (op1 | op2)
        }
        OR(s3, s3, s4);   // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX(X_AF) {
            ANDI(s4, s3, 0x08); // AF: cc & 0x08
            BEQZ(s4, 8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX(X_OF) {
            SRLI(s3, s3, 14);
            SRLI(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            BEQZ(s3, 8);
            ORI(xFlags, xFlags, 1 << F_OF2);
        }
    }

    IFX(X_CF) {
        SRLI(s3, s1, 16);
        BEQZ(s3, 8);
        ORI(xFlags, xFlags, 1 << F_CF);
    }

    ZEXTH(s1, s1);

    IFX(X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_SF) {
        SRLI(s3, s1, 15);
        BEQZ(s3, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit ADD8 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_add8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    CLEAR_FLAGS();
    IFX(X_PEND) {
        SB(s1, xEmu, offsetof(x64emu_t, op1));
        SB(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, d_add8);
    } else IFX(X_ALL) {
        SET_DFNONE();
    }
    IFX(X_AF | X_OF) {
        OR(s3, s1, s2);       // s3 = op1 | op2
        AND(s4, s1, s2);       // s4 = op1 & op2
    }
    ADD(s1, s1, s2);

    IFX(X_AF|X_OF) {
        if(rv64_zbb) {
            ANDN(s3, s3, s1);   // s3 = ~res & (op1 | op2)
        } else {
            NOT(s2, s1);   // s4 = ~res
            AND(s3, s2, s3);   // s3 = ~res & (op1 | op2)
        }
        OR(s3, s3, s4);   // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX(X_AF) {
            ANDI(s4, s3, 0x08); // AF: cc & 0x08
            BEQZ(s4, 8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX(X_OF) {
            SRLI(s3, s3, 6);
            SRLI(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            BEQZ(s3, 8);
            ORI(xFlags, xFlags, 1 << F_OF2);
        }
    }
    IFX(X_CF) {
        SRLI(s3, s1, 8);
        BEQZ(s3, 8);
        ORI(xFlags, xFlags, 1 << F_CF);
    }
    IFX(X_PEND) {
        SH(s1, xEmu, offsetof(x64emu_t, res));
    }
    ANDI(s1, s1, 0xff);
    IFX(X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_SF) {
        SRLI(s3, s1, 7);
        BEQZ(s3, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit ADD8 instruction, from s1, const c, store result in s1 using s3 and s4 as scratch
void emit_add8c(dynarec_rv64_t* dyn, int ninst, int s1, int c, int s2, int s3, int s4)
{
    CLEAR_FLAGS();
    IFX(X_PEND) {
        MOV32w(s4, c&0xff);
        SB(s1, xEmu, offsetof(x64emu_t, op1));
        SB(s4, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, d_add8);
    } else IFX(X_ALL) {
        SET_DFNONE();
    }
    IFX(X_AF | X_OF) {
        IFX(X_PEND) {} else {MOV32w(s4, c&0xff);}
        OR(s3, s1, s4);       // s3 = op1 | op2
        AND(s4, s1, s4);       // s4 = op1 & op2
    }
    ADDI(s1, s1, c);

    IFX(X_AF|X_OF) {
        if(rv64_zbb) {
            ANDN(s3, s3, s1);   // s3 = ~res & (op1 | op2)
        } else {
            NOT(s2, s1);   // s2 = ~res
            AND(s3, s2, s3);   // s3 = ~res & (op1 | op2)
        }
        OR(s3, s3, s4);   // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX(X_AF) {
            ANDI(s4, s3, 0x08); // AF: cc & 0x08
            BEQZ(s4, 8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX(X_OF) {
            SRLI(s3, s3, 6);
            SRLI(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            BEQZ(s3, 8);
            ORI(xFlags, xFlags, 1 << F_OF2);
        }
    }
    IFX(X_CF) {
        SRLI(s3, s1, 8);
        BEQZ(s3, 8);
        ORI(xFlags, xFlags, 1 << F_CF);
    }
    IFX(X_PEND) {
        SH(s1, xEmu, offsetof(x64emu_t, res));
    }
    ANDI(s1, s1, 0xff);
    IFX(X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_SF) {
        SRLI(s3, s1, 7);
        BEQZ(s3, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit SUB8 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_sub8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    CLEAR_FLAGS();
    IFX(X_PEND) {
        SB(s1, xEmu, offsetof(x64emu_t, op1));
        SB(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, d_sub8);
    } else IFX(X_ALL) {
        SET_DFNONE();
    }

    IFX(X_AF | X_CF | X_OF) {
        // for later flag calculation
        NOT(s5, s1);
    }

    SUB(s1, s1, s2);
    ANDI(s1, s1, 0xff);
    IFX(X_SF) {
        SRLI(s3, s1, 7);
        BEQZ(s3, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX(X_PEND) {
        SB(s1, xEmu, offsetof(x64emu_t, res));
    }
    CALC_SUB_FLAGS(s5, s2, s1, s3, s4, 8);
    IFX(X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit SUB8 instruction, from s1, constant c, store result in s1 using s3 and s4 as scratch
void emit_sub8c(dynarec_rv64_t* dyn, int ninst, int s1, int c, int s2, int s3, int s4, int s5)
{
    MOV32w(s2, c&0xff);
    emit_sub8(dyn, ninst, s1, s2, s3, s4, s5);
}

// emit SUB16 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_sub16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    CLEAR_FLAGS();
    IFX(X_PEND) {
        SH(s1, xEmu, offsetof(x64emu_t, op1));
        SH(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, d_sub16);
    } else IFX(X_ALL) {
        SET_DFNONE();
    }

    IFX(X_AF | X_CF | X_OF) {
        // for later flag calculation
        NOT(s5, s1);
    }

    SUBW(s1, s1, s2);
    IFX(X_PEND) {
        SH(s1, xEmu, offsetof(x64emu_t, res));
    }
    SLLI(s1, s1, 48);
    IFX(X_SF) {
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    SRLI(s1, s1, 48);

    CALC_SUB_FLAGS(s5, s2, s1, s3, s4, 16);
    IFX(X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
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
        SET_DFNONE();
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
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    CALC_SUB_FLAGS(s5, s2, s1, s3, s4, rex.w?64:32);
    IFX(X_ZF) {
        BNEZ(s1, 8);
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
            SUBI(s1, s1, c);
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
        SET_DFNONE();
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
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    CALC_SUB_FLAGS(s5, s2, s1, s3, s4, rex.w?64:32);
    IFX(X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit INC8 instruction, from s1, store result in s1 using s2, s3 and s4 as scratch
void emit_inc8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX(X_ALL) {
        ANDI(xFlags, xFlags, ~((1UL<<F_AF) | (1UL<<F_OF2) | (1UL<<F_ZF) | (1UL<<F_SF) | (1UL<<F_PF)));
    }
    IFX(X_PEND) {
        SB(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s3, d_inc8);
    } else IFX(X_ALL) {
        SET_DFNONE();
    }
    IFX(X_AF | X_OF) {
        ORI(s3, s1, 1);      // s3 = op1 | op2
        ANDI(s4, s1, 1);      // s5 = op1 & op2
    }

    ADDIW(s1, s1, 1);

    IFX(X_PEND) {
        SB(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_AF | X_OF) {
        if(rv64_zbb) {
            ANDN(s3, s3, s1);   // s3 = ~res & (op1 | op2)
        } else {
            NOT(s2, s1);   // s2 = ~res
            AND(s3, s2, s3);   // s3 = ~res & (op1 | op2)
        }
        OR(s3, s3, s4);   // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX(X_AF) {
            ANDI(s2, s3, 0x08); // AF: cc & 0x08
            BEQZ(s2, 8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX(X_OF) {
            SRLI(s3, s3, 6);
            SRLI(s2, s3, 1);
            XOR(s3, s3, s2);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            BEQZ(s3, 8);
            ORI(xFlags, xFlags, 1 << F_OF2);
        }
    }
    IFX(X_SF) {
        ANDI(s2, s1, 0x80);
        BEQZ(s2, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    ANDI(s1, s1, 0xff);
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s2);
    }
    IFX(X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
}


// emit DEC8 instruction, from s1, store result in s1 using s2, s3 and s4 as scratch
void emit_dec8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX(X_ALL) {
        ANDI(xFlags, xFlags, ~((1UL<<F_AF) | (1UL<<F_OF2) | (1UL<<F_ZF) | (1UL<<F_SF) | (1UL<<F_PF)));
    }
    IFX(X_PEND) {
        SB(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s3, d_dec8);
    } else IFX(X_ALL) {
        SET_DFNONE();
    }
    IFX(X_AF | X_OF) {
        NOT(s4, s1);        // s4 = ~op1
        ORI(s3, s4, 1);      // s3 = ~op1 | op2
        ANDI(s4, s4, 1);      // s4 = ~op1 & op2
    }

    ADDIW(s1, s1, -1);

    IFX(X_PEND) {
        SB(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_AF | X_OF) {
        AND(s3, s1, s3);   // s3 = res & (~op1 | op2)
        OR(s3, s3, s4);   // cc = (res & (~op1 | op2)) | (~op1 & op2)
        IFX(X_AF) {
            ANDI(s2, s3, 0x08); // AF: cc & 0x08
            BEQZ(s2, 8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX(X_OF) {
            SRLI(s3, s3, 6);
            SRLI(s2, s3, 1);
            XOR(s3, s3, s2);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            BEQZ(s3, 8);
            ORI(xFlags, xFlags, 1 << F_OF2);
        }
    }
    IFX(X_SF) {
        ANDI(s2, s1, 0x80);
        BEQZ(s2, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    ANDI(s1, s1, 0xff);
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s2);
    }
    IFX(X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
}

// emit INC32 instruction, from s1, store result in s1 using s3 and s4 as scratch
void emit_inc32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    IFX(X_ALL) {
        ANDI(xFlags, xFlags, ~((1UL<<F_AF) | (1UL<<F_OF2) | (1UL<<F_ZF) | (1UL<<F_SF) | (1UL<<F_PF)));
    }
    IFX(X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s3, rex.w?d_inc64:d_inc32);
    } else IFX(X_ALL) {
        SET_DFNONE();
    }
    IFX(X_AF | X_OF) {
        ORI(s3, s1, 1);      // s3 = op1 | op2
        ANDI(s5, s1, 1);      // s5 = op1 & op2
    }

    ADDIxw(s1, s1, 1);

    IFX(X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_AF | X_OF) {
        if(rv64_zbb) {
            ANDN(s3, s3, s1);    // s3 = ~res & (op1 | op2)
        } else {
            NOT(s2, s1);   // s2 = ~res
            AND(s3, s2, s3);   // s3 = ~res & (op1 | op2)
        }
        OR(s3, s3, s5);   // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX(X_AF) {
            ANDI(s2, s3, 0x08); // AF: cc & 0x08
            BEQZ(s2, 8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX(X_OF) {
            SRLI(s3, s3, rex.w?62:30);
            SRLI(s2, s3, 1);
            XOR(s3, s3, s2);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            BEQZ(s3, 8);
            ORI(xFlags, xFlags, 1 << F_OF2);
        }
    }
    IFX(X_SF) {
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s2);
    }
    IFX(X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
}

// emit DEC32 instruction, from s1, store result in s1 using s3 and s4 as scratch
void emit_dec32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    IFX(X_ALL) {
        ANDI(xFlags, xFlags, ~((1UL<<F_AF) | (1UL<<F_OF2) | (1UL<<F_ZF) | (1UL<<F_SF) | (1UL<<F_PF)));
    }
    IFX(X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s3, rex.w?d_dec64:d_dec32);
    } else IFX(X_ALL) {
        SET_DFNONE();
    }
    IFX(X_AF | X_OF) {
        NOT(s5, s1);
        ORI(s3, s5, 1);      // s3 = ~op1 | op2
        ANDI(s5, s5, 1);      // s5 = ~op1 & op2
    }

    ADDIxw(s1, s1, -1);

    IFX(X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_AF | X_OF) {
        AND(s3, s1, s3);   // s3 = res & (~op1 | op2)
        OR(s3, s3, s5);   // cc = (res & (~op1 | op2)) | (~op1 & op2)
        IFX(X_AF) {
            ANDI(s2, s3, 0x08); // AF: cc & 0x08
            BEQZ(s2, 8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX(X_OF) {
            SRLI(s3, s3, rex.w?62:30);
            SRLI(s2, s3, 1);
            XOR(s3, s3, s2);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            BEQZ(s3, 8);
            ORI(xFlags, xFlags, 1 << F_OF2);
        }
    }
    IFX(X_SF) {
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s2);
    }
    IFX(X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
}

// emit INC16 instruction, from s1, store result in s1 using s3 and s4 as scratch
void emit_inc16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX(X_ALL) {
        ANDI(xFlags, xFlags, ~((1UL<<F_AF) | (1UL<<F_OF2) | (1UL<<F_ZF) | (1UL<<F_SF) | (1UL<<F_PF)));
    }
    IFX(X_PEND) {
        SH(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s3, d_inc16);
    } else IFX(X_ZF|X_OF|X_AF|X_SF|X_PF) {
        SET_DFNONE();
    }
    IFX(X_AF | X_OF) {
        ORI(s3, s1, 1);    // s3 = op1 | op2
        ANDI(s4, s1, 1);    // s4 = op1 & op2
    }

    ADDI(s1, s1, 1);

    IFX(X_PEND) {
        SH(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_AF | X_OF) {
        if(rv64_zbb) {
            ANDN(s3, s3, s1);   // s3 = ~res & (op1 | op2)
        } else {
            NOT(s2, s1);   // s2 = ~res
            AND(s3, s2, s3);   // s3 = ~res & (op1 | op2)
        }
        OR(s3, s3, s4);   // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX(X_AF) {
            ANDI(s4, s3, 0x08); // AF: cc & 0x08
            BEQZ(s4, 8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX(X_OF) {
            SRLI(s3, s3, 14);
            SRLI(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            BEQZ(s3, 8);
            ORI(xFlags, xFlags, 1 << F_OF2);
        }
    }

    ZEXTH(s1, s1);

    IFX(X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_SF) {
        SRLI(s3, s1, 15);
        BEQZ(s3, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit DEC16 instruction, from s1, store result in s1 using s3 and s4 as scratch
void emit_dec16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    IFX(X_ALL) {
        ANDI(xFlags, xFlags, ~((1UL<<F_AF) | (1UL<<F_OF2) | (1UL<<F_ZF) | (1UL<<F_SF) | (1UL<<F_PF)));
    }
    IFX(X_PEND) {
        SH(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s3, d_dec16);
    } else IFX(X_ALL) {
        SET_DFNONE();
    }
    IFX(X_AF | X_OF) {
        NOT(s5, s1);
        ORI(s3, s5, 1);      // s3 = ~op1 | op2
        ANDI(s5, s5, 1);      // s5 = ~op1 & op2
    }

    ADDIW(s1, s1, -1);

    IFX(X_PEND) {
        SH(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_AF | X_OF) {
        AND(s3, s1, s3);   // s3 = res & (~op1 | op2)
        OR(s3, s3, s5);   // cc = (res & (~op1 | op2)) | (~op1 & op2)
        IFX(X_AF) {
            ANDI(s2, s3, 0x08); // AF: cc & 0x08
            BEQZ(s2, 8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX(X_OF) {
            SRLI(s3, s3, 14);
            SRLI(s2, s3, 1);
            XOR(s3, s3, s2);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            BEQZ(s3, 8);
            ORI(xFlags, xFlags, 1 << F_OF2);
        }
    }
    SLLIW(s1, s1, 16);
    IFX(X_SF) {
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    SRLIW(s1, s1, 16);
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s2);
    }
    IFX(X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
}

// emit SBB8 instruction, from s1, s2, store result in s1 using s3, s4 and s5 as scratch
void emit_sbb8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    IFX(X_PEND) {
        SB(s1, xEmu, offsetof(x64emu_t, op1));
        SB(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, d_sbb8);
    } else IFX(X_ALL) {
        SET_DFNONE();
    }

    IFX(X_AF | X_CF | X_OF) {
        // for later flag calculation
        NOT(s5, s1);
    }

    SUBW(s1, s1, s2);
    ANDI(s3, xFlags, 1 << F_CF);
    SUBW(s1, s1, s3);
    ANDI(s1, s1, 0xff);

    CLEAR_FLAGS();
    IFX(X_PEND) {
        SB(s1, xEmu, offsetof(x64emu_t, res));
    }

    CALC_SUB_FLAGS(s5, s2, s1, s3, s4, 8);
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

// emit ADC8 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_adc8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5) {
    IFX(X_PEND) {
        SH(s1, xEmu, offsetof(x64emu_t, op1));
        SH(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, d_adc8);
    } else IFX(X_ALL) {
        SET_DFNONE();
    }
    IFX(X_AF | X_OF) {
        OR(s4, s1, s2);  // s4 = op1 | op2
        AND(s5, s1, s2); // s5 = op1 & op2
    }

    ADD(s1, s1, s2);
    ANDI(s3, xFlags, 1 << F_CF);
    ADD(s1, s1, s3);

    CLEAR_FLAGS();
    IFX(X_PEND) {
        SW(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_AF | X_OF) {
        if(rv64_zbb) {
            ANDN(s3, s4, s1);   // s3 = ~res & (op1 | op2)
        } else {
            NOT(s2, s1);     // s2 = ~res
            AND(s3, s2, s4); // s3 = ~res & (op1 | op2)
        }
        OR(s3, s3, s5);  // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX(X_AF) {
            ANDI(s4, s3, 0x08); // AF: cc & 0x08
            BEQZ(s4, 8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX(X_OF) {
            SRLI(s3, s3, 6);
            SRLI(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            BEQZ(s3, 8);
            ORI(xFlags, xFlags, 1 << F_OF2);
        }
    }
    IFX(X_CF) {
        SRLI(s3, s1, 8);
        BEQZ(s3, 8);
        ORI(xFlags, xFlags, 1 << F_CF);
    }

    ANDI(s1, s1, 0xff);

    IFX(X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_SF) {
        SRLI(s3, s1, 7);
        BEQZ(s3, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit ADC8 instruction, from s1, const c, store result in s1 using s3, s4, s5 and s6 as scratch
void emit_adc8c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4, int s5, int s6) {
    MOV32w(s5, c&0xff);
    emit_adc8(dyn, ninst, s1, s5, s3, s4, s6);
}

// emit SBB8 instruction, from s1, constant c, store result in s1 using s3, s4, s5 and s6 as scratch
void emit_sbb8c(dynarec_rv64_t* dyn, int ninst, int s1, int c, int s3, int s4, int s5, int s6)
{
    MOV32w(s6, c&0xff);
    emit_sbb8(dyn, ninst, s1, s6, s3, s4, s5);
}

// emit SBB16 instruction, from s1, s2, store result in s1 using s3, s4 and s5 as scratch
void emit_sbb16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    IFX(X_PEND) {
        SH(s1, xEmu, offsetof(x64emu_t, op1));
        SH(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, d_sbb16);
    } else IFX(X_ALL) {
        SET_DFNONE();
    }

    IFX(X_AF | X_CF | X_OF) {
        // for later flag calculation
        NOT(s5, s1);
    }

    SUBW(s1, s1, s2);
    ANDI(s3, xFlags, 1 << F_CF);
    SUBW(s1, s1, s3);

    CLEAR_FLAGS();
    SLLIW(s1, s1, 16);
    IFX(X_SF) {
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    SRLIW(s1, s1, 16);

    IFX(X_PEND) {
        SH(s1, xEmu, offsetof(x64emu_t, res));
    }

    CALC_SUB_FLAGS(s5, s2, s1, s3, s4, 16);
    IFX(X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit SBB32 instruction, from s1, s2, store result in s1 using s3, s4 and s5 as scratch
void emit_sbb32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    IFX(X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SDxw(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, rex.w?d_sbb64:d_sbb32);
    } else IFX(X_ALL) {
        SET_DFNONE();
    }

    IFX(X_AF | X_CF | X_OF) {
        // for later flag calculation
        NOT(s5, s1);
    }

    SUBxw(s1, s1, s2);
    ANDI(s3, xFlags, 1 << F_CF);
    SUBxw(s1, s1, s3);

    CLEAR_FLAGS();
    IFX(X_SF) {
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }

    IFX(X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }

    CALC_SUB_FLAGS(s5, s2, s1, s3, s4, rex.w?64:32);
    IFX(X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit NEG32 instruction, from s1, store result in s1 using s2 and s3 as scratch
void emit_neg32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3)
{
    CLEAR_FLAGS();
    IFX(X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s3, rex.w?d_neg64:d_neg32);
    } else IFX(X_ALL) {
        SET_DFNONE();
    }
    IFX(X_AF | X_OF) {
        MV(s3, s1);      // s3 = op1
    }

    NEGxw(s1, s1);
    IFX(X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }

    IFX(X_CF) {
        BEQZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_CF);
    }

    IFX(X_AF | X_OF) {
        OR(s3, s1, s3); // s3 = res | op1
        IFX(X_AF) {
            /* af = bc & 0x8 */
            ANDI(s2, s3, 8);
            BEQZ(s2, 8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX(X_OF) {
            /* of = ((bc >> (width-2)) ^ (bc >> (width-1))) & 0x1; */
            SRLI(s2, s3, (rex.w?64:32)-2);
            SRLI(s3, s2, 1);
            XOR(s2, s2, s3);
            ANDI(s2, s2, 1);
            BEQZ(s2, 8);
            ORI(xFlags, xFlags, 1 << F_OF2);
        }
    }
    IFX(X_SF) {
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s2);
    }
    IFX(X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
}

// emit NEG16 instruction, from s1, store result in s1 using s2 and s3 as scratch
void emit_neg16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3)
{
    CLEAR_FLAGS();
    IFX(X_PEND) {
        SH(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s3, d_neg16);
    } else IFX(X_ALL) {
        SET_DFNONE();
    }
    IFX(X_AF | X_OF) {
        MV(s3, s1);      // s3 = op1
    }

    NEG(s1, s1);
    ZEXTH(s1, s1);
    IFX(X_PEND) {
        SH(s1, xEmu, offsetof(x64emu_t, res));
    }

    IFX(X_CF) {
        BEQZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_CF);
    }

    IFX(X_AF | X_OF) {
        OR(s3, s1, s3); // s3 = res | op1
        IFX(X_AF) {
            /* af = bc & 0x8 */
            ANDI(s2, s3, 8);
            BEQZ(s2, 8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX(X_OF) {
            /* of = ((bc >> (width-2)) ^ (bc >> (width-1))) & 0x1; */
            SRLI(s2, s3, 14);
            SRLI(s3, s2, 1);
            XOR(s2, s2, s3);
            ANDI(s2, s2, 1);
            BEQZ(s2, 8);
            ORI(xFlags, xFlags, 1 << F_OF2);
        }
    }
    IFX(X_SF) {
        SRLI(s3, s1, 15-F_SF);    // put sign bit in place
        ANDI(s3, s3, 1 << F_SF);    // 1<<F_SF is sign bit, so just mask
        OR(xFlags, xFlags, s3);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s2);
    }
    IFX(X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
}

// emit NEG8 instruction, from s1, store result in s1 using s2 and s3 as scratch
void emit_neg8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3)
{
    CLEAR_FLAGS();
    IFX(X_PEND) {
        SB(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s3, d_neg8);
    } else IFX(X_ALL) {
        SET_DFNONE();
    }
    IFX(X_AF | X_OF) {
        MV(s3, s1);      // s3 = op1
    }

    NEG(s1, s1);
    ANDI(s1, s1, 0xff);
    IFX(X_PEND) {
        SB(s1, xEmu, offsetof(x64emu_t, res));
    }

    IFX(X_CF) {
        BEQZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_CF);
    }

    IFX(X_AF | X_OF) {
        OR(s3, s1, s3); // s3 = res | op1
        IFX(X_AF) {
            /* af = bc & 0x8 */
            ANDI(s2, s3, 8);
            BEQZ(s2, 8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX(X_OF) {
            /* of = ((bc >> (width-2)) ^ (bc >> (width-1))) & 0x1; */
            SRLI(s2, s3, 6);
            SRLI(s3, s2, 1);
            XOR(s2, s2, s3);
            ANDI(s2, s2, 1);
            BEQZ(s2, 8);
            ORI(xFlags, xFlags, 1 << F_OF2);
        }
    }
    IFX(X_SF) {
        ANDI(s3, s1, 1 << F_SF);    // 1<<F_SF is sign bit, so just mask
        OR(xFlags, xFlags, s3);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s2);
    }
    IFX(X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
}

// emit ADC16 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_adc16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    IFX(X_PEND) {
        SH(s1, xEmu, offsetof(x64emu_t, op1));
        SH(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, d_adc16);
    } else IFX(X_ALL) {
        SET_DFNONE();
    }
    IFX(X_AF | X_OF) {
        OR(s4, s1, s2);  // s4 = op1 | op2
        AND(s5, s1, s2); // s5 = op1 & op2
    }

    ADD(s1, s1, s2);
    ANDI(s3, xFlags, 1 << F_CF);
    ADD(s1, s1, s3);

    CLEAR_FLAGS();
    IFX(X_PEND) {
        SW(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_AF | X_OF) {
        if(rv64_zbb) {
            ANDN(s3, s4, s1);   // s3 = ~res & (op1 | op2)
        } else {
            NOT(s2, s1);     // s2 = ~res
            AND(s3, s2, s4); // s3 = ~res & (op1 | op2)
        }
        OR(s3, s3, s5);  // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX(X_AF) {
            ANDI(s4, s3, 0x08); // AF: cc & 0x08
            BEQZ(s4, 8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX(X_OF) {
            SRLI(s3, s3, 14);
            SRLI(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            BEQZ(s3, 8);
            ORI(xFlags, xFlags, 1 << F_OF2);
        }
    }
    IFX(X_CF) {
        SRLI(s3, s1, 16);
        BEQZ(s3, 8);
        ORI(xFlags, xFlags, 1 << F_CF);
    }

    ZEXTH(s1, s1);

    IFX(X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_SF) {
        SRLI(s3, s1, 15);
        BEQZ(s3, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit ADC32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_adc32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5, int s6)
{
    IFX(X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SDxw(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, rex.w?d_adc64:d_adc32b);
    } else IFX(X_ALL) {
        SET_DFNONE();
    }
    IFX(X_CF) {
        if (rex.w) {
            AND(s5, xMASK, s1);
            if(rv64_zba) ADDUW(s5, s2, s5); else {AND(s4, xMASK, s2); ADD(s5, s5, s4);} // lo
            ANDI(s3, xFlags, 1);
            ADD(s5, s5, s3);    // add carry
            SRLI(s3, s1, 0x20);
            SRLI(s4, s2, 0x20);
            ADD(s4, s4, s3);
            SRLI(s5, s5, 0x20);
            ADD(s5, s5, s4); // hi
            SRAI(s6, s5, 0x20);
        } else {
            AND(s3, s1, xMASK);
            AND(s4, s2, xMASK);
            ADD(s5, s3, s4);
            ANDI(s3, xFlags, 1);
            ADD(s5, s5, s3);    // add carry
            SRLI(s6, s5, 0x20);
        }
    }
    IFX(X_AF | X_OF) {
        OR(s4, s1, s2);  // s4 = op1 | op2
        AND(s5, s1, s2); // s5 = op1 & op2
    }

    ADDxw(s1, s1, s2);
    ANDI(s3, xFlags, 1 << F_CF);
    ADDxw(s1, s1, s3);

    CLEAR_FLAGS();
    IFX(X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_CF) {
        BEQZ(s6, 8);
        ORI(xFlags, xFlags, 1 << F_CF);
    }
    IFX(X_AF | X_OF) {
        if(rv64_zbb) {
            ANDN(s3, s4, s1);   // s3 = ~res & (op1 | op2)
        } else {
            NOT(s3, s1);     // s2 = ~res
            AND(s3, s3, s4); // s3 = ~res & (op1 | op2)
        }
        OR(s3, s3, s5);  // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX(X_AF) {
            ANDI(s4, s3, 0x08); // AF: cc & 0x08
            BEQZ(s4, 8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX(X_OF) {
            SRLI(s3, s3, rex.w?62:30);
            SRLI(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDI(s3, s3, 1); // OF: xor of two MSB's of cc
            BEQZ(s3, 8);
            ORI(xFlags, xFlags, 1 << F_OF2);
        }
    }
    IFX(X_SF) {
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    IFX(X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
}
