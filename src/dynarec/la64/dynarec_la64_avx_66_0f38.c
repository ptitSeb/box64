#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "env.h"
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

uintptr_t dynarec64_AVX_66_0F38(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed;
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

    switch (opcode) {
        case 0x18:
            INST_NAME("VBROADCASTSS Gx, Ex");
            nextop = F8;
            GETEYSS(q2, 0, 0);
            GETGYxy_empty(q0);
            if (vex.l) {
                XVREPLVE0_W(q0, q2);
            } else {
                VREPLVE_W(q0, q2, 0);
            }
            break;
        case 0x19:
            INST_NAME("VBROADCASTSD Gx, Ex");
            nextop = F8;
            GETEYSD(q2, 0, 0);
            GETGYxy_empty(q0);
            if (vex.l) {
                XVREPLVE0_D(q0, q2);
            } else {
                VREPLVE_D(q0, q2, 0);
            }
            break;
        case 0x1A:
            INST_NAME("VBROADCASTF128 Gx, Ex");
            nextop = F8;
            GETGY_empty_EY_xy(q0, q2, 0);
            XVREPLVE0_Q(q0, q2);
            break;
        case 0x2C:
            INST_NAME("VMASKMOVPS Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            if (vex.l) {
                XVXOR_V(d0, d0, d0);
                XVSLTI_W(d1, v1, 0); // create all-one mask for negetive element.
                XVBITSEL_V(v0, d0, v2, d1);
            } else {
                VXOR_V(d0, d0, d0);
                VSLTI_W(d1, v1, 0); // create all-one mask for negetive element.
                VBITSEL_V(v0, d0, v2, d1);
            }
            break;
        case 0x2D:
            INST_NAME("VMASKMOVPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            if (vex.l) {
                XVXOR_V(d0, d0, d0);
                XVSLTI_D(d1, v1, 0); // create all-one mask for negetive element.
                XVBITSEL_V(v0, d0, v2, d1);
            } else {
                VXOR_V(d0, d0, d0);
                VSLTI_D(d1, v1, 0); // create all-one mask for negetive element.
                VBITSEL_V(v0, d0, v2, d1);
            }
            break;
        case 0x2E:
            INST_NAME("VMASKMOVPS Ex, Gx, Vx");
            nextop = F8;
            GETEY_VYGY_xy(v0, v2, v1, 0);
            d0 = fpu_get_scratch(dyn);
            if (vex.l) {
                XVSLTI_W(d0, v1, 0); // create all-one mask for negetive element.
                XVBITSEL_V(v0, v0, v2, d0);
                PUTEYy(v0);
            } else {
                VSLTI_W(d0, v1, 0); // create all-one mask for negetive element.
                VBITSEL_V(v0, v0, v2, d0);
                PUTEYx(v0);
            }
            break;
        case 0x2F:
            INST_NAME("VMASKMOVPD Ex, Gx, Vx");
            nextop = F8;
            GETEY_VYGY_xy(v0, v2, v1, 0);
            d0 = fpu_get_scratch(dyn);
            if (vex.l) {
                XVSLTI_D(d0, v1, 0); // create all-one mask for negetive element.
                XVBITSEL_V(v0, v0, v2, d0);
                PUTEYy(v0);
            } else {
                VSLTI_D(d0, v1, 0); // create all-one mask for negetive element.
                VBITSEL_V(v0, v0, v2, d0);
                PUTEYx(v0);
            }
            break;
        case 0x8C:
            INST_NAME("VPMASKMOVD/Q Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            if (vex.l) {
                XVXOR_V(d0, d0, d0);
                if (rex.w) {
                    XVSLTI_D(d1, v1, 0);
                } else {
                    XVSLTI_W(d1, v1, 0);
                }
                XVBITSEL_V(v0, d0, v2, d1);
            } else {
                VXOR_V(d0, d0, d0);
                if (rex.w) {
                    VSLTI_D(d1, v1, 0);
                } else {
                    VSLTI_W(d1, v1, 0);
                }
                VBITSEL_V(v0, d0, v2, d1);
            }
            break;
        case 0x8E:
            INST_NAME("VPMASKMOVD/Q Ex, Vx, Gx");
            nextop = F8;
            GETEY_VYGY_xy(v0, v2, v1, 0);
            d0 = fpu_get_scratch(dyn);
            if (vex.l) {
                if (rex.w) {
                    XVSLTI_D(d0, v1, 0);
                } else {
                    XVSLTI_W(d0, v1, 0);
                }
                XVBITSEL_V(v0, v0, v2, d0);
                PUTEYy(v0);
            } else {
                if (rex.w) {
                    VSLTI_D(d0, v1, 0);
                } else {
                    VSLTI_W(d0, v1, 0);
                }
                VBITSEL_V(v0, v0, v2, d0);
                PUTEYx(v0);
            }
            break;
        default:
            DEFAULT;
    }
    return addr;
}
