#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "la64_emitter.h"
#include "la64_mapping.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"
#include "custommem.h"

#include "la64_printer.h"
#include "dynarec_la64_private.h"
#include "../dynarec_helper.h"
#include "dynarec_la64_functions.h"


uintptr_t dynarec64_66(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
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
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGW(x2);
            GETEW(x1, 0);
            emit_add16(dyn, ninst, x1, x2, x4, x5, x6);
            EWBACK;
            break;
        case 0x03:
            INST_NAME("ADD Gw, Ew");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGW(x1);
            GETEW(x2, 0);
            emit_add16(dyn, ninst, x1, x2, x3, x4, x6);
            GWBACK;
            break;
        case 0x05:
            INST_NAME("ADD AX, Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i32 = F16;
            BSTRPICK_D(x1, xRAX, 15, 0);
            MOV32w(x2, i32);
            emit_add16(dyn, ninst, x1, x2, x3, x4, x6);
            BSTRINSz(xRAX, x1, 15, 0);
            break;
        case 0x09:
            INST_NAME("OR Ew, Gw");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGW(x2);
            GETEW(x1, 0);
            emit_or16(dyn, ninst, x1, x2, x4, x5);
            EWBACK;
            break;
        case 0x0B:
            INST_NAME("OR Gw, Ew");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGW(x1);
            GETEW(x2, 0);
            emit_or16(dyn, ninst, x1, x2, x4, x5);
            GWBACK;
            break;
        case 0x0D:
            INST_NAME("OR AX, Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i32 = F16;
            BSTRPICK_D(x1, xRAX, 15, 0);
            MOV32w(x2, i32);
            emit_or16(dyn, ninst, x1, x2, x3, x4);
            BSTRINSz(xRAX, x1, 15, 0);
            break;
        case 0x0F:
            switch (rep) {
                case 0: addr = dynarec64_660F(dyn, addr, ip, ninst, rex, ok, need_epilog); break;
                case 1: addr = dynarec64_66F20F(dyn, addr, ip, ninst, rex, ok, need_epilog); break;
                case 2: addr = dynarec64_66F30F(dyn, addr, ip, ninst, rex, ok, need_epilog); break;
                default:
                    DEFAULT;
            }
            break;
        case 0x11:
            INST_NAME("ADC Ew, Gw");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGW(x2);
            GETEW(x1, 0);
            emit_adc16(dyn, ninst, x1, x2, x4, x6, x5);
            EWBACK;
            break;
        case 0x13:
            INST_NAME("ADC Gw, Ew");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGW(x1);
            GETEW(x2, 0);
            emit_adc16(dyn, ninst, x1, x2, x4, x6, x5);
            GWBACK;
            break;
        case 0x15:
            INST_NAME("ADC AX, Iw");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            u64 = F16;
            BSTRPICK_D(x1, xRAX, 15, 0);
            MOV64x(x2, u64);
            emit_adc16(dyn, ninst, x1, x2, x3, x4, x5);
            BSTRINSz(xRAX, x1, 15, 0);
            break;
        case 0x19:
            INST_NAME("SBB Ew, Gw");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGW(x2);
            GETEW(x1, 0);
            emit_sbb16(dyn, ninst, x1, x2, x4, x5, x6);
            EWBACK;
            break;
        case 0x1B:
            INST_NAME("SBB Gw, Ew");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGW(x1);
            GETEW(x2, 0);
            emit_sbb16(dyn, ninst, x1, x2, x6, x4, x5);
            GWBACK;
            break;
        case 0x1D:
            INST_NAME("SBB AX, Iw");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            BSTRPICK_D(x1, xRAX, 15, 0);
            u64 = F16;
            MOV64x(x2, u64);
            emit_sbb16(dyn, ninst, x1, x2, x3, x4, x5);
            BSTRINSz(xRAX, x1, 15, 0);
            break;
        case 0x21:
            INST_NAME("AND Ew, Gw");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGW(x2);
            GETEW(x1, 0);
            emit_and16(dyn, ninst, x1, x2, x4, x5);
            EWBACK;
            break;
        case 0x23:
            INST_NAME("AND Gw, Ew");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGW(x1);
            GETEW(x2, 0);
            emit_and16(dyn, ninst, x1, x2, x3, x4);
            GWBACK;
            break;
        case 0x25:
            INST_NAME("AND AX, Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i32 = F16;
            BSTRPICK_D(x1, xRAX, 15, 0);
            MOV32w(x2, i32);
            emit_and16(dyn, ninst, x1, x2, x3, x4);
            BSTRINSz(xRAX, x1, 15, 0);
            break;
        case 0x29:
            INST_NAME("SUB Ew, Gw");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGW(x1);
            GETEW(x2, 0);
            emit_sub16(dyn, ninst, x2, x1, x4, x5, x6);
            EWBACK;
            break;
        case 0x2B:
            INST_NAME("SUB Gw, Ew");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGW(x1);
            GETEW(x2, 0);
            emit_sub16(dyn, ninst, x1, x2, x3, x4, x5);
            GWBACK;
            break;
        case 0x2D:
            INST_NAME("SUB AX, Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i32 = F16;
            BSTRPICK_D(x1, xRAX, 15, 0);
            MOV32w(x2, i32);
            emit_sub16(dyn, ninst, x1, x2, x3, x4, x5);
            BSTRINSz(xRAX, x1, 15, 0);
            break;
        case 0x31:
            INST_NAME("XOR Ew, Gw");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGW(x2);
            GETEW(x1, 0);
            emit_xor16(dyn, ninst, x1, x2, x4, x5, x6);
            EWBACK;
            break;
        case 0x33:
            INST_NAME("XOR Gw, Ew");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGW(x1);
            GETEW(x2, 0);
            emit_xor16(dyn, ninst, x1, x2, x3, x4, x5);
            GWBACK;
            break;
        case 0x35:
            INST_NAME("XOR AX, Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i32 = F16;
            BSTRPICK_D(x1, xRAX, 15, 0);
            MOV32w(x2, i32);
            emit_xor16(dyn, ninst, x1, x2, x3, x4, x5);
            BSTRINSz(xRAX, x1, 15, 0);
            break;
        case 0x39:
            INST_NAME("CMP Ew, Gw");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGW(x2);
            GETEW(x1, 0);
            emit_cmp16(dyn, ninst, x1, x2, x3, x4, x5, x6);
            break;
        case 0x3B:
            INST_NAME("CMP Gw, Ew");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGW(x1);
            GETEW(x2, 0);
            emit_cmp16(dyn, ninst, x1, x2, x3, x4, x5, x6);
            break;
        case 0x3D:
            INST_NAME("CMP AX, Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i32 = F16;
            BSTRPICK_D(x1, xRAX, 15, 0);
            if (i32) {
                MOV32w(x2, i32);
                emit_cmp16(dyn, ninst, x1, x2, x3, x4, x5, x6);
            } else {
                emit_cmp16_0(dyn, ninst, x1, x3, x4);
            }
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
            SETFLAGS(X_ALL, SF_PENDING, NAT_FLAGS_NOFUSION);
            nextop = F8;
            GETSEW(x1, (opcode == 0x69) ? 2 : 1);
            if (opcode == 0x69)
                i32 = F16S;
            else
                i32 = F8S;
            MOV32w(x2, i32);
            MUL_W(x2, x2, x1);
            UFLAG_RES(x2);
            gd = x2;
            GWBACK;
            UFLAG_DF(x1, d_imul16);
            break;
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
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEW(x1, (opcode == 0x81) ? 2 : 1);
                    if (opcode == 0x81)
                        u64 = F16;
                    else
                        u64 = (uint16_t)(int16_t)F8S;
                    MOV64x(x5, u64);
                    emit_add16(dyn, ninst, ed, x5, x2, x4, x6);
                    EWBACK;
                    break;
                case 1: // OR
                    if (opcode == 0x81) {
                        INST_NAME("OR Ew, Iw");
                    } else {
                        INST_NAME("OR Ew, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEW(x1, (opcode == 0x81) ? 2 : 1);
                    if (opcode == 0x81)
                        u64 = F16;
                    else
                        u64 = (uint16_t)(int16_t)F8S;
                    MOV64x(x5, u64);
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
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEW(x1, (opcode == 0x81) ? 2 : 1);
                    if (opcode == 0x81)
                        u64 = F16;
                    else
                        u64 = (uint16_t)(int16_t)F8S;
                    MOV64x(x5, u64);
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
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEW(x1, (opcode == 0x81) ? 2 : 1);
                    if (opcode == 0x81)
                        u64 = F16;
                    else
                        u64 = (uint16_t)(int16_t)F8S;
                    MOV64x(x5, u64);
                    emit_sbb16(dyn, ninst, x1, x5, x2, x4, x6);
                    EWBACK;
                    break;
                case 4: // AND
                    if (opcode == 0x81) {
                        INST_NAME("AND Ew, Iw");
                    } else {
                        INST_NAME("AND Ew, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEW(x1, (opcode == 0x81) ? 2 : 1);
                    if (opcode == 0x81)
                        u64 = F16;
                    else
                        u64 = (uint16_t)(int16_t)F8S;
                    MOV64x(x5, u64);
                    emit_and16(dyn, ninst, x1, x5, x2, x4);
                    EWBACK;
                    break;
                case 5: // SUB
                    if (opcode == 0x81) {
                        INST_NAME("SUB Ew, Iw");
                    } else {
                        INST_NAME("SUB Ew, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEW(x1, (opcode == 0x81) ? 2 : 1);
                    if (opcode == 0x81)
                        u64 = F16;
                    else
                        u64 = (uint16_t)(int16_t)F8S;
                    MOV32w(x5, u64);
                    emit_sub16(dyn, ninst, x1, x5, x2, x4, x6);
                    EWBACK;
                    break;
                case 6: // XOR
                    if (opcode == 0x81) {
                        INST_NAME("XOR Ew, Iw");
                    } else {
                        INST_NAME("XOR Ew, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEW(x1, (opcode == 0x81) ? 2 : 1);
                    if (opcode == 0x81)
                        u64 = F16;
                    else
                        u64 = (uint16_t)(int16_t)F8S;
                    MOV32w(x5, u64);
                    emit_xor16(dyn, ninst, x1, x5, x2, x4, x6);
                    EWBACK;
                    break;
                case 7: // CMP
                    if (opcode == 0x81) {
                        INST_NAME("CMP Ew, Iw");
                    } else {
                        INST_NAME("CMP Ew, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEW(x1, (opcode == 0x81) ? 2 : 1);
                    if (opcode == 0x81)
                        u64 = F16;
                    else
                        u64 = (uint16_t)(int16_t)F8S;
                    if (u64) {
                        MOV64x(x2, u64);
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
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETEW(x1, 0);
            GETGW(x2);
            emit_test16(dyn, ninst, x1, x2, x3, x4, x5);
            break;
        case 0x89:
            INST_NAME("MOV Ew, Gw");
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                if (ed != gd) {
                    BSTRINSz(ed, gd, 15, 0);
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, 1, 0);
                ST_H(gd, ed, fixedaddress);
                SMWRITELOCK(lock);
            }
            break;
        case 0x8B:
            INST_NAME("MOV Gw, Ew");
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                if (ed != gd) {
                    BSTRINSz(gd, ed, 15, 0);
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, 1, 0);
                SMREADLOCK(lock);
                LD_HU(x1, ed, fixedaddress);
                BSTRINSz(gd, x1, 15, 0);
            }
            break;
        case 0x8E:
            INST_NAME("MOV Seg, Ew");
            nextop = F8;
            u8 = (nextop & 0x38) >> 3;
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                LD_HU(x1, wback, fixedaddress);
                ed = x1;
            }
            ST_H(ed, xEmu, offsetof(x64emu_t, segs[u8]));
            if ((u8 == _FS) || (u8 == _GS)) {
                // refresh offset if needed
                CBZ_NEXT(ed);
                MOV32w(x1, u8);
                CALL(const_getsegmentbase, -1, x1, x2);
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
                INST_NAME("XCHG AX, Reg");
                MV(x2, xRAX);
                BSTRINSz(xRAX, gd, 15, 0);
                BSTRINSz(gd, x2, 15, 0);
            }
            break;
        case 0x98:
            INST_NAME("CBW");
            SLLI_D(x1, xRAX, 56);
            SRAI_D(x1, x1, 56);
            BSTRINSz(xRAX, x1, 15, 0);
            break;
        case 0xA1:
            INST_NAME("MOV EAX,Od");
            if (rex.is32bits)
                u64 = F32;
            else
                u64 = F64;
            MOV64z(x1, u64);
            lock = isLockAddress(u64);
            SMREADLOCK(lock);
            LD_HU(x2, x1, 0);
            BSTRINSz(xRAX, x2, 15, 0);
            break;
        case 0xA3:
            INST_NAME("MOV Od,EAX");
            if (rex.is32bits)
                u64 = F32;
            else
                u64 = F64;
            MOV64z(x1, u64);
            lock = isLockAddress(u64);
            ST_H(xRAX, x1, 0);
            SMWRITELOCK(lock);
            break;
        case 0xA4:
            if (rep) {
                INST_NAME("REP MOVSB");
                CBZ_NEXT(xRCX);
                ANDI(x1, xFlags, 1 << F_DF);
                BNEZ_MARK2(x1);
                // special optim for large RCX value on forward case only
                OR(x1, xRSI, xRDI);
                ANDI(x1, x1, 7);
                BNEZ_MARK(x1);
                ADDI_D(x6, xZR, 8);
                MARK3;
                BLT_MARK(xRCX, x6);
                LD_D(x1, xRSI, 0);
                ST_D(x1, xRDI, 0);
                ADDI_D(xRSI, xRSI, 8);
                ADDI_D(xRDI, xRDI, 8);
                ADDI_D(xRCX, xRCX, -8);
                BNEZ_MARK3(xRCX);
                B_NEXT_nocond;
                MARK; // Part with DF==0
                LD_BU(x1, xRSI, 0);
                ST_B(x1, xRDI, 0);
                ADDI_D(xRSI, xRSI, 1);
                ADDI_D(xRDI, xRDI, 1);
                ADDI_D(xRCX, xRCX, -1);
                BNEZ_MARK(xRCX);
                B_NEXT_nocond;
                MARK2; // Part with DF==1
                LD_BU(x1, xRSI, 0);
                ST_B(x1, xRDI, 0);
                ADDI_D(xRSI, xRSI, -1);
                ADDI_D(xRDI, xRDI, -1);
                ADDI_D(xRCX, xRCX, -1);
                BNEZ_MARK2(xRCX);
                // done
            } else {
                INST_NAME("MOVSB");
                GETDIR(x3, x1, 1);
                LD_BU(x1, xRSI, 0);
                ST_B(x1, xRDI, 0);
                ADD_D(xRSI, xRSI, x3);
                ADD_D(xRDI, xRDI, x3);
            }
            break;
        case 0xA5:
            if (rep) {
                INST_NAME("REP MOVSW");
                CBZ_NEXT(xRCX);
                ANDI(x1, xFlags, 1 << F_DF);
                BNEZ_MARK2(x1);
                MARK; // Part with DF==0
                LD_H(x1, xRSI, 0);
                ST_H(x1, xRDI, 0);
                ADDI_D(xRSI, xRSI, 2);
                ADDI_D(xRDI, xRDI, 2);
                ADDI_D(xRCX, xRCX, -1);
                BNEZ_MARK(xRCX);
                B_NEXT_nocond;
                MARK2; // Part with DF==1
                LD_H(x1, xRSI, 0);
                ST_H(x1, xRDI, 0);
                ADDI_D(xRSI, xRSI, -2);
                ADDI_D(xRDI, xRDI, -2);
                ADDI_D(xRCX, xRCX, -1);
                BNEZ_MARK2(xRCX);
                // done
            } else {
                INST_NAME("MOVSW");
                GETDIR(x3, x1, 2);
                LD_H(x1, xRSI, 0);
                ST_H(x1, xRDI, 0);
                ADD_D(xRSI, xRSI, x3);
                ADD_D(xRDI, xRDI, x3);
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
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_NOFUSION);
                    CBZ_NEXT(xRCX);
                    ANDI(x1, xFlags, 1 << F_DF);
                    BNEZ_MARK2(x1);
                    MARK; // DF==0
                    LD_HU(x1, xRSI, 0);
                    LD_HU(x2, xRDI, 0);
                    ADDI_D(xRSI, xRSI, 2);
                    ADDI_D(xRDI, xRDI, 2);
                    ADDI_D(xRCX, xRCX, -1);
                    if (rep == 1) {
                        BEQ_MARK3(x1, x2); // REPNZ
                    } else {
                        BNE_MARK3(x1, x2); // REPZ
                    }
                    BNEZ_MARK(xRCX);
                    B_MARK3_nocond;
                    MARK2; // DF=1
                    LD_HU(x1, xRSI, 0);
                    LD_HU(x2, xRDI, 0);
                    ADDI_D(xRSI, xRSI, -2);
                    ADDI_D(xRDI, xRDI, -2);
                    ADDI_D(xRCX, xRCX, -1);
                    if (rep == 1) {
                        BEQ_MARK3(x1, x2); // REPNZ
                    } else {
                        BNE_MARK3(x1, x2); // REPZ
                    }
                    BNEZ_MARK2(xRCX);
                    MARK3; // end
                    emit_cmp16(dyn, ninst, x1, x2, x3, x4, x5, x6);
                    break;
                default:
                    INST_NAME("CMPSW");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_NOFUSION);
                    GETDIR(x3, x1, 2);
                    LD_HU(x1, xRSI, 0);
                    LD_HU(x2, xRDI, 0);
                    ADD_D(xRSI, xRSI, x3);
                    ADD_D(xRDI, xRDI, x3);
                    emit_cmp16(dyn, ninst, x1, x2, x3, x4, x5, x6);
                    break;
            }
            break;
        case 0xA9:
            INST_NAME("TEST AX,Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            u16 = F16;
            MOV32w(x2, u16);
            BSTRPICK_D(x1, xRAX, 15, 0);
            emit_test16(dyn, ninst, x1, x2, x3, x4, x5);
            break;
        case 0xAB:
            if (rep) {
                INST_NAME("REP STOSW");
                CBZ_NEXT(xRCX);
                ANDI(x1, xFlags, 1 << F_DF);
                BNEZ_MARK2(x1);
                MARK; // Part with DF==0
                ST_H(xRAX, xRDI, 0);
                ADDI_D(xRDI, xRDI, 2);
                ADDI_D(xRCX, xRCX, -1);
                BNEZ_MARK(xRCX);
                B_NEXT_nocond;
                MARK2; // Part with DF==1
                ST_H(xRAX, xRDI, 0);
                ADDI_D(xRDI, xRDI, -2);
                ADDI_D(xRCX, xRCX, -1);
                BNEZ_MARK2(xRCX);
                // done
            } else {
                INST_NAME("STOSW");
                GETDIR(x3, x1, 2);
                ST_H(xRAX, xRDI, 0);
                ADD_D(xRDI, xRDI, x3);
            }
            break;
        case 0xAD:
            if (rep) {
                DEFAULT;
            } else {
                INST_NAME("LODSW");
                GETDIR(x1, x2, 2);
                LD_HU(x2, xRSI, 0);
                ADD_D(xRSI, xRSI, x1);
                BSTRINSz(xRAX, x2, 15, 0);
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
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    CBZ_NEXT(xRCX);
                    GETDIR(x3, x1, rex.w ? 8 : 2);
                    if (rex.w) {
                        MARK;
                        LD_D(x2, xRDI, 0);
                        ADD_D(xRDI, xRDI, x3);
                        ADDI_D(xRCX, xRCX, -1);
                        if (rep == 1) {
                            BEQ_MARK3(xRAX, x2);
                        } else {
                            BNE_MARK3(xRAX, x2);
                        }
                        BNE_MARK(xRCX, xZR);
                        MARK3;
                        emit_cmp32(dyn, ninst, rex, xRAX, x2, x3, x4, x5, x6);
                    } else {
                        BSTRPICK_D(x1, xRAX, 15, 0);
                        MARK;
                        LD_HU(x2, xRDI, 0);
                        ADD_D(xRDI, xRDI, x3);
                        ADDI_D(xRCX, xRCX, -1);
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
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETDIR(x3, x1, rex.w ? 8 : 2);
                    if (rex.w) {
                        LD_D(x2, xRDI, 0);
                        ADD_D(xRDI, xRDI, x3);
                        emit_cmp32(dyn, ninst, rex, xRAX, x2, x3, x4, x5, x6);
                    } else {
                        BSTRPICK_D(x1, xRAX, 15, 0);
                        LD_HU(x2, xRDI, 0);
                        ADD_D(xRDI, xRDI, x3);
                        emit_cmp16(dyn, ninst, x1, x2, x3, x4, x5, x6);
                    }
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
            INST_NAME("MOV Reg16, Iw");
            u16 = F16;
            MOV32w(x1, u16);
            gd = TO_NAT((opcode & 7) + (rex.b << 3));
            BSTRINSz(gd, x1, 15, 0);
            break;
        case 0xC1:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                    INST_NAME("ROL Ew, Ib");
                    if (geted_ib(dyn, addr, ninst, nextop) & 0x1f) {
                        // removed PENDING on purpose
                        SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION);
                        GETEW(x1, 1);
                        u8 = (F8) & 0x1f;
                        emit_rol16c(dyn, ninst, x1, u8, x4, x5, x6);
                        EWBACK;
                    } else {
                        FAKEED;
                        F8;
                    }
                    break;
                case 1:
                    INST_NAME("ROR Ew, Ib");
                    if (geted_ib(dyn, addr, ninst, nextop) & 0x1f) {
                        // removed PENDING on purpose
                        SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION);
                        GETEW(x1, 1);
                        u8 = (F8) & 0x1f;
                        emit_ror16c(dyn, ninst, x1, u8, x4, x5);
                        EWBACK;
                    } else {
                        FAKEED;
                        F8;
                    }
                    break;
                case 2:
                    INST_NAME("RCL Ew, Ib");
                    if (geted_ib(dyn, addr, ninst, nextop) & 0x1f) {
                        READFLAGS(X_CF);
                        // removed PENDING on purpose
                        SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION);
                        GETEW(x1, 1);
                        u8 = (F8) & 0x1f;
                        emit_rcl16c(dyn, ninst, ed, u8, x4, x5);
                        EWBACK;
                    } else {
                        FAKEED;
                        F8;
                    }
                    break;
                case 3:
                    INST_NAME("RCR Ew, Ib");
                    if (geted_ib(dyn, addr, ninst, nextop) & 0x1f) {
                        READFLAGS(X_CF);
                        // removed PENDING on purpose
                        SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION);
                        GETEW(x1, 1);
                        u8 = (F8) & 0x1f;
                        emit_rcr16c(dyn, ninst, ed, u8, x4, x5);
                        EWBACK;
                    } else {
                        FAKEED;
                        F8;
                    }
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ew, Ib");
                    if (geted_ib(dyn, addr, ninst, nextop) & 0x1f) {
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
                        GETEW(x1, 0);
                        u8 = (F8) & 0x1f;
                        emit_shl16c(dyn, ninst, x1, u8, x5, x4, x6);
                        EWBACK;
                    } else {
                        FAKEED;
                        F8;
                    }
                    break;
                case 5:
                    INST_NAME("SHR Ew, Ib");
                    if (geted_ib(dyn, addr, ninst, nextop) & 0x1f) {
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
                        GETEW(x1, 0);
                        u8 = (F8) & 0x1f;
                        emit_shr16c(dyn, ninst, x1, u8, x5, x4, x6);
                        EWBACK;
                    } else {
                        FAKEED;
                        F8;
                    }
                    break;
                case 7:
                    INST_NAME("SAR Ew, Ib");
                    if (geted_ib(dyn, addr, ninst, nextop) & 0x1f) {
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
                        GETSEW(x1, 0);
                        u8 = (F8) & 0x1f;
                        emit_sar16c(dyn, ninst, x1, u8, x5, x4, x6);
                        EWBACK;
                    } else {
                        FAKEED;
                        F8;
                    }
                    break;
            }
            break;
        case 0xC7:
            INST_NAME("MOV Ew, Iw");
            nextop = F8;
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                u16 = F16;
                MOV32w(x1, u16);
                BSTRINSz(ed, x1, 15, 0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, 1, 2);
                u16 = F16;
                MOV32w(x1, u16);
                ST_H(x1, ed, fixedaddress);
                SMWRITELOCK(lock);
            }
            break;
        case 0xD1:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                    INST_NAME("ROL Ew, 1");
                    // removed PENDING on purpose
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION);
                    GETEW(x1, 0);
                    emit_rol16c(dyn, ninst, x1, 1, x5, x4, x6);
                    EWBACK;
                    break;
                case 1:
                    INST_NAME("ROR Ew, 1");
                    // removed PENDING on purpose
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION);
                    GETEW(x1, 1);
                    emit_ror16c(dyn, ninst, x1, 1, x5, x4);
                    EWBACK;
                    break;
                case 2:
                    INST_NAME("RCL Ew, 1");
                    READFLAGS(X_CF);
                    // removed PENDING on purpose
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION);
                    GETEW(x1, 0);
                    emit_rcl16c(dyn, ninst, x1, 1, x5, x4);
                    EWBACK;
                    break;
                case 3:
                    INST_NAME("RCR Ew, 1");
                    READFLAGS(X_CF);
                    // removed PENDING on purpose
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION);
                    GETEW(x1, 0);
                    emit_rcr16c(dyn, ninst, x1, 1, x5, x4);
                    EWBACK;
                    break;
                case 5:
                    INST_NAME("SHR Ew, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
                    GETEW(x1, 0);
                    emit_shr16c(dyn, ninst, x1, 1, x5, x4, x6);
                    EWBACK;
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ew, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
                    GETEW(x1, 0);
                    emit_shl16c(dyn, ninst, x1, 1, x5, x4, x6);
                    EWBACK;
                    break;
                case 7:
                    INST_NAME("SAR Ew, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
                    GETSEW(x1, 0);
                    emit_sar16c(dyn, ninst, x1, 1, x5, x4, x6);
                    EWBACK;
                    break;
            }
            break;

        case 0xD3:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                    INST_NAME("ROL Ew, CL");
                    ANDI(x2, xRCX, 0x1f);
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    SETFLAGS(X_OF | X_CF, SF_SET_DF, NAT_FLAGS_NOFUSION);
                    if (BOX64DRENV(dynarec_safeflags) > 1) MAYSETFLAGS();
                    GETEW(x1, 1);
                    CALL_(const_rol16, x1, x3, x1, x2);
                    EWBACK;
                    break;
                case 1:
                    INST_NAME("ROR Ew, CL");
                    ANDI(x2, xRCX, 0x1f);
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    SETFLAGS(X_OF | X_CF, SF_SET_DF, NAT_FLAGS_NOFUSION);
                    if (BOX64DRENV(dynarec_safeflags) > 1) MAYSETFLAGS();
                    GETEW(x1, 1);
                    CALL_(const_ror16, x1, x3, x1, x2);
                    EWBACK;
                    break;
                case 2:
                    INST_NAME("RCL Ew, CL");
                    ANDI(x2, xRCX, 0x1f);
                    MESSAGE("LOG_DUMP", "Need optimization\n");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF | X_CF, SF_SET_DF, NAT_FLAGS_NOFUSION);
                    if (BOX64DRENV(dynarec_safeflags) > 1) MAYSETFLAGS();
                    GETEW(x1, 1);
                    CALL_(const_rcl16, x1, x3, x1, x2);
                    EWBACK;
                    break;
                case 3:
                    INST_NAME("RCR Ew, CL");
                    ANDI(x2, xRCX, 0x1f);
                    MESSAGE("LOG_DUMP", "Need optimization\n");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF | X_CF, SF_SET_DF, NAT_FLAGS_NOFUSION);
                    if (BOX64DRENV(dynarec_safeflags) > 1) MAYSETFLAGS();
                    GETEW(x1, 1);
                    CALL_(const_rcr16, x1, x3, x1, x2);
                    EWBACK;
                    break;
                case 5:
                    INST_NAME("SHR Ew, CL");
                    ANDI(x2, xRCX, 0x1f);
                    BEQ_NEXT(x2, xZR);
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
                    if (BOX64DRENV(dynarec_safeflags) > 1) MAYSETFLAGS();
                    GETEW(x1, 0);
                    emit_shr16(dyn, ninst, x1, x2, x5, x4, x6);
                    EWBACK;
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ew, CL");
                    ANDI(x2, xRCX, 0x1f);
                    BEQ_NEXT(x2, xZR);
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
                    if (BOX64DRENV(dynarec_safeflags) > 1) MAYSETFLAGS();
                    GETEW(x1, 0);
                    emit_shl16(dyn, ninst, x1, x2, x5, x4, x6);
                    EWBACK;
                    break;
                case 7:
                    INST_NAME("SAR Ew, CL");
                    ANDI(x2, xRCX, 0x1f);
                    BEQ_NEXT(x2, xZR);
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
                    if (BOX64DRENV(dynarec_safeflags) > 1) MAYSETFLAGS();
                    GETSEW(x1, 0);
                    emit_sar16(dyn, ninst, x1, x2, x5, x4, x6);
                    EWBACK;
                    break;
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
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEW(x1, 2);
                    u16 = F16;
                    MOV32w(x2, u16);
                    emit_test16(dyn, ninst, x1, x2, x3, x4, x5);
                    break;
                case 2:
                    INST_NAME("NOT Ew");
                    GETEW(x1, 0);
                    MOV32w(x5, 0xffff);
                    XOR(ed, ed, x5); // No flags affected
                    EWBACK;
                    break;
                case 3:
                    INST_NAME("NEG Ew");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEW(x1, 0);
                    emit_neg16(dyn, ninst, ed, x2, x4);
                    EWBACK;
                    break;
                case 6:
                    INST_NAME("DIV Ew");
                    SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
                    SET_DFNONE();
                    GETEW(x1, 0);
                    BSTRPICK_D(x2, xRAX, 15, 0);
                    SLLI_D(x7, xRDX, 48);
                    SRLI_D(x7, x7, 32);
                    OR(x2, x2, x7);
                    if (BOX64ENV(dynarec_div0)) {
                        BNE_MARK3(ed, xZR);
                        GETIP_(ip, x6);
                        STORE_XEMU_CALL();
                        CALL(const_native_div0, -1, 0, 0);
                        CLEARIP();
                        LOAD_XEMU_CALL();
                        jump_to_epilog(dyn, 0, xRIP, ninst);
                        MARK3;
                    }
                    DIV_WU(x7, x2, ed);
                    MOD_WU(x4, x2, ed);
                    BSTRINSz(xRAX, x7, 15, 0);
                    BSTRINSz(xRDX, x4, 15, 0);
                    break;
                case 7:
                    INST_NAME("IDIV Ew");
                    NOTEST(x1);
                    SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
                    SET_DFNONE();
                    GETSEW(x1, 0);
                    if (BOX64ENV(dynarec_div0)) {
                        BNE_MARK3(ed, xZR);
                        GETIP_(ip, x7);
                        STORE_XEMU_CALL();
                        CALL(const_native_div0, -1, 0, 0);
                        CLEARIP();
                        LOAD_XEMU_CALL();
                        jump_to_epilog(dyn, 0, xRIP, ninst);
                        MARK3;
                    }
                    BSTRPICK_D(x2, xRAX, 15, 0);
                    SLLI_D(x3, xRDX, 16);
                    OR(x2, x2, x3);
                    DIV_W(x3, x2, ed);
                    MOD_W(x4, x2, ed);
                    BSTRINSz(xRAX, x3, 15, 0);
                    BSTRINSz(xRDX, x4, 15, 0);
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
                    SETFLAGS(X_ALL & ~X_CF, SF_SUBSET_PENDING, NAT_FLAGS_FUSION);
                    GETEW(x1, 0);
                    emit_inc16(dyn, ninst, x1, x2, x4, x5);
                    EWBACK;
                    break;
                case 1:
                    INST_NAME("DEC Ew");
                    SETFLAGS(X_ALL & ~X_CF, SF_SUBSET_PENDING, NAT_FLAGS_FUSION);
                    GETEW(x1, 0);
                    emit_dec16(dyn, ninst, x1, x2, x4, x5, x6);
                    EWBACK;
                    break;
                case 6: // Push Ew
                    INST_NAME("PUSH Ew");
                    GETEW(x1, 0);
                    PUSH1_16(ed);
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
