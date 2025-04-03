#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "la64_emitter.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"

#include "la64_printer.h"
#include "dynarec_la64_private.h"
#include "../dynarec_helper.h"
#include "dynarec_la64_functions.h"

uintptr_t dynarec64_6664(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int seg, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop;
    uint8_t gd, ed;
    int64_t j64;
    int v0, v1;
    int64_t fixedaddress;
    int unscaled;
    MAYUSE(j64);

    GETREX();

    switch (opcode) {
        case 0x8B:
            INST_NAME("MOV Gd, FS:Ed");
            nextop = F8;
            GETGD;
            if (MODREG) { // reg <= reg
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                if (rex.w) {
                    MV(gd, ed);
                } else {
                    if (ed != gd) BSTRINS_D(gd, ed, 15, 0);
                }
            } else { // mem <= reg
                grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                ADDz(x4, ed, x4);
                if (rex.w) {
                    LD_D(gd, x4, fixedaddress);
                } else {
                    LD_HU(x1, x4, fixedaddress);
                    BSTRINS_D(gd, x1, 15, 0);
                }
            }
            break;

        default:
            DEFAULT;
    }
    return addr;
}
