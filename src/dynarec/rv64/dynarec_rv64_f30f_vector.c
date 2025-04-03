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
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                v0 = sse_get_reg_vector(dyn, ninst, x1, gd, 1, VECTOR_SEW32);
                v1 = sse_get_reg_vector(dyn, ninst, x1, ed, 0, VECTOR_SEW32);
                if (rv64_xtheadvector) {
                    VECTOR_LOAD_VMASK(0b0001, x4, 1);
                    VMERGE_VVM(v0, v0, v1); // implies VMASK
                } else {
                    VMV_X_S(x4, v1);
                    VMV_S_X(v0, x4);
                }
            } else {
                SMREAD();
                v0 = sse_get_reg_empty_vector(dyn, ninst, x1, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                LWU(x4, ed, fixedaddress);
                VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
                VMV_S_X(v0, x4);
            }
            break;
        case 0x11:
            INST_NAME("MOVSS Ex, Gx");
            nextop = F8;
            GETG;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
            v0 = sse_get_reg_vector(dyn, ninst, x1, gd, 0, VECTOR_SEW32);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                d0 = sse_get_reg_vector(dyn, ninst, x1, ed, 1, VECTOR_SEW32);
                if (rv64_xtheadvector) {
                    VECTOR_LOAD_VMASK(0b0001, x4, 1);
                    VMERGE_VVM(v0, v0, v1); // implies VMASK
                } else {
                    VMV_X_S(x4, v1);
                    VMV_S_X(v0, x4);
                }
            } else {
                VMV_X_S(x4, v0);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                SW(x4, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0x12:
            INST_NAME("MOVSLDUP Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            if (MODREG) {
                q1 = sse_get_reg_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW64);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 0, 0);
                q1 = fpu_get_scratch(dyn);
                VLE64_V(q1, ed, VECTOR_UNMASKED, VECTOR_NFIELD1);
            }
            GETGX_empty_vector(q0);
            ADDI(x4, xZR, 32);
            v0 = fpu_get_scratch(dyn);
            VSLL_VX(q0, q1, x4, VECTOR_UNMASKED);
            VSRL_VX(v0, q0, x4, VECTOR_UNMASKED);
            VOR_VV(q0, q0, v0, VECTOR_UNMASKED);
            break;
        case 0x16:
            INST_NAME("MOVSHDUP Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            if (MODREG) {
                q1 = sse_get_reg_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW64);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 0, 0);
                q1 = fpu_get_scratch(dyn);
                VLE64_V(q1, ed, VECTOR_UNMASKED, VECTOR_NFIELD1);
            }
            GETGX_empty_vector(q0);
            ADDI(x4, xZR, 32);
            v0 = fpu_get_scratch(dyn);
            VSRL_VX(q0, q1, x4, VECTOR_UNMASKED);
            VSLL_VX(v0, q0, x4, VECTOR_UNMASKED);
            VOR_VV(q0, q0, v0, VECTOR_UNMASKED);
            break;
        case 0x1E:
            return 0;
        case 0x2A:
            INST_NAME("CVTSI2SS Gx, Ed");
            nextop = F8;
            if (rex.w) {
                SET_ELEMENT_WIDTH(x3, VECTOR_SEW64, 1);
                GETGX_vector(v0, 1, VECTOR_SEW64);
                GETED(0);
                FCVTSL(v0, ed, RD_RNE);
                SET_ELEMENT_WIDTH(x3, VECTOR_SEW32, 1);
            } else {
                SET_ELEMENT_WIDTH(x3, VECTOR_SEW32, 1);
                GETGX_vector(v0, 1, VECTOR_SEW32);
                GETED(0);
                FCVTSW(v0, ed, RD_RNE);
            }
            if (rv64_xtheadvector) {
                v1 = fpu_get_scratch(dyn);
                VFMV_S_F(v1, v0);
                VECTOR_LOAD_VMASK(0b0001, x4, 1);
                VMERGE_VVM(v0, v0, v1); // implies VMASK
            } else {
                VFMV_S_F(v0, v0);
            }
            break;
        case 0x2C:
            INST_NAME("CVTTSS2SI Gd, Ex");
            nextop = F8;
            GETGD;
            SET_ELEMENT_WIDTH(x3, VECTOR_SEW32, 1);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                d0 = sse_get_reg_vector(dyn, ninst, x1, ed, 0, VECTOR_SEW32);
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
            GETGD;
            SET_ELEMENT_WIDTH(x3, VECTOR_SEW32, 1);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                d0 = sse_get_reg_vector(dyn, ninst, x1, ed, 0, VECTOR_SEW32);
                VFMV_F_S(d0, d0);
            } else {
                GETEXSS(d0, 0);
            }
            if (!BOX64ENV(dynarec_fastround)) FSFLAGSI(0);
            u8 = sse_setround(dyn, ninst, x5, x6);
            FCVTSxw(gd, d0, RD_DYN);
            x87_restoreround(dyn, ninst, u8);
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
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
            if (MODREG) {
                GETGX_vector(v0, 1, VECTOR_SEW32);
                v1 = sse_get_reg_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW32);
            } else {
                SMREAD();
                v1 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                LWU(x4, ed, fixedaddress);
                VMV_S_X(v1, x4);
                GETGX_vector(v0, 1, VECTOR_SEW32);
            }
            VECTOR_LOAD_VMASK(0b0001, x4, 1);
            VFSQRT_V(v0, v1, VECTOR_MASKED);
            break;
        case 0x52:
            INST_NAME("RSQRTSS Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
            if (MODREG) {
                GETGX_vector(v0, 1, VECTOR_SEW32);
                v1 = sse_get_reg_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW32);
            } else {
                SMREAD();
                v1 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                LWU(x4, ed, fixedaddress);
                VMV_S_X(v1, x4);
                GETGX_vector(v0, 1, VECTOR_SEW32);
            }
            LUI(x4, 0x3f800);
            FMVWX(v1, x4); // 1.0f
            VECTOR_LOAD_VMASK(0b0001, x4, 1);
            VFSQRT_V(v0, v1, VECTOR_MASKED);
            VFRDIV_VF(v0, v0, v1, VECTOR_MASKED);
            break;
        case 0x53:
            INST_NAME("RCPSS Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
            if (MODREG) {
                GETGX_vector(v0, 1, VECTOR_SEW32);
                v1 = sse_get_reg_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW32);
            } else {
                SMREAD();
                v1 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                LWU(x4, ed, fixedaddress);
                VMV_S_X(v1, x4);
                GETGX_vector(v0, 1, VECTOR_SEW32);
            }
            LUI(x4, 0x3f800);
            FMVWX(v1, x4); // 1.0f
            VECTOR_LOAD_VMASK(0b0001, x4, 1);
            VFRDIV_VF(v0, v1, v1, VECTOR_MASKED);
            break;
        case 0x58:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("ADDSS Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
            if (MODREG) {
                GETGX_vector(v0, 1, VECTOR_SEW32);
                v1 = sse_get_reg_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW32);
            } else {
                SMREAD();
                v1 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                LWU(x4, ed, fixedaddress);
                VMV_S_X(v1, x4);
                GETGX_vector(v0, 1, VECTOR_SEW32);
            }
            VECTOR_LOAD_VMASK(0b0001, x4, 1);
            VFADD_VV(v0, v0, v1, VECTOR_MASKED);
            break;
        case 0x59:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("MULSS Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
            if (MODREG) {
                GETGX_vector(v0, 1, VECTOR_SEW32);
                v1 = sse_get_reg_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW32);
            } else {
                SMREAD();
                v1 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                LWU(x4, ed, fixedaddress);
                VMV_S_X(v1, x4);
                GETGX_vector(v0, 1, VECTOR_SEW32);
            }
            VECTOR_LOAD_VMASK(0b0001, x4, 1);
            VFMUL_VV(v0, v0, v1, VECTOR_MASKED);
            break;
        case 0x5A:
            INST_NAME("CVTSS2SD Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
            if (MODREG) {
                GETGX_vector(v0, 1, VECTOR_SEW32);
                v1 = sse_get_reg_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW32);
            } else {
                SMREAD();
                v1 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                LWU(x4, ed, fixedaddress);
                VMV_S_X(v1, x4);
                GETGX_vector(v0, 1, VECTOR_SEW32);
            }
            d0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            VECTOR_LOAD_VMASK(0b0001, x4, 1);
            VFWCVT_F_F_V(d0, v1, VECTOR_MASKED);
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            if (rv64_xtheadvector) {
                VMERGE_VVM(v0, v0, d0); // implies VMASK
            } else {
                VMV_X_S(x4, d0);
                VMV_S_X(v0, x4);
            }
            break;
        case 0x5B:
            if (!BOX64ENV(dynarec_fastround)) return 0;
            INST_NAME("CVTTPS2DQ Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
            GETEX_vector(v1, 0, 0, VECTOR_SEW32);
            GETGX_empty_vector(v0);
            if (rv64_xtheadvector) {
                ADDI(x4, xZR, 1); // RTZ
                FSRM(x4, x4);
                VFCVT_X_F_V(v0, v1, VECTOR_UNMASKED);
                FSRM(xZR, x4);
            } else {
                VFCVT_RTZ_X_F_V(v0, v1, VECTOR_UNMASKED);
            }
            break;
        case 0x5C:
            if (!BOX64ENV(dynarec_fastnan)) return 0;
            INST_NAME("SUBSS Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
            if (MODREG) {
                GETGX_vector(v0, 1, VECTOR_SEW32);
                v1 = sse_get_reg_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW32);
            } else {
                SMREAD();
                v1 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                LWU(x4, ed, fixedaddress);
                VMV_S_X(v1, x4);
                GETGX_vector(v0, 1, VECTOR_SEW32);
            }
            VECTOR_LOAD_VMASK(0b0001, x4, 1);
            VFSUB_VV(v0, v0, v1, VECTOR_MASKED);
            break;
        case 0x5D:
            INST_NAME("MINSS Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
            if (MODREG) {
                GETGX_vector(v0, 1, VECTOR_SEW32);
                v1 = sse_get_reg_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW32);
            } else {
                SMREAD();
                v1 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                LWU(x4, ed, fixedaddress);
                VMV_S_X(v1, x4);
                GETGX_vector(v0, 1, VECTOR_SEW32);
            }
            if (BOX64ENV(dynarec_fastnan)) {
                VECTOR_LOAD_VMASK(0b0001, x4, 1);
                VFMIN_VV(v0, v0, v1, VECTOR_MASKED);
            } else {
                d0 = fpu_get_scratch(dyn);
                d1 = fpu_get_scratch(dyn);
                VFMV_F_S(d0, v0);
                VFMV_F_S(d1, v1);
                FEQS(x2, d0, d0);
                FEQS(x3, d1, d1);
                AND(x2, x2, x3);
                BEQ_MARK(x2, xZR);
                FLES(x2, d1, d0);
                BEQ_MARK2(x2, xZR);
                MARK;
                FMVS(d0, d1);
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
            INST_NAME("DIVSS Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
            if (MODREG) {
                GETGX_vector(v0, 1, VECTOR_SEW32);
                v1 = sse_get_reg_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW32);
            } else {
                SMREAD();
                v1 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                LWU(x4, ed, fixedaddress);
                VMV_S_X(v1, x4);
                GETGX_vector(v0, 1, VECTOR_SEW32);
            }
            VECTOR_LOAD_VMASK(0b0001, x4, 1);
            VFDIV_VV(v0, v0, v1, VECTOR_MASKED);
            break;
        case 0x5F:
            INST_NAME("MAXSS Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
            if (MODREG) {
                GETGX_vector(v0, 1, VECTOR_SEW32);
                v1 = sse_get_reg_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW32);
            } else {
                SMREAD();
                v1 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                LWU(x4, ed, fixedaddress);
                VMV_S_X(v1, x4);
                GETGX_vector(v0, 1, VECTOR_SEW32);
            }
            if (BOX64ENV(dynarec_fastnan)) {
                VECTOR_LOAD_VMASK(0b0001, x4, 1);
                VFMAX_VV(v0, v0, v1, VECTOR_MASKED);
            } else {
                d0 = fpu_get_scratch(dyn);
                d1 = fpu_get_scratch(dyn);
                VFMV_F_S(d0, v0);
                VFMV_F_S(d1, v1);
                FEQS(x2, d0, d0);
                FEQS(x3, d1, d1);
                AND(x2, x2, x3);
                BEQ_MARK(x2, xZR);
                FLES(x2, d0, d1);
                BEQ_MARK2(x2, xZR);
                MARK;
                FMVS(d0, d1);
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
        case 0x6F:
            INST_NAME("MOVDQU Gx, Ex");
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
        case 0x70:
            INST_NAME("PSHUFHW Gx, Ex, Ib");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW16, 1);
            GETEX_vector(v1, 0, 1, VECTOR_SEW16);
            GETGX_vector(v0, 1, VECTOR_SEW16);
            u8 = F8;
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            vector_vsetvli(dyn, ninst, x1, VECTOR_SEW64, VECTOR_LMUL1, 1);
            tmp64u0 = ((((uint64_t)u8 >> 6) & 3) << 48) | ((((uint64_t)u8 >> 4) & 3) << 32) | (((u8 >> 2) & 3) << 16) | (u8 & 3);
            tmp64u0 += 0x0004000400040004ULL;
            MOV64x(x5, tmp64u0);
            VMV_S_X(d1, x5);
            tmp64u0 = 0x0003000200010000ULL;
            MOV64x(x5, tmp64u0);
            VSLIDE1UP_VX(d0, d1, x5, VECTOR_UNMASKED);
            vector_vsetvli(dyn, ninst, x1, VECTOR_SEW16, VECTOR_LMUL1, 1);
            if (v0 == v1) {
                v1 = fpu_get_scratch(dyn);
                VMV_V_V(v1, v0);
            }
            VRGATHER_VV(v0, v1, d0, VECTOR_UNMASKED);
            break;
        case 0x7E:
            INST_NAME("MOVQ Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
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
            if (!rv64_xtheadvector) VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
            VMV_S_X(v0, x4);
            break;
        case 0x7F:
            INST_NAME("MOVDQU Ex, Gx");
            nextop = F8;
            if (MODREG) {
                SET_ELEMENT_WIDTH(x1, VECTOR_SEWANY, 1);
                GETGX_vector(v0, 0, dyn->vector_eew);
                ed = (nextop & 7) + (rex.b << 3);
                v1 = sse_get_reg_empty_vector(dyn, ninst, x1, ed);
                VMV_V_V(v1, v0);
            } else {
                SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1); // unaligned!
                GETGX_vector(v0, 0, dyn->vector_eew);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                VSE_V(v0, ed, dyn->vector_eew, VECTOR_UNMASKED, VECTOR_NFIELD1);
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
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
            if (MODREG) {
                GETGX_vector(d0, 1, VECTOR_SEW32);
                d1 = sse_get_reg_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW32);
            } else {
                SMREAD();
                d1 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 1);
                LWU(x4, ed, fixedaddress);
                VXOR_VV(d1, d1, d1, VECTOR_UNMASKED);
                VMV_S_X(d1, x4);
                GETGX_vector(d0, 1, VECTOR_SEW32);
            }
            u8 = F8;
            VFMV_F_S(d0, d0);
            VFMV_F_S(d1, d1);
            if ((u8 & 7) == 0) { // Equal
                FEQS(x2, d0, d1);
            } else if ((u8 & 7) == 4) { // Not Equal or unordered
                FEQS(x2, d0, d1);
                XORI(x2, x2, 1);
            } else {
                // x2 = !(isnan(d0) || isnan(d1))
                FEQS(x3, d0, d0);
                FEQS(x2, d1, d1);
                AND(x2, x2, x3);
                switch (u8 & 7) {
                    case 1:
                        BEQ_MARK(x2, xZR);
                        FLTS(x2, d0, d1);
                        break; // Less than
                    case 2:
                        BEQ_MARK(x2, xZR);
                        FLES(x2, d0, d1);
                        break;                      // Less or equal
                    case 3: XORI(x2, x2, 1); break; // NaN
                    case 5: {                       // Greater or equal or unordered
                        BEQ_MARK2(x2, xZR);
                        FLES(x2, d1, d0);
                        B_MARK_nocond;
                        break;
                    }
                    case 6: { // Greater or unordered, test inverted, N!=V so unordered or less than (inverted)
                        BEQ_MARK2(x2, xZR);
                        FLTS(x2, d1, d0);
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
                VECTOR_LOAD_VMASK(0b0001, x4, 1);
                VMERGE_VVM(d0, d0, v0); // implies VMASK
            } else {
                VMV_S_X(d0, x2);
            }
            break;
        case 0xE6:
            INST_NAME("CVTDQ2PD Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            if (MODREG) {
                v1 = sse_get_reg_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW64);
                GETGX_empty_vector(v0);
            } else {
                SMREAD();
                v1 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                LD(x4, ed, fixedaddress);
                VMV_S_X(v1, x4);
                GETGX_empty_vector(v0);
            }
            vector_vsetvli(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL1, 0.5);
            d0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            VFWCVT_F_X_V(d0, v1, VECTOR_UNMASKED);
            vector_vsetvli(dyn, ninst, x1, VECTOR_SEW64, VECTOR_LMUL1, 1);
            VMV_V_V(v0, d0);
            break;
        default:
            DEFAULT_VECTOR;
    }
    return addr;
}
