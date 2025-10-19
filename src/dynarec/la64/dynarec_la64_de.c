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
#include "emu/x87emu_private.h"
#include "dynarec_native.h"

#include "la64_printer.h"
#include "dynarec_la64_private.h"
#include "../dynarec_helper.h"
#include "dynarec_la64_functions.h"


uintptr_t dynarec64_DE(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
{
    (void)ip;
    (void)rep;
    (void)need_epilog;

    uint8_t nextop = F8;
    uint8_t wback;
    uint8_t u8;
    int64_t fixedaddress;
    int v1, v2;

    MAYUSE(v2);
    MAYUSE(v1);

    if (MODREG)
        switch (nextop) {
            case 0xC0 ... 0xC7:
                INST_NAME("FADDP STx, ST0");
                v2 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop & 7));
                v1 = x87_get_st(dyn, ninst, x1, x2, nextop & 7, X87_COMBINE(0, nextop & 7));
                if (!BOX64ENV(dynarec_fastround)) u8 = x87_setround(dyn, ninst, x1, x5);
                if (ST_IS_F(0)) {
                    FADD_S(v1, v1, v2);
                } else {
                    FADD_D(v1, v1, v2);
                }
                X87_CHECK_PRECISION(v1);
                if (!BOX64ENV(dynarec_fastround)) x87_restoreround(dyn, ninst, u8);
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            case 0xC8 ... 0xCF:
                INST_NAME("FMULP STx, ST0");
                v2 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop & 7));
                v1 = x87_get_st(dyn, ninst, x1, x2, nextop & 7, X87_COMBINE(0, nextop & 7));
                if (!BOX64ENV(dynarec_fastround)) u8 = x87_setround(dyn, ninst, x1, x5);
                if (ST_IS_F(0)) {
                    FMUL_S(v1, v1, v2);
                } else {
                    FMUL_D(v1, v1, v2);
                }
                X87_CHECK_PRECISION(v1);
                if (!BOX64ENV(dynarec_fastround)) x87_restoreround(dyn, ninst, u8);
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            case 0xD0 ... 0xD7:
                INST_NAME("FCOMP ST0, STx"); // yep
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop & 7));
                v2 = x87_get_st(dyn, ninst, x1, x2, nextop & 7, X87_COMBINE(0, nextop & 7));
                if (ST_IS_F(0)) {
                    FCOMS(v1, v2, x1, x2, x3);
                } else {
                    FCOMD(v1, v2, x1, x2, x3);
                }
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            case 0xD9:
                INST_NAME("FCOMPP ST0, STx");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop & 7));
                v2 = x87_get_st(dyn, ninst, x1, x2, nextop & 7, X87_COMBINE(0, nextop & 7));
                if (ST_IS_F(0)) {
                    FCOMS(v1, v2, x1, x2, x3);
                } else {
                    FCOMD(v1, v2, x1, x2, x3);
                }
                X87_POP_OR_FAIL(dyn, ninst, x3);
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            case 0xE0 ... 0xE7:
                INST_NAME("FSUBRP STx, ST0");
                v2 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop & 7));
                v1 = x87_get_st(dyn, ninst, x1, x2, nextop & 7, X87_COMBINE(0, nextop & 7));
                if (!BOX64ENV(dynarec_fastround)) u8 = x87_setround(dyn, ninst, x1, x5);
                if (ST_IS_F(0)) {
                    FSUB_S(v1, v2, v1);
                } else {
                    FSUB_D(v1, v2, v1);
                }
                X87_CHECK_PRECISION(v1);
                if (!BOX64ENV(dynarec_fastround)) x87_restoreround(dyn, ninst, u8);
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            case 0xE8 ... 0xEF:
                INST_NAME("FSUBP STx, ST0");
                v2 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop & 7));
                v1 = x87_get_st(dyn, ninst, x1, x2, nextop & 7, X87_COMBINE(0, nextop & 7));
                if (!BOX64ENV(dynarec_fastround)) u8 = x87_setround(dyn, ninst, x1, x5);
                if (ST_IS_F(0)) {
                    FSUB_S(v1, v1, v2);
                } else {
                    FSUB_D(v1, v1, v2);
                }
                X87_CHECK_PRECISION(v1);
                if (!BOX64ENV(dynarec_fastround)) x87_restoreround(dyn, ninst, u8);
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            case 0xF0 ... 0xF7:
                INST_NAME("FDIVRP STx, ST0");
                v2 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop & 7));
                v1 = x87_get_st(dyn, ninst, x1, x2, nextop & 7, X87_COMBINE(0, nextop & 7));
                if (!BOX64ENV(dynarec_fastround)) u8 = x87_setround(dyn, ninst, x1, x5);
                if (ST_IS_F(0)) {
                    FDIV_S(v1, v2, v1);
                } else {
                    FDIV_D(v1, v2, v1);
                }
                X87_CHECK_PRECISION(v1);
                if (!BOX64ENV(dynarec_fastround)) x87_restoreround(dyn, ninst, u8);
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            case 0xF8 ... 0xFF:
                INST_NAME("FDIVP STx, ST0");
                v2 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop & 7));
                v1 = x87_get_st(dyn, ninst, x1, x2, nextop & 7, X87_COMBINE(0, nextop & 7));
                if (!BOX64ENV(dynarec_fastround)) u8 = x87_setround(dyn, ninst, x1, x5);
                if (ST_IS_F(0)) {
                    FDIV_S(v1, v1, v2);
                } else {
                    FDIV_D(v1, v1, v2);
                }
                X87_CHECK_PRECISION(v1);
                if (!BOX64ENV(dynarec_fastround)) x87_restoreround(dyn, ninst, u8);
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            default:
                DEFAULT;
                break;
        }
    else
        switch ((nextop >> 3) & 7) {
            default:
                DEFAULT;
        }
    return addr;
}
