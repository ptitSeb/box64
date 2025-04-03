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
    int64_t fixedaddress, gdoffset, vxoffset;
    int unscaled;

    rex_t rex = vex.rex;

    switch (opcode) {
        case 0x10:
            INST_NAME("VMOVSS Gx, [Vx,] Ex");
            nextop = F8;
            GETG;
            if (MODREG) {
                if (gd == vex.v) {
                    v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
                    q0 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 1);
                    FMVS(v0, q0);
                } else {
                    GETGX();
                    GETVX();
                    GETEX(x2, 0, 1);
                    if (rv64_xtheadmempair) {
                        ADD(x1, vback, vxoffset);
                        TH_LDD(x3, x4, x1, 0);
                    } else {
                        LD(x3, vback, vxoffset);
                        LD(x4, vback, vxoffset + 8);
                    }
                    LWU(x5, wback, fixedaddress);
                    if (rv64_xtheadmempair) {
                        ADDI(x1, gback, gdoffset);
                        TH_SDD(x3, x4, x1, 0);
                    } else {
                        SD(x3, gback, gdoffset);
                        SD(x4, gback, gdoffset + 8);
                    }
                    SW(x5, gback, gdoffset);
                }
            } else {
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd, 1);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                FLW(v0, ed, fixedaddress);
                // reset upper part
                SW(xZR, xEmu, offsetof(x64emu_t, xmm[gd]) + 4);
                SD(xZR, xEmu, offsetof(x64emu_t, xmm[gd]) + 8);
            }
            YMM0(gd);
            break;
        default:
            DEFAULT;
    }
    return addr;
}
