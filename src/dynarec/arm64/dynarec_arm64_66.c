#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "emu/x64run_private.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x87emu_private.h"
#include "x64trace.h"
#include "dynarec_native.h"
#include "custommem.h"

#include "arm64_printer.h"
#include "dynarec_arm64_private.h"
#include "../dynarec_helper.h"
#include "dynarec_arm64_functions.h"


uintptr_t dynarec64_66(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
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

    if(rex.w && !(opcode==0x0f))   // rex.w cancels "66", but not for 66 0f type of prefix
        return dynarec64_00(dyn, addr-1, ip, ninst, rex, ok, need_epilog); // addr-1, to "put back" opcode

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
            i16 = F16;
            UXTHw(x1, xRAX);
            MOV32w(x2, i16);
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
            break;

        case 0x09:
            INST_NAME("OR Ew, Gw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x2);
            GETEW(x1, 0);
            emit_or16(dyn, ninst, x1, x2, x4, x5);
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
            i16 = F16;
            UXTHw(x1, xRAX);
            emit_or16c(dyn, ninst, x1, i16, x3, x4);
            BFIz(xRAX, x1, 0, 16);
            break;

        case 0x0F:
            switch(rex.rep) {
            case 1:
                addr = dynarec64_66F20F(dyn, addr, ip, ninst, rex, ok, need_epilog);
                break;
            case 2:
                addr = dynarec64_66F30F(dyn, addr, ip, ninst, rex, ok, need_epilog);
                break;
            default:
                addr = dynarec64_660F(dyn, addr, ip, ninst, rex, ok, need_epilog);
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
            u16 = F16;
            UXTHw(x1, xRAX);
            MOV32w(x2, u16);
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
            i16 = F16;
            UXTHw(x1, xRAX);
            emit_and16c(dyn, ninst, x1, i16, x3, x4);
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
            i16 = F16;
            UXTHw(x1, xRAX);
            MOV32w(x2, i16);
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
            i16 = F16;
            UXTHw(x1, xRAX);
            emit_xor16c(dyn, ninst, x1, i16, x3, x4);
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
            u16 = F16;
            UXTHw(x1, xRAX);
            if(u16) {
                MOV32w(x2, u16);
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
            SETFLAGS(X_ALL&~X_CF, SF_SUBSET);
            gd = TO_NAT (opcode&7);
            UXTHw(x1, gd);
            emit_inc16(dyn, ninst, x1, x2, x3);
            BFIz(gd, x1, 0, 16);
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
            SETFLAGS(X_ALL&~X_CF, SF_SUBSET);
            gd = TO_NAT (opcode&7);
            UXTHw(x1, gd);
            emit_dec16(dyn, ninst, x1, x2, x3);
            BFIz(gd, x1, 0, 16);
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
            gd = TO_NAT((opcode & 0x07) + (rex.b << 3));
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
            gd = TO_NAT((opcode & 0x07) + (rex.b << 3));
            POP1_16(x1);
            BFIz(gd, x1, 0, 16);
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
            } else
                return dynarec64_00(dyn, addr-1, ip, ninst, rex, ok, need_epilog);
            break;
        case 0x61:
            if(rex.is32bits) {
                INST_NAME("POPA 16bits (32bits)");
                POP1_16(x1);
                BFIz(xRDI, x1, 0, 16);
                POP1_16(x1);
                BFIz(xRSI, x1, 0, 16);
                POP1_16(x1);
                BFIz(xRBP, x1, 0, 16);
                POP1_16(x1); // RSP ignored
                POP1_16(x1);
                BFIz(xRBX, x1, 0, 16);
                POP1_16(x1);
                BFIz(xRDX, x1, 0, 16);
                POP1_16(x1);
                BFIz(xRCX, x1, 0, 16);
                POP1_16(x1);
                BFIz(xRAX, x1, 0, 16);
            } else 
                return dynarec64_00(dyn, addr-1, ip, ninst, rex, ok, need_epilog);
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
            if (BOX64DRENV(dynarec_safeflags) > 1 && BOX64ENV(cputype)) {
                SETFLAGS(X_OF|X_CF, SF_SET);
            } else {
                SETFLAGS(X_ALL, SF_SET);
            }
            nextop = F8;
            GETSEW(x1, (opcode==0x69)?2:1);
            if(opcode==0x69) i32 = F16S; else i32 = F8S;
            MOV32w(x2, i32);
            MULw(x2, x2, x1);
            gd=x2;
            GWBACK;
            SET_DFNONE();
            IFX(X_CF|X_OF) {
                ASRw(x1, x2, 15);
                CMPSw_REG_ASR(x1, x2, 31);
                CSETw(x3, cNE);
                IFX(X_CF) {
                    BFIw(xFlags, x3, F_CF, 1);
                }
                IFX(X_OF) {
                    BFIw(xFlags, x3, F_OF, 1);
                }
            }
            IFX2(X_AF, && !BOX64ENV(cputype)) {BFCw(xFlags, F_AF, 1);}
            IFX2(X_ZF, && !BOX64ENV(cputype)) {BFCw(xFlags, F_ZF, 1);}
            IFX2(X_SF, && !BOX64ENV(cputype)) {
                LSRw(x3, x2, 15);
                BFIw(xFlags, x3, F_SF, 1);
            }
            IFX2(X_PF, && !BOX64ENV(cputype)) emit_pf(dyn, ninst, x2, x3);
            break;
        case 0x6A:
            INST_NAME("PUSH Ib");
            i16 = F8S;
            MOV32w(x2, (uint16_t)i16);
            PUSH1_16(x2);
            break;
        case 0x6C:
        case 0x6E:
            return dynarec64_00(dyn, addr-1, ip, ninst, rex, ok, need_epilog);
        case 0x6D:
            INST_NAME("INSW");
            if(BOX64DRENV(dynarec_safeflags)>1) {
                READFLAGS(X_PEND);
            } else {
                SETFLAGS(X_ALL, SF_SET_NODF);    // Hack to set flags in "don't care" state
            }
            GETIP(ip);
            STORE_XEMU_CALL(xRIP);
            CALL_S(const_native_priv, -1);
            LOAD_XEMU_CALL(xRIP);
            jump_to_epilog(dyn, 0, xRIP, ninst);
            *need_epilog = 0;
            *ok = 0;
            break;
        case 0x6F:
            INST_NAME("OUTSW");
            if(BOX64DRENV(dynarec_safeflags)>1) {
                READFLAGS(X_PEND);
            } else {
                SETFLAGS(X_ALL, SF_SET_NODF);    // Hack to set flags in "don't care" state
            }
            GETIP(ip);
            STORE_XEMU_CALL(xRIP);
            CALL_S(const_native_priv, -1);
            LOAD_XEMU_CALL(xRIP);
            jump_to_epilog(dyn, 0, xRIP, ninst);
            *need_epilog = 0;
            *ok = 0;
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
            return dynarec64_00(dyn, addr-1, ip, ninst, rex, ok, need_epilog);

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
                    emit_or16c(dyn, ninst, x1, i16, x2, x4);
                    EWBACK;
                    break;
                case 2: //ADC
                    if(opcode==0x81) {INST_NAME("ADC Ew, Iw");} else {INST_NAME("ADC Ew, Ib");}
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEW(x1, (opcode==0x81)?2:1);
                    if(opcode==0x81) i16 = F16S; else i16 = F8S;
                    MOVZw(x5, (uint16_t)i16);
                    emit_adc16(dyn, ninst, x1, x5, x2, x4);
                    EWBACK;
                    break;
                case 3: //SBB
                    if(opcode==0x81) {INST_NAME("SBB Ew, Iw");} else {INST_NAME("SBB Ew, Ib");}
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEW(x1, (opcode==0x81)?2:1);
                    if(opcode==0x81) i16 = F16S; else i16 = F8S;
                    MOVZw(x5, (uint16_t)i16);
                    emit_sbb16(dyn, ninst, x1, x5, x2, x4);
                    EWBACK;
                    break;
                case 4: //AND
                    if(opcode==0x81) {INST_NAME("AND Ew, Iw");} else {INST_NAME("AND Ew, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEW(x1, (opcode==0x81)?2:1);
                    if(opcode==0x81) i16 = F16S; else i16 = F8S;
                    emit_and16c(dyn, ninst, x1, i16, x2, x4);
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
                    emit_xor16c(dyn, ninst, x1, i16, x2, x4);
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
                    TBNZ_MARK(ed, 0);
                }
                if(cpuext.atomics) {
                    SWPALH(gd, x1, ed);
                    if(!ALIGNED_ATOMICH) {
                        B_MARK2_nocond;
                    }
                } else {
                    MARKLOCK;
                    LDAXRH(x1, ed);
                    STLXRH(x3, gd, ed);
                    CBNZx_MARKLOCK(x3);
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
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
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
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
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
            if (MODREG) {
                BFIz(TO_NAT((nextop & 7) + (rex.b << 3)), x3, 0, 16);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, &unscaled, 0xfff<<1, 1, rex, NULL, 0, 0);
                STH(x3, wback, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0x8D:
            INST_NAME("LEA Gd, Ed");
            nextop=F8;
            GETGD;
            if(MODREG) {   // reg <= reg? that's an invalid operation
                DEFAULT;
            } else {                    // mem <= reg
                rex.seg = 0;
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                BFIx(gd, ed, 0, 16);
            }
            break;
        case 0x8E:
            INST_NAME("MOV Seg,Ew");
            nextop = F8;
            u8 = (nextop&0x38)>>3;
            if((u8>5) || (u8==1)) {
                INST_NAME("Invalid MOV Seg,Ew");
                UDF(0);
                *need_epilog = 1;
                *ok = 0;
            } else {
                if (MODREG) {
                    ed = TO_NAT((nextop & 7) + (rex.b << 3));
                } else {
                    SMREAD();
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, &unscaled, 0xfff<<1, 1, rex, NULL, 0, 0);
                    LDH(x1, wback, fixedaddress);
                    ed = x1;
                }
                STRH_U12(ed, xEmu, offsetof(x64emu_t, segs[u8]));
                if((u8==_FS) || (u8==_GS)) {
                    // refresh offset if needed
                    CBZw_NEXT(ed);
                    MOV32w(x1, u8);
                    CALL(const_getsegmentbase, -1);
                }
            }
            break;
        case 0x8F:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("POP Ew");
                    POP1_16(x1);
                    if (MODREG) {
                        wback = TO_NAT((nextop & 7) + (rex.b << 3));
                        BFIz(wback, x1, 0, 16);
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, &unscaled, 0xfff<<1, 1, rex, NULL, 0, 0);
                        STH(x1, wback, fixedaddress);
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
            SXTHw(x1, xRAX);    // sign extend ax to x1
            BFXILx(xRDX, x1, 16, 16);   // insert high 16bits of x1 as low DX
            break;
        case 0x9C:
            INST_NAME("PUSHF");
            READFLAGS(X_ALL);
            PUSH1_16(xFlags);
            SMWRITE();
            break;
        case 0x9D:
            INST_NAME("POPF");
            SETFLAGS(X_ALL, SF_SET);
            SMREAD();
            POP1_16(x1);
            MOV32w(x2, 0x7FD7);
            ANDw_REG(x1, x1, x2);
            ORRw_mask(x1, x1, 0b011111, 0);   //mask=0x00000002
            BFIw(xFlags, x1, 0, 16);
            SET_DFNONE();
            if(box64_wine) {    // should this be done all the time?
                TBZ_NEXT(xFlags, F_TF);
                // go to epilog, TF should trigger at end of next opcode, so using Interpreter only
                jump_to_epilog(dyn, addr, 0, ninst);
            }
            break;

        case 0xA1:
            INST_NAME("MOV AX,Od");
            if(rex.is32bits && rex.is67)
                u64 = F16S;
            else if(rex.is32bits || rex.is67)
                u64 = F32S;
            else
                u64 = F64;
            unscaled = 0; fixedaddress = 0;
            if(rex.seg && (u64<0x1000 || (int64_t)u64>-0x1000 || (u64<(0x1000<<1) && !(u64&1)))) {
                grab_segdata(dyn, addr, ninst, x1, rex.seg, 0);
                if(u64) {
                    if(u64<0x100)
                        {fixedaddress = u64; unscaled = 1;}
                    else if(u64<(0x1000<<1) && !(u64&1))
                        fixedaddress = u64;
                    else if(u64<0x1000)
                        ADDx_U12(x1, x1, u64);
                    else if((int64_t)u64 > -0x100)
                        {fixedaddress = (int64_t)u64; unscaled = 1;}
                    else
                        SUBx_U12(x1, x1, -(int64_t)u64);
                }
            } else {
                MOV64y(x1, u64);
                if(rex.seg) {
                    grab_segdata(dyn, addr, ninst, x3, rex.seg, 0);
                    ADDx_REGy(x1, x3, x1);
                }
            }
            lock=(rex.seg)?0:isLockAddress(u64);
            SMREADLOCK(lock);
            LDH(x2, x1, fixedaddress);
            BFIz(xRAX, x2, 0, 16);
            break;

        case 0xA3:
            INST_NAME("MOV Od,AX");
            if(rex.is32bits && rex.is67)
                u64 = F16S;
            else if(rex.is32bits || rex.is67)
                u64 = F32S;
            else
                u64 = F64;
            unscaled = 0; fixedaddress = 0;
            if(rex.seg && (u64<0x1000 || (int64_t)u64>-0x1000 || (u64<(0x1000<<1) && !(u64&1)))) {
                grab_segdata(dyn, addr, ninst, x1, rex.seg, 0);
                if(u64) {
                    if(u64<0x100)
                        {fixedaddress = u64; unscaled = 1;}
                    else if(u64<(0x1000<<1) && !(u64&1))
                        fixedaddress = u64;
                    else if(u64<0x1000)
                        ADDx_U12(x1, x1, u64);
                    else if((int64_t)u64 > -0x100)
                        {fixedaddress = (int64_t)u64; unscaled = 1;}
                    else
                        SUBx_U12(x1, x1, -(int64_t)u64);
                }
            } else {
                MOV64y(x1, u64);
                if(rex.seg) {
                    grab_segdata(dyn, addr, ninst, x3, rex.seg, 0);
                    ADDx_REGy(x1, x3, x1);
                }
            }
            lock=(rex.seg)?0:isLockAddress(u64);
            SMREADLOCK(lock);
            STH(xRAX, x1, fixedaddress);
            SMWRITELOCK(lock);
            break;
        case 0xA4:
            return dynarec64_00(dyn, addr-1, ip, ninst, rex, ok, need_epilog);
        case 0xA5:
            SMREAD();
            if(rex.rep) {
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
            switch(rex.rep) {
            case 1:
            case 2:
                if(rex.rep==1) {INST_NAME("REPNZ CMPSW");} else {INST_NAME("REPZ CMPSW");}
                if(BOX64DRENV(dynarec_safeflags)>1) {
                    READFLAGS(X_ALL);
                    SETFLAGS(X_ALL, SF_SET);
                } else
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                CBZx_NEXT(xRCX);
                TBNZ_MARK2(xFlags, F_DF);
                MARK;   // Part with DF==0
                LDRH_S9_postindex(x1, xRSI, 2);
                LDRH_S9_postindex(x2, xRDI, 2);
                SUBx_U12(xRCX, xRCX, 1);
                CMPSw_REG(x1, x2);
                B_MARK3((rex.rep==1)?cEQ:cNE);
                CBNZx_MARK(xRCX);
                B_MARK3_nocond;
                MARK2;  // Part with DF==1
                LDRH_S9_postindex(x1, xRSI, -2);
                LDRH_S9_postindex(x2, xRDI, -2);
                SUBx_U12(xRCX, xRCX, 1);
                CMPSw_REG(x1, x2);
                B_MARK3((rex.rep==1)?cEQ:cNE);
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
            UBFXx(x1, xRAX, 0, 16);
            emit_test16c(dyn, ninst, x1, u16, x3, x4, x5);
            break;

        case 0xAB:
            if(rex.rep) {
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
            if(rex.rep) {
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
            switch(rex.rep) {
            case 1:
            case 2:
                if(rex.rep==1) {INST_NAME("REPNZ SCASW");} else {INST_NAME("REPZ SCASW");}
                if(BOX64DRENV(dynarec_safeflags)>1) {
                    READFLAGS(X_ALL);
                    SETFLAGS(X_ALL, SF_SET);
                } else
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                CBZx_NEXT(xRCX);
                UXTHw(x1, xRAX);
                TBNZ_MARK2(xFlags, F_DF);
                MARK;   // Part with DF==0
                LDRH_S9_postindex(x2, xRDI, 2);
                SUBx_U12(xRCX, xRCX, 1);
                CMPSw_REG(x1, x2);
                B_MARK3((rex.rep==1)?cEQ:cNE);
                CBNZx_MARK(xRCX);
                B_MARK3_nocond;
                MARK2;  // Part with DF==1
                LDRH_S9_postindex(x2, xRDI, -2);
                SUBx_U12(xRCX, xRCX, 1);
                CMPSw_REG(x1, x2);
                B_MARK3((rex.rep==1)?cEQ:cNE);
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
            gd = TO_NAT((opcode & 7) + (rex.b << 3));
            if(u16) {
                MOV32w(x1, u16);
                BFIz(gd, x1, 0, 16);
            } else
                BFCz(gd, 0, 16);
            break;

        case 0xC1:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("ROL Ew, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop) & 0x1f;
                    if (u8) {
                        SETFLAGS(X_OF|X_CF, SF_SUBSET); // removed PENDING on purpose
                        GETEW(x1, 1);
                        u8 = (F8)&0x1f;
                        emit_rol16c(dyn, ninst, x1, u8, x4, x5);
                        EWBACK;
                    } else {
                        FAKEED;
                        F8;
                    }
                    break;
                case 1:
                    INST_NAME("ROR Ew, Ib");
                    if (geted_ib(dyn, addr, ninst, nextop) & 0x1f) {
                        SETFLAGS(X_OF|X_CF, SF_SUBSET); // removed PENDING on purpose
                        GETEW(x1, 1);
                        u8 = (F8)&0x1f;
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
                        SETFLAGS(X_OF|X_CF, SF_SUBSET); // removed PENDING on purpose
                        GETEW(x1, 1);
                        u8 = (F8)&0x1f;
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
                        SETFLAGS(X_OF|X_CF, SF_SUBSET); // removed PENDING on purpose
                        GETEW(x1, 1);
                        u8 = (F8)&0x1f;
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
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("MOV Ew, Iw");
                    if(MODREG) {
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        u16 = F16;
                        if(u16) {
                            MOV32w(x1, u16);
                            BFIz(ed, x1, 0, 16);
                        } else
                            BFCz(ed, 0, 16);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff<<1, 1, rex, &lock, 0, 2);
                        u16 = F16;
                        if(u16) {
                            MOV32w(x1, u16);
                            STH(x1, ed, fixedaddress);
                        } else {
                            STH(xZR, ed, fixedaddress);
                        }
                        SMWRITELOCK(lock);
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;

        case 0xD1:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("ROL Ew, 1");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET); // removed PENDING on purpose
                    GETEW(x1, 0);
                    emit_rol16c(dyn, ninst, x1, 1, x5, x4);
                    EWBACK;
                    break;
                case 1:
                    INST_NAME("ROR Ew, 1");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET); // removed PENDING on purpose
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
                    if(BOX64DRENV(dynarec_safeflags)>1) {
                        READFLAGS(X_OF|X_CF);
                    }
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    UFLAG_IF {
                        ANDw_mask(x2, xRCX, 0, 0b00100);  //mask=0x00000001f
                        CBZw_NEXT(x2);
                    }
                    ANDw_mask(x2, xRCX, 0, 0b00011);  //mask=0x00000000f
                    MOV32w(x4, 16);
                    SUBx_REG(x2, x4, x2);
                    GETEW(x1, 0);
                    IFX2(X_OF, && !BOX64ENV(cputype)) {
                        LSRw(x4, ed, 14);
                        EORw_REG_LSR(x4, x4, x4, 1);
                        BFIw(xFlags, x4, F_OF, 1);
                    }
                    ORRw_REG_LSL(ed, ed, ed, 16);
                    LSRw_REG(ed, ed, x2);
                    EWBACK;
                    IFX2(X_OF, && BOX64ENV(cputype)) {
                        EORxw_REG_LSR(x3, ed, ed, 15);
                        BFIw(xFlags, x3, F_OF, 1);
                    }
                    IFX(X_CF) {
                        BFIw(xFlags, ed, F_CF, 1);
                    }
                    break;
                case 1:
                    INST_NAME("ROR Ew, CL");
                    if(BOX64DRENV(dynarec_safeflags)>1) {
                        READFLAGS(X_OF|X_CF);
                    }
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    UFLAG_IF {
                        ANDw_mask(x2, xRCX, 0, 0b00100);  //mask=0x00000001f
                        CBZw_NEXT(x2);
                    }
                    ANDw_mask(x2, xRCX, 0, 0b00011);  //mask=0x00000000f
                    GETEW(x1, 0);
                    IFX2(X_OF, && !BOX64ENV(cputype)) {
                        EORw_REG_LSR(x4, ed, ed, 15);
                        BFIw(xFlags, x4, F_OF, 1);
                    }
                    ORRw_REG_LSL(ed, ed, ed, 16);
                    LSRw_REG(ed, ed, x2);
                    EWBACK;
                    IFX2(X_OF, && BOX64ENV(cputype)) {
                        LSRxw(x2, ed, 14); // x2 = d>>6
                        EORw_REG_LSR(x2, x2, x2, 1); // x2 = ((d>>14) ^ ((d>>14)>>1))
                        BFIw(xFlags, x2, F_OF, 1);
                    }
                    IFX(X_CF) {
                        BFXILw(xFlags, ed, 15, 1);
                    }
                    break;
                case 2:
                    INST_NAME("RCL Ew, CL");
                    if(BOX64DRENV(dynarec_safeflags)>1) {
                        READFLAGS(X_OF|X_CF);
                    } else {
                        READFLAGS(X_CF);
                    }
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    ANDw_mask(x2, xRCX, 0, 0b00100);  //mask=0x00000001f
                    CBZw_NEXT(x2);
                    // get CL % 17
                    MOV32w(x3, 0xf10); // 0x10000 / 17 + 1 (this is precise enough in the 0..31 range)
                    MULw(x3, x3, x2);
                    LSRw(x3, x3, 16);   // x3 = CL / 17
                    MOV32w(x4, 17);
                    MSUBw(x2, x3, x4, x2);  // CL mod 17
                    GETEW(x1, 0);
                    CBZw_MARK(x2);
                    IFX2(X_OF, && !BOX64ENV(cputype)) {
                        LSRw(x5, ed, 14);
                        EORw_REG_LSR(x5, x5, x5, 1);
                        BFIw(xFlags, x5, F_OF, 1);
                    }
                    BFIw(ed, xFlags, 16, 1); // insert CF
                    ORRx_REG_LSL(ed, ed, ed, 17);    // insert rest of ed
                    SUBw_REG(x2, x4, x2);
                    IFX(X_OF|X_CF) {
                        SUBw_U12(x5, x2, 1);
                        LSRx_REG(x5, ed, x5);   // keep the new CF in x5
                    }
                    LSRx_REG(ed, ed, x2);
                    EWBACK;
                    u8 = X_CF;
                    if(BOX64ENV(cputype)) u8 |= X_OF;
                    IFX(u8) {
                        BFXILw(xFlags, x5, 0, 1);
                    }
                    MARK;
                    IFX2(X_OF, && BOX64ENV(cputype)) {
                        EORw_REG_LSR(x2, xFlags, ed, 7);
                        BFIw(xFlags, x2, F_OF, 1);
                    }
                    break;
                case 3:
                    INST_NAME("RCR Ew, CL");
                    MESSAGE(LOG_DUMP, "Need Optimization (RCR Ew, CL)\n");
                    if(BOX64DRENV(dynarec_safeflags)>1) {
                        READFLAGS(X_OF|X_CF);
                    } else {
                        READFLAGS(X_CF);
                    }
                    SETFLAGS(X_OF|X_CF, SF_SET_DF);
                    ANDw_mask(x2, xRCX, 0, 0b00100);
                    CBZw_NEXT(x2);
                    GETEW(x1, 0);
                    CALL_(const_rcr16, x1, x3);
                    EWBACK;
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ew, CL");
                    if(BOX64DRENV(dynarec_safeflags)>1) {
                        READFLAGS(X_ALL);
                        SETFLAGS(X_ALL, SF_SET);
                    } else
                        SETFLAGS(X_ALL, SF_SET_PENDING);
                    ANDw_mask(x2, xRCX, 0, 0b00100);  //mask=0x00000001f
                    UFLAG_IF {
                        CBZw_NEXT(x2);
                    }
                    GETEW(x1, 0);
                    emit_shl16(dyn, ninst, x1, x2, x5, x4);
                    EWBACK;
                    break;
                case 5:
                    INST_NAME("SHR Ew, CL");
                    if(BOX64DRENV(dynarec_safeflags)>1) {
                        READFLAGS(X_ALL);
                        SETFLAGS(X_ALL, SF_SET);
                    } else
                        SETFLAGS(X_ALL, SF_SET_PENDING);
                    ANDw_mask(x2, xRCX, 0, 0b00100);  //mask=0x00000001f
                    UFLAG_IF {
                        CBZw_NEXT(x2);
                    }
                    GETEW(x1, 0);
                    emit_shr16(dyn, ninst, x1, x2, x5, x4);
                    EWBACK;
                    break;
                case 7:
                    INST_NAME("SAR Ew, CL");
                    if(BOX64DRENV(dynarec_safeflags)>1) {
                        READFLAGS(X_ALL);
                        SETFLAGS(X_ALL, SF_SET);
                    } else
                        SETFLAGS(X_ALL, SF_SET_PENDING);
                    ANDw_mask(x2, xRCX, 0, 0b00100);  //mask=0x00000001f
                    UFLAG_IF {
                        CBZw_NEXT(x2);
                    } else {
                    }
                    GETSEW(x1, 0);
                    emit_sar16(dyn, ninst, x1, x2, x5, x4);
                    EWBACK;
                    break;
            }
            break;

        case 0xD9:
            nextop = F8;
            if(MODREG) {
                DEFAULT;
            } else
                switch((nextop>>3)&7) {
                    case 6:
                        INST_NAME("FNSTENV Ed");
                        MESSAGE(LOG_DUMP, "Need Optimization (FNSTENV16)\n");
                        BARRIER(BARRIER_FLOAT); // maybe only x87, not SSE?
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                        if(ed!=x1) {MOVx_REG(x1, ed);}
                        MOV32w(x2, 1);
                        CALL(const_fpu_savenv, -1);
                        break;
                    default:
                        DEFAULT;
            }
            break;

        case 0xDD:
            nextop = F8;
            if(MODREG) {
                DEFAULT;
            } else
                switch((nextop>>3)&7) {
                    case 4:
                        INST_NAME("FRSTOR Ed");
                        MESSAGE(LOG_DUMP, "Need Optimization (FRSTOR16)\n");
                        BARRIER(BARRIER_FLOAT);
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                        if(ed!=x1) {MOVx_REG(x1, ed);}
                        CALL(const_native_frstor16, -1);
                        break;
                    case 6:
                        INST_NAME("FNSAVE Ed");
                        MESSAGE(LOG_DUMP, "Need Optimization (FNSAVE16)\n");
                        BARRIER(BARRIER_FLOAT);
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                        if(ed!=x1) {MOVx_REG(x1, ed);}
                        CALL(const_native_fsave16, -1);
                        break;
                    default:
                        DEFAULT;
            }
            break;

        case 0xE4:                      /* IN AL, Ib */
        case 0xE5:                      /* IN AX, Ib */
        case 0xE6:                      /* OUT Ib, AL */
        case 0xE7:                      /* OUT Ib, AX */
            return dynarec64_00(dyn, addr-1, ip, ninst, rex, ok, need_epilog);

        case 0xEC:                      /* IN AL, DX */
        case 0xED:                      /* IN AX, DX */
        case 0xEE:                      /* OUT DX, AL */
        case 0xEF:                      /* OUT DX, AX */
            return dynarec64_00(dyn, addr-1, ip, ninst, rex, ok, need_epilog);

        case 0xF7:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                case 1:
                    INST_NAME("TEST Ew, Iw");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEW(x1, 2);
                    u16 = F16;
                    emit_test16c(dyn, ninst, x1, u16, x3, x4, x5);
                    break;
                case 2:
                    INST_NAME("NOT Ew");
                    if(MODREG) {
                        CALCEW();
                        int mask = convert_bitmask_x(0xffff);
                        EORx_mask(wback, wback, (mask>>12)&1, mask&0x3F, (mask>>6)&0x3F);
                    } else {
                        GETEW(x1, 0);
                        MVNw_REG(ed, ed);
                        EWBACK;
                    }
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
                    if (BOX64DRENV(dynarec_safeflags) && BOX64ENV(cputype)) {
                        SETFLAGS(X_OF|X_CF, SF_SET);
                    } else {
                        SETFLAGS(X_ALL, SF_SET);
                    }
                    GETEW(x1, 0);
                    UXTHw(x2, xRAX);
                    MULw(x1, x2, x1);
                    BFIz(xRAX, x1, 0, 16);
                    BFXILx(xRDX, x1, 16, 16);
                    SET_DFNONE();
                    IFX(X_CF|X_OF) {
                        CMPSw_REG_LSR(xZR, x1, 16);
                        CSETw(x3, cNE);
                        IFX(X_CF) {
                            BFIw(xFlags, x3, F_CF, 1);
                        }
                        IFX(X_OF) {
                            BFIw(xFlags, x3, F_OF, 1);
                        }
                    }
                    IFX2(X_AF, && !BOX64ENV(cputype)) {BFCw(xFlags, F_AF, 1);}
                    IFX2(X_ZF, && !BOX64ENV(cputype)) {BFCw(xFlags, F_ZF, 1);}
                    IFX2(X_SF, && !BOX64ENV(cputype)) {
                        LSRxw(x3, xRAX, 15);
                        BFIw(xFlags, x3, F_SF, 1);
                    }
                    IFX2(X_PF, && !BOX64ENV(cputype)) emit_pf(dyn, ninst, xRAX, x3);
                    break;
                case 5:
                    INST_NAME("IMUL AX, Ew");
                    if (BOX64DRENV(dynarec_safeflags) && BOX64ENV(cputype)) {
                        SETFLAGS(X_OF|X_CF, SF_SET);
                    } else {
                        SETFLAGS(X_ALL, SF_SET);
                    }
                    GETSEW(x1, 0);
                    SXTHw(x2, xRAX);
                    MULw(x1, x2, x1);
                    BFIz(xRAX, x1, 0, 16);
                    BFXILx(xRDX, x1, 16, 16);
                    SET_DFNONE();
                    IFX(X_CF|X_OF) {
                        ASRw(x2, x1, 15);
                        CMPSw_REG_ASR(x2, x1, 31);
                        CSETw(x3, cNE);
                        IFX(X_CF) {
                            BFIw(xFlags, x3, F_CF, 1);
                        }
                        IFX(X_OF) {
                            BFIw(xFlags, x3, F_OF, 1);
                        }
                    }
                    IFX2(X_AF, && !BOX64ENV(cputype)) {BFCw(xFlags, F_AF, 1);}
                    IFX2(X_ZF, && !BOX64ENV(cputype)) {BFCw(xFlags, F_ZF, 1);}
                    IFX2(X_SF, && !BOX64ENV(cputype)) {
                        LSRxw(x3, xRAX, 15);
                        BFIw(xFlags, x3, F_SF, 1);
                    }
                    IFX2(X_PF, && !BOX64ENV(cputype)) emit_pf(dyn, ninst, xRAX, x3);
                    break;
                case 6:
                    INST_NAME("DIV Ew");
                    SETFLAGS(X_ALL, SF_SET);
                    GETEW(x1, 0);
                    UXTHw(x2, xRAX);
                    BFIw(x2, xRDX, 16, 16);
                    if(BOX64ENV(dynarec_div0)) {
                        CBNZw_MARK3(ed);
                        GETIP_(ip);
                        STORE_XEMU_CALL(xRIP);
                        CALL_S(const_native_div0, -1);
                        CLEARIP();
                        LOAD_XEMU_CALL(xRIP);
                        jump_to_epilog(dyn, 0, xRIP, ninst);
                        MARK3;
                    }
                    UDIVw(x3, x2, ed);
                    MSUBw(x4, x3, ed, x2);  // x4 = x2 mod ed (i.e. x2 - x3*ed)
                    BFIz(xRAX, x3, 0, 16);
                    BFIz(xRDX, x4, 0, 16);
                    SET_DFNONE();
                    IFX(X_OF)                         {BFCw(xFlags, F_OF, 1);}
                    IFX(X_CF)                         {BFCw(xFlags, F_CF, 1);}
                    IFX2(X_AF, && !BOX64ENV(cputype)) {BFCw(xFlags, F_AF, 1);}
                    IFX2(X_AF, && BOX64ENV(cputype))  {ORRw_mask(xFlags, xFlags, 28, 0);}   //mask=0x10
                    IFX2(X_ZF, && !BOX64ENV(cputype)) {ORRw_mask(xFlags, xFlags, 26, 0);}   //mask=0x40
                    IFX2(X_ZF, && BOX64ENV(cputype))  {BFCw(xFlags, F_ZF, 1);}
                    IFX(X_SF)                         {BFCw(xFlags, F_SF, 1);}
                    IFX2(X_PF, && !BOX64ENV(cputype)) {ORRw_mask(xFlags, xFlags, 30, 0);}   //mask=0x04
                    IFX2(X_PF, && BOX64ENV(cputype))  {BFCw(xFlags, F_PF, 1);}
                    break;
                case 7:
                    INST_NAME("IDIV Ew");
                    SKIPTEST(x1);
                    if (!BOX64DRENV(dynarec_safeflags)) {
                        SETFLAGS(X_ALL, SF_SET);
                    } else if (BOX64ENV(cputype)) {
                        SETFLAGS(X_SF|X_PF|X_ZF|X_AF, SF_SET);
                    }
                    GETSEW(x1, 0);
                    if(BOX64ENV(dynarec_div0)) {
                        CBNZw_MARK3(ed);
                        GETIP_(ip);
                        STORE_XEMU_CALL(xRIP);
                        CALL_S(const_native_div0, -1);
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
                    if (!BOX64DRENV(dynarec_safeflags)) {
                        SET_DFNONE();
                    }
                    IFX2(X_AF, && BOX64ENV(cputype))  {ORRw_mask(xFlags, xFlags, 28, 0);}   //mask=0x10
                    IFX2(X_ZF, && BOX64ENV(cputype))  {BFCw(xFlags, F_ZF, 1);}
                    IFX2(X_SF, && BOX64ENV(cputype))  {BFCw(xFlags, F_SF, 1);}
                    IFX2(X_PF, && BOX64ENV(cputype))  {BFCw(xFlags, F_PF, 1);}
                    break;
            }
            break;
        case 0xF8:
        case 0xF9:
            return dynarec64_00(dyn, addr-1, ip, ninst, rex, ok, need_epilog);

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
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET);
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
