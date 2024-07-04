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
            // fallback to the scalar version
            return 0;
    }
    return addr;
}
