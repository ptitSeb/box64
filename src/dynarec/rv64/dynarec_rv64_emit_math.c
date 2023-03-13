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

// emit SUB32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_sub32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    CLEAR_FLAGS()
    IFX(X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SDxw(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, rex.w?d_sub64:d_sub32);
    } else IFX(X_ALL) {
        SET_DFNONE(s3);
    }

    IFX(X_AF) {
        // for later borrow chain calculation
        NOT(s5, s1);
    }

    SUBxw(s1, s1, s2);

    IFX(X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }

    IFX(X_AF | X_CF | X_OF) {
        // calc borrow chain
        // bc = (res & (~op1 | op2)) | (~op1 & op2)
        OR(s3, s5, s2);
        AND(s4, s1, s3);
        AND(s5, s5, s2);
        OR(s4, s4, s5);
        IFX(X_AF) {
            // af = bc & 0x8
            ANDI(s3, s4, 8);
            BEQZ(s3, 4);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX(X_CF) {
            // cf = bc & (rex.w?(1<<63):(1<<31))
            SRLI(s3, s4, rex.w?63:31);
            BEQZ(s3, 4);
            ORI(xFlags, xFlags, 1 << F_CF);
        }
        IFX(X_OF) {
            // of = ((bc >> rex.w?62:30) ^ (bc >> rex.w?63:31)) & 0x1;
            SRLI(s3, s4, rex.w?62:30);
            SRLI(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDI(s3, s3, 1);
            BEQZ(s3, 4);
            ORI(xFlags, xFlags, 1 << F_OF);
        }
    }
    IFX(X_ZF) {
        BEQZ(s1, 4);
        ORI(xFlags, xFlags, 1 << F_ZF);
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
