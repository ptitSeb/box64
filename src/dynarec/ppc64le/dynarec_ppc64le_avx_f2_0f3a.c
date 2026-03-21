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

#include "ppc64le_printer.h"
#include "dynarec_ppc64le_private.h"
#include "dynarec_ppc64le_functions.h"
#include "../dynarec_helper.h"
#include "dynarec_ppc64le_helper.h"

uintptr_t dynarec64_AVX_F2_0F3A(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed;
    uint8_t wback, wb1, wb2;
    int64_t fixedaddress;
    int unscaled;
    rex_t rex = vex.rex;
    MAYUSE(u8);
    MAYUSE(wback);
    MAYUSE(wb1);
    MAYUSE(wb2);

    switch (opcode) {
        case 0xF0:
            // RORX: rotate right by immediate, no flags
            INST_NAME("RORX Gd, Ed, Ib");
            nextop = F8;
            GETGD;
            GETED(1);
            u8 = F8;
            if (rex.w) {
                // 64-bit rotate right by u8: ROTLDI(gd, ed, 64 - (u8 & 63))
                u8 &= 0x3f;
                if (u8) {
                    RLDICL(gd, ed, 64 - u8, 0);    // rotate left by (64-n) = rotate right by n
                } else {
                    MR(gd, ed);
                }
            } else {
                // 32-bit rotate right by u8: ROTRWI
                u8 &= 0x1f;
                if (u8) {
                    ROTRWI(gd, ed, u8);
                    ZEROUP(gd);
                } else {
                    ZEROUP2(gd, ed);
                }
            }
            break;
        default:
            DEFAULT;
    }

    return addr;
}
