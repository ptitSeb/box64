#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "dynarec.h"
#include "emu/x64emu_private.h"
#include "emu/x64run_private.h"
#include "x64run.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_functions.h"
#include "dynarec_rv64_helper.h"

uintptr_t dynarec64_F20F(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip; (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop;
    uint8_t gd, ed;
    uint8_t wb1, wback, wb2, gback;
    uint8_t u8;
    uint64_t u64, j64;
    int v0, v1;
    int q0;
    int d0, d1;
    int s0, s1;
    int64_t fixedaddress, gdoffset;
    int unscaled;

    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(q0);
    MAYUSE(v0);
    MAYUSE(v1);

    switch(opcode) {

        case 0x10:
            INST_NAME("MOVSD Gx, Ex");
            nextop = F8;
            GETG;
            if(MODREG) {
                ed = (nextop&7)+ (rex.b<<3);
                v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
                d0 = sse_get_reg(dyn, ninst, x1, ed, 0);
                FMVD(v0, d0);
            } else {
                SMREAD();
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd, 0);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 8, 0);
                FLD(v0, ed, fixedaddress);
                // reset upper part
                SD(xZR, xEmu, offsetof(x64emu_t, xmm[gd])+8);
            }
            break;
        case 0x11:
            INST_NAME("MOVSD Ex, Gx");
            nextop = F8;
            GETG;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
            if(MODREG) {
                ed = (nextop&7)+ (rex.b<<3);
                d0 = sse_get_reg(dyn, ninst, x1, ed, 0);
                FMVD(d0, v0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                FSD(v0, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0x12:
            INST_NAME("MOVDDUP Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            LD(x3, wback, fixedaddress+0);
            SD(x3, gback, gdoffset+0);
            SD(x3, gback, gdoffset+8);
            break;
        case 0x2A:
            INST_NAME("CVTSI2SD Gx, Ed");
            nextop = F8;
            GETGXSD(v0);
            GETED(0);
            if(rex.w) {
                FCVTDL(v0, ed, RD_RNE);
            } else {
                FCVTDW(v0, ed, RD_RNE);
            }
            break;
        case 0x2C:
            INST_NAME("CVTTSD2SI Gd, Ex");
            nextop = F8;
            GETGD;
            GETEXSD(v0, 0);
            if(!box64_dynarec_fastround) {
                FSFLAGSI(0);  // // reset all bits
            }
            FCVTLDxw(gd, v0, RD_RTZ);
            if(!rex.w)
                ZEROUP(gd);
            if(!box64_dynarec_fastround) {
                FRFLAGS(x5);   // get back FPSR to check the IOC bit
                ANDI(x5, x5, (1<<FR_NV)|(1<<FR_OF));
                CBZ_NEXT(x5);
                if(rex.w) {
                    MOV64x(gd, 0x8000000000000000LL);
                } else {
                    MOV32w(gd, 0x80000000);
                }
            }
            break;
        case 0x2D:
            INST_NAME("CVTSD2SI Gd, Ex");
            nextop = F8;
            GETGD;
            GETEXSD(v0, 0);
            if(!box64_dynarec_fastround) {
                FSFLAGSI(0);  // // reset all bits
            }
            u8 = sse_setround(dyn, ninst, x2, x3);
            FCVTLDxw(gd, v0, RD_DYN);
            x87_restoreround(dyn, ninst, u8);
            if(!box64_dynarec_fastround) {
                FRFLAGS(x5);   // get back FPSR to check the IOC bit
                ANDI(x5, x5, (1<<FR_NV)|(1<<FR_OF));
                CBZ_NEXT(x5);
                if(rex.w) {
                    MOV64x(gd, 0x8000000000000000LL);
                } else {
                    MOV32w(gd, 0x80000000);
                }
            }
            break;
        case 0x38:  // these are some more SSSE4.2+ opcodes
            opcode = F8;
            switch(opcode) {

                case 0xF0:  // CRC32 Gd, Eb
                    INST_NAME("CRC32 Gd, Eb");
                    nextop = F8;
                    GETEB(x1, 0);
                    GETGD;
                    XOR(gd, gd, ed);
                    MOV32w(x2, 0x82f63b78);
                    for (int i = 0; i < 8; i++) {
                        SRLI((i&1)?gd:x4, (i&1)?x4:gd, 1);
                        ANDI(x6, (i&1)?x4:gd, 1);
                        BEQZ(x6, 4+4);
                        XOR((i&1)?gd:x4, (i&1)?gd:x4, x2);
                    }
                    break;
                case 0xF1:  // CRC32 Gd, Ed
                    INST_NAME("CRC32 Gd, Ed");
                    nextop = F8;
                    GETGD;
                    GETED(0);
                    MOV32w(x2, 0x82f63b78);
                    for(int j=0; j<4*(rex.w+1); ++j) {
                        SRLI(x5, ed, 8*j);
                        ANDI(x3, x5, 0xFF);
                        XOR(gd, gd, x3);
                        for (int i = 0; i < 8; i++) {
                            SRLI((i&1)?gd:x4, (i&1)?x4:gd, 1);
                            ANDI(x6, (i&1)?x4:gd, 1);
                            BEQZ(x6, 4+4);
                            XOR((i&1)?gd:x4, (i&1)?gd:x4, x2);
                        }
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0x51:
            INST_NAME("SQRTSD Gx, Ex");
            nextop = F8;
            GETEXSD(d0, 0);
            GETGXSD_empty(d1);
            if(!box64_dynarec_fastnan) {
                v0 = fpu_get_scratch(dyn);  // need a scratch in case d0 == d1
                FMVDX(v0, xZR);
                FLTD(x3, d0, v0);
            }
            FSQRTD(d1, d0);
            if(!box64_dynarec_fastnan) {
                BEQ(x3, xZR, 8);
                FNEGD(d1, d1);
            }
            break;
        case 0x58:
            INST_NAME("ADDSD Gx, Ex");
            nextop = F8;
            GETGXSD(v0);
            GETEXSD(v1, 0);
            if(!box64_dynarec_fastnan) {
                FEQD(x3, v0, v0);
                FEQD(x4, v1, v1);
            }
            FADDD(v0, v0, v1);
            if(!box64_dynarec_fastnan) {
                AND(x3, x3, x4);
                CBZ_NEXT(x3);
                FEQD(x3, v0, v0);
                CBNZ_NEXT(x3);
                FNEGD(v0, v0);
            }
            break;
        case 0x59:
            INST_NAME("MULSD Gx, Ex");
            nextop = F8;
            GETGXSD(v0);
            GETEXSD(v1, 0);
            if(!box64_dynarec_fastnan) {
                FEQD(x3, v0, v0);
                FEQD(x4, v1, v1);
            }
            FMULD(v0, v0, v1);
            if(!box64_dynarec_fastnan) {
                AND(x3, x3, x4);
                CBZ_NEXT(x3);
                FEQD(x3, v0, v0);
                CBNZ_NEXT(x3);
                FNEGD(v0, v0);
            }
            break;
        case 0x5A:
            INST_NAME("CVTSD2SS Gx, Ex");
            nextop = F8;
            GETEXSD(v1, 0);
            GETGXSS_empty(v0);
            FCVTSD(v0, v1);
            break;
        case 0x5C:
            INST_NAME("SUBSD Gx, Ex");
            nextop = F8;
            GETGXSD(v0);
            GETEXSD(v1, 0);
            if(!box64_dynarec_fastnan) {
                FEQD(x3, v0, v0);
                FEQD(x4, v1, v1);
            }
            FSUBD(v0, v0, v1);
            if(!box64_dynarec_fastnan) {
                AND(x3, x3, x4);
                CBZ_NEXT(x3);
                FEQD(x3, v0, v0);
                CBNZ_NEXT(x3);
                FNEGD(v0, v0);
            }
            break;
        case 0x5D:
            INST_NAME("MINSD Gx, Ex");
            nextop = F8;
            GETGXSD(v0);
            GETEXSD(v1, 0);
            FEQD(x2, v0, v0);
            FEQD(x3, v1, v1);
            AND(x2, x2, x3);
            BEQ_MARK(x2, xZR);
            FLED(x2, v1, v0);
            BEQ_MARK2(x2, xZR);
            MARK;
            FMVD(v0, v1);
            MARK2;
            break;
        case 0x5E:
            INST_NAME("DIVSD Gx, Ex");
            nextop = F8;
            GETGXSD(v0);
            GETEXSD(v1, 0);
            if(!box64_dynarec_fastnan) {
                FEQD(x3, v0, v0);
                FEQD(x4, v1, v1);
            }
            FDIVD(v0, v0, v1);
            if(!box64_dynarec_fastnan) {
                AND(x3, x3, x4);
                CBZ_NEXT(x3);
                FEQD(x3, v0, v0);
                CBNZ_NEXT(x3);
                FNEGD(v0, v0);
            }
            break;
        case 0x5F:
            INST_NAME("MAXSD Gx, Ex");
            nextop = F8;
            GETGXSD(v0);
            GETEXSD(v1, 0);
            FEQD(x2, v0, v0);
            FEQD(x3, v1, v1);
            AND(x2, x2, x3);
            BEQ_MARK(x2, xZR);
            FLED(x2, v0, v1);
            BEQ_MARK2(x2, xZR);
            MARK;
            FMVD(v0, v1);
            MARK2;
            break;
        case 0x70: // TODO: Optimize this!
            INST_NAME("PSHUFLW Gx, Ex, Ib");
            nextop = F8;
            GETGX();
            GETEX(x2, 1);
            u8 = F8;
            int32_t idx;

            idx = (u8>>(0*2))&3;
            LHU(x3, wback, fixedaddress+idx*2);
            idx = (u8>>(1*2))&3;
            LHU(x4, wback, fixedaddress+idx*2);
            idx = (u8>>(2*2))&3;
            LHU(x5, wback, fixedaddress+idx*2);
            idx = (u8>>(3*2))&3;
            LHU(x6, wback, fixedaddress+idx*2);

            SH(x3, gback, gdoffset+0*2);
            SH(x4, gback, gdoffset+1*2);
            SH(x5, gback, gdoffset+2*2);
            SH(x6, gback, gdoffset+3*2);

            if (!(MODREG && (gd==ed))) {
                LD(x3, wback, fixedaddress+8);
                SD(x3, gback, gdoffset+8);
            }
            break;
        case 0x7C:
            INST_NAME("HADDPS Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            s0 = fpu_get_scratch(dyn);
            s1 = fpu_get_scratch(dyn);
            // GX->f[0] += GX->f[1];
            FLW(s0, gback, gdoffset + 0);
            FLW(s1, gback, gdoffset + 4);
            FADDS(s0, s0, s1);
            FSW(s0, gback, gdoffset + 0);
            // GX->f[1] = GX->f[2] + GX->f[3];
            FLW(s0, gback, gdoffset + 8);
            FLW(s1, gback, gdoffset + 12);
            FADDS(s0, s0, s1);
            FSW(s0, gback, gdoffset + 4);
            if (MODREG && gd == (nextop & 7) + (rex.b << 3)) {
                // GX->f[2] = GX->f[0];
                FLW(s1, gback, gdoffset + 0);
                FSW(s1, gback, gdoffset + 8);
                // GX->f[3] = GX->f[1];
                FSW(s0, gback, gdoffset + 12);
            } else {
                // GX->f[2] = EX->f[0] + EX->f[1];
                FLW(s0, wback, fixedaddress + 0);
                FLW(s1, wback, fixedaddress + 4);
                FADDS(s0, s0, s1);
                FSW(s0, gback, gdoffset + 8);
                // GX->f[3] = EX->f[2] + EX->f[3];
                FLW(s0, wback, fixedaddress + 8);
                FLW(s1, wback, fixedaddress + 12);
                FADDS(s0, s0, s1);
                FSW(s0, gback, gdoffset + 12);
            }
            break;
        case 0xC2:
            INST_NAME("CMPSD Gx, Ex, Ib");
            nextop = F8;
            GETGXSD(d0);
            GETEXSD(d1, 1);
            u8 = F8;
            if ((u8&7) == 0) {                                      // Equal
                FEQD(x2, d0, d1);
            } else if ((u8&7) == 4) {                               // Not Equal or unordered
                FEQD(x2, d0, d1);
                XORI(x2, x2, 1);
            } else {
                // x2 = !(isnan(d0) || isnan(d1))
                FEQD(x3, d0, d0);
                FEQD(x2, d1, d1);
                AND(x2, x2, x3);

                switch(u8&7) {
                case 1: BEQ_MARK(x2, xZR); FLTD(x2, d0, d1); break; // Less than
                case 2: BEQ_MARK(x2, xZR); FLED(x2, d0, d1); break; // Less or equal
                case 3: XORI(x2, x2, 1); break;                     // NaN
                case 5: {                                           // Greater or equal or unordered
                    BEQ_MARK2(x2, xZR);
                    FLED(x2, d1, d0);
                    B_MARK_nocond;
                    break;
                }
                case 6: {                                           // Greater or unordered, test inverted, N!=V so unordered or less than (inverted)
                    BEQ_MARK2(x2, xZR);
                    FLTD(x2, d1, d0);
                    B_MARK_nocond;
                    break;
                }
                case 7: break;                                      // Not NaN
                }

                MARK2;
                if ((u8&7) == 5 || (u8&7) == 6) {
                    MOV32w(x2, 1);
                }
                MARK;
            }
            NEG(x2, x2);
            FMVDX(d0, x2);
            break;
        case 0xE6:
            INST_NAME("CVTPD2DQ Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            d0 = fpu_get_scratch(dyn);
            u8 = sse_setround(dyn, ninst, x6, x4);
            for (int i=0; i<2 ; ++i) {
                FLD(d0, wback, fixedaddress+8*i);
                FCVTLD(x3, d0, RD_DYN);
                SEXT_W(x5, x3);
                BEQ(x5, x3, 8);
                LUI(x3, 0x80000); // INT32_MIN
                SW(x3, gback, gdoffset+4*i);
            }
            x87_restoreround(dyn, ninst, u8);
            SD(xZR, gback, gdoffset+8);
            break;
        case 0xF0:
            INST_NAME("LDDQU Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            SSE_LOOP_MV_Q(x3);
            break;
        default:
            DEFAULT;
    }
    return addr;
}
