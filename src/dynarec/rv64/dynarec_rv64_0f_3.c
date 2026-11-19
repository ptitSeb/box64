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


uintptr_t dynarec64_0F_3(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
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
        case 0xB0:
            INST_NAME("CMPXCHG Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_NOFUSION);
            nextop = F8;
            GETEB(x1, 0);
            ANDI(x4, xRAX, 0xff); // AL
            UFLAG_IF { emit_cmp8(dyn, ninst, x4, x1, x2, x5, x6, x7); }
            BNE_MARK(x4, x1);
            GETGB(x5);
            MV(x1, x5);
            ed = x1;
            EBBACK(x2, 0);
            B_NEXT_nocond;
            MARK;
            ANDI(xRAX, xRAX, ~0xff);
            OR(xRAX, xRAX, x1);
            break;
        case 0xB1:
            INST_NAME("CMPXCHG Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_NOFUSION);
            nextop = F8;
            GETGDs;
            MARKREGsd(xRAX);
            MVxw(x7, xRAX);
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                MARKREGsd(ed);
                UP32_READ(ed);
                MVxw(x1, ed);
                wback = 0;
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                LDxw(x1, wback, fixedaddress);
            }
            UFLAG_IF { emit_cmp32(dyn, ninst, rex, x7, x1, x3, x4, x5, x6); }
            BNE_MARK(x7, x1);
            if (wback) {
                SDxw(gd, wback, fixedaddress);
                SMWRITE();
            } else {
                MVxw(ed, gd);
            }
            B_NEXT_nocond;
            MARK;
            MVxw(xRAX, x1);
            break;
        case 0xB3:
            INST_NAME("BTR Ed, Gd");
            SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                MARKREGsd(ed);
                wback = 0;
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                if (rex.w)
                    SRAI(x1, gd, 6);
                else
                    SRAIW(x1, gd, 5);
                if (!rex.w && !rex.is32bits) { ADDIW(x1, x1, 0); }
                ADDSLy(x3, wback, x1, 2 + rex.w, x1);
                LDxw(x1, x3, fixedaddress);
                ed = x1;
                wback = x3;
            }
            BEXT(x4, ed, gd, x2); // F_CF is 1
            ANDI(xFlags, xFlags, ~1);
            OR(xFlags, xFlags, x4);
            if (cpuext.zbs) {
                ANDI(x2, gd, rex.w ? 0x3f : 0x1f);
                BCLR(ed, ed, x2);
            } else {
                ADDI(x4, xZR, 1);
                ANDI(x2, gd, rex.w ? 0x3f : 0x1f);
                SLL(x4, x4, x2);
                NOT(x4, x4);
                AND(ed, ed, x4);
            }
            if (wback) {
                SDxw(ed, wback, fixedaddress);
                SMWRITE();
            } else if (!rex.w) {
                if (NEED_ZEROUP(ed)) ZEROUP_RESULT(ed);
            }
            break;
        case 0xB6:
            INST_NAME("MOVZX Gd, Eb");
            nextop = F8;
            GETGDd;
            SCRATCH_USAGE(0);
            if (MODREG) {
                if (rex.rex) {
                    eb1 = TO_NAT((nextop & 7) + (rex.b << 3));
                    eb2 = 0;
                } else {
                    ed = (nextop & 7);
                    eb1 = TO_NAT(ed & 3); // Ax, Cx, Dx or Bx
                    eb2 = (ed & 4) >> 2;  // L or H
                }
                if (eb2) {
                    SRLI(gd, eb1, 8);
                    ANDI(gd, gd, 0xff);
                } else {
                    ANDI(gd, eb1, 0xff);
                }
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                LBU(gd, ed, fixedaddress);
            }
            break;
        case 0xB7:
            INST_NAME("MOVZX Gd, Ew");
            nextop = F8;
            GETGDd;
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                ZEXTH(gd, ed);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                LHU(gd, ed, fixedaddress);
            }
            break;
        case 0xBA:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 4:
                    INST_NAME("BT Ed, Ib");
                    SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                    SET_DFNONE();
                    GETED(1);
                    u8 = F8;
                    u8 &= rex.w ? 0x3f : 0x1f;
                    BEXTI(x3, ed, u8); // F_CF is 1
                    ANDI(xFlags, xFlags, ~1);
                    OR(xFlags, xFlags, x3);
                    break;
                case 5:
                    INST_NAME("BTS Ed, Ib");
                    SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                    SET_DFNONE();
                    GETEDsd(1);
                    u8 = F8;
                    u8 &= (rex.w ? 0x3f : 0x1f);
                    if (cpuext.zbs) {
                        BEXTI(x3, ed, u8); // F_CF is 1
                        ANDI(xFlags, xFlags, ~1);
                        OR(xFlags, xFlags, x3);
                        BSETI_(ed, ed, u8);
                    } else {
                        ORI(xFlags, xFlags, 1 << F_CF);
                        if (u8 <= 10) {
                            ANDI(x6, ed, 1 << u8);
                            BNE_MARK(x6, xZR);
                            ANDI(xFlags, xFlags, ~(1 << F_CF));
                            XORI(ed, ed, 1 << u8);
                        } else {
                            ORI(x6, xZR, 1);
                            SLLI(x6, x6, u8);
                            AND(x4, ed, x6);
                            BNE_MARK(x4, xZR);
                            ANDI(xFlags, xFlags, ~(1 << F_CF));
                            XOR(ed, ed, x6);
                        }
                    }
                    if (wback) {
                        SDxw(ed, wback, fixedaddress);
                        SMWRITE();
                    }
                    MARK;
                    if (!rex.w && !wback)
                        if (NEED_ZEROUP(ed)) ZEROUP_RESULT(ed);
                    break;
                case 6:
                    INST_NAME("BTR Ed, Ib");
                    SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                    SET_DFNONE();
                    GETEDsd(1);
                    u8 = F8;
                    u8 &= (rex.w ? 0x3f : 0x1f);
                    if (cpuext.zbs) {
                        BEXTI(x3, ed, u8); // F_CF is 1
                        ANDI(xFlags, xFlags, ~1);
                        OR(xFlags, xFlags, x3);
                        BCLI(ed, ed, u8);
                    } else {
                        ANDI(xFlags, xFlags, ~(1 << F_CF));
                        if (u8 <= 10) {
                            ANDI(x6, ed, 1 << u8);
                            BEQ_MARK(x6, xZR);
                            ORI(xFlags, xFlags, 1 << F_CF);
                            XORI(ed, ed, 1 << u8);
                        } else {
                            ORI(x6, xZR, 1);
                            SLLI(x6, x6, u8);
                            AND(x6, ed, x6);
                            BEQ_MARK(x6, xZR);
                            ORI(xFlags, xFlags, 1 << F_CF);
                            XOR(ed, ed, x6);
                        }
                    }
                    MARK;
                    if (wback) {
                        SDxw(ed, wback, fixedaddress);
                        SMWRITE();
                    } else if (!rex.w) {
                        if (NEED_ZEROUP(ed)) ZEROUP_RESULT(ed);
                    }
                    break;
                case 7:
                    INST_NAME("BTC Ed, Ib");
                    SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                    SET_DFNONE();
                    GETEDsd(1);
                    u8 = F8;
                    u8 &= rex.w ? 0x3f : 0x1f;
                    BEXTI(x3, ed, u8); // F_CF is 1
                    ANDI(xFlags, xFlags, ~1);
                    OR(xFlags, xFlags, x3);
                    if (cpuext.zbs) {
                        BINVI(ed, ed, u8);
                    } else if (u8 <= 10) {
                        XORI(ed, ed, (1LL << u8));
                    } else {
                        MOV64xw(x3, (1LL << u8));
                        XOR(ed, ed, x3);
                    }
                    if (wback) {
                        SDxw(ed, wback, fixedaddress);
                        SMWRITE();
                    } else if (!rex.w) {
                        if (NEED_ZEROUP(ed)) ZEROUP_RESULT(ed);
                    }
                    break;
                default: DEFAULT;
            }
            break;
        case 0xBB:
            INST_NAME("BTC Ed, Gd");
            SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                MARKREGsd(ed);
                wback = 0;
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x1, &fixedaddress, rex, NULL, 1, 0);
                if (rex.w)
                    SRAI(x1, gd, 6);
                else
                    SRAIW(x1, gd, 5);
                if (!rex.w && !rex.is32bits) { ADDIW(x1, x1, 0); }
                ADDSLy(x3, wback, x1, 2 + rex.w, x1);
                LDxw(x1, x3, fixedaddress);
                ed = x1;
                wback = x3;
            }
            BEXT(x4, ed, gd, x2); // F_CF is 1
            ANDI(xFlags, xFlags, ~1);
            OR(xFlags, xFlags, x4);
            if (cpuext.zbs) {
                ANDI(x2, gd, rex.w ? 0x3f : 0x1f);
                BINV(ed, ed, x2);
            } else {
                ADDI(x4, xZR, 1);
                ANDI(x2, gd, rex.w ? 0x3f : 0x1f);
                SLL(x4, x4, x2);
                XOR(ed, ed, x4);
            }
            if (wback) {
                SDxw(ed, wback, fixedaddress);
                SMWRITE();
            } else if (!rex.w) {
                if (NEED_ZEROUP(ed)) ZEROUP_RESULT(ed);
            }
            break;
        case 0xBC:
            INST_NAME("BSF Gd, Ed");
            if (!BOX64DRENV(dynarec_safeflags)) {
                SETFLAGS(X_ZF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            } else {
                SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION);
            }
            SET_DFNONE();
            CLEAR_FLAGS();
            nextop = F8;
            GETED(0);
            GETGD;
            if (!rex.w && MODREG) {
                ZEXTW2(x4, ed);
                ed = x4;
            }
            if (BOX64DRENV(dynarec_safeflags)) {
                IFX (X_PF) MV(x3, xZR);
            }
            BNE_MARK(ed, xZR);
            IFX (X_ZF) ORI(xFlags, xFlags, 1 << F_ZF);
            B_MARK2_nocond;
            MARK;
            CTZxw(gd, ed, rex.w, x3, x5);
            if (BOX64DRENV(dynarec_safeflags)) {
                IFX (X_PF) MV(x3, gd);
            }
            MARK2;
            if (BOX64DRENV(dynarec_safeflags)) {
                IFX (X_PF) emit_pf(dyn, ninst, x3, x2, x5);
            }
            break;
        case 0xBD:
            INST_NAME("BSR Gd, Ed");
            if (!BOX64DRENV(dynarec_safeflags)) {
                SETFLAGS(X_ZF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            } else {
                SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION);
            }
            SET_DFNONE();
            CLEAR_FLAGS();
            nextop = F8;
            GETED(0);
            GETGD;
            if (!rex.w && MODREG) {
                ZEXTW2(x4, ed);
                ed = x4;
            }
            if (BOX64DRENV(dynarec_safeflags)) {
                IFX (X_PF) MV(x2, xZR);
            }
            BNE_MARK(ed, xZR);
            IFX (X_ZF) ORI(xFlags, xFlags, 1 << F_ZF);
            B_MARK2_nocond;
            MARK;
            CLZxw(gd, ed, rex.w, x3, x5, x7);
            ADDI(x3, xZR, rex.w ? 63 : 31);
            SUB(gd, x3, gd);
            if (BOX64DRENV(dynarec_safeflags)) {
                IFX (X_PF) MV(x2, gd);
            }
            MARK2;
            if (BOX64DRENV(dynarec_safeflags)) {
                IFX (X_PF) emit_pf(dyn, ninst, x2, x4, x5);
            }
            break;
        case 0xBE:
            INST_NAME("MOVSX Gd, Eb");
            nextop = F8;
            GETGDd;
            if (MODREG) {
                if (rex.rex) {
                    wback = TO_NAT((nextop & 7) + (rex.b << 3));
                    wb2 = 0;
                } else {
                    wback = (nextop & 7);
                    wb2 = (wback >> 2) * 8;
                    wback = TO_NAT(wback & 3);
                }
                SLLI(gd, wback, 56 - wb2);
                SRAI(gd, gd, 56);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, x1, &fixedaddress, rex, NULL, 1, 0);
                LB(gd, ed, fixedaddress);
            }
            if (!rex.w)
                if (NEED_ZEROUP(gd)) ZEROUP_RESULT(gd);
            break;
        case 0xBF:
            INST_NAME("MOVSX Gd, Ew");
            nextop = F8;
            GETGDd;
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                SEXTH(gd, ed);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, x1, &fixedaddress, rex, NULL, 1, 0);
                LH(gd, ed, fixedaddress);
            }
            if (!rex.w)
                if (NEED_ZEROUP(gd)) ZEROUP_RESULT(gd);
            break;
        case 0xC0:
            INST_NAME("XADD Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETEB(x1, 0);
            GETGB(x2);
            if (!(MODREG && wback == gb1 && !!(wb2) == !!(gb2))) MV(x7, ed);
            emit_add8(dyn, ninst, ed, gd, x4, x5, x6);
            if (!(MODREG && wback == gb1 && !!(wb2) == !!(gb2))) MV(gd, x7);
            EBBACK(x5, 0);
            if (!(MODREG && wback == gb1 && !!(wb2) == !!(gb2))) GBBACK(x5);
            break;
        case 0xC1:
            INST_NAME("XADD Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGDsd;
            GETED(0);
            if (ed != gd) MV(x7, ed);
            emit_add32(dyn, ninst, rex, ed, gd, x4, x5, x6);
            WBACK;
            if (ed != gd) MVxw(gd, x7);
            break;
        case 0xC2:
            INST_NAME("CMPPS Gx, Ex, Ib");
            nextop = F8;
            GETGX();
            GETEX(x2, 1, 12);
            u8 = F8;
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            for (int i = 0; i < 4; ++i) {
                FLW(d0, gback, gdoffset + i * 4);
                FLW(d1, wback, fixedaddress + i * 4);
                if ((u8 & 7) == 0) { // Equal
                    FEQS(x3, d0, d1);
                } else if ((u8 & 7) == 4) { // Not Equal or unordered
                    FEQS(x3, d0, d1);
                    XORI(x3, x3, 1);
                } else {
                    // x4 = !(isnan(d0) || isnan(d1))
                    FEQS(x4, d0, d0);
                    FEQS(x3, d1, d1);
                    AND(x3, x3, x4);

                    switch (u8 & 7) {
                        case 1:
                            BEQ(x3, xZR, 8); // MARK2
                            FLTS(x3, d0, d1);
                            break; // Less than
                        case 2:
                            BEQ(x3, xZR, 8); // MARK2
                            FLES(x3, d0, d1);
                            break;                      // Less or equal
                        case 3: XORI(x3, x3, 1); break; // NaN
                        case 5: {                       // Greater or equal or unordered
                            BEQ(x3, xZR, 12);           // MARK2
                            FLES(x3, d1, d0);
                            J(8); // MARK;
                            break;
                        }
                        case 6: {             // Greater or unordered, test inverted, N!=V so unordered or less than (inverted)
                            BEQ(x3, xZR, 12); // MARK2
                            FLTS(x3, d1, d0);
                            J(8); // MARK;
                            break;
                        }
                        case 7: break; // Not NaN
                    }

                    // MARK2;
                    if ((u8 & 7) == 5 || (u8 & 7) == 6) { MOV32w(x3, 1); }
                    // MARK;
                }
                NEG(x3, x3);
                SW(x3, gback, gdoffset + i * 4);
            }
            break;
        case 0xC3:
            INST_NAME("MOVNTI Ed, Gd");
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                MARKREGd(ed);
                MVxw(ed, gd);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                SDxw(gd, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0xC4:
            INST_NAME("PINSRW Gm,Ed,Ib");
            nextop = F8;
            GETGM();
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x3, &fixedaddress, rex, NULL, 1, 1);
                LHU(x3, wback, fixedaddress);
                ed = x3;
            }
            u8 = (F8) & 3;
            SH(ed, gback, gdoffset + u8 * 2);
            break;
        case 0xC5:
            INST_NAME("PEXTRW Gd,Em,Ib");
            nextop = F8;
            GETGDd;
            GETEM(x2, 1, 6);
            u8 = (F8) & 3;
            LHU(gd, wback, fixedaddress + u8 * 2);
            break;
        case 0xC6:
            INST_NAME("SHUFPS Gx, Ex, Ib");
            nextop = F8;
            GETGX();
            GETEX(x2, 1, 12);
            u8 = F8;
            int32_t idx;

            idx = (u8 >> (0 * 2)) & 3;
            LWU(x3, gback, gdoffset + idx * 4);
            idx = (u8 >> (1 * 2)) & 3;
            LWU(x4, gback, gdoffset + idx * 4);
            idx = (u8 >> (2 * 2)) & 3;
            LWU(x5, wback, fixedaddress + idx * 4);
            idx = (u8 >> (3 * 2)) & 3;
            LWU(x6, wback, fixedaddress + idx * 4);

            SW(x3, gback, gdoffset + 0 * 4);
            SW(x4, gback, gdoffset + 1 * 4);
            SW(x5, gback, gdoffset + 2 * 4);
            SW(x6, gback, gdoffset + 3 * 4);
            break;
        case 0xC7:
            // rep has no impact here
            nextop = F8;
            if (MODREG) {
                switch ((nextop >> 3) & 7) {
                    default: DEFAULT;
                }
            } else {
                switch ((nextop >> 3) & 7) {
                    case 1:
                        if (rex.w) {
                            INST_NAME("CMPXCHG16B Gq, Eq");
                            MARKREGsd(xRAX);
                            MARKREGsd(xRDX);
                            MARKREGs(xRBX);
                            MARKREGs(xRCX);
                            SETFLAGS(X_ZF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                            SET_DFNONE();
                            SMREAD();
                            addr = geted(dyn, addr, ninst, nextop, &wback, x1, x2, &fixedaddress, rex, NULL, 0, 0);
                            ANDI(xFlags, xFlags, ~(1 << F_ZF));
                            LD(x3, wback, fixedaddress + 0);
                            LD(x4, wback, fixedaddress + 8);
                            BNE_MARK(x3, xRAX);
                            BNE_MARK2(x4, xRDX);
                            SD(xRBX, wback, fixedaddress + 0);
                            SD(xRCX, wback, fixedaddress + 8);
                            SMWRITE();
                            ORI(xFlags, xFlags, 1 << F_ZF);
                            B_NEXT_nocond;
                            MARK;
                            MV(xRAX, x3);
                            MARK2;
                            MV(xRDX, x4);
                        } else {
                            INST_NAME("CMPXCHG8B Gq, Eq");
                            SETFLAGS(X_ZF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                            SMREAD();
                            addr = geted(dyn, addr, ninst, nextop, &wback, x1, x2, &fixedaddress, rex, NULL, 0, 0);
                            ANDI(xFlags, xFlags, ~(1 << F_ZF));
                            ZEXTW2(x3, xRAX);
                            SLLI(x2, xRDX, 32);
                            OR(x3, x3, x2); // x3 is edx:eax
                            LD(x2, wback, fixedaddress);
                            BNE_MARK(x2, x3);
                            ZEXTW2(x4, xRBX);
                            SLLI(x5, xRCX, 32);
                            OR(x4, x4, x5); // x4 is ecx:ebx
                            SD(x4, wback, fixedaddress);
                            SMWRITE();
                            ORI(xFlags, xFlags, 1 << F_ZF);
                            B_NEXT_nocond;
                            MARK;
                            SRLI(xRDX, x2, 32);
                            ZEXTW2(xRAX, x2);
                        }
                        break;
                    case 4:
                        INST_NAME("Unsupported XSAVEC Ed");
                        FAKEED;
                        UDF();
                        break;
                    default: DEFAULT;
                }
            }
            break;
        case 0xC8:
        case 0xC9:
        case 0xCA:
        case 0xCB:
        case 0xCC:
        case 0xCD:
        case 0xCE:
        case 0xCF:
            INST_NAME("BSWAP Reg");
            gd = TO_NAT((opcode & 7) + (rex.b << 3));
            MARKREGsd(gd);
            REV8xw(gd, gd, x1, x2, x3, x4);
            break;
        case 0xD1:
            INST_NAME("PSRLW Gm, Em");
            nextop = F8;
            GETGM();
            GETEM(x4, 0, 1);
            LD(x1, wback, fixedaddress);
            ADDI(x2, xZR, 15);
            BLTU_MARK(x2, x1);
            for (int i = 0; i < 4; ++i) {
                LHU(x3, gback, gdoffset + 2 * i);
                SRL(x3, x3, x1);
                SH(x3, gback, gdoffset + 2 * i);
            }
            B_NEXT_nocond;
            MARK;
            SD(xZR, gback, gdoffset + 0);
            break;
        case 0xD2:
            INST_NAME("PSRLD Gm, Em");
            nextop = F8;
            GETGM();
            GETEM(x4, 0, 1);
            LD(x1, wback, fixedaddress);
            ADDI(x2, xZR, 31);
            BLTU_MARK(x2, x1);
            for (int i = 0; i < 2; ++i) {
                LWU(x3, gback, gdoffset + 4 * i);
                SRL(x3, x3, x1);
                SW(x3, gback, gdoffset + 4 * i);
            }
            B_NEXT_nocond;
            MARK;
            SD(xZR, gback, gdoffset + 0);
            break;
        case 0xD3:
            INST_NAME("PSRLQ Gm,Em");
            nextop = F8;
            GETGM();
            GETEM(x4, 0, 1);
            LD(x1, wback, fixedaddress);
            ADDI(x2, xZR, 63);
            BLTU_MARK(x2, x1);
            LD(x3, gback, gdoffset + 0);
            SRL(x3, x3, x1);
            SD(x3, gback, gdoffset + 0);
            B_NEXT_nocond;
            MARK;
            SD(xZR, gback, gdoffset + 0);
            break;
        case 0xD4:
            INST_NAME("PADDQ Gm,Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0, 1);
            LD(x1, wback, fixedaddress);
            LD(x2, gback, gdoffset);
            ADD(x1, x1, x2);
            SD(x1, gback, gdoffset);
            break;
        case 0xD5:
            INST_NAME("PMULLW Gm, Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0, 6);
            MMX_LOOP_WS(x3, x4, MULW(x3, x3, x4));
            break;
        case 0xD7:
            INST_NAME("PMOVMSKB Gd, Em");
            nextop = F8;
            GETGDd;
            GETEM(x2, 0, 1);
            LD(x1, wback, fixedaddress + 0);
            for (int i = 0; i < 8; i++) {
                if (cpuext.zbs) {
                    if (i == 0) {
                        BEXTI(gd, x1, 63);
                    } else {
                        BEXTI(x6, x1, 63 - i * 8);
                    }
                } else {
                    if (i == 0) {
                        SRLI(gd, x1, 63);
                    } else {
                        SRLI(x6, x1, 63 - i * 8);
                        ANDI(x6, x6, 1);
                    }
                }
                if (i != 0) {
                    if (cpuext.zba) {
                        SH1ADD(gd, gd, x6);
                    } else {
                        SLLI(gd, gd, 1);
                        OR(gd, gd, x6);
                    }
                }
            }
            break;
        case 0xD8:
            INST_NAME("PSUBUSB Gm, Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0, 7);
            for (int i = 0; i < 8; ++i) {
                LBU(x3, gback, gdoffset + i);
                LBU(x4, wback, fixedaddress + i);
                SUB(x3, x3, x4);
                if (cpuext.zbb) {
                    MAX(x3, x3, xZR);
                } else {
                    NOT(x4, x3);
                    SRAI(x4, x4, 63);
                    AND(x3, x3, x4);
                }
                SB(x3, gback, gdoffset + i);
            }
            break;
        case 0xD9:
            INST_NAME("PSUBUSW Gm, Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0, 6);
            MMX_LOOP_W(
                x3, x4, SUB(x3, x3, x4); if (cpuext.zbb) { MAX(x3, x3, xZR); } else {
                    NOT(x4, x3);
                    SRAI(x4, x4, 63);
                    AND(x3, x3, x4);
                } SH(x3, gback, gdoffset + i * 2););
            break;
        case 0xDA:
            INST_NAME("PMINUB Gm, Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0, 7);
            for (int i = 0; i < 8; ++i) {
                LBU(x3, gback, gdoffset + i);
                LBU(x4, wback, fixedaddress + i);
                if (cpuext.zbb) {
                    MINU(x3, x3, x4);
                } else {
                    BLTU(x3, x4, 8);
                    MV(x3, x4);
                }
                SB(x3, gback, gdoffset + i);
            }
            break;
        case 0xDB:
            INST_NAME("PAND Gm, Em");
            nextop = F8;
            GETGM();
            GETEM(x4, 0, 1);
            LD(x1, wback, fixedaddress);
            LD(x2, gback, gdoffset);
            AND(x1, x1, x2);
            SD(x1, gback, gdoffset);
            break;
        case 0xDC:
            INST_NAME("PADDUSB Gm,Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0, 7);
            ADDI(x5, xZR, 0xFF);
            for (int i = 0; i < 8; ++i) {
                LBU(x3, gback, gdoffset + i);
                LBU(x4, wback, fixedaddress + i);
                ADD(x3, x3, x4);
                if (cpuext.zbb) {
                    MINU(x3, x3, x5);
                } else {
                    BLT(x3, x5, 8);
                    ADDI(x3, xZR, 0xFF);
                }
                SB(x3, gback, gdoffset + i);
            }
            break;
        case 0xDD:
            INST_NAME("PADDUSW Gm,Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0, 6);
            MOV32w(x5, 65535);
            for (int i = 0; i < 4; ++i) {
                // tmp32s = (int32_t)GX->uw[i] + EX->uw[i];
                // GX->uw[i] = (tmp32s>65535)?65535:tmp32s;
                LHU(x3, gback, gdoffset + i * 2);
                LHU(x4, wback, fixedaddress + i * 2);
                ADDW(x3, x3, x4);
                if (cpuext.zbb) {
                    MINU(x3, x3, x5);
                } else {
                    BGE(x5, x3, 8); // tmp32s <= 65535?
                    MV(x3, x5);
                }
                SH(x3, gback, gdoffset + i * 2);
            }
            break;
        case 0xDE:
            INST_NAME("PMAXUB Gm, Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0, 7);
            for (int i = 0; i < 8; ++i) {
                LBU(x3, gback, gdoffset + i);
                LBU(x4, wback, fixedaddress + i);
                if (cpuext.zbb) {
                    MAXU(x3, x3, x4);
                } else {
                    BLTU(x4, x3, 8);
                    MV(x3, x4);
                }
                SB(x3, gback, gdoffset + i);
            }
            break;
        case 0xDF:
            INST_NAME("PANDN Gm, Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0, 1);
            LD(x1, gback, gdoffset);
            LD(x3, wback, fixedaddress);
            ANDN(x1, x3, x1, x1);
            SD(x1, gback, gdoffset);
            break;
        case 0xE0:
            INST_NAME("PAVGB Gm, Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0, 7);
            for (int i = 0; i < 8; ++i) {
                LBU(x3, gback, gdoffset + i);
                LBU(x4, wback, fixedaddress + i);
                ADDW(x3, x3, x4);
                ADDIW(x3, x3, 1);
                SRAIW(x3, x3, 1);
                SB(x3, gback, gdoffset + i);
            }
            break;
        case 0xE1:
            INST_NAME("PSRAW Gm,Em");
            nextop = F8;
            GETGM();
            GETEM(x4, 0, 1);
            LD(x1, wback, fixedaddress);
            ADDI(x2, xZR, 15);
            if (cpuext.zbb) {
                MINU(x1, x1, x2);
            } else {
                BLTU(x1, x2, 4 + 4);
                MV(x1, x2);
            }
            for (int i = 0; i < 4; ++i) {
                LH(x3, gback, gdoffset + 2 * i);
                SRAW(x3, x3, x1);
                SH(x3, gback, gdoffset + 2 * i);
            }
            break;
        case 0xE2:
            INST_NAME("PSRAD Gm, Em");
            nextop = F8;
            GETGM();
            GETEM(x4, 0, 1);
            LD(x1, wback, fixedaddress);
            ADDI(x2, xZR, 31);
            if (cpuext.zbb) {
                MINU(x1, x1, x2);
            } else {
                BLTU(x1, x2, 4 + 4);
                MV(x1, x2);
            }
            for (int i = 0; i < 2; ++i) {
                LW(x3, gback, gdoffset + 4 * i);
                SRAW(x3, x3, x1);
                SW(x3, gback, gdoffset + 4 * i);
            }
            break;
        case 0xE3:
            INST_NAME("PAVGW Gm,Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0, 6);
            for (int i = 0; i < 4; ++i) {
                LHU(x3, gback, gdoffset + 2 * i);
                LHU(x4, wback, fixedaddress + 2 * i);
                ADDW(x3, x3, x4);
                ADDIW(x3, x3, 1);
                SRAIW(x3, x3, 1);
                SH(x3, gback, gdoffset + 2 * i);
            }
            break;
        case 0xE4:
            INST_NAME("PMULHUW Gm,Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0, 6);
            for (int i = 0; i < 4; ++i) {
                LHU(x3, gback, gdoffset + 2 * i);
                LHU(x4, wback, fixedaddress + 2 * i);
                MULW(x3, x3, x4);
                SRLIW(x3, x3, 16);
                SH(x3, gback, gdoffset + 2 * i);
            }
            break;
        case 0xE5:
            INST_NAME("PMULHW Gm,Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0, 6);
            for (int i = 0; i < 4; ++i) {
                LH(x3, gback, gdoffset + 2 * i);
                LH(x4, wback, fixedaddress + 2 * i);
                MULW(x3, x3, x4);
                SRAIW(x3, x3, 16);
                SH(x3, gback, gdoffset + 2 * i);
            }
            break;
        case 0xE7:
            INST_NAME("MOVNTQ Em, Gm");
            nextop = F8;
            gd = (nextop & 0x38) >> 3;
            if (MODREG) {
                DEFAULT;
            } else {
                v0 = mmx_get_reg(dyn, ninst, x1, x2, x3, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                FSD(v0, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0xE8:
            INST_NAME("PSUBSB Gm,Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0, 7);
            ADDI(x5, xZR, 0x7f);
            ADDI(x6, xZR, 0xf80);
            for (int i = 0; i < 8; ++i) {
                LB(x3, gback, gdoffset + i);
                LB(x4, wback, fixedaddress + i);
                SUBW(x3, x3, x4);
                if (cpuext.zbb) {
                    MIN(x3, x3, x5);
                    MAX(x3, x3, x6);
                } else {
                    BLT(x3, x5, 4 + 4);
                    MV(x3, x5);
                    BLT(x6, x3, 4 + 4);
                    MV(x3, x6);
                }
                SB(x3, gback, gdoffset + i);
            }
            break;
        case 0xE9:
            INST_NAME("PSUBSW Gm,Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0, 6);
            MOV64x(x5, 32767);
            MOV64x(x6, -32768);
            for (int i = 0; i < 4; ++i) {
                // tmp32s = (int32_t)GM->sw[i] - EM->sw[i];
                // GM->sw[i] = (tmp32s>32767)?32767:((tmp32s<-32768)?-32768:tmp32s);
                LH(x3, gback, gdoffset + 2 * i);
                LH(x4, wback, fixedaddress + 2 * i);
                SUBW(x3, x3, x4);
                if (cpuext.zbb) {
                    MIN(x3, x3, x5);
                    MAX(x3, x3, x6);
                } else {
                    BLT(x3, x5, 4 + 4);
                    MV(x3, x5);
                    BLT(x6, x3, 4 + 4);
                    MV(x3, x6);
                }
                SH(x3, gback, gdoffset + 2 * i);
            }
            break;
        case 0xEA:
            INST_NAME("PMINSW Gx,Ex");
            nextop = F8;
            GETGM();
            GETEM(x2, 0, 6);
            for (int i = 0; i < 4; ++i) {
                LH(x3, gback, gdoffset + 2 * i);
                LH(x4, wback, fixedaddress + 2 * i);
                if (cpuext.zbb) {
                    MIN(x3, x3, x4);
                } else {
                    BLT(x3, x4, 8);
                    MV(x3, x4);
                }
                SH(x3, gback, gdoffset + 2 * i);
            }
            break;
        case 0xEB:
            INST_NAME("POR Gm, Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0, 1);
            LD(x3, gback, gdoffset);
            LD(x4, wback, fixedaddress);
            OR(x3, x3, x4);
            SD(x3, gback, gdoffset);
            break;
        case 0xEC:
            INST_NAME("PADDSB Gm,Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0, 7);
            MOV64x(x5, 127);
            MOV64x(x6, -128);
            for (int i = 0; i < 8; ++i) {
                // tmp16s = (int16_t)GX->sb[i] + EX->sb[i];
                // GX->sb[i] = (tmp16s>127)?127:((tmp16s<-128)?-128:tmp16s);
                LB(x3, gback, gdoffset + i);
                LB(x4, wback, fixedaddress + i);
                ADDW(x3, x3, x4);
                if (cpuext.zbb) {
                    MIN(x3, x3, x5);
                    MAX(x3, x3, x6);
                } else {
                    BLT(x3, x5, 4 + 4);
                    MV(x3, x5);
                    BLT(x6, x3, 4 + 4);
                    MV(x3, x6);
                }
                SB(x3, gback, gdoffset + i);
            }
            break;
        case 0xED:
            INST_NAME("PADDSW Gm,Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0, 6);
            MOV64x(x5, 32767);
            MOV64x(x6, -32768);
            for (int i = 0; i < 4; ++i) {
                // tmp32s = (int32_t)GM->sw[i] + EM->sw[i];
                // GM->sw[i] = (tmp32s>32767)?32767:((tmp32s<-32768)?-32768:tmp32s);
                LH(x3, gback, gdoffset + 2 * i);
                LH(x4, wback, fixedaddress + 2 * i);
                ADDW(x3, x3, x4);
                if (cpuext.zbb) {
                    MIN(x3, x3, x5);
                    MAX(x3, x3, x6);
                } else {
                    BLT(x3, x5, 4 + 4);
                    MV(x3, x5);
                    BLT(x6, x3, 4 + 4);
                    MV(x3, x6);
                }
                SH(x3, gback, gdoffset + 2 * i);
            }
            break;
        case 0xEE:
            INST_NAME("PMAXSW Gm,Em");
            nextop = F8;
            GETGM();
            GETEM(x1, 0, 6);
            MMX_LOOP_WS(
                x3, x4, if (cpuext.zbb) { MAX(x3, x3, x4); } else {
                    BGE(x3, x4, 8);
                    MV(x3, x4);
                });
            break;
        case 0xEF:
            INST_NAME("PXOR Gm,Em");
            nextop = F8;
            GETGM();
            if (MODREG && gd == (nextop & 7)) {
                // just zero dest
                SD(xZR, gback, gdoffset + 0);
            } else {
                GETEM(x2, 0, 1);
                LD(x3, gback, gdoffset + 0);
                LD(x4, wback, fixedaddress);
                XOR(x3, x3, x4);
                SD(x3, gback, gdoffset + 0);
            }
            break;
        case 0xF1:
            INST_NAME("PSLLW Gm,Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0, 1);
            ADDI(x4, xZR, 15);
            LD(x1, wback, fixedaddress + 0);
            BLTU_MARK(x4, x1);
            LH(x3, gback, gdoffset + 0 * 2);
            LH(x4, gback, gdoffset + 1 * 2);
            LH(x5, gback, gdoffset + 2 * 2);
            LH(x6, gback, gdoffset + 3 * 2);
            SLL(x3, x3, x1);
            SLL(x4, x4, x1);
            SLL(x5, x5, x1);
            SLL(x6, x6, x1);
            SH(x3, gback, gdoffset + 0 * 2);
            SH(x4, gback, gdoffset + 1 * 2);
            SH(x5, gback, gdoffset + 2 * 2);
            SH(x6, gback, gdoffset + 3 * 2);
            B_NEXT_nocond;
            MARK;
            SD(xZR, gback, gdoffset + 0);
            break;
        case 0xF2:
            INST_NAME("PSLLD Gm,Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0, 1);
            ADDI(x4, xZR, 31);
            LD(x1, wback, fixedaddress + 0);
            BLTU_MARK(x4, x1);
            LW(x3, gback, gdoffset + 0 * 4);
            LW(x4, gback, gdoffset + 1 * 4);
            SLL(x3, x3, x1);
            SLL(x4, x4, x1);
            SW(x3, gback, gdoffset + 0 * 4);
            SW(x4, gback, gdoffset + 1 * 4);
            B_NEXT_nocond;
            MARK;
            SD(xZR, gback, gdoffset + 0);
            break;
        case 0xF3:
            INST_NAME("PSLLQ Gm,Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0, 1);
            ADDI(x4, xZR, 63);
            LD(x1, gback, gdoffset + 0);
            LD(x3, wback, fixedaddress + 0);
            BLTU_MARK(x4, x3);
            SLL(x1, x1, x3);
            SD(x1, gback, gdoffset + 0);
            B_NEXT_nocond;
            MARK;
            SD(xZR, gback, gdoffset + 0);
            break;
        case 0xF4:
            INST_NAME("PMULUDQ Gm,Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0, 1);
            LWU(x3, gback, gdoffset + 0 * 4);
            LWU(x4, wback, fixedaddress + 0 * 4);
            MUL(x3, x3, x4);
            SD(x3, gback, gdoffset + 0);
            break;
        case 0xF5:
            INST_NAME("PMADDWD Gm, Em");
            nextop = F8;
            GETGM();
            GETEM(x5, 0, 6);
            for (int i = 0; i < 2; ++i) {
                LH(x1, gback, gdoffset + i * 4);
                LH(x2, gback, gdoffset + i * 4 + 2);
                LH(x3, wback, fixedaddress + i * 4);
                LH(x4, wback, fixedaddress + i * 4 + 2);
                MULW(x1, x1, x3);
                MULW(x2, x2, x4);
                ADDW(x1, x1, x2);
                SW(x1, gback, gdoffset + i * 4);
            }
            break;
        case 0xF6:
            INST_NAME("PSADBW Gm, Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0, 7);
            MV(x6, xZR);
            for (int i = 0; i < 8; ++i) {
                LBU(x3, gback, gdoffset + i);
                LBU(x4, wback, fixedaddress + i);
                SUBW(x3, x3, x4);
                if (cpuext.zbb) {
                    NEG(x5, x3);
                    MAX(x3, x3, x5);
                } else {
                    SRAIW(x5, x3, 31);
                    XOR(x3, x5, x3);
                    SUBW(x3, x3, x5);
                }
                ADDW(x6, x6, x3);
                if (i == 7) { SD(x6, gback, gdoffset + 0); }
            }
            break;
        case 0xF7:
            INST_NAME("MASKMOVQ Gm, Em");
            UP32_READ(xRDI);
            nextop = F8;
            GETGM();
            GETEM(x5, 0, 7);
            for (int i = 0; i < 8; i++) {
                LB(x1, wback, fixedaddress + i);
                BLE(xZR, x1, 4 * 3);
                LB(x2, gback, gdoffset + i);
                SB(x2, xRDI, i);
            }
            break;
        case 0xF8:
            INST_NAME("PSUBB Gm, Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0, 7);
            for (int i = 0; i < 8; ++i) {
                LB(x3, wback, fixedaddress + i);
                LB(x4, gback, gdoffset + i);
                SUB(x3, x4, x3);
                SB(x3, gback, gdoffset + i);
            }
            break;
        case 0xF9:
            INST_NAME("PSUBW Gm, Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0, 6);
            MMX_LOOP_W(x3, x4, SUBW(x3, x3, x4));
            break;
        case 0xFA:
            INST_NAME("PSUBD Gm, Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0, 4);
            MMX_LOOP_D(x3, x4, SUBW(x3, x3, x4));
            break;
        case 0xFB:
            INST_NAME("PSUBQ Gm, Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0, 1);
            LD(x1, gback, gdoffset + 0);
            LD(x3, wback, fixedaddress + 0);
            SUB(x1, x1, x3);
            SD(x1, gback, gdoffset + 0);
            break;
        case 0xFC:
            INST_NAME("PADDB Gm, Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0, 7);
            for (int i = 0; i < 8; ++i) {
                // GM->sb[i] += EM->sb[i];
                LB(x3, gback, gdoffset + i);
                LB(x4, wback, fixedaddress + i);
                ADDW(x3, x3, x4);
                SB(x3, gback, gdoffset + i);
            }
            break;
        case 0xFD:
            INST_NAME("PADDW Gm, Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0, 6);
            MMX_LOOP_W(x3, x4, ADDW(x3, x3, x4));
            break;
        case 0xFE:
            INST_NAME("PADDD Gm, Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0, 4);
            MMX_LOOP_D(x3, x4, ADDW(x3, x3, x4));
            break;
        default: DEFAULT;
    }
    return addr;
}
