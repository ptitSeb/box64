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

// Get Ex as a double, not a quad (warning, x2 get used)
#define GETEX(a, D)                                                                                 \
    if(MODREG) {                                                                                    \
        a = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3));                                     \
    } else {                                                                                        \
        parity = getedparity(dyn, ninst, addr, nextop, 7, D);                                       \
        a = fpu_get_scratch(dyn);                                                                   \
        if(parity) {                                                                                \
            addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0xfff<<3, 7, rex, 0, D); \
            VLDR64_U12(a, ed, fixedaddress);                                                        \
        } else {                                                                                    \
            addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0xfff<<3, 7, rex, 0, D); \
            LDRx_U12(x2, ed, fixedaddress+0);                                                       \
            VMOVQDfrom(a, 0, x2);                                                                   \
        }                                                                                           \
    }

#define GETGX   gd = ((nextop&0x38)>>3)+(rex.r<<3)

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

        case 0x10:
            INST_NAME("MOVSD Gx, Ex");
            nextop = F8;
            GETGX;
            if(MODREG) {
                ed = (nextop&7)+ (rex.b<<3);
                v0 = sse_get_reg(dyn, ninst, x1, gd);
                d0 = sse_get_reg(dyn, ninst, x1, ed);
                VMOV(v0, d0);
            } else {
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0xfff<<3, 7, rex, 0, 0);
                LDRx_U12(x2, ed, fixedaddress);
                VEORQ(v0, v0, v0);
                VMOVQDfrom(v0, 0, x2);
            }
            break;
        case 0x11:
            INST_NAME("MOVSD Ex, Gx");
            nextop = F8;
            GETGX;
            v0 = sse_get_reg(dyn, ninst, x1, gd);
            if(MODREG) {
                ed = (nextop&7)+ (rex.b<<3);
                d0 = sse_get_reg(dyn, ninst, x1, ed);
                VMOV(d0, v0);
            } else {
                VMOVQDto(x2, v0, 0);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0xfff<<3, 7, rex, 0, 0);
                STRx_U12(x2, ed, fixedaddress);
            }
            break;

        case 0x58:
            INST_NAME("ADDSD Gx, Ex");
            nextop = F8;
            GETGX;
            v0 = sse_get_reg(dyn, ninst, x1, gd);
            GETEX(d0, 0);
            FADDD(v0, v0, d0);
            break;

        case 0x5C:
            INST_NAME("SUBSD Gx, Ex");
            nextop = F8;
            GETGX;
            v0 = sse_get_reg(dyn, ninst, x1, gd);
            GETEX(d0, 0);
            FSUBD(v0, v0, d0);
            break;

        default:
            DEFAULT;
    }
    return addr;
}

