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

#include "arm64_printer.h"
#include "dynarec_arm64_private.h"
#include "dynarec_arm64_helper.h"
#include "dynarec_arm64_functions.h"


uintptr_t dynarec64_66(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
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

    while((opcode==0x2E) || (opcode==0x36) || (opcode==0x26) || (opcode==0x66))   // ignoring CS:, SS:, ES: or multiple 0x66
        opcode = F8;

    while((opcode==0xF2) || (opcode==0xF3)) {
        rep = opcode-0xF1;
        opcode = F8;
    }
    GETREX();

    if(rex.w && !(opcode==0x0f || opcode==0xf0 || opcode==0x64 || opcode==0x65))   // rex.w cancels "66", but not for 66 0f type of prefix
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
            UXTHw(x1, xRAX);
            MOV32w(x2, i32);
            emit_add16(dyn, ninst, x1, x2, x3, x4);
            BFIz(xRAX, x1, 0, 16);
            break;
        case 0x06:
            INST_NAME("PUSH ES");
            LDRH_U12(x1, xEmu, offsetof(x64emu_t, segs[_ES]));
            PUSH1_16(x1);
            break;
        case 0x07:
            INST_NAME("POP ES");
            POP1_16(x1);
            STRH_U12(x1, xEmu, offsetof(x64emu_t, segs[_ES]));
            STRw_U12(xZR, xEmu, offsetof(x64emu_t, segs_serial[_ES]));
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
            emit_or16(dyn, ninst, x1, x2, x4, x3);
            GWBACK;
            break;
        case 0x0D:
            INST_NAME("OR AX, Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i32 = F16;
            UXTHw(x1, xRAX);
            MOV32w(x2, i32);
            emit_or16(dyn, ninst, x1, x2, x3, x4);
            BFIz(xRAX, x1, 0, 16);
            break;

        case 0x0F:
            switch(rep) {
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
            emit_adc16(dyn, ninst, x1, x2, x4, x5);
            EWBACK;
            break;
        case 0x13:
            INST_NAME("ADC Gw, Ew");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x1);
            GETEW(x2, 0);
            emit_adc16(dyn, ninst, x1, x2, x4, x3);
            GWBACK;
            break;
        case 0x15:
            INST_NAME("ADC AX, Iw");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i32 = F16;
            UXTHw(x1, xRAX);
            MOV32w(x2, i32);
            emit_adc16(dyn, ninst, x1, x2, x3, x4);
            BFIz(xRAX, x1, 0, 16);
            break;

        case 0x19:
            INST_NAME("SBB Ew, Gw");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x2);
            GETEW(x1, 0);
            emit_sbb16(dyn, ninst, x1, x2, x4, x5);
            EWBACK;
            break;
        case 0x1B:
            INST_NAME("SBB Gw, Ew");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x1);
            GETEW(x2, 0);
            emit_sbb16(dyn, ninst, x1, x2, x4, x3);
            GWBACK;
            break;
        case 0x1D:
            INST_NAME("SBB AX, Iw");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i16 = F16S;
            UXTHw(x1, xRAX);
            MOVZw(x2, i16);
            emit_sbb16(dyn, ninst, x1, x2, x3, x4);
            BFIz(xRAX, x1, 0, 16);
            break;
        case 0x1E:
            INST_NAME("PUSH DS");
            LDRH_U12(x1, xEmu, offsetof(x64emu_t, segs[_DS]));
            PUSH1_16(x1);
            break;
        case 0x1F:
            INST_NAME("POP DS");
            POP1_16(x1);
            STRH_U12(x1, xEmu, offsetof(x64emu_t, segs[_DS]));
            STRw_U12(xZR, xEmu, offsetof(x64emu_t, segs_serial[_DS]));
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
            UXTHw(x1, xRAX);
            MOV32w(x2, i32);
            emit_and16(dyn, ninst, x1, x2, x3, x4);
            BFIz(xRAX, x1, 0, 16);
            break;

        case 0x29:
            INST_NAME("SUB Ew, Gw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x2);
            GETEW(x1, 0);
            emit_sub16(dyn, ninst, x1, x2, x4, x5);
            EWBACK;
            break;
        case 0x2B:
            INST_NAME("SUB Gw, Ew");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x1);
            GETEW(x2, 0);
            emit_sub16(dyn, ninst, x1, x2, x3, x4);
            GWBACK;
            break;
        case 0x2D:
            INST_NAME("SUB AX, Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i32 = F16;
            UXTHw(x1, xRAX);
            MOV32w(x2, i32);
            emit_sub16(dyn, ninst, x1, x2, x3, x4);
            BFIz(xRAX, x1, 0, 16);
            break;

        case 0x31:
            INST_NAME("XOR Ew, Gw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x2);
            GETEW(x1, 0);
            emit_xor16(dyn, ninst, x1, x2, x4, x5);
            EWBACK;
            break;
        case 0x33:
            INST_NAME("XOR Gw, Ew");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x1);
            GETEW(x2, 0);
            emit_xor16(dyn, ninst, x1, x2, x3, x4);
            GWBACK;
            break;
        case 0x35:
            INST_NAME("XOR AX, Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i32 = F16;
            UXTHw(x1, xRAX);
            MOV32w(x2, i32);
            emit_xor16(dyn, ninst, x1, x2, x3, x4);
            BFIz(xRAX, x1, 0, 16);
            break;

        case 0x39:
            INST_NAME("CMP Ew, Gw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x2);
            GETEW(x1, 0);
            emit_cmp16(dyn, ninst, x1, x2, x3, x4, x5);
            break;
        case 0x3B:
            INST_NAME("CMP Gw, Ew");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x1);
            GETEW(x2, 0);
            emit_cmp16(dyn, ninst, x1, x2, x3, x4, x5);
            break;
        case 0x3D:
            INST_NAME("CMP AX, Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i32 = F16;
            UXTHw(x1, xRAX);
            if(i32) {
                MOV32w(x2, i32);
                emit_cmp16(dyn, ninst, x1, x2, x3, x4, x5);
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
            SETFLAGS(X_ALL&~X_CF, SF_SUBSET_PENDING);
            gd = xRAX + (opcode&7);
            UXTHw(x1, gd);
            emit_inc16(dyn, ninst, x1, x2, x3);
            BFIw(gd, x1, 0, 16);
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
            SETFLAGS(X_ALL&~X_CF, SF_SUBSET_PENDING);
            gd = xRAX + (opcode&7);
            UXTHw(x1, gd);
            emit_dec16(dyn, ninst, x1, x2, x3);
            BFIw(gd, x1, 0, 16);
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
            gd = xRAX+(opcode&0x07)+(rex.b<<3);
            if (gd==xRSP) {
                MOVw_REG(x1, xRSP);
                PUSH1_16(x1);
            } else {
                PUSH1_16(gd);
            }
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
            gd = xRAX+(opcode&0x07)+(rex.b<<3);
            POP1_16(x1);
            BFIw(gd, x1, 0, 16);
            break;
        case 0x60:
            if(rex.is32bits) {
                INST_NAME("PUSHA 16bits (32bits)");
                MOVw_REG(x1, xRSP);
                PUSH1_16(xRAX);
                PUSH1_16(xRCX);
                PUSH1_16(xRDX);
                PUSH1_16(xRBX);
                PUSH1_16(x1);
                PUSH1_16(xRBP);
                PUSH1_16(xRSI);
                PUSH1_16(xRDI);
            } else {
                DEFAULT;
            }
            break;
        case 0x61:
            if(rex.is32bits) {
                INST_NAME("POPA 16bits (32bits)");
                POP1_16(x1);
                BFIw(xRDI, x1, 0, 16);
                POP1_16(x1);
                BFIw(xRSI, x1, 0, 16);
                POP1_16(x1);
                BFIw(xRBP, x1, 0, 16);
                POP1_16(x1); // RSP ignored
                POP1_16(x1);
                BFIw(xRBX, x1, 0, 16);
                POP1_16(x1);
                BFIw(xRDX, x1, 0, 16);
                POP1_16(x1);
                BFIw(xRCX, x1, 0, 16);
                POP1_16(x1);
                BFIw(xRAX, x1, 0, 16);
            } else {
                DEFAULT;
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

        case 0x68:
            INST_NAME("PUSH Iw");
            u16 = F16;
            MOV32w(x2, u16);
            PUSH1_16(x2);
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
            GETSEW(x1, (opcode==0x69)?2:1);
            if(opcode==0x69) i32 = F16S; else i32 = F8S;
            MOV32w(x2, i32);
            MULw(x2, x2, x1);
            UFLAG_RES(x2);
            gd=x2;
            GWBACK;
            UFLAG_DF(x1, d_imul16);
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
                case 0: //ADD
                    if(opcode==0x81) {INST_NAME("ADD Ew, Iw");} else {INST_NAME("ADD Ew, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEW(x1, (opcode==0x81)?2:1);
                    if(opcode==0x81) i16 = F16S; else i16 = F8S;
                    MOVZw(x5, i16);
                    emit_add16(dyn, ninst, ed, x5, x2, x4);
                    EWBACK;
                    break;
                case 1: //OR
                    if(opcode==0x81) {INST_NAME("OR Ew, Iw");} else {INST_NAME("OR Ew, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEW(x1, (opcode==0x81)?2:1);
                    if(opcode==0x81) i16 = F16S; else i16 = F8S;
                    MOVZw(x5, i16);
                    emit_or16(dyn, ninst, x1, x5, x2, x4);
                    EWBACK;
                    break;
                case 2: //ADC
                    if(opcode==0x81) {INST_NAME("ADC Ew, Iw");} else {INST_NAME("ADC Ew, Ib");}
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEW(x1, (opcode==0x81)?2:1);
                    if(opcode==0x81) i16 = F16S; else i16 = F8S;
                    MOVZw(x5, i16);
                    emit_adc16(dyn, ninst, x1, x5, x2, x4);
                    EWBACK;
                    break;
                case 3: //SBB
                    if(opcode==0x81) {INST_NAME("SBB Ew, Iw");} else {INST_NAME("SBB Ew, Ib");}
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEW(x1, (opcode==0x81)?2:1);
                    if(opcode==0x81) i16 = F16S; else i16 = F8S;
                    MOVZw(x5, i16);
                    emit_sbb16(dyn, ninst, x1, x5, x2, x4);
                    EWBACK;
                    break;
                case 4: //AND
                    if(opcode==0x81) {INST_NAME("AND Ew, Iw");} else {INST_NAME("AND Ew, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEW(x1, (opcode==0x81)?2:1);
                    if(opcode==0x81) i16 = F16S; else i16 = F8S;
                    MOVZw(x5, i16);
                    emit_and16(dyn, ninst, x1, x5, x2, x4);
                    EWBACK;
                    break;
                case 5: //SUB
                    if(opcode==0x81) {INST_NAME("SUB Ew, Iw");} else {INST_NAME("SUB Ew, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEW(x1, (opcode==0x81)?2:1);
                    if(opcode==0x81) i16 = F16S; else i16 = F8S;
                    MOVZw(x5, i16);
                    emit_sub16(dyn, ninst, x1, x5, x2, x4);
                    EWBACK;
                    break;
                case 6: //XOR
                    if(opcode==0x81) {INST_NAME("XOR Ew, Iw");} else {INST_NAME("XOR Ew, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEW(x1, (opcode==0x81)?2:1);
                    if(opcode==0x81) i16 = F16S; else i16 = F8S;
                    MOVZw(x5, i16);
                    emit_xor16(dyn, ninst, x1, x5, x2, x4);
                    EWBACK;
                    break;
                case 7: //CMP
                    if(opcode==0x81) {INST_NAME("CMP Ew, Iw");} else {INST_NAME("CMP Ew, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEW(x1, (opcode==0x81)?2:1);
                    if(opcode==0x81) i16 = F16S; else i16 = F8S;
                    if(i16) {
                        MOVZw(x2, i16);
                        emit_cmp16(dyn, ninst, x1, x2, x3, x4, x5);
                    } else
                        emit_cmp16_0(dyn, ninst, x1, x3, x4);
                    break;
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
            if(MODREG) {
                GETGD;
                GETED(0);
                MOVxw_REG(x1, gd);
                BFIz(gd, ed, 0, 16);
                BFIz(ed, x1, 0, 16);
            } else {
                GETGD;
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, 0);
                if(!ALIGNED_ATOMICH) {
                    TSTx_mask(ed, 1, 0, 0);    // mask=1
                    B_MARK(cNE);
                }
                if(arm64_atomics) {
                    SWPALH(gd, x1, ed);
                    SMDMB();
                    if(!ALIGNED_ATOMICH) {
                        B_MARK2_nocond;
                    }
                } else {
                    MARKLOCK;
                    LDAXRH(x1, ed);
                    STLXRH(x3, gd, ed);
                    CBNZx_MARKLOCK(x3);
                    SMDMB();
                    if(!ALIGNED_ATOMICH) {
                        B_MARK2_nocond;
                    }
                }
                if(!ALIGNED_ATOMICH) {
                    MARK;
                    LDRH_U12(x1, ed, 0);
                    LDAXRB(x3, ed);
                    STLXRB(x3, gd, ed);
                    CBNZx_MARK(x3);
                    STRH_U12(gd, ed, 0);
                    SMDMB();
                    MARK2;
                }
                BFIz(gd, x1, 0, 16);
            }
            break;

        case 0x89:
            INST_NAME("MOV Ew, Gw");
            nextop = F8;
            GETGD;  // don't need GETGW here
            if(MODREG) {
                ed = xRAX+(nextop&7)+(rex.b<<3);
                if(ed!=gd) {
                    BFIz(ed, gd, 0, 16);
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff<<1, 1, rex, &lock, 0, 0);
                STH(gd, ed, fixedaddress);
                SMWRITELOCK(lock);
            }
            break;
        case 0x8B:
            INST_NAME("MOV Gw, Ew");
            nextop = F8;
            GETGD;  // don't need GETGW neither
            if(MODREG) {
                ed = xRAX+(nextop&7)+(rex.b<<3);
                if(ed!=gd) {
                    BFIz(gd, ed, 0, 16);
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff<<1, 1, rex, &lock, 0, 0);
                SMREADLOCK(lock);
                LDH(x1, ed, fixedaddress);
                BFIz(gd, x1, 0, 16);
            }
            break;
        case 0x8C:
            INST_NAME("MOV EW, Seg");
            nextop=F8;
            u8 = (nextop&0x38)>>3;
            LDRH_U12(x3, xEmu, offsetof(x64emu_t, segs[u8]));
            if((nextop&0xC0)==0xC0) {   // reg <= seg
                BFIz(xRAX+(nextop&7)+(rex.b<<3), x3, 0, 16);
            } else {                    // mem <= seg
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, &unscaled, 0xfff<<1, 1, rex, NULL, 0, 0);
                STH(x3, wback, fixedaddress);
                SMWRITE2();
            }
            break;

        case 0x8E:
            INST_NAME("MOV Seg,Ew");
            nextop = F8;
            u8 = (nextop&0x38)>>3;
            if((nextop&0xC0)==0xC0) {
                ed = xRAX+(nextop&7)+(rex.b<<3);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, &unscaled, 0xfff<<1, 1, rex, NULL, 0, 0);
                LDH(x1, wback, fixedaddress);
                ed = x1;
            }
            STRH_U12(ed, xEmu, offsetof(x64emu_t, segs[u8]));
            STRw_U12(wZR, xEmu, offsetof(x64emu_t, segs_serial[u8]));
            break;
        case 0x8F:
            INST_NAME("POP Ew");
            nextop = F8;
            POP1_16(x1);
            if((nextop&0xC0)==0xC0) {
                wback = xRAX+(nextop&7)+(rex.b<<3);
                BFIz(wback, x1, 0, 16);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, &unscaled, 0xfff<<1, 1, rex, NULL, 0, 0);
                STH(x1, wback, fixedaddress);
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
                MOVw_REG(x2, xRAX);
                BFIz(xRAX, gd, 0, 16);
                BFIz(gd, x2, 0, 16);
            }
            break;
        case 0x98:
            INST_NAME("CBW");
            SXTBw(x1, xRAX);
            BFIz(xRAX, x1, 0, 16);
            break;
        case 0x99:
            INST_NAME("CWD");
            SXTHw(x1, xRAX);
            BFXILx(xRDX, x1, 16, 16);
            break;
        case 0x9C:
            INST_NAME("PUSHF");
            READFLAGS(X_ALL);
            WILLWRITE();
            PUSH1_16(xFlags);
            SMWRITE();
            break;
        case 0x9D:
            INST_NAME("POPF");
            SETFLAGS(X_ALL, SF_SET);
            SMREAD();
            POP1_16(x1);    // probably not usefull...
            BFIw(xFlags, x1, 0, 16);
            MOV32w(x1, 0x3F7FD7);
            ANDw_REG(xFlags, xFlags, x1);
            ORRw_mask(xFlags, xFlags, 0b011111, 0);   //mask=0x00000002
            SET_DFNONE(x1);
            if(box64_wine) {    // should this be done all the time?
                TBZ_NEXT(xFlags, F_TF);
                // go to epilog, TF should trigger at end of next opcode, so using Interpretor only
                jump_to_epilog(dyn, addr, 0, ninst);
            }
            break;

        case 0xA1:
            INST_NAME("MOV EAX,Od");
            if(rex.is32bits)
                u64 = F32;
            else
                u64 = F64;
            MOV64z(x1, u64);
            if(isLockAddress(u64)) lock=1; else lock = 0;
            SMREADLOCK(lock);
            LDRH_U12(x2, x1, 0);
            BFIz(xRAX, x2, 0, 16);
            break;

        case 0xA3:
            INST_NAME("MOV Od,EAX");
            if(rex.is32bits)
                u64 = F32;
            else
                u64 = F64;
            MOV64z(x1, u64);
            if(isLockAddress(u64)) lock=1; else lock = 0;
            WILLWRITELOCK(lock);
            STRH_U12(xRAX, x1, 0);
            SMWRITELOCK(lock);
            break;

        case 0xA5:
            SMREAD();
            if(rep) {
                INST_NAME("REP MOVSW");
                CBZx_NEXT(xRCX);
                TBNZ_MARK2(xFlags, F_DF);
                MARK;   // Part with DF==0
                LDRH_S9_postindex(x1, xRSI, 2);
                STRH_S9_postindex(x1, xRDI, 2);
                SUBx_U12(xRCX, xRCX, 1);
                CBNZx_MARK(xRCX);
                B_NEXT_nocond;
                MARK2;  // Part with DF==1
                LDRH_S9_postindex(x1, xRSI, -2);
                STRH_S9_postindex(x1, xRDI, -2);
                SUBx_U12(xRCX, xRCX, 1);
                CBNZx_MARK2(xRCX);
                // done
            } else {
                INST_NAME("MOVSW");
                GETDIR(x3, 2);
                LDRH_U12(x1, xRSI, 0);
                STRH_U12(x1, xRDI, 0);
                ADDx_REG(xRSI, xRSI, x3);
                ADDx_REG(xRDI, xRDI, x3);
            }
            SMWRITE();
            break;

        case 0xA7:
            SMREAD();
            switch(rep) {
            case 1:
            case 2:
                if(rep==1) {INST_NAME("REPNZ CMPSW");} else {INST_NAME("REPZ CMPSW");}
                MAYSETFLAGS();
                SETFLAGS(X_ALL, SF_SET_PENDING);
                CBZx_NEXT(xRCX);
                TBNZ_MARK2(xFlags, F_DF);
                MARK;   // Part with DF==0
                LDRH_S9_postindex(x1, xRSI, 2);
                LDRH_S9_postindex(x2, xRDI, 2);
                SUBx_U12(xRCX, xRCX, 1);
                CMPSw_REG(x1, x2);
                B_MARK3((rep==1)?cEQ:cNE);
                CBNZx_MARK(xRCX);
                B_MARK3_nocond;
                MARK2;  // Part with DF==1
                LDRH_S9_postindex(x1, xRSI, -2);
                LDRH_S9_postindex(x2, xRDI, -2);
                SUBx_U12(xRCX, xRCX, 1);
                CMPSw_REG(x1, x2);
                B_MARK3((rep==1)?cEQ:cNE);
                CBNZx_MARK2(xRCX);
                MARK3;  // end
                emit_cmp16(dyn, ninst, x1, x2, x3, x4, x5);
                break;
            default:
                INST_NAME("CMPSW");
                SETFLAGS(X_ALL, SF_SET_PENDING);
                GETDIR(x3, 2);
                LDRH_U12(x1, xRSI, 0);
                LDRH_U12(x2, xRDI, 0);
                ADDx_REG(xRSI, xRSI, x3);
                ADDx_REG(xRDI, xRDI, x3);
                emit_cmp16(dyn, ninst, x1, x2, x3, x4, x5);
                break;
            }
            break;

        case 0xA9:
            INST_NAME("TEST AX,Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            u16 = F16;
            MOV32w(x2, u16);
            UBFXx(x1, xRAX, 0, 16);
            emit_test16(dyn, ninst, x1, x2, x3, x4, x5);
            break;

        case 0xAB:
            WILLWRITE();
            if(rep) {
                INST_NAME("REP STOSW");
                CBZx_NEXT(xRCX);
                TBNZ_MARK2(xFlags, F_DF);
                MARK;   // Part with DF==0
                STRH_S9_postindex(xRAX, xRDI, 2);
                SUBx_U12(xRCX, xRCX, 1);
                CBNZx_MARK(xRCX);
                B_MARK3_nocond;
                MARK2;  // Part with DF==1
                STRH_S9_postindex(xRAX, xRDI, -2);
                SUBx_U12(xRCX, xRCX, 1);
                CBNZx_MARK2(xRCX);
                MARK3;
                // done
            } else {
                INST_NAME("STOSW");
                GETDIR(x3, 2);
                STRH_U12(xRAX, xRDI, 0);
                ADDx_REG(xRDI, xRDI, x3);
            }
            SMWRITE();
            break;

        case 0xAD:
            if(rep) {
                INST_NAME("REP LODSW");
                CBZx_NEXT(xRCX);
                TBNZ_MARK2(xFlags, F_DF);
                MARK;   // Part with DF==0
                LDRH_S9_postindex(x2, xRSI, 2);
                SUBx_U12(xRCX, xRCX, 1);
                CBNZx_MARK(xRCX);
                B_MARK3_nocond;
                MARK2;  // Part with DF==1
                LDRH_S9_postindex(x2, xRSI, -2);
                SUBx_U12(xRCX, xRCX, 1);
                CBNZx_MARK2(xRCX);
                MARK3;
                BFIz(xRAX, x2, 0, 16);
                // done
            } else {
                INST_NAME("LODSW");
                GETDIR(x3, 2);
                LDRH_U12(x2, xRSI, 0);
                ADDx_REG(xRSI, xRSI, x3);
                BFIz(xRAX, x2, 0, 16);
            }
            break;

        case 0xAF:
            switch(rep) {
            case 1:
            case 2:
                if(rep==1) {INST_NAME("REPNZ SCASW");} else {INST_NAME("REPZ SCASW");}
                MAYSETFLAGS();
                SETFLAGS(X_ALL, SF_SET_PENDING);
                CBZx_NEXT(xRCX);
                UXTHw(x1, xRAX);
                TBNZ_MARK2(xFlags, F_DF);
                MARK;   // Part with DF==0
                LDRH_S9_postindex(x2, xRDI, 2);
                SUBx_U12(xRCX, xRCX, 1);
                CMPSw_REG(x1, x2);
                B_MARK3((rep==1)?cEQ:cNE);
                CBNZx_MARK(xRCX);
                B_MARK3_nocond;
                MARK2;  // Part with DF==1
                LDRH_S9_postindex(x2, xRDI, -2);
                SUBx_U12(xRCX, xRCX, 1);
                CMPSw_REG(x1, x2);
                B_MARK3((rep==1)?cEQ:cNE);
                CBNZx_MARK2(xRCX);
                MARK3;  // end
                emit_cmp16(dyn, ninst, x1, x2, x3, x4, x5);
                break;
            default:
                INST_NAME("SCASW");
                SETFLAGS(X_ALL, SF_SET_PENDING);
                GETDIR(x3, 2);
                UXTHw(x1, xRAX);
                LDRH_U12(x2, xRDI, 0);
                ADDx_REG(xRDI, xRDI, x3);
                emit_cmp16(dyn, ninst, x1, x2, x3, x4, x5);
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
            gd = xRAX+(opcode&7)+(rex.b<<3);
            BFIz(gd, x1, 0, 16);
            break;

        case 0xC1:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("ROL Ew, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop) & 15;
                    if (u8) {
                        SETFLAGS(X_CF | X_OF, SF_SUBSET_PENDING);
                        GETEW(x1, 1);
                        u8 = F8;
                        emit_rol16c(dyn, ninst, x1, u8, x4, x5);
                        EWBACK;
                    } else {
                        FAKEED;
                        F8;
                    }
                    break;
                case 1:
                    INST_NAME("ROR Ew, Ib");
                    if (geted_ib(dyn, addr, ninst, nextop) & 15) {
                        SETFLAGS(X_CF | X_OF, SF_SUBSET_PENDING);
                        GETEW(x1, 1);
                        u8 = F8;
                        emit_ror16c(dyn, ninst, x1, u8, x4, x5);
                        EWBACK;
                    } else {
                        FAKEED;
                        F8;
                    }
                    break;
                case 2:
                    INST_NAME("RCL Ew, Ib");
                    if (geted_ib(dyn, addr, ninst, nextop) & 31) {
                        READFLAGS(X_CF);
                        SETFLAGS(X_OF|X_CF, SF_SUBSET); // removed PENDING on purpose
                        GETEW(x1, 1);
                        u8 = F8;
                        emit_rcl16c(dyn, ninst, ed, u8, x4, x5);
                        EWBACK;
                    } else {
                        FAKEED;
                        F8;
                    }
                    break;
                case 3:
                    INST_NAME("RCR Ew, Ib");
                    if (geted_ib(dyn, addr, ninst, nextop) & 31) {
                        READFLAGS(X_CF);
                        SETFLAGS(X_OF|X_CF, SF_SUBSET); // removed PENDING on purpose
                        GETEW(x1, 1);
                        u8 = F8;
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
                        SETFLAGS(X_ALL, SF_SET_PENDING); // some flags are left undefined
                        GETEW(x1, 0);
                        u8 = (F8)&0x1f;
                        emit_shl16c(dyn, ninst, x1, u8, x5, x4);
                        EWBACK;
                    } else {
                        FAKEED;
                        F8;
                    }
                    break;
                case 5:
                    INST_NAME("SHR Ew, Ib");
                    if (geted_ib(dyn, addr, ninst, nextop) & 0x1f) {
                        SETFLAGS(X_ALL, SF_SET_PENDING); // some flags are left undefined
                        GETEW(x1, 0);
                        u8 = (F8)&0x1f;
                        emit_shr16c(dyn, ninst, x1, u8, x5, x4);
                        EWBACK;
                    } else {
                        FAKEED;
                        F8;
                    }
                    break;
                case 7:
                    INST_NAME("SAR Ew, Ib");
                    if (geted_ib(dyn, addr, ninst, nextop) & 0x1f) {
                        SETFLAGS(X_ALL, SF_SET_PENDING); // some flags are left undefined
                        GETSEW(x1, 0);
                        u8 = (F8)&0x1f;
                        emit_sar16c(dyn, ninst, x1, u8, x5, x4);
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
            if(MODREG) {
                ed = xRAX+(nextop&7)+(rex.b<<3);
                u16 = F16;
                MOV32w(x1, u16);
                BFIz(ed, x1, 0, 16);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff<<1, 1, rex, &lock, 0, 2);
                u16 = F16;
                MOV32w(x1, u16);
                STH(x1, ed, fixedaddress);
                SMWRITELOCK(lock);
            }
            break;

        case 0xD1:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("ROL Ew, 1");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET_PENDING);
                    GETEW(x1, 0);
                    emit_rol16c(dyn, ninst, x1, 1, x5, x4);
                    EWBACK;
                    break;
                case 1:
                    INST_NAME("ROR Ew, 1");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET_PENDING);
                    GETEW(x1, 0);
                    emit_ror16c(dyn, ninst, x1, 1, x5, x4);
                    EWBACK;
                    break;
                case 2:
                    INST_NAME("RCL Ew, 1");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SUBSET); // removed PENDING on purpose
                    GETEW(x1, 0);
                    emit_rcl16c(dyn, ninst, x1, 1, x5, x4);
                    EWBACK;
                    break;
                case 3:
                    INST_NAME("RCR Ew, 1");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SUBSET); // removed PENDING on purpose
                    GETEW(x1, 0);
                    emit_rcr16c(dyn, ninst, x1, 1, x5, x4);
                    EWBACK;
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ew, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    GETEW(x1, 0);
                    emit_shl16c(dyn, ninst, x1, 1, x5, x4);
                    EWBACK;
                    break;
                case 5:
                    INST_NAME("SHR Ew, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    GETEW(x1, 0);
                    emit_shr16c(dyn, ninst, x1, 1, x5, x4);
                    EWBACK;
                    break;
                case 7:
                    INST_NAME("SAR Ew, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    GETSEW(x1, 0);
                    emit_sar16c(dyn, ninst, x1, 1, x5, x4);
                    EWBACK;
                    break;
            }
            break;

        case 0xD3:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("ROL Ew, CL");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    if(box64_dynarec_safeflags>1)
                        MAYSETFLAGS();
                    UFLAG_IF {
                        TSTw_mask(xRCX, 0, 0b00100);  //mask=0x00000001f
                        B_NEXT(cEQ);
                    }
                    ANDw_mask(x2, xRCX, 0, 0b00011);  //mask=0x00000000f
                    MOV32w(x4, 16);
                    SUBx_REG(x2, x4, x2);
                    GETEW(x1, 0);
                    ORRw_REG_LSL(ed, ed, ed, 16);
                    LSRw_REG(ed, ed, x2);
                    EWBACK;
                    UFLAG_IF {  // calculate flags directly
                        CMPSw_U12(x2, 15);
                        B_MARK(cNE);
                            ADDxw_REG_LSR(x3, ed, ed, 15);
                            BFIw(xFlags, x3, F_OF, 1);
                        MARK;
                        BFIw(xFlags, ed, F_CF, 1);
                        UFLAG_DF(x2, d_none);
                    }
                    break;
                case 1:
                    INST_NAME("ROR Ew, CL");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    if(box64_dynarec_safeflags>1)
                        MAYSETFLAGS();
                    UFLAG_IF {
                        TSTw_mask(xRCX, 0, 0b00100);  //mask=0x00000001f
                        B_NEXT(cEQ);
                    }
                    ANDw_mask(x2, xRCX, 0, 0b00011);  //mask=0x00000000f
                    GETEW(x1, 0);
                    ORRw_REG_LSL(ed, ed, ed, 16);
                    LSRw_REG(ed, ed, x2);
                    EWBACK;
                    UFLAG_IF {  // calculate flags directly
                        CMPSw_U12(x2, 1);
                        B_MARK(cNE);
                            LSRxw(x2, ed, 14); // x2 = d>>14
                            EORw_REG_LSR(x2, x2, x2, 1); // x2 = ((d>>14) ^ ((d>>14)>>1))
                            BFIw(xFlags, x2, F_OF, 1);
                        MARK;
                        BFXILw(xFlags, ed, 15, 1);
                        UFLAG_DF(x2, d_none);
                    }
                    break;
                case 2:
                    INST_NAME("RCL Ew, CL");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    READFLAGS(X_CF);
                    if(box64_dynarec_safeflags>1)
                        MAYSETFLAGS();
                    SETFLAGS(X_OF|X_CF, SF_SET_DF);
                    ANDw_mask(x2, xRCX, 0, 0b00100);
                    GETEW(x1, 0);
                    CALL_(rcl16, x1, x3);
                    EWBACK;
                    break;
                case 3:
                    INST_NAME("RCR Ew, CL");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    READFLAGS(X_CF);
                    if(box64_dynarec_safeflags>1)
                        MAYSETFLAGS();
                    SETFLAGS(X_OF|X_CF, SF_SET_DF);
                    ANDw_mask(x2, xRCX, 0, 0b00100);
                    GETEW(x1, 0);
                    CALL_(rcr16, x1, x3);
                    EWBACK;
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ew, CL");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    if(box64_dynarec_safeflags>1)
                        MAYSETFLAGS();
                    UFLAG_IF {
                        ANDSw_mask(x2, xRCX, 0, 0b00100);  //mask=0x00000001f
                        B_NEXT(cEQ);
                    } else {
                        ANDw_mask(x2, xRCX, 0, 0b00100);  //mask=0x00000001f
                    }
                    GETEW(x1, 0);
                    emit_shl16(dyn, ninst, x1, x2, x5, x4);
                    EWBACK;
                    break;
                case 5:
                    INST_NAME("SHR Ew, CL");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    if(box64_dynarec_safeflags>1)
                        MAYSETFLAGS();
                    UFLAG_IF {
                        ANDSw_mask(x2, xRCX, 0, 0b00100);  //mask=0x00000001f
                        B_NEXT(cEQ);
                    } else {
                        ANDw_mask(x2, xRCX, 0, 0b00100);  //mask=0x00000001f
                    }
                    GETEW(x1, 0);
                    emit_shr16(dyn, ninst, x1, x2, x5, x4);
                    EWBACK;
                    break;
                case 7:
                    INST_NAME("SAR Ew, CL");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    if(box64_dynarec_safeflags>1)
                        MAYSETFLAGS();
                    UFLAG_IF {
                        ANDSw_mask(x2, xRCX, 0, 0b00100);  //mask=0x00000001f
                        B_NEXT(cEQ);
                    } else {
                        ANDw_mask(x2, xRCX, 0, 0b00100);  //mask=0x00000001f
                    }
                    GETSEW(x1, 0);
                    emit_sar16(dyn, ninst, x1, x2, x5, x4);
                    EWBACK;
                    break;
            }
            break;

        case 0xF0:
            return dynarec64_66F0(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);

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
                case 2:
                    INST_NAME("NOT Ew");
                    GETEW(x1, 0);
                    MVNw_REG(ed, ed);
                    EWBACK;
                    break;
                case 3:
                    INST_NAME("NEG Ew");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEW(x1, 0);
                    emit_neg16(dyn, ninst, ed, x2, x4);
                    EWBACK;
                    break;
                case 4:
                    INST_NAME("MUL AX, Ew");
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETEW(x1, 0);
                    UXTHw(x2, xRAX);
                    MULw(x1, x2, x1);
                    UFLAG_RES(x1);
                    BFIz(xRAX, x1, 0, 16);
                    BFXILx(xRDX, x1, 16, 16);
                    UFLAG_DF(x1, d_mul16);
                    break;
                case 5:
                    INST_NAME("IMUL AX, Ew");
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETSEW(x1, 0);
                    SXTHw(x2, xRAX);
                    MULw(x1, x2, x1);
                    UFLAG_RES(x1);
                    BFIz(xRAX, x1, 0, 16);
                    BFXILx(xRDX, x1, 16, 16);
                    UFLAG_DF(x1, d_imul16);
                    break;
                case 6:
                    INST_NAME("DIV Ew");
                    SETFLAGS(X_ALL, SF_SET);
                    SET_DFNONE(x1);
                    GETEW(x1, 0);
                    UXTHw(x2, xRAX);
                    BFIw(x2, xRDX, 16, 16);
                    if(box64_dynarec_div0) {
                        CBNZw_MARK3(ed);
                        GETIP_(ip);
                        STORE_XEMU_CALL(xRIP);
                        CALL(native_div0, -1);
                        CLEARIP();
                        LOAD_XEMU_CALL(xRIP);
                        jump_to_epilog(dyn, 0, xRIP, ninst);
                        MARK3;
                    }
                    UDIVw(x3, x2, ed);
                    MSUBw(x4, x3, ed, x2);  // x4 = x2 mod ed (i.e. x2 - x3*ed)
                    BFIz(xRAX, x3, 0, 16);
                    BFIz(xRDX, x4, 0, 16);
                    break;
                case 7:
                    INST_NAME("IDIV Ew");
                    SKIPTEST(x1);
                    SETFLAGS(X_ALL, SF_SET);
                    SET_DFNONE(x1);
                    GETSEW(x1, 0);
                    if(box64_dynarec_div0) {
                        CBNZw_MARK3(ed);
                        GETIP_(ip);
                        STORE_XEMU_CALL(xRIP);
                        CALL(native_div0, -1);
                        CLEARIP();
                        LOAD_XEMU_CALL(xRIP);
                        jump_to_epilog(dyn, 0, xRIP, ninst);
                        MARK3;
                    }
                    UXTHw(x2, xRAX);
                    BFIw(x2, xRDX, 16, 16);
                    SDIVw(x3, x2, ed);
                    MSUBw(x4, x3, ed, x2);  // x4 = x2 mod ed (i.e. x2 - x3*ed)
                    BFIz(xRAX, x3, 0, 16);
                    BFIz(xRDX, x4, 0, 16);
                    break;
            }
            break;
        case 0xF8:
            INST_NAME("CLC");
            SETFLAGS(X_CF, SF_SUBSET);
            SET_DFNONE(x1);
            BFCx(xFlags, F_CF, 1);
            break;
        case 0xF9:
            INST_NAME("STC");
            SETFLAGS(X_CF, SF_SUBSET);
            SET_DFNONE(x1);
            ORRx_mask(xFlags, xFlags, 1, 0, 0); // xFlags | 1
            break;

        case 0xFF:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("INC Ew");
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET_PENDING);
                    GETEW(x1, 0);
                    emit_inc16(dyn, ninst, x1, x2, x4);
                    EWBACK;
                    break;
                case 1:
                    INST_NAME("DEC Ew");
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET_PENDING);
                    GETEW(x1, 0);
                    emit_dec16(dyn, ninst, x1, x2, x4);
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
