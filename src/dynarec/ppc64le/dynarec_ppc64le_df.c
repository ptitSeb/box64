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

uintptr_t dynarec64_DF(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t nextop = F8;
    uint8_t ed, wback, wb1, u8;
    int v1, v2;
    int s0;
    int64_t j64;
    int64_t fixedaddress;
    int unscaled;

    MAYUSE(wb1);
    MAYUSE(s0);
    MAYUSE(v2);
    MAYUSE(v1);
    MAYUSE(j64);

    if (MODREG)
        switch (nextop) {
            case 0xC0 ... 0xC7:
                INST_NAME("FFREEP STx");
                // not handling Tag...
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            case 0xD0:
            case 0xD1:
            case 0xD2:
            case 0xD3:
            case 0xD4:
            case 0xD5:
            case 0xD6:
            case 0xD7:
                INST_NAME("FSTP STx, ST0");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop & 7));
                v2 = x87_get_st(dyn, ninst, x1, x2, nextop & 7, X87_COMBINE(0, nextop & 7));
                if (ST_IS_F(0)) {
                    FMR(v2, v1);
                } else {
                    FMR(v2, v1);
                }
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            case 0xE0:
                INST_NAME("FNSTSW AX");
                LWZ(x2, offsetof(x64emu_t, top), xEmu);
                if (dyn->v.x87stack) {
                    ADDI(x2, x2, -dyn->v.x87stack);
                    ANDId(x2, x2, 0x7);
                }
                LHZ(x1, offsetof(x64emu_t, sw), xEmu);
                ANDId(x1, x1, 0xC7FF); // mask off TOP bits (clear bits 11-13)
                SLDI(x2, x2, 11);
                OR(x1, x1, x2); // inject top
                STH(x1, offsetof(x64emu_t, sw), xEmu);
                SRDI(xRAX, xRAX, 16);
                SLDI(xRAX, xRAX, 16);
                OR(xRAX, xRAX, x1);
                break;
            case 0xE8 ... 0xF7:
                if (nextop < 0xF0) {
                    INST_NAME("FUCOMIP ST0, STx");
                } else {
                    INST_NAME("FCOMIP ST0, STx");
                }
                SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
                SET_DFNONE();
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop & 7));
                v2 = x87_get_st(dyn, ninst, x1, x2, nextop & 7, X87_COMBINE(0, nextop & 7));
                if (ST_IS_F(0)) {
                    FCOMIS(v1, v2, x1, x2);
                } else {
                    FCOMID(v1, v2, x1, x2);
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
                INST_NAME("FILD ST0, Ew");
                X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, VMX_CACHE_ST_F);
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x4, &fixedaddress, rex, NULL, DS_DISP, 0);
                LHZ(x1, fixedaddress, wback);
                EXTSH(x1, x1);
                MTVSRD(VSXREG_X87(v1), x1);
                if (ST_IS_F(0)) {
                    FCFIDS(v1, v1);
                } else {
                    FCFID(v1, v1);
                }
                break;
            case 1:
                INST_NAME("FISTTP Ew, ST0");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, VMX_CACHE_ST_F);
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x4, &fixedaddress, rex, NULL, DS_DISP, 0);
                s0 = fpu_get_scratch(dyn);
                // Clear FPSCR VXCVI bit before conversion
                MTFSB0(23);
                // Truncate to int32 (round toward zero), then store low 16 bits
                FCTIWZ(s0, v1);
                // Use raw s0 (not VSXREG) since FCTIWZ writes to FPR space
                MFVSRWZ(x4, s0);
                // Check FPSCR VXCVI: if set, Inf/NaN overflow → use 0x8000
                MFFS(s0);           // read FPSCR into FPR (overwrites s0, result already in x4)
                MFVSRD(x5, s0);     // FPSCR to GPR (VXCVI = bit 23 from LSB)
                RLWINM(x5, x5, 24, 31, 31);  // extract VXCVI (ISA bit 23) → bit 0
                BNEZ_MARK(x5);      // if VXCVI set, jump to indefinite
                // MFVSRWZ zero-extends to 64 bits; sign-extend int32 first for correct comparison
                EXTSW(x4, x4);
                // Clamp to int16 range: if value overflows, use 0x8000 (indefinite)
                EXTSH(x5, x4);
                BEQ_MARK2(x5, x4);  // if fits in int16, skip to store
                MARK;
                // overflow or Inf/NaN
                MOV32w(x4, 0x8000);
                MARK2;
                STH(x4, fixedaddress, wback);
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            case 2:
                INST_NAME("FIST Ew, ST0");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, VMX_CACHE_ST_F);
                u8 = x87_setround(dyn, ninst, x1, x7);
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x3, &fixedaddress, rex, NULL, DS_DISP, 0);
                s0 = fpu_get_scratch(dyn);
                // Clear FPSCR VXCVI bit before conversion
                MTFSB0(23);
                // Convert to int32 using current rounding mode
                FCTIW(s0, v1);
                // Use raw s0 (not VSXREG) since FCTIW writes to FPR space
                MFVSRWZ(x4, s0);
                // Check FPSCR VXCVI BEFORE restoreround (which overwrites entire FPSCR)
                MFFS(s0);           // read FPSCR into FPR (overwrites s0, result already in x4)
                MFVSRD(x5, s0);     // FPSCR to GPR (VXCVI = ISA bit 23)
                RLWINM(x5, x5, 24, 31, 31);  // extract VXCVI (ISA bit 23) → bit 0
                x87_restoreround(dyn, ninst, u8);
                BNEZ_MARK(x5);      // if VXCVI set, jump to indefinite
                // MFVSRWZ zero-extends to 64 bits; sign-extend int32 first for correct comparison
                EXTSW(x4, x4);
                // Clamp to int16 range
                EXTSH(x5, x4);
                BEQ_MARK2(x5, x4);
                MARK;
                MOV32w(x4, 0x8000);
                MARK2;
                STH(x4, fixedaddress, wback);
                break;
            case 3:
                INST_NAME("FISTP Ew, ST0");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, VMX_CACHE_ST_F);
                u8 = x87_setround(dyn, ninst, x1, x7);
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x3, &fixedaddress, rex, NULL, DS_DISP, 0);
                s0 = fpu_get_scratch(dyn);
                // Clear FPSCR VXCVI bit before conversion
                MTFSB0(23);
                // Convert to int32 using current rounding mode
                FCTIW(s0, v1);
                // Use raw s0 (not VSXREG) since FCTIW writes to FPR space
                MFVSRWZ(x4, s0);
                // Check FPSCR VXCVI BEFORE restoreround (which overwrites entire FPSCR)
                MFFS(s0);           // read FPSCR into FPR (overwrites s0, result already in x4)
                MFVSRD(x5, s0);     // FPSCR to GPR (VXCVI = ISA bit 23)
                RLWINM(x5, x5, 24, 31, 31);  // extract VXCVI (ISA bit 23) → bit 0
                x87_restoreround(dyn, ninst, u8);
                BNEZ_MARK(x5);      // if VXCVI set, jump to indefinite
                // MFVSRWZ zero-extends to 64 bits; sign-extend int32 first for correct comparison
                EXTSW(x4, x4);
                // Clamp to int16 range
                EXTSH(x5, x4);
                BEQ_MARK2(x5, x4);
                MARK;
                MOV32w(x4, 0x8000);
                MARK2;
                STH(x4, fixedaddress, wback);
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            case 4:
                INST_NAME("FBLD ST0, tbytes");
                X87_PUSH_EMPTY_OR_FAIL(dyn, ninst, x1);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, NO_DISP, 0);
                s0 = x87_stackcount(dyn, ninst, x3);
                CALL(const_fpu_fbld, -1, ed, 0);
                x87_unstackcount(dyn, ninst, x3, s0);
                break;
            case 5:
                INST_NAME("FILD ST0, i64");
                X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, VMX_CACHE_ST_I64);
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x3, &fixedaddress, rex, NULL, DS_DISP, 0);

                if (ST_IS_I64(0)) {
                    LFD(v1, fixedaddress, wback);
                } else {
                    LD(x1, fixedaddress, wback);
                    if (rex.is32bits) {
                        // need to also feed the STll stuff...
                        ADDI(x4, xEmu, offsetof(x64emu_t, fpu_ll));
                        LWZ(x5, offsetof(x64emu_t, top), xEmu);
                        int a = 0 - dyn->v.x87stack;
                        if (a) {
                            ADDI(x5, x5, a);
                            ANDId(x5, x5, 0x7);
                        }
                        SLDI(x5, x5, 4); // fpu_ll is 2 i64
                        ADD(x5, x5, x4);
                        STD(x1, 8, x5); // ll
                    }
                    MTVSRD(VSXREG_X87(v1), x1);
                    FCFID(v1, v1);
                    if (rex.is32bits) {
                        STFD(v1, 0, x5); // ref
                    }
                }
                break;
            case 6:
                INST_NAME("FBSTP tbytes, ST0");
                x87_forget(dyn, ninst, x1, x2, 0);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, NO_DISP, 0);
                s0 = x87_stackcount(dyn, ninst, x3);
                CALL(const_fpu_fbst, -1, ed, 0);
                x87_unstackcount(dyn, ninst, x3, s0);
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            case 7:
                INST_NAME("FISTP i64, ST0");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, VMX_CACHE_ST_I64);
                v2 = fpu_get_scratch(dyn);
                if (!ST_IS_I64(0)) {
                    u8 = x87_setround(dyn, ninst, x1, x7);
                }
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x3, &fixedaddress, rex, NULL, DS_DISP, 0);

                if (ST_IS_I64(0)) {
                    STFD(v1, fixedaddress, wback);
                } else {
                    if (rex.is32bits) {
                        // need to check STll first...
                        ADDI(x4, xEmu, offsetof(x64emu_t, fpu_ll));
                        LWZ(x5, offsetof(x64emu_t, top), xEmu);
                        int a = 0 - dyn->v.x87stack;
                        if (a) {
                            ADDI(x5, x5, a);
                            ANDId(x5, x5, 0x7);
                        }
                        SLDI(x5, x5, 4); // fpu_ll is 2 i64
                        ADD(x5, x5, x4);
                        MFVSRD(x3, VSXREG_X87(v1));
                        LD(x6, 0, x5); // ref
                        BNE_MARK(x6, x3);
                        LD(x6, 8, x5); // ll
                        STD(x6, fixedaddress, wback);
                        B_MARK3_nocond;
                        MARK;
                    }

                    // Clear FPSCR VXCVI bit before conversion
                    MTFSB0(23);
                    // Convert double to int64 using current rounding mode
                    FCTID(v2, v1);
                    // Move int64 result to GPR
                    MFVSRD(x5, v2);
                    // Check FPSCR VXCVI: PPC gives wrong result for Inf/NaN, x86 wants 0x8000000000000000
                    MFFS(v2);           // read FPSCR into FPR (overwrites v2, result already in x5)
                    MFVSRD(x4, v2);     // FPSCR to GPR (VXCVI = bit 23 from LSB)
                    RLWINM(x4, x4, 24, 31, 31);  // extract VXCVI (ISA bit 23) → bit 0
                    BEQZ_MARK2(x4);     // if no overflow, skip
                    MOV64x(x5, 0x8000000000000000LL);  // x86 integer indefinite (int64)
                    MARK2;
                    STD(x5, fixedaddress, wback);
                    MARK3;
                    x87_restoreround(dyn, ninst, u8);
                }
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            default:
                DEFAULT;
                break;
        }
    return addr;
}
