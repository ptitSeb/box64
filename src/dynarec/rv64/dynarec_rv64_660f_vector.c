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

uintptr_t dynarec64_660F_vector(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;
    uint8_t opcode = F8;
    uint8_t nextop, u8, s8;
    int32_t i32;
    uint8_t gd, ed;
    uint8_t wback, wb1, wb2, gback;
    uint8_t eb1, eb2;
    int64_t j64;
    uint64_t tmp64u, tmp64u2;
    int v0, v1;
    int q0, q1;
    int d0, d1, d2;
    uint64_t tmp64u0, tmp64u1;
    int64_t fixedaddress, gdoffset;
    uint32_t vtypei;
    int unscaled;
    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(j64);
    switch (opcode) {
        case 0x10:
            INST_NAME("MOVUPD Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG) {
                SET_ELEMENT_WIDTH(x1, VECTOR_SEWANY, 1);
                v1 = sse_get_reg_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, dyn->vector_eew);
                v0 = sse_get_reg_empty_vector(dyn, ninst, x1, gd);
                VMV_V_V(v0, v1);
            } else {
                SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1); // unaligned!
                SMREAD();
                v0 = sse_get_reg_empty_vector(dyn, ninst, x1, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                VLE8_V(v0, ed, VECTOR_UNMASKED, VECTOR_NFIELD1);
            }
            break;
        case 0x11:
            INST_NAME("MOVUPD Ex, Gx");
            nextop = F8;
            GETG;
            if (MODREG) {
                SET_ELEMENT_WIDTH(x1, VECTOR_SEWANY, 1);
                v0 = sse_get_reg_vector(dyn, ninst, x1, gd, 0, dyn->vector_eew);
                v1 = sse_get_reg_empty_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3));
                VMV_V_V(v1, v0);
            } else {
                SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1); // unaligned!
                v0 = sse_get_reg_vector(dyn, ninst, x1, gd, 0, dyn->vector_eew);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                VSE8_V(v0, ed, VECTOR_UNMASKED, VECTOR_NFIELD1);
                SMWRITE2();
            }
            break;
        case 0x12:
            INST_NAME("MOVLPD Gx, Eq");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            GETGX_vector(v0, 1, VECTOR_SEW64);
            if (MODREG) {
                // access register instead of memory is bad opcode!
                DEFAULT;
                return addr;
            }
            SMREAD();
            addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
            LD(x4, ed, fixedaddress);
            if (cpuext.xtheadvector) {
                v1 = fpu_get_scratch(dyn);
                VMV_S_X(v1, x4);
                VECTOR_LOAD_VMASK(0b01, x3, 1);
                VMERGE_VVM(v0, v0, v1); // implies VMASK
            } else {
                VMV_S_X(v0, x4);
            }
            break;
        case 0x13:
            INST_NAME("MOVLPD Eq, Gx");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            GETGX_vector(v0, 0, VECTOR_SEW64);
            if (MODREG) {
                DEFAULT;
                return addr;
            }
            addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
            VMV_X_S(x4, v0);
            SD(x4, ed, fixedaddress);
            SMWRITE2();
            break;
        case 0x14:
            INST_NAME("UNPCKLPD Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            // GX->q[0] = GX->q[0]; -> unchanged
            // GX->q[1] = EX->q[0];
            GETGX_vector(v0, 1, VECTOR_SEW64);
            if (MODREG) {
                v1 = sse_get_reg_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW64);
                if (v0 == v1) {
                    // for vslideup.vi, cannot be overlapped
                    v1 = fpu_get_scratch(dyn);
                    VMV_V_V(v1, v0);
                }
                VSLIDEUP_VI(v0, v1, 1, VECTOR_UNMASKED);
            } else {
                q0 = fpu_get_scratch(dyn);
                VXOR_VV(q0, q0, q0, VECTOR_UNMASKED);
                VECTOR_LOAD_VMASK(0b10, x1, 1);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, x2, &fixedaddress, rex, NULL, 0, 0);
                VLUXEI64_V(v0, q0, ed, VECTOR_MASKED, VECTOR_NFIELD1);
            }
            break;
        case 0x15:
            INST_NAME("UNPCKHPD Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            // GX->q[0] = GX->q[1];
            // GX->q[1] = EX->q[1];
            GETGX_vector(v0, 1, VECTOR_SEW64);
            if (MODREG) {
                v1 = sse_get_reg_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW64);
                q0 = fpu_get_scratch(dyn);
                VSLIDE1DOWN_VX(q0, v0, xZR, VECTOR_UNMASKED);
                if (cpuext.xtheadvector) {
                    VECTOR_LOAD_VMASK(0b01, x4, 1);
                    VMERGE_VVM(v0, v1, q0); // implies VMASK
                } else {
                    if (v0 != v1) { VMV_V_V(v0, v1); }
                    VMV_X_S(x4, q0);
                    VMV_S_X(v0, x4);
                }
            } else {
                q0 = fpu_get_scratch(dyn);
                VECTOR_LOAD_VMASK(0b10, x1, 1);
                VSLIDE1DOWN_VX(v0, v0, xZR, VECTOR_UNMASKED);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, x2, &fixedaddress, rex, NULL, 0, 0);
                VLE64_V(v0, ed, VECTOR_MASKED, VECTOR_NFIELD1);
            }
            break;
        case 0x16:
            INST_NAME("MOVHPD Gx, Eq");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            GETGX_vector(v0, 1, VECTOR_SEW64);
            if (MODREG) {
                // access register instead of memory is bad opcode!
                DEFAULT;
                return addr;
            }
            SMREAD();
            addr = geted(dyn, addr, ninst, nextop, &ed, x3, x2, &fixedaddress, rex, NULL, 0, 0);
            LD(x4, ed, fixedaddress);
            d0 = fpu_get_scratch(dyn);
            VMV_S_X(d0, x4);
            VSLIDEUP_VI(v0, d0, 1, VECTOR_UNMASKED);
            break;
        case 0x17:
            INST_NAME("MOVHPD Eq, Gx");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            GETGX_vector(v0, 1, VECTOR_SEW64);
            if (MODREG) {
                // access register instead of memory is bad opcode!
                DEFAULT;
                return addr;
            }
            SMREAD();
            addr = geted(dyn, addr, ninst, nextop, &ed, x3, x2, &fixedaddress, rex, NULL, 0, 0);
            d0 = fpu_get_scratch(dyn);
            VSLIDE1DOWN_VX(d0, v0, xZR, VECTOR_UNMASKED);
            VMV_X_S(x5, d0);
            SD(x5, ed, fixedaddress);
            SMWRITE2();
            break;
        case 0x18:
        case 0x19:
        case 0x1F:
            return 0;
        case 0x28:
            INST_NAME("MOVAPD Gx, Ex");
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
            INST_NAME("MOVAPD Ex, Gx");
            nextop = F8;
            GETG;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEWANY, 1);
            v0 = sse_get_reg_vector(dyn, ninst, x1, gd, 0, dyn->vector_eew);
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
            INST_NAME("MOVNTPD Ex, Gx");
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
        case 0x2E:
        case 0x2F:
            return 0;
        case 0x38: // SSSE3 opcodes
            nextop = F8;
            switch (nextop) {
                case 0x00:
                    INST_NAME("PSHUFB Gx, Ex");
                    nextop = F8;
                    SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
                    GETGX_vector(q0, 1, VECTOR_SEW8);
                    GETEX_vector(q1, 0, 0, VECTOR_SEW8);
                    v0 = fpu_get_scratch(dyn);
                    v1 = fpu_get_scratch(dyn);
                    ADDI(x4, xZR, 0b000010001111);
                    VMV_V_X(v0, x4); // broadcast the mask
                    VAND_VV(v0, q1, v0, VECTOR_UNMASKED);
                    VRGATHER_VV(v1, q0, v0, VECTOR_UNMASKED); // registers cannot be overlapped!!
                    VMV_V_V(q0, v1);
                    break;
                case 0x01:
                    INST_NAME("PHADDW Gx, Ex");
                    nextop = F8;
                    SET_ELEMENT_WIDTH(x1, VECTOR_SEW16, 1);
                    GETGX_vector(q0, 1, VECTOR_SEW16);
                    GETEX_vector(q1, 0, 0, VECTOR_SEW16);
                    v0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                    d1 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                    d0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                    VMV_V_V(v0, q0);
                    if (q1 & 1) VMV_V_V(d1, q1);
                    vector_vsetvli(dyn, ninst, x1, VECTOR_SEW16, VECTOR_LMUL2, 2);
                    VSLIDEUP_VI(v0, (q1 & 1) ? d1 : q1, 8, VECTOR_UNMASKED);
                    vector_vsetvli(dyn, ninst, x1, VECTOR_SEW16, VECTOR_LMUL1, 1);
                    VNSRL_WI(d0, v0, 0, VECTOR_UNMASKED);
                    VNSRL_WI(d1, v0, 16, VECTOR_UNMASKED);
                    VADD_VV(q0, d1, d0, VECTOR_UNMASKED);
                    break;
                case 0x02:
                    INST_NAME("PHADDD Gx, Ex");
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
                    VADD_VV(q0, d1, d0, VECTOR_UNMASKED);
                    break;
                case 0x03:
                    INST_NAME("PHADDSW Gx, Ex");
                    nextop = F8;
                    SET_ELEMENT_WIDTH(x1, VECTOR_SEW16, 1);
                    GETGX_vector(q0, 1, VECTOR_SEW16);
                    GETEX_vector(q1, 0, 0, VECTOR_SEW16);
                    v0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                    d1 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                    d0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                    VMV_V_V(v0, q0);
                    if (q1 & 1) VMV_V_V(d1, q1);
                    vector_vsetvli(dyn, ninst, x1, VECTOR_SEW16, VECTOR_LMUL2, 2);
                    VSLIDEUP_VI(v0, (q1 & 1) ? d1 : q1, 8, VECTOR_UNMASKED);
                    vector_vsetvli(dyn, ninst, x1, VECTOR_SEW16, VECTOR_LMUL1, 1);
                    VNSRL_WI(d0, v0, 0, VECTOR_UNMASKED);
                    VNSRL_WI(d1, v0, 16, VECTOR_UNMASKED);
                    VSADD_VV(q0, d1, d0, VECTOR_UNMASKED);
                    break;
                case 0x04:
                    INST_NAME("PMADDUBSW Gx, Ex");
                    nextop = F8;
                    SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
                    GETGX_vector(q0, 1, VECTOR_SEW8);
                    GETEX_vector(q1, 0, 0, VECTOR_SEW8);
                    v0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                    d0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                    d1 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2); // no more scratches!
                    VWMULSU_VV(v0, q1, q0, VECTOR_UNMASKED);
                    SET_ELEMENT_WIDTH(x1, VECTOR_SEW16, 1);
                    VNSRL_WI(d0, v0, 0, VECTOR_UNMASKED);
                    VNSRL_WI(d1, v0, 16, VECTOR_UNMASKED);
                    VSADD_VV(q0, d1, d0, VECTOR_UNMASKED);
                    break;
                case 0x05:
                    INST_NAME("PHSUBW Gx, Ex");
                    nextop = F8;
                    SET_ELEMENT_WIDTH(x1, VECTOR_SEW16, 1);
                    GETGX_vector(q0, 1, VECTOR_SEW16);
                    GETEX_vector(q1, 0, 0, VECTOR_SEW16);
                    v0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                    d1 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                    d0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                    VMV_V_V(v0, q0);
                    if (q1 & 1) VMV_V_V(d1, q1);
                    vector_vsetvli(dyn, ninst, x1, VECTOR_SEW16, VECTOR_LMUL2, 2);
                    VSLIDEUP_VI(v0, (q1 & 1) ? d1 : q1, 8, VECTOR_UNMASKED);
                    vector_vsetvli(dyn, ninst, x1, VECTOR_SEW16, VECTOR_LMUL1, 1);
                    VNSRL_WI(d0, v0, 0, VECTOR_UNMASKED);
                    VNSRL_WI(d1, v0, 16, VECTOR_UNMASKED);
                    VSUB_VV(q0, d0, d1, VECTOR_UNMASKED);
                    break;
                case 0x08 ... 0x0A:
                    if (nextop == 0x08) {
                        INST_NAME("PSIGNB Gx, Ex");
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
                        i32 = 7;
                        nextop = F8;
                        GETGX_vector(q0, 1, VECTOR_SEW8);
                        GETEX_vector(q1, 0, 0, VECTOR_SEW8);
                    } else if (nextop == 0x09) {
                        INST_NAME("PSIGNW Gx, Ex");
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW16, 1);
                        i32 = 15;
                        nextop = F8;
                        GETGX_vector(q0, 1, VECTOR_SEW16);
                        GETEX_vector(q1, 0, 0, VECTOR_SEW16);
                    } else {
                        INST_NAME("PSIGND Gx, Ex");
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
                        i32 = 31;
                        nextop = F8;
                        GETGX_vector(q0, 1, VECTOR_SEW32);
                        GETEX_vector(q1, 0, 0, VECTOR_SEW32);
                    }
                    v0 = fpu_get_scratch(dyn);
                    v1 = fpu_get_scratch(dyn);
                    VMSLT_VX(VMASK, q1, xZR, VECTOR_UNMASKED);
                    VRSUB_VX(q0, q0, xZR, VECTOR_MASKED);
                    VMSEQ_VX(VMASK, q1, xZR, VECTOR_UNMASKED);
                    VXOR_VV(q0, q0, q0, VECTOR_MASKED);
                    break;
                case 0x0B:
                    INST_NAME("PMULHRSW Gx, Ex");
                    nextop = F8;
                    SET_ELEMENT_WIDTH(x1, VECTOR_SEW16, 1);
                    GETGX_vector(q0, 1, VECTOR_SEW16);
                    GETEX_vector(q1, 0, 0, VECTOR_SEW16);
                    v0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                    fpu_get_scratch(dyn);
                    VWMUL_VV(v0, q1, q0, VECTOR_UNMASKED);
                    vector_vsetvli(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL2, 2);
                    VSRL_VI(v0, v0, 14, VECTOR_UNMASKED);
                    VADD_VI(v0, v0, 1, VECTOR_UNMASKED);
                    vector_vsetvli(dyn, ninst, x1, VECTOR_SEW16, VECTOR_LMUL1, 1);
                    VNSRL_WI(q0, v0, 1, VECTOR_UNMASKED);
                    break;
                case 0x10:
                    INST_NAME("PBLENDVB Gx, Ex");
                    nextop = F8;
                    SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
                    GETGX_vector(q0, 1, VECTOR_SEW8);
                    GETEX_vector(q1, 0, 0, VECTOR_SEW8);
                    v0 = sse_get_reg_vector(dyn, ninst, x4, 0, 0, VECTOR_SEW8);
                    VMSLT_VX(VMASK, v0, xZR, VECTOR_UNMASKED);
                    VADD_VX(q0, q1, xZR, VECTOR_MASKED);
                    break;
                case 0x14:
                    INST_NAME("PBLENDVPS Gx, Ex");
                    nextop = F8;
                    SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
                    GETGX_vector(q0, 1, VECTOR_SEW32);
                    GETEX_vector(q1, 0, 0, VECTOR_SEW32);
                    v0 = sse_get_reg_vector(dyn, ninst, x4, 0, 0, VECTOR_SEW32);
                    VMSLT_VX(VMASK, v0, xZR, VECTOR_UNMASKED);
                    VADD_VX(q0, q1, xZR, VECTOR_MASKED);
                    break;
                case 0x15:
                    INST_NAME("PBLENDVPD Gx, Ex");
                    nextop = F8;
                    SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
                    GETGX_vector(q0, 1, VECTOR_SEW64);
                    GETEX_vector(q1, 0, 0, VECTOR_SEW64);
                    v0 = sse_get_reg_vector(dyn, ninst, x4, 0, 0, VECTOR_SEW64);
                    VMSLT_VX(VMASK, v0, xZR, VECTOR_UNMASKED);
                    VADD_VX(q0, q1, xZR, VECTOR_MASKED);
                    break;
                case 0x17:
                    INST_NAME("PTEST Gx, Ex");
                    nextop = F8;
                    SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
                    SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
                    GETGX_vector(q0, 0, VECTOR_SEW64);
                    GETEX_vector(q1, 0, 0, VECTOR_SEW64);
                    CLEAR_FLAGS();
                    SET_DFNONE();
                    v0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                    IFX (X_ZF) {
                        VAND_VV(v0, q1, q0, VECTOR_UNMASKED);
                        if (cpuext.xtheadvector) {
                            // Force the mask element width to 32
                            vector_vsetvli(dyn, ninst, x1, VECTOR_SEW64, VECTOR_LMUL2, 1);
                        }
                        VMSGTU_VX(VMASK, v0, xZR, VECTOR_UNMASKED);
                        if (cpuext.xtheadvector) {
                            vector_vsetvli(dyn, ninst, x1, VECTOR_SEW64, VECTOR_LMUL1, 1);
                        }
                        VMV_X_S(x4, VMASK);
                        if (!cpuext.xtheadvector) ANDI(x4, x4, 0b11);
                        BNEZ(x4, 8);
                        ORI(xFlags, xFlags, 1 << F_ZF);
                    }
                    IFX (X_CF) {
                        VXOR_VI(v0, q0, 0x1F, VECTOR_UNMASKED);
                        VAND_VV(v0, q1, v0, VECTOR_UNMASKED);
                        if (cpuext.xtheadvector) {
                            // Force the mask element width to 32
                            vector_vsetvli(dyn, ninst, x1, VECTOR_SEW64, VECTOR_LMUL2, 1);
                        }
                        VMSGTU_VX(VMASK, v0, xZR, VECTOR_UNMASKED);
                        if (cpuext.xtheadvector) {
                            vector_vsetvli(dyn, ninst, x1, VECTOR_SEW64, VECTOR_LMUL1, 1);
                        }
                        VMV_X_S(x4, VMASK);
                        if (!cpuext.xtheadvector) ANDI(x4, x4, 0b11);
                        BNEZ(x4, 8);
                        ORI(xFlags, xFlags, 1 << F_CF);
                    }
                    break;
                case 0x1C ... 0x1E:
                    if (nextop == 0x1C) {
                        INST_NAME("PABSB Gx, Ex");
                        u8 = VECTOR_SEW8;
                    } else if (nextop == 0x1D) {
                        INST_NAME("PABSW Gx, Ex");
                        u8 = VECTOR_SEW16;
                    } else {
                        INST_NAME("PABSD Gx, Ex");
                        u8 = VECTOR_SEW32;
                    }
                    nextop = F8;
                    SET_ELEMENT_WIDTH(x1, u8, 1);
                    GETEX_vector(q1, 0, 0, u8);
                    GETGX_empty_vector(q0);
                    v0 = fpu_get_scratch(dyn);
                    VSRA_VI(v0, q1, 0x1F, VECTOR_UNMASKED);
                    VXOR_VV(q0, v0, q1, VECTOR_UNMASKED);
                    VSUB_VV(q0, q0, v0, VECTOR_UNMASKED);
                    break;
                case 0x20:
                    INST_NAME("PMOVSXBW Gx, Ex");
                    nextop = F8;
                    if (cpuext.xtheadvector) {
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
                        GETEX_vector(q1, 0, 0, VECTOR_SEW8);
                        GETGX_empty_vector(q0);
                        v0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                        vector_vsetvli(dyn, ninst, x1, VECTOR_SEW8, VECTOR_LMUL1, 0.5);
                        VWADD_VX(v0, q1, xZR, VECTOR_UNMASKED);
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW16, 1);
                        VMV_V_V(q0, v0);
                    } else {
                        if (!MODREG) SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
                        GETEX_vector(q1, 0, 0, VECTOR_SEW8);
                        GETGX_empty_vector(q0);
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW16, 1);
                        if (q0 == q1) {
                            q1 = fpu_get_scratch(dyn);
                            VMV_V_V(q1, q0);
                        }
                        VSEXT_VF2(q0, q1, VECTOR_UNMASKED);
                    }
                    break;
                case 0x21:
                    INST_NAME("PMOVSXBD Gx, Ex");
                    nextop = F8;
                    if (cpuext.xtheadvector) {
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
                        GETEX_vector(q1, 0, 0, VECTOR_SEW8);
                        GETGX_empty_vector(q0);
                        v0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                        fpu_get_scratch(dyn);
                        v1 = fpu_get_scratch(dyn);
                        vector_vsetvli(dyn, ninst, x1, VECTOR_SEW8, VECTOR_LMUL1, 0.25);
                        VWADD_VX(v0, q1, xZR, VECTOR_UNMASKED);
                        vector_vsetvli(dyn, ninst, x1, VECTOR_SEW16, VECTOR_LMUL1, 0.5);
                        VWADD_VX(v1, v0, xZR, VECTOR_UNMASKED);
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
                        VMV_V_V(q0, v1);
                    } else {
                        if (!MODREG) SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
                        GETEX_vector(q1, 0, 0, VECTOR_SEW8);
                        GETGX_empty_vector(q0);
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
                        if (q0 == q1) {
                            q1 = fpu_get_scratch(dyn);
                            VMV_V_V(q1, q0);
                        }
                        VSEXT_VF4(q0, q1, VECTOR_UNMASKED);
                    }
                    break;
                    break;
                case 0x22:
                    INST_NAME("PMOVSXBQ Gx, Ex");
                    nextop = F8;
                    if (cpuext.xtheadvector) {
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
                        GETEX_vector(q1, 0, 0, VECTOR_SEW8);
                        GETGX_empty_vector(q0);
                        v0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                        fpu_get_scratch(dyn);
                        v1 = fpu_get_scratch(dyn);
                        vector_vsetvli(dyn, ninst, x1, VECTOR_SEW8, VECTOR_LMUL1, 0.125);
                        VWADD_VX(v0, q1, xZR, VECTOR_UNMASKED);
                        vector_vsetvli(dyn, ninst, x1, VECTOR_SEW16, VECTOR_LMUL1, 0.25);
                        VWADD_VX(v1, v0, xZR, VECTOR_UNMASKED);
                        vector_vsetvli(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL1, 0.5);
                        VWADD_VX(v0, v1, xZR, VECTOR_UNMASKED);
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
                        VMV_V_V(q0, v0);
                    } else {
                        if (!MODREG) SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
                        GETEX_vector(q1, 0, 0, VECTOR_SEW8);
                        GETGX_empty_vector(q0);
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
                        if (q0 == q1) {
                            q1 = fpu_get_scratch(dyn);
                            VMV_V_V(q1, q0);
                        }
                        VSEXT_VF8(q0, q1, VECTOR_UNMASKED);
                    }
                    break;
                case 0x23:
                    INST_NAME("PMOVSXWD Gx, Ex");
                    nextop = F8;
                    if (cpuext.xtheadvector) {
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
                        GETEX_vector(q1, 0, 0, VECTOR_SEW8);
                        GETGX_empty_vector(q0);
                        v0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                        vector_vsetvli(dyn, ninst, x1, VECTOR_SEW16, VECTOR_LMUL1, 0.5);
                        VWADD_VX(v0, q1, xZR, VECTOR_UNMASKED);
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
                        VMV_V_V(q0, v0);
                    } else {
                        if (!MODREG) SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
                        GETEX_vector(q1, 0, 0, VECTOR_SEW8);
                        GETGX_empty_vector(q0);
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
                        if (q0 == q1) {
                            q1 = fpu_get_scratch(dyn);
                            VMV_V_V(q1, q0);
                        }
                        VSEXT_VF2(q0, q1, VECTOR_UNMASKED);
                    }
                    break;
                case 0x24:
                    INST_NAME("PMOVSXWQ Gx, Ex");
                    nextop = F8;
                    if (cpuext.xtheadvector) {
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
                        GETEX_vector(q1, 0, 0, VECTOR_SEW8);
                        GETGX_empty_vector(q0);
                        v0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                        fpu_get_scratch(dyn);
                        v1 = fpu_get_scratch(dyn);
                        vector_vsetvli(dyn, ninst, x1, VECTOR_SEW16, VECTOR_LMUL1, 0.25);
                        VWADD_VX(v0, q1, xZR, VECTOR_UNMASKED);
                        vector_vsetvli(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL1, 0.5);
                        VWADD_VX(v1, v0, xZR, VECTOR_UNMASKED);
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
                        VMV_V_V(q0, v1);
                    } else {
                        if (!MODREG) SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
                        GETEX_vector(q1, 0, 0, VECTOR_SEW8);
                        GETGX_empty_vector(q0);
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
                        if (q0 == q1) {
                            q1 = fpu_get_scratch(dyn);
                            VMV_V_V(q1, q0);
                        }
                        VSEXT_VF4(q0, q1, VECTOR_UNMASKED);
                    }
                    break;
                case 0x25:
                    INST_NAME("PMOVSXDQ Gx, Ex");
                    nextop = F8;
                    if (cpuext.xtheadvector) {
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
                        GETEX_vector(q1, 0, 0, VECTOR_SEW8);
                        GETGX_empty_vector(q0);
                        v0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                        vector_vsetvli(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL1, 0.5);
                        VWADD_VX(v0, q1, xZR, VECTOR_UNMASKED);
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
                        VMV_V_V(q0, v0);
                    } else {
                        if (!MODREG) SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
                        GETEX_vector(q1, 0, 0, VECTOR_SEW8);
                        GETGX_empty_vector(q0);
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
                        if (q0 == q1) {
                            q1 = fpu_get_scratch(dyn);
                            VMV_V_V(q1, q0);
                        }
                        VSEXT_VF2(q0, q1, VECTOR_UNMASKED);
                    }
                    break;
                case 0x28:
                    INST_NAME("PMULDQ Gx, Ex");
                    nextop = F8;
                    if (cpuext.vlen >= 32) {
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
                        GETGX_vector(v0, 1, VECTOR_SEW64);
                        GETEX_vector(v1, 0, 0, VECTOR_SEW64);
                        d1 = fpu_get_scratch(dyn);
                        ADDI(x2, xZR, 32);
                        VSLL_VX(v0, v0, x2, VECTOR_UNMASKED);
                        VSRA_VX(v0, v0, x2, VECTOR_UNMASKED);
                        VSLL_VX(d1, v1, x2, VECTOR_UNMASKED);
                        VSRA_VX(d1, d1, x2, VECTOR_UNMASKED);
                        VMUL_VV(v0, v0, d1, VECTOR_UNMASKED);
                    } else {
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
                        GETGX_vector(v0, 1, VECTOR_SEW32);
                        GETEX_vector(v1, 0, 0, VECTOR_SEW32);
                        d0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                        d1 = fpu_get_scratch(dyn);
                        VWMUL_VV(d0, v0, v1, VECTOR_UNMASKED);
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
                        VSLIDEUP_VI(d0, d1, 1, VECTOR_UNMASKED);
                        VMV_V_V(v0, d0);
                    }
                    break;
                case 0x29:
                    INST_NAME("PCMPEQQ Gx, Ex"); // SSE4 opcode!
                    nextop = F8;
                    SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
                    GETEX_vector(q1, 0, 0, VECTOR_SEW64);
                    GETGX_vector(q0, 1, VECTOR_SEW64);
                    VMSEQ_VV(VMASK, q0, q1, VECTOR_UNMASKED);
                    VXOR_VV(q0, q0, q0, VECTOR_UNMASKED);
                    VMERGE_VIM(q0, q0, 0b11111);
                    break;
                case 0x2B:
                    INST_NAME("PACKUSDW Gx, Ex");
                    nextop = F8;
                    SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
                    GETEX_vector(q1, 0, 0, VECTOR_SEW32);
                    GETGX_vector(q0, 1, VECTOR_SEW32);
                    d0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                    VXOR_VV(d0, d0, d0, VECTOR_UNMASKED);
                    d1 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                    v0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                    VMAX_VX(d0, q0, xZR, VECTOR_UNMASKED);
                    if (q0 != q1) VMAX_VX(d1, q1, xZR, VECTOR_UNMASKED);
                    vector_vsetvli(dyn, ninst, x1, VECTOR_SEW16, VECTOR_LMUL1, 0.5);
                    VNCLIPU_WX(q0, d0, xZR, VECTOR_UNMASKED);
                    if (q0 != q1) VNCLIPU_WX(v0, d1, xZR, VECTOR_UNMASKED);
                    SET_ELEMENT_WIDTH(x1, VECTOR_SEW16, 1);
                    if (q0 == q1) VMV_V_V(v0, q0);
                    VSLIDEUP_VI(q0, v0, 4, VECTOR_UNMASKED);
                    break;
                case 0x30:
                    INST_NAME("PMOVZXBW Gx, Ex");
                    nextop = F8;
                    if (cpuext.xtheadvector) {
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
                        GETEX_vector(q1, 0, 0, VECTOR_SEW8);
                        GETGX_empty_vector(q0);
                        v0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                        vector_vsetvli(dyn, ninst, x1, VECTOR_SEW8, VECTOR_LMUL1, 0.5);
                        VWADDU_VX(v0, q1, xZR, VECTOR_UNMASKED);
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW16, 1);
                        VMV_V_V(q0, v0);
                    } else {
                        if (!MODREG) SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
                        GETEX_vector(q1, 0, 0, VECTOR_SEW8);
                        GETGX_empty_vector(q0);
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW16, 1);
                        if (q0 == q1) {
                            q1 = fpu_get_scratch(dyn);
                            VMV_V_V(q1, q0);
                        }
                        VZEXT_VF2(q0, q1, VECTOR_UNMASKED);
                    }
                    break;
                case 0x31:
                    INST_NAME("PMOVZXBD Gx, Ex");
                    nextop = F8;
                    if (cpuext.xtheadvector) {
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
                        GETEX_vector(q1, 0, 0, VECTOR_SEW8);
                        GETGX_empty_vector(q0);
                        v0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                        fpu_get_scratch(dyn);
                        v1 = fpu_get_scratch(dyn);
                        vector_vsetvli(dyn, ninst, x1, VECTOR_SEW8, VECTOR_LMUL1, 0.25);
                        VWADDU_VX(v0, q1, xZR, VECTOR_UNMASKED);
                        vector_vsetvli(dyn, ninst, x1, VECTOR_SEW16, VECTOR_LMUL1, 0.5);
                        VWADDU_VX(v1, v0, xZR, VECTOR_UNMASKED);
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
                        VMV_V_V(q0, v1);
                    } else {
                        if (!MODREG) SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
                        GETEX_vector(q1, 0, 0, VECTOR_SEW8);
                        GETGX_empty_vector(q0);
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
                        if (q0 == q1) {
                            q1 = fpu_get_scratch(dyn);
                            VMV_V_V(q1, q0);
                        }
                        VZEXT_VF4(q0, q1, VECTOR_UNMASKED);
                    }
                    break;
                case 0x32:
                    INST_NAME("PMOVZXBQ Gx, Ex");
                    nextop = F8;
                    if (cpuext.xtheadvector) {
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
                        GETEX_vector(q1, 0, 0, VECTOR_SEW8);
                        GETGX_empty_vector(q0);
                        v0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                        fpu_get_scratch(dyn);
                        v1 = fpu_get_scratch(dyn);
                        vector_vsetvli(dyn, ninst, x1, VECTOR_SEW8, VECTOR_LMUL1, 0.125);
                        VWADDU_VX(v0, q1, xZR, VECTOR_UNMASKED);
                        vector_vsetvli(dyn, ninst, x1, VECTOR_SEW16, VECTOR_LMUL1, 0.25);
                        VWADDU_VX(v1, v0, xZR, VECTOR_UNMASKED);
                        vector_vsetvli(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL1, 0.5);
                        VWADDU_VX(v0, v1, xZR, VECTOR_UNMASKED);
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
                        VMV_V_V(q0, v0);
                    } else {
                        if (!MODREG) SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
                        GETEX_vector(q1, 0, 0, VECTOR_SEW8);
                        GETGX_empty_vector(q0);
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
                        if (q0 == q1) {
                            q1 = fpu_get_scratch(dyn);
                            VMV_V_V(q1, q0);
                        }
                        VZEXT_VF8(q0, q1, VECTOR_UNMASKED);
                    }
                    break;
                case 0x33:
                    INST_NAME("PMOVZXWD Gx, Ex");
                    nextop = F8;
                    if (cpuext.xtheadvector) {
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
                        GETEX_vector(q1, 0, 0, VECTOR_SEW8);
                        GETGX_empty_vector(q0);
                        v0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                        vector_vsetvli(dyn, ninst, x1, VECTOR_SEW16, VECTOR_LMUL1, 0.5);
                        VWADDU_VX(v0, q1, xZR, VECTOR_UNMASKED);
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
                        VMV_V_V(q0, v0);
                    } else {
                        if (!MODREG) SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
                        GETEX_vector(q1, 0, 0, VECTOR_SEW8);
                        GETGX_empty_vector(q0);
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
                        if (q0 == q1) {
                            q1 = fpu_get_scratch(dyn);
                            VMV_V_V(q1, q0);
                        }
                        VZEXT_VF2(q0, q1, VECTOR_UNMASKED);
                    }
                    break;
                case 0x34:
                    INST_NAME("PMOVZXWQ Gx, Ex");
                    nextop = F8;
                    if (cpuext.xtheadvector) {
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
                        GETEX_vector(q1, 0, 0, VECTOR_SEW8);
                        GETGX_empty_vector(q0);
                        v0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                        fpu_get_scratch(dyn);
                        v1 = fpu_get_scratch(dyn);
                        vector_vsetvli(dyn, ninst, x1, VECTOR_SEW16, VECTOR_LMUL1, 0.25);
                        VWADDU_VX(v0, q1, xZR, VECTOR_UNMASKED);
                        vector_vsetvli(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL1, 0.5);
                        VWADDU_VX(v1, v0, xZR, VECTOR_UNMASKED);
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
                        VMV_V_V(q0, v1);
                    } else {
                        if (!MODREG) SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
                        GETEX_vector(q1, 0, 0, VECTOR_SEW8);
                        GETGX_empty_vector(q0);
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
                        if (q0 == q1) {
                            q1 = fpu_get_scratch(dyn);
                            VMV_V_V(q1, q0);
                        }
                        VZEXT_VF4(q0, q1, VECTOR_UNMASKED);
                    }
                    break;
                case 0x35:
                    INST_NAME("PMOVZXDQ Gx, Ex");
                    nextop = F8;
                    if (cpuext.xtheadvector) {
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
                        GETEX_vector(q1, 0, 0, VECTOR_SEW8);
                        GETGX_empty_vector(q0);
                        v0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                        vector_vsetvli(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL1, 0.5);
                        VWADDU_VX(v0, q1, xZR, VECTOR_UNMASKED);
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
                        VMV_V_V(q0, v0);
                    } else {
                        if (!MODREG) SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
                        GETEX_vector(q1, 0, 0, VECTOR_SEW8);
                        GETGX_empty_vector(q0);
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
                        if (q0 == q1) {
                            q1 = fpu_get_scratch(dyn);
                            VMV_V_V(q1, q0);
                        }
                        VZEXT_VF2(q0, q1, VECTOR_UNMASKED);
                    }
                    break;
                case 0x37:
                    INST_NAME("PCMPGTQ Gx, Ex");
                    nextop = F8;
                    SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
                    GETGX_vector(q0, 1, dyn->vector_eew);
                    GETEX_vector(q1, 0, 0, dyn->vector_eew);
                    VMSLT_VV(VMASK, q1, q0, VECTOR_UNMASKED);
                    VXOR_VV(q0, q0, q0, VECTOR_UNMASKED);
                    VMERGE_VIM(q0, q0, 0b11111); // implies vmask and widened it
                    break;
                case 0x39:
                    INST_NAME("PMINSD Gx, Ex");
                    nextop = F8;
                    SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
                    GETGX_vector(q0, 1, VECTOR_SEW32);
                    GETEX_vector(q1, 0, 0, VECTOR_SEW32);
                    VMIN_VV(q0, q1, q0, VECTOR_UNMASKED);
                    break;
                case 0x3A:
                    INST_NAME("PMINUW Gx, Ex");
                    nextop = F8;
                    SET_ELEMENT_WIDTH(x1, VECTOR_SEW16, 1);
                    GETEX_vector(q1, 0, 0, VECTOR_SEW16);
                    GETGX_vector(q0, 1, VECTOR_SEW16);
                    VMINU_VV(q0, q1, q0, VECTOR_UNMASKED);
                    break;
                case 0x3B:
                    INST_NAME("PMINUD Gx, Ex");
                    nextop = F8;
                    SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
                    GETEX_vector(q1, 0, 0, VECTOR_SEW32);
                    GETGX_vector(q0, 1, VECTOR_SEW32);
                    VMINU_VV(q0, q1, q0, VECTOR_UNMASKED);
                    break;
                case 0x3D:
                    INST_NAME("PMAXSD Gx, Ex");
                    nextop = F8;
                    SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
                    GETGX_vector(q0, 1, VECTOR_SEW32);
                    GETEX_vector(q1, 0, 0, VECTOR_SEW32);
                    VMAX_VV(q0, q1, q0, VECTOR_UNMASKED);
                    break;
                case 0x40:
                    INST_NAME("PMULLD Gx, Ex");
                    nextop = F8;
                    SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
                    GETEX_vector(q1, 0, 0, VECTOR_SEW32);
                    GETGX_vector(q0, 1, VECTOR_SEW32);
                    VMUL_VV(q0, q1, q0, VECTOR_UNMASKED);
                    break;
                case 0xDB ... 0xDF:
                    return 0;
                default:
                    DEFAULT_VECTOR;
            }
            break;
        case 0x3A: // these are some more SSSE3+ opcodes
            opcode = F8;
            switch (opcode) {
                case 0x0E:
                    INST_NAME("PBLENDW Gx, Ex, Ib");
                    nextop = F8;
                    SET_ELEMENT_WIDTH(x1, VECTOR_SEW16, 1);
                    GETGX_vector(q0, 1, VECTOR_SEW16);
                    GETEX_vector(q1, 0, 1, VECTOR_SEW16);
                    u8 = F8;
                    VECTOR_LOAD_VMASK(u8, x4, 1);
                    VADD_VI(q0, q1, 0, VECTOR_MASKED);
                    break;
                case 0x0F:
                    INST_NAME("PALIGNR Gx, Ex, Ib");
                    nextop = F8;
                    SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
                    GETGX_vector(q0, 1, VECTOR_SEW8);
                    GETEX_vector(q1, 0, 1, VECTOR_SEW8);
                    u8 = F8;
                    if (u8 > 31) {
                        VXOR_VV(q0, q0, q0, VECTOR_UNMASKED);
                    } else if (u8 > 16) {
                        d0 = fpu_get_scratch(dyn);
                        if (cpuext.vlen >= 32) {
                            // clear high bits before slidedown!
                            vector_vsetvli(dyn, ninst, x1, VECTOR_SEW8, VECTOR_LMUL1, 2);
                            VXOR_VV(d0, d0, d0, VECTOR_UNMASKED);
                            vector_vsetvli(dyn, ninst, x1, VECTOR_SEW8, VECTOR_LMUL1, 1);
                        }
                        VMV_V_V(d0, q0);
                        VSLIDEDOWN_VI(q0, d0, u8 - 16, VECTOR_UNMASKED);
                    } else if (u8 == 16) {
                        // nop
                    } else if (u8 > 0) {
                        v0 = fpu_get_scratch(dyn);
                        v1 = fpu_get_scratch(dyn);
                        VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
                        VSLIDEUP_VI(v0, q0, 16 - u8, VECTOR_UNMASKED);
                        if (cpuext.vlen >= 32) {
                            // clear high bits before slidedown!
                            d0 = fpu_get_scratch(dyn);
                            vector_vsetvli(dyn, ninst, x1, VECTOR_SEW8, VECTOR_LMUL1, 2);
                            VXOR_VV(d0, d0, d0, VECTOR_UNMASKED);
                            vector_vsetvli(dyn, ninst, x1, VECTOR_SEW8, VECTOR_LMUL1, 1);
                            VMV_V_V(d0, q1);
                            q1 = d0;
                        }
                        VSLIDEDOWN_VI(v1, q1, u8, VECTOR_UNMASKED);
                        VOR_VV(q0, v1, v0, VECTOR_UNMASKED);
                    } else {
                        if (q0 != q1) VMV_V_V(q0, q1);
                    }
                    break;
                case 0x16:
                    nextop = F8;
                    if (rex.w) {
                        INST_NAME("PEXTRQ Ed, Gx, Ib");
                    } else {
                        INST_NAME("PEXTRD Ed, Gx, Ib");
                    }
                    SET_ELEMENT_WIDTH(x1, (rex.w ? VECTOR_SEW64 : VECTOR_SEW32), 1);
                    GETGX_vector(q0, 1, dyn->vector_eew);
                    if (MODREG) {
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        u8 = F8;
                        if (u8 & (rex.w ? 1 : 3)) {
                            if (cpuext.xtheadvector) {
                                ADDI(x4, xZR, (u8 & (rex.w ? 1 : 3)));
                                VEXT_X_V(ed, q0, x4);
                            } else {
                                d0 = fpu_get_scratch(dyn);
                                VSLIDEDOWN_VI(d0, q0, (u8 & (rex.w ? 1 : 3)), VECTOR_UNMASKED);
                                VMV_X_S(ed, d0);
                                if (!rex.w) ZEROUP(ed);
                            }
                        } else {
                            VMV_X_S(ed, q0);
                            if (!cpuext.xtheadvector && !rex.w) ZEROUP(ed);
                        }
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 1);
                        u8 = F8;
                        if (u8 & (rex.w ? 1 : 3)) {
                            if (cpuext.xtheadvector) {
                                ADDI(x4, xZR, (u8 & (rex.w ? 1 : 3)));
                                VEXT_X_V(x5, q0, x4);
                            } else {
                                d0 = fpu_get_scratch(dyn);
                                VSLIDEDOWN_VI(d0, q0, (u8 & (rex.w ? 1 : 3)), VECTOR_UNMASKED);
                                VMV_X_S(x5, d0);
                            }
                        } else {
                            VMV_X_S(x5, q0);
                        }
                        SDxw(x5, ed, fixedaddress);
                        SMWRITE2();
                    }
                    break;
                case 0x22:
                    INST_NAME("PINSRD Gx, Ed, Ib");
                    nextop = F8;
                    SET_ELEMENT_WIDTH(x1, rex.w ? VECTOR_SEW64 : VECTOR_SEW32, 1);
                    GETGX_vector(q0, 1, dyn->vector_eew);
                    if (MODREG) {
                        u8 = (F8) & (rex.w ? 1 : 3);
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 1);
                        u8 = (F8) & (rex.w ? 1 : 3);
                        LDxw(x4, ed, fixedaddress);
                        ed = x4;
                    }
                    VECTOR_LOAD_VMASK((1 << u8), x5, 1);
                    v0 = fpu_get_scratch(dyn);
                    VMERGE_VXM(v0, q0, ed); // uses VMASK
                    VMV_V_V(q0, v0);
                    break;
                case 0x40:
                    INST_NAME("DPPS Gx, Ex, Ib");
                    nextop = F8;
                    SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
                    GETGX_vector(q0, 1, VECTOR_SEW32);
                    GETEX_vector(q1, 0, 1, VECTOR_SEW32);
                    u8 = F8;
                    v0 = fpu_get_scratch(dyn);
                    v1 = fpu_get_scratch(dyn);
                    d0 = fpu_get_scratch(dyn);
                    VXOR_VV(v1, v1, v1, VECTOR_UNMASKED);
                    VECTOR_LOAD_VMASK((u8 >> 4), x4, 1);
                    VFMUL_VV(v0, q0, q1, VECTOR_MASKED);
                    VFREDUSUM_VS(d0, v0, v1, VECTOR_MASKED);
                    VMV_X_S(x4, d0);
                    VMV_V_X(d0, x4);
                    VECTOR_LOAD_VMASK((u8 & 0xf), x4, 1);
                    VMERGE_VVM(q0, v1, d0);
                    break;
                case 0x44:
                case 0x60 ... 0x63:
                case 0xDF: return 0;
                default: DEFAULT_VECTOR;
            }
            break;
        case 0x40 ... 0x4F: return 0;
        case 0x50:
            INST_NAME("PMOVMSKD Gd, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            GETGD;
            GETEX_vector(q0, 0, 0, VECTOR_SEW64);
            if (cpuext.xtheadvector) {
                v0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                ADDI(x4, xZR, 63);
                VSRL_VX(v0, q0, x4, VECTOR_UNMASKED);
                // Force the mask element width to 32
                vector_vsetvli(dyn, ninst, x1, VECTOR_SEW64, VECTOR_LMUL2, 1);
                VMSNE_VX(VMASK, v0, xZR, VECTOR_UNMASKED);
                vector_vsetvli(dyn, ninst, x1, VECTOR_SEW64, VECTOR_LMUL1, 1);
                VMV_X_S(x4, VMASK);
                ANDI(gd, x4, 1);
                SRLI(x4, x4, 31);
                OR(gd, gd, x4);
            } else {
                VMSLT_VX(VMASK, q0, xZR, VECTOR_UNMASKED);
                VMV_X_S(x4, VMASK);
                ANDI(gd, x4, 0b11);
            }
            break;
        case 0x51:
            INST_NAME("SQRTPD Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            GETGX_vector(q0, 1, VECTOR_SEW64);
            GETEX_vector(q1, 0, 0, VECTOR_SEW64);
            if (!BOX64ENV(dynarec_fastnan)) {
                v0 = fpu_get_scratch(dyn);
                VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
                VMFLT_VV(VMASK, q1, v0, VECTOR_UNMASKED);
            }
            VFSQRT_V(q0, q1, VECTOR_UNMASKED);
            if (!BOX64ENV(dynarec_fastnan)) {
                VFSGNJN_VV(q0, q0, q0, VECTOR_MASKED);
            }
            break;
        case 0x54:
            INST_NAME("ANDPD Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEWANY, 1);
            GETGX_vector(q0, 1, dyn->vector_eew);
            GETEX_vector(q1, 0, 0, dyn->vector_eew);
            VAND_VV(q0, q1, q0, VECTOR_UNMASKED);
            break;
        case 0x55:
            INST_NAME("ANDNPD Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEWANY, 1);
            GETGX_vector(q0, 1, dyn->vector_eew);
            GETEX_vector(q1, 0, 0, dyn->vector_eew);
            VXOR_VI(q0, q0, 0x1F, VECTOR_UNMASKED);
            VAND_VV(q0, q1, q0, VECTOR_UNMASKED);
            break;
        case 0x56:
            INST_NAME("ORPD Gx, Ex");
            nextop = F8;
            GETG;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEWANY, 1);
            if (MODREG && ((nextop & 7) + (rex.b << 3) == gd)) {
                // special case for XORPD Gx, Gx
                q0 = sse_get_reg_empty_vector(dyn, ninst, x1, gd);
                VOR_VV(q0, q0, q0, VECTOR_UNMASKED);
            } else {
                GETGX_vector(q0, 1, dyn->vector_eew);
                GETEX_vector(q1, 0, 0, dyn->vector_eew);
                VOR_VV(q0, q1, q0, VECTOR_UNMASKED);
            }
            break;
        case 0x57:
            INST_NAME("XORPD Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEWANY, 1);
            GETGX_vector(q0, 1, dyn->vector_eew);
            GETEX_vector(q1, 0, 0, dyn->vector_eew);
            VXOR_VV(q0, q1, q0, VECTOR_UNMASKED);
            break;
        case 0x58:
            INST_NAME("ADDPD Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            GETGX_vector(q0, 1, VECTOR_SEW64);
            GETEX_vector(q1, 0, 0, VECTOR_SEW64);
            if (!BOX64ENV(dynarec_fastnan)) {
                v0 = fpu_get_scratch(dyn);
                VMFEQ_VV(VMASK, q0, q0, VECTOR_UNMASKED);
                VMFEQ_VV(v0, q1, q1, VECTOR_UNMASKED);
            }
            VFADD_VV(q0, q0, q1, VECTOR_UNMASKED);
            if (!BOX64ENV(dynarec_fastnan)) {
                VMAND_MM(VMASK, v0, VMASK);
                VMFEQ_VV(v0, q0, q0, VECTOR_UNMASKED);
                VXOR_VI(v0, v0, 0x1F, VECTOR_UNMASKED);
                VMAND_MM(VMASK, v0, VMASK);
                VFSGNJN_VV(q0, q0, q0, VECTOR_MASKED);
            }
            break;
        case 0x59:
            INST_NAME("MULPD Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            GETGX_vector(q0, 1, VECTOR_SEW64);
            GETEX_vector(q1, 0, 0, VECTOR_SEW64);
            if (!BOX64ENV(dynarec_fastnan)) {
                v0 = fpu_get_scratch(dyn);
                VMFEQ_VV(VMASK, q0, q0, VECTOR_UNMASKED);
                VMFEQ_VV(v0, q1, q1, VECTOR_UNMASKED);
            }
            VFMUL_VV(q0, q0, q1, VECTOR_UNMASKED);
            if (!BOX64ENV(dynarec_fastnan)) {
                VMAND_MM(VMASK, v0, VMASK);
                VMFEQ_VV(v0, q0, q0, VECTOR_UNMASKED);
                VXOR_VI(v0, v0, 0x1F, VECTOR_UNMASKED);
                VMAND_MM(VMASK, v0, VMASK);
                VFSGNJN_VV(q0, q0, q0, VECTOR_MASKED);
            }
            break;
        case 0x5A:
            INST_NAME("CVTPD2PS Gx, Ex");
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
            VFNCVT_F_F_W(d0, d1, VECTOR_UNMASKED);
            vector_vsetvli(dyn, ninst, x1, VECTOR_SEW64, VECTOR_LMUL1, 1);
            if (!cpuext.xtheadvector) VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
            VMV_X_S(x4, d0);
            VMV_S_X(v0, x4);
            break;
        case 0x5B:
            if (!BOX64ENV(dynarec_fastround)) return 0;
            INST_NAME("CVTPS2DQ Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
            GETEX_vector(v1, 0, 0, VECTOR_SEW32);
            GETGX_empty_vector(v0);
            u8 = sse_setround(dyn, ninst, x6, x4);
            VFCVT_X_F_V(v0, v1, VECTOR_UNMASKED);
            x87_restoreround(dyn, ninst, u8);
            break;
        case 0x5C:
            INST_NAME("SUBPD Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            GETGX_vector(q0, 1, VECTOR_SEW64);
            GETEX_vector(q1, 0, 0, VECTOR_SEW64);
            if (!BOX64ENV(dynarec_fastnan)) {
                v0 = fpu_get_scratch(dyn);
                VMFEQ_VV(VMASK, q0, q0, VECTOR_UNMASKED);
                VMFEQ_VV(v0, q1, q1, VECTOR_UNMASKED);
            }
            VFSUB_VV(q0, q0, q1, VECTOR_UNMASKED);
            if (!BOX64ENV(dynarec_fastnan)) {
                VMAND_MM(VMASK, v0, VMASK);
                VMFEQ_VV(v0, q0, q0, VECTOR_UNMASKED);
                VXOR_VI(v0, v0, 0x1F, VECTOR_UNMASKED);
                VMAND_MM(VMASK, v0, VMASK);
                VFSGNJN_VV(q0, q0, q0, VECTOR_MASKED);
            }
            break;
        case 0x5D:
            INST_NAME("MINPD Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            GETGX_vector(q0, 1, VECTOR_SEW64);
            GETEX_vector(q1, 0, 0, VECTOR_SEW64);
            v0 = fpu_get_scratch(dyn);
            if (!BOX64ENV(dynarec_fastnan)) {
                VMFEQ_VV(VMASK, q0, q0, VECTOR_UNMASKED);
                VMFEQ_VV(v0, q1, q1, VECTOR_UNMASKED);
            }
            VFMIN_VV(q0, q0, q1, VECTOR_UNMASKED);
            if (!BOX64ENV(dynarec_fastnan)) {
                VMAND_MM(VMASK, v0, VMASK);
                VXOR_VI(VMASK, VMASK, 0x1F, VECTOR_UNMASKED);
                VADD_VX(q0, q1, xZR, VECTOR_MASKED);
            }
            break;
        case 0x5E:
            INST_NAME("DIVPD Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            GETGX_vector(q0, 1, VECTOR_SEW64);
            GETEX_vector(q1, 0, 0, VECTOR_SEW64);
            if (!BOX64ENV(dynarec_fastnan)) {
                v0 = fpu_get_scratch(dyn);
                VMFEQ_VV(VMASK, q0, q0, VECTOR_UNMASKED);
                VMFEQ_VV(v0, q1, q1, VECTOR_UNMASKED);
            }
            VFDIV_VV(q0, q0, q1, VECTOR_UNMASKED);
            if (!BOX64ENV(dynarec_fastnan)) {
                VMAND_MM(VMASK, v0, VMASK);
                VMFEQ_VV(v0, q0, q0, VECTOR_UNMASKED);
                VXOR_VI(v0, v0, 0x1F, VECTOR_UNMASKED);
                VMAND_MM(VMASK, v0, VMASK);
                VFSGNJN_VV(q0, q0, q0, VECTOR_MASKED);
            }
            break;
        case 0x5F:
            INST_NAME("MAXPD Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            GETGX_vector(q0, 1, VECTOR_SEW64);
            GETEX_vector(q1, 0, 0, VECTOR_SEW64);
            v0 = fpu_get_scratch(dyn);
            if (!BOX64ENV(dynarec_fastnan)) {
                VMFEQ_VV(VMASK, q0, q0, VECTOR_UNMASKED);
                VMFEQ_VV(v0, q1, q1, VECTOR_UNMASKED);
            }
            VFMAX_VV(q0, q0, q1, VECTOR_UNMASKED);
            if (!BOX64ENV(dynarec_fastnan)) {
                VMAND_MM(VMASK, v0, VMASK);
                VXOR_VI(VMASK, VMASK, 0x1F, VECTOR_UNMASKED);
                VADD_VX(q0, q1, xZR, VECTOR_MASKED);
            }
            break;
        case 0x60:
            INST_NAME("PUNPCKLBW Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
            GETGX_vector(q0, 1, VECTOR_SEW8);
            GETEX_vector(q1, 0, 0, VECTOR_SEW8);
            v0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            d0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            VWADDU_VX(d0, q0, xZR, VECTOR_UNMASKED);
            VWADDU_VX(v0, q1, xZR, VECTOR_UNMASKED);
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW16, 1);
            VSLL_VI(v0, v0, 8, VECTOR_UNMASKED);
            VOR_VV(q0, d0, v0, VECTOR_UNMASKED);
            break;
        case 0x61:
            INST_NAME("PUNPCKLWD Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW16, 1);
            GETGX_vector(q0, 1, VECTOR_SEW16);
            GETEX_vector(q1, 0, 0, VECTOR_SEW16);
            v0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            d0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            VWADDU_VX(d0, q0, xZR, VECTOR_UNMASKED);
            VWADDU_VX(v0, q1, xZR, VECTOR_UNMASKED);
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
            VSLL_VI(v0, v0, 16, VECTOR_UNMASKED);
            VOR_VV(q0, d0, v0, VECTOR_UNMASKED);
            break;
        case 0x62:
            INST_NAME("PUNPCKLDQ Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
            GETGX_vector(q0, 1, VECTOR_SEW32);
            GETEX_vector(q1, 0, 0, VECTOR_SEW32);
            v0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            d0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            MOV32w(x2, 32);
            VWADDU_VX(d0, q0, xZR, VECTOR_UNMASKED);
            VWADDU_VX(v0, q1, xZR, VECTOR_UNMASKED);
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            VSLL_VX(v0, v0, x2, VECTOR_UNMASKED);
            VOR_VV(q0, d0, v0, VECTOR_UNMASKED);
            break;
        case 0x63:
            INST_NAME("PACKSSWB Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW16, 1);
            GETGX_vector(q0, 1, VECTOR_SEW16);
            GETEX_vector(q1, 0, 0, VECTOR_SEW16);
            d0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            d1 = fpu_get_scratch(dyn);
            if (cpuext.vlen >= 32) {
                vector_vsetvli(dyn, ninst, x1, VECTOR_SEW16, VECTOR_LMUL1, 2); // double the vl for slideup.
                VMV_V_V(d0, q0);
                VSLIDEUP_VI(d0, q1, 8, VECTOR_UNMASKED); // splice q0 and q1 here!
            } else {
                VMV_V_V(d0, q0);
                VMV_V_V(d1, q1);
            }
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
            VNCLIP_WI(q0, d0, 0, VECTOR_UNMASKED);
            break;
        case 0x64 ... 0x66:
            if (opcode == 0x64) {
                INST_NAME("PCMPGTB Gx, Ex");
                u8 = VECTOR_SEW8;
            } else if (opcode == 0x65) {
                INST_NAME("PCMPGTW Gx, Ex");
                u8 = VECTOR_SEW16;
            } else {
                INST_NAME("PCMPGTD Gx, Ex");
                u8 = VECTOR_SEW32;
            }
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, u8, 1);
            GETGX_vector(q0, 1, dyn->vector_eew);
            GETEX_vector(q1, 0, 0, dyn->vector_eew);
            VMSLT_VV(VMASK, q1, q0, VECTOR_UNMASKED);
            VXOR_VV(q0, q0, q0, VECTOR_UNMASKED);
            VMERGE_VIM(q0, q0, 0b11111); // implies vmask and widened it
            break;
        case 0x67:
            INST_NAME("PACKUSWB Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW16, 1);
            GETGX_vector(q0, 1, VECTOR_SEW16);
            GETEX_vector(q1, 0, 0, VECTOR_SEW16);
            d0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            d1 = fpu_get_scratch(dyn);
            if (cpuext.vlen >= 32) {
                vector_vsetvli(dyn, ninst, x1, VECTOR_SEW16, VECTOR_LMUL1, 2); // double the vl for slideup.
                if (q0 == q1) {
                    VMV_V_V(d0, q0);
                    VSLIDEUP_VI(d0, q1, 8, VECTOR_UNMASKED); // splice q0 and q1 here!
                    VMAX_VX(d0, d0, xZR, VECTOR_UNMASKED);
                } else {
                    VSLIDEUP_VI(q0, q1, 8, VECTOR_UNMASKED); // splice q0 and q1 here!
                    VMAX_VX(d0, q0, xZR, VECTOR_UNMASKED);
                }
            } else {
                VMAX_VX(d0, q0, xZR, VECTOR_UNMASKED);
                VMAX_VX(d1, q1, xZR, VECTOR_UNMASKED);
            }
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
            VNCLIPU_WI(q0, d0, 0, VECTOR_UNMASKED);
            break;
        case 0x68 ... 0x6A:
            if (opcode == 0x68) {
                INST_NAME("PUNPCKHBW Gx, Ex");
                u8 = VECTOR_SEW8;
                i32 = VECTOR_SEW16;
                s8 = 8;
                d2 = 8;
            } else if (opcode == 0x69) {
                INST_NAME("PUNPCKHWD Gx, Ex");
                u8 = VECTOR_SEW16;
                i32 = VECTOR_SEW32;
                s8 = 4;
                d2 = 16;
            } else {
                INST_NAME("PUNPCKHDQ Gx, Ex");
                u8 = VECTOR_SEW32;
                i32 = VECTOR_SEW64;
                s8 = 2;
                d2 = 32;
                MOV32w(x5, 32);
            }
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, u8, 1);
            GETGX_vector(q0, 1, dyn->vector_eew);
            GETEX_vector(q1, 0, 0, dyn->vector_eew);
            v0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            v1 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            d0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            VSLIDEDOWN_VI(v0, q0, s8, VECTOR_UNMASKED);
            VSLIDEDOWN_VI(v1, q1, s8, VECTOR_UNMASKED);
            VWADDU_VX(d0, v0, xZR, VECTOR_UNMASKED);
            VWADDU_VX(v0, v1, xZR, VECTOR_UNMASKED);
            SET_ELEMENT_WIDTH(x1, i32, 1);
            if (d2 < 32)
                VSLL_VI(v0, v0, d2, VECTOR_UNMASKED);
            else
                VSLL_VX(v0, v0, x5, VECTOR_UNMASKED);
            VOR_VV(q0, d0, v0, VECTOR_UNMASKED);
            break;
        case 0x6B:
            INST_NAME("PACKSSDW Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
            GETGX_vector(q0, 1, VECTOR_SEW32);
            GETEX_vector(q1, 0, 0, VECTOR_SEW32);
            d0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            d1 = fpu_get_scratch(dyn);
            if (cpuext.vlen >= 32) {
                vector_vsetvli(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL1, 2); // double the vl for slideup.
                VMV_V_V(d0, q0);
                VSLIDEUP_VI(d0, q1, 4, VECTOR_UNMASKED); // splice q0 and q1 here!
            } else {
                VMV_V_V(d0, q0);
                VMV_V_V(d1, q1);
            }
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW16, 1);
            VNCLIP_WI(q0, d0, 0, VECTOR_UNMASKED);
            break;
        case 0x6C:
            INST_NAME("PUNPCKLQDQ Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            // GX->q[0] = GX->q[0]; -> unchanged
            // GX->q[1] = EX->q[0];
            GETGX_vector(v0, 1, VECTOR_SEW64);
            if (MODREG) {
                v1 = sse_get_reg_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW64);
                if (v0 == v1) {
                    // for vslideup.vi, cannot be overlapped
                    v1 = fpu_get_scratch(dyn);
                    VMV_V_V(v1, v0);
                }
                VSLIDEUP_VI(v0, v1, 1, VECTOR_UNMASKED);
            } else {
                q0 = fpu_get_scratch(dyn);
                VXOR_VV(q0, q0, q0, VECTOR_UNMASKED);
                VECTOR_LOAD_VMASK(0b10, x1, 1);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, x2, &fixedaddress, rex, NULL, 0, 0);
                VLUXEI64_V(v0, q0, ed, VECTOR_MASKED, VECTOR_NFIELD1);
            }
            break;
        case 0x6D:
            INST_NAME("PUNPCKHQDQ Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            // GX->q[0] = GX->q[1];
            // GX->q[1] = EX->q[1];
            GETGX_vector(v0, 1, VECTOR_SEW64);
            if (MODREG) {
                v1 = sse_get_reg_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW64);
                q0 = fpu_get_scratch(dyn);
                VSLIDE1DOWN_VX(q0, v0, xZR, VECTOR_UNMASKED);
                if (cpuext.xtheadvector) {
                    VECTOR_LOAD_VMASK(0b01, x4, 1);
                    VMERGE_VVM(v0, v1, q0); // implies VMASK
                } else {
                    if (v0 != v1) { VMV_V_V(v0, v1); }
                    VMV_X_S(x4, q0);
                    VMV_S_X(v0, x4);
                }
            } else {
                q0 = fpu_get_scratch(dyn);
                VECTOR_LOAD_VMASK(0b10, x1, 1);
                VSLIDE1DOWN_VX(v0, v0, xZR, VECTOR_UNMASKED);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, x2, &fixedaddress, rex, NULL, 0, 0);
                VLE64_V(v0, ed, VECTOR_MASKED, VECTOR_NFIELD1);
            }
            break;
        case 0x6E:
            INST_NAME("MOVD Gx, Ed");
            nextop = F8;
            GETGX_empty_vector(v0);
            GETED(0);
            if (rex.w) {
                SET_ELEMENT_WIDTH(x3, VECTOR_SEW64, 1);
            } else {
                SET_ELEMENT_WIDTH(x3, VECTOR_SEW32, 1);
            }
            VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
            VECTOR_LOAD_VMASK(1, x4, 1);
            VMERGE_VXM(v0, v0, ed);
            break;
        case 0x6F:
            INST_NAME("MOVDQA Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            if (MODREG) {
                v1 = sse_get_reg_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, dyn->vector_eew);
                GETGX_empty_vector(v0);
                VMV_V_V(v0, v1);
            } else {
                GETGX_empty_vector(v0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 0, 0);
                VLE_V(v0, ed, dyn->vector_eew, VECTOR_UNMASKED, VECTOR_NFIELD1);
            }
            break;
        case 0x70:
            INST_NAME("PSHUFD Gx, Ex, Ib");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            GETEX_vector(q1, 0, 1, VECTOR_SEW64);
            GETGX_empty_vector(q0);
            v0 = fpu_get_scratch(dyn);
            tmp64u0 = F8;
            if (cpuext.xtheadvector) { // lack of vrgatherei16.vv
                v1 = fpu_get_scratch(dyn);
                if (tmp64u0 == 0) {
                    VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
                } else {
                    tmp64u1 = (((tmp64u0 >> 6) & 3) << 32) | ((tmp64u0 >> 4) & 3);
                    tmp64u0 = (((tmp64u0 >> 2) & 3) << 32) | (tmp64u0 & 3);
                    if (tmp64u1 == 0) {
                        VXOR_VV(v1, v1, v1, VECTOR_UNMASKED);
                    } else {
                        MOV64x(x4, tmp64u1);
                        VMV_S_X(v1, x4); // clears upper bits!
                    }
                    if (tmp64u0 == tmp64u1) {
                        VSLIDE1UP_VX(v0, v1, x4, VECTOR_UNMASKED);
                    } else if (tmp64u0 == 0) {
                        VSLIDE1UP_VX(v0, v1, xZR, VECTOR_UNMASKED);
                    } else {
                        MOV64x(x4, tmp64u0);
                        VSLIDE1UP_VX(v0, v1, x4, VECTOR_UNMASKED);
                    }
                }
                SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
                if (q0 == q1) {
                    VRGATHER_VV(v1, q1, v0, VECTOR_UNMASKED);
                    VMV_V_V(q0, v1);
                } else {
                    VRGATHER_VV(q0, q1, v0, VECTOR_UNMASKED);
                }
            } else {
                tmp64u0 = ((tmp64u0 >> 6) << 48) | (((tmp64u0 >> 4) & 3) << 32) | (((tmp64u0 >> 2) & 3) << 16) | (tmp64u0 & 3);
                VECTOR_SPLAT_IMM(v0, tmp64u0, x4);
                SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
                if (q0 == q1) {
                    v1 = fpu_get_scratch(dyn);
                    VRGATHEREI16_VV(v1, q1, v0, VECTOR_UNMASKED);
                    VMV_V_V(q0, v1);
                } else {
                    VRGATHEREI16_VV(q0, q1, v0, VECTOR_UNMASKED);
                }
            }
            break;
        case 0x71:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 2:
                    INST_NAME("PSRLW Ex, Ib");
                    SET_ELEMENT_WIDTH(x1, VECTOR_SEW16, 1);
                    GETEX_vector(q0, 1, 1, VECTOR_SEW16);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 15) {
                            VXOR_VV(q0, q0, q0, VECTOR_UNMASKED);
                        } else {
                            VSRL_VI(q0, q0, u8, VECTOR_UNMASKED);
                        }
                        PUTEX_vector(q0, VECTOR_SEW16);
                    }
                    break;
                case 4:
                    INST_NAME("PSRAW Ex, Ib");
                    SET_ELEMENT_WIDTH(x1, VECTOR_SEW16, 1);
                    GETEX_vector(q0, 1, 1, VECTOR_SEW16);
                    u8 = F8;
                    if (u8 > 15) u8 = 15;
                    if (u8) {
                        VSRA_VI(q0, q0, u8, VECTOR_UNMASKED);
                    }
                    PUTEX_vector(q0, VECTOR_SEW16);
                    break;
                case 6:
                    INST_NAME("PSLLW Ex, Ib");
                    SET_ELEMENT_WIDTH(x1, VECTOR_SEW16, 1);
                    GETEX_vector(q0, 1, 1, VECTOR_SEW16);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 15) {
                            VXOR_VV(q0, q0, q0, VECTOR_UNMASKED);
                        } else {
                            VSLL_VI(q0, q0, u8, VECTOR_UNMASKED);
                        }
                        PUTEX_vector(q0, VECTOR_SEW16);
                    }
                    break;
                default:
                    DEFAULT_VECTOR;
            }
            break;
        case 0x72:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 2:
                    INST_NAME("PSRLD Ex, Ib");
                    SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
                    GETEX_vector(q0, 1, 1, VECTOR_SEW32);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 31) {
                            VXOR_VV(q0, q0, q0, VECTOR_UNMASKED);
                        } else if (u8) {
                            VSRL_VI(q0, q0, u8, VECTOR_UNMASKED);
                        }
                        PUTEX_vector(q0, VECTOR_SEW32);
                    }
                    break;
                case 4:
                    INST_NAME("PSRAD Ex, Ib");
                    SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
                    GETEX_vector(q0, 1, 1, VECTOR_SEW32);
                    u8 = F8;
                    if (u8 > 31) u8 = 31;
                    if (u8) {
                        VSRA_VI(q0, q0, u8, VECTOR_UNMASKED);
                    }
                    PUTEX_vector(q0, VECTOR_SEW32);
                    break;
                case 6:
                    INST_NAME("PSLLD Ex, Ib");
                    SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
                    GETEX_vector(q0, 1, 1, VECTOR_SEW32);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 31) {
                            VXOR_VV(q0, q0, q0, VECTOR_UNMASKED);
                        } else {
                            VSLL_VI(q0, q0, u8, VECTOR_UNMASKED);
                        }
                        PUTEX_vector(q0, VECTOR_SEW32);
                    }
                    break;
                default:
                    DEFAULT_VECTOR;
            }
            break;
        case 0x73:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 2:
                    INST_NAME("PSRLQ Ex, Ib");
                    SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
                    GETEX_vector(q0, 1, 1, VECTOR_SEW64);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 63) {
                            VXOR_VV(q0, q0, q0, VECTOR_UNMASKED);
                        } else {
                            MOV64x(x4, u8);
                            VSRL_VX(q0, q0, x4, VECTOR_UNMASKED);
                        }
                        PUTEX_vector(q0, VECTOR_SEW64);
                    }
                    break;
                case 3:
                    INST_NAME("PSRLDQ Ex, Ib");
                    SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
                    GETEX_vector(q0, 1, 1, VECTOR_SEW8);
                    u8 = F8;
                    if (!u8) break;
                    if (u8 > 15) {
                        VXOR_VV(q0, q0, q0, VECTOR_UNMASKED);
                    } else {
                        d0 = fpu_get_scratch(dyn);
                        if (cpuext.vlen >= 32) {
                            // clear high bits before slidedown!
                            vector_vsetvli(dyn, ninst, x1, VECTOR_SEW8, VECTOR_LMUL1, 2);
                            VXOR_VV(d0, d0, d0, VECTOR_UNMASKED);
                            vector_vsetvli(dyn, ninst, x1, VECTOR_SEW8, VECTOR_LMUL1, 1);
                        }
                        VMV_V_V(d0, q0);
                        VSLIDEDOWN_VI(q0, d0, u8, VECTOR_UNMASKED);
                    }
                    PUTEX_vector(q0, VECTOR_SEW8);
                    break;
                case 6:
                    INST_NAME("PSLLQ Ex, Ib");
                    SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
                    GETEX_vector(q0, 1, 1, VECTOR_SEW64);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 63) {
                            VXOR_VV(q0, q0, q0, VECTOR_UNMASKED);
                        } else {
                            MOV64x(x4, u8);
                            VSLL_VX(q0, q0, x4, VECTOR_UNMASKED);
                        }
                        PUTEX_vector(q0, VECTOR_SEW64);
                    }
                    break;
                case 7:
                    INST_NAME("PSLLDQ Ex, Ib");
                    SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
                    GETEX_vector(q0, 1, 1, VECTOR_SEW8);
                    u8 = F8;
                    if (!u8) break;
                    if (u8 > 15) {
                        VXOR_VV(q0, q0, q0, VECTOR_UNMASKED);
                        PUTEX_vector(q0, VECTOR_SEW8);
                    } else {
                        d0 = fpu_get_scratch(dyn);
                        VXOR_VV(d0, d0, d0, VECTOR_UNMASKED);
                        VSLIDEUP_VI(d0, q0, u8, VECTOR_UNMASKED);
                        if (MODREG) {
                            VMV_V_V(q0, d0);
                        } else {
                            PUTEX_vector(d0, VECTOR_SEW8);
                        }
                    }
                    break;
                default: DEFAULT_VECTOR;
            }
            break;
        case 0x74 ... 0x76:
            if (opcode == 0x74) {
                INST_NAME("PCMPEQB Gx, Ex");
                u8 = VECTOR_SEW8;
            } else if (opcode == 0x75) {
                INST_NAME("PCMPEQW Gx, Ex");
                u8 = VECTOR_SEW16;
            } else {
                INST_NAME("PCMPEQD Gx, Ex");
                u8 = VECTOR_SEW32;
            }
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, u8, 1);
            GETGX_vector(q0, 1, dyn->vector_eew);
            GETEX_vector(q1, 0, 0, dyn->vector_eew);
            VMSEQ_VV(VMASK, q1, q0, VECTOR_UNMASKED);
            VXOR_VV(q0, q0, q0, VECTOR_UNMASKED);
            VMERGE_VIM(q0, q0, 0b11111); // implies vmask and widened it
            break;
        case 0x7C:
            INST_NAME("HADDPD Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            GETGX_vector(q0, 1, VECTOR_SEW64);
            GETEX_vector(q1, 0, 0, VECTOR_SEW64);
            v0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            v1 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            d0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2); // no more scratches!
            VMV_V_V(v0, q0);
            if (q1 & 1) VMV_V_V(d1, q1);
            VMV_V_I(VMASK, cpuext.xtheadvector ? 1 : 0b0101);
            vector_vsetvli(dyn, ninst, x1, VECTOR_SEW64, VECTOR_LMUL2, 2);
            VSLIDEUP_VI(v0, (q1 & 1) ? d1 : q1, 2, VECTOR_UNMASKED);
            VCOMPRESS_VM(d0, v0, VMASK);
            VXOR_VI(VMASK, VMASK, 0x1F, VECTOR_UNMASKED);
            VCOMPRESS_VM(d1, v0, VMASK);
            vector_vsetvli(dyn, ninst, x1, VECTOR_SEW64, VECTOR_LMUL1, 1);
            if (!BOX64ENV(dynarec_fastnan)) {
                VMFEQ_VV(v0, d0, d0, VECTOR_UNMASKED);
                VMFEQ_VV(v1, d1, d1, VECTOR_UNMASKED);
                VMAND_MM(v0, v0, v1);
            }
            VFADD_VV(q0, d0, d1, VECTOR_UNMASKED);
            if (!BOX64ENV(dynarec_fastnan)) {
                VMFEQ_VV(v1, q0, q0, VECTOR_UNMASKED);
                VMANDN_MM(VMASK, v0, v1);
                VFSGNJN_VV(q0, q0, q0, VECTOR_MASKED);
            }
            break;
        case 0x7E:
            return 0;
        case 0x7F:
            INST_NAME("MOVDQA Ex, Gx");
            nextop = F8;
            GETG;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                v0 = sse_get_reg_empty_vector(dyn, ninst, x1, ed);
                v1 = sse_get_reg_vector(dyn, ninst, x1, gd, 0, dyn->vector_eew);
                VMV_V_V(v0, v1);
            } else {
                SMREAD();
                v1 = sse_get_reg_vector(dyn, ninst, x1, gd, 0, dyn->vector_eew);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                VSE_V(v1, ed, dyn->vector_eew, VECTOR_UNMASKED, VECTOR_NFIELD1);
            }
            break;
        case 0xA3 ... 0xC1: return 0;
        case 0xC2:
            INST_NAME("CMPPD Gx, Ex, Ib");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            GETGX_vector(v0, 1, VECTOR_SEW64);
            GETEX_vector(v1, 0, 1, VECTOR_SEW64);
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
        case 0xC4:
            INST_NAME("PINSRW Gx, Ed, Ib");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW16, 1);
            GETGX_vector(q0, 1, VECTOR_SEW16);
            if (MODREG) {
                u8 = (F8) & 7;
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 1);
                u8 = (F8) & 7;
                LHU(x4, ed, fixedaddress);
                ed = x4;
            }
            VECTOR_LOAD_VMASK((1 << u8), x5, 1);
            v0 = fpu_get_scratch(dyn);
            VMERGE_VXM(v0, q0, ed); // uses VMASK
            VMV_V_V(q0, v0);
            break;
        case 0xC5:
            INST_NAME("PEXTRW Gd, Ex, Ib");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW16, 1);
            GETGD;
            if (MODREG) {
                GETEX_vector(q0, 0, 1, VECTOR_SEW16);
                u8 = (F8) & 7;
                v0 = fpu_get_scratch(dyn);
                VSLIDEDOWN_VI(v0, q0, u8, VECTOR_UNMASKED);
                VMV_X_S(gd, v0);
                ZEXTH(gd, gd);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 1);
                u8 = (F8) & 7;
                LHU(gd, ed, u8 * 2);
            }
            break;
        case 0xC6:
            INST_NAME("SHUFPD Gx, Ex, Ib");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            GETGX_vector(v0, 1, VECTOR_SEW64);
            GETEX_vector(v1, 0, 1, VECTOR_SEW64);
            u8 = F8;
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            if ((u8 >> 1) & 1) {
                VSLIDE1DOWN_VX(d1, v1, xZR, VECTOR_UNMASKED);
                v1 = d1;
            } else if (v0 == v1) {
                v1 = fpu_get_scratch(dyn);
                VMV_V_V(v1, v0);
            }
            if (u8 & 1) {
                if (cpuext.xtheadvector) {
                    ADDI(x5, xZR, 1);
                    VEXT_X_V(x4, v0, x5);
                } else {
                    VSLIDE1DOWN_VX(d0, v0, xZR, VECTOR_UNMASKED);
                    VMV_X_S(x4, d0);
                }
            } else {
                d0 = v0;
                VMV_X_S(x4, d0);
            }
            VSLIDE1UP_VX(v0, v1, x4, VECTOR_UNMASKED);
            break;
        case 0xD1:
        case 0xD2:
        case 0xD3:
            if (opcode == 0xD1) {
                INST_NAME("PSRLW Gx, Ex");
                u8 = VECTOR_SEW16;
                i32 = 16;
            } else if (opcode == 0xD2) {
                INST_NAME("PSRLD Gx, Ex");
                u8 = VECTOR_SEW32;
                i32 = 32;
            } else {
                INST_NAME("PSRLQ Gx, Ex");
                u8 = VECTOR_SEW64;
                i32 = 64;
            }
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            GETGX_vector(q0, 1, VECTOR_SEW64);
            if (MODREG) {
                q1 = sse_get_reg_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW64);
                VMV_X_S(x4, q1);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x2, &fixedaddress, rex, NULL, 0, 0);
                LD(x4, wback, fixedaddress);
            }
            SET_ELEMENT_WIDTH(x1, u8, 1);
            SLTIU(x3, x4, i32);
            SUB(x3, xZR, x3);
            VSRL_VX(q0, q0, x4, VECTOR_UNMASKED);
            VAND_VX(q0, q0, x3, VECTOR_UNMASKED);
            break;
        case 0xD4:
            INST_NAME("PADDQ Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            GETGX_vector(q0, 1, VECTOR_SEW64);
            GETEX_vector(q1, 0, 0, VECTOR_SEW64);
            VADD_VV(q0, q1, q0, VECTOR_UNMASKED);
            break;
        case 0xD5:
            INST_NAME("PMULLW Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW16, 1);
            GETGX_vector(q0, 1, VECTOR_SEW16);
            GETEX_vector(q1, 0, 0, VECTOR_SEW16);
            VMUL_VV(q0, q1, q0, VECTOR_UNMASKED);
            break;
        case 0xD6:
            INST_NAME("MOVQ Ex, Gx");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            GETGX_vector(q0, 1, VECTOR_SEW64);
            if (MODREG) {
                q1 = sse_get_reg_empty_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3));
                VMV_X_S(x4, q0);
                if (!cpuext.xtheadvector) {
                    VXOR_VV(q1, q1, q1, VECTOR_UNMASKED);
                }
                VMV_S_X(q1, x4);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                VMV_X_S(x4, q0);
                SD(x4, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0xD7:
            INST_NAME("PMOVMSKB Gd, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
            GETGD;
            GETEX_vector(q0, 0, 0, VECTOR_SEW8);
            if (cpuext.xtheadvector) {
                v0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL8);
                VSRL_VI(v0, q0, 7, VECTOR_UNMASKED);
                // Force the element width to 1bit
                vector_vsetvli(dyn, ninst, x4, VECTOR_SEW8, VECTOR_LMUL8, 1);
                VMSNE_VX(VMASK, v0, xZR, VECTOR_UNMASKED);
            } else {
                VMSLT_VX(VMASK, q0, xZR, VECTOR_UNMASKED);
            }
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW16, 1);
            VMV_X_S(gd, VMASK);
            if (!cpuext.xtheadvector) { ZEXTH(gd, gd); }
            break;
        case 0xD8:
        case 0xD9:
            if (opcode == 0xD8) {
                INST_NAME("PSUBUSB Gx, Ex");
                u8 = VECTOR_SEW8;
            } else {
                INST_NAME("PSUBUSW Gx, Ex");
                u8 = VECTOR_SEW16;
            }
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, u8, 1);
            GETGX_vector(q0, 1, u8);
            GETEX_vector(q1, 0, 0, u8);
            VSSUBU_VV(q0, q0, q1, VECTOR_UNMASKED);
            break;
        case 0xDA:
            INST_NAME("PMINUB Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
            GETGX_vector(q0, 1, VECTOR_SEW8);
            GETEX_vector(q1, 0, 0, VECTOR_SEW8);
            VMINU_VV(q0, q1, q0, VECTOR_UNMASKED);
            break;
        case 0xDB:
            INST_NAME("PAND Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEWANY, 1);
            GETGX_vector(q0, 1, dyn->vector_eew);
            GETEX_vector(q1, 0, 0, dyn->vector_eew);
            VAND_VV(q0, q1, q0, VECTOR_UNMASKED);
            break;
        case 0xDC:
        case 0xDD:
            if (opcode == 0xDC) {
                INST_NAME("PADDUSB Gx, Ex");
                u8 = VECTOR_SEW8;
            } else {
                INST_NAME("PADDUSW Gx, Ex");
                u8 = VECTOR_SEW16;
            }
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, u8, 1);
            GETGX_vector(q0, 1, u8);
            GETEX_vector(q1, 0, 0, u8);
            VSADDU_VV(q0, q0, q1, VECTOR_UNMASKED);
            break;
        case 0xDE:
            INST_NAME("PMAXUB Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
            GETGX_vector(q0, 1, VECTOR_SEW8);
            GETEX_vector(q1, 0, 0, VECTOR_SEW8);
            VMAXU_VV(q0, q1, q0, VECTOR_UNMASKED);
            break;
        case 0xDF:
            INST_NAME("PANDN Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEWANY, 1);
            GETGX_vector(q0, 1, dyn->vector_eew);
            GETEX_vector(q1, 0, 0, dyn->vector_eew);
            VXOR_VI(q0, q0, 0x1F, VECTOR_UNMASKED);
            VAND_VV(q0, q1, q0, VECTOR_UNMASKED);
            break;
        case 0xE0:
            INST_NAME("PAVGB Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
            GETGX_vector(q0, 1, VECTOR_SEW8);
            GETEX_vector(q1, 0, 0, VECTOR_SEW8);
            if (cpuext.xtheadvector) { // lack of vaddu.vv
                v0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                VWADDU_VV(v0, q0, q1, VECTOR_UNMASKED);
                vector_vsetvli(dyn, ninst, x1, VECTOR_SEW16, VECTOR_LMUL2, 2);
                VADD_VI(v0, v0, 1, VECTOR_UNMASKED);
                vector_vsetvli(dyn, ninst, x1, VECTOR_SEW8, VECTOR_LMUL1, 1);
                VNSRL_WI(q0, v0, 1, VECTOR_UNMASKED);
            } else {
                CSRRWI(xZR, 0b00 /* rnu */, 0x00A /* vxrm */);
                VAADDU_VV(q0, q0, q1, VECTOR_UNMASKED);
            }
            break;
        case 0xE1:
            INST_NAME("PSRAW Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            GETGX_vector(q0, 1, VECTOR_SEW64);
            VECTOR_LOAD_VMASK(1, x1, 1);
            if (MODREG) {
                q1 = sse_get_reg_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW64);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, x2, &fixedaddress, rex, NULL, 0, 0);
                q1 = fpu_get_scratch(dyn);
                VLE_V(q1, ed, VECTOR_SEW64, VECTOR_MASKED, VECTOR_NFIELD1);
            }
            v1 = fpu_get_scratch(dyn);
            ADDI(x4, xZR, 15);
            VMINU_VX(v1, q1, x4, VECTOR_MASKED);
            VMV_X_S(x4, v1);
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW16, 1);
            VSRA_VX(q0, q0, x4, VECTOR_UNMASKED);
            break;
        case 0xE2:
            INST_NAME("PSRAD Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            GETGX_vector(q0, 1, VECTOR_SEW64);
            VECTOR_LOAD_VMASK(1, x1, 1);
            if (MODREG) {
                q1 = sse_get_reg_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW64);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, x2, &fixedaddress, rex, NULL, 0, 0);
                q1 = fpu_get_scratch(dyn);
                VLE_V(q1, ed, VECTOR_SEW64, VECTOR_MASKED, VECTOR_NFIELD1);
            }
            v1 = fpu_get_scratch(dyn);
            ADDI(x4, xZR, 31);
            VMINU_VX(v1, q1, x4, VECTOR_MASKED);
            VMV_X_S(x4, v1);
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
            VSRA_VX(q0, q0, x4, VECTOR_UNMASKED);
            break;
        case 0xE3:
            INST_NAME("PAVGW Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW16, 1);
            GETGX_vector(q0, 1, VECTOR_SEW16);
            GETEX_vector(q1, 0, 0, VECTOR_SEW16);
            if (cpuext.xtheadvector) { // lack of vaddu.vv
                v0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                VWADDU_VV(v0, q0, q1, VECTOR_UNMASKED);
                vector_vsetvli(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL2, 2);
                VADD_VI(v0, v0, 1, VECTOR_UNMASKED);
                vector_vsetvli(dyn, ninst, x1, VECTOR_SEW16, VECTOR_LMUL1, 1);
                VNSRL_WI(q0, v0, 1, VECTOR_UNMASKED);
            } else {
                CSRRWI(xZR, 0b00 /* rnu */, 0x00A /* vxrm */);
                VAADDU_VV(q0, q0, q1, VECTOR_UNMASKED);
            }
            break;
        case 0xE4:
            INST_NAME("PMULHUW Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW16, 1);
            GETGX_vector(q0, 1, VECTOR_SEW16);
            GETEX_vector(q1, 0, 0, VECTOR_SEW16);
            VMULHU_VV(q0, q0, q1, VECTOR_UNMASKED);
            break;
        case 0xE5:
            INST_NAME("PMULHW Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW16, 1);
            GETGX_vector(q0, 1, VECTOR_SEW16);
            GETEX_vector(q1, 0, 0, VECTOR_SEW16);
            VMULH_VV(q0, q0, q1, VECTOR_UNMASKED);
            break;
        case 0xE6:
            if (!BOX64ENV(dynarec_fastround)) return 0;
            INST_NAME("CVTTPD2DQ Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
            GETEX_vector(v1, 0, 0, VECTOR_SEW32);
            GETGX_empty_vector(v0);
            d0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            VMV_V_V(d0, v1);
            if (cpuext.xtheadvector) {
                vector_vsetvli(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL1, 0.5);
                ADDI(x4, xZR, 1); // RTZ
                FSRM(x4, x4);
                VFNCVT_X_F_W(v0, d0, VECTOR_UNMASKED);
                FSRM(xZR, x4);
            } else {
                VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
                vector_vsetvli(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL1, 0.5);
                VFNCVT_RTZ_X_F_W(v0, d0, VECTOR_UNMASKED);
            }
            vector_vsetvli(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL1, 1);
            break;
        case 0xE7:
            INST_NAME("MOVNTDQ Ex, Gx");
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
        case 0xE8:
            INST_NAME("PSUBSB Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
            GETGX_vector(q0, 1, VECTOR_SEW8);
            GETEX_vector(q1, 0, 0, VECTOR_SEW8);
            VSSUB_VV(q0, q0, q1, VECTOR_UNMASKED);
            break;
        case 0xE9:
            INST_NAME("PSUBSW Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW16, 1);
            GETGX_vector(q0, 1, VECTOR_SEW16);
            GETEX_vector(q1, 0, 0, VECTOR_SEW16);
            VSSUB_VV(q0, q0, q1, VECTOR_UNMASKED);
            break;
        case 0xEA:
            INST_NAME("PMINSW Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW16, 1);
            GETGX_vector(q0, 1, VECTOR_SEW16);
            GETEX_vector(q1, 0, 0, VECTOR_SEW16);
            VMIN_VV(q0, q1, q0, VECTOR_UNMASKED);
            break;
        case 0xEB:
            INST_NAME("POR Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEWANY, 1);
            GETGX_vector(q0, 1, dyn->vector_eew);
            GETEX_vector(q1, 0, 0, dyn->vector_eew);
            VOR_VV(q0, q1, q0, VECTOR_UNMASKED);
            break;
        case 0xEC:
            INST_NAME("PADDSB Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
            GETGX_vector(q0, 1, VECTOR_SEW8);
            GETEX_vector(q1, 0, 0, VECTOR_SEW8);
            VSADD_VV(q0, q0, q1, VECTOR_UNMASKED);
            break;
        case 0xED:
            INST_NAME("PADDSW Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW16, 1);
            GETGX_vector(q0, 1, VECTOR_SEW16);
            GETEX_vector(q1, 0, 0, VECTOR_SEW16);
            VSADD_VV(q0, q0, q1, VECTOR_UNMASKED);
            break;
        case 0xEE:
            INST_NAME("PMAXSW Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW16, 1);
            GETGX_vector(q0, 1, VECTOR_SEW16);
            GETEX_vector(q1, 0, 0, VECTOR_SEW16);
            VMAX_VV(q0, q1, q0, VECTOR_UNMASKED);
            break;
        case 0xEF:
            INST_NAME("PXOR Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG && gd == (nextop & 7) + (rex.b << 3)) {
                SET_ELEMENT_WIDTH(x1, VECTOR_SEWANY, 1);
                // special case
                q0 = sse_get_reg_empty_vector(dyn, ninst, x1, gd);
                VXOR_VV(q0, q0, q0, VECTOR_UNMASKED);
            } else {
                SET_ELEMENT_WIDTH(x1, VECTOR_SEWANY, 1);
                q0 = sse_get_reg_vector(dyn, ninst, x1, gd, 1, dyn->vector_eew);
                GETEX_vector(q1, 0, 0, dyn->vector_eew);
                VXOR_VV(q0, q1, q0, VECTOR_UNMASKED);
            }
            break;
        case 0xF1:
        case 0xF2:
        case 0xF3:
            if (opcode == 0xF1) {
                INST_NAME("PSLLW Gx, Ex");
                u8 = VECTOR_SEW16;
                i32 = 16;
            } else if (opcode == 0xF2) {
                INST_NAME("PSLLD Gx, Ex");
                u8 = VECTOR_SEW32;
                i32 = 32;
            } else {
                INST_NAME("PSLLQ Gx, Ex");
                u8 = VECTOR_SEW64;
                i32 = 64;
            }
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            GETGX_vector(q0, 1, VECTOR_SEW64);
            if (MODREG) {
                q1 = sse_get_reg_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW64);
                VMV_X_S(x4, q1);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x2, &fixedaddress, rex, NULL, 0, 0);
                LD(x4, wback, fixedaddress);
            }
            SET_ELEMENT_WIDTH(x1, u8, 1);
            SLTIU(x3, x4, i32);
            SUB(x3, xZR, x3);
            VSLL_VX(q0, q0, x4, VECTOR_UNMASKED);
            VAND_VX(q0, q0, x3, VECTOR_UNMASKED);
            break;
        case 0xF4:
            INST_NAME("PMULUDQ Gx, Ex");
            nextop = F8;
            if (cpuext.vlen >= 32) {
                SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
                GETGX_vector(v0, 1, VECTOR_SEW64);
                GETEX_vector(v1, 0, 0, VECTOR_SEW64);
                d1 = fpu_get_scratch(dyn);
                ADDI(x2, xZR, 32);
                VSLL_VX(v0, v0, x2, VECTOR_UNMASKED);
                VSRL_VX(v0, v0, x2, VECTOR_UNMASKED);
                VSLL_VX(d1, v1, x2, VECTOR_UNMASKED);
                VSRL_VX(d1, d1, x2, VECTOR_UNMASKED);
                VMUL_VV(v0, v0, d1, VECTOR_UNMASKED);
            } else {
                SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
                GETGX_vector(v0, 1, VECTOR_SEW32);
                GETEX_vector(v1, 0, 0, VECTOR_SEW32);
                d0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
                d1 = fpu_get_scratch(dyn);
                VWMULU_VV(d0, v0, v1, VECTOR_UNMASKED);
                SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
                VSLIDEUP_VI(d0, d1, 1, VECTOR_UNMASKED);
                VMV_V_V(v0, d0);
            }
            break;
        case 0xF5:
            INST_NAME("PMADDWD Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW16, 1);
            GETGX_vector(v0, 1, VECTOR_SEW16);
            GETEX_vector(v1, 0, 0, VECTOR_SEW16);
            q1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            ADDI(x3, xZR, 32);
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW16, 1);
            VWMUL_VV(q0, v1, v0, VECTOR_UNMASKED);
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
            VNSRL_WX(q1, q0, x3, VECTOR_UNMASKED);
            VNSRL_WI(v0, q0, 0, VECTOR_UNMASKED);
            VADD_VV(v0, v0, q1, VECTOR_UNMASKED);
            break;
        case 0xF6:
            INST_NAME("PSADBW Gx, Ex");
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
            GETGX_vector(q0, 1, VECTOR_SEW8);
            GETEX_vector(q1, 0, 0, VECTOR_SEW8);
            v0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            v1 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            d0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2); // no more scratches!
            VWSUBU_VV(v0, q0, q1, VECTOR_UNMASKED);
            vector_vsetvli(dyn, ninst, x1, VECTOR_SEW16, VECTOR_LMUL2, 2);
            VSRA_VI(v1, v0, 15, VECTOR_UNMASKED);
            VXOR_VV(v0, v1, v0, VECTOR_UNMASKED);
            VSUB_VV(v1, v0, v1, VECTOR_UNMASKED);
            VECTOR_LOAD_VMASK(0xFF, x4, 2);
            VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
            VREDSUM_VS(v0, v1, v0, VECTOR_MASKED); // sum low 64
            VSLIDEDOWN_VI(d0, v1, 8, VECTOR_UNMASKED);
            VXOR_VV(v1, v1, v1, VECTOR_UNMASKED);
            VREDSUM_VS(v1, d0, v1, VECTOR_MASKED); // sum high 64
            VSLIDEUP_VI(v0, v1, 4, VECTOR_UNMASKED);
            vector_vsetvli(dyn, ninst, x1, VECTOR_SEW8, VECTOR_LMUL1, 1);
            VMV_V_V(q0, v0);
            break;
        case 0xF8 ... 0xFB:
            if (opcode == 0xF8) {
                INST_NAME("PSUBB Gx, Ex");
                u8 = VECTOR_SEW8;
            } else if (opcode == 0xF9) {
                INST_NAME("PSUBW Gx, Ex");
                u8 = VECTOR_SEW16;
            } else if (opcode == 0xFA) {
                INST_NAME("PSUBD Gx, Ex");
                u8 = VECTOR_SEW32;
            } else {
                INST_NAME("PSUBQ Gx, Ex");
                u8 = VECTOR_SEW64;
            }
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, u8, 1);
            GETGX_vector(q0, 1, u8);
            GETEX_vector(q1, 0, 0, u8);
            VSUB_VV(q0, q0, q1, VECTOR_UNMASKED);
            break;
        case 0xFC ... 0xFE:
            nextop = F8;
            if (opcode == 0xFC) {
                INST_NAME("PADDB Gx, Ex");
                u8 = VECTOR_SEW8;
            } else if (opcode == 0xFD) {
                INST_NAME("PADDW Gx, Ex");
                u8 = VECTOR_SEW16;
            } else {
                INST_NAME("PADDD Gx, Ex");
                u8 = VECTOR_SEW32;
            }
            SET_ELEMENT_WIDTH(x1, u8, 1);
            GETGX_vector(q0, 1, dyn->vector_eew);
            GETEX_vector(q1, 0, 0, dyn->vector_eew);
            VADD_VV(q0, q1, q0, VECTOR_UNMASKED);
            break;
        default:
            DEFAULT_VECTOR;
    }

    return addr;
}
