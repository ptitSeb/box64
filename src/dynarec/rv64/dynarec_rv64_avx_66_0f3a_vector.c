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

uintptr_t dynarec64_AVX_66_0F3A_vector(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed;
    int v0, v1, v2;
    int q0, q1, q2, q3, q4, d0, d1, d2;
    int64_t fixedaddress;
    int64_t j64;
    rex_t rex = vex.rex;
    static const int8_t round_round[] = {RD_RNE, RD_RDN, RD_RUP, RD_RTZ};

    MAYUSE(v1);
    MAYUSE(v2);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(q2);
    MAYUSE(q3);
    MAYUSE(q4);
    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(d2);
    MAYUSE(u8);
    MAYUSE(j64);

    switch (opcode) {
        case 0x00:
        case 0x01:
            if (opcode)
                INST_NAME("VPERMPD Gx, Ex, Imm8");
            else
                INST_NAME("VPERMQ Gx, Ex, Imm8");
            nextop = F8;
            GETEY_vector(q0, 1, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            u8 = F8;
            q1 = vpu_get_scratch(dyn);
            q2 = vpu_get_scratch_lmul(dyn, vex.l ? VECTOR_LMUL1 : VECTOR_LMUL2);
            {
                uint64_t iota = (u8 & 3) | (((u8 >> 2) & 3) << 16);
                iota |= ((uint64_t)((u8 >> 4) & 3)) << 32;
                iota |= ((uint64_t)((u8 >> 6) & 3)) << 48;
                rvv_set_vl_for_width(dyn, ninst, x1, VECTOR_SEW64, VECTOR_LMUL1, 16 << vex.l);
                VECTOR_SPLAT_IMM_S(q2, iota, x4, x3);
            }
            if (v0 == q0 || v0 == q2) {
                VRGATHEREI16_VV(q1, q0, q2, VECTOR_UNMASKED);
                VMV_V_V(v0, q1);
            } else {
                VRGATHEREI16_VV(v0, q0, q2, VECTOR_UNMASKED);
            }
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x02:
            INST_NAME("VPBLENDD Gx, Vx, Ex, Imm8");
            nextop = F8;
            GETGY_empty_VYEY_vector_D(v0, q0, q1, VECTOR_SEW32, 1);
            u8 = F8;
            VECTOR_LOAD_VMASK(vex.l ? u8 : (u8 & 0xf), x4);
            q2 = vpu_get_scratch(dyn);
            VMERGE_VVM(q2, q0, q1);
            PUTGY_vector(q2, VECTOR_SEW32);
            break;
        case 0x04:
            INST_NAME("VPERMILPS Gx, Ex, Ib");
            nextop = F8;
            GETEY_vector(q0, 1, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            u8 = F8;
            VEC_SRC_KEEP(q0, v0);
            q1 = vpu_get_scratch(dyn);
            MOV64xs(x4, (uint64_t)(u8 & 3) | ((uint64_t)((u8 >> 2) & 3) << 16) | ((uint64_t)((u8 >> 4) & 3) << 32) | ((uint64_t)((u8 >> 6) & 3) << 48), x3);
            rvv_set_vl_for_width(dyn, ninst, x1, VECTOR_SEW64, VECTOR_LMUL1, 16);
            if (vex.l) {
                VMV_V_X(q1, x4);
                rvv_set_vl_for_width(dyn, ninst, x1, VECTOR_SEW16, VECTOR_LMUL1, 16);
                VID_V(VMASK, VECTOR_UNMASKED);
                VMSGT_VI(VMASK, VMASK, 3, VECTOR_UNMASKED);
                VOR_VI(q1, q1, 4, VECTOR_MASKED);
            } else {
                VMV_S_X(q1, x4);
            }
            simd_set_width(dyn, ninst, x1, VECTOR_SEW32, 16 << vex.l);
            VRGATHEREI16_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x05:
            INST_NAME("VPERMILPD Gx, Ex, Ib");
            nextop = F8;
            GETEY_vector(q0, 1, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            u8 = F8;
            VEC_SRC_KEEP(q0, v0);
            q1 = vpu_get_scratch(dyn);
            MOV64x(x4, (uint64_t)(u8 & 1) | ((uint64_t)((u8 >> 1) & 1) << 16) | ((uint64_t)(((u8 >> 2) & 1) + 2) << 32) | ((uint64_t)(((u8 >> 3) & 1) + 2) << 48));
            MOV64xs(x4, (uint64_t)(u8 & 1) | ((uint64_t)((u8 >> 1) & 1) << 16) | ((uint64_t)(((u8 >> 2) & 1) + 2) << 32) | ((uint64_t)(((u8 >> 3) & 1) + 2) << 48), x3);
            VMV_S_X(q1, x4);
            VRGATHEREI16_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x06:
        case 0x46: {
            if (!vex.l) return 0;
            INST_NAME(opcode == 0x06 ? "VPERM2F128 Gx, Vx, Ex, Ib" : "VPERM2I128 Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGY_empty_VYEY_vector_D(v0, q0, q1, VECTOR_SEW64, 1);
            u8 = F8;
            int low = (u8 & 2) ? q1 : q0;
            int high = (u8 & 0x20) ? q1 : q0;
            int zero_low = u8 & 8;
            int zero_high = u8 & 0x80;
            int low_from_high = u8 & 1;
            int high_from_high = u8 & 0x10;
            if (zero_low && zero_high) {
                VMV_V_I(v0, 0);
            } else if (zero_high || (zero_low && high_from_high)) {
                if (zero_high && low_from_high) {
                    VSLIDEDOWN_VI(v0, low, 2, VECTOR_UNMASKED);
                    low = v0;
                }
                if (!zero_high || !low_from_high || cpuext.vlen > 32) {
                    VECTOR_LOAD_VMASK(zero_high ? 0xc : 0x3, x4);
                    VMERGE_VIM(v0, zero_high ? low : high, 0);
                }
            } else if (!low_from_high && high_from_high) {
                if (low != high) {
                    VECTOR_LOAD_VMASK(0xc, x4);
                    VMERGE_VVM(v0, low, high);
                } else if (v0 != low) {
                    VMV_V_V(v0, low);
                }
            } else {
                if (high_from_high) {
                    q2 = vpu_get_scratch(dyn);
                    VSLIDEDOWN_VI(q2, high, 2, VECTOR_UNMASKED);
                    high = q2;
                } else {
                    VEC_SRC_KEEP(high, v0);
                }
                if (zero_low) {
                    VMV_V_I(v0, 0);
                } else if (low_from_high) {
                    VSLIDEDOWN_VI(v0, low, 2, VECTOR_UNMASKED);
                } else if (v0 != low) {
                    VMV_V_V(v0, low);
                }
                VSLIDEUP_VI(v0, high, 2, VECTOR_UNMASKED);
            }
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        }
        case 0x0E:
            INST_NAME("VPBLENDW Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGY_empty_VYEY_vector_D(v0, q0, q1, VECTOR_SEW16, 1);
            u8 = F8;
            VECTOR_LOAD_VMASK(u8 | (vex.l ? (u8 << 8) : 0), x4);
            VMERGE_VVM(v0, q0, q1);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0x18:
        case 0x38:
            if (!vex.l) return 0;
            INST_NAME(opcode == 0x18 ? "VINSERTF128 Gx, Vx, Ex, Ib" : "VINSERTI128 Gx, Vx, Ex, Ib");
            nextop = F8;
            if (!MODREG) {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 1);
            }
            u8 = F8;
            q0 = simd_access(dyn, ninst, x1, vex.v, (u8 & 1) ? SIMD_XMM : SIMD_YMM, 0);
            if (MODREG) {
                q1 = simd_access(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), SIMD_XMM, 0);
            } else {
                q1 = vpu_get_scratch(dyn);
                simd_set_width(dyn, ninst, x1, VECTOR_SEW64, 16);
                VLE64_V(q1, ed, VECTOR_UNMASKED, VECTOR_NFIELD1);
            }
            GETGY_empty_vector(v0);
            if (u8 & 1) {
                if (v0 == q1) {
                    q2 = vpu_get_scratch(dyn);
                    VMV1R_V(q2, q1);
                    q1 = q2;
                }
                if (v0 != q0) VMV1R_V(v0, q0);
                simd_set_width(dyn, ninst, x1, VECTOR_SEW64, 32);
                VSLIDEUP_VI(v0, q1, 2, VECTOR_UNMASKED);
            } else if (v0 != q1) {
                if (v0 != q0) VMV1R_V(v0, q0);
                simd_set_width(dyn, ninst, x1, VECTOR_SEW64, 16);
                VMV_V_V(v0, q1);
            } else if (v0 != q0) {
                q2 = vpu_get_scratch(dyn);
                simd_set_width(dyn, ninst, x1, VECTOR_SEW64, 16);
                VSLIDEDOWN_VI(q2, q0, 2, VECTOR_UNMASKED);
                simd_set_width(dyn, ninst, x1, VECTOR_SEW64, 32);
                VSLIDEUP_VI(v0, q2, 2, VECTOR_UNMASKED);
            }
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x19:
        case 0x39:
            if (!vex.l) return 0;
            INST_NAME(opcode == 0x19 ? "VEXTRACTF128 Ex, Gx, Ib" : "VEXTRACTI128 Ex, Gx, Ib");
            nextop = F8;
            gd = ((nextop >> 3) & 7) + (rex.r << 3);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                u8 = F8;
                if (u8 & 1) {
                    q0 = simd_access(dyn, ninst, x1, gd, SIMD_HI128, 0);
                    v0 = simd_get_reg_empty(dyn, ninst, x1, ed);
                    simd_set_width(dyn, ninst, x1, VECTOR_SEW64, 16);
                    VSLIDEDOWN_VI(v0, q0, 2, VECTOR_UNMASKED);
                    simd_store_reg(dyn, ninst, x1, v0, ed, 16, VECTOR_SEW64);
                } else {
                    simd_copy(dyn, ninst, x1, ed, gd, 16, 1);
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 1);
                u8 = F8;
                q0 = simd_access(dyn, ninst, x1, gd, (u8 & 1) ? SIMD_HI128 : SIMD_XMM, 0);
                simd_set_width(dyn, ninst, x1, VECTOR_SEW8, 16);
                if (u8 & 1) {
                    q1 = vpu_get_scratch(dyn);
                    VSLIDEDOWN_VI(q1, q0, 16, VECTOR_UNMASKED);
                    q0 = q1;
                }
                VSE8_V(q0, ed, VECTOR_UNMASKED, VECTOR_NFIELD1);
                SMWRITE2();
            }
            break;
        case 0x1D: {
            if (!cpuext.zvfh) return 0;
            INST_NAME("VCVTPS2PH Ex, Gx, Ib");
            nextop = F8;
            GETG;
            int source = simd_access(dyn, ninst, x1, gd, vex.l ? SIMD_YMM : SIMD_XMM, 0);
            if (source & 1) {
                int aligned = vpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                VMV1R_V(aligned, source);
                source = aligned;
            }
            int destination = -1;
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                destination = simd_get_reg_empty(dyn, ninst, x1, ed);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 1);
            }
            u8 = F8;
            int halves = destination < 0 || destination == source || destination == source + 1 ? vpu_get_scratch(dyn) : destination;
            int sign = vpu_get_scratch(dyn);
            int payload = vpu_get_scratch(dyn);
            if (u8 & 4) {
                LWU(x4, xEmu, offsetof(x64emu_t, mxcsr));
                SRLI(x4, x4, 12);
                ANDI(x4, x4, 6);
                ADDI(x5, xZR, 0x78);
                SRL(x5, x5, x4);
                ANDI(x5, x5, 3);
                SLLI(x5, x5, 5);
                CSRRW(x6, x5, 0x003);
            } else if (u8 & 3) {
                ADDI(x5, xZR, round_round[u8 & 3] << 5);
                CSRRW(x6, x5, 0x003);
            } else {
                CSRRW(x6, xZR, 0x003);
            }
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW32);
            VMFNE_VV(VMASK, source, source, VECTOR_UNMASKED);
            simd_set_width(dyn, ninst, x1, VECTOR_SEW16, 8 << vex.l);
            VFNCVT_F_F_W(halves, source, VECTOR_UNMASKED);
            CSRRW(xZR, x6, 0x003);
            VNSRA_WI(sign, source, 31, VECTOR_UNMASKED);
            MOV32w(x4, 0x7fff);
            VOR_VX(sign, sign, x4, VECTOR_UNMASKED);
            VNSRL_WI(payload, source, 13, VECTOR_UNMASKED);
            MOV32w(x4, 0x0200);
            VOR_VX(payload, payload, x4, VECTOR_UNMASKED);
            VAND_VV(halves, payload, sign, VECTOR_MASKED);
            if (MODREG) {
                if (destination != halves) VMV_V_V(destination, halves);
                simd_commit(dyn, ninst, ed, vex.l ? SIMD_XMM : SIMD_LO64, (vex.l ? 0 : SIMD_HI64) | SIMD_HI128);
            } else {
                VSE16_V(halves, ed, VECTOR_UNMASKED, VECTOR_NFIELD1);
                SMWRITE2();
            }
            break;
        }
        case 0x42: {
            INST_NAME("VMPSADBW Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGY_empty_VYEY_vector_D(v0, q0, q1, VECTOR_SEW8, 1);
            u8 = F8;
            int group = vex.l ? VECTOR_LMUL2 : VECTOR_LMUL1;
            int outputs = 8 << vex.l;
            int pair_sums = vpu_get_scratch_lmul(dyn, group + 1);
            int left_index = vpu_get_scratch_lmul(dyn, group);
            int right_index = vpu_get_scratch_lmul(dyn, group);
            int distances = vpu_get_scratch_lmul(dyn, group);
            int shifted = vpu_get_scratch_lmul(dyn, group);
            for (int slot = EMM0; slot < EMM0 + 8; ++slot) {
                if ((dyn->e.vpu_scratch & (1u << EXTREG(slot))) && dyn->e.extcache[slot].t == EXT_CACHE_MMV)
                    mmx_forget_reg(dyn, ninst, x1, dyn->e.extcache[slot].n);
            }
            int left_offset = ((u8 >> 2) & 1) * 4;
            int right_offset = (u8 & 3) * 4;
            int left_delta = ((u8 >> 5) & 1) * 4 - left_offset;
            int right_delta = ((u8 >> 3) & 3) * 4 - right_offset;

            rvv_set_config(dyn, ninst, x1, VECTOR_SEW8, group, outputs * 4);
            VID_V(left_index, VECTOR_UNMASKED);
            VSRL_VI(right_index, left_index, 3 + vex.l, VECTOR_UNMASKED);
            if (vex.l) {
                VAND_VI(distances, left_index, 8, VECTOR_UNMASKED);
                if (left_delta || right_delta) VMSNE_VI(VMASK, distances, 0, VECTOR_UNMASKED);
                VSLL_VI(distances, distances, 1, VECTOR_UNMASKED);
                VADD_VV(right_index, right_index, distances, VECTOR_UNMASKED);
            }
            VAND_VI(left_index, left_index, 7, VECTOR_UNMASKED);
            VADD_VV(left_index, left_index, right_index, VECTOR_UNMASKED);
            if (left_offset) VADD_VI(left_index, left_index, left_offset, VECTOR_UNMASKED);
            if (right_offset) VADD_VI(right_index, right_index, right_offset, VECTOR_UNMASKED);
            if (vex.l) {
                if (left_delta) VADD_VI(left_index, left_index, left_delta, VECTOR_MASKED);
                if (right_delta) VADD_VI(right_index, right_index, right_delta, VECTOR_MASKED);
                if (q0 & 1) {
                    VMV1R_V(pair_sums, q0);
                    q0 = pair_sums;
                }
            }
            VRGATHER_VV(distances, q0, left_index, VECTOR_UNMASKED);
            if (vex.l && (q1 & 1)) {
                VMV1R_V(pair_sums, q1);
                q1 = pair_sums;
            }
            VRGATHER_VV(shifted, q1, right_index, VECTOR_UNMASKED);
            VMAXU_VV(left_index, distances, shifted, VECTOR_UNMASKED);
            VMINU_VV(shifted, distances, shifted, VECTOR_UNMASKED);
            VSUB_VV(distances, left_index, shifted, VECTOR_UNMASKED);

            if (vex.l) {
                ADDI(x4, xZR, 32);
                VSLIDEDOWN_VX(shifted, distances, x4, VECTOR_UNMASKED);
            } else {
                VSLIDEDOWN_VI(shifted, distances, 16, VECTOR_UNMASKED);
            }
            VWADDU_VV(pair_sums, distances, shifted, VECTOR_UNMASKED);
            rvv_set_config(dyn, ninst, x1, VECTOR_SEW16, group, outputs);
            VSLIDEDOWN_VI(shifted, pair_sums, outputs, VECTOR_UNMASKED);
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW16);
            VADD_VV(v0, pair_sums, shifted, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        }
        case 0x08:
        case 0x09: {
            INST_NAME(opcode == 0x08 ? "VROUNDPS Gx, Ex, Ib" : "VROUNDPD Gx, Ex, Ib");
            int sew = opcode == 0x08 ? VECTOR_SEW32 : VECTOR_SEW64;
            nextop = F8;
            GETEY_vector(q0, 1, sew);
            GETGY_empty_vector(v0);
            u8 = F8;
            SET_AVX_VECTOR_WIDTH(x1, sew);
            int rounded = vpu_get_scratch(dyn);
            int truncate = (u8 & 7) == 3;
            int fixed_round = !(u8 & 4) && !truncate;
            int suppress_inexact = (u8 & 8) && BOX64ENV(sse_flushto0);
            VSLL_VI(rounded, q0, 1, VECTOR_UNMASKED);
            if (sew == VECTOR_SEW32) {
                MOV32w(x5, 0x96000000);
            } else {
                MOV64x(x5, 0x8660000000000000ULL);
            }
            VMSLTU_VX(VMASK, rounded, x5, VECTOR_UNMASKED);
            if (suppress_inexact && fixed_round) {
                ADDI(x5, xZR, round_round[u8 & 3] << 5);
                CSRRW(x6, x5, 0x003);
            } else if (suppress_inexact) {
                FRFLAGS(x6);
            } else if (fixed_round) {
                CSRRWI(x6, round_round[u8 & 3], 0x002);
            }
            if (truncate) {
                VFCVT_RTZ_X_F_V(rounded, q0, VECTOR_MASKED);
            } else {
                VFCVT_X_F_V(rounded, q0, VECTOR_MASKED);
            }
            VFCVT_F_X_V(rounded, rounded, VECTOR_MASKED);
            if (suppress_inexact) {
                CSRRW(xZR, x6, fixed_round ? 0x003 : 0x001);
            } else if (fixed_round) {
                FSRM(xZR, x6);
            }
            VFSGNJ_VV(rounded, rounded, q0, VECTOR_MASKED);
            VMERGE_VVM(v0, q0, rounded);
            VMFNE_VV(VMASK, v0, v0, VECTOR_UNMASKED);
            if (sew == VECTOR_SEW32) {
                MOV32w(x5, 0x00400000);
            } else {
                MOV64x(x5, 0x0008000000000000ULL);
            }
            VOR_VX(v0, v0, x5, VECTOR_MASKED);
            PUTGY_vector(v0, sew);
            break;
        }
        case 0x0A:
            INST_NAME("VROUNDSS Gx, Vx, Ex, Ib");
            nextop = F8;
            if (cpuext.zfa) {
                gd = ((nextop >> 3) & 7) + (rex.r << 3);
                d0 = fpu_get_scratch(dyn);
                if (MODREG) {
                    q0 = simd_get(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 4, VECTOR_SEW32, 0);
                    VFMV_F_S(d0, q0);
                } else {
                    SMREAD();
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 1);
                    FLW(d0, ed, fixedaddress);
                }
                u8 = F8;
                d1 = fpu_get_scratch(dyn);
                FROUNDS(d1, d0, (u8 & 4) ? RD_DYN : round_round[u8 & 3], !(u8 & 8));
                FEQS(x4, d0, d0);
                BNEZ_MARK(x4);
                FMVXW(x4, d0);
                BSETI(x4, x4, 22, x5);
                FMVWX(d1, x4);
                MARK;
                v0 = simd_seed_scalar(dyn, ninst, x1, gd, vex.v, VECTOR_SEW32);
                VFMV_S_F(v0, d1);
                break;
            }
            q0 = vpu_get_scratch(dyn);
            simd_load_reg(dyn, ninst, x1, q0, vex.v, 16, VECTOR_SEW32);
            d0 = fpu_get_scratch(dyn);
            if (MODREG) {
                q1 = vpu_get_scratch(dyn);
                simd_load_reg(dyn, ninst, x1, q1, (nextop & 7) + (rex.b << 3), 16, VECTOR_SEW32);
                VFMV_F_S(d0, q1);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 1);
                FLW(d0, ed, fixedaddress);
            }
            u8 = F8;
            d1 = fpu_get_scratch(dyn);
            d2 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            FEQS(x2, d0, d0);
            BNEZ_MARK(x2);
            if (!BOX64ENV(dynarec_fastnan)) {
                // sNaN -> qNaN
                FMVXW(x3, d0);
                MOV32w(x4, 0x00400000);
                OR(x3, x3, x4);
                FMVWX(d2, x3);
            } else {
                FMVS(d2, d0);
            }
            B_MARK3_nocond;
            MARK; // d0 is not nan
            FABSS(v1, d0);
            MOV64x(x3, 1ULL << __FLT_MANT_DIG__);
            FCVTSW(d1, x3, RD_RTZ);
            FLTS(x3, v1, d1);
            BNEZ_MARK2(x3);
            FMVS(d2, d0);
            B_MARK3_nocond;
            MARK2;
            FMVXW(x3, d0);
            if (u8 & 4) {
                FCVTWS(x5, d0, RD_DYN);
                FCVTSW(d2, x5, RD_RTZ);
            } else {
                FCVTWS(x5, d0, round_round[u8 & 3]);
                FCVTSW(d2, x5, RD_RTZ);
            }
            SEQZ(x4, x5);
            SLLI(x4, x4, 31);
            AND(x3, x3, x4);
            FMVXW(x4, d2);
            OR(x4, x4, x3);
            FMVWX(d2, x4);
            MARK3;
            GETGY_empty_vector(v0);
            VMV_V_V(v0, q0);
            VFMV_S_F(v0, d2);
            simd_store_reg(dyn, ninst, x1, v0, gd, 16, VECTOR_SEW32);
            break;
        case 0x0B:
            INST_NAME("VROUNDSD Gx, Vx, Ex, Ib");
            nextop = F8;
            if (cpuext.zfa) {
                gd = ((nextop >> 3) & 7) + (rex.r << 3);
                d0 = fpu_get_scratch(dyn);
                if (MODREG) {
                    q0 = simd_get(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 8, VECTOR_SEW64, 0);
                    VFMV_F_S(d0, q0);
                } else {
                    SMREAD();
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 1);
                    FLD(d0, ed, fixedaddress);
                }
                u8 = F8;
                d1 = fpu_get_scratch(dyn);
                FROUNDD(d1, d0, (u8 & 4) ? RD_DYN : round_round[u8 & 3], !(u8 & 8));
                FEQD(x4, d0, d0);
                BNEZ_MARK(x4);
                FMVXD(x4, d0);
                BSETI(x4, x4, 51, x5);
                FMVDX(d1, x4);
                MARK;
                v0 = simd_seed_scalar(dyn, ninst, x1, gd, vex.v, VECTOR_SEW64);
                VFMV_S_F(v0, d1);
                break;
            }
            q0 = vpu_get_scratch(dyn);
            simd_load_reg(dyn, ninst, x1, q0, vex.v, 16, VECTOR_SEW64);
            d0 = fpu_get_scratch(dyn);
            if (MODREG) {
                q1 = vpu_get_scratch(dyn);
                simd_load_reg(dyn, ninst, x1, q1, (nextop & 7) + (rex.b << 3), 16, VECTOR_SEW64);
                VFMV_F_S(d0, q1);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 1);
                FLD(d0, ed, fixedaddress);
            }
            u8 = F8;
            d1 = fpu_get_scratch(dyn);
            d2 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            FEQD(x2, d0, d0);
            BNEZ_MARK(x2);
            if (!BOX64ENV(dynarec_fastnan)) {
                // sNaN -> qNaN
                FMVXD(x3, d0);
                MOV64x(x4, 0x0008000000000000ULL);
                OR(x3, x3, x4);
                FMVDX(d2, x3);
            } else {
                FMVD(d2, d0);
            }
            B_MARK3_nocond;
            MARK; // d0 is not nan
            FABSD(v1, d0);
            MOV64x(x3, 1ULL << __DBL_MANT_DIG__);
            FCVTDL(d1, x3, RD_RTZ);
            FLTD(x3, v1, d1);
            BNEZ_MARK2(x3);
            FMVD(d2, d0);
            B_MARK3_nocond;
            MARK2;
            FMVXD(x3, d0);
            if (u8 & 4) {
                FCVTLD(x5, d0, RD_DYN);
                FCVTDL(d2, x5, RD_RTZ);
            } else {
                FCVTLD(x5, d0, round_round[u8 & 3]);
                FCVTDL(d2, x5, RD_RTZ);
            }
            SEQZ(x4, x5);
            SLLI(x4, x4, 63);
            AND(x3, x3, x4);
            FMVXD(x4, d2);
            OR(x4, x4, x3);
            FMVDX(d2, x4);
            MARK3;
            GETGY_empty_vector(v0);
            VMV_V_V(v0, q0);
            VFMV_S_F(v0, d2);
            simd_store_reg(dyn, ninst, x1, v0, gd, 16, VECTOR_SEW64);
            break;
        case 0x0C:
            INST_NAME("VBLENDPS Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGY_empty_VYEY_vector_D(v0, q0, q1, VECTOR_SEW32, 1);
            u8 = F8;
            VECTOR_LOAD_VMASK(vex.l ? u8 : (u8 & 0xf), x4);
            q2 = vpu_get_scratch(dyn);
            VMERGE_VVM(q2, q0, q1);
            PUTGY_vector(q2, VECTOR_SEW32);
            break;
        case 0x14:
            INST_NAME("VPEXTRB Gd, Ex, Ib");
            nextop = F8;
            q0 = vpu_get_scratch(dyn);
            simd_load_reg(dyn, ninst, x1, q0, ((nextop & 0x38) >> 3) + (rex.r << 3), 16, VECTOR_SEW8);
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW8);
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 1);
            }
            u8 = F8;
            q1 = vpu_get_scratch(dyn);
            VSLIDEDOWN_VI(q1, q0, u8 & 15, VECTOR_UNMASKED);
            VMV_X_S(x4, q1);
            if (MODREG) {
                ANDI(ed, x4, 0xff);
            } else {
                SB(x4, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0x15:
            INST_NAME("VPEXTRW Ew, Gx, Ib");
            nextop = F8;
            q0 = vpu_get_scratch(dyn);
            simd_load_reg(dyn, ninst, x1, q0, ((nextop & 0x38) >> 3) + (rex.r << 3), 16, VECTOR_SEW16);
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW16);
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 1);
            }
            u8 = F8;
            q1 = vpu_get_scratch(dyn);
            VSLIDEDOWN_VI(q1, q0, u8 & 7, VECTOR_UNMASKED);
            VMV_X_S(x4, q1);
            if (MODREG) {
                ZEXTH(ed, x4);
            } else {
                SH(x4, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0x16:
            if (rex.w) {
                INST_NAME("VPEXTRQ Ed, Gx, Ib");
            } else {
                INST_NAME("VPEXTRD Ed, Gx, Ib");
            }
            nextop = F8;
            q0 = vpu_get_scratch(dyn);
            simd_load_reg(dyn, ninst, x1, q0, ((nextop & 0x38) >> 3) + (rex.r << 3), 16, rex.w ? VECTOR_SEW64 : VECTOR_SEW32);
            SET_AVX_VECTOR_WIDTH(x1, rex.w ? VECTOR_SEW64 : VECTOR_SEW32);
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 1);
            }
            u8 = F8;
            q1 = vpu_get_scratch(dyn);
            VSLIDEDOWN_VI(q1, q0, rex.w ? (u8 & 1) : (u8 & 3), VECTOR_UNMASKED);
            VMV_X_S(x4, q1);
            if (MODREG) {
                if (rex.w) {
                    MV(ed, x4);
                } else {
                    MV(ed, x4);
                    ZEROUP(ed);
                }
            } else {
                if (rex.w) {
                    SD(x4, ed, fixedaddress);
                } else {
                    SW(x4, ed, fixedaddress);
                }
                SMWRITE2();
            }
            break;
        case 0x17:
            if (rex.w) return 0;
            INST_NAME("VEXTRACTPS Em, Gx, Ib");
            nextop = F8;
            q0 = vpu_get_scratch(dyn);
            simd_load_reg(dyn, ninst, x1, q0, ((nextop & 0x38) >> 3) + (rex.r << 3), 16, VECTOR_SEW32);
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW32);
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 1);
            }
            u8 = F8;
            q1 = vpu_get_scratch(dyn);
            VSLIDEDOWN_VI(q1, q0, u8 & 3, VECTOR_UNMASKED);
            VMV_X_S(x4, q1);
            if (MODREG) {
                MV(ed, x4);
                ZEROUP(ed);
            } else {
                SW(x4, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0x20:
            if (rex.w) return 0;
            INST_NAME("VPINSRB Gx, Vx, Ed, Ib");
            nextop = F8;
            q0 = vpu_get_scratch(dyn);
            simd_load_reg(dyn, ninst, x1, q0, vex.v, 16, VECTOR_SEW8);
            GETGY_empty_vector(v0);
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                ANDI(x5, ed, 0xFF);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 1);
                LBU(x5, ed, fixedaddress);
            }
            u8 = F8;
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW8);
            VECTOR_LOAD_VMASK(1 << (u8 & 15), x4);
            q1 = vpu_get_scratch(dyn);
            q2 = vpu_get_scratch(dyn);
            VMV_V_X(q1, x5);
            VMERGE_VVM(q2, q0, q1);
            simd_store_reg(dyn, ninst, x1, q2, gd, 16, VECTOR_SEW8);
            break;
        case 0x22:
            if (rex.w) {
                INST_NAME("VPINSRQ Gx, Vx, ED, Ib");
            } else {
                INST_NAME("VPINSRD Gx, Vx, ED, Ib");
            }
            nextop = F8;
            gd = ((nextop & 0x38) >> 3) + (rex.r << 3);
            q0 = vpu_get_scratch(dyn);
            simd_load_reg(dyn, ninst, x1, q0, vex.v, 16, rex.w ? VECTOR_SEW64 : VECTOR_SEW32);
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                MARKREGs(ed);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 1);
                if (rex.w) {
                    LD(x5, ed, fixedaddress);
                } else {
                    LWU(x5, ed, fixedaddress);
                }
                ed = x5;
            }
            u8 = F8;
            if (rex.w) {
                SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW64);
                VECTOR_LOAD_VMASK(1 << (u8 & 1), x4);
            } else {
                SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW32);
                VECTOR_LOAD_VMASK(1 << (u8 & 3), x4);
            }
            q1 = vpu_get_scratch(dyn);
            q2 = vpu_get_scratch(dyn);
            VMV_V_X(q1, ed);
            VMERGE_VVM(q2, q0, q1);
            simd_store_reg(dyn, ninst, x1, q2, gd, 16, rex.w ? VECTOR_SEW64 : VECTOR_SEW32);
            break;
        case 0x21:
            INST_NAME("VINSERTPS Gx, Vx, Ex, Ib");
            nextop = F8;
            q0 = vpu_get_scratch(dyn);
            simd_load_reg(dyn, ninst, x1, q0, vex.v, 16, VECTOR_SEW32);
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW32);
            if (MODREG) {
                q1 = vpu_get_scratch(dyn);
                simd_load_reg(dyn, ninst, x1, q1, (nextop & 7) + (rex.b << 3), 16, VECTOR_SEW32);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 1);
            }
            GETGY_empty_vector(v0);
            u8 = F8;
            q2 = vpu_get_scratch(dyn);
            VMV_V_V(q2, q0);
            if (MODREG) {
                q3 = vpu_get_scratch(dyn);
                int count_s = (u8 >> 6) & 3;
                int count_d = (u8 >> 4) & 3;
                if (count_s > count_d) {
                    VSLIDEDOWN_VI(q3, q1, count_s - count_d, VECTOR_UNMASKED);
                } else if (count_s < count_d) {
                    VSLIDEUP_VI_SAFE(q3, q1, count_d - count_s, VECTOR_UNMASKED);
                } else {
                    q3 = q1;
                }
            } else {
                q3 = vpu_get_scratch(dyn);
                LWU(x5, ed, fixedaddress);
                VMV_V_X(q3, x5);
            }
            VECTOR_LOAD_VMASK(1 << ((u8 >> 4) & 3), x4);
            VMERGE_VVM(q2, q2, q3);
            // zero out the dwords selected by imm8[3:0]
            if (u8 & 0xf) {
                VECTOR_LOAD_VMASK(u8 & 0xf, x4);
                VXOR_VV(q2, q2, q2, VECTOR_MASKED);
            }
            PUTGY_vector(q2, VECTOR_SEW32);
            break;
        case 0x40: {
            INST_NAME("VDPPS Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGY_empty_VYEY_vector_D(v0, q0, q1, VECTOR_SEW32, 1);
            u8 = F8;
            unsigned products_mask = u8 >> 4;
            unsigned result_mask = u8 & 0x0f;
            if (products_mask == 0) {
                VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
                PUTGY_vector(v0, VECTOR_SEW32);
                break;
            }
            int products = vpu_get_scratch(dyn);
            int zero = vpu_get_scratch(dyn);
            int upper = vex.l ? vpu_get_scratch(dyn) : -1;
            int masked = products_mask != 0x0f;
            if (masked) VECTOR_LOAD_VMASK(products_mask | (products_mask << 4), x4);
            VFMUL_VV(products, q0, q1, masked ? VECTOR_MASKED : VECTOR_UNMASKED);
            if (vex.l) {
                simd_set_width(dyn, ninst, x1, VECTOR_SEW32, 16);
                VSLIDEDOWN_VI(upper, products, 4, VECTOR_UNMASKED);
            }
            VXOR_VV(zero, zero, zero, VECTOR_UNMASKED);
            VFREDOSUM_VS(products, products, zero, masked ? VECTOR_MASKED : VECTOR_UNMASKED);
            if (vex.l) {
                VFREDOSUM_VS(upper, upper, zero, masked ? VECTOR_MASKED : VECTOR_UNMASKED);
                SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW32);
            }
            if (result_mask == 0x0f) {
                VRGATHER_VI(v0, products, 0, VECTOR_UNMASKED);
            } else {
                VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
                if (result_mask) {
                    VECTOR_LOAD_VMASK(result_mask, x4);
                    VRGATHER_VI(v0, products, 0, VECTOR_MASKED);
                }
            }
            if (vex.l && result_mask) {
                VECTOR_LOAD_VMASK(result_mask << 4, x4);
                VRGATHER_VI(v0, upper, 0, VECTOR_MASKED);
            }
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        }
        case 0x41: {
            if (vex.l) return 0;
            INST_NAME("VDPPD Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGY_empty_VYEY_vector_D(v0, q0, q1, VECTOR_SEW64, 1);
            u8 = F8;
            unsigned products_mask = (u8 >> 4) & 3;
            unsigned result_mask = u8 & 3;
            if (products_mask == 0) {
                VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
                PUTGY_vector(v0, VECTOR_SEW64);
                break;
            }
            int products = vpu_get_scratch(dyn);
            if (products_mask == 3) {
                VFMUL_VV(products, q0, q1, VECTOR_UNMASKED);
                VECTOR_LOAD_VMASK(2, x4);
                VFREDOSUM_VS(products, products, products, VECTOR_MASKED);
            } else {
                int zero = vpu_get_scratch(dyn);
                VECTOR_LOAD_VMASK(products_mask, x4);
                VFMUL_VV(products, q0, q1, VECTOR_MASKED);
                VXOR_VV(zero, zero, zero, VECTOR_UNMASKED);
                VFREDOSUM_VS(products, products, zero, VECTOR_MASKED);
            }
            if (result_mask == 3) {
                VRGATHER_VI(v0, products, 0, VECTOR_UNMASKED);
            } else {
                VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
                if (result_mask) {
                    VECTOR_LOAD_VMASK(result_mask, x4);
                    VRGATHER_VI(v0, products, 0, VECTOR_MASKED);
                }
            }
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        }
        case 0x44:
            if (!cpuext.zvbc) return 0;
            INST_NAME("VPCLMULQDQ Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGY_empty_VYEY_vector_D(v0, q0, q1, VECTOR_SEW64, 1);
            u8 = F8;
            q2 = q0;
            q3 = q1;
            if (u8 & 1) {
                q2 = vpu_get_scratch(dyn);
                VSLIDEDOWN_VI(q2, q0, 1, VECTOR_UNMASKED);
            }
            if (u8 & 0x10) {
                if (q0 == q1 && (u8 & 1)) {
                    q3 = q2;
                } else {
                    q3 = vpu_get_scratch(dyn);
                    VSLIDEDOWN_VI(q3, q1, 1, VECTOR_UNMASKED);
                }
            }
            q4 = vpu_get_scratch(dyn);
            VCLMULH_VV(q4, q2, q3, VECTOR_UNMASKED);
            VCLMUL_VV(v0, q2, q3, VECTOR_UNMASKED);
            if (vex.l) {
                VECTOR_LOAD_VMASK(0xa, x4);
                VSLIDEUP_VI(v0, q4, 1, VECTOR_MASKED);
            } else {
                VSLIDEUP_VI(v0, q4, 1, VECTOR_UNMASKED);
            }
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x4A:
            INST_NAME("VBLENDVPS Gx, Vx, Ex, Vx2");
            nextop = F8;
            q0 = vpu_get_scratch(dyn);
            simd_load_reg(dyn, ninst, x1, q0, vex.v, 16 << vex.l, VECTOR_SEW32);
            GETEY_vector(q1, 1, VECTOR_SEW32);
            u8 = F8;
            q2 = vpu_get_scratch(dyn);
            simd_load_reg(dyn, ninst, x1, q2, (u8 >> 4) & 0xf, 16 << vex.l, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            VMSLT_VX(VMASK, q2, xZR, VECTOR_UNMASKED);
            VMERGE_VVM(v0, q0, q1);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x4C:
            if (rex.w) return 0;
            INST_NAME("VPBLENDVB Gx, Vx, Ex, Vx2");
            nextop = F8;
            q0 = vpu_get_scratch(dyn);
            simd_load_reg(dyn, ninst, x1, q0, vex.v, 16 << vex.l, VECTOR_SEW8);
            GETEY_vector(q1, 1, VECTOR_SEW8);
            u8 = F8;
            q2 = vpu_get_scratch(dyn);
            simd_load_reg(dyn, ninst, x1, q2, (u8 >> 4) & 0xf, 16 << vex.l, VECTOR_SEW8);
            GETGY_empty_vector(v0);
            VMSLT_VX(VMASK, q2, xZR, VECTOR_UNMASKED);
            VMERGE_VVM(v0, q0, q1);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0x4B:
            INST_NAME("VBLENDVPD Gx, Vx, Ex, Vx2");
            nextop = F8;
            q0 = vpu_get_scratch(dyn);
            simd_load_reg(dyn, ninst, x1, q0, vex.v, 16 << vex.l, VECTOR_SEW64);
            GETEY_vector(q1, 1, VECTOR_SEW64);
            u8 = F8;
            q2 = vpu_get_scratch(dyn);
            simd_load_reg(dyn, ninst, x1, q2, (u8 >> 4) & 0xf, 16 << vex.l, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            VMSLT_VX(VMASK, q2, xZR, VECTOR_UNMASKED);
            VMERGE_VVM(v0, q0, q1);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x0D:
            INST_NAME("VBLENDPD Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGY_empty_VYEY_vector_D(v0, q0, q1, VECTOR_SEW64, 1);
            u8 = F8;
            VECTOR_LOAD_VMASK(vex.l ? (u8 & 0xf) : (u8 & 0x3), x4);
            VMERGE_VVM(v0, q0, q1);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x0F:
            INST_NAME("VPALIGNR Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGY_empty_VYEY_vector_D(v0, q0, q1, VECTOR_SEW8, 1);
            u8 = F8;
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW8);
            if (u8 >= 32) {
                VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
            } else if (u8 == 0 || u8 == 16) {
                int selected = u8 ? q0 : q1;
                if (v0 != selected) VMV_V_V(v0, selected);
            } else {
                const unsigned int byte_offset = u8 & 15;
                const unsigned int tail_start = 16 - byte_offset;
                if (vex.l || u8 > 16) {
                    const uint32_t lane_tail_mask = ((1u << byte_offset) - 1) << tail_start;
                    VECTOR_LOAD_VMASK(vex.l ? lane_tail_mask | (lane_tail_mask << 16) : lane_tail_mask, x4);
                }
                if (u8 < 16) {
                    VEC_SRC_KEEP(q0, v0);
                    VSLIDEDOWN_VI(v0, q1, byte_offset, VECTOR_UNMASKED);
                    VSLIDEUP_VI(v0, q0, tail_start, vex.l ? VECTOR_MASKED : VECTOR_UNMASKED);
                } else {
                    VSLIDEDOWN_VI(v0, q0, byte_offset, VECTOR_UNMASKED);
                    VMERGE_VIM(v0, v0, 0);
                }
            }
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0x60:
            INST_NAME("VPCMPESTRM Gx, Ex, Ib");
            nextop = F8;
            if (vex.l) {
                DEFAULT_VECTOR;
            }
            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            gd = ((nextop & 0x38) >> 3) + (rex.r << 3);
            simd_forget_fp_reg(dyn, ninst, x6, gd);
            ADDI(x3, xEmu, XMMOFF(gd));
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                simd_forget_fp_reg(dyn, ninst, x6, ed);
                ADDI(x1, xEmu, XMMOFF(ed));
                ed = x1;
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 0, 1);
            }
            MV(x2, xRDX);
            MV(x4, xRAX);
            u8 = F8;
            ADDI(x5, xZR, u8);
            ADDI(x7, xZR, rex.w);
            CALL6(const_sse42_compare_string_explicit_len, x1, ed, x2, x3, x4, x5, x7);
            ZEROUP(x1);
            simd_forget_fp_reg(dyn, ninst, x3, 0);
            if (u8 & 0b1000000) {
                if (u8 & 1) {
                    for (int i = 0; i < 8; ++i) {
                        BEXTI(x2, x1, i);
                        SUB(x2, xZR, x2);
                        SH(x2, xEmu, XMMOFF(0) + 2 * i);
                    }
                } else {
                    for (int i = 0; i < 16; ++i) {
                        BEXTI(x2, x1, i);
                        SUB(x2, xZR, x2);
                        SB(x2, xEmu, XMMOFF(0) + i);
                    }
                }
            } else {
                SD(xZR, xEmu, XMMOFF(0));
                SD(xZR, xEmu, XMMOFF(0) + 8);
                SH(x1, xEmu, XMMOFF(0));
            }
            YMM0(0);
            break;
        case 0x61:
            INST_NAME("VPCMPESTRI Gx, Ex, Ib");
            MARKREGs(xRAX);
            MARKREGs(xRDX);
            nextop = F8;
            if (vex.l) {
                DEFAULT_VECTOR;
            }
            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            gd = ((nextop & 0x38) >> 3) + (rex.r << 3);
            simd_forget_fp_reg(dyn, ninst, x6, gd);
            ADDI(x3, xEmu, XMMOFF(gd));
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                simd_forget_fp_reg(dyn, ninst, x6, ed);
                ADDI(x1, xEmu, XMMOFF(ed));
                ed = x1;
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 0, 1);
            }
            MV(x2, xRDX);
            MV(x4, xRAX);
            u8 = F8;
            ADDI(x5, xZR, u8);
            ADDI(x7, xZR, rex.w);
            CALL6(const_sse42_compare_string_explicit_len, x1, ed, x2, x3, x4, x5, x7);
            ZEROUP(x1);
            BNEZ_MARK(x1);
            MOV32w(xRCX, (u8 & 1) ? 8 : 16);
            B_NEXT_nocond;
            MARK;
            if (u8 & 0b1000000) {
                CLZxw(xRCX, x1, 0, x2, x3, x4);
                ADDI(x2, xZR, 31);
                SUB(xRCX, x2, xRCX);
            } else {
                CTZxw(xRCX, x1, 0, x2, x3);
            }
            break;
        case 0x62:
            INST_NAME("VPCMPISTRM Gx, Ex, Ib");
            nextop = F8;
            if (vex.l) {
                DEFAULT_VECTOR;
            }
            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            gd = ((nextop & 0x38) >> 3) + (rex.r << 3);
            simd_forget_fp_reg(dyn, ninst, x6, gd);
            ADDI(x2, xEmu, XMMOFF(gd));
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                simd_forget_fp_reg(dyn, ninst, x6, ed);
                ADDI(x1, xEmu, XMMOFF(ed));
                ed = x1;
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x5, &fixedaddress, rex, NULL, 0, 1);
            }
            u8 = F8;
            MOV32w(x3, u8);
            CALL4(const_sse42_compare_string_implicit_len, x1, ed, x2, x3, 0);
            ZEROUP(x1);
            simd_forget_fp_reg(dyn, ninst, x6, 0);
            if (u8 & 0b1000000) {
                switch (u8 & 1) {
                    case 0b00:
                        for (int i = 0; i < 16; ++i) {
                            BEXTI(x3, x1, i);
                            NEG(x3, x3);
                            SB(x3, xEmu, XMMOFF(0) + i);
                        }
                        break;
                    case 0b01:
                        for (int i = 0; i < 8; ++i) {
                            BEXTI(x3, x1, i);
                            NEG(x3, x3);
                            SH(x3, xEmu, XMMOFF(0) + i * 2);
                        }
                        break;
                }
            } else {
                SW(x1, xEmu, XMMOFF(0));
                SW(xZR, xEmu, XMMOFF(0) + 4);
                SD(xZR, xEmu, XMMOFF(0) + 8);
            }
            YMM0(0);
            break;
        case 0x63:
            INST_NAME("VPCMPISTRI Gx, Ex, Ib");
            MARKREGs(xRAX);
            MARKREGs(xRDX);
            nextop = F8;
            if (vex.l) {
                DEFAULT_VECTOR;
            }
            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            gd = ((nextop & 0x38) >> 3) + (rex.r << 3);
            simd_forget_fp_reg(dyn, ninst, x6, gd);
            ADDI(x2, xEmu, XMMOFF(gd));
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                simd_forget_fp_reg(dyn, ninst, x6, ed);
                ADDI(x1, xEmu, XMMOFF(ed));
                ed = x1;
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x5, &fixedaddress, rex, NULL, 0, 1);
            }
            u8 = F8;
            MOV32w(x3, u8);
            CALL4(const_sse42_compare_string_implicit_len, x1, ed, x2, x3, 0);
            ZEROUP(x1);
            BNEZ_MARK(x1);
            MOV32w(xRCX, (u8 & 1) ? 8 : 16);
            B_NEXT_nocond;
            MARK;
            if (u8 & 0b1000000) {
                CLZxw(xRCX, x1, 0, x2, x3, x4);
                ADDI(x2, xZR, 31);
                SUB(xRCX, x2, xRCX);
            } else {
                CTZxw(xRCX, x1, 0, x2, x3);
            }
            break;
        case 0xDF:
            INST_NAME("VAESKEYGENASSIST Gx, Ex, Ib");
            nextop = F8;
            GETG;
            simd_forget_fp_reg(dyn, ninst, x6, gd);
            MOV32w(x1, gd);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                simd_forget_fp_reg(dyn, ninst, x6, ed);
                MOV32w(x2, ed);
                MOV32w(x3, 0);
            } else {
                MOV32w(x2, 0);
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, x5, &fixedaddress, rex, NULL, 0, 1);
                if (ed != x3) {
                    MV(x3, ed);
                }
            }
            u8 = F8;
            MOV32w(x4, u8);
            CALL4(const_native_aeskeygenassist, -1, x1, x2, x3, x4);
            YMM0(gd);
            break;
        default:
            DEFAULT_VECTOR;
    }
    return addr;
}
