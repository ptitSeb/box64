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


uintptr_t dynarec64_DB(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
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
    MAYUSE(v1);
    MAYUSE(v2);
    MAYUSE(s0);
    MAYUSE(j64);

    if (MODREG)
        switch (nextop) {
            case 0xC0 ... 0xC7:
                INST_NAME("FCMOVNB ST0, STx");
                READFLAGS(X_CF);
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop & 7));
                v2 = x87_get_st(dyn, ninst, x1, x2, nextop & 7, X87_COMBINE(0, nextop & 7));
                RESTORE_EFLAGS(x5);
                ANDId(x1, xFlags, 1 << F_CF);
                CBNZ_NEXT(x1);
                FMR(v1, v2); // F_CF==0, copy STx to ST0
                break;
            case 0xC8 ... 0xCF:
                INST_NAME("FCMOVNE ST0, STx");
                READFLAGS(X_ZF);
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop & 7));
                v2 = x87_get_st(dyn, ninst, x1, x2, nextop & 7, X87_COMBINE(0, nextop & 7));
                RESTORE_EFLAGS(x5);
                ANDId(x1, xFlags, 1 << F_ZF);
                CBNZ_NEXT(x1);
                FMR(v1, v2); // F_ZF==0, copy STx to ST0
                break;
            case 0xD0 ... 0xD7:
                INST_NAME("FCMOVNBE ST0, STx");
                READFLAGS(X_CF | X_ZF);
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop & 7));
                v2 = x87_get_st(dyn, ninst, x1, x2, nextop & 7, X87_COMBINE(0, nextop & 7));
                RESTORE_EFLAGS(x5);
                ANDId(x1, xFlags, (1 << F_CF) | (1 << F_ZF));
                CBNZ_NEXT(x1);
                FMR(v1, v2); // F_CF==0 & F_ZF==0, copy STx to ST0
                break;
            case 0xD8 ... 0xDF:
                INST_NAME("FCMOVNU ST0, STx");
                READFLAGS(X_PF);
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop & 7));
                v2 = x87_get_st(dyn, ninst, x1, x2, nextop & 7, X87_COMBINE(0, nextop & 7));
                RESTORE_EFLAGS(x5);
                ANDId(x1, xFlags, 1 << F_PF);
                CBNZ_NEXT(x1);
                FMR(v1, v2); // F_PF==0, copy STx to ST0
                break;
            case 0xE1:
                INST_NAME("FDISI8087_NOP"); // so.. NOP?
                break;
            case 0xE2:
                INST_NAME("FNCLEX");
                LHZ(x2, offsetof(x64emu_t, sw), xEmu);
                // Clear bits 0-7 (IE..PE, SF, ES) and bit 15 (B)
                // Mask: keep bits 8-14, clear 0-7 and 15
                // 0x7F00 = 0111 1111 0000 0000
                ANDId(x2, x2, 0x7F00);
                STH(x2, offsetof(x64emu_t, sw), xEmu);
                break;
            case 0xE3:
                INST_NAME("FNINIT");
                MESSAGE(LOG_DUMP, "Need Optimization\n");
                x87_purgecache(dyn, ninst, 0, x1, x2, x3);
                CALL(const_reset_fpu, -1, 0, 0);
                NATIVE_RESTORE_X87PC();
                break;
            case 0xE8 ... 0xEF:
                INST_NAME("FUCOMI ST0, STx");
                SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop & 7));
                v2 = x87_get_st(dyn, ninst, x1, x2, nextop & 7, X87_COMBINE(0, nextop & 7));
                if (ST_IS_F(0)) {
                    FCOMIS(v1, v2, x1, x2);
                } else {
                    FCOMID(v1, v2, x1, x2);
                }
                break;
            case 0xF0 ... 0xF7:
                INST_NAME("FCOMI ST0, STx");
                SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop & 7));
                v2 = x87_get_st(dyn, ninst, x1, x2, nextop & 7, X87_COMBINE(0, nextop & 7));
                if (ST_IS_F(0)) {
                    FCOMIS(v1, v2, x1, x2);
                } else {
                    FCOMID(v1, v2, x1, x2);
                }
                break;
            default:
                DEFAULT;
                break;
        }
    else
        switch ((nextop >> 3) & 7) {
            case 0:
                INST_NAME("FILD ST0, Ed");
                X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, VMX_CACHE_ST_D);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, DS_DISP, 0);
                // Load 32-bit integer, sign-extend to 64-bit, convert to double
                // Use LWZ + EXTSW instead of LWA because LWA requires 4-byte aligned offset
                LWZ(x1, fixedaddress, ed);
                EXTSW(x1, x1);
                MTVSRD(VSXREG_X87(v1), x1);  // move int64 to FPR
                FCFID(v1, v1);            // int64 -> double
                break;
            case 1:
                INST_NAME("FISTTP Ed, ST0");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, VMX_CACHE_ST_D);
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x4, &fixedaddress, rex, NULL, DS_DISP, 0);
                s0 = fpu_get_scratch(dyn);
                // Clear FPSCR VXCVI bit before conversion
                MTFSB0(23);
                // Truncate double to int32 (round toward zero)
                FCTIWZ(s0, v1);
                // Move int32 result to GPR (use raw s0 since FCTIWZ writes to FPR space)
                MFVSRWZ(x5, s0);
                // Check FPSCR VXCVI: PPC gives 0x7FFFFFFF for +Inf, x86 wants 0x80000000
                MFFS(s0);           // read FPSCR into FPR (overwrites s0, result already in x5)
                MFVSRD(x4, s0);     // FPSCR to GPR (VXCVI = bit 23 from LSB)
                RLWINM(x4, x4, 24, 31, 31);  // extract VXCVI (ISA bit 23) → bit 0
                BEQZ_MARK(x4);     // if no overflow, skip to MARK
                MOV32w(x5, 0x80000000);  // x86 integer indefinite
                MARK;
                STW(x5, fixedaddress, wback);
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            case 2:
                INST_NAME("FIST Ed, ST0");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, VMX_CACHE_ST_D);
                u8 = x87_setround(dyn, ninst, x1, x5);
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x3, &fixedaddress, rex, NULL, DS_DISP, 0);
                s0 = fpu_get_scratch(dyn);
                // Clear FPSCR VXCVI bit before conversion
                MTFSB0(23);
                // Convert double to int32 using current rounding mode
                FCTIW(s0, v1);
                // Move low 32 bits from FPR to GPR (use raw s0, not VSXREG, since FCTIW writes to FPR space)
                MFVSRWZ(x5, s0);
                // Check FPSCR VXCVI: PPC gives 0x7FFFFFFF for +Inf, x86 wants 0x80000000
                MFFS(s0);           // read FPSCR into FPR (overwrites s0, result already in x5)
                MFVSRD(x4, s0);     // FPSCR to GPR (VXCVI = bit 23 from LSB)
                RLWINM(x4, x4, 24, 31, 31);  // extract VXCVI (ISA bit 23) → bit 0
                BEQZ_MARK(x4);     // if no overflow, skip
                MOV32w(x5, 0x80000000);  // x86 integer indefinite
                MARK;
                STW(x5, fixedaddress, wback);
                x87_restoreround(dyn, ninst, u8);
                break;
            case 3:
                INST_NAME("FISTP Ed, ST0");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, VMX_CACHE_ST_D);
                u8 = x87_setround(dyn, ninst, x1, x5);
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x3, &fixedaddress, rex, NULL, DS_DISP, 0);
                s0 = fpu_get_scratch(dyn);
                // Clear FPSCR VXCVI bit before conversion
                MTFSB0(23);
                // Convert double to int32 using current rounding mode
                FCTIW(s0, v1);
                // Move low 32 bits from FPR to GPR (use raw s0, not VSXREG, since FCTIW writes to FPR space)
                MFVSRWZ(x5, s0);
                // Check FPSCR VXCVI: PPC gives 0x7FFFFFFF for +Inf, x86 wants 0x80000000
                MFFS(s0);           // read FPSCR into FPR (overwrites s0, result already in x5)
                MFVSRD(x4, s0);     // FPSCR to GPR (VXCVI = bit 23 from LSB)
                RLWINM(x4, x4, 24, 31, 31);  // extract VXCVI (ISA bit 23) → bit 0
                BEQZ_MARK(x4);     // if no overflow, skip
                MOV32w(x5, 0x80000000);  // x86 integer indefinite
                MARK;
                STW(x5, fixedaddress, wback);
                x87_restoreround(dyn, ninst, u8);
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            case 5:
                INST_NAME("FLD tbyte");
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 8, 0);
                if ((PK(0) == 0xDB && ((PK(1) >> 3) & 7) == 7) || (!rex.is32bits && PK(0) >= 0x40 && PK(0) <= 0x4f && PK(1) == 0xDB && ((PK(2) >> 3) & 7) == 7)) {
                    NOTEST(x5);
                    // the FLD is immediately followed by an FSTP
                    LD(x5, fixedaddress + 0, ed);
                    LHZ(x6, fixedaddress + 8, ed);
                    // no persistent scratch register, so unroll both instructions here...
                    MESSAGE(LOG_DUMP, "\tHack: FSTP tbyte\n");
                    nextop = F8; // 0xDB or rex
                    if (!rex.is32bits && nextop >= 0x40 && nextop <= 0x4f) {
                        rex.rex = nextop;
                        nextop = F8; // 0xDB
                    } else
                        rex.rex = 0;
                    nextop = F8; // modrm
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 8, 0);
                    STD(x5, fixedaddress + 0, ed);
                    STH(x6, fixedaddress + 8, ed);
                } else {
                    if (BOX64ENV(x87_no80bits)) {
                        X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, VMX_CACHE_ST_D);
                        LFD(v1, fixedaddress, ed);
                    } else {
                        if (ed != x1 || fixedaddress) {
                            if (fixedaddress)
                                ADDI(x1, ed, fixedaddress);
                            else
                                MR(x1, ed);
                        }
                        // x1 now points to the 80-bit value
                        X87_PUSH_EMPTY_OR_FAIL(dyn, ninst, x3);
                        x87_reflectcount(dyn, ninst, x3, x4);
                        CALL(const_native_fld, -1, x1, 0);
                        x87_unreflectcount(dyn, ninst, x3, x4);
                    }
                }
                break;
            case 7:
                INST_NAME("FSTP tbyte");
                if (BOX64ENV(x87_no80bits)) {
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, VMX_CACHE_ST_D);
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, DS_DISP, 0);
                    STFD(v1, fixedaddress, wback);
                } else {
                    x87_forget(dyn, ninst, x1, x3, 0);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, NO_DISP, 0);
                    if (ed != x1 || fixedaddress) {
                        if (fixedaddress)
                            ADDI(x1, ed, fixedaddress);
                        else
                            MR(x1, ed);
                    }
                    x87_reflectcount(dyn, ninst, x3, x4);
                    CALL(const_native_fstp, -1, x1, 0);
                    x87_unreflectcount(dyn, ninst, x3, x4);
                }
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            default:
                DEFAULT;
        }
    return addr;
}
