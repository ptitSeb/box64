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

    switch (opcode) {
        case 0x01:
            INST_NAME("VPHADDW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            VPICKEVxy(H, q0, v2, v1);
            VPICKODxy(H, q1, v2, v1);
            VADDxy(H, v0, q0, q1);
            break;
        case 0x02:
            INST_NAME("VPHADDD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            VPICKEVxy(W, q0, v2, v1);
            VPICKODxy(W, q1, v2, v1);
            VADDxy(W, v0, q0, q1);
            break;
        case 0x03:
            INST_NAME("VPHADDSW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            VPICKEVxy(H, q0, v2, v1);
            VPICKODxy(H, q1, v2, v1);
            VSADDxy(H, v0, q0, q1);
            break;
        case 0x04:
            INST_NAME("VPMADDUBSW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            VMULWEVxy(H_BU_B, q0, v1, v2);
            VMULWODxy(H_BU_B, q1, v1, v2);
            VSADDxy(H, v0, q0, q1);
            break;
        case 0x05:
            INST_NAME("VPHSUBW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            VPICKEVxy(H, q0, v2, v1);
            VPICKODxy(H, q1, v2, v1);
            VSUBxy(H, v0, q0, q1);
            break;
        case 0x06:
            INST_NAME("VPHSUBD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            VPICKEVxy(W, q0, v2, v1);
            VPICKODxy(W, q1, v2, v1);
            VSUBxy(W, v0, q0, q1);
            break;
        case 0x07:
            INST_NAME("VPHSUBSW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            VPICKEVxy(H, q0, v2, v1);
            VPICKODxy(H, q1, v2, v1);
            VSSUBxy(H, v0, q0, q1);
            break;
        case 0x08:
            INST_NAME("VPSIGNB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VSIGNCOVxy(B, v0, v2, v1);
            break;
        case 0x09:
            INST_NAME("VPSIGNW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VSIGNCOVxy(H, v0, v2, v1);
            break;
        case 0x0A:
            INST_NAME("VPSIGND Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VSIGNCOVxy(W, v0, v2, v1);
            break;
        case 0x0B:
            INST_NAME("VPMULHRSW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            if (vex.l) {
                XVMULWEV_W_H(q0, v1, v2);
                XVMULWOD_W_H(q1, v1, v2);
                XVSRLI_W(q0, q0, 14);
                XVSRLI_W(q1, q1, 14);
                XVADDI_WU(q0, q0, 1);
                XVADDI_WU(q1, q1, 1);
                XVSRLNI_H_W(q0, q0, 1);
                XVSRLNI_H_W(q1, q1, 1);
                XVILVL_H(v0, q1, q0);
            } else {
                VEXT2XV_W_H(q0, v1);
                VEXT2XV_W_H(q1, v2);
                XVMUL_W(q0, q0, q1);
                XVSRLI_W(q0, q0, 14);
                XVADDI_WU(q0, q0, 1);
                XVSRLNI_H_W(q0, q0, 1);
                XVPERMI_D(v0, q0, 0b1000);
            }
            break;
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
        case 0x1C:
            INST_NAME("VPABSB Gx, Ex");
            nextop = F8;
            GETGY_empty_EY_xy(v0, v1, 0);
            q0 = fpu_get_scratch(dyn);
            XVXOR_V(q0, q0, q0);
            VABSDxy(B, v0, v1, q0);
            break;
        case 0x1D:
            INST_NAME("VPABSW Gx, Ex");
            nextop = F8;
            GETGY_empty_EY_xy(v0, v1, 0);
            q0 = fpu_get_scratch(dyn);
            XVXOR_V(q0, q0, q0);
            VABSDxy(H, v0, v1, q0);
            break;
        case 0x1E:
            INST_NAME("VPABSD Gx, Ex");
            nextop = F8;
            GETGY_empty_EY_xy(v0, v1, 0);
            q0 = fpu_get_scratch(dyn);
            XVXOR_V(q0, q0, q0);
            VABSDxy(W, v0, v1, q0);
            break;
        case 0x20:
            INST_NAME("VPMOVSXBW Gx, Ex");
            nextop = F8;
            if (vex.l) {
                GETEYx(q1, 0, 0);
                GETGYy_empty(q0);
                VEXT2XV_H_B(q0, q1);
            } else {
                GETEYSD(q1, 0, 0);
                GETGYx_empty(q0);
                VSLLWIL_H_B(q0, q1, 0);
            }
            break;
        case 0x21:
            INST_NAME("VPMOVSXBD Gx, Ex");
            nextop = F8;
            if (vex.l) {
                GETEYSD(q1, 0, 0);
            } else {
                GETEYSS(q1, 0, 0);
            }
            GETGYxy_empty(q0);
            VEXT2XV_W_B(q0, q1);
            break;
        case 0x22:
            INST_NAME("VPMOVSXBQ Gx, Ex");
            nextop = F8;
            if (vex.l) {
                GETEYSS(q1, 0, 0);
            } else {
                if (MODREG) {
                    GETEYx(q1, 0, 0);
                } else {
                    SMREAD();
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                    LD_H(x5, ed, fixedaddress);
                    q1 = fpu_get_scratch(dyn);
                    MOVGR2FR_W(q1, x5);
                }
            }
            GETGYxy_empty(q0);
            VEXT2XV_D_B(q0, q1);
            break;
        case 0x23:
            INST_NAME("VPMOVSXWD Gx, Ex");
            nextop = F8;
            if (vex.l) {
                GETEYx(q1, 0, 0);
                GETGYy_empty(q0);
                VEXT2XV_W_H(q0, q1);
            } else {
                GETEYSD(q1, 0, 0);
                GETGYx_empty(q0);
                VSLLWIL_W_H(q0, q1, 0);
            }
            break;
        case 0x24:
            INST_NAME("VPMOVSXWQ Gx, Ex");
            nextop = F8;
            if (vex.l) {
                GETEYSD(q1, 0, 0);
            } else {
                GETEYSS(q1, 0, 0);
            }
            GETGYxy_empty(q0);
            VEXT2XV_D_H(q0, q1);
            break;
        case 0x25:
            INST_NAME("VPMOVSXDQ Gx, Ex");
            nextop = F8;
            if (vex.l) {
                GETEYx(q1, 0, 0);
                GETGYy_empty(q0);
                VEXT2XV_D_W(q0, q1);
            } else {
                GETEYSD(q1, 0, 0);
                GETGYx_empty(q0);
                VSLLWIL_D_W(q0, q1, 0);
            }
            break;
        case 0x28:
            INST_NAME("VPMULDQ Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VMULWEVxy(D_W, v0, v1, v2);
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
            INST_NAME("VMASKMOVPS Ex, Vx, Gx");
            nextop = F8;
            GETEY_VYGY_xy(v0, v1, v2, 0);
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
            INST_NAME("VMASKMOVPD Ex, Vx, Gx");
            nextop = F8;
            GETEY_VYGY_xy(v0, v1, v2, 0);
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
        case 0x30:
            INST_NAME("VPMOVZXBW Gx, Ex");
            nextop = F8;
            if (vex.l) {
                GETEYx(q1, 0, 0);
                GETGYy_empty(q0);
                VEXT2XV_HU_BU(q0, q1);
            } else {
                GETEYSD(q1, 0, 0);
                GETGYx_empty(q0);
                VSLLWIL_HU_BU(q0, q1, 0);
            }
            break;
        case 0x31:
            INST_NAME("VPMOVZXBD Gx, Ex");
            nextop = F8;
            if (vex.l) {
                GETEYSD(q1, 0, 0);
            } else {
                GETEYSS(q1, 0, 0);
            }
            GETGYxy_empty(q0);
            VEXT2XV_WU_BU(q0, q1);
            break;
        case 0x32:
            INST_NAME("VPMOVZXBQ Gx, Ex");
            nextop = F8;
            if (vex.l) {
                GETEYSS(q1, 0, 0);
            } else {
                if (MODREG) {
                    GETEYx(q1, 0, 0);
                } else {
                    SMREAD();
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                    LD_H(x5, ed, fixedaddress);
                    q1 = fpu_get_scratch(dyn);
                    MOVGR2FR_W(q1, x5);
                }
            }
            GETGYxy_empty(q0);
            VEXT2XV_DU_BU(q0, q1);
            break;
        case 0x33:
            INST_NAME("VPMOVZXWD Gx, Ex");
            nextop = F8;
            if (vex.l) {
                GETEYx(q1, 0, 0);
                GETGYy_empty(q0);
                VEXT2XV_WU_HU(q0, q1);
            } else {
                GETEYSD(q1, 0, 0);
                GETGYx_empty(q0);
                VSLLWIL_WU_HU(q0, q1, 0);
            }
            break;
        case 0x34:
            INST_NAME("VPMOVZXWQ Gx, Ex");
            nextop = F8;
            if (vex.l) {
                GETEYSD(q1, 0, 0);
            } else {
                GETEYSS(q1, 0, 0);
            }
            GETGYxy_empty(q0);
            VEXT2XV_DU_HU(q0, q1);
            break;
        case 0x35:
            INST_NAME("VPMOVZXDQ Gx, Ex");
            nextop = F8;
            if (vex.l) {
                GETEYx(q1, 0, 0);
                GETGYy_empty(q0);
                VEXT2XV_DU_WU(q0, q1);
            } else {
                GETEYSD(q1, 0, 0);
                GETGYx_empty(q0);
                VSLLWIL_DU_WU(q0, q1, 0);
            }
            break;
        case 0x38:
            INST_NAME("VPMINSB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VMINxy(B, v0, v1, v2);
            break;
        case 0x39:
            INST_NAME("VPMINSD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VMINxy(W, v0, v1, v2);
            break;
        case 0x3A:
            INST_NAME("VPMINUW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VMINxy(HU, v0, v1, v2);
            break;
        case 0x3B:
            INST_NAME("VPMINUD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VMINxy(WU, v0, v1, v2);
            break;
        case 0x3C:
            INST_NAME("VPMAXSB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VMAXxy(B, v0, v1, v2);
            break;
        case 0x3D:
            INST_NAME("VPMAXSD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VMAXxy(W, v0, v1, v2);
            break;
        case 0x3E:
            INST_NAME("VPMAXUW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VMAXxy(HU, v0, v1, v2);
            break;
        case 0x3F:
            INST_NAME("VPMAXUD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VMAXxy(WU, v0, v1, v2);
            break;
        case 0x40:
            INST_NAME("VPMULLD Gx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VMULxy(W, v0, v1, v2);
            break;
        case 0x45:
            INST_NAME("VPSRLVD/Q Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            d1 = fpu_get_scratch(dyn);
            if (rex.w) {
                d0 = fpu_get_scratch(dyn);
                VLDIxy(d0, (0b011 << 10) | 63);
                VSLExy(DU, d1, v2, d0);
                VSRLxy(D, v0, v1, v2);
                VAND_Vxy(v0, v0, d1);
            } else {
                VSLEIxy(WU, d1, v2, 31);
                VSRLxy(W, v0, v1, v2);
                VAND_Vxy(v0, v0, d1);
            }
            break;
        case 0x46:
            INST_NAME("VPSRAVD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            VMINIxy(WU, d0, v2, 31);
            VSRAxy(W, v0, v1, d0);
            break;
        case 0x47:
            INST_NAME("VPSLLVD/Q Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            d1 = fpu_get_scratch(dyn);
            if (rex.w) {
                d0 = fpu_get_scratch(dyn);
                VLDIxy(d0, (0b011 << 10) | 63);
                VSLExy(DU, d1, v2, d0);
                VSLLxy(D, v0, v1, v2);
                VAND_Vxy(v0, v0, d1);
            } else {
                VSLEIxy(WU, d1, v2, 31);
                VSLLxy(W, v0, v1, v2);
                VAND_Vxy(v0, v0, d1);
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
        case 0xF7:
            INST_NAME("SHLX Gd, Ed, Vd");
            nextop = F8;
            GETGD;
            GETED(0);
            GETVD;
            ANDI(x5, vd, rex.w ? 0x3f : 0x1f);
            SLLxw(gd, ed, x5);
            break;
        default:
            DEFAULT;
    }
    return addr;
}
