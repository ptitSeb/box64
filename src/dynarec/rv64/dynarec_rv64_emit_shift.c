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

// emit SAR32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_sar32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4)
{
    CLEAR_FLAGS();

    IFX(X_PEND) {
        MOV64x(s3, c);
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SDxw(s3, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, rex.w?d_sar64:d_sar32);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    if(!c) {
        IFX(X_PEND) {
            SDxw(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }
    IFX(X_CF) {
        if (c > 1) {
            SRAI(s3, s1, c-1);
            ANDI(s3, s3, 1); // LSB
            BEQZ(s3, 4);
        } else {
            // no need to shift
            ANDI(s3, s1, 1);
            BEQZ(s3, 4);
        }
        ORI(xFlags, xFlags, F_CF);
    }

    SRAIxw(s1, s1, c);

    // SRAIW sign-extends, so test sign bit before clearing upper bits
    IFX(X_SF) {
        BGE(s1, xZR, 4);
        ORI(xFlags, xFlags, F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    IFX(X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_ZF) {
        BNEZ(s1, 4);
        ORI(xFlags, xFlags, F_ZF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}