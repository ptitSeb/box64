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
    int q0, q1;
    int d0, d1;
    int s0, s1;
    uint64_t tmp64u0, tmp64u1;
    int64_t fixedaddress, gdoffset;
    int unscaled;

    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(q0);
    MAYUSE(v0);
    MAYUSE(v1);

    switch (opcode) {
        case 0x10:
            INST_NAME("MOVSD Gx, Ex");
            nextop = F8;
            GETG;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                v0 = sse_get_reg_vector(dyn, ninst, x1, gd, 1, VECTOR_SEW64);
                v1 = sse_get_reg_vector(dyn, ninst, x1, ed, 0, VECTOR_SEW64);
                if (rv64_xtheadvector) {
                    VECTOR_LOAD_VMASK(0b01, x4, 1);
                    VMERGE_VVM(v0, v0, v1); // implies VMASK
                } else {
                    VMV_X_S(x4, v1);
                    VMV_S_X(v0, x4);
                }
            } else {
                SMREAD();
                v0 = sse_get_reg_empty_vector(dyn, ninst, x1, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                LD(x4, ed, fixedaddress);
                if (!rv64_xtheadvector) VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
                VMV_S_X(v0, x4);
            }
            break;
        case 0x11:
            INST_NAME("MOVSD Ex, Gx");
            nextop = F8;
            GETG;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            v0 = sse_get_reg_vector(dyn, ninst, x1, gd, 0, VECTOR_SEW64);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                d0 = sse_get_reg_vector(dyn, ninst, x1, ed, 1, VECTOR_SEW64);
                if (rv64_xtheadvector) {
                    VECTOR_LOAD_VMASK(0b01, x4, 1);
                    VMERGE_VVM(v0, v0, v1); // implies VMASK
                } else {
                    VMV_X_S(x4, v1);
                    VMV_S_X(v0, x4);
                }
            } else {
                VMV_X_S(x4, v0);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                SD(x4, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0x12:
            INST_NAME("MOVDDUP Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            if (MODREG) {
                v1 = sse_get_reg_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW64);
                GETGX_empty_vector(v0);
                VMV_X_S(x4, v1);
            } else {
                SMREAD();
                GETGX_empty_vector(v0);
                v1 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                LD(x4, ed, fixedaddress);
            }
            VMV_V_X(v0, x4);
            break;
        case 0x2A:
            INST_NAME("CVTSI2SD Gx, Ed");
            nextop = F8;
            if (rex.w) {
                SET_ELEMENT_WIDTH(x3, VECTOR_SEW64, 1);
                GETGX_vector(v0, 1, VECTOR_SEW64);
                GETED(0);
                FCVTDL(v0, ed, RD_RNE);
            } else {
                SET_ELEMENT_WIDTH(x3, VECTOR_SEW32, 1);
                GETGX_vector(v0, 1, VECTOR_SEW32);
                GETED(0);
                FCVTDW(v0, ed, RD_RNE);
                SET_ELEMENT_WIDTH(x3, VECTOR_SEW64, 1);
            }
            if (rv64_xtheadvector) {
                v1 = fpu_get_scratch(dyn);
                VFMV_S_F(v1, v0);
                VECTOR_LOAD_VMASK(0b01, x4, 1);
                VMERGE_VVM(v0, v0, v1); // implies VMASK
            } else {
                VFMV_S_F(v0, v0);
            }
            break;
        case 0x2C:
            INST_NAME("CVTTSD2SI Gd, Ex");
            nextop = F8;
            GETGD;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            if (MODREG) {
                v0 = sse_get_reg_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, dyn->vector_eew);
            } else {
                SMREAD();
                v0 = fpu_get_scratch(dyn);
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
            GETGD;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            if (MODREG) {
                v0 = sse_get_reg_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, dyn->vector_eew);
            } else {
                SMREAD();
                v0 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                LD(x4, ed, fixedaddress);
                VMV_S_X(v0, x4);
            }
            if (BOX64ENV(dynarec_fastround)) {
                VFMV_F_S(v0, v0);
                u8 = sse_setround(dyn, ninst, x2, x3);
                FCVTLDxw(gd, v0, RD_DYN);
                if (!rex.w) ZEROUP(gd);
                x87_restoreround(dyn, ninst, u8);
            } else {
                VFMV_F_S(v0, v0);
                FSFLAGSI(0); // // reset all bits
                u8 = sse_setround(dyn, ninst, x2, x3);
                FCVTLDxw(gd, v0, RD_DYN);
                if (!rex.w) ZEROUP(gd);
                x87_restoreround(dyn, ninst, u8);
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
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            if (MODREG) {
                GETGX_vector(v0, 1, VECTOR_SEW64);
                v1 = sse_get_reg_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW64);
            } else {
                SMREAD();
                GETGX_vector(v0, 1, VECTOR_SEW64);
                v1 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                LD(x4, ed, fixedaddress);
                VMV_S_X(v1, x4);
            }
            if (BOX64ENV(dynarec_fastnan)) {
                VECTOR_LOAD_VMASK(0b01, x4, 1);
                VFSQRT_V(v0, v1, VECTOR_MASKED);
            } else {
                d0 = fpu_get_scratch(dyn);
                d1 = fpu_get_scratch(dyn);
                VFSQRT_V(d1, v1, VECTOR_UNMASKED);
                FMVDX(d0, xZR);
                VMFLT_VF(VMASK, v1, d0, VECTOR_UNMASKED);
                VFSGNJN_VV(d1, d1, d1, VECTOR_MASKED);
                if (rv64_xtheadvector) {
                    VECTOR_LOAD_VMASK(0b01, x4, 1);
                    VMERGE_VVM(v0, v0, d1); // implies VMASK
                } else {
                    VMV_X_S(x4, d1);
                    VMV_S_X(v0, x4);
                }
            }
            break;
        case 0x58:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("ADDSD Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            if (MODREG) {
                GETGX_vector(v0, 1, VECTOR_SEW64);
                v1 = sse_get_reg_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW64);
            } else {
                SMREAD();
                v1 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                LD(x4, ed, fixedaddress);
                VMV_S_X(v1, x4);
                GETGX_vector(v0, 1, VECTOR_SEW64);
            }
            VECTOR_LOAD_VMASK(0b01, x4, 1);
            VFADD_VV(v0, v0, v1, VECTOR_MASKED);
            break;
        case 0x59:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("MULSD Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            if (MODREG) {
                GETGX_vector(v0, 1, VECTOR_SEW64);
                v1 = sse_get_reg_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW64);
            } else {
                SMREAD();
                v1 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                LD(x4, ed, fixedaddress);
                VMV_S_X(v1, x4);
                GETGX_vector(v0, 1, VECTOR_SEW64);
            }
            VECTOR_LOAD_VMASK(0b01, x4, 1);
            VFMUL_VV(v0, v0, v1, VECTOR_MASKED);
            break;
        case 0x5A:
            INST_NAME("CVTSD2SS Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            if (MODREG) {
                GETGX_vector(v0, 1, VECTOR_SEW64);
                v1 = sse_get_reg_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW64);
            } else {
                SMREAD();
                v1 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                LD(x4, ed, fixedaddress);
                VMV_S_X(v1, x4);
                GETGX_vector(v0, 1, VECTOR_SEW64);
            }
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
            VECTOR_LOAD_VMASK(0b0001, x4, 1);
            d0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            // as per section 5.2 Vector Operands of V-spec v1.0,
            // > A destination vector register group can overlap a source vector register group only if one of the following holds:
            // > - ...
            // > - The destination EEW is smaller than the source EEW and the overlap is in the lowest-numbered part of the source register group
            // > - ...
            if (v1 & 1 || v0 == v1 + 1) {
                d1 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                VMV_V_V(d1, v1);
                if (rv64_xtheadvector) {
                    VFNCVT_F_F_W(d0, d1, VECTOR_MASKED);
                    VMERGE_VVM(v0, v0, d0); // implies VMASK
                } else {
                    VFNCVT_F_F_W(v0, d1, VECTOR_MASKED);
                }
            } else {
                if (rv64_xtheadvector) {
                    VFNCVT_F_F_W(d0, v1, VECTOR_MASKED);
                    VMERGE_VVM(v0, v0, d0); // implies VMASK
                } else {
                    VFNCVT_F_F_W(v0, v1, VECTOR_MASKED);
                }
            }
            break;
        case 0x5C:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("SUBSD Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            if (MODREG) {
                GETGX_vector(v0, 1, VECTOR_SEW64);
                v1 = sse_get_reg_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW64);
            } else {
                SMREAD();
                v1 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                LD(x4, ed, fixedaddress);
                VMV_S_X(v1, x4);
                GETGX_vector(v0, 1, VECTOR_SEW64);
            }
            VECTOR_LOAD_VMASK(0b01, x4, 1);
            VFSUB_VV(v0, v0, v1, VECTOR_MASKED);
            break;
        case 0x5D:
            INST_NAME("MINSD Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            if (MODREG) {
                GETGX_vector(v0, 1, VECTOR_SEW64);
                v1 = sse_get_reg_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW64);
            } else {
                SMREAD();
                v1 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                LD(x4, ed, fixedaddress);
                VMV_S_X(v1, x4);
                GETGX_vector(v0, 1, VECTOR_SEW64);
            }
            if (BOX64ENV(dynarec_fastnan)) {
                VECTOR_LOAD_VMASK(0b01, x4, 1);
                VFMIN_VV(v0, v0, v1, VECTOR_MASKED);
            } else {
                d0 = fpu_get_scratch(dyn);
                d1 = fpu_get_scratch(dyn);
                VFMV_F_S(d0, v0);
                VFMV_F_S(d1, v1);
                FEQD(x2, d0, d0);
                FEQD(x3, d1, d1);
                AND(x2, x2, x3);
                BEQ_MARK(x2, xZR);
                FLED(x2, d1, d0);
                BEQ_MARK2(x2, xZR);
                MARK;
                FMVD(d0, d1);
                MARK2;
                if (rv64_xtheadvector) {
                    VFMV_S_F(d0, d0);
                    VECTOR_LOAD_VMASK(0b0001, x4, 1);
                    VMERGE_VVM(v0, v0, d0); // implies VMASK
                } else {
                    VFMV_S_F(v0, d0);
                }
            }
            break;
        case 0x5E:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("DIVSD Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            if (MODREG) {
                GETGX_vector(v0, 1, VECTOR_SEW64);
                v1 = sse_get_reg_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW64);
            } else {
                SMREAD();
                v1 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                LD(x4, ed, fixedaddress);
                VMV_S_X(v1, x4);
                GETGX_vector(v0, 1, VECTOR_SEW64);
            }
            VECTOR_LOAD_VMASK(0b01, x4, 1);
            VFDIV_VV(v0, v0, v1, VECTOR_MASKED);
            break;
        case 0x5F:
            INST_NAME("MAXSD Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            if (MODREG) {
                GETGX_vector(v0, 1, VECTOR_SEW64);
                v1 = sse_get_reg_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW64);
            } else {
                SMREAD();
                v1 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                LD(x4, ed, fixedaddress);
                VMV_S_X(v1, x4);
                GETGX_vector(v0, 1, VECTOR_SEW64);
            }
            if (BOX64ENV(dynarec_fastnan)) {
                VECTOR_LOAD_VMASK(0b01, x4, 1);
                VFMAX_VV(v0, v0, v1, VECTOR_MASKED);
            } else {
                d0 = fpu_get_scratch(dyn);
                d1 = fpu_get_scratch(dyn);
                VFMV_F_S(d0, v0);
                VFMV_F_S(d1, v1);
                FEQD(x2, d0, d0);
                FEQD(x3, d1, d1);
                AND(x2, x2, x3);
                BEQ_MARK(x2, xZR);
                FLED(x2, d0, d1);
                BEQ_MARK2(x2, xZR);
                MARK;
                FMVD(d0, d1);
                MARK2;
                if (rv64_xtheadvector) {
                    VFMV_S_F(d0, d0);
                    VECTOR_LOAD_VMASK(0b0001, x4, 1);
                    VMERGE_VVM(v0, v0, d0); // implies VMASK
                } else {
                    VFMV_S_F(v0, d0);
                }
            }
            break;
        case 0x70:
            INST_NAME("PSHUFLW Gx, Ex, Ib");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW16, 1);
            GETEX_vector(v1, 0, 1, VECTOR_SEW16);
            GETGX_vector(v0, 1, VECTOR_SEW16);
            u8 = F8;
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            vector_vsetvli(dyn, ninst, x1, VECTOR_SEW64, VECTOR_LMUL1, 1);
            tmp64u0 = 0x0007000600050004ULL;
            MOV64x(x5, tmp64u0);
            VMV_S_X(d1, x5);
            tmp64u0 = ((((uint64_t)u8 >> 6) & 3) << 48) | ((((uint64_t)u8 >> 4) & 3) << 32) | (((u8 >> 2) & 3) << 16) | (u8 & 3);
            MOV64x(x5, tmp64u0);
            VSLIDE1UP_VX(d0, d1, x5, VECTOR_UNMASKED);
            vector_vsetvli(dyn, ninst, x1, VECTOR_SEW16, VECTOR_LMUL1, 1);
            if (v0 == v1) {
                v1 = fpu_get_scratch(dyn);
                VMV_V_V(v1, v0);
            }
            VRGATHER_VV(v0, v1, d0, VECTOR_UNMASKED);
            break;
        case 0x7C:
            INST_NAME("HADDPS Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
            GETGX_vector(q0, 1, VECTOR_SEW32);
            GETEX_vector(q1, 0, 0, VECTOR_SEW32);
            v0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            d1 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            d0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            VMV_V_V(v0, q0);
            if (q1 & 1) VMV_V_V(d1, q1);
            vector_vsetvli(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL2, 2);
            VSLIDEUP_VI(v0, (q1 & 1) ? d1 : q1, 4, VECTOR_UNMASKED);
            vector_vsetvli(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL1, 1);
            ADDI(x4, xZR, 32);
            VNSRL_WX(d0, v0, xZR, VECTOR_UNMASKED);
            VNSRL_WX(d1, v0, x4, VECTOR_UNMASKED);
            VFADD_VV(q0, d1, d0, VECTOR_UNMASKED);
            break;
        case 0xC2:
            INST_NAME("CMPSD Gx, Ex, Ib");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            if (MODREG) {
                GETGX_vector(d0, 1, VECTOR_SEW64);
                d1 = sse_get_reg_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW64);
            } else {
                SMREAD();
                d1 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 1);
                LD(x4, ed, fixedaddress);
                VMV_S_X(d1, x4);
                GETGX_vector(d0, 1, VECTOR_SEW64);
            }
            u8 = F8;
            VFMV_F_S(d0, d0);
            VFMV_F_S(d1, d1);
            if ((u8 & 7) == 0) { // Equal
                FEQD(x2, d0, d1);
            } else if ((u8 & 7) == 4) { // Not Equal or unordered
                FEQD(x2, d0, d1);
                XORI(x2, x2, 1);
            } else {
                // x2 = !(isnan(d0) || isnan(d1))
                FEQD(x3, d0, d0);
                FEQD(x2, d1, d1);
                AND(x2, x2, x3);
                switch (u8 & 7) {
                    case 1:
                        BEQ_MARK(x2, xZR);
                        FLTD(x2, d0, d1);
                        break; // Less than
                    case 2:
                        BEQ_MARK(x2, xZR);
                        FLED(x2, d0, d1);
                        break;                      // Less or equal
                    case 3: XORI(x2, x2, 1); break; // NaN
                    case 5: {                       // Greater or equal or unordered
                        BEQ_MARK2(x2, xZR);
                        FLED(x2, d1, d0);
                        B_MARK_nocond;
                        break;
                    }
                    case 6: { // Greater or unordered, test inverted, N!=V so unordered or less than (inverted)
                        BEQ_MARK2(x2, xZR);
                        FLTD(x2, d1, d0);
                        B_MARK_nocond;
                        break;
                    }
                    case 7: break; // Not NaN
                }

                MARK2;
                if ((u8 & 7) == 5 || (u8 & 7) == 6) {
                    MOV32w(x2, 1);
                }
                MARK;
            }
            NEG(x2, x2);
            if (rv64_xtheadvector) {
                v0 = fpu_get_scratch(dyn);
                VMV_S_X(v0, x2);
                VECTOR_LOAD_VMASK(0b01, x4, 1);
                VMERGE_VVM(d0, d0, v0); // implies VMASK
            } else {
                VMV_S_X(d0, x2);
            }
            break;
        case 0xE6:
            if (!BOX64ENV(dynarec_fastround)) return 0;
            INST_NAME("CVTPD2DQ Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            GETEX_vector(v1, 0, 0, VECTOR_SEW64);
            GETGX_empty_vector(v0);
            if (v1 & 1) {
                d1 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                VMV_V_V(d1, v1);
            } else {
                d1 = v1;
            }
            vector_vsetvli(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL1, 0.5);
            d0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            u8 = sse_setround(dyn, ninst, x6, x4);
            VFNCVT_X_F_W(d0, d1, VECTOR_UNMASKED);
            x87_restoreround(dyn, ninst, u8);
            vector_vsetvli(dyn, ninst, x1, VECTOR_SEW64, VECTOR_LMUL1, 1);
            if (!rv64_xtheadvector) VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
            VMV_X_S(x4, d0);
            VMV_S_X(v0, x4);
            break;
        case 0xF0:
            INST_NAME("LDDQU Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG) {
                SET_ELEMENT_WIDTH(x1, VECTOR_SEWANY, 1);
                ed = (nextop & 7) + (rex.b << 3);
                v1 = sse_get_reg_vector(dyn, ninst, x1, ed, 0, dyn->vector_eew);
                v0 = sse_get_reg_empty_vector(dyn, ninst, x1, gd);
                VMV_V_V(v0, v1);
            } else {
                SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1); // unaligned!
                SMREAD();
                v0 = sse_get_reg_empty_vector(dyn, ninst, x1, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                VLE_V(v0, ed, dyn->vector_eew, VECTOR_UNMASKED, VECTOR_NFIELD1);
            }
            break;
        default: DEFAULT_VECTOR;
    }
    return addr;
}
