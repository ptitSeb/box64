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

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_functions.h"
#include "../dynarec_helper.h"

uintptr_t dynarec64_F20F_vector(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop;
    uint8_t gd, ed;
    uint8_t wb1, wback, wb2, gback;
    uint8_t u8;
    uint64_t u64, j64;
    int v0, v1;
    int q0, q1, q2, q3, q4, q5;
    int d0, d1;
    int s0, s1;
    uint64_t tmp64u0, tmp64u1;
    int64_t fixedaddress, gdoffset;
    int unscaled;

    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(q2);
    MAYUSE(q3);
    MAYUSE(q4);
    MAYUSE(q5);
    MAYUSE(v0);
    MAYUSE(v1);

    switch (opcode) {
        case 0x10:
            INST_NAME("MOVSD Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG) {
                v1 = simd_get(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 8, VECTOR_SEW64, 0);
                v0 = simd_write(dyn, ninst, x1, gd, 8, VECTOR_SEW64);
                if (v0 != v1) VMV_V_V(v0, v1);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                v0 = simd_write(dyn, ninst, x1, gd, 8, VECTOR_SEW64);
                VLE_V(v0, ed, VECTOR_SEW64, VECTOR_UNMASKED, VECTOR_NFIELD1);
                simd_commit(dyn, ninst, gd, SIMD_LO64, SIMD_HI64);
            }
            break;
        case 0x11:
            INST_NAME("MOVSD Ex, Gx");
            nextop = F8;
            GETG;
            v0 = simd_get(dyn, ninst, x1, gd, 8, VECTOR_SEW64, 0);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                v1 = simd_write(dyn, ninst, x1, ed, 8, VECTOR_SEW64);
                if (v1 != v0) VMV_V_V(v1, v0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                VSE_V(v0, ed, VECTOR_SEW64, VECTOR_UNMASKED, VECTOR_NFIELD1);
                SMWRITE2();
            }
            break;
        case 0x12:
            INST_NAME("MOVDDUP Gx, Ex");
            nextop = F8;
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                v1 = simd_access(dyn, ninst, x1, ed, SIMD_LO64, 0);
                GETGX_empty_vector(v0);
                SET_ELEMENT_WIDTH(x1, VECTOR_SEW64);
                int duplicate = v0 == v1 ? vpu_get_scratch(dyn) : v0;
                VRGATHER_VI(duplicate, v1, 0, VECTOR_UNMASKED);
                if (duplicate != v0) VMV_V_V(v0, duplicate);
            } else {
                SMREAD();
                GETGX_empty_vector(v0);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                LD(x4, ed, fixedaddress);
                SET_ELEMENT_WIDTH(x1, VECTOR_SEW64);
                VMV_V_X(v0, x4);
            }
            break;
        case 0x2A:
            INST_NAME("CVTSI2SD Gx, Ed");
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
                FCVTDL(v0, ed, BOX64ENV(dynarec_fastround) <= 1 ? RD_DYN : RD_RNE);
            } else {
                FCVTDW(v0, ed, BOX64ENV(dynarec_fastround) <= 1 ? RD_DYN : RD_RNE);
                SET_ELEMENT_WIDTH(x3, VECTOR_SEW64);
            }
            if (cpuext.xtheadvector) {
                v1 = vpu_get_scratch(dyn);
                VFMV_S_F(v1, v0);
                VECTOR_LOAD_VMASK(0b01, x4);
                VMERGE_VVM(v0, v0, v1);
            } else {
                FMVXD(x4, v0);
                VMV_S_X(v0, x4);
            }
            break;
        case 0x2C:
            INST_NAME("CVTTSD2SI Gd, Ex");
            nextop = F8;
            GETGDd;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64);
            if (MODREG) {
                v0 = simd_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, dyn->vector_eew);
            } else {
                SMREAD();
                v0 = vpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                LD(x4, ed, fixedaddress);
                VMV_S_X(v0, x4);
            }
            if (BOX64ENV(dynarec_fastround)) {
                VFMV_F_S(v0, v0);
                FCVTLDxw(gd, v0, RD_RTZ);
                if (!rex.w) ZEROUP(gd);
            } else {
                VFMV_F_S(v0, v0);
                FSFLAGSI(0); // // reset all bits
                FCVTLDxw(gd, v0, RD_RTZ);
                if (!rex.w) ZEROUP(gd);
                FRFLAGS(x5); // get back FPSR to check the IOC bit
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
            INST_NAME("CVTSD2SI Gd, Ex");
            nextop = F8;
            GETGDd;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64);
            if (MODREG) {
                v0 = simd_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, dyn->vector_eew);
            } else {
                SMREAD();
                v0 = vpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                LD(x4, ed, fixedaddress);
                VMV_S_X(v0, x4);
            }
            if (BOX64ENV(dynarec_fastround)) {
                VFMV_F_S(v0, v0);
                FCVTLDxw(gd, v0, RD_DYN);
                if (!rex.w) ZEROUP(gd);
            } else {
                VFMV_F_S(v0, v0);
                FSFLAGSI(0); // // reset all bits
                FCVTLDxw(gd, v0, RD_DYN);
                if (!rex.w) ZEROUP(gd);
                FRFLAGS(x5); // get back FPSR to check the IOC bit
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
            INST_NAME("SQRTSD Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG) {
                v1 = simd_get(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 8, VECTOR_SEW64, 0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                v1 = vpu_get_scratch(dyn);
                rvv_set_config(dyn, ninst, x1, VECTOR_SEW64, VECTOR_LMUL1, 1);
                VLE_V(v1, ed, VECTOR_SEW64, VECTOR_UNMASKED, VECTOR_NFIELD1);
            }
            v0 = simd_write(dyn, ninst, x1, gd, 8, VECTOR_SEW64);
            simd_scalar_arith(dyn, ninst, x1, v0, v1, v1, VECTOR_SEW64, 0x51);
            break;
        case 0x58:
            INST_NAME("ADDSD Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG) {
                v1 = simd_get(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 8, VECTOR_SEW64, 0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                v1 = vpu_get_scratch(dyn);
                rvv_set_config(dyn, ninst, x1, VECTOR_SEW64, VECTOR_LMUL1, 1);
                VLE_V(v1, ed, VECTOR_SEW64, VECTOR_UNMASKED, VECTOR_NFIELD1);
            }
            v0 = simd_get(dyn, ninst, x1, gd, 8, VECTOR_SEW64, 1);
            simd_scalar_arith(dyn, ninst, x1, v0, v0, v1, VECTOR_SEW64, 0x58);
            break;
        case 0x59:
            INST_NAME("MULSD Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG) {
                v1 = simd_get(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 8, VECTOR_SEW64, 0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                v1 = vpu_get_scratch(dyn);
                rvv_set_config(dyn, ninst, x1, VECTOR_SEW64, VECTOR_LMUL1, 1);
                VLE_V(v1, ed, VECTOR_SEW64, VECTOR_UNMASKED, VECTOR_NFIELD1);
            }
            v0 = simd_get(dyn, ninst, x1, gd, 8, VECTOR_SEW64, 1);
            simd_scalar_arith(dyn, ninst, x1, v0, v0, v1, VECTOR_SEW64, 0x59);
            break;
        case 0x5A:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("CVTSD2SS Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64);
            if (MODREG) {
                GETGX_vector(v0, 1, VECTOR_SEW64);
                v1 = simd_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW64);
            } else {
                SMREAD();
                v1 = vpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                LD(x4, ed, fixedaddress);
                VMV_S_X(v1, x4);
                GETGX_vector(v0, 1, VECTOR_SEW64);
            }
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32);
            VECTOR_LOAD_VMASK(0b0001, x4);
            if (v1 & 1 || v0 == v1 + 1) {
                d1 = vpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                VMV_V_V(d1, v1);
                if (cpuext.xtheadvector) {
                    d0 = vpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                    VFNCVT_F_F_W(d0, d1, VECTOR_MASKED);
                    VMERGE_VVM(v0, v0, d0); // implies VMASK
                } else {
                    VFNCVT_F_F_W(v0, d1, VECTOR_MASKED);
                }
            } else {
                if (cpuext.xtheadvector) {
                    d0 = vpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                    VFNCVT_F_F_W(d0, v1, VECTOR_MASKED);
                    VMERGE_VVM(v0, v0, d0); // implies VMASK
                } else {
                    VFNCVT_F_F_W(v0, v1, VECTOR_MASKED);
                }
            }
            break;
        case 0x5C:
            INST_NAME("SUBSD Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG) {
                v1 = simd_get(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 8, VECTOR_SEW64, 0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                v1 = vpu_get_scratch(dyn);
                rvv_set_config(dyn, ninst, x1, VECTOR_SEW64, VECTOR_LMUL1, 1);
                VLE_V(v1, ed, VECTOR_SEW64, VECTOR_UNMASKED, VECTOR_NFIELD1);
            }
            v0 = simd_get(dyn, ninst, x1, gd, 8, VECTOR_SEW64, 1);
            simd_scalar_arith(dyn, ninst, x1, v0, v0, v1, VECTOR_SEW64, 0x5c);
            break;
        case 0x5D:
            INST_NAME("MINSD Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG) {
                v1 = simd_get(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 8, VECTOR_SEW64, 0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                v1 = vpu_get_scratch(dyn);
                rvv_set_config(dyn, ninst, x1, VECTOR_SEW64, VECTOR_LMUL1, 1);
                VLE_V(v1, ed, VECTOR_SEW64, VECTOR_UNMASKED, VECTOR_NFIELD1);
            }
            v0 = simd_get(dyn, ninst, x1, gd, 8, VECTOR_SEW64, 1);
            VMFLT_VV(VMASK, v0, v1, VECTOR_UNMASKED);
            VMERGE_VVM(v0, v1, v0);
            break;
        case 0x5E:
            INST_NAME("DIVSD Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG) {
                v1 = simd_get(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 8, VECTOR_SEW64, 0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                v1 = vpu_get_scratch(dyn);
                rvv_set_config(dyn, ninst, x1, VECTOR_SEW64, VECTOR_LMUL1, 1);
                VLE_V(v1, ed, VECTOR_SEW64, VECTOR_UNMASKED, VECTOR_NFIELD1);
            }
            v0 = simd_get(dyn, ninst, x1, gd, 8, VECTOR_SEW64, 1);
            simd_scalar_arith(dyn, ninst, x1, v0, v0, v1, VECTOR_SEW64, 0x5e);
            break;
        case 0x5F:
            INST_NAME("MAXSD Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG) {
                v1 = simd_get(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 8, VECTOR_SEW64, 0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                v1 = vpu_get_scratch(dyn);
                rvv_set_config(dyn, ninst, x1, VECTOR_SEW64, VECTOR_LMUL1, 1);
                VLE_V(v1, ed, VECTOR_SEW64, VECTOR_UNMASKED, VECTOR_NFIELD1);
            }
            v0 = simd_get(dyn, ninst, x1, gd, 8, VECTOR_SEW64, 1);
            VMFLT_VV(VMASK, v1, v0, VECTOR_UNMASKED);
            VMERGE_VVM(v0, v1, v0);
            break;
        case 0x70: {
            INST_NAME("PSHUFLW Gx, Ex, Ib");
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
                uint64_t lower_indices = (control & 3) | ((uint64_t)((control >> 2) & 3) << 16) | ((uint64_t)((control >> 4) & 3) << 32) | ((uint64_t)(control >> 6) << 48);
                SET_ELEMENT_WIDTH(x1, VECTOR_SEW64);
                MOV64xs(x5, 0x0007000600050004ULL, x3);
                VMV_S_X(upper_indices, x5);
                MOV64xs(x5, lower_indices, x3);
                VSLIDE1UP_VX(indices, upper_indices, x5, VECTOR_UNMASKED);
                SET_ELEMENT_WIDTH(x1, VECTOR_SEW16);
            } else {
                uint32_t selectors = (control & 3) | (((control >> 2) & 3) << 8) | (((control >> 4) & 3) << 16) | ((control >> 6) << 24);
                uint64_t packed = 0x0706050400000000ULL | selectors;
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
        case 0x7C:
        case 0x7D: {
            if (opcode == 0x7C && !BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME(opcode == 0x7C ? "HADDPS Gx, Ex" : "HSUBPS Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32);
            GETGX_vector(q0, 1, VECTOR_SEW32);
            GETEX_vector(q1, 0, 0, VECTOR_SEW32);
            int grouped = vpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            int first_terms = vpu_get_scratch(dyn);
            int second_terms = vpu_get_scratch(dyn);
            if (cpuext.vlen >= 32) {
                simd_set_width(dyn, ninst, x1, VECTOR_SEW64, 32);
                VMV_V_V(grouped, q0);
                VSLIDEUP_VI(grouped, q1, 2, VECTOR_UNMASKED);
            } else {
                VMV_V_V(grouped, q0);
                int upper_pairs = q1;
                if (q1 & 1) {
                    upper_pairs = vpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                    VMV_V_V(upper_pairs, q1);
                }
                rvv_set_vl_for_width(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL2, 32);
                VSLIDEUP_VI(grouped, upper_pairs, 4, VECTOR_UNMASKED);
            }
            rvv_set_vl_for_width(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL1, 16);
            ADDI(x4, xZR, 32);
            VNSRL_WI(first_terms, grouped, 0, VECTOR_UNMASKED);
            VNSRL_WX(second_terms, grouped, x4, VECTOR_UNMASKED);
            if (opcode == 0x7C)
                VFADD_VV(q0, first_terms, second_terms, VECTOR_UNMASKED);
            else
                VFSUB_VV(q0, first_terms, second_terms, VECTOR_UNMASKED);
            if (!BOX64ENV(dynarec_fastnan)) {
                MOV32w(x5, 0x80000000);
                VMFEQ_VV(VMASK, q0, q0, VECTOR_UNMASKED);
                VMNAND_MM(VMASK, VMASK, VMASK);
                VOR_VX(q0, q0, x5, VECTOR_MASKED);
                MOV32w(x5, 0x00400000);
                VMFNE_VV(VMASK, second_terms, second_terms, VECTOR_UNMASKED);
                VOR_VX(second_terms, second_terms, x5, VECTOR_UNMASKED);
                VMERGE_VVM(q0, q0, second_terms);
                VMFNE_VV(VMASK, first_terms, first_terms, VECTOR_UNMASKED);
                VOR_VX(first_terms, first_terms, x5, VECTOR_UNMASKED);
                VMERGE_VVM(q0, q0, first_terms);
            }
            break;
        }
        case 0xC2:
            INST_NAME("CMPSD Gx, Ex, Ib");
            nextop = F8;
            gd = ((nextop >> 3) & 7) + (rex.r << 3);
            d0 = simd_get(dyn, ninst, x1, gd, 8, VECTOR_SEW64, 0);
            if (MODREG) {
                d1 = simd_get(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 8, VECTOR_SEW64, 0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 1);
                d1 = vpu_get_scratch(dyn);
                VLE_V(d1, ed, VECTOR_SEW64, VECTOR_UNMASKED, VECTOR_NFIELD1);
            }
            u8 = F8;
            v0 = simd_write(dyn, ninst, x1, gd, 8, VECTOR_SEW64);
            simd_compare(dyn, ninst, v0, d0, d1, u8 & 7);
            break;
        case 0xE6: {
            INST_NAME("CVTPD2DQ Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64);
            GETEX_vector(v1, 0, 0, VECTOR_SEW64);
            GETGX_empty_vector(v0);
            if (!cpuext.xtheadvector) {
                if (BOX64ENV(dynarec_fastround)) {
                    rvv_set_vl_for_width(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMULF2, 8);
                    VFNCVT_X_F_W(v0, v1, VECTOR_UNMASKED);
                } else {
                    int rounded = vpu_get_scratch(dyn);
                    FSFLAGSI(0);
                    VFCVT_X_F_V(rounded, v1, VECTOR_UNMASKED);
                    MOV64x(x4, (int64_t)INT32_MIN);
                    VSUB_VX(v0, rounded, x4, VECTOR_UNMASKED);
                    SRLI(x5, x4, 32);
                    VMSGTU_VX(VMASK, v0, x5, VECTOR_UNMASKED);
                    rvv_set_vl_for_width(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMULF2, 8);
                    VNSRL_WI(v0, rounded, 0, VECTOR_UNMASKED);
                    VMERGE_VXM(v0, v0, x4);
                    VFIRST_M(x4, VMASK, VECTOR_UNMASKED);
                    BLT_MARK(x4, xZR);
                    emit_mxcsr_raise_invalid(x5);
                    if (BOX64ENV(sse_flushto0)) CSRRSI(xZR, 1 << FR_NV, 0x001);
                    MARK;
                }
                simd_commit(dyn, ninst, gd, SIMD_LO64, SIMD_HI64);
                break;
            }
            if (v1 & 1) {
                d1 = vpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                VMV_V_V(d1, v1);
            } else {
                d1 = v1;
            }
            if (!BOX64ENV(dynarec_fastround)) FSFLAGSI(0);
            d0 = vpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            if (BOX64ENV(dynarec_fastround)) {
                rvv_set_vl_for_width(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL1, 8);
                VFNCVT_X_F_W(d0, d1, VECTOR_UNMASKED);
                q1 = d0;
            } else {
                VFCVT_X_F_V(d0, d1, VECTOR_UNMASKED);
                q0 = vpu_get_scratch(dyn);
                MOV64x(x4, (int64_t)INT32_MIN);
                VMSLT_VX(q0, d0, x4, VECTOR_UNMASKED);
                MOV64x(x4, (int64_t)INT32_MAX);
                VMSGT_VX(VMASK, d0, x4, VECTOR_UNMASKED);
                VMOR_MM(q0, q0, VMASK);
                rvv_set_vl_for_width(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL1, 8);
                q1 = vpu_get_scratch(dyn);
                VNSRL_WI(q1, d0, 0, VECTOR_UNMASKED);
                MOV32w(x4, 0x80000000);
                VMV_V_X(d0, x4);
                VMAND_MM(VMASK, q0, q0);
                VMERGE_VVM(q1, q1, d0);
                FRFLAGS(x5);
                ANDI(x5, x5, (1 << FR_NV) | (1 << FR_OF));
                BEQ_MARK(x5, xZR);
                emit_mxcsr_raise_invalid(x4);
                MARK;
            }
            rvv_set_vl_for_width(dyn, ninst, x1, VECTOR_SEW64, VECTOR_LMUL1, 16);
            VMV_X_S(x4, q1);
            VMV_S_X(v0, x4);
            break;
        }
        case 0xF0:
            INST_NAME("LDDQU Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG) {
                SET_ELEMENT_WIDTH(x1, VECTOR_SEWANY);
                ed = (nextop & 7) + (rex.b << 3);
                v1 = simd_get_reg(dyn, ninst, x1, ed, 0, dyn->vector_eew);
                v0 = simd_get_reg_empty(dyn, ninst, x1, gd);
                VMV_V_V(v0, v1);
            } else {
                SET_ELEMENT_WIDTH(x1, VECTOR_SEW8); // unaligned!
                SMREAD();
                v0 = simd_get_reg_empty(dyn, ninst, x1, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                VLE_V(v0, ed, dyn->vector_eew, VECTOR_UNMASKED, VECTOR_NFIELD1);
            }
            break;
        case 0xD6:
            INST_NAME("MOVDQ2Q Gm, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64);
            GETGM_vector(v0);
            if (MODREG) {
                v1 = simd_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW64);
                VMV_X_S(x4, v1);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 1);
                LD(x4, ed, fixedaddress);
            }
            VMV_S_X(v0, x4);
            break;
        case 0xD0: {
            INST_NAME("ADDSUBPS Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32);
            GETGX_vector(v0, 1, VECTOR_SEW32);
            GETEX_vector(q0, 0, 0, VECTOR_SEW32);
            int sum = vpu_get_scratch(dyn);
            int ordered = vpu_get_scratch(dyn);
            int quiet_nan = vpu_get_scratch(dyn);
            VID_V(sum, VECTOR_UNMASKED);
            VXOR_VI(sum, sum, 1, VECTOR_UNMASKED);
            VSLL_VI(sum, sum, 31, VECTOR_UNMASKED);
            VXOR_VV(sum, q0, sum, VECTOR_UNMASKED);
            VFADD_VV(sum, v0, sum, VECTOR_UNMASKED);
            VMFEQ_VV(VMASK, v0, v0, VECTOR_UNMASKED);
            VMFEQ_VV(ordered, q0, q0, VECTOR_UNMASKED);
            VMAND_MM(ordered, ordered, VMASK);
            VMERGE_VVM(quiet_nan, v0, q0);
            MOV32w(x4, 0x00400000);
            VOR_VX(quiet_nan, quiet_nan, x4, VECTOR_UNMASKED);
            VMFNE_VV(VMASK, sum, sum, VECTOR_UNMASKED);
            VMAND_MM(VMASK, VMASK, ordered);
            VFSGNJN_VV(sum, sum, sum, VECTOR_MASKED);
            VMNAND_MM(VMASK, ordered, ordered);
            VMERGE_VVM(v0, sum, quiet_nan);
            break;
        }
        default: DEFAULT_VECTOR;
    }
    return addr;
}
