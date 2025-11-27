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

// emit XOR8 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch, s4 can be same as s2 (and so s2 destroyed)
void emit_xor8(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX (X_PEND) {
        SET_DF(s4, d_xor8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFXA (X_ALL, cpuext.lbt) {
        X64_XOR_B(s1, s2);
    }

    XOR(s1, s1, s2);
    ANDI(s1, s1, 0xff);

    IFX (X_PEND) {
        ST_B(s1, xEmu, offsetof(x64emu_t, res));
    }

    if (cpuext.lbt) {
        if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
        return;
    }

    CLEAR_FLAGS(s3);
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
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit XOR8 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_xor8c(dynarec_la64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    IFX (X_PEND) {
        SET_DF(s4, d_xor8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    if (cpuext.lbt) {
        IFX (X_ALL) {
            ADDI_D(s3, xZR, c & 0xff);
            X64_XOR_B(s1, s3);
        }
        XORI(s1, s1, c & 0xff);
        ANDI(s1, s1, 0xff);
        IFX (X_PEND)
            ST_B(s1, xEmu, offsetof(x64emu_t, res));
        if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
        return;
    }

    XORI(s1, s1, c & 0xff);
    ANDI(s1, s1, 0xff);
    CLEAR_FLAGS(s3);
    IFX (X_SF) {
        SRLI_D(s3, s1, 7);
        BEQZ(s3, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX (X_PEND) {
        ST_B(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit XOR16 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch, s4 can be same as s2 (and so s2 destroyed)
void emit_xor16(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    IFX (X_PEND) {
        SET_DF(s4, d_xor16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }


    IFXA (X_ALL, cpuext.lbt) X64_XOR_W(s1, s2);

    XOR(s1, s1, s2);
    BSTRINS_D(s1, s1, 15, 0);

    IFXA (X_SF, BOX64DRENV(dynarec_safeflags)) {
        SRLI_D(s3, s1, 15 - F_SF);
        X64_SET_EFLAGS(s3, X_SF);
    }

    IFX (X_PEND) {
        ST_H(s1, xEmu, offsetof(x64emu_t, res));
    }

    if (cpuext.lbt) {
        if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
        return;
    }

    CLEAR_FLAGS(s3);
    IFX (X_ZF | X_SF) {
        IFX (X_ZF) {
            BNEZ(s1, 8);
            ORI(xFlags, xFlags, 1 << F_ZF);
        }
        IFX (X_SF) {
            SRLI_D(s3, s1, 15);
            BEQZ(s3, 8);
            ORI(xFlags, xFlags, 1 << F_SF);
        }
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit XOR32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_xor32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4)
{
    IFX(X_PEND) {
        SET_DF(s4, rex.w ? d_xor64 : d_xor32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    if (cpuext.lbt) {
        IFX(X_ALL) {
            if (rex.w)
                X64_XOR_D(s1, s2);
            else
                X64_XOR_W(s1, s2);
        }
        XOR(s1, s1, s2);
        if (!rex.w && s1 != s2) ZEROUP(s1);

        IFX(X_PEND)
            SDxw(s1, xEmu, offsetof(x64emu_t, res));
        if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
        return;
    }

    CLEAR_FLAGS(s3);
    XOR(s1, s1, s2);

    // test sign bit before zeroup.
    IFX(X_SF) {
        if (!rex.w) SEXT_W(s1, s1);
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w && s1 != s2) {
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
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}


// emit XOR32 instruction, from s1, c, store result in s1 using s3 and s4 as scratch
void emit_xor32c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4)
{
    IFX (X_PEND) {
        SET_DF(s4, rex.w ? d_xor64 : d_xor32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    if (cpuext.lbt) {
        IFX (X_ALL) {
            MOV64xw(s3, c);
            if (rex.w)
                X64_XOR_D(s1, s3);
            else
                X64_XOR_W(s1, s3);
        }

        if (c >= 0 && c <= 4095) {
            XORI(s1, s1, c);
        } else {
            IFX (X_ALL) {
            } else {
                MOV64xw(s3, c);
            }
            XOR(s1, s1, s3);
        }
        if (!rex.w) {
            ZEROUP(s1);
        }
        IFX (X_PEND)
            SDxw(s1, xEmu, offsetof(x64emu_t, res));
        if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
        return;
    }

    if (c >= 0 && c <= 4095) {
        XORI(s1, s1, c);
    } else {
        MOV64xw(s3, c);
        XOR(s1, s1, s3);
    }

    CLEAR_FLAGS(s3);
    // test sign bit before zeroup.
    IFX (X_SF) {
        if (!rex.w) SEXT_W(s1, s1);
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }

    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }

    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit AND8 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch, s4 can be same as s2 (and so s2 destroyed)
void emit_and8(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX(X_PEND) {
        SET_DF(s3, d_and8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFXA(X_ALL, cpuext.lbt) {
        X64_AND_B(s1, s2);
    }

    AND(s1, s1, s2);

    IFX(X_PEND) {
        ST_B(s1, xEmu, offsetof(x64emu_t, res));
    }

    if (cpuext.lbt) {
        if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
        return;
    }

    CLEAR_FLAGS(s3);
    IFX(X_SF) {
        SRLI_D(s3, s1, 7);
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
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}


// emit AND8 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_and8c(dynarec_la64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    IFX (X_PEND) {
        SET_DF(s3, d_and8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }


    IFXA(X_ALL, cpuext.lbt) {
        MOV32w(s3, c);
        X64_AND_B(s1, s3);
    }

    ANDI(s1, s1, c&0xff);

    IFX(X_PEND) {
        ST_D(s1, xEmu, offsetof(x64emu_t, res));
    }

    if (cpuext.lbt) {
        if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
        return;
    }

    CLEAR_FLAGS(s3);
    IFX(X_SF) {
        SRLI_D(s3, s1, 7);
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
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

void emit_and16(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX (X_PEND) {
        SET_DF(s3, d_tst16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFXA (X_ALL, cpuext.lbt) {
        X64_AND_H(s1, s2);
    }

    AND(s1, s1, s2); // res = s1 & s2


    IFX (X_PEND) {
        ST_H(s1, xEmu, offsetof(x64emu_t, res));
    }

    if (cpuext.lbt) {
        if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
        return;
    }

    CLEAR_FLAGS(s3);
    IFX (X_SF) {
        SRLI_D(s3, s1, 15);
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
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit AND32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_and32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4)
{
    IFX (X_PEND) {
        SET_DF(s3, rex.w ? d_tst64 : d_tst32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFXA(X_ALL, cpuext.lbt) {
        if (rex.w)
            X64_AND_D(s1, s2);
        else
            X64_AND_W(s1, s2);
    }

    AND(s1, s1, s2); // res = s1 & s2
    if (!rex.w) ZEROUP(s1);

    IFX(X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }

    if (cpuext.lbt) {
        if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
        return;
    }

    CLEAR_FLAGS(s3);
    IFX(X_SF) {
        SRLI_D(s3, s1, rex.w ? 63 : 31);
        BEQZ(s3, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX(X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_PF)
    {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit AND32 instruction, from s1, c, store result in s1 using s3 and s4 as scratch
void emit_and32c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4)
{
    IFX (X_PEND) {
        SET_DF(s3, rex.w ? d_tst64 : d_tst32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFXA(X_ALL, cpuext.lbt) {
        MOV64xw(s3, c);
        if (rex.w)
            X64_AND_D(s1, s3);
        else
            X64_AND_W(s1, s3);
    }

    if (c >= 0 && c <= 4095) {
        ANDI(s1, s1, c);
    } else {
        IFXA(X_ALL, cpuext.lbt) { } else MOV64xw(s3, c);
        AND(s1, s1, s3); // res = s1 & s2
    }

    IFX(X_PEND)  {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }

    if (cpuext.lbt) {
        if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
        return;
    }

    CLEAR_FLAGS(s3);
    IFX(X_SF) {
        SRLI_D(s3, s1, rex.w ? 63 : 31);
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
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}


// emit OR16 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch, s4 can be same as s2 (and so s2 destroyed)
void emit_or16(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX (X_PEND) {
        SET_DF(s3, d_or16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFXA (X_ALL, cpuext.lbt) {
        X64_OR_H(s1, s2);
    }

    OR(s1, s1, s2);
    BSTRPICK_D(s1, s1, 15, 0);
    IFX (X_PEND) {
        ST_D(s1, xEmu, offsetof(x64emu_t, res));
    }

    if (cpuext.lbt) {
        if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
        return;
    }

    CLEAR_FLAGS(s3);
    IFX (X_SF) {
        SRLI_D(s3, s1, 15);
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
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit OR32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_or32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4)
{
    IFX (X_PEND) {
        SET_DF(s4, rex.w?d_or64:d_or32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFXA (X_ALL, cpuext.lbt) {
        if (rex.w)
            X64_OR_D(s1, s2);
        else
            X64_OR_W(s1, s2);
    }

    OR(s1, s1, s2);

    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }

    if (cpuext.lbt) {
        if (!rex.w) ZEROUP(s1);
        if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
        return;
    }

    CLEAR_FLAGS(s3);
    // test sign bit before zeroup.
    IFX(X_SF) {
        if (!rex.w) SEXT_W(s1, s1);
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }

    if (!rex.w) ZEROUP(s1);
    IFX(X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit OR32 instruction, from s1, c, store result in s1 using s3 and s4 as scratch
void emit_or32c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4)
{
    IFX (X_PEND) {
        SET_DF(s4, rex.w ? d_or64 : d_or32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFXA(X_ALL, cpuext.lbt) {
        MOV64xw(s3, c);
        if (rex.w)
            X64_OR_D(s1, s3);
        else
            X64_OR_W(s1, s3);
    }

    if (c >= 0 && c <= 4095) {
        ORI(s1, s1, c);
    } else {
        IFXA(X_ALL, cpuext.lbt) { } else MOV64xw(s3, c);
        OR(s1, s1, s3);
    }

    IFX(X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }

    if (cpuext.lbt) {
        if (!rex.w) ZEROUP(s1);
        if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
        return;
    }

    CLEAR_FLAGS(s3);
    // test sign bit before zeroup.
    IFX(X_SF) {
        if (!rex.w) SEXT_W(s1, s1);
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }

    IFX(X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}


// emit OR8 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch, s4 can be same as s2 (and so s2 destroyed)
void emit_or8(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX (X_PEND) {
        SET_DF(s3, d_or8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFXA (X_ALL, cpuext.lbt) {
        X64_OR_B(s1, s2);
    }

    OR(s1, s1, s2);

    IFX (X_PEND) {
        ST_B(s1, xEmu, offsetof(x64emu_t, res));
    }

    if (cpuext.lbt) {
        if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
        return;
    }

    CLEAR_FLAGS(s3);
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
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit OR8 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_or8c(dynarec_la64_t* dyn, int ninst, int s1, int32_t c, int s2, int s3, int s4)
{
    MOV32w(s2, c & 0xff);
    emit_or8(dyn, ninst, s1, s2, s3, s4);
}
