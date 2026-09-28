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

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_functions.h"
#include "../dynarec_helper.h"

uintptr_t dynarec64_AVX_66_0F3A(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8, tmp8u;
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

    static const int8_t round_round[] = { RD_RNE, RD_RDN, RD_RUP, RD_RTZ };

    switch (opcode) {
        case 0x00:
        case 0x01:
            if (opcode)
                INST_NAME("VPERMPD Gx, Ex, Imm8");
            else
                INST_NAME("VPERMQ Gx, Ex, Imm8");
            nextop = F8;
            GETEX(x2, 1, vex.l ? 28 : 12);
            GETGX();
            GETGY();
            u8 = F8;
            if (!vex.l) {
                UDF();
            }
            if (MODREG && gd == ed) {
                for (int i = 0; i < 2; ++i) {
                    LD(x4, wback, fixedaddress + 8 * i);
                    SD(x4, xEmu, offsetof(x64emu_t, scratch) + 8 * i);
                    LD(x4, xEmu, offsetof(x64emu_t, ymm[ed]) + 8 * i);
                    SD(x4, xEmu, offsetof(x64emu_t, scratch) + 16 + 8 * i);
                }
                fixedaddress = offsetof(x64emu_t, scratch);
            }
            for (int i = 0; i < 2; ++i) {
                int sel = (u8 >> (2 * i)) & 3;
                if (sel < 2 || (MODREG && gd == ed))
                    LD(x4, wback, fixedaddress + 8 * sel);
                else if (MODREG)
                    LD(x4, xEmu, offsetof(x64emu_t, ymm[ed]) + 8 * (sel - 2));
                else
                    LD(x4, wback, fixedaddress + 16 + 8 * (sel - 2));
                SD(x4, gback, gdoffset + 8 * i);
            }
            for (int i = 0; i < 2; ++i) {
                int sel = (u8 >> (2 * (2 + i))) & 3;
                if (sel < 2 || (MODREG && gd == ed))
                    LD(x4, wback, fixedaddress + 8 * sel);
                else if (MODREG)
                    LD(x4, xEmu, offsetof(x64emu_t, ymm[ed]) + 8 * (sel - 2));
                else
                    LD(x4, wback, fixedaddress + 16 + 8 * (sel - 2));
                SD(x4, gback, gyoffset + 8 * i);
            }
            break;
        case 0x1D:
            INST_NAME("VCVTPS2PH Ex, Gx, Ib");
            nextop = F8;
            GETGX();
            GETGY();
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                sse_forget_reg(dyn, ninst, x3, ed);
                ADDI(x3, xEmu, offsetof(x64emu_t, xmm[ed]));
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 0, 1);
                ADDI(x3, wback, fixedaddress);
            }
            SD(x3, xEmu, offsetof(x64emu_t, scratch) + 48);
            LD(x4, gback, gdoffset + 0);
            SD(x4, xEmu, offsetof(x64emu_t, scratch) + 0);
            LD(x4, gback, gdoffset + 8);
            SD(x4, xEmu, offsetof(x64emu_t, scratch) + 8);
            if (vex.l) {
                LD(x4, gback, gyoffset + 0);
                SD(x4, xEmu, offsetof(x64emu_t, scratch) + 16);
                LD(x4, gback, gyoffset + 8);
                SD(x4, xEmu, offsetof(x64emu_t, scratch) + 24);
            }
            u8 = F8;
            if (u8 & 4) {
                LWU(x1, xEmu, offsetof(x64emu_t, mxcsr));
                SRLI(x1, x1, 13);
                ANDI(x1, x1, 3);
            } else {
                ADDI(x1, xZR, u8 & 3);
            }
            ADDI(x2, xEmu, offsetof(x64emu_t, scratch) + 32);
            ADDI(x3, xEmu, offsetof(x64emu_t, scratch));
            ADDI(x4, xZR, vex.l ? 8 : 4);
            CALL4(const_native_cvtps2ph, -1, x2, x3, x4, x1);
            LD(x3, xEmu, offsetof(x64emu_t, scratch) + 48);
            LD(x4, xEmu, offsetof(x64emu_t, scratch) + 32);
            SD(x4, x3, 0);
            if (vex.l) {
                LD(x4, xEmu, offsetof(x64emu_t, scratch) + 40);
                SD(x4, x3, 8);
            }
            if (MODREG) {
                if (!vex.l) {
                    SD(xZR, x3, 8);
                }
                YMM0(ed);
            }
            if (!MODREG) {
                SMWRITE();
            }
            break;
        case 0x08:
            INST_NAME("VROUNDPS Gx, Ex, Ib");
            nextop = F8;
            GETGX();
            GETGY();
            GETEX(x2, 1, vex.l ? 28 : 12);
            u8 = F8;
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            d2 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            MOV64x(x3, 1ULL << __FLT_MANT_DIG__);
            FCVTSW(d1, x3, RD_RTZ);
            if (MODREG) {
                LD(x4, wback, fixedaddress + 0);
                LD(x5, wback, fixedaddress + 8);
                SD(x4, xEmu, offsetof(x64emu_t, scratch) + 0);
                SD(x5, xEmu, offsetof(x64emu_t, scratch) + 8);
                if (vex.l) {
                    LD(x4, xEmu, offsetof(x64emu_t, ymm) + ed * 16 + 0);
                    LD(x5, xEmu, offsetof(x64emu_t, ymm) + ed * 16 + 8);
                    SD(x4, xEmu, offsetof(x64emu_t, scratch) + 16);
                    SD(x5, xEmu, offsetof(x64emu_t, scratch) + 24);
                }
                ADDI(x1, xEmu, offsetof(x64emu_t, scratch));
            } else {
                ADDI(x1, wback, fixedaddress);
            }
            ADDI(x2, xEmu, offsetof(x64emu_t, scratch) + 32);
            MOV32w(x6, vex.l ? 8 : 4);
            MARK;
            FLW(d0, x1, 0);
            FABSS(v1, d0);
            FLTS(x3, v1, d1);
            FEQS(x4, d0, d0);
            AND(x3, x3, x4);
            BNEZ_MARK2(x3);
            FEQS(x3, d0, d0);
            XORI(x3, x3, 1);
            SLLI(x3, x3, 22);
            FMVXW(x4, d0);
            OR(x4, x4, x3);
            FMVWX(d2, x4);
            B_MARK3_nocond;
            MARK2;
            FMVXW(x3, d0);
            if (u8 & 4) {
                tmp8u = sse_setround(dyn, ninst, x4, x5);
                FCVTWS(x5, d0, RD_DYN);
                FCVTSW(d2, x5, RD_RTZ);
                x87_restoreround(dyn, ninst, tmp8u);
            } else {
                FCVTWS(x5, d0, round_round[u8 & 3]);
                FCVTSW(d2, x5, RD_RTZ);
            }
            SEQZ(x4, x5);
            SLLI(x4, x4, 31);
            AND(x3, x3, x4);
            FMVXW(x4, d2);
            OR(x4, x4, x3);
            FMVWX(d2, x4);
            MARK3;
            FSW(d2, x2, 0);
            ADDI(x1, x1, 4);
            ADDI(x2, x2, 4);
            ADDI(x6, x6, -1);
            BNEZ(x6, GETMARK - (dyn->native_size));
            LD(x4, xEmu, offsetof(x64emu_t, scratch) + 32 + 0);
            LD(x5, xEmu, offsetof(x64emu_t, scratch) + 32 + 8);
            SD(x4, gback, gdoffset + 0);
            SD(x5, gback, gdoffset + 8);
            if (vex.l) {
                LD(x4, xEmu, offsetof(x64emu_t, scratch) + 32 + 16);
                LD(x5, xEmu, offsetof(x64emu_t, scratch) + 32 + 24);
                SD(x4, gback, gyoffset + 0);
                SD(x5, gback, gyoffset + 8);
            } else
                YMM0(gd);
            break;
        case 0x09:
            INST_NAME("VROUNDPD Gx, Ex, Ib");
            nextop = F8;
            GETGX();
            GETGY();
            GETEX(x2, 1, vex.l ? 24 : 8);
            u8 = F8;
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            MOV64x(x3, 1ULL << __DBL_MANT_DIG__);
            FCVTDL(d1, x3, RD_RTZ);
            if (MODREG) {
                LD(x4, wback, fixedaddress + 0);
                LD(x5, wback, fixedaddress + 8);
                SD(x4, xEmu, offsetof(x64emu_t, scratch) + 0);
                SD(x5, xEmu, offsetof(x64emu_t, scratch) + 8);
                if (vex.l) {
                    LD(x4, xEmu, offsetof(x64emu_t, ymm) + ed * 16 + 0);
                    LD(x5, xEmu, offsetof(x64emu_t, ymm) + ed * 16 + 8);
                    SD(x4, xEmu, offsetof(x64emu_t, scratch) + 16);
                    SD(x5, xEmu, offsetof(x64emu_t, scratch) + 24);
                }
                ADDI(x1, xEmu, offsetof(x64emu_t, scratch));
            } else {
                ADDI(x1, wback, fixedaddress);
            }
            ADDI(x2, xEmu, offsetof(x64emu_t, scratch) + 32);
            MOV32w(x6, vex.l ? 4 : 2);
            MARK;
            FLD(d0, x1, 0);
            FEQD(x4, d0, d0);
            BNEZ(x4, BOX64ENV(dynarec_fastnan) ? 8 : (7 * 4));
            if (!BOX64ENV(dynarec_fastnan)) {
                FMVXD(x3, d0);
                ADDI(x4, xZR, 1);
                SLLI(x4, x4, 51);
                OR(x3, x3, x4);
                FMVDX(d0, x3);
            }
            B_MARK2_nocond;
            FABSD(v1, d0);
            FLTD(x4, v1, d1);
            BNEZ(x4, 8);
            B_MARK2_nocond;
            FMVXD(x3, d0);
            if (u8 & 4) {
                tmp8u = sse_setround(dyn, ninst, x4, x5);
                FCVTLD(x5, d0, RD_DYN);
                FCVTDL(d0, x5, RD_RTZ);
                x87_restoreround(dyn, ninst, tmp8u);
            } else {
                FCVTLD(x5, d0, round_round[u8 & 3]);
                FCVTDL(d0, x5, RD_RTZ);
            }
            SEQZ(x4, x5);
            SLLI(x4, x4, 63);
            AND(x3, x3, x4);
            FMVXD(x4, d0);
            OR(x4, x4, x3);
            FMVDX(d0, x4);
            MARK2;
            FSD(d0, x2, 0);
            ADDI(x1, x1, 8);
            ADDI(x2, x2, 8);
            ADDI(x6, x6, -1);
            BNEZ(x6, GETMARK - (dyn->native_size));
            LD(x4, xEmu, offsetof(x64emu_t, scratch) + 32 + 0);
            LD(x5, xEmu, offsetof(x64emu_t, scratch) + 32 + 8);
            SD(x4, gback, gdoffset + 0);
            SD(x5, gback, gdoffset + 8);
            if (vex.l) {
                LD(x4, xEmu, offsetof(x64emu_t, scratch) + 32 + 16);
                LD(x5, xEmu, offsetof(x64emu_t, scratch) + 32 + 24);
                SD(x4, gback, gyoffset + 0);
                SD(x5, gback, gyoffset + 8);
            } else
                YMM0(gd);
            break;
        case 0x0D:
            INST_NAME("VBLENDPD Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGX();
            GETEX(x2, 1, vex.l ? 30 : 14);
            GETVX();
            GETVY();
            GETGY();
            u8 = F8;
            for (int i = 0; i < 2; ++i) {
                if (u8 & (1 << i)) {
                    if (gd != ed) {
                        LD(x3, wback, fixedaddress + 8 * i);
                        SD(x3, gback, gdoffset + 8 * i);
                    }
                } else if (gd != vex.v) {
                    LD(x3, vback, vxoffset + 8 * i);
                    SD(x3, gback, gdoffset + 8 * i);
                }
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 2; ++i) {
                    if (u8 & (1 << (i + 2))) {
                        if (gd != ed) {
                            LD(x3, wback, fixedaddress + 8 * i);
                            SD(x3, gback, gyoffset + 8 * i);
                        }
                    } else if (gd != vex.v) {
                        LD(x3, vback, vyoffset + 8 * i);
                        SD(x3, gback, gyoffset + 8 * i);
                    }
                }
            } else
                YMM0(gd);
            break;
        case 0x0E:
            INST_NAME("VPBLENDW Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGX();
            GETEX(x2, 1, vex.l ? 30 : 14);
            GETVX();
            GETVY();
            GETGY();
            u8 = F8;
            for (int i = 0; i < 8; ++i) {
                if (u8 & (1 << i)) {
                    if (gd != ed) {
                        LHU(x3, wback, fixedaddress + 2 * i);
                        SH(x3, gback, gdoffset + 2 * i);
                    }
                } else if (gd != vex.v) {
                    LHU(x3, vback, vxoffset + 2 * i);
                    SH(x3, gback, gdoffset + 2 * i);
                }
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 8; ++i) {
                    if (u8 & (1 << i)) {
                        if (gd != ed) {
                            LHU(x3, wback, fixedaddress + 2 * i);
                            SH(x3, gback, gyoffset + 2 * i);
                        }
                    } else if (gd != vex.v) {
                        LHU(x3, vback, vyoffset + 2 * i);
                        SH(x3, gback, gyoffset + 2 * i);
                    }
                }
            } else
                YMM0(gd);
            break;
        case 0x0F:
            INST_NAME("VPALIGNR Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGX();
            GETEX(x2, 1, vex.l ? 24 : 8);
            GETVX();
            GETVY();
            GETGY();
            u8 = F8;
            if (u8 > 31) {
                SD(xZR, gback, gdoffset + 0);
                SD(xZR, gback, gdoffset + 8);
            } else if (u8 > 23) {
                LD(x5, vback, vxoffset + 8);
                if (u8 > 24) {
                    SRLI(x5, x5, 8 * (u8 - 24));
                }
                SD(x5, gback, gdoffset + 0);
                SD(xZR, gback, gdoffset + 8);
            } else if (u8 > 15) {
                if (u8 > 16) {
                    LD(x5, vback, vxoffset + 8);
                    LD(x4, vback, vxoffset + 0);
                    SRLI(x3, x5, 8 * (u8 - 16)); // lower of higher 64 bits
                    SLLI(x5, x5, 8 * (24 - u8)); // higher of lower 64 bits
                    SD(x3, gback, gdoffset + 8);
                    SRLI(x4, x4, 8 * (u8 - 16)); // lower of lower 64 bits
                    OR(x4, x4, x5);              // lower 64 bits
                    SD(x4, gback, gdoffset + 0);
                } else if (gd != vex.v) {
                    LD(x4, vback, vxoffset + 0);
                    LD(x5, vback, vxoffset + 8);
                    SD(x4, gback, gdoffset + 0);
                    SD(x5, gback, gdoffset + 8);
                }
            } else if (u8 > 7) {
                if (u8 > 8) {
                    LD(x5, vback, vxoffset + 8);
                    LD(x4, vback, vxoffset + 0);
                    LD(x3, wback, fixedaddress + 8);
                    SLLI(x5, x5, 8 * (16 - u8)); // higher of higher 64 bits
                    SRLI(x1, x4, 8 * (u8 - 8));  // lower of higher 64 bits
                    SLLI(x4, x4, 8 * (16 - u8)); // higher of lower 64 bits
                    OR(x5, x1, x5);              // higher 64 bits
                    SRLI(x3, x3, 8 * (u8 - 8));  // lower of lower 64 bits
                    SD(x5, gback, gdoffset + 8);
                    OR(x4, x4, x3); // lower 64 bits
                    SD(x4, gback, gdoffset + 0);
                } else {
                    LD(x5, vback, vxoffset + 0);
                    LD(x4, wback, fixedaddress + 8);
                    SD(x5, gback, gdoffset + 8);
                    SD(x4, gback, gdoffset + 0);
                }
            } else {
                if (u8 > 0) {
                    LD(x5, vback, vxoffset + 0);
                    LD(x4, wback, fixedaddress + 8);
                    LD(x3, wback, fixedaddress + 0);
                    SLLI(x5, x5, 8 * (8 - u8)); // higher of higher 64 bits
                    SRLI(x1, x4, 8 * (u8 - 0)); // lower of higher 64 bits
                    SLLI(x4, x4, 8 * (8 - u8)); // higher of lower 64 bits
                    OR(x5, x1, x5);             // higher 64 bits
                    SRLI(x3, x3, 8 * (u8 - 0)); // lower of lower 64 bits
                    SD(x5, gback, gdoffset + 8);
                    OR(x4, x4, x3); // lower 64 bits
                    SD(x4, gback, gdoffset + 0);
                } else {
                    LD(x5, wback, fixedaddress + 8);
                    LD(x4, wback, fixedaddress + 0);
                    SD(x5, gback, gdoffset + 8);
                    SD(x4, gback, gdoffset + 0);
                }
            }
            if (vex.l) {
                GETEY();
                if (u8 > 31) {
                    YMM0(gd);
                } else if (u8 > 23) {
                    LD(x5, vback, vyoffset + 8);
                    if (u8 > 24) {
                        SRLI(x5, x5, 8 * (u8 - 24));
                    }
                    SD(x5, gback, gyoffset + 0);
                    SD(xZR, gback, gyoffset + 8);
                } else if (u8 > 15) {
                    if (u8 > 16) {
                        LD(x5, vback, vyoffset + 8);
                        LD(x4, vback, vyoffset + 0);
                        SRLI(x3, x5, 8 * (u8 - 16)); // lower of higher 64 bits
                        SLLI(x5, x5, 8 * (24 - u8)); // higher of lower 64 bits
                        SD(x3, gback, gyoffset + 8);
                        SRLI(x4, x4, 8 * (u8 - 16)); // lower of lower 64 bits
                        OR(x4, x4, x5);              // lower 64 bits
                        SD(x4, gback, gyoffset + 0);
                    } else if (gd != vex.v) {
                        LD(x4, vback, vyoffset + 0);
                        LD(x5, vback, vyoffset + 8);
                        SD(x4, gback, gyoffset + 0);
                        SD(x5, gback, gyoffset + 8);
                    }
                } else if (u8 > 7) {
                    if (u8 > 8) {
                        LD(x5, vback, vyoffset + 8);
                        LD(x4, vback, vyoffset + 0);
                        LD(x3, wback, fixedaddress + 8);
                        SLLI(x5, x5, 8 * (16 - u8)); // higher of higher 64 bits
                        SRLI(x1, x4, 8 * (u8 - 8));  // lower of higher 64 bits
                        SLLI(x4, x4, 8 * (16 - u8)); // higher of lower 64 bits
                        OR(x5, x1, x5);              // higher 64 bits
                        SRLI(x3, x3, 8 * (u8 - 8));  // lower of lower 64 bits
                        SD(x5, gback, gyoffset + 8);
                        OR(x4, x4, x3); // lower 64 bits
                        SD(x4, gback, gyoffset + 0);
                    } else {
                        LD(x5, vback, vyoffset + 0);
                        LD(x4, wback, fixedaddress + 8);
                        SD(x5, gback, gyoffset + 8);
                        SD(x4, gback, gyoffset + 0);
                    }
                } else {
                    if (u8 > 0) {
                        LD(x5, vback, vyoffset + 0);
                        LD(x4, wback, fixedaddress + 8);
                        LD(x3, wback, fixedaddress + 0);
                        SLLI(x5, x5, 8 * (8 - u8)); // higher of higher 64 bits
                        SRLI(x1, x4, 8 * (u8 - 0)); // lower of higher 64 bits
                        SLLI(x4, x4, 8 * (8 - u8)); // higher of lower 64 bits
                        OR(x5, x1, x5);             // higher 64 bits
                        SRLI(x3, x3, 8 * (u8 - 0)); // lower of lower 64 bits
                        SD(x5, gback, gyoffset + 8);
                        OR(x4, x4, x3); // lower 64 bits
                        SD(x4, gback, gyoffset + 0);
                    } else {
                        LD(x5, wback, fixedaddress + 8);
                        LD(x4, wback, fixedaddress + 0);
                        SD(x5, gback, gyoffset + 8);
                        SD(x4, gback, gyoffset + 0);
                    }
                }
            } else
                YMM0(gd);
            break;
        case 0x14:
            INST_NAME("VPEXTRB Gd, Ex, Ib");
            nextop = F8;
            GETGX();
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                wback = 0;
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 1, 1);
                ed = x1;
            }
            u8 = F8;
            LBU(ed, gback, gdoffset + (u8 & 15));
            if (wback) {
                SB(ed, wback, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0x15:
            INST_NAME("VPEXTRW Ew, Gx, Ib");
            nextop = F8;
            GETGX();
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                wback = 0;
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 1, 1);
                ed = x1;
            }
            u8 = F8;
            LHU(ed, gback, gdoffset + 2 * (u8 & 7));
            if (wback) {
                SH(ed, wback, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0x16:
            if (rex.w) {
                INST_NAME("VPEXTRQ Ed, Gx, Ib");
            } else {
                INST_NAME("VPEXTRD Ed, Gx, Ib");
            }
            nextop = F8;
            GETGX();
            GETED(1);
            u8 = F8;
            if (rex.w) {
                LD(ed, gback, gdoffset + 8 * (u8 & 0x1));
                if (!MODREG) {
                    SD(ed, wback, fixedaddress);
                    SMWRITE2();
                }
            } else {
                if (MODREG) {
                    LWU(ed, gback, gdoffset + 4 * (u8 & 0x3));
                } else {
                    LW(ed, gback, gdoffset + 4 * (u8 & 0x3));
                    SW(ed, wback, fixedaddress);
                    SMWRITE2();
                }
            }
            break;
        case 0x17:
            INST_NAME("VEXTRACTPS Ed, Gx, Ib");
            nextop = F8;
            GETGX();
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                wback = 0;
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 1, 1);
                ed = x1;
            }
            u8 = F8;
            LWU(ed, gback, gdoffset + 4 * (u8 & 3));
            if (wback) {
                SW(ed, wback, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0x18:
        case 0x38:
            if (opcode == 0x18) {
                INST_NAME("VINSERTF128 Gx, Vx, Ex, imm8");
            } else {
                INST_NAME("VINSERTI128 Gx, Vx, Ex, imm8");
            }
            nextop = F8;
            GETEX(x1, 1, 8);
            GETGX();
            GETVX();
            GETGY();
            GETVY();
            u8 = F8;
            if (u8 & 1) {
                LD(x4, wback, fixedaddress + 0);
                LD(x5, wback, fixedaddress + 8);
                SD(x4, gback, gyoffset + 0);
                SD(x5, gback, gyoffset + 8);
                if (gd != vex.v) {
                    LD(x4, vback, vxoffset + 0);
                    LD(x5, vback, vxoffset + 8);
                    SD(x4, gback, gdoffset + 0);
                    SD(x5, gback, gdoffset + 8);
                }
            } else {
                LD(x4, wback, fixedaddress + 0);
                LD(x5, wback, fixedaddress + 8);
                SD(x4, gback, gdoffset + 0);
                SD(x5, gback, gdoffset + 8);
                if (gd != vex.v) {
                    LD(x4, vback, vyoffset + 0);
                    LD(x5, vback, vyoffset + 8);
                    SD(x4, gback, gyoffset + 0);
                    SD(x5, gback, gyoffset + 8);
                }
            }
            break;
        case 0x19:
        case 0x39:
            if (opcode == 0x19) {
                INST_NAME("VEXTRACTF128 Ex, Gx, imm8");
            } else {
                INST_NAME("VEXTRACTI128 Ex, Gx, imm8");
            }
            nextop = F8;
            GETEX(x1, 1, 8);
            GETGX();
            GETGY();
            u8 = F8;
            if (u8 & 1) {
                LD(x4, gback, gyoffset + 0);
                LD(x5, gback, gyoffset + 8);
                SD(x4, wback, fixedaddress + 0);
                SD(x5, wback, fixedaddress + 8);
            } else {
                LD(x4, gback, gdoffset + 0);
                LD(x5, gback, gdoffset + 8);
                SD(x4, wback, fixedaddress + 0);
                SD(x5, wback, fixedaddress + 8);
            }
            if (MODREG) {
                YMM0(ed);
            } else {
                SMWRITE2();
            }
            break;
        case 0x22:
            if (rex.w) {
                INST_NAME("VPINSRQ Gx, Vx, ED, Ib");
            } else {
                INST_NAME("VPINSRD Gx, Vx, ED, Ib");
            }
            nextop = F8;
            GETGX();
            GETED(1);
            GETGY();
            GETVX();
            u8 = F8;
            if (gd != vex.v) {
                LD(x4, vback, vxoffset + 0);
                LD(x5, vback, vxoffset + 8);
                SD(x4, gback, gdoffset + 0);
                SD(x5, gback, gdoffset + 8);
            }
            if (rex.w) {
                SD(ed, gback, gdoffset + 8 * (u8 & 0x1));
            } else {
                SW(ed, gback, gdoffset + 4 * (u8 & 0x3));
            }
            YMM0(gd);
            break;
        case 0x42:
            INST_NAME("VMPSADBW Gx, Vx, Ex, Ib");
            nextop = F8;
            GETEX(x2, 1, vex.l ? 28 : 12);
            GETVX();
            GETVY();
            GETGX();
            GETGY();
            u8 = F8;
            {
                int src = (u8 & 3) * 4;
                int dst = ((u8 >> 2) & 1) * 4;
#define VMPSAD4_SUM(acc, AB, aoff, BB, boff)  \
    do {                                      \
        ADDI(acc, xZR, 0);                    \
        for (int _k = 0; _k < 4; ++_k) {      \
            LBU(x4, AB, (aoff) + _k);         \
            LBU(x6, BB, (boff) + _k);         \
            SUBW(x4, x4, x6);                 \
            if (cpuext.zbb) {                 \
                NEG(x7, x4);                  \
                MAX(x4, x4, x7);              \
            } else {                          \
                SRAIW(x7, x4, 31);            \
                XOR(x4, x4, x7);              \
                SUBW(x4, x4, x7);             \
            }                                 \
            ADD(acc, acc, x4);                \
        }                                     \
    } while (0)
                for (int i = 0; i < 8; ++i) {
                    VMPSAD4_SUM(x5, vback, vxoffset + dst + i, wback, fixedaddress + src);
                    SH(x5, gback, gdoffset + 2 * i);
                }
#undef VMPSAD4_SUM
            }
            if (vex.l) {
                GETEY();
                {
                    int src = ((u8 >> 3) & 3) * 4;
                    int dst = ((u8 >> 5) & 1) * 4;
#define VMPSAD4_SUM(acc, AB, aoff, BB, boff)  \
    do {                                      \
        ADDI(acc, xZR, 0);                    \
        for (int _k = 0; _k < 4; ++_k) {      \
            LBU(x4, AB, (aoff) + _k);         \
            LBU(x6, BB, (boff) + _k);         \
            SUBW(x4, x4, x6);                 \
            if (cpuext.zbb) {                 \
                NEG(x7, x4);                  \
                MAX(x4, x4, x7);              \
            } else {                          \
                SRAIW(x7, x4, 31);            \
                XOR(x4, x4, x7);              \
                SUBW(x4, x4, x7);             \
            }                                 \
            ADD(acc, acc, x4);                \
        }                                     \
    } while (0)
                    for (int i = 0; i < 8; ++i) {
                        VMPSAD4_SUM(x5, vback, vyoffset + dst + i, wback, fixedaddress + src);
                        SH(x5, gback, gyoffset + 2 * i);
                    }
#undef VMPSAD4_SUM
                }
            } else
                YMM0(gd);
            break;
        case 0x44:
            INST_NAME("VPCLMULQDQ Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGX();
            GETVX();
            GETGY();
            GETVY();
            sse_forget_reg(dyn, ninst, x6, gd);
            sse_forget_reg(dyn, ninst, x6, vex.v);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                sse_forget_reg(dyn, ninst, x6, ed);
                ADDI(x3, xEmu, offsetof(x64emu_t, xmm[ed]));
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, x2, &fixedaddress, rex, NULL, 0, 1);
                if (ed != x3) MV(x3, ed);
                if (fixedaddress) ADDI(x3, x3, fixedaddress);
            }
            u8 = F8;
            SD(x3, xEmu, offsetof(x64emu_t, scratch) + 16);
            LD(x4, vback, vxoffset + 0);
            LD(x5, vback, vxoffset + 8);
            SD(x4, gback, gdoffset + 0);
            SD(x5, gback, gdoffset + 8);
            MOV32w(x1, gd);
            ADDI(x4, xZR, 0);
            MOV32w(x5, u8);
            CALL4(const_native_pclmul, -1, x1, x4, x3, x5);
            if (vex.l) {
                LD(x4, gback, gdoffset + 0);
                LD(x5, gback, gdoffset + 8);
                SD(x4, xEmu, offsetof(x64emu_t, scratch) + 0);
                SD(x5, xEmu, offsetof(x64emu_t, scratch) + 8);
                if (MODREG) {
                    ADDI(x3, xEmu, offsetof(x64emu_t, ymm[ed]));
                } else {
                    LD(x3, xEmu, offsetof(x64emu_t, scratch) + 16);
                    ADDI(x3, x3, 16);
                }
                LD(x4, vback, vyoffset + 0);
                LD(x5, vback, vyoffset + 8);
                SD(x4, gback, gdoffset + 0);
                SD(x5, gback, gdoffset + 8);
                MOV32w(x1, gd);
                ADDI(x4, xZR, 0);
                MOV32w(x5, u8);
                CALL4(const_native_pclmul, -1, x1, x4, x3, x5);
                LD(x4, gback, gdoffset + 0);
                LD(x5, gback, gdoffset + 8);
                SD(x4, gback, gyoffset + 0);
                SD(x5, gback, gyoffset + 8);
                LD(x4, xEmu, offsetof(x64emu_t, scratch) + 0);
                LD(x5, xEmu, offsetof(x64emu_t, scratch) + 8);
                SD(x4, gback, gdoffset + 0);
                SD(x5, gback, gdoffset + 8);
            } else
                YMM0(gd);
            break;
        case 0x06:
            INST_NAME("VPERM2F128 Gx, Vx, Ex, Ib");
            nextop = F8;
            if (!vex.l) {
                DEFAULT;
            }
            GETEX(x2, 1, 28);
            GETGX();
            GETVX();
            GETVY();
            GETGY();
            u8 = F8;
            {
                int64_t eyoffset = MODREG ? offsetof(x64emu_t, ymm[ed]) : fixedaddress + 16;
                const int scratch[4] = {x4, x5, x6, x7};
                for (int i = 0; i < 2; ++i) {
                    int control = i ? (u8 >> 4) : u8;
                    int sel = control & 3;
                    if (control & 8) {
                        MV(scratch[2 * i + 0], xZR);
                        MV(scratch[2 * i + 1], xZR);
                    } else {
                        for (int j = 0; j < 2; ++j) {
                            if (sel == 0)
                                LD(scratch[2 * i + j], vback, vxoffset + 8 * j);
                            else if (sel == 1)
                                LD(scratch[2 * i + j], vback, vyoffset + 8 * j);
                            else if (sel == 2)
                                LD(scratch[2 * i + j], wback, fixedaddress + 8 * j);
                            else
                                LD(scratch[2 * i + j], wback, eyoffset + 8 * j);
                        }
                    }
                }
                for (int i = 0; i < 2; ++i)
                    for (int j = 0; j < 2; ++j)
                        SD(scratch[2 * i + j], gback, (i ? gyoffset : gdoffset) + 8 * j);
            }
            break;
        case 0x46:
            INST_NAME("VPERM2I128 Gx, Vx, Ex, Ib");
            nextop = F8;
            if (!vex.l) {
                DEFAULT;
            }
            GETEX(x2, 1, 28);
            GETGX();
            GETVX();
            GETVY();
            GETGY();
            u8 = F8;
            {
                int64_t eyoffset = MODREG ? offsetof(x64emu_t, ymm[ed]) : fixedaddress + 16;
                const int scratch[4] = {x4, x5, x6, x7};
                for (int i = 0; i < 2; ++i) {
                    int control = i ? (u8 >> 4) : u8;
                    int sel = control & 3;
                    if (control & 8) {
                        MV(scratch[2 * i + 0], xZR);
                        MV(scratch[2 * i + 1], xZR);
                    } else {
                        for (int j = 0; j < 2; ++j) {
                            if (sel == 0)
                                LD(scratch[2 * i + j], vback, vxoffset + 8 * j);
                            else if (sel == 1)
                                LD(scratch[2 * i + j], vback, vyoffset + 8 * j);
                            else if (sel == 2)
                                LD(scratch[2 * i + j], wback, fixedaddress + 8 * j);
                            else
                                LD(scratch[2 * i + j], wback, eyoffset + 8 * j);
                        }
                    }
                }
                for (int i = 0; i < 2; ++i)
                    for (int j = 0; j < 2; ++j)
                        SD(scratch[2 * i + j], gback, (i ? gyoffset : gdoffset) + 8 * j);
            }
            break;
        case 0x4A:
            INST_NAME("VBLENDVPS Gx, Vx, Ex, XMMImm8");
            nextop = F8;
            u8 = geted_ib(dyn, addr, ninst, nextop) >> 4;
            sse_forget_reg(dyn, ninst, x3, u8);
            GETEX(x1, 1, vex.l ? 28 : 12);
            GETGX();
            GETVX();
            GETGY();
            GETVY();
            F8;
            for (int i = 0; i < 4; ++i) {
                LW(x3, xEmu, offsetof(x64emu_t, xmm) + u8 * 16 + i * 4);
                LWU(x4, wback, fixedaddress + i * 4);
                LWU(x5, vback, vxoffset + i * 4);
                BGE(x3, xZR, 4 + 4);
                MV(x5, x4);
                SW(x5, gback, gdoffset + i * 4);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 4; ++i) {
                    LW(x3, xEmu, offsetof(x64emu_t, ymm) + u8 * 16 + i * 4);
                    LWU(x4, wback, fixedaddress + i * 4);
                    LWU(x5, vback, vyoffset + i * 4);
                    BGE(x3, xZR, 4 + 4);
                    MV(x5, x4);
                    SW(x5, gback, gyoffset + i * 4);
                }
            } else
                YMM0(gd);
            break;
        case 0x4B:
            INST_NAME("VBLENDVPD Gx, Vx, Ex, XMMImm8");
            nextop = F8;
            u8 = geted_ib(dyn, addr, ninst, nextop) >> 4;
            sse_forget_reg(dyn, ninst, x3, u8);
            GETEX(x1, 1, vex.l ? 24 : 8);
            GETGX();
            GETVX();
            GETGY();
            GETVY();
            F8;
            for (int i = 0; i < 2; ++i) {
                LD(x3, xEmu, offsetof(x64emu_t, xmm) + u8 * 16 + i * 8);
                LD(x4, wback, fixedaddress + i * 8);
                LD(x5, vback, vxoffset + i * 8);
                BGE(x3, xZR, 4 + 4);
                MV(x5, x4);
                SD(x5, gback, gdoffset + i * 8);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 2; ++i) {
                    LD(x3, xEmu, offsetof(x64emu_t, ymm) + u8 * 16 + i * 8);
                    LD(x4, wback, fixedaddress + i * 8);
                    LD(x5, vback, vyoffset + i * 8);
                    BGE(x3, xZR, 4 + 4);
                    MV(x5, x4);
                    SD(x5, gback, gyoffset + i * 8);
                }
            } else
                YMM0(gd);
            break;
        case 0x4C:
            INST_NAME("VPBLENDVB Gx, Vx, Ex, XMMImm8");
            nextop = F8;
            u8 = geted_ib(dyn, addr, ninst, nextop) >> 4;
            sse_forget_reg(dyn, ninst, x3, u8);
            GETEX(x1, 1, vex.l ? 31 : 15);
            GETGX();
            GETVX();
            GETGY();
            GETVY();
            F8;
            for (int i = 0; i < 16; ++i) {
                LB(x3, xEmu, offsetof(x64emu_t, xmm) + u8 * 16 + i);
                LBU(x4, wback, fixedaddress + i);
                LBU(x5, vback, vxoffset + i);
                BGE(x3, xZR, 4 + 4);
                MV(x5, x4);
                SB(x5, gback, gdoffset + i);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 16; ++i) {
                    LB(x3, xEmu, offsetof(x64emu_t, ymm) + u8 * 16 + i);
                    LBU(x4, wback, fixedaddress + i);
                    LBU(x5, vback, vyoffset + i);
                    BGE(x3, xZR, 4 + 4);
                    MV(x5, x4);
                    SB(x5, gback, gyoffset + i);
                }
            } else
                YMM0(gd);
            break;
        case 0x60:
            INST_NAME("VPCMPESTRM Gx, Ex, Ib");
            nextop = F8;
            if (vex.l) {
                DEFAULT;
            }
            SETFLAGS(X_ALL, SF_SET_DF, NAT_FLAGS_NOFUSION);
            gd = ((nextop & 0x38) >> 3) + (rex.r << 3);
            sse_reflect_reg(dyn, ninst, x6, gd);
            ADDI(x3, xEmu, offsetof(x64emu_t, xmm[gd]));
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                sse_reflect_reg(dyn, ninst, x6, ed);
                ADDI(x1, xEmu, offsetof(x64emu_t, xmm[ed]));
                ed = x1;
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 0, 1);
            }
            SEXT_W(x2, xRDX);
            SEXT_W(x4, xRAX);
            u8 = F8;
            ADDI(x5, xZR, u8);
            CALL6(const_sse42_compare_string_explicit_len, x1, ed, x2, x3, x4, x5, 0);
            ZEROUP(x1);
            sse_forget_reg(dyn, ninst, x3, 0);
            if (u8 & 0b1000000) {
                if (u8 & 1) {
                    for (int i = 0; i < 8; ++i) {
                        BEXTI(x2, x1, i);
                        SUB(x2, xZR, x2);
                        SH(x2, xEmu, offsetof(x64emu_t, xmm[0]) + 2 * i);
                    }
                } else {
                    for (int i = 0; i < 16; ++i) {
                        BEXTI(x2, x1, i);
                        SUB(x2, xZR, x2);
                        SB(x2, xEmu, offsetof(x64emu_t, xmm[0]) + i);
                    }
                }
            } else {
                SD(xZR, xEmu, offsetof(x64emu_t, xmm[0]));
                SD(xZR, xEmu, offsetof(x64emu_t, xmm[0]) + 8);
                SH(x1, xEmu, offsetof(x64emu_t, xmm[0]));
            }
            YMM0(0);
            break;
        case 0x61:
            INST_NAME("VPCMPESTRI Gx, Ex, Ib");
            nextop = F8;
            if (vex.l) {
                DEFAULT;
            }
            SETFLAGS(X_ALL, SF_SET_DF, NAT_FLAGS_NOFUSION);
            gd = ((nextop & 0x38) >> 3) + (rex.r << 3);
            sse_reflect_reg(dyn, ninst, x6, gd);
            ADDI(x3, xEmu, offsetof(x64emu_t, xmm[gd]));
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                sse_reflect_reg(dyn, ninst, x6, ed);
                ADDI(x1, xEmu, offsetof(x64emu_t, xmm[ed]));
                ed = x1;
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 0, 1);
            }
            SEXT_W(x2, xRDX);
            SEXT_W(x4, xRAX);
            u8 = F8;
            ADDI(x5, xZR, u8);
            CALL6(const_sse42_compare_string_explicit_len, x1, ed, x2, x3, x4, x5, 0);
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
                CTZxw(xRCX, x1, 0, x2, x3);
            }
            break;
        case 0x63:
            INST_NAME("VPCMPISTRI Gx, Ex, Ib");
            nextop = F8;
            if (vex.l) {
                DEFAULT;
            }
            SETFLAGS(X_ALL, SF_SET_DF, NAT_FLAGS_NOFUSION);
            gd = ((nextop & 0x38) >> 3) + (rex.r << 3);
            sse_reflect_reg(dyn, ninst, x6, gd);
            ADDI(x2, xEmu, offsetof(x64emu_t, xmm[gd]));
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                sse_reflect_reg(dyn, ninst, x6, ed);
                ADDI(x1, xEmu, offsetof(x64emu_t, xmm[ed]));
                ed = x1;
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x5, &fixedaddress, rex, NULL, 0, 1);
            }
            u8 = F8;
            MOV32w(x3, u8);
            CALL4(const_sse42_compare_string_implicit_len, x1, ed, x2, x3, 0);
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
                CTZxw(xRCX, x1, 0, x2, x3);
            }
            break;
        default:
            DEFAULT;
    }
    return addr;
}
