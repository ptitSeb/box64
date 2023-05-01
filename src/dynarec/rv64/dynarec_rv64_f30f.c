#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
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
#include "bitutils.h"

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_functions.h"
#include "dynarec_rv64_helper.h"

uintptr_t dynarec64_F30F(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip; (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed;
    uint8_t wback, gback;
    uint64_t u64;
    int v0, v1;
    int q0, q1;
    int d0, d1;
    int64_t fixedaddress;
    int unscaled;
    int64_t j64;

    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(v0);
    MAYUSE(v1);
    MAYUSE(j64);

    switch(opcode) {
        case 0x10:
            INST_NAME("MOVSS Gx, Ex");
            nextop = F8;
            GETG;
            if(MODREG) {
                v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
                q0 = sse_get_reg(dyn, ninst, x1, (nextop&7) + (rex.b<<3), 1);
                FMVS(v0, q0);
            } else {
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd, 1);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 8, 0);
                FLW(v0, ed, fixedaddress);
                // reset upper part
                SW(xZR, xEmu, offsetof(x64emu_t, xmm[gd])+4);
                SD(xZR, xEmu, offsetof(x64emu_t, xmm[gd])+8);
            }
            break;
        case 0x11:
            INST_NAME("MOVSS Ex, Gx");
            nextop = F8;
            GETG;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
            if(MODREG) {
                q0 = sse_get_reg(dyn, ninst, x1, (nextop&7) + (rex.b<<3), 1);
                FMVS(q0, v0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                FSW(v0, ed, fixedaddress);
                SMWRITE2();
            }
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
            if(rex.w) {
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
            if(!box64_dynarec_fastround) {
                FSFLAGSI(xZR);  // // reset all bits
            }
            FCVTSxw(gd, d0, RD_RTZ);
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
        case 0x51:
            INST_NAME("SQRTSS Gx, Ex");
            nextop = F8;
            GETEXSS(v0, 0);
            GETGXSS_empty(v1);
            FSQRTS(v1, v0);
            break;
        case 0x58:
            INST_NAME("ADDSS Gx, Ex");
            nextop = F8;
            GETGXSS(v0);
            GETEXSS(d0, 0);
            FADDS(v0, v0, d0);
            break;
        case 0x59:
            INST_NAME("MULSS Gx, Ex");
            nextop = F8;
            GETGXSS(v0);
            GETEXSS(d0, 0);
            FMULS(v0, v0, d0);
            break;
        case 0x5A:
            INST_NAME("CVTSS2SD Gx, Ex");
            nextop = F8;
            GETEXSS(v1, 0);
            GETGXSD_empty(v0);
            FCVTDS(v0, v1);
            break;
        case 0x5C:
            INST_NAME("SUBSS Gx, Ex");
            nextop = F8;
            GETGXSS(v0);
            GETEXSS(d0, 0);
            FSUBS(v0, v0, d0);
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
            FLTS(x2, d1, d0);
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
            FDIVS(v0, v0, d0);
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
            FLTS(x2, d0, d1);
            BEQ_MARK2(x2, xZR);
            MARK;
            FMVS(d0, d1);
            MARK2;
            break;
        case 0x6F:
            INST_NAME("MOVDQU Gx,Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            SSE_LOOP_MV_Q(x3);
            break;
        case 0x70: // TODO: Optimize this!
            INST_NAME("PSHUFHW Gx, Ex, Ib");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 1);
            u8 = F8;
            int32_t idx;

            idx = 4+((u8>>(0*2))&3);
            LHU(x3, wback, fixedaddress+idx*2);
            idx = 4+((u8>>(1*2))&3);
            LHU(x4, wback, fixedaddress+idx*2);
            idx = 4+((u8>>(2*2))&3);
            LHU(x5, wback, fixedaddress+idx*2);
            idx = 4+((u8>>(3*2))&3);
            LHU(x6, wback, fixedaddress+idx*2);

            SH(x3, gback, (4+0)*2);
            SH(x4, gback, (4+1)*2);
            SH(x5, gback, (4+2)*2);
            SH(x6, gback, (4+3)*2);

            if (!(MODREG && (gd==ed))) {
                LD(x3, wback, fixedaddress+0);
                SD(x3, gback, 0);
            }
            break;
        case 0x7E:
            INST_NAME("MOVQ Gx, Ex");
            nextop = F8;
            // Will load Gx as SD. Is that a good choice?
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7) + (rex.b<<3), 0);
                GETGXSD_empty(v0);
                FMVD(v0, v1);
            } else {
                GETGXSD_empty(v0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                FLD(v0, ed, fixedaddress);
            }
            SD(xZR, xEmu, offsetof(x64emu_t, xmm[gd])+8);
            break;
        case 0x7F:
            INST_NAME("MOVDQU Ex,Gx");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            SSE_LOOP_MV_Q2(x3);
            if(!MODREG) SMWRITE2();
            break;
        
        case 0x5B:
            INST_NAME("CVTTPS2DQ Gx, Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            v0 = fpu_get_scratch(dyn);
            for(int i=0; i<4; ++i) {
                if(!box64_dynarec_fastround) {
                    FSFLAGSI(xZR); // reset all bits
                }
                FLW(v0, wback, fixedaddress+i*4);
                FCVTWS(x3, v0, RD_RTZ);
                if(!box64_dynarec_fastround) {
                    FRFLAGS(x5);   // get back FPSR to check the IOC bit
                    ANDI(x5, x5, (1<<FR_NV)|(1<<FR_OF));
                    BEQZ(x5, 8);
                    MOV32w(x3, 0x80000000);
                }
                SW(x3, gback, i*4);
            }
            break;
        case 0xBC:
            INST_NAME("TZCNT Gd, Ed");
            SETFLAGS(X_ZF, SF_SUBSET);
            SET_DFNONE();
            nextop = F8;
            GETED(0);
            GETGD;
            if(!rex.w && MODREG) {
                AND(x4, ed, xMASK);
                ed = x4;
            }
            ANDI(xFlags, xFlags, ~((1<<F_ZF) | (1<<F_CF)));
            BNE_MARK(ed, xZR);
            ORI(xFlags, xFlags, 1<<F_CF);
            MOV32w(gd, rex.w?64:32);
            B_NEXT_nocond;
            MARK;
            if(rv64_zbb) {
                CTZxw(gd, ed);
            } else {
                NEG(x2, ed);
                AND(x2, x2, ed);
                TABLE64(x3, 0x03f79d71b4ca8b09ULL);
                MUL(x2, x2, x3);
                SRLI(x2, x2, 64-6);
                TABLE64(x1, (uintptr_t)&deBruijn64tab);
                ADD(x1, x1, x2);
                LBU(gd, x1, 0);
            }
            BNE(gd, xZR, 4+4);
            ORI(xFlags, xFlags, 1<<F_ZF);
            break;
        case 0xBD:
            INST_NAME("LZCNT Gd, Ed");
            SETFLAGS(X_ZF|X_CF, SF_SUBSET);
            SET_DFNONE();
            nextop = F8;
            GETED(0);
            GETGD;
            if(!rex.w && MODREG) {
                AND(x4, ed, xMASK);
                ed = x4;
            }
            BNE_MARK(ed, xZR);
            MOV32w(gd, rex.w?64:32);
            ANDI(xFlags, xFlags, ~(1<<F_ZF));
            ORI(xFlags, xFlags, 1<<F_CF);
            B_NEXT_nocond;
            MARK;
            if(rv64_zbb) {
                CLZxw(gd, ed);
            } else {
                if(ed!=gd)
                    u8 = gd;
                else
                    u8 = x1;
                ADDI(u8, xZR, rex.w?63:31);
                if(rex.w) {
                    MV(x2, ed);
                    SRLI(x3, x2, 32);
                    BEQZ(x3, 4+2*4);
                    SUBI(u8, u8, 32);
                    MV(x2, x3);
                } else {
                    AND(x2, ed, xMASK);
                }
                SRLI(x3, x2, 16);
                BEQZ(x3, 4+2*4);
                SUBI(u8, u8, 16);
                MV(x2, x3);
                SRLI(x3, x2, 8);
                BEQZ(x3, 4+2*4);
                SUBI(u8, u8, 8);
                MV(x2, x3);
                SRLI(x3, x2, 4);
                BEQZ(x3, 4+2*4);
                SUBI(u8, u8, 4);
                MV(x2, x3);
                ANDI(x2, x2, 0b1111); 
                TABLE64(x3, (uintptr_t)&lead0tab);
                ADD(x3, x3, x2);
                LBU(x2, x3, 0);
                SUB(gd, u8, x2);
                MARK2;
            }
            ANDI(xFlags, xFlags, ~((1<<F_ZF) | (1<<F_CF)));
            BNE(gd, xZR, 4+4);
            ORI(xFlags, xFlags, 1<<F_ZF);
            break;

        case 0xC2:
            INST_NAME("CMPSS Gx, Ex, Ib");
            nextop = F8;
            GETGXSS(d0);
            GETEXSS(d1, 1);
            u8 = F8;
            if ((u8&7) == 0) {                                      // Equal
                FEQS(x2, d0, d1);
            } else if ((u8&7) == 4) {                               // Not Equal or unordered
                FEQS(x2, d0, d1);
                XORI(x2, x2, 1);
            } else {
                // x2 = !(isnan(d0) || isnan(d1))
                FEQS(x3, d0, d0);
                FEQS(x2, d1, d1);
                AND(x2, x2, x3);

                switch(u8&7) {
                case 1: BEQ_MARK(x2, xZR); FLTS(x2, d0, d1); break; // Less than
                case 2: BEQ_MARK(x2, xZR); FLES(x2, d0, d1); break; // Less or equal
                case 3: XORI(x2, x2, 1); break;                     // NaN
                case 5: {                                           // Greater or equal or unordered
                    BEQ_MARK2(x2, xZR);
                    FLES(x2, d1, d0);
                    B_MARK_nocond;
                    break;
                }
                case 6: {                                           // Greater or unordered, test inverted, N!=V so unordered or less than (inverted)
                    BEQ_MARK2(x2, xZR);
                    FLTS(x2, d1, d0);
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
            FMVWX(d0, x2);
            break;

        case 0xE6:
            INST_NAME("CVTDQ2PD Gx, Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            LW(x3, wback, fixedaddress+0);
            LW(x4, wback, fixedaddress+4);
            FCVTDW(q0, x3, RD_RTZ);
            FCVTDW(q1, x4, RD_RTZ);
            FSD(q0, gback, 0);
            FSD(q1, gback, 8);
            break;

        default:
            DEFAULT;
    }
    return addr;
}
