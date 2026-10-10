// SPDX-License-Identifier: MIT
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
    int s0, s1;
    uint64_t tmp64u, u64;
    int64_t j64;
    int64_t fixedaddress, gdoffset, vxoffset, vyoffset, gyoffset;
    int unscaled;

    rex_t rex = vex.rex;

    switch (opcode) {
        case 0x70:
            INST_NAME("VPSHUFHW Gx, Ex, Ib");
            nextop = F8;
            GETEX(x2, 1, vex.l ? 28 : 12);
            GETGX();
            GETGY();
            u8 = F8;
            LD(x4, wback, fixedaddress + 0);
            SD(x4, gback, gdoffset + 0);
            LHU(x3, wback, fixedaddress + 8 + ((u8 >> 0) & 3) * 2);
            LHU(x4, wback, fixedaddress + 8 + ((u8 >> 2) & 3) * 2);
            LHU(x5, wback, fixedaddress + 8 + ((u8 >> 4) & 3) * 2);
            LHU(x6, wback, fixedaddress + 8 + ((u8 >> 6) & 3) * 2);
            SH(x3, gback, gdoffset + 8);
            SH(x4, gback, gdoffset + 10);
            SH(x5, gback, gdoffset + 12);
            SH(x6, gback, gdoffset + 14);
            if (vex.l) {
                GETEY();
                LD(x4, wback, fixedaddress + 0);
                SD(x4, gback, gyoffset + 0);
                LHU(x3, wback, fixedaddress + 8 + ((u8 >> 0) & 3) * 2);
                LHU(x4, wback, fixedaddress + 8 + ((u8 >> 2) & 3) * 2);
                LHU(x5, wback, fixedaddress + 8 + ((u8 >> 4) & 3) * 2);
                LHU(x6, wback, fixedaddress + 8 + ((u8 >> 6) & 3) * 2);
                SH(x3, gback, gyoffset + 8);
                SH(x4, gback, gyoffset + 10);
                SH(x5, gback, gyoffset + 12);
                SH(x6, gback, gyoffset + 14);
            } else
                YMM0(gd);
            break;
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
            } else
                SMWRITE2();
            break;
        case 0x12:
            INST_NAME("VMOVSLDUP Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, vex.l ? 28 : 12);
            GETGY();
            LW(x3, wback, fixedaddress + 0);
            SW(x3, gback, gdoffset + 0);
            SW(x3, gback, gdoffset + 4);
            LW(x3, wback, fixedaddress + 8);
            SW(x3, gback, gdoffset + 8);
            SW(x3, gback, gdoffset + 12);
            if (vex.l) {
                GETEY();
                LW(x3, wback, fixedaddress + 0);
                SW(x3, gback, gyoffset + 0);
                SW(x3, gback, gyoffset + 4);
                LW(x3, wback, fixedaddress + 8);
                SW(x3, gback, gyoffset + 8);
                SW(x3, gback, gyoffset + 12);
            } else
                YMM0(gd);
            break;
        case 0x16:
            INST_NAME("VMOVSHDUP Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, vex.l ? 28 : 12);
            GETGY();
            LW(x3, wback, fixedaddress + 4);
            SW(x3, gback, gdoffset + 0);
            SW(x3, gback, gdoffset + 4);
            LW(x3, wback, fixedaddress + 12);
            SW(x3, gback, gdoffset + 8);
            SW(x3, gback, gdoffset + 12);
            if (vex.l) {
                GETEY();
                LW(x3, wback, fixedaddress + 4);
                SW(x3, gback, gyoffset + 0);
                SW(x3, gback, gyoffset + 4);
                LW(x3, wback, fixedaddress + 12);
                SW(x3, gback, gyoffset + 8);
                SW(x3, gback, gyoffset + 12);
            } else
                YMM0(gd);
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
        case 0x2A:
            INST_NAME("VCVTSI2SS Gx, Vx, Ed");
            nextop = F8;
            GETGX();
            GETVX();
            GETED(0);
            u8 = sse_setround(dyn, ninst, x6, x4);
            d0 = fpu_get_scratch(dyn);
            if (rex.w) {
                FCVTSL(d0, ed, RD_DYN);
            } else {
                FCVTSW(d0, ed, RD_DYN);
            }
            x87_restoreround(dyn, ninst, u8);
            if (gd != vex.v) {
                LD(x2, vback, vxoffset + 0);
                LD(x3, vback, vxoffset + 8);
                SD(x2, gback, gdoffset + 0);
                SD(x3, gback, gdoffset + 8);
            }
            FSW(d0, gback, gdoffset + 0);
            YMM0(gd);
            break;
        case 0x2C:
            INST_NAME("VCVTTSS2SI Gd, Ex");
            nextop = F8;
            GETGDd;
            d0 = fpu_get_scratch(dyn);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                sse_forget_reg(dyn, ninst, x1, ed);
                FLW(d0, xEmu, offsetof(x64emu_t, xmm[ed]));
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 0, 0);
                FLW(d0, wback, fixedaddress);
            }
            if (!BOX64ENV(dynarec_fastround)) {
                FSFLAGSI(0);
            }
            FCVTSxw(gd, d0, RD_RTZ);
            if (!rex.w) ZEROUP(gd);
            if (!BOX64ENV(dynarec_fastround)) {
                FRFLAGS(x5);
                ANDI(x5, x5, (1 << FR_NV) | (1 << FR_OF));
                CBZ_NEXT(x5);
                if (rex.w) {
                    MOV64x(gd, 0x8000000000000000LL);
                } else {
                    MOV32w(gd, 0x80000000);
                }
            }
            break;
        case 0x58:
            INST_NAME("VADDSS Gx, Vx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x1, 0, 1);
            GETVX();
            GETGY();
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            FLW(d0, vback, vxoffset);
            FLW(d1, wback, fixedaddress);
            if (!BOX64ENV(dynarec_fastnan)) {
                MOV32w(x6, 0x00400000);
                FMVS(q0, d1);
                FEQS(x3, d1, d1);
                FEQS(x4, d0, d0);
                AND(x5, x3, x4);
                BEQZ(x5, 4 + 4 * 4);
            }
            FADDS(q0, d0, d1);
            if (!BOX64ENV(dynarec_fastnan)) {
                FEQS(x5, q0, q0);
                BNEZ(x5, 4 + 6 * 4);
                FNEGS(q0, q0);
                BNEZ(x4, 4 + 4);
                FMVS(q0, d0);
                FMVXW(x5, q0);
                OR(x5, x5, x6);
                FMVWX(q0, x5);
            }
            FSW(q0, gback, gdoffset);
            if (gd != vex.v) {
                LWU(x2, vback, vxoffset + 4);
                SW(x2, gback, gdoffset + 4);
                LD(x2, vback, vxoffset + 8);
                SD(x2, gback, gdoffset + 8);
            }
            YMM0(gd);
            break;
        case 0x59:
            INST_NAME("VMULSS Gx, Vx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x1, 0, 1);
            GETVX();
            GETGY();
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            FLW(d0, vback, vxoffset);
            FLW(d1, wback, fixedaddress);
            if (!BOX64ENV(dynarec_fastnan)) {
                MOV32w(x6, 0x00400000);
                FMVS(q0, d1);
                FEQS(x3, d1, d1);
                FEQS(x4, d0, d0);
                AND(x5, x3, x4);
                BEQZ(x5, 4 + 4 * 4);
            }
            FMULS(q0, d0, d1);
            if (!BOX64ENV(dynarec_fastnan)) {
                FEQS(x5, q0, q0);
                BNEZ(x5, 4 + 6 * 4);
                FNEGS(q0, q0);
                BNEZ(x4, 4 + 4);
                FMVS(q0, d0);
                FMVXW(x5, q0);
                OR(x5, x5, x6);
                FMVWX(q0, x5);
            }
            FSW(q0, gback, gdoffset);
            if (gd != vex.v) {
                LWU(x2, vback, vxoffset + 4);
                SW(x2, gback, gdoffset + 4);
                LD(x2, vback, vxoffset + 8);
                SD(x2, gback, gdoffset + 8);
            }
            YMM0(gd);
            break;
        case 0x5B:
            INST_NAME("VCVTTPS2DQ Gx, Ex");
            nextop = F8;
            GETEX(x2, 0, vex.l ? 28 : 12);
            GETGX();
            GETGY();
            v0 = fpu_get_scratch(dyn);
            for (int i = 0; i < 4; ++i) {
                if (!BOX64ENV(dynarec_fastround)) {
                    FSFLAGSI(0);
                }
                FLW(v0, wback, fixedaddress + i * 4);
                FCVTWS(x3, v0, RD_RTZ);
                if (!BOX64ENV(dynarec_fastround)) {
                    FRFLAGS(x5);
                    ANDI(x5, x5, (1 << FR_NV) | (1 << FR_OF));
                    BEQZ(x5, 4 + 4);
                    LUI(x3, 0x80000000 >> 12);
                }
                SW(x3, gback, gdoffset + i * 4);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 4; ++i) {
                    if (!BOX64ENV(dynarec_fastround)) {
                        FSFLAGSI(0);
                    }
                    FLW(v0, wback, fixedaddress + i * 4);
                    FCVTWS(x3, v0, RD_RTZ);
                    if (!BOX64ENV(dynarec_fastround)) {
                        FRFLAGS(x5);
                        ANDI(x5, x5, (1 << FR_NV) | (1 << FR_OF));
                        BEQZ(x5, 4 + 4);
                        LUI(x3, 0x80000000 >> 12);
                    }
                    SW(x3, gback, gyoffset + i * 4);
                }
            } else
                YMM0(gd);
            break;
        case 0x5C:
            INST_NAME("VSUBSS Gx, Vx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x1, 0, 1);
            GETVX();
            GETGY();
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            FLW(d0, vback, vxoffset);
            FLW(d1, wback, fixedaddress);
            if (!BOX64ENV(dynarec_fastnan)) {
                MOV32w(x6, 0x00400000);
                FMVS(q0, d1);
                FEQS(x3, d1, d1);
                FEQS(x4, d0, d0);
                AND(x5, x3, x4);
                BEQZ(x5, 4 + 4 * 4);
            }
            FSUBS(q0, d0, d1);
            if (!BOX64ENV(dynarec_fastnan)) {
                FEQS(x5, q0, q0);
                BNEZ(x5, 4 + 6 * 4);
                FNEGS(q0, q0);
                BNEZ(x4, 4 + 4);
                FMVS(q0, d0);
                FMVXW(x5, q0);
                OR(x5, x5, x6);
                FMVWX(q0, x5);
            }
            FSW(q0, gback, gdoffset);
            if (gd != vex.v) {
                LWU(x2, vback, vxoffset + 4);
                SW(x2, gback, gdoffset + 4);
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
        case 0x6F:
            INST_NAME("VMOVDQU Gx, Ex");
            nextop = F8;
            GETEX(x2, 0, vex.l ? 24 : 8);
            GETGX();
            GETGY();
            LD(x7, wback, fixedaddress + 0);
            LD(x4, wback, fixedaddress + 8);
            SD(x7, gback, gdoffset + 0);
            SD(x4, gback, gdoffset + 8);
            if (vex.l) {
                GETEY();
                LD(x7, wback, fixedaddress + 0);
                LD(x4, wback, fixedaddress + 8);
                SD(x7, gback, gyoffset + 0);
                SD(x4, gback, gyoffset + 8);
            } else {
                YMM0(gd);
            }
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
        case 0x7F:
            INST_NAME("VMOVDQU Ex, Gx");
            nextop = F8;
            GETEX(x2, 0, vex.l ? 24 : 8);
            GETGX();
            GETGY();
            LD(x7, gback, gdoffset + 0);
            LD(x4, gback, gdoffset + 8);
            SD(x7, wback, fixedaddress + 0);
            SD(x4, wback, fixedaddress + 8);
            if (vex.l) {
                GETEY();
                LD(x7, gback, gyoffset + 0);
                LD(x4, gback, gyoffset + 8);
                SD(x7, wback, fixedaddress + 0);
                SD(x4, wback, fixedaddress + 8);
            } else if (MODREG) {
                YMM0(ed);
            }
            if (!MODREG) SMWRITE2();
            break;
        case 0xC2:
            INST_NAME("VCMPSS Gx, Vx, Ex, Ib");
            nextop = F8;
            GETEX(x1, 1, 1);
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
        case 0xE6:
            INST_NAME("VCVTDQ2PD Gx, Ex");
            nextop = F8;
            GETEX(x2, 0, vex.l ? 12 : 4);
            GETGX();
            GETGY();
            d0 = fpu_get_scratch(dyn);
            u8 = sse_setround(dyn, ninst, x6, x4);
            LW(x3, wback, fixedaddress + 0);
            LW(x4, wback, fixedaddress + 4);
            if (vex.l) {
                LW(x5, wback, fixedaddress + 8);
                LW(x7, wback, fixedaddress + 12);
            }
            FCVTDW(d0, x3, RD_DYN);
            FSD(d0, gback, gdoffset + 0);
            FCVTDW(d0, x4, RD_DYN);
            FSD(d0, gback, gdoffset + 8);
            if (vex.l) {
                FCVTDW(d0, x5, RD_DYN);
                FSD(d0, gback, gyoffset + 0);
                FCVTDW(d0, x7, RD_DYN);
                FSD(d0, gback, gyoffset + 8);
            } else
                YMM0(gd);
            x87_restoreround(dyn, ninst, u8);
            break;
        default:
            DEFAULT;
    }
    return addr;
}
