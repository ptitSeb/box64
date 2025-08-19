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

uintptr_t dynarec64_AVX_66_0F38(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
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
        case 0x00:
            INST_NAME("VPSHUFB Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 31 : 15);
            GETGX();
            GETVX();
            GETGY();
            GETVY();

            if (gd == vex.v) {
                ADDI(x5, xEmu, offsetof(x64emu_t, scratch));
                LD(x3, vback, vxoffset + 0);
                LD(x4, vback, vxoffset + 8);
                SD(x3, x5, 0);
                SD(x4, x5, 8);
                vback = x5;
                vxoffset = 0;
            }

            for (int i = 0; i < 16; ++i) {
                LBU(x3, wback, fixedaddress + i);
                ANDI(x4, x3, 128);
                BEQZ(x4, 4 + 4 * 2);
                SB(xZR, gback, gdoffset + i);
                J(4 + 4 * 4); // continue
                ANDI(x4, x3, 15);
                ADD(x4, x4, vback);
                LBU(x4, x4, vxoffset);
                SB(x4, gback, gdoffset + i);
            }

            if (vex.l) {
                GETEY();
                if (gd == vex.v) {
                    LD(x3, vback, vyoffset + 0);
                    LD(x4, vback, vyoffset + 8);
                    SD(x3, x5, 0);
                    SD(x4, x5, 8);
                    vback = x5;
                    vyoffset = 0;
                }
                for (int i = 0; i < 16; ++i) {
                    LBU(x3, wback, fixedaddress + i);
                    ANDI(x4, x3, 128);
                    BEQZ(x4, 4 + 4 * 2);
                    SB(xZR, gback, gdoffset + i);
                    J(4 + 4 * 4); // continue
                    ANDI(x4, x3, 15);
                    ADD(x4, x4, vback);
                    LBU(x4, x4, vxoffset);
                    SB(x4, gback, gdoffset + i);
                }
            } else {
                SD(xZR, gback, gyoffset + 0);
                SD(xZR, gback, gyoffset + 8);
            }
            break;
        case 0x01:
        case 0x03:
            if (opcode == 0x01)
                INST_NAME("VPHADDW Gx, Vx, Ex");
            else
                INST_NAME("VPHADDSW Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 30 : 14);
            GETGX();
            GETVX();
            GETGY();
            GETVY();
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
                // GX->sw[i] = VX->sw[i*2+0]+VX->sw[i*2+1];
                LH(x3, vback, vxoffset + 2 * (i * 2 + 0));
                LH(x4, vback, vxoffset + 2 * (i * 2 + 1));
                ADDW(x3, x3, x4);
                if (opcode == 0x03) SAT16(x3, x6);
                SH(x3, gback, gdoffset + 2 * i);
            }
            if (MODREG && ed == vex.v) {
                // GX->q[1] = GX->q[0];
                LD(x3, gback, gdoffset + 0);
                SD(x3, gback, gdoffset + 8);
            } else {
                for (int i = 0; i < 4; ++i) {
                    // GX->sw[4+i] = EX->sw[i*2+0]+EX->sw[i*2+1];
                    LH(x3, wback, fixedaddress + 2 * (i * 2 + 0));
                    LH(x4, wback, fixedaddress + 2 * (i * 2 + 1));
                    ADDW(x3, x3, x4);
                    if (opcode == 0x03) SAT16(x3, x6);
                    SH(x3, gback, gdoffset + 2 * (4 + i));
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
                    // GY->sw[i] = VY->sw[i*2+0]+VY->sw[i*2+1];
                    LH(x3, vback, vyoffset + 2 * (i * 2 + 0));
                    LH(x4, vback, vyoffset + 2 * (i * 2 + 1));
                    ADDW(x3, x3, x4);
                    if (opcode == 0x03) SAT16(x3, x6);
                    SH(x3, gback, gyoffset + 2 * i);
                }
                if (MODREG && ed == vex.v) {
                    // GY->q[1] = GY->q[0];
                    LD(x3, gback, gyoffset + 0);
                    SD(x3, gback, gyoffset + 8);
                } else {
                    for (int i = 0; i < 4; ++i) {
                        // GY->sw[4+i] = EY->sw[i*2+0]+EY->sw[i*2+1];
                        LH(x3, wback, fixedaddress + 2 * (i * 2 + 0));
                        LH(x4, wback, fixedaddress + 2 * (i * 2 + 1));
                        ADDW(x3, x3, x4);
                        if (opcode == 0x03) SAT16(x3, x6);
                        SH(x3, gback, gyoffset + 2 * (4 + i));
                    }
                }
            } else {
                SD(xZR, gback, gyoffset + 0);
                SD(xZR, gback, gyoffset + 8);
            }
            break;
        case 0x02:
            INST_NAME("VPHADDD Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 28 : 12);
            GETGX();
            GETVX();
            GETGY();
            GETVY();
            if (gd == ed) {
                ADDI(x5, xEmu, offsetof(x64emu_t, scratch));
                LD(x3, wback, fixedaddress + 0);
                LD(x4, wback, fixedaddress + 8);
                SD(x3, x5, 0);
                SD(x4, x5, 8);
                wback = x5;
                fixedaddress = 0;
            }
            for (int i = 0; i < 2; ++i) {
                // GX->sd[i] = VX->sd[i*2+0]+VX->sd[i*2+1];
                LW(x3, vback, vxoffset + 4 * (i * 2 + 0));
                LW(x4, vback, vxoffset + 4 * (i * 2 + 1));
                ADDW(x3, x3, x4);
                SW(x3, gback, gdoffset + 4 * i);
            }
            if (MODREG && ed == vex.v) {
                // GX->q[1] = GX->q[0];
                LD(x3, gback, gdoffset + 0);
                SD(x3, gback, gdoffset + 8);
            } else {
                for (int i = 0; i < 2; ++i) {
                    // GX->sd[4+i] = EX->sd[i*2+0]+EX->sd[i*2+1];
                    LW(x3, wback, fixedaddress + 4 * (i * 2 + 0));
                    LW(x4, wback, fixedaddress + 4 * (i * 2 + 1));
                    ADDW(x3, x3, x4);
                    SW(x3, gback, gdoffset + 4 * (2 + i));
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
                for (int i = 0; i < 2; ++i) {
                    // GY->sd[i] = VY->sd[i*2+0]+VY->sd[i*2+1];
                    LW(x3, vback, vyoffset + 4 * (i * 2 + 0));
                    LW(x4, vback, vyoffset + 4 * (i * 2 + 1));
                    ADDW(x3, x3, x4);
                    SW(x3, gback, gyoffset + 4 * i);
                }
                if (MODREG && ed == vex.v) {
                    // GY->q[1] = GY->q[0];
                    LD(x3, gback, gyoffset + 0);
                    SD(x3, gback, gyoffset + 8);
                } else {
                    for (int i = 0; i < 4; ++i) {
                        // GY->sd[4+i] = EY->sd[i*2+0]+EY->sd[i*2+1];
                        LW(x3, wback, fixedaddress + 4 * (i * 2 + 0));
                        LW(x4, wback, fixedaddress + 4 * (i * 2 + 1));
                        ADDW(x3, x3, x4);
                        SW(x3, gback, gyoffset + 4 * (2 + i));
                    }
                }
            } else {
                SD(xZR, gback, gyoffset + 0);
                SD(xZR, gback, gyoffset + 8);
            }
            break;
        case 0x04:
            INST_NAME("VPMADDUBSW Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 31 : 15);
            GETGX();
            GETVX();
            GETGY();
            GETVY();
            for (int i = 0; i < 8; ++i) {
                LBU(x3, vback, vxoffset + i * 2);
                LB(x4, wback, fixedaddress + i * 2);
                MUL(x7, x3, x4);
                LBU(x3, vback, vxoffset + i * 2 + 1);
                LB(x4, wback, fixedaddress + i * 2 + 1);
                MUL(x3, x3, x4);
                ADD(x3, x3, x7);
                SAT16(x3, x6);
                SH(x3, gback, gdoffset + i * 2);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 8; ++i) {
                    LBU(x3, vback, vyoffset + i * 2);
                    LB(x4, wback, fixedaddress + i * 2);
                    MUL(x7, x3, x4);
                    LBU(x3, vback, vyoffset + i * 2 + 1);
                    LB(x4, wback, fixedaddress + i * 2 + 1);
                    MUL(x3, x3, x4);
                    ADD(x3, x3, x7);
                    SAT16(x3, x6);
                    SH(x3, gback, gyoffset + i * 2);
                }
            } else {
                SD(xZR, gback, gyoffset + 0);
                SD(xZR, gback, gyoffset + 8);
            }
            break;
        case 0x05:
        case 0x07:
            if (opcode == 0x05)
                INST_NAME("VPHSUBW Gx, Vx, Ex");
            else
                INST_NAME("VPHSUBSW Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 30 : 14);
            GETGX();
            GETVX();
            GETGY();
            GETVY();
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
                // GX->sw[i] = VX->sw[i*2+0]-VX->sw[i*2+1];
                LH(x3, vback, vxoffset + 2 * (i * 2 + 0));
                LH(x4, vback, vxoffset + 2 * (i * 2 + 1));
                SUBW(x3, x3, x4);
                if (opcode == 0x07) SAT16(x3, x6);
                SH(x3, gback, gdoffset + 2 * i);
            }
            if (MODREG && ed == vex.v) {
                // GX->q[1] = GX->q[0];
                LD(x3, gback, gdoffset + 0);
                SD(x3, gback, gdoffset + 8);
            } else {
                for (int i = 0; i < 4; ++i) {
                    // GX->sw[4+i] = EX->sw[i*2+0]-EX->sw[i*2+1];
                    LH(x3, wback, fixedaddress + 2 * (i * 2 + 0));
                    LH(x4, wback, fixedaddress + 2 * (i * 2 + 1));
                    SUBW(x3, x3, x4);
                    if (opcode == 0x07) SAT16(x3, x6);
                    SH(x3, gback, gdoffset + 2 * (4 + i));
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
                    // GY->sw[i] = VY->sw[i*2+0]-VY->sw[i*2+1];
                    LH(x3, vback, vyoffset + 2 * (i * 2 + 0));
                    LH(x4, vback, vyoffset + 2 * (i * 2 + 1));
                    SUBW(x3, x3, x4);
                    if (opcode == 0x07) SAT16(x3, x6);
                    SH(x3, gback, gyoffset + 2 * i);
                }
                if (MODREG && ed == vex.v) {
                    // GY->q[1] = GY->q[0];
                    LD(x3, gback, gyoffset + 0);
                    SD(x3, gback, gyoffset + 8);
                } else {
                    for (int i = 0; i < 4; ++i) {
                        // GY->sw[4+i] = EY->sw[i*2+0]-EY->sw[i*2+1];
                        LH(x3, wback, fixedaddress + 2 * (i * 2 + 0));
                        LH(x4, wback, fixedaddress + 2 * (i * 2 + 1));
                        SUBW(x3, x3, x4);
                        if (opcode == 0x07) SAT16(x3, x6);
                        SH(x3, gback, gyoffset + 2 * (4 + i));
                    }
                }
            } else {
                SD(xZR, gback, gyoffset + 0);
                SD(xZR, gback, gyoffset + 8);
            }
            break;
        case 0x06:
            INST_NAME("VPHSUBD Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 28 : 12);
            GETGX();
            GETVX();
            GETGY();
            GETVY();
            if (gd == ed) {
                ADDI(x5, xEmu, offsetof(x64emu_t, scratch));
                LD(x3, wback, fixedaddress + 0);
                LD(x4, wback, fixedaddress + 8);
                SD(x3, x5, 0);
                SD(x4, x5, 8);
                wback = x5;
                fixedaddress = 0;
            }
            for (int i = 0; i < 2; ++i) {
                // GX->sd[i] = VX->sd[i*2+0]-VX->sd[i*2+1];
                LW(x3, vback, vxoffset + 4 * (i * 2 + 0));
                LW(x4, vback, vxoffset + 4 * (i * 2 + 1));
                SUBW(x3, x3, x4);
                SW(x3, gback, gdoffset + 4 * i);
            }
            if (MODREG && ed == vex.v) {
                // GX->q[1] = GX->q[0];
                LD(x3, gback, gdoffset + 0);
                SD(x3, gback, gdoffset + 8);
            } else {
                for (int i = 0; i < 2; ++i) {
                    // GX->sd[4+i] = EX->sd[i*2+0]-EX->sd[i*2+1];
                    LW(x3, wback, fixedaddress + 4 * (i * 2 + 0));
                    LW(x4, wback, fixedaddress + 4 * (i * 2 + 1));
                    SUBW(x3, x3, x4);
                    SW(x3, gback, gdoffset + 4 * (2 + i));
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
                for (int i = 0; i < 2; ++i) {
                    // GY->sd[i] = VY->sd[i*2+0]-VY->sd[i*2+1];
                    LW(x3, vback, vyoffset + 4 * (i * 2 + 0));
                    LW(x4, vback, vyoffset + 4 * (i * 2 + 1));
                    SUBW(x3, x3, x4);
                    SW(x3, gback, gyoffset + 4 * i);
                }
                if (MODREG && ed == vex.v) {
                    // GY->q[1] = GY->q[0];
                    LD(x3, gback, gyoffset + 0);
                    SD(x3, gback, gyoffset + 8);
                } else {
                    for (int i = 0; i < 4; ++i) {
                        // GY->sd[4+i] = EY->sd[i*2+0]-EY->sd[i*2+1];
                        LW(x3, wback, fixedaddress + 4 * (i * 2 + 0));
                        LW(x4, wback, fixedaddress + 4 * (i * 2 + 1));
                        SUBW(x3, x3, x4);
                        SW(x3, gback, gyoffset + 4 * (2 + i));
                    }
                }
            } else {
                SD(xZR, gback, gyoffset + 0);
                SD(xZR, gback, gyoffset + 8);
            }
            break;
        case 0x08:
            INST_NAME("VPSIGNB Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 31 : 15);
            GETGX();
            GETVX();
            GETGY();
            GETVY();
            for (int i = 0; i < 16; ++i) {
                LB(x3, vback, vxoffset + i);
                LB(x4, wback, fixedaddress + i);
                SLT(x1, xZR, x4);
                SRAI(x5, x4, 63);
                OR(x1, x1, x5);
                MUL(x3, x1, x3);
                SB(x3, gback, gdoffset + i);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 16; ++i) {
                    LB(x3, vback, vyoffset + i);
                    LB(x4, wback, fixedaddress + i);
                    SLT(x1, xZR, x4);
                    SRAI(x5, x4, 63);
                    OR(x1, x1, x5);
                    MUL(x3, x1, x3);
                    SB(x3, gback, gyoffset + i);
                }
            } else {
                SD(xZR, gback, gyoffset + 0);
                SD(xZR, gback, gyoffset + 8);
            }
            break;
        case 0x09:
            INST_NAME("VPSIGNW Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 30 : 14);
            GETGX();
            GETVX();
            GETGY();
            GETVY();
            for (int i = 0; i < 8; ++i) {
                LH(x3, vback, vxoffset + i * 2);
                LH(x4, wback, fixedaddress + i * 2);
                SLT(x1, xZR, x4);
                SRAI(x5, x4, 63);
                OR(x1, x1, x5);
                MUL(x3, x1, x3);
                SH(x3, gback, gdoffset + i * 2);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 8; ++i) {
                    LH(x3, vback, vyoffset + i * 2);
                    LH(x4, wback, fixedaddress + i * 2);
                    SLT(x1, xZR, x4);
                    SRAI(x5, x4, 63);
                    OR(x1, x1, x5);
                    MUL(x3, x1, x3);
                    SH(x3, gback, gyoffset + i * 2);
                }
            } else {
                SD(xZR, gback, gyoffset + 0);
                SD(xZR, gback, gyoffset + 8);
            }
            break;
        case 0x0A:
            INST_NAME("VPSIGND Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 28 : 12);
            GETGX();
            GETVX();
            GETGY();
            GETVY();
            for (int i = 0; i < 4; ++i) {
                LH(x3, vback, vxoffset + i * 4);
                LH(x4, wback, fixedaddress + i * 4);
                SLT(x1, xZR, x4);
                SRAI(x5, x4, 63);
                OR(x1, x1, x5);
                MUL(x3, x1, x3);
                SH(x3, gback, gdoffset + i * 4);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 4; ++i) {
                    LH(x3, vback, vyoffset + i * 4);
                    LH(x4, wback, fixedaddress + i * 4);
                    SLT(x1, xZR, x4);
                    SRAI(x5, x4, 63);
                    OR(x1, x1, x5);
                    MUL(x3, x1, x3);
                    SH(x3, gback, gyoffset + i * 4);
                }
            } else {
                SD(xZR, gback, gyoffset + 0);
                SD(xZR, gback, gyoffset + 8);
            }
            break;
        case 0x0B:
            INST_NAME("VPMULHRSW Gx, Vx, Ex");
            nextop = F8;
            GETEX(x1, 0, vex.l ? 30 : 14);
            GETGX();
            GETVX();
            GETGY();
            GETVY();
            for (int i = 0; i < 8; ++i) {
                LH(x3, gback, vxoffset + i * 2);
                LH(x4, wback, fixedaddress + i * 2);
                MUL(x3, x3, x4);
                SRAI(x3, x3, 14);
                ADDI(x3, x3, 1);
                SRAI(x3, x3, 1);
                SH(x3, gback, gdoffset + i * 2);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 8; ++i) {
                    LH(x3, gback, vyoffset + i * 2);
                    LH(x4, wback, fixedaddress + i * 2);
                    MUL(x3, x3, x4);
                    SRAI(x3, x3, 14);
                    ADDI(x3, x3, 1);
                    SRAI(x3, x3, 1);
                    SH(x3, gback, gyoffset + i * 2);
                }
            } else {
                SD(xZR, gback, gyoffset + 0);
                SD(xZR, gback, gyoffset + 8);
            }
            break;
        case 0x17:
            INST_NAME("VPTEST Gx, Ex");
            nextop = F8;
            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            GETEX(x1, 0, vex.l ? 24 : 8);
            GETGX();
            CLEAR_FLAGS();
            SET_DFNONE();
            IFX (X_ZF | X_CF) {
                LD(x2, wback, fixedaddress + 0);
                LD(x3, wback, fixedaddress + 8);
                LD(x4, gback, gdoffset + 0);
                LD(x5, gback, gdoffset + 8);

                IFX (X_ZF) {
                    AND(x6, x4, x2);
                    AND(x7, x5, x3);
                    OR(x6, x6, x7);
                    BNEZ(x6, 4 + 4);
                    ORI(xFlags, xFlags, 1 << F_ZF);
                }
                IFX (X_CF) {
                    NOT(x4, x4);
                    NOT(x5, x5);
                    AND(x6, x4, x2);
                    AND(x7, x5, x3);
                    OR(x6, x6, x7);
                    BNEZ(x3, 4 + 4);
                    ORI(xFlags, xFlags, 1 << F_CF);
                }
            }
            if (vex.l) {
                GETEY();
                LD(x2, wback, fixedaddress + 0);
                LD(x3, wback, fixedaddress + 8);
                LD(x4, gback, gyoffset + 0);
                LD(x5, gback, gyoffset + 8);

                IFX (X_ZF) {
                    AND(x6, x4, x2);
                    AND(x7, x5, x3);
                    OR(x6, x6, x7);
                    BNEZ(x6, 4 + 2 * 4);
                    ANDI(x6, xFlags, 1 << F_ZF);
                    OR(xFlags, xFlags, x6);
                }
                IFX (X_CF) {
                    NOT(x4, x4);
                    NOT(x5, x5);
                    AND(x6, x4, x2);
                    AND(x7, x5, x3);
                    OR(x6, x6, x7);
                    BNEZ(x6, 4 + 2 * 4);
                    ANDI(x6, xFlags, 1 << F_CF);
                    OR(xFlags, xFlags, x6);
                }
            }
            break;
        default:
            DEFAULT;
    }
    return addr;
}
