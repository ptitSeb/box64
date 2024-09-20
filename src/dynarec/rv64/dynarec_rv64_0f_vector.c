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
                GETGX_vector(v0, 1, dyn->vector_eew);
                GETEX_vector(v1, 0, 0, VECTOR_SEW64);
                q0 = fpu_get_scratch(dyn);
                VSLIDEDOWN_VI(q0, 1, v1, VECTOR_UNMASKED);
                VMV_X_S(x4, q0);
                VMV_S_X(v0, x4);
            } else {
                INST_NAME("MOVLPS Gx, Ex");
                SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
                GETGX_vector(v0, 1, VECTOR_SEW64);
                GETEX_vector(v1, 0, 0, VECTOR_SEW64);
                VMV_X_S(x4, v1);
                VMV_S_X(v0, x4);
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
                VSLIDEUP_VI(v0, 1, v1, VECTOR_UNMASKED);
            } else {
                INST_NAME("MOVHPS Gx, Ex");
                SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
                GETGX_vector(v0, 1, VECTOR_SEW8);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, x2, &fixedaddress, rex, NULL, 0, 0);
                v1 = fpu_get_scratch(dyn);
                MOV64x(x4, 0xFF);
                VMV_S_X(VMASK, x4);
                VLE8_V(v1, ed, VECTOR_MASKED, VECTOR_NFIELD1);
                VSLIDEUP_VI(v0, 8, v1, VECTOR_UNMASKED);
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
                VSLIDE1DOWN_VX(q0, xZR, v0, VECTOR_UNMASKED);
                VMV_X_S(x4, q0);
                VMV_S_X(v1, x4);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                q0 = fpu_get_scratch(dyn);
                VSLIDE1DOWN_VX(q0, xZR, v0, VECTOR_UNMASKED);
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
            tmp64u0 = (((u8 >> 2) & 3) << 16) | (u8 & 3);
            VECTOR_SPLAT_IMM(q0, tmp64u0, x4);
            VRGATHEREI16_VV(d0, q0, v0, VECTOR_UNMASKED);
            tmp64u1 = (((u8 >> 6) & 3) << 16) | ((u8 >> 4) & 3);
            if (tmp64u1 != tmp64u0) {
                VECTOR_SPLAT_IMM(q0, tmp64u1, x4);
            }
            VRGATHEREI16_VV(d1, q0, v1, VECTOR_UNMASKED);
            VMV_V_V(v0, d0);
            VSLIDEUP_VI(v0, 2, d1, VECTOR_UNMASKED);
            break;
        case 0x00 ... 0x0F:
        case 0x18:
        case 0x1F:
        case 0x2C ... 0x2F:
        case 0x31:
        case 0x40 ... 0x4F:
        case 0x80 ... 0xBF:
        case 0xC8 ... 0xCF:
            return 0;
        default:
            DEFAULT_VECTOR;
    }
    return addr;
}
