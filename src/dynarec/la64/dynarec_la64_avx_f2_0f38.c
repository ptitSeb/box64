#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "env.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"

#include "la64_printer.h"
#include "dynarec_la64_private.h"
#include "dynarec_la64_functions.h"
#include "../dynarec_helper.h"

uintptr_t dynarec64_AVX_F2_0F38(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

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

    switch (opcode) {
        case 0xF5:
            INST_NAME("PDEP Gd, Vd, Ed");
            nextop = F8;
            GETGD;
            GETVD;
            GETED(0);
            if (gd == ed || gd == vd) {
                gb1 = gd;
                gd = x6;
            } else {
                gb1 = 0;
            }
            MOV64x(gd, 0);
            MOV64x(x3, 1);
            MOV64x(x4, 1);
            MARK;
            AND(x5, ed, x4);
            BEQZ_MARK2(x5);
            AND(x5, vd, x3);
            BEQZ_MARK3(x5);
            OR(gd, gd, x4);
            MARK3;
            SLLIxw(x3, x3, 1);
            MARK2;
            SLLIxw(x4, x4, 1);
            BNEZ_MARK(x4);
            if (gb1)
                OR(gb1, gd, gd);
            break;
        case 0xF6:
            INST_NAME("MULX Gd, Vd, Ed (,RDX)");
            nextop = F8;
            GETGD;
            GETED(0);
            GETVD;
            if ((gd == xRDX) || (gd == ed) || (gd == vd))
                gb1 = x3;
            else
                gb1 = gd;
            if (rex.w) {
                MULH_DU(gb1, xRDX, ed);
                if (gd != vd) { MUL_D(vd, xRDX, ed); }
                if (gb1 == x3) {
                    OR(gd, gb1, gb1);
                }
            } else {
                MULH_WU(gb1, xRDX, ed);
                if (gd != vd) { MUL_W(vd, xRDX, ed); }
            }
            if (gb1 == x3) {
                BSTRINS_D(gd, gb1, 31, 0);
            }
            break;
        case 0xF7:
            INST_NAME("SHRX Gd, Ed, Vd");
            nextop = F8;
            GETGD;
            GETED(0);
            GETVD;
            ANDI(x5, vd, rex.w ? 0x3f : 0x1f);
            SRLxw(gd, ed, x5);
            break;
        default:
            DEFAULT;
    }
    return addr;
}
