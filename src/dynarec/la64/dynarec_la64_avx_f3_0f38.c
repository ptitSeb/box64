// SPDX-License-Identifier: MIT
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

uintptr_t dynarec64_AVX_F3_0F38(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
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
            INST_NAME("PEXT Gd, Vd, Ed");
            nextop = F8;
            GETGDd;
            GETVDs;
            GETED(0);
            if (gd == ed || gd == vd) {
                gb1 = gd;
                gd = x6;
            } else {
                gb1 = 0;
            }
            MOV64x(gd, 0);
            MV(x4, ed);
            MOV64x(x7, 0);
            BEQZ_MARK2(x4);
            MARK;
            CTZxw(x3, x4);
            SRLxw(x5, vd, x3);
            ANDI(x5, x5, 1);
            SLLxw(x5, x5, x7);
            OR(gd, gd, x5);
            if (rex.w)
                ADDI_D(x5, x4, -1);
            else
                ADDI_W(x5, x4, -1);
            AND(x4, x4, x5);
            ADDI_D(x7, x7, 1);
            MARK2;
            BNEZ_MARK(x4);
            if (gb1)
                OR(gb1, gd, gd);
            break;

        case 0xF7:
            INST_NAME("SARX Gd, Ed, Vd");
            nextop = F8;
            GETGDd;
            GETED(0);
            GETVDs;
            ANDI(x5, vd, rex.w ? 0x3f : 0x1f);
            SRAxw(gd, ed, x5);
            break;
        default:
            DEFAULT;
    }
    return addr;
}
