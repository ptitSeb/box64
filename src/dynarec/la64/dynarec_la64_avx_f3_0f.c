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
                if (q0 != q2) VEXTRINS_W(q0, q2, 0b00000);
                if (q0 != q1) {
                    VEXTRINS_D(q0, q1, 0b10001);
                    VEXTRINS_W(q0, q1, 0b10001);
                }
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
                GETVYx(q1, 0);
                GETEYSD(q0, 1, 0);
                if (q0 != q2) VEXTRINS_W(q0, q2, 0b00000);
                if (q0 != q1) {
                    VEXTRINS_D(q0, q1, 0b10001);
                    VEXTRINS_W(q0, q1, 0b10001);
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
        case 0x2A:
            INST_NAME("VCVTSI2SS Gx, Vx, Ed");
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
                FFINT_S_L(d1, d1);
            } else {
                MOVGR2FR_W(d1, ed);
                FFINT_S_W(d1, d1);
            }
            if (!BOX64ENV(dynarec_fastround)) {
                x87_restoreround(dyn, ninst, u8);
            }
            if(v0 != v1) VOR_V(v0, v1, v1);
            VEXTRINS_W(v0, d1, 0);
            break;
        case 0x2C:
            INST_NAME("VCVTTSS2SI Gd, Ex");
            nextop = F8;
            GETGD;
            GETEYSS(d0, 0, 0);
            if (!BOX64ENV(dynarec_fastround)) {
                MOVGR2FCSR(FCSR2, xZR); // reset all bits
            }
            u8 = sse_setround(dyn, ninst, x5, x6);
            d1 = fpu_get_scratch(dyn);
            if (rex.w) {
                FTINTRZ_L_S(d1, d0);
                MOVFR2GR_D(gd, d1);
            } else {
                FTINTRZ_W_S(d1, d0);
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
        case 0x2D:
            INST_NAME("VCVTSS2SI Gd, Ex");
            nextop = F8;
            GETGD;
            GETEYSS(d0, 0, 0);
            if (!BOX64ENV(dynarec_fastround)) {
                MOVGR2FCSR(FCSR2, xZR); // reset all bits
            }
            u8 = sse_setround(dyn, ninst, x5, x6);
            d1 = fpu_get_scratch(dyn);
            if (rex.w) {
                FTINT_L_S(d1, d0);
                MOVFR2GR_D(gd, d1);
            } else {
                FTINT_W_S(d1, d0);
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
            INST_NAME("VSQRTSS Gx, Vx, Ex");
            nextop = F8;
            GETVYx(v1, 0);
            GETEYSS(v2, 0, 0);
            GETGYx_empty(v0);
            d1 = fpu_get_scratch(dyn);
            FSQRT_S(d1, v2);
            if (!BOX64ENV(dynarec_fastnan)) {
                d0 = fpu_get_scratch(dyn);
                VXOR_V(d0, d0, d0);
                FCMP_S(fcc0, v2, d0, cLT);
                BCEQZ(fcc0, 4 + 4);
                FNEG_S(d1, d1);
            }
            if (v0 != v1) VOR_V(v0, v1, v1);
            VEXTRINS_W(v0, d1, 0);
            break;
        case 0x52:
            INST_NAME("VRSQRTSS Gx, Vx, Ex");
            nextop = F8;
            GETVYx(v1, 0);
            GETEYSS(v2, 0, 0);
            GETGYx_empty(v0);
            d0 = fpu_get_scratch(dyn);
            if (cpuext.frecipe) {
                FRSQRTE_S(d0, v1);
            } else {
                FRSQRT_S(d0, v1);
            }
            if (v0 != v1) VOR_V(v0, v1, v1);
            VEXTRINS_W(v0, d0, 0);
            break;
        case 0x53:
            INST_NAME("VRCPSS Gx, Vx, Ex");
            nextop = F8;
            GETVYx(v1, 0);
            GETEYSS(v2, 0, 0);
            GETGYx_empty(v0);
            d0 = fpu_get_scratch(dyn);
            if (cpuext.frecipe) {
                FRECIPE_S(d0, v1);
            } else {
                FRECIP_S(d0, v1);
            }
            if (v0 != v1) VOR_V(v0, v1, v1);
            VEXTRINS_W(v0, d0, 0);
            break;
        case 0x58:
            INST_NAME("VADDSS Gx, Vx, Ex");
            nextop = F8;
            GETVYx(v1, 0);
            GETEYSS(v2, 0, 0);
            GETGYx_empty(v0);
            d0 = fpu_get_scratch(dyn);
            FADD_S(d0, v1, v2);
            if (!BOX64ENV(dynarec_fastnan)) {
                FCMP_S(fcc0, v1, v2, cUN);
                BCNEZ_MARK(fcc0);
                FCMP_S(fcc1, d0, d0, cOR);
                BCNEZ_MARK(fcc1);
                FNEG_S(d0, d0);
            }
            MARK;
            if (v0 != v1) VOR_V(v0, v1, v1);
            VEXTRINS_W(v0, d0, 0);
            break;
        case 0x59:
            INST_NAME("VMULSS Gx, Vx, Ex");
            nextop = F8;
            GETVYx(v1, 0);
            GETEYSS(v2, 0, 0);
            GETGYx_empty(v0);
            d0 = fpu_get_scratch(dyn);
            FMUL_S(d0, v1, v2);
            if (!BOX64ENV(dynarec_fastnan)) {
                FCMP_S(fcc0, v1, v2, cUN);
                BCNEZ_MARK(fcc0);
                FCMP_S(fcc1, d0, d0, cOR);
                BCNEZ_MARK(fcc1);
                FNEG_S(d0, d0);
            }
            MARK;
            if (v0 != v1) VOR_V(v0, v1, v1);
            VEXTRINS_W(v0, d0, 0);
            break;
        case 0x5A:
            INST_NAME("VCVTSS2SD Gx, Vx, Ex");
            nextop = F8;
            GETVYx(v1, 0);
            GETEYSS(v2, 0, 0);
            GETGYx_empty(v0);
            d1 = fpu_get_scratch(dyn);
            FCVT_D_S(d1, v2);
            if(v0 != v1) VOR_V(v0, v1, v1);
            VEXTRINS_D(v0, d1, 0);
            break;
        case 0x5B:
            INST_NAME("VCVTTPS2DQ Gx, Ex");
            nextop = F8;
            GETGY_empty_EY_xy(v0, v1, 0);
            if (vex.l) {
                if (!BOX64ENV(dynarec_fastround)) {
                    d1 = fpu_get_scratch(dyn);
                    q0 = fpu_get_scratch(dyn);
                    q1 = fpu_get_scratch(dyn); // mask
                    d0 = fpu_get_scratch(dyn);
                    XVFTINTRZ_W_S(d1, v1);
                    XVLDI(q0, 0b1001110000000); // broadcast 0x80000000 to all
                    XVLDI(d0, (0b10011 << 8) | 0x4f);
                    XVFCMP_S(q1, d0, v1, cULE); // get Nan,+overflow mark
                    XVBITSEL_V(v0, d1, q0, q1);
                } else {
                    XVFTINTRZ_W_S(v0, v1);
                }
            } else {
                if (!BOX64ENV(dynarec_fastround)) {
                    d1 = fpu_get_scratch(dyn);
                    q0 = fpu_get_scratch(dyn);
                    q1 = fpu_get_scratch(dyn); // mask
                    d0 = fpu_get_scratch(dyn);
                    VFTINTRZ_W_S(d1, v1);
                    VLDI(q0, 0b1001110000000); // broadcast 0x80000000 to all
                    VLDI(d0, (0b10011 << 8) | 0x4f);
                    VFCMP_S(q1, d0, v1, cULE); // get Nan,+overflow mark
                    VBITSEL_V(v0, d1, q0, q1);
                } else {
                    VFTINTRZ_W_S(v0, v1);
                }
            }
            break;
        case 0x5C:
            INST_NAME("VSUBSS Gx, Vx, Ex");
            nextop = F8;
            GETVYx(v1, 0);
            GETEYSS(v2, 0, 0);
            GETGYx_empty(v0);
            d0 = fpu_get_scratch(dyn);
            FSUB_S(d0, v1, v2);
            if (!BOX64ENV(dynarec_fastnan)) {
                FCMP_S(fcc0, v1, v2, cUN);
                BCNEZ_MARK(fcc0);
                FCMP_S(fcc1, d0, d0, cOR);
                BCNEZ_MARK(fcc1);
                FNEG_S(d0, d0);
            }
            MARK;
            if (v0 != v1) VOR_V(v0, v1, v1);
            VEXTRINS_W(v0, d0, 0);
            break;
        case 0x5D:
            INST_NAME("VMINSS Gx, Vx, Ex");
            nextop = F8;
            GETVYx(v1, 0);
            GETEYSS(v2, 0, 0);
            GETGYx_empty(v0);
            q0 = fpu_get_scratch(dyn);
            if (BOX64ENV(dynarec_fastnan)) {
                FMIN_S(q0, v1, v2);
            } else {
                FCMP_S(fcc0, v2, v1, cULE);
                FSEL(q0, v1, v2, fcc0);
            }
            if (v0 != v1) VOR_V(v0, v1, v1);
            VEXTRINS_W(v0, q0, 0);
            break;
        case 0x5E:
            INST_NAME("VDIVSS Gx, Vx, Ex");
            nextop = F8;
            GETVYx(v1, 0);
            GETEYSS(v2, 0, 0);
            GETGYx_empty(v0);
            d0 = fpu_get_scratch(dyn);
            FDIV_S(d0, v1, v2);
            if (!BOX64ENV(dynarec_fastnan)) {
                FCMP_S(fcc0, v1, v2, cUN);
                BCNEZ_MARK(fcc0);
                FCMP_S(fcc1, d0, d0, cOR);
                BCNEZ_MARK(fcc1);
                FNEG_S(d0, d0);
            }
            MARK;
            if (v0 != v1) VOR_V(v0, v1, v1);
            VEXTRINS_W(v0, d0, 0);
            break;
        case 0x5F:
            INST_NAME("VMAXSS Gx, Vx, Ex");
            nextop = F8;
            GETVYx(v1, 0);
            GETEYSS(v2, 0, 0);
            GETGYx_empty(v0);
            q0 = fpu_get_scratch(dyn);
            if (BOX64ENV(dynarec_fastnan)) {
                FMAX_S(q0, v1, v2);
            } else {
                FCMP_S(fcc0, v2, v1, cLT);
                FSEL(q0, v2, v1, fcc0);
            }
            if (v0 != v1) VOR_V(v0, v1, v1);
            VEXTRINS_W(v0, q0, 0);
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
        case 0x70:
            INST_NAME("VPSHUFHW Gx, Ex, Ib");
            nextop = F8;
            GETGY_empty_EY_xy(v0, v1, 1);
            u8 = F8;
            d0 = fpu_get_scratch(dyn);
            if (v0 != v1) {
                VSHUF4Ixy(H, v0, v1, u8);
                VEXTRINSxy(D, v0, v1, VEXTRINS_IMM_4_0(0, 0));
            } else {
                VSHUF4Ixy(H, d0, v1, u8);
                VEXTRINSxy(D, v0, d0, VEXTRINS_IMM_4_0(1, 1));
            }
            break;
        case 0x7E:
            INST_NAME("VMOVQ Gx, Ex");
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
        case 0xC2:
            INST_NAME("VCMPSS Gx, Vx, Ex, Ib");
            nextop = F8;
            GETVYx(v1, 0);
            GETEYSS(v2, 0, 1);
            GETGYx(v0, 1);
            q0 = fpu_get_scratch(dyn);
            u8 = F8;
            switch (u8 & 0xf) {
                case 0x00: VFCMP_S(q0, v1, v2, cEQ); break;  // Equal, not unordered
                case 0x01: VFCMP_S(q0, v1, v2, cLT); break;  // Less than
                case 0x02: VFCMP_S(q0, v1, v2, cLE); break;  // Less or equal
                case 0x03: VFCMP_S(q0, v1, v2, cUN); break;  // unordered
                case 0x04: VFCMP_S(q0, v1, v2, cUNE); break; // Not Equal (or unordered on ARM, not on X86...)
                case 0x05: VFCMP_S(q0, v2, v1, cULE); break; // Greater or equal or unordered
                case 0x06: VFCMP_S(q0, v2, v1, cULT); break; // Greater or unordered
                case 0x07: VFCMP_S(q0, v1, v2, cOR); break;  // Greater or unordered
                case 0x08: VFCMP_S(q0, v1, v2, cUEQ); break; // Equal, or unordered
                case 0x09: VFCMP_S(q0, v1, v2, cULT); break; // Less than or unordered
                case 0x0a: VFCMP_S(q0, v1, v2, cULE); break; // Less or equal or unordered
                case 0x0b: VXOR_V(q0, q0, q0); break;        // false
                case 0x0c: VFCMP_S(q0, v1, v2, cNE); break;  // Not Eual, ordered
                case 0x0d: VFCMP_S(q0, v2, v1, cLE); break;  // Greater or Equal ordered
                case 0x0e: VFCMP_S(q0, v2, v1, cLT); break;  // Greater ordered
                case 0x0f: VSEQ_B(q0, v1, v1); break;        // true
            }
            XVPERMI_Q(v0, v1, XVPERMI_IMM_4_0(3, 0));
            XVINSVE0_W(v0, q0, 0);
            break;
        case 0xE6:
            INST_NAME("VCVTDQ2PD Gx, Ex");
            nextop = F8;
            d0 = fpu_get_scratch(dyn);
            if(vex.l){
                GETEYx(v1, 0, 0);
                GETGYy_empty(v0);
                /*  
                    xvffintl.d.w  convert [v0,v1,v2,v3,v4,v5,v6,v7] to [v0,v1,v4,v5]
                    xvffinth.d.w  convert [v0,v1,v2,v3,v4,v5,v6,v7] to [v2,v3,v6,v7]
                    so user xvpermi.d to reorder input [v0v1,v2v3,v4v5,v6v7] to [v0v1,v4v5,v2v3,v6v7]
                */
                XVPERMI_D(d0, v1, 0b11011000);
                XVFFINTL_D_W(v0, d0);
            }else{
                GETEYSD(v1, 0, 0);
                GETGYx_empty(v0);
                VFFINTL_D_W(v0, v1);
            }
            break;
        default:
            DEFAULT;
    }
    return addr;
}
