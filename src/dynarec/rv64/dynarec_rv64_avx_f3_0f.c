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
            YMM0(gd);
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
                YMM0(ed);
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
            if (!BOX64ENV(dynarec_fastnan)) {
                FEQS(x3, s0, s0);
                FMVXW(x4, s0);
            }
            FCVTDS(s0, s0);
            if (!BOX64ENV(dynarec_fastnan)) {
                BNEZ_MARK(x3);
                SRLIW(x5, x4, 31);
                SLLI(x5, x5, 63);
                SLLI(x4, x4, 41);
                SRLI(x4, x4, 12);
                OR(x4, x4, x5);
                MOV64x(x5, 0x7ff8000000000000ULL);
                OR(x4, x4, x5);
                FMVDX(s0, x4);
                MARK;
            }
            FSD(s0, gback, gdoffset);
            if (gd != vex.v) {
                LD(x2, vback, vxoffset + 8);
                SD(x2, gback, gdoffset + 8);
            }
            YMM0(gd);
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
            YMM0(gd);
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
            YMM0(gd);
            break;
        case 0x7E:
            INST_NAME("VMOVQ Gx, Ex");
            nextop = F8;
            GETEX(x1, 0, 1);
            GETGX();
            GETGY();
            LD(x3, wback, fixedaddress);
            SD(x3, gback, gdoffset + 0);
            SD(xZR, gback, gdoffset + 8);
            YMM0(gd);
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
            if ((u8 & 0xf) != 0x0b && (u8 & 0xf) != 0x0f) {
                // x6 = !(isnan(d0) || isnan(d1))
                FEQS(x4, d0, d0);
                FEQS(x3, d1, d1);
                AND(x6, x3, x4);
            }

            switch (u8 & 0x7) {
                case 0:
                    FEQS(x3, d0, d1);
                    break; // Equal
                case 1:
                    BEQ(x6, xZR, 8);
                    FLTS(x3, d0, d1);
                    break; // Less than
                case 2:
                    BEQ(x6, xZR, 8);
                    FLES(x3, d0, d1);
                    break; // Less or equal
                case 3:
                    if (u8 & 0x8)
                        ADDI(x3, xZR, 0);
                    else
                        XORI(x3, x6, 1);
                    break; // Unordered
                case 4:
                    FEQS(x3, d0, d1);
                    XORI(x3, x3, 1);
                    break; // Not equal or unordered
                case 5:
                    BEQ(x6, xZR, 12);
                    FLES(x3, d1, d0);
                    J(8);
                    ADDI(x3, xZR, 1);
                    break; // Greater or equal or unordered
                case 6:
                    BEQ(x6, xZR, 12);
                    FLTS(x3, d1, d0);
                    J(8);
                    ADDI(x3, xZR, 1);
                    break; // Greater or unordered
                case 7:
                    if (u8 & 0x8)
                        ADDI(x3, xZR, 1);
                    else
                        MV(x3, x6);
                    break; // Ordered
            }
            if ((u8 & 0x3) != 0x3) {
                if ((u8 & 0xC) == 0x8 || (u8 & 0xC) == 0x4) {
                    XORI(x7, x6, 1);
                    OR(x3, x3, x7);
                } else
                    AND(x3, x3, x6);
            }
            NEG(x3, x3);
            SW(x3, gback, gdoffset);
            if (gd != vex.v) {
                LWU(x2, vback, vxoffset + 4);
                SW(x2, gback, gdoffset + 4);
                LD(x2, vback, vxoffset + 8);
                SD(x2, gback, gdoffset + 8);
            }
            YMM0(gd);
            break;
        default:
            DEFAULT;
    }
    return addr;
}
