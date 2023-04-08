#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
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
    // REX prefix before the F0 are ignored
    rex.rex = 0;
    while(opcode>=0x40 && opcode<=0x4f) {
        rex.rex = opcode;
        opcode = F8;
    }

    // TODO: Take care of unligned memory access for all the LOCK ones.
    // https://github.com/ptitSeb/box64/pull/604
    switch(opcode) {
        case 0x0F:
            nextop = F8;
            switch(nextop) {
                case 0xB1:
                    switch (rep) {
                        case 0:
                            INST_NAME("LOCK CMPXCHG Ed, Gd");
                            SETFLAGS(X_ALL, SF_SET_PENDING);
                            nextop = F8;
                            GETGD;
                            SMDMB();
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
                                B_NEXT_nocond;
                            } else {
                                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                                MARKLOCK;
                                LRxw(x1, wback, 1, 1);
                                SUB(x3, x1, xRAX);
                                BNE_MARK(x3, xZR);
                                // EAX == Ed
                                SCxw(x4, gd, wback, 1, 1);
                                BNEZ_MARKLOCK(x4);
                                MARK;
                                UFLAG_IF {emit_cmp32(dyn, ninst, rex, xRAX, x1, x3, x4, x5, x6);}
                                MVxw(xRAX, x1);
                            }
                            SMDMB();
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
                default:
                    DEFAULT;
            }
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
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, (opcode==0x81)?4:1);
                        if(opcode==0x81) i64 = F32S; else i64 = F8S;
                        MARKLOCK;
                        LRxw(x1, wback, 1, 1);
                        if(i64>=-2048 && i64<2048)
                            ADDIxw(x4, x1, i64);
                        else {
                            MOV64xw(x4, i64);
                            ADDxw(x4, x1, x4);
                        }
                        SCxw(x3, x4, wback, 1, 1);
                        BNEZ_MARKLOCK(x3);
                        IFX(X_ALL|X_PEND)
                            emit_add32c(dyn, ninst, rex, x1, i64, x3, x4, x5, x6);
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
                        emit_sub32(dyn, ninst, rex, ed, i64, x3, x4, x5);
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
