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
#include "../dynarec_helper.h"

uintptr_t dynarec64_0F(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
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

        case 0x01:
            // TODO:, /0 is SGDT. While 0F 01 D0 is XGETBV, etc...
            nextop = F8;
            if (MODREG) {
                switch (nextop) {
                    case 0xD0:
                        INST_NAME("XGETBV");
                        ZEXTW2(x1, xRCX);
                        BEQZ_MARK(x1);
                        UDF();
                        MARK;
                        MOV32w(xRAX, 0b111);
                        MOV32w(xRDX, 0);
                        break;
                    case 0xE0:
                    case 0xE1:
                    case 0xE2:
                    case 0xE3:
                    case 0xE4:
                    case 0xE5:
                    case 0xE6:
                    case 0xE7:
                        INST_NAME("SMSW Ed");
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        MOV32w(ed, (1 << 0) | (1 << 4)); // only PE and ET set...
                        break;
                    case 0xF9:
                        INST_NAME("RDTSCP");
                        NOTEST(x1);
                        if (box64_rdtsc) {
                            CALL(const_readtsc, x3, 0, 0); // will return the u64 in x3
                        } else {
                            CSRRS(x3, xZR, 0xC01); // RDTIME
                        }
                        if (box64_rdtsc_shift) {
                            SRLI(x3, x3, box64_rdtsc_shift);
                        }
                        SRLI(xRDX, x3, 32);
                        ZEXTW2(xRAX, x3); // wipe upper part
                        MV(xRCX, xZR);    // IA32_TSC, 0 for now
                        break;
                    default:
                        DEFAULT;
                }
            } else {
                switch ((nextop >> 3) & 7) {
                    default:
                        DEFAULT;
                }
            }
            break;
        case 0x05:
            INST_NAME("SYSCALL");
            NOTEST(x1);
            SMEND();
            GETIP(addr, x7);
            STORE_XEMU_CALL(x3);
            CALL_S(const_x64syscall, -1, 0);
            LOAD_XEMU_CALL();
            TABLE64(x3, addr); // expected return address
            BNE_MARK(xRIP, x3);
            LW(x1, xEmu, offsetof(x64emu_t, quit));
            CBZ_NEXT(x1);
            MARK;
            LOAD_XEMU_REM(x3);
            jump_to_epilog(dyn, 0, xRIP, ninst);
            break;

        case 0x09:
            INST_NAME("WBINVD");
            if (BOX64DRENV(dynarec_safeflags) > 1) {
                READFLAGS(X_PEND);
            } else {
                SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION); // Hack to set flags in "don't care" state
            }
            GETIP(ip, x7);
            STORE_XEMU_CALL(x3);
            CALL(const_native_ud, -1, 0, 0);
            LOAD_XEMU_CALL();
            jump_to_epilog(dyn, 0, xRIP, ninst);
            *need_epilog = 0;
            *ok = 0;
            break;

        case 0x0B:
            INST_NAME("UD2");
            if (BOX64DRENV(dynarec_safeflags) > 1) {
                READFLAGS(X_PEND);
            } else {
                SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION); // Hack to set flags in "don't care" state
            }
            GETIP(ip, x7);
            STORE_XEMU_CALL(x3);
            CALL(const_native_ud, -1, 0, 0);
            LOAD_XEMU_CALL();
            jump_to_epilog(dyn, 0, xRIP, ninst);
            *need_epilog = 0;
            *ok = 0;
            break;

        case 0x0D:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 1:
                    INST_NAME("PREFETCHW");
                    // nop without Zicbom, Zicbop, Zicboz extensions
                    FAKEED;
                    break;
                default: //???
                    DEFAULT;
            }
            break;

        case 0x10:
            INST_NAME("MOVUPS Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 8);
            LD(x3, wback, fixedaddress + 0);
            LD(x4, wback, fixedaddress + 8);
            SD(x3, gback, gdoffset + 0);
            SD(x4, gback, gdoffset + 8);
            break;
        case 0x11:
            INST_NAME("MOVUPS Ex,Gx");
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
                LD(x3, gback, gdoffset + 0);
                LD(x4, gback, gdoffset + 8);
                SD(x3, wback, fixedaddress + 0);
                SD(x4, wback, fixedaddress + 8);
            }
            if (!MODREG)
                SMWRITE2();
            break;
        case 0x12:
            nextop = F8;
            if (MODREG) {
                INST_NAME("MOVHLPS Gx,Ex");
                GETGX();
                GETEX(x2, 0, 8);
                LD(x3, wback, fixedaddress + 8);
                SD(x3, gback, gdoffset + 0);
            } else {
                INST_NAME("MOVLPS Gx,Ex");
                GETEXSD(v0, 0);
                GETGXSD_empty(v1);
                FMVD(v1, v0);
            }
            break;
        case 0x13:
            INST_NAME("MOVLPS Ex,Gx");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 1);
            LD(x3, gback, gdoffset + 0);
            SD(x3, wback, fixedaddress + 0);
            if (!MODREG)
                SMWRITE2();
            break;
        case 0x14:
            INST_NAME("UNPCKLPS Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 4);
            LWU(x5, gback, gdoffset + 1 * 4);
            LWU(x3, wback, fixedaddress + 0);
            LWU(x4, wback, fixedaddress + 4);
            SW(x4, gback, gdoffset + 3 * 4);
            SW(x5, gback, gdoffset + 2 * 4);
            SW(x3, gback, gdoffset + 1 * 4);
            break;
        case 0x15:
            INST_NAME("UNPCKHPS Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 12);
            LWU(x3, wback, fixedaddress + 2 * 4);
            LWU(x4, wback, fixedaddress + 3 * 4);
            LWU(x5, gback, gdoffset + 2 * 4);
            LWU(x6, gback, gdoffset + 3 * 4);
            SW(x5, gback, gdoffset + 0 * 4);
            SW(x3, gback, gdoffset + 1 * 4);
            SW(x6, gback, gdoffset + 2 * 4);
            SW(x4, gback, gdoffset + 3 * 4);
            break;
        case 0x16:
            nextop = F8;
            if (MODREG) {
                INST_NAME("MOVLHPS Gx,Ex");
            } else {
                INST_NAME("MOVHPS Gx,Ex");
                SMREAD();
            }
            GETGX();
            GETEX(x2, 0, 1);
            LD(x4, wback, fixedaddress + 0);
            SD(x4, gback, gdoffset + 8);
            break;
        case 0x17:
            INST_NAME("MOVHPS Ex,Gx");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 1);
            LD(x4, gback, gdoffset + 8);
            SD(x4, wback, fixedaddress + 0);
            if (!MODREG)
                SMWRITE2();
            break;
        case 0x18:
            nextop = F8;
            if (MODREG) {
                INST_NAME("NOP (multibyte)");
            } else
                switch ((nextop >> 3) & 7) {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                        INST_NAME("PREFETCHh Ed");
                        FAKEED;
                        break;
                    default:
                        INST_NAME("NOP (multibyte)");
                        FAKEED;
                }
            break;

        case 0x19:
        case 0x1A:
        case 0x1B:
        case 0x1C:
        case 0x1D:
        case 0x1E:
        case 0x1F:
            INST_NAME("NOP (multibyte)");
            nextop = F8;
            FAKEED;
            break;

        case 0x28:
            INST_NAME("MOVAPS Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 8);
            SSE_LOOP_MV_Q(x3);
            break;
        case 0x29:
            INST_NAME("MOVAPS Ex,Gx");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 8);
            SSE_LOOP_MV_Q2(x3);
            if (!MODREG)
                SMWRITE2();
            break;
        case 0x2A:
            INST_NAME("CVTPI2PS Gx,Em");
            nextop = F8;
            GETGX();
            GETEM(x2, 0, 4);
            d0 = fpu_get_scratch(dyn);
            u8 = sse_setround(dyn, ninst, x4, x5);
            for (int i = 0; i < 2; ++i) {
                LW(x3, wback, fixedaddress + i * 4);
                FCVTSW(d0, x3, RD_DYN);
                FSW(d0, gback, gdoffset + i * 4);
            }
            x87_restoreround(dyn, ninst, u8);
            break;
        case 0x2B:
            INST_NAME("MOVNTPS Ex,Gx");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 8);
            LD(x3, gback, gdoffset + 0);
            LD(x4, gback, gdoffset + 8);
            SD(x3, wback, fixedaddress + 0);
            SD(x4, wback, fixedaddress + 8);
            break;
        case 0x2C:
            INST_NAME("CVTTPS2PI Gm,Ex");
            nextop = F8;
            GETGM();
            GETEX(x2, 0, 4);
            d0 = fpu_get_scratch(dyn);
            for (int i = 0; i < 2; ++i) {
                if (!BOX64ENV(dynarec_fastround)) {
                    FSFLAGSI(0); // // reset all bits
                }
                FLW(d0, wback, fixedaddress + i * 4);
                FCVTWS(x1, d0, RD_RTZ);
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
            INST_NAME("CVTPS2PI Gm, Ex");
            nextop = F8;
            GETGM();
            GETEX(x2, 0, 4);
            d0 = fpu_get_scratch(dyn);
            u8 = sse_setround(dyn, ninst, x6, x4);
            for (int i = 0; i < 2; ++i) {
                if (!BOX64ENV(dynarec_fastround)) {
                    FSFLAGSI(0); // // reset all bits
                }
                FLW(d0, wback, fixedaddress + i * 4);
                FCVTWS(x1, d0, RD_DYN);
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
                INST_NAME("COMISS Gx, Ex");
            } else {
                INST_NAME("UCOMISS Gx, Ex");
            }
            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            nextop = F8;
            GETGXSS(d0);
            GETEXSS(v0, 0);
            CLEAR_FLAGS();
            // if isnan(d0) || isnan(v0)
            IFX (X_ZF | X_PF | X_CF) {
                FEQS(x3, d0, d0);
                FEQS(x2, v0, v0);
                AND(x2, x2, x3);
                BNE_MARK(x2, xZR);
                ORI(xFlags, xFlags, (1 << F_ZF) | (1 << F_PF) | (1 << F_CF));
                B_NEXT_nocond;
            }
            MARK;
            // else if isless(d0, v0)
            IFX (X_CF) {
                FLTS(x2, d0, v0);
                BEQ_MARK2(x2, xZR);
                ORI(xFlags, xFlags, 1 << F_CF);
                B_NEXT_nocond;
            }
            MARK2;
            // else if d0 == v0
            IFX (X_ZF) {
                FEQS(x2, d0, v0);
                CBZ_NEXT(x2);
                ORI(xFlags, xFlags, 1 << F_ZF);
            }
            break;
        case 0x31:
            INST_NAME("RDTSC");
            NOTEST(x1);
            if (box64_rdtsc) {
                CALL(const_readtsc, x3, 0, 0); // will return the u64 in x3
            } else {
                CSRRS(x3, xZR, 0xC01); // RDTIME
            }
            if (box64_rdtsc_shift) {
                SRLI(x3, x3, box64_rdtsc_shift);
            }
            SRLI(xRDX, x3, 32);
            ZEXTW2(xRAX, x3); // wipe upper part
            break;
        case 0x38:
            // SSE3
            nextop = F8;
            switch (nextop) {
                case 0x00:
                    INST_NAME("PSHUFB Gm, Em");
                    nextop = F8;
                    GETGM();
                    GETEM(x2, 0, 7);
                    LD(x4, gback, gdoffset);
                    for (int i = 0; i < 8; ++i) {
                        LB(x3, wback, fixedaddress + i);
                        BGE(x3, xZR, 4 + 4 * 2);
                        SB(xZR, gback, gdoffset + i);
                        J(4 + 4 * 4);
                        ANDI(x3, x3, 0x7);
                        SLLI(x3, x3, 3);
                        SRL(x3, x4, x3);
                        SB(x3, gback, gdoffset + i);
                    }
                    break;
                case 0x01:
                    INST_NAME("PHADDW Gm, Em");
                    nextop = F8;
                    GETGM();
                    for (int i = 0; i < 2; ++i) {
                        // tmp32s = GX->sw[i*2+0]+GX->sw[i*2+1];
                        // GX->sw[i] = sat(tmp32s);
                        LH(x3, gback, gdoffset + 2 * (i * 2 + 0));
                        LH(x4, gback, gdoffset + 2 * (i * 2 + 1));
                        ADDW(x3, x3, x4);
                        SH(x3, gback, gdoffset + i * 2);
                    }
                    if (MODREG && gd == (nextop & 7) + (rex.b << 3)) {
                        // GM->d[1] = GM->d[0];
                        LW(x3, gback, gdoffset + 0);
                        SW(x3, gback, gdoffset + 4);
                    } else {
                        GETEM(x2, 0, 6);
                        for (int i = 0; i < 2; ++i) {
                            // tmp32s = EX->sw[i*2+0] + EX->sw[i*2+1];
                            // GX->sw[4+i] = sat(tmp32s);
                            LH(x3, wback, fixedaddress + 2 * (i * 2 + 0));
                            LH(x4, wback, fixedaddress + 2 * (i * 2 + 1));
                            ADDW(x3, x3, x4);
                            SH(x3, gback, gdoffset + 2 * (2 + i));
                        }
                    }
                    break;
                case 0x02:
                    INST_NAME("PHADDD Gm, Em");
                    nextop = F8;
                    GETGM();
                    // GM->sd[0] += GM->sd[1];
                    LW(x3, gback, gdoffset + 0 * 4);
                    LW(x4, gback, gdoffset + 1 * 4);
                    ADDW(x3, x3, x4);
                    SW(x3, gback, gdoffset + 0 * 4);
                    if (MODREG && gd == (nextop & 7) + (rex.b << 3)) {
                        // GM->sd[1] = GM->sd[0];
                        SW(x3, gback, gdoffset + 1 * 4);
                    } else {
                        GETEM(x2, 0, 4);
                        // GM->sd[1] = EM->sd[0] + EM->sd[1];
                        LW(x3, wback, fixedaddress + 0 * 4);
                        LW(x4, wback, fixedaddress + 1 * 4);
                        ADDW(x3, x3, x4);
                        SW(x3, gback, gdoffset + 1 * 4);
                    }
                    break;
                case 0x03:
                    INST_NAME("PHADDSW Gm, Em");
                    nextop = F8;
                    GETGM();
                    MOV64x(x5, 32767);
                    MOV64x(x6, -32768);
                    for (int i = 0; i < 2; ++i) {
                        // tmp32s = GX->sw[i*2+0]+GX->sw[i*2+1];
                        // GX->sw[i] = sat(tmp32s);
                        LH(x3, gback, gdoffset + 2 * (i * 2 + 0));
                        LH(x4, gback, gdoffset + 2 * (i * 2 + 1));
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
                    if (MODREG && gd == (nextop & 7) + (rex.b << 3)) {
                        // GM->d[1] = GM->d[0];
                        LW(x3, gback, gdoffset + 0);
                        SW(x3, gback, gdoffset + 4);
                    } else {
                        GETEM(x2, 0, 6);
                        for (int i = 0; i < 2; ++i) {
                            // tmp32s = EX->sw[i*2+0] + EX->sw[i*2+1];
                            // GX->sw[4+i] = sat(tmp32s);
                            LH(x3, wback, fixedaddress + 2 * (i * 2 + 0));
                            LH(x4, wback, fixedaddress + 2 * (i * 2 + 1));
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
                            SH(x3, gback, gdoffset + 2 * (2 + i));
                        }
                    }
                    break;
                case 0x04:
                    INST_NAME("PMADDUBSW Gm,Em");
                    nextop = F8;
                    GETGM();
                    GETEM(x2, 0, 7);
                    MOV64x(x5, 32767);
                    MOV64x(x6, -32768);
                    for (int i = 0; i < 4; ++i) {
                        LBU(x3, gback, gdoffset + i * 2);
                        LB(x4, wback, fixedaddress + i * 2);
                        MUL(x7, x3, x4);
                        LBU(x3, gback, gdoffset + i * 2 + 1);
                        LB(x4, wback, fixedaddress + i * 2 + 1);
                        MUL(x3, x3, x4);
                        ADD(x3, x3, x7);
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
                case 0x05:
                    INST_NAME("PHSUBW Gm,Em");
                    nextop = F8;
                    GETGM();
                    for (int i = 0; i < 2; ++i) {
                        // tmp32s = GX->sw[i*2+0]+GX->sw[i*2+1];
                        // GX->sw[i] = sat(tmp32s);
                        LH(x3, gback, gdoffset + 2 * (i * 2 + 0));
                        LH(x4, gback, gdoffset + 2 * (i * 2 + 1));
                        SUBW(x3, x3, x4);
                        SH(x3, gback, gdoffset + i * 2);
                    }
                    if (MODREG && gd == (nextop & 7) + (rex.b << 3)) {
                        // GM->d[1] = GM->d[0];
                        LW(x3, gback, gdoffset + 0);
                        SW(x3, gback, gdoffset + 4);
                    } else {
                        GETEM(x2, 0, 6);
                        for (int i = 0; i < 2; ++i) {
                            // tmp32s = EX->sw[i*2+0] + EX->sw[i*2+1];
                            // GX->sw[4+i] = sat(tmp32s);
                            LH(x3, wback, fixedaddress + 2 * (i * 2 + 0));
                            LH(x4, wback, fixedaddress + 2 * (i * 2 + 1));
                            SUBW(x3, x3, x4);
                            SH(x3, gback, gdoffset + 2 * (2 + i));
                        }
                    }
                    break;
                case 0x06:
                    INST_NAME("PHSUBD Gm,Em");
                    nextop = F8;
                    GETGM();
                    // GM->sd[0] += GM->sd[1];
                    LW(x3, gback, gdoffset + 0 * 4);
                    LW(x4, gback, gdoffset + 1 * 4);
                    SUBW(x3, x3, x4);
                    SW(x3, gback, gdoffset + 0 * 4);
                    if (MODREG && gd == (nextop & 7) + (rex.b << 3)) {
                        // GM->sd[1] = GM->sd[0];
                        SW(x3, gback, gdoffset + 1 * 4);
                    } else {
                        GETEM(x2, 0, 4);
                        // GM->sd[1] = EM->sd[0] + EM->sd[1];
                        LW(x3, wback, fixedaddress + 0 * 4);
                        LW(x4, wback, fixedaddress + 1 * 4);
                        SUBW(x3, x3, x4);
                        SW(x3, gback, gdoffset + 1 * 4);
                    }
                    break;
                case 0x07:
                    INST_NAME("PHSUBSW Gm,Em");
                    nextop = F8;
                    GETGM();
                    MOV64x(x5, 32767);
                    MOV64x(x6, -32768);
                    for (int i = 0; i < 2; ++i) {
                        // tmp32s = GX->sw[i*2+0]+GX->sw[i*2+1];
                        // GX->sw[i] = sat(tmp32s);
                        LH(x3, gback, gdoffset + 2 * (i * 2 + 0));
                        LH(x4, gback, gdoffset + 2 * (i * 2 + 1));
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
                    if (MODREG && gd == (nextop & 7) + (rex.b << 3)) {
                        // GM->d[1] = GM->d[0];
                        LW(x3, gback, gdoffset + 0);
                        SW(x3, gback, gdoffset + 4);
                    } else {
                        GETEM(x2, 0, 6);
                        for (int i = 0; i < 2; ++i) {
                            // tmp32s = EX->sw[i*2+0] + EX->sw[i*2+1];
                            // GX->sw[4+i] = sat(tmp32s);
                            LH(x3, wback, fixedaddress + 2 * (i * 2 + 0));
                            LH(x4, wback, fixedaddress + 2 * (i * 2 + 1));
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
                            SH(x3, gback, gdoffset + 2 * (2 + i));
                        }
                    }
                    break;
                case 0x08:
                    INST_NAME("PSIGNB Gm,Em");
                    nextop = F8;
                    GETGM();
                    GETEM(x2, 0, 7);
                    for (int i = 0; i < 8; ++i) {
                        LB(x3, gback, gdoffset + i);
                        LB(x4, wback, fixedaddress + i);
                        SLT(x1, xZR, x4);
                        SRAI(x5, x4, 63);
                        OR(x1, x1, x5);
                        MUL(x3, x1, x3);
                        SB(x3, gback, gdoffset + i);
                    }
                    break;
                case 0x09:
                    INST_NAME("PSIGNW Gm,Em");
                    nextop = F8;
                    GETGM();
                    GETEM(x2, 0, 6);
                    for (int i = 0; i < 4; ++i) {
                        LH(x3, gback, gdoffset + i * 2);
                        LH(x4, wback, fixedaddress + i * 2);
                        SLT(x1, xZR, x4);
                        SRAI(x5, x4, 63);
                        OR(x1, x1, x5);
                        MUL(x3, x1, x3);
                        SH(x3, gback, gdoffset + i * 2);
                    }
                    break;
                case 0x0A:
                    INST_NAME("PSIGND Gm,Em");
                    nextop = F8;
                    GETGM();
                    GETEM(x2, 0, 4);
                    for (int i = 0; i < 2; ++i) {
                        LW(x3, gback, gdoffset + i * 4);
                        LW(x4, wback, fixedaddress + i * 4);
                        SLT(x1, xZR, x4);
                        SRAI(x5, x4, 63);
                        OR(x1, x1, x5);
                        MUL(x3, x1, x3);
                        SW(x3, gback, gdoffset + i * 4);
                    }
                    break;
                case 0x0B:
                    INST_NAME("PMULHRSW Gm,Em");
                    nextop = F8;
                    GETGM();
                    GETEM(x2, 0, 6);
                    for (int i = 0; i < 4; ++i) {
                        LH(x3, gback, gdoffset + i * 2);
                        LH(x4, wback, fixedaddress + i * 2);
                        MUL(x3, x3, x4);
                        SRAI(x3, x3, 14);
                        ADDI(x3, x3, 1);
                        SRAI(x3, x3, 1);
                        SH(x3, gback, gdoffset + i * 2);
                    }
                    break;
                case 0x1C:
                    INST_NAME("PABSB Gm,Em");
                    nextop = F8;
                    GETGM();
                    GETEM(x2, 0, 7);
                    for (int i = 0; i < 8; ++i) {
                        LB(x4, wback, fixedaddress + i);
                        BGE(x4, xZR, 4 + 4);
                        NEG(x4, x4);
                        SB(x4, gback, gdoffset + i);
                    }
                    break;
                case 0x1D:
                    INST_NAME("PABSW Gm,Em");
                    nextop = F8;
                    GETGM();
                    GETEM(x2, 0, 6);
                    for (int i = 0; i < 4; ++i) {
                        LH(x4, wback, fixedaddress + i * 2);
                        BGE(x4, xZR, 4 + 4);
                        NEG(x4, x4);
                        SH(x4, gback, gdoffset + i * 2);
                    }
                    break;
                case 0x1E:
                    INST_NAME("PABSD Gm,Em");
                    nextop = F8;
                    GETGM();
                    GETEM(x2, 0, 4);
                    for (int i = 0; i < 2; ++i) {
                        LW(x4, wback, fixedaddress + i * 4);
                        BGE(x4, xZR, 4 + 4);
                        NEG(x4, x4);
                        SW(x4, gback, gdoffset + i * 4);
                    }
                    break;
                case 0xC8 ... 0xCD:
                    u8 = nextop;
                    switch (u8) {
                        case 0xC8:
                            INST_NAME("SHA1NEXTE Gx, Ex");
                            break;
                        case 0xC9:
                            INST_NAME("SHA1MSG1 Gx, Ex");
                            break;
                        case 0xCA:
                            INST_NAME("SHA1MSG2 Gx, Ex");
                            break;
                        case 0xCB:
                            INST_NAME("SHA256RNDS2 Gx, Ex");
                            break;
                        case 0xCC:
                            INST_NAME("SHA256MSG1 Gx, Ex");
                            break;
                        case 0xCD:
                            INST_NAME("SHA256MSG2 Gx, Ex");
                            break;
                    }
                    nextop = F8;
                    if (MODREG) {
                        ed = (nextop & 7) + (rex.b << 3);
                        sse_reflect_reg(dyn, ninst, x6, ed);
                        ADDI(x2, xEmu, offsetof(x64emu_t, xmm[ed]));
                        ed = x2;
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 0, 0);
                    }
                    GETG;
                    sse_forget_reg(dyn, ninst, x6, gd);
                    ADDI(x1, xEmu, offsetof(x64emu_t, xmm[gd]));
                    sse_reflect_reg(dyn, ninst, x6, 0);
                    switch (u8) {
                        case 0xC8:
                            CALL(const_sha1nexte, -1, x1, ed);
                            break;
                        case 0xC9:
                            CALL(const_sha1msg1, -1, x1, ed);
                            break;
                        case 0xCA:
                            CALL(const_sha1msg2, -1, x1, ed);
                            break;
                        case 0xCB:
                            CALL(const_sha256rnds2, -1, x1, ed);
                            break;
                        case 0xCC:
                            CALL(const_sha256msg1, -1, x1, ed);
                            break;
                        case 0xCD:
                            CALL(const_sha256msg2, -1, x1, ed);
                            break;
                    }
                    break;
                case 0xF0:
                    INST_NAME("MOVBE Gd, Ed");
                    nextop = F8;
                    GETGD;
                    SMREAD();
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                    LDxw(gd, ed, fixedaddress);
                    REV8xw(gd, gd, x1, x2, x3, x4);
                    break;
                case 0xF1:
                    INST_NAME("MOVBE Ed, Gd");
                    nextop = F8;
                    GETGD;
                    SMREAD();
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                    REV8xw(x1, gd, x1, x2, x3, x4);
                    SDxw(x1, wback, fixedaddress);
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0x3A: // more SSE3 opcodes
            opcode = F8;
            switch (opcode) {
                case 0x0F:
                    INST_NAME("PALIGNR Gm, Em, Ib");
                    nextop = F8;
                    GETGM();
                    GETEM(x2, 1, 1);
                    u8 = F8;
                    if (u8 > 15) {
                        SD(xZR, gback, gdoffset);
                    } else if (u8 > 7) {
                        if (u8 > 8) {
                            LD(x1, gback, gdoffset);
                            SRLI(x1, x1, (u8 - 8) * 8);
                            SD(x1, gback, gdoffset);
                        }
                    } else {
                        if (u8 > 0) {
                            LD(x3, wback, fixedaddress);
                            LD(x1, gback, gdoffset);
                            SRLI(x3, x3, u8 * 8);
                            SLLI(x1, x1, (8 - u8) * 8);
                            OR(x1, x1, x3);
                            SD(x1, gback, gdoffset);
                        } else {
                            LD(x1, wback, fixedaddress);
                            SD(x1, gback, gdoffset);
                        }
                    }
                    break;
                case 0xCC:
                    INST_NAME("SHA1RNDS4 Gx, Ex, Ib");
                    nextop = F8;
                    if (MODREG) {
                        ed = (nextop & 7) + (rex.b << 3);
                        sse_reflect_reg(dyn, ninst, x6, ed);
                        ADDI(x2, xEmu, offsetof(x64emu_t, xmm[ed]));
                        wback = x2;
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 0, 1);
                    }
                    u8 = F8;
                    GETG;
                    sse_forget_reg(dyn, ninst, x6, gd);
                    ADDI(x1, xEmu, offsetof(x64emu_t, xmm[gd]));
                    MOV32w(x3, u8);
                    CALL4(const_sha1rnds4, -1, x1, wback, x3, 0);
                    break;
                default:
                    DEFAULT;
            }
            break;

#define GO(GETFLAGS, NO, YES, NATNO, NATYES, F)                                                  \
    READFLAGS_FUSION(F, x1, x2, x3, x4, x5);                                                     \
    if (!dyn->insts[ninst].nat_flags_fusion) {                                                   \
        GETFLAGS;                                                                                \
    }                                                                                            \
    nextop = F8;                                                                                 \
    GETGD;                                                                                       \
    if (MODREG) {                                                                                \
        ed = TO_NAT((nextop & 7) + (rex.b << 3));                                                \
        if (dyn->insts[ninst].nat_flags_fusion) {                                                \
            NATIVEMV(NATYES, gd, ed);                                                            \
        } else {                                                                                 \
            MV##YES(gd, ed, tmp1);                                                               \
        }                                                                                        \
        if (!rex.w) ZEROUP(gd);                                                                  \
    } else {                                                                                     \
        addr = geted(dyn, addr, ninst, nextop, &ed, tmp2, tmp3, &fixedaddress, rex, NULL, 1, 0); \
        if (dyn->insts[ninst].nat_flags_fusion) {                                                \
            NATIVEJUMP(NATNO, 8);                                                                \
        } else {                                                                                 \
            B##NO(tmp1, 8);                                                                      \
        }                                                                                        \
        LDxw(gd, ed, fixedaddress);                                                              \
    }

            GOCOND(0x40, "CMOV", "Gd, Ed");
#undef GO
        case 0x50:
            INST_NAME("MOVMSKPS Gd, Ex");
            nextop = F8;
            GETGD;
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
            for (int i = 0; i < 4; ++i) {
                FLW(d0, wback, fixedaddress + 4 * i);
                if (!BOX64ENV(dynarec_fastnan)) {
                    FEQS(x3, d0, d0);
                    BNEZ(x3, 4 + 2 * 4); // isnan(d0)? copy it
                    FSW(d0, gback, gdoffset + i * 4);
                    J(4 + 5 * 4); // continue
                }
                FSQRTS(d0, d0);
                if (!BOX64ENV(dynarec_fastnan)) {
                    FEQS(x3, d0, d0);
                    BNEZ(x3, 4 + 4); // isnan(d0)? negate it
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
            LUI(x3, 0x3f800);
            FMVWX(s1, x3); // 1.0f
            if (!BOX64ENV(dynarec_fastnan)) {
                FCVTSW(v0, xZR, RD_DYN);
            }
            for (int i = 0; i < 4; ++i) {
                FLW(s0, wback, fixedaddress + i * 4);
                if (!BOX64ENV(dynarec_fastnan)) {
                    FLTS(x3, v0, s0); // s0 > 0.0f?
                    BNEZ(x3, 4 + 5 * 4);
                    FEQS(x3, v0, s0); // s0 == 0.0f?
                    BEQZ(x3, 4 + 3 * 4);
                    FDIVS(s0, s1, v0); // generate an inf
                    FSW(s0, gback, gdoffset + i * 4);
                    J(4 + 6 * 4); // continue
                }
                FSQRTS(s0, s0);
                FDIVS(s0, s1, s0);
                if (!BOX64ENV(dynarec_fastnan)) {
                    FEQS(x3, s0, s0);
                    BNEZ(x3, 4 + 4); // isnan(s0)? negate it
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
            LUI(x3, 0x3f800);
            FMVWX(d0, x3); // 1.0f
            for (int i = 0; i < 4; ++i) {
                FLW(d1, wback, fixedaddress + 4 * i);
                if (!BOX64ENV(dynarec_fastnan)) {
                    FEQS(x3, d1, d1);
                    BNEZ(x3, 4 + 2 * 4); // isnan(d1)? copy it
                    FSW(d1, gback, gdoffset + i * 4);
                    J(4 + 5 * 4); // continue
                }
                FDIVS(d1, d0, d1);
                if (!BOX64ENV(dynarec_fastnan)) {
                    FEQS(x3, d1, d1);
                    BNEZ(x3, 4 + 4); // isnan(d1)? negate it
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
            for (int i = 0; i < 4; ++i) {
                // GX->f[i] += EX->f[i];
                FLW(s0, wback, fixedaddress + i * 4);
                FLW(s1, gback, gdoffset + i * 4);
                FADDS(s1, s1, s0);
                FSW(s1, gback, gdoffset + i * 4);
            }
            break;
        case 0x59:
            INST_NAME("MULPS Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 12);
            s0 = fpu_get_scratch(dyn);
            s1 = fpu_get_scratch(dyn);
            for (int i = 0; i < 4; ++i) {
                // GX->f[i] *= EX->f[i];
                FLW(s0, wback, fixedaddress + i * 4);
                FLW(s1, gback, gdoffset + i * 4);
                if (!BOX64ENV(dynarec_fastnan)) {
                    FEQS(x3, s0, s0);
                    FEQS(x4, s1, s1);
                }
                FMULS(s0, s0, s1);
                if (!BOX64ENV(dynarec_fastnan)) {
                    AND(x3, x3, x4);
                    BEQZ(x3, 16);
                    FEQS(x3, s0, s0);
                    BNEZ(x3, 8);
                    FNEGS(s0, s0);
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
            FCVTDS(s0, s0);
            FCVTDS(s1, s1);
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
                FCVTSW(s0, x3, RD_RNE);
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
            for (int i = 0; i < 4; ++i) {
                // GX->f[i] -= EX->f[i];
                FLW(s0, wback, fixedaddress + i * 4);
                FLW(s1, gback, gdoffset + i * 4);
                if (!BOX64ENV(dynarec_fastnan)) {
                    FEQS(x3, s0, s0);
                    FEQS(x4, s1, s1);
                }
                FSUBS(s0, s1, s0);
                if (!BOX64ENV(dynarec_fastnan)) {
                    AND(x3, x3, x4);
                    BEQZ(x3, 16);
                    FEQS(x3, s0, s0);
                    BNEZ(x3, 8);
                    FNEGS(s0, s0);
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
                if (!BOX64ENV(dynarec_fastnan)) {
                    FEQS(x3, s0, s0);
                    FEQS(x4, s1, s1);
                    AND(x3, x3, x4);
                    BEQZ(x3, 12);
                    FLTS(x3, s0, s1);
                    BEQZ(x3, 8);
                    FSW(s0, gback, gdoffset + i * 4);
                } else {
                    FMINS(s1, s1, s0);
                    FSW(s1, gback, gdoffset + i * 4);
                }
            }
            break;
        case 0x5E:
            INST_NAME("DIVPS Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0, 12);
            s0 = fpu_get_scratch(dyn);
            s1 = fpu_get_scratch(dyn);
            for (int i = 0; i < 4; ++i) {
                // GX->f[i] /= EX->f[i];
                FLW(s0, wback, fixedaddress + i * 4);
                FLW(s1, gback, gdoffset + i * 4);
                if (!BOX64ENV(dynarec_fastnan)) {
                    FEQS(x3, s0, s0);
                    FEQS(x4, s1, s1);
                }
                FDIVS(s0, s1, s0);
                if (!BOX64ENV(dynarec_fastnan)) {
                    AND(x3, x3, x4);
                    BEQZ(x3, 16);
                    FEQS(x3, s0, s0);
                    BNEZ(x3, 8);
                    FNEGS(s0, s0);
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
                if (!BOX64ENV(dynarec_fastnan)) {
                    FEQS(x3, s0, s0);
                    FEQS(x4, s1, s1);
                    AND(x3, x3, x4);
                    BEQZ(x3, 12);
                    FLTS(x3, s1, s0);
                    BEQZ(x3, 8);
                    FSW(s0, gback, gdoffset + i * 4);
                } else {
                    FMAXS(s1, s1, s0);
                    FSW(s1, gback, gdoffset + i * 4);
                }
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
                BGE(x5, x3, 8);
                ADDI(x3, xZR, 0xFF);
                NOT(x4, x3);
                SRAI(x4, x4, 63);
                AND(x3, x3, x4);
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
                    BGE(x5, x3, 8);
                    ADDI(x3, xZR, 0xFF);
                    NOT(x4, x3);
                    SRAI(x4, x4, 63);
                    AND(x3, x3, x4);
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
            for (int i = 0; i < 2; ++i) {
                // GM->sw[i] = (GM->sd[i]<-32768)?-32768:((GM->sd[i]>32767)?32767:GM->sd[i]);
                LW(x3, gback, gdoffset + i * 4);
                LUI(x4, 0xFFFF8); // -32768
                BGE(x3, x4, 12);
                SH(x4, gback, gdoffset + i * 2);
                J(20);      // continue
                LUI(x4, 8); // 32768
                BLT(x3, x4, 8);
                ADDIW(x3, x4, -1);
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
                    LUI(x4, 0xFFFF8); // -32768
                    BGE(x3, x4, 12);
                    SH(x4, gback, gdoffset + 4 + i * 2);
                    J(20);      // continue
                    LUI(x4, 8); // 32768
                    BLT(x3, x4, 8);
                    ADDIW(x3, x4, -1);
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
                default:
                    *ok = 0;
                    DEFAULT;
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
                default:
                    DEFAULT;
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
                default:
                    DEFAULT;
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
            /*emu->top = 0;
            emu->fpu_stack = 0;*/
            // TODO: Check if something is needed here?
            break;
        case 0x7E:
            INST_NAME("MOVD Ed, Gm");
            nextop = F8;
            gd = ((nextop & 0x38) >> 3);
            v0 = mmx_get_reg(dyn, ninst, x1, x2, x3, gd);
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                if (rex.w)
                    FMVXD(ed, v0);
                else {
                    FMVXW(ed, v0);
                    ZEROUP(ed);
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
            break;
#define GO(GETFLAGS, NO, YES, NATNO, NATYES, F)                                             \
    READFLAGS_FUSION(F, x1, x2, x3, x4, x5);                                                \
    i32_ = F32S;                                                                            \
    if (rex.is32bits)                                                                       \
        j64 = (uint32_t)(addr + i32_);                                                      \
    else                                                                                    \
        j64 = addr + i32_;                                                                  \
    BARRIER(BARRIER_MAYBE);                                                                 \
    JUMP(j64, 1);                                                                           \
    if (!dyn->insts[ninst].nat_flags_fusion) {                                              \
        GETFLAGS;                                                                           \
    }                                                                                       \
    if (dyn->insts[ninst].x64.jmp_insts == -1 || CHECK_CACHE()) {                           \
        /* out of the block */                                                              \
        i32 = dyn->insts[ninst].epilog - (dyn->native_size);                                \
        if (dyn->insts[ninst].nat_flags_fusion) {                                           \
            NATIVEJUMP_safe(NATNO, i32);                                                    \
        } else {                                                                            \
            B##NO##_safe(tmp1, i32);                                                        \
        }                                                                                   \
        if (dyn->insts[ninst].x64.jmp_insts == -1) {                                        \
            if (!(dyn->insts[ninst].x64.barrier & BARRIER_FLOAT))                           \
                fpu_purgecache(dyn, ninst, 1, tmp1, tmp2, tmp3);                            \
            jump_to_next(dyn, j64, 0, ninst, rex.is32bits);                                 \
        } else {                                                                            \
            CacheTransform(dyn, ninst, cacheupd, tmp1, tmp2, tmp3);                         \
            i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address - (dyn->native_size); \
            B(i32);                                                                         \
        }                                                                                   \
    } else {                                                                                \
        /* inside the block */                                                              \
        i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address - (dyn->native_size);     \
        if (dyn->insts[ninst].nat_flags_fusion) {                                           \
            NATIVEJUMP_safe(NATYES, i32);                                                   \
        } else {                                                                            \
            B##YES##_safe(tmp1, i32);                                                       \
        }                                                                                   \
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
            LUI(tmp1, 0xffff0);                                                                  \
            ORI(tmp1, tmp1, 0xff);                                                               \
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

        case 0xA2:
            INST_NAME("CPUID");
            NOTEST(x1);
            CALL_(const_cpuid, -1, 0, xRAX, 0);
            // BX and DX are not synchronized during the call, so need to force the update
            LD(xRDX, xEmu, offsetof(x64emu_t, regs[_DX]));
            LD(xRBX, xEmu, offsetof(x64emu_t, regs[_BX]));
            break;
        case 0xA3:
            INST_NAME("BT Ed, Gd");
            SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x1, &fixedaddress, rex, NULL, 1, 0);
                if (rex.w)
                    SRAI(x1, gd, 6);
                else
                    SRAIW(x1, gd, 5);
                ADDSL(x3, wback, x1, 2 + rex.w, x1);
                LDxw(x1, x3, fixedaddress);
                ed = x1;
            }
            if (X_CF) {
                BEXT(x4, ed, gd, x2);
                ANDI(xFlags, xFlags, ~1); // F_CF is 1
                OR(xFlags, xFlags, x4);
            }
            break;
        case 0xA4:
            nextop = F8;
            INST_NAME("SHLD Ed, Gd, Ib");
            if (geted_ib(dyn, addr, ninst, nextop)) {
                SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                GETED(1);
                GETGD;
                u8 = F8;
                emit_shld32c(dyn, ninst, rex, ed, gd, u8, x3, x4, x5);
                WBACK;
            } else {
                FAKEED;
                if (!rex.w && !rex.is32bits && MODREG) { ZEROUP(ed); }
                F8;
            }
            break;
        case 0xA5:
            nextop = F8;
            INST_NAME("SHLD Ed, Gd, CL");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
            if (BOX64DRENV(dynarec_safeflags) > 1)
                MAYSETFLAGS();
            GETGD;
            GETED(0);
            if (!rex.w && !rex.is32bits && MODREG) { ZEROUP(ed); }
            ANDI(x3, xRCX, rex.w ? 0x3f : 0x1f);
            BEQ_NEXT(x3, xZR);
            emit_shld32(dyn, ninst, rex, ed, gd, x3, x4, x5, x6);
            WBACK;
            break;
        case 0xAB:
            INST_NAME("BTS Ed, Gd");
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
                if (rex.w)
                    SRAI(x1, gd, 6);
                else
                    SRAIW(x1, gd, 5);
                ADDSL(x3, wback, x1, 2 + rex.w, x1);
                LDxw(x1, x3, fixedaddress);
                ed = x1;
                wback = x3;
            }
            BEXT(x4, ed, gd, x2);
            ANDI(xFlags, xFlags, ~1);
            OR(xFlags, xFlags, x4);
            ADDI(x4, xZR, 1);
            ANDI(x2, gd, rex.w ? 0x3f : 0x1f);
            SLL(x4, x4, x2);
            OR(ed, ed, x4);
            if (wback) {
                SDxw(ed, wback, fixedaddress);
                SMWRITE();
            } else if (!rex.w) {
                ZEROUP(ed);
            }
            break;
        case 0xAC:
            nextop = F8;
            INST_NAME("SHRD Ed, Gd, Ib");
            if (geted_ib(dyn, addr, ninst, nextop)) {
                SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                GETED(1);
                GETGD;
                u8 = F8;
                u8 &= (rex.w ? 0x3f : 0x1f);
                emit_shrd32c(dyn, ninst, rex, ed, gd, u8, x3, x4, x5);
                WBACK;
            } else {
                FAKEED;
                if (!rex.w && !rex.is32bits && MODREG) { ZEROUP(ed); }
                F8;
            }
            break;
        case 0xAD:
            nextop = F8;
            INST_NAME("SHRD Ed, Gd, CL");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            if (BOX64DRENV(dynarec_safeflags) > 1)
                MAYSETFLAGS();
            GETGD;
            GETED(0);
            if (!rex.w && !rex.is32bits && MODREG) { ZEROUP(ed); }
            ANDI(x3, xRCX, rex.w ? 0x3f : 0x1f);
            BEQ_NEXT(x3, xZR);
            emit_shrd32(dyn, ninst, rex, ed, gd, x3, x5, x4, x6);
            WBACK;
            break;
        case 0xAE:
            nextop = F8;
            if (MODREG) {
                switch (nextop) {
                    case 0xE8:
                        INST_NAME("LFENCE");
                        SMDMB();
                        break;
                    case 0xF0:
                        INST_NAME("MFENCE");
                        SMDMB();
                        break;
                    case 0xF8:
                        INST_NAME("SFENCE");
                        SMDMB();
                        break;
                    default:
                        DEFAULT;
                }
            } else {
                switch ((nextop >> 3) & 7) {
                    case 0:
                        INST_NAME("FXSAVE Ed");
                        MESSAGE(LOG_DUMP, "Need Optimization\n");
                        SKIPTEST(x1);
                        fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, x3, &fixedaddress, rex, NULL, 0, 0);
                        CALL(rex.is32bits ? (const_fpu_fxsave32) : (const_fpu_fxsave64), -1, ed, 0);
                        break;
                    case 1:
                        INST_NAME("FXRSTOR Ed");
                        MESSAGE(LOG_DUMP, "Need Optimization\n");
                        SKIPTEST(x1);
                        fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, x3, &fixedaddress, rex, NULL, 0, 0);
                        CALL(rex.is32bits ? (const_fpu_fxrstor32) : (const_fpu_fxrstor64), -1, ed, 0);
                        break;
                    case 2:
                        INST_NAME("LDMXCSR Md");
                        GETED(0);
                        SW(ed, xEmu, offsetof(x64emu_t, mxcsr));
                        if (BOX64ENV(sse_flushto0)) {
                            /* RV <-> x86
                               0  <-> 5    inexact
                               1  <-> 4    underflow
                               2  <-> 3    overflow
                               3  <-> 2    divide by zero
                               x  <-> 1    denormal
                               4  <-> 0    invalid operation
                            */
                            // Doing x86 -> RV here, 543210 => 0123x4, ignore denormal
                            // x5 = (ed & 0b1) << 4
                            SLLIW(x5, ed, 4);
                            ANDI(x5, x5, 16);
                            // x3 = x5 | ((ed & 0b100) << 1);
                            SLLIW(x3, ed, 1);
                            ANDI(x3, x3, 8);
                            OR(x3, x3, x5);
                            // x3 = x3 | (ed & 0b1000) >> 1;
                            SRLIW(x4, ed, 1);
                            ANDI(x4, x4, 4);
                            OR(x3, x3, x4);
                            // x3 = x3 | (ed & 0b10000) >> 3;
                            SRLIW(x5, ed, 3);
                            ANDI(x5, x5, 2);
                            OR(x3, x3, x5);
                            // x3 = x3 | (ed & 0b100000) >> 5;
                            SRLIW(x5, ed, 5);
                            ANDI(x5, x5, 1);
                            OR(x3, x3, x5);
                            CSRRW(xZR, x3, /* fflags */ 0x001);
                        }
                        break;
                    case 3:
                        INST_NAME("STMXCSR Md");
                        addr = geted(dyn, addr, ninst, nextop, &wback, x1, x2, &fixedaddress, rex, NULL, 0, 0);
                        LWU(x4, xEmu, offsetof(x64emu_t, mxcsr));
                        if (BOX64ENV(sse_flushto0)) {
                            // Doing RV -> x86, 43210 => 02345, ignore denormal
                            ANDI(x4, x4, 0xfc0);
                            CSRRS(x3, xZR, /* fflags */ 0x001);
                            // x4 = x4 | (x3 & 0b1) << 5;
                            SLLIW(x5, x3, 5);
                            ANDI(x5, x5, 32);
                            OR(x4, x4, x5);
                            // x4 = x4 | (x3 & 0b10) << 3;
                            SLLIW(x6, x3, 3);
                            ANDI(x6, x6, 16);
                            OR(x4, x4, x6);
                            // x4 = x4 | (x3 & 0b100) << 1;
                            SLLIW(x6, x3, 1);
                            ANDI(x6, x6, 8);
                            OR(x4, x4, x6);
                            // x4 = x4 | (x3 & 0b1000) >> 1;
                            SRLIW(x5, x3, 1);
                            ANDI(x5, x5, 4);
                            OR(x4, x4, x5);
                            // x4 = x4 | (x3 & 0b10000) >> 4;
                            SRLIW(x5, x3, 4);
                            ANDI(x5, x5, 2);
                            OR(x4, x4, x5);
                        }
                        SW(x4, wback, fixedaddress);
                        break;
                    case 4:
                        INST_NAME("XSAVE Ed");
                        MESSAGE(LOG_DUMP, "Need Optimization\n");
                        fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 0, 0);
                        MOV32w(x2, rex.w ? 0 : 1);
                        CALL(const_fpu_xsave, -1, ed, x2);
                        break;
                    case 5:
                        INST_NAME("XRSTOR Ed");
                        MESSAGE(LOG_DUMP, "Need Optimization\n");
                        fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 0, 0);
                        MOV32w(x2, rex.w ? 0 : 1);
                        CALL(const_fpu_xrstor, -1, ed, x2);
                        break;
                    case 7:
                        INST_NAME("CLFLUSH Ed");
                        MESSAGE(LOG_DUMP, "Need Optimization?\n");
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 0, 0);
                        CALL_(const_native_clflush, -1, 0, ed, 0);
                        break;
                    default:
                        DEFAULT;
                }
            }
            break;
        case 0xAF:
            // TODO: Refine this
            INST_NAME("IMUL Gd, Ed");
            SETFLAGS(X_ALL, SF_PENDING, NAT_FLAGS_NOFUSION);
            nextop = F8;
            GETGD;
            GETED(0);
            if (rex.w) {
                // 64bits imul
                UFLAG_IF {
                    MULH(x3, gd, ed);
                    MUL(gd, gd, ed);
                    UFLAG_OP1(x3);
                    UFLAG_RES(gd);
                    UFLAG_DF(x3, d_imul64);
                } else {
                    MULxw(gd, gd, ed);
                }
            } else {
                // 32bits imul
                UFLAG_IF {
                    SEXT_W(gd, gd);
                    SEXT_W(x3, ed);
                    MUL(gd, gd, x3);
                    UFLAG_RES(gd);
                    SRLI(x3, gd, 32);
                    UFLAG_OP1(x3);
                    UFLAG_DF(x3, d_imul32);
                } else {
                    MULxw(gd, gd, ed);
                }
                ZEROUP(gd);
            }
            break;
        case 0xB3:
            INST_NAME("BTR Ed, Gd");
            SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                wback = 0;
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                if (rex.w)
                    SRAI(x1, gd, 6);
                else
                    SRAIW(x1, gd, 5);
                ADDSL(x3, wback, x1, 2 + rex.w, x1);
                LDxw(x1, x3, fixedaddress);
                ed = x1;
                wback = x3;
            }
            BEXT(x4, ed, gd, x2); // F_CF is 1
            ANDI(xFlags, xFlags, ~1);
            OR(xFlags, xFlags, x4);
            ADDI(x4, xZR, 1);
            ANDI(x2, gd, rex.w ? 0x3f : 0x1f);
            SLL(x4, x4, x2);
            NOT(x4, x4);
            AND(ed, ed, x4);
            if (wback) {
                SDxw(ed, wback, fixedaddress);
                SMWRITE();
            } else if (!rex.w) {
                ZEROUP(ed);
            }
            break;
        case 0xB6:
            INST_NAME("MOVZX Gd, Eb");
            nextop = F8;
            GETGD;
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
            GETGD;
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
                    GETED(1);
                    u8 = F8;
                    u8 &= (rex.w ? 0x3f : 0x1f);
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
                    if (wback) {
                        SDxw(ed, wback, fixedaddress);
                        SMWRITE();
                    }
                    MARK;
                    if (!rex.w && !wback) ZEROUP(ed);
                    break;
                case 6:
                    INST_NAME("BTR Ed, Ib");
                    SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                    SET_DFNONE();
                    GETED(1);
                    u8 = F8;
                    u8 &= (rex.w ? 0x3f : 0x1f);
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
                    MARK;
                    if (wback) {
                        SDxw(ed, wback, fixedaddress);
                        SMWRITE();
                    } else if (!rex.w) {
                        ZEROUP(ed);
                    }
                    break;
                case 7:
                    INST_NAME("BTC Ed, Ib");
                    SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                    SET_DFNONE();
                    GETED(1);
                    u8 = F8;
                    u8 &= rex.w ? 0x3f : 0x1f;
                    BEXTI(x3, ed, u8); // F_CF is 1
                    ANDI(xFlags, xFlags, ~1);
                    OR(xFlags, xFlags, x3);
                    if (u8 <= 10) {
                        XORI(ed, ed, (1LL << u8));
                    } else {
                        MOV64xw(x3, (1LL << u8));
                        XOR(ed, ed, x3);
                    }
                    if (wback) {
                        SDxw(ed, wback, fixedaddress);
                        SMWRITE();
                    } else if (!rex.w) {
                        ZEROUP(ed);
                    }
                    break;
                default:
                    DEFAULT;
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
                wback = 0;
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x1, &fixedaddress, rex, NULL, 1, 0);
                if (rex.w)
                    SRAI(x1, gd, 6);
                else
                    SRAIW(x1, gd, 5);
                ADDSL(x3, wback, x1, 2 + rex.w, x1);
                LDxw(x1, x3, fixedaddress);
                ed = x1;
                wback = x3;
            }
            BEXT(x4, ed, gd, x2); // F_CF is 1
            ANDI(xFlags, xFlags, ~1);
            OR(xFlags, xFlags, x4);
            ADDI(x4, xZR, 1);
            ANDI(x2, gd, rex.w ? 0x3f : 0x1f);
            SLL(x4, x4, x2);
            XOR(ed, ed, x4);
            if (wback) {
                SDxw(ed, wback, fixedaddress);
                SMWRITE();
            } else if (!rex.w) {
                ZEROUP(ed);
            }
            break;
        case 0xBC:
            INST_NAME("BSF Gd, Ed");
            SETFLAGS(X_ZF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            nextop = F8;
            GETED(0);
            GETGD;
            if (!rex.w && MODREG) {
                ZEXTW2(x4, ed);
                ed = x4;
            }
            BNE_MARK(ed, xZR);
            ORI(xFlags, xFlags, 1 << F_ZF);
            B_NEXT_nocond;
            MARK;
            // gd is undefined if ed is all zeros, don't worry.
            CTZxw(gd, ed, rex.w, x3, x5);
            ANDI(xFlags, xFlags, ~(1 << F_ZF));
            break;
        case 0xBD:
            INST_NAME("BSR Gd, Ed");
            SETFLAGS(X_ZF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            nextop = F8;
            GETED(0);
            GETGD;
            if (!rex.w && MODREG) {
                ZEXTW2(x4, ed);
                ed = x4;
            }
            BNE_MARK(ed, xZR);
            ORI(xFlags, xFlags, 1 << F_ZF);
            B_NEXT_nocond;
            MARK;
            ANDI(xFlags, xFlags, ~(1 << F_ZF));
            CLZxw(gd, ed, rex.w, x3, x5, x7);
            ADDI(x3, xZR, rex.w ? 63 : 31);
            SUB(gd, x3, gd);
            break;
        case 0xBE:
            INST_NAME("MOVSX Gd, Eb");
            nextop = F8;
            GETGD;
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
                ZEROUP(gd);
            break;
        case 0xBF:
            INST_NAME("MOVSX Gd, Ew");
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                SLLI(gd, ed, 48);
                SRAI(gd, gd, 48);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, x1, &fixedaddress, rex, NULL, 1, 0);
                LH(gd, ed, fixedaddress);
            }
            if (!rex.w)
                ZEROUP(gd);
            break;
        case 0xC0:
            INST_NAME("XADD Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETEB(x1, 0);
            GETGB(x2);
            if (!(MODREG && wback == gb1 && !!(wb2) == !!(gb2)))
                MV(x7, ed);
            emit_add8(dyn, ninst, ed, gd, x4, x5, x6);
            if (!(MODREG && wback == gb1 && !!(wb2) == !!(gb2)))
                MV(gd, x7);
            EBBACK(x5, 0);
            if (!(MODREG && wback == gb1 && !!(wb2) == !!(gb2)))
                GBBACK(x5);
            break;
        case 0xC1:
            INST_NAME("XADD Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED(0);
            if (ed != gd)
                MV(x7, ed);
            emit_add32(dyn, ninst, rex, ed, gd, x4, x5, x6);
            if (ed != gd)
                MVxw(gd, x7);
            WBACK;
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
                    if ((u8 & 7) == 5 || (u8 & 7) == 6) {
                        MOV32w(x3, 1);
                    }
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
                MVxw(TO_NAT((nextop & 7) + (rex.b << 3)), gd);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                SDxw(gd, ed, fixedaddress);
            }
            break;
        case 0xC4:
            INST_NAME("PINSRW Gm,Ed,Ib");
            nextop = F8;
            GETED(1);
            GETGM();
            u8 = (F8) & 3;
            SH(ed, gback, gdoffset + u8 * 2);
            break;
        case 0xC5:
            INST_NAME("PEXTRW Gd,Em,Ib");
            nextop = F8;
            GETGD;
            GETEM(x2, 0, 6);
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
                    default:
                        DEFAULT;
                }
            } else {
                switch ((nextop >> 3) & 7) {
                    case 4:
                        INST_NAME("Unsupported XSAVEC Ed");
                        FAKEED;
                        UDF();
                        break;
                    default:
                        DEFAULT;
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
            GETGD;
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
            MMX_LOOP_W(x3, x4, SUB(x3, x3, x4); if (cpuext.zbb) { MAX(x3, x3, xZR); } else {
                    NOT(x4, x3);
                    SRAI(x4, x4, 63);
                    AND(x3, x3, x4); } SH(x3, gback, gdoffset + i * 2););
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
            if (cpuext.zbb) {
                ANDN(x1, x3, x1);
            } else {
                NOT(x1, x1);
                AND(x1, x1, x3);
            }
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
            MMX_LOOP_WS(x3, x4, if (cpuext.zbb) { MAX(x3, x3, x4); } else {
                    BGE(x3, x4, 8);
                    MV(x3, x4); });
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
                SRAIW(x5, x3, 31);
                XOR(x3, x5, x3);
                SUBW(x3, x3, x5);
                ANDI(x3, x3, 0xff);
                ADDW(x6, x6, x3);
                if (i == 7) {
                    SD(x6, gback, gdoffset + 0);
                }
            }
            break;
        case 0xF7:
            INST_NAME("MASKMOVQ Gm, Em");
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
        default:
            DEFAULT;
    }
    return addr;
}
