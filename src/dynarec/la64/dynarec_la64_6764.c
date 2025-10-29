#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <assert.h>

#include "la64_mapping.h"
#include "x64_signals.h"
#include "os.h"
#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "la64_emitter.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "bridge.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"
#include "custommem.h"
#include "alternate.h"

#include "la64_printer.h"
#include "dynarec_la64_private.h"
#include "dynarec_la64_functions.h"
#include "../dynarec_helper.h"

uintptr_t dynarec64_6764(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int seg, int* ok, int* need_epilog)
{
    (void)ip; (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop;
    uint8_t gd, ed, wback, wb, wb1, wb2, gb1, gb2, eb1, eb2;
    int64_t fixedaddress;
    int unscaled;
    int8_t  i8;
    uint8_t u8;
    int32_t i32;
    int64_t j64, i64;
    int cacheupd = 0;
    int lock;
    int v0, v1, s0;
    MAYUSE(i32);
    MAYUSE(j64);
    MAYUSE(v0);
    MAYUSE(v1);
    MAYUSE(s0);
    MAYUSE(lock);
    MAYUSE(cacheupd);

    rep = 0;
    while ((opcode == 0xF2) || (opcode == 0xF3)) {
        rep = opcode - 0xF1;
        opcode = F8;
    }

    rex.rex = 0;
    while (opcode >= 0x40 && opcode <= 0x4f) {
        rex.rex = opcode;
        opcode = F8;
    }

    switch(opcode) {

        case 0x89:
            INST_NAME("MOV Seg:Ed, Gd");
            nextop = F8;
            GETGD;
            if (MODREG) { // reg <= reg
                MVxw(TO_NAT((nextop & 7) + (rex.b << 3)), gd);
            } else {      // mem <= reg
                grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                SDXxw(gd, x4, ed);
            }
            break;

        case 0x8B:
            INST_NAME("MOV Gd, Seg:Ed");
            nextop = F8;
            GETGD;
            if (MODREG) { // reg => reg
                MVxw(gd, TO_NAT((nextop & 7) + (rex.b << 3)));
            } else {      // mem => reg
                grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                LDXxw(gd, x4, ed);
            }
            break;

        default:
            DEFAULT;
    }
    return addr;
}
