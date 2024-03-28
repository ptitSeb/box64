#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "dynarec.h"
#include "emu/x64emu_private.h"
#include "emu/x64run_private.h"
#include "x64run.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"
#include "custommem.h"

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_helper.h"
#include "dynarec_rv64_functions.h"


uintptr_t dynarec64_66(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
{
    uint8_t opcode = F8;
    uint8_t nextop, u8;
    int16_t i16;
    uint16_t u16;
    uint64_t u64;
    int32_t i32;
    int64_t j64;
    uint8_t gd, ed;
    uint8_t wback, wb1;
    int64_t fixedaddress;
    int unscaled;
    int lock;
    MAYUSE(u8);
    MAYUSE(u16);
    MAYUSE(u64);
    MAYUSE(j64);
    MAYUSE(lock);

    while ((opcode == 0x2E) || (opcode == 0x36) || (opcode == 0x66)) // ignoring CS:, SS: or multiple 0x66
        opcode = F8;

    while ((opcode == 0xF2) || (opcode == 0xF3)) {
        rep = opcode - 0xF1;
        opcode = F8;
    }

    GETREX();

    if (rex.w && !(opcode == 0x0f || opcode == 0xf0 || opcode == 0x64 || opcode == 0x65)) // rex.w cancels "66", but not for 66 0f type of prefix
        return dynarec64_00(dyn, addr - 1, ip, ninst, rex, rep, ok, need_epilog);         // addr-1, to "put back" opcode

    switch (opcode) {
        case 0x01:
            INST_NAME("ADD Ew, Gw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x2);
            GETEW(x1, 0);
            emit_add16(dyn, ninst, x1, x2, x4, x5, x6);
            EWBACK;
            break;
        case 0x03:
            INST_NAME("ADD Gw, Ew");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x1);
            GETEW(x2, 0);
            emit_add16(dyn, ninst, x1, x2, x3, x4, x6);
            GWBACK;
            break;
        case 0x05:
            INST_NAME("ADD AX, Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i32 = F16;
            ZEXTH(x1, xRAX);
            MOV32w(x2, i32);
            emit_add16(dyn, ninst, x1, x2, x3, x4, x6);
            LUI(x3, 0xffff0);
            AND(xRAX, xRAX, x3);
            OR(xRAX, xRAX, x1);
            break;
        case 0x09:
            INST_NAME("OR Ew, Gw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x2);
            GETEW(x1, 0);
            emit_or16(dyn, ninst, x1, x2, x4, x2);
            EWBACK;
            break;
        case 0x0B:
            INST_NAME("OR Gw, Ew");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x1);
            GETEW(x2, 0);
            emit_or16(dyn, ninst, x1, x2, x4, x5);
            GWBACK;
            break;
        case 0x0D:
            INST_NAME("OR AX, Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i32 = F16;
            ZEXTH(x1, xRAX);
            MOV32w(x2, i32);
            emit_or16(dyn, ninst, x1, x2, x3, x4);
            LUI(x3, 0xffff0);
            AND(xRAX, xRAX, x3);
            OR(xRAX, xRAX, x1);
            break;
        case 0x0F:
            switch (rep) {
                case 0: addr = dynarec64_660F(dyn, addr, ip, ninst, rex, ok, need_epilog); break;
                case 1: addr = dynarec64_66F20F(dyn, addr, ip, ninst, rex, ok, need_epilog); break;
                case 2: addr = dynarec64_66F30F(dyn, addr, ip, ninst, rex, ok, need_epilog); break;
            }
            break;
        case 0x11:
            INST_NAME("ADC Ew, Gw");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x2);
            GETEW(x1, 0);
            emit_adc16(dyn, ninst, x1, x2, x4, x3, x5);
            EWBACK;
            break;
        case 0x13:
            INST_NAME("ADC Gw, Ew");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x1);
            GETEW(x2, 0);
            emit_adc16(dyn, ninst, x1, x2, x4, x3, x5);
            GWBACK;
            break;
        case 0x15:
            INST_NAME("ADC AX, Iw");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i16 = F16;
            SRLI(x6, xMASK, 16);
            AND(x1, xRAX, x6);
            MOV32w(x2, i16);
            emit_adc16(dyn, ninst, x1, x2, x3, x4, x5);
            if (rv64_zbb) {
                ANDN(xRAX, xRAX, x6);
            } else {
                NOT(x6, x6);
                AND(xRAX, xRAX, x6);
            }
            OR(xRAX, xRAX, x1);
            break;
        case 0x19:
            INST_NAME("SBB Ew, Gw");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x2);
            GETEW(x1, 0);
            emit_sbb16(dyn, ninst, x1, x2, x4, x5, x6);
            EWBACK;
            break;
        case 0x1B:
            INST_NAME("SBB Gw, Ew");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x1);
            GETEW(x2, 0);
            emit_sbb16(dyn, ninst, x1, x2, x3, x4, x5);
            GWBACK;
            break;
        case 0x1D:
            INST_NAME("SBB AX, Iw");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            ZEXTH(x1, xRAX);
            i16 = F16;
            MOV64xw(x2, i16);
            emit_sbb16(dyn, ninst, x1, x2, x3, x4, x5);
            SRLI(xRAX, xRAX, 16);
            SLLI(xRAX, xRAX, 16);
            OR(xRAX, xRAX, x1);
            break;
        case 0x21:
            INST_NAME("AND Ew, Gw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x2);
            GETEW(x1, 0);
            emit_and16(dyn, ninst, x1, x2, x4, x5);
            EWBACK;
            break;
        case 0x23:
            INST_NAME("AND Gw, Ew");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x1);
            GETEW(x2, 0);
            emit_and16(dyn, ninst, x1, x2, x4, x5);
            GWBACK;
            break;
        case 0x25:
            INST_NAME("AND AX, Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i32 = F16;
            ZEXTH(x1, xRAX);
            MOV32w(x2, i32);
            emit_and16(dyn, ninst, x1, x2, x3, x4);
            LUI(x3, 0xffff0);
            AND(xRAX, xRAX, x3);
            OR(xRAX, xRAX, x1);
            break;
        case 0x29:
            INST_NAME("SUB Ew, Gw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x1);
            GETEW(x2, 0);
            emit_sub16(dyn, ninst, x2, x1, x4, x5, x6);
            EWBACK;
            break;
        case 0x2B:
            INST_NAME("SUB Gw, Ew");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x1);
            GETEW(x2, 0);
            emit_sub16(dyn, ninst, x1, x2, x3, x4, x5);
            GWBACK;
            break;
        case 0x2D:
            INST_NAME("SUB AX, Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i32 = F16;
            ZEXTH(x1, xRAX);
            MOV32w(x2, i32);
            emit_sub16(dyn, ninst, x1, x2, x3, x4, x5);
            LUI(x2, 0xffff0);
            AND(xRAX, xRAX, x2);
            OR(xRAX, xRAX, x1);
            break;
        case 0x31:
            INST_NAME("XOR Ew, Gw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            // try to determine ed and gd
            ed = 0;
            GETGD;
            if (MODREG) {
                GETED(0);
            }
            if (ed == gd) {
                // optimize XOR Gw, Gw
                CLEAR_FLAGS();
                IFX (X_PEND) {
                    SET_DF(x6, d_xor16);
                } else IFX (X_ALL) {
                    SET_DFNONE();
                }
                SRLI(ed, ed, 16);
                SLLI(ed, ed, 16);
                IFX (X_PEND) {
                    SH(ed, xEmu, offsetof(x64emu_t, res));
                }
                IFX (X_ZF) {
                    ORI(xFlags, xFlags, 1 << F_ZF);
                }
                IFX (X_PF) {
                    ORI(xFlags, xFlags, 1 << F_PF);
                }
            } else {
                GETGW(x2);
                GETEW(x1, 0);
                emit_xor16(dyn, ninst, x1, x2, x4, x5, x6);
                EWBACK;
            }
            break;
        case 0x33:
            INST_NAME("XOR Gw, Ew");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            // try to determine ed and gd
            ed = 0;
            GETGD;
            if (MODREG) {
                GETED(0);
            }
            if (ed == gd) {
                // optimize XOR Gw, Gw
                CLEAR_FLAGS();
                IFX (X_PEND) {
                    SET_DF(x6, d_xor16);
                } else IFX (X_ALL) {
                    SET_DFNONE();
                }
                SRLI(ed, ed, 16);
                SLLI(ed, ed, 16);
                IFX (X_PEND) {
                    SH(ed, xEmu, offsetof(x64emu_t, res));
                }
                IFX (X_ZF) {
                    ORI(xFlags, xFlags, 1 << F_ZF);
                }
                IFX (X_PF) {
                    ORI(xFlags, xFlags, 1 << F_PF);
                }
            } else {
                GETGW(x1);
                GETEW(x2, 0);
                emit_xor16(dyn, ninst, x1, x2, x4, x5, x6);
                GWBACK;
            }
            break;
        case 0x35:
            INST_NAME("XOR AX, Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i32 = F16;
            ZEXTH(x1, xRAX);
            MOV32w(x2, i32);
            emit_xor16(dyn, ninst, x1, x2, x3, x4, x5);
            LUI(x5, 0xffff0);
            AND(xRAX, xRAX, x5);
            OR(xRAX, xRAX, x1);
            break;
        case 0x39:
            INST_NAME("CMP Ew, Gw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x2);
            GETEW(x1, 0);
            emit_cmp16(dyn, ninst, x1, x2, x3, x4, x5, x6);
            break;
        case 0x3B:
            INST_NAME("CMP Gw, Ew");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x1);
            GETEW(x2, 0);
            emit_cmp16(dyn, ninst, x1, x2, x3, x4, x5, x6);
            break;
        case 0x3D:
            INST_NAME("CMP AX, Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i32 = F16;
            ZEXTH(x1, xRAX);
            if (i32) {
                MOV32w(x2, i32);
                emit_cmp16(dyn, ninst, x1, x2, x3, x4, x5, x6);
            } else {
                emit_cmp16_0(dyn, ninst, x1, x3, x4);
            }
            break;
        case 0x40:
        case 0x41:
        case 0x42:
        case 0x43:
        case 0x44:
        case 0x45:
        case 0x46:
        case 0x47:
            INST_NAME("INC Reg16 (32bits)");
            SETFLAGS(X_ALL & ~X_CF, SF_SUBSET_PENDING);
            gd = xRAX + (opcode & 7);
            ZEXTH(x1, gd);
            emit_inc16(dyn, ninst, x1, x2, x3, x4);
            LUI(x3, 0xffff0);
            AND(gd, gd, x3);
            OR(gd, gd, x1);
            ZEROUP(gd);
            break;
        case 0x48:
        case 0x49:
        case 0x4A:
        case 0x4B:
        case 0x4C:
        case 0x4D:
        case 0x4E:
        case 0x4F:
            INST_NAME("DEC Reg16 (32bits)");
            SETFLAGS(X_ALL & ~X_CF, SF_SUBSET_PENDING);
            gd = xRAX + (opcode & 7);
            ZEXTH(x1, gd);
            emit_dec16(dyn, ninst, x1, x2, x3, x4, x5);
            LUI(x3, 0xffff0);
            AND(gd, gd, x3);
            OR(gd, gd, x1);
            ZEROUP(gd);
            break;
        case 0x64:
            addr = dynarec64_6664(dyn, addr, ip, ninst, rex, _FS, ok, need_epilog);
            break;
        case 0x65:
            addr = dynarec64_6664(dyn, addr, ip, ninst, rex, _GS, ok, need_epilog);
            break;
        case 0x66:
            addr = dynarec64_66(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
            break;
        case 0x69:
        case 0x6B:
            if (opcode == 0x69) {
                INST_NAME("IMUL Gw,Ew,Iw");
            } else {
                INST_NAME("IMUL Gw,Ew,Ib");
            }
            SETFLAGS(X_ALL, SF_PENDING);
            nextop = F8;
            UFLAG_DF(x1, d_imul16);
            GETSEW(x1, (opcode == 0x69) ? 2 : 1);
            if (opcode == 0x69)
                i32 = F16S;
            else
                i32 = F8S;
            MOV32w(x2, i32);
            MULW(x2, x2, x1);
            ZEXTH(x2, x2);
            UFLAG_RES(x2);
            gd = x2;
            GWBACK;
            break;
        case 0x70:
        case 0x71:
        case 0x72:
        case 0x73:
        case 0x74:
        case 0x75:
        case 0x76:
        case 0x77:
        case 0x78:
        case 0x79:
        case 0x7a:
        case 0x7b:
        case 0x7c:
        case 0x7d:
        case 0x7e:
        case 0x7f:
            // just use regular conditional jump
            return dynarec64_00(dyn, addr - 1, ip, ninst, rex, rep, ok, need_epilog);

        case 0x81:
        case 0x83:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0: // ADD
                    if (opcode == 0x81) {
                        INST_NAME("ADD Ew, Iw");
                    } else {
                        INST_NAME("ADD Ew, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEW(x1, (opcode == 0x81) ? 2 : 1);
                    if (opcode == 0x81)
                        i16 = F16S;
                    else
                        i16 = F8S;
                    MOV64x(x5, i16);
                    emit_add16(dyn, ninst, ed, x5, x2, x4, x6);
                    EWBACK;
                    break;
                case 1: // OR
                    if (opcode == 0x81) {
                        INST_NAME("OR Ew, Iw");
                    } else {
                        INST_NAME("OR Ew, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEW(x1, (opcode == 0x81) ? 2 : 1);
                    if (opcode == 0x81)
                        i16 = F16S;
                    else
                        i16 = F8S;
                    MOV64x(x5, i16);
                    emit_or16(dyn, ninst, x1, x5, x2, x4);
                    EWBACK;
                    break;
                case 2: // ADC
                    if (opcode == 0x81) {
                        INST_NAME("ADC Ew, Iw");
                    } else {
                        INST_NAME("ADC Ew, Ib");
                    }
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEW(x1, (opcode == 0x81) ? 2 : 1);
                    if (opcode == 0x81)
                        i16 = F16S;
                    else
                        i16 = F8S;
                    MOV64x(x5, i16);
                    emit_adc16(dyn, ninst, x1, x5, x2, x4, x6);
                    EWBACK;
                    break;
                case 3: // SBB
                    if (opcode == 0x81) {
                        INST_NAME("SBB Ew, Iw");
                    } else {
                        INST_NAME("SBB Ew, Ib");
                    }
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEW(x1, (opcode == 0x81) ? 2 : 1);
                    if (opcode == 0x81)
                        i16 = F16S;
                    else
                        i16 = F8S;
                    MOV64x(x5, i16);
                    emit_sbb16(dyn, ninst, x1, x5, x2, x4, x6);
                    EWBACK;
                    break;
                case 4: // AND
                    if (opcode == 0x81) {
                        INST_NAME("AND Ew, Iw");
                    } else {
                        INST_NAME("AND Ew, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEW(x1, (opcode == 0x81) ? 2 : 1);
                    if (opcode == 0x81)
                        i16 = F16S;
                    else
                        i16 = F8S;
                    MOV64x(x5, i16);
                    emit_and16(dyn, ninst, x1, x5, x2, x4);
                    EWBACK;
                    break;
                case 5: // SUB
                    if (opcode == 0x81) {
                        INST_NAME("SUB Ew, Iw");
                    } else {
                        INST_NAME("SUB Ew, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEW(x1, (opcode == 0x81) ? 2 : 1);
                    if (opcode == 0x81)
                        i16 = F16S;
                    else
                        i16 = F8S;
                    MOV32w(x5, i16);
                    emit_sub16(dyn, ninst, x1, x5, x2, x4, x6);
                    EWBACK;
                    break;
                case 6: // XOR
                    if (opcode == 0x81) {
                        INST_NAME("XOR Ew, Iw");
                    } else {
                        INST_NAME("XOR Ew, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEW(x1, (opcode == 0x81) ? 2 : 1);
                    if (opcode == 0x81)
                        i16 = F16S;
                    else
                        i16 = F8S;
                    MOV32w(x5, i16);
                    emit_xor16(dyn, ninst, x1, x5, x2, x4, x6);
                    EWBACK;
                    break;
                case 7: // CMP
                    if (opcode == 0x81) {
                        INST_NAME("CMP Ew, Iw");
                    } else {
                        INST_NAME("CMP Ew, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEW(x1, (opcode == 0x81) ? 2 : 1);
                    if (opcode == 0x81)
                        i16 = F16S;
                    else
                        i16 = F8S;
                    if (i16) {
                        MOV64x(x2, i16);
                        emit_cmp16(dyn, ninst, x1, x2, x3, x4, x5, x6);
                    } else
                        emit_cmp16_0(dyn, ninst, x1, x3, x4);
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0x85:
            INST_NAME("TEST Ew, Gw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEW(x1, 0);
            GETGW(x2);
            emit_test16(dyn, ninst, x1, x2, x3, x4, x5);
            break;
        case 0x87:
            INST_NAME("(LOCK)XCHG Ew, Gw");
            nextop = F8;
            if (MODREG) {
                GETGD;
                GETED(0);
                ADDI(x1, gd, 0);
                LUI(x3, 0xffff0);
                AND(gd, gd, x3);
                ZEXTH(x4, ed);
                OR(gd, gd, x4);
                AND(ed, ed, x3);
                ZEXTH(x4, x1);
                OR(ed, ed, x4);
            } else {
                GETGD;
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                SMDMB();

                ANDI(x3, ed, 1);
                BNEZ_MARK(x3);

                ANDI(x3, ed, 0b10);
                LUI(x5, 0xffff0);
                ZEXTH(x6, gd);
                BNEZ_MARK3(x3);

                MARKLOCK;
                LR_W(x1, ed, 1, 0);
                AND(x3, x1, x5);
                OR(x3, x3, x6);
                SC_W(x3, x3, ed, 0, 1);
                BNEZ_MARKLOCK(x3);
                ZEXTH(x1, x1);
                B_MARK2_nocond;

                MARK3;
                NOT(x5, x5);
                SLLI(x5, x5, 16);
                SLLI(x6, x6, 16);
                // 0xffffffff0000ffff
                NOT(x5, x5);
                ANDI(x4, ed, ~0b11);
                LR_W(x1, x4, 1, 0);
                AND(x3, x1, x5);
                OR(x3, x3, x6);
                SC_W(x3, x3, x4, 0, 1);
                BNEZ(x3, -4 * 4);
                SRLI(x1, x1, 16);
                ZEXTH(x1, x1);
                B_MARK2_nocond;

                MARK;
                LHU(x1, ed, 0);
                SH(gd, ed, 0);

                MARK2;
                SMDMB();
                LUI(x5, 0xffff0);
                AND(gd, gd, x5);
                OR(gd, gd, x1);
            }
            break;
        case 0x89:
            INST_NAME("MOV Ew, Gw");
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = xRAX + (nextop & 7) + (rex.b << 3);
                if (ed != gd) {
                    // we don't use GETGW above, so we need let gd & 0xffff.
                    LUI(x1, 0xffff0);
                    AND(ed, ed, x1);
                    ZEXTH(x2, gd);
                    OR(ed, ed, x2);
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, 1, 0);
                SH(gd, ed, fixedaddress);
                SMWRITELOCK(lock);
            }
            break;
        case 0x8B:
            INST_NAME("MOV Gw, Ew");
            nextop = F8;
            GETGD; // don't need GETGW neither
            if (MODREG) {
                ed = xRAX + (nextop & 7) + (rex.b << 3);
                if (ed != gd) {
                    LUI(x1, 0xffff0);
                    AND(gd, gd, x1);
                    ZEXTH(x2, ed);
                    OR(gd, gd, x2);
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, 1, 0);
                SMREADLOCK(lock);
                LHU(x1, ed, fixedaddress);
                LUI(x4, 0xffff0);
                AND(gd, gd, x4);
                OR(gd, gd, x1);
            }
            break;
        case 0x8C:
            INST_NAME("MOV Ed, Seg");
            nextop = F8;
            if ((nextop & 0xC0) == 0xC0) { // reg <= seg
                LHU(xRAX + (nextop & 7) + (rex.b << 3), xEmu, offsetof(x64emu_t, segs[(nextop & 0x38) >> 3]));
            } else { // mem <= seg
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                LHU(x3, xEmu, offsetof(x64emu_t, segs[(nextop & 0x38) >> 3]));
                SH(x3, ed, fixedaddress);
                SMWRITE2();
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
            gd = xRAX + (opcode & 0x07) + (rex.b << 3);
            if (gd == xRAX) {
                INST_NAME("NOP");
            } else {
                INST_NAME("XCHG AX, Reg");
                LUI(x4, 0xffff0);
                // x2 <- rax
                MV(x2, xRAX);
                // rax[15:0] <- gd[15:0]
                ZEXTH(x3, gd);
                AND(xRAX, xRAX, x4);
                OR(xRAX, xRAX, x3);
                // gd[15:0] <- x2[15:0]
                ZEXTH(x2, x2);
                AND(gd, gd, x4);
                OR(gd, gd, x2);
            }
            break;
        case 0x98:
            INST_NAME("CBW");
            SLLI(x1, xRAX, 56);
            SRAI(x1, x1, 56);
            LUI(x2, 0xffff0);
            AND(xRAX, xRAX, x2);
            NOT(x2, x2);
            AND(x1, x1, x2);
            OR(xRAX, xRAX, x1);
            break;
        case 0x99:
            INST_NAME("CWD");
            SLLI(x1, xRAX, 48);
            SRAI(x1, x1, 48);
            SRLI(x1, x1, 48);
            SRLI(xRDX, xRDX, 16);
            SLLI(xRDX, xRDX, 16);
            OR(xRDX, xRDX, x1);
            break;
        case 0x9C:
            INST_NAME("PUSHF");
            NOTEST(x1);
            READFLAGS(X_ALL);
            FLAGS_ADJUST_TO11(x3, xFlags, x2);
            PUSH1_16(x3);
            break;
        case 0x9D:
            INST_NAME("POPF");
            SETFLAGS(X_ALL, SF_SET);
            POP1_16(x1);
            FLAGS_ADJUST_FROM11(x1, x1, x2);
            LUI(x2, 0xffff0);
            AND(xFlags, xFlags, x2);
            OR(xFlags, xFlags, x1);
            MOV32w(x1, 0x3F7FD7);
            AND(xFlags, xFlags, x1);
            ORI(xFlags, xFlags, 0x2);
            SET_DFNONE();
            if (box64_wine) { // should this be done all the time?
                ANDI(x1, xFlags, 1 << F_TF);
                CBZ_NEXT(x1);
                // go to epilog, TF should trigger at end of next opcode, so using Interpretor only
                jump_to_epilog(dyn, addr, 0, ninst);
            }
            break;
        case 0xA1:
            INST_NAME("MOV EAX,Od");
            if (rex.is32bits)
                u64 = F32;
            else
                u64 = F64;
            MOV64z(x1, u64);
            if (isLockAddress(u64))
                lock = 1;
            else
                lock = 0;
            SMREADLOCK(lock);
            LHU(x2, x1, 0);
            LUI(x3, 0xffff0);
            AND(xRAX, xRAX, x3);
            OR(xRAX, xRAX, x2);
            break;
        case 0xA3:
            INST_NAME("MOV Od,EAX");
            if (rex.is32bits)
                u64 = F32;
            else
                u64 = F64;
            MOV64z(x1, u64);
            if (isLockAddress(u64))
                lock = 1;
            else
                lock = 0;
            SH(xRAX, x1, 0);
            SMWRITELOCK(lock);
            break;

        case 0xA5:
            if (rep) {
                INST_NAME("REP MOVSW");
                CBZ_NEXT(xRCX);
                ANDI(x1, xFlags, 1 << F_DF);
                BNEZ_MARK2(x1);
                MARK; // Part with DF==0
                LH(x1, xRSI, 0);
                SH(x1, xRDI, 0);
                ADDI(xRSI, xRSI, 2);
                ADDI(xRDI, xRDI, 2);
                SUBI(xRCX, xRCX, 1);
                BNEZ_MARK(xRCX);
                B_NEXT_nocond;
                MARK2; // Part with DF==1
                LH(x1, xRSI, 0);
                SUBI(xRSI, xRSI, 2);
                SH(x1, xRDI, 0);
                SUBI(xRDI, xRDI, 2);
                SUBI(xRCX, xRCX, 1);
                BNEZ_MARK2(xRCX);
                // done
            } else {
                INST_NAME("MOVSW");
                GETDIR(x3, x1, 2);
                LH(x1, xRSI, 0);
                SH(x1, xRDI, 0);
                ADD(xRSI, xRSI, x3);
                ADD(xRDI, xRDI, x3);
            }
            break;
        case 0xA7:
            switch (rep) {
                case 1:
                case 2:
                    if (rep == 1) {
                        INST_NAME("REPNZ CMPSW");
                    } else {
                        INST_NAME("REPZ CMPSW");
                    }
                    MAYSETFLAGS();
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    CBZ_NEXT(xRCX);
                    ANDI(x1, xFlags, 1 << F_DF);
                    BNEZ_MARK2(x1);
                    MARK; // Part with DF==0
                    LHU(x1, xRSI, 0);
                    ADDI(xRSI, xRSI, 2);
                    LHU(x2, xRDI, 0);
                    ADDI(xRDI, xRDI, 2);
                    SUBI(xRCX, xRCX, 1);
                    if (rep == 1) {
                        BEQ_MARK3(x1, x2);
                    } else {
                        BNE_MARK3(x1, x2);
                    }
                    BNEZ_MARK(xRCX);
                    B_MARK3_nocond;
                    MARK2; // Part with DF==1
                    LHU(x1, xRSI, 0);
                    SUBI(xRSI, xRSI, 2);
                    LHU(x2, xRDI, 0);
                    SUBI(xRDI, xRDI, 2);
                    SUBI(xRCX, xRCX, 1);
                    if (rep == 1) {
                        BEQ_MARK3(x1, x2);
                    } else {
                        BNE_MARK3(x1, x2);
                    }
                    BNEZ_MARK2(xRCX);
                    MARK3; // end
                    emit_cmp16(dyn, ninst, x1, x2, x3, x4, x5, x6);
                    break;
                default:
                    INST_NAME("CMPSW");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETDIR(x3, x1, 2);
                    LHU(x1, xRSI, 0);
                    LHU(x2, xRDI, 0);
                    ADD(xRSI, xRSI, x3);
                    ADD(xRDI, xRDI, x3);
                    emit_cmp16(dyn, ninst, x1, x2, x3, x4, x5, x6);
                    break;
            }
            break;
        case 0xA9:
            INST_NAME("TEST AX,Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            u16 = F16;
            MOV32w(x2, u16);
            SLLIW(x1, xRAX, 16);
            SRLIW(x1, x1, 16);
            emit_test16(dyn, ninst, x1, x2, x3, x4, x5);
            break;
        case 0xAB:
            if (rep) {
                INST_NAME("REP STOSW");
                CBZ_NEXT(xRCX);
                ANDI(x1, xFlags, 1 << F_DF);
                BNEZ_MARK2(x1);
                MARK; // Part with DF==0
                SH(xRAX, xRDI, 0);
                ADDI(xRDI, xRDI, 2);
                SUBI(xRCX, xRCX, 1);
                BNEZ_MARK(xRCX);
                B_NEXT_nocond;
                MARK2; // Part with DF==1
                SH(xRAX, xRDI, 0);
                SUBI(xRDI, xRDI, 2);
                SUBI(xRCX, xRCX, 1);
                BNEZ_MARK2(xRCX);
                // done
            } else {
                INST_NAME("STOSW");
                GETDIR(x3, x1, 2);
                SH(xRAX, xRDI, 0);
                ADD(xRDI, xRDI, x3);
            }
            break;
        case 0xAD:
            if (rep) {
                DEFAULT;
            } else {
                INST_NAME("LODSW");
                GETDIR(x1, x2, 2);
                LHU(x2, xRSI, 0);
                ADD(xRSI, xRSI, x1);
                LUI(x1, 0xffff0);
                AND(xRAX, xRAX, x1);
                OR(xRAX, xRAX, x2);
            }
            break;
        case 0xAF:
            switch (rep) {
                case 1:
                case 2:
                    if (rep == 1) {
                        INST_NAME("REPNZ SCASW");
                    } else {
                        INST_NAME("REPZ SCASW");
                    }
                    MAYSETFLAGS();
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    CBZ_NEXT(xRCX);
                    GETDIR(x3, x1, rex.w ? 8 : 2);
                    if (rex.w) {
                        MARK;
                        LD(x2, xRDI, 0);
                        ADD(xRDI, xRDI, x3);
                        ADDI(xRCX, xRCX, -1);
                        if (rep == 1) {
                            BEQ_MARK3(xRAX, x2);
                        } else {
                            BNE_MARK3(xRAX, x2);
                        }
                        BNE_MARK(xRCX, xZR);
                        MARK3;
                        emit_cmp32(dyn, ninst, rex, xRAX, x2, x3, x4, x5, x6);
                    } else {
                        ZEXTH(x1, xRAX);
                        MARK;
                        LHU(x2, xRDI, 0);
                        ADD(xRDI, xRDI, x3);
                        ADDI(xRCX, xRCX, -1);
                        if (rep == 1) {
                            BEQ_MARK3(x1, x2);
                        } else {
                            BNE_MARK3(x1, x2);
                        }
                        BNE_MARK(xRCX, xZR);
                        MARK3;
                        emit_cmp16(dyn, ninst, x1, x2, x3, x4, x5, x6);
                    }
                    break;
                default:
                    INST_NAME("SCASW");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETDIR(x3, x1, rex.w ? 8 : 2);
                    if (rex.w) {
                        LD(x2, xRDI, 0);
                        emit_cmp32(dyn, ninst, rex, xRAX, x2, x3, x4, x5, x6);
                    } else {
                        ZEXTH(x1, xRAX);
                        LHU(x2, xRDI, 0);
                        emit_cmp16(dyn, ninst, x1, x2, x3, x4, x5, x6);
                    }
                    ADD(xRDI, xRDI, x3);
                    break;
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
            INST_NAME("MOV Reg, Iw");
            gd = xRAX + (opcode & 7) + (rex.b << 3);
            if (rex.w) {
                u64 = F64;
                MOV64x(gd, u64);
            } else {
                u16 = F16;
                MOV64x(x1, ~0xffff);
                AND(gd, gd, x1);
                if (u16 < 2048) {
                    ORI(gd, gd, u16);
                } else {
                    MOV32w(x1, u16);
                    OR(gd, gd, x1);
                }
            }
            break;

        case 0xC1:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                    INST_NAME("ROL Ew, Ib");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    SETFLAGS(X_OF | X_CF, SF_SET);
                    GETEW(x1, 1);
                    u8 = F8;
                    MOV32w(x2, u8);
                    CALL_(rol16, x1, x3);
                    EWBACK;
                    break;
                case 1:
                    INST_NAME("ROR Ew, Ib");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    SETFLAGS(X_OF | X_CF, SF_SET);
                    GETEW(x1, 1);
                    u8 = F8;
                    MOV32w(x2, u8);
                    CALL_(ror16, x1, x3);
                    EWBACK;
                    break;
                case 2:
                    INST_NAME("RCL Ew, Ib");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF | X_CF, SF_SET);
                    GETEW(x1, 1);
                    u8 = F8;
                    MOV32w(x2, u8);
                    CALL_(rcl16, x1, x3);
                    EWBACK;
                    break;
                case 3:
                    INST_NAME("RCR Ew, Ib");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF | X_CF, SF_SET);
                    GETEW(x1, 1);
                    u8 = F8;
                    MOV32w(x2, u8);
                    CALL_(rcr16, x1, x3);
                    EWBACK;
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ew, Ib");
                    UFLAG_IF { MESSAGE(LOG_DUMP, "Need Optimization for flags\n"); }
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETEW(x1, 1);
                    u8 = F8;
                    UFLAG_IF { MOV32w(x2, (u8 & 15)); }
                    UFLAG_OP12(ed, x2)
                    if (MODREG) {
                        SLLI(ed, ed, 48 + (u8 & 15));
                        SRLI(ed, ed, 48);
                    } else {
                        SLLI(ed, ed, u8 & 15);
                    }
                    EWBACK;
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, d_shl16);
                    break;
                case 5:
                    INST_NAME("SHR Ew, Ib");
                    UFLAG_IF { MESSAGE(LOG_DUMP, "Need Optimization for flags\n"); }
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETEW(x1, 1);
                    u8 = F8;
                    UFLAG_IF { MOV32w(x2, (u8 & 15)); }
                    UFLAG_OP12(ed, x2)
                    SRLI(ed, ed, u8 & 15);
                    EWBACK;
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, d_shr16);
                    break;
                case 7:
                    INST_NAME("SAR Ew, Ib");
                    SETFLAGS(X_ALL, SF_PENDING);
                    UFLAG_IF { MESSAGE(LOG_DUMP, "Need Optimization for flags\n"); }
                    GETSEW(x1, 1);
                    u8 = F8;
                    UFLAG_IF { MOV32w(x2, (u8 & 15)); }
                    UFLAG_OP12(ed, x2)
                    SRAI(ed, ed, u8 & 15);
                    if (MODREG) {
                        ZEXTH(ed, ed);
                    }
                    EWBACK;
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, d_sar16);
                    break;
            }
            break;

        case 0xC7:
            INST_NAME("MOV Ew, Iw");
            nextop = F8;
            if (MODREG) {
                ed = xRAX + (nextop & 7) + (rex.b << 3);
                ADDI(x1, xZR, -1);
                SRLI(x1, x1, 48);
                AND(ed, ed, x1);
                u16 = F16;
                MOV32w(x1, u16);
                ORI(ed, ed, x1);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, 1, 2);
                u16 = F16;
                MOV32w(x1, u16);
                SH(x1, ed, fixedaddress);
                SMWRITELOCK(lock);
            }
            break;
        case 0xD1:
        case 0xD3:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                    if (opcode == 0xD1) {
                        INST_NAME("ROL Ew, 1");
                        MOV32w(x2, 1);
                    } else {
                        INST_NAME("ROL Ew, CL");
                        ANDI(x2, xRCX, 15);
                    }
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    SETFLAGS(X_OF | X_CF, SF_SET);
                    GETEW(x1, 1);
                    CALL_(rol16, x1, x3);
                    EWBACK;
                    break;
                case 1:
                    if (opcode == 0xD1) {
                        INST_NAME("ROR Ew, 1");
                        MOV32w(x2, 1);
                    } else {
                        INST_NAME("ROR Ew, CL");
                        ANDI(x2, xRCX, 15);
                    }
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    SETFLAGS(X_OF | X_CF, SF_SET);
                    GETEW(x1, 1);
                    CALL_(ror16, x1, x3);
                    EWBACK;
                    break;
                case 2:
                    if (opcode == 0xD1) {
                        INST_NAME("RCL Ew, 1");
                        MOV32w(x2, 1);
                    } else {
                        INST_NAME("RCL Ew, CL");
                        ANDI(x2, xRCX, 15);
                    }
                    MESSAGE("LOG_DUMP", "Need optimization\n");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF | X_CF, SF_SET);
                    GETEW(x1, 1);
                    CALL_(rcl16, x1, x3);
                    EWBACK;
                case 3:
                    if (opcode == 0xD1) {
                        INST_NAME("RCR Ew, 1");
                        MOV32w(x2, 1);
                    } else {
                        INST_NAME("RCR Ew, CL");
                        ANDI(x2, xRCX, 15);
                    }
                    MESSAGE("LOG_DUMP", "Need optimization\n");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF | X_CF, SF_SET);
                    GETEW(x1, 1);
                    CALL_(rcr16, x1, x3);
                    EWBACK;
                case 5:
                    if (opcode == 0xD1) {
                        INST_NAME("SHR Ew, 1");
                        MOV32w(x4, 1);
                    } else {
                        INST_NAME("SHR Ew, CL");
                        ANDI(x4, xRCX, 15);
                    }
                    UFLAG_IF { MESSAGE(LOG_DUMP, "Need Optimization for flags\n"); }
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETEW(x1, 0);
                    UFLAG_OP12(ed, x4)
                    SRL(ed, ed, x4);
                    EWBACK;
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, d_shr16);
                    break;
                case 4:
                case 6:
                    if (opcode == 0xD1) {
                        INST_NAME("SHL Ew, 1");
                        MOV32w(x4, 1);
                    } else {
                        INST_NAME("SHL Ew, CL");
                        ANDI(x4, xRCX, 15);
                    }
                    UFLAG_IF { MESSAGE(LOG_DUMP, "Need Optimization for flags\n"); }
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETEW(x1, 0);
                    UFLAG_OP12(ed, x4)
                    SLL(ed, ed, x4);
                    ZEXTH(ed, ed);
                    EWBACK;
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, d_shl16);
                    break;
                case 7:
                    if (opcode == 0xD1) {
                        INST_NAME("SAR Ew, 1");
                        MOV32w(x4, 1);
                    } else {
                        INST_NAME("SAR Ew, CL");
                        ANDI(x4, xRCX, 15);
                    }
                    UFLAG_IF { MESSAGE(LOG_DUMP, "Need Optimization for flags\n"); }
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETSEW(x1, 0);
                    UFLAG_OP12(ed, x4);
                    SRA(ed, ed, x4);
                    ZEXTH(ed, ed);
                    EWBACK;
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, d_sar16);
                    break;
                default:
                    DEFAULT;
            }
            break;

        case 0xF0:
            return dynarec64_66F0(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);

        case 0xF7:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                case 1:
                    INST_NAME("TEST Ew, Iw");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEW(x1, 2);
                    u16 = F16;
                    MOV32w(x2, u16);
                    emit_test16(dyn, ninst, x1, x2, x3, x4, x5);
                    break;
                case 2:
                    INST_NAME("NOT Ew");
                    GETEW(x1, 0);
                    NOT(ed, ed); // No flags affected
                    EWBACK;
                    break;
                case 3:
                    INST_NAME("NEG Ew");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEW(x1, 0);
                    emit_neg16(dyn, ninst, ed, x2, x4);
                    EWBACK;
                    break;
                case 6:
                    INST_NAME("DIV Ew");
                    SETFLAGS(X_ALL, SF_SET);
                    GETEW(x1, 0);
                    ZEXTH(x2, xRAX);
                    SLLI(x3, xRDX, 48);
                    SRLI(x3, x3, 32);
                    OR(x2, x2, x3);
                    DIVUW(x3, x2, ed);
                    REMUW(x4, x2, ed);
                    MOV64x(x5, ~0xffff);
                    AND(xRAX, xRAX, x5);
                    AND(xRDX, xRDX, x5);
                    NOT(x5, x5);
                    AND(x3, x3, x5);
                    AND(x4, x4, x5);
                    OR(xRAX, xRAX, x3);
                    OR(xRDX, xRDX, x4);
                    break;
                case 7:
                    INST_NAME("IDIV Ew");
                    NOTEST(x1);
                    SETFLAGS(X_ALL, SF_SET);
                    GETSEW(x1, 0);
                    ZEXTH(x2, xRAX);
                    SLLI(x3, xRDX, 48);
                    SRLI(x3, x3, 32);
                    OR(x2, x2, x3);
                    DIVW(x3, x2, ed);
                    REMW(x4, x2, ed);
                    MOV64x(x5, ~0xffff);
                    AND(xRAX, xRAX, x5);
                    AND(xRDX, xRDX, x5);
                    NOT(x5, x5);
                    AND(x3, x3, x5);
                    AND(x4, x4, x5);
                    OR(xRAX, xRAX, x3);
                    OR(xRDX, xRDX, x4);
                    break;
                default:
                    DEFAULT;
            }
            break;

        case 0xFF:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                    INST_NAME("INC Ew");
                    SETFLAGS(X_ALL & ~X_CF, SF_SUBSET_PENDING);
                    GETEW(x1, 0);
                    emit_inc16(dyn, ninst, x1, x2, x4, x5);
                    EWBACK;
                    break;
                case 1:
                    INST_NAME("DEC Ew");
                    SETFLAGS(X_ALL & ~X_CF, SF_SUBSET_PENDING);
                    GETEW(x1, 0);
                    emit_dec16(dyn, ninst, x1, x2, x4, x5, x6);
                    EWBACK;
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
