#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "la64_emitter.h"
#include "la64_mapping.h"
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

        IFXA ((X_AF | X_OF), BOX64DRENV(dynarec_safeflags)) {
            SRLI_D(s3, s1, 14 - F_OF);
            SRLI_D(s5, s3, 1);
            XOR(s3, s3, s5);
            BSTRINS_D(s3, xZR, F_AF, F_AF);
            X64_SET_EFLAGS(s3, (X_OF | X_AF));
        }

        SLL_D(s1, s1, s2);
        BSTRPICK_D(s1, s1, 15, 0);
        IFX (X_PEND) {
            ST_H(s1, xEmu, offsetof(x64emu_t, res));
        }

        if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
        return;
    }

    CLEAR_FLAGS(s3);
    IFX (X_OF) {
        SRLI_D(s3, s1, 14);
        SRLI_D(s5, s3, 1);
        XOR(s3, s3, s5);
        BSTRINS_D(xFlags, s3, F_OF, F_OF);
    }
    SLL_D(s1, s1, s2);

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
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SHL16 instruction, from s1 , constant c, store result in s1 using s3, s4 and s5 as scratch
void emit_shl16c(dynarec_la64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4, int s5)
{
    if (!c) return;
    // c != 0

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

        IFXA ((X_AF | X_OF), BOX64DRENV(dynarec_safeflags)) {
            SRLI_D(s3, s1, 14 - F_OF);
            SRLI_D(s5, s3, 1);
            XOR(s3, s3, s5);
            BSTRINS_D(s3, xZR, F_AF, F_AF);
            X64_SET_EFLAGS(s3, (X_OF | X_AF));
        }

        SLLI_D(s1, s1, c);

        IFX (X_PEND) {
            ST_H(s1, xEmu, offsetof(x64emu_t, res));
        }
        if (dyn->insts[ninst].nat_flags_fusion) {
            BSTRPICK_D(s1, s1, 15, 0);
            NAT_FLAGS_OPS(s1, xZR, s3, xZR);
        }
        return;
    }

    CLEAR_FLAGS(s3);

    IFX (X_OF) {
        SRLI_D(s3, s1, 14);
        SRLI_D(s5, s3, 1);
        XOR(s3, s3, s5);
        BSTRINS_D(xFlags, s3, F_OF, F_OF);
    }

    if (c < 16) {
        IFX (X_CF) {
            SRLI_D(s3, s1, 16 - c);
            BSTRINS_D(xFlags, s3, F_CF, F_CF);
        }

        SLLI_D(s1, s1, c + 48);
        IFX (X_SF) {
            SLTI(s3, s1, 0);
            BSTRINS_D(xFlags, s3, F_SF, F_SF);
        }
        SRLI_D(s1, s1, 48);

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
        if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
    } else {
        IFXA (X_CF, c == 16) {
            ANDI(s3, s1, 1);
            OR(xFlags, xFlags, s3); // F_CF == 0
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
        if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(xZR, xZR, xZR, xZR);
    }
}

// emit SHL32 instruction, from s1 , shift s2, store result in s1 using s3, s4 and s5 as scratch
void emit_shl32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
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
        IFXA ((X_AF | X_OF), BOX64DRENV(dynarec_safeflags)) {
            SRLIxw(s3, s1, (rex.w ? 62 : 30) - F_OF);
            SRLI_D(s5, s3, 1);
            XOR(s3, s3, s5);
            BSTRINS_D(s3, xZR, F_AF, F_AF);
            X64_SET_EFLAGS(s3, (X_OF | X_AF));
        }
        SLL_D(s1, s1, s2);
        if (!rex.w) { ZEROUP(s1); }
        IFX (X_PEND) {
            SDxw(s1, xEmu, offsetof(x64emu_t, res));
        }
        if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
        return;
    }

    CLEAR_FLAGS(s3);
    IFX (X_OF) {
        SRLIxw(s3, s1, rex.w ? 62 : 30);
        SRLI_D(s5, s3, 1);
        XOR(s3, s3, s5);
        BSTRINS_D(xFlags, s3, F_OF, F_OF);
    }
    IFX (X_CF) {
        ADDI_D(s5, s2, rex.w ? -64 : -32);
        NEG_D(s5, s5);
        SRL_D(s3, s1, s5);
        BSTRINS_D(xFlags, s3, F_CF, F_CF);
    }
    if (rex.w) {
        SLL_D(s1, s1, s2);
    } else {
        SLL_W(s1, s1, s2);
    }
    IFX (X_SF) {
        SLTI(s3, s1, 0);
        BSTRINS_D(xFlags, s3, F_SF, F_SF);
    }
    if (!rex.w) ZEROUP(s1);
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
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SHL32 instruction, from s1 , constant c, store result in s1 using s3, s4 and s5 as scratch
void emit_shl32c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4, int s5)
{
    if (!c) return;

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
        IFXA ((X_AF | X_OF), BOX64DRENV(dynarec_safeflags)) {
            SRLIxw(s3, s1, (rex.w ? 62 : 30) - F_OF);
            SRLI_D(s5, s3, 1);
            XOR(s3, s3, s5);
            BSTRINS_D(s3, xZR, F_AF, F_AF);
            X64_SET_EFLAGS(s3, (X_OF | X_AF));
        }
        SLLIxw(s1, s1, c);
        if (!rex.w) ZEROUP(s1);
        IFX (X_PEND) {
            SDxw(s1, xEmu, offsetof(x64emu_t, res));
        }
        if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
        return;
    }

    CLEAR_FLAGS(s3);
    IFX (X_OF) {
        SRLIxw(s3, s1, rex.w ? 62 : 30);
        SRLI_D(s5, s3, 1);
        XOR(s3, s3, s5);
        BSTRINS_D(xFlags, s3, F_OF, F_OF);
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
        SLTI(s3, s1, 0);
        BSTRINS_D(xFlags, s3, F_SF, F_SF);
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
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SHL8 instruction, from s1 , shift s2, store result in s1 using s3, s4 and s5 as scratch
void emit_shl8(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    IFX (X_PEND) {
        ST_B(s1, xEmu, offsetof(x64emu_t, op1));
        ST_B(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, d_shl8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    if (cpuext.lbt) {
        IFX (X_ALL) X64_SLL_B(s1, s2);
        IFXA ((X_OF | X_AF), BOX64DRENV(dynarec_safeflags)) {
            SLLI_D(s3, s1, F_OF - 6);
            SRLI_D(s4, s3, 1);
            XOR(s3, s3, s4);
            BSTRINS_D(s3, xZR, F_AF, F_AF);
            X64_SET_EFLAGS(s3, (X_OF | X_AF));
        }
        SLL_D(s1, s1, s2);
        ANDI(s1, s1, 0xff);

        IFX (X_PEND) {
            ST_B(s1, xEmu, offsetof(x64emu_t, res));
        }
        if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
        return;
    }

    CLEAR_FLAGS(s3);

    IFX (X_OF) {
        SRLI_D(s3, s1, 6);
        SRLI_D(s4, s3, 1);
        XOR(s3, s3, s4);
        BSTRINS_D(xFlags, s3, F_OF, F_OF);
    }

    SLL_D(s1, s1, s2);

    IFX (X_CF) {
        SRLI_D(s5, s1, 8);
        BSTRINS_D(xFlags, s5, F_CF, F_CF);
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
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SHR8 instruction, from s1 , shift s2 (!0 and and'd already), store result in s1 using s3 and s4 as scratch
void emit_shr8(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{

    IFX (X_PEND) {
        ST_B(s2, xEmu, offsetof(x64emu_t, op2));
        ST_B(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, d_shr8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    if (cpuext.lbt) {
        IFX (X_ALL) X64_SRL_B(s1, s2);
        IFXA ((X_AF | X_OF), BOX64DRENV(dynarec_safeflags)) {
            SLLI_D(s3, s1, F_OF - 7);
            BSTRINS_D(s3, xZR, F_AF, F_AF);
            X64_SET_EFLAGS(s3, (X_OF | X_AF));
        }
        SRL_D(s1, s1, s2);
        ANDI(s1, s1, 0xff);

        IFX (X_PEND) {
            ST_B(s1, xEmu, offsetof(x64emu_t, res));
        }
        if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
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
        // OF flag is set to the most-significant bit of the original operand
        SRLI_D(s3, s1, 7);
        BSTRINS_D(xFlags, s3, F_OF, F_OF);
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
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SAR8 instruction, from s1 , shift s2 (!0 and and'd already), store result in s1 using s3, s4 and s5 as scratch
void emit_sar8(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    int64_t j64;
    IFX (X_PEND) {
        ST_B(s2, xEmu, offsetof(x64emu_t, op2));
        ST_B(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, d_sar8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    if (cpuext.lbt) {
        IFX (X_ALL) X64_SRA_B(s1, s2);
        IFXA ((X_AF | X_OF), BOX64DRENV(dynarec_safeflags)) X64_SET_EFLAGS(xZR, (X_AF | X_OF));
        SRA_D(s1, s1, s2);
        ANDI(s1, s1, 0xff);

        IFX (X_PEND) {
            ST_B(s1, xEmu, offsetof(x64emu_t, res));
        }
        if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
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
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SHL8 instruction, from s1 , constant c, store result in s1 using s3, s4 as scratch
void emit_shl8c(dynarec_la64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4)
{
    if (!c) return;
    IFX (X_PEND) {
        MOV64x(s3, c);
        ST_B(s3, xEmu, offsetof(x64emu_t, op2));
        ST_B(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, d_shl8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    CLEAR_FLAGS(s3);
    IFX (X_OF) {
        SRLI_D(s3, s1, 6);
        SRLI_D(s4, s3, 1);
        XOR(s3, s3, s4);
        BSTRINS_D(xFlags, s3, F_OF, F_OF);
    }
    SLLI_D(s1, s1, c);
    IFX (X_CF) {
        BSTRPICK_D(s3, s1, 8, 8);
        BSTRINS_D(xFlags, s3, F_CF, F_CF);
    }
    IFX (X_PEND) {
        ST_B(s1, xEmu, offsetof(x64emu_t, res));
    }
    ANDI(s1, s1, 0xff);
    IFX (X_SF) {
        SRLI_D(s3, s1, 7);
        BSTRINS_D(xFlags, s3, F_SF, F_SF);
    }
    IFX (X_ZF) {
        SEQZ(s3, s1);
        BSTRINS_D(xFlags, s3, F_ZF, F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    IFXA (X_ALL, cpuext.lbt) SPILL_EFLAGS();
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SHR8 instruction, from s1 , constant c, store result in s1 using s3, s4 as scratch
void emit_shr8c(dynarec_la64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4)
{
    if (!c) return;
    IFX (X_PEND) {
        MOV64x(s3, c);
        ST_B(s3, xEmu, offsetof(x64emu_t, op2));
        ST_B(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, d_shr8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    CLEAR_FLAGS(s3);
    IFX (X_CF) {
        if (c == 1) {
            BSTRINS_D(xFlags, s1, F_CF, F_CF);
        } else {
            BSTRPICK_D(s3, s1, c - 1, c - 1);
            BSTRINS_D(xFlags, s3, F_CF, F_CF);
        }
    }
    IFX (X_OF) {
        BSTRPICK_D(s3, s1, 7, 7);
        BSTRINS_D(xFlags, s3, F_OF, F_OF);
    }

    SRLI_D(s1, s1, c);
    ANDI(s1, s1, 0xff);

    IFX (X_PEND) {
        ST_B(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        SEQZ(s4, s1);
        BSTRINS_D(xFlags, s4, F_ZF, F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    IFXA (X_ALL, cpuext.lbt) SPILL_EFLAGS();
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SAR8 instruction, from s1 , constant c, store result in s1 using s3, s4 as scratch
void emit_sar8c(dynarec_la64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4)
{
    if (!c) return;
    IFX (X_PEND) {
        MOV64x(s3, c);
        ST_B(s3, xEmu, offsetof(x64emu_t, op2));
        ST_B(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, d_sar8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    CLEAR_FLAGS(s3);
    IFXA (X_CF, c < 8) {
        BSTRPICK_D(s3, s1, c - 1, c - 1);
        BSTRINS_D(xFlags, s3, F_CF, F_CF);
    }

    SRAI_D(s1, s1, c);
    ANDI(s1, s1, 0xff);
    IFX (X_PEND) {
        ST_B(s1, xEmu, offsetof(x64emu_t, res));
    }
    if (c < 8) {
        IFX (X_ZF) {
            SEQZ(s3, s1);
            BSTRINS_D(xFlags, s3, F_ZF, F_ZF);
        }
        IFX (X_SF) {
            SRLI_D(s3, s1, 7);
            BSTRINS_D(xFlags, s3, F_SF, F_SF);
        }
        IFX (X_PF) {
            emit_pf(dyn, ninst, s1, s3, s4);
        }
    } else {
        IFX (X_CF | X_ZF | X_SF | X_PF) {
            SRLI_D(s3, s1, 7);
            IFX (X_CF) BSTRINS_D(xFlags, s3, F_CF, F_CF);
            IFX (X_SF) BSTRINS_D(xFlags, s3, F_SF, F_SF);
            IFX (X_ZF) {
                SEQZ(s4, s1);
                BSTRINS_D(xFlags, s4, F_ZF, F_ZF);
            }
            IFX (X_PF) ORI(xFlags, xFlags, 1 << F_PF);
        }
    }

    IFXA (X_ALL, cpuext.lbt) SPILL_EFLAGS();
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SHR16 instruction, from s1 , shift s2 (!0 and and'd already), store result in s1 using s3 and s4 as scratch
void emit_shr16(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    int64_t j64;
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

        IFXA ((X_AF | X_OF), BOX64DRENV(dynarec_safeflags)) {
            SRLI_D(s3, s1, 15 - F_OF);
            BSTRINS_D(s3, xZR, F_AF, F_AF);
            X64_SET_EFLAGS(s3, (X_OF | X_AF));
        }

        SRL_D(s1, s1, s2);
        BSTRPICK_D(s1, s1, 15, 0);

        IFX (X_PEND) {
            ST_H(s1, xEmu, offsetof(x64emu_t, res));
        }
        if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
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
        // OF flag is set to the most-significant bit of the original operand
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
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SHR16 instruction, from s1 , constant c, store result in s1 using s3, s4 and s5 as scratch
void emit_shr16c(dynarec_la64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4, int s5)
{
    if (!c) return;
    IFX (X_PEND) {
        MOV64x(s3, c);
        ST_H(s3, xEmu, offsetof(x64emu_t, op2));
        ST_H(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, d_shr16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    if (cpuext.lbt) {
        IFX (X_ALL) {
            MOV64x(s3, c);
            X64_SRL_H(s1, s3);
        }

        IFXA ((X_AF | X_OF), BOX64DRENV(dynarec_safeflags)) {
            SRLI_D(s3, s1, 15 - F_OF);
            BSTRINS_D(s3, xZR, F_AF, F_AF);
            X64_SET_EFLAGS(s3, (X_OF | X_AF));
        }

        SRLI_D(s1, s1, c);
        IFX (X_PEND) {
            ST_H(s1, xEmu, offsetof(x64emu_t, res));
        }
        if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
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
        // OF flag is set to the most-significant bit of the original operand
        SRLI_D(s3, s1, 15);
        SLLI_D(s3, s3, F_OF);
        OR(xFlags, xFlags, s3);
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
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SHR32 instruction, from s1 , shift s2 (!0 and and'd already), store result in s1 using s3 and s4 as scratch
void emit_shr32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4)
{
    int64_t j64;
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
        IFXA ((X_AF | X_OF), BOX64DRENV(dynarec_safeflags)) {
            SRLIxw(s3, s1, (rex.w ? 63 : 31) - F_OF);
            BSTRINS_D(s3, xZR, F_AF, F_AF);
            X64_SET_EFLAGS(s3, X_OF | X_AF);
        }
        SRL_D(s1, s1, s2);
        if (!rex.w) ZEROUP(s1);
        IFX (X_PEND) {
            SDxw(s1, xEmu, offsetof(x64emu_t, res));
        }
        if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
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
        // OF flag is set to the most-significant bit of the original operand
        SRLIxw(s3, s1, rex.w ? 63 : 31);
        BSTRINS_D(xFlags, s3, F_OF, F_OF);
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
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SHR32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_shr32c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4)
{
    if (!c) return;
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

    if (cpuext.lbt) {
        IFX (X_PEND) {
        } else IFX (X_ALL) {
            MOV64x(s3, c);
        }
        IFX (X_ALL) {
            if (rex.w)
                X64_SRL_D(s1, s3);
            else
                X64_SRL_W(s1, s3);
        }
        IFXA ((X_AF | X_OF), BOX64DRENV(dynarec_safeflags)) {
            SRLIxw(s3, s1, (rex.w ? 63 : 31) - F_OF);
            BSTRINS_D(s3, xZR, F_AF, F_AF);
            X64_SET_EFLAGS(s3, X_OF | X_AF);
        }
        SRLIxw(s1, s1, c); // no need to zeroup on !rex.w
        IFX (X_PEND) {
            SDxw(s1, xEmu, offsetof(x64emu_t, res));
        }
        if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
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
        // OF flag is set to the most-significant bit of the original operand
        SRLIxw(s3, s1, rex.w ? 63 : 31);
        BSTRINS_D(xFlags, s3, F_OF, F_OF);
    }

    if (rex.w) {
        SRLI_D(s1, s1, c);
    } else {
        SRLI_W(s1, s1, c); // no need to zeroup
    }

    IFX (X_SF) {
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
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
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}


// emit SAR16 instruction, from s1, shift s2 (!0 and and'd already), store result in s1 using s3, s4 and s5 as scratch
void emit_sar16(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    int64_t j64;
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
        IFXA ((X_AF | X_OF), BOX64DRENV(dynarec_safeflags)) X64_SET_EFLAGS(xZR, (X_AF | X_OF));

        SRL_D(s1, s1, s2);
        BSTRPICK_D(s1, s1, 15, 0);

        IFX (X_PEND) {
            ST_H(s1, xEmu, offsetof(x64emu_t, res));
        }
        if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
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
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}


// emit SAR16 instruction, from s1 , constant c, store result in s1 using s3, s4 and s5 as scratch
void emit_sar16c(dynarec_la64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4, int s5)
{
    if (!c) return;
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
        IFXA ((X_AF | X_OF), BOX64DRENV(dynarec_safeflags)) X64_SET_EFLAGS(xZR, (X_AF | X_OF));
        SRLI_D(s1, s1, c);
        IFX (X_PEND) {
            ST_H(s1, xEmu, offsetof(x64emu_t, res));
        }
        if (dyn->insts[ninst].nat_flags_fusion) {
            BSTRPICK_D(s1, s1, 15, 0);
            NAT_FLAGS_OPS(s1, xZR, s3, xZR);
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
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SAR32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_sar32c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4)
{
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

        IFXA ((X_AF | X_OF), BOX64DRENV(dynarec_safeflags)) X64_SET_EFLAGS(xZR, X_OF | X_AF);

        SRAIxw(s1, s1, c);
        if (!rex.w) ZEROUP(s1);
        IFX (X_PEND) {
            SDxw(s1, xEmu, offsetof(x64emu_t, res));
        }
        if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
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
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

void emit_sar32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4)
{
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SDxw(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, rex.w ? d_sar64 : d_sar32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    if (cpuext.lbt) {
        IFX (X_ALL) {
            if (rex.w)
                X64_SRA_D(s1, s2);
            else
                X64_SRA_W(s1, s2);
        }

        IFXA ((X_AF | X_OF), BOX64DRENV(dynarec_safeflags)) X64_SET_EFLAGS(xZR, X_OF | X_AF);

        SRAxw(s1, s1, s2);
        if (!rex.w) ZEROUP(s1);
        IFX (X_PEND) {
            SDxw(s1, xEmu, offsetof(x64emu_t, res));
        }
        if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
        return;
    }

    CLEAR_FLAGS(s3);
    IFX (X_CF) {
        ADDIxw(s3, s2, -1);
        SRAxw(s3, s1, s3);
        BSTRINS_D(xFlags, s3, F_CF, F_CF);
    }
    if (rex.w)
        SRA_D(s1, s1, s2);
     else 
        SRA_W(s1, s1, s2);

    IFX (X_SF) {
        BGE(s1, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) ZEROUP(s1);
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
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}


// emit ROR32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_ror32c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4)
{
    if (!c) return;

    IFXORNAT (X_ALL) SET_DFNONE();
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

    IFXA (X_OF, (!cpuext.lbt || BOX64DRENV(dynarec_safeflags))) {
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

    if (cpuext.lbt) {
        if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
        return;
    }

    IFX (X_CF) {
        SRLIxw(s3, s1, rex.w ? 63 : 31);
        OR(xFlags, xFlags, s3);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit ROL8 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_rol8(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFXORNAT (X_ALL) SET_DFNONE();
    RESTORE_EFLAGS(s3);

    IFX (X_OF) {
        SRLI_W(s3, s1, 6);
        SRLI_D(s4, s3, 1);
        XOR(s3, s3, s4);
        BSTRINS_D(xFlags, s3, F_OF, F_OF);
    }
    ADDI_D(s4, xZR, 8);
    SUB_D(s4, s4, s2);
    BSTRINS_D(s1, s1, 15, 8);
    SRL_D(s1, s1, s4);
    IFX (X_CF) {
        BSTRINS_D(xFlags, s1, F_CF, F_CF);
    }

    IFXA (X_ALL, cpuext.lbt) SPILL_EFLAGS();
    if (dyn->insts[ninst].nat_flags_fusion) {
        ANDI(s1, s1, 0xFF);
        NAT_FLAGS_OPS(s1, xZR, s3, xZR);
    }
}

// emit ROL8 instruction, from s1, c, store result in s1 using s3 s4 and s5 as scratch
void emit_rol8c(dynarec_la64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4, int s5)
{
    int64_t j64;
    if (!c) return;
    if (cpuext.lbt) {
        IFX (X_CF | X_OF) X64_ROTLI_B(s1, c);
        IFXA (X_OF, BOX64DRENV(dynarec_safeflags)) {
            SRLI_W(s3, s1, 6);
            SRLI_D(s4, s3, 1);
            XOR(s3, s3, s4);
            SLLI_D(s3, s3, F_OF);
            X64_SET_EFLAGS(s3, X_OF);
        }
        if (c & 7)
            ROTRI_B(s1, s1, 8 - (c & 7));

        IFX (X_PEND) {
            ST_B(s1, xEmu, offsetof(x64emu_t, res));
        }

        if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
        return;
    }

    IFX (X_OF) {
        SRLI_W(s3, s1, 6);
        SRLI_D(s4, s3, 1);
        XOR(s3, s3, s4);
        BSTRINS_D(xFlags, s3, F_OF, F_OF);
    }
    if (c & 7) {
        SLLI_D(s3, s1, (c & 7));
        SRLI_D(s1, s1, 8 - (c & 7));
        OR(s1, s3, s1);
    }

    IFX (X_CF) {
        BSTRINS_D(xFlags, s1, F_CF, F_CF);
    }
    if (dyn->insts[ninst].nat_flags_fusion) {
        ANDI(s1, s1, 0xFF);
        NAT_FLAGS_OPS(s1, xZR, s3, xZR);
    }
}

// emit ROL16 instruction, from s1, c, store result in s1 using s3 s4 and s5 as scratch
void emit_rol16c(dynarec_la64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4, int s5)
{
    if (!c) return;
    IFXORNAT (X_ALL) SET_DFNONE();
    RESTORE_EFLAGS(s3);

    IFXA (X_OF, c == 1) {
        SRLI_D(s3, s1, 14);
        SRLI_D(s4, s3, 1);
        XOR(s3, s4, s3);
        BSTRINS_D(xFlags, s3, F_OF, F_OF);
    }

    if (c & 15) {
        int rc = 16 - (c & 15);
        if (cpuext.lbt)
            ROTRI_H(s1, s1, rc);
        else {
            SLLI_D(s3, s1, 16);
            OR(s1, s3, s1);
            SRLI_D(s1, s1, rc);
        }
    }

    IFX (X_CF) BSTRINS_D(xFlags, s1, F_CF, F_CF);

    IFXA (X_ALL, cpuext.lbt) SPILL_EFLAGS();
    if (dyn->insts[ninst].nat_flags_fusion) {
        if (!cpuext.lbt) BSTRPICK_D(s1, s1, 15, 0);
        NAT_FLAGS_OPS(s1, xZR, s3, xZR);
    }
}

// emit ROL32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_rol32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4)
{
    IFXORNAT (X_ALL) SET_DFNONE();
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


    IFXA (X_OF, (!cpuext.lbt || BOX64DRENV(dynarec_safeflags))) {
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
    if (!rex.w) ZEROUP(s1);

    if (cpuext.lbt) {
        if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
        return;
    }

    IFX (X_CF) {
        ANDI(s4, s1, 1); // LSB == F_CF
        OR(xFlags, xFlags, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit ROR32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_ror32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4)
{
    IFXORNAT (X_ALL) SET_DFNONE();
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

    IFXA (X_OF, (!cpuext.lbt || BOX64DRENV(dynarec_safeflags))) {
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
    if (!rex.w) ZEROUP(s1);

    if (cpuext.lbt) {
        if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
        return;
    }

    IFX (X_CF) {
        SRLIxw(s3, s1, rex.w ? 63 : 31);
        OR(xFlags, xFlags, s3);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit ROL32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_rol32c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4)
{
    if (!c) return;

    IFXORNAT (X_ALL) SET_DFNONE();
    IFXA (X_CF | X_OF, !cpuext.lbt) {
        MOV64x(s3, ((1UL << F_CF) | (1UL << F_OF)));
        ANDN(xFlags, xFlags, s3);
    }

    IFXA ((X_CF | X_OF), cpuext.lbt) {
        if (rex.w)
            X64_ROTLI_D(s1, c);
        else
            X64_ROTLI_W(s1, c);
    }

    IFXA (X_OF, (!cpuext.lbt || BOX64DRENV(dynarec_safeflags))) {
        SRLIxw(s3, s1, rex.w ? 62 : 30);
        SRLI_D(s4, s3, 1);
        XOR(s3, s3, s4);
        if (cpuext.lbt) {
            SLLI_D(s3, s3, F_OF);
            X64_SET_EFLAGS(s3, X_OF);
        } else
            BSTRINS_D(xFlags, s3, F_OF, F_OF);
    }

    ROTRIxw(s1, s1, (rex.w ? 64 : 32) - c);
    if (!rex.w) ZEROUP(s1);

    if (cpuext.lbt) {
        if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
        return;
    }

    IFX (X_CF) {
        ANDI(s4, s1, 1 << F_CF);
        OR(xFlags, xFlags, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}


void emit_shld32c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, uint32_t c, int s3, int s4)
{
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

    CLEAR_FLAGS(s3);
    IFX (X_CF) {
        SRLI_D(s3, s1, (rex.w ? 64 : 32) - c);
        BSTRINS_D(xFlags, s3, F_CF, F_CF);
    }
    IFX (X_OF) {
        SRLIxw(s3, s1, rex.w ? 62 : 30);
        SRLI_D(s4, s3, 1);
        XOR(s3, s3, s4);
        BSTRINS_D(xFlags, s3, F_OF, F_OF);
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
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    IFXA (X_ALL, cpuext.lbt) SPILL_EFLAGS();
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}


// emit SHRD32 instruction, from s1, fill s2 , constant c, store result in s1 using s3 and s4 as scratch
void emit_shrd32c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, uint32_t c, int s3, int s4)
{
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

    CLEAR_FLAGS(s3);
    IFX (X_CF) {
        int reg;
        if (c > 1) {
            SRAI_D(s3, s1, c - 1);
            reg = s3;
        } else
            reg = s1;
        BSTRINS_D(xFlags, reg, F_CF, F_CF);
    }

    IFX (X_OF) {
        SRLIxw(s3, s1, rex.w ? 63 : 31);
        XOR(s3, s3, s2);
        BSTRINS_D(xFlags, s3, F_OF, F_OF);
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
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    IFXA (X_ALL, cpuext.lbt) SPILL_EFLAGS();
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

void emit_shrd32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s5, int s3, int s4, int s6)
{
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
        BSTRINS_D(xFlags, s3, F_CF, F_CF);
    }
    IFX (X_OF) {
        SRLIxw(s3, s1, rex.w ? 63 : 31);
        XOR(s3, s3, s2);
        BSTRINS_D(xFlags, s3, F_OF, F_OF);
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
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

void emit_shld32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s5, int s3, int s4, int s6)
{
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
        BSTRINS_D(xFlags, s4, F_CF, F_CF);
    }
    IFX (X_OF) {
        SRLIxw(s6, s1, rex.w ? 62 : 30);
        SRLI_D(s4, s6, 1);
        XOR(s6, s6, s4);
        BSTRINS_D(xFlags, s6, F_OF, F_OF);
    }
    SLLxw(s4, s1, s5);
    SRLxw(s3, s2, s3);
    OR(s1, s3, s4);

    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    if (!rex.w) ZEROUP(s1);
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
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit RCL8 instruction, from s1, s1, store result in s1 using s3, s4 and s5 as scratch
void emit_rcl8(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    int64_t j64;
    IFXORNAT (X_ALL) SET_DFNONE();
    RESTORE_EFLAGS(s3);

    ADDI_D(s4, xZR, 9);
    MOD_DU(s2, s2, s4);
    CBZ_NEXT(s2);

    IFX (X_OF) {
        SRLI_D(s3, s1, 6);
        SRLI_D(s5, s3, 1);
        XOR(s3, s3, s5);
        BSTRINS_D(xFlags, s3, F_OF, F_OF);
    }

    BSTRINS_D(s1, xFlags, 8, 8); // insert CF to bit 8
    BSTRINS_D(s1, s1, 16, 9);
    SUB_D(s2, s4, s2);
    IFX (X_CF) {
        ADDI_D(s5, s2, -1);
        SRL_D(s5, s1, s5);
        BSTRINS_D(xFlags, s5, F_CF, F_CF);
    }
    SRL_D(s1, s1, s2);

    IFXA (X_ALL, cpuext.lbt) SPILL_EFLAGS();
    if (dyn->insts[ninst].nat_flags_fusion) {
        ANDI(s1, s1, 0xFF);
        NAT_FLAGS_OPS(s1, xZR, s3, xZR);
    }
}

// emit RCL8 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_rcl8c(dynarec_la64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4)
{
    c %= 9;
    if (!c) return;

    IFXORNAT (X_ALL) SET_DFNONE();
    RESTORE_EFLAGS(s3);

    BSTRINS_D(s1, xFlags, 8, 8); // insert CF to bit 8
    IFX (X_OF | X_CF) {
        BSTRPICK_D(s3, s1, 8 - c, 8 - c);
        BSTRINS_D(xFlags, s3, F_CF, F_CF);
    }

    IFX (X_OF) {
        SRLI_D(s3, s1, 6);
        SRLI_D(s4, s3, 1);
        XOR(s3, s3, s4);
        BSTRINS_D(xFlags, s3, F_OF, F_OF);
    }

    BSTRINS_D(s1, s1, 16, 9);
    SRLI_D(s1, s1, 9 - c);

    IFXA (X_ALL, cpuext.lbt) SPILL_EFLAGS();
    if (dyn->insts[ninst].nat_flags_fusion) {
        ANDI(s1, s1, 0xFF);
        NAT_FLAGS_OPS(s1, xZR, s3, xZR);
    }
}

// emit RCL16 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_rcl16c(dynarec_la64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4)
{
    c %= 17;
    if (!c) return;

    IFXORNAT (X_ALL) SET_DFNONE();
    RESTORE_EFLAGS(s3);

    BSTRINS_D(s1, xFlags, 16, 16); // insert CF to bit 16
    IFX (X_CF) {
        BSTRPICK_D(s3, s1, 16 - c, 16 - c);
        BSTRINS_D(xFlags, s3, F_CF, F_CF);
    }

    IFX (X_OF) {
        SRLI_D(s3, s1, 14);
        SRLI_D(s4, s3, 1);
        XOR(s3, s3, s4);
        BSTRINS_D(xFlags, s3, F_OF, F_OF);
    }

    BSTRINS_D(s1, s1, 32, 17);
    SRLI_D(s1, s1, 17 - c);

    IFXA (X_ALL, cpuext.lbt) SPILL_EFLAGS();
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit RCL32 instruction, from s1, s2, store result in s1 using s3, s4 and s5 as scratch
void emit_rcl32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    IFXORNAT (X_ALL) SET_DFNONE();
    RESTORE_EFLAGS(s3);
    IFX (X_CF) {
        ADDI_D(s4, xZR, rex.w ? 64 : 32);
        SUB_D(s4, s4, s2);
        SRL_D(s3, s1, s4);
    }
    IFX (X_OF) {
        SRLI_D(s4, s1, rex.w ? 62 : 30);
        SRLI_D(s5, s4, 1);
        XOR(s4, s5, s4);
        BSTRINS_D(xFlags, s4, F_OF, F_OF);
    }
    MV(s4, xFlags);
    BSTRINS_D(s4, s1, rex.w ? 63 : 31, 1);
    ADDI_D(s5, s2, -1);
    BEQZ(s5, 4 + 4 * 5); // goto label
    SLL_D(s4, s4, s5);
    ADDI_D(s5, xZR, rex.w ? 65 : 33);
    SUB_D(s2, s5, s2);
    if (rex.w)
        SRL_D(s1, s1, s2);
    else
        SRL_W(s1, s1, s2);
    OR(s4, s1, s4);
    // label
    if (rex.w)
        MV(s1, s4);
    else
        ZEROUP2(s1, s4);

    IFX (X_CF) {
        BSTRINS_D(xFlags, s3, F_CF, F_CF);
    }

    IFXA (X_ALL, cpuext.lbt) SPILL_EFLAGS();
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit RCL32 instruction, from s1 , constant c, store result in s1 using s3, s4 and s5 as scratch
void emit_rcl32c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4, int s5)
{
    if (!c) return;

    IFXORNAT (X_ALL) SET_DFNONE();
    RESTORE_EFLAGS(s3);
    IFX (X_CF) {
        SRLI_D(s3, s1, (rex.w ? 64 : 32) - c);
    }

    IFX (X_OF) {
        SRLI_D(s4, s1, rex.w ? 62 : 30);
        SRLI_D(s5, s4, 1);
        XOR(s4, s5, s4);
        BSTRINS_D(xFlags, s4, F_OF, F_OF);
    }
    if (c == 1) {
        SLLI_D(s1, s1, 1);
        BSTRINS_D(s1, xFlags, 0, 0);
    } else {
        SLLI_D(s4, s1, c);
        BSTRINS_D(s4, xFlags, c - 1, c - 1);
        if (rex.w)
            SRLI_D(s5, s1, 65 - c);
        else
            SRLI_W(s5, s1, 33 - c);
        OR(s1, s5, s4);
    }
    if (!rex.w) ZEROUP(s1);

    IFX (X_CF) {
        BSTRINS_D(xFlags, s3, F_CF, F_CF);
    }

    IFXA (X_ALL, cpuext.lbt) SPILL_EFLAGS();
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit RCR8 instruction, from s1, s2, store result in s1 using s3, s4 and s5 as scratch
void emit_rcr8(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{

    int64_t j64;
    IFXORNAT (X_ALL) SET_DFNONE();
    RESTORE_EFLAGS(s3);

    ADDI_D(s4, xZR, 9);
    MOD_DU(s2, s2, s4);
    CBZ_NEXT(s2);

    BSTRINS_D(s1, xFlags, 8, 8); // insert CF to bit 8
    BSTRINS_D(s1, s1, 16, 9);

    IFX (X_OF) {
        SRLI_D(s3, s1, 7);
        XOR(s3, s3, xFlags);
        BSTRINS_D(xFlags, s3, F_OF, F_OF);
    }
    IFX (X_CF) {
        ADDI_D(s4, s2, -1);
        SRL_D(s4, s1, s4);
        BSTRINS_D(xFlags, s4, F_CF, F_CF);
    }
    SRL_D(s1, s1, s2);

    IFXA (X_ALL, cpuext.lbt) SPILL_EFLAGS();
    if (dyn->insts[ninst].nat_flags_fusion) {
        ANDI(s1, s1, 0xFF);
        NAT_FLAGS_OPS(s1, xZR, s3, xZR);
    }
}

// emit RCR8 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_rcr8c(dynarec_la64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4)
{
    c %= 9;
    if (!c) return;

    IFXORNAT (X_ALL) SET_DFNONE();
    RESTORE_EFLAGS(s3);
    IFX (X_OF) {
        SRLI_D(s3, s1, 7);
        XOR(s3, s3, xFlags);
        BSTRINS_D(xFlags, s3, F_OF, F_OF);
    }

    if (c) {
        BSTRINS_D(s1, xFlags, 8, 8); // insert CF to bit 8
        IFX (X_CF) {
            BSTRPICK_D(s3, s1, c - 1, c - 1);
            BSTRINS_D(xFlags, s3, F_CF, F_CF);
        }
        if (c > 1) {
            BSTRINS_D(s1, s1, 16, 9);
            OR(s1, s1, s3);
        }
        SRLI_D(s1, s1, c);
    }

    IFXA (X_ALL, cpuext.lbt) SPILL_EFLAGS();
    if (dyn->insts[ninst].nat_flags_fusion) {
        ANDI(s1, s1, 0xFF);
        NAT_FLAGS_OPS(s1, xZR, s3, xZR);
    }
}

// emit RCR16 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_rcr16c(dynarec_la64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4)
{
    c %= 17;
    if (!c) return;

    IFXORNAT (X_ALL) SET_DFNONE();
    RESTORE_EFLAGS(s3);
    IFX (X_OF) {
        SRLI_D(s3, s1, 15);
        XOR(s3, s3, xFlags);
        BSTRINS_D(xFlags, s3, F_OF, F_OF);
    }

    if (c) {
        BSTRINS_D(s1, xFlags, 16, 16); // insert CF to bit 16
        IFX (X_CF) {
            BSTRPICK_D(s3, s1, c - 1, c - 1);
            BSTRINS_D(xFlags, s3, F_CF, F_CF);
        }
        if (c > 1) BSTRINS_D(s1, s1, 32, 17);
        SRLI_D(s1, s1, c);
    }

    IFXA (X_ALL, cpuext.lbt) SPILL_EFLAGS();
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit RCR32 instruction, from s1, s2, store result in s1 using s3, s4 and s5 as scratch
void emit_rcr32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    IFXORNAT (X_ALL) SET_DFNONE();
    RESTORE_EFLAGS(s3);

    IFX (X_OF) {
        SRLI_D(s3, s1, rex.w ? 63 : 31);
        XOR(s3, s3, xFlags);
        BSTRINS_D(xFlags, s3, F_OF, F_OF);
    }
    ADDI_D(s5, s2, -1);
    IFX (X_CF) SRL_D(s3, s1, s5); // for later

    SRLI_D(s4, s1, 1);
    BSTRINS_D(s4, xFlags, rex.w ? 63 : 31, rex.w ? 63 : 31); // insert cf
    BEQZ(s5, 4 + 4 * 5);                                     // goto label
    SRL_D(s4, s4, s5);
    ADDI_D(s5, xZR, rex.w ? 65 : 33);
    SUB_D(s2, s5, s2);
    if (rex.w)
        SLL_D(s1, s1, s2);
    else
        SLL_W(s1, s1, s2);
    OR(s4, s1, s4);
    // label
    if (rex.w)
        MV(s1, s4);
    else
        ZEROUP2(s1, s4);

    IFX (X_CF) BSTRINS_D(xFlags, s3, F_CF, F_CF);
    IFXA (X_ALL, cpuext.lbt) SPILL_EFLAGS();
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit RCR32 instruction, from s1 , constant c, store result in s1 using s3, s4 and s5 as scratch
void emit_rcr32c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4, int s5)
{
    if (!c) return;

    IFXORNAT (X_ALL) SET_DFNONE();
    RESTORE_EFLAGS(s3);
    IFX (X_OF) {
        SRLI_D(s3, s1, rex.w ? 63 : 31);
        XOR(s3, s3, xFlags);
        BSTRINS_D(xFlags, s3, F_OF, F_OF);
    }
    IFX (X_CF) BSTRPICK_D(s5, s1, c - 1, c - 1);
    if (c == 1) {
        SRLI_D(s1, s1, 1);
        BSTRINS_D(s1, xFlags, rex.w ? 63 : 31, rex.w ? 63 : 31);
    } else {
        SRLIxw(s4, s1, c);
        BSTRINS_D(s4, xFlags, (rex.w ? 64 : 32) - c, (rex.w ? 64 : 32) - c);
        SLLI_D(s3, s1, (rex.w ? 65 : 33) - c);
        OR(s1, s3, s4);
    }
    if (!rex.w) ZEROUP(s1);

    IFX (X_CF) {
        BSTRINS_D(xFlags, s5, F_CF, F_CF);
    }

    IFXA (X_ALL, cpuext.lbt) SPILL_EFLAGS();
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit ROR8 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_ror8(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFXORNAT (X_ALL) SET_DFNONE();
    RESTORE_EFLAGS(s3);

    IFX (X_OF) {
        SRLI_D(s3, s1, 7);
        XOR(s3, s1, s3);
        BSTRINS_D(xFlags, s3, F_OF, F_OF);
    }

    BSTRINS_D(s1, s1, 15, 8);
    SRL_D(s1, s1, s2);

    IFX (X_CF) {
        SRLI_D(s3, s1, 7);
        BSTRINS_D(xFlags, s3, F_CF, F_CF);
    }

    IFXA (X_ALL, cpuext.lbt) SPILL_EFLAGS();
    if (dyn->insts[ninst].nat_flags_fusion) {
        ANDI(s1, s1, 0xFF);
        NAT_FLAGS_OPS(s1, xZR, s3, xZR);
    }
}

// emit ROR8 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_ror8c(dynarec_la64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4)
{
    if (!c) return;
    IFXORNAT (X_ALL) SET_DFNONE();
    RESTORE_EFLAGS(s3);

    IFX (X_OF) {
        SRLI_D(s3, s1, 7);
        XOR(s3, s1, s3);
        BSTRINS_D(xFlags, s3, F_OF, F_OF);
    }

    if (c & 7) {
        BSTRINS_D(s1, s1, 15, 8);
        SRLI_D(s1, s1, c & 7);
    }
    IFX (X_CF) {
        SRLI_D(s3, s1, 7);
        BSTRINS_D(xFlags, s3, F_CF, F_CF);
    }

    IFXA (X_ALL, cpuext.lbt) SPILL_EFLAGS();
    if (dyn->insts[ninst].nat_flags_fusion) {
        ANDI(s1, s1, 0xFF);
        NAT_FLAGS_OPS(s1, xZR, s3, xZR);
    }
}

// emit ROR16 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_ror16c(dynarec_la64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4)
{
    if (!c) return;
    IFXORNAT (X_ALL) SET_DFNONE();
    RESTORE_EFLAGS(s3);
    IFXA (X_OF, (c == 1)) {
        SRLI_D(s3, s1, 15);
        XOR(s3, s3, s1);
        BSTRINS_D(xFlags, s3, F_OF, F_OF);
    }

    if (c & 15) {
        if (cpuext.lbt) {
            ROTRI_H(s1, s1, (c & 15));
        } else {
            SRLI_D(s3, s1, c & 15);
            SLLI_D(s1, s1, 16 - (c & 15));
            OR(s1, s1, s3);
            BSTRPICK_D(s1, s1, 15, 0);
        }
    }

    IFX (X_CF) {
        SRLI_D(s3, s1, 15);
        BSTRINS_D(xFlags, s3, F_CF, F_CF);
    }

    IFXA (X_ALL, cpuext.lbt) SPILL_EFLAGS();
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

void emit_shld16c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, uint32_t c, int s3, int s4, int s5)
{
    c &= 0x1f;
    IFX (X_PEND) {
        if (c) {
            MOV64x(s3, c);
            ST_H(s3, xEmu, offsetof(x64emu_t, op2));
        } else
            ST_H(xZR, xEmu, offsetof(x64emu_t, op2));
        ST_H(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, d_shld16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    if (!c) {
        IFX (X_PEND) {
            ST_H(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }

    CLEAR_FLAGS(s3);
    IFX (X_OF) {
        SRLI_D(s3, s1, 14);
        SRLI_D(s4, s3, 1);
        XOR(s3, s3, s4);
        BSTRINS_D(xFlags, s3, F_OF, F_OF);
    }

    // create concat first
    SLLI_D(s5, s2, 16);
    OR(s1, s1, s5);

    IFX (X_CF) {
        if (c <= 16) {
            SRLI_D(s3, s1, 16 - c);
        } else {
            SRLI_D(s3, s2, 32 - c);
        }
        BSTRINS_D(xFlags, s3, F_CF, F_CF);
    }
    SLLI_D(s3, s1, c);
    SRLI_D(s1, s1, 32 - c);
    OR(s1, s1, s3);
    BSTRPICK_D(s1, s1, 15, 0);

    IFX (X_PEND) {
        ST_H(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_SF) {
        SLLI_W(s4, s1, 16);
        BGE(s4, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }

    IFXA (X_ALL, cpuext.lbt) SPILL_EFLAGS();
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}


void emit_shrd16c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, uint32_t c, int s3, int s4, int s5)
{
    c &= 0x1f;

    IFX (X_PEND) {
        if (c) {
            MOV64x(s3, c);
            ST_H(s3, xEmu, offsetof(x64emu_t, op2));
        } else
            ST_H(xZR, xEmu, offsetof(x64emu_t, op2));
        ST_H(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, d_shrd16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    if (!c) {
        IFX (X_PEND) {
            ST_H(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }
    CLEAR_FLAGS(s3);
    IFX (X_OF) {
        SRLI_D(s3, s1, 15);
        XOR(s3, s3, s2);
        BSTRINS_D(xFlags, s3, F_OF, F_OF);
    }

    // create concat first
    SLLI_D(s5, s2, 16);
    OR(s1, s1, s5);

    IFX (X_CF) {
        if (c > 1) {
            SRAI_D(s3, s1, c - 1);
            BSTRINS_D(xFlags, s3, F_CF, F_CF);
        } else {
            BSTRINS_D(xFlags, s1, F_CF, F_CF);
        }
    }

    SRLI_D(s5, s1, c);
    SLLI_D(s1, s1, 32 - c);
    OR(s1, s1, s5);
    BSTRPICK_D(s1, s1, 15, 0);

    IFX (X_PEND) {
        ST_H(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_SF) {
        SLLI_W(s4, s1, 16);
        BGE(s4, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    IFXA (X_ALL, cpuext.lbt) SPILL_EFLAGS();
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}


void emit_shld16(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5, int s6)
{
    IFX (X_PEND) {
        ST_H(s1, xEmu, offsetof(x64emu_t, op1));
        ST_H(s3, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, d_shld16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    CLEAR_FLAGS(s6);
    IFX (X_OF) {
        SRLI_D(s5, s1, 14);
        SRLI_D(s4, s5, 1);
        XOR(s5, s5, s4);
        ANDI(s5, s5, 1);
        SLLI_D(s5, s5, F_OF);
        OR(xFlags, xFlags, s5);
    }

    MOV32w(s6, 32);
    SUB_W(s6, s6, s3);
    IFX (X_CF) {
        SLLI_D(s5, s1, 16);
        OR(s4, s2, s5);
        SRL_W(s5, s4, s6);
        ANDI(s5, s5, 1); // LSB == F_CF
        OR(xFlags, xFlags, s5);
    }
    // create concat first
    SLLI_D(s5, s2, 16);
    OR(s1, s1, s5);

    SLL_W(s5, s1, s3);
    SRL_W(s1, s1, s6);
    OR(s1, s1, s5);
    BSTRPICK_D(s1, s1, 15, 0);

    IFX (X_PEND) {
        ST_H(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_SF) {
        SLLI_W(s4, s1, 16);
        BGE(s4, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s4, s5);
    }

    IFXA (X_ALL, cpuext.lbt) SPILL_EFLAGS();
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

void emit_shrd16(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5, int s6)
{
    IFX (X_PEND) {
        ST_H(s1, xEmu, offsetof(x64emu_t, op1));
        ST_H(s3, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, d_shrd16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    CLEAR_FLAGS(s6);
    SLLI_D(s5, s2, 16);
    OR(s1, s1, s5);
    IFX (X_OF) {
        SRLI_D(s5, s1, 15);
        XOR(s5, s5, s2);
        ANDI(s5, s5, 1);
        SLLI_D(s5, s5, F_OF);
        OR(xFlags, xFlags, s5);
    }

    IFX (X_CF) {
        ADDI_W(s5, s3, -1);
        SRL_W(s5, s1, s5);
        ANDI(s5, s5, 1); // LSB == F_CF
        OR(xFlags, xFlags, s5);
    }

    MOV32w(s6, 32);
    SUB_W(s6, s6, s3);
    SRL_W(s5, s1, s3);
    SLL_W(s1, s1, s6);
    OR(s1, s1, s5);
    BSTRPICK_D(s1, s1, 15, 0);

    IFX (X_PEND) {
        ST_H(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        BNEZ(s1, 8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_SF) {
        SLLI_W(s4, s1, 16);
        BGE(s4, xZR, 8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s4, s5);
    }
    IFXA (X_ALL, cpuext.lbt) SPILL_EFLAGS();
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit ROL16 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_rol16(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFXORNAT (X_ALL) SET_DFNONE();
    RESTORE_EFLAGS(s3);

    ANDI(s2, s2, 0xf);
    IFX (X_OF) {
        SRLI_W(s3, s1, 14);
        SRLI_D(s4, s3, 1);
        XOR(s3, s3, s4);
        BSTRINS_D(xFlags, s3, F_OF, F_OF);
    }

    ADDI_D(s4, xZR, 16);
    SUB_D(s4, s4, s2);
    BSTRINS_D(s1, s1, 31, 16);
    SRL_D(s1, s1, s4);
    IFX (X_CF) {
        BSTRINS_D(xFlags, s1, F_CF, F_CF);
    }

    IFXA (X_ALL, cpuext.lbt) SPILL_EFLAGS();
    if (dyn->insts[ninst].nat_flags_fusion) {
        BSTRPICK_D(s1, s1, 15, 0);
        NAT_FLAGS_OPS(s1, xZR, s3, xZR);
    }
}

// emit ROR16 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_ror16(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFXORNAT (X_ALL) SET_DFNONE();
    RESTORE_EFLAGS(s3);

    ANDI(s2, s2, 0xf);
    IFX (X_OF) {
        SRLI_D(s3, s1, 15);
        XOR(s3, s1, s3);
        BSTRINS_D(xFlags, s3, F_OF, F_OF);
    }

    BSTRINS_D(s1, s1, 31, 16);
    SRL_D(s1, s1, s2);

    IFX (X_CF) {
        SRLI_D(s3, s1, 15);
        BSTRINS_D(xFlags, s3, F_CF, F_CF);
    }

    IFXA (X_ALL, cpuext.lbt) SPILL_EFLAGS();
    if (dyn->insts[ninst].nat_flags_fusion) {
        BSTRPICK_D(s1, s1, 15, 0);
        NAT_FLAGS_OPS(s1, xZR, s3, xZR);
    }
}

// emit RCL16 instruction, from s1, s1, store result in s1 using s3, s4 and s5 as scratch
void emit_rcl16(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    int64_t j64;
    IFXORNAT (X_ALL) SET_DFNONE();
    RESTORE_EFLAGS(s3);

    ADDI_D(s4, xZR, 17);
    MOD_DU(s2, s2, s4);
    CBZ_NEXT(s2);

    IFX (X_OF) {
        SRLI_D(s3, s1, 14);
        SRLI_D(s5, s3, 1);
        XOR(s3, s3, s5);
        BSTRINS_D(xFlags, s3, F_OF, F_OF);
    }

    BSTRINS_D(s1, xFlags, 16, 16); // insert CF to bit 16
    BSTRINS_D(s1, s1, 32, 17);
    SUB_D(s2, s4, s2);
    IFX (X_CF) {
        ADDI_D(s5, s2, -1);
        SRL_D(s5, s1, s5);
        BSTRINS_D(xFlags, s5, F_CF, F_CF);
    }
    SRL_D(s1, s1, s2);
    IFXA (X_ALL, cpuext.lbt) SPILL_EFLAGS();
    if (dyn->insts[ninst].nat_flags_fusion) {
        BSTRPICK_D(s1, s1, 15, 0);
        NAT_FLAGS_OPS(s1, xZR, s3, xZR);
    }
}

// emit RCR16 instruction, from s1, s2, store result in s1 using s3, s4 and s5 as scratch
void emit_rcr16(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{

    int64_t j64;
    IFXORNAT (X_ALL) SET_DFNONE();
    RESTORE_EFLAGS(s3);

    ADDI_D(s4, xZR, 17);
    MOD_DU(s2, s2, s4);
    CBZ_NEXT(s2);

    BSTRINS_D(s1, xFlags, 16, 16); // insert CF to bit 16
    BSTRINS_D(s1, s1, 32, 17);

    IFX (X_OF) {
        SRLI_D(s3, s1, 15);
        XOR(s3, s3, xFlags);
        BSTRINS_D(xFlags, s3, F_OF, F_OF);
    }
    IFX (X_CF) {
        ADDI_D(s4, s2, -1);
        SRL_D(s4, s1, s4);
        BSTRINS_D(xFlags, s4, F_CF, F_CF);
    }
    SRL_D(s1, s1, s2);

    IFXA (X_ALL, cpuext.lbt) SPILL_EFLAGS();
    if (dyn->insts[ninst].nat_flags_fusion) {
        BSTRPICK_D(s1, s1, 15, 0);
        NAT_FLAGS_OPS(s1, xZR, s3, xZR);
    }
}
