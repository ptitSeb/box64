#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <assert.h>

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

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "../dynarec_helper.h"
#include "dynarec_rv64_functions.h"

uintptr_t dynarec64_67(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop;
    uint8_t gd, ed, wback, wb, wb1, wb2, gback, gb, gb1, gb2, eb1, eb2;
    int64_t fixedaddress, gdoffset;
    int unscaled;
    int8_t i8;
    uint8_t u8;
    int32_t i32;
    int64_t j64, i64;
    int cacheupd = 0;
    int lock;
    int v0, v1, s0;
    MAYUSE(i32);
    MAYUSE(j64);
    MAYUSE(v0);
    MAYUSE(v1);
    MAYUSE(s0);
    MAYUSE(lock);
    MAYUSE(cacheupd);

    if (rex.is32bits) {
        // should do a different file
        DEFAULT;
        return addr;
    }

    GETREX();

    rep = 0;
    while ((opcode == 0xF2) || (opcode == 0xF3)) {
        rep = opcode - 0xF1;
        opcode = F8;
    }

    switch (opcode) {

        case 0x01:
            INST_NAME("ADD Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED32(0);
            emit_add32(dyn, ninst, rex, ed, gd, x3, x4, x5);
            WBACK;
            break;
        case 0x02:
            INST_NAME("ADD Gb, Eb");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            FAST_8BIT_OPERATION(gb, wb, x1, ADD(gb, gb, x1));
            GETEB32(x2, 0);
            GETGB(x1);
            emit_add8(dyn, ninst, x1, x2, x3, x4, x6);
            GBBACK(x4);
            break;
        case 0x03:
            INST_NAME("ADD Gd, Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED32(0);
            emit_add32(dyn, ninst, rex, gd, ed, x3, x4, x5);
            break;

        case 0x05:
            INST_NAME("ADD EAX, Id");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i64 = F32S;
            emit_add32c(dyn, ninst, rex, xRAX, i64, x3, x4, x5, x6);
            break;

        case 0x09:
            INST_NAME("OR Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED32(0);
            emit_or32(dyn, ninst, rex, ed, gd, x3, x4);
            WBACK;
            break;
        case 0x0A:
            INST_NAME("OR Gb, Eb");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            FAST_8BIT_OPERATION(gb, wb, x1, OR(gb, gb, x1));
            GETEB32(x2, 0);
            GETGB(x1);
            emit_or8(dyn, ninst, x1, x2, x3, x4);
            GBBACK(x4);
            break;
        case 0x0B:
            INST_NAME("OR Gd, Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED32(0);
            emit_or32(dyn, ninst, rex, gd, ed, x3, x4);
            break;

        case 0x0D:
            INST_NAME("OR EAX, Id");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i64 = F32S;
            emit_or32c(dyn, ninst, rex, xRAX, i64, x3, x4);
            break;

        case 0x0F:
            opcode = F8;
            switch (opcode) {
                case 0x11:
                    switch (rep) {
                        case 0:
                            INST_NAME("MOVUPS Ex, Gx");
                            nextop = F8;
                            GETGX();
                            GETEX32(x2, 0, 8);
                            LD(x3, gback, gdoffset + 0);
                            LD(x4, gback, gdoffset + 8);
                            SD(x3, wback, fixedaddress + 0);
                            SD(x4, wback, fixedaddress + 8);
                            if (!MODREG) SMWRITE2();
                            break;
                        default:
                            DEFAULT;
                    }
                    break;
                case 0x2E:
                    // no special check...
                case 0x2F:
                    switch (rep) {
                        case 0:
                            if (opcode == 0x2F) {
                                INST_NAME("COMISS Gx, Ex");
                            } else {
                                INST_NAME("UCOMISS Gx, Ex");
                            }
                            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
                            nextop = F8;
                            GETGXSS(s0);
                            if (MODREG) {
                                v0 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 1);
                            } else {
                                v0 = fpu_get_scratch(dyn);
                                SMREAD();
                                addr = geted32(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                                FLW(v0, ed, fixedaddress);
                            }
                            CLEAR_FLAGS();
                            // if isnan(s0) || isnan(v0)
                            IFX (X_ZF | X_PF | X_CF) {
                                FEQS(x3, s0, s0);
                                FEQS(x2, v0, v0);
                                AND(x2, x2, x3);
                                BNE_MARK(x2, xZR);
                                ORI(xFlags, xFlags, (1 << F_ZF) | (1 << F_PF) | (1 << F_CF));
                                B_NEXT_nocond;
                            }
                            MARK;
                            // else if isless(d0, v0)
                            IFX (X_CF) {
                                FLTS(x2, s0, v0);
                                BEQ_MARK2(x2, xZR);
                                ORI(xFlags, xFlags, 1 << F_CF);
                                B_NEXT_nocond;
                            }
                            MARK2;
                            // else if d0 == v0
                            IFX (X_ZF) {
                                FEQS(x2, s0, v0);
                                CBZ_NEXT(x2);
                                ORI(xFlags, xFlags, 1 << F_ZF);
                            }
                            break;
                        default:
                            DEFAULT;
                    }
                    break;
                case 0xB6:
                    INST_NAME("MOVZX Gd, Eb");
                    nextop = F8;
                    GETGD;
                    SCRATCH_USAGE(0);
                    if (MODREG) {
                        if (rex.rex) {
                            eb1 = TO_NAT((nextop & 7) + (rex.b << 3));
                            eb2 = 0;
                        } else {
                            ed = (nextop & 7);
                            eb1 = TO_NAT(ed & 3); // Ax, Cx, Dx or Bx
                            eb2 = (ed & 4) >> 2;  // L or H
                        }
                        if (eb2) {
                            SRLI(gd, eb1, 8);
                            ANDI(gd, gd, 0xff);
                        } else {
                            ANDI(gd, eb1, 0xff);
                        }
                    } else {
                        SMREAD();
                        addr = geted32(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                        LBU(gd, ed, fixedaddress);
                    }
                    break;
                case 0xB7:
                    INST_NAME("MOVZX Gd, Ew");
                    nextop = F8;
                    GETGD;
                    SCRATCH_USAGE(0);
                    if (MODREG) {
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        ZEXTH(gd, ed);
                    } else {
                        SMREAD();
                        addr = geted32(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                        LHU(gd, ed, fixedaddress);
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;

        case 0x11:
            INST_NAME("ADC Ed, Gd");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED32(0);
            emit_adc32(dyn, ninst, rex, ed, gd, x3, x4, x5, x6);
            WBACK;
            break;

        case 0x13:
            INST_NAME("ADC Gd, Ed");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED32(0);
            emit_adc32(dyn, ninst, rex, gd, ed, x3, x4, x5, x6);
            break;

        case 0x15:
            INST_NAME("ADC EAX, Id");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i64 = F32S;
            MOV64xw(x1, i64);
            emit_adc32(dyn, ninst, rex, xRAX, x1, x3, x4, x5, x6);
            break;

        case 0x19:
            INST_NAME("SBB Ed, Gd");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED32(0);
            emit_sbb32(dyn, ninst, rex, ed, gd, x3, x4, x5);
            WBACK;
            break;
        case 0x1A:
            INST_NAME("SBB Gb, Eb");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            FAST_8BIT_OPERATION(gb, wb, x1, {
                SUB(gb, gb, x1);
                ANDI(x2, xFlags, 1 << F_CF);
                SLLI(x2, x2, 64 - 8);
                SUB(gb, gb, x2);
            });
            GETEB32(x2, 0);
            GETGB(x1);
            emit_sbb8(dyn, ninst, x1, x2, x3, x4, x5);
            GBBACK(x4);
            break;
        case 0x1B:
            INST_NAME("SBB Gd, Ed");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED32(0);
            emit_sbb32(dyn, ninst, rex, gd, ed, x3, x4, x5);
            break;

        case 0x1D:
            INST_NAME("SBB EAX, Id");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i64 = F32S;
            MOV64xw(x2, i64);
            emit_sbb32(dyn, ninst, rex, xRAX, x2, x3, x4, x5);
            break;

        case 0x21:
            INST_NAME("AND Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED32(0);
            emit_and32(dyn, ninst, rex, ed, gd, x3, x4);
            WBACK;
            break;
        case 0x22:
            INST_NAME("AND Gb, Eb");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETEB32(x2, 0);
            GETGB(x1);
            emit_and8(dyn, ninst, x1, x2, x3, x4);
            GBBACK(x4);
            break;
        case 0x23:
            INST_NAME("AND Gd, Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED32(0);
            emit_and32(dyn, ninst, rex, gd, ed, x3, x4);
            break;

        case 0x25:
            INST_NAME("AND EAX, Id");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i64 = F32S;
            emit_and32c(dyn, ninst, rex, xRAX, i64, x3, x4);
            break;

        case 0x29:
            INST_NAME("SUB Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED32(0);
            emit_sub32(dyn, ninst, rex, ed, gd, x3, x4, x5);
            WBACK;
            break;
        case 0x2A:
            INST_NAME("SUB Gb, Eb");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            FAST_8BIT_OPERATION(gb, wb, x1, SUB(gb, gb, x1));
            GETEB32(x2, 0);
            GETGB(x1);
            emit_sub8(dyn, ninst, x1, x2, x3, x4, x5);
            GBBACK(x5);
            break;
        case 0x2B:
            INST_NAME("SUB Gd, Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED32(0);
            emit_sub32(dyn, ninst, rex, gd, ed, x3, x4, x5);
            break;

        case 0x2D:
            INST_NAME("SUB EAX, Id");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i64 = F32S;
            emit_sub32c(dyn, ninst, rex, xRAX, i64, x3, x4, x5, x6);
            break;

        case 0x31:
            INST_NAME("XOR Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED32(0);
            emit_xor32(dyn, ninst, rex, ed, gd, x3, x4);
            WBACK;
            break;
        case 0x32:
            INST_NAME("XOR Gb, Eb");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            FAST_8BIT_OPERATION(gb, wb, x1, XOR(gb, gb, x1));
            GETEB32(x2, 0);
            GETGB(x1);
            emit_xor8(dyn, ninst, x1, x2, x3, x4);
            GBBACK(x4);
            break;
        case 0x33:
            INST_NAME("XOR Gd, Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED32(0);
            emit_xor32(dyn, ninst, rex, gd, ed, x3, x4);
            break;

        case 0x35:
            INST_NAME("XOR EAX, Id");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i64 = F32S;
            emit_xor32c(dyn, ninst, rex, xRAX, i64, x3, x4);
            break;

        case 0x38:
            INST_NAME("CMP Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETEB32(x1, 0);
            GETGB(x2);
            emit_cmp8(dyn, ninst, x1, x2, x3, x4, x5, x6);
            break;
        case 0x39:
            INST_NAME("CMP Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED32(0);
            emit_cmp32(dyn, ninst, rex, ed, gd, x3, x4, x5, x6);
            break;
        case 0x3A:
            INST_NAME("CMP Gb, Eb");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETEB32(x2, 0);
            GETGB(x1);
            emit_cmp8(dyn, ninst, x1, x2, x3, x4, x5, x6);
            break;
        case 0x3B:
            INST_NAME("CMP Gd, Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED32(0);
            emit_cmp32(dyn, ninst, rex, gd, ed, x3, x4, x5, x6);
            break;
        case 0x3C:
            INST_NAME("CMP AL, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            u8 = F8;
            ANDI(x1, xRAX, 0xff);
            if (u8) {
                MOV32w(x2, u8);
                emit_cmp8(dyn, ninst, x1, x2, x3, x4, x5, x6);
            } else {
                emit_cmp8_0(dyn, ninst, x1, x3, x4);
            }
            break;
        case 0x3D:
            INST_NAME("CMP EAX, Id");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i64 = F32S;
            if (i64) {
                MOV64x(x2, i64);
                emit_cmp32(dyn, ninst, rex, xRAX, x2, x3, x4, x5, x6);
            } else
                emit_cmp32_0(dyn, ninst, rex, nextop, xRAX, x3, x4, x5);
            break;
        case 0x63:
            INST_NAME("MOVSXD Gd, Ed");
            nextop = F8;
            GETGD;
            SCRATCH_USAGE(0);
            if (rex.w) {
                if (MODREG) { // reg <= reg
                    ADDIW(gd, TO_NAT((nextop & 7) + (rex.b << 3)), 0);
                } else { // mem <= reg
                    SMREAD();
                    addr = geted32(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                    LW(gd, ed, fixedaddress);
                }
            } else {
                if (MODREG) { // reg <= reg
                    ZEXTW2(gd, TO_NAT((nextop & 7) + (rex.b << 3)));
                } else { // mem <= reg
                    SMREAD();
                    addr = geted32(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                    LWU(gd, ed, fixedaddress);
                }
            }
            break;
        case 0x66:
            opcode = F8;
            switch (opcode) {
                case 0x89:
                    INST_NAME("MOV Ew, Gw");
                    nextop = F8;
                    GETGD; // don't need GETGW here
                    if (MODREG) {
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        if (ed != gd) {
                            LUI(x1, 0xffff0);
                            AND(ed, ed, x1);
                            ZEXTH(x2, gd);
                            OR(ed, ed, x2);
                        }
                    } else {
                        SCRATCH_USAGE(0);
                        addr = geted32(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, 1, 0);
                        SH(gd, ed, fixedaddress);
                        SMWRITELOCK(lock);
                    }
                    break;

                default:
                    DEFAULT;
            }
            break;

        case 0x81:
        case 0x83:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0: // ADD
                    if (opcode == 0x81) {
                        INST_NAME("ADD Ed, Id");
                    } else {
                        INST_NAME("ADD Ed, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETED32((opcode == 0x81) ? 4 : 1);
                    if (opcode == 0x81)
                        i64 = F32S;
                    else
                        i64 = F8S;
                    emit_add32c(dyn, ninst, rex, ed, i64, x3, x4, x5, x6);
                    WBACK;
                    break;
                case 1: // OR
                    if (opcode == 0x81) {
                        INST_NAME("OR Ed, Id");
                    } else {
                        INST_NAME("OR Ed, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETED32((opcode == 0x81) ? 4 : 1);
                    if (opcode == 0x81)
                        i64 = F32S;
                    else
                        i64 = F8S;
                    emit_or32c(dyn, ninst, rex, ed, i64, x3, x4);
                    WBACK;
                    break;
                case 2: // ADC
                    if (opcode == 0x81) {
                        INST_NAME("ADC Ed, Id");
                    } else {
                        INST_NAME("ADC Ed, Ib");
                    }
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETED32((opcode == 0x81) ? 4 : 1);
                    if (opcode == 0x81)
                        i64 = F32S;
                    else
                        i64 = F8S;
                    MOV64xw(x5, i64);
                    emit_adc32(dyn, ninst, rex, ed, x5, x3, x4, x6, x7);
                    WBACK;
                    break;
                case 3: // SBB
                    if (opcode == 0x81) {
                        INST_NAME("SBB Ed, Id");
                    } else {
                        INST_NAME("SBB Ed, Ib");
                    }
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETED32((opcode == 0x81) ? 4 : 1);
                    if (opcode == 0x81)
                        i64 = F32S;
                    else
                        i64 = F8S;
                    MOV64xw(x5, i64);
                    emit_sbb32(dyn, ninst, rex, ed, x5, x3, x4, x5);
                    WBACK;
                    break;
                case 4: // AND
                    if (opcode == 0x81) {
                        INST_NAME("AND Ed, Id");
                    } else {
                        INST_NAME("AND Ed, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETED32((opcode == 0x81) ? 4 : 1);
                    if (opcode == 0x81)
                        i64 = F32S;
                    else
                        i64 = F8S;
                    emit_and32c(dyn, ninst, rex, ed, i64, x3, x4);
                    WBACK;
                    break;
                case 5: // SUB
                    if (opcode == 0x81) {
                        INST_NAME("SUB Ed, Id");
                    } else {
                        INST_NAME("SUB Ed, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETED32((opcode == 0x81) ? 4 : 1);
                    if (opcode == 0x81)
                        i64 = F32S;
                    else
                        i64 = F8S;
                    emit_sub32c(dyn, ninst, rex, ed, i64, x3, x4, x5, x6);
                    WBACK;
                    break;
                case 6: // XOR
                    if (opcode == 0x81) {
                        INST_NAME("XOR Ed, Id");
                    } else {
                        INST_NAME("XOR Ed, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETED32((opcode == 0x81) ? 4 : 1);
                    if (opcode == 0x81)
                        i64 = F32S;
                    else
                        i64 = F8S;
                    emit_xor32c(dyn, ninst, rex, ed, i64, x3, x4);
                    WBACK;
                    break;
                case 7: // CMP
                    if (opcode == 0x81) {
                        INST_NAME("CMP Ed, Id");
                    } else {
                        INST_NAME("CMP Ed, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETED32((opcode == 0x81) ? 4 : 1);
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
            INST_NAME("MOV Eb, Gb");
            nextop = F8;
            gd = ((nextop & 0x38) >> 3) + (rex.r << 3);
            if (rex.rex) {
                gb2 = 0;
                gb1 = TO_NAT(gd);
            } else {
                gb2 = ((gd & 4) >> 2);
                gb1 = TO_NAT(gd & 3);
            }
            gd = x4;
            if (gb2) {
                SRLI(x4, gb1, 8);
                gb1 = x4;
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
                    AND(x1, eb1, x1);
                    SLLI(gd, gd, 8);
                    OR(eb1, x1, gd);
                } else {
                    ANDI(x1, eb1, ~0xff);
                    OR(eb1, x1, gd);
                }
            } else {
                addr = geted32(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, 1, 0);
                SB(gb1, ed, fixedaddress);
                SMWRITELOCK(lock);
            }
            break;
        case 0x89:
            INST_NAME("MOV Ed, Gd");
            nextop = F8;
            GETGD;
            SCRATCH_USAGE(0);
            if (MODREG) { // reg <= reg
                MVxw(TO_NAT((nextop & 7) + (rex.b << 3)), gd);
            } else { // mem <= reg
                addr = geted32(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, 1, 0);
                SDxw(gd, ed, fixedaddress);
                SMWRITELOCK(lock);
            }
            break;
        case 0x8B:
            INST_NAME("MOV Gd, Ed");
            nextop = F8;
            GETGD;
            SCRATCH_USAGE(0);
            if (MODREG) {
                MVxw(gd, TO_NAT((nextop & 7) + (rex.b << 3)));
            } else {
                addr = geted32(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, 1, 0);
                SMREADLOCK(lock);
                LDxw(gd, ed, fixedaddress);
            }
            break;
        case 0x8D:
            INST_NAME("LEA Gd, Ed");
            nextop = F8;
            GETGD;
            if (MODREG) { // reg <= reg? that's an invalid operation
                DEFAULT;
            } else { // mem <= reg
                SCRATCH_USAGE(0);
                addr = geted32(dyn, addr, ninst, nextop, &ed, gd, x1, &fixedaddress, rex, NULL, 0, 0);
                ZEXTW2(gd, ed);
            }
            break;

        case 0xC1:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 5:
                    INST_NAME("SHR Ed, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop) & (rex.w ? 0x3f : 0x1f);
                    // flags are not affected if count is 0, we make it a nop if possible.
                    if (u8) {
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
                        GETED32(1);
                        F8;
                        emit_shr32c(dyn, ninst, rex, ed, u8, x3, x4);
                        WBACK;
                    } else {
                        if (MODREG && !rex.w) {
                            GETED(1);
                            ZEROUP(ed);
                        } else {
                            FAKEED;
                        }
                        F8;
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;

        case 0xC7:
            INST_NAME("MOV Ed, Id");
            nextop = F8;
            if (MODREG) { // reg <= i32
                SCRATCH_USAGE(0);
                i64 = F32S;
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                MOV64xw(ed, i64);
            } else { // mem <= i32
                addr = geted32(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, 1, 4);
                i64 = F32S;
                MOV64x(x3, i64);
                SDxw(x3, ed, fixedaddress);
                SMWRITELOCK(lock);
            }
            break;

#define GO(Z)                                                                               \
    BARRIER(BARRIER_MAYBE);                                                                 \
    JUMP(addr + i8, 1);                                                                     \
    if (dyn->insts[ninst].x64.jmp_insts == -1 || CHECK_CACHE()) {                           \
        /* out of the block */                                                              \
        i32 = dyn->insts[ninst].epilog - (dyn->native_size);                                \
        if (Z) {                                                                            \
            BNE(x1, xZR, i32);                                                              \
        } else {                                                                            \
            BEQ(x1, xZR, i32);                                                              \
        };                                                                                  \
        if (dyn->insts[ninst].x64.jmp_insts == -1) {                                        \
            if (!(dyn->insts[ninst].x64.barrier & BARRIER_FLOAT))                           \
                fpu_purgecache(dyn, ninst, 1, x1, x2, x3);                                  \
            jump_to_next(dyn, addr + i8, 0, ninst, rex.is32bits);                           \
        } else {                                                                            \
            CacheTransform(dyn, ninst, cacheupd, x1, x2, x3);                               \
            i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address - (dyn->native_size); \
            B(i32);                                                                         \
        }                                                                                   \
    } else {                                                                                \
        /* inside the block */                                                              \
        i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address - (dyn->native_size);     \
        if (Z) {                                                                            \
            BEQ(x1, xZR, i32);                                                              \
        } else {                                                                            \
            BNE(x1, xZR, i32);                                                              \
        };                                                                                  \
    }
        case 0xE0:
            INST_NAME("LOOPNZ (32bits)");
            READFLAGS(X_ZF);

            i8 = F8S;
            SUBI(xRCX, xRCX, 1);
            ANDI(x1, xFlags, 1 << F_ZF);
            CBNZ_NEXT(x1);
            ZEXTW2(x1, xRCX);
            GO(0);
            break;
        case 0xE1:
            INST_NAME("LOOPZ (32bits)");
            READFLAGS(X_ZF);
            i8 = F8S;
            SUBI(xRCX, xRCX, 1);
            ANDI(x1, xFlags, 1 << F_ZF);
            CBZ_NEXT(x1);
            ZEXTW2(x1, xRCX);
            GO(0);
            break;
        case 0xE2:
            INST_NAME("LOOP (32bits)");
            i8 = F8S;
            SUBI(xRCX, xRCX, 1);
            ZEXTW2(x1, xRCX);
            GO(0);
            break;
        case 0xE3:
            INST_NAME("JECXZ (32bits)");
            i8 = F8S;
            ZEXTW2(x1, xRCX);
            GO(1);
            break;
#undef GO

        case 0xE8:
            return dynarec64_00(dyn, addr - 1, ip, ninst, rex, rep, ok, need_epilog); // addr-1, to "put back" opcode)

        case 0xF7:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 4:
                    INST_NAME("MUL EAX, Ed");
                    SETFLAGS(X_ALL, SF_PENDING, NAT_FLAGS_NOFUSION);
                    GETED32(0);
                    if (rex.w) {
                        if (ed == xRDX)
                            gd = x3;
                        else
                            gd = xRDX;
                        MULHU(gd, xRAX, ed);
                        MUL(xRAX, xRAX, ed);
                        if (gd != xRDX) MV(xRDX, gd);
                    } else {
                        MUL(xRDX, xRAX, ed); // 64 <- 32x32
                        ZEXTW2(xRAX, xRDX);
                        SRLIW(xRDX, xRDX, 32);
                    }
                    UFLAG_RES(xRAX);
                    UFLAG_OP1(xRDX);
                    UFLAG_DF(x2, rex.w ? d_mul64 : d_mul32);
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
