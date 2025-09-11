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

uintptr_t dynarec64_AVX_0F(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
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
    int s0, s1;
    uint64_t tmp64u, u64;
    int64_t j64;
    int64_t fixedaddress, gdoffset, vxoffset, gyoffset, vyoffset;
    int unscaled;

    rex_t rex = vex.rex;

    switch (opcode) {
        case 0x10:
            INST_NAME("VMOVUPS Gx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 24 : 8);
            GETGX();
            GETGY();
            LD(x3, wback, fixedaddress + 0);
            LD(x4, wback, fixedaddress + 8);
            SD(x3, gback, gdoffset + 0);
            SD(x4, gback, gdoffset + 8);
            if (vex.l) {
                GETEY();
                LD(x3, wback, fixedaddress + 0);
                LD(x4, wback, fixedaddress + 8);
                SD(x3, gback, gyoffset + 0);
                SD(x4, gback, gyoffset + 8);
            } else
                YMM0(gd);
            break;
        case 0x11:
            INST_NAME("VMOVUPS Ex, Gx");
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
            } else
                YMM0(ed);
            if (!MODREG) SMWRITE2();
            break;
        case 0x12:
            nextop = F8;
            GETEX(x2, 0, 8);
            GETGX();
            GETVX();
            if (MODREG) {
                INST_NAME("VMOVHLPS Gx, Vx, Ex");
                LD(x3, wback, fixedaddress + 8);
                SD(x3, gback, gdoffset);
            } else {
                INST_NAME("VMOVLPS Gx, Vx, Ex");
                LD(x3, wback, fixedaddress);
                SD(x3, gback, gdoffset);
            }
            LD(x3, vback, vxoffset + 8);
            SD(x3, gback, gdoffset + 8);
            YMM0(gd);
            break;
        case 0x14:
            INST_NAME("VUNPCKLPS Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 20 : 4);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            LWU(x3, vback, vxoffset + 0);
            LWU(x5, vback, vxoffset + 4);
            LWU(x4, wback, fixedaddress + 0);
            LWU(x6, wback, fixedaddress + 4);
            SW(x3, gback, gdoffset + 0);
            SW(x4, gback, gdoffset + 4);
            SW(x5, gback, gdoffset + 8);
            SW(x6, gback, gdoffset + 12);
            if (vex.l) {
                GETEY();
                LWU(x3, vback, vyoffset + 0);
                LWU(x5, vback, vyoffset + 4);
                LWU(x4, wback, fixedaddress + 0);
                LWU(x6, wback, fixedaddress + 4);
                SW(x3, gback, gyoffset + 0);
                SW(x4, gback, gyoffset + 4);
                SW(x5, gback, gyoffset + 8);
                SW(x6, gback, gyoffset + 12);
            } else
                YMM0(gd);
            break;
        case 0x15:
            INST_NAME("VUNPCKHPS Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 28 : 12);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            LWU(x3, vback, vxoffset + 8);
            LWU(x5, vback, vxoffset + 12);
            LWU(x4, wback, fixedaddress + 8);
            LWU(x6, wback, fixedaddress + 12);
            SW(x3, gback, gdoffset + 0);
            SW(x4, gback, gdoffset + 4);
            SW(x5, gback, gdoffset + 8);
            SW(x6, gback, gdoffset + 12);
            if (vex.l) {
                GETEY();
                LWU(x3, vback, vyoffset + 8);
                LWU(x5, vback, vyoffset + 12);
                LWU(x4, wback, fixedaddress + 8);
                LWU(x6, wback, fixedaddress + 12);
                SW(x3, gback, gyoffset + 0);
                SW(x4, gback, gyoffset + 4);
                SW(x5, gback, gyoffset + 8);
                SW(x6, gback, gyoffset + 12);
            } else
                YMM0(gd);
            break;
        case 0x16:
            nextop = F8;
            GETEX(x2, 0, 1);
            GETGX();
            GETVX();
            if (MODREG) {
                INST_NAME("VMOVLHPS Gx, Vx, Ex");
                LD(x3, wback, fixedaddress);
                SD(x3, gback, gdoffset + 8);
            } else {
                INST_NAME("VMOVHPS Gx, Vx, Ex");
                LD(x3, wback, fixedaddress);
                SD(x3, gback, gdoffset + 8);
            }
            LD(x4, vback, vxoffset);
            SD(x4, gback, gdoffset);
            YMM0(gd);
            break;
        case 0x28:
            INST_NAME("VMOVAPS Gx, Ex");
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
            INST_NAME("VMOVAPS Ex, Gx");
            nextop = F8;
            GETEX(x2, 0, vex.l ? 24 : 8);
            GETGX();
            LD(x3, gback, gdoffset);
            SD(x3, wback, fixedaddress);
            LD(x3, gback, gdoffset + 8);
            SD(x3, wback, fixedaddress + 8);
            if (vex.l) {
                GETEY();
                GETGY();
                LD(x3, gback, gyoffset);
                SD(x3, wback, fixedaddress);
                LD(x3, gback, gyoffset + 8);
                SD(x3, wback, fixedaddress + 8);
            } else if (MODREG)
                YMM0(ed);
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
        case 0x51:
            INST_NAME("VSQRTPS Gx, Ex");
            nextop = F8;
            GETEX(x2, 0, vex.l ? 28 : 12);
            GETGX();
            GETGY();
            d0 = fpu_get_scratch(dyn);
            for (int i = 0; i < 4; ++i) {
                FLW(d0, wback, fixedaddress + i * 4);
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
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 4; ++i) {
                    FLW(d0, wback, fixedaddress + i * 4);
                    if (!BOX64ENV(dynarec_fastnan)) {
                        FEQS(x3, d0, d0);
                        BNEZ(x3, 4 + 2 * 4); // isnan(d0)? copy it
                        FSW(d0, gback, gyoffset + i * 4);
                        J(4 + 5 * 4); // continue
                    }
                    FSQRTS(d0, d0);
                    if (!BOX64ENV(dynarec_fastnan)) {
                        FEQS(x3, d0, d0);
                        BNEZ(x3, 4 + 4); // isnan(d0)? negate it
                        FNEGS(d0, d0);
                    }
                    FSW(d0, gback, gyoffset + i * 4);
                }
            } else
                YMM0(gd);
            break;
        case 0x52:
            INST_NAME("VRSQRTPS Gx, Ex");
            nextop = F8;
            GETEX(x2, 0, vex.l ? 28 : 12);
            GETGX();
            GETGY();
            s0 = fpu_get_scratch(dyn);
            s1 = fpu_get_scratch(dyn); // 1.0f
            v0 = fpu_get_scratch(dyn); // 0.0f
            LUI(x3, 0x3f800);
            FMVWX(s1, x3); // 1.0f
            if (!BOX64ENV(dynarec_fastnan)) {
                FMVWX(v0, xZR);
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
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 4; ++i) {
                    FLW(s0, wback, fixedaddress + i * 4);
                    if (!BOX64ENV(dynarec_fastnan)) {
                        FLTS(x3, v0, s0); // s0 > 0.0f?
                        BNEZ(x3, 4 + 5 * 4);
                        FEQS(x3, v0, s0); // s0 == 0.0f?
                        BEQZ(x3, 4 + 3 * 4);
                        FDIVS(s0, s1, v0); // generate an inf
                        FSW(s0, gback, gyoffset + i * 4);
                        J(4 + 6 * 4); // continue
                    }
                    FSQRTS(s0, s0);
                    FDIVS(s0, s1, s0);
                    if (!BOX64ENV(dynarec_fastnan)) {
                        FEQS(x3, s0, s0);
                        BNEZ(x3, 4 + 4); // isnan(s0)? negate it
                        FNEGS(s0, s0);
                    }
                    FSW(s0, gback, gyoffset + i * 4);
                }
            } else
                YMM0(gd);
            break;
        case 0x53:
            INST_NAME("VRCPPS Gx, Ex");
            nextop = F8;
            GETEX(x2, 0, vex.l ? 28 : 12);
            GETGX();
            GETGY();
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
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 4; ++i) {
                    FLW(d1, wback, fixedaddress + 4 * i);
                    if (!BOX64ENV(dynarec_fastnan)) {
                        FEQS(x3, d1, d1);
                        BNEZ(x3, 4 + 2 * 4); // isnan(d1)? copy it
                        FSW(d1, gback, gyoffset + i * 4);
                        J(4 + 5 * 4); // continue
                    }
                    FDIVS(d1, d0, d1);
                    if (!BOX64ENV(dynarec_fastnan)) {
                        FEQS(x3, d1, d1);
                        BNEZ(x3, 4 + 4); // isnan(d1)? negate it
                        FNEGS(d1, d1);
                    }
                    FSW(d1, gback, gyoffset + 4 * i);
                }
            } else
                YMM0(gd);
            break;
        case 0x54:
            INST_NAME("VANDPS Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 24 : 8);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            LD(x3, vback, vxoffset + 0);
            LD(x4, wback, fixedaddress + 0);
            AND(x5, x3, x4);
            SD(x5, gback, gdoffset + 0);
            LD(x3, vback, vxoffset + 8);
            LD(x4, wback, fixedaddress + 8);
            AND(x5, x3, x4);
            SD(x5, gback, gdoffset + 8);
            if (vex.l) {
                GETEY();
                LD(x3, vback, vyoffset + 0);
                LD(x4, wback, fixedaddress + 0);
                AND(x5, x3, x4);
                SD(x5, gback, gyoffset + 0);
                LD(x3, vback, vyoffset + 8);
                LD(x4, wback, fixedaddress + 8);
                AND(x5, x3, x4);
                SD(x5, gback, gyoffset + 8);
            } else
                YMM0(gd);
            break;
        case 0x55:
            INST_NAME("VANDNPS Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 24 : 8);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            LD(x3, vback, vxoffset + 0);
            LD(x4, wback, fixedaddress + 0);
            NOT(x3, x3);
            AND(x5, x3, x4);
            SD(x5, gback, gdoffset + 0);
            LD(x3, vback, vxoffset + 8);
            LD(x4, wback, fixedaddress + 8);
            NOT(x3, x3);
            AND(x5, x3, x4);
            SD(x5, gback, gdoffset + 8);
            if (vex.l) {
                GETEY();
                LD(x3, vback, vyoffset + 0);
                LD(x4, wback, fixedaddress + 0);
                NOT(x3, x3);
                AND(x5, x3, x4);
                SD(x5, gback, gyoffset + 0);
                LD(x3, vback, vyoffset + 8);
                LD(x4, wback, fixedaddress + 8);
                NOT(x3, x3);
                AND(x5, x3, x4);
                SD(x5, gback, gyoffset + 8);
            } else
                YMM0(gd);
            break;
        case 0x56:
            INST_NAME("VORPS Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 24 : 8);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            LD(x3, vback, vxoffset + 0);
            LD(x4, wback, fixedaddress + 0);
            OR(x5, x3, x4);
            SD(x5, gback, gdoffset + 0);
            LD(x3, vback, vxoffset + 8);
            LD(x4, wback, fixedaddress + 8);
            OR(x5, x3, x4);
            SD(x5, gback, gdoffset + 8);
            if (vex.l) {
                GETEY();
                LD(x3, vback, vyoffset + 0);
                LD(x4, wback, fixedaddress + 0);
                OR(x5, x3, x4);
                SD(x5, gback, gyoffset + 0);
                LD(x3, vback, vyoffset + 8);
                LD(x4, wback, fixedaddress + 8);
                OR(x5, x3, x4);
                SD(x5, gback, gyoffset + 8);
            } else
                YMM0(gd);
            break;
        case 0x57:
            INST_NAME("VXORPS Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 24 : 8);
            GETGX();
            GETGY();
            GETVX();
            GETVY();
            if (vex.v == ed) {
                SD(xZR, gback, gdoffset + 0);
                SD(xZR, gback, gdoffset + 8);
            } else {
                LD(x3, vback, vxoffset + 0);
                LD(x4, wback, fixedaddress + 0);
                XOR(x5, x3, x4);
                SD(x5, gback, gdoffset + 0);
                LD(x3, vback, vxoffset + 8);
                LD(x4, wback, fixedaddress + 8);
                XOR(x5, x3, x4);
                SD(x5, gback, gdoffset + 8);
            }
            if (vex.l) {
                GETEY();
                if (vex.v == ed) {
                    YMM0(gd);
                } else {
                    LD(x3, vback, vyoffset + 0);
                    LD(x4, wback, fixedaddress + 0);
                    XOR(x5, x3, x4);
                    SD(x5, gback, gyoffset + 0);
                    LD(x3, vback, vyoffset + 8);
                    LD(x4, wback, fixedaddress + 8);
                    XOR(x5, x3, x4);
                    SD(x5, gback, gyoffset + 8);
                }
            } else
                YMM0(gd);
            break;
        case 0x5A:
            INST_NAME("VCVTPS2PD Gx, Ex");
            nextop = F8;
            GETGX();
            GETGY();
            GETEX(x2, 0, vex.l ? 12 : 4);
            s0 = fpu_get_scratch(dyn);
            s1 = fpu_get_scratch(dyn);
            FLW(s0, wback, fixedaddress);
            FLW(s1, wback, fixedaddress + 4);
            FCVTDS(s0, s0);
            FCVTDS(s1, s1);
            FSD(s0, gback, gdoffset + 0);
            FSD(s1, gback, gdoffset + 8);
            if (vex.l) {
                FLW(s0, wback, fixedaddress + 8);
                FLW(s1, wback, fixedaddress + 12);
                FCVTDS(s0, s0);
                FCVTDS(s1, s1);
                FSD(s0, gback, gyoffset + 0);
                FSD(s1, gback, gyoffset + 8);
            } else
                YMM0(gd);
            break;
        case 0xC2:
            INST_NAME("VCMPPS Gx, Vx, Ex, Ib");
            nextop = F8;
            GETEX(x2, 1, vex.l ? 28 : 12);
            GETGX();
            GETVX();
            GETGY();
            GETVY();
            u8 = F8;
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            for (int i = 0; i < 4 + (vex.l ? 4 : 0); ++i) {
                if (i == 4) { GETEY(); }
                if (i < 4) {
                    FLW(d0, vback, vxoffset + 4 * i);
                    FLW(d1, wback, fixedaddress + 4 * i);
                } else {
                    FLW(d0, vback, vyoffset + 4 * (i - 4));
                    FLW(d1, wback, fixedaddress + 4 * (i - 4));
                }

                if ((u8 & 0xf) != 0x0b && (u8 & 0xf) != 0xf) {
                    // x6 = !(isnan(d0) || isnan(d1))
                    FEQS(x4, d0, d0);
                    FEQS(x3, d1, d1);
                    AND(x6, x3, x4);
                }

                switch (u8 & 0x7) {
                    case 0:
                        FEQS(x3, d0, d1);
                        break; // Equal
                    case 1:
                        BEQ(x6, xZR, 8);
                        FLTS(x3, d0, d1);
                        break; // Less than
                    case 2:
                        BEQ(x6, xZR, 8);
                        FLES(x3, d0, d1);
                        break; // Less or equal
                    case 3:
                        if (u8 & 0x8)
                            ADDI(x3, xZR, 0);
                        else
                            XORI(x3, x6, 1);
                        break;
                    case 4:
                        FEQS(x3, d0, d1);
                        XORI(x3, x3, 1);
                        break; // Not Equal or unordered
                    case 5:
                        BEQ(x6, xZR, 12);
                        FLES(x3, d1, d0);
                        J(8);
                        ADDI(x3, xZR, 1);
                        break; // Greater or equal or unordered
                    case 6:
                        BEQ(x6, xZR, 12);
                        FLTS(x3, d1, d0);
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
                if (i < 4) {
                    SW(x3, gback, gdoffset + 4 * i);
                } else {
                    SW(x3, gback, gyoffset + 4 * (i - 4));
                }
            }
            if (!vex.l) YMM0(gd);
            break;
        default:
            DEFAULT;
    }
    return addr;
}
