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
    int v1, v2;
    int s0;

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
            INST_NAME("FFREE STx");
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            x87_purgecache(dyn, ninst, 0, x1, x2, x3);
            MOV32w(x1, nextop&7);
            CALL(fpu_do_free, -1);
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
            x87_do_pop(dyn, ninst, x3);
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
            x87_swapreg(dyn, ninst, x1, x2, 0, nextop&7);
            x87_do_pop(dyn, ninst, x3);
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
            x87_do_pop(dyn, ninst, x3);
            break;

        case 0xC8:
        case 0xC9:
        case 0xCA:
        case 0xCB:
        case 0xCC:
        case 0xCD:
        case 0xCE:
        case 0xCF:
        case 0xF0:
        case 0xF1:
        case 0xF2:
        case 0xF3:
        case 0xF4:
        case 0xF5:
        case 0xF6:
        case 0xF7:
        case 0xF8:
        case 0xF9:
        case 0xFA:
        case 0xFB:
        case 0xFC:
        case 0xFD:
        case 0xFE:
        case 0xFF:
            DEFAULT;
            break;

        default:
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("FLD double");
                    v1 = x87_do_push(dyn, ninst, x3, NEON_CACHE_ST_D);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0xfff<<3, 7, rex, NULL, 0, 0);
                    VLDR64_U12(v1, ed, fixedaddress);
                    break;
                case 1:
                    INST_NAME("FISTTP i64, ST0");
                    v1 = x87_do_push(dyn, ninst, x3, NEON_CACHE_ST_D);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0xfff<<3, 7, rex, NULL, 0, 0);
                    s0 = fpu_get_scratch(dyn);
                    #if 0
                    // those are ARM 8.5 opcode!
                    FRINT64ZD(s0, v1);
                    #else
                    FRINTRRD(s0, v1, 3);    // not ideal, might averflow and not set 0x8000000000000000 correctly
                    #endif
                    FCVTZSxD(x2, s0);
                    STRx_U12(x2, ed, fixedaddress);
                    x87_do_pop(dyn, ninst, x3);
                    break;
                case 2:
                    INST_NAME("FST double");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0xfff<<3, 7, rex, NULL, 0, 0);
                    VSTR64_U12(v1, ed, fixedaddress);
                    break;
                case 3:
                    INST_NAME("FSTP double");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0xfff<<3, 7, rex, NULL, 0, 0);
                    VSTR64_U12(v1, ed, fixedaddress);
                    x87_do_pop(dyn, ninst, x3);
                    break;
                case 4: 
                    INST_NAME("FRSTOR m108byte");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, rex, NULL, 0, 0);
                    if(ed!=x1) {MOVx_REG(x1, ed);}
                    CALL(arm_frstor, -1);
                    break;
                case 6: 
                    INST_NAME("FSAVE m108byte");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, rex, NULL, 0, 0);
                    if(ed!=x1) {MOVx_REG(x1, ed);}
                    CALL(arm_fsave, -1);
                    break;
                case 7:
                    INST_NAME("FNSTSW m2byte");
                    fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x4, &fixedaddress, 0xfff<<1, 1, rex, NULL, 0, 0);
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
                    STRH_U12(x3, ed, fixedaddress);   // store whole sw flags
                    break;
                default:
                    DEFAULT;
            }
    }
    return addr;
}
