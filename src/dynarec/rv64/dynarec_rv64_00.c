#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <signal.h>
#include <assert.h>

#include "debug.h"
#include "box64context.h"
#include "dynarec.h"
#include "emu/x64emu_private.h"
#include "emu/x64run_private.h"
#include "x64run.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "bridge.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"
#include "custommem.h"

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_functions.h"
#include "dynarec_rv64_helper.h"

uintptr_t dynarec64_00(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
{
    uint8_t opcode;

    opcode = PK(0);
    switch(opcode) {
        case 0x00 ... 0x3f: addr = dynarec64_00_0(dyn, addr, ip, ninst, rex, rep, ok, need_epilog); break;
        case 0x40 ... 0x7f: addr = dynarec64_00_1(dyn, addr, ip, ninst, rex, rep, ok, need_epilog); break;
        case 0x80 ... 0xbf: addr = dynarec64_00_2(dyn, addr, ip, ninst, rex, rep, ok, need_epilog); break;
        case 0xc0 ... 0xff: addr = dynarec64_00_3(dyn, addr, ip, ninst, rex, rep, ok, need_epilog); break;
    }

    return addr;
}
