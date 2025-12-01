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

#include "arm64_printer.h"
#include "dynarec_arm64_private.h"
#include "../dynarec_helper.h"
#include "dynarec_arm64_functions.h"


uintptr_t dynarec64_DA(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    uint8_t nextop = F8;
    int64_t j64;
    uint8_t ed;
    uint8_t wback;
    uint8_t u8;
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
            INST_NAME("FCMOVB ST0, STx");
            READFLAGS(X_CF);
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            TSTw_mask(xFlags, 0, 0);    //mask=1<<F_CF
            if(ST_IS_F(0)) {
                FCSELS(v1, v2, v1, cNE);   // F_CF==1
            } else {
                FCSELD(v1, v2, v1, cNE);   // F_CF==1
            }
            break;
        case 0xC8:
        case 0xC9:
        case 0xCA:
        case 0xCB:
        case 0xCC:
        case 0xCD:
        case 0xCE:
        case 0xCF:
            INST_NAME("FCMOVE ST0, STx");
            READFLAGS(X_ZF);
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            TSTw_mask(xFlags, 0b011010, 0); //mask=1<<F_ZF
            if(ST_IS_F(0)) {
                FCSELS(v1, v2, v1, cNE);
            } else {
                FCSELD(v1, v2, v1, cNE);        // F_ZF==0
            }
            break;
        case 0xD0:
        case 0xD1:
        case 0xD2:
        case 0xD3:
        case 0xD4:
        case 0xD5:
        case 0xD6:
        case 0xD7:
            INST_NAME("FCMOVBE ST0, STx");
            READFLAGS(X_CF|X_ZF);
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            MOV32w(x1, (1<<F_CF)|(1<<F_ZF));
            TSTw_REG(xFlags, x1);
            if(ST_IS_F(0)) {
                FCSELS(v1, v2, v1, cNE);
            } else {
                FCSELD(v1, v2, v1, cNE);   // F_CF==0 & F_ZF==0
            }
            break;
        case 0xD8:
        case 0xD9:
        case 0xDA:
        case 0xDB:
        case 0xDC:
        case 0xDD:
        case 0xDE:
        case 0xDF:
            INST_NAME("FCMOVU ST0, STx");
            READFLAGS(X_PF);
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            TSTw_mask(xFlags, 0b011110, 0); //mask=1<<F_PF
            if(ST_IS_F(0)) {
                FCSELS(v1, v2, v1, cNE);
            } else {
                FCSELD(v1, v2, v1, cNE);        // F_PF==0
            }
            break;
        case 0xE9:
            INST_NAME("FUCOMPP ST0, ST1");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, 1, X87_COMBINE(0, nextop&7));
            if(ST_IS_F(0)) {
                FCMPS(v1, v2);
            } else {
                FCMPD(v1, v2);
            }
            FCOM(x1, x2, x3);
            X87_POP_OR_FAIL(dyn, ninst, x3);
            X87_POP_OR_FAIL(dyn, ninst, x3);
            break;

        default:
            DEFAULT;
            break;
    } else
        switch((nextop>>3)&7) {
            case 0:
                INST_NAME("FIADD ST0, Ed");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                v2 = fpu_get_scratch(dyn, ninst);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff<<2, 3, rex, NULL, 0, 0);
                VLD32(v2, ed, fixedaddress);
                SXTL_32(v2, v2);    // i32 -> i64
                SCVTFDD(v2, v2);    // i64 -> double
                if(!BOX64ENV(dynarec_fastround))
                    u8 = x87_setround(dyn, ninst, x1, x5, x4);
                FADDD(v1, v1, v2);
                X87_CHECK_PRECISION(v1);
                if(!BOX64ENV(dynarec_fastround))
                    x87_restoreround(dyn, ninst, u8);
                break;
            case 1:
                INST_NAME("FIMUL ST0, Ed");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                v2 = fpu_get_scratch(dyn, ninst);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff<<2, 3, rex, NULL, 0, 0);
                VLD32(v2, ed, fixedaddress);
                SXTL_32(v2, v2);    // i32 -> i64
                SCVTFDD(v2, v2);    // i64 -> double
                if(!BOX64ENV(dynarec_fastround))
                    u8 = x87_setround(dyn, ninst, x1, x5, x4);
                FMULD(v1, v1, v2);
                X87_CHECK_PRECISION(v1);
                if(!BOX64ENV(dynarec_fastround))
                    x87_restoreround(dyn, ninst, u8);
                break;
            case 2:
                INST_NAME("FICOM ST0, Ed");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                v2 = fpu_get_scratch(dyn, ninst);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff<<2, 3, rex, NULL, 0, 0);
                VLD32(v2, ed, fixedaddress);
                SXTL_32(v2, v2);    // i32 -> i64
                SCVTFDD(v2, v2);    // i64 -> double
                FCMPD(v1, v2);
                FCOM(x1, x2, x3);
                break;
            case 3:
                INST_NAME("FICOMP ST0, Ed");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                v2 = fpu_get_scratch(dyn, ninst);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff<<2, 3, rex, NULL, 0, 0);
                VLD32(v2, ed, fixedaddress);
                SXTL_32(v2, v2);    // i32 -> i64
                SCVTFDD(v2, v2);    // i64 -> double
                FCMPD(v1, v2);
                FCOM(x1, x2, x3);
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            case 4:
                INST_NAME("FISUB ST0, Ed");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                v2 = fpu_get_scratch(dyn, ninst);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff<<2, 3, rex, NULL, 0, 0);
                VLD32(v2, ed, fixedaddress);
                SXTL_32(v2, v2);    // i32 -> i64
                SCVTFDD(v2, v2);    // i64 -> double
                if(!BOX64ENV(dynarec_fastround))
                    u8 = x87_setround(dyn, ninst, x1, x5, x4);
                FSUBD(v1, v1, v2);
                X87_CHECK_PRECISION(v1);
                if(!BOX64ENV(dynarec_fastround))
                    x87_restoreround(dyn, ninst, u8);
                break;
            case 5:
                INST_NAME("FISUBR ST0, Ed");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                v2 = fpu_get_scratch(dyn, ninst);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff<<2, 3, rex, NULL, 0, 0);
                VLD32(v2, ed, fixedaddress);
                SXTL_32(v2, v2);    // i32 -> i64
                SCVTFDD(v2, v2);    // i64 -> double
                if(!BOX64ENV(dynarec_fastround))
                    u8 = x87_setround(dyn, ninst, x1, x5, x4);
                FSUBD(v1, v2, v1);
                X87_CHECK_PRECISION(v1);
                if(!BOX64ENV(dynarec_fastround))
                    x87_restoreround(dyn, ninst, u8);
                break;
            case 6:
                INST_NAME("FIDIV ST0, Ed");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                v2 = fpu_get_scratch(dyn, ninst);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff<<2, 3, rex, NULL, 0, 0);
                VLD32(v2, ed, fixedaddress);
                SXTL_32(v2, v2);    // i32 -> i64
                SCVTFDD(v2, v2);    // i64 -> double
                if(!BOX64ENV(dynarec_fastround))
                    u8 = x87_setround(dyn, ninst, x1, x5, x4);
                FDIVD(v1, v1, v2);
                X87_CHECK_PRECISION(v1);
                if(!BOX64ENV(dynarec_fastround))
                    x87_restoreround(dyn, ninst, u8);
                break;
            case 7:
                INST_NAME("FIDIVR ST0, Ed");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                v2 = fpu_get_scratch(dyn, ninst);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff<<2, 3, rex, NULL, 0, 0);
                VLD32(v2, ed, fixedaddress);
                SXTL_32(v2, v2);    // i32 -> i64
                SCVTFDD(v2, v2);    // i64 -> double
                if(!BOX64ENV(dynarec_fastround))
                    u8 = x87_setround(dyn, ninst, x1, x5, x4);
                FDIVD(v1, v2, v1);
                X87_CHECK_PRECISION(v1);
                if(!BOX64ENV(dynarec_fastround))
                    x87_restoreround(dyn, ninst, u8);
                break;
        }
    return addr;
}

