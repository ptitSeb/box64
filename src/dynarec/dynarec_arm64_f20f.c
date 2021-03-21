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

#include "dynarec_arm64_functions.h"
#include "dynarec_arm64_helper.h"

// Get Ex as a double, not a quad (warning, x2 and x3 may get used)
#define GETEX(a) \
    if((nextop&0xC0)==0xC0) { \
        a = sse_get_reg(dyn, ninst, x1, nextop&7); \
    } else {    \
        parity = getedparity(dyn, ninst, addr, nextop, 3);  \
        a = fpu_get_scratch_double(dyn);            \
        if(parity) {                                \
            addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 1023, 3); \
            VLDR_64(a, ed, fixedaddress);           \
        } else {                                    \
            addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095-4, 0);\
            LDR_IMM9(x2, ed, fixedaddress+0);       \
            LDR_IMM9(x3, ed, fixedaddress+4);       \
            VMOVtoV_D(a, x2, x3);                   \
        }                                           \
    }

uintptr_t dynarec64_F20F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    uint8_t opcode = F8;
    uint8_t nextop, u8;
    int32_t i32, j32;
    uint8_t gd, ed;
    uint8_t wback, wb1;
    uint8_t eb1, eb2;
    int v0, v1;
    int q0, q1;
    int d0;
    int s0;
    int fixedaddress;
    int parity;

    MAYUSE(d0);
    MAYUSE(q1);
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(j32);

    switch(opcode) {


        default:
            DEFAULT;
    }
    return addr;
}

