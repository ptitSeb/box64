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
#include "bitutils.h"

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_functions.h"
#include "../dynarec_helper.h"
#include "emu/x64compstrings.h"

uintptr_t dynarec64_660F38(dynarec_rv64_t* dyn, uintptr_t addr, uint8_t opcode, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t nextop, u8, s8;
    int32_t i32;
    uint8_t gd, ed;
    uint8_t wback, wb1, wb2, gback;
    uint8_t eb1, eb2;
    int64_t j64;
    uint64_t tmp64u, tmp64u2;
    int v0, v1;
    int q0, q1;
    int d0, d1, d2;
    int64_t fixedaddress, gdoffset;
    int unscaled;

    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(j64);

    static const int8_t round_round[] = { RD_RNE, RD_RDN, RD_RUP, RD_RTZ };

    switch (opcode) {
        case 0x38: // SSSE3 opcodes
            nextop = F8;
            switch (nextop) {
                case 0x00:
                    INST_NAME("PSHUFB Gx, Ex");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0, 15);

                    ADDI(x5, xEmu, offsetof(x64emu_t, scratch));

                    // preserve gd
                    LD(x3, gback, gdoffset + 0);
                    LD(x4, gback, gdoffset + 8);
                    SD(x3, x5, 0);
                    SD(x4, x5, 8);

                    for (int i = 0; i < 16; ++i) {
                        LBU(x3, wback, fixedaddress + i);
                        ANDI(x4, x3, 128);
                        BEQZ(x4, 4 + 4 * 2);
                        SB(xZR, gback, gdoffset + i);
                        J(4 + 4 * 4); // continue
                        ANDI(x4, x3, 15);
                        ADD(x4, x4, x5);
                        LBU(x4, x4, 0);
                        SB(x4, gback, gdoffset + i);
                    }
                    break;
                case 0x01:
                    INST_NAME("PHADDW Gx, Ex");
                    nextop = F8;
                    GETGX();
                    for (int i = 0; i < 4; ++i) {
                        // GX->sw[i] = GX->sw[i*2+0]+GX->sw[i*2+1];
                        LH(x3, gback, gdoffset + 2 * (i * 2 + 0));
                        LH(x4, gback, gdoffset + 2 * (i * 2 + 1));
                        ADDW(x3, x3, x4);
                        SH(x3, gback, gdoffset + 2 * i);
                    }
                    if (MODREG && gd == (nextop & 7) + (rex.b << 3)) {
                        // GX->q[1] = GX->q[0];
                        LD(x3, gback, gdoffset + 0);
                        SD(x3, gback, gdoffset + 8);
                    } else {
                        GETEX(x2, 0, 14);
                        for (int i = 0; i < 4; ++i) {
                            // GX->sw[4+i] = EX->sw[i*2+0] + EX->sw[i*2+1];
                            LH(x3, wback, fixedaddress + 2 * (i * 2 + 0));
                            LH(x4, wback, fixedaddress + 2 * (i * 2 + 1));
                            ADDW(x3, x3, x4);
                            SH(x3, gback, gdoffset + 2 * (4 + i));
                        }
                    }
                    break;
                case 0x02:
                    INST_NAME("PHADDD Gx, Ex");
                    nextop = F8;
                    GETGX();
                    // GX->sd[0] += GX->sd[1];
                    LW(x3, gback, gdoffset + 0 * 4);
                    LW(x4, gback, gdoffset + 1 * 4);
                    ADDW(x3, x3, x4);
                    SW(x3, gback, gdoffset + 0 * 4);
                    // GX->sd[1] = GX->sd[2] + GX->sd[3];
                    LW(x3, gback, gdoffset + 2 * 4);
                    LW(x4, gback, gdoffset + 3 * 4);
                    ADDW(x3, x3, x4);
                    SW(x3, gback, gdoffset + 1 * 4);
                    if (MODREG && gd == (nextop & 7) + (rex.b << 3)) {
                        // GX->q[1] = GX->q[0];
                        LD(x3, gback, gdoffset + 0);
                        SD(x3, gback, gdoffset + 8);
                    } else {
                        GETEX(x2, 0, 12);
                        // GX->sd[2] = EX->sd[0] + EX->sd[1];
                        LW(x3, wback, fixedaddress + 0 * 4);
                        LW(x4, wback, fixedaddress + 1 * 4);
                        ADDW(x3, x3, x4);
                        SW(x3, gback, gdoffset + 2 * 4);
                        // GX->sd[3] = EX->sd[2] + EX->sd[3];
                        LW(x3, wback, fixedaddress + 2 * 4);
                        LW(x4, wback, fixedaddress + 3 * 4);
                        ADDW(x3, x3, x4);
                        SW(x3, gback, gdoffset + 3 * 4);
                    }
                    break;
                case 0x03:
                    INST_NAME("PHADDSW Gx, Ex");
                    nextop = F8;
                    GETGX();
                    LUI(x6, 0xFFFF8); // -32768
                    LUI(x7, 0x8);     // 32768
                    for (int i = 0; i < 4; ++i) {
                        // tmp32s = GX->sw[i*2+0]+GX->sw[i*2+1];
                        // GX->sw[i] = sat(tmp32s);
                        LH(x3, gback, gdoffset + 2 * (i * 2 + 0));
                        LH(x4, gback, gdoffset + 2 * (i * 2 + 1));
                        ADDW(x3, x3, x4);
                        SATw(x3, x6, x7);
                        SH(x3, gback, gdoffset + i * 2);
                    }
                    if (MODREG && gd == (nextop & 7) + (rex.b << 3)) {
                        // GX->q[1] = GX->q[0];
                        LD(x3, gback, gdoffset + 0);
                        SD(x3, gback, gdoffset + 8);
                    } else {
                        GETEX(x2, 0, 14);
                        for (int i = 0; i < 4; ++i) {
                            // tmp32s = EX->sw[i*2+0] + EX->sw[i*2+1];
                            // GX->sw[4+i] = sat(tmp32s);
                            LH(x3, wback, fixedaddress + 2 * (i * 2 + 0));
                            LH(x4, wback, fixedaddress + 2 * (i * 2 + 1));
                            ADDW(x3, x3, x4);
                            SATw(x3, x6, x7);
                            SH(x3, gback, gdoffset + 2 * (4 + i));
                        }
                    }
                    break;
                case 0x04:
                    INST_NAME("PMADDUBSW Gx, Ex");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0, 15);
                    LUI(x6, 0xFFFF8); // -32768
                    LUI(x7, 0x8);     // 32768
                    for (int i = 0; i < 8; ++i) {
                        LBU(x3, gback, gdoffset + i * 2);
                        LB(x4, wback, fixedaddress + i * 2);
                        MUL(x1, x3, x4);
                        LBU(x3, gback, gdoffset + i * 2 + 1);
                        LB(x4, wback, fixedaddress + i * 2 + 1);
                        MUL(x3, x3, x4);
                        ADD(x3, x3, x1);
                        SATw(x3, x6, x7);
                        SH(x3, gback, gdoffset + i * 2);
                    }
                    break;
                case 0x05:
                    INST_NAME("PHSUBW Gx, Ex");
                    nextop = F8;
                    GETGX();
                    for (int i = 0; i < 4; ++i) {
                        // GX->sw[i] = GX->sw[i*2+0] - GX->sw[i*2+1];
                        LH(x3, gback, gdoffset + 2 * (i * 2 + 0));
                        LH(x4, gback, gdoffset + 2 * (i * 2 + 1));
                        SUBW(x3, x3, x4);
                        SH(x3, gback, gdoffset + 2 * i);
                    }
                    if (MODREG && gd == (nextop & 7) + (rex.b << 3)) {
                        // GX->q[1] = GX->q[0];
                        LD(x3, gback, gdoffset + 0);
                        SD(x3, gback, gdoffset + 8);
                    } else {
                        GETEX(x2, 0, 14);
                        for (int i = 0; i < 4; ++i) {
                            // GX->sw[4+i] = EX->sw[i*2+0] - EX->sw[i*2+1];
                            LH(x3, wback, fixedaddress + 2 * (i * 2 + 0));
                            LH(x4, wback, fixedaddress + 2 * (i * 2 + 1));
                            SUBW(x3, x3, x4);
                            SH(x3, gback, gdoffset + 2 * (4 + i));
                        }
                    }
                    break;
                case 0x08:
                    INST_NAME("PSIGNB Gx, Ex");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0, 15);
                    for (int i = 0; i < 16; ++i) {
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
                    INST_NAME("PSIGNW Gx, Ex");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0, 14);
                    for (int i = 0; i < 8; ++i) {
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
                    INST_NAME("PSIGND Gx, Ex");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0, 12);
                    for (int i = 0; i < 4; ++i) {
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
                    INST_NAME("PMULHRSW Gx, Ex");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0, 14);
                    for (int i = 0; i < 8; ++i) {
                        LH(x3, gback, gdoffset + i * 2);
                        LH(x4, wback, fixedaddress + i * 2);
                        MUL(x3, x3, x4);
                        SRAI(x3, x3, 14);
                        ADDI(x3, x3, 1);
                        SRAI(x3, x3, 1);
                        SH(x3, gback, gdoffset + i * 2);
                    }
                    break;
                case 0x10:
                    INST_NAME("PBLENDVB Gx,Ex");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0, 15);
                    sse_forget_reg(dyn, ninst, x6, 0); // forget xmm[0]
                    for (int i = 0; i < 16; ++i) {
                        LB(x3, xEmu, offsetof(x64emu_t, xmm[0]) + i);
                        BGE(x3, xZR, 12); // continue
                        LBU(x3, wback, fixedaddress + i);
                        SB(x3, gback, gdoffset + i);
                        // continue
                    }
                    break;
                case 0x14:
                    INST_NAME("PBLENDVPS Gx,Ex");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0, 12);
                    sse_forget_reg(dyn, ninst, x6, 0);
                    for (int i = 0; i < 4; ++i) {
                        LW(x3, xEmu, offsetof(x64emu_t, xmm[0]) + i * 4);
                        BGE(x3, xZR, 4 + 4 * 2);
                        LWU(x3, wback, fixedaddress + i * 4);
                        SW(x3, gback, gdoffset + i * 4);
                    }
                    break;
                case 0x15:
                    INST_NAME("PBLENDVPD Gx,Ex");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0, 8);
                    sse_forget_reg(dyn, ninst, x6, 0);
                    for (int i = 0; i < 2; ++i) {
                        LD(x3, xEmu, offsetof(x64emu_t, xmm[0]) + i * 8);
                        BGE(x3, xZR, 4 + 4 * 2);
                        LD(x3, wback, fixedaddress + i * 8);
                        SD(x3, gback, gdoffset + i * 8);
                    }
                    break;
                case 0x17:
                    INST_NAME("PTEST Gx, Ex");
                    nextop = F8;
                    SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
                    GETGX();
                    GETEX(x1, 0, 8);
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
                            BNEZ(x6, 4 + 4);
                            ORI(xFlags, xFlags, 1 << F_CF);
                        }
                    }
                    break;

                case 0x1C:
                    INST_NAME("PABSB Gx, Ex");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0, 15);
                    for (int i = 0; i < 16; ++i) {
                        LB(x4, wback, fixedaddress + i);
                        BGE(x4, xZR, 4 + 4);
                        NEG(x4, x4);
                        SB(x4, gback, gdoffset + i);
                    }
                    break;
                case 0x1D:
                    INST_NAME("PABSW Gx, Ex");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0, 14);
                    for (int i = 0; i < 8; ++i) {
                        LH(x4, wback, fixedaddress + i * 2);
                        BGE(x4, xZR, 4 + 4);
                        NEG(x4, x4);
                        SH(x4, gback, gdoffset + i * 2);
                    }
                    break;
                case 0x1E:
                    INST_NAME("PABSD Gx, Ex");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0, 12);
                    for (int i = 0; i < 4; ++i) {
                        LW(x4, wback, fixedaddress + i * 4);
                        BGE(x4, xZR, 4 + 4);
                        NEG(x4, x4);
                        SW(x4, gback, gdoffset + i * 4);
                    }
                    break;
                case 0x20:
                    INST_NAME("PMOVSXBW Gx, Ex"); // SSE4 opcode!
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0, 7);
                    for (int i = 7; i >= 0; --i) {
                        // GX->sw[i] = EX->sb[i];
                        LB(x3, wback, fixedaddress + i);
                        SH(x3, gback, gdoffset + i * 2);
                    }
                    break;
                case 0x21:
                    INST_NAME("PMOVSXBD Gx, Ex"); // SSE4 opcode!
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0, 3);
                    for (int i = 3; i >= 0; --i) {
                        // GX->sd[i] = EX->sb[i];
                        LB(x3, wback, fixedaddress + i);
                        SW(x3, gback, gdoffset + i * 4);
                    }
                    break;
                case 0x22:
                    INST_NAME("PMOVSXBQ Gx, Ex"); // SSE4 opcode!
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0, 1);
                    for (int i = 1; i >= 0; --i) {
                        // GX->sq[i] = EX->sb[i];
                        LB(x3, wback, fixedaddress + i);
                        SD(x3, gback, gdoffset + i * 8);
                    }
                    break;
                case 0x23:
                    INST_NAME("PMOVSXWD Gx, Ex"); // SSE4 opcode!
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0, 6);
                    for (int i = 3; i >= 0; --i) {
                        // GX->sd[i] = EX->sw[i];
                        LH(x3, wback, fixedaddress + i * 2);
                        SW(x3, gback, gdoffset + i * 4);
                    }
                    break;
                case 0x24:
                    INST_NAME("PMOVSXWQ Gx, Ex"); // SSE4 opcode!
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0, 2);
                    for (int i = 1; i >= 0; --i) {
                        // GX->sq[i] = EX->sw[i];
                        LH(x3, wback, fixedaddress + i * 2);
                        SD(x3, gback, gdoffset + i * 8);
                    }
                    break;
                case 0x25:
                    INST_NAME("PMOVSXDQ Gx, Ex");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0, 4);
                    for (int i = 1; i >= 0; --i) {
                        // GX->sq[i] = EX->sd[i];
                        LW(x4, wback, fixedaddress + i * 4);
                        SD(x4, gback, gdoffset + i * 8);
                    }
                    break;
                case 0x28:
                    INST_NAME("PMULDQ Gx, Ex");
                    nextop = F8;
                    GETEX(x2, 0, 8);
                    GETGX();
                    for (int i = 1; i >= 0; --i) {
                        LW(x3, wback, fixedaddress + i * 8);
                        LW(x4, gback, gdoffset + i * 8);
                        MUL(x3, x3, x4);
                        SD(x3, gback, gdoffset + i * 8);
                    }
                    break;
                case 0x29:
                    INST_NAME("PCMPEQQ Gx, Ex"); // SSE4 opcode!
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0, 8);
                    SSE_LOOP_Q(x3, x4, XOR(x3, x3, x4); SNEZ(x3, x3); ADDI(x3, x3, -1));
                    break;
                case 0x2B:
                    INST_NAME("PACKUSDW Gx, Ex");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0, 12);
                    LUI(x5, 0x10); // 65536
                    for (int i = 0; i < 4; ++i) {
                        LW(x3, gback, gdoffset + i * 4);
                        SATUw(x3, x5);
                        SH(x3, gback, gdoffset + i * 2);
                    }
                    if (MODREG && gd == ed) {
                        LD(x3, gback, gdoffset + 0);
                        SD(x3, gback, gdoffset + 8);
                    } else
                        for (int i = 0; i < 4; ++i) {
                            LW(x3, wback, fixedaddress + i * 4);
                            SATUw(x3, x5);
                            SH(x3, gback, gdoffset + 8 + i * 2);
                        }
                    break;

                case 0x30:
                    INST_NAME("PMOVZXBW Gx, Ex");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0, 7);
                    for (int i = 7; i >= 0; --i) {
                        LBU(x3, wback, fixedaddress + i);
                        SH(x3, gback, gdoffset + i * 2);
                    }
                    break;
                case 0x31:
                    INST_NAME("PMOVZXBD Gx, Ex");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0, 3);
                    for (int i = 3; i >= 0; --i) {
                        LBU(x3, wback, fixedaddress + i);
                        SW(x3, gback, gdoffset + i * 4);
                    }
                    break;
                case 0x32:
                    INST_NAME("PMOVZXBQ Gx, Ex");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0, 2);
                    for (int i = 1; i >= 0; --i) {
                        LBU(x3, wback, fixedaddress + i);
                        SD(x3, gback, gdoffset + i * 8);
                    }
                    break;
                case 0x33:
                    INST_NAME("PMOVZXWD Gx, Ex");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0, 6);
                    for (int i = 3; i >= 0; --i) {
                        LHU(x3, wback, fixedaddress + i * 2);
                        SW(x3, gback, gdoffset + i * 4);
                    }
                    break;
                case 0x34:
                    INST_NAME("PMOVZXWQ Gx, Ex");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0, 2);
                    for (int i = 1; i >= 0; --i) {
                        LHU(x3, wback, fixedaddress + i * 2);
                        SD(x3, gback, gdoffset + i * 8);
                    }
                    break;
                case 0x35:
                    INST_NAME("PMOVZXDQ Gx, Ex");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0, 4);
                    for (int i = 1; i >= 0; --i) {
                        LWU(x3, wback, fixedaddress + i * 4);
                        SD(x3, gback, gdoffset + i * 8);
                    }
                    break;
                case 0x37:
                    INST_NAME("PCMPGTQ Gx, Ex"); // SSE4 opcode!
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0, 8);
                    SSE_LOOP_Q(x3, x4, SLT(x4, x4, x3); NEG(x3, x4));
                    break;
                case 0x38:
                    INST_NAME("PMINSB Gx, Ex"); // SSE4 opcode!
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0, 15);
                    for (int i = 0; i < 16; ++i) {
                        LB(x3, gback, gdoffset + i);
                        LB(x4, wback, fixedaddress + i);
                        if (cpuext.zbb)
                            MIN(x4, x3, x4);
                        else
                            BLT(x3, x4, 4 + 4);
                        SB(x4, gback, gdoffset + i);
                    }
                    break;
                case 0x39:
                    INST_NAME("PMINSD Gx, Ex"); // SSE4 opcode!
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0, 12);
                    for (int i = 0; i < 4; ++i) {
                        LW(x3, gback, gdoffset + i * 4);
                        LW(x4, wback, fixedaddress + i * 4);
                        if (cpuext.zbb)
                            MIN(x4, x3, x4);
                        else
                            BLT(x3, x4, 4 + 4);
                        SW(x4, gback, gdoffset + i * 4);
                    }
                    break;
                case 0x3A:
                    INST_NAME("PMINUW Gx, Ex"); // SSE4 opcode!
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0, 14);
                    for (int i = 0; i < 8; ++i) {
                        LHU(x3, gback, gdoffset + i * 2);
                        LHU(x4, wback, fixedaddress + i * 2);
                        if (cpuext.zbb)
                            MINU(x4, x3, x4);
                        else
                            BLTU(x3, x4, 4 + 4);
                        SH(x4, gback, gdoffset + i * 2);
                    }
                    break;
                case 0x3B:
                    INST_NAME("PMINUD Gx, Ex"); // SSE4 opcode!
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0, 12);
                    for (int i = 0; i < 4; ++i) {
                        LWU(x3, gback, gdoffset + i * 4);
                        LWU(x4, wback, fixedaddress + i * 4);
                        if (cpuext.zbb)
                            MINU(x4, x3, x4);
                        else
                            BLTU(x3, x4, 4 + 4);
                        SW(x4, gback, gdoffset + i * 4);
                    }
                    break;
                case 0x3C:
                    INST_NAME("PMAXSB Gx, Ex"); // SSE4 opcode!
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0, 15);
                    for (int i = 0; i < 16; ++i) {
                        LB(x3, gback, gdoffset + i);
                        LB(x4, wback, fixedaddress + i);
                        if (cpuext.zbb)
                            MAX(x4, x3, x4);
                        else
                            BLT(x4, x3, 4 + 4);
                        SB(x4, gback, gdoffset + i);
                    }
                    break;
                case 0x3D:
                    INST_NAME("PMAXSD Gx, Ex"); // SSE4 opcode!
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0, 12);
                    for (int i = 0; i < 4; ++i) {
                        LW(x3, gback, gdoffset + i * 4);
                        LW(x4, wback, fixedaddress + i * 4);
                        if (cpuext.zbb)
                            MAX(x4, x3, x4);
                        else
                            BLT(x4, x3, 4 + 4);
                        SW(x4, gback, gdoffset + i * 4);
                    }
                    break;
                case 0x3E:
                    INST_NAME("PMAXUW Gx, Ex"); // SSE4 opcode!
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0, 14);
                    for (int i = 0; i < 8; ++i) {
                        LHU(x3, gback, gdoffset + i * 2);
                        LHU(x4, wback, fixedaddress + i * 2);
                        if (cpuext.zbb)
                            MAXU(x4, x3, x4);
                        else
                            BLTU(x4, x3, 4 + 4);
                        SH(x4, gback, gdoffset + i * 2);
                    }
                    break;
                case 0x3F:
                    INST_NAME("PMAXUD Gx, Ex"); // SSE4 opcode!
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0, 12);
                    for (int i = 0; i < 4; ++i) {
                        LWU(x3, gback, gdoffset + i * 4);
                        LWU(x4, wback, fixedaddress + i * 4);
                        if (cpuext.zbb)
                            MAXU(x4, x3, x4);
                        else
                            BLTU(x4, x3, 4 + 4);
                        SW(x4, gback, gdoffset + i * 4);
                    }
                    break;
                case 0x40:
                    INST_NAME("PMULLD Gx, Ex");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0, 12);
                    for (int i = 0; i < 4; ++i) {
                        LW(x3, gback, gdoffset + i * 4);
                        LW(x4, wback, fixedaddress + i * 4);
                        MUL(x3, x3, x4);
                        SW(x3, gback, gdoffset + i * 4);
                    }
                    break;
                case 0x61:
                    INST_NAME("PCMPESTRI Gx, Ex, Ib");
                    SETFLAGS(X_ALL, SF_SET_DF, NAT_FLAGS_NOFUSION);
                    nextop = F8;
                    GETG;
                    sse_reflect_reg(dyn, ninst, x6, gd);
                    ADDI(x3, xEmu, offsetof(x64emu_t, xmm[gd]));
                    if (MODREG) {
                        ed = (nextop & 7) + (rex.b << 3);
                        sse_reflect_reg(dyn, ninst, x6, ed);
                        ADDI(x1, xEmu, offsetof(x64emu_t, xmm[ed]));
                        ed = x1;
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 0, 1);
                    }
                    // prepare rest arguments
                    MV(x2, xRDX);
                    MV(x4, xRAX);
                    u8 = F8;
                    MOV32w(x5, u8);
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
                case 0xDB:
                    INST_NAME("AESIMC Gx, Ex"); // AES-NI
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 0, 8);
                    SSE_LOOP_MV_Q(x3);
                    sse_forget_reg(dyn, ninst, x6, gd);
                    MOV32w(x1, gd);
                    CALL(const_native_aesimc, -1, x1, 0);
                    break;
                case 0xDC:
                    INST_NAME("AESENC Gx, Ex"); // AES-NI
                    nextop = F8;
                    GETG;
                    sse_forget_reg(dyn, ninst, x6, gd);
                    MOV32w(x1, gd);
                    CALL(const_native_aese, -1, x1, 0);
                    GETGX();
                    GETEX(x2, 0, 8);
                    SSE_LOOP_Q(x3, x4, XOR(x3, x3, x4));
                    break;
                case 0xDD:
                    INST_NAME("AESENCLAST Gx, Ex"); // AES-NI
                    nextop = F8;
                    GETG;
                    sse_forget_reg(dyn, ninst, x6, gd);
                    MOV32w(x1, gd);
                    CALL(const_native_aeselast, -1, x1, 0);
                    GETGX();
                    GETEX(x2, 0, 8);
                    SSE_LOOP_Q(x3, x4, XOR(x3, x3, x4));
                    break;
                case 0xDE:
                    INST_NAME("AESDEC Gx, Ex"); // AES-NI
                    nextop = F8;
                    GETG;
                    sse_forget_reg(dyn, ninst, x6, gd);
                    MOV32w(x1, gd);
                    CALL(const_native_aesd, -1, x1, 0);
                    GETGX();
                    GETEX(x2, 0, 8);
                    SSE_LOOP_Q(x3, x4, XOR(x3, x3, x4));
                    break;

                case 0xDF:
                    INST_NAME("AESDECLAST Gx, Ex"); // AES-NI
                    nextop = F8;
                    GETG;
                    sse_forget_reg(dyn, ninst, x6, gd);
                    MOV32w(x1, gd);
                    CALL(const_native_aesdlast, -1, x1, 0);
                    GETGX();
                    GETEX(x2, 0, 8);
                    SSE_LOOP_Q(x3, x4, XOR(x3, x3, x4));
                    break;
                case 0xF0:
                    INST_NAME("MOVBE Gw, Ew");
                    nextop = F8;
                    GETGD;
                    SMREAD();
                    addr = geted(dyn, addr, ninst, nextop, &ed, x3, x2, &fixedaddress, rex, NULL, 1, 0);
                    LHU(x1, ed, fixedaddress);
                    if (cpuext.zbb) {
                        REV8(x1, x1);
                        SRLI(x1, x1, 48);
                    } else if (cpuext.xtheadbb) {
                        TH_REVW(x1, x1);
                        SRLI(x1, x1, 16);
                    } else {
                        ANDI(x2, x1, 0xff);
                        SLLI(x2, x2, 8);
                        SRLI(x1, x1, 8);
                        OR(x1, x1, x2);
                    }
                    LUI(x2, 0xffff0);
                    AND(gd, gd, x2);
                    OR(gd, gd, x1);
                    break;
                case 0xF1:
                    INST_NAME("MOVBE Ew, Gw");
                    nextop = F8;
                    GETGD;
                    SMREAD();
                    addr = geted(dyn, addr, ninst, nextop, &wback, x3, x2, &fixedaddress, rex, NULL, 1, 0);
                    if (cpuext.zbb) {
                        REV8(x1, gd);
                        SRLI(x1, x1, 48);
                    } else if (cpuext.xtheadbb) {
                        TH_REVW(x1, gd);
                        SRLI(x1, x1, 16);
                    } else {
                        ANDI(x1, gd, 0xff);
                        SLLI(x1, x1, 8);
                        SRLI(x2, gd, 8);
                        ANDI(x2, x2, 0xff);
                        OR(x1, x1, x2);
                    }
                    SH(x1, wback, fixedaddress);
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0x3A: // these are some more SSSE3+ opcodes
            opcode = F8;
            switch (opcode) {
                case 0x09:
                    INST_NAME("ROUNDPD Gx, Ex, Ib");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 1, 8);
                    u8 = F8;
                    d0 = fpu_get_scratch(dyn);
                    d1 = fpu_get_scratch(dyn);
                    v1 = fpu_get_scratch(dyn);
                    MOV64x(x3, 1ULL << __DBL_MANT_DIG__);
                    FCVTDL(d1, x3, RD_RTZ);

                    // i = 0
                    FLD(d0, wback, fixedaddress);
                    FEQD(x4, d0, d0);
                    BNEZ(x4, 8);
                    B_MARK_nocond;
                    // d0 is not nan
                    FABSD(v1, d0);
                    FLTD(x4, v1, d1);
                    BNEZ(x4, 8);
                    B_MARK_nocond;
                    if (u8 & 4) {
                        u8 = sse_setround(dyn, ninst, x4, x5);
                        FCVTLD(x5, d0, RD_DYN);
                        FCVTDL(d0, x5, RD_RTZ);
                        x87_restoreround(dyn, ninst, u8);
                    } else {
                        FCVTLD(x5, d0, round_round[u8 & 3]);
                        FCVTDL(d0, x5, RD_RTZ);
                    }
                    MARK;
                    FSD(d0, gback, gdoffset + 0);

                    // i = 1
                    FLD(d0, wback, fixedaddress + 8);
                    FEQD(x4, d0, d0);
                    BNEZ(x4, 8);
                    B_MARK2_nocond;
                    // d0 is not nan
                    FABSD(v1, d0);
                    FLTD(x4, v1, d1);
                    BNEZ(x4, 8);
                    B_MARK2_nocond;
                    if (u8 & 4) {
                        u8 = sse_setround(dyn, ninst, x4, x5);
                        FCVTLD(x5, d0, RD_DYN);
                        FCVTDL(d0, x5, RD_RTZ);
                        x87_restoreround(dyn, ninst, u8);
                    } else {
                        FCVTLD(x5, d0, round_round[u8 & 3]);
                        FCVTDL(d0, x5, RD_RTZ);
                    }
                    MARK2;
                    FSD(d0, gback, gdoffset + 8);
                    break;
                case 0x0A:
                    INST_NAME("ROUNDSS Gx, Ex, Ib");
                    nextop = F8;
                    GETEXSS(d0, 1);
                    GETGXSS_empty(v0);
                    d1 = fpu_get_scratch(dyn);
                    v1 = fpu_get_scratch(dyn);
                    u8 = F8;
                    FEQS(x2, d0, d0);
                    BNEZ_MARK(x2);
                    if (v0 != d0) FMVS(v0, d0);
                    B_NEXT_nocond;
                    MARK; // d0 is not nan
                    FABSS(v1, d0);
                    MOV64x(x3, 1ULL << __FLT_MANT_DIG__);
                    FCVTSW(d1, x3, RD_RTZ);
                    FLTS(x3, v1, d1);
                    BNEZ_MARK2(x3);
                    if (v0 != d0) FMVS(v0, d0);
                    B_NEXT_nocond;
                    MARK2;
                    if (u8 & 4) {
                        u8 = sse_setround(dyn, ninst, x4, x2);
                        FCVTWS(x5, d0, RD_DYN);
                        FCVTSW(v0, x5, RD_RTZ);
                        x87_restoreround(dyn, ninst, u8);
                    } else {
                        FCVTWS(x5, d0, round_round[u8 & 3]);
                        FCVTSW(v0, x5, RD_RTZ);
                    }
                    break;
                case 0x0B:
                    INST_NAME("ROUNDSD Gx, Ex, Ib");
                    nextop = F8;
                    GETEXSD(d0, 1);
                    GETGXSD_empty(v0);
                    d1 = fpu_get_scratch(dyn);
                    v1 = fpu_get_scratch(dyn);
                    u8 = F8;
                    FEQD(x2, d0, d0);
                    BNEZ_MARK(x2);
                    if (v0 != d0) FMVD(v0, d0);
                    B_NEXT_nocond;
                    MARK; // d0 is not nan
                    FABSD(v1, d0);
                    MOV64x(x3, 1ULL << __DBL_MANT_DIG__);
                    FCVTDL(d1, x3, RD_RTZ);
                    FLTD(x3, v1, d1);
                    BNEZ_MARK2(x3);
                    if (v0 != d0) FMVD(v0, d0);
                    B_NEXT_nocond;
                    MARK2;
                    if (u8 & 4) {
                        u8 = sse_setround(dyn, ninst, x4, x2);
                        FCVTLD(x5, d0, RD_DYN);
                        FCVTDL(v0, x5, RD_RTZ);
                        x87_restoreround(dyn, ninst, u8);
                    } else {
                        FCVTLD(x5, d0, round_round[u8 & 3]);
                        FCVTDL(v0, x5, RD_RTZ);
                    }
                    break;
                case 0x0C:
                    INST_NAME("BLENDPS Gx, Ex, Ib");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 1, 12);
                    u8 = F8 & 0b1111;
                    for (int i = 0; i < 4; ++i)
                        if (u8 & (1 << i)) {
                            LWU(x1, wback, fixedaddress + i * 4);
                            SW(x1, gback, gdoffset + i * 4);
                        }
                    break;
                case 0x0E:
                    INST_NAME("PBLENDW Gx, Ex, Ib");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 1, 14);
                    u8 = F8;
                    i32 = 0;
                    if (MODREG && gd == ed) break;
                    while (u8)
                        if (u8 & 1) {
                            if (!(i32 & 1) && u8 & 2) {
                                if (!(i32 & 3) && (u8 & 0xf) == 0xf) {
                                    // whole 64bits
                                    LD(x3, wback, fixedaddress + 8 * (i32 >> 2));
                                    SD(x3, gback, gdoffset + 8 * (i32 >> 2));
                                    i32 += 4;
                                    u8 >>= 4;
                                } else {
                                    // 32bits
                                    LWU(x3, wback, fixedaddress + 4 * (i32 >> 1));
                                    SW(x3, gback, gdoffset + 4 * (i32 >> 1));
                                    i32 += 2;
                                    u8 >>= 2;
                                }
                            } else {
                                // 16 bits
                                LHU(x3, wback, fixedaddress + 2 * i32);
                                SH(x3, gback, gdoffset + 2 * i32);
                                i32++;
                                u8 >>= 1;
                            }
                        } else {
                            // nope
                            i32++;
                            u8 >>= 1;
                        }
                    break;
                case 0x0F:
                    INST_NAME("PALIGNR Gx, Ex, Ib");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 1, 8);
                    u8 = F8;
                    if (u8 > 31) {
                        SD(xZR, gback, gdoffset + 0);
                        SD(xZR, gback, gdoffset + 8);
                    } else if (u8 > 23) {
                        LD(x5, gback, gdoffset + 8);
                        if (u8 > 24) {
                            SRLI(x5, x5, 8 * (u8 - 24));
                        }
                        SD(x5, gback, gdoffset + 0);
                        SD(xZR, gback, gdoffset + 8);
                    } else if (u8 > 15) {
                        if (u8 > 16) {
                            LD(x5, gback, gdoffset + 8);
                            LD(x4, gback, gdoffset + 0);
                            SRLI(x3, x5, 8 * (u8 - 16)); // lower of higher 64 bits
                            SLLI(x5, x5, 8 * (24 - u8)); // higher of lower 64 bits
                            SD(x3, gback, gdoffset + 8);
                            SRLI(x4, x4, 8 * (u8 - 16)); // lower of lower 64 bits
                            OR(x4, x4, x5);              // lower 64 bits
                            SD(x4, gback, gdoffset + 0);
                        }
                    } else if (u8 > 7) {
                        if (u8 > 8) {
                            LD(x5, gback, gdoffset + 8);
                            LD(x4, gback, gdoffset + 0);
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
                            LD(x5, gback, gdoffset + 0);
                            LD(x4, wback, fixedaddress + 8);
                            SD(x5, gback, gdoffset + 8);
                            SD(x4, gback, gdoffset + 0);
                        }
                    } else {
                        if (u8 > 0) {
                            LD(x5, gback, gdoffset + 0);
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
                    break;
                case 0x16:
                    if (rex.w) {
                        INST_NAME("PEXTRQ Ed, Gx, Ib");
                    } else {
                        INST_NAME("PEXTRD Ed, Gx, Ib");
                    }
                    nextop = F8;
                    GETGX();
                    GETED(1);
                    u8 = F8;
                    if (rex.w)
                        LD(ed, gback, gdoffset + 8 * (u8 & 1));
                    else
                        LWU(ed, gback, gdoffset + 4 * (u8 & 3));
                    if (wback) {
                        SDxw(ed, wback, fixedaddress);
                        SMWRITE2();
                    }
                    break;
                case 0x17:
                    INST_NAME("EXTRACTPS Ew, Gx, Ib");
                    nextop = F8;
                    GETGX();
                    GETED(1);
                    u8 = F8;
                    LWU(ed, gback, gdoffset + 4 * (u8 & 3));
                    if (wback) {
                        SW(ed, wback, fixedaddress);
                        SMWRITE2();
                    }
                    break;
                case 0x20:
                    INST_NAME("PINSRB Gx, ED, Ib");
                    nextop = F8;
                    GETGX();
                    if (MODREG) {
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        wback = 0;
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 1, 1);
                        LB(x1, wback, fixedaddress);
                        ed = x1;
                    }
                    u8 = F8;
                    SB(ed, gback, gdoffset + (u8 & 0xF));
                    break;
                case 0x21:
                    INST_NAME("INSERTPS GX, EX, Ib");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 1, 12);
                    u8 = F8;
                    if (MODREG)
                        s8 = (u8 >> 6) & 3;
                    else
                        s8 = 0;
                    // GX->ud[(tmp8u>>4)&3] = EX->ud[tmp8s];
                    LWU(x3, wback, fixedaddress + 4 * s8);
                    SW(x3, gback, gdoffset + 4 * ((u8 >> 4) & 3));
                    for (int i = 0; i < 4; ++i) {
                        if (u8 & (1 << i))
                            // GX->ud[i] = 0;
                            SW(xZR, gback, gdoffset + 4 * i);
                    }
                    break;
                case 0x22:
                    INST_NAME("PINSRD Gx, ED, Ib");
                    nextop = F8;
                    GETGX();
                    GETED(1);
                    u8 = F8;
                    if (rex.w) {
                        SD(ed, gback, gdoffset + 8 * (u8 & 0x1));
                    } else {
                        SW(ed, gback, gdoffset + 4 * (u8 & 0x3));
                    }
                    break;
                case 0x40:
                    INST_NAME("DPPS Gx, Ex, Ib");
                    nextop = F8;
                    GETGX();
                    GETEX(x2, 1, 12);
                    u8 = F8;
                    d0 = fpu_get_scratch(dyn);
                    d1 = fpu_get_scratch(dyn);
                    d2 = fpu_get_scratch(dyn);
                    FMVWX(d2, xZR);
                    for (int i = 0; i < 4; ++i)
                        if (u8 & (1 << (i + 4))) {
                            FLW(d0, gback, gdoffset + i * 4);
                            FLW(d1, wback, fixedaddress + i * 4);
                            FMULS(d0, d0, d1);
                            FADDS(d2, d2, d0);
                        }
                    for (int i = 0; i < 4; ++i)
                        if (u8 & (1 << i))
                            FSW(d2, gback, gdoffset + i * 4);
                        else
                            SW(xZR, gback, gdoffset + i * 4);
                    break;
                case 0x44:
                    INST_NAME("PCLMULQDQ Gx, Ex, Ib");
                    nextop = F8;
                    GETG;
                    sse_forget_reg(dyn, ninst, x6, gd);
                    MOV32w(x1, gd); // gx
                    if (MODREG) {
                        ed = (nextop & 7) + (rex.b << 3);
                        sse_forget_reg(dyn, ninst, x6, ed);
                        MOV32w(x2, ed);
                        MOV32w(x3, 0); // p = NULL
                    } else {
                        MOV32w(x2, 0);
                        addr = geted(dyn, addr, ninst, nextop, &ed, x3, x5, &fixedaddress, rex, NULL, 0, 1);
                        if (ed != x3) {
                            MV(x3, ed);
                        }
                    }
                    u8 = F8;
                    MOV32w(x4, u8);
                    CALL4(const_native_pclmul, -1, x1, x2, x3, x4);
                    break;
                case 0x60:
                    INST_NAME("PCMPESTRM Gx, Ex, Ib");
                    SETFLAGS(X_ALL, SF_SET_DF, NAT_FLAGS_NOFUSION);
                    nextop = F8;
                    GETG;
                    sse_forget_reg(dyn, ninst, x6, gd);
                    sse_forget_reg(dyn, ninst, x6, 0);
                    ADDI(x3, xEmu, offsetof(x64emu_t, xmm[gd]));
                    if (MODREG) {
                        ed = (nextop & 7) + (rex.b << 3);
                        sse_reflect_reg(dyn, ninst, x6, ed);
                        ADDI(x1, xEmu, offsetof(x64emu_t, xmm[ed]));
                        ed = x1;
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 0, 1);
                    }
                    MV(x2, xRDX);
                    MV(x4, xRAX);
                    u8 = F8;
                    ADDI(x5, xZR, u8);
                    CALL6(const_sse42_compare_string_explicit_len, x1, ed, x2, x3, x4, x5, 0);
                    if (u8 & 0b1000000) {
                        switch (u8 & 1) {
                            case 0b00:
                                for (int i = 0; i < 16; ++i) {
                                    SRLI(x3, x1, i);
                                    ANDI(x3, x3, 1);
                                    NEG(x3, x3);
                                    SB(x3, xEmu, offsetof(x64emu_t, xmm[0]) + i);
                                }
                                break;
                            case 0b01:
                                for (int i = 0; i < 8; ++i) {
                                    SRLI(x3, x1, i);
                                    ANDI(x3, x3, 1);
                                    NEG(x3, x3);
                                    SH(x3, xEmu, offsetof(x64emu_t, xmm[0]) + i * 2);
                                }
                                break;
                        }
                    } else {
                        SW(x1, xEmu, offsetof(x64emu_t, xmm[0]));
                        SW(xZR, xEmu, offsetof(x64emu_t, xmm[0]) + 4);
                        SD(xZR, xEmu, offsetof(x64emu_t, xmm[0]) + 8);
                    }
                    break;
                case 0x61:
                    INST_NAME("PCMPESTRI Gx, Ex, Ib");
                    nextop = F8;
                    GETG;
                    u8 = geted_ib(dyn, addr, ninst, nextop);
                    SETFLAGS(X_ALL, SF_SET_DF, NAT_FLAGS_NOFUSION);
                    sse_reflect_reg(dyn, ninst, x6, gd);
                    ADDI(x3, xEmu, offsetof(x64emu_t, xmm[gd]));
                    if (MODREG) {
                        ed = (nextop & 7) + (rex.b << 3);
                        sse_reflect_reg(dyn, ninst, x6, ed);
                        ADDI(x1, xEmu, offsetof(x64emu_t, xmm[ed]));
                        ed = x1;
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 0, 1);
                    }
                    ZEXTW2(x2, xRDX);
                    ZEXTW2(x4, xRAX);
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
                case 0x62:
                    INST_NAME("PCMPISTRM Gx, Ex, Ib");
                    SETFLAGS(X_ALL, SF_SET_DF, NAT_FLAGS_NOFUSION);
                    nextop = F8;
                    GETG;
                    sse_forget_reg(dyn, ninst, x6, gd);
                    sse_forget_reg(dyn, ninst, x6, 0);
                    ADDI(x2, xEmu, offsetof(x64emu_t, xmm[gd]));
                    if (MODREG) {
                        ed = (nextop & 7) + (rex.b << 3);
                        sse_reflect_reg(dyn, ninst, x6, ed);
                        ADDI(x1, xEmu, offsetof(x64emu_t, xmm[ed]));
                        ed = x1;
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 0, 1);
                    }
                    u8 = F8;
                    ADDI(x3, xZR, u8);
                    CALL4(const_sse42_compare_string_implicit_len, x1, ed, x2, x3, 0);
                    if (u8 & 0b1000000) {
                        switch (u8 & 1) {
                            case 0b00:
                                for (int i = 0; i < 16; ++i) {
                                    SRLI(x3, x1, i);
                                    ANDI(x3, x3, 1);
                                    NEG(x3, x3);
                                    SB(x3, xEmu, offsetof(x64emu_t, xmm[0]) + i);
                                }
                                break;
                            case 0b01:
                                for (int i = 0; i < 8; ++i) {
                                    SRLI(x3, x1, i);
                                    ANDI(x3, x3, 1);
                                    NEG(x3, x3);
                                    SH(x3, xEmu, offsetof(x64emu_t, xmm[0]) + i * 2);
                                }
                                break;
                        }
                    } else {
                        SW(x1, xEmu, offsetof(x64emu_t, xmm[0]));
                        SW(xZR, xEmu, offsetof(x64emu_t, xmm[0]) + 4);
                        SD(xZR, xEmu, offsetof(x64emu_t, xmm[0]) + 8);
                    }
                    break;
                case 0x63:
                    INST_NAME("PCMPISTRI Gx, Ex, Ib");
                    SETFLAGS(X_ALL, SF_SET_DF, NAT_FLAGS_NOFUSION);
                    nextop = F8;
                    GETG;
                    sse_reflect_reg(dyn, ninst, x6, gd);
                    ADDI(x2, xEmu, offsetof(x64emu_t, xmm[gd]));
                    if (MODREG) {
                        ed = (nextop & 7) + (rex.b << 3);
                        sse_reflect_reg(dyn, ninst, x6, ed);
                        ADDI(x1, xEmu, offsetof(x64emu_t, xmm[ed]));
                        ed = x1;
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 0, 1);
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
                case 0xDF:
                    INST_NAME("AESKEYGENASSIST Gx, Ex, Ib"); // AES-NI
                    nextop = F8;
                    GETG;
                    sse_forget_reg(dyn, ninst, x6, gd);
                    MOV32w(x1, gd); // gx
                    if (MODREG) {
                        ed = (nextop & 7) + (rex.b << 3);
                        sse_forget_reg(dyn, ninst, x6, ed);
                        MOV32w(x2, ed);
                        MOV32w(x3, 0); // p = NULL
                    } else {
                        MOV32w(x2, 0);
                        addr = geted(dyn, addr, ninst, nextop, &ed, x3, x2, &fixedaddress, rex, NULL, 0, 1);
                        if (ed != x3) {
                            MV(x3, ed);
                        }
                    }
                    u8 = F8;
                    MOV32w(x4, u8);
                    CALL4(const_native_aeskeygenassist, -1, x1, x2, x3, x4);
                    break;
                default:
                    DEFAULT;
            }
            break;
        default:
            DEFAULT;
    }
    return addr;
}
