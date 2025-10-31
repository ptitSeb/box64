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
#include "custommem.h"

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "../dynarec_helper.h"
#include "dynarec_rv64_functions.h"

uintptr_t dynarec64_64(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int seg, int* ok, int* need_epilog)
{
    (void)ip;
    (void)rep;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop;
    uint8_t u8;
    uint8_t gd, ed, eb1, eb2, gb1, gb2;
    uint8_t gback, wback, wb1, wb2, wb;
    int64_t i64, j64;
    uint64_t u64;
    int v0, v1;
    int q0;
    int d0;
    int64_t fixedaddress, gdoffset;
    int unscaled;
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(wb1);
    MAYUSE(wb2);
    MAYUSE(gb1);
    MAYUSE(gb2);
    MAYUSE(j64);
    MAYUSE(d0);
    MAYUSE(q0);
    MAYUSE(v0);
    MAYUSE(v1);

    while ((opcode == 0xF2) || (opcode == 0xF3)) {
        rep = opcode - 0xF1;
        opcode = F8;
    }

    GETREX();

    switch (opcode) {
        case 0x03:
            INST_NAME("ADD Gd, Seg:Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
            GETGD;
            GETEDO(x4, 0, x5);
            emit_add32(dyn, ninst, rex, gd, ed, x3, x4, x5);
            break;
        case 0x0F:
            opcode = F8;
            switch (opcode) {
                case 0x11:
                    switch (rep) {
                        case 0:
                            INST_NAME("MOVUPS Ex,Gx");
                            nextop = F8;
                            GETGX();
                            GETEX(x2, 0, 8);
                            if (!MODREG) {
                                grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
                                ADDz(x4, x4, wback);
                                wback = x4;
                            }
                            LD(x3, gback, gdoffset + 0);
                            LD(x5, gback, gdoffset + 8);
                            SD(x3, wback, fixedaddress + 0);
                            SD(x5, wback, fixedaddress + 8);
                            if (!MODREG)
                                SMWRITE2();
                            break;
                        case 1:
                            INST_NAME("MOVSD Ex, Gx");
                            nextop = F8;
                            GETG;
                            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
                            if (MODREG) {
                                ed = (nextop & 7) + (rex.b << 3);
                                d0 = sse_get_reg(dyn, ninst, x1, ed, 0);
                                FMVD(d0, v0);
                            } else {
                                grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
                                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                                ADDz(x4, x4, ed);
                                ed = x4;
                                FSD(v0, ed, fixedaddress);
                                SMWRITE2();
                            }
                            break;
                        case 2:
                            INST_NAME("MOVSS Ex, Gx");
                            nextop = F8;
                            GETG;
                            v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
                            if (MODREG) {
                                q0 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 1);
                                FMVS(q0, v0);
                            } else {
                                grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
                                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                                ADDz(x4, x4, ed);
                                ed = x4;
                                FSW(v0, ed, fixedaddress);
                                SMWRITE2();
                            }
                            break;
                        default:
                            DEFAULT;
                    }
                    break;

                case 0x18:
                case 0x19:
                case 0x1F:
                    INST_NAME("NOP (multibyte)");
                    nextop = F8;
                    FAKEED;
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0x2B:
            INST_NAME("SUB Gd, Seg:Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
            GETGD;
            GETEDO(x4, 0, x5);
            emit_sub32(dyn, ninst, rex, gd, ed, x3, x4, x5);
            break;

        case 0x33:
            INST_NAME("XOR Gd, Seg:Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
            GETGD;
            GETEDO(x4, 0, x5);
            emit_xor32(dyn, ninst, rex, gd, ed, x3, x4);
            break;

        case 0x66:
            addr = dynarec64_6664(dyn, addr, ip, ninst, rex, seg, ok, need_epilog);
            break;
        case 0x80:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0: // ADD
                    INST_NAME("ADD Eb, Ib");
                    grab_segdata(dyn, addr, ninst, x1, seg, (MODREG));
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEBO(x1, 1);
                    u8 = F8;
                    emit_add8c(dyn, ninst, x1, u8, x2, x4, x5, x6);
                    EBBACK(x5, 0);
                    break;
                case 1: // OR
                    INST_NAME("OR Eb, Ib");
                    grab_segdata(dyn, addr, ninst, x1, seg, (MODREG));
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEBO(x1, 1);
                    u8 = F8;
                    emit_or8c(dyn, ninst, x1, u8, x2, x4, x5);
                    EBBACK(x5, 0);
                    break;
                case 2: // ADC
                    INST_NAME("ADC Eb, Ib");
                    grab_segdata(dyn, addr, ninst, x1, seg, (MODREG));
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEBO(x1, 1);
                    u8 = F8;
                    emit_adc8c(dyn, ninst, x1, u8, x2, x4, x5, x6);
                    EBBACK(x5, 0);
                    break;
                case 3: // SBB
                    INST_NAME("SBB Eb, Ib");
                    grab_segdata(dyn, addr, ninst, x1, seg, (MODREG));
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEBO(x1, 1);
                    u8 = F8;
                    emit_sbb8c(dyn, ninst, x1, u8, x2, x4, x5, x6);
                    EBBACK(x5, 0);
                    break;
                case 4: // AND
                    INST_NAME("AND Eb, Ib");
                    grab_segdata(dyn, addr, ninst, x1, seg, (MODREG));
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEBO(x1, 1);
                    u8 = F8;
                    emit_and8c(dyn, ninst, x1, u8, x2, x4);
                    EBBACK(x5, 0);
                    break;
                case 5: // SUB
                    INST_NAME("SUB Eb, Ib");
                    grab_segdata(dyn, addr, ninst, x1, seg, (MODREG));
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEBO(x1, 1);
                    u8 = F8;
                    emit_sub8c(dyn, ninst, x1, u8, x2, x4, x5, x6);
                    EBBACK(x5, 0);
                    break;
                case 6: // XOR
                    INST_NAME("XOR Eb, Ib");
                    grab_segdata(dyn, addr, ninst, x1, seg, (MODREG));
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEBO(x1, 1);
                    u8 = F8;
                    emit_xor8c(dyn, ninst, x1, u8, x2, x4);
                    EBBACK(x5, 0);
                    break;
                case 7: // CMP
                    INST_NAME("CMP Eb, Ib");
                    grab_segdata(dyn, addr, ninst, x1, seg, (MODREG));
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEBO(x1, 1);
                    u8 = F8;
                    if (u8) {
                        MOV32w(x2, u8);
                        emit_cmp8(dyn, ninst, x1, x2, x3, x4, x5, x6);
                    } else {
                        emit_cmp8_0(dyn, ninst, x1, x3, x4);
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0x81:
        case 0x83:
            nextop = F8;
            grab_segdata(dyn, addr, ninst, x6, seg, (MODREG));
            switch ((nextop >> 3) & 7) {
                case 0: // ADD
                    if (opcode == 0x81) {
                        INST_NAME("ADD Ed, Id");
                    } else {
                        INST_NAME("ADD Ed, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEDO(x6, (opcode == 0x81) ? 4 : 1, x5);
                    if (opcode == 0x81)
                        i64 = F32S;
                    else
                        i64 = F8S;
                    emit_add32c(dyn, ninst, rex, ed, i64, x3, x4, x5, x7);
                    WBACKO(x6);
                    break;
                case 1: // OR
                    if (opcode == 0x81) {
                        INST_NAME("OR Ed, Id");
                    } else {
                        INST_NAME("OR Ed, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEDO(x6, (opcode == 0x81) ? 4 : 1, x5);
                    if (opcode == 0x81)
                        i64 = F32S;
                    else
                        i64 = F8S;
                    emit_or32c(dyn, ninst, rex, ed, i64, x3, x4);
                    WBACKO(x6);
                    break;
                case 2: // ADC
                    if (opcode == 0x81) {
                        INST_NAME("ADC Ed, Id");
                    } else {
                        INST_NAME("ADC Ed, Ib");
                    }
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEDO(x6, (opcode == 0x81) ? 4 : 1, x5);
                    if (opcode == 0x81)
                        i64 = F32S;
                    else
                        i64 = F8S;
                    MOV64xw(x5, i64);
                    SD(x6, xEmu, offsetof(x64emu_t, scratch));
                    emit_adc32(dyn, ninst, rex, ed, x5, x3, x4, x6, x7);
                    LD(x6, xEmu, offsetof(x64emu_t, scratch));
                    WBACKO(x6);
                    break;
                case 3: // SBB
                    if (opcode == 0x81) {
                        INST_NAME("SBB Ed, Id");
                    } else {
                        INST_NAME("SBB Ed, Ib");
                    }
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEDO(x6, (opcode == 0x81) ? 4 : 1, x5);
                    if (opcode == 0x81)
                        i64 = F32S;
                    else
                        i64 = F8S;
                    MOV64xw(x5, i64);
                    emit_sbb32(dyn, ninst, rex, ed, x5, x3, x4, x7);
                    WBACKO(x6);
                    break;
                case 4: // AND
                    if (opcode == 0x81) {
                        INST_NAME("AND Ed, Id");
                    } else {
                        INST_NAME("AND Ed, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEDO(x6, (opcode == 0x81) ? 4 : 1, x5);
                    if (opcode == 0x81)
                        i64 = F32S;
                    else
                        i64 = F8S;
                    emit_and32c(dyn, ninst, rex, ed, i64, x3, x4);
                    WBACKO(x6);
                    break;
                case 5: // SUB
                    if (opcode == 0x81) {
                        INST_NAME("SUB Ed, Id");
                    } else {
                        INST_NAME("SUB Ed, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEDO(x6, (opcode == 0x81) ? 4 : 1, x5);
                    if (opcode == 0x81)
                        i64 = F32S;
                    else
                        i64 = F8S;
                    emit_sub32c(dyn, ninst, rex, ed, i64, x3, x4, x5, x7);
                    WBACKO(x6);
                    break;
                case 6: // XOR
                    if (opcode == 0x81) {
                        INST_NAME("XOR Ed, Id");
                    } else {
                        INST_NAME("XOR Ed, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEDO(x6, (opcode == 0x81) ? 4 : 1, x5);
                    if (opcode == 0x81)
                        i64 = F32S;
                    else
                        i64 = F8S;
                    emit_xor32c(dyn, ninst, rex, ed, i64, x3, x4);
                    WBACKO(x6);
                    break;
                case 7: // CMP
                    if (opcode == 0x81) {
                        INST_NAME("CMP Ed, Id");
                    } else {
                        INST_NAME("CMP Ed, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEDO(x6, (opcode == 0x81) ? 4 : 1, x5);
                    if (opcode == 0x81)
                        i64 = F32S;
                    else
                        i64 = F8S;
                    if (i64) {
                        MOV64x(x2, i64);
                        emit_cmp32(dyn, ninst, rex, ed, x2, x3, x4, x5, x6);
                    } else
                        emit_cmp32_0(dyn, ninst, rex, nextop, ed, x3, x4, x5);
                    break;
            }
            break;
        case 0x88:
            INST_NAME("MOV Seg:Eb, Gb");
            nextop = F8;
            grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
            gd = ((nextop & 0x38) >> 3) + (rex.r << 3);
            if (rex.rex) {
                gb2 = 0;
                gb1 = TO_NAT(gd);
            } else {
                gb2 = ((gd & 4) >> 2);
                gb1 = TO_NAT(gd & 3);
            }
            gd = x5;
            if (gb2) {
                SRLI(x5, gb1, 8);
                gb1 = x5;
            }
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                if (rex.rex) {
                    eb1 = TO_NAT(ed);
                    eb2 = 0;
                } else {
                    eb1 = TO_NAT(ed & 3);  // Ax, Cx, Dx or Bx
                    eb2 = ((ed & 4) >> 2); // L or H
                }
                ANDI(gd, gb1, 0xff);
                if (eb2) {
                    MOV64x(x1, 0xffffffffffff00ffLL);
                    ANDI(x1, eb1, x1);
                    SLLI(gd, gd, 8);
                    OR(eb1, x1, gd);
                } else {
                    ANDI(x1, eb1, ~0xff);
                    OR(eb1, x1, gd);
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                ADD(x4, ed, x4);
                SB(gb1, x4, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0x89:
            INST_NAME("MOV Seg:Ed, Gd");
            nextop = F8;
            grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
            GETGD;
            if (MODREG) { // reg <= reg
                MVxw(TO_NAT((nextop & 7) + (rex.b << 3)), gd);
            } else { // mem <= reg
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                ADDz(x4, ed, x4);
                SDxw(gd, x4, fixedaddress);
                SMWRITE2();
            }
            break;

        case 0x8B:
            INST_NAME("MOV Gd, Seg:Ed");
            nextop = F8;
            grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
            GETGD;
            if (MODREG) { // reg <= reg
                MVxw(gd, TO_NAT((nextop & 7) + (rex.b << 3)));
            } else { // mem <= reg
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                ADDz(x4, ed, x4);
                LDxw(gd, x4, fixedaddress);
            }
            break;
        case 0x8E:
            INST_NAME("MOV Seg,Ew");
            nextop = F8;
            grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
            u8 = (nextop & 0x38) >> 3;
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 0, 0);
                ADDz(x4, wback, x4);
                LHU(x1, x4, 0);
                ed = x1;
            }
            SH(ed, xEmu, offsetof(x64emu_t, segs[u8]));
            break;
        case 0x8F:
            INST_NAME("POP FS:Ed");
            nextop = F8;
            grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
            if (MODREG) {
                POP1z(TO_NAT((nextop & 7) + (rex.b << 3)));
            } else {
                POP1z(x3); // so this can handle POP [ESP] and maybe some variant too
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 0, 0);
                if (ed == xRSP) {
                    ADDz(x4, ed, x4);
                    SDz(x3, x4, 0);
                } else {
                    // complicated to just allow a segfault that can be recovered correctly
                    ADDIz(xRSP, xRSP, rex.is32bits ? -4 : -8);
                    ADDz(x4, ed, x4);
                    SDz(x3, x4, 0);
                    ADDIz(xRSP, xRSP, rex.is32bits ? 4 : 8);
                }
            }
            break;
        case 0xA1:
            INST_NAME("MOV EAX,FS:Od");
            grab_segdata(dyn, addr, ninst, x4, seg, 0);
            if (rex.is32bits)
                u64 = F32;
            else
                u64 = F64;
            if (u64 < 0x800) {
                ADDIz(x1, x4, u64);
            } else {
                MOV64z(x1, u64);
                ADDz(x1, x1, x4);
            }
            LDxw(xRAX, x1, 0);
            break;

        case 0xA3:
            INST_NAME("MOV FS:Od,EAX");
            grab_segdata(dyn, addr, ninst, x4, seg, 0);
            if (rex.is32bits)
                u64 = F32;
            else
                u64 = F64;
            if (u64 < 0x800) {
                ADDIz(x1, x4, u64);
            } else {
                MOV64z(x1, u64);
                ADDz(x1, x1, x4);
            }
            SDxw(xRAX, x1, 0);
            SMWRITE2();
            break;

        case 0xC6:
            INST_NAME("MOV Seg:Eb, Ib");
            nextop = F8;
            grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
            if (MODREG) { // reg <= u8
                u8 = F8;
                if (!rex.rex) {
                    ed = (nextop & 7);
                    eb1 = TO_NAT(ed & 3); // Ax, Cx, Dx or Bx
                    eb2 = (ed & 4) >> 2;  // L or H
                } else {
                    eb1 = TO_NAT((nextop & 7) + (rex.b << 3));
                    eb2 = 0;
                }

                if (eb2) {
                    // load a mask to x3 (ffffffffffff00ff)
                    LUI(x3, 0xffff0);
                    ORI(x3, x3, 0xff);
                    // apply mask
                    AND(eb1, eb1, x3);
                    if (u8) {
                        if ((u8 << 8) < 2048) {
                            ADDI(x4, xZR, u8 << 8);
                        } else {
                            ADDI(x4, xZR, u8);
                            SLLI(x4, x4, 8);
                        }
                        OR(eb1, eb1, x4);
                    }
                } else {
                    ANDI(eb1, eb1, 0xf00); // mask ffffffffffffff00
                    ORI(eb1, eb1, u8);
                }
            } else { // mem <= u8
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 1, 1);
                u8 = F8;
                if (u8) {
                    ADDI(x3, xZR, u8);
                    ed = x3;
                } else
                    ed = xZR;
                ADDz(x4, wback, x4);
                SB(ed, x4, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0xC7:
            INST_NAME("MOV Seg:Ed, Id");
            nextop = F8;
            grab_segdata(dyn, addr, ninst, x4, seg, (MODREG));
            if (MODREG) { // reg <= i32
                i64 = F32S;
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                MOV64xw(ed, i64);
            } else { // mem <= i32
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 1, 4);
                i64 = F32S;
                if (i64) {
                    MOV64x(x3, i64);
                    ed = x3;
                } else
                    ed = xZR;
                ADDz(x4, wback, x4);
                SDxw(ed, x4, fixedaddress);
                SMWRITE2();
            }
            break;

        case 0xFF:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 6: // Push Ed
                    INST_NAME("PUSH Ed");
                    grab_segdata(dyn, addr, ninst, x6, seg, (MODREG));
                    GETEDOz(x6, 0, x3);
                    PUSH1z(ed);
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
