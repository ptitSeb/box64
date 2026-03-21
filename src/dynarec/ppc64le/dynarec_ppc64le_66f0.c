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


uintptr_t dynarec64_66F0(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip;
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
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, 0);
                // PPC64LE has native LHARX/STHCXd for halfword atomics
                LWSYNC();
                MARKLOCK;
                LHARX(x1, 0, wback);
                ADD(x4, x1, x5);
                RLDICL(x4, x4, 0, 48);  // zero-extend to 16 bits
                STHCXd(x4, 0, wback);
                BNE_MARKLOCK_CR0;
                LWSYNC();
                IFXORNAT (X_ALL | X_PEND) {
                    BF_EXTRACT(x6, x1, 15, 0);
                    emit_add16(dyn, ninst, x6, x5, x3, x4, x7);
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
                INST_NAME("LOCK OR Ew, Gw");
                SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                GETGW(x5);
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, 0);
                LWSYNC();
                MARKLOCK;
                LHARX(x1, 0, wback);
                OR(x4, x1, x5);
                STHCXd(x4, 0, wback);
                BNE_MARKLOCK_CR0;
                LWSYNC();
                IFXORNAT (X_ALL | X_PEND) {
                    BF_EXTRACT(x6, x1, 15, 0);
                    emit_or16(dyn, ninst, x6, x5, x3, x4);
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
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, 0);
                        // For BTS Ew,Gw: bit offset = sign_extend_16(gd) with byte addressing
                        // Compute byte offset and bit within byte
                        EXTSH(x4, gd);        // sign-extend 16-bit register value
                        SRAWI(x3, x4, 3);     // byte offset = bit_offset >> 3 (arithmetic)
                        ADD(x6, wback, x3);   // adjusted address
                        ANDI(x5, x4, 7);      // bit within byte
                        LI(x3, 1);
                        SLW(x3, x3, x5);      // mask
                        // Use LBARX/STBCXd for byte-level atomic
                        LWSYNC();
                        MARKLOCK;
                        LBARX(x1, 0, x6);
                        OR(x4, x1, x3);
                        STBCXd(x4, 0, x6);
                        BNE_MARKLOCK_CR0;
                        LWSYNC();
                        IFXORNAT (X_CF) {
                            SRW(x1, x1, x5);
                            ANDI(x1, x1, 1);
                            IFX (X_CF) {
                                BF_INSERT(xFlags, x1, F_CF, F_CF);
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
                        BF_EXTRACT(x6, xRAX, 15, 0);  // x6 = AX (expected)
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, 0);
                        // Halfword CMPXCHG using LHARX/STHCXd
                        LWSYNC();
                        MARKLOCK;
                        LHARX(x1, 0, wback);          // x1 = current halfword
                        BF_EXTRACT(x1, x1, 15, 0);    // zero-extend
                        CMPW(x1, x6);
                        BNE(3*4);                      // skip store+branch if not equal
                        STHCXd(gd, 0, wback);          // store Gw
                        BNE_MARKLOCK_CR0;
                        LWSYNC();
                        // x1 = old value, x6 = expected (AX)
                        CMPW(x1, x6);
                        BEQ(2*4);                      // skip if equal (AX unchanged)
                        BF_INSERT(xRAX, x1, 15, 0);   // AX = old value
                        IFXORNAT (X_ALL | X_PEND) {
                            emit_cmp16(dyn, ninst, x6, x1, x3, x4, x5, x7);
                        }
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
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, 0);
                        // Same byte-level approach as BTS
                        EXTSH(x4, gd);
                        SRAWI(x3, x4, 3);
                        ADD(x6, wback, x3);
                        ANDI(x5, x4, 7);
                        LI(x3, 1);
                        SLW(x3, x3, x5);
                        NOT(x3, x3);           // inverted mask
                        LWSYNC();
                        MARKLOCK;
                        LBARX(x1, 0, x6);
                        AND(x4, x1, x3);
                        STBCXd(x4, 0, x6);
                        BNE_MARKLOCK_CR0;
                        LWSYNC();
                        IFXORNAT (X_CF) {
                            SRW(x1, x1, x5);
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
                                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, NO_DISP, 1);
                                u8 = F8;
                                u8 &= (rex.w ? 0x3f : 0xf);
                                // For 16-bit operand: bit offset is u8 & 0xf
                                // Compute byte offset from base and bit within byte
                                ADDI(x6, wback, u8 >> 3);   // byte address
                                LI(x3, 1);
                                SLWI(x3, x3, u8 & 7);       // mask for bit within byte
                                LWSYNC();
                                MARKLOCK;
                                LBARX(x1, 0, x6);
                                OR(x4, x1, x3);
                                STBCXd(x4, 0, x6);
                                BNE_MARKLOCK_CR0;
                                LWSYNC();
                                IFXORNAT (X_CF) {
                                    SRWI(x1, x1, u8 & 7);
                                    ANDI(x1, x1, 1);
                                    IFX (X_CF) {
                                        BF_INSERT(xFlags, x1, F_CF, F_CF);
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
                                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, NO_DISP, 1);
                                u8 = F8;
                                u8 &= (rex.w ? 0x3f : 0xf);
                                ADDI(x6, wback, u8 >> 3);
                                LI(x3, 1);
                                SLWI(x3, x3, u8 & 7);
                                NOT(x3, x3);
                                LWSYNC();
                                MARKLOCK;
                                LBARX(x1, 0, x6);
                                AND(x4, x1, x3);
                                STBCXd(x4, 0, x6);
                                BNE_MARKLOCK_CR0;
                                LWSYNC();
                                IFXORNAT (X_CF) {
                                    SRWI(x1, x1, u8 & 7);
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
                        BF_EXTRACT(x5, gd, 15, 0);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, 0);
                        LWSYNC();
                        MARKLOCK;
                        LHARX(x1, 0, wback);
                        ADD(x4, x1, x5);
                        RLDICL(x4, x4, 0, 48);  // zero-extend to 16 bits
                        STHCXd(x4, 0, wback);
                        BNE_MARKLOCK_CR0;
                        LWSYNC();
                        IFXORNAT (X_ALL | X_PEND) {
                            BF_EXTRACT(x6, x1, 15, 0);
                            emit_add16(dyn, ninst, x6, gd, x3, x4, x5);
                        }
                        // Gw = old value
                        BF_INSERT(gd, x1, 15, 0);
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
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, (opcode == 0x81) ? 2 : 1);
                        if (opcode == 0x81)
                            u64 = F16;
                        else
                            u64 = (uint16_t)(int16_t)F8S;
                        MOV64x(x5, u64);
                        LWSYNC();
                        MARKLOCK;
                        LHARX(x1, 0, wback);
                        ADD(x4, x1, x5);
                        RLDICL(x4, x4, 0, 48);
                        STHCXd(x4, 0, wback);
                        BNE_MARKLOCK_CR0;
                        LWSYNC();
                        IFXORNAT (X_ALL | X_PEND) {
                            BF_EXTRACT(x6, x1, 15, 0);
                            emit_add16(dyn, ninst, x6, x5, x3, x4, x7);
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
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, 0);
                        LWSYNC();
                        MARKLOCK;
                        LHARX(x1, 0, wback);
                        ADDI(x4, x1, 1);
                        RLDICL(x4, x4, 0, 48);
                        STHCXd(x4, 0, wback);
                        BNE_MARKLOCK_CR0;
                        LWSYNC();
                        IFXORNAT (X_ALL | X_PEND) {
                            ADDI(x5, xZR, 1);
                            BF_EXTRACT(x6, x1, 15, 0);
                            emit_inc16(dyn, ninst, x6, x5, x3, x4);
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
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, LOCK_LOCK, NO_DISP, 0);
                        LWSYNC();
                        MARKLOCK;
                        LHARX(x1, 0, wback);
                        ADDI(x4, x1, -1);
                        RLDICL(x4, x4, 0, 48);
                        STHCXd(x4, 0, wback);
                        BNE_MARKLOCK_CR0;
                        LWSYNC();
                        IFXORNAT (X_ALL | X_PEND) {
                            ADDI(x5, xZR, -1);
                            BF_EXTRACT(x6, x1, 15, 0);
                            emit_dec16(dyn, ninst, x6, x5, x3, x4, x7);
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
