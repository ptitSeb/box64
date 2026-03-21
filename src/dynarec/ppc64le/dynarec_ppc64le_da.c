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


uintptr_t dynarec64_DA(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t nextop = F8;
    uint8_t ed, wback, wb1, u8;
    int v1, v2;
    int d0;
    int s0;
    int64_t j64;
    int64_t fixedaddress;
    int unscaled;

    MAYUSE(u8);
    MAYUSE(wb1);
    MAYUSE(d0);
    MAYUSE(s0);
    MAYUSE(v1);
    MAYUSE(v2);
    MAYUSE(j64);
    MAYUSE(ed);

    if (MODREG)
        switch (nextop) {
            case 0xC0 ... 0xC7:
                INST_NAME("FCMOVB ST0, STx");
                READFLAGS(X_CF);
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop & 7));
                v2 = x87_get_st(dyn, ninst, x1, x2, nextop & 7, X87_COMBINE(0, nextop & 7));
                RESTORE_EFLAGS(x5);
                ANDId(x1, xFlags, 1 << F_CF);
                CBZ_NEXT(x1);   // skip if CF==0 (condition not met)
                FMR(v1, v2);    // CF==1 (below), copy STx to ST0
                break;
            case 0xC8 ... 0xCF:
                INST_NAME("FCMOVE ST0, STx");
                READFLAGS(X_ZF);
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop & 7));
                v2 = x87_get_st(dyn, ninst, x1, x2, nextop & 7, X87_COMBINE(0, nextop & 7));
                RESTORE_EFLAGS(x5);
                ANDId(x1, xFlags, 1 << F_ZF);
                CBZ_NEXT(x1);   // skip if ZF==0
                FMR(v1, v2);    // ZF==1 (equal), copy STx to ST0
                break;
            case 0xD0 ... 0xD7:
                INST_NAME("FCMOVBE ST0, STx");
                READFLAGS(X_CF | X_ZF);
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop & 7));
                v2 = x87_get_st(dyn, ninst, x1, x2, nextop & 7, X87_COMBINE(0, nextop & 7));
                RESTORE_EFLAGS(x5);
                ANDId(x1, xFlags, (1 << F_CF) | (1 << F_ZF));
                CBZ_NEXT(x1);   // skip if both CF==0 and ZF==0
                FMR(v1, v2);    // CF==1 or ZF==1 (below or equal), copy STx to ST0
                break;
            case 0xD8 ... 0xDF:
                INST_NAME("FCMOVU ST0, STx");
                READFLAGS(X_PF);
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop & 7));
                v2 = x87_get_st(dyn, ninst, x1, x2, nextop & 7, X87_COMBINE(0, nextop & 7));
                RESTORE_EFLAGS(x5);
                ANDId(x1, xFlags, 1 << F_PF);
                CBZ_NEXT(x1);   // skip if PF==0
                FMR(v1, v2);    // PF==1 (unordered), copy STx to ST0
                break;
            case 0xE9:
                INST_NAME("FUCOMPP ST0, ST1");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop & 7));
                v2 = x87_get_st(dyn, ninst, x1, x2, 1, X87_COMBINE(0, nextop & 7));
                if (ST_IS_F(0)) {
                    FCOMS(v1, v2, x1, x2, x3);
                } else {
                    FCOMD(v1, v2, x1, x2, x3);
                }
                X87_POP_OR_FAIL(dyn, ninst, x3);
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            default:
                DEFAULT;
                break;
        }
    else
        switch ((nextop >> 3) & 7) {
            case 0:
                INST_NAME("FIADD ST0, Ed");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, VMX_CACHE_ST_D);
                v2 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, DS_DISP, 0);
                LWZ(x1, fixedaddress, ed);      // load int32
                EXTSW(x1, x1);                  // sign-extend to 64-bit
                MTVSRD(v2, x1);                   // move to FPR (raw idx = FPR space)
                FCFID(v2, v2);                   // int64 -> double
                if (!BOX64ENV(dynarec_fastround)) u8 = x87_setround(dyn, ninst, x1, x5);
                FADD(v1, v1, v2);
                X87_CHECK_PRECISION(v1);
                if (!BOX64ENV(dynarec_fastround)) x87_restoreround(dyn, ninst, u8);
                break;
            case 1:
                INST_NAME("FIMUL ST0, Ed");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, VMX_CACHE_ST_D);
                v2 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, DS_DISP, 0);
                LWZ(x1, fixedaddress, ed);
                EXTSW(x1, x1);
                MTVSRD(v2, x1);
                FCFID(v2, v2);
                if (!BOX64ENV(dynarec_fastround)) u8 = x87_setround(dyn, ninst, x1, x5);
                FMUL(v1, v1, v2);
                X87_CHECK_PRECISION(v1);
                if (!BOX64ENV(dynarec_fastround)) x87_restoreround(dyn, ninst, u8);
                break;
            case 2:
                INST_NAME("FICOM ST0, Ed");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, VMX_CACHE_ST_D);
                v2 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, DS_DISP, 0);
                LWZ(x1, fixedaddress, ed);
                EXTSW(x1, x1);
                MTVSRD(v2, x1);
                FCFID(v2, v2);
                FCOMD(v1, v2, x1, x2, x3);
                break;
            case 3:
                INST_NAME("FICOMP ST0, Ed");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, VMX_CACHE_ST_D);
                v2 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, DS_DISP, 0);
                LWZ(x1, fixedaddress, ed);
                EXTSW(x1, x1);
                MTVSRD(v2, x1);
                FCFID(v2, v2);
                FCOMD(v1, v2, x1, x2, x3);
                X87_POP_OR_FAIL(dyn, ninst, x3);
                break;
            case 4:
                INST_NAME("FISUB ST0, Ed");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, VMX_CACHE_ST_D);
                v2 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, DS_DISP, 0);
                LWZ(x1, fixedaddress, ed);
                EXTSW(x1, x1);
                MTVSRD(v2, x1);
                FCFID(v2, v2);
                if (!BOX64ENV(dynarec_fastround)) u8 = x87_setround(dyn, ninst, x1, x5);
                FSUB(v1, v1, v2);
                X87_CHECK_PRECISION(v1);
                if (!BOX64ENV(dynarec_fastround)) x87_restoreround(dyn, ninst, u8);
                break;
            case 5:
                INST_NAME("FISUBR ST0, Ed");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, VMX_CACHE_ST_D);
                v2 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, DS_DISP, 0);
                LWZ(x1, fixedaddress, ed);
                EXTSW(x1, x1);
                MTVSRD(v2, x1);
                FCFID(v2, v2);
                if (!BOX64ENV(dynarec_fastround)) u8 = x87_setround(dyn, ninst, x1, x5);
                FSUB(v1, v2, v1);  // reversed: v2 - v1
                X87_CHECK_PRECISION(v1);
                if (!BOX64ENV(dynarec_fastround)) x87_restoreround(dyn, ninst, u8);
                break;
            case 6:
                INST_NAME("FIDIV ST0, Ed");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, VMX_CACHE_ST_D);
                v2 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, DS_DISP, 0);
                LWZ(x1, fixedaddress, ed);
                EXTSW(x1, x1);
                MTVSRD(v2, x1);
                FCFID(v2, v2);
                if (!BOX64ENV(dynarec_fastround)) u8 = x87_setround(dyn, ninst, x1, x5);
                FDIV(v1, v1, v2);
                X87_CHECK_PRECISION(v1);
                if (!BOX64ENV(dynarec_fastround)) x87_restoreround(dyn, ninst, u8);
                break;
            case 7:
                INST_NAME("FIDIVR ST0, Ed");
                v1 = x87_get_st(dyn, ninst, x1, x2, 0, VMX_CACHE_ST_D);
                v2 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, DS_DISP, 0);
                LWZ(x1, fixedaddress, ed);
                EXTSW(x1, x1);
                MTVSRD(v2, x1);
                FCFID(v2, v2);
                if (!BOX64ENV(dynarec_fastround)) u8 = x87_setround(dyn, ninst, x1, x5);
                FDIV(v1, v2, v1);  // reversed: v2 / v1
                X87_CHECK_PRECISION(v1);
                if (!BOX64ENV(dynarec_fastround)) x87_restoreround(dyn, ninst, u8);
                break;
        }
    return addr;
}
