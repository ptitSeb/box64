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

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_helper.h"
#include "dynarec_rv64_functions.h"

uintptr_t dynarec64_DF(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
{
    (void)ip; (void)rep; (void)need_epilog;

    uint8_t nextop = F8;
    uint8_t ed, wback, u8;
    int v1, v2;
    int s0;
    int64_t j64;
    int64_t fixedaddress;

    MAYUSE(s0);
    MAYUSE(v2);
    MAYUSE(v1);
    MAYUSE(j64);

    switch(nextop) {
        case 0xC0 ... 0xC7:
        
        case 0xE0:
        
        case 0xE8 ... 0xEF:
            DEFAULT;
            break;
        
        case 0xF0 ... 0xF7:
            INST_NAME("FCOMIP ST0, STx");
            SETFLAGS(X_ALL, SF_SET);
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            CLEAR_FLAGS();
            IFX(F_ZF | F_PF | F_CF) {
                if(ST_IS_F(0)) {
                    FEQS(x5, v1, v1);
                    FEQS(x4, v2, v2);
                    AND(x5, x5, x4);
                    BEQZ(x5, 24); // undefined/NaN
                    FEQS(x5, v1, v2);
                    BNEZ(x5, 24); // equal
                    FLTS(x3, v1, v2); // x3 = (v1<v2)?1:0
                    OR(xFlags, xFlags, x3); // CF is the least significant bit
                    JAL_gen(xZR, 16); // end
                    // NaN
                    ORI(xFlags, xFlags, (1<<F_ZF) | (1<<F_PF) | (1<<F_CF));
                    JAL_gen(xZR, 8); // end
                    // equal
                    ORI(xFlags, xFlags, 1<<F_ZF);
                    // end
                } else {
                    FEQD(x5, v1, v1);
                    FEQD(x4, v2, v2);
                    AND(x5, x5, x4);
                    BEQZ(x5, 24); // undefined/NaN
                    FEQD(x5, v1, v2);
                    BNEZ(x5, 24); // equal
                    FLTD(x3, v1, v2); // x3 = (v1<v2)?1:0
                    OR(xFlags, xFlags, x3); // CF is the least significant bit
                    JAL_gen(xZR, 16); // end
                    // NaN
                    ORI(xFlags, xFlags, (1<<F_ZF) | (1<<F_PF) | (1<<F_CF));
                    JAL_gen(xZR, 8); // end
                    // equal
                    ORI(xFlags, xFlags, 1<<F_ZF);
                    // end
                }
            }
            x87_do_pop(dyn, ninst, x3);
            break;
        case 0xC8 ... 0xDF:
        case 0xE1 ... 0xE7:
        case 0xF8 ... 0xFF:
            DEFAULT;
            break;

        default:
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("FILD ST0, Ew");
                    v1 = x87_do_push(dyn, ninst, x1, EXT_CACHE_ST_F);
                    addr = geted(dyn, addr, ninst, nextop, &wback, x3, x4, &fixedaddress, rex, NULL, 1, 0);
                    LH(x1, wback, fixedaddress);
                    if(ST_IS_F(0)) {
                        FCVTSL(v1, x1);
                    } else {
                        FCVTDL(v1, x1);
                    }
                    break;
                default:
                    DEFAULT;
                    break;
            }
    }
    return addr;
}