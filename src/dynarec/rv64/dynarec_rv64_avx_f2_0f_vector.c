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
        case 0x10:
            INST_NAME("VMOVSD Gx, [Vx,] Ex");
            nextop = F8;
            d0 = fpu_get_scratch(dyn);
            if (MODREG) {
                q0 = fpu_get_scratch(dyn);
                avx_load_reg_vector(dyn, ninst, x1, q0, vex.v, 16, VECTOR_SEW64);
                q1 = fpu_get_scratch(dyn);
                avx_load_reg_vector(dyn, ninst, x1, q1, (nextop & 7) + (rex.b << 3), 16, VECTOR_SEW64);
                VFMV_F_S(d0, q1);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                FLD(d0, ed, fixedaddress);
            }
            GETGY_empty_vector(v0);
            if (MODREG) {
                if (v0 != q0)
                    VMV_V_V(v0, q0);
            } else {
                SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW64);
                VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
            }
            VFMV_S_F(v0, d0);
            avx_store_reg_vector(dyn, ninst, x1, v0, gd, 16, VECTOR_SEW64);
            break;
        case 0x11:
            INST_NAME("VMOVSD Ex, [Vx,] Gx");
            nextop = F8;
            gd = ((nextop & 0x38) >> 3) + (rex.r << 3);
            q0 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q0, gd, 16, VECTOR_SEW64);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                q2 = fpu_get_scratch(dyn);
                avx_load_reg_vector(dyn, ninst, x1, q2, vex.v, 16, VECTOR_SEW64);
                VFMV_F_S(d0 = fpu_get_scratch(dyn), q0);
                VFMV_S_F(q2, d0);
                avx_store_reg_vector(dyn, ninst, x1, q2, ed, 16, VECTOR_SEW64);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                VFMV_F_S(d0 = fpu_get_scratch(dyn), q0);
                FSD(d0, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0x58:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("VADDSD Gx, Vx, Ex");
            nextop = F8;
            q0 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q0, vex.v, 16, VECTOR_SEW64);
            d1 = fpu_get_scratch(dyn);
            if (MODREG) {
                q1 = fpu_get_scratch(dyn);
                avx_load_reg_vector(dyn, ninst, x1, q1, (nextop & 7) + (rex.b << 3), 16, VECTOR_SEW64);
                VFMV_F_S(d1, q1);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                FLD(d1, ed, fixedaddress);
            }
            d0 = fpu_get_scratch(dyn);
            VFMV_F_S(d0, q0);
            FADDD(d0, d0, d1);
            GETGY_empty_vector(v0);
            if (v0 != q0)
                VMV_V_V(v0, q0);
            VFMV_S_F(v0, d0);
            avx_store_reg_vector(dyn, ninst, x1, v0, gd, 16, VECTOR_SEW64);
            break;
        case 0x59:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("VMULSD Gx, Vx, Ex");
            nextop = F8;
            q0 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q0, vex.v, 16, VECTOR_SEW64);
            d1 = fpu_get_scratch(dyn);
            if (MODREG) {
                q1 = fpu_get_scratch(dyn);
                avx_load_reg_vector(dyn, ninst, x1, q1, (nextop & 7) + (rex.b << 3), 16, VECTOR_SEW64);
                VFMV_F_S(d1, q1);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                FLD(d1, ed, fixedaddress);
            }
            d0 = fpu_get_scratch(dyn);
            VFMV_F_S(d0, q0);
            FMULD(d0, d0, d1);
            GETGY_empty_vector(v0);
            if (v0 != q0)
                VMV_V_V(v0, q0);
            VFMV_S_F(v0, d0);
            avx_store_reg_vector(dyn, ninst, x1, v0, gd, 16, VECTOR_SEW64);
            break;
        case 0x5C:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("VSUBSD Gx, Vx, Ex");
            nextop = F8;
            q0 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q0, vex.v, 16, VECTOR_SEW64);
            d1 = fpu_get_scratch(dyn);
            if (MODREG) {
                q1 = fpu_get_scratch(dyn);
                avx_load_reg_vector(dyn, ninst, x1, q1, (nextop & 7) + (rex.b << 3), 16, VECTOR_SEW64);
                VFMV_F_S(d1, q1);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                FLD(d1, ed, fixedaddress);
            }
            d0 = fpu_get_scratch(dyn);
            VFMV_F_S(d0, q0);
            FSUBD(d0, d0, d1);
            GETGY_empty_vector(v0);
            if (v0 != q0)
                VMV_V_V(v0, q0);
            VFMV_S_F(v0, d0);
            avx_store_reg_vector(dyn, ninst, x1, v0, gd, 16, VECTOR_SEW64);
            break;
        case 0x5D:
            INST_NAME("VMINSD Gx, Vx, Ex");
            nextop = F8;
            q0 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q0, vex.v, 16, VECTOR_SEW64);
            d1 = fpu_get_scratch(dyn);
            if (MODREG) {
                q1 = fpu_get_scratch(dyn);
                avx_load_reg_vector(dyn, ninst, x1, q1, (nextop & 7) + (rex.b << 3), 16, VECTOR_SEW64);
                VFMV_F_S(d1, q1);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                FLD(d1, ed, fixedaddress);
            }
            d0 = fpu_get_scratch(dyn);
            VFMV_F_S(d0, q0);
            FLTD(x2, d0, d1);
            BNEZ(x2, 8);
            FMVD(d0, d1);
            GETGY_empty_vector(v0);
            if (v0 != q0)
                VMV_V_V(v0, q0);
            VFMV_S_F(v0, d0);
            avx_store_reg_vector(dyn, ninst, x1, v0, gd, 16, VECTOR_SEW64);
            break;
        case 0x5E:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("VDIVSD Gx, Vx, Ex");
            nextop = F8;
            q0 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q0, vex.v, 16, VECTOR_SEW64);
            d1 = fpu_get_scratch(dyn);
            if (MODREG) {
                q1 = fpu_get_scratch(dyn);
                avx_load_reg_vector(dyn, ninst, x1, q1, (nextop & 7) + (rex.b << 3), 16, VECTOR_SEW64);
                VFMV_F_S(d1, q1);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                FLD(d1, ed, fixedaddress);
            }
            d0 = fpu_get_scratch(dyn);
            VFMV_F_S(d0, q0);
            FDIVD(d0, d0, d1);
            GETGY_empty_vector(v0);
            if (v0 != q0)
                VMV_V_V(v0, q0);
            VFMV_S_F(v0, d0);
            avx_store_reg_vector(dyn, ninst, x1, v0, gd, 16, VECTOR_SEW64);
            break;
        case 0x5F:
            INST_NAME("VMAXSD Gx, Vx, Ex");
            nextop = F8;
            q0 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q0, vex.v, 16, VECTOR_SEW64);
            d1 = fpu_get_scratch(dyn);
            if (MODREG) {
                q1 = fpu_get_scratch(dyn);
                avx_load_reg_vector(dyn, ninst, x1, q1, (nextop & 7) + (rex.b << 3), 16, VECTOR_SEW64);
                VFMV_F_S(d1, q1);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                FLD(d1, ed, fixedaddress);
            }
            d0 = fpu_get_scratch(dyn);
            VFMV_F_S(d0, q0);
            FLTD(x2, d1, d0);
            BNEZ(x2, 8);
            FMVD(d0, d1);
            GETGY_empty_vector(v0);
            if (v0 != q0) VMV_V_V(v0, q0);
            VFMV_S_F(v0, d0);
            avx_store_reg_vector(dyn, ninst, x1, v0, gd, 16, VECTOR_SEW64);
            break;
        case 0x5A:
            INST_NAME("VCVTSD2SS Gx, Vx, Ex");
            nextop = F8;
            d0 = fpu_get_scratch(dyn);
            if (MODREG) {
                q1 = fpu_get_scratch(dyn);
                avx_load_reg_vector(dyn, ninst, x1, q1, (nextop & 7) + (rex.b << 3), 16, VECTOR_SEW64);
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
                LUI(x5, 0x7fc00);
                OR(x4, x4, x5);
                OR(x4, x4, x6);
                FMVWX(d1, x4);
                MARK;
            }
            q0 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q0, vex.v, 16, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            if (v0 != q0)
                VMV_V_V(v0, q0);
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW32);
            VFMV_S_F(v0, d1);
            avx_store_reg_vector(dyn, ninst, x1, v0, gd, 16, VECTOR_SEW32);
            break;
        case 0x2A:
            INST_NAME("VCVTSI2SD Gx, Vx, Ed");
            nextop = F8;
            q0 = fpu_get_scratch(dyn);
            avx_load_reg_vector(dyn, ninst, x1, q0, vex.v, 16, VECTOR_SEW64);
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
                FCVTDL(d0, ed, RD_DYN);
            else
                FCVTDW(d0, ed, RD_DYN);
            x87_restoreround(dyn, ninst, u8);
            GETGY_empty_vector(v0);
            if (v0 != q0)
                VMV_V_V(v0, q0);
            VFMV_S_F(v0, d0);
            avx_store_reg_vector(dyn, ninst, x1, v0, gd, 16, VECTOR_SEW64);
            break;
        case 0x2C:
            INST_NAME("VCVTTSD2SI Gd, Ex");
            nextop = F8;
            GETGDd;
            d0 = fpu_get_scratch(dyn);
            if (MODREG) {
                q0 = fpu_get_scratch(dyn);
                avx_load_reg_vector(dyn, ninst, x1, q0, (nextop & 7) + (rex.b << 3), 16, VECTOR_SEW64);
                VFMV_F_S(d0, q0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                FLD(d0, ed, fixedaddress);
            }
            if (!BOX64ENV(dynarec_fastround)) FSFLAGSI(0);
            if (rex.w)
                FCVTLD(gd, d0, RD_RTZ);
            else
                FCVTWD(gd, d0, RD_RTZ);
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
        case 0x70:
            INST_NAME("VPSHUFLW Gx, Ex, Ib");
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
                VECTOR_LOAD_VMASK(vex.l ? ((1 << k) | (1 << (8 + k))) : (1 << k), x4, vex.l ? 2 : 1);
                int delta = (int8_t)((u8 >> (k * 2)) & 3) - k;
                if (delta) VADD_VI(d0, q0, delta, VECTOR_UNMASKED);
                VMERGE_VVM(q2, q2, delta ? d0 : q0);
            }
            VRGATHER_VV_SAFE(v0, q1, q2, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW16);
            break;
        case 0x12:
            INST_NAME("VMOVDDUP Gx, Ex");
            nextop = F8;
            GETEY_vector_w(v1, 0, VECTOR_SEW64, vex.l ? 32 : 8);
            GETGY_empty_vector(v0);
            v2 = fpu_get_scratch(dyn);
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW64);
            VID_V(v2, VECTOR_UNMASKED);
            VAND_VI(v2, v2, 0x1e, VECTOR_UNMASKED);
            VRGATHER_VV_SAFE(v0, v1, v2, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW64);
            break;
        case 0x51:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("VSQRTSD Gx, Vx, Ex");
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
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                FLD(d0, ed, fixedaddress);
            }
            FSQRTD(d0, d0);
            GETGY_empty_vector(v0);
            if (v0 != q0)
                VMV_V_V(v0, q0);
            VFMV_S_F(v0, d0);
            avx_store_reg_vector(dyn, ninst, x1, v0, gd, 16, VECTOR_SEW64);
            break;
        case 0xF0:
            INST_NAME("VLDDQU Gx, Ex");
            nextop = F8;
            GETEY_vector(v1, 0, VECTOR_SEW8);
            GETGY_empty_vector(v0);
            VMV_V_V(v0, v1);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        default:
            DEFAULT_VECTOR;
    }
    return addr;
}
