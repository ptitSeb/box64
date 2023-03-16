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
#include "dynarec_native.h"

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_helper.h"
#include "dynarec_rv64_functions.h"


uintptr_t dynarec64_66(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
{
    uint8_t opcode = F8;
    uint8_t nextop, u8;
    int16_t i16;
    uint16_t u16;
    uint64_t u64;
    int32_t i32;
    int64_t j64;
    uint8_t gd, ed;
    uint8_t wback, wb1;
    int64_t fixedaddress;
    int unscaled;
    int lock;
    MAYUSE(u8);
    MAYUSE(u16);
    MAYUSE(u64);
    MAYUSE(j64);
    MAYUSE(lock);

    while((opcode==0x2E) || (opcode==0x36) || (opcode==0x66))   // ignoring CS:, SS: or multiple 0x66
        opcode = F8;

    while((opcode==0xF2) || (opcode==0xF3)) {
        rep = opcode-0xF1;
        opcode = F8;
    }
    // REX prefix before the 66 are ignored
    rex.rex = 0;
    while(opcode>=0x40 && opcode<=0x4f) {
        rex.rex = opcode;
        opcode = F8;
    }

    if(rex.w && opcode!=0x0f)   // rex.w cancels "66", but not for 66 0f type of prefix
        return dynarec64_00(dyn, addr-1, ip, ninst, rex, rep, ok, need_epilog); // addr-1, to "put back" opcode

    switch(opcode) {

        case 0x0F:
            addr = dynarec64_660F(dyn, addr, ip, ninst, rex, ok, need_epilog);
            break;

        case 0x70:
        case 0x71:
        case 0x72:
        case 0x73:
        case 0x74:
        case 0x75:
        case 0x76:
        case 0x77:
        case 0x78:
        case 0x79:
        case 0x7a:
        case 0x7b:
        case 0x7c:
        case 0x7d:
        case 0x7e:
        case 0x7f:
            // just use regular conditional jump
            return dynarec64_00(dyn, addr-1, ip, ninst, rex, rep, ok, need_epilog);

        default:
            DEFAULT;
    }
    return addr;
}
