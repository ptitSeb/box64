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
            INST_NAME("FFREEP STx");
            DEFAULT;
            break;

        case 0xE0:
            INST_NAME("FNSTSW AX");
            DEFAULT;
            break;

        case 0xE8 ... 0xF7:
            if (nextop < 0xF0) {
                INST_NAME("FUCOMIP ST0, STx");
            } else {
                INST_NAME("FCOMIP ST0, STx");
            }
            SETFLAGS(X_ALL, SF_SET);
            SET_DFNONE();
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
                    J(16); // end
                    // NaN
                    ORI(xFlags, xFlags, (1<<F_ZF) | (1<<F_PF) | (1<<F_CF));
                    J(8); // end
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
                    J(16); // end
                    // NaN
                    ORI(xFlags, xFlags, (1<<F_ZF) | (1<<F_PF) | (1<<F_CF));
                    J(8); // end
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
                        FCVTSL(v1, x1, RD_RNE);
                    } else {
                        FCVTDL(v1, x1, RD_RNE);
                    }
                    break;
                case 1:
                    INST_NAME("FISTTP Ew, ST0");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, EXT_CACHE_ST_F);
                    addr = geted(dyn, addr, ninst, nextop, &wback, x3, x4, &fixedaddress, rex, NULL, 1, 0);
                    if(!box64_dynarec_fastround) {
                        FSFLAGSI(xZR); // reset all bits
                    }
                    FCVTWD(x4, v1, RD_RTZ);
                    if(!box64_dynarec_fastround) {
                        FRFLAGS(x5);   // get back FPSR to check the IOC bit
                        ANDI(x5, x5, 1<<FR_NV);
                        BNEZ_MARK(x5);
                        SLLIW(x5, x4, 16);
                        SRAIW(x5, x5, 16);
                        BEQ_MARK2(x5, x4);
                        MARK;
                        MOV32w(x4, 0x8000);
                    }
                    MARK2;
                    SH(x4, wback, fixedaddress);
                    x87_do_pop(dyn, ninst, x3);
                    break;
                case 3:
                    INST_NAME("FISTP Ew, ST0");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, EXT_CACHE_ST_F);
                    u8 = sse_setround(dyn, ninst, x1, x2);
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                    if(!box64_dynarec_fastround) {
                        FSFLAGSI(xZR); // reset all bits
                    }
                    FCVTWD(x4, v1, RD_RTZ);
                    x87_restoreround(dyn, ninst, u8);
                    if(!box64_dynarec_fastround) {
                        FRFLAGS(x5);   // get back FPSR to check the IOC bit
                        ANDI(x5, x5, 1<<FR_NV);
                        BNEZ_MARK(x5);
                        SLLIW(x5, x4, 16);
                        SRAIW(x5, x5, 16);
                        BEQ_MARK2(x5, x4);
                        MARK;
                        MOV32w(x4, 0x8000);
                    }
                    MARK2;
                    SH(x4, wback, fixedaddress);
                    x87_do_pop(dyn, ninst, x3);
                    break;
                case 7:
                    INST_NAME("FISTP i64, ST0");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, EXT_CACHE_ST_D);
                    u8 = x87_setround(dyn, ninst, x1, x2);
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                    v2 = fpu_get_scratch(dyn);
                    if(!box64_dynarec_fastround) {
                        FSFLAGSI(xZR); // reset all bits
                    }
                    FCVTLD(x4, v1, RD_DYN);
                    x87_restoreround(dyn, ninst, u8);
                    if(!box64_dynarec_fastround) {
                        FRFLAGS(x5);   // get back FPSR to check the IOC bit
                        ANDI(x5, x5, 1<<FR_NV);
                        BEQ_MARK2(x5, xZR);
                        MOV64x(x4, 0x8000000000000000LL);
                    }
                    MARK2;
                    SD(x4, wback, fixedaddress);
                    x87_do_pop(dyn, ninst, x3);
                    break;
                default:
                    DEFAULT;
                    break;
            }
    }
    return addr;
}
