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
    uint8_t nextop, u8, u8s;
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
    MAYUSE(u8s);
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
            q1 = fpu_get_scratch(dyn);
            q2 = fpu_get_scratch_lmul(dyn, vex.l ? VECTOR_LMUL1 : VECTOR_LMUL2);
            {
                uint64_t iota = (u8 & 3) | (((u8 >> 2) & 3) << 16);
                iota |= ((uint64_t)((u8 >> 4) & 3)) << 32;
                iota |= ((uint64_t)((u8 >> 6) & 3)) << 48;
                vector_vsetvli(dyn, ninst, x1, VECTOR_SEW64, VECTOR_LMUL1, vex.l ? 2 : 1);
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
            VECTOR_LOAD_VMASK(vex.l ? u8 : (u8 & 0xf), x4, vex.l ? 2 : 1);
            q2 = fpu_get_scratch(dyn);
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
            q1 = fpu_get_scratch(dyn);
            MOV64xs(x4, (uint64_t)(u8 & 3) | ((uint64_t)((u8 >> 2) & 3) << 16) | ((uint64_t)((u8 >> 4) & 3) << 32) | ((uint64_t)((u8 >> 6) & 3) << 48), x3);
            vector_vsetvli(dyn, ninst, x1, VECTOR_SEW64, VECTOR_LMUL1, 1);
            if (vex.l) {
                VMV_V_X(q1, x4);
                vector_vsetvli(dyn, ninst, x1, VECTOR_SEW16, VECTOR_LMUL1, 1);
                VID_V(VMASK, VECTOR_UNMASKED);
                VMSGT_VI(VMASK, VMASK, 3, VECTOR_UNMASKED);
                VOR_VI(q1, q1, 4, VECTOR_MASKED);
            } else {
                VMV_S_X(q1, x4);
            }
            avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW32, 16 << vex.l);
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
            q1 = fpu_get_scratch(dyn);
            MOV64x(x4, (uint64_t)(u8 & 1) | ((uint64_t)((u8 >> 1) & 1) << 16) | ((uint64_t)(((u8 >> 2) & 1) + 2) << 32) | ((uint64_t)(((u8 >> 3) & 1) + 2) << 48));
            MOV64xs(x4, (uint64_t)(u8 & 1) | ((uint64_t)((u8 >> 1) & 1) << 16) | ((uint64_t)(((u8 >> 2) & 1) + 2) << 32) | ((uint64_t)(((u8 >> 3) & 1) + 2) << 48), x3);
            VMV_S_X(q1, x4);
            VRGATHEREI16_VV(v0, q0, q1, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x06:
            if (!vex.l) return 0;
            INST_NAME("VPERM2F128 Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGY_empty_VYEY_vector_D(v0, q0, q1, VECTOR_SEW64, 1);
            u8 = F8;
            VEC_SRC_KEEP(q0, v0);
            VEC_SRC_KEEP(q1, v0);
            VMV_V_V(v0, q0);
            for (int j = 0; j < 2; ++j) {
                int control = j ? (u8 >> 4) & 3 : u8 & 3;
                int zero = j ? (u8 & 0x80) : (u8 & 0x08);
                int mask = j ? 0xc : 0x3;
                if (zero) {
                    VECTOR_LOAD_VMASK(mask, x4, 2);
                    VXOR_VV(v0, v0, v0, VECTOR_MASKED);
                } else if (control != j) {
                    int src = (control & 2) ? q1 : q0;
                    VECTOR_LOAD_VMASK(mask, x4, 2);
                    if (((j == 0) && ((control & 1) == 0)) || ((j == 1) && ((control & 1) == 1))) {
                        VMERGE_VVM(v0, v0, src);
                    } else {
                        q2 = fpu_get_scratch(dyn);
                        if (j)
                            VSLIDEUP_VI(q2, src, 2, VECTOR_UNMASKED);
                        else
                            VSLIDEDOWN_VI(q2, src, 2, VECTOR_UNMASKED);
                        VMERGE_VVM(v0, v0, q2);
                    }
                }
            }
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x0E:
            INST_NAME("VPBLENDW Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGY_empty_VYEY_vector_D(v0, q0, q1, VECTOR_SEW16, 1);
            u8 = F8;
            VECTOR_LOAD_VMASK(u8 | (vex.l ? (u8 << 8) : 0), x4, vex.l ? 2 : 1);
            VMERGE_VVM(v0, q0, q1);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0x18:
            if (!vex.l) return 0;
            INST_NAME("VINSERTF128 Gx, Vx, Ex, Ib");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW64);
            q1 = fpu_get_scratch(dyn);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                avx_load_reg_vector(dyn, ninst, x1, q1, ed, 16, VECTOR_SEW64);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 1);
                vector_vsetvli(dyn, ninst, x1, VECTOR_SEW64, VECTOR_LMUL1, 1);
                VLE64_V(q1, ed, VECTOR_UNMASKED, VECTOR_NFIELD1);
            }
            u8 = F8;
            vector_vsetvli(dyn, ninst, x1, VECTOR_SEW64, VECTOR_LMUL1, 2);
            GETGY_empty_vector(v0);
            VMV_V_V(v0, q0);
            if (u8 & 1) {
                VSLIDEUP_VI_SAFE(v0, q1, 2, VECTOR_UNMASKED);
            } else {
                vector_vsetvli(dyn, ninst, x1, VECTOR_SEW64, VECTOR_LMUL1, 1);
                VMV_V_V(v0, q1);
            }
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x19:
            if (!vex.l) return 0;
            INST_NAME("VEXTRACTF128 Ex, Gx, Ib");
            nextop = F8;
            GETGY_vector(q0, VECTOR_SEW8);
            q1 = fpu_get_scratch(dyn);
            if (MODREG) {
                u8 = F8;
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 1);
                u8 = F8;
            }
            if (u8 & 1) {
                avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW8, 32);
                VSLIDEDOWN_VI(q1, q0, 16, VECTOR_UNMASKED);
            } else {
                avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW8, 16);
                VMV_V_V(q1, q0);
            }
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                avx_store_reg_vector(dyn, ninst, x1, q1, ed, 16, VECTOR_SEW8);
            } else {
                avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW8, 16);
                VSE8_V(q1, ed, VECTOR_UNMASKED, VECTOR_NFIELD1);
                SMWRITE2();
            }
            break;
        case 0x38:
            if (!vex.l) return 0;
            INST_NAME("VINSERTI128 Gx, Vx, Ex, Ib");
            nextop = F8;
            GETVY_vector(q0, VECTOR_SEW64);
            q1 = fpu_get_scratch(dyn);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                avx_load_reg_vector(dyn, ninst, x1, q1, ed, 16, VECTOR_SEW64);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 1);
                avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW64, 16);
                VLE64_V(q1, ed, VECTOR_UNMASKED, VECTOR_NFIELD1);
            }
            u8 = F8;
            GETGY_empty_vector(v0);
            avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW64, 32);
            VMV_V_V(v0, q0);
            if (u8 & 1) {
                v1 = fpu_get_scratch(dyn);
                VXOR_VV(v1, v1, v1, VECTOR_UNMASKED);
                VSLIDEUP_VI_SAFE(v1, q1, 2, VECTOR_UNMASKED);
                VECTOR_LOAD_VMASK(0b1100, x4, 2);
                VMERGE_VVM(v0, v0, v1);
            } else {
                VECTOR_LOAD_VMASK(0b0011, x4, 2);
                VMERGE_VVM(v0, v0, q1);
            }
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x39:
            if (!vex.l) return 0;
            INST_NAME("VEXTRACTI128 Ex, Gx, Ib");
            nextop = F8;
            GETGY_vector(q0, VECTOR_SEW8);
            q1 = fpu_get_scratch(dyn);
            if (MODREG) {
                u8 = F8;
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 1);
                u8 = F8;
            }
            if (u8 & 1) {
                avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW8, 32);
                VSLIDEDOWN_VI(q1, q0, 16, VECTOR_UNMASKED);
            } else {
                avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW8, 16);
                VMV_V_V(q1, q0);
            }
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                avx_store_reg_vector(dyn, ninst, x1, q1, ed, 16, VECTOR_SEW8);
            } else {
                avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW8, 16);
                VSE8_V(q1, ed, VECTOR_UNMASKED, VECTOR_NFIELD1);
                SMWRITE2();
            }
            break;
        case 0x42:
            INST_NAME("VMPSADBW Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGY_empty_VYEY_vector_D(v0, q0, q1, VECTOR_SEW8, 1);
            u8 = F8;
            int q3 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            int q4 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            int q2 = fpu_borrow_reg(dyn, ninst, x1, x2, 8);
            int q5 = fpu_borrow_reg(dyn, ninst, x1, x2, 9);
            int q6 = fpu_borrow_reg(dyn, ninst, x1, x2, 10);
            int q7 = fpu_borrow_reg(dyn, ninst, x1, x2, 11);
            int q8 = fpu_borrow_reg(dyn, ninst, x1, x2, 12);

            for (int pass = 0; pass < (vex.l ? 2 : 1); ++pass) {
                vector_vsetvli(dyn, ninst, x1, VECTOR_SEW8, VECTOR_LMUL1, 2);
                if (pass)
                    VSLIDEDOWN_VI(q7, q0, 16, VECTOR_UNMASKED);
                else
                    VMV_V_V(q7, q0);
                if (pass)
                    VSLIDEDOWN_VI(q8, q1, 16, VECTOR_UNMASKED);
                else
                    VMV_V_V(q8, q1);

                VID_V(q5, VECTOR_UNMASKED);
                VSRL_VI(q6, q5, 3, VECTOR_UNMASKED);
                VAND_VI(q5, q5, 7, VECTOR_UNMASKED);
                VADD_VV(q5, q5, q6, VECTOR_UNMASKED);
                int off5 = (pass ? ((u8 >> 5) & 1) : ((u8 >> 2) & 1)) * 4;
                int off6 = (pass ? ((u8 >> 3) & 3) : (u8 & 3)) * 4;
                if (off5) VADD_VI(q5, q5, off5, VECTOR_UNMASKED);
                if (off6) VADD_VI(q6, q6, off6, VECTOR_UNMASKED);

                VRGATHER_VV_SAFE(q3, q7, q5, VECTOR_UNMASKED);
                VRGATHER_VV_SAFE(q4, q8, q6, VECTOR_UNMASKED);
                VMAXU_VV(q7, q3, q4, VECTOR_UNMASKED);
                VMINU_VV(q4, q3, q4, VECTOR_UNMASKED);
                VSUB_VV(q7, q7, q4, VECTOR_UNMASKED);

                vector_vsetvli(dyn, ninst, x1, VECTOR_SEW16, VECTOR_LMUL2, 4);
                VZEXT_VF2(q3, q7, VECTOR_UNMASKED);
                VSLIDEDOWN_VI(q4, q3, 8, VECTOR_UNMASKED);
                VADD_VV(q3, q3, q4, VECTOR_UNMASKED);
                VSLIDEDOWN_VI(q4, q3, 16, VECTOR_UNMASKED);
                VADD_VV(q3, q3, q4, VECTOR_UNMASKED);

                if (!pass) {
                    vector_vsetvli(dyn, ninst, x1, VECTOR_SEW16, VECTOR_LMUL1, 1);
                    VMV_V_V(q2, q3);
                }
            }

            avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW16, 16 << vex.l);
            VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
            vector_vsetvli(dyn, ninst, x1, VECTOR_SEW16, VECTOR_LMUL1, 1);
            VMV_V_V(v0, q2);
            if (vex.l) {
                avx_set_vector_width(dyn, ninst, x1, VECTOR_SEW16, 32);
                VSLIDEUP_VI_SAFE(v0, q3, 8, VECTOR_UNMASKED);
            }
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0x46:
            if (!vex.l) return 0;
            INST_NAME("VPERM2I128 Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGY_empty_VYEY_vector_D(v0, q0, q1, VECTOR_SEW64, 1);
            u8 = F8;
            VEC_SRC_KEEP(q0, v0);
            VEC_SRC_KEEP(q1, v0);
            VMV_V_V(v0, q0);
            for (int j = 0; j < 2; ++j) {
                int control = j ? (u8 >> 4) & 3 : u8 & 3;
                int zero = j ? (u8 & 0x80) : (u8 & 0x08);
                int mask = j ? 0xc : 0x3;
                if (zero) {
                    VECTOR_LOAD_VMASK(mask, x4, 2);
                    VXOR_VV(v0, v0, v0, VECTOR_MASKED);
                } else if (control != j) {
                    int src = (control & 2) ? q1 : q0;
                    VECTOR_LOAD_VMASK(mask, x4, 2);
                    if (((j == 0) && ((control & 1) == 0)) || ((j == 1) && ((control & 1) == 1))) {
                        VMERGE_VVM(v0, v0, src);
                    } else {
                        q2 = fpu_get_scratch(dyn);
                        if (j)
                            VSLIDEUP_VI(q2, src, 2, VECTOR_UNMASKED);
                        else
                            VSLIDEDOWN_VI(q2, src, 2, VECTOR_UNMASKED);
                        VMERGE_VVM(v0, v0, q2);
                    }
                }
            }
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x08:
            INST_NAME("VROUNDPS Gx, Ex, Ib");
            nextop = F8;
            GETEY_vector(q0, 1, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            u8 = F8;
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW32);
            q1 = fpu_get_scratch(dyn);
            q2 = fpu_get_scratch(dyn);
            q3 = fpu_get_scratch(dyn);
            q4 = fpu_get_scratch(dyn);
            if (u8 & 4) {
                u8s = sse_setround(dyn, ninst, x5, x6);
            } else {
                ADDI(x5, xZR, round_round[u8 & 3]);
                FSRM(x6, x5);
            }
            VFCVT_X_F_V(q1, q0, VECTOR_UNMASKED);  // qi
            VFCVT_F_X_V(q2, q1, VECTOR_UNMASKED);  // qf
            // restore -0.0 where result is 0
            MOV32w(x5, 0x80000000);
            VAND_VX(q3, q0, x5, VECTOR_UNMASKED);
            VMSEQ_VI(VMASK, q1, 0, VECTOR_UNMASKED);
            VMERGE_VVM(q2, q2, q3);
            // NaN -> passthrough
            MOV32w(x5, 0x00400000);
            VOR_VX(q3, q0, x5, VECTOR_UNMASKED);
            VMFEQ_VV(VMASK, q0, q0, VECTOR_UNMASKED);
            VMERGE_VVM(q2, q3, q2);
            // >= 2^31 -> passthrough
            d0 = fpu_get_scratch(dyn);
            MOV32w(x5, 0x4f000000);
            FMVWX(d0, x5);
            VMFGE_VF(VMASK, q0, d0, VECTOR_UNMASKED);
            VMERGE_VVM(q2, q2, q0);
            // <= -2^31 -> passthrough
            d1 = fpu_get_scratch(dyn);
            MOV32w(x5, 0xcf000000);
            FMVWX(d1, x5);
            VMFLE_VF(VMASK, q0, d1, VECTOR_UNMASKED);
            VMERGE_VVM(q2, q2, q0);
            if (u8 & 4) {
                x87_restoreround(dyn, ninst, u8s);
            } else {
                FSRM(x5, x6);
            }
            PUTGY_vector(q2, VECTOR_SEW32);
            break;
        case 0x09:
            INST_NAME("VROUNDPD Gx, Ex, Ib");
            nextop = F8;
            GETEY_vector(q0, 1, VECTOR_SEW64);
            GETGY_empty_vector(v0);
            u8 = F8;
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW64);
            q1 = fpu_get_scratch(dyn);
            q2 = fpu_get_scratch(dyn);
            q3 = fpu_get_scratch(dyn);
            if (u8 & 4) {
                u8s = sse_setround(dyn, ninst, x5, x6);
            } else {
                ADDI(x5, xZR, round_round[u8 & 3]);
                FSRM(x6, x5);
            }
            VFCVT_X_F_V(q1, q0, VECTOR_UNMASKED);  // qi
            VFCVT_F_X_V(q2, q1, VECTOR_UNMASKED);  // qf
            // restore -0.0 where result is 0
            MOV64x(x5, 0x8000000000000000LL);
            VAND_VX(q3, q0, x5, VECTOR_UNMASKED);
            VMSEQ_VI(VMASK, q1, 0, VECTOR_UNMASKED);
            VMERGE_VVM(q2, q2, q3);
            // NaN -> passthrough
            MOV64x(x5, 0x0008000000000000LL);
            VOR_VX(q3, q0, x5, VECTOR_UNMASKED);
            VMFEQ_VV(VMASK, q0, q0, VECTOR_UNMASKED);
            VMERGE_VVM(q2, q3, q2);
            // >= 2^63 -> passthrough
            d0 = fpu_get_scratch(dyn);
            MOV64x(x5, 0x43e0000000000000LL);
            FMVDX(d0, x5);
            VMFGE_VF(VMASK, q0, d0, VECTOR_UNMASKED);
            VMERGE_VVM(q2, q2, q0);
            // <= -2^63 -> passthrough
            d1 = fpu_get_scratch(dyn);
            MOV64x(x5, 0xc3e0000000000000LL);
            FMVDX(d1, x5);
            VMFLE_VF(VMASK, q0, d1, VECTOR_UNMASKED);
            VMERGE_VVM(q2, q2, q0);
            if (u8 & 4) {
                x87_restoreround(dyn, ninst, u8s);
            } else {
                FSRM(x5, x6);
            }
            PUTGY_vector(q2, VECTOR_SEW64);
            break;
        case 0x0A:
            INST_NAME("VROUNDSS Gx, Vx, Ex, Ib");
            nextop = F8;
            q0 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q0, vex.v, 16, VECTOR_SEW32);
            d0 = fpu_get_scratch(dyn);
            if (MODREG) {
                q1 = fpu_get_scratch(dyn);
                avx_load_reg_vector(dyn, ninst, x1, q1, (nextop & 7) + (rex.b << 3), 16, VECTOR_SEW32);
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
                u8s = sse_setround(dyn, ninst, x4, x2);
                FCVTWS(x5, d0, RD_DYN);
                FCVTSW(d2, x5, RD_RTZ);
                x87_restoreround(dyn, ninst, u8s);
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
            avx_store_reg_vector(dyn, ninst, x1, v0, gd, 16, VECTOR_SEW32);
            break;
        case 0x0B:
            INST_NAME("VROUNDSD Gx, Vx, Ex, Ib");
            nextop = F8;
            q0 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q0, vex.v, 16, VECTOR_SEW64);
            d0 = fpu_get_scratch(dyn);
            if (MODREG) {
                q1 = fpu_get_scratch(dyn);
                avx_load_reg_vector(dyn, ninst, x1, q1, (nextop & 7) + (rex.b << 3), 16, VECTOR_SEW64);
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
                u8s = sse_setround(dyn, ninst, x4, x2);
                FCVTLD(x5, d0, RD_DYN);
                FCVTDL(d2, x5, RD_RTZ);
                x87_restoreround(dyn, ninst, u8s);
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
            avx_store_reg_vector(dyn, ninst, x1, v0, gd, 16, VECTOR_SEW64);
            break;
        case 0x0C:
            INST_NAME("VBLENDPS Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGY_empty_VYEY_vector_D(v0, q0, q1, VECTOR_SEW32, 1);
            u8 = F8;
            VECTOR_LOAD_VMASK(vex.l ? u8 : (u8 & 0xf), x4, vex.l ? 2 : 1);
            q2 = fpu_get_scratch(dyn);
            VMERGE_VVM(q2, q0, q1);
            PUTGY_vector(q2, VECTOR_SEW32);
            break;
        case 0x14:
            INST_NAME("VPEXTRB Gd, Ex, Ib");
            nextop = F8;
            q0 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q0, ((nextop & 0x38) >> 3) + (rex.r << 3), 16, VECTOR_SEW8);
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW8);
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 1);
            }
            u8 = F8;
            q1 = fpu_get_scratch(dyn);
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
            q0 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q0, ((nextop & 0x38) >> 3) + (rex.r << 3), 16, VECTOR_SEW16);
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW16);
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 1);
            }
            u8 = F8;
            q1 = fpu_get_scratch(dyn);
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
            q0 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q0, ((nextop & 0x38) >> 3) + (rex.r << 3), 16, rex.w ? VECTOR_SEW64 : VECTOR_SEW32);
            SET_AVX_VECTOR_WIDTH(x1, rex.w ? VECTOR_SEW64 : VECTOR_SEW32);
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 1);
            }
            u8 = F8;
            q1 = fpu_get_scratch(dyn);
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
            q0 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q0, ((nextop & 0x38) >> 3) + (rex.r << 3), 16, VECTOR_SEW32);
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW32);
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 1);
            }
            u8 = F8;
            q1 = fpu_get_scratch(dyn);
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
            q0 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q0, vex.v, 16, VECTOR_SEW8);
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
            VECTOR_LOAD_VMASK(1 << (u8 & 15), x4, 1);
            q1 = fpu_get_scratch(dyn);
            q2 = fpu_get_scratch(dyn);
            VMV_V_X(q1, x5);
            VMERGE_VVM(q2, q0, q1);
            avx_store_reg_vector(dyn, ninst, x1, q2, gd, 16, VECTOR_SEW8);
            break;
        case 0x22:
            if (rex.w) {
                INST_NAME("VPINSRQ Gx, Vx, ED, Ib");
            } else {
                INST_NAME("VPINSRD Gx, Vx, ED, Ib");
            }
            nextop = F8;
            gd = ((nextop & 0x38) >> 3) + (rex.r << 3);
            q0 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q0, vex.v, 16, rex.w ? VECTOR_SEW64 : VECTOR_SEW32);
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
                VECTOR_LOAD_VMASK(1 << (u8 & 1), x4, 1);
            } else {
                SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW32);
                VECTOR_LOAD_VMASK(1 << (u8 & 3), x4, 1);
            }
            q1 = fpu_get_scratch(dyn);
            q2 = fpu_get_scratch(dyn);
            VMV_V_X(q1, ed);
            VMERGE_VVM(q2, q0, q1);
            avx_store_reg_vector(dyn, ninst, x1, q2, gd, 16, rex.w ? VECTOR_SEW64 : VECTOR_SEW32);
            break;
        case 0x21:
            INST_NAME("VINSERTPS Gx, Vx, Ex, Ib");
            nextop = F8;
            q0 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q0, vex.v, 16, VECTOR_SEW32);
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW32);
            if (MODREG) {
                q1 = fpu_get_scratch(dyn);
                avx_load_reg_vector(dyn, ninst, x1, q1, (nextop & 7) + (rex.b << 3), 16, VECTOR_SEW32);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 1);
            }
            GETGY_empty_vector(v0);
            u8 = F8;
            q2 = fpu_get_scratch(dyn);
            VMV_V_V(q2, q0);
            if (MODREG) {
                q3 = fpu_get_scratch(dyn);
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
                q3 = fpu_get_scratch(dyn);
                LWU(x5, ed, fixedaddress);
                VMV_V_X(q3, x5);
            }
            VECTOR_LOAD_VMASK(1 << ((u8 >> 4) & 3), x4, 1);
            VMERGE_VVM(q2, q2, q3);
            // zero out the dwords selected by imm8[3:0]
            if (u8 & 0xf) {
                VECTOR_LOAD_VMASK(u8 & 0xf, x4, 1);
                VXOR_VV(q2, q2, q2, VECTOR_MASKED);
            }
            PUTGY_vector(q2, VECTOR_SEW32);
            break;
        case 0x40: {
            INST_NAME("VDPPS Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGY_empty_VYEY_vector_D(v0, q0, q1, VECTOR_SEW32, 1);
            u8 = F8;
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW32);
            q3 = fpu_get_scratch(dyn);
            VXOR_VV(q3, q3, q3, VECTOR_UNMASKED);
            d0 = fpu_get_scratch(dyn);
            q2 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            d2 = fpu_get_scratch(dyn);
            int nl = vex.l ? 2 : 1;
            for (int L = 0; L < nl; ++L) {
                FMVWX(d0, xZR); // sum = 0.0f
                for (int i = 0; i < 4; ++i) {
                    if ((u8 >> (4 + i)) & 1) {
                        VSLIDEDOWN_VI(q2, q0, L * 4 + i, VECTOR_UNMASKED);
                        VFMV_F_S(d1, q2);
                        VSLIDEDOWN_VI(q2, q1, L * 4 + i, VECTOR_UNMASKED);
                        VFMV_F_S(d2, q2);
                        FMULS(d1, d1, d2);
                        FADDS(d0, d0, d1);
                    }
                }
                int out_mask = u8 & 0xF;
                out_mask <<= 4 * L;
                if (out_mask) {
                    VECTOR_LOAD_VMASK(out_mask, x4, vex.l ? 2 : 1);
                    VFMV_V_F(q2, d0);
                    VMERGE_VVM(q3, q3, q2);
                }
            }
            PUTGY_vector(q3, VECTOR_SEW32);
            break;
        }
        case 0x4A:
            INST_NAME("VBLENDVPS Gx, Vx, Ex, Vx2");
            nextop = F8;
            q0 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q0, vex.v, 16 << vex.l, VECTOR_SEW32);
            GETEY_vector(q1, 1, VECTOR_SEW32);
            u8 = F8;
            q2 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q2, (u8 >> 4) & 0xf, 16 << vex.l, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            VMSLT_VX(VMASK, q2, xZR, VECTOR_UNMASKED);
            VMERGE_VVM(v0, q0, q1);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x4C:
            if (rex.w) return 0;
            INST_NAME("VPBLENDVB Gx, Vx, Ex, Vx2");
            nextop = F8;
            q0 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q0, vex.v, 16 << vex.l, VECTOR_SEW8);
            GETEY_vector(q1, 1, VECTOR_SEW8);
            u8 = F8;
            q2 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q2, (u8 >> 4) & 0xf, 16 << vex.l, VECTOR_SEW8);
            GETGY_empty_vector(v0);
            VMSLT_VX(VMASK, q2, xZR, VECTOR_UNMASKED);
            VMERGE_VVM(v0, q0, q1);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0x4B:
            INST_NAME("VBLENDVPD Gx, Vx, Ex, Vx2");
            nextop = F8;
            q0 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q0, vex.v, 16 << vex.l, VECTOR_SEW64);
            GETEY_vector(q1, 1, VECTOR_SEW64);
            u8 = F8;
            q2 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q2, (u8 >> 4) & 0xf, 16 << vex.l, VECTOR_SEW64);
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
            VECTOR_LOAD_VMASK(vex.l ? (u8 & 0xf) : (u8 & 0x3), x4, vex.l ? 2 : 1);
            VMERGE_VVM(v0, q0, q1);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x0F:
            INST_NAME("VPALIGNR Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGY_empty_VYEY_vector_D(v0, q0, q1, VECTOR_SEW8, 1);
            u8 = F8;
            if (u8 > 31) {
                VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
            } else {
                q2 = fpu_get_scratch(dyn);
                vector_vsetvli(dyn, ninst, x1, VECTOR_SEW8, VECTOR_LMUL1, 4);
                VXOR_VV(q2, q2, q2, VECTOR_UNMASKED);
                vector_vsetvli(dyn, ninst, x1, VECTOR_SEW8, VECTOR_LMUL1, 2);
                VMV_V_V(q2, q1);
                VSLIDEUP_VI_SAFE(q2, q0, 16, VECTOR_UNMASKED);
                if (vex.l) {
                    q3 = fpu_get_scratch(dyn);
                    q4 = fpu_get_scratch(dyn);
                    vector_vsetvli(dyn, ninst, x1, VECTOR_SEW8, VECTOR_LMUL1, 4);
                    VXOR_VV(q3, q3, q3, VECTOR_UNMASKED);
                    vector_vsetvli(dyn, ninst, x1, VECTOR_SEW8, VECTOR_LMUL1, 2);
                    VMV_V_V(q3, q0);
                    vector_vsetvli(dyn, ninst, x1, VECTOR_SEW8, VECTOR_LMUL1, 1);
                    VSLIDEDOWN_VI(q3, q1, 16, VECTOR_UNMASKED);
                    VSLIDEDOWN_VI(q4, q3, u8, VECTOR_UNMASKED);
                }
                vector_vsetvli(dyn, ninst, x1, VECTOR_SEW8, VECTOR_LMUL1, 1);
                VSLIDEDOWN_VI(v0, q2, u8, VECTOR_UNMASKED);
                if (vex.l) {
                    vector_vsetvli(dyn, ninst, x1, VECTOR_SEW8, VECTOR_LMUL1, 2);
                    VSLIDEUP_VI_SAFE(v0, q4, 16, VECTOR_UNMASKED);
                }
            }
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        default:
            DEFAULT_VECTOR;
    }
    return addr;
}
