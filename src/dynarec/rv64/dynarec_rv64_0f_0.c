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


uintptr_t dynarec64_0F_0(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
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
                        if (box64_rdtsc_shift) { SLLI(x3, x3, box64_rdtsc_shift); }
                        SRLI(xRDX, x3, 32);
                        ZEXTW2(xRAX, x3); // wipe upper part
                        MV(xRCX, xZR);    // IA32_TSC, 0 for now
                        break;
                    default: DEFAULT;
                }
            } else {
                switch ((nextop >> 3) & 7) {
                    default: DEFAULT;
                }
            }
            break;
        case 0x05:
            INST_NAME("SYSCALL");
            if (rex.is32bits) {
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
            }
            NOTEST(x1);
            SMEND();
            GETIP(addr, x7);
            STORE_XEMU_CALL(x3);
            if (!box64_wine || FindElfAddress(my_context, ip)) {
                CALL_S(const_x64syscall_linux, -1, 0);
            } else {
                CALL_S(const_x64syscall, -1, 0);
            }
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
                case 0:
                    INST_NAME("PREFETCH");
                    if (cpuext.zicbop) {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 0, 0);
                        CBO_PREFETCH_R(ed, 0);
                    } else {
                        FAKEED;
                    }
                    break;
                case 1: // PREFETCHW
                case 2: // PREFETCHWT1
                    INST_NAME("PREFETCHWh");
                    if (cpuext.zicbop) {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 0, 0);
                        CBO_PREFETCH_W(ed, 0);
                    } else {
                        FAKEED;
                    }
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
            IF_UNALIGNED (ip) {
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
            if (!MODREG) SMWRITE2();
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
            nextop = F8;
            if (MODREG) {
                INST_NAME("Illegal 0F 13");
                UDF();
            } else {
                INST_NAME("MOVLPS Ex,Gx");
                GETGX();
                GETEX(x2, 0, 1);
                LD(x3, gback, gdoffset + 0);
                SD(x3, wback, fixedaddress + 0);
                SMWRITE2();
            }
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
            }
            GETGX();
            GETEX(x2, 0, 1);
            LD(x4, wback, fixedaddress + 0);
            SD(x4, gback, gdoffset + 8);
            break;
        case 0x17:
            nextop = F8;
            if (MODREG) {
                INST_NAME("Illegal 0F 17");
                UDF();
            } else {
                INST_NAME("MOVHPS Ex,Gx");
                GETGX();
                GETEX(x2, 0, 1);
                LD(x4, gback, gdoffset + 8);
                SD(x4, wback, fixedaddress + 0);
                SMWRITE2();
            }
            break;
        case 0x18:
            nextop = F8;
            if (MODREG) {
                INST_NAME("NOP (multibyte)");
            } else
                switch ((nextop >> 3) & 7) {
                    case 0:
                        INST_NAME("PREFETCHNTA Ed");
                        FAKEED;
                        break;
                    case 1:
                    case 2:
                    case 3:
                        INST_NAME("PREFETCHh Ed");
                        if (cpuext.zicbop) {
                            addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 0, 0);
                            CBO_PREFETCH_R(ed, 0);
                        } else {
                            FAKEED;
                        }
                        break;
                    default: INST_NAME("NOP (multibyte)"); FAKEED;
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
            if (!MODREG) SMWRITE2();
            break;
        case 0x2A:
            INST_NAME("CVTPI2PS Gx,Em");
            nextop = F8;
            GETGX();
            GETEM(x2, 0, 4);
            d0 = fpu_get_scratch(dyn);
            u8 = simd_set_rounding(dyn, ninst, x4, x5);
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
            if (!MODREG) SMWRITE2();
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
                    emit_mxcsr_raise_invalid(x5);
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
            u8 = simd_set_rounding(dyn, ninst, x6, x4);
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
                    emit_mxcsr_raise_invalid(x5);
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
            if (box64_rdtsc_shift) { SLLI(x3, x3, box64_rdtsc_shift); }
            SRLI(xRDX, x3, 32);
            MARKREGd(xRAX);
            MARKREGd(xRDX);
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
                        case 0xC8: INST_NAME("SHA1NEXTE Gx, Ex"); break;
                        case 0xC9: INST_NAME("SHA1MSG1 Gx, Ex"); break;
                        case 0xCA: INST_NAME("SHA1MSG2 Gx, Ex"); break;
                        case 0xCB: INST_NAME("SHA256RNDS2 Gx, Ex"); break;
                        case 0xCC: INST_NAME("SHA256MSG1 Gx, Ex"); break;
                        case 0xCD: INST_NAME("SHA256MSG2 Gx, Ex"); break;
                    }
                    nextop = F8;
                    if (u8 == 0xCB && cpuext.vector && cpuext.zvknha) {
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW32);
                        GETGX_vector(v0, 1, VECTOR_SEW32);
                        GETEX_vector(v1, 0, 0, VECTOR_SEW32);
                        if (v0 == v1) {
                            int t = vpu_get_scratch(dyn);
                            VMV_V_V(t, v1);
                            v1 = t;
                        }
                        int vx0 = simd_get_reg(dyn, ninst, x1, 0, 0, VECTOR_SEW32);
                        if (vx0 == v0 || vx0 == v1) {
                            int t = vpu_get_scratch(dyn);
                            VMV_V_V(t, vx0);
                            vx0 = t;
                        }
                        VSHA2CL_VV(v0, v1, vx0);
                        break;
                    }
                    if (u8 != 0xCB && cpuext.vector && cpuext.zvbb) {
                        SET_ELEMENT_WIDTH(x1, VECTOR_SEW32);
                        GETGX_vector(v0, 1, VECTOR_SEW32);
                        GETEX_vector(v1, 0, 0, VECTOR_SEW32);
                        d0 = vpu_get_scratch(dyn);
                        d1 = vpu_get_scratch(dyn);
                        q1 = vpu_get_scratch(dyn);
                        int vz = vpu_get_scratch(dyn);
                        switch (u8) {
                            case 0xC8:
                                VROR_VI(d0, v0, 2, 1);
                                VADD_VV(d0, d0, v1, 1);
                                if (v0 != v1) VOR_VV(v0, v1, v1, 1);
                                VSLIDEDOWN_VI(d1, d0, 3, 1);
                                VSLIDEUP_VI(v0, d1, 3, 1);
                                break;
                            case 0xC9:
                                VSLIDEDOWN_VI(d0, v1, 2, 1);
                                VSLIDEUP_VI(d0, v0, 2, 1);
                                VXOR_VV(v0, v0, d0, 1);
                                break;
                            case 0xCA:
                                VXOR_VV(vz, vz, vz, 1);
                                VSLIDE1UP_VX(d0, v1, xZR, 1);
                                VXOR_VV(d0, d0, v0, 1);
                                VROR_VI(v0, d0, 31, 1);
                                VSLIDEDOWN_VI(d1, v0, 3, 1);
                                VSLIDEUP_VI(d1, vz, 1, 1);
                                VROR_VI(d1, d1, 31, 1);
                                VXOR_VV(v0, v0, d1, 1);
                                break;
                            case 0xCC:
                                VSLIDEDOWN_VI(d0, v0, 1, 1);
                                VSLIDEUP_VI(d0, v1, 3, 1);
                                VROR_VI(d1, d0, 7, 1);
                                VROR_VI(q1, d0, 18, 1);
                                VXOR_VV(d1, d1, q1, 1);
                                VSRL_VI(q1, d0, 3, 1);
                                VXOR_VV(d1, d1, q1, 1);
                                VADD_VV(v0, v0, d1, 1);
                                break;
                            case 0xCD:
                                VXOR_VV(vz, vz, vz, 1);
                                VSLIDEDOWN_VI(d0, v1, 2, 1);
                                VSLIDEUP_VI(d0, vz, 2, 1);
                                VROR_VI(d1, d0, 17, 1);
                                VROR_VI(q1, d0, 19, 1);
                                VXOR_VV(d1, d1, q1, 1);
                                VSRL_VI(q1, d0, 10, 1);
                                VXOR_VV(d1, d1, q1, 1);
                                VADD_VV(v0, v0, d1, 1);
                                VSLIDEUP_VI(d0, vz, 0, 1);
                                VSLIDEUP_VI(d0, v0, 2, 1);
                                VROR_VI(d1, d0, 17, 1);
                                VROR_VI(q1, d0, 19, 1);
                                VXOR_VV(d1, d1, q1, 1);
                                VSRL_VI(q1, d0, 10, 1);
                                VXOR_VV(d1, d1, q1, 1);
                                VADD_VV(v0, v0, d1, 1);
                                break;
                        }
                        break;
                    }
                    if (MODREG) {
                        ed = (nextop & 7) + (rex.b << 3);
                        simd_reflect_fp_reg(dyn, ninst, x6, ed);
                        ADDI(x2, xEmu, offsetof(x64emu_t, xmm[ed]));
                        ed = x2;
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 0, 0);
                        // the address is passed as an argument to the helper, so it must not stay in a guest register
                        if (ed != x2 || fixedaddress) {
                            ADDI(x2, ed, fixedaddress);
                            ed = x2;
                        }
                    }
                    GETG;
                    simd_forget_fp_reg(dyn, ninst, x6, gd);
                    ADDI(x1, xEmu, offsetof(x64emu_t, xmm[gd]));
                    simd_reflect_fp_reg(dyn, ninst, x6, 0);
                    switch (u8) {
                        case 0xC8: CALL(const_sha1nexte, -1, x1, ed); break;
                        case 0xC9: CALL(const_sha1msg1, -1, x1, ed); break;
                        case 0xCA: CALL(const_sha1msg2, -1, x1, ed); break;
                        case 0xCB: CALL(const_sha256rnds2, -1, x1, ed); break;
                        case 0xCC: CALL(const_sha256msg1, -1, x1, ed); break;
                        case 0xCD: CALL(const_sha256msg2, -1, x1, ed); break;
                    }
                    break;
                case 0xF0:
                    INST_NAME("MOVBE Gd, Ed");
                    nextop = F8;
                    GETGDd;
                    SMREAD();
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                    LDxw(gd, ed, fixedaddress);
                    REV8xw(gd, gd, x1, x2, x3, x4);
                    break;
                case 0xF1:
                    INST_NAME("MOVBE Ed, Gd");
                    nextop = F8;
                    GETGD;
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                    REV8xw(x3, gd, x3, x4, x5, x6);
                    SDxw(x3, wback, fixedaddress);
                    SMWRITE2();
                    break;
                default: DEFAULT;
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
                case 0xCC: {
                    INST_NAME("SHA1RNDS4 Gx, Ex, Ib");
                    nextop = F8;
                    if (!cpuext.vector || !cpuext.zvbb) {
                        if (MODREG) {
                            ed = (nextop & 7) + (rex.b << 3);
                            simd_reflect_fp_reg(dyn, ninst, x6, ed);
                            ADDI(x2, xEmu, offsetof(x64emu_t, xmm[ed]));
                            wback = x2;
                        } else {
                            SMREAD();
                            addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 0, 1);
                            if (wback != x2 || fixedaddress) {
                                ADDI(x2, wback, fixedaddress);
                                wback = x2;
                            }
                        }
                        u8 = F8;
                        GETG;
                        simd_forget_fp_reg(dyn, ninst, x6, gd);
                        ADDI(x1, xEmu, offsetof(x64emu_t, xmm[gd]));
                        MOV32w(x3, u8);
                        CALL4(const_sha1rnds4, -1, x1, wback, x3, 0);
                        break;
                    }
                    static const uint32_t sha1_k[4] = { 0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC, 0xCA62C1D6 };
                    int qA = vpu_get_scratch(dyn);
                    int qB = vpu_get_scratch(dyn);
                    int qC = vpu_get_scratch(dyn);
                    int qD = vpu_get_scratch(dyn);
                    int qT = vpu_get_scratch(dyn);
                    int qU = vpu_get_scratch(dyn);
                    int qM = vpu_get_scratch(dyn);
                    GETG;
                    SET_ELEMENT_WIDTH(x1, VECTOR_SEW32);
                    if (MODREG) {
                        ed = (nextop & 7) + (rex.b << 3);
                        u8 = F8;
                        q0 = simd_get_reg(dyn, ninst, x1, gd, 0, VECTOR_SEW32);
                        q1 = simd_get_reg(dyn, ninst, x1, ed, 0, VECTOR_SEW32);
                        VMV_V_V(qM, q1);
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 1);
                        u8 = F8;
                        q0 = simd_get_reg(dyn, ninst, x1, gd, 0, VECTOR_SEW32);
                        VLE32_V(qM, ed, VECTOR_UNMASKED, VECTOR_NFIELD1);
                    }
                    MOV32w(x7, sha1_k[u8 & 3]);
                    VADD_VX(qM, qM, x7, VECTOR_UNMASKED);
                    VSLIDEDOWN_VI(qA, q0, 3, VECTOR_UNMASKED);
                    VSLIDEDOWN_VI(qB, q0, 2, VECTOR_UNMASKED);
                    VSLIDEDOWN_VI(qC, q0, 1, VECTOR_UNMASKED);

#define SHA1_F(dst, vb, vc, vd, tmp)                     \
    do {                                                 \
        switch (u8 & 3) {                                \
            case 0:                                      \
                VXOR_VV(tmp, vc, vd, VECTOR_UNMASKED);   \
                VAND_VV(tmp, vb, tmp, VECTOR_UNMASKED);  \
                VXOR_VV(dst, tmp, vd, VECTOR_UNMASKED);  \
                break;                                   \
            case 1:                                      \
            case 3:                                      \
                VXOR_VV(dst, vb, vc, VECTOR_UNMASKED);   \
                VXOR_VV(dst, dst, vd, VECTOR_UNMASKED);  \
                break;                                   \
            default:                                     \
                VXOR_VV(dst, vb, vc, VECTOR_UNMASKED);   \
                VAND_VV(dst, dst, vd, VECTOR_UNMASKED);  \
                VAND_VV(tmp, vb, vc, VECTOR_UNMASKED);   \
                VXOR_VV(dst, dst, tmp, VECTOR_UNMASKED); \
                break;                                   \
        }                                                \
    } while (0)

                    SHA1_F(qD, qB, qC, q0, qT);
                    VROR_VI(qB, qB, 2, VECTOR_UNMASKED);
                    SHA1_F(qU, qA, qB, qC, qT);
                    VROR_VI(qT, qA, 2, VECTOR_UNMASKED);
                    VADD_VV(qU, qU, q0, VECTOR_UNMASKED);
                    VSLIDEDOWN_VI(q0, qM, 3, VECTOR_UNMASKED);
                    VADD_VV(qD, qD, q0, VECTOR_UNMASKED);
                    VSLIDEDOWN_VI(q0, qM, 2, VECTOR_UNMASKED);
                    VADD_VV(qU, qU, q0, VECTOR_UNMASKED);
                    VROR_VI(qA, qA, 27, VECTOR_UNMASKED);
                    VSLIDEDOWN_VI(q0, qM, 1, VECTOR_UNMASKED);
                    VADD_VV(qC, qC, q0, VECTOR_UNMASKED);
                    VADD_VV(qA, qA, qD, VECTOR_UNMASKED);

                    VROR_VI(qD, qA, 27, VECTOR_UNMASKED);
                    VADD_VV(qD, qD, qU, VECTOR_UNMASKED);
                    SHA1_F(qU, qA, qT, qB, q0);
                    VADD_VV(qU, qU, qC, VECTOR_UNMASKED);
                    VADD_VV(qB, qB, qM, VECTOR_UNMASKED);

                    VROR_VI(qA, qA, 2, VECTOR_UNMASKED);
                    VROR_VI(qM, qD, 2, VECTOR_UNMASKED);
                    SHA1_F(qC, qD, qA, qT, q0);
                    VROR_VI(q0, qD, 27, VECTOR_UNMASKED);
                    VADD_VV(qC, qC, qB, VECTOR_UNMASKED);
                    VADD_VV(q0, q0, qU, VECTOR_UNMASKED);
                    VROR_VI(qD, q0, 27, VECTOR_UNMASKED);
                    VADD_VV(qD, qD, qC, VECTOR_UNMASKED);

#undef SHA1_F

                    v0 = simd_get_reg_empty(dyn, ninst, x1, gd);
                    rvv_set_vl_for_width(dyn, ninst, x1, VECTOR_SEW32, VECTOR_LMUL1, 16);
                    VMV_X_S(x7, qA);
                    VMV_X_S(x6, qM);
                    VMV_X_S(x5, q0);
                    VMV_X_S(x4, qD);
                    VMV_S_X(qT, x4);
                    VSLIDE1UP_VX(v0, qT, x5, VECTOR_UNMASKED);
                    VSLIDE1UP_VX(qT, v0, x6, VECTOR_UNMASKED);
                    VSLIDE1UP_VX(v0, qT, x7, VECTOR_UNMASKED);
                    break;
                }
                default: DEFAULT;
            }
            break;

#define GO(GETFLAGS, NO, YES, NATNO, NATYES, F)                                                  \
    READFLAGS_FUSION(F, x1, x2, x3, x4, x5);                                                     \
    if (!dyn->insts[ninst].nat_flags_fusion) { GETFLAGS; }                                       \
    nextop = F8;                                                                                 \
    GETGDsd;                                                                                     \
    if (MODREG) {                                                                                \
        ed = TO_NAT((nextop & 7) + (rex.b << 3));                                                \
        MARKREGs(ed);                                                                            \
        if (dyn->insts[ninst].nat_flags_fusion) {                                                \
            NATIVEMV(NATYES, gd, ed);                                                            \
        } else {                                                                                 \
            MV##YES(gd, ed, tmp1);                                                               \
        }                                                                                        \
        if (NEED_ZEROUP(gd)) ZEROUP_RESULT(gd);                                                  \
    } else {                                                                                     \
        addr = geted(dyn, addr, ninst, nextop, &ed, tmp2, tmp3, &fixedaddress, rex, NULL, 1, 0); \
        if (dyn->insts[ninst].nat_flags_fusion) {                                                \
            NATIVEJUMP(NATNO, 8);                                                                \
        } else {                                                                                 \
            B##NO(tmp1, 8);                                                                      \
        }                                                                                        \
        LDxw(gd, ed, fixedaddress);                                                              \
        if (NEED_ZEROUP(gd)) ZEROUP_RESULT(gd);                                                  \
    }

            GOCOND(0x40, "CMOV", "Gd, Ed");
#undef GO
        default: DEFAULT;
    }
    return addr;
}
