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
#include "bitutils.h"

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_functions.h"
#include "dynarec_rv64_helper.h"

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
            if (MODREG) {
                SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
                ed = (nextop & 7) + (rex.b << 3);
                v0 = sse_get_reg_vector(dyn, ninst, x1, gd, 1, VECTOR_SEW32);
                v1 = sse_get_reg_vector(dyn, ninst, x1, ed, 0, VECTOR_SEW32);
                if (rv64_xtheadvector) {
                    vector_loadmask(dyn, ninst, VMASK, 0b0001, x4, 1);
                    VMERGE_VVM(v0, v0, v1); // implies VMASK
                } else {
                    VMV_X_S(x4, v1);
                    VMV_S_X(v0, x4);
                }
            } else {
                SMREAD();
                SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
                v0 = sse_get_reg_empty_vector(dyn, ninst, x1, gd);
                d0 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 0, 0);
                vector_loadmask(dyn, ninst, VMASK, 0xF, x4, 1);
                VLE8_V(d0, ed, VECTOR_MASKED, VECTOR_NFIELD1);
                VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
                VMERGE_VVM(v0, v0, d0); // implies VMASK
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
                    vector_loadmask(dyn, ninst, VMASK, 0b0001, x4, 1);
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
                vector_loadmask(dyn, ninst, VMASK, 0b0001, x4, 1);
                VMERGE_VVM(v0, v0, v1); // implies VMASK
            } else {
                VFMV_S_F(v0, v0);
            }
            break;
        case 0x38:
            return 0;
        case 0x59:
            INST_NAME("MULSS Gx, Ex");
            nextop = F8;
            if (MODREG) {
                SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
                GETGX_vector(v0, 1, VECTOR_SEW32);
                v1 = sse_get_reg_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW32);
            } else {
                SMREAD();
                v1 = fpu_get_scratch(dyn);
                SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 0, 0);
                vector_loadmask(dyn, ninst, VMASK, 0xFF, x4, 1);
                VLE8_V(v1, ed, VECTOR_MASKED, VECTOR_NFIELD1);
                SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
                GETGX_vector(v0, 1, VECTOR_SEW32);
            }
            if (box64_dynarec_fastnan) {
                vector_loadmask(dyn, ninst, VMASK, 0b0001, x4, 1);
                VFMUL_VV(v0, v0, v1, VECTOR_MASKED);
            } else {
                VFMV_F_S(v0, v0);
                VFMV_F_S(v1, v1);
                FEQS(x3, v0, v0);
                FEQS(x4, v1, v1);
                FMULS(v0, v0, v1);
                AND(x3, x3, x4);
                BEQZ_MARK(x3);
                FEQS(x3, v0, v0);
                BNEZ_MARK(x3);
                FNEGS(v0, v0);
                MARK;
                if (rv64_xtheadvector) {
                    d0 = fpu_get_scratch(dyn);
                    VFMV_S_F(d0, v0);
                    vector_loadmask(dyn, ninst, VMASK, 0b0001, x4, 1);
                    VMERGE_VVM(v0, v0, d0); // implies VMASK
                } else {
                    VFMV_S_F(v0, v0);
                }
            }
            break;
        case 0x5A:
            INST_NAME("CVTSS2SD Gx, Ex");
            nextop = F8;
            if (MODREG) {
                SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
                GETGX_vector(v0, 1, VECTOR_SEW32);
                v1 = sse_get_reg_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW32);
            } else {
                SMREAD();
                v1 = fpu_get_scratch(dyn);
                SET_ELEMENT_WIDTH(x1, VECTOR_SEW8, 1);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 0, 0);
                vector_loadmask(dyn, ninst, VMASK, 0xFF, x4, 1);
                VLE8_V(v1, ed, VECTOR_MASKED, VECTOR_NFIELD1);
                SET_ELEMENT_WIDTH(x1, VECTOR_SEW32, 1);
                GETGX_vector(v0, 1, VECTOR_SEW32);
            }
            d0 = fpu_get_scratch_lmul(dyn, VECTOR_LMUL2);
            vector_loadmask(dyn, ninst, VMASK, 0b0001, x4, 1);
            VFWCVT_F_F_V(d0, v1, VECTOR_MASKED);
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            if (rv64_xtheadvector) {
                VMERGE_VVM(v0, v0, d0); // implies VMASK
            } else {
                VMV_X_S(x4, d0);
                VMV_S_X(v0, x4);
            }
            break;
        case 0xAE:
        case 0xB8:
        case 0xBC:
        case 0xBD:
            return 0;
        default:
            DEFAULT_VECTOR;
    }
    return addr;
}
