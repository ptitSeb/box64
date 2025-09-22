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
        case 0x4A:
            INST_NAME("VBLENDVPS Gx, Vx, Ex, XMMImm8");
            nextop = F8;
            u8 = geted_ib(dyn, addr, ninst, nextop) >> 4;
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
                MV(x3, x5);
                SW(x3, gback, gdoffset + i * 4);
            }
            if (vex.l) {
                GETEY();
                for (int i = 0; i < 4; ++i) {
                    LW(x3, xEmu, offsetof(x64emu_t, ymm) + u8 * 16 + i * 4);
                    LWU(x4, wback, fixedaddress + i * 4);
                    LWU(x5, vback, vyoffset + i * 4);
                    BGE(x3, xZR, 4 + 4);
                    MV(x5, x4);
                    MV(x3, x5);
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
