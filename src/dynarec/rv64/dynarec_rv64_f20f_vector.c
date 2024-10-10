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

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_functions.h"
#include "dynarec_rv64_helper.h"

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
    int q0;
    int d0, d1;
    int s0, s1;
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
            if (MODREG) {
                SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
                ed = (nextop & 7) + (rex.b << 3);
                v0 = sse_get_reg_vector(dyn, ninst, x1, gd, 1, VECTOR_SEW64);
                v1 = sse_get_reg_vector(dyn, ninst, x1, ed, 0, VECTOR_SEW64);
                if (rv64_xtheadvector) {
                    vector_loadmask(dyn, ninst, VMASK, 0b01, x4, 1);
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
                vector_loadmask(dyn, ninst, VMASK, 0xFF, x4, 1);
                VLE8_V(d0, ed, VECTOR_MASKED, VECTOR_NFIELD1);
                VXOR_VV(v0, v0, v0, VECTOR_UNMASKED);
                VMERGE_VVM(v0, v0, d0); // implies VMASK
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
                    vector_loadmask(dyn, ninst, VMASK, 0b01, x4, 1);
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
                vector_loadmask(dyn, ninst, VMASK, 0b01, x4, 1);
                VMERGE_VVM(v0, v0, v1); // implies VMASK
            } else {
                VFMV_S_F(v0, v0);
            }
            break;
        case 0x38:
            return 0;
        case 0x59:
            INST_NAME("MULSD Gx, Ex"); // TODO: box64_dynarec_fastnan
            nextop = F8;
            SET_ELEMENT_WIDTH(x1, VECTOR_SEW64, 1);
            GETGX_vector(v0, 1, VECTOR_SEW64);
            v1 = fpu_get_scratch(dyn);
            vector_loadmask(dyn, ninst, VMASK, 0b01, x4, 1);
            if (MODREG) {
                v1 = sse_get_reg_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VECTOR_SEW64);
            } else {
                SMREAD();
                v1 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 0, 0);
                VLE64_V(v1, ed, VECTOR_MASKED, VECTOR_NFIELD1);
            }
            VFMUL_VV(v0, v0, v1, VECTOR_MASKED);
            break;
        default: DEFAULT_VECTOR;
    }
    return addr;
}
