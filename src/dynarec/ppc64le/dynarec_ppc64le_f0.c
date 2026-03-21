#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "ppc64le_emitter.h"
#include "ppc64le_mapping.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"

#include "ppc64le_printer.h"
#include "dynarec_ppc64le_private.h"
#include "../dynarec_helper.h"
#include "dynarec_ppc64le_functions.h"


uintptr_t dynarec64_F0(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
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
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, 0);
                // PPC64LE has byte-level LBARX/STBCXd
                LOCK_8_OP(ADD(x4, x1, gd), x1, wback, x3, x4, x5, x6);
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
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, 0);
                if (rex.w) {
                    // 64-bit atomic add
                    if (!ALIGNED_ATOMICxw) {
                        ANDI(x3, wback, 0b111);
                        BNEZ_MARK2(x3);
                    }
                    LWSYNC();
                    MARKLOCK;
                    LDARX(x1, 0, wback);
                    ADD(x4, x1, gd);
                    STDCXd(x4, 0, wback);
                    BNE_MARKLOCK_CR0;
                    LWSYNC();
                    if (!ALIGNED_ATOMICxw) { B_MARK3_nocond; }
                } else {
                    // 32-bit atomic add
                    if (!ALIGNED_ATOMICxw) {
                        ANDI(x3, wback, 0b11);
                        BNEZ_MARK(x3);
                    }
                    LWSYNC();
                    MARKLOCK;
                    LWARX(x1, 0, wback);
                    ADD(x4, x1, gd);
                    RLDICL(x4, x4, 0, 32); // zero upper 32 bits
                    STWCXd(x4, 0, wback);
                    BNE_MARKLOCK_CR0;
                    LWSYNC();
                    if (!ALIGNED_ATOMICxw) {
                        B_MARK3_nocond;
                        MARK;
                        ANDI(x3, wback, 0b111);
                        CMPWI(x3, 4);
                        BGE_MARK2(x3, x3); // actually need BLT, but check if offset >= 5 crosses boundary
                        // Within 8-byte block
                        LOCK_32_IN_8BYTE(ADD(x4, x1, gd), x1, wback, x3, x4, x5, x6);
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
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, 0);
                LOCK_8_OP(OR(x4, x1, gd), x1, wback, x3, x4, x5, x6);
                IFXORNAT (X_ALL | X_PEND) {
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
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, 0);
                if (rex.w) {
                    if (!ALIGNED_ATOMICxw) {
                        ANDI(x3, wback, 0b111);
                        BNEZ_MARK2(x3);
                    }
                    LWSYNC();
                    MARKLOCK;
                    LDARX(x1, 0, wback);
                    OR(x4, x1, gd);
                    STDCXd(x4, 0, wback);
                    BNE_MARKLOCK_CR0;
                    LWSYNC();
                    if (!ALIGNED_ATOMICxw) { B_MARK3_nocond; }
                } else {
                    if (!ALIGNED_ATOMICxw) {
                        ANDI(x3, wback, 0b11);
                        BNEZ_MARK(x3);
                    }
                    LWSYNC();
                    MARKLOCK;
                    LWARX(x1, 0, wback);
                    OR(x4, x1, gd);
                    STWCXd(x4, 0, wback);
                    BNE_MARKLOCK_CR0;
                    LWSYNC();
                    if (!ALIGNED_ATOMICxw) {
                        B_MARK3_nocond;
                        MARK;
                        ANDI(x3, wback, 0b111);
                        CMPWI(x3, 4);
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
            opcode = F8;
            switch(opcode) {

                case 0xAB:
                    nextop = F8;
                    if (MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF();
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        INST_NAME("LOCK BTS Ed, Gd");
                        SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                        GETGD;
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, 0);
                        // Compute bit position and byte offset
                        if (rex.w) {
                            // 64-bit: bit offset = gd (can be > 63, so address adjusted)
                            SRADI(x3, gd, 6);    // qword offset
                            SLDI(x3, x3, 3);     // byte offset
                            ADD(x6, wback, x3);  // adjusted address
                            ANDI(x5, gd, 63);    // bit within qword
                            LI(x3, 1);
                            SLD(x3, x3, x5);     // mask
                            LWSYNC();
                            MARKLOCK;
                            LDARX(x1, 0, x6);
                            OR(x4, x1, x3);
                            STDCXd(x4, 0, x6);
                            BNE_MARKLOCK_CR0;
                            LWSYNC();
                        } else {
                            SRAWI(x3, gd, 5);    // dword offset
                            SLDI(x3, x3, 2);     // byte offset
                            ADD(x6, wback, x3);  // adjusted address
                            ANDI(x5, gd, 31);    // bit within dword
                            LI(x3, 1);
                            SLW(x3, x3, x5);     // mask
                            LWSYNC();
                            MARKLOCK;
                            LWARX(x1, 0, x6);
                            OR(x4, x1, x3);
                            STWCXd(x4, 0, x6);
                            BNE_MARKLOCK_CR0;
                            LWSYNC();
                        }
                        // Set CF from old value
                        IFXORNAT (X_CF) {
                            SRD(x1, x1, x5);
                            ANDI(x1, x1, 1);
                            IFX (X_CF) {
                                BF_INSERT(xFlags, x1, F_CF, F_CF);
                            }
                        }
                    }
                    break;

                case 0xB0:
                    nextop = F8;
                    if (MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF();
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        INST_NAME("LOCK CMPXCHG Eb, Gb");
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_NOFUSION);
                        GETGB(x7);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, 0);
                        // AL = expected; compare [wback] with AL
                        BF_EXTRACT(x5, xRAX, 7, 0); // x5 = AL
                        LWSYNC();
                        MARKLOCK;
                        LBARX(x1, 0, wback);         // x1 = current byte
                        BF_EXTRACT(x3, x1, 7, 0);    // x3 = current (zero-extended)
                        CMPW(x3, x5);
                        BNE(3*4);                     // skip store+branch if not equal
                        STBCXd(gd, 0, wback);         // store Gb
                        BNE_MARKLOCK_CR0;
                        LWSYNC();
                        // At this point: x3 = old value, x5 = AL
                        // If equal: [wback] = Gb, ZF=1
                        // If not equal: AL = old value, ZF=0
                        CMPW(x3, x5);
                        BEQ(2*4);                      // skip if equal (already correct)
                        BF_INSERT(xRAX, x3, 7, 0);   // AL = old value
                        // Emit flags from CMP(old, AL)
                        IFXORNAT (X_ALL | X_PEND) {
                            emit_cmp8(dyn, ninst, x3, x5, x4, x6, x1, x2);
                        }
                    }
                    break;

                case 0xB1:
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
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, 0);
                        // EAX/RAX = expected
                        MVxw(x5, xRAX);
                        if (rex.w) {
                            LWSYNC();
                            MARKLOCK;
                            LDARX(x1, 0, wback);
                            CMPD(x1, x5);
                            BNE(3*4);               // skip store+branch if not equal
                            STDCXd(gd, 0, wback);
                            BNE_MARKLOCK_CR0;
                            LWSYNC();
                        } else {
                            LWSYNC();
                            MARKLOCK;
                            LWARX(x1, 0, wback);
                            CMPW(x1, x5);
                            BNE(3*4);               // skip store+branch
                            STWCXd(gd, 0, wback);
                            BNE_MARKLOCK_CR0;
                            LWSYNC();
                        }
                        // x1 = old value, x5 = expected (EAX/RAX)
                        if (rex.w) CMPD(x1, x5); else CMPW(x1, x5);
                        BEQ(2*4);                    // skip if equal
                        MVxw(xRAX, x1);              // RAX/EAX = old value
                        IFXORNAT (X_ALL | X_PEND) {
                            emit_cmp32(dyn, ninst, rex, x1, x5, x3, x4, x6, x7);
                        }
                    }
                    break;

                case 0xB3:
                    nextop = F8;
                    if (MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF();
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        INST_NAME("LOCK BTR Ed, Gd");
                        SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                        GETGD;
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, 0);
                        if (rex.w) {
                            SRADI(x3, gd, 6);
                            SLDI(x3, x3, 3);
                            ADD(x6, wback, x3);
                            ANDI(x5, gd, 63);
                            LI(x3, 1);
                            SLD(x3, x3, x5);    // mask
                            NOT(x3, x3);         // inverted mask
                            LWSYNC();
                            MARKLOCK;
                            LDARX(x1, 0, x6);
                            AND(x4, x1, x3);
                            STDCXd(x4, 0, x6);
                            BNE_MARKLOCK_CR0;
                            LWSYNC();
                        } else {
                            SRAWI(x3, gd, 5);
                            SLDI(x3, x3, 2);
                            ADD(x6, wback, x3);
                            ANDI(x5, gd, 31);
                            LI(x3, 1);
                            SLW(x3, x3, x5);
                            NOT(x3, x3);
                            LWSYNC();
                            MARKLOCK;
                            LWARX(x1, 0, x6);
                            AND(x4, x1, x3);
                            STWCXd(x4, 0, x6);
                            BNE_MARKLOCK_CR0;
                            LWSYNC();
                        }
                        IFXORNAT (X_CF) {
                            SRD(x1, x1, x5);
                            ANDI(x1, x1, 1);
                            IFX (X_CF) {
                                BF_INSERT(xFlags, x1, F_CF, F_CF);
                            }
                        }
                    }
                    break;

                case 0xBA:
                    nextop = F8;
                    switch ((nextop >> 3) & 7) {
                        case 4:
                            INST_NAME("Invalid LOCK BT");
                            UDF();
                            *need_epilog = 1;
                            *ok = 0;
                            break;
                        case 5:
                            if (MODREG) {
                                INST_NAME("Invalid LOCK");
                                UDF();
                                *need_epilog = 1;
                                *ok = 0;
                            } else {
                                INST_NAME("LOCK BTS Ed, Ib");
                                SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, 0);
                                u8 = F8;
                                u8 = rex.w ? (u8 & 63) : (u8 & 31);
                                LI(x3, 1);
                                if (rex.w) {
                                    SLDI(x3, x3, u8);
                                    LWSYNC();
                                    MARKLOCK;
                                    LDARX(x1, 0, wback);
                                    OR(x4, x1, x3);
                                    STDCXd(x4, 0, wback);
                                    BNE_MARKLOCK_CR0;
                                    LWSYNC();
                                } else {
                                    SLWI(x3, x3, u8);
                                    LWSYNC();
                                    MARKLOCK;
                                    LWARX(x1, 0, wback);
                                    OR(x4, x1, x3);
                                    STWCXd(x4, 0, wback);
                                    BNE_MARKLOCK_CR0;
                                    LWSYNC();
                                }
                                IFXORNAT (X_CF) {
                                    if (rex.w) SRDI(x1, x1, u8); else SRWI(x1, x1, u8);
                                    ANDI(x1, x1, 1);
                                    IFX (X_CF) {
                                        BF_INSERT(xFlags, x1, F_CF, F_CF);
                                    }
                                }
                            }
                            break;
                        case 6:
                            if (MODREG) {
                                INST_NAME("Invalid LOCK");
                                UDF();
                                *need_epilog = 1;
                                *ok = 0;
                            } else {
                                INST_NAME("LOCK BTR Ed, Ib");
                                SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, 0);
                                u8 = F8;
                                u8 = rex.w ? (u8 & 63) : (u8 & 31);
                                LI(x3, 1);
                                if (rex.w) {
                                    SLDI(x3, x3, u8);
                                    NOT(x3, x3);
                                    LWSYNC();
                                    MARKLOCK;
                                    LDARX(x1, 0, wback);
                                    AND(x4, x1, x3);
                                    STDCXd(x4, 0, wback);
                                    BNE_MARKLOCK_CR0;
                                    LWSYNC();
                                } else {
                                    SLWI(x3, x3, u8);
                                    NOT(x3, x3);
                                    LWSYNC();
                                    MARKLOCK;
                                    LWARX(x1, 0, wback);
                                    AND(x4, x1, x3);
                                    STWCXd(x4, 0, wback);
                                    BNE_MARKLOCK_CR0;
                                    LWSYNC();
                                }
                                IFXORNAT (X_CF) {
                                    if (rex.w) SRDI(x1, x1, u8); else SRWI(x1, x1, u8);
                                    ANDI(x1, x1, 1);
                                    IFX (X_CF) {
                                        BF_INSERT(xFlags, x1, F_CF, F_CF);
                                    }
                                }
                            }
                            break;
                        default:
                            DEFAULT;
                    }
                    break;

                case 0xC0:
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
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, 0);
                        // Atomic: old = [wback]; [wback] = old + Gb; Gb = old
                        LOCK_8_OP(ADD(x4, x1, gd), x1, wback, x3, x4, x5, x6);
                        // x1 = old value; store old value back to Gb
                        BF_INSERT(gb1, x1, gb2 + 7, gb2);
                        IFXORNAT (X_ALL | X_PEND) {
                            emit_add8(dyn, ninst, x1, gd, x3, x4);
                        }
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
                        INST_NAME("LOCK XADD Ed, Gd");
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                        GETGD;
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, 0);
                        // Atomic exchange-add: old = [wback]; [wback] = old + Gd; Gd = old
                        if (rex.w) {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b111);
                                BNEZ_MARK2(x3);
                            }
                            LWSYNC();
                            MARKLOCK;
                            LDARX(x1, 0, wback);
                            ADD(x4, x1, gd);
                            STDCXd(x4, 0, wback);
                            BNE_MARKLOCK_CR0;
                            LWSYNC();
                            if (!ALIGNED_ATOMICxw) { B_MARK3_nocond; }
                        } else {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b11);
                                BNEZ_MARK(x3);
                            }
                            LWSYNC();
                            MARKLOCK;
                            LWARX(x1, 0, wback);
                            ADD(x4, x1, gd);
                            RLDICL(x4, x4, 0, 32);
                            STWCXd(x4, 0, wback);
                            BNE_MARKLOCK_CR0;
                            LWSYNC();
                            if (!ALIGNED_ATOMICxw) {
                                B_MARK3_nocond;
                                MARK;
                                LOCK_32_IN_8BYTE(ADD(x4, x1, gd), x1, wback, x3, x4, x5, x6);
                                B_MARK3_nocond;
                            }
                        }
                        if (!ALIGNED_ATOMICxw) {
                            MARK2;
                            LOCK_3264_CROSS_8BYTE(ADDxw(x4, x1, gd), x1, wback, x4, x5, x6);
                            MARK3;
                        }
                        // Gd = old value
                        MVxw(gd, x1);
                        IFXORNAT (X_ALL | X_PEND) {
                            emit_add32(dyn, ninst, rex, x1, gd, x3, x4, x5);
                        }
                    }
                    break;

                case 0xC7:
                    nextop = F8;
                    switch ((nextop >> 3) & 7) {
                        case 1:
                            if (MODREG) {
                                INST_NAME("Invalid LOCK");
                                UDF();
                                *need_epilog = 1;
                                *ok = 0;
                            } else if (rex.w) {
                                INST_NAME("LOCK CMPXCHG16B Gq, Eq");
                                static int warned = 0;
                                PASS3(if (!warned) dynarec_log(LOG_INFO, "Warning, LOCK CMPXCHG16B is not well supported on PPC64LE and issues are expected.\n"));
                                PASS3(warned = 1);
                                SETFLAGS(X_ZF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                                SKIPTEST(x1);
                                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, 0);
                                // RDX:RAX = expected, RCX:RBX = replacement
                                // PPC64LE (pre-POWER10) lacks 128-bit LL/SC, so use the
                                // mutex_16b spinlock to protect the critical section, matching
                                // the interpreter and the LA64/RV64 fallback paths.
                                RLWINM(xFlags, xFlags, 0, 32-F_ZF, 30-F_ZF); // clear ZF
                                // Acquire mutex_16b spinlock
                                LD(x5, offsetof(x64emu_t, context), xEmu);  // x5 = emu->context
                                ADDI(x5, x5, offsetof(box64context_t, mutex_16b)); // x5 = &mutex_16b
                                LI(x4, 1);
                                MARK2;
                                LWARX(x6, 0, x5);          // x6 = old mutex value
                                BNEZ_MARK2(x6);             // spin if already locked
                                STWCXd(x4, 0, x5);          // try to store 1
                                BNE_MARK2_CR0;              // retry if reservation lost
                                ISYNC();                     // acquire barrier
                                // Under lock: plain loads of 128-bit value
                                LD(x1, 0, wback);           // x1 = low qword
                                LD(x3, 8, wback);           // x3 = high qword
                                // Compare with RDX:RAX
                                BNE_MARK(x1, xRAX);
                                BNE_MARK(x3, xRDX);
                                // Equal: store RCX:RBX
                                STD(xRBX, 0, wback);
                                STD(xRCX, 8, wback);
                                // Set ZF=1
                                ORI(xFlags, xFlags, 1 << F_ZF);
                                B_MARK3_nocond;
                                // Not equal: load old value into RDX:RAX
                                MARK;
                                MV(xRAX, x1);
                                MV(xRDX, x3);
                                MARK3;
                                // Release mutex_16b spinlock
                                LWSYNC();                    // release barrier
                                LI(x6, 0);
                                STW(x6, 0, x5);             // store 0 to unlock
                            } else {
                                INST_NAME("LOCK CMPXCHG8B Gq, Eq");
                                SETFLAGS(X_ZF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                                SKIPTEST(x1);
                                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, 0);
                                // EDX:EAX = expected, ECX:EBX = replacement
                                // [wback] is 8 bytes = one doubleword
                                // x86 low dword = EAX, high dword = EDX
                                // Combine expected into one 64-bit value
                                ZEROUP(x5);          // clear upper bits of temp
                                RLDICL(x5, xRDX, 0, 32);   // x5 = EDX (32-bit)
                                SLDI(x5, x5, 32);
                                RLDICL(x3, xRAX, 0, 32);   // x3 = EAX (32-bit)
                                OR(x5, x5, x3);             // x5 = EDX:EAX combined
                                // Combine replacement
                                RLDICL(x6, xRCX, 0, 32);
                                SLDI(x6, x6, 32);
                                RLDICL(x3, xRBX, 0, 32);
                                OR(x6, x6, x3);             // x6 = ECX:EBX combined
                                LWSYNC();
                                MARKLOCK;
                                LDARX(x1, 0, wback);
                                // Mismatch branches to MARK
                                BNE_MARK(x1, x5);
                                STDCXd(x6, 0, wback);
                                BNE_MARKLOCK_CR0;
                                LWSYNC();
                                // Match: set ZF=1
                                ORI(xFlags, xFlags, 1 << F_ZF);
                                B_MARK3_nocond;              // skip not-equal path
                                // Not equal: load old value into EDX:EAX
                                MARK;
                                RLWINM(xFlags, xFlags, 0, 32-F_ZF, 30-F_ZF);
                                RLDICL(xRAX, x1, 0, 32);           // EAX = low 32 bits
                                SRDI(x1, x1, 32);
                                RLDICL(xRDX, x1, 0, 32);           // EDX = high 32 bits
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
                SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                GETGD;
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, 0);
                RESTORE_EFLAGS(x5);
                // x7 = gd + CF (safe: x7 not clobbered by LOCK macros)
                BF_EXTRACT(x5, xFlags, F_CF, F_CF);
                ADD(x7, gd, x5);  // x7 = gd + CF
                if (rex.w) {
                    if (!ALIGNED_ATOMICxw) {
                        ANDI(x3, wback, 0b111);
                        BNEZ_MARK2(x3);
                    }
                    LWSYNC();
                    MARKLOCK;
                    LDARX(x1, 0, wback);
                    ADD(x4, x1, x7);
                    STDCXd(x4, 0, wback);
                    BNE_MARKLOCK_CR0;
                    LWSYNC();
                    if (!ALIGNED_ATOMICxw) { B_MARK3_nocond; }
                } else {
                    if (!ALIGNED_ATOMICxw) {
                        ANDI(x3, wback, 0b11);
                        BNEZ_MARK(x3);
                    }
                    LWSYNC();
                    MARKLOCK;
                    LWARX(x1, 0, wback);
                    ADD(x4, x1, x7);
                    RLDICL(x4, x4, 0, 32);
                    STWCXd(x4, 0, wback);
                    BNE_MARKLOCK_CR0;
                    LWSYNC();
                    if (!ALIGNED_ATOMICxw) {
                        B_MARK3_nocond;
                        MARK;
                        ANDI(x3, wback, 0b111);
                        CMPWI(x3, 4);
                        BGE_MARK2(x3, x3);
                        LOCK_32_IN_8BYTE(ADD(x4, x1, x7), x1, wback, x3, x4, x5, x6);
                        B_MARK3_nocond;
                    }
                }
                if (!ALIGNED_ATOMICxw) {
                    MARK2;
                    LOCK_3264_CROSS_8BYTE(ADDxw(x4, x1, x7), x1, wback, x4, x5, x6);
                    MARK3;
                }
                IFXORNAT (X_ALL | X_PEND) {
                    emit_add32(dyn, ninst, rex, x1, x7, x3, x4, x6);
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
                SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                GETGD;
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, 0);
                RESTORE_EFLAGS(x5);
                // x7 = gd + CF (safe: x7 not clobbered by LOCK macros)
                BF_EXTRACT(x5, xFlags, F_CF, F_CF);
                ADD(x7, gd, x5);  // x7 = gd + CF
                if (rex.w) {
                    if (!ALIGNED_ATOMICxw) {
                        ANDI(x3, wback, 0b111);
                        BNEZ_MARK2(x3);
                    }
                    LWSYNC();
                    MARKLOCK;
                    LDARX(x1, 0, wback);
                    SUB(x4, x1, x7);
                    STDCXd(x4, 0, wback);
                    BNE_MARKLOCK_CR0;
                    LWSYNC();
                    if (!ALIGNED_ATOMICxw) { B_MARK3_nocond; }
                } else {
                    if (!ALIGNED_ATOMICxw) {
                        ANDI(x3, wback, 0b11);
                        BNEZ_MARK(x3);
                    }
                    LWSYNC();
                    MARKLOCK;
                    LWARX(x1, 0, wback);
                    SUB(x4, x1, x7);
                    RLDICL(x4, x4, 0, 32);
                    STWCXd(x4, 0, wback);
                    BNE_MARKLOCK_CR0;
                    LWSYNC();
                    if (!ALIGNED_ATOMICxw) {
                        B_MARK3_nocond;
                        MARK;
                        ANDI(x3, wback, 0b111);
                        CMPWI(x3, 4);
                        BGE_MARK2(x3, x3);
                        LOCK_32_IN_8BYTE(SUB(x4, x1, x7), x1, wback, x3, x4, x5, x6);
                        B_MARK3_nocond;
                    }
                }
                if (!ALIGNED_ATOMICxw) {
                    MARK2;
                    LOCK_3264_CROSS_8BYTE(SUBxw(x4, x1, x7), x1, wback, x4, x5, x6);
                    MARK3;
                }
                IFXORNAT (X_ALL | X_PEND) {
                    emit_sub32(dyn, ninst, rex, x1, x7, x3, x4, x5);
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
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, 0);
                // For byte AND, we need to AND the byte without affecting neighbors
                // Use LBARX/STBCXd since PPC supports byte-level LL/SC
                LOCK_8_OP(AND(x4, x1, gd), x1, wback, x3, x4, x5, x6);
                IFXORNAT (X_ALL | X_PEND) {
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
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, 0);
                if (rex.w) {
                    if (!ALIGNED_ATOMICxw) {
                        ANDI(x3, wback, 0b111);
                        BNEZ_MARK2(x3);
                    }
                    LWSYNC();
                    MARKLOCK;
                    LDARX(x1, 0, wback);
                    AND(x4, x1, gd);
                    STDCXd(x4, 0, wback);
                    BNE_MARKLOCK_CR0;
                    LWSYNC();
                    if (!ALIGNED_ATOMICxw) { B_MARK3_nocond; }
                } else {
                    if (!ALIGNED_ATOMICxw) {
                        ANDI(x3, wback, 0b11);
                        BNEZ_MARK(x3);
                    }
                    LWSYNC();
                    MARKLOCK;
                    LWARX(x1, 0, wback);
                    AND(x4, x1, gd);
                    STWCXd(x4, 0, wback);
                    BNE_MARKLOCK_CR0;
                    LWSYNC();
                    if (!ALIGNED_ATOMICxw) {
                        B_MARK3_nocond;
                        MARK;
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
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, 0);
                if (rex.w) {
                    if (!ALIGNED_ATOMICxw) {
                        ANDI(x3, wback, 0b111);
                        BNEZ_MARK2(x3);
                    }
                    LWSYNC();
                    MARKLOCK;
                    LDARX(x1, 0, wback);
                    SUB(x4, x1, gd);
                    STDCXd(x4, 0, wback);
                    BNE_MARKLOCK_CR0;
                    LWSYNC();
                    if (!ALIGNED_ATOMICxw) { B_MARK3_nocond; }
                } else {
                    if (!ALIGNED_ATOMICxw) {
                        ANDI(x3, wback, 0b11);
                        BNEZ_MARK(x3);
                    }
                    LWSYNC();
                    MARKLOCK;
                    LWARX(x1, 0, wback);
                    SUB(x4, x1, gd);
                    RLDICL(x4, x4, 0, 32);
                    STWCXd(x4, 0, wback);
                    BNE_MARKLOCK_CR0;
                    LWSYNC();
                    if (!ALIGNED_ATOMICxw) {
                        B_MARK3_nocond;
                        MARK;
                        LOCK_32_IN_8BYTE(SUB(x4, x1, gd), x1, wback, x3, x4, x5, x6);
                        B_MARK3_nocond;
                    }
                }
                if (!ALIGNED_ATOMICxw) {
                    MARK2;
                    LOCK_3264_CROSS_8BYTE(SUBxw(x4, x1, gd), x1, wback, x4, x5, x6);
                    MARK3;
                }
                IFXORNAT (X_ALL | X_PEND) {
                    emit_sub32(dyn, ninst, rex, x1, gd, x3, x4, x5);
                }
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
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, 0);
                if (rex.w) {
                    if (!ALIGNED_ATOMICxw) {
                        ANDI(x3, wback, 0b111);
                        BNEZ_MARK2(x3);
                    }
                    LWSYNC();
                    MARKLOCK;
                    LDARX(x1, 0, wback);
                    XOR(x4, x1, gd);
                    STDCXd(x4, 0, wback);
                    BNE_MARKLOCK_CR0;
                    LWSYNC();
                    if (!ALIGNED_ATOMICxw) { B_MARK3_nocond; }
                } else {
                    if (!ALIGNED_ATOMICxw) {
                        ANDI(x3, wback, 0b11);
                        BNEZ_MARK(x3);
                    }
                    LWSYNC();
                    MARKLOCK;
                    LWARX(x1, 0, wback);
                    XOR(x4, x1, gd);
                    STWCXd(x4, 0, wback);
                    BNE_MARKLOCK_CR0;
                    LWSYNC();
                    if (!ALIGNED_ATOMICxw) {
                        B_MARK3_nocond;
                        MARK;
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
                case 1:
                    if (MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF();
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        INST_NAME("LOCK OR Eb, Ib");
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, 1);
                        u8 = F8;
                        LI(x7, u8);
                        LOCK_8_OP(OR(x4, x1, x7), x1, wback, x3, x4, x5, x6);
                        IFXORNAT (X_ALL | X_PEND) {
                            emit_or8c(dyn, ninst, x1, u8, x3, x4, x5);
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
                        INST_NAME("LOCK AND Eb, Ib");
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, 1);
                        u8 = F8;
                        LI(x7, u8);
                        LOCK_8_OP(AND(x4, x1, x7), x1, wback, x3, x4, x5, x6);
                        IFXORNAT (X_ALL | X_PEND) {
                            emit_and8c(dyn, ninst, x1, u8, x3, x4);
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
                case 0: // LOCK ADD Ed, Id/Ib
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
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, (opcode==0x81)?4:1);
                        if (opcode == 0x81) i64 = F32S; else i64 = F8S;
                        MOV64x(x7, i64);
                        if (rex.w) {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b111);
                                BNEZ_MARK2(x3);
                            }
                            LWSYNC();
                            MARKLOCK;
                            LDARX(x1, 0, wback);
                            ADD(x4, x1, x7);
                            STDCXd(x4, 0, wback);
                            BNE_MARKLOCK_CR0;
                            LWSYNC();
                            if (!ALIGNED_ATOMICxw) { B_MARK3_nocond; }
                        } else {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b11);
                                BNEZ_MARK(x3);
                            }
                            LWSYNC();
                            MARKLOCK;
                            LWARX(x1, 0, wback);
                            ADD(x4, x1, x7);
                            RLDICL(x4, x4, 0, 32);
                            STWCXd(x4, 0, wback);
                            BNE_MARKLOCK_CR0;
                            LWSYNC();
                            if (!ALIGNED_ATOMICxw) {
                                B_MARK3_nocond;
                                MARK;
                                ANDI(x3, wback, 0b111);
                                CMPWI(x3, 4);
                                BGE_MARK2(x3, x3);
                                LOCK_32_IN_8BYTE(ADD(x4, x1, x7), x1, wback, x3, x4, x5, x6);
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
                case 1: // LOCK OR Ed, Id/Ib
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
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, (opcode==0x81)?4:1);
                        if (opcode == 0x81) i64 = F32S; else i64 = F8S;
                        MOV64x(x7, i64);
                        if (rex.w) {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b111);
                                BNEZ_MARK2(x3);
                            }
                            LWSYNC();
                            MARKLOCK;
                            LDARX(x1, 0, wback);
                            OR(x4, x1, x7);
                            STDCXd(x4, 0, wback);
                            BNE_MARKLOCK_CR0;
                            LWSYNC();
                            if (!ALIGNED_ATOMICxw) { B_MARK3_nocond; }
                        } else {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b11);
                                BNEZ_MARK(x3);
                            }
                            LWSYNC();
                            MARKLOCK;
                            LWARX(x1, 0, wback);
                            OR(x4, x1, x7);
                            STWCXd(x4, 0, wback);
                            BNE_MARKLOCK_CR0;
                            LWSYNC();
                            if (!ALIGNED_ATOMICxw) {
                                B_MARK3_nocond;
                                MARK;
                                ANDI(x3, wback, 0b111);
                                CMPWI(x3, 4);
                                BGE_MARK2(x3, x3);
                                LOCK_32_IN_8BYTE(OR(x4, x1, x7), x1, wback, x3, x4, x5, x6);
                                B_MARK3_nocond;
                            }
                        }
                        if (!ALIGNED_ATOMICxw) {
                            MARK2;
                            LOCK_3264_CROSS_8BYTE(OR(x4, x1, x7), x1, wback, x4, x5, x6);
                            MARK3;
                        }
                        IFXORNAT (X_ALL | X_PEND) {
                            emit_or32c(dyn, ninst, rex, x1, i64, x3, x4);
                        }
                    }
                    break;
                case 2: // LOCK ADC Ed, Id/Ib
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
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, (opcode==0x81)?4:1);
                        if (opcode == 0x81) i64 = F32S; else i64 = F8S;
                        RESTORE_EFLAGS(x5);
                        BF_EXTRACT(x5, xFlags, F_CF, F_CF);
                        MOV64x(x7, i64);
                        ADD(x7, x7, x5);  // x7 = imm + CF
                        if (rex.w) {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b111);
                                BNEZ_MARK2(x3);
                            }
                            LWSYNC();
                            MARKLOCK;
                            LDARX(x1, 0, wback);
                            ADD(x4, x1, x7);
                            STDCXd(x4, 0, wback);
                            BNE_MARKLOCK_CR0;
                            LWSYNC();
                            if (!ALIGNED_ATOMICxw) { B_MARK3_nocond; }
                        } else {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b11);
                                BNEZ_MARK(x3);
                            }
                            LWSYNC();
                            MARKLOCK;
                            LWARX(x1, 0, wback);
                            ADD(x4, x1, x7);
                            RLDICL(x4, x4, 0, 32);
                            STWCXd(x4, 0, wback);
                            BNE_MARKLOCK_CR0;
                            LWSYNC();
                            if (!ALIGNED_ATOMICxw) {
                                B_MARK3_nocond;
                                MARK;
                                ANDI(x3, wback, 0b111);
                                CMPWI(x3, 4);
                                BGE_MARK2(x3, x3);
                                LOCK_32_IN_8BYTE(ADD(x4, x1, x7), x1, wback, x3, x4, x5, x6);
                                B_MARK3_nocond;
                            }
                        }
                        if (!ALIGNED_ATOMICxw) {
                            MARK2;
                            LOCK_3264_CROSS_8BYTE(ADDxw(x4, x1, x7), x1, wback, x4, x5, x6);
                            MARK3;
                        }
                        IFXORNAT (X_ALL | X_PEND) {
                            emit_add32(dyn, ninst, rex, x1, x7, x3, x4, x5);
                        }
                    }
                    break;
                case 3: // LOCK SBB Ed, Id/Ib
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
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, (opcode==0x81)?4:1);
                        if (opcode == 0x81) i64 = F32S; else i64 = F8S;
                        RESTORE_EFLAGS(x5);
                        BF_EXTRACT(x5, xFlags, F_CF, F_CF);
                        MOV64x(x7, i64);
                        ADD(x7, x7, x5);   // x7 = imm + CF
                        if (rex.w) {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b111);
                                BNEZ_MARK2(x3);
                            }
                            LWSYNC();
                            MARKLOCK;
                            LDARX(x1, 0, wback);
                            SUB(x4, x1, x7);
                            STDCXd(x4, 0, wback);
                            BNE_MARKLOCK_CR0;
                            LWSYNC();
                            if (!ALIGNED_ATOMICxw) { B_MARK3_nocond; }
                        } else {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b11);
                                BNEZ_MARK(x3);
                            }
                            LWSYNC();
                            MARKLOCK;
                            LWARX(x1, 0, wback);
                            SUB(x4, x1, x7);
                            RLDICL(x4, x4, 0, 32);
                            STWCXd(x4, 0, wback);
                            BNE_MARKLOCK_CR0;
                            LWSYNC();
                            if (!ALIGNED_ATOMICxw) {
                                B_MARK3_nocond;
                                MARK;
                                ANDI(x3, wback, 0b111);
                                CMPWI(x3, 4);
                                BGE_MARK2(x3, x3);
                                LOCK_32_IN_8BYTE(SUB(x4, x1, x7), x1, wback, x3, x4, x5, x6);
                                B_MARK3_nocond;
                            }
                        }
                        if (!ALIGNED_ATOMICxw) {
                            MARK2;
                            LOCK_3264_CROSS_8BYTE(SUBxw(x4, x1, x7), x1, wback, x4, x5, x6);
                            MARK3;
                        }
                        IFXORNAT (X_ALL | X_PEND) {
                            emit_sub32(dyn, ninst, rex, x1, x7, x3, x4, x5);
                        }
                    }
                    break;
                case 4: // LOCK AND Ed, Id/Ib
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
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, (opcode==0x81)?4:1);
                        if (opcode == 0x81) i64 = F32S; else i64 = F8S;
                        MOV64x(x7, i64);
                        if (rex.w) {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b111);
                                BNEZ_MARK2(x3);
                            }
                            LWSYNC();
                            MARKLOCK;
                            LDARX(x1, 0, wback);
                            AND(x4, x1, x7);
                            STDCXd(x4, 0, wback);
                            BNE_MARKLOCK_CR0;
                            LWSYNC();
                            if (!ALIGNED_ATOMICxw) { B_MARK3_nocond; }
                        } else {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b11);
                                BNEZ_MARK(x3);
                            }
                            LWSYNC();
                            MARKLOCK;
                            LWARX(x1, 0, wback);
                            AND(x4, x1, x7);
                            STWCXd(x4, 0, wback);
                            BNE_MARKLOCK_CR0;
                            LWSYNC();
                            if (!ALIGNED_ATOMICxw) {
                                B_MARK3_nocond;
                                MARK;
                                ANDI(x3, wback, 0b111);
                                CMPWI(x3, 4);
                                BGE_MARK2(x3, x3);
                                LOCK_32_IN_8BYTE(AND(x4, x1, x7), x1, wback, x3, x4, x5, x6);
                                B_MARK3_nocond;
                            }
                        }
                        if (!ALIGNED_ATOMICxw) {
                            MARK2;
                            LOCK_3264_CROSS_8BYTE(AND(x4, x1, x7), x1, wback, x4, x5, x6);
                            MARK3;
                        }
                        IFXORNAT (X_ALL | X_PEND) {
                            emit_and32c(dyn, ninst, rex, x1, i64, x3, x4);
                        }
                    }
                    break;
                case 5: // LOCK SUB Ed, Id/Ib
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
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, (opcode==0x81)?4:1);
                        if (opcode == 0x81) i64 = F32S; else i64 = F8S;
                        MOV64x(x7, i64);
                        if (rex.w) {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b111);
                                BNEZ_MARK2(x3);
                            }
                            LWSYNC();
                            MARKLOCK;
                            LDARX(x1, 0, wback);
                            SUB(x4, x1, x7);
                            STDCXd(x4, 0, wback);
                            BNE_MARKLOCK_CR0;
                            LWSYNC();
                            if (!ALIGNED_ATOMICxw) { B_MARK3_nocond; }
                        } else {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b11);
                                BNEZ_MARK(x3);
                            }
                            LWSYNC();
                            MARKLOCK;
                            LWARX(x1, 0, wback);
                            SUB(x4, x1, x7);
                            RLDICL(x4, x4, 0, 32);
                            STWCXd(x4, 0, wback);
                            BNE_MARKLOCK_CR0;
                            LWSYNC();
                            if (!ALIGNED_ATOMICxw) {
                                B_MARK3_nocond;
                                MARK;
                                ANDI(x3, wback, 0b111);
                                CMPWI(x3, 4);
                                BGE_MARK2(x3, x3);
                                LOCK_32_IN_8BYTE(SUB(x4, x1, x7), x1, wback, x3, x4, x5, x6);
                                B_MARK3_nocond;
                            }
                        }
                        if (!ALIGNED_ATOMICxw) {
                            MARK2;
                            LOCK_3264_CROSS_8BYTE(SUBxw(x4, x1, x7), x1, wback, x4, x5, x6);
                            MARK3;
                        }
                        IFXORNAT (X_ALL | X_PEND) {
                            emit_sub32c(dyn, ninst, rex, x1, i64, x3, x4, x5, x6);
                        }
                    }
                    break;
                case 6: // LOCK XOR Ed, Id/Ib
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
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, (opcode==0x81)?4:1);
                        if (opcode == 0x81) i64 = F32S; else i64 = F8S;
                        MOV64x(x7, i64);
                        if (rex.w) {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b111);
                                BNEZ_MARK2(x3);
                            }
                            LWSYNC();
                            MARKLOCK;
                            LDARX(x1, 0, wback);
                            XOR(x4, x1, x7);
                            STDCXd(x4, 0, wback);
                            BNE_MARKLOCK_CR0;
                            LWSYNC();
                            if (!ALIGNED_ATOMICxw) { B_MARK3_nocond; }
                        } else {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b11);
                                BNEZ_MARK(x3);
                            }
                            LWSYNC();
                            MARKLOCK;
                            LWARX(x1, 0, wback);
                            XOR(x4, x1, x7);
                            STWCXd(x4, 0, wback);
                            BNE_MARKLOCK_CR0;
                            LWSYNC();
                            if (!ALIGNED_ATOMICxw) {
                                B_MARK3_nocond;
                                MARK;
                                ANDI(x3, wback, 0b111);
                                CMPWI(x3, 4);
                                BGE_MARK2(x3, x3);
                                LOCK_32_IN_8BYTE(XOR(x4, x1, x7), x1, wback, x3, x4, x5, x6);
                                B_MARK3_nocond;
                            }
                        }
                        if (!ALIGNED_ATOMICxw) {
                            MARK2;
                            LOCK_3264_CROSS_8BYTE(XOR(x4, x1, x7), x1, wback, x4, x5, x6);
                            MARK3;
                        }
                        IFXORNAT (X_ALL | X_PEND) {
                            emit_xor32c(dyn, ninst, rex, x1, i64, x3, x4);
                        }
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
                // No flags affected
                GETGD;
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, 0);
                if (rex.w) {
                    if (!ALIGNED_ATOMICxw) {
                        ANDI(x3, wback, 0b111);
                        BNEZ_MARK2(x3);
                    }
                    LWSYNC();
                    MARKLOCK;
                    LDARX(x1, 0, wback);
                    STDCXd(gd, 0, wback);
                    BNE_MARKLOCK_CR0;
                    LWSYNC();
                    if (!ALIGNED_ATOMICxw) { B_MARK3_nocond; }
                } else {
                    if (!ALIGNED_ATOMICxw) {
                        ANDI(x3, wback, 0b11);
                        BNEZ_MARK(x3);
                    }
                    LWSYNC();
                    MARKLOCK;
                    LWARX(x1, 0, wback);
                    STWCXd(gd, 0, wback);
                    BNE_MARKLOCK_CR0;
                    LWSYNC();
                    if (!ALIGNED_ATOMICxw) {
                        B_MARK3_nocond;
                        MARK;
                        ANDI(x3, wback, 0b111);
                        CMPWI(x3, 4);
                        BGE_MARK2(x3, x3);
                        // 32-bit unaligned within 8-byte block: use LDARX on containing 8B
                        MR(x4, gd); // x4 = new value to store
                        LOCK_32_IN_8BYTE(MR(x4, x4), x1, wback, x3, x4, x5, x6);
                        // x1 = old value is set by macro (via SRD+RLDICL at the end)
                        B_MARK3_nocond;
                    }
                }
                if (!ALIGNED_ATOMICxw) {
                    MARK2;
                    // Cross 8-byte boundary: best-effort
                    RLDICR(x6, wback, 0, 60);
                    LWSYNC();
                    MARKLOCK;
                    LDARX(x5, 0, x6);
                    if (rex.w) { LD(x1, 0, wback); } else { LWZ(x1, 0, wback); }
                    if (rex.w) { STD(gd, 0, wback); } else { STW(gd, 0, wback); }
                    STDCXd(x5, 0, x6);
                    BNE_MARKLOCK_CR0;
                    LWSYNC();
                    MARK3;
                }
                MVxw(gd, x1);
            }
            break;

        case 0xF7:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 2: // LOCK NOT Ed
                    if (MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF();
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        INST_NAME("LOCK NOT Ed");
                        // No flags affected
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, 0);
                        if (rex.w) {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b111);
                                BNEZ_MARK2(x3);
                            }
                            LWSYNC();
                            MARKLOCK;
                            LDARX(x1, 0, wback);
                            NOT(x4, x1);
                            STDCXd(x4, 0, wback);
                            BNE_MARKLOCK_CR0;
                            LWSYNC();
                            if (!ALIGNED_ATOMICxw) { B_MARK3_nocond; }
                        } else {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b11);
                                BNEZ_MARK(x3);
                            }
                            LWSYNC();
                            MARKLOCK;
                            LWARX(x1, 0, wback);
                            NOT(x4, x1);
                            RLDICL(x4, x4, 0, 32);
                            STWCXd(x4, 0, wback);
                            BNE_MARKLOCK_CR0;
                            LWSYNC();
                            if (!ALIGNED_ATOMICxw) {
                                B_MARK3_nocond;
                                MARK;
                                ANDI(x3, wback, 0b111);
                                CMPWI(x3, 4);
                                BGE_MARK2(x3, x3);
                                LOCK_32_IN_8BYTE(NOT(x4, x1), x1, wback, x3, x4, x5, x6);
                                B_MARK3_nocond;
                            }
                        }
                        if (!ALIGNED_ATOMICxw) {
                            MARK2;
                            LOCK_3264_CROSS_8BYTE(NOT(x4, x1), x1, wback, x4, x5, x6);
                            MARK3;
                        }
                    }
                    break;
                case 3: // LOCK NEG Ed
                    if (MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF();
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        INST_NAME("LOCK NEG Ed");
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, 0);
                        if (rex.w) {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b111);
                                BNEZ_MARK2(x3);
                            }
                            LWSYNC();
                            MARKLOCK;
                            LDARX(x1, 0, wback);
                            NEG(x4, x1);
                            STDCXd(x4, 0, wback);
                            BNE_MARKLOCK_CR0;
                            LWSYNC();
                            if (!ALIGNED_ATOMICxw) { B_MARK3_nocond; }
                        } else {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b11);
                                BNEZ_MARK(x3);
                            }
                            LWSYNC();
                            MARKLOCK;
                            LWARX(x1, 0, wback);
                            NEG(x4, x1);
                            RLDICL(x4, x4, 0, 32);
                            STWCXd(x4, 0, wback);
                            BNE_MARKLOCK_CR0;
                            LWSYNC();
                            if (!ALIGNED_ATOMICxw) {
                                B_MARK3_nocond;
                                MARK;
                                ANDI(x3, wback, 0b111);
                                CMPWI(x3, 4);
                                BGE_MARK2(x3, x3);
                                LOCK_32_IN_8BYTE(NEG(x4, x1), x1, wback, x3, x4, x5, x6);
                                B_MARK3_nocond;
                            }
                        }
                        if (!ALIGNED_ATOMICxw) {
                            MARK2;
                            LOCK_3264_CROSS_8BYTE(NEG(x4, x1), x1, wback, x4, x5, x6);
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
                case 0: // LOCK INC Eb
                    if (MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF();
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        INST_NAME("LOCK INC Eb");
                        SETFLAGS(X_ALL & ~X_CF, SF_SUBSET, NAT_FLAGS_FUSION);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, 0);
                        LI(x7, 1);
                        LOCK_8_OP(ADD(x4, x1, x7), x1, wback, x3, x4, x5, x6);
                        IFXORNAT (X_ALL | X_PEND) {
                            emit_inc8(dyn, ninst, x1, x3, x4, x5);
                        }
                    }
                    break;
                case 1: // LOCK DEC Eb
                    if (MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF();
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        INST_NAME("LOCK DEC Eb");
                        SETFLAGS(X_ALL & ~X_CF, SF_SUBSET, NAT_FLAGS_FUSION);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, 0);
                        LI(x7, -1);
                        LOCK_8_OP(ADD(x4, x1, x7), x1, wback, x3, x4, x5, x6);
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
                case 0: // LOCK INC Ed
                    if (MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF();
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        INST_NAME("LOCK INC Ed");
                        SETFLAGS(X_ALL & ~X_CF, SF_SUBSET, NAT_FLAGS_FUSION);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, 0);
                        if (rex.w) {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b111);
                                BNEZ_MARK2(x3);
                            }
                            LWSYNC();
                            MARKLOCK;
                            LDARX(x1, 0, wback);
                            ADDI(x4, x1, 1);
                            STDCXd(x4, 0, wback);
                            BNE_MARKLOCK_CR0;
                            LWSYNC();
                            if (!ALIGNED_ATOMICxw) { B_MARK3_nocond; }
                        } else {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b11);
                                BNEZ_MARK(x3);
                            }
                            LWSYNC();
                            MARKLOCK;
                            LWARX(x1, 0, wback);
                            ADDI(x4, x1, 1);
                            RLDICL(x4, x4, 0, 32);
                            STWCXd(x4, 0, wback);
                            BNE_MARKLOCK_CR0;
                            LWSYNC();
                            if (!ALIGNED_ATOMICxw) {
                                B_MARK3_nocond;
                                MARK;
                                ANDI(x3, wback, 0b111);
                                CMPWI(x3, 4);
                                BGE_MARK2(x3, x3);
                                LOCK_32_IN_8BYTE(ADDI(x4, x1, 1), x1, wback, x3, x4, x5, x6);
                                B_MARK3_nocond;
                            }
                        }
                        if (!ALIGNED_ATOMICxw) {
                            MARK2;
                            LOCK_3264_CROSS_8BYTE(ADDIxw(x4, x1, 1), x1, wback, x4, x5, x6);
                            MARK3;
                        }
                        IFXORNAT (X_ALL | X_PEND) {
                            emit_inc32(dyn, ninst, rex, x1, x3, x4, x5, x6);
                        }
                    }
                    break;
                case 1: // LOCK DEC Ed
                    if (MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF();
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        INST_NAME("LOCK DEC Ed");
                        SETFLAGS(X_ALL & ~X_CF, SF_SUBSET, NAT_FLAGS_FUSION);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, 0);
                        if (rex.w) {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b111);
                                BNEZ_MARK2(x3);
                            }
                            LWSYNC();
                            MARKLOCK;
                            LDARX(x1, 0, wback);
                            ADDI(x4, x1, -1);
                            STDCXd(x4, 0, wback);
                            BNE_MARKLOCK_CR0;
                            LWSYNC();
                            if (!ALIGNED_ATOMICxw) { B_MARK3_nocond; }
                        } else {
                            if (!ALIGNED_ATOMICxw) {
                                ANDI(x3, wback, 0b11);
                                BNEZ_MARK(x3);
                            }
                            LWSYNC();
                            MARKLOCK;
                            LWARX(x1, 0, wback);
                            ADDI(x4, x1, -1);
                            RLDICL(x4, x4, 0, 32);
                            STWCXd(x4, 0, wback);
                            BNE_MARKLOCK_CR0;
                            LWSYNC();
                            if (!ALIGNED_ATOMICxw) {
                                B_MARK3_nocond;
                                MARK;
                                ANDI(x3, wback, 0b111);
                                CMPWI(x3, 4);
                                BGE_MARK2(x3, x3);
                                LOCK_32_IN_8BYTE(ADDI(x4, x1, -1), x1, wback, x3, x4, x5, x6);
                                B_MARK3_nocond;
                            }
                        }
                        if (!ALIGNED_ATOMICxw) {
                            MARK2;
                            LOCK_3264_CROSS_8BYTE(ADDIxw(x4, x1, -1), x1, wback, x4, x5, x6);
                            MARK3;
                        }
                        IFXORNAT (X_ALL | X_PEND) {
                            emit_dec32(dyn, ninst, rex, x1, x3, x4, x5, x6);
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
