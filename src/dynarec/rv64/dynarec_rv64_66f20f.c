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

uintptr_t dynarec64_66F20F(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
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
        case 0x38: // these are some more SSSE4.2+ opcodes
            opcode = F8;
            switch (opcode) {
                case 0xF1:
                    if (rex.w) return dynarec64_F20F(dyn, addr - 2, ip, ninst, rex, ok, need_epilog);
                    INST_NAME("CRC32 Gd, Ew");
                    nextop = F8;
                    GETGD;
                    GETEW(x1, 0);
                    MOV32w(x2, 0x82f63b78);
                    for (int j = 0; j < 2; ++j) {
                        SRLI(x5, ed, 8 * j);
                        ANDI(x6, x5, 0xFF);
                        XOR(gd, gd, x6);
                        for (int i = 0; i < 8; i++) {
                            SRLI((i & 1) ? gd : x4, (i & 1) ? x4 : gd, 1);
                            ANDI(x6, (i & 1) ? x4 : gd, 1);
                            BEQZ(x6, 4 + 4);
                            XOR((i & 1) ? gd : x4, (i & 1) ? gd : x4, x2);
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
