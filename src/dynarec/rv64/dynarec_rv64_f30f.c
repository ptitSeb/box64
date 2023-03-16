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
#include "dynarec_rv64_functions.h"
#include "dynarec_rv64_helper.h"

uintptr_t dynarec64_F30F(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip; (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed;
    uint8_t wback;
    uint64_t u64;
    int v0, v1;
    int q0, q1;
    int d0, d1;
    int64_t fixedaddress;
    int unscaled;
    int64_t j64;

    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(v0);
    MAYUSE(v1);
    MAYUSE(j64);

    switch(opcode) {
        case 0x1E:
            INST_NAME("NOP / ENDBR32 / ENDBR64");
            nextop = F8;
            FAKEED;
            break;
        default:
            DEFAULT;
    }
    return addr;
}
