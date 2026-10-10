// SPDX-License-Identifier: MIT
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

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "../dynarec_helper.h"
#include "dynarec_rv64_functions.h"


uintptr_t dynarec64_DE(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t nextop = F8;
    uint8_t wback;
    uint8_t u8;
    uint8_t ed;
    int64_t fixedaddress;
    int v1, v2, s1, s2;
    int64_t j64;

    MAYUSE(ed);

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
                    if (!BOX64ENV(dynarec_fastnan)) {
                        s1 = fpu_get_scratch(dyn);
                        s2 = fpu_get_scratch(dyn);
                        FMVS(s1, v1);
                        FMVS(s2, v2);
                    }
                    FADDS(v1, v1, v2);
                    if (!BOX64ENV(dynarec_fastnan)) {
                        X87_ARITH_NAN_FIX_S(v1, s1, s2, x5, x6, x4);
                    }
                } else {
                    if (!BOX64ENV(dynarec_fastnan)) {
                        s1 = fpu_get_scratch(dyn);
                        s2 = fpu_get_scratch(dyn);
                        FMVD(s1, v1);
                        FMVD(s2, v2);
                    }
                    FADDD(v1, v1, v2);
                    if (!BOX64ENV(dynarec_fastnan)) {
                        X87_ARITH_NAN_FIX_D(v1, s1, s2, x5, x6, x4);
                    }
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
                    if (!BOX64ENV(dynarec_fastnan)) {
                        s1 = fpu_get_scratch(dyn);
                        s2 = fpu_get_scratch(dyn);
                        FMVS(s1, v1);
                        FMVS(s2, v2);
                    }
                    FMULS(v1, v1, v2);
                    if (!BOX64ENV(dynarec_fastnan)) {
                        X87_ARITH_NAN_FIX_S(v1, s1, s2, x5, x6, x4);
                    }
                } else {
                    if (!BOX64ENV(dynarec_fastnan)) {
                        s1 = fpu_get_scratch(dyn);
                        s2 = fpu_get_scratch(dyn);
                        FMVD(s1, v1);
                        FMVD(s2, v2);
                    }
                    FMULD(v1, v1, v2);
                    if (!BOX64ENV(dynarec_fastnan)) {
                        X87_ARITH_NAN_FIX_D(v1, s1, s2, x5, x6, x4);
                    }
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
                    FCOMS(v1, v2, x1, x2, x3, x4, x5);
                } else {
                    FCOMD(v1, v2, x1, x2, x3, x4, x5);
                }
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            case 0xD9:
                INST_NAME("FCOMPP ST0, STx");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop & 7));
                v2 = x87_get_st(dyn, ninst, x1, x2, nextop & 7, X87_COMBINE(0, nextop & 7));
                if (ST_IS_F(0)) {
                    FCOMS(v1, v2, x1, x2, x3, x4, x5);
                } else {
                    FCOMD(v1, v2, x1, x2, x3, x4, x5);
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
                    if (!BOX64ENV(dynarec_fastnan)) {
                        s1 = fpu_get_scratch(dyn);
                        s2 = fpu_get_scratch(dyn);
                        FMVS(s1, v2);
                        FMVS(s2, v1);
                    }
                    FSUBS(v1, v2, v1);
                    if (!BOX64ENV(dynarec_fastnan)) {
                        X87_ARITH_NAN_FIX_S(v1, s1, s2, x5, x6, x4);
                    }
                } else {
                    if (!BOX64ENV(dynarec_fastnan)) {
                        s1 = fpu_get_scratch(dyn);
                        s2 = fpu_get_scratch(dyn);
                        FMVD(s1, v2);
                        FMVD(s2, v1);
                    }
                    FSUBD(v1, v2, v1);
                    if (!BOX64ENV(dynarec_fastnan)) {
                        X87_ARITH_NAN_FIX_D(v1, s1, s2, x5, x6, x4);
                    }
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
                    if (!BOX64ENV(dynarec_fastnan)) {
                        s1 = fpu_get_scratch(dyn);
                        s2 = fpu_get_scratch(dyn);
                        FMVS(s1, v1);
                        FMVS(s2, v2);
                    }
                    FSUBS(v1, v1, v2);
                    if (!BOX64ENV(dynarec_fastnan)) {
                        X87_ARITH_NAN_FIX_S(v1, s1, s2, x5, x6, x4);
                    }
                } else {
                    if (!BOX64ENV(dynarec_fastnan)) {
                        s1 = fpu_get_scratch(dyn);
                        s2 = fpu_get_scratch(dyn);
                        FMVD(s1, v1);
                        FMVD(s2, v2);
                    }
                    FSUBD(v1, v1, v2);
                    if (!BOX64ENV(dynarec_fastnan)) {
                        X87_ARITH_NAN_FIX_D(v1, s1, s2, x5, x6, x4);
                    }
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
                    if (!BOX64ENV(dynarec_fastnan)) {
                        s1 = fpu_get_scratch(dyn);
                        s2 = fpu_get_scratch(dyn);
                        FMVS(s1, v2);
                        FMVS(s2, v1);
                    }
                    FDIVS(v1, v2, v1);
                    if (!BOX64ENV(dynarec_fastnan)) {
                        X87_ARITH_NAN_FIX_S(v1, s1, s2, x5, x6, x4);
                    }
                } else {
                    if (!BOX64ENV(dynarec_fastnan)) {
                        s1 = fpu_get_scratch(dyn);
                        s2 = fpu_get_scratch(dyn);
                        FMVD(s1, v2);
                        FMVD(s2, v1);
                    }
                    FDIVD(v1, v2, v1);
                    if (!BOX64ENV(dynarec_fastnan)) {
                        X87_ARITH_NAN_FIX_D(v1, s1, s2, x5, x6, x4);
                    }
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
                    if (!BOX64ENV(dynarec_fastnan)) {
                        s1 = fpu_get_scratch(dyn);
                        s2 = fpu_get_scratch(dyn);
                        FMVS(s1, v1);
                        FMVS(s2, v2);
                    }
                    FDIVS(v1, v1, v2);
                    if (!BOX64ENV(dynarec_fastnan)) {
                        X87_ARITH_NAN_FIX_S(v1, s1, s2, x5, x6, x4);
                    }
                } else {
                    if (!BOX64ENV(dynarec_fastnan)) {
                        s1 = fpu_get_scratch(dyn);
                        s2 = fpu_get_scratch(dyn);
                        FMVD(s1, v1);
                        FMVD(s2, v2);
                    }
                    FDIVD(v1, v1, v2);
                    if (!BOX64ENV(dynarec_fastnan)) {
                        X87_ARITH_NAN_FIX_D(v1, s1, s2, x5, x6, x4);
                    }
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
            case 0:
                INST_NAME("FIADD ST0, Ed");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, EXT_CACHE_ST_D);
                v2 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                LH(x1, ed, fixedaddress);
                FCVTDW(v2, x1, RD_RNE); // i32 -> double
                if (!BOX64ENV(dynarec_fastround)) u8 = x87_setround(dyn, ninst, x1, x5);
                if (!BOX64ENV(dynarec_fastnan)) {
                    s1 = fpu_get_scratch(dyn);
                    s2 = fpu_get_scratch(dyn);
                    FMVD(s1, v1);
                    FMVD(s2, v2);
                }
                FADDD(v1, v1, v2);
                if (!BOX64ENV(dynarec_fastnan)) {
                    X87_ARITH_NAN_FIX_D(v1, s1, s2, x5, x6, x4);
                }
                X87_CHECK_PRECISION(v1);
                if (!BOX64ENV(dynarec_fastround)) x87_restoreround(dyn, ninst, u8);
                break;
            case 1:
                INST_NAME("FIMUL ST0, Ed");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, EXT_CACHE_ST_D);
                v2 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                LH(x1, ed, fixedaddress);
                FCVTDW(v2, x1, RD_RNE); // i32 -> double
                if (!BOX64ENV(dynarec_fastround)) u8 = x87_setround(dyn, ninst, x1, x5);
                if (!BOX64ENV(dynarec_fastnan)) {
                    s1 = fpu_get_scratch(dyn);
                    s2 = fpu_get_scratch(dyn);
                    FMVD(s1, v1);
                    FMVD(s2, v2);
                }
                FMULD(v1, v1, v2);
                if (!BOX64ENV(dynarec_fastnan)) {
                    X87_ARITH_NAN_FIX_D(v1, s1, s2, x5, x6, x4);
                }
                X87_CHECK_PRECISION(v1);
                if (!BOX64ENV(dynarec_fastround)) x87_restoreround(dyn, ninst, u8);
                break;
            case 2:
                INST_NAME("FICOM ST0, Ed");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, EXT_CACHE_ST_D);
                v2 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                LH(x1, ed, fixedaddress);
                FCVTDW(v2, x1, RD_RNE); // i32 -> double
                FCOMD(v1, v2, x1, x2, x3, x4, x5);
                break;
            case 3:
                INST_NAME("FICOMP ST0, Ed");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, EXT_CACHE_ST_D);
                v2 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                LH(x1, ed, fixedaddress);
                FCVTDW(v2, x1, RD_RNE); // i32 -> double
                FCOMD(v1, v2, x1, x2, x3, x4, x5);
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            case 4:
                INST_NAME("FISUB ST0, Ed");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, EXT_CACHE_ST_D);
                v2 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                LH(x1, ed, fixedaddress);
                FCVTDW(v2, x1, RD_RNE); // i32 -> double
                if (!BOX64ENV(dynarec_fastround)) u8 = x87_setround(dyn, ninst, x1, x5);
                if (!BOX64ENV(dynarec_fastnan)) {
                    s1 = fpu_get_scratch(dyn);
                    s2 = fpu_get_scratch(dyn);
                    FMVD(s1, v1);
                    FMVD(s2, v2);
                }
                FSUBD(v1, v1, v2);
                if (!BOX64ENV(dynarec_fastnan)) {
                    X87_ARITH_NAN_FIX_D(v1, s1, s2, x5, x6, x4);
                }
                X87_CHECK_PRECISION(v1);
                if (!BOX64ENV(dynarec_fastround)) x87_restoreround(dyn, ninst, u8);
                break;
            case 5:
                INST_NAME("FISUBR ST0, Ed");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, EXT_CACHE_ST_D);
                v2 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                LH(x1, ed, fixedaddress);
                FCVTDW(v2, x1, RD_RNE); // i32 -> double
                if (!BOX64ENV(dynarec_fastround)) u8 = x87_setround(dyn, ninst, x1, x5);
                if (!BOX64ENV(dynarec_fastnan)) {
                    s1 = fpu_get_scratch(dyn);
                    s2 = fpu_get_scratch(dyn);
                    FMVD(s1, v2);
                    FMVD(s2, v1);
                }
                FSUBD(v1, v2, v1);
                if (!BOX64ENV(dynarec_fastnan)) {
                    X87_ARITH_NAN_FIX_D(v1, s1, s2, x5, x6, x4);
                }
                X87_CHECK_PRECISION(v1);
                if (!BOX64ENV(dynarec_fastround)) x87_restoreround(dyn, ninst, u8);
                break;
            case 6:
                INST_NAME("FIDIV ST0, Ed");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, EXT_CACHE_ST_D);
                v2 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                LH(x1, ed, fixedaddress);
                FCVTDW(v2, x1, RD_RNE); // i32 -> double
                if (!BOX64ENV(dynarec_fastround)) u8 = x87_setround(dyn, ninst, x1, x5);
                if (!BOX64ENV(dynarec_fastnan)) {
                    s1 = fpu_get_scratch(dyn);
                    s2 = fpu_get_scratch(dyn);
                    FMVD(s1, v1);
                    FMVD(s2, v2);
                }
                FDIVD(v1, v1, v2);
                if (!BOX64ENV(dynarec_fastnan)) {
                    X87_ARITH_NAN_FIX_D(v1, s1, s2, x5, x6, x4);
                }
                X87_CHECK_PRECISION(v1);
                if (!BOX64ENV(dynarec_fastround)) x87_restoreround(dyn, ninst, u8);
                break;
            case 7:
                INST_NAME("FIDIVR ST0, Ed");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, EXT_CACHE_ST_D);
                v2 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                LH(x1, ed, fixedaddress);
                FCVTDW(v2, x1, RD_RNE); // i32 -> double
                if (!BOX64ENV(dynarec_fastround)) u8 = x87_setround(dyn, ninst, x1, x5);
                if (!BOX64ENV(dynarec_fastnan)) {
                    s1 = fpu_get_scratch(dyn);
                    s2 = fpu_get_scratch(dyn);
                    FMVD(s1, v2);
                    FMVD(s2, v1);
                }
                FDIVD(v1, v2, v1);
                if (!BOX64ENV(dynarec_fastnan)) {
                    X87_ARITH_NAN_FIX_D(v1, s1, s2, x5, x6, x4);
                }
                X87_CHECK_PRECISION(v1);
                if (!BOX64ENV(dynarec_fastround)) x87_restoreround(dyn, ninst, u8);
                break;
        }
    return addr;
}
