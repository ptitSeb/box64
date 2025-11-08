#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "la64_emitter.h"
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

// emit SHL16 instruction, from s1 , shift s2, store result in s1 using s3, s4 and s5 as scratch
void emit_shl16(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    // s2 is not 0 here and is 1..1f/3f
    IFX (X_PEND) {
        ST_H(s1, xEmu, offsetof(x64emu_t, op1));
        ST_H(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, d_shl16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    if (cpuext.lbt) {
        IFX (X_ALL) {
            X64_SLL_H(s1, s2);
        }
        SLL_D(s1, s1, s2);
        BSTRPICK_D(s1, s1, 15, 0);
        IFX (X_PEND) {
            ST_H(s1, xEmu, offsetof(x64emu_t, res));
        }

        return;
    }

    SLL_D(s1, s1, s2);

    CLEAR_FLAGS(s3);
    IFX (X_CF | X_OF) {
        SRLI_D(s5, s1, 16);
        ANDI(s5, s5, 1); // LSB == F_CF
        IFX (X_CF) {
            OR(xFlags, xFlags, s5);
        }
    }

    SLLI_D(s1, s1, 48);
    IFX (X_SF) {
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    SRLI_D(s1, s1, 48);

    IFX (X_PEND) {
        ST_H(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_OF) {
        // OF flag is affected only on 1-bit shifts
        ADDI_D(s3, s2, -1);
        BNEZ(s3, 4 + 4 * 4);
        SRLI_D(s3, s1, 15);
        XOR(s3, s3, s5);
        SLLI_D(s3, s3, F_OF);
        OR(xFlags, xFlags, s3);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit SHL16 instruction, from s1 , constant c, store result in s1 using s3, s4 and s5 as scratch
void emit_shl16c(dynarec_la64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4, int s5)
{
    if (!c) return;
    // c != 0

    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    IFX (X_PEND) {
        MOV64x(s3, c);
        ST_H(s3, xEmu, offsetof(x64emu_t, op2));
        ST_H(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, d_shl16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    if (cpuext.lbt) {
        IFX (X_PEND) {
        } else {
            MOV64x(s3, c);
        }
        IFX (X_ALL) {
            X64_SLL_H(s1, s3);
        }

        SLLI_D(s1, s1, c);
        BSTRPICK_D(s1, s1, 15, 0);

        IFX (X_PEND) {
            ST_H(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }

    CLEAR_FLAGS(s3);
    if (c < 16) {
        IFX (X_CF | X_OF) {
            SRLI_D(s3, s1, 16 - c);
            ANDI(s5, s3, 1); // LSB == F_CF
            IFX (X_CF) {
                OR(xFlags, xFlags, s5);
            }
        }

        SLLI_D(s1, s1, c + 48);
        IFX (X_SF) {
            BGE(s1, xZR, 8);
            ORI(xFlags, xFlags, 1 << F_SF);
        }
        SRLI_D(s1, s1, 48);

        IFX (X_PEND) {
            ST_H(s1, xEmu, offsetof(x64emu_t, res));
        }
        IFX (X_ZF) {
            BNEZ(s1, 8);
            ORI(xFlags, xFlags, 1 << F_ZF);
        }
        IFX (X_OF) {
            // OF flag is affected only on 1-bit shifts
            if (c == 1) {
                SRLI_D(s3, s1, 15);
                XOR(s3, s3, s5);
                SLLI_D(s3, s3, F_OF);
                OR(xFlags, xFlags, s3);
            }
        }
        IFX (X_PF) {
            emit_pf(dyn, ninst, s1, s3, s4);
        }
    } else {
        IFX (X_CF) {
            if (c == 16) {
                ANDI(s3, s1, 1);
                OR(xFlags, xFlags, s3); // F_CF == 0
            }
        }
        MV(s1, xZR);

        IFX (X_PEND) {
            ST_H(s1, xEmu, offsetof(x64emu_t, res));
        }
        // OF nop
        // SF nop
        // AF nop
        IFX (X_PF | X_ZF) {
            IFX (X_ZF) {
                ORI(xFlags, xFlags, 1 << F_ZF);
            }
            IFX (X_PF) {
                ORI(xFlags, xFlags, 1 << F_PF);
            }
        }
    }
}

// emit SHL32 instruction, from s1 , shift s2, store result in s1 using s3, s4 and s5 as scratch
void emit_shl32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    // s2 is not 0 here and is 1..1f/3f
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SDxw(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, rex.w ? d_shl64 : d_shl32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    if (cpuext.lbt) {
        IFX (X_ALL) {
            if (rex.w)
                X64_SLL_D(s1, s2);
            else
                X64_SLL_W(s1, s2);
        }
        IFXA (F_OF, (BOX64DRENV(dynarec_safeflags) > 1)) {
            SRLIxw(s3, s1, rex.w ? 62 : 30);
            SRLI_D(s5, s3, 1);
            XOR(s3, s3, s5);
            SLLI_D(s3, s3, F_OF);
            X64_SET_EFLAGS(s3, X_OF);
        }
        SLL_D(s1, s1, s2);
        if (!rex.w) { ZEROUP(s1); }
        IFX (X_PEND) {
            SDxw(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }

    CLEAR_FLAGS(s3);
    IFX (X_OF) {
        SRLIxw(s3, s1, rex.w ? 62 : 30);
        SRLI_D(s5, s3, 1);
        XOR(s3, s3, s5);
        BSTRINS_D(xFlags, x3, F_OF, F_OF);
    }
    IFX (X_CF) {
        ADDI_D(s5, s2, rex.w ? -64 : -32);
        NEG_D(s5, s5);
        SRL_D(s3, s1, s5);
        BSTRINS_D(xFlags, s3, F_CF, F_CF);
    }
    SLL_D(s1, s1, s2);

    IFX (X_SF) {
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit SHL32 instruction, from s1 , constant c, store result in s1 using s3, s4 and s5 as scratch
void emit_shl32c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4, int s5)
{
    if (!c) return;

    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    IFX (X_PEND) {
        if (c) {
            MOV64x(s3, c);
            SDxw(s3, xEmu, offsetof(x64emu_t, op2));
        } else {
            SDxw(xZR, xEmu, offsetof(x64emu_t, op2));
        }
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, rex.w ? d_shl64 : d_shl32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    if (cpuext.lbt) {
        IFX (X_PEND) {
        } else {
            MOV64x(s3, c);
        }
        IFX (X_ALL) {
            if (rex.w)
                X64_SLL_D(s1, s3);
            else
                X64_SLL_W(s1, s3);
        }
        IFXA (F_OF, (BOX64DRENV(dynarec_safeflags) > 1)) {
            SRLIxw(s3, s1, rex.w ? 62 : 30);
            SRLI_D(s5, s3, 1);
            XOR(s3, s3, s5);
            SLLI_D(s3, s3, F_OF);
            X64_SET_EFLAGS(s3, X_OF);
        }
        SLLIxw(s1, s1, c);
        if (!rex.w) ZEROUP(s1);
        IFX (X_PEND) {
            SDxw(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }

    CLEAR_FLAGS(s3);
    IFX (X_OF) {
        SRLIxw(s3, s1, rex.w ? 62 : 30);
        SRLI_D(s5, s3, 1);
        XOR(s3, s3, s5);
        BSTRINS_D(xFlags, x3, F_OF, F_OF);
    }
    IFX (X_CF) {
        SRLI_D(s3, s1, (rex.w ? 64 : 32) - c);
        BSTRINS_D(xFlags, s3, F_CF, F_CF);
    }

    if (rex.w) {
        SLLI_D(s1, s1, c);
    } else {
        SLLI_W(s1, s1, c);
    }

    IFX (X_SF) {
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit SHL8 instruction, from s1 , shift s2, store result in s1 using s3, s4 and s5 as scratch
void emit_shl8(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);

    IFX (X_PEND) {
        ST_B(s1, xEmu, offsetof(x64emu_t, op1));
        ST_B(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, d_shl8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    if (cpuext.lbt) {
        IFX (X_ALL) {
            X64_SLL_B(s1, s2);
        }
        SLL_D(s1, s1, s2);
        ANDI(s1, s1, 0xff);

        IFX (X_PEND) {
            ST_B(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }

    SLL_D(s1, s1, s2);

    // s2 is not 0 here and is 1..1f/3f
    CLEAR_FLAGS(s3);
    IFX (X_CF | X_OF) {
        SRLI_D(s5, s1, 8);
        ANDI(s5, s5, 1); // LSB == F_CF
        IFX (X_CF) {
            OR(xFlags, xFlags, s5);
        }
    }

    SLLI_D(s1, s1, 56);
    IFX (X_SF) {
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    SRLI_D(s1, s1, 56);

    IFX (X_PEND) {
        ST_B(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_OF) {
        // OF flag is affected only on 1-bit shifts
        ADDI_D(s3, s2, -1);
        BNEZ(s3, 4 + 4 * 4);
        SRLI_D(s3, s1, 7);
        XOR(s3, s3, s5);
        SLLI_D(s3, s3, F_OF);
        OR(xFlags, xFlags, s3);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit SHR8 instruction, from s1 , shift s2 (!0 and and'd already), store result in s1 using s3 and s4 as scratch
void emit_shr8(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);

    IFX (X_PEND) {
        ST_B(s2, xEmu, offsetof(x64emu_t, op2));
        ST_B(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, d_shr8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    if (cpuext.lbt) {
        IFX (X_ALL) {
            X64_SRL_B(s1, s2);
        }
        SRL_D(s1, s1, s2);
        ANDI(s1, s1, 0xff);

        IFX (X_PEND) {
            ST_B(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }

    CLEAR_FLAGS(s3);
    IFX (X_CF) {
        ADDI_D(s3, s2, -1);
        SRA_D(s3, s1, s3);
        ANDI(s3, s3, 1); // LSB == F_CF
        OR(xFlags, xFlags, s3);
    }
    IFX (X_OF) {
        // OF flag is affected only on 1-bit shifts
        // OF flag is set to the most-significant bit of the original operand
        ADDI_D(s3, xZR, 1);
        BNE(s2, s3, 4 + 3 * 4);
        SRLI_D(s3, s1, 7);
        SLLI_D(s3, s3, F_OF);
        OR(xFlags, xFlags, s3);
    }

    SRL_D(s1, s1, s2);
    ANDI(s1, s1, 0xff);

    // SF should be unset
    IFX (X_PEND) {
        ST_B(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit SAR8 instruction, from s1 , shift s2 (!0 and and'd already), store result in s1 using s3, s4 and s5 as scratch
void emit_sar8(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    int64_t j64;

    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);

    IFX (X_PEND) {
        ST_B(s2, xEmu, offsetof(x64emu_t, op2));
        ST_B(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, d_sar8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    if (cpuext.lbt) {
        IFX (X_ALL) {
            X64_SRA_B(s1, s2);
        }
        SRA_D(s1, s1, s2);
        ANDI(s1, s1, 0xff);

        IFX (X_PEND) {
            ST_B(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }

    CLEAR_FLAGS(s3);
    IFX (X_CF) {
        ADDI_D(s3, s2, -1);
        SRA_D(s3, s1, s3);
        ANDI(s3, s3, 1); // LSB == F_CF
        OR(xFlags, xFlags, s3);
    }
    // For the SAR instruction, the OF flag is cleared for all 1-bit shifts.
    // OF nop
    IFX (X_SF) {
        // SF is the same as the original operand
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }

    SRA_D(s1, s1, s2);
    ANDI(s1, s1, 0xff);

    IFX (X_PEND) {
        ST_B(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit SHR16 instruction, from s1 , shift s2 (!0 and and'd already), store result in s1 using s3 and s4 as scratch
void emit_shr16(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    int64_t j64;

    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    IFX (X_PEND) {
        ST_H(s2, xEmu, offsetof(x64emu_t, op2));
        ST_H(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, d_shr16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    if (cpuext.lbt) {
        IFX (X_ALL) {
            X64_SRL_H(s1, s2);
        }

        SRL_D(s1, s1, s2);
        BSTRPICK_D(s1, s1, 15, 0);

        IFX (X_PEND) {
            ST_H(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }

    CLEAR_FLAGS(s3);
    IFX (X_CF) {
        ADDI_D(s3, s2, -1);
        SRA_D(s3, s1, s3);
        ANDI(s3, s3, 1); // LSB == F_CF
        OR(xFlags, xFlags, s3);
    }
    IFX (X_OF) {
        // OF flag is affected only on 1-bit shifts
        // OF flag is set to the most-significant bit of the original operand
        ADDI_D(s3, xZR, 1);
        BNE(s2, s3, 4 + 3 * 4);
        SRLI_D(s3, s1, 15);
        SLLI_D(s3, s3, F_OF);
        OR(xFlags, xFlags, s3);
    }

    SRL_D(s1, s1, s2);
    BSTRPICK_D(s1, s1, 15, 0);

    // SF should be unset
    IFX (X_PEND) {
        ST_H(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit SHR16 instruction, from s1 , constant c, store result in s1 using s3, s4 and s5 as scratch
void emit_shr16c(dynarec_la64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4, int s5)
{
    if (!c) return;
    // c != 0

    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    IFX (X_PEND) {
        MOV64x(s3, c);
        ST_H(s3, xEmu, offsetof(x64emu_t, op2));
        ST_H(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, d_shr16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    if (cpuext.lbt) {
        IFX (X_PEND) {
        } else {
            MOV64x(s3, c);
        }
        IFX (X_ALL) {
            X64_SRL_H(s1, s3);
        }
        SRLI_D(s1, s1, c);
        IFX (X_PEND) {
            ST_H(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }

    CLEAR_FLAGS(s3);
    IFX (X_CF) {
        if (c > 1) {
            SRAI_D(s3, s1, c - 1);
            ANDI(s3, s3, 1); // LSB == F_CF
        } else {
            // no need to shift
            ANDI(s3, s1, 1); // LSB == F_CF
        }
        OR(xFlags, xFlags, s3);
    }
    IFX (X_OF) {
        // OF flag is affected only on 1-bit shifts
        // OF flag is set to the most-significant bit of the original operand
        if (c == 1) {
            SRLI_D(s3, s1, 15);
            SLLI_D(s3, s3, F_OF);
            OR(xFlags, xFlags, s3);
        }
    }

    SRLI_D(s1, s1, c);

    // SF should be unset
    IFX (X_PEND) {
        ST_H(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit SHR32 instruction, from s1 , shift s2 (!0 and and'd already), store result in s1 using s3 and s4 as scratch
void emit_shr32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4)
{
    int64_t j64;


    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    IFX (X_PEND) {
        SDxw(s2, xEmu, offsetof(x64emu_t, op2));
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, rex.w ? d_shr64 : d_shr32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    if (cpuext.lbt) {
        IFX (X_ALL) {
            if (rex.w)
                X64_SRL_D(s1, s2);
            else
                X64_SRL_W(s1, s2);
        }
        SRL_D(s1, s1, s2);
        if (!rex.w) { ZEROUP(s1); }
        IFX (X_PEND) {
            SDxw(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }

    CLEAR_FLAGS(s3);
    IFX (X_CF) {
        ADDI_D(s3, s2, -1);
        SRA_D(s3, s1, s3);
        ANDI(s3, s3, 1); // LSB == F_CF
        OR(xFlags, xFlags, s3);
    }
    IFX (X_OF) {
        // OF flag is affected only on 1-bit shifts
        // OF flag is set to the most-significant bit of the original operand
        ADDI_D(s3, xZR, 1);
        BNE(s2, s3, 4 + 3 * 4);
        SRLIxw(s3, s1, rex.w ? 63 : 31);
        SLLI_D(s3, s3, F_OF);
        OR(xFlags, xFlags, s3);
    }

    SRL_D(s1, s1, s2);

    IFX (X_SF) {
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit SHR32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_shr32c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4)
{
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    IFX (X_PEND) {
        if (c) {
            MOV64x(s3, c);
            SDxw(s3, xEmu, offsetof(x64emu_t, op2));
        } else
            SDxw(xZR, xEmu, offsetof(x64emu_t, op2));
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, rex.w ? d_shr64 : d_shr32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    if (!c) {
        IFX (X_PEND) {
            SDxw(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }

    if (cpuext.lbt) {
        IFX (X_PEND) {
        } else {
            MOV64x(s3, c);
        }
        IFX (X_ALL) {
            if (rex.w)
                X64_SRL_D(s1, s3);
            else
                X64_SRL_W(s1, s3);
        }

        SRLIxw(s1, s1, c);
        if (!rex.w) ZEROUP(s1);
        IFX (X_PEND) {
            SDxw(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }

    CLEAR_FLAGS(s3);
    IFX (X_CF) {
        if (c > 1) {
            SRAI_D(s3, s1, c - 1);
            ANDI(s3, s3, 1); // LSB == F_CF
        } else {
            // no need to shift
            ANDI(s3, s1, 1); // LSB == F_CF
        }
        OR(xFlags, xFlags, s3);
    }
    IFX (X_OF) {
        // OF flag is affected only on 1-bit shifts
        // OF flag is set to the most-significant bit of the original operand
        if (c == 1) {
            SRLIxw(s3, s1, rex.w ? 63 : 31);
            SLLI_D(s3, s3, F_OF);
            OR(xFlags, xFlags, s3);
        }
    }

    if (rex.w) {
        SRLI_D(s1, s1, c);
    } else {
        SRLI_W(s1, s1, c);
    }

    IFX (X_SF) {
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}


// emit SAR16 instruction, from s1, shift s2 (!0 and and'd already), store result in s1 using s3, s4 and s5 as scratch
void emit_sar16(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    int64_t j64;

    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    IFX (X_PEND) {
        ST_H(s2, xEmu, offsetof(x64emu_t, op2));
        ST_H(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, d_sar16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    if (cpuext.lbt) {
        IFX (X_ALL) {
            X64_SRA_H(s1, s2);
        }

        SRL_D(s1, s1, s2);
        BSTRPICK_D(s1, s1, 15, 0);

        IFX (X_PEND) {
            ST_H(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }

    CLEAR_FLAGS(s3);
    IFX (X_CF) {
        ADDI_D(s3, s2, -1);
        SRA_D(s3, s1, s3);
        ANDI(s3, s3, 1); // LSB == F_CF
        OR(xFlags, xFlags, s3);
    }
    // For the SAR instruction, the OF flag is cleared for all 1-bit shifts.
    // OF nop
    IFX (X_SF) {
        // SF is the same as the original operand
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }

    SRL_D(s1, s1, s2);
    BSTRPICK_D(s1, s1, 15, 0);

    IFX (X_PEND) {
        ST_H(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}


// emit SAR16 instruction, from s1 , constant c, store result in s1 using s3, s4 and s5 as scratch
void emit_sar16c(dynarec_la64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4, int s5)
{
    if (!c) return;
    // c != 0

    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    IFX (X_PEND) {
        MOV64x(s3, c);
        ST_H(s3, xEmu, offsetof(x64emu_t, op2));
        ST_H(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, d_sar16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    if (cpuext.lbt) {
        IFX (X_PEND) {
        } else {
            MOV64x(s3, c);
        }
        IFX (X_ALL) {
            X64_SRA_H(s1, s3);
        }
        SRLI_D(s1, s1, c);
        BSTRPICK_D(s1, s1, 15, 0);
        IFX (X_PEND) {
            ST_H(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }

    CLEAR_FLAGS(s3);
    IFX (X_CF) {
        if (c > 1) {
            SRAI_D(s3, s1, c - 1);
            ANDI(s3, s3, 1); // LSB == F_CF
        } else {
            // no need to shift
            ANDI(s3, s1, 1); // LSB == F_CF
        }
        OR(xFlags, xFlags, s3);
    }
    // For the SAR instruction, the OF flag is cleared for all 1-bit shifts.
    // OF nop
    IFX (X_SF) {
        // SF is the same as the original operand
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }

    SRLI_D(s1, s1, c);
    BSTRPICK_D(s1, s1, 15, 0);

    IFX (X_PEND) {
        ST_H(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit SAR32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_sar32c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4)
{
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    IFX (X_PEND) {
        if (c) {
            MOV64x(s3, c);
            SDxw(s3, xEmu, offsetof(x64emu_t, op2));
        } else
            SDxw(xZR, xEmu, offsetof(x64emu_t, op2));
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, rex.w ? d_sar64 : d_sar32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    if (!c) {
        IFX (X_PEND) {
            SDxw(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }

    if (cpuext.lbt) {
        IFX (X_PEND) {
        } else {
            MOV64x(s3, c);
        }
        IFX (X_ALL) {
            if (rex.w)
                X64_SRA_D(s1, s3);
            else
                X64_SRA_W(s1, s3);
        }

        SRAIxw(s1, s1, c);
        if (!rex.w) ZEROUP(s1);
        IFX (X_PEND) {
            SDxw(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }

    CLEAR_FLAGS(s3);
    IFX (X_CF) {
        if (c > 1) {
            SRAI_D(s3, s1, c - 1);
            ANDI(s3, s3, 1); // LSB == F_CF
        } else {
            // no need to shift
            ANDI(s3, s1, 1); // LSB == F_CF
        }
        OR(xFlags, xFlags, s3);
    }
    // For the SAR instruction, the OF flag is cleared for all 1-bit shifts.
    // OF nop

    if (rex.w) {
        SRAI_D(s1, s1, c);
    } else {
        SRAI_W(s1, s1, c);
    }

    // SRAI_W sign-extends, so test sign bit before clearing upper bits
    IFX (X_SF) {
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}


// emit ROR32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_ror32c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4)
{
    if (!c) return;

    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    IFX (X_PEND) {
        MOV32w(s3, c);
        SDxw(s3, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, rex.w ? d_ror64 : d_ror32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    if (!c) {
        IFX (X_PEND) {
            SDxw(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }

    IFXA ((X_CF | X_OF), cpuext.lbt) {
        if (rex.w)
            X64_ROTRI_D(s1, c);
        else
            X64_ROTRI_W(s1, c);
    }

    IFXA (X_CF | X_OF, !cpuext.lbt) {
        MOV64x(s4, ((1UL << F_CF) | (1UL << F_OF)));
        ANDN(xFlags, xFlags, s4);
    }

    IFXA (X_OF, (!cpuext.lbt || BOX64DRENV(dynarec_safeflags) > 1)) {
        SRLIxw(s3, s1, rex.w ? 63 : 31);
        XOR(s3, s3, s1);
        if (cpuext.lbt) {
            SLLI_D(s3, s3, F_OF);
            X64_SET_EFLAGS(s3, X_OF);
        } else
            BSTRINS_D(xFlags, s3, F_OF, F_OF);
    }

    ROTRIxw(s1, s1, c);
    if (!rex.w) ZEROUP(s1);

    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }

    if (cpuext.lbt) return;

    IFX (X_CF) {
        SRLIxw(s3, s1, rex.w ? 63 : 31);
        OR(xFlags, xFlags, s3);
    }
}

// emit ROL8 instruction, from s1, s2, store result in s1 using s3 s4 and s5 as scratch
void emit_rol8(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    int64_t j64;
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    BEQ_NEXT(s2, xZR);
    ANDI(s5, s2, 0b111); // TEMPCOUNT = (COUNT & COUNTMASK) MOD 8
    IFX (X_PEND) {
        ST_B(s2, xEmu, offsetof(x64emu_t, op2));
        ST_B(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, d_rol8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFXA ((X_CF | X_OF), cpuext.lbt) {
        X64_ROTL_B(s1, s5);
    }
    if (cpuext.lbt) {
        MOV64x(s3, 8);
        SUB_D(s3, s3, s5);
        ROTR_B(s1, s1, s3);
    } else {
        SLL_D(s3, s1, s5);
        NEG_D(s4, s5);
        ADDI_D(s4, s4, 8);
        SRL_D(s1, s1, s4);
        OR(s1, s3, s1);
    }

    IFX (X_PEND) {
        ST_B(s1, xEmu, offsetof(x64emu_t, res));
    }

    if (cpuext.lbt) return;

    IFX (X_CF | X_OF) {
        MOV64x(s4, ((1UL << F_CF) | (1UL << F_OF)));
        ANDN(xFlags, xFlags, s4);
    }
    IFX (X_CF | X_OF) {
        ANDI(s4, s1, 1); // LSB == F_CF
        IFX (X_CF) OR(xFlags, xFlags, s4);
    }
    IFX (X_OF) {
        // the OF flag is set to the exclusive OR of the CF bit (after the rotate) and the most-significant bit of the result.
        ADDI_D(s3, s2, -1);
        BNEZ_MARK(s3);
        SRLI_W(s3, s1, 7);
        XOR(s3, s3, s4); // s3: MSB, s4: CF bit
        SLLI_D(s3, s3, F_OF);
        OR(xFlags, xFlags, s3);
        MARK;
    }
}

// emit ROL8 instruction, from s1, c, store result in s1 using s3 s4 and s5 as scratch
void emit_rol8c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4, int s5)
{
    int64_t j64;
    if (!c) return;
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    int count = c % 8;
    IFX (X_PEND) {
        MOV32w(s3, c);
        ST_B(s3, xEmu, offsetof(x64emu_t, op2));
        ST_B(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, d_rol8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFXA ((X_CF | X_OF), cpuext.lbt) {
        X64_ROTLI_B(s1, c);
    }
    if (cpuext.lbt) {
        ROTRI_B(s1, s1, 8 - count);
    } else {
        SLLI_D(s3, s1, count);
        ADDI_D(s4, xZR, -count);
        ADDI_D(s4, s4, 8);
        SRL_D(s1, s1, s4);
        OR(s1, s3, s1);
    }


    IFX (X_PEND) {
        ST_B(s1, xEmu, offsetof(x64emu_t, res));
    }

    if (cpuext.lbt) return;

    IFX (X_CF | X_OF) {
        MOV64x(s4, ((1UL << F_CF) | (1UL << F_OF)));
        ANDN(xFlags, xFlags, s4);
    }
    IFX (X_CF | X_OF) {
        ANDI(s4, s1, 1); // LSB == F_CF
        IFX (X_CF) OR(xFlags, xFlags, s4);
    }
    IFX (X_OF) {
        // the OF flag is set to the exclusive OR of the CF bit (after the rotate) and the most-significant bit of the result.
        if (c == 1) {
            SRLI_W(s3, s1, 7);
            XOR(s3, s3, s4); // s3: MSB, s4: CF bit
            SLLI_D(s3, s3, F_OF);
            OR(xFlags, xFlags, s3);
        }
    }
}

// emit ROL16 instruction, from s1, s2, store result in s1 using s3 s4 and s5 as scratch
void emit_rol16(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    int64_t j64;
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    BEQ_NEXT(s2, xZR);
    ANDI(s5, s2, 0b1111); // TEMPCOUNT = (COUNT & COUNTMASK) MOD 16
    IFX (X_PEND) {
        ST_H(s2, xEmu, offsetof(x64emu_t, op2));
        ST_H(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, d_rol16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFXA ((X_CF | X_OF), cpuext.lbt) {
        X64_ROTL_H(s1, s5);
    }
    if (cpuext.lbt) {
        MOV64x(s3, 16);
        SUB_D(s3, s3, s5);
        ROTR_H(s1, s1, s3);
    } else {
        SLL_D(s3, s1, s5);
        NEG_D(s4, s5);
        ADDI_D(s4, s4, 16);
        SRL_D(s1, s1, s4);
        OR(s1, s3, s1);
    }
    IFX (X_PEND) {
        ST_H(s1, xEmu, offsetof(x64emu_t, res));
    }

    if (cpuext.lbt) return;

    IFX (X_CF | X_OF) {
        MOV64x(s4, ((1UL << F_CF) | (1UL << F_OF)));
        ANDN(xFlags, xFlags, s4);
    }
    IFX (X_CF | X_OF) {
        ANDI(s4, s1, 1); // LSB == F_CF
        IFX (X_CF) OR(xFlags, xFlags, s4);
    }
    IFX (X_OF) {
        // the OF flag is set to the exclusive OR of the CF bit (after the rotate) and the most-significant bit of the result.
        ADDI_D(s3, s2, -1);
        BNEZ_MARK(s3);
        SRLI_W(s3, s1, 15);
        XOR(s3, s3, s4); // s3: MSB, s4: CF bit
        SLLI_D(s3, s3, F_OF);
        OR(xFlags, xFlags, s3);
        MARK;
    }
}


// emit ROL16 instruction, from s1, c, store result in s1 using s3 s4 and s5 as scratch
void emit_rol16c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4, int s5)
{
    int64_t j64;
    if (!c) return;
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    int count = c % 16;
    IFX (X_PEND) {
        MOV32w(s3, c);
        ST_H(s3, xEmu, offsetof(x64emu_t, op2));
        ST_H(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, d_rol16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFXA ((X_CF | X_OF), cpuext.lbt) {
        X64_ROTLI_H(s1, c);
    }
    if (cpuext.lbt) {
        ROTRI_H(s1, s1, 16 - count);
    } else {
        SLLI_W(s3, s1, count);
        ADDI_D(s4, xZR, -count);
        ADDI_D(s4, s4, 16);
        SRL_W(s1, s1, s4);
        OR(s1, s3, s1);
    }

    IFX (X_PEND) {
        ST_H(s1, xEmu, offsetof(x64emu_t, res));
    }

    if (cpuext.lbt) return;

    IFX (X_CF | X_OF) {
        MOV64x(s4, ((1UL << F_CF) | (1UL << F_OF)));
        ANDN(xFlags, xFlags, s4);
    }
    IFX (X_CF | X_OF) {
        ANDI(s4, s1, 1); // LSB == F_CF
        IFX (X_CF) OR(xFlags, xFlags, s4);
    }
    IFX (X_OF) {
        // the OF flag is set to the exclusive OR of the CF bit (after the rotate) and the most-significant bit of the result.
        if (c == 1) {
            SRLI_W(s3, s1, 15);
            XOR(s3, s3, s4); // s3: MSB, s4: CF bit
            SLLI_D(s3, s3, F_OF);
            OR(xFlags, xFlags, s3);
        }
    }
}

// emit ROL32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_rol32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4)
{
    int64_t j64;
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    if (!rex.w) ZEROUP(s1);
    BEQ_NEXT(s2, xZR);
    IFX (X_PEND) {
        SDxw(s2, xEmu, offsetof(x64emu_t, op2));
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, rex.w ? d_rol64 : d_rol32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFXA (X_CF | X_OF, !cpuext.lbt) {
        MOV64x(s4, ((1UL << F_CF) | (1UL << F_OF)));
        ANDN(xFlags, xFlags, s4);
    }

    IFXA ((X_CF | X_OF), cpuext.lbt) {
        if (rex.w)
            X64_ROTL_D(s1, s2);
        else
            X64_ROTL_W(s1, s2);
    }


    IFXA (X_OF, (!cpuext.lbt || BOX64DRENV(dynarec_safeflags) > 1)) {
        SRLIxw(s3, s1, rex.w ? 62 : 30);
        SRLI_D(s4, s3, 1);
        XOR(s3, s3, s4);
        if (cpuext.lbt) {
            SLLI_D(s3, s3, F_OF);
            X64_SET_EFLAGS(s3, X_OF);
        } else
            BSTRINS_D(xFlags, s3, F_OF, F_OF);
    }

    MOV64x(s3, rex.w ? 64 : 32);
    SUB_D(s3, s3, s2);
    if (rex.w) {
        ROTR_D(s1, s1, s3);
    } else {
        ROTR_W(s1, s1, s3);
    }

    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }

    if (cpuext.lbt) return;

    IFX (X_CF) {
        ANDI(s4, s1, 1); // LSB == F_CF
        OR(xFlags, xFlags, s4);
    }
}

// emit ROR32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_ror32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4)
{
    int64_t j64;

    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    if (!rex.w) {
        ZEROUP(s1);
    }
    BEQ_NEXT(s2, xZR);
    IFX (X_PEND) {
        SDxw(s2, xEmu, offsetof(x64emu_t, op2));
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, rex.w ? d_ror64 : d_ror32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFXA ((X_CF | X_OF), cpuext.lbt) {
        if (rex.w)
            X64_ROTR_D(s1, s2);
        else
            X64_ROTR_W(s1, s2);
    }

    IFXA (X_CF | X_OF, !cpuext.lbt) {
        MOV64x(s4, ((1UL << F_CF) | (1UL << F_OF)));
        ANDN(xFlags, xFlags, s4);
    }

    IFXA (X_OF, (!cpuext.lbt || BOX64DRENV(dynarec_safeflags) > 1)) {
        SRLIxw(s3, s1, rex.w ? 63 : 31);
        XOR(s3, s3, s1);
        if (cpuext.lbt) {
            SLLI_D(s3, s3, F_OF);
            X64_SET_EFLAGS(s3, X_OF);
        } else
            BSTRINS_D(xFlags, s3, F_OF, F_OF);
    }

    if (rex.w) {
        ROTR_D(s1, s1, s2);
    } else {
        ROTR_W(s1, s1, s2);
    }

    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }

    if (cpuext.lbt) return;

    IFX (X_CF) {
        SRLIxw(s3, s1, rex.w ? 63 : 31);
        OR(xFlags, xFlags, s3);
    }
}

// emit ROL32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_rol32c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4)
{
    if (!c) return;

    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    IFX (X_PEND) {
        MOV32w(s3, c);
        SDxw(s3, xEmu, offsetof(x64emu_t, op2));
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, rex.w ? d_rol64 : d_rol32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    if (!c) {
        IFX (X_PEND) {
            SDxw(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }

    IFXA ((X_CF | X_OF), cpuext.lbt) {
        if (rex.w)
            X64_ROTLI_D(s1, c);
        else
            X64_ROTLI_W(s1, c);
    }

    ROTRIxw(s1, s1, (rex.w ? 64 : 32) - c);

    if (!rex.w) ZEROUP(s1);

    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }

    if (cpuext.lbt) return;

    IFX (X_CF | X_OF) {
        MOV64x(s3, ((1UL << F_CF) | (1UL << F_OF)));
        ANDN(xFlags, xFlags, s3);
    }
    IFX (X_CF | X_OF) {
        ANDI(s4, s1, 1 << F_CF);
        IFX (X_CF) OR(xFlags, xFlags, s4);
    }
    IFX (X_OF) {
        // the OF flag is set to the exclusive OR of the CF bit (after the rotate) and the most-significant bit of the result.
        if (c == 1) {
            SRLIxw(s3, s1, rex.w ? 63 : 31);
            XOR(s3, s3, s4);
            SLLI_D(s3, s3, F_OF);
            OR(xFlags, xFlags, s3);
        }
    }
}


void emit_shld32c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, uint32_t c, int s3, int s4)
{
    c &= (rex.w ? 0x3f : 0x1f);

    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    IFX (X_PEND) {
        if (c) {
            MOV64x(s3, c);
            SDxw(s3, xEmu, offsetof(x64emu_t, op2));
        } else
            SDxw(xZR, xEmu, offsetof(x64emu_t, op2));
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, rex.w ? d_shld64 : d_shld32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    if (!c) {
        IFX (X_PEND) {
            SDxw(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }

    CLEAR_FLAGS(s3);
    IFX (X_CF) {
        if (c > 0) {
            SRLI_D(s3, s1, (rex.w ? 64 : 32) - c);
            ANDI(s4, s3, 1); // F_CF
            OR(xFlags, xFlags, s4);
        }
    }
    IFX (X_OF) {
        // Store sign for later use.
        if (c == 1) SRLIxw(s4, s1, rex.w ? 63 : 31);
    }

    SLLIxw(s3, s1, c);
    SRLIxw(s1, s2, (rex.w ? 64 : 32) - c);
    OR(s1, s1, s3);

    if (!rex.w) {
        ZEROUP(s1);
    }
    IFX (X_SF) {
        SRLIxw(s3, s1, rex.w ? 63 : 31);
        BEQZ(s3, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_OF) {
        // the OF flag is set if a sign change occurred
        if (c == 1) {
            SRLIxw(s3, s1, rex.w ? 63 : 31);
            XOR(s3, s3, s4);
            SLLI_D(s3, s3, F_OF);
            ORI(xFlags, xFlags, s3);
        }
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }

    IFXA (X_ALL, cpuext.lbt) {
        SPILL_EFLAGS();
    }
}


// emit SHRD32 instruction, from s1, fill s2 , constant c, store result in s1 using s3 and s4 as scratch
void emit_shrd32c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, uint32_t c, int s3, int s4)
{
    c &= (rex.w ? 0x3f : 0x1f);

    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    IFX (X_PEND) {
        if (c) {
            MOV64x(s3, c);
            SDxw(s3, xEmu, offsetof(x64emu_t, op2));
        } else
            SDxw(xZR, xEmu, offsetof(x64emu_t, op2));
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, rex.w ? d_shrd64 : d_shrd32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    if (!c) {
        IFX (X_PEND) {
            SDxw(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }


    CLEAR_FLAGS(s3);
    IFX (X_CF) {
        if (c > 1) {
            SRAI_D(s3, s1, c - 1);
            ANDI(s3, s3, 1); // LSB == F_CF
        } else {
            ANDI(s3, s1, 1); // LSB == F_CF
        }
        OR(xFlags, xFlags, s3);
    }
    IFX (X_OF) {
        // Store sign for later use.
        if (c == 1) SRLIxw(s4, s1, rex.w ? 63 : 31);
    }

    SRLIxw(s3, s1, c);
    SLLIxw(s1, s2, (rex.w ? 64 : 32) - c);
    OR(s1, s1, s3);

    if (!rex.w) {
        ZEROUP(s1);
    }
    IFX (X_SF) {
        SRLIxw(s3, s1, rex.w ? 63 : 31);
        BEQZ(s3, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_OF) {
        // the OF flag is set if a sign change occurred
        if (c == 1) {
            SRLI_D(s3, s1, rex.w ? 63 : 31);
            XOR(s3, s3, s4);
            SLLI_D(s3, s3, F_OF);
            OR(xFlags, xFlags, s3);
        }
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }

    IFXA (X_ALL, cpuext.lbt) {
        SPILL_EFLAGS();
    }
}

void emit_shrd32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s5, int s3, int s4, int s6)
{
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    int64_t j64;
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SDxw(s5, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, rex.w ? d_shrd64 : d_shrd32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    CLEAR_FLAGS(s3);
    IFX (X_CF) {
        ADDI_D(s3, s5, -1);
        SRA_D(s3, s1, s3);
        ANDI(s3, s3, 1); // LSB == F_CF
        OR(xFlags, xFlags, s3);
    }
    IFX (X_OF) {
        // Store current sign for later use.
        SRLIxw(s6, s1, rex.w ? 63 : 31);
    }
    ADDI_D(s4, xZR, (rex.w ? 64 : 32));
    SUB_D(s4, s4, s5);
    SRLxw(s3, s1, s5);
    SLLxw(s4, s2, s4);
    OR(s1, s4, s3);

    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    IFX (X_OF) {
        ADDI_D(s5, s5, -1);
        BNEZ_MARK(s5);
        SRLIxw(s3, s1, rex.w ? 63 : 31);
        XOR(s3, s3, s6);
        SLLI_D(s3, s3, F_OF);
        OR(xFlags, xFlags, s3);
        MARK;
    }
    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_SF) {
        SRLIxw(s3, s1, rex.w ? 63 : 31);
        BEQZ(s3, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    IFXA (X_ALL, cpuext.lbt) SPILL_EFLAGS();
}

void emit_shld32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s5, int s3, int s4, int s6)
{
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR);
    int64_t j64;
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SDxw(s5, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, rex.w ? d_shld64 : d_shld32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    CLEAR_FLAGS(s3);
    MOV32w(s3, (rex.w ? 64 : 32));
    SUB_D(s3, s3, s5);

    IFX (X_CF) {
        SRL_D(s4, s1, s3);
        ANDI(s4, s4, 1); // LSB == F_CF
        OR(xFlags, xFlags, s4);
    }
    IFX (X_OF) {
        // Store current sign for later use.
        SRLIxw(s6, s1, rex.w ? 63 : 31);
    }
    SLLxw(s4, s1, s5);
    SRLxw(s3, s2, s3);
    OR(s1, s3, s4);

    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    if (!rex.w) {
        ZEROUP(s1);
    }

    IFX (X_OF) {
        ADDI_D(s5, s5, -1);
        BNEZ_MARK(s5);
        SRLIxw(s3, s1, rex.w ? 63 : 31);
        XOR(s3, s3, s6);
        SLLI_D(s3, s3, F_OF);
        OR(xFlags, xFlags, s3);
        MARK;
    }
    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_SF) {
        SRLIxw(s3, s1, rex.w ? 63 : 31);
        BEQZ(s3, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    IFXA (X_ALL, cpuext.lbt) SPILL_EFLAGS();
}
