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

uintptr_t dynarec64_AVX_66_0F(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
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
    int64_t fixedaddress, gdoffset, vxoffset, gyoffset;
    int unscaled;

    rex_t rex = vex.rex;

    switch (opcode) {
        case 0x28:
            INST_NAME("VMOVAPD Gx, Ex");
            nextop = F8;
            GETEX(x2, 0, vex.l ? 24 : 8);
            GETGX();
            GETGY();
            LD(x3, wback, fixedaddress);
            SD(x3, gback, gdoffset);
            LD(x3, wback, fixedaddress + 8);
            SD(x3, gback, gdoffset + 8);
            if (vex.l) {
                GETEY();
                LD(x3, wback, fixedaddress);
                SD(x3, gback, gyoffset);
                LD(x3, wback, fixedaddress + 8);
                SD(x3, gback, gyoffset + 8);
            } else {
                SD(xZR, gback, gyoffset);
                SD(xZR, gback, gyoffset + 8);
            }
            break;
        case 0x6E:
            INST_NAME("VMOVD Gx, Ed");
            nextop = F8;
            GETED(0);
            GETGX();
            GETGY();
            SD(ed, gback, gdoffset);
            SD(xZR, gback, gdoffset + 8);
            SD(xZR, gback, gyoffset);
            SD(xZR, gback, gyoffset + 8);
            break;
        case 0x6F:
            INST_NAME("VMOVDQA Gx, Ex");
            nextop = F8;
            GETEX(x2, 0, vex.l ? 24 : 8);
            GETGX();
            GETGY();
            LD(x3, wback, fixedaddress);
            SD(x3, gback, gdoffset);
            LD(x3, wback, fixedaddress + 8);
            SD(x3, gback, gdoffset + 8);
            if (vex.l) {
                GETEY();
                LD(x3, wback, fixedaddress);
                SD(x3, gback, gyoffset);
                LD(x3, wback, fixedaddress + 8);
                SD(x3, gback, gyoffset + 8);
            } else {
                SD(xZR, gback, gyoffset);
                SD(xZR, gback, gyoffset + 8);
            }
            break;
        case 0x7E:
            INST_NAME("VMOVD Ed, Gx");
            nextop = F8;
            GETGX();
            ed = TO_NAT((nextop & 7) + (rex.b << 3));
            if (rex.w) {
                if (MODREG) {
                    LD(ed, gback, gdoffset);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                    LD(x3, gback, gdoffset);
                    SD(x3, wback, fixedaddress);
                    SMWRITE2();
                }
            } else {
                if (MODREG) {
                    LWU(ed, gback, gdoffset);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                    LWU(x3, gback, gdoffset);
                    SW(x3, wback, fixedaddress);
                    SMWRITE2();
                }
            }
            break;
        case 0x7F:
            INST_NAME("VMOVDQA Ex, Gx");
            nextop = F8;
            GETEX(x2, 0, vex.l ? 24 : 8);
            GETGX();
            GETGY();
            LD(x3, gback, gdoffset);
            SD(x3, wback, fixedaddress);
            LD(x3, gback, gdoffset + 8);
            SD(x3, wback, fixedaddress + 8);
            if (vex.l) {
                GETEY();
                LD(x3, gback, gyoffset);
                SD(x3, wback, fixedaddress);
                LD(x3, gback, gyoffset + 8);
                SD(x3, wback, fixedaddress + 8);
            } else if (MODREG) {
                SD(xZR, wback, fixedaddress);
                SD(xZR, wback, fixedaddress + 8);
            }
            break;
        default:
            DEFAULT;
    }
    return addr;
}
