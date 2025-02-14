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

#include "arm64_printer.h"
#include "dynarec_arm64_private.h"
#include "dynarec_arm64_functions.h"
#include "../dynarec_helper.h"
#include "emu/x64compstrings.h"

uintptr_t dynarec64_66F30F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip; (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    int32_t i32;
    uint8_t gd, ed;
    uint8_t wback, wb1, wb2;
    uint8_t eb1, eb2;
    int64_t j64;
    uint64_t tmp64u, tmp64u2;
    int v0, v1;
    int q0, q1;
    int d0, d1;
    int64_t fixedaddress;
    int unscaled;

    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(j64);
    #if STEP > 1
    static const int8_t mask_shift8[] = { -7, -6, -5, -4, -3, -2, -1, 0 };
    static const int8_t mask_string8[] = { 7, 6, 5, 4, 3, 2, 1, 0 };
    static const int8_t mask_string16[] = { 15, 14, 13, 12, 11, 10, 9, 8 };
    static const int8_t round_round[] = { 0, 2, 1, 3};
    #endif

    switch(opcode) {
        case 0xB8:
            INST_NAME("POPCNT Gw, Ew");
            SETFLAGS(X_ALL, SF_SET);
            SET_DFNONE();
            nextop = F8;
            GETGW(x2);
            GETEW(x1, 0);
            v1 = fpu_get_scratch(dyn, ninst);
            VEORQ(v1, v1, v1);
            VMOVQDfrom(v1, 0, ed);
            CNT_8(v1, v1);
            UADDLV_8(v1, v1);
            VMOVHto(gd, v1, 0);
            IFX(X_ALL) {
                IFX(X_AF|X_PF|X_SF|X_OF|X_CF) {
                    MOV32w(x1, (1<<F_OF) | (1<<F_SF) | (1<<F_ZF) | (1<<F_AF) | (1<<F_CF) | (1<<F_PF));
                    BICw(xFlags, xFlags, x1);
                }
                IFX(X_ZF) {
                    CMPSw_U12(gd, 0);
                    IFNATIVE(NF_EQ) {}
                    else {
                        CSETw(x1, cEQ);
                        BFIw(xFlags, x1, F_ZF, 1);
                    }
                }
            }
            GWBACK;
            break;

        case 0xBC:
            INST_NAME("TZCNT Gw, Ew");
            SETFLAGS(X_CF|X_ZF, SF_SUBSET);
            SET_DFNONE();
            nextop = F8;
            GETEW(x1, 0);
            GETGW(x2);
            TSTxw_REG(ed, ed);
            IFX(X_CF) {
                CSETw(x3, cEQ);
                BFIw(xFlags, x3, F_CF, 1);  // CF = is source 0?
            }
            RBITw(x3, ed);   // reverse
            CLZw(gd, x3);    // x2 gets leading 0 == TZCNT
            MOV32w(x3, 16);
            CSELw(gd, x3, gd, cEQ); // if src is zero, use bit width as res
            IFX(X_ZF) {
                TSTxw_REG(gd, gd);
                CSETw(x3, cEQ);
                BFIw(xFlags, x3, F_ZF, 1);  // ZF = is dest 0?
            }
            GWBACK;
            break;

        case 0xBD:
            INST_NAME("LZCNT Gw, Ew");
            SETFLAGS(X_CF|X_ZF, SF_SUBSET);
            SET_DFNONE();
            nextop = F8;
            GETEW(x1, 0);
            GETGW(x2);
            IFX(X_CF) {
                TSTxw_REG(ed, ed);
                CSETw(x3, cEQ);
                BFIw(xFlags, x3, F_CF, 1);  // CF = is source 0?
            }
            CLZw(gd, ed);
            SUBw_U12(gd, gd, 16);   // sub zero cnt of high word
            IFX(X_ZF) {
                TSTxw_REG(gd, gd);
                CSETw(x3, cEQ);
                BFIw(xFlags, x3, F_ZF, 1);  // ZF = is dest 0?
            }
            GWBACK;
            break;

        default:
            DEFAULT;
    }
    return addr;
}
