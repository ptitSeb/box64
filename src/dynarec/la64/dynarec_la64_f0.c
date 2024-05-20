#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

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
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"

#include "la64_printer.h"
#include "dynarec_la64_private.h"
#include "dynarec_la64_helper.h"
#include "dynarec_la64_functions.h"


uintptr_t dynarec64_F0(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
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

    switch (opcode) {
        case 0x01:
            INST_NAME("LOCK ADD Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            SMDMB();
            if ((nextop & 0xC0) == 0xC0) {
                ed = TO_LA64((nextop & 7) + (rex.b << 3));
                emit_add32(dyn, ninst, rex, ed, gd, x3, x4, x5);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                MARKLOCK;
                LLxw(x1, wback, 0);
                ADDxw(x4, x1, gd);
                SCxw(x4, wback, 0);
                BEQZ_MARKLOCK(x4);
                IFX (X_ALL | X_PEND) {
                    emit_add32(dyn, ninst, rex, x1, gd, x3, x4, x5);
                }
            }
            SMDMB();
            break;
        case 0x0F:
            nextop = F8;
            switch (nextop) {
                case 0xB1:
                    switch (rep) {
                        case 0:
                            INST_NAME("LOCK CMPXCHG Ed, Gd");
                            SETFLAGS(X_ALL, SF_SET_PENDING);
                            nextop = F8;
                            GETGD;
                            if (MODREG) {
                                ed = TO_LA64((nextop & 7) + (rex.b << 3));
                                wback = 0;
                                UFLAG_IF { emit_cmp32(dyn, ninst, rex, xRAX, ed, x3, x4, x5, x6); }
                                MV(x1, ed); // save value
                                SUB_D(x2, x1, xRAX);
                                BNE_MARK2(x2, xZR);
                                MV(ed, gd);
                                MARK2;
                                MVxw(xRAX, x1);
                            } else {
                                SMDMB();
                                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                                MARKLOCK;
                                MV(x4, gd);
                                LLxw(x1, wback, 0);
                                SUBxw(x3, x1, xRAX);
                                BNEZ_MARK(x3);
                                // EAX == Ed
                                SCxw(x4, wback, 0);
                                BEQZ_MARKLOCK(x4);
                                MARK;
                                UFLAG_IF { emit_cmp32(dyn, ninst, rex, xRAX, x1, x3, x4, x5, x6); }
                                MVxw(xRAX, x1);
                                SMDMB();
                            }
                            break;
                        default:
                            DEFAULT;
                    }
                    break;
                case 0xC1:
                    switch (rep) {
                        case 0:
                            INST_NAME("LOCK XADD Gd, Ed");
                            SETFLAGS(X_ALL, SF_SET_PENDING);
                            nextop = F8;
                            GETGD;
                            SMDMB();
                            if (MODREG) {
                                ed = TO_LA64((nextop & 7) + (rex.b << 3));
                                MVxw(x1, ed);
                                MVxw(ed, gd);
                                MVxw(gd, x1);
                                emit_add32(dyn, ninst, rex, ed, gd, x3, x4, x5);
                            } else {
                                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                                MARKLOCK;
                                LLxw(x1, wback, 0);
                                ADDxw(x4, x1, gd);
                                SCxw(x4, wback, 0);
                                BEQZ_MARKLOCK(x4);
                                IFX(X_ALL | X_PEND) {
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
        case 0x29:
            INST_NAME("LOCK SUB Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            SMDMB();
            if (MODREG) {
                ed = TO_LA64((nextop & 7) + (rex.b << 3));
                emit_sub32(dyn, ninst, rex, ed, gd, x3, x4, x5);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                MARKLOCK;
                LLxw(x1, wback, 0);
                SUB_D(x4, x1, gd);
                SCxw(x4, wback, 0);
                BEQZ_MARKLOCK(x4);
                IFX (X_ALL | X_PEND)
                    emit_sub32(dyn, ninst, rex, x1, gd, x3, x4, x5);
            }
            SMDMB();
            break;
        case 0x81:
        case 0x83:
            nextop = F8;
            SMDMB();
            switch ((nextop >> 3) & 7) {
                case 0: // ADD
                    if (opcode == 0x81) {
                        INST_NAME("LOCK ADD Ed, Id");
                    } else {
                        INST_NAME("LOCK ADD Ed, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    if (MODREG) {
                        if (opcode == 0x81)
                            i64 = F32S;
                        else
                            i64 = F8S;
                        ed = TO_LA64((nextop & 7) + (rex.b << 3));
                        emit_add32c(dyn, ninst, rex, ed, i64, x3, x4, x5, x6);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, (opcode == 0x81) ? 4 : 1);
                        if (opcode == 0x81)
                            i64 = F32S;
                        else
                            i64 = F8S;
                        if (i64 < -2048 || i64 >= 2048) {
                            MOV64xw(x3, i64);
                        }
                        MARKLOCK;
                        LLxw(x1, wback, 0);
                        if (i64 >= -2048 && i64 < 2048) {
                            ADDIxw(x4, x1, i64);
                        } else {
                            ADDxw(x4, x1, x3);
                        }
                        SCxw(x4, wback, 0);
                        BEQZ_MARKLOCK(x4);
                        IFX(X_ALL | X_PEND) {
                            emit_add32c(dyn, ninst, rex, x1, i64, x3, x4, x5, x6);
                        }
                        SMDMB();
                    }
                    break;
                case 5: // SUB
                    if (opcode == 0x81) {
                        INST_NAME("LOCK SUB Ed, Id");
                    } else {
                        INST_NAME("LOCK SUB Ed, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    if (MODREG) {
                        if (opcode == 0x81)
                            i64 = F32S;
                        else
                            i64 = F8S;
                        ed = xRAX + (nextop & 7) + (rex.b << 3);
                        emit_sub32c(dyn, ninst, rex, ed, i64, x3, x4, x5, x6);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, (opcode == 0x81) ? 4 : 1);
                        if (opcode == 0x81)
                            i64 = F32S;
                        else
                            i64 = F8S;
                        if (i64 <= -2048 || i64 > 2048) {
                            MOV64xw(x3, i64);
                        }
                        MARKLOCK;
                        LLxw(x1, wback, 0);
                        if (i64 > -2048 && i64 <= 2048) {
                            ADDIxw(x4, x1, -i64);
                        } else {
                            SUBxw(x4, x1, x3);
                        }
                        SCxw(x4, wback, 0);
                        BEQZ_MARKLOCK(x4);
                        IFX (X_ALL | X_PEND)
                            emit_sub32c(dyn, ninst, rex, x1, i64, x3, x4, x5, x6);
                    }
                    break;
                default:
                    DEFAULT;
            }
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
                MARKLOCK;
                LLxw(x1, wback, 0);
                MV(x4, gd);
                SCxw(x4, wback, 0);
                BEQZ_MARKLOCK(x4);
                MVxw(gd, x1);
                SMDMB();
            }
            break;
        default:
            DEFAULT;
    }
    return addr;
}
