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
            VPICKVE2GR_HU(x2, v0, 0);
            BSTRINS_D(gd, x2, 15, 0);
            IFX (X_ALL) {
                CLEAR_FLAGS(x5);
                BNEZ_MARK(x2);
                ORI(xFlags, xFlags, 1 << F_ZF);
                MARK;
                SPILL_EFLAGS();
            }
            break;
        case 0xBC:
            INST_NAME("TZCNT Gw, Ew");
            if (!BOX64DRENV(dynarec_safeflags)) {
                SETFLAGS(X_CF | X_ZF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            } else {
                SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            }
            SET_DFNONE();
            nextop = F8;
            GETEW(x1, 0);
            GETGW(x2);
            CLEAR_FLAGS(x2);
            IFX (X_CF) {
                SEQZ(x3, ed);
                BSTRINS_D(xFlags, x3, F_CF, F_CF);
            }
            ADDI_D(x4, xZR, -1);
            BSTRINS_D(x4, ed, 15, 0);
            CTZ_W(gd, x4);
            IFX (X_ZF) {
                SEQZ(x3, gd);
                BSTRINS_D(xFlags, x3, F_ZF, F_ZF);
            }
            SPILL_EFLAGS();
            GWBACK;
            break;
        case 0xBD:
            INST_NAME("LZCNT Gw, Ew");
            if (!BOX64DRENV(dynarec_safeflags)) {
                SETFLAGS(X_CF | X_ZF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            } else {
                SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            }
            SET_DFNONE();
            nextop = F8;
            GETEW(x1, 0);
            GETGW(x2);
            CLEAR_FLAGS(x2);
            IFX (X_CF) {
                SEQZ(x3, ed);
                BSTRINS_D(xFlags, x3, F_CF, F_CF);
            }
            CLZ_W(gd, ed);
            ADDI_D(gd, gd, -16);
            IFX (X_ZF) {
                SEQZ(x3, gd);
                BSTRINS_D(xFlags, x3, F_ZF, F_ZF);
            }
            SPILL_EFLAGS();
            GWBACK;
            break;
        default:
            DEFAULT;
    }
    return addr;
}
