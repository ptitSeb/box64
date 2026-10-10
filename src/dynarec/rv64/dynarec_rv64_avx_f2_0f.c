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

uintptr_t dynarec64_AVX_F2_0F(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
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
    int64_t fixedaddress, gdoffset, vxoffset, gyoffset, vyoffset;
    int unscaled;

    rex_t rex = vex.rex;

    switch (opcode) {
        case 0x70:
            INST_NAME("VPSHUFLW Gx, Ex, Ib");
            nextop = F8;
            GETEX(x2, 1, vex.l ? 28 : 12);
            GETGX();
            GETGY();
            u8 = F8;
            LHU(x3, wback, fixedaddress + ((u8 >> 0) & 3) * 2);
            LHU(x4, wback, fixedaddress + ((u8 >> 2) & 3) * 2);
            LHU(x5, wback, fixedaddress + ((u8 >> 4) & 3) * 2);
            LHU(x6, wback, fixedaddress + ((u8 >> 6) & 3) * 2);
            SH(x3, gback, gdoffset + 0);
            SH(x4, gback, gdoffset + 2);
            SH(x5, gback, gdoffset + 4);
            SH(x6, gback, gdoffset + 6);
            LD(x4, wback, fixedaddress + 8);
            SD(x4, gback, gdoffset + 8);
            if (vex.l) {
                GETEY();
                LHU(x3, wback, fixedaddress + ((u8 >> 0) & 3) * 2);
                LHU(x4, wback, fixedaddress + ((u8 >> 2) & 3) * 2);
                LHU(x5, wback, fixedaddress + ((u8 >> 4) & 3) * 2);
                LHU(x6, wback, fixedaddress + ((u8 >> 6) & 3) * 2);
                SH(x3, gback, gyoffset + 0);
                SH(x4, gback, gyoffset + 2);
                SH(x5, gback, gyoffset + 4);
                SH(x6, gback, gyoffset + 6);
                LD(x4, wback, fixedaddress + 8);
                SD(x4, gback, gyoffset + 8);
            } else
                YMM0(gd);
            break;
        case 0x10:
            INST_NAME("VMOVSD Gx, [Vx,] Ex");
            nextop = F8;
            GETEX(x1, 0, 1);
            GETGX();
            GETGY();
            LD(x3, wback, fixedaddress);
            SD(x3, gback, gdoffset);
            if (MODREG) {
                GETVX();
                LD(x3, vback, vxoffset + 8);
                SD(x3, gback, gdoffset + 8);
            } else {
                SD(xZR, gback, gdoffset + 8);
            }
            YMM0(gd);
            break;
        case 0x11:
            INST_NAME("VMOVSD Ex, [Vx,] Gx");
            nextop = F8;
            GETEX(x1, 0, 24);
            GETGX();
            GETGY();
            LD(x3, gback, gdoffset);
            SD(x3, wback, fixedaddress);
            if (MODREG) {
                GETVX();
                LD(x3, vback, vxoffset + 8);
                SD(x3, wback, fixedaddress + 8);
                YMM0(ed);
            } else
                SMWRITE2();
            break;
        case 0x12:
            INST_NAME("VMOVDDUP Gx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 16 : 1);
            GETGX();
            GETGY();
            LD(x3, wback, fixedaddress);
            SD(x3, gback, gdoffset);
            SD(x3, gback, gdoffset + 8);
            if (vex.l) {
                GETEY();
                LD(x3, wback, fixedaddress + 0);
                SD(x3, gback, gyoffset + 0);
                SD(x3, gback, gyoffset + 8);
            } else
                YMM0(gd);
            break;
        case 0x2A:
            INST_NAME("VCVTSI2SD Gx, Vx, Ed");
            nextop = F8;
            GETGX();
            GETVX();
            GETED(0);
            u8 = sse_setround(dyn, ninst, x6, x4);
            d0 = fpu_get_scratch(dyn);
            if (rex.w) {
                FCVTDL(d0, ed, RD_DYN);
            } else {
                FCVTDW(d0, ed, RD_DYN);
            }
            x87_restoreround(dyn, ninst, u8);
            if (gd != vex.v) {
                LD(x2, vback, vxoffset + 0);
                LD(x3, vback, vxoffset + 8);
                SD(x2, gback, gdoffset + 0);
                SD(x3, gback, gdoffset + 8);
            }
            FSD(d0, gback, gdoffset + 0);
            YMM0(gd);
            break;
        case 0x2C:
            INST_NAME("VCVTTSD2SI Gd, Ex");
            nextop = F8;
            GETGDd;
            d0 = fpu_get_scratch(dyn);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                sse_forget_reg(dyn, ninst, x1, ed);
                FLD(d0, xEmu, offsetof(x64emu_t, xmm[ed]));
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 0, 0);
                FLD(d0, wback, fixedaddress);
            }
            if (!BOX64ENV(dynarec_fastround)) {
                FSFLAGSI(0); // reset all bits
            }
            FCVTLDxw(gd, d0, RD_RTZ);
            if (!rex.w) ZEROUP(gd);
            if (!BOX64ENV(dynarec_fastround)) {
                FRFLAGS(x5); // get back FPSR to check the IOC bit
                ANDI(x5, x5, (1 << FR_NV) | (1 << FR_OF));
                CBZ_NEXT(x5);
                if (rex.w) {
                    MOV64x(gd, 0x8000000000000000LL);
                } else {
                    MOV32w(gd, 0x80000000);
                }
            }
            break;
        case 0x2D:
            INST_NAME("VCVTSD2SI Gd, Ex");
            nextop = F8;
            GETGDd;
            d0 = fpu_get_scratch(dyn);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                sse_forget_reg(dyn, ninst, x1, ed);
                FLD(d0, xEmu, offsetof(x64emu_t, xmm[ed]));
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 0, 0);
                FLD(d0, wback, fixedaddress);
            }
            if (!BOX64ENV(dynarec_fastround)) {
                FSFLAGSI(0); // reset all bits
            }
            u8 = sse_setround(dyn, ninst, x2, x3);
            FCVTLDxw(gd, d0, RD_DYN);
            if (!rex.w) ZEROUP(gd);
            x87_restoreround(dyn, ninst, u8);
            if (!BOX64ENV(dynarec_fastround)) {
                FRFLAGS(x5); // get back FPSR to check the IOC bit
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
            INST_NAME("VADDSD Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, 1);
            GETGX();
            GETVX();
            v0 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            FLD(v0, vback, vxoffset + 0);
            FLD(v1, wback, fixedaddress + 0);
            if (!BOX64ENV(dynarec_fastnan)) {
                FEQD(x3, v1, v1);
                FEQD(x4, v0, v0);
                AND(x5, x3, x4);
                BEQZ(x5, 4 + 4 * 4);
            }
            FADDD(v1, v1, v0);
            if (!BOX64ENV(dynarec_fastnan)) {
                FEQD(x5, v1, v1);
                BNEZ_MARK(x5);
                FNEGD(v1, v1);
                BNEZ(x4, 4 + 4);
                FMVD(v1, v0);
                // SNaN -> QNaN
                FMVXD(x5, v1);
                MOV64x(x3, 0x0008000000000000LL);
                OR(x5, x5, x3);
                FMVDX(v1, x5);
                MARK;
            }
            FMVD(v0, v1);
            FSD(v0, gback, gdoffset + 0);
            if (gd != vex.v) {
                LD(x3, vback, vxoffset + 8);
                SD(x3, gback, gdoffset + 8);
            }
            YMM0(gd);
            break;
        case 0x59:
            INST_NAME("VMULSD Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, 1);
            GETGX();
            GETVX();
            v0 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            FLD(v0, vback, vxoffset + 0);
            FLD(v1, wback, fixedaddress + 0);
            if (!BOX64ENV(dynarec_fastnan)) {
                FEQD(x3, v1, v1);
                FEQD(x4, v0, v0);
                AND(x5, x3, x4);
                BEQZ(x5, 4 + 4 * 4);
            }
            FMULD(v1, v1, v0);
            if (!BOX64ENV(dynarec_fastnan)) {
                FEQD(x5, v1, v1);
                BNEZ_MARK(x5);
                FNEGD(v1, v1);
                BNEZ(x4, 4 + 4);
                FMVD(v1, v0);
                // SNaN -> QNaN
                FMVXD(x5, v1);
                MOV64x(x3, 0x0008000000000000LL);
                OR(x5, x5, x3);
                FMVDX(v1, x5);
                MARK;
            }
            FMVD(v0, v1);
            FSD(v0, gback, gdoffset + 0);
            if (gd != vex.v) {
                LD(x3, vback, vxoffset + 8);
                SD(x3, gback, gdoffset + 8);
            }
            YMM0(gd);
            break;
        case 0x5C:
            INST_NAME("VSUBSD Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, 1);
            GETGX();
            GETVX();
            v0 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            FLD(v0, vback, vxoffset + 0);
            FLD(v1, wback, fixedaddress + 0);
            if (!BOX64ENV(dynarec_fastnan)) {
                FEQD(x3, v1, v1);
                FEQD(x4, v0, v0);
                AND(x5, x3, x4);
                BEQZ(x5, 4 + 4 * 4);
            }
            FSUBD(v1, v0, v1);
            if (!BOX64ENV(dynarec_fastnan)) {
                FEQD(x5, v1, v1);
                BNEZ_MARK(x5);
                FNEGD(v1, v1);
                BNEZ(x4, 4 + 4);
                FMVD(v1, v0);
                // SNaN -> QNaN
                FMVXD(x5, v1);
                MOV64x(x3, 0x0008000000000000LL);
                OR(x5, x5, x3);
                FMVDX(v1, x5);
                MARK;
            }
            FMVD(v0, v1);
            FSD(v0, gback, gdoffset + 0);
            if (gd != vex.v) {
                LD(x3, vback, vxoffset + 8);
                SD(x3, gback, gdoffset + 8);
            }
            YMM0(gd);
            break;
        case 0x5D:
            INST_NAME("VMINSD Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, 1);
            GETGX();
            GETVX();
            v0 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            FLD(v0, vback, vxoffset + 0);
            FLD(v1, wback, fixedaddress + 0);
            FEQD(x2, v0, v0);
            FEQD(x3, v1, v1);
            AND(x2, x2, x3);
            BEQ_MARK(x2, xZR);
            FLED(x2, v1, v0);
            BEQ_MARK2(x2, xZR);
            MARK;
            FMVD(v0, v1);
            MARK2;
            FSD(v0, gback, gdoffset + 0);
            if (gd != vex.v) {
                LD(x3, vback, vxoffset + 8);
                SD(x3, gback, gdoffset + 8);
            }
            YMM0(gd);
            break;
        case 0x5E:
            INST_NAME("VDIVSD Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, 1);
            GETGX();
            GETVX();
            v0 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            FLD(v0, vback, vxoffset + 0);
            FLD(v1, wback, fixedaddress + 0);
            if (!BOX64ENV(dynarec_fastnan)) {
                FEQD(x3, v1, v1);
                FEQD(x4, v0, v0);
                AND(x5, x3, x4);
                BEQZ(x5, 4 + 4 * 4);
            }
            FDIVD(v1, v0, v1);
            if (!BOX64ENV(dynarec_fastnan)) {
                FEQD(x5, v1, v1);
                BNEZ_MARK(x5);
                FNEGD(v1, v1);
                BNEZ(x4, 4 + 4);
                FMVD(v1, v0);
                // SNaN -> QNaN
                FMVXD(x5, v1);
                MOV64x(x3, 0x0008000000000000LL);
                OR(x5, x5, x3);
                FMVDX(v1, x5);
                MARK;
            }
            FMVD(v0, v1);
            FSD(v0, gback, gdoffset + 0);
            if (gd != vex.v) {
                LD(x3, vback, vxoffset + 8);
                SD(x3, gback, gdoffset + 8);
            }
            YMM0(gd);
            break;
        case 0x5A:
            INST_NAME("VCVTSD2SS Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, 1);
            GETGX();
            GETVX();
            v0 = fpu_get_scratch(dyn);
            FLD(v0, wback, fixedaddress);
            if (!BOX64ENV(dynarec_fastnan)) {
                FEQD(x3, v0, v0);
                LD(x4, wback, fixedaddress);
            }
            FCVTSD(v0, v0);
            if (!BOX64ENV(dynarec_fastnan)) {
                BNEZ_MARK(x3);
                SRLI(x6, x4, 63);
                SLLI(x6, x6, 31);
                SRLI(x4, x4, 29);
                MOV32w(x5, 0x007fffff);
                AND(x4, x4, x5);
                LUI(x5, 0x7fc00);
                OR(x4, x4, x5);
                OR(x4, x4, x6);
                FMVWX(v0, x4);
                MARK;
            }
            FSW(v0, gback, gdoffset);
            if (gd != vex.v) {
                LW(x3, vback, vxoffset + 4);
                SW(x3, gback, gdoffset + 4);
                LD(x3, vback, vxoffset + 8);
                SD(x3, gback, gdoffset + 8);
            }
            YMM0(gd);
            break;
        case 0x5F:
            INST_NAME("VMAXSD Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, 1);
            GETGX();
            GETVX();
            v0 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            FLD(v0, vback, vxoffset + 0);
            FLD(v1, wback, fixedaddress + 0);
            FEQD(x2, v0, v0);
            FEQD(x3, v1, v1);
            AND(x2, x2, x3);
            BEQ_MARK(x2, xZR);
            FLED(x2, v0, v1);
            BEQ_MARK2(x2, xZR);
            MARK;
            FMVD(v0, v1);
            MARK2;
            FSD(v0, gback, gdoffset + 0);
            if (gd != vex.v) {
                LD(x3, vback, vxoffset + 8);
                SD(x3, gback, gdoffset + 8);
            }
            YMM0(gd);
            break;
        case 0xC2:
            INST_NAME("VCMPSD Gx, Vx, Ex, Ib");
            nextop = F8;
            GETEX(x2, 1, 1);
            GETGX();
            GETVX();
            u8 = F8;
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            FLD(d0, vback, vxoffset);
            FLD(d1, wback, fixedaddress);

            if ((u8 & 0xf) != 0x0b && (u8 & 0xf) != 0xf) {
                // x6 = !(isnan(d0) || isnan(d1))
                FEQD(x4, d0, d0);
                FEQD(x3, d1, d1);
                AND(x6, x3, x4);
            }

            switch (u8 & 0x7) {
                case 0:
                    FEQD(x3, d0, d1);
                    break; // Equal
                case 1:
                    BEQ(x6, xZR, 8);
                    FLTD(x3, d0, d1);
                    break; // Less than
                case 2:
                    BEQ(x6, xZR, 8);
                    FLED(x3, d0, d1);
                    break; // Less or equal
                case 3:
                    if (u8 & 0x8)
                        ADDI(x3, xZR, 0);
                    else
                        XORI(x3, x6, 1);
                    break;
                case 4:
                    FEQD(x3, d0, d1);
                    XORI(x3, x3, 1);
                    break; // Not Equal or unordered
                case 5:
                    BEQ(x6, xZR, 12);
                    FLED(x3, d1, d0);
                    J(8);
                    ADDI(x3, xZR, 1);
                    break; // Greater or equal or unordered
                case 6:
                    BEQ(x6, xZR, 12);
                    FLTD(x3, d1, d0);
                    J(8);
                    ADDI(x3, xZR, 1);
                    break; // Greater or unordered
                case 7:
                    if (u8 & 0x8)
                        ADDI(x3, xZR, 1);
                    else
                        MV(x3, x6);
                    break; // Not NaN
            }
            if ((u8 & 0x3) != 0x3) {
                if ((u8 & 0xC) == 0x8 || (u8 & 0xC) == 0x4) {
                    XORI(x7, x6, 1);
                    OR(x3, x3, x7);
                } else
                    AND(x3, x3, x6);
            }
            NEG(x3, x3);
            SD(x3, gback, gdoffset);
            if (gd != vex.v) {
                LD(x3, vback, vxoffset + 8);
                SD(x3, gback, gdoffset + 8);
            }
            YMM0(gd);
            break;
        case 0xD0:
            INST_NAME("VADDSUBPS Gx, Vx, Ex");
            nextop = F8;
            GETEX(x2, 0, vex.l ? 28 : 12);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            s0 = fpu_get_scratch(dyn);
            s1 = fpu_get_scratch(dyn);
            if (!BOX64ENV(dynarec_fastnan))
                MOV32w(x6, 0x00400000);
            for (int i = 0; i < 4; ++i) {
                FLW(s0, wback, fixedaddress + i * 4);
                FLW(s1, vback, vxoffset + i * 4);
                if (!BOX64ENV(dynarec_fastnan)) {
                    FEQS(x3, s0, s0);
                    FEQS(x4, s1, s1);
                    AND(x3, x3, x4);
                    BEQZ(x3, 6 * 4);
                }
                if (i == 1 || i == 3)
                    FADDS(s0, s0, s1);
                else
                    FSUBS(s0, s1, s0);
                if (!BOX64ENV(dynarec_fastnan)) {
                    FEQS(x3, s0, s0);
                    BNEZ(x3, 8 * 4);
                    FNEGS(s0, s0);
                    J(6 * 4);
                    BNEZ(x4, 2 * 4);
                    FMVS(s0, s1);
                    FMVXW(x3, s0);
                    OR(x3, x3, x6);
                    FMVWX(s0, x3);
                }
                FSW(s0, gback, gdoffset + i * 4);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 4; ++i) {
                    FLW(s0, wback, fixedaddress + i * 4);
                    FLW(s1, vback, vyoffset + i * 4);
                    if (!BOX64ENV(dynarec_fastnan)) {
                        FEQS(x3, s0, s0);
                        FEQS(x4, s1, s1);
                        AND(x3, x3, x4);
                        BEQZ(x3, 6 * 4);
                    }
                    if (i == 1 || i == 3)
                        FADDS(s0, s0, s1);
                    else
                        FSUBS(s0, s1, s0);
                    if (!BOX64ENV(dynarec_fastnan)) {
                        FEQS(x3, s0, s0);
                        BNEZ(x3, 8 * 4);
                        FNEGS(s0, s0);
                        J(6 * 4);
                        BNEZ(x4, 2 * 4);
                        FMVS(s0, s1);
                        FMVXW(x3, s0);
                        OR(x3, x3, x6);
                        FMVWX(s0, x3);
                    }
                    FSW(s0, gback, gyoffset + i * 4);
                }
            } else
                YMM0(gd);
            break;
        case 0x7D:
            INST_NAME("VHSUBPS Gx, Vx, Ex");
            nextop = F8;
            GETEX(x2, 0, vex.l ? 24 : 8);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            s0 = fpu_get_scratch(dyn);
            s1 = fpu_get_scratch(dyn);
            if (!BOX64ENV(dynarec_fastnan)) MOV32w(x6, 0x00400000);
            if (gd == ed) {
                LD(x3, wback, fixedaddress + 0);
                LD(x4, wback, fixedaddress + 8);
                SD(x3, xEmu, offsetof(x64emu_t, scratch) + 0);
                SD(x4, xEmu, offsetof(x64emu_t, scratch) + 8);
            }
            for (int i = 0; i < 2; ++i) {
                FLW(s0, vback, vxoffset + 8 * i);
                FLW(s1, vback, vxoffset + 8 * i + 4);
                if (!BOX64ENV(dynarec_fastnan)) {
                    FEQS(x3, s0, s0);
                    FEQS(x4, s1, s1);
                    AND(x5, x3, x4);
                    BEQZ(x5, 4 * 6);
                }
                FSUBS(s0, s0, s1);
                if (!BOX64ENV(dynarec_fastnan)) {
                    FEQS(x5, s0, s0);
                    BNEZ(x5, 4 * 11);
                    FNEGS(s0, s0);
                    B(4 * 9);
                    BNEZ(x3, 4 * 5);
                    FMVXW(x5, s0);
                    OR(x5, x5, x6);
                    FMVWX(s0, x5);
                    B(4 * 4);
                    FMVXW(x5, s1);
                    OR(x5, x5, x6);
                    FMVWX(s0, x5);
                }
                FSW(s0, gback, gdoffset + 4 * i);
            }
            {
                if (gd == ed) {
                    wback = xEmu;
                    fixedaddress = offsetof(x64emu_t, scratch);
                }
                for (int i = 0; i < 2; ++i) {
                    FLW(s0, wback, fixedaddress + 8 * i);
                    FLW(s1, wback, fixedaddress + 8 * i + 4);
                    if (!BOX64ENV(dynarec_fastnan)) {
                        FEQS(x3, s0, s0);
                        FEQS(x4, s1, s1);
                        AND(x5, x3, x4);
                        BEQZ(x5, 4 * 6);
                    }
                    FSUBS(s0, s0, s1);
                    if (!BOX64ENV(dynarec_fastnan)) {
                        FEQS(x5, s0, s0);
                        BNEZ(x5, 4 * 11);
                        FNEGS(s0, s0);
                        B(4 * 9);
                        BNEZ(x3, 4 * 5);
                        FMVXW(x5, s0);
                        OR(x5, x5, x6);
                        FMVWX(s0, x5);
                        B(4 * 4);
                        FMVXW(x5, s1);
                        OR(x5, x5, x6);
                        FMVWX(s0, x5);
                    }
                    FSW(s0, gback, gdoffset + 8 + 4 * i);
                }
            }
            if (vex.l) {
                GETEY();
                if (gd == ed) {
                    LD(x3, wback, fixedaddress + 0);
                    LD(x4, wback, fixedaddress + 8);
                    SD(x3, xEmu, offsetof(x64emu_t, scratch) + 16);
                    SD(x4, xEmu, offsetof(x64emu_t, scratch) + 24);
                }
                for (int i = 0; i < 2; ++i) {
                    FLW(s0, vback, vyoffset + 8 * i);
                    FLW(s1, vback, vyoffset + 8 * i + 4);
                    if (!BOX64ENV(dynarec_fastnan)) {
                        FEQS(x3, s0, s0);
                        FEQS(x4, s1, s1);
                        AND(x5, x3, x4);
                        BEQZ(x5, 4 * 6);
                    }
                    FSUBS(s0, s0, s1);
                    if (!BOX64ENV(dynarec_fastnan)) {
                        FEQS(x5, s0, s0);
                        BNEZ(x5, 4 * 11);
                        FNEGS(s0, s0);
                        B(4 * 9);
                        BNEZ(x3, 4 * 5);
                        FMVXW(x5, s0);
                        OR(x5, x5, x6);
                        FMVWX(s0, x5);
                        B(4 * 4);
                        FMVXW(x5, s1);
                        OR(x5, x5, x6);
                        FMVWX(s0, x5);
                    }
                    FSW(s0, gback, gyoffset + 4 * i);
                }
                {
                    if (gd == ed) {
                        wback = xEmu;
                        fixedaddress = offsetof(x64emu_t, scratch) + 16;
                    }
                    for (int i = 0; i < 2; ++i) {
                        FLW(s0, wback, fixedaddress + 8 * i);
                        FLW(s1, wback, fixedaddress + 8 * i + 4);
                        if (!BOX64ENV(dynarec_fastnan)) {
                            FEQS(x3, s0, s0);
                            FEQS(x4, s1, s1);
                            AND(x5, x3, x4);
                            BEQZ(x5, 4 * 6);
                        }
                        FSUBS(s0, s0, s1);
                        if (!BOX64ENV(dynarec_fastnan)) {
                            FEQS(x5, s0, s0);
                            BNEZ(x5, 4 * 11);
                            FNEGS(s0, s0);
                            B(4 * 9);
                            BNEZ(x3, 4 * 5);
                            FMVXW(x5, s0);
                            OR(x5, x5, x6);
                            FMVWX(s0, x5);
                            B(4 * 4);
                            FMVXW(x5, s1);
                            OR(x5, x5, x6);
                            FMVWX(s0, x5);
                        }
                        FSW(s0, gback, gyoffset + 8 + 4 * i);
                    }
                }
            } else
                YMM0(gd);
            break;
        case 0x51:
            INST_NAME("VSQRTSD Gx, Vx, Ex");
            nextop = F8;
            GETGX();
            GETVX();
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                sse_forget_reg(dyn, ninst, x3, ed);
                FLD(d0, xEmu, offsetof(x64emu_t, xmm[ed]));
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                FLD(d0, wback, fixedaddress);
            }
            if (!BOX64ENV(dynarec_fastnan)) {
                v0 = fpu_get_scratch(dyn);
                FMVDX(v0, xZR);
                FLTD(x3, d0, v0);
                FEQD(x4, d0, d0);
                FMVXD(x5, d0);
            }
            FSQRTD(d1, d0);
            if (!BOX64ENV(dynarec_fastnan)) {
                BNEZ_MARK(x4);
                MOV64x(x6, 0x0008000000000000ULL);
                OR(x5, x5, x6);
                FMVDX(d1, x5);
                B_MARK2_nocond;
                MARK;
                BEQ(x3, xZR, 8);
                FNEGD(d1, d1);
                MARK2;
            }
            FSD(d1, gback, gdoffset + 0);
            LD(x3, vback, vxoffset + 8);
            SD(x3, gback, gdoffset + 8);
            YMM0(gd);
            break;
        case 0x7C:
            INST_NAME("VHADDPS Gx, Vx, Ex");
            nextop = F8;
            GETEX(x2, 0, vex.l ? 24 : 8);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            s0 = fpu_get_scratch(dyn);
            s1 = fpu_get_scratch(dyn);
            if (!BOX64ENV(dynarec_fastnan)) MOV32w(x6, 0x00400000);
            if (gd == ed) {
                LD(x3, wback, fixedaddress + 0);
                LD(x4, wback, fixedaddress + 8);
                SD(x3, xEmu, offsetof(x64emu_t, scratch) + 0);
                SD(x4, xEmu, offsetof(x64emu_t, scratch) + 8);
            }
            for (int i = 0; i < 2; ++i) {
                FLW(s0, vback, vxoffset + 8 * i);
                FLW(s1, vback, vxoffset + 8 * i + 4);
                if (!BOX64ENV(dynarec_fastnan)) {
                    FEQS(x3, s0, s0);
                    FEQS(x4, s1, s1);
                    AND(x5, x3, x4);
                    BEQZ(x5, 4 * 6);
                }
                FADDS(s0, s0, s1);
                if (!BOX64ENV(dynarec_fastnan)) {
                    FEQS(x5, s0, s0);
                    BNEZ(x5, 4 * 11);
                    FNEGS(s0, s0);
                    B(4 * 9);
                    BNEZ(x3, 4 * 5);
                    FMVXW(x5, s0);
                    OR(x5, x5, x6);
                    FMVWX(s0, x5);
                    B(4 * 4);
                    FMVXW(x5, s1);
                    OR(x5, x5, x6);
                    FMVWX(s0, x5);
                }
                FSW(s0, gback, gdoffset + 4 * i);
            }
            if (vex.v == ed) {
                LW(x3, gback, gdoffset + 0);
                LW(x4, gback, gdoffset + 4);
                SW(x3, gback, gdoffset + 8);
                SW(x4, gback, gdoffset + 12);
            } else {
                if (gd == ed) {
                    wback = xEmu;
                    fixedaddress = offsetof(x64emu_t, scratch);
                }
                for (int i = 0; i < 2; ++i) {
                    FLW(s0, wback, fixedaddress + 8 * i);
                    FLW(s1, wback, fixedaddress + 8 * i + 4);
                    if (!BOX64ENV(dynarec_fastnan)) {
                        FEQS(x3, s0, s0);
                        FEQS(x4, s1, s1);
                        AND(x5, x3, x4);
                        BEQZ(x5, 4 * 6);
                    }
                    FADDS(s0, s0, s1);
                    if (!BOX64ENV(dynarec_fastnan)) {
                        FEQS(x5, s0, s0);
                        BNEZ(x5, 4 * 11);
                        FNEGS(s0, s0);
                        B(4 * 9);
                        BNEZ(x3, 4 * 5);
                        FMVXW(x5, s0);
                        OR(x5, x5, x6);
                        FMVWX(s0, x5);
                        B(4 * 4);
                        FMVXW(x5, s1);
                        OR(x5, x5, x6);
                        FMVWX(s0, x5);
                    }
                    FSW(s0, gback, gdoffset + 8 + 4 * i);
                }
            }
            if (vex.l) {
                GETEY();
                if (gd == ed) {
                    LD(x3, wback, fixedaddress + 0);
                    LD(x4, wback, fixedaddress + 8);
                    SD(x3, xEmu, offsetof(x64emu_t, scratch) + 16);
                    SD(x4, xEmu, offsetof(x64emu_t, scratch) + 24);
                }
                for (int i = 0; i < 2; ++i) {
                    FLW(s0, vback, vyoffset + 8 * i);
                    FLW(s1, vback, vyoffset + 8 * i + 4);
                    if (!BOX64ENV(dynarec_fastnan)) {
                        FEQS(x3, s0, s0);
                        FEQS(x4, s1, s1);
                        AND(x5, x3, x4);
                        BEQZ(x5, 4 * 6);
                    }
                    FADDS(s0, s0, s1);
                    if (!BOX64ENV(dynarec_fastnan)) {
                        FEQS(x5, s0, s0);
                        BNEZ(x5, 4 * 11);
                        FNEGS(s0, s0);
                        B(4 * 9);
                        BNEZ(x3, 4 * 5);
                        FMVXW(x5, s0);
                        OR(x5, x5, x6);
                        FMVWX(s0, x5);
                        B(4 * 4);
                        FMVXW(x5, s1);
                        OR(x5, x5, x6);
                        FMVWX(s0, x5);
                    }
                    FSW(s0, gback, gyoffset + 4 * i);
                }
                if (vex.v == ed) {
                    LW(x3, gback, gyoffset + 0);
                    LW(x4, gback, gyoffset + 4);
                    SW(x3, gback, gyoffset + 8);
                    SW(x4, gback, gyoffset + 12);
                } else {
                    if (gd == ed) {
                        wback = xEmu;
                        fixedaddress = offsetof(x64emu_t, scratch) + 16;
                    }
                    for (int i = 0; i < 2; ++i) {
                        FLW(s0, wback, fixedaddress + 8 * i);
                        FLW(s1, wback, fixedaddress + 8 * i + 4);
                        if (!BOX64ENV(dynarec_fastnan)) {
                            FEQS(x3, s0, s0);
                            FEQS(x4, s1, s1);
                            AND(x5, x3, x4);
                            BEQZ(x5, 4 * 6);
                        }
                        FADDS(s0, s0, s1);
                        if (!BOX64ENV(dynarec_fastnan)) {
                            FEQS(x5, s0, s0);
                            BNEZ(x5, 4 * 11);
                            FNEGS(s0, s0);
                            B(4 * 9);
                            BNEZ(x3, 4 * 5);
                            FMVXW(x5, s0);
                            OR(x5, x5, x6);
                            FMVWX(s0, x5);
                            B(4 * 4);
                            FMVXW(x5, s1);
                            OR(x5, x5, x6);
                            FMVWX(s0, x5);
                        }
                        FSW(s0, gback, gyoffset + 8 + 4 * i);
                    }
                }
            } else
                YMM0(gd);
            break;
        case 0xE6:
            INST_NAME("VCVTPD2DQ Gx, Ex");
            nextop = F8;
            GETEX(x2, 0, vex.l ? 24 : 8);
            GETGX();
            d0 = fpu_get_scratch(dyn);
            u8 = sse_setround(dyn, ninst, x6, x4);
            for (int i = 0; i < 2; ++i) {
                FLD(d0, wback, fixedaddress + 8 * i);
                FCVTLD(x3, d0, RD_DYN);
                SEXT_W(x5, x3);
                BEQ(x5, x3, 8);
                LUI(x3, 0x80000);
                SW(x3, gback, gdoffset + 4 * i);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 2; ++i) {
                    FLD(d0, wback, fixedaddress + 8 * i);
                    FCVTLD(x3, d0, RD_DYN);
                    SEXT_W(x5, x3);
                    BEQ(x5, x3, 8);
                    LUI(x3, 0x80000);
                    SW(x3, gback, gdoffset + 8 + 4 * i);
                }
            } else {
                SW(xZR, gback, gdoffset + 8);
                SW(xZR, gback, gdoffset + 12);
            }
            x87_restoreround(dyn, ninst, u8);
            YMM0(gd);
            break;
        case 0xF0:
            INST_NAME("VLDDQU Gx, Ex");
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
        default:
            DEFAULT;
    }
    return addr;
}
