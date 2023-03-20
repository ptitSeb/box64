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

        case 0x0F:
            addr = dynarec64_660F(dyn, addr, ip, ninst, rex, ok, need_epilog);
            break;

        case 0x3D:
            INST_NAME("CMP AX, Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i32 = F16;
            SLLI(x1, xRAX, 48);
            SRLI(x1, xRAX, 48);
            if(i32) {
                MOV32w(x2, i32);
                emit_cmp16(dyn, ninst, x1, x2, x3, x4, x5, x6);
            } else {
                emit_cmp16_0(dyn, ninst, x1, x3, x4);
            }
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
                case 1: // OR
                    if(opcode==0x81) {INST_NAME("OR Ew, Iw");} else {INST_NAME("OR Ew, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEW(x1, (opcode==0x81)?2:1);
                    if(opcode==0x81) i16 = F16S; else i16 = F8S;
                    MOV64x(x5, i16);
                    emit_or16(dyn, ninst, x1, x5, x2, x4);
                    EWBACK;
                    break;
                default:
                    DEFAULT;
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

        default:
            DEFAULT;
    }
    return addr;
}
