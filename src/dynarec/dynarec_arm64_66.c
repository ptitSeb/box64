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
#include "dynarec_arm64.h"
#include "dynarec_arm64_private.h"
#include "arm64_printer.h"

#include "dynarec_arm64_helper.h"
#include "dynarec_arm64_functions.h"


uintptr_t dynarec64_66(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
{
    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint32_t u32;
    int32_t i32, j32;
    int16_t i16;
    uint16_t u16;
    uint8_t gd, ed;
    uint8_t wback, wb1;
    int fixedaddress;
    MAYUSE(u16);
    MAYUSE(u8);
    MAYUSE(j32);

    while((opcode==0x2E) || (opcode==0x66))   // ignoring CS: or multiple 0x66
        opcode = F8;

    while((opcode==0xF2) || (opcode==0xF3)) {
        rep = opcode-0xF1;
        opcode = F8;
    }
    // REX prefix before the F0 are ignored
    rex.rex = 0;
    while(opcode>=0x40 && opcode<=0x4f) {
        rex.rex = opcode;
        opcode = F8;
    }

    switch(opcode) {
        case 0x01:
            INST_NAME("ADD Ew, Gw");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGW(x2);
            GETEW(x1, 0);
            emit_add16(dyn, ninst, x1, x2, x4, x5);
            EWBACK;
            break;
        case 0x03:
            INST_NAME("ADD Gw, Ew");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGW(x1);
            GETEW(x2, 0);
            emit_add16(dyn, ninst, x1, x2, x3, x4);
            GWBACK;
            break;
        case 0x05:
            INST_NAME("ADD AX, Iw");
            SETFLAGS(X_ALL, SF_PENDING);
            i32 = F16;
            UXTHw(x1, xRAX);
            MOV32w(x2, i32);
            emit_add16(dyn, ninst, x1, x2, x3, x4);
            BFIx(xRAX, x1, 0, 16);
            break;
                
        case 0x09:
            INST_NAME("OR Ew, Gw");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGW(x2);
            GETEW(x1, 0);
            emit_or16(dyn, ninst, x1, x2, x4, x2);
            EWBACK;
            break;
        case 0x0B:
            INST_NAME("OR Gw, Ew");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGW(x1);
            GETEW(x2, 0);
            emit_or16(dyn, ninst, x1, x2, x4, x3);
            GWBACK;
            break;
        case 0x0D:
            INST_NAME("OR AX, Iw");
            SETFLAGS(X_ALL, SF_SET);
            i32 = F16;
            UXTHw(x1, xRAX);
            MOV32w(x2, i32);
            emit_or16(dyn, ninst, x1, x2, x3, x4);
            BFIx(xRAX, x1, 0, 16);
            break;

        case 0x0F:
            addr = dynarec64_660F(dyn, addr, ip, ninst, rex, ok, need_epilog);
            break;
        case 0x11:
            INST_NAME("ADC Ew, Gw");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGW(x2);
            GETEW(x1, 0);
            emit_adc16(dyn, ninst, x1, x2, x4, x5);
            EWBACK;
            break;
        case 0x13:
            INST_NAME("ADC Gw, Ew");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGW(x1);
            GETEW(x2, 0);
            emit_adc16(dyn, ninst, x1, x2, x4, x3);
            GWBACK;
            break;
        case 0x15:
            INST_NAME("ADC AX, Iw");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET);
            i32 = F16;
            UXTHw(x1, xRAX);
            MOV32w(x2, i32);
            emit_adc16(dyn, ninst, x1, x2, x3, x4);
            BFIx(xRAX, x1, 0, 16);
            break;

        case 0x19:
            INST_NAME("SBB Ew, Gw");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGW(x2);
            GETEW(x1, 0);
            emit_sbb16(dyn, ninst, x1, x2, x4, x5);
            EWBACK;
            break;
        case 0x1B:
            INST_NAME("SBB Gw, Ew");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGW(x1);
            GETEW(x2, 0);
            emit_sbb16(dyn, ninst, x1, x2, x4, x3);
            GWBACK;
            break;
        case 0x1D:
            INST_NAME("SBB AX, Iw");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET);
            i16 = F16;
            UXTHw(x1, xRAX);
            MOV32w(x2, i16);
            emit_sbb16(dyn, ninst, x1, x2, x3, x4);
            BFIx(xRAX, x1, 0, 16);
            break;

        case 0x21:
            INST_NAME("AND Ew, Gw");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGW(x2);
            GETEW(x1, 0);
            emit_and16(dyn, ninst, x1, x2, x4, x5);
            EWBACK;
            break;
        case 0x23:
            INST_NAME("AND Gw, Ew");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGW(x1);
            GETEW(x2, 0);
            emit_and16(dyn, ninst, x1, x2, x3, x4);
            GWBACK;
            break;
        case 0x25:
            INST_NAME("AND AX, Iw");
            SETFLAGS(X_ALL, SF_SET);
            i32 = F16;
            UXTHw(x1, xRAX);
            MOV32w(x2, i32);
            emit_and16(dyn, ninst, x1, x2, x3, x4);
            BFIx(xRAX, x1, 0, 16);
            break;

        case 0x29:
            INST_NAME("SUB Ew, Gw");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGW(x2);
            GETEW(x1, 0);
            emit_sub16(dyn, ninst, x1, x2, x4, x5);
            EWBACK;
            break;
        case 0x2B:
            INST_NAME("SUB Gw, Ew");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGW(x1);
            GETEW(x2, 0);
            emit_sub16(dyn, ninst, x1, x2, x3, x4);
            GWBACK;
            break;
        case 0x2D:
            INST_NAME("SUB AX, Iw");
            SETFLAGS(X_ALL, SF_SET);
            i32 = F16;
            UXTHw(x1, xRAX);
            MOV32w(x2, i32);
            emit_sub16(dyn, ninst, x1, x2, x3, x4);
            BFIx(xRAX, x1, 0, 16);
            break;

        case 0x31:
            INST_NAME("XOR Ew, Gw");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGW(x2);
            GETEW(x1, 0);
            emit_xor16(dyn, ninst, x1, x2, x4, x5);
            EWBACK;
            break;
        case 0x33:
            INST_NAME("XOR Gw, Ew");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGW(x1);
            GETEW(x2, 0);
            emit_xor16(dyn, ninst, x1, x2, x3, x4);
            GWBACK;
            break;
        case 0x35:
            INST_NAME("XOR AX, Iw");
            SETFLAGS(X_ALL, SF_SET);
            i32 = F16;
            UXTHw(x1, xRAX);
            MOV32w(x2, i32);
            emit_xor16(dyn, ninst, x1, x2, x3, x4);
            BFIx(xRAX, x1, 0, 16);
            break;

        case 0x39:
            INST_NAME("CMP Ew, Gw");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGW(x2);
            GETEW(x1, 0);
            emit_cmp16(dyn, ninst, x1, x2, x3, x4, x5);
            break;
        case 0x3B:
            INST_NAME("CMP Gw, Ew");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGW(x1);
            GETEW(x2, 0);
            emit_cmp16(dyn, ninst, x1, x2, x3, x4, x5);
            break;
        case 0x3D:
            INST_NAME("CMP AX, Iw");
            SETFLAGS(X_ALL, SF_SET);
            i32 = F16;
            UXTHw(x1, xRAX);
            if(i32) {
                emit_cmp16_0(dyn, ninst, x1, x3, x4);
            } else {
                MOV32w(x2, i32);
                emit_cmp16(dyn, ninst, x1, x2, x3, x4, x5);
            }
            break;

        case 0xD1:
        case 0xD3:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    if(opcode==0xD1) {
                        INST_NAME("ROL Ew, 1");
                        MOV32w(x2, 1);
                    } else {
                        INST_NAME("ROL Ew, CL");
                        ANDSw_mask(x2, xRCX, 0, 0b00100);
                    }
                    SETFLAGS(X_OF|X_CF, SF_SET);
                    GETEW(x1, 0);
                    CALL_(rol16, x1, x3);
                    EWBACK;
                    break;
                case 1:
                    if(opcode==0xD1) {
                        INST_NAME("ROR Ew, 1");
                        MOV32w(x2, 1);
                    } else {
                        INST_NAME("ROR Ew, CL");
                        ANDSw_mask(x2, xRCX, 0, 0b00100);
                    }
                    SETFLAGS(X_OF|X_CF, SF_SET);
                    GETEW(x1, 0);
                    CALL_(ror16, x1, x3);
                    EWBACK;
                    break;
                case 2:
                    if(opcode==0xD1) {INST_NAME("RCL Ew, 1"); } else { INST_NAME("RCL Ew, CL");}
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET);
                    if(opcode==0xD1) {MOV32w(x2, 1);} else {ANDSw_mask(x2, xRCX, 0, 0b00100);}
                    GETEW(x1, 0);
                    CALL_(rcl16, x1, x3);
                    EWBACK;
                    break;
                case 3:
                    if(opcode==0xD1) {INST_NAME("RCR Ew, 1");} else {INST_NAME("RCR Ew, CL");}
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET);
                    if(opcode==0xD1) {MOV32w(x2, 1);} else {ANDSw_mask(x2, xRCX, 0, 0b00100);}
                    GETEW(x1, 0);
                    CALL_(rcr16, x1, x3);
                    EWBACK;
                    break;
                case 4:
                case 6:
                    if(opcode==0xD1) {
                        INST_NAME("SHL Ew, 1");
                        MOV32w(x4, 1);
                    } else {
                        INST_NAME("SHL Ew, CL");
                        ANDSw_mask(x4, xRCX, 0, 0b00100);
                    }
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETEW(x1, 0);
                    UFLAG_OP12(ed, x4)
                    LSLw_REG(ed, ed, x4);
                    EWBACK;
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, d_shl16);
                    break;
                case 5:
                    if(opcode==0xD1) {
                        INST_NAME("SHR Ew, 1");
                        MOV32w(x4, 1);
                    } else {
                        INST_NAME("SHR Ew, CL");
                        ANDSw_mask(x4, xRCX, 0, 0b00100);
                    }
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETEW(x1, 0);
                    UFLAG_OP12(ed, x4)
                    LSRw_REG(ed, ed, x4);
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
                        ANDSw_mask(x4, xRCX, 0, 0b00100);
                    }
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETSEW(x1, 0);
                    UFLAG_OP12(ed, x4)
                    ASRw_REG(ed, ed, x4);
                    EWBACK;
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, d_sar16);
                    break;
            }
            break;
            
        case 0xF7:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                case 1:
                    INST_NAME("TEST Ew, Iw");
                    SETFLAGS(X_ALL, SF_SET);
                    GETEW(x1, 2);
                    u16 = F16;
                    MOV32w(x2, u16);
                    emit_test16(dyn, ninst, x1, x2, x3, x4, x5);
                    break;
                case 2:
                    INST_NAME("NOT Ew");
                    GETEW(x1, 0);
                    MVNw_REG(ed, ed);
                    EWBACK;
                    break;
                case 3:
                    INST_NAME("NEG Ew");
                    SETFLAGS(X_ALL, SF_SET);
                    GETEW(x1, 0);
                    emit_neg16(dyn, ninst, ed, x2, x4);
                    EWBACK;
                    break;
                case 4:
                    INST_NAME("MUL AX, Ew");
                    SETFLAGS(X_ALL, SF_PENDING);
                    UFLAG_DF(x1, d_mul16);
                    GETEW(x1, 0);
                    UXTHw(x2, xRAX);
                    MULw(x1, x2, x1);
                    UFLAG_RES(x1);
                    BFIx(xRAX, x1, 0, 16);
                    BFXILx(xRDX, x1, 16, 16);
                    break;
                case 5:
                    INST_NAME("IMUL AX, Ew");
                    SETFLAGS(X_ALL, SF_PENDING);
                    UFLAG_DF(x1, d_imul16);
                    GETSEW(x1, 0);
                    SXTHw(x2, xRAX);
                    MULw(x1, x2, x1);
                    UFLAG_RES(x1);
                    BFIx(xRAX, x1, 0, 16);
                    BFXILx(xRDX, x1, 16, 16);
                    break;
                case 6:
                    INST_NAME("DIV Ew");
                    SETFLAGS(X_ALL, SF_SET);
                    GETEW(x1, 0);
                    CALL(div16, -1);
                    break;
                case 7:
                    INST_NAME("IDIV Ew");
                    SETFLAGS(X_ALL, SF_SET);
                    GETEW(x1, 0);
                    CALL(idiv16, -1);
                    break;
            }
            break;
            
        case 0xFF:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("INC Ew");
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET);
                    GETEW(x1, 0);
                    emit_inc16(dyn, ninst, x1, x2, x4);
                    EWBACK;
                    break;
                case 1:
                    INST_NAME("DEC Ew");
                    SETFLAGS(X_ALL&~X_CF, SF_SET);
                    GETEW(x1, 0);
                    emit_dec16(dyn, ninst, x1, x2, x4);
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

