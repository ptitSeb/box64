#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
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
#include "dynarec_arm64.h"
#include "dynarec_arm64_private.h"
#include "arm64_printer.h"

#include "dynarec_arm64_helper.h"
#include "dynarec_arm64_functions.h"


uintptr_t dynarec64_64(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
{
    uint8_t opcode = F8;
    uint8_t nextop;
    int32_t j32;
    uint8_t gd, ed;
    uint8_t wback;
    int fixedaddress;
    MAYUSE(j32);

    while((opcode==0xF2) || (opcode==0xF3)) {
        rep = opcode-0xF1;
        opcode = F8;
    }
    // REX prefix before the F0 are ignored
    rex.rex = 0;
    while(opcode>=0x40 && opcode<=0x4f) {
        rex.rex = opcode;
        opcode = F8;
    }

    switch(opcode) {

        case 0x33:
            INST_NAME("XOR Gd, FS:Ed");
            SETFLAGS(X_ALL, SF_SET);
            grab_segdata(dyn, addr, ninst, x4, _FS);
            nextop = F8;
            GETGD;
            GETEDO(x4, 0);
            emit_xor32(dyn, ninst, rex, gd, ed, x3, x4);
            break;
                    
        case 0x8B:
            INST_NAME("MOV Gd, FS:Ed");
            grab_segdata(dyn, addr, ninst, x4, _FS);
            nextop=F8;
            GETGD;
            if(MODREG) {   // reg <= reg
                MOVxw_REG(gd, xRAX+(nextop&7)+(rex.b<<3));
            } else {                    // mem <= reg
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0, 0, rex, 0, 0);
                LDRxw_REG(gd, ed, x4);
            }
            break;

        default:
            DEFAULT;
    }
    return addr;
}

