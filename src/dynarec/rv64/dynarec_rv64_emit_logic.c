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

// emit XOR32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_xor32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4)
{
    CLEAR_FLAGS();
    IFX(X_PEND) {
        SET_DF(s4, rex.w?d_xor64:d_xor32);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }

    XORxw(s1, s1, s2);
    IFX(X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }

    IFX(X_ZF) {
        BNEZ(s1, 4);
        ORI(xFlags, xFlags, F_ZF);
    }
    IFX(X_SF) {
        SRLI(s3, s1, rex.w?63:31);
        BEQZ(s3, 4);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}
