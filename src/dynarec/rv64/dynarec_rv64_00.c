#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <assert.h>

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

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_functions.h"
#include "dynarec_rv64_helper.h"

int isSimpleWrapper(wrapper_t fun);

uintptr_t dynarec64_00(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
{
    uint8_t nextop, opcode;
    uint8_t gd, ed;
    int8_t i8;
    int32_t i32, tmp;
    int64_t i64, j64;
    uint8_t u8;
    uint8_t gb1, gb2, eb1, eb2;
    uint32_t u32;
    uint64_t u64;
    uint8_t wback, wb1, wb2, wb;
    int64_t fixedaddress;
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
            EBBACK(x5, 0);
            break;
        case 0x01:
            INST_NAME("ADD Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_add32(dyn, ninst, rex, ed, gd, x3, x4, x5);
            WBACK;
            break;
        case 0x02:
            INST_NAME("ADD Gb, Eb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB(x1, 0);
            GETGB(x2);
            emit_add8(dyn, ninst, x2, x1, x4, x5);
            GBBACK(x5);
            break;
        case 0x03:
            INST_NAME("ADD Gd, Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_add32(dyn, ninst, rex, gd, ed, x3, x4, x5);
            break;
        case 0x04:
            INST_NAME("ADD AL, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            u8 = F8;
            ANDI(x1, xRAX, 0xff);
            emit_add8c(dyn, ninst, x1, u8, x3, x4, x5);
            ANDI(xRAX, xRAX, ~0xff);
            OR(xRAX, xRAX, x1);
            break;
        case 0x05:
            INST_NAME("ADD EAX, Id");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i64 = F32S;
            emit_add32c(dyn, ninst, rex, xRAX, i64, x3, x4, x5, x6);
            break;
        case 0x08:
            INST_NAME("OR Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB(x1, 0);
            GETGB(x2);
            emit_or8(dyn, ninst, x1, x2, x4, x5);
            EBBACK(x5, 0);
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
            GETEB(x1, 0);
            GETGB(x2);
            emit_or8(dyn, ninst, x2, x1, x4, x5);
            GBBACK(x5);
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
            ANDI(x1, xRAX, 0xff);
            emit_or8c(dyn, ninst, x1, u8, x3, x4, x5);
            ANDI(xRAX, xRAX, ~0xff);
            OR(xRAX, xRAX, x1);
            break;
        case 0x0D:
            INST_NAME("OR EAX, Id");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i64 = F32S;
            emit_or32c(dyn, ninst, rex, xRAX, i64, x3, x4);
            break;

        case 0x0F:
            switch(rep) {
            case 0:
                addr = dynarec64_0F(dyn, addr, ip, ninst, rex, ok, need_epilog);
                break;
            case 1:
                addr = dynarec64_F20F(dyn, addr, ip, ninst, rex, ok, need_epilog);
                break;
            case 2:
                addr = dynarec64_F30F(dyn, addr, ip, ninst, rex, ok, need_epilog);
                break;
            default:
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
            emit_sbb8(dyn, ninst, x1, x2, x4, x5, x6);
            EBBACK(x5, 0);
            break;
        case 0x19:
            INST_NAME("SBB Ed, Gd");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_sbb32(dyn, ninst, rex, ed, gd, x3, x4, x5);
            WBACK;
            break;
        case 0x1C:
            INST_NAME("SBB AL, Ib");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            u8 = F8;
            ANDI(x1, xRAX, 0xff);
            emit_sbb8c(dyn, ninst, x1, u8, x3, x4, x5, x6);
            ANDI(xRAX, xRAX, ~0xff);
            OR(xRAX, xRAX, x1);
            break;
        case 0x20:
            INST_NAME("AND Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB(x1, 0);
            GETGB(x2);
            emit_and8(dyn, ninst, x1, x2, x4, x5);
            EBBACK(x4, 0);
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
            GETEB(x1, 0);
            GETGB(x2);
            emit_and8(dyn, ninst, x2, x1, x4, x5);
            GBBACK(x5);
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
            ANDI(x1, xRAX, 0xff);
            emit_and8c(dyn, ninst, x1, u8, x3, x4);
            ANDI(xRAX, xRAX, ~0xff);
            OR(xRAX, xRAX, x1);
            break;
        case 0x25:
            INST_NAME("AND EAX, Id");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i64 = F32S;
            emit_and32c(dyn, ninst, rex, xRAX, i64, x3, x4);
            break;
        case 0x29:
            INST_NAME("SUB Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_sub32(dyn, ninst, rex, ed, gd, x3, x4, x5);
            WBACK;
            break;
        case 0x2A:
            INST_NAME("SUB Gb, Eb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB(x1, 0);
            GETGB(x2);
            emit_sub8(dyn, ninst, x2, x1, x4, x5, x6);
            GBBACK(x5);
            break;
        case 0x2B:
            INST_NAME("SUB Gd, Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_sub32(dyn, ninst, rex, gd, ed, x3, x4, x5);
            break;
        case 0x2C:
            INST_NAME("SUB AL, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            u8 = F8;
            ANDI(x1, xRAX, 0xff);
            emit_sub8c(dyn, ninst, x1, u8, x2, x3, x4, x5);
            ANDI(xRAX, xRAX, ~0xff);
            OR(xRAX, xRAX, x1);
            break;
        case 0x2D:
            INST_NAME("SUB EAX, Id");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i64 = F32S;
            emit_sub32c(dyn, ninst, rex, xRAX, i64, x2, x3, x4, x5);
            break;
        case 0x30:
            INST_NAME("XOR Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB(x1, 0);
            GETGB(x2);
            emit_xor8(dyn, ninst, x1, x2, x4, x5);
            EBBACK(x5, 0);
            break;
        case 0x31:
            INST_NAME("XOR Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_xor32(dyn, ninst, rex, ed, gd, x3, x4);
            if(ed!=gd) {
                WBACK;
            }
            break;
        case 0x32:
            INST_NAME("XOR Gb, Eb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB(x1, 0);
            GETGB(x2);
            emit_xor8(dyn, ninst, x2, x1, x4, x5);
            GBBACK(x5);
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
            ANDI(x1, xRAX, 0xff);
            emit_xor8c(dyn, ninst, x1, u8, x3, x4);
            ANDI(xRAX, xRAX, ~0xff);
            OR(xRAX, xRAX, x1);
            break;
        case 0x35:
            INST_NAME("XOR EAX, Id");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i64 = F32S;
            emit_xor32c(dyn, ninst, rex, xRAX, i64, x3, x4);
            break;
        case 0x38:
            INST_NAME("CMP Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB(x1, 0);
            GETGB(x2);
            emit_cmp8(dyn, ninst, x1, x2, x3, x4, x5, x6);
            break;
        case 0x39:
            INST_NAME("CMP Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_cmp32(dyn, ninst, rex, ed, gd, x3, x4, x5, x6);
            break;
        case 0x3A:
            INST_NAME("CMP Gb, Eb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB(x1, 0);
            GETGB(x2);
            emit_cmp8(dyn, ninst, x2, x1, x3, x4, x5, x6);
            break;
        case 0x3B:
            INST_NAME("CMP Gd, Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_cmp32(dyn, ninst, rex, gd, ed, x3, x4, x5, x6);
            break;
        case 0x3C:
            INST_NAME("CMP AL, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            u8 = F8;
            ANDI(x1, xRAX, 0xff);
            if(u8) {
                MOV32w(x2, u8);
                emit_cmp8(dyn, ninst, x1, x2, x3, x4, x5, x6);
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
                emit_cmp32(dyn, ninst, rex, xRAX, x2, x3, x4, x5, x6);
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
            SD(gd, xRSP, -8);
            SUBI(xRSP, xRSP, 8);
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
            LD(gd, xRSP, 0);
            if(gd!=xRSP) {
                ADDI(xRSP, xRSP, 8);
            }
            break;

        case 0x63:
            INST_NAME("MOVSXD Gd, Ed");
            nextop = F8;
            GETGD;
            if(rex.w) {
                if(MODREG) {   // reg <= reg
                    ADDIW(gd, xRAX+(nextop&7)+(rex.b<<3), 0);
                } else {                    // mem <= reg
                    SMREAD();
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                    LW(gd, ed, fixedaddress);
                }
            } else {
                if(MODREG) {   // reg <= reg
                    AND(gd, xRAX+(nextop&7)+(rex.b<<3), xMASK);
                } else {                    // mem <= reg
                    SMREAD();
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                    LWU(gd, ed, fixedaddress);
                }
            }
            break;
        case 0x64:
            addr = dynarec64_64(dyn, addr, ip, ninst, rex, rep, _FS, ok, need_epilog);
            break;
        case 0x66:
            addr = dynarec64_66(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
            break;

        case 0x68:
            INST_NAME("PUSH Id");
            i64 = F32S;
            if(PK(0)==0xC3) {
                MESSAGE(LOG_DUMP, "PUSH then RET, using indirect\n");
                TABLE64(x3, addr-4);
                LW(x1, x3, 0);
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
                    MULH(x3, ed, x4);
                    MULW(gd, ed, x4);
                    UFLAG_OP1(x3);
                    UFLAG_RES(gd);
                    UFLAG_DF(x3, d_imul64);
                } else {
                    MULxw(gd, ed, x4);
                }
            } else {
                // 32bits imul
                UFLAG_IF {
                    MUL(gd, ed, x4);
                    UFLAG_RES(gd);
                    SRLI(x3, gd, 32);
                    UFLAG_OP1(x3);
                    UFLAG_DF(x3, d_imul32);
                } else {
                    MULxw(gd, ed, x4);
                }
                ZEROUP(gd);
            }
            break;
        case 0x6A:
            INST_NAME("PUSH Ib");
            i64 = F8S;
            MOV64x(x3, i64);
            PUSH1(x3);
            break;
        case 0x6B:
            INST_NAME("IMUL Gd, Ed, Id");
            SETFLAGS(X_ALL, SF_PENDING);
            nextop = F8;
            GETGD;
            GETED(4);
            i64 = F8S;
            MOV64xw(x4, i64);
            if(rex.w) {
                // 64bits imul
                UFLAG_IF {
                    MULH(x3, ed, x4);
                    MULW(gd, ed, x4);
                    UFLAG_OP1(x3);
                    UFLAG_RES(gd);
                    UFLAG_DF(x3, d_imul64);
                } else {
                    MULxw(gd, ed, x4);
                }
            } else {
                // 32bits imul
                UFLAG_IF {
                    MUL(gd, ed, x4);
                    UFLAG_RES(gd);
                    SRLI(x3, gd, 32);
                    UFLAG_OP1(x3);
                    UFLAG_DF(x3, d_imul32);
                } else {
                    MULxw(gd, ed, x4);
                }
                ZEROUP(gd);
            }
            break;

        #define GO(GETFLAGS, NO, YES, F)                                \
            READFLAGS(F);                                               \
            i8 = F8S;                                                   \
            BARRIER(BARRIER_MAYBE);                                     \
            JUMP(addr+i8, 1);                                           \
            GETFLAGS;                                                   \
            if(dyn->insts[ninst].x64.jmp_insts==-1 ||                   \
                CHECK_CACHE()) {                                        \
                /* out of the block */                                  \
                i32 = dyn->insts[ninst].epilog-(dyn->native_size);      \
                B##NO##_safe(x1, i32);                                  \
                if(dyn->insts[ninst].x64.jmp_insts==-1) {               \
                    if(!(dyn->insts[ninst].x64.barrier&BARRIER_FLOAT))  \
                        fpu_purgecache(dyn, ninst, 1, x1, x2, x3);      \
                    jump_to_next(dyn, addr+i8, 0, ninst);               \
                } else {                                                \
                    CacheTransform(dyn, ninst, cacheupd, x1, x2, x3);   \
                    i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address-(dyn->native_size);\
                    B(i32);                                             \
                }                                                       \
            } else {                                                    \
                /* inside the block */                                  \
                i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address-(dyn->native_size);    \
                B##YES##_safe(x1, i32);                                 \
            }

        GOCOND(0x70, "J", "ib");

        #undef GO
        
        case 0x80:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0: // ADD
                    INST_NAME("ADD Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEB(x1, 1);
                    u8 = F8;
                    emit_add8c(dyn, ninst, x1, u8, x2, x4, x5);
                    EBBACK(x5, 0);
                    break;
                case 1: // OR
                    INST_NAME("OR Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEB(x1, 1);
                    u8 = F8;
                    emit_or8c(dyn, ninst, x1, u8, x2, x4, x5);
                    EBBACK(x5, 0);
                    break;
                case 3: // SBB
                    INST_NAME("SBB Eb, Ib");
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEB(x1, 1);
                    u8 = F8;
                    emit_sbb8c(dyn, ninst, x1, u8, x2, x4, x5, x6);
                    EBBACK(x5, 0);
                    break;
                case 4: // AND
                    INST_NAME("AND Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEB(x1, 1);
                    u8 = F8;
                    emit_and8c(dyn, ninst, x1, u8, x2, x4);
                    EBBACK(x5, 0);
                    break;
                case 5: // SUB
                    INST_NAME("SUB Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEB(x1, 1);
                    u8 = F8;
                    emit_sub8c(dyn, ninst, x1, u8, x2, x4, x5, x6);
                    EBBACK(x5, 0);
                    break;
                case 6: // XOR
                    INST_NAME("XOR Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEB(x1, 1);
                    u8 = F8;
                    emit_xor8c(dyn, ninst, x1, u8, x2, x4);
                    EBBACK(x5, 0);
                    break;
                case 7: // CMP
                    INST_NAME("CMP Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEB(x1, 1);
                    u8 = F8;
                    if(u8) {
                        ADDI(x2, xZR, u8);
                        emit_cmp8(dyn, ninst, x1, x2, x3, x4, x5, x6);
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
                case 0: // ADD
                    if(opcode==0x81) {INST_NAME("ADD Ed, Id");} else {INST_NAME("ADD Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETED((opcode==0x81)?4:1);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    emit_add32c(dyn, ninst, rex, ed, i64, x3, x4, x5, x6);
                    WBACK;
                    break;
                case 1: // OR
                    if(opcode==0x81) {INST_NAME("OR Ed, Id");} else {INST_NAME("OR Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETED((opcode==0x81)?4:1);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    emit_or32c(dyn, ninst, rex, ed, i64, x3, x4);
                    WBACK;
                    break;
                case 2: // ADC
                    if(opcode==0x81) {INST_NAME("ADC Ed, Id");} else {INST_NAME("ADC Ed, Ib");}
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETED((opcode==0x81)?4:1);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    MOV64xw(x5, i64);
                    emit_adc32(dyn, ninst, rex, ed, x5, x3, x4, x6);
                    WBACK;
                    break;
                case 3: // SBB
                    if(opcode==0x81) {INST_NAME("SBB Ed, Id");} else {INST_NAME("SBB Ed, Ib");}
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETED((opcode==0x81)?4:1);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    MOV64xw(x5, i64);
                    emit_sbb32(dyn, ninst, rex, ed, x5, x3, x4, x6);
                    WBACK;
                    break;
                case 4: // AND
                    if(opcode==0x81) {INST_NAME("AND Ed, Id");} else {INST_NAME("AND Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETED((opcode==0x81)?4:1);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    emit_and32c(dyn, ninst, rex, ed, i64, x3, x4);
                    WBACK;
                    break;
                case 5: // SUB
                    if(opcode==0x81) {INST_NAME("SUB Ed, Id");} else {INST_NAME("SUB Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETED((opcode==0x81)?4:1);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    emit_sub32c(dyn, ninst, rex, ed, i64, x3, x4, x5, x6);
                    WBACK;
                    break;
                case 6: // XOR
                    if(opcode==0x81) {INST_NAME("XOR Ed, Id");} else {INST_NAME("XOR Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETED((opcode==0x81)?4:1);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    emit_xor32c(dyn, ninst, rex, ed, i64, x3, x4);
                    WBACK;
                    break;
                case 7: // CMP
                    if(opcode==0x81) {INST_NAME("CMP Ed, Id");} else {INST_NAME("CMP Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETED((opcode==0x81)?4:1);
                    if(opcode==0x81) i64 = F32S; else i64 = F8S;
                    if(i64) {
                        MOV64xw(x2, i64);
                        emit_cmp32(dyn, ninst, rex, ed, x2, x3, x4, x5, x6);
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
            emit_test32(dyn, ninst, rex, ed, gd, x3, x4, x5);
            break;
        case 0x86:
            INST_NAME("(LOCK)XCHG Eb, Gb");
            nextop = F8;
            if(MODREG) {
                GETGB(x1);
                GETEB(x2, 0);
                MV(x4, gd);
                MV(gd, ed);
                MV(ed, x4);
                GBBACK(x4);
                EBBACK(x4, 0);
            } else {
                GETGB(x3);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                SMDMB();
                LBU(x1, ed, 0);
                SB(gd, ed, 0);
                SMDMB();
                gd = x1;
                GBBACK(x3);
            }
            break;
        case 0x87:
            INST_NAME("(LOCK)XCHG Ed, Gd");
            nextop = F8;
            if(MODREG) {
                GETGD;
                GETED(0);
                MVxw(x1, gd);
                MVxw(gd, ed);
                MVxw(ed, x1);
            } else {
                GETGD;
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                SMDMB();
                ANDI(x3, ed, (1<<(2+rex.w))-1);
                BNE_MARK(x3, xZR);
                MARKLOCK;
                LRxw(x1, ed, 1, 0);
                SCxw(x3, gd, ed, 0, 1);
                BNE_MARKLOCK(x3, xZR);
                B_MARK2_nocond;
                MARK;
                LDxw(x1, ed, 0);
                SDxw(gd, ed, 0);
                MARK2;
                SMDMB();
                MVxw(gd, x1);
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
            gd = x4;
            if(gb2) {
                SRLI(x4, gb1, 8);
                gb1 = x4;
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
                ANDI(gd, gb1, 0xff);
                if(eb2) {
                    MOV64x(x1, 0xffffffffffff00ffLL);
                    ANDI(x1, eb1, x1);
                    SLLI(gd, gd, 8);
                    OR(eb1, x1, gd);
                } else {
                    ANDI(x1, eb1, ~0xff);
                    OR(eb1, x1, gd);
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, 1, 0);
                SB(gb1, ed, fixedaddress);
                SMWRITELOCK(lock);
            }
            break;
        case 0x89:
            INST_NAME("MOV Ed, Gd");
            nextop=F8;
            GETGD;
            if(MODREG) {   // reg <= reg
                MVxw(xRAX+(nextop&7)+(rex.b<<3), gd);
            } else {                    // mem <= reg
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, 1, 0);
                SDxw(gd, ed, fixedaddress);
                SMWRITELOCK(lock);
            }
            break;
        case 0x8A:
            INST_NAME("MOV Gb, Eb");
            nextop = F8;
            gd = ((nextop&0x38)>>3)+(rex.r<<3);
            if(rex.rex) {
                gb2 = 0;
                gb1 = xRAX + gd;
            } else {
                gb2 = ((gd&4)>>2);
                gb1 = xRAX+(gd&3);
            }
            gd = x4;
            if(MODREG) {
                ed = (nextop&7) + (rex.b<<3);
                if(rex.rex) {
                    eb1 = xRAX+ed;
                    eb2 = 0;
                } else {
                    eb1 = xRAX+(ed&3);  // Ax, Cx, Dx or Bx
                    eb2 = ((ed&4)>>2);    // L or H
                }
                if(eb2) {
                    SRLI(x1, eb1, 8);
                    ANDI(x1, x1, 0xff);
                } else {
                    ANDI(x1, eb1, 0xff);
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, 1, 0);
                SMREADLOCK(lock);
                LBU(x1, ed, fixedaddress);
            }
            if(gb2) {
                MOV64x(x4, ~0xff00);
                AND(gb1, gb1, x4);
                SLLI(x1, x1, 8);
            } else {
                ANDI(gb1, gb1, ~0xff);
            }
            OR(gb1, gb1, x1);
            break;
        case 0x8B:
            INST_NAME("MOV Gd, Ed");
            nextop=F8;
            GETGD;
            if(MODREG) {
                MVxw(gd, xRAX+(nextop&7)+(rex.b<<3));
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, 1, 0);
                SMREADLOCK(lock);
                LDxw(gd, ed, fixedaddress);
            }
            break;

        case 0x8D:
            INST_NAME("LEA Gd, Ed");
            nextop=F8;
            GETGD;
            if(MODREG) {   // reg <= reg? that's an invalid operation
                DEFAULT;
            } else {                    // mem <= reg
                addr = geted(dyn, addr, ninst, nextop, &ed, gd, x1, &fixedaddress, rex, NULL, 0, 0);
                if(gd!=ed) {    // it's sometimes used as a 3 bytes NOP
                    MV(gd, ed);
                }
                if(!rex.w) {
                    ZEROUP(gd);   //truncate the higher 32bits as asked
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
                MVxw(x2, xRAX);
                MVxw(xRAX, gd);
                MVxw(gd, x2);
            }
            break;
        case 0x98:
            INST_NAME("CWDE");
            if(rex.w) {
                SEXT_W(xRAX, xRAX);
            } else {
                SLLI(xRAX, xRAX, 16);
                SRAIW(xRAX, xRAX, 16);
                ZEROUP(xRAX);
            }
            break;
        case 0x99:
            INST_NAME("CDQ");
            if(rex.w) {
                SRAI(xRDX, xRAX, 63);
            } else {
                SLLI(xRDX, xRAX, 32);
                SRAI(xRDX, xRDX, 63);
                ZEROUP(xRDX);
            }
            break;
        case 0x9C:
            INST_NAME("PUSHF");
            READFLAGS(X_ALL);
            PUSH1(xFlags);
            break;
        case 0xA4:
            if(rep) {
                INST_NAME("REP MOVSB");
                CBZ_NEXT(xRCX);
                ANDI(x1, xFlags, 1<<F_DF);
                BNEZ_MARK2(x1);
                MARK;   // Part with DF==0
                LBU(x1, xRSI, 0);
                SB(x1, xRDI, 0);
                ADDI(xRSI, xRSI, 1);
                ADDI(xRDI, xRDI, 1);
                SUBI(xRCX, xRCX, 1);
                BNEZ_MARK(xRCX);
                B_NEXT_nocond;
                MARK2;  // Part with DF==1
                LBU(x1, xRSI, 0);
                LBU(x1, xRDI, 0);
                SUBI(xRSI, xRSI, 1);
                SUBI(xRDI, xRDI, 1);
                SUBI(xRCX, xRCX, 1);
                BNEZ_MARK2(xRCX);
                // done
            } else {
                INST_NAME("MOVSB");
                GETDIR(x3, x1, 1);
                LBU(x1, xRSI, 0);
                SB(x1, xRDI, 0);
                ADD(xRSI, xRSI, x3);
                ADD(xRDI, xRDI, x3);
            }
            break;
        case 0xA5:
            if(rep) {
                INST_NAME("REP MOVSD");
                CBZ_NEXT(xRCX);
                ANDI(x1, xFlags, 1<<F_DF);
                BNEZ_MARK2(x1);
                MARK;   // Part with DF==0
                LDxw(x1, xRSI, 0);
                ADDI(xRSI, xRSI, rex.w?8:4);
                SDxw(x1, xRDI, 0);
                ADDI(xRDI, xRDI, rex.w?8:4);
                SUBI(xRCX, xRCX, 1);
                BNEZ_MARK(xRCX);
                B_NEXT_nocond;
                MARK2;  // Part with DF==1
                LDxw(x1, xRSI, 0);
                SUBI(xRSI, xRSI, rex.w?8:4);
                SDxw(x1, xRDI, 0);
                SUBI(xRDI, xRDI, rex.w?8:4);
                SUBI(xRCX, xRCX, 1);
                BNEZ_MARK2(xRCX);
                // done
            } else {
                INST_NAME("MOVSD");
                GETDIR(x3, x1, rex.w?8:4);
                LDxw(x1, xRSI, 0);
                SDxw(x1, xRDI, 0);
                ADD(xRSI, xRSI, x3);
                ADD(xRDI, xRDI, x3);
            }
            break;
        case 0xA8:
            INST_NAME("TEST AL, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            ANDI(x1, xRAX, 0xff);
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
        case 0xAB:
            if(rep) {
                INST_NAME("REP STOSD");
                CBZ_NEXT(xRCX);
                ANDI(x1, xFlags, 1<<F_DF);
                BNEZ_MARK2(x1);
                MARK;   // Part with DF==0
                SDxw(xRAX, xRDI, 0);
                ADDI(xRDI, xRDI, rex.w?8:4);
                SUBI(xRCX, xRCX, 1);
                BNEZ_MARK(xRCX);
                B_NEXT_nocond;
                MARK2;  // Part with DF==1
                SDxw(xRAX, xRDI, 0);
                SUBI(xRDI, xRDI, rex.w?8:4);
                SUBI(xRCX, xRCX, 1);
                BNEZ_MARK2(xRCX);
                // done
            } else {
                INST_NAME("STOSD");
                GETDIR(x3, x1, rex.w?8:4);
                SDxw(xRAX, xRDI, 0);
                ADD(xRDI, xRDI, x3);
            }
            break;
        case 0xB0:
        case 0xB1:
        case 0xB2:
        case 0xB3:
            INST_NAME("MOV xL, Ib");
            u8 = F8;
            if(rex.rex)
                gb1 = xRAX+(opcode&7)+(rex.b<<3);
            else
                gb1 = xRAX+(opcode&3);
            ANDI(gb1, gb1, ~0xff);
            ORI(gb1, gb1, u8);
            break;
        case 0xB4:
        case 0xB5:
        case 0xB6:
        case 0xB7:
            INST_NAME("MOV xH, Ib");
            u8 = F8;
            if(rex.rex) {
                gb1 = xRAX+(opcode&7)+(rex.b<<3);
                ANDI(gb1, gb1, ~0xff);
                ORI(gb1, gb1, u8);
            } else {
                MOV32w(x1, u8);
                gb1 = xRAX+(opcode&3);
                MOV64x(x2, 0xffffffffffff00ffLL);
                AND(gb1, gb1, x2);
                SLLI(x1, x1, 8);
                OR(gb1, gb1, x1);
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
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    SETFLAGS(X_OF|X_CF, SF_SET);
                    GETEB(x1, 1);
                    u8 = F8;
                    MOV32w(x2, u8);
                    CALL_(rol8, ed, x3);
                    EBBACK(x5, 0);
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Eb, Ib");
                    GETEB(x1, 1);
                    u8 = (F8)&0x1f;
                    if(u8) {
                        SETFLAGS(X_ALL, SF_PENDING);
                        UFLAG_IF{
                            MOV32w(x4, u8); UFLAG_OP2(x4);
                        };
                        UFLAG_OP1(ed);
                        SLLIW(ed, ed, u8);
                        EBBACK(x5, 1);
                        UFLAG_RES(ed);
                        UFLAG_DF(x3, d_shl8);
                    } else {
                        NOP();
                    }
                    break;
                case 5:
                    INST_NAME("SHR Eb, Ib");
                    GETEB(x1, 1);
                    u8 = (F8)&0x1f;
                    if(u8) {
                        SETFLAGS(X_ALL, SF_PENDING);
                        UFLAG_IF{
                            MOV32w(x4, u8); UFLAG_OP2(x4);
                        };
                        UFLAG_OP1(ed);
                        if(u8) {
                            SRLIW(ed, ed, u8);
                            EBBACK(x5, 1);
                        }
                        UFLAG_RES(ed);
                        UFLAG_DF(x3, d_shr8);
                    } else {
                        NOP();
                    }
                    break;
                case 7:
                    INST_NAME("SAR Eb, Ib");
                    GETSEB(x1, 1);
                    u8 = (F8)&0x1f;
                    if(u8) {
                        SETFLAGS(X_ALL, SF_PENDING);
                        UFLAG_IF{
                            MOV32w(x4, u8); UFLAG_OP2(x4);
                        };
                        UFLAG_OP1(ed);
                        if(u8) {
                            SRAIW(ed, ed, u8);
                            EBBACK(x5, 1);
                        }
                        UFLAG_RES(ed);
                        UFLAG_DF(x3, d_sar8);
                    } else {
                        NOP();
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xC1:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 4:
                case 6:
                    INST_NAME("SHL Ed, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    GETED(1);
                    u8 = (F8)&(rex.w?0x3f:0x1f);
                    emit_shl32c(dyn, ninst, rex, ed, u8, x3, x4, x5);
                    if(u8) WBACK;
                    break;
                case 5:
                    INST_NAME("SHR Ed, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    GETED(1);
                    u8 = (F8)&(rex.w?0x3f:0x1f);
                    emit_shr32c(dyn, ninst, rex, ed, u8, x3, x4);
                    if(u8) { WBACK; }
                    break;
                case 7:
                    INST_NAME("SAR Ed, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    GETED(1);
                    u8 = (F8)&(rex.w?0x3f:0x1f);
                    emit_sar32c(dyn, ninst, rex, ed, u8, x3, x4);
                    if(u8) { WBACK; }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xC2:
            INST_NAME("RETN");
            //SETFLAGS(X_ALL, SF_SET);    // Hack, set all flags (to an unknown state...)
            if(box64_dynarec_safeflags) {
                READFLAGS(X_PEND);  // lets play safe here too
            }
            BARRIER(BARRIER_FLOAT);
            i32 = F16;
            retn_to_epilog(dyn, ninst, i32);
            *need_epilog = 0;
            *ok = 0;
            break;
        case 0xC3:
            INST_NAME("RET");
            // SETFLAGS(X_ALL, SF_SET);    // Hack, set all flags (to an unknown state...)
            if(box64_dynarec_safeflags) {
                READFLAGS(X_PEND);  // so instead, force the deferred flags, so it's not too slow, and flags are not lost
            }
            BARRIER(BARRIER_FLOAT);
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

                if (eb2) {
                    // load a mask to x3 (ffffffffffff00ff)
                    LUI(x3, 0xffffffffffff0);
                    ORI(x3, x3, 0xff);
                    // apply mask
                    AND(eb1, eb1, x3);
                    if(u8) {
                        if((u8<<8)<2048) {
                            ADDI(x4, xZR, u8<<8);
                        } else {
                            ADDI(x4, xZR, u8);
                            SLLI(x4, x4, 8);
                        }
                        OR(eb1, eb1, x4);
                    }
                } else {
                    ANDI(eb1, eb1, 0xf00);  // mask ffffffffffffff00
                    ORI(eb1, eb1, u8);
                }
            } else {                    // mem <= u8
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, &lock, 0, 1);
                u8 = F8;
                if(u8) {
                    ADDI(x3, xZR, u8);
                    ed = x3;
                } else
                    ed = xZR;
                SB(ed, wback, fixedaddress);
                SMWRITELOCK(lock);
            }
            break;
        case 0xC7:
            INST_NAME("MOV Ed, Id");
            nextop=F8;
            if(MODREG) {    // reg <= i32
                i64 = F32S;
                ed = xRAX+(nextop&7)+(rex.b<<3);
                MOV64xw(ed, i64);
            } else {        // mem <= i32
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, &lock, 0, 4);
                i64 = F32S;
                if(i64) {
                    MOV64xw(x3, i64);
                    ed = x3;
                } else
                    ed = xZR;
                SDxw(ed, wback, fixedaddress);
                SMWRITELOCK(lock);
            }
            break;

        case 0xC9:
            INST_NAME("LEAVE");
            MV(xRSP, xRBP);
            POP1(xRBP);
            break;

        case 0xCC:
            SETFLAGS(X_ALL, SF_SET);    // Hack, set all flags (to an unknown state...)
            SKIPTEST(x1);
            if(PK(0)=='S' && PK(1)=='C') {
                addr+=2;
                BARRIER(BARRIER_FLOAT);
                INST_NAME("Special Box64 instruction");
                if((PK64(0)==0))
                {
                    addr+=8;
                    MESSAGE(LOG_DEBUG, "Exit x64 Emu\n");
                    //GETIP(ip+1+2);    // no use
                    //STORE_XEMU_REGS(xRIP);    // no need, done in epilog
                    MOV64x(x1, 1);
                    SW(x1, xEmu, offsetof(x64emu_t, quit));
                    *ok = 0;
                    *need_epilog = 1;
                } else {
                    MESSAGE(LOG_DUMP, "Native Call to %s\n", GetNativeName(GetNativeFnc(ip)));
                    x87_forget(dyn, ninst, x3, x4, 0);
                    sse_purge07cache(dyn, ninst, x3);
                    // disabling isSimpleWrapper because all signed value less than 64bits needs to be sign extended
                    // and return value needs to be cleanned up
                    tmp = 0;//isSimpleWrapper(*(wrapper_t*)(addr));
                    if(tmp<0 || tmp>1)
                        tmp=0;  //TODO: removed when FP is in place
                    if((box64_log<2 && !cycle_log) && tmp) {
                        //GETIP(ip+3+8+8); // read the 0xCC
                        call_n(dyn, ninst, *(void**)(addr+8), tmp);
                        addr+=8+8;
                    } else {
                        GETIP(ip+1); // read the 0xCC
                        STORE_XEMU_CALL();
                        ADDI(x1, xEmu, (uint32_t)offsetof(x64emu_t, ip)); // setup addr as &emu->ip
                        CALL_S(x64Int3, -1);
                        LOAD_XEMU_CALL();
                        addr+=8+8;
                        TABLE64(x3, addr); // expected return address
                        BNE_MARK(xRIP, x3);
                        LW(w1, xEmu, offsetof(x64emu_t, quit));
                        CBZ_NEXT(w1);
                        MARK;
                        jump_to_epilog_fast(dyn, 0, xRIP, ninst);
                    }
                }
            } else {
                #if 1
                INST_NAME("INT 3");
                // check if TRAP signal is handled
                LD(x1, xEmu, offsetof(x64emu_t, context));
                MOV64x(x2, offsetof(box64context_t, signals[SIGTRAP]));
                ADD(x2, x2, x1);
                LD(x3, x2, 0);
                CBNZ_NEXT(x3);
                MOV64x(x1, SIGTRAP);
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
                case 5:
                    if(opcode==0xD0) {
                        INST_NAME("SHR Eb, 1");
                        MOV32w(x4, 1);
                    } else {
                        INST_NAME("SHR Eb, CL");
                        ANDI(x4, xRCX, 0x1F);
                    }
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETEB(x1, 0);
                    UFLAG_OP12(ed, x4);
                    SRLW(ed, ed, x4);
                    EBBACK(x5, 1);
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, d_shr8);
                    break;
                case 7:
                    if(opcode==0xD0) {
                        INST_NAME("SAR Eb, 1");
                        MOV32w(x4, 1);
                    } else {
                        INST_NAME("SAR Eb, CL");
                        ANDI(x4, xRCX, 0x1f);
                    }
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETSEB(x1, 0);
                    UFLAG_OP12(ed, x4)
                    SRA(ed, ed, x4);
                    EBBACK(x3, 1);
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, d_sar8);
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xD1:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 5:
                    INST_NAME("SHR Ed, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    GETED(1);
                    emit_shr32c(dyn, ninst, rex, ed, 1, x3, x4);
                    WBACK;
                    break;
                case 7:
                    INST_NAME("SAR Ed, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    GETED(1);
                    emit_sar32c(dyn, ninst, rex, ed, 1, x3, x4);
                    WBACK;
                    break;
                default:
                    DEFAULT;
            }
            break;

        case 0xD3:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 4:
                case 6:
                    INST_NAME("SHL Ed, CL");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    ANDI(x3, xRCX, rex.w?0x3f:0x1f);
                    GETED(0);
                    if(!rex.w && MODREG) {ZEROUP(ed);}
                    CBZ_NEXT(x3);
                    emit_shl32(dyn, ninst, rex, ed, x3, x5, x4, x6);
                    WBACK;
                    break;
                case 5:
                    INST_NAME("SHR Ed, CL");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    ANDI(x3, xRCX, rex.w?0x3f:0x1f);
                    GETED(0);
                    if(!rex.w && MODREG) {ZEROUP(ed);}
                    CBZ_NEXT(x3);
                    emit_shr32(dyn, ninst, rex, ed, x3, x5, x4);
                    WBACK;
                    break;
                case 7:
                    INST_NAME("SAR Ed, CL");
                    SETFLAGS(X_ALL, SF_PENDING);
                    ANDI(x3, xRCX, rex.w?0x3f:0x1f);
                    GETED(0);
                    if(!rex.w && MODREG) {ZEROUP(ed);}
                    CBZ_NEXT(x3);
                    UFLAG_OP12(ed, x3);
                    SRAxw(ed, ed, x3);
                    WBACK;
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, rex.w?d_sar64:d_sar32);
                    break;
                default:
                    DEFAULT;
            }
            break;

        case 0xD8:
            addr = dynarec64_D8(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
            break;
        case 0xD9:
            addr = dynarec64_D9(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
            break;

        case 0xDB:
            addr = dynarec64_DB(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
            break;

        case 0xDE:
            addr = dynarec64_DE(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
            break;
        case 0xDF:
            addr = dynarec64_DF(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
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
                    SKIPTEST(x1);
                    BARRIER(BARRIER_FULL);
                    //BARRIER_NEXT(BARRIER_FULL);
                    if(dyn->last_ip && (addr-dyn->last_ip<0x1000)) {
                        ADDI(x2, xRIP, addr-dyn->last_ip);
                    } else {
                        TABLE64(x2, addr);
                    }
                    PUSH1(x2);
                    MESSAGE(LOG_DUMP, "Native Call to %s (retn=%d)\n", GetNativeName(GetNativeFnc(dyn->insts[ninst].natcall-1)), dyn->insts[ninst].retn);
                    // calling a native function
                    sse_purge07cache(dyn, ninst, x3);
                    if((box64_log<2 && !cycle_log) && dyn->insts[ninst].natcall) {
                        // disabling isSimpleWrapper because all signed value less than 64bits needs to be sign extended
                        // and return value needs to be cleanned up
                        tmp=0;//isSimpleWrapper(*(wrapper_t*)(dyn->insts[ninst].natcall+2));
                        if(tmp>1 || tmp<0)
                            tmp=0;  // float paramters not ready!
                    } else
                        tmp=0;
                    if((box64_log<2 && !cycle_log) && dyn->insts[ninst].natcall && tmp) {
                        //GETIP(ip+3+8+8); // read the 0xCC
                        call_n(dyn, ninst, *(void**)(dyn->insts[ninst].natcall+2+8), tmp);
                        POP1(xRIP);       // pop the return address
                        dyn->last_ip = addr;
                    } else {
                        GETIP_(dyn->insts[ninst].natcall); // read the 0xCC already
                        STORE_XEMU_CALL();
                        ADDI(x1, xEmu, (uint32_t)offsetof(x64emu_t, ip)); // setup addr as &emu->ip
                        CALL_S(x64Int3, -1);
                        LOAD_XEMU_CALL();
                        TABLE64(x3, dyn->insts[ninst].natcall);
                        ADDI(x3, x3, 2+8+8);
                        BNE_MARK(xRIP, x3);    // Not the expected address, exit dynarec block
                        POP1(xRIP);       // pop the return address
                        if(dyn->insts[ninst].retn) {
                            if(dyn->insts[ninst].retn<0x1000) {
                                ADDI(xRSP, xRSP, dyn->insts[ninst].retn);
                            } else {
                                MOV64x(x3, dyn->insts[ninst].retn);
                                ADD(xRSP, xRSP, x3);
                            }
                        }
                        TABLE64(x3, addr);
                        BNE_MARK(xRIP, x3);    // Not the expected address again
                        LW(w1, xEmu, offsetof(x64emu_t, quit));
                        CBZ_NEXT(w1);
                        MARK;
                        jump_to_epilog_fast(dyn, 0, xRIP, ninst);
                        dyn->last_ip = addr;
                    }
                    break;
                default:
                    if((box64_dynarec_safeflags>1) || (ninst && dyn->insts[ninst-1].x64.set_flags)) {
                        READFLAGS(X_PEND);  // that's suspicious
                    } else {
                        SETFLAGS(X_ALL, SF_SET);    // Hack to set flags to "dont'care" state
                    }
                    // regular call
                    //BARRIER_NEXT(1);
                    if(box64_dynarec_callret && box64_dynarec_bigblock>1) {
                        BARRIER(BARRIER_FULL);
                    } else {
                        BARRIER(BARRIER_FLOAT);
                        *need_epilog = 0;
                        *ok = 0;
                    }
                    if(addr<0x100000000LL) {
                        MOV64x(x2, addr);
                    } else {
                        TABLE64(x2, addr);
                    }
                    PUSH1(x2);
                    // TODO: Add support for CALLRET optim
                    /*if(box64_dynarec_callret) {
                        // Push actual return address
                        if(addr < (dyn->start+dyn->isize)) {
                            // there is a next...
                            j64 = (dyn->insts)?(dyn->insts[ninst].epilog-(dyn->native_size)):0;
                            ADR_S20(x4, j64);
                        } else {
                            j64 = getJumpTableAddress64(addr);
                            TABLE64(x4, j64);
                            LDR(x4, x4, 0);
                        }
                        PUSH1(x4);
                        PUSH1(x2);
                    } else */ //CALLRET optim disable for now.
                    {
                        *ok = 0;
                        *need_epilog = 0;
                    }
                    if(addr+i32==0) {   // self modifying code maybe? so use indirect address fetching
                        if(addr-4<0x100000000LL) {
                            MOV64x(x4, addr-4);
                        } else {
                            TABLE64(x4, addr-4);
                        }
                        LD(x4, x4, 0);
                        jump_to_next(dyn, 0, x4, ninst);
                    } else
                        jump_to_next(dyn, addr+i32, 0, ninst);
                    break;
            }
            break;
        case 0xE9:
        case 0xEB:
            BARRIER(BARRIER_MAYBE);
            if(opcode==0xE9) {
                INST_NAME("JMP Id");
                i32 = F32S;
            } else {
                INST_NAME("JMP Ib");
                i32 = F8S;
            }
            JUMP(addr+i32, 0);
            if(dyn->insts[ninst].x64.jmp_insts==-1) {
                // out of the block
                fpu_purgecache(dyn, ninst, 1, x1, x2, x3);
                jump_to_next(dyn, addr+i32, 0, ninst);
            } else {
                // inside the block
                CacheTransform(dyn, ninst, CHECK_CACHE(), x1, x2, x3);
                tmp = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address-(dyn->native_size);
                MESSAGE(1, "Jump to %d / 0x%x\n", tmp, tmp);
                if(tmp==4) {
                    NOP();
                } else {
                    B(tmp);
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
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEB(x1, 1);
                    u8 = F8;
                    MOV32w(x2, u8);
                    emit_test8(dyn, ninst, x1, x2, x3, x4, x5);
                    break;
                case 2:
                    INST_NAME("NOT Eb");
                    GETEB(x1, 0);
                    NOT(x1, x1);
                    EBBACK(x5, 1);
                    break;
                case 3:
                    INST_NAME("NEG Eb");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEB(x1, 0);
                    emit_neg8(dyn, ninst, x1, x2, x4);
                    EBBACK(x5, 0);
                    break;
                case 4:
                    INST_NAME("MUL AL, Ed");
                    SETFLAGS(X_ALL, SF_PENDING);
                    UFLAG_DF(x1, d_mul8);
                    GETEB(x1, 0);
                    ANDI(x2, xRAX, 0xff);
                    MULW(x1, x2, x1);
                    UFLAG_RES(x1);
                    LUI(x2, 0xffff0);
                    AND(xRAX, xRAX, x2);
                    SLLI(x1, x1, 48);
                    SRLI(x1, x1, 48);
                    OR(xRAX, xRAX, x1);
                    break;
                case 5:
                    INST_NAME("IMUL AL, Eb");
                    SETFLAGS(X_ALL, SF_PENDING);
                    UFLAG_DF(x1, d_imul8);
                    GETSEB(x1, 0);
                    SLLI(x2, xRAX, 56);
                    SRAI(x2, x2, 56);
                    MULW(x1, x2, x1);
                    UFLAG_RES(x1);
                    LUI(x2, 0xffff0);
                    AND(xRAX, xRAX, x2);
                    SLLI(x1, x1, 48);
                    SRLI(x1, x1, 48);
                    OR(xRAX, xRAX, x1);
                    break;
                case 6:
                    INST_NAME("DIV Eb");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    SETFLAGS(X_ALL, SF_SET);
                    GETEB(x1, 0);
                    CALL(div8, -1);
                    break;
                case 7:
                    INST_NAME("IDIV Eb");
                    SKIPTEST(x1);
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    SETFLAGS(X_ALL, SF_SET);
                    GETEB(x1, 0);
                    CALL(idiv8, -1);
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
                    INST_NAME("TEST Ed, Id");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETED(4);
                    i64 = F32S;
                    emit_test32c(dyn, ninst, rex, ed, i64, x3, x4, x5);
                    break;
                case 2:
                    INST_NAME("NOT Ed");
                    GETED(0);
                    XORI(ed, ed, -1);
                    if(!rex.w && MODREG)
                        ZEROUP(ed);
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
                    UFLAG_DF(x2, rex.w?d_mul64:d_mul32);
                    GETED(0);
                    if(rex.w) {
                        if(ed==xRDX) gd=x3; else gd=xRDX;
                        MULHU(gd, xRAX, ed);
                        MUL(xRAX, xRAX, ed);
                        if(gd!=xRDX) {MV(xRDX, gd);}
                    } else {
                        MUL(xRDX, xRAX, ed);  //64 <- 32x32
                        AND(xRAX, xRDX, xMASK);
                        SRLI(xRDX, xRDX, 32);
                    }
                    UFLAG_RES(xRAX);
                    UFLAG_OP1(xRDX);
                    break;
                case 5:
                    INST_NAME("IMUL EAX, Ed");
                    SETFLAGS(X_ALL, SF_PENDING);
                    UFLAG_DF(x2, rex.w?d_imul64:d_imul32);
                    GETSED(0);
                    if(rex.w) {
                        if(ed==xRDX) gd=x3; else gd=xRDX;
                        MULH(gd, xRAX, ed);
                        MUL(xRAX, xRAX, ed);
                        if(gd!=xRDX) {MV(xRDX, gd);}
                    } else {
                        MUL(xRDX, xRAX, ed);  //64 <- 32x32
                        AND(xRAX, xRDX, xMASK);
                        SRLI(xRDX, xRDX, 32);
                    }
                    UFLAG_RES(xRAX);
                    UFLAG_OP1(xRDX);
                    break;
                case 6:
                    INST_NAME("DIV Ed");
                    SETFLAGS(X_ALL, SF_SET);
                    if(!rex.w) {
                        SET_DFNONE();
                        GETED(0);
                        SLLI(x3, xRDX, 32);
                        AND(x2, xRAX, xMASK);
                        OR(x3, x3, x2);
                        if(MODREG) {
                            AND(x4, ed, xMASK);
                            ed = x4;
                        }
                        DIVU(x2, x3, ed);
                        REMU(xRDX, x3, ed);
                        AND(xRAX, x2, xMASK);
                        ZEROUP(xRDX);
                    } else {
                        if(ninst 
                           && dyn->insts[ninst-1].x64.addr 
                           && *(uint8_t*)(dyn->insts[ninst-1].x64.addr)==0x31 
                           && *(uint8_t*)(dyn->insts[ninst-1].x64.addr+1)==0xD2) {
                            SET_DFNONE();
                            GETED(0);
                            DIVU(x2, xRAX, ed);
                            REMU(xRDX, xRAX, ed);
                            MV(xRAX, x2);
                        } else {
                            GETEDH(x1, 0);  // get edd changed addr, so cannot be called 2 times for same op...
                            BEQ_MARK(xRDX, xZR);
                            if(ed!=x1) {MV(x1, ed);}
                            CALL(div64, -1);
                            B_NEXT_nocond;
                            MARK;
                            DIVU(x2, xRAX, ed);
                            REMU(xRDX, xRAX, ed);
                            MV(xRAX, x2);
                            SET_DFNONE();
                        }
                    }
                    break;
                case 7:
                    INST_NAME("IDIV Ed");
                    SKIPTEST(x1);
                    SETFLAGS(X_ALL, SF_SET);
                    if(!rex.w) {
                        SET_DFNONE()
                        GETSED(0);
                        SLLI(x3, xRDX, 32);
                        AND(x2, xRAX, xMASK);
                        OR(x3, x3, x2);
                        DIV(x2, x3, ed);
                        REM(xRDX, x3, ed);
                        AND(xRAX, x2, xMASK);
                        ZEROUP(xRDX);
                    } else {
                        if(ninst && dyn->insts
                           &&  dyn->insts[ninst-1].x64.addr 
                           && *(uint8_t*)(dyn->insts[ninst-1].x64.addr)==0x48
                           && *(uint8_t*)(dyn->insts[ninst-1].x64.addr+1)==0x99) {
                            SET_DFNONE()
                            GETED(0);
                            DIV(x2, xRAX, ed);
                            REM(xRDX, xRAX, ed);
                            MV(xRAX, x2);
                        } else {
                            GETEDH(x1, 0);  // get edd changed addr, so cannot be called 2 times for same op...
                            //Need to see if RDX==0 and RAX not signed
                            // or RDX==-1 and RAX signed
                            BNE_MARK2(xRDX, xZR);
                            BGE_MARK(xRAX, xZR);
                            MARK2;
                            NOT(x2, xRDX);
                            BNE_MARK3(x2, xZR);
                            BLT_MARK(xRAX, xZR);
                            MARK3;
                            if(ed!=x1) MV(x1, ed);
                            CALL((void*)idiv64, -1);
                            B_NEXT_nocond;
                            MARK;
                            DIV(x2, xRAX, ed);
                            REM(xRDX, xRAX, ed);
                            MV(xRAX, x2);
                            SET_DFNONE()
                        }
                    }
                    break;
                default:
                    DEFAULT;
            };
            break;

        case 0xFF:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0: // INC Ed
                    INST_NAME("INC Ed");
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET_PENDING);
                    GETED(0);
                    emit_inc32(dyn, ninst, rex, ed, x3, x4, x5, x6);
                    WBACK;
                    break;
                case 1: //DEC Ed
                    INST_NAME("DEC Ed");
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET_PENDING);
                    GETED(0);
                    emit_dec32(dyn, ninst, rex, ed, x3, x4, x5, x6);
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
                        SETFLAGS(X_ALL, SF_SET);    //Hack to put flag in "don't care" state
                    }
                    GETEDx(0);
                    if(box64_dynarec_callret && box64_dynarec_bigblock>1) {
                        BARRIER(BARRIER_FULL);
                    } else {
                        BARRIER(BARRIER_FLOAT);
                        *need_epilog = 0;
                        *ok = 0;
                    }
                    GETIP_(addr);
                    // TODO: Add suport for CALLRET optim
                    /*if(box64_dynarec_callret) {
                        // Push actual return address
                        if(addr < (dyn->start+dyn->isize)) {
                            // there is a next...
                            j64 = (dyn->insts)?(dyn->insts[ninst].epilog-(dyn->native_size)):0;
                            ADR_S20(x4, j64);
                        } else {
                            j64 = getJumpTableAddress64(addr);
                            TABLE64(x4, j64);
                            LDRx_U12(x4, x4, 0);
                        }
                        STPx_S7_preindex(x4, xRIP, xSP, -16);
                    }*/
                    PUSH1(xRIP);
                    jump_to_next(dyn, 0, ed, ninst);
                    break;
                case 4: // JMP Ed
                    INST_NAME("JMP Ed");
                    READFLAGS(X_PEND);
                    BARRIER(BARRIER_FLOAT);
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
