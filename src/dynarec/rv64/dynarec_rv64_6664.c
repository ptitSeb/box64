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
#include "dynarec_rv64_helper.h"
#include "dynarec_rv64_functions.h"

#define GETG        gd = ((nextop&0x38)>>3)+(rex.r<<3)

uintptr_t dynarec64_6664(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int seg, int* ok, int* need_epilog)
{
    (void)ip; (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop;
    uint8_t gd, ed;
    int64_t j64;
    int v0, v1;
    int64_t fixedaddress;
    int unscaled;
    MAYUSE(j64);

    // REX prefix before the 66 are ignored
    rex.rex = 0;
    while(opcode>=0x40 && opcode<=0x4f) {
        rex.rex = opcode;
        opcode = F8;
    }

    switch(opcode) {
        case 0x8B:
            INST_NAME("MOV Gd, FS:Ed");
            nextop=F8;
            GETGD;
            if(MODREG) {   // reg <= reg
                ed = xRAX+(nextop&7)+(rex.b<<3);
                if(rex.w) {
                    MV(gd, ed);
                } else {
                    if(ed!=gd) {
                        LUI(x1, 0xffff0);
                        AND(gd, gd, x1);
                        ZEXTH(x1, ed);
                        OR(gd, gd, x1);
                    }
                }
            } else {                    // mem <= reg
                grab_segdata(dyn, addr, ninst, x4, seg);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                ADD(x4, ed, x4);
                if(rex.w) {
                    LD(gd, x4, fixedaddress);
                } else {
                    LHU(x1, x4, fixedaddress);
                    SRLI(gd, gd, 16);
                    SLLI(gd, gd, 16);
                    OR(gd, gd, x1);
                }
            }
            break;


        default:
            DEFAULT;
    }
    return addr;
}
