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


uintptr_t dynarec64_D9(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
{
    (void)ip; (void)rep; (void)need_epilog;

    uint8_t nextop = F8;
    uint8_t ed;
    uint8_t wback, wb1;
    uint8_t u8;
    int64_t fixedaddress, j64;
    int unscaled;
    int v1, v2;
    int s0;
    int i1, i2, i3;

    MAYUSE(s0);
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
            INST_NAME("FLD STx");
            X87_PUSH_OR_FAIL(v2, dyn, ninst, x1, X87_ST(nextop&7));
            v1 = x87_get_st(dyn, ninst, x1, x2, (nextop&7)+1, X87_COMBINE(0, (nextop&7)+1));
            if(ST_IS_F(0)) {
                FMOVS(v2, v1);
            } else {
                FMOVD(v2, v1);
            }
            break;

        case 0xC8:
            INST_NAME("FXCH ST0");
            break;
        case 0xC9:
        case 0xCA:
        case 0xCB:
        case 0xCC:
        case 0xCD:
        case 0xCE:
        case 0xCF:
            INST_NAME("FXCH STx");
            // swap the cache value, not the double value itself :p
            x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_ST(nextop&7));
            x87_get_st(dyn, ninst, x1, x2, 0, X87_ST0);
            x87_swapreg(dyn, ninst, x1, x2, 0, nextop&7);
            // should set C1 to 0
            break;

        case 0xD0:
            INST_NAME("FNOP");
            break;

        case 0xD8:
            INST_NAME("FSTPNCE ST0, ST0");
            X87_POP_OR_FAIL(dyn, ninst, x3);
            break;
        case 0xD9:
        case 0xDA:
        case 0xDB:
        case 0xDC:
        case 0xDD:
        case 0xDE:
        case 0xDF:
            INST_NAME("FSTPNCE ST0, STx");
            // copy the cache value for st0 to stx
            x87_get_st_empty(dyn, ninst, x1, x2, nextop&7, X87_ST(nextop&7));
            x87_get_st(dyn, ninst, x1, x2, 0, X87_ST0);
            x87_swapreg(dyn, ninst, x1, x2, 0, nextop&7);
            X87_POP_OR_FAIL(dyn, ninst, x3);
            break;
        case 0xE0:
            INST_NAME("FCHS");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_ST0);
            if(ST_IS_F(0)) {
                FNEGS(v1, v1);
            } else {
                FNEGD(v1, v1);
            }
            break;
        case 0xE1:
            INST_NAME("FABS");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_ST0);
            if(ST_IS_F(0)) {
                FABSS(v1, v1);
            } else {
                FABSD(v1, v1);
            }
            break;

        case 0xE4:
            INST_NAME("FTST");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_ST0);
            if(ST_IS_F(0)) {
                FCMPS_0(v1);
            } else {
                FCMPD_0(v1);
            }
            FCOM(x1, x2, x3);   // same flags...
            break;
        case 0xE5:
            INST_NAME("FXAM");
            #if 1
            i1 = x87_get_current_cache(dyn, ninst, 0, NEON_CACHE_ST_D);
            // value put in x14
            if(i1==-1) {
                if(fpu_is_st_freed(dyn, ninst, 0)) {
                    MOV32w(x4, 0b100000100000000);
                    B_MARK3_nocond;
                } else {
                    // not in cache, so check Empty status and load it
                    i2 = -dyn->n.x87stack;
                    LDRw_U12(x3, xEmu, offsetof(x64emu_t, fpu_stack));
                    if(i2) {
                        if(i2<0) {
                            ADDw_U12(x3, x3, -i2);
                        } else {
                            SUBw_U12(x3, x3, i2);
                        }
                    }
                    CMPSw_U12(x3, 0);
                    MOV32w(x4, 0b100000100000000);  // empty: C3,C2,C0 = 101
                    B_MARK3(cLE);
                    // x5 will be the actual top
                    LDRw_U12(x5, xEmu, offsetof(x64emu_t, top));
                    if(i2) {
                        if(i2<0) {
                            SUBw_U12(x5, x5, -i2);
                        } else {
                            ADDw_U12(x5, x5, i2);
                        }
                        ANDw_mask(x5, x5, 0, 3);    // (emu->top + i)&7
                    }
                    // load tag
                    LDRH_U12(x3, xEmu, offsetof(x64emu_t, fpu_tags));
                    TSTw_mask(x3, 0, 1);    // 0b11
                    B_MARK3(cNE);   // empty: C3,C2,C0 = 101
                    // load x2 with ST0 anyway, for sign extraction
                    ADDx_REG_LSL(x1, xEmu, x5, 3);
                    LDRx_U12(x2, x1, offsetof(x64emu_t, x87));
                }
            } else {
                // simply move from cache reg to x2
                v1 = dyn->n.x87reg[i1];
                VMOVQDto(x2, v1, 0);
            }
            // get exponant in x1
            LSRx_IMM(x1, x2, 20+32);
            ANDSx_mask(x1, x1, 1, 0b00000, 0b001010); // 0x7ff
            B_MARK(cNE); // not zero or denormal
            ANDSx_mask(x1, x2, 1, 0, 0b111110); // 0x7fffffffffffffff
            MOV32w(x4, 0b100000000000000); // Zero: C3,C2,C0 = 100
            MOV32w(x5, 0b100010000000000); // Denormal: C3,C2,C0 = 110
            CSELx(x4, x4, x5, cEQ);
            B_MARK3(c__);
            MARK;
            CMPSx_U12(x1, 0x7ff);   // infinite/NaN?
            MOV32w(x5, 0b000010000000000); // normal: C3,C2,C0 = 010
            CSELx(x4, x5, x4, cNE);
            B_MARK3(cNE);
            TSTx_mask(x2, 1, 0b000000, 0b110011); // 0x000fffffffffffff
            MOV32w(x4, 0b000010100000000); // infinity: C3,C2,C0 = 011
            MOV32w(x3, 0b000000100000000); // NaN: C3,C2,C0 = 001
            CSELx(x4, x4, x3, cEQ);
            MARK3;
            // Extract signa & Update SW
            LSRx_IMM(x1, x2, 63);
            BFIx(x4, x1, 9, 1); //C1
            LDRH_U12(x1, xEmu, offsetof(x64emu_t, sw));
            MOV32w(x2, 0b01000111);
            BICw_REG_LSL(x1, x1, x2, 8);
            ORRw_REG(x4, x4, x1);
            STRH_U12(x4, xEmu, offsetof(x64emu_t, sw));
            #else
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            i1 = x87_stackcount(dyn, ninst, x1);
            x87_forget(dyn, ninst, x1, x2, 0);
            CALL(fpu_fxam, -1);  // should be possible inline, but is it worth it?
            x87_unstackcount(dyn, ninst, x1, i1);
            #endif
            break;

        case 0xE8:
            INST_NAME("FLD1");
            if(ninst<dyn->size+2 && (dyn->insts[ninst+1].pred_sz==1) && (dyn->insts[ninst+2].pred_sz==1)
                && PK(0)==0xD9 && PK(1)==0xE8
                && PK(2)==0xD9 && PK(3)==0xF3
            ) {
                MESSAGE(LOG_DUMP, "Hack for FLD1 FLD1 FPATAN");
                X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, NEON_CACHE_ST_F);
                FTABLE64(v1, PI/4.0);
                addr+=4;
            } else {
                X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, NEON_CACHE_ST_F);
                if(ST_IS_F(0)) {
                    FMOVS_8(v1, 0b01110000);
                } else {
                    FMOVD_8(v1, 0b01110000);
                }
            }
            break;
        case 0xE9:
            INST_NAME("FLDL2T");
            X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, NEON_CACHE_ST_D);
            FTABLE64(v1, L2T);
            break;
        case 0xEA:
            INST_NAME("FLDL2E");
            X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, NEON_CACHE_ST_D);
            FTABLE64(v1, L2E);
            break;
        case 0xEB:
            INST_NAME("FLDPI");
            X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, NEON_CACHE_ST_D);
            FTABLE64(v1, PI);
            break;
        case 0xEC:
            INST_NAME("FLDLG2");
            X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, NEON_CACHE_ST_D);
            FTABLE64(v1, LG2);
            break;
        case 0xED:
            INST_NAME("FLDLN2");
            X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, NEON_CACHE_ST_D);
            FTABLE64(v1, LN2);
            break;
        case 0xEE:
            INST_NAME("FLDZ");
            X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, NEON_CACHE_ST_F);
            VEOR(v1, v1, v1);
            break;

        case 0xF0:
            INST_NAME("F2XM1");
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            i1 = x87_stackcount(dyn, ninst, x1);
            x87_forget(dyn, ninst, x1, x2, 0);
            CALL(native_f2xm1, -1);
            x87_unstackcount(dyn, ninst, x1, i1);
            break;
        case 0xF1:
            INST_NAME("FYL2X");
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            i1 = x87_stackcount(dyn, ninst, x1);
            x87_forget(dyn, ninst, x1, x2, 0);
            x87_forget(dyn, ninst, x1, x2, 1);
            CALL(native_fyl2x, -1);
            x87_unstackcount(dyn, ninst, x1, i1);
            X87_POP_OR_FAIL(dyn, ninst, x3);
            break;
        case 0xF2:
            INST_NAME("FPTAN");
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            i1 = x87_stackcount(dyn, ninst, x1);
            x87_forget(dyn, ninst, x1, x2, 0);
            CALL(native_ftan, -1);
            x87_unstackcount(dyn, ninst, x1, i1);
            if(PK(0)==0xdd && PK(1)==0xd8) {
                MESSAGE(LOG_DUMP, "Optimized next DD D8 fstp st0, st0, not emiting 1\n");
                u8 = F8;
                u8 = F8;
            } else {
                X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, NEON_CACHE_ST_F);
                if(ST_IS_F(0)) {
                    FMOVS_8(v1, 0b01110000);
                } else {
                    FMOVD_8(v1, 0b01110000);
                }
            }
            break;
        case 0xF3:
            INST_NAME("FPATAN");
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            i1 = x87_stackcount(dyn, ninst, x1);
            x87_forget(dyn, ninst, x1, x2, 0);
            x87_forget(dyn, ninst, x1, x2, 1);
            CALL(native_fpatan, -1);
            x87_unstackcount(dyn, ninst, x1, i1);
            X87_POP_OR_FAIL(dyn, ninst, x3);
            break;
        case 0xF4:
            INST_NAME("FXTRACT");
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            X87_PUSH_EMPTY_OR_FAIL(dyn, ninst, 0);
            i1 = x87_stackcount(dyn, ninst, x1);
            x87_forget(dyn, ninst, x1, x2, 1);
            CALL(native_fxtract, -1);
            x87_unstackcount(dyn, ninst, x1, i1);
            break;
        case 0xF5:
            INST_NAME("FPREM1");
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            #if 0
            i1 = x87_stackcount(dyn, ninst, x1);
            x87_forget(dyn, ninst, x1, x2, 0);
            x87_forget(dyn, ninst, x1, x2, 1);
            CALL(native_fprem1, -1);
            x87_unstackcount(dyn, ninst, x1, i1);
            #else
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
            v2 = x87_get_st(dyn, ninst, x1, x2, 1, NEON_CACHE_ST_D);
            s0 = fpu_get_scratch(dyn);
            FDIVD(s0, v1, v2);
            FRINTRRD(s0, s0, 0b00); // Nearest == TieToEven?
            FCVTZSxD(x4, s0);
            FMULD(s0, s0, v2);
            FSUBD(v1, v1, s0);
            LDRw_U12(x1, xEmu, offsetof(x64emu_t, sw));
            // set C2 = 0
            BFCw(x1, 10, 1);
            // set C1 = Q0
            BFIw(x1, x4, 9, 1);
            // set C3 = Q1
            LSRx_IMM(x4, x4, 1);
            BFIw(x1, x4, 14, 1);
            // Set C0 = Q2
            LSRx(x4, x4, 1);
            BFIw(x1, x4, 8, 1);
            STRw_U12(x1, xEmu, offsetof(x64emu_t, sw));
            #endif
            break;
        case 0xF6:
            INST_NAME("FDECSTP");
            fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
            LDRw_U12(x2, xEmu, offsetof(x64emu_t, top));
            SUBw_U12(x2, x2, 1);
            ANDw_mask(x2, x2, 0, 2);    //mask=7
            STRw_U12(x2, xEmu, offsetof(x64emu_t, top));
            break;
        case 0xF7:
            INST_NAME("FINCSTP");
            fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
            LDRw_U12(x2, xEmu, offsetof(x64emu_t, top));
            ADDw_U12(x2, x2, 1);
            ANDw_mask(x2, x2, 0, 2);    //mask=7
            STRw_U12(x2, xEmu, offsetof(x64emu_t, top));
            break;
        case 0xF8:
            INST_NAME("FPREM");
            #if 0
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            i1 = x87_stackcount(dyn, ninst, x1);
            x87_forget(dyn, ninst, x1, x2, 0);
            x87_forget(dyn, ninst, x1, x2, 1);
            CALL(native_fprem, -1);
            x87_unstackcount(dyn, ninst, x1, i1);
            #else
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
            v2 = x87_get_st(dyn, ninst, x1, x2, 1, NEON_CACHE_ST_D);
            s0 = fpu_get_scratch(dyn);
            FDIVD(s0, v1, v2);
            FRINTZD(s0, s0);
            FCVTZSxD(x4, s0);
            FMULD(s0, s0, v2);
            FSUBD(v1, v1, s0);
            LDRw_U12(x1, xEmu, offsetof(x64emu_t, sw));
            // set C2 = 0
            BFCw(x1, 10, 1);
            // set C1 = Q0
            BFIw(x1, x4, 9, 1);
            // set C3 = Q1
            LSRx_IMM(x4, x4, 1);
            BFIw(x1, x4, 14, 1);
            // Set C0 = Q2
            LSRx(x4, x4, 1);
            BFIw(x1, x4, 8, 1);
            STRw_U12(x1, xEmu, offsetof(x64emu_t, sw));
            #endif
            break;
        case 0xF9:
            INST_NAME("FYL2XP1");
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            i1 = x87_stackcount(dyn, ninst, x1);
            x87_forget(dyn, ninst, x1, x2, 0);
            x87_forget(dyn, ninst, x1, x2, 1);
            CALL(native_fyl2xp1, -1);
            x87_unstackcount(dyn, ninst, x1, i1);
            X87_POP_OR_FAIL(dyn, ninst, x3);
            break;
        case 0xFA:
            INST_NAME("FSQRT");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_ST0);
            if(ST_IS_F(0)) {
                FSQRTS(v1, v1);
            } else {
                FSQRTD(v1, v1);
            }
            break;
        case 0xFB:
            INST_NAME("FSINCOS");
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            X87_PUSH_EMPTY_OR_FAIL(dyn, ninst, 0);
            i1 = x87_stackcount(dyn, ninst, x1);
            x87_forget(dyn, ninst, x1, x2, 1);
            CALL(native_fsincos, -1);
            x87_unstackcount(dyn, ninst, x1, i1);
            break;
        case 0xFC:
            INST_NAME("FRNDINT");
            #if 0
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            // use C helper for now, nothing staightforward is available
            x87_forget(dyn, ninst, x1, x2, 0);
            CALL(native_frndint, -1);
            #else
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_ST0);
            u8 = x87_setround(dyn, ninst, x1, x2, x3);
            if(ST_IS_F(0)) {
                FRINTIS(v1, v1);
            } else {
                FRINTID(v1, v1);
            }
            x87_restoreround(dyn, ninst, u8);
            #endif
            break;
        case 0xFD:
            INST_NAME("FSCALE");
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            i1 = x87_stackcount(dyn, ninst, x1);
            x87_forget(dyn, ninst, x1, x2, 0);
            x87_forget(dyn, ninst, x1, x2, 1);
            CALL(native_fscale, -1);
            x87_unstackcount(dyn, ninst, x1, i1);
            break;
        case 0xFE:
            INST_NAME("FSIN");
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            i1 = x87_stackcount(dyn, ninst, x1);
            x87_forget(dyn, ninst, x1, x2, 0);
            CALL(native_fsin, -1);
            x87_unstackcount(dyn, ninst, x1, i1);
            break;
        case 0xFF:
            INST_NAME("FCOS");
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            i1 = x87_stackcount(dyn, ninst, x1);
            x87_forget(dyn, ninst, x1, x2, 0);
            CALL(native_fcos, -1);
            x87_unstackcount(dyn, ninst, x1, i1);
            break;
        default:
            DEFAULT;
            break;
    } else
        switch((nextop>>3)&7) {
            case 0:
                INST_NAME("FLD ST0, float[ED]");
                X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, box64_dynarec_x87double?NEON_CACHE_ST_D:NEON_CACHE_ST_F);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff<<2, 3, rex, NULL, 0, 0);
                VLD32(v1, ed, fixedaddress);
                if(!ST_IS_F(0)) {
                    FCVT_D_S(v1, v1);
                }
                break;
            case 2:
                INST_NAME("FST float[ED], ST0");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_F);
                if(ST_IS_F(0))
                    s0 = v1;
                else {
                    s0 = fpu_get_scratch(dyn);
                    FCVT_S_D(s0, v1);
                }
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff<<2, 3, rex, NULL, 0, 0);
                VST32(s0, ed, fixedaddress);
                break;
            case 3:
                INST_NAME("FSTP float[ED], ST0");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_F);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff<<2, 3, rex, NULL, 0, 0);
                if(!ST_IS_F(0)) {
                    FCVT_S_D(v1, v1);
                }
                VST32(v1, ed, fixedaddress);
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            case 4:
                INST_NAME("FLDENV Ed");
                MESSAGE(LOG_DUMP, "Need Optimization\n");
                fpu_purgecache(dyn, ninst, 0, x1, x2, x3); // maybe only x87, not SSE?
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                if(ed!=x1) {
                    MOVx_REG(x1, ed);
                }
                MOV32w(x2, 0);
                CALL(fpu_loadenv, -1);
                break;
            case 5:
                INST_NAME("FLDCW Ew");
                GETEW(x1, 0);
                STRH_U12(x1, xEmu, offsetof(x64emu_t, cw));    // hopefully cw is not too far for an imm8
                break;
            case 6:
                INST_NAME("FNSTENV Ed");
                MESSAGE(LOG_DUMP, "Need Optimization\n");
                fpu_purgecache(dyn, ninst, 0, x1, x2, x3); // maybe only x87, not SSE?
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                if(ed!=x1) {
                    MOVx_REG(x1, ed);
                }
                MOV32w(x2, 0);
                CALL(fpu_savenv, -1);
                break;
            case 7:
                INST_NAME("FNSTCW Ew");
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, &unscaled, 0xfff<<1, 1, rex, NULL, 0, 0);
                ed = x1;
                wb1 = 1;
                LDRH_U12(x1, xEmu, offsetof(x64emu_t, cw));
                EWBACK;
                break;
            default:
                DEFAULT;
        }
    return addr;
}
