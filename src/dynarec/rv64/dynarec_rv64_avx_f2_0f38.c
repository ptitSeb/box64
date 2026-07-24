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

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_functions.h"
#include "../dynarec_helper.h"

uintptr_t dynarec64_AVX_F2_0F38(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
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
            ADDI(gd, xZR, 0);
            ADDI(x3, xZR, 1);
            ADDI(x4, xZR, 1);
            MARK;
            AND(x5, ed, x4);
            BEQZ_MARK2(x5);
            AND(x5, vd, x3);
            if (cpuext.zicond) {
                CZERO_EQZ(x5, x4, x5);
                OR(gd, gd, x5);
            } else {
                BEQZ_MARK3(x5);
                OR(gd, gd, x4);
            }
            MARK3;
            SLLI(x3, x3, 1);
            MARK2;
            SLLI(x4, x4, 1);
            BNEZ_MARK(x4);
            if (gb1)
                MV(gb1, gd);
            break;
        case 0xF6:
            INST_NAME("MULX Gd, Vd, Ed (,RDX)");
            nextop = F8;
            GETGD;
            GETED(0);
            GETVD;
            if (rex.w) {
                if ((gd == xRDX) || (gd == ed) || (gd == vd))
                    gb1 = x3;
                else
                    gb1 = gd;
                MULHU(gb1, xRDX, ed);
                if (gd != vd) { MUL(vd, xRDX, ed); }
                if (gb1 == x3) {
                    MV(gd, gb1);
                }
            } else {
                ZEXTW2(x4, xRDX);
                ZEXTW2(x5, ed);
                MUL(x3, x4, x5);
                if (gd != vd) { ZEXTW2(vd, x3); }
                SRLI(gd, x3, 32);
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
