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

uintptr_t dynarec64_AVX_F2_0F_vector(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed;
    int v0, v1, v2;
    int q0, q1, q2, q3, q4, q5, q6, d0, d1;
    int64_t fixedaddress;
    int64_t j64;
    rex_t rex = vex.rex;

    MAYUSE(v1);
    MAYUSE(v2);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(q2);
    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(u8);
    MAYUSE(j64);

    switch (opcode) {
        case 0x10:
            INST_NAME("VMOVSD Gx, [Vx,] Ex");
            nextop = F8;
            if (MODREG) {
                AVX_SCALAR_OP_PROLOG(VECTOR_SEW64);
                if (v0 != q1) VMV_V_V(v0, q1);
            } else {
                gd = ((nextop >> 3) & 7) + (rex.r << 3);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                v0 = simd_write(dyn, ninst, x1, gd, 8, VECTOR_SEW64);
                VLE_V(v0, ed, VECTOR_SEW64, VECTOR_UNMASKED, VECTOR_NFIELD1);
                simd_commit(dyn, ninst, gd, SIMD_LO64, SIMD_HI64 | SIMD_HI128);
            }
            break;
        case 0x11:
            INST_NAME("VMOVSD Ex, [Vx,] Gx");
            nextop = F8;
            gd = ((nextop >> 3) & 7) + (rex.r << 3);
            q1 = simd_get(dyn, ninst, x1, gd, 8, VECTOR_SEW64, 0);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                if (ed == gd && ed != vex.v) {
                    q2 = vpu_get_scratch(dyn);
                    VMV_V_V(q2, q1);
                    q1 = q2;
                }
                simd_get(dyn, ninst, x1, vex.v, 8, VECTOR_SEW64, 0);
                v0 = simd_seed_scalar(dyn, ninst, x1, ed, vex.v, VECTOR_SEW64);
                if (v0 != q1) VMV_V_V(v0, q1);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                VSE_V(q1, ed, VECTOR_SEW64, VECTOR_UNMASKED, VECTOR_NFIELD1);
                SMWRITE2();
            }
            break;
        case 0x58:
            INST_NAME("VADDSD Gx, Vx, Ex");
            nextop = F8;
            AVX_SCALAR_OP_PROLOG(VECTOR_SEW64);
            simd_scalar_arith(dyn, ninst, x1, v0, q0, q1, VECTOR_SEW64, 0x58);
            AVX_SCALAR_OP_STORE(VECTOR_SEW64);
            break;
        case 0x59:
            INST_NAME("VMULSD Gx, Vx, Ex");
            nextop = F8;
            AVX_SCALAR_OP_PROLOG(VECTOR_SEW64);
            simd_scalar_arith(dyn, ninst, x1, v0, q0, q1, VECTOR_SEW64, 0x59);
            AVX_SCALAR_OP_STORE(VECTOR_SEW64);
            break;
        case 0x5C:
            INST_NAME("VSUBSD Gx, Vx, Ex");
            nextop = F8;
            AVX_SCALAR_OP_PROLOG(VECTOR_SEW64);
            simd_scalar_arith(dyn, ninst, x1, v0, q0, q1, VECTOR_SEW64, 0x5c);
            AVX_SCALAR_OP_STORE(VECTOR_SEW64);
            break;
        case 0x5D:
            INST_NAME("VMINSD Gx, Vx, Ex");
            nextop = F8;
            AVX_SCALAR_OP_PROLOG(VECTOR_SEW64);
            VMFLT_VV(VMASK, q0, q1, VECTOR_UNMASKED);
            VMERGE_VVM(v0, q1, q0);
            AVX_SCALAR_OP_STORE(VECTOR_SEW64);
            break;
        case 0x5E:
            INST_NAME("VDIVSD Gx, Vx, Ex");
            nextop = F8;
            AVX_SCALAR_OP_PROLOG(VECTOR_SEW64);
            simd_scalar_arith(dyn, ninst, x1, v0, q0, q1, VECTOR_SEW64, 0x5e);
            AVX_SCALAR_OP_STORE(VECTOR_SEW64);
            break;
        case 0x5F:
            INST_NAME("VMAXSD Gx, Vx, Ex");
            nextop = F8;
            AVX_SCALAR_OP_PROLOG(VECTOR_SEW64);
            VMFLT_VV(VMASK, q1, q0, VECTOR_UNMASKED);
            VMERGE_VVM(v0, q1, q0);
            AVX_SCALAR_OP_STORE(VECTOR_SEW64);
            break;
        case 0x5A:
            INST_NAME("VCVTSD2SS Gx, Vx, Ex");
            nextop = F8;
            d0 = fpu_get_scratch(dyn);
            if (MODREG) {
                q1 = vpu_get_scratch(dyn);
                simd_load_reg(dyn, ninst, x1, q1, (nextop & 7) + (rex.b << 3), 16, VECTOR_SEW64);
                VFMV_F_S(d0, q1);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                FLD(d0, ed, fixedaddress);
            }
            d1 = fpu_get_scratch(dyn);
            FCVTSD(d1, d0);
            if (!BOX64ENV(dynarec_fastnan)) {
                FEQD(x3, d0, d0);
                FMVXD(x4, d0);
                BNEZ_MARK(x3);
                SRLI(x6, x4, 63);
                SLLI(x6, x6, 31);
                SRLI(x4, x4, 29);
                MOV32w(x5, 0x007fffff);
                AND(x4, x4, x5);
                MOV32w(x5, 0x7FC00000);
                OR(x4, x4, x5);
                OR(x4, x4, x6);
                FMVWX(d1, x4);
                MARK;
            }
            q0 = vpu_get_scratch(dyn);
            simd_load_reg(dyn, ninst, x1, q0, vex.v, 16, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            if (v0 != q0)
                VMV_V_V(v0, q0);
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW32);
            VFMV_S_F(v0, d1);
            simd_store_reg(dyn, ninst, x1, v0, gd, 16, VECTOR_SEW32);
            break;
        case 0x2A:
            INST_NAME("VCVTSI2SD Gx, Vx, Ed");
            nextop = F8;
            q0 = VREG(vex.v);
            simd_load_reg(dyn, ninst, x1, q0, vex.v, 16, VECTOR_SEW64);
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                if (rex.w)
                    LD(x4, ed, fixedaddress);
                else
                    LW(x4, ed, fixedaddress);
                ed = x4;
            }
            u8 = simd_set_rounding(dyn, ninst, x5, x6);
            d0 = fpu_get_scratch(dyn);
            if (rex.w)
                FCVTDL(d0, ed, RD_DYN);
            else
                FCVTDW(d0, ed, RD_DYN);
            x87_restoreround(dyn, ninst, u8);
            GETGY_empty_vector(v0);
            if (v0 != q0) {
                simd_set_width(dyn, ninst, x1, VECTOR_SEW64, 16);
                VMV_V_V(v0, q0);
            }
            SET_SCALAR_FP_WIDTH(x1, VECTOR_SEW64);
            VFMV_S_F(v0, d0);
            simd_store_reg(dyn, ninst, x1, v0, gd, 16, VECTOR_SEW64);
            break;
        case 0x2C:
            INST_NAME("VCVTTSD2SI Gd, Ex");
            nextop = F8;
            GETGDd;
            d0 = fpu_get_scratch(dyn);
            if (MODREG) {
                q0 = VREG((nextop & 7) + (rex.b << 3));
                simd_load_reg(dyn, ninst, x1, q0, (nextop & 7) + (rex.b << 3), 16, VECTOR_SEW64);
                VFMV_F_S(d0, q0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                FLD(d0, ed, fixedaddress);
            }
            if (BOX64ENV(dynarec_fastround)) {
                if (rex.w)
                    FCVTLD(gd, d0, RD_RTZ);
                else
                    FCVTWD(gd, d0, RD_RTZ);
                if (!rex.w) ZEROUP(gd);
            } else {
                FSFLAGSI(0); // reset all bits
                if (rex.w)
                    FCVTLD(gd, d0, RD_RTZ);
                else
                    FCVTWD(gd, d0, RD_RTZ);
                if (!rex.w) ZEROUP(gd);
                FRFLAGS(x5); // get back FPSR to check the invalid bit
                ANDI(x5, x5, (1 << FR_NV) | (1 << FR_OF));
                BEQZ_MARK(x5);
                emit_mxcsr_raise_invalid(x4);
                if (rex.w)
                    MOV64x(gd, 0x8000000000000000LL);
                else
                    MOV32w(gd, 0x80000000);
                MARK;
            }
            break;
        case 0x2D:
            INST_NAME("VCVTSD2SI Gd, Ex");
            nextop = F8;
            GETGDd;
            d0 = fpu_get_scratch(dyn);
            if (MODREG) {
                q0 = VREG((nextop & 7) + (rex.b << 3));
                simd_load_reg(dyn, ninst, x1, q0, (nextop & 7) + (rex.b << 3), 16, VECTOR_SEW64);
                VFMV_F_S(d0, q0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                FLD(d0, ed, fixedaddress);
            }
            if (BOX64ENV(dynarec_fastround)) {
                u8 = simd_set_rounding(dyn, ninst, x2, x3);
                FCVTLDxw(gd, d0, RD_DYN);
                if (!rex.w) ZEROUP(gd);
                x87_restoreround(dyn, ninst, u8);
            } else {
                FSFLAGSI(0); // reset all bits
                u8 = simd_set_rounding(dyn, ninst, x6, x4);
                FCVTLDxw(gd, d0, RD_DYN);
                x87_restoreround(dyn, ninst, u8);
                if (!rex.w) ZEROUP(gd);
                FRFLAGS(x5); // get back FPSR to check the invalid bit
                ANDI(x5, x5, (1 << FR_NV) | (1 << FR_OF));
                BEQZ_MARK(x5);
                emit_mxcsr_raise_invalid(x4);
                if (rex.w)
                    MOV64x(gd, 0x8000000000000000LL);
                else
                    MOV32w(gd, 0x80000000);
                MARK;
            }
            break;
        case 0x70:
            INST_NAME("VPSHUFLW Gx, Ex, Ib");
            nextop = F8;
            GETEY_vector(q1, 1, VECTOR_SEW16);
            GETGY_empty_vector(v0);
            u8 = F8;
            q0 = vpu_get_scratch(dyn);
            q2 = vpu_get_scratch(dyn);
            MOV64xs(x4, (uint64_t)(u8 & 3) | ((uint64_t)((u8 >> 2) & 3) << 16) | ((uint64_t)((u8 >> 4) & 3) << 32) | ((uint64_t)((u8 >> 6) & 3) << 48), x3);
            rvv_set_vl_for_width(dyn, ninst, x1, VECTOR_SEW64, VECTOR_LMUL1, 16);
            VMV_V_X(q0, x4);
            rvv_set_vl_for_width(dyn, ninst, x1, VECTOR_SEW16, VECTOR_LMUL1, 16);
            VID_V(q2, VECTOR_UNMASKED);
            VID_V(VMASK, VECTOR_UNMASKED);
            VMSGT_VI(VMASK, VMASK, 3, VECTOR_UNMASKED);
            VMERGE_VVM(q0, q0, q2);
            if (vex.l) {
                q3 = vpu_get_scratch(dyn);
                VMV_V_V(q3, q0);
                rvv_set_vl_for_width(dyn, ninst, x1, VECTOR_SEW16, VECTOR_LMUL1, 32);
                VSLIDEUP_VI_SAFE(q0, q3, 8, VECTOR_UNMASKED);
                VID_V(VMASK, VECTOR_UNMASKED);
                VMSGT_VI(VMASK, VMASK, 7, VECTOR_UNMASKED);
                VOR_VI(q0, q0, 8, VECTOR_MASKED);
            }
            simd_set_width(dyn, ninst, x1, VECTOR_SEW16, 16 << vex.l);
            VRGATHER_VV_SAFE(v0, q1, q0, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0x12:
            INST_NAME("VMOVDDUP Gx, Ex");
            nextop = F8;
            GETEY_vector_w(v1, 0, VECTOR_SEW64, vex.l ? 32 : 8);
            GETGY_empty_vector(v0);
            v2 = vpu_get_scratch(dyn);
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW64);
            VID_V(v2, VECTOR_UNMASKED);
            VAND_VI(v2, v2, 0x1e, VECTOR_UNMASKED);
            VRGATHER_VV_SAFE(v0, v1, v2, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x51:
            INST_NAME("VSQRTSD Gx, Vx, Ex");
            nextop = F8;
            AVX_SCALAR_OP_PROLOG(VECTOR_SEW64);
            simd_scalar_arith(dyn, ninst, x1, v0, q1, q1, VECTOR_SEW64, 0x51);
            AVX_SCALAR_OP_STORE(VECTOR_SEW64);
            break;
        case 0xF0:
            INST_NAME("VLDDQU Gx, Ex");
            nextop = F8;
            GETEY_vector(v1, 0, VECTOR_SEW8);
            GETGY_empty_vector(v0);
            VMV_V_V(v0, v1);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0xC2:
            INST_NAME("VCMPSD Gx, Vx, Ex, Ib");
            nextop = F8;
            AVX_SCALAR_CMP(VECTOR_SEW64);
            break;
        case 0xD0:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("VADDSUBPS Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW32);
            q2 = vpu_get_scratch(dyn);
            VID_V(q2, VECTOR_UNMASKED);
            VAND_VI(q2, q2, 1, VECTOR_UNMASKED);
            VMSEQ_VX(VMASK, q2, xZR, VECTOR_UNMASKED);
            VFSUB_VV(v0, q0, q1, VECTOR_MASKED);
            VXOR_VI(VMASK, VMASK, 0x1f, VECTOR_UNMASKED);
            VFADD_VV(v0, q0, q1, VECTOR_MASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x7C:
        case 0x7D:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            if (opcode == 0x7C) {
                INST_NAME("VHADDPS Gx, Vx, Ex");
            } else {
                INST_NAME("VHSUBPS Gx, Vx, Ex");
            }
            nextop = F8;
            GETGY_empty_VYEY_vector(v0, q0, q1, VECTOR_SEW32);
            q2 = vpu_get_scratch(dyn);
            q3 = vpu_get_scratch(dyn);
            q4 = vpu_get_scratch(dyn);
            q5 = vpu_get_scratch(dyn);
            q6 = vpu_get_scratch(dyn);
            VSLIDEDOWN_VI(q2, q0, 1, VECTOR_UNMASKED);
            if (opcode == 0x7C)
                VFADD_VV(q2, q0, q2, VECTOR_UNMASKED);
            else
                VFSUB_VV(q2, q0, q2, VECTOR_UNMASKED);
            VSLIDEDOWN_VI(q3, q1, 1, VECTOR_UNMASKED);
            if (opcode == 0x7C)
                VFADD_VV(q3, q1, q3, VECTOR_UNMASKED);
            else
                VFSUB_VV(q3, q1, q3, VECTOR_UNMASKED);
            rvv_set_vl_for_width(dyn, ninst, x1, VECTOR_SEW16, VECTOR_LMUL1, 16);
            VID_V(q4, VECTOR_UNMASKED);
            VAND_VI(q5, q4, 1, VECTOR_UNMASKED);
            VSLL_VI(q5, q5, 1, VECTOR_UNMASKED);
            VAND_VI(q4, q4, 4, VECTOR_UNMASKED);
            VOR_VV(q4, q4, q5, VECTOR_UNMASKED);
            simd_set_width(dyn, ninst, x1, VECTOR_SEW32, 16 << vex.l);
            VRGATHEREI16_VV(q5, q2, q4, VECTOR_UNMASKED);
            VRGATHEREI16_VV(q6, q3, q4, VECTOR_UNMASKED);
            VECTOR_LOAD_VMASK(vex.l ? 0xcc : 0x0c, x4);
            VMERGE_VVM(v0, q5, q6);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0xE6:
            INST_NAME("VCVTPD2DQ Gx, Ex");
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
                u8 = simd_set_rounding(dyn, ninst, x6, x4);
                VFNCVT_X_F_W(q2, d0, VECTOR_UNMASKED);
                x87_restoreround(dyn, ninst, u8);
                VMV_V_V(v0, q2);
            } else {
                FSFLAGSI(0);
                u8 = simd_set_rounding(dyn, ninst, x6, x4);
                VFCVT_X_F_V(d0, d0, VECTOR_UNMASKED);
                x87_restoreround(dyn, ninst, u8);
                q3 = vpu_get_scratch(dyn);
                MOV64x(x5, (int64_t)INT32_MIN);
                VMSLT_VX(q3, d0, x5, VECTOR_UNMASKED);
                MOV64x(x5, (int64_t)INT32_MAX);
                VMSGT_VX(VMASK, d0, x5, VECTOR_UNMASKED);
                VMOR_MM(q3, q3, VMASK);
                simd_set_width(dyn, ninst, x1, VECTOR_SEW32, 16);
                VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
                rvv_set_vl_for_width(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL1, (vex.l ? 16 : 8));
                VNSRL_WI(q2, d0, 0, VECTOR_UNMASKED);
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
        default:
            DEFAULT_VECTOR;
    }
    return addr;
}
