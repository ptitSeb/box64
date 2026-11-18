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
#include "bitutils.h"
#include "freq.h"
#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_functions.h"
#include "elfloader.h"
#include "../dynarec_helper.h"


uintptr_t dynarec64_0F_1(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed;
    uint8_t wb1, wback, wb2, gback;
    uint8_t eb1, eb2;
    uint8_t gb1, gb2;
    uint8_t tmp1, tmp2, tmp3;
    int32_t i32, i32_;
    int cacheupd = 0;
    int v0, v1;
    int q0, q1;
    int d0, d1;
    int s0, s1;
    uint64_t tmp64u;
    int64_t j64;
    int64_t fixedaddress, gdoffset;
    int unscaled;
    MAYUSE(wb2);
    MAYUSE(gback);
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(s0);
    MAYUSE(j64);
    MAYUSE(cacheupd);
    switch (opcode) {
        case 0x50:
            INST_NAME("MOVMSKPS Gd, Ex");
            nextop = F8;
            GETGDd;
            GETEX(x1, 0, 12);
            XOR(gd, gd, gd);
            for (int i = 0; i < 4; ++i) {
                LWU(x2, wback, fixedaddress + i * 4);
                SRLI(x2, x2, 31 - i);
                if (i > 0) ANDI(x2, x2, 1 << i);
                OR(gd, gd, x2);
            }
            break;
        case 0x51:
            INST_NAME("SQRTPS Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 12);
            d0 = fpu_get_scratch(dyn);
            if (!BOX64ENV(dynarec_fastnan)) MOV32w(x6, 0x00400000);
            for (int i = 0; i < 4; ++i) {
                FLW(d0, wback, fixedaddress + 4 * i);
                if (!BOX64ENV(dynarec_fastnan)) {
                    FMVXW(x4, d0);
                    FEQS(x3, d0, d0);
                }
                FSQRTS(d0, d0);
                if (!BOX64ENV(dynarec_fastnan)) {
                    FEQS(x5, d0, d0);
                    BNEZ(x5, 24);
                    BNEZ(x3, 16);
                    OR(x4, x4, x6);
                    FMVWX(d0, x4);
                    J(8);
                    FNEGS(d0, d0);
                }
                FSW(d0, gback, gdoffset + i * 4);
            }
            break;
        case 0x52:
            INST_NAME("RSQRTPS Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 12);
            s0 = fpu_get_scratch(dyn);
            s1 = fpu_get_scratch(dyn); // 1.0f
            v0 = fpu_get_scratch(dyn); // 0.0f
            MOV32w(x3, 0x3F800000);
            FMVWX(s1, x3); // 1.0f
            if (!BOX64ENV(dynarec_fastnan)) {
                FCVTSW(v0, xZR, RD_DYN);
                MOV32w(x6, 0x00400000);
            }
            for (int i = 0; i < 4; ++i) {
                FLW(s0, wback, fixedaddress + i * 4);
                if (!BOX64ENV(dynarec_fastnan)) {
                    FEQS(x3, s0, s0);
                    BNEZ(x3, 6 * 4);
                    FMVXW(x4, s0);
                    OR(x4, x4, x6);
                    FMVWX(s0, x4);
                    FSW(s0, gback, gdoffset + i * 4);
                    J(14 * 4);        // continue
                    FLTS(x3, v0, s0); // s0 > 0.0f?
                    BNEZ(x3, 6 * 4);
                    FEQS(x3, v0, s0); // s0 == 0.0f? (also true for -0.0f)
                    BEQZ(x3, 4 * 4);
                    FDIVS(s0, s1, s0); // generate an inf with the correct sign
                    FSW(s0, gback, gdoffset + i * 4);
                    J(7 * 4); // continue
                }
                FSQRTS(s0, s0);
                FDIVS(s0, s1, s0);
                if (!BOX64ENV(dynarec_fastnan)) {
                    FEQS(x3, s0, s0);
                    BNEZ(x3, 2 * 4); // isnan(s0)? negate it
                    FNEGS(s0, s0);
                }
                FSW(s0, gback, gdoffset + i * 4);
            }
            break;
        case 0x53:
            INST_NAME("RCPPS Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 12);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            MOV32w(x3, 0x3F800000);
            FMVWX(d0, x3); // 1.0f
            if (!BOX64ENV(dynarec_fastnan)) MOV32w(x6, 0x00400000);
            for (int i = 0; i < 4; ++i) {
                FLW(d1, wback, fixedaddress + 4 * i);
                if (!BOX64ENV(dynarec_fastnan)) {
                    FEQS(x3, d1, d1);
                    BNEZ(x3, 6 * 4); // isnan(d1)? quiet it
                    FMVXW(x4, d1);
                    OR(x4, x4, x6);
                    FMVWX(d1, x4);
                    FSW(d1, gback, gdoffset + i * 4);
                    J(5 * 4); // continue
                }
                FDIVS(d1, d0, d1);
                if (!BOX64ENV(dynarec_fastnan)) {
                    FEQS(x3, d1, d1);
                    BNEZ(x3, 2 * 4); // isnan(d1)? negate it
                    FNEGS(d1, d1);
                }
                FSW(d1, gback, gdoffset + 4 * i);
            }
            break;
        case 0x54:
            INST_NAME("ANDPS Gx, Ex");
            nextop = F8;
            gd = ((nextop & 0x38) >> 3) + (rex.r << 3);
            if (!(MODREG && gd == (nextop & 7) + (rex.b << 3))) {
                GETGX();
                GETEX(x2, 0, 8);
                SSE_LOOP_Q(x3, x4, AND(x3, x3, x4));
            }
            break;
        case 0x55:
            INST_NAME("ANDNPS Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 8);
            SSE_LOOP_Q(x3, x4, NOT(x3, x3); AND(x3, x3, x4));
            break;
        case 0x56:
            INST_NAME("ORPS Gx, Ex");
            nextop = F8;
            gd = ((nextop & 0x38) >> 3) + (rex.r << 3);
            if (!(MODREG && gd == (nextop & 7) + (rex.b << 3))) {
                GETGX();
                GETEX(x2, 0, 8);
                SSE_LOOP_Q(x3, x4, OR(x3, x3, x4));
            }
            break;
        case 0x57:
            INST_NAME("XORPS Gx, Ex");
            nextop = F8;
            GETGX();
            if (MODREG && gd == (nextop & 7) + (rex.b << 3)) {
                // just zero dest
                SD(xZR, gback, gdoffset + 0);
                SD(xZR, gback, gdoffset + 8);
            } else {
                GETEX(x2, 0, 8);
                SSE_LOOP_Q(x3, x4, XOR(x3, x3, x4));
            }
            break;
        case 0x58:
            INST_NAME("ADDPS Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 12);
            s0 = fpu_get_scratch(dyn);
            s1 = fpu_get_scratch(dyn);
            if (!BOX64ENV(dynarec_fastnan)) MOV32w(x6, 0x00400000);
            for (int i = 0; i < 4; ++i) {
                FLW(s0, wback, fixedaddress + i * 4);
                FLW(s1, gback, gdoffset + i * 4);
                if (!BOX64ENV(dynarec_fastnan)) {
                    FEQS(x3, s0, s0);
                    FEQS(x4, s1, s1);
                    AND(x5, x3, x4);
                    BEQZ(x5, 4 + 4 * 4);
                }
                FADDS(s0, s0, s1);
                if (!BOX64ENV(dynarec_fastnan)) {
                    FEQS(x5, s0, s0);
                    BNEZ(x5, 4 + 6 * 4);
                    FNEGS(s0, s0);
                    BNEZ(x4, 4 + 4);
                    FMVS(s0, s1);
                    FMVXW(x5, s0);
                    OR(x5, x5, x6);
                    FMVWX(s0, x5);
                }
                FSW(s0, gback, gdoffset + i * 4);
            }
            break;
        case 0x59:
            INST_NAME("MULPS Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 12);
            s0 = fpu_get_scratch(dyn);
            s1 = fpu_get_scratch(dyn);
            if (!BOX64ENV(dynarec_fastnan)) MOV32w(x6, 0x00400000);
            for (int i = 0; i < 4; ++i) {
                // GX->f[i] *= EX->f[i];
                FLW(s0, wback, fixedaddress + i * 4);
                FLW(s1, gback, gdoffset + i * 4);
                if (!BOX64ENV(dynarec_fastnan)) {
                    FEQS(x3, s0, s0);
                    FEQS(x4, s1, s1);
                    AND(x5, x3, x4);
                    BEQZ(x5, 4 + 4 * 4);
                }
                FMULS(s0, s0, s1);
                if (!BOX64ENV(dynarec_fastnan)) {
                    FEQS(x5, s0, s0);
                    BNEZ(x5, 4 + 6 * 4);
                    FNEGS(s0, s0);
                    BNEZ(x4, 4 + 4);
                    FMVS(s0, s1);
                    FMVXW(x5, s0);
                    OR(x5, x5, x6);
                    FMVWX(s0, x5);
                }
                FSW(s0, gback, gdoffset + i * 4);
            }
            break;
        case 0x5A:
            INST_NAME("CVTPS2PD Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 4);
            s0 = fpu_get_scratch(dyn);
            s1 = fpu_get_scratch(dyn);
            FLW(s0, wback, fixedaddress);
            FLW(s1, wback, fixedaddress + 4);
            if (!BOX64ENV(dynarec_fastnan)) {
                MOV64x(x6, 0x7ff8000000000000ULL);
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
                OR(x4, x4, x6);
                FMVDX(s0, x4);
                MARK;
                FEQS(x3, s1, s1);
                FMVXW(x4, s1);
            }
            FCVTDS(s1, s1);
            if (!BOX64ENV(dynarec_fastnan)) {
                BNEZ_MARK2(x3);
                SRLIW(x5, x4, 31);
                SLLI(x5, x5, 63);
                SLLI(x4, x4, 41);
                SRLI(x4, x4, 12);
                OR(x4, x4, x5);
                OR(x4, x4, x6);
                FMVDX(s1, x4);
                MARK2;
            }
            FSD(s0, gback, gdoffset + 0);
            FSD(s1, gback, gdoffset + 8);
            break;
        case 0x5B:
            INST_NAME("CVTDQ2PS Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 12);
            s0 = fpu_get_scratch(dyn);
            for (int i = 0; i < 4; ++i) {
                LW(x3, wback, fixedaddress + i * 4);
                FCVTSW(s0, x3, RD_DYN);
                FSW(s0, gback, gdoffset + i * 4);
            }
            break;
        case 0x5C:
            INST_NAME("SUBPS Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 12);
            s0 = fpu_get_scratch(dyn);
            s1 = fpu_get_scratch(dyn);
            if (!BOX64ENV(dynarec_fastnan)) MOV32w(x6, 0x00400000);
            for (int i = 0; i < 4; ++i) {
                // GX->f[i] -= EX->f[i];
                FLW(s0, wback, fixedaddress + i * 4);
                FLW(s1, gback, gdoffset + i * 4);
                if (!BOX64ENV(dynarec_fastnan)) {
                    FEQS(x3, s0, s0);
                    FEQS(x4, s1, s1);
                    AND(x5, x3, x4);
                    BEQZ(x5, 4 + 4 * 4);
                }
                FSUBS(s0, s1, s0);
                if (!BOX64ENV(dynarec_fastnan)) {
                    FEQS(x5, s0, s0);
                    BNEZ(x5, 4 + 6 * 4);
                    FNEGS(s0, s0);
                    BNEZ(x4, 4 + 4);
                    FMVS(s0, s1);
                    FMVXW(x5, s0);
                    OR(x5, x5, x6);
                    FMVWX(s0, x5);
                }
                FSW(s0, gback, gdoffset + i * 4);
            }
            break;
        case 0x5D:
            INST_NAME("MINPS Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 12);
            s0 = fpu_get_scratch(dyn);
            s1 = fpu_get_scratch(dyn);
            for (int i = 0; i < 4; ++i) {
                FLW(s0, wback, fixedaddress + i * 4);
                FLW(s1, gback, gdoffset + i * 4);
                FLTS(x3, s1, s0);
                BNEZ(x3, 8);
                FSW(s0, gback, gdoffset + i * 4);
            }
            break;
        case 0x5E:
            INST_NAME("DIVPS Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 12);
            s0 = fpu_get_scratch(dyn);
            s1 = fpu_get_scratch(dyn);
            if (!BOX64ENV(dynarec_fastnan)) MOV32w(x6, 0x00400000);
            for (int i = 0; i < 4; ++i) {
                // GX->f[i] /= EX->f[i];
                FLW(s0, wback, fixedaddress + i * 4);
                FLW(s1, gback, gdoffset + i * 4);
                if (!BOX64ENV(dynarec_fastnan)) {
                    FEQS(x3, s0, s0);
                    FEQS(x4, s1, s1);
                    AND(x5, x3, x4);
                    BEQZ(x5, 4 + 4 * 4);
                }
                FDIVS(s0, s1, s0);
                if (!BOX64ENV(dynarec_fastnan)) {
                    FEQS(x5, s0, s0);
                    BNEZ(x5, 4 + 6 * 4);
                    FNEGS(s0, s0);
                    BNEZ(x4, 4 + 4);
                    FMVS(s0, s1);
                    FMVXW(x5, s0);
                    OR(x5, x5, x6);
                    FMVWX(s0, x5);
                }
                FSW(s0, gback, gdoffset + i * 4);
            }
            break;
        case 0x5F:
            INST_NAME("MAXPS Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 12);
            s0 = fpu_get_scratch(dyn);
            s1 = fpu_get_scratch(dyn);
            for (int i = 0; i < 4; ++i) {
                FLW(s0, wback, fixedaddress + i * 4);
                FLW(s1, gback, gdoffset + i * 4);
                FLTS(x3, s0, s1);
                BNEZ(x3, 8);
                FSW(s0, gback, gdoffset + i * 4);
            }
            break;
        case 0x60:
            INST_NAME("PUNPCKLBW Gm,Em");
            nextop = F8;
            GETGM();
            for (int i = 3; i > 0; --i) { // 0 is untouched
                // GX->ub[2 * i] = GX->ub[i];
                LBU(x3, gback, gdoffset + i);
                SB(x3, gback, gdoffset + 2 * i);
            }
            if (MODREG && gd == (nextop & 7)) {
                for (int i = 0; i < 4; ++i) {
                    // GX->ub[2 * i + 1] = GX->ub[2 * i];
                    LBU(x3, gback, gdoffset + 2 * i);
                    SB(x3, gback, gdoffset + 2 * i + 1);
                }
            } else {
                GETEM(x2, 0, 3);
                for (int i = 0; i < 4; ++i) {
                    // GX->ub[2 * i + 1] = EX->ub[i];
                    LBU(x3, wback, fixedaddress + i);
                    SB(x3, gback, gdoffset + 2 * i + 1);
                }
            }
            break;
        case 0x61:
            INST_NAME("PUNPCKLWD Gm, Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0, 2);
            // GM->uw[3] = EM->uw[1];
            LHU(x3, wback, fixedaddress + 2 * 1);
            SH(x3, gback, gdoffset + 2 * 3);
            // GM->uw[2] = GM->uw[1];
            LHU(x3, gback, gdoffset + 2 * 1);
            SH(x3, gback, gdoffset + 2 * 2);
            // GM->uw[1] = EM->uw[0];
            LHU(x3, wback, fixedaddress + 2 * 0);
            SH(x3, gback, gdoffset + 2 * 1);
            break;
        case 0x62:
            INST_NAME("PUNPCKLDQ Gm, Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0, 1);
            // GM->ud[1] = EM->ud[0];
            LWU(x3, wback, fixedaddress);
            SW(x3, gback, gdoffset + 4 * 1);
            break;
        case 0x63:
            INST_NAME("PACKSSWB Gm,Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0, 6);
            MOV64x(x5, 127);
            MOV64x(x6, -128);
            for (int i = 0; i < 4; ++i) {
                LH(x3, gback, gdoffset + i * 2);
                if (cpuext.zbb) {
                    MIN(x3, x3, x5);
                    MAX(x3, x3, x6);
                } else {
                    BLT(x3, x5, 4 + 4);
                    MV(x3, x5);
                    BGE(x3, x6, 4 + 4);
                    MV(x3, x6);
                }
                SB(x3, gback, gdoffset + i);
            }
            if (MODREG && gd == ed) {
                LW(x3, gback, gdoffset + 0);
                SW(x3, gback, gdoffset + 4);
            } else
                for (int i = 0; i < 4; ++i) {
                    LH(x3, wback, fixedaddress + i * 2);
                    if (cpuext.zbb) {
                        MIN(x3, x3, x5);
                        MAX(x3, x3, x6);
                    } else {
                        BLT(x3, x5, 4 + 4);
                        MV(x3, x5);
                        BGE(x3, x6, 4 + 4);
                        MV(x3, x6);
                    }
                    SB(x3, gback, gdoffset + 4 + i);
                }
            break;
        case 0x64:
            INST_NAME("PCMPGTB Gm,Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0, 7);
            for (int i = 0; i < 8; ++i) {
                // GX->ub[i] = (GX->sb[i]>EX->sb[i])?0xFF:0x00;
                LB(x3, wback, fixedaddress + i);
                LB(x4, gback, gdoffset + i);
                SLT(x3, x3, x4);
                NEG(x3, x3);
                SB(x3, gback, gdoffset + i);
            }
            break;
        case 0x65:
            INST_NAME("PCMPGTW Gm,Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0, 6);
            MMX_LOOP_WS(x3, x4, SLT(x3, x4, x3); NEG(x3, x3));
            break;
        case 0x66:
            INST_NAME("PCMPGTD Gm,Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0, 4);
            MMX_LOOP_DS(x3, x4, SLT(x3, x4, x3); NEG(x3, x3));
            break;
        case 0x67:
            INST_NAME("PACKUSWB Gm, Em");
            nextop = F8;
            GETGM();
            ADDI(x5, xZR, 0xFF);
            for (int i = 0; i < 4; ++i) {
                // GX->ub[i] = (GX->sw[i]<0)?0:((GX->sw[i]>0xff)?0xff:GX->sw[i]);
                LH(x3, gback, gdoffset + i * 2);
                SATUw(x3, x5);
                SB(x3, gback, gdoffset + i);
            }
            if (MODREG && gd == (nextop & 7)) {
                // GM->ud[1] = GM->ud[0];
                LW(x3, gback, gdoffset + 0 * 4);
                SW(x3, gback, gdoffset + 1 * 4);
            } else {
                GETEM(x1, 0, 6);
                for (int i = 0; i < 4; ++i) {
                    // GX->ub[4+i] = (EX->sw[i]<0)?0:((EX->sw[i]>0xff)?0xff:EX->sw[i]);
                    LH(x3, wback, fixedaddress + i * 2);
                    SATUw(x3, x5);
                    SB(x3, gback, gdoffset + 4 + i);
                }
            }
            break;
        case 0x68:
            INST_NAME("PUNPCKHBW Gm,Em");
            nextop = F8;
            GETGM();
            for (int i = 0; i < 4; ++i) {
                // GX->ub[2 * i] = GX->ub[i + 4];
                LBU(x3, gback, gdoffset + i + 4);
                SB(x3, gback, gdoffset + 2 * i);
            }
            if (MODREG && gd == (nextop & 7)) {
                for (int i = 0; i < 4; ++i) {
                    // GX->ub[2 * i + 1] = GX->ub[2 * i];
                    LBU(x3, gback, gdoffset + 2 * i);
                    SB(x3, gback, gdoffset + 2 * i + 1);
                }
            } else {
                GETEM(x2, 0, 7);
                for (int i = 0; i < 4; ++i) {
                    // GX->ub[2 * i + 1] = EX->ub[i + 4];
                    LBU(x3, wback, fixedaddress + i + 4);
                    SB(x3, gback, gdoffset + 2 * i + 1);
                }
            }
            break;
        case 0x69:
            INST_NAME("PUNPCKHWD Gm,Em");
            nextop = F8;
            GETGM();
            for (int i = 0; i < 2; ++i) {
                // GX->uw[2 * i] = GX->uw[i + 2];
                LHU(x3, gback, gdoffset + (i + 2) * 2);
                SH(x3, gback, gdoffset + 2 * i * 2);
            }
            if (MODREG && gd == (nextop & 7)) {
                for (int i = 0; i < 2; ++i) {
                    // GX->uw[2 * i + 1] = GX->uw[2 * i];
                    LHU(x3, gback, gdoffset + 2 * i * 2);
                    SH(x3, gback, gdoffset + (2 * i + 1) * 2);
                }
            } else {
                GETEM(x1, 0, 6);
                for (int i = 0; i < 2; ++i) {
                    // GX->uw[2 * i + 1] = EX->uw[i + 2];
                    LHU(x3, wback, fixedaddress + (i + 2) * 2);
                    SH(x3, gback, gdoffset + (2 * i + 1) * 2);
                }
            }
            break;
        case 0x6A:
            INST_NAME("PUNPCKHDQ Gm,Em");
            nextop = F8;
            GETGM();
            GETEM(x1, 0, 4);
            // GM->ud[0] = GM->ud[1];
            LWU(x3, gback, gdoffset + 1 * 4);
            SW(x3, gback, gdoffset + 0 * 4);
            if (!(MODREG && (gd == ed))) {
                // GM->ud[1] = EM->ud[1];
                LWU(x3, wback, fixedaddress + 1 * 4);
                SW(x3, gback, gdoffset + 1 * 4);
            }
            break;
        case 0x6B:
            INST_NAME("PACKSSDW Gm,Em");
            nextop = F8;
            GETGM();
            MOV64x(x5, 32767);
            MOV64x(x6, -32768);
            for (int i = 0; i < 2; ++i) {
                // GM->sw[i] = (GM->sd[i]<-32768)?-32768:((GM->sd[i]>32767)?32767:GM->sd[i]);
                LW(x3, gback, gdoffset + i * 4);
                SATw(x3, x6, x5);
                SH(x3, gback, gdoffset + i * 2);
            }
            if (MODREG && gd == (nextop & 7)) {
                LWU(x3, gback, gdoffset);
                SW(x3, gback, gdoffset + 4);
            } else {
                GETEM(x1, 0, 4);
                for (int i = 0; i < 2; ++i) {
                    // GM->sw[2+i] = (EM->sd[i]<-32768)?-32768:((EM->sd[i]>32767)?32767:EM->sd[i]);
                    LW(x3, wback, fixedaddress + i * 4);
                    SATw(x3, x6, x5);
                    SH(x3, gback, gdoffset + 4 + i * 2);
                }
            }
            break;
        case 0x6E:
            INST_NAME("MOVD Gm, Ed");
            nextop = F8;
            GETGM();
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                MARKREGs(ed);
                if (!rex.w) {
                    ZEXTW2(x4, ed);
                    ed = x4;
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, x2, &fixedaddress, rex, NULL, 1, 0);
                LDxw(x4, ed, fixedaddress);
                ed = x4;
            }
            SD(ed, gback, gdoffset + 0);
            break;
        case 0x6F:
            INST_NAME("MOVQ Gm, Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0, 1);
            LD(x3, wback, fixedaddress);
            SD(x3, gback, gdoffset + 0);
            break;
        case 0x70:
            INST_NAME("PSHUFW Gm, Em, Ib");
            nextop = F8;
            GETGM();
            GETEM(x2, 1, 6);
            u8 = F8;
            LHU(x3, wback, fixedaddress + ((u8 >> (0 * 2)) & 3) * 2);
            LHU(x4, wback, fixedaddress + ((u8 >> (1 * 2)) & 3) * 2);
            LHU(x5, wback, fixedaddress + ((u8 >> (2 * 2)) & 3) * 2);
            LHU(x6, wback, fixedaddress + ((u8 >> (3 * 2)) & 3) * 2);
            SH(x3, gback, gdoffset + 0 * 2);
            SH(x4, gback, gdoffset + 1 * 2);
            SH(x5, gback, gdoffset + 2 * 2);
            SH(x6, gback, gdoffset + 3 * 2);
            break;
        case 0x71:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 2:
                    INST_NAME("PSRLW Em, Ib");
                    GETEM(x1, 1, 6);
                    u8 = F8;
                    if (u8 > 15) {
                        // just zero dest
                        SD(xZR, wback, fixedaddress);
                    } else if (u8) {
                        for (int i = 0; i < 4; ++i) {
                            // EX->uw[i] >>= u8;
                            LHU(x3, wback, fixedaddress + i * 2);
                            SRLI(x3, x3, u8);
                            SH(x3, wback, fixedaddress + i * 2);
                        }
                    }
                    break;
                case 4:
                    INST_NAME("PSRAW Em, Ib");
                    GETEM(x1, 1, 6);
                    u8 = F8;
                    if (u8 > 15) u8 = 15;
                    if (u8) {
                        for (int i = 0; i < 4; ++i) {
                            // EX->sw[i] >>= u8;
                            LH(x3, wback, fixedaddress + i * 2);
                            SRAI(x3, x3, u8);
                            SH(x3, wback, fixedaddress + i * 2);
                        }
                    }
                    break;
                case 6:
                    INST_NAME("PSLLW Em, Ib");
                    GETEM(x1, 1, 6);
                    u8 = F8;
                    if (u8 > 15) {
                        // just zero dest
                        SD(xZR, wback, fixedaddress + 0);
                    } else if (u8) {
                        for (int i = 0; i < 4; ++i) {
                            // EX->uw[i] <<= u8;
                            LHU(x3, wback, fixedaddress + i * 2);
                            SLLI(x3, x3, u8);
                            SH(x3, wback, fixedaddress + i * 2);
                        }
                    }
                    break;
                default: DEFAULT;
            }
            break;
        case 0x72:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 2:
                    INST_NAME("PSRLD Em, Ib");
                    GETEM(x4, 1, 4);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 31) {
                            SD(xZR, wback, fixedaddress);
                        } else {
                            LD(x1, wback, fixedaddress);
                            SRLI(x2, x1, 32 + u8);
                            SRLIW(x1, x1, u8);
                            SW(x1, wback, fixedaddress);
                            SW(x2, wback, fixedaddress + 4);
                        }
                    }
                    break;
                case 4:
                    INST_NAME("PSRAD Em, Ib");
                    GETEM(x4, 1, 4);
                    u8 = F8;
                    if (u8 > 31) u8 = 31;
                    if (u8) {
                        LD(x1, wback, fixedaddress);
                        SRAI(x2, x1, 32 + u8);
                        SRAIW(x1, x1, u8);
                        SW(x1, wback, fixedaddress);
                        SW(x2, wback, fixedaddress + 4);
                    }
                    break;
                case 6:
                    INST_NAME("PSLLD Em, Ib");
                    GETEM(x4, 1, 4);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 31) {
                            SD(xZR, wback, fixedaddress);
                        } else {
                            LD(x1, wback, fixedaddress);
                            SRLI(x2, x1, 32);
                            SLLIW(x1, x1, u8);
                            SLLIW(x2, x2, u8);
                            SW(x1, wback, fixedaddress);
                            SW(x2, wback, fixedaddress + 4);
                        }
                    }
                    break;
                default: DEFAULT;
            }
            break;
        case 0x73:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 2:
                    INST_NAME("PSRLQ Em, Ib");
                    GETEM(x4, 1, 1);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 63) {
                            SD(xZR, wback, fixedaddress);
                        } else if (u8) {
                            LD(x1, wback, fixedaddress);
                            SRLI(x1, x1, u8);
                            SD(x1, wback, fixedaddress);
                        }
                    }
                    break;
                case 6:
                    INST_NAME("PSLLQ Em, Ib");
                    GETEM(x4, 1, 1);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 63) {
                            SD(xZR, wback, fixedaddress);
                        } else {
                            LD(x1, wback, fixedaddress);
                            SLLI(x1, x1, u8);
                            SD(x1, wback, fixedaddress);
                        }
                    }
                    break;
                default: DEFAULT;
            }
            break;
        case 0x74:
            INST_NAME("PCMPEQB Gm,Em");
            nextop = F8;
            GETGM();
            if (cpuext.xtheadbb) {
                GETEM(x2, 0, 0);
                LD(x3, gback, gdoffset);
                LD(x4, wback, fixedaddress);
                XOR(x3, x3, x4);
                TH_TSTNBZ(x3, x3);
                SD(x3, gback, gdoffset);
            } else {
                GETEM(x2, 0, 7);
                for (int i = 0; i < 8; ++i) {
                    LBU(x3, gback, gdoffset + i);
                    LBU(x4, wback, fixedaddress + i);
                    SUB(x3, x3, x4);
                    SEQZ(x3, x3);
                    NEG(x3, x3);
                    SB(x3, gback, gdoffset + i);
                }
            }
            break;
        case 0x75:
            INST_NAME("PCMPEQW Gm,Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0, 6);
            MMX_LOOP_W(x3, x4, SUB(x3, x3, x4); SEQZ(x3, x3); NEG(x3, x3));
            break;
        case 0x76:
            INST_NAME("PCMPEQD Gm,Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0, 4);
            MMX_LOOP_D(x3, x4, SUB(x3, x3, x4); SEQZ(x3, x3); NEG(x3, x3));
            break;
        case 0x77:
            INST_NAME("EMMS");
            // empty MMX, FPU now usable
            mmx_purgecache(dyn, ninst, 0, x1);
            x87_purgecache(dyn, ninst, 0, x1, x2, x3); // also purge x87 and hard reset it
            SW(xZR, xEmu, offsetof(x64emu_t, top));
            SW(xZR, xEmu, offsetof(x64emu_t, fpu_stack));
            MOV32w(x1, TAGS_EMPTY);
            SD(x1, xEmu, offsetof(x64emu_t, fpu_tags));
            break;
        case 0x7E:
            INST_NAME("MOVD Ed, Gm");
            nextop = F8;
            gd = ((nextop & 0x38) >> 3);
            v0 = mmx_get_reg(dyn, ninst, x1, x2, x3, gd);
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                MARKREGd(ed);
                if (rex.w)
                    FMVXD(ed, v0);
                else {
                    FMVXW(ed, v0);
                    if (NEED_ZEROUP(ed)) ZEROUP_RESULT(ed);
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x2, &fixedaddress, rex, NULL, 1, 0);
                if (rex.w) {
                    FMVXD(x1, v0);
                    SD(x1, wback, fixedaddress);
                } else {
                    FMVXW(x1, v0);
                    SW(x1, wback, fixedaddress);
                }
                SMWRITE2();
            }
            break;
        case 0x7F:
            INST_NAME("MOVQ Em, Gm");
            nextop = F8;
            GETGM();
            GETEM(x2, 0, 1);
            LD(x3, gback, gdoffset + 0);
            SD(x3, wback, fixedaddress);
            if (!MODREG) SMWRITE2();
            break;
#define GO(GETFLAGS, NO, YES, NATNO, NATYES, F)                                                                \
    READFLAGS_FUSION(F, x1, x2, x3, x4, x5);                                                                   \
    i32_ = F32S;                                                                                               \
    if (rex.is32bits)                                                                                          \
        j64 = (uint32_t)(addr + i32_);                                                                         \
    else                                                                                                       \
        j64 = addr + i32_;                                                                                     \
    JUMP(j64, 1);                                                                                              \
    if (!dyn->insts[ninst].nat_flags_fusion) { GETFLAGS; }                                                     \
    rvv_prepare_branch(dyn, ninst, tmp1);                                                                      \
    if (dyn->insts[ninst].x64.jmp_insts == -1 || CHECK_CACHE()) {                                              \
        /* out of the block */                                                                                 \
        i32 = dyn->insts[ninst].epilog - (dyn->native_size);                                                   \
        if (dyn->insts[ninst].nat_flags_fusion) {                                                              \
            NATIVEJUMP_safe(NATNO, i32);                                                                       \
        } else {                                                                                               \
            B##NO##_safe(tmp1, i32);                                                                           \
        }                                                                                                      \
        if (dyn->insts[ninst].x64.jmp_insts == -1) {                                                           \
            if (!(dyn->insts[ninst].x64.barrier & BARRIER_FLOAT)) fpu_exitcache(dyn, ninst, tmp1, tmp2, tmp3); \
            jump_to_next(dyn, j64, 0, ninst, rex.is32bits);                                                    \
        } else {                                                                                               \
            simd_cache_transform(dyn, ninst, cacheupd, tmp1, tmp2, tmp3);                                      \
            i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address - (dyn->native_size);                    \
            B(i32);                                                                                            \
        }                                                                                                      \
    } else {                                                                                                   \
        /* inside the block */                                                                                 \
        i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address - (dyn->native_size);                        \
        if (dyn->insts[ninst].nat_flags_fusion) {                                                              \
            NATIVEJUMP_safe(NATYES, i32);                                                                      \
        } else {                                                                                               \
            B##YES##_safe(tmp1, i32);                                                                          \
        }                                                                                                      \
    }
            GOCOND(0x80, "J", "Id");
#undef GO

#define GO(GETFLAGS, NO, YES, NATNO, NATYES, F)                                                  \
    READFLAGS_FUSION(F, x1, x2, x3, x4, x5);                                                     \
    if (!dyn->insts[ninst].nat_flags_fusion) { GETFLAGS; }                                       \
    nextop = F8;                                                                                 \
    if (dyn->insts[ninst].nat_flags_fusion) {                                                    \
        NATIVESET(NATYES, tmp3);                                                                 \
    } else {                                                                                     \
        S##YES(tmp3, tmp1);                                                                      \
    }                                                                                            \
    if (MODREG) {                                                                                \
        if (rex.rex) {                                                                           \
            eb1 = TO_NAT((nextop & 7) + (rex.b << 3));                                           \
            eb2 = 0;                                                                             \
        } else {                                                                                 \
            ed = (nextop & 7);                                                                   \
            eb2 = (ed >> 2) * 8;                                                                 \
            eb1 = TO_NAT(ed & 3);                                                                \
        }                                                                                        \
        if (eb2) {                                                                               \
            MOV64x(tmp1, 0xFFFFFFFFFFFF00FF);                                                    \
            AND(eb1, eb1, tmp1);                                                                 \
            SLLI(tmp3, tmp3, 8);                                                                 \
        } else {                                                                                 \
            ANDI(eb1, eb1, 0xf00);                                                               \
        }                                                                                        \
        OR(eb1, eb1, tmp3);                                                                      \
    } else {                                                                                     \
        addr = geted(dyn, addr, ninst, nextop, &ed, tmp2, tmp1, &fixedaddress, rex, NULL, 1, 0); \
        SB(tmp3, ed, fixedaddress);                                                              \
        SMWRITE();                                                                               \
    }
            GOCOND(0x90, "SET", "Eb");
#undef GO

        default: DEFAULT;
    }
    return addr;
}
