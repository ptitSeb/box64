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

uintptr_t dynarec64_AVX_F2_0F(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
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
    uint64_t tmp64u, u64;
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
            INST_NAME("VMOVSD Gx, [Vx,] Ex");
            nextop = F8;
            if (MODREG) {
                GETVYx(q1, 0);
                GETEYSD(q2, 0, 0);
                GETGYx_empty(q0);
                if (q0 != q2) VEXTRINS_D(q0, q2, 0b00000);
                if (q0 != q1) VEXTRINS_D(q0, q1, 0b10001);
            } else {
                GETEYSD(q2, 0, 0);
                GETGYx_empty(q0);
                XVPICKVE_D(q0, q2, 0);
                YMM_UNMARK_UPPER_ZERO(q0);
            }
            break;
        case 0x11:
            INST_NAME("VMOVSD Ex, [Vx,] Gx");
            nextop = F8;
            GETGYx(q2, 0);
            if (MODREG) {
                GETVYx(q1, 0);
                GETEYSD(q0, 1, 0);
                if (q0 != q2) VEXTRINS_D(q0, q2, 0b00000);
                if (q0 != q1) VEXTRINS_D(q0, q1, 0b10001);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                FST_D(q2, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0x12:
            INST_NAME("VMOVDDUP Gx, Ex");
            nextop = F8;
            if (MODREG) {
                GETGY_empty_EY_xy(q0, q1, 0);
            } else {
                GETGYxy_empty(q0);
                q1 = fpu_get_scratch(dyn);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x4, x5, &fixedaddress, rex, NULL, 0, 0);
                if (vex.l) {
                    XVLD(q1, ed, 0);
                } else {
                    VLDREPL_D(q0, ed, 0);
                }
            }
            if (vex.l) {
                XVSHUF4I_D(q0, q1, 0b1010);
            } else if (MODREG) {
                VREPLVE_D(q0, q1, 0);
            }
            break;
        case 0x2A:
            INST_NAME("VCVTSI2SD Gx, Vx, Ed");
            nextop = F8;
            GETED(0);
            GETVYx(v1, 0);
            GETGYx_empty(v0);
            d1 = fpu_get_scratch(dyn);
            if (!BOX64ENV(dynarec_fastround)) {
                u8 = sse_setround(dyn, ninst, x2, x3);
            }
            if (rex.w) {
                MOVGR2FR_D(d1, ed);
                FFINT_D_L(d1, d1);
            } else {
                MOVGR2FR_W(d1, ed);
                FFINT_D_W(d1, d1);
            }
            if (!BOX64ENV(dynarec_fastround)) {
                x87_restoreround(dyn, ninst, u8);
            }
            if(v0 != v1) VOR_V(v0, v1, v1);
            VEXTRINS_D(v0, d1, 0);
            break;
        case 0x2C:
            INST_NAME("VCVTTSD2SI Gd, Ex");
            nextop = F8;
            GETGD;
            GETEYSD(q0, 0, 0);
            if (!BOX64ENV(dynarec_fastround)) {
                MOVGR2FCSR(FCSR2, xZR); // reset all bits
            }
            d1 = fpu_get_scratch(dyn);
            if (rex.w) {
                FTINTRZ_L_D(d1, q0);
                MOVFR2GR_D(gd, d1);
            } else {
                FTINTRZ_W_D(d1, q0);
                MOVFR2GR_S(gd, d1);
                ZEROUP(gd);
            }
            if (!BOX64ENV(dynarec_fastround)) {
                MOVFCSR2GR(x5, FCSR2); // get back FPSR to check
                MOV32w(x3, (1 << FR_V) | (1 << FR_O));
                AND(x5, x5, x3);
                CBZ_NEXT(x5);
                if (rex.w) {
                    MOV64x(gd, 0x8000000000000000LL);
                } else {
                    MOV32w(gd, 0x80000000);
                }
            }
            break;
        case 0x2D:
            INST_NAME("VCVTSD2SI Gd, Ex");
            nextop = F8;
            GETGD;
            GETEYSD(q0, 0, 0);
            if (!BOX64ENV(dynarec_fastround)) {
                MOVGR2FCSR(FCSR2, xZR); // reset all bits
            }
            d1 = fpu_get_scratch(dyn);
            u8 = sse_setround(dyn, ninst, x2, x3);
            if (rex.w) {
                FTINT_L_D(d1, q0);
                MOVFR2GR_D(gd, d1);
            } else {
                FTINT_W_D(d1, q0);
                MOVFR2GR_S(gd, d1);
                ZEROUP(gd);
            }
            x87_restoreround(dyn, ninst, u8);
            if (!BOX64ENV(dynarec_fastround)) {
                MOVFCSR2GR(x5, FCSR2); // get back FPSR to check
                MOV32w(x3, (1 << FR_V) | (1 << FR_O));
                AND(x5, x5, x3);
                CBZ_NEXT(x5);
                if (rex.w) {
                    MOV64x(gd, 0x8000000000000000LL);
                } else {
                    MOV32w(gd, 0x80000000);
                }
            }
            break;
        case 0x51:
            INST_NAME("VSQRTSD Gx, Vx, Ex");
            nextop = F8;
            GETVYx(v1, 0);
            GETEYSD(v2, 0, 0);
            GETGYx_empty(v0);
            d1 = fpu_get_scratch(dyn);
            FSQRT_D(d1, v2);
            if (!BOX64ENV(dynarec_fastnan)) {
                d0 = fpu_get_scratch(dyn);
                VXOR_V(d0, d0, d0);
                FCMP_D(fcc0, v2, d0, cLT);
                BCEQZ(fcc0, 4 + 4);
                FNEG_D(d1, d1);
            }
            if (v0 != v1) VOR_V(v0, v1, v1);
            VEXTRINS_D(v0, d1, 0);
            break;
        case 0x58:
            INST_NAME("VADDSD Gx, Vx, Ex");
            nextop = F8;
            GETVYx(v1, 0);
            GETEYSD(v2, 0, 0);
            GETGYx_empty(v0);
            d0 = fpu_get_scratch(dyn);
            FADD_D(d0, v1, v2);
            if (!BOX64ENV(dynarec_fastnan)) {
                FCMP_D(fcc0, v1, v2, cUN);
                BCNEZ_MARK(fcc0);
                FCMP_D(fcc1, d0, d0, cOR);
                BCNEZ_MARK(fcc1);
                FNEG_D(d0, d0);
            }
            MARK;
            if (v0 != v1) VOR_V(v0, v1, v1);
            VEXTRINS_D(v0, d0, 0);
            break;
        case 0x59:
            INST_NAME("VMULSD Gx, Vx, Ex");
            nextop = F8;
            GETVYx(v1, 0);
            GETEYSD(v2, 0, 0);
            GETGYx_empty(v0);
            d0 = fpu_get_scratch(dyn);
            FMUL_D(d0, v1, v2);
            if (!BOX64ENV(dynarec_fastnan)) {
                FCMP_D(fcc0, v1, v2, cUN);
                BCNEZ_MARK(fcc0);
                FCMP_D(fcc1, d0, d0, cOR);
                BCNEZ_MARK(fcc1);
                FNEG_D(d0, d0);
            }
            MARK;
            if (v0 != v1) VOR_V(v0, v1, v1);
            VEXTRINS_D(v0, d0, 0);
            break;
        case 0x5A:
            INST_NAME("VCVTSD2SS Gx, Vx, Ex");
            nextop = F8;
            GETEYSD(v2, 0, 0);
            GETVYx(v1, 0);
            GETGYx_empty(v0);
            d0 = fpu_get_scratch(dyn);
            FCVT_S_D(d0, v2);
            if(v0 != v1) VOR_V(v0, v1, v1);
            XVINSVE0_W(v0, d0, 0);
            break;
        case 0x5C:
            INST_NAME("VSUBSD Gx, Vx, Ex");
            nextop = F8;
            GETVYx(v1, 0);
            GETEYSD(v2, 0, 0);
            GETGYx_empty(v0);
            d0 = fpu_get_scratch(dyn);
            FSUB_D(d0, v1, v2);
            if (!BOX64ENV(dynarec_fastnan)) {
                FCMP_D(fcc0, v1, v2, cUN);
                BCNEZ_MARK(fcc0);
                FCMP_D(fcc1, d0, d0, cOR);
                BCNEZ_MARK(fcc1);
                FNEG_D(d0, d0);
            }
            MARK;
            if (v0 != v1) VOR_V(v0, v1, v1);
            VEXTRINS_D(v0, d0, 0);
            break;
        case 0x5D:
            INST_NAME("VMINSD Gx, Vx, Ex");
            nextop = F8;
            GETVYx(v1, 0);
            GETEYSD(v2, 0, 0);
            GETGYx_empty(v0);
            q0 = fpu_get_scratch(dyn);
            if (BOX64ENV(dynarec_fastnan)) {
                FMIN_D(q0, v1, v2);
            } else {
                FCMP_D(fcc0, v2, v1, cULE);
                FSEL(q0, v1, v2, fcc0);
            }
            if (v0 != v1) VOR_V(v0, v1, v1);
            VEXTRINS_D(v0, q0, 0);
            break;
        case 0x5E:
            INST_NAME("VDIVSD Gx, Vx, Ex");
            nextop = F8;
            GETVYx(v1, 0);
            GETEYSD(v2, 0, 0);
            GETGYx_empty(v0);
            d0 = fpu_get_scratch(dyn);
            FDIV_D(d0, v1, v2);
            if (!BOX64ENV(dynarec_fastnan)) {
                FCMP_D(fcc0, v1, v2, cUN);
                BCNEZ_MARK(fcc0);
                FCMP_D(fcc1, d0, d0, cOR);
                BCNEZ_MARK(fcc1);
                FNEG_D(d0, d0);
            }
            MARK;
            if (v0 != v1) VOR_V(v0, v1, v1);
            VEXTRINS_D(v0, d0, 0);
            break;
        case 0x5F:
            INST_NAME("VMAXSD Gx, Vx, Ex");
            nextop = F8;
            GETVYx(v1, 0);
            GETEYSD(v2, 0, 0);
            GETGYx_empty(v0);
            q0 = fpu_get_scratch(dyn);
            if (BOX64ENV(dynarec_fastnan)) {
                FMAX_D(q0, v1, v2);
            } else {
                FCMP_D(fcc0, v2, v1, cLT);
                FSEL(q0, v2, v1, fcc0);
            }
            if (v0 != v1) VOR_V(v0, v1, v1);
            VEXTRINS_D(v0, q0, 0);
            break;
        case 0x70:
            INST_NAME("VPSHUFLW Gx, Ex, Ib");
            nextop = F8;
            GETGY_empty_EY_xy(v0, v1, 1);
            u8 = F8;
            d0 = fpu_get_scratch(dyn);
            if (v0 != v1) {
                VSHUF4Ixy(H, v0, v1, u8);
                VEXTRINSxy(D, v0, v1, VEXTRINS_IMM_4_0(1, 1));
            } else {
                VSHUF4Ixy(H, d0, v1, u8);
                VEXTRINSxy(D, v0, d0, VEXTRINS_IMM_4_0(0, 0));
            }
            break;
        case 0x7C:
            INST_NAME("VHADDPS Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            VPICKEVxy(W, q0, v2, v1);
            VPICKODxy(W, v0, v2, v1);
            if (!BOX64ENV(dynarec_fastnan)) {
                d0 = fpu_get_scratch(dyn);
                d1 = fpu_get_scratch(dyn);
                VFCMPxy(S, d0, q0, v0, cUN);
            }
            VFADDxy(S, v0, q0, v0);
            if (!BOX64ENV(dynarec_fastnan)) {
                VFCMPxy(S, d1, v0, v0, cUN);
                VANDN_Vxy(d0, d0, d1);
                VLDIxy(d1, (0b010 << 9) | 0b1111111100);
                VSLLIxy(W, d1, d1, 20); // broadcast 0xFFC00000
                VBITSEL_Vxy(v0, v0, d1, d0);
            }
            break;
        case 0x7D:
            INST_NAME("VHSUBPS Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            VPICKEVxy(W, q0, v2, v1);
            VPICKODxy(W, v0, v2, v1);
            if (!BOX64ENV(dynarec_fastnan)) {
                d0 = fpu_get_scratch(dyn);
                d1 = fpu_get_scratch(dyn);
                VFCMPxy(S, d0, q0, v0, cUN);
            }
            VFSUBxy(S, v0, q0, v0);
            if (!BOX64ENV(dynarec_fastnan)) {
                VFCMPxy(S, d1, v0, v0, cUN);
                VANDN_Vxy(d0, d0, d1);
                VLDIxy(d1, (0b010 << 9) | 0b1111111100);
                VSLLIxy(W, d1, d1, 20); // broadcast 0xFFC00000
                VBITSEL_Vxy(v0, v0, d1, d0);
            }
            break;
        case 0xC2:
            INST_NAME("VCMPSD Gx, Vx, Ex, Ib");
            nextop = F8;
            GETVYx(v1, 0);
            GETEYSD(v2, 0, 1);
            GETGYx(v0, 1);
            q0 = fpu_get_scratch(dyn);
            u8 = F8;
            switch (u8 & 0xf) {
                case 0x00: VFCMP_D(q0, v1, v2, cEQ); break;  // Equal, not unordered
                case 0x01: VFCMP_D(q0, v1, v2, cLT); break;  // Less than
                case 0x02: VFCMP_D(q0, v1, v2, cLE); break;  // Less or equal
                case 0x03: VFCMP_D(q0, v1, v2, cUN); break;  // unordered
                case 0x04: VFCMP_D(q0, v1, v2, cUNE); break; // Not Equal (or unordered on ARM, not on X86...)
                case 0x05: VFCMP_D(q0, v2, v1, cULE); break; // Greater or equal or unordered
                case 0x06: VFCMP_D(q0, v2, v1, cULT); break; // Greater or unordered
                case 0x07: VFCMP_D(q0, v1, v2, cOR); break;  // Greater or unordered
                case 0x08: VFCMP_D(q0, v1, v2, cUEQ); break; // Equal, or unordered
                case 0x09: VFCMP_D(q0, v1, v2, cULT); break; // Less than or unordered
                case 0x0a: VFCMP_D(q0, v1, v2, cULE); break; // Less or equal or unordered
                case 0x0b: VXOR_V(q0, q0, q0); break;        // false
                case 0x0c: VFCMP_D(q0, v1, v2, cNE); break;  // Not Eual, ordered
                case 0x0d: VFCMP_D(q0, v2, v1, cLE); break;  // Greater or Equal ordered
                case 0x0e: VFCMP_D(q0, v2, v1, cLT); break;  // Greater ordered
                case 0x0f: VSEQ_B(q0, v1, v1); break;        // true
            }
            XVPERMI_Q(v0, v1, XVPERMI_IMM_4_0(3, 0));
            XVINSVE0_D(v0, q0, 0);
            break;
        case 0xD0:
            INST_NAME("VADDSUBPS Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            if (!BOX64ENV(dynarec_fastnan)) {
                d0 = fpu_get_scratch(dyn);
                d1 = fpu_get_scratch(dyn);
                VFCMPxy(S, d0, v1, v2, cUN);
            }
            q0 = fpu_get_scratch(dyn);
            VFSUBxy(S, q0, v1, v2);
            VFADDxy(S, v0, v1, v2);
            VEXTRINSxy(W, v0, q0, VEXTRINS_IMM_4_0(0, 0));
            VEXTRINSxy(W, v0, q0, VEXTRINS_IMM_4_0(2, 2));
            if (!BOX64ENV(dynarec_fastnan)) {
                VFCMPxy(S, d1, v0, v0, cUN);
                VANDN_Vxy(d0, d0, d1);
                VLDIxy(d1, (0b010 << 9) | 0b1111111100);
                VSLLIxy(W, d1, d1, 20); // broadcast 0xFFC00000
                VBITSEL_Vxy(v0, v0, d1, d0);
            }
            break;
        case 0xE6:
            INST_NAME("VCVTPD2DQ Gx, Ex");
            nextop = F8;
            GETEYxy(v1, 0, 0);
            GETGYx_empty(v0);
            u8 = sse_setround(dyn, ninst, x1, x2);
            d0 = fpu_get_scratch(dyn);
            if (vex.l) {
                XVXOR_V(d0, d0, d0);
                XVFTINT_W_D(v0, d0, v1);       // v0 [lo0, lo1, --, --, hi0, hi1, --, -- ]
                if (!BOX64ENV(dynarec_fastround)) {
                    q0 = fpu_get_scratch(dyn);
                    q1 = fpu_get_scratch(dyn);
                    q2 = fpu_get_scratch(dyn);
                    d1 = fpu_get_scratch(dyn);
                    XVFTINT_L_D(q2, v1);
                    XVLDI(q0, 0b1001110000000); // broadcast 0x80000000 to all
                    MOV32w(x5, 0x7FFFFFFF);
                    BSTRPICK_D(x5, x5, 31, 0);
                    XVREPLGR2VR_D(q1, x5);       
                    XVFCMP_D(d0, v1, v1, cUN); // get Nan mask
                    XVSLT_D(d1, q1, q2); // get +inf mask
                    XVOR_V(d0, d1, d0);
                    XVSRLNI_W_D(d0, d0, 0); // [A,B,C,D] => [a,b,--,--,c,d,--,--]
                    XVBITSEL_V(v0, v0, q0, d0);
                }
                XVPERMI_D(v0, v0, 0b11011000);
            } else {
                VFTINT_W_D(d0, v1, v1);
                if (!BOX64ENV(dynarec_fastround)) {
                    q0 = fpu_get_scratch(dyn);
                    q1 = fpu_get_scratch(dyn);
                    q2 = fpu_get_scratch(dyn);
                    d1 = fpu_get_scratch(dyn);
                    VFTINT_L_D(d1, v1);
                    VLDI(q0, 0b1001110000000); // broadcast 0x80000000 to all
                    MOV32w(x5, 0x7FFFFFFF);
                    BSTRPICK_D(x5, x5, 31, 0);
                    VREPLGR2VR_D(q1, x5);       
                    VSLT_D(q1, q1, d1); // get +inf mask
                    VFCMP_D(q2, v1, v1, cUN); // get Nan mask
                    VOR_V(q1, q1, q2);
                    VSHUF4I_W(q1, q1, 0b11011000);
                    VBITSEL_V(d0, d0, q0, q1);
                }
                XVPICKVE_D(v0, d0, 0);
                YMM_UNMARK_UPPER_ZERO(v0);
            }
            x87_restoreround(dyn, ninst, u8);
            break;
        case 0xF0:
            INST_NAME("VLDDQU Gx, Ex");
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
        default:
            DEFAULT;
    }
    return addr;
}
