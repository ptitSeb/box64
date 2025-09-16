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

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "../dynarec_helper.h"
#include "dynarec_rv64_functions.h"


uintptr_t dynarec64_66F0(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
{
    (void)ip;
    (void)rep;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop;
    uint8_t gd, ed, u8;
    uint8_t wback, wb1, wb2, gb1, gb2;
    int32_t i32;
    int64_t i64, j64;
    uint64_t u64;
    int64_t fixedaddress;
    int unscaled;
    MAYUSE(gb1);
    MAYUSE(gb2);
    MAYUSE(wb1);
    MAYUSE(wb2);
    MAYUSE(j64);

    while ((opcode == 0xF2) || (opcode == 0xF3)) {
        rep = opcode - 0xF1;
        opcode = F8;
    }

    GETREX();

    switch (opcode) {
        case 0x0F:
            opcode = F8;
            switch (opcode) {
                case 0xB1:
                    INST_NAME("LOCK CMPXCHG Ew, Gw");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    nextop = F8;
                    GETGD;
                    ZEXTH(x6, xRAX);
                    if (MODREG) {
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        wback = 0;
                        ZEXTH(x1, ed);
                        BNE_MARK(x6, x1);
                        INSH(ed, gd, x2, x3, 1, 1);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x3, x2, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                        ANDI(x5, wback, 0b10);
                        BNEZ_MARK2(x5);

                        // lower 16 bits
                        MARKLOCK;
                        LR_W(x2, wback, 1, 1);
                        ZEXTH(x1, x2);
                        BNE_MARK(x6, x1);
                        INSH(x2, gd, x5, x4, 1, 1);
                        SC_W(x5, x2, wback, 1, 1);
                        BNEZ_MARKLOCK(x5);
                        B_MARK_nocond;

                        MARK2;
                        // upper 16 bits
                        XORI(wback, wback, 0b10);
                        MARKLOCK2;
                        LR_W(x2, wback, 1, 1);
                        SRLIW(x1, x2, 16);
                        BNE_MARK(x6, x1);
                        SLLIW(x2, x2, 16);
                        SRLIW(x2, x2, 16);
                        SLLIW(x5, gd, 16);
                        OR(x2, x2, x5);
                        SC_W(x5, x2, wback, 1, 1);
                        BNEZ_MARKLOCK2(x5);
                    }
                    MARK;
                    UFLAG_IF { emit_cmp16(dyn, ninst, x6, x1, x2, x3, x4, x5); }
                    INSH(xRAX, x1, x2, x3, 1, 0);
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
                        INST_NAME("LOCK ADD Ew, Iw");
                    } else {
                        INST_NAME("LOCK ADD Ew, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    if (MODREG) {
                        if (opcode == 0x81)
                            u64 = F16;
                        else
                            u64 = (uint16_t)(int16_t)F8S;
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        MOV64x(x5, u64);
                        ZEXTH(x6, ed);
                        emit_add16(dyn, ninst, x6, x5, x3, x4, x2);
                        SRLI(ed, ed, 16);
                        SLLI(ed, ed, 16);
                        OR(ed, ed, x6);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, (opcode == 0x81) ? 2 : 1);
                        if (opcode == 0x81)
                            u64 = F16;
                        else
                            u64 = (uint16_t)(int16_t)F8S;
                        MOV64x(x5, u64);

                        ANDI(x3, wback, 0b10);
                        BNEZ_MARK(x3);

                        // lower 16 bits
                        MARKLOCK;
                        LR_W(x1, wback, 1, 1);
                        SRLIW(x3, x1, 16);
                        SLLIW(x3, x3, 16);
                        ADD(x4, x1, x5);
                        SLLIW(x4, x4, 16);
                        SRLIW(x4, x4, 16);
                        OR(x4, x4, x3);
                        SC_W(x3, x4, wback, 1, 1);
                        BNEZ_MARKLOCK(x3);
                        IFXORNAT (X_ALL | X_PEND) {
                            SLLIW(x1, x1, 16);
                            SRLIW(x1, x1, 16);
                        }
                        B_MARK3_nocond;

                        MARK;
                        // upper 16 bits
                        XORI(wback, wback, 0b10);
                        MARK2;
                        LR_W(x1, wback, 1, 1);
                        SLLIW(x3, x1, 16);
                        SRLIW(x3, x3, 16);
                        SRLIW(x1, x1, 16);
                        ADD(x4, x1, x5);
                        SLLIW(x4, x4, 16);
                        OR(x4, x4, x3);
                        SC_W(x3, x4, wback, 1, 1);
                        BNEZ_MARK2(x3);

                        MARK3;
                        // final
                        IFXORNAT (X_ALL | X_PEND) {
                            emit_add16(dyn, ninst, x1, x5, x3, x4, x6);
                        }
                    }
                    break;
                case 1: // OR
                    if (opcode == 0x81) {
                        INST_NAME("LOCK OR Ew, Iw");
                    } else {
                        INST_NAME("LOCK OR Ew, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    if (MODREG) {
                        if (opcode == 0x81)
                            u64 = F16;
                        else
                            u64 = (uint16_t)(int16_t)F8S;
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        MOV64x(x5, u64);
                        ZEXTH(x6, ed);
                        emit_or16(dyn, ninst, x6, x5, x3, x4);
                        SRLI(ed, ed, 16);
                        SLLI(ed, ed, 16);
                        OR(ed, ed, x6);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, (opcode == 0x81) ? 2 : 1);
                        if (opcode == 0x81)
                            u64 = F16;
                        else
                            u64 = (uint16_t)(int16_t)F8S;
                        MOV64x(x5, u64);

                        ANDI(x3, wback, 0b10);
                        BNEZ_MARK(x3);

                        // lower 16 bits
                        MARKLOCK;
                        LR_W(x1, wback, 1, 1);
                        SRLIW(x3, x1, 16);
                        SLLIW(x3, x3, 16);
                        OR(x4, x1, x5);
                        OR(x4, x4, x3);
                        SC_W(x3, x4, wback, 1, 1);
                        BNEZ_MARKLOCK(x3);
                        IFXORNAT (X_ALL | X_PEND) {
                            SLLIW(x1, x1, 16);
                            SRLIW(x1, x1, 16);
                        }
                        B_MARK3_nocond;

                        MARK;
                        // upper 16 bits
                        XORI(wback, wback, 0b10);
                        MARK2;
                        LR_W(x1, wback, 1, 1);
                        SLLIW(x3, x1, 16);
                        SRLIW(x3, x3, 16);
                        SRLIW(x1, x1, 16);
                        OR(x4, x1, x5);
                        SLLIW(x4, x4, 16);
                        OR(x4, x4, x3);
                        SC_W(x3, x4, wback, 1, 1);
                        BNEZ_MARK2(x3);

                        MARK3;
                        // final
                        IFXORNAT (X_ALL | X_PEND) {
                            emit_or16(dyn, ninst, x1, x5, x3, x4);
                        }
                    }
                    break;
                case 4: // AND
                    if (opcode == 0x81) {
                        INST_NAME("LOCK AND Ew, Iw");
                    } else {
                        INST_NAME("LOCK AND Ew, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    if (MODREG) {
                        if (opcode == 0x81)
                            u64 = F16;
                        else
                            u64 = (uint16_t)(int16_t)F8S;
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        MOV64x(x5, u64);
                        ZEXTH(x6, ed);
                        emit_and16(dyn, ninst, x6, x5, x3, x4);
                        SRLI(ed, ed, 16);
                        SLLI(ed, ed, 16);
                        OR(ed, ed, x6);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, (opcode == 0x81) ? 2 : 1);
                        if (opcode == 0x81)
                            u64 = F16;
                        else
                            u64 = (uint16_t)(int16_t)F8S;
                        MOV64x(x5, u64);

                        ANDI(x3, wback, 0b10);
                        BNEZ_MARK(x3);

                        // lower 16 bits
                        MARKLOCK;
                        LR_W(x1, wback, 1, 1);
                        SRLIW(x3, x1, 16);
                        SLLIW(x3, x3, 16);
                        AND(x4, x1, x5);
                        OR(x4, x4, x3);
                        SC_W(x3, x4, wback, 1, 1);
                        BNEZ_MARKLOCK(x3);
                        IFXORNAT (X_ALL | X_PEND) {
                            SLLIW(x1, x1, 16);
                            SRLIW(x1, x1, 16);
                        }
                        B_MARK3_nocond;

                        MARK;
                        // upper 16 bits
                        XORI(wback, wback, 0b10);
                        MARK2;
                        LR_W(x1, wback, 1, 1);
                        SLLIW(x3, x1, 16);
                        SRLIW(x3, x3, 16);
                        SRLIW(x1, x1, 16);
                        AND(x4, x1, x5);
                        SLLIW(x4, x4, 16);
                        OR(x4, x4, x3);
                        SC_W(x3, x4, wback, 1, 1);
                        BNEZ_MARK2(x3);

                        MARK3;
                        // final
                        IFXORNAT (X_ALL | X_PEND) {
                            emit_and16(dyn, ninst, x1, x5, x3, x4);
                        }
                    }
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
