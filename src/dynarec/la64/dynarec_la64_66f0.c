#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "la64_emitter.h"
#include "la64_mapping.h"
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


uintptr_t dynarec64_66F0(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
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
        case 0x01:
            nextop = F8;
            if (MODREG) {
                INST_NAME("Invalid LOCK");
                UDF();
                *need_epilog = 1;
                *ok = 0;
            } else {
                INST_NAME("LOCK ADD Ew, Gw");
                SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                GETGW(x5);
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                if (cpuext.lam_bh) {
                    AMADD_DB_H(x1, x5, wback);
                } else {
                    ANDI(x3, wback, 0b10);
                    BNEZ_MARK(x3);
                    // lower 16 bits
                    MARKLOCK;
                    LL_W(x1, wback, 0);
                    ADD_D(x4, x1, x5);
                    MV(x3, x1);
                    BSTRINS_D(x3, x4, 15, 0);
                    SC_W(x3, wback, 0);
                    BEQZ_MARKLOCK(x3);
                    B_MARK3_nocond;
                    MARK;
                    // upper 16 bits
                    XORI(wback, wback, 0b10);
                    MARK2;
                    LL_W(x1, wback, 0);
                    BSTRPICK_D(x3, x1, 15, 0);
                    SRLI_W(x1, x1, 16);
                    ADD_D(x4, x1, x5);
                    BSTRINS_D(x3, x4, 31, 15);
                    SC_W(x3, wback, 0);
                    BEQZ_MARK2(x3);
                    MARK3;
                }
                // final
                IFXORNAT (X_ALL | X_PEND) {
                    BSTRPICK_D(x6, x1, 15, 0);
                    emit_add16(dyn, ninst, x6, x5, x3, x4, x6);
                }
            }
            break;
        case 0x0F:
            nextop = F8;
            switch (nextop) {
                case 0xAB:
                    if (MODREG) {
                        INST_NAME("Invalid LOCK BTS");
                        UDF();
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        INST_NAME("LOCK BTS Ew, Gw");
                        SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                        SET_DFNONE();
                        nextop = F8;
                        GETGD;
                        addr = geted(dyn, addr, ninst, nextop, &wback, x3, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                        if (rex.w) {
                            SRAI_D(x1, gd, 3);
                        } else {
                            EXT_W_H(x4, gd);
                            SRAI_W(x1, x4, 3);
                        }
                        ANDI(x2, gd, 7);
                        ADD_D(x6, wback, x1);

                        ANDI(x4, x6, 0b11);
                        ALSL_D(x2, x4, x2, 3);
                        BSTRINS_D(x6, xZR, 1, 0);

                        ADDI_D(x5, xZR, 1);
                        SLL_D(x5, x5, x2);
                        AMOR_DB_W(x4, x5, x6);
                        IFX (X_CF) {
                            SRL_D(x4, x4, x2);
                            if (cpuext.lbt) {
                                X64_SET_EFLAGS(x4, X_CF);
                            } else {
                                BSTRINS_D(xFlags, x4, F_CF, F_CF);
                            }
                        }
                    }
                    break;
                case 0xB1:
                    if (MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF();
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        INST_NAME("LOCK CMPXCHG Ew, Gw");
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_NOFUSION);
                        nextop = F8;
                        GETGD;
                        BSTRPICK_D(x6, xRAX, 15, 0);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                        // Aligned
                        if (cpuext.lamcas) {
                            MV(x1, x6);
                            AMCAS_DB_H(x1, gd, wback);
                            BSTRPICK_D(x1, x1, 15, 0);
                            BEQ_MARK(x1, x6); // equal = cas success.
                        } else {
                            MV(x7, wback);
                            BSTRINS_D(x7, xZR, 1, 0);
                            ANDI(x3, wback, 0b10);
                            BEQZ(x3, 4 + 4 * 7);
                            // hi16
                            LL_W(x5, x7, 0);
                            BSTRPICK_D(x1, x5, 31, 16);
                            BNE_MARK2(x1, x6);
                            BSTRINS_D(x5, gd, 31, 16);
                            SC_W(x5, x7, 0);
                            BEQZ(x5, -4 * 5);
                            B_MARK_nocond;
                            // lo16
                            LL_W(x5, x7, 0);
                            BSTRPICK_D(x1, x5, 15, 0);
                            BNE_MARK2(x1, x6);
                            BSTRINS_D(x5, gd, 15, 0);
                            SC_W(x5, x7, 0);
                            BEQZ(x5, -4 * 5);
                            B_MARK_nocond;
                        }
                        MARK2;
                        BSTRINS_D(xRAX, x1, 15, 0);
                        MARK;
                        UFLAG_IF { emit_cmp16(dyn, ninst, x6, x1, x3, x4, x5, x7); }
                    }
                    break;
                case 0xB3:
                    if (MODREG) {
                        INST_NAME("Invalid LOCK BTR");
                        UDF();
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        INST_NAME("LOCK BTR Ew, Gw");
                        SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                        SET_DFNONE();
                        nextop = F8;
                        GETGD;
                        addr = geted(dyn, addr, ninst, nextop, &wback, x3, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                        if (rex.w) {
                            SRAI_D(x1, gd, 3);
                        } else {
                            EXT_W_H(x4, gd);
                            SRAI_W(x1, x4, 3);
                        }
                        ANDI(x2, gd, 7);
                        ADD_D(x6, wback, x1);

                        ANDI(x4, x6, 0b11);
                        ALSL_D(x2, x4, x2, 3);
                        BSTRINS_D(x6, xZR, 1, 0);

                        ADDI_D(x5, xZR, 1);
                        SLL_D(x5, x5, x2);
                        NOR(x5, x5, xZR);
                        AMAND_DB_W(x4, x5, x6);
                        IFX (X_CF) {
                            SRL_D(x4, x4, x2);
                            if (cpuext.lbt) {
                                X64_SET_EFLAGS(x4, X_CF);
                            } else {
                                BSTRINS_D(xFlags, x4, F_CF, F_CF);
                            }
                        }
                    }
                    break;
                case 0xBA:
                    nextop = F8;
                    switch ((nextop >> 3) & 7) {
                        case 4:
                            INST_NAME("Invalid LOCK");
                            UDF();
                            *need_epilog = 1;
                            *ok = 0;
                            break;
                        case 5:
                            if (MODREG) {
                                INST_NAME("Invalid LOCK BTS");
                                UDF();
                                *need_epilog = 1;
                                *ok = 0;
                            } else {
                                INST_NAME("LOCK BTS Ew, Ib");
                                SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                                SET_DFNONE();
                                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x1, &fixedaddress, rex, NULL, 0, 1);
                                u8 = F8;
                                u8 &= (rex.w ? 0x3f : 0xf);
                                ADDI_D(x6, wback, u8 >> 3);
                                MOV64x(x2, u8 & 7);
                                ANDI(x4, x6, 0b11);
                                ALSL_D(x2, x4, x2, 3);
                                BSTRINS_D(x6, xZR, 1, 0); // aligned_dword_addr = byte_addr & ~3

                                ADDI_D(x5, xZR, 1);
                                SLL_D(x5, x5, x2); // mask = 1 << total_bit_offset
                                AMOR_DB_W(x4, x5, x6);
                                IFX (X_CF) {
                                    SRL_D(x4, x4, x2);
                                    if (cpuext.lbt) {
                                        X64_SET_EFLAGS(x4, X_CF);
                                    } else {
                                        BSTRINS_D(xFlags, x4, F_CF, F_CF);
                                    }
                                }
                            }
                            break;
                        case 6:
                            if (MODREG) {
                                INST_NAME("Invalid LOCK BTR");
                                UDF();
                                *need_epilog = 1;
                                *ok = 0;
                            } else {
                                INST_NAME("LOCK BTR Ew, Ib");
                                SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                                SET_DFNONE();
                                GETGD;
                                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x1, &fixedaddress, rex, NULL, 0, 1);
                                u8 = F8;
                                u8 &= (rex.w ? 0x3f : 0xf);
                                ADDI_D(x6, wback, u8 >> 3);
                                MOV64x(x2, u8 & 7);
                                ANDI(x4, x6, 0b11);
                                ALSL_D(x2, x4, x2, 3);
                                BSTRINS_D(x6, xZR, 1, 0); // aligned_dword_addr = byte_addr & ~3

                                ADDI_D(x5, xZR, 1);
                                SLL_D(x5, x5, x2); // mask = 1 << total_bit_offset
                                NOR(x5, x5, xZR);
                                AMAND_DB_W(x4, x5, x6);
                                IFX (X_CF) {
                                    SRL_D(x4, x4, x2);
                                    if (cpuext.lbt) {
                                        X64_SET_EFLAGS(x4, X_CF);
                                    } else {
                                        BSTRINS_D(xFlags, x4, F_CF, F_CF);
                                    }
                                }
                            }
                            break;
                        default:
                            DEFAULT;
                    }
                    break;
                case 0xC1:
                    nextop = F8;
                    if (MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF();
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        INST_NAME("LOCK XADD Gw, Ew");
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                        GETGD;
                        BSTRPICK_D(x5, gd, 15, 0);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                        if (cpuext.lam_bh) {
                            AMADD_DB_H(x1, x5, wback);
                        } else {
                            ANDI(x3, wback, 0b10);
                            BNEZ_MARK(x3);
                            // lower 16 bits
                            MARKLOCK;
                            LL_W(x1, wback, 0);
                            ADD_D(x4, x1, x5);
                            MV(x3, x1);
                            BSTRINS_D(x3, x4, 15, 0);
                            SC_W(x3, wback, 0);
                            BEQZ_MARKLOCK(x3);
                            B_MARK3_nocond;
                            MARK;
                            // upper 16 bits
                            XORI(wback, wback, 0b10);
                            MARK2;
                            LL_W(x1, wback, 0);
                            BSTRPICK_D(x3, x1, 15, 0);
                            SRLI_W(x1, x1, 16);
                            ADD_D(x4, x1, x5);
                            BSTRINS_D(x3, x4, 31, 15);
                            SC_W(x3, wback, 0);
                            BEQZ_MARK2(x3);
                            MARK3;
                        }
                        IFXORNAT (X_ALL | X_PEND) {
                            BSTRPICK_D(x6, x1, 15, 0);
                            emit_add16(dyn, ninst, x6, gd, x3, x4, x5);
                        }
                        BSTRINS_D(gd, x1, 15, 0);
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
                    if (MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF();
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        if (opcode == 0x81) {
                            INST_NAME("LOCK ADD Ew, Iw");
                        } else {
                            INST_NAME("LOCK ADD Ew, Ib");
                        }
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, (opcode == 0x81) ? 2 : 1);
                        if (opcode == 0x81)
                            u64 = F16;
                        else
                            u64 = (uint16_t)(int16_t)F8S;
                        MOV64x(x5, u64);

                        if (cpuext.lam_bh) {
                            AMADD_DB_H(x1, x5, wback);
                        } else {
                            ANDI(x3, wback, 0b10);
                            BNEZ_MARK(x3);
                            // lower 16 bits
                            MARKLOCK;
                            LL_W(x1, wback, 0);
                            ADD_D(x4, x1, x5);
                            MV(x3, x1);
                            BSTRINS_D(x3, x4, 15, 0);
                            SC_W(x3, wback, 0);
                            BEQZ_MARKLOCK(x3);
                            B_MARK3_nocond;
                            MARK;
                            // upper 16 bits
                            XORI(wback, wback, 0b10);
                            MARK2;
                            LL_W(x1, wback, 0);
                            BSTRPICK_D(x3, x1, 15, 0);
                            SRLI_W(x1, x1, 16);
                            ADD_D(x4, x1, x5);
                            BSTRINS_D(x3, x4, 31, 15);
                            SC_W(x3, wback, 0);
                            BEQZ_MARK2(x3);
                            MARK3;
                        }
                        // final
                        IFXORNAT (X_ALL | X_PEND) {
                            BSTRPICK_D(x6, x1, 15, 0);
                            emit_add16(dyn, ninst, x6, x5, x3, x4, x6);
                        }
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xFF:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                    if (MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF();
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        INST_NAME("LOCK INC Ew");
                        SETFLAGS(X_ALL & ~X_CF, SF_SUBSET, NAT_FLAGS_FUSION);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                        if (cpuext.lam_bh) {
                            ADDI_D(x5, xZR, 1);
                            AMADD_DB_H(x1, x5, wback);
                        } else {
                            ANDI(x3, wback, 0b10);
                            BNEZ_MARK(x3);
                            // lower 16 bits
                            MARKLOCK;
                            LL_W(x1, wback, 0);
                            ADDI_D(x4, x1, 1);
                            MV(x3, x1);
                            BSTRINS_D(x3, x4, 15, 0);
                            SC_W(x3, wback, 0);
                            BEQZ_MARKLOCK(x3);
                            B_MARK3_nocond;
                            MARK;
                            // upper 16 bits
                            XORI(wback, wback, 0b10);
                            MARK2;
                            LL_W(x1, wback, 0);
                            BSTRPICK_D(x3, x1, 15, 0);
                            SRLI_W(x1, x1, 16);
                            ADDI_D(x4, x1, 1);
                            BSTRINS_D(x3, x4, 31, 15);
                            SC_W(x3, wback, 0);
                            BEQZ_MARK2(x3);
                            MARK3;
                        }
                        // final
                        IFXORNAT (X_ALL | X_PEND) {
                            if (!cpuext.lam_bh) ADDI_D(x5, xZR, 1);
                            BSTRPICK_D(x6, x1, 15, 0);
                            emit_add16(dyn, ninst, x6, x5, x3, x4, x6);
                        }
                    }
                    break;
                case 1:
                    if (MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF();
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        INST_NAME("LOCK DEC Ew");
                        SETFLAGS(X_ALL & ~X_CF, SF_SUBSET, NAT_FLAGS_FUSION);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                        if (cpuext.lam_bh) {
                            ADDI_D(x5, xZR, -1);
                            AMADD_DB_H(x1, x5, wback);
                        } else {
                            ANDI(x3, wback, 0b10);
                            BNEZ_MARK(x3);
                            // lower 16 bits
                            MARKLOCK;
                            LL_W(x1, wback, 0);
                            ADDI_D(x4, x1, -1);
                            MV(x3, x1);
                            BSTRINS_D(x3, x4, 15, 0);
                            SC_W(x3, wback, 0);
                            BEQZ_MARKLOCK(x3);
                            B_MARK3_nocond;
                            MARK;
                            // upper 16 bits
                            XORI(wback, wback, 0b10);
                            MARK2;
                            LL_W(x1, wback, 0);
                            BSTRPICK_D(x3, x1, 15, 0);
                            SRLI_W(x1, x1, 16);
                            ADDI_D(x4, x1, -1);
                            BSTRINS_D(x3, x4, 31, 15);
                            SC_W(x3, wback, 0);
                            BEQZ_MARK2(x3);
                            MARK3;
                        }
                        // final
                        IFXORNAT (X_ALL | X_PEND) {
                            if (!cpuext.lam_bh) ADDI_D(x5, xZR, -1);
                            BSTRPICK_D(x6, x1, 15, 0);
                            emit_add16(dyn, ninst, x6, x5, x3, x4, x6);
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
