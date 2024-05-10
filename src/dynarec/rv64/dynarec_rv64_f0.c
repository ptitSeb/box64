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

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_helper.h"
#include "dynarec_rv64_functions.h"


uintptr_t dynarec64_F0(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
{
    (void)ip; (void)rep; (void)need_epilog;

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

    while((opcode==0xF2) || (opcode==0xF3)) {
        rep = opcode-0xF1;
        opcode = F8;
    }

    GETREX();

    // TODO: Add support for unligned memory access for all the LOCK ones.
    // TODO: Add support for BOX4_DYNAREC_ALIGNED_ATOMICS.

    switch(opcode) {
        case 0x01:
            INST_NAME("LOCK ADD Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            SMDMB();
            if((nextop&0xC0)==0xC0) {
                ed = xRAX+(nextop&7)+(rex.b<<3);
                emit_add32(dyn, ninst, rex, ed, gd, x3, x4, x5);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                MARKLOCK;
                LRxw(x1, wback, 1, 1);
                ADDxw(x4, x1, gd);
                SCxw(x3, x4, wback, 1, 1);
                BNEZ_MARKLOCK(x3);
                IFX(X_ALL|X_PEND) {
                    emit_add32(dyn, ninst, rex, x1, gd, x3, x4, x5);
                }
            }
            SMDMB();
            break;
        case 0x09:
            INST_NAME("LOCK OR Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            SMDMB();
            if (MODREG) {
                ed = xRAX+(nextop&7)+(rex.b<<3);
                emit_or32(dyn, ninst, rex, ed, gd, x3, x4);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                MARKLOCK;
                LRxw(x1, wback, 1, 1);
                OR(x4, x1, gd);
                SCxw(x3, x4, wback, 1, 1);
                BNEZ_MARKLOCK(x3);
                IFX(X_ALL|X_PEND)
                    emit_or32(dyn, ninst, rex, x1, gd, x3, x4);
            }
            SMDMB();
            break;

        case 0x0F:
            nextop = F8;
            switch(nextop) {
                case 0xB0:
                    switch(rep) {
                        case 0:
                            INST_NAME("LOCK CMPXCHG Eb, Gb");
                            SETFLAGS(X_ALL, SF_SET_PENDING);
                            nextop = F8;
                            ANDI(x6, xRAX, 0xff); // AL
                            SMDMB();
                            if(MODREG) {
                                if(rex.rex) {
                                    wback = xRAX+(nextop&7)+(rex.b<<3);
                                    wb2 = 0;
                                } else {
                                    wback = (nextop&7);
                                    wb2 = (wback>>2)*8;
                                    wback = xRAX+(wback&3);
                                }
                                if (wb2) {
                                    MV(x2, wback);
                                    SRLI(x2, x2, wb2);
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
                                if (wb2) {
                                    MV(wback, x2);
                                    SRLI(wback, wback, wb2);
                                    ANDI(wback, wback, 0xff);
                                } else {
                                    ANDI(wback, x2, 0xff);
                                }
                                GETGB(x1);
                                MV(ed, gd);
                                MARK2;
                                ANDI(xRAX, xRAX, ~0xff);
                                OR(xRAX, xRAX, x2);
                                B_NEXT_nocond;
                            } else {
                                // this one is tricky, and did some repetitive work.
                                // mostly because we only got 6 scratch registers,
                                // and has so much to do.
                                if(rex.rex) {
                                    gb1 = xRAX+((nextop&0x38)>>3)+(rex.r<<3);
                                    gb2 = 0;
                                } else {
                                    gd = (nextop&0x38)>>3;
                                    gb2 = ((gd&4)>>2);
                                    gb1 = xRAX+(gd&3);
                                }
                                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x2, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                                ANDI(x5, wback, 0b11);
                                SLLI(x5, x5, 3);        // shamt
                                MARKLOCK;
                                ANDI(x2, wback, ~0b11); // align to 32bit
                                LWU(x1, x2, 0);
                                LR_W(x4, x2, 1, 1);
                                SRL(x4, x4, x5);
                                ANDI(x4, x4, 0xff);
                                BNE_MARK(x6, x4); // compare AL with m8
                                // AL == m8, r8 is loaded into m8
                                ADDI(x2, xZR, 0xff);
                                SLL(x2, x2, x5);
                                NOT(x2, x2);
                                AND(x2, x1, x2);
                                if (gb2) {
                                    MV(x1, gb1);
                                    SRLI(x1, x1, 8);
                                    ANDI(x1, x1, 0xff);
                                } else {
                                    ANDI(x1, gb1, 0xff);
                                }
                                SLL(x1, x1, x5);
                                OR(x1, x1, x2);
                                ANDI(x2, wback, ~0b11); // align to 32bit again
                                SC_W(x9, x1, x2, 1, 1);
                                BNEZ_MARKLOCK(x9);
                                // done
                                MARK;
                                UFLAG_IF {emit_cmp8(dyn, ninst, x6, x4, x1, x2, x3, x5);}
                                // load m8 into AL
                                ANDI(xRAX, xRAX, ~0xff);
                                OR(xRAX, xRAX, x4);
                            }
                            SMDMB();
                            break;
                        default:
                            DEFAULT;
                    }
                    break;
                case 0xB1:
                    switch (rep) {
                        case 0:
                            INST_NAME("LOCK CMPXCHG Ed, Gd");
                            SETFLAGS(X_ALL, SF_SET_PENDING);
                            nextop = F8;
                            GETGD;
                            if (MODREG) {
                                ed = xRAX+(nextop&7)+(rex.b<<3);
                                wback = 0;
                                UFLAG_IF {emit_cmp32(dyn, ninst, rex, xRAX, ed, x3, x4, x5, x6);}
                                MV(x1, ed); // save value
                                SUB(x2, x1, xRAX);
                                BNE_MARK2(x2, xZR);
                                MV(ed, gd);
                                MARK2;
                                MVxw(xRAX, x1);
                            } else {
                                SMDMB();
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
                                MARK2; // Use MARK2 as a "MARKLOCK" since we're running out of marks.
                                LDxw(x6, wback, 0);
                                LRxw(x1, x5, 1, 1);
                                SUBxw(x3, x6, xRAX);
                                BNEZ_MARK(x3);
                                // EAX == Ed
                                SCxw(x4, x1, x5, 1, 1);
                                BNEZ_MARK2(x4);
                                SDxw(gd, wback, 0);
                                MARK;
                                UFLAG_IF {emit_cmp32(dyn, ninst, rex, xRAX, x1, x3, x4, x5, x6);}
                                MVxw(xRAX, x1);
                                SMDMB();
                            }
                            break;
                        default:
                            DEFAULT;
                    }
                    break;
                case 0xC1:
                    switch(rep) {
                        case 0:
                            INST_NAME("LOCK XADD Gd, Ed");
                            SETFLAGS(X_ALL, SF_SET_PENDING);
                            nextop = F8;
                            GETGD;
                            SMDMB();
                            if(MODREG) {
                                ed = xRAX+(nextop&7)+(rex.b<<3);
                                MVxw(x1, ed);
                                MVxw(ed, gd);
                                MVxw(gd, x1);
                                emit_add32(dyn, ninst, rex, ed, gd, x3, x4, x5);
                            } else {
                                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                                MARKLOCK;
                                LRxw(x1, wback, 1, 1);
                                ADDxw(x4, x1, gd);
                                SCxw(x3, x4, wback, 1, 1);
                                BNEZ_MARKLOCK(x3);
                                IFX(X_ALL|X_PEND) {
                                    MVxw(x2, x1);
                                    emit_add32(dyn, ninst, rex, x2, gd, x3, x4, x5);
                                }
                                MVxw(gd, x1);
                            }
                            SMDMB();
                            break;
                        default:
                            DEFAULT;
                    }
                    break;

                case 0xC7:
                    switch(rep) {
                        case 0:
                            if (rex.w) {
                                INST_NAME("LOCK CMPXCHG16B Gq, Eq");
                            } else {
                                INST_NAME("LOCK CMPXCHG8B Gq, Eq");
                            }
                            SETFLAGS(X_ZF, SF_SUBSET);
                            nextop = F8;
                            addr = geted(dyn, addr, ninst, nextop, &wback, x1, x2, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                            ANDI(xFlags, xFlags, ~(1<<F_ZF));
                            if (rex.w) {
                                // there is no atomic move on 16bytes, so implement it with mutex
                                LD(x9, xEmu, offsetof(x64emu_t, context));
                                ADDI(x9, x9, offsetof(box64context_t, mutex_16b));
                                ADDI(x4, xZR, 1);
                                MARKLOCK;
                                AMOSWAP_W(x4, x4, x9, 1, 1);
                                // x4 == 1 if locked
                                BNEZ_MARKLOCK(x4);

                                SMDMB();
                                LD(x2, wback, 0);
                                LD(x3, wback, 8);
                                BNE_MARK(x2, xRAX);
                                BNE_MARK(x3, xRDX);
                                SD(xRBX, wback, 0);
                                SD(xRCX, wback, 8);
                                ORI(xFlags, xFlags, 1<<F_ZF);
                                B_MARK3_nocond;
                                MARK;
                                MV(xRAX, x2);
                                MV(xRDX, x3);
                                MARK3;
                                SMDMB();

                                // unlock
                                AMOSWAP_W(xZR, xZR, x9, 1, 1);
                            } else {
                                SMDMB();
                                AND(x3, xRAX, xMASK);
                                SLLI(x2, xRDX, 32);
                                OR(x3, x3, x2);
                                AND(x4, xRBX, xMASK);
                                SLLI(x2, xRCX, 32);
                                OR(x4, x4, x2);
                                MARKLOCK;
                                LR_D(x2, wback, 1, 1);
                                BNE_MARK(x2, x3); // EDX_EAX != Ed
                                SC_D(x5, x4, wback, 1, 1);
                                BNEZ_MARKLOCK(x5);
                                ORI(xFlags, xFlags, 1<<F_ZF);
                                B_MARK3_nocond;
                                MARK;
                                SLLI(xRDX, x2, 32);
                                AND(xRAX, x2, xMASK);
                                MARK3;
                                SMDMB();
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
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGB(x2);
            SMDMB();
            if((nextop&0xC0)==0xC0) {
                if(rex.rex) {
                    wback = xRAX + (nextop&7) + (rex.b<<3);
                    wb2 = 0;
                } else {
                    wback = (nextop&7);
                    wb2 = (wback>>2);
                    wback = xRAX+(wback&3);
                }
                SRLIW(x1, wback, wb2*8);
                ANDI(x1, x1, 0xFF);
                emit_adc8(dyn, ninst, x1, x2, x3, x4, x5);
                SLLI(x1, x1, wb2*8);
                MOV_U12(x3, 0xFF);
                SLLI(x3, x3, wb2*8);
                NOT(x3, x3);
                AND(wback, wback, x3);
                OR(wback, wback, x1);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x1, x3, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                ANDI(x3, wback, 0b11);
                BNEZ_MARK(x3);
                MARKLOCK;
                LR_W(x5, wback, 1, 1);
                ANDI(x4, x5, 0xff); // x4 = Ed.b[0]
                ANDI(x5, x5, ~0xff); // x5 = clear Ed.b[0]
                ADDW(x6, x4, x2);
                ANDI(x9, xFlags, 1 << F_CF);
                ADDW(x6, x6, x9);   // x6 = adc
                ANDI(x6, x6, 0xff);
                OR(x5, x5, x6);
                SC_W(x9, x5, wback, 1, 1);
                BNEZ_MARKLOCK(x9);
                B_MARK3_nocond;
                MARK;
                SLLI(x3, x3, 3);
                MOV_U12(x4, 0xff);
                ANDI(wback, wback, ~3); // aligning address
                SLLI(x4, x4, x3);       // x4 = byte mask
                NOT(x5, x4);            // x5 = ~mask
                SLL(x2, x2, x3);        // x2 = extented Gb
                MARK2;
                LR_W(x6, wback, 1, 1);  // x6 = Ed
                AND(x9, x6, x4);        // x9 = extended Ed.b[dest]
                AND(x6, x6, x5);        // x6 = clear Ed.b[dest]
                ADDW(x5, x9, x2);
                ANDI(x4, xFlags, 1 << F_CF);
                SLL(x4, x4, x3);        // extented
                ADDW(x5, x5, x4);       // x5 = adc
                OR(x5, x5, x6);
                SC_W(x4, x5, wback, 1, 1);
                BNEZ_MARK2(x4);
                IFX(X_ALL|X_PEND) {
                    SRLI(x2, x2, x3);  // Gb
                    SRLI(x4, x9, x3);  // Eb
                }
                MARK3;
                IFX(X_ALL|X_PEND) {
                    emit_adc8(dyn, ninst, x4, x2, x3, x5, x6);
                }
            }
            SMDMB();
            break;
        case 0x11:
            INST_NAME("LOCK ADC Ed, Gd");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            SMDMB();
            if(MODREG) {
                ed = xRAX+(nextop&7)+(rex.b<<3);
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
                IFX(X_ALL|X_PEND) {
                    emit_adc32(dyn, ninst, rex, x1, gd, x3, x4, x5, x6);
                }
            }
            SMDMB();
            break;
        case 0x21:
            INST_NAME("LOCK AND Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            SMDMB();
            if(MODREG) {
                ed = xRAX+(nextop&7)+(rex.b<<3);
                emit_and32(dyn, ninst, rex, ed, gd, x3, x4);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                MARKLOCK;
                LRxw(x1, wback, 1, 1);
                AND(x4, x1, gd);
                SCxw(x3, x4, wback, 1, 1);
                BNEZ_MARKLOCK(x3);
                IFX(X_ALL|X_PEND)
                    emit_and32(dyn, ninst, rex, x1, gd, x3, x4);
            }
            SMDMB();
            break;
        case 0x29:
            INST_NAME("LOCK SUB Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            SMDMB();
            if(MODREG) {
                ed = xRAX+(nextop&7)+(rex.b<<3);
                emit_sub32(dyn, ninst, rex, ed, gd, x3, x4, x5);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                MARKLOCK;
                LRxw(x1, wback, 1, 1);
                SUB(x4, x1, gd);
                SCxw(x3, x4, wback, 1, 1);
                BNEZ_MARKLOCK(x3);
                IFX(X_ALL|X_PEND)
                    emit_sub32(dyn, ninst, rex, x1, gd, x3, x4, x5);
            }
            SMDMB();
            break;
        case 0x66:
            return dynarec64_66F0(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);

        case 0x80:
            nextop = F8;
            SMDMB();
            switch((nextop>>3)&7) {
                case 1: // OR
                    INST_NAME("LOCK OR Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    if(MODREG) {
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
                        SLL(x1, x1, x2);     // Ib << offset
                        MARKLOCK;
                        LR_W(x4, x3, 1, 1);
                        OR(x6, x4, x1);
                        SC_W(x6, x6, x3, 1, 1);
                        BNEZ_MARKLOCK(x6);
                        IFX(X_ALL|X_PEND) {
                            SRL(x1, x4, x2);
                            ANDI(x1, x1, 0xFF);
                            emit_or8c(dyn, ninst, x1, u8, x2, x4, x5);
                        }
                    }
                    break;
                default:
                    DEFAULT;
            }
            SMDMB();
            break;
        case 0x81:
        case 0x83:
            nextop = F8;
            SMDMB();
            switch((nextop>>3)&7) {
                case 0: // ADD
                    if(opcode==0x81) {
                        INST_NAME("LOCK ADD Ed, Id");
                    } else {
                        INST_NAME("LOCK ADD Ed, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    if(MODREG) {
                        if(opcode==0x81) i64 = F32S; else i64 = F8S;
                        ed = xRAX+(nextop&7)+(rex.b<<3);
                        emit_add32c(dyn, ninst, rex, ed, i64, x3, x4, x5, x6);
                    } else {
                        SMDMB();
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, (opcode==0x81)?4:1);
                        if(opcode==0x81) i64 = F32S; else i64 = F8S;
                        if (i64 < -2048 || i64 >= 2048)
                            MOV64xw(x4, i64);
                        ANDI(x1, wback, (1 << (rex.w + 2)) - 1);
                        BNEZ_MARK3(x1);
                        // Aligned
                        MARKLOCK;
                        LRxw(x1, wback, 1, 1);
                        if (i64 >= -2048 && i64 < 2048)
                            ADDIxw(x4, x1, i64);
                        else
                            ADDxw(x4, x1, x4);
                        SCxw(x3, x4, wback, 1, 1);
                        BNEZ_MARKLOCK(x3);
                        B_MARK_nocond;
                        MARK3;
                        // Unaligned
                        ANDI(x5, wback, -(1 << (rex.w + 2)));
                        MARK2; // Use MARK2 as a "MARKLOCK" since we're running out of marks.
                        LDxw(x6, wback, 0);
                        LRxw(x1, x5, 1, 1);
                        if (i64 >= -2048 && i64 < 2048)
                            ADDIxw(x4, x6, i64);
                        else
                            ADDxw(x4, x6, x4);
                        SCxw(x3, x1, x5, 1, 1);
                        BNEZ_MARK2(x3);
                        SDxw(x4, wback, 0);
                        MARK;
                        IFX (X_ALL | X_PEND)
                            emit_add32c(dyn, ninst, rex, x1, i64, x3, x4, x5, x6);
                        SMDMB();
                    }
                    break;
                case 1: // OR
                    if(opcode==0x81) {
                        INST_NAME("LOCK OR Ed, Id");
                    } else {
                        INST_NAME("LOCK OR Ed, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    if(MODREG) {
                        if(opcode==0x81) i64 = F32S; else i64 = F8S;
                        ed = xRAX+(nextop&7)+(rex.b<<3);
                        emit_or32c(dyn, ninst, rex, ed, i64, x3, x4);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, (opcode==0x81)?4:1);
                        if(opcode==0x81) i64 = F32S; else i64 = F8S;
                        MARKLOCK;
                        LRxw(x1, wback, 1, 1);
                        if (i64>=-2048 && i64<2048) {
                            ORI(x4, x1, i64);
                        } else {
                            MOV64xw(x4, i64);
                            OR(x4, x1, x4);
                        }
                        if (!rex.w) ZEROUP(x4);
                        SCxw(x3, x4, wback, 1, 1);
                        BNEZ_MARKLOCK(x3);
                        IFX(X_ALL|X_PEND)
                            emit_or32c(dyn, ninst, rex, x1, i64, x3, x4);
                    }
                    break;
                case 4: // AND
                    if(opcode==0x81) {
                        INST_NAME("LOCK AND Ed, Id");
                    } else {
                        INST_NAME("LOCK AND Ed, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    if(MODREG) {
                        if(opcode==0x81) i64 = F32S; else i64 = F8S;
                        ed = xRAX+(nextop&7)+(rex.b<<3);
                        emit_and32c(dyn, ninst, rex, ed, i64, x3, x4);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, (opcode==0x81)?4:1);
                        if(opcode==0x81) i64 = F32S; else i64 = F8S;
                        MARKLOCK;
                        LRxw(x1, wback, 1, 1);
                        if (i64>=-2048 && i64<2048) {
                            ANDI(x4, x1, i64);
                        } else {
                            MOV64xw(x4, i64);
                            AND(x4, x1, x4);
                        }
                        if (!rex.w) ZEROUP(x4);
                        SCxw(x3, x4, wback, 1, 1);
                        BNEZ_MARKLOCK(x3);
                        IFX(X_ALL|X_PEND)
                            emit_and32c(dyn, ninst, rex, x1, i64, x3, x4);
                    }
                    break;
                case 5: // SUB
                    if(opcode==0x81) {
                        INST_NAME("LOCK SUB Ed, Id");
                    } else {
                        INST_NAME("LOCK SUB Ed, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    if(MODREG) {
                        if(opcode==0x81) i64 = F32S; else i64 = F8S;
                        ed = xRAX+(nextop&7)+(rex.b<<3);
                        emit_sub32c(dyn, ninst, rex, ed, i64, x3, x4, x5, x6);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, (opcode==0x81)?4:1);
                        if(opcode==0x81) i64 = F32S; else i64 = F8S;
                        MARKLOCK;
                        LRxw(x1, wback, 1, 1);
                        if (i64>-2048 && i64<=2048) {
                            ADDIxw(x4, x1, -i64);
                        } else {
                            MOV64xw(x4, i64);
                            SUBxw(x4, x1, x4);
                        }
                        SCxw(x3, x4, wback, 1, 1);
                        BNEZ_MARKLOCK(x3);
                        IFX(X_ALL|X_PEND)
                            emit_sub32c(dyn, ninst, rex, x1, i64, x3, x4, x5, x6);
                    }
                    break;
                case 6: // XOR
                    if (opcode == 0x81) {
                        INST_NAME("LOCK XOR Ed, Id");
                    } else {
                        INST_NAME("LOCK XOR Ed, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    if (MODREG) {
                        if (opcode == 0x81)
                            i64 = F32S;
                        else
                            i64 = F8S;
                        ed = xRAX + (nextop & 7) + (rex.b << 3);
                        emit_xor32c(dyn, ninst, rex, ed, i64, x3, x4);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, (opcode == 0x81) ? 4 : 1);
                        if (opcode == 0x81)
                            i64 = F32S;
                        else
                            i64 = F8S;
                        MARKLOCK;
                        LRxw(x1, wback, 1, 1);
                        if (i64 >= -2048 && i64 <= 2047) {
                            XORI(x4, x1, i64);
                        } else {
                            MOV64xw(x4, i64);
                            XOR(x4, x1, x4);
                        }
                        SCxw(x3, x4, wback, 1, 1);
                        BNEZ_MARKLOCK(x3);
                        IFX(X_ALL | X_PEND)
                            emit_xor32c(dyn, ninst, rex, x1, i64, x3, x4);
                    }
                    break;
                default: DEFAULT;
            }
            SMDMB();
            break;
        case 0x87:
            INST_NAME("LOCK XCHG Ed, Gd");
            nextop = F8;
            if (MODREG) {
                GETGD;
                GETED(0);
                MV(x1, gd);
                MV(gd, ed);
                MV(ed, x1);
            } else {
                SMDMB();
                GETGD;
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                ANDI(x1, wback, (1 << (rex.w + 2)) - 1);
                BNEZ_MARK3(x1);
                // Aligned
                MARKLOCK;
                LRxw(x1, wback, 1, 1);
                SCxw(x4, gd, wback, 1, 1);
                BNEZ_MARKLOCK(x4);
                B_MARK_nocond;
                MARK3;
                // Unaligned
                ANDI(x5, wback, -(1 << (rex.w + 2)));
                MARK2; // Use MARK2 as a "MARKLOCK" since we're running out of marks.
                LDxw(x1, wback, 0);
                LRxw(x3, x5, 1, 1);
                SCxw(x4, x3, x5, 1, 1);
                BNEZ_MARK2(x4);
                SDxw(gd, wback, 0);
                MARK;
                MVxw(gd, x1);
                SMDMB();
            }
            break;
        case 0xFF:
            nextop = F8;
            switch((nextop>>3)&7)
            {
                case 0: // INC Ed
                    INST_NAME("LOCK INC Ed");
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET_PENDING);
                    SMDMB();
                    if(MODREG) {
                        ed = xRAX+(nextop&7)+(rex.b<<3);
                        emit_inc32(dyn, ninst, rex, ed, x3, x4, x5, x6);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                        MARKLOCK;
                        LRxw(x1, wback, 1, 1);
                        ADDIxw(x4, x1, 1);
                        SCxw(x3, x4, wback, 1, 1);
                        BNEZ_MARKLOCK(x3);
                        IFX(X_ALL|X_PEND)
                            emit_inc32(dyn, ninst, rex, x1, x3, x4, x5, x6);
                    }
                    break;
                case 1: // DEC Ed
                    INST_NAME("LOCK DEC Ed");
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET_PENDING);
                    SMDMB();
                    if(MODREG) {
                        ed = xRAX+(nextop&7)+(rex.b<<3);
                        emit_dec32(dyn, ninst, rex, ed, x3, x4, x5, x6);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                        MARKLOCK;
                        LRxw(x1, wback, 1, 1);
                        ADDIxw(x4, x1, -1);
                        SCxw(x3, x4, wback, 1, 1);
                        BNEZ_MARKLOCK(x3);
                        IFX(X_ALL|X_PEND)
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
