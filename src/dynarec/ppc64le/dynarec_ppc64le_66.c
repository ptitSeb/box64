#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "ppc64le_emitter.h"
#include "ppc64le_mapping.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"
#include "custommem.h"

#include "ppc64le_printer.h"
#include "dynarec_ppc64le_private.h"
#include "../dynarec_helper.h"
#include "dynarec_ppc64le_functions.h"


uintptr_t dynarec64_66(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    uint8_t opcode = F8;
    uint8_t nextop, u8;
    int16_t i16;
    uint16_t u16;
    uint64_t u64;
    int32_t i32;
    int64_t i64, j64;
    uint8_t gd, ed;
    uint8_t wback, wb1, wb2;
    uint8_t gb1, gb2;
    int64_t fixedaddress;
    int unscaled;
    int lock;
    MAYUSE(u8);
    MAYUSE(u16);
    MAYUSE(u64);
    MAYUSE(i64);
    MAYUSE(j64);
    MAYUSE(lock);
    MAYUSE(wb2);
    MAYUSE(gb1);
    MAYUSE(gb2);

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
            emit_add16(dyn, ninst, gd, ed, x4, x5, x6);
            GWBACK;
            break;
        case 0x05:
            INST_NAME("ADD AX, Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i32 = F16;
            BF_EXTRACT(x1, xRAX, 15, 0);
            MOV32w(x2, i32);
            emit_add16(dyn, ninst, x1, x2, x3, x4, x5);
            BF_INSERT(xRAX, x1, 15, 0);
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
            BF_EXTRACT(x1, xRAX, 15, 0);
            MOV32w(x2, i32);
            emit_or16(dyn, ninst, x1, x2, x3, x4);
            BF_INSERT(xRAX, x1, 15, 0);
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
            emit_adc16(dyn, ninst, ed, gd, x4, x5, x6);
            EWBACK;
            break;
        case 0x13:
            INST_NAME("ADC Gw, Ew");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGWEW(x1, x2, 0);
            emit_adc16(dyn, ninst, gd, ed, x4, x5, x6);
            GWBACK;
            break;
        case 0x15:
            INST_NAME("ADC AX, Iw");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i32 = F16;
            BF_EXTRACT(x1, xRAX, 15, 0);
            MOV32w(x2, i32);
            emit_adc16(dyn, ninst, x1, x2, x3, x4, x5);
            BF_INSERT(xRAX, x1, 15, 0);
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
            emit_sbb16(dyn, ninst, gd, ed, x4, x5, x6);
            GWBACK;
            break;
        case 0x1D:
            INST_NAME("SBB AX, Iw");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i32 = F16;
            BF_EXTRACT(x1, xRAX, 15, 0);
            MOV32w(x2, i32);
            emit_sbb16(dyn, ninst, x1, x2, x3, x4, x5);
            BF_INSERT(xRAX, x1, 15, 0);
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
            emit_and16(dyn, ninst, gd, ed, x4, x5);
            GWBACK;
            break;
        case 0x25:
            INST_NAME("AND AX, Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i32 = F16;
            BF_EXTRACT(x1, xRAX, 15, 0);
            MOV32w(x2, i32);
            emit_and16(dyn, ninst, x1, x2, x3, x4);
            BF_INSERT(xRAX, x1, 15, 0);
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
            emit_sub16(dyn, ninst, gd, ed, x4, x5, x6);
            GWBACK;
            break;
        case 0x2D:
            INST_NAME("SUB AX, Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i32 = F16;
            BF_EXTRACT(x1, xRAX, 15, 0);
            MOV32w(x2, i32);
            emit_sub16(dyn, ninst, x1, x2, x3, x4, x5);
            BF_INSERT(xRAX, x1, 15, 0);
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
            emit_xor16(dyn, ninst, gd, ed, x4, x5, x6);
            GWBACK;
            break;
        case 0x35:
            INST_NAME("XOR AX, Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i32 = F16;
            BF_EXTRACT(x1, xRAX, 15, 0);
            MOV32w(x2, i32);
            emit_xor16(dyn, ninst, x1, x2, x3, x4, x5);
            BF_INSERT(xRAX, x1, 15, 0);
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
            BF_EXTRACT(x1, xRAX, 15, 0);
            if (i32) {
                MOV32w(x2, i32);
                emit_cmp16(dyn, ninst, x1, x2, x3, x4, x5, x6);
            } else {
                emit_cmp16_0(dyn, ninst, x1, x3, x4);
            }
            break;
        case 0x68:
            INST_NAME("PUSH Iw");
            i32 = F16;
            MOV32w(x3, i32);
            PUSH1_16(x3);
            break;
        case 0x69:
            INST_NAME("IMUL Gw, Ew, Iw");
            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            nextop = F8;
            GETSEW(x1, 2);
            i32 = F16;
            MOV32w(x2, i32);
            MULLW(x2, x2, ed);
            EXTSH(x1, x2);
            BF_EXTRACT(x2, x2, 15, 0);
            gd = x2;
            GWBACK;
            UFLAG_RES(x2);
            UFLAG_OP1(x1);
            UFLAG_DF(x1, d_imul16);
            break;
        case 0x6A:
            INST_NAME("PUSH Ib");
            i32 = F8S;
            MOV64z(x3, (uint16_t)i32);
            PUSH1_16(x3);
            break;
        case 0x6B:
            INST_NAME("IMUL Gw, Ew, Ib");
            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            nextop = F8;
            GETSEW(x1, 1);
            i32 = F8S;
            MOV32w(x2, i32);
            MULLW(x2, x2, ed);
            EXTSH(x1, x2);
            BF_EXTRACT(x2, x2, 15, 0);
            gd = x2;
            GWBACK;
            UFLAG_RES(x2);
            UFLAG_OP1(x1);
            UFLAG_DF(x1, d_imul16);
            break;
        case 0x81:
        case 0x83:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                    if (opcode == 0x81) {
                        INST_NAME("ADD Ew, Iw");
                    } else {
                        INST_NAME("ADD Ew, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEW(x1, (opcode == 0x81) ? 2 : 1);
                    if (opcode == 0x81) i32 = F16; else i32 = (uint16_t)(int16_t)F8S;
                    MOV32w(x5, i32);
                    emit_add16(dyn, ninst, ed, x5, x2, x4, x6);
                    EWBACK;
                    break;
                case 1:
                    if (opcode == 0x81) {
                        INST_NAME("OR Ew, Iw");
                    } else {
                        INST_NAME("OR Ew, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEW(x1, (opcode == 0x81) ? 2 : 1);
                    if (opcode == 0x81) i32 = F16; else i32 = (uint16_t)(int16_t)F8S;
                    MOV32w(x5, i32);
                    emit_or16(dyn, ninst, ed, x5, x2, x4);
                    EWBACK;
                    break;
                case 2:
                    if (opcode == 0x81) {
                        INST_NAME("ADC Ew, Iw");
                    } else {
                        INST_NAME("ADC Ew, Ib");
                    }
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEW(x1, (opcode == 0x81) ? 2 : 1);
                    if (opcode == 0x81) i32 = F16; else i32 = (uint16_t)(int16_t)F8S;
                    MOV32w(x5, i32);
                    emit_adc16(dyn, ninst, ed, x5, x2, x4, x6);
                    EWBACK;
                    break;
                case 3:
                    if (opcode == 0x81) {
                        INST_NAME("SBB Ew, Iw");
                    } else {
                        INST_NAME("SBB Ew, Ib");
                    }
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEW(x1, (opcode == 0x81) ? 2 : 1);
                    if (opcode == 0x81) i32 = F16; else i32 = (uint16_t)(int16_t)F8S;
                    MOV32w(x5, i32);
                    emit_sbb16(dyn, ninst, ed, x5, x2, x4, x6);
                    EWBACK;
                    break;
                case 4:
                    if (opcode == 0x81) {
                        INST_NAME("AND Ew, Iw");
                    } else {
                        INST_NAME("AND Ew, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEW(x1, (opcode == 0x81) ? 2 : 1);
                    if (opcode == 0x81) i32 = F16; else i32 = (uint16_t)(int16_t)F8S;
                    MOV32w(x5, i32);
                    emit_and16(dyn, ninst, ed, x5, x2, x4);
                    EWBACK;
                    break;
                case 5:
                    if (opcode == 0x81) {
                        INST_NAME("SUB Ew, Iw");
                    } else {
                        INST_NAME("SUB Ew, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEW(x1, (opcode == 0x81) ? 2 : 1);
                    if (opcode == 0x81) i32 = F16; else i32 = (uint16_t)(int16_t)F8S;
                    MOV32w(x5, i32);
                    emit_sub16(dyn, ninst, ed, x5, x2, x4, x6);
                    EWBACK;
                    break;
                case 6:
                    if (opcode == 0x81) {
                        INST_NAME("XOR Ew, Iw");
                    } else {
                        INST_NAME("XOR Ew, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEW(x1, (opcode == 0x81) ? 2 : 1);
                    if (opcode == 0x81) i32 = F16; else i32 = (uint16_t)(int16_t)F8S;
                    MOV32w(x5, i32);
                    emit_xor16(dyn, ninst, ed, x5, x2, x4, x6);
                    EWBACK;
                    break;
                case 7:
                    if (opcode == 0x81) {
                        INST_NAME("CMP Ew, Iw");
                    } else {
                        INST_NAME("CMP Ew, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEW(x1, (opcode == 0x81) ? 2 : 1);
                    if (opcode == 0x81) i32 = F16; else i32 = (uint16_t)(int16_t)F8S;
                    if (i32) {
                        MOV32w(x5, i32);
                        emit_cmp16(dyn, ninst, ed, x5, x2, x3, x4, x6);
                    } else {
                        emit_cmp16_0(dyn, ninst, ed, x3, x4);
                    }
                    break;
                default: DEFAULT;
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
            gd = TO_NAT(((nextop & 0x38) >> 3) + (rex.r << 3));
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                BF_EXTRACT(x1, gd, 15, 0);
                BF_INSERT(gd, ed, 15, 0);
                BF_INSERT(ed, x1, 15, 0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, &lock, DS_DISP, 0);
                SMREADLOCK(lock);
                LHZ(x1, fixedaddress, wback);
                BF_EXTRACT(x3, gd, 15, 0);
                STH(x3, fixedaddress, wback);
                SMWRITELOCK(lock);
                BF_INSERT(gd, x1, 15, 0);
            }
            break;
        case 0x89:
            INST_NAME("MOV Ew, Gw");
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                if (ed != gd) BF_INSERT(ed, gd, 15, 0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, DS_DISP, 0);
                STH(gd, fixedaddress, ed);
                SMWRITELOCK(lock);
            }
            break;
        case 0x8B:
            INST_NAME("MOV Gw, Ew");
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                if (ed != gd) BF_INSERT(gd, ed, 15, 0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, DS_DISP, 0);
                SMREADLOCK(lock);
                LHZ(x1, fixedaddress, ed);
                BF_INSERT(gd, x1, 15, 0);
            }
            break;
        case 0x8D:
            INST_NAME("LEA Gw, Ed");
            nextop = F8;
            GETGD;
            if (MODREG) {
                DEFAULT;
            } else {
                rex.seg = 0; // LEA doesn't use segment
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, NO_DISP, 0);
                // 16-bit LEA: insert only bits 0-15 into gd, preserving bits 16-63
                BF_INSERT(gd, ed, 15, 0);
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
                BF_INSERT(xRAX, gd, 15, 0);
                BF_INSERT(gd, x2, 15, 0);
            }
            break;
        case 0x98:
            INST_NAME("CBW");
            EXTSB(x1, xRAX);
            BF_INSERT(xRAX, x1, 15, 0);
            break;
        case 0x99:
            INST_NAME("CWD");
            EXTSH(x1, xRAX);
            SRADI(x1, x1, 15);
            BF_INSERT(xRDX, x1, 15, 0);
            break;
        case 0xA1:
            INST_NAME("MOV EAX, Od");
            if (rex.is32bits) {
                i64 = F32;
            } else {
                i64 = F64;
            }
            MOV64z(x1, i64);
            LHZ(x2, 0, x1);
            BF_INSERT(xRAX, x2, 15, 0);
            break;
        case 0xA3:
            INST_NAME("MOV Od, EAX");
            if (rex.is32bits) {
                i64 = F32;
            } else {
                i64 = F64;
            }
            MOV64z(x1, i64);
            STH(xRAX, 0, x1);
            break;
        case 0xA9:
            INST_NAME("TEST AX, Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            u16 = F16;
            BF_EXTRACT(x1, xRAX, 15, 0);
            MOV32w(x2, u16);
            emit_test16(dyn, ninst, x1, x2, x3, x4, x5);
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
            gd = TO_NAT((opcode & 7) + (rex.b << 3));
            MOV32w(x1, u16);
            BF_INSERT(gd, x1, 15, 0);
            break;
        case 0xC1:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                    INST_NAME("ROL Ew, Ib");
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION);
                    GETEW(x1, 1);
                    u8 = F8;
                    emit_rol16c(dyn, ninst, ed, u8 & 0x1f, x4, x5, x6);
                    EWBACK;
                    break;
                case 1:
                    INST_NAME("ROR Ew, Ib");
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION);
                    GETEW(x1, 1);
                    u8 = F8;
                    emit_ror16c(dyn, ninst, ed, u8 & 0x1f, x4, x5);
                    EWBACK;
                    break;
                case 4:
                    INST_NAME("SHL Ew, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEW(x1, 1);
                    u8 = (F8) & 0x1f;
                    emit_shl16c(dyn, ninst, ed, u8, x5, x4, x6);
                    EWBACK;
                    break;
                case 5:
                    INST_NAME("SHR Ew, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEW(x1, 1);
                    u8 = (F8) & 0x1f;
                    emit_shr16c(dyn, ninst, ed, u8, x5, x4, x6);
                    EWBACK;
                    break;
                case 7:
                    INST_NAME("SAR Ew, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETSEW(x1, 1);
                    u8 = (F8) & 0x1f;
                    emit_sar16c(dyn, ninst, ed, u8, x5, x4, x6);
                    EWBACK;
                    break;
                default: DEFAULT;
            }
            break;
        case 0xC7:
            INST_NAME("MOV Ew, Iw");
            nextop = F8;
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                u16 = F16;
                MOV32w(x1, u16);
                BF_INSERT(ed, x1, 15, 0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, DS_DISP, 2);
                u16 = F16;
                MOV32w(x1, u16);
                STH(x1, fixedaddress, ed);
                SMWRITELOCK(lock);
            }
            break;
        case 0xD1:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                    INST_NAME("ROL Ew, 1");
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION);
                    GETEW(x1, 0);
                    emit_rol16c(dyn, ninst, ed, 1, x5, x4, x6);
                    EWBACK;
                    break;
                case 1:
                    INST_NAME("ROR Ew, 1");
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION);
                    GETEW(x1, 0);
                    emit_ror16c(dyn, ninst, ed, 1, x5, x4);
                    EWBACK;
                    break;
                case 4:
                    INST_NAME("SHL Ew, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEW(x1, 0);
                    emit_shl16c(dyn, ninst, ed, 1, x5, x4, x6);
                    EWBACK;
                    break;
                case 5:
                    INST_NAME("SHR Ew, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEW(x1, 0);
                    emit_shr16c(dyn, ninst, ed, 1, x5, x4, x6);
                    EWBACK;
                    break;
                case 7:
                    INST_NAME("SAR Ew, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETSEW(x1, 0);
                    emit_sar16c(dyn, ninst, ed, 1, x5, x4, x6);
                    EWBACK;
                    break;
                default: DEFAULT;
            }
            break;
        case 0xD3:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 4:
                    INST_NAME("SHL Ew, CL");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEW(x1, 0);
                    BF_EXTRACT(x2, xRCX, 4, 0);
                    emit_shl16(dyn, ninst, ed, x2, x5, x4, x6);
                    EWBACK;
                    break;
                case 5:
                    INST_NAME("SHR Ew, CL");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEW(x1, 0);
                    BF_EXTRACT(x2, xRCX, 4, 0);
                    emit_shr16(dyn, ninst, ed, x2, x5, x4, x6);
                    EWBACK;
                    break;
                case 7:
                    INST_NAME("SAR Ew, CL");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETSEW(x1, 0);
                    BF_EXTRACT(x2, xRCX, 4, 0);
                    emit_sar16(dyn, ninst, ed, x2, x5, x4, x6);
                    EWBACK;
                    break;
                default: DEFAULT;
            }
            break;
        case 0xF7:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                case 1:
                    INST_NAME("TEST Ew, Iw");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEW(x1, 2);
                    u16 = F16;
                    MOV32w(x5, u16);
                    emit_test16(dyn, ninst, ed, x5, x3, x4, x6);
                    break;
                case 2:
                    INST_NAME("NOT Ew");
                    GETEW(x1, 0);
                    NOT(x1, ed);
                    ed = x1;
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
                    SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
                    GETEW(x1, 0);
                    BF_EXTRACT(x2, xRAX, 15, 0);
                    MULLW(x2, x2, ed);
                    UFLAG_IF {
                        STW(x2, offsetof(x64emu_t, res), xEmu);  // store full 32-bit product
                    }
                    BF_INSERT(xRAX, x2, 15, 0);
                    SRDI(x2, x2, 16);
                    BF_INSERT(xRDX, x2, 15, 0);
                    UFLAG_DF(x2, d_mul16);
                    break;
                case 5:
                    INST_NAME("IMUL AX, Ew");
                    SETFLAGS(X_ALL, SF_PENDING, NAT_FLAGS_NOFUSION);
                    GETSEW(x1, 0);
                    EXTSH(x2, xRAX);
                    MULLW(x2, x2, ed);
                    UFLAG_IF {
                        STW(x2, offsetof(x64emu_t, res), xEmu);  // store full 32-bit product
                    }
                    BF_INSERT(xRAX, x2, 15, 0);
                    SRDI(x2, x2, 16);
                    BF_INSERT(xRDX, x2, 15, 0);
                    UFLAG_DF(x2, d_imul16);
                    break;
                case 6:
                    INST_NAME("DIV Ew");
                    SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
                    GETEW(x1, 0);
                    BF_EXTRACT(x2, xRDX, 15, 0);
                    SLDI(x2, x2, 16);
                    BF_EXTRACT(x3, xRAX, 15, 0);
                    OR(x2, x2, x3);
                    DIVWU(x3, x2, ed);
                    MODUW(x4, x2, ed);
                    BF_INSERT(xRAX, x3, 15, 0);
                    BF_INSERT(xRDX, x4, 15, 0);
                    SET_DFNONE();
                    break;
                case 7:
                    INST_NAME("IDIV Ew");
                    SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
                    GETSEW(x1, 0);
                    EXTSH(x2, xRDX);
                    SLDI(x2, x2, 16);
                    BF_EXTRACT(x3, xRAX, 15, 0);
                    OR(x2, x2, x3);
                    EXTSW(x2, x2);
                    DIVW(x3, x2, ed);
                    MODSW(x4, x2, ed);
                    BF_INSERT(xRAX, x3, 15, 0);
                    BF_INSERT(xRDX, x4, 15, 0);
                    SET_DFNONE();
                    break;
                default: DEFAULT;
            }
            break;
        case 0xFF:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                    INST_NAME("INC Ew");
                    SETFLAGS(X_ALL & ~X_CF, SF_SUBSET, NAT_FLAGS_FUSION);
                    GETEW(x1, 0);
                    emit_inc16(dyn, ninst, ed, x2, x4, x5);
                    EWBACK;
                    break;
                case 1:
                    INST_NAME("DEC Ew");
                    SETFLAGS(X_ALL & ~X_CF, SF_SUBSET, NAT_FLAGS_FUSION);
                    GETEW(x1, 0);
                    emit_dec16(dyn, ninst, ed, x2, x4, x5, x6);
                    EWBACK;
                    break;
                case 6:
                    INST_NAME("PUSH Ew");
                    GETEW(x1, 0);
                    PUSH1_16(ed);
                    break;
                default: DEFAULT;
            }
            break;
        default:
            DEFAULT;
    }

    return addr;
}
