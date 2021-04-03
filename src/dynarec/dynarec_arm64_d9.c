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
#include "dynarec_arm64.h"
#include "dynarec_arm64_private.h"
#include "arm64_printer.h"
#include "emu/x87emu_private.h"

#include "dynarec_arm64_helper.h"
#include "dynarec_arm64_functions.h"


uintptr_t dynarec64_D9(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
{
    uint8_t nextop = F8;
    uint8_t ed;
    uint8_t wback, wb1;
    int fixedaddress;
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
            v1 = x87_get_st(dyn, ninst, x1, x2, nextop&7);
            v2 = x87_do_push(dyn, ninst);
            FMOVD(v2, v1);
            break;

        case 0xC8:
        case 0xC9:
        case 0xCA:
        case 0xCB:
        case 0xCC:
        case 0xCD:
        case 0xCE:
        case 0xCF:
            INST_NAME("FXCH STx");
            // swap the cache value, not the double value itself :p
            i1 = x87_get_cache(dyn, ninst, x1, x2, nextop&7);
            i2 = x87_get_cache(dyn, ninst, x1, x2, 0);
            i3 = dyn->x87cache[i1];
            dyn->x87cache[i1] = dyn->x87cache[i2];
            dyn->x87cache[i2] = i3;
            break;

        case 0xD0:
            INST_NAME("FNOP");
            break;

        case 0xE0:
            INST_NAME("FCHS");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0);
            FNEGD(v1, v1);
            break;
        case 0xE1:
            INST_NAME("FABS");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0);
            FABSD(v1, v1);
            break;

        case 0xE4:
            INST_NAME("FTST");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0);
            FCMPD_0(v1);
            FCOM(x1, x2, x3);   // same flags...
            break;
        case 0xE5:
            INST_NAME("FXAM");
            x87_refresh(dyn, ninst, x1, x2, 0);
            CALL(fpu_fxam, -1);  // should be possible inline, but is it worth it?
            break;

        case 0xE8:
            INST_NAME("FLD1");
            v1 = x87_do_push(dyn, ninst);
            FTABLE64(v1, 1.0);
            break;
        case 0xE9:
            INST_NAME("FLDL2T");
            v1 = x87_do_push(dyn, ninst);
            FTABLE64(v1, L2T);
            break;
        case 0xEA:     
            INST_NAME("FLDL2E");
            v1 = x87_do_push(dyn, ninst);
            FTABLE64(v1, L2E);
            break;
        case 0xEB:
            INST_NAME("FLDPI");
            v1 = x87_do_push(dyn, ninst);
            FTABLE64(v1, PI);
            break;
        case 0xEC:
            INST_NAME("FLDLG2");
            v1 = x87_do_push(dyn, ninst);
            FTABLE64(v1, LG2);
            break;
        case 0xED:
            INST_NAME("FLDLN2");
            v1 = x87_do_push(dyn, ninst);
            FTABLE64(v1, LN2);
            break;
        case 0xEE:
            INST_NAME("FLDZ");
            v1 = x87_do_push(dyn, ninst);
            FTABLE64(v1, 0.0);
            break;

        case 0xFA:
            INST_NAME("FSQRT");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0);
            FSQRTD(v1, v1);
            break;

        case 0xFC:
            INST_NAME("FRNDINT");
            // use C helper for now, nothing staightforward is available
            x87_forget(dyn, ninst, x1, x2, 0);
            CALL(arm_frndint, -1);
            /*
            v1 = x87_get_st(dyn, ninst, x1, x2, 0);
            VCMP_F64_0(v1);
            VMRS_APSR();
            B_NEXT(cVS);    // Unordered, skip
            B_NEXT(cEQ);    // Zero, skip
            u8 = x87_setround(dyn, ninst, x1, x2, x3);
            VCVT_S32_F64(x1, v1);   // limit to 32bits....
            VCVT_F64_S32(v1, x1);
            x87_restoreround(dyn, ninst, u8);
            */
            break;
        case 0xF0:
            INST_NAME("F2XM1");
            x87_forget(dyn, ninst, x1, x2, 0);
            CALL(arm_f2xm1, -1);
            break;
        case 0xF1:
            INST_NAME("FYL2X");
            x87_forget(dyn, ninst, x1, x2, 0);
            x87_forget(dyn, ninst, x1, x2, 1);
            CALL(arm_fyl2x, -1);
            x87_do_pop(dyn, ninst);
            break;
        case 0xF2:
            INST_NAME("FTAN");
            x87_forget(dyn, ninst, x1, x2, 0);
            CALL(arm_ftan, -1);
            v1 = x87_do_push(dyn, ninst);
            FTABLE64(v1, 1.0);
            break;
        case 0xF3:
            INST_NAME("FPATAN");
            x87_forget(dyn, ninst, x1, x2, 0);
            x87_forget(dyn, ninst, x1, x2, 1);
            CALL(arm_fpatan, -1);
            x87_do_pop(dyn, ninst);
            break;
        case 0xF4:
            INST_NAME("FXTRACT");
            x87_do_push_empty(dyn, ninst, 0);
            x87_forget(dyn, ninst, x1, x2, 1);
            CALL(arm_fxtract, -1);
            break;
        case 0xF5:
            INST_NAME("FPREM1");
            x87_forget(dyn, ninst, x1, x2, 0);
            x87_forget(dyn, ninst, x1, x2, 1);
            CALL(arm_fprem1, -1);
            break;
        case 0xF6:
            INST_NAME("FDECSTP");
            fpu_purgecache(dyn, ninst, x1, x2, x3);
            LDRw_U12(x2, xEmu, offsetof(x64emu_t, top));
            SUBw_U12(x2, x2, 1);
            ANDw_mask(x2, x2, 0, 2);    //mask=7
            STRw_U12(x2, xEmu, offsetof(x64emu_t, top));
            break;
        case 0xF7:
            INST_NAME("FINCSTP");
            fpu_purgecache(dyn, ninst, x1, x2, x3);
            LDRw_U12(x2, xEmu, offsetof(x64emu_t, top));
            ADDw_U12(x2, x2, 1);
            ANDw_mask(x2, x2, 0, 2);    //mask=7
            STRw_U12(x2, xEmu, offsetof(x64emu_t, top));
            break;
        case 0xF8:
            INST_NAME("FPREM");
            x87_forget(dyn, ninst, x1, x2, 0);
            x87_forget(dyn, ninst, x1, x2, 1);
            CALL(arm_fprem, -1);
            break;
        case 0xF9:
            INST_NAME("FYL2XP1");
            x87_forget(dyn, ninst, x1, x2, 0);
            x87_forget(dyn, ninst, x1, x2, 1);
            CALL(arm_fyl2xp1, -1);
            x87_do_pop(dyn, ninst);
            break;
        case 0xFB:
            INST_NAME("FSINCOS");
            x87_do_push_empty(dyn, ninst, 0);
            x87_forget(dyn, ninst, x1, x2, 1);
            CALL(arm_fsincos, -1);
            break;
        case 0xFD:
            INST_NAME("FSCALE");
            x87_forget(dyn, ninst, x1, x2, 0);
            x87_forget(dyn, ninst, x1, x2, 1);
            CALL(arm_fscale, -1);
            break;
        case 0xFE:
            INST_NAME("FSIN");
            x87_forget(dyn, ninst, x1, x2, 0);
            CALL(arm_fsin, -1);
            break;
        case 0xFF:
            INST_NAME("FCOS");
            x87_forget(dyn, ninst, x1, x2, 0);
            CALL(arm_fcos, -1);
            break;


        case 0xD1:
        case 0xD4:
        case 0xD5:
        case 0xD6:
        case 0xD7:
        case 0xD8:
        case 0xD9:
        case 0xDA:
        case 0xDB:
        case 0xDC:
        case 0xDD:
        case 0xDE:
        case 0xDF:
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
                    v1 = x87_do_push(dyn, ninst);
                    s0 = fpu_get_scratch(dyn);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0xfff<<2, 3, rex, 0, 0);
                    VLDR32_U12(s0, ed, fixedaddress);
                    FCVT_D_S(v1, s0);
                    break;
                case 2:
                    INST_NAME("FST float[ED], ST0");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0);
                    s0 = fpu_get_scratch(dyn);
                    FCVT_S_D(s0, v1);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0xfff<<2, 3, rex, 0, 0);
                    VSTR32_U12(s0, ed, fixedaddress);
                    break;
                case 3:
                    INST_NAME("FSTP float[ED], ST0");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0);
                    s0 = fpu_get_scratch(dyn);
                    FCVT_S_D(s0, v1);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0xfff<<2, 3, rex, 0, 0);
                    VSTR32_U12(s0, ed, fixedaddress);
                    x87_do_pop(dyn, ninst);
                    break;
                case 4:
                    INST_NAME("FLDENV Ed");
                    fpu_purgecache(dyn, ninst, x1, x2, x3); // maybe only x87, not SSE?
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, rex, 0, 0);
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
                    UBFXw(x1, x1, 10, 2);    // extract round
                    STRw_U12(x1, xEmu, offsetof(x64emu_t, round));
                    break;
                case 6:
                    INST_NAME("FNSTENV Ed");
                    fpu_purgecache(dyn, ninst, x1, x2, x3); // maybe only x87, not SSE?
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, rex, 0, 0);
                    if(ed!=x1) {
                        MOVx_REG(x1, ed);
                    }
                    MOV32w(x2, 0);
                    CALL(fpu_savenv, -1);
                    break;
                case 7:
                    INST_NAME("FNSTCW Ew");
                    addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 0xfff<<1, 1, rex, 0, 0);
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

