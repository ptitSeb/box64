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

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "../dynarec_helper.h"
#include "dynarec_rv64_functions.h"


uintptr_t dynarec64_DD(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t nextop = F8;
    uint8_t ed, wback;
    int64_t fixedaddress;
    int unscaled;
    int v1, v2;
    int s0;
    int64_t j64;

    MAYUSE(s0);
    MAYUSE(v2);
    MAYUSE(v1);
    MAYUSE(j64);

    if (MODREG)
        switch (nextop) {
            case 0xC0 ... 0xC7:
#if 1
                if ((nextop & 7) == 0 && PK(0) == 0xD9 && PK(1) == 0xF7) {
                    MESSAGE(LOG_DUMP, "Hack for FFREE ST0 / FINCSTP\n");
                    x87_do_pop(dyn, ninst, x1);
                    addr += 2;
                    SKIPTEST(x1);
                } else
                    x87_free(dyn, ninst, x1, x2, x3, nextop & 7);
#else
                MESSAGE(LOG_DUMP, "Need Optimization\n");
                x87_purgecache(dyn, ninst, 0, x1, x2, x3);
                MOV32w(x1, nextop & 7);
                CALL(fpu_do_free, -1, x1, 0);
#endif
                break;
            case 0xD0 ... 0xD7:
                INST_NAME("FST ST0, STx");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop & 7));
                v2 = x87_get_st(dyn, ninst, x1, x2, nextop & 7, X87_COMBINE(0, nextop & 7));
                if (ST_IS_F(0)) {
                    FMVS(v2, v1);
                } else {
                    FMVD(v2, v1);
                }
                break;
            case 0xD8:
                INST_NAME("FSTP ST0, ST0");
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            case 0xD9 ... 0xDF:
                INST_NAME("FSTP ST0, STx");
                // copy the cache value for st0 to stx
                x87_get_st_empty(dyn, ninst, x1, x2, nextop & 7, X87_ST(nextop & 7));
                x87_get_st(dyn, ninst, x1, x2, 0, X87_ST0);
                x87_swapreg(dyn, ninst, x1, x2, 0, nextop & 7);
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            case 0xE0 ... 0xE7:
                INST_NAME("FUCOM ST0, STx");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop & 7));
                v2 = x87_get_st(dyn, ninst, x1, x2, nextop & 7, X87_COMBINE(0, nextop & 7));
                if (ST_IS_F(0)) {
                    FCOMS(v1, v2, x1, x2, x3, x4, x5);
                } else {
                    FCOMD(v1, v2, x1, x2, x3, x4, x5);
                }
                break;
            case 0xE8 ... 0xEF:
                INST_NAME("FUCOMP ST0, STx");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop & 7));
                v2 = x87_get_st(dyn, ninst, x1, x2, nextop & 7, X87_COMBINE(0, nextop & 7));
                if (ST_IS_F(0)) {
                    FCOMS(v1, v2, x1, x2, x3, x4, x5);
                } else {
                    FCOMD(v1, v2, x1, x2, x3, x4, x5);
                }
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            default:
                DEFAULT;
                break;
        }
    else
        switch ((nextop >> 3) & 7) {
            case 0:
                INST_NAME("FLD double");
                X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, EXT_CACHE_ST_D);
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                FLD(v1, wback, fixedaddress);
                break;
            case 1:
                INST_NAME("FISTTP i64, ST0");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, EXT_CACHE_ST_I64);
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x4, &fixedaddress, rex, NULL, 1, 0);
                if (ST_IS_I64(0)) {
                    FSD(v1, wback, fixedaddress);
                } else {
                    if (!BOX64ENV(dynarec_fastround)) {
                        FSFLAGSI(0); // reset all bits
                    }
                    FCVTLD(x4, v1, RD_RTZ);
                    if (!BOX64ENV(dynarec_fastround)) {
                        FRFLAGS(x5); // get back FPSR to check the IOC bit
                        ANDI(x5, x5, 1 << FR_NV);
                        BEQZ_MARK(x5);
                        MOV64x(x4, 0x8000000000000000);
                        MARK;
                    }
                    SD(x4, wback, fixedaddress);
                }
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            case 2:
                INST_NAME("FST double");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, EXT_CACHE_ST_D);
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                FSD(v1, wback, fixedaddress);
                break;
            case 3:
                INST_NAME("FSTP double");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, EXT_CACHE_ST_D);
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                FSD(v1, wback, fixedaddress);
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            case 4:
                INST_NAME("FRSTOR m108byte");
                MESSAGE(LOG_DUMP, "Need Optimization (FRSTOR)\n");
                fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
                addr = geted(dyn, addr, ninst, nextop, &ed, x4, x6, &fixedaddress, rex, NULL, 0, 0);
                CALL(const_native_frstor, -1, ed, 0);
                break;
            case 6:
                INST_NAME("FNSAVE m108byte");
                MESSAGE(LOG_DUMP, "Need Optimization\n");
                fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
                addr = geted(dyn, addr, ninst, nextop, &ed, x4, x6, &fixedaddress, rex, NULL, 0, 0);
                CALL(const_native_fsave, -1, ed, 0);
                NATIVE_RESTORE_X87PC();
                break;
            case 7:
                INST_NAME("FNSTSW m2byte");
                // fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
                addr = geted(dyn, addr, ninst, nextop, &ed, x4, x6, &fixedaddress, rex, NULL, 0, 0);
                LWU(x2, xEmu, offsetof(x64emu_t, top));
                LHU(x3, xEmu, offsetof(x64emu_t, sw));
                if (dyn->e.x87stack) {
                    // update top
                    ADDI(x2, x2, -dyn->e.x87stack);
                    ANDI(x2, x2, 7);
                }
                MOV32w(x5, ~0x3800);
                AND(x3, x3, x5);          // mask out TOP
                SLLI(x2, x2, 11);         // shift TOP to bit 11
                OR(x3, x3, x2);           // inject TOP
                SH(x3, xEmu, offsetof(x64emu_t, sw));
                SH(x3, ed, fixedaddress); // store whole sw flags
                break;
            default:
                DEFAULT;
        }
    return addr;
}
