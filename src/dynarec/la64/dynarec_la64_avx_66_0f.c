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

uintptr_t dynarec64_AVX_66_0F(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
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
        case 0x10:
            INST_NAME("VMOVUPD Gx, Ex");
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
        case 0x11:
            INST_NAME("VMOVUPD Ex, Gx");
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
        case 0x28:
            INST_NAME("VMOVAPD Gx, Ex");
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
        case 0x29:
            INST_NAME("VMOVAPD Ex, Gx");
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
        case 0x2B:
            INST_NAME("VMOVNTPD Ex, Gx");
            nextop = F8;
            GETGYxy(q0, 0);
            if (MODREG) {
                DEFAULT;
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
        case 0x6E:
            INST_NAME("VMOVD Gx, Ed");
            nextop = F8;
            GETED(0);
            GETGYx_empty(q0);
            XVXOR_V(q0, q0, q0);
            if (rex.w) {
                XVINSGR2VR_D(q0, ed, 0);
            } else {
                XVINSGR2VR_W(q0, ed, 0);
            }
            YMM_UNMARK_UPPER_ZERO(q0);
            break;
        case 0x6F:
            INST_NAME("VMOVDQA Gx, Ex");
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
            INST_NAME("VMOVD Ed, Gx");
            nextop = F8;
            GETGYx(v0, 0);
            if (MODREG) {
                ed = TO_NAT((nextop & 0x07) + (rex.b << 3));
                if (rex.w) {
                    VPICKVE2GR_D(ed, v0, 0);
                } else {
                    VPICKVE2GR_W(ed, v0, 0);
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                if (rex.w) {
                    FST_D(v0, ed, fixedaddress);
                } else {
                    FST_S(v0, ed, fixedaddress);
                }
                SMWRITE2();
            }
            break;
        case 0x7F:
            INST_NAME("VMOVDQA Ex, Gx");
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
        case 0xD6:
            INST_NAME("VMOVD Ex, Gx");
            nextop = F8;
            GETGYx(q0, 0);
            if (MODREG) {
                GETEYx_empty(q1, 0);
                XVXOR_V(q1, q1, q1);
                XVINSVE0_D(q1, q0, 0);
                YMM_UNMARK_UPPER_ZERO(q1);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x4, x5, &fixedaddress, rex, NULL, 1, 0);
                FST_D(q0, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0xE7:
            INST_NAME("VMOVNTDQ Ex, Gx");
            nextop = F8;
            GETGYxy(q0, 0);
            if (MODREG) {
                DEFAULT;
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
