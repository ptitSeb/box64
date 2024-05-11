#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "dynarec.h"
#include "emu/x64emu_private.h"
#include "emu/x64run_private.h"
#include "la64_emitter.h"
#include "x64run.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"
#include "custommem.h"

#include "la64_printer.h"
#include "dynarec_la64_private.h"
#include "dynarec_la64_helper.h"
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
            BSTRPICK_D(x1, xRAX, 15, 0);
            MOV32w(x2, i32);
            emit_add16(dyn, ninst, x1, x2, x3, x4, x6);
            BSTRINS_D(xRAX, x1, 15, 0);
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
        case 0x0F:
            switch (rep) {
                case 0: addr = dynarec64_660F(dyn, addr, ip, ninst, rex, ok, need_epilog); break;
                default:
                    DEFAULT;
            }
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
            emit_and16(dyn, ninst, x1, x2, x3, x4);
            GWBACK;
            break;
        case 0x25:
            INST_NAME("AND AX, Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i32 = F16;
            BSTRPICK_D(x1, xRAX, 15, 0);
            MOV32w(x2, i32);
            emit_and16(dyn, ninst, x1, x2, x3, x4);
            BSTRINS_D(xRAX, x1, 15, 0);
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
            BSTRPICK_D(x1, xRAX, 15, 0);
            MOV32w(x2, i32);
            emit_sub16(dyn, ninst, x1, x2, x3, x4, x5);
            BSTRINS_D(xRAX, x1, 15, 0);
            break;
        case 0x31:
            INST_NAME("XOR Ew, Gw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x2);
            GETEW(x1, 0);
            emit_xor16(dyn, ninst, x1, x2, x4, x5, x6);
            EWBACK;
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
            BSTRPICK_D(x1, xRAX, 15, 0);
            if (i32) {
                MOV32w(x2, i32);
                emit_cmp16(dyn, ninst, x1, x2, x3, x4, x5, x6);
            } else {
                emit_cmp16_0(dyn, ninst, x1, x3, x4);
            }
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
        case 0x89:
            INST_NAME("MOV Ew, Gw");
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = TO_LA64((nextop & 7) + (rex.b << 3));
                if (ed != gd) {
                    BSTRINS_D(ed, gd, 15, 0);
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, 1, 0);
                ST_H(gd, ed, fixedaddress);
                SMWRITELOCK(lock);
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
            gd = TO_LA64((opcode & 0x07) + (rex.b << 3));
            if (gd == xRAX) {
                INST_NAME("NOP");
            } else {
                INST_NAME("XCHG AX, Reg");
                MV(x2, xRAX);
                BSTRINS_D(xRAX, gd, 15, 0);
                BSTRINS_D(gd, x2, 15, 0);
            }
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
            gd = TO_LA64((opcode & 7) + (rex.b << 3));
            BSTRINS_D(gd, x1, 15, 0);
            break;
        case 0xC1:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                    INST_NAME("ROL Ew, Ib");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    SETFLAGS(X_OF | X_CF, SF_SET_DF);
                    GETEW(x1, 1);
                    u8 = F8;
                    MOV32w(x2, u8);
                    CALL_(rol16, x1, x3);
                    EWBACK;
                    break;
                case 1:
                    INST_NAME("ROR Ew, Ib");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    SETFLAGS(X_OF | X_CF, SF_SET_DF);
                    GETEW(x1, 1);
                    u8 = F8;
                    MOV32w(x2, u8);
                    CALL_(ror16, x1, x3);
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
                        SLLI_D(ed, ed, 48 + (u8 & 15));
                        SRLI_D(ed, ed, 48);
                    } else {
                        SLLI_D(ed, ed, u8 & 15);
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
                    SRLI_D(ed, ed, u8 & 15);
                    EWBACK;
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, d_shr16);
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xC7:
            INST_NAME("MOV Ew, Iw");
            nextop = F8;
            if (MODREG) {
                ed = TO_LA64((nextop & 7) + (rex.b << 3));
                u16 = F16;
                MOV32w(x1, u16);
                BSTRINS_D(ed, x1, 15, 0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, 1, 2);
                u16 = F16;
                MOV32w(x1, u16);
                ST_H(x1, ed, fixedaddress);
                SMWRITELOCK(lock);
            }
            break;
        case 0xD1:
        case 0xD3:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 5:
                    if (opcode == 0xD1) {
                        INST_NAME("SHR Ew, 1");
                        MOV32w(x2, 1);
                    } else {
                        INST_NAME("SHR Ew, CL");
                        ANDI(x2, xRCX, 0x1f);
                        BEQ_NEXT(x2, xZR);
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING); // some flags are left undefined
                    if (box64_dynarec_safeflags > 1) MAYSETFLAGS();
                    GETEW(x1, 0);
                    emit_shr16(dyn, ninst, x1, x2, x5, x4, x6);
                    EWBACK;
                    break;
                case 4:
                case 6:
                    if (opcode == 0xD1) {
                        INST_NAME("SHL Ew, 1");
                        MOV32w(x2, 1);
                    } else {
                        INST_NAME("SHL Ew, CL");
                        ANDI(x2, xRCX, 0x1f);
                        BEQ_NEXT(x2, xZR);
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING); // some flags are left undefined
                    if (box64_dynarec_safeflags > 1)
                        MAYSETFLAGS();
                    GETEW(x1, 0);
                    emit_shl16(dyn, ninst, x1, x2, x5, x4, x6);
                    EWBACK;
                    break;
                case 7:
                    if (opcode == 0xD1) {
                        INST_NAME("SAR Ew, 1");
                        MOV32w(x2, 1);
                    } else {
                        INST_NAME("SAR Ew, CL");
                        ANDI(x2, xRCX, 0x1f);
                        BEQ_NEXT(x2, xZR);
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING); // some flags are left undefined
                    if (box64_dynarec_safeflags > 1)
                        MAYSETFLAGS();
                    GETSEW(x1, 0);
                    emit_sar16(dyn, ninst, x1, x2, x5, x4, x6);
                    EWBACK;
                    break;
                default:
                    DEFAULT;
            }
            break;
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
                default:
                    DEFAULT;
            }
        default:
            DEFAULT;
    }
    return addr;
}
