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


uintptr_t dynarec64_DD(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
{
    (void)ip; (void)rep; (void)need_epilog;

    uint8_t nextop = F8;
    uint8_t ed;
    int64_t fixedaddress;
    int unscaled;
    int v1, v2;
    int s0;
    int64_t j64;

    MAYUSE(s0);
    MAYUSE(v2);
    MAYUSE(v1);
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
            INST_NAME("FFREE STx");
            #if 1
            if((nextop&7)==0 && PK(0)==0xD9 && PK(1)==0xF7) {
                MESSAGE(LOG_DUMP, "Hack for FFREE ST0 / FINCSTP\n");
                x87_do_pop(dyn, ninst, x1);
                addr+=2;
                SKIPTEST(x1);
            } else
                x87_free(dyn, ninst, x1, x2, x3, nextop&7);
            #else
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            x87_purgecache(dyn, ninst, 0, x1, x2, x3);
            MOV32w(x1, nextop&7);
            CALL(fpu_do_free, -1);
            #endif
            break;
        case 0xD0:
        case 0xD1:
        case 0xD2:
        case 0xD3:
        case 0xD4:
        case 0xD5:
        case 0xD6:
        case 0xD7:
            INST_NAME("FST ST0, STx");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            if(ST_IS_F(0)) {
                FMOVS(v2, v1);
            } else {
                FMOVD(v2, v1);
            }
            break;
        case 0xD8:
            INST_NAME("FSTP ST0, ST0");
            X87_POP_OR_FAIL(dyn, ninst, x3);
            break;
        case 0xD9:
        case 0xDA:
        case 0xDB:
        case 0xDC:
        case 0xDD:
        case 0xDE:
        case 0xDF:
            INST_NAME("FSTP ST0, STx");
            // copy the cache value for st0 to stx
            x87_get_st_empty(dyn, ninst, x1, x2, nextop&7, X87_ST(nextop&7));
            x87_get_st(dyn, ninst, x1, x2, 0, X87_ST0);
            x87_swapreg(dyn, ninst, x1, x2, 0, nextop&7);
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
            INST_NAME("FUCOM ST0, STx");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            if(ST_IS_F(0)) {
                FCMPS(v1, v2);
            } else {
                FCMPD(v1, v2);
            }
            FCOM(x1, x2, x3);
            break;
        case 0xE8:
        case 0xE9:
        case 0xEA:
        case 0xEB:
        case 0xEC:
        case 0xED:
        case 0xEE:
        case 0xEF:
            INST_NAME("FUCOMP ST0, STx");
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

        default:
            DEFAULT;
            break;
    } else
        switch((nextop>>3)&7) {
            case 0:
                INST_NAME("FLD double");
                X87_PUSH_OR_FAIL(v1, dyn, ninst, x3, NEON_CACHE_ST_D);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<3, 7, rex, NULL, 0, 0);
                VLD64(v1, ed, fixedaddress);
                break;
            case 1:
                INST_NAME("FISTTP i64, ST0");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_I64);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<3, 7, rex, NULL, 0, 0);
                if(ST_IS_I64(0)) {
                    VST64(v1, ed, fixedaddress);
                } else {
                    s0 = fpu_get_scratch(dyn);
                    if(arm64_frintts) {
                        FRINT64ZD(s0, v1);
                        FCVTZSxD(x2, s0);
                        STx(x2, ed, fixedaddress);
                    } else {
                        MRS_fpsr(x5);
                        BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                        MSR_fpsr(x5);
                        FRINTRRD(s0, v1, 3);
                        FCVTZSxD(x2, s0);
                        MRS_fpsr(x5);   // get back FPSR to check the IOC bit
                        TBZ_MARK3(x5, FPSR_IOC);
                        ORRx_mask(x2, xZR, 1, 1, 0);    //0x8000000000000000
                        MARK3;
                        STx(x2, ed, fixedaddress);
                    }
                }
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            case 2:
                INST_NAME("FST double");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<3, 7, rex, NULL, 0, 0);
                VST64(v1, ed, fixedaddress);
                break;
            case 3:
                INST_NAME("FSTP double");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<3, 7, rex, NULL, 0, 0);
                VST64(v1, ed, fixedaddress);
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            case 4:
                INST_NAME("FRSTOR m108byte");
                MESSAGE(LOG_DUMP, "Need Optimization\n");
                fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                if(ed!=x1) {MOVx_REG(x1, ed);}
                CALL(native_frstor, -1);
                break;
            case 6:
                INST_NAME("FNSAVE m108byte");
                MESSAGE(LOG_DUMP, "Need Optimization\n");
                fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                if(ed!=x1) {MOVx_REG(x1, ed);}
                CALL(native_fsave, -1);
                CALL(reset_fpu, -1);
                break;
            case 7:
                INST_NAME("FNSTSW m2byte");
                //fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
                addr = geted(dyn, addr, ninst, nextop, &ed, x4, &fixedaddress, &unscaled, 0xfff<<1, 1, rex, NULL, 0, 0);
                LDRw_U12(x2, xEmu, offsetof(x64emu_t, top));
                LDRH_U12(x3, xEmu, offsetof(x64emu_t, sw));
                if(dyn->n.x87stack) {
                    // update top
                    if(dyn->n.x87stack>0) {
                        SUBw_U12(x2, x2, dyn->n.x87stack);
                    } else {
                        ADDw_U12(x2, x2, -dyn->n.x87stack);
                    }
                    ANDw_mask(x2, x2, 0, 2);
                }
                BFIw(x3, x2, 11, 3); // inject TOP at bit 11 (3 bits)
                STRH_U12(x3, xEmu, offsetof(x64emu_t, sw));
                STH(x3, ed, fixedaddress);   // store whole sw flags
                break;
            default:
                DEFAULT;
        }
    return addr;
}
