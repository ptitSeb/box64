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
        case 0x13:
            INST_NAME("MOVLPS Ex, Gx");
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
        case 0x2B:
            INST_NAME("MOVNTPS Ex, Gx");
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
        case 0x50:
            INST_NAME("MOVMSKPS Gd, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
            GETGD;
            GETEX_vector(q0, 0, 0, VECTOR_SEW32);
            v0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL8);
            VSRL_VI(v0, q0, 31, VECTOR_UNMASKED);
            if (rv64_xtheadvector) {
                // Force the element width to 4bit
                vector_vsetvli(dyn, ninst, x4, VECTOR_SEW32, VECTOR_LMUL8, 1);
                VMSNE_VX(VMASK, v0, xZR, VECTOR_UNMASKED);
                vector_vsetvli(dyn, ninst, x4, VECTOR_SEW32, VECTOR_LMUL1, 1);
                VMV_X_S(x4, VMASK);
                BEXTI(gd, x4, 12);
                BEXTI(x5, x4, 8);
                ADDSL(gd, x5, gd, 1, x6);
                BEXTI(x5, x4, 4);
                ADDSL(gd, x5, gd, 1, x6);
                BEXTI(x5, x4, 0);
                ADDSL(gd, x5, gd, 1, x6);
            } else {
                VMSNE_VX(VMASK, v0, xZR, VECTOR_UNMASKED);
                VMV_X_S(gd, VMASK);
                ZEROUP(gd);
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
        case 0x52:
            if (!box64_dynarec_fastround) return 0;
            INST_NAME("RSQRTPS Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
            GETEX_vector(v0, 0, 0, VECTOR_SEW32);
            GETGX_empty_vector(v1);
            LUI(x4, 0x3f800);
            FMVWX(v0, x4); // 1.0f
            VFSQRT_V(v1, v0, VECTOR_UNMASKED);
            VFRDIV_VF(v1, v1, v0, VECTOR_UNMASKED);
            break;
        case 0x53:
            INST_NAME("RCPPS Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
            GETEX_vector(v0, 0, 0, VECTOR_SEW32);
            GETGX_empty_vector(v1);
            LUI(x4, 0x3f800);
            FMVWX(v0, x4); // 1.0f
            VFRDIV_VF(v1, v0, v0, VECTOR_UNMASKED);
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
        case 0x5A:
            INST_NAME("CVTPS2PD Gx, Ex");
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
            VFWCVT_F_F_V(d0, v1, VECTOR_UNMASKED);
            vector_vsetvli(dyn, ninst, x1, VECTOR_SEW64, VECTOR_LMUL1, 1);
            VMV_V_V(v0, d0);
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
        case 0x6F:
            INST_NAME("MOVQ Gm, Em");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            GETG;
            if (MODREG) {
                v1 = mmx_get_reg_vector(dyn, ninst, x1, x2, x3, nextop & 7);
                v0 = mmx_get_reg_empty_vector(dyn, ninst, x1, x2, x3, gd);
                VMV_V_V(v0, v1);
            } else {
                v0 = mmx_get_reg_empty_vector(dyn, ninst, x1, x2, x3, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                LD(x4, ed, fixedaddress);
                VMV_S_X(v0, x4);
            }
            break;
        case 0x7F:
            INST_NAME("MOVQ Em, Gm");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            GETG;
            if (MODREG) {
                v1 = mmx_get_reg_vector(dyn, ninst, x1, x2, x3, gd);
                v0 = mmx_get_reg_empty_vector(dyn, ninst, x1, x2, x3, nextop & 7);
                VMV_V_V(v0, v1);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                v1 = mmx_get_reg_vector(dyn, ninst, x1, x2, x3, gd);
                VMV_X_S(x4, v1);
                SD(x4, ed, fixedaddress);
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
        case 0xD1:
        case 0xD2:
        case 0xD3:
            if (opcode == 0xD1) {
                INST_NAME("PSRLW Gm, Em");
                u8 = VECTOR_SEW16;
                i32 = 16;
            } else if (opcode == 0xD2) {
                INST_NAME("PSRLD Gm, Em");
                u8 = VECTOR_SEW32;
                i32 = 32;
            } else {
                INST_NAME("PSRLQ Gm, Em");
                u8 = VECTOR_SEW64;
                i32 = 64;
            }
            nextop = F8;
            q0 = fpu_get_scratch(dyn);
            GETGM_vector(v0);
            SET_ELEMENT_WIDTH(x1, u8, 1);
            if (MODREG) {
                v1 = mmx_get_reg_vector(dyn, ninst, x1, x2, x3, (nextop & 7));
                VMV_X_S(x4, v1);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, v1, x3, &fixedaddress, rex, NULL, 1, 0);
                LD(x4, wback, fixedaddress);
            }
            SLTIU(x3, x4, i32);
            SUB(x3, xZR, x3);
            NOT(x3, x3); // mask
            VSRL_VX(v0, v0, x4, VECTOR_UNMASKED);
            VAND_VX(q0, v0, x3, VECTOR_UNMASKED);
            VXOR_VV(v0, v0, q0, VECTOR_UNMASKED);
            break;
        case 0xD8:
        case 0xD9:
            if (opcode == 0xD8) {
                INST_NAME("PSUBUSB Gm, Em");
                u8 = VECTOR_SEW8;
            } else {
                INST_NAME("PSUBUSW Gm, Em");
                u8 = VECTOR_SEW16;
            }
            nextop = F8;
            GETGM_vector(v0);
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            GETEM_vector(v1, 0);
            SET_ELEMENT_WIDTH(x1, u8, 1);
            VSSUBU_VV(v0, v0, v1, VECTOR_UNMASKED);
            break;
        case 0xDB:
            INST_NAME("PAND Gm, Em");
            nextop = F8;
            GETGM_vector(v0);
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            GETEM_vector(v1, 0);
            VAND_VV(v0, v0, v1, VECTOR_UNMASKED);
            break;
        case 0xDC:
        case 0xDD:
            if (opcode == 0xDC) {
                INST_NAME("PADDUSB Gm, Em");
                u8 = VECTOR_SEW8;
            } else {
                INST_NAME("PADDUSW Gm, Em");
                u8 = VECTOR_SEW16;
            }
            nextop = F8;
            GETGM_vector(v0);
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            GETEM_vector(v1, 0);
            SET_ELEMENT_WIDTH(x1, u8, 1);
            VSADDU_VV(v0, v0, v1, VECTOR_UNMASKED);
            break;
        case 0xDF:
            INST_NAME("PANDN Gm, Em");
            nextop = F8;
            GETGM_vector(v0);
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            GETEM_vector(v1, 0);
            VXOR_VI(v0, v0, 0x1F, VECTOR_UNMASKED);
            VAND_VV(v0, v0, v1, VECTOR_UNMASKED);
            break;
        case 0xE8:
            INST_NAME("PSUBSB Gm, Em");
            nextop = F8;
            GETGM_vector(v0);
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            GETEM_vector(v1, 0);
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
            VSSUB_VV(v0, v0, v1, VECTOR_UNMASKED);
            break;
        case 0xE9:
            INST_NAME("PSUBSW Gm, Em");
            nextop = F8;
            GETGM_vector(v0);
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            GETEM_vector(v1, 0);
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW16, 1);
            VSSUB_VV(v0, v0, v1, VECTOR_UNMASKED);
            break;
        case 0xEB:
            INST_NAME("POR Gm, Em");
            nextop = F8;
            GETGM_vector(v0);
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            GETEM_vector(v1, 0);
            VOR_VV(v0, v0, v1, VECTOR_UNMASKED);
            break;
        case 0xEC:
            INST_NAME("PADDSB Gm, Em");
            nextop = F8;
            GETGM_vector(v0);
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            GETEM_vector(v1, 0);
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
            VSADD_VV(v0, v0, v1, VECTOR_UNMASKED);
            break;
        case 0xED:
            INST_NAME("PADDSW Gm, Em");
            nextop = F8;
            GETGM_vector(v0);
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            GETEM_vector(v1, 0);
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW16, 1);
            VSADD_VV(v0, v0, v1, VECTOR_UNMASKED);
            break;
        case 0xEF:
            INST_NAME("PXOR Gm, Em");
            nextop = F8;
            GETG;
            if (MODREG && gd == (nextop & 7) + (rex.b << 3)) {
                SET_ELEMENT_WIDTH(x1, VECTOR_SEWANY, 1);
                // special case
                q0 = mmx_get_reg_empty_vector(dyn, ninst, x1, x2, x3, gd);
                VXOR_VV(q0, q0, q0, VECTOR_UNMASKED);
            } else {
                GETGM_vector(v0);
                SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
                GETEM_vector(v1, 0);
                VXOR_VV(v0, v0, v1, VECTOR_UNMASKED);
            }
            break;
        case 0xF1:
        case 0xF2:
        case 0xF3:
            if (opcode == 0xF1) {
                INST_NAME("PSLLW Gm, Em");
                u8 = VECTOR_SEW16;
                i32 = 16;
            } else if (opcode == 0xF2) {
                INST_NAME("PSLLD Gm, Em");
                u8 = VECTOR_SEW32;
                i32 = 32;
            } else {
                INST_NAME("PSLLQ Gm, Em");
                u8 = VECTOR_SEW64;
                i32 = 64;
            }
            nextop = F8;
            q0 = fpu_get_scratch(dyn);
            GETGM_vector(v0);
            SET_ELEMENT_WIDTH(x1, u8, 1);
            if (MODREG) {
                v1 = mmx_get_reg_vector(dyn, ninst, x1, x2, x3, (nextop & 7));
                VMV_X_S(x4, v1);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, v1, x3, &fixedaddress, rex, NULL, 1, 0);
                LD(x4, wback, fixedaddress);
            }
            SLTIU(x3, x4, i32);
            SUB(x3, xZR, x3);
            NOT(x3, x3); // mask
            VSLL_VX(v0, v0, x4, VECTOR_UNMASKED);
            VAND_VX(q0, v0, x3, VECTOR_UNMASKED);
            VXOR_VV(v0, v0, q0, VECTOR_UNMASKED);
            break;
        case 0xF8 ... 0xFB:
            nextop = F8;
            if (opcode == 0xF8) {
                INST_NAME("PSUBB Gm, Em");
                u8 = VECTOR_SEW8;
            } else if (opcode == 0xF9) {
                INST_NAME("PSUBW Gm, Em");
                u8 = VECTOR_SEW16;
            } else if (opcode == 0xFA) {
                INST_NAME("PSUBD Gm, Em");
                u8 = VECTOR_SEW32;
            } else {
                INST_NAME("PSUBQ Gm, Em");
                u8 = VECTOR_SEW64;
            }
            GETGM_vector(v0);
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            GETEM_vector(v1, 0);
            SET_ELEMENT_WIDTH(x1, u8, 1);
            VSUB_VV(v0, v0, v1, VECTOR_UNMASKED);
            break;
        case 0xFC ... 0xFE:
            nextop = F8;
            if (opcode == 0xFC) {
                INST_NAME("PADDB Gm, Em");
                u8 = VECTOR_SEW8;
            } else if (opcode == 0xFD) {
                INST_NAME("PADDW Gm, Em");
                u8 = VECTOR_SEW16;
            } else {
                INST_NAME("PADDD Gm, Em");
                u8 = VECTOR_SEW32;
            }
            GETGM_vector(v0);
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            GETEM_vector(v1, 0);
            SET_ELEMENT_WIDTH(x1, u8, 1);
            VADD_VV(v0, v0, v1, VECTOR_UNMASKED);
            break;
        case 0x00 ... 0x0F:
        case 0x18:
        case 0x1F:
        case 0x31:
        case 0x40 ... 0x4F:
        case 0x77:
        case 0x7E:
        case 0x80 ... 0xBF:
        case 0xC0 ... 0xC1:
        case 0xC3 ... 0xC5:
        case 0xC7 ... 0xCF:
            return 0;
        default:
            DEFAULT_VECTOR;
    }
    return addr;
}
