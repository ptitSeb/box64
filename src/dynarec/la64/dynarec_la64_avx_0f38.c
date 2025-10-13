#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "env.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"

#include "la64_printer.h"
#include "dynarec_la64_private.h"
#include "dynarec_la64_functions.h"
#include "../dynarec_helper.h"

uintptr_t dynarec64_AVX_0F38(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed, vd;
    uint8_t wback, wb1, wb2;
    uint8_t eb1, eb2, gb1, gb2;
    int32_t i32, i32_;
    int cacheupd = 0;
    int v0, v1, v2;
    int q0, q1, q2;
    int d0, d1, d2;
    int s0;
    uint64_t tmp64u;
    int64_t j64;
    int64_t fixedaddress;
    int unscaled;
    MAYUSE(wb1);
    MAYUSE(wb2);
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(gb1);
    MAYUSE(gb2);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(s0);
    MAYUSE(j64);
    MAYUSE(cacheupd);

    rex_t rex = vex.rex;

    switch (opcode) {
        case 0xF2:
            INST_NAME("ANDN Gd, Vd, Ed");
            nextop = F8;
            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            GETGD;
            GETED(0);
            GETVD;
            ANDN(gd, ed, vd);
            if (!rex.w) {
                BSTRPICK_D(gd, gd, 31, 0);
            }
            CLEAR_FLAGS(x6);
            IFX (X_SF) {
                SRLI_D(x6, gd, rex.w ? 63 : 31);
                SLLI_D(x6, x6, F_SF);
                OR(xFlags, xFlags, x6);
            }
            IFX (X_ZF) {
                BNEZ(gd, 8);
                ORI(xFlags, xFlags, 1 << F_ZF);
            }
            SPILL_EFLAGS();
            break;
        case 0xF3:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 1:
                    INST_NAME("BLSR Vd, Ed");
                    SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
                    GETED(0);
                    GETVD;
                    CLEAR_FLAGS(x6);
                    IFX (X_CF) {
                        BNEZ(ed, 8);
                        ORI(xFlags, xFlags, 1 << F_CF);
                    }
                    ADDIxw(x3, ed, -1);
                    AND(vd, ed, x3);
                    if (!rex.w) {
                        BSTRPICK_D(vd, vd, 31, 0);
                    }
                    IFX (X_ZF) {
                        BNEZ(vd, 8);
                        ORI(xFlags, xFlags, 1 << F_ZF);
                    }
                    IFX (X_SF) {
                        BSTRPICK_D(x5, vd, rex.w ? 63 : 31, rex.w ? 63 : 31);
                        SLLI_D(x5, x5, F_SF);
                        OR(xFlags, xFlags, x5);
                    }
                    SPILL_EFLAGS();
                    break;
                case 2:
                    INST_NAME("BLSMSK Vd, Ed");
                    SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
                    GETED(0);
                    GETVD;
                    CLEAR_FLAGS(x6);
                    IFX (X_CF) {
                        BNEZ(ed, 8);
                        ORI(xFlags, xFlags, 1 << F_CF);
                    }
                    ADDIxw(x3, ed, -1);
                    XOR(vd, ed, x3);
                    if (!rex.w) {
                        BSTRPICK_D(vd, vd, 31, 0);
                    }
                    IFX (X_SF) {
                        BSTRPICK_D(x5, vd, rex.w ? 63 : 31, rex.w ? 63 : 31);
                        SLLI_D(x5, x5, F_SF);
                        OR(xFlags, xFlags, x5);
                    }
                    SPILL_EFLAGS();
                    break;
                case 3:
                    INST_NAME("BLSI Vd, Ed");
                    SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
                    GETED(0);
                    GETVD;
                    CLEAR_FLAGS(x6);
                    IFX (X_CF) {
                        BEQZ(ed, 8);
                        ORI(xFlags, xFlags, 1 << F_CF);
                    }
                    SUBxw(x3, xZR, ed);
                    AND(vd, ed, x3);
                    if (!rex.w) {
                        BSTRPICK_D(vd, vd, 31, 0);
                    }
                    IFX (X_ZF) {
                        BNEZ(vd, 8);
                        ORI(xFlags, xFlags, 1 << F_ZF);
                    }
                    IFX (X_SF) {
                        BSTRPICK_D(x5, vd, rex.w ? 63 : 31, rex.w ? 63 : 31);
                        SLLI_D(x5, x5, F_SF);
                        OR(xFlags, xFlags, x5);
                    }
                    SPILL_EFLAGS();
                    break;
                default:
                    DEFAULT;
            }
            break;

        case 0xF5:
            INST_NAME("BZHI Gd, Ed, Vd");
            nextop = F8;
            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            GETGD;
            GETED(0);
            GETVD;
            CLEAR_FLAGS(x6);
            BSTRPICK_D(x4, vd, 7, 0);
            MOV64x(x5, rex.w ? 64 : 32);
            BGE_MARK(x4, x5);
            ADDI_D(x6, xZR, -1);
            SLL_D(x6, x6, x4);
            ANDN(gd, ed, x6);
            B_MARK2_nocond;
            MARK;
            OR(gd, ed, ed);
            IFX (X_CF) {
                ORI(xFlags, xFlags, 1 << F_CF);
            }
            MARK2;
            if (!rex.w) {
                BSTRPICK_D(gd, gd, 31, 0);
            }
            IFX (X_ZF) {
                BNEZ(gd, 8);
                ORI(xFlags, xFlags, 1 << F_ZF);
            }
            IFX (X_SF) {
                BSTRPICK_D(x5, gd, rex.w ? 63 : 31, rex.w ? 63 : 31);
                SLLI_D(x5, x5, F_SF);
                OR(xFlags, xFlags, x5);
            }
            SPILL_EFLAGS();
            break;

        case 0xF7:
            INST_NAME("BEXTR Gd, Vd, Ed");
            nextop = F8;
            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            GETGD;
            GETED(0);
            GETVD;
            BSTRPICK_D(x4, vd, 7, 0);  // start
            BSTRPICK_D(x3, vd, 15, 8); // length
            XOR(x5, x5, x5);
            BEQZ_MARK(x3);
            MOV64xw(x6, rex.w ? 64 : 32);
            BGE_MARK(x4, x6);
            SRLxw(x5, ed, x4);
            BLT_MARK(x6, x3);
            SUBxw(x6, x6, x3);
            SLLxw(x5, x5, x6);
            SRLxw(x5, x5, x6);
            MARK;
            OR(gd, x5, x5);
            if (!rex.w) {
                BSTRPICK_D(gd, gd, 31, 0);
            }
            CLEAR_FLAGS(x6);
            IFX (X_ZF) {
                BNEZ(gd, 8);
                ORI(xFlags, xFlags, 1 << F_ZF);
            }
            SPILL_EFLAGS();
            break;
        default:
            DEFAULT;
    }
    return addr;
}
