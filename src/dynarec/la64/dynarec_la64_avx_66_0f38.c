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
        case 0x00:
            INST_NAME("VPSHUFB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            VLDIxy(q0, 0b0000010001111); // broadcast 0b10001111 as byte
            VAND_Vxy(q0, q0, v2);
            VMINIxy(BU, q0, q0, 0x1f);
            VXOR_Vxy(q1, q1, q1);
            VSHUF_Bxy(v0, q1, v1, q0);
            break;
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
        case 0x0C:
            INST_NAME("VPERMILPS Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            VANDIxy(d0, v2, 0b11);
            VSHUFxy(W, d0, v1, v1);
            VOR_Vxy(v0, d0, d0);
            break;
        case 0x0D:
            INST_NAME("VPERMILPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            VSRLIxy(D, d0, v2, 0x1);
            VANDIxy(d0, d0, 0b1);
            VSHUFxy(D, d0, v2, v1);
            VOR_Vxy(v0, d0, d0);
            break;
        case 0x0E:
        case 0x0F:
            if (opcode == 0x0E) {
                INST_NAME("VTESTPS Gx, Ex");
            } else {
                INST_NAME("VTESTPD Gx, Ex");
            }
            nextop = F8;
            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            GETGYxy(q0, 0);
            GETEYxy(q1, 0, 0);
            if (!cpuext.lbt) {
                CLEAR_FLAGS(x3);
            } else IFX (X_ALL) {
                X64_SET_EFLAGS(xZR, X_ALL);
            }
            SET_DFNONE();
            v0 = fpu_get_scratch(dyn);
            IFX (X_ZF) {
                VAND_Vxy(v0, q1, q0);
                if (opcode == 0x0E) {
                    VMSKLTZxy(W, v0, v0);
                } else {
                    VMSKLTZxy(D, v0, v0);
                }
                VSETEQZ_Vxy(fcc0, v0);
                BCEQZ_MARK(fcc0);
                if (cpuext.lbt) {
                    ADDI_D(x3, xZR, 1 << F_ZF);
                    X64_SET_EFLAGS(x3, X_ZF);
                } else {
                    ORI(xFlags, xFlags, 1 << F_ZF);
                }
            }
            MARK;
            IFX (X_CF) {
                VANDN_Vxy(v0, q0, q1);
                if (opcode == 0x0E) {
                    VMSKLTZxy(W, v0, v0);
                } else {
                    VMSKLTZxy(D, v0, v0);
                }
                VSETEQZ_Vxy(fcc0, v0);
                BCEQZ_MARK2(fcc0);
                if (cpuext.lbt) {
                    ADDI_D(x3, xZR, 1 << F_CF);
                    X64_SET_EFLAGS(x3, X_CF);
                } else {
                    ORI(xFlags, xFlags, 1 << F_CF);
                }
            }
            MARK2;
            break;
        case 0x13:
            INST_NAME("VCVTPH2PS Gx, Ex");
            nextop = F8;
            GETEYSD(v1, 0, 0);
            GETGYxy_empty(v0);
            d0 = fpu_get_scratch(dyn);
            if (vex.l) {
                XVFCVTH_S_H(d0, v1);
                XVFCVTL_S_H(v0, v1);
                XVPERMI_Q(v0, d0, XVPERMI_IMM_4_0(0, 2));
            } else {
                VFCVTL_S_H(v0, v1);
            }
            break;
        case 0x16:
            INST_NAME("VPERMPS Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            XVPERM_W(v0, v2, v1);
            break;
        case 0x17:
            INST_NAME("VPTEST Gx, Ex");
            nextop = F8;
            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            GETGYxy(q0, 0);
            GETEYxy(q1, 0, 0);
            if (!cpuext.lbt) {
                CLEAR_FLAGS(x3);
            } else IFX (X_ALL) {
                X64_SET_EFLAGS(xZR, X_ALL);
            }
            SET_DFNONE();
            v0 = fpu_get_scratch(dyn);
            IFX (X_ZF) {
                VAND_Vxy(v0, q1, q0);
                VSETEQZ_Vxy(fcc0, v0);
                BCEQZ_MARK(fcc0);
                if (cpuext.lbt) {
                    ADDI_D(x3, xZR, 1 << F_ZF);
                    X64_SET_EFLAGS(x3, X_ZF);
                } else {
                    ORI(xFlags, xFlags, 1 << F_ZF);
                }
            }
            MARK;
            IFX (X_CF) {
                VANDN_Vxy(v0, q0, q1);
                VSETEQZ_Vxy(fcc0, v0);
                BCEQZ_MARK2(fcc0);
                if (cpuext.lbt) {
                    ADDI_D(x3, xZR, 1 << F_CF);
                    X64_SET_EFLAGS(x3, X_CF);
                } else {
                    ORI(xFlags, xFlags, 1 << F_CF);
                }
            }
            MARK2;
            break;
        case 0x18:
            INST_NAME("VBROADCASTSS Gx, Ex");
            nextop = F8;
            GETEYSS(q2, 0, 0);
            GETGYxy_empty(q0);
            VREPLVE0xy(W, q0, q2);
            break;
        case 0x19:
            INST_NAME("VBROADCASTSD Gx, Ex");
            nextop = F8;
            GETEYSD(q2, 0, 0);
            GETGYxy_empty(q0);
            VREPLVE0xy(D, q0, q2);
            break;
        case 0x1A:
            INST_NAME("VBROADCASTF128 Gx, Ex");
            nextop = F8;
            GETEYx(q2, 0, 0);
            GETGYxy_empty(q0);
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
        case 0x29:
            INST_NAME("VPCMPEQQ Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VSEQxy(D, v0, v2, v1);
            break;
        case 0x2A:
            INST_NAME("VMOVNTDQA Gx, Ex");
            nextop = F8;
            if (MODREG) {
                DEFAULT;
            } else {
                GETGYxy_empty(q0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 0, 0);
                if (vex.l) {
                    XVLD(q0, ed, fixedaddress);
                } else {
                    VLD(q0, ed, fixedaddress);
                }
            }
            break;
        case 0x2B:
            INST_NAME("VPACKUSDW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            d0 = fpu_get_scratch(dyn);
            VLDIxy(d0, (0b10111 << 8) | 0x00); // Broadcast 0x0000FFFF as 32bits to all lane
            if (v1 == v2) {
                VMAXIxy(W, q0, v1, 0);
                VMINxy(W, q0, q0, d0);
                VPICKEVxy(H, v0, q0, q0);
            } else {
                VMAXIxy(W, q1, v2, 0);
                VMAXIxy(W, q0, v1, 0);
                VMINxy(W, q1, q1, d0);
                VMINxy(W, q0, q0, d0);
                VPICKEVxy(H, v0, q1, q0);
            }
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
        case 0x36:
            INST_NAME("VPERMD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            XVPERM_W(v0, v2, v1);
            break;
        case 0x37:
            INST_NAME("VPCMPGTQ Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VSLTxy(D, v0, v2, v1);
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
        case 0x41:
            INST_NAME("VPHMINPOSUW Gx, Ex");
            nextop = F8;
            GETEYx(v1, 0, 0);
            GETGYx_empty(v0);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            q2 = fpu_get_scratch(dyn);
            // v1[a,b,c,d,e,f,g,h]
            VSHUF4I_W(q0, v1, 0b01001110); // q0[e,f,g,h,a,b,c,d]
            VMIN_HU(q1, v1, q0);           // q1[ae,bf,cg,dh ...]

            VSHUF4I_H(q2, q1, 0b10110001); // q2[bf,ae,dh,cg ...]
            VMIN_HU(q1, q1, q2);           // q1[aebf,aebf,cgdh,cgdh ...]
            VSHUF4I_H(q0, q1, 0b01001110); // q0[cgdh,cgdh,aebf,aebf]
            VMIN_HU(q2, q0, q1);           // all lane is min(abcdefgh)
            VSEQ_H(q0, q2, v1);            // get mask(0xffff)
            VFRSTPI_H(q2, q0, 1);          // find first neg(0xffff),insert index to q2
            XVPICKVE_W(v0, q2, 0);
            YMM_UNMARK_UPPER_ZERO(v0);
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
        case 0x58:
            INST_NAME("VPBROADCASTD Gx, Ex");
            nextop = F8;
            if (MODREG) {
                GETEYx(v1, 0, 0);
                GETGYxy_empty(v0);
                VREPLVE0xy(W, v0, v1);
            } else {
                GETGYxy_empty(v0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x4, x5, &fixedaddress, rex, NULL, 0, 0);
                VLDREPLxy(W, v0, ed, 0);
            }
            break;
        case 0x59:
            INST_NAME("VPBROADCASTQ Gx, Ex");
            nextop = F8;
            if (MODREG) {
                GETEYx(v1, 0, 0);
                GETGYxy_empty(v0);
                VREPLVE0xy(D, v0, v1);
            } else {
                GETGYxy_empty(v0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x4, x5, &fixedaddress, rex, NULL, 0, 0);
                VLDREPLxy(D, v0, ed, 0);
            }
            break;
        case 0x5A:
            INST_NAME("VBROADCASTI128 Gx, Ex");
            nextop = F8;
            GETEYx(q2, 0, 0);
            GETGYxy_empty(q0);
            XVREPLVE0_Q(q0, q2);
            break;
        case 0x78:
            INST_NAME("VPBROADCASTB Gx, Ex");
            nextop = F8;
            if (MODREG) {
                GETEYx(v1, 0, 0);
                GETGYxy_empty(v0);
                VREPLVE0xy(B, v0, v1);
            } else {
                GETGYxy_empty(v0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x4, x5, &fixedaddress, rex, NULL, 0, 0);
                VLDREPLxy(B, v0, ed, 0);
            }
            break;
        case 0x79:
            INST_NAME("VPBROADCASTW Gx, Ex");
            nextop = F8;
            if (MODREG) {
                GETEYx(v1, 0, 0);
                GETGYxy_empty(v0);
                VREPLVE0xy(H, v0, v1);
            } else {
                GETGYxy_empty(v0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x4, x5, &fixedaddress, rex, NULL, 0, 0);
                VLDREPLxy(H, v0, ed, 0);
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
        case 0x90:
        case 0x92:
            if (opcode == 0x90) {
                INST_NAME("VPGATHERDD/VPGATHERDQ Gx, VSIB, Vx");
            } else {
                INST_NAME("VGATHERDPD/VGATHERDPS Gx, VSIB, Vx");
            }
            nextop = F8;
            if (((nextop & 7) != 4) || MODREG) { UDF(); }
            GETG;
            u8 = F8; // SIB
            if ((u8 & 0x7) == 0x5 && !(nextop & 0xC0)) {
                int64_t i64 = F32S64;
                MOV64x(x5, i64);
                eb1 = x5;
            } else
                eb1 = TO_NAT((u8 & 0x7) + (rex.b << 3)); // base
            eb2 = ((u8 >> 3) & 7) + (rex.x << 3);        // index
            if (nextop & 0x40)
                i32 = F8S;
            else if (nextop & 0x80)
                i32 = F32S;
            else
                i32 = 0;
            if (!i32)
                ed = eb1;
            else {
                ed = x3;
                if (i32 < -2048 || i32 >= 2048) {
                    MOV64x(ed, i32);
                    ADD_D(ed, ed, eb1);
                } else {
                    ADDI_D(ed, eb1, i32);
                }
            }
            // ed is base
            wb1 = u8 >> 6; // scale
            GETVYxy(v2, 1);
            GETGYxy(v0, 1);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);

            if (vex.l) {
                v1 = avx_get_reg(dyn, ninst, x6, eb2, 0, LSX_AVX_WIDTH_256);
                if (rex.w) {
                    XVSRLI_D(d1, v2, 63);
                } else {
                    XVSRLI_W(d1, v2, 31);
                };
                for (int i = 0; i < (rex.w ? 4 : 8); i++) {
                    XVPICKVE2GRxw(x4, d1, i);
                    BEQZ(x4, 4 + 4 * 4);
                    XVPICKVE2GR_W(x4, v1, i);
                    SLLI_D(x4, x4, wb1);
                    LDXxw(x6, ed, x4);
                    XVINSGR2VRxw(v0, x6, i);
                }
                XVXOR_V(v2, v2, v2);
            } else {
                v1 = avx_get_reg(dyn, ninst, x6, eb2, 0, LSX_AVX_WIDTH_128);
                if (rex.w) {
                    VSRLI_D(d1, v2, 63);
                } else {
                    VSRLI_W(d1, v2, 31);
                };
                for (int i = 0; i < (rex.w ? 2 : 4); i++) {
                    VPICKVE2GRxw(x4, d1, i);
                    BEQZ(x4, 4 + 4 * 4);
                    VPICKVE2GR_W(x4, v1, i);
                    SLLI_D(x4, x4, wb1);
                    LDXxw(x6, ed, x4);
                    VINSGR2VRxw(v0, x6, i);
                }
                VXOR_V(v2, v2, v2);
            }
            break;
        case 0x91:
        case 0x93:
            if (opcode == 0x91) {
                INST_NAME("VPGATHERQD/VPGATHERQQ Gx, VSIB, Vx");
            } else {
                INST_NAME("VGATHERQPD/VGATHERQPS Gx, VSIB, Vx");
            }
            nextop = F8;
            if (((nextop & 7) != 4) || MODREG) { UDF(); }
            GETG;
            u8 = F8; // SIB
            if ((u8 & 0x7) == 0x5 && !(nextop & 0xC0)) {
                int64_t i64 = F32S64;
                MOV64x(x5, i64);
                eb1 = x5;
            } else
                eb1 = TO_NAT((u8 & 0x7) + (rex.b << 3)); // base
            eb2 = ((u8 >> 3) & 7) + (rex.x << 3);        // index
            if (nextop & 0x40)
                i32 = F8S;
            else if (nextop & 0x80)
                i32 = F32S;
            else
                i32 = 0;
            if (!i32)
                ed = eb1;
            else {
                ed = x3;
                if (i32 < -2048 || i32 >= 2048) {
                    MOV64x(ed, i32);
                    ADD_D(ed, ed, eb1);
                } else {
                    ADDI_D(ed, eb1, i32);
                }
            }
            // ed is base
            wb1 = u8 >> 6; // scale
            GETVYxy(v2, 1);
            GETGYxy(v0, 1);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);

            if (vex.l) {
                v1 = avx_get_reg(dyn, ninst, x6, eb2, 0, LSX_AVX_WIDTH_256);
                if (rex.w) {
                    XVSRLI_D(d1, v2, 63);
                } else {
                    XVSRLI_W(d1, v2, 31);
                };
                for (int i = 0; i < 4; i++) {
                    XVPICKVE2GRxw(x4, d1, i);
                    BEQZ(x4, 4 + 4 * 4);
                    XVPICKVE2GR_D(x4, v1, i);
                    SLLI_D(x4, x4, wb1);
                    LDXxw(x6, ed, x4);
                    XVINSGR2VRxw(v0, x6, i);
                }
                XVXOR_V(v2, v2, v2);
            } else {
                v1 = avx_get_reg(dyn, ninst, x6, eb2, 0, LSX_AVX_WIDTH_128);
                if (rex.w) {
                    VSRLI_D(d1, v2, 63);
                } else {
                    VSRLI_W(d1, v2, 31);
                };
                for (int i = 0; i < 2; i++) {
                    VPICKVE2GRxw(x4, d1, i);
                    BEQZ(x4, 4 + 4 * 4);
                    VPICKVE2GR_D(x4, v1, i);
                    SLLI_D(x4, x4, wb1);
                    LDXxw(x6, ed, x4);
                    VINSGR2VRxw(v0, x6, i);
                }
                VXOR_V(v2, v2, v2);
                if(!rex.w) VINSGR2VR_D(v0, xZR, 1);     // for set DEST[127:64] to zero, cause 128bit op only gather 2 32bits float.
            }
            break;
        case 0x96:
            INST_NAME("VFMADDSUB132PS/D Gx, Vx, Ex");
            nextop = F8;
            GETGY_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            VFMADDxyxw(q0, v0, v2, v1);
            VFMSUBxyxw(v0, v0, v2, v1);
            if (rex.w) {
                VEXTRINSxy(D, v0, q0, VEXTRINS_IMM_4_0(1, 1));
            } else {
                VEXTRINSxy(W, v0, q0, VEXTRINS_IMM_4_0(1, 1));
                VEXTRINSxy(W, v0, q0, VEXTRINS_IMM_4_0(3, 3));
            }
            break;
        case 0x97:
            INST_NAME("VFMSUBADD132PS/D Gx, Vx, Ex");
            nextop = F8;
            GETGY_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            VFMSUBxyxw(q0, v0, v2, v1);
            VFMADDxyxw(v0, v0, v2, v1);
            if (rex.w) {
                VEXTRINSxy(D, v0, q0, VEXTRINS_IMM_4_0(1, 1));
            } else {
                VEXTRINSxy(W, v0, q0, VEXTRINS_IMM_4_0(1, 1));
                VEXTRINSxy(W, v0, q0, VEXTRINS_IMM_4_0(3, 3));
            }
            break;
        case 0x98:
            INST_NAME("VFMADD132PS/D Gx, Vx, Ex");
            nextop = F8;
            GETGY_VYEY_xy(v0, v1, v2, 0);
            VFMADDxyxw(v0, v0, v2, v1);
            break;
        case 0x99:
            INST_NAME("VFMADD132SS/D Gx, Vx, Ex");
            nextop = F8;
            GETGYx_VYx_EYxw(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            FMADDxw(d0, v0, v2, v1);
            VEXTRINSxw(v0, d0, 0);
            break;
        case 0x9A:
            INST_NAME("VFMSUB132PS/D Gx, Vx, Ex");
            nextop = F8;
            GETGY_VYEY_xy(v0, v1, v2, 0);
            VFMSUBxyxw(v0, v0, v2, v1);
            break;
        case 0x9B:
            INST_NAME("VFMSUB132SS/D Gx, Vx, Ex");
            nextop = F8;
            GETGYx_VYx_EYxw(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            FMSUBxw(d0, v0, v2, v1);
            VEXTRINSxw(v0, d0, 0);
            break;
        case 0x9C:
            INST_NAME("VFNMADD132PS/D Gx, Vx, Ex");
            nextop = F8;
            GETGY_VYEY_xy(v0, v1, v2, 0);
            VFNMSUBxyxw(v0, v0, v2, v1);
            break;
        case 0x9D:
            INST_NAME("VFNMADD132SS/D Gx, Vx, Ex");
            nextop = F8;
            GETGYx_VYx_EYxw(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            FNMSUBxw(d0, v0, v2, v1);
            VEXTRINSxw(v0, d0, 0);
            break;
        case 0x9E:
            INST_NAME("VFNMSUB132PS/D Gx, Vx, Ex");
            nextop = F8;
            GETGY_VYEY_xy(v0, v1, v2, 0);
            VFNMADDxyxw(v0, v0, v2, v1);
            break;
        case 0x9F:
            INST_NAME("VFNMSUB132SS/D Gx, Vx, Ex");
            nextop = F8;
            GETGYx_VYx_EYxw(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            FNMADDxw(d0, v0, v2, v1);
            VEXTRINSxw(v0, d0, 0);
            break;
        case 0xA6:
            INST_NAME("VFMADDSUB213PS/D Gx, Vx, Ex");
            nextop = F8;
            GETGY_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            VFMADDxyxw(q0, v1, v0, v2);
            VFMSUBxyxw(v0, v1, v0, v2);
            if (rex.w) {
                VEXTRINSxy(D, v0, q0, VEXTRINS_IMM_4_0(1, 1));
            } else {
                VEXTRINSxy(W, v0, q0, VEXTRINS_IMM_4_0(1, 1));
                VEXTRINSxy(W, v0, q0, VEXTRINS_IMM_4_0(3, 3));
            }
            break;
        case 0xA7:
            INST_NAME("VFMSUBADD213PS/D Gx, Vx, Ex");
            nextop = F8;
            GETGY_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            VFMSUBxyxw(q0, v1, v0, v2);
            VFMADDxyxw(v0, v1, v0, v2);
            if (rex.w) {
                VEXTRINSxy(D, v0, q0, VEXTRINS_IMM_4_0(1, 1));
            } else {
                VEXTRINSxy(W, v0, q0, VEXTRINS_IMM_4_0(1, 1));
                VEXTRINSxy(W, v0, q0, VEXTRINS_IMM_4_0(3, 3));
            }
            break;
        case 0xA8:
            INST_NAME("VFMADD213PS/D Gx, Vx, Ex");
            nextop = F8;
            GETGY_VYEY_xy(v0, v1, v2, 0);
            VFMADDxyxw(v0, v1, v0, v2);
            break;
        case 0xA9:
            INST_NAME("VFMADD213SS/D Gx, Vx, Ex");
            nextop = F8;
            GETGYx_VYx_EYxw(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            FMADDxw(d0, v1, v0, v2);
            VEXTRINSxw(v0, d0, 0);
            break;
        case 0xAA:
            INST_NAME("VFMSUB213PS/D Gx, Vx, Ex");
            nextop = F8;
            GETGY_VYEY_xy(v0, v1, v2, 0);
            VFMSUBxyxw(v0, v1, v0, v2);
            break;
        case 0xAB:
            INST_NAME("VFMSUB213SS/D Gx, Vx, Ex");
            nextop = F8;
            GETGYx_VYx_EYxw(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            FMSUBxw(d0, v1, v0, v2);
            VEXTRINSxw(v0, d0, 0);
            break;
        case 0xAC:
            INST_NAME("VFNMADD213PS/D Gx, Vx, Ex");
            nextop = F8;
            GETGY_VYEY_xy(v0, v1, v2, 0);
            VFNMSUBxyxw(v0, v1, v0, v2);
            break;
        case 0xAD:
            INST_NAME("VFNMADD213SS/D Gx, Vx, Ex");
            nextop = F8;
            GETGYx_VYx_EYxw(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            FNMSUBxw(d0, v1, v0, v2);
            VEXTRINSxw(v0, d0, 0);
            break;
        case 0xAE:
            INST_NAME("VFNMSUB213PS/D Gx, Vx, Ex");
            nextop = F8;
            GETGY_VYEY_xy(v0, v1, v2, 0);
            VFNMADDxyxw(v0, v1, v0, v2);
            break;
        case 0xAF:
            INST_NAME("VFNMSUB213SS/D Gx, Vx, Ex");
            nextop = F8;
            GETGYx_VYx_EYxw(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            FNMADDxw(d0, v1, v0, v2);
            VEXTRINSxw(v0, d0, 0);
            break;
        case 0xB6:
            INST_NAME("VFMADDSUB231PS/D Gx, Vx, Ex");
            nextop = F8;
            GETGY_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            VFMADDxyxw(q0, v1, v2, v0);
            VFMSUBxyxw(v0, v1, v2, v0);
            if (rex.w) {
                VEXTRINSxy(D, v0, q0, VEXTRINS_IMM_4_0(1, 1));
            } else {
                VEXTRINSxy(W, v0, q0, VEXTRINS_IMM_4_0(1, 1));
                VEXTRINSxy(W, v0, q0, VEXTRINS_IMM_4_0(3, 3));
            }
            break;
        case 0xB7:
            INST_NAME("VFMSUBADD231PS/D Gx, Vx, Ex");
            nextop = F8;
            GETGY_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            VFMSUBxyxw(q0, v1, v2, v0);
            VFMADDxyxw(v0, v1, v2, v0);
            if (rex.w) {
                VEXTRINSxy(D, v0, q0, VEXTRINS_IMM_4_0(1, 1));
            } else {
                VEXTRINSxy(W, v0, q0, VEXTRINS_IMM_4_0(1, 1));
                VEXTRINSxy(W, v0, q0, VEXTRINS_IMM_4_0(3, 3));
            }
            break;
        case 0xB8:
            INST_NAME("VFMADD231PS/D Gx, Vx, Ex");
            nextop = F8;
            GETGY_VYEY_xy(v0, v1, v2, 0);
            VFMADDxyxw(v0, v1, v2, v0);
            break;
        case 0xB9:
            INST_NAME("VFMADD231SS/D Gx, Vx, Ex");
            nextop = F8;
            GETGYx_VYx_EYxw(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            FMADDxw(d0, v1, v2, v0);
            VEXTRINSxw(v0, d0, 0);
            break;
        case 0xBA:
            INST_NAME("VFMSUB231PS/D Gx, Vx, Ex");
            nextop = F8;
            GETGY_VYEY_xy(v0, v1, v2, 0);
            VFMSUBxyxw(v0, v1, v2, v0);
            break;
        case 0xBB:
            INST_NAME("VFMSUB231SS/D Gx, Vx, Ex");
            nextop = F8;
            GETGYx_VYx_EYxw(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            FMSUBxw(d0, v1, v2, v0);
            VEXTRINSxw(v0, d0, 0);
            break;
        case 0xBC:
            INST_NAME("VFNMADD231PS/D Gx, Vx, Ex");
            nextop = F8;
            GETGY_VYEY_xy(v0, v1, v2, 0);
            VFNMSUBxyxw(v0, v1, v2, v0);
            break;
        case 0xBD:
            INST_NAME("VFNMADD231SS/D Gx, Vx, Ex");
            nextop = F8;
            GETGYx_VYx_EYxw(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            FNMSUBxw(d0, v1, v2, v0);
            VEXTRINSxw(v0, d0, 0);
            break;
        case 0xBE:
            INST_NAME("VFNMSUB231PS/D Gx, Vx, Ex");
            nextop = F8;
            GETGY_VYEY_xy(v0, v1, v2, 0);
            VFNMADDxyxw(v0, v1, v2, v0);
            break;
        case 0xBF:
            INST_NAME("VFNMSUB231SS/D Gx, Vx, Ex");
            nextop = F8;
            GETGYx_VYx_EYxw(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            FNMADDxw(d0, v1, v2, v0);
            VEXTRINSxw(v0, d0, 0);
            break;
        case 0xDB:
            INST_NAME("VAESIMC Gx, Ex");
            nextop = F8;
            GETEYx(q1, 0, 0);
            GETGYx_empty(q0);
            if (q0 != q1) {
                VOR_V(q0, q1, q1);
            }
            avx_forget_reg(dyn, ninst, gd);
            MOV32w(x1, gd);
            CALL(const_native_aesimc, -1, x1, 0);
            GETGYx(q0, 1);  // reget writable for mark zeroup hi-128bits.
            break;
        case 0xDC:
            INST_NAME("VAESENC Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(q0, q1, q2, 0);
            if (MODREG && (gd == (nextop & 7) + (rex.b << 3))) {
                d0 = fpu_get_scratch(dyn);
                VOR_Vxy(d0, q2, q2);
            } else
                d0 = -1;
            if (gd != vex.v) {
                VOR_Vxy(q0, q1, q1);
            }
            avx_forget_reg(dyn, ninst, gd);
            MOV32w(x1, gd);
            CALL(const_native_aese, -1, x1, 0);
            if (vex.l) {
                MOV32w(x1, gd);
                CALL(const_native_aese_y, -1, x1, 0);
            }
            GETGYxy(q0, 1);
            VXOR_Vxy(q0, q0, (d0 != -1) ? d0 : q2);
            break;
        case 0xDD:
            INST_NAME("VAESENCLAST Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(q0, q1, q2, 0);
            if (MODREG && (gd == (nextop & 7) + (rex.b << 3))) {
                d0 = fpu_get_scratch(dyn);
                VOR_Vxy(d0, q2, q2);
            } else
                d0 = -1;
            if (gd != vex.v) {
                VOR_Vxy(q0, q1, q1);
            }
            avx_forget_reg(dyn, ninst, gd);
            MOV32w(x1, gd);
            CALL(const_native_aeselast, -1, x1, 0);
            if (vex.l) {
                MOV32w(x1, gd);
                CALL(const_native_aeselast_y, -1, x1, 0);
            }
            GETGYxy(q0, 1);
            VXOR_Vxy(q0, q0, (d0 != -1) ? d0 : q2);
            break;
        case 0xDE:
            INST_NAME("VAESDEC Gx, Vx, Ex"); // AES-NI
            nextop = F8;
            GETGY_empty_VYEY_xy(q0, q1, q2, 0);
            if (MODREG && (gd == (nextop & 7) + (rex.b << 3))) {
                d0 = fpu_get_scratch(dyn);
                VOR_Vxy(d0, q2, q2);
            } else
                d0 = -1;
            if (gd != vex.v) {
                VOR_Vxy(q0, q1, q1);
            }
            avx_forget_reg(dyn, ninst, gd);
            MOV32w(x1, gd);
            CALL(const_native_aesd, -1, x1, 0);
            if (vex.l) {
                MOV32w(x1, gd);
                CALL(const_native_aesd_y, -1, x1, 0);
            }
            GETGYxy(q0, 1);
            VXOR_Vxy(q0, q0, (d0 != -1) ? d0 : q2);
            break;
        case 0xDF:
            INST_NAME("VAESDECLAST Gx, Vx, Ex"); // AES-NI
            nextop = F8;
            GETGY_empty_VYEY_xy(q0, q1, q2, 0);
            if (MODREG && (gd == (nextop & 7) + (rex.b << 3))) {
                d0 = fpu_get_scratch(dyn);
                VOR_Vxy(d0, q2, q2);
            } else
                d0 = -1;
            if (gd != vex.v) {
                VOR_Vxy(q0, q1, q1);
            }
            avx_forget_reg(dyn, ninst, gd);
            MOV32w(x1, gd);
            CALL(const_native_aesdlast, -1, x1, 0);
            if (vex.l) {
                MOV32w(x1, gd);
                CALL(const_native_aesdlast_y, -1, x1, 0);
            }
            GETGYxy(q0, 1);
            VXOR_Vxy(q0, q0, (d0 != -1) ? d0 : q2);
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
