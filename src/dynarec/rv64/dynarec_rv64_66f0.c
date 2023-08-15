#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "dynarec.h"
#include "emu/x64emu_private.h"
#include "emu/x64run_private.h"
#include "rv64_emitter.h"
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


uintptr_t dynarec64_66F0(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
{
    (void)ip; (void)rep; (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop;
    uint8_t gd, ed, u8;
    uint8_t wback, wb1, wb2, gb1, gb2;
    int32_t i32;
    int64_t i64, j64;
    int64_t fixedaddress;
    int unscaled;
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

    switch(opcode) {
        case 0x11:
            INST_NAME("LOCK ADC Ew, Gw");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x1);
            SMDMB();
            if(MODREG) {
                ed = xRAX+(nextop&7)+(rex.b<<3);
                MOV32w(x6, 0xFFFF);
                AND(x2, ed, x6);
                emit_adc16(dyn, ninst, x1, x2, x3, x4, x5);
                if (rv64_zbb) {
                    ANDN(ed, ed, x6);
                } else {
                    NOT(x6, x6);
                    AND(ed, ed, x6);
                }
                OR(ed, ed, x2);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x3, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                ANDI(x3, wback, 0b10);
                MOV32w(x4, 0xffff); // x4 = mask
                BNEZ_MARK(x3);
                // lower 16bits
                MARKLOCK;
                LR_W(x5, wback, 1, 1);
                AND(x6, x5, x4);        // x6 = Ed.h[0]
                SRLIW(x5, x5, 16);
                SLLIW(x5, x5, 16);      // x5 = clear Ed.h[0]
                ADDW(x2, x1, x6);       // x2 = Gw + Ew
                ANDI(x9, xFlags, 1 << F_CF);
                ADDW(x2, x2, x9);       // x2 = adc
                AND(x2, x2, x4);
                OR(x5, x5, x2);
                SC_W(x2, x5, wback, 1, 1);
                BNEZ_MARKLOCK(x2);
                B_MARK3_nocond;
                MARK;
                // upper 16bits
                SLLIW(x3, x4, 16);     // x3 = mask
                SUBI(wback, wback, 2); // aligning address
                SLLI(x1, x1, 16);      // x1 = extended Gw
                MARK2;
                LR_W(x6, wback, 1, 1); // x6 = Ed
                AND(x5, x6, x3);       // x5 = Ed.h[1] << 16
                ADDW(x5,x5, x1);
                ANDI(x9, xFlags, 1 << F_CF);
                SLLIW(x9, x9, 16);
                ADDW(x5, x5, x9);      // x5 = adc << 16
                AND(x9, x6, x4);       // x9 = Ed.h[0]
                OR(x5, x5, x9);
                SC_W(x9, x5, wback, 1, 1);
                BNEZ_MARK2(x9);
                IFX(X_ALL|X_PEND) {
                    SRLIW(x6, x6, 16);
                    SRLIW(x1, x1, 16);
                }
                MARK3;
                IFX(X_ALL|X_PEND) {
                    emit_adc16(dyn, ninst, x1, x6, x3, x4, x5);
                }
            }
            SMDMB();
            break;
        case 0x81:
        case 0x83:
            nextop = F8;
            SMDMB();
            switch((nextop>>3)&7) {
                case 0: //ADD
                    if(opcode==0x81) {
                        INST_NAME("LOCK ADD Ew, Iw");
                    } else {
                        INST_NAME("LOCK ADD Ew, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    if(MODREG) {
                        if(opcode==0x81) i32 = F16S; else i32 = F8S;
                        ed = xRAX+(nextop&7)+(rex.b<<3);
                        MOV32w(x5, i32);
                        ZEXTH(x6, ed);
                        emit_add16(dyn, ninst, x6, x5, x3, x4, x2);
                        SRLI(ed, ed, 16);
                        SLLI(ed, ed, 16);
                        OR(ed, ed, x6);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, (opcode==0x81)?2:1);
                        if(opcode==0x81) i32 = F16S; else i32 = F8S;
                        MOV32w(x5, i32);

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
                        IFX(X_ALL|X_PEND) {
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
                        IFX(X_ALL|X_PEND) {
                            emit_add16(dyn, ninst, x1, x5, x3, x4, x6);
                        }
                    }
                    break;
                default:
                    DEFAULT;
            }
            SMDMB();
            break;

        default:
            DEFAULT;
    }

    return addr;
}