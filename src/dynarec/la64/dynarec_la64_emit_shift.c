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

// emit SHR32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_shr32c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4)
{
    CLEAR_FLAGS(s3);

    IFX(X_PEND) {
        if (c) {
            MOV64x(s3, c);
            SDxw(s3, xEmu, offsetof(x64emu_t, op2));
        } else
            SDxw(xZR, xEmu, offsetof(x64emu_t, op2));
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, rex.w ? d_shr64 : d_shr32);
    } else IFX(X_ALL) {
        SET_DFNONE();
    }
    if (!c) {
        IFX(X_PEND) {
            SDxw(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }

    if (la64_lbt) {
        IFX(X_PEND) {} else { MOV64x(s3, c); }
        IFX(X_ALL) {
            if (rex.w) X64_SRL_D(s1, s3); else X64_SRL_W(s1, s3);
            X64_GET_EFLAGS(s3, X_ALL);
            OR(xFlags, xFlags, s3);
        }

        SRLIxw(s1, s1, c);

        IFX(X_PEND) {
            SDxw(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }

    IFX(X_CF) {
        if (c > 1) {
            SRAI_D(s3, s1, c - 1);
            ANDI(s3, s3, 1); // LSB == F_CF
        } else {
            // no need to shift
            ANDI(s3, s1, 1); // LSB == F_CF
        }
        OR(xFlags, xFlags, s3);
    }
    IFX(X_OF) {
        // OF flag is affected only on 1-bit shifts
        // OF flag is set to the most-significant bit of the original operand
        if (c == 1) {
            SRLIxw(s3, s1, rex.w ? 63 : 31);
            SLLI_D(s3, s3, F_OF);
            OR(xFlags, xFlags, s3);
        }
    }

    SRLIxw(s1, s1, c);

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
    IFX(X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}


// emit SAR32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_sar32c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4)
{
    CLEAR_FLAGS(s3);

    IFX(X_PEND) {
        if (c) {
            MOV64x(s3, c);
            SDxw(s3, xEmu, offsetof(x64emu_t, op2));
        } else
            SDxw(xZR, xEmu, offsetof(x64emu_t, op2));
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, rex.w ? d_sar64 : d_sar32);
    } else IFX(X_ALL) {
        SET_DFNONE();
    }
    if (!c) {
        IFX(X_PEND) {
            SDxw(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }

    if (la64_lbt) {
        IFX(X_PEND) {} else { MOV64x(s3, c); }
        IFX(X_ALL) {
            if (rex.w) X64_SRA_D(s1, s3); else X64_SRA_W(s1, s3);
            X64_GET_EFLAGS(s3, X_ALL);
            OR(xFlags, xFlags, s3);
        }

        SRAIxw(s1, s1, c);

        IFX(X_PEND) {
            SDxw(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }

    IFX(X_CF) {
        if (c > 1) {
            SRAI_D(s3, s1, c - 1);
            ANDI(s3, s3, 1); // LSB == F_CF
        } else {
            // no need to shift
            ANDI(s3, s1, 1); // LSB == F_CF
        }
        OR(xFlags, xFlags, s3);
    }

    SRAIxw(s1, s1, c);

    // SRAIW sign-extends, so test sign bit before clearing upper bits
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
    IFX(X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}