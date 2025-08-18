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
            SD(xZR, gback, gyoffset);
            SD(xZR, gback, gyoffset + 8);
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
        case 0x5A:
            INST_NAME("VCVTSS2SD Gx, Vx, Ex");
            nextop = F8;
            GETEX(x2, 0, 1);
            GETGX();
            GETVX();
            GETGY();
            s0 = fpu_get_scratch(dyn);
            FLW(s0, wback, fixedaddress);
            FCVTDS(s0, s0);
            FSD(s0, gback, gdoffset);
            if (gd != vex.v) {
                LD(x2, vback, vxoffset + 8);
                SD(x2, gback, gdoffset + 8);
            }
            SD(xZR, gback, gyoffset);
            SD(xZR, gback, gyoffset + 8);
            break;
        case 0x5D:
            INST_NAME("VMINSS Gx, Vx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x1, 0, 1);
            GETVX();
            GETGY();
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            FLW(d0, vback, vxoffset);
            FLW(d1, wback, fixedaddress);
            FEQS(x2, d0, d0);
            FEQS(x3, d1, d1);
            AND(x2, x2, x3);
            BEQ_MARK(x2, xZR);
            FLES(x2, d1, d0);
            BEQ_MARK2(x2, xZR);
            MARK;
            FMVS(d0, d1);
            MARK2;
            FSW(d0, gback, gdoffset);
            if (gd != vex.v) {
                LWU(x2, vback, vxoffset + 4);
                SW(x2, gback, gdoffset + 4);
                LD(x2, vback, vxoffset + 8);
                SD(x2, gback, gdoffset + 8);
            }
            SD(xZR, gback, gyoffset);
            SD(xZR, gback, gyoffset + 8);
            break;
        case 0x5F:
            INST_NAME("VMAXSS Gx, Vx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x1, 0, 1);
            GETVX();
            GETGY();
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            FLW(d0, vback, vxoffset);
            FLW(d1, wback, fixedaddress);
            FEQS(x2, d0, d0);
            FEQS(x3, d1, d1);
            AND(x2, x2, x3);
            BEQ_MARK(x2, xZR);
            FLES(x2, d0, d1);
            BEQ_MARK2(x2, xZR);
            MARK;
            FMVS(d0, d1);
            MARK2;
            FSW(d0, gback, gdoffset);
            if (gd != vex.v) {
                LWU(x2, vback, vxoffset + 4);
                SW(x2, gback, gdoffset + 4);
                LD(x2, vback, vxoffset + 8);
                SD(x2, gback, gdoffset + 8);
            }
            SD(xZR, gback, gyoffset);
            SD(xZR, gback, gyoffset + 8);
            break;
        case 0xC2:
            INST_NAME("VCMPSS Gx, Vx, Ex, Ib");
            nextop = F8;
            GETEX(x1, 0, 1);
            GETGX();
            GETVX();
            GETGY();
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            FLW(d0, vback, vxoffset);
            FLW(d1, wback, fixedaddress);
            u8 = F8;
            if ((u8 & 7) == 0) { // Equal
                FEQS(x2, d0, d1);
            } else if ((u8 & 7) == 4) { // Not Equal or unordered
                FEQS(x2, d0, d1);
                XORI(x2, x2, 1);
            } else {
                // x2 = !(isnan(d0) || isnan(d1))
                FEQS(x3, d0, d0);
                FEQS(x2, d1, d1);
                AND(x2, x2, x3);

                switch (u8 & 7) {
                    case 1:
                        BEQ_MARK(x2, xZR);
                        FLTS(x2, d0, d1);
                        break; // Less than
                    case 2:
                        BEQ_MARK(x2, xZR);
                        FLES(x2, d0, d1);
                        break;                      // Less or equal
                    case 3: XORI(x2, x2, 1); break; // NaN
                    case 5: {                       // Greater or equal or unordered
                        BEQ_MARK2(x2, xZR);
                        FLES(x2, d1, d0);
                        B_MARK_nocond;
                        break;
                    }
                    case 6: { // Greater or unordered, test inverted, N!=V so unordered or less than (inverted)
                        BEQ_MARK2(x2, xZR);
                        FLTS(x2, d1, d0);
                        B_MARK_nocond;
                        break;
                    }
                    case 7: break; // Not NaN
                }

                MARK2;
                if ((u8 & 7) == 5 || (u8 & 7) == 6) {
                    MOV32w(x2, 1);
                }
                MARK;
            }
            NEG(x2, x2);
            SW(x2, gback, gdoffset);
            if (gd != vex.v) {
                LWU(x2, vback, vxoffset + 4);
                SW(x2, gback, gdoffset + 4);
                LD(x2, vback, vxoffset + 8);
                SD(x2, gback, gdoffset + 8);
            }
            SD(xZR, gback, gyoffset);
            SD(xZR, gback, gyoffset + 8);
            break;
        default:
            DEFAULT;
    }
    return addr;
}
