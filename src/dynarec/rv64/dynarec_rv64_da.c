#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
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


uintptr_t dynarec64_DA(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
{
    uint8_t nextop = F8;
    int64_t j64;
    uint8_t ed;
    uint8_t wback;
    int v1, v2;
    int d0;
    int s0;
    int64_t fixedaddress;
    int unscaled;

    MAYUSE(s0);
    MAYUSE(d0);
    MAYUSE(v2);
    MAYUSE(v1);
    MAYUSE(ed);
    MAYUSE(j64);

    switch (nextop) {
        case 0xC0:
        case 0xC1:
        case 0xC2:
        case 0xC3:
        case 0xC4:
        case 0xC5:
        case 0xC6:
        case 0xC7:
            DEFAULT;
            break;
        case 0xC8:
        case 0xC9:
        case 0xCA:
        case 0xCB:
        case 0xCC:
        case 0xCD:
        case 0xCE:
        case 0xCF:
            DEFAULT;
            break;
        case 0xD0:
        case 0xD1:
        case 0xD2:
        case 0xD3:
        case 0xD4:
        case 0xD5:
        case 0xD6:
        case 0xD7:
            DEFAULT;
            break;
        case 0xD8:
        case 0xD9:
        case 0xDA:
        case 0xDB:
        case 0xDC:
        case 0xDD:
        case 0xDE:
        case 0xDF:
            DEFAULT;
            break;
        case 0xE9:
            DEFAULT;
            break;

        case 0xE4:
        case 0xF0:
        case 0xF1:
        case 0xF4:
        case 0xF5:
        case 0xF6:
        case 0xF7:
        case 0xF8:
        case 0xF9:
        case 0xFD:
            DEFAULT;
            break;

        default:
            switch ((nextop >> 3) & 7) {
                case 6:
                    INST_NAME("FIDIV ST0, Ed");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, EXT_CACHE_ST_D);
                    v2 = fpu_get_scratch(dyn);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                    LW(x1, ed, fixedaddress);
                    FCVTDW(v2, x1, RD_RNE);
                    FDIVD(v1, v1, v2);
                    break;
                default:
                    DEFAULT;
                    break;
            }
    }
    return addr;
}
