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

uintptr_t dynarec64_AVX_F3_0F38(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed, vd;
    uint8_t wback, wb1, wb2;
    uint8_t gb1;
    int64_t j64;
    int64_t fixedaddress;
    int unscaled;
    rex_t rex = vex.rex;
    MAYUSE(u8);
    MAYUSE(j64);
    MAYUSE(wback);
    MAYUSE(wb1);
    MAYUSE(wb2);

    switch (opcode) {
        case 0xF5:
            // PEXT: parallel bit extract — gather bits from vd at mask positions (ed) → gd
            INST_NAME("PEXT Gd, Vd, Ed");
            nextop = F8;
            GETGD;
            GETVD;
            GETED(0);
            if (gd == ed || gd == vd) {
                gb1 = gd;
                gd = x6;
            } else {
                gb1 = 0;
            }
            MOV64x(gd, 0);         // result = 0
            MOV64x(x3, 1);         // dst_bit = 1 (walks destination bits)
            MOV64x(x4, 1);         // mask_bit = 1 (walks mask bits)
            MARK;
            AND(x5, ed, x4);       // test mask bit
            BEQZ_MARK2(x5);        // if mask bit clear, skip extract
            AND(x5, vd, x4);       // test source bit at mask position
            BEQZ_MARK3(x5);        // if source bit clear, skip OR
            OR(gd, gd, x3);        // set result bit at destination position
            MARK3;
            SLLIxw(x3, x3, 1);     // advance destination bit
            MARK2;
            SLLIxw(x4, x4, 1);     // advance mask bit
            BNEZ_MARK(x4);         // loop until mask_bit overflows
            if (gb1)
                MR(gb1, gd);
            break;
        case 0xF7:
            // SARX: arithmetic shift right, no flags
            INST_NAME("SARX Gd, Ed, Vd");
            nextop = F8;
            GETGD;
            GETED(0);
            GETVD;
            ANDI(x5, vd, rex.w ? 0x3f : 0x1f);
            SRAxw(gd, ed, x5);
            break;
        default:
            DEFAULT;
    }

    return addr;
}
