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


uintptr_t dynarec64_F0(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip;
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

    switch (opcode) {
        case 0x00:
            nextop = F8;
            if (MODREG) {
                INST_NAME("Invalid LOCK");
                UDF();
                *need_epilog = 1;
                *ok = 0;
            } else {
                INST_NAME("LOCK ADD Eb, Gb");
                SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                GETGB(x7);
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                if (cpuext.lam_bh) {
                    AMADD_DB_B(x1, gd, wback);
                } else {
                    LOCK_8_OP(ADD_D(x4, x1, gd), x1, wback, x3, x4, x5, x6);
                }
                IFXORNAT (X_ALL | X_PEND) {
                    emit_add8(dyn, ninst, x1, gd, x3, x4);
                }
            }
            break;
        case 0x01:
            nextop = F8;
            if (MODREG) {
                INST_NAME("Invalid LOCK");
                UDF();
                *need_epilog = 1;
                *ok = 0;
            } else {
                INST_NAME("LOCK ADD Ed, Gd");
                SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                GETGD;
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                if (rex.w) {
                    if (!ALIGNED_ATOMICxw) {
                        ANDI(x3, wback, 0b111);
                        BNEZ_MARK2(x3);
                    }
                    AMADD_DB_D(x1, gd, wback);
                    if (!ALIGNED_ATOMICxw) { B_MARK3_nocond; }
                } else {
                    if (!ALIGNED_ATOMICxw) {
                        ANDI(x3, wback, 0b11);
                        BNEZ_MARK(x3);
                    }
                    // aligned 4byte
                    AMADD_DB_W(x1, gd, wback);
                    if (!ALIGNED_ATOMICxw) {
                        B_MARK3_nocond;
                        MARK;
                        ANDI(x3, wback, 0b111);
                        SLTI(x4, x3, 4);
                        BEQZ_MARK2(x4); // addr %8 >4 , cross 8bytes or cross cacheline
                        LOCK_32_IN_8BYTE(ADD_W(x4, x1, gd), x1, wback, x3, x4, x5, x6);
                        B_MARK3_nocond;
                    }
                }
                if (!ALIGNED_ATOMICxw) {
                    MARK2;
                    LOCK_3264_CROSS_8BYTE(ADDxw(x4, x1, gd), x1, wback, x4, x5, x6);
                    MARK3;
                }
                IFXORNAT (X_ALL | X_PEND) {
                    emit_add32(dyn, ninst, rex, x1, gd, x3, x4, x5);
                }
            }
            break;
        case 0x08:
            nextop = F8;
            if (MODREG) {
                INST_NAME("Invalid LOCK");
                UDF();
                *need_epilog = 1;
                *ok = 0;
            } else {
                INST_NAME("LOCK OR Eb, Gb");
                SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                GETGB(x7);
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                ANDI(x3, wback, 0b11);
                SLLI_W(x3, x3, 3);
                MV(x6, wback);
                BSTRINS_D(x6, xZR, 1, 0); // aligned to 4-byte
                SLL_W(x4, gd, x3);        // shift GETGB's BSTRPICK result to pos
                AMOR_DB_W(x1, x4, x6);
                IFXORNAT (X_ALL | X_PEND) {
                    SRL_W(x1, x1, x3);
                    BSTRPICK_D(x1, x1, 7, 0);
                    emit_or8(dyn, ninst, x1, gd, x3, x4);
                }
            }
            break;
        case 0x09:
            nextop = F8;
            if (MODREG) {
                INST_NAME("Invalid LOCK");
                UDF();
                *need_epilog = 1;
                *ok = 0;
            } else {
                INST_NAME("LOCK OR Ed, Gd");
                SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                GETGD;
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                if (rex.w) {
                    if (!ALIGNED_ATOMICxw) {
                        ANDI(x3, wback, 0b111);
                        BNEZ_MARK2(x3);
                    }
                    AMOR_DB_D(x1, gd, wback);
                    if (!ALIGNED_ATOMICxw) { B_MARK3_nocond; }
                } else {
                    if (!ALIGNED_ATOMICxw) {
                        ANDI(x3, wback, 0b11);
                        BNEZ_MARK(x3);
                    }
                    // aligned 4byte
                    AMOR_DB_W(x1, gd, wback);
                    if (!ALIGNED_ATOMICxw) {
                        B_MARK3_nocond;
                        MARK;
                        ANDI(x3, wback, 0b111);
                        SLTI(x4, x3, 4);
                        BEQZ_MARK2(x4); // addr %8 >4 , cross 8bytes or cross cacheline
                        LOCK_32_IN_8BYTE(OR(x4, x1, gd), x1, wback, x3, x4, x5, x6);
                        B_MARK3_nocond;
                    }
                }
                if (!ALIGNED_ATOMICxw) {
                    MARK2;
                    LOCK_3264_CROSS_8BYTE(OR(x4, x1, gd), x1, wback, x4, x5, x6);
                    MARK3;
                }
                IFXORNAT (X_ALL | X_PEND)
                    emit_or32(dyn, ninst, rex, x1, gd, x3, x4);
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
                        INST_NAME("LOCK BTS Ed, Gd");
                        SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                        SET_DFNONE();
                        nextop = F8;
                        GETGD;
                        addr = geted(dyn, addr, ninst, nextop, &wback, x3, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                        if (rex.w) {
                            SRAI_D(x1, gd, 3);
                        } else {
                            SRAI_W(x1, gd, 3);
                        }
                        if (!rex.w && !rex.is32bits) { ADDI_W(x1, x1, 0); }
                        ADDy(x6, wback, x1);
                        ANDI(x2, gd, 7);
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
                case 0xB0:
                    switch (rex.rep) {
                        case 0:
                            if (MODREG) {
                                INST_NAME("Invalid LOCK");
                                UDF();
                                *need_epilog = 1;
                                *ok = 0;
                            } else {
                                INST_NAME("LOCK CMPXCHG Eb, Gb");
                                SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_NOFUSION);
                                nextop = F8;
                                ANDI(x6, xRAX, 0xff); // AL
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
                    switch (rex.rep) {
                        case 0:
                            nextop = F8;
                            if (MODREG) {
                                INST_NAME("Invalid LOCK");
                                UDF();
                                *need_epilog = 1;
                                *ok = 0;
                            } else {
                                INST_NAME("LOCK CMPXCHG Ed, Gd");
                                SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_NOFUSION);
                                GETGD;
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
                                B_MARK_nocond;
                                MARK3;
                                // Unaligned
                                ADDI_D(x5, xZR, -(1 << (rex.w + 2)));
                                AND(x5, x5, wback);
                                MARKLOCK2;
                                LDxw(x1, wback, 0);
                                LLxw(x7, x5, 0);
                                SUBxw(x3, x1, xRAX);
                                BNEZ(x3, 4 + (rex.w ? 12 : 16));
                                // EAX == Ed
                                SCxw(x7, x5, 0);
                                BEQZ_MARKLOCK2(x7);
                                SDxw(gd, wback, 0);
                                if (!rex.w) { B_MARK_nocond; }
                                MVxw(xRAX, x1);
                                MARK;
                                UFLAG_IF { emit_cmp32(dyn, ninst, rex, x6, x1, x3, x4, x5, x7); }
                            }
                            break;
                        default:
                            DEFAULT;
                    }
                    break;
                case 0xB3:
                    if (MODREG) {
                        INST_NAME("Invalid LOCK BTR");
                        UDF();
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        INST_NAME("LOCK BTR Ed, Gd");
                        SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                        SET_DFNONE();
                        nextop = F8;
                        GETGD;
                        addr = geted(dyn, addr, ninst, nextop, &wback, x3, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                        if (rex.w) {
                            SRAI_D(x1, gd, 3);
                        } else {
                            SRAI_W(x1, gd, 3);
                        }
                        if (!rex.w && !rex.is32bits) { ADDI_W(x1, x1, 0); }
                        ADDy(x6, wback, x1);
                        ANDI(x2, gd, 7);

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
                                INST_NAME("LOCK BTS Ed, Ib");
                                SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                                SET_DFNONE();
                                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x1, &fixedaddress, rex, NULL, 0, 1);
                                u8 = F8;
                                u8 &= (rex.w ? 0x3f : 0x1f);
                                ADDI_D(x6, wback, u8 >> 3);
                                MOV64x(x2, u8 & 7);
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
                        case 6:
                            if (MODREG) {
                                INST_NAME("Invalid LOCK BTR");
                                UDF();
                                *need_epilog = 1;
                                *ok = 0;
                            } else {
                                INST_NAME("LOCK BTR Ed, Ib");
                                SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                                SET_DFNONE();
                                GETGD;
                                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x1, &fixedaddress, rex, NULL, 0, 1);
                                u8 = F8;
                                u8 &= (rex.w ? 0x3f : 0x1f);
                                ADDI_D(x6, wback, u8 >> 3);
                                MOV64x(x2, u8 & 7);
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
                        default:
                            DEFAULT;
                    }
                    break;
                case 0xC0:
                    switch (rex.rep) {
                        case 0:
                            nextop = F8;
                            if (MODREG) {
                                INST_NAME("Invalid LOCK");
                                UDF();
                                *need_epilog = 1;
                                *ok = 0;
                            } else {
                                INST_NAME("LOCK XADD Eb, Gb");
                                SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                                GETGB(x7);
                                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                                if (cpuext.lam_bh) {
                                    AMADD_DB_B(x1, gd, wback);
                                } else {
                                    LOCK_8_OP(ADD_D(x4, gd, x1), x1, wback, x3, x4, x5, x6);
                                }
                                IFXORNAT (X_ALL | X_PEND) {
                                    ANDI(x6, x1, 0xff);
                                    emit_add8(dyn, ninst, x6, gd, x3, x4);
                                }
                                BSTRINSz(gb1, x1, gb2 + 7, gb2);
                            }
                            break;
                        default:
                            DEFAULT;
                    }
                    break;
                case 0xC1:
                    switch (rex.rep) {
                        case 0:
                            nextop = F8;
                            if (MODREG) {
                                INST_NAME("Invalid LOCK");
                                UDF();
                                *need_epilog = 1;
                                *ok = 0;
                            } else {
                                INST_NAME("LOCK XADD Ed, Gd");
                                SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                                GETGD;
                                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                                if (rex.w) {
                                    if (!ALIGNED_ATOMICxw) {
                                        ANDI(x3, wback, 0b111);
                                        BNEZ_MARK2(x3);
                                    }
                                    AMADD_DB_D(x1, gd, wback);
                                    if (!ALIGNED_ATOMICxw) { B_MARK3_nocond; }
                                } else {
                                    if (!ALIGNED_ATOMICxw) {
                                        ANDI(x3, wback, 0b11);
                                        BNEZ_MARK(x3);
                                    }
                                    // aligned 4byte
                                    AMADD_DB_W(x1, gd, wback);
                                    if (!ALIGNED_ATOMICxw) {
                                        B_MARK3_nocond;
                                        MARK;
                                        ANDI(x3, wback, 0b111);
                                        SLTI(x4, x3, 4);
                                        BEQZ_MARK2(x4); // addr %8 >4 , cross 8bytes or cross cacheline
                                        LOCK_32_IN_8BYTE(ADD_W(x4, x1, gd), x1, wback, x3, x4, x5, x6);
                                        B_MARK3_nocond;
                                    }
                                }
                                if (!ALIGNED_ATOMICxw) {
                                    MARK2;
                                    LOCK_3264_CROSS_8BYTE(ADDxw(x4, x1, gd), x1, wback, x4, x5, x6);
                                    MARK3;
                                }
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
            nextop = F8;
            if (MODREG) {
                INST_NAME("Invalid LOCK");
                UDF();
                *need_epilog = 1;
                *ok = 0;
            } else {
                INST_NAME("LOCK ADC Ed, Gd");
                READFLAGS(X_CF);
                SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                GETGD;
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                if (cpuext.lbt) {
                    X64_GET_EFLAGS(x7, X_CF);
                } else {
                    BSTRPICK_D(x7, xFlags, F_CF, F_CF);
                }
                if (rex.w) {
                    if (!ALIGNED_ATOMICxw) {
                        ANDI(x3, wback, 0b111);
                        BNEZ_MARK2(x3);
                    }
                    ADD_D(x7, gd, x7);
                    AMADD_DB_D(x1, x7, wback);
                    if (!ALIGNED_ATOMICxw) { B_MARK3_nocond; }
                } else {
                    if (!ALIGNED_ATOMICxw) {
                        ANDI(x3, wback, 0b11);
                        BNEZ_MARK(x3);
                    }
                    // aligned 4byte
                    ADD_W(x7, gd, x7);
                    AMADD_DB_W(x1, x7, wback);
                    if (!ALIGNED_ATOMICxw) {
                        B_MARK3_nocond;
                        MARK;
                        ANDI(x3, wback, 0b111);
                        SLTI(x4, x3, 4);
                        BEQZ_MARK2(x4); // addr %8 >4 , cross 8bytes or cross cacheline
                        ADD_W(x7, gd, x7);
                        LOCK_32_IN_8BYTE(ADD_W(x4, x1, x7), x1, wback, x3, x4, x5, x6);
                        B_MARK3_nocond;
                    }
                }
                if (!ALIGNED_ATOMICxw) {
                    MARK2;
                    ADDxw(x7, gd, x7);
                    LOCK_3264_CROSS_8BYTE(ADDxw(x4, x1, x7), x1, wback, x4, x5, x6);
                    MARK3;
                }
                IFXORNAT (X_ALL | X_PEND) {
                    emit_adc32(dyn, ninst, rex, x1, gd, x3, x4, x5, x6);
                }
            }
            break;
        case 0x19:
            nextop = F8;
            if (MODREG) {
                INST_NAME("Invalid LOCK");
                UDF();
                *need_epilog = 1;
                *ok = 0;
            } else {
                INST_NAME("LOCK SBB Ed, Gd");
                READFLAGS(X_CF);
                SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                GETGD;
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                if (cpuext.lbt) {
                    X64_GET_EFLAGS(x6, X_CF);
                } else {
                    BSTRPICK_D(x6, xFlags, F_CF, F_CF);
                }
                if (rex.w) {
                    if (!ALIGNED_ATOMICxw) {
                        ANDI(x3, wback, 0b111);
                        BNEZ_MARK2(x3);
                    }
                    SUB_D(x7, xZR, gd);
                    SUB_D(x7, x7, x6);
                    AMADD_DB_D(x1, x7, wback);
                    if (!ALIGNED_ATOMICxw) { B_MARK3_nocond; }
                } else {
                    if (!ALIGNED_ATOMICxw) {
                        ANDI(x3, wback, 0b11);
                        BNEZ_MARK(x3);
                    }
                    // aligned 4byte
                    SUB_W(x7, xZR, gd);
                    SUB_W(x7, x7, x6);
                    AMADD_DB_W(x1, x7, wback);
                    if (!ALIGNED_ATOMICxw) {
                        B_MARK3_nocond;
                        MARK;
                        ANDI(x3, wback, 0b111);
                        SLTI(x4, x3, 4);
                        BEQZ_MARK2(x4); // addr %8 >4 , cross 8bytes or cross cacheline
                        SUB_W(x7, xZR, gd);
                        SUB_W(x7, x7, x6);
                        LOCK_32_IN_8BYTE(ADD_W(x4, x1, x7), x1, wback, x3, x4, x5, x6);
                        B_MARK3_nocond;
                    }
                }
                if (!ALIGNED_ATOMICxw) {
                    MARK2;
                    SUBxw(x7, xZR, gd);
                    SUBxw(x7, x7, x6);
                    LOCK_3264_CROSS_8BYTE(ADDxw(x4, x1, x7), x1, wback, x4, x5, x6);
                    MARK3;
                }
                IFXORNAT (X_ALL | X_PEND) {
                    emit_sbb32(dyn, ninst, rex, x1, gd, x3, x4, x5);
                }
            }
            break;
        case 0x20:
            nextop = F8;
            if (MODREG) {
                INST_NAME("Invalid LOCK");
                UDF();
                *need_epilog = 1;
                *ok = 0;
            } else {
                INST_NAME("LOCK AND Eb, Gb");
                SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                GETGB(x7);
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                ANDI(x3, wback, 0b11);
                SLLI_W(x3, x3, 3);
                MV(x6, wback);
                BSTRINS_D(x6, xZR, 1, 0);
                SLL_W(x4, gd, x3);
                ADDI_D(x5, xZR, 0xFF);
                SLL_W(x5, x5, x3);
                ORN(x4, x4, x5);
                AMAND_DB_W(x1, x4, x6);
                IFXORNAT (X_ALL | X_PEND) {
                    SRL_W(x1, x1, x3);
                    BSTRPICK_D(x1, x1, 7, 0);
                    emit_and8(dyn, ninst, x1, gd, x3, x4);
                }
            }
            break;
        case 0x21:
            nextop = F8;
            if (MODREG) {
                INST_NAME("Invalid LOCK");
                UDF();
                *need_epilog = 1;
                *ok = 0;
            } else {
                INST_NAME("LOCK AND Ed, Gd");
                SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                GETGD;
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                if (rex.w) {
                    if (!ALIGNED_ATOMICxw) {
                        ANDI(x3, wback, 0b111);
                        BNEZ_MARK2(x3);
                    }
                    AMAND_DB_D(x1, gd, wback);
                    if (!ALIGNED_ATOMICxw) { B_MARK3_nocond; }
                } else {
                    if (!ALIGNED_ATOMICxw) {
                        ANDI(x3, wback, 0b11);
                        BNEZ_MARK(x3);
                    }
                    // aligned 4byte
                    AMAND_DB_W(x1, gd, wback);
                    if (!ALIGNED_ATOMICxw) {
                        B_MARK3_nocond;
                        MARK;
                        ANDI(x3, wback, 0b111);
                        SLTI(x4, x3, 4);
                        BEQZ_MARK2(x4); // addr %8 >4 , cross 8bytes or cross cacheline
                        LOCK_32_IN_8BYTE(AND(x4, x1, gd), x1, wback, x3, x4, x5, x6);
                        B_MARK3_nocond;
                    }
                }
                if (!ALIGNED_ATOMICxw) {
                    MARK2;
                    LOCK_3264_CROSS_8BYTE(AND(x4, x1, gd), x1, wback, x4, x5, x6);
                    MARK3;
                }
                IFXORNAT (X_ALL | X_PEND)
                    emit_and32(dyn, ninst, rex, x1, gd, x3, x4);
            }
            break;
        case 0x29:
            nextop = F8;
            if (MODREG) {
                INST_NAME("Invalid LOCK");
                UDF();
                *need_epilog = 1;
                *ok = 0;
            } else {
                INST_NAME("LOCK SUB Ed, Gd");
                SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                GETGD;
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                if (rex.w) {
                    if (!ALIGNED_ATOMICxw) {
                        ANDI(x3, wback, 0b111);
                        BNEZ_MARK2(x3);
                    }
                    NEG_D(x7, gd);
                    AMADD_DB_D(x1, x7, wback);
                    if (!ALIGNED_ATOMICxw) { B_MARK3_nocond; }
                } else {
                    if (!ALIGNED_ATOMICxw) {
                        ANDI(x3, wback, 0b11);
                        BNEZ_MARK(x3);
                    }
                    // aligned 4byte
                    NEGxw(x7, gd);
                    AMADD_DB_W(x1, x7, wback);
                    if (!ALIGNED_ATOMICxw) {
                        B_MARK3_nocond;
                        MARK;
                        ANDI(x3, wback, 0b111);
                        SLTI(x4, x3, 4);
                        BEQZ_MARK2(x4); // addr %8 >4 , cross 8bytes or cross cacheline
                        LOCK_32_IN_8BYTE(SUB_W(x4, x1, gd), x1, wback, x3, x4, x5, x6);
                        B_MARK3_nocond;
                    }
                }
                if (!ALIGNED_ATOMICxw) {
                    MARK2;
                    LOCK_3264_CROSS_8BYTE(SUBxw(x4, x1, gd), x1, wback, x4, x5, x6);
                    MARK3;
                }
                IFXORNAT (X_ALL | X_PEND)
                    emit_sub32(dyn, ninst, rex, x1, gd, x3, x4, x5);
            }
            break;
        case 0x31:
            nextop = F8;
            if (MODREG) {
                INST_NAME("Invalid LOCK");
                UDF();
                *need_epilog = 1;
                *ok = 0;
            } else {
                INST_NAME("LOCK XOR Ed, Gd");
                SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                GETGD;
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                if (rex.w) {
                    if (!ALIGNED_ATOMICxw) {
                        ANDI(x3, wback, 0b111);
                        BNEZ_MARK2(x3);
                    }
                    AMXOR_DB_D(x1, gd, wback);
                    if (!ALIGNED_ATOMICxw) { B_MARK3_nocond; }
                } else {
                    if (!ALIGNED_ATOMICxw) {
                        ANDI(x3, wback, 0b11);
                        BNEZ_MARK(x3);
                    }
                    // aligned 4byte
                    AMXOR_DB_W(x1, gd, wback);
                    if (!ALIGNED_ATOMICxw) {
                        B_MARK3_nocond;
                        MARK;
                        ANDI(x3, wback, 0b111);
                        SLTI(x4, x3, 4);
                        BEQZ_MARK2(x4); // addr %8 >4 , cross 8bytes or cross cacheline
                        LOCK_32_IN_8BYTE(XOR(x4, x1, gd), x1, wback, x3, x4, x5, x6);
                        B_MARK3_nocond;
                    }
                }
                if (!ALIGNED_ATOMICxw) {
                    MARK2;
                    LOCK_3264_CROSS_8BYTE(XOR(x4, x1, gd), x1, wback, x4, x5, x6);
                    MARK3;
                }
                IFXORNAT (X_ALL | X_PEND)
                    emit_xor32(dyn, ninst, rex, x1, gd, x3, x4);
            }
            break;

        case 0x80:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 1: // OR
                    if (MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF();
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        INST_NAME("LOCK OR Eb, Ib");
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x5, x1, &fixedaddress, rex, LOCK_LOCK, 0, 1);
                        u8 = F8;
                        ANDI(x2, wback, 3);
                        SLLI_D(x2, x2, 3); // offset in bits
                        MV(x3, wback);     // aligned addr
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
                case 4: // AND
                    if (MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF();
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        INST_NAME("LOCK AND Eb, Ib");
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x5, x1, &fixedaddress, rex, LOCK_LOCK, 0, 1);
                        u8 = F8;
                        ANDI(x2, wback, 3);
                        SLLI_D(x2, x2, 3);
                        MV(x3, wback);
                        BSTRINS_D(x3, xZR, 1, 0);
                        ADDI_D(x1, xZR, u8);
                        SLL_D(x1, x1, x2);
                        AMAND_DB_W(x4, x1, x3);
                        IFXORNAT (X_ALL | X_PEND) {
                            SRL_D(x1, x4, x2);
                            ANDI(x1, x1, 0xFF);
                            emit_and8c(dyn, ninst, x1, u8, x2, x4);
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
                    if (MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF();
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        if (opcode == 0x81) {
                            INST_NAME("LOCK ADD Ed, Id");
                        } else {
                            INST_NAME("LOCK ADD Ed, Ib");
                        }
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, (opcode == 0x81) ? 4 : 1);
                        if (opcode == 0x81)
                            i64 = F32S;
                        else
                            i64 = F8S;
                        MOV64xw(x7, i64);
                        if (rex.w) {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b111);
                                BNEZ_MARK2(x3);
                            }
                            AMADD_DB_D(x1, x7, wback);
                            if (!ALIGNED_ATOMICxw) { B_MARK3_nocond; }
                        } else {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b11);
                                BNEZ_MARK(x3);
                            }
                            // aligned 4byte
                            AMADD_DB_W(x1, x7, wback);
                            if (!ALIGNED_ATOMICxw) {
                                B_MARK3_nocond;
                                MARK;
                                ANDI(x3, wback, 0b111);
                                SLTI(x4, x3, 4);
                                BEQZ_MARK2(x4); // addr %8 >4 , cross 8bytes or cross cacheline
                                LOCK_32_IN_8BYTE(ADD_W(x4, x1, x7), x1, wback, x3, x4, x5, x6);
                                B_MARK3_nocond;
                            }
                        }
                        if (!ALIGNED_ATOMICxw) {
                            MARK2;
                            LOCK_3264_CROSS_8BYTE(ADDxw(x4, x1, x7), x1, wback, x4, x5, x6);
                            MARK3;
                        }
                        IFXORNAT (X_ALL | X_PEND) {
                            emit_add32c(dyn, ninst, rex, x1, i64, x3, x4, x5, x6);
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
                        if (opcode == 0x81) {
                            INST_NAME("LOCK OR Ed, Id");
                        } else {
                            INST_NAME("LOCK OR Ed, Ib");
                        }
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, (opcode == 0x81) ? 4 : 1);
                        if (opcode == 0x81) {
                            i64 = F32S;
                        } else {
                            i64 = F8S;
                        }
                        MOV64xw(x7, i64);
                        if (rex.w) {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b111);
                                BNEZ_MARK2(x3);
                            }
                            AMOR_DB_D(x1, x7, wback);
                            if (!ALIGNED_ATOMICxw) { B_MARK3_nocond; }
                        } else {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b11);
                                BNEZ_MARK(x3);
                            }
                            // aligned 4byte
                            AMOR_DB_W(x1, x7, wback);
                            if (!ALIGNED_ATOMICxw) {
                                B_MARK3_nocond;
                                MARK;
                                ANDI(x3, wback, 0b111);
                                SLTI(x4, x3, 4);
                                BEQZ_MARK2(x4); // addr %8 >4 , cross 8bytes or cross cacheline
                                LOCK_32_IN_8BYTE(OR(x4, x1, x7), x1, wback, x3, x4, x5, x6);
                                B_MARK3_nocond;
                            }
                        }
                        if (!ALIGNED_ATOMICxw) {
                            MARK2;
                            LOCK_3264_CROSS_8BYTE(OR(x4, x1, x7), x1, wback, x4, x5, x6);
                            MARK3;
                        }
                        IFXORNAT (X_ALL | X_PEND)
                            emit_or32c(dyn, ninst, rex, x1, i64, x3, x4);
                    }
                    break;
                case 2:
                    if (MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF();
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        if (opcode == 0x81) {
                            INST_NAME("LOCK ADC Ed, Id");
                        } else {
                            INST_NAME("LOCK ADC Ed, Ib");
                        }
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, (opcode == 0x81) ? 4 : 1);
                        if (cpuext.lbt) {
                            X64_GET_EFLAGS(x6, X_CF);
                        } else {
                            BSTRPICK_D(x6, xFlags, F_CF, F_CF);
                        }
                        if (opcode == 0x81)
                            i64 = F32S;
                        else
                            i64 = F8S;
                        MOV64xw(x7, i64);
                        ADDxw(x7, x7, x6);
                        if (rex.w) {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b111);
                                BNEZ_MARK2(x3);
                            }
                            AMADD_DB_D(x1, x7, wback);
                            if (!ALIGNED_ATOMICxw) { B_MARK3_nocond; }
                        } else {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b11);
                                BNEZ_MARK(x3);
                            }
                            // aligned 4byte
                            AMADD_DB_W(x1, x7, wback);
                            if (!ALIGNED_ATOMICxw) {
                                B_MARK3_nocond;
                                MARK;
                                ANDI(x3, wback, 0b111);
                                SLTI(x4, x3, 4);
                                BEQZ_MARK2(x4); // addr %8 >4 , cross 8bytes or cross cacheline
                                LOCK_32_IN_8BYTE(ADD_W(x4, x1, x7), x1, wback, x3, x4, x5, x6);
                                B_MARK3_nocond;
                            }
                        }
                        if (!ALIGNED_ATOMICxw) {
                            MARK2;
                            LOCK_3264_CROSS_8BYTE(ADDxw(x4, x1, x7), x1, wback, x4, x5, x6);
                            MARK3;
                        }
                        IFXORNAT (X_ALL | X_PEND) {
                            MOV64xw(x7, i64);
                            emit_adc32(dyn, ninst, rex, x1, x7, x3, x4, x5, x6);
                        }
                    }
                    break;
                case 3:
                    if (MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF();
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        if (opcode == 0x81) {
                            INST_NAME("LOCK SBB Ed, Id");
                        } else {
                            INST_NAME("LOCK SBB Ed, Ib");
                        }
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, (opcode == 0x81) ? 4 : 1);
                        if (cpuext.lbt) {
                            X64_GET_EFLAGS(x6, X_CF);
                        } else {
                            BSTRPICK_D(x6, xFlags, F_CF, F_CF);
                        }
                        if (opcode == 0x81)
                            i64 = F32S;
                        else
                            i64 = F8S;
                        MOV64xw(x7, -i64);
                        SUBxw(x7, x7, x6);
                        if (rex.w) {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b111);
                                BNEZ_MARK2(x3);
                            }
                            AMADD_DB_D(x1, x7, wback);
                            if (!ALIGNED_ATOMICxw) { B_MARK3_nocond; }
                        } else {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b11);
                                BNEZ_MARK(x3);
                            }
                            // aligned 4byte
                            AMADD_DB_W(x1, x7, wback);
                            if (!ALIGNED_ATOMICxw) {
                                B_MARK3_nocond;
                                MARK;
                                ANDI(x3, wback, 0b111);
                                SLTI(x4, x3, 4);
                                BEQZ_MARK2(x4); // addr %8 >4 , cross 8bytes or cross cacheline
                                LOCK_32_IN_8BYTE(ADD_W(x4, x1, x7), x1, wback, x3, x4, x5, x6);
                                B_MARK3_nocond;
                            }
                        }
                        if (!ALIGNED_ATOMICxw) {
                            MARK2;
                            LOCK_3264_CROSS_8BYTE(ADDxw(x4, x1, x7), x1, wback, x4, x5, x6);
                            MARK3;
                        }
                        IFXORNAT (X_ALL | X_PEND) {
                            MOV64xw(x7, i64);
                            emit_sbb32(dyn, ninst, rex, x1, x7, x3, x4, x5);
                        }
                    }
                    break;
                case 4:
                    if (MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF();
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        if (opcode == 0x81) {
                            INST_NAME("LOCK AND Ed, Id");
                        } else {
                            INST_NAME("LOCK AND Ed, Ib");
                        }
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, (opcode == 0x81) ? 4 : 1);
                        if (opcode == 0x81)
                            i64 = F32S;
                        else
                            i64 = F8S;
                        MOV64xw(x7, i64);
                        if (rex.w) {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b111);
                                BNEZ_MARK2(x3);
                            }
                            AMAND_DB_D(x1, x7, wback);
                            if (!ALIGNED_ATOMICxw) { B_MARK3_nocond; }
                        } else {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b11);
                                BNEZ_MARK(x3);
                            }
                            // aligned 4byte
                            AMAND_DB_W(x1, x7, wback);
                            if (!ALIGNED_ATOMICxw) {
                                B_MARK3_nocond;
                                MARK;
                                ANDI(x3, wback, 0b111);
                                SLTI(x4, x3, 4);
                                BEQZ_MARK2(x4); // addr %8 >4 , cross 8bytes or cross cacheline
                                LOCK_32_IN_8BYTE(AND(x4, x1, x7), x1, wback, x3, x4, x5, x6);
                                B_MARK3_nocond;
                            }
                        }
                        if (!ALIGNED_ATOMICxw) {
                            MARK2;
                            LOCK_3264_CROSS_8BYTE(AND(x4, x1, x7), x1, wback, x4, x5, x6);
                            MARK3;
                        }
                        IFXORNAT (X_ALL | X_PEND)
                            emit_and32c(dyn, ninst, rex, x1, i64, x3, x4);
                    }
                    break;
                case 5:
                    if (MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF();
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        if (opcode == 0x81) {
                            INST_NAME("LOCK SUB Ed, Id");
                        } else {
                            INST_NAME("LOCK SUB Ed, Ib");
                        }
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, (opcode == 0x81) ? 4 : 1);
                        if (opcode == 0x81)
                            i64 = F32S;
                        else
                            i64 = F8S;
                        MOV64xw(x7, -i64);
                        if (rex.w) {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b111);
                                BNEZ_MARK2(x3);
                            }
                            AMADD_DB_D(x1, x7, wback);
                            if (!ALIGNED_ATOMICxw) { B_MARK3_nocond; }
                        } else {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b11);
                                BNEZ_MARK(x3);
                            }
                            // aligned 4byte
                            AMADD_DB_W(x1, x7, wback);
                            if (!ALIGNED_ATOMICxw) {
                                B_MARK3_nocond;
                                MARK;
                                ANDI(x3, wback, 0b111);
                                SLTI(x4, x3, 4);
                                BEQZ_MARK2(x4); // addr %8 >4 , cross 8bytes or cross cacheline
                                LOCK_32_IN_8BYTE(ADD_W(x4, x1, x7), x1, wback, x3, x4, x5, x6);
                                B_MARK3_nocond;
                            }
                        }
                        if (!ALIGNED_ATOMICxw) {
                            MARK2;
                            LOCK_3264_CROSS_8BYTE(ADDxw(x4, x1, x7), x1, wback, x4, x5, x6);
                            MARK3;
                        }
                        IFXORNAT (X_ALL | X_PEND)
                            emit_sub32c(dyn, ninst, rex, x1, i64, x3, x4, x5, x6);
                    }
                    break;
                case 6:
                    if (MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF();
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        if (opcode == 0x81) {
                            INST_NAME("LOCK XOR Ed, Id");
                        } else {
                            INST_NAME("LOCK XOR Ed, Ib");
                        }
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, (opcode == 0x81) ? 4 : 1);
                        if (opcode == 0x81)
                            i64 = F32S;
                        else
                            i64 = F8S;
                        MOV64xw(x7, i64);
                        if (rex.w) {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b111);
                                BNEZ_MARK2(x3);
                            }
                            AMXOR_DB_D(x1, x7, wback);
                            if (!ALIGNED_ATOMICxw) { B_MARK3_nocond; };
                        } else {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b11);
                                BNEZ_MARK(x3);
                            }
                            // aligned 4byte
                            AMXOR_DB_W(x1, x7, wback);
                            if (!ALIGNED_ATOMICxw) {
                                B_MARK3_nocond;
                                MARK;
                                ANDI(x3, wback, 0b111);
                                SLTI(x4, x3, 4);
                                BEQZ_MARK2(x4); // addr %8 >4 , cross 8bytes or cross cacheline
                                LOCK_32_IN_8BYTE(XOR(x4, x1, x7), x1, wback, x3, x4, x5, x6);
                                B_MARK3_nocond;
                            }
                        }
                        if (!ALIGNED_ATOMICxw) {
                            MARK2;
                            LOCK_3264_CROSS_8BYTE(XOR(x4, x1, x7), x1, wback, x4, x5, x6);
                            MARK3;
                        }
                        IFXORNAT (X_ALL | X_PEND)
                            emit_xor32c(dyn, ninst, rex, x1, i64, x3, x4);
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0x87:
            nextop = F8;
            if (MODREG) {
                INST_NAME("Invalid LOCK");
                UDF();
                *need_epilog = 1;
                *ok = 0;
            } else {
                INST_NAME("LOCK XCHG Ed, Gd");
                GETGD;
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                if (rex.w) {
                    if (!ALIGNED_ATOMICxw) {
                        ANDI(x3, wback, 0b111);
                        BNEZ_MARK2(x3);
                    }
                    AMSWAP_DB_D(x1, gd, wback);
                    if (!ALIGNED_ATOMICxw) { B_MARK3_nocond; }
                } else {
                    if (!ALIGNED_ATOMICxw) {
                        ANDI(x3, wback, 0b11);
                        BNEZ_MARK(x3);
                    }
                    // aligned 4byte
                    AMSWAP_DB_W(x1, gd, wback);
                    if (!ALIGNED_ATOMICxw) {
                        B_MARK3_nocond;
                        MARK;
                        ANDI(x3, wback, 0b111);
                        SLTI(x4, x3, 4);
                        BEQZ_MARK2(x4); // addr %8 >4 , cross 8bytes or cross cacheline
                        LOCK_32_IN_8BYTE(ADDI_W(x4, gd, 0), x1, wback, x3, x4, x5, x6);
                        B_MARK3_nocond;
                    }
                }
                if (!ALIGNED_ATOMICxw) {
                    MARK2;
                    LOCK_3264_CROSS_8BYTE(ADDI_D(x4, gd, 0), x1, wback, x4, x5, x6);
                    MARK3;
                }
                MVxw(gd, x1);
            }
            break;
        case 0xF7:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 2:
                    if (MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF();
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        INST_NAME("LOCK NOT Ed");
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                        MOV64xw(x7, -1);
                        if (rex.w) {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b111);
                                BNEZ_MARK2(x3);
                            }
                            AMXOR_DB_D(x1, x7, wback);
                            if (!ALIGNED_ATOMICxw) { B_MARK3_nocond; }
                        } else {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b11);
                                BNEZ_MARK(x3);
                            }
                            // aligned 4byte
                            AMXOR_DB_W(x1, x7, wback);
                            if (!ALIGNED_ATOMICxw) {
                                B_MARK3_nocond;
                                MARK;
                                ANDI(x3, wback, 0b111);
                                SLTI(x4, x3, 4);
                                BEQZ_MARK2(x4); // addr %8 >4 , cross 8bytes or cross cacheline
                                LOCK_32_IN_8BYTE(XOR(x4, x1, x7), x1, wback, x3, x4, x5, x6);
                                B_MARK3_nocond;
                            }
                        }
                        if (!ALIGNED_ATOMICxw) {
                            MARK2;
                            LOCK_3264_CROSS_8BYTE(XOR(x4, x1, x7), x1, wback, x4, x5, x6);
                            MARK3;
                        }
                    }
                    break;
                case 3:
                    if (MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF();
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        INST_NAME("LOCK NEG Ed");
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                        if (rex.w) {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b111);
                                BNEZ_MARK2(x3);
                            }
                            if (cpuext.lamcas) {
                                LD_D(x5, wback, 0);
                                SUB_D(x4, xZR, x5);
                                MV(x6, x5);
                                AMCAS_DB_D(x5, x4, wback);
                                BNE(x6, x5, -4 * 3);
                            } else {
                                LL_D(x1, wback, 0);
                                SUB_D(x4, xZR, x1);
                                SC_D(x4, wback, 0);
                                BEQZ(x4, -4 * 3);
                            }
                            if (!ALIGNED_ATOMICxw) { B_MARK3_nocond; }
                        } else {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b11);
                                BNEZ_MARK(x3);
                            }
                            // aligned 4byte
                            if (cpuext.lamcas) {
                                LD_WU(x5, wback, 0);
                                SUB_W(x4, xZR, x5);
                                MV(x6, x5);
                                AMCAS_DB_W(x5, x4, wback);
                                BNE(x6, x5, -4 * 3);
                            } else {
                                LL_W(x1, wback, 0);
                                SUB_W(x4, xZR, x1);
                                SC_W(x4, wback, 0);
                                BEQZ(x4, -4 * 3);
                            }
                            if (!ALIGNED_ATOMICxw) {
                                B_MARK3_nocond;
                                MARK;
                                ANDI(x3, wback, 0b111);
                                SLTI(x4, x3, 4);
                                BEQZ_MARK2(x4); // addr %8 >4 , cross 8bytes or cross cacheline
                                LOCK_32_IN_8BYTE(SUB_W(x4, xZR, x1), x1, wback, x3, x4, x5, x6);
                                B_MARK3_nocond;
                            }
                        }
                        if (!ALIGNED_ATOMICxw) {
                            MARK2;
                            LOCK_3264_CROSS_8BYTE(SUBxw(x4, xZR, x1), x1, wback, x4, x5, x6);
                            MARK3;
                        }
                        IFXORNAT (X_ALL | X_PEND) {
                            emit_neg32(dyn, ninst, rex, x1, x3, x4);
                        }
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xFE:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0: // INC Eb
                    if (MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF();
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        INST_NAME("LOCK INC Eb");
                        SETFLAGS(X_ALL & ~X_CF, SF_SUBSET_PENDING, NAT_FLAGS_FUSION);
                        MOV64x(x7, 1);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                        if (cpuext.lam_bh) {
                            AMADD_DB_B(x1, x7, wback);
                        } else {
                            LOCK_8_OP(ADD_D(x4, x1, x7), x1, wback, x3, x4, x5, x6);
                        }
                        IFXORNAT (X_ALL | X_PEND) {
                            emit_inc8(dyn, ninst, x1, x3, x4, x5);
                        }
                    }
                    break;
                case 1: // DEC Eb
                    if (MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF();
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        INST_NAME("LOCK DEC Eb");
                        SETFLAGS(X_ALL & ~X_CF, SF_SUBSET_PENDING, NAT_FLAGS_FUSION);
                        MOV64x(x7, -1);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                        if (cpuext.lam_bh) {
                            AMADD_DB_B(x1, x7, wback);
                        } else {
                            LOCK_8_OP(ADD_D(x4, x1, x7), x1, wback, x3, x4, x5, x6);
                        }
                        IFXORNAT (X_ALL | X_PEND) {
                            emit_dec8(dyn, ninst, x1, x3, x4, x5);
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
                        INST_NAME("LOCK INC Ed");
                        SETFLAGS(X_ALL & ~X_CF, SF_SUBSET_PENDING, NAT_FLAGS_FUSION);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                        if (rex.w) {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b111);
                                BNEZ_MARK2(x3);
                            }
                            MOV32w(x7, 1);
                            AMADD_DB_D(x1, x7, wback);
                            if (!ALIGNED_ATOMICxw) { B_MARK3_nocond; }
                        } else {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b11);
                                BNEZ_MARK(x3);
                            }
                            // aligned 4byte
                            MOV32w(x7, 1);
                            AMADD_DB_W(x1, x7, wback);
                            if (!ALIGNED_ATOMICxw) {
                                B_MARK3_nocond;
                                MARK;
                                ANDI(x3, wback, 0b111);
                                SLTI(x4, x3, 4);
                                BEQZ_MARK2(x4); // addr %8 >4 , cross 8bytes or cross cacheline
                                LOCK_32_IN_8BYTE(ADDI_W(x4, x1, 1), x1, wback, x3, x4, x5, x6);
                                B_MARK3_nocond;
                            }
                        }
                        if (!ALIGNED_ATOMICxw) {
                            MARK2;
                            LOCK_3264_CROSS_8BYTE(ADDIxw(x4, x1, 1), x1, wback, x4, x5, x6);
                            MARK3;
                        }
                        IFXORNAT (X_ALL | X_PEND)
                            emit_inc32(dyn, ninst, rex, x1, x3, x4, x5, x6);
                    }
                    break;
                case 1:
                    if (MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF();
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        INST_NAME("LOCK DEC Ed");
                        SETFLAGS(X_ALL & ~X_CF, SF_SUBSET_PENDING, NAT_FLAGS_FUSION);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, 0, 0);
                        if (rex.w) {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b111);
                                BNEZ_MARK2(x3);
                            }
                            MOV64xw(x7, -1);
                            AMADD_DB_D(x1, x7, wback);
                            if (!ALIGNED_ATOMICxw) { B_MARK3_nocond; }
                        } else {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b11);
                                BNEZ_MARK(x3);
                            }
                            // aligned 4byte
                            MOV64xw(x7, -1);
                            AMADD_DB_W(x1, x7, wback);
                            if (!ALIGNED_ATOMICxw) {
                                B_MARK3_nocond;
                                MARK;
                                ANDI(x3, wback, 0b111);
                                SLTI(x4, x3, 4);
                                BEQZ_MARK2(x4); // addr %8 >4 , cross 8bytes or cross cacheline
                                LOCK_32_IN_8BYTE(ADDI_W(x4, x1, -1), x1, wback, x3, x4, x5, x6);
                                B_MARK3_nocond;
                            }
                        }
                        if (!ALIGNED_ATOMICxw) {
                            MARK2;
                            LOCK_3264_CROSS_8BYTE(ADDIxw(x4, x1, -1), x1, wback, x4, x5, x6);
                            MARK3;
                        }
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
