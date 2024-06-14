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
#include "my_cpuid.h"
#include "emu/x87emu_private.h"
#include "emu/x64shaext.h"

#include "arm64_printer.h"
#include "dynarec_arm64_private.h"
#include "dynarec_arm64_functions.h"
#include "dynarec_arm64_helper.h"

uintptr_t dynarec64_AVX_0F38(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
{
    (void)ip; (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed, vd;
    uint8_t wback, wb1, wb2;
    uint8_t eb1, eb2, gb1, gb2;
    int32_t i32, i32_;
    int cacheupd = 0;
    int v0, v1, v2;
    int q0, q1, q2;
    int d0, d1, d2;
    int s0;
    uint64_t tmp64u;
    int64_t j64;
    int64_t fixedaddress;
    int unscaled;
    MAYUSE(wb1);
    MAYUSE(wb2);
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(gb1);
    MAYUSE(gb2);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(s0);
    MAYUSE(j64);
    MAYUSE(cacheupd);

    rex_t rex = vex.rex;

    switch(opcode) {

        case 0xF2:
            INST_NAME("ANDN Gd, Vd, Ed");
            nextop = F8;
            SETFLAGS(X_ALL, SF_SET);
            GETGD;
            GETED(0);
            GETVD;
            BICxw(gd, ed, vd);
            break;
        case 0xF3:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 1:
                    INST_NAME("BLSR Vd, Ed");
                    SETFLAGS(X_ALL, SF_SET);
                    GETED(0);
                    GETVD;
                    IFX(X_CF) {
                        TSTxw_REG(ed, ed);
                        CSETMw(x3, cEQ);
                        BFIw(xFlags, x3, F_CF, 1);
                    }
                    SUBxw_U12(x3, ed, 1);
                    IFX(X_ZF)
                        ANDSxw_REG(vd, ed, x3);
                    else
                        ANDxw_REG(vd, ed, x3);
                    IFX(X_ZF) {
                        CSETMw(x3, cEQ);
                        BFIw(xFlags, x3, F_ZF, 1);
                    }
                    IFX(X_SF) {
                        LSRxw(x3, vd, rex.w?63:31);
                        BFIw(xFlags, x3, F_SF, 1);
                    }
                    IFX(X_OF) BFCw(xFlags, F_OF, 1);
                    if(box64_dynarec_test) {
                        IFX(X_AF) BFCw(xFlags, F_AF, 1);
                        IFX(X_PF) BFCw(xFlags, F_PF, 1);
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;

        case 0xF5:
            INST_NAME("BZHI Gd, Ed, Vd");
            nextop = F8;
            SETFLAGS(X_ALL, SF_SET);
            GETGD;
            GETED(0);
            GETVD;
            UXTBw(x1, vd);
            CMPSw_U12(x1, rex.w?64:32);
            CSETxw(x2, cPL);
            IFX(F_CF) {
                BFIw(xFlags, x2, F_CF, 1);
            }
            MVNxw_REG(x2, x2); //prepare mask
            B_MARK(cPL);
            LSLxw_REG(x2, x2, x1);
            MARK;
            IFX(X_ZF) {
                BICSxw(gd, ed, x2);
                CSETw(x3, cEQ);
                BFIw(xFlags, x3, F_ZF, 1);
            } else
                BICxw(gd, ed, x2);
            IFX(X_SF) {
                LSRxw(x3, gd, rex.w?63:31);
                BFIw(xFlags, x3, F_SF, 1);
            }
            IFX(X_AF) BFCw(xFlags, F_AF, 1);
            IFX(X_PF) BFCw(xFlags, F_PF, 1);
            IFX(X_OF) BFCw(xFlags, F_OF, 1);
            break;

        default:
            DEFAULT;
    }
    return addr;
}
