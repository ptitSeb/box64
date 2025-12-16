#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <signal.h>
#include <assert.h>

#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
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
#include "../dynarec_helper.h"

uintptr_t dynarec64_00_0(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    uint8_t nextop, opcode;
    uint8_t gd, ed;
    int8_t i8;
    int32_t i32, tmp;
    int64_t i64, j64;
    uint8_t u8;
    uint8_t gb, gb1, gb2, eb1, eb2;
    uint32_t u32;
    uint64_t u64;
    uint8_t wback, wb1, wb2, wb;
    int64_t fixedaddress;
    int lock;
    int cacheupd = 0;
    uintptr_t retaddr = 0;

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
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            FAST_8BIT_OPERATION(wb, gb, x1, ADD(wb, wb, x1));
            GETEB(x1, 0);
            GETGB(x2);
            emit_add8(dyn, ninst, x1, x2, x4, x5, x6);
            EBBACK(x5, 0);
            break;
        case 0x01:
            INST_NAME("ADD Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_add32(dyn, ninst, rex, ed, gd, x3, x4, x5);
            WBACK;
            break;
        case 0x02:
            INST_NAME("ADD Gb, Eb");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            FAST_8BIT_OPERATION(gb, wb, x1, ADD(gb, gb, x1));
            GETEB(x1, 0);
            GETGB(x2);
            emit_add8(dyn, ninst, x2, x1, x4, x5, x6);
            GBBACK(x5);
            break;
        case 0x03:
            INST_NAME("ADD Gd, Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_add32(dyn, ninst, rex, gd, ed, x3, x4, x5);
            break;
        case 0x04:
            INST_NAME("ADD AL, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            u8 = F8;
            ANDI(x1, xRAX, 0xff);
            emit_add8c(dyn, ninst, x1, u8, x3, x4, x5, x6);
            ANDI(xRAX, xRAX, ~0xff);
            OR(xRAX, xRAX, x1);
            break;
        case 0x05:
            INST_NAME("ADD EAX, Id");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i64 = F32S;
            emit_add32c(dyn, ninst, rex, xRAX, i64, x3, x4, x5, x6);
            break;
        case 0x06:
            if (rex.is32bits) {
                INST_NAME("PUSH ES");
                LHU(x1, xEmu, offsetof(x64emu_t, segs[_ES]));
                PUSH1_32(x1);
            } else {
                INST_NAME("Illegal 06");
                if (BOX64DRENV(dynarec_safeflags) > 1) {
                    READFLAGS(X_PEND);
                } else {
                    SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION); // Hack to set flags in "don't care" state
                }
                GETIP(ip, x7);
                BARRIER(BARRIER_FLOAT);
                UDF();
                *need_epilog = 1;
                *ok = 0;
            }
            break;
        case 0x07:
            if (rex.is32bits) {
                INST_NAME("POP ES");
                POP1_32(x1);
                SH(x1, xEmu, offsetof(x64emu_t, segs[_ES]));
            } else {
                INST_NAME("Illegal 07");
                if (BOX64DRENV(dynarec_safeflags) > 1) {
                    READFLAGS(X_PEND);
                } else {
                    SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION); // Hack to set flags in "don't care" state
                }
                GETIP(ip, x7);
                BARRIER(BARRIER_FLOAT);
                UDF();
                *need_epilog = 1;
                *ok = 0;
            }
            break;
        case 0x08:
            INST_NAME("OR Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            FAST_8BIT_OPERATION(wb, gb, x1, OR(wb, wb, x1));
            GETEB(x1, 0);
            GETGB(x2);
            emit_or8(dyn, ninst, x1, x2, x4, x5);
            EBBACK(x5, 0);
            break;
        case 0x09:
            INST_NAME("OR Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_or32(dyn, ninst, rex, ed, gd, x3, x4);
            WBACK;
            break;
        case 0x0A:
            INST_NAME("OR Gb, Eb");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            FAST_8BIT_OPERATION(gb, wb, x1, OR(gb, gb, x1));
            GETEB(x1, 0);
            GETGB(x2);
            emit_or8(dyn, ninst, x2, x1, x4, x5);
            GBBACK(x5);
            break;
        case 0x0B:
            INST_NAME("OR Gd, Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_or32(dyn, ninst, rex, gd, ed, x3, x4);
            break;
        case 0x0C:
            INST_NAME("OR AL, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            u8 = F8;
            ANDI(x1, xRAX, 0xff);
            emit_or8c(dyn, ninst, x1, u8, x3, x4, x5);
            ANDI(xRAX, xRAX, ~0xff);
            OR(xRAX, xRAX, x1);
            break;
        case 0x0D:
            INST_NAME("OR EAX, Id");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i64 = F32S;
            emit_or32c(dyn, ninst, rex, xRAX, i64, x3, x4);
            break;
        case 0x0E:
            if (rex.is32bits) {
                INST_NAME("PUSH CS");
                LHU(x1, xEmu, offsetof(x64emu_t, segs[_CS]));
                PUSH1_32(x1);
                SMWRITE();
            } else {
                INST_NAME("Illegal 0E");
                if (BOX64DRENV(dynarec_safeflags) > 1) {
                    READFLAGS(X_PEND);
                } else {
                    SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION); // Hack to set flags in "don't care" state
                }
                GETIP(ip, x7);
                BARRIER(BARRIER_FLOAT);
                UDF();
                *need_epilog = 1;
                *ok = 0;
            }
            break;
        case 0x0F:
            switch (rex.rep) {
                case 0:
                    if (cpuext.vector)
                        retaddr = dynarec64_0F_vector(dyn, addr, ip, ninst, rex, ok, need_epilog);
                    addr = retaddr ? retaddr : dynarec64_0F(dyn, addr, ip, ninst, rex, ok, need_epilog);
                    break;
                case 1:
                    if (cpuext.vector)
                        retaddr = dynarec64_F20F_vector(dyn, addr, ip, ninst, rex, ok, need_epilog);
                    addr = retaddr ? retaddr : dynarec64_F20F(dyn, addr, ip, ninst, rex, ok, need_epilog);
                    break;
                case 2:
                    if (cpuext.vector)
                        retaddr = dynarec64_F30F_vector(dyn, addr, ip, ninst, rex, ok, need_epilog);
                    addr = retaddr ? retaddr : dynarec64_F30F(dyn, addr, ip, ninst, rex, ok, need_epilog);
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0x10:
            INST_NAME("ADC Eb, Gb");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            FAST_8BIT_OPERATION(wb, gb, x1, {
                ADD(wb, wb, x1);
                ANDI(x2, xFlags, 1 << F_CF);
                SLLI(x2, x2, 64 - 8);
                ADD(wb, wb, x2);
            });
            GETEB(x1, 0);
            GETGB(x2);
            emit_adc8(dyn, ninst, x1, x2, x4, x5, x6);
            EBBACK(x5, 0);
            break;
        case 0x11:
            INST_NAME("ADC Ed, Gd");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_adc32(dyn, ninst, rex, ed, gd, x3, x4, x5, x6);
            WBACK;
            break;
        case 0x12:
            INST_NAME("ADC Gb, Eb");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            FAST_8BIT_OPERATION(gb, wb, x1, {
                ADD(gb, gb, x1);
                ANDI(x2, xFlags, 1 << F_CF);
                SLLI(x2, x2, 64 - 8);
                ADD(gb, gb, x2);
            });
            GETEB(x2, 0);
            GETGB(x1);
            emit_adc8(dyn, ninst, x1, x2, x4, x6, x5);
            GBBACK(x5);
            break;
        case 0x13:
            INST_NAME("ADC Gd, Ed");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_adc32(dyn, ninst, rex, gd, ed, x3, x4, x5, x6);
            break;
        case 0x14:
            INST_NAME("ADC AL, Ib");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            u8 = F8;
            ANDI(x1, xRAX, 0xff);
            emit_adc8c(dyn, ninst, x1, u8, x3, x4, x5, x6);
            ANDI(xRAX, xRAX, ~0xff);
            OR(xRAX, xRAX, x1);
            break;
        case 0x15:
            INST_NAME("ADC EAX, Id");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i64 = F32S;
            MOV64xw(x1, i64);
            emit_adc32(dyn, ninst, rex, xRAX, x1, x3, x4, x5, x6);
            break;
        case 0x16:
            if (rex.is32bits) {
                INST_NAME("PUSH SS");
                LHU(x1, xEmu, offsetof(x64emu_t, segs[_SS]));
                PUSH1_32(x1);
                SMWRITE();
            } else {
                INST_NAME("Illegal 16");
                if (BOX64DRENV(dynarec_safeflags) > 1) {
                    READFLAGS(X_PEND);
                } else {
                    SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION); // Hack to set flags in "don't care" state
                }
                GETIP(ip, x7);
                BARRIER(BARRIER_FLOAT);
                UDF();
                *need_epilog = 1;
                *ok = 0;
            }
            break;
        case 0x17:
            if (rex.is32bits) {
                INST_NAME("POP SS");
                SMREAD();
                POP1_32(x1);
                SH(x1, xEmu, offsetof(x64emu_t, segs[_SS]));
            } else {
                INST_NAME("Illegal 17");
                if (BOX64DRENV(dynarec_safeflags) > 1) {
                    READFLAGS(X_PEND);
                } else {
                    SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION); // Hack to set flags in "don't care" state
                }
                GETIP(ip, x7);
                BARRIER(BARRIER_FLOAT);
                UDF();
                *need_epilog = 1;
                *ok = 0;
            }
            break;
        case 0x18:
            INST_NAME("SBB Eb, Gb");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            FAST_8BIT_OPERATION(wb, gb, x1, {
                SUB(wb, wb, x1);
                ANDI(x2, xFlags, 1 << F_CF);
                SLLI(x2, x2, 64 - 8);
                SUB(wb, wb, x2);
            });
            GETEB(x1, 0);
            GETGB(x2);
            emit_sbb8(dyn, ninst, x1, x2, x4, x5, x6);
            EBBACK(x5, 0);
            break;
        case 0x19:
            INST_NAME("SBB Ed, Gd");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_sbb32(dyn, ninst, rex, ed, gd, x3, x4, x5);
            WBACK;
            break;
        case 0x1A:
            INST_NAME("SBB Gb, Eb");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            FAST_8BIT_OPERATION(gb, wb, x1, {
                SUB(gb, gb, x1);
                ANDI(x2, xFlags, 1 << F_CF);
                SLLI(x2, x2, 64 - 8);
                SUB(gb, gb, x2);
            });
            GETEB(x2, 0);
            GETGB(x1);
            emit_sbb8(dyn, ninst, x1, x2, x6, x4, x5);
            GBBACK(x5);
            break;
        case 0x1B:
            INST_NAME("SBB Gd, Ed");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_sbb32(dyn, ninst, rex, gd, ed, x3, x4, x5);
            break;
        case 0x1C:
            INST_NAME("SBB AL, Ib");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            u8 = F8;
            ANDI(x1, xRAX, 0xff);
            emit_sbb8c(dyn, ninst, x1, u8, x3, x4, x5, x6);
            ANDI(xRAX, xRAX, ~0xff);
            OR(xRAX, xRAX, x1);
            break;
        case 0x1D:
            INST_NAME("SBB EAX, Id");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i64 = F32S;
            MOV64xw(x2, i64);
            emit_sbb32(dyn, ninst, rex, xRAX, x2, x3, x4, x5);
            break;
        case 0x1E:
            if (rex.is32bits) {
                INST_NAME("PUSH DS");
                LHU(x1, xEmu, offsetof(x64emu_t, segs[_DS]));
                PUSH1_32(x1);
                SMWRITE();
            } else {
                INST_NAME("Illegal 1E");
                if (BOX64DRENV(dynarec_safeflags) > 1) {
                    READFLAGS(X_PEND);
                } else {
                    SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION); // Hack to set flags in "don't care" state
                }
                GETIP(ip, x7);
                BARRIER(BARRIER_FLOAT);
                UDF();
                *need_epilog = 1;
                *ok = 0;
            }
            break;
        case 0x1F:
            if (rex.is32bits) {
                INST_NAME("POP DS");
                SMREAD();
                POP1_32(x1);
                SH(x1, xEmu, offsetof(x64emu_t, segs[_DS]));
            } else {
                INST_NAME("Illegal 1F");
                if (BOX64DRENV(dynarec_safeflags) > 1) {
                    READFLAGS(X_PEND);
                } else {
                    SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION); // Hack to set flags in "don't care" state
                }
                GETIP(ip, x7);
                BARRIER(BARRIER_FLOAT);
                UDF();
                *need_epilog = 1;
                *ok = 0;
            }
            break;
        case 0x20:
            INST_NAME("AND Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETEB(x1, 0);
            GETGB(x2);
            emit_and8(dyn, ninst, x1, x2, x4, x5);
            EBBACK(x4, 0);
            break;
        case 0x21:
            INST_NAME("AND Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_and32(dyn, ninst, rex, ed, gd, x3, x4);
            WBACK;
            break;
        case 0x22:
            INST_NAME("AND Gb, Eb");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETEB(x1, 0);
            GETGB(x2);
            emit_and8(dyn, ninst, x2, x1, x4, x5);
            GBBACK(x5);
            break;
        case 0x23:
            INST_NAME("AND Gd, Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_and32(dyn, ninst, rex, gd, ed, x3, x4);
            break;
        case 0x24:
            INST_NAME("AND AL, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            u8 = F8;
            ANDI(x1, xRAX, 0xff);
            emit_and8c(dyn, ninst, x1, u8, x3, x4);
            ANDI(xRAX, xRAX, ~0xff);
            OR(xRAX, xRAX, x1);
            break;
        case 0x25:
            INST_NAME("AND EAX, Id");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i64 = F32S;
            emit_and32c(dyn, ninst, rex, xRAX, i64, x3, x4);
            break;
        case 0x28:
            INST_NAME("SUB Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            FAST_8BIT_OPERATION(wb, gb, x1, SUB(wb, wb, x1));
            GETEB(x1, 0);
            GETGB(x2);
            emit_sub8(dyn, ninst, x1, x2, x4, x5, x6);
            EBBACK(x5, 0);
            break;
        case 0x29:
            INST_NAME("SUB Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_sub32(dyn, ninst, rex, ed, gd, x3, x4, x5);
            WBACK;
            break;
        case 0x2A:
            INST_NAME("SUB Gb, Eb");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            FAST_8BIT_OPERATION(gb, wb, x1, SUB(gb, gb, x1));
            GETEB(x1, 0);
            GETGB(x2);
            emit_sub8(dyn, ninst, x2, x1, x4, x5, x6);
            GBBACK(x5);
            break;
        case 0x2B:
            INST_NAME("SUB Gd, Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_sub32(dyn, ninst, rex, gd, ed, x3, x4, x5);
            break;
        case 0x2C:
            INST_NAME("SUB AL, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            u8 = F8;
            ANDI(x1, xRAX, 0xff);
            emit_sub8c(dyn, ninst, x1, u8, x2, x3, x4, x5);
            ANDI(xRAX, xRAX, ~0xff);
            OR(xRAX, xRAX, x1);
            break;
        case 0x2D:
            INST_NAME("SUB EAX, Id");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i64 = F32S;
            emit_sub32c(dyn, ninst, rex, xRAX, i64, x2, x3, x4, x5);
            break;
        case 0x2E:
            INST_NAME("CS:");
            break;
        case 0x30:
            INST_NAME("XOR Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            FAST_8BIT_OPERATION(wb, gb, x1, XOR(wb, wb, x1));
            GETEB(x1, 0);
            GETGB(x2);
            emit_xor8(dyn, ninst, x1, x2, x4, x5);
            EBBACK(x5, 0);
            break;
        case 0x31:
            INST_NAME("XOR Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
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
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            FAST_8BIT_OPERATION(gb, wb, x1, XOR(gb, gb, x1));
            GETEB(x1, 0);
            GETGB(x2);
            emit_xor8(dyn, ninst, x2, x1, x4, x5);
            GBBACK(x5);
            break;
        case 0x33:
            INST_NAME("XOR Gd, Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_xor32(dyn, ninst, rex, gd, ed, x3, x4);
            break;
        case 0x34:
            INST_NAME("XOR AL, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            u8 = F8;
            ANDI(x1, xRAX, 0xff);
            emit_xor8c(dyn, ninst, x1, u8, x3, x4);
            ANDI(xRAX, xRAX, ~0xff);
            OR(xRAX, xRAX, x1);
            break;
        case 0x35:
            INST_NAME("XOR EAX, Id");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i64 = F32S;
            emit_xor32c(dyn, ninst, rex, xRAX, i64, x3, x4);
            break;
        case 0x36:
            INST_NAME("SS:");
            break;
        case 0x38:
            INST_NAME("CMP Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETEB(x1, 0);
            GETGB(x2);
            emit_cmp8(dyn, ninst, x1, x2, x7, x4, x5, x6);
            break;
        case 0x39:
            INST_NAME("CMP Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_cmp32(dyn, ninst, rex, ed, gd, x3, x4, x5, x6);
            break;
        case 0x3A:
            INST_NAME("CMP Gb, Eb");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETEB(x1, 0);
            GETGB(x2);
            emit_cmp8(dyn, ninst, x2, x1, x7, x4, x5, x6);
            break;
        case 0x3B:
            INST_NAME("CMP Gd, Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED(0);
            emit_cmp32(dyn, ninst, rex, gd, ed, x3, x4, x5, x6);
            break;
        case 0x3C:
            INST_NAME("CMP AL, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
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
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i64 = F32S;
            if (i64) {
                MOV64x(x2, i64);
                emit_cmp32(dyn, ninst, rex, xRAX, x2, x3, x4, x5, x6);
            } else
                emit_cmp32_0(dyn, ninst, rex, 0xC0 /* fake nextop */, xRAX, x3, x4, x5);
            break;

        default:
            DEFAULT;
    }

    return addr;
}
