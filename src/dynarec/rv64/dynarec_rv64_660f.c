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
    uint8_t tmp1, tmp2, tmp3;
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
            GETEX(x1, 0, 8);
            GETGX();
            SSE_LOOP_MV_Q(x3);
            break;
        case 0x11:
            INST_NAME("MOVUPD Ex,Gx");
            nextop = F8;
            GETEX(x1, 0, 8);
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
            GETEX(x2, 0, 2);
            // GX->q[1] = EX->q[0];
            LD(x3, wback, fixedaddress + 0);
            SD(x3, gback, gdoffset + 8);
            break;
        case 0x15:
            INST_NAME("UNPCKHPD Gx, Ex");
            nextop = F8;
            GETEX(x1, 0, 8);
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
        case 0x17:
            INST_NAME("MOVHPD Eq, Gx");
            nextop = F8;
            GETGX();
            if (MODREG) {
                // access register instead of memory is bad opcode!
                DEFAULT;
                return addr;
            }
            SMREAD();
            addr = geted(dyn, addr, ninst, nextop, &wback, x2, x3, &fixedaddress, rex, NULL, 1, 0);
            LD(x3, gback, gdoffset + 8);
            SD(x3, wback, fixedaddress);
            break;
        case 0x18:
        case 0x19:
        case 0x1F:
            INST_NAME("NOP (multibyte)");
            nextop = F8;
            FAKEED;
            break;
        case 0x28:
            INST_NAME("MOVAPD Gx,Ex");
            nextop = F8;
            GETEX(x1, 0, 8);
            GETGX();
            SSE_LOOP_MV_Q(x3);
            break;
        case 0x29:
            INST_NAME("MOVAPD Ex,Gx");
            nextop = F8;
            GETEX(x1, 0, 8);
            GETGX();
            SSE_LOOP_MV_Q2(x3);
            if (!MODREG) SMWRITE2();
            break;
        case 0x2A:
            INST_NAME("CVTPI2PD Gx,Em");
            nextop = F8;
            GETGX();
            GETEM(x2, 0, 4);
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
            GETEX(x2, 0, 8);
            SSE_LOOP_MV_Q2(x3);
            break;
        case 0x2C:
            INST_NAME("CVTTPD2PI Gm,Ex");
            nextop = F8;
            GETGM();
            GETEX(x2, 0, 8);
            d0 = fpu_get_scratch(dyn);
            for (int i = 0; i < 2; ++i) {
                if (!BOX64ENV(dynarec_fastround)) {
                    FSFLAGSI(0); // // reset all bits
                }
                FLD(d0, wback, fixedaddress + i * 8);
                FCVTWD(x1, d0, RD_RTZ);
                if (!BOX64ENV(dynarec_fastround)) {
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
            GETEX(x2, 0, 8);
            d0 = fpu_get_scratch(dyn);
            u8 = sse_setround(dyn, ninst, x4, x5);
            for (int i = 0; i < 2; ++i) {
                if (!BOX64ENV(dynarec_fastround)) {
                    FSFLAGSI(0); // // reset all bits
                }
                FLD(d0, wback, fixedaddress + i * 8);
                FCVTWD(x1, d0, RD_DYN);
                if (!BOX64ENV(dynarec_fastround)) {
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
            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            nextop = F8;
            GETGXSD(d0);
            GETEXSD(v0, 0);
            CLEAR_FLAGS();
            // if isnan(d0) || isnan(v0)
            IFX (X_ZF | X_PF | X_CF) {
                FEQD(x3, d0, d0);
                FEQD(x2, v0, v0);
                AND(x2, x2, x3);
                BNE_MARK(x2, xZR);
                ORI(xFlags, xFlags, (1 << F_ZF) | (1 << F_PF) | (1 << F_CF));
                B_NEXT_nocond;
            }
            MARK;
            // else if isless(d0, v0)
            IFX (X_CF) {
                FLTD(x2, d0, v0);
                BEQ_MARK2(x2, xZR);
                ORI(xFlags, xFlags, 1 << F_CF);
                B_NEXT_nocond;
            }
            MARK2;
            // else if d0 == v0
            IFX (X_ZF) {
                FEQD(x2, d0, v0);
                CBZ_NEXT(x2);
                ORI(xFlags, xFlags, 1 << F_ZF);
            }
            break;
        case 0x38: // SSSE3 opcodes
        case 0x3A: // these are some more SSSE3+ opcodes
            addr = dynarec64_660F38(dyn, addr, opcode, ip, ninst, rex, ok, need_epilog);
            break;

#define GO(GETFLAGS, NO, YES, NATNO, NATYES, F)                                              \
    READFLAGS(F);                                                                            \
    tmp1 = x1;                                                                               \
    tmp3 = x3;                                                                               \
    GETFLAGS;                                                                                \
    nextop = F8;                                                                             \
    GETGD;                                                                                   \
    if (MODREG) {                                                                            \
        ed = TO_NAT((nextop & 7) + (rex.b << 3));                                            \
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
            GETEX(x1, 0, 8);
            XOR(gd, gd, gd);
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
            GETEX(x2, 0, 8);
            d0 = fpu_get_scratch(dyn);
            if (!BOX64ENV(dynarec_fastnan)) {
                d1 = fpu_get_scratch(dyn);
                FMVDX(d1, xZR);
            }
            for (int i = 0; i < 2; ++i) {
                FLD(d0, wback, fixedaddress + i * 8);
                if (!BOX64ENV(dynarec_fastnan)) {
                    FLTD(x3, d0, d1);
                }
                FSQRTD(d0, d0);
                if (!BOX64ENV(dynarec_fastnan)) {
                    BEQ(x3, xZR, 8);
                    FNEGD(d0, d0);
                }
                FSD(d0, gback, gdoffset + i * 8);
            }
            break;
        case 0x54:
            INST_NAME("ANDPD Gx, Ex");
            nextop = F8;
            GETEX(x1, 0, 8);
            GETGX();
            SSE_LOOP_Q(x3, x4, AND(x3, x3, x4));
            break;
        case 0x55:
            INST_NAME("ANDNPD Gx, Ex");
            nextop = F8;
            GETEX(x1, 0, 8);
            GETGX();
            SSE_LOOP_Q(x3, x4, NOT(x3, x3); AND(x3, x3, x4));
            break;
        case 0x56:
            INST_NAME("ORPD Gx, Ex");
            nextop = F8;
            GETEX(x1, 0, 8);
            GETGX();
            SSE_LOOP_Q(x3, x4, OR(x3, x3, x4));
            break;
        case 0x57:
            INST_NAME("XORPD Gx, Ex");
            nextop = F8;
            GETEX(x1, 0, 8);
            GETGX();
            SSE_LOOP_Q(x3, x4, XOR(x3, x3, x4));
            break;
        case 0x58:
            INST_NAME("ADDPD Gx, Ex");
            nextop = F8;
            GETEX(x1, 0, 8);
            GETGX();
            SSE_LOOP_FQ(x3, x4, {
                if (!BOX64ENV(dynarec_fastnan)) {
                    FEQD(x3, v0, v0);
                    FEQD(x4, v1, v1);
                }
                FADDD(v0, v0, v1);
                if (!BOX64ENV(dynarec_fastnan)) {
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
            GETEX(x1, 0, 8);
            GETGX();
            SSE_LOOP_FQ(x3, x4, {
                if (!BOX64ENV(dynarec_fastnan)) {
                    FEQD(x3, v0, v0);
                    FEQD(x4, v1, v1);
                }
                FMULD(v0, v0, v1);
                if (!BOX64ENV(dynarec_fastnan)) {
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
            GETEX(x2, 0, 8);
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
            GETEX(x2, 0, 12);
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
            GETEX(x1, 0, 8);
            GETGX();
            SSE_LOOP_FQ(x3, x4, {
                if (!BOX64ENV(dynarec_fastnan)) {
                    FEQD(x3, v0, v0);
                    FEQD(x4, v1, v1);
                }
                FSUBD(v0, v0, v1);
                if (!BOX64ENV(dynarec_fastnan)) {
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
            GETEX(x2, 0, 8);
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
            GETEX(x1, 0, 8);
            GETGX();
            SSE_LOOP_FQ(x3, x4, {
                if (!BOX64ENV(dynarec_fastnan)) {
                    FEQD(x3, v0, v0);
                    FEQD(x4, v1, v1);
                }
                FDIVD(v0, v0, v1);
                if (!BOX64ENV(dynarec_fastnan)) {
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
            GETEX(x2, 0, 8);
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
                GETEX(x1, 0, 7);
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
                GETEX(x1, 0, 6);
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
            GETEX(x1, 0, 4);
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
            GETEX(x2, 0, 14);
            MOV64x(x5, 127);
            MOV64x(x6, -128);
            for (int i = 0; i < 8; ++i) {
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
                LD(x3, gback, gdoffset + 0);
                SD(x3, gback, gdoffset + 8);
            } else
                for (int i = 0; i < 8; ++i) {
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
                    SB(x3, gback, gdoffset + 8 + i);
                }
            break;
        case 0x64:
            INST_NAME("PCMPGTB Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 15);
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
            GETEX(x2, 0, 14);
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
            GETEX(x1, 0, 12);
            GETGX();
            SSE_LOOP_DS(x3, x4, SLT(x4, x4, x3); NEG(x3, x4));
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
                GETEX(x1, 0, 14);
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
                GETEX(x2, 0, 7);
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
                GETEX(x1, 0, 14);
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
            GETEX(x1, 0, 12);
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
                GETEX(x1, 0, 12);
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
            GETEX(x2, 0, 8);
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
            GETEX(x2, 0, 8);
            SSE_LOOP_MV_Q(x3);
            break;
        case 0x70: // TODO: Optimize this!
            INST_NAME("PSHUFD Gx,Ex,Ib");
            nextop = F8;
            GETGX();
            GETEX(x2, 1, 12);
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
                    GETEX(x1, 1, 14);
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
                    GETEX(x1, 1, 14);
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
                    GETEX(x1, 1, 14);
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
                    GETEX(x1, 1, 12);
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
                    GETEX(x1, 1, 12);
                    u8 = F8;
                    if (u8 > 31) u8 = 31;
                    if (u8) {
                        SSE_LOOP_D_S(x3, SRAIW(x3, x3, u8));
                    }
                    break;
                case 6:
                    INST_NAME("PSLLD Ex, Ib");
                    GETEX(x1, 1, 12);
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
                    GETEX(x1, 1, 8);
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
                    GETEX(x1, 1, 8);
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
                    GETEX(x1, 1, 8);
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
                    GETEX(x1, 1, 8);
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
            if (cpuext.xtheadbb) {
                GETEX(x2, 0, 8);
                SSE_LOOP_Q(x3, x4, XOR(x3, x3, x4); TH_TSTNBZ(x3, x3););
            } else {
                GETEX(x2, 0, 15);
                for (int i = 0; i < 16; ++i) {
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
            INST_NAME("PCMPEQW Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 14);
            SSE_LOOP_W(x3, x4, SUB(x3, x3, x4); SEQZ(x3, x3); NEG(x3, x3));
            break;
        case 0x76:
            INST_NAME("PCMPEQD Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 12);
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
            if (!BOX64ENV(dynarec_fastnan)) {
                FEQD(x3, d0, d0);
                FEQD(x4, d1, d1);
                AND(x3, x3, x4);
            }
            FADDD(d0, d0, d1);
            if (!BOX64ENV(dynarec_fastnan)) {
                FEQD(x4, d0, d0);
                BEQZ(x3, 12);
                BNEZ(x4, 8);
                FNEGD(d0, d0);
            }
            FSD(d0, gback, gdoffset + 0);
            if (MODREG && gd == (nextop & 7) + (rex.b << 3)) {
                FSD(d0, gback, gdoffset + 8);
            } else {
                GETEX(x2, 0, 8);
                FLD(d0, wback, fixedaddress + 0);
                FLD(d1, wback, fixedaddress + 8);
                if (!BOX64ENV(dynarec_fastnan)) {
                    FEQD(x3, d0, d0);
                    FEQD(x4, d1, d1);
                    AND(x3, x3, x4);
                }
                FADDD(d0, d0, d1);
                if (!BOX64ENV(dynarec_fastnan)) {
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
                    ed = TO_NAT((nextop & 7) + (rex.b << 3));
                    LD(ed, gback, gdoffset + 0);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                    LD(x3, gback, gdoffset + 0);
                    SD(x3, ed, fixedaddress);
                    SMWRITE2();
                }
            } else {
                if (MODREG) {
                    ed = TO_NAT((nextop & 7) + (rex.b << 3));
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
            GETEX(x2, 0, 8);
            SSE_LOOP_MV_Q2(x3);
            if (!MODREG) SMWRITE2();
            break;
        case 0xA3:
            INST_NAME("BT Ew, Gw");
            SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
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
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            GETEW(x1, 0);
            GETGW(x2);
            u8 = F8;
            emit_shld16c(dyn, ninst, rex, ed, gd, u8, x6, x4, x5);
            EWBACK;
            break;
        case 0xAB:
            INST_NAME("BTS Ew, Gw");
            SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
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
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            GETEW(x1, 0);
            GETGW(x2);
            u8 = F8;
            emit_shrd16c(dyn, ninst, rex, ed, gd, u8, x6, x4, x5);
            EWBACK;
            break;
        case 0xAF:
            INST_NAME("IMUL Gw,Ew");
            SETFLAGS(X_ALL, SF_PENDING, NAT_FLAGS_NOFUSION);
            nextop = F8;
            GETSEW(x1, 0);
            GETSGW(x2);
            MULW(x2, x2, x1);
            UFLAG_RES(x2);
            ZEXTH(x2, x2);
            GWBACK;
            UFLAG_DF(x1, d_imul16);
            break;
        case 0xB3:
            INST_NAME("BTR Ew, Gw");
            SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            nextop = F8;
            GETEW(x1, 0);
            GETGW(x2);
            ANDI(gd, gd, 15);
            BEXT(x4, ed, gd, x5); // F_CF is 1
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
            gd = TO_NAT(((nextop & 0x38) >> 3) + (rex.r << 3));
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
                    SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                    SET_DFNONE();
                    GETED(1);
                    u8 = F8;
                    u8 &= rex.w ? 0x3f : 15;
                    IFX (X_CF) {
                        BEXTI(x3, ed, u8); // F_CF is 1
                        ANDI(xFlags, xFlags, ~1);
                        OR(xFlags, xFlags, x3);
                    }
                    break;
                case 5:
                    INST_NAME("BTS Ew, Ib");
                    SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                    SET_DFNONE();
                    GETEW(x1, 1);
                    u8 = F8;
                    u8 &= (rex.w ? 0x3f : 15);
                    IFX (X_CF) ORI(xFlags, xFlags, 1 << F_CF);
                    if (u8 <= 10) {
                        ANDI(x6, ed, 1 << u8);
                        BNE_MARK(x6, xZR);
                        IFX (X_CF) ANDI(xFlags, xFlags, ~(1 << F_CF));
                        XORI(ed, ed, 1 << u8);
                    } else {
                        ORI(x6, xZR, 1);
                        SLLI(x6, x6, u8);
                        AND(x4, ed, x6);
                        BNE_MARK(x4, xZR);
                        IFX (X_CF) ANDI(xFlags, xFlags, ~(1 << F_CF));
                        XOR(ed, ed, x6);
                    }
                    EWBACK;
                    MARK;
                    break;
                case 6:
                    INST_NAME("BTR Ed, Ib");
                    SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                    SET_DFNONE();
                    GETEW(x1, 1);
                    u8 = F8;
                    u8 &= (rex.w ? 0x3f : 15);
                    IFX (X_CF) ANDI(xFlags, xFlags, ~(1 << F_CF));
                    if (u8 <= 10) {
                        ANDI(x6, ed, 1 << u8);
                        BEQ_MARK(x6, xZR);
                        IFX (X_CF) ORI(xFlags, xFlags, 1 << F_CF);
                        XORI(ed, ed, 1 << u8);
                    } else {
                        ORI(x6, xZR, 1);
                        SLLI(x6, x6, u8);
                        AND(x6, ed, x6);
                        BEQ_MARK(x6, xZR);
                        IFX (X_CF) ORI(xFlags, xFlags, 1 << F_CF);
                        XOR(ed, ed, x6);
                    }
                    MARK;
                    EWBACK;
                    break;
                case 7:
                    INST_NAME("BTC Ew, Ib");
                    SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                    SET_DFNONE();
                    GETEW(x1, 1);
                    u8 = F8;
                    u8 &= rex.w ? 0x3f : 15;
                    IFX (X_CF) {
                        BEXTI(x6, ed, u8); // F_CF is 1
                        ANDI(xFlags, xFlags, ~1);
                        OR(xFlags, xFlags, x6);
                    }
                    if (u8 <= 10) {
                        XORI(ed, ed, (1LL << u8));
                    } else {
                        MOV64xw(x6, (1LL << u8));
                        XOR(ed, ed, x6);
                    }
                    EWBACK;
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xBB:
            INST_NAME("BTC Ew, Gw");
            SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
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
            IFX (X_CF) {
                if (cpuext.zbs) {
                    if (rex.w) {
                        BEXT_(x4, ed, gd);
                    } else {
                        ANDI(x2, gd, 0xf);
                        BEXT_(x4, ed, x2);
                    }
                } else {
                    ANDI(x2, gd, rex.w ? 0x3f : 0xf);
                    SRL(x4, ed, x2);
                    ANDI(x4, x4, 1);
                }
                ANDI(xFlags, xFlags, ~1);
                OR(xFlags, xFlags, x4);
            }
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
            SETFLAGS(X_ZF, SF_SUBSET, NAT_FLAGS_NOFUSION);
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
            SETFLAGS(X_ZF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            nextop = F8;
            GETEW(x5, 0);
            GETGW(x4);
            BNE_MARK(ed, xZR);
            ORI(xFlags, xFlags, 1 << F_ZF);
            B_NEXT_nocond;
            MARK;
            ANDI(xFlags, xFlags, ~(1 << F_ZF));
            CLZxw(gd, ed, 1, x1, x2, x6);
            ADDI(x1, xZR, 63);
            SUB(gd, x1, gd);
            GWBACK;
            break;
        case 0xBE:
            INST_NAME("MOVSX Gw, Eb");
            nextop = F8;
            GETGD;
            if (MODREG) {
                if (rex.rex) {
                    ed = TO_NAT((nextop & 7) + (rex.b << 3));
                    eb1 = ed;
                    eb2 = 0;
                } else {
                    ed = (nextop & 7);
                    eb1 = TO_NAT(ed & 3); // Ax, Cx, Dx or Bx
                    eb2 = (ed & 4) >> 2;  // L or H
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
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGW(x1);
            GETEW(x2, 0);
            if (!(MODREG && wback == TO_NAT(((nextop & 0x38) >> 3) + (rex.r << 3))))
                MV(x7, ed);
            emit_add16(dyn, ninst, ed, gd, x4, x5, x6);
            if (!(MODREG && wback == TO_NAT(((nextop & 0x38) >> 3) + (rex.r << 3))))
                MV(gd, x7);
            EWBACK;
            if (!(MODREG && wback == TO_NAT(((nextop & 0x38) >> 3) + (rex.r << 3))))
                GWBACK;
            break;
        case 0xC2:
            INST_NAME("CMPPD Gx, Ex, Ib");
            nextop = F8;
            GETGX();
            GETEX(x2, 1, 8);
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
                            BEQ(x3, xZR, 8); // MARK
                            FLTD(x3, d0, d1);
                            break; // Less than
                        case 2:
                            BEQ(x3, xZR, 8); // MARK
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
            u8 = (F8) & 7;
            SH(ed, gback, gdoffset + u8 * 2);
            break;
        case 0xC5:
            INST_NAME("PEXTRW Gd,Ex,Ib");
            nextop = F8;
            GETGD;
            GETEX(x1, 0, 14);
            u8 = (F8) & 7;
            LHU(gd, wback, fixedaddress + u8 * 2);
            break;
        case 0xC6:
            INST_NAME("SHUFPD Gx, Ex, Ib");
            nextop = F8;
            GETGX();
            GETEX(x2, 1, 8);
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
        case 0xCF: /* BSWAP reg */
            INST_NAME("BSWAP Reg");
            gd = TO_NAT((opcode & 7) + (rex.b << 3));
            if (rex.w) {
                REV8xw(gd, gd, x1, x2, x3, x4);
            } else {
                ANDI(x1, gd, 0xff);
                SRLI(x2, gd, 8);
                SLLI(x1, x1, 8);
                ANDI(x2, x2, 0xff);
                SRLI(gd, gd, 16);
                OR(x1, x1, x2);
                SLLI(gd, gd, 16);
                OR(gd, gd, x1);
            }
            break;
        case 0xD1:
            INST_NAME("PSRLW Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 1);
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
            GETEX(x2, 0, 1);
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
            GETEX(x2, 0, 1);
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
            GETEX(x2, 0, 8);
            SSE_LOOP_Q(x3, x4, ADD(x3, x3, x4));
            break;
        case 0xD5:
            INST_NAME("PMULLW Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 14);
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
            GETEX(x2, 0, 8);
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
            GETEX(x2, 0, 8);
            GETGD;
            LD(x1, wback, fixedaddress + 8); // high part
            LD(x2, wback, fixedaddress + 0); // low part, also destroyed wback(x2)
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
            for (int i = 0; i < 8; i++) {
                if (cpuext.zbs) {
                    BEXTI(x6, x2, 63 - i * 8);
                } else {
                    SRLI(x6, x2, 63 - i * 8);
                    ANDI(x6, x6, 1);
                }
                if (cpuext.zba) {
                    SH1ADD(gd, gd, x6);
                } else {
                    SLLI(gd, gd, 1);
                    OR(gd, gd, x6);
                }
            }
            break;
        case 0xD8:
            INST_NAME("PSUBUSB Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 15);
            for (int i = 0; i < 16; ++i) {
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
            INST_NAME("PSUBUSW Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 14);
            SSE_LOOP_W(
                x3, x4,
                SUB(x3, x3, x4);
                if (cpuext.zbb) {
                    MAX(x3, x3, xZR);
                } else {
                    NOT(x4, x3);
                    SRAI(x4, x4, 63);
                    AND(x3, x3, x4);
                });
            break;
        case 0xDA:
            INST_NAME("PMINUB Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 15);
            for (int i = 0; i < 16; ++i) {
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
            INST_NAME("PAND Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 8);
            SSE_LOOP_Q(x3, x4, AND(x3, x3, x4));
            break;
        case 0xDC:
            INST_NAME("PADDUSB Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 15);
            ADDI(x5, xZR, 0xFF);
            for (int i = 0; i < 16; ++i) {
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
            INST_NAME("PADDUSW Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 14);
            MOV32w(x5, 65535);
            for (int i = 0; i < 8; ++i) {
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
            INST_NAME("PMAXUB Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 15);
            for (int i = 0; i < 16; ++i) {
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
            INST_NAME("PANDN Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 8);
            SSE_LOOP_Q(x3, x4, NOT(x3, x3); AND(x3, x3, x4));
            break;
        case 0xE0:
            INST_NAME("PAVGB Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 15);
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
            GETEX(x2, 0, 1);
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
            GETEX(x2, 0, 1);
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
            GETEX(x2, 0, 14);
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
            GETEX(x2, 0, 14);
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
            GETEX(x2, 0, 14);
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
            GETEX(x2, 0, 8);
            v0 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            FLD(v0, wback, fixedaddress + 0);
            FLD(v1, wback, fixedaddress + 8);
            if (!BOX64ENV(dynarec_fastround)) {
                FSFLAGSI(0); // // reset all bits
            }
            FCVTWD(x3, v0, RD_RTZ);
            if (!BOX64ENV(dynarec_fastround)) {
                FRFLAGS(x5); // get back FPSR to check the IOC bit
                ANDI(x5, x5, (1 << FR_NV) | (1 << FR_OF));
                BEQ_MARK(x5, xZR);
                MOV32w(x3, 0x80000000);
                MARK;
                FSFLAGSI(0); // // reset all bits
            }
            FCVTWD(x4, v1, RD_RTZ);
            if (!BOX64ENV(dynarec_fastround)) {
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
            GETEX(x2, 0, 8);
            SSE_LOOP_MV_Q2(x3);
            break;
        case 0xE8:
            INST_NAME("PSUBSB Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 15);
            ADDI(x5, xZR, 0x7f);
            ADDI(x6, xZR, 0xf80);
            for (int i = 0; i < 16; ++i) {
                // tmp16s = (int16_t)GX->sb[i] - EX->sb[i];
                // GX->sb[i] = (tmp16s<-128)?-128:((tmp16s>127)?127:tmp16s);
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
            INST_NAME("PSUBSW Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 14);
            MOV64x(x5, 32767);
            MOV64x(x6, -32768);
            for (int i = 0; i < 8; ++i) {
                // tmp32s = (int32_t)GX->sw[i] - EX->sw[i];
                // GX->sw[i] = sat16(tmp32s);
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
                SH(x3, gback, gdoffset + i * 2);
            }
            break;
        case 0xEA:
            INST_NAME("PMINSW Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 14);
            for (int i = 0; i < 8; ++i) {
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
            INST_NAME("POR Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 8);
            SSE_LOOP_Q(x3, x4, OR(x3, x3, x4));
            break;
        case 0xEC:
            INST_NAME("PADDSB Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 15);
            MOV64x(x5, 127);
            MOV64x(x6, -128);
            for (int i = 0; i < 16; ++i) {
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
            INST_NAME("PADDSW Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 14);
            MOV64x(x5, 32767);
            MOV64x(x6, -32768);
            for (int i = 0; i < 8; ++i) {
                // tmp32s = (int32_t)GX->sw[i] + EX->sw[i];
                // GX->sw[i] = sat16(tmp32s);
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
                SH(x3, gback, gdoffset + i * 2);
            }
            break;
        case 0xEE:
            INST_NAME("PMAXSW Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 14);
            SSE_LOOP_WS(x3, x4, if (cpuext.zbb) { MAX(x3, x3, x4); } else {
                    BGE(x3, x4, 8);
                    MV(x3, x4); });
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
                GETEX(x2, 0, 8);
                SSE_LOOP_Q(x3, x4, XOR(x3, x3, x4));
            }
            break;
        case 0xF1:
            INST_NAME("PSLLW Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 1);
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
            INST_NAME("PSLLD Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 1);
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
            GETEX(x2, 0, 1);
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
            GETEX(x2, 0, 8);
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
            GETEX(x2, 0, 14);
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
            GETEX(x2, 0, 15);
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
            GETEX(x2, 0, 15);
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
            GETEX(x2, 0, 14);
            SSE_LOOP_W(x3, x4, SUBW(x3, x3, x4));
            break;
        case 0xFA:
            INST_NAME("PSUBD Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 12);
            SSE_LOOP_D(x3, x4, SUBW(x3, x3, x4));
            break;
        case 0xFB:
            INST_NAME("PSUBQ Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 8);
            SSE_LOOP_Q(x3, x4, SUB(x3, x3, x4));
            break;
        case 0xFC:
            INST_NAME("PADDB Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 15);
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
            GETEX(x2, 0, 14);
            SSE_LOOP_W(x3, x4, ADDW(x3, x3, x4));
            break;
        case 0xFE:
            INST_NAME("PADDD Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 12);
            SSE_LOOP_D(x3, x4, ADDW(x3, x3, x4));
            break;
        default:
            DEFAULT;
    }
    return addr;
}
