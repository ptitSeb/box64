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

uintptr_t dynarec64_AVX_F3_0F(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed, vd;
    uint8_t wback, wb1, wb2, gback, vback;
    uint8_t eb1, eb2, gb1, gb2;
    int32_t i32, i32_;
    int cacheupd = 0;
    int v0, v1, v2;
    int q0, q1, q2;
    int d0, d1, d2;
    int s0;
    uint64_t tmp64u, u64;
    int64_t j64;
    int64_t fixedaddress, gdoffset, vxoffset;
    int unscaled;

    rex_t rex = vex.rex;

    switch (opcode) {
        case 0x10:
            INST_NAME("VMOVSS Gx, [Vx,] Ex");
            nextop = F8;
            if (MODREG) {
                GETVYx(q1, 0);
                GETEYSS(q2, 0, 0);
                GETGYx_empty(q0);
                if (!gd == vex.v) VOR_V(q0, q1, q1);
                VEXTRINS_W(q0, q2, 0);
            } else {
                GETEYSS(q2, 0, 0);
                GETGYx_empty(q0);
                XVPICKVE_W(q0, q2, 0);
                YMM_UNMARK_UPPER_ZERO(q0);
            }
            break;
        case 0x11:
            INST_NAME("VMOVSS Ex, [Vx,] Gx");
            nextop = F8;
            GETGYx(q2, 0);
            if (MODREG) {
                if (ed == vex.v) {
                    GETEYSS(q0, 1, 0);
                    VEXTRINS_W(q0, q2, 0);
                } else {
                    GETVYx(q1, 0);
                    GETEYSS(q0, 1, 0);
                    VOR_V(q0, q1, q1);
                    VEXTRINS_W(q0, q2, 0);
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                FST_S(q2, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0x12:
            INST_NAME("VMOVSLDUP Gx, Ex");
            nextop = F8;
            GETGY_empty_EY_xy(q0, q1, 0);
            if (vex.l) {
                XVPACKEV_W(q0, q1, q1);
            } else {
                VPACKEV_W(q0, q1, q1);
            }
            break;
        case 0x16:
            INST_NAME("VMOVSHDUP Gx, Ex");
            nextop = F8;
            GETGY_empty_EY_xy(q0, q1, 0);
            if (vex.l) {
                XVPACKOD_W(q0, q1, q1);
            } else {
                VPACKOD_W(q0, q1, q1);
            }
            break;
        case 0x6F:
            INST_NAME("VMOVDQU Gx, Ex");
            nextop = F8;
            if (MODREG) {
                GETGY_empty_EY_xy(q0, q1, 0);
                if (vex.l) {
                    XVOR_V(q0, q1, q1);
                } else {
                    VOR_V(q0, q1, q1);
                }
            } else {
                GETGYxy_empty(q0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                if (vex.l) {
                    XVLD(q0, ed, fixedaddress);
                } else {
                    VLD(q0, ed, fixedaddress);
                }
            }
            break;
        case 0x7E:
            INST_NAME("VMOVD Gx, Ex");
            nextop = F8;
            GETEYSD(q1, 0, 0);
            GETGYx_empty(q0);
            XVPICKVE_D(q0, q1, 0);
            YMM_UNMARK_UPPER_ZERO(q0);
            break;
        case 0x7F:
            INST_NAME("VMOVDQU Ex, Gx");
            nextop = F8;
            GETGYxy(q0, 0);
            if (MODREG) {
                GETEYxy_empty(q1, 0);
                if (vex.l) {
                    XVOR_V(q1, q0, q0);
                } else {
                    VOR_V(q1, q0, q0);
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x4, x5, &fixedaddress, rex, NULL, 1, 0);
                if (vex.l) {
                    XVST(q0, ed, fixedaddress);
                } else {
                    VST(q0, ed, fixedaddress);
                }
                SMWRITE2();
            }
            break;

        default:
            DEFAULT;
    }
    return addr;
}