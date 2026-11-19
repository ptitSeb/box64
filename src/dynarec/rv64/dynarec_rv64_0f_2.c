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


uintptr_t dynarec64_0F_2(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
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
        case 0xA2:
            INST_NAME("CPUID");
            NOTEST(x1);
            CALL_(const_cpuid, -1, 0, 0, 0);
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
                MARKREGs(ed);
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
            }
            IFX (X_CF) {
                BEXT(x4, ed, gd, x2);
                ANDI(xFlags, xFlags, ~1); // F_CF is 1
                OR(xFlags, xFlags, x4);
            }
            break;
        case 0xA4:
            nextop = F8;
            INST_NAME("SHLD Ed, Gd, Ib");
            if (geted_ib(dyn, addr, ninst, nextop) & (rex.w ? 63 : 31)) {
                SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                GETEDsd(1);
                GETGD;
                u8 = F8 & (rex.w ? 63 : 31);
                emit_shld32c(dyn, ninst, rex, ed, gd, u8, x3, x4, x5);
                WBACK;
            } else {
                FAKEED;
                if (!rex.w && MODREG) {
                    if (NEED_ZEROUP(ed)) ZEROUP_RESULT(ed);
                }
                F8;
            }
            break;
        case 0xA5:
            nextop = F8;
            INST_NAME("SHLD Ed, Gd, CL");
            if (BOX64DRENV(dynarec_safeflags) > 1) {
                READFLAGS(X_ALL);
                SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION);
            } else
                SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_NOFUSION); // some flags are left undefined
            GETGD;
            GETEDsd(0);
            if (!rex.w && MODREG) {
                if (NEED_ZEROUP(ed)) ZEROUP_RESULT(ed);
            }
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
            BEXT(x4, ed, gd, x2);
            ANDI(xFlags, xFlags, ~1);
            OR(xFlags, xFlags, x4);
            if (cpuext.zbs) {
                ANDI(x2, gd, rex.w ? 0x3f : 0x1f);
                BSET(ed, ed, x2);
            } else {
                ADDI(x4, xZR, 1);
                ANDI(x2, gd, rex.w ? 0x3f : 0x1f);
                SLL(x4, x4, x2);
                OR(ed, ed, x4);
            }
            if (wback) {
                SDxw(ed, wback, fixedaddress);
                SMWRITE();
            } else if (!rex.w) {
                if (NEED_ZEROUP(ed)) ZEROUP_RESULT(ed);
            }
            break;
        case 0xAC:
            nextop = F8;
            INST_NAME("SHRD Ed, Gd, Ib");
            if (geted_ib(dyn, addr, ninst, nextop) & (rex.w ? 63 : 31)) {
                SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                GETEDsd(1);
                GETGD;
                u8 = F8 & (rex.w ? 63 : 31);
                emit_shrd32c(dyn, ninst, rex, ed, gd, u8, x3, x4, x5);
                WBACK;
            } else {
                FAKEED;
                if (!rex.w && MODREG) {
                    if (NEED_ZEROUP(ed)) ZEROUP_RESULT(ed);
                }
                F8;
            }
            break;
        case 0xAD:
            nextop = F8;
            INST_NAME("SHRD Ed, Gd, CL");
            if (BOX64DRENV(dynarec_safeflags) > 1) {
                READFLAGS(X_ALL);
                SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION);
            } else
                SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_NOFUSION);
            GETGD;
            GETEDsd(0);
            if (!rex.w && MODREG) {
                if (NEED_ZEROUP(ed)) ZEROUP_RESULT(ed);
            }
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
                    default: DEFAULT;
                }
            } else {
                switch ((nextop >> 3) & 7) {
                    case 0:
                        INST_NAME("FXSAVE Ed");
                        MESSAGE(LOG_DUMP, "Need Optimization\n");
                        SKIPTEST(x1);
                        BARRIER(BARRIER_FLOAT);
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, x3, &fixedaddress, rex, NULL, 0, 0);
                        CALL(rex.is32bits ? (const_fpu_fxsave32) : (const_fpu_fxsave64), -1, ed, 0);
                        break;
                    case 1:
                        INST_NAME("FXRSTOR Ed");
                        MESSAGE(LOG_DUMP, "Need Optimization\n");
                        SKIPTEST(x1);
                        BARRIER(BARRIER_FLOAT);
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
                            BEXTI(x5, ed, 5);
                            OR(x3, x3, x5);
                            CSRRW(xZR, x3, /* fflags */ 0x001);
                        }
                        simd_sync_rounding(dyn, ninst, x3, x4);
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
                            ANDI(x5, x5, 1);
                            OR(x4, x4, x5);
                        }
                        SW(x4, wback, fixedaddress);
                        break;
                    case 4:
                        INST_NAME("XSAVE Ed");
                        MESSAGE(LOG_DUMP, "Need Optimization\n");
                        BARRIER(BARRIER_FLOAT);
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 0, 0);
                        MOV32w(x2, rex.is32bits);
                        CALL(const_fpu_xsave, -1, ed, x2);
                        break;
                    case 5:
                        INST_NAME("XRSTOR Ed");
                        MESSAGE(LOG_DUMP, "Need Optimization\n");
                        BARRIER(BARRIER_FLOAT);
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 0, 0);
                        MOV32w(x2, rex.is32bits);
                        CALL(const_fpu_xrstor, -1, ed, x2);
                        break;
                    case 7:
                        INST_NAME("CLFLUSH Ed");
                        if (cpuext.zicbom) {
                            addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 0, 0);
                            CBO_FLUSH(ed);
                        } else {
                            FAKEED;
                        }
                        SMDMB();
                        break;
                    default: DEFAULT;
                }
            }
            break;
        case 0xAF:
            INST_NAME("IMUL Gd, Ed");
            SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION);
            nextop = F8;
            GETGDsd;
            GETED(0);
            CLEAR_FLAGS();
            if (rex.w) {
                // 64bits imul
                UFLAG_IF {
                    MULH(x3, gd, ed);
                    MUL(gd, gd, ed);
                    SET_DFNONE();
                    IFX (X_CF | X_OF) {
                        SRAI(x5, gd, 63);
                        XOR(x3, x3, x5);
                        SNEZ(x3, x3);
                        IFX (X_CF) OR(xFlags, xFlags, x3); // F_CF == 0
                        IFX (X_OF) {
                            SLLI(x3, x3, F_OF2);
                            OR(xFlags, xFlags, x3);
                        }
                    }
                } else {
                    MULxw(gd, gd, ed);
                }
            } else {
                // 32bits imul
                UFLAG_IF {
                    SEXT_W(gd, gd);
                    SEXT_W(x3, ed);
                    MUL(x5, gd, x3); // full 64bits product
                    SET_DFNONE();
                    IFX (X_CF | X_OF) {
                        SEXT_W(x3, x5);
                        XOR(x3, x5, x3);
                        SNEZ(x3, x3);
                        IFX (X_CF) OR(xFlags, xFlags, x3); // F_CF == 0
                        IFX (X_OF) {
                            SLLI(x3, x3, F_OF2);
                            OR(xFlags, xFlags, x3);
                        }
                    }
                    ZEXTW2(gd, x5);
                } else {
                    MULxw(gd, gd, ed);
                    if (NEED_ZEROUP(gd)) ZEROUP_RESULT(gd);
                }
            }
            IFX (X_SF) {
                SRLI(x5, gd, rex.w ? 63 : 31);
                SLLI(x5, x5, F_SF);
                OR(xFlags, xFlags, x5);
            }
            IFX (X_PF) emit_pf(dyn, ninst, gd, x3, x4);
            break;
        default: DEFAULT;
    }
    return addr;
}
