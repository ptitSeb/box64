#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "dynarec.h"
#include "emu/x64emu_private.h"
#include "emu/x64run_private.h"
#include "x64run.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "emu/x87emu_private.h"
#include "dynarec_native.h"

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_helper.h"
#include "dynarec_rv64_functions.h"


uintptr_t dynarec64_DC(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
{
    (void)ip; (void)rep; (void)need_epilog;

    uint8_t nextop = F8;
    uint8_t wback;
    int64_t fixedaddress;
    int unscaled;
    int v1, v2;

    MAYUSE(v2);
    MAYUSE(v1);

    switch(nextop) {
        case 0xC0 ... 0xC7:
            INST_NAME("FADD STx, ST0");
            DEFAULT;
            break;
        case 0xC8 ... 0xCF:
            INST_NAME("FMUL STx, ST0");
            DEFAULT;
            break;
        case 0xD0 ... 0xD7:
            INST_NAME("FCOM ST0, STx"); //yep
            DEFAULT;
            break;
        case 0xD8 ... 0xDF:
            INST_NAME("FCOMP ST0, STx");
            DEFAULT;
            break;
        case 0xE0 ... 0xE7:
            INST_NAME("FSUBR STx, ST0");
            DEFAULT;
            break;
            break;
        case 0xE8 ... 0xEF:
            INST_NAME("FSUB STx, ST0");
            DEFAULT;
            break;
        case 0xF0 ... 0xF7:
            INST_NAME("FDIVR STx, ST0");
            DEFAULT;
            break;
        case 0xF8 ... 0xFF:
            INST_NAME("FDIV STx, ST0");
            DEFAULT;
            break;
        default:
            switch((nextop>>3)&7) {
                case 3:
                    INST_NAME("FCOMP ST0, double[ED]");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, EXT_CACHE_ST_D);
                    v2 = fpu_get_scratch(dyn);
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                    FLD(v2, wback, fixedaddress);

                    LHU(x3, xEmu, offsetof(x64emu_t, sw));
                    MOV32w(x1, 0b1110100011111111); // mask off c0,c1,c2,c3
                    AND(x3, x3, x1);
                    FEQD(x5, v1, v1);
                    FEQD(x4, v2, v2);
                    AND(x5, x5, x4);
                    BEQZ(x5, 24); // undefined/NaN
                    FEQD(x5, v1, v2);
                    BNEZ(x5, 28); // equal
                    FLTD(x3, v1, v2); // x3 = (v1<v2)?1:0
                    SLLI(x1, x3, 8);
                    J(20); // end
                    // undefined/NaN
                    LUI(x1, 1);
                    ADDI(x1, x1, 0b010100000000);
                    J(8); // end
                    // equal
                    LUI(x1, 1);
                    // end
                    OR(x3, x3, x1);
                    SH(x3, xEmu, offsetof(x64emu_t, sw));

                    x87_do_pop(dyn, ninst, x3);
                    break;
                case 6:
                    INST_NAME("FDIV ST0, double[ED]");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, EXT_CACHE_ST_D);
                    v2 = fpu_get_scratch(dyn);
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                    FLD(v2, wback, fixedaddress);
                    FDIVD(v1, v1, v2);
                    break;
                default:
                    DEFAULT;
            }
    }
    return addr;
}
