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
        case 0x12:
            INST_NAME("VMOVLPD Gx, Vx, Eq");
            nextop = F8;
            if (MODREG) {
                // access register instead of memory is bad opcode!
                DEFAULT;
                return addr;
            }
            GETVYx(v1, 0);
            GETEYSD(v2, 0, 0);
            GETGYx_empty(v0);
            VEXTRINS_D(v0, v2, VEXTRINS_IMM_4_0(0, 0));
            VEXTRINS_D(v0, v1, VEXTRINS_IMM_4_0(1, 1));
            break;
        case 0x13:
            INST_NAME("VMOVLPD Eq, Gx");
            nextop = F8;
            if (MODREG) {
                // access register instead of memory is bad opcode!
                DEFAULT;
                return addr;
            }
            GETGYx(v0, 0);
            addr = geted(dyn, addr, ninst, nextop, &ed, x4, x5, &fixedaddress, rex, NULL, 1, 0);
            FST_D(v0, ed, fixedaddress);
            SMWRITE2();
            break;
        case 0x16:
            INST_NAME("VMOVHPD Gx, Vx, Eq");
            nextop = F8;
            if (MODREG) {
                // access register instead of memory is bad opcode!
                DEFAULT;
                return addr;
            }
            GETVYx(v1, 0);
            GETEYSD(v2, 0, 0);
            GETGYx_empty(v0);
            VEXTRINS_D(v0, v2, VEXTRINS_IMM_4_0(1, 0));
            VEXTRINS_D(v0, v1, VEXTRINS_IMM_4_0(0, 0));
            break;
        case 0x17:
            INST_NAME("VMOVHPD Eq, Gx");
            nextop = F8;
            if (MODREG) {
                // access register instead of memory is bad opcode!
                DEFAULT;
                return addr;
            }
            GETGYx(v0, 0);
            addr = geted(dyn, addr, ninst, nextop, &ed, x4, x5, &fixedaddress, rex, NULL, 0, 0);
            VSTELM_D(v0, ed, 0, 1);
            SMWRITE2();
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
        case 0x50:
            nextop = F8;
            INST_NAME("VMOVMSKPD Gd, Ex");
            GETEYxy(v0, 0, 0);
            GETGD;
            d1 = fpu_get_scratch(dyn);
            if (vex.l) {
                XVMSKLTZ_D(d1, v0);
                VPICKVE2GR_DU(gd, d1, 0);
                VPICKVE2GR_DU(x4, d1, 2);
                BSTRINS_D(gd, x4, 3, 2);
            } else {
                VMSKLTZ_D(d1, v0);
                VPICKVE2GR_DU(gd, d1, 0);
            }
            break;
        case 0x54:
            INST_NAME("VANDPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VAND_Vxy(v0, v1, v2);
            break;
        case 0x55:
            INST_NAME("VANDNPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VANDN_Vxy(v0, v1, v2);
            break;
        case 0x56:
            INST_NAME("VORPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VOR_Vxy(v0, v1, v2);
            break;
        case 0x57:
            INST_NAME("VXORPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VXOR_Vxy(v0, v1, v2);
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
        case 0x71:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 2:
                    INST_NAME("VPSRLW Vx, Ex, Ib");
                    GETEYxy(v1, 0, 1);
                    GETVYxy_empty(v0);
                    u8 = F8;
                    if (u8 > 15) {
                        VXOR_Vxy(v0, v0, v0);
                    } else {
                        VSRLIxy(H, v0, v1, u8);
                    }
                    break;
                case 4:
                    INST_NAME("VPSRAW Vx, Ex, Ib");
                    GETEYxy(v1, 0, 1);
                    GETVYxy_empty(v0);
                    u8 = F8;
                    if (u8 > 15) u8 = 15;
                    if (u8) {
                        VSRAIxy(H, v0, v1, u8);
                    }
                    break;
                case 6:
                    INST_NAME("VPSLLW Vx, Ex, Ib");
                    GETEYxy(v1, 0, 1);
                    GETVYxy_empty(v0);
                    u8 = F8;
                    if (u8 > 15) {
                        VXOR_Vxy(v0, v0, v0);
                    } else {
                        VSLLIxy(H, v0, v1, u8);
                    }
                    break;
                default:
                    *ok = 0;
                    DEFAULT;
            }
            break;
        case 0x72:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 2:
                    INST_NAME("VPSRLD Vx, Ex, Ib");
                    GETEYxy(v1, 0, 1);
                    GETVYxy_empty(v0);
                    u8 = F8;
                    if (u8 > 31) {
                        VXOR_Vxy(v0, v0, v0);
                    } else {
                        VSRLIxy(W, v0, v1, u8);
                    }
                    break;
                case 4:
                    INST_NAME("VPSRAD Vx, Ex, Ib");
                    GETEYxy(v1, 0, 1);
                    GETVYxy_empty(v0);
                    u8 = F8;
                    if (u8 > 31) u8 = 31;
                    if (u8) {
                        VSRAIxy(W, v0, v1, u8);
                    }
                    break;
                case 6:
                    INST_NAME("VPSLLD Vx, Ex, Ib");
                    GETEYxy(v1, 0, 1);
                    GETVYxy_empty(v0);
                    u8 = F8;
                    if (u8 > 31) {
                        VXOR_Vxy(v0, v0, v0);
                    } else {
                        VSLLIxy(W, v0, v1, u8);
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0x73:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 2:
                    INST_NAME("VPSRLQ Vx, Ex, Ib");
                    GETEYxy(v1, 0, 1);
                    GETVYxy_empty(v0);
                    u8 = F8;
                    if (u8 > 63) {
                        VXOR_Vxy(v0, v0, v0);
                    } else {
                        VSRLIxy(D, v0, v1, u8);
                    }
                    break;
                case 3:
                    INST_NAME("VPSRLDQ Vx, Ex, Ib");
                    GETEYxy(v1, 0, 1);
                    GETVYxy_empty(v0);
                    u8 = F8;
                    if (u8 > 15) {
                        VXOR_Vxy(v0, v0, v0);
                    } else {
                        VBSRL_Vxy(v0, v1, u8);
                    }
                    break;
                case 6:
                    INST_NAME("VPSLLQ Vx, Ex, Ib");
                    GETEYxy(v1, 0, 1);
                    GETVYxy_empty(v0);
                    u8 = F8;
                    if (u8 > 63) {
                        VXOR_Vxy(v0, v0, v0);
                    } else {
                        VSLLIxy(D, v0, v1, u8);
                    }
                    break;
                case 7:
                    INST_NAME("VPSLLDQ Vx, Ex, Ib");
                    GETEYxy(v1, 0, 1);
                    GETVYxy_empty(v0);
                    u8 = F8;
                    if (u8 > 15) {
                        VXOR_Vxy(v0, v0, v0);
                    } else {
                        VBSLL_Vxy(v0, v1, u8);
                    }
                    break;
                default:
                    DEFAULT;
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
        case 0xD1:
            INST_NAME("VPSRLW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            d0 = fpu_get_scratch(dyn);
            VREPLVE0xy(D, q0, v2);
            VREPLVE0xy(H, d0, v2);
            VSLEIxy(DU, q0, q0, 15);
            VSRLxy(H, v0, v1, d0);
            VAND_Vxy(v0, v0, q0);
            break;
        case 0xD2:
            INST_NAME("VPSRLD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            d0 = fpu_get_scratch(dyn);
            VREPLVE0xy(D, q0, v2);
            VREPLVE0xy(W, d0, v2);
            VSLEIxy(DU, q0, q0, 31);
            VSRLxy(W, v0, v1, d0);
            VAND_Vxy(v0, v0, q0);
            break;
        case 0xD3:
            INST_NAME("VPSRLQ Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            d0 = fpu_get_scratch(dyn);
            VREPLVE0xy(D, q0, v2);
            VLDIxy(d0, (0b011 << 10) | 0x3f);
            VSLExy(DU, d0, q0, d0);
            VSRLxy(D, v0, v1, q0);
            VAND_Vxy(v0, v0, d0);
            break;
        case 0xD4:
            INST_NAME("VPADDQ Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VADDxy(D, v0, v1, v2);
            break;
        case 0xD5:
            INST_NAME("VPMULLW Gx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VMULxy(H, v0, v1, v2);
            break;
        case 0xD6:
            INST_NAME("VMOVD Ex, Gx");
            nextop = F8;
            GETGYx(q0, 0);
            if (MODREG) {
                GETEYx_empty(q1, 0);
                XVPICKVE_D(q1, q0, 0);
                YMM_UNMARK_UPPER_ZERO(q1);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x4, x5, &fixedaddress, rex, NULL, 1, 0);
                FST_D(q0, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0xD7:
            nextop = F8;
            INST_NAME("VPMOVMSKB Gd, Ex");
            GETEYxy(v0, 0, 0);
            GETGD;
            d1 = fpu_get_scratch(dyn);
            if (vex.l) {
                XVMSKLTZ_B(d1, v0);
                VPICKVE2GR_DU(gd, d1, 0);
                VPICKVE2GR_DU(x4, d1, 2);
                BSTRINS_D(gd, x4, 31, 16);
            } else {
                VMSKLTZ_B(d1, v0);
                VPICKVE2GR_DU(gd, d1, 0);
            }
            break;
        case 0xD8:
            INST_NAME("VPSUBUSB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VSSUBxy(BU, v0, v1, v2);
            break;
        case 0xD9:
            INST_NAME("VPSUBUSW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VSSUBxy(HU, v0, v1, v2);
            break;
        case 0xDA:
            INST_NAME("VPMINUB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VMINxy(BU, v0, v1, v2);
            break;
        case 0xDB:
            INST_NAME("VPAND Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VAND_Vxy(v0, v1, v2);
            break;
        case 0xDC:
            INST_NAME("VPADDUSB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VSADDxy(BU, v0, v1, v2);
            break;
        case 0xDD:
            INST_NAME("VPADDUSW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VSADDxy(HU, v0, v1, v2);
            break;
        case 0xDE:
            INST_NAME("VPMAXUB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VMAXxy(BU, v0, v1, v2);
            break;
        case 0xDF:
            INST_NAME("VPANDN Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VANDN_Vxy(v0, v1, v2);
            break;
        case 0xE0:
            INST_NAME("VPAVGB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VAVGRxy(BU, v0, v1, v2);
            break;
        case 0xE1:
            INST_NAME("VPSRAW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            VMINIxy(DU, d0, v2, 15);
            VREPLVE0xy(H, d0, d0);
            VSRAxy(H, v0, v1, d0);
            break;
        case 0xE2:
            INST_NAME("VPSRAD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            VMINIxy(DU, d0, v2, 31);
            VREPLVE0xy(W, d0, d0);
            VSRAxy(W, v0, v1, d0);
            break;
        case 0xE3:
            INST_NAME("VPAVGW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VAVGRxy(HU, v0, v1, v2);
            break;
        case 0xE4:
            INST_NAME("VPMULHUW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VMUHxy(HU, v0, v1, v2);
            break;
        case 0xE5:
            INST_NAME("VPMULHW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VMUHxy(H, v0, v1, v2);
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
        case 0xE8:
            INST_NAME("VPSUBSB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VSSUBxy(B, v0, v1, v2);
            break;
        case 0xE9:
            INST_NAME("VPSUBSW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VSSUBxy(H, v0, v1, v2);
            break;
        case 0xEA:
            INST_NAME("VPMINSW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VMINxy(H, v0, v1, v2);
            break;
        case 0xEB:
            INST_NAME("VPOR Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VOR_Vxy(v0, v1, v2);
            break;
        case 0xEC:
            INST_NAME("VPADDSB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VSADDxy(B, v0, v1, v2);
            break;
        case 0xED:
            INST_NAME("VPADDSW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VSADDxy(H, v0, v1, v2);
            break;
        case 0xEE:
            INST_NAME("VPMAXSW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VMAXxy(H, v0, v1, v2);
            break;
        case 0xEF:
            INST_NAME("VPXOR Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VXOR_Vxy(v0, v1, v2);
            break;
        case 0xF1:
            INST_NAME("VPSLLW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            d0 = fpu_get_scratch(dyn);
            VREPLVE0xy(D, q0, v2);
            VSLEIxy(DU, q0, q0, 15);
            VREPLVE0xy(H, d0, v2);
            VSLLxy(H, v0, v1, d0);
            VAND_Vxy(v0, v0, q0);
            break;
        case 0xF2:
            INST_NAME("VPSLLD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            d0 = fpu_get_scratch(dyn);
            VREPLVE0xy(D, q0, v2);
            VSLEIxy(DU, q0, q0, 31);
            VREPLVE0xy(W, d0, v2);
            VSLLxy(W, v0, v1, d0);
            VAND_Vxy(v0, v0, q0);
            break;
        case 0xF3:
            INST_NAME("VPSLLQ Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            d0 = fpu_get_scratch(dyn);
            VREPLVE0xy(D, q0, v2);
            VLDIxy(d0, (0b011 << 10) | 0x3f);
            VSLExy(DU, d0, q0, d0);
            VSLLxy(D, v0, v1, q0);
            VAND_Vxy(v0, v0, d0);
            break;
        case 0xF4:
            INST_NAME("VPMULLUDQ Gx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VMULWEVxy(D_WU, v0, v1, v2);
            break;
        case 0xF5:
            INST_NAME("VPMADDWD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            VMULWEVxy(W_H, q0, v1, v2);
            VMULWODxy(W_H, q1, v1, v2);
            VADDxy(W, v0, q0, q1);
            break;
        case 0xF6:
            INST_NAME("VPSADBW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VABSDxy(BU, v0, v1, v2);
            VHADDWxy(HU_BU, v0, v0, v0);
            VHADDWxy(WU_HU, v0, v0, v0);
            VHADDWxy(DU_WU, v0, v0, v0);
            break;
        case 0xF7:
            INST_NAME("VMASKMOVDQU Gx, Ex");
            nextop = F8;
            GETEYx(v1, 0, 0);
            GETGYx(v0, 1);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            VSLTI_B(q1, v1, 0); // q1 = byte selection mask
            VLD(q0, xRDI, 0);
            VBITSEL_V(q0, q0, v0, q1); // sel v0 if mask is 1
            VST(q0, xRDI, 0);
            break;
        case 0xF8:
            INST_NAME("VPSUBB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VSUBxy(B, v0, v1, v2);
            break;
        case 0xF9:
            INST_NAME("VPSUBW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VSUBxy(H, v0, v1, v2);
            break;
        case 0xFA:
            INST_NAME("VPSUBD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VSUBxy(W, v0, v1, v2);
            break;
        case 0xFB:
            INST_NAME("VPSUBQ Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VSUBxy(D, v0, v1, v2);
            break;
        case 0xFC:
            INST_NAME("VPADDB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VADDxy(B, v0, v1, v2);
            break;
        case 0xFD:
            INST_NAME("VPADDW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VADDxy(H, v0, v1, v2);
            break;
        case 0xFE:
            INST_NAME("VPADDD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VADDxy(W, v0, v1, v2);
            break;
        default:
            DEFAULT;
    }
    return addr;
}
