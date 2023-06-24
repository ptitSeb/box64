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
    int64_t fixedaddress;
    int unscaled;
    int v1, v2;
    int s0;
    int i1, i2, i3;

    MAYUSE(s0);
    MAYUSE(v2);
    MAYUSE(v1);

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
            v2 = x87_do_push(dyn, ninst, x1, X87_ST(nextop&7));
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
            x87_do_pop(dyn, ninst, x3);
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
            x87_do_pop(dyn, ninst, x3);
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
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            x87_refresh(dyn, ninst, x1, x2, 0);
            CALL(fpu_fxam, -1);  // should be possible inline, but is it worth it?
            break;

        case 0xE8:
            INST_NAME("FLD1");
            v1 = x87_do_push(dyn, ninst, x1, NEON_CACHE_ST_F);
            if(ST_IS_F(0)) {
                FMOVS_8(v1, 0b01110000);
            } else {
                FMOVD_8(v1, 0b01110000);
            }
            break;
        case 0xE9:
            INST_NAME("FLDL2T");
            v1 = x87_do_push(dyn, ninst, x1, NEON_CACHE_ST_D);
            FTABLE64(v1, L2T);
            break;
        case 0xEA:     
            INST_NAME("FLDL2E");
            v1 = x87_do_push(dyn, ninst, x1, NEON_CACHE_ST_D);
            FTABLE64(v1, L2E);
            break;
        case 0xEB:
            INST_NAME("FLDPI");
            v1 = x87_do_push(dyn, ninst, x1, NEON_CACHE_ST_D);
            FTABLE64(v1, PI);
            break;
        case 0xEC:
            INST_NAME("FLDLG2");
            v1 = x87_do_push(dyn, ninst, x1, NEON_CACHE_ST_D);
            FTABLE64(v1, LG2);
            break;
        case 0xED:
            INST_NAME("FLDLN2");
            v1 = x87_do_push(dyn, ninst, x1, NEON_CACHE_ST_D);
            FTABLE64(v1, LN2);
            break;
        case 0xEE:
            INST_NAME("FLDZ");
            v1 = x87_do_push(dyn, ninst, x1, NEON_CACHE_ST_F);
            VEOR(v1, v1, v1);
            break;

        case 0xF0:
            INST_NAME("F2XM1");
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            x87_forget(dyn, ninst, x1, x2, 0);
            CALL(native_f2xm1, -1);
            break;
        case 0xF1:
            INST_NAME("FYL2X");
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            x87_forget(dyn, ninst, x1, x2, 0);
            x87_forget(dyn, ninst, x1, x2, 1);
            CALL(native_fyl2x, -1);
            x87_do_pop(dyn, ninst, x3);
            break;
        case 0xF2:
            INST_NAME("FPTAN");
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            x87_forget(dyn, ninst, x1, x2, 0);
            CALL(native_ftan, -1);
            v1 = x87_do_push(dyn, ninst, x1, NEON_CACHE_ST_F);
            if(ST_IS_F(0)) {
                FMOVS_8(v1, 0b01110000);
            } else {
                FMOVD_8(v1, 0b01110000);
            }
            break;
        case 0xF3:
            INST_NAME("FPATAN");
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            x87_forget(dyn, ninst, x1, x2, 0);
            x87_forget(dyn, ninst, x1, x2, 1);
            CALL(native_fpatan, -1);
            x87_do_pop(dyn, ninst, x3);
            break;
        case 0xF4:
            INST_NAME("FXTRACT");
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            x87_do_push_empty(dyn, ninst, 0);
            x87_forget(dyn, ninst, x1, x2, 1);
            CALL(native_fxtract, -1);
            break;
        case 0xF5:
            INST_NAME("FPREM1");
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            x87_forget(dyn, ninst, x1, x2, 0);
            x87_forget(dyn, ninst, x1, x2, 1);
            CALL(native_fprem1, -1);
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
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            x87_forget(dyn, ninst, x1, x2, 0);
            x87_forget(dyn, ninst, x1, x2, 1);
            CALL(native_fprem, -1);
            break;
        case 0xF9:
            INST_NAME("FYL2XP1");
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            x87_forget(dyn, ninst, x1, x2, 0);
            x87_forget(dyn, ninst, x1, x2, 1);
            CALL(native_fyl2xp1, -1);
            x87_do_pop(dyn, ninst, x3);
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
            x87_do_push_empty(dyn, ninst, 0);
            x87_forget(dyn, ninst, x1, x2, 1);
            CALL(native_fsincos, -1);
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
            x87_forget(dyn, ninst, x1, x2, 0);
            x87_forget(dyn, ninst, x1, x2, 1);
            CALL(native_fscale, -1);
            break;
        case 0xFE:
            INST_NAME("FSIN");
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            x87_forget(dyn, ninst, x1, x2, 0);
            CALL(native_fsin, -1);
            break;
        case 0xFF:
            INST_NAME("FCOS");
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            x87_forget(dyn, ninst, x1, x2, 0);
            CALL(native_fcos, -1);
            break;


        case 0xD1:
        case 0xD4:
        case 0xD5:
        case 0xD6:
        case 0xD7:
        case 0xE2:
        case 0xE3:
        case 0xE6:
        case 0xE7:
        case 0xEF:
            DEFAULT;
            break;
             
        default:
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("FLD ST0, float[ED]");
                    v1 = x87_do_push(dyn, ninst, x1, box64_dynarec_x87double?NEON_CACHE_ST_D:NEON_CACHE_ST_F);
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
                    x87_do_pop(dyn, ninst, x3);
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
    }
    return addr;
}
