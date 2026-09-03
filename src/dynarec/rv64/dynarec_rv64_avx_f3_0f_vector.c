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
    int q0, q1, q2, d0, d1;
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
        case 0x12:
            INST_NAME("VMOVSLDUP Gx, Ex");
            nextop = F8;
            GETEY_vector(v1, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            v2 = fpu_get_scratch(dyn);
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW32);
            VID_V(v2, VECTOR_UNMASKED);
            VAND_VI(v2, v2, 0x1e, VECTOR_UNMASKED);
            VRGATHER_VV(v0, v1, v2, VECTOR_UNMASKED);
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
            VRGATHER_VV(v0, v1, v2, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x6F:
            INST_NAME("VMOVDQU Gx, Ex");
            nextop = F8;
            GETEY_vector(v1, 0, VECTOR_SEW8);
            GETGY_empty_vector(v0);
            VMV_V_V(v0, v1);
            PUTGY_vector(v0, VECTOR_SEW8);
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
            q0 = fpu_get_scratch(dyn);
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
            u8 = sse_setround(dyn, ninst, x5, x6);
            d0 = fpu_get_scratch(dyn);
            if (rex.w)
                FCVTSL(d0, ed, RD_DYN);
            else
                FCVTSW(d0, ed, RD_DYN);
            x87_restoreround(dyn, ninst, u8);
            GETGY_empty_vector(v0);
            if (v0 != q0)
                VMV_V_V(v0, q0);
            VFMV_S_F(v0, d0);
            avx_store_reg_vector(dyn, ninst, x1, v0, gd, 16, VECTOR_SEW32);
            break;
        case 0x2C:
            INST_NAME("VCVTTSS2SI Gd, Ex");
            nextop = F8;
            GETGD;
            d0 = fpu_get_scratch(dyn);
            if (MODREG) {
                q0 = fpu_get_scratch(dyn);
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
        case 0x51:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("VSQRTSS Gx, Vx, Ex");
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
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                FLW(d0, ed, fixedaddress);
            }
            FSQRTS(d0, d0);
            GETGY_empty_vector(v0);
            if (v0 != q0)
                VMV_V_V(v0, q0);
            VFMV_S_F(v0, d0);
            avx_store_reg_vector(dyn, ninst, x1, v0, gd, 16, VECTOR_SEW32);
            break;
        case 0x58:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("VADDSS Gx, Vx, Ex");
            nextop = F8;
            q0 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q0, vex.v, 16, VECTOR_SEW32);
            d1 = fpu_get_scratch(dyn);
            if (MODREG) {
                q1 = fpu_get_scratch(dyn);
                avx_load_reg_vector(dyn, ninst, x1, q1, (nextop & 7) + (rex.b << 3), 16, VECTOR_SEW32);
                VFMV_F_S(d1, q1);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                FLW(d1, ed, fixedaddress);
            }
            d0 = fpu_get_scratch(dyn);
            VFMV_F_S(d0, q0);
            FADDS(d0, d0, d1);
            GETGY_empty_vector(v0);
            if (v0 != q0)
                VMV_V_V(v0, q0);
            VFMV_S_F(v0, d0);
            avx_store_reg_vector(dyn, ninst, x1, v0, gd, 16, VECTOR_SEW32);
            break;
        case 0x59:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("VMULSS Gx, Vx, Ex");
            nextop = F8;
            q0 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q0, vex.v, 16, VECTOR_SEW32);
            d1 = fpu_get_scratch(dyn);
            if (MODREG) {
                q1 = fpu_get_scratch(dyn);
                avx_load_reg_vector(dyn, ninst, x1, q1, (nextop & 7) + (rex.b << 3), 16, VECTOR_SEW32);
                VFMV_F_S(d1, q1);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                FLW(d1, ed, fixedaddress);
            }
            d0 = fpu_get_scratch(dyn);
            VFMV_F_S(d0, q0);
            FMULS(d0, d0, d1);
            GETGY_empty_vector(v0);
            if (v0 != q0)
                VMV_V_V(v0, q0);
            VFMV_S_F(v0, d0);
            avx_store_reg_vector(dyn, ninst, x1, v0, gd, 16, VECTOR_SEW32);
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
            q0 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q0, vex.v, 16, VECTOR_SEW32);
            d1 = fpu_get_scratch(dyn);
            if (MODREG) {
                q1 = fpu_get_scratch(dyn);
                avx_load_reg_vector(dyn, ninst, x1, q1, (nextop & 7) + (rex.b << 3), 16, VECTOR_SEW32);
                VFMV_F_S(d1, q1);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                FLW(d1, ed, fixedaddress);
            }
            d0 = fpu_get_scratch(dyn);
            VFMV_F_S(d0, q0);
            FSUBS(d0, d0, d1);
            GETGY_empty_vector(v0);
            if (v0 != q0)
                VMV_V_V(v0, q0);
            VFMV_S_F(v0, d0);
            avx_store_reg_vector(dyn, ninst, x1, v0, gd, 16, VECTOR_SEW32);
            break;
        case 0x5E:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("VDIVSS Gx, Vx, Ex");
            nextop = F8;
            q0 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q0, vex.v, 16, VECTOR_SEW32);
            d1 = fpu_get_scratch(dyn);
            if (MODREG) {
                q1 = fpu_get_scratch(dyn);
                avx_load_reg_vector(dyn, ninst, x1, q1, (nextop & 7) + (rex.b << 3), 16, VECTOR_SEW32);
                VFMV_F_S(d1, q1);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                FLW(d1, ed, fixedaddress);
            }
            d0 = fpu_get_scratch(dyn);
            VFMV_F_S(d0, q0);
            FDIVS(d0, d0, d1);
            GETGY_empty_vector(v0);
            if (v0 != q0)
                VMV_V_V(v0, q0);
            VFMV_S_F(v0, d0);
            avx_store_reg_vector(dyn, ninst, x1, v0, gd, 16, VECTOR_SEW32);
            break;
        case 0x70:
            INST_NAME("VPSHUFHW Gx, Ex, Ib");
            nextop = F8;
            GETEY_vector(q1, 1, VECTOR_SEW16);
            GETGY_empty_vector(v0);
            u8 = F8;
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW16);
            q0 = fpu_get_scratch(dyn);
            q2 = fpu_get_scratch(dyn);
            d0 = fpu_get_scratch(dyn);
            VID_V(q0, VECTOR_UNMASKED);
            VMV_V_V(q2, q0);
            for (int k = 0; k < 4; ++k) {
                VECTOR_LOAD_VMASK(vex.l ? ((1 << (4 + k)) | (1 << (12 + k))) : (1 << (4 + k)), x4, vex.l ? 2 : 1);
                MOV32w(x4, (int8_t)((u8 >> (k * 2)) & 3) - k);
                VADD_VX(d0, q0, x4, VECTOR_UNMASKED);
                VMERGE_VVM(q2, q2, d0);
            }
            VRGATHER_VV(v0, q1, q2, VECTOR_UNMASKED);
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
        default:
            DEFAULT_VECTOR;
    }
    return addr;
}
