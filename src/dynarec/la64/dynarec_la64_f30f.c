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

#include "la64_printer.h"
#include "dynarec_la64_private.h"
#include "dynarec_la64_functions.h"
#include "dynarec_la64_helper.h"

uintptr_t dynarec64_F30F(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
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
        case 0x1E:
            INST_NAME("NOP / ENDBR32 / ENDBR64");
            nextop = F8;
            FAKEED;
            break;
        case 0x6F:
            INST_NAME("MOVDQU Gx, Ex");
            nextop = F8;
            if (MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);
                GETGX_empty(v0);
                VOR_V(v0, v1, v1);
            } else {
                GETGX_empty(v0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                VLD(v0, ed, fixedaddress);
            }
            break;
        case 0x7E:
            INST_NAME("MOVQ Gx, Ex");
            nextop = F8;
            GETGX_empty(v0);
            VXOR_V(v0, v0, v0);
            if (MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                v1 = fpu_get_scratch(dyn);
                FLD_D(v1, ed, fixedaddress);
            }
            VEXTRINS_D(v0, v1, 0); // v0[63:0] = v1[63:0]
            break;
        default:
            DEFAULT;
    }
    return addr;
}