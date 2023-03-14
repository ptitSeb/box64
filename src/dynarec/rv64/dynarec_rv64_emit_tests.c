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

// emit TEST32 instruction, from test s1, s2, using s3 and s4 as scratch
void emit_test32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    CLEAR_FLAGS();
    IFX_PENDOR0 {
        SET_DF(s3, rex.w?d_tst64:d_tst32);
    } else {
        SET_DFNONE(s4);
    }

    AND(s3, s1, s2); // res = s1 & s2

    IFX_PENDOR0 {
        SDxw(s3, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_SF) {
        if (!rex.w) ZEROUP(s3);
        SRLI(s4, s3, rex.w?63:31);
        BEQZ(s4, 4);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX(X_ZF) {
        BNEZ(s3, 4);
        ORI(xFlags, xFlags, F_ZF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s3, s4, s5);
    }
}
