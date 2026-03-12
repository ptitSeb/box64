#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "ppc64le_emitter.h"
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

// emit ADD32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_add32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SDxw(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, rex.w ? d_add64 : d_add32b);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    CLEAR_FLAGS(s3);
    IFX (X_CF) {
        if (rex.w) {
            ZEROUP2(s5, s1);
            ZEROUP2(s4, s2);
            ADD(s5, s5, s4);
            SRDI(s3, s1, 0x20);
            SRDI(s4, s2, 0x20);
            ADD(s4, s4, s3);
            SRDI(s5, s5, 0x20);
            ADD(s5, s5, s4); // hi
            SRADI(s5, s5, 0x20);
            CMPDI(s5, 0);
            BEQ(8);
            ORI(xFlags, xFlags, 1 << F_CF);
        } else {
            ZEROUP2(s3, s1);
            ZEROUP2(s4, s2);
            ADD(s5, s3, s4);
            SRDI(s5, s5, 0x20);
            CMPDI(s5, 0);
            BEQ(8);
            ORI(xFlags, xFlags, 1 << F_CF);
        }
    }
    IFX (X_AF | X_OF) {
        OR(s3, s1, s2);  // s3 = op1 | op2
        AND(s4, s1, s2); // s4 = op1 & op2
    }

    ADDxw(s1, s1, s2);

    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_AF | X_OF) {
        ANDC(s3, s3, s1); // s3 = ~res & (op1 | op2)
        OR(s3, s3, s4);   // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX (X_AF) {
            ANDId(s4, s3, 0x08); // AF: cc & 0x08
            CMPDI(s4, 0);
            BEQ(8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX (X_OF) {
            SRDI(s3, s3, rex.w ? 62 : 30);
            SRDI(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDId(s3, s3, 1); // OF: xor of two MSB's of cc
            BF_INSERT(xFlags, s3, F_OF, F_OF);
        }
    }
    IFX (X_SF) {
        SRDI(s3, s1, rex.w ? 63 : 31);
        CMPDI(s3, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit ADD32 instruction, from s1, constant c, store result in s1 using s3 and s4 as scratch
void emit_add32c(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s2, int s3, int s4, int s5)
{
    if ((s1 == xRSP) && (BOX64DRENV(dynarec_safeflags) < 2) && (!dyn->insts || (dyn->insts[ninst].x64.gen_flags == X_PEND) || (!BOX64ENV(dynarec_df) && (dyn->insts[ninst].x64.gen_flags == X_ALL)))) {
        // special case when doing math on ESP and only PEND is needed: ignoring it!
        if (c >= -32768 && c < 32768) {
            ADDI(s1, s1, c);
        } else {
            MOV64x(s2, c);
            ADD(s1, s1, s2);
        }
        if (!rex.w) { ZEROUP(s1); }
        return;
    }
    IFX (X_PEND | X_AF | X_CF | X_OF) {
        MOV64xw(s2, c);
    } else if (c < -32768 || c >= 32768) {
        MOV64xw(s2, c);
    }
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SDxw(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, rex.w ? d_add64 : d_add32b);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    CLEAR_FLAGS(s3);
    IFX (X_CF) {
        if (rex.w) {
            ZEROUP2(s5, s1);
            ZEROUP2(s4, s2);
            ADD(s5, s5, s4);
            SRDI(s3, s1, 0x20);
            SRDI(s4, s2, 0x20);
            ADD(s4, s4, s3);
            SRDI(s5, s5, 0x20);
            ADD(s5, s5, s4); // hi
            SRADI(s5, s5, 0x20);
            CMPDI(s5, 0);
            BEQ(8);
            ORI(xFlags, xFlags, 1 << F_CF);
        } else {
            ZEROUP2(s3, s1);
            ZEROUP2(s4, s2);
            ADD(s5, s3, s4);
            SRDI(s5, s5, 0x20);
            CMPDI(s5, 0);
            BEQ(8);
            ORI(xFlags, xFlags, 1 << F_CF);
        }
    }
    IFX (X_AF | X_OF) {
        OR(s3, s1, s2);  // s3 = op1 | op2
        AND(s4, s1, s2); // s4 = op1 & op2
    }

    if (c >= -32768 && c < 32768) {
        ADDIxw(s1, s1, c);
    } else {
        ADDxw(s1, s1, s2);
    }

    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_AF | X_OF) {
        ANDC(s3, s3, s1); // s3 = ~res & (op1 | op2)
        OR(s3, s3, s4);   // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX (X_AF) {
            ANDId(s4, s3, 0x08); // AF: cc & 0x08
            CMPDI(s4, 0);
            BEQ(8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX (X_OF) {
            SRDI(s3, s3, rex.w ? 62 : 30);
            SRDI(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDId(s3, s3, 1); // OF: xor of two MSB's of cc
            BF_INSERT(xFlags, s3, F_OF, F_OF);
        }
    }
    IFX (X_SF) {
        SRDI(s3, s1, rex.w ? 63 : 31);
        CMPDI(s3, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit ADD8 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_add8(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX (X_PEND) {
        STB(s1, offsetof(x64emu_t, op1), xEmu);
        STB(s2, offsetof(x64emu_t, op2), xEmu);
        SET_DF(s3, d_add8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    CLEAR_FLAGS(s3);
    IFX (X_AF | X_OF) {
        OR(s3, s1, s2);  // s3 = op1 | op2
        AND(s4, s1, s2); // s4 = op1 & op2
    }
    ADD(s1, s1, s2);

    IFX (X_AF | X_OF) {
        ANDC(s3, s3, s1); // s3 = ~res & (op1 | op2)
        OR(s3, s3, s4);   // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX (X_AF) {
            ANDId(s4, s3, 0x08); // AF: cc & 0x08
            CMPDI(s4, 0);
            BEQ(8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX (X_OF) {
            SRDI(s3, s3, 6);
            SRDI(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDId(s3, s3, 1); // OF: xor of two MSB's of cc
            BF_INSERT(xFlags, s3, F_OF, F_OF);
        }
    }
    IFX (X_CF) {
        SRDI(s3, s1, 8);
        CMPDI(s3, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_CF);
    }
    // d_add8 will use 16bits result to check for CF, so save 16bits and truncate after
    IFX (X_PEND) {
        STH(s1, offsetof(x64emu_t, res), xEmu);
    }
    ANDId(s1, s1, 0xff);
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_SF) {
        SRDI(s3, s1, 7);
        CMPDI(s3, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit ADD8 instruction, from s1, const c, store result in s1 using s3 and s4 as scratch
void emit_add8c(dynarec_ppc64le_t* dyn, int ninst, int s1, int32_t c, int s2, int s3, int s4)
{
    IFX (X_PEND) {
        MOV32w(s4, c & 0xff);
        STB(s1, offsetof(x64emu_t, op1), xEmu);
        STB(s4, offsetof(x64emu_t, op2), xEmu);
        SET_DF(s3, d_add8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    CLEAR_FLAGS(s3);
    IFX (X_AF | X_OF) {
        IFX (X_PEND) {
        } else {
            MOV32w(s4, c & 0xff);
        }
        OR(s3, s1, s4);  // s3 = op1 | op2
        AND(s4, s1, s4); // s4 = op1 & op2
    }
    ADDI(s1, s1, c & 0xff);

    IFX (X_AF | X_OF) {
        ANDC(s3, s3, s1); // s3 = ~res & (op1 | op2)
        OR(s3, s3, s4);   // cc = (~res & (op1 | op2)) | (op1 & op2)
        IFX (X_AF) {
            ANDId(s4, s3, 0x08); // AF: cc & 0x08
            CMPDI(s4, 0);
            BEQ(8);
            ORI(xFlags, xFlags, 1 << F_AF);
        }
        IFX (X_OF) {
            SRDI(s3, s3, 6);
            SRDI(s4, s3, 1);
            XOR(s3, s3, s4);
            ANDId(s3, s3, 1); // OF: xor of two MSB's of cc
            BF_INSERT(xFlags, s3, F_OF, F_OF);
        }
    }
    IFX (X_CF) {
        SRDI(s3, s1, 8);
        CMPDI(s3, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_CF);
    }
    // d_add8 will use 16bits result to check for CF, so save 16bits and truncate after
    IFX (X_PEND) {
        STH(s1, offsetof(x64emu_t, res), xEmu);
    }
    ANDId(s1, s1, 0xff);
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_SF) {
        SRDI(s3, s1, 7);
        CMPDI(s3, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}


