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
    int64_t fixedaddress, gdoffset;
    int unscaled;
    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(j64);
    switch (opcode) {
        case 0x1F:
            return 0;
        case 0x28:
            INST_NAME("MOVAPD Gx, Ex");
            nextop = F8;
            // FIXME
            vector_vsetvl_emul1(dyn, ninst, x1, VECTOR_SEW8);

            GETG;
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                v1 = sse_get_reg_vector(dyn, ninst, x1, ed, 0);
                v0 = sse_get_reg_empty_vector(dyn, ninst, x1, gd);
                VMV_V_V(v0, v1);
            } else {
                SMREAD();
                v0 = sse_get_reg_empty_vector(dyn, ninst, x1, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                VLE8_V(v0, ed, VECTOR_UNMASKED, VECTOR_NFIELD1);
            }
            break;
        case 0x38: // SSSE3 opcodes
            nextop = F8;
            switch (nextop) {
                case 0x00:
                    INST_NAME("PSHUFB Gx, Ex");
                    nextop = F8;
                    // FIXME
                    vector_vsetvl_emul1(dyn, ninst, x1, VECTOR_SEW8);

                    GETGX_vector(q0, 1);
                    GETEX_vector(q1, 0, 0);
                    v0 = fpu_get_scratch(dyn);
                    v1 = fpu_get_scratch(dyn);
                    ADDI(x4, xZR, 0b000010001111);
                    VMV_V_X(v0, x4); // broadcast the mask
                    VAND_VV(v0, v0, q1, VECTOR_UNMASKED);
                    VRGATHER_VV(v1, v0, q0, VECTOR_UNMASKED); // registers cannot be overlapped!!
                    VMV_V_V(q0, v1);
                    break;
                default:
                    DEFAULT_VECTOR;
            }
            break;
        case 0x6E:
            return 0;
        case 0x6F:
            INST_NAME("MOVDQA Gx, Ex");
            nextop = F8;
            // FIXME
            vector_vsetvl_emul1(dyn, ninst, x1, VECTOR_SEW8);

            if (MODREG) {
                v1 = sse_get_reg_vector(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);
                GETGX_empty_vector(v0);
                VMV_V_V(v0, v1);
            } else {
                GETGX_empty_vector(v0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 0, 0);
                VLE8_V(v0, ed, VECTOR_UNMASKED, VECTOR_NFIELD1);
            }
            break;
        case 0x7E:
            return 0;
        case 0xEF:
            INST_NAME("PXOR Gx, Ex");
            nextop = F8;
            // FIXME: we should try to minimize vsetvl usage as it may hurts performance a lot.
            vector_vsetvl_emul1(dyn, ninst, x1, VECTOR_SEW8);

            GETG;
            if (MODREG && gd == (nextop & 7) + (rex.b << 3)) {
                // special case
                q0 = sse_get_reg_empty_vector(dyn, ninst, x1, gd);
                VXOR_VV(q0, q0, q0, VECTOR_UNMASKED);
            } else {
                q0 = sse_get_reg_vector(dyn, ninst, x1, gd, 1);
                GETEX_vector(q1, 0, 0);
                VXOR_VV(q0, q0, q1, VECTOR_UNMASKED);
            }
            break;
        default:
            DEFAULT_VECTOR;
    }

    return addr;
}
