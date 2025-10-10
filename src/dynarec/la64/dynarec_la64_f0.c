#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "la64_emitter.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"

#include "la64_printer.h"
#include "dynarec_la64_private.h"
#include "../dynarec_helper.h"
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
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                emit_add32(dyn, ninst, rex, ed, gd, x3, x4, x5);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                MARKLOCK;
                LLxw(x1, wback, 0);
                ADDxw(x4, x1, gd);
                SCxw(x4, wback, 0);
                BEQZ_MARKLOCK(x4);
                IFXORNAT (X_ALL | X_PEND) {
                    emit_add32(dyn, ninst, rex, x1, gd, x3, x4, x5);
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
                MARKLOCK;
                LLxw(x1, wback, 0);
                OR(x4, x1, gd);
                SCxw(x4, wback, 0);
                BEQZ_MARKLOCK(x4);
                IFXORNAT (X_ALL | X_PEND)
                    emit_or32(dyn, ninst, rex, x1, gd, x3, x4);
            }
            break;
        case 0x0F:
            nextop = F8;
            switch (nextop) {
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
                                BSTRPICK_D(x2, wback, wb2 + 7, wb2);
                                wb1 = 0;
                                ed = x2;
                                UFLAG_IF {
                                    emit_cmp8(dyn, ninst, x6, ed, x3, x4, x5, x1);
                                }
                                BNE_MARK2(x6, x2);
                                GETGB(x1);
                                BSTRINS_D(wback, gd, wb2 + 7, wb2);
                                MARK2;
                                BSTRINS_D(xRAX, x2, 7, 0);
                            } else {
                                if (rex.rex) {
                                    gb1 = TO_NAT(((nextop & 0x38) >> 3) + (rex.r << 3));
                                    gb2 = 0;
                                } else {
                                    gd = (nextop & 0x38) >> 3;
                                    gb2 = ((gd & 4) >> 2) * 8;
                                    gb1 = TO_NAT(gd & 3);
                                }
                                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x2, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                                ANDI(x5, wback, 0b11);
                                SLLI_D(x5, x5, 3); // shamt
                                MARKLOCK;
                                ADDI_D(x7, xZR, ~0b11);
                                AND(x7, wback, x7); // align to 32bit
                                LL_W(x4, x7, 0);
                                MV(x1, x4);
                                SRL_D(x4, x4, x5);
                                ANDI(x4, x4, 0xff);
                                BNE_MARK(x6, x4); // compare AL with m8
                                // AL == m8, r8 is loaded into m8
                                ADDI_D(x2, xZR, 0xff);
                                SLL_D(x2, x2, x5);
                                NOR(x2, x2, xZR);
                                AND(x2, x1, x2);
                                BSTRPICK_D(x1, gb1, gb2 + 7, gb2);
                                SLL_D(x1, x1, x5);
                                OR(x1, x1, x2);
                                SC_W(x1, x7, 0);
                                BEQZ_MARKLOCK(x1);
                                // done
                                MARK;
                                UFLAG_IF { emit_cmp8(dyn, ninst, x6, x4, x1, x2, x3, x5); }
                                BSTRINS_D(xRAX, x4, 7, 0);
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
                                SUBxw(x2, x1, xRAX);
                                BNE_MARK2(x2, xZR);
                                MVxw(ed, gd);
                                if (!rex.w) { B_NEXT_nocond; }
                                MARK2;
                                MVxw(xRAX, x1);
                            } else {
                                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                                UFLAG_IF { MVxw(x6, xRAX); }
                                ANDI(x1, wback, (1 << (rex.w + 2)) - 1);
                                BNEZ_MARK3(x1);
                                // Aligned
                                MARKLOCK;
                                MV(x4, gd);
                                LLxw(x1, wback, 0);
                                SUBxw(x3, x1, xRAX);
                                BNEZ(x3, 4 + (rex.w ? 8 : 12));
                                // EAX == Ed
                                SCxw(x4, wback, 0);
                                BEQZ_MARKLOCK(x4);
                                if (!rex.w) { B_MARK_nocond; }
                                MVxw(xRAX, x1);
                                if (rex.w) { B_MARK_nocond; }
                                MARK3;
                                // Unaligned
                                ADDI_D(x5, xZR, -(1 << (rex.w + 2)));
                                AND(x5, x5, wback);
                                MARKLOCK2;
                                LDxw(x1, wback, 0);
                                LLxw(x6, x5, 0);
                                SUBxw(x3, x1, xRAX);
                                BNEZ(x3, 4 + (rex.w ? 12 : 16));
                                // EAX == Ed
                                SCxw(x6, x5, 0);
                                BEQZ_MARKLOCK2(x6);
                                SDxw(gd, wback, 0);
                                if (!rex.w) { B_MARK_nocond; }
                                MVxw(xRAX, x1);
                                MARK;
                                UFLAG_IF { emit_cmp32(dyn, ninst, rex, x6, x1, x3, x4, x5, x6); }
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
                                MARKLOCK;
                                LLxw(x1, wback, 0);
                                ADDxw(x4, x1, gd);
                                SCxw(x4, wback, 0);
                                BEQZ_MARKLOCK(x4);
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
                    // rep has no impact here
                    nextop = F8;
                    switch ((nextop >> 3) & 7) {
                        case 1:
                            if (rex.w) {
                                INST_NAME("LOCK CMPXCHG16B Gq, Eq");
                                if (!cpuext.scq) {
                                    static int warned = 0;
                                    PASS3(if (!warned) dynarec_log(LOG_INFO, "Warning, LOCK CMPXCHG16B is not well supported on LoongArch without SCQ and issues are expected.\n"));
                                    warned = 1;
                                }
                            } else {
                                INST_NAME("LOCK CMPXCHG8B Gq, Eq");
                            }
                            SETFLAGS(X_ZF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                            addr = geted(dyn, addr, ninst, nextop, &wback, x1, x2, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                            UFLAG_IF {
                                if (cpuext.lbt) {
                                    X64_SET_EFLAGS(xZR, X_ZF);
                                } else {
                                    BSTRINS_D(xFlags, xZR, F_ZF, F_ZF);
                                }
                            }
                            if (rex.w) {
                                if (cpuext.scq) {
                                    MARKLOCK;
                                    LL_D(x2, wback, 0);
                                    LD_D(x3, wback, 8);
                                    // compare RDX:RAX with x3:x2
                                    BNE_MARK(x2, xRAX);
                                    BNE_MARK(x3, xRDX);
                                    MV(x5, xRBX);
                                    SC_Q(x5, xRCX, wback);
                                    BEQZ_MARKLOCK(x5);
                                    UFLAG_IF {
                                        if (cpuext.lbt) {
                                            ADDI_D(x5, xZR, -1);
                                            X64_SET_EFLAGS(x5, X_ZF);
                                        } else {
                                            ORI(xFlags, xFlags, 1 << F_ZF);
                                        }
                                    }
                                    B_MARK3_nocond;
                                    MARK;
                                    MV(xRAX, x2);
                                    MV(xRDX, x3);
                                    MARK3;
                                } else {
                                    // if scq extension is not available, implement it with mutex
                                    LD_D(x6, xEmu, offsetof(x64emu_t, context));
                                    ADDI_D(x6, x6, offsetof(box64context_t, mutex_16b));
                                    ADDI_D(x4, xZR, 1);
                                    MARKLOCK;
                                    AMSWAP_DB_W(x5, x4, x6);
                                    // x4 == 1 if locked
                                    BNEZ_MARKLOCK(x5);

                                    LD_D(x2, wback, 0);
                                    LD_D(x3, wback, 8);
                                    BNE_MARK(x2, xRAX);
                                    BNE_MARK(x3, xRDX);
                                    ST_D(xRBX, wback, 0);
                                    ST_D(xRCX, wback, 8);
                                    UFLAG_IF {
                                        if (cpuext.lbt) {
                                            ADDI_D(x5, xZR, -1);
                                            X64_SET_EFLAGS(x5, X_ZF);
                                        } else {
                                            ORI(xFlags, xFlags, 1 << F_ZF);
                                        }
                                    }
                                    B_MARK3_nocond;
                                    MARK;
                                    MV(xRAX, x2);
                                    MV(xRDX, x3);
                                    MARK3;

                                    // unlock
                                    AMSWAP_DB_W(xZR, xZR, x6);
                                }
                            } else {
                                BSTRINS_D(x3, xRAX, 31, 0);
                                BSTRINS_D(x3, xRDX, 63, 32);
                                BSTRINS_D(x4, xRBX, 31, 0);
                                BSTRINS_D(x4, xRCX, 63, 32);
                                ANDI(x2, wback, 7);
                                BNEZ_MARK2(x2);
                                // Aligned
                                MARKLOCK;
                                LL_D(x2, wback, 0);
                                BNE_MARK(x2, x3); // EDX_EAX != Ed
                                MV(x5, x4);
                                SC_D(x5, wback, 0);
                                BEQZ_MARKLOCK(x5);
                                UFLAG_IF {
                                    if (cpuext.lbt) {
                                        ADDI_D(x5, xZR, -1);
                                        X64_SET_EFLAGS(x5, X_ZF);
                                    } else {
                                        ORI(xFlags, xFlags, 1 << F_ZF);
                                    }
                                }
                                B_MARK3_nocond;
                                MARK2;
                                // Unaligned
                                ADDI_W(x5, xZR, 0xFF8);
                                AND(x5, wback, x5);
                                MARKLOCK2;
                                LD_D(x2, wback, 0);
                                LL_D(x6, x5, 0);
                                BNE_MARK(x2, x3); // EDX_EAX != Ed
                                SC_D(x6, x5, 0);
                                BEQZ_MARKLOCK2(x6);
                                ST_D(x4, wback, 0);
                                UFLAG_IF {
                                    if (cpuext.lbt) {
                                        ADDI_D(x5, xZR, -1);
                                        X64_SET_EFLAGS(x5, X_ZF);
                                    } else {
                                        ORI(xFlags, xFlags, 1 << F_ZF);
                                    }
                                }
                                B_MARK3_nocond;
                                MARK;
                                SRLI_D(xRDX, x2, 32);
                                ZEROUP2(xRAX, x2);
                                MARK3;
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
                LLxw(x1, wback, 0);
                ADDxw(x3, x1, gd);
                ANDI(x4, xFlags, 1 << F_CF);
                ADDxw(x3, x3, x4);
                SCxw(x3, wback, 0);
                BEQZ_MARKLOCK(x3);
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
                AMAND_DBxw(x1, gd, wback);
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
                MARKLOCK;
                LLxw(x1, wback, 0);
                SUB_D(x4, x1, gd);
                SCxw(x4, wback, 0);
                BEQZ_MARKLOCK(x4);
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
                        EBBACK();
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x5, x1, &fixedaddress, rex, LOCK_LOCK, 0, 1);
                        u8 = F8;
                        ANDI(x2, wback, 3);
                        SLLI_D(x2, x2, 3);   // offset in bits
                        MV(x3, wback); // aligned addr
                        BSTRINS_D(x3, xZR, 1, 0);
                        ADDI_D(x1, xZR, u8);
                        SLL_D(x1, x1, x2); // Ib << offset
                        AMOR_DB_W(x4, x1, x3);
                        IFXORNAT (X_ALL | X_PEND) {
                            SRL_D(x1, x4, x2);
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
                case 0:
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
                        IFXORNAT (X_ALL | X_PEND) {
                            emit_add32c(dyn, ninst, rex, x1, i64, x3, x4, x5, x6);
                        }
                    }
                    break;
                case 1:
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
                        if (opcode == 0x81) {
                            i64 = F32S;
                        } else {
                            i64 = F8S;
                        }
                        if (i64 <= -2048 || i64 > 2048) {
                            MOV64xw(x3, i64);
                        }
                        MARKLOCK;
                        LLxw(x1, wback, 0);
                        if (i64 >= -2048 && i64 < 2048) {
                            ORI(x4, x1, i64);
                        } else {
                            OR(x4, x1, x3);
                        }
                        if (!rex.w) ZEROUP(x4);
                        SCxw(x4, wback, 0);
                        BEQZ_MARKLOCK(x4);
                        IFXORNAT (X_ALL | X_PEND)
                            emit_or32c(dyn, ninst, rex, x1, i64, x3, x4);
                    }
                    break;
                case 4:
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
                        if (rex.w) {
                            AMAND_DB_D(x1, x7, wback);
                        } else {
                            AMAND_DB_W(x1, x7, wback);
                        }
                        IFXORNAT (X_ALL | X_PEND)
                            emit_and32c(dyn, ninst, rex, x1, i64, x3, x4);
                    }
                    break;
                case 5:
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
                        IFXORNAT (X_ALL | X_PEND)
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
                GETGD;
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                MARKLOCK;
                LLxw(x1, wback, 0);
                MV(x4, gd);
                SCxw(x4, wback, 0);
                BEQZ_MARKLOCK(x4);
                MVxw(gd, x1);
            }
            break;
        case 0xFF:
            nextop = F8;

            switch ((nextop >> 3) & 7) {
                case 0:
                    INST_NAME("LOCK INC Ed");
                    SETFLAGS(X_ALL & ~X_CF, SF_SUBSET_PENDING, NAT_FLAGS_FUSION);
                    if (MODREG) {
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        emit_inc32(dyn, ninst, rex, ed, x3, x4, x5, x6);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                        MARKLOCK;
                        LLxw(x1, wback, 0);
                        ADDIxw(x4, x1, 1);
                        SCxw(x4, wback, 0);
                        BEQZ_MARKLOCK(x4);
                        IFXORNAT (X_ALL | X_PEND)
                            emit_inc32(dyn, ninst, rex, x1, x3, x4, x5, x6);
                    }
                    break;
                case 1:
                    INST_NAME("LOCK DEC Ed");
                    SETFLAGS(X_ALL & ~X_CF, SF_SUBSET_PENDING, NAT_FLAGS_FUSION);
                    if (MODREG) {
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        emit_dec32(dyn, ninst, rex, ed, x3, x4, x5, x6);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                        MARKLOCK;
                        LLxw(x1, wback, 0);
                        ADDIxw(x4, x1, -1);
                        SCxw(x4, wback, 0);
                        BEQZ_MARKLOCK(x4);
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
