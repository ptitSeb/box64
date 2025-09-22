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

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_functions.h"
#include "../dynarec_helper.h"

uintptr_t dynarec64_AVX_66_0F(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed, vd;
    uint8_t wback, wb1, wb2, gback, vback;
    uint8_t eb1, eb2, gb1, gb2;
    int32_t i32, i32_;
    int cacheupd = 0;
    int v0, v1, v2;
    int q0, q1, q2;
    int d0, d1, d2;
    int s0;
    uint64_t tmp64u, u64;
    int64_t j64;
    int64_t fixedaddress, gdoffset, vxoffset, gyoffset, vyoffset;
    int unscaled;

    rex_t rex = vex.rex;

    switch (opcode) {
        case 0x10:
            INST_NAME("VMOVUPD Gx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 24 : 8);
            GETGX();
            GETGY();
            LD(x3, wback, fixedaddress);
            LD(x4, wback, fixedaddress + 8);
            SD(x3, gback, gdoffset);
            SD(x4, gback, gdoffset + 8);
            if (vex.l) {
                GETEY();
                LD(x3, wback, fixedaddress);
                LD(x4, wback, fixedaddress + 8);
                SD(x3, gback, gyoffset);
                SD(x4, gback, gyoffset + 8);
            } else
                YMM0(gd);
            break;
        case 0x11:
            INST_NAME("VMOVUPD Ex, Gx");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 24 : 8);
            GETGX();
            GETGY();
            LD(x3, gback, gdoffset + 0);
            LD(x4, gback, gdoffset + 8);
            SD(x3, wback, fixedaddress + 0);
            SD(x4, wback, fixedaddress + 8);
            if (vex.l) {
                GETEY();
                LD(x3, gback, gyoffset + 0);
                LD(x4, gback, gyoffset + 8);
                SD(x3, wback, fixedaddress + 0);
                SD(x4, wback, fixedaddress + 8);
            } else if (MODREG)
                YMM0(ed);
            if (!MODREG) SMWRITE2();
            break;
        case 0x12:
            INST_NAME("VMOVLPD Gx, Vx, Eq");
            nextop = F8;
            if (MODREG) {
                // access register instead of memory is bad opcode!
                DEFAULT;
                return addr;
            }
            GETGX();
            GETVX();
            SMREAD();
            addr = geted(dyn, addr, ninst, nextop, &wback, x2, x3, &fixedaddress, rex, NULL, 1, 0);
            LD(x3, wback, fixedaddress);
            SD(x3, gback, gdoffset + 0);
            LD(x3, vback, vxoffset + 8);
            SD(x3, gback, gdoffset + 8);
            YMM0(gd);
            break;
        case 0x14:
            INST_NAME("VUNPCKLPD Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 16 : 0);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            LD(x3, vback, vxoffset);
            LD(x4, wback, fixedaddress);
            SD(x3, gback, gdoffset);
            SD(x4, gback, gdoffset + 8);
            if (vex.l) {
                GETEY();
                LD(x3, vback, vyoffset);
                LD(x4, wback, fixedaddress);
                SD(x3, gback, gyoffset);
                SD(x4, gback, gyoffset + 8);
            } else
                YMM0(gd);
            break;
        case 0x15:
            INST_NAME("VUNPCKHPD Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 16 : 0);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            LD(x3, vback, vxoffset + 8);
            LD(x4, wback, fixedaddress + 8);
            SD(x3, gback, gdoffset);
            SD(x4, gback, gdoffset + 8);
            if (vex.l) {
                GETEY();
                LD(x3, vback, vyoffset + 8);
                LD(x4, wback, fixedaddress + 8);
                SD(x3, gback, gyoffset);
                SD(x4, gback, gyoffset + 8);
            } else
                YMM0(gd);
            break;
        case 0x28:
            INST_NAME("VMOVAPD Gx, Ex");
            nextop = F8;
            GETEX(x2, 0, vex.l ? 24 : 8);
            GETGX();
            GETGY();
            LD(x3, wback, fixedaddress);
            SD(x3, gback, gdoffset);
            LD(x3, wback, fixedaddress + 8);
            SD(x3, gback, gdoffset + 8);
            if (vex.l) {
                GETEY();
                LD(x3, wback, fixedaddress);
                SD(x3, gback, gyoffset);
                LD(x3, wback, fixedaddress + 8);
                SD(x3, gback, gyoffset + 8);
            } else
                YMM0(gd);
            break;
        case 0x29:
            INST_NAME("VMOVAPD Ex, Gx");
            nextop = F8;
            GETEX(x2, 0, vex.l ? 24 : 8);
            GETGX();
            GETGY();
            LD(x3, gback, gdoffset + 0);
            LD(x4, gback, gdoffset + 8);
            SD(x3, wback, fixedaddress + 0);
            SD(x4, wback, fixedaddress + 8);
            if (vex.l) {
                GETEY();
                LD(x3, gback, gyoffset + 0);
                LD(x4, gback, gyoffset + 8);
                SD(x3, wback, fixedaddress + 0);
                SD(x4, wback, fixedaddress + 8);
            } else if (MODREG)
                YMM0(ed);
            if (!MODREG) SMWRITE2();
            break;
        case 0x2E:
            // no special check...
        case 0x2F:
            if (opcode == 0x2F) {
                INST_NAME("VCOMISD Gx, Ex");
            } else {
                INST_NAME("VUCOMISD Gx, Ex");
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
        case 0x50:
            INST_NAME("VMOVMSKPD Gd, Ex");
            nextop = F8;
            GETGD;
            GETEX(x1, 0, vex.l ? 24 : 8);
            XOR(gd, gd, gd);
            for (int i = 0; i < 2; ++i) {
                // GD->dword[0] |= ((EX->q[i]>>63)&1)<<i;
                LD(x2, wback, fixedaddress + 8 * i);
                SRLI(x2, x2, 63);
                if (i) SLLI(x2, x2, 1);
                OR(gd, gd, x2);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 2; ++i) {
                    LD(x2, wback, fixedaddress + 8 * i);
                    SRLI(x2, x2, 63);
                    SLLI(x2, x2, i + 2);
                    OR(gd, gd, x2);
                }
            }
            break;
        case 0x51:
            INST_NAME("VSQRTPD Gx, Ex");
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
            if (vex.l) {
                GETEY();
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
                    FSD(d0, gback, gyoffset + i * 8);
                }
            } else
                YMM0(gd);
            break;
        case 0x54:
            INST_NAME("VANDPD Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 24 : 8);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            LD(x3, wback, fixedaddress + 0);
            LD(x4, wback, fixedaddress + 8);
            LD(x5, vback, vxoffset + 0);
            LD(x6, vback, vxoffset + 8);
            AND(x5, x5, x3);
            AND(x6, x6, x4);
            SD(x5, gback, gdoffset + 0);
            SD(x6, gback, gdoffset + 8);
            if (vex.l) {
                GETEY();
                LD(x3, wback, fixedaddress + 0);
                LD(x4, wback, fixedaddress + 8);
                LD(x5, vback, vyoffset + 0);
                LD(x6, vback, vyoffset + 8);
                AND(x5, x5, x3);
                AND(x6, x6, x4);
                SD(x5, gback, gyoffset + 0);
                SD(x6, gback, gyoffset + 8);
            } else
                YMM0(gd);
            break;
        case 0x55:
            INST_NAME("VANDNPD Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 24 : 8);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            LD(x3, wback, fixedaddress + 0);
            LD(x4, wback, fixedaddress + 8);
            LD(x5, vback, vxoffset + 0);
            LD(x6, vback, vxoffset + 8);
            if (cpuext.zbb) {
                ANDN(x5, x3, x5);
                ANDN(x6, x4, x6);
            } else {
                NOT(x5, x5);
                NOT(x6, x6);
                AND(x5, x5, x3);
                AND(x6, x6, x4);
            }
            SD(x5, gback, gdoffset + 0);
            SD(x6, gback, gdoffset + 8);
            if (vex.l) {
                GETEY();
                LD(x3, wback, fixedaddress + 0);
                LD(x4, wback, fixedaddress + 8);
                LD(x5, vback, vyoffset + 0);
                LD(x6, vback, vyoffset + 8);
                if (cpuext.zbb) {
                    ANDN(x5, x3, x5);
                    ANDN(x6, x4, x6);
                } else {
                    NOT(x5, x5);
                    NOT(x6, x6);
                    AND(x5, x5, x3);
                    AND(x6, x6, x4);
                }
                SD(x5, gback, gyoffset + 0);
                SD(x6, gback, gyoffset + 8);
            } else
                YMM0(gd);
            break;
        case 0x56:
            INST_NAME("VORPD Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 24 : 8);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            LD(x3, wback, fixedaddress + 0);
            LD(x4, wback, fixedaddress + 8);
            LD(x5, vback, vxoffset + 0);
            LD(x6, vback, vxoffset + 8);
            OR(x5, x5, x3);
            OR(x6, x6, x4);
            SD(x5, gback, gdoffset + 0);
            SD(x6, gback, gdoffset + 8);
            if (vex.l) {
                GETEY();
                LD(x3, wback, fixedaddress + 0);
                LD(x4, wback, fixedaddress + 8);
                LD(x5, vback, vyoffset + 0);
                LD(x6, vback, vyoffset + 8);
                OR(x5, x5, x3);
                OR(x6, x6, x4);
                SD(x5, gback, gyoffset + 0);
                SD(x6, gback, gyoffset + 8);
            } else
                YMM0(gd);
            break;
        case 0x57:
            INST_NAME("VXORPD Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 24 : 8);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            LD(x3, wback, fixedaddress + 0);
            LD(x4, wback, fixedaddress + 8);
            LD(x5, vback, vxoffset + 0);
            LD(x6, vback, vxoffset + 8);
            XOR(x5, x5, x3);
            XOR(x6, x6, x4);
            SD(x5, gback, gdoffset + 0);
            SD(x6, gback, gdoffset + 8);
            if (vex.l) {
                GETEY();
                LD(x3, wback, fixedaddress + 0);
                LD(x4, wback, fixedaddress + 8);
                LD(x5, vback, vyoffset + 0);
                LD(x6, vback, vyoffset + 8);
                XOR(x5, x5, x3);
                XOR(x6, x6, x4);
                SD(x5, gback, gyoffset + 0);
                SD(x6, gback, gyoffset + 8);
            } else
                YMM0(gd);
            break;
        case 0x58:
            INST_NAME("VADDPD Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 24 : 8);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            v0 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            for (int i = 0; i < 2; ++i) {
                FLD(v0, wback, fixedaddress + 8 * i);
                FLD(v1, vback, vxoffset + 8 * i);
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
                FSD(v0, gback, gdoffset + 8 * i);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 2; ++i) {
                    FLD(v0, wback, fixedaddress + 8 * i);
                    FLD(v1, vback, vyoffset + 8 * i);
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
                    FSD(v0, gback, gyoffset + 8 * i);
                }
            } else
                YMM0(gd);
            break;
        case 0x59:
            INST_NAME("VMULPD Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 24 : 8);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            v0 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            for (int i = 0; i < 2; ++i) {
                FLD(v0, wback, fixedaddress + 8 * i);
                FLD(v1, vback, vxoffset + 8 * i);
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
                FSD(v0, gback, gdoffset + 8 * i);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 2; ++i) {
                    FLD(v0, wback, fixedaddress + 8 * i);
                    FLD(v1, vback, vyoffset + 8 * i);
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
                    FSD(v0, gback, gyoffset + 8 * i);
                }
            } else
                YMM0(gd);
            break;
        case 0x5A:
            INST_NAME("VCVTPD2PS Gx, Ex");
            nextop = F8;
            GETEX(x2, 0, vex.l ? 24 : 8);
            GETGX();
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            FLD(d0, wback, fixedaddress + 0);
            FLD(d1, wback, fixedaddress + 8);
            FCVTSD(d0, d0);
            FCVTSD(d1, d1);
            FSW(d0, gback, gdoffset + 0);
            FSW(d1, gback, gdoffset + 4);
            if (vex.l) {
                GETEY();
                FLD(d0, wback, fixedaddress + 0);
                FLD(d1, wback, fixedaddress + 8);
                FCVTSD(d0, d0);
                FCVTSD(d1, d1);
                FSW(d0, gback, gdoffset + 8);
                FSW(d1, gback, gdoffset + 12);
            } else
                SD(xZR, gback, gdoffset + 8);
            YMM0(gd);
            break;
        case 0x5B:
            INST_NAME("VCVTPS2DQ Gx, Ex");
            nextop = F8;
            GETEX(x2, 0, vex.l ? 28 : 12);
            GETGX();
            GETGY();
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
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 4; ++i) {
                    FLW(d0, wback, fixedaddress + 4 * i);
                    FCVTLS(x3, d0, RD_DYN);
                    SEXT_W(x5, x3);
                    BEQ(x5, x3, 8);
                    LUI(x3, 0x80000); // INT32_MIN
                    SW(x3, gback, gyoffset + 4 * i);
                }
            } else
                YMM0(gd);
            x87_restoreround(dyn, ninst, u8);
            break;
        case 0x5C:
            INST_NAME("VSUBPD Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 24 : 8);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            v0 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            for (int i = 0; i < 2; ++i) {
                FLD(v0, wback, fixedaddress + 8 * i);
                FLD(v1, vback, vxoffset + 8 * i);
                if (!BOX64ENV(dynarec_fastnan)) {
                    FEQD(x3, v0, v0);
                    FEQD(x4, v1, v1);
                }
                FSUBD(v0, v1, v0);
                if (!BOX64ENV(dynarec_fastnan)) {
                    AND(x3, x3, x4);
                    BEQZ(x3, 16);
                    FEQD(x3, v0, v0);
                    BNEZ(x3, 8);
                    FNEGD(v0, v0);
                }
                FSD(v0, gback, gdoffset + 8 * i);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 2; ++i) {
                    FLD(v0, wback, fixedaddress + 8 * i);
                    FLD(v1, vback, vyoffset + 8 * i);
                    if (!BOX64ENV(dynarec_fastnan)) {
                        FEQD(x3, v0, v0);
                        FEQD(x4, v1, v1);
                    }
                    FSUBD(v0, v1, v0);
                    if (!BOX64ENV(dynarec_fastnan)) {
                        AND(x3, x3, x4);
                        BEQZ(x3, 16);
                        FEQD(x3, v0, v0);
                        BNEZ(x3, 8);
                        FNEGD(v0, v0);
                    }
                    FSD(v0, gback, gyoffset + 8 * i);
                }
            } else
                YMM0(gd);
            break;
        case 0x5D:
            INST_NAME("VMINPD Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 24 : 8);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            v0 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            for (int i = 0; i < 2; ++i) {
                FLD(v0, vback, vxoffset + 8 * i);
                FLD(v1, wback, fixedaddress + 8 * i);
                FEQD(x3, v0, v0);
                FEQD(x4, v1, v1);
                AND(x3, x3, x4);
                BEQ(x3, xZR, 4 + 3 * 4);
                FLTD(x3, v0, v1);
                BEQ(x3, xZR, 4 + 4); // continue
                FMVD(v1, v0);
                FSD(v1, gback, gdoffset + 8 * i);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 2; ++i) {
                    FLD(v0, vback, vyoffset + 8 * i);
                    FLD(v1, wback, fixedaddress + 8 * i);
                    FEQD(x3, v0, v0);
                    FEQD(x4, v1, v1);
                    AND(x3, x3, x4);
                    BEQ(x3, xZR, 4 + 3 * 4);
                    FLTD(x3, v0, v1);
                    BEQ(x3, xZR, 4 + 4); // continue
                    FMVD(v1, v0);
                    FSD(v1, gback, gyoffset + 8 * i);
                }
            } else
                YMM0(gd);
            break;
        case 0x5E:
            INST_NAME("VDIVPD Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 24 : 8);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            v0 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            for (int i = 0; i < 2; ++i) {
                FLD(v0, wback, fixedaddress + 8 * i);
                FLD(v1, vback, vxoffset + 8 * i);
                if (!BOX64ENV(dynarec_fastnan)) {
                    FEQD(x3, v0, v0);
                    FEQD(x4, v1, v1);
                }
                FDIVD(v0, v1, v0);
                if (!BOX64ENV(dynarec_fastnan)) {
                    AND(x3, x3, x4);
                    BEQZ(x3, 16);
                    FEQD(x3, v0, v0);
                    BNEZ(x3, 8);
                    FNEGD(v0, v0);
                }
                FSD(v0, gback, gdoffset + 8 * i);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 2; ++i) {
                    FLD(v0, wback, fixedaddress + 8 * i);
                    FLD(v1, vback, vyoffset + 8 * i);
                    if (!BOX64ENV(dynarec_fastnan)) {
                        FEQD(x3, v0, v0);
                        FEQD(x4, v1, v1);
                    }
                    FDIVD(v0, v1, v0);
                    if (!BOX64ENV(dynarec_fastnan)) {
                        AND(x3, x3, x4);
                        BEQZ(x3, 16);
                        FEQD(x3, v0, v0);
                        BNEZ(x3, 8);
                        FNEGD(v0, v0);
                    }
                    FSD(v0, gback, gyoffset + 8 * i);
                }
            } else
                YMM0(gd);
            break;
        case 0x5F:
            INST_NAME("VMAXPD Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 24 : 8);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            v0 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            for (int i = 0; i < 2; ++i) {
                FLD(v0, vback, vxoffset + 8 * i);
                FLD(v1, wback, fixedaddress + 8 * i);
                FEQD(x3, v0, v0);
                FEQD(x4, v1, v1);
                AND(x3, x3, x4);
                BEQ(x3, xZR, 4 + 3 * 4);
                FLTD(x3, v1, v0);
                BEQ(x3, xZR, 4 + 4); // continue
                FMVD(v1, v0);
                FSD(v1, gback, gdoffset + 8 * i);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 2; ++i) {
                    FLD(v0, vback, vyoffset + 8 * i);
                    FLD(v1, wback, fixedaddress + 8 * i);
                    FEQD(x3, v0, v0);
                    FEQD(x4, v1, v1);
                    AND(x3, x3, x4);
                    BEQ(x3, xZR, 4 + 3 * 4);
                    FLTD(x3, v1, v0);
                    BEQ(x3, xZR, 4 + 4); // continue
                    FMVD(v1, v0);
                    FSD(v1, gback, gyoffset + 8 * i);
                }
            } else
                YMM0(gd);
            break;
        case 0x60:
            INST_NAME("VPUNPCKLBW Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 23 : 7);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            for (int i = 7; i >= 0; --i) {
                LBU(x3, wback, fixedaddress + i);
                LBU(x4, vback, vxoffset + i);
                SB(x3, gback, gdoffset + 2 * i + 1);
                SB(x4, gback, gdoffset + 2 * i);
            }
            if (vex.l) {
                GETEY();
                for (int i = 7; i >= 0; --i) {
                    LBU(x3, wback, fixedaddress + i);
                    LBU(x4, vback, vyoffset + i);
                    SB(x3, gback, gyoffset + 2 * i + 1);
                    SB(x4, gback, gyoffset + 2 * i);
                }
            } else
                YMM0(gd);
            break;
        case 0x61:
            INST_NAME("VPUNPCKLWD Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 22 : 6);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            for (int i = 3; i >= 0; --i) {
                LHU(x3, wback, fixedaddress + i * 2);
                LHU(x4, vback, vxoffset + i * 2);
                SH(x3, gback, gdoffset + 4 * i + 2);
                SH(x4, gback, gdoffset + 4 * i);
            }
            if (vex.l) {
                GETEY();
                for (int i = 3; i >= 0; --i) {
                    LHU(x3, wback, fixedaddress + i * 2);
                    LHU(x4, vback, vyoffset + i * 2);
                    SH(x3, gback, gyoffset + 4 * i + 2);
                    SH(x4, gback, gyoffset + 4 * i);
                }
            } else
                YMM0(gd);
            break;
        case 0x62:
            INST_NAME("VPUNPCKLDQ Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 20 : 4);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            for (int i = 1; i >= 0; --i) {
                LWU(x3, wback, fixedaddress + i * 4);
                LWU(x4, vback, vxoffset + i * 4);
                SW(x3, gback, gdoffset + 8 * i + 4);
                SW(x4, gback, gdoffset + 8 * i);
            }
            if (vex.l) {
                GETEY();
                for (int i = 1; i >= 0; --i) {
                    LWU(x3, wback, fixedaddress + i * 4);
                    LWU(x4, vback, vyoffset + i * 4);
                    SW(x3, gback, gyoffset + 8 * i + 4);
                    SW(x4, gback, gyoffset + 8 * i);
                }
            } else
                YMM0(gd);
            break;
        case 0x63:
        case 0x67:
            if (opcode == 0x63)
                INST_NAME("VPACKSSWB Gx, Vx, Ex");
            else
                INST_NAME("VPACKUSWB Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 30 : 14);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            if (opcode == 0x63) {
                ADDIW(x6, xZR, 0xF80); // -128
                ADDIW(x7, xZR, 0x80);  // 128
            } else {
                ADDIW(x6, xZR, 0x100); // 256
            }
            if (gd == ed) {
                ADDI(x5, xEmu, offsetof(x64emu_t, scratch));
                LD(x3, wback, fixedaddress + 0);
                LD(x4, wback, fixedaddress + 8);
                SD(x3, x5, 0);
                SD(x4, x5, 8);
                wback = x5;
                fixedaddress = 0;
            }
            for (int i = 0; i < 8; ++i) {
                LH(x3, vback, vxoffset + i * 2);
                if (opcode == 0x63)
                    SATw(x3, x6, x7);
                else
                    SATUw(x3, x6);
                SB(x3, gback, gdoffset + i);
            }
            if (vex.v == ed) {
                LD(x3, gback, gdoffset + 0);
                SD(x3, gback, gdoffset + 8);
            } else {
                for (int i = 0; i < 8; ++i) {
                    LH(x3, wback, fixedaddress + i * 2);
                    if (opcode == 0x63)
                        SATw(x3, x6, x7);
                    else
                        SATUw(x3, x6);
                    SB(x3, gback, gdoffset + 8 + i);
                }
            }
            if (vex.l) {
                GETEY();
                if (gd == ed) {
                    ADDI(x5, xEmu, offsetof(x64emu_t, scratch));
                    LD(x3, wback, fixedaddress + 0);
                    LD(x4, wback, fixedaddress + 8);
                    SD(x3, x5, 0);
                    SD(x4, x5, 8);
                    wback = x5;
                    fixedaddress = 0;
                }
                for (int i = 0; i < 8; ++i) {
                    LH(x3, vback, vyoffset + i * 2);
                    if (opcode == 0x63)
                        SATw(x3, x6, x7);
                    else
                        SATUw(x3, x6);
                    SB(x3, gback, gyoffset + i);
                }
                if (vex.v == ed) {
                    LD(x3, gback, gyoffset + 0);
                    SD(x3, gback, gyoffset + 8);
                } else {
                    for (int i = 0; i < 8; ++i) {
                        LH(x3, wback, fixedaddress + i * 2);
                        if (opcode == 0x63)
                            SATw(x3, x6, x7);
                        else
                            SATUw(x3, x6);
                        SB(x3, gback, gyoffset + 8 + i);
                    }
                }
            } else
                YMM0(gd);
            break;
        case 0x64:
            INST_NAME("VPCMPGTB Gx, Vx, Ex");
            nextop = F8;
            GETEX(x2, 0, vex.l ? 31 : 15);
            GETGX();
            GETVX();
            GETGY();
            GETVY();
            for (int i = 0; i < 16; ++i) {
                LB(x3, vback, vxoffset + i);
                LB(x4, wback, fixedaddress + i);
                SLT(x4, x4, x3);
                NEG(x3, x4);
                SB(x3, gback, gdoffset + i);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 16; ++i) {
                    LB(x3, vback, vyoffset + i);
                    LB(x4, wback, fixedaddress + i);
                    SLT(x4, x4, x3);
                    NEG(x3, x4);
                    SB(x3, gback, gyoffset + i);
                }
            } else
                YMM0(gd);
            break;
        case 0x65:
            INST_NAME("VPCMPGTW Gx, Vx, Ex");
            nextop = F8;
            GETEX(x2, 0, vex.l ? 30 : 14);
            GETGX();
            GETVX();
            GETGY();
            GETVY();
            for (int i = 0; i < 8; ++i) {
                LH(x3, vback, vxoffset + i * 2);
                LH(x4, wback, fixedaddress + i * 2);
                SLT(x4, x4, x3);
                NEG(x3, x4);
                SH(x3, gback, gdoffset + i * 2);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 8; ++i) {
                    LH(x3, vback, vyoffset + i * 2);
                    LH(x4, wback, fixedaddress + i * 2);
                    SLT(x4, x4, x3);
                    NEG(x3, x4);
                    SH(x3, gback, gyoffset + i * 2);
                }
            } else
                YMM0(gd);
            break;
        case 0x66:
            INST_NAME("VPCMPGTD Gx, Vx, Ex");
            nextop = F8;
            GETEX(x2, 0, vex.l ? 28 : 12);
            GETGX();
            GETVX();
            GETGY();
            GETVY();
            for (int i = 0; i < 4; ++i) {
                LW(x3, vback, vxoffset + i * 4);
                LW(x4, wback, fixedaddress + i * 4);
                SLT(x4, x4, x3);
                NEG(x3, x4);
                SW(x3, gback, gdoffset + i * 4);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 4; ++i) {
                    LW(x3, vback, vyoffset + i * 4);
                    LW(x4, wback, fixedaddress + i * 4);
                    SLT(x4, x4, x3);
                    NEG(x3, x4);
                    SW(x3, gback, gyoffset + i * 4);
                }
            } else
                YMM0(gd);
            break;
        case 0x68:
            INST_NAME("VPUNPCKHBW Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 31 : 15);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            for (int i = 0; i < 8; ++i) {
                LBU(x3, vback, vxoffset + i + 8);
                LBU(x4, wback, fixedaddress + i + 8);
                SB(x3, gback, gdoffset + i * 2);
                SB(x4, gback, gdoffset + i * 2 + 1);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 8; ++i) {
                    LBU(x3, vback, vyoffset + i + 8);
                    LBU(x4, wback, fixedaddress + i + 8);
                    SB(x3, gback, gyoffset + i * 2);
                    SB(x4, gback, gyoffset + i * 2 + 1);
                }
            } else
                YMM0(gd);
            break;
        case 0x69:
            INST_NAME("VPUNPCKHWD Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 30 : 14);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            for (int i = 0; i < 4; ++i) {
                LHU(x3, vback, vxoffset + i * 2 + 8);
                LHU(x4, wback, fixedaddress + i * 2 + 8);
                SH(x3, gback, gdoffset + i * 4);
                SH(x4, gback, gdoffset + i * 4 + 2);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 4; ++i) {
                    LHU(x3, vback, vyoffset + i * 2 + 8);
                    LHU(x4, wback, fixedaddress + i * 2 + 8);
                    SH(x3, gback, gyoffset + i * 4);
                    SH(x4, gback, gyoffset + i * 4 + 2);
                }
            } else
                YMM0(gd);
            break;
        case 0x6A:
            INST_NAME("VPUNPCKHDQ Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 28 : 12);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            for (int i = 0; i < 2; ++i) {
                LWU(x3, vback, vxoffset + i * 4 + 8);
                LWU(x4, wback, fixedaddress + i * 4 + 8);
                SW(x3, gback, gdoffset + i * 8);
                SW(x4, gback, gdoffset + i * 8 + 4);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 2; ++i) {
                    LWU(x3, vback, vyoffset + i * 4 + 8);
                    LWU(x4, wback, fixedaddress + i * 4 + 8);
                    SW(x3, gback, gyoffset + i * 8);
                    SW(x4, gback, gyoffset + i * 8 + 4);
                }
            } else
                YMM0(gd);
            break;
        case 0x6B:
            INST_NAME("VPACKSSDW Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 28 : 12);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            LUI(x6, 0xFFFF8); // -32768
            LUI(x7, 0x8);     // 32768
            if (gd == ed) {
                ADDI(x5, xEmu, offsetof(x64emu_t, scratch));
                LD(x3, wback, fixedaddress + 0);
                LD(x4, wback, fixedaddress + 8);
                SD(x3, x5, 0);
                SD(x4, x5, 8);
                wback = x5;
                fixedaddress = 0;
            }
            for (int i = 0; i < 4; ++i) {
                LW(x3, vback, vxoffset + i * 4);
                SATw(x3, x6, x7);
                SH(x3, gback, gdoffset + i * 2);
            }
            if (vex.v == ed) {
                LD(x3, gback, gdoffset + 0);
                SD(x3, gback, gdoffset + 8);
            } else {
                for (int i = 0; i < 4; ++i) {
                    LW(x3, wback, fixedaddress + i * 4);
                    SATw(x3, x6, x7);
                    SH(x3, gback, gdoffset + (4 + i) * 2);
                }
            }
            if (vex.l) {
                GETEY();
                if (gd == ed) {
                    ADDI(x5, xEmu, offsetof(x64emu_t, scratch));
                    LD(x3, wback, fixedaddress + 0);
                    LD(x4, wback, fixedaddress + 8);
                    SD(x3, x5, 0);
                    SD(x4, x5, 8);
                    wback = x5;
                    fixedaddress = 0;
                }
                for (int i = 0; i < 4; ++i) {
                    LW(x3, vback, vyoffset + i * 4);
                    SATw(x3, x6, x7);
                    SH(x3, gback, gyoffset + i * 2);
                }
                if (vex.v == ed) {
                    LD(x3, gback, gyoffset + 0);
                    SD(x3, gback, gyoffset + 8);
                } else {
                    for (int i = 0; i < 4; ++i) {
                        LW(x3, wback, fixedaddress + i * 4);
                        SATw(x3, x6, x7);
                        SH(x3, gback, gyoffset + (4 + i) * 2);
                    }
                }
            } else
                YMM0(gd);
            break;
        case 0x6C:
            INST_NAME("VPUNPCKLQDQ Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 16 : 1);
            GETGX();
            GETVX();
            GETGY();
            GETVY();
            LD(x3, vback, vxoffset + 0);
            LD(x4, wback, fixedaddress + 0);
            SD(x3, gback, gdoffset + 0);
            SD(x4, gback, gdoffset + 8);
            if (vex.l) {
                GETEY();
                LD(x3, vback, vyoffset + 0);
                LD(x4, wback, fixedaddress + 0);
                SD(x3, gback, gyoffset + 0);
                SD(x4, gback, gyoffset + 8);
            } else
                YMM0(gd);
            break;
        case 0x6D:
            INST_NAME("VPUNPCKHQDQ Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 24 : 8);
            GETGX();
            GETVX();
            GETGY();
            GETVY();
            LD(x3, vback, vxoffset + 8);
            LD(x4, wback, fixedaddress + 8);
            SD(x3, gback, gdoffset + 0);
            SD(x4, gback, gdoffset + 8);
            if (vex.l) {
                GETEY();
                LD(x3, vback, vyoffset + 8);
                LD(x4, wback, fixedaddress + 8);
                SD(x3, gback, gyoffset + 0);
                SD(x4, gback, gyoffset + 8);
            } else
                YMM0(gd);
            break;
        case 0x6E:
            INST_NAME("VMOVD Gx, Ed");
            nextop = F8;
            GETED(0);
            GETGX();
            GETGY();
            if (MODREG && !rex.w) {
                ZEXTW2(x3, ed);
                ed = x3;
            }
            SD(ed, gback, gdoffset);
            SD(xZR, gback, gdoffset + 8);
            YMM0(gd);
            break;
        case 0x6F:
            INST_NAME("VMOVDQA Gx, Ex");
            nextop = F8;
            GETEX(x2, 0, vex.l ? 24 : 8);
            GETGX();
            GETGY();
            LD(x3, wback, fixedaddress);
            SD(x3, gback, gdoffset);
            LD(x3, wback, fixedaddress + 8);
            SD(x3, gback, gdoffset + 8);
            if (vex.l) {
                GETEY();
                LD(x3, wback, fixedaddress);
                SD(x3, gback, gyoffset);
                LD(x3, wback, fixedaddress + 8);
                SD(x3, gback, gyoffset + 8);
            } else
                YMM0(gd);
            break;
        case 0x70:
            INST_NAME("VPSHUFD Gx, Ex, Ib");
            nextop = F8;
            GETEX(x2, 1, vex.l ? 28 : 12);
            GETGX();
            GETGY();
            u8 = F8;

            LWU(x3, wback, fixedaddress + ((u8 >> (0 * 2)) & 3) * 4);
            LWU(x4, wback, fixedaddress + ((u8 >> (1 * 2)) & 3) * 4);
            LWU(x5, wback, fixedaddress + ((u8 >> (2 * 2)) & 3) * 4);
            LWU(x6, wback, fixedaddress + ((u8 >> (3 * 2)) & 3) * 4);
            SW(x3, gback, gdoffset + 0 * 4);
            SW(x4, gback, gdoffset + 1 * 4);
            SW(x5, gback, gdoffset + 2 * 4);
            SW(x6, gback, gdoffset + 3 * 4);
            if (vex.l) {
                GETEY();
                LWU(x3, wback, fixedaddress + ((u8 >> (0 * 2)) & 3) * 4);
                LWU(x4, wback, fixedaddress + ((u8 >> (1 * 2)) & 3) * 4);
                LWU(x5, wback, fixedaddress + ((u8 >> (2 * 2)) & 3) * 4);
                LWU(x6, wback, fixedaddress + ((u8 >> (3 * 2)) & 3) * 4);
                SW(x3, gback, gyoffset + 0 * 4);
                SW(x4, gback, gyoffset + 1 * 4);
                SW(x5, gback, gyoffset + 2 * 4);
                SW(x6, gback, gyoffset + 3 * 4);
            } else
                YMM0(gd);
            break;
        case 0x74:
            INST_NAME("VPCMPEQB Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 31 : 15);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            for (int i = 0; i < 16; ++i) {
                LBU(x3, vback, vxoffset + i);
                LBU(x4, wback, fixedaddress + i);
                if (cpuext.xtheadbb) {
                    XOR(x3, x3, x4);
                    TH_TSTNBZ(x3, x3);
                } else {
                    SUB(x3, x3, x4);
                    SEQZ(x3, x3);
                    NEG(x3, x3);
                }
                SB(x3, gback, gdoffset + i);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 16; ++i) {
                    LBU(x3, vback, vyoffset + i);
                    LBU(x4, wback, fixedaddress + i);
                    if (cpuext.xtheadbb) {
                        XOR(x3, x3, x4);
                        TH_TSTNBZ(x3, x3);
                    } else {
                        SUB(x3, x3, x4);
                        SEQZ(x3, x3);
                        NEG(x3, x3);
                    }
                    SB(x3, gback, gyoffset + i);
                }
            } else
                YMM0(gd);
            break;
        case 0x75:
            INST_NAME("VPCMPEQW Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 30 : 14);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            for (int i = 0; i < 8; ++i) {
                LHU(x3, vback, vxoffset + i * 2);
                LHU(x4, wback, fixedaddress + i * 2);
                SUB(x3, x3, x4);
                SEQZ(x3, x3);
                NEG(x3, x3);
                SH(x3, gback, gdoffset + i * 2);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 8; ++i) {
                    LHU(x3, vback, vyoffset + i * 2);
                    LHU(x4, wback, fixedaddress + i * 2);
                    SUB(x3, x3, x4);
                    SEQZ(x3, x3);
                    NEG(x3, x3);
                    SH(x3, gback, gyoffset + i * 2);
                }
            } else
                YMM0(gd);
            break;
        case 0x76:
            INST_NAME("VPCMPEQD Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 28 : 12);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            for (int i = 0; i < 4; ++i) {
                LWU(x3, vback, vxoffset + i * 4);
                LWU(x4, wback, fixedaddress + i * 4);
                SUB(x3, x3, x4);
                SEQZ(x3, x3);
                NEG(x3, x3);
                SW(x3, gback, gdoffset + i * 4);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 4; ++i) {
                    LWU(x3, vback, vyoffset + i * 4);
                    LWU(x4, wback, fixedaddress + i * 4);
                    SUB(x3, x3, x4);
                    SEQZ(x3, x3);
                    NEG(x3, x3);
                    SW(x3, gback, gyoffset + i * 4);
                }
            } else
                YMM0(gd);
            break;
        case 0x7C:
            INST_NAME("VHADDPD Gx, Vx, Ex");
            nextop = F8;
            GETEX(x2, 0, vex.l ? 24 : 8);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            v0 = fpu_get_scratch(dyn);
            if (gd == ed) {
                FLD(v0, gback, gdoffset + 0);
            }
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            FLD(d0, gback, vxoffset + 0);
            FLD(d1, gback, vxoffset + 8);
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
            if (vex.v == ed) {
                FSD(d0, gback, gdoffset + 8);
            } else {
                if (gd == ed)
                    d0 = v0;
                else
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
            if (vex.l) {
                GETEY();
                if (gd == ed) {
                    FLD(v0, gback, gyoffset + 0);
                }
                FLD(d0, gback, vyoffset + 0);
                FLD(d1, gback, vyoffset + 8);
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
                FSD(d0, gback, gyoffset + 0);
                if (vex.v == ed) {
                    FSD(d0, gback, gyoffset + 8);
                } else {
                    if (gd == ed)
                        d0 = v0;
                    else
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
                    FSD(d0, gback, gyoffset + 8);
                }
            } else
                YMM0(gd);
            break;
        case 0x7D:
            INST_NAME("VHSUBPD Gx, Vx, Ex");
            nextop = F8;
            GETEX(x2, 0, vex.l ? 24 : 8);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            v0 = fpu_get_scratch(dyn);
            if (gd == ed) {
                FLD(v0, gback, gdoffset + 0);
            }
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            FLD(d0, gback, vxoffset + 0);
            FLD(d1, gback, vxoffset + 8);
            if (!BOX64ENV(dynarec_fastnan)) {
                FEQD(x3, d0, d0);
                FEQD(x4, d1, d1);
                AND(x3, x3, x4);
            }
            FSUBD(d0, d0, d1);
            if (!BOX64ENV(dynarec_fastnan)) {
                FEQD(x4, d0, d0);
                BEQZ(x3, 12);
                BNEZ(x4, 8);
                FNEGD(d0, d0);
            }
            FSD(d0, gback, gdoffset + 0);
            if (vex.v == ed) {
                FSD(d0, gback, gdoffset + 8);
            } else {
                if (gd == ed)
                    d0 = v0;
                else
                    FLD(d0, wback, fixedaddress + 0);
                FLD(d1, wback, fixedaddress + 8);
                if (!BOX64ENV(dynarec_fastnan)) {
                    FEQD(x3, d0, d0);
                    FEQD(x4, d1, d1);
                    AND(x3, x3, x4);
                }
                FSUBD(d0, d0, d1);
                if (!BOX64ENV(dynarec_fastnan)) {
                    FEQD(x4, d0, d0);
                    BEQZ(x3, 12);
                    BNEZ(x4, 8);
                    FNEGD(d0, d0);
                }
                FSD(d0, gback, gdoffset + 8);
            }
            if (vex.l) {
                GETEY();
                if (gd == ed) {
                    FLD(v0, gback, gyoffset + 0);
                }
                FLD(d0, gback, vyoffset + 0);
                FLD(d1, gback, vyoffset + 8);
                if (!BOX64ENV(dynarec_fastnan)) {
                    FEQD(x3, d0, d0);
                    FEQD(x4, d1, d1);
                    AND(x3, x3, x4);
                }
                FSUBD(d0, d0, d1);
                if (!BOX64ENV(dynarec_fastnan)) {
                    FEQD(x4, d0, d0);
                    BEQZ(x3, 12);
                    BNEZ(x4, 8);
                    FNEGD(d0, d0);
                }
                FSD(d0, gback, gyoffset + 0);
                if (vex.v == ed) {
                    FSD(d0, gback, gyoffset + 8);
                } else {
                    if (gd == ed)
                        d0 = v0;
                    else
                        FLD(d0, wback, fixedaddress + 0);
                    FLD(d1, wback, fixedaddress + 8);
                    if (!BOX64ENV(dynarec_fastnan)) {
                        FEQD(x3, d0, d0);
                        FEQD(x4, d1, d1);
                        AND(x3, x3, x4);
                    }
                    FSUBD(d0, d0, d1);
                    if (!BOX64ENV(dynarec_fastnan)) {
                        FEQD(x4, d0, d0);
                        BEQZ(x3, 12);
                        BNEZ(x4, 8);
                        FNEGD(d0, d0);
                    }
                    FSD(d0, gback, gyoffset + 8);
                }
            } else
                YMM0(gd);
            break;
        case 0x7E:
            INST_NAME("VMOVD Ed, Gx");
            nextop = F8;
            GETGX();
            ed = TO_NAT((nextop & 7) + (rex.b << 3));
            if (rex.w) {
                if (MODREG) {
                    LD(ed, gback, gdoffset);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                    LD(x3, gback, gdoffset);
                    SD(x3, wback, fixedaddress);
                    SMWRITE2();
                }
            } else {
                if (MODREG) {
                    LWU(ed, gback, gdoffset);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                    LWU(x3, gback, gdoffset);
                    SW(x3, wback, fixedaddress);
                    SMWRITE2();
                }
            }
            break;
        case 0x7F:
            INST_NAME("VMOVDQA Ex, Gx");
            nextop = F8;
            GETEX(x2, 0, vex.l ? 24 : 8);
            GETGX();
            GETGY();
            LD(x3, gback, gdoffset);
            SD(x3, wback, fixedaddress);
            LD(x3, gback, gdoffset + 8);
            SD(x3, wback, fixedaddress + 8);
            if (vex.l) {
                GETEY();
                LD(x3, gback, gyoffset);
                SD(x3, wback, fixedaddress);
                LD(x3, gback, gyoffset + 8);
                SD(x3, wback, fixedaddress + 8);
            } else if (MODREG)
                YMM0(ed);
            break;
        case 0xC2:
            INST_NAME("VCMPPD Gx, Vx, Ex, Ib");
            nextop = F8;
            GETEX(x2, 1, vex.l ? 24 : 8);
            GETGX();
            GETVX();
            GETGY();
            GETVY();
            u8 = F8;
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            for (int i = 0; i < 2 + (vex.l ? 2 : 0); ++i) {
                if (i == 2) { GETEY(); }
                if (i < 2) {
                    FLD(d0, vback, vxoffset + 8 * i);
                    FLD(d1, wback, fixedaddress + 8 * i);
                } else {
                    FLD(d0, vback, vyoffset + 8 * (i - 2));
                    FLD(d1, wback, fixedaddress + 8 * (i - 2));
                }

                if ((u8 & 0xf) != 0x0b && (u8 & 0xf) != 0xf) {
                    // x6 = !(isnan(d0) || isnan(d1))
                    FEQD(x4, d0, d0);
                    FEQD(x3, d1, d1);
                    AND(x6, x3, x4);
                }

                switch (u8 & 0x7) {
                    case 0:
                        FEQD(x3, d0, d1);
                        break; // Equal
                    case 1:
                        BEQ(x6, xZR, 8);
                        FLTD(x3, d0, d1);
                        break; // Less than
                    case 2:
                        BEQ(x6, xZR, 8);
                        FLED(x3, d0, d1);
                        break; // Less or equal
                    case 3:
                        if (u8 & 0x8)
                            ADDI(x3, xZR, 0);
                        else
                            XORI(x3, x6, 1);
                        break;
                    case 4:
                        FEQD(x3, d0, d1);
                        XORI(x3, x3, 1);
                        break; // Not Equal or unordered
                    case 5:
                        BEQ(x6, xZR, 12);
                        FLED(x3, d1, d0);
                        J(8);
                        ADDI(x3, xZR, 1);
                        break; // Greater or equal or unordered
                    case 6:
                        BEQ(x6, xZR, 12);
                        FLTD(x3, d1, d0);
                        J(8);
                        ADDI(x3, xZR, 1);
                        break; // Greater or unordered
                    case 7:
                        if (u8 & 0x8)
                            ADDI(x3, xZR, 1);
                        else
                            MV(x3, x6);
                        break; // Not NaN
                }
                if ((u8 & 0x3) != 0x3) {
                    if ((u8 & 0xC) == 0x8 || (u8 & 0xC) == 0x4) {
                        XORI(x7, x6, 1);
                        OR(x3, x3, x7);
                    } else
                        AND(x3, x3, x6);
                }
                NEG(x3, x3);
                if (i < 2) {
                    SD(x3, gback, gdoffset + 8 * i);
                } else {
                    SD(x3, gback, gyoffset + 8 * (i - 2));
                }
            }
            if (!vex.l) YMM0(gd);
            break;
        case 0xC6:
            INST_NAME("VSHUFPD Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGX();
            GETEX(x2, 1, 8);
            GETGY();
            GETVX();
            GETVY();
            u8 = F8;
            LD(x3, vback, vxoffset + 8 * (u8 & 1));
            LD(x4, wback, fixedaddress + 8 * ((u8 >> 1) & 1));
            SD(x3, gback, gdoffset + 0);
            SD(x4, gback, gdoffset + 8);
            if (vex.l) {
                GETEY();
                LD(x3, vback, vyoffset + 8 * ((u8 >> 2) & 1));
                LD(x4, wback, fixedaddress + 8 * ((u8 >> 3) & 1));
                SD(x3, gback, gyoffset + 0);
                SD(x4, gback, gyoffset + 8);
            } else
                YMM0(gd);
            break;
        case 0xD0:
            INST_NAME("VADDSUBPD Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 24 : 8);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            v0 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            for (int i = 0; i < 2; ++i) {
                FLD(v0, wback, fixedaddress + 8 * i);
                FLD(v1, vback, vxoffset + 8 * i);
                if (!BOX64ENV(dynarec_fastnan)) {
                    FEQD(x3, v0, v0);
                    FEQD(x4, v1, v1);
                }
                if (i == 0)
                    FSUBD(v0, v1, v0);
                else
                    FADDD(v0, v1, v0);
                if (!BOX64ENV(dynarec_fastnan)) {
                    AND(x3, x3, x4);
                    BEQZ(x3, 16);
                    FEQD(x3, v0, v0);
                    BNEZ(x3, 8);
                    FNEGD(v0, v0);
                }
                FSD(v0, gback, gdoffset + 8 * i);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 2; ++i) {
                    FLD(v0, wback, fixedaddress + 8 * i);
                    FLD(v1, vback, vyoffset + 8 * i);
                    if (!BOX64ENV(dynarec_fastnan)) {
                        FEQD(x3, v0, v0);
                        FEQD(x4, v1, v1);
                    }
                    if (i == 0)
                        FSUBD(v0, v1, v0);
                    else
                        FADDD(v0, v1, v0);
                    if (!BOX64ENV(dynarec_fastnan)) {
                        AND(x3, x3, x4);
                        BEQZ(x3, 16);
                        FEQD(x3, v0, v0);
                        BNEZ(x3, 8);
                        FNEGD(v0, v0);
                    }
                    FSD(v0, gback, gyoffset + 8 * i);
                }
            } else
                YMM0(gd);
            break;
        case 0xD1:
            INST_NAME("VPSRLW Gx, Vx, Ex");
            nextop = F8;
            GETEX(x2, 0, 1);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            LD(x3, wback, fixedaddress);
            ADDI(x4, xZR, 16);
            BLTU_MARK(x3, x4);
            SD(xZR, gback, gdoffset + 0);
            SD(xZR, gback, gdoffset + 8);
            B_MARK2_nocond;
            MARK;
            for (int i = 0; i < 8; ++i) {
                LHU(x5, vback, vxoffset + 2 * i);
                SRLW(x5, x5, x3);
                SH(x5, gback, gdoffset + 2 * i);
            }
            MARK2;
            if (vex.l) {
                BLTU_MARK3(x3, x4);
                SD(xZR, gback, gyoffset + 0);
                SD(xZR, gback, gyoffset + 8);
                B_NEXT_nocond;
                MARK3;
                for (int i = 0; i < 8; ++i) {
                    LHU(x5, vback, vyoffset + 2 * i);
                    SRLW(x5, x5, x3);
                    SH(x5, gback, gyoffset + 2 * i);
                }
            } else
                YMM0(gd);
            break;
        case 0xD2:
            INST_NAME("VPSRLD Gx, Vx, Ex");
            nextop = F8;
            GETEX(x2, 0, 1);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            LD(x3, wback, fixedaddress);
            ADDI(x4, xZR, 32);
            BLTU_MARK(x3, x4);
            SD(xZR, gback, gdoffset + 0);
            SD(xZR, gback, gdoffset + 8);
            B_MARK2_nocond;
            MARK;
            for (int i = 0; i < 4; ++i) {
                LWU(x5, vback, vxoffset + 4 * i);
                SRLW(x5, x5, x3);
                SW(x5, gback, gdoffset + 4 * i);
            }
            MARK2;
            if (vex.l) {
                BLTU_MARK3(x3, x4);
                SD(xZR, gback, gyoffset + 0);
                SD(xZR, gback, gyoffset + 8);
                B_NEXT_nocond;
                MARK3;
                for (int i = 0; i < 4; ++i) {
                    LWU(x5, vback, vyoffset + 4 * i);
                    SRLW(x5, x5, x3);
                    SW(x5, gback, gyoffset + 4 * i);
                }
            } else
                YMM0(gd);
            break;
        case 0xD3:
            INST_NAME("VPSRLQ Gx, Vx, Ex");
            nextop = F8;
            GETEX(x2, 0, 1);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            LD(x3, wback, fixedaddress);
            ADDI(x4, xZR, 64);
            BLTU_MARK(x3, x4);
            SD(xZR, gback, gdoffset + 0);
            SD(xZR, gback, gdoffset + 8);
            B_MARK2_nocond;
            MARK;
            for (int i = 0; i < 2; ++i) {
                LD(x5, vback, vxoffset + 8 * i);
                SRL(x5, x5, x3);
                SD(x5, gback, gdoffset + 8 * i);
            }
            MARK2;
            if (vex.l) {
                BLTU_MARK3(x3, x4);
                SD(xZR, gback, gyoffset + 0);
                SD(xZR, gback, gyoffset + 8);
                B_NEXT_nocond;
                MARK3;
                for (int i = 0; i < 2; ++i) {
                    LD(x5, vback, vyoffset + 8 * i);
                    SRL(x5, x5, x3);
                    SD(x5, gback, gyoffset + 8 * i);
                }
            } else
                YMM0(gd);
            break;
        case 0xD4:
            INST_NAME("VPADDQ Gx, Vx, Ex");
            nextop = F8;
            GETEX(x2, 0, 8);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            for (int i = 0; i < 2; ++i) {
                LD(x3, vback, vxoffset + 8 * i);
                LD(x4, wback, fixedaddress + 8 * i);
                ADD(x3, x3, x4);
                SD(x3, gback, gdoffset + 8 * i);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 2; ++i) {
                    LD(x3, vback, vyoffset + 8 * i);
                    LD(x4, wback, fixedaddress + 8 * i);
                    ADD(x3, x3, x4);
                    SD(x3, gback, gyoffset + 8 * i);
                }
            } else
                YMM0(gd);
            break;
        case 0xD5:
            INST_NAME("VPMULLW Gx, Vx, Ex");
            nextop = F8;
            GETEX(x2, 0, vex.l ? 30 : 14);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            for (int i = 0; i < 8; ++i) {
                LH(x3, vback, vxoffset + 2 * i);
                LH(x4, wback, fixedaddress + 2 * i);
                MULW(x3, x3, x4);
                SH(x3, gback, gdoffset + 2 * i);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 8; ++i) {
                    LH(x3, vback, vyoffset + 2 * i);
                    LH(x4, wback, fixedaddress + 2 * i);
                    MULW(x3, x3, x4);
                    SH(x3, gback, gyoffset + 2 * i);
                }
            } else
                YMM0(gd);
            break;
        case 0xD6:
            INST_NAME("VMOVQ Ex, Gx");
            nextop = F8;
            GETEX(x2, 0, 8);
            GETGX();
            LD(x3, gback, gdoffset);
            SD(x3, wback, fixedaddress);
            if (MODREG) {
                SD(xZR, wback, fixedaddress + 8);
                YMM0(ed);
            } else {
                SMWRITE2();
            }
            break;
        case 0xD8:
            INST_NAME("VPSUBUSB Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 31 : 15);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            for (int i = 0; i < 16; ++i) {
                LBU(x3, vback, vxoffset + i);
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
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 16; ++i) {
                    LBU(x3, vback, vyoffset + i);
                    LBU(x4, wback, fixedaddress + i);
                    SUB(x3, x3, x4);
                    if (cpuext.zbb) {
                        MAX(x3, x3, xZR);
                    } else {
                        NOT(x4, x3);
                        SRAI(x4, x4, 63);
                        AND(x3, x3, x4);
                    }
                    SB(x3, gback, gyoffset + i);
                }
            } else
                YMM0(gd);
            break;
        case 0xD9:
            INST_NAME("VPSUBUSW Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 31 : 15);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            for (int i = 0; i < 8; ++i) {
                LHU(x3, vback, vxoffset + i * 2);
                LHU(x4, wback, fixedaddress + i * 2);
                SUB(x3, x3, x4);
                if (cpuext.zbb) {
                    MAX(x3, x3, xZR);
                } else {
                    NOT(x4, x3);
                    SRAI(x4, x4, 63);
                    AND(x3, x3, x4);
                }
                SH(x3, gback, gdoffset + i * 2);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 8; ++i) {
                    LHU(x3, vback, vyoffset + i * 2);
                    LHU(x4, wback, fixedaddress + i * 2);
                    SUB(x3, x3, x4);
                    if (cpuext.zbb) {
                        MAX(x3, x3, xZR);
                    } else {
                        NOT(x4, x3);
                        SRAI(x4, x4, 63);
                        AND(x3, x3, x4);
                    }
                    SH(x3, gback, gyoffset + i * 2);
                }
            } else
                YMM0(gd);
            break;
        case 0xDA:
            INST_NAME("VPMINUB Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 31 : 15);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            for (int i = 0; i < 16; ++i) {
                LBU(x3, vback, vxoffset + i);
                LBU(x4, wback, fixedaddress + i);
                if (cpuext.zbb) {
                    MINU(x3, x3, x4);
                } else {
                    BLTU(x3, x4, 8);
                    MV(x3, x4);
                }
                SB(x3, gback, gdoffset + i);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 16; ++i) {
                    LBU(x3, vback, vyoffset + i);
                    LBU(x4, wback, fixedaddress + i);
                    if (cpuext.zbb) {
                        MINU(x3, x3, x4);
                    } else {
                        BLTU(x3, x4, 8);
                        MV(x3, x4);
                    }
                    SB(x3, gback, gyoffset + i);
                }
            } else
                YMM0(gd);
            break;
        case 0xDB:
            INST_NAME("VPAND Gx, Vx, Ex");
            nextop = F8;
            GETEX(x2, 0, 8);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            for (int i = 0; i < 2; ++i) {
                LD(x3, vback, vxoffset + 8 * i);
                LD(x4, wback, fixedaddress + 8 * i);
                AND(x3, x3, x4);
                SD(x3, gback, gdoffset + 8 * i);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 2; ++i) {
                    LD(x3, vback, vyoffset + 8 * i);
                    LD(x4, wback, fixedaddress + 8 * i);
                    AND(x3, x3, x4);
                    SD(x3, gback, gyoffset + 8 * i);
                }
            } else
                YMM0(gd);
            break;
        case 0xDC:
            INST_NAME("VPADDUSB Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 31 : 15);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            ADDI(x5, xZR, 0xFF);
            for (int i = 0; i < 16; ++i) {
                LBU(x3, vback, vxoffset + i);
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
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 16; ++i) {
                    LBU(x3, vback, vyoffset + i);
                    LBU(x4, wback, fixedaddress + i);
                    ADD(x3, x3, x4);
                    if (cpuext.zbb) {
                        MINU(x3, x3, x5);
                    } else {
                        BLT(x3, x5, 8);
                        ADDI(x3, xZR, 0xFF);
                    }
                    SB(x3, gback, gyoffset + i);
                }
            } else
                YMM0(gd);
            break;
        case 0xDD:
            INST_NAME("VPADDUSW Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 30 : 14);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            MOV32w(x5, 65535);
            for (int i = 0; i < 8; ++i) {
                LHU(x3, vback, vxoffset + i * 2);
                LHU(x4, wback, fixedaddress + i * 2);
                ADDW(x3, x3, x4);
                if (cpuext.zbb) {
                    MINU(x3, x3, x5);
                } else {
                    BGE(x5, x3, 8);
                    MV(x3, x5);
                }
                SH(x3, gback, gdoffset + i * 2);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 8; ++i) {
                    LHU(x3, vback, vyoffset + i * 2);
                    LHU(x4, wback, fixedaddress + i * 2);
                    ADDW(x3, x3, x4);
                    if (cpuext.zbb) {
                        MINU(x3, x3, x5);
                    } else {
                        BGE(x5, x3, 8);
                        MV(x3, x5);
                    }
                    SH(x3, gback, gyoffset + i * 2);
                }
            } else
                YMM0(gd);
            break;
        case 0xDE:
            INST_NAME("VPMAXUB Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 31 : 15);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            for (int i = 0; i < 16; ++i) {
                LBU(x3, vback, vxoffset + i);
                LBU(x4, wback, fixedaddress + i);
                if (cpuext.zbb) {
                    MAXU(x3, x3, x4);
                } else {
                    BLTU(x4, x3, 8);
                    MV(x3, x4);
                }
                SB(x3, gback, gdoffset + i);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 16; ++i) {
                    LBU(x3, vback, vyoffset + i);
                    LBU(x4, wback, fixedaddress + i);
                    if (cpuext.zbb) {
                        MAXU(x3, x3, x4);
                    } else {
                        BLTU(x4, x3, 8);
                        MV(x3, x4);
                    }
                    SB(x3, gback, gyoffset + i);
                }
            } else
                YMM0(gd);
            break;
        case 0xDF:
            INST_NAME("VPANDN Gx, Vx, Ex");
            nextop = F8;
            GETEX(x2, 0, 8);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            for (int i = 0; i < 2; ++i) {
                LD(x3, vback, vxoffset + 8 * i);
                LD(x4, wback, fixedaddress + 8 * i);
                NOT(x3, x3);
                AND(x3, x3, x4);
                SD(x3, gback, gdoffset + 8 * i);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 2; ++i) {
                    LD(x3, vback, vyoffset + 8 * i);
                    LD(x4, wback, fixedaddress + 8 * i);
                    NOT(x3, x3);
                    AND(x3, x3, x4);
                    SD(x3, gback, gyoffset + 8 * i);
                }
            } else
                YMM0(gd);
            break;
        case 0xE0:
            INST_NAME("VPAVGB Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 31 : 15);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            for (int i = 0; i < 16; ++i) {
                LBU(x3, vback, vxoffset + i);
                LBU(x4, wback, fixedaddress + i);
                ADDW(x3, x3, x4);
                ADDIW(x3, x3, 1);
                SRAIW(x3, x3, 1);
                SB(x3, gback, gdoffset + i);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 16; ++i) {
                    LBU(x3, vback, vyoffset + i);
                    LBU(x4, wback, fixedaddress + i);
                    ADDW(x3, x3, x4);
                    ADDIW(x3, x3, 1);
                    SRAIW(x3, x3, 1);
                    SB(x3, gback, gyoffset + i);
                }
            } else
                YMM0(gd);
            break;
        case 0xE1:
            INST_NAME("VPSRAW Gx, Vx, Ex");
            nextop = F8;
            GETEX(x2, 0, 1);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            ADDI(x4, xZR, 16);
            LD(x3, wback, fixedaddress);
            BLTU(x3, x4, 8);
            SUBI(x3, x4, 1);
            for (int i = 0; i < 8; ++i) {
                LH(x5, vback, vxoffset + 2 * i);
                SRAW(x5, x5, x3);
                SH(x5, gback, gdoffset + 2 * i);
            }
            if (vex.l) {
                for (int i = 0; i < 8; ++i) {
                    LH(x5, vback, vyoffset + 2 * i);
                    SRAW(x5, x5, x3);
                    SH(x5, gback, gyoffset + 2 * i);
                }
            } else
                YMM0(gd);
            break;
        case 0xE2:
            INST_NAME("VPSRAD Gx, Vx, Ex");
            nextop = F8;
            GETEX(x2, 0, 1);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            ADDI(x4, xZR, 32);
            LD(x3, wback, fixedaddress);
            BLTU(x3, x4, 8);
            SUBI(x3, x4, 1);
            for (int i = 0; i < 4; ++i) {
                LW(x5, vback, vxoffset + 4 * i);
                SRAW(x5, x5, x3);
                SW(x5, gback, gdoffset + 4 * i);
            }
            if (vex.l) {
                for (int i = 0; i < 4; ++i) {
                    LW(x5, vback, vyoffset + 4 * i);
                    SRAW(x5, x5, x3);
                    SW(x5, gback, gyoffset + 4 * i);
                }
            } else
                YMM0(gd);
            break;
        case 0xE3:
            INST_NAME("VPAVGW Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 30 : 14);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            for (int i = 0; i < 8; ++i) {
                LHU(x3, vback, vxoffset + i * 2);
                LHU(x4, wback, fixedaddress + i * 2);
                ADDW(x3, x3, x4);
                ADDIW(x3, x3, 1);
                SRAIW(x3, x3, 1);
                SH(x3, gback, gdoffset + i * 2);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 8; ++i) {
                    LHU(x3, vback, vyoffset + i * 2);
                    LHU(x4, wback, fixedaddress + i * 2);
                    ADDW(x3, x3, x4);
                    ADDIW(x3, x3, 1);
                    SRAIW(x3, x3, 1);
                    SH(x3, gback, gyoffset + i * 2);
                }
            } else
                YMM0(gd);
            break;
        case 0xE4:
            INST_NAME("VPMULHUW Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 30 : 14);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            for (int i = 0; i < 8; ++i) {
                LHU(x3, vback, vxoffset + i * 2);
                LHU(x4, wback, fixedaddress + i * 2);
                MULW(x3, x3, x4);
                SRLIW(x3, x3, 16);
                SH(x3, gback, gdoffset + i * 2);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 8; ++i) {
                    LHU(x3, vback, vyoffset + i * 2);
                    LHU(x4, wback, fixedaddress + i * 2);
                    MULW(x3, x3, x4);
                    SRLIW(x3, x3, 16);
                    SH(x3, gback, gyoffset + i * 2);
                }
            } else
                YMM0(gd);
            break;
        case 0xE5:
            INST_NAME("VPMULHW Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 30 : 14);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            for (int i = 0; i < 8; ++i) {
                LH(x3, vback, vxoffset + i * 2);
                LH(x4, wback, fixedaddress + i * 2);
                MULW(x3, x3, x4);
                SRAIW(x3, x3, 16);
                SH(x3, gback, gdoffset + i * 2);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 8; ++i) {
                    LH(x3, vback, vyoffset + i * 2);
                    LH(x4, wback, fixedaddress + i * 2);
                    MULW(x3, x3, x4);
                    SRAIW(x3, x3, 16);
                    SH(x3, gback, gyoffset + i * 2);
                }
            } else
                YMM0(gd);
            break;
        case 0xE8:
            INST_NAME("VPSUBSB Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 31 : 15);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            ADDIW(x6, xZR, 0xF80); // -128
            ADDIW(x7, xZR, 0x80);  // 128
            for (int i = 0; i < 16; ++i) {
                LB(x3, vback, vxoffset + i);
                LB(x4, wback, fixedaddress + i);
                SUBW(x3, x3, x4);
                SATw(x3, x6, x7);
                SB(x3, gback, gdoffset + i);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 16; ++i) {
                    LB(x3, vback, vyoffset + i);
                    LB(x4, wback, fixedaddress + i);
                    SUBW(x3, x3, x4);
                    SATw(x3, x6, x7);
                    SB(x3, gback, gyoffset + i);
                }
            } else
                YMM0(gd);
            break;
        case 0xE9:
            INST_NAME("VPSUBSW Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 30 : 14);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            LUI(x6, 0xFFFF8); // -32768
            LUI(x7, 0x8);     // 32768
            for (int i = 0; i < 8; ++i) {
                LH(x3, vback, vxoffset + i * 2);
                LH(x4, wback, fixedaddress + i * 2);
                SUBW(x3, x3, x4);
                SATw(x3, x6, x7);
                SH(x3, gback, gdoffset + i * 2);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 8; ++i) {
                    LH(x3, vback, vyoffset + i * 2);
                    LH(x4, wback, fixedaddress + i * 2);
                    SUBW(x3, x3, x4);
                    SATw(x3, x6, x7);
                    SH(x3, gback, gyoffset + i * 2);
                }
            } else
                YMM0(gd);
            break;
        case 0xEA:
            INST_NAME("VPMINSW Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 30 : 14);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            for (int i = 0; i < 8; ++i) {
                LH(x3, gback, vxoffset + 2 * i);
                LH(x4, wback, fixedaddress + 2 * i);
                if (cpuext.zbb) {
                    MIN(x3, x3, x4);
                } else {
                    BLT(x3, x4, 8);
                    MV(x3, x4);
                }
                SH(x3, gback, gdoffset + 2 * i);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 8; ++i) {
                    LH(x3, gback, vyoffset + 2 * i);
                    LH(x4, wback, fixedaddress + 2 * i);
                    if (cpuext.zbb) {
                        MIN(x3, x3, x4);
                    } else {
                        BLT(x3, x4, 8);
                        MV(x3, x4);
                    }
                    SH(x3, gback, gyoffset + 2 * i);
                }
            } else
                YMM0(gd);
            break;
        case 0xEB:
            INST_NAME("VPOR Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 24 : 8);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            for (int i = 0; i < 2; ++i) {
                LD(x3, vback, vxoffset + 8 * i);
                LD(x4, wback, fixedaddress + 8 * i);
                OR(x3, x3, x4);
                SD(x3, gback, gdoffset + 8 * i);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 2; ++i) {
                    LD(x3, vback, vyoffset + 8 * i);
                    LD(x4, wback, fixedaddress + 8 * i);
                    OR(x3, x3, x4);
                    SD(x3, gback, gyoffset + 8 * i);
                }
            } else
                YMM0(gd);
            break;
        case 0xEC:
            INST_NAME("VPADDSB Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 31 : 15);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            ADDIW(x6, xZR, 0xF80); // -128
            ADDIW(x7, xZR, 0x80);  // 128
            for (int i = 0; i < 16; ++i) {
                LB(x3, vback, vxoffset + i);
                LB(x4, wback, fixedaddress + i);
                ADD(x3, x3, x4);
                SATw(x3, x6, x7);
                SB(x3, gback, gdoffset + i);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 16; ++i) {
                    LB(x3, vback, vyoffset + i);
                    LB(x4, wback, fixedaddress + i);
                    ADD(x3, x3, x4);
                    SATw(x3, x6, x7);
                    SB(x3, gback, gyoffset + i);
                }
            } else
                YMM0(gd);
            break;
        case 0xED:
            INST_NAME("VPADDSW Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 30 : 14);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            LUI(x6, 0xFFFF8); // -32768
            LUI(x7, 0x8);     // 32768
            for (int i = 0; i < 8; ++i) {
                LH(x3, vback, vxoffset + i * 2);
                LH(x4, wback, fixedaddress + i * 2);
                ADD(x3, x3, x4);
                SATw(x3, x6, x7);
                SH(x3, gback, gdoffset + i * 2);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 8; ++i) {
                    LH(x3, vback, vyoffset + i * 2);
                    LH(x4, wback, fixedaddress + i * 2);
                    ADD(x3, x3, x4);
                    SATw(x3, x6, x7);
                    SH(x3, gback, gyoffset + i * 2);
                }
            } else
                YMM0(gd);
            break;
        case 0xEE:
            INST_NAME("VPMAXSW Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 30 : 14);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            for (int i = 0; i < 8; ++i) {
                LH(x3, gback, vxoffset + 2 * i);
                LH(x4, wback, fixedaddress + 2 * i);
                if (cpuext.zbb) {
                    MAX(x3, x3, x4);
                } else {
                    BLT(x4, x3, 8);
                    MV(x3, x4);
                }
                SH(x3, gback, gdoffset + 2 * i);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 8; ++i) {
                    LH(x3, gback, vyoffset + 2 * i);
                    LH(x4, wback, fixedaddress + 2 * i);
                    if (cpuext.zbb) {
                        MAX(x3, x3, x4);
                    } else {
                        BLT(x4, x3, 8);
                        MV(x3, x4);
                    }
                    SH(x3, gback, gyoffset + 2 * i);
                }
            } else
                YMM0(gd);
            break;
        case 0xEF:
            INST_NAME("VPXOR Gx, Vx, Ex");
            nextop = F8;
            GETEX(x2, 0, vex.l ? 24 : 8);
            GETGX();
            GETVX();
            GETGY();
            GETVY();
            LD(x3, vback, vxoffset + 0);
            LD(x4, wback, fixedaddress + 0);
            XOR(x3, x3, x4);
            SD(x3, gback, gdoffset + 0);
            LD(x3, vback, vxoffset + 8);
            LD(x4, wback, fixedaddress + 8);
            XOR(x3, x3, x4);
            SD(x3, gback, gdoffset + 8);
            if (vex.l) {
                GETEY();
                LD(x3, vback, vyoffset + 0);
                LD(x4, wback, fixedaddress + 0);
                XOR(x3, x3, x4);
                SD(x3, gback, gyoffset + 0);
                LD(x3, vback, vyoffset + 8);
                LD(x4, wback, fixedaddress + 8);
                XOR(x3, x3, x4);
                SD(x3, gback, gyoffset + 8);
            } else
                YMM0(gd);
            break;
        case 0xF1:
            INST_NAME("VPSLLW Gx, Vx, Ex");
            nextop = F8;
            GETEX(x2, 0, 1);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            LD(x3, wback, fixedaddress);
            ADDI(x4, xZR, 16);
            BLTU_MARK(x3, x4);
            SD(xZR, gback, gdoffset + 0);
            SD(xZR, gback, gdoffset + 8);
            B_MARK2_nocond;
            MARK;
            for (int i = 0; i < 8; ++i) {
                LHU(x5, vback, vxoffset + 2 * i);
                SLLW(x5, x5, x3);
                SH(x5, gback, gdoffset + 2 * i);
            }
            MARK2;
            if (vex.l) {
                BLTU_MARK3(x3, x4);
                SD(xZR, gback, gyoffset + 0);
                SD(xZR, gback, gyoffset + 8);
                B_NEXT_nocond;
                MARK3;
                for (int i = 0; i < 8; ++i) {
                    LHU(x5, vback, vyoffset + 2 * i);
                    SLLW(x5, x5, x3);
                    SH(x5, gback, gyoffset + 2 * i);
                }
            } else
                YMM0(gd);
            break;
        case 0xF2:
            INST_NAME("VPSLLD Gx, Vx, Ex");
            nextop = F8;
            GETEX(x2, 0, 1);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            LD(x3, wback, fixedaddress);
            ADDI(x4, xZR, 32);
            BLTU_MARK(x3, x4);
            SD(xZR, gback, gdoffset + 0);
            SD(xZR, gback, gdoffset + 8);
            B_MARK2_nocond;
            MARK;
            for (int i = 0; i < 4; ++i) {
                LWU(x5, vback, vxoffset + 4 * i);
                SLLW(x5, x5, x3);
                SW(x5, gback, gdoffset + 4 * i);
            }
            MARK2;
            if (vex.l) {
                BLTU_MARK3(x3, x4);
                SD(xZR, gback, gyoffset + 0);
                SD(xZR, gback, gyoffset + 8);
                B_NEXT_nocond;
                MARK3;
                for (int i = 0; i < 4; ++i) {
                    LWU(x5, vback, vyoffset + 4 * i);
                    SLLW(x5, x5, x3);
                    SW(x5, gback, gyoffset + 4 * i);
                }
            } else
                YMM0(gd);
            break;
        case 0xF3:
            INST_NAME("VPSLLQ Gx, Vx, Ex");
            nextop = F8;
            GETEX(x2, 0, 1);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            LD(x3, wback, fixedaddress);
            ADDI(x4, xZR, 64);
            BLTU_MARK(x3, x4);
            SD(xZR, gback, gdoffset + 0);
            SD(xZR, gback, gdoffset + 8);
            B_MARK2_nocond;
            MARK;
            for (int i = 0; i < 2; ++i) {
                LD(x5, vback, vxoffset + 8 * i);
                SLL(x5, x5, x3);
                SD(x5, gback, gdoffset + 8 * i);
            }
            MARK2;
            if (vex.l) {
                BLTU_MARK3(x3, x4);
                SD(xZR, gback, gyoffset + 0);
                SD(xZR, gback, gyoffset + 8);
                B_NEXT_nocond;
                MARK3;
                for (int i = 0; i < 2; ++i) {
                    LD(x5, vback, vyoffset + 8 * i);
                    SLL(x5, x5, x3);
                    SD(x5, gback, gyoffset + 8 * i);
                }
            } else
                YMM0(gd);
            break;
        case 0xF4:
            INST_NAME("VPMULUDQ Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 24 : 8);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            for (int i = 0; i < 2; ++i) {
                LWU(x3, gback, vxoffset + i * 8);
                LWU(x4, wback, fixedaddress + i * 8);
                MUL(x3, x3, x4);
                SD(x3, gback, gdoffset + i * 8);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 2; ++i) {
                    LWU(x3, gback, vyoffset + i * 8);
                    LWU(x4, wback, fixedaddress + i * 8);
                    MUL(x3, x3, x4);
                    SD(x3, gback, gyoffset + i * 8);
                }
            } else
                YMM0(gd);
            break;
        case 0xF5:
            INST_NAME("VPMADDWD Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 30 : 14);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            for (int i = 0; i < 4; ++i) {
                LH(x3, gback, vxoffset + 2 * (i * 2 + 0));
                LH(x4, wback, fixedaddress + 2 * (i * 2 + 0));
                MULW(x5, x3, x4);
                LH(x3, gback, vxoffset + 2 * (i * 2 + 1));
                LH(x4, wback, fixedaddress + 2 * (i * 2 + 1));
                MULW(x6, x3, x4);
                ADDW(x5, x5, x6);
                SW(x5, gback, gdoffset + 4 * i);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 4; ++i) {
                    LH(x3, gback, vyoffset + 2 * (i * 2 + 0));
                    LH(x4, wback, fixedaddress + 2 * (i * 2 + 0));
                    MULW(x5, x3, x4);
                    LH(x3, gback, vyoffset + 2 * (i * 2 + 1));
                    LH(x4, wback, fixedaddress + 2 * (i * 2 + 1));
                    MULW(x6, x3, x4);
                    ADDW(x5, x5, x6);
                    SW(x5, gback, gyoffset + 4 * i);
                }
            } else
                YMM0(gd);
            break;
        case 0xF6:
            INST_NAME("VPSADBW Gx, Vx, Ex");
            nextop = F8;
            GETEX(x2, 0, vex.l ? 31 : 15);
            GETGX();
            GETVX();
            GETGY();
            GETVY();
            for (int i = 0; i < 16; ++i) {
                if (i == 0 || i == 8) MV(x6, xZR);
                LBU(x3, gback, vxoffset + i);
                LBU(x4, wback, fixedaddress + i);
                SUBW(x3, x3, x4);
                SRAIW(x5, x3, 31);
                XOR(x3, x5, x3);
                SUBW(x3, x3, x5);
                ANDI(x3, x3, 0xff);
                ADDW(x6, x6, x3);
                if (i == 7 || i == 15)
                    SD(x6, gback, gdoffset + i + 1 - 8);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 16; ++i) {
                    if (i == 0 || i == 8) MV(x6, xZR);
                    LBU(x3, gback, vyoffset + i);
                    LBU(x4, wback, fixedaddress + i);
                    SUBW(x3, x3, x4);
                    SRAIW(x5, x3, 31);
                    XOR(x3, x5, x3);
                    SUBW(x3, x3, x5);
                    ANDI(x3, x3, 0xff);
                    ADDW(x6, x6, x3);
                    if (i == 7 || i == 15)
                        SD(x6, gback, gyoffset + i + 1 - 8);
                }
            } else
                YMM0(gd);
            break;
        case 0xF8:
            INST_NAME("VPSUBB Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 31 : 15);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            for (int i = 0; i < 16; ++i) {
                LB(x3, vback, vxoffset + i);
                LB(x4, wback, fixedaddress + i);
                SUB(x3, x3, x4);
                SB(x3, gback, gdoffset + i);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 16; ++i) {
                    LB(x3, vback, vyoffset + i);
                    LB(x4, wback, fixedaddress + i);
                    SUB(x3, x3, x4);
                    SB(x3, gback, gyoffset + i);
                }
            } else
                YMM0(gd);
            break;
        case 0xF9:
            INST_NAME("VPSUBW Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 30 : 14);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            for (int i = 0; i < 8; ++i) {
                LH(x3, vback, vxoffset + i * 2);
                LH(x4, wback, fixedaddress + i * 2);
                SUB(x3, x3, x4);
                SH(x3, gback, gdoffset + i * 2);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 8; ++i) {
                    LH(x3, vback, vyoffset + i * 2);
                    LH(x4, wback, fixedaddress + i * 2);
                    SUB(x3, x3, x4);
                    SH(x3, gback, gyoffset + i * 2);
                }
            } else
                YMM0(gd);
            break;
        case 0xFA:
            INST_NAME("VPSUBD Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 28 : 12);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            for (int i = 0; i < 4; ++i) {
                LW(x3, vback, vxoffset + i * 4);
                LW(x4, wback, fixedaddress + i * 4);
                SUB(x3, x3, x4);
                SW(x3, gback, gdoffset + i * 4);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 4; ++i) {
                    LW(x3, vback, vyoffset + i * 4);
                    LW(x4, wback, fixedaddress + i * 4);
                    SUB(x3, x3, x4);
                    SW(x3, gback, gyoffset + i * 4);
                }
            } else
                YMM0(gd);
            break;
        case 0xFB:
            INST_NAME("VPSUBQ Gx, Vx, Ex");
            nextop = F8;
            GETEX(x2, 0, 8);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            for (int i = 0; i < 2; ++i) {
                LD(x3, vback, vxoffset + 8 * i);
                LD(x4, wback, fixedaddress + 8 * i);
                SUB(x3, x3, x4);
                SD(x3, gback, gdoffset + 8 * i);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 2; ++i) {
                    LD(x3, vback, vyoffset + 8 * i);
                    LD(x4, wback, fixedaddress + 8 * i);
                    SUB(x3, x3, x4);
                    SD(x3, gback, gyoffset + 8 * i);
                }
            } else
                YMM0(gd);
            break;
        case 0xFC:
            INST_NAME("VPADDB Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 31 : 15);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            for (int i = 0; i < 16; ++i) {
                LB(x3, vback, vxoffset + i);
                LB(x4, wback, fixedaddress + i);
                ADD(x3, x3, x4);
                SB(x3, gback, gdoffset + i);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 16; ++i) {
                    LB(x3, vback, vyoffset + i);
                    LB(x4, wback, fixedaddress + i);
                    ADD(x3, x3, x4);
                    SB(x3, gback, gyoffset + i);
                }
            } else
                YMM0(gd);
            break;
        case 0xFD:
            INST_NAME("VPADDW Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 30 : 14);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            for (int i = 0; i < 8; ++i) {
                LH(x3, vback, vxoffset + i * 2);
                LH(x4, wback, fixedaddress + i * 2);
                ADD(x3, x3, x4);
                SH(x3, gback, gdoffset + i * 2);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 8; ++i) {
                    LH(x3, vback, vyoffset + i * 2);
                    LH(x4, wback, fixedaddress + i * 2);
                    ADD(x3, x3, x4);
                    SH(x3, gback, gyoffset + i * 2);
                }
            } else
                YMM0(gd);
            break;
        case 0xFE:
            INST_NAME("VPADDD Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 28 : 12);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            for (int i = 0; i < 4; ++i) {
                LW(x3, vback, vxoffset + i * 4);
                LW(x4, wback, fixedaddress + i * 4);
                ADD(x3, x3, x4);
                SW(x3, gback, gdoffset + i * 4);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 4; ++i) {
                    LW(x3, vback, vyoffset + i * 4);
                    LW(x4, wback, fixedaddress + i * 4);
                    ADD(x3, x3, x4);
                    SW(x3, gback, gyoffset + i * 4);
                }
            } else
                YMM0(gd);
            break;
        default:
            DEFAULT;
    }
    return addr;
}
