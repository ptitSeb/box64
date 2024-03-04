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


// emit XOR32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_xor32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4)
{
    CLEAR_FLAGS(s3);
    IFX(X_PEND) {
        SET_DF(s4, rex.w ? d_xor64 : d_xor32);
    } else IFX(X_ALL) {
        SET_DFNONE();
    }

    if (la64_lbt) {
        IFX(X_ALL) {
            if (rex.w) X64_XOR_D(s1, s2); else X64_XOR_W(s1, s2);
            X64_GET_EFLAGS(s3, X_ALL);
            OR(xFlags, xFlags, s3);
        }
        XOR(s1, s1, s2);
        if (!rex.w && s1 != s2) ZEROUP(s1);

        IFX(X_PEND)
            SDxw(s1, xEmu, offsetof(x64emu_t, res));
        return;
    }

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
}