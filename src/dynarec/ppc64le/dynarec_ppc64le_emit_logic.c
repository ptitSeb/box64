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

// emit XOR8 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch, s4 can be same as s2 (and so s2 destroyed)
void emit_xor8(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX (X_PEND) {
        SET_DF(s3, d_xor8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    XOR(s1, s1, s2);
    ANDId(s1, s1, 0xff);

    IFX (X_PEND) {
        STB(s1, offsetof(x64emu_t, res), xEmu);
    }

    CLEAR_FLAGS(s3);
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

// emit XOR8 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_xor8c(dynarec_ppc64le_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    IFX (X_PEND) {
        SET_DF(s3, d_xor8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    XORI(s1, s1, c & 0xff);
    ANDId(s1, s1, 0xff);

    CLEAR_FLAGS(s3);
    IFX (X_SF) {
        SRDI(s3, s1, 7);
        CMPDI(s3, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX (X_PEND) {
        STB(s1, offsetof(x64emu_t, res), xEmu);
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

// emit XOR16 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch, s4 can be same as s2 (and so s2 destroyed)
void emit_xor16(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    IFX (X_PEND) {
        SET_DF(s4, d_xor16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    XOR(s1, s1, s2);
    BF_EXTRACT(s1, s1, 15, 0);

    IFX (X_PEND) {
        STH(s1, offsetof(x64emu_t, res), xEmu);
    }

    CLEAR_FLAGS(s3);
    IFX (X_ZF | X_SF) {
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
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit XOR32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_xor32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4)
{
    IFX (X_PEND) {
        SET_DF(s3, rex.w ? d_xor64 : d_xor32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    CLEAR_FLAGS(s3);
    XOR(s1, s1, s2);

    // test sign bit before zeroup.
    IFX (X_SF) {
        if (!rex.w) EXTSW(s1, s1);
        CMPDI(s1, 0);
        BGE(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w && s1 != s2) {
        ZEROUP(s1);
    }

    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
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

// emit XOR32 instruction, from s1, c, store result in s1 using s3 and s4 as scratch
void emit_xor32c(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4)
{
    IFX (X_PEND) {
        SET_DF(s3, rex.w ? d_xor64 : d_xor32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    if (c >= 0 && c <= 65535) {
        XORI(s1, s1, c);
    } else {
        MOV64xw(s3, c);
        XOR(s1, s1, s3);
    }

    CLEAR_FLAGS(s3);
    // test sign bit before zeroup.
    IFX (X_SF) {
        if (!rex.w) EXTSW(s1, s1);
        CMPDI(s1, 0);
        BGE(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }

    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
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

// emit AND8 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch, s4 can be same as s2 (and so s2 destroyed)
void emit_and8(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX (X_PEND) {
        SET_DF(s3, d_and8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    AND(s1, s1, s2);

    IFX (X_PEND) {
        STB(s1, offsetof(x64emu_t, res), xEmu);
    }

    CLEAR_FLAGS(s3);
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

// emit AND8 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_and8c(dynarec_ppc64le_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    IFX (X_PEND) {
        SET_DF(s3, d_and8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    ANDId(s1, s1, c & 0xff);

    IFX (X_PEND) {
        STD(s1, offsetof(x64emu_t, res), xEmu);
    }

    CLEAR_FLAGS(s3);
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

// emit AND16 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_and16(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX (X_PEND) {
        SET_DF(s3, d_tst16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    AND(s1, s1, s2); // res = s1 & s2

    IFX (X_PEND) {
        STH(s1, offsetof(x64emu_t, res), xEmu);
    }

    CLEAR_FLAGS(s3);
    IFX (X_SF) {
        SRDI(s3, s1, 15);
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

// emit AND32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_and32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4)
{
    IFX (X_PEND) {
        SET_DF(s3, rex.w ? d_tst64 : d_tst32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    AND(s1, s1, s2); // res = s1 & s2
    if (!rex.w) ZEROUP(s1);

    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }

    CLEAR_FLAGS(s3);
    IFX (X_SF) {
        SRDI(s3, s1, rex.w ? 63 : 31);
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

// emit AND32 instruction, from s1, c, store result in s1 using s3 and s4 as scratch
void emit_and32c(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4)
{
    IFX (X_PEND) {
        SET_DF(s3, rex.w ? d_tst64 : d_tst32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    if (c >= 0 && c <= 65535) {
        ANDId(s1, s1, c);
    } else {
        MOV64xw(s3, c);
        AND(s1, s1, s3); // res = s1 & s2
    }

    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }

    CLEAR_FLAGS(s3);
    IFX (X_SF) {
        SRDI(s3, s1, rex.w ? 63 : 31);
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

// emit OR8 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch, s4 can be same as s2 (and so s2 destroyed)
void emit_or8(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX (X_PEND) {
        SET_DF(s3, d_or8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    OR(s1, s1, s2);

    IFX (X_PEND) {
        STB(s1, offsetof(x64emu_t, res), xEmu);
    }

    CLEAR_FLAGS(s3);
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

// emit OR8 instruction, from s1 , constant c, store result in s1 using s2, s3 and s4 as scratch
void emit_or8c(dynarec_ppc64le_t* dyn, int ninst, int s1, int32_t c, int s2, int s3, int s4)
{
    MOV32w(s2, c & 0xff);
    emit_or8(dyn, ninst, s1, s2, s3, s4);
}

// emit OR16 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch, s4 can be same as s2 (and so s2 destroyed)
void emit_or16(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX (X_PEND) {
        SET_DF(s3, d_or16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    OR(s1, s1, s2);
    BF_EXTRACT(s1, s1, 15, 0);
    IFX (X_PEND) {
        STD(s1, offsetof(x64emu_t, res), xEmu);
    }

    CLEAR_FLAGS(s3);
    IFX (X_SF) {
        SRDI(s3, s1, 15);
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

// emit OR32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_or32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4)
{
    IFX (X_PEND) {
        SET_DF(s4, rex.w ? d_or64 : d_or32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    OR(s1, s1, s2);

    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }

    CLEAR_FLAGS(s3);
    // test sign bit before zeroup.
    IFX (X_SF) {
        if (!rex.w) EXTSW(s1, s1);
        CMPDI(s1, 0);
        BGE(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }

    if (!rex.w) ZEROUP(s1);
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

// emit OR32 instruction, from s1, c, store result in s1 using s3 and s4 as scratch
void emit_or32c(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4)
{
    IFX (X_PEND) {
        SET_DF(s4, rex.w ? d_or64 : d_or32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    if (c >= 0 && c <= 65535) {
        ORI(s1, s1, c);
    } else {
        MOV64xw(s3, c);
        OR(s1, s1, s3);
    }

    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }

    CLEAR_FLAGS(s3);
    // test sign bit before zeroup.
    IFX (X_SF) {
        if (!rex.w) EXTSW(s1, s1);
        CMPDI(s1, 0);
        BGE(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
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
