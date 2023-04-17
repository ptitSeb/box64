#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
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

    while((opcode==0x2E) || (opcode==0x36) || (opcode==0x66))   // ignoring CS:, SS: or multiple 0x66
        opcode = F8;

    while((opcode==0xF2) || (opcode==0xF3)) {
        rep = opcode-0xF1;
        opcode = F8;
    }
    // REX prefix before the 66 are ignored
    rex.rex = 0;
    while(opcode>=0x40 && opcode<=0x4f) {
        rex.rex = opcode;
        opcode = F8;
    }

    if(rex.w && opcode!=0x0f)   // rex.w cancels "66", but not for 66 0f type of prefix
        return dynarec64_00(dyn, addr-1, ip, ninst, rex, rep, ok, need_epilog); // addr-1, to "put back" opcode

    switch(opcode) {
        case 0x01:
            INST_NAME("ADD Ew, Gw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x2);
            GETEW(x1, 0);
            emit_add16(dyn, ninst, x1, x2, x4, x5);
            EWBACK;
            break;
        case 0x03:
            INST_NAME("ADD Gw, Ew");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x1);
            GETEW(x2, 0);
            emit_add16(dyn, ninst, x1, x2, x3, x4);
            GWBACK;
            break;
        case 0x05:
            INST_NAME("ADD AX, Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i32 = F16;
            SLLI(x1 , xRAX, 48);
            SRLI(x1, x1, 48);
            MOV32w(x2, i32);
            emit_add16(dyn, ninst, x1, x2, x3, x4);
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
            SLLI(x1, xRAX, 48);
            SRLI(x1, x1, 48);
            MOV32w(x2, i32);
            emit_or16(dyn, ninst, x1, x2, x3, x4);
            LUI(x3, 0xffff0);
            AND(xRAX, xRAX, x3);
            OR(xRAX, xRAX, x1);
            break;
        case 0x0F:
            addr = dynarec64_660F(dyn, addr, ip, ninst, rex, ok, need_epilog);
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
            SLLI(x1, xRAX, 48);
            SRLI(x1, x1, 48);
            MOV32w(x2, i32);
            emit_and16(dyn, ninst, x1, x2, x3, x4);
            LUI(x3, 0xffff0);
            AND(xRAX, xRAX, x3);
            OR(xRAX, xRAX, x1);
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
            SLLI(x1, xRAX, 48);
            SRLI(x1, x1, 48);
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
            GETGW(x2);
            GETEW(x1, 0);
            emit_xor16(dyn, ninst, x1, x2, x4, x5, x6);
            EWBACK;
            break;
        case 0x33:
            INST_NAME("XOR Gw, Ew");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x1);
            GETEW(x2, 0);
            emit_xor16(dyn, ninst, x1, x2, x4, x5, x6);
            GWBACK;
            break;
        case 0x35:
            INST_NAME("XOR AX, Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i32 = F16;
            SLLI(x1, xRAX, 48);
            SRLI(x1, x1, 48);
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
            SLLI(x1, xRAX, 48);
            SRLI(x1, x1, 48);
            if(i32) {
                MOV32w(x2, i32);
                emit_cmp16(dyn, ninst, x1, x2, x3, x4, x5, x6);
            } else {
                emit_cmp16_0(dyn, ninst, x1, x3, x4);
            }
            break;
        case 0x69:
        case 0x6B:
            if(opcode==0x69) {
                INST_NAME("IMUL Gw,Ew,Iw");
            } else {
                INST_NAME("IMUL Gw,Ew,Ib");
            }
            SETFLAGS(X_ALL, SF_PENDING);
            nextop = F8;
            UFLAG_DF(x1, d_imul16);
            GETSEW(x1, (opcode==0x69)?2:1);
            if(opcode==0x69) i32 = F16S; else i32 = F8S;
            MOV32w(x2, i32);
            MULW(x2, x2, x1);
            SLLI(x2, x2, 48);
            SRLI(x2, x2, 48);
            UFLAG_RES(x2);
            gd=x2;
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
            return dynarec64_00(dyn, addr-1, ip, ninst, rex, rep, ok, need_epilog);

        case 0x81:
        case 0x83:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0: // ADD
                    if(opcode==0x81) {INST_NAME("ADD Ew, Iw");} else {INST_NAME("ADD Ew, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEW(x1, (opcode==0x81)?2:1);
                    if(opcode==0x81) i16 = F16S; else i16 = F8S;
                    MOV64x(x5, i16);
                    emit_add16(dyn, ninst, ed, x5, x2, x4);
                    EWBACK;
                    break;
                case 1: // OR
                    if(opcode==0x81) {INST_NAME("OR Ew, Iw");} else {INST_NAME("OR Ew, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEW(x1, (opcode==0x81)?2:1);
                    if(opcode==0x81) i16 = F16S; else i16 = F8S;
                    MOV64x(x5, i16);
                    emit_or16(dyn, ninst, x1, x5, x2, x4);
                    EWBACK;
                    break;
                case 2: // ADC
                    if(opcode==0x81) {INST_NAME("ADC Ew, Iw");} else {INST_NAME("ADC Ew, Ib");}
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEW(x1, (opcode==0x81)?2:1);
                    if(opcode==0x81) i16 = F16S; else i16 = F8S;
                    MOV64x(x5, i16);
                    emit_adc16(dyn, ninst, x1, x5, x2, x4, x6);
                    EWBACK;
                    break;
                case 4: // AND
                    if(opcode==0x81) {INST_NAME("AND Ew, Iw");} else {INST_NAME("AND Ew, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEW(x1, (opcode==0x81)?2:1);
                    if(opcode==0x81) i16 = F16S; else i16 = F8S;
                    MOV64x(x5, i16);
                    emit_and16(dyn, ninst, x1, x5, x2, x4);
                    EWBACK;
                    break;
                case 5: // SUB
                    if(opcode==0x81) {INST_NAME("SUB Ew, Iw");} else {INST_NAME("SUB Ew, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEW(x1, (opcode==0x81)?2:1);
                    if(opcode==0x81) i16 = F16S; else i16 = F8S;
                    MOV32w(x5, i16);
                    emit_sub16(dyn, ninst, x1, x5, x2, x4, x6);
                    EWBACK;
                    break;
                case 6: // XOR
                    if(opcode==0x81) {INST_NAME("XOR Ew, Iw");} else {INST_NAME("XOR Ew, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEW(x1, (opcode==0x81)?2:1);
                    if(opcode==0x81) i16 = F16S; else i16 = F8S;
                    MOV32w(x5, i16);
                    emit_xor16(dyn, ninst, x1, x5, x2, x4, x6);
                    EWBACK;
                    break;
                case 7: // CMP
                    if(opcode==0x81) {INST_NAME("CMP Ew, Iw");} else {INST_NAME("CMP Ew, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEW(x1, (opcode==0x81)?2:1);
                    if(opcode==0x81) i16 = F16S; else i16 = F8S;
                    if(i16) {
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
            if(MODREG) {
                ed = xRAX+(nextop&7)+(rex.b<<3);
                if(ed!=gd) {
                    // we don't use GETGW above, so we need let gd & 0xffff.
                    LUI(x1, 0xffff0);
                    AND(ed, ed, x1);
                    SLLI(x2, gd, 48);
                    SRLI(x2, x2, 48);
                    OR(ed, ed, x2);
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, 0, 0);
                SH(gd, ed, fixedaddress);
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
                gd = xRAX+(opcode&0x07)+(rex.b<<3);
                if(gd==xRAX) {
                    INST_NAME("NOP");
                } else {
                    INST_NAME("XCHG AX, Reg");
                    LUI(x4, 0xffff0);
                    // x2 <- rax
                    MV(x2, xRAX);
                    // rax[15:0] <- gd[15:0]
                    SLLI(x3, gd, 48);
                    SRLI(x3, x3, 48);
                    AND(xRAX, xRAX, x4);
                    OR(xRAX, xRAX, x3);
                    // gd[15:0] <- x2[15:0]
                    SLLI(x2, x2, 48);
                    SRLI(x2, x2, 48);
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
        case 0xA9:
            INST_NAME("TEST AX,Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            u16 = F16;
            MOV32w(x2, u16);
            SLLIW(x1, xRAX, 16);
            SRLIW(x1, x1, 16);
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
            INST_NAME("MOV Reg, Iw");
            gd = xRAX+(opcode&7)+(rex.b<<3);
            if(rex.w) {
                u64 = F64;
                MOV64x(gd, u64);
            } else {
                u16 = F16;
                MOV64x(x1, ~0xffff);
                AND(gd, gd, x1);
                if(u16<2048) {
                    ORI(gd, gd, u16);
                } else {
                    MOV32w(x1, u16);
                    OR(gd, gd, x1);
                }
            }
            break;
            
        case 0xC1:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("ROL Ew, Ib");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    SETFLAGS(X_OF|X_CF, SF_SET);
                    GETEW(x1, 1);
                    u8 = F8;
                    MOV32w(x2, u8);
                    CALL_(rol16, x1, x3);
                    EWBACK;
                    break;
                case 1:
                    INST_NAME("ROR Ew, Ib");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    SETFLAGS(X_OF|X_CF, SF_SET);
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
                    SETFLAGS(X_OF|X_CF, SF_SET);
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
                    SETFLAGS(X_OF|X_CF, SF_SET);
                    GETEW(x1, 1);
                    u8 = F8;
                    MOV32w(x2, u8);
                    CALL_(rcr16, x1, x3);
                    EWBACK;
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ew, Ib");
                    UFLAG_IF {MESSAGE(LOG_DUMP, "Need Optimization for flags\n");}
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETEW(x1, 1);
                    u8 = F8;
                    UFLAG_IF {MOV32w(x2, (u8&0x1f));}
                    UFLAG_OP12(ed, x2)
                    if(MODREG) {
                        SLLI(ed, ed, 48+(u8&0x1f));
                        SRLI(ed, ed, 48);
                    } else {
                        SLLI(ed, ed, u8&0x1f);
                    }
                    EWBACK;
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, d_shl16);
                    break;
                case 5:
                    INST_NAME("SHR Ed, Ib");
                    UFLAG_IF {MESSAGE(LOG_DUMP, "Need Optimization for flags\n");}
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETEW(x1, 1);
                    u8 = F8;
                    UFLAG_IF {MOV32w(x2, (u8&0x1f));}
                    UFLAG_OP12(ed, x2)
                    SRLI(ed, ed, u8&0x1f);
                    EWBACK;
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, d_shr16);
                    break;
                case 7:
                    INST_NAME("SAR Ed, Ib");
                    SETFLAGS(X_ALL, SF_PENDING);
                    UFLAG_IF {MESSAGE(LOG_DUMP, "Need Optimization for flags\n");}
                    GETSEW(x1, 1);
                    u8 = F8;
                    UFLAG_IF {MOV32w(x2, (u8&0x1f));}
                    UFLAG_OP12(ed, x2)
                    SRAI(ed, ed, u8&0x1f);
                    if(MODREG) {
                        SLLI(ed, ed, 48);
                        SRLI(ed, ed, 48);
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
            if(MODREG) {
                ed = xRAX+(nextop&7)+(rex.b<<3);
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
            switch((nextop>>3)&7) {
                case 5:
                    if(opcode==0xD1) {
                        INST_NAME("SHR Ew, 1");
                        MOV32w(x4, 1);
                    } else {
                        INST_NAME("SHR Ew, CL");
                        ANDI(x4, xRCX, 0x1f);
                    }
                    UFLAG_IF {MESSAGE(LOG_DUMP, "Need Optimization for flags\n");}
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETEW(x1, 0);
                    UFLAG_OP12(ed, x4)
                    SRL(ed, ed, x4);
                    EWBACK;
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, d_shr16);
                    break;
                case 7:
                    if(opcode==0xD1) {
                        INST_NAME("SAR Ew, 1");
                        MOV32w(x4, 1);
                    } else {
                        INST_NAME("SAR Ew, CL");
                        ANDI(x4, xRCX, 0x1f);
                    }
                    UFLAG_IF {MESSAGE(LOG_DUMP, "Need Optimization for flags\n");}
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETSEW(x1, 0);
                    UFLAG_OP12(ed, x4)
                    SRA(ed, ed, x4);
                    SLLI(ed, ed, 48);
                    SRLI(ed, ed, 48);
                    EWBACK;
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, d_sar16);
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xF7:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                case 1:
                    INST_NAME("TEST Ew, Iw");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEW(x1, 2);
                    u16 = F16;
                    MOV32w(x2, u16);
                    emit_test16(dyn, ninst, x1, x2, x3, x4, x5);
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
                    SLLI(x2, xRAX, 48);
                    SLLI(x3, xRDX, 48);
                    SRLI(x2, x2, 48);
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
                    SLLI(x2, xRAX, 48);
                    SLLI(x3, xRDX, 48);
                    SRLI(x2, x2, 48);
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

        default:
            DEFAULT;
    }
    return addr;
}
