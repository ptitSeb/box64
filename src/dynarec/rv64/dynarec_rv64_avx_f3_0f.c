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

uintptr_t dynarec64_AVX_F3_0F(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed, vd;
    uint8_t wback, wb1, wb2, gback, vback, gyback;
    uint8_t eb1, eb2, gb1, gb2;
    int32_t i32, i32_;
    int cacheupd = 0;
    int v0, v1, v2;
    int q0, q1, q2;
    int d0, d1, d2;
    int s0;
    uint64_t tmp64u, u64;
    int64_t j64;
    int64_t fixedaddress, gdoffset, vxoffset, gyoffset;
    int unscaled;

    rex_t rex = vex.rex;

    switch (opcode) {
        case 0x10:
            INST_NAME("VMOVSS Gx, [Vx,] Ex");
            nextop = F8;
            GETEX(x2, 0, 1);
            GETGX();
            LWU(x3, wback, fixedaddress);
            SW(x3, gback, gdoffset);
            if (MODREG) {
                GETVX();
                if (gd != vex.v) {
                    LWU(x3, vback, vxoffset + 4);
                    SW(x3, gback, gdoffset + 4);
                    LD(x3, vback, vxoffset + 8);
                    SD(x3, gback, gdoffset + 8);
                }
            } else {
                SW(xZR, gback, gdoffset + 4);
                SD(xZR, gback, gdoffset + 8);
            }
            GETGY();
            SD(xZR, gyback, gyoffset);
            SD(xZR, gyback, gyoffset + 8);
            break;
        case 0x11:
            INST_NAME("VMOVSS Ex, [Vx,] Gx");
            nextop = F8;
            GETEX(x2, 0, 1);
            GETGX();
            LWU(x3, gback, gdoffset);
            SW(x3, wback, fixedaddress);
            if (MODREG) {
                GETVX();
                LWU(x3, vback, vxoffset + 4);
                SW(x3, wback, fixedaddress + 4);
                LD(x3, vback, vxoffset + 8);
                SD(x3, wback, fixedaddress + 8);
                GETEY();
                SD(xZR, wback, fixedaddress);
                SD(xZR, wback, fixedaddress + 8);
            }
            break;
        default:
            DEFAULT;
    }
    return addr;
}
