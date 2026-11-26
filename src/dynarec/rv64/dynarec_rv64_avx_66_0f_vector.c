#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_functions.h"
#include "../dynarec_helper.h"

uintptr_t dynarec64_AVX_66_0F_vector(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed;
    uint8_t wback;
    int v0, v1, v2;
    int q0, q1, q2, q3, q4, q5, d0, d1;
    int j64;
    int64_t fixedaddress;
    rex_t rex = vex.rex;

    MAYUSE(v1);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(q2);
    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(j64);

    switch (opcode) {
        case 0x10:
            INST_NAME("VMOVUPD Gx, Ex");
            nextop = F8;
            gd = ((nextop >> 3) & 7) + (rex.r << 3);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                simd_copy(dyn, ninst, x1, gd, ed, 16 << vex.l, 1);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                v0 = simd_write(dyn, ninst, x1, gd, 16 << vex.l, VECTOR_SEW8);
                VLE_V(v0, ed, VECTOR_SEW8, VECTOR_UNMASKED, VECTOR_NFIELD1);
                simd_commit(dyn, ninst, gd, vex.l ? SIMD_YMM : SIMD_XMM, vex.l ? 0 : SIMD_HI128);
            }
            break;
        case 0x11:
            INST_NAME("VMOVUPD Ex, Gx");
            nextop = F8;
            gd = ((nextop >> 3) & 7) + (rex.r << 3);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                simd_copy(dyn, ninst, x1, ed, gd, 16 << vex.l, 1);
            } else {
                v0 = simd_get(dyn, ninst, x1, gd, 16 << vex.l, VECTOR_SEW8, 0);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                VSE_V(v0, ed, VECTOR_SEW8, VECTOR_UNMASKED, VECTOR_NFIELD1);
                SMWRITE2();
            }
            break;
        case 0x12:
            INST_NAME("VMOVLPD Gx, Vx, Ex");
            nextop = F8;
            if (MODREG) return 0;
            q0 = vpu_get_scratch(dyn);
            simd_load_reg(dyn, ninst, x1, q0, vex.v, 16, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            SMREAD();
            addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
            LD(x2, ed, fixedaddress);
            rvv_set_vl_for_width(dyn, ninst, x1, VECTOR_SEW64, VECTOR_LMUL1, 16);
            VMV_V_V(v0, q0);
            VMV_S_X(v0, x2);
            simd_store_reg(dyn, ninst, x1, v0, gd, 16, VECTOR_SEW64);
            break;
        case 0x13:
            INST_NAME("VMOVLPD Ex, Gx");
            nextop = F8;
            if (MODREG) return 0;
            GETGY_vector(q0, VECTOR_SEW64);
            addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
            VMV_X_S(x4, q0);
            SD(x4, ed, fixedaddress);
            SMWRITE2();
            break;
        case 0x14:
            INST_NAME("VUNPCKLPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW64);
            q2 = vpu_get_scratch(dyn);
            v1 = vpu_get_scratch(dyn);
            v2 = vpu_get_scratch(dyn);
            VID_V(q2, VECTOR_UNMASKED);
            VAND_VI(q2, q2, 2, VECTOR_UNMASKED);
            VRGATHER_VV_SAFE(v1, q0, q2, VECTOR_UNMASKED);
            VRGATHER_VV_SAFE(v2, q1, q2, VECTOR_UNMASKED);
            VECTOR_LOAD_VMASK(0x0a, x4);
            VMERGE_VVM(v0, v1, v2);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x15:
            INST_NAME("VUNPCKHPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW64);
            q2 = vpu_get_scratch(dyn);
            v1 = vpu_get_scratch(dyn);
            v2 = vpu_get_scratch(dyn);
            VID_V(q2, VECTOR_UNMASKED);
            VAND_VI(q2, q2, 2, VECTOR_UNMASKED);
            VADD_VI(q2, q2, 1, VECTOR_UNMASKED);
            VRGATHER_VV_SAFE(v1, q0, q2, VECTOR_UNMASKED);
            VRGATHER_VV_SAFE(v2, q1, q2, VECTOR_UNMASKED);
            VECTOR_LOAD_VMASK(0x0a, x4);
            VMERGE_VVM(v0, v1, v2);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x16:
            INST_NAME("VMOVHPD Gx, Vx, Ex");
            nextop = F8;
            q0 = vpu_get_scratch(dyn);
            simd_load_reg(dyn, ninst, x1, q0, vex.v, 16, VECTOR_SEW64);
            q1 = vpu_get_scratch(dyn);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                simd_load_reg(dyn, ninst, x1, q1, ed, 16, VECTOR_SEW64);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                LD(x4, ed, fixedaddress);
                VMV_S_X(q1, x4);
            }
            GETGY_empty_vector(v0);
            simd_set_width(dyn, ninst, x1, VECTOR_SEW64, 16);
            VMV_V_V(v0, q0);
            VSLIDEUP_VI_SAFE(v0, q1, 1, VECTOR_UNMASKED);
            simd_store_reg(dyn, ninst, x1, v0, gd, 16, VECTOR_SEW64);
            break;
        case 0x17:
            INST_NAME("VMOVHPD Ex, Gx");
            nextop = F8;
            if (MODREG) return 0;
            GETGY_vector(q0, VECTOR_SEW64);
            q1 = vpu_get_scratch(dyn);
            VSLIDEDOWN_VI(q1, q0, 1, VECTOR_UNMASKED);
            VMV_X_S(x4, q1);
            addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
            SD(x4, ed, fixedaddress);
            SMWRITE2();
            break;
        case 0x28:
            INST_NAME("VMOVAPD Gx, Ex");
            nextop = F8;
            gd = ((nextop >> 3) & 7) + (rex.r << 3);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                simd_copy(dyn, ninst, x1, gd, ed, 16 << vex.l, 1);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                v0 = simd_write(dyn, ninst, x1, gd, 16 << vex.l, VECTOR_SEW8);
                VLE_V(v0, ed, VECTOR_SEW8, VECTOR_UNMASKED, VECTOR_NFIELD1);
                simd_commit(dyn, ninst, gd, vex.l ? SIMD_YMM : SIMD_XMM, vex.l ? 0 : SIMD_HI128);
            }
            break;
        case 0x29:
            INST_NAME("VMOVAPD Ex, Gx");
            nextop = F8;
            gd = ((nextop >> 3) & 7) + (rex.r << 3);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                simd_copy(dyn, ninst, x1, ed, gd, 16 << vex.l, 1);
            } else {
                v0 = simd_get(dyn, ninst, x1, gd, 16 << vex.l, VECTOR_SEW8, 0);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                VSE_V(v0, ed, VECTOR_SEW8, VECTOR_UNMASKED, VECTOR_NFIELD1);
                SMWRITE2();
            }
            break;
        case 0x2B:
            INST_NAME("VMOVNTPD Ex, Gx");
            nextop = F8;
            if (MODREG) return 0;
            GETGY_vector(v0, VECTOR_SEW8);
            addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW8);
            VSE8_V(v0, ed, VECTOR_UNMASKED, VECTOR_NFIELD1);
            SMWRITE2();
            break;
        case 0x2E:
        case 0x2F:
            if (opcode == 0x2F) {
                INST_NAME("VCOMISD Gx, Ex");
            } else {
                INST_NAME("VUCOMISD Gx, Ex");
            }
            SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            nextop = F8;
            gd = ((nextop & 0x38) >> 3) + (rex.r << 3);
            q0 = vpu_get_scratch(dyn);
            simd_load_reg(dyn, ninst, x1, q0, gd, 16, VECTOR_SEW64);
            d0 = fpu_get_scratch(dyn);
            VFMV_F_S(d0, q0);
            d1 = fpu_get_scratch(dyn);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                q1 = VREG(ed);
                simd_load_reg(dyn, ninst, x1, q1, ed, 16, VECTOR_SEW64);
                VFMV_F_S(d1, q1);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, x2, &fixedaddress, rex, NULL, 0, 0);
                FLD(d1, ed, fixedaddress);
            }
            CLEAR_FLAGS();
            IFX (X_ZF | X_PF | X_CF) {
                FEQD(x3, d0, d0);
                FEQD(x2, d1, d1);
                AND(x2, x2, x3);
                BNE_MARK(x2, xZR);
                ORI(xFlags, xFlags, (1 << F_ZF) | (1 << F_PF) | (1 << F_CF));
                B_NEXT_nocond;
            }
            MARK;
            IFX (X_CF) {
                FLTD(x2, d0, d1);
                BEQ_MARK2(x2, xZR);
                ORI(xFlags, xFlags, 1 << F_CF);
                B_NEXT_nocond;
            }
            MARK2;
            IFX (X_ZF) {
                FEQD(x2, d0, d1);
                CBZ_NEXT(x2);
                ORI(xFlags, xFlags, 1 << F_ZF);
            }
            break;
        case 0x50:
            INST_NAME("VMOVMSKPD Gd, Ex");
            nextop = F8;
            GETEY_vector(q0, 0, VECTOR_SEW64);
            GETGDd;
            VMSLT_VX(VMASK, q0, xZR, VECTOR_UNMASKED);
            VMV_X_S(gd, VMASK);
            ANDI(gd, gd, vex.l ? 0x0f : 0x03);
            break;
        case 0x51:
            INST_NAME("VSQRTPD Gx, Ex");
            nextop = F8;
            GETEY_vector(v1, 0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            if (v0 == v1) {
                v2 = vpu_get_scratch(dyn);
                VMV_V_V(v2, v1);
                v1 = v2;
            }
            if (BOX64ENV(dynarec_fastround)) {
                VFSQRT_V(v0, v1, VECTOR_UNMASKED);
            } else {
                VFSQRT_V(v0, v1, VECTOR_UNMASKED);
            }
            if (!BOX64ENV(dynarec_fastnan)) {
                q2 = vpu_get_scratch(dyn);
                d0 = vpu_get_scratch(dyn);
                d1 = vpu_get_scratch(dyn);
                MOV64x(x4, 0x0008000000000000LL);
                VOR_VX(d0, v1, x4, VECTOR_UNMASKED);   // quiet(Vx)
                VMFNE_VV(q2, v1, v1, VECTOR_UNMASKED); // Vx is NaN
                VMAND_MM(VMASK, q2, q2);
                VMERGE_VVM(v0, v0, d0);                // NaN source wins
                VMFNE_VV(d1, v0, v0, VECTOR_UNMASKED); // result is NaN
                VMNAND_MM(q2, q2, q2);                 // no input was NaN
                VMAND_MM(d1, d1, q2);                  // invalid operation
                MOV64x(x4, 0xfff8000000000000LL);
                VMV_V_X(d0, x4);
                VMAND_MM(VMASK, d1, d1);
                VMERGE_VVM(v0, v0, d0);
            }
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x54:
            INST_NAME("VANDPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW64);
            VAND_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x55:
            INST_NAME("VANDNPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW64);
            if (cpuext.zvbb) {
                VANDN_VV(v0, q1, q0, VECTOR_UNMASKED);
            } else {
                VEC_SRC_KEEP(q1, v0);
                VXOR_VI(v0, q0, 0x1f, VECTOR_UNMASKED);
                VAND_VV(v0, v0, q1, VECTOR_UNMASKED);
            }
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x56:
            INST_NAME("VORPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW64);
            VOR_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x57:
            INST_NAME("VXORPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW64);
            VXOR_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x58:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("VADDPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW64);
            VFADD_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x59:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("VMULPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW64);
            VFMUL_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x5A:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("VCVTPD2PS Gx, Ex");
            nextop = F8;
            d0 = vpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                simd_load_reg(dyn, ninst, x1, d0, ed, 16 << vex.l, VECTOR_SEW64);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                if (fixedaddress) { ADDI(x4, ed, fixedaddress); ed = x4; fixedaddress = 0; }
                SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW64);
                VLE64_V(d0, ed, VECTOR_UNMASKED, VECTOR_NFIELD1);
            }
            v0 = vpu_get_scratch(dyn);
            if (!vex.l && !cpuext.xtheadvector) {
                VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
            }
            rvv_set_vl_for_width(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL1, vex.l ? 16 : 8);
            VFNCVT_F_F_W(v0, d0, VECTOR_UNMASKED);
            gd = ((nextop & 0x38) >> 3) + (rex.r << 3);
            simd_store_reg(dyn, ninst, x1, v0, gd, 16, VECTOR_SEW32);
            break;
        case 0x5B:
            INST_NAME("VCVTPS2DQ Gx, Ex");
            nextop = F8;
            GETEY_vector(q0, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            if (!BOX64ENV(dynarec_fastround)) FSFLAGSI(0);
            VFCVT_X_F_V(v0, q0, VECTOR_UNMASKED);
            if (!BOX64ENV(dynarec_fastround)) {
                MOV32w(x4, 0x7fffffff);
                VMSEQ_VX(VMASK, v0, x4, VECTOR_UNMASKED);
                MOV32w(x4, 0x80000000);
                VMERGE_VXM(v0, v0, x4);
                FRFLAGS(x5);
                ANDI(x5, x5, (1 << FR_NV) | (1 << FR_OF));
                BEQ_MARK(x5, xZR);
                emit_mxcsr_raise_invalid(x4);
                MARK;
                FRFLAGS(x5);
                ANDI(x5, x5, 1 << FR_NX);
                BEQ_MARK2(x5, xZR);
                emit_mxcsr_raise_precision(x4);
                MARK2;
            }
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x5C:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("VSUBPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW64);
            VFSUB_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x5D:
            INST_NAME("VMINPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW64);
            VMFLT_VV(VMASK, q0, q1, VECTOR_UNMASKED);
            VMERGE_VVM(v0, q1, q0);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x5E:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("VDIVPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW64);
            VFDIV_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x5F:
            INST_NAME("VMAXPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW64);
            VMFLT_VV(VMASK, q1, q0, VECTOR_UNMASKED);
            VMERGE_VVM(v0, q1, q0);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x60 ... 0x62:
        case 0x68 ... 0x6A: {
            int element_sew = VECTOR_SEW8 + (opcode & 7);
            int unpack_high = !!(opcode & 8);
            if (element_sew == VECTOR_SEW8) {
                INST_NAME(unpack_high ? "VPUNPCKHBW Gx, Vx, Ex" : "VPUNPCKLBW Gx, Vx, Ex");
            } else if (element_sew == VECTOR_SEW16) {
                INST_NAME(unpack_high ? "VPUNPCKHWD Gx, Vx, Ex" : "VPUNPCKLWD Gx, Vx, Ex");
            } else {
                INST_NAME(unpack_high ? "VPUNPCKHDQ Gx, Vx, Ex" : "VPUNPCKLDQ Gx, Vx, Ex");
            }
            nextop = F8;
            unsigned regions = vex.l ? SIMD_YMM : SIMD_XMM;
            q0 = simd_access(dyn, ninst, x1, vex.v, regions, 0);
            if (MODREG) {
                q1 = simd_access(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), regions, 0);
            } else {
                GETEY_vector(q1, 0, element_sew);
            }
            GETGY_empty_vector(v0);
            if (vex.l) {
                int selected_left = vpu_get_scratch(dyn);
                int selected_right = q0 == q1 ? selected_left : vpu_get_scratch(dyn);
                SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW64);
                VMV_V_I(VMASK, unpack_high ? 0b1010 : 0b0101);
                VCOMPRESS_VM(selected_left, q0, VMASK);
                if (selected_left != selected_right) { VCOMPRESS_VM(selected_right, q1, VMASK); }
                q0 = selected_left;
                q1 = selected_right;
            }
            rvv_set_vl_for_width(dyn, ninst, x1, element_sew, VECTOR_LMULF2, 8 << vex.l);
            if (!vex.l && unpack_high) {
                int selected_left = vpu_get_scratch(dyn);
                int selected_right = q0 == q1 ? selected_left : vpu_get_scratch(dyn);
                int lane_offset = 8 >> element_sew;
                VSLIDEDOWN_VI(selected_left, q0, lane_offset, VECTOR_UNMASKED);
                if (selected_left != selected_right) { VSLIDEDOWN_VI(selected_right, q1, lane_offset, VECTOR_UNMASKED); }
                q0 = selected_left;
                q1 = selected_right;
            }
            int shift_bits = 8 << element_sew;
            int odd_elements = vpu_get_scratch(dyn);
            if (cpuext.zvbb) {
                if (shift_bits == 32) {
                    ADDI(x4, xZR, 32);
                    VWSLL_VX(odd_elements, q1, x4, VECTOR_UNMASKED);
                } else {
                    VWSLL_VI(odd_elements, q1, shift_bits, VECTOR_UNMASKED);
                }
                int result = v0 == q0 ? odd_elements : v0;
                VWADDU_WV(result, odd_elements, q0, VECTOR_UNMASKED);
                if (result != v0) { VMV1R_V(v0, result); }
            } else {
                int even_elements = vpu_get_scratch(dyn);
                VWADDU_VX(odd_elements, q1, xZR, VECTOR_UNMASKED);
                VWADDU_VX(even_elements, q0, xZR, VECTOR_UNMASKED);
                SET_AVX_VECTOR_WIDTH(x1, element_sew + 1);
                if (shift_bits == 32) {
                    ADDI(x4, xZR, 32);
                    VSLL_VX(odd_elements, odd_elements, x4, VECTOR_UNMASKED);
                } else {
                    VSLL_VI(odd_elements, odd_elements, shift_bits, VECTOR_UNMASKED);
                }
                VOR_VV(v0, even_elements, odd_elements, VECTOR_UNMASKED);
            }
            PUTGY_vector(v0, element_sew);
            break;
        }
        case 0x63:
        case 0x67:
        case 0x6B: {
            int wide_sew = opcode == 0x6B ? VECTOR_SEW32 : VECTOR_SEW16;
            int narrow_sew = wide_sew - 1;
            int unsigned_output = opcode == 0x67;
            if (opcode == 0x63) {
                INST_NAME("VPACKSSWB Gx, Vx, Ex");
            } else if (unsigned_output) {
                INST_NAME("VPACKUSWB Gx, Vx, Ex");
            } else {
                INST_NAME("VPACKSSDW Gx, Vx, Ex");
            }
            nextop = F8;
            unsigned regions = vex.l ? SIMD_YMM : SIMD_XMM;
            q0 = simd_access(dyn, ninst, x1, vex.v, regions, 0);
            if (MODREG) {
                q1 = simd_access(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), regions, 0);
            } else {
                GETEY_vector(q1, 0, wide_sew);
            }
            GETGY_empty_vector(v0);
            if (vex.l) {
                int packed_left = vpu_get_scratch(dyn);
                int packed_right = q0 == q1 ? packed_left : vpu_get_scratch(dyn);
                if (unsigned_output) {
                    SET_AVX_VECTOR_WIDTH(x1, wide_sew);
                    VMAX_VX(packed_left, q0, xZR, VECTOR_UNMASKED);
                    if (q0 != q1) { VMAX_VX(packed_right, q1, xZR, VECTOR_UNMASKED); }
                    rvv_set_vl_for_width(dyn, ninst, x1, narrow_sew, VECTOR_LMULF2, 16);
                    VNCLIPU_WI(packed_left, packed_left, 0, VECTOR_UNMASKED);
                    if (q0 != q1) { VNCLIPU_WI(packed_right, packed_right, 0, VECTOR_UNMASKED); }
                } else {
                    rvv_set_vl_for_width(dyn, ninst, x1, narrow_sew, VECTOR_LMULF2, 16);
                    VNCLIP_WI(packed_left, q0, 0, VECTOR_UNMASKED);
                    if (q0 != q1) { VNCLIP_WI(packed_right, q1, 0, VECTOR_UNMASKED); }
                }
                SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW64);
                uint64_t lane_indices = 0x0001000100000000ULL;
                if (q0 != q1) {
                    VSLIDEUP_VI(packed_left, packed_right, 2, VECTOR_UNMASKED);
                    lane_indices = 0x0003000100020000ULL;
                }
                int indices = vpu_get_scratch(dyn);
                MOV64x(x4, lane_indices);
                VMV_S_X(indices, x4);
                VRGATHEREI16_VV(v0, packed_left, indices, VECTOR_UNMASKED);
            } else {
                int joined = vpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                simd_set_width(dyn, ninst, x1, wide_sew, 32);
                if (unsigned_output) {
                    int positive_right = vpu_get_scratch(dyn);
                    VMAX_VX(joined, q0, xZR, VECTOR_UNMASKED);
                    VMAX_VX(positive_right, q1, xZR, VECTOR_UNMASKED);
                    q1 = positive_right;
                } else {
                    VMV_V_V(joined, q0);
                }
                VSLIDEUP_VI(joined, q1, 16 >> wide_sew, VECTOR_UNMASKED);
                SET_AVX_VECTOR_WIDTH(x1, narrow_sew);
                if (unsigned_output) {
                    VNCLIPU_WI(v0, joined, 0, VECTOR_UNMASKED);
                } else {
                    VNCLIP_WI(v0, joined, 0, VECTOR_UNMASKED);
                }
            }
            PUTGY_vector(v0, narrow_sew);
            break;
        }
        case 0x6C:
            INST_NAME("VPUNPCKLQDQ Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector_safe(v0, q0, q1, VECTOR_SEW64);
            if (vex.l) {
                q2 = vpu_get_scratch(dyn);
                VID_V(q2, VECTOR_UNMASKED);
                VAND_VI(q2, q2, 1, VECTOR_UNMASKED);
                VMSNE_VX(VMASK, q2, xZR, VECTOR_UNMASKED);
                VMV_V_V(v0, q0);
                VSLIDEUP_VI(v0, q1, 1, VECTOR_MASKED);
            } else {
                VMV_V_V(v0, q0);
                VSLIDEUP_VI_SAFE(v0, q1, 1, VECTOR_UNMASKED);
            }
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x6D:
            INST_NAME("VPUNPCKHQDQ Gx, Vx, Ex");
            nextop = F8;
            if (vex.l) {
                GETGY_empty_VYEY_vector_safe(v0, q0, q1, VECTOR_SEW64);
                q2 = vpu_get_scratch(dyn);
                v1 = vpu_get_scratch(dyn);
                v2 = vpu_get_scratch(dyn);
                VID_V(q2, VECTOR_UNMASKED);
                VAND_VI(q2, q2, 1, VECTOR_UNMASKED);
                VMSNE_VX(VMASK, q2, xZR, VECTOR_UNMASKED);
                VSLIDEDOWN_VI(v1, q0, 1, VECTOR_UNMASKED);
                VSLIDEDOWN_VI(v2, q1, 1, VECTOR_UNMASKED);
                VSLIDEUP_VI(v1, v2, 1, VECTOR_MASKED);
                VMV_V_V(v0, v1);
            } else {
                GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW64);
                q2 = vpu_get_scratch(dyn);
                v1 = vpu_get_scratch(dyn);
                VSLIDEDOWN_VI(q2, q0, 1, VECTOR_UNMASKED);
                VSLIDEDOWN_VI(v1, q1, 1, VECTOR_UNMASKED);
                VSLIDEUP_VI_SAFE(q2, v1, 1, VECTOR_UNMASKED);
                VMV_V_V(v0, q2);
            }
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x6E:
            if (vex.l) return 0;
            if (rex.w)
                INST_NAME("VMOVQ Gx, Eq");
            else
                INST_NAME("VMOVD Gx, Ed");
            nextop = F8;
            GETGY_empty_vector(v0);
            GETED(0);
            simd_set_width(dyn, ninst, x1, VECTOR_SEW64, 16);
            VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
            if (!rex.w) {
                ZEXTW2(x4, ed);
                ed = x4;
            }
            VMV_S_X(v0, ed);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x70: {
            INST_NAME("VPSHUFD Gx, Ex, Ib");
            nextop = F8;
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                if (PK(0) == 0xe4) {
                    ++addr;
                    gd = ((nextop >> 3) & 7) + (rex.r << 3);
                    simd_copy(dyn, ninst, x1, gd, ed, 16 << vex.l, 1);
                    break;
                }
                q1 = simd_access(dyn, ninst, x1, ed, vex.l ? SIMD_YMM : SIMD_XMM, 0);
            } else {
                GETEY_vector(q1, 1, VECTOR_SEW32);
            }
            GETGY_empty_vector(v0);
            uint8_t control = F8;
            if (control == 0xe4) {
                PUTGY_vector(q1, VECTOR_SEW32);
                break;
            }
            if (cpuext.zvbb && control == 0xb1) {
                SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW64);
                VROR_VI(v0, q1, 32, VECTOR_UNMASKED);
                PUTGY_vector(v0, VECTOR_SEW64);
                break;
            }

            int result = v0 == q1 ? vpu_get_scratch(dyn) : v0;
            int first_element = control & 3;
            if (control == 0x55 * first_element) {
                SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW32);
                VRGATHER_VI(result, q1, first_element, VECTOR_UNMASKED);
                if (vex.l) {
                    VECTOR_LOAD_VMASK(0xf0, x4);
                    VRGATHER_VI(result, q1, first_element + 4, VECTOR_MASKED);
                }
            } else {
                uint64_t packed_indices = first_element | ((uint64_t)((control >> 2) & 3) << 16) | ((uint64_t)((control >> 4) & 3) << 32) | ((uint64_t)(control >> 6) << 48);
                int indices = vpu_get_scratch(dyn);
                rvv_set_vl_for_width(dyn, ninst, x1, VECTOR_SEW64, VECTOR_LMUL1, 16);
                MOV64x(x4, packed_indices);
                if (vex.l) {
                    int upper_indices = vpu_get_scratch(dyn);
                    MOV64x(x5, 0x0004000400040004ULL);
                    ADD(x5, x4, x5);
                    VMV_S_X(upper_indices, x5);
                    VSLIDE1UP_VX(indices, upper_indices, x4, VECTOR_UNMASKED);
                } else {
                    VMV_S_X(indices, x4);
                }
                SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW32);
                VRGATHEREI16_VV(result, q1, indices, VECTOR_UNMASKED);
            }
            PUTGY_vector(result, VECTOR_SEW32);
            break;
        }
        case 0x71:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 2:
                    INST_NAME("VPSRLW Gx, Ex, Ib");
                    GETEY_vector(q1, 1, VECTOR_SEW16);
                    gd = vex.v;
                    v0 = simd_get_reg_empty(dyn, ninst, x1, gd);
                    u8 = F8;
                    if (u8 > 15)
                        VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
                    else
                        VSRL_VI(v0, q1, u8, VECTOR_UNMASKED);
                    PUTGY_vector(v0, VECTOR_SEW16);
                    break;
                case 4:
                    INST_NAME("VPSRAW Gx, Ex, Ib");
                    GETEY_vector(q1, 1, VECTOR_SEW16);
                    gd = vex.v;
                    v0 = simd_get_reg_empty(dyn, ninst, x1, gd);
                    u8 = F8;
                    if (u8 > 15) u8 = 15;
                    VSRA_VI(v0, q1, u8, VECTOR_UNMASKED);
                    PUTGY_vector(v0, VECTOR_SEW16);
                    break;
                case 6:
                    INST_NAME("VPSLLW Gx, Ex, Ib");
                    GETEY_vector(q1, 1, VECTOR_SEW16);
                    gd = vex.v;
                    v0 = simd_get_reg_empty(dyn, ninst, x1, gd);
                    u8 = F8;
                    if (u8 > 15)
                        VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
                    else
                        VSLL_VI(v0, q1, u8, VECTOR_UNMASKED);
                    PUTGY_vector(v0, VECTOR_SEW16);
                    break;
                default:
                    DEFAULT_VECTOR;
            }
            break;
        case 0x72:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 2:
                    INST_NAME("VPSRLD Gx, Ex, Ib");
                    GETEY_vector(q1, 1, VECTOR_SEW32);
                    gd = vex.v;
                    v0 = simd_get_reg_empty(dyn, ninst, x1, gd);
                    u8 = F8;
                    if (u8 > 31)
                        VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
                    else
                        VSRL_VI(v0, q1, u8, VECTOR_UNMASKED);
                    PUTGY_vector(v0, VECTOR_SEW32);
                    break;
                case 4:
                    INST_NAME("VPSRAD Gx, Ex, Ib");
                    GETEY_vector(q1, 1, VECTOR_SEW32);
                    gd = vex.v;
                    v0 = simd_get_reg_empty(dyn, ninst, x1, gd);
                    u8 = F8;
                    if (u8 > 31) u8 = 31;
                    VSRA_VI(v0, q1, u8, VECTOR_UNMASKED);
                    PUTGY_vector(v0, VECTOR_SEW32);
                    break;
                case 6:
                    INST_NAME("VPSLLD Gx, Ex, Ib");
                    GETEY_vector(q1, 1, VECTOR_SEW32);
                    gd = vex.v;
                    v0 = simd_get_reg_empty(dyn, ninst, x1, gd);
                    u8 = F8;
                    if (u8 > 31)
                        VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
                    else
                        VSLL_VI(v0, q1, u8, VECTOR_UNMASKED);
                    PUTGY_vector(v0, VECTOR_SEW32);
                    break;
                default:
                    DEFAULT_VECTOR;
            }
            break;
        case 0x73:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 2:
                    INST_NAME("VPSRLQ Gx, Ex, Ib");
                    GETEY_vector(q1, 1, VECTOR_SEW64);
                    gd = vex.v;
                    v0 = simd_get_reg_empty(dyn, ninst, x1, gd);
                    u8 = F8;
                    if (u8 > 63)
                        VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
                    else if (u8 > 31) {
                        ADDI(x1, xZR, u8);
                        VSRL_VX(v0, q1, x1, VECTOR_UNMASKED);
                    } else
                        VSRL_VI(v0, q1, u8, VECTOR_UNMASKED);
                    PUTGY_vector(v0, VECTOR_SEW64);
                    break;
                case 3:
                case 7: {
                    int shift_left = !!(nextop & 0x20);
                    INST_NAME(shift_left ? "VPSLLDQ Gx, Ex, Ib" : "VPSRLDQ Gx, Ex, Ib");
                    unsigned immediate = MODREG ? PK(0) : 0;
                    int element_sew = immediate && immediate < 16 ? __builtin_ctz(immediate) : VECTOR_SEW8;
                    GETEY_vector(q1, 1, element_sew);
                    gd = vex.v;
                    v0 = simd_get_reg_empty(dyn, ninst, x1, gd);
                    u8 = F8;
                    if (u8 >= 16) {
                        VMV_V_I(v0, 0);
                    } else if (u8 == 0) {
                        if (v0 != q1) { VMV_V_V(v0, q1); }
                    } else {
                        int slide = u8 >> element_sew;
                        int lane_elements = 16 >> element_sew;
                        if (!vex.l && shift_left) {
                            VEC_SRC_KEEP(q1, v0);
                            if (slide == 1) {
                                VSLIDE1UP_VX(v0, q1, xZR, VECTOR_UNMASKED);
                            } else {
                                VMV_V_I(v0, 0);
                                VSLIDEUP_VI(v0, q1, slide, VECTOR_UNMASKED);
                            }
                        } else if (!vex.l && slide == 1) {
                            VSLIDE1DOWN_VX(v0, q1, xZR, VECTOR_UNMASKED);
                        } else {
                            uint32_t zero_mask = (1u << slide) - 1;
                            if (!shift_left) zero_mask <<= lane_elements - slide;
                            if (vex.l) zero_mask |= zero_mask << lane_elements;
                            VECTOR_LOAD_VMASK(zero_mask, x4);
                            if (shift_left) {
                                int shifted = v0 == q1 ? vpu_get_scratch(dyn) : v0;
                                VSLIDEUP_VI(shifted, q1, slide, VECTOR_UNMASKED);
                                VMERGE_VIM(v0, shifted, 0);
                            } else {
                                VSLIDEDOWN_VI(v0, q1, slide, VECTOR_UNMASKED);
                                VMERGE_VIM(v0, v0, 0);
                            }
                        }
                    }
                    PUTGY_vector(v0, element_sew);
                    break;
                }
                case 6:
                    INST_NAME("VPSLLQ Gx, Ex, Ib");
                    GETEY_vector(q1, 1, VECTOR_SEW64);
                    gd = vex.v;
                    v0 = simd_get_reg_empty(dyn, ninst, x1, gd);
                    u8 = F8;
                    if (u8 > 63)
                        VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
                    else if (u8 > 31) {
                        ADDI(x1, xZR, u8);
                        VSLL_VX(v0, q1, x1, VECTOR_UNMASKED);
                    } else
                        VSLL_VI(v0, q1, u8, VECTOR_UNMASKED);
                    PUTGY_vector(v0, VECTOR_SEW64);
                    break;
                default:
                    DEFAULT_VECTOR;
            }
            break;
        case 0x64:
            INST_NAME("VPCMPGTB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW8);
            VMSLT_VV(VMASK, q1, q0, VECTOR_UNMASKED);
            VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
            VMERGE_VIM(v0, v0, 0x1f);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0x65:
            INST_NAME("VPCMPGTW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW16);
            VMSLT_VV(VMASK, q1, q0, VECTOR_UNMASKED);
            VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
            VMERGE_VIM(v0, v0, 0x1f);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0x66:
            INST_NAME("VPCMPGTD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW32);
            VMSLT_VV(VMASK, q1, q0, VECTOR_UNMASKED);
            VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
            VMERGE_VIM(v0, v0, 0x1f);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x6F:
            INST_NAME("VMOVDQA Gx, Ex");
            nextop = F8;
            gd = ((nextop >> 3) & 7) + (rex.r << 3);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                simd_copy(dyn, ninst, x1, gd, ed, 16 << vex.l, 1);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                v0 = simd_write(dyn, ninst, x1, gd, 16 << vex.l, VECTOR_SEW8);
                VLE_V(v0, ed, VECTOR_SEW8, VECTOR_UNMASKED, VECTOR_NFIELD1);
                simd_commit(dyn, ninst, gd, vex.l ? SIMD_YMM : SIMD_XMM, vex.l ? 0 : SIMD_HI128);
            }
            break;
        case 0x74:
            INST_NAME("VPCMPEQB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW8);
            VMSEQ_VV(VMASK, q0, q1, VECTOR_UNMASKED);
            VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
            VMERGE_VIM(v0, v0, 0x1f);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0x75:
            INST_NAME("VPCMPEQW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW16);
            VMSEQ_VV(VMASK, q0, q1, VECTOR_UNMASKED);
            VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
            VMERGE_VIM(v0, v0, 0x1f);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0x76:
            INST_NAME("VPCMPEQD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW32);
            VMSEQ_VV(VMASK, q0, q1, VECTOR_UNMASKED);
            VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
            VMERGE_VIM(v0, v0, 0x1f);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x7F:
            INST_NAME("VMOVDQA Ex, Gx");
            nextop = F8;
            gd = ((nextop >> 3) & 7) + (rex.r << 3);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                simd_copy(dyn, ninst, x1, ed, gd, 16 << vex.l, 1);
            } else {
                v0 = simd_get(dyn, ninst, x1, gd, 16 << vex.l, VECTOR_SEW8, 0);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                VSE_V(v0, ed, VECTOR_SEW8, VECTOR_UNMASKED, VECTOR_NFIELD1);
                SMWRITE2();
            }
            break;
        case 0x7E:
            INST_NAME(rex.w ? "VMOVQ Ed, Gx" : "VMOVD Ed, Gx");
            nextop = F8;
            q0 = vpu_get_scratch(dyn);
            simd_load_reg(dyn, ninst, x1, q0, ((nextop & 0x38) >> 3) + (rex.r << 3), 16, rex.w ? VECTOR_SEW64 : VECTOR_SEW32);
            SET_AVX_VECTOR_WIDTH(x1, rex.w ? VECTOR_SEW64 : VECTOR_SEW32);
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                VMV_X_S(ed, q0);
                if (!rex.w) ZEROUP(ed);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                VMV_X_S(x4, q0);
                if (rex.w)
                    SD(x4, ed, fixedaddress);
                else
                    SW(x4, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0xD0: {
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("VADDSUBPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW64);
            VECTOR_LOAD_VMASK(vex.l ? 0x05 : 0x01, x4);
            int addend = vpu_get_scratch(dyn);
            VMV_V_V(addend, q1);
            VFSGNJN_VV(addend, q1, q1, VECTOR_MASKED);
            VFADD_VV(v0, q0, addend, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        }
        case 0xD1:
            INST_NAME("VPSRLW Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW16);
            GETEY128_vector(q1, 0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            VMV_X_S(x4, q1);
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW16);
            SLTIU(x5, x4, 16);
            SUB(x5, xZR, x5);
            VSRL_VX(v0, q0, x4, VECTOR_UNMASKED);
            VAND_VX(v0, v0, x5, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0xD2:
            INST_NAME("VPSRLD Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW32);
            GETEY128_vector(q1, 0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            VMV_X_S(x4, q1);
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW32);
            SLTIU(x5, x4, 32);
            SUB(x5, xZR, x5);
            VSRL_VX(v0, q0, x4, VECTOR_UNMASKED);
            VAND_VX(v0, v0, x5, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0xD3:
            INST_NAME("VPSRLQ Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW64);
            GETEY128_vector(q1, 0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            VMV_X_S(x4, q1);
            SLTIU(x5, x4, 64);
            SUB(x5, xZR, x5);
            VSRL_VX(v0, q0, x4, VECTOR_UNMASKED);
            VAND_VX(v0, v0, x5, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0xE1:
            INST_NAME("VPSRAW Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW16);
            GETEY128_vector(q1, 0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            VMV_X_S(x4, q1);
            ADDI(x5, xZR, 15);
            MINU(x4, x4, x5);
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW16);
            VSRA_VX(v0, q0, x4, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0xE2:
            INST_NAME("VPSRAD Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW32);
            GETEY128_vector(q1, 0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            VMV_X_S(x4, q1);
            ADDI(x5, xZR, 31);
            MINU(x4, x4, x5);
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW32);
            VSRA_VX(v0, q0, x4, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0xD6:
            if (vex.l) return 0;
            INST_NAME("VMOVQ Ex, Gx");
            nextop = F8;
            GETGY_vector(q0, VECTOR_SEW64);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                q1 = vpu_get_scratch(dyn);
                simd_set_width(dyn, ninst, x1, VECTOR_SEW64, 16);
                VXOR_VV(q1, q1, q1, VECTOR_UNMASKED);
                VECTOR_LOAD_VMASK(1, x4);
                VMERGE_VVM(q1, q1, q0);
                simd_store_reg(dyn, ninst, x1, q1, ed, 16, VECTOR_SEW64);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                VMV_X_S(x4, q0);
                SD(x4, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0xD4:
            INST_NAME("VPADDQ Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW64);
            VADD_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0xD5:
            INST_NAME("VPMULLW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW16);
            VMUL_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0xD8:
            INST_NAME("VPSUBUSB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW8);
            q2 = vpu_get_scratch(dyn);
            VMSLTU_VV(VMASK, q0, q1, VECTOR_UNMASKED);
            VSUB_VV(v0, q0, q1, VECTOR_UNMASKED);
            VXOR_VV(q2, q2, q2, VECTOR_UNMASKED);
            VMERGE_VVM(v0, v0, q2);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0xD9:
            INST_NAME("VPSUBUSW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW16);
            q2 = vpu_get_scratch(dyn);
            VMSLTU_VV(VMASK, q0, q1, VECTOR_UNMASKED);
            VSUB_VV(v0, q0, q1, VECTOR_UNMASKED);
            VXOR_VV(q2, q2, q2, VECTOR_UNMASKED);
            VMERGE_VVM(v0, v0, q2);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0xDA:
            INST_NAME("VPMINUB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW8);
            VMINU_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0xDB:
            INST_NAME("VPAND Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW64);
            VAND_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0xDC:
            INST_NAME("VPADDUSB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW8);
            VSADDU_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0xDD:
            INST_NAME("VPADDUSW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW16);
            VSADDU_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0xDE:
            INST_NAME("VPMAXUB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW8);
            VMAXU_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0xDF:
            INST_NAME("VPANDN Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW64);
            if (cpuext.zvbb) {
                VANDN_VV(v0, q1, q0, VECTOR_UNMASKED);
            } else {
                VEC_SRC_KEEP(q1, v0);
                VXOR_VI(v0, q0, 0x1f, VECTOR_UNMASKED);
                VAND_VV(v0, v0, q1, VECTOR_UNMASKED);
            }
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0xE0:
        case 0xE3: {
            INST_NAME(opcode == 0xE0 ? "VPAVGB Gx, Vx, Ex" : "VPAVGW Gx, Vx, Ex");
            nextop = F8;
            const int width = opcode == 0xE0 ? VECTOR_SEW8 : VECTOR_SEW16;
            GETGY_empty_VYEY_vector(v0, q0, q1, width);
            if (q0 == q1) {
                v0 = q0;
            } else {
                int difference = vpu_get_scratch(dyn);
                VXOR_VV(difference, q0, q1, VECTOR_UNMASKED);
                VOR_VV(v0, q0, q1, VECTOR_UNMASKED);
                VSRL_VI(difference, difference, 1, VECTOR_UNMASKED);
                VSUB_VV(v0, v0, difference, VECTOR_UNMASKED);
            }
            PUTGY_vector(v0, width);
            break;
        }
        case 0xE4:
            INST_NAME("VPMULHUW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW16);
            VMULHU_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0xE5:
            INST_NAME("VPMULHW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW16);
            VMULH_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0xE7:
            INST_NAME("VMOVNTDQ Ex, Gx");
            nextop = F8;
            if (MODREG) return 0;
            GETGY_vector(v0, VECTOR_SEW8);
            addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW8);
            VSE8_V(v0, ed, VECTOR_UNMASKED, VECTOR_NFIELD1);
            SMWRITE2();
            break;
        case 0xE8:
            INST_NAME("VPSUBSB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW8);
            VSSUB_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0xE9:
            INST_NAME("VPSUBSW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW16);
            VSSUB_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0xEA:
            INST_NAME("VPMINSW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW16);
            VMIN_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0xEB:
            INST_NAME("VPOR Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW64);
            VOR_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0xEC:
            INST_NAME("VPADDSB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW8);
            VSADD_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0xED:
            INST_NAME("VPADDSW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW16);
            VSADD_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0xEE:
            INST_NAME("VPMAXSW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW16);
            VMAX_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0xEF:
            INST_NAME("VPXOR Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW64);
            VXOR_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0xF1:
            INST_NAME("VPSLLW Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW16);
            GETEY128_vector(q1, 0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            VMV_X_S(x4, q1);
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW16);
            SLTIU(x5, x4, 16);
            SUB(x5, xZR, x5);
            VSLL_VX(v0, q0, x4, VECTOR_UNMASKED);
            VAND_VX(v0, v0, x5, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0xF2:
            INST_NAME("VPSLLD Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW32);
            GETEY128_vector(q1, 0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            VMV_X_S(x4, q1);
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW32);
            SLTIU(x5, x4, 32);
            SUB(x5, xZR, x5);
            VSLL_VX(v0, q0, x4, VECTOR_UNMASKED);
            VAND_VX(v0, v0, x5, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0xF3:
            INST_NAME("VPSLLQ Gx, Vx, Ex");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW64);
            GETEY128_vector(q1, 0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            VMV_X_S(x4, q1);
            SLTIU(x5, x4, 64);
            SUB(x5, xZR, x5);
            VSLL_VX(v0, q0, x4, VECTOR_UNMASKED);
            VAND_VX(v0, v0, x5, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0xF4: {
            INST_NAME("VPMULUDQ Gx, Vx, Ex");
            nextop = F8;
            unsigned regions = vex.l ? SIMD_YMM : SIMD_XMM;
            q0 = simd_access(dyn, ninst, x1, vex.v, regions, 0);
            if (MODREG) {
                q1 = simd_access(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), regions, 0);
            } else {
                GETEY_vector(q1, 0, VECTOR_SEW64);
            }
            GETGY_empty_vector(v0);
            rvv_set_vl_for_width(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMULF2, 8 << vex.l);
            int left_dwords = vpu_get_scratch(dyn);
            int right_dwords = left_dwords;
            VNSRL_WI(left_dwords, q0, 0, VECTOR_UNMASKED);
            if (q0 != q1) {
                right_dwords = vpu_get_scratch(dyn);
                VNSRL_WI(right_dwords, q1, 0, VECTOR_UNMASKED);
            }
            VWMULU_VV(v0, left_dwords, right_dwords, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        }
        case 0xF5:
            INST_NAME("VPMADDWD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW16);
            q2 = vpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            d0 = vpu_get_scratch(dyn);
            d1 = vpu_get_scratch(dyn);
            simd_set_width(dyn, ninst, x1, VECTOR_SEW16, 32);
            VWMUL_VV(q2, q0, q1, VECTOR_UNMASKED);
            simd_set_width(dyn, ninst, x1, VECTOR_SEW32, 16 << vex.l);
            MOV32w(x4, 32);
            VNSRL_WX(d0, q2, xZR, VECTOR_UNMASKED);
            VNSRL_WX(d1, q2, x4, VECTOR_UNMASKED);
            VADD_VV(v0, d0, d1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0xF6: {
            INST_NAME("VPSADBW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW8);
            int byte_diffs = vpu_get_scratch(dyn);
            int work = vpu_get_scratch(dyn);
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW8);
            VMAXU_VV(byte_diffs, q0, q1, VECTOR_UNMASKED);
            VMINU_VV(work, q0, q1, VECTOR_UNMASKED);
            VSUB_VV(byte_diffs, byte_diffs, work, VECTOR_UNMASKED);
            if (vex.l) {
                SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW16);
                ADDI(x4, xZR, 0xff);
                VSRL_VI(work, byte_diffs, 8, VECTOR_UNMASKED);
                VAND_VX(byte_diffs, byte_diffs, x4, VECTOR_UNMASKED);
                VADD_VV(byte_diffs, byte_diffs, work, VECTOR_UNMASKED);
                SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW64);
                // Multiplication adds the four pair sums into the top 16 bits of each qword without carry between words.
                MOV64xs(x4, 0x0001000100010001ULL, x5);
                VMUL_VX(v0, byte_diffs, x4, VECTOR_UNMASKED);
                ADDI(x4, xZR, 48);
                VSRL_VX(v0, v0, x4, VECTOR_UNMASKED);
            } else {
                int upper_sum = vpu_get_scratch(dyn);
                VMV_V_I(v0, 0);
                VMV_V_I(upper_sum, 0);
                rvv_set_vl_for_width(dyn, ninst, x1, VECTOR_SEW8, VECTOR_LMUL1, 8);
                VSLIDEDOWN_VI(work, byte_diffs, 8, VECTOR_UNMASKED);
                VWREDSUMU_VS(v0, byte_diffs, v0, VECTOR_UNMASKED);
                VWREDSUMU_VS(upper_sum, work, upper_sum, VECTOR_UNMASKED);
                SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW64);
                VSLIDEUP_VI(v0, upper_sum, 1, VECTOR_UNMASKED);
            }
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        }
        case 0xF8:
            INST_NAME("VPSUBB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW8);
            VSUB_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0xF9:
            INST_NAME("VPSUBW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW16);
            VSUB_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0xFA:
            INST_NAME("VPSUBD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW32);
            VSUB_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0xFB:
            INST_NAME("VPSUBQ Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW64);
            VSUB_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0xFC:
            INST_NAME("VPADDB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW8);
            VADD_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0xFD:
            INST_NAME("VPADDW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW16);
            VADD_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0xFE:
            INST_NAME("VPADDD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW32);
            VADD_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0xC4:
            if (vex.l) return 0;
            INST_NAME("VPINSRW Gx, Vx, Ed, Ib");
            nextop = F8;
            q0 = vpu_get_scratch(dyn);
            simd_load_reg(dyn, ninst, x1, q0, vex.v, 16, VECTOR_SEW16);
            GETGY_empty_vector(v0);
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                ZEXTH(x5, ed);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 1);
                LHU(x5, ed, fixedaddress);
            }
            u8 = F8;
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW16);
            VECTOR_LOAD_VMASK(1 << (u8 & 7), x4);
            q1 = vpu_get_scratch(dyn);
            q2 = vpu_get_scratch(dyn);
            VMV_V_X(q1, x5);
            VMERGE_VVM(q2, q0, q1);
            simd_store_reg(dyn, ninst, x1, q2, gd, 16, VECTOR_SEW16);
            break;
        case 0xC5:
            if (vex.l) return 0;
            nextop = F8;
            if (!MODREG) return 0;
            INST_NAME("VPEXTRW Gd, Ex, Ib");
            GETGDd;
            q0 = vpu_get_scratch(dyn);
            simd_load_reg(dyn, ninst, x1, q0, (nextop & 7) + (rex.b << 3), 16, VECTOR_SEW16);
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW16);
            u8 = F8;
            q1 = vpu_get_scratch(dyn);
            VSLIDEDOWN_VI(q1, q0, u8 & 7, VECTOR_UNMASKED);
            VMV_X_S(x4, q1);
            ZEXTH(gd, x4);
            break;
        case 0xD7:
            INST_NAME("VPMOVMSKB Gd, Ex");
            nextop = F8;
            GETGDd;
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW8);
            q0 = simd_get(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 16 << vex.l, VECTOR_SEW8, 0);
            if (cpuext.xtheadvector) {
                v0 = vpu_get_scratch_lmul(dyn, VECTOR_LMUL8);
                VSRL_VI(v0, q0, 7, VECTOR_UNMASKED);
                // Force the element width to 1bit
                rvv_set_vl_for_width(dyn, ninst, x4, VECTOR_SEW8, VECTOR_LMUL8, 16);
                VMSNE_VX(VMASK, v0, xZR, VECTOR_UNMASKED);
            } else {
                VMSLT_VX(VMASK, q0, xZR, VECTOR_UNMASKED);
            }
            simd_set_width(dyn, ninst, x1, vex.l ? VECTOR_SEW32 : VECTOR_SEW16, 16);
            VMV_X_S(gd, VMASK);
            if (vex.l) {
                ZEXTW2(gd, gd);
            } else {
                ZEXTH(gd, gd);
            }
            break;
        case 0xC6:
            INST_NAME("VSHUFPD Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGY_empty_VYEY_vector_D(v0, q0, q1, VECTOR_SEW64, 1);
            u8 = F8;
            if (!vex.l || (u8 & 3) == ((u8 >> 2) & 3)) {
                if (u8 & 1) {
                    q2 = vpu_get_scratch(dyn);
                    VSLIDEDOWN_VI(q2, q0, 1, VECTOR_UNMASKED);
                    q0 = q2;
                }
                if (!(u8 & 2)) {
                    q2 = vpu_get_scratch(dyn);
                    VSLIDE1UP_VX(q2, q1, xZR, VECTOR_UNMASKED);
                    q1 = q2;
                }
            } else {
                uint64_t selectors = 0;
                for (int element = 0; element < 4; ++element)
                    selectors |= (uint64_t)((element & 2) + ((u8 >> element) & 1)) << (element * 16);
                v1 = vpu_get_scratch(dyn);
                q2 = vpu_get_scratch(dyn);
                MOV64xs(x4, selectors, x3);
                VMV_S_X(v1, x4);
                VRGATHEREI16_VV(q2, q0, v1, VECTOR_UNMASKED);
                if (q0 == q1) {
                    q1 = q2;
                } else {
                    v2 = vpu_get_scratch(dyn);
                    VRGATHEREI16_VV(v2, q1, v1, VECTOR_UNMASKED);
                    q1 = v2;
                }
                q0 = q2;
            }
            if (q0 == q1) {
                v0 = q0;
            } else {
                VECTOR_LOAD_VMASK(0xa, x4);
                VMERGE_VVM(v0, q0, q1);
            }
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0xC2:
            INST_NAME("VCMPPD Gx, Vx, Ex, Ib");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW64);
            GETEY_vector(q1, 1, VECTOR_SEW64);
            u8 = F8;
            GETGY_empty_vector(v0);
            simd_compare(dyn, ninst, v0, q0, q1, u8);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0xE6:
            INST_NAME("VCVTTPD2DQ Gx, Ex");
            nextop = F8;
            GETEY_vector(q0, 0, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            d0 = vpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            q2 = vpu_get_scratch(dyn);
            simd_set_width(dyn, ninst, x1, VECTOR_SEW64, 16 << vex.l);
            VMV_V_V(d0, q0);
            if (BOX64ENV(dynarec_fastround)) {
                simd_set_width(dyn, ninst, x1, VECTOR_SEW32, 16);
                VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
                rvv_set_vl_for_width(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL1, (vex.l ? 16 : 8));
                VFNCVT_RTZ_X_F_W(q2, d0, VECTOR_UNMASKED);
                VMV_V_V(v0, q2);
            } else {
                FSFLAGSI(0);
                q3 = vpu_get_scratch(dyn);
                d1 = vpu_get_scratch(dyn);
                MOV64x(x5, 0xC1E0000000200000LL); // -2^31 - 1
                FMVDX(d1, x5);
                VMFLE_VF(q3, d0, d1, VECTOR_UNMASKED);
                MOV64x(x5, 0x41E0000000000000LL); // 2^31
                FMVDX(d1, x5);
                VMFGE_VF(VMASK, d0, d1, VECTOR_UNMASKED);
                VMOR_MM(q3, q3, VMASK);
                VMFNE_VV(VMASK, d0, d0, VECTOR_UNMASKED); // NaN
                VMOR_MM(q3, q3, VMASK);
                simd_set_width(dyn, ninst, x1, VECTOR_SEW32, 16);
                VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
                rvv_set_vl_for_width(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL1, (vex.l ? 16 : 8));
                VFNCVT_RTZ_X_F_W(q2, d0, VECTOR_UNMASKED);
                v2 = vpu_get_scratch(dyn);
                MOV32w(x5, 0x80000000);
                VMV_V_X(v2, x5);
                VMAND_MM(VMASK, q3, q3);
                VMERGE_VVM(q2, q2, v2);
                VMV_V_V(v0, q2);
                VCPOP_M(x4, VMASK, VECTOR_UNMASKED);
                BEQZ_MARK(x4);
                emit_mxcsr_raise_invalid(x5);
                MARK;
            }
            simd_store_reg(dyn, ninst, x1, v0, gd, 16, VECTOR_SEW32);
            break;
        case 0x7C:
        case 0x7D: {
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            if (opcode == 0x7C) {
                INST_NAME("VHADDPD Gx, Vx, Ex");
            } else {
                INST_NAME("VHSUBPD Gx, Vx, Ex");
            }
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW64);
            int even_terms = vpu_get_scratch(dyn);
            int odd_terms = vpu_get_scratch(dyn);
            if (!vex.l && q0 == q1) {
                VRGATHER_VI(even_terms, q0, 0, VECTOR_UNMASKED);
                VRGATHER_VI(odd_terms, q0, 1, VECTOR_UNMASKED);
            } else {
                VSLIDE1UP_VX(even_terms, q1, xZR, VECTOR_UNMASKED);
                VSLIDEDOWN_VI(odd_terms, q0, 1, VECTOR_UNMASKED);
                VECTOR_LOAD_VMASK(vex.l ? 0x0a : 0x02, x4);
                VMERGE_VVM(even_terms, q0, even_terms);
                VMERGE_VVM(odd_terms, odd_terms, q1);
            }
            if (opcode == 0x7C)
                VFADD_VV(v0, even_terms, odd_terms, VECTOR_UNMASKED);
            else
                VFSUB_VV(v0, even_terms, odd_terms, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        }
        case 0xF7:
            INST_NAME("VMASKMOVDQU Gx, Ex");
            UP32_READ(xRDI);
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW8);
            GETGX_vector(q0, 0, VECTOR_SEW8);
            GETEX_vector(q1, 0, 0, VECTOR_SEW8);
            VMSLT_VX(VMASK, q1, xZR, VECTOR_UNMASKED); // mask = sign bit of each byte
            ADDI(x4, xRDI, 0);
            VSE8_V(q0, x4, VECTOR_MASKED, VECTOR_NFIELD1);
            SMWRITE2();
            break;
        default:
            DEFAULT_VECTOR;
    }
    return addr;
}
