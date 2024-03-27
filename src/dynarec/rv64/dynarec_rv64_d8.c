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


uintptr_t dynarec64_D8(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
{
    (void)ip;
    (void)rep;
    (void)need_epilog;

    uint8_t nextop = F8;
    uint8_t ed;
    uint8_t wback, wb1;
    uint8_t u8;
    int64_t fixedaddress;
    int unscaled;
    int v1, v2;
    int s0;
    int i1, i2, i3;

    MAYUSE(s0);
    MAYUSE(v2);
    MAYUSE(v1);

    switch (nextop) {
        case 0xC0 ... 0xC7:
            INST_NAME("FADD ST0, STx");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop & 7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop & 7, X87_COMBINE(0, nextop & 7));
            if (ST_IS_F(0)) {
                FADDS(v1, v1, v2);
            } else {
                FADDD(v1, v1, v2);
            }
            break;
        case 0xC8 ... 0xCF:
            INST_NAME("FMUL ST0, STx");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop & 7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop & 7, X87_COMBINE(0, nextop & 7));
            if (ST_IS_F(0)) {
                FMULS(v1, v1, v2);
            } else {
                FMULD(v1, v1, v2);
            }
            break;
        case 0xD0 ... 0xD7:
            INST_NAME("FCOM ST0, STx");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop & 7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop & 7, X87_COMBINE(0, nextop & 7));
            if (ST_IS_F(0)) {
                FCOMS(v1, v2, x1, x2, x3, x4, x5);
            } else {
                FCOMD(v1, v2, x1, x2, x3, x4, x5);
            }
            break;
        case 0xD8 ... 0xDF:
            INST_NAME("FCOMP ST0, STx");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop & 7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop & 7, X87_COMBINE(0, nextop & 7));
            if (ST_IS_F(0)) {
                FCOMS(v1, v2, x1, x2, x3, x4, x5);
            } else {
                FCOMD(v1, v2, x1, x2, x3, x4, x5);
            }
            X87_POP_OR_FAIL(dyn, ninst, x3);
            break;
        case 0xE0 ... 0xE7:
            INST_NAME("FSUB ST0, STx");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop & 7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop & 7, X87_COMBINE(0, nextop & 7));
            if (ST_IS_F(0)) {
                FSUBS(v1, v1, v2);
            } else {
                FSUBD(v1, v1, v2);
            }
            break;
        case 0xE8 ... 0xEF:
            INST_NAME("FSUBR ST0, STx");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop & 7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop & 7, X87_COMBINE(0, nextop & 7));
            if (ST_IS_F(0)) {
                FSUBS(v1, v2, v1);
            } else {
                FSUBD(v1, v2, v1);
            }
            break;
        case 0xF0 ... 0xF7:
            INST_NAME("FDIV ST0, STx");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop & 7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop & 7, X87_COMBINE(0, nextop & 7));
            if (ST_IS_F(0)) {
                FDIVS(v1, v1, v2);
            } else {
                FDIVD(v1, v1, v2);
            }
            break;
        case 0xF8 ... 0xFF:
            INST_NAME("FDIVR ST0, STx");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop & 7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop & 7, X87_COMBINE(0, nextop & 7));
            if (ST_IS_F(0)) {
                FDIVS(v1, v2, v1);
            } else {
                FDIVD(v1, v2, v1);
            }
            break;

        default:
            switch ((nextop >> 3) & 7) {
                case 0:
                    INST_NAME("FADD ST0, float[ED]");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_ST0);
                    s0 = fpu_get_scratch(dyn);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                    FLW(s0, ed, fixedaddress);
                    if (ST_IS_F(0)) {
                        FADDS(v1, v1, s0);
                    } else {
                        FCVTDS(s0, s0);
                        FADDD(v1, v1, s0);
                    }
                    break;
                case 1:
                    INST_NAME("FMUL ST0, float[ED]");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_ST0);
                    s0 = fpu_get_scratch(dyn);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                    FLW(s0, ed, fixedaddress);
                    if (ST_IS_F(0)) {
                        FMULS(v1, v1, s0);
                    } else {
                        FCVTDS(s0, s0);
                        FMULD(v1, v1, s0);
                    }
                    break;
                case 2:
                    INST_NAME("FCOM ST0, float[ED]");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_ST0);
                    s0 = fpu_get_scratch(dyn);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                    FLW(s0, ed, fixedaddress);
                    if (ST_IS_F(0)) {
                        FCOMS(v1, s0, x1, x6, x3, x4, x5);
                    } else {
                        FCVTDS(s0, s0);
                        FCOMD(v1, s0, x1, x6, x3, x4, x5);
                    }
                    break;
                case 3:
                    INST_NAME("FCOMP ST0, float[ED]");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_ST0);
                    s0 = fpu_get_scratch(dyn);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                    FLW(s0, ed, fixedaddress);
                    if (ST_IS_F(0)) {
                        FCOMS(v1, s0, x1, x6, x3, x4, x5);
                    } else {
                        FCVTDS(s0, s0);
                        FCOMD(v1, s0, x1, x6, x3, x4, x5);
                    }
                    X87_POP_OR_FAIL(dyn, ninst, x3);
                    break;
                case 4:
                    INST_NAME("FSUB ST0, float[ED]");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_ST0);
                    s0 = fpu_get_scratch(dyn);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                    FLW(s0, ed, fixedaddress);
                    if (ST_IS_F(0)) {
                        FSUBS(v1, v1, s0);
                    } else {
                        FCVTDS(s0, s0);
                        FSUBD(v1, v1, s0);
                    }
                    break;
                case 5:
                    INST_NAME("FSUBR ST0, float[ED]");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_ST0);
                    s0 = fpu_get_scratch(dyn);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                    FLW(s0, ed, fixedaddress);
                    if (ST_IS_F(0)) {
                        FSUBS(v1, s0, v1);
                    } else {
                        FCVTDS(s0, s0);
                        FSUBD(v1, s0, v1);
                    }
                    break;
                case 6:
                    INST_NAME("FDIV ST0, float[ED]");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_ST0);
                    s0 = fpu_get_scratch(dyn);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                    FLW(s0, ed, fixedaddress);
                    if (ST_IS_F(0)) {
                        FDIVS(v1, v1, s0);
                    } else {
                        FCVTDS(s0, s0);
                        FDIVD(v1, v1, s0);
                    }
                    break;
                case 7:
                    INST_NAME("FDIVR ST0, float[ED]");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_ST0);
                    s0 = fpu_get_scratch(dyn);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                    FLW(s0, ed, fixedaddress);
                    if (ST_IS_F(0)) {
                        FDIVS(v1, s0, v1);
                    } else {
                        FCVTDS(s0, s0);
                        FDIVD(v1, s0, v1);
                    }
                    break;
            }
    }
    return addr;
}
