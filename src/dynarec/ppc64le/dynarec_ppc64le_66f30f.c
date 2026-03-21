#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "ppc64le_emitter.h"
#include "ppc64le_mapping.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"

#include "ppc64le_printer.h"
#include "dynarec_ppc64le_private.h"
#include "../dynarec_helper.h"
#include "dynarec_ppc64le_functions.h"


uintptr_t dynarec64_66F30F(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed;
    uint8_t wback, wb1;
    int64_t j64;
    int64_t fixedaddress;
    int unscaled;
    MAYUSE(u8);
    MAYUSE(j64);

    switch (opcode) {
        case 0xB8:
            INST_NAME("POPCNT Gw, Ew");
            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            nextop = F8;
            GETGD;
            GETEW(x4, 0);
            // Mask to 16 bits, then popcount word
            RLDICL(x2, ed, 0, 48);   // zero-extend to 16 bits
            POPCNTW(x2, x2);
            BF_INSERT(gd, x2, 15, 0);
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
                // CF=1 if source is zero (all 16 bits zero)
                RLDICL(x3, ed, 0, 48);
                CMPWI(x3, 0);
                BNE(3*4);
                ORI(xFlags, xFlags, 1 << F_CF);
                B(1*4);   // skip the next nop-like
                // not zero: CF=0 (already cleared)
            }
            // TZCNT on 16-bit: set bit 16 as sentinel so CNTTZW gives correct result for all-zero
            LI(x4, 1);
            SLDI(x4, x4, 16);          // x4 = 0x10000
            RLDICL(x3, ed, 0, 48);     // x3 = zero-extended 16-bit value
            OR(x3, x3, x4);            // set bit 16 as sentinel
            CNTTZW(gd, x3);            // count trailing zeros (max 16)
            IFX (X_ZF) {
                // ZF=1 if result is zero (i.e., bit 0 was set)
                CMPWI(gd, 0);
                BNE(2*4);
                ORI(xFlags, xFlags, 1 << F_ZF);
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
                // CF=1 if source is zero
                RLDICL(x3, ed, 0, 48);
                CMPWI(x3, 0);
                BNE(3*4);
                ORI(xFlags, xFlags, 1 << F_CF);
                B(1*4);
            }
            // LZCNT on 16-bit: CNTLZW counts leading zeros of a 32-bit word
            // For a 16-bit value zero-extended to 32-bit, CNTLZW gives (result + 16)
            RLDICL(x3, ed, 0, 48);
            CNTLZW(gd, x3);
            ADDI(gd, gd, -16);         // adjust: subtract the 16 extra leading zeros
            IFX (X_ZF) {
                CMPWI(gd, 0);
                BNE(2*4);
                ORI(xFlags, xFlags, 1 << F_ZF);
            }
            SPILL_EFLAGS();
            GWBACK;
            break;
        default:
            DEFAULT;
    }
    return addr;
}
