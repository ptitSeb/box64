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

uintptr_t dynarec64_AVX_F3_0F_vector(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed;
    int v0, v1, v2;
    int q0, q1, q2, q3, d0, d1;
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
            INST_NAME("VMOVSS Gx, [Vx,] Ex");
            nextop = F8;
            d0 = fpu_get_scratch(dyn);
            if (MODREG) {
                q0 = fpu_get_scratch(dyn);
                avx_load_reg_vector(dyn, ninst, x1, q0, vex.v, 16, VECTOR_SEW32);
                q1 = fpu_get_scratch(dyn);
                avx_load_reg_vector(dyn, ninst, x1, q1, (nextop & 7) + (rex.b << 3), 16, VECTOR_SEW32);
                VFMV_F_S(d0, q1);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                FLW(d0, ed, fixedaddress);
            }
            GETGY_empty_vector(v0);
            if (MODREG) {
                if (v0 != q0)
                    VMV_V_V(v0, q0);
            } else {
                SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW32);
                VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
            }
            VFMV_S_F(v0, d0);
            avx_store_reg_vector(dyn, ninst, x1, v0, gd, 16, VECTOR_SEW32);
            break;
        case 0x11:
            INST_NAME("VMOVSS Ex, [Vx,] Gx");
            nextop = F8;
            gd = ((nextop & 0x38) >> 3) + (rex.r << 3);
            q0 = VREG(gd);
            avx_load_reg_vector(dyn, ninst, x1, q0, gd, 16, VECTOR_SEW32);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                q2 = fpu_get_scratch(dyn);
                avx_load_reg_vector(dyn, ninst, x1, q2, vex.v, 16, VECTOR_SEW32);
                VFMV_F_S(d0 = fpu_get_scratch(dyn), q0);
                VFMV_S_F(q2, d0);
                avx_store_reg_vector(dyn, ninst, x1, q2, ed, 16, VECTOR_SEW32);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                SET_SCALAR_FP_WIDTH(x1, VECTOR_SEW32);
                VSE32_V(q0, ed, VECTOR_UNMASKED, VECTOR_NFIELD1);
                SMWRITE2();
            }
            break;
        case 0x5A:
            INST_NAME("VCVTSS2SD Gx, Vx, Ex");
            nextop = F8;
            d0 = fpu_get_scratch(dyn);
            if (MODREG) {
                q1 = fpu_get_scratch(dyn);
                avx_load_reg_vector(dyn, ninst, x1, q1, (nextop & 7) + (rex.b << 3), 16, VECTOR_SEW32);
                VFMV_F_S(d0, q1);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                FLW(d0, ed, fixedaddress);
            }
            d1 = fpu_get_scratch(dyn);
            FCVTDS(d1, d0);
            if (!BOX64ENV(dynarec_fastnan)) {
                FEQS(x3, d0, d0);
                FMVXW(x4, d0);
                BNEZ_MARK(x3);
                SRLIW(x5, x4, 31);
                SLLI(x5, x5, 63);
                SLLI(x4, x4, 41);
                SRLI(x4, x4, 12);
                OR(x4, x4, x5);
                MOV64x(x5, 0x7ff8000000000000ULL);
                OR(x4, x4, x5);
                FMVDX(d1, x4);
                MARK;
            }
            q0 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q0, vex.v, 16, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            if (v0 != q0)
                VMV_V_V(v0, q0);
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW64);
            VFMV_S_F(v0, d1);
            avx_store_reg_vector(dyn, ninst, x1, v0, gd, 16, VECTOR_SEW64);
            break;
        case 0x7E:
            INST_NAME("VMOVQ Gx, Ex");
            nextop = F8;
            if (MODREG) {
                v1 = fpu_get_scratch(dyn);
                avx_load_reg_vector(dyn, ninst, x1, v1, (nextop & 7) + (rex.b << 3), 16, VECTOR_SEW64);
                SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW64);
                VMV_X_S(x5, v1);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                LD(x5, ed, fixedaddress);
            }
            GETGY_empty_vector(v0);
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW64);
            VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
            VMV_S_X(v0, x5);
            avx_store_reg_vector(dyn, ninst, x1, v0, gd, 16, VECTOR_SEW64);
            break;
        case 0x12:
            INST_NAME("VMOVSLDUP Gx, Ex");
            nextop = F8;
            GETEY_vector(v1, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            v2 = fpu_get_scratch(dyn);
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW32);
            VID_V(v2, VECTOR_UNMASKED);
            VAND_VI(v2, v2, 0x1e, VECTOR_UNMASKED);
            VRGATHER_VV_SAFE(v0, v1, v2, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x16:
            INST_NAME("VMOVSHDUP Gx, Ex");
            nextop = F8;
            GETEY_vector(v1, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            v2 = fpu_get_scratch(dyn);
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW32);
            VID_V(v2, VECTOR_UNMASKED);
            VOR_VI(v2, v2, 1, VECTOR_UNMASKED);
            VAND_VI(v2, v2, 0x1f, VECTOR_UNMASKED);
            VRGATHER_VV_SAFE(v0, v1, v2, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x6F:
            INST_NAME("VMOVDQU Gx, Ex");
            nextop = F8;
            if (!MODREG) {
                gd = ((nextop & 0x38) >> 3) + (rex.r << 3);
                v0 = sse_get_reg_empty_vector(dyn, ninst, x1, gd);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, x2, &fixedaddress, rex, NULL, 0, 0);
                avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW8, 16 << vex.l);
                VLE8_V(v0, ed, VECTOR_UNMASKED, VECTOR_NFIELD1);
                avx_store_reg_vector(dyn, ninst, x1, v0, gd, 16 << vex.l, VECTOR_SEW8);
            } else {
                GETEY_vector(v1, 0, VECTOR_SEW8);
                GETGY_empty_vector(v0);
                VMV_V_V(v0, v1);
                PUTGY_vector(v0, VECTOR_SEW8);
            }
            break;
        case 0x7F:
            INST_NAME("VMOVDQU Ex, Gx");
            nextop = F8;
            GETGY_vector(v0, VECTOR_SEW8);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                avx_store_reg_vector(dyn, ninst, x1, v0, ed, 16 << vex.l, VECTOR_SEW8);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW8);
                VSE8_V(v0, ed, VECTOR_UNMASKED, VECTOR_NFIELD1);
                SMWRITE2();
            }
            break;
        case 0x2A:
            INST_NAME("VCVTSI2SS Gx, Vx, Ed");
            nextop = F8;
            q0 = VREG(vex.v);
            avx_load_reg_vector(dyn, ninst, x1, q0, vex.v, 16, VECTOR_SEW32);
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
            LW(x5, xEmu, offsetof(x64emu_t, mxcsr));
            SRLI(x5, x5, 13);
            ANDI(x5, x5, 3);
            d0 = fpu_get_scratch(dyn);
            BEQZ_MARK(x5);
            ADDI(x6, xZR, 3);
            BEQ_MARK2(x5, x6);
            ADDI(x6, xZR, 2);
            BEQ_MARK3(x5, x6);
            if (rex.w)
                FCVTSL(d0, ed, RD_RDN);
            else
                FCVTSW(d0, ed, RD_RDN);
            B_MARK4_nocond;
            MARK;
            if (rex.w)
                FCVTSL(d0, ed, RD_RNE);
            else
                FCVTSW(d0, ed, RD_RNE);
            B_MARK4_nocond;
            MARK2;
            if (rex.w)
                FCVTSL(d0, ed, RD_RTZ);
            else
                FCVTSW(d0, ed, RD_RTZ);
            B_MARK4_nocond;
            MARK3;
            if (rex.w)
                FCVTSL(d0, ed, RD_RUP);
            else
                FCVTSW(d0, ed, RD_RUP);
            MARK4;
            GETGY_empty_vector(v0);
            if (v0 != q0) {
                avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW32, 16);
                VMV_V_V(v0, q0);
            }
            SET_SCALAR_FP_WIDTH(x1, VECTOR_SEW32);
            VFMV_S_F(v0, d0);
            avx_store_reg_vector(dyn, ninst, x1, v0, gd, 16, VECTOR_SEW32);
            break;
        case 0x2C:
            INST_NAME("VCVTTSS2SI Gd, Ex");
            nextop = F8;
            GETGDd;
            d0 = fpu_get_scratch(dyn);
            if (MODREG) {
                q0 = VREG((nextop & 7) + (rex.b << 3));
                avx_load_reg_vector(dyn, ninst, x1, q0, (nextop & 7) + (rex.b << 3), 16, VECTOR_SEW32);
                VFMV_F_S(d0, q0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                FLW(d0, ed, fixedaddress);
            }
            if (!BOX64ENV(dynarec_fastround)) FSFLAGSI(0);
            FCVTSxw(gd, d0, RD_RTZ);
            if (!rex.w) ZEROUP(gd);
            if (!BOX64ENV(dynarec_fastround)) {
                FRFLAGS(x5);
                ANDI(x5, x5, (1 << FR_NV) | (1 << FR_OF));
                CBZ_NEXT(x5);
                if (rex.w)
                    MOV64x(gd, 0x8000000000000000LL);
                else
                    MOV32w(gd, 0x80000000);
            }
            break;
        case 0x2D:
            INST_NAME("VCVTSS2SI Gd, Ex");
            nextop = F8;
            GETGDd;
            d0 = fpu_get_scratch(dyn);
            if (MODREG) {
                q0 = VREG((nextop & 7) + (rex.b << 3));
                avx_load_reg_vector(dyn, ninst, x1, q0, (nextop & 7) + (rex.b << 3), 16, VECTOR_SEW32);
                VFMV_F_S(d0, q0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                FLW(d0, ed, fixedaddress);
            }
            if (!BOX64ENV(dynarec_fastround)) FSFLAGSI(0);
            LW(x5, xEmu, offsetof(x64emu_t, mxcsr));
            SRLI(x5, x5, 13);
            ANDI(x5, x5, 3);
            BEQZ_MARK(x5);
            ADDI(x6, xZR, 3);
            BEQ_MARK2(x5, x6);
            ADDI(x6, xZR, 2);
            BEQ_MARK3(x5, x6);
            FCVTSxw(gd, d0, RD_RDN);
            B_MARK4_nocond;
            MARK;
            FCVTSxw(gd, d0, RD_RNE);
            B_MARK4_nocond;
            MARK2;
            FCVTSxw(gd, d0, RD_RTZ);
            B_MARK4_nocond;
            MARK3;
            FCVTSxw(gd, d0, RD_RUP);
            MARK4;
            if (!rex.w) ZEROUP(gd);
            if (!BOX64ENV(dynarec_fastround)) {
                FRFLAGS(x5);
                ANDI(x5, x5, (1 << FR_NV) | (1 << FR_OF));
                CBZ_NEXT(x5);
                if (rex.w)
                    MOV64x(gd, 0x8000000000000000LL);
                else
                    MOV32w(gd, 0x80000000);
            }
            break;
        case 0x51:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("VSQRTSS Gx, Vx, Ex");
            nextop = F8;
            AVX_SCALAR_OP_PROLOG(VECTOR_SEW32);
            VFSQRT_V(v0, q1, VECTOR_UNMASKED);
            AVX_SCALAR_OP_STORE(VECTOR_SEW32);
            break;
        case 0x58:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("VADDSS Gx, Vx, Ex");
            nextop = F8;
            AVX_SCALAR_OP_PROLOG(VECTOR_SEW32);
            VFADD_VV(v0, q0, q1, VECTOR_UNMASKED);
            AVX_SCALAR_OP_STORE(VECTOR_SEW32);
            break;
        case 0x59:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("VMULSS Gx, Vx, Ex");
            nextop = F8;
            AVX_SCALAR_OP_PROLOG(VECTOR_SEW32);
            VFMUL_VV(v0, q0, q1, VECTOR_UNMASKED);
            AVX_SCALAR_OP_STORE(VECTOR_SEW32);
            break;
        case 0x5B:
            INST_NAME("VCVTTPS2DQ Gx, Ex");
            nextop = F8;
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW32);
            GETEY_vector(q1, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            v1 = fpu_get_scratch(dyn);
            VFCVT_RTZ_X_F_V(v1, q1, VECTOR_UNMASKED);
            // out of range (>= 2^31) gives 0x80000000
            q2 = fpu_get_scratch(dyn);
            MOV32w(x4, 0x80000000);
            VMV_V_X(q2, x4);
            d0 = fpu_get_scratch(dyn);
            MOV32w(x4, 0x4f000000); // 2^31.0f
            FMVWX(d0, x4);
            VMFGE_VF(VMASK, q1, d0, VECTOR_UNMASKED);
            VMERGE_VVM(v1, v1, q2);
            // NaN gives 0x80000000 too
            VMFEQ_VV(VMASK, q1, q1, VECTOR_UNMASKED);
            VMERGE_VVM(v1, q2, v1);
            VMV_V_V(v0, v1);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x5C:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("VSUBSS Gx, Vx, Ex");
            nextop = F8;
            AVX_SCALAR_OP_PROLOG(VECTOR_SEW32);
            VFSUB_VV(v0, q0, q1, VECTOR_UNMASKED);
            AVX_SCALAR_OP_STORE(VECTOR_SEW32);
            break;
        case 0x5D:
            INST_NAME("VMINSS Gx, Vx, Ex");
            nextop = F8;
            AVX_SCALAR_OP_PROLOG(VECTOR_SEW32);
            VMFLT_VV(VMASK, q0, q1, VECTOR_UNMASKED);
            VMERGE_VVM(v0, q1, q0);
            AVX_SCALAR_OP_STORE(VECTOR_SEW32);
            break;
        case 0x5E:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("VDIVSS Gx, Vx, Ex");
            nextop = F8;
            AVX_SCALAR_OP_PROLOG(VECTOR_SEW32);
            VFDIV_VV(v0, q0, q1, VECTOR_UNMASKED);
            AVX_SCALAR_OP_STORE(VECTOR_SEW32);
            break;
        case 0x5F:
            INST_NAME("VMAXSS Gx, Vx, Ex");
            nextop = F8;
            AVX_SCALAR_OP_PROLOG(VECTOR_SEW32);
            VMFLT_VV(VMASK, q1, q0, VECTOR_UNMASKED);
            VMERGE_VVM(v0, q1, q0);
            AVX_SCALAR_OP_STORE(VECTOR_SEW32);
            break;
        case 0x70:
            INST_NAME("VPSHUFHW Gx, Ex, Ib");
            nextop = F8;
            GETEY_vector(q1, 1, VECTOR_SEW16);
            GETGY_empty_vector(v0);
            u8 = F8;
            q0 = fpu_get_scratch(dyn);
            q2 = fpu_get_scratch(dyn);
            MOV64xs(x4, (uint64_t)(4 + (u8 & 3)) | ((uint64_t)(4 + ((u8 >> 2) & 3)) << 16) | ((uint64_t)(4 + ((u8 >> 4) & 3)) << 32) | ((uint64_t)(4 + ((u8 >> 6) & 3)) << 48), x3);
            vector_vsetvli(dyn, ninst, x1, VECTOR_SEW64, VECTOR_LMUL1, 1);
            VMV_V_X(q0, x4);
            vector_vsetvli(dyn, ninst, x1, VECTOR_SEW16, VECTOR_LMUL1, 1);
            VID_V(q2, VECTOR_UNMASKED);
            VID_V(VMASK, VECTOR_UNMASKED);
            VMSGT_VI(VMASK, VMASK, 3, VECTOR_UNMASKED);
            VMERGE_VVM(q0, q2, q0);
            if (vex.l) {
                q3 = fpu_get_scratch(dyn);
                VMV_V_V(q3, q0);
                vector_vsetvli(dyn, ninst, x1, VECTOR_SEW16, VECTOR_LMUL1, 2);
                VSLIDEUP_VI_SAFE(q0, q3, 8, VECTOR_UNMASKED);
                VID_V(VMASK, VECTOR_UNMASKED);
                VMSGT_VI(VMASK, VMASK, 7, VECTOR_UNMASKED);
                VOR_VI(q0, q0, 8, VECTOR_MASKED);
            }
            avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW16, 16 << vex.l);
            VRGATHER_VV_SAFE(v0, q1, q0, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0xE6:
            INST_NAME("VCVTDQ2PD Gx, Ex");
            nextop = F8;
            v1 = fpu_get_scratch(dyn);
            if (MODREG) {
                avx_load_reg_vector(dyn, ninst, x1, v1, (nextop & 7) + (rex.b << 3), 16, VECTOR_SEW32);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                vector_vsetvli(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL1, vex.l ? 1 : 0.5);
                VLE32_V(v1, ed, VECTOR_UNMASKED, VECTOR_NFIELD1);
            }
            GETGY_empty_vector(v0);
            vector_vsetvli(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL1, vex.l ? 1 : 0.5);
            d0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            VFWCVT_F_X_V(d0, v1, VECTOR_UNMASKED);
            avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW64, 16 << vex.l);
            VMV_V_V(v0, d0);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0xC2:
            INST_NAME("VCMPSS Gx, Vx, Ex, Ib");
            nextop = F8;
            AVX_SCALAR_CMP(VECTOR_SEW32);
            break;
        default:
            DEFAULT_VECTOR;
    }
    return addr;
}
