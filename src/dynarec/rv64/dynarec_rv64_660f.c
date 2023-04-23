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

    static const int8_t round_round[] = { RD_RNE, RD_RDN, RD_RUP, RD_RTZ };
    
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
        case 0x12:
            INST_NAME("MOVLPD Gx, Eq");
            nextop = F8;
            GETGX(x1);
            if(MODREG) {
                // access register instead of memory is bad opcode!
                DEFAULT;
                return addr;
            }
            SMREAD();
            addr = geted(dyn, addr, ninst, nextop, &wback, x2, x3, &fixedaddress, rex, NULL, 1, 0);
            LD(x3, wback, fixedaddress);
            SD(x3, gback, 0);
            break;
        case 0x16:
            INST_NAME("MOVHPD Gx, Eq");
            nextop = F8;
            GETGX(x1);
            if(MODREG) {
                // access register instead of memory is bad opcode!
                DEFAULT;
                return addr;
            }
            SMREAD();
            addr = geted(dyn, addr, ninst, nextop, &wback, x2, x3, &fixedaddress, rex, NULL, 1, 0);
            LD(x3, wback, fixedaddress);
            SD(x3, gback, 8);
            break;
        case 0x14:
            INST_NAME("UNPCKLPD Gx, Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            // GX->q[1] = EX->q[0];
            LD(x3, wback, fixedaddress+0);
            SD(x3, gback, 8);
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
        case 0x38:  // SSSE3 opcodes
            nextop = F8;
            switch(nextop) {
                case 0x00:
                    INST_NAME("PSHUFB Gx, Ex");
                    nextop = F8;
                    GETGX(x1);
                    GETEX(x2, 0);
                    sse_forget_reg(dyn, ninst, x5);

                    ADDI(x5, xEmu, offsetof(x64emu_t, scratch));

                    // perserve gd
                    LD(x3, gback, 0);
                    LD(x4, gback, 8);
                    SD(x3, x5, 0);
                    SD(x4, x5, 8);

                    for (int i=0; i<16; ++i) {
                        LBU(x3, wback, fixedaddress+i);
                        ANDI(x4, x3, 128);
                        BEQZ(x4, 12);
                        SB(xZR, gback, i);
                        BEQZ(xZR, 20); // continue
                        ANDI(x4, x3, 15);
                        ADD(x4, x4, x5);
                        LBU(x4, x4, 0);
                        SB(x4, gback, i);
                    }
                    break;
                case 0x01:
                    INST_NAME("PHADDW Gx, Ex");
                    nextop = F8;
                    GETGX(x1);
                    for (int i=0; i<4; ++i) {
                        // GX->sw[i] = GX->sw[i*2+0]+GX->sw[i*2+1];
                        LH(x3, gback, 2*(i*2+0));
                        LH(x4, gback, 2*(i*2+1));
                        ADDW(x3, x3, x4);
                        SH(x3, gback, 2*i);
                    }
                    if (MODREG && gd==(nextop&7)+(rex.b<<3)) {
                        // GX->q[1] = GX->q[0];
                        LD(x3, gback, 0);
                        SD(x3, gback, 8);
                    } else {
                        GETEX(x2, 0);
                        for (int i=0; i<4; ++i) {
                            // GX->sw[4+i] = EX->sw[i*2+0] + EX->sw[i*2+1];
                            LH(x3, wback, fixedaddress+2*(i*2+0));
                            LH(x4, wback, fixedaddress+2*(i*2+1));
                            ADDW(x3, x3, x4);
                            SH(x3, gback, 2*(4+i));
                        }
                    }
                    break;
                case 0x02:
                    INST_NAME("PHADDD Gx, Ex");
                    nextop = F8;
                    GETGX(x1);
                    // GX->sd[0] += GX->sd[1];
                    LW(x3, gback, 0*4);
                    LW(x4, gback, 1*4);
                    ADDW(x3, x3, x4);
                    SW(x3, gback, 0*4);
                    // GX->sd[1] = GX->sd[2] + GX->sd[3];
                    LW(x3, gback, 2*4);
                    LW(x4, gback, 3*4);
                    ADDW(x3, x3, x4);
                    SW(x3, gback, 1*4);
                    if (MODREG && gd==(nextop&7)+(rex.b<<3)) {
                        // GX->q[1] = GX->q[0];
                        LD(x3, gback, 0);
                        SD(x3, gback, 8);
                    } else {
                        GETEX(x2, 0);
                        // GX->sd[2] = EX->sd[0] + EX->sd[1];
                        LW(x3, wback, fixedaddress+0*4);
                        LW(x4, wback, fixedaddress+1*4);
                        ADDW(x3, x3, x4);
                        SW(x3, gback, 2*4);
                        // GX->sd[3] = EX->sd[2] + EX->sd[3];
                        LW(x3, wback, fixedaddress+2*4);
                        LW(x4, wback, fixedaddress+3*4);
                        ADDW(x3, x3, x4);
                        SW(x3, gback, 3*4);
                    }
                    break;
                case 0x17:
                    INST_NAME("PTEST Gx, Ex");
                    nextop = F8;
                    SETFLAGS(X_ALL, SF_SET);
                    GETGX(x1);
                    GETEX(x2, 0);
                    CLEAR_FLAGS();
                    SET_DFNONE();
                    IFX(X_ZF|X_CF) {
                        LD(x5, wback, fixedaddress+0);
                        LD(x6, wback, fixedaddress+8);

                        IFX(X_ZF) {
                            LD(x3, gback, 0);
                            LD(x4, gback, 8);
                            AND(x3, x3, x5);
                            AND(x4, x4, x6);
                            OR(x3, x3, x4);
                            BNEZ(x3, 8);
                            ORI(xFlags, xFlags, 1<<F_ZF);
                        }
                        IFX(X_CF) {
                            LD(x3, gback, 0);
                            NOT(x3, x3);
                            LD(x4, gback, 8);
                            NOT(x4, x4);
                            AND(x3, x3, x5);
                            AND(x4, x4, x6);
                            OR(x3, x3, x4);
                            BNEZ(x3, 8);
                            ORI(xFlags, xFlags, 1<<F_ZF);
                        }
                    }
                    break;
                case 0x3A:
                    INST_NAME("PMINUW Gx, Ex");  // SSE4 opcode!
                    nextop = F8;
                    GETGX(x1);
                    GETEX(x2, 0);
                    for(int i=0; i<8; ++i) {
                        // if(GX->uw[i]>EX->uw[i]) GX->uw[i] = EX->uw[i];
                        LHU(x3, gback, i*2);
                        LHU(x4, wback, fixedaddress+i*2);
                        BLTU(x3, x4, 8);
                        SH(x4, gback, i*2);
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0x3A:  // these are some more SSSE3+ opcodes
            opcode = F8;
            switch(opcode) {
                case 0x0B:
                    INST_NAME("ROUNDSD Gx, Ex, Ib");
                    nextop = F8;
                    GETEXSD(d0, 0);
                    GETGXSD_empty(v0);
                    d1 = fpu_get_scratch(dyn);
                    u8 = F8;
                    FEQD(x2, d0, d0);
                    BNEZ_MARK(x2);
                    FADDD(v0, d0, d0);
                    B_NEXT_nocond;
                    MARK; // d0 is not nan
                    FABSD(v0, d0);
                    MOV64x(x3, 1ULL << __DBL_MANT_DIG__);
                    FCVTDL(d1, x3, RD_RTZ);
                    FLTD(x3, v0, d1);
                    BNEZ_MARK2(x3);
                    if (v0!=d0) FMVD(v0, d0);
                    B_NEXT_nocond;
                    MARK2;
                    if(u8&4) {
                        u8 = sse_setround(dyn, ninst, x4, x2);
                        FCVTLD(x5, d0, RD_DYN);
                        FCVTDL(v0, x5, RD_DYN);
                        x87_restoreround(dyn, ninst, u8);
                    } else {
                        FCVTLD(x5, d0, round_round[u8&3]);
                        FCVTDL(v0, x5, round_round[u8&3]);
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0x51:
            INST_NAME("SQRTPD Gx, Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            d0 = fpu_get_scratch(dyn);
            if(!box64_dynarec_fastnan) {
                d1 = fpu_get_scratch(dyn);
                FMVDX(d1, xZR);
            }
            for (int i=0; i<2; ++i) {
                FLD(d0, wback, fixedaddress+i*8);
                if(!box64_dynarec_fastnan) {
                    FLTD(x3, d0, d1);
                }
                FSQRTD(d0, d0);
                if(!box64_dynarec_fastnan) {
                    BEQ(x3, xZR, 8);
                    FNEGD(d0, d0);
                }
                FSD(d0, gback, i*8);
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
        case 0x58:
            INST_NAME("ADDPD Gx, Ex");
            nextop = F8;
            //TODO: fastnan handling
            GETEX(x1, 0);
            GETGX(x2);
            SSE_LOOP_FQ(x3, x4, FADDD(v0, v0, v1));
            break;
        case 0x59:
            INST_NAME("MULPD Gx, Ex");
            nextop = F8;
            GETEX(x1, 0);
            GETGX(x2);
            SSE_LOOP_FQ(x3, x4, {
                if(!box64_dynarec_fastnan) {
                    FEQD(x3, v0, v0);
                    FEQD(x4, v1, v1);
                }
                FMULD(v0, v0, v1);
                if(!box64_dynarec_fastnan) {
                    AND(x3, x3, x4);
                    BEQZ(x3, 16);
                    FEQD(x3, v0, v0);
                    BNEZ(x3, 8);
                    FNEGD(v0, v0);
                }
            });
            break;
        case 0x5A:
            INST_NAME("CVTPD2PS Gx, Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            d0 = fpu_get_scratch(dyn);
            // GX->f[0] = EX->d[0];
            FLD(d0, wback, fixedaddress+0);
            FCVTSD(d0, d0);
            FSD(d0, gback, 0);
            // GX->f[1] = EX->d[1];
            FLD(d0, wback, fixedaddress+8);
            FCVTSD(d0, d0);
            FSD(d0, gback, 4);
            // GX->q[1] = 0;
            SD(xZR, gback, 8);
            break;
        case 0x5C:
            INST_NAME("SUBPD Gx, Ex");
            nextop = F8;
            //TODO: fastnan handling
            GETEX(x1, 0);
            GETGX(x2);
            SSE_LOOP_FQ(x3, x4, FSUBD(v0, v0, v1));
            break;
        case 0x60:
            INST_NAME("PUNPCKLBW Gx,Ex");
            nextop = F8;
            GETGX(x2);
            for(int i=7; i>0; --i) { // 0 is untouched
                // GX->ub[2 * i] = GX->ub[i];
                LBU(x3, gback, i);
                SB(x3, gback, 2*i);
            }
            if (MODREG && gd==(nextop&7)+(rex.b<<3)) {
                for(int i=0; i<8; ++i) {
                    // GX->ub[2 * i + 1] = GX->ub[2 * i];
                    LBU(x3, gback, 2*i);
                    SB(x3, gback, 2*i+1);
                }
            } else {
                GETEX(x1, 0);
                for(int i=0; i<8; ++i) {
                    // GX->ub[2 * i + 1] = EX->ub[i];
                    LBU(x3, wback, fixedaddress+i);
                    SB(x3, gback, 2*i+1);
                }
            }
            break;
        case 0x61:
            INST_NAME("PUNPCKLWD Gx,Ex");
            nextop = F8;
            GETGX(x2);
            for(int i=3; i>0; --i) {
                // GX->uw[2 * i] = GX->uw[i];
                LHU(x3, gback, i*2);
                SH(x3, gback, 2*i*2);
            }
            if (MODREG && gd==(nextop&7)+(rex.b<<3)) {
                for(int i=0; i<4; ++i) {
                    // GX->uw[2 * i + 1] = GX->uw[2 * i];
                    LHU(x3, gback, 2*i*2);
                    SH(x3, gback, (2*i+1)*2);
                }
            } else {
                GETEX(x1, 0);
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
        case 0x64:
            INST_NAME("PCMPGTB Gx,Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            for(int i=0; i<16; ++i) {
                // GX->ub[i] = (GX->sb[i]>EX->sb[i])?0xFF:0x00;
                LB(x3, wback, fixedaddress+i);
                LB(x4, gback, i);
                SLT(x3, x3, x4);
                NEG(x3, x3);
                SB(x3, gback, i);
            }
            break;
        case 0x65:
            INST_NAME("PCMPGTW Gx,Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            for(int i=0; i<8; ++i) {
                // GX->uw[i] = (GX->sw[i]>EX->sw[i])?0xFFFF:0x0000;
                LH(x3, wback, fixedaddress+i*2);
                LH(x4, gback, i*2);
                SLT(x3, x3, x4);
                NEG(x3, x3);
                SH(x3, gback, i*2);
            }
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
            GETGX(x2);
            ADDI(x5, xZR, 0xFF);
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
            if (MODREG && gd==(nextop&7)+(rex.b<<3)) {
                // GX->q[1] = GX->q[0];
                LD(x3, gback, 0*8);
                SD(x3, gback, 1*8);
            } else {
                GETEX(x1, 0);
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
        case 0x68:
            INST_NAME("PUNPCKHBW Gx,Ex");
            nextop = F8;
            GETGX(x1);
            for(int i=0; i<8; ++i) {
                // GX->ub[2 * i] = GX->ub[i + 8];
                LBU(x3, gback, i+8);
                SB(x3, gback, 2*i);
            }
            if (MODREG && gd==(nextop&7)+(rex.b<<3)) {
                for(int i=0; i<8; ++i) {
                    // GX->ub[2 * i + 1] = GX->ub[2 * i];
                    LBU(x3, gback, 2*i);
                    SB(x3, gback, 2*i+1);
                }
            } else {
                GETEX(x2, 0);
                for(int i=0; i<8; ++i) {
                    // GX->ub[2 * i + 1] = EX->ub[i + 8];
                    LBU(x3, wback, fixedaddress+i+8);
                    SB(x3, gback, 2*i+1);
                }
            }
            break;
        case 0x69:
            INST_NAME("PUNPCKHWD Gx,Ex");
            nextop = F8;
            GETGX(x2);
            for(int i=0; i<4; ++i) {
                // GX->uw[2 * i] = GX->uw[i + 4];
                LHU(x3, gback, (i+4)*2);
                SH(x3, gback, 2*i*2);
            }
            if (MODREG && gd==(nextop&7)+(rex.b<<3)) {
                for(int i=0; i<4; ++i) {
                    // GX->uw[2 * i + 1] = GX->uw[2 * i];
                    LHU(x3, gback, 2*i*2);
                    SH(x3, gback, (2*i+1)*2);
                }
            } else {
                GETEX(x1, 0);
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
        case 0x6B:
            INST_NAME("PACKSSDW Gx,Ex");
            nextop = F8;
            GETGX(x2);
            MOV64x(x5, 32768);
            NEG(x6, x5);
            for(int i=0; i<4; ++i) {
                // GX->sw[i] = (GX->sd[i]<-32768)?-32768:((GX->sd[i]>32767)?32767:GX->sd[i]);
                LW(x3, gback, i*4);
                BGE(x5, x3, 8);
                ADDI(x3, x5, -1);
                BGE(x3, x6, 8);
                MV(x3, x6);
                SH(x3, gback, i*2);
            }
            if (MODREG && gd==(nextop&7)+(rex.b<<3)) {
                // GX->q[1] = GX->q[0];
                LD(x3, gback, 0*8);
                SD(x3, gback, 1*8);
            } else {
                GETEX(x1, 0);
                for(int i=0; i<4; ++i) {
                    // GX->sw[4+i] = (EX->sd[i]<-32768)?-32768:((EX->sd[i]>32767)?32767:EX->sd[i]);
                    LW(x3, wback, fixedaddress+i*4);
                    BGE(x5, x3, 8);
                    ADDI(x3, x5, -1);
                    BGE(x3, x6, 8);
                    MV(x3, x6);
                    SH(x3, gback, (4+i)*2);
                }
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
        case 0x71:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 2:
                    INST_NAME("PSRLW Ex, Ib");
                    GETEX(x1, 1);
                    u8 = F8;
                    if (u8>15) {
                        // just zero dest
                        SD(xZR, x1, fixedaddress+0);
                        SD(xZR, x1, fixedaddress+8);
                    } else if(u8) {
                        for (int i=0; i<8; ++i) {
                            // EX->uw[i] >>= u8;
                            LHU(x3, wback, fixedaddress+i*2);
                            SRLI(x3, x3, u8);
                            SH(x3, wback, fixedaddress+i*2);
                        }
                    }
                    break;
                case 4:
                    INST_NAME("PSRAW Ex, Ib");
                    GETEX(x1, 1);
                    u8 = F8;
                    if(u8>15) u8=15;
                    if(u8) {
                        for (int i=0; i<8; ++i) {
                            // EX->sw[i] >>= u8;
                            LH(x3, wback, fixedaddress+i*2);
                            SRAI(x3, x3, u8);
                            SH(x3, wback, fixedaddress+i*2);
                        }
                    }
                    break;
                case 6:
                    INST_NAME("PSLLW Ex, Ib");
                    GETEX(x1, 1);
                    u8 = F8;
                    if (u8>15) {
                        // just zero dest
                        SD(xZR, x1, fixedaddress+0);
                        SD(xZR, x1, fixedaddress+8);
                    } else if(u8) {
                        for (int i=0; i<8; ++i) {
                            // EX->uw[i] <<= u8;
                            LHU(x3, wback, fixedaddress+i*2);
                            SLLI(x3, x3, u8);
                            SH(x3, wback, fixedaddress+i*2);
                        }
                    }
                    break;
                default:
                    *ok = 0;
                    DEFAULT;
            }
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
                        SD(x3, wback, fixedaddress+0);
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
                case 6:
                    INST_NAME("PSLLQ Ex, Ib");
                    GETEX(x1, 1);
                    u8 = F8;
                    if(!u8) break;
                    if(u8>63) {
                        // just zero dest
                        SD(xZR, x1, fixedaddress+0);
                        SD(xZR, x1, fixedaddress+8);
                    } else {
                        LD(x3, wback, fixedaddress+0);
                        LD(x4, wback, fixedaddress+8);
                        SLLI(x3, x3, u8);
                        SLLI(x4, x4, u8);
                        SD(x3, wback, fixedaddress+0);
                        SD(x4, wback, fixedaddress+8);
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
        case 0xC4:
            INST_NAME("PINSRW Gx,Ed,Ib");
            nextop = F8;
            GETED(1);
            GETGX(x3);
            u8 = (F8)&7;
            SH(ed, gback, u8*2);
            break;
        case 0xC5:
            INST_NAME("PEXTRW Gd,Ex,Ib");
            nextop = F8;
            GETGD;
            GETEX(x1, 0);
            u8 = (F8)&7;
            LHU(gd, wback, fixedaddress+u8*2);
            break;
        case 0xD1:
            INST_NAME("PSRLW Gx,Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            LD(x3, wback, fixedaddress);
            ADDI(x4, xZR, 16);
            BLTU_MARK(x3, x4);
            SD(xZR, gback, 0);
            SD(xZR, gback, 8);
            B_NEXT_nocond;
            MARK;
            for (int i=0; i<8; ++i) {
                LHU(x5, gback, 2*i);
                SRLW(x5, x5, x3);
                SH(x5, gback, 2*i);
            }
            break;
        case 0xD2:
            INST_NAME("PSRLD Gx,Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            LD(x3, wback, fixedaddress);
            ADDI(x4, xZR, 32);
            BLTU_MARK(x3, x4);
            SD(xZR, gback, 0);
            SD(xZR, gback, 8);
            B_NEXT_nocond;
            MARK;
            for (int i=0; i<4; ++i) {
                LWU(x5, gback, 4*i);
                SRLW(x5, x5, x3);
                SW(x5, gback, 4*i);
            }
            break;
        case 0xD3:
            INST_NAME("PSRLQ Gx,Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            LD(x3, wback, fixedaddress);
            ADDI(x4, xZR, 64);
            BLTU_MARK(x3, x4);
            SD(xZR, gback, 0);
            SD(xZR, gback, 8);
            B_NEXT_nocond;
            MARK;
            for (int i=0; i<2; ++i) {
                LD(x5, gback, 8*i);
                SRL(x5, x5, x3);
                SD(x5, gback, 8*i);
            }
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
        case 0xDD:
            INST_NAME("PADDUSW Gx,Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            for(int i=0; i<8; ++i) {
                // tmp32s = (int32_t)GX->uw[i] + EX->uw[i];
                // GX->uw[i] = (tmp32s>65535)?65535:tmp32s;
                LHU(x3, gback, i*2);
                LHU(x4, wback, fixedaddress+i*2);
                ADDW(x3, x3, x4);
                MOV32w(x4, 65536);
                BLT(x3, x4, 8);
                ADDIW(x3, x4, -1);
                SH(x3, gback, i*2);
            }
            break;
        case 0xDF:
            INST_NAME("PANDN Gx,Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            SSE_LOOP_Q(x3, x4, NOT(x3, x3); AND(x3, x3, x4));
            break;
        case 0xE4:
            INST_NAME("PMULHUW Gx,Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            for(int i=0; i<8; ++i) {
                LHU(x3, gback, 2*i);
                LHU(x4, wback, fixedaddress+2*i);
                MULW(x3, x3, x4);
                SRLIW(x3, x3, 16);
                SH(x3, gback, 2*i);
            }
            break;
        case 0xE8:
            INST_NAME("PSUBSB Gx,Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            for(int i=0; i<16; ++i) {
                // tmp16s = (int16_t)GX->sb[i] - EX->sb[i];
                // GX->sb[i] = (tmp16s<-128)?-128:((tmp16s>127)?127:tmp16s);
                LB(x3, gback, i);
                LB(x4, wback, fixedaddress+i);
                SUBW(x3, x3, x4);
                SLLIW(x3, x3, 16);
                SRAIW(x3, x3, 16);
                ADDI(x4, xZR, 0x7f);
                BLT(x3, x4, 12);     // tmp16s>127?
                SB(x4, gback, i);
                J(24);               // continue
                ADDI(x4, xZR, 0xf80);
                BLT(x4, x3, 12);     // tmp16s<-128?
                SB(x4, gback, i);
                J(8);                // continue
                SB(x3, gback, i);
            }
            break;
        case 0xEB:
            INST_NAME("POR Gx,Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            SSE_LOOP_Q(x3, x4, OR(x3, x3, x4));
            break;
        case 0xEC:
            INST_NAME("PADDSB Gx,Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            for(int i=0; i<16; ++i) {
                // tmp16s = (int16_t)GX->sb[i] + EX->sb[i];
                // GX->sb[i] = (tmp16s>127)?127:((tmp16s<-128)?-128:tmp16s);
                LB(x3, gback, i);
                LB(x4, wback, fixedaddress+i);
                ADDW(x3, x3, x4);
                SLLIW(x3, x3, 16);
                SRAIW(x3, x3, 16);
                ADDI(x4, xZR, 0x7f);
                BLT(x3, x4, 12);     // tmp16s>127?
                SB(x4, gback, i);
                J(24);               // continue
                ADDI(x4, xZR, 0xf80);
                BLT(x4, x3, 12);     // tmp16s<-128?
                SB(x4, gback, i);
                J(8);                // continue
                SB(x3, gback, i);
            }
            break;
        case 0xED:
            INST_NAME("PADDSW Gx,Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            for(int i=0; i<8; ++i) {
                // tmp32s = (int32_t)GX->sw[i] + EX->sw[i];
                // GX->sw[i] = (tmp32s>32767)?32767:((tmp32s<-32768)?-32768:tmp32s);
                LH(x3, gback, 2*i);
                LH(x4, wback, fixedaddress+2*i);
                ADDW(x3, x3, x4);
                LUI(x4, 0xFFFF8); // -32768
                BGE(x3, x4, 12);
                SH(x4, gback, 2*i);
                J(20); // continue
                LUI(x4, 8); // 32768
                BLT(x3, x4, 8);
                ADDIW(x3, x4, -1);
                SH(x3, gback, 2*i);
            }
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
        case 0xF1:
            INST_NAME("PSLLQ Gx,Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            ADDI(x4, xZR, 16);
            LD(x3, wback, fixedaddress+0);
            BLTU_MARK(x3, x4);
            // just zero dest
            SD(xZR, gback, 0);
            SD(xZR, gback, 8);
            B_NEXT_nocond;
            MARK;
            for (int i=0; i<8; ++i) {
                LHU(x4, gback, 2*i);
                SLLW(x4, x4, x3);
                SH(x4, gback, 2*i);
            }
            break;
        case 0xF2:
            INST_NAME("PSLLQ Gx,Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            ADDI(x4, xZR, 32);
            LD(x3, wback, fixedaddress+0);
            BLTU_MARK(x3, x4);
            // just zero dest
            SD(xZR, gback, 0);
            SD(xZR, gback, 8);
            B_NEXT_nocond;
            MARK;
            for (int i=0; i<4; ++i) {
                LWU(x4, gback, 4*i);
                SLLW(x4, x4, x3);
                SW(x4, gback, 4*i);
            }
            break;
        case 0xF3:
            INST_NAME("PSLLQ Gx,Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            ADDI(x4, xZR, 64);
            LD(x3, wback, fixedaddress+0);
            BLTU_MARK(x3, x4);
            // just zero dest
            SD(xZR, gback, 0);
            SD(xZR, gback, 8);
            B_NEXT_nocond;
            MARK;
            for (int i=0; i<2; ++i) {
                LD(x4, gback, 8*i);
                SLL(x4, x4, x3);
                SD(x4, gback, 8*i);
            }
            break;
        case 0xF4:
            INST_NAME("PMULUDQ Gx,Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            // GX->q[1] = (uint64_t)EX->ud[2]*GX->ud[2];
            LWU(x3, gback, 2*4);
            LWU(x4, wback, fixedaddress+2*4);
            MUL(x3, x3, x4);
            SD(x3, gback, 8);
            // GX->q[0] = (uint64_t)EX->ud[0]*GX->ud[0];
            LWU(x3, gback, 0*4);
            LWU(x4, wback, fixedaddress+0*4);
            MUL(x3, x3, x4);
            SD(x3, gback, 0);
            break;
        case 0xF5:
            INST_NAME("PMADDWD Gx, Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            for (int i=0; i<4; ++i) {
                // GX->sd[i] = (int32_t)(GX->sw[i*2+0])*EX->sw[i*2+0] + 
                //             (int32_t)(GX->sw[i*2+1])*EX->sw[i*2+1];
                LH(x3, gback, 2*(i*2+0));
                LH(x4, wback, fixedaddress+2*(i*2+0));
                MULW(x5, x3, x4);
                LH(x3, gback, 2*(i*2+1));
                LH(x4, wback, fixedaddress+2*(i*2+1));
                MULW(x6, x3, x4);
                ADDW(x5, x5, x6);
                SW(x5, gback, 4*i);
            }
            break;
        case 0xF6:
            INST_NAME("PSADBW Gx, Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            MV(x6, xZR);
            for (int i=0; i<16; ++i) {
                LBU(x3, gback, i);
                LBU(x4, wback, fixedaddress+i);
                SUBW(x3, x3, x4);
                SRAIW(x5, x3, 31);
                XOR(x3, x5, x3);
                SUBW(x3, x3, x5);
                ANDI(x3, x3, 0xff);
                ADDW(x6, x6, x3);
                if (i==7 || i == 15) {
                    SD(x6, gback, i+1-8);
                    if (i==7) MV(x6, xZR);
                }
            }
            break;
        case 0xF8:
            INST_NAME("PSUBB Gx,Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            for(int i=0; i<16; ++i) {
                // GX->sb[i] -= EX->sb[i];
                LB(x3, wback, fixedaddress+i);
                LB(x4, gback, i);
                SUB(x3, x4, x3);
                SB(x3, gback, i);
            }
            break;
        case 0xF9:
            INST_NAME("PSUBW Gx,Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            SSE_LOOP_W(x3, x4, SUBW(x3, x3, x4));
            break;
        case 0xFA:
            INST_NAME("PSUBD Gx,Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            SSE_LOOP_D(x3, x4, SUBW(x3, x3, x4));
            break;
        case 0xFB:
            INST_NAME("PSUBQ Gx,Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            SSE_LOOP_Q(x3, x4, SUB(x3, x3, x4));
            break;
        case 0xFC:
            INST_NAME("PADDB Gx,Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            for(int i=0; i<16; ++i) {
                // GX->sb[i] += EX->sb[i];
                LB(x3, gback, i);
                LB(x4, wback, fixedaddress+i);
                ADDW(x3, x3, x4);
                SB(x3, gback, i);
            }
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
