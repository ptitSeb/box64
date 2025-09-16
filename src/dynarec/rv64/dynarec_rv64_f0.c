#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
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


uintptr_t dynarec64_F0(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
{
    (void)ip;
    (void)rep;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop;
    uint8_t gd, ed, u8;
    uint8_t wback, wb1, wb2, eb1, eb2, gb1, gb2;
    int32_t i32;
    int64_t i64, j64;
    int64_t fixedaddress;
    int unscaled;
    MAYUSE(eb1);
    MAYUSE(eb2);
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

    // TODO: Add support for unaligned memory access for all the LOCK ones.
    // TODO: Add support for BOX4_DYNAREC_ALIGNED_ATOMICS.

    switch (opcode) {
        case 0x01:
            INST_NAME("LOCK ADD Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                emit_add32(dyn, ninst, rex, ed, gd, x3, x4, x5);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                AMOADDxw(x1, gd, wback, 1, 1);
                IFXORNAT (X_ALL | X_PEND) {
                    emit_add32(dyn, ninst, rex, x1, gd, x3, x4, x5);
                }
            }
            break;
        case 0x08:
            INST_NAME("LOCK OR Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGB(x1);
            if (MODREG) {
                GETEB(x2, 1);
                emit_or8(dyn, ninst, x2, x1, x4, x5);
                EBBACK(x5, 0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x5, x6, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                ANDI(x2, wback, 3);
                SLLI(x2, x2, 3);
                ANDI(x3, wback, ~3);
                SLL(x1, x1, x2);
                AMOOR_W(x4, x1, x3, 1, 1);
                IFXORNAT (X_ALL | X_PEND) {
                    SRL(x2, x4, x2);
                    ANDI(x2, x2, 0xFF);
                    emit_or8(dyn, ninst, x2, x1, x4, x5);
                }
            }
            break;
        case 0x09:
            INST_NAME("LOCK OR Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                emit_or32(dyn, ninst, rex, ed, gd, x3, x4);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                AMOORxw(x1, gd, wback, 1, 1);
                IFXORNAT (X_ALL | X_PEND)
                    emit_or32(dyn, ninst, rex, x1, gd, x3, x4);
            }
            break;

        case 0x0F:
            nextop = F8;
            switch (nextop) {
                case 0xAB:
                    INST_NAME("LOCK BTS Ed, Gd");
                    SETFLAGS(X_ALL & ~X_ZF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                    SET_DFNONE();
                    nextop = F8;
                    GETGD;
                    if (MODREG) {
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        wback = 0;
                        BEXT(x4, ed, gd, x2);
                        ANDI(xFlags, xFlags, ~1);
                        OR(xFlags, xFlags, x4);
                        ADDI(x4, xZR, 1);
                        ANDI(x2, gd, rex.w ? 0x3f : 0x1f);
                        SLL(x4, x4, x2);
                        OR(ed, ed, x4);
                        if (!rex.w) ZEROUP(ed);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x3, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                        SRAIxw(x1, gd, 5 + rex.w);
                        ADDSL(x3, wback, x1, 2 + rex.w, x1);
                        ed = x1;
                        wback = x3;
                        MARKLOCK;
                        LRxw(ed, wback, 1, 1);
                        BEXT(x4, ed, gd, x2);
                        ANDI(xFlags, xFlags, ~1);
                        OR(xFlags, xFlags, x4);
                        ADDI(x4, xZR, 1);
                        ANDI(x2, gd, rex.w ? 0x3f : 0x1f);
                        SLL(x4, x4, x2);
                        OR(ed, ed, x4);
                        SCxw(x7, ed, wback, 1, 1);
                        BNEZ_MARKLOCK(x7);
                    }
                    break;
                case 0xB0:
                    switch (rep) {
                        case 0:
                            INST_NAME("LOCK CMPXCHG Eb, Gb");
                            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_NOFUSION);
                            nextop = F8;
                            ANDI(x6, xRAX, 0xff); // AL
                            if (MODREG) {
                                if (rex.rex) {
                                    wback = TO_NAT((nextop & 7) + (rex.b << 3));
                                    wb2 = 0;
                                } else {
                                    wback = (nextop & 7);
                                    wb2 = (wback >> 2) * 8;
                                    wback = TO_NAT(wback & 3);
                                }
                                if (wb2) {
                                    SRLI(x2, wback, wb2);
                                    ANDI(x2, x2, 0xff);
                                } else {
                                    ANDI(x2, wback, 0xff);
                                }
                                wb1 = 0;
                                ed = x2;
                                UFLAG_IF {
                                    emit_cmp8(dyn, ninst, x6, ed, x3, x4, x5, x1);
                                }
                                BNE_MARK2(x6, x2);
                                GETGB(x5);
                                if (wb2) {
                                    MOV64x(x4, ~0xff00);
                                    AND(wback, wback, x4);
                                    SLLI(gd, gd, 8);
                                } else {
                                    ANDI(wback, wback, ~0xff);
                                }
                                OR(wback, wback, gd);
                                MARK2;
                                ANDI(xRAX, xRAX, ~0xff);
                                OR(xRAX, xRAX, x2);
                            } else {
                                if (rex.rex) {
                                    gb1 = TO_NAT(((nextop & 0x38) >> 3) + (rex.r << 3));
                                    gb2 = 0;
                                } else {
                                    gd = (nextop & 0x38) >> 3;
                                    gb2 = ((gd & 4) >> 2);
                                    gb1 = TO_NAT(gd & 3);
                                }
                                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x2, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                                ANDI(x5, wback, 0b11);
                                SLLI(x5, x5, 3); // shamt
                                MARKLOCK;
                                ANDI(x2, wback, ~0b11); // align to 32bit
                                LR_W(x4, x2, 1, 1);
                                MV(x1, x4);
                                SRL(x4, x4, x5);
                                ANDI(x4, x4, 0xff);
                                BNE_MARK(x6, x4); // compare AL with m8
                                // AL == m8, r8 is loaded into m8
                                ADDI(x2, xZR, 0xff);
                                SLL(x2, x2, x5);
                                NOT(x2, x2);
                                AND(x2, x1, x2);
                                if (gb2) {
                                    SRLI(x1, gb1, 8);
                                    ANDI(x1, x1, 0xff);
                                } else {
                                    ANDI(x1, gb1, 0xff);
                                }
                                SLL(x1, x1, x5);
                                OR(x1, x1, x2);
                                ANDI(x2, wback, ~0b11); // align to 32bit again
                                SC_W(x7, x1, x2, 1, 1);
                                BNEZ_MARKLOCK(x7);
                                // done
                                MARK;
                                UFLAG_IF { emit_cmp8(dyn, ninst, x6, x4, x1, x2, x3, x5); }
                                // load m8 into AL
                                ANDI(xRAX, xRAX, ~0xff);
                                OR(xRAX, xRAX, x4);
                            }
                            break;
                        default:
                            DEFAULT;
                    }
                    break;
                case 0xB1:
                    switch (rep) {
                        case 0:
                            INST_NAME("LOCK CMPXCHG Ed, Gd");
                            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_NOFUSION);
                            nextop = F8;
                            GETGD;
                            if (MODREG) {
                                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                                wback = 0;
                                UFLAG_IF { emit_cmp32(dyn, ninst, rex, xRAX, ed, x3, x4, x5, x6); }
                                MV(x1, ed); // save value
                                SUBxw(x2, ed, xRAX);
                                BNE_MARK2(x2, xZR);
                                MVxw(ed, gd);
                                MARK2;
                                MVxw(xRAX, x1);
                            } else {
                                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                                ANDI(x1, wback, (1 << (rex.w + 2)) - 1);
                                BNEZ_MARK3(x1);
                                // Aligned
                                MARKLOCK;
                                LRxw(x1, wback, 1, 1);
                                SUBxw(x3, x1, xRAX);
                                BNEZ_MARK(x3);
                                // EAX == Ed
                                SCxw(x4, gd, wback, 1, 1);
                                BNEZ_MARKLOCK(x4);
                                B_MARK_nocond;
                                MARK3;
                                // Unaligned
                                ANDI(x5, wback, -(1 << (rex.w + 2)));
                                MARKLOCK2;
                                LDxw(x1, wback, 0);
                                LRxw(x6, x5, 1, 1);
                                SUBxw(x3, x1, xRAX);
                                BNEZ_MARK(x3);
                                // EAX == Ed
                                SCxw(x4, x6, x5, 1, 1);
                                BNEZ_MARKLOCK2(x4);
                                SDxw(gd, wback, 0);
                                MARK;
                                UFLAG_IF { emit_cmp32(dyn, ninst, rex, xRAX, x1, x3, x4, x5, x6); }
                                MVxw(xRAX, x1);
                            }
                            break;
                        default:
                            DEFAULT;
                    }
                    break;
                case 0xB3:
                    INST_NAME("LOCK BTR Ed, Gd");
                    SETFLAGS(X_ALL & ~X_ZF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                    SET_DFNONE();
                    nextop = F8;
                    GETGD;
                    if (MODREG) {
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        wback = 0;
                        BEXT(x4, ed, gd, x2); // F_CF is 1
                        ANDI(xFlags, xFlags, ~1);
                        OR(xFlags, xFlags, x4);
                        ADDI(x4, xZR, 1);
                        ANDI(x2, gd, rex.w ? 0x3f : 0x1f);
                        SLL(x4, x4, x2);
                        NOT(x4, x4);
                        AND(ed, ed, x4);
                        if (!rex.w) ZEROUP(ed);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x3, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                        SRAIxw(x1, gd, 5 + rex.w);
                        ADDSL(x3, wback, x1, 2 + rex.w, x1);
                        LDxw(x1, x3, fixedaddress);
                        ed = x1;
                        wback = x3;
                        MARKLOCK;
                        LRxw(ed, wback, 1, 1);
                        BEXT(x4, ed, gd, x2); // F_CF is 1
                        ANDI(xFlags, xFlags, ~1);
                        OR(xFlags, xFlags, x4);
                        ADDI(x4, xZR, 1);
                        ANDI(x2, gd, rex.w ? 0x3f : 0x1f);
                        SLL(x4, x4, x2);
                        NOT(x4, x4);
                        AND(ed, ed, x4);
                        SCxw(x7, ed, wback, 1, 1);
                        BNEZ_MARKLOCK(x7);
                    }
                    break;
                case 0xC1:
                    switch (rep) {
                        case 0:
                            INST_NAME("LOCK XADD Gd, Ed");
                            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                            nextop = F8;
                            GETGD;
                            if (MODREG) {
                                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                                MVxw(x1, ed);
                                MVxw(ed, gd);
                                MVxw(gd, x1);
                                emit_add32(dyn, ninst, rex, ed, gd, x3, x4, x5);
                            } else {
                                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                                AMOADDxw(x1, gd, wback, 1, 1);
                                IFXORNAT (X_ALL | X_PEND) {
                                    MVxw(x2, x1);
                                    emit_add32(dyn, ninst, rex, x2, gd, x3, x4, x5);
                                }
                                MVxw(gd, x1);
                            }
                            break;
                        default:
                            DEFAULT;
                    }
                    break;

                case 0xC7:
                    switch (rep) {
                        case 0:
                            switch (rex.w) {
                                case 0:
                                    INST_NAME("LOCK CMPXCHG8B Gq, Eq");
                                    SETFLAGS(X_ZF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                                    nextop = F8;
                                    addr = geted(dyn, addr, ninst, nextop, &wback, x1, x2, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                                    ANDI(xFlags, xFlags, ~(1 << F_ZF));
                                    ZEXTW2(x3, xRAX);
                                    SLLI(x2, xRDX, 32);
                                    OR(x3, x3, x2); // x3 is edx:eax
                                    ZEXTW2(x4, xRBX);
                                    SLLI(x2, xRCX, 32);
                                    OR(x4, x4, x2); // x4 is ecx:ebx
                                    ANDI(x5, wback, 7);
                                    BNEZ_MARK3(x5);
                                    // Aligned
                                    MARKLOCK;
                                    LR_D(x2, wback, 1, 1);
                                    BNE_MARK(x2, x3);          // edx:eax != ed, load m64 into edx:eax
                                    SC_D(x5, x4, wback, 1, 1); // set ZF and load ecx:ebx into m64
                                    BNEZ_MARKLOCK(x5);
                                    ORI(xFlags, xFlags, 1 << F_ZF);
                                    B_NEXT_nocond;
                                    MARK;
                                    SRLI(xRDX, x2, 32);
                                    ZEXTW2(xRAX, x2);
                                    B_NEXT_nocond;
                                    MARK3;
                                    // Unaligned
                                    ANDI(x5, wback, -8);
                                    MARKLOCK2;
                                    LD(x2, wback, 0);
                                    LR_D(x6, x5, 1, 1);
                                    BNE_MARK2(x2, x3); // edx:eax != ed, load m64 into edx:eax
                                    SC_D(x7, x6, x5, 1, 1);
                                    BNEZ_MARKLOCK2(x7);
                                    SD(x4, wback, 0); // set ZF and load ecx:ebx into m64
                                    ORI(xFlags, xFlags, 1 << F_ZF);
                                    B_NEXT_nocond;
                                    MARK2;
                                    SRLI(xRDX, x2, 32);
                                    ZEXTW2(xRAX, x2);
                                    break;
                                case 1:
                                    INST_NAME("LOCK CMPXCHG16B Gq, Eq");
                                    static int warned = 0;
                                    PASS3(if (!warned) dynarec_log(LOG_INFO, "Warning, LOCK CMPXCHG16B is not well supported on RISC-V and issues are expected.\n"));
                                    PASS3(warned = 1);
                                    SETFLAGS(X_ZF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                                    nextop = F8;
                                    addr = geted(dyn, addr, ninst, nextop, &wback, x1, x2, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                                    ANDI(xFlags, xFlags, ~(1 << F_ZF));
                                    // there is no atomic move on 16bytes, so implement it with mutex
                                    LD(x7, xEmu, offsetof(x64emu_t, context));
                                    ADDI(x7, x7, offsetof(box64context_t, mutex_16b));
                                    ADDI(x4, xZR, 1);
                                    MARK2;
                                    AMOSWAP_W(x4, x4, x7, 1, 1);
                                    // x4 == 1 if locked
                                    BNEZ_MARK2(x4);

                                    MARKLOCK;
                                    LD(x3, wback, 8);
                                    LR_D(x2, wback, 1, 1);
                                    BNE_MARK(x2, xRAX);
                                    BNE_MARK(x3, xRDX);
                                    SC_D(x5, xRBX, wback, 1, 1);
                                    BNEZ_MARKLOCK(x5);
                                    SD(xRCX, wback, 8);
                                    ORI(xFlags, xFlags, 1 << F_ZF);
                                    B_MARK3_nocond;
                                    MARK;
                                    MV(xRAX, x2);
                                    MV(xRDX, x3);
                                    MARK3;

                                    // unlock
                                    AMOSWAP_W(xZR, xZR, x7, 1, 1);
                                    break;
                            }
                            break;
                        default:
                            DEFAULT;
                    }
                    break;

                default:
                    DEFAULT;
            }
            break;
        case 0x10:
            INST_NAME("LOCK ADC Eb, Gb");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGB(x2);
            if (MODREG) {
                if (rex.rex) {
                    wback = TO_NAT((nextop & 7) + (rex.b << 3));
                    wb2 = 0;
                } else {
                    wback = (nextop & 7);
                    wb2 = (wback >> 2);
                    wback = TO_NAT(wback & 3);
                }
                SRLIW(x1, wback, wb2 * 8);
                ANDI(x1, x1, 0xFF);
                emit_adc8(dyn, ninst, x1, x2, x3, x4, x5);
                SLLI(x5, x1, wb2 * 8);
                MOV_U12(x3, 0xFF);
                SLLI(x3, x3, wb2 * 8);
                NOT(x3, x3);
                AND(wback, wback, x3);
                OR(wback, wback, x5);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x1, x3, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                ANDI(x3, wback, 0b11);
                BNEZ_MARK(x3);
                MARKLOCK;
                LR_W(x5, wback, 1, 1);
                ANDI(x4, x5, 0xff);  // x4 = Ed.b[0]
                ANDI(x5, x5, ~0xff); // x5 = clear Ed.b[0]
                ADDW(x6, x4, x2);
                ANDI(x7, xFlags, 1 << F_CF);
                ADDW(x6, x6, x7); // x6 = adc
                ANDI(x6, x6, 0xff);
                OR(x5, x5, x6);
                SC_W(x7, x5, wback, 1, 1);
                BNEZ_MARKLOCK(x7);
                B_MARK3_nocond;
                MARK;
                SLLI(x3, x3, 3);
                ADDI(x4, xZR, 0xff);
                ANDI(wback, wback, ~3); // aligning address
                SLL(x4, x4, x3);       // x4 = byte mask
                NOT(x5, x4);            // x5 = ~mask
                SLL(x2, x2, x3);        // x2 = extented Gb
                MARK2;
                LR_W(x6, wback, 1, 1); // x6 = Ed
                AND(x7, x6, x4);       // x7 = extended Ed.b[dest]
                AND(x6, x6, x5);       // x6 = clear Ed.b[dest]
                ADDW(x5, x7, x2);
                ANDI(x4, xFlags, 1 << F_CF);
                SLL(x4, x4, x3);  // extented
                ADDW(x5, x5, x4); // x5 = adc
                ADDI(x4, xZR, 0xff);
                SLL(x4, x4, x3);
                AND(x5, x5, x4);
                OR(x5, x5, x6);
                SC_W(x4, x5, wback, 1, 1);
                BNEZ_MARK2(x4);
                IFXORNAT (X_ALL | X_PEND) {
                    SRL(x2, x2, x3); // Gb
                    SRL(x4, x7, x3); // Eb
                }
                MARK3;
                IFXORNAT (X_ALL | X_PEND) {
                    emit_adc8(dyn, ninst, x4, x2, x3, x5, x6);
                }
            }
            break;
        case 0x11:
            INST_NAME("LOCK ADC Ed, Gd");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                emit_adc32(dyn, ninst, rex, ed, gd, x3, x4, x5, x6);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                MARKLOCK;
                LRxw(x1, wback, 1, 1);
                ADDxw(x3, x1, gd);
                ANDI(x4, xFlags, 1 << F_CF);
                ADDxw(x3, x3, x4);
                SCxw(x4, x3, wback, 1, 1);
                BNEZ_MARKLOCK(x4);
                IFXORNAT (X_ALL | X_PEND) {
                    emit_adc32(dyn, ninst, rex, x1, gd, x3, x4, x5, x6);
                }
            }
            break;
        case 0x21:
            INST_NAME("LOCK AND Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                emit_and32(dyn, ninst, rex, ed, gd, x3, x4);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                AMOANDxw(x1, gd, wback, 1, 1);
                IFXORNAT (X_ALL | X_PEND)
                    emit_and32(dyn, ninst, rex, x1, gd, x3, x4);
            }
            break;
        case 0x29:
            INST_NAME("LOCK SUB Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                emit_sub32(dyn, ninst, rex, ed, gd, x3, x4, x5);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                SUBxw(x4, xZR, gd);
                AMOADDxw(x1, x4, wback, 1, 1);
                IFXORNAT (X_ALL | X_PEND)
                    emit_sub32(dyn, ninst, rex, x1, gd, x3, x4, x5);
            }
            break;
        case 0x66:
            return dynarec64_66F0(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);

        case 0x80:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 1: // OR
                    INST_NAME("LOCK OR Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    if (MODREG) {
                        GETEB(x1, 1);
                        u8 = F8;
                        emit_or8c(dyn, ninst, x1, u8, x2, x4, x5);
                        EBBACK(x5, 0);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x5, x1, &fixedaddress, rex, LOCK_LOCK, 0, 1);
                        u8 = F8;
                        ANDI(x2, wback, 3);
                        SLLI(x2, x2, 3);     // offset in bits
                        ANDI(x3, wback, ~3); // aligned addr
                        ADDI(x1, xZR, u8);
                        SLL(x1, x1, x2); // Ib << offset
                        AMOOR_W(x4, x1, x3, 1, 1);
                        IFXORNAT (X_ALL | X_PEND) {
                            SRL(x1, x4, x2);
                            ANDI(x1, x1, 0xFF);
                            emit_or8c(dyn, ninst, x1, u8, x2, x4, x5);
                        }
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
                        INST_NAME("LOCK ADD Ed, Id");
                    } else {
                        INST_NAME("LOCK ADD Ed, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    if (MODREG) {
                        if (opcode == 0x81)
                            i64 = F32S;
                        else
                            i64 = F8S;
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        emit_add32c(dyn, ninst, rex, ed, i64, x3, x4, x5, x6);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, (opcode == 0x81) ? 4 : 1);
                        if (opcode == 0x81)
                            i64 = F32S;
                        else
                            i64 = F8S;
                        MOV64xw(x7, i64);
                        ANDI(x1, wback, (1 << (rex.w + 2)) - 1);
                        BNEZ_MARK3(x1);
                        // Aligned
                        AMOADDxw(x1, x7, wback, 1, 1);
                        B_MARK_nocond;
                        MARK3;
                        // Unaligned
                        ANDI(x5, wback, -(1 << (rex.w + 2)));
                        MARKLOCK2;
                        LDxw(x1, wback, 0);
                        LRxw(x6, x5, 1, 1);
                        ADDxw(x4, x1, x7);
                        SCxw(x3, x6, x5, 1, 1);
                        BNEZ_MARKLOCK2(x3);
                        SDxw(x4, wback, 0);
                        MARK;
                        IFXORNAT (X_ALL | X_PEND)
                            emit_add32c(dyn, ninst, rex, x1, i64, x3, x4, x5, x6);
                    }
                    break;
                case 1: // OR
                    if (opcode == 0x81) {
                        INST_NAME("LOCK OR Ed, Id");
                    } else {
                        INST_NAME("LOCK OR Ed, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    if (MODREG) {
                        if (opcode == 0x81)
                            i64 = F32S;
                        else
                            i64 = F8S;
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        emit_or32c(dyn, ninst, rex, ed, i64, x3, x4);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, (opcode == 0x81) ? 4 : 1);
                        if (opcode == 0x81)
                            i64 = F32S;
                        else
                            i64 = F8S;
                        MOV64xw(x4, i64);
                        AMOORxw(x1, x4, wback, 1, 1);
                        IFXORNAT (X_ALL | X_PEND)
                            emit_or32c(dyn, ninst, rex, x1, i64, x3, x4);
                    }
                    break;
                case 4: // AND
                    if (opcode == 0x81) {
                        INST_NAME("LOCK AND Ed, Id");
                    } else {
                        INST_NAME("LOCK AND Ed, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    if (MODREG) {
                        if (opcode == 0x81)
                            i64 = F32S;
                        else
                            i64 = F8S;
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        emit_and32c(dyn, ninst, rex, ed, i64, x3, x4);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, (opcode == 0x81) ? 4 : 1);
                        if (opcode == 0x81)
                            i64 = F32S;
                        else
                            i64 = F8S;
                        MOV64xw(x7, i64);
                        AMOANDxw(x1, x7, wback, 1, 1);
                        IFXORNAT (X_ALL | X_PEND)
                            emit_and32c(dyn, ninst, rex, x1, i64, x3, x4);
                    }
                    break;
                case 5: // SUB
                    if (opcode == 0x81) {
                        INST_NAME("LOCK SUB Ed, Id");
                    } else {
                        INST_NAME("LOCK SUB Ed, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    if (MODREG) {
                        if (opcode == 0x81)
                            i64 = F32S;
                        else
                            i64 = F8S;
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        emit_sub32c(dyn, ninst, rex, ed, i64, x3, x4, x5, x6);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, (opcode == 0x81) ? 4 : 1);
                        if (opcode == 0x81)
                            i64 = F32S;
                        else
                            i64 = F8S;
                        MOV64xw(x7, i64);
                        ANDI(x1, wback, (1 << (rex.w + 2)) - 1);
                        BNEZ_MARK3(x1);
                        // Aligned
                        SUBxw(x4, xZR, x7);
                        AMOADDxw(x1, x4, wback, 1, 1);
                        B_MARK_nocond;
                        MARK3;
                        // Unaligned
                        ANDI(x5, wback, -(1 << (rex.w + 2)));
                        MARKLOCK2;
                        LDxw(x1, wback, 0);
                        LRxw(x6, x5, 1, 1);
                        SUBxw(x4, x1, x7);
                        SCxw(x3, x6, x5, 1, 1);
                        BNEZ_MARKLOCK2(x3);
                        SDxw(x4, wback, 0);
                        MARK;
                        IFXORNAT (X_ALL | X_PEND)
                            emit_sub32c(dyn, ninst, rex, x1, i64, x3, x4, x5, x6);
                    }
                    break;
                case 6: // XOR
                    if (opcode == 0x81) {
                        INST_NAME("LOCK XOR Ed, Id");
                    } else {
                        INST_NAME("LOCK XOR Ed, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    if (MODREG) {
                        if (opcode == 0x81)
                            i64 = F32S;
                        else
                            i64 = F8S;
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        emit_xor32c(dyn, ninst, rex, ed, i64, x3, x4);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, (opcode == 0x81) ? 4 : 1);
                        if (opcode == 0x81)
                            i64 = F32S;
                        else
                            i64 = F8S;
                        MOV64xw(x7, i64);
                        AMOXORxw(x1, x7, wback, 1, 1);
                        IFXORNAT (X_ALL | X_PEND)
                            emit_xor32c(dyn, ninst, rex, x1, i64, x3, x4);
                    }
                    break;
                default: DEFAULT;
            }
            break;
        case 0x87:
            INST_NAME("LOCK XCHG Ed, Gd");
            nextop = F8;
            if (MODREG) {
                GETGD;
                GETED(0);
                MV(x5, gd);
                MV(gd, ed);
                MV(ed, x5);
            } else {
                GETGD;
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                ANDI(x1, wback, (1 << (rex.w + 2)) - 1);
                BNEZ_MARK3(x1);
                // Aligned
                AMOSWAPxw(gd, gd, wback, 1, 1);
                if (!rex.w) ZEROUP(gd);
                B_NEXT_nocond;
                MARK3;
                // Unaligned
                ANDI(x5, wback, -(1 << (rex.w + 2)));
                MARKLOCK;
                LDxw(x1, wback, 0);
                LRxw(x3, x5, 1, 1);
                SCxw(x4, x3, x5, 1, 1);
                BNEZ_MARKLOCK(x4);
                SDxw(gd, wback, 0);
                MVxw(gd, x1);
            }
            break;
        case 0xF6:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 2:
                    INST_NAME("LOCK NOT Eb");
                    if (MODREG) {
                        GETEB(x1, 0);
                        NOT(x1, x1);
                        EBBACK(x5, 1);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                        ANDI(x3, wback, 3);
                        ANDI(x5, wback, ~3);
                        MOV32w(x4, 0xFF);
                        SLLIW(x3, x3, 3);
                        SLLW(x4, x4, x3); // mask
                        AMOXOR_W(xZR, x4, x5, 1, 1);
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xFF:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0: // INC Ed
                    INST_NAME("LOCK INC Ed");
                    SETFLAGS(X_ALL & ~X_CF, SF_SUBSET_PENDING, NAT_FLAGS_FUSION);
                    if (MODREG) {
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        emit_inc32(dyn, ninst, rex, ed, x3, x4, x5, x6);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                        ANDI(x1, wback, (1 << (rex.w + 2)) - 1);
                        BNEZ_MARK3(x1);
                        // Aligned
                        ADDIxw(x4, xZR, 1); 
                        AMOADDxw(x1, x4, wback, 1, 1);
                        B_MARK_nocond;
                        MARK3;
                        // Unaligned
                        ANDI(x5, wback, -(1 << (rex.w + 2)));
                        MARKLOCK2;
                        LDxw(x1, wback, 0);
                        LRxw(x6, x5, 1, 1);
                        ADDIxw(x4, x1, 1);
                        SCxw(x3, x6, x5, 1, 1);
                        BNEZ_MARKLOCK2(x3);
                        SDxw(x4, wback, 0);
                        MARK;
                        IFXORNAT (X_ALL | X_PEND)
                            emit_inc32(dyn, ninst, rex, x1, x3, x4, x5, x6);
                    }
                    break;
                case 1: // DEC Ed
                    INST_NAME("LOCK DEC Ed");
                    SETFLAGS(X_ALL & ~X_CF, SF_SUBSET_PENDING, NAT_FLAGS_FUSION);
                    if (MODREG) {
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        emit_dec32(dyn, ninst, rex, ed, x3, x4, x5, x6);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                        ANDI(x1, wback, (1 << (rex.w + 2)) - 1);
                        BNEZ_MARK3(x1);
                        // Aligned
                        ADDIxw(x4, xZR, -1); 
                        AMOADDxw(x1, x4, wback, 1, 1);
                        B_MARK_nocond;
                        MARK3;
                        // Unaligned
                        ANDI(x5, wback, -(1 << (rex.w + 2)));
                        MARKLOCK2;
                        LDxw(x1, wback, 0);
                        LRxw(x6, x5, 1, 1);
                        ADDIxw(x4, x1, -1);
                        SCxw(x3, x6, x5, 1, 1);
                        BNEZ_MARKLOCK2(x3);
                        SDxw(x4, wback, 0);
                        MARK;
                        IFXORNAT (X_ALL | X_PEND)
                            emit_dec32(dyn, ninst, rex, x1, x3, x4, x5, x6);
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
