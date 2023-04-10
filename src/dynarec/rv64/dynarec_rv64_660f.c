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

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_functions.h"
#include "dynarec_rv64_helper.h"

uintptr_t dynarec64_660F(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip; (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    int32_t i32;
    uint8_t gd, ed;
    uint8_t wback, wb1, wb2, gback;
    uint8_t eb1, eb2;
    int64_t j64;
    uint64_t tmp64u, tmp64u2;
    int v0, v1;
    int q0, q1;
    int d0, d1;
    int64_t fixedaddress;
    int unscaled;

    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(j64);
    
    switch(opcode) {
        case 0x10:
            INST_NAME("MOVUPD Gx,Ex");
            nextop = F8;
            GETEX(x1, 0);
            GETGX(x2);
            SSE_LOOP_MV_Q(x3);
            break;
        case 0x11:
            INST_NAME("MOVUPD Ex,Gx");
            nextop = F8;
            GETEX(x1, 0);
            GETGX(x2);
            SSE_LOOP_MV_Q2(x3);
            if(!MODREG) SMWRITE2();
            break;
        case 0x14:
            INST_NAME("UNPCKLPD Gx, Ex");
            nextop = F8;
            GETEXSD(d0, 0);
            GETGX(x3);
            FSD(d0, x3, 8);
            break;
        case 0x15:
            INST_NAME("UNPCKHPD Gx, Ex");
            nextop = F8;
            GETEX(x1, 0);
            GETGX(x2);
            // GX->q[0] = GX->q[1];
            LD(x3, gback, 8);
            SD(x3, gback, 0);
            // GX->q[1] = EX->q[1];
            LD(x3, wback, fixedaddress+8);
            SD(x3, gback, 8);
            break;
        case 0x1F:
            INST_NAME("NOP (multibyte)");
            nextop = F8;
            FAKEED;
            break;
        
        #define GO(GETFLAGS, NO, YES, F)            \
            READFLAGS(F);                           \
            GETFLAGS;                               \
            nextop=F8;                              \
            GETGD;                                  \
            if(MODREG) {                            \
                ed = xRAX+(nextop&7)+(rex.b<<3);    \
                SLLI(x4, ed, 48);                   \
                SRLI(x4, x4, 48);                   \
            } else {                                \
                SMREAD();                           \
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x4, &fixedaddress, rex, NULL, 1, 0); \
                LHU(x4, ed, fixedaddress);          \
                ed = x4;                            \
            }                                       \
            B##NO(x1, 4+4*4);                       \
            ADDI(x3, xZR, -1);                      \
            SRLI(x3, x3, 48);                       \
            AND(gd, gd, x3);                        \
            OR(gd, gd, ed);

        GOCOND(0x40, "CMOV", "Gw, Ew");
        #undef GO
        case 0x28:
            INST_NAME("MOVAPD Gx,Ex");
            nextop = F8;
            GETEX(x1, 0);
            GETGX(x2);
            SSE_LOOP_MV_Q(x3);
            break;
        case 0x29:
            INST_NAME("MOVAPD Ex,Gx");
            nextop = F8;
            GETEX(x1, 0);
            GETGX(x2);
            SSE_LOOP_MV_Q2(x3);
            if(!MODREG) SMWRITE2();
            break;
        case 0x2E:
            // no special check...
        case 0x2F:
            if(opcode==0x2F) {INST_NAME("COMISD Gx, Ex");} else {INST_NAME("UCOMISD Gx, Ex");}
            SETFLAGS(X_ALL, SF_SET);
            SET_DFNONE();
            nextop = F8;
            GETGXSD(d0);
            GETEXSD(v0, 0);
            CLEAR_FLAGS();
            // if isnan(d0) || isnan(v0)
            IFX(X_ZF | X_PF | X_CF) {
                FEQD(x3, d0, d0);
                FEQD(x2, v0, v0);
                AND(x2, x2, x3);
                BNE_MARK(x2, xZR);
                ORI(xFlags, xFlags, (1<<F_ZF) | (1<<F_PF) | (1<<F_CF));
                B_NEXT_nocond;
            }
            MARK;
            // else if isless(d0, v0)
            IFX(X_CF) {
                FLTD(x2, d0, v0);
                BEQ_MARK2(x2, xZR);
                ORI(xFlags, xFlags, 1<<F_CF);
                B_NEXT_nocond;
            }
            MARK2;
            // else if d0 == v0
            IFX(X_ZF) {
                FEQD(x2, d0, v0);
                CBZ_NEXT(x2);
                ORI(xFlags, xFlags, 1<<F_ZF);
            }
            break;
        case 0x54:
            INST_NAME("ANDPD Gx, Ex");
            nextop = F8;
            GETEX(x1, 0);
            GETGX(x2);
            SSE_LOOP_Q(x3, x4, AND(x3, x3, x4));
            break;
        case 0x55:
            INST_NAME("ANDNPD Gx, Ex");
            nextop = F8;
            GETEX(x1, 0);
            GETGX(x2);
            SSE_LOOP_Q(x3, x4, NOT(x3, x3); AND(x3, x3, x4));
            break;
        case 0x56:
            INST_NAME("ORPD Gx, Ex");
            nextop = F8;
            GETEX(x1, 0);
            GETGX(x2);
            SSE_LOOP_Q(x3, x4, OR(x3, x3, x4));
            break;
        case 0x57:
            INST_NAME("XORPD Gx, Ex");
            nextop = F8;
            GETEX(x1, 0);
            GETGX(x2);
            SSE_LOOP_Q(x3, x4, XOR(x3, x3, x4));
            break;
        case 0x5C:
            INST_NAME("SUBPD Gx, Ex");
            nextop = F8;
            //TODO: fastnan handling
            GETEX(x1, 0);
            GETGX(x2);
            SSE_LOOP_FQ(x3, x4, FSUBD(v0, v0, v1));
            break;
        case 0x61:
            INST_NAME("PUNPCKLWD Gx,Ex");
            nextop = F8;
            GETEX(x1, 0);
            GETGX(x2);
            for(int i=3; i>0; --i) {
                // GX->uw[2 * i] = GX->uw[i];
                LHU(x3, gback, i*2);
                SH(x3, gback, 2*i*2);
            }
            if (MODREG && (ed==gd)) {
                for(int i=0; i<4; ++i) {
                    // GX->uw[2 * i + 1] = GX->uw[2 * i];
                    LHU(x3, gback, 2*i*2);
                    SH(x3, gback, (2*i+1)*2);
                }
            } else {
                for(int i=0; i<4; ++i) {
                    // GX->uw[2 * i + 1] = EX->uw[i];
                    LHU(x3, wback, fixedaddress+i*2);
                    SH(x3, gback, (2*i+1)*2);
                }
            }
            break;
        case 0x62:
            INST_NAME("PUNPCKLDQ Gx,Ex");
            nextop = F8;
            GETEX(x1, 0);
            GETGX(x2);
            // GX->ud[3] = EX->ud[1];
            LWU(x3, x1, fixedaddress+1*4);
            SW(x3, x2, 3*4);
            // GX->ud[2] = GX->ud[1];
            LWU(x3, x2, 1*4);
            SW(x3, x2, 2*4);
            // GX->ud[1] = EX->ud[0];
            LWU(x3, x1, fixedaddress+0*4);
            SW(x3, x2, 1*4);
            break;
        case 0x66:
            INST_NAME("PCMPGTD Gx,Ex");
            nextop = F8;
            GETEX(x1, 0);
            GETGX(x2);
            SSE_LOOP_DS(x3, x4, SLT(x4, x4, x3); SLLI(x3, x4, 63); SRAI(x3, x3, 63));
            break;
        case 0x67:
            INST_NAME("PACKUSWB Gx, Ex");
            nextop = F8;
            GETEX(x1, 0);
            GETGX(x2);
            ADDI(x5, xZR, 0xFF);
            if (MODREG && (ed==gd)) {
                for(int i=0; i<8; ++i) {
                    // GX->ub[i] = (EX->sw[i]<0)?0:((EX->sw[i]>0xff)?0xff:EX->sw[i]);
                    LH(x3, wback, fixedaddress+i*2);
                    BGE(x5, x3, 8);
                    ADDI(x3, xZR, 0xFF);
                    NOT(x4, x3);
                    SRAI(x4, x4, 63);
                    AND(x3, x3, x4);
                    SB(x3, gback, i);
                }
                // GX->q[1] = GX->q[0];
                LD(x3, gback, 0*8);
                SD(x3, gback, 1*8);
            } else {
                for(int i=0; i<8; ++i) {
                    // GX->ub[i] = (GX->sw[i]<0)?0:((GX->sw[i]>0xff)?0xff:GX->sw[i]);
                    LH(x3, gback, i*2);
                    BGE(x5, x3, 8);
                    ADDI(x3, xZR, 0xFF);
                    NOT(x4, x3);
                    SRAI(x4, x4, 63);
                    AND(x3, x3, x4);
                    SB(x3, gback, i);
                }
                for(int i=0; i<8; ++i) {
                    // GX->ub[8+i] = (EX->sw[i]<0)?0:((EX->sw[i]>0xff)?0xff:EX->sw[i]);
                    LH(x3, wback, fixedaddress+i*2);
                    BGE(x5, x3, 8);
                    ADDI(x3, xZR, 0xFF);
                    NOT(x4, x3);
                    SRAI(x4, x4, 63);
                    AND(x3, x3, x4);
                    SB(x3, gback, 8+i);
                }
            }
            break;
        case 0x69:
            INST_NAME("PUNPCKHWD Gx,Ex");
            nextop = F8;
            GETEX(x1, 0);
            GETGX(x2);
            for(int i=0; i<4; ++i) {
                // GX->uw[2 * i] = GX->uw[i + 4];
                LHU(x3, gback, (i+4)*2);
                SH(x3, gback, 2*i*2);
            }
            if (MODREG && (ed==gd)) {
                for(int i=0; i<4; ++i) {
                    // GX->uw[2 * i + 1] = GX->uw[2 * i];
                    LHU(x3, gback, 2*i*2);
                    SH(x3, gback, (2*i+1)*2);
                }
            } else {
                for(int i=0; i<4; ++i) {
                    // GX->uw[2 * i + 1] = EX->uw[i + 4];
                    LHU(x3, wback, fixedaddress+(i+4)*2);
                    SH(x3, gback, (2*i+1)*2);
                }
            }
            break;
        case 0x6A:
            INST_NAME("PUNPCKHDQ Gx,Ex");
            nextop = F8;
            GETEX(x1, 0);
            GETGX(x2);
            // GX->ud[0] = GX->ud[2];
            LWU(x3, gback, 2*4);
            SW(x3, gback, 0*4);
            // GX->ud[1] = EX->ud[2];
            LWU(x3, wback, fixedaddress+2*4);
            SW(x3, gback, 1*4);
            // GX->ud[2] = GX->ud[3];
            LWU(x3, gback, 3*4);
            SW(x3, gback, 2*4);
            // GX->ud[3] = EX->ud[3];
            if (!(MODREG && (gd==ed))) {
                LWU(x3, wback, fixedaddress+3*4);
                SW(x3, gback, 3*4);
            }
            break;
        case 0x6C:
            INST_NAME("PUNPCKLQDQ Gx,Ex");
            nextop = F8;
            GETGX(x1);
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x2, (nextop&7)+(rex.b<<3), 0);
                FSD(v1, gback, 8);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                LD(x3, ed, fixedaddress+0);
                SD(x3, gback, 8);
            }
            break;
        case 0x6D:
            INST_NAME("PUNPCKHQDQ Gx,Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            LD(x3, gback, 8);
            SD(x3, gback, 0);
            LD(x3, wback, fixedaddress+8);
            SD(x3, gback, 8);
            break;
        case 0x6E:
            INST_NAME("MOVD Gx, Ed");
            nextop = F8;
            if(rex.w) {
                GETGXSD_empty(v0);
            } else {
                GETGXSS_empty(v0);
            }
            GETED(0);
            if(rex.w) {
                FMVDX(v0, ed);
            } else {
                FMVWX(v0, ed);
                SW(xZR, xEmu, offsetof(x64emu_t, xmm[gd])+4);
            }
            SD(xZR, xEmu, offsetof(x64emu_t, xmm[gd])+8);
            break;
        case 0x6F:
            INST_NAME("MOVDQA Gx,Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            SSE_LOOP_MV_Q(x3);
            break;
        case 0x70: // TODO: Optimize this!
            INST_NAME("PSHUFD Gx,Ex,Ib");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 1);
            u8 = F8;
            int32_t idx;

            idx = (u8>>(0*2))&3;
            LWU(x3, wback, fixedaddress+idx*4);
            idx = (u8>>(1*2))&3;
            LWU(x4, wback, fixedaddress+idx*4);
            idx = (u8>>(2*2))&3;
            LWU(x5, wback, fixedaddress+idx*4);
            idx = (u8>>(3*2))&3;
            LWU(x6, wback, fixedaddress+idx*4);

            SW(x3, gback, 0*4);
            SW(x4, gback, 1*4);
            SW(x5, gback, 2*4);
            SW(x6, gback, 3*4);
            break;
        case 0x72:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 2:
                    INST_NAME("PSRLD Ex, Ib");
                    GETEX(x1, 1);
                    u8 = F8;
                    if(u8) {
                        if (u8>31) {
                            // just zero dest
                            SD(xZR, x1, fixedaddress+0);
                            SD(xZR, x1, fixedaddress+8);
                        } else if(u8) {
                            SSE_LOOP_D_S(x3, SRLI(x3, x3, u8));
                        }
                    }
                    break;
                case 4:
                    INST_NAME("PSRAD Ex, Ib");
                    GETEX(x1, 1);
                    u8 = F8;
                    if(u8>31) u8=31;
                    if (u8) {
                        SSE_LOOP_D_S(x3, SRAIW(x3, x3, u8));
                    }
                    break;
                case 6:
                    INST_NAME("PSLLD Ex, Ib");
                    GETEX(x1, 1);
                    u8 = F8;
                    if(u8) {
                        if (u8>31) {
                            // just zero dest
                            SD(xZR, x1, fixedaddress+0);
                            SD(xZR, x1, fixedaddress+8);
                        } else if(u8) {
                            SSE_LOOP_D_S(x3, SLLI(x3, x3, u8));
                        }
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0x73:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 2:
                    INST_NAME("PSRLQ Ex, Ib");
                    GETEX(x1, 1);
                    u8 = F8;
                    if(!u8) break;
                    if(u8>63) {
                        // just zero dest
                        SD(xZR, wback, fixedaddress+0);
                        SD(xZR, wback, fixedaddress+8);
                    } else {
                        LD(x3, wback, fixedaddress+0);
                        LD(x4, wback, fixedaddress+8);
                        SRLI(x3, x3, u8);
                        SRLI(x4, x4, u8);
                        SD(x3, wback, fixedaddress+8);
                        SD(x4, wback, fixedaddress+8);
                    }
                    break;
                case 3:
                    INST_NAME("PSRLDQ Ex, Ib");
                    GETEX(x1, 1);
                    u8 = F8;
                    if(!u8) break;
                    if(u8>15) {
                        // just zero dest
                        SD(xZR, x1, fixedaddress+0);
                        SD(xZR, x1, fixedaddress+8);
                    } else {
                        u8*=8;
                        if (u8 < 64) {
                            LD(x3, x1, fixedaddress+0);
                            LD(x4, x1, fixedaddress+8);
                            SRLI(x3, x3, u8);
                            SLLI(x5, x4, 64-u8);
                            OR(x3, x3, x5);
                            SD(x3, x1, fixedaddress+0);
                            SRLI(x4, x4, u8);
                            SD(x4, x1, fixedaddress+8);
                        } else {
                            LD(x3, x1, fixedaddress+8);
                            if (u8-64 > 0) { SRLI(x3, x3, u8-64); }
                            SD(x3, x1, fixedaddress+0);
                            SD(xZR, x1, fixedaddress+8);
                        }
                    }
                    break;
                case 7:
                    INST_NAME("PSLLDQ Ex, Ib");
                    GETEX(x1, 1);
                    u8 = F8;
                    if(!u8) break;
                    if(u8>15) {
                        // just zero dest
                        SD(xZR, x1, fixedaddress+0);
                        SD(xZR, x1, fixedaddress+8);
                    } else {
                        u8*=8;
                        if (u8 < 64) {
                            LD(x3, x1, fixedaddress+0);
                            LD(x4, x1, fixedaddress+8);
                            SLLI(x4, x4, u8);
                            SRLI(x5, x3, 64-u8);
                            OR(x4, x4, x5);
                            SD(x4, x1, fixedaddress+8);
                            SLLI(x3, x3, u8);
                            SD(x3, x1, fixedaddress+0);
                        } else {
                            LD(x3, x1, fixedaddress+0);
                            if (u8-64 > 0) { SLLI(x3, x3, u8-64); }
                            SD(x3, x1, fixedaddress+8);
                            SD(xZR, x1, fixedaddress+0);
                        }
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0x74:
            INST_NAME("PCMPEQB Gx,Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            for (int i=0; i<16; ++i) {
                LBU(x3, gback, i);
                LBU(x4, wback, fixedaddress+i);
                SUB(x3, x3, x4);
                SEQZ(x3, x3);
                NEG(x3, x3);
                SB(x3, gback, i);
            }
            break;
        case 0x75:
            INST_NAME("PCMPEQW Gx,Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            SSE_LOOP_W(x3, x4, SUB(x3, x3, x4); SEQZ(x3, x3); NEG(x3, x3));
            break;
        case 0x76:
            INST_NAME("PCMPEQD Gx,Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            SSE_LOOP_D(x3, x4, XOR(x3, x3, x4); SNEZ(x3, x3); ADDI(x3, x3, -1));
            break;
        case 0x7E:
            INST_NAME("MOVD Ed,Gx");
            nextop = F8;
            GETGX(x1);
            if(rex.w) {
                if(MODREG) {
                    ed = xRAX + (nextop&7) + (rex.b<<3);
                    LD(ed, x1, 0);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                    LD(x3, x1, 0);
                    SD(x3, ed, fixedaddress);
                    SMWRITE2();
                }
            } else {
                if(MODREG) {
                    ed = xRAX + (nextop&7) + (rex.b<<3);
                    LWU(ed, x1, 0);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                    LWU(x3, x1, 0);
                    SW(x3, ed, fixedaddress);
                    SMWRITE2();
                }
            }
            break;
        case 0x7F:
            INST_NAME("MOVDQA Ex,Gx");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            SSE_LOOP_MV_Q2(x3);
            if(!MODREG) SMWRITE2();
            break;
        case 0xAF:
            INST_NAME("IMUL Gw,Ew");
            SETFLAGS(X_ALL, SF_PENDING);
            nextop = F8;
            UFLAG_DF(x1, d_imul16);
            GETSEW(x1, 0);
            GETSGW(x2);
            MULW(x2, x2, x1);
            UFLAG_RES(x2);
            SLLI(x2, x2, 48);
            SRLI(x2, x2, 48);
            GWBACK;
            break;

        case 0xBE:
            INST_NAME("MOVSX Gw, Eb");
            nextop = F8;
            GETGD;
            if(MODREG) {
                if(rex.rex) {
                    ed = xRAX+(nextop&7)+(rex.b<<3);
                    eb1=ed;
                    eb2=0;
                } else {
                    ed = (nextop&7);
                    eb1 = xRAX+(ed&3);  // Ax, Cx, Dx or Bx
                    eb2 = (ed&4)>>2;    // L or H
                }
                SLLI(x1, eb1, 56-eb2*8);
                SRAI(x1, x1, 56);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x4, &fixedaddress, rex, NULL, 0, 0);
                LB(x1, ed, fixedaddress);
            }
            LUI(x5, 0xffff0);
            AND(gd, gd, x5);
            NOT(x5, x5);
            AND(x1, x1, x5);
            OR(gd, gd, x1);
            break;
        case 0xC5:
            INST_NAME("PEXTRW Gd,Ex,Ib");
            nextop = F8;
            GETGD;
            GETEX(x1, 0);
            u8 = (F8)&7;
            LHU(gd, wback, fixedaddress+u8*2);
            break;
        case 0xD4:
            INST_NAME("PADDQ Gx,Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            SSE_LOOP_Q(x3, x4, ADD(x3, x3, x4));
            break;
        case 0xD6:
            INST_NAME("MOVQ Ex, Gx");
            nextop = F8;
            GETGXSD(d0);
            GETEX(x2, 0);
            FSD(d0, wback, fixedaddress+0);
            if (MODREG) {
                SD(xZR, wback, fixedaddress+8);
            } else {
                SMWRITE2();
            }
            break;
        case 0xD7:
            INST_NAME("PMOVMSKB Gd, Ex");
            nextop = F8;
            GETEX(x2, 0);
            GETGD;
            MV(gd, xZR);
            for (int i=0; i<16; ++i) {
                LB(x1, wback, fixedaddress+i);
                SLT(x3, x1, xZR);
                if (i > 0) SLLI(x3, x3, i);
                OR(gd, gd, x3);
            }
            break;
        case 0xD9:
            INST_NAME("PSUBUSW Gx, Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            SSE_LOOP_W(x3, x4, SUB(x3, x3, x4); NOT(x4, x3); SRAI(x4, x4, 63); AND(x3, x3, x4));
            break;
        case 0xDB:
            INST_NAME("PAND Gx,Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            SSE_LOOP_Q(x3, x4, AND(x3, x3, x4));
            break;
        case 0xDF:
            INST_NAME("PANDN Gx,Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            SSE_LOOP_Q(x3, x4, NOT(x3, x3); AND(x3, x3, x4));
            break;
        case 0xEB:
            INST_NAME("POR Gx,Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            SSE_LOOP_Q(x3, x4, OR(x3, x3, x4));
            break;
        case 0xEE:
            INST_NAME("PMAXSW Gx,Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            SSE_LOOP_WS(x3, x4, BGE(x3, x4, 8); MV(x3, x4));
            break;
        case 0xEF:
            INST_NAME("PXOR Gx, Ex");
            nextop = F8;
            GETGX(x1);
            if(MODREG && gd==(nextop&7)+(rex.b<<3))
            {
                // just zero dest
                SD(xZR, x1, 0);
                SD(xZR, x1, 8);
            } else {
                GETEX(x2, 0);
                SSE_LOOP_Q(x3, x4, XOR(x3, x3, x4));
            }
            break;
        case 0xFA:
            INST_NAME("PSUBD Gx,Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            SSE_LOOP_D(x3, x4, SUBW(x3, x3, x4));
            break;
        case 0xFD:
            INST_NAME("PADDW Gx,Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            SSE_LOOP_W(x3, x4, ADDW(x3, x3, x4));
            break;
        case 0xFE:
            INST_NAME("PADDD Gx,Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            SSE_LOOP_D(x3, x4, ADDW(x3, x3, x4));
            break;
        default:
            DEFAULT;
    }
    return addr;
}
