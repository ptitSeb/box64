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

#include "ppc64le_printer.h"
#include "dynarec_ppc64le_private.h"
#include "../dynarec_helper.h"
#include "dynarec_ppc64le_functions.h"


uintptr_t dynarec64_DD(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t nextop = F8;
    uint8_t ed, wback, wb1, u8;
    int64_t fixedaddress;
    int unscaled;
    int v1, v2;
    int s0;
    int64_t j64;

    MAYUSE(u8);
    MAYUSE(wb1);
    MAYUSE(s0);
    MAYUSE(v2);
    MAYUSE(v1);
    MAYUSE(j64);

    if (MODREG)
        switch (nextop) {
            case 0xC0 ... 0xC7:
                INST_NAME("FFREE STx");
                if ((nextop & 7) == 0 && PK(0) == 0xD9 && PK(1) == 0xF7) {
                    MESSAGE(LOG_DUMP, "Hack for FFREE ST0 / FINCSTP\n");
                    x87_do_pop(dyn, ninst, x1);
                    addr += 2;
                    SKIPTEST(x1);
                } else
                    x87_free(dyn, ninst, x1, x2, x3, nextop & 7);
                break;
            case 0xD0 ... 0xD7:
                INST_NAME("FST ST0, STx");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop & 7));
                v2 = x87_get_st(dyn, ninst, x1, x2, nextop & 7, X87_COMBINE(0, nextop & 7));
                if (ST_IS_F(0)) {
                    FMR(v2, v1);
                } else {
                    FMR(v2, v1);
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
                    FCOMS(v1, v2, x1, x2, x3);
                } else {
                    FCOMD(v1, v2, x1, x2, x3);
                }
                break;
            case 0xE8 ... 0xEF:
                INST_NAME("FUCOMP ST0, STx");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop & 7));
                v2 = x87_get_st(dyn, ninst, x1, x2, nextop & 7, X87_COMBINE(0, nextop & 7));
                if (ST_IS_F(0)) {
                    FCOMS(v1, v2, x1, x2, x3);
                } else {
                    FCOMD(v1, v2, x1, x2, x3);
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
                X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, VMX_CACHE_ST_D);
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, DS_DISP, 0);
                LFD(v1, fixedaddress, wback);
                break;
            case 1:
                INST_NAME("FISTTP i64, ST0");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, VMX_CACHE_ST_I64);
                v2 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x4, &fixedaddress, rex, NULL, DS_DISP, 0);
                if (ST_IS_I64(0)) {
                    STFD(v1, fixedaddress, wback);
                } else {
                    // Clear FPSCR VXCVI bit before conversion
                    MTFSB0(23);
                    // Truncate double to int64 (round toward zero)
                    FCTIDZ(v2, v1);
                    // Move int64 result to GPR
                    MFVSRD(x5, v2);
                    // Check FPSCR VXCVI: PPC gives wrong result for Inf/NaN, x86 wants 0x8000000000000000
                    MFFS(v2);           // read FPSCR into FPR (overwrites v2, result already in x5)
                    MFVSRD(x4, v2);     // FPSCR to GPR (VXCVI = bit 23 from LSB)
                    RLWINM(x4, x4, 24, 31, 31);  // extract VXCVI (ISA bit 23) → bit 0
                    BEQZ_MARK(x4);     // if no overflow, skip
                    MOV64x(x5, 0x8000000000000000LL);  // x86 integer indefinite (int64)
                    MARK;
                    STD(x5, fixedaddress, wback);
                }
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            case 2:
                INST_NAME("FST double");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, VMX_CACHE_ST_D);
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, DS_DISP, 0);
                STFD(v1, fixedaddress, wback);
                break;
            case 3:
                INST_NAME("FSTP double");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, VMX_CACHE_ST_D);
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, DS_DISP, 0);
                STFD(v1, fixedaddress, wback);
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            case 4:
                INST_NAME("FRSTOR m108byte");
                MESSAGE(LOG_DUMP, "Need Optimization (FRSTOR)\n");
                BARRIER(BARRIER_FLOAT);
                addr = geted(dyn, addr, ninst, nextop, &ed, x4, x6, &fixedaddress, rex, NULL, NO_DISP, 0);
                CALL(const_native_frstor, -1, ed, 0);
                break;
            case 6:
                INST_NAME("FNSAVE m108byte");
                MESSAGE(LOG_DUMP, "Need Optimization\n");
                BARRIER(BARRIER_FLOAT);
                addr = geted(dyn, addr, ninst, nextop, &ed, x4, x6, &fixedaddress, rex, NULL, NO_DISP, 0);
                CALL(const_native_fsave, -1, ed, 0);
                NATIVE_RESTORE_X87PC();
                break;
            case 7:
                INST_NAME("FNSTSW m2byte");
                addr = geted(dyn, addr, ninst, nextop, &ed, x4, x6, &fixedaddress, rex, NULL, NO_DISP, 0);
                LWZ(x2, offsetof(x64emu_t, top), xEmu);
                LHZ(x3, offsetof(x64emu_t, sw), xEmu);
                if (dyn->v.x87stack) {
                    // update top
                    ADDI(x2, x2, -dyn->v.x87stack);
                    ANDId(x2, x2, 7);
                }
                MOV32w(x5, ~0x3800);
                AND(x3, x3, x5);     // mask out TOP
                SLDI(x2, x2, 11);    // shift TOP to bit 11
                OR(x3, x3, x2);      // inject TOP
                STH(x3, offsetof(x64emu_t, sw), xEmu);
                STH(x3, fixedaddress, ed);  // store whole sw flags
                break;
            default:
                DEFAULT;
        }
    return addr;
}
