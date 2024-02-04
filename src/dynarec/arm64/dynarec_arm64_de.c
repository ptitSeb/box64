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

#include "arm64_printer.h"
#include "dynarec_arm64_private.h"
#include "dynarec_arm64_helper.h"
#include "dynarec_arm64_functions.h"


uintptr_t dynarec64_DE(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
{
    (void)ip; (void)rep; (void)need_epilog;

    uint8_t nextop = F8;
    uint8_t wback;
    int64_t fixedaddress;
    int unscaled;
    int v1, v2;

    MAYUSE(v2);
    MAYUSE(v1);

    if(MODREG)
    switch(nextop) {
        case 0xC0:
        case 0xC1:
        case 0xC2:
        case 0xC3:
        case 0xC4:
        case 0xC5:
        case 0xC6:
        case 0xC7:
            INST_NAME("FADDP STx, ST0");
            v2 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v1 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            if(ST_IS_F(0)) {
                FADDS(v1, v1, v2);
            } else {
                FADDD(v1, v1, v2);
            }
            X87_POP_OR_FAIL(dyn, ninst, x3);
            break;
        case 0xC8:
        case 0xC9:
        case 0xCA:
        case 0xCB:
        case 0xCC:
        case 0xCD:
        case 0xCE:
        case 0xCF:
            INST_NAME("FMULP STx, ST0");
            v2 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v1 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            if(ST_IS_F(0)) {
                FMULS(v1, v1, v2);
            } else {
                FMULD(v1, v1, v2);
            }
            X87_POP_OR_FAIL(dyn, ninst, x3);
            break;
        case 0xD0:
        case 0xD1:
        case 0xD2:
        case 0xD3:
        case 0xD4:
        case 0xD5:
        case 0xD6:
        case 0xD7:
            INST_NAME("FCOMP ST0, STx"); //yep
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            if(ST_IS_F(0)) {
                FCMPS(v1, v2);
            } else {
                FCMPD(v1, v2);
            }
            FCOM(x1, x2, x3);
            X87_POP_OR_FAIL(dyn, ninst, x3);
            break;
        case 0xD9:
            INST_NAME("FCOMPP ST0, STx");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            if(ST_IS_F(0)) {
                FCMPS(v1, v2);
            } else {
                FCMPD(v1, v2);
            }
            FCOM(x1, x2, x3);
            X87_POP_OR_FAIL(dyn, ninst, x3);
            X87_POP_OR_FAIL(dyn, ninst, x3);
            break;
        case 0xE0:
        case 0xE1:
        case 0xE2:
        case 0xE3:
        case 0xE4:
        case 0xE5:
        case 0xE6:
        case 0xE7:
            INST_NAME("FSUBRP STx, ST0");
            v2 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v1 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            if(ST_IS_F(0)) {
                FSUBS(v1, v2, v1);
            } else {
                FSUBD(v1, v2, v1);
            }
            X87_POP_OR_FAIL(dyn, ninst, x3);
            break;
        case 0xE8:
        case 0xE9:
        case 0xEA:
        case 0xEB:
        case 0xEC:
        case 0xED:
        case 0xEE:
        case 0xEF:
            INST_NAME("FSUBP STx, ST0");
            v2 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v1 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            if(ST_IS_F(0)) {
                FSUBS(v1, v1, v2);
            } else {
                FSUBD(v1, v1, v2);
            }
            X87_POP_OR_FAIL(dyn, ninst, x3);
            break;
        case 0xF0:
        case 0xF1:
        case 0xF2:
        case 0xF3:
        case 0xF4:
        case 0xF5:
        case 0xF6:
        case 0xF7:
            INST_NAME("FDIVRP STx, ST0");
            v2 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v1 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            if(ST_IS_F(0)) {
                FDIVS(v1, v2, v1);
            } else {
                FDIVD(v1, v2, v1);
            }
            X87_POP_OR_FAIL(dyn, ninst, x3);
            break;
        case 0xF8:
        case 0xF9:
        case 0xFA:
        case 0xFB:
        case 0xFC:
        case 0xFD:
        case 0xFE:
        case 0xFF:
            INST_NAME("FDIVP STx, ST0");
            v2 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v1 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            if(ST_IS_F(0)) {
                FDIVS(v1, v1, v2);
            } else {
                FDIVD(v1, v1, v2);
            }
            X87_POP_OR_FAIL(dyn, ninst, x3);
            break;
        default:
            DEFAULT;
            break;
    } else
        switch((nextop>>3)&7) {
            case 0:
                INST_NAME("FIADD ST0, word[ED]");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                v2 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, &unscaled, 0xfff<<1, 1, rex, NULL, 0, 0);
                VLD16(v2, wback, fixedaddress);
                SXTL_16(v2, v2);
                SXTL_32(v2, v2);
                SCVTFDD(v2, v2);
                FADDD(v1, v1, v2);
                break;
            case 1:
                INST_NAME("FIMUL ST0, word[ED]");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                v2 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, &unscaled, 0xfff<<1, 1, rex, NULL, 0, 0);
                VLD16(v2, wback, fixedaddress);
                SXTL_16(v2, v2);
                SXTL_32(v2, v2);
                SCVTFDD(v2, v2);
                FMULD(v1, v1, v2);
                break;
            case 2:
                INST_NAME("FICOM ST0, word[ED]");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                v2 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, &unscaled, 0xfff<<1, 1, rex, NULL, 0, 0);
                VLD16(v2, wback, fixedaddress);
                SXTL_16(v2, v2);
                SXTL_32(v2, v2);
                SCVTFDD(v2, v2);
                FCMPD(v1, v2);
                FCOM(x1, x2, x3);
                break;
            case 3:
                INST_NAME("FICOMP ST0, word[ED]");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                v2 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, &unscaled, 0xfff<<1, 1, rex, NULL, 0, 0);
                VLD16(v2, wback, fixedaddress);
                SXTL_16(v2, v2);
                SXTL_32(v2, v2);
                SCVTFDD(v2, v2);
                FCMPD(v1, v2);
                FCOM(x1, x2, x3);
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            case 4:
                INST_NAME("FISUB ST0, word[ED]");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                v2 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, &unscaled, 0xfff<<1, 1, rex, NULL, 0, 0);
                VLD16(v2, wback, fixedaddress);
                SXTL_16(v2, v2);
                SXTL_32(v2, v2);
                SCVTFDD(v2, v2);
                FSUBD(v1, v1, v2);
                break;
            case 5:
                INST_NAME("FISUBR ST0, word[ED]");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                v2 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, &unscaled, 0xfff<<1, 1, rex, NULL, 0, 0);
                VLD16(v2, wback, fixedaddress);
                SXTL_16(v2, v2);
                SXTL_32(v2, v2);
                SCVTFDD(v2, v2);
                FSUBD(v1, v2, v1);
                break;
            case 6:
                INST_NAME("FIDIV ST0, word[ED]");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                v2 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, &unscaled, 0xfff<<1, 1, rex, NULL, 0, 0);
                VLD16(v2, wback, fixedaddress);
                SXTL_16(v2, v2);
                SXTL_32(v2, v2);
                SCVTFDD(v2, v2);
                FDIVD(v1, v1, v2);
                break;
            case 7:
                INST_NAME("FIDIVR ST0, word[ED]");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                v2 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, &unscaled, 0xfff<<1, 1, rex, NULL, 0, 0);
                VLD16(v2, wback, fixedaddress);
                SXTL_16(v2, v2);
                SXTL_32(v2, v2);
                SCVTFDD(v2, v2);
                FDIVD(v1, v2, v1);
                break;
        }
    return addr;
}
