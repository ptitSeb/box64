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
            } else {
                SD(xZR, gback, gyoffset);
                SD(xZR, gback, gyoffset + 8);
            }
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
            } else if (MODREG) {
                GETEY();
                SD(xZR, wback, fixedaddress + 0);
                SD(xZR, wback, fixedaddress + 8);
            }
            if (!MODREG) SMWRITE2();
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
            } else {
                SD(xZR, gback, gyoffset + 0);
                SD(xZR, gback, gyoffset + 8);
            }
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
            } else {
                SD(xZR, gback, gyoffset);
                SD(xZR, gback, gyoffset + 8);
            }
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
            } else {
                SD(xZR, gback, gyoffset);
                SD(xZR, gback, gyoffset + 8);
            }
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
            } else {
                SD(xZR, gback, gyoffset);
                SD(xZR, gback, gyoffset + 8);
            }
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
            } else {
                SD(xZR, gback, gyoffset);
                SD(xZR, gback, gyoffset + 8);
            }
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
            } else {
                SD(xZR, gback, gyoffset + 0);
                SD(xZR, gback, gyoffset + 8);
            }
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
            SD(xZR, gback, gyoffset);
            SD(xZR, gback, gyoffset + 8);
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
            } else {
                SD(xZR, gback, gyoffset);
                SD(xZR, gback, gyoffset + 8);
            }
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
            } else if (MODREG) {
                GETEY();
                SD(xZR, wback, fixedaddress);
                SD(xZR, wback, fixedaddress + 8);
            }
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
            } else {
                SD(xZR, gback, gyoffset + 0);
                SD(xZR, gback, gyoffset + 8);
            }
            break;
        default:
            DEFAULT;
    }
    return addr;
}
