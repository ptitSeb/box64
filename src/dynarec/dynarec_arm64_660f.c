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

// Get EX as a quad
#define GETEX(a)                \
    if(MODREG) {   \
        a = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3));  \
    } else {                    \
        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0); \
        a = fpu_get_scratch_quad(dyn); \
        VLD1Q_8(a, ed);       \
    }
#define GETGX(a)    \
    gd = ((nextop&0x38)>>3)+(rex.r<<3);  \
    a = sse_get_reg(dyn, ninst, x1, gd)

uintptr_t dynarec64_660F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
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
    #if STEP == 3
    static const int8_t mask_shift8[] = { -7, -6, -5, -4, -3, -2, -1, 0 };
    #endif

    switch(opcode) {


        case 0x1F:
            INST_NAME("NOP (multibyte)");
            nextop = F8;
            FAKEED;
            break;
        
        case 0xA3:
            INST_NAME("BT Ew, Gw");
            SETFLAGS(X_CF, SF_SET);
            nextop = F8;
            gd = xRAX+((nextop&0x38)>>3)+(rex.r<<3);    // GETGD
            GETEW(x4, 0);
            ANDw_mask(x2, gd, 0, 0b000011);  // mask=0x0f
            LSRw_REG(x1, ed, x2);
            BFIw(xFlags, x1, F_CF, 1);
            break;

        case 0xAB:
            INST_NAME("BTS Ew, Gw");
            SETFLAGS(X_CF, SF_SET);
            nextop = F8;
            gd = xRAX+((nextop&0x38)>>3)+(rex.r<<3);    // GETGD
            GETEW(x4, 0);
            ANDw_mask(x2, gd, 0, 0b000011);  // mask=0x0f
            LSRw_REG(x1, ed, x2);
            BFIw(xFlags, x1, F_CF, 1);
            ANDSw_mask(x1, x1, 0, 0);  //mask=1
            B_NEXT(cNE);
            MOV32w(x1, 1);
            LSLxw_REG(x1, x1, x2);
            EORxw_REG(ed, ed, x1);
            EWBACK;
            break;

        case 0xB3:
            INST_NAME("BTR Ew, Gw");
            SETFLAGS(X_CF, SF_SET);
            nextop = F8;
            gd = xRAX+((nextop&0x38)>>3)+(rex.r<<3);    // GETGD
            GETEW(x4, 0);
            ANDw_mask(x2, gd, 0, 0b000011);  // mask=0x0f
            LSRw_REG(x1, ed, x2);
            BFIw(xFlags, x1, F_CF, 1);
            ANDSw_mask(x1, x1, 0, 0);  //mask=1
            B_NEXT(cEQ);
            MOV32w(x1, 1);
            LSLxw_REG(x1, x1, x2);
            EORxw_REG(ed, ed, x1);
            EWBACK;
            break;

        case 0xBB:
            INST_NAME("BTC Ew, Gw");
            SETFLAGS(X_CF, SF_SET);
            nextop = F8;
            gd = xRAX+((nextop&0x38)>>3)+(rex.r<<3);    // GETGD
            GETEW(x4, 0);
            ANDw_mask(x2, gd, 0, 0b000011);  // mask=0x0f
            LSRw_REG(x1, ed, x2);
            BFIw(xFlags, x1, F_CF, 1);
            ANDw_mask(x1, x1, 0, 0);  //mask=1
            MOV32w(x1, 1);
            LSLxw_REG(x1, x1, x2);
            EORxw_REG(ed, ed, x1);
            EWBACK;
            break;

        default:
            DEFAULT;
    }
    return addr;
}

