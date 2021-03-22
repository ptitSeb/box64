#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
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
#include "dynarec_arm64.h"
#include "dynarec_arm64_private.h"
#include "arm64_printer.h"

#include "dynarec_arm64_functions.h"
#include "dynarec_arm64_helper.h"

uintptr_t dynarec64_00(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog)
{
    uint8_t nextop, opcode;
    uint8_t gd, ed;
    int8_t i8;
    int32_t i32, j32, tmp;
    int64_t i64;
    uint8_t u8;
    uint8_t gb1, gb2, eb1, eb2;
    uint32_t u32;
    uint64_t u64;
    uint8_t wback, wb1, wb2;
    int fixedaddress;
    rex_t rex;
    int rep;    // 0 none, 1=F2 prefix, 2=F3 prefix

    opcode = F8;
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(tmp);
    MAYUSE(j32);

    rep = 0;
    while((opcode==0xF2) || (opcode==0xF3)) {
        rep = opcode-0xF1;
        opcode = F8;
    }
    rex.rex = 0;
    while(opcode>=0x40 && opcode<=0x4f) {
        rex.rex = opcode;
        opcode = F8;
    }

    switch(opcode) {
        case 0x00:
            INST_NAME("ADD Eb, Gb");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETEB(x1, 0);
            GETGB(x2);
            emit_add8(dyn, ninst, x1, x2, x4, x5);
            EBBACK;
            break;
        case 0x01:
            INST_NAME("ADD Ed, Gd");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_add32(dyn, ninst, rex, ed, gd, x3, x4);
            WBACK;
            break;
        case 0x02:
            INST_NAME("ADD Gb, Eb");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETEB(x2, 0);
            GETGB(x1);
            emit_add8(dyn, ninst, x1, x2, x3, x4);
            GBBACK;
            break;
        case 0x03:
            INST_NAME("ADD Gd, Ed");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_add32(dyn, ninst, rex, gd, ed, x3, x4);
            break;
        case 0x04:
            INST_NAME("ADD AL, Ib");
            SETFLAGS(X_ALL, SF_SET);
            u8 = F8;
            UXTBw(x1, xRAX);
            emit_add8c(dyn, ninst, x1, u8, x3, x4);
            BFIx(xRAX, x1, 0, 8);
            break;
        case 0x05:
            INST_NAME("ADD EAX, Id");
            SETFLAGS(X_ALL, SF_SET);
            i64 = F32S;
            emit_add32c(dyn, ninst, rex, xRAX, i64, x3, x4, x5);
            break;

        case 0x08:
            INST_NAME("OR Eb, Gb");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETEB(x1, 0);
            GETGB(x2);
            emit_or8(dyn, ninst, x1, x2, x4, x2);
            EBBACK;
            break;
        case 0x09:
            INST_NAME("OR Ed, Gd");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_or32(dyn, ninst, rex, ed, gd, x3, x4);
            WBACK;
            break;
        case 0x0A:
            INST_NAME("OR Gb, Eb");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETEB(x2, 0);
            GETGB(x1);
            emit_or8(dyn, ninst, x1, x2, x3, x4);
            GBBACK;
            break;
        case 0x0B:
            INST_NAME("OR Gd, Ed");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_or32(dyn, ninst, rex, gd, ed, x3, x4);
            break;
        case 0x0C:
            INST_NAME("OR AL, Ib");
            SETFLAGS(X_ALL, SF_SET);
            u8 = F8;
            UXTBw(x1, xRAX);
            emit_or8c(dyn, ninst, x1, u8, x3, x4);
            BFIx(xRAX, x1, 0, 8);
            break;
        case 0x0D:
            INST_NAME("OR EAX, Id");
            SETFLAGS(X_ALL, SF_SET);
            i64 = F32S;
            emit_or32c(dyn, ninst, rex, xRAX, i64, x3, x4);
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
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETEB(x1, 0);
            GETGB(x2);
            emit_adc8(dyn, ninst, x1, x2, x4, x5);
            EBBACK;
            break;
        case 0x11:
            INST_NAME("ADC Ed, Gd");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_adc32(dyn, ninst, rex, ed, gd, x3, x4);
            WBACK;
            break;
        case 0x12:
            INST_NAME("ADC Gb, Eb");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETEB(x2, 0);
            GETGB(x1);
            emit_adc8(dyn, ninst, x1, x2, x4, x3);
            GBBACK;
            break;
        case 0x13:
            INST_NAME("ADC Gd, Ed");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_adc32(dyn, ninst, rex, gd, ed, x3, x4);
            break;
        case 0x14:
            INST_NAME("ADC AL, Ib");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET);
            u8 = F8;
            UXTBw(x1, xRAX);
            emit_adc8c(dyn, ninst, x1, u8, x3, x4, x5);
            BFIx(xRAX, x1, 0, 8);
            break;
        case 0x15:
            INST_NAME("ADC EAX, Id");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET);
            i64 = F32S;
            MOV64xw(x1, i64);
            emit_adc32(dyn, ninst, rex, xRAX, x1, x3, x4);
            break;

        case 0x18:
            INST_NAME("SBB Eb, Gb");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETEB(x1, 0);
            GETGB(x2);
            emit_sbb8(dyn, ninst, x1, x2, x4, x5);
            EBBACK;
            break;
        case 0x19:
            INST_NAME("SBB Ed, Gd");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_sbb32(dyn, ninst, rex, ed, gd, x3, x4);
            WBACK;
            break;
        case 0x1A:
            INST_NAME("SBB Gb, Eb");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETEB(x2, 0);
            GETGB(x1);
            emit_sbb8(dyn, ninst, x1, x2, x3, x4);
            GBBACK;
            break;
        case 0x1B:
            INST_NAME("SBB Gd, Ed");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_sbb32(dyn, ninst, rex, gd, ed, x3, x4);
            break;
        case 0x1C:
            INST_NAME("SBB AL, Ib");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET);
            u8 = F8;
            UXTBw(x1, xRAX);
            emit_sbb8c(dyn, ninst, x1, u8, x3, x4, x5);
            BFIx(xRAX, x1, 0, 8);
            break;
        case 0x1D:
            INST_NAME("SBB EAX, Id");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET);
            i64 = F32S;
            MOV64xw(x2, i64);
            emit_sbb32(dyn, ninst, rex, xRAX, x2, x3, x4);
            break;

        case 0x20:
            INST_NAME("AND Eb, Gb");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETEB(x1, 0);
            GETGB(x2);
            emit_and8(dyn, ninst, x1, x2, x4, x5);
            EBBACK;
            break;
        case 0x21:
            INST_NAME("AND Ed, Gd");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_and32(dyn, ninst, rex, ed, gd, x3, x4);
            WBACK;
            break;
        case 0x22:
            INST_NAME("AND Gb, Eb");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETEB(x2, 0);
            GETGB(x1);
            emit_and8(dyn, ninst, x1, x2, x3, x4);
            GBBACK;
            break;
        case 0x23:
            INST_NAME("AND Gd, Ed");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_and32(dyn, ninst, rex, gd, ed, x3, x4);
            break;
        case 0x24:
            INST_NAME("AND AL, Ib");
            SETFLAGS(X_ALL, SF_PENDING);
            u8 = F8;
            UXTBw(x1, xRAX);
            emit_and8c(dyn, ninst, x1, u8, x3, x4);
            BFIx(xRAX, x1, 0, 8);
            break;
        case 0x25:
            INST_NAME("AND EAX, Id");
            SETFLAGS(X_ALL, SF_SET);
            i64 = F32S;
            emit_and32c(dyn, ninst, rex, xRAX, i64, x3, x4);
            break;

        case 0x28:
            INST_NAME("SUB Eb, Gb");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETEB(x1, 0);
            GETGB(x2);
            emit_sub8(dyn, ninst, x1, x2, x4, x5);
            EBBACK;
            break;
        case 0x29:
            INST_NAME("SUB Ed, Gd");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_sub32(dyn, ninst, rex, ed, gd, x3, x4);
            WBACK;
            break;
        case 0x2A:
            INST_NAME("SUB Gb, Eb");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETEB(x2, 0);
            GETGB(x1);
            emit_sub8(dyn, ninst, x1, x2, x3, x4);
            GBBACK;
            break;
        case 0x2B:
            INST_NAME("SUB Gd, Ed");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_sub32(dyn, ninst, rex, gd, ed, x3, x4);
            break;
        case 0x2C:
            INST_NAME("SUB AL, Ib");
            SETFLAGS(X_ALL, SF_SET);
            u8 = F8;
            UXTBw(x1, xRAX);
            emit_sub8c(dyn, ninst, x1, u8, x3, x4, x5);
            BFIx(xRAX, x1, 0, 8);
            break;
        case 0x2D:
            INST_NAME("SUB EAX, Id");
            SETFLAGS(X_ALL, SF_SET);
            i64 = F32S;
            emit_sub32c(dyn, ninst, rex, xRAX, i64, x3, x4, x5);
            break;

        case 0x30:
            INST_NAME("XOR Eb, Gb");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETEB(x1, 0);
            GETGB(x2);
            emit_xor8(dyn, ninst, x1, x2, x4, x5);
            EBBACK;
            break;
        case 0x31:
            INST_NAME("XOR Ed, Gd");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_xor32(dyn, ninst, rex, ed, gd, x3, x4);
            WBACK;
            break;
        case 0x32:
            INST_NAME("XOR Gb, Eb");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETEB(x2, 0);
            GETGB(x1);
            emit_xor8(dyn, ninst, x1, x2, x3, x4);
            GBBACK;
            break;
        case 0x33:
            INST_NAME("XOR Gd, Ed");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_xor32(dyn, ninst, rex, gd, ed, x3, x4);
            break;
        case 0x34:
            INST_NAME("XOR AL, Ib");
            SETFLAGS(X_ALL, SF_SET);
            u8 = F8;
            UXTBw(x1, xRAX);
            emit_xor8c(dyn, ninst, x1, u8, x3, x4);
            BFIx(xRAX, x1, 0, 8);
            break;
        case 0x35:
            INST_NAME("XOR EAX, Id");
            SETFLAGS(X_ALL, SF_SET);
            i64 = F32S;
            emit_xor32c(dyn, ninst, rex, xRAX, i64, x3, x4);
            break;

        case 0x38:
            INST_NAME("CMP Eb, Gb");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETEB(x1, 0);
            GETGB(x2);
            emit_cmp8(dyn, ninst, x1, x2, x3, x4, x5);
            break;
        case 0x39:
            INST_NAME("CMP Ed, Gd");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_cmp32(dyn, ninst, rex, ed, gd, x3, x4, x5);
            break;
        case 0x3A:
            INST_NAME("CMP Gb, Eb");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETEB(x2, 0);
            GETGB(x1);
            emit_cmp8(dyn, ninst, x1, x2, x3, x4, x5);
            break;
        case 0x3B:
            INST_NAME("CMP Gd, Ed");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_cmp32(dyn, ninst, rex, gd, ed, x3, x4, x5);
            break;
        case 0x3C:
            INST_NAME("CMP AL, Ib");
            SETFLAGS(X_ALL, SF_SET);
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
            SETFLAGS(X_ALL, SF_SET);
            i64 = F32S;
            if(i64) {
                MOV64xw(x2, i64);
                emit_cmp32(dyn, ninst, rex, xRAX, x2, x3, x4, x5);
            } else
                emit_cmp32_0(dyn, ninst, rex, xRAX, x3, x4);
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
            if(gd==xRSP) {
                MOVx_REG(x1, gd);
                gd = x1;
            }
            PUSH1(gd);
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
            if(gd == xRSP) {
                POP1(x1);
                MOVx_REG(gd, x1);
            } else {
                POP1(gd);
            }
            break;

        case 0x63:
            INST_NAME("MOVSXD Gd, Ed");
            nextop = F8;
            GETGD;
            if(rex.w) {
                if(MODREG) {   // reg <= reg
                    SXTWx(gd, xRAX+(nextop&7)+(rex.b<<3));
                } else {                    // mem <= reg
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0xfff<<2, 3, rex, 0, 0);
                    LDRSW_U12(gd, ed, fixedaddress);
                }
            } else {
                if(MODREG) {   // reg <= reg
                    MOVw_REG(gd, xRAX+(nextop&7)+(rex.b<<3));
                } else {                    // mem <= reg
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0xfff<<2, 3, rex, 0, 0);
                    LDRw_U12(gd, ed, fixedaddress);
                }
            }
            break;
        case 0x64:
            addr = dynarec64_64(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
            break;

        case 0x66:
            addr = dynarec64_66(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
            break;
        case 0x67:
            addr = dynarec64_67(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
            break;
        case 0x68:
            INST_NAME("PUSH Id");
            i64 = F32S;
            if(PK(0)==0xC3) {
                MESSAGE(LOG_DUMP, "PUSH then RET, using indirect\n");
                TABLE64(x3, ip+1);
                LDRSW_U12(x1, x3, 0);
                PUSH1(x1);
            } else {
                MOV64x(x3, i64);
                PUSH1(x3);
            }
            break;
        case 0x69:
            INST_NAME("IMUL Gd, Ed, Id");
            SETFLAGS(X_ALL, SF_PENDING);
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
                    UFLAG_OP1(x3);
                    UFLAG_RES(gd);
                    UFLAG_DF(x3, d_imul64);
                } else {
                    MULxw(gd, ed, x4);
                }
            } else {
                // 32bits imul
                UFLAG_IF {
                    SMULL(gd, ed, x4);
                    UFLAG_RES(gd);
                    LSRx(x3, gd, 32);
                    UFLAG_OP1(x3);
                    UFLAG_DF(x3, d_imul32);
                    MOVw_REG(gd, gd);
                } else {
                    MULxw(gd, ed, x4);
                }
            }
            break;
        case 0x6A:
            INST_NAME("PUSH Ib");
            i64 = F8S;
            MOV64x(x3, i64);
            PUSH1(x3);
            break;
        case 0x6B:
            INST_NAME("IMUL Gd, Ed, Ib");
            SETFLAGS(X_ALL, SF_PENDING);
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
                    UFLAG_OP1(x3);
                    UFLAG_RES(gd);
                    UFLAG_DF(x3, d_imul64);
                } else {
                    MULxw(gd, ed, x4);
                }
            } else {
                // 32bits imul
                UFLAG_IF {
                    SMULL(gd, ed, x4);
                    UFLAG_RES(gd);
                    LSRx(x3, gd, 32);
                    UFLAG_OP1(x3);
                    UFLAG_DF(x3, d_imul32);
                    MOVw_REG(gd, gd);
                } else {
                    MULxw(gd, ed, x4);
                }
            }
            break;

        #define GO(GETFLAGS, NO, YES, F)    \
            READFLAGS(F);                   \
            i8 = F8S;   \
            BARRIER(2); \
            JUMP(addr+i8);\
            GETFLAGS;   \
            if(dyn->insts) {    \
                if(dyn->insts[ninst].x64.jmp_insts==-1) {   \
                    /* out of the block */                  \
                    i32 = dyn->insts[ninst+1].address-(dyn->arm_size); \
                    Bcond(NO, i32);     \
                    jump_to_next(dyn, addr+i8, 0, ninst); \
                } else {    \
                    /* inside the block */  \
                    i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address-(dyn->arm_size);    \
                    Bcond(YES, i32);    \
                }   \
            }

        GOCOND(0x70, "J", "ib");

        #undef GO
        
        case 0x80:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0: //ADD
                    INST_NAME("ADD Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET);
                    GETEB(x1, 1);
                    u8 = F8;
                    emit_add8c(dyn, ninst, x1, u8, x2, x4);
                    EBBACK;
                    break;
                case 1: //OR
                    INST_NAME("OR Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET);
                    GETEB(x1, 1);
                    u8 = F8;
                    emit_or8c(dyn, ninst, x1, u8, x2, x4);
                    EBBACK;
                    break;
                case 2: //ADC
                    INST_NAME("ADC Eb, Ib");
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET);
                    GETEB(x1, 1);
                    u8 = F8;
                    emit_adc8c(dyn, ninst, x1, u8, x2, x4, x5);
                    EBBACK;
                    break;
                case 3: //SBB
                    INST_NAME("SBB Eb, Ib");
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET);
                    GETEB(x1, 1);
                    u8 = F8;
                    emit_sbb8c(dyn, ninst, x1, u8, x2, x4, x5);
                    EBBACK;
                    break;
                case 4: //AND
                    INST_NAME("AND Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET);
                    GETEB(x1, 1);
                    u8 = F8;
                    emit_and8c(dyn, ninst, x1, u8, x2, x4);
                    EBBACK;
                    break;
                case 5: //SUB
                    INST_NAME("SUB Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET);
                    GETEB(x1, 1);
                    u8 = F8;
                    emit_sub8c(dyn, ninst, x1, u8, x2, x4, x5);
                    EBBACK;
                    break;
                case 6: //XOR
                    INST_NAME("XOR Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET);
                    GETEB(x1, 1);
                    u8 = F8;
                    emit_xor8c(dyn, ninst, x1, u8, x2, x4);
                    EBBACK;
                    break;
                case 7: //CMP
                    INST_NAME("CMP Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET);
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
                    SETFLAGS(X_ALL, SF_SET);
                    GETED((opcode==0x81)?4:1);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    emit_add32c(dyn, ninst, rex, ed, i64, x3, x4, x5);
                    WBACK;
                    break;
                case 1: //OR
                    if(opcode==0x81) {INST_NAME("OR Ed, Id");} else {INST_NAME("OR Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET);
                    GETED((opcode==0x81)?4:1);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    emit_or32c(dyn, ninst, rex, ed, i64, x3, x4);
                    WBACK;
                    break;
                case 2: //ADC
                    if(opcode==0x81) {INST_NAME("ADC Ed, Id");} else {INST_NAME("ADC Ed, Ib");}
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET);
                    GETED((opcode==0x81)?4:1);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    MOV64xw(x5, i64);
                    emit_adc32(dyn, ninst, rex, ed, x5, x3, x4);
                    WBACK;
                    break;
                case 3: //SBB
                    if(opcode==0x81) {INST_NAME("SBB Ed, Id");} else {INST_NAME("SBB Ed, Ib");}
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET);
                    GETED((opcode==0x81)?4:1);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    MOV64xw(x5, i64);
                    emit_sbb32(dyn, ninst, rex, ed, x5, x3, x4);
                    WBACK;
                    break;
                case 4: //AND
                    if(opcode==0x81) {INST_NAME("AND Ed, Id");} else {INST_NAME("AND Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET);
                    GETED((opcode==0x81)?4:1);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    emit_and32c(dyn, ninst, rex, ed, i64, x3, x4);
                    WBACK;
                    break;
                case 5: //SUB
                    if(opcode==0x81) {INST_NAME("SUB Ed, Id");} else {INST_NAME("SUB Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET);
                    GETED((opcode==0x81)?4:1);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    emit_sub32c(dyn, ninst, rex, ed, i64, x3, x4, x5);
                    WBACK;
                    break;
                case 6: //XOR
                    if(opcode==0x81) {INST_NAME("XOR Ed, Id");} else {INST_NAME("XOR Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET);
                    GETED((opcode==0x81)?4:1);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    emit_xor32c(dyn, ninst, rex, ed, i64, x3, x4);
                    WBACK;
                    break;
                case 7: //CMP
                    if(opcode==0x81) {INST_NAME("CMP Ed, Id");} else {INST_NAME("CMP Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET);
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
            SETFLAGS(X_ALL, SF_SET);
            nextop=F8;
            GETEB(x1, 0);
            GETGB(x2);
            emit_test8(dyn, ninst, x1, x2, x3, x4, x5);
            break;
        case 0x85:
            INST_NAME("TEST Ed, Gd");
            SETFLAGS(X_ALL, SF_SET);
            nextop=F8;
            GETGD;
            GETED(0);
            emit_test32(dyn, ninst, rex, ed, gd, x3, x5);
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
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0, 0, rex, 0, 0);
                TSTx_mask(ed, 1, 0, 1+rex.w);    // mask=3 or 7
                B_MARK(cNE);
                MARKLOCK;
                LDAXRxw(x1, ed);
                STLXRxw(x3, gd, ed);
                CBNZx_MARKLOCK(x3);
                B_MARK2_nocond;
                MARK;
                LDRxw_U12(x1, ed, 0);
                STRxw_U12(gd, ed, 0);
                MARK2;
                MOVxw_REG(gd, x1);
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
                gb2 = ((gd&4)>>2);
                gb1 = xRAX+(gd&3);
            }
            if(gb2) {
                gd = x4;
                UBFXw(gd, gb1, gb2*8, 8);
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
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0xfff, 0, rex, 0, 0);
                STRB_U12(gd, ed, fixedaddress);
            }
            break;
        case 0x89:
            INST_NAME("MOV Ed, Gd");
            nextop=F8;
            GETGD;
            if(MODREG) {   // reg <= reg
                MOVxw_REG(xRAX+(nextop&7)+(rex.b<<3), gd);
            } else {                    // mem <= reg
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0xfff<<(2+rex.w), (1<<(2+rex.w))-1, rex, 0, 0);
                STRxw_U12(gd, ed, fixedaddress);
            }
            break;

        case 0x8B:
            INST_NAME("MOV Gd, Ed");
            nextop=F8;
            GETGD;
            if(MODREG) {   // reg <= reg
                MOVxw_REG(gd, xRAX+(nextop&7)+(rex.b<<3));
            } else {                    // mem <= reg
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0xfff<<(2+rex.w), (1<<(2+rex.w))-1, rex, 0, 0);
                LDRxw_U12(gd, ed, fixedaddress);
            }
            break;

        case 0x8D:
            INST_NAME("LEA Gd, Ed");
            nextop=F8;
            GETGD;
            if(MODREG) {   // reg <= reg? that's an invalid operation
                DEFAULT;
            } else {                    // mem <= reg
                addr = geted(dyn, addr, ninst, nextop, &ed, gd, &fixedaddress, 0, 0, rex, 0, 0);
                if(gd!=ed) {    // it's sometimes used as a 3 bytes NOP
                    MOVxw_REG(gd, ed);
                } else if(ed>=xRAX && !rex.w) {
                    MOVw_REG(gd, gd);   //truncate the higher 32bits as asked
                }
            }
            break;

        case 0x90:
            INST_NAME("NOP");
            break;

        case 0x98:
            INST_NAME("CWDE");
            if(rex.w) {
                SXTWx(xRAX, xRAX);
            } else {
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
            PUSH1(xFlags);
            break;
        case 0x9D:
            INST_NAME("POPF");
            SETFLAGS(X_ALL, SF_SET);
            POP1(xFlags);
            MOV32w(x1, 0x3F7FD7);
            ANDw_REG(xFlags, xFlags, x1);
            ORRw_mask(xFlags, xFlags, 0b011111, 0);   //mask=0x00000002
            SET_DFNONE(x1);
            break;

        case 0xA5:
            if(rep) {
                INST_NAME("REP MOVSD");
                TSTw_REG(xRCX, xRCX);
                B_NEXT(cEQ);    // end of loop
                GETDIR(x3, rex.w?8:4);
                MARK;
                LDRxw_U12(x1, xRSI, 0);
                STRxw_U12(x1, xRDI, 0);
                ADDx_REG(xRSI, xRSI, x3);
                ADDx_REG(xRDI, xRDI, x3);
                SUBx_U12(xRCX, xRCX, 1);
                CBNZx_MARK(xRCX);
                // done
            } else {
                INST_NAME("MOVSD");
                GETDIR(x3, rex.w?8:4);
                LDRxw_U12(x1, xRSI, 0);
                STRxw_U12(x1, xRDI, 0);
                ADDx_REG(xRSI, xRSI, x3);
                ADDx_REG(xRDI, xRDI, x3);
            }
            break;

        case 0xA8:
            INST_NAME("TEST AL, Ib");
            SETFLAGS(X_ALL, SF_SET);
            UXTBx(x1, xRAX);
            u8 = F8;
            MOV32w(x2, u8);
            emit_test8(dyn, ninst, x1, x2, x3, x4, x5);
            break;
        case 0xA9:
            INST_NAME("TEST EAX, Id");
            SETFLAGS(X_ALL, SF_SET);
            i64 = F32S;
            MOV64xw(x2, i64);
            emit_test32(dyn, ninst, rex, xRAX, x2, x3, x4);
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
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    GETEB(x1, 1);
                    u8 = F8;
                    MOV32w(x2, u8);
                    CALL_(rol8, ed, x3);
                    EBBACK;
                    break;
                case 1:
                    INST_NAME("ROR Eb, Ib");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    GETEB(x1, 1);
                    u8 = F8;
                    MOV32w(x2, u8);
                    CALL_(ror8, ed, x3);
                    EBBACK;
                    break;
                case 2:
                    INST_NAME("RCL Eb, Ib");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET);
                    GETEB(x1, 1);
                    u8 = F8;
                    MOV32w(x2, u8);
                    CALL_(rcl8, ed, x3);
                    EBBACK;
                    break;
                case 3:
                    INST_NAME("RCR Eb, Ib");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET);
                    GETEB(x1, 1);
                    u8 = F8;
                    MOV32w(x2, u8);
                    CALL_(rcr8, ed, x3);
                    EBBACK;
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Eb, Ib");
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETEB(x1, 1);
                    u8 = (F8)&0x1f;
                    UFLAG_IF{
                        MOV32w(x4, u8); UFLAG_OP2(x4);
                    };
                    UFLAG_OP1(ed);
                    LSLw(ed, ed, u8);
                    EBBACK;
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, d_shl8);
                    break;
                case 5:
                    INST_NAME("SHR Eb, Ib");
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETEB(x1, 1);
                    u8 = (F8)&0x1f;
                    UFLAG_IF{
                        MOV32w(x4, u8); UFLAG_OP2(x4);
                    };
                    UFLAG_OP1(ed);
                    if(u8) {
                        LSRw(ed, ed, u8);
                        EBBACK;
                    }
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, d_shr8);
                    break;
                case 7:
                    INST_NAME("SAR Eb, Ib");
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETSEB(x1, 1);
                    u8 = (F8)&0x1f;
                    UFLAG_IF{
                        MOV32w(x4, u8); UFLAG_OP2(x4);
                    };
                    UFLAG_OP1(ed);
                    if(u8) {
                        ASRw(ed, ed, u8);
                        EBBACK;
                    }
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, d_sar8);
                    break;
            }
            break;
        case 0xC1:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("ROL Ed, Ib");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    GETED(1);
                    u8 = (F8)&(rex.w?0x3f:0x1f);
                    emit_rol32c(dyn, ninst, rex, ed, u8, x3, x4);
                    if(u8) { WBACK; }
                    break;
                case 1:
                    INST_NAME("ROR Ed, Ib");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    GETED(1);
                    u8 = (F8)&(rex.w?0x3f:0x1f);
                    emit_ror32c(dyn, ninst, rex, ed, u8, x3, x4);
                    if(u8) { WBACK; }
                    break;
                case 2:
                    INST_NAME("RCL Ed, Ib");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET);
                    GETEDW(x4, x1, 1);
                    u8 = F8;
                    MOV32w(x2, u8);
                    CALL_(rex.w?((void*)rcl64):((void*)rcl32), ed, x4);
                    WBACK;
                    break;
                case 3:
                    INST_NAME("RCR Ed, Ib");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET);
                    GETEDW(x4, x1, 1);
                    u8 = F8;
                    MOV32w(x2, u8);
                    CALL_(rex.w?((void*)rcr64):((void*)rcr32), ed, x4);
                    WBACK;
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ed, Ib");
                    SETFLAGS(X_ALL, SF_SET);    // some flags are left undefined
                    GETED(1);
                    u8 = (F8)&(rex.w?0x3f:0x1f);
                    emit_shl32c(dyn, ninst, rex, ed, u8, x3, x4);
                    WBACK;
                    break;
                case 5:
                    INST_NAME("SHR Ed, Ib");
                    SETFLAGS(X_ALL, SF_SET);    // some flags are left undefined
                    GETED(1);
                    u8 = (F8)&(rex.w?0x3f:0x1f);
                    emit_shr32c(dyn, ninst, rex, ed, u8, x3, x4);
                    if(u8) {
                        WBACK;
                    }
                    break;
                case 7:
                    INST_NAME("SAR Ed, Ib");
                    SETFLAGS(X_ALL, SF_SET);    // some flags are left undefined
                    GETED(1);
                    u8 = (F8)&(rex.w?0x3f:0x1f);
                    emit_sar32c(dyn, ninst, rex, ed, u8, x3, x4);
                    if(u8) {
                        WBACK;
                    }
                    break;
            }
            break;
        case 0xC2:
            INST_NAME("RETN");
            //SETFLAGS(X_ALL, SF_SET);    // Hack, set all flags (to an unknown state...)
            READFLAGS(X_PEND);  // lets play safe here too
            BARRIER(2);
            i32 = F16;
            retn_to_epilog(dyn, ninst, i32);
            *need_epilog = 0;
            *ok = 0;
            break;
        case 0xC3:
            INST_NAME("RET");
            // SETFLAGS(X_ALL, SF_SET);    // Hack, set all flags (to an unknown state...)
            READFLAGS(X_PEND);  // so instead, force the defered flags, so it's not too slow, and flags are not lost
            BARRIER(2);
            ret_to_epilog(dyn, ninst);
            *need_epilog = 0;
            *ok = 0;
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
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0xfff, 0, rex, 0, 1);
                u8 = F8;
                MOV32w(x3, u8);
                STRB_U12(x3, ed, fixedaddress);
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
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0xfff<<(2+rex.w), (1<<(2+rex.w))-1, rex, 0, 4);
                i64 = F32S;
                MOV64xw(x3, i64);
                STRxw_U12(x3, ed, fixedaddress);
            }
            break;

        case 0xCC:
            SETFLAGS(X_ALL, SF_SET);    // Hack, set all flags (to an unknown state...)
            if(PK(0)=='S' && PK(1)=='C') {
                addr+=2;
                BARRIER(2);
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
                    x87_forget(dyn, ninst, x3, x4, 0);
                    sse_purge07cache(dyn, ninst, x3);
                    GETIP(ip+1); // read the 0xCC
                    STORE_XEMU_CALL(xRIP);
                    CALL_S(x64Int3, -1);
                    LOAD_XEMU_CALL(xRIP);
                    addr+=8+8;
                    TABLE64(x3, addr); // expected return address
                    CMPSx_REG(xRIP, x3);
                    B_MARK(cNE);
                    LDRw_U12(x1, xEmu, offsetof(x64emu_t, quit));
                    CMPSw_U12(x1, 1);
                    B_NEXT(cNE);
                    MARK;
                    jump_to_epilog(dyn, 0, xRIP, ninst);
                }
            } else {
                #if 0
                INST_NAME("INT 3");
                // check if TRAP signal is handled
                LDRx_U12(x1, xEmu, offsetof(x64emu_t, context));
                MOV32w(x2, offsetof(box64context_t, signals[SIGTRAP]));
                LDRx_REG_LSL3(x3, x1, x2);
                CMPSx_U12(x3, 0);
                B_NEXT(cNE);
                MOV32w(x1, SIGTRAP);
                CALL_(raise, -1, 0);
                break;
                #else
                DEFAULT;
                #endif
            }
            break;
        case 0xD0:
        case 0xD2:  // TODO: Jump if CL is 0
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    if(opcode==0xD0) {
                        INST_NAME("ROL Eb, 1");
                        MOV32w(x2, 1);
                    } else {
                        INST_NAME("ROL Eb, CL");
                        ANDSw_mask(x2, xRCX, 0, 0b00100);
                    }
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    GETEB(x1, 0);
                    CALL_(rol8, x1, x3);
                    EBBACK;
                    break;
                case 1:
                    if(opcode==0xD0) {
                        INST_NAME("ROR Eb, 1");
                        MOV32w(x2, 1);
                    } else {
                        INST_NAME("ROR Eb, CL");
                        ANDSw_mask(x2, xRCX, 0, 0b00100);
                    }
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    GETEB(x1, 0);
                    CALL_(ror8, x1, x3);
                    EBBACK;
                    break;
                case 2:
                    if(opcode==0xD0) {INST_NAME("RCL Eb, 1");} else {INST_NAME("RCL Eb, CL");}
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET);
                    if(opcode==0xD0) {MOV32w(x2, 1);} else {ANDSw_mask(x2, xRCX, 0, 0b00100);}
                    GETEB(x1, 0);
                    CALL_(rcl8, x1, x3);
                    EBBACK;
                    break;
                case 3:
                    if(opcode==0xD0) {INST_NAME("RCR Eb, 1");} else {INST_NAME("RCR Eb, CL");}
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET);
                    if(opcode==0xD0) {MOV32w(x2, 1);} else {ANDSw_mask(x2, xRCX, 0, 0b00100);}
                    GETEB(x1, 0);
                    CALL_(rcr8, x1, x3);
                    EBBACK;
                    break;
                case 4:
                case 6:
                    if(opcode==0xD0) {
                        INST_NAME("SHL Eb, 1");
                        MOV32w(x2, 1);
                    } else {
                        INST_NAME("SHL Eb, CL");
                        ANDSw_mask(x2, xRCX, 0, 0b00100);
                    }
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETEB(x1, 0);
                    UFLAG_OP12(ed, x2)
                    LSLw_REG(ed, ed, x2);
                    EBBACK;
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, d_shl8);
                    break;
                case 5:
                    if(opcode==0xD0) {
                        INST_NAME("SHR Eb, 1");
                        MOV32w(x2, 1);
                    } else {
                        INST_NAME("SHR Eb, CL");
                        ANDSw_mask(x2, xRCX, 0, 0b00100);
                    }
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETEB(x1, 0);
                    UFLAG_OP12(ed, x2);
                    LSRw_REG(ed, ed, x2);
                    EBBACK;
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, d_shr8);
                    break;
                case 7:
                    if(opcode==0xD0) {
                        INST_NAME("SAR Eb, 1");
                        MOV32w(x2, 1);
                    } else {
                        INST_NAME("SAR Eb, CL");
                        ANDSw_mask(x2, xRCX, 0, 0b00100);
                    }
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETSEB(x1, 0);
                    UFLAG_OP12(ed, x2)
                    ASRw_REG(ed, ed, x2);
                    EBBACK;
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, d_sar8);
                    break;
            }
            break;
        case 0xD1:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("ROL Ed, 1");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    GETED(0);
                    emit_rol32c(dyn, ninst, rex, ed, 1, x3, x4);
                    WBACK;
                    break;
                case 1:
                    INST_NAME("ROR Ed, 1");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    GETED(0);
                    emit_ror32c(dyn, ninst, rex, ed, 1, x3, x4);
                    WBACK;
                    break;
                case 2:
                    INST_NAME("RCL Ed, 1");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET);
                    MOV32w(x2, 1);
                    GETEDW(x4, x1, 0);
                    CALL_(rcl32, ed, x4);
                    WBACK;
                    break;
                case 3:
                    INST_NAME("RCR Ed, 1");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET);
                    MOV32w(x2, 1);
                    GETEDW(x4, x1, 0);
                    CALL_(rcr32, ed, x4);
                    WBACK;
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ed, 1");
                    SETFLAGS(X_ALL, SF_SET);    // some flags are left undefined
                    GETED(0);
                    emit_shl32c(dyn, ninst, rex, ed, 1, x3, x4);
                    WBACK;
                    break;
                case 5:
                    INST_NAME("SHR Ed, 1");
                    SETFLAGS(X_ALL, SF_SET);    // some flags are left undefined
                    GETED(0);
                    emit_shr32c(dyn, ninst, rex, ed, 1, x3, x4);
                    WBACK;
                    break;
                case 7:
                    INST_NAME("SAR Ed, 1");
                    SETFLAGS(X_ALL, SF_SET);    // some flags are left undefined
                    GETED(0);
                    emit_sar32c(dyn, ninst, rex, ed, 1, x3, x4);
                    WBACK;
                    break;
            }
            break;
        case 0xD3:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("ROL Ed, CL");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    if(rex.w) {
                        ANDSx_mask(x3, xRCX, 1, 0, 0b00101);  //mask=0x000000000000003f
                    } else {
                        ANDSw_mask(x3, xRCX, 0, 0b00100);  //mask=0x00000001f
                    }
                    MOV64xw(x4, (rex.w?64:32));
                    SUBx_REG(x3, x4, x3);
                    GETEDW(x4, x2, 0);
                    if(!rex.w && MODREG) {MOVw_REG(ed, ed);}
                    B_NEXT(cEQ);
                    RORxw_REG(ed, ed, x3);
                    WBACK;
                    UFLAG_IF {  // calculate flags directly
                        CMPSw_U12(x3, rex.w?63:31);
                        B_MARK(cNE);
                            LSRxw(x1, ed, rex.w?63:31);
                            ADDxw_REG(x1, x1, ed);
                            BFIw(xFlags, x1, F_OF, 1);
                        MARK;
                        BFIw(xFlags, ed, F_CF, 1);
                        UFLAG_DF(x2, d_none);
                    }
                    break;
                case 1:
                    INST_NAME("ROR Ed, CL");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    if(rex.w) {
                        ANDSx_mask(x3, xRCX, 1, 0, 0b00101);  //mask=0x000000000000003f
                    } else {
                        ANDSw_mask(x3, xRCX, 0, 0b00100);  //mask=0x00000001f
                    }
                    GETEDW(x4, x2, 0);
                    if(!rex.w && MODREG) {MOVw_REG(ed, ed);}
                    B_NEXT(cEQ);
                    RORxw_REG(ed, ed, x3);
                    WBACK;
                    UFLAG_IF {  // calculate flags directly
                        CMPSw_U12(x3, 1);
                        B_MARK(cNE);
                            LSRxw(x2, ed, rex.w?62:30); // x2 = d>>30
                            EORw_REG_LSR(x2, x2, x2, 1); // x2 = ((d>>30) ^ ((d>>30)>>1))
                            BFIw(xFlags, x2, F_OF, 1);
                        MARK;
                        LSRxw(x2, ed, rex.w?63:31);
                        BFIw(xFlags, x2, F_CF, 1);
                        UFLAG_DF(x2, d_none);
                    }
                    break;
                case 2:
                    INST_NAME("RCL Ed, CL");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    if(rex.w) {
                        ANDSx_mask(x2, xRCX, 1, 0, 0b00101);  //mask=0x000000000000003f
                    } else {
                        ANDSw_mask(x2, xRCX, 0, 0b00100);  //mask=0x00000001f
                    }
                    GETEDW(x4, x1, 0);
                    if(!rex.w && MODREG) {MOVw_REG(ed, ed);}
                    B_NEXT(cEQ);
                    CALL_(rex.w?((void*)rcl64):((void*)rcl32), ed, x4);
                    WBACK;
                    break;
                case 3:
                    INST_NAME("RCR Ed, CL");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    if(rex.w) {
                        ANDSx_mask(x2, xRCX, 1, 0, 0b00101);  //mask=0x000000000000003f
                    } else {
                        ANDSw_mask(x2, xRCX, 0, 0b00100);  //mask=0x00000001f
                    }
                    GETEDW(x4, x1, 0);
                    if(!rex.w && MODREG) {MOVw_REG(ed, ed);}
                    B_NEXT(cEQ);
                    CALL_(rex.w?((void*)rcr64):((void*)rcr32), ed, x4);
                    WBACK;
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ed, CL");
                    SETFLAGS(X_ALL, SF_SET);    // some flags are left undefined
                    if(rex.w) {
                        ANDSx_mask(x3, xRCX, 1, 0, 0b00101);  //mask=0x000000000000003f
                    } else {
                        ANDSw_mask(x3, xRCX, 0, 0b00100);  //mask=0x00000001f
                    }
                    GETED(0);
                    if(!rex.w && MODREG) {MOVw_REG(ed, ed);}
                    B_NEXT(cEQ);
                    emit_shl32(dyn, ninst, rex, ed, x3, x5, x4);
                    WBACK;
                    break;
                case 5:
                    INST_NAME("SHR Ed, CL");
                    SETFLAGS(X_ALL, SF_SET);    // some flags are left undefined
                    if(rex.w) {
                        ANDSx_mask(x3, xRCX, 1, 0, 0b00101);  //mask=0x000000000000003f
                    } else {
                        ANDSw_mask(x3, xRCX, 0, 0b00100);  //mask=0x00000001f
                    }
                    GETED(0);
                    if(!rex.w && MODREG) {MOVw_REG(ed, ed);}
                    B_NEXT(cEQ);
                    emit_shr32(dyn, ninst, rex, ed, x3, x5, x4);
                    WBACK;
                    break;
                case 7:
                    INST_NAME("SAR Ed, CL");
                    SETFLAGS(X_ALL, SF_PENDING);
                    if(rex.w) {
                        ANDSx_mask(x3, xRCX, 1, 0, 0b00101);  //mask=0x000000000000003f
                    } else {
                        ANDSw_mask(x3, xRCX, 0, 0b00100);  //mask=0x00000001f
                    }
                    GETED(0);
                    if(!rex.w && MODREG) {MOVw_REG(ed, ed);}
                    B_NEXT(cEQ);
                    UFLAG_OP12(ed, x3);
                    ASRxw_REG(ed, ed, x3);
                    WBACK;
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, rex.w?d_sar64:d_sar32);
                    break;
            }
            break;
        
        case 0xE8:
            INST_NAME("CALL Id");
            i32 = F32S;
            if(addr+i32==0) {
                #if STEP == 3
                printf_log(LOG_INFO, "Warning, CALL to 0x0 at %p (%p)\n", (void*)addr, (void*)(addr-1));
                #endif
            }
            #if STEP == 0
            if(isNativeCall(dyn, addr+i32, NULL, NULL))
                tmp = 3;
            else 
                tmp = 0;
            #elif STEP < 2
            if(isNativeCall(dyn, addr+i32, &dyn->insts[ninst].natcall, &dyn->insts[ninst].retn))
                tmp = dyn->insts[ninst].pass2choice = 3;
            else 
                tmp = dyn->insts[ninst].pass2choice = 0;
            #else
                tmp = dyn->insts[ninst].pass2choice;
            #endif
            switch(tmp) {
                case 3:
                    SETFLAGS(X_ALL, SF_SET);    // Hack to set flags to "dont'care" state
                    BARRIER(1);
                    BARRIER_NEXT(1);
                    TABLE64(x2, addr);
                    PUSH1(x2);
                    MESSAGE(LOG_DUMP, "Native Call to %s (retn=%d)\n", GetNativeName(GetNativeFnc(dyn->insts[ninst].natcall-1)), dyn->insts[ninst].retn);
                    // calling a native function
                    sse_purge07cache(dyn, ninst, x3);
                    GETIP_(dyn->insts[ninst].natcall); // read the 0xCC already
                    STORE_XEMU_CALL(xRIP);
                    CALL_S(x64Int3, -1);
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
                    jump_to_epilog(dyn, 0, xRIP, ninst);
                    break;
                default:
                    if(ninst && dyn->insts && dyn->insts[ninst-1].x64.set_flags) {
                        READFLAGS(X_PEND);  // that's suspicious
                    } else {
                        SETFLAGS(X_ALL, SF_SET);    // Hack to set flags to "dont'care" state
                    }
                    // regular call
                    BARRIER(1);
                    BARRIER_NEXT(1);
                    if(!dyn->insts || ninst==dyn->size-1) {
                        *need_epilog = 0;
                        *ok = 0;
                    }
                    TABLE64(x2, addr);
                    PUSH1(x2);
                    if(addr+i32==0) {   // self modifying code maybe? so use indirect address fetching
                        TABLE64(x4, addr-4);
                        LDRx_U12(x4, x4, 0);
                        jump_to_next(dyn, 0, x4, ninst);
                    } else
                        jump_to_next(dyn, addr+i32, 0, ninst);
                    break;
            }
            break;
        case 0xE9:
        case 0xEB:
            BARRIER(1);
            if(opcode==0xE9) {
                INST_NAME("JMP Id");
                i32 = F32S;
            } else {
                INST_NAME("JMP Ib");
                i32 = F8S;
            }
            JUMP(addr+i32);
            if(dyn->insts) {
                PASS2IF(dyn->insts[ninst].x64.jmp_insts==-1, 1) {
                    // out of the block
                    jump_to_next(dyn, addr+i32, 0, ninst);
                } else {
                    // inside the block
                    tmp = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address-(dyn->arm_size);
                    if(tmp==4) {
                        NOP;
                    } else {
                        B(tmp);
                    }
                }
            }
            *need_epilog = 0;
            *ok = 0;
            break;

        case 0xF0:
            addr = dynarec64_F0(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
            break;

        case 0xF6:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                case 1:
                    INST_NAME("TEST Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET);
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
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETEB(x1, 0);
                    emit_neg8(dyn, ninst, x1, x2, x4);
                    EBBACK;
                    break;
                case 4:
                    INST_NAME("MUL AL, Ed");
                    SETFLAGS(X_ALL, SF_PENDING);
                    UFLAG_DF(x1, d_mul8);
                    GETEB(x1, 0);
                    UXTBw(x2, xRAX);
                    MULw(x1, x2, x1);
                    UFLAG_RES(x1);
                    BFIx(xRAX, x1, 0, 16);
                    break;
                case 5:
                    INST_NAME("IMUL AL, Eb");
                    SETFLAGS(X_ALL, SF_PENDING);
                    UFLAG_DF(x1, d_imul8);
                    GETSEB(x1, 0);
                    SXTBw(x2, xRAX);
                    MULw(x1, x2, x1);
                    UFLAG_RES(x1);
                    BFIx(xRAX, x1, 0, 16);
                    break;
                case 6:
                    INST_NAME("DIV Eb");
                    SETFLAGS(X_ALL, SF_SET);
                    GETEB(x1, 0);
                    CALL(div8, -1);
                    break;
                case 7:
                    INST_NAME("IDIV Eb");
                    SETFLAGS(X_ALL, SF_SET);
                    GETEB(x1, 0);
                    CALL(idiv8, -1);
                    break;
            }
            break;
        case 0xF7:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                case 1:
                    INST_NAME("TEST Ed, Id");
                    SETFLAGS(X_ALL, SF_SET);
                    GETEDH(x1, 4);
                    i64 = F32S;
                    MOV64xw(x2, i64);
                    emit_test32(dyn, ninst, rex, ed, x2, x3, x4);
                    break;
                case 2:
                    INST_NAME("NOT Ed");
                    GETED(4);
                    MVNxw_REG(ed, ed);
                    WBACK;
                    break;
                case 3:
                    INST_NAME("NEG Ed");
                    SETFLAGS(X_ALL, SF_SET);
                    GETED(0);
                    emit_neg32(dyn, ninst, rex, ed, x3, x4);
                    WBACK;
                    break;
                case 4:
                    INST_NAME("MUL EAX, Ed");
                    SETFLAGS(X_ALL, SF_PENDING);
                    UFLAG_DF(x2, rex.w?d_mul64:d_mul32);
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
                    break;
                case 5:
                    INST_NAME("IMUL EAX, Ed");
                    SETFLAGS(X_ALL, SF_PENDING);
                    UFLAG_DF(x2, rex.w?d_imul64:d_imul32);
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
                    break;
                case 6:
                    INST_NAME("DIV Ed");
                    SETFLAGS(X_ALL, SF_SET);
                    GETEDH(x1, 0);
                    if(ed!=x1) {MOVxw_REG(x1, ed);}
                    CALL(rex.w?((void*)div64):((void*)div32), -1);
                    break;
                case 7:
                    INST_NAME("IDIV Ed");
                    SETFLAGS(X_ALL, SF_SET);
                    GETEDH(x1, 0);
                    if(ed!=x1) {MOVxw_REG(x1, ed);}
                    CALL(rex.w?((void*)idiv64):((void*)idiv32), -1);
                    break;
            }
            break;
        
        case 0xFE:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("INC Eb");
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET);
                    GETEB(x1, 0);
                    emit_inc8(dyn, ninst, x1, x2, x4);
                    EBBACK;
                    break;
                case 1:
                    INST_NAME("DEC Eb");
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET);
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
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET);
                    GETED(0);
                    emit_inc32(dyn, ninst, rex, ed, x3, x4);
                    WBACK;
                    break;
                case 1: //DEC Ed
                    INST_NAME("DEC Ed");
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET);
                    GETED(0);
                    emit_dec32(dyn, ninst, rex, ed, x3, x4);
                    WBACK;
                    break;
                case 2: // CALL Ed
                    INST_NAME("CALL Ed");
                    PASS2IF(ninst && dyn->insts && dyn->insts[ninst-1].x64.set_flags, 1) {
                        READFLAGS(X_PEND);          // that's suspicious
                    } else {
                        SETFLAGS(X_ALL, SF_SET);    //Hack to put flag in "don't care" state
                    }
                    GETEDx(0);
                    BARRIER(1);
                    BARRIER_NEXT(1);
                    if(!dyn->insts || ninst==dyn->size-1) {
                        *need_epilog = 0;
                        *ok = 0;
                    }
                    GETIP(addr);
                    PUSH1(xRIP);
                    jump_to_next(dyn, 0, ed, ninst);
                    break;
                case 4: // JMP Ed
                    INST_NAME("JMP Ed");
                    BARRIER(1);
                    GETEDx(0);
                    jump_to_next(dyn, 0, ed, ninst);
                    *need_epilog = 0;
                    *ok = 0;
                    break;
                case 6: // Push Ed
                    INST_NAME("PUSH Ed");
                    GETEDx(0);
                    PUSH1(ed);
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

