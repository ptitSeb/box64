#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <signal.h>

#include "debug.h"
#include "box64context.h"
#include "dynarec.h"
#include "emu/x64emu_private.h"
#include "emu/x64run_private.h"
#include "x64run.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "bridge.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"
#include "custommem.h"

#include "arm64_printer.h"
#include "dynarec_arm64_private.h"
#include "dynarec_arm64_functions.h"
#include "dynarec_arm64_helper.h"

int isSimpleWrapper(wrapper_t fun);
int isRetX87Wrapper(wrapper_t fun);

uintptr_t dynarec64_00(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
{
    uint8_t nextop, opcode;
    uint8_t gd, ed;
    int8_t i8;
    int32_t i32, tmp;
    int64_t i64, j64;
    uint8_t u8;
    uint8_t gb1, gb2, eb1, eb2;
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;
    uint8_t wback, wb1, wb2, wb;
    int64_t fixedaddress;
    int unscaled;
    int lock;
    int cacheupd = 0;

    opcode = F8;
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(j64);
    MAYUSE(wb);
    MAYUSE(lock);
    MAYUSE(cacheupd);

    switch(opcode) {
        case 0x00:
            INST_NAME("ADD Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB(x1, 0);
            GETGB(x2);
            emit_add8(dyn, ninst, x1, x2, x4, x5);
            EBBACK;
            break;
        case 0x01:
            INST_NAME("ADD Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_add32(dyn, ninst, rex, ed, gd, x3, x4);
            WBACK;
            break;
        case 0x02:
            INST_NAME("ADD Gb, Eb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB(x2, 0);
            GETGB(x1);
            emit_add8(dyn, ninst, x1, x2, x3, x4);
            GBBACK;
            break;
        case 0x03:
            INST_NAME("ADD Gd, Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_add32(dyn, ninst, rex, gd, ed, x3, x4);
            break;
        case 0x04:
            INST_NAME("ADD AL, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            u8 = F8;
            UXTBw(x1, xRAX);
            emit_add8c(dyn, ninst, x1, u8, x3, x4);
            BFIx(xRAX, x1, 0, 8);
            break;
        case 0x05:
            INST_NAME("ADD EAX, Id");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i64 = F32S;
            emit_add32c(dyn, ninst, rex, xRAX, i64, x3, x4, x5);
            break;
        case 0x06:
            if(rex.is32bits) {
                INST_NAME("PUSH ES");
                LDRH_U12(x1, xEmu, offsetof(x64emu_t, segs[_ES]));
                PUSH1_32(x1);
            } else {
                DEFAULT;
            }
            break;
        case 0x07:
            if(rex.is32bits) {
                INST_NAME("POP ES");
                POP1_32(x1);
                STRH_U12(x1, xEmu, offsetof(x64emu_t, segs[_ES]));
                STRw_U12(xZR, xEmu, offsetof(x64emu_t, segs_serial[_ES]));
            } else {
                DEFAULT;
            }
            break;
        case 0x08:
            INST_NAME("OR Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB(x1, 0);
            GETGB(x2);
            emit_or8(dyn, ninst, x1, x2, x4, x5);
            EBBACK;
            break;
        case 0x09:
            INST_NAME("OR Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_or32(dyn, ninst, rex, ed, gd, x3, x4);
            WBACK;
            break;
        case 0x0A:
            INST_NAME("OR Gb, Eb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB(x2, 0);
            GETGB(x1);
            emit_or8(dyn, ninst, x1, x2, x3, x4);
            GBBACK;
            break;
        case 0x0B:
            INST_NAME("OR Gd, Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_or32(dyn, ninst, rex, gd, ed, x3, x4);
            break;
        case 0x0C:
            INST_NAME("OR AL, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            u8 = F8;
            UXTBw(x1, xRAX);
            emit_or8c(dyn, ninst, x1, u8, x3, x4);
            BFIx(xRAX, x1, 0, 8);
            break;
        case 0x0D:
            INST_NAME("OR EAX, Id");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i64 = F32S;
            emit_or32c(dyn, ninst, rex, xRAX, i64, x3, x4);
            break;
        case 0x0E:
            if(rex.is32bits) {
                INST_NAME("PUSH CS");
                LDRH_U12(x1, xEmu, offsetof(x64emu_t, segs[_CS]));
                PUSH1_32(x1);
                SMWRITE();
            } else {
                DEFAULT;
            }
            break;
        case 0x0F:
            switch(rep) {
            case 1:
                addr = dynarec64_F20F(dyn, addr, ip, ninst, rex, ok, need_epilog);
                break;
            case 2:
                addr = dynarec64_F30F(dyn, addr, ip, ninst, rex, ok, need_epilog);
                break;
            default:
                addr = dynarec64_0F(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
            }
            break;
        case 0x10:
            INST_NAME("ADC Eb, Gb");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB(x1, 0);
            GETGB(x2);
            emit_adc8(dyn, ninst, x1, x2, x4, x5);
            EBBACK;
            break;
        case 0x11:
            INST_NAME("ADC Ed, Gd");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_adc32(dyn, ninst, rex, ed, gd, x3, x4);
            WBACK;
            break;
        case 0x12:
            INST_NAME("ADC Gb, Eb");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB(x2, 0);
            GETGB(x1);
            emit_adc8(dyn, ninst, x1, x2, x4, x3);
            GBBACK;
            break;
        case 0x13:
            INST_NAME("ADC Gd, Ed");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_adc32(dyn, ninst, rex, gd, ed, x3, x4);
            break;
        case 0x14:
            INST_NAME("ADC AL, Ib");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            u8 = F8;
            UXTBw(x1, xRAX);
            emit_adc8c(dyn, ninst, x1, u8, x3, x4, x5);
            BFIx(xRAX, x1, 0, 8);
            break;
        case 0x15:
            INST_NAME("ADC EAX, Id");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i64 = F32S;
            MOV64xw(x1, i64);
            emit_adc32(dyn, ninst, rex, xRAX, x1, x3, x4);
            break;
        case 0x16:
            if(rex.is32bits) {
                INST_NAME("PUSH SS");
                LDRH_U12(x1, xEmu, offsetof(x64emu_t, segs[_SS]));
                PUSH1_32(x1);
                SMWRITE();
            } else {
                DEFAULT;
            }
            break;
        case 0x17:
            if(rex.is32bits) {
                INST_NAME("POP SS");
                SMREAD();
                POP1_32(x1);
                STRH_U12(x1, xEmu, offsetof(x64emu_t, segs[_SS]));
                STRw_U12(xZR, xEmu, offsetof(x64emu_t, segs_serial[_SS]));
            } else {
                DEFAULT;
            }
            break;
        case 0x18:
            INST_NAME("SBB Eb, Gb");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB(x1, 0);
            GETGB(x2);
            emit_sbb8(dyn, ninst, x1, x2, x4, x5);
            EBBACK;
            break;
        case 0x19:
            INST_NAME("SBB Ed, Gd");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_sbb32(dyn, ninst, rex, ed, gd, x3, x4);
            WBACK;
            break;
        case 0x1A:
            INST_NAME("SBB Gb, Eb");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB(x2, 0);
            GETGB(x1);
            emit_sbb8(dyn, ninst, x1, x2, x3, x4);
            GBBACK;
            break;
        case 0x1B:
            INST_NAME("SBB Gd, Ed");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_sbb32(dyn, ninst, rex, gd, ed, x3, x4);
            break;
        case 0x1C:
            INST_NAME("SBB AL, Ib");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            u8 = F8;
            UXTBw(x1, xRAX);
            emit_sbb8c(dyn, ninst, x1, u8, x3, x4, x5);
            BFIx(xRAX, x1, 0, 8);
            break;
        case 0x1D:
            INST_NAME("SBB EAX, Id");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i64 = F32S;
            MOV64xw(x2, i64);
            emit_sbb32(dyn, ninst, rex, xRAX, x2, x3, x4);
            break;
        case 0x1E:
            if(rex.is32bits) {
                INST_NAME("PUSH DS");
                LDRH_U12(x1, xEmu, offsetof(x64emu_t, segs[_DS]));
                PUSH1_32(x1);
                SMWRITE();
            } else {
                DEFAULT;
            }
            break;
        case 0x1F:
            if(rex.is32bits) {
                INST_NAME("POP DS");
                SMREAD();
                POP1_32(x1);
                STRH_U12(x1, xEmu, offsetof(x64emu_t, segs[_DS]));
                STRw_U12(xZR, xEmu, offsetof(x64emu_t, segs_serial[_DS]));
            } else {
                DEFAULT;
            }
            break;
        case 0x20:
            INST_NAME("AND Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB(x1, 0);
            GETGB(x2);
            emit_and8(dyn, ninst, x1, x2, x4, x5);
            EBBACK;
            break;
        case 0x21:
            INST_NAME("AND Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_and32(dyn, ninst, rex, ed, gd, x3, x4);
            WBACK;
            break;
        case 0x22:
            INST_NAME("AND Gb, Eb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB(x2, 0);
            GETGB(x1);
            emit_and8(dyn, ninst, x1, x2, x3, x4);
            GBBACK;
            break;
        case 0x23:
            INST_NAME("AND Gd, Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_and32(dyn, ninst, rex, gd, ed, x3, x4);
            break;
        case 0x24:
            INST_NAME("AND AL, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            u8 = F8;
            UXTBw(x1, xRAX);
            emit_and8c(dyn, ninst, x1, u8, x3, x4);
            BFIx(xRAX, x1, 0, 8);
            break;
        case 0x25:
            INST_NAME("AND EAX, Id");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i64 = F32S;
            emit_and32c(dyn, ninst, rex, xRAX, i64, x3, x4);
            break;
        case 0x27:
            if(rex.is32bits) {
                INST_NAME("DAA");
                MESSAGE(LOG_DUMP, "Need Optimization DAA\n");
                READFLAGS(X_AF|X_CF);
                SETFLAGS(X_ALL, SF_SET_DF);
                UXTBx(x1, xRAX);
                CALL_(daa8, x1, 0);
                BFIz(xRAX, x1, 0, 8);
            } else {
                DEFAULT;
            }
            break;
        case 0x28:
            INST_NAME("SUB Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB(x1, 0);
            GETGB(x2);
            emit_sub8(dyn, ninst, x1, x2, x4, x5);
            EBBACK;
            break;
        case 0x29:
            INST_NAME("SUB Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_sub32(dyn, ninst, rex, ed, gd, x3, x4);
            WBACK;
            break;
        case 0x2A:
            INST_NAME("SUB Gb, Eb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB(x2, 0);
            GETGB(x1);
            emit_sub8(dyn, ninst, x1, x2, x3, x4);
            GBBACK;
            break;
        case 0x2B:
            INST_NAME("SUB Gd, Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_sub32(dyn, ninst, rex, gd, ed, x3, x4);
            break;
        case 0x2C:
            INST_NAME("SUB AL, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            u8 = F8;
            UXTBw(x1, xRAX);
            emit_sub8c(dyn, ninst, x1, u8, x3, x4, x5);
            BFIx(xRAX, x1, 0, 8);
            break;
        case 0x2D:
            INST_NAME("SUB EAX, Id");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i64 = F32S;
            emit_sub32c(dyn, ninst, rex, xRAX, i64, x3, x4, x5);
            break;
        case 0x2E:
            INST_NAME("CS:");
            break;
        case 0x2F:
            if(rex.is32bits) {
                INST_NAME("DAS");
                MESSAGE(LOG_DUMP, "Need Optimization DAS\n");
                READFLAGS(X_AF|X_CF);
                SETFLAGS(X_ALL, SF_SET_DF);
                UXTBx(x1, xRAX);
                CALL_(das8, x1, 0);
                BFIz(xRAX, x1, 0, 8);
            } else {
                DEFAULT;
            }
            break;
        case 0x30:
            INST_NAME("XOR Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB(x1, 0);
            GETGB(x2);
            emit_xor8(dyn, ninst, x1, x2, x4, x5);
            EBBACK;
            break;
        case 0x31:
            INST_NAME("XOR Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_xor32(dyn, ninst, rex, ed, gd, x3, x4);
            WBACK;
            break;
        case 0x32:
            INST_NAME("XOR Gb, Eb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB(x2, 0);
            GETGB(x1);
            emit_xor8(dyn, ninst, x1, x2, x3, x4);
            GBBACK;
            break;
        case 0x33:
            INST_NAME("XOR Gd, Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_xor32(dyn, ninst, rex, gd, ed, x3, x4);
            break;
        case 0x34:
            INST_NAME("XOR AL, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            u8 = F8;
            UXTBw(x1, xRAX);
            emit_xor8c(dyn, ninst, x1, u8, x3, x4);
            BFIx(xRAX, x1, 0, 8);
            break;
        case 0x35:
            INST_NAME("XOR EAX, Id");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i64 = F32S;
            emit_xor32c(dyn, ninst, rex, xRAX, i64, x3, x4);
            break;
        case 0x36:
            INST_NAME("SS:");
            break;
        case 0x37:
            if(rex.is32bits) {
                INST_NAME("AAA");
                MESSAGE(LOG_DUMP, "Need Optimization AAA\n");
                READFLAGS(X_AF);
                SETFLAGS(X_ALL, SF_SET_DF);
                UXTHx(x1, xRAX);
                CALL_(aaa16, x1, 0);
                BFIz(xRAX, x1, 0, 16);
            } else {
                DEFAULT;
            }
            break;
        case 0x38:
            INST_NAME("CMP Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB(x1, 0);
            GETGB(x2);
            emit_cmp8(dyn, ninst, x1, x2, x3, x4, x5);
            break;
        case 0x39:
            INST_NAME("CMP Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_cmp32(dyn, ninst, rex, ed, gd, x3, x4, x5);
            break;
        case 0x3A:
            INST_NAME("CMP Gb, Eb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB(x2, 0);
            GETGB(x1);
            emit_cmp8(dyn, ninst, x1, x2, x3, x4, x5);
            break;
        case 0x3B:
            INST_NAME("CMP Gd, Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_cmp32(dyn, ninst, rex, gd, ed, x3, x4, x5);
            break;
        case 0x3C:
            INST_NAME("CMP AL, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            u8 = F8;
            UXTBw(x1, xRAX);
            if(u8) {
                MOV32w(x2, u8);
                emit_cmp8(dyn, ninst, x1, x2, x3, x4, x5);
            } else {
                emit_cmp8_0(dyn, ninst, x1, x3, x4);
            }
            break;
        case 0x3D:
            INST_NAME("CMP EAX, Id");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i64 = F32S;
            if(i64) {
                MOV64xw(x2, i64);
                emit_cmp32(dyn, ninst, rex, xRAX, x2, x3, x4, x5);
            } else
                emit_cmp32_0(dyn, ninst, rex, xRAX, x3, x4);
            break;

        case 0x3F:
            if(rex.is32bits) {
                INST_NAME("AAS");
                MESSAGE(LOG_DUMP, "Need Optimization AAS\n");
                READFLAGS(X_AF);
                SETFLAGS(X_ALL, SF_SET_DF);
                UXTHw(x1, xRAX);
                CALL_(aas16, x1, 0);
                BFIx(xRAX, x1, 0, 16);
            } else {
                DEFAULT;
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
            INST_NAME("INC Reg (32bits)");
            SETFLAGS(X_ALL&~X_CF, SF_SUBSET_PENDING);
            gd = xRAX + (opcode&7);
            emit_inc32(dyn, ninst, rex, gd, x1, x2);
            break;
        case 0x48:
        case 0x49:
        case 0x4A:
        case 0x4B:
        case 0x4C:
        case 0x4D:
        case 0x4E:
        case 0x4F:
            INST_NAME("DEC Reg (32bits)");
            SETFLAGS(X_ALL&~X_CF, SF_SUBSET_PENDING);
            gd = xRAX + (opcode&7);
            emit_dec32(dyn, ninst, rex, gd, x1, x2);
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
            if(dyn->doublepush) {
                SKIPTEST(x1);
                dyn->doublepush = 0;
            } else {
                WILLWRITE();
                gd = xRAX+(opcode&0x07)+(rex.b<<3);
                u32 = PK(0);
                i32 = 1;
                rex.rex = 0;
                if(!rex.is32bits)
                    while(u32>=0x40 && u32<=0x4f) {
                        rex.rex = u32;
                        u32 = PK(i32);
                        i32++;
                    }
                if(!box64_dynarec_test && u32>=0x50 && u32<=0x57 && (dyn->size>(ninst+1) && dyn->insts[ninst+1].pred_sz==1) && gd != xRSP) {
                    u32= xRAX+(u32&0x07)+(rex.b<<3);
                    if(u32==xRSP) {
                        PUSH1z(gd);
                    } else {
                        // double push!
                        MESSAGE(LOG_DUMP, "DOUBLE PUSH\n");
                        PUSH2z(gd, u32);
                        dyn->doublepush = 1;
                    }
                } else {
                    if (gd == xRSP) {
                        MOVz_REG(x1, xRSP);
                        PUSH1z(x1);
                    } else {
                        PUSH1z(gd);
                    }
                }
                SMWRITE();
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
            if(dyn->doublepop) {
                SKIPTEST(x1);
                dyn->doublepop = 0;
            } else {
                SMREAD();
                gd = xRAX+(opcode&0x07)+(rex.b<<3);
                u32 = PK(0);
                i32 = 1;
                rex.rex = 0;
                if(!rex.is32bits)
                    while(u32>=0x40 && u32<=0x4f) {
                        rex.rex = u32;
                        u32 = PK(i32);
                        i32++;
                    }
                if(!box64_dynarec_test && (gd!=xRSP) && u32>=0x58 && u32<=0x5f && (dyn->size>(ninst+1) && dyn->insts[ninst+1].pred_sz==1)) {
                    // double pop!
                    u32= xRAX+(u32&0x07)+(rex.b<<3);
                    MESSAGE(LOG_DUMP, "DOUBLE POP\n");
                    if(gd==u32) {
                        ADDz_U12(xRSP, xRSP, rex.is32bits?0x4:0x8);
                        POP1z(gd);
                    } else {
                        POP2z(gd, (u32==xRSP)?x1:u32);
                        if(u32==xRSP) {
                            MOVz_REG(u32, x1);
                        }
                    }
                    dyn->doublepop = 1;
                    SKIPTEST(x1);  // disable test for this OP
                } else {
                    if(gd == xRSP) {
                        POP1z(x1);
                        MOVz_REG(gd, x1);
                    } else {
                        POP1z(gd);
                    }
                }
            }
            break;
        case 0x60:
            if(rex.is32bits) {
                INST_NAME("PUSHAD");
                WILLWRITE();
                MOVw_REG(x1, xRSP);
                PUSH2_32(xRAX, xRCX);
                PUSH2_32(xRDX, xRBX);
                PUSH2_32(x1, xRBP);
                PUSH2_32(xRSI, xRDI);
                SMWRITE();
            } else {
                DEFAULT;
            }
            break;
        case 0x61:
            if(rex.is32bits) {
                INST_NAME("POPAD");
                SMREAD();
                POP2_32(xRDI, xRSI);
                POP2_32(xRBP, x1);
                POP2_32(xRBX, xRDX);
                POP2_32(xRCX, xRAX);
            } else {
                DEFAULT;
            }
            break;
        case 0x62:
            if(rex.is32bits) {
                // BOUND here
                DEFAULT;                
            } else {
                INST_NAME("BOUND Gd, Ed");
                nextop = F8;
                FAKEED(0);
            }
            break;
        case 0x63:
            if(rex.is32bits) {
                // ARPL here
                DEFAULT;                
            } else {
                INST_NAME("MOVSXD Gd, Ed");
                nextop = F8;
                GETGD;
                if(rex.w) {
                    if(MODREG) {   // reg <= reg
                        SXTWx(gd, xRAX+(nextop&7)+(rex.b<<3));
                    } else {                    // mem <= reg
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff<<2, 3, rex, NULL, 0, 0);
                        LDSW(gd, ed, fixedaddress);
                    }
                } else {
                    if(MODREG) {   // reg <= reg
                        MOVw_REG(gd, xRAX+(nextop&7)+(rex.b<<3));
                    } else {                    // mem <= reg
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff<<2, 3, rex, NULL, 0, 0);
                        LDW(gd, ed, fixedaddress);
                    }
                }
            }
            break;
        case 0x64:
            addr = dynarec64_64(dyn, addr, ip, ninst, rex, rep, _FS, ok, need_epilog);
            break;
        case 0x65:
            addr = dynarec64_64(dyn, addr, ip, ninst, rex, rep, _GS, ok, need_epilog);
            break;
        case 0x66:
            addr = dynarec64_66(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
            break;
        case 0x67:
            if(rex.is32bits)
                addr = dynarec64_67_32(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
            else
                addr = dynarec64_67(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
            break;
        case 0x68:
            INST_NAME("PUSH Id");
            i64 = F32S;
            if(PK(0)==0xC3) {
                MESSAGE(LOG_DUMP, "PUSH then RET, using indirect\n");
                TABLE64(x3, addr-4);
                LDRSW_U12(x1, x3, 0);
                PUSH1z(x1);
            } else {
                WILLWRITE();
                MOV64z(x3, i64);
                PUSH1z(x3);
                SMWRITE();
            }
            break;
        case 0x69:
            INST_NAME("IMUL Gd, Ed, Id");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED(4);
            i64 = F32S;
            MOV64xw(x4, i64);
            if(rex.w) {
                // 64bits imul
                UFLAG_IF {
                    SMULH(x3, ed, x4);
                    MULx(gd, ed, x4);
                    IFX(X_PEND) {
                        UFLAG_OP1(x3);
                        UFLAG_RES(gd);
                        UFLAG_DF(x1, d_imul64);
                    } else {
                        SET_DFNONE(x1);
                    }
                    IFX(X_ZF | X_PF | X_AF | X_SF) {
                        MOV32w(x1, (1<<F_ZF)|(1<<F_AF)|(1<<F_PF)|(1<<F_SF));
                        BICw(xFlags, xFlags, x1);
                    }
                    IFX(X_CF | X_OF) {
                        ASRx(x4, gd, 63);
                        CMPSx_REG(x3, x4);
                        CSETw(x1, cNE);
                        IFX(X_CF) {
                            BFIw(xFlags, x1, F_CF, 1);
                        }
                        IFX(X_OF) {
                            BFIw(xFlags, x1, F_OF, 1);
                        }
                    }
                } else {
                    MULxw(gd, ed, x4);
                }
            } else {
                // 32bits imul
                UFLAG_IF {
                    SMULL(gd, ed, x4);
                    LSRx(x3, gd, 32);
                    MOVw_REG(gd, gd);
                    IFX(X_PEND) {
                        UFLAG_RES(gd);
                        UFLAG_OP1(x3);
                        UFLAG_DF(x1, d_imul32);
                    } else {
                        SET_DFNONE(x1);
                    }
                    IFX(X_ZF | X_PF | X_AF | X_SF) {
                        MOV32w(x1, (1<<F_ZF)|(1<<F_AF)|(1<<F_PF)|(1<<F_SF));
                        BICw(xFlags, xFlags, x1);
                    }
                    IFX(X_CF | X_OF) {
                        ASRw(x4, gd, 31);
                        CMPSw_REG(x3, x4);
                        CSETw(x1, cNE);
                        IFX(X_CF) {
                            BFIw(xFlags, x1, F_CF, 1);
                        }
                        IFX(X_OF) {
                            BFIw(xFlags, x1, F_OF, 1);
                        }
                    }
                } else {
                    MULxw(gd, ed, x4);
                }
            }
            break;
        case 0x6A:
            INST_NAME("PUSH Ib");
            i64 = F8S;
            MOV64z(x3, i64);
            WILLWRITE();
            PUSH1z(x3);
            SMWRITE();
            break;
        case 0x6B:
            INST_NAME("IMUL Gd, Ed, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED(1);
            i64 = F8S;
            MOV64xw(x4, i64);
            if(rex.w) {
                // 64bits imul
                UFLAG_IF {
                    SMULH(x3, ed, x4);
                    MULx(gd, ed, x4);
                    IFX(X_PEND) {
                        UFLAG_OP1(x3);
                        UFLAG_RES(gd);
                        UFLAG_DF(x1, d_imul64);
                    } else {
                        SET_DFNONE(x1);
                    }
                    IFX(X_ZF | X_PF | X_AF | X_SF) {
                        MOV32w(x1, (1<<F_ZF)|(1<<F_AF)|(1<<F_PF)|(1<<F_SF));
                        BICw(xFlags, xFlags, x1);
                    }
                    IFX(X_CF | X_OF) {
                        ASRx(x4, gd, 63);
                        CMPSx_REG(x3, x4);
                        CSETw(x1, cNE);
                        IFX(X_CF) {
                            BFIw(xFlags, x1, F_CF, 1);
                        }
                        IFX(X_OF) {
                            BFIw(xFlags, x1, F_OF, 1);
                        }
                    }
                } else {
                    MULxw(gd, ed, x4);
                }
            } else {
                // 32bits imul
                UFLAG_IF {
                    SMULL(gd, ed, x4);
                    LSRx(x3, gd, 32);
                    MOVw_REG(gd, gd);
                    IFX(X_PEND) {
                        UFLAG_RES(gd);
                        UFLAG_OP1(x3);
                        UFLAG_DF(x1, d_imul32);
                    } else {
                        SET_DFNONE(x1);
                    }
                    IFX(X_ZF | X_PF | X_AF | X_SF) {
                        MOV32w(x1, (1<<F_ZF)|(1<<F_AF)|(1<<F_PF)|(1<<F_SF));
                        BICw(xFlags, xFlags, x1);
                    }
                    IFX(X_CF | X_OF) {
                        ASRw(x4, gd, 31);
                        CMPSw_REG(x3, x4);
                        CSETw(x1, cNE);
                        IFX(X_CF) {
                            BFIw(xFlags, x1, F_CF, 1);
                        }
                        IFX(X_OF) {
                            BFIw(xFlags, x1, F_OF, 1);
                        }
                    }
                } else {
                    MULxw(gd, ed, x4);
                }
            }
            break;
        case 0x6C:
        case 0x6D:
            INST_NAME(opcode == 0x6C ? "INSB" : "INSD");
            SETFLAGS(X_ALL, SF_SET_NODF);    // Hack to set flags in "don't care" state
            GETIP(ip);
            STORE_XEMU_CALL(xRIP);
            CALL(native_priv, -1);
            LOAD_XEMU_CALL(xRIP);
            jump_to_epilog(dyn, 0, xRIP, ninst);
            *need_epilog = 0;
            *ok = 0;
            break;
        case 0x6E:
        case 0x6F:
            INST_NAME(opcode == 0x6C ? "OUTSB" : "OUTSD");
            SETFLAGS(X_ALL, SF_SET_NODF);    // Hack to set flags in "don't care" state
            GETIP(ip);
            STORE_XEMU_CALL(xRIP);
            CALL(native_priv, -1);
            LOAD_XEMU_CALL(xRIP);
            jump_to_epilog(dyn, 0, xRIP, ninst);
            *need_epilog = 0;
            *ok = 0;
            break;

        #define GO(GETFLAGS, NO, YES, F)                                \
            if (box64_dynarec_test == 2) { NOTEST(x1); }                \
            READFLAGS(F);                                               \
            i8 = F8S;                                                   \
            BARRIER(BARRIER_MAYBE);                                     \
            JUMP(addr+i8, 1);                                           \
            GETFLAGS;                                                   \
            if(dyn->insts[ninst].x64.jmp_insts==-1 ||                   \
                CHECK_CACHE()) {                                        \
                /* out of the block */                                  \
                i32 = dyn->insts[ninst].epilog-(dyn->native_size);      \
                Bcond(NO, i32);                                         \
                if(dyn->insts[ninst].x64.jmp_insts==-1) {               \
                    if(!(dyn->insts[ninst].x64.barrier&BARRIER_FLOAT))  \
                        fpu_purgecache(dyn, ninst, 1, x1, x2, x3);      \
                    jump_to_next(dyn, addr+i8, 0, ninst, rex.is32bits); \
                } else {                                                \
                    CacheTransform(dyn, ninst, cacheupd, x1, x2, x3);   \
                    i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address-(dyn->native_size);\
                    B(i32);                                             \
                }                                                       \
            } else {                                                    \
                /* inside the block */                                  \
                i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address-(dyn->native_size);    \
                Bcond(YES, i32);                                        \
            }

        GOCOND(0x70, "J", "ib");

        #undef GO

        case 0x82:
            if(!rex.is32bits) {
                DEFAULT;
                return ip;
            }
            // fallthru
        case 0x80:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0: //ADD
                    INST_NAME("ADD Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEB(x1, 1);
                    u8 = F8;
                    emit_add8c(dyn, ninst, x1, u8, x2, x4);
                    EBBACK;
                    break;
                case 1: //OR
                    INST_NAME("OR Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEB(x1, 1);
                    u8 = F8;
                    emit_or8c(dyn, ninst, x1, u8, x2, x4);
                    EBBACK;
                    break;
                case 2: //ADC
                    INST_NAME("ADC Eb, Ib");
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEB(x1, 1);
                    u8 = F8;
                    emit_adc8c(dyn, ninst, x1, u8, x2, x4, x5);
                    EBBACK;
                    break;
                case 3: //SBB
                    INST_NAME("SBB Eb, Ib");
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEB(x1, 1);
                    u8 = F8;
                    emit_sbb8c(dyn, ninst, x1, u8, x2, x4, x5);
                    EBBACK;
                    break;
                case 4: //AND
                    INST_NAME("AND Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEB(x1, 1);
                    u8 = F8;
                    emit_and8c(dyn, ninst, x1, u8, x2, x4);
                    EBBACK;
                    break;
                case 5: //SUB
                    INST_NAME("SUB Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEB(x1, 1);
                    u8 = F8;
                    emit_sub8c(dyn, ninst, x1, u8, x2, x4, x5);
                    EBBACK;
                    break;
                case 6: //XOR
                    INST_NAME("XOR Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEB(x1, 1);
                    u8 = F8;
                    emit_xor8c(dyn, ninst, x1, u8, x2, x4);
                    EBBACK;
                    break;
                case 7: //CMP
                    INST_NAME("CMP Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEB(x1, 1);
                    u8 = F8;
                    if(u8) {
                        MOV32w(x2, u8);
                        emit_cmp8(dyn, ninst, x1, x2, x3, x4, x5);
                    } else {
                        emit_cmp8_0(dyn, ninst, x1, x3, x4);
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0x81:
        case 0x83:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0: //ADD
                    if(opcode==0x81) {INST_NAME("ADD Ed, Id");} else {INST_NAME("ADD Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETED((opcode==0x81)?4:1);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    emit_add32c(dyn, ninst, rex, ed, i64, x3, x4, x5);
                    WBACK;
                    break;
                case 1: //OR
                    if(opcode==0x81) {INST_NAME("OR Ed, Id");} else {INST_NAME("OR Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETED((opcode==0x81)?4:1);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    emit_or32c(dyn, ninst, rex, ed, i64, x3, x4);
                    WBACK;
                    break;
                case 2: //ADC
                    if(opcode==0x81) {INST_NAME("ADC Ed, Id");} else {INST_NAME("ADC Ed, Ib");}
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETED((opcode==0x81)?4:1);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    MOV64xw(x5, i64);
                    emit_adc32(dyn, ninst, rex, ed, x5, x3, x4);
                    WBACK;
                    break;
                case 3: //SBB
                    if(opcode==0x81) {INST_NAME("SBB Ed, Id");} else {INST_NAME("SBB Ed, Ib");}
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETED((opcode==0x81)?4:1);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    MOV64xw(x5, i64);
                    emit_sbb32(dyn, ninst, rex, ed, x5, x3, x4);
                    WBACK;
                    break;
                case 4: //AND
                    if(opcode==0x81) {INST_NAME("AND Ed, Id");} else {INST_NAME("AND Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETED((opcode==0x81)?4:1);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    emit_and32c(dyn, ninst, rex, ed, i64, x3, x4);
                    WBACK;
                    break;
                case 5: //SUB
                    if(opcode==0x81) {INST_NAME("SUB Ed, Id");} else {INST_NAME("SUB Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETED((opcode==0x81)?4:1);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    emit_sub32c(dyn, ninst, rex, ed, i64, x3, x4, x5);
                    WBACK;
                    break;
                case 6: //XOR
                    if(opcode==0x81) {INST_NAME("XOR Ed, Id");} else {INST_NAME("XOR Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETED((opcode==0x81)?4:1);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    emit_xor32c(dyn, ninst, rex, ed, i64, x3, x4);
                    WBACK;
                    break;
                case 7: //CMP
                    if(opcode==0x81) {INST_NAME("CMP Ed, Id");} else {INST_NAME("CMP Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETED((opcode==0x81)?4:1);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    if(i64) {
                        MOV64xw(x2, i64);
                        emit_cmp32(dyn, ninst, rex, ed, x2, x3, x4, x5);
                    } else
                        emit_cmp32_0(dyn, ninst, rex, ed, x3, x4);
                    break;
            }
            break;
        case 0x84:
            INST_NAME("TEST Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop=F8;
            GETEB(x1, 0);
            GETGB(x2);
            emit_test8(dyn, ninst, x1, x2, x3, x4, x5);
            break;
        case 0x85:
            INST_NAME("TEST Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop=F8;
            GETGD;
            GETED(0);
            emit_test32(dyn, ninst, rex, ed, gd, x3, x5, x6);
            break;
        case 0x86:
            INST_NAME("(LOCK)XCHG Eb, Gb");
            // Do the swap
            nextop = F8;
            GETGB(x4);
            if(MODREG) {
                if(rex.rex) {
                    ed = xRAX+(nextop&7)+(rex.b<<3);
                    eb1 = ed;
                    eb2 = 0;
                } else {
                    ed = (nextop&7);
                    eb1 = xRAX+(ed&3);
                    eb2 = ((ed&4)<<1);
                }
                UBFXw(x1, eb1, eb2, 8);
                // do the swap 14 -> ed, 1 -> gd
                BFIx(eb1, x4, eb2, 8);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, 0);
                if(arm64_atomics) {
                    SWPALB(x4, x1, ed);
                } else {
                    MARKLOCK;
                    // do the swap with exclusive locking
                    LDAXRB(x1, ed);
                    // do the swap 4 -> strb(ed), 1 -> gd
                    STLXRB(x3, x4, ed);
                    CBNZx_MARKLOCK(x3);
                }
                SMDMB();
            }
            BFIx(gb1, x1, gb2, 8);
            break;
        case 0x87:
            INST_NAME("(LOCK)XCHG Ed, Gd");
            nextop = F8;
            if(MODREG) {
                GETGD;
                GETED(0);
                MOVxw_REG(x1, gd);
                MOVxw_REG(gd, ed);
                MOVxw_REG(ed, x1);
            } else {
                GETGD;
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, 0);
                if(!ALIGNED_ATOMICxw) {
                    if(arm64_uscat) {
                        ANDx_mask(x1, ed, 1, 0, 3);  // mask = F
                        CMPSw_U12(x1, 16-(1<<(2+rex.w)));
                        B_MARK(cGT);
                    } else {
                        TSTx_mask(ed, 1, 0, 1+rex.w);    // mask=3 or 7
                        B_MARK(cNE);
                    }
                }
                if(arm64_atomics) {
                    SWPALxw(gd, gd, ed);
                    if(!ALIGNED_ATOMICxw) {
                        B_NEXT_nocond;
                    }
                } else {
                    MARKLOCK;
                    LDAXRxw(x1, ed);
                    STLXRxw(x3, gd, ed);
                    CBNZx_MARKLOCK(x3);
                    if(!ALIGNED_ATOMICxw) {
                        B_MARK2_nocond;
                    }
                }
                if(!ALIGNED_ATOMICxw) {
                    MARK;
                    LDRxw_U12(x1, ed, 0);
                    LDAXRB(x3, ed);
                    STLXRB(x3, gd, ed);
                    CBNZx_MARK(x3);
                    STRxw_U12(gd, ed, 0);
                    MARK2;
                }
                SMDMB();
                if(!ALIGNED_ATOMICxw || !arm64_atomics) {
                    MOVxw_REG(gd, x1);
                }
            }
            break;
        case 0x88:
            INST_NAME("MOV Eb, Gb");
            nextop = F8;
            gd = ((nextop&0x38)>>3)+(rex.r<<3);
            if(rex.rex) {
                gb2 = 0;
                gb1 = xRAX + gd;
            } else {
                gb2 = ((gd&4)<<1);
                gb1 = xRAX+(gd&3);
            }
            if(gb2) {
                gd = x4;
                UBFXw(gd, gb1, gb2, 8);
            } else {
                gd = gb1;   // no need to extract
            }
            if(MODREG) {
                ed = (nextop&7) + (rex.b<<3);
                if(rex.rex) {
                    eb1 = xRAX+ed;
                    eb2 = 0;
                } else {
                    eb1 = xRAX+(ed&3);  // Ax, Cx, Dx or Bx
                    eb2 = ((ed&4)>>2);    // L or H
                }
                BFIx(eb1, gd, eb2*8, 8);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff, 0, rex, &lock, 0, 0);
                WILLWRITELOCK(lock);
                STB(gd, ed, fixedaddress);
                SMWRITELOCK(lock);
            }
            break;
        case 0x89:
            INST_NAME("MOV Ed, Gd");
            nextop=F8;
            GETGD;
            if(MODREG) {   // reg <= reg
                MOVxw_REG(xRAX+(nextop&7)+(rex.b<<3), gd);
            } else {                    // mem <= reg
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff<<(2+rex.w), (1<<(2+rex.w))-1, rex, &lock, 0, 0);
                WILLWRITELOCK(lock);
                STxw(gd, ed, fixedaddress);
                SMWRITELOCK(lock);
            }
            break;
        case 0x8A:
            INST_NAME("MOV Gb, Eb");
            nextop = F8;
            if(rex.rex) {
                gb1 = gd = xRAX+((nextop&0x38)>>3)+(rex.r<<3);
                gb2=0;
            } else {
                gd = (nextop&0x38)>>3;
                gb1 = xRAX+(gd&3);
                gb2 = ((gd&4)<<1);
            }
            if(MODREG) {
                if(rex.rex) {
                    wback = xRAX+(nextop&7)+(rex.b<<3);
                    wb2 = 0;
                } else {
                    wback = (nextop&7);
                    wb2 = (wback>>2);
                    wback = xRAX+(wback&3);
                }
                if(wb2) {
                    UBFXw(x4, wback, wb2*8, 8);
                    ed = x4;
                } else {
                    ed = wback;
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, &unscaled, 0xfff, 0, rex, &lock, 0, 0);
                SMREADLOCK(lock);
                LDB(x4, wback, fixedaddress);
                ed = x4;
            }
            BFIx(gb1, ed, gb2, 8);
            break;
        case 0x8B:
            INST_NAME("MOV Gd, Ed");
            nextop=F8;
            GETGD;
            if(MODREG) {
                MOVxw_REG(gd, xRAX+(nextop&7)+(rex.b<<3));
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff<<(2+rex.w), (1<<(2+rex.w))-1, rex, &lock, 0, 0);
                SMREADLOCK(lock);
                LDxw(gd, ed, fixedaddress);
            }
            break;
        case 0x8C:
            INST_NAME("MOV Ed, Seg");
            nextop=F8;
            u8 = (nextop&0x38)>>3;
            if((nextop&0xC0)==0xC0) {   // reg <= seg
                gd = xRAX+(nextop&7)+(rex.b<<3);
                LDRH_U12(gd, xEmu, offsetof(x64emu_t, segs[u8]));
                UXTHw(gd, gd);
            } else {                    // mem <= seg
                LDRH_U12(x3, xEmu, offsetof(x64emu_t, segs[u8]));
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
                addr = geted(dyn, addr, ninst, nextop, &ed, gd, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                if(gd!=ed) {    // it's sometimes used as a 3 bytes NOP
                    MOVxw_REG(gd, ed);
                }
                else if(!rex.w && !rex.is32bits) {
                    MOVw_REG(gd, gd);   //truncate the higher 32bits as asked
                }
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
            INST_NAME("POP Ed");
            nextop = F8;
            SMREAD();
            if(MODREG) {
                POP1z(xRAX+(nextop&7)+(rex.b<<3));
            } else {
                POP1z(x2); // so this can handle POP [ESP] and maybe some variant too
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<3, 7, rex, NULL, 0, 0);
                if(ed==xRSP) {
                    STz(x2, ed, fixedaddress);
                } else {
                    // complicated to just allow a segfault that can be recovered correctly
                    SUBz_U12(xRSP, xRSP, rex.is32bits?4:8);
                    STz(x2, ed, fixedaddress);
                    ADDz_U12(xRSP, xRSP, rex.is32bits?4:8);
                }
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
                INST_NAME("XCHG EAX, Reg");
                MOVxw_REG(x2, xRAX);
                MOVxw_REG(xRAX, gd);
                MOVxw_REG(gd, x2);
            }
            break;

        case 0x98:
            if(rex.w) {
                INST_NAME("CDQE");
                SXTWx(xRAX, xRAX);
            } else {
                INST_NAME("CWDE");
                SXTHw(xRAX, xRAX);
            }
            break;
        case 0x99:
            INST_NAME("CDQ");
            SBFXxw(xRDX, xRAX, rex.w?63:31, 1);
            break;

        case 0x9B:
            INST_NAME("FWAIT");
            break;
        case 0x9C:
            INST_NAME("PUSHF");
            READFLAGS(X_ALL);
            PUSH1z(xFlags);
            break;
        case 0x9D:
            INST_NAME("POPF");
            SETFLAGS(X_ALL, SF_SET);
            POP1z(xFlags);
            MOV32w(x1, 0x3F7FD7);
            ANDw_REG(xFlags, xFlags, x1);
            MOV32w(x1, 0x202);
            ORRw_REG(xFlags, xFlags, x1);
            SET_DFNONE(x1);
            if(box64_wine) {    // should this be done all the time?
                TBZ_NEXT(xFlags, F_TF);
                // go to epilog, TF should trigger at end of next opcode, so using Interpretor only
                jump_to_epilog(dyn, addr, 0, ninst);
            }
            break;
        case 0x9E:
            INST_NAME("SAHF");
            SETFLAGS(X_CF|X_PF|X_AF|X_ZF|X_SF, SF_SUBSET);
            MOV32w(x2, 0b11010101);
            BICw_REG(xFlags, xFlags, x2);
            ANDw_REG_LSR(x1, x2, xRAX, 8);
            ORRw_REG(xFlags, xFlags, x1);
            SET_DFNONE(x1);
            break;
        case 0x9F:
            INST_NAME("LAHF");
            READFLAGS(X_CF|X_PF|X_AF|X_ZF|X_SF);
            BFIx(xRAX, xFlags, 8, 8);
            break;
        case 0xA0:
            INST_NAME("MOV AL,Ob");
            if(rex.is32bits)
                u64 = F32;
            else
                u64 = F64;
            MOV64z(x1, u64);
            lock=isLockAddress(u64);
            SMREADLOCK(lock);
            LDRB_U12(x2, x1, 0);
            BFIx(xRAX, x2, 0, 8);
            break;
        case 0xA1:
            INST_NAME("MOV EAX,Od");
            if(rex.is32bits)
                u64 = F32;
            else
                u64 = F64;
            MOV64z(x1, u64);
            lock=isLockAddress(u64);
            SMREADLOCK(lock);
            LDRxw_U12(xRAX, x1, 0);
            break;
        case 0xA2:
            INST_NAME("MOV Ob,AL");
            if(rex.is32bits)
                u64 = F32;
            else
                u64 = F64;
            MOV64z(x1, u64);
            lock=isLockAddress(u64);
            WILLWRITELOCK(lock);
            STRB_U12(xRAX, x1, 0);
            SMWRITELOCK(lock);
            break;
        case 0xA3:
            INST_NAME("MOV Od,EAX");
            if(rex.is32bits)
                u64 = F32;
            else
                u64 = F64;
            MOV64z(x1, u64);
            lock=isLockAddress(u64);
            WILLWRITELOCK(lock);
            STRxw_U12(xRAX, x1, 0);
            SMWRITELOCK(lock);
            break;
        case 0xA4:
            SMREAD();
            if(rep) {
                INST_NAME("REP MOVSB");
                CBZx_NEXT(xRCX);
                TBNZ_MARK2(xFlags, F_DF);
                MARK;   // Part with DF==0
                LDRB_S9_postindex(x1, xRSI, 1);
                STRB_S9_postindex(x1, xRDI, 1);
                SUBx_U12(xRCX, xRCX, 1);
                CBNZx_MARK(xRCX);
                B_NEXT_nocond;
                MARK2;  // Part with DF==1
                LDRB_S9_postindex(x1, xRSI, -1);
                STRB_S9_postindex(x1, xRDI, -1);
                SUBx_U12(xRCX, xRCX, 1);
                CBNZx_MARK2(xRCX);
                // done
            } else {
                INST_NAME("MOVSB");
                GETDIR(x3, 1);
                LDRB_U12(x1, xRSI, 0);
                STRB_U12(x1, xRDI, 0);
                ADDx_REG(xRSI, xRSI, x3);
                ADDx_REG(xRDI, xRDI, x3);
            }
            SMWRITE();
            break;
        case 0xA5:
            SMREAD();
            if(rep) {
                INST_NAME("REP MOVSD");
                CBZx_NEXT(xRCX);
                TBNZ_MARK2(xFlags, F_DF);
                MARK;   // Part with DF==0
                LDRxw_S9_postindex(x1, xRSI, rex.w?8:4);
                STRxw_S9_postindex(x1, xRDI, rex.w?8:4);
                SUBx_U12(xRCX, xRCX, 1);
                CBNZx_MARK(xRCX);
                B_NEXT_nocond;
                MARK2;  // Part with DF==1
                LDRxw_S9_postindex(x1, xRSI, rex.w?-8:-4);
                STRxw_S9_postindex(x1, xRDI, rex.w?-8:-4);
                SUBx_U12(xRCX, xRCX, 1);
                CBNZx_MARK2(xRCX);
                // done
            } else {
                INST_NAME("MOVSD");
                GETDIR(x3, rex.w?8:4);
                LDRxw_U12(x1, xRSI, 0);
                STRxw_U12(x1, xRDI, 0);
                ADDx_REG(xRSI, xRSI, x3);
                ADDx_REG(xRDI, xRDI, x3);
            }
            SMWRITE();
            break;
        case 0xA6:
            switch(rep) {
            case 1:
            case 2:
                if(rep==1) {INST_NAME("REPNZ CMPSB");} else {INST_NAME("REPZ CMPSB");}
                MAYSETFLAGS();
                SMREAD();
                SETFLAGS(X_ALL, SF_SET_PENDING);
                CBZx_NEXT(xRCX);
                TBNZ_MARK2(xFlags, F_DF);
                MARK;   // Part with DF==0
                LDRB_S9_postindex(x1, xRSI, 1);
                LDRB_S9_postindex(x2, xRDI, 1);
                SUBx_U12(xRCX, xRCX, 1);
                CMPSw_REG(x1, x2);
                B_MARK3((rep==1)?cEQ:cNE);
                CBNZx_MARK(xRCX);
                B_MARK3_nocond;
                MARK2;  // Part with DF==1
                LDRB_S9_postindex(x1, xRSI, -1);
                LDRB_S9_postindex(x2, xRDI, -1);
                SUBx_U12(xRCX, xRCX, 1);
                CMPSw_REG(x1, x2);
                B_MARK3((rep==1)?cEQ:cNE);
                CBNZx_MARK2(xRCX);
                MARK3;  // end
                emit_cmp8(dyn, ninst, x1, x2, x3, x4, x5);
                break;
            default:
                INST_NAME("CMPSB");
                SETFLAGS(X_ALL, SF_SET_PENDING);
                GETDIR(x3, 1);
                SMREAD();
                LDRB_U12(x1, xRSI, 0);
                LDRB_U12(x2, xRDI, 0);
                ADDx_REG(xRSI, xRSI, x3);
                ADDx_REG(xRDI, xRDI, x3);
                emit_cmp8(dyn, ninst, x1, x2, x3, x4, x5);
                break;
            }
            break;
        case 0xA7:
            switch(rep) {
            case 1:
            case 2:
                if(rep==1) {INST_NAME("REPNZ CMPSD");} else {INST_NAME("REPZ CMPSD");}
                MAYSETFLAGS();
                SETFLAGS(X_ALL, SF_SET_PENDING);
                SMREAD();
                CBZx_NEXT(xRCX);
                TBNZ_MARK2(xFlags, F_DF);
                MARK;   // Part with DF==0
                LDRxw_S9_postindex(x1, xRSI, rex.w?8:4);
                LDRxw_S9_postindex(x2, xRDI, rex.w?8:4);
                SUBx_U12(xRCX, xRCX, 1);
                CMPSxw_REG(x1, x2);
                B_MARK3((rep==1)?cEQ:cNE);
                CBNZx_MARK(xRCX);
                B_MARK3_nocond;
                MARK2;  // Part with DF==1
                LDRxw_S9_postindex(x1, xRSI, rex.w?-8:-4);
                LDRxw_S9_postindex(x2, xRDI, rex.w?-8:-4);
                SUBx_U12(xRCX, xRCX, 1);
                CMPSxw_REG(x1, x2);
                B_MARK3((rep==1)?cEQ:cNE);
                CBNZx_MARK2(xRCX);
                MARK3;  // end
                emit_cmp32(dyn, ninst, rex, x1, x2, x3, x4, x5);
                break;
            default:
                INST_NAME("CMPSD");
                SETFLAGS(X_ALL, SF_SET_PENDING);
                GETDIR(x3, rex.w?8:4);
                SMREAD();
                LDRxw_U12(x1, xRSI, 0);
                LDRxw_U12(x2, xRDI, 0);
                ADDx_REG(xRSI, xRSI, x3);
                ADDx_REG(xRDI, xRDI, x3);
                emit_cmp32(dyn, ninst, rex, x1, x2, x3, x4, x5);
                break;
            }
            break;
        case 0xA8:
            INST_NAME("TEST AL, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            UXTBx(x1, xRAX);
            u8 = F8;
            MOV32w(x2, u8);
            emit_test8(dyn, ninst, x1, x2, x3, x4, x5);
            break;
        case 0xA9:
            INST_NAME("TEST EAX, Id");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i64 = F32S;
            MOV64xw(x2, i64);
            emit_test32(dyn, ninst, rex, xRAX, x2, x3, x4, x5);
            break;
        case 0xAA:
            WILLWRITE();
            if(rep) {
                INST_NAME("REP STOSB");
                CBZx_NEXT(xRCX);
                TBNZ_MARK2(xFlags, F_DF);
                MARK;   // Part with DF==0
                STRB_S9_postindex(xRAX, xRDI, 1);
                SUBx_U12(xRCX, xRCX, 1);
                CBNZx_MARK(xRCX);
                B_MARK3_nocond;
                MARK2;  // Part with DF==1
                STRB_S9_postindex(xRAX, xRDI, -1);
                SUBx_U12(xRCX, xRCX, 1);
                CBNZx_MARK2(xRCX);
                MARK3;
                // done
            } else {
                INST_NAME("STOSB");
                GETDIR(x3, 1);
                STRB_U12(xRAX, xRDI, 0);
                ADDx_REG(xRDI, xRDI, x3);
            }
            SMWRITE();
            break;
        case 0xAB:
            WILLWRITE();
            if(rep) {
                INST_NAME("REP STOSD");
                CBZx_NEXT(xRCX);
                TBNZ_MARK2(xFlags, F_DF);
                MARK;   // Part with DF==0
                STRxw_S9_postindex(xRAX, xRDI, rex.w?8:4);
                SUBx_U12(xRCX, xRCX, 1);
                CBNZx_MARK(xRCX);
                B_MARK3_nocond;
                MARK2;  // Part with DF==1
                STRxw_S9_postindex(xRAX, xRDI, rex.w?-8:-4);
                SUBx_U12(xRCX, xRCX, 1);
                CBNZx_MARK2(xRCX);
                MARK3;
                // done
            } else {
                INST_NAME("STOSD");
                GETDIR(x3, rex.w?8:4);
                STRxw_U12(xRAX, xRDI, 0);
                ADDx_REG(xRDI, xRDI, x3);
            }
            SMWRITE();
            break;
        case 0xAC:
            if(rep) {
                INST_NAME("REP LODSB");
            } else {
                INST_NAME("LODSB");
            }
            GETDIR(x1, 1);
            SMREAD();
            if(rep) {
                CBZx_NEXT(xRCX);
                MARK;
            }
            LDRB_U12(x2, xRSI, 0);
            ADDx_REG(xRSI, xRSI, x1);
            if(rep) {
                SUBx_U12(xRCX, xRCX, 1);
                CBNZx_MARK(xRCX);
            }
            BFIx(xRAX, x2, 0, 8);
            break;
        case 0xAD:
            if(rep) {
                INST_NAME("REP LODSD");
            } else {
                INST_NAME("LODSD");
            }
            GETDIR(x1, rex.w?8:4);
            if(rep) {
                CBZx_NEXT(xRCX);
                MARK;
            }
            LDRxw_U12(xRAX, xRSI, 0);
            ADDx_REG(xRSI, xRSI, x1);
            if(rep) {
                SUBx_U12(xRCX, xRCX, 1);
                CBNZx_MARK(xRCX);
            }
            break;
        case 0xAE:
            switch(rep) {
            case 1:
            case 2:
                if(rep==1) {INST_NAME("REPNZ SCASB");} else {INST_NAME("REPZ SCASB");}
                MAYSETFLAGS();
                SMREAD();
                SETFLAGS(X_ALL, SF_SET_PENDING);
                CBZx_NEXT(xRCX);
                UBFXw(x1, xRAX, 0, 8);
                TBNZ_MARK2(xFlags, F_DF);
                MARK;   // Part with DF==0
                LDRB_S9_postindex(x2, xRDI, 1);
                SUBx_U12(xRCX, xRCX, 1);
                CMPSw_REG(x1, x2);
                B_MARK3((rep==1)?cEQ:cNE);
                CBNZx_MARK(xRCX);
                B_MARK3_nocond;
                MARK2;  // Part with DF==1
                LDRB_S9_postindex(x2, xRDI, -1);
                SUBx_U12(xRCX, xRCX, 1);
                CMPSw_REG(x1, x2);
                B_MARK3((rep==1)?cEQ:cNE);
                CBNZx_MARK2(xRCX);
                MARK3;  // end
                emit_cmp8(dyn, ninst, x1, x2, x3, x4, x5);
                break;
            default:
                INST_NAME("SCASB");
                SETFLAGS(X_ALL, SF_SET_PENDING);
                GETDIR(x3, 1);
                UBFXw(x1, xRAX, 0, 8);
                LDRB_U12(x2, xRDI, 0);
                ADDx_REG(xRDI, xRDI, x3);
                emit_cmp8(dyn, ninst, x1, x2, x3, x4, x5);
                break;
            }
            break;
        case 0xAF:
            switch(rep) {
            case 1:
            case 2:
                if(rep==1) {INST_NAME("REPNZ SCASD");} else {INST_NAME("REPZ SCASD");}
                MAYSETFLAGS();
                SMREAD();
                SETFLAGS(X_ALL, SF_SET_PENDING);
                CBZx_NEXT(xRCX);
                TBNZ_MARK2(xFlags, F_DF);
                MARK;   // Part with DF==0
                LDRxw_S9_postindex(x2, xRDI, rex.w?8:4);
                SUBx_U12(xRCX, xRCX, 1);
                CMPSxw_REG(xRAX, x2);
                B_MARK3((rep==1)?cEQ:cNE);
                CBNZx_MARK(xRCX);
                B_MARK3_nocond;
                MARK2;  // Part with DF==1
                LDRxw_S9_postindex(x2, xRDI, rex.w?-8:-4);
                SUBx_U12(xRCX, xRCX, 1);
                CMPSxw_REG(xRAX, x2);
                B_MARK3((rep==1)?cEQ:cNE);
                CBNZx_MARK2(xRCX);
                MARK3;  // end
                emit_cmp32(dyn, ninst, rex, xRAX, x2, x3, x4, x5);
                break;
            default:
                INST_NAME("SCASD");
                SETFLAGS(X_ALL, SF_SET_PENDING);
                GETDIR(x3, rex.w?8:4);
                LDRxw_U12(x2, xRDI, 0);
                ADDx_REG(xRDI, xRDI, x3);
                emit_cmp32(dyn, ninst, rex, xRAX, x2, x3, x4, x5);
                break;
            }
            break;


        case 0xB0:
        case 0xB1:
        case 0xB2:
        case 0xB3:
            INST_NAME("MOV xL, Ib");
            u8 = F8;
            MOV32w(x1, u8);
            if(rex.rex)
                gb1 = xRAX+(opcode&7)+(rex.b<<3);
            else
                gb1 = xRAX+(opcode&3);
            BFIx(gb1, x1, 0, 8);
            break;
        case 0xB4:
        case 0xB5:
        case 0xB6:
        case 0xB7:
            INST_NAME("MOV xH, Ib");
            u8 = F8;
            MOV32w(x1, u8);
            if(rex.rex) {
                gb1 = xRAX+(opcode&7)+(rex.b<<3);
                BFIx(gb1, x1, 0, 8);
            } else {
                gb1 = xRAX+(opcode&3);
                BFIx(gb1, x1, 8, 8);
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
            INST_NAME("MOV Reg, Id");
            gd = xRAX+(opcode&7)+(rex.b<<3);
            if(rex.w) {
                u64 = F64;
                MOV64x(gd, u64);
            } else {
                u32 = F32;
                MOV32w(gd, u32);
            }
            break;
        case 0xC0:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("ROL Eb, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop)&0x1f;
                    if(u8) {
                        SETFLAGS(X_CF|X_OF, SF_SUBSET_PENDING);
                        GETEB(x1, 1);
                        u8 = F8&0x1f;
                        emit_rol8c(dyn, ninst, x1, u8, x4, x5);
                        EBBACK;
                    } else {
                        FAKEED;
                        F8;
                    }
                    break;
                case 1:
                    INST_NAME("ROR Eb, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop)&0x1f;
                    if(u8) {
                        SETFLAGS(X_CF|X_OF, SF_SUBSET_PENDING);
                        GETEB(x1, 1);
                        u8 = F8&0x1f;
                        emit_ror8c(dyn, ninst, x1, u8, x4, x5);
                        EBBACK;
                    } else {
                        FAKEED;
                        F8;
                    }
                    break;
                case 2:
                    INST_NAME("RCL Eb, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop)&0x1f;
                    if(u8) {
                        READFLAGS(X_CF);
                        SETFLAGS(X_OF|X_CF, SF_SUBSET_PENDING);
                        GETEB(x1, 1);
                        u8 = F8&0x1f;
                        emit_rcl8c(dyn, ninst, x1, u8, x4, x5);
                        EBBACK;
                    } else {
                        FAKEED;
                        F8;
                    }
                    break;
                case 3:
                    INST_NAME("RCR Eb, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop)&0x1f;
                    if(u8) {
                        READFLAGS(X_CF);
                        SETFLAGS(X_OF|X_CF, SF_SUBSET_PENDING);
                        GETEB(x1, 1);
                        u8 = F8&0x1f;
                        emit_rcr8c(dyn, ninst, x1, u8, x4, x5);
                        EBBACK;
                    } else {
                        FAKEED;
                        F8;
                    }
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Eb, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop)&0x1f;
                    if(u8) {
                        SETFLAGS(X_ALL, SF_SET_PENDING);
                        GETEB(x1, 1);
                        u8 = (F8)&0x1f;
                        emit_shl8c(dyn, ninst, ed, u8, x4, x5);
                        EBBACK;
                    } else {
                        FAKEED;
                        F8;
                    }
                    break;
                case 5:
                    INST_NAME("SHR Eb, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop)&0x1f;
                    if(u8) {
                        SETFLAGS(X_ALL, SF_SET_PENDING);
                        GETEB(x1, 1);
                        u8 = (F8)&0x1f;
                        emit_shr8c(dyn, ninst, ed, u8, x4, x5);
                        EBBACK;
                    } else {
                        FAKEED;
                        F8;
                    }
                    break;
                case 7:
                    INST_NAME("SAR Eb, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop)&0x1f;
                    if(u8) {
                        SETFLAGS(X_ALL, SF_SET_PENDING);
                        GETSEB(x1, 1);
                        u8 = (F8)&0x1f;
                        emit_sar8c(dyn, ninst, ed, u8, x4, x5);
                        EBBACK;
                    } else {
                        FAKEED;
                        F8;
                    }
                    break;
            }
            break;
        case 0xC1:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("ROL Ed, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop)&(0x1f+(rex.w*0x20));
                    if(u8) {
                        SETFLAGS(X_CF|X_OF, SF_SUBSET_PENDING);
                        GETED(1);
                        u8 = (F8)&(rex.w?0x3f:0x1f);
                        emit_rol32c(dyn, ninst, rex, ed, u8, x3, x4);
                        WBACK;
                    } else {
                        if(MODREG && ! rex.w && !rex.is32bits) {
                            GETED(1);
                            MOVw_REG(ed, ed);
                        } else {
                            FAKEED;
                        }
                        F8;
                    }
                    break;
                case 1:
                    INST_NAME("ROR Ed, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop)&(0x1f+(rex.w*0x20));
                    if(u8) {
                        SETFLAGS(X_CF|X_OF, SF_SUBSET_PENDING);
                        GETED(1);
                        u8 = (F8)&(rex.w?0x3f:0x1f);
                        emit_ror32c(dyn, ninst, rex, ed, u8, x3, x4);
                        WBACK;
                    } else {
                        if(MODREG && ! rex.w && !rex.is32bits) {
                            GETED(1);
                            MOVw_REG(ed, ed);
                        } else {
                            FAKEED;
                        }
                        F8;
                    }
                    break;
                case 2:
                    INST_NAME("RCL Ed, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop)&(0x1f+(rex.w*0x20));
                    if(u8) {
                        READFLAGS(X_CF);
                        SETFLAGS(X_CF|X_OF, SF_SUBSET); // removed PENDING on purpose
                        GETED(1);
                        u8 = (F8)&(rex.w?0x3f:0x1f);
                        emit_rcl32c(dyn, ninst, rex, ed, u8, x3, x4);
                        WBACK;
                    } else {
                        if(MODREG && ! rex.w && !rex.is32bits) {
                            GETED(1);
                            MOVw_REG(ed, ed);
                        } else {
                            FAKEED;
                        }
                        F8;
                    }
                    break;
                case 3:
                    INST_NAME("RCR Ed, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop)&(0x1f+(rex.w*0x20));
                    if(u8) {
                        READFLAGS(X_CF);
                        SETFLAGS(X_CF|X_OF, SF_SUBSET); // removed PENDING on purpose
                        GETED(1);
                        u8 = (F8)&(rex.w?0x3f:0x1f);
                        emit_rcr32c(dyn, ninst, rex, ed, u8, x3, x4);
                        WBACK;
                    } else {
                        if(MODREG && ! rex.w && !rex.is32bits) {
                            GETED(1);
                            MOVw_REG(ed, ed);
                        } else {
                            FAKEED;
                        }
                        F8;
                    }
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ed, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop)&(0x1f+(rex.w*0x20));
                    if(u8) {
                        SETFLAGS(X_ALL, SF_SET_PENDING);
                        GETED(1);
                        u8 = (F8)&(rex.w?0x3f:0x1f);
                        emit_shl32c(dyn, ninst, rex, ed, u8, x3, x4);
                        WBACK;
                    } else {
                        if(MODREG && ! rex.w && !rex.is32bits) {
                            GETED(1);
                            MOVw_REG(ed, ed);
                        } else {
                            FAKEED;
                        }
                        F8;
                    }
                    break;
                case 5:
                    INST_NAME("SHR Ed, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop)&(0x1f+(rex.w*0x20));
                    if(u8) {
                        SETFLAGS(X_ALL, SF_SET_PENDING);
                        GETED(1);
                        u8 = (F8)&(rex.w?0x3f:0x1f);
                        emit_shr32c(dyn, ninst, rex, ed, u8, x3, x4);
                        WBACK;
                    } else {
                        if(MODREG && ! rex.w && !rex.is32bits) {
                            GETED(1);
                            MOVw_REG(ed, ed);
                        } else {
                            FAKEED;
                        }
                        F8;
                    }
                    break;
                case 7:
                    INST_NAME("SAR Ed, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop)&(0x1f+(rex.w*0x20));
                    if(u8) {
                        SETFLAGS(X_ALL, SF_SET_PENDING);
                        GETED(1);
                        u8 = (F8)&(rex.w?0x3f:0x1f);
                        emit_sar32c(dyn, ninst, rex, ed, u8, x3, x4);
                        WBACK;
                    } else {
                        if(MODREG && ! rex.w && !rex.is32bits) {
                            GETED(1);
                            MOVw_REG(ed, ed);
                        } else {
                            FAKEED;
                        }
                        F8;
                    }
                    break;
            }
            break;
        case 0xC2:
            INST_NAME("RETN");
            //SETFLAGS(X_ALL, SF_SET_NODF);    // Hack, set all flags (to an unknown state...)
            if(box64_dynarec_safeflags) {
                READFLAGS(X_PEND);  // lets play safe here too
            }
            fpu_purgecache(dyn, ninst, 1, x1, x2, x3);  // using next, even if there no next
            i32 = F16;
            retn_to_epilog(dyn, ninst, rex, i32);
            *need_epilog = 0;
            *ok = 0;
            break;
        case 0xC3:
            INST_NAME("RET");
            // SETFLAGS(X_ALL, SF_SET_NODF);    // Hack, set all flags (to an unknown state...)
            if(box64_dynarec_safeflags) {
                READFLAGS(X_PEND);  // so instead, force the deferred flags, so it's not too slow, and flags are not lost
            }
            fpu_purgecache(dyn, ninst, 1, x1, x2, x3);  // using next, even if there no next
            ret_to_epilog(dyn, ninst, rex);
            *need_epilog = 0;
            *ok = 0;
            break;
        case 0xC4:
            nextop = F8;
            if(rex.is32bits && !(MODREG)) {
                INST_NAME("LES Gd, Ed");
                GETGD;
                addr = geted(dyn, addr, ninst, nextop, &wback, x1, &fixedaddress, &unscaled, 0, 0, rex, NULL, 0, 1);
                LDRw_U12(gd, wback, 0);
                LDRH_U12(x1, wback, 4);
                STRH_U12(x1, xEmu, offsetof(x64emu_t, segs[_ES]));
                STRw_U12(xZR, xEmu, offsetof(x64emu_t, segs_serial[_ES]));
            } else {
                DEFAULT;
            }
            break;
        case 0xC5:
            nextop = F8;
            if(rex.is32bits && !(MODREG)) {
                INST_NAME("LDS Gd, Ed");
                GETGD;
                addr = geted(dyn, addr, ninst, nextop, &wback, x1, &fixedaddress, &unscaled, 0, 0, rex, NULL, 0, 1);
                LDRw_U12(gd, wback, 0);
                LDRH_U12(x1, wback, 4);
                STRH_U12(x1, xEmu, offsetof(x64emu_t, segs[_DS]));
                STRw_U12(xZR, xEmu, offsetof(x64emu_t, segs_serial[_DS]));
            } else {
                DEFAULT;
            }
            break;
        case 0xC6:
            INST_NAME("MOV Eb, Ib");
            nextop=F8;
            if(MODREG) {   // reg <= u8
                u8 = F8;
                if(!rex.rex) {
                    ed = (nextop&7);
                    eb1 = xRAX+(ed&3);  // Ax, Cx, Dx or Bx
                    eb2 = (ed&4)>>2;    // L or H
                } else {
                    eb1 = xRAX+(nextop&7)+(rex.b<<3);
                    eb2 = 0;
                }
                MOV32w(x3, u8);
                BFIx(eb1, x3, eb2*8, 8);
            } else {                    // mem <= u8
                addr = geted(dyn, addr, ninst, nextop, &wback, x1, &fixedaddress, &unscaled, 0xfff, 0, rex, &lock, 0, 1);
                u8 = F8;
                if(u8) {
                    MOV32w(x3, u8);
                    ed = x3;
                } else
                    ed = xZR;
                WILLWRITELOCK(lock);
                STB(ed, wback, fixedaddress);
                SMWRITELOCK(lock);
            }
            break;
        case 0xC7:
            INST_NAME("MOV Ed, Id");
            nextop=F8;
            if(MODREG) {   // reg <= i32
                i64 = F32S;
                ed = xRAX+(nextop&7)+(rex.b<<3);
                MOV64xw(ed, i64);
            } else {                    // mem <= i32
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, &unscaled, 0xfff<<(2+rex.w), (1<<(2+rex.w))-1, rex, &lock, 0, 4);
                i64 = F32S;
                if(i64) {
                    MOV64xw(x3, i64);
                    ed = x3;
                } else
                    ed = xZR;
                WILLWRITELOCK(lock);
                STxw(ed, wback, fixedaddress);
                SMWRITELOCK(lock);
            }
            break;
        case 0xC8:
            INST_NAME("ENTER Iw,Ib");
            u16 = F16;
            u8 = (F8) & 0x1f;
            if(u8) {
                MOVx_REG(x1, xRBP);
            }
            PUSH1z(xRBP);
            MOVx_REG(xRBP, xRSP);
            if (u8) {
                for (u32 = 1; u32 < u8; u32++) {
                    SUBx_U12(x1, x1, rex.is32bits?4:8);
                    LDRz_U12(x2, x1, 0);
                    PUSH1z(x2);
                }
                PUSH1z(xRBP);
            }
            if(u16<4096) {
                SUBx_U12(xRSP, xRSP, u16);
            } else {
                MOV32w(x2, u16);
                SUBx_REG(xRSP, xRSP, x2);
            }
            break;
        case 0xC9:
            INST_NAME("LEAVE");
            MOVz_REG(xRSP, xRBP);
            POP1z(xRBP);
            break;

        case 0xCC:
            SETFLAGS(X_ALL, SF_SET_NODF);    // Hack, set all flags (to an unknown state...)
            NOTEST(x1);
            if(PK(0)=='S' && PK(1)=='C') {
                addr+=2;
                //BARRIER(BARRIER_FLOAT);
                INST_NAME("Special Box64 instruction");
                if((PK64(0)==0))
                {
                    addr+=8;
                    MESSAGE(LOG_DEBUG, "Exit x64 Emu\n");
                    //GETIP(ip+1+2);    // no use
                    //STORE_XEMU_REGS(xRIP);    // no need, done in epilog
                    MOV32w(x1, 1);
                    STRw_U12(x1, xEmu, offsetof(x64emu_t, quit));
                    *ok = 0;
                    *need_epilog = 1;
                } else {
                    MESSAGE(LOG_DUMP, "Native Call to %s\n", GetNativeName(GetNativeFnc(ip)));
                    x87_stackcount(dyn, ninst, x1);
                    x87_forget(dyn, ninst, x3, x4, 0);
                    sse_purge07cache(dyn, ninst, x3);
                    SMEND();
                    tmp = isSimpleWrapper(*(wrapper_t*)(addr));
                    if(isRetX87Wrapper(*(wrapper_t*)(addr)))
                        // return value will be on the stack, so the stack depth needs to be updated
                        x87_purgecache(dyn, ninst, 0, x3, x1, x4);
                    if((box64_log<2 && !cycle_log) && tmp) {
                        //GETIP(ip+3+8+8); // read the 0xCC
                        call_n(dyn, ninst, *(void**)(addr+8), tmp);
                        addr+=8+8;
                    } else {
                        WILLWRITE2();
                        GETIP(ip+1); // read the 0xCC
                        STORE_XEMU_CALL(xRIP);
                        ADDx_U12(x1, xEmu, (uint32_t)offsetof(x64emu_t, ip)); // setup addr as &emu->ip
                        CALL_S(x64Int3, -1);
                        SMWRITE2();
                        LOAD_XEMU_CALL(xRIP);
                        addr+=8+8;
                        TABLE64(x3, addr); // expected return address
                        CMPSx_REG(xRIP, x3);
                        B_MARK(cNE);
                        LDRw_U12(w1, xEmu, offsetof(x64emu_t, quit));
                        CBZw_NEXT(w1);
                        MARK;
                        LOAD_XEMU_REM();
                        jump_to_epilog(dyn, 0, xRIP, ninst);
                    }
                }
            } else {
                INST_NAME("INT 3");
                if(!box64_ignoreint3) {
                    // check if TRAP signal is handled
                    TABLE64(x1, (uintptr_t)my_context);
                    MOV32w(x2, offsetof(box64context_t, signals[SIGTRAP]));
                    LDRx_REG(x3, x1, x2);
                    //LDRx_U12(x3, x1, offsetof(box64context_t, signals[SIGTRAP]));
                    CMPSx_U12(x3, 0);
                    B_NEXT(cEQ);
                    GETIP(ip);
                    STORE_XEMU_CALL(xRIP);
                    CALL(native_int3, -1);
                    LOAD_XEMU_CALL(xRIP);
                    *need_epilog = 0;
                    *ok = 0;
                }
                break;
            }
            break;
        case 0xCD:
            u8 = F8;
            if(box64_wine && (u8==0x2D || u8==0x2C || u8==0x29)) {
                INST_NAME("INT 29/2c/2d");
                // lets do nothing
                MESSAGE(LOG_INFO, "INT 29/2c/2d Windows interruption\n");
                GETIP(ip);
                STORE_XEMU_CALL(xRIP);
                MOV32w(x1, u8);
                CALL(native_int, -1);
                LOAD_XEMU_CALL(xRIP);
            } else if (u8==0x80) {
                INST_NAME("32bits SYSCALL");
                NOTEST(x1);
                SMEND();
                GETIP(addr);
                STORE_XEMU_CALL(xRIP);
                CALL_S(x86Syscall, -1);
                LOAD_XEMU_CALL(xRIP);
                TABLE64(x3, addr); // expected return address
                CMPSx_REG(xRIP, x3);
                B_MARK(cNE);
                LDRw_U12(w1, xEmu, offsetof(x64emu_t, quit));
                CBZw_NEXT(w1);
                MARK;
                LOAD_XEMU_REM();
                jump_to_epilog(dyn, 0, xRIP, ninst);
            } else {
                INST_NAME("INT n");
                SETFLAGS(X_ALL, SF_SET_NODF);    // Hack to set flags in "don't care" state
                GETIP(ip);
                STORE_XEMU_CALL(xRIP);
                CALL(native_int, -1);
                LOAD_XEMU_CALL(xRIP);
                jump_to_epilog(dyn, 0, xRIP, ninst);
                *need_epilog = 0;
                *ok = 0;
            }
            break;
        case 0xCE:
            if(!rex.is32bits) {
                DEFAULT;
            } else {
                INST_NAME("INTO");
                READFLAGS(X_OF);
                GETIP(ip);
                TBZ_NEXT(wFlags, F_OF);
                STORE_XEMU_CALL(xRIP);
                CALL(native_int, -1);
                LOAD_XEMU_CALL(xRIP);
            }
            break;
        case 0xCF:
            INST_NAME("IRET");
            SETFLAGS(X_ALL, SF_SET_NODF);    // Not a hack, EFLAGS are restored
            BARRIER(BARRIER_FLOAT);
            iret_to_epilog(dyn, ninst, rex.w);
            *need_epilog = 0;
            *ok = 0;
            break;
        case 0xD0:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("ROL Eb, 1");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET_PENDING);
                    GETEB(x1, 0);
                    emit_rol8c(dyn, ninst, ed, 1, x4, x5);
                    EBBACK;
                    break;
                case 1:
                    INST_NAME("ROR Eb, 1");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET_PENDING);
                    GETEB(x1, 0);
                    emit_ror8c(dyn, ninst, ed, 1, x4, x5);
                    EBBACK;
                    break;
                case 2:
                    INST_NAME("RCL Eb, 1");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SUBSET); // removed PENDING on purpose
                    GETEB(x1, 0);
                    emit_rcl8c(dyn, ninst, ed, 1, x4, x5);
                    EBBACK;
                    break;
                case 3:
                    INST_NAME("RCR Eb, 1");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SUBSET); // removed PENDING on purpose
                    GETEB(x1, 0);
                    emit_rcr8c(dyn, ninst, ed, 1, x4, x5);
                    EBBACK;
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Eb, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    GETEB(x1, 0);
                    emit_shl8c(dyn, ninst, ed, 1, x4, x5);
                    EBBACK;
                    break;
                case 5:
                    INST_NAME("SHR Eb, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    GETEB(x1, 0);
                    emit_shr8c(dyn, ninst, ed, 1, x4, x5);
                    EBBACK;
                    break;
                case 7:
                    INST_NAME("SAR Eb, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    GETSEB(x1, 0);
                    emit_sar8c(dyn, ninst, ed, 1, x4, x5);
                    EBBACK;
                    break;
            }
            break;
        case 0xD1:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("ROL Ed, 1");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET_PENDING);
                    GETED(0);
                    emit_rol32c(dyn, ninst, rex, ed, 1, x3, x4);
                    WBACK;
                    break;
                case 1:
                    INST_NAME("ROR Ed, 1");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET_PENDING);
                    GETED(0);
                    emit_ror32c(dyn, ninst, rex, ed, 1, x3, x4);
                    WBACK;
                    break;
                case 2:
                    INST_NAME("RCL Ed, 1");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SUBSET); // removed PENDING on purpose
                    GETED(0);
                    emit_rcl32c(dyn, ninst, rex, ed, 1, x3, x4);
                    WBACK;
                    break;
                case 3:
                    INST_NAME("RCR Ed, 1");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SUBSET); // removed PENDING on purpose
                    GETED(0);
                    emit_rcr32c(dyn, ninst, rex, ed, 1, x3, x4);
                    WBACK;
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ed, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    GETED(0);
                    emit_shl32c(dyn, ninst, rex, ed, 1, x3, x4);
                    WBACK;
                    break;
                case 5:
                    INST_NAME("SHR Ed, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    GETED(0);
                    emit_shr32c(dyn, ninst, rex, ed, 1, x3, x4);
                    WBACK;
                    break;
                case 7:
                    INST_NAME("SAR Ed, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    GETED(0);
                    emit_sar32c(dyn, ninst, rex, ed, 1, x3, x4);
                    WBACK;
                    break;
            }
            break;
        case 0xD2:  // TODO: Jump if CL is 0
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("ROL Eb, CL");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    if(box64_dynarec_safeflags>1)
                        MAYSETFLAGS();
                    UFLAG_IF {
                        TSTw_mask(xRCX, 0, 0b00100);  //mask=0x00000001f
                        B_NEXT(cEQ);
                    }
                    ANDw_mask(x2, xRCX, 0, 0b00010);  //mask=0x000000007
                    MOV32w(x4, 8);
                    SUBx_REG(x2, x4, x2);
                    GETEB(x1, 0);
                    ORRw_REG_LSL(ed, ed, ed, 8);
                    LSRw_REG(ed, ed, x2);
                    EBBACK;
                    UFLAG_IF {  // calculate flags directly
                        CMPSw_U12(x2, 7);
                        B_MARK(cNE);
                            ADDxw_REG_LSR(x3, ed, ed, 7);
                            BFIw(xFlags, x3, F_OF, 1);
                        MARK;
                        BFIw(xFlags, ed, F_CF, 1);
                        UFLAG_DF(x2, d_none);
                    }
                    break;
                case 1:
                    INST_NAME("ROR Eb, CL");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    if(box64_dynarec_safeflags>1)
                        MAYSETFLAGS();
                    UFLAG_IF {
                        TSTw_mask(xRCX, 0, 0b00100);  //mask=0x00000001f
                        B_NEXT(cEQ);
                    }
                    ANDw_mask(x2, xRCX, 0, 0b00010);  //mask=0x000000007
                    GETEB(x1, 0);
                    ORRw_REG_LSL(ed, ed, ed, 8);
                    LSRw_REG(ed, ed, x2);
                    EBBACK;
                    UFLAG_IF {  // calculate flags directly
                        CMPSw_U12(x2, 1);
                        B_MARK(cNE);
                            LSRxw(x2, ed, 6); // x2 = d>>30
                            EORw_REG_LSR(x2, x2, x2, 1); // x2 = ((d>>30) ^ ((d>>30)>>1))
                            BFIw(xFlags, x2, F_OF, 1);
                        MARK;
                        BFXILw(xFlags, ed, 7, 1);
                        UFLAG_DF(x2, d_none);
                    }
                    break;
                case 2:
                    INST_NAME("RCL Eb, CL");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    READFLAGS(X_CF);
                    if(box64_dynarec_safeflags>1)
                        MAYSETFLAGS();
                    SETFLAGS(X_OF|X_CF, SF_SET_DF);
                    ANDSw_mask(x2, xRCX, 0, 0b00100);
                    GETEB(x1, 0);
                    CALL_(rcl8, x1, x3);
                    EBBACK;
                    break;
                case 3:
                    INST_NAME("RCR Eb, CL");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    READFLAGS(X_CF);
                    if(box64_dynarec_safeflags>1)
                        MAYSETFLAGS();
                    SETFLAGS(X_OF|X_CF, SF_SET_DF);
                    ANDSw_mask(x2, xRCX, 0, 0b00100);
                    GETEB(x1, 0);
                    CALL_(rcr8, x1, x3);
                    EBBACK;
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Eb, CL");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    if(box64_dynarec_safeflags>1)
                        MAYSETFLAGS();
                    UFLAG_IF {
                        ANDSw_mask(x2, xRCX, 0, 0b00100);  //mask=0x00000001f
                        B_NEXT(cEQ);
                    } else {
                        ANDw_mask(x2, xRCX, 0, 0b00100);  //mask=0x00000001f
                    }
                    GETEB(x1, 0);
                    emit_shl8(dyn, ninst, x1, x2, x5, x4);
                    EBBACK;
                    break;
                case 5:
                    INST_NAME("SHR Eb, CL");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    if(box64_dynarec_safeflags>1)
                        MAYSETFLAGS();
                    UFLAG_IF {
                        ANDSw_mask(x2, xRCX, 0, 0b00100);  //mask=0x00000001f
                        B_NEXT(cEQ);
                    } else {
                        ANDw_mask(x2, xRCX, 0, 0b00100);  //mask=0x00000001f
                    }
                    GETEB(x1, 0);
                    emit_shr8(dyn, ninst, x1, x2, x5, x4);
                    EBBACK;
                    break;
                case 7:
                    INST_NAME("SAR Eb, CL");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    if(box64_dynarec_safeflags>1)
                        MAYSETFLAGS();
                    UFLAG_IF {
                        ANDSw_mask(x2, xRCX, 0, 0b00100);  //mask=0x00000001f
                        B_NEXT(cEQ);
                    } else {
                        ANDw_mask(x2, xRCX, 0, 0b00100);  //mask=0x00000001f
                    }
                    GETSEB(x1, 0);
                    emit_sar8(dyn, ninst, x1, x2, x5, x4);
                    EBBACK;
                    break;
            }
            break;
        case 0xD3:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("ROL Ed, CL");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    if(box64_dynarec_safeflags>1)
                        MAYSETFLAGS();
                    UFLAG_IF {
                        if(rex.w) {
                            ANDSx_mask(x3, xRCX, 1, 0, 0b00101);  //mask=0x000000000000003f
                        } else {
                            ANDSw_mask(x3, xRCX, 0, 0b00100);  //mask=0x00000001f
                        }
                    } else {
                        if(rex.w) {
                            ANDx_mask(x3, xRCX, 1, 0, 0b00101);  //mask=0x000000000000003f
                        } else {
                            ANDw_mask(x3, xRCX, 0, 0b00100);  //mask=0x00000001f
                        }
                    }
                    MOV64xw(x4, (rex.w?64:32));
                    SUBx_REG(x3, x4, x3);
                    GETED(0);
                    UFLAG_IF {
                        if(!rex.w && !rex.is32bits && MODREG) {MOVw_REG(ed, ed);}
                        B_NEXT(cEQ);
                    }
                    RORxw_REG(ed, ed, x3);
                    WBACK;
                    UFLAG_IF {  // calculate flags directly
                        CMPSw_U12(x3, rex.w?63:31);
                        B_MARK(cNE);
                            ADDxw_REG_LSR(x1, ed, ed, rex.w?63:31);
                            BFIw(xFlags, x1, F_OF, 1);
                        MARK;
                        BFIw(xFlags, ed, F_CF, 1);
                        UFLAG_DF(x2, d_none);
                    }
                    break;
                case 1:
                    INST_NAME("ROR Ed, CL");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    if(box64_dynarec_safeflags>1)
                        MAYSETFLAGS();
                    UFLAG_IF {
                        if(rex.w) {
                            ANDSx_mask(x3, xRCX, 1, 0, 0b00101);  //mask=0x000000000000003f
                        } else {
                            ANDSw_mask(x3, xRCX, 0, 0b00100);  //mask=0x00000001f
                        }
                    } else {
                        if(rex.w) {
                            ANDx_mask(x3, xRCX, 1, 0, 0b00101);  //mask=0x000000000000003f
                        } else {
                            ANDw_mask(x3, xRCX, 0, 0b00100);  //mask=0x00000001f
                        }
                    }
                    GETED(0);
                    UFLAG_IF {
                        if(!rex.w && !rex.is32bits && MODREG) {MOVw_REG(ed, ed);}
                        B_NEXT(cEQ);
                    }
                    RORxw_REG(ed, ed, x3);
                    WBACK;
                    UFLAG_IF {  // calculate flags directly
                        CMPSw_U12(x3, 1);
                        B_MARK(cNE);
                            LSRxw(x2, ed, rex.w?62:30); // x2 = d>>30
                            EORw_REG_LSR(x2, x2, x2, 1); // x2 = ((d>>30) ^ ((d>>30)>>1))
                            BFIw(xFlags, x2, F_OF, 1);
                        MARK;
                        BFXILxw(xFlags, ed, rex.w?63:31, 1);
                        UFLAG_DF(x2, d_none);
                    }
                    break;
                case 2:
                    INST_NAME("RCL Ed, CL");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET_DF);
                    if(box64_dynarec_safeflags>1)
                        MAYSETFLAGS();
                    if(rex.w) {
                        ANDSx_mask(x2, xRCX, 1, 0, 0b00101);  //mask=0x000000000000003f
                    } else {
                        ANDSw_mask(x2, xRCX, 0, 0b00100);  //mask=0x00000001f
                    }
                    GETEDW(x4, x1, 0);
                    if(!rex.w && !rex.is32bits && MODREG) {MOVw_REG(ed, ed);}
                    B_NEXT(cEQ);
                    CALL_(rex.w?((void*)rcl64):((void*)rcl32), ed, x4);
                    WBACK;
                    break;
                case 3:
                    INST_NAME("RCR Ed, CL");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET_DF);
                    if(box64_dynarec_safeflags>1)
                        MAYSETFLAGS();
                    if(rex.w) {
                        ANDSx_mask(x2, xRCX, 1, 0, 0b00101);  //mask=0x000000000000003f
                    } else {
                        ANDSw_mask(x2, xRCX, 0, 0b00100);  //mask=0x00000001f
                    }
                    GETEDW(x4, x1, 0);
                    if(!rex.w && !rex.is32bits && MODREG) {MOVw_REG(ed, ed);}
                    B_NEXT(cEQ);
                    CALL_(rex.w?((void*)rcr64):((void*)rcr32), ed, x4);
                    WBACK;
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ed, CL");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    if(box64_dynarec_safeflags>1)
                        MAYSETFLAGS();
                    UFLAG_IF {
                        if(rex.w) {
                            ANDSx_mask(x3, xRCX, 1, 0, 0b00101);  //mask=0x000000000000003f
                        } else {
                            ANDSw_mask(x3, xRCX, 0, 0b00100);  //mask=0x00000001f
                        }
                    } else {
                        if(rex.w) {
                            ANDx_mask(x3, xRCX, 1, 0, 0b00101);  //mask=0x000000000000003f
                        } else {
                            ANDw_mask(x3, xRCX, 0, 0b00100);  //mask=0x00000001f
                        }
                    }
                    GETED(0);
                    UFLAG_IF {
                        if(!rex.w && !rex.is32bits && MODREG) {MOVw_REG(ed, ed);}
                        B_NEXT(cEQ);
                    }
                    emit_shl32(dyn, ninst, rex, ed, x3, x5, x4);
                    WBACK;
                    break;
                case 5:
                    INST_NAME("SHR Ed, CL");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    if(box64_dynarec_safeflags>1)
                        MAYSETFLAGS();
                    UFLAG_IF {
                        if(rex.w) {
                            ANDSx_mask(x3, xRCX, 1, 0, 0b00101);  //mask=0x000000000000003f
                        } else {
                            ANDSw_mask(x3, xRCX, 0, 0b00100);  //mask=0x00000001f
                        }
                    } else {
                        if(rex.w) {
                            ANDx_mask(x3, xRCX, 1, 0, 0b00101);  //mask=0x000000000000003f
                        } else {
                            ANDw_mask(x3, xRCX, 0, 0b00100);  //mask=0x00000001f
                        }
                    }
                    GETED(0);
                    UFLAG_IF {
                        if(!rex.w && !rex.is32bits && MODREG) {MOVw_REG(ed, ed);}
                        B_NEXT(cEQ);
                    }
                    emit_shr32(dyn, ninst, rex, ed, x3, x5, x4);
                    WBACK;
                    break;
                case 7:
                    INST_NAME("SAR Ed, CL");
                    SETFLAGS(X_ALL, SF_PENDING);
                    if(box64_dynarec_safeflags>1)
                        MAYSETFLAGS();
                    UFLAG_IF {
                        if(rex.w) {
                            ANDSx_mask(x3, xRCX, 1, 0, 0b00101);  //mask=0x000000000000003f
                        } else {
                            ANDSw_mask(x3, xRCX, 0, 0b00100);  //mask=0x00000001f
                        }
                    } else {
                        if(rex.w) {
                            ANDx_mask(x3, xRCX, 1, 0, 0b00101);  //mask=0x000000000000003f
                        } else {
                            ANDw_mask(x3, xRCX, 0, 0b00100);  //mask=0x00000001f
                        }
                    }
                    GETED(0);
                    UFLAG_IF {
                        if(!rex.w && !rex.is32bits && MODREG) {MOVw_REG(ed, ed);}
                        B_NEXT(cEQ);
                    }
                    UFLAG_OP12(ed, x3);
                    ASRxw_REG(ed, ed, x3);
                    WBACK;
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, rex.w?d_sar64:d_sar32);
                    break;
            }
            break;
        case 0xD4:
            if(rex.is32bits) {
                INST_NAME("AAM Ib");
                SETFLAGS(X_ALL, SF_SET_DF);
                UBFXx(x1, xRAX, 0, 8);    // load AL
                u8 = F8;
                MOV32w(x2, u8);
                CALL_(aam16, x1, 0);
                BFIz(xRAX, x1, 0, 16);
            } else {
                DEFAULT;
            }
            break;
        case 0xD5:
            if(rex.is32bits) {
                INST_NAME("AAD Ib");
                SETFLAGS(X_ALL, SF_SET_DF);
                UBFXx(x1, xRAX, 0, 16);    // load AX
                u8 = F8;
                MOV32w(x2, u8);
                CALL_(aad16, x1, 0);
                BFIz(xRAX, x1, 0, 16);
            } else {
                DEFAULT;
            }
            break;

        case 0xD7:
            INST_NAME("XLAT");
            UXTBw(x1, xRAX);
            LDRB_REG(x1, xRBX, x1);
            BFIx(xRAX, x1, 0, 8);
            break;
        case 0xD8:
            addr = dynarec64_D8(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
            break;
        case 0xD9:
            addr = dynarec64_D9(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
            break;
        case 0xDA:
            addr = dynarec64_DA(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
            break;
        case 0xDB:
            addr = dynarec64_DB(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
            break;
        case 0xDC:
            addr = dynarec64_DC(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
            break;
        case 0xDD:
            addr = dynarec64_DD(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
            break;
        case 0xDE:
            addr = dynarec64_DE(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
            break;
        case 0xDF:
            addr = dynarec64_DF(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
            break;
        #define GO(Z)                                                   \
            BARRIER(BARRIER_MAYBE);                                     \
            JUMP(addr+i8, 1);                                           \
            if(dyn->insts[ninst].x64.jmp_insts==-1 ||                   \
                CHECK_CACHE()) {                                        \
                /* out of the block */                                  \
                i32 = dyn->insts[ninst].epilog-(dyn->native_size);      \
                if(Z) {CBNZz(xRCX, i32);} else {CBZz(xRCX, i32);};      \
                if(dyn->insts[ninst].x64.jmp_insts==-1) {               \
                    if(!(dyn->insts[ninst].x64.barrier&BARRIER_FLOAT))  \
                        fpu_purgecache(dyn, ninst, 1, x1, x2, x3);      \
                    jump_to_next(dyn, addr+i8, 0, ninst, rex.is32bits); \
                } else {                                                \
                    CacheTransform(dyn, ninst, cacheupd, x1, x2, x3);   \
                    i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address-(dyn->native_size);    \
                    Bcond(c__, i32);                                    \
                }                                                       \
            } else {                                                    \
                /* inside the block */                                  \
                i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address-(dyn->native_size);    \
                if(Z) {CBZz(xRCX, i32);} else {CBNZz(xRCX, i32);};      \
            }
        case 0xE0:
            INST_NAME("LOOPNZ");
            READFLAGS(X_ZF);
            i8 = F8S;
            SUBz_U12(xRCX, xRCX, 1);
            TBNZ_NEXT(xFlags, F_ZF);
            GO(0);
            break;
        case 0xE1:
            INST_NAME("LOOPZ");
            READFLAGS(X_ZF);
            i8 = F8S;
            SUBz_U12(xRCX, xRCX, 1);
            TBZ_NEXT(xFlags, F_ZF);
            GO(0);
            break;
        case 0xE2:
            INST_NAME("LOOP");
            i8 = F8S;
            SUBz_U12(xRCX, xRCX, 1);
            GO(0);
            break;
        case 0xE3:
            INST_NAME("JECXZ");
            i8 = F8S;
            GO(1);
            break;
        #undef GO
        case 0xE4:                      /* IN AL, Ib */
        case 0xE5:                      /* IN EAX, Ib */
        case 0xE6:                      /* OUT Ib, AL */
        case 0xE7:                      /* OUT Ib, EAX */
            INST_NAME(opcode==0xE4?"IN AL, Ib":(opcode==0xE5?"IN EAX, Ib":(opcode==0xE6?"OUT Ib, AL":"OUT Ib, EAX")));
            SETFLAGS(X_ALL, SF_SET_NODF);    // Hack to set flags in "don't care" state
            u8 = F8;
            GETIP(ip);
            STORE_XEMU_CALL(xRIP);
            CALL(native_priv, -1);
            LOAD_XEMU_CALL(xRIP);
            jump_to_epilog(dyn, 0, xRIP, ninst);
            *need_epilog = 0;
            *ok = 0;
            break;
        case 0xE8:
            INST_NAME("CALL Id");
            i32 = F32S;
            if(addr+i32==0) {
                #if STEP == 3
                printf_log(LOG_INFO, "Warning, CALL to 0x0 at %p (%p)\n", (void*)addr, (void*)(addr-1));
                #endif
            }
            #if STEP < 2
            if(!rex.is32bits && isNativeCall(dyn, addr+i32, &dyn->insts[ninst].natcall, &dyn->insts[ninst].retn))
                tmp = dyn->insts[ninst].pass2choice = 3;
            else
                tmp = dyn->insts[ninst].pass2choice = 0;
            #else
                tmp = dyn->insts[ninst].pass2choice;
            #endif
            switch(tmp) {
                case 3:
                    SETFLAGS(X_ALL, SF_SET_NODF);    // Hack to set flags to "dont'care" state
                    if(dyn->last_ip && (addr-dyn->last_ip<0x1000)) {
                        ADDx_U12(x2, xRIP, addr-dyn->last_ip);
                    } else {
                        TABLE64(x2, addr);
                    }
                    WILLWRITE2();
                    PUSH1(x2);
                    MESSAGE(LOG_DUMP, "Native Call to %s (retn=%d)\n", GetNativeName(GetNativeFnc(dyn->insts[ninst].natcall-1)), dyn->insts[ninst].retn);
                    SKIPTEST(x1);    // disable test as this hack dos 2 instructions for 1
                    // calling a native function
                    SMEND();
                    sse_purge07cache(dyn, ninst, x3);
                    if((box64_log<2 && !cycle_log) && dyn->insts[ninst].natcall) {
                        tmp=isSimpleWrapper(*(wrapper_t*)(dyn->insts[ninst].natcall+2));
                    } else
                        tmp=0;
                    if(dyn->insts[ninst].natcall && isRetX87Wrapper(*(wrapper_t*)(dyn->insts[ninst].natcall+2)))
                    // return value will be on the stack, so the stack depth needs to be updated
                        x87_purgecache(dyn, ninst, 0, x3, x1, x4);
                    if((box64_log<2 && !cycle_log) && dyn->insts[ninst].natcall && tmp) {
                        //GETIP(ip+3+8+8); // read the 0xCC
                        call_n(dyn, ninst, *(void**)(dyn->insts[ninst].natcall+2+8), tmp);
                        SMWRITE2();
                        POP1(xRIP);   // pop the return address
                        dyn->last_ip = addr;
                    } else {
                        GETIP_(dyn->insts[ninst].natcall); // read the 0xCC already
                        STORE_XEMU_CALL(xRIP);
                        ADDx_U12(x1, xEmu, (uint32_t)offsetof(x64emu_t, ip)); // setup addr as &emu->ip
                        CALL_S(x64Int3, -1);
                        SMWRITE2();
                        LOAD_XEMU_CALL(xRIP);
                        TABLE64(x3, dyn->insts[ninst].natcall);
                        ADDx_U12(x3, x3, 2+8+8);
                        CMPSx_REG(xRIP, x3);
                        B_MARK(cNE);    // Not the expected address, exit dynarec block
                        POP1(xRIP);   // pop the return address
                        if(dyn->insts[ninst].retn) {
                            ADDx_U12(xRSP, xRSP, dyn->insts[ninst].retn);
                        }
                        TABLE64(x3, addr);
                        CMPSx_REG(xRIP, x3);
                        B_MARK(cNE);    // Not the expected address again
                        LDRw_U12(w1, xEmu, offsetof(x64emu_t, quit));
                        CBZw_NEXT(w1);  // not quitting, so lets continue
                        MARK;
                        LOAD_XEMU_REM();    // load remaining register, has they have changed
                        jump_to_epilog(dyn, 0, xRIP, ninst);
                        dyn->last_ip = addr;
                    }
                    break;
                default:
                    if((box64_dynarec_safeflags>1) || (ninst && dyn->insts[ninst-1].x64.set_flags)) {
                        READFLAGS(X_PEND);  // that's suspicious
                    } else {
                        SETFLAGS(X_ALL, SF_SET_NODF);    // Hack to set flags to "dont'care" state
                    }
                    // regular call
                    /*if(box64_dynarec_callret && box64_dynarec_bigblock>1) {
                        BARRIER(BARRIER_FULL);
                        BARRIER_NEXT(BARRIER_FULL);
                    } else {
                        BARRIER(BARRIER_FLOAT);
                        *need_epilog = 0;
                        *ok = 0;
                    }*/
                    if(rex.is32bits) {
                        MOV32w(x2, addr);
                    } else {
                        TABLE64(x2, addr);
                    }
                    fpu_purgecache(dyn, ninst, 1, x1, x3, x4);
                    PUSH1z(x2);
                    if(box64_dynarec_callret) {
                        SET_HASCALLRET();
                        // Push actual return address
                        if(addr < (dyn->start+dyn->isize)) {
                            // there is a next...
                            j64 = (dyn->insts)?(dyn->insts[ninst].epilog-(dyn->native_size)):0;
                            ADR_S20(x4, j64);
                            MESSAGE(LOG_NONE, "\tCALLRET set return to +%di\n", j64>>2);
                        } else {
                            MESSAGE(LOG_NONE, "\tCALLRET set return to Jmptable(%p)\n", (void*)addr);
                            j64 = getJumpTableAddress64(addr);
                            TABLE64(x4, j64);
                            LDRx_U12(x4, x4, 0);
                        }
                        STPx_S7_preindex(x4, x2, xSP, -16);
                    } else {
                        *ok = 0;
                        *need_epilog = 0;
                    }
                    jump_to_next(dyn, addr+i32, 0, ninst, rex.is32bits);
                    break;
            }
            break;
        case 0xE9:
        case 0xEB:
            BARRIER(BARRIER_MAYBE);
            if (box64_dynarec_test == 2) { NOTEST(x1); }
            if(opcode==0xEB && PK(0)==0xFF) {
                INST_NAME("JMP ib");
                MESSAGE(LOG_DEBUG, "Hack for EB FF opcode");
                NOP;
            } else {
                if(opcode==0xE9) {
                    INST_NAME("JMP Id");
                    i32 = F32S;
                } else {
                    INST_NAME("JMP Ib");
                    i32 = F8S;
                }
                if(rex.is32bits)
                    j64 = (uint32_t)(addr+i32);
                else
                    j64 = addr+i32;

                JUMP((uintptr_t)getAlternate((void*)j64), 0);
                if(dyn->insts[ninst].x64.jmp_insts==-1) {
                    // out of the block
                    fpu_purgecache(dyn, ninst, 1, x1, x2, x3);
                    jump_to_next(dyn, (uintptr_t)getAlternate((void*)j64), 0, ninst, rex.is32bits);
                } else {
                    // inside the block
                    CacheTransform(dyn, ninst, CHECK_CACHE(), x1, x2, x3);
                    tmp = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address-(dyn->native_size);
                    if(tmp==4) {
                        NOP;
                    } else {
                        B(tmp);
                    }
                }
                *need_epilog = 0;
                *ok = 0;
            }
            break;

        case 0xEC:                      /* IN AL, DX */
        case 0xED:                      /* IN EAX, DX */
        case 0xEE:                      /* OUT DX, AL */
        case 0xEF:                      /* OUT DX, EAX */
            INST_NAME(opcode==0xEC?"IN AL, DX":(opcode==0xED?"IN EAX, DX":(opcode==0xEE?"OUT DX, AL":"OUT DX, EAX")));
            SETFLAGS(X_ALL, SF_SET_NODF);    // Hack to set flags in "don't care" state
            GETIP(ip);
            STORE_XEMU_CALL(xRIP);
            CALL(native_priv, -1);
            LOAD_XEMU_CALL(xRIP);
            jump_to_epilog(dyn, 0, xRIP, ninst);
            *need_epilog = 0;
            *ok = 0;
            break;

        case 0xF0:
            addr = dynarec64_F0(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
            break;

        case 0xF4:
            INST_NAME("HLT");
            SETFLAGS(X_ALL, SF_SET_NODF);    // Hack to set flags in "don't care" state
            GETIP(ip);
            STORE_XEMU_CALL(xRIP);
            CALL(native_priv, -1);
            LOAD_XEMU_CALL(xRIP);
            jump_to_epilog(dyn, 0, xRIP, ninst);
            *need_epilog = 0;
            *ok = 0;
            break;
        case 0xF5:
            INST_NAME("CMC");
            READFLAGS(X_CF);
            SETFLAGS(X_CF, SF_SUBSET);
            EORw_mask(xFlags, xFlags, 0, 0); //mask=0x00000001
            break;
        case 0xF6:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                case 1:
                    INST_NAME("TEST Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEB(x1, 1);
                    u8 = F8;
                    MOV32w(x2, u8);
                    emit_test8(dyn, ninst, x1, x2, x3, x4, x5);
                    break;
                case 2:
                    INST_NAME("NOT Eb");
                    GETEB(x1, 0);
                    MVNw_REG(x1, x1);
                    EBBACK;
                    break;
                case 3:
                    INST_NAME("NEG Eb");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETSEB(x1, 0);
                    emit_neg8(dyn, ninst, x1, x2, x4);
                    EBBACK;
                    break;
                case 4:
                    INST_NAME("MUL AL, Ed");
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETEB(x1, 0);
                    UXTBw(x2, xRAX);
                    MULw(x1, x2, x1);
                    UFLAG_RES(x1);
                    BFIx(xRAX, x1, 0, 16);
                    UFLAG_DF(x1, d_mul8);
                    break;
                case 5:
                    INST_NAME("IMUL AL, Eb");
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETSEB(x1, 0);
                    SXTBw(x2, xRAX);
                    MULw(x1, x2, x1);
                    UFLAG_RES(x1);
                    BFIx(xRAX, x1, 0, 16);
                    UFLAG_DF(x1, d_imul8);
                    break;
                case 6:
                    INST_NAME("DIV Eb");
                    SETFLAGS(X_ALL, SF_SET);
                    SET_DFNONE(x1);
                    GETEB(x1, 0);
                    UXTHw(x2, xRAX);
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
                    BFIx(xRAX, x3, 0, 8);
                    BFIx(xRAX, x4, 8, 8);
                    break;
                case 7:
                    INST_NAME("IDIV Eb");
                    SKIPTEST(x1);
                    SETFLAGS(X_ALL, SF_SET);
                    SET_DFNONE(x1);
                    GETSEB(x1, 0);
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
                    SXTHw(x2, xRAX);
                    SDIVw(x3, x2, ed);
                    MSUBw(x4, x3, ed, x2);  // x4 = x2 mod ed (i.e. x2 - x3*ed)
                    BFIx(xRAX, x3, 0, 8);
                    BFIx(xRAX, x4, 8, 8);
                    break;
            }
            break;
        case 0xF7:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                case 1:
                    INST_NAME("TEST Ed, Id");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEDH(x1, 4);
                    i64 = F32S;
                    MOV64xw(x2, i64);
                    emit_test32(dyn, ninst, rex, ed, x2, x3, x4, x5);
                    break;
                case 2:
                    INST_NAME("NOT Ed");
                    GETED(0);
                    MVNxw_REG(ed, ed);
                    WBACK;
                    break;
                case 3:
                    INST_NAME("NEG Ed");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETED(0);
                    emit_neg32(dyn, ninst, rex, ed, x3, x4);
                    WBACK;
                    break;
                case 4:
                    INST_NAME("MUL EAX, Ed");
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETED(0);
                    if(rex.w) {
                        if(ed==xRDX) gd=x3; else gd=xRDX;
                        UMULH(gd, xRAX, ed);
                        MULx(xRAX, xRAX, ed);
                        if(gd!=xRDX) {MOVx_REG(xRDX, gd);}
                    } else {
                        UMULL(xRDX, xRAX, ed);  //64 <- 32x32
                        MOVw_REG(xRAX, xRDX);
                        LSRx(xRDX, xRDX, 32);
                    }
                    UFLAG_RES(xRAX);
                    UFLAG_OP1(xRDX);
                    UFLAG_DF(x2, rex.w?d_mul64:d_mul32);
                    break;
                case 5:
                    INST_NAME("IMUL EAX, Ed");
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETED(0);
                    if(rex.w) {
                        if(ed==xRDX) gd=x3; else gd=xRDX;
                        SMULH(gd, xRAX, ed);
                        MULx(xRAX, xRAX, ed);
                        if(gd!=xRDX) {MOVx_REG(xRDX, gd);}
                    } else {
                        SMULL(xRDX, xRAX, ed);  //64 <- 32x32
                        MOVw_REG(xRAX, xRDX);
                        LSRx(xRDX, xRDX, 32);
                    }
                    UFLAG_RES(xRAX);
                    UFLAG_OP1(xRDX);
                    UFLAG_DF(x2, rex.w?d_imul64:d_imul32);
                    break;
                case 6:
                    INST_NAME("DIV Ed");
                    SETFLAGS(X_ALL, SF_SET);
                    if(!rex.w) {
                        SET_DFNONE(x2);
                        GETED(0);
                        if(ninst && (nextop==0xF0)
                           && dyn->insts[ninst-1].x64.addr
                           && *(uint8_t*)(dyn->insts[ninst-1].x64.addr)==0xB8
                           && *(uint32_t*)(dyn->insts[ninst-1].x64.addr+1)==0) {
                            // hack for some protection that check a divide by zero actualy trigger a divide by zero exception
                            MESSAGE(LOG_INFO, "Divide by 0 hack\n");
                            GETIP(ip);
                            STORE_XEMU_CALL(xRIP);
                            CALL(native_div0, -1);
                            LOAD_XEMU_CALL(xRIP);
                        } else {
                            if(box64_dynarec_div0) {
                                CBNZx_MARK3(ed);
                                GETIP_(ip);
                                STORE_XEMU_CALL(xRIP);
                                CALL(native_div0, -1);
                                CLEARIP();
                                LOAD_XEMU_CALL(xRIP);
                                jump_to_epilog(dyn, 0, xRIP, ninst);
                                MARK3;
                            }
                            MOVw_REG(x3, xRAX);
                            ORRx_REG_LSL(x3, x3, xRDX, 32);
                            if(MODREG) {
                                MOVw_REG(x4, ed);
                                ed = x4;
                            }
                            UDIVx(x2, x3, ed);
                            MSUBx(x4, x2, ed, xRAX);
                            MOVw_REG(xRAX, x2);
                            MOVw_REG(xRDX, x4);
                        }
                    } else {
                        if(ninst
                           && dyn->insts[ninst-1].x64.addr
                           && *(uint8_t*)(dyn->insts[ninst-1].x64.addr)==0x31
                           && *(uint8_t*)(dyn->insts[ninst-1].x64.addr+1)==0xD2) {
                            SET_DFNONE(x2);
                            GETED(0);
                            if(box64_dynarec_div0) {
                                CBNZx_MARK3(ed);
                                GETIP_(ip);
                                STORE_XEMU_CALL(xRIP);
                                CALL(native_div0, -1);
                                CLEARIP();
                                LOAD_XEMU_CALL(xRIP);
                                jump_to_epilog(dyn, 0, xRIP, ninst);
                                MARK3;
                            }
                            UDIVx(x2, xRAX, ed);
                            MSUBx(xRDX, x2, ed, xRAX);
                            MOVx_REG(xRAX, x2);
                        } else {
                            GETEDH(x1, 0);  // get edd changed addr, so cannot be called 2 times for same op...
                            if(box64_dynarec_div0) {
                                CBNZx_MARK3(ed);
                                GETIP_(ip);
                                STORE_XEMU_CALL(xRIP);
                                CALL(native_div0, -1);
                                CLEARIP();
                                LOAD_XEMU_CALL(xRIP);
                                jump_to_epilog(dyn, 0, xRIP, ninst);
                                MARK3;
                            }
                            CBZxw_MARK(xRDX);
                            if(ed!=x1) {MOVx_REG(x1, ed);}
                            CALL(div64, -1);
                            B_NEXT_nocond;
                            MARK;
                            UDIVx(x2, xRAX, ed);
                            MSUBx(xRDX, x2, ed, xRAX);
                            MOVx_REG(xRAX, x2);
                            SET_DFNONE(x2);
                        }
                    }
                    break;
                case 7:
                    INST_NAME("IDIV Ed");
                    SKIPTEST(x1);
                    SETFLAGS(X_ALL, SF_SET);
                    SET_DFNONE(x2)
                    if(!rex.w) {
                        GETSEDw(0);
                        if(box64_dynarec_div0) {
                            CBNZx_MARK3(wb);
                            GETIP_(ip);
                            STORE_XEMU_CALL(xRIP);
                            CALL(native_div0, -1);
                            CLEARIP();
                            LOAD_XEMU_CALL(xRIP);
                            jump_to_epilog(dyn, 0, xRIP, ninst);
                            MARK3;
                        }
                        MOVw_REG(x3, xRAX);
                        ORRx_REG_LSL(x3, x3, xRDX, 32);
                        SDIVx(x2, x3, wb);
                        MSUBx(x4, x2, wb, x3);
                        MOVw_REG(xRAX, x2);
                        MOVw_REG(xRDX, x4);
                    } else {
                        if(ninst && dyn->insts
                           &&  dyn->insts[ninst-1].x64.addr
                           && *(uint8_t*)(dyn->insts[ninst-1].x64.addr)==0x48
                           && *(uint8_t*)(dyn->insts[ninst-1].x64.addr+1)==0x99) {
                            GETED(0);
                            if(box64_dynarec_div0) {
                                CBNZx_MARK3(ed);
                                GETIP_(ip);
                                STORE_XEMU_CALL(xRIP);
                                CALL(native_div0, -1);
                                CLEARIP();
                                LOAD_XEMU_CALL(xRIP);
                                jump_to_epilog(dyn, 0, xRIP, ninst);
                                MARK3;
                            }
                            SDIVx(x2, xRAX, ed);
                            MSUBx(xRDX, x2, ed, xRAX);
                            MOVx_REG(xRAX, x2);
                        } else {
                            GETEDH(x1, 0);  // get edd changed addr, so cannot be called 2 times for same op...
                            if(box64_dynarec_div0) {
                                CBNZx_MARK3(ed);
                                GETIP_(ip);
                                STORE_XEMU_CALL(xRIP);
                                CALL(native_div0, -1);
                                CLEARIP();
                                LOAD_XEMU_CALL(xRIP);
                                jump_to_epilog(dyn, 0, xRIP, ninst);
                                MARK3;
                            }
                            //Need to see if RDX==0 and RAX not signed
                            // or RDX==-1 and RAX signed
                            CBNZx_MARK2(xRDX);
                            TBZ_MARK(xRAX, 63);
                            MARK2;
                            MVNx_REG(x2, xRDX);
                            CBNZx_MARK3(x2);
                            TBNZ_MARK(xRAX, 63);
                            MARK3;
                            if(ed!=x1) {MOVx_REG(x1, ed);}
                            CALL((void*)idiv64, -1);
                            B_NEXT_nocond;
                            MARK;
                            SDIVx(x2, xRAX, ed);
                            MSUBx(xRDX, x2, ed, xRAX);
                            MOVx_REG(xRAX, x2);
                        }
                    }
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
        case 0xFA:                      /* STI */
        case 0xFB:                      /* CLI */
            INST_NAME(opcode==0xFA?"CLI":"STI");
            SETFLAGS(X_ALL, SF_SET_NODF);    // Hack to set flags in "don't care" state
            GETIP(ip);
            STORE_XEMU_CALL(xRIP);
            CALL(native_priv, -1);
            LOAD_XEMU_CALL(xRIP);
            jump_to_epilog(dyn, 0, xRIP, ninst);
            *need_epilog = 0;
            *ok = 0;
            break;
        case 0xFC:
            INST_NAME("CLD");
            BFCw(xFlags, F_DF, 1);
            break;
        case 0xFD:
            INST_NAME("STD");
            MOV32w(x1, 1);
            BFIw(xFlags, x1, F_DF, 1);
            break;
        case 0xFE:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("INC Eb");
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET_PENDING);
                    GETEB(x1, 0);
                    emit_inc8(dyn, ninst, x1, x2, x4);
                    EBBACK;
                    break;
                case 1:
                    INST_NAME("DEC Eb");
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET_PENDING);
                    GETEB(x1, 0);
                    emit_dec8(dyn, ninst, x1, x2, x4);
                    EBBACK;
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xFF:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0: // INC Ed
                    INST_NAME("INC Ed");
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET_PENDING);
                    GETED(0);
                    emit_inc32(dyn, ninst, rex, ed, x3, x4);
                    WBACK;
                    break;
                case 1: //DEC Ed
                    INST_NAME("DEC Ed");
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET_PENDING);
                    GETED(0);
                    emit_dec32(dyn, ninst, rex, ed, x3, x4);
                    WBACK;
                    break;
                case 2: // CALL Ed
                    INST_NAME("CALL Ed");
                    PASS2IF((box64_dynarec_safeflags>1) ||
                        ((ninst && dyn->insts[ninst-1].x64.set_flags)
                        || ((ninst>1) && dyn->insts[ninst-2].x64.set_flags)), 1)
                    {
                        READFLAGS(X_PEND);          // that's suspicious
                    } else {
                        SETFLAGS(X_ALL, SF_SET_NODF);    //Hack to put flag in "don't care" state
                    }
                    GETEDz(0);
                    if(box64_dynarec_callret && box64_dynarec_bigblock>1) {
                        BARRIER(BARRIER_FULL);
                        BARRIER_NEXT(BARRIER_FULL);
                    } else {
                        BARRIER(BARRIER_FLOAT);
                        *need_epilog = 0;
                        *ok = 0;
                    }
                    GETIP_(addr);
                    if(box64_dynarec_callret) {
                        SET_HASCALLRET();
                        // Push actual return address
                        if(addr < (dyn->start+dyn->isize)) {
                            // there is a next...
                            j64 = (dyn->insts)?(dyn->insts[ninst].epilog-(dyn->native_size)):0;
                            ADR_S20(x4, j64);
                            MESSAGE(LOG_NONE, "\tCALLRET set return to +%di\n", j64>>2);
                        } else {
                            MESSAGE(LOG_NONE, "\tCALLRET set return to Jmptable(%p)\n", (void*)addr);
                            j64 = getJumpTableAddress64(addr);
                            TABLE64(x4, j64);
                            LDRx_U12(x4, x4, 0);
                        }
                        STPx_S7_preindex(x4, xRIP, xSP, -16);
                    }
                    PUSH1z(xRIP);
                    jump_to_next(dyn, 0, ed, ninst, rex.is32bits);
                    break;
                case 3: // CALL FAR Ed
                    if(MODREG) {
                        DEFAULT;
                    } else {
                        INST_NAME("CALL FAR Ed");
                        READFLAGS(X_PEND);
                        BARRIER(BARRIER_FLOAT);
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, &unscaled, 0, 0, rex, NULL, 0, 0);
                        LDxw(x1, wback, 0);
                        ed = x1;
                        LDH(x3, wback, rex.w?8:4);
                        LDH(x4, xEmu, offsetof(x64emu_t, segs[_CS]));
                        GETIP_(addr);
                        /*
                        if(box64_dynarec_callret) {
                            SET_HASCALLRET();
                            // Push actual return address
                            if(addr < (dyn->start+dyn->isize)) {
                                // there is a next...
                                j64 = (dyn->insts)?(dyn->insts[ninst].epilog-(dyn->native_size)):0;
                                ADR_S20(x4, j64);
                                MESSAGE(LOG_NONE, "\tCALLRET set return to +%di\n", j64>>2);
                            } else {
                                MESSAGE(LOG_NONE, "\tCALLRET set return to Jmptable(%p)\n", (void*)addr);
                                j64 = getJumpTableAddress64(addr);
                                TABLE64(x4, j64);
                                LDRx_U12(x4, x4, 0);
                            }
                            STPx_S7_preindex(x4, xRIP, xSP, -16);
                        }
                        */ // not doing callret because call far will exit the dynablock anyway, to be sure to recompute CS segment
                        PUSH1z(x4);
                        PUSH1z(xRIP);
                        STH(x3, xEmu, offsetof(x64emu_t, segs[_CS]));
                        STW(xZR, xEmu, offsetof(x64emu_t, segs_serial[_CS]));
                        jump_to_epilog(dyn, 0, ed, ninst);
                        *need_epilog = 0;
                        *ok = 0;
                    }
                    break;
                case 4: // JMP Ed
                    INST_NAME("JMP Ed");
                    READFLAGS(X_PEND);
                    BARRIER(BARRIER_FLOAT);
                    GETEDz(0);
                    jump_to_next(dyn, 0, ed, ninst, rex.is32bits);
                    *need_epilog = 0;
                    *ok = 0;
                    break;
                case 5: // JMP FAR Ed
                    if(MODREG) {
                        DEFAULT;
                    } else {
                        INST_NAME("JMP FAR Ed");
                        READFLAGS(X_PEND);
                        BARRIER(BARRIER_FLOAT);
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, &unscaled, 0, 0, rex, NULL, 0, 0);
                        LDxw(x1, wback, 0);
                        ed = x1;
                        LDH(x3, wback, rex.w?8:4);
                        STH(x3, xEmu, offsetof(x64emu_t, segs[_CS]));
                        STW(xZR, xEmu, offsetof(x64emu_t, segs_serial[_CS]));
                        jump_to_epilog(dyn, 0, ed, ninst);
                        *need_epilog = 0;
                        *ok = 0;
                    }
                    break;
                case 6: // Push Ed
                    INST_NAME("PUSH Ed");
                    GETEDz(0);
                    PUSH1z(ed);
                    SMWRITE();
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
