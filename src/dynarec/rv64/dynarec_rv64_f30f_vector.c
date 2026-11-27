// SPDX-License-Identifier: MIT
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
#include "bitutils.h"

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_functions.h"
#include "../dynarec_helper.h"

uintptr_t dynarec64_F30F_vector(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed;
    uint8_t wback, gback;
    uint64_t u64;
    int v0, v1;
    int q0, q1;
    int d0, d1;
    uint64_t tmp64u0, tmp64u1;
    int64_t fixedaddress, gdoffset;
    int unscaled;
    int64_t j64;

    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(v0);
    MAYUSE(v1);
    MAYUSE(j64);

    switch (opcode) {
        case 0x10:
            INST_NAME("MOVSS Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG) {
                v1 = simd_get(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 4, VECTOR_SEW32, 0);
                v0 = simd_write(dyn, ninst, x1, gd, 4, VECTOR_SEW32);
                if (v0 != v1) VMV_V_V(v0, v1);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                v0 = simd_write(dyn, ninst, x1, gd, 4, VECTOR_SEW32);
                VLE_V(v0, ed, VECTOR_SEW32, VECTOR_UNMASKED, VECTOR_NFIELD1);
                simd_commit(dyn, ninst, gd, SIMD_LO32, SIMD_HI32 | SIMD_HI64);
            }
            break;
        case 0x11:
            INST_NAME("MOVSS Ex, Gx");
            nextop = F8;
            GETG;
            v0 = simd_get(dyn, ninst, x1, gd, 4, VECTOR_SEW32, 0);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                v1 = simd_write(dyn, ninst, x1, ed, 4, VECTOR_SEW32);
                if (v1 != v0) VMV_V_V(v1, v0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                VSE_V(v0, ed, VECTOR_SEW32, VECTOR_UNMASKED, VECTOR_NFIELD1);
                SMWRITE2();
            }
            break;
        case 0x12:
        case 0x16: {
            int duplicate_high = opcode == 0x16;
            INST_NAME(duplicate_high ? "MOVSHDUP Gx, Ex" : "MOVSLDUP Gx, Ex");
            nextop = F8;
            if (cpuext.xtheadvector || !MODREG) {
                SET_ELEMENT_WIDTH(x1, VECTOR_SEW64);
                GETEX_vector(q1, 0, 0, VECTOR_SEW64);
                GETGX_empty_vector(q0);
                ADDI(x4, xZR, 32);
                v0 = vpu_get_scratch(dyn);
                if (duplicate_high) {
                    VSRL_VX(q0, q1, x4, VECTOR_UNMASKED);
                    VSLL_VX(v0, q0, x4, VECTOR_UNMASKED);
                } else {
                    VSLL_VX(q0, q1, x4, VECTOR_UNMASKED);
                    VSRL_VX(v0, q0, x4, VECTOR_UNMASKED);
                }
                VOR_VV(q0, q0, v0, VECTOR_UNMASKED);
            } else {
                GETEX_vector(q1, 0, 0, VECTOR_SEW32);
                GETGX_empty_vector(q0);
                if (q0 != q1) VMV_V_V(q0, q1);
                VECTOR_LOAD_VMASK(duplicate_high ? 0x5 : 0xa, x4);
                if (duplicate_high) {
                    VSLIDEDOWN_VI(q0, q1, 1, VECTOR_MASKED);
                } else {
                    VEC_SRC_KEEP(q1, q0);
                    VSLIDEUP_VI(q0, q1, 1, VECTOR_MASKED);
                }
            }
            break;
        }
        case 0x1E:
            return 0;
        case 0x2A:
            INST_NAME("CVTSI2SS Gx, Ed");
            nextop = F8;
            if (rex.w) {
                SET_ELEMENT_WIDTH(x3, VECTOR_SEW64);
                GETGX_vector(v0, 1, VECTOR_SEW64);
                GETED(0);
            } else {
                SET_ELEMENT_WIDTH(x3, VECTOR_SEW32);
                GETGX_vector(v0, 1, VECTOR_SEW32);
                GETED(0);
            }
            if (rex.w) {
                FCVTSL(v0, ed, BOX64ENV(dynarec_fastround) <= 1 ? RD_DYN : RD_RNE);
                SET_ELEMENT_WIDTH(x3, VECTOR_SEW32);
            } else {
                FCVTSW(v0, ed, BOX64ENV(dynarec_fastround) <= 1 ? RD_DYN : RD_RNE);
            }
            if (cpuext.xtheadvector) {
                v1 = vpu_get_scratch(dyn);
                VFMV_S_F(v1, v0);
                VECTOR_LOAD_VMASK(0b0001, x4);
                VMERGE_VVM(v0, v0, v1); // implies VMASK
            } else {
                VFMV_S_F(v0, v0);
            }
            break;
        case 0x2C:
            INST_NAME("CVTTSS2SI Gd, Ex");
            nextop = F8;
            GETGDd;
            SET_ELEMENT_WIDTH(x3, VECTOR_SEW32);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                d0 = simd_get_reg(dyn, ninst, x1, ed, 0, VECTOR_SEW32);
                VFMV_F_S(d0, d0);
            } else {
                GETEXSS(d0, 0);
            }
            if (!BOX64ENV(dynarec_fastround)) FSFLAGSI(0);
            FCVTSxw(gd, d0, RD_RTZ);
            if (!rex.w) ZEROUP(gd);
            if (!BOX64ENV(dynarec_fastround)) {
                FRFLAGS(x5);
                ANDI(x5, x5, (1 << FR_NV) | (1 << FR_OF));
                CBZ_NEXT(x5);
                if (rex.w) {
                    MOV64x(gd, 0x8000000000000000LL);
                } else {
                    MOV32w(gd, 0x80000000);
                }
            }
            break;
        case 0x2D:
            INST_NAME("CVTSS2SI Gd, Ex");
            nextop = F8;
            GETGDd;
            SET_ELEMENT_WIDTH(x3, VECTOR_SEW32);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                d0 = simd_get_reg(dyn, ninst, x1, ed, 0, VECTOR_SEW32);
                VFMV_F_S(d0, d0);
            } else {
                GETEXSS(d0, 0);
            }
            if (!BOX64ENV(dynarec_fastround)) FSFLAGSI(0);
            FCVTSxw(gd, d0, RD_DYN);
            if (!rex.w) ZEROUP(gd);
            if (!BOX64ENV(dynarec_fastround)) {
                FRFLAGS(x5);
                ANDI(x5, x5, (1 << FR_NV) | (1 << FR_OF));
                CBZ_NEXT(x5);
                if (rex.w) {
                    MOV64x(gd, 0x8000000000000000LL);
                } else {
                    MOV32w(gd, 0x80000000);
                }
            }
            break;
        case 0x38:
            return 0;
        case 0x51:
            INST_NAME("SQRTSS Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG) {
                v1 = simd_get(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 4, VECTOR_SEW32, 0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                v1 = vpu_get_scratch(dyn);
                rvv_set_config(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL1, 1);
                VLE_V(v1, ed, VECTOR_SEW32, VECTOR_UNMASKED, VECTOR_NFIELD1);
            }
            v0 = simd_write(dyn, ninst, x1, gd, 4, VECTOR_SEW32);
            simd_scalar_arith(dyn, ninst, x1, v0, v1, v1, VECTOR_SEW32, 0x51);
            break;
        case 0x52:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("RSQRTSS Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32);
            if (MODREG) {
                GETGX_vector(v0, 1, VECTOR_SEW32);
                v1 = simd_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW32);
            } else {
                SMREAD();
                v1 = vpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                LWU(x4, ed, fixedaddress);
                VMV_S_X(v1, x4);
                GETGX_vector(v0, 1, VECTOR_SEW32);
            }
            MOV32w(x4, 0x3F800000);
            int scalar_one_sqrt = fpu_get_scratch(dyn);
            FMVWX(scalar_one_sqrt, x4); // 1.0f
            SET_SCALAR_FP_WIDTH(x1, VECTOR_SEW32);
            VFSQRT_V(v0, v1, VECTOR_UNMASKED);
            VFRDIV_VF(v0, v0, scalar_one_sqrt, VECTOR_UNMASKED);
            break;
        case 0x53:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("RCPSS Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32);
            if (MODREG) {
                GETGX_vector(v0, 1, VECTOR_SEW32);
                v1 = simd_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW32);
            } else {
                SMREAD();
                v1 = vpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                LWU(x4, ed, fixedaddress);
                VMV_S_X(v1, x4);
                GETGX_vector(v0, 1, VECTOR_SEW32);
            }
            MOV32w(x4, 0x3F800000);
            int scalar_one_rcp = fpu_get_scratch(dyn);
            FMVWX(scalar_one_rcp, x4); // 1.0f
            VECTOR_LOAD_VMASK(0b0001, x4);
            VFRDIV_VF(v0, v1, scalar_one_rcp, VECTOR_MASKED);
            break;
        case 0x58:
            INST_NAME("ADDSS Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG) {
                v1 = simd_get(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 4, VECTOR_SEW32, 0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                v1 = vpu_get_scratch(dyn);
                rvv_set_config(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL1, 1);
                VLE_V(v1, ed, VECTOR_SEW32, VECTOR_UNMASKED, VECTOR_NFIELD1);
            }
            v0 = simd_get(dyn, ninst, x1, gd, 4, VECTOR_SEW32, 1);
            simd_scalar_arith(dyn, ninst, x1, v0, v0, v1, VECTOR_SEW32, 0x58);
            break;
        case 0x59:
            INST_NAME("MULSS Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG) {
                v1 = simd_get(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 4, VECTOR_SEW32, 0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                v1 = vpu_get_scratch(dyn);
                rvv_set_config(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL1, 1);
                VLE_V(v1, ed, VECTOR_SEW32, VECTOR_UNMASKED, VECTOR_NFIELD1);
            }
            v0 = simd_get(dyn, ninst, x1, gd, 4, VECTOR_SEW32, 1);
            simd_scalar_arith(dyn, ninst, x1, v0, v0, v1, VECTOR_SEW32, 0x59);
            break;
        case 0x5A:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("CVTSS2SD Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32);
            if (MODREG) {
                GETGX_vector(v0, 1, VECTOR_SEW32);
                v1 = simd_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW32);
            } else {
                SMREAD();
                v1 = vpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                LWU(x4, ed, fixedaddress);
                VMV_S_X(v1, x4);
                GETGX_vector(v0, 1, VECTOR_SEW32);
            }
            d0 = vpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            VECTOR_LOAD_VMASK(0b0001, x4);
            VFWCVT_F_F_V(d0, v1, VECTOR_MASKED);
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64);
            if (cpuext.xtheadvector) {
                VMERGE_VVM(v0, v0, d0); // implies VMASK
            } else {
                VMV_X_S(x4, d0);
                VMV_S_X(v0, x4);
            }
            break;
        case 0x5B:
            INST_NAME("CVTTPS2DQ Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32);
            GETEX_vector(v1, 0, 0, VECTOR_SEW32);
            GETGX_empty_vector(v0);
            if (!BOX64ENV(dynarec_fastround)) FSFLAGSI(0);
            if (cpuext.xtheadvector) {
                ADDI(x4, xZR, 1); // RTZ
                FSRM(x4, x4);
                VFCVT_X_F_V(v0, v1, VECTOR_UNMASKED);
                FSRM(xZR, x4);
            } else {
                VFCVT_RTZ_X_F_V(v0, v1, VECTOR_UNMASKED);
            }
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
            break;
        case 0x5C:
            INST_NAME("SUBSS Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG) {
                v1 = simd_get(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 4, VECTOR_SEW32, 0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                v1 = vpu_get_scratch(dyn);
                rvv_set_config(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL1, 1);
                VLE_V(v1, ed, VECTOR_SEW32, VECTOR_UNMASKED, VECTOR_NFIELD1);
            }
            v0 = simd_get(dyn, ninst, x1, gd, 4, VECTOR_SEW32, 1);
            simd_scalar_arith(dyn, ninst, x1, v0, v0, v1, VECTOR_SEW32, 0x5c);
            break;
        case 0x5D:
            INST_NAME("MINSS Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG) {
                v1 = simd_get(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 4, VECTOR_SEW32, 0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                v1 = vpu_get_scratch(dyn);
                rvv_set_config(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL1, 1);
                VLE_V(v1, ed, VECTOR_SEW32, VECTOR_UNMASKED, VECTOR_NFIELD1);
            }
            v0 = simd_get(dyn, ninst, x1, gd, 4, VECTOR_SEW32, 1);
            VMFLT_VV(VMASK, v0, v1, VECTOR_UNMASKED);
            VMERGE_VVM(v0, v1, v0);
            break;
        case 0x5E:
            INST_NAME("DIVSS Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG) {
                v1 = simd_get(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 4, VECTOR_SEW32, 0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                v1 = vpu_get_scratch(dyn);
                rvv_set_config(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL1, 1);
                VLE_V(v1, ed, VECTOR_SEW32, VECTOR_UNMASKED, VECTOR_NFIELD1);
            }
            v0 = simd_get(dyn, ninst, x1, gd, 4, VECTOR_SEW32, 1);
            simd_scalar_arith(dyn, ninst, x1, v0, v0, v1, VECTOR_SEW32, 0x5e);
            break;
        case 0x5F:
            INST_NAME("MAXSS Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG) {
                v1 = simd_get(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 4, VECTOR_SEW32, 0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                v1 = vpu_get_scratch(dyn);
                rvv_set_config(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL1, 1);
                VLE_V(v1, ed, VECTOR_SEW32, VECTOR_UNMASKED, VECTOR_NFIELD1);
            }
            v0 = simd_get(dyn, ninst, x1, gd, 4, VECTOR_SEW32, 1);
            VMFLT_VV(VMASK, v1, v0, VECTOR_UNMASKED);
            VMERGE_VVM(v0, v1, v0);
            break;
        case 0x6F:
            INST_NAME("MOVDQU Gx, Ex");
            nextop = F8;
            gd = ((nextop >> 3) & 7) + (rex.r << 3);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                simd_copy(dyn, ninst, x1, gd, ed, 16, 0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                v0 = simd_write(dyn, ninst, x1, gd, 16, VECTOR_SEW8);
                VLE_V(v0, ed, VECTOR_SEW8, VECTOR_UNMASKED, VECTOR_NFIELD1);
            }
            break;
        case 0x70: {
            INST_NAME("PSHUFHW Gx, Ex, Ib");
            nextop = F8;
            if (!cpuext.xtheadvector && MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                if (PK(0) == 0xe4) {
                    ++addr;
                    gd = ((nextop >> 3) & 7) + (rex.r << 3);
                    simd_copy(dyn, ninst, x1, gd, ed, 16, 0);
                    break;
                }
                v1 = simd_access(dyn, ninst, x1, ed, SIMD_XMM, 0);
            } else {
                SET_ELEMENT_WIDTH(x1, VECTOR_SEW16);
                GETEX_vector(v1, 0, 1, VECTOR_SEW16);
            }
            if (cpuext.xtheadvector) {
                GETGX_vector(v0, 1, VECTOR_SEW16);
            } else {
                GETGX_empty_vector(v0);
            }
            uint8_t control = F8;
            int indices = vpu_get_scratch(dyn);
            if (cpuext.xtheadvector) {
                int upper_indices = vpu_get_scratch(dyn);
                uint64_t upper_words = 0x0004000400040004ULL | (control & 3) | ((uint64_t)((control >> 2) & 3) << 16) | ((uint64_t)((control >> 4) & 3) << 32) | ((uint64_t)(control >> 6) << 48);
                SET_ELEMENT_WIDTH(x1, VECTOR_SEW64);
                MOV64xs(x5, upper_words, x3);
                VMV_S_X(upper_indices, x5);
                MOV64x(x5, 0x0003000200010000ULL);
                VSLIDE1UP_VX(indices, upper_indices, x5, VECTOR_UNMASKED);
                SET_ELEMENT_WIDTH(x1, VECTOR_SEW16);
            } else {
                uint32_t selectors = (control & 3) | (((control >> 2) & 3) << 8) | (((control >> 4) & 3) << 16) | ((control >> 6) << 24);
                uint64_t packed = ((uint64_t)(selectors | 0x04040404U) << 32) | 0x03020100U;
                int byte_indices = vpu_get_scratch(dyn);
                rvv_set_vl_for_width(dyn, ninst, x1, VECTOR_SEW64, VECTOR_LMUL1, 8);
                MOV64xs(x4, packed, x3);
                VMV_S_X(byte_indices, x4);
                SET_ELEMENT_WIDTH(x1, VECTOR_SEW16);
                VZEXT_VF2(indices, byte_indices, VECTOR_UNMASKED);
            }
            VRGATHER_VV_SAFE(v0, v1, indices, VECTOR_UNMASKED);
            break;
        }
        case 0x7E:
            INST_NAME("MOVQ Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64);
            if (MODREG) {
                GETEX_vector(v1, 0, 0, VECTOR_SEW64);
                GETGX_empty_vector(v0);
                VMV_X_S(x4, v1);
            } else {
                SMREAD();
                GETGX_empty_vector(v0);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                LD(x4, ed, fixedaddress);
            }
            if (!cpuext.xtheadvector) VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
            VMV_S_X(v0, x4);
            break;
        case 0x7F:
            INST_NAME("MOVDQU Ex, Gx");
            nextop = F8;
            gd = ((nextop >> 3) & 7) + (rex.r << 3);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                simd_copy(dyn, ninst, x1, ed, gd, 16, 0);
            } else {
                v0 = simd_get(dyn, ninst, x1, gd, 16, VECTOR_SEW8, 0);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                VSE_V(v0, ed, VECTOR_SEW8, VECTOR_UNMASKED, VECTOR_NFIELD1);
                SMWRITE2();
            }
            break;
        case 0xAE:
        case 0xB8:
        case 0xBC:
        case 0xBD:
            return 0;
        case 0xC2:
            INST_NAME("CMPSS Gx, Ex, Ib");
            nextop = F8;
            gd = ((nextop >> 3) & 7) + (rex.r << 3);
            d0 = simd_get(dyn, ninst, x1, gd, 4, VECTOR_SEW32, 0);
            if (MODREG) {
                d1 = simd_get(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 4, VECTOR_SEW32, 0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 1);
                d1 = vpu_get_scratch(dyn);
                VLE_V(d1, ed, VECTOR_SEW32, VECTOR_UNMASKED, VECTOR_NFIELD1);
            }
            u8 = F8;
            v0 = simd_write(dyn, ninst, x1, gd, 4, VECTOR_SEW32);
            simd_compare(dyn, ninst, v0, d0, d1, u8 & 7);
            break;
        case 0xE6:
            INST_NAME("CVTDQ2PD Gx, Ex");
            nextop = F8;
            if (!cpuext.xtheadvector) {
                if (MODREG) {
                    v1 = simd_access(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), SIMD_LO64, 0);
                } else {
                    SMREAD();
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 0, 0);
                    v1 = vpu_get_scratch(dyn);
                    rvv_set_vl_for_width(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMULF2, 8);
                    VLE32_V(v1, ed, VECTOR_UNMASKED, VECTOR_NFIELD1);
                }
                GETGX_empty_vector(v0);
                if (v1 == v0) {
                    int input = vpu_get_scratch(dyn);
                    VMV1R_V(input, v1);
                    v1 = input;
                }
                rvv_set_vl_for_width(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMULF2, 8);
                VFWCVT_F_X_V(v0, v1, VECTOR_UNMASKED);
                break;
            }
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64);
            if (MODREG) {
                v1 = simd_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW64);
                GETGX_empty_vector(v0);
            } else {
                SMREAD();
                v1 = vpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                LD(x4, ed, fixedaddress);
                VMV_S_X(v1, x4);
                GETGX_empty_vector(v0);
            }
            rvv_set_vl_for_width(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL1, 8);
            d0 = vpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            VFWCVT_F_X_V(d0, v1, VECTOR_UNMASKED);
            rvv_set_vl_for_width(dyn, ninst, x1, VECTOR_SEW64, VECTOR_LMUL1, 16);
            VMV_V_V(v0, d0);
            break;
        case 0xD6:
            INST_NAME("MOVQ2DQ Gx, Em");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64);
            GETEM_vector(q0, 0);
            GETGX_empty_vector(v0);
            VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64);
            VECTOR_LOAD_VMASK(0b01, x4);
            VMERGE_VVM(v0, v0, q0);
            break;
        default:
            DEFAULT_VECTOR;
    }
    return addr;
}
