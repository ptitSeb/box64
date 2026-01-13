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


uintptr_t dynarec64_66(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
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

    if (rex.w && !(opcode == 0x0f))                                          // rex.w cancels "66", but not for 66 0f type of prefix
        return dynarec64_00(dyn, addr - 1, ip, ninst, rex, ok, need_epilog); // addr-1, to "put back" opcode

    switch (opcode) {
        case 0x01:
            INST_NAME("ADD Ew, Gw");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGWEW(x1, x2, 0);
            emit_add16(dyn, ninst, ed, gd, x4, x5, x6);
            EWBACK;
            break;
        case 0x03:
            INST_NAME("ADD Gw, Ew");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGWEW(x1, x2, 0);
            emit_add16(dyn, ninst, gd, ed, x3, x4, x6);
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
        case 0x06:
            INST_NAME("PUSH ES");
            LD_HU(x1, xEmu, offsetof(x64emu_t, segs[_ES]));
            PUSH1_16(x1);
            break;
        case 0x07:
            INST_NAME("POP ES");
            POP1_16(x1);
            ST_H(x1, xEmu, offsetof(x64emu_t, segs[_ES]));
            break;
        case 0x09:
            INST_NAME("OR Ew, Gw");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGWEW(x1, x2, 0);
            emit_or16(dyn, ninst, ed, gd, x4, x5);
            EWBACK;
            break;
        case 0x0B:
            INST_NAME("OR Gw, Ew");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGWEW(x1, x2, 0);
            emit_or16(dyn, ninst, gd, ed, x4, x5);
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
            switch (rex.rep) {
                case 1: addr = dynarec64_66F20F(dyn, addr, ip, ninst, rex, ok, need_epilog); break;
                case 2: addr = dynarec64_66F30F(dyn, addr, ip, ninst, rex, ok, need_epilog); break;
                default: addr = dynarec64_660F(dyn, addr, ip, ninst, rex, ok, need_epilog); break;
            }
            break;
        case 0x11:
            INST_NAME("ADC Ew, Gw");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGWEW(x1, x2, 0);
            emit_adc16(dyn, ninst, ed, gd, x4, x6, x5);
            EWBACK;
            break;
        case 0x13:
            INST_NAME("ADC Gw, Ew");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGWEW(x1, x2, 0);
            emit_adc16(dyn, ninst, gd, ed, x4, x6, x5);
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
            GETGWEW(x1, x2, 0);
            emit_sbb16(dyn, ninst, ed, gd, x4, x5, x6);
            EWBACK;
            break;
        case 0x1B:
            INST_NAME("SBB Gw, Ew");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGWEW(x1, x2, 0);
            emit_sbb16(dyn, ninst, gd, ed, x6, x4, x5);
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
        case 0x1E:
            INST_NAME("PUSH DS");
            LD_HU(x1, xEmu, offsetof(x64emu_t, segs[_DS]));
            PUSH1_16(x1);
            break;
        case 0x1F:
            INST_NAME("POP DS");
            POP1_16(x1);
            ST_H(x1, xEmu, offsetof(x64emu_t, segs[_DS]));
            break;
        case 0x21:
            INST_NAME("AND Ew, Gw");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGWEW(x1, x2, 0);
            emit_and16(dyn, ninst, ed, gd, x4, x5);
            EWBACK;
            break;
        case 0x23:
            INST_NAME("AND Gw, Ew");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGWEW(x1, x2, 0);
            emit_and16(dyn, ninst, gd, ed, x3, x4);
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
            GETGWEW(x1, x2, 0);
            emit_sub16(dyn, ninst, ed, gd, x4, x5, x6);
            EWBACK;
            break;
        case 0x2B:
            INST_NAME("SUB Gw, Ew");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGWEW(x1, x2, 0);
            emit_sub16(dyn, ninst, gd, ed, x3, x4, x5);
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
            GETGWEW(x1, x2, 0);
            emit_xor16(dyn, ninst, ed, gd, x4, x5, x6);
            EWBACK;
            break;
        case 0x33:
            INST_NAME("XOR Gw, Ew");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGWEW(x1, x2, 0);
            emit_xor16(dyn, ninst, gd, ed, x3, x4, x5);
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
            GETGWEW(x1, x2, 0);
            emit_cmp16(dyn, ninst, ed, gd, x3, x4, x5, x6);
            break;
        case 0x3B:
            INST_NAME("CMP Gw, Ew");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGWEW(x1, x2, 0);
            emit_cmp16(dyn, ninst, gd, ed, x3, x4, x5, x6);
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
        case 0x40:
        case 0x41:
        case 0x42:
        case 0x43:
        case 0x44:
        case 0x45:
        case 0x46:
        case 0x47:
            INST_NAME("INC Reg16 (32bits)");
            SETFLAGS(X_ALL & ~X_CF, SF_SUBSET, NAT_FLAGS_FUSION);
            gd = TO_NAT(opcode & 7);
            BSTRPICK_D(x1, gd, 15, 0);
            emit_inc16(dyn, ninst, x1, x2, x3, x4);
            BSTRINS_D(gd, x1, 15, 0);
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
            SETFLAGS(X_ALL & ~X_CF, SF_SUBSET, NAT_FLAGS_FUSION);
            gd = TO_NAT(opcode & 7);
            BSTRPICK_D(x1, gd, 15, 0);
            emit_dec16(dyn, ninst, x1, x2, x3, x4, x5);
            BSTRINS_D(gd, x1, 15, 0);
            break;
        case 0x50:
        case 0x51:
        case 0x52:
        case 0x53:
        case 0x54:
        case 0x55:
        case 0x56:
        case 0x57:
            INST_NAME("PUSH reg");
            SCRATCH_USAGE(0);
            gd = TO_NAT((opcode & 0x07) + (rex.b << 3));
            PUSH1_16(gd);
            break;
        case 0x58:
        case 0x59:
        case 0x5A:
        case 0x5B:
        case 0x5C:
        case 0x5D:
        case 0x5E:
        case 0x5F:
            INST_NAME("POP reg");
            gd = TO_NAT((opcode & 0x07) + (rex.b << 3));
            POP1_16(x1);
            BSTRINS_D(gd, x1, 15, 0);
            break;
        case 0x60:
            if (rex.is32bits) {
                INST_NAME("PUSHA 16bits (32bits)");
                MV(x1, xRSP);
                PUSH1_16(xRAX);
                PUSH1_16(xRCX);
                PUSH1_16(xRDX);
                PUSH1_16(xRBX);
                PUSH1_16(x1);
                PUSH1_16(xRBP);
                PUSH1_16(xRSI);
                PUSH1_16(xRDI);
            } else
                return dynarec64_00(dyn, addr - 1, ip, ninst, rex, ok, need_epilog);
            break;
        case 0x61:
            if (rex.is32bits) {
                INST_NAME("POPA 16bits (32bits)");
                POP1_16(x1);
                BSTRINS_D(xRDI, x1, 15, 0);
                POP1_16(x1);
                BSTRINS_D(xRSI, x1, 15, 0);
                POP1_16(x1);
                BSTRINS_D(xRBP, x1, 15, 0);
                POP1_16(x1); // RSP ignored
                POP1_16(x1);
                BSTRINS_D(xRBX, x1, 15, 0);
                POP1_16(x1);
                BSTRINS_D(xRDX, x1, 15, 0);
                POP1_16(x1);
                BSTRINS_D(xRCX, x1, 15, 0);
                POP1_16(x1);
                BSTRINS_D(xRAX, x1, 15, 0);
            } else
                return dynarec64_00(dyn, addr - 1, ip, ninst, rex, ok, need_epilog);
            break;
        case 0x68:
            INST_NAME("PUSH Iw");
            u16 = F16;
            MOV32w(x2, u16);
            PUSH1_16(x2);
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
        case 0x6A:
            INST_NAME("PUSH Ib");
            i16 = F8S;
            MOV32w(x2, (uint16_t)i16);
            PUSH1_16(x2);
            break;
        case 0x6C:
        case 0x6E:
            return dynarec64_00(dyn, addr - 1, ip, ninst, rex, ok, need_epilog);
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
            return dynarec64_00(dyn, addr - 1, ip, ninst, rex, ok, need_epilog);
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
            GETGWEW(x1, x2, 0);
            emit_test16(dyn, ninst, ed, gd, x3, x4, x5);
            break;
        case 0x87:
            INST_NAME("(LOCK) XCHG Ew, Gw");
            nextop = F8;
            if (MODREG) {
                GETGD;
                GETED(0);
                MV(x1, gd);
                BSTRINS_D(gd, ed, 15, 0);
                BSTRINS_D(ed, x1, 15, 0);
            } else {
                GETGD;
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                ANDI(x3, wback, 1);
                BEQ_MARK(x3, xZR);
                SMDMB();
                LD_HU(x1, wback, 0);
                ST_H(gd, wback, 0);
                SMDMB();
                BSTRINS_D(gd, x1, 15, 0);
                B_NEXT_nocond;
                MARK;
                if (cpuext.lam_bh) {
                    AMSWAP_DB_H(x1, gd, wback);
                } else if (cpuext.lamcas) {
                    LD_H(x1, wback, 0);
                    MARKLOCK;
                    MV(x6, x1);
                    AMCAS_DB_H(x1, gd, wback);
                    BNE_MARKLOCK(x1, x6);
                } else {
                    MV(x6, wback);
                    BSTRINS_D(x6, xZR, 1, 0);
                    ANDI(x3, wback, 0b10);
                    BEQZ(x3, 4 + 4 * 6);
                    // hi16
                    LL_W(x5, x6, 0);
                    BSTRPICK_D(x1, x5, 31, 16);
                    BSTRINS_D(x5, gd, 31, 16);
                    SC_W(x5, x6, 0);
                    BEQZ(x5, -4 * 4);
                    B(4 + 4 * 5);
                    // lo16
                    LL_W(x5, x6, 0);
                    BSTRPICK_D(x1, x5, 15, 0);
                    BSTRINS_D(x5, gd, 15, 0);
                    SC_W(x5, x6, 0);
                    BEQZ(x5, -4 * 4);
                }
                BSTRINS_D(gd, x1, 15, 0);
            }
            break;
        case 0x89:
            INST_NAME("MOV Ew, Gw");
            nextop = F8;
            GETGD;
            SCRATCH_USAGE(0);
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                if (ed != gd) BSTRINSz(ed, gd, 15, 0);
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
                SCRATCH_USAGE(0);
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                if (ed != gd) BSTRINSz(gd, ed, 15, 0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, 1, 0);
                SMREADLOCK(lock);
                LD_HU(x1, ed, fixedaddress);
                BSTRINSz(gd, x1, 15, 0);
            }
            break;
        case 0x8C:
            INST_NAME("MOV Ew, Seg");
            nextop = F8;
            LD_HU(x3, xEmu, offsetof(x64emu_t, segs[(nextop & 0x38) >> 3]));
            if (MODREG) {
                BSTRINSz(TO_NAT((nextop & 7) + (rex.b << 3)), x3, 15, 0);
            } else { // mem <= seg
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                ST_H(x3, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0x8D:
            INST_NAME("LEA Gd, Ed");
            nextop = F8;
            GETGD;
            if (MODREG) {
                INST_NAME("Invalid 8D");
                UDF();
                *need_epilog = 1;
                *ok = 0;
            } else { // mem <= reg
                rex.seg = 0;
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 0, 0);
                BSTRINS_D(gd, ed, 15, 0);
            }
            break;
        case 0x8E:
            nextop = F8;
            u8 = (nextop & 0x38) >> 3;
            if ((u8 > 5) || (u8 == 1)) {
                INST_NAME("Invalid MOV Seg, Ew");
                UDF();
                *need_epilog = 1;
                *ok = 0;
            } else {
                INST_NAME("MOV Seg, Ew");
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
            }
            break;
        case 0x8F:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                    INST_NAME("POP Ew");
                    POP1_16(x1);
                    if (MODREG) {
                        wback = TO_NAT((nextop & 7) + (rex.b << 3));
                        BSTRINS_D(wback, x1, 15, 0);
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                        ST_H(x1, wback, fixedaddress);
                    }
                    break;
                default:
                    DEFAULT;
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
        case 0x99:
            INST_NAME("CWD");
            SLLI_D(x1, xRAX, 48);
            SRAI_D(x1, x1, 48);
            SRLI_D(x1, x1, 48);
            BSTRINS_D(xRDX, x1, 15, 0);
            break;
        case 0x9C:
            INST_NAME("PUSHF");
            READFLAGS(X_ALL);
            RESTORE_EFLAGS(x1);
            PUSH1_16(xFlags);
            SMWRITE();
            break;
        case 0x9D:
            INST_NAME("POPF");
            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            SMREAD();
            POP1_16(x1);
            MOV32w(x2, 0x7FD7);
            AND(x1, x1, x2);
            ORI(x1, x1, 0x202);
            // no need to restore eflags here
            BSTRINS_D(xFlags, x1, 15, 0);
            SPILL_EFLAGS();
            SET_DFNONE();
            if (box64_wine) { // should this be done all the time?
                ANDI(x1, xFlags, 1 << F_TF);
                CBZ_NEXT(x1);
                // go to epilog, TF should trigger at end of next opcode, so using Interpreter only
                LD_W(x4, xEmu, offsetof(x64emu_t, flags));
                ORI(x4, x4, 1<<FLAGS_NO_TF);
                ST_W(x4, xEmu, offsetof(x64emu_t, flags));
                jump_to_epilog(dyn, addr, 0, ninst);
            }
            break;
        case 0xA1:
            INST_NAME("MOV EAX, Od");
            if (rex.is32bits && rex.is67)
                u64 = F16S;
            else if (rex.is32bits || rex.is67)
                u64 = F32S;
            else
                u64 = F64;
            MOV64y(x1, u64);
            if (rex.seg) {
                grab_segdata(dyn, addr, ninst, x3, rex.seg, 0);
                ADDxREGy(x1, x3, x1, x1);
            }
            lock = (rex.seg) ? 0 : isLockAddress(u64);
            SMREADLOCK(lock);
            LD_HU(x2, x1, 0);
            BSTRINSz(xRAX, x2, 15, 0);
            break;
        case 0xA3:
            INST_NAME("MOV Od, EAX");
            if (rex.is32bits && rex.is67)
                u64 = F16S;
            else if (rex.is32bits || rex.is67)
                u64 = F32S;
            else
                u64 = F64;
            MOV64y(x1, u64);
            if (rex.seg) {
                grab_segdata(dyn, addr, ninst, x3, rex.seg, 0);
                ADDxREGy(x1, x3, x1, x1);
            }
            lock = (rex.seg) ? 0 : isLockAddress(u64);
            ST_H(xRAX, x1, 0);
            SMWRITELOCK(lock);
            break;
        case 0xA4:
            return dynarec64_00(dyn, addr - 1, ip, ninst, rex, ok, need_epilog);
            break;
        case 0xA5:
            if (rex.rep) {
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
            SMWRITE();
            break;
        case 0xA7:
            switch (rex.rep) {
                case 1:
                case 2:
                    if (rex.rep == 1) {
                        INST_NAME("REPNZ CMPSW");
                    } else {
                        INST_NAME("REPZ CMPSW");
                    }
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_ALL);
                        SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
                    } else
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
                    if (rex.rep == 1) {
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
                    if (rex.rep == 1) {
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
            if (rex.rep) {
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
            SMWRITE();
            break;
        case 0xAD:
            if (rex.rep) {
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
            switch (rex.rep) {
                case 1:
                case 2:
                    if (rex.rep == 1) {
                        INST_NAME("REPNZ SCASW");
                    } else {
                        INST_NAME("REPZ SCASW");
                    }
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_ALL);
                        SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
                    } else
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_NOFUSION);
                    CBZ_NEXT(xRCX);
                    GETDIR(x3, x1, rex.w ? 8 : 2);
                    if (rex.w) {
                        MARK;
                        LD_D(x2, xRDI, 0);
                        ADD_D(xRDI, xRDI, x3);
                        ADDI_D(xRCX, xRCX, -1);
                        if (rex.rep == 1) {
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
                        if (rex.rep == 1) {
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
                        SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
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
                        SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
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
                        SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
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
                        SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
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
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                    GETEW(x1, 0);
                    emit_rol16c(dyn, ninst, x1, 1, x5, x4, x6);
                    EWBACK;
                    break;
                case 1:
                    INST_NAME("ROR Ew, 1");
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                    GETEW(x1, 1);
                    emit_ror16c(dyn, ninst, x1, 1, x5, x4);
                    EWBACK;
                    break;
                case 2:
                    INST_NAME("RCL Ew, 1");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                    GETEW(x1, 0);
                    emit_rcl16c(dyn, ninst, x1, 1, x5, x4);
                    EWBACK;
                    break;
                case 3:
                    INST_NAME("RCR Ew, 1");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
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
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_OF | X_CF);
                    }
                    SETFLAGS(X_OF | X_CF, SF_SET_DF, NAT_FLAGS_NOFUSION);
                    ANDI(x2, xRCX, 0x1f);
                    GETEW(x1, 1);
                    CALL_(const_rol16, x1, x3, x1, x2);
                    EWBACK;
                    break;
                case 1:
                    INST_NAME("ROR Ew, CL");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_OF | X_CF);
                    }
                    SETFLAGS(X_OF | X_CF, SF_SET_DF, NAT_FLAGS_NOFUSION);
                    ANDI(x2, xRCX, 0x1f);
                    GETEW(x1, 1);
                    CALL_(const_ror16, x1, x3, x1, x2);
                    EWBACK;
                    break;
                case 2:
                    INST_NAME("RCL Ew, CL");
                    MESSAGE("LOG_DUMP", "Need optimization\n");
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_OF | X_CF);
                    } else {
                        READFLAGS(X_CF);
                    }
                    SETFLAGS(X_OF | X_CF, SF_SET_DF, NAT_FLAGS_NOFUSION);
                    ANDI(x2, xRCX, 0x1f);
                    GETEW(x1, 1);
                    CALL_(const_rcl16, x1, x3, x1, x2);
                    EWBACK;
                    break;
                case 3:
                    INST_NAME("RCR Ew, CL");
                    MESSAGE("LOG_DUMP", "Need optimization\n");
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_OF | X_CF);
                    } else {
                        READFLAGS(X_CF);
                    }
                    SETFLAGS(X_OF | X_CF, SF_SET_DF, NAT_FLAGS_NOFUSION);
                    ANDI(x2, xRCX, 0x1f);
                    GETEW(x1, 1);
                    CALL_(const_rcr16, x1, x3, x1, x2);
                    EWBACK;
                    break;
                case 5:
                    INST_NAME("SHR Ew, CL");
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_ALL);
                        SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_FUSION);
                    } else
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
                    ANDI(x2, xRCX, 0x1f);
                    CBZ_NEXT(x2);
                    GETEW(x1, 0);
                    emit_shr16(dyn, ninst, x1, x2, x5, x4, x6);
                    EWBACK;
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ew, CL");
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_ALL);
                        SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_FUSION);
                    } else
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
                    ANDI(x2, xRCX, 0x1f);
                    CBZ_NEXT(x2);
                    GETEW(x1, 0);
                    emit_shl16(dyn, ninst, x1, x2, x5, x4, x6);
                    EWBACK;
                    break;
                case 7:
                    INST_NAME("SAR Ew, CL");
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_ALL);
                        SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_FUSION);
                    } else
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
                    ANDI(x2, xRCX, 0x1f);
                    CBZ_NEXT(x2);
                    GETSEW(x1, 0);
                    emit_sar16(dyn, ninst, x1, x2, x5, x4, x6);
                    EWBACK;
                    break;
            }
            break;

        case 0xE4: /* IN AL, Ib */
        case 0xE5: /* IN AX, Ib */
        case 0xE6: /* OUT Ib, AL */
        case 0xE7: /* OUT Ib, AX */
            return dynarec64_00(dyn, addr - 1, ip, ninst, rex, ok, need_epilog);
        case 0xEC: /* IN AL, DX */
        case 0xED: /* IN AX, DX */
        case 0xEE: /* OUT DX, AL */
        case 0xEF: /* OUT DX, AX */
            return dynarec64_00(dyn, addr - 1, ip, ninst, rex, ok, need_epilog);

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
                case 4:
                    INST_NAME("MUL AX, Ew");
                    SETFLAGS(X_ALL, SF_PENDING, NAT_FLAGS_NOFUSION);
                    GETEW(x1, 0);
                    BSTRPICK_D(x2, xRAX, 15, 0);
                    MUL_W(x1, x2, x1);
                    ZEROUP(x1);
                    UFLAG_RES(x1);
                    BSTRINSz(xRAX, x1, 15, 0);
                    SRLI_D(x1, x1, 16);
                    BSTRINS_D(xRDX, x1, 15, 0);
                    UFLAG_DF(x1, d_mul16);
                    break;
                case 6:
                    INST_NAME("DIV Ew");
                    SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
                    SET_DFNONE();
                    GETEW(x1, 0);
                    BSTRPICK_D(x2, xRAX, 15, 0);
                    BSTRINS_W(x2, xRDX, 31, 16);
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
                    DIV_WU(x7, x2, ed); // warning: x2 and ed must be signed extended!
                    MOD_WU(x4, x2, ed); // warning: x2 and ed must be signed extended!
                    BSTRINSz(xRAX, x7, 15, 0);
                    BSTRINSz(xRDX, x4, 15, 0);
                    SET_DFNONE();
                    CLEAR_FLAGS(x5);
                    ADDI_D(x5, xZR, ((1 << F_ZF) | (1 << F_PF)));
                    OR(xFlags, xFlags, x5);
                    SPILL_EFLAGS();
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
                    BSTRINS_W(x2, xRDX, 31, 16);
                    DIV_W(x3, x2, ed); // warning: x2 and ed must be signed extended!
                    MOD_W(x4, x2, ed); // warning: x2 and ed must be signed extended!
                    BSTRINSz(xRAX, x3, 15, 0);
                    BSTRINSz(xRDX, x4, 15, 0);
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xF8:
        case 0xF9:
            return dynarec64_00(dyn, addr - 1, ip, ninst, rex, ok, need_epilog);
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
