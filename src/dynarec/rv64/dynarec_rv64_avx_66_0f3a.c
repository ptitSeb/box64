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

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_functions.h"
#include "../dynarec_helper.h"

uintptr_t dynarec64_AVX_66_0F3A(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed, vd;
    uint8_t wback, wb1, wb2, gback, vback;
    uint8_t eb1, eb2, gb1, gb2;
    int32_t i32, i32_;
    int cacheupd = 0;
    int v0, v1, v2;
    int q0, q1, q2;
    int d0, d1, d2;
    int s0;
    uint64_t tmp64u, u64;
    int64_t j64;
    int64_t fixedaddress, gdoffset, vxoffset, gyoffset, vyoffset;
    int unscaled;

    rex_t rex = vex.rex;

    switch (opcode) {
        case 0x4A:
            INST_NAME("VBLENDVPS Gx, Vx, Ex, XMMImm8");
            nextop = F8;
            u8 = geted_ib(dyn, addr, ninst, nextop) >> 4;
            GETEX(x1, 1, vex.l ? 28 : 12);
            GETGX();
            GETVX();
            GETGY();
            F8;
            for (int i = 0; i < 4; ++i) {
                LW(x3, xEmu, offsetof(x64emu_t, xmm) + u8 * 16 + i * 4);
                LWU(x4, wback, fixedaddress + i * 4);
                LWU(x5, vback, vxoffset + i * 4);
                BGE(x3, xZR, 4 + 4);
                MV(x5, x4);
                MV(x3, x5);
                SW(x3, gback, gdoffset + i * 4);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 4; ++i) {
                    LW(x3, xEmu, offsetof(x64emu_t, ymm) + u8 * 16 + i * 4);
                    LWU(x4, wback, fixedaddress + i * 4);
                    LWU(x5, vback, vyoffset + i * 4);
                    BGE(x3, xZR, 4 + 4);
                    MV(x5, x4);
                    MV(x3, x5);
                    SW(x3, gback, gyoffset + i * 4);
                }
            } else {
                SD(xZR, gback, gyoffset + 0);
                SD(xZR, gback, gyoffset + 8);
            }
            break;
        default:
            DEFAULT;
    }
    return addr;
}
