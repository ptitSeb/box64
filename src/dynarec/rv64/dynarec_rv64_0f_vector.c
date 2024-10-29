#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "dynarec.h"
#include "emu/x64emu_private.h"
#include "emu/x64run_private.h"
#include "x64run.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"
#include "my_cpuid.h"
#include "emu/x87emu_private.h"
#include "emu/x64shaext.h"
#include "bitutils.h"

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_functions.h"
#include "dynarec_rv64_helper.h"

uintptr_t dynarec64_0F_vector(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed;
    uint8_t wb1, wback, wb2, gback;
    uint8_t eb1, eb2;
    uint8_t gb1, gb2;
    int32_t i32, i32_;
    int cacheupd = 0;
    int v0, v1;
    int q0, q1;
    int d0, d1;
    int s0, s1;
    uint64_t tmp64u0, tmp64u1;
    int64_t j64;
    int64_t fixedaddress, gdoffset;
    int unscaled;
    MAYUSE(wb2);
    MAYUSE(gback);
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(s0);
    MAYUSE(j64);
    MAYUSE(cacheupd);

    switch (opcode) {
        case 0x10:
            INST_NAME("MOVUPS Gx, Ex");
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
        case 0x11:
            INST_NAME("MOVUPS Ex, Gx");
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
        case 0x12:
            nextop = F8;
            if (MODREG) {
                INST_NAME("MOVHLPS Gx, Ex");
                SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
                GETGX_vector(v0, 1, VECTOR_SEW64);
                GETEX_vector(v1, 0, 0, VECTOR_SEW64);
                q0 = fpu_get_scratch(dyn);
                VSLIDEDOWN_VI(q0, v1, 1, VECTOR_UNMASKED);
                if (rv64_xtheadvector) {
                    VECTOR_LOAD_VMASK(0b01, x4, 1);
                    VMERGE_VVM(v0, v0, q0); // implies VMASK
                } else {
                    VMV_X_S(x4, q0);
                    VMV_S_X(v0, x4);
                }
            } else {
                INST_NAME("MOVLPS Gx, Ex");
                SMREAD();
                SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1); // unaligned!
                GETGX_vector(v0, 1, VECTOR_SEW8);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 0, 0);
                VECTOR_LOAD_VMASK(0xFF, x4, 1);
                VLE8_V(v0, ed, VECTOR_MASKED, VECTOR_NFIELD1);
            }
            break;
        case 0x14:
            INST_NAME("UNPCKLPS Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
            GETGX_vector(q0, 1, VECTOR_SEW32);
            GETEX_vector(q1, 0, 0, VECTOR_SEW32);
            if (q0 == q1) {
                q1 = fpu_get_scratch(dyn);
                VMV_V_V(q1, q0);
            }
            v0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            v1 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            d0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2); // no more scratches!
            // Zvbb VWSLL would help here....
            VWADDU_VX(v0, q0, xZR, VECTOR_UNMASKED);
            VWADDU_VX(v1, q1, xZR, VECTOR_UNMASKED);
            VSLIDE1UP_VX(d0, v1, xZR, VECTOR_UNMASKED);
            VOR_VV(q0, v0, d0, VECTOR_UNMASKED);
            break;
        case 0x15:
            INST_NAME("UNPCKHPS Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
            GETGX_vector(q0, 1, VECTOR_SEW32);
            GETEX_vector(q1, 0, 0, VECTOR_SEW32);
            if (q0 == q1) {
                q1 = fpu_get_scratch(dyn);
                VMV_V_V(q1, q0);
            }
            v0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            v1 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            d0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2); // no more scratches!
            if (rv64_vlen >= 256) {
                VWADDU_VX(v0, q0, xZR, VECTOR_UNMASKED);
                VWADDU_VX(v1, q1, xZR, VECTOR_UNMASKED);
                vector_vsetvli(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL2, 2);
                VSLIDEDOWN_VI(d0, v1, 3, VECTOR_UNMASKED);
                VSLIDEDOWN_VI(v1, v0, 4, VECTOR_UNMASKED);
                vector_vsetvli(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL1, 1);
                VOR_VV(q0, v1, d0, VECTOR_UNMASKED);
            } else {
                // Zvbb VWSLL would help here....
                VWADDU_VX(v0, q0, xZR, VECTOR_UNMASKED);
                VWADDU_VX(v1, q1, xZR, VECTOR_UNMASKED);
                VSLIDE1UP_VX(d0, v1 + 1, xZR, VECTOR_UNMASKED);
                VOR_VV(q0, v0 + 1, d0, VECTOR_UNMASKED);
            }
            break;
        case 0x16:
            nextop = F8;
            if (MODREG) {
                INST_NAME("MOVLHPS Gx, Ex");
                SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
                GETGX_vector(v0, 1, VECTOR_SEW64);
                v1 = sse_get_reg_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW64);
                if (v0 == v1) {
                    // for vslideup.vi, cannot be overlapped
                    v1 = fpu_get_scratch(dyn);
                    VMV_V_V(v1, v0);
                }
                VSLIDEUP_VI(v0, v1, 1, VECTOR_UNMASKED);
            } else {
                INST_NAME("MOVHPS Gx, Ex");
                SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
                GETGX_vector(v0, 1, VECTOR_SEW8);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, x2, &fixedaddress, rex, NULL, 0, 0);
                v1 = fpu_get_scratch(dyn);
                VECTOR_LOAD_VMASK(0xFF, x4, 1);
                VLE8_V(v1, ed, VECTOR_MASKED, VECTOR_NFIELD1);
                VSLIDEUP_VI(v0, v1, 8, VECTOR_UNMASKED);
            }
            break;
        case 0x17:
            INST_NAME("MOVHPS Ex, Gx");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            GETGX_vector(v0, 1, VECTOR_SEW64);
            // EX->q[0] = GX->q[1];
            if (MODREG) {
                v1 = sse_get_reg_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW64);
                q0 = fpu_get_scratch(dyn);
                VSLIDE1DOWN_VX(q0, v0, xZR, VECTOR_UNMASKED);
                if (rv64_xtheadvector) {
                    VECTOR_LOAD_VMASK(0b01, x4, 1);
                    VMERGE_VVM(v1, v1, q0); // implies VMASK
                } else {
                    VMV_X_S(x4, q0);
                    VMV_S_X(v1, x4);
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                q0 = fpu_get_scratch(dyn);
                VSLIDE1DOWN_VX(q0, v0, xZR, VECTOR_UNMASKED);
                VMV_X_S(x4, q0);
                SD(x4, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0x28:
            INST_NAME("MOVAPS Gx, Ex");
            nextop = F8;
            GETG;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEWANY, 1);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                v1 = sse_get_reg_vector(dyn, ninst, x1, ed, 0, dyn->vector_eew);
                v0 = sse_get_reg_empty_vector(dyn, ninst, x1, gd);
                VMV_V_V(v0, v1);
            } else {
                SMREAD();
                v0 = sse_get_reg_empty_vector(dyn, ninst, x1, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                VLE_V(v0, ed, dyn->vector_eew, VECTOR_UNMASKED, VECTOR_NFIELD1);
            }
            break;
        case 0x29:
            INST_NAME("MOVAPS Ex, Gx");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEWANY, 1);
            GETGX_vector(v0, 0, dyn->vector_eew);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                v1 = sse_get_reg_empty_vector(dyn, ninst, x1, ed);
                VMV_V_V(v1, v0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                VSE_V(v0, ed, dyn->vector_eew, VECTOR_UNMASKED, VECTOR_NFIELD1);
                SMWRITE2();
            }
            break;
        case 0x51:
            INST_NAME("SQRTPS Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
            GETEX_vector(v0, 0, 0, VECTOR_SEW32);
            GETGX_empty_vector(v1);
            VFSQRT_V(v1, v0, VECTOR_UNMASKED);
            break;
        case 0x53:
            INST_NAME("RCPPS Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
            GETEX_vector(v0, 0, 0, VECTOR_SEW32);
            GETGX_empty_vector(v1);
            LUI(x4, 0x3f800);
            d0 = fpu_get_scratch(dyn);
            FMVWX(d0, x4); // 1.0f
            VFRDIV_VF(v1, v0, d0, VECTOR_UNMASKED);
            break;
        case 0x54:
            INST_NAME("ANDPS Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
            GETEX_vector(q0, 0, 0, VECTOR_SEW32);
            GETGX_vector(v0, 1, VECTOR_SEW32);
            VAND_VV(v0, v0, q0, VECTOR_UNMASKED);
            break;
        case 0x55:
            INST_NAME("ANDNPS Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
            GETEX_vector(q0, 0, 0, VECTOR_SEW32);
            GETGX_vector(v0, 1, VECTOR_SEW32);
            VXOR_VI(v0, v0, 0x1f, VECTOR_UNMASKED);
            VAND_VV(v0, v0, q0, VECTOR_UNMASKED);
            break;
        case 0x56:
            INST_NAME("ORPS Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
            GETEX_vector(q0, 0, 0, VECTOR_SEW32);
            GETGX_vector(v0, 1, VECTOR_SEW32);
            VOR_VV(v0, v0, q0, VECTOR_UNMASKED);
            break;
        case 0x57:
            INST_NAME("XORPS Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEWANY, 1);
            GETG;
            if (MODREG && ((nextop & 7) + (rex.b << 3) == gd)) {
                // special case for XORPS Gx, Gx
                q0 = sse_get_reg_empty_vector(dyn, ninst, x1, gd);
                VXOR_VV(q0, q0, q0, VECTOR_UNMASKED);
            } else {
                q0 = sse_get_reg_vector(dyn, ninst, x1, gd, 1, dyn->vector_eew);
                GETEX_vector(q1, 0, 0, dyn->vector_eew);
                VXOR_VV(q0, q1, q0, VECTOR_UNMASKED);
            }
            break;
        case 0x58:
            INST_NAME("ADDPS Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
            GETEX_vector(q0, 0, 0, VECTOR_SEW32);
            GETGX_vector(v0, 1, VECTOR_SEW32);
            VFADD_VV(v0, v0, q0, VECTOR_UNMASKED);
            break;
        case 0x59:
            INST_NAME("MULPS Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
            GETEX_vector(q0, 0, 0, VECTOR_SEW32);
            GETGX_vector(v0, 1, VECTOR_SEW32);
            VFMUL_VV(v0, v0, q0, VECTOR_UNMASKED);
            break;
        case 0x5B:
            INST_NAME("CVTDQ2PS Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
            GETEX_vector(q0, 0, 0, VECTOR_SEW32);
            GETGX_empty_vector(v0);
            VFCVT_F_X_V(v0, q0, VECTOR_UNMASKED);
            break;
        case 0x5C:
            INST_NAME("SUBPS Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
            GETEX_vector(q0, 0, 0, VECTOR_SEW32);
            GETGX_vector(v0, 1, VECTOR_SEW32);
            VFSUB_VV(v0, v0, q0, VECTOR_UNMASKED);
            break;
        case 0x5D:
            INST_NAME("MINPS Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
            GETGX_vector(q0, 1, VECTOR_SEW32);
            GETEX_vector(q1, 0, 0, VECTOR_SEW32);
            if (!box64_dynarec_fastnan) {
                v0 = fpu_get_scratch(dyn);
                VMFEQ_VV(VMASK, q0, q0, VECTOR_UNMASKED);
                VMFEQ_VV(v0, q1, q1, VECTOR_UNMASKED);
            }
            VFMIN_VV(q0, q0, q1, VECTOR_UNMASKED);
            if (!box64_dynarec_fastnan) {
                VMAND_MM(VMASK, v0, VMASK);
                VXOR_VI(VMASK, VMASK, 0x1F, VECTOR_UNMASKED);
                VADD_VX(q0, q1, xZR, VECTOR_MASKED);
            }
            break;
        case 0x5E:
            INST_NAME("DIVPS Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
            GETGX_vector(q0, 1, VECTOR_SEW32);
            GETEX_vector(q1, 0, 0, VECTOR_SEW32);
            VFDIV_VV(q0, q0, q1, VECTOR_UNMASKED);
            break;
        case 0x5F:
            INST_NAME("MAXPS Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
            GETGX_vector(q0, 1, VECTOR_SEW32);
            GETEX_vector(q1, 0, 0, VECTOR_SEW32);
            v0 = fpu_get_scratch(dyn);
            if (!box64_dynarec_fastnan) {
                VMFEQ_VV(VMASK, q0, q0, VECTOR_UNMASKED);
                VMFEQ_VV(v0, q1, q1, VECTOR_UNMASKED);
            }
            VFMAX_VV(q0, q0, q1, VECTOR_UNMASKED);
            if (!box64_dynarec_fastnan) {
                VMAND_MM(VMASK, v0, VMASK);
                VXOR_VI(VMASK, VMASK, 0x1F, VECTOR_UNMASKED);
                VADD_VX(q0, q1, xZR, VECTOR_MASKED);
            }
            break;
        case 0xC2:
            INST_NAME("CMPPS Gx, Ex, Ib");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
            GETGX_vector(v0, 1, VECTOR_SEW32);
            GETEX_vector(v1, 0, 1, VECTOR_SEW32);
            u8 = F8;
            if ((u8 & 7) == 0) { // Equal
                VMFEQ_VV(VMASK, v0, v1, VECTOR_UNMASKED);
            } else if ((u8 & 7) == 4) { // Not Equal or unordered
                VMFEQ_VV(VMASK, v0, v1, VECTOR_UNMASKED);
                VXOR_VI(VMASK, VMASK, 0x1F, VECTOR_UNMASKED);
            } else {
                d0 = fpu_get_scratch(dyn);
                VMFEQ_VV(VMASK, v0, v0, VECTOR_UNMASKED);
                VMFEQ_VV(d0, v1, v1, VECTOR_UNMASKED);
                VMAND_MM(VMASK, VMASK, d0);
                switch (u8 & 7) {
                    case 1: // Less than
                        VMFLT_VV(d0, v0, v1, VECTOR_UNMASKED);
                        VMAND_MM(VMASK, VMASK, d0);
                        break;
                    case 2: // Less or equal
                        VMFLE_VV(d0, v0, v1, VECTOR_UNMASKED);
                        VMAND_MM(VMASK, VMASK, d0);
                        break;
                    case 3: // NaN
                        VXOR_VI(VMASK, VMASK, 0x1F, VECTOR_UNMASKED);
                        break;
                    case 5: // Greater or equal or unordered
                        VMFLE_VV(d0, v1, v0, VECTOR_UNMASKED);
                        VMORN_MM(VMASK, d0, VMASK);
                        break;
                    case 6: // Greater or unordered, test inverted, N!=V so unordered or less than (inverted)
                        VMFLT_VV(d0, v1, v0, VECTOR_UNMASKED);
                        VMORN_MM(VMASK, d0, VMASK);
                        break;
                    case 7: // Not NaN
                        break;
                }
            }
            VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
            VXOR_VI(v0, v0, 0x1F, VECTOR_MASKED);
            break;
        case 0xC6:
            INST_NAME("SHUFPS Gx, Ex, Ib");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
            GETGX_vector(v0, 1, VECTOR_SEW32);
            GETEX_vector(v1, 0, 1, VECTOR_SEW32);
            u8 = F8;
            q0 = fpu_get_scratch(dyn);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            if (rv64_xtheadvector) { // lack of vrgatherei16.vv
                q1 = fpu_get_scratch(dyn);
                vector_vsetvli(dyn, ninst, x1, VECTOR_SEW64, VECTOR_LMUL1, 1);
                tmp64u0 = ((((uint64_t)u8 >> 2) & 3) << 32) | (u8 & 3);
                VECTOR_SPLAT_IMM(q0, tmp64u0, x4);
                tmp64u1 = ((((uint64_t)u8 >> 6) & 3) << 32) | ((u8 >> 4) & 3);
                if (tmp64u1 != tmp64u0) {
                    VECTOR_SPLAT_IMM(q1, tmp64u1, x4);
                } else {
                    q1 = q0;
                }
                vector_vsetvli(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL1, 1);
                VRGATHER_VV(d0, v0, q0, VECTOR_UNMASKED);
                VRGATHER_VV(d1, v1, q1, VECTOR_UNMASKED);
            } else {
                tmp64u0 = ((((uint64_t)u8 >> 2) & 3) << 16) | (u8 & 3);
                VECTOR_SPLAT_IMM(q0, tmp64u0, x4);
                VRGATHEREI16_VV(d0, v0, q0, VECTOR_UNMASKED);
                tmp64u1 = ((((uint64_t)u8 >> 6) & 3) << 16) | ((u8 >> 4) & 3);
                if (tmp64u1 != tmp64u0) {
                    VECTOR_SPLAT_IMM(q0, tmp64u1, x4);
                }
                VRGATHEREI16_VV(d1, v1, q0, VECTOR_UNMASKED);
            }
            VMV_V_V(v0, d0);
            VSLIDEUP_VI(v0, d1, 2, VECTOR_UNMASKED);
            break;
        case 0x00 ... 0x0F:
        case 0x18:
        case 0x1F:
        case 0x2C ... 0x2F:
        case 0x31:
        case 0x40 ... 0x4F:
        case 0x60 ... 0x7F:
        case 0x80 ... 0xBF:
        case 0xC8 ... 0xCF:
            return 0;
        default:
            DEFAULT_VECTOR;
    }
    return addr;
}
