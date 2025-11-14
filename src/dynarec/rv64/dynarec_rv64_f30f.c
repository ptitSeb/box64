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
#include "bitutils.h"

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_functions.h"
#include "../dynarec_helper.h"

uintptr_t dynarec64_F30F(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed;
    uint8_t wback, gback;
    uint64_t u64;
    int v0, v1;
    int q0, q1;
    int d0, d1;
    int64_t fixedaddress, gdoffset;
    int unscaled;
    int64_t j64;

    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(v0);
    MAYUSE(v1);
    MAYUSE(j64);

    switch (opcode) {
        case 0x10:
            INST_NAME("MOVSS Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG) {
                v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
                q0 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 1);
                FMVS(v0, q0);
            } else {
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd, 1);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                FLW(v0, ed, fixedaddress);
                // reset upper part
                SW(xZR, xEmu, offsetof(x64emu_t, xmm[gd]) + 4);
                SD(xZR, xEmu, offsetof(x64emu_t, xmm[gd]) + 8);
            }
            break;
        case 0x11:
            INST_NAME("MOVSS Ex, Gx");
            nextop = F8;
            GETG;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
            if (MODREG) {
                q0 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 1);
                FMVS(q0, v0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                FSW(v0, ed, fixedaddress);
                SMWRITE2();
            }
            break;

        case 0x12:
            INST_NAME("MOVSLDUP Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 8);

            // GX->ud[1] = GX->ud[0] = EX->ud[0];
            // GX->ud[3] = GX->ud[2] = EX->ud[2];
            LW(x3, wback, fixedaddress + 0);
            SW(x3, gback, gdoffset + 0);
            SW(x3, gback, gdoffset + 4);
            LW(x3, wback, fixedaddress + 8);
            SW(x3, gback, gdoffset + 8);
            SW(x3, gback, gdoffset + 12);
            break;
        case 0x16:
            INST_NAME("MOVSHDUP Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 12);

            // GX->ud[1] = GX->ud[0] = EX->ud[1];
            // GX->ud[3] = GX->ud[2] = EX->ud[3];
            LW(x3, wback, fixedaddress + 4);
            SW(x3, gback, gdoffset + 0);
            SW(x3, gback, gdoffset + 4);
            LW(x3, wback, fixedaddress + 12);
            SW(x3, gback, gdoffset + 8);
            SW(x3, gback, gdoffset + 12);
            break;
        case 0x1E:
            INST_NAME("NOP / ENDBR32 / ENDBR64");
            nextop = F8;
            FAKEED;
            break;

        case 0x2A:
            INST_NAME("CVTSI2SS Gx, Ed");
            nextop = F8;
            GETGXSS(v0);
            GETED(0);
            if (rex.w) {
                FCVTSL(v0, ed, RD_RNE);
            } else {
                FCVTSW(v0, ed, RD_RNE);
            }
            break;

        case 0x2C:
            INST_NAME("CVTTSS2SI Gd, Ex");
            nextop = F8;
            GETGD;
            GETEXSS(d0, 0);
            if (!BOX64ENV(dynarec_fastround)) {
                FSFLAGSI(0); // // reset all bits
            }
            FCVTSxw(gd, d0, RD_RTZ);
            if (!rex.w)
                ZEROUP(gd);
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
            INST_NAME("CVTSS2SI Gd, Ex");
            nextop = F8;
            GETGD;
            GETEXSS(d0, 0);
            if (!BOX64ENV(dynarec_fastround)) {
                FSFLAGSI(0); // // reset all bits
            }
            u8 = sse_setround(dyn, ninst, x5, x6);
            FCVTSxw(gd, d0, RD_DYN);
            x87_restoreround(dyn, ninst, u8);
            if (!rex.w)
                ZEROUP(gd);
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

        case 0x51:
            INST_NAME("SQRTSS Gx, Ex");
            nextop = F8;
            GETEXSS(v0, 0);
            GETGXSS_empty(v1);
            FSQRTS(v1, v0);
            break;
        case 0x52:
            INST_NAME("RSQRTSS Gx, Ex");
            nextop = F8;
            GETEXSS(v0, 0);
            GETGXSS_empty(v1);
            q0 = fpu_get_scratch(dyn);
            LUI(x3, 0x3F800); // 1.0f
            FMVWX(q0, x3);
            FSQRTS(v1, v0);
            FDIVS(v1, q0, v1);
            break;
        case 0x53:
            INST_NAME("RCPSS Gx, Ex");
            nextop = F8;
            GETEXSS(v0, 0);
            GETGXSS_empty(v1);
            q0 = fpu_get_scratch(dyn);
            LUI(x3, 0x3F800); // 1.0f
            FMVWX(q0, x3);
            FDIVS(v1, q0, v0);
            break;
        case 0x58:
            INST_NAME("ADDSS Gx, Ex");
            nextop = F8;
            GETGXSS(v0);
            GETEXSS(d0, 0);
            if (!BOX64ENV(dynarec_fastnan)) {
                FEQS(x3, v0, v0);
                FEQS(x4, d0, d0);
            }
            FADDS(v0, v0, d0);
            if (!BOX64ENV(dynarec_fastnan)) {
                AND(x3, x3, x4);
                BNEZ_MARK(x3);
                CBNZ_NEXT(x4);
                FMVS(v0, d0);
                MARK;
            }
            break;
        case 0x59:
            INST_NAME("MULSS Gx, Ex");
            nextop = F8;
            GETGXSS(v0);
            GETEXSS(d0, 0);
            if (!BOX64ENV(dynarec_fastnan)) {
                FEQS(x3, v0, v0);
                FEQS(x4, d0, d0);
            }
            FMULS(v0, v0, d0);
            if (!BOX64ENV(dynarec_fastnan)) {
                AND(x3, x3, x4);
                BNEZ_MARK(x3);
                CBNZ_NEXT(x4);
                FMVS(v0, d0);
                B_NEXT_nocond;
                MARK;
                FEQS(x3, v0, v0);
                CBNZ_NEXT(x3);
                FNEGS(v0, v0);
            }
            break;
        case 0x5A:
            INST_NAME("CVTSS2SD Gx, Ex");
            nextop = F8;
            gd = ((nextop & 0x38) >> 3) + (rex.r << 3);
            GETEXSS(v1, 0);
            if (MODREG && gd == (nextop & 7) + (rex.b << 3)) {
                v0 = sse_get_reg_size_changed(dyn, ninst, x2, gd, 0);
                FCVTDS(v0, v0);
            } else {
                GETGXSD_empty(v0);
                FCVTDS(v0, v1);
            }
            break;
        case 0x5B:
            INST_NAME("CVTTPS2DQ Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 12);
            v0 = fpu_get_scratch(dyn);
            for (int i = 0; i < 4; ++i) {
                if (!BOX64ENV(dynarec_fastround)) {
                    FSFLAGSI(0); // reset all bits
                }
                FLW(v0, wback, fixedaddress + i * 4);
                FCVTWS(x3, v0, RD_RTZ);
                if (!BOX64ENV(dynarec_fastround)) {
                    FRFLAGS(x5); // get back FPSR to check the IOC bit
                    ANDI(x5, x5, (1 << FR_NV) | (1 << FR_OF));
                    BEQZ(x5, 8);
                    MOV32w(x3, 0x80000000);
                }
                SW(x3, gback, gdoffset + i * 4);
            }
            break;
        case 0x5C:
            INST_NAME("SUBSS Gx, Ex");
            nextop = F8;
            GETGXSS(v0);
            GETEXSS(d0, 0);
            if (!BOX64ENV(dynarec_fastnan)) {
                FEQS(x3, v0, v0);
                FEQS(x4, d0, d0);
            }
            FSUBS(v0, v0, d0);
            if (!BOX64ENV(dynarec_fastnan)) {
                AND(x3, x3, x4);
                BNEZ_MARK(x3);
                CBNZ_NEXT(x4);
                FMVS(v0, d0);
                MARK;
            }
            break;
        case 0x5D:
            INST_NAME("MINSS Gx, Ex");
            nextop = F8;
            GETGXSS(d0);
            GETEXSS(d1, 0);
            FEQS(x2, d0, d0);
            FEQS(x3, d1, d1);
            AND(x2, x2, x3);
            BEQ_MARK(x2, xZR);
            FLES(x2, d1, d0);
            BEQ_MARK2(x2, xZR);
            MARK;
            FMVS(d0, d1);
            MARK2;
            break;
        case 0x5E:
            INST_NAME("DIVSS Gx, Ex");
            nextop = F8;
            GETGXSS(v0);
            GETEXSS(d0, 0);
            if (!BOX64ENV(dynarec_fastnan)) {
                FEQS(x3, v0, v0);
                FEQS(x4, d0, d0);
            }
            FDIVS(v0, v0, d0);
            if (!BOX64ENV(dynarec_fastnan)) {
                AND(x3, x3, x4);
                BNEZ_MARK(x3);
                CBNZ_NEXT(x4);
                FMVS(v0, d0);
                MARK;
            }
            break;
        case 0x5F:
            INST_NAME("MAXSS Gx, Ex");
            nextop = F8;
            GETGXSS(d0);
            GETEXSS(d1, 0);
            FEQS(x2, d0, d0);
            FEQS(x3, d1, d1);
            AND(x2, x2, x3);
            BEQ_MARK(x2, xZR);
            FLES(x2, d0, d1);
            BEQ_MARK2(x2, xZR);
            MARK;
            FMVS(d0, d1);
            MARK2;
            break;
        case 0x6F:
            INST_NAME("MOVDQU Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 8);
            SSE_LOOP_MV_Q(x3);
            break;
        case 0x70: // TODO: Optimize this!
            INST_NAME("PSHUFHW Gx, Ex, Ib");
            nextop = F8;
            GETGX();
            GETEX(x2, 1, 14);
            u8 = F8;
            int32_t idx;

            idx = 4 + ((u8 >> (0 * 2)) & 3);
            LHU(x3, wback, fixedaddress + idx * 2);
            idx = 4 + ((u8 >> (1 * 2)) & 3);
            LHU(x4, wback, fixedaddress + idx * 2);
            idx = 4 + ((u8 >> (2 * 2)) & 3);
            LHU(x5, wback, fixedaddress + idx * 2);
            idx = 4 + ((u8 >> (3 * 2)) & 3);
            LHU(x6, wback, fixedaddress + idx * 2);

            SH(x3, gback, gdoffset + (4 + 0) * 2);
            SH(x4, gback, gdoffset + (4 + 1) * 2);
            SH(x5, gback, gdoffset + (4 + 2) * 2);
            SH(x6, gback, gdoffset + (4 + 3) * 2);

            if (!(MODREG && (gd == ed))) {
                LD(x3, wback, fixedaddress + 0);
                SD(x3, gback, gdoffset + 0);
            }
            break;
        case 0x7E:
            INST_NAME("MOVQ Gx, Ex");
            nextop = F8;
            // Will load Gx as SD. Is that a good choice?
            if (MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);
                GETGXSD_empty(v0);
                FMVD(v0, v1);
            } else {
                GETGXSD_empty(v0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                FLD(v0, ed, fixedaddress);
            }
            SD(xZR, xEmu, offsetof(x64emu_t, xmm[gd]) + 8);
            break;
        case 0x7F:
            INST_NAME("MOVDQU Ex,Gx");
            nextop = F8;
            GETGX();
            IF_UNALIGNED(ip) {
                GETEX(x2, 0, 15);
                LD(x3, gback, gdoffset + 0);
                LD(x4, gback, gdoffset + 8);
                for (int i = 0; i < 8; i++) {
                    if (i == 0) {
                        SB(x3, wback, fixedaddress);
                    } else {
                        SRLI(x5, x3, i * 8);
                        SB(x5, wback, fixedaddress + i);
                    }
                }
                for (int i = 0; i < 8; i++) {
                    if (i == 0) {
                        SB(x4, wback, fixedaddress + 8);
                    } else {
                        SRLI(x5, x4, i * 8);
                        SB(x5, wback, fixedaddress + i + 8);
                    }
                }
            } else {
                GETEX(x2, 0, 8);
                SSE_LOOP_MV_Q2(x3);
            }
            if (!MODREG) SMWRITE2();
            break;
        case 0xAE:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                case 1:
                    if(rex.is32bits || !MODREG) {
                        INST_NAME("Illegal AE");
                        FAKEED;
                        UDF();
                    } else {
                        if(((nextop>>3)&7)==1) {INST_NAME("RDGSBASE");} else {INST_NAME("RDFSBASE");}
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        int seg = _FS + ((nextop>>3)&7);
                        grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
                        MV(ed, x4);
                    }
                     break;
                case 2:
                case 3:
                    if(rex.is32bits || !MODREG) {
                        INST_NAME("Illegal AE");
                        FAKEED;
                        UDF();
                    } else {
                        if(((nextop>>3)&7)==3) {INST_NAME("WRGSBASE");} else {INST_NAME("WRFSBASE");}
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        int seg = _FS + ((nextop>>3)&7)-2;
                        SD(ed, xEmu, offsetof(x64emu_t, segs_offs[seg]));
                    }
                    break;
                case 5:
                    INST_NAME("(unsupported) INCSSPD/INCSSPQ Ed");
                    FAKEED;
                    UDF();
                    break;
                case 6:
                    INST_NAME("(unsupported) UMONITOR Ed");
                    FAKEED;
                    UDF();
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xB8:
            INST_NAME("POPCNT Gd, Ed");
            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            nextop = F8;
            GETED(0);
            GETGD;
            if (!rex.w && MODREG) {
                ZEXTW2(x4, ed);
                ed = x4;
            }
            CLEAR_FLAGS();
            BNE_MARK(ed, xZR);
            ORI(xFlags, xFlags, 1 << F_ZF);
            MOV32w(gd, 0);
            B_NEXT_nocond;
            MARK;
            if (cpuext.zbb) {
                CPOPxw(gd, ed);
            } else {
                if (rex.w) {
                    // x7 = 0x5555555555555555
                    LUI(x7, 0x55555);
                    ADDIW(x7, x7, 0x555);
                    SLLI(x6, x7, 32);
                    ADD(x7, x7, x6);

                    // v = v - ((v >> 1) & x7)
                    SRLI(x5, ed, 1);
                    AND(x5, x5, x7);
                    SUB(x5, ed, x5);

                    // x3 = 0x3333333333333333
                    LUI(x3, 0x33333);
                    ADDIW(x3, x3, 0x333);
                    SLLI(x6, x3, 32);
                    ADD(x3, x3, x6);

                    // v = (v & x3) + ((v >> 2) & x3);
                    SRLI(x7, x5, 2);
                    AND(x7, x7, x3);
                    AND(x5, x5, x3);
                    ADD(x5, x5, x7);

                    // x3 = 0x0F0F0F0F0F0F0F0F
                    LUI(x3, 0xF0F1);
                    ADDIW(x3, x3, 0xF0F);
                    SLLI(x6, x3, 32);
                    ADD(x3, x3, x6);

                    // v = (v + (v >> 4) & x3)
                    SRLI(x7, x5, 4);
                    ADD(x5, x5, x7);
                    AND(x5, x5, x3);

                    // x3 = 0x0101010101010101
                    LUI(x3, 0x1010);
                    ADDIW(x3, x3, 0x101);
                    SLLI(x6, x3, 32);
                    ADD(x3, x3, x6);

                    // count = (v * x3) >> 56
                    MUL(gd, x5, x3);
                    SRLI(gd, gd, 56);
                } else {
                    // x7 = 0x55555555uLL
                    LUI(x7, 0x55555);
                    ADDIW(x7, x7, 0x555);

                    // v = v - ((v >> 1) & x7)
                    SRLI(x5, ed, 1);
                    AND(x5, x5, x7);
                    SUB(x5, ed, x5);

                    // x3 = 0x33333333uLL
                    LUI(x3, 0x33333);
                    ADDIW(x3, x3, 0x333);

                    // v = (v & x3) + ((v >> 2) & x3);
                    SRLI(x7, x5, 2);
                    AND(x7, x7, x3);
                    AND(x5, x5, x3);
                    ADD(x5, x5, x7);

                    // x3 = 0x0F0F0F0FuLL
                    LUI(x3, 0xF0F1);
                    ADDIW(x3, x3, 0xF0F);

                    // v = (v + (v >> 4) & x3)
                    SRLI(x7, x5, 4);
                    ADD(x5, x5, x7);
                    AND(x5, x5, x3);

                    // x3 = 01010101uLL
                    LUI(x3, 0x1010);
                    ADDIW(x3, x3, 0x101);

                    // count = (v * x3) >> 24
                    MULW(gd, x5, x3);
                    SRLIW(gd, gd, 24);
                }
            }
            break;
        case 0xBC:
            INST_NAME("TZCNT Gd, Ed");
            if (!BOX64DRENV(dynarec_safeflags)) {
                SETFLAGS(X_CF | X_ZF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            } else {
                SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            }
            SET_DFNONE();
            nextop = F8;
            GETED(0);
            GETGD;
            CLEAR_FLAGS();
            if (!rex.w && MODREG) {
                ZEXTW2(x4, ed);
                ed = x4;
            }
            IFX (X_CF) SET_FLAGS_EQZ(ed, F_CF, x3);
            BNEZ_MARK(ed);
            IFX (X_ZF) ANDI(xFlags, xFlags, ~(1 << F_ZF));
            MOV32w(gd, rex.w ? 64 : 32);
            B_NEXT_nocond;
            MARK;
            CTZxw(gd, ed, rex.w, x3, x5);
            IFX (X_ZF) SET_FLAGS_EQZ(gd, F_ZF, x3);
            break;
        case 0xBD:
            INST_NAME("LZCNT Gd, Ed");
            if (!BOX64DRENV(dynarec_safeflags)) {
                SETFLAGS(X_CF | X_ZF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            } else {
                SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            }
            SET_DFNONE();
            nextop = F8;
            GETED(0);
            GETGD;
            CLEAR_FLAGS();
            if (!rex.w && MODREG) {
                ZEXTW2(x4, ed);
                ed = x4;
            }
            IFX (X_CF) SET_FLAGS_EQZ(ed, F_CF, x3);
            BNEZ_MARK(ed);
            MOV32w(gd, rex.w ? 64 : 32);
            IFX (X_ZF) ANDI(xFlags, xFlags, ~(1 << F_ZF));
            B_NEXT_nocond;
            MARK;
            CLZxw(gd, ed, rex.w, x5, x2, x3);
            IFX (X_ZF) SET_FLAGS_EQZ(gd, F_ZF, x3);
            break;

        case 0xC2:
            INST_NAME("CMPSS Gx, Ex, Ib");
            nextop = F8;
            GETGXSS(d0);
            GETEXSS(d1, 1);
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
            FMVWX(d0, x2);
            break;

        case 0xE6:
            INST_NAME("CVTDQ2PD Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 4);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            LW(x3, wback, fixedaddress + 0);
            LW(x4, wback, fixedaddress + 4);
            FCVTDW(q0, x3, RD_RTZ);
            FCVTDW(q1, x4, RD_RTZ);
            FSD(q0, gback, gdoffset + 0);
            FSD(q1, gback, gdoffset + 8);
            break;

        default:
            DEFAULT;
    }
    return addr;
}
