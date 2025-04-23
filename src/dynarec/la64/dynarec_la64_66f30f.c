#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
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

uintptr_t dynarec64_66F30F(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8, s8;
    int32_t i32;
    uint8_t gd, ed;
    uint8_t wback, wb1, wb2, gback;
    uint8_t eb1, eb2;
    int64_t j64;
    uint64_t tmp64u, tmp64u2;
    int v0, v1;
    int q0, q1;
    int d0, d1;
    int64_t fixedaddress, gdoffset;
    int unscaled;

    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(j64);

    switch (opcode) {
        case 0xB8:
            INST_NAME("POPCNT Gw, Ew");
            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            nextop = F8;
            GETGD;
            GETEW(x4, 0);
            v0 = fpu_get_scratch(dyn);
            VINSGR2VR_H(v0, ed, 0);
            VPCNT_H(v0, v0);
            VPICKVE2GR_HU(gd, v0, 0);
            IFX (X_ALL) {
                if (la64_lbt) {
                    X64_SET_EFLAGS(xZR, X_ALL);
                    BNEZ_MARK(gd);
                    ADDI_D(x5, xZR, 1 << F_ZF);
                    X64_SET_EFLAGS(x5, X_ZF);
                } else {
                    CLEAR_FLAGS(x2);
                    BNEZ_MARK(gd);
                    ORI(xFlags, xFlags, 1 << F_ZF);
                }
                MARK;
            }
            break;
        case 0xBC:
            INST_NAME("TZCNT Gw, Ew");
            SETFLAGS(X_ZF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            nextop = F8;
            GETEW(x5, 0);
            GETGD;
            RESTORE_EFLAGS(x1);
            /*
                ZF is set if gd is zero, unset non-zero.
                CF is set if ed is zero, unset non-zero.
                OF, SF, PF, and AF flags are undefined
            */
            CLEAR_FLAGS(x2);
            ADDI_D(x4, xZR, -1);
            BSTRINS_D(x4, ed, 15, 0);
            CTZ_W(gd, x4);
            BNE(gd, xZR, 4 + 4);
            ORI(xFlags, xFlags, 1 << F_ZF);
            SRLI_W(x5, gd, 4); // maximum value is 16, F_CF = 0
            OR(xFlags, xFlags, x5);
            SPILL_EFLAGS();
            break;
        case 0xBD:
            INST_NAME("LZCNT Gw, Ew");
            SETFLAGS(X_ZF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            nextop = F8;
            GETEW(x5, 0);
            GETGD;
            RESTORE_EFLAGS(x1);
            /*
                ZF is set if gd is zero, unset non-zero.
                CF is set if ed is zero, unset non-zero.
                OF, SF, PF, and AF flags are undefined
            */
            CLEAR_FLAGS(x2);
            ADDI_D(x4, xZR, -1);
            BSTRINS_D(x4, ed, 31, 16);
            CLZ_W(gd, x4);
            BNE(gd, xZR, 4 + 4);
            ORI(xFlags, xFlags, 1 << F_ZF);
            SRLI_W(x5, gd, 4); // maximum value is 16, F_CF = 0
            OR(xFlags, xFlags, x5);
            SPILL_EFLAGS();
            break;
        default:
            DEFAULT;
    }
    return addr;
}
