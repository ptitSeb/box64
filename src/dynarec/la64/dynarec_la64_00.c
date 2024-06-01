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
#include "la64_emitter.h"
#include "x64run.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "bridge.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"
#include "custommem.h"

#include "la64_printer.h"
#include "dynarec_la64_private.h"
#include "dynarec_la64_functions.h"
#include "dynarec_la64_helper.h"

int isSimpleWrapper(wrapper_t fun);
int isRetX87Wrapper(wrapper_t fun);

uintptr_t dynarec64_00(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
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

    switch (opcode) {
        case 0x00:
            INST_NAME("ADD Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB(x1, 0);
            GETGB(x2);
            emit_add8(dyn, ninst, x1, x2, x4, x5);
            EBBACK();
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
            GBBACK();
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
            BSTRINS_D(xRAX, x1, 7, 0);
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
            EBBACK();
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
            GBBACK();
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
            BSTRINS_D(xRAX, x1, 7, 0);
            break;
        case 0x0D:
            INST_NAME("OR EAX, Id");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i64 = F32S;
            emit_or32c(dyn, ninst, rex, xRAX, i64, x3, x4);
            break;
        case 0x0F:
            switch (rep) {
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
        case 0x11:
            INST_NAME("ADC Ed, Gd");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_adc32(dyn, ninst, rex, ed, gd, x3, x4, x5, x6);
            WBACK;
            break;
        case 0x13:
            INST_NAME("ADC Gd, Ed");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_adc32(dyn, ninst, rex, gd, ed, x3, x4, x5, x6);
            break;
        case 0x18:
            INST_NAME("SBB Eb, Gb");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB(x1, 0);
            GETGB(x2);
            emit_sbb8(dyn, ninst, x1, x2, x4, x5, x6);
            EBBACK();
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
        case 0x1B:
            INST_NAME("SBB Gd, Ed");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_sbb32(dyn, ninst, rex, gd, ed, x3, x4, x5);
            break;
        case 0x1C:
            INST_NAME("SBB AL, Ib");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            u8 = F8;
            ANDI(x1, xRAX, 0xff);
            emit_sbb8c(dyn, ninst, x1, u8, x3, x4, x5, x6);
            BSTRINS_D(xRAX, x1, 7, 0);
            break;
        case 0x20:
            INST_NAME("AND Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB(x1, 0);
            GETGB(x2);
            emit_and8(dyn, ninst, x1, x2, x4, x5);
            EBBACK();
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
            GBBACK();
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
            BSTRINS_D(xRAX, x1, 7, 0);
            break;
        case 0x25:
            INST_NAME("AND EAX, Id");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i64 = F32S;
            emit_and32c(dyn, ninst, rex, xRAX, i64, x3, x4);
            break;
        case 0x28:
            INST_NAME("SUB Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB(x1, 0);
            GETGB(x2);
            emit_sub8(dyn, ninst, x1, x2, x4, x5, x6);
            EBBACK();
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
            GBBACK();
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
            BSTRINS_D(xRAX, x1, 7, 0);
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
            EBBACK();
            break;
        case 0x31:
            INST_NAME("XOR Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_xor32(dyn, ninst, rex, ed, gd, x3, x4);
            if (ed != gd) {
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
            GBBACK();
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
            BSTRINS_D(xRAX, x1, 7, 0);
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
        case 0x3B:
            INST_NAME("CMP Gd, Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_cmp32(dyn, ninst, rex, gd, ed, x3, x4, x5, x6);
            break;
        case 0x3A:
            INST_NAME("CMP Gb, Eb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB(x1, 0);
            GETGB(x2);
            emit_cmp8(dyn, ninst, x2, x1, x3, x4, x5, x6);
            break;
        case 0x3C:
            INST_NAME("CMP AL, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            u8 = F8;
            ANDI(x1, xRAX, 0xff);
            if (u8) {
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
            gd = TO_LA64((opcode & 0x07) + (rex.b << 3));
            PUSH1z(gd);
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
            gd = TO_LA64((opcode & 0x07) + (rex.b << 3));
            POP1z(gd);
            break;
            break;
        case 0x63:
            if (rex.is32bits) {
                // this is ARPL opcode
                DEFAULT;
            } else {
                INST_NAME("MOVSXD Gd, Ed");
                nextop = F8;
                GETGD;
                if (rex.w) {
                    if (MODREG) { // reg <= reg
                        ADDI_W(gd, TO_LA64((nextop & 7) + (rex.b << 3)), 0);
                    } else { // mem <= reg
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                        LD_W(gd, ed, fixedaddress);
                    }
                } else {
                    if (MODREG) { // reg <= reg
                        AND(gd, TO_LA64((nextop & 7) + (rex.b << 3)), xMASK);
                    } else { // mem <= reg
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                        LD_WU(gd, ed, fixedaddress);
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
            if (rex.is32bits) {
                DEFAULT;
            } else
                addr = dynarec64_67(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
            break;
        case 0x68:
            INST_NAME("PUSH Id");
            i64 = F32S;
            if(PK(0)==0xC3) {
                MESSAGE(LOG_DUMP, "PUSH then RET, using indirect\n");
                TABLE64(x3, addr-4);
                LD_W(x1, x3, 0);
                PUSH1z(x1);
            } else {
                MOV64z(x3, i64);
                PUSH1z(x3);
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
            if (rex.w) {
                // 64bits imul
                UFLAG_IF {
                    MULH_D(x3, ed, x4);
                    MUL_D(gd, ed, x4);
                    UFLAG_OP1(x3);
                    UFLAG_RES(gd);
                    UFLAG_DF(x3, d_imul64);
                } else {
                    MULxw(gd, ed, x4);
                }
            } else {
                // 32bits imul
                UFLAG_IF {
                    MUL_D(gd, ed, x4);
                    UFLAG_RES(gd);
                    SRLI_D(x3, gd, 32);
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
            MOV64z(x3, i64);
            PUSH1z(x3);
            break;
        case 0x6B:
            INST_NAME("IMUL Gd, Ed, Ib");
            SETFLAGS(X_ALL, SF_PENDING);
            nextop = F8;
            GETGD;
            GETED(1);
            i64 = F8S;
            MOV64xw(x4, i64);
            if (rex.w) {
                // 64bits imul
                UFLAG_IF {
                    MULH_D(x3, ed, x4);
                    MUL_D(gd, ed, x4);
                    UFLAG_OP1(x3);
                    UFLAG_RES(gd);
                    UFLAG_DF(x3, d_imul64);
                } else {
                    MUL_D(gd, ed, x4);
                }
            } else {
                // 32bits imul
                UFLAG_IF {
                    MUL_D(gd, ed, x4);
                    UFLAG_RES(gd);
                    SRLI_D(x3, gd, 32);
                    UFLAG_OP1(x3);
                    UFLAG_DF(x3, d_imul32);
                } else {
                    MUL_W(gd, ed, x4);
                }
                ZEROUP(gd);
            }
            break;

        #define GO(GETFLAGS, NO, YES, F, I)                                                         \
            if (box64_dynarec_test == 2) { NOTEST(x1); }                                            \
            READFLAGS(F);                                                                           \
            i8 = F8S;                                                                               \
            BARRIER(BARRIER_MAYBE);                                                                 \
            JUMP(addr + i8, 1);                                                                     \
            if (la64_lbt) {                                                                         \
                X64_SETJ(x1, I);                                                                    \
            } else {                                                                                \
                GETFLAGS;                                                                           \
            }                                                                                       \
            if (dyn->insts[ninst].x64.jmp_insts == -1 || CHECK_CACHE()) {                           \
                /* out of block */                                                                  \
                i32 = dyn->insts[ninst].epilog - (dyn->native_size);                                \
                if (la64_lbt)                                                                       \
                    BEQZ_safe(x1, i32);                                                             \
                else                                                                                \
                    B##NO##_safe(x1, i32);                                                          \
                if (dyn->insts[ninst].x64.jmp_insts == -1) {                                        \
                    if (!(dyn->insts[ninst].x64.barrier & BARRIER_FLOAT))                           \
                        fpu_purgecache(dyn, ninst, 1, x1, x2, x3);                                  \
                    jump_to_next(dyn, addr + i8, 0, ninst, rex.is32bits);                           \
                } else {                                                                            \
                    CacheTransform(dyn, ninst, cacheupd, x1, x2, x3);                               \
                    i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address - (dyn->native_size); \
                    B(i32);                                                                         \
                }                                                                                   \
            } else {                                                                                \
                /* inside the block */                                                              \
                i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address - (dyn->native_size);     \
                if (la64_lbt)                                                                       \
                    BNEZ_safe(x1, i32);                                                             \
                else                                                                                \
                    B##YES##_safe(x1, i32);                                                         \
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
                    EBBACK();
                    break;
                case 1: // OR
                    INST_NAME("OR Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEB(x1, 1);
                    u8 = F8;
                    emit_or8c(dyn, ninst, x1, u8, x2, x4, x5);
                    EBBACK();
                    break;
                case 4: // AND
                    INST_NAME("AND Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEB(x1, 1);
                    u8 = F8;
                    emit_and8c(dyn, ninst, x1, u8, x2, x4);
                    EBBACK();
                    break;
                case 5: // SUB
                    INST_NAME("SUB Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEB(x1, 1);
                    u8 = F8;
                    emit_sub8c(dyn, ninst, x1, u8, x2, x4, x5, x6);
                    EBBACK();
                    break;
                case 6: // XOR
                    INST_NAME("XOR Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEB(x1, 1);
                    u8 = F8;
                    emit_xor8c(dyn, ninst, x1, u8, x2, x4);
                    EBBACK();
                    break;
                case 7: // CMP
                    INST_NAME("CMP Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEB(x1, 1);
                    u8 = F8;
                    if(u8) {
                        ADDI_D(x2, xZR, u8);
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
            switch ((nextop >> 3) & 7) {
                case 0: // ADD
                    if (opcode == 0x81) {
                        INST_NAME("ADD Ed, Id");
                    } else {
                        INST_NAME("ADD Ed, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETED((opcode == 0x81) ? 4 : 1);
                    if (opcode == 0x81) i64 = F32S; else i64 = F8S;
                    emit_add32c(dyn, ninst, rex, ed, i64, x3, x4, x5, x6);
                    WBACK;
                    break;
                case 1: // OR
                    if (opcode == 0x81) {
                        INST_NAME("OR Ed, Id");
                    } else {
                        INST_NAME("OR Ed, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETED((opcode == 0x81) ? 4 : 1);
                    if (opcode == 0x81) i64 = F32S; else i64 = F8S;
                    emit_or32c(dyn, ninst, rex, ed, i64, x3, x4);
                    WBACK;
                    break;
                case 2: // ADC
                    if (opcode == 0x81) {
                        INST_NAME("ADC Ed, Id");
                    } else {
                        INST_NAME("ADC Ed, Ib");
                    }
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETED((opcode == 0x81) ? 4 : 1);
                    if (opcode == 0x81)
                        i64 = F32S;
                    else
                        i64 = F8S;
                    MOV64xw(x5, i64);
                    emit_adc32(dyn, ninst, rex, ed, x5, x3, x4, x6, x1);
                    WBACK;
                    break;
                case 4: // AND
                    if (opcode == 0x81) {
                        INST_NAME("AND Ed, Id");
                    } else {
                        INST_NAME("AND Ed, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETED((opcode == 0x81) ? 4 : 1);
                    if (opcode == 0x81) i64 = F32S; else i64 = F8S;
                    emit_and32c(dyn, ninst, rex, ed, i64, x3, x4);
                    WBACK;
                    break;
                case 3: // SBB
                    if (opcode == 0x81) {
                        INST_NAME("SBB Ed, Id");
                    } else {
                        INST_NAME("SBB Ed, Ib");
                    }
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETED((opcode == 0x81) ? 4 : 1);
                    if (opcode == 0x81)
                        i64 = F32S;
                    else
                        i64 = F8S;
                    MOV64xw(x5, i64);
                    emit_sbb32(dyn, ninst, rex, ed, x5, x3, x4, x6);
                    WBACK;
                    break;
                case 5: // SUB
                    if (opcode == 0x81) {
                        INST_NAME("SUB Ed, Id");
                    } else {
                        INST_NAME("SUB Ed, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETED((opcode == 0x81) ? 4 : 1);
                    if (opcode == 0x81)
                        i64 = F32S;
                    else
                        i64 = F8S;
                    emit_sub32c(dyn, ninst, rex, ed, i64, x3, x4, x5, x6);
                    WBACK;
                    break;
                case 6: // XOR
                    if (opcode == 0x81) {
                        INST_NAME("XOR Ed, Id");
                    } else {
                        INST_NAME("XOR Ed, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETED((opcode == 0x81) ? 4 : 1);
                    if (opcode == 0x81)
                        i64 = F32S;
                    else
                        i64 = F8S;
                    emit_xor32c(dyn, ninst, rex, ed, i64, x3, x4);
                    WBACK;
                    break;
                case 7: // CMP
                    if (opcode == 0x81) {
                        INST_NAME("CMP Ed, Id");
                    } else {
                        INST_NAME("CMP Ed, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETED((opcode == 0x81) ? 4 : 1);
                    if (opcode == 0x81) i64 = F32S; else i64 = F8S;
                    if (i64) {
                        MOV64xw(x2, i64);
                        emit_cmp32(dyn, ninst, rex, ed, x2, x3, x4, x5, x6);
                    } else {
                        if (!rex.w && MODREG) {
                            AND(x1, ed, xMASK);
                            ed = x1;
                        }
                        emit_cmp32_0(dyn, ninst, rex, ed, x3, x4);
                    }
                    break;
                default:
                    DEFAULT;
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
            nextop = F8;
            GETGD;
            GETED(0);
            emit_test32(dyn, ninst, rex, ed, gd, x3, x4, x5);
            break;
        case 0x86:
            INST_NAME("(LOCK) XCHG Eb, Gb");
            nextop = F8;
            if (MODREG) {
                GETGB(x1);
                GETEB(x2, 0);
                BSTRINS_D(wback, gd, wb2 + 7, wb2);
                BSTRINS_D(gb1, ed, gb2 + 7, gb2);
            } else {
                GETGB(x3);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                if (la64_lam_bh)
                    AMSWAP_DB_B(x1, gd, ed);
                else {
                    SMDMB();

                    // calculate shift amount
                    ANDI(x1, ed, 0x3);
                    SLLI_D(x1, x1, 3);

                    // align address to 4-bytes to use ll.w/sc.w
                    ADDI_D(x4, xZR, 0xffc);
                    AND(x6, ed, x4);

                    // load aligned data
                    LD_WU(x5, x6, 0);

                    // insert gd byte into the aligned data
                    ADDI_D(x4, xZR, 0xff);
                    SLL_D(x4, x4, x1);
                    NOR(x4, x4, xZR);
                    AND(x4, x5, x4);
                    SLL_D(x5, gd, x1);
                    OR(x4, x4, x5);

                    // do aligned ll/sc sequence
                    MARKLOCK;
                    LL_W(x1, x6, 0);
                    MV(x5, x4);
                    SC_W(x5, x6, 0);
                    BEQZ_MARKLOCK(x5);

                    // calculate shift amount again
                    ANDI(x4, ed, 0x3);
                    SLLI_D(x4, x4, 3);

                    // extract loaded byte
                    SRL_D(x1, x1, x4);
                }
                BSTRINS_D(gb1, x1, gb2 + 7, gb2);
            }
            break;
        case 0x87:
            INST_NAME("(LOCK) XCHG Ed, Gd");
            nextop = F8;
            if (MODREG) {
                GETGD;
                GETED(0);
                MVxw(x1, gd);
                MVxw(gd, ed);
                MVxw(ed, x1);
            } else {
                GETGD;
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                SMDMB();
                ANDI(x3, ed, (1 << (2 + rex.w)) - 1);
                BNEZ_MARK(x3);
                MARKLOCK;
                LLxw(x1, ed, 0);
                MVxw(x3, gd);
                SCxw(x3, ed, 0);
                BEQZ_MARKLOCK(x3);
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
            gd = ((nextop & 0x38) >> 3) + (rex.r << 3);
            if (rex.rex) {
                gb2 = 0;
                gb1 = TO_LA64(gd);
            } else {
                gb2 = ((gd & 4) << 1);
                gb1 = TO_LA64(gd & 3);
            }
            if (gb2) {
                gd = x4;
                BSTRPICK_D(gd, gb1, gb2 + 7, gb2);
            } else {
                gd = gb1; // no need to extract
            }
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                if (rex.rex) {
                    eb1 = TO_LA64(ed);
                    eb2 = 0;
                } else {
                    eb1 = TO_LA64(ed & 3); // Ax, Cx, Dx or Bx
                    eb2 = ((ed & 4) >> 2); // L or H
                }
                BSTRINS_D(eb1, gd, eb2 * 8 + 7, eb2 * 8);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, 1, 0);
                ST_B(gd, ed, fixedaddress);
                SMWRITELOCK(lock);
            }
            break;
        case 0x89:
            INST_NAME("MOV Ed, Gd");
            nextop = F8;
            GETGD;
            if (MODREG) { // reg <= reg
                MVxw(TO_LA64((nextop & 7) + (rex.b << 3)), gd);
            } else { // mem <= reg
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, 1, 0);
                if (rex.w) {
                    ST_D(gd, ed, fixedaddress);
                } else {
                    ST_W(gd, ed, fixedaddress);
                }
                SMWRITELOCK(lock);
            }
            break;
        case 0x8A:
            INST_NAME("MOV Gb, Eb");
            nextop = F8;
            if (rex.rex) {
                gb1 = gd = TO_LA64(((nextop & 0x38) >> 3) + (rex.r << 3));
                gb2 = 0;
            } else {
                gd = (nextop & 0x38) >> 3;
                gb1 = TO_LA64(gd & 3);
                gb2 = ((gd & 4) << 1);
            }
            if (MODREG) {
                if (rex.rex) {
                    wback = TO_LA64((nextop & 7) + (rex.b << 3));
                    wb2 = 0;
                } else {
                    wback = (nextop & 7);
                    wb2 = (wback >> 2);
                    wback = TO_LA64(wback & 3);
                }
                if (wb2) {
                    BSTRPICK_D(x4, wback, 7 + wb2 * 8, wb2 * 8);
                    ed = x4;
                } else {
                    ed = wback;
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, &lock, 1, 0);
                SMREADLOCK(lock);
                LD_BU(x4, wback, fixedaddress);
                ed = x4;
            }
            BSTRINS_D(gb1, ed, gb2 + 7, gb2);
            break;
        case 0x8B:
            INST_NAME("MOV Gd, Ed");
            nextop = F8;
            GETGD;
            if (MODREG) {
                MVxw(gd, TO_LA64((nextop & 7) + (rex.b << 3)));
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, 1, 0);
                SMREADLOCK(lock);
                LDxw(gd, ed, fixedaddress);
            }
            break;
        case 0x8D:
            INST_NAME("LEA Gd, Ed");
            nextop = F8;
            GETGD;
            if (MODREG) { // reg <= reg? that's an invalid operation
                DEFAULT;
            } else { // mem <= reg
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 0, 0);
                MV(gd, ed);
                if (!rex.w || rex.is32bits) {
                    ZEROUP(gd); // truncate the higher 32bits as asked
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
            gd = TO_LA64((opcode & 0x07) + (rex.b << 3));
            if (gd == xRAX) {
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
            if (rex.w) {
                SEXT_W(xRAX, xRAX);
            } else {
                EXT_W_H(xRAX, xRAX);
                ZEROUP(xRAX);
            }
            break;
        case 0x99:
            INST_NAME("CDQ");
            if (rex.w) {
                SRAI_D(xRDX, xRAX, 63);
            } else {
                SRAI_W(xRDX, xRAX, 31);
                BSTRPICK_D(xRDX, xRDX, 31, 0);
            }
            break;
        case 0xA0:
            INST_NAME("MOV AL,Ob");
            if(rex.is32bits) u64 = F32; else u64 = F64;
            MOV64z(x1, u64);
            LD_BU(x2, x1, 0);
            BSTRINS_D(xRAX, x2, 7, 0);
            break;
        case 0xA4:
            if (rep) {
                INST_NAME("REP MOVSB");
                CBZ_NEXT(xRCX);
                ANDI(x1, xFlags, 1 << F_DF);
                BNEZ_MARK2(x1);
                MARK; // Part with DF==0
                LD_BU(x1, xRSI, 0);
                ST_B(x1, xRDI, 0);
                ADDI_D(xRSI, xRSI, 1);
                ADDI_D(xRDI, xRDI, 1);
                ADDI_D(xRCX, xRCX, -1);
                BNEZ_MARK(xRCX);
                B_NEXT_nocond;
                MARK2; // Part with DF==1
                LD_BU(x1, xRSI, 0);
                ST_B(x1, xRDI, 0);
                ADDI_D(xRSI, xRSI, -1);
                ADDI_D(xRDI, xRDI, -1);
                ADDI_D(xRCX, xRCX, -1);
                BNEZ_MARK2(xRCX);
                // done
            } else {
                INST_NAME("MOVSB");
                GETDIR(x3, x1, 1);
                LD_BU(x1, xRSI, 0);
                ST_B(x1, xRDI, 0);
                ADD_D(xRSI, xRSI, x3);
                ADD_D(xRDI, xRDI, x3);
            }
            break;
        case 0xA5:
            if (rep) {
                INST_NAME("REP MOVSD");
                CBZ_NEXT(xRCX);
                ANDI(x1, xFlags, 1 << F_DF);
                BNEZ_MARK2(x1);
                MARK; // Part with DF==0
                LDxw(x1, xRSI, 0);
                SDxw(x1, xRDI, 0);
                ADDI_D(xRSI, xRSI, rex.w ? 8 : 4);
                ADDI_D(xRDI, xRDI, rex.w ? 8 : 4);
                ADDI_D(xRCX, xRCX, -1);
                BNEZ_MARK(xRCX);
                B_NEXT_nocond;
                MARK2; // Part with DF==1
                LDxw(x1, xRSI, 0);
                SDxw(x1, xRDI, 0);
                ADDI_D(xRSI, xRSI, rex.w ? -8 : -4);
                ADDI_D(xRDI, xRDI, rex.w ? -8 : -4);
                ADDI_D(xRCX, xRCX, -1);
                BNEZ_MARK2(xRCX);
                // done
            } else {
                INST_NAME("MOVSD");
                GETDIR(x3, x1, rex.w ? 8 : 4);
                LDxw(x1, xRSI, 0);
                SDxw(x1, xRDI, 0);
                ADD_D(xRSI, xRSI, x3);
                ADD_D(xRDI, xRDI, x3);
            }
            break;
        case 0xA6:
            switch (rep) {
                case 1:
                case 2:
                    if (rep == 1) {
                        INST_NAME("REPNZ CMPSB");
                    } else {
                        INST_NAME("REPZ CMPSB");
                    }
                    MAYSETFLAGS();
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    CBZ_NEXT(xRCX);
                    ANDI(x1, xFlags, 1 << F_DF);
                    BNEZ_MARK2(x1);
                    MARK; // Part with DF==0
                    LD_BU(x1, xRSI, 0);
                    LD_BU(x2, xRDI, 0);
                    ADDI_D(xRSI, xRSI, 1);
                    ADDI_D(xRDI, xRDI, 1);
                    ADDI_D(xRCX, xRCX, -1);
                    if (rep == 1) {
                        BEQ_MARK3(x1, x2);
                    } else {
                        BNE_MARK3(x1, x2);
                    }
                    BNEZ_MARK(xRCX);
                    B_MARK3_nocond;
                    MARK2; // Part with DF==1
                    LD_BU(x1, xRSI, 0);
                    LD_BU(x2, xRDI, 0);
                    ADDI_D(xRSI, xRSI, -1);
                    ADDI_D(xRDI, xRDI, -1);
                    ADDI_D(xRCX, xRCX, -1);
                    if (rep == 1) {
                        BEQ_MARK3(x1, x2);
                    } else {
                        BNE_MARK3(x1, x2);
                    }
                    BNEZ_MARK2(xRCX);
                    MARK3; // end
                    emit_cmp8(dyn, ninst, x1, x2, x3, x4, x5, x6);
                    break;
                default:
                    INST_NAME("CMPSB");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETDIR(x3, x1, 1);
                    LD_BU(x1, xRSI, 0);
                    LD_BU(x2, xRDI, 0);
                    ADD_D(xRSI, xRSI, x3);
                    ADD_D(xRDI, xRDI, x3);
                    emit_cmp8(dyn, ninst, x1, x2, x3, x4, x5, x6);
                    break;
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
        case 0xAA:
            if (rep) {
                INST_NAME("REP STOSB");
                CBZ_NEXT(xRCX);
                ANDI(x1, xFlags, 1 << F_DF);
                BNEZ_MARK2(x1);
                MARK; // Part with DF==0
                ST_B(xRAX, xRDI, 0);
                ADDI_D(xRDI, xRDI, 1);
                ADDI_D(xRCX, xRCX, -1);
                BNEZ_MARK(xRCX);
                B_NEXT_nocond;
                MARK2; // Part with DF==1
                ST_B(xRAX, xRDI, 0);
                ADDI_D(xRDI, xRDI, -1);
                ADDI_D(xRCX, xRCX, -1);
                BNEZ_MARK2(xRCX);
                // done
            } else {
                INST_NAME("STOSB");
                GETDIR(x3, x1, 1);
                ST_B(xRAX, xRDI, 0);
                ADD_D(xRDI, xRDI, x3);
            }
            break;
        case 0xAB:
            if (rep) {
                INST_NAME("REP STOSD");
                CBZ_NEXT(xRCX);
                ANDI(x1, xFlags, 1 << F_DF);
                BNEZ_MARK2(x1);
                MARK; // Part with DF==0
                SDxw(xRAX, xRDI, 0);
                ADDI_D(xRDI, xRDI, rex.w ? 8 : 4);
                ADDI_D(xRCX, xRCX, -1);
                BNEZ_MARK(xRCX);
                B_NEXT_nocond;
                MARK2; // Part with DF==1
                SDxw(xRAX, xRDI, 0);
                ADDI_D(xRDI, xRDI, rex.w ? -8 : -4);
                ADDI_D(xRCX, xRCX, -1);
                BNEZ_MARK2(xRCX);
                // done
            } else {
                INST_NAME("STOSD");
                GETDIR(x3, x1, rex.w ? 8 : 4);
                SDxw(xRAX, xRDI, 0);
                ADD_D(xRDI, xRDI, x3);
            }
            break;
        case 0xAE:
            switch (rep) {
                case 1:
                case 2:
                    if (rep == 1) {
                        INST_NAME("REPNZ SCASB");
                    } else {
                        INST_NAME("REPZ SCASB");
                    }
                    MAYSETFLAGS();
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    CBZ_NEXT(xRCX);
                    ANDI(x1, xRAX, 0xff);
                    ANDI(x2, xFlags, 1 << F_DF);
                    BNEZ_MARK2(x2);
                    MARK; // Part with DF==0
                    LD_BU(x2, xRDI, 0);
                    ADDI_D(xRDI, xRDI, 1);
                    ADDI_D(xRCX, xRCX, -1);
                    if (rep == 1) {
                        BEQ_MARK3(x1, x2);
                    } else {
                        BNE_MARK3(x1, x2);
                    }
                    BNE_MARK(xRCX, xZR);
                    B_MARK3_nocond;
                    MARK2; // Part with DF==1
                    LD_BU(x2, xRDI, 0);
                    ADDI_D(xRDI, xRDI, -1);
                    ADDI_D(xRCX, xRCX, -1);
                    if (rep == 1) {
                        BEQ_MARK3(x1, x2);
                    } else {
                        BNE_MARK3(x1, x2);
                    }
                    BNE_MARK2(xRCX, xZR);
                    MARK3; // end
                    emit_cmp8(dyn, ninst, x1, x2, x3, x4, x5, x6);
                    break;
                default:
                    INST_NAME("SCASB");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETDIR(x3, x1, 1);
                    ANDI(x1, xRAX, 0xff);
                    LD_BU(x2, xRDI, 0);
                    ADD_D(xRDI, xRDI, x3);
                    emit_cmp8(dyn, ninst, x1, x2, x3, x4, x5, x6);
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
            if (rex.rex)
                gb1 = TO_LA64((opcode & 7) + (rex.b << 3));
            else
                gb1 = TO_LA64(opcode & 3);
            BSTRINS_D(gb1, x1, 7, 0);
            break;
        case 0xB4:
        case 0xB5:
        case 0xB6:
        case 0xB7:
            INST_NAME("MOV xH, Ib");
            u8 = F8;
            MOV32w(x1, u8);
            if (rex.rex) {
                gb1 = TO_LA64((opcode & 7) + (rex.b << 3));
                BSTRINS_D(gb1, x1, 7, 0);
            } else {
                gb1 = TO_LA64(opcode & 3);
                BSTRINS_D(gb1, x1, 15, 8);
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
            gd = TO_LA64((opcode & 7) + (rex.b << 3));
            if (rex.w) {
                u64 = F64;
                MOV64x(gd, u64);
            } else {
                u32 = F32;
                MOV32w(gd, u32);
            }
            break;
        case 0xC0:
            nextop = F8;
            // TODO: refine these...
            switch ((nextop >> 3) & 7) {
                case 4:
                case 6:
                    INST_NAME("SHL Eb, Ib");
                    GETEB(x1, 1);
                    u8 = (F8) & 0x1f;
                    if (u8) {
                        SETFLAGS(X_ALL, SF_PENDING);
                        UFLAG_IF {
                            MOV32w(x4, u8);
                            UFLAG_OP2(x4);
                        };
                        UFLAG_OP1(ed);
                        SLLI_W(ed, ed, u8);
                        EBBACK();
                        UFLAG_RES(ed);
                        UFLAG_DF(x3, d_shl8);
                    } else {
                        NOP();
                    }
                    break;
                case 5:
                    INST_NAME("SHR Eb, Ib");
                    GETEB(x1, 1);
                    u8 = (F8) & 0x1f;
                    if (u8) {
                        SETFLAGS(X_ALL, SF_PENDING);
                        UFLAG_IF {
                            MOV32w(x4, u8);
                            UFLAG_OP2(x4);
                        };
                        UFLAG_OP1(ed);
                        if (u8) {
                            SRLI_W(ed, ed, u8);
                            EBBACK();
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
                    u8 = (F8) & 0x1f;
                    if (u8) {
                        SETFLAGS(X_ALL, SF_PENDING);
                        UFLAG_IF {
                            MOV32w(x4, u8);
                            UFLAG_OP2(x4);
                        };
                        UFLAG_OP1(ed);
                        if (u8) {
                            SRAI_W(ed, ed, u8);
                            EBBACK();
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
            switch ((nextop >> 3) & 7) {
                case 0:
                    INST_NAME("ROL Ed, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop) & (rex.w ? 0x3f : 0x1f);
                    // flags are not affected if count is 0, we make it a nop if possible.
                    if (u8) {
                        SETFLAGS(X_OF | X_CF, SF_SUBSET_PENDING);
                        GETED(1);
                        F8;
                        emit_rol32c(dyn, ninst, rex, ed, u8, x3, x4);
                        WBACK;
                    } else {
                        if (MODREG && !rex.w) {
                            GETED(1);
                            ZEROUP(ed);
                        } else {
                            FAKEED;
                        }
                        F8;
                    }
                    break;
                case 1:
                    INST_NAME("ROR Ed, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop) & (rex.w ? 0x3f : 0x1f);
                    // flags are not affected if count is 0, we make it a nop if possible.
                    if (u8) {
                        SETFLAGS(X_OF | X_CF, SF_SUBSET_PENDING);
                        GETED(1);
                        F8;
                        emit_ror32c(dyn, ninst, rex, ed, u8, x3, x4);
                        WBACK;
                    } else {
                        if (MODREG && !rex.w) {
                            GETED(1);
                            ZEROUP(ed);
                        } else {
                            FAKEED;
                        }
                        F8;
                    }
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ed, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop) & (rex.w ? 0x3f : 0x1f);
                    // flags are not affected if count is 0, we make it a nop if possible.
                    if (u8) {
                        SETFLAGS(X_ALL, SF_SET_PENDING); // some flags are left undefined
                        GETED(1);
                        u8 = (F8) & (rex.w ? 0x3f : 0x1f);
                        emit_shl32c(dyn, ninst, rex, ed, u8, x3, x4, x5);
                        WBACK;
                    } else {
                        if (MODREG && !rex.w && !rex.is32bits) {
                            GETED(1);
                            ZEROUP(ed);
                        } else {
                            FAKEED;
                        }
                        F8;
                    }
                    break;
                case 5:
                    INST_NAME("SHR Ed, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop) & (rex.w ? 0x3f : 0x1f);
                    if (u8) {
                        SETFLAGS(X_ALL, SF_SET_PENDING); // some flags are left undefined
                        GETED(1);
                        u8 = (F8) & (rex.w ? 0x3f : 0x1f);
                        emit_shr32c(dyn, ninst, rex, ed, u8, x3, x4);
                        WBACK;
                    } else {
                        if (MODREG && !rex.w && !rex.is32bits) {
                            GETED(1);
                            ZEROUP(ed);
                        } else {
                            FAKEED;
                        }
                        F8;
                    }
                    break;
                case 7:
                    INST_NAME("SAR Ed, Ib");
                    // flags are not affected if count is 0, we make it a nop if possible.
                    u8 = geted_ib(dyn, addr, ninst, nextop) & (rex.w ? 0x3f : 0x1f);
                    if (u8) {
                        SETFLAGS(X_ALL, SF_SET_PENDING); // some flags are left undefined
                        GETED(1);
                        u8 = (F8) & (rex.w ? 0x3f : 0x1f);
                        emit_sar32c(dyn, ninst, rex, ed, u8, x3, x4);
                        WBACK;
                    } else {
                        if (MODREG && !rex.w && !rex.is32bits) {
                            GETED(1);
                            ZEROUP(ed);
                        } else {
                            FAKEED;
                        }
                        F8;
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xC2:
            INST_NAME("RETN");
            // SETFLAGS(X_ALL, SF_SET);    // Hack, set all flags (to an unknown state...)
            if (box64_dynarec_safeflags) {
                READFLAGS(X_PEND); // lets play safe here too
            }
            fpu_purgecache(dyn, ninst, 1, x1, x2, x3);  // using next, even if there no next
            i32 = F16;
            retn_to_epilog(dyn, ninst, rex, i32);
            *need_epilog = 0;
            *ok = 0;
            break;
        case 0xC3:
            INST_NAME("RET");
            // SETFLAGS(X_ALL, SF_SET);    // Hack, set all flags (to an unknown state...)
            if (box64_dynarec_safeflags) {
                READFLAGS(X_PEND); // so instead, force the deferred flags, so it's not too slow, and flags are not lost
            }
            fpu_purgecache(dyn, ninst, 1, x1, x2, x3);  // using next, even if there no next
            ret_to_epilog(dyn, ninst, rex);
            *need_epilog = 0;
            *ok = 0;
            break;
        case 0xC6:
            INST_NAME("MOV Eb, Ib");
            nextop = F8;
            if (MODREG) { // reg <= u8
                u8 = F8;
                if (!rex.rex) {
                    ed = (nextop & 7);
                    eb1 = TO_LA64((ed & 3)); // Ax, Cx, Dx or Bx
                    eb2 = (ed & 4) >> 2;   // L or H
                } else {
                    eb1 = TO_LA64((nextop & 7) + (rex.b << 3));
                    eb2 = 0;
                }
                MOV32w(x3, u8);
                BSTRINS_D(eb1, x3, eb2 * 8 + 7, eb2 * 8);
            } else { // mem <= u8
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, &lock, 1, 1);
                u8 = F8;
                if (u8) {
                    ADDI_D(x3, xZR, u8);
                    ed = x3;
                } else
                    ed = xZR;
                ST_B(ed, wback, fixedaddress);
                SMWRITELOCK(lock);
            }
            break;
        case 0xC7:
            INST_NAME("MOV Ed, Id");
            nextop = F8;
            if (MODREG) { // reg <= i32
                i64 = F32S;
                ed = TO_LA64((nextop & 7) + (rex.b << 3));
                MOV64xw(ed, i64);
            } else { // mem <= i32
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, &lock, 0, 4);
                i64 = F32S;
                if (i64) {
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
            MVz(xRSP, xRBP);
            POP1z(xRBP);
            break;
        case 0xCC:
            SETFLAGS(X_ALL, SF_SET);
            SKIPTEST(x1);
            if (PK(0) == 'S' && PK(1) == 'C') {
                addr += 2;
                BARRIER(BARRIER_FLOAT);
                INST_NAME("Special Box64 instruction");
                if (PK64(0) == 0) {
                    addr += 8;
                    MESSAGE(LOG_DEBUG, "Exit x64 Emu\n");
                    MOV64x(x1, 1);
                    ST_W(x1, xEmu, offsetof(x64emu_t, quit));
                    *ok = 0;
                    *need_epilog = 1;
                } else {
                    MESSAGE(LOG_DUMP, "Native Call to %s\n", GetNativeName(GetNativeFnc(ip)));
                    x87_forget(dyn, ninst, x3, x4, 0);
                    sse_purge07cache(dyn, ninst, x3);

                    // FIXME: Even the basic support of isSimpleWrapper is disabled for now.

                    GETIP(ip + 1); // read the 0xCC
                    STORE_XEMU_CALL();
                    ADDI_D(x1, xEmu, (uint32_t)offsetof(x64emu_t, ip)); // setup addr as &emu->ip
                    CALL_S(x64Int3, -1);
                    LOAD_XEMU_CALL();
                    addr += 8 + 8;
                    TABLE64(x3, addr); // expected return address
                    BNE_MARK(xRIP, x3);
                    LD_W(w1, xEmu, offsetof(x64emu_t, quit));
                    CBZ_NEXT(w1);
                    MARK;
                    jump_to_epilog_fast(dyn, 0, xRIP, ninst);
                }
            } else {
                if (!box64_ignoreint3) {
                    INST_NAME("INT 3");
                    // check if TRAP signal is handled
                    LD_D(x1, xEmu, offsetof(x64emu_t, context));
                    MOV64x(x2, offsetof(box64context_t, signals[SIGTRAP]));
                    ADD_D(x2, x2, x1);
                    LD_D(x3, x2, 0);
                    CBZ_NEXT(x3);
                    GETIP(ip);
                    STORE_XEMU_CALL();
                    CALL(native_int3, -1);
                    LOAD_XEMU_CALL();
                }
            }
            break;
        case 0xD0:
        case 0xD2: // TODO: Jump if CL is 0
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 5:
                    if (opcode == 0xD0) {
                        INST_NAME("SHR Eb, 1");
                        MOV32w(x2, 1);
                    } else {
                        INST_NAME("SHR Eb, CL");
                        ANDI(x2, xRCX, 0x1F);
                        BEQ_NEXT(x2, xZR);
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING); // some flags are left undefined
                    if (box64_dynarec_safeflags > 1)
                        MAYSETFLAGS();
                    GETEB(x1, 0);
                    emit_shr8(dyn, ninst, x1, x2, x5, x4, x6);
                    EBBACK();
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xD1:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                    INST_NAME("ROL Ed, 1");
                    SETFLAGS(X_OF | X_CF, SF_SUBSET_PENDING);
                    GETED(0);
                    emit_rol32c(dyn, ninst, rex, ed, 1, x3, x4);
                    WBACK;
                    if (!wback && !rex.w) ZEROUP(ed);
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ed, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING); // some flags are left undefined
                    GETED(0);
                    emit_shl32c(dyn, ninst, rex, ed, 1, x3, x4, x5);
                    WBACK;
                    break;
                case 5:
                    INST_NAME("SHR Ed, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING); // some flags are left undefined
                    GETED(0);
                    emit_shr32c(dyn, ninst, rex, ed, 1, x3, x4);
                    WBACK;
                    break;
                case 7:
                    INST_NAME("SAR Ed, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING); // some flags are left undefined
                    GETED(0);
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
                case 0:
                    INST_NAME("ROL Ed, CL");
                    SETFLAGS(X_OF | X_CF, SF_SUBSET);
                    GETED(0);
                    emit_rol32(dyn, ninst, rex, ed, xRCX, x3, x4);
                    WBACK;
                    if (!wback && !rex.w) ZEROUP(ed);
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ed, CL");
                    SETFLAGS(X_ALL, SF_SET_PENDING); // some flags are left undefined
                    ANDI(x3, xRCX, rex.w ? 0x3f : 0x1f);
                    GETED(0);
                    if (!rex.w && MODREG) { ZEROUP(ed); }
                    CBZ_NEXT(x3);
                    emit_shl32(dyn, ninst, rex, ed, x3, x5, x4, x6);
                    WBACK;
                    break;
                case 5:
                    INST_NAME("SHR Ed, CL");
                    SETFLAGS(X_ALL, SF_SET_PENDING); // some flags are left undefined
                    ANDI(x3, xRCX, rex.w ? 0x3f : 0x1f);
                    GETED(0);
                    if (!rex.w && MODREG) { ZEROUP(ed); }
                    CBZ_NEXT(x3);
                    emit_shr32(dyn, ninst, rex, ed, x3, x5, x4);
                    WBACK;
                    break;
                case 7:
                    INST_NAME("SAR Ed, CL");
                    SETFLAGS(X_ALL, SF_PENDING);
                    ANDI(x3, xRCX, rex.w ? 0x3f : 0x1f);
                    GETED(0);
                    if (!rex.w && MODREG) { ZEROUP(ed); }
                    CBZ_NEXT(x3);
                    UFLAG_OP12(ed, x3);
                    SRAxw(ed, ed, x3);
                    WBACK;
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, rex.w ? d_sar64 : d_sar32);
                    break;
                default:
                    DEFAULT;
            }
            break;
        #define GO(Z)                                                                               \
            BARRIER(BARRIER_MAYBE);                                                                 \
            JUMP(addr + i8, 1);                                                                     \
            if (dyn->insts[ninst].x64.jmp_insts == -1 || CHECK_CACHE()) {                           \
                /* out of the block */                                                              \
                i32 = dyn->insts[ninst].epilog - (dyn->native_size);                                \
                if (Z) {                                                                            \
                    BNE(xRCX, xZR, i32);                                                            \
                } else {                                                                            \
                    BEQ(xRCX, xZR, i32);                                                            \
                }                                                                                   \
                if (dyn->insts[ninst].x64.jmp_insts == -1) {                                        \
                    if (!(dyn->insts[ninst].x64.barrier & BARRIER_FLOAT))                           \
                        fpu_purgecache(dyn, ninst, 1, x1, x2, x3);                                  \
                    jump_to_next(dyn, addr + i8, 0, ninst, rex.is32bits);                           \
                } else {                                                                            \
                    CacheTransform(dyn, ninst, cacheupd, x1, x2, x3);                               \
                    i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address - (dyn->native_size); \
                    B(i32);                                                                         \
                }                                                                                   \
            } else {                                                                                \
                /* inside the block */                                                              \
                i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address - (dyn->native_size);     \
                if (Z) {                                                                            \
                    BEQ(xRCX, xZR, i32);                                                            \
                } else {                                                                            \
                    BNE(xRCX, xZR, i32);                                                            \
                };                                                                                  \
            }
        case 0xE0:
            INST_NAME("LOOPNZ");
            READFLAGS(X_ZF);
            i8 = F8S;
            ADDI_D(xRCX, xRCX, -1);
            if (la64_lbt) X64_GET_EFLAGS(x1, X_ZF); else ANDI(x1, xFlags, 1 << F_ZF);
            CBNZ_NEXT(x1);
            GO(0);
            break;
        case 0xE1:
            INST_NAME("LOOPZ");
            READFLAGS(X_ZF);
            i8 = F8S;
            ADDI_D(xRCX, xRCX, -1);
            if (la64_lbt) X64_GET_EFLAGS(x1, X_ZF); else ANDI(x1, xFlags, 1 << F_ZF);
            CBZ_NEXT(x1);
            GO(0);
            break;
        case 0xE2:
            INST_NAME("LOOP");
            i8 = F8S;
            ADDI_D(xRCX, xRCX, -1);
            GO(0);
            break;
        case 0xE3:
            INST_NAME("JECXZ");
            i8 = F8S;
            GO(1);
            break;
        #undef GO
        case 0xE8:
            INST_NAME("CALL Id");
            i32 = F32S;
            if (addr + i32 == 0) {
                #if STEP == 3
                printf_log(LOG_INFO, "Warning, CALL to 0x0 at %p (%p)\n", (void*)addr, (void*)(addr - 1));
                #endif
            }
            #if STEP < 2
            if (!rex.is32bits && isNativeCall(dyn, addr + i32, &dyn->insts[ninst].natcall, &dyn->insts[ninst].retn))
                tmp = dyn->insts[ninst].pass2choice = 3;
            else
                tmp = dyn->insts[ninst].pass2choice = 0;
            #else
            tmp = dyn->insts[ninst].pass2choice;
            #endif
            switch (tmp) {
                case 3:
                    SETFLAGS(X_ALL, SF_SET); // Hack to set flags to "dont'care" state
                    SKIPTEST(x1);
                    BARRIER(BARRIER_FULL);
                    // BARRIER_NEXT(BARRIER_FULL);
                    if (dyn->last_ip && (addr - dyn->last_ip < 0x1000)) {
                        ADDI_D(x2, xRIP, addr - dyn->last_ip);
                    } else {
                        TABLE64(x2, addr);
                    }
                    PUSH1(x2);
                    MESSAGE(LOG_DUMP, "Native Call to %s (retn=%d)\n", GetNativeName(GetNativeFnc(dyn->insts[ninst].natcall - 1)), dyn->insts[ninst].retn);
                    // calling a native function
                    sse_purge07cache(dyn, ninst, x3);
                    if ((box64_log < 2 && !cycle_log) && dyn->insts[ninst].natcall) {
                        // FIXME: Add basic support for isSimpleWrapper
                        tmp = 0; // isSimpleWrapper(*(wrapper_t*)(dyn->insts[ninst].natcall + 2));
                    } else
                        tmp = 0;
                    if (tmp < 0 || tmp > 1)
                        tmp = 0; // TODO: removed when FP is in place
                    // FIXME: if (dyn->insts[ninst].natcall && isRetX87Wrapper(*(wrapper_t*)(dyn->insts[ninst].natcall + 2)))
                    //     // return value will be on the stack, so the stack depth needs to be updated
                    //     x87_purgecache(dyn, ninst, 0, x3, x1, x4);
                    if ((box64_log < 2 && !cycle_log) && dyn->insts[ninst].natcall && tmp) {
                        // GETIP(ip+3+8+8); // read the 0xCC
                        // FIXME: call_n(dyn, ninst, *(void**)(dyn->insts[ninst].natcall + 2 + 8), tmp);
                        POP1(xRIP); // pop the return address
                        dyn->last_ip = addr;
                    } else {
                        GETIP_(dyn->insts[ninst].natcall); // read the 0xCC already
                        STORE_XEMU_CALL();
                        ADDI_D(x1, xEmu, (uint32_t)offsetof(x64emu_t, ip)); // setup addr as &emu->ip
                        CALL_S(x64Int3, -1);
                        LOAD_XEMU_CALL();
                        TABLE64(x3, dyn->insts[ninst].natcall);
                        ADDI_D(x3, x3, 2 + 8 + 8);
                        BNE_MARK(xRIP, x3); // Not the expected address, exit dynarec block
                        POP1(xRIP);         // pop the return address
                        if (dyn->insts[ninst].retn) {
                            if (dyn->insts[ninst].retn < 0x1000) {
                                ADDI_D(xRSP, xRSP, dyn->insts[ninst].retn);
                            } else {
                                MOV64x(x3, dyn->insts[ninst].retn);
                                ADD_D(xRSP, xRSP, x3);
                            }
                        }
                        TABLE64(x3, addr);
                        BNE_MARK(xRIP, x3); // Not the expected address again
                        LD_W(w1, xEmu, offsetof(x64emu_t, quit));
                        CBZ_NEXT(w1);
                        MARK;
                        jump_to_epilog_fast(dyn, 0, xRIP, ninst);
                        dyn->last_ip = addr;
                    }
                    break;
                default:
                    if ((box64_dynarec_safeflags > 1) || (ninst && dyn->insts[ninst - 1].x64.set_flags)) {
                        READFLAGS(X_PEND); // that's suspicious
                    } else {
                        SETFLAGS(X_ALL, SF_SET); // Hack to set flags to "dont'care" state
                    }
                    // regular call
                    /*if (box64_dynarec_callret && box64_dynarec_bigblock > 1) {
                        BARRIER(BARRIER_FULL);
                    } else {
                        BARRIER(BARRIER_FLOAT);
                        *need_epilog = 0;
                        *ok = 0;
                    }*/

                    if (rex.is32bits) {
                        MOV32w(x2, addr);
                    } else {
                        TABLE64(x2, addr);
                    }
                    fpu_purgecache(dyn, ninst, 1, x1, x3, x4);
                    PUSH1z(x2);
                    if (box64_dynarec_callret) {
                        SET_HASCALLRET();
                        // Push actual return address
                        if (addr < (dyn->start + dyn->isize)) {
                            // there is a next...
                            j64 = (dyn->insts) ? (dyn->insts[ninst].epilog - (dyn->native_size)) : 0;
                            PCADDU12I(x4, ((j64 + 0x800) >> 12) & 0xfffff);
                            ADDI_D(x4, x4, j64 & 0xfff);
                            MESSAGE(LOG_NONE, "\tCALLRET set return to +%di\n", j64 >> 2);
                        } else {
                            MESSAGE(LOG_NONE, "\tCALLRET set return to Jmptable(%p)\n", (void*)addr);
                            j64 = getJumpTableAddress64(addr);
                            TABLE64(x4, j64);
                            LD_D(x4, x4, 0);
                        }
                        ADDI_D(xSP, xSP, -16);
                        ST_D(x4, xSP, 0);
                        ST_D(x2, xSP, 8);
                    } else {
                        *ok = 0;
                        *need_epilog = 0;
                    }
                    jump_to_next(dyn, addr + i32, 0, ninst, rex.is32bits);
                    break;
            }
            break;
        case 0xE9:
        case 0xEB:
            BARRIER(BARRIER_MAYBE);
            if (box64_dynarec_test == 2) { NOTEST(x1); }
            if (opcode == 0xE9) {
                INST_NAME("JMP Id");
                i32 = F32S;
            } else {
                INST_NAME("JMP Ib");
                i32 = F8S;
            }
            JUMP((uintptr_t)getAlternate((void*)(addr + i32)), 0);
            if (dyn->insts[ninst].x64.jmp_insts == -1) {
                // out of the block
                fpu_purgecache(dyn, ninst, 1, x1, x2, x3);
                jump_to_next(dyn, (uintptr_t)getAlternate((void*)(addr + i32)), 0, ninst, rex.is32bits);
            } else {
                // inside the block
                CacheTransform(dyn, ninst, CHECK_CACHE(), x1, x2, x3);
                tmp = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address - (dyn->native_size);
                MESSAGE(1, "Jump to %d / 0x%x\n", tmp, tmp);
                if (tmp == 4) {
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
            switch ((nextop >> 3) & 7) {
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
                    NOR(x1, x1, xZR);
                    EBBACK();
                    break;
                case 3:
                    INST_NAME("NEG Eb");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEB(x1, 0);
                    emit_neg8(dyn, ninst, x1, x2, x4);
                    EBBACK();
                    break;
                case 4:
                    INST_NAME("MUL AL, Ed");
                    SETFLAGS(X_ALL, SF_PENDING);
                    UFLAG_DF(x1, d_mul8);
                    GETEB(x1, 0);
                    ANDI(x2, xRAX, 0xff);
                    MUL_W(x1, x2, x1);
                    UFLAG_RES(x1);
                    BSTRINS_D(xRAX, x1, 15, 0);
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
                    INST_NAME("TEST Ed, Id");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETED(4);
                    i64 = F32S;
                    emit_test32c(dyn, ninst, rex, ed, i64, x3, x4, x5);
                    break;
                case 2:
                    INST_NAME("NOT Ed");
                    GETED(0);
                    NOR(ed, ed, xZR);
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
                    UFLAG_DF(x2, rex.w ? d_mul64 : d_mul32);
                    GETED(0);
                    if (rex.w) {
                        if (ed == xRDX)
                            gd = x3;
                        else
                            gd = xRDX;
                        MULH_DU(gd, xRAX, ed);
                        MUL_D(xRAX, xRAX, ed);
                        if (gd != xRDX) { MV(xRDX, gd); }
                    } else {
                        AND(x3, xRAX, xMASK);
                        if (MODREG) {
                            AND(x4, ed, xMASK);
                            ed = x4;
                        }
                        MUL_D(xRDX, x3, ed); // 64 <- 32x32
                        AND(xRAX, xRDX, xMASK);
                        SRLI_D(xRDX, xRDX, 32);
                    }
                    UFLAG_RES(xRAX);
                    UFLAG_OP1(xRDX);
                    break;
                case 5:
                    INST_NAME("IMUL EAX, Ed");
                    SETFLAGS(X_ALL, SF_PENDING);
                    UFLAG_DF(x2, rex.w ? d_imul64 : d_imul32);
                    GETSED(0);
                    if (rex.w) {
                        if (ed == xRDX)
                            gd = x3;
                        else
                            gd = xRDX;
                        MULH_D(gd, xRAX, ed);
                        MUL_D(xRAX, xRAX, ed);
                        if (gd != xRDX) { MV(xRDX, gd); }
                    } else {
                        ADDI_W(x3, xRAX, 0); // sign extend 32bits-> 64bits
                        MUL_D(xRDX, x3, ed); // 64 <- 32x32
                        AND(xRAX, xRDX, xMASK);
                        SRLI_D(xRDX, xRDX, 32);
                    }
                    UFLAG_RES(xRAX);
                    UFLAG_OP1(xRDX);
                    break;
                case 6:
                    INST_NAME("DIV Ed");
                    SETFLAGS(X_ALL, SF_SET);
                    // TODO: handle zero divisor
                    if (!rex.w) {
                        SET_DFNONE();
                        GETED(0);
                        SLLI_D(x3, xRDX, 32);
                        AND(x2, xRAX, xMASK);
                        OR(x3, x3, x2);
                        if (MODREG) {
                            AND(x4, ed, xMASK);
                            ed = x4;
                        }
                        DIV_DU(x2, x3, ed);
                        MOD_DU(xRDX, x3, ed);
                        AND(xRAX, x2, xMASK);
                        ZEROUP(xRDX);
                    } else {
                        if (ninst
                            && dyn->insts[ninst - 1].x64.addr
                            && *(uint8_t*)(dyn->insts[ninst - 1].x64.addr) == 0x31
                            && *(uint8_t*)(dyn->insts[ninst - 1].x64.addr + 1) == 0xD2) {
                            SET_DFNONE();
                            GETED(0);
                            DIV_DU(x2, xRAX, ed);
                            MOD_DU(xRDX, xRAX, ed);
                            MV(xRAX, x2);
                        } else {
                            GETEDH(x1, 0); // get edd changed addr, so cannot be called 2 times for same op...
                            BEQ_MARK(xRDX, xZR);
                            if (ed != x1) { MV(x1, ed); }
                            CALL(div64, -1);
                            B_NEXT_nocond;
                            MARK;
                            DIV_DU(x2, xRAX, ed);
                            MOD_DU(xRDX, xRAX, ed);
                            MV(xRAX, x2);
                            SET_DFNONE();
                        }
                    }
                    break;
                case 7:
                    INST_NAME("IDIV Ed");
                    SKIPTEST(x1);
                    SETFLAGS(X_ALL, SF_SET);
                    // TODO: handle zero divisor
                    if (!rex.w) {
                        SET_DFNONE()
                        GETSED(0);
                        SLLI_D(x3, xRDX, 32);
                        AND(x2, xRAX, xMASK);
                        OR(x3, x3, x2);
                        DIV_D(x2, x3, ed);
                        MOD_D(xRDX, x3, ed);
                        AND(xRAX, x2, xMASK);
                        ZEROUP(xRDX);
                    } else {
                        if (ninst && dyn->insts
                            && dyn->insts[ninst - 1].x64.addr
                            && *(uint8_t*)(dyn->insts[ninst - 1].x64.addr) == 0x48
                            && *(uint8_t*)(dyn->insts[ninst - 1].x64.addr + 1) == 0x99) {
                            SET_DFNONE()
                            GETED(0);
                            DIV_D(x2, xRAX, ed);
                            MOD_D(xRDX, xRAX, ed);
                            MV(xRAX, x2);
                        } else {
                            GETEDH(x1, 0); // get edd changed addr, so cannot be called 2 times for same op...
                            // need to see if RDX == 0 and RAX not signed
                            // or RDX == -1 and RAX signed
                            BNE_MARK2(xRDX, xZR);
                            BGE_MARK(xRAX, xZR);
                            MARK2;
                            NOR(x2, xZR, xRDX);
                            BNE_MARK3(x2, xZR);
                            BLT_MARK(xRAX, xZR);
                            MARK3;
                            if (ed != x1) MV(x1, ed);
                            CALL((void*)idiv64, -1);
                            B_NEXT_nocond;
                            MARK;
                            DIV_D(x2, xRAX, ed);
                            MOD_D(xRDX, xRAX, ed);
                            MV(xRAX, x2);
                            SET_DFNONE()
                        }
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xFC:
            INST_NAME("CLD");
            BSTRINS_D(xFlags, xZR, F_DF, F_DF);
            break;
        case 0xFD:
            INST_NAME("STD");
            ORI(xFlags, xFlags, 1 << F_DF);
            break;
        case 0xFE:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                    INST_NAME("INC Eb");
                    SETFLAGS(X_ALL & ~X_CF, SF_SUBSET_PENDING);
                    GETEB(x1, 0);
                    emit_inc8(dyn, ninst, ed, x2, x4, x5);
                    EBBACK();
                    break;
                case 1:
                    INST_NAME("DEC Eb");
                    SETFLAGS(X_ALL & ~X_CF, SF_SUBSET_PENDING);
                    GETEB(x1, 0);
                    emit_dec8(dyn, ninst, ed, x2, x4, x5);
                    EBBACK();
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xFF:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0: // INC Ed
                    INST_NAME("INC Ed");
                    SETFLAGS(X_ALL & ~X_CF, SF_SUBSET_PENDING);
                    GETED(0);
                    emit_inc32(dyn, ninst, rex, ed, x3, x4, x5, x6);
                    WBACK;
                    break;
                case 1: // DEC Ed
                    INST_NAME("DEC Ed");
                    SETFLAGS(X_ALL & ~X_CF, SF_SUBSET_PENDING);
                    GETED(0);
                    emit_dec32(dyn, ninst, rex, ed, x3, x4, x5, x6);
                    WBACK;
                    break;
                case 2:
                    INST_NAME("CALL Ed");
                    PASS2IF ((box64_dynarec_safeflags > 1) || ((ninst && dyn->insts[ninst - 1].x64.set_flags) || ((ninst > 1) && dyn->insts[ninst - 2].x64.set_flags)), 1) {
                        READFLAGS(X_PEND); // that's suspicious
                    } else {
                        SETFLAGS(X_ALL, SF_SET); // Hack to put flag in "don't care" state
                    }
                    GETEDz(0);
                    if (box64_dynarec_callret && box64_dynarec_bigblock > 1) {
                        BARRIER(BARRIER_FULL);
                    } else {
                        BARRIER(BARRIER_FLOAT);
                        *need_epilog = 0;
                        *ok = 0;
                    }
                    GETIP_(addr);
                    if (box64_dynarec_callret) {
                        SET_HASCALLRET();
                        // Push actual return address
                        if (addr < (dyn->start + dyn->isize)) {
                            // there is a next
                            j64 = (dyn->insts) ? (dyn->insts[ninst].epilog - (dyn->native_size)) : 0;
                            PCADDU12I(x4, ((j64 + 0x800) >> 12) & 0xfffff);
                            ADDI_D(x4, x4, j64 & 0xfff);
                            MESSAGE(LOG_NONE, "\tCALLRET set return to +%di\n", j64 >> 2);
                        } else {
                            MESSAGE(LOG_NONE, "\tCALLRET set return to Jmptable(%p)\n", (void*)addr);
                            j64 = getJumpTableAddress64(addr);
                            TABLE64(x4, j64);
                            LD_D(x4, x4, 0);
                        }
                        ADDI_D(xSP, xSP, -16);
                        ST_D(x4, xSP, 0);
                        ST_D(xRIP, xSP, 8);
                    }
                    PUSH1z(xRIP);
                    jump_to_next(dyn, 0, ed, ninst, rex.is32bits);
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
                case 6: // Push Ed
                    INST_NAME("PUSH Ed");
                    GETEDz(0);
                    PUSH1z(ed);
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
