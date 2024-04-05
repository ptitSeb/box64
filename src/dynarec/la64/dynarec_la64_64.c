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
#include "custommem.h"

#include "la64_printer.h"
#include "dynarec_la64_private.h"
#include "dynarec_la64_helper.h"
#include "dynarec_la64_functions.h"

uintptr_t dynarec64_64(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int seg, int* ok, int* need_epilog)
{
    (void)ip;
    (void)rep;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop;
    uint8_t u8;
    uint8_t gd, ed, eb1, eb2, gb1, gb2;
    uint8_t gback, wback, wb1, wb2, wb;
    int64_t i64, j64;
    uint64_t u64;
    int v0, v1;
    int q0;
    int d0;
    int64_t fixedaddress, gdoffset;
    int unscaled;
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(wb1);
    MAYUSE(wb2);
    MAYUSE(gb1);
    MAYUSE(gb2);
    MAYUSE(j64);
    MAYUSE(d0);
    MAYUSE(q0);
    MAYUSE(v0);
    MAYUSE(v1);

    while ((opcode == 0xF2) || (opcode == 0xF3)) {
        rep = opcode - 0xF1;
        opcode = F8;
    }

    GETREX();

    switch (opcode) {
        case 0x33:
            INST_NAME("XOR Gd, Seg:Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            grab_segdata(dyn, addr, ninst, x4, seg);
            nextop = F8;
            GETGD;
            GETEDO(x4, 0);
            emit_xor32(dyn, ninst, rex, gd, ed, x3, x4);
            break;
        case 0x8B:
            INST_NAME("MOV Gd, Seg:Ed");
            grab_segdata(dyn, addr, ninst, x4, seg);
            nextop = F8;
            GETGD;
            if (MODREG) { // reg <= reg
                MVxw(gd, TO_LA64((nextop & 7) + (rex.b << 3)));
            } else { // mem <= reg
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                ADD_D(x4, ed, x4);
                LDxw(gd, x4, fixedaddress);
            }
            break;
        default:
            DEFAULT;
    }
    return addr;
}
