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
