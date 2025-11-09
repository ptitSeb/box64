#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <signal.h>
#include <assert.h>

#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "bridge.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"
#include "custommem.h"

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_functions.h"
#include "../dynarec_helper.h"

uintptr_t dynarec64_00_2(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
{
    uint8_t nextop, opcode;
    uint8_t gd, ed;
    int8_t i8;
    int32_t i32, tmp;
    int64_t i64, j64;
    uint8_t u8;
    uint8_t gb1, gb2, eb1, eb2;
    uint32_t u32;
    uint64_t u64;
    uint8_t wback, wb1, wb2, wb;
    int64_t fixedaddress;
    int lock;
    int cacheupd = 0;

    opcode = F8;
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(j64);
    MAYUSE(wb);
    MAYUSE(lock);
    MAYUSE(cacheupd);

    switch (opcode) {
        case 0x80:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0: // ADD
                    INST_NAME("ADD Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEB(x1, 1);
                    u8 = F8;
                    emit_add8c(dyn, ninst, x1, u8, x2, x4, x5, x6);
                    EBBACK(x5, 0);
                    break;
                case 1: // OR
                    INST_NAME("OR Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEB(x1, 1);
                    u8 = F8;
                    emit_or8c(dyn, ninst, x1, u8, x2, x4, x5);
                    EBBACK(x5, 0);
                    break;
                case 2: // ADC
                    INST_NAME("ADC Eb, Ib");
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEB(x1, 1);
                    u8 = F8;
                    emit_adc8c(dyn, ninst, x1, u8, x2, x4, x5, x6);
                    EBBACK(x5, 0);
                    break;
                case 3: // SBB
                    INST_NAME("SBB Eb, Ib");
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEB(x1, 1);
                    u8 = F8;
                    emit_sbb8c(dyn, ninst, x1, u8, x2, x4, x5, x6);
                    EBBACK(x5, 0);
                    break;
                case 4: // AND
                    INST_NAME("AND Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEB(x1, 1);
                    u8 = F8;
                    emit_and8c(dyn, ninst, x1, u8, x2, x4);
                    EBBACK(x5, 0);
                    break;
                case 5: // SUB
                    INST_NAME("SUB Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEB(x1, 1);
                    u8 = F8;
                    emit_sub8c(dyn, ninst, x1, u8, x2, x4, x5, x6);
                    EBBACK(x5, 0);
                    break;
                case 6: // XOR
                    INST_NAME("XOR Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEB(x1, 1);
                    u8 = F8;
                    emit_xor8c(dyn, ninst, x1, u8, x2, x4);
                    EBBACK(x5, 0);
                    break;
                case 7: // CMP
                    INST_NAME("CMP Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEB(x1, 1);
                    u8 = F8;
                    if (u8) {
                        ADDI(x2, xZR, u8);
                        emit_cmp8(dyn, ninst, x1, x2, x7, x4, x5, x6);
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
            switch ((nextop >> 3) & 7) {
                case 0: // ADD
                    if (opcode == 0x81) {
                        INST_NAME("ADD Ed, Id");
                    } else {
                        INST_NAME("ADD Ed, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETED((opcode == 0x81) ? 4 : 1);
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
                    GETED((opcode == 0x81) ? 4 : 1);
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
                    GETED((opcode == 0x81) ? 4 : 1);
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
                    GETED((opcode == 0x81) ? 4 : 1);
                    if (opcode == 0x81)
                        i64 = F32S;
                    else
                        i64 = F8S;
                    MOV64xw(x5, i64);
                    emit_sbb32(dyn, ninst, rex, ed, x5, x3, x4, x6);
                    WBACK;
                    break;
                case 4: // AND
                    if (opcode == 0x81) {
                        INST_NAME("AND Ed, Id");
                    } else {
                        INST_NAME("AND Ed, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETED((opcode == 0x81) ? 4 : 1);
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
                    GETED((opcode == 0x81) ? 4 : 1);
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
                    GETED((opcode == 0x81) ? 4 : 1);
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
                    GETED((opcode == 0x81) ? 4 : 1);
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
        case 0x84:
            INST_NAME("TEST Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETEB(x1, 0);
            if (GB_EQ_EB())
                u8 = x1;
            else {
                GETGB(x2);
                u8 = x2;
            }
            emit_test8(dyn, ninst, x1, u8, x6, x4, x5);
            break;
        case 0x85:
            INST_NAME("TEST Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_test32(dyn, ninst, rex, ed, gd, x3, x4, x5);
            break;
        case 0x86:
            INST_NAME("(LOCK) XCHG Eb, Gb");
            nextop = F8;
            if (MODREG) {
                GETGB(x1);
                GETEB(x2, 0);
                MV(x4, gd);
                MV(gd, ed);
                MV(ed, x4);
                GBBACK(x4);
                EBBACK(x4, 0);
            } else {
                GETGB(x3);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);

                // calculate shift amount
                ANDI(x1, ed, 0x3);
                SLLI(x1, x1, 3);

                // align address to 4-bytes to use ll.w/sc.w
                ANDI(x6, ed, ~3);

                // prepare mask
                ADDI(x4, xZR, 0xff);
                SLL(x4, x4, x1);
                NOT(x4, x4);
                SLL(x7, gd, x1);

                // do aligned ll/sc sequence, reusing x2 (ed might be x2 but is no longer needed)
                MARKLOCK;
                LR_W(x2, x6, 1, 1);
                AND(x5, x2, x4);
                OR(x5, x5, x7);
                SC_W(x5, x5, x6, 1, 1);
                BNEZ_MARKLOCK(x5);

                // extract loaded byte
                SRL(gd, x2, x1);
                ANDI(gd, gd, 0xff);

                GBBACK(x5);
            }
            break;
        case 0x87:
            INST_NAME("(LOCK) XCHG Ed, Gd");
            nextop = F8;
            if (MODREG) {
                GETGD;
                GETED(0);
                MVxw(x3, gd);
                MVxw(gd, ed);
                MVxw(ed, x3);
            } else {
                GETGD;
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                ANDI(x3, ed, (1 << (2 + rex.w)) - 1);
                BNE_MARK(x3, xZR);
                AMOSWAPxw(gd, gd, ed, 1, 1);
                if (!rex.w) ZEROUP(gd);
                B_NEXT_nocond;
                MARK;
                // Unaligned
                ANDI(x5, ed, -(1 << (rex.w + 2)));
                MARKLOCK;
                LDxw(x1, ed, 0);
                LRxw(x3, x5, 1, 1);
                SCxw(x4, x3, x5, 1, 1);
                BNEZ_MARKLOCK(x4);
                SDxw(gd, ed, 0);
                MVxw(gd, x1);
                SMDMB();
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
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, 1, 0);
                SB(gb1, ed, fixedaddress);
                SMWRITELOCK(lock);
            }
            break;
        case 0x89:
            INST_NAME("MOV Ed, Gd");
            nextop = F8;
            GETGD;
            if (MODREG) { // reg <= reg
                SCRATCH_USAGE(0);
                MVxw(TO_NAT((nextop & 7) + (rex.b << 3)), gd);
            } else { // mem <= reg
                IF_UNALIGNED(ip) {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, (1 << (2 + rex.w)) - 1, 0);
                    for (int i = 0; i < (1 << (2 + rex.w)); i++) {
                        if (i == 0) {
                            SB(gd, ed, fixedaddress);
                        } else {
                            SRLI(x3, gd, i * 8);
                            SB(x3, ed, fixedaddress + i);
                        }
                    }
                } else {
                    SCRATCH_USAGE(0);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, 1, 0);
                    if(!lock && BOX64ENV(unity) && !VolatileRangesContains(ip) && ((fixedaddress==0x80) || (fixedaddress==0x84) || (fixedaddress==0xc0) || (fixedaddress==0xc4))) {
                        DMB_ISH();
                        lock = 1;
                    }
                    SDxw(gd, ed, fixedaddress);
                }
                SMWRITELOCK(lock);
            }
            break;
        case 0x8A:
            INST_NAME("MOV Gb, Eb");
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
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                if (rex.rex) {
                    eb1 = TO_NAT(ed);
                    eb2 = 0;
                } else {
                    eb1 = TO_NAT(ed & 3);  // Ax, Cx, Dx or Bx
                    eb2 = ((ed & 4) >> 2); // L or H
                }
                if (eb2) {
                    SRLI(x1, eb1, 8);
                    ANDI(x1, x1, 0xff);
                } else {
                    ANDI(x1, eb1, 0xff);
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, 1, 0);
                SMREADLOCK(lock);
                LBU(x1, ed, fixedaddress);
            }
            if (gb2) {
                MOV64x(x4, ~0xff00);
                AND(gb1, gb1, x4);
                SLLI(x1, x1, 8);
            } else {
                ANDI(gb1, gb1, ~0xff);
            }
            OR(gb1, gb1, x1);
            break;
        case 0x8B:
            INST_NAME("MOV Gd, Ed");
            nextop = F8;
            GETGD;
            SCRATCH_USAGE(0);
            if (MODREG) {
                MVxw(gd, TO_NAT((nextop & 7) + (rex.b << 3)));
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, 1, 0);
                SMREADLOCK(lock);
                LDxw(gd, ed, fixedaddress);
            }
            break;
        case 0x8C:
            INST_NAME("MOV Ed, Seg");
            nextop = F8;
            if (MODREG) {
                SCRATCH_USAGE(0);
                LHU(TO_NAT((nextop & 7) + (rex.b << 3)), xEmu, offsetof(x64emu_t, segs[(nextop & 0x38) >> 3]));
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                LHU(x3, xEmu, offsetof(x64emu_t, segs[(nextop & 0x38) >> 3]));
                SH(x3, ed, fixedaddress);
                SMWRITE2();
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
                addr = geted(dyn, addr, ninst, nextop, &ed, gd, x1, &fixedaddress, rex, NULL, 0, 0);
                if (gd != ed) {
                    MVxw(gd, ed);
                } else if (!rex.w && !rex.is32bits) {
                    ZEROUP(gd); // truncate the higher 32bits as asked
                }
            }
            break;
        case 0x8E:
            INST_NAME("MOV Seg,Ew");
            nextop = F8;
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                LHU(x1, ed, fixedaddress);
                ed = x1;
            }
            SH(ed, xEmu, offsetof(x64emu_t, segs[(nextop & 0x38) >> 3]));
            if((u8==_FS) || (u8==_GS)) {
                // refresh offset if needed
                CBZ_NEXT(ed);
                MOV32w(x1, u8);
                CALL(const_getsegmentbase, -1, x1, x2);
            }
            break;
        case 0x8F:
            INST_NAME("POP Ed");
            nextop = F8;
            if (MODREG) {
                POP1z(TO_NAT((nextop & 7) + (rex.b << 3)));
            } else {
                POP1z(x2); // so this can handle POP [ESP] and maybe some variant too
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, x1, &fixedaddress, rex, NULL, 1, 0);
                if (ed == xRSP) {
                    SDz(x2, ed, fixedaddress);
                } else {
                    // complicated to just allow a segfault that can be recovered correctly
                    ADDIz(xRSP, xRSP, rex.is32bits ? -4 : -8);
                    SDz(x2, ed, fixedaddress);
                    ADDIz(xRSP, xRSP, rex.is32bits ? 4 : 8);
                }
            }
            break;
        case 0x90:
        case 0x91:
        case 0x92:
        case 0x93:
        case 0x94:
        case 0x95:
        case 0x96:
        case 0x97:
            gd = TO_NAT((opcode & 0x07) + (rex.b << 3));
            if (gd == xRAX) {
                INST_NAME("NOP");
            } else {
                INST_NAME("XCHG EAX, Reg");
                MVxw(x2, xRAX);
                MVxw(xRAX, gd);
                MVxw(gd, x2);
            }
            break;
        case 0x98:
            INST_NAME("CWDE");
            if (rex.w) {
                SEXT_W(xRAX, xRAX);
            } else {
                SLLI(xRAX, xRAX, 16);
                SRAIW(xRAX, xRAX, 16);
                ZEROUP(xRAX);
            }
            break;
        case 0x99:
            INST_NAME("CDQ");
            if (rex.w) {
                SRAI(xRDX, xRAX, 63);
            } else {
                SLLI(xRDX, xRAX, 32);
                SRAI(xRDX, xRDX, 63);
                ZEROUP(xRDX);
            }
            break;
        case 0x9B:
            INST_NAME("FWAIT");
            break;
        case 0x9C:
            INST_NAME("PUSHF");
            READFLAGS(X_ALL);
            FLAGS_ADJUST_TO11(x3, xFlags, x2);
            PUSH1z(x3);
            break;
        case 0x9D:
            INST_NAME("POPF");
            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            POP1z(xFlags);
            FLAGS_ADJUST_FROM11(xFlags, xFlags, x2);
            MOV32w(x1, 0x3F7FF7);
            AND(xFlags, xFlags, x1);
            ORI(xFlags, xFlags, 0x202);
            SET_DFNONE();
            if (box64_wine) { // should this be done all the time?
                ANDI(x1, xFlags, 1 << F_TF);
                CBZ_NEXT(x1);
                // go to epilog, TF should trigger at end of next opcode, so using Interpreter only
                jump_to_epilog(dyn, addr, 0, ninst);
            }
            break;
        case 0x9E:
            INST_NAME("SAHF");
            SETFLAGS(X_CF | X_PF | X_AF | X_ZF | X_SF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            ADDI(x1, xZR, ~0b11010101);
            AND(xFlags, xFlags, x1);
            NOT(x1, x1);
            SRLI(x2, xRAX, 8);
            AND(x1, x1, x2);
            OR(xFlags, xFlags, x1);
            SET_DFNONE();
            break;
        case 0x9F:
            INST_NAME("LAHF");
            READFLAGS(X_CF | X_PF | X_AF | X_ZF | X_SF);
            ANDI(x1, xFlags, 0b11010111); // leave reserved bits out (we are using one as OF2)
            SLLI(x1, x1, 8);
            MOV64x(x2, 0xffffffffffff00ffLL);
            AND(xRAX, xRAX, x2);
            OR(xRAX, xRAX, x1);
            break;
        case 0xA0:
            INST_NAME("MOV AL,Ob");
            if (rex.is32bits)
                u64 = F32;
            else
                u64 = F64;
            MOV64z(x1, u64);
            LBU(x1, x1, 0);
            ANDI(xRAX, xRAX, ~0xff);
            OR(xRAX, xRAX, x1);
            break;
        case 0xA1:
            INST_NAME("MOV EAX,Od");
            if (rex.is32bits)
                u64 = F32;
            else
                u64 = F64;
            MOV64z(x1, u64);
            LDxw(xRAX, x1, 0);
            break;
        case 0xA2:
            INST_NAME("MOV Ob,AL");
            if (rex.is32bits)
                u64 = F32;
            else
                u64 = F64;
            MOV64z(x1, u64);
            SB(xRAX, x1, 0);
            SMWRITE();
            break;
        case 0xA3:
            INST_NAME("MOV Od,EAX");
            if (rex.is32bits)
                u64 = F32;
            else
                u64 = F64;
            MOV64z(x1, u64);
            SDxw(xRAX, x1, 0);
            SMWRITE();
            break;
        case 0xA4:
            if (rep) {
                INST_NAME("REP MOVSB");
                CBZ_NEXT(xRCX);
                ANDI(x1, xFlags, 1 << F_DF);
                BNEZ_MARK2(x1);
                if (BOX64DRENV(dynarec_safeflags)) {
                    // check for overlapping
                    SUB(x2, xRDI, xRSI);
                    BLT_MARK(x2, 8);
                }
                OR(x1, xRSI, xRDI);
                ANDI(x1, x1, 7);
                BNEZ_MARK(x1);
                ADDI(x6, xZR, 8);
                MARK3;
                BLT_MARK(xRCX, x6);
                LD(x1, xRSI, 0);
                SD(x1, xRDI, 0);
                ADDI(xRSI, xRSI, 8);
                ADDI(xRDI, xRDI, 8);
                SUBI(xRCX, xRCX, 8);
                BNEZ_MARK3(xRCX);
                B_MARKLOCK_nocond;
                MARK; // Part with DF==0
                LBU(x1, xRSI, 0);
                SB(x1, xRDI, 0);
                ADDI(xRSI, xRSI, 1);
                ADDI(xRDI, xRDI, 1);
                SUBI(xRCX, xRCX, 1);
                BNEZ_MARK(xRCX);
                B_MARKLOCK_nocond;
                MARK2; // Part with DF==1
                LBU(x1, xRSI, 0);
                SB(x1, xRDI, 0);
                SUBI(xRSI, xRSI, 1);
                SUBI(xRDI, xRDI, 1);
                SUBI(xRCX, xRCX, 1);
                BNEZ_MARK2(xRCX);
                MARKLOCK;
                // done
            } else {
                INST_NAME("MOVSB");
                GETDIR(x3, x1, 1);
                LBU(x1, xRSI, 0);
                SB(x1, xRDI, 0);
                ADD(xRSI, xRSI, x3);
                ADD(xRDI, xRDI, x3);
            }
            SMWRITE();
            break;
        case 0xA5:
            if (rep) {
                INST_NAME("REP MOVSD");
                CBZ_NEXT(xRCX);
                ANDI(x1, xFlags, 1 << F_DF);
                BNEZ_MARK2(x1);
                MARK; // Part with DF==0
                LDxw(x1, xRSI, 0);
                SDxw(x1, xRDI, 0);
                ADDI(xRSI, xRSI, rex.w ? 8 : 4);
                ADDI(xRDI, xRDI, rex.w ? 8 : 4);
                SUBI(xRCX, xRCX, 1);
                BNEZ_MARK(xRCX);
                B_MARKLOCK_nocond;
                MARK2; // Part with DF==1
                LDxw(x1, xRSI, 0);
                SDxw(x1, xRDI, 0);
                SUBI(xRSI, xRSI, rex.w ? 8 : 4);
                SUBI(xRDI, xRDI, rex.w ? 8 : 4);
                SUBI(xRCX, xRCX, 1);
                BNEZ_MARK2(xRCX);
                MARKLOCK;
                // done
            } else {
                INST_NAME("MOVSD");
                GETDIR(x3, x1, rex.w ? 8 : 4);
                LDxw(x1, xRSI, 0);
                SDxw(x1, xRDI, 0);
                ADD(xRSI, xRSI, x3);
                ADD(xRDI, xRDI, x3);
            }
            SMWRITE();
            break;
        case 0xA6:
            switch (rep) {
                case 1:
                case 2:
                    if (rep == 1) {
                        INST_NAME("REPNZ CMPSB");
                    } else {
                        INST_NAME("REPZ CMPSB");
                    }
                    MAYSETFLAGS();
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_NOFUSION);
                    CBZ_NEXT(xRCX);
                    ANDI(x1, xFlags, 1 << F_DF);
                    BNEZ_MARK2(x1);
                    MARK; // Part with DF==0
                    LBU(x1, xRSI, 0);
                    LBU(x2, xRDI, 0);
                    ADDI(xRSI, xRSI, 1);
                    ADDI(xRDI, xRDI, 1);
                    SUBI(xRCX, xRCX, 1);
                    if (rep == 1) {
                        BEQ_MARK3(x1, x2);
                    } else {
                        BNE_MARK3(x1, x2);
                    }
                    BNEZ_MARK(xRCX);
                    B_MARK3_nocond;
                    MARK2; // Part with DF==1
                    LBU(x1, xRSI, 0);
                    LBU(x2, xRDI, 0);
                    SUBI(xRSI, xRSI, 1);
                    SUBI(xRDI, xRDI, 1);
                    SUBI(xRCX, xRCX, 1);
                    if (rep == 1) {
                        BEQ_MARK3(x1, x2);
                    } else {
                        BNE_MARK3(x1, x2);
                    }
                    BNEZ_MARK2(xRCX);
                    MARK3; // end
                    emit_cmp8(dyn, ninst, x1, x2, x3, x4, x5, x6);
                    break;
                default:
                    INST_NAME("CMPSB");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETDIR(x3, x1, 1);
                    LBU(x1, xRSI, 0);
                    LBU(x2, xRDI, 0);
                    ADD(xRSI, xRSI, x3);
                    ADD(xRDI, xRDI, x3);
                    emit_cmp8(dyn, ninst, x1, x2, x3, x4, x5, x6);
                    break;
            }
            break;
        case 0xA7:
            switch (rep) {
                case 1:
                case 2:
                    if (rep == 1) {
                        INST_NAME("REPNZ CMPSD");
                    } else {
                        INST_NAME("REPZ CMPSD");
                    }
                    MAYSETFLAGS();
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_NOFUSION);
                    CBZ_NEXT(xRCX);
                    ANDI(x1, xFlags, 1 << F_DF);
                    BNEZ_MARK2(x1);
                    MARK; // Part with DF==0
                    LDxw(x1, xRSI, 0);
                    LDxw(x2, xRDI, 0);
                    ADDI(xRSI, xRSI, rex.w ? 8 : 4);
                    ADDI(xRDI, xRDI, rex.w ? 8 : 4);
                    SUBI(xRCX, xRCX, 1);
                    if (rep == 1) {
                        BEQ_MARK3(x1, x2);
                    } else {
                        BNE_MARK3(x1, x2);
                    }
                    BNEZ_MARK(xRCX);
                    B_MARK3_nocond;
                    MARK2; // Part with DF==1
                    LDxw(x1, xRSI, 0);
                    LDxw(x2, xRDI, 0);
                    SUBI(xRSI, xRSI, rex.w ? 8 : 4);
                    SUBI(xRDI, xRDI, rex.w ? 8 : 4);
                    SUBI(xRCX, xRCX, 1);
                    if (rep == 1) {
                        BEQ_MARK3(x1, x2);
                    } else {
                        BNE_MARK3(x1, x2);
                    }
                    BNEZ_MARK2(xRCX);
                    MARK3; // end
                    emit_cmp32(dyn, ninst, rex, x1, x2, x3, x4, x5, x6);
                    break;
                default:
                    INST_NAME("CMPSD");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_NOFUSION);
                    GETDIR(x3, x1, rex.w ? 8 : 4);
                    LDxw(x1, xRSI, 0);
                    LDxw(x2, xRDI, 0);
                    ADD(xRSI, xRSI, x3);
                    ADD(xRDI, xRDI, x3);
                    emit_cmp32(dyn, ninst, rex, x1, x2, x3, x4, x5, x6);
                    break;
            }
            break;
        case 0xA8:
            INST_NAME("TEST AL, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            u8 = F8;
            ADDI(x2, xZR, u8);
            emit_test8(dyn, ninst, x2, xRAX, x3, x4, x5);
            break;
        case 0xA9:
            INST_NAME("TEST EAX, Id");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i64 = F32S;
            MOV64xw(x2, i64);
            emit_test32(dyn, ninst, rex, xRAX, x2, x3, x4, x5);
            break;
        case 0xAA:
            if (rep) {
                INST_NAME("REP STOSB");
                CBZ_NEXT(xRCX);
                ANDI(x1, xFlags, 1 << F_DF);
                BNEZ_MARK2(x1);
                MARK; // Part with DF==0
                SB(xRAX, xRDI, 0);
                ADDI(xRDI, xRDI, 1);
                ADDI(xRCX, xRCX, -1);
                BNEZ_MARK(xRCX);
                B_NEXT_nocond;
                MARK2; // Part with DF==1
                SB(xRAX, xRDI, 0);
                ADDI(xRDI, xRDI, -1);
                ADDI(xRCX, xRCX, -1);
                BNEZ_MARK2(xRCX);
                // done
            } else {
                INST_NAME("STOSB");
                GETDIR(x3, x1, 1);
                SB(xRAX, xRDI, 0);
                ADD(xRDI, xRDI, x3);
            }
            break;
        case 0xAB:
            if (rep) {
                INST_NAME("REP STOSD");
                CBZ_NEXT(xRCX);
                ANDI(x1, xFlags, 1 << F_DF);
                BNEZ_MARK2(x1);
                MARK; // Part with DF==0
                SDxw(xRAX, xRDI, 0);
                ADDI(xRDI, xRDI, rex.w ? 8 : 4);
                SUBI(xRCX, xRCX, 1);
                BNEZ_MARK(xRCX);
                B_NEXT_nocond;
                MARK2; // Part with DF==1
                SDxw(xRAX, xRDI, 0);
                SUBI(xRDI, xRDI, rex.w ? 8 : 4);
                SUBI(xRCX, xRCX, 1);
                BNEZ_MARK2(xRCX);
                // done
            } else {
                INST_NAME("STOSD");
                GETDIR(x3, x1, rex.w ? 8 : 4);
                SDxw(xRAX, xRDI, 0);
                ADD(xRDI, xRDI, x3);
            }
            break;
        case 0xAC:
            if (rep) {
                DEFAULT;
            } else {
                INST_NAME("LODSB");
                GETDIR(x1, x2, 1);
                LBU(x2, xRSI, 0);
                ADD(xRSI, xRSI, x1);
                ANDI(xRAX, xRAX, ~0xff);
                OR(xRAX, xRAX, x2);
            }
            break;
        case 0xAD:
            if (rep) {
                DEFAULT;
            } else {
                INST_NAME("LODSD");
                GETDIR(x1, x2, rex.w ? 8 : 4);
                LDxw(xRAX, xRSI, 0);
                ADD(xRSI, xRSI, x1);
            }
            break;
        case 0xAE:
            switch (rep) {
                case 1:
                case 2:
                    if (rep == 1) {
                        INST_NAME("REPNZ SCASB");
                    } else {
                        INST_NAME("REPZ SCASB");
                    }
                    MAYSETFLAGS();
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_NOFUSION);
                    CBZ_NEXT(xRCX);
                    ANDI(x1, xRAX, 0xff);
                    ANDI(x2, xFlags, 1 << F_DF);
                    BNEZ_MARK2(x2);
                    MARK; // Part with DF==0
                    LBU(x2, xRDI, 0);
                    ADDI(xRDI, xRDI, 1);
                    SUBI(xRCX, xRCX, 1);
                    if (rep == 1) {
                        BEQ_MARK3(x1, x2);
                    } else {
                        BNE_MARK3(x1, x2);
                    }
                    BNE_MARK(xRCX, xZR);
                    B_MARK3_nocond;
                    MARK2; // Part with DF==1
                    LBU(x2, xRDI, 0);
                    SUBI(xRDI, xRDI, 1);
                    SUBI(xRCX, xRCX, 1);
                    if (rep == 1) {
                        BEQ_MARK3(x1, x2);
                    } else {
                        BNE_MARK3(x1, x2);
                    }
                    BNE_MARK2(xRCX, xZR);
                    MARK3; // end
                    emit_cmp8(dyn, ninst, x1, x2, x3, x4, x5, x6);
                    break;
                default:
                    INST_NAME("SCASB");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_NOFUSION);
                    GETDIR(x3, x1, 1);
                    ANDI(x1, xRAX, 0xff);
                    LBU(x2, xRDI, 0);
                    ADD(xRDI, xRDI, x3);
                    emit_cmp8(dyn, ninst, x1, x2, x3, x4, x5, x6);
                    break;
            }
            break;
        case 0xAF:
            switch (rep) {
                case 1:
                case 2:
                    if (rep == 1) {
                        INST_NAME("REPNZ SCASD");
                    } else {
                        INST_NAME("REPZ SCASD");
                    }
                    MAYSETFLAGS();
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_NOFUSION);
                    CBZ_NEXT(xRCX);
                    if (rex.w) {
                        MV(x1, xRAX);
                    } else {
                        ZEXTW2(x1, xRAX);
                    }
                    ANDI(x2, xFlags, 1 << F_DF);
                    BNEZ_MARK2(x2);
                    MARK; // Part with DF==0
                    LDxw(x2, xRDI, 0);
                    ADDI(xRDI, xRDI, rex.w ? 8 : 4);
                    SUBI(xRCX, xRCX, 1);
                    if (rep == 1) {
                        BEQ_MARK3(x1, x2);
                    } else {
                        BNE_MARK3(x1, x2);
                    }
                    BNE_MARK(xRCX, xZR);
                    B_MARK3_nocond;
                    MARK2; // Part with DF==1
                    LDxw(x2, xRDI, 0);
                    SUBI(xRDI, xRDI, rex.w ? 8 : 4);
                    SUBI(xRCX, xRCX, 1);
                    if (rep == 1) {
                        BEQ_MARK3(x1, x2);
                    } else {
                        BNE_MARK3(x1, x2);
                    }
                    BNE_MARK2(xRCX, xZR);
                    MARK3; // end
                    emit_cmp32(dyn, ninst, rex, x1, x2, x3, x4, x5, x6);
                    break;
                default:
                    INST_NAME("SCASD");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_NOFUSION);
                    GETDIR(x3, x1, rex.w ? 8 : 4);
                    LDxw(x2, xRDI, 0);
                    ADD(xRDI, xRDI, x3);
                    emit_cmp32(dyn, ninst, rex, xRAX, x2, x3, x4, x5, x6);
                    break;
            }
            break;
        case 0xB0:
        case 0xB1:
        case 0xB2:
        case 0xB3:
            INST_NAME("MOV xL, Ib");
            u8 = F8;
            SCRATCH_USAGE(0);
            if (rex.rex)
                gb1 = TO_NAT((opcode & 7) + (rex.b << 3));
            else
                gb1 = TO_NAT(opcode & 3);
            ANDI(gb1, gb1, ~0xff);
            ORI(gb1, gb1, u8);
            break;
        case 0xB4:
        case 0xB5:
        case 0xB6:
        case 0xB7:
            INST_NAME("MOV xH, Ib");
            u8 = F8;
            if (rex.rex) {
                gb1 = TO_NAT((opcode & 7) + (rex.b << 3));
                ANDI(gb1, gb1, ~0xff);
                ORI(gb1, gb1, u8);
            } else {
                MOV32w(x1, u8);
                gb1 = TO_NAT(opcode & 3);
                MOV64x(x2, 0xffffffffffff00ffLL);
                AND(gb1, gb1, x2);
                SLLI(x1, x1, 8);
                OR(gb1, gb1, x1);
            }
            break;
        case 0xB8:
        case 0xB9:
        case 0xBA:
        case 0xBB:
        case 0xBC:
        case 0xBD:
        case 0xBE:
        case 0xBF:
            INST_NAME("MOV Reg, Id");
            gd = TO_NAT((opcode & 7) + (rex.b << 3));
            SCRATCH_USAGE(0);
            if (rex.w) {
                u64 = F64;
                MOV64x(gd, u64);
            } else {
                u32 = F32;
                MOV32w(gd, u32);
            }
            break;
        default:
            DEFAULT;
    }

    return addr;
}
