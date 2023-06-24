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

// emit XOR8 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch, s4 can be same as s2 (and so s2 destroyed)
void emit_xor8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    CLEAR_FLAGS();
    IFX(X_PEND) {
        SET_DF(s4, d_xor8);
    } else IFX(X_ALL) {
        SET_DFNONE();
    }
    XOR(s1, s1, s2);
    ANDI(s1, s1, 0xff);
    IFX(X_SF) {
        SRLI(s3, s1, 7);
        BEQZ(s3, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX(X_PEND) {
        SB(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit XOR8 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_xor8c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    CLEAR_FLAGS();
    IFX(X_PEND) {
        SET_DF(s4, d_xor8);
    } else IFX(X_ALL) {
        SET_DFNONE();
    }
    XORI(s1, s1, c&0xff);
    ANDI(s1, s1, 0xff);
    IFX(X_SF) {
        SRLI(s3, s1, 7);
        BEQZ(s3, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX(X_PEND) {
        SB(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit XOR32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_xor32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4)
{
    CLEAR_FLAGS();
    IFX(X_PEND) {
        SET_DF(s4, rex.w?d_xor64:d_xor32);
    } else IFX(X_ALL) {
        SET_DFNONE();
    }

    XOR(s1, s1, s2);

    // test sign bit before zeroup.
    IFX(X_SF) {
        if (!rex.w) SEXT_W(s1, s1);
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w && s1!=s2) {
        ZEROUP(s1);
    }

    IFX(X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }

    IFX(X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit XOR32 instruction, from s1, c, store result in s1 using s3 and s4 as scratch
void emit_xor32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4)
{
    CLEAR_FLAGS();
    IFX(X_PEND) {
        SET_DF(s4, rex.w?d_xor64:d_xor32);
    } else IFX(X_ALL) {
        SET_DFNONE();
    }

    if(c>=-2048 && c<=2047) {
        XORI(s1, s1, c);
    } else {
        MOV64xw(s3, c);
        XOR(s1, s1, s3);
    }

    // test sign bit before zeroup.
    IFX(X_SF) {
        if (!rex.w) SEXT_W(s1, s1);
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }

    IFX(X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }

    IFX(X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit XOR16 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch, s4 can be same as s2 (and so s2 destroyed)
void emit_xor16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    CLEAR_FLAGS();
    IFX(X_PEND) {
        SET_DF(s4, d_xor16);
    } else IFX(X_ALL) {
        SET_DFNONE();
    }

    XOR(s1, s1, s2);
    ZEXTH(s1, s1);

    IFX(X_PEND) {
        SH(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_ZF | X_SF) {
        IFX(X_ZF) {
            BNEZ(s1, 8);
            ORI(xFlags, xFlags, 1 << F_ZF);
        }
        IFX(X_SF) {
            SRLI(s3, s1, 15);
            BEQZ(s3, 8);
            ORI(xFlags, xFlags, 1 << F_SF);
        }
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit OR16 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch, s4 can be same as s2 (and so s2 destroyed)
void emit_or16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4) {
    CLEAR_FLAGS();
    IFX(X_PEND) {
        SET_DF(s3, d_or16);
    } else IFX(X_ALL) {
        SET_DFNONE();
    }

    OR(s1, s1, s2);
    ZEXTH(s1, s1);
    IFX(X_PEND) {
        SD(s1, xEmu, offsetof(x64emu_t, res));
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

// emit OR32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_or32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4)
{
    CLEAR_FLAGS();
    IFX(X_PEND) {
        SET_DF(s4, rex.w?d_or64:d_or32);
    } else IFX(X_ALL) {
        SET_DFNONE();
    }

    OR(s1, s1, s2);

    // test sign bit before zeroup.
    IFX(X_SF) {
        if (!rex.w) SEXT_W(s1, s1);
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }

    IFX(X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }

    IFX(X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}
// emit OR32 instruction, from s1, c, store result in s1 using s3 and s4 as scratch
void emit_or32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4)
{
    CLEAR_FLAGS();
    IFX(X_PEND) {
        SET_DF(s4, rex.w?d_or64:d_or32);
    } else IFX(X_ALL) {
        SET_DFNONE();
    }

    if(c>=-2048 && c<=2047) {
        ORI(s1, s1, c);
    } else {
        MOV64xw(s3, c);
        OR(s1, s1, s3);
    }

    // test sign bit before zeroup.
    IFX(X_SF) {
        if (!rex.w) SEXT_W(s1, s1);
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }

    IFX(X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }

    IFX(X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}


// emit AND8 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch, s4 can be same as s2 (and so s2 destroyed)
void emit_and8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{

    CLEAR_FLAGS();
    IFX(X_PEND) {
        SET_DF(s3, d_and8);
    } else IFX(X_ALL) {
        SET_DFNONE();
    }

    AND(s1, s1, s2);

    IFX(X_PEND) {
        SB(s1, xEmu, offsetof(x64emu_t, res));
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


// emit AND8 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_and8c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    CLEAR_FLAGS();
    IFX(X_PEND) {
        SET_DF(s3, d_and8);
    } else IFX(X_ALL) {
        SET_DFNONE();
    }

    ANDI(s1, s1, c&0xff);

    IFX(X_PEND) {
        SD(s1, xEmu, offsetof(x64emu_t, res));
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

void emit_and16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4) {
    CLEAR_FLAGS();
    IFX(X_PEND) {
        SET_DF(s3, d_tst16);
    } else IFX(X_ALL) {
        SET_DFNONE();
    }

    AND(s1, s1, s2); // res = s1 & s2

    IFX(X_PEND) {
        SH(s1, xEmu, offsetof(x64emu_t, res));
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

// emit AND32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_and32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4)
{
    CLEAR_FLAGS();
    IFX(X_PEND) {
        SET_DF(s3, rex.w?d_tst64:d_tst32);
    } else IFX(X_ALL) {
        SET_DFNONE();
    }

    AND(s1, s1, s2); // res = s1 & s2
    if (!rex.w) ZEROUP(s1);

    IFX(X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_SF) {
        SRLI(s3, s1, rex.w?63:31);
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

// emit AND32 instruction, from s1, c, store result in s1 using s3 and s4 as scratch
void emit_and32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4)
{
    CLEAR_FLAGS();
    IFX(X_PEND) {
        SET_DF(s3, rex.w?d_tst64:d_tst32);
    } else IFX(X_ALL) {
        SET_DFNONE();
    }

    if(c>=-2048 && c<=2047) {
        ANDI(s1, s1, c);
    } else {
        MOV64xw(s3, c);
        AND(s1, s1, s3); // res = s1 & s2
    }
    if (!rex.w && c<0 && c>=-2048) ZEROUP(s1);

    IFX(X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_SF) {
        SRLI(s3, s1, rex.w?63:31);
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

// emit OR8 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch, s4 can be same as s2 (and so s2 destroyed)
void emit_or8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    CLEAR_FLAGS();
    IFX(X_PEND) {
        SET_DF(s3, d_or8);
    } else IFX(X_ALL) {
        SET_DFNONE();
    }

    OR(s1, s1, s2);
    
    IFX(X_PEND) {
        SB(s1, xEmu, offsetof(x64emu_t, res));
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

// emit OR8 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_or8c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s2, int s3, int s4)
{
    MOV32w(s2, c&0xff);
    emit_or8(dyn, ninst, s1, s2, s3, s4);
}
