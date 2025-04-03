#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"
#include "my_cpuid.h"
#include "emu/x87emu_private.h"
#include "emu/x64shaext.h"

#include "arm64_printer.h"
#include "dynarec_arm64_private.h"
#include "dynarec_arm64_functions.h"
#include "../dynarec_helper.h"

uintptr_t dynarec64_AVX_F2_0F38(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
{
    (void)ip; (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed, vd;
    uint8_t wback, wb1, wb2;
    uint8_t eb1, eb2, gb1, gb2;
    int32_t i32, i32_;
    int cacheupd = 0;
    int v0, v1, v2;
    int q0, q1, q2;
    int d0, d1, d2;
    int s0;
    uint64_t tmp64u;
    int64_t j64;
    int64_t fixedaddress;
    int unscaled;
    MAYUSE(wb1);
    MAYUSE(wb2);
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(gb1);
    MAYUSE(gb2);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(s0);
    MAYUSE(j64);
    MAYUSE(cacheupd);

    rex_t rex = vex.rex;

    switch(opcode) {
        case 0xF5:
            INST_NAME("PDEP Gd, Ed, Vd");
            nextop = F8;
            GETGD;
            GETED(0);
            GETVD;
            if(gd==ed || gd==vd) {
                gb1 = gd;
                gd = x4;
            } else {
                gb1 = 0;
            }
            // x3 = mask of mask, loop while not 0
            MOV32w(gd, 0);
            MOV64x(x2, 1);
            MOV64x(x3, 1);
            MARK;
            TSTxw_REG(ed, x3);
            B_MARK2(cEQ);
            TSTxw_REG(vd, x2);
            B_MARK3(cEQ);
            ORRxw_REG(gd, gd, x3);
            MARK3;
            LSLxw_IMM(x2, x2, 1);
            MARK2;
            LSLxw_IMM(x3, x3, 1);
            CBNZxw_MARK(x3);
            if(gb1)
                MOVxw_REG(gb1, gd);
            break;
        case 0xF6:
            INST_NAME("MULX Gd, Vd, Ed (,RDX)");
            nextop = F8;
            GETGD;
            GETED(0);
            GETVD;
            if(rex.w) {
                // 64bits mul
                if((gd==xRDX) || (gd==ed) || (gd==vd))
                    gb1 = x3;
                else
                    gb1 = gd;
                UMULH(gb1, xRDX, ed);
                if(gd!=vd) {MULx(vd, xRDX, ed);}
                if(gb1==x3) {
                    MOVx_REG(gd, gb1);
                }
            } else {
                // 32bits mul
                UMULL(x3, xRDX, ed);
                if(gd!=vd) {MOVw_REG(vd, x3);}
                LSRx(gd, x3, 32);
            }
            break;
        case 0xF7:
            INST_NAME("SHRX Gd, Ed, Vd");
            nextop = F8;
            GETGD;
            GETED(0);
            GETVD;
            ANDx_mask(x3, vd, 1, 0, rex.w?5:4); // mask 0x3f/0x1f
            LSRxw_REG(gd, ed, x3);
            break;

        default:
            DEFAULT;
    }
    return addr;
}
