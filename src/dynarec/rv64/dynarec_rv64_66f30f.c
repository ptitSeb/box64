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
#include "dynarec_rv64_functions.h"
#include "../dynarec_helper.h"

uintptr_t dynarec64_66F30F(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8, s8;
    int32_t i32;
    uint8_t gd, ed;
    uint8_t wback, wb1, wb2, gback;
    uint8_t eb1, eb2;
    int64_t j64;
    uint64_t tmp64u, tmp64u2;
    int v0, v1;
    int q0, q1;
    int d0, d1;
    int64_t fixedaddress, gdoffset;
    int unscaled;

    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(j64);

    static const int8_t round_round[] = { RD_RNE, RD_RDN, RD_RUP, RD_RTZ };

    switch (opcode) {
        case 0xB8:
            INST_NAME("POPCNT Gw, Ew");
            SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            nextop = F8;
            GETEW(x1, 0);
            GETGW(x2);
            CLEAR_FLAGS();
            if (cpuext.zbb) {
                CPOPW(gd, ed);
            } else {
                // w = ed - ((ed >> 1) & 0x5555)
                SRLI(x4, ed, 1);
                MOV64x(x5, 0x5555);
                AND(x4, x4, x5);
                SUB(x3, ed, x4);

                // w = (w & 0x3333) + ((w >> 2) & 0x3333)
                SRLI(x4, x3, 2);
                MOV64x(x5, 0x3333);
                AND(x6, x3, x5);
                AND(x4, x4, x5);
                ADD(x3, x6, x4);

                // w = (w + (w >> 4)) & 0x0F0F
                SRLI(x4, x3, 4);
                MOV64x(x5, 0x0F0F);
                ADD(x3, x3, x4);
                AND(x3, x3, x5);

                // w = (w + (w >> 8)) & 0xFF
                SRLI(x4, x3, 8);
                ADD(x3, x3, x4);
                ANDI(gd, x3, 0xFF);
            }
            IFX (X_ALL) {
                SET_FLAGS_EQZ(gd, F_ZF, x7);
            }
            GWBACK;
            break;
        case 0xBC:
            INST_NAME("TZCNT Gw, Ew");
            if (!BOX64DRENV(dynarec_safeflags)) {
                SETFLAGS(X_CF | X_ZF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            } else {
                SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION);
            }
            SET_DFNONE();
            nextop = F8;
            GETEW(x1, 0);
            GETGW(x2);
            CLEAR_FLAGS();
            IFX (X_CF) {
                SEQZ(x3, ed);
                OR(xFlags, xFlags, x3); // F_CF at bit0.
            }
            if (cpuext.zbs) {
                BSETI(x6, ed, 16);
            } else {
                ADDI(x6, xZR, 1);
                SLLI(x6, x6, 16);
                OR(x6, ed, x6);
            }
            if (cpuext.zbb) {
                CTZW(gd, x6);
            } else {
                ADDI(gd, xZR, 0);
                MARK;
                ANDI(x5, x6, 1);
                BNEZ_MARK3(x5);
                ADDI(gd, gd, 1);
                SRLI(x6, x6, 1);
                B_MARK_nocond;
                MARK3;
            }
            IFX (X_ZF) SET_FLAGS_EQZ(gd, F_ZF, x3);
            GWBACK;
            break;
        case 0xBD:
            INST_NAME("LZCNT Gw, Ew");
            if (!BOX64DRENV(dynarec_safeflags)) {
                SETFLAGS(X_CF | X_ZF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            } else {
                SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION);
            }
            SET_DFNONE();
            nextop = F8;
            GETEW(x1, 0);
            GETGW(x2);
            CLEAR_FLAGS();
            IFX (X_CF) {
                SEQZ(x3, ed);
                OR(xFlags, xFlags, x3); // F_CF at bit0.
            }
            if (cpuext.zbb) {
                CLZW(gd, ed);
                ADDI(gd, gd, -16);
            } else {
                BEQZ_MARK2(ed);
                ADDI(gd, xZR, 0);
                MV(x4, ed);
                MOV64x(x6, 0x8000);
                MARK;
                AND(x5, x4, x6);
                BNEZ_MARK3(x5);
                ADDI(gd, gd, 1);
                SLLI(x4, x4, 1);
                B_MARK_nocond;
                MARK2;
                ADDI(gd, xZR, 16);
                MARK3;
            }
            IFX (X_ZF) SET_FLAGS_EQZ(gd, F_ZF, x3);
            GWBACK;
            break;
        default:
            DEFAULT;
    }
    return addr;
}
