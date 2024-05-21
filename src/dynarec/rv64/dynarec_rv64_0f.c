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
#include "my_cpuid.h"
#include "emu/x87emu_private.h"
#include "emu/x64shaext.h"
#include "bitutils.h"

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_functions.h"
#include "dynarec_rv64_helper.h"

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
            if(MODREG) {
                switch(nextop) {
                    case 0xD0:
                        INST_NAME("FAKE xgetbv");
                        nextop = F8;
                        addr = fakeed(dyn, addr, ninst, nextop);
                        SETFLAGS(X_ALL, SF_SET_NODF); // Hack to set flags in "don't care" state
                        GETIP(ip);
                        STORE_XEMU_CALL(x3);
                        CALL(native_ud, -1);
                        LOAD_XEMU_CALL();
                        jump_to_epilog(dyn, 0, xRIP, ninst);
                        *need_epilog = 0;
                        *ok = 0;
                        break;

                    case 0xF9:
                        INST_NAME("RDTSCP");
                        NOTEST(x1);
                        if (box64_rdtsc) {
                            CALL(ReadTSC, x3); // will return the u64 in x3
                        } else {
                            CSRRS(x3, xZR, 0xC01); // RDTIME
                        }
                        if(box64_rdtsc_shift) {
                            SRLI(x3, x3, box64_rdtsc_shift);
                        }
                        SRLI(xRDX, x3, 32);
                        AND(xRAX, x3, xMASK); // wipe upper part
                        MV(xRCX, xZR);    // IA32_TSC, 0 for now
                        break;
                    default:
                        DEFAULT;
                }
            } else {
                switch((nextop>>3)&7) {
                    default:
                        DEFAULT;
                }
            }
            break;
        case 0x05:
            INST_NAME("SYSCALL");
            NOTEST(x1);
            SMEND();
            GETIP(addr);
            STORE_XEMU_CALL(x3);
            CALL_S(x64Syscall, -1);
            LOAD_XEMU_CALL();
            TABLE64(x3, addr); // expected return address
            BNE_MARK(xRIP, x3);
            LW(w1, xEmu, offsetof(x64emu_t, quit));
            CBZ_NEXT(w1);
            MARK;
            LOAD_XEMU_REM(x3);
            jump_to_epilog(dyn, 0, xRIP, ninst);
            break;

        case 0x09:
            INST_NAME("WBINVD");
            SETFLAGS(X_ALL, SF_SET_NODF); // Hack to set flags in "don't care" state
            GETIP(ip);
            STORE_XEMU_CALL(x3);
            CALL(native_ud, -1);
            LOAD_XEMU_CALL();
            jump_to_epilog(dyn, 0, xRIP, ninst);
            *need_epilog = 0;
            *ok = 0;
            break;

        case 0x0B:
            INST_NAME("UD2");
            SETFLAGS(X_ALL, SF_SET_NODF); // Hack to set flags in "don't care" state
            GETIP(ip);
            STORE_XEMU_CALL(x3);
            CALL(native_ud, -1);
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
            GETEX(x2, 0);
            LD(x3, wback, fixedaddress + 0);
            LD(x4, wback, fixedaddress + 8);
            SD(x3, gback, gdoffset + 0);
            SD(x4, gback, gdoffset + 8);
            break;
        case 0x11:
            INST_NAME("MOVUPS Ex,Gx");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            LD(x3, gback, gdoffset + 0);
            LD(x4, gback, gdoffset + 8);
            SD(x3, wback, fixedaddress + 0);
            SD(x4, wback, fixedaddress + 8);
            if (!MODREG)
                SMWRITE2();
            break;
        case 0x12:
            nextop = F8;
            if (MODREG) {
                INST_NAME("MOVHLPS Gx,Ex");
                GETGX();
                GETEX(x2, 0);
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
            GETEX(x2, 0);
            LD(x3, gback, gdoffset + 0);
            SD(x3, wback, fixedaddress + 0);
            if (!MODREG)
                SMWRITE2();
            break;
        case 0x14:
            INST_NAME("UNPCKLPS Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
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
            GETEX(x2, 0);
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
            GETEX(x2, 0);
            LD(x4, wback, fixedaddress + 0);
            SD(x4, gback, gdoffset + 8);
            break;
        case 0x17:
            INST_NAME("MOVHPS Ex,Gx");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            LD(x4, gback, gdoffset + 8);
            SD(x4, wback, fixedaddress + 0);
            if (!MODREG)
                SMWRITE2();
            break;
        case 0x18:
            nextop = F8;
            if ((nextop & 0xC0) == 0xC0) {
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

        case 0x1F:
            INST_NAME("NOP (multibyte)");
            nextop = F8;
            FAKEED;
            break;

        case 0x28:
            INST_NAME("MOVAPS Gx,Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            SSE_LOOP_MV_Q(x3);
            break;
        case 0x29:
            INST_NAME("MOVAPS Ex,Gx");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            SSE_LOOP_MV_Q2(x3);
            if (!MODREG)
                SMWRITE2();
            break;
        case 0x2A:
            INST_NAME("CVTPI2PS Gx,Em");
            nextop = F8;
            GETGX();
            GETEM(x2, 0);
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
            GETEX(x2, 0);
            LD(x3, gback, gdoffset + 0);
            LD(x4, gback, gdoffset + 8);
            SD(x3, wback, fixedaddress + 0);
            SD(x4, wback, fixedaddress + 8);
            break;
        case 0x2C:
            INST_NAME("CVTTPS2PI Gm,Ex");
            nextop = F8;
            GETGM();
            GETEX(x2, 0);
            d0 = fpu_get_scratch(dyn);
            for (int i = 0; i < 2; ++i) {
                if (!box64_dynarec_fastround) {
                    FSFLAGSI(0); // // reset all bits
                }
                FLW(d0, wback, fixedaddress + i * 4);
                FCVTWS(x1, d0, RD_RTZ);
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
            INST_NAME("CVTPS2PI Gm, Ex");
            nextop = F8;
            GETGM();
            GETEX(x2, 0);
            d0 = fpu_get_scratch(dyn);
            u8 = sse_setround(dyn, ninst, x6, x4);
            for (int i = 0; i < 2; ++i) {
                if (!box64_dynarec_fastround) {
                    FSFLAGSI(0); // // reset all bits
                }
                FLW(d0, wback, fixedaddress + i * 4);
                FCVTWS(x1, d0, RD_DYN);
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
                INST_NAME("COMISS Gx, Ex");
            } else {
                INST_NAME("UCOMISS Gx, Ex");
            }
            SETFLAGS(X_ALL, SF_SET);
            SET_DFNONE();
            nextop = F8;
            GETGXSS(d0);
            GETEXSS(v0, 0);
            CLEAR_FLAGS();
            // if isnan(d0) || isnan(v0)
            IFX(X_ZF | X_PF | X_CF)
            {
                FEQS(x3, d0, d0);
                FEQS(x2, v0, v0);
                AND(x2, x2, x3);
                BNE_MARK(x2, xZR);
                ORI(xFlags, xFlags, (1 << F_ZF) | (1 << F_PF) | (1 << F_CF));
                B_NEXT_nocond;
            }
            MARK;
            // else if isless(d0, v0)
            IFX(X_CF)
            {
                FLTS(x2, d0, v0);
                BEQ_MARK2(x2, xZR);
                ORI(xFlags, xFlags, 1 << F_CF);
                B_NEXT_nocond;
            }
            MARK2;
            // else if d0 == v0
            IFX(X_ZF)
            {
                FEQS(x2, d0, v0);
                CBZ_NEXT(x2);
                ORI(xFlags, xFlags, 1 << F_ZF);
            }
            break;
        case 0x31:
            INST_NAME("RDTSC");
            NOTEST(x1);
            if (box64_rdtsc) {
                CALL(ReadTSC, x3); // will return the u64 in x3
            } else {
                CSRRS(x3, xZR, 0xC01); // RDTIME
            }
            if(box64_rdtsc_shift) {
                SRLI(x3, x3, box64_rdtsc_shift);
            }
            SRLI(xRDX, x3, 32);
            AND(xRAX, x3, xMASK); // wipe upper part
            break;
        case 0x38:
            // SSE3
            nextop = F8;
            switch (nextop) {
                case 0x00:
                    INST_NAME("PSHUFB Gm, Em");
                    nextop = F8;
                    GETGM();
                    GETEM(x2, 0);
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
                        sse_reflect_reg(dyn, ninst, ed);
                        ADDI(x2, xEmu, offsetof(x64emu_t, xmm[ed]));
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 0, 0);
                        if (ed != x2) {
                            MV(x2, ed);
                        }
                    }
                    GETG;
                    sse_forget_reg(dyn, ninst, gd);
                    ADDI(x1, xEmu, offsetof(x64emu_t, xmm[gd]));
                    sse_reflect_reg(dyn, ninst, 0);
                    switch (u8) {
                        case 0xC8:
                            CALL(sha1nexte, -1);
                            break;
                        case 0xC9:
                            CALL(sha1msg1, -1);
                            break;
                        case 0xCA:
                            CALL(sha1msg2, -1);
                            break;
                        case 0xCB:
                            CALL(sha256rnds2, -1);
                            break;
                        case 0xCC:
                            CALL(sha256msg1, -1);
                            break;
                        case 0xCD:
                            CALL(sha256msg2, -1);
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
                case 0xCC:
                    INST_NAME("SHA1RNDS4 Gx, Ex, Ib");
                    nextop = F8;
                    if (MODREG) {
                        ed = (nextop & 7) + (rex.b << 3);
                        sse_reflect_reg(dyn, ninst, ed);
                        ADDI(x2, xEmu, offsetof(x64emu_t, xmm[ed]));
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 0, 1);
                        if (wback != x2) MV(x2, wback);
                    }
                    u8 = F8;
                    GETG;
                    sse_forget_reg(dyn, ninst, gd);
                    ADDI(x1, xEmu, offsetof(x64emu_t, xmm[gd]));
                    MOV32w(x3, u8);
                    CALL(sha1rnds4, -1);
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
        B##NO(x1, 8);                                                                        \
        MV(gd, ed);                                                                          \
    } else {                                                                                 \
        addr = geted(dyn, addr, ninst, nextop, &ed, x2, x4, &fixedaddress, rex, NULL, 1, 0); \
        B##NO(x1, 8);                                                                        \
        LDxw(gd, ed, fixedaddress);                                                          \
    }                                                                                        \
    if (!rex.w) ZEROUP(gd);

            GOCOND(0x40, "CMOV", "Gd, Ed");
#undef GO
        case 0x50:
            INST_NAME("MOVMSKPS Gd, Ex");
            nextop = F8;
            GETGD;
            GETEX(x1, 0);
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
            GETEX(x2, 0);
            d0 = fpu_get_scratch(dyn);
            for (int i = 0; i < 4; ++i) {
                FLW(d0, wback, fixedaddress + 4 * i);
                FSQRTS(d0, d0);
                FSW(d0, gback, gdoffset + 4 * i);
            }
            break;
        case 0x52:
            INST_NAME("RSQRTPS Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            s0 = fpu_get_scratch(dyn);
            s1 = fpu_get_scratch(dyn); // 1.0f
            v0 = fpu_get_scratch(dyn); // 0.0f
            // do accurate computation, because riscv doesn't have rsqrt
            MOV32w(x3, 1);
            FCVTSW(s1, x3, RD_DYN);
            if (!box64_dynarec_fastnan) {
                FCVTSW(v0, xZR, RD_DYN);
            }
            for (int i = 0; i < 4; ++i) {
                FLW(s0, wback, fixedaddress + i * 4);
                if (!box64_dynarec_fastnan) {
                    FLES(x3, v0, s0); // s0 >= 0.0f?
                    BNEZ(x3, 6 * 4);
                    FEQS(x3, s0, s0); // isnan(s0)?
                    BEQZ(x3, 2 * 4);
                    // s0 is negative, so generate a NaN
                    FDIVS(s0, s1, v0);
                    // s0 is a NaN, just copy it
                    FSW(s0, gback, gdoffset + i * 4);
                    J(4 * 4);
                    // do regular computation
                }
                FSQRTS(s0, s0);
                FDIVS(s0, s1, s0);
                FSW(s0, gback, gdoffset + i * 4);
            }
            break;
        case 0x53:
            INST_NAME("RCPPS Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            LUI(x3, 0x3f800);
            FMVWX(d0, x3); // 1.0f
            for (int i = 0; i < 4; ++i) {
                FLW(d1, wback, fixedaddress + 4 * i);
                FDIVS(d1, d0, d1);
                FSW(d1, gback, gdoffset + 4 * i);
            }
            break;
        case 0x54:
            INST_NAME("ANDPS Gx, Ex");
            nextop = F8;
            gd = ((nextop & 0x38) >> 3) + (rex.r << 3);
            if (!(MODREG && gd == (nextop & 7) + (rex.b << 3))) {
                GETGX();
                GETEX(x2, 0);
                SSE_LOOP_Q(x3, x4, AND(x3, x3, x4));
            }
            break;
        case 0x55:
            INST_NAME("ANDNPS Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            SSE_LOOP_Q(x3, x4, NOT(x3, x3); AND(x3, x3, x4));
            break;
        case 0x56:
            INST_NAME("ORPS Gx, Ex");
            nextop = F8;
            gd = ((nextop & 0x38) >> 3) + (rex.r << 3);
            if (!(MODREG && gd == (nextop & 7) + (rex.b << 3))) {
                GETGX();
                GETEX(x2, 0);
                SSE_LOOP_Q(x3, x4, OR(x3, x3, x4));
            }
            break;
        case 0x57:
            INST_NAME("XORPS Gx, Ex");
            nextop = F8;
            // TODO: it might be possible to check if SS or SD are used and not purge them to optimize a bit
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
        case 0x58:
            INST_NAME("ADDPS Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
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
            GETEX(x2, 0);
            s0 = fpu_get_scratch(dyn);
            s1 = fpu_get_scratch(dyn);
            for (int i = 0; i < 4; ++i) {
                // GX->f[i] *= EX->f[i];
                FLW(s0, wback, fixedaddress + i * 4);
                FLW(s1, gback, gdoffset + i * 4);
                FMULS(s1, s1, s0);
                FSW(s1, gback, gdoffset + i * 4);
            }
            break;
        case 0x5A:
            INST_NAME("CVTPS2PD Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
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
            GETEX(x2, 0);
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
            GETEX(x2, 0);
            s0 = fpu_get_scratch(dyn);
            s1 = fpu_get_scratch(dyn);
            for (int i = 0; i < 4; ++i) {
                // GX->f[i] -= EX->f[i];
                FLW(s0, wback, fixedaddress + i * 4);
                FLW(s1, gback, gdoffset + i * 4);
                FSUBS(s1, s1, s0);
                FSW(s1, gback, gdoffset + i * 4);
            }
            break;
        case 0x5D:
            INST_NAME("MINPS Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            s0 = fpu_get_scratch(dyn);
            s1 = fpu_get_scratch(dyn);
            for (int i = 0; i < 4; ++i) {
                FLW(s0, wback, fixedaddress + i * 4);
                FLW(s1, gback, gdoffset + i * 4);
                if (!box64_dynarec_fastnan) {
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
            GETEX(x2, 0);
            s0 = fpu_get_scratch(dyn);
            s1 = fpu_get_scratch(dyn);
            for (int i = 0; i < 4; ++i) {
                // GX->f[i] /= EX->f[i];
                FLW(s0, wback, fixedaddress + i * 4);
                FLW(s1, gback, gdoffset + i * 4);
                FDIVS(s1, s1, s0);
                FSW(s1, gback, gdoffset + i * 4);
            }
            break;
        case 0x5F:
            INST_NAME("MAXPS Gx, Ex");
            nextop = F8;
            GETGX();
            GETEX(x2, 0);
            s0 = fpu_get_scratch(dyn);
            s1 = fpu_get_scratch(dyn);
            for (int i = 0; i < 4; ++i) {
                FLW(s0, wback, fixedaddress + i * 4);
                FLW(s1, gback, gdoffset + i * 4);
                if (!box64_dynarec_fastnan) {
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
                GETEM(x2, 0);
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
            GETEM(x2, 0);
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
            GETEM(x2, 0);
            // GM->ud[1] = EM->ud[0];
            LWU(x3, wback, fixedaddress);
            SW(x3, gback, gdoffset + 4 * 1);
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
                GETEM(x1, 0);
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
                GETEM(x2, 0);
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
                GETEM(x1, 0);
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
            GETEM(x1, 0);
            GETGM();
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
                GETEM(x1, 0);
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
                ed = xRAX + (nextop & 7) + (rex.b << 3);
                if (!rex.w) {
                    AND(x4, ed, xMASK);
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
            GETEM(x2, 0);
            LD(x3, wback, fixedaddress);
            SD(x3, gback, gdoffset + 0);
            break;
        case 0x70:
            INST_NAME("PSHUFW Gm, Em, Ib");
            nextop = F8;
            GETGM();
            GETEM(x2, 1);
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
                    GETEM(x1, 1);
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
                    GETEM(x1, 1);
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
                    GETEM(x1, 1);
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
                    GETEM(x4, 1);
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
                    GETEM(x4, 1);
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
                    GETEM(x4, 1);
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
                    GETEM(x4, 1);
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
                    GETEM(x4, 1);
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
        case 0x75:
            INST_NAME("PCMPEQW Gm,Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0);
            MMX_LOOP_W(x3, x4, SUB(x3, x3, x4); SEQZ(x3, x3); NEG(x3, x3));
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
            GETGM();
            if ((nextop & 0xC0) == 0xC0) {
                ed = xRAX + (nextop & 7) + (rex.b << 3);
                LDxw(ed, gback, gdoffset);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x2, &fixedaddress, rex, NULL, 1, 0);
                LDxw(x1, gback, gdoffset);
                SDxw(x1, wback, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0x7F:
            INST_NAME("MOVQ Em, Gm");
            nextop = F8;
            GETGM();
            GETEM(x2, 0);
            LD(x3, gback, gdoffset + 0);
            SD(x3, wback, fixedaddress);
            break;
#define GO(GETFLAGS, NO, YES, F)                                                            \
    if (box64_dynarec_test == 2) { NOTEST(x1); }                                            \
    READFLAGS(F);                                                                           \
    i32_ = F32S;                                                                            \
    BARRIER(BARRIER_MAYBE);                                                                 \
    JUMP(addr + i32_, 1);                                                                   \
    GETFLAGS;                                                                               \
    if (dyn->insts[ninst].x64.jmp_insts == -1 || CHECK_CACHE()) {                           \
        /* out of the block */                                                              \
        i32 = dyn->insts[ninst].epilog - (dyn->native_size);                                \
        B##NO##_safe(x1, i32);                                                              \
        if (dyn->insts[ninst].x64.jmp_insts == -1) {                                        \
            if (!(dyn->insts[ninst].x64.barrier & BARRIER_FLOAT))                           \
                fpu_purgecache(dyn, ninst, 1, x1, x2, x3);                                  \
            jump_to_next(dyn, addr + i32_, 0, ninst, rex.is32bits);                         \
        } else {                                                                            \
            CacheTransform(dyn, ninst, cacheupd, x1, x2, x3);                               \
            i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address - (dyn->native_size); \
            B(i32);                                                                         \
        }                                                                                   \
    } else {                                                                                \
        /* inside the block */                                                              \
        i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address - (dyn->native_size);     \
        B##YES##_safe(x1, i32);                                                             \
    }

            GOCOND(0x80, "J", "Id");
#undef GO

#define GO(GETFLAGS, NO, YES, F)                                                             \
    READFLAGS(F);                                                                            \
    GETFLAGS;                                                                                \
    nextop = F8;                                                                             \
    S##YES(x3, x1);                                                                          \
    if (MODREG) {                                                                            \
        if (rex.rex) {                                                                       \
            eb1 = xRAX + (nextop & 7) + (rex.b << 3);                                        \
            eb2 = 0;                                                                         \
        } else {                                                                             \
            ed = (nextop & 7);                                                               \
            eb2 = (ed >> 2) * 8;                                                             \
            eb1 = xRAX + (ed & 3);                                                           \
        }                                                                                    \
        if (eb2) {                                                                           \
            LUI(x1, 0xffff0);                                                                \
            ORI(x1, x1, 0xff);                                                               \
            AND(eb1, eb1, x1);                                                               \
            SLLI(x3, x3, 8);                                                                 \
        } else {                                                                             \
            ANDI(eb1, eb1, 0xf00);                                                           \
        }                                                                                    \
        OR(eb1, eb1, x3);                                                                    \
    } else {                                                                                 \
        addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0); \
        SB(x3, ed, fixedaddress);                                                            \
        SMWRITE();                                                                           \
    }

            GOCOND(0x90, "SET", "Eb");
#undef GO

        case 0xA2:
            INST_NAME("CPUID");
            NOTEST(x1);
            MV(A1, xRAX);
            CALL_(my_cpuid, -1, 0);
            // BX and DX are not synchronized durring the call, so need to force the update
            LD(xRDX, xEmu, offsetof(x64emu_t, regs[_DX]));
            LD(xRBX, xEmu, offsetof(x64emu_t, regs[_BX]));
            break;
        case 0xA3:
            INST_NAME("BT Ed, Gd");
            SETFLAGS(X_CF, SF_SUBSET);
            SET_DFNONE();
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = xRAX + (nextop & 7) + (rex.b << 3);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x1, &fixedaddress, rex, NULL, 1, 0);
                SRAIxw(x1, gd, 5 + rex.w); // r1 = (gd>>5)
                ADDSL(x3, wback, x1, 2 + rex.w, x1);
                LDxw(x1, x3, fixedaddress);
                ed = x1;
            }
            BEXT(x4, ed, gd, x2);
            ANDI(xFlags, xFlags, ~1); // F_CF is 1
            OR(xFlags, xFlags, x4);
            break;
        case 0xA4:
            nextop = F8;
            INST_NAME("SHLD Ed, Gd, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            GETED(1);
            GETGD;
            u8 = F8;
            emit_shld32c(dyn, ninst, rex, ed, gd, u8, x3, x4);
            WBACK;
            break;
        case 0xA5:
            nextop = F8;
            INST_NAME("SHLD Ed, Gd, CL");
            SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
            if(box64_dynarec_safeflags>1)
                MAYSETFLAGS();
            GETGD;
            GETED(0);
            if(!rex.w && !rex.is32bits && MODREG) { ZEROUP(ed); }
            ANDI(x3, xRCX, rex.w ? 0x3f : 0x1f);
            BEQ_NEXT(x3, xZR);
            emit_shld32(dyn, ninst, rex, ed, gd, x3, x4, x5, x6);
            WBACK;
            break;
        case 0xAB:
            INST_NAME("BTS Ed, Gd");
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
            } else if(!rex.w) {
                ZEROUP(ed);
            }
            break;
        case 0xAC:
            nextop = F8;
            INST_NAME("SHRD Ed, Gd, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            GETED(1);
            GETGD;
            u8 = F8;
            u8 &= (rex.w ? 0x3f : 0x1f);
            emit_shrd32c(dyn, ninst, rex, ed, gd, u8, x3, x4);
            WBACK;
            break;
        case 0xAD:
            nextop = F8;
            INST_NAME("SHRD Ed, Gd, CL");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            if (box64_dynarec_safeflags > 1)
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
            if ((nextop & 0xF8) == 0xE8) {
                INST_NAME("LFENCE");
                SMDMB();
            } else if ((nextop & 0xF8) == 0xF0) {
                INST_NAME("MFENCE");
                SMDMB();
            } else if ((nextop & 0xF8) == 0xF8) {
                INST_NAME("SFENCE");
                SMDMB();
            } else {
                switch ((nextop >> 3) & 7) {
                    case 0:
                        INST_NAME("FXSAVE Ed");
                        MESSAGE(LOG_DUMP, "Need Optimization\n");
                        SKIPTEST(x1);
                        fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
                        if (MODREG) {
                            DEFAULT;
                        } else {
                            addr = geted(dyn, addr, ninst, nextop, &ed, x1, x3, &fixedaddress, rex, NULL, 0, 0);
                            if (ed != x1) { MV(x1, ed); }
                            CALL(rex.w ? ((void*)fpu_fxsave64) : ((void*)fpu_fxsave32), -1);
                        }
                        break;
                    case 1:
                        INST_NAME("FXRSTOR Ed");
                        MESSAGE(LOG_DUMP, "Need Optimization\n");
                        SKIPTEST(x1);
                        fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
                        if (MODREG) {
                            DEFAULT;
                        } else {
                            addr = geted(dyn, addr, ninst, nextop, &ed, x1, x3, &fixedaddress, rex, NULL, 0, 0);
                            if (ed != x1) { MV(x1, ed); }
                            CALL(rex.w ? ((void*)fpu_fxrstor64) : ((void*)fpu_fxrstor32), -1);
                        }
                        break;
                    case 2:
                        INST_NAME("LDMXCSR Md");
                        GETED(0);
                        SW(ed, xEmu, offsetof(x64emu_t, mxcsr));
                        if (box64_sse_flushto0) {
                            // TODO: applyFlushTo0 also needs to add RISC-V support.
                        }
                        break;
                    case 3:
                        INST_NAME("STMXCSR Md");
                        addr = geted(dyn, addr, ninst, nextop, &wback, x1, x2, &fixedaddress, rex, NULL, 0, 0);
                        LWU(x4, xEmu, offsetof(x64emu_t, mxcsr));
                        SW(x4, wback, fixedaddress);
                        break;
                    case 7:
                        INST_NAME("CLFLUSH Ed");
                        MESSAGE(LOG_DUMP, "Need Optimization?\n");
                        addr = geted(dyn, addr, ninst, nextop, &wback, x1, x2, &fixedaddress, rex, NULL, 0, 0);
                        if (wback != A1) {
                            MV(A1, wback);
                        }
                        CALL_(native_clflush, -1, 0);
                        break;
                    default:
                        DEFAULT;
                }
            }
            break;
        case 0xAF:
            // TODO: Refine this
            INST_NAME("IMUL Gd, Ed");
            SETFLAGS(X_ALL, SF_PENDING);
            nextop = F8;
            GETGD;
            GETED(0);
            if (rex.w) {
                // 64bits imul
                UFLAG_IF
                {
                    MULH(x3, gd, ed);
                    MUL(gd, gd, ed);
                    UFLAG_OP1(x3);
                    UFLAG_RES(gd);
                    UFLAG_DF(x3, d_imul64);
                }
                else
                {
                    MULxw(gd, gd, ed);
                }
            } else {
                // 32bits imul
                UFLAG_IF
                {
                    SLLI(gd, gd, 32);
                    SRAI(gd, gd, 32);
                    if(MODREG) {
                        SLLI(x1, ed, 32);
                        ed = x1;
                    } else {
                        SLLI(ed, ed, 32);
                    }
                    SRAI(ed, ed, 32);
                    MUL(gd, gd, ed);
                    UFLAG_RES(gd);
                    SRLI(x3, gd, 32);
                    UFLAG_OP1(x3);
                    UFLAG_DF(x3, d_imul32);
                }
                else
                {
                    MULxw(gd, gd, ed);
                }
                SLLI(gd, gd, 32);
                SRLI(gd, gd, 32);
            }
            break;
        case 0xB3:
            INST_NAME("BTR Ed, Gd");
            SETFLAGS(X_CF, SF_SUBSET);
            SET_DFNONE();
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = xRAX + (nextop & 7) + (rex.b << 3);
                wback = 0;
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                SRAIxw(x1, gd, 5 + rex.w);
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
            } else if(!rex.w) {
                ZEROUP(ed);
            }
            break;
        case 0xB6:
            INST_NAME("MOVZX Gd, Eb");
            nextop = F8;
            GETGD;
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
                ed = xRAX + (nextop & 7) + (rex.b << 3);
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
                    SETFLAGS(X_CF, SF_SUBSET);
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
                    SETFLAGS(X_CF, SF_SUBSET);
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
                    } else if(!rex.w) {
                        ZEROUP(ed);
                    }
                    MARK;
                    break;
                case 6:
                    INST_NAME("BTR Ed, Ib");
                    SETFLAGS(X_CF, SF_SUBSET);
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
                    if (wback) {
                        SDxw(ed, wback, fixedaddress);
                        SMWRITE();
                    } else if(!rex.w) {
                        ZEROUP(ed);
                    }
                    MARK;
                    break;
                case 7:
                    INST_NAME("BTC Ed, Ib");
                    SETFLAGS(X_CF, SF_SUBSET);
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
                    } else if(!rex.w) {
                        ZEROUP(ed);
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xBB:
            INST_NAME("BTC Ed, Gd");
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
            } else if(!rex.w) {
                ZEROUP(ed);
            }
            break;
        case 0xBC:
            INST_NAME("BSF Gd, Ed");
            SETFLAGS(X_ZF, SF_SUBSET);
            SET_DFNONE();
            nextop = F8;
            GETED(0);
            GETGD;
            if (!rex.w && MODREG) {
                AND(x4, ed, xMASK);
                ed = x4;
            }
            BNE_MARK(ed, xZR);
            ORI(xFlags, xFlags, 1 << F_ZF);
            B_NEXT_nocond;
            MARK;
            // gd is undefined if ed is all zeros, don't worry.
            CTZxw(gd, ed, rex.w, x1, x2);
            ANDI(xFlags, xFlags, ~(1 << F_ZF));
            break;
        case 0xBD:
            INST_NAME("BSR Gd, Ed");
            SETFLAGS(X_ZF, SF_SUBSET);
            SET_DFNONE();
            nextop = F8;
            GETED(0);
            GETGD;
            if (!rex.w && MODREG) {
                AND(x4, ed, xMASK);
                ed = x4;
            }
            BNE_MARK(ed, xZR);
            ORI(xFlags, xFlags, 1 << F_ZF);
            B_NEXT_nocond;
            MARK;
            ANDI(xFlags, xFlags, ~(1 << F_ZF));
            CLZxw(gd, ed, rex.w, x1, x2, x3);
            ADDI(x1, xZR, rex.w ? 63 : 31);
            SUB(gd, x1, gd);
            break;
        case 0xBE:
            INST_NAME("MOVSX Gd, Eb");
            nextop = F8;
            GETGD;
            if (MODREG) {
                if (rex.rex) {
                    wback = xRAX + (nextop & 7) + (rex.b << 3);
                    wb2 = 0;
                } else {
                    wback = (nextop & 7);
                    wb2 = (wback >> 2) * 8;
                    wback = xRAX + (wback & 3);
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
                ed = xRAX + (nextop & 7) + (rex.b << 3);
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
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB(x1, 0);
            GETGB(x2);
            MV(x9, ed);
            emit_add8(dyn, ninst, ed, gd, x4, x5);
            MV(gd, x9);
            EBBACK(x5, 0);
            GBBACK(x5);
            break;
        case 0xC1:
            INST_NAME("XADD Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED(0);
            if(ed!=gd)
                MV(x9, ed);
            emit_add32(dyn, ninst, rex, ed, gd, x4, x5, x6);
            if(ed!=gd)
                MVxw(gd, x9);
            WBACK;
            break;
        case 0xC2:
            INST_NAME("CMPPS Gx, Ex, Ib");
            nextop = F8;
            GETGX();
            GETEX(x2, 1);
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
                MVxw(xRAX + (nextop & 7) + (rex.b << 3), gd);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                SDxw(gd, ed, fixedaddress);
            }
            break;
        case 0xC6: // TODO: Optimize this!
            INST_NAME("SHUFPS Gx, Ex, Ib");
            nextop = F8;
            GETGX();
            GETEX(x2, 1);
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

        case 0xC8:
        case 0xC9:
        case 0xCA:
        case 0xCB:
        case 0xCC:
        case 0xCD:
        case 0xCE:
        case 0xCF:
            INST_NAME("BSWAP Reg");
            gd = xRAX + (opcode & 7) + (rex.b << 3);
            REV8xw(gd, gd, x1, x2, x3, x4);
            break;
        case 0xD5:
            INST_NAME("PMULLW Gm, Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0);
            MMX_LOOP_WS(x3, x4, MULW(x3, x3, x4));
            break;
        case 0xD9:
            INST_NAME("PSUBUSW Gm, Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0);
            MMX_LOOP_W(x3, x4, SUB(x3, x3, x4); SLT(x4, xZR, x3); NEG(x4, x4); AND(x3, x3, x4));
            break;
        case 0xDB:
            INST_NAME("PAND Gm, Em");
            nextop = F8;
            GETGM();
            GETEM(x4, 0);
            LD(x1, wback, fixedaddress);
            LD(x2, gback, gdoffset);
            AND(x1, x1, x2);
            SD(x1, gback, gdoffset);
            break;
        case 0xE2:
            INST_NAME("PSRAD Gm, Em");
            nextop = F8;
            GETGM();
            GETEM(x4, 0);
            LBU(x1, wback, fixedaddress);
            ADDI(x2, xZR, 31);
            if (rv64_zbb) {
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
        case 0xE5:
            INST_NAME("PMULHW Gm,Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0);
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
        case 0xE9:
            INST_NAME("PSUBSW Gm,Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0);
            for (int i = 0; i < 4; ++i) {
                // tmp32s = (int32_t)GM->sw[i] - EM->sw[i];
                // GM->sw[i] = (tmp32s>32767)?32767:((tmp32s<-32768)?-32768:tmp32s);
                LH(x3, gback, gdoffset + 2 * i);
                LH(x4, wback, fixedaddress + 2 * i);
                SUBW(x3, x3, x4);
                LUI(x4, 0xFFFF8); // -32768
                BGE(x3, x4, 12);
                SH(x4, gback, gdoffset + 2 * i);
                J(20);      // continue
                LUI(x4, 8); // 32768
                BLT(x3, x4, 8);
                ADDIW(x3, x4, -1);
                SH(x3, gback, gdoffset + 2 * i);
            }
            break;
        case 0xEB:
            INST_NAME("POR Gm, Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0);
            LD(x3, gback, gdoffset);
            LD(x4, wback, fixedaddress);
            OR(x3, x3, x4);
            SD(x3, gback, gdoffset);
            break;
        case 0xED:
            INST_NAME("PADDSW Gm,Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0);
            for (int i = 0; i < 4; ++i) {
                // tmp32s = (int32_t)GM->sw[i] + EM->sw[i];
                // GM->sw[i] = (tmp32s>32767)?32767:((tmp32s<-32768)?-32768:tmp32s);
                LH(x3, gback, gdoffset + 2 * i);
                LH(x4, wback, fixedaddress + 2 * i);
                ADDW(x3, x3, x4);
                LUI(x4, 0xFFFF8); // -32768
                BGE(x3, x4, 12);
                SH(x4, gback, gdoffset + 2 * i);
                J(20);      // continue
                LUI(x4, 8); // 32768
                BLT(x3, x4, 8);
                ADDIW(x3, x4, -1);
                SH(x3, gback, gdoffset + 2 * i);
            }
            break;
        case 0xEF:
            INST_NAME("PXOR Gm,Em");
            nextop = F8;
            GETGM();
            if (MODREG && gd == (nextop & 7)) {
                // just zero dest
                SD(xZR, gback, gdoffset + 0);
            } else {
                GETEM(x2, 0);
                LD(x3, gback, gdoffset + 0);
                LD(x4, wback, fixedaddress);
                XOR(x3, x3, x4);
                SD(x3, gback, gdoffset + 0);
            }
            break;
        case 0xF5:
            INST_NAME("PMADDWD Gm, Em");
            nextop = F8;
            GETGM();
            GETEM(x5, 0);
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
        case 0xF9:
            INST_NAME("PSUBW Gm, Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0);
            MMX_LOOP_W(x3, x4, SUBW(x3, x3, x4));
            break;
        case 0xFD:
            INST_NAME("PADDW Gm, Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0);
            MMX_LOOP_W(x3, x4, ADDW(x3, x3, x4));
            break;
        case 0xFE:
            INST_NAME("PADDD Gm, Em");
            nextop = F8;
            GETGM();
            GETEM(x2, 0);
            MMX_LOOP_D(x3, x4, ADDW(x3, x3, x4));
            break;
        default:
            DEFAULT;
    }
    return addr;
}
