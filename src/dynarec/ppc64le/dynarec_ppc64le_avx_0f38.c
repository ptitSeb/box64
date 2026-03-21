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
#include "dynarec_native.h"

#include "ppc64le_printer.h"
#include "dynarec_ppc64le_private.h"
#include "dynarec_ppc64le_functions.h"
#include "../dynarec_helper.h"
#include "dynarec_ppc64le_helper.h"

uintptr_t dynarec64_AVX_0F38(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed, vd;
    uint8_t wback, wb1, wb2;
    int64_t j64;
    int64_t fixedaddress;
    int unscaled;
    rex_t rex = vex.rex;
    MAYUSE(u8);
    MAYUSE(j64);
    MAYUSE(wback);
    MAYUSE(wb1);
    MAYUSE(wb2);

    switch (opcode) {
        case 0xF2:
            // ANDN: gd = ed & ~vd, set SF/ZF, clear OF/CF
            INST_NAME("ANDN Gd, Vd, Ed");
            nextop = F8;
            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            GETGD;
            GETED(0);
            GETVD;
            ANDC(gd, ed, vd);
            if (!rex.w) {
                ZEROUP(gd);
            }
            CLEAR_FLAGS(x6);
            IFX (X_SF) {
                SRDI(x6, gd, rex.w ? 63 : 31);
                SLDI(x6, x6, F_SF);
                OR(xFlags, xFlags, x6);
            }
            IFX (X_ZF) {
                CMPDI(gd, 0);
                BNE(8);
                ORI(xFlags, xFlags, 1 << F_ZF);
            }
            SPILL_EFLAGS();
            break;
        case 0xF3:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 1:
                    // BLSR: vd = ed & (ed - 1), CF = (ed == 0)
                    INST_NAME("BLSR Vd, Ed");
                    SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
                    GETED(0);
                    GETVD;
                    CLEAR_FLAGS(x6);
                    IFX (X_CF) {
                        CMPDI(ed, 0);
                        BNE(8);
                        ORI(xFlags, xFlags, 1 << F_CF);
                    }
                    ADDIxw(x3, ed, -1);
                    AND(vd, ed, x3);
                    if (!rex.w) {
                        ZEROUP(vd);
                    }
                    IFX (X_ZF) {
                        CMPDI(vd, 0);
                        BNE(8);
                        ORI(xFlags, xFlags, 1 << F_ZF);
                    }
                    IFX (X_SF) {
                        SRDI(x5, vd, rex.w ? 63 : 31);
                        SLDI(x5, x5, F_SF);
                        OR(xFlags, xFlags, x5);
                    }
                    SPILL_EFLAGS();
                    break;
                case 2:
                    // BLSMSK: vd = ed ^ (ed - 1), CF = (ed == 0)
                    INST_NAME("BLSMSK Vd, Ed");
                    SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
                    GETED(0);
                    GETVD;
                    CLEAR_FLAGS(x6);
                    IFX (X_CF) {
                        CMPDI(ed, 0);
                        BNE(8);
                        ORI(xFlags, xFlags, 1 << F_CF);
                    }
                    ADDIxw(x3, ed, -1);
                    XOR(vd, ed, x3);
                    if (!rex.w) {
                        ZEROUP(vd);
                    }
                    IFX (X_SF) {
                        SRDI(x5, vd, rex.w ? 63 : 31);
                        SLDI(x5, x5, F_SF);
                        OR(xFlags, xFlags, x5);
                    }
                    SPILL_EFLAGS();
                    break;
                case 3:
                    // BLSI: vd = ed & (-ed), CF = (ed != 0)
                    INST_NAME("BLSI Vd, Ed");
                    SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
                    GETED(0);
                    GETVD;
                    CLEAR_FLAGS(x6);
                    IFX (X_CF) {
                        CMPDI(ed, 0);
                        BEQ(8);
                        ORI(xFlags, xFlags, 1 << F_CF);
                    }
                    NEG(x3, ed);
                    if (!rex.w) ZEROUP(x3);
                    AND(vd, ed, x3);
                    if (!rex.w) {
                        ZEROUP(vd);
                    }
                    IFX (X_ZF) {
                        CMPDI(vd, 0);
                        BNE(8);
                        ORI(xFlags, xFlags, 1 << F_ZF);
                    }
                    IFX (X_SF) {
                        SRDI(x5, vd, rex.w ? 63 : 31);
                        SLDI(x5, x5, F_SF);
                        OR(xFlags, xFlags, x5);
                    }
                    SPILL_EFLAGS();
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xF5:
            // BZHI: zero high bits starting at bit position vd[7:0]
            // gd = ed & ((1 << n) - 1) if n < opsize, else gd = ed with CF set
            INST_NAME("BZHI Gd, Ed, Vd");
            nextop = F8;
            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            GETGD;
            GETED(0);
            GETVD;
            CLEAR_FLAGS(x6);
            ANDI(x4, vd, 0xFF);            // x4 = n = vd[7:0]
            MOV64xw(x5, rex.w ? 64 : 32);
            BGE_MARK(x4, x5);              // if n >= opsize, jump to MARK
            // n < opsize: build mask = ~((-1) << n), apply
            MOV64x(x6, -1);
            SLD(x6, x6, x4);               // x6 = (-1) << n
            ANDC(gd, ed, x6);              // gd = ed & ~((-1) << n)
            B_MARK2_nocond;
            MARK;
            // n >= opsize: result = ed, set CF
            MR(gd, ed);
            IFX (X_CF) {
                ORI(xFlags, xFlags, 1 << F_CF);
            }
            MARK2;
            if (!rex.w) {
                ZEROUP(gd);
            }
            IFX (X_ZF) {
                CMPDI(gd, 0);
                BNE(8);
                ORI(xFlags, xFlags, 1 << F_ZF);
            }
            IFX (X_SF) {
                SRDI(x5, gd, rex.w ? 63 : 31);
                SLDI(x5, x5, F_SF);
                OR(xFlags, xFlags, x5);
            }
            SPILL_EFLAGS();
            break;
        case 0xF7:
            // BEXTR: extract bit field from ed, start=vd[7:0], len=vd[15:8]
            INST_NAME("BEXTR Gd, Ed, Vd");
            nextop = F8;
            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            GETGD;
            GETED(0);
            GETVD;
            ANDI(x4, vd, 0xFF);                    // x4 = start
            SRDI(x3, vd, 8);
            ANDI(x3, x3, 0xFF);                    // x3 = length
            XOR(x5, x5, x5);                       // x5 = 0 (default result)
            BEQZ_MARK(x3);                         // if length == 0, result = 0
            MOV64xw(x6, rex.w ? 64 : 32);
            BGE_MARK(x4, x6);                      // if start >= opsize, result = 0
            // Shift right by start
            SRLxw(x5, ed, x4);
            // If length >= opsize, keep all bits (skip masking)
            BLT_MARK(x6, x3);
            // Mask to length bits: shift left by (opsize - length), then shift right
            SUBxw(x6, x6, x3);
            SLLxw(x5, x5, x6);
            SRLxw(x5, x5, x6);
            MARK;
            MR(gd, x5);
            if (!rex.w) {
                ZEROUP(gd);
            }
            CLEAR_FLAGS(x6);
            IFX (X_ZF) {
                CMPDI(gd, 0);
                BNE(8);
                ORI(xFlags, xFlags, 1 << F_ZF);
            }
            SPILL_EFLAGS();
            break;
        default:
            DEFAULT;
    }

    return addr;
}
