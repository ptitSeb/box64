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

// emit SHL8 instruction, from s1 , shift s2, store result in s1 using s3, s4 and s5 as scratch
void emit_shl8(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    // s2 is not 0 here and is 1..1f/3f
    IFX (X_PEND) {
        STB(s1, offsetof(x64emu_t, op1), xEmu);
        STB(s2, offsetof(x64emu_t, op2), xEmu);
        SET_DF(s4, d_shl8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    CLEAR_FLAGS(s3);

    IFX (X_OF) {
        SRDI(s3, s1, 6);
        SRDI(s4, s3, 1);
        XOR(s3, s3, s4);
        BF_INSERT(xFlags, s3, F_OF, F_OF);
    }

    SLD(s1, s1, s2);

    IFX (X_CF) {
        SRDI(s5, s1, 8);
        BF_INSERT(xFlags, s5, F_CF, F_CF);
    }

    SLDI(s1, s1, 56);
    IFX (X_SF) {
        CMPDI(s1, 0);
        BGE(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    SRDI(s1, s1, 56);

    IFX (X_PEND) {
        STB(s1, offsetof(x64emu_t, res), xEmu);
    }
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SHL8 instruction, from s1 , constant c, store result in s1 using s3, s4 as scratch
void emit_shl8c(dynarec_ppc64le_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4)
{
    if (!c) return;
    IFX (X_PEND) {
        MOV64x(s3, c);
        STB(s3, offsetof(x64emu_t, op2), xEmu);
        STB(s1, offsetof(x64emu_t, op1), xEmu);
        SET_DF(s4, d_shl8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    CLEAR_FLAGS(s3);
    IFX (X_OF) {
        SRDI(s3, s1, 6);
        SRDI(s4, s3, 1);
        XOR(s3, s3, s4);
        BF_INSERT(xFlags, s3, F_OF, F_OF);
    }
    SLDI(s1, s1, c);
    IFX (X_CF) {
        BF_EXTRACT(s3, s1, 8, 8);
        BF_INSERT(xFlags, s3, F_CF, F_CF);
    }
    IFX (X_PEND) {
        STB(s1, offsetof(x64emu_t, res), xEmu);
    }
    ANDId(s1, s1, 0xff);
    IFX (X_SF) {
        SRDI(s3, s1, 7);
        BF_INSERT(xFlags, s3, F_SF, F_SF);
    }
    IFX (X_ZF) {
        // SEQZ: if s1==0 then s3=1, else s3=0
        CNTLZD(s3, s1);
        SRDI(s3, s3, 6);
        BF_INSERT(xFlags, s3, F_ZF, F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SHL16 instruction, from s1 , shift s2, store result in s1 using s3, s4 and s5 as scratch
void emit_shl16(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    // s2 is not 0 here and is 1..1f/3f
    IFX (X_PEND) {
        STH(s1, offsetof(x64emu_t, op1), xEmu);
        STH(s2, offsetof(x64emu_t, op2), xEmu);
        SET_DF(s4, d_shl16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    CLEAR_FLAGS(s3);
    IFX (X_OF) {
        SRDI(s3, s1, 14);
        SRDI(s5, s3, 1);
        XOR(s3, s3, s5);
        BF_INSERT(xFlags, s3, F_OF, F_OF);
    }
    SLD(s1, s1, s2);

    IFX (X_CF | X_OF) {
        SRDI(s5, s1, 16);
        ANDId(s5, s5, 1); // LSB == F_CF
        IFX (X_CF) {
            OR(xFlags, xFlags, s5);
        }
    }

    SLDI(s1, s1, 48);
    IFX (X_SF) {
        CMPDI(s1, 0);
        BGE(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    SRDI(s1, s1, 48);

    IFX (X_PEND) {
        STH(s1, offsetof(x64emu_t, res), xEmu);
    }
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SHL16 instruction, from s1 , constant c, store result in s1 using s3, s4 and s5 as scratch
void emit_shl16c(dynarec_ppc64le_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4, int s5)
{
    if (!c) return;
    // c != 0

    IFX (X_PEND) {
        MOV64x(s3, c);
        STH(s3, offsetof(x64emu_t, op2), xEmu);
        STH(s1, offsetof(x64emu_t, op1), xEmu);
        SET_DF(s4, d_shl16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    CLEAR_FLAGS(s3);

    IFX (X_OF) {
        SRDI(s3, s1, 14);
        SRDI(s5, s3, 1);
        XOR(s3, s3, s5);
        BF_INSERT(xFlags, s3, F_OF, F_OF);
    }

    if (c < 16) {
        IFX (X_CF) {
            SRDI(s3, s1, 16 - c);
            BF_INSERT(xFlags, s3, F_CF, F_CF);
        }

        SLDI(s1, s1, c + 48);
        IFX (X_SF) {
            // extract sign bit (bit 63 after shift)
            SRDI(s3, s1, 63);
            BF_INSERT(xFlags, s3, F_SF, F_SF);
        }
        SRDI(s1, s1, 48);

        IFX (X_PEND) {
            STH(s1, offsetof(x64emu_t, res), xEmu);
        }
        IFX (X_ZF) {
            CMPDI(s1, 0);
            BNE(8);
            ORI(xFlags, xFlags, 1 << F_ZF);
        }
        IFX (X_PF) {
            emit_pf(dyn, ninst, s1, s3, s4);
        }
        if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
    } else {
        IFXA (X_CF, c == 16) {
            ANDId(s3, s1, 1);
            OR(xFlags, xFlags, s3); // F_CF == 0
        }
        LI(s1, 0);

        IFX (X_PEND) {
            STH(s1, offsetof(x64emu_t, res), xEmu);
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
void emit_shl32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    // s2 is not 0 here and is 1..1f/3f
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SDxw(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, rex.w ? d_shl64 : d_shl32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    CLEAR_FLAGS(s3);
    IFX (X_OF) {
        SRLIxw(s3, s1, rex.w ? 62 : 30);
        SRDI(s5, s3, 1);
        XOR(s3, s3, s5);
        BF_INSERT(xFlags, s3, F_OF, F_OF);
    }
    IFX (X_CF) {
        ADDI(s5, s2, rex.w ? -64 : -32);
        NEG(s5, s5);
        SRD(s3, s1, s5);
        BF_INSERT(xFlags, s3, F_CF, F_CF);
    }
    if (rex.w) {
        SLD(s1, s1, s2);
    } else {
        SLW(s1, s1, s2);
    }
    IFX (X_SF) {
        SRDI(s3, s1, rex.w ? 63 : 31);
        CMPDI(s3, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) ZEROUP(s1);
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SHL32 instruction, from s1 , constant c, store result in s1 using s3, s4 and s5 as scratch
void emit_shl32c(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4, int s5)
{
    if (!c) return;

    IFX (X_PEND) {
        if (c) {
            MOV64x(s3, c);
            SDxw(s3, xEmu, offsetof(x64emu_t, op2));
        } else {
            LI(s3, 0);
            SDxw(s3, xEmu, offsetof(x64emu_t, op2));
        }
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, rex.w ? d_shl64 : d_shl32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    CLEAR_FLAGS(s3);
    IFX (X_OF) {
        SRLIxw(s3, s1, rex.w ? 62 : 30);
        SRDI(s5, s3, 1);
        XOR(s3, s3, s5);
        BF_INSERT(xFlags, s3, F_OF, F_OF);
    }
    IFX (X_CF) {
        SRDI(s3, s1, (rex.w ? 64 : 32) - c);
        BF_INSERT(xFlags, s3, F_CF, F_CF);
    }

    if (rex.w) {
        SLDI(s1, s1, c);
    } else {
        SLWI(s1, s1, c);
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
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SHR8 instruction, from s1 , shift s2 (!0 and and'd already), store result in s1 using s3 and s4 as scratch
void emit_shr8(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    IFX (X_PEND) {
        STB(s2, offsetof(x64emu_t, op2), xEmu);
        STB(s1, offsetof(x64emu_t, op1), xEmu);
        SET_DF(s4, d_shr8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    CLEAR_FLAGS(s3);
    IFX (X_CF) {
        ADDI(s3, s2, -1);
        SRAD(s3, s1, s3);
        ANDId(s3, s3, 1); // LSB == F_CF
        OR(xFlags, xFlags, s3);
    }
    IFX (X_OF) {
        // OF flag is set to the most-significant bit of the original operand
        SRDI(s3, s1, 7);
        BF_INSERT(xFlags, s3, F_OF, F_OF);
    }

    SRD(s1, s1, s2);
    ANDId(s1, s1, 0xff);

    // SF should be unset
    IFX (X_PEND) {
        STB(s1, offsetof(x64emu_t, res), xEmu);
    }
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SHR8 instruction, from s1 , constant c, store result in s1 using s3, s4 as scratch
void emit_shr8c(dynarec_ppc64le_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4)
{
    if (!c) return;
    IFX (X_PEND) {
        MOV64x(s3, c);
        STB(s3, offsetof(x64emu_t, op2), xEmu);
        STB(s1, offsetof(x64emu_t, op1), xEmu);
        SET_DF(s4, d_shr8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    CLEAR_FLAGS(s3);
    IFX (X_CF) {
        if (c == 1) {
            BF_INSERT(xFlags, s1, F_CF, F_CF);
        } else {
            BF_EXTRACT(s3, s1, c - 1, c - 1);
            BF_INSERT(xFlags, s3, F_CF, F_CF);
        }
    }
    IFX (X_OF) {
        BF_EXTRACT(s3, s1, 7, 7);
        BF_INSERT(xFlags, s3, F_OF, F_OF);
    }

    SRDI(s1, s1, c);
    ANDId(s1, s1, 0xff);

    IFX (X_PEND) {
        STB(s1, offsetof(x64emu_t, res), xEmu);
    }
    IFX (X_ZF) {
        // SEQZ: if s1==0 then s4=1, else s4=0
        CNTLZD(s4, s1);
        SRDI(s4, s4, 6);
        BF_INSERT(xFlags, s4, F_ZF, F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SHR16 instruction, from s1 , shift s2 (!0 and and'd already), store result in s1 using s3 and s4 as scratch
void emit_shr16(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    IFX (X_PEND) {
        STH(s2, offsetof(x64emu_t, op2), xEmu);
        STH(s1, offsetof(x64emu_t, op1), xEmu);
        SET_DF(s4, d_shr16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    CLEAR_FLAGS(s3);
    IFX (X_CF) {
        ADDI(s3, s2, -1);
        SRAD(s3, s1, s3);
        ANDId(s3, s3, 1); // LSB == F_CF
        OR(xFlags, xFlags, s3);
    }
    IFX (X_OF) {
        // OF flag is set to the most-significant bit of the original operand
        SRDI(s3, s1, 15);
        SLDI(s3, s3, F_OF);
        OR(xFlags, xFlags, s3);
    }

    SRD(s1, s1, s2);
    BF_EXTRACT(s1, s1, 15, 0);

    // SF should be unset
    IFX (X_PEND) {
        STH(s1, offsetof(x64emu_t, res), xEmu);
    }
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SHR16 instruction, from s1 , constant c, store result in s1 using s3, s4 and s5 as scratch
void emit_shr16c(dynarec_ppc64le_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4, int s5)
{
    if (!c) return;
    IFX (X_PEND) {
        MOV64x(s3, c);
        STH(s3, offsetof(x64emu_t, op2), xEmu);
        STH(s1, offsetof(x64emu_t, op1), xEmu);
        SET_DF(s4, d_shr16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    CLEAR_FLAGS(s3);
    IFX (X_CF) {
        if (c > 1) {
            SRADI(s3, s1, c - 1);
            ANDId(s3, s3, 1); // LSB == F_CF
        } else {
            // no need to shift
            ANDId(s3, s1, 1); // LSB == F_CF
        }
        OR(xFlags, xFlags, s3);
    }
    IFX (X_OF) {
        // OF flag is set to the most-significant bit of the original operand
        SRDI(s3, s1, 15);
        SLDI(s3, s3, F_OF);
        OR(xFlags, xFlags, s3);
    }

    SRDI(s1, s1, c);

    // SF should be unset
    IFX (X_PEND) {
        STH(s1, offsetof(x64emu_t, res), xEmu);
    }
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SHR32 instruction, from s1 , shift s2 (!0 and and'd already), store result in s1 using s3 and s4 as scratch
void emit_shr32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4)
{
    IFX (X_PEND) {
        SDxw(s2, xEmu, offsetof(x64emu_t, op2));
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, rex.w ? d_shr64 : d_shr32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    CLEAR_FLAGS(s3);
    IFX (X_CF) {
        ADDI(s3, s2, -1);
        SRAD(s3, s1, s3);
        ANDId(s3, s3, 1); // LSB == F_CF
        OR(xFlags, xFlags, s3);
    }
    IFX (X_OF) {
        // OF flag is set to the most-significant bit of the original operand
        SRLIxw(s3, s1, rex.w ? 63 : 31);
        BF_INSERT(xFlags, s3, F_OF, F_OF);
    }

    SRD(s1, s1, s2);

    IFX (X_SF) {
        CMPDI(s1, 0);
        BGE(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SHR32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_shr32c(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4)
{
    if (!c) return;
    IFX (X_PEND) {
        if (c) {
            MOV64x(s3, c);
            SDxw(s3, xEmu, offsetof(x64emu_t, op2));
        } else {
            LI(s3, 0);
            SDxw(s3, xEmu, offsetof(x64emu_t, op2));
        }
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, rex.w ? d_shr64 : d_shr32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    CLEAR_FLAGS(s3);
    IFX (X_CF) {
        if (c > 1) {
            SRADI(s3, s1, c - 1);
            ANDId(s3, s3, 1); // LSB == F_CF
        } else {
            // no need to shift
            ANDId(s3, s1, 1); // LSB == F_CF
        }
        OR(xFlags, xFlags, s3);
    }
    IFX (X_OF) {
        // OF flag is set to the most-significant bit of the original operand
        SRLIxw(s3, s1, rex.w ? 63 : 31);
        BF_INSERT(xFlags, s3, F_OF, F_OF);
    }

    if (rex.w) {
        SRDI(s1, s1, c);
    } else {
        SRWI(s1, s1, c); // no need to zeroup
    }

    IFX (X_SF) {
        CMPDI(s1, 0);
        BGE(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SAR8 instruction, from s1 , shift s2 (!0 and and'd already), store result in s1 using s3, s4 and s5 as scratch
void emit_sar8(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    IFX (X_PEND) {
        STB(s2, offsetof(x64emu_t, op2), xEmu);
        STB(s1, offsetof(x64emu_t, op1), xEmu);
        SET_DF(s4, d_sar8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    CLEAR_FLAGS(s3);
    IFX (X_CF) {
        ADDI(s3, s2, -1);
        SRAD(s3, s1, s3);
        ANDId(s3, s3, 1); // LSB == F_CF
        OR(xFlags, xFlags, s3);
    }
    // For the SAR instruction, the OF flag is cleared for all 1-bit shifts.
    // OF nop
    IFX (X_SF) {
        // SF is the same as the original operand
        CMPDI(s1, 0);
        BGE(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }

    SRAD(s1, s1, s2);
    ANDId(s1, s1, 0xff);

    IFX (X_PEND) {
        STB(s1, offsetof(x64emu_t, res), xEmu);
    }
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SAR8 instruction, from s1 , constant c, store result in s1 using s3, s4 as scratch
void emit_sar8c(dynarec_ppc64le_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4)
{
    if (!c) return;
    IFX (X_PEND) {
        MOV64x(s3, c);
        STB(s3, offsetof(x64emu_t, op2), xEmu);
        STB(s1, offsetof(x64emu_t, op1), xEmu);
        SET_DF(s4, d_sar8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    CLEAR_FLAGS(s3);
    IFXA (X_CF, c < 8) {
        BF_EXTRACT(s3, s1, c - 1, c - 1);
        BF_INSERT(xFlags, s3, F_CF, F_CF);
    }

    SRADI(s1, s1, c);
    ANDId(s1, s1, 0xff);
    IFX (X_PEND) {
        STB(s1, offsetof(x64emu_t, res), xEmu);
    }
    if (c < 8) {
        IFX (X_ZF) {
            // SEQZ
            CNTLZD(s3, s1);
            SRDI(s3, s3, 6);
            BF_INSERT(xFlags, s3, F_ZF, F_ZF);
        }
        IFX (X_SF) {
            SRDI(s3, s1, 7);
            BF_INSERT(xFlags, s3, F_SF, F_SF);
        }
        IFX (X_PF) {
            emit_pf(dyn, ninst, s1, s3, s4);
        }
    } else {
        IFX (X_CF | X_ZF | X_SF | X_PF) {
            SRDI(s3, s1, 7);
            IFX (X_CF) BF_INSERT(xFlags, s3, F_CF, F_CF);
            IFX (X_SF) BF_INSERT(xFlags, s3, F_SF, F_SF);
            IFX (X_ZF) {
                // SEQZ
                CNTLZD(s4, s1);
                SRDI(s4, s4, 6);
                BF_INSERT(xFlags, s4, F_ZF, F_ZF);
            }
            IFX (X_PF) ORI(xFlags, xFlags, 1 << F_PF);
        }
    }

    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SAR16 instruction, from s1, shift s2 (!0 and and'd already), store result in s1 using s3, s4 and s5 as scratch
void emit_sar16(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    IFX (X_PEND) {
        STH(s2, offsetof(x64emu_t, op2), xEmu);
        STH(s1, offsetof(x64emu_t, op1), xEmu);
        SET_DF(s4, d_sar16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    CLEAR_FLAGS(s3);
    IFX (X_CF) {
        ADDI(s3, s2, -1);
        SRAD(s3, s1, s3);
        ANDId(s3, s3, 1); // LSB == F_CF
        OR(xFlags, xFlags, s3);
    }
    // For the SAR instruction, the OF flag is cleared for all 1-bit shifts.
    // OF nop
    IFX (X_SF) {
        // SF is the same as the original operand
        CMPDI(s1, 0);
        BGE(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }

    SRD(s1, s1, s2);
    BF_EXTRACT(s1, s1, 15, 0);

    IFX (X_PEND) {
        STH(s1, offsetof(x64emu_t, res), xEmu);
    }
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SAR16 instruction, from s1 , constant c, store result in s1 using s3, s4 and s5 as scratch
void emit_sar16c(dynarec_ppc64le_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4, int s5)
{
    if (!c) return;
    IFX (X_PEND) {
        MOV64x(s3, c);
        STH(s3, offsetof(x64emu_t, op2), xEmu);
        STH(s1, offsetof(x64emu_t, op1), xEmu);
        SET_DF(s4, d_sar16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    CLEAR_FLAGS(s3);
    IFX (X_CF) {
        if (c > 1) {
            SRADI(s3, s1, c - 1);
            ANDId(s3, s3, 1); // LSB == F_CF
        } else {
            // no need to shift
            ANDId(s3, s1, 1); // LSB == F_CF
        }
        OR(xFlags, xFlags, s3);
    }
    // For the SAR instruction, the OF flag is cleared for all 1-bit shifts.
    // OF nop
    IFX (X_SF) {
        // SF is the same as the original operand
        CMPDI(s1, 0);
        BGE(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }

    SRDI(s1, s1, c);
    BF_EXTRACT(s1, s1, 15, 0);

    IFX (X_PEND) {
        STH(s1, offsetof(x64emu_t, res), xEmu);
    }
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SAR32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_sar32c(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4)
{
    IFX (X_PEND) {
        if (c) {
            MOV64x(s3, c);
            SDxw(s3, xEmu, offsetof(x64emu_t, op2));
        } else {
            LI(s3, 0);
            SDxw(s3, xEmu, offsetof(x64emu_t, op2));
        }
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

    CLEAR_FLAGS(s3);
    IFX (X_CF) {
        if (c > 1) {
            SRADI(s3, s1, c - 1);
            ANDId(s3, s3, 1); // LSB == F_CF
        } else {
            // no need to shift
            ANDId(s3, s1, 1); // LSB == F_CF
        }
        OR(xFlags, xFlags, s3);
    }
    // For the SAR instruction, the OF flag is cleared for all 1-bit shifts.
    // OF nop

    if (rex.w) {
        SRADI(s1, s1, c);
    } else {
        SRAWI(s1, s1, c);
    }

    // SRAWI sign-extends, so test sign bit before clearing upper bits
    IFX (X_SF) {
        CMPDI(s1, 0);
        BGE(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SAR32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_sar32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4)
{
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SDxw(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, rex.w ? d_sar64 : d_sar32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    CLEAR_FLAGS(s3);
    IFX (X_CF) {
        ADDIxw(s3, s2, -1);
        SRAxw(s3, s1, s3);
        BF_INSERT(xFlags, s3, F_CF, F_CF);
    }
    if (rex.w)
        SRAD(s1, s1, s2);
     else
        SRAW(s1, s1, s2);

    IFX (X_SF) {
        CMPDI(s1, 0);
        BGE(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    if (!rex.w) ZEROUP(s1);
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit ROL8 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_rol8(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    // RESTORE_EFLAGS is a no-op on PPC64LE

    IFX (X_OF) {
        SRWI(s3, s1, 6);
        SRDI(s4, s3, 1);
        XOR(s3, s3, s4);
        BF_INSERT(xFlags, s3, F_OF, F_OF);
    }
    LI(s4, 8);
    SUB(s4, s4, s2);
    BF_INSERT(s1, s1, 15, 8);
    SRD(s1, s1, s4);
    IFX (X_CF) {
        BF_INSERT(xFlags, s1, F_CF, F_CF);
    }

    if (dyn->insts[ninst].nat_flags_fusion) {
        ANDId(s1, s1, 0xFF);
        NAT_FLAGS_OPS(s1, xZR, s3, xZR);
    }
}

// emit ROL8 instruction, from s1, c, store result in s1 using s3 s4 and s5 as scratch
void emit_rol8c(dynarec_ppc64le_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4, int s5)
{
    if (!c) return;

    IFX (X_OF) {
        SRWI(s3, s1, 6);
        SRDI(s4, s3, 1);
        XOR(s3, s3, s4);
        BF_INSERT(xFlags, s3, F_OF, F_OF);
    }
    if (c & 7) {
        SLDI(s3, s1, (c & 7));
        SRDI(s1, s1, 8 - (c & 7));
        OR(s1, s3, s1);
    }

    IFX (X_CF) {
        BF_INSERT(xFlags, s1, F_CF, F_CF);
    }
    if (dyn->insts[ninst].nat_flags_fusion) {
        ANDId(s1, s1, 0xFF);
        NAT_FLAGS_OPS(s1, xZR, s3, xZR);
    }
}

// emit ROL16 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_rol16(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    // RESTORE_EFLAGS is a no-op on PPC64LE

    ANDId(s2, s2, 0xf);
    IFX (X_OF) {
        SRWI(s3, s1, 14);
        SRDI(s4, s3, 1);
        XOR(s3, s3, s4);
        BF_INSERT(xFlags, s3, F_OF, F_OF);
    }

    LI(s4, 16);
    SUB(s4, s4, s2);
    BF_INSERT(s1, s1, 31, 16);
    SRD(s1, s1, s4);
    IFX (X_CF) {
        BF_INSERT(xFlags, s1, F_CF, F_CF);
    }

    if (dyn->insts[ninst].nat_flags_fusion) {
        BF_EXTRACT(s1, s1, 15, 0);
        NAT_FLAGS_OPS(s1, xZR, s3, xZR);
    }
}

// emit ROL16 instruction, from s1, c, store result in s1 using s3 s4 and s5 as scratch
void emit_rol16c(dynarec_ppc64le_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4, int s5)
{
    if (!c) return;
    // RESTORE_EFLAGS is a no-op on PPC64LE

    IFXA (X_OF, c == 1) {
        SRDI(s3, s1, 14);
        SRDI(s4, s3, 1);
        XOR(s3, s4, s3);
        BF_INSERT(xFlags, s3, F_OF, F_OF);
    }

    if (c & 15) {
        int rc = 16 - (c & 15);
        // no LBT ROTRI_H — do it manually
        SLDI(s3, s1, 16);
        OR(s1, s3, s1);
        SRDI(s1, s1, rc);
    }

    IFX (X_CF) BF_INSERT(xFlags, s1, F_CF, F_CF);

    if (dyn->insts[ninst].nat_flags_fusion) {
        BF_EXTRACT(s1, s1, 15, 0);
        NAT_FLAGS_OPS(s1, xZR, s3, xZR);
    }
}

// emit ROL32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_rol32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4)
{
    IFX (X_CF | X_OF) {
        MOV64x(s4, ((1UL << F_CF) | (1UL << F_OF)));
        ANDC(xFlags, xFlags, s4);
    }

    IFX (X_OF) {
        SRLIxw(s3, s1, rex.w ? 62 : 30);
        SRDI(s4, s3, 1);
        XOR(s3, s3, s4);
        BF_INSERT(xFlags, s3, F_OF, F_OF);
    }

    // ROL = rotate left by s2.  PPC64LE has native rotate-left.
    if (rex.w) {
        // 64-bit: RLDCL(Ra, Rs, Rb, 0) = rotate left doubleword then clear left (mb=0 means no masking)
        RLDCL(s1, s1, s2, 0);
    } else {
        // 32-bit: RLWNM(Ra, Rs, Rb, 0, 31)
        RLWNM(s1, s1, s2, 0, 31);
    }
    if (!rex.w) ZEROUP(s1);

    IFX (X_CF) {
        ANDId(s4, s1, 1); // LSB == F_CF
        OR(xFlags, xFlags, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit ROL32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_rol32c(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4)
{
    if (!c) return;

    IFX (X_CF | X_OF) {
        MOV64x(s3, ((1UL << F_CF) | (1UL << F_OF)));
        ANDC(xFlags, xFlags, s3);
    }

    IFX (X_OF) {
        SRLIxw(s3, s1, rex.w ? 62 : 30);
        SRDI(s4, s3, 1);
        XOR(s3, s3, s4);
        BF_INSERT(xFlags, s3, F_OF, F_OF);
    }

    // ROL by constant c: rotate left by c
    if (rex.w) {
        RLDICL(s1, s1, c, 0);
    } else {
        RLWINM(s1, s1, c, 0, 31);
    }
    if (!rex.w) ZEROUP(s1);

    IFX (X_CF) {
        ANDId(s4, s1, 1 << F_CF);
        OR(xFlags, xFlags, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit ROR8 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_ror8(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    // RESTORE_EFLAGS is a no-op on PPC64LE

    IFX (X_OF) {
        SRDI(s3, s1, 7);
        XOR(s3, s1, s3);
        BF_INSERT(xFlags, s3, F_OF, F_OF);
    }

    BF_INSERT(s1, s1, 15, 8);
    SRD(s1, s1, s2);

    IFX (X_CF) {
        SRDI(s3, s1, 7);
        BF_INSERT(xFlags, s3, F_CF, F_CF);
    }

    if (dyn->insts[ninst].nat_flags_fusion) {
        ANDId(s1, s1, 0xFF);
        NAT_FLAGS_OPS(s1, xZR, s3, xZR);
    }
}

// emit ROR8 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_ror8c(dynarec_ppc64le_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4)
{
    if (!c) return;
    // RESTORE_EFLAGS is a no-op on PPC64LE

    IFX (X_OF) {
        SRDI(s3, s1, 7);
        XOR(s3, s1, s3);
        BF_INSERT(xFlags, s3, F_OF, F_OF);
    }

    if (c & 7) {
        BF_INSERT(s1, s1, 15, 8);
        SRDI(s1, s1, c & 7);
    }
    IFX (X_CF) {
        SRDI(s3, s1, 7);
        BF_INSERT(xFlags, s3, F_CF, F_CF);
    }

    if (dyn->insts[ninst].nat_flags_fusion) {
        ANDId(s1, s1, 0xFF);
        NAT_FLAGS_OPS(s1, xZR, s3, xZR);
    }
}

// emit ROR16 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_ror16(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    // RESTORE_EFLAGS is a no-op on PPC64LE

    ANDId(s2, s2, 0xf);
    IFX (X_OF) {
        SRDI(s3, s1, 15);
        XOR(s3, s1, s3);
        BF_INSERT(xFlags, s3, F_OF, F_OF);
    }

    BF_INSERT(s1, s1, 31, 16);
    SRD(s1, s1, s2);

    IFX (X_CF) {
        SRDI(s3, s1, 15);
        BF_INSERT(xFlags, s3, F_CF, F_CF);
    }

    if (dyn->insts[ninst].nat_flags_fusion) {
        BF_EXTRACT(s1, s1, 15, 0);
        NAT_FLAGS_OPS(s1, xZR, s3, xZR);
    }
}

// emit ROR16 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_ror16c(dynarec_ppc64le_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4)
{
    if (!c) return;
    // RESTORE_EFLAGS is a no-op on PPC64LE
    IFXA (X_OF, (c == 1)) {
        SRDI(s3, s1, 15);
        XOR(s3, s3, s1);
        BF_INSERT(xFlags, s3, F_OF, F_OF);
    }

    if (c & 15) {
        // no LBT ROTRI_H — do it manually
        SRDI(s3, s1, c & 15);
        SLDI(s1, s1, 16 - (c & 15));
        OR(s1, s1, s3);
        BF_EXTRACT(s1, s1, 15, 0);
    }

    IFX (X_CF) {
        SRDI(s3, s1, 15);
        BF_INSERT(xFlags, s3, F_CF, F_CF);
    }

    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit ROR32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_ror32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4)
{
    IFX (X_CF | X_OF) {
        MOV64x(s4, ((1UL << F_CF) | (1UL << F_OF)));
        ANDC(xFlags, xFlags, s4);
    }

    IFX (X_OF) {
        SRLIxw(s3, s1, rex.w ? 63 : 31);
        XOR(s3, s3, s1);
        BF_INSERT(xFlags, s3, F_OF, F_OF);
    }

    // ROR by register: compute 64/32 - s2, then rotate left
    if (rex.w) {
        LI(s3, 64);
        SUB(s3, s3, s2);
        RLDCL(s1, s1, s3, 0);
    } else {
        LI(s3, 32);
        SUB(s3, s3, s2);
        RLWNM(s1, s1, s3, 0, 31);
    }
    if (!rex.w) ZEROUP(s1);

    IFX (X_CF) {
        SRLIxw(s3, s1, rex.w ? 63 : 31);
        OR(xFlags, xFlags, s3);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit ROR32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_ror32c(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4)
{
    if (!c) return;

    IFX (X_CF | X_OF) {
        MOV64x(s4, ((1UL << F_CF) | (1UL << F_OF)));
        ANDC(xFlags, xFlags, s4);
    }

    IFX (X_OF) {
        SRLIxw(s3, s1, rex.w ? 63 : 31);
        XOR(s3, s3, s1);
        BF_INSERT(xFlags, s3, F_OF, F_OF);
    }

    // ROR by constant c: rotate left by (width - c)
    if (rex.w) {
        RLDICL(s1, s1, 64 - c, 0);
    } else {
        RLWINM(s1, s1, 32 - c, 0, 31);
    }
    if (!rex.w) ZEROUP(s1);

    IFX (X_CF) {
        SRLIxw(s3, s1, rex.w ? 63 : 31);
        OR(xFlags, xFlags, s3);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit RCL8 instruction, from s1, s2, store result in s1 using s3, s4 and s5 as scratch
void emit_rcl8(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    // RESTORE_EFLAGS is a no-op on PPC64LE
    int64_t j64;

    LI(s4, 9);
    MOD_DU(s2, s2, s4);
    CBZ_NEXT(s2);

    IFX (X_OF) {
        SRDI(s3, s1, 6);
        SRDI(s5, s3, 1);
        XOR(s3, s3, s5);
        BF_INSERT(xFlags, s3, F_OF, F_OF);
    }

    BF_INSERT(s1, xFlags, 8, 8); // insert CF to bit 8
    BF_INSERT(s1, s1, 16, 9);
    SUB(s2, s4, s2);
    IFX (X_CF) {
        ADDI(s5, s2, -1);
        SRD(s5, s1, s5);
        BF_INSERT(xFlags, s5, F_CF, F_CF);
    }
    SRD(s1, s1, s2);

    if (dyn->insts[ninst].nat_flags_fusion) {
        ANDId(s1, s1, 0xFF);
        NAT_FLAGS_OPS(s1, xZR, s3, xZR);
    }
}

// emit RCL8 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_rcl8c(dynarec_ppc64le_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4)
{
    c %= 9;
    if (!c) return;

    // RESTORE_EFLAGS is a no-op on PPC64LE

    BF_INSERT(s1, xFlags, 8, 8); // insert CF to bit 8
    IFX (X_OF | X_CF) {
        BF_EXTRACT(s3, s1, 8 - c, 8 - c);
        BF_INSERT(xFlags, s3, F_CF, F_CF);
    }

    IFX (X_OF) {
        SRDI(s3, s1, 6);
        SRDI(s4, s3, 1);
        XOR(s3, s3, s4);
        BF_INSERT(xFlags, s3, F_OF, F_OF);
    }

    BF_INSERT(s1, s1, 16, 9);
    SRDI(s1, s1, 9 - c);

    if (dyn->insts[ninst].nat_flags_fusion) {
        ANDId(s1, s1, 0xFF);
        NAT_FLAGS_OPS(s1, xZR, s3, xZR);
    }
}

// emit RCL16 instruction, from s1, s2, store result in s1 using s3, s4 and s5 as scratch
void emit_rcl16(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    // RESTORE_EFLAGS is a no-op on PPC64LE
    int64_t j64;

    LI(s4, 17);
    MOD_DU(s2, s2, s4);
    CBZ_NEXT(s2);

    IFX (X_OF) {
        SRDI(s3, s1, 14);
        SRDI(s5, s3, 1);
        XOR(s3, s3, s5);
        BF_INSERT(xFlags, s3, F_OF, F_OF);
    }

    BF_INSERT(s1, xFlags, 16, 16); // insert CF to bit 16
    BF_INSERT(s1, s1, 32, 17);
    SUB(s2, s4, s2);
    IFX (X_CF) {
        ADDI(s5, s2, -1);
        SRD(s5, s1, s5);
        BF_INSERT(xFlags, s5, F_CF, F_CF);
    }
    SRD(s1, s1, s2);
    if (dyn->insts[ninst].nat_flags_fusion) {
        BF_EXTRACT(s1, s1, 15, 0);
        NAT_FLAGS_OPS(s1, xZR, s3, xZR);
    }
}

// emit RCL16 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_rcl16c(dynarec_ppc64le_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4)
{
    c %= 17;
    if (!c) return;

    // RESTORE_EFLAGS is a no-op on PPC64LE

    BF_INSERT(s1, xFlags, 16, 16); // insert CF to bit 16
    IFX (X_CF) {
        BF_EXTRACT(s3, s1, 16 - c, 16 - c);
        BF_INSERT(xFlags, s3, F_CF, F_CF);
    }

    IFX (X_OF) {
        SRDI(s3, s1, 14);
        SRDI(s4, s3, 1);
        XOR(s3, s3, s4);
        BF_INSERT(xFlags, s3, F_OF, F_OF);
    }

    BF_INSERT(s1, s1, 32, 17);
    SRDI(s1, s1, 17 - c);

    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit RCL32 instruction, from s1, s2, store result in s1 using s3, s4 and s5 as scratch
void emit_rcl32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    // RESTORE_EFLAGS is a no-op on PPC64LE
    IFX (X_CF) {
        LI(s4, rex.w ? 64 : 32);
        SUB(s4, s4, s2);
        SRD(s3, s1, s4);
    }
    IFX (X_OF) {
        SRDI(s4, s1, rex.w ? 62 : 30);
        SRDI(s5, s4, 1);
        XOR(s4, s5, s4);
        BF_INSERT(xFlags, s4, F_OF, F_OF);
    }
    MR(s4, xFlags);
    BF_INSERT(s4, s1, rex.w ? 63 : 31, 1);
    ADDI(s5, s2, -1);
    CMPDI(s5, 0);
    BEQ(4 + 4 * 5); // goto label (skip 5 instructions)
    SLD(s4, s4, s5);
    LI(s5, rex.w ? 65 : 33);
    SUB(s2, s5, s2);
    if (rex.w)
        SRD(s1, s1, s2);
    else
        SRW(s1, s1, s2);
    OR(s4, s1, s4);
    // label
    if (rex.w)
        MR(s1, s4);
    else
        ZEROUP2(s1, s4);

    IFX (X_CF) {
        BF_INSERT(xFlags, s3, F_CF, F_CF);
    }

    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit RCL32 instruction, from s1 , constant c, store result in s1 using s3, s4 and s5 as scratch
void emit_rcl32c(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4, int s5)
{
    if (!c) return;

    // RESTORE_EFLAGS is a no-op on PPC64LE
    IFX (X_CF) {
        SRDI(s3, s1, (rex.w ? 64 : 32) - c);
    }

    IFX (X_OF) {
        SRDI(s4, s1, rex.w ? 62 : 30);
        SRDI(s5, s4, 1);
        XOR(s4, s5, s4);
        BF_INSERT(xFlags, s4, F_OF, F_OF);
    }
    if (c == 1) {
        SLDI(s1, s1, 1);
        BF_INSERT(s1, xFlags, 0, 0);
    } else {
        SLDI(s4, s1, c);
        BF_INSERT(s4, xFlags, c - 1, c - 1);
        if (rex.w)
            SRDI(s5, s1, 65 - c);
        else
            SRWI(s5, s1, 33 - c);
        OR(s1, s5, s4);
    }
    if (!rex.w) ZEROUP(s1);

    IFX (X_CF) {
        BF_INSERT(xFlags, s3, F_CF, F_CF);
    }

    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit RCR8 instruction, from s1, s2, store result in s1 using s3, s4 and s5 as scratch
void emit_rcr8(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    // RESTORE_EFLAGS is a no-op on PPC64LE
    int64_t j64;

    LI(s4, 9);
    MOD_DU(s2, s2, s4);
    CBZ_NEXT(s2);

    BF_INSERT(s1, xFlags, 8, 8); // insert CF to bit 8
    BF_INSERT(s1, s1, 16, 9);

    IFX (X_OF) {
        SRDI(s3, s1, 7);
        XOR(s3, s3, xFlags);
        BF_INSERT(xFlags, s3, F_OF, F_OF);
    }
    IFX (X_CF) {
        ADDI(s4, s2, -1);
        SRD(s4, s1, s4);
        BF_INSERT(xFlags, s4, F_CF, F_CF);
    }
    SRD(s1, s1, s2);

    if (dyn->insts[ninst].nat_flags_fusion) {
        ANDId(s1, s1, 0xFF);
        NAT_FLAGS_OPS(s1, xZR, s3, xZR);
    }
}

// emit RCR8 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_rcr8c(dynarec_ppc64le_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4)
{
    c %= 9;
    if (!c) return;

    // RESTORE_EFLAGS is a no-op on PPC64LE
    IFX (X_OF) {
        SRDI(s3, s1, 7);
        XOR(s3, s3, xFlags);
        BF_INSERT(xFlags, s3, F_OF, F_OF);
    }

    if (c) {
        BF_INSERT(s1, xFlags, 8, 8); // insert CF to bit 8
        IFX (X_CF) {
            BF_EXTRACT(s3, s1, c - 1, c - 1);
            BF_INSERT(xFlags, s3, F_CF, F_CF);
        }
        if (c > 1) {
            BF_INSERT(s1, s1, 16, 9);
        }
        SRDI(s1, s1, c);
    }

    if (dyn->insts[ninst].nat_flags_fusion) {
        ANDId(s1, s1, 0xFF);
        NAT_FLAGS_OPS(s1, xZR, s3, xZR);
    }
}

// emit RCR16 instruction, from s1, s2, store result in s1 using s3, s4 and s5 as scratch
void emit_rcr16(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    // RESTORE_EFLAGS is a no-op on PPC64LE
    int64_t j64;

    LI(s4, 17);
    MOD_DU(s2, s2, s4);
    CBZ_NEXT(s2);

    BF_INSERT(s1, xFlags, 16, 16); // insert CF to bit 16
    BF_INSERT(s1, s1, 32, 17);

    IFX (X_OF) {
        SRDI(s3, s1, 15);
        XOR(s3, s3, xFlags);
        BF_INSERT(xFlags, s3, F_OF, F_OF);
    }
    IFX (X_CF) {
        ADDI(s4, s2, -1);
        SRD(s4, s1, s4);
        BF_INSERT(xFlags, s4, F_CF, F_CF);
    }
    SRD(s1, s1, s2);

    if (dyn->insts[ninst].nat_flags_fusion) {
        BF_EXTRACT(s1, s1, 15, 0);
        NAT_FLAGS_OPS(s1, xZR, s3, xZR);
    }
}

// emit RCR16 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_rcr16c(dynarec_ppc64le_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4)
{
    c %= 17;
    if (!c) return;

    // RESTORE_EFLAGS is a no-op on PPC64LE
    IFX (X_OF) {
        SRDI(s3, s1, 15);
        XOR(s3, s3, xFlags);
        BF_INSERT(xFlags, s3, F_OF, F_OF);
    }

    if (c) {
        BF_INSERT(s1, xFlags, 16, 16); // insert CF to bit 16
        IFX (X_CF) {
            BF_EXTRACT(s3, s1, c - 1, c - 1);
            BF_INSERT(xFlags, s3, F_CF, F_CF);
        }
        if (c > 1) BF_INSERT(s1, s1, 32, 17);
        SRDI(s1, s1, c);
    }

    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit RCR32 instruction, from s1, s2, store result in s1 using s3, s4 and s5 as scratch
void emit_rcr32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    // RESTORE_EFLAGS is a no-op on PPC64LE

    IFX (X_OF) {
        SRDI(s3, s1, rex.w ? 63 : 31);
        XOR(s3, s3, xFlags);
        BF_INSERT(xFlags, s3, F_OF, F_OF);
    }
    ADDI(s5, s2, -1);
    IFX (X_CF) SRD(s3, s1, s5); // for later

    SRDI(s4, s1, 1);
    BF_INSERT(s4, xFlags, rex.w ? 63 : 31, rex.w ? 63 : 31); // insert cf
    CMPDI(s5, 0);
    BEQ(4 + 4 * 5);                                     // goto label (skip 5 instructions)
    SRD(s4, s4, s5);
    LI(s5, rex.w ? 65 : 33);
    SUB(s2, s5, s2);
    if (rex.w)
        SLD(s1, s1, s2);
    else
        SLW(s1, s1, s2);
    OR(s4, s1, s4);
    // label
    if (rex.w)
        MR(s1, s4);
    else
        ZEROUP2(s1, s4);

    IFX (X_CF) BF_INSERT(xFlags, s3, F_CF, F_CF);
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit RCR32 instruction, from s1 , constant c, store result in s1 using s3, s4 and s5 as scratch
void emit_rcr32c(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4, int s5)
{
    if (!c) return;

    // RESTORE_EFLAGS is a no-op on PPC64LE
    IFX (X_OF) {
        SRDI(s3, s1, rex.w ? 63 : 31);
        XOR(s3, s3, xFlags);
        BF_INSERT(xFlags, s3, F_OF, F_OF);
    }
    IFX (X_CF) BF_EXTRACT(s5, s1, c - 1, c - 1);
    if (c == 1) {
        SRDI(s1, s1, 1);
        BF_INSERT(s1, xFlags, rex.w ? 63 : 31, rex.w ? 63 : 31);
    } else {
        SRLIxw(s4, s1, c);
        BF_INSERT(s4, xFlags, (rex.w ? 64 : 32) - c, (rex.w ? 64 : 32) - c);
        SLDI(s3, s1, (rex.w ? 65 : 33) - c);
        OR(s1, s3, s4);
    }
    if (!rex.w) ZEROUP(s1);

    IFX (X_CF) {
        BF_INSERT(xFlags, s5, F_CF, F_CF);
    }

    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SHLD32 instruction, from s1, fill s2, constant c, store result in s1 using s3 and s4 as scratch
void emit_shld32c(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, uint32_t c, int s3, int s4)
{
    IFX (X_PEND) {
        if (c) {
            MOV64x(s3, c);
            SDxw(s3, xEmu, offsetof(x64emu_t, op2));
        } else {
            LI(s3, 0);
            SDxw(s3, xEmu, offsetof(x64emu_t, op2));
        }
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, rex.w ? d_shld64 : d_shld32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    CLEAR_FLAGS(s3);
    IFX (X_CF) {
        SRDI(s3, s1, (rex.w ? 64 : 32) - c);
        BF_INSERT(xFlags, s3, F_CF, F_CF);
    }
    IFX (X_OF) {
        SRLIxw(s3, s1, rex.w ? 62 : 30);
        SRDI(s4, s3, 1);
        XOR(s3, s3, s4);
        BF_INSERT(xFlags, s3, F_OF, F_OF);
    }

    SLLIxw(s3, s1, c);
    SRLIxw(s1, s2, (rex.w ? 64 : 32) - c);
    OR(s1, s1, s3);

    if (!rex.w) {
        ZEROUP(s1);
    }
    IFX (X_SF) {
        SRLIxw(s3, s1, rex.w ? 63 : 31);
        CMPDI(s3, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SHLD32 instruction, from s1, fill s2, shift s5, store result in s1 using s3, s4 and s6 as scratch
void emit_shld32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s5, int s3, int s4, int s6)
{
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SDxw(s5, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, rex.w ? d_shld64 : d_shld32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    CLEAR_FLAGS(s3);
    MOV32w(s3, (rex.w ? 64 : 32));
    SUB(s3, s3, s5);

    IFX (X_CF) {
        SRD(s4, s1, s3);
        BF_INSERT(xFlags, s4, F_CF, F_CF);
    }
    IFX (X_OF) {
        SRLIxw(s6, s1, rex.w ? 62 : 30);
        SRDI(s4, s6, 1);
        XOR(s6, s6, s4);
        BF_INSERT(xFlags, s6, F_OF, F_OF);
    }
    SLLxw(s4, s1, s5);
    SRLxw(s3, s2, s3);
    OR(s1, s3, s4);

    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    if (!rex.w) ZEROUP(s1);
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_SF) {
        SRLIxw(s3, s1, rex.w ? 63 : 31);
        CMPDI(s3, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SHRD32 instruction, from s1, fill s2 , constant c, store result in s1 using s3 and s4 as scratch
void emit_shrd32c(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, uint32_t c, int s3, int s4)
{
    IFX (X_PEND) {
        if (c) {
            MOV64x(s3, c);
            SDxw(s3, xEmu, offsetof(x64emu_t, op2));
        } else {
            LI(s3, 0);
            SDxw(s3, xEmu, offsetof(x64emu_t, op2));
        }
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, rex.w ? d_shrd64 : d_shrd32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    CLEAR_FLAGS(s3);
    IFX (X_CF) {
        int reg;
        if (c > 1) {
            SRADI(s3, s1, c - 1);
            reg = s3;
        } else
            reg = s1;
        BF_INSERT(xFlags, reg, F_CF, F_CF);
    }

    IFX (X_OF) {
        SRLIxw(s3, s1, rex.w ? 63 : 31);
        XOR(s3, s3, s2);
        BF_INSERT(xFlags, s3, F_OF, F_OF);
    }

    SRLIxw(s3, s1, c);
    SLLIxw(s1, s2, (rex.w ? 64 : 32) - c);
    OR(s1, s1, s3);

    if (!rex.w) {
        ZEROUP(s1);
    }
    IFX (X_SF) {
        SRLIxw(s3, s1, rex.w ? 63 : 31);
        CMPDI(s3, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SHRD32 instruction, from s1, fill s2, shift s5, store result in s1 using s3, s4 and s6 as scratch
void emit_shrd32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s5, int s3, int s4, int s6)
{
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SDxw(s5, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, rex.w ? d_shrd64 : d_shrd32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    CLEAR_FLAGS(s3);
    IFX (X_CF) {
        ADDI(s3, s5, -1);
        SRD(s3, s1, s3);
        BF_INSERT(xFlags, s3, F_CF, F_CF);
    }

    IFX (X_OF) {
        SRLIxw(s3, s1, rex.w ? 63 : 31);
        XOR(s3, s3, s2);
        BF_INSERT(xFlags, s3, F_OF, F_OF);
    }

    LI(s4, rex.w ? 64 : 32);
    SUB(s4, s4, s5);
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
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_SF) {
        SRLIxw(s3, s1, rex.w ? 63 : 31);
        CMPDI(s3, 0);
        BEQ(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SHLD16 instruction, from s1, fill s2, constant c, store result in s1 using s3, s4 and s5 as scratch
void emit_shld16c(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, uint32_t c, int s3, int s4, int s5)
{
    c &= 0x1f;
    IFX (X_PEND) {
        if (c) {
            MOV64x(s3, c);
            STH(s3, offsetof(x64emu_t, op2), xEmu);
        } else {
            LI(s3, 0);
            STH(s3, offsetof(x64emu_t, op2), xEmu);
        }
        STH(s1, offsetof(x64emu_t, op1), xEmu);
        SET_DF(s4, d_shld16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    if (!c) {
        IFX (X_PEND) {
            STH(s1, offsetof(x64emu_t, res), xEmu);
        }
        return;
    }

    CLEAR_FLAGS(s3);
    IFX (X_OF) {
        SRDI(s3, s1, 14);
        SRDI(s4, s3, 1);
        XOR(s3, s3, s4);
        BF_INSERT(xFlags, s3, F_OF, F_OF);
    }

    // create concat first
    SLDI(s5, s2, 16);
    OR(s1, s1, s5);

    IFX (X_CF) {
        if (c <= 16) {
            SRDI(s3, s1, 16 - c);
        } else {
            SRDI(s3, s2, 32 - c);
        }
        BF_INSERT(xFlags, s3, F_CF, F_CF);
    }
    SLDI(s3, s1, c);
    SRDI(s1, s1, 32 - c);
    OR(s1, s1, s3);
    BF_EXTRACT(s1, s1, 15, 0);

    IFX (X_PEND) {
        STH(s1, offsetof(x64emu_t, res), xEmu);
    }
    IFX (X_SF) {
        SLWI(s4, s1, 16);
        CMPDI(s4, 0);
        BGE(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }

    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SHLD16 instruction, from s1, fill s2, shift s3, store result in s1 using s4, s5 and s6 as scratch
void emit_shld16(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5, int s6)
{
    IFX (X_PEND) {
        STH(s1, offsetof(x64emu_t, op1), xEmu);
        STH(s3, offsetof(x64emu_t, op2), xEmu);
        SET_DF(s4, d_shld16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    CLEAR_FLAGS(s6);
    IFX (X_OF) {
        SRDI(s5, s1, 14);
        SRDI(s4, s5, 1);
        XOR(s5, s5, s4);
        ANDId(s5, s5, 1);
        SLDI(s5, s5, F_OF);
        OR(xFlags, xFlags, s5);
    }

    MOV32w(s6, 32);
    SUB(s6, s6, s3);
    IFX (X_CF) {
        SLDI(s5, s1, 16);
        OR(s4, s2, s5);
        SRW(s5, s4, s6);
        ANDId(s5, s5, 1); // LSB == F_CF
        OR(xFlags, xFlags, s5);
    }
    // create concat first
    SLDI(s5, s2, 16);
    OR(s1, s1, s5);

    SLW(s5, s1, s3);
    SRW(s1, s1, s6);
    OR(s1, s1, s5);
    BF_EXTRACT(s1, s1, 15, 0);

    IFX (X_PEND) {
        STH(s1, offsetof(x64emu_t, res), xEmu);
    }
    IFX (X_SF) {
        SLWI(s4, s1, 16);
        CMPDI(s4, 0);
        BGE(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s4, s5);
    }

    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SHRD16 instruction, from s1, fill s2, constant c, store result in s1 using s3, s4 and s5 as scratch
void emit_shrd16c(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, uint32_t c, int s3, int s4, int s5)
{
    c &= 0x1f;

    IFX (X_PEND) {
        if (c) {
            MOV64x(s3, c);
            STH(s3, offsetof(x64emu_t, op2), xEmu);
        } else {
            LI(s3, 0);
            STH(s3, offsetof(x64emu_t, op2), xEmu);
        }
        STH(s1, offsetof(x64emu_t, op1), xEmu);
        SET_DF(s4, d_shrd16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    if (!c) {
        IFX (X_PEND) {
            STH(s1, offsetof(x64emu_t, res), xEmu);
        }
        return;
    }
    CLEAR_FLAGS(s3);
    IFX (X_OF) {
        SRDI(s3, s1, 15);
        XOR(s3, s3, s2);
        BF_INSERT(xFlags, s3, F_OF, F_OF);
    }

    // create concat first
    SLDI(s5, s2, 16);
    OR(s1, s1, s5);

    IFX (X_CF) {
        if (c > 1) {
            SRADI(s3, s1, c - 1);
            BF_INSERT(xFlags, s3, F_CF, F_CF);
        } else {
            BF_INSERT(xFlags, s1, F_CF, F_CF);
        }
    }

    SRDI(s5, s1, c);
    SLDI(s1, s1, 32 - c);
    OR(s1, s1, s5);
    BF_EXTRACT(s1, s1, 15, 0);

    IFX (X_PEND) {
        STH(s1, offsetof(x64emu_t, res), xEmu);
    }
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_SF) {
        SLWI(s4, s1, 16);
        CMPDI(s4, 0);
        BGE(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SHRD16 instruction, from s1, fill s2, shift s3, store result in s1 using s4, s5 and s6 as scratch
void emit_shrd16(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5, int s6)
{
    IFX (X_PEND) {
        STH(s1, offsetof(x64emu_t, op1), xEmu);
        STH(s3, offsetof(x64emu_t, op2), xEmu);
        SET_DF(s4, d_shrd16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    CLEAR_FLAGS(s6);
    SLDI(s5, s2, 16);
    OR(s1, s1, s5);
    IFX (X_OF) {
        SRDI(s5, s1, 15);
        XOR(s5, s5, s2);
        ANDId(s5, s5, 1);
        SLDI(s5, s5, F_OF);
        OR(xFlags, xFlags, s5);
    }

    IFX (X_CF) {
        ADDI(s5, s3, -1);
        SRW(s5, s1, s5);
        ANDId(s5, s5, 1); // LSB == F_CF
        OR(xFlags, xFlags, s5);
    }

    MOV32w(s6, 32);
    SUB(s6, s6, s3);
    SRW(s5, s1, s3);
    SLW(s1, s1, s6);
    OR(s1, s1, s5);
    BF_EXTRACT(s1, s1, 15, 0);

    IFX (X_PEND) {
        STH(s1, offsetof(x64emu_t, res), xEmu);
    }
    IFX (X_ZF) {
        CMPDI(s1, 0);
        BNE(8);
        ORI(xFlags, xFlags, 1 << F_ZF);
    }
    IFX (X_SF) {
        SLWI(s4, s1, 16);
        CMPDI(s4, 0);
        BGE(8);
        ORI(xFlags, xFlags, 1 << F_SF);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s4, s5);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}
