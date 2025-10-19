#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "la64_emitter.h"
#include "la64_mapping.h"
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


uintptr_t dynarec64_D9(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
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
    int v0, v1, v2;
    int s0;
    int i1, i2, i3;
    int64_t j64;

    MAYUSE(s0);
    MAYUSE(v0);
    MAYUSE(v1);
    MAYUSE(v2);
    MAYUSE(j64);

    if (MODREG)
        switch (nextop) {
            case 0xC0 ... 0xC7:
                INST_NAME("FLD STx");
                X87_PUSH_OR_FAIL(v2, dyn, ninst, x1, X87_ST(nextop & 7));
                v1 = x87_get_st(dyn, ninst, x1, x2, (nextop & 7) + 1, X87_COMBINE(0, (nextop & 7) + 1));
                if (ST_IS_F(0)) {
                    FMOV_S(v2, v1);
                } else {
                    FMOV_D(v2, v1);
                }
                break;

            case 0xC8:
                INST_NAME("FXCH ST0");
                break;
            case 0xC9 ... 0xCF:
                INST_NAME("FXCH STx");
                // swap the cache value, not the double value itself :p
                x87_get_st(dyn, ninst, x1, x2, nextop & 7, X87_ST(nextop & 7));
                x87_get_st(dyn, ninst, x1, x2, 0, X87_ST0);
                x87_swapreg(dyn, ninst, x1, x2, 0, nextop & 7);
                // should set C1 to 0
                break;

            case 0xD0:
                INST_NAME("FNOP");
                break;

            case 0xD8:
                INST_NAME("FSTPNCE ST0, ST0");
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            case 0xD9 ... 0xDF:
                INST_NAME("FSTPNCE ST0, STx");
                // copy the cache value for st0 to stx
                x87_get_st_empty(dyn, ninst, x1, x2, nextop & 7, X87_ST(nextop & 7));
                x87_get_st(dyn, ninst, x1, x2, 0, X87_ST0);
                x87_swapreg(dyn, ninst, x1, x2, 0, nextop & 7);
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            case 0xE0:
                INST_NAME("FCHS");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_ST0);
                if (ST_IS_F(0)) {
                    FNEG_S(v1, v1);
                } else {
                    FNEG_D(v1, v1);
                }
                break;
            case 0xE1:
                INST_NAME("FABS");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_ST0);
                if (ST_IS_F(0)) {
                    FABS_S(v1, v1);
                } else {
                    FABS_D(v1, v1);
                }
                break;

            case 0xE4:
                INST_NAME("FTST");
                DEFAULT;
                break;
            case 0xE5:
                INST_NAME("FXAM");
#if 1
                i1 = x87_get_current_cache(dyn, ninst, 0, LSX_CACHE_ST_D);
                // value put in x4
                if (i1 == -1) {
                    if (fpu_is_st_freed(dyn, ninst, 0)) {
                        MOV32w(x4, 0b100000100000000);
                        B_MARK3_nocond;
                    } else {
                        // not in cache, so check Empty status and load it
                        i2 = -dyn->lsx.x87stack;
                        LD_WU(x3, xEmu, offsetof(x64emu_t, fpu_stack));
                        if (i2) {
                            ADDI_D(x3, x3, -i2);
                        }
                        MOV32w(x4, 0b100000100000000); // empty: C3,C2,C0 = 101
                        BGE_MARK3(xZR, x3);
                        // x5 will be the actual top
                        LD_WU(x5, xEmu, offsetof(x64emu_t, top));
                        if (i2) {
                            ADDI_D(x5, x5, i2);
                            ANDI(x5, x5, 7); // (emu->top + i)&7
                        }
                        // load x2 with ST0 anyway, for sign extraction
                        SLLI_D(x5, x5, 3);
                        ADD_D(x1, xEmu, x5);
                        LD_D(x2, x1, offsetof(x64emu_t, x87));
                        // load tag
                        if (i2 >= 0) {
                            LD_HU(x3, xEmu, offsetof(x64emu_t, fpu_tags));
                            if (i2 > 0) {
                                BSTRINS_D(x3, xZR, 15, 0);
                                SRLI_D(x3, x3, i2 * 2);
                            }
                            ANDI(x3, x3, 0b11);
                            BNEZ_MARK3(x3); // empty: C3,C2,C0 = 101
                        }
                    }
                } else {
                    // simply move from cache reg to x2
                    v1 = dyn->lsx.x87reg[i1];
                    MOVFR2GR_D(x2, v1);
                }
                // get exponant in x1
                SRLI_D(x1, x2, 20 + 32);
                ANDI(x1, x1, 0x7ff); // 0x7ff
                BNEZ_MARK(x1);       // not zero or denormal
                MOV64x(x3, 0x7fffffffffffffff);
                AND(x1, x2, x3);
                MOV32w(x4, 0b100000000000000); // Zero: C3,C2,C0 = 100
                BEQZ_MARK3(x1);
                MOV32w(x4, 0b100010000000000); // Denormal: C3,C2,C0 = 110
                B_MARK3_nocond;
                MARK;
                ADDI_D(x3, xZR, 0x7ff);        // infinite/NaN?
                MOV32w(x4, 0b000010000000000); // normal: C3,C2,C0 = 010
                BNE_MARK3(x1, x3);
                SLLI_D(x3, x2, 12);
                SRLI_D(x3, x3, 12);            // and 0x000fffffffffffff
                MOV32w(x4, 0b000010100000000); // infinity: C3,C2,C0 = 011
                BEQZ_MARK3(x3);
                MOV32w(x4, 0b000000100000000); // NaN: C3,C2,C0 = 001
                MARK3;
                // Extract sign & Update SW
                SRLI_D(x1, x2, 63);
                SLLI_D(x1, x1, 9);
                OR(x4, x4, x1); // C1
                LD_HU(x1, xEmu, offsetof(x64emu_t, sw));
                MOV32w(x2, ~0b0100011100000000);
                AND(x1, x1, x2);
                OR(x4, x4, x1);
                ST_H(x4, xEmu, offsetof(x64emu_t, sw));
#else
                MESSAGE(LOG_DUMP, "Need Optimization\n");
                x87_refresh(dyn, ninst, x1, x2, 0);
                s0 = x87_stackcount(dyn, ninst, x1);
                CALL(fpu_fxam, -1, 0, 0); // should be possible inline, but is it worth it?
                x87_unstackcount(dyn, ninst, x1, s0);
#endif
                break;

            case 0xE8:
                INST_NAME("FLD1");
                X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, LSX_CACHE_ST_F);
                if (ST_IS_F(0)) {
                    MOV32w(x1, 0x3f800000);
                    MOVGR2FR_W(v1, x1);
                } else {
                    MOV64x(x1, 0x3FF0000000000000);
                    MOVGR2FR_D(v1, x1);
                }
                break;
            case 0xE9:
                INST_NAME("FLDL2T");
                X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, LSX_CACHE_ST_D);
                FTABLE64(v1, L2T);
                break;
            case 0xEA:
                INST_NAME("FLDL2E");
                X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, LSX_CACHE_ST_D);
                FTABLE64(v1, L2E);
                break;
            case 0xEB:
                INST_NAME("FLDPI");
                X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, LSX_CACHE_ST_D);
                FTABLE64(v1, PI);
                break;
            case 0xEC:
                INST_NAME("FLDLG2");
                X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, LSX_CACHE_ST_D);
                FTABLE64(v1, LG2);
                break;
            case 0xED:
                INST_NAME("FLDLN2");
                X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, LSX_CACHE_ST_D);
                FTABLE64(v1, LN2);
                break;
            case 0xEE:
                INST_NAME("FLDZ");
                X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, LSX_CACHE_ST_F);
                if (ST_IS_F(0))
                    MOVGR2FR_W(v1, xZR);
                else
                    MOVGR2FR_D(v1, xZR);
                break;

            case 0xF0:
                INST_NAME("F2XM1");
                MESSAGE(LOG_DUMP, "Need Optimization\n");
                x87_forget(dyn, ninst, x1, x2, 0);
                s0 = x87_stackcount(dyn, ninst, x3);
                CALL(const_native_f2xm1, -1, 0, 0);
                x87_unstackcount(dyn, ninst, x3, s0);
                break;
            case 0xF1:
                INST_NAME("FYL2X");
                MESSAGE(LOG_DUMP, "Need Optimization\n");
                x87_forget(dyn, ninst, x1, x2, 0);
                x87_forget(dyn, ninst, x1, x2, 1);
                s0 = x87_stackcount(dyn, ninst, x3);
                CALL(const_native_fyl2x, -1, 0, 0);
                x87_unstackcount(dyn, ninst, x3, s0);
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            case 0xF2:
                INST_NAME("FPTAN");
                MESSAGE(LOG_DUMP, "Need Optimization\n");
                x87_forget(dyn, ninst, x1, x2, 0);
                s0 = x87_stackcount(dyn, ninst, x3);
                if (!BOX64ENV(dynarec_fastround)) u8 = x87_setround(dyn, ninst, x1, x2);
                CALL_(const_native_ftan, -1, BOX64ENV(dynarec_fastround) ? 0 : u8, 0, 0);
                if (!BOX64ENV(dynarec_fastround)) x87_restoreround(dyn, ninst, u8);
                x87_unstackcount(dyn, ninst, x3, s0);
                X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, LSX_CACHE_ST_F);
                if (ST_IS_F(0)) {
                    MOV32w(x1, 0x3f800000);
                    MOVGR2FR_W(v1, x1);
                } else {
                    MOV64x(x1, 0x3FF0000000000000);
                    MOVGR2FR_D(v1, x1);
                }
                break;
            case 0xF3:
                INST_NAME("FPATAN");
                MESSAGE(LOG_DUMP, "Need Optimization\n");
                x87_forget(dyn, ninst, x1, x2, 0);
                x87_forget(dyn, ninst, x1, x2, 1);
                s0 = x87_stackcount(dyn, ninst, x3);
                if (!BOX64ENV(dynarec_fastround)) u8 = x87_setround(dyn, ninst, x1, x2);
                CALL_(const_native_fpatan, -1, BOX64ENV(dynarec_fastround) ? 0 : u8, 0, 0);
                if (!BOX64ENV(dynarec_fastround)) x87_restoreround(dyn, ninst, u8);
                x87_unstackcount(dyn, ninst, x3, s0);
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            case 0xF4:
                INST_NAME("FXTRACT");
                MESSAGE(LOG_DUMP, "Need Optimization\n");
                X87_PUSH_EMPTY_OR_FAIL(dyn, ninst, x3);
                x87_forget(dyn, ninst, x1, x2, 1);
                s0 = x87_stackcount(dyn, ninst, x3);
                CALL(const_native_fxtract, -1, 0, 0);
                x87_unstackcount(dyn, ninst, x3, s0);
                break;
            case 0xF5:
                INST_NAME("FPREM1");
                MESSAGE(LOG_DUMP, "Need Optimization\n");
                x87_forget(dyn, ninst, x1, x2, 0);
                x87_forget(dyn, ninst, x1, x2, 1);
                s0 = x87_stackcount(dyn, ninst, x3);
                CALL(const_native_fprem1, -1, 0, 0);
                x87_unstackcount(dyn, ninst, x3, s0);
                break;
            case 0xF6:
                INST_NAME("FDECSTP");
                fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
                LD_W(x2, xEmu, offsetof(x64emu_t, top));
                ADDI_D(x2, x2, -1);
                ANDI(x2, x2, 7);
                ST_W(x2, xEmu, offsetof(x64emu_t, top));
                break;
            case 0xF7:
                INST_NAME("FINCSTP");
                fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
                LD_W(x2, xEmu, offsetof(x64emu_t, top));
                ADDI_D(x2, x2, 1);
                ANDI(x2, x2, 7);
                ST_W(x2, xEmu, offsetof(x64emu_t, top));
                break;
            case 0xF8:
                INST_NAME("FPREM");
                MESSAGE(LOG_DUMP, "Need Optimization\n");
                x87_forget(dyn, ninst, x1, x2, 0);
                x87_forget(dyn, ninst, x1, x2, 1);
                s0 = x87_stackcount(dyn, ninst, x3);
                CALL(const_native_fprem, -1, 0, 0);
                x87_unstackcount(dyn, ninst, x3, s0);
                break;
            case 0xF9:
                INST_NAME("FYL2XP1");
                MESSAGE(LOG_DUMP, "Need Optimization\n");
                x87_forget(dyn, ninst, x1, x2, 0);
                x87_forget(dyn, ninst, x1, x2, 1);
                s0 = x87_stackcount(dyn, ninst, x3);
                CALL(const_native_fyl2xp1, -1, 0, 0);
                x87_unstackcount(dyn, ninst, x3, s0);
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            case 0xFA:
                INST_NAME("FSQRT");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_ST0);
                if (!BOX64ENV(dynarec_fastround)) u8 = x87_setround(dyn, ninst, x1, x2);
                if (ST_IS_F(0)) {
                    FSQRT_S(v1, v1);
                } else {
                    FSQRT_D(v1, v1);
                }
                X87_CHECK_PRECISION(v1);
                if (!BOX64ENV(dynarec_fastround)) x87_restoreround(dyn, ninst, u8);
                break;
            case 0xFB:
                INST_NAME("FSINCOS");
                MESSAGE(LOG_DUMP, "Need Optimization\n");
                X87_PUSH_EMPTY_OR_FAIL(dyn, ninst, x3);
                x87_forget(dyn, ninst, x1, x2, 1);
                s0 = x87_stackcount(dyn, ninst, x3);
                if (!BOX64ENV(dynarec_fastround)) u8 = x87_setround(dyn, ninst, x1, x2);
                CALL_(const_native_fsincos, -1, BOX64ENV(dynarec_fastround) ? 0 : u8, 0, 0);
                if (!BOX64ENV(dynarec_fastround)) x87_restoreround(dyn, ninst, u8);
                x87_unstackcount(dyn, ninst, x3, s0);
                break;
            case 0xFC:
                INST_NAME("FRNDINT");
                v0 = x87_get_st(dyn, ninst, x1, x2, 0, X87_ST0);
                v1 = fpu_get_scratch(dyn);
                v2 = fpu_get_scratch(dyn);
                u8 = x87_setround(dyn, ninst, x1, x2);

                if (ST_IS_F(0)) {
                    FCMP_S(fcc0, v0, v0, cEQ);
                    BCNEZ_MARK(fcc0);
                    B_NEXT_nocond;
                    MARK; // v0 is not nan
                    FABS_S(v1, v0);
                    MOV64x(x3, 1ULL << __FLT_MANT_DIG__);
                    MOVGR2FR_W(v2, x3);
                    FFINT_S_L(v2, v2);
                    FCMP_S(fcc1, v1, v2, cLT);
                    BCNEZ_MARK2(fcc1);
                    B_NEXT_nocond;
                    MARK2;
                    FTINT_L_S(v1, v0);
                    FFINT_S_L(v1, v1);
                    FCOPYSIGN_S(v0, v1, v0);
                } else {
                    FCMP_D(fcc0, v0, v0, cEQ);
                    BCNEZ_MARK(fcc0);
                    B_NEXT_nocond;
                    MARK; // v0 is not nan
                    FABS_D(v1, v0);
                    MOV64x(x3, 1ULL << __DBL_MANT_DIG__);
                    MOVGR2FR_D(v2, x3);
                    FFINT_D_L(v2, v2);
                    FCMP_D(fcc1, v1, v2, cLT);
                    BCNEZ_MARK2(fcc1);
                    B_NEXT_nocond;
                    MARK2;
                    FTINT_L_D(v1, v0);
                    FFINT_D_L(v1, v1);
                    FCOPYSIGN_D(v0, v1, v0);
                }
                x87_restoreround(dyn, ninst, u8);
                break;
            case 0xFD:
                INST_NAME("FSCALE");
                MESSAGE(LOG_DUMP, "Need Optimization\n");
                x87_forget(dyn, ninst, x1, x2, 0);
                x87_forget(dyn, ninst, x1, x2, 1);
                s0 = x87_stackcount(dyn, ninst, x3);
                if (!BOX64ENV(dynarec_fastround)) u8 = x87_setround(dyn, ninst, x1, x2);
                CALL_(const_native_fscale, -1, BOX64ENV(dynarec_fastround) ? 0 : u8, 0, 0);
                if (!BOX64ENV(dynarec_fastround)) x87_restoreround(dyn, ninst, u8);
                x87_unstackcount(dyn, ninst, x3, s0);
                break;
            case 0xFE:
                INST_NAME("FSIN");
                MESSAGE(LOG_DUMP, "Need Optimization\n");
                x87_forget(dyn, ninst, x1, x2, 0);
                s0 = x87_stackcount(dyn, ninst, x3);
                if (!BOX64ENV(dynarec_fastround)) u8 = x87_setround(dyn, ninst, x1, x2);
                CALL_(const_native_fsin, -1, BOX64ENV(dynarec_fastround) ? 0 : u8, 0, 0);
                if (!BOX64ENV(dynarec_fastround)) x87_restoreround(dyn, ninst, u8);
                x87_unstackcount(dyn, ninst, x3, s0);
                break;
            case 0xFF:
                INST_NAME("FCOS");
                MESSAGE(LOG_DUMP, "Need Optimization\n");
                x87_forget(dyn, ninst, x1, x2, 0);
                s0 = x87_stackcount(dyn, ninst, x3);
                if (!BOX64ENV(dynarec_fastround)) u8 = x87_setround(dyn, ninst, x1, x2);
                CALL_(const_native_fcos, -1, BOX64ENV(dynarec_fastround) ? 0 : u8, 0, 0);
                if (!BOX64ENV(dynarec_fastround)) x87_restoreround(dyn, ninst, u8);
                x87_unstackcount(dyn, ninst, x3, s0);
                break;
            default:
                DEFAULT;
                break;
        }
    else
        switch ((nextop >> 3) & 7) {
            case 0:
                INST_NAME("FLD ST0, float[ED]");
                X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, (BOX64ENV(dynarec_x87double) == 1) ? LSX_CACHE_ST_D : LSX_CACHE_ST_F);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                FLD_S(v1, ed, fixedaddress);
                if (!ST_IS_F(0)) {
                    FCVT_D_S(v1, v1);
                }
                break;
            case 2:
                INST_NAME("FST float[ED], ST0");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, LSX_CACHE_ST_F);
                if (ST_IS_F(0))
                    s0 = v1;
                else {
                    s0 = fpu_get_scratch(dyn);
                    if (!BOX64ENV(dynarec_fastround)) u8 = x87_setround(dyn, ninst, x1, x2);
                    FCVT_S_D(s0, v1);
                    if (!BOX64ENV(dynarec_fastround)) x87_restoreround(dyn, ninst, u8);
                }
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                FST_S(s0, ed, fixedaddress);
                break;
            case 3:
                INST_NAME("FSTP float[ED], ST0");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, LSX_CACHE_ST_F);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                if (!ST_IS_F(0)) {
                    if (!BOX64ENV(dynarec_fastround)) u8 = x87_setround(dyn, ninst, x1, x3);
                    FCVT_S_D(v1, v1);
                    if (!BOX64ENV(dynarec_fastround)) x87_restoreround(dyn, ninst, u8);
                }
                FST_S(v1, ed, fixedaddress);
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            case 4:
                INST_NAME("FLDENV Ed");
                MESSAGE(LOG_DUMP, "Need Optimization\n");
                fpu_purgecache(dyn, ninst, 0, x1, x2, x3); // maybe only x87, not SSE?
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 0, 0);
                MOV32w(x2, 0);
                CALL(const_fpu_loadenv, -1, ed, x2);
                NATIVE_RESTORE_X87PC();
                break;
            case 5:
                INST_NAME("FLDCW Ew");
                GETEW(x1, 0);
                ST_H(x1, xEmu, offsetof(x64emu_t, cw)); // hopefully cw is not too far for an imm8
                if (dyn->need_x87check) {
                    SRLI_D(x87pc, x1, 8);
                    ANDI(x87pc, x87pc, 0b11);
                }
                break;
            case 6:
                INST_NAME("FNSTENV Ed");
                MESSAGE(LOG_DUMP, "Need Optimization\n");
                fpu_purgecache(dyn, ninst, 0, x1, x2, x3); // maybe only x87, not SSE?
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 0, 0);
                MOV32w(x2, 0);
                CALL(const_fpu_savenv, -1, ed, x2);
                break;
            case 7:
                INST_NAME("FNSTCW Ew");
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x1, &fixedaddress, rex, NULL, 0, 0);
                ed = x1;
                wb1 = 1;
                LD_H(x1, xEmu, offsetof(x64emu_t, cw));
                EWBACK;
                break;
            default:
                DEFAULT;
        }
    return addr;
}
