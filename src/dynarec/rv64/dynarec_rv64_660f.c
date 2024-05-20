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
#include "bitutils.h"

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_functions.h"
#include "dynarec_rv64_helper.h"
#include "emu/x64compstrings.h"

uintptr_t dynarec64_660F(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8, s8;
    int32_t i32;
    uint8_t gd, ed;
    uint8_t wback, wb1, wb2, gback;
    uint8_t eb1, eb2;
    int64_t j64;
    uint64_t tmp64u, tmp64u2;
    int v0, v1;
    int q0, q1;
    int d0, d1, d2;
    int64_t fixedaddress, gdoffset;
    int unscaled;

    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(j64);

    static const int8_t round_round[] = { RD_RNE, RD_RDN, RD_RUP, RD_RTZ };

    switch (opcode) {
        case 0x10:
            INST_NAME("MOVUPD Gx,Ex");
            nextop = F8;
            GETEX(x1, 0);
            GETGX();
            SSE_LOOP_MV_Q(x3);
            break;
        case 0x11:
            INST_NAME("MOVUPD Ex,Gx");
            nextop = F8;
            GETEX(x1, 0);
            GETGX();
            SSE_LOOP_MV_Q2(x3);
            if (!MODREG) SMWRITE2();
            break;
        case 0x12:
            INST_NAME("MOVLPD Gx, Eq");
            nextop = F8;
            GETGX();
            if (MODREG) {
                // access register instead of memory is bad opcode!
                DEFAULT;
                return addr;
            }
            SMREAD();
            addr = geted(dyn, addr, ninst, nextop, &wback, x2, x3, &fixedaddress, rex, NULL, 1, 0);
            LD(x3, wback, fixedaddress);
            SD(x3, gback, gdoffset + 0);
            break;
        case 0x13:
            INST_NAME("MOVLPD Eq, Gx");
            nextop = F8;
            GETGX();
            if (MODREG) {
                // access register instead of memory is bad opcode!
                DEFAULT;
                return addr;
            }
            addr = geted(dyn, addr, ninst, nextop, &wback, x2, x3, &fixedaddress, rex, NULL, 1, 0);
            LD(x3, gback, gdoffset + 0);
            SD(x3, wback, fixedaddress);
            SMWRITE2();
            break;
        case 0x14:
            INST_NAME("UNPCKLPD Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            // GX->q[1] = EX->q[0];
            LD(x3, wback, fixedaddress + 0);
            SD(x3, gback, gdoffset + 8);
            break;
        case 0x15:
            INST_NAME("UNPCKHPD Gx, Ex");
            nextop = F8;
            GETEX(x1, 0);
            GETGX();
            // GX->q[0] = GX->q[1];
            LD(x3, gback, gdoffset + 8);
            SD(x3, gback, gdoffset + 0);
            // GX->q[1] = EX->q[1];
            LD(x3, wback, fixedaddress + 8);
            SD(x3, gback, gdoffset + 8);
            break;
        case 0x16:
            INST_NAME("MOVHPD Gx, Eq");
            nextop = F8;
            GETGX();
            if (MODREG) {
                // access register instead of memory is bad opcode!
                DEFAULT;
                return addr;
            }
            SMREAD();
            addr = geted(dyn, addr, ninst, nextop, &wback, x2, x3, &fixedaddress, rex, NULL, 1, 0);
            LD(x3, wback, fixedaddress);
            SD(x3, gback, gdoffset + 8);
            break;
        case 0x1F:
            INST_NAME("NOP (multibyte)");
            nextop = F8;
            FAKEED;
            break;
        case 0x28:
            INST_NAME("MOVAPD Gx,Ex");
            nextop = F8;
            GETEX(x1, 0);
            GETGX();
            SSE_LOOP_MV_Q(x3);
            break;
        case 0x29:
            INST_NAME("MOVAPD Ex,Gx");
            nextop = F8;
            GETEX(x1, 0);
            GETGX();
            SSE_LOOP_MV_Q2(x3);
            if (!MODREG) SMWRITE2();
            break;
        case 0x2A:
            INST_NAME("CVTPI2PD Gx,Em");
            nextop = F8;
            GETGX();
            GETEM(x2, 0);
            d0 = fpu_get_scratch(dyn);
            for (int i = 0; i < 2; ++i) {
                LW(x1, wback, fixedaddress + i * 4);
                FCVTDW(d0, x1, RD_RTZ);
                FSD(d0, gback, gdoffset + i * 8);
            }
            break;
        case 0x2B:
            INST_NAME("MOVNTPD Ex, Gx");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            SSE_LOOP_MV_Q2(x3);
            break;
        case 0x2C:
            INST_NAME("CVTTPD2PI Gm,Ex");
            nextop = F8;
            GETGM();
            GETEX(x2, 0);
            d0 = fpu_get_scratch(dyn);
            for (int i = 0; i < 2; ++i) {
                if (!box64_dynarec_fastround) {
                    FSFLAGSI(0); // // reset all bits
                }
                FLD(d0, wback, fixedaddress + i * 8);
                FCVTWD(x1, d0, RD_RTZ);
                if (!box64_dynarec_fastround) {
                    FRFLAGS(x5); // get back FPSR to check the IOC bit
                    ANDI(x5, x5, (1 << FR_NV) | (1 << FR_OF));
                    BEQ_MARKi(x5, xZR, i);
                    MOV32w(x1, 0x80000000);
                    MARKi(i);
                }
                SW(x1, gback, gdoffset + i * 4);
            }
            break;
        case 0x2D:
            INST_NAME("CVTPD2PI Gm,Ex");
            nextop = F8;
            GETGM();
            GETEX(x2, 0);
            d0 = fpu_get_scratch(dyn);
            u8 = sse_setround(dyn, ninst, x4, x5);
            for (int i = 0; i < 2; ++i) {
                if (!box64_dynarec_fastround) {
                    FSFLAGSI(0); // // reset all bits
                }
                FLD(d0, wback, fixedaddress + i * 8);
                FCVTWD(x1, d0, RD_DYN);
                if (!box64_dynarec_fastround) {
                    FRFLAGS(x5); // get back FPSR to check the IOC bit
                    ANDI(x5, x5, (1 << FR_NV) | (1 << FR_OF));
                    BEQ_MARKi(x5, xZR, i);
                    MOV32w(x1, 0x80000000);
                    MARKi(i);
                }
                SW(x1, gback, gdoffset + i * 4);
            }
            x87_restoreround(dyn, ninst, u8);
            break;
        case 0x2E:
            // no special check...
        case 0x2F:
            if (opcode == 0x2F) {
                INST_NAME("COMISD Gx, Ex");
            } else {
                INST_NAME("UCOMISD Gx, Ex");
            }
            SETFLAGS(X_ALL, SF_SET);
            SET_DFNONE();
            nextop = F8;
            GETGXSD(d0);
            GETEXSD(v0, 0);
            CLEAR_FLAGS();
            // if isnan(d0) || isnan(v0)
            IFX(X_ZF | X_PF | X_CF)
            {
                FEQD(x3, d0, d0);
                FEQD(x2, v0, v0);
                AND(x2, x2, x3);
                BNE_MARK(x2, xZR);
                ORI(xFlags, xFlags, (1 << F_ZF) | (1 << F_PF) | (1 << F_CF));
                B_NEXT_nocond;
            }
            MARK;
            // else if isless(d0, v0)
            IFX(X_CF)
            {
                FLTD(x2, d0, v0);
                BEQ_MARK2(x2, xZR);
                ORI(xFlags, xFlags, 1 << F_CF);
                B_NEXT_nocond;
            }
            MARK2;
            // else if d0 == v0
            IFX(X_ZF)
            {
                FEQD(x2, d0, v0);
                CBZ_NEXT(x2);
                ORI(xFlags, xFlags, 1 << F_ZF);
            }
            break;
        case 0x38: // SSSE3 opcodes
            nextop = F8;
            switch (nextop) {
                case 0x00:
                    INST_NAME("PSHUFB Gx, Ex");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0);
                    sse_forget_reg(dyn, ninst, x5);

                    ADDI(x5, xEmu, offsetof(x64emu_t, scratch));

                    // perserve gd
                    LD(x3, gback, gdoffset + 0);
                    LD(x4, gback, gdoffset + 8);
                    SD(x3, x5, 0);
                    SD(x4, x5, 8);

                    for (int i = 0; i < 16; ++i) {
                        LBU(x3, wback, fixedaddress + i);
                        ANDI(x4, x3, 128);
                        BEQZ(x4, 12);
                        SB(xZR, gback, gdoffset + i);
                        BEQZ(xZR, 20); // continue
                        ANDI(x4, x3, 15);
                        ADD(x4, x4, x5);
                        LBU(x4, x4, 0);
                        SB(x4, gback, gdoffset + i);
                    }
                    break;
                case 0x01:
                    INST_NAME("PHADDW Gx, Ex");
                    nextop = F8;
                    GETGX();
                    for (int i = 0; i < 4; ++i) {
                        // GX->sw[i] = GX->sw[i*2+0]+GX->sw[i*2+1];
                        LH(x3, gback, gdoffset + 2 * (i * 2 + 0));
                        LH(x4, gback, gdoffset + 2 * (i * 2 + 1));
                        ADDW(x3, x3, x4);
                        SH(x3, gback, gdoffset + 2 * i);
                    }
                    if (MODREG && gd == (nextop & 7) + (rex.b << 3)) {
                        // GX->q[1] = GX->q[0];
                        LD(x3, gback, gdoffset + 0);
                        SD(x3, gback, gdoffset + 8);
                    } else {
                        GETEX(x2, 0);
                        for (int i = 0; i < 4; ++i) {
                            // GX->sw[4+i] = EX->sw[i*2+0] + EX->sw[i*2+1];
                            LH(x3, wback, fixedaddress + 2 * (i * 2 + 0));
                            LH(x4, wback, fixedaddress + 2 * (i * 2 + 1));
                            ADDW(x3, x3, x4);
                            SH(x3, gback, gdoffset + 2 * (4 + i));
                        }
                    }
                    break;
                case 0x02:
                    INST_NAME("PHADDD Gx, Ex");
                    nextop = F8;
                    GETGX();
                    // GX->sd[0] += GX->sd[1];
                    LW(x3, gback, gdoffset + 0 * 4);
                    LW(x4, gback, gdoffset + 1 * 4);
                    ADDW(x3, x3, x4);
                    SW(x3, gback, gdoffset + 0 * 4);
                    // GX->sd[1] = GX->sd[2] + GX->sd[3];
                    LW(x3, gback, gdoffset + 2 * 4);
                    LW(x4, gback, gdoffset + 3 * 4);
                    ADDW(x3, x3, x4);
                    SW(x3, gback, gdoffset + 1 * 4);
                    if (MODREG && gd == (nextop & 7) + (rex.b << 3)) {
                        // GX->q[1] = GX->q[0];
                        LD(x3, gback, gdoffset + 0);
                        SD(x3, gback, gdoffset + 8);
                    } else {
                        GETEX(x2, 0);
                        // GX->sd[2] = EX->sd[0] + EX->sd[1];
                        LW(x3, wback, fixedaddress + 0 * 4);
                        LW(x4, wback, fixedaddress + 1 * 4);
                        ADDW(x3, x3, x4);
                        SW(x3, gback, gdoffset + 2 * 4);
                        // GX->sd[3] = EX->sd[2] + EX->sd[3];
                        LW(x3, wback, fixedaddress + 2 * 4);
                        LW(x4, wback, fixedaddress + 3 * 4);
                        ADDW(x3, x3, x4);
                        SW(x3, gback, gdoffset + 3 * 4);
                    }
                    break;
                case 0x03:
                    INST_NAME("PHADDSW Gx, Ex");
                    nextop = F8;
                    GETGX();
                    for (int i = 0; i < 4; ++i) {
                        // tmp32s = GX->sw[i*2+0]+GX->sw[i*2+1];
                        // GX->sw[i] = sat(tmp32s);
                        LH(x3, gback, gdoffset + 2 * (i * 2 + 0));
                        LH(x4, gback, gdoffset + 2 * (i * 2 + 1));
                        ADDW(x3, x3, x4);
                        SAT16(x3, x4);
                        SH(x3, gback, gdoffset + 2 * i);
                    }
                    if (MODREG && gd == (nextop & 7) + (rex.b << 3)) {
                        // GX->q[1] = GX->q[0];
                        LD(x3, gback, gdoffset + 0);
                        SD(x3, gback, gdoffset + 8);
                    } else {
                        GETEX(x2, 0);
                        for (int i = 0; i < 4; ++i) {
                            // tmp32s = EX->sw[i*2+0] + EX->sw[i*2+1];
                            // GX->sw[4+i] = sat(tmp32s);
                            LH(x3, wback, fixedaddress + 2 * (i * 2 + 0));
                            LH(x4, wback, fixedaddress + 2 * (i * 2 + 1));
                            ADDW(x3, x3, x4);
                            SAT16(x3, x4);
                            SH(x3, gback, gdoffset + 2 * (4 + i));
                        }
                    }
                    break;
                case 0x04:
                    INST_NAME("PADDUBSW Gx, Ex");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0);
                    MOV64x(x5, 32767);
                    MOV64x(x6, -32768);
                    for (int i = 0; i < 8; ++i) {
                        LBU(x3, gback, gdoffset + i * 2);
                        LB(x4, wback, fixedaddress + i * 2);
                        MUL(x9, x3, x4);
                        LBU(x3, gback, gdoffset + i * 2 + 1);
                        LB(x4, wback, fixedaddress + i * 2 + 1);
                        MUL(x3, x3, x4);
                        ADD(x3, x3, x9);
                        if (rv64_zbb) {
                            MIN(x3, x3, x5);
                            MAX(x3, x3, x6);
                        } else {
                            BLT(x3, x5, 4 + 4);
                            MV(x3, x5);
                            BLT(x6, x3, 4 + 4);
                            MV(x3, x6);
                        }
                        SH(x3, gback, gdoffset + i * 2);
                    }
                    break;
                case 0x05:
                    INST_NAME("PHSUBW Gx, Ex");
                    nextop = F8;
                    GETGX();
                    for (int i = 0; i < 4; ++i) {
                        // GX->sw[i] = GX->sw[i*2+0] - GX->sw[i*2+1];
                        LH(x3, gback, gdoffset + 2 * (i * 2 + 0));
                        LH(x4, gback, gdoffset + 2 * (i * 2 + 1));
                        SUBW(x3, x3, x4);
                        SH(x3, gback, gdoffset + 2 * i);
                    }
                    if (MODREG && gd == (nextop & 7) + (rex.b << 3)) {
                        // GX->q[1] = GX->q[0];
                        LD(x3, gback, gdoffset + 0);
                        SD(x3, gback, gdoffset + 8);
                    } else {
                        GETEX(x2, 0);
                        for (int i = 0; i < 4; ++i) {
                            // GX->sw[4+i] = EX->sw[i*2+0] - EX->sw[i*2+1];
                            LH(x3, wback, fixedaddress + 2 * (i * 2 + 0));
                            LH(x4, wback, fixedaddress + 2 * (i * 2 + 1));
                            SUBW(x3, x3, x4);
                            SH(x3, gback, gdoffset + 2 * (4 + i));
                        }
                    }
                    break;
                case 0x08:
                    INST_NAME("PSIGNB Gx, Ex");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0);
                    for (int i = 0; i < 16; ++i) {
                        LB(x3, gback, gdoffset + i);
                        LB(x4, wback, fixedaddress + i);
                        BGE(x4, xZR, 4 + 4);
                        NEG(x3, x3);
                        BNE(x4, xZR, 4 + 4);
                        MOV_U12(x3, 0);
                        SB(x3, gback, gdoffset + i);
                    }
                    break;
                case 0x09:
                    INST_NAME("PSIGNW Gx, Ex");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0);
                    for (int i = 0; i < 8; ++i) {
                        LH(x3, gback, gdoffset + i * 2);
                        LH(x4, wback, fixedaddress + i * 2);
                        BGE(x4, xZR, 4 + 4);
                        NEG(x3, x3);
                        BNE(x4, xZR, 4 + 4);
                        MOV_U12(x3, 0);
                        SH(x3, gback, gdoffset + i * 2);
                    }
                    break;
                case 0x0A:
                    INST_NAME("PSIGND Gx, Ex");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0);
                    for (int i = 0; i < 4; ++i) {
                        LW(x3, gback, gdoffset + i * 4);
                        LW(x4, wback, fixedaddress + i * 4);
                        BGE(x4, xZR, 4 + 4);
                        NEG(x3, x3);
                        BNE(x4, xZR, 4 + 4);
                        ADDI(x3, xZR, 0);
                        SW(x3, gback, gdoffset + i * 4);
                    }
                    break;
                case 0x0B:
                    INST_NAME("PMULHRSW Gx, Ex");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0);
                    for (int i = 0; i < 8; ++i) {
                        LH(x3, gback, gdoffset + i * 2);
                        LH(x4, wback, fixedaddress + i * 2);
                        MUL(x3, x3, x4);
                        SRAI(x3, x3, 14);
                        ADDI(x3, x3, 1);
                        SRAI(x3, x3, 1);
                        SH(x3, gback, gdoffset + i * 2);
                    }
                    break;
                case 0x10:
                    INST_NAME("PBLENDVB Gx,Ex");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0);
                    sse_forget_reg(dyn, ninst, 0); // forget xmm[0]
                    for (int i = 0; i < 16; ++i) {
                        LB(x3, xEmu, offsetof(x64emu_t, xmm[0]) + i);
                        BGE(x3, xZR, 12); // continue
                        LBU(x3, wback, fixedaddress + i);
                        SB(x3, gback, gdoffset + i);
                        // continue
                    }
                    break;
                case 0x14:
                    INST_NAME("PBLENDVPS Gx,Ex");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0);
                    for (int i = 0; i < 4; ++i) {
                        LW(x3, xEmu, offsetof(x64emu_t, xmm[0]) + i * 4);
                        BGE(x3, xZR, 4 + 4 * 2);
                        LWU(x3, wback, fixedaddress + i * 4);
                        SW(x3, gback, gdoffset + i * 4);
                    }
                    break;
                case 0x17:
                    INST_NAME("PTEST Gx, Ex");
                    nextop = F8;
                    SETFLAGS(X_ALL, SF_SET);
                    GETGX();
                    GETEX(x2, 0);
                    CLEAR_FLAGS();
                    SET_DFNONE();
                    IFX(X_ZF | X_CF)
                    {
                        LD(x5, wback, fixedaddress + 0);
                        LD(x6, wback, fixedaddress + 8);

                        IFX(X_ZF)
                        {
                            LD(x3, gback, gdoffset + 0);
                            LD(x4, gback, gdoffset + 8);
                            AND(x3, x3, x5);
                            AND(x4, x4, x6);
                            OR(x3, x3, x4);
                            BNEZ(x3, 8);
                            ORI(xFlags, xFlags, 1 << F_ZF);
                        }
                        IFX(X_CF)
                        {
                            LD(x3, gback, gdoffset + 0);
                            NOT(x3, x3);
                            LD(x4, gback, gdoffset + 8);
                            NOT(x4, x4);
                            AND(x3, x3, x5);
                            AND(x4, x4, x6);
                            OR(x3, x3, x4);
                            BNEZ(x3, 8);
                            ORI(xFlags, xFlags, 1 << F_CF);
                        }
                    }
                    break;

                case 0x1C:
                    INST_NAME("PABSB Gx, Ex");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0);
                    for (int i = 0; i < 16; ++i) {
                        LB(x4, wback, fixedaddress + i);
                        BGE(x4, xZR, 4 + 4);
                        NEG(x4, x4);
                        SB(x4, gback, gdoffset + i);
                    }
                    break;
                case 0x1D:
                    INST_NAME("PABSW Gx, Ex");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0);
                    for (int i = 0; i < 8; ++i) {
                        LH(x4, wback, fixedaddress + i * 2);
                        BGE(x4, xZR, 4 + 4);
                        NEG(x4, x4);
                        SH(x4, gback, gdoffset + i * 2);
                    }
                    break;
                case 0x1E:
                    INST_NAME("PABSD Gx, Ex");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0);
                    MOV64x(x5, ~(1 << 31));
                    for (int i = 0; i < 4; ++i) {
                        LW(x4, wback, fixedaddress + i * 4);
                        BGE(x4, xZR, 4 + 4);
                        NEG(x4, x4);
                        SW(x4, gback, gdoffset + i * 4);
                    }
                    break;
                case 0x20:
                    INST_NAME("PMOVSXBW Gx, Ex"); // SSE4 opcode!
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0);
                    for (int i = 7; i >= 0; --i) {
                        // GX->sw[i] = EX->sb[i];
                        LB(x3, wback, fixedaddress + i);
                        SH(x3, gback, gdoffset + i * 2);
                    }
                    break;
                case 0x21:
                    INST_NAME("PMOVSXBD Gx, Ex"); // SSE4 opcode!
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0);
                    for (int i = 3; i >= 0; --i) {
                        // GX->sd[i] = EX->sb[i];
                        LB(x3, wback, fixedaddress + i);
                        SW(x3, gback, gdoffset + i * 4);
                    }
                    break;
                case 0x22:
                    INST_NAME("PMOVSXBQ Gx, Ex"); // SSE4 opcode!
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0);
                    for (int i = 1; i >= 0; --i) {
                        // GX->sq[i] = EX->sb[i];
                        LB(x3, wback, fixedaddress + i);
                        SD(x3, gback, gdoffset + i * 8);
                    }
                    break;
                case 0x23:
                    INST_NAME("PMOVSXWD Gx, Ex"); // SSE4 opcode!
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0);
                    for (int i = 3; i >= 0; --i) {
                        // GX->sd[i] = EX->sw[i];
                        LH(x3, wback, fixedaddress + i * 2);
                        SW(x3, gback, gdoffset + i * 4);
                    }
                    break;
                case 0x24:
                    INST_NAME("PMOVSXWQ Gx, Ex"); // SSE4 opcode!
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0);
                    for (int i = 1; i >= 0; --i) {
                        // GX->sq[i] = EX->sw[i];
                        LH(x3, wback, fixedaddress + i * 2);
                        SD(x3, gback, gdoffset + i * 8);
                    }
                    break;
                case 0x25:
                    INST_NAME("PMOVSXDQ Gx, Ex");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0);
                    for (int i = 1; i >= 0; --i) {
                        // GX->sq[i] = EX->sd[i];
                        LW(x4, wback, fixedaddress + i * 4);
                        SD(x4, gback, gdoffset + i * 8);
                    }
                    break;
                case 0x28:
                    INST_NAME("PMULDQ Gx, Ex");
                    nextop = F8;
                    GETEX(x2, 0);
                    GETGX();
                    for (int i = 1; i >= 0; --i) {
                        LW(x3, wback, fixedaddress + i * 8);
                        LW(x4, gback, gdoffset + i * 8);
                        MUL(x3, x3, x4);
                        SD(x3, gback, gdoffset + i * 8);
                    }
                    break;
                case 0x2B:
                    INST_NAME("PACKUSDW Gx, Ex");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0);
                    MOV64x(x5, 65535);
                    for (int i = 0; i < 4; ++i) {
                        LW(x3, gback, gdoffset + i * 4);
                        if (rv64_zbb) {
                            MIN(x3, x3, x5);
                            MAX(x3, x3, xZR);
                        } else {
                            BGE(x3, xZR, 4 + 4);
                            MV(x3, xZR);
                            BLT(x3, x5, 4 + 4);
                            MV(x3, x5);
                        }
                        SH(x3, gback, gdoffset + i * 2);
                    }
                    if (MODREG && gd == ed) {
                        LD(x3, gback, gdoffset + 0);
                        SD(x3, gback, gdoffset + 8);
                    } else
                        for (int i = 0; i < 4; ++i) {
                            LW(x3, wback, fixedaddress + i * 4);
                            if (rv64_zbb) {
                                MIN(x3, x3, x5);
                                MAX(x3, x3, xZR);
                            } else {
                                BGE(x3, xZR, 4 + 4);
                                MV(x3, xZR);
                                BLT(x3, x5, 4 + 4);
                                MV(x3, x5);
                            }
                            SH(x3, gback, gdoffset + 8 + i * 2);
                        }
                    break;

                case 0x30:
                    INST_NAME("PMOVZXBW Gx, Ex");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0);
                    for (int i = 7; i >= 0; --i) {
                        LBU(x3, wback, fixedaddress + i);
                        SH(x3, gback, gdoffset + i * 2);
                    }
                    break;
                case 0x31:
                    INST_NAME("PMOVZXBD Gx, Ex");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0);
                    for (int i = 3; i >= 0; --i) {
                        LBU(x3, wback, fixedaddress + i);
                        SW(x3, gback, gdoffset + i * 4);
                    }
                    break;
                case 0x32:
                    INST_NAME("PMOVZXBQ Gx, Ex");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0);
                    for (int i = 1; i >= 0; --i) {
                        LBU(x3, wback, fixedaddress + i);
                        SD(x3, gback, gdoffset + i * 8);
                    }
                    break;
                case 0x33:
                    INST_NAME("PMOVZXWD Gx, Ex");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0);
                    for (int i = 3; i >= 0; --i) {
                        LHU(x3, wback, fixedaddress + i * 2);
                        SW(x3, gback, gdoffset + i * 4);
                    }
                    break;
                case 0x34:
                    INST_NAME("PMOVZXWQ Gx, Ex");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0);
                    for (int i = 1; i >= 0; --i) {
                        LHU(x3, wback, fixedaddress + i * 2);
                        SD(x3, gback, gdoffset + i * 8);
                    }
                    break;
                case 0x35:
                    INST_NAME("PMOVZXDQ Gx, Ex");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0);
                    for (int i = 1; i >= 0; --i) {
                        LWU(x3, wback, fixedaddress + i * 4);
                        SD(x3, gback, gdoffset + i * 8);
                    }
                    break;

                case 0x38:
                    INST_NAME("PMINSB Gx, Ex"); // SSE4 opcode!
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0);
                    for (int i = 0; i < 16; ++i) {
                        LB(x3, gback, gdoffset + i);
                        LB(x4, wback, fixedaddress + i);
                        if (rv64_zbb)
                            MIN(x4, x3, x4);
                        else
                            BLT(x3, x4, 4 + 4);
                        SB(x4, gback, gdoffset + i);
                    }
                    break;
                case 0x39:
                    INST_NAME("PMINSD Gx, Ex"); // SSE4 opcode!
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0);
                    for (int i = 0; i < 4; ++i) {
                        LW(x3, gback, gdoffset + i * 4);
                        LW(x4, wback, fixedaddress + i * 4);
                        if (rv64_zbb)
                            MIN(x4, x3, x4);
                        else
                            BLT(x3, x4, 4 + 4);
                        SW(x4, gback, gdoffset + i * 4);
                    }
                    break;
                case 0x3A:
                    INST_NAME("PMINUW Gx, Ex"); // SSE4 opcode!
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0);
                    for (int i = 0; i < 8; ++i) {
                        LHU(x3, gback, gdoffset + i * 2);
                        LHU(x4, wback, fixedaddress + i * 2);
                        if (rv64_zbb)
                            MINU(x4, x3, x4);
                        else
                            BLTU(x3, x4, 4 + 4);
                        SH(x4, gback, gdoffset + i * 2);
                    }
                    break;
                case 0x3B:
                    INST_NAME("PMINUD Gx, Ex"); // SSE4 opcode!
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0);
                    for (int i = 0; i < 4; ++i) {
                        LWU(x3, gback, gdoffset + i * 4);
                        LWU(x4, wback, fixedaddress + i * 4);
                        if (rv64_zbb)
                            MINU(x4, x3, x4);
                        else
                            BLTU(x3, x4, 4 + 4);
                        SW(x4, gback, gdoffset + i * 4);
                    }
                    break;
                case 0x3C:
                    INST_NAME("PMAXSB Gx, Ex"); // SSE4 opcode!
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0);
                    for (int i = 0; i < 16; ++i) {
                        LB(x3, gback, gdoffset + i);
                        LB(x4, wback, fixedaddress + i);
                        if (rv64_zbb)
                            MAX(x4, x3, x4);
                        else
                            BLT(x4, x3, 4 + 4);
                        SB(x4, gback, gdoffset + i);
                    }
                    break;
                case 0x3D:
                    INST_NAME("PMAXSD Gx, Ex"); // SSE4 opcode!
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0);
                    for (int i = 0; i < 4; ++i) {
                        LW(x3, gback, gdoffset + i * 4);
                        LW(x4, wback, fixedaddress + i * 4);
                        if (rv64_zbb)
                            MAX(x4, x3, x4);
                        else
                            BLT(x4, x3, 4 + 4);
                        SW(x4, gback, gdoffset + i * 4);
                    }
                    break;
                case 0x3E:
                    INST_NAME("PMAXUW Gx, Ex"); // SSE4 opcode!
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0);
                    for (int i = 0; i < 8; ++i) {
                        LHU(x3, gback, gdoffset + i * 2);
                        LHU(x4, wback, fixedaddress + i * 2);
                        if (rv64_zbb)
                            MAXU(x4, x3, x4);
                        else
                            BLTU(x4, x3, 4 + 4);
                        SH(x4, gback, gdoffset + i * 2);
                    }
                    break;
                case 0x3F:
                    INST_NAME("PMAXUD Gx, Ex"); // SSE4 opcode!
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0);
                    for (int i = 0; i < 4; ++i) {
                        LWU(x3, gback, gdoffset + i * 4);
                        LWU(x4, wback, fixedaddress + i * 4);
                        if (rv64_zbb)
                            MAXU(x4, x3, x4);
                        else
                            BLTU(x4, x3, 4 + 4);
                        SW(x4, gback, gdoffset + i * 4);
                    }
                    break;
                case 0x40:
                    INST_NAME("PMULLD Gx, Ex");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0);
                    for (int i = 0; i < 4; ++i) {
                        LW(x3, gback, gdoffset + i * 4);
                        LW(x4, wback, fixedaddress + i * 4);
                        MUL(x3, x3, x4);
                        SW(x3, gback, gdoffset + i * 4);
                    }
                    break;
                case 0x61:
                    INST_NAME("PCMPESTRI Gx, Ex, Ib");
                    SETFLAGS(X_ALL, SF_SET_DF);
                    nextop = F8;
                    GETG;
                    sse_reflect_reg(dyn, ninst, gd);
                    ADDI(x3, xEmu, offsetof(x64emu_t, xmm[gd]));
                    if (MODREG) {
                        ed = (nextop & 7) + (rex.b << 3);
                        sse_reflect_reg(dyn, ninst, ed);
                        ADDI(x1, xEmu, offsetof(x64emu_t, xmm[ed]));
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x1, x2, &fixedaddress, rex, NULL, 0, 1);
                        if (ed != x1) {
                            MV(x1, ed);
                        }
                    }
                    // prepare rest arguments
                    MV(x2, xRDX);
                    MV(x4, xRAX);
                    u8 = F8;
                    MOV32w(x5, u8);
                    CALL(sse42_compare_string_explicit_len, x1);
                    ZEROUP(x1);
                    BNEZ_MARK(x1);
                    MOV32w(xRCX, (u8 & 1) ? 8 : 16);
                    B_NEXT_nocond;
                    MARK;
                    if (u8 & 0b1000000) {
                        CLZxw(xRCX, x1, 0, x2, x3, x4);
                        ADDI(x2, xZR, 31);
                        SUB(xRCX, x2, xRCX);
                    } else {
                        CTZxw(xRCX, xRCX, 0, x1, x2);
                    }
                    break;
                case 0xDB:
                    INST_NAME("AESIMC Gx, Ex"); // AES-NI
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0);
                    SSE_LOOP_MV_Q(x3);
                    sse_forget_reg(dyn, ninst, gd);
                    MOV32w(x1, gd);
                    CALL(native_aesimc, -1);
                    break;
                case 0xDC:
                    INST_NAME("AESENC Gx, Ex"); // AES-NI
                    nextop = F8;
                    GETG;
                    sse_forget_reg(dyn, ninst, gd);
                    MOV32w(x1, gd);
                    CALL(native_aese, -1);
                    GETGX();
                    GETEX(x2, 0);
                    SSE_LOOP_Q(x3, x4, XOR(x3, x3, x4));
                    break;
                case 0xDD:
                    INST_NAME("AESENCLAST Gx, Ex"); // AES-NI
                    nextop = F8;
                    GETG;
                    sse_forget_reg(dyn, ninst, gd);
                    MOV32w(x1, gd);
                    CALL(native_aeselast, -1);
                    GETGX();
                    GETEX(x2, 0);
                    SSE_LOOP_Q(x3, x4, XOR(x3, x3, x4));
                    break;
                case 0xDE:
                    INST_NAME("AESDEC Gx, Ex"); // AES-NI
                    nextop = F8;
                    GETG;
                    sse_forget_reg(dyn, ninst, gd);
                    MOV32w(x1, gd);
                    CALL(native_aesd, -1);
                    GETGX();
                    GETEX(x2, 0);
                    SSE_LOOP_Q(x3, x4, XOR(x3, x3, x4));
                    break;

                case 0xDF:
                    INST_NAME("AESDECLAST Gx, Ex"); // AES-NI
                    nextop = F8;
                    GETG;
                    sse_forget_reg(dyn, ninst, gd);
                    MOV32w(x1, gd);
                    CALL(native_aesdlast, -1);
                    GETGX();
                    GETEX(x2, 0);
                    SSE_LOOP_Q(x3, x4, XOR(x3, x3, x4));
                    break;
                case 0xF0:
                    INST_NAME("MOVBE Gw, Ew");
                    nextop = F8;
                    GETGD;
                    SMREAD();
                    addr = geted(dyn, addr, ninst, nextop, &ed, x3, x2, &fixedaddress, rex, NULL, 1, 0);
                    LHU(x1, ed, fixedaddress);
                    if (rv64_zbb) {
                        REV8(x1, x1);
                        SRLI(x1, x1, 48);
                    } else if (rv64_xtheadbb) {
                        TH_REVW(x1, x1);
                        SRLI(x1, x1, 16);
                    } else {
                        ANDI(x2, x1, 0xff);
                        SLLI(x2, x2, 8);
                        SRLI(x1, x1, 8);
                        OR(x1, x1, x2);
                    }
                    LUI(x2, 0xffff0);
                    AND(gd, gd, x2);
                    OR(gd, gd, x1);
                    break;
                case 0xF1:
                    INST_NAME("MOVBE Ew, Gw");
                    nextop = F8;
                    GETGD;
                    SMREAD();
                    addr = geted(dyn, addr, ninst, nextop, &wback, x3, x2, &fixedaddress, rex, NULL, 1, 0);
                    if (rv64_zbb) {
                        REV8(x1, gd);
                        SRLI(x1, x1, 48);
                    } else if (rv64_xtheadbb) {
                        TH_REVW(x1, gd);
                        SRLI(x1, x1, 16);
                    } else {
                        ANDI(x1, gd, 0xff);
                        SLLI(x1, x1, 8);
                        SRLI(x2, gd, 8);
                        ANDI(x2, x2, 0xff);
                        OR(x1, x1, x2);
                    }
                    SH(x1, wback, fixedaddress);
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0x3A: // these are some more SSSE3+ opcodes
            opcode = F8;
            switch (opcode) {
                case 0x0B:
                    INST_NAME("ROUNDSD Gx, Ex, Ib");
                    nextop = F8;
                    GETEXSD(d0, 1);
                    GETGXSD_empty(v0);
                    d1 = fpu_get_scratch(dyn);
                    v1 = fpu_get_scratch(dyn);
                    u8 = F8;
                    FEQD(x2, d0, d0);
                    BNEZ_MARK(x2);
                    if (v0 != d0) FMVD(v0, d0);
                    B_NEXT_nocond;
                    MARK; // d0 is not nan
                    FABSD(v1, d0);
                    MOV64x(x3, 1ULL << __DBL_MANT_DIG__);
                    FCVTDL(d1, x3, RD_RTZ);
                    FLTD(x3, v1, d1);
                    BNEZ_MARK2(x3);
                    if (v0 != d0) FMVD(v0, d0);
                    B_NEXT_nocond;
                    MARK2;
                    if (u8 & 4) {
                        u8 = sse_setround(dyn, ninst, x4, x2);
                        FCVTLD(x5, d0, RD_DYN);
                        FCVTDL(v0, x5, RD_RTZ);
                        x87_restoreround(dyn, ninst, u8);
                    } else {
                        FCVTLD(x5, d0, round_round[u8 & 3]);
                        FCVTDL(v0, x5, RD_RTZ);
                    }
                    break;
                case 0x09:
                    INST_NAME("ROUNDPD Gx, Ex, Ib");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 1);
                    u8 = F8;
                    d0 = fpu_get_scratch(dyn);
                    d1 = fpu_get_scratch(dyn);
                    v1 = fpu_get_scratch(dyn);
                    MOV64x(x3, 1ULL << __DBL_MANT_DIG__);
                    FCVTDL(d1, x3, RD_RTZ);

                    // i = 0
                    FLD(d0, wback, fixedaddress);
                    FEQD(x4, d0, d0);
                    BNEZ(x4, 8);
                    B_MARK_nocond;
                    // d0 is not nan
                    FABSD(v1, d0);
                    FLTD(x4, v1, d1);
                    BNEZ(x4, 8);
                    B_MARK_nocond;
                    if (u8 & 4) {
                        u8 = sse_setround(dyn, ninst, x4, x5);
                        FCVTLD(x5, d0, RD_DYN);
                        FCVTDL(d0, x5, RD_RTZ);
                        x87_restoreround(dyn, ninst, u8);
                    } else {
                        FCVTLD(x5, d0, round_round[u8 & 3]);
                        FCVTDL(d0, x5, RD_RTZ);
                    }
                    MARK;
                    FSD(d0, gback, gdoffset + 0);

                    // i = 1
                    FLD(d0, wback, fixedaddress + 8);
                    FEQD(x4, d0, d0);
                    BNEZ(x4, 8);
                    B_MARK2_nocond;
                    // d0 is not nan
                    FABSD(v1, d0);
                    FLTD(x4, v1, d1);
                    BNEZ(x4, 8);
                    B_MARK2_nocond;
                    if (u8 & 4) {
                        u8 = sse_setround(dyn, ninst, x4, x5);
                        FCVTLD(x5, d0, RD_DYN);
                        FCVTDL(d0, x5, RD_RTZ);
                        x87_restoreround(dyn, ninst, u8);
                    } else {
                        FCVTLD(x5, d0, round_round[u8 & 3]);
                        FCVTDL(d0, x5, RD_RTZ);
                    }
                    MARK2;
                    FSD(d0, gback, gdoffset + 8);
                    break;
                case 0x0E:
                    INST_NAME("PBLENDW Gx, Ex, Ib");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 1);
                    u8 = F8;
                    i32 = 0;
                    if (MODREG && gd == ed) break;
                    while (u8)
                        if (u8 & 1) {
                            if (!(i32 & 1) && u8 & 2) {
                                if (!(i32 & 3) && (u8 & 0xf) == 0xf) {
                                    // whole 64bits
                                    LD(x3, wback, fixedaddress + 8 * (i32 >> 2));
                                    SD(x3, gback, gdoffset + 8 * (i32 >> 2));
                                    i32 += 4;
                                    u8 >>= 4;
                                } else {
                                    // 32bits
                                    LWU(x3, wback, fixedaddress + 4 * (i32 >> 1));
                                    SW(x3, gback, gdoffset + 4 * (i32 >> 1));
                                    i32 += 2;
                                    u8 >>= 2;
                                }
                            } else {
                                // 16 bits
                                LHU(x3, wback, fixedaddress + 2 * i32);
                                SH(x3, gback, gdoffset + 2 * i32);
                                i32++;
                                u8 >>= 1;
                            }
                        } else {
                            // nope
                            i32++;
                            u8 >>= 1;
                        }
                    break;
                case 0x0F:
                    INST_NAME("PALIGNR Gx, Ex, Ib");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 1);
                    u8 = F8;
                    sse_forget_reg(dyn, ninst, x5);
                    ADDI(x5, xEmu, offsetof(x64emu_t, scratch));
                    // perserve gd
                    LD(x3, gback, gdoffset + 0);
                    LD(x4, gback, gdoffset + 8);
                    SD(x3, x5, 0);
                    SD(x4, x5, 8);
                    if (u8 > 31) {
                        SD(xZR, gback, gdoffset + 0);
                        SD(xZR, gback, gdoffset + 8);
                    } else {
                        for (int i = 0; i < 16; ++i, ++u8) {
                            if (u8 > 15) {
                                if (u8 > 31) {
                                    SB(xZR, gback, gdoffset + i);
                                    continue;
                                } else
                                    LBU(x3, x5, u8 - 16);
                            } else {
                                LBU(x3, wback, fixedaddress + u8);
                            }
                            SB(x3, gback, gdoffset + i);
                        }
                    }
                    break;
                case 0x16:
                    if (rex.w) {
                        INST_NAME("PEXTRQ Ed, Gx, Ib");
                    } else {
                        INST_NAME("PEXTRD Ed, Gx, Ib");
                    }
                    nextop = F8;
                    GETGX();
                    GETED(1);
                    u8 = F8;
                    if (rex.w)
                        LD(ed, gback, gdoffset + 8 * (u8 & 1));
                    else
                        LWU(ed, gback, gdoffset + 4 * (u8 & 3));
                    if (wback) {
                        SDxw(ed, wback, fixedaddress);
                        SMWRITE2();
                    }
                    break;
                case 0x20:
                    INST_NAME("PINSRB Gx, ED, Ib");
                    nextop = F8;
                    GETGX();
                    GETED(1);
                    u8 = F8;
                    SB(ed, gback, gdoffset + u8 & 0xF);
                    break;
                case 0x21:
                    INST_NAME("INSERTPS GX, EX, Ib");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 1);
                    u8 = F8;
                    if (MODREG)
                        s8 = (u8 >> 6) & 3;
                    else
                        s8 = 0;
                    // GX->ud[(tmp8u>>4)&3] = EX->ud[tmp8s];
                    LWU(x3, wback, fixedaddress + 4 * s8);
                    SW(x3, gback, gdoffset + 4 * (u8 >> 4));
                    for (int i = 0; i < 4; ++i) {
                        if (u8 & (1 << i))
                            // GX->ud[i] = 0;
                            SW(xZR, gback, gdoffset + 4 * i);
                    }
                    break;
                case 0x22:
                    INST_NAME("PINSRD Gx, ED, Ib");
                    nextop = F8;
                    GETGX();
                    GETED(1);
                    u8 = F8;
                    if (rex.w) {
                        SD(ed, gback, gdoffset + 8 * (u8 & 0x1));
                    } else {
                        SW(ed, gback, gdoffset + 4 * (u8 & 0x3));
                    }
                    break;
                case 0x40:
                    INST_NAME("DPPS Gx, Ex, Ib");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 1);
                    u8 = F8;
                    d0 = fpu_get_scratch(dyn);
                    d1 = fpu_get_scratch(dyn);
                    d2 = fpu_get_scratch(dyn);
                    FMVWX(d2, xZR);
                    for (int i = 0; i < 4; ++i)
                        if (u8 & (1 << (i + 4))) {
                            FLW(d0, gback, gdoffset + i * 4);
                            FLW(d1, wback, fixedaddress + i * 4);
                            FMULS(d0, d0, d1);
                            FADDS(d2, d2, d0);
                        }
                    for (int i = 0; i < 4; ++i)
                        if (u8 & (1 << i))
                            FSW(d2, gback, gdoffset + i * 4);
                        else
                            SW(xZR, gback, gdoffset + i * 4);
                    break;
                case 0x44:
                    INST_NAME("PCLMULQDQ Gx, Ex, Ib");
                    nextop = F8;
                    GETG;
                    sse_forget_reg(dyn, ninst, gd);
                    MOV32w(x1, gd); // gx
                    if (MODREG) {
                        ed = (nextop & 7) + (rex.b << 3);
                        sse_forget_reg(dyn, ninst, ed);
                        MOV32w(x2, ed);
                        MOV32w(x3, 0); // p = NULL
                    } else {
                        MOV32w(x2, 0);
                        addr = geted(dyn, addr, ninst, nextop, &ed, x3, x5, &fixedaddress, rex, NULL, 0, 1);
                        if (ed != x3) {
                            MV(x3, ed);
                        }
                    }
                    u8 = F8;
                    MOV32w(x4, u8);
                    CALL(native_pclmul, -1);
                    break;
                case 0xDF:
                    INST_NAME("AESKEYGENASSIST Gx, Ex, Ib"); // AES-NI
                    nextop = F8;
                    GETG;
                    sse_forget_reg(dyn, ninst, gd);
                    MOV32w(x1, gd); // gx
                    if (MODREG) {
                        ed = (nextop & 7) + (rex.b << 3);
                        sse_forget_reg(dyn, ninst, ed);
                        MOV32w(x2, ed);
                        MOV32w(x3, 0); // p = NULL
                    } else {
                        MOV32w(x2, 0);
                        addr = geted(dyn, addr, ninst, nextop, &ed, x3, x2, &fixedaddress, rex, NULL, 0, 1);
                        if (ed != x3) {
                            MV(x3, ed);
                        }
                    }
                    u8 = F8;
                    MOV32w(x4, u8);
                    CALL(native_aeskeygenassist, -1);
                    break;
                default:
                    DEFAULT;
            }
            break;
#define GO(GETFLAGS, NO, YES, F)                                                             \
    READFLAGS(F);                                                                            \
    GETFLAGS;                                                                                \
    nextop = F8;                                                                             \
    GETGD;                                                                                   \
    if (MODREG) {                                                                            \
        ed = xRAX + (nextop & 7) + (rex.b << 3);                                             \
        ZEXTH(x4, ed);                                                                       \
        ed = x4;                                                                             \
    } else {                                                                                 \
        SMREAD();                                                                            \
        addr = geted(dyn, addr, ninst, nextop, &ed, x2, x4, &fixedaddress, rex, NULL, 1, 0); \
        LHU(x4, ed, fixedaddress);                                                           \
        ed = x4;                                                                             \
    }                                                                                        \
    B##NO(x1, 4 + 3 * 4);                                                                    \
    LUI(x3, 0xffff0);                                                                        \
    AND(gd, gd, x3);                                                                         \
    OR(gd, gd, ed);

            GOCOND(0x40, "CMOV", "Gw, Ew");
#undef GO
        case 0x50:
            INST_NAME("PMOVMSKD Gd, Ex");
            nextop = F8;
            GETGD;
            GETEX(x1, 0);
            MV(gd, xZR);
            for (int i = 0; i < 2; ++i) {
                // GD->dword[0] |= ((EX->q[i]>>63)&1)<<i;
                LD(x2, wback, fixedaddress + 8 * i);
                SRLI(x2, x2, 63);
                if (i) SLLI(x2, x2, 1);
                OR(gd, gd, x2);
            }
            break;
        case 0x51:
            INST_NAME("SQRTPD Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            d0 = fpu_get_scratch(dyn);
            if (!box64_dynarec_fastnan) {
                d1 = fpu_get_scratch(dyn);
                FMVDX(d1, xZR);
            }
            for (int i = 0; i < 2; ++i) {
                FLD(d0, wback, fixedaddress + i * 8);
                if (!box64_dynarec_fastnan) {
                    FLTD(x3, d0, d1);
                }
                FSQRTD(d0, d0);
                if (!box64_dynarec_fastnan) {
                    BEQ(x3, xZR, 8);
                    FNEGD(d0, d0);
                }
                FSD(d0, gback, gdoffset + i * 8);
            }
            break;
        case 0x54:
            INST_NAME("ANDPD Gx, Ex");
            nextop = F8;
            GETEX(x1, 0);
            GETGX();
            SSE_LOOP_Q(x3, x4, AND(x3, x3, x4));
            break;
        case 0x55:
            INST_NAME("ANDNPD Gx, Ex");
            nextop = F8;
            GETEX(x1, 0);
            GETGX();
            SSE_LOOP_Q(x3, x4, NOT(x3, x3); AND(x3, x3, x4));
            break;
        case 0x56:
            INST_NAME("ORPD Gx, Ex");
            nextop = F8;
            GETEX(x1, 0);
            GETGX();
            SSE_LOOP_Q(x3, x4, OR(x3, x3, x4));
            break;
        case 0x57:
            INST_NAME("XORPD Gx, Ex");
            nextop = F8;
            GETEX(x1, 0);
            GETGX();
            SSE_LOOP_Q(x3, x4, XOR(x3, x3, x4));
            break;
        case 0x58:
            INST_NAME("ADDPD Gx, Ex");
            nextop = F8;
            GETEX(x1, 0);
            GETGX();
            SSE_LOOP_FQ(x3, x4, {
                if (!box64_dynarec_fastnan) {
                    FEQD(x3, v0, v0);
                    FEQD(x4, v1, v1);
                }
                FADDD(v0, v0, v1);
                if (!box64_dynarec_fastnan) {
                    AND(x3, x3, x4);
                    BEQZ(x3, 16);
                    FEQD(x3, v0, v0);
                    BNEZ(x3, 8);
                    FNEGD(v0, v0);
                }
            });
            break;
        case 0x59:
            INST_NAME("MULPD Gx, Ex");
            nextop = F8;
            GETEX(x1, 0);
            GETGX();
            SSE_LOOP_FQ(x3, x4, {
                if (!box64_dynarec_fastnan) {
                    FEQD(x3, v0, v0);
                    FEQD(x4, v1, v1);
                }
                FMULD(v0, v0, v1);
                if (!box64_dynarec_fastnan) {
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
            GETGX();
            GETEX(x2, 0);
            d0 = fpu_get_scratch(dyn);
            // GX->f[0] = EX->d[0];
            FLD(d0, wback, fixedaddress + 0);
            FCVTSD(d0, d0);
            FSD(d0, gback, gdoffset + 0);
            // GX->f[1] = EX->d[1];
            FLD(d0, wback, fixedaddress + 8);
            FCVTSD(d0, d0);
            FSD(d0, gback, gdoffset + 4);
            // GX->q[1] = 0;
            SD(xZR, gback, gdoffset + 8);
            break;
        case 0x5B:
            INST_NAME("CVTPS2DQ Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            d0 = fpu_get_scratch(dyn);
            u8 = sse_setround(dyn, ninst, x6, x4);
            for (int i = 0; i < 4; ++i) {
                FLW(d0, wback, fixedaddress + 4 * i);
                FCVTLS(x3, d0, RD_DYN);
                SEXT_W(x5, x3);
                BEQ(x5, x3, 8);
                LUI(x3, 0x80000); // INT32_MIN
                SW(x3, gback, gdoffset + 4 * i);
            }
            x87_restoreround(dyn, ninst, u8);
            break;
        case 0x5C:
            INST_NAME("SUBPD Gx, Ex");
            nextop = F8;
            GETEX(x1, 0);
            GETGX();
            SSE_LOOP_FQ(x3, x4, {
                if (!box64_dynarec_fastnan) {
                    FEQD(x3, v0, v0);
                    FEQD(x4, v1, v1);
                }
                FSUBD(v0, v0, v1);
                if (!box64_dynarec_fastnan) {
                    AND(x3, x3, x4);
                    BEQZ(x3, 16);
                    FEQD(x3, v0, v0);
                    BNEZ(x3, 8);
                    FNEGD(v0, v0);
                }
            });
            break;
        case 0x5D:
            INST_NAME("MINPD Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            for (int i = 0; i < 2; ++i) {
                FLD(d0, gback, gdoffset + 8 * i);
                FLD(d1, wback, fixedaddress + 8 * i);
                FEQD(x3, d0, d0);
                FEQD(x4, d1, d1);
                AND(x3, x3, x4);
                BEQ(x3, xZR, 12);
                FLTD(x3, d1, d0);
                BEQ(x3, xZR, 8); // continue
                FSD(d1, gback, gdoffset + 8 * i);
            }
            break;
        case 0x5E:
            INST_NAME("DIVPD Gx, Ex");
            nextop = F8;
            GETEX(x1, 0);
            GETGX();
            SSE_LOOP_FQ(x3, x4, {
                if (!box64_dynarec_fastnan) {
                    FEQD(x3, v0, v0);
                    FEQD(x4, v1, v1);
                }
                FDIVD(v0, v0, v1);
                if (!box64_dynarec_fastnan) {
                    AND(x3, x3, x4);
                    BEQZ(x3, 16);
                    FEQD(x3, v0, v0);
                    BNEZ(x3, 8);
                    FNEGD(v0, v0);
                }
            });
            break;
        case 0x5F:
            INST_NAME("MAXPD Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            for (int i = 0; i < 2; ++i) {
                FLD(d0, gback, gdoffset + 8 * i);
                FLD(d1, wback, fixedaddress + 8 * i);
                FEQD(x3, d0, d0);
                FEQD(x4, d1, d1);
                AND(x3, x3, x4);
                BEQ(x3, xZR, 12);
                FLTD(x3, d0, d1);
                BEQ(x3, xZR, 8); // continue
                FSD(d1, gback, gdoffset + 8 * i);
            }
            break;
        case 0x60:
            INST_NAME("PUNPCKLBW Gx,Ex");
            nextop = F8;
            GETGX();
            for (int i = 7; i > 0; --i) { // 0 is untouched
                // GX->ub[2 * i] = GX->ub[i];
                LBU(x3, gback, gdoffset + i);
                SB(x3, gback, gdoffset + 2 * i);
            }
            if (MODREG && gd == (nextop & 7) + (rex.b << 3)) {
                for (int i = 0; i < 8; ++i) {
                    // GX->ub[2 * i + 1] = GX->ub[2 * i];
                    LBU(x3, gback, gdoffset + 2 * i);
                    SB(x3, gback, gdoffset + 2 * i + 1);
                }
            } else {
                GETEX(x1, 0);
                for (int i = 0; i < 8; ++i) {
                    // GX->ub[2 * i + 1] = EX->ub[i];
                    LBU(x3, wback, fixedaddress + i);
                    SB(x3, gback, gdoffset + 2 * i + 1);
                }
            }
            break;
        case 0x61:
            INST_NAME("PUNPCKLWD Gx,Ex");
            nextop = F8;
            GETGX();
            for (int i = 3; i > 0; --i) {
                // GX->uw[2 * i] = GX->uw[i];
                LHU(x3, gback, gdoffset + i * 2);
                SH(x3, gback, gdoffset + 2 * i * 2);
            }
            if (MODREG && gd == (nextop & 7) + (rex.b << 3)) {
                for (int i = 0; i < 4; ++i) {
                    // GX->uw[2 * i + 1] = GX->uw[2 * i];
                    LHU(x3, gback, gdoffset + 2 * i * 2);
                    SH(x3, gback, gdoffset + (2 * i + 1) * 2);
                }
            } else {
                GETEX(x1, 0);
                for (int i = 0; i < 4; ++i) {
                    // GX->uw[2 * i + 1] = EX->uw[i];
                    LHU(x3, wback, fixedaddress + i * 2);
                    SH(x3, gback, gdoffset + (2 * i + 1) * 2);
                }
            }
            break;
        case 0x62:
            INST_NAME("PUNPCKLDQ Gx,Ex");
            nextop = F8;
            GETEX(x1, 0);
            GETGX();
            // GX->ud[3] = EX->ud[1];
            LWU(x3, wback, fixedaddress + 1 * 4);
            SW(x3, gback, gdoffset + 3 * 4);
            // GX->ud[2] = GX->ud[1];
            LWU(x3, gback, gdoffset + 1 * 4);
            SW(x3, gback, gdoffset + 2 * 4);
            // GX->ud[1] = EX->ud[0];
            LWU(x3, wback, fixedaddress + 0 * 4);
            SW(x3, gback, gdoffset + 1 * 4);
            break;
        case 0x63:
            INST_NAME("PACKSSWB Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            MOV64x(x5, 127);
            MOV64x(x6, -128);
            for (int i = 0; i < 8; ++i) {
                LH(x3, gback, gdoffset + i * 2);
                if (rv64_zbb) {
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
                LD(x3, gback, gdoffset + 0);
                SD(x3, gback, gdoffset + 8);
            } else
                for (int i = 0; i < 8; ++i) {
                    LH(x3, wback, fixedaddress + i * 2);
                    if (rv64_zbb) {
                        MIN(x3, x3, x5);
                        MAX(x3, x3, x6);
                    } else {
                        BLT(x3, x5, 4 + 4);
                        MV(x3, x5);
                        BGE(x3, x6, 4 + 4);
                        MV(x3, x6);
                    }
                    SB(x3, gback, gdoffset + 8 + i);
                }
            break;
        case 0x64:
            INST_NAME("PCMPGTB Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            for (int i = 0; i < 16; ++i) {
                // GX->ub[i] = (GX->sb[i]>EX->sb[i])?0xFF:0x00;
                LB(x3, wback, fixedaddress + i);
                LB(x4, gback, gdoffset + i);
                SLT(x3, x3, x4);
                NEG(x3, x3);
                SB(x3, gback, gdoffset + i);
            }
            break;
        case 0x65:
            INST_NAME("PCMPGTW Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            for (int i = 0; i < 8; ++i) {
                // GX->uw[i] = (GX->sw[i]>EX->sw[i])?0xFFFF:0x0000;
                LH(x3, wback, fixedaddress + i * 2);
                LH(x4, gback, gdoffset + i * 2);
                SLT(x3, x3, x4);
                NEG(x3, x3);
                SH(x3, gback, gdoffset + i * 2);
            }
            break;
        case 0x66:
            INST_NAME("PCMPGTD Gx,Ex");
            nextop = F8;
            GETEX(x1, 0);
            GETGX();
            SSE_LOOP_DS(x3, x4, SLT(x4, x4, x3); SLLI(x3, x4, 63); SRAI(x3, x3, 63));
            break;
        case 0x67:
            INST_NAME("PACKUSWB Gx, Ex");
            nextop = F8;
            GETGX();
            ADDI(x5, xZR, 0xFF);
            for (int i = 0; i < 8; ++i) {
                // GX->ub[i] = (GX->sw[i]<0)?0:((GX->sw[i]>0xff)?0xff:GX->sw[i]);
                LH(x3, gback, gdoffset + i * 2);
                BGE(x5, x3, 8);
                ADDI(x3, xZR, 0xFF);
                NOT(x4, x3);
                SRAI(x4, x4, 63);
                AND(x3, x3, x4);
                SB(x3, gback, gdoffset + i);
            }
            if (MODREG && gd == (nextop & 7) + (rex.b << 3)) {
                // GX->q[1] = GX->q[0];
                LD(x3, gback, gdoffset + 0 * 8);
                SD(x3, gback, gdoffset + 1 * 8);
            } else {
                GETEX(x1, 0);
                for (int i = 0; i < 8; ++i) {
                    // GX->ub[8+i] = (EX->sw[i]<0)?0:((EX->sw[i]>0xff)?0xff:EX->sw[i]);
                    LH(x3, wback, fixedaddress + i * 2);
                    BGE(x5, x3, 8);
                    ADDI(x3, xZR, 0xFF);
                    NOT(x4, x3);
                    SRAI(x4, x4, 63);
                    AND(x3, x3, x4);
                    SB(x3, gback, gdoffset + 8 + i);
                }
            }
            break;
        case 0x68:
            INST_NAME("PUNPCKHBW Gx,Ex");
            nextop = F8;
            GETGX();
            for (int i = 0; i < 8; ++i) {
                // GX->ub[2 * i] = GX->ub[i + 8];
                LBU(x3, gback, gdoffset + i + 8);
                SB(x3, gback, gdoffset + 2 * i);
            }
            if (MODREG && gd == (nextop & 7) + (rex.b << 3)) {
                for (int i = 0; i < 8; ++i) {
                    // GX->ub[2 * i + 1] = GX->ub[2 * i];
                    LBU(x3, gback, gdoffset + 2 * i);
                    SB(x3, gback, gdoffset + 2 * i + 1);
                }
            } else {
                GETEX(x2, 0);
                for (int i = 0; i < 8; ++i) {
                    // GX->ub[2 * i + 1] = EX->ub[i + 8];
                    LBU(x3, wback, fixedaddress + i + 8);
                    SB(x3, gback, gdoffset + 2 * i + 1);
                }
            }
            break;
        case 0x69:
            INST_NAME("PUNPCKHWD Gx,Ex");
            nextop = F8;
            GETGX();
            for (int i = 0; i < 4; ++i) {
                // GX->uw[2 * i] = GX->uw[i + 4];
                LHU(x3, gback, gdoffset + (i + 4) * 2);
                SH(x3, gback, gdoffset + 2 * i * 2);
            }
            if (MODREG && gd == (nextop & 7) + (rex.b << 3)) {
                for (int i = 0; i < 4; ++i) {
                    // GX->uw[2 * i + 1] = GX->uw[2 * i];
                    LHU(x3, gback, gdoffset + 2 * i * 2);
                    SH(x3, gback, gdoffset + (2 * i + 1) * 2);
                }
            } else {
                GETEX(x1, 0);
                for (int i = 0; i < 4; ++i) {
                    // GX->uw[2 * i + 1] = EX->uw[i + 4];
                    LHU(x3, wback, fixedaddress + (i + 4) * 2);
                    SH(x3, gback, gdoffset + (2 * i + 1) * 2);
                }
            }
            break;
        case 0x6A:
            INST_NAME("PUNPCKHDQ Gx,Ex");
            nextop = F8;
            GETEX(x1, 0);
            GETGX();
            // GX->ud[0] = GX->ud[2];
            LWU(x3, gback, gdoffset + 2 * 4);
            SW(x3, gback, gdoffset + 0 * 4);
            // GX->ud[1] = EX->ud[2];
            LWU(x3, wback, fixedaddress + 2 * 4);
            SW(x3, gback, gdoffset + 1 * 4);
            // GX->ud[2] = GX->ud[3];
            LWU(x3, gback, gdoffset + 3 * 4);
            SW(x3, gback, gdoffset + 2 * 4);
            // GX->ud[3] = EX->ud[3];
            if (!(MODREG && (gd == ed))) {
                LWU(x3, wback, fixedaddress + 3 * 4);
                SW(x3, gback, gdoffset + 3 * 4);
            }
            break;
        case 0x6B:
            INST_NAME("PACKSSDW Gx,Ex");
            nextop = F8;
            GETGX();
            MOV64x(x5, 32768);
            NEG(x6, x5);
            for (int i = 0; i < 4; ++i) {
                // GX->sw[i] = (GX->sd[i]<-32768)?-32768:((GX->sd[i]>32767)?32767:GX->sd[i]);
                LW(x3, gback, gdoffset + i * 4);
                BGE(x5, x3, 8);
                ADDI(x3, x5, -1);
                BGE(x3, x6, 8);
                MV(x3, x6);
                SH(x3, gback, gdoffset + i * 2);
            }
            if (MODREG && gd == (nextop & 7) + (rex.b << 3)) {
                // GX->q[1] = GX->q[0];
                LD(x3, gback, gdoffset + 0 * 8);
                SD(x3, gback, gdoffset + 1 * 8);
            } else {
                GETEX(x1, 0);
                for (int i = 0; i < 4; ++i) {
                    // GX->sw[4+i] = (EX->sd[i]<-32768)?-32768:((EX->sd[i]>32767)?32767:EX->sd[i]);
                    LW(x3, wback, fixedaddress + i * 4);
                    BGE(x5, x3, 8);
                    ADDI(x3, x5, -1);
                    BGE(x3, x6, 8);
                    MV(x3, x6);
                    SH(x3, gback, gdoffset + (4 + i) * 2);
                }
            }
            break;
        case 0x6C:
            INST_NAME("PUNPCKLQDQ Gx,Ex");
            nextop = F8;
            GETGX();
            if (MODREG) {
                v1 = sse_get_reg(dyn, ninst, x2, (nextop & 7) + (rex.b << 3), 0);
                FSD(v1, gback, gdoffset + 8);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                LD(x3, ed, fixedaddress + 0);
                SD(x3, gback, gdoffset + 8);
            }
            break;
        case 0x6D:
            INST_NAME("PUNPCKHQDQ Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            LD(x3, gback, gdoffset + 8);
            SD(x3, gback, gdoffset + 0);
            LD(x3, wback, fixedaddress + 8);
            SD(x3, gback, gdoffset + 8);
            break;
        case 0x6E:
            INST_NAME("MOVD Gx, Ed");
            nextop = F8;
            if (rex.w) {
                GETGXSD_empty(v0);
            } else {
                GETGXSS_empty(v0);
            }
            GETED(0);
            if (rex.w) {
                FMVDX(v0, ed);
            } else {
                FMVWX(v0, ed);
                SW(xZR, xEmu, offsetof(x64emu_t, xmm[gd]) + 4);
            }
            SD(xZR, xEmu, offsetof(x64emu_t, xmm[gd]) + 8);
            break;
        case 0x6F:
            INST_NAME("MOVDQA Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            SSE_LOOP_MV_Q(x3);
            break;
        case 0x70: // TODO: Optimize this!
            INST_NAME("PSHUFD Gx,Ex,Ib");
            nextop = F8;
            GETGX();
            GETEX(x2, 1);
            u8 = F8;
            int32_t idx;

            idx = (u8 >> (0 * 2)) & 3;
            LWU(x3, wback, fixedaddress + idx * 4);
            idx = (u8 >> (1 * 2)) & 3;
            LWU(x4, wback, fixedaddress + idx * 4);
            idx = (u8 >> (2 * 2)) & 3;
            LWU(x5, wback, fixedaddress + idx * 4);
            idx = (u8 >> (3 * 2)) & 3;
            LWU(x6, wback, fixedaddress + idx * 4);

            SW(x3, gback, gdoffset + 0 * 4);
            SW(x4, gback, gdoffset + 1 * 4);
            SW(x5, gback, gdoffset + 2 * 4);
            SW(x6, gback, gdoffset + 3 * 4);
            break;
        case 0x71:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 2:
                    INST_NAME("PSRLW Ex, Ib");
                    GETEX(x1, 1);
                    u8 = F8;
                    if (u8 > 15) {
                        // just zero dest
                        SD(xZR, wback, fixedaddress + 0);
                        SD(xZR, wback, fixedaddress + 8);
                    } else if (u8) {
                        for (int i = 0; i < 8; ++i) {
                            // EX->uw[i] >>= u8;
                            LHU(x3, wback, fixedaddress + i * 2);
                            SRLI(x3, x3, u8);
                            SH(x3, wback, fixedaddress + i * 2);
                        }
                    }
                    break;
                case 4:
                    INST_NAME("PSRAW Ex, Ib");
                    GETEX(x1, 1);
                    u8 = F8;
                    if (u8 > 15) u8 = 15;
                    if (u8) {
                        for (int i = 0; i < 8; ++i) {
                            // EX->sw[i] >>= u8;
                            LH(x3, wback, fixedaddress + i * 2);
                            SRAI(x3, x3, u8);
                            SH(x3, wback, fixedaddress + i * 2);
                        }
                    }
                    break;
                case 6:
                    INST_NAME("PSLLW Ex, Ib");
                    GETEX(x1, 1);
                    u8 = F8;
                    if (u8 > 15) {
                        // just zero dest
                        SD(xZR, wback, fixedaddress + 0);
                        SD(xZR, wback, fixedaddress + 8);
                    } else if (u8) {
                        for (int i = 0; i < 8; ++i) {
                            // EX->uw[i] <<= u8;
                            LHU(x3, wback, fixedaddress + i * 2);
                            SLLI(x3, x3, u8);
                            SH(x3, wback, fixedaddress + i * 2);
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
            switch ((nextop >> 3) & 7) {
                case 2:
                    INST_NAME("PSRLD Ex, Ib");
                    GETEX(x1, 1);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 31) {
                            // just zero dest
                            SD(xZR, wback, fixedaddress + 0);
                            SD(xZR, wback, fixedaddress + 8);
                        } else if (u8) {
                            SSE_LOOP_D_S(x3, SRLI(x3, x3, u8));
                        }
                    }
                    break;
                case 4:
                    INST_NAME("PSRAD Ex, Ib");
                    GETEX(x1, 1);
                    u8 = F8;
                    if (u8 > 31) u8 = 31;
                    if (u8) {
                        SSE_LOOP_D_S(x3, SRAIW(x3, x3, u8));
                    }
                    break;
                case 6:
                    INST_NAME("PSLLD Ex, Ib");
                    GETEX(x1, 1);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 31) {
                            // just zero dest
                            SD(xZR, wback, fixedaddress + 0);
                            SD(xZR, wback, fixedaddress + 8);
                        } else if (u8) {
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
            switch ((nextop >> 3) & 7) {
                case 2:
                    INST_NAME("PSRLQ Ex, Ib");
                    GETEX(x1, 1);
                    u8 = F8;
                    if (!u8) break;
                    if (u8 > 63) {
                        // just zero dest
                        SD(xZR, wback, fixedaddress + 0);
                        SD(xZR, wback, fixedaddress + 8);
                    } else {
                        LD(x3, wback, fixedaddress + 0);
                        LD(x4, wback, fixedaddress + 8);
                        SRLI(x3, x3, u8);
                        SRLI(x4, x4, u8);
                        SD(x3, wback, fixedaddress + 0);
                        SD(x4, wback, fixedaddress + 8);
                    }
                    break;
                case 3:
                    INST_NAME("PSRLDQ Ex, Ib");
                    GETEX(x1, 1);
                    u8 = F8;
                    if (!u8) break;
                    if (u8 > 15) {
                        // just zero dest
                        SD(xZR, wback, fixedaddress + 0);
                        SD(xZR, wback, fixedaddress + 8);
                    } else {
                        u8 *= 8;
                        if (u8 < 64) {
                            LD(x3, wback, fixedaddress + 0);
                            LD(x4, wback, fixedaddress + 8);
                            SRLI(x3, x3, u8);
                            SLLI(x5, x4, 64 - u8);
                            OR(x3, x3, x5);
                            SD(x3, wback, fixedaddress + 0);
                            SRLI(x4, x4, u8);
                            SD(x4, wback, fixedaddress + 8);
                        } else {
                            LD(x3, wback, fixedaddress + 8);
                            if (u8 - 64 > 0) { SRLI(x3, x3, u8 - 64); }
                            SD(x3, wback, fixedaddress + 0);
                            SD(xZR, wback, fixedaddress + 8);
                        }
                    }
                    break;
                case 6:
                    INST_NAME("PSLLQ Ex, Ib");
                    GETEX(x1, 1);
                    u8 = F8;
                    if (!u8) break;
                    if (u8 > 63) {
                        // just zero dest
                        SD(xZR, wback, fixedaddress + 0);
                        SD(xZR, wback, fixedaddress + 8);
                    } else {
                        LD(x3, wback, fixedaddress + 0);
                        LD(x4, wback, fixedaddress + 8);
                        SLLI(x3, x3, u8);
                        SLLI(x4, x4, u8);
                        SD(x3, wback, fixedaddress + 0);
                        SD(x4, wback, fixedaddress + 8);
                    }
                    break;
                case 7:
                    INST_NAME("PSLLDQ Ex, Ib");
                    GETEX(x1, 1);
                    u8 = F8;
                    if (!u8) break;
                    if (u8 > 15) {
                        // just zero dest
                        SD(xZR, wback, fixedaddress + 0);
                        SD(xZR, wback, fixedaddress + 8);
                    } else {
                        u8 *= 8;
                        if (u8 < 64) {
                            LD(x3, wback, fixedaddress + 0);
                            LD(x4, wback, fixedaddress + 8);
                            SLLI(x4, x4, u8);
                            SRLI(x5, x3, 64 - u8);
                            OR(x4, x4, x5);
                            SD(x4, wback, fixedaddress + 8);
                            SLLI(x3, x3, u8);
                            SD(x3, wback, fixedaddress + 0);
                        } else {
                            LD(x3, wback, fixedaddress + 0);
                            if (u8 - 64 > 0) { SLLI(x3, x3, u8 - 64); }
                            SD(x3, wback, fixedaddress + 8);
                            SD(xZR, wback, fixedaddress + 0);
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
            GETGX();
            GETEX(x2, 0);
            for (int i = 0; i < 16; ++i) {
                LBU(x3, gback, gdoffset + i);
                LBU(x4, wback, fixedaddress + i);
                SUB(x3, x3, x4);
                SEQZ(x3, x3);
                NEG(x3, x3);
                SB(x3, gback, gdoffset + i);
            }
            break;
        case 0x75:
            INST_NAME("PCMPEQW Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            SSE_LOOP_W(x3, x4, SUB(x3, x3, x4); SEQZ(x3, x3); NEG(x3, x3));
            break;
        case 0x76:
            INST_NAME("PCMPEQD Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            SSE_LOOP_D(x3, x4, XOR(x3, x3, x4); SNEZ(x3, x3); ADDI(x3, x3, -1));
            break;
        case 0x7C:
            INST_NAME("HADDPD Gx, Ex");
            nextop = F8;
            GETGX();
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            FLD(d0, gback, gdoffset + 0);
            FLD(d1, gback, gdoffset + 8);
            if (!box64_dynarec_fastnan) {
                FEQD(x3, d0, d0);
                FEQD(x4, d1, d1);
                AND(x3, x3, x4);
            }
            FADDD(d0, d0, d1);
            if (!box64_dynarec_fastnan) {
                FEQD(x4, d0, d0);
                BEQZ(x3, 12);
                BNEZ(x4, 8);
                FNEGD(d0, d0);
            }
            FSD(d0, gback, gdoffset + 0);
            if (MODREG && gd == (nextop & 7) + (rex.b << 3)) {
                FSD(d0, gback, gdoffset + 8);
            } else {
                GETEX(x2, 0);
                FLD(d0, wback, fixedaddress + 0);
                FLD(d1, wback, fixedaddress + 8);
                if (!box64_dynarec_fastnan) {
                    FEQD(x3, d0, d0);
                    FEQD(x4, d1, d1);
                    AND(x3, x3, x4);
                }
                FADDD(d0, d0, d1);
                if (!box64_dynarec_fastnan) {
                    FEQD(x4, d0, d0);
                    BEQZ(x3, 12);
                    BNEZ(x4, 8);
                    FNEGD(d0, d0);
                }
                FSD(d0, gback, gdoffset + 8);
            }
            break;
        case 0x7E:
            INST_NAME("MOVD Ed,Gx");
            nextop = F8;
            GETGX();
            if (rex.w) {
                if (MODREG) {
                    ed = xRAX + (nextop & 7) + (rex.b << 3);
                    LD(ed, gback, gdoffset + 0);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                    LD(x3, gback, gdoffset + 0);
                    SD(x3, ed, fixedaddress);
                    SMWRITE2();
                }
            } else {
                if (MODREG) {
                    ed = xRAX + (nextop & 7) + (rex.b << 3);
                    LWU(ed, gback, gdoffset + 0);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                    LWU(x3, gback, gdoffset + 0);
                    SW(x3, ed, fixedaddress);
                    SMWRITE2();
                }
            }
            break;
        case 0x7F:
            INST_NAME("MOVDQA Ex,Gx");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            SSE_LOOP_MV_Q2(x3);
            if (!MODREG) SMWRITE2();
            break;
        case 0xA3:
            INST_NAME("BT Ew, Gw");
            SETFLAGS(X_CF, SF_SUBSET);
            SET_DFNONE();
            nextop = F8;
            GETEW(x1, 0);
            GETGW(x2);
            ANDI(gd, gd, 15);
            BEXT(x4, ed, gd, x6);
            ANDI(xFlags, xFlags, ~1);
            OR(xFlags, xFlags, x4);
            break;
        case 0xA4:
            nextop = F8;
            INST_NAME("SHLD Ew, Gw, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            GETEW(x1, 0);
            GETGW(x2);
            u8 = F8;
            emit_shld16c(dyn, ninst, rex, ed, gd, u8, x3, x4, x5);
            EWBACK;
            break;
        case 0xAB:
            INST_NAME("BTS Ew, Gw");
            SETFLAGS(X_CF, SF_SUBSET);
            SET_DFNONE();
            nextop = F8;
            GETEW(x1, 0);
            GETGW(x2);
            ANDI(gd, gd, 15);
            BEXT(x4, ed, gd, x6);
            ANDI(xFlags, xFlags, ~1);
            OR(xFlags, xFlags, x4);
            ADDI(x4, xZR, 1);
            SLL(x4, x4, gd);
            OR(ed, ed, x4);
            EWBACK;
            break;
        case 0xAC:
            nextop = F8;
            INST_NAME("SHRD Ew, Gw, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            GETEW(x1, 0);
            GETGW(x2);
            u8 = F8;
            emit_shrd16c(dyn, ninst, rex, ed, gd, u8, x3, x4);
            EWBACK;
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
            ZEXTH(x2, x2);
            GWBACK;
            break;
        case 0xB3:
            INST_NAME("BTR Ew, Gw");
            SETFLAGS(X_CF, SF_SUBSET);
            SET_DFNONE();
            nextop = F8;
            GETEW(x1, 0);
            GETGW(x2);
            ANDI(gd, gd, 15);
            BEXT(x4, ed, gd, x3); // F_CF is 1
            ANDI(xFlags, xFlags, ~1);
            OR(xFlags, xFlags, x4);
            ADDI(x4, xZR, 1);
            SLL(x4, x4, gd);
            NOT(x4, x4);
            AND(ed, ed, x4);
            EWBACK;
            break;
        case 0xB6:
            INST_NAME("MOVZX Gw, Eb");
            nextop = F8;
            gd = xRAX+((nextop&0x38)>>3)+(rex.r<<3);
            if (MODREG) {
                if (rex.rex) {
                    eb1 = xRAX + (nextop & 7) + (rex.b << 3);
                    eb2 = 0;
                } else {
                    ed = (nextop & 7);
                    eb1 = xRAX + (ed & 3); // Ax, Cx, Dx or Bx
                    eb2 = (ed & 4) >> 2;   // L or H
                }
                if (eb2) {
                    SRLI(x1, eb1, 8);
                    eb1 = x1;
                }
                ANDI(x1, eb1, 0xff);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x4, &fixedaddress, rex, NULL, 1, 0);
                LBU(x1, ed, fixedaddress);
            }
            LUI(x5, 0xffff0);
            AND(gd, gd, x5);
            OR(gd, gd, x1);
            break;
        case 0xBA:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 4:
                    INST_NAME("BT Ew, Ib");
                    SETFLAGS(X_CF, SF_SUBSET);
                    SET_DFNONE();
                    GETED(1);
                    u8 = F8;
                    u8 &= rex.w ? 0x3f : 15;
                    BEXTI(x3, ed, u8); // F_CF is 1
                    ANDI(xFlags, xFlags, ~1);
                    OR(xFlags, xFlags, x3);
                    break;
                case 5:
                    INST_NAME("BTS Ew, Ib");
                    SETFLAGS(X_CF, SF_SUBSET);
                    SET_DFNONE();
                    GETEW(x1, 1);
                    u8 = F8;
                    u8 &= (rex.w ? 0x3f : 15);
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
                    EWBACK;
                    MARK;
                    break;
                case 6:
                    INST_NAME("BTR Ed, Ib");
                    SETFLAGS(X_CF, SF_SUBSET);
                    SET_DFNONE();
                    GETEW(x1, 1);
                    u8 = F8;
                    u8 &= (rex.w ? 0x3f : 15);
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
                    EWBACK;
                    MARK;
                    break;
                case 7:
                    INST_NAME("BTC Ew, Ib");
                    SETFLAGS(X_CF, SF_SUBSET);
                    SET_DFNONE();
                    GETEW(x1, 1);
                    u8 = F8;
                    u8 &= rex.w ? 0x3f : 15;
                    BEXTI(x3, ed, u8); // F_CF is 1
                    ANDI(xFlags, xFlags, ~1);
                    OR(xFlags, xFlags, x3);
                    if (u8 <= 0x10) {
                        XORI(ed, ed, (1LL << u8));
                    } else {
                        MOV64xw(x3, (1LL << u8));
                        XOR(ed, ed, x3);
                    }
                    EWBACK;
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xBB:
            INST_NAME("BTC Ew, Gw");
            SETFLAGS(X_CF, SF_SUBSET);
            SET_DFNONE();
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = xRAX + (nextop & 7) + (rex.b << 3);
                wback = 0;
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x1, &fixedaddress, rex, NULL, 1, 0);
                SRAIxw(x1, gd, 5 + rex.w);
                ADDSL(x3, wback, x1, 2 + rex.w, x1);
                LDxw(x1, x3, fixedaddress);
                ed = x1;
                wback = x3;
            }
            if (rv64_zbs)
                BEXT_(x4, ed, gd);
            else {
                ANDI(x2, gd, rex.w ? 0x3f : 0xf);
                SRL(x4, ed, x2);
                ANDI(x4, x4, 1);
            }
            ANDI(xFlags, xFlags, ~1);
            OR(xFlags, xFlags, x4);
            ADDI(x4, xZR, 1);
            ANDI(x2, gd, rex.w ? 0x3f : 15);
            SLL(x4, x4, x2);
            XOR(ed, ed, x4);
            if (wback) {
                SDxw(ed, wback, fixedaddress);
                SMWRITE();
            }
            break;
        case 0xBC:
            INST_NAME("BSF Gw, Ew");
            SETFLAGS(X_ZF, SF_SUBSET);
            SET_DFNONE();
            nextop = F8;
            GETEW(x5, 0);
            GETGW(x4);
            BNE_MARK(ed, xZR);
            ORI(xFlags, xFlags, 1 << F_ZF);
            B_NEXT_nocond;
            MARK;
            // gd is undefined if ed is all zeros, don't worry.
            CTZxw(gd, ed, 0, x1, x2);
            ANDI(xFlags, xFlags, ~(1 << F_ZF));
            GWBACK;
            break;
        case 0xBD:
            INST_NAME("BSR Gw, Ew");
            SETFLAGS(X_ZF, SF_SUBSET);
            SET_DFNONE();
            nextop = F8;
            GETEW(x5, 0);
            GETGW(x4);
            BNE_MARK(ed, xZR);
            ORI(xFlags, xFlags, 1 << F_ZF);
            B_NEXT_nocond;
            MARK;
            ANDI(xFlags, xFlags, ~(1 << F_ZF));
            CLZxw(gd, ed, 0, x1, x2, x3);
            ADDI(x1, xZR, rex.w ? 63 : 31);
            SUB(gd, x1, gd);
            GWBACK;
            break;
        case 0xBE:
            INST_NAME("MOVSX Gw, Eb");
            nextop = F8;
            GETGD;
            if (MODREG) {
                if (rex.rex) {
                    ed = xRAX + (nextop & 7) + (rex.b << 3);
                    eb1 = ed;
                    eb2 = 0;
                } else {
                    ed = (nextop & 7);
                    eb1 = xRAX + (ed & 3); // Ax, Cx, Dx or Bx
                    eb2 = (ed & 4) >> 2;   // L or H
                }
                SLLI(x1, eb1, 56 - eb2 * 8);
                SRAI(x1, x1, 56);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x4, &fixedaddress, rex, NULL, 1, 0);
                LB(x1, ed, fixedaddress);
            }
            LUI(x5, 0xffff0);
            AND(gd, gd, x5);
            NOT(x5, x5);
            AND(x1, x1, x5);
            OR(gd, gd, x1);
            break;
        case 0xC1:
            INST_NAME("XADD Ew, Gw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x1);
            GETEW(x2, 0);
            MV(x9, ed);
            emit_add16(dyn, ninst, ed, gd, x4, x5, x6);
            MV(gd, x9);
            EWBACK;
            GWBACK;
            break;
        case 0xC2:
            INST_NAME("CMPPD Gx, Ex, Ib");
            nextop = F8;
            GETGX();
            GETEX(x2, 1);
            u8 = F8;
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            for (int i = 0; i < 2; ++i) {
                FLD(d0, gback, gdoffset + 8 * i);
                FLD(d1, wback, fixedaddress + 8 * i);
                if ((u8 & 7) == 0) { // Equal
                    FEQD(x3, d0, d1);
                } else if ((u8 & 7) == 4) { // Not Equal or unordered
                    FEQD(x3, d0, d1);
                    XORI(x3, x3, 1);
                } else {
                    // x4 = !(isnan(d0) || isnan(d1))
                    FEQD(x4, d0, d0);
                    FEQD(x3, d1, d1);
                    AND(x3, x3, x4);

                    switch (u8 & 7) {
                        case 1:
                            BEQ_MARK(x3, xZR);
                            FLTD(x3, d0, d1);
                            break; // Less than
                        case 2:
                            BEQ_MARK(x3, xZR);
                            FLED(x3, d0, d1);
                            break;                      // Less or equal
                        case 3: XORI(x3, x3, 1); break; // NaN
                        case 5: {                       // Greater or equal or unordered
                            BEQ(x3, xZR, 12);           // MARK2
                            FLED(x3, d1, d0);
                            J(8); // MARK;
                            break;
                        }
                        case 6: {             // Greater or unordered
                            BEQ(x3, xZR, 12); // MARK2
                            FLTD(x3, d1, d0);
                            J(8); // MARK;
                            break;
                        }
                        case 7: break; // Not NaN
                    }

                    // MARK2;
                    if ((u8 & 7) == 5 || (u8 & 7) == 6) {
                        MOV32w(x3, 1);
                    }
                    // MARK;
                }
                NEG(x3, x3);
                SD(x3, gback, gdoffset + 8 * i);
            }
            break;
        case 0xC4:
            INST_NAME("PINSRW Gx,Ed,Ib");
            nextop = F8;
            GETED(1);
            GETGX();
            u8 = (F8)&7;
            SH(ed, gback, gdoffset + u8 * 2);
            break;
        case 0xC5:
            INST_NAME("PEXTRW Gd,Ex,Ib");
            nextop = F8;
            GETGD;
            GETEX(x1, 0);
            u8 = (F8)&7;
            LHU(gd, wback, fixedaddress + u8 * 2);
            break;
        case 0xC6:
            INST_NAME("SHUFPD Gx, Ex, Ib");
            nextop = F8;
            GETGX();
            GETEX(x2, 1);
            u8 = F8;
            if (MODREG && gd == (nextop & 7) + (rex.b << 3) && u8 == 0) {
                LD(x3, gback, gdoffset + 0);
                SD(x3, gback, gdoffset + 8);
                break;
            }
            LD(x3, gback, gdoffset + 8 * (u8 & 1));
            LD(x4, wback, fixedaddress + 8 * ((u8 >> 1) & 1));
            SD(x3, gback, gdoffset + 0);
            SD(x4, gback, gdoffset + 8);
            break;
        case 0xC8:
        case 0xC9:
        case 0xCA:
        case 0xCB:
        case 0xCC:
        case 0xCD:
        case 0xCE:
        case 0xCF:                  /* BSWAP reg */
            INST_NAME("BSWAP Reg");
            gd = xRAX+(opcode&7)+(rex.b<<3);
            if(rex.w) {
                REV8xw(gd, gd, x1, x2, x3, x4);
            } else {
                ANDI(x1, gd, 0xff);
                SLLI(x1, x1, 8);
                SRLI(x2, gd, 8);
                ANDI(x2, x2, 0xff);
                SRLI(x3, gd, 16);
                SLLI(x4, x3, 16);
                AND(x1, x4, x1);
                AND(gd, x1, x2);
            }
            break;
        case 0xD1:
            INST_NAME("PSRLW Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            LD(x3, wback, fixedaddress);
            ADDI(x4, xZR, 16);
            BLTU_MARK(x3, x4);
            SD(xZR, gback, gdoffset + 0);
            SD(xZR, gback, gdoffset + 8);
            B_NEXT_nocond;
            MARK;
            for (int i = 0; i < 8; ++i) {
                LHU(x5, gback, gdoffset + 2 * i);
                SRLW(x5, x5, x3);
                SH(x5, gback, gdoffset + 2 * i);
            }
            break;
        case 0xD2:
            INST_NAME("PSRLD Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            LD(x3, wback, fixedaddress);
            ADDI(x4, xZR, 32);
            BLTU_MARK(x3, x4);
            SD(xZR, gback, gdoffset + 0);
            SD(xZR, gback, gdoffset + 8);
            B_NEXT_nocond;
            MARK;
            for (int i = 0; i < 4; ++i) {
                LWU(x5, gback, gdoffset + 4 * i);
                SRLW(x5, x5, x3);
                SW(x5, gback, gdoffset + 4 * i);
            }
            break;
        case 0xD3:
            INST_NAME("PSRLQ Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            LD(x3, wback, fixedaddress);
            ADDI(x4, xZR, 64);
            BLTU_MARK(x3, x4);
            SD(xZR, gback, gdoffset + 0);
            SD(xZR, gback, gdoffset + 8);
            B_NEXT_nocond;
            MARK;
            for (int i = 0; i < 2; ++i) {
                LD(x5, gback, gdoffset + 8 * i);
                SRL(x5, x5, x3);
                SD(x5, gback, gdoffset + 8 * i);
            }
            break;
        case 0xD4:
            INST_NAME("PADDQ Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            SSE_LOOP_Q(x3, x4, ADD(x3, x3, x4));
            break;
        case 0xD5:
            INST_NAME("PMULLW Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            for (int i = 0; i < 8; ++i) {
                LH(x3, gback, gdoffset + 2 * i);
                LH(x4, wback, fixedaddress + 2 * i);
                MULW(x3, x3, x4);
                SH(x3, gback, gdoffset + 2 * i);
            }
            break;
        case 0xD6:
            INST_NAME("MOVQ Ex, Gx");
            nextop = F8;
            GETGXSD(d0);
            GETEX(x2, 0);
            FSD(d0, wback, fixedaddress + 0);
            if (MODREG) {
                SD(xZR, wback, fixedaddress + 8);
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
            for (int i = 0; i < 16; ++i) {
                LB(x1, wback, fixedaddress + i);
                SLT(x3, x1, xZR);
                if (i > 0) SLLI(x3, x3, i);
                OR(gd, gd, x3);
            }
            break;
        case 0xD8:
            INST_NAME("PSUBUSB Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            for (int i = 0; i < 16; ++i) {
                LBU(x3, gback, gdoffset + i);
                LBU(x4, wback, fixedaddress + i);
                SUB(x3, x3, x4);
                NOT(x4, x3);
                SRAI(x4, x4, 63);
                AND(x3, x3, x4);
                SB(x3, gback, gdoffset + i);
            }
            break;
        case 0xD9:
            INST_NAME("PSUBUSW Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            SSE_LOOP_W(x3, x4, SUB(x3, x3, x4); NOT(x4, x3); SRAI(x4, x4, 63); AND(x3, x3, x4));
            break;
        case 0xDA:
            INST_NAME("PMINUB Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            for (int i = 0; i < 16; ++i) {
                LBU(x3, gback, gdoffset + i);
                LBU(x4, wback, fixedaddress + i);
                BLTU(x3, x4, 8);
                MV(x3, x4);
                SB(x3, gback, gdoffset + i);
            }
            break;
        case 0xDB:
            INST_NAME("PAND Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            SSE_LOOP_Q(x3, x4, AND(x3, x3, x4));
            break;
        case 0xDC:
            INST_NAME("PADDUSB Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            ADDI(x5, xZR, 0xFF);
            for (int i = 0; i < 16; ++i) {
                LBU(x3, gback, gdoffset + i);
                LBU(x4, wback, fixedaddress + i);
                ADD(x3, x3, x4);
                BLT(x3, x5, 8);
                ADDI(x3, xZR, 0xFF);
                SB(x3, gback, gdoffset + i);
            }
            break;
        case 0xDD:
            INST_NAME("PADDUSW Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            for (int i = 0; i < 8; ++i) {
                // tmp32s = (int32_t)GX->uw[i] + EX->uw[i];
                // GX->uw[i] = (tmp32s>65535)?65535:tmp32s;
                LHU(x3, gback, gdoffset + i * 2);
                LHU(x4, wback, fixedaddress + i * 2);
                ADDW(x3, x3, x4);
                MOV32w(x4, 65536);
                BLT(x3, x4, 8);
                ADDIW(x3, x4, -1);
                SH(x3, gback, gdoffset + i * 2);
            }
            break;
        case 0xDE:
            INST_NAME("PMAXUB Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            for (int i = 0; i < 16; ++i) {
                LBU(x3, gback, gdoffset + i);
                LBU(x4, wback, fixedaddress + i);
                BLTU(x4, x3, 8);
                MV(x3, x4);
                SB(x3, gback, gdoffset + i);
            }
            break;
        case 0xDF:
            INST_NAME("PANDN Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            SSE_LOOP_Q(x3, x4, NOT(x3, x3); AND(x3, x3, x4));
            break;
        case 0xE0:
            INST_NAME("PAVGB Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            for (int i = 0; i < 16; ++i) {
                LBU(x3, gback, gdoffset + i);
                LBU(x4, wback, fixedaddress + i);
                ADDW(x3, x3, x4);
                ADDIW(x3, x3, 1);
                SRAIW(x3, x3, 1);
                SB(x3, gback, gdoffset + i);
            }
            break;
        case 0xE1:
            INST_NAME("PSRAW Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            ADDI(x4, xZR, 16);
            LD(x3, wback, fixedaddress);
            BLTU(x3, x4, 8);
            SUBI(x3, x4, 1);
            for (int i = 0; i < 8; ++i) {
                LH(x4, gback, gdoffset + 2 * i);
                SRAW(x4, x4, x3);
                SH(x4, gback, gdoffset + 2 * i);
            }
            break;
        case 0xE2:
            INST_NAME("PSRAD Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            ADDI(x4, xZR, 32);
            LD(x3, wback, fixedaddress);
            BLTU(x3, x4, 8);
            SUBI(x3, x4, 1);
            for (int i = 0; i < 4; ++i) {
                LW(x4, gback, gdoffset + 4 * i);
                SRAW(x4, x4, x3);
                SW(x4, gback, gdoffset + 4 * i);
            }
            break;
        case 0xE3:
            INST_NAME("PAVGW Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            for (int i = 0; i < 8; ++i) {
                LHU(x3, gback, gdoffset + 2 * i);
                LHU(x4, wback, fixedaddress + 2 * i);
                ADDW(x3, x3, x4);
                ADDIW(x3, x3, 1);
                SRAIW(x3, x3, 1);
                SH(x3, gback, gdoffset + 2 * i);
            }
            break;
        case 0xE4:
            INST_NAME("PMULHUW Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            for (int i = 0; i < 8; ++i) {
                LHU(x3, gback, gdoffset + 2 * i);
                LHU(x4, wback, fixedaddress + 2 * i);
                MULW(x3, x3, x4);
                SRLIW(x3, x3, 16);
                SH(x3, gback, gdoffset + 2 * i);
            }
            break;
        case 0xE5:
            INST_NAME("PMULHW Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            for (int i = 0; i < 8; ++i) {
                LH(x3, gback, gdoffset + 2 * i);
                LH(x4, wback, fixedaddress + 2 * i);
                MULW(x3, x3, x4);
                SRAIW(x3, x3, 16);
                SH(x3, gback, gdoffset + 2 * i);
            }
            break;
        case 0xE6:
            INST_NAME("CVTTPD2DQ Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            v0 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            FLD(v0, wback, fixedaddress + 0);
            FLD(v1, wback, fixedaddress + 8);
            if (!box64_dynarec_fastround) {
                FSFLAGSI(0); // // reset all bits
            }
            FCVTWD(x3, v0, RD_RTZ);
            if (!box64_dynarec_fastround) {
                FRFLAGS(x5); // get back FPSR to check the IOC bit
                ANDI(x5, x5, (1 << FR_NV) | (1 << FR_OF));
                BEQ_MARK(x5, xZR);
                MOV32w(x3, 0x80000000);
                MARK;
                FSFLAGSI(0); // // reset all bits
            }
            FCVTWD(x4, v1, RD_RTZ);
            if (!box64_dynarec_fastround) {
                FRFLAGS(x5); // get back FPSR to check the IOC bit
                ANDI(x5, x5, (1 << FR_NV) | (1 << FR_OF));
                BEQ_MARK2(x5, xZR);
                MOV32w(x4, 0x80000000);
                MARK2;
            }
            SW(x3, gback, gdoffset + 0);
            SW(x4, gback, gdoffset + 4);
            SD(xZR, gback, gdoffset + 8);
            break;
        case 0xE7:
            INST_NAME("MOVNTDQ Ex, Gx");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            SSE_LOOP_MV_Q2(x3);
            break;
        case 0xE8:
            INST_NAME("PSUBSB Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            for (int i = 0; i < 16; ++i) {
                // tmp16s = (int16_t)GX->sb[i] - EX->sb[i];
                // GX->sb[i] = (tmp16s<-128)?-128:((tmp16s>127)?127:tmp16s);
                LB(x3, gback, gdoffset + i);
                LB(x4, wback, fixedaddress + i);
                SUBW(x3, x3, x4);
                SLLIW(x3, x3, 16);
                SRAIW(x3, x3, 16);
                ADDI(x4, xZR, 0x7f);
                BLT(x3, x4, 12); // tmp16s>127?
                SB(x4, gback, gdoffset + i);
                J(24); // continue
                ADDI(x4, xZR, 0xf80);
                BLT(x4, x3, 12); // tmp16s<-128?
                SB(x4, gback, gdoffset + i);
                J(8); // continue
                SB(x3, gback, gdoffset + i);
            }
            break;
        case 0xE9:
            INST_NAME("PSUBSW Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            for (int i = 0; i < 8; ++i) {
                // tmp32s = (int32_t)GX->sw[i] - EX->sw[i];
                // GX->sw[i] = sat16(tmp32s);
                LH(x3, gback, gdoffset + 2 * i);
                LH(x4, wback, fixedaddress + 2 * i);
                SUBW(x3, x3, x4);
                SAT16(x3, x4);
                SH(x3, gback, gdoffset + 2 * i);
            }
            break;
        case 0xEA:
            INST_NAME("PMINSW Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            for (int i = 0; i < 8; ++i) {
                LH(x3, gback, gdoffset + 2 * i);
                LH(x4, wback, fixedaddress + 2 * i);
                BLT(x3, x4, 8);
                MV(x3, x4);
                SH(x3, gback, gdoffset + 2 * i);
            }
            break;
        case 0xEB:
            INST_NAME("POR Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            SSE_LOOP_Q(x3, x4, OR(x3, x3, x4));
            break;
        case 0xEC:
            INST_NAME("PADDSB Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            for (int i = 0; i < 16; ++i) {
                // tmp16s = (int16_t)GX->sb[i] + EX->sb[i];
                // GX->sb[i] = (tmp16s>127)?127:((tmp16s<-128)?-128:tmp16s);
                LB(x3, gback, gdoffset + i);
                LB(x4, wback, fixedaddress + i);
                ADDW(x3, x3, x4);
                SLLIW(x3, x3, 16);
                SRAIW(x3, x3, 16);
                ADDI(x4, xZR, 0x7f);
                BLT(x3, x4, 12); // tmp16s>127?
                SB(x4, gback, gdoffset + i);
                J(24); // continue
                ADDI(x4, xZR, 0xf80);
                BLT(x4, x3, 12); // tmp16s<-128?
                SB(x4, gback, gdoffset + i);
                J(8); // continue
                SB(x3, gback, gdoffset + i);
            }
            break;
        case 0xED:
            INST_NAME("PADDSW Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            for (int i = 0; i < 8; ++i) {
                // tmp32s = (int32_t)GX->sw[i] + EX->sw[i];
                // GX->sw[i] = sat16(tmp32s);
                LH(x3, gback, gdoffset + 2 * i);
                LH(x4, wback, fixedaddress + 2 * i);
                ADDW(x3, x3, x4);
                SAT16(x3, x4);
                SH(x3, gback, gdoffset + 2 * i);
            }
            break;
        case 0xEE:
            INST_NAME("PMAXSW Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            SSE_LOOP_WS(x3, x4, BGE(x3, x4, 8); MV(x3, x4));
            break;
        case 0xEF:
            INST_NAME("PXOR Gx, Ex");
            nextop = F8;
            GETGX();
            if (MODREG && gd == (nextop & 7) + (rex.b << 3)) {
                // just zero dest
                SD(xZR, gback, gdoffset + 0);
                SD(xZR, gback, gdoffset + 8);
            } else {
                GETEX(x2, 0);
                SSE_LOOP_Q(x3, x4, XOR(x3, x3, x4));
            }
            break;
        case 0xF1:
            INST_NAME("PSLLQ Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            ADDI(x4, xZR, 16);
            LD(x3, wback, fixedaddress + 0);
            BLTU_MARK(x3, x4);
            // just zero dest
            SD(xZR, gback, gdoffset + 0);
            SD(xZR, gback, gdoffset + 8);
            B_NEXT_nocond;
            MARK;
            for (int i = 0; i < 8; ++i) {
                LHU(x4, gback, gdoffset + 2 * i);
                SLLW(x4, x4, x3);
                SH(x4, gback, gdoffset + 2 * i);
            }
            break;
        case 0xF2:
            INST_NAME("PSLLQ Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            ADDI(x4, xZR, 32);
            LD(x3, wback, fixedaddress + 0);
            BLTU_MARK(x3, x4);
            // just zero dest
            SD(xZR, gback, gdoffset + 0);
            SD(xZR, gback, gdoffset + 8);
            B_NEXT_nocond;
            MARK;
            for (int i = 0; i < 4; ++i) {
                LWU(x4, gback, gdoffset + 4 * i);
                SLLW(x4, x4, x3);
                SW(x4, gback, gdoffset + 4 * i);
            }
            break;
        case 0xF3:
            INST_NAME("PSLLQ Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            ADDI(x4, xZR, 64);
            LD(x3, wback, fixedaddress + 0);
            BLTU_MARK(x3, x4);
            // just zero dest
            SD(xZR, gback, gdoffset + 0);
            SD(xZR, gback, gdoffset + 8);
            B_NEXT_nocond;
            MARK;
            for (int i = 0; i < 2; ++i) {
                LD(x4, gback, gdoffset + 8 * i);
                SLL(x4, x4, x3);
                SD(x4, gback, gdoffset + 8 * i);
            }
            break;
        case 0xF4:
            INST_NAME("PMULUDQ Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            // GX->q[1] = (uint64_t)EX->ud[2]*GX->ud[2];
            LWU(x3, gback, gdoffset + 2 * 4);
            LWU(x4, wback, fixedaddress + 2 * 4);
            MUL(x3, x3, x4);
            SD(x3, gback, gdoffset + 8);
            // GX->q[0] = (uint64_t)EX->ud[0]*GX->ud[0];
            LWU(x3, gback, gdoffset + 0 * 4);
            LWU(x4, wback, fixedaddress + 0 * 4);
            MUL(x3, x3, x4);
            SD(x3, gback, gdoffset + 0);
            break;
        case 0xF5:
            INST_NAME("PMADDWD Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            for (int i = 0; i < 4; ++i) {
                // GX->sd[i] = (int32_t)(GX->sw[i*2+0])*EX->sw[i*2+0] +
                //             (int32_t)(GX->sw[i*2+1])*EX->sw[i*2+1];
                LH(x3, gback, gdoffset + 2 * (i * 2 + 0));
                LH(x4, wback, fixedaddress + 2 * (i * 2 + 0));
                MULW(x5, x3, x4);
                LH(x3, gback, gdoffset + 2 * (i * 2 + 1));
                LH(x4, wback, fixedaddress + 2 * (i * 2 + 1));
                MULW(x6, x3, x4);
                ADDW(x5, x5, x6);
                SW(x5, gback, gdoffset + 4 * i);
            }
            break;
        case 0xF6:
            INST_NAME("PSADBW Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            MV(x6, xZR);
            for (int i = 0; i < 16; ++i) {
                LBU(x3, gback, gdoffset + i);
                LBU(x4, wback, fixedaddress + i);
                SUBW(x3, x3, x4);
                SRAIW(x5, x3, 31);
                XOR(x3, x5, x3);
                SUBW(x3, x3, x5);
                ANDI(x3, x3, 0xff);
                ADDW(x6, x6, x3);
                if (i == 7 || i == 15) {
                    SD(x6, gback, gdoffset + i + 1 - 8);
                    if (i == 7) MV(x6, xZR);
                }
            }
            break;
        case 0xF8:
            INST_NAME("PSUBB Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            for (int i = 0; i < 16; ++i) {
                // GX->sb[i] -= EX->sb[i];
                LB(x3, wback, fixedaddress + i);
                LB(x4, gback, gdoffset + i);
                SUB(x3, x4, x3);
                SB(x3, gback, gdoffset + i);
            }
            break;
        case 0xF9:
            INST_NAME("PSUBW Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            SSE_LOOP_W(x3, x4, SUBW(x3, x3, x4));
            break;
        case 0xFA:
            INST_NAME("PSUBD Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            SSE_LOOP_D(x3, x4, SUBW(x3, x3, x4));
            break;
        case 0xFB:
            INST_NAME("PSUBQ Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            SSE_LOOP_Q(x3, x4, SUB(x3, x3, x4));
            break;
        case 0xFC:
            INST_NAME("PADDB Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            for (int i = 0; i < 16; ++i) {
                // GX->sb[i] += EX->sb[i];
                LB(x3, gback, gdoffset + i);
                LB(x4, wback, fixedaddress + i);
                ADDW(x3, x3, x4);
                SB(x3, gback, gdoffset + i);
            }
            break;
        case 0xFD:
            INST_NAME("PADDW Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            SSE_LOOP_W(x3, x4, ADDW(x3, x3, x4));
            break;
        case 0xFE:
            INST_NAME("PADDD Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            SSE_LOOP_D(x3, x4, ADDW(x3, x3, x4));
            break;
        default:
            DEFAULT;
    }
    return addr;
}
