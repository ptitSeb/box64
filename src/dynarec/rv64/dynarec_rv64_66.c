#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "rv64_emitter.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"
#include "custommem.h"

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "../dynarec_helper.h"
#include "dynarec_rv64_functions.h"


uintptr_t dynarec64_66(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
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
    uintptr_t retaddr = 0;
    MAYUSE(u8);
    MAYUSE(u16);
    MAYUSE(u64);
    MAYUSE(j64);
    MAYUSE(lock);

    if (rex.w && !(opcode == 0x0f))                                          // rex.w cancels "66", but not for 66 0f type of prefix
        return dynarec64_00(dyn, addr - 1, ip, ninst, rex, ok, need_epilog); // addr-1, to "put back" opcode

    switch (opcode) {
        case 0x01:
            INST_NAME("ADD Ew, Gw");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            FAST_16BIT_OPERATION(ed, gd, x1, ADD(ed, ed, x1));
            GETGW(x2);
            GETEW(x1, 0);
            emit_add16(dyn, ninst, x1, x2, x4, x5, x6);
            EWBACK;
            break;
        case 0x03:
            INST_NAME("ADD Gw, Ew");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            FAST_16BIT_OPERATION(gd, ed, x1, ADD(gd, gd, x1));
            GETGW(x1);
            GETEW(x2, 0);
            emit_add16(dyn, ninst, x1, x2, x5, x4, x6);
            GWBACK;
            break;
        case 0x05:
            INST_NAME("ADD AX, Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i32 = F16;
            ZEXTH(x1, xRAX);
            MOV32w(x2, i32);
            emit_add16(dyn, ninst, x1, x2, x3, x4, x6);
            INSHz(xRAX, x1, x3, x4, 1, 0);
            break;
        case 0x06:
            INST_NAME("PUSH ES");
            LHU(x1, xEmu, offsetof(x64emu_t, segs[_ES]));
            PUSH1_16(x1);
            break;
        case 0x07:
            INST_NAME("POP ES");
            POP1_16(x1);
            SH(x1, xEmu, offsetof(x64emu_t, segs[_ES]));
            break;
        case 0x09:
            INST_NAME("OR Ew, Gw");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            FAST_16BIT_OPERATION(ed, gd, x1, OR(ed, ed, x1));
            GETGW(x2);
            GETEW(x1, 0);
            emit_or16(dyn, ninst, x1, x2, x4, x5);
            EWBACK;
            break;
        case 0x0B:
            INST_NAME("OR Gw, Ew");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            FAST_16BIT_OPERATION(gd, ed, x1, OR(gd, gd, x1));
            GETGW(x1);
            GETEW(x2, 0);
            emit_or16(dyn, ninst, x1, x2, x4, x5);
            GWBACK;
            break;
        case 0x0D:
            INST_NAME("OR AX, Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i32 = F16;
            ZEXTH(x1, xRAX);
            MOV32w(x2, i32);
            emit_or16(dyn, ninst, x1, x2, x3, x4);
            INSHz(xRAX, x1, x3, x4, 1, 0);
            break;
        case 0x0F:
            switch (rex.rep) {
                case 1: addr = dynarec64_66F20F(dyn, addr, ip, ninst, rex, ok, need_epilog); break;
                case 2: addr = dynarec64_66F30F(dyn, addr, ip, ninst, rex, ok, need_epilog); break;
                default: {
                    if (cpuext.vector)
                        retaddr = dynarec64_660F_vector(dyn, addr, ip, ninst, rex, ok, need_epilog);
                    addr = retaddr ? retaddr : dynarec64_660F(dyn, addr, ip, ninst, rex, ok, need_epilog);
                    break;
                }
            }
            break;
        case 0x11:
            INST_NAME("ADC Ew, Gw");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            FAST_16BIT_OPERATION(ed, gd, x1, {
                ADD(ed, ed, x1);
                ANDI(x2, xFlags, 1 << F_CF);
                SLLI(x2, x2, 64 - 16);
                ADD(ed, ed, x2);
            });
            GETGW(x2);
            GETEW(x1, 0);
            emit_adc16(dyn, ninst, x1, x2, x4, x6, x5);
            EWBACK;
            break;
        case 0x13:
            INST_NAME("ADC Gw, Ew");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            FAST_16BIT_OPERATION(gd, ed, x1, {
                ADD(gd, gd, x1);
                ANDI(x2, xFlags, 1 << F_CF);
                SLLI(x2, x2, 64 - 16);
                ADD(gd, gd, x2);
            });
            GETGW(x1);
            GETEW(x2, 0);
            emit_adc16(dyn, ninst, x1, x2, x4, x6, x5);
            GWBACK;
            break;
        case 0x15:
            INST_NAME("ADC AX, Iw");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            u64 = F16;
            ZEXTH(x1, xRAX);
            MOV64x(x2, u64);
            emit_adc16(dyn, ninst, x1, x2, x3, x4, x5);
            INSHz(xRAX, x1, x3, x4, 1, 0);
            break;
        case 0x19:
            INST_NAME("SBB Ew, Gw");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            FAST_16BIT_OPERATION(ed, gd, x1, {
                SUB(ed, ed, x1);
                ANDI(x2, xFlags, 1 << F_CF);
                SLLI(x2, x2, 64 - 16);
                SUB(ed, ed, x2);
            });
            GETGW(x2);
            GETEW(x1, 0);
            emit_sbb16(dyn, ninst, x1, x2, x4, x5, x6);
            EWBACK;
            break;
        case 0x1B:
            INST_NAME("SBB Gw, Ew");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            FAST_16BIT_OPERATION(gd, ed, x1, {
                SUB(gd, gd, x1);
                ANDI(x2, xFlags, 1 << F_CF);
                SLLI(x2, x2, 64 - 16);
                SUB(gd, gd, x2);
            });
            GETGW(x1);
            GETEW(x2, 0);
            emit_sbb16(dyn, ninst, x1, x2, x6, x4, x5);
            GWBACK;
            break;
        case 0x1D:
            INST_NAME("SBB AX, Iw");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            ZEXTH(x1, xRAX);
            u64 = F16;
            MOV64x(x2, u64);
            emit_sbb16(dyn, ninst, x1, x2, x3, x4, x5);
            INSHz(xRAX, x1, x3, x4, 1, 0);
            break;
        case 0x1E:
            INST_NAME("PUSH DS");
            LHU(x1, xEmu, offsetof(x64emu_t, segs[_DS]));
            PUSH1_16(x1);
            break;
        case 0x1F:
            INST_NAME("POP DS");
            POP1_16(x1);
            SH(x1, xEmu, offsetof(x64emu_t, segs[_DS]));
            break;
        case 0x21:
            INST_NAME("AND Ew, Gw");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGW(x2);
            GETEW(x1, 0);
            emit_and16(dyn, ninst, x1, x2, x4, x5);
            EWBACK;
            break;
        case 0x23:
            INST_NAME("AND Gw, Ew");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGW(x1);
            GETEW(x2, 0);
            emit_and16(dyn, ninst, x1, x2, x4, x5);
            GWBACK;
            break;
        case 0x25:
            INST_NAME("AND AX, Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i32 = F16;
            ZEXTH(x1, xRAX);
            MOV32w(x2, i32);
            emit_and16(dyn, ninst, x1, x2, x3, x4);
            INSHz(xRAX, x1, x3, x4, 1, 0);
            break;
        case 0x29:
            INST_NAME("SUB Ew, Gw");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            FAST_16BIT_OPERATION(ed, gd, x1, SUB(ed, ed, x1));
            GETGW(x1);
            GETEW(x2, 0);
            emit_sub16(dyn, ninst, x2, x1, x4, x5, x6);
            EWBACK;
            break;
        case 0x2B:
            INST_NAME("SUB Gw, Ew");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            FAST_16BIT_OPERATION(gd, ed, x1, SUB(gd, gd, x1));
            GETGW(x1);
            GETEW(x2, 0);
            emit_sub16(dyn, ninst, x1, x2, x6, x4, x5);
            GWBACK;
            break;
        case 0x2D:
            INST_NAME("SUB AX, Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i32 = F16;
            ZEXTH(x1, xRAX);
            MOV32w(x2, i32);
            emit_sub16(dyn, ninst, x1, x2, x3, x4, x5);
            INSHz(xRAX, x1, x3, x4, 1, 0);
            break;
        case 0x31:
            INST_NAME("XOR Ew, Gw");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            if (MODREG && !dyn->insts[ninst].x64.gen_flags) {
                gd = TO_NAT(((nextop & 0x38) >> 3) + (rex.r << 3));
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                if (ed == gd) {
                    SRLI(ed, ed, 16);
                    SLLI(ed, ed, 16);
                    break;
                }
            }
            FAST_16BIT_OPERATION(ed, gd, x1, XOR(ed, ed, x1));
            GETGW(x2);
            GETEW(x1, 0);
            emit_xor16(dyn, ninst, x1, x2, x4, x5, x6);
            EWBACK;
            break;
        case 0x33:
            INST_NAME("XOR Gw, Ew");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            if (MODREG && !dyn->insts[ninst].x64.gen_flags) {
                gd = TO_NAT(((nextop & 0x38) >> 3) + (rex.r << 3));
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                if (ed == gd) {
                    SRLI(gd, gd, 16);
                    SLLI(gd, gd, 16);
                    break;
                }
            }
            FAST_16BIT_OPERATION(gd, ed, x1, XOR(gd, gd, x1));
            GETGW(x1);
            GETEW(x2, 0);
            emit_xor16(dyn, ninst, x1, x2, x4, x5, x6);
            GWBACK;
            break;
        case 0x35:
            INST_NAME("XOR AX, Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i32 = F16;
            ZEXTH(x1, xRAX);
            MOV32w(x2, i32);
            emit_xor16(dyn, ninst, x1, x2, x3, x4, x5);
            INSHz(xRAX, x1, x3, x4, 1, 0);
            break;
        case 0x39:
            INST_NAME("CMP Ew, Gw");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGW(x2);
            GETEW(x1, 0);
            emit_cmp16(dyn, ninst, x1, x2, x7, x4, x5, x6);
            break;
        case 0x3B:
            INST_NAME("CMP Gw, Ew");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGW(x1);
            GETEW(x2, 0);
            emit_cmp16(dyn, ninst, x1, x2, x7, x4, x5, x6);
            break;
        case 0x3D:
            INST_NAME("CMP AX, Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            i32 = F16;
            ZEXTH(x1, xRAX);
            if (i32) {
                MOV32w(x2, i32);
                emit_cmp16(dyn, ninst, x1, x2, x3, x4, x5, x6);
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
            SETFLAGS(X_ALL & ~X_CF, SF_SUBSET_PENDING, NAT_FLAGS_FUSION);
            gd = TO_NAT(opcode & 7);
            ZEXTH(x1, gd);
            emit_inc16(dyn, ninst, x1, x2, x3, x4, x5);
            INSHz(gd, x1, x3, x4, 1, 0);
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
            SETFLAGS(X_ALL & ~X_CF, SF_SUBSET_PENDING, NAT_FLAGS_FUSION);
            gd = TO_NAT(opcode & 7);
            ZEXTH(x1, gd);
            emit_dec16(dyn, ninst, x1, x2, x3, x4, x5);
            INSHz(gd, x1, x3, x4, 1, 0);
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
            SCRATCH_USAGE(0);
            gd = TO_NAT((opcode & 0x07) + (rex.b << 3));
            PUSH1_16(gd);
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
            INSHz(gd, x1, x2, x3, 1, 0);
            break;
        case 0x60:
            if (rex.is32bits) {
                INST_NAME("PUSHA 16bits (32bits)");
                MV(x1, xRSP);
                PUSH1_16(xRAX);
                PUSH1_16(xRCX);
                PUSH1_16(xRDX);
                PUSH1_16(xRBX);
                PUSH1_16(x1);
                PUSH1_16(xRBP);
                PUSH1_16(xRSI);
                PUSH1_16(xRDI);
            } else
                return dynarec64_00(dyn, addr - 1, ip, ninst, rex, ok, need_epilog);
            break;
        case 0x61:
            if (rex.is32bits) {
                INST_NAME("POPA 16bits (32bits)");
                POP1_16(x1);
                INSH(xRDI, x1, x5, x6, 1, 0);
                POP1_16(x1);
                INSH(xRSI, x1, x5, x6, 0, 0);
                POP1_16(x1);
                INSH(xRBP, x1, x5, x6, 0, 0);
                POP1_16(x1); // RSP ignored
                POP1_16(x1);
                INSH(xRBX, x1, x5, x6, 0, 0);
                POP1_16(x1);
                INSH(xRDX, x1, x5, x6, 0, 0);
                POP1_16(x1);
                INSH(xRCX, x1, x5, x6, 0, 0);
                POP1_16(x1);
                INSH(xRAX, x1, x5, x6, 0, 0);
            } else
                return dynarec64_00(dyn, addr - 1, ip, ninst, rex, ok, need_epilog);
            break;
        case 0x68:
            INST_NAME("PUSH Iw");
            u16 = F16;
            MOV32w(x2, u16);
            PUSH1_16(x2);
            break;
        case 0x69:
        case 0x6B:
            if (opcode == 0x69) {
                INST_NAME("IMUL Gw, Ew, Iw");
            } else {
                INST_NAME("IMUL Gw, Ew, Ib");
            }
            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            nextop = F8;
            GETSEW(x1, (opcode == 0x69) ? 2 : 1);
            if (opcode == 0x69)
                i32 = F16S;
            else
                i32 = F8S;
            MOV32w(x2, i32);
            MULW(x5, x2, x1);
            ZEXTH(x2, x5);
            SET_DFNONE();
            CLEAR_FLAGS();
            IFX (X_CF | X_OF) {
                SRAIW(x6, x5, 15);
                SRAIW(x7, x5, 31);
                XOR(x6, x6, x7);
                IFX (X_CF) SET_FLAGS_NEZ(x6, F_CF, x7);
                IFX (X_OF) SET_FLAGS_NEZ(x6, F_OF2, x7);
            }
            IFX (X_SF) {
                SRLI(x6, x5, 15 - F_SF);
                ANDI(x6, x6, 1 << F_SF);
                OR(xFlags, xFlags, x6);
            }
            IFX (X_PF) emit_pf(dyn, ninst, x5, x6, x7);
            gd = x2;
            GWBACK;
            break;
        case 0x6A:
            INST_NAME("PUSH Ib");
            i16 = F8S;
            MOV32w(x2, (uint16_t)i16);
            PUSH1_16(x2);
            break;
        case 0x6C:
        case 0x6E:
            return dynarec64_00(dyn, addr - 1, ip, ninst, rex, ok, need_epilog);
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
        case 0x7A:
        case 0x7B:
        case 0x7C:
        case 0x7D:
        case 0x7E:
        case 0x7F:
            // just use regular conditional jump
            return dynarec64_00(dyn, addr - 1, ip, ninst, rex, ok, need_epilog);

        case 0x81:
        case 0x83:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0: // ADD
                    if (opcode == 0x81) {
                        INST_NAME("ADD Ew, Iw");
                    } else {
                        INST_NAME("ADD Ew, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEW(x1, (opcode == 0x81) ? 2 : 1);
                    if (opcode == 0x81)
                        u64 = F16;
                    else
                        u64 = (uint16_t)(int16_t)F8S;
                    MOV64x(x5, u64);
                    emit_add16(dyn, ninst, ed, x5, x2, x4, x6);
                    EWBACK;
                    break;
                case 1: // OR
                    if (opcode == 0x81) {
                        INST_NAME("OR Ew, Iw");
                    } else {
                        INST_NAME("OR Ew, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEW(x1, (opcode == 0x81) ? 2 : 1);
                    if (opcode == 0x81)
                        u64 = F16;
                    else
                        u64 = (uint16_t)(int16_t)F8S;
                    MOV64x(x5, u64);
                    emit_or16(dyn, ninst, x1, x5, x2, x4);
                    EWBACK;
                    break;
                case 2: // ADC
                    if (opcode == 0x81) {
                        INST_NAME("ADC Ew, Iw");
                    } else {
                        INST_NAME("ADC Ew, Ib");
                    }
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEW(x1, (opcode == 0x81) ? 2 : 1);
                    if (opcode == 0x81)
                        u64 = F16;
                    else
                        u64 = (uint16_t)(int16_t)F8S;
                    MOV64x(x5, u64);
                    emit_adc16(dyn, ninst, x1, x5, x2, x4, x6);
                    EWBACK;
                    break;
                case 3: // SBB
                    if (opcode == 0x81) {
                        INST_NAME("SBB Ew, Iw");
                    } else {
                        INST_NAME("SBB Ew, Ib");
                    }
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEW(x1, (opcode == 0x81) ? 2 : 1);
                    if (opcode == 0x81)
                        u64 = F16;
                    else
                        u64 = (uint16_t)(int16_t)F8S;
                    MOV64x(x5, u64);
                    emit_sbb16(dyn, ninst, x1, x5, x2, x4, x6);
                    EWBACK;
                    break;
                case 4: // AND
                    if (opcode == 0x81) {
                        INST_NAME("AND Ew, Iw");
                    } else {
                        INST_NAME("AND Ew, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEW(x1, (opcode == 0x81) ? 2 : 1);
                    if (opcode == 0x81)
                        u64 = F16;
                    else
                        u64 = (uint16_t)(int16_t)F8S;
                    MOV64x(x5, u64);
                    emit_and16(dyn, ninst, x1, x5, x2, x4);
                    EWBACK;
                    break;
                case 5: // SUB
                    if (opcode == 0x81) {
                        INST_NAME("SUB Ew, Iw");
                    } else {
                        INST_NAME("SUB Ew, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEW(x1, (opcode == 0x81) ? 2 : 1);
                    if (opcode == 0x81)
                        u64 = F16;
                    else
                        u64 = (uint16_t)(int16_t)F8S;
                    MOV64x(x5, u64);
                    emit_sub16(dyn, ninst, x1, x5, x2, x4, x6);
                    EWBACK;
                    break;
                case 6: // XOR
                    if (opcode == 0x81) {
                        INST_NAME("XOR Ew, Iw");
                    } else {
                        INST_NAME("XOR Ew, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEW(x1, (opcode == 0x81) ? 2 : 1);
                    if (opcode == 0x81)
                        u64 = F16;
                    else
                        u64 = (uint16_t)(int16_t)F8S;
                    MOV64x(x5, u64);
                    emit_xor16(dyn, ninst, x1, x5, x2, x4, x6);
                    EWBACK;
                    break;
                case 7: // CMP
                    if (opcode == 0x81) {
                        INST_NAME("CMP Ew, Iw");
                    } else {
                        INST_NAME("CMP Ew, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEW(x1, (opcode == 0x81) ? 2 : 1);
                    if (opcode == 0x81)
                        u64 = F16;
                    else
                        u64 = (uint16_t)(int16_t)F8S;
                    if (u64) {
                        MOV64x(x2, u64);
                        emit_cmp16(dyn, ninst, x1, x2, x7, x4, x5, x6);
                    } else
                        emit_cmp16_0(dyn, ninst, x1, x7, x4);
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0x85:
            INST_NAME("TEST Ew, Gw");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETEW(x1, 0);
            GETGW(x2);
            emit_test16(dyn, ninst, x1, x2, x6, x4, x5);
            break;
        case 0x87:
            INST_NAME("(LOCK) XCHG Ew, Gw");
            nextop = F8;
            if (MODREG) {
                GETGD;
                GETED(0);
                MV(x5, gd);
                INSHz(gd, ed, x3, x4, 1, 1);
                INSHz(ed, x5, x3, x4, 0, 1);
            } else {
                GETGD;
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);

                ANDI(x3, ed, 1);
                BNEZ_MARK(x3);

                ANDI(x3, ed, 0b10);
                LUI(x5, 0xffff0);
                ZEXTH(x6, gd);
                BNEZ_MARK3(x3);

                MARKLOCK;
                LR_W(x1, ed, 1, 0);
                AND(x3, x1, x5);
                OR(x3, x3, x6);
                SC_W(x3, x3, ed, 0, 1);
                BNEZ_MARKLOCK(x3);
                ZEXTH(x1, x1);
                B_MARK2_nocond;

                MARK3;
                NOT(x5, x5);
                SLLI(x5, x5, 16);
                SLLI(x6, x6, 16);
                // 0xffffffff0000ffff
                NOT(x5, x5);
                ANDI(x4, ed, ~0b11);
                LR_W(x1, x4, 1, 0);
                AND(x3, x1, x5);
                OR(x3, x3, x6);
                SC_W(x3, x3, x4, 0, 1);
                BNEZ(x3, -4 * 4);
                SRLI(x1, x1, 16);
                ZEXTH(x1, x1);
                B_MARK2_nocond;

                MARK;
                SMDMB();
                LHU(x1, ed, 0);
                SH(gd, ed, 0);
                SMDMB();

                MARK2;
                INSHz(gd, x1, x3, x4, 1, 0);
            }
            break;
        case 0x89:
            INST_NAME("MOV Ew, Gw");
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                if (ed != gd) {
                    INSHz(ed, gd, x2, x3, 1, 1);
                }
            } else {
                SCRATCH_USAGE(0);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, 1, 0);
                SH(gd, ed, fixedaddress);
                SMWRITELOCK(lock);
            }
            break;
        case 0x8B:
            INST_NAME("MOV Gw, Ew");
            nextop = F8;
            GETGD; // don't need GETGW neither
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                if (ed != gd) {
                    INSHz(gd, ed, x2, x3, 1, 1);
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, 1, 0);
                SMREADLOCK(lock);
                LHU(x1, ed, fixedaddress);
                INSHz(gd, x1, x2, x3, 1, 0);
            }
            break;
        case 0x8C:
            INST_NAME("MOV Ew, Seg");
            nextop = F8;
            LHU(x3, xEmu, offsetof(x64emu_t, segs[(nextop & 0x38) >> 3]));
            if (MODREG) {
                INSHz(TO_NAT((nextop & 7) + (rex.b << 3)), x3, x1, x2, 1, 0);
            } else { // mem <= seg
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                SH(x3, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0x8D:
            INST_NAME("LEA Gd, Ed");
            nextop = F8;
            GETGD;
            if (MODREG) { // reg <= reg? that's an invalid operation
                DEFAULT;
            } else { // mem <= reg
                rex.seg = 0;
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 0, 0);
                INSH(gd, ed, x5, x6, 1, 1);
            }
            break;
        case 0x8E:
            INST_NAME("MOV Seg, Ew");
            nextop = F8;
            u8 = (nextop & 0x38) >> 3;
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                LHU(x1, wback, fixedaddress);
                ed = x1;
            }
            SH(ed, xEmu, offsetof(x64emu_t, segs[u8]));
            if((u8==_FS) || (u8==_GS)) {
                // refresh offset if needed
                CBZ_NEXT(ed);
                MOV32w(x1, u8);
                CALL(const_getsegmentbase, -1, x1, x2);
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
            if (gd == xRAX) {
                INST_NAME("NOP");
            } else {
                INST_NAME("XCHG AX, Reg");
                MV(x2, xRAX);
                INSHz(xRAX, gd, x3, x4, 1, 1);
                INSHz(gd, x2, x3, x4, 0, 1);
            }
            break;
        case 0x98:
            INST_NAME("CBW");
            SLLI(x1, xRAX, 56);
            SRAI(x1, x1, 56);
            INSHz(xRAX, x1, x2, x3, 1, 1);
            break;
        case 0x99:
            INST_NAME("CWD");
            SLLI(x1, xRAX, 48);
            SRAI(x1, x1, 48);
            SRLI(x1, x1, 48);
            SRLI(xRDX, xRDX, 16);
            SLLI(xRDX, xRDX, 16);
            OR(xRDX, xRDX, x1);
            break;
        case 0x9C:
            INST_NAME("PUSHF");
            READFLAGS(X_ALL);
            FLAGS_ADJUST_TO11(x3, xFlags, x2);
            PUSH1_16(x3);
            break;
        case 0x9D:
            INST_NAME("POPF");
            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            POP1_16(x1);
            FLAGS_ADJUST_FROM11(x1, x1, x2);
            LUI(x2, 0xffff0);
            AND(xFlags, xFlags, x2);
            OR(xFlags, xFlags, x1);
            MOV32w(x1, 0x3F7FF7);
            AND(xFlags, xFlags, x1);
            ORI(xFlags, xFlags, 0x2);
            SET_DFNONE();
            if (box64_wine) { // should this be done all the time?
                ANDI(x1, xFlags, 1 << F_TF);
                CBZ_NEXT(x1);
                // go to epilog, TF should trigger at end of next opcode, so using Interpreter only
                jump_to_epilog(dyn, addr, 0, ninst);
            }
            break;
        case 0xA1:
            INST_NAME("MOV EAX, Od");
            if (rex.is32bits && rex.is67)
                u64 = F16S;
            else if (rex.is32bits || rex.is67)
                u64 = F32S;
            else
                u64 = F64;
            MOV64y(x1, u64);
            if (rex.seg) {
                grab_segdata(dyn, addr, ninst, x3, rex.seg, 0);
                ADDxREGy(x1, x3, x1);
            }
            lock = (rex.seg) ? 0 : isLockAddress(u64);
            SMREADLOCK(lock);
            LHU(x2, x1, 0);
            INSHz(xRAX, x2, x3, x4, 1, 0);
            break;
        case 0xA3:
            INST_NAME("MOV Od, EAX");
            if (rex.is32bits && rex.is67)
                u64 = F16S;
            else if (rex.is32bits || rex.is67)
                u64 = F32S;
            else
                u64 = F64;
            MOV64y(x1, u64);
            if (rex.seg) {
                grab_segdata(dyn, addr, ninst, x3, rex.seg, 0);
                ADDxREGy(x1, x3, x1);
            }
            lock = (rex.seg) ? 0 : isLockAddress(u64);
            SH(xRAX, x1, 0);
            SMWRITELOCK(lock);
            break;
        case 0xA4:
            return dynarec64_00(dyn, addr - 1, ip, ninst, rex, ok, need_epilog);
        case 0xA5:
            if (rex.rep) {
                INST_NAME("REP MOVSW");
                CBZ_NEXT(xRCX);
                ANDI(x1, xFlags, 1 << F_DF);
                BNEZ_MARK2(x1);
                MARK; // Part with DF==0
                LH(x1, xRSI, 0);
                SH(x1, xRDI, 0);
                ADDI(xRSI, xRSI, 2);
                ADDI(xRDI, xRDI, 2);
                SUBI(xRCX, xRCX, 1);
                BNEZ_MARK(xRCX);
                B_MARKLOCK_nocond;
                MARK2; // Part with DF==1
                LH(x1, xRSI, 0);
                SH(x1, xRDI, 0);
                SUBI(xRSI, xRSI, 2);
                SUBI(xRDI, xRDI, 2);
                SUBI(xRCX, xRCX, 1);
                BNEZ_MARK2(xRCX);
                MARKLOCK;
                // done
            } else {
                INST_NAME("MOVSW");
                GETDIR(x3, x1, 2);
                LH(x1, xRSI, 0);
                IF_UNALIGNED(ip) {
                    SB(x1, xRDI, 0);
                    SRLI(x1, x1, 8);
                    SB(x1, xRDI, 1);
                } else {
                    SH(x1, xRDI, 0);
                }
                ADD(xRSI, xRSI, x3);
                ADD(xRDI, xRDI, x3);
            }
            SMWRITE();
            break;
        case 0xA7:
            switch (rex.rep) {
                case 1:
                case 2:
                    if (rex.rep == 1) {
                        INST_NAME("REPNZ CMPSW");
                    } else {
                        INST_NAME("REPZ CMPSW");
                    }
                    MAYSETFLAGS();
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    CBZ_NEXT(xRCX);
                    ANDI(x1, xFlags, 1 << F_DF);
                    BNEZ_MARK2(x1);
                    MARK; // Part with DF==0
                    LHU(x1, xRSI, 0);
                    LHU(x2, xRDI, 0);
                    ADDI(xRSI, xRSI, 2);
                    ADDI(xRDI, xRDI, 2);
                    SUBI(xRCX, xRCX, 1);
                    if (rex.rep == 1) {
                        BEQ_MARK3(x1, x2);
                    } else {
                        BNE_MARK3(x1, x2);
                    }
                    BNEZ_MARK(xRCX);
                    B_MARK3_nocond;
                    MARK2; // Part with DF==1
                    LHU(x1, xRSI, 0);
                    LHU(x2, xRDI, 0);
                    SUBI(xRSI, xRSI, 2);
                    SUBI(xRDI, xRDI, 2);
                    SUBI(xRCX, xRCX, 1);
                    if (rex.rep == 1) {
                        BEQ_MARK3(x1, x2);
                    } else {
                        BNE_MARK3(x1, x2);
                    }
                    BNEZ_MARK2(xRCX);
                    MARK3; // end
                    emit_cmp16(dyn, ninst, x1, x2, x3, x4, x5, x6);
                    break;
                default:
                    INST_NAME("CMPSW");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETDIR(x3, x1, 2);
                    LHU(x1, xRSI, 0);
                    LHU(x2, xRDI, 0);
                    ADD(xRSI, xRSI, x3);
                    ADD(xRDI, xRDI, x3);
                    emit_cmp16(dyn, ninst, x1, x2, x3, x4, x5, x6);
                    break;
            }
            break;
        case 0xA9:
            INST_NAME("TEST AX,Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            u16 = F16;
            MOV32w(x2, u16);
            SLLIW(x1, xRAX, 16);
            SRLIW(x1, x1, 16);
            emit_test16(dyn, ninst, x1, x2, x3, x4, x5);
            break;
        case 0xAB:
            if (rex.rep) {
                INST_NAME("REP STOSW");
                CBZ_NEXT(xRCX);
                ANDI(x1, xFlags, 1 << F_DF);
                BNEZ_MARK2(x1);
                MARK; // Part with DF==0
                SH(xRAX, xRDI, 0);
                ADDI(xRDI, xRDI, 2);
                SUBI(xRCX, xRCX, 1);
                BNEZ_MARK(xRCX);
                B_NEXT_nocond;
                MARK2; // Part with DF==1
                SH(xRAX, xRDI, 0);
                SUBI(xRDI, xRDI, 2);
                SUBI(xRCX, xRCX, 1);
                BNEZ_MARK2(xRCX);
                // done
            } else {
                INST_NAME("STOSW");
                GETDIR(x3, x1, 2);
                SH(xRAX, xRDI, 0);
                ADD(xRDI, xRDI, x3);
            }
            break;
        case 0xAD:
            if (rex.rep) {
                DEFAULT;
            } else {
                INST_NAME("LODSW");
                GETDIR(x1, x2, 2);
                LHU(x2, xRSI, 0);
                ADD(xRSI, xRSI, x1);
                INSHz(xRAX, x2, x3, x4, 1, 0);
            }
            break;
        case 0xAF:
            switch (rex.rep) {
                case 1:
                case 2:
                    if (rex.rep == 1) {
                        INST_NAME("REPNZ SCASW");
                    } else {
                        INST_NAME("REPZ SCASW");
                    }
                    MAYSETFLAGS();
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    CBZ_NEXT(xRCX);
                    GETDIR(x3, x1, rex.w ? 8 : 2);
                    if (rex.w) {
                        MARK;
                        LD(x2, xRDI, 0);
                        ADD(xRDI, xRDI, x3);
                        ADDI(xRCX, xRCX, -1);
                        if (rex.rep == 1) {
                            BEQ_MARK3(xRAX, x2);
                        } else {
                            BNE_MARK3(xRAX, x2);
                        }
                        BNE_MARK(xRCX, xZR);
                        MARK3;
                        emit_cmp32(dyn, ninst, rex, xRAX, x2, x3, x4, x5, x6);
                    } else {
                        ZEXTH(x1, xRAX);
                        MARK;
                        LHU(x2, xRDI, 0);
                        ADD(xRDI, xRDI, x3);
                        ADDI(xRCX, xRCX, -1);
                        if (rex.rep == 1) {
                            BEQ_MARK3(x1, x2);
                        } else {
                            BNE_MARK3(x1, x2);
                        }
                        BNE_MARK(xRCX, xZR);
                        MARK3;
                        emit_cmp16(dyn, ninst, x1, x2, x3, x4, x5, x6);
                    }
                    break;
                default:
                    INST_NAME("SCASW");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETDIR(x3, x1, rex.w ? 8 : 2);
                    if (rex.w) {
                        LD(x2, xRDI, 0);
                        ADD(xRDI, xRDI, x3);
                        emit_cmp32(dyn, ninst, rex, xRAX, x2, x3, x4, x5, x6);
                    } else {
                        ZEXTH(x1, xRAX);
                        LHU(x2, xRDI, 0);
                        ADD(xRDI, xRDI, x3);
                        emit_cmp16(dyn, ninst, x1, x2, x3, x4, x5, x6);
                    }
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
            INST_NAME("MOV Reg, Iw");
            u16 = F16;
            MOV32w(x1, u16);
            gd = TO_NAT((opcode & 7) + (rex.b << 3));
            INSHz(gd, x1, x2, x3, 1, 0);
            break;

        case 0xC1:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                    INST_NAME("ROL Ew, Ib");
                    if (geted_ib(dyn, addr, ninst, nextop) & 0x1f) {
                        // removed PENDING on purpose
                        SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION);
                        GETEW(x1, 1);
                        u8 = (F8) & 0x1f;
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
                        // removed PENDING on purpose
                        SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION);
                        GETEW(x1, 1);
                        u8 = (F8) & 0x1f;
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
                        // removed PENDING on purpose
                        SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION);
                        GETEW(x1, 1);
                        u8 = (F8) & 0x1f;
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
                        // removed PENDING on purpose
                        SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION);
                        GETEW(x1, 1);
                        u8 = (F8) & 0x1f;
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
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
                        GETEW(x1, 0);
                        u8 = (F8) & 0x1f;
                        emit_shl16c(dyn, ninst, x1, u8, x5, x4, x6);
                        EWBACK;
                    } else {
                        FAKEED;
                        F8;
                    }
                    break;
                case 5:
                    INST_NAME("SHR Ew, Ib");
                    if (geted_ib(dyn, addr, ninst, nextop) & 0x1f) {
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
                        if (MODREG && !dyn->insts[ninst].x64.gen_flags) {
                            // save an srli instruction...
                            wback = TO_NAT((nextop & 7) + (rex.b << 3));
                            u8 = (F8) & 0x1f;
                            if (u8 > 15) {
                                ed = xZR;
                            } else {
                                SLLI(x1, wback, 48);
                                SRLI(x1, x1, 48 + u8);
                                ed = x1;
                            }
                            wb1 = 0;
                            EWBACK;
                            if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(ed, xZR, x5, xZR);
                        } else {
                            GETEW(x1, 0);
                            u8 = (F8) & 0x1f;
                            emit_shr16c(dyn, ninst, x1, u8, x5, x4, x6);
                            EWBACK;
                        }
                    } else {
                        FAKEED;
                        F8;
                    }
                    break;
                case 7:
                    INST_NAME("SAR Ew, Ib");
                    if (geted_ib(dyn, addr, ninst, nextop) & 0x1f) {
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
                        GETSEW(x1, 0);
                        u8 = (F8) & 0x1f;
                        emit_sar16c(dyn, ninst, x1, u8, x5, x4, x6);
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
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                u16 = F16;
                MOV32w(x1, u16);
                INSHz(ed, x1, x2, x3, 1, 0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, 1, 2);
                u16 = F16;
                MOV32w(x1, u16);
                SH(x1, ed, fixedaddress);
                SMWRITELOCK(lock);
            }
            break;
        case 0xD1:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                    INST_NAME("ROL Ew, 1");
                    // removed PENDING on purpose
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION);
                    GETEW(x1, 0);
                    emit_rol16c(dyn, ninst, x1, 1, x5, x4);
                    EWBACK;
                    break;
                case 1:
                    INST_NAME("ROR Ew, 1");
                    // removed PENDING on purpose
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION);
                    GETEW(x1, 1);
                    emit_ror16c(dyn, ninst, x1, 1, x5, x4);
                    EWBACK;
                    break;
                case 2:
                    INST_NAME("RCL Ew, 1");
                    READFLAGS(X_CF);
                    // removed PENDING on purpose
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION);
                    GETEW(x1, 0);
                    emit_rcl16c(dyn, ninst, x1, 1, x5, x4);
                    EWBACK;
                    break;
                case 3:
                    INST_NAME("RCR Ew, 1");
                    READFLAGS(X_CF);
                    // removed PENDING on purpose
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION);
                    GETEW(x1, 0);
                    emit_rcr16c(dyn, ninst, x1, 1, x5, x4);
                    EWBACK;
                    break;
                case 5:
                    INST_NAME("SHR Ew, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
                    GETEW(x1, 0);
                    emit_shr16c(dyn, ninst, x1, 1, x5, x4, x6);
                    EWBACK;
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ew, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
                    GETEW(x1, 0);
                    emit_shl16c(dyn, ninst, x1, 1, x5, x4, x6);
                    EWBACK;
                    break;
                case 7:
                    INST_NAME("SAR Ew, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
                    GETSEW(x1, 0);
                    emit_sar16c(dyn, ninst, x1, 1, x5, x4, x6);
                    EWBACK;
                    break;
            }
            break;

        case 0xD3:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                    INST_NAME("ROL Ew, CL");
                    ANDI(x2, xRCX, 0x1f);
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    SETFLAGS(X_OF | X_CF, SF_SET_DF, NAT_FLAGS_NOFUSION);
                    if (BOX64DRENV(dynarec_safeflags) > 1) MAYSETFLAGS();
                    GETEW(x1, 1);
                    CALL_(const_rol16, x1, x3, x1, x2);
                    EWBACK;
                    break;
                case 1:
                    INST_NAME("ROR Ew, CL");
                    ANDI(x2, xRCX, 0x1f);
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    SETFLAGS(X_OF | X_CF, SF_SET_DF, NAT_FLAGS_NOFUSION);
                    if (BOX64DRENV(dynarec_safeflags) > 1) MAYSETFLAGS();
                    GETEW(x1, 1);
                    CALL_(const_ror16, x1, x3, x1, x2);
                    EWBACK;
                    break;
                case 2:
                    INST_NAME("RCL Ew, CL");
                    ANDI(x2, xRCX, 0x1f);
                    MESSAGE("LOG_DUMP", "Need optimization\n");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF | X_CF, SF_SET_DF, NAT_FLAGS_NOFUSION);
                    if (BOX64DRENV(dynarec_safeflags) > 1) MAYSETFLAGS();
                    GETEW(x1, 1);
                    CALL_(const_rcl16, x1, x3, x1, x2);
                    EWBACK;
                    break;
                case 3:
                    INST_NAME("RCR Ew, CL");
                    ANDI(x2, xRCX, 0x1f);
                    MESSAGE("LOG_DUMP", "Need optimization\n");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF | X_CF, SF_SET_DF, NAT_FLAGS_NOFUSION);
                    if (BOX64DRENV(dynarec_safeflags) > 1) MAYSETFLAGS();
                    GETEW(x1, 1);
                    CALL_(const_rcr16, x1, x3, x1, x2);
                    EWBACK;
                    break;
                case 5:
                    INST_NAME("SHR Ew, CL");
                    ANDI(x2, xRCX, 0x1f);
                    BEQ_NEXT(x2, xZR);
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
                    if (BOX64DRENV(dynarec_safeflags) > 1) MAYSETFLAGS();
                    GETEW(x1, 0);
                    emit_shr16(dyn, ninst, x1, x2, x5, x4, x6);
                    EWBACK;
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ew, CL");
                    ANDI(x2, xRCX, 0x1f);
                    BEQ_NEXT(x2, xZR);
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
                    if (BOX64DRENV(dynarec_safeflags) > 1) MAYSETFLAGS();
                    GETEW(x1, 0);
                    emit_shl16(dyn, ninst, x1, x2, x5, x4, x6);
                    EWBACK;
                    break;
                case 7:
                    INST_NAME("SAR Ew, CL");
                    ANDI(x2, xRCX, 0x1f);
                    BEQ_NEXT(x2, xZR);
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
                    if (BOX64DRENV(dynarec_safeflags) > 1) MAYSETFLAGS();
                    GETSEW(x1, 0);
                    emit_sar16(dyn, ninst, x1, x2, x5, x4, x6);
                    EWBACK;
                    break;
            }
            break;
        case 0xE4: /* IN AL, Ib */
        case 0xE5: /* IN AX, Ib */
        case 0xE6: /* OUT Ib, AL */
        case 0xE7: /* OUT Ib, AX */
            return dynarec64_00(dyn, addr - 1, ip, ninst, rex, ok, need_epilog);
        case 0xEC: /* IN AL, DX */
        case 0xED: /* IN AX, DX */
        case 0xEE: /* OUT DX, AL */
        case 0xEF: /* OUT DX, AX */
            return dynarec64_00(dyn, addr - 1, ip, ninst, rex, ok, need_epilog);
        case 0xF7:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                case 1:
                    INST_NAME("TEST Ew, Iw");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEW(x1, 2);
                    u16 = F16;
                    MOV32w(x2, u16);
                    emit_test16(dyn, ninst, x1, x2, x6, x4, x5);
                    break;
                case 2:
                    INST_NAME("NOT Ew");
                    GETEW(x1, 0);
                    MOV32w(x5, 0xffff);
                    XOR(ed, ed, x5); // No flags affected
                    EWBACK;
                    break;
                case 3:
                    INST_NAME("NEG Ew");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEW(x1, 0);
                    emit_neg16(dyn, ninst, ed, x2, x4, x5);
                    EWBACK;
                    break;
                case 4:
                    INST_NAME("MUL AX, Ew");
                    SETFLAGS(X_ALL, SF_PENDING, NAT_FLAGS_NOFUSION);
                    GETEW(x1, 0);
                    ZEXTH(x2, xRAX);
                    MULW(x1, x2, x1);
                    ZEROUP(x1);
                    UFLAG_RES(x1);
                    INSHz(xRAX, x1, x4, x5, 1, 1);
                    SRLI(xRDX, xRDX, 16);
                    SLLI(xRDX, xRDX, 16);
                    SRLI(x1, x1, 16);
                    OR(xRDX, xRDX, x1);
                    UFLAG_DF(x1, d_mul16);
                    break;
                case 5:
                    INST_NAME("IMUL AX, Ew");
                    SETFLAGS(X_ALL, SF_PENDING, NAT_FLAGS_NOFUSION);
                    GETSEW(x1, 0);
                    SLLI(x2, xRAX, 16);
                    SRAIW(x2, x2, 16);
                    MULW(x1, x2, x1);
                    ZEROUP(x1);
                    UFLAG_RES(x1);
                    INSHz(xRAX, x1, x4, x5, 1, 1);
                    SRLI(xRDX, xRDX, 16);
                    SLLI(xRDX, xRDX, 16);
                    SRLI(x1, x1, 16);
                    OR(xRDX, xRDX, x1);
                    UFLAG_DF(x1, d_imul16);
                    break;
                case 6:
                    INST_NAME("DIV Ew");
                    SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
                    SET_DFNONE();
                    GETEW(x1, 0);
                    ZEXTH(x2, xRAX);
                    SLLI(x7, xRDX, 48);
                    SRLI(x7, x7, 32);
                    OR(x2, x2, x7);
                    if (BOX64ENV(dynarec_div0)) {
                        BNE_MARK3(ed, xZR);
                        GETIP_(ip, x7);
                        STORE_XEMU_CALL(x6);
                        CALL(const_native_div0, -1, 0, 0);
                        CLEARIP();
                        LOAD_XEMU_CALL();
                        jump_to_epilog(dyn, 0, xRIP, ninst);
                        MARK3;
                    }
                    DIVUW(x7, x2, ed);
                    REMUW(x4, x2, ed);
                    INSHz(xRAX, x7, x5, x6, 1, 1);
                    INSHz(xRDX, x4, x5, x6, 0, 1);
                    SET_DFNONE();
                    CLEAR_FLAGS();
                    ADDI(x5, xZR, ((1 << F_ZF) | (1 << F_PF)));
                    OR(xFlags, xFlags, x5);
                    break;
                case 7:
                    INST_NAME("IDIV Ew");
                    NOTEST(x1);
                    SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
                    SET_DFNONE();
                    GETSEW(x1, 0);
                    if (BOX64ENV(dynarec_div0)) {
                        BNE_MARK3(ed, xZR);
                        GETIP_(ip, x7);
                        STORE_XEMU_CALL(x6);
                        CALL(const_native_div0, -1, 0, 0);
                        CLEARIP();
                        LOAD_XEMU_CALL();
                        jump_to_epilog(dyn, 0, xRIP, ninst);
                        MARK3;
                    }
                    ZEXTH(x2, xRAX);
                    SLLI(x3, xRDX, 48);
                    SRLI(x3, x3, 32);
                    OR(x2, x2, x3);
                    DIVW(x3, x2, ed);
                    REMW(x4, x2, ed);
                    INSHz(xRAX, x3, x5, x6, 1, 1);
                    INSHz(xRDX, x4, x5, x6, 0, 1);
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xF8:
        case 0xF9:
            return dynarec64_00(dyn, addr - 1, ip, ninst, rex, ok, need_epilog);
        case 0xFF:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                    INST_NAME("INC Ew");
                    SETFLAGS(X_ALL & ~X_CF, SF_SUBSET_PENDING, NAT_FLAGS_FUSION);
                    GETEW(x1, 0);
                    emit_inc16(dyn, ninst, x1, x2, x4, x5, x6);
                    EWBACK;
                    break;
                case 1:
                    INST_NAME("DEC Ew");
                    SETFLAGS(X_ALL & ~X_CF, SF_SUBSET_PENDING, NAT_FLAGS_FUSION);
                    GETEW(x1, 0);
                    emit_dec16(dyn, ninst, x1, x2, x4, x5, x6);
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
