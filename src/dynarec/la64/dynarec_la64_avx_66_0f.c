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
        case 0x14:
            INST_NAME("VUNPCKLPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VILVLxy(D, v0, v2, v1);
            break;
        case 0x15:
            INST_NAME("VUNPCKHPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VILVHxy(D, v0, v2, v1);
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
        case 0x2E:
            // no special check...
        case 0x2F:
            if (opcode == 0x2F) {
                INST_NAME("VCOMISD Gx, Ex");
            } else {
                INST_NAME("VUCOMISD Gx, Ex");
            }
            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            nextop = F8;
            GETGYx(d0, 0);
            GETEYSD(v0, 0, 0);

            CLEAR_FLAGS(x3);
            // if isnan(d0) || isnan(v0)
            IFX (X_ZF | X_PF | X_CF) {
                FCMP_D(fcc0, d0, v0, cUN);
                BCEQZ_MARK(fcc0);
                ORI(xFlags, xFlags, (1 << F_ZF) | (1 << F_PF) | (1 << F_CF));
                B_MARK3_nocond;
            }
            MARK;
            // else if isless(d0, v0)
            IFX (X_CF) {
                FCMP_D(fcc1, d0, v0, cLT);
                BCEQZ_MARK2(fcc1);
                ORI(xFlags, xFlags, 1 << F_CF);
                B_MARK3_nocond;
            }
            MARK2;
            // else if d0 == v0
            IFX (X_ZF) {
                FCMP_D(fcc2, d0, v0, cEQ);
                BCEQZ_MARK3(fcc2);
                ORI(xFlags, xFlags, 1 << F_ZF);
            }
            MARK3;
            SPILL_EFLAGS();
            break;
        case 0x50:
            nextop = F8;
            INST_NAME("VMOVMSKPD Gd, Ex");
            GETEYxy(v0, 0, 0);
            GETGD;
            d1 = fpu_get_scratch(dyn);
            if (vex.l) {
                XVMSKLTZ_D(d1, v0);
                XVPICKVE2GR_DU(gd, d1, 0);
                XVPICKVE2GR_DU(x4, d1, 2);
                BSTRINS_D(gd, x4, 3, 2);
            } else {
                VMSKLTZ_D(d1, v0);
                VPICKVE2GR_DU(gd, d1, 0);
            }
            break;
        case 0x51:
            INST_NAME("VSQRTPD Gx, Ex");
            nextop = F8;
            GETGY_empty_EY_xy(v0, v1, 0);
            if (!BOX64ENV(dynarec_fastnan)) {
                d0 = fpu_get_scratch(dyn);
                d1 = fpu_get_scratch(dyn);
                VFCMPxy(D, d0, v1, v1, cEQ);
                VFSQRTxy(D, v0, v1);
                VFCMPxy(D, d1, v0, v0, cEQ);
                VANDN_Vxy(d1, d1, d0);
                VSLLIxy(D, d1, d1, 63);
                VOR_Vxy(v0, v0, d1);
            } else {
                VFSQRTxy(D, v0, v1);
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
        case 0x58:
            INST_NAME("VADDPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            if (!BOX64ENV(dynarec_fastnan)) {
                d0 = fpu_get_scratch(dyn);
                d1 = fpu_get_scratch(dyn);
                VFCMPxy(D, d0, v1, v2, cUN);
            }
            VFADDxy(D, v0, v1, v2);
            if (!BOX64ENV(dynarec_fastnan)) {
                VFCMPxy(D, d1, v0, v0, cUN);
                VANDN_Vxy(d0, d0, d1);
                VLDIxy(d1, (0b011 << 9) | 0b111111000);
                VSLLIxy(D, d1, d1, 48); // broadcast 0xfff8000000000000
                VBITSEL_Vxy(v0, v0, d1, d0);
            }
            break;
        case 0x59:
            INST_NAME("VMULPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            if (!BOX64ENV(dynarec_fastnan)) {
                d0 = fpu_get_scratch(dyn);
                d1 = fpu_get_scratch(dyn);
                VFCMPxy(D, d0, v1, v2, cUN);
            }
            VFMULxy(D, v0, v1, v2);
            if (!BOX64ENV(dynarec_fastnan)) {
                VFCMPxy(D, d1, v0, v0, cUN);
                VANDN_Vxy(d0, d0, d1);
                VLDIxy(d1, (0b011 << 9) | 0b111111000);
                VSLLIxy(D, d1, d1, 48); // broadcast 0xfff8000000000000
                VBITSEL_Vxy(v0, v0, d1, d0);
            }
            break;
        case 0x5A:
            INST_NAME("CVTPD2PS Gx, Ex");
            nextop = F8;
            GETEYxy(v1, 0, 0);
            GETGYx_empty(v0);
            if(!BOX64ENV(dynarec_fastround)) {
                u8 = sse_setround(dyn, ninst, x6, x4);
            }
            d0 = fpu_get_scratch(dyn);
            if(vex.l) {
                XVXOR_V(d0, d0, d0);
                XVFCVT_S_D(v0, d0, v1);
                XVPERMI_D(v0, v0, 0b11011000);
            } else {
                VFCVT_S_D(d0, v1, v1);
                XVPICKVE_D(v0, d0, 0);
                YMM_UNMARK_UPPER_ZERO(v0);
            }
            if(!BOX64ENV(dynarec_fastround)) {
                x87_restoreround(dyn, ninst, u8);
            }
            break;
        case 0x5B:
            INST_NAME("VCVTPS2DQ Gx, Ex");
            nextop = F8;
            GETGY_empty_EY_xy(v0, v1, 0);
            u8 = sse_setround(dyn, ninst, x6, x4);
            if(vex.l){
                if (!BOX64ENV(dynarec_fastround)) {
                    d1 = fpu_get_scratch(dyn);
                    q0 = fpu_get_scratch(dyn);
                    q1 = fpu_get_scratch(dyn); // mask
                    d0 = fpu_get_scratch(dyn);
                    XVFTINT_W_S(d1, v1);
                    XVLDI(q0, 0b1001110000000); // broadcast 0x80000000 to all
                    XVLDI(d0, (0b10011 << 8) | 0x4f);
                    XVFCMP_S(q1, d0, v1, cULE); // get Nan,+overflow mark
                    XVBITSEL_V(v0, d1, q0, q1);
                } else {
                    XVFTINT_W_S(v0, v1);
                }
            } else {
                if (!BOX64ENV(dynarec_fastround)) {
                    d1 = fpu_get_scratch(dyn);
                    q0 = fpu_get_scratch(dyn);
                    q1 = fpu_get_scratch(dyn); // mask
                    d0 = fpu_get_scratch(dyn);
                    VFTINT_W_S(d1, v1);
                    VLDI(q0, 0b1001110000000); // broadcast 0x80000000 to all
                    VLDI(d0, (0b10011 << 8) | 0x4f);
                    VFCMP_S(q1, d0, v1, cULE); // get Nan,+overflow mark
                    VBITSEL_V(v0, d1, q0, q1);
                } else {
                    VFTINT_W_S(v0, v1);
                }
            }
            x87_restoreround(dyn, ninst, u8);
            break;
        case 0x5C:
            INST_NAME("VSUBPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            if (!BOX64ENV(dynarec_fastnan)) {
                d0 = fpu_get_scratch(dyn);
                d1 = fpu_get_scratch(dyn);
                VFCMPxy(D, d0, v1, v2, cUN);
            }
            VFSUBxy(D, v0, v1, v2);
            if (!BOX64ENV(dynarec_fastnan)) {
                VFCMPxy(D, d1, v0, v0, cUN);
                VANDN_Vxy(d0, d0, d1);
                VLDIxy(d1, (0b011 << 9) | 0b111111000);
                VSLLIxy(D, d1, d1, 48); // broadcast 0xfff8000000000000
                VBITSEL_Vxy(v0, v0, d1, d0);
            }
            break;
        case 0x5D:
            INST_NAME("VMINPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            if (BOX64ENV(dynarec_fastnan)) {
                VFMINxy(D, v0, v2, v1);
            } else {
                q0 = fpu_get_scratch(dyn);
                q1 = fpu_get_scratch(dyn);
                VFCMPxy(D, q0, v2, v1, cULE);
                VBITSEL_Vxy(v0, v1, v2, q0);
            }
            break;
        case 0x5E:
            INST_NAME("VDIVPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            if (!BOX64ENV(dynarec_fastnan)) {
                d0 = fpu_get_scratch(dyn);
                d1 = fpu_get_scratch(dyn);
                VFCMPxy(D, d0, v1, v2, cUN);
            }
            VFDIVxy(D, v0, v1, v2);
            if (!BOX64ENV(dynarec_fastnan)) {
                VFCMPxy(D, d1, v0, v0, cUN);
                VANDN_Vxy(d0, d0, d1);
                VLDIxy(d1, (0b011 << 9) | 0b111111000);
                VSLLIxy(D, d1, d1, 48); // broadcast 0xfff8000000000000
                VBITSEL_Vxy(v0, v0, d1, d0);
            }
            break;
        case 0x5F:
            INST_NAME("VMAXPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            if (BOX64ENV(dynarec_fastnan)) {
                VFMAXxy(D, v0, v2, v1);
            } else {
                q0 = fpu_get_scratch(dyn);
                q1 = fpu_get_scratch(dyn);
                VFCMPxy(D, q0, v2, v1, cLT);
                VBITSEL_Vxy(v0, v2, v1, q0);
            }
            break;
        case 0x60:
            INST_NAME("VPUNPCKLBW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VILVLxy(B, v0, v2, v1);
            break;
        case 0x61:
            INST_NAME("VPUNPCKLWD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VILVLxy(H, v0, v2, v1);
            break;
        case 0x62:
            INST_NAME("VPUNPCKLDQ Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VILVLxy(W, v0, v2, v1);
            break;
        case 0x63:
            INST_NAME("VPACKSSWB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            if (v1 == v2) {
                VSATxy(H, v0, v1, 7);
                VPICKEVxy(B, v0, v0, v0);
            } else {
                VSATxy(H, q0, v2, 7);
                VSATxy(H, v0, v1, 7);
                VPICKEVxy(B, v0, q0, v0);
            }
            break;
        case 0x64:
            INST_NAME("VPCMPGTB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VSLTxy(B, v0, v2, v1);
            break;
        case 0x65:
            INST_NAME("VPCMPGTW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VSLTxy(H, v0, v2, v1);
            break;
        case 0x66:
            INST_NAME("VPCMPGTD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VSLTxy(W, v0, v2, v1);
            break;
        case 0x67:
            INST_NAME("VPACKUSWB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            d0 = fpu_get_scratch(dyn);
            VLDIxy(q0, 0b0010011111111); // broadcast 0xff as 16-bit elements to all lanes
            if (v1 == v2) {
                VMAXIxy(H, d0, v1, 0);
                VMINxy(H, d0, d0, q0);
                VPICKEVxy(B, v0, d0, d0);
            } else {
                VMAXIxy(H, d0, v1, 0);
                VMAXIxy(H, q1, v2, 0);
                VMINxy(H, d0, d0, q0);
                VMINxy(H, q1, q1, q0);
                VPICKEVxy(B, v0, q1, d0);
            }
            break;
        case 0x68:
            INST_NAME("VPUNPCKHBW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VILVHxy(B, v0, v2, v1);
            break;
        case 0x69:
            INST_NAME("VPUNPCKHWD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VILVHxy(H, v0, v2, v1);
            break;
        case 0x6A:
            INST_NAME("VPUNPCKHDQ Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VILVHxy(W, v0, v2, v1);
            break;
        case 0x6B:
            INST_NAME("VPACKSSDW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            d0 = fpu_get_scratch(dyn);
            if (v1 == v2) {
                VSATxy(W, d0, v1, 15);
                VPICKEVxy(H, v0, d0, d0);
            } else {
                VSATxy(W, d0, v1, 15);
                VSATxy(W, q0, v2, 15);
                VPICKEVxy(H, v0, q0, d0);
            }
            break;
        case 0x6C:
            INST_NAME("VPUNPCKLQDQ Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VILVLxy(D, v0, v2, v1);
            break;
        case 0x6D:
            INST_NAME("VPUNPCKHQDQ Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VILVHxy(D, v0, v2, v1);
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
        case 0x70:
            INST_NAME("VPSHUFD Gx, Ex, Ib");
            nextop = F8;
            GETGY_empty_EY_xy(v0, v1, 1);
            u8 = F8;
            VSHUF4Ixy(W, v0, v1, u8);
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
        case 0x74:
            INST_NAME("VPCMPEQB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VSEQxy(B, v0, v1, v2);
            break;
        case 0x75:
            INST_NAME("VPCMPEQW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VSEQxy(H, v0, v1, v2);
            break;
        case 0x76:
            INST_NAME("VPCMPEQD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VSEQxy(W, v0, v1, v2);
            break;
        case 0x7C:
            INST_NAME("VHADDPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            VPICKEVxy(D, q0, v2, v1);
            VPICKODxy(D, v0, v2, v1);
            if (!BOX64ENV(dynarec_fastnan)) {
                d0 = fpu_get_scratch(dyn);
                d1 = fpu_get_scratch(dyn);
                VFCMPxy(D, d0, q0, v0, cUN);
            }
            VFADDxy(D, v0, q0, v0);
            if (!BOX64ENV(dynarec_fastnan)) {
                VFCMPxy(D, d1, v0, v0, cUN);
                VANDN_Vxy(d0, d0, d1);
                VLDIxy(d1, (0b011 << 9) | 0b111111000);
                VSLLIxy(D, d1, d1, 48); // broadcast 0xfff8000000000000
                VBITSEL_Vxy(v0, v0, d1, d0);
            }
            break;
        case 0x7D:
            INST_NAME("VHSUBPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            VPICKEVxy(D, q0, v2, v1);
            VPICKODxy(D, v0, v2, v1);
            if (!BOX64ENV(dynarec_fastnan)) {
                d0 = fpu_get_scratch(dyn);
                d1 = fpu_get_scratch(dyn);
                VFCMPxy(D, d0, q0, v0, cUN);
            }
            VFSUBxy(D, v0, q0, v0);
            if (!BOX64ENV(dynarec_fastnan)) {
                VFCMPxy(D, d1, v0, v0, cUN);
                VANDN_Vxy(d0, d0, d1);
                VLDIxy(d1, (0b011 << 9) | 0b111111000);
                VSLLIxy(D, d1, d1, 48); // broadcast 0xfff8000000000000
                VBITSEL_Vxy(v0, v0, d1, d0);
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
                    VPICKVE2GR_WU(ed, v0, 0);
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
        case 0xC2:
            INST_NAME("VCMPPD Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 1);
            u8 = F8;
            switch (u8 & 0xf) {
                case 0x00: VFCMPxy(D, v0, v1, v2, cEQ); break;  // Equal, not unordered
                case 0x01: VFCMPxy(D, v0, v1, v2, cLT); break;  // Less than
                case 0x02: VFCMPxy(D, v0, v1, v2, cLE); break;  // Less or equal
                case 0x03: VFCMPxy(D, v0, v1, v2, cUN); break;  // unordered
                case 0x04: VFCMPxy(D, v0, v1, v2, cUNE); break; // Not Equal (or unordered on ARM, not on X86...)
                case 0x05: VFCMPxy(D, v0, v2, v1, cULE); break; // Greater or equal or unordered
                case 0x06: VFCMPxy(D, v0, v2, v1, cULT); break; // Greater or unordered
                case 0x07: VFCMPxy(D, v0, v1, v2, cOR); break;  // Greater or unordered
                case 0x08: VFCMPxy(D, v0, v1, v2, cUEQ); break; // Equal, or unordered
                case 0x09: VFCMPxy(D, v0, v1, v2, cULT); break; // Less than or unordered
                case 0x0a: VFCMPxy(D, v0, v1, v2, cULE); break; // Less or equal or unordered
                case 0x0b: XVXOR_V(v0, v0, v0); break;          // false
                case 0x0c: VFCMPxy(D, v0, v1, v2, cNE); break;  // Not Eual, ordered
                case 0x0d: VFCMPxy(D, v0, v2, v1, cLE); break;  // Greater or Equal ordered
                case 0x0e: VFCMPxy(D, v0, v2, v1, cLT); break;  // Greater ordered
                case 0x0f: VSEQxy(B, v0, v1, v1); break;        // true
            }
            break;
        case 0xC4:
            INST_NAME("VPINSRW Gx, Vx, ED, Ib");
            nextop = F8;
            GETEWW(0, x5, 1);
            GETVYx(v1, 0);
            GETGYx_empty(v0);
            u8 = F8;
            if(v0 != v1) VOR_V(v0, v1, v1);
            VINSGR2VR_H(v0, ed, (u8 & 0x7));
            break;
        case 0xC5:
            INST_NAME("VPEXTRW Gd, Ex, Ib");
            nextop = F8;
            GETGD;
            if (MODREG) {
                GETEYx(v0, 0, 1);
                u8 = (F8) & 7;
                VPICKVE2GR_HU(gd, v0, u8);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x4, &fixedaddress, rex, NULL, 0, 1);
                u8 = (F8) & 7;
                LD_HU(gd, wback, (u8 << 1));
            }
            break;
        case 0xC6:
            INST_NAME("VSHUFPD Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 1);
            u8 = F8 & 0xf;
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            if (vex.l) {
                if ((u8 >> 2) == (u8 & 0b11)) {
                    XVOR_V(d0, v1, v1);
                    XVSHUF4I_D(d0, v2, 0x8 | (u8 & 1) | ((u8 & 2) << 1));
                    XVOR_V(v0, d0, d0);
                } else {
                    XVOR_V(d0, v1, v1);
                    XVOR_V(d1, v1, v1);
                    XVSHUF4I_D(d0, v2, 0x8 | (u8 & 1) | ((u8 & 2) << 1));
                    XVSHUF4I_D(d1, v2, 0x8 | ((u8 & 4) >> 2) | ((u8 & 8) >> 1));
                    XVPERMI_Q(d1, d0, XVPERMI_IMM_4_0(3, 0));
                    XVOR_V(v0, d1, d1);
                }
            } else {
                VOR_V(d0, v1, v1);
                VSHUF4I_D(d0, v2, 0x8 | (u8 & 1) | ((u8 & 2) << 1));
                VOR_V(v0, d0, d0);
            }
            break;
        case 0xD0:
            INST_NAME("VADDSUBPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            if (!BOX64ENV(dynarec_fastnan)) {
                d0 = fpu_get_scratch(dyn);
                d1 = fpu_get_scratch(dyn);
                VFCMPxy(D, d0, v1, v2, cUN);
            }
            q0 = fpu_get_scratch(dyn);
            VFSUBxy(D, q0, v1, v2);
            VFADDxy(D, v0, v1, v2);
            VEXTRINSxy(D, v0, q0, 0);
            if (!BOX64ENV(dynarec_fastnan)) {
                VFCMPxy(D, d1, v0, v0, cUN);
                VANDN_Vxy(d0, d0, d1);
                VLDIxy(d1, (0b011 << 9) | 0b111111000);
                VSLLIxy(D, d1, d1, 48); // broadcast 0xfff8000000000000
                VBITSEL_Vxy(v0, v0, d1, d0);
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
                XVPICKVE2GR_DU(gd, d1, 0);
                XVPICKVE2GR_DU(x4, d1, 2);
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
        case 0xE6:
            INST_NAME("VCVTTPD2DQ Gx, Ex");
            nextop = F8;
            GETEYxy(v1, 0, 0);
            GETGYx_empty(v0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            if(vex.l){
                XVXOR_V(d0, d0, d0);
                XVFTINTRZ_W_D(d1, d0, v1);       // v0 [lo0, lo1, --, --, hi0, hi1, --, -- ]
                if (!BOX64ENV(dynarec_fastround)) {
                    q0 = fpu_get_scratch(dyn);
                    q1 = fpu_get_scratch(dyn);
                    XVLDI(q0, 0b1001110000000); // broadcast 0x80000000 to all
                    /*
                        VCVTTPD2DQ has default rounding mode RZ
                        so we could combine +-NAN +overflow to xvfcmp.cule 0x41e0000000000000
                    */
                    LU52I_D(x5, xZR, 0x41e);
                    XVREPLGR2VR_D(q1, x5);       
                    XVFCMP_D(d0, q1, v1, cULE); // get Nan mask
                    XVSRLNI_W_D(d0, d0, 0);
                    XVBITSEL_V(v0, d1, q0, d0);
                }
                XVPERMI_D(v0, v0, 0b11011000);
            }else{
                VFTINTRZ_W_D(d0, v1, v1);
                if (!BOX64ENV(dynarec_fastround)) {
                    q0 = fpu_get_scratch(dyn);
                    q1 = fpu_get_scratch(dyn);
                    XVLDI(q0, 0b1001110000000); // broadcast 0x80000000 to all
                    LU52I_D(x5, xZR, 0x41e);
                    XVREPLGR2VR_D(q1, x5);       
                    XVFCMP_D(q1, q1, v1, cULE); // get Nan mask
                    VSHUF4I_W(q1, q1, 0b11011000);
                    VBITSEL_V(d0, d0, q0, q1);
                }
                XVPICKVE_D(v0, d0, 0);
                YMM_UNMARK_UPPER_ZERO(v0);
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
            INST_NAME("VPMULUDQ Gx, Vx, Ex");
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
            GETGYx(v0, 0);
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
