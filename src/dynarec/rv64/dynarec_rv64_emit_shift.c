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

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_functions.h"
#include "../dynarec_helper.h"

// emit SHL8 instruction, from s1 , constant c, store result in s1 using s3, s4 and s5 as scratch
void emit_shl8c(dynarec_rv64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4, int s5)
{
    if (!c) return;
    // c != 0

    CLEAR_FLAGS();
    IFX (X_PEND) {
        MOV64x(s3, c);
        SB(s3, xEmu, offsetof(x64emu_t, op2));
        SB(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, d_shl8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFX (X_OF) {
        SRLI(s3, s1, 6);
        SRLI(s4, s3, 1);
        XOR(s3, s3, s4);
        ANDI(s3, s3, 1);
        SLLI(s3, s3, F_OF2);
        OR(xFlags, xFlags, s3);
    }

    if (c < 8) {
        IFX (X_CF) {
            SRLI(s3, s1, 8 - c);
            ANDI(s5, s3, 1); // LSB == F_CF
            OR(xFlags, xFlags, s5);
        }

        SLLI(s1, s1, c + 56);
        IFX (X_SF) {
            SET_FLAGS_LTZ(s1, F_SF, s3, s4);
        }
        SRLI(s1, s1, 56);

        IFX (X_PEND) {
            SB(s1, xEmu, offsetof(x64emu_t, res));
        }
        IFX (X_ZF) {
            SET_FLAGS_EQZ(s1, F_ZF, s3);
        }
        IFX (X_PF) {
            emit_pf(dyn, ninst, s1, s3, s4);
        }
        if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
    } else {
        IFX (X_CF) {
            if (c == 8) {
                ANDI(s3, s1, 1);
                OR(xFlags, xFlags, s3); // F_CF == 0
            }
        }
        MV(s1, xZR);

        IFX (X_PEND) {
            SB(s1, xEmu, offsetof(x64emu_t, res));
        }
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

// emit SHR8 instruction, from s1 , constant c, store result in s1 using s3, s4 and s5 as scratch
void emit_shr8c(dynarec_rv64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4, int s5)
{
    if (!c) return;
    // c != 0
    CLEAR_FLAGS();
    IFX (X_PEND) {
        MOV64x(s3, c);
        SB(s3, xEmu, offsetof(x64emu_t, op2));
        SB(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, d_shr8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    IFX (X_CF) {
        if (c > 1) {
            SRAI(s3, s1, c - 1);
            ANDI(s3, s3, 1); // LSB == F_CF
        } else {
            // no need to shift
            ANDI(s3, s1, 1); // LSB == F_CF
        }
        OR(xFlags, xFlags, s3);
    }
    IFX (X_OF) {
        SRLI(s3, s1, 7);
        SLLI(s3, s3, F_OF2);
        OR(xFlags, xFlags, s3);
    }

    SRLI(s1, s1, c);
    ANDI(s1, s1, 0xff);

    // SF should be unset
    IFX (X_PEND) {
        SB(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s5);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SAR8 instruction, from s1 , constant c, store result in s1 using s3, s4 and s5 as scratch
void emit_sar8c(dynarec_rv64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4, int s5)
{
    if (!c) return;
    // c != 0
    CLEAR_FLAGS();
    IFX (X_PEND) {
        MOV64x(s3, c);
        SB(s3, xEmu, offsetof(x64emu_t, op2));
        SB(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, d_sar8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    IFX (X_CF) {
        if (c > 1) {
            SRAI(s3, s1, c - 1);
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
        SET_FLAGS_LTZ(s1, F_SF, s3, s4);
    }

    SRLI(s1, s1, c);
    ANDI(s1, s1, 0xff);

    IFX (X_PEND) {
        SB(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s3);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SHL8 instruction, from s1 , shift s2, store result in s1 using s3, s4 and s5 as scratch
void emit_shl8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    // s2 is not 0 here and is 1..1f/3f
    CLEAR_FLAGS();
    IFX (X_PEND) {
        SB(s1, xEmu, offsetof(x64emu_t, op1));
        SB(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, d_shl8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }


    IFX (X_OF) {
        SRLI(s3, s1, 6);
        SRLI(s4, s3, 1);
        XOR(s3, s3, s4);
        ANDI(s3, s3, 1);
        SLLI(s3, s3, F_OF2);
        OR(xFlags, xFlags, s3);
    }

    SLL(s1, s1, s2);

    IFX (X_CF) {
        SRLI(s5, s1, 8);
        ANDI(s5, s5, 1); // LSB == F_CF
        OR(xFlags, xFlags, s5);
    }

    SLLI(s1, s1, 56);
    IFX (X_SF) {
        SET_FLAGS_LTZ(s1, F_SF, s3, s4);
    }
    SRLI(s1, s1, 56);

    IFX (X_PEND) {
        SB(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s3);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SHR8 instruction, from s1 , shift s2 (!0 and and'd already), store result in s1 using s3 and s4 as scratch
void emit_shr8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    int64_t j64;

    CLEAR_FLAGS();

    IFX (X_PEND) {
        SB(s2, xEmu, offsetof(x64emu_t, op2));
        SB(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, d_shr8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFX (X_CF) {
        SUBI(s3, s2, 1);
        SRA(s3, s1, s3);
        ANDI(s3, s3, 1); // LSB == F_CF
        OR(xFlags, xFlags, s3);
    }
    IFX (X_OF) {
        SRLI(s3, s1, 7);
        SLLI(s3, s3, F_OF2);
        OR(xFlags, xFlags, s3);
    }

    SRL(s1, s1, s2);
    ANDI(s1, s1, 0xff);

    // SF should be unset
    IFX (X_PEND) {
        SB(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s3);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SAR8 instruction, from s1 , shift s2 (!0 and and'd already), store result in s1 using s3, s4 and s5 as scratch
void emit_sar8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    int64_t j64;

    CLEAR_FLAGS();

    IFX (X_PEND) {
        SB(s2, xEmu, offsetof(x64emu_t, op2));
        SB(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, d_sar8);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    IFX (X_CF) {
        SUBI(s3, s2, 1);
        SRA(s3, s1, s3);
        ANDI(s3, s3, 1); // LSB == F_CF
        OR(xFlags, xFlags, s3);
    }
    // For the SAR instruction, the OF flag is cleared for all 1-bit shifts.
    // OF nop
    IFX (X_SF) {
        // SF is the same as the original operand
        SET_FLAGS_LTZ(s1, F_SF, s3, s4);
    }

    SRL(s1, s1, s2);
    ANDI(s1, s1, 0xff);

    IFX (X_PEND) {
        SB(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s3);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SHL16 instruction, from s1 , constant c, store result in s1 using s3, s4 and s5 as scratch
void emit_shl16c(dynarec_rv64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4, int s5)
{
    if (!c) return;
    // c != 0

    CLEAR_FLAGS();
    IFX (X_PEND) {
        MOV64x(s3, c);
        SH(s3, xEmu, offsetof(x64emu_t, op2));
        SH(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, d_shl16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFX (X_OF) {
        SRLI(s3, s1, 14);
        SRLI(s5, s3, 1);
        XOR(s3, s3, s5);
        ANDI(s3, s3, 1);
        SLLI(s3, s3, F_OF2);
        OR(xFlags, xFlags, s3);
    }

    if (c < 16) {
        IFX (X_CF) {
            SRLI(s3, s1, 16 - c);
            ANDI(s5, s3, 1); // LSB == F_CF
            OR(xFlags, xFlags, s5);
        }

        SLLI(s1, s1, c + 48);
        IFX (X_SF) {
            SET_FLAGS_LTZ(s1, F_SF, s3, s4);
        }
        SRLI(s1, s1, 48);

        IFX (X_PEND) {
            SH(s1, xEmu, offsetof(x64emu_t, res));
        }
        IFX (X_ZF) {
            SET_FLAGS_EQZ(s1, F_ZF, s3);
        }
        IFX (X_PF) {
            emit_pf(dyn, ninst, s1, s3, s4);
        }
        if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
    } else {
        IFXA (X_CF, (c == 16)) {
            ANDI(s3, s1, 1);
            OR(xFlags, xFlags, s3); // F_CF == 0
        }
        MV(s1, xZR);

        IFX (X_PEND) {
            SH(s1, xEmu, offsetof(x64emu_t, res));
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

// emit SHR16 instruction, from s1 , constant c, store result in s1 using s3, s4 and s5 as scratch
void emit_shr16c(dynarec_rv64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4, int s5)
{
    if (!c) return;
    // c != 0
    CLEAR_FLAGS();
    IFX (X_PEND) {
        MOV64x(s3, c);
        SH(s3, xEmu, offsetof(x64emu_t, op2));
        SH(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, d_shr16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    IFX (X_CF) {
        if (c > 1) {
            SRAI(s3, s1, c - 1);
            ANDI(s3, s3, 1); // LSB == F_CF
        } else {
            // no need to shift
            ANDI(s3, s1, 1); // LSB == F_CF
        }
        OR(xFlags, xFlags, s3);
    }
    IFX (X_OF) {
        // OF flag is set to the most-significant bit of the original operand
        SRLI(s3, s1, 15);
        SLLI(s3, s3, F_OF2);
        OR(xFlags, xFlags, s3);
    }

    SRLI(s1, s1, c);

    // SF should be unset

    IFX (X_PEND) {
        SH(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s3);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SAR16 instruction, from s1 , constant c, store result in s1 using s3, s4 and s5 as scratch
void emit_sar16c(dynarec_rv64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4, int s5)
{
    if (!c) return;
    // c != 0
    CLEAR_FLAGS();
    IFX (X_PEND) {
        MOV64x(s3, c);
        SH(s3, xEmu, offsetof(x64emu_t, op2));
        SH(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, d_sar16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    IFX (X_CF) {
        if (c > 1) {
            SRAI(s3, s1, c - 1);
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
        SET_FLAGS_LTZ(s1, F_SF, s3, s4);
    }

    SRLI(s1, s1, c);
    ZEXTH(s1, s1);

    IFX (X_PEND) {
        SH(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s3);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}


// emit SHL16 instruction, from s1 , shift s2, store result in s1 using s3, s4 and s5 as scratch
void emit_shl16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    // s2 is not 0 here and is 1..1f/3f
    CLEAR_FLAGS();
    IFX (X_PEND) {
        SH(s1, xEmu, offsetof(x64emu_t, op1));
        SH(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, d_shl16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFX (X_OF) {
        SRLI(s3, s1, 14);
        SRLI(s5, s3, 1);
        XOR(s3, s3, s5);
        ANDI(s3, s3, 1);
        SLLI(s3, s3, F_OF2);
        OR(xFlags, xFlags, s3);
    }

    SLL(s1, s1, s2);

    IFX (X_CF) {
        SRLI(s5, s1, 16);
        ANDI(s5, s5, 1); // LSB == F_CF
        OR(xFlags, xFlags, s5);
    }

    SLLI(s1, s1, 48);
    IFX (X_SF) {
        SET_FLAGS_LTZ(s1, F_SF, s3, s4);
    }
    SRLI(s1, s1, 48);

    IFX (X_PEND) {
        SH(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s3);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SHR16 instruction, from s1 , shift s2 (!0 and and'd already), store result in s1 using s3 and s4 as scratch
void emit_shr16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    int64_t j64;

    CLEAR_FLAGS();

    IFX (X_PEND) {
        SH(s2, xEmu, offsetof(x64emu_t, op2));
        SH(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, d_shr16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFX (X_CF) {
        SUBI(s3, s2, 1);
        SRA(s3, s1, s3);
        ANDI(s3, s3, 1); // LSB == F_CF
        OR(xFlags, xFlags, s3);
    }
    IFX (X_OF) {
        // OF flag is set to the most-significant bit of the original operand
        SRLI(s3, s1, 15);
        SLLI(s3, s3, F_OF2);
        OR(xFlags, xFlags, s3);
    }

    SRL(s1, s1, s2);
    ZEXTH(s1, s1);

    // SF should be unset
    IFX (X_PEND) {
        SH(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s3);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SAR16 instruction, from s1 , shift s2 (!0 and and'd already), store result in s1 using s3, s4 and s5 as scratch
void emit_sar16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    int64_t j64;

    CLEAR_FLAGS();

    IFX (X_PEND) {
        SH(s2, xEmu, offsetof(x64emu_t, op2));
        SH(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, d_sar16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    IFX (X_CF) {
        SUBI(s3, s2, 1);
        SRA(s3, s1, s3);
        ANDI(s3, s3, 1); // LSB == F_CF
        OR(xFlags, xFlags, s3);
    }
    // For the SAR instruction, the OF flag is cleared for all 1-bit shifts.
    // OF nop
    IFX (X_SF) {
        // SF is the same as the original operand
        SET_FLAGS_LTZ(s1, F_SF, s3, s4);
    }

    SRL(s1, s1, s2);
    ZEXTH(s1, s1);

    IFX (X_PEND) {
        SH(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s3);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SHL32 instruction, from s1 , shift s2, store result in s1 using s3, s4 and s5 as scratch
void emit_shl32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    // s2 is not 0 here and is 1..1f/3f
    CLEAR_FLAGS();
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SDxw(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, rex.w ? d_shl64 : d_shl32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFX (X_OF) {
        SRLIxw(s3, s1, rex.w ? 62 : 30);
        SRLI(s5, s3, 1);
        XOR(s3, s3, s5);
        ANDI(s3, s3, 1);
        SLLI(s3, s3, F_OF2);
        OR(xFlags, xFlags, s3);
    }
    IFX (X_CF) {
        SUBI(s5, s2, rex.w ? 64 : 32);
        NEG(s5, s5);
        SRL(s3, s1, s5);
        ANDI(s5, s3, 1); // LSB == F_CF
        OR(xFlags, xFlags, s5);
    }

    if (rex.w) {
        SLL(s1, s1, s2);
    } else {
        SLLW(s1, s1, s2);
    }

    IFX (X_SF) {
        SET_FLAGS_LTZ(s1, F_SF, s3, s4);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s3);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}
// emit SHL32 instruction, from s1 , constant c, store result in s1 using s3, s4 and s5 as scratch
void emit_shl32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4, int s5)
{
    if (!c) {
        if (!rex.w) ZEROUP(s1);
        return;
    }

    CLEAR_FLAGS();
    IFX (X_PEND) {
        MOV64x(s3, c);
        SDxw(s3, xEmu, offsetof(x64emu_t, op2));
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, rex.w ? d_shl64 : d_shl32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFX (X_OF) {
        SRLIxw(s3, s1, rex.w ? 62 : 30);
        SRLI(s5, s3, 1);
        XOR(s3, s3, s5);
        ANDI(s3, s3, 1);
        SLLI(s3, s3, F_OF2);
        OR(xFlags, xFlags, s3);
    }
    IFX (X_CF) {
        SRLI(s3, s1, (rex.w ? 64 : 32) - c);
        ANDI(s5, s3, 1); // LSB == F_CF
        OR(xFlags, xFlags, s5);
    }

    if (rex.w) {
        SLLI(s1, s1, c);
    } else {
        SLLIW(s1, s1, c);
    }

    IFX (X_SF) {
        SET_FLAGS_LTZ(s1, F_SF, s3, s4);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s3);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SHR32 instruction, from s1 , shift s2 (!0 and and'd already), store result in s1 using s3 and s4 as scratch
void emit_shr32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4)
{
    int64_t j64;

    CLEAR_FLAGS();

    IFX (X_PEND) {
        SDxw(s2, xEmu, offsetof(x64emu_t, op2));
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, rex.w ? d_shr64 : d_shr32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFX (X_CF) {
        SUBI(s3, s2, 1);
        SRA(s3, s1, s3);
        ANDI(s3, s3, 1); // LSB == F_CF
        OR(xFlags, xFlags, s3);
    }
    IFX (X_OF) {
        // OF flag is set to the most-significant bit of the original operand
        SRLIxw(s3, s1, rex.w ? 63 : 31);
        SLLI(s3, s3, F_OF2);
        OR(xFlags, xFlags, s3);
    }

    SRL(s1, s1, s2);

    IFX (X_SF) {
        SET_FLAGS_LTZ(s1, F_SF, s3, s4);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s3);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SHR32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_shr32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4)
{
    if (!c) return;

    CLEAR_FLAGS();

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
    IFX (X_CF) {
        if (c > 1) {
            SRAI(s3, s1, c - 1);
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
        SLLI(s3, s3, F_OF2);
        OR(xFlags, xFlags, s3);
    }

    if (rex.w) {
        SRLI(s1, s1, c);
    } else {
        SRLIW(s1, s1, c);
    }

    IFX (X_SF) {
        SET_FLAGS_LTZ(s1, F_SF, s3, s4);
    }
    if (!rex.w && c == 0) {
        ZEROUP(s1);
    }
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s3);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SAR32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_sar32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4)
{
    if (!c) {
        if (!rex.w) ZEROUP(s1);
        return;
    }

    CLEAR_FLAGS();

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
    IFX (X_CF) {
        if (c > 1) {
            SRAI(s3, s1, c - 1);
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
        SRAI(s1, s1, c);
    } else {
        SRAIW(s1, s1, c);
    }

    // SRAIW sign-extends, so test sign bit before clearing upper bits
    IFX (X_SF) {
        SET_FLAGS_LTZ(s1, F_SF, s3, s4);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s3);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SAR32 instruction, from s1, count in s2 (masked to 0x1f/0x3f, non-zero), store result in s1 using s3, s4 and s5 as scratch
void emit_sar32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    int64_t j64;

    CLEAR_FLAGS();
    SET_DFNONE();

    IFX (X_CF) {
        ADDI(s5, s2, -1);
        SRLxw(s3, s1, s5);
        ANDI(s3, s3, 1);
        OR(xFlags, xFlags, s3);
    }
    // For the SAR instruction, the OF flag is cleared for all 1-bit shifts.
    // OF nop

    if (rex.w) {
        SRA(s1, s1, s2);
    } else {
        SRAW(s1, s1, s2);
    }

    // SRAW sign-extends, so test sign bit before clearing upper bits
    IFX (X_SF) {
        SET_FLAGS_LTZ(s1, F_SF, s3, s4);
    }
    if (!rex.w) {
        ZEROUP(s1);
    }
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s3);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit ROL32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_rol32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4)
{
    int64_t j64;

    if (!rex.w) {
        ZEROUP(s1);
    }

    IFX (X_CF | X_OF) {
        ANDI(xFlags, xFlags, ~((1UL << F_CF) | (1UL << F_OF2)));
    }

    IFX (X_OF) {
        SRLIxw(s3, s1, rex.w ? 62 : 30);
        SRLI(s4, s3, 1);
        XOR(s3, s3, s4);
        ANDI(s3, s3, 1);
        SLLI(s3, s3, F_OF2);
        OR(xFlags, xFlags, s3);
    }

    if (cpuext.zbb) {
        if (rex.w) {
            ROL(s1, s1, s2);
        } else {
            ROLW(s1, s1, s2);
            ZEROUP(s1);
        }
    } else {
        SLLxw(s3, s1, s2);
        NEG(s4, s2);
        ADDI(s4, s4, rex.w ? 64 : 32);
        SRLxw(s1, s1, s4);
        OR(s1, s3, s1);
    }

    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_CF) {
        ANDI(s4, s1, 1); // LSB == F_CF
        OR(xFlags, xFlags, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit ROR32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_ror32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4)
{
    int64_t j64;

    if (!rex.w) {
        ZEROUP(s1);
    }

    IFX (X_CF) ANDI(xFlags, xFlags, ~(1UL << F_CF));
    IFX (X_OF) ANDI(xFlags, xFlags, ~(1UL << F_OF2));

    IFX (X_OF) {
        SRLIxw(s3, s1, rex.w ? 63 : 31);
        XOR(s3, s3, s1);
        ANDI(s3, s3, 1);
        SLLI(s3, s3, F_OF2);
        OR(xFlags, xFlags, s3);
    }

    if (cpuext.zbb) {
        if (rex.w) {
            ROR(s1, s1, s2);
        } else {
            RORW(s1, s1, s2);
            ZEROUP(s1);
        }
    } else {
        SRLxw(s3, s1, s2);
        NEG(s4, s2);
        ADDI(s4, s4, rex.w ? 64 : 32);
        SLLxw(s1, s1, s4);
        OR(s1, s3, s1);
    }

    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_CF) {
        SRLIxw(s3, s1, rex.w ? 63 : 31);
        OR(xFlags, xFlags, s3);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit ROL8 instruction, from s1, constant c (already masked to 0x1f, non-zero), store result in s1 using s3 and s4 as scratch
void emit_rol8c(dynarec_rv64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4)
{
    SET_DFNONE();

    IFX (X_OF) {
        ANDI(xFlags, xFlags, ~(1UL << F_OF2));
        SRLI(s3, s1, 6);
        SRLI(s4, s3, 1);
        XOR(s3, s3, s4);
        ANDI(s3, s3, 1);
        SLLI(s3, s3, F_OF2);
        OR(xFlags, xFlags, s3);
    }

    if (c & 7) {
        SLLI(s3, s1, c & 7);
        SRLI(s1, s1, 8 - (c & 7));
        OR(s1, s1, s3);
        ANDI(s1, s1, 0xff);
    }

    IFX (X_CF) {
        ANDI(xFlags, xFlags, ~(1UL << F_CF));
        ANDI(s3, s1, 1); // LSB == F_CF
        OR(xFlags, xFlags, s3);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit ROR8 instruction, from s1, constant c (already masked to 0x1f, non-zero), store result in s1 using s3 and s4 as scratch
void emit_ror8c(dynarec_rv64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4)
{
    SET_DFNONE();

    IFX (X_OF) {
        ANDI(xFlags, xFlags, ~(1UL << F_OF2));
        SRLI(s3, s1, 7);
        XOR(s3, s3, s1);
        ANDI(s3, s3, 1);
        SLLI(s3, s3, F_OF2);
        OR(xFlags, xFlags, s3);
    }

    if (c & 7) {
        SRLI(s3, s1, c & 7);
        SLLI(s1, s1, 8 - (c & 7));
        OR(s1, s1, s3);
        ANDI(s1, s1, 0xff);
    }

    IFX (X_CF) {
        ANDI(xFlags, xFlags, ~(1UL << F_CF));
        SRLI(s3, s1, 7);
        ANDI(s3, s3, 1);
        OR(xFlags, xFlags, s3);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit ROL8 instruction, from s1, count in s2 (already masked to 0x1f, non-zero), store result in s1 using s3 and s4 as scratch
void emit_rol8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    SET_DFNONE();

    IFX (X_OF) {
        ANDI(xFlags, xFlags, ~(1UL << F_OF2));
        SRLI(s3, s1, 6);
        SRLI(s4, s3, 1);
        XOR(s3, s3, s4);
        ANDI(s3, s3, 1);
        SLLI(s3, s3, F_OF2);
        OR(xFlags, xFlags, s3);
    }

    ANDI(s4, s2, 7);
    SLL(s3, s1, s4);
    SUB(s4, xZR, s4);
    ADDI(s4, s4, 8);
    SRL(s1, s1, s4);
    OR(s1, s1, s3);
    ANDI(s1, s1, 0xff);

    IFX (X_CF) {
        ANDI(xFlags, xFlags, ~(1UL << F_CF));
        ANDI(s3, s1, 1); // LSB == F_CF
        OR(xFlags, xFlags, s3);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit ROR8 instruction, from s1, count in s2 (already masked to 0x1f, non-zero), store result in s1 using s3 and s4 as scratch
void emit_ror8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    SET_DFNONE();

    IFX (X_OF) {
        ANDI(xFlags, xFlags, ~(1UL << F_OF2));
        SRLI(s3, s1, 7);
        XOR(s3, s3, s1);
        ANDI(s3, s3, 1);
        SLLI(s3, s3, F_OF2);
        OR(xFlags, xFlags, s3);
    }

    ANDI(s4, s2, 7);
    SRL(s3, s1, s4);
    SUB(s4, xZR, s4);
    ADDI(s4, s4, 8);
    SLL(s1, s1, s4);
    OR(s1, s1, s3);
    ANDI(s1, s1, 0xff);

    IFX (X_CF) {
        ANDI(xFlags, xFlags, ~(1UL << F_CF));
        SRLI(s3, s1, 7);
        ANDI(s3, s3, 1);
        OR(xFlags, xFlags, s3);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit RCL8 instruction, from s1, constant c (masked to 0x1f, non-zero), store result in s1 using s3 and s4 as scratch
void emit_rcl8c(dynarec_rv64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4)
{
    c %= 9;
    if (!c) return;

    SET_DFNONE();

    IFX (X_OF) {
        ANDI(xFlags, xFlags, ~(1UL << F_OF2));
        SRLI(s3, s1, 6);
        SRLI(s4, s3, 1);
        XOR(s3, s3, s4);
        ANDI(s3, s3, 1);
        SLLI(s3, s3, F_OF2);
        OR(xFlags, xFlags, s3);
    }

    ANDI(s3, xFlags, 1 << F_CF);
    SLLI(s3, s3, 8);
    OR(s1, s1, s3); // insert CF to bit 8

    IFX (X_CF) {
        ANDI(xFlags, xFlags, ~(1UL << F_CF));
        SRLI(s3, s1, 8 - c); // CF = bit(8-c) of the original
        ANDI(s3, s3, 1);
        OR(xFlags, xFlags, s3);
    }

    SLLI(s4, s1, 55 + c);
    SLLI(s3, s4, 1);
    SRLI(s3, s3, 56);
    SRLI(s1, s1, 9 - c);
    OR(s1, s1, s3);
    ANDI(s1, s1, 0xff);
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit RCR8 instruction, from s1, constant c (masked to 0x1f, non-zero), store result in s1 using s3 and s4 as scratch
void emit_rcr8c(dynarec_rv64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4)
{
    c %= 9;
    if (!c) return;

    SET_DFNONE();

    IFX (X_OF) {
        ANDI(xFlags, xFlags, ~(1UL << F_OF2));
        SRLI(s3, s1, 7);
        XOR(s3, s3, xFlags); // OF = bit7 ^ old CF
        ANDI(s3, s3, 1);
        SLLI(s3, s3, F_OF2);
        OR(xFlags, xFlags, s3);
    }

    ANDI(s3, xFlags, 1 << F_CF);
    SLLI(s3, s3, 8);
    OR(s1, s1, s3); // insert CF to bit 8

    IFX (X_CF) {
        ANDI(xFlags, xFlags, ~(1UL << F_CF));
        SRLI(s3, s1, c - 1); // CF = bit(c-1) of the original
        ANDI(s3, s3, 1);
        OR(xFlags, xFlags, s3);
    }

    SRLI(s3, s1, c);
    SLLI(s1, s1, 9 - c);
    OR(s1, s1, s3);
    ANDI(s1, s1, 0xff);
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit RCL8 instruction, from s1, count in s2 (masked to 0x1f), store result in s1 using s3, s4 and s5 as scratch
void emit_rcl8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    int64_t j64;
    SET_DFNONE();

    // s2 %= 9 (s2 is 0..31)
    ADDI(s3, xZR, 9);
    SLTU(s4, s2, s3);
    XORI(s4, s4, 1);
    SLLI(s5, s4, 3);
    ADD(s4, s4, s5);
    SUB(s2, s2, s4);
    SLTU(s4, s2, s3);
    XORI(s4, s4, 1);
    SLLI(s5, s4, 3);
    ADD(s4, s4, s5);
    SUB(s2, s2, s4);
    SLTU(s4, s2, s3);
    XORI(s4, s4, 1);
    SLLI(s5, s4, 3);
    ADD(s4, s4, s5);
    SUB(s2, s2, s4);
    BEQ_NEXT(s2, xZR);

    IFX (X_OF) {
        ANDI(xFlags, xFlags, ~(1UL << F_OF2));
        SRLI(s3, s1, 6);
        SRLI(s4, s3, 1);
        XOR(s3, s3, s4);
        ANDI(s3, s3, 1);
        SLLI(s3, s3, F_OF2);
        OR(xFlags, xFlags, s3);
    }

    ANDI(s3, xFlags, 1 << F_CF);
    SLLI(s3, s3, 8);
    OR(s1, s1, s3); // insert CF to bit 8
    SLLI(s3, s1, 9);
    OR(s1, s1, s3); // duplicate the byte at bits 9..16

    ADDI(s4, xZR, 9);
    SUB(s4, s4, s2); // 9 - cnt
    IFX (X_CF) {
        ANDI(xFlags, xFlags, ~(1UL << F_CF));
        ADDI(s5, s4, -1);
        SRL(s5, s1, s5); // CF = bit(8-cnt) of the original
        ANDI(s5, s5, 1);
        OR(xFlags, xFlags, s5);
    }
    SRL(s1, s1, s4);
    ANDI(s1, s1, 0xff);
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit RCR8 instruction, from s1, count in s2 (masked to 0x1f), store result in s1 using s3, s4 and s5 as scratch
void emit_rcr8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    int64_t j64;
    SET_DFNONE();

    // s2 %= 9 (s2 is 0..31)
    ADDI(s3, xZR, 9);
    SLTU(s4, s2, s3);
    XORI(s4, s4, 1);
    SLLI(s5, s4, 3);
    ADD(s4, s4, s5);
    SUB(s2, s2, s4);
    SLTU(s4, s2, s3);
    XORI(s4, s4, 1);
    SLLI(s5, s4, 3);
    ADD(s4, s4, s5);
    SUB(s2, s2, s4);
    SLTU(s4, s2, s3);
    XORI(s4, s4, 1);
    SLLI(s5, s4, 3);
    ADD(s4, s4, s5);
    SUB(s2, s2, s4);
    BEQ_NEXT(s2, xZR);

    IFX (X_OF) {
        ANDI(xFlags, xFlags, ~(1UL << F_OF2));
        SRLI(s3, s1, 7);
        XOR(s3, s3, xFlags); // OF = bit7 ^ old CF
        ANDI(s3, s3, 1);
        SLLI(s3, s3, F_OF2);
        OR(xFlags, xFlags, s3);
    }

    ANDI(s3, xFlags, 1 << F_CF);
    SLLI(s3, s3, 8);
    OR(s1, s1, s3); // insert CF to bit 8
    SLLI(s3, s1, 9);
    OR(s1, s1, s3); // duplicate the byte at bits 9..16

    IFX (X_CF) {
        ANDI(xFlags, xFlags, ~(1UL << F_CF));
        ADDI(s5, s2, -1);
        SRL(s5, s1, s5); // CF = bit(cnt-1) of the original
        ANDI(s5, s5, 1);
        OR(xFlags, xFlags, s5);
    }
    SRL(s1, s1, s2);
    ANDI(s1, s1, 0xff);
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit ROL16 instruction, from s1, count in s2 (masked to 0x1f, non-zero), store result in s1 using s3 and s4 as scratch
void emit_rol16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    SET_DFNONE();

    IFX (X_OF) {
        ANDI(xFlags, xFlags, ~(1UL << F_OF2));
        SRLI(s3, s1, 14);
        SRLI(s4, s3, 1);
        XOR(s3, s3, s4);
        ANDI(s3, s3, 1);
        SLLI(s3, s3, F_OF2);
        OR(xFlags, xFlags, s3);
    }

    ANDI(s4, s2, 15);
    SLL(s3, s1, s4);
    SUB(s4, xZR, s4);
    ADDI(s4, s4, 16);
    SRL(s1, s1, s4);
    OR(s1, s1, s3);
    ZEXTH(s1, s1);

    IFX (X_CF) {
        ANDI(xFlags, xFlags, ~(1UL << F_CF));
        ANDI(s3, s1, 1); // LSB == F_CF
        OR(xFlags, xFlags, s3);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit ROR16 instruction, from s1, count in s2 (masked to 0x1f, non-zero), store result in s1 using s3 and s4 as scratch
void emit_ror16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    SET_DFNONE();

    IFX (X_OF) {
        ANDI(xFlags, xFlags, ~(1UL << F_OF2));
        SRLI(s3, s1, 15);
        XOR(s3, s3, s1);
        ANDI(s3, s3, 1);
        SLLI(s3, s3, F_OF2);
        OR(xFlags, xFlags, s3);
    }

    ANDI(s4, s2, 15);
    SRL(s3, s1, s4);
    SUB(s4, xZR, s4);
    ADDI(s4, s4, 16);
    SLL(s1, s1, s4);
    OR(s1, s1, s3);
    ZEXTH(s1, s1);

    IFX (X_CF) {
        ANDI(xFlags, xFlags, ~(1UL << F_CF));
        SRLI(s3, s1, 15);
        ANDI(s3, s3, 1);
        OR(xFlags, xFlags, s3);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit RCL16 instruction, from s1, count in s2 (masked to 0x1f), store result in s1 using s3, s4 and s5 as scratch
void emit_rcl16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    int64_t j64;
    SET_DFNONE();

    // s2 %= 17 (s2 is 0..31)
    ADDI(s3, xZR, 17);
    SLTU(s4, s2, s3);
    XORI(s4, s4, 1);
    SLLI(s5, s4, 4);
    ADD(s4, s4, s5);
    SUB(s2, s2, s4);
    BEQ_NEXT(s2, xZR);

    IFX (X_OF) {
        ANDI(xFlags, xFlags, ~(1UL << F_OF2));
        SRLI(s3, s1, 14);
        SRLI(s4, s3, 1);
        XOR(s3, s3, s4);
        ANDI(s3, s3, 1);
        SLLI(s3, s3, F_OF2);
        OR(xFlags, xFlags, s3);
    }

    ANDI(s3, xFlags, 1 << F_CF);
    SLLI(s3, s3, 16);
    OR(s1, s1, s3); // insert CF to bit 16
    SLLI(s3, s1, 17);
    OR(s1, s1, s3); // duplicate the word at bits 17..32

    ADDI(s4, xZR, 17);
    SUB(s4, s4, s2); // 17 - cnt
    IFX (X_CF) {
        ANDI(xFlags, xFlags, ~(1UL << F_CF));
        ADDI(s5, s4, -1);
        SRL(s5, s1, s5); // CF = bit(16-cnt) of the original
        ANDI(s5, s5, 1);
        OR(xFlags, xFlags, s5);
    }
    SRL(s1, s1, s4);
    ZEXTH(s1, s1);
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit RCR16 instruction, from s1, count in s2 (masked to 0x1f), store result in s1 using s3, s4 and s5 as scratch
void emit_rcr16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    int64_t j64;
    SET_DFNONE();

    // s2 %= 17 (s2 is 0..31)
    ADDI(s3, xZR, 17);
    SLTU(s4, s2, s3);
    XORI(s4, s4, 1);
    SLLI(s5, s4, 4);
    ADD(s4, s4, s5);
    SUB(s2, s2, s4);
    BEQ_NEXT(s2, xZR);

    IFX (X_OF) {
        ANDI(xFlags, xFlags, ~(1UL << F_OF2));
        SRLI(s3, s1, 15);
        XOR(s3, s3, xFlags); // OF = bit15 ^ old CF
        ANDI(s3, s3, 1);
        SLLI(s3, s3, F_OF2);
        OR(xFlags, xFlags, s3);
    }

    ANDI(s3, xFlags, 1 << F_CF);
    SLLI(s3, s3, 16);
    OR(s1, s1, s3); // insert CF to bit 16
    SLLI(s3, s1, 17);
    OR(s1, s1, s3); // duplicate the word at bits 17..32

    IFX (X_CF) {
        ANDI(xFlags, xFlags, ~(1UL << F_CF));
        ADDI(s5, s2, -1);
        SRL(s5, s1, s5); // CF = bit(cnt-1) of the original
        ANDI(s5, s5, 1);
        OR(xFlags, xFlags, s5);
    }
    SRL(s1, s1, s2);
    ZEXTH(s1, s1);
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit RCL32 instruction, from s1, constant c (masked to 0x1f, non-zero), store result in s1 using s3 and s4 as scratch
void emit_rcl32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4, int s5)
{
    c %= rex.w ? 65 : 33;
    if (!c) return;

    SET_DFNONE();
    if (!rex.w) ZEROUP(s1);

    IFX (X_OF) {
        ANDI(xFlags, xFlags, ~(1UL << F_OF2));
        SRLIxw(s3, s1, rex.w ? 62 : 30);
        SRLI(s4, s3, 1);
        XOR(s3, s3, s4);
        ANDI(s3, s3, 1);
        SLLI(s3, s3, F_OF2);
        OR(xFlags, xFlags, s3);
    }

    ANDI(s3, xFlags, 1 << F_CF);
    IFX (X_CF) {
        ANDI(xFlags, xFlags, ~(1UL << F_CF));
        SRLIxw(s5, s1, (rex.w ? 64 : 32) - c);
        ANDI(s5, s5, 1);
        OR(xFlags, xFlags, s5);
    }

    if (c == 1) {
        SLLIxw(s1, s1, 1);
        OR(s1, s1, s3);
    } else {
        SLLIxw(s4, s1, c);
        SLLI(s3, s3, c - 1);
        OR(s4, s4, s3);
        SRLIxw(s5, s1, (rex.w ? 65 : 33) - c);
        OR(s1, s4, s5);
    }
    if (!rex.w) ZEROUP(s1);
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit RCR32 instruction, from s1, constant c (masked to 0x1f, non-zero), store result in s1 using s3 and s4 as scratch
void emit_rcr32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4, int s5)
{
    c %= rex.w ? 65 : 33;
    if (!c) return;

    SET_DFNONE();
    if (!rex.w) ZEROUP(s1);

    IFX (X_OF) {
        ANDI(xFlags, xFlags, ~(1UL << F_OF2));
        SRLIxw(s3, s1, rex.w ? 63 : 31);
        XOR(s3, s3, xFlags);
        ANDI(s3, s3, 1);
        SLLI(s3, s3, F_OF2);
        OR(xFlags, xFlags, s3);
    }

    ANDI(s3, xFlags, 1 << F_CF);
    IFX (X_CF) {
        ANDI(xFlags, xFlags, ~(1UL << F_CF));
        SRLIxw(s5, s1, c - 1);
        ANDI(s5, s5, 1);
        OR(xFlags, xFlags, s5);
    }

    if (c == 1) {
        SRLIxw(s1, s1, 1);
        SLLI(s3, s3, rex.w ? 63 : 31);
        OR(s1, s1, s3);
    } else {
        SRLIxw(s4, s1, c);
        SLLI(s3, s3, (rex.w ? 64 : 32) - c);
        OR(s4, s4, s3);
        SLLIxw(s5, s1, (rex.w ? 65 : 33) - c);
        OR(s1, s4, s5);
    }
    if (!rex.w) ZEROUP(s1);
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit RCL32 instruction, from s1, count in s2 (masked to 0x1f, non-zero), store result in s1 using s3, s4 and s5 as scratch
void emit_rcl32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    int64_t j64;
    SET_DFNONE();
    if (!rex.w) ZEROUP(s1);

    IFX (X_OF) {
        ANDI(xFlags, xFlags, ~(1UL << F_OF2));
        SRLIxw(s3, s1, rex.w ? 62 : 30);
        SRLI(s4, s3, 1);
        XOR(s3, s3, s4);
        ANDI(s3, s3, 1);
        SLLI(s3, s3, F_OF2);
        OR(xFlags, xFlags, s3);
    }

    ANDI(s3, xFlags, 1 << F_CF);
    IFX (X_CF) {
        ANDI(xFlags, xFlags, ~(1UL << F_CF));
        ADDI(s4, xZR, rex.w ? 64 : 32);
        SUB(s4, s4, s2);
        SRL(s4, s1, s4);
        ANDI(s4, s4, 1);
        OR(xFlags, xFlags, s4);
    }

    SLL(s4, s1, s2);
    ADDI(s5, s2, -1);
    SLL(s5, s3, s5);
    OR(s4, s4, s5);
    if (rex.w) {
        ADDI(s5, s2, -1);
        BEQZ(s5, 4 * 4);
    }
    ADDI(s5, xZR, rex.w ? 65 : 33);
    SUB(s5, s5, s2);
    SRL(s5, s1, s5);
    OR(s4, s4, s5);
    MV(s1, s4);
    if (!rex.w) ZEROUP(s1);
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit RCR32 instruction, from s1, count in s2 (masked to 0x1f, non-zero), store result in s1 using s3, s4 and s5 as scratch
void emit_rcr32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    int64_t j64;
    SET_DFNONE();
    if (!rex.w) ZEROUP(s1);

    IFX (X_OF) {
        ANDI(xFlags, xFlags, ~(1UL << F_OF2));
        SRLIxw(s3, s1, rex.w ? 63 : 31);
        XOR(s3, s3, xFlags);
        ANDI(s3, s3, 1);
        SLLI(s3, s3, F_OF2);
        OR(xFlags, xFlags, s3);
    }

    ANDI(s3, xFlags, 1 << F_CF);
    IFX (X_CF) {
        ANDI(xFlags, xFlags, ~(1UL << F_CF));
        ADDI(s5, s2, -1);
        SRL(s5, s1, s5);
        ANDI(s5, s5, 1);
        OR(xFlags, xFlags, s5);
    }

    SRL(s4, s1, s2);
    ADDI(s5, xZR, rex.w ? 64 : 32);
    SUB(s5, s5, s2);
    SLL(s5, s3, s5);
    OR(s4, s4, s5);
    if (rex.w) {
        ADDI(s5, s2, -1);
        BEQZ(s5, 4 * 4);
    }
    ADDI(s5, xZR, rex.w ? 65 : 33);
    SUB(s5, s5, s2);
    SLL(s5, s1, s5);
    OR(s4, s4, s5);
    MV(s1, s4);
    if (!rex.w) ZEROUP(s1);
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit ROL16 instruction, from s1, constant c, store result in s1 using s3 and s4 as scratch
void emit_rol16c(dynarec_rv64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4)
{
    c &= 0x1f;
    if (!c) return;

    SET_DFNONE();

    IFX (X_CF) ANDI(xFlags, xFlags, ~(1UL << F_CF));
    IFX (X_OF) {
        ANDI(xFlags, xFlags, ~(1UL << F_OF2));
        SRLI(s3, s1, 14);
        SRLI(s4, s3, 1);
        XOR(s3, s3, s4);
        ANDI(s3, s3, 1);
        SLLI(s3, s3, F_OF2);
        OR(xFlags, xFlags, s3);
    }

    if (c & 15) {
        SRLI(s3, s1, 16 - (c & 15));
        SLLI(s1, s1, c & 15);
        OR(s1, s1, s3);
        ZEXTH(s1, s1);
    }

    IFX (X_CF) {
        ANDI(xFlags, xFlags, ~(1UL << F_CF));
        ANDI(s3, s1, 1); // LSB == F_CF
        OR(xFlags, xFlags, s3);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit ROL32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_rol32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4)
{
    c &= rex.w ? 0x3f : 0x1f;
    if (!c) {
        if (!rex.w) ZEROUP(s1);
        return;
    }

    SET_DFNONE(); // rotate flags are computed inline, never deferred

    IFX (X_CF) ANDI(xFlags, xFlags, ~(1UL << F_CF));
    IFX (X_OF) ANDI(xFlags, xFlags, ~(1UL << F_OF2));

    IFX (X_OF) {
        SRLIxw(s3, s1, rex.w ? 62 : 30);
        SRLI(s4, s3, 1);
        XOR(s3, s3, s4);
        ANDI(s3, s3, 1);
        SLLI(s3, s3, F_OF2);
        OR(xFlags, xFlags, s3);
    }
    if (cpuext.zbb) {
        RORIxw(s1, s1, (rex.w ? 64 : 32) - c);
    } else if (cpuext.xtheadbb) {
        TH_SRRIxw(s1, s1, (rex.w ? 64 : 32) - c);
    } else {
        SLLIxw(s3, s1, c);
        SRLIxw(s1, s1, (rex.w ? 64 : 32) - c);
        OR(s1, s3, s1);
    }

    if (!rex.w) ZEROUP(s1);

    IFX (X_CF) {
        ANDI(s4, s1, 1 << F_CF);
        OR(xFlags, xFlags, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit ROR16 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_ror16c(dynarec_rv64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4)
{
    c &= 0x1f;
    if (!c) return;

    SET_DFNONE();

    IFX (X_CF) ANDI(xFlags, xFlags, ~(1UL << F_CF));
    IFX (X_OF) {
        ANDI(xFlags, xFlags, ~(1UL << F_OF2));
        SRLI(s3, s1, 15);
        XOR(s3, s3, s1);
        ANDI(s3, s3, 1);
        SLLI(s3, s3, F_OF2);
        OR(xFlags, xFlags, s3);
    }

    if (c & 15) {
        SRLI(s3, s1, c & 15);
        SLLI(s1, s1, 16 - (c & 15));
        OR(s1, s1, s3);
        ZEXTH(s1, s1);
    }

    IFX (X_CF) {
        SRLI(s3, s1, 15);
        ANDI(s3, s3, 1);
        OR(xFlags, xFlags, s3);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit ROR32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_ror32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4)
{
    c &= rex.w ? 0x3f : 0x1f;
    if (!c) return;

    SET_DFNONE();

    IFX (X_CF | X_OF) {
        ANDI(xFlags, xFlags, ~(1UL << F_CF));
    }

    IFX (X_OF) {
        ANDI(xFlags, xFlags, ~(1UL << F_OF2));
        SRLIxw(s3, s1, rex.w ? 63 : 31);
        XOR(s3, s3, s1);
        ANDI(s3, s3, 1);
        SLLI(s3, s3, F_OF2);
        OR(xFlags, xFlags, s3);
    }

    if (cpuext.zbb) {
        RORIxw(s1, s1, c);
    } else if (cpuext.xtheadbb) {
        TH_SRRIxw(s1, s1, c);
    } else {
        SRLIxw(s3, s1, c);
        SLLIxw(s1, s1, (rex.w ? 64 : 32) - c);
        OR(s1, s3, s1);
    }

    if (!rex.w) ZEROUP(s1);

    IFX (X_CF) {
        SRLIxw(s3, s1, rex.w ? 63 : 31);
        OR(xFlags, xFlags, s3);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit SHRD32 instruction, from s1, fill s2 , constant c, store result in s1 using s3 and s4 as scratch
void emit_shrd32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, uint32_t c, int s3, int s4, int s5)
{
    CLEAR_FLAGS();

    IFX (X_PEND) {
        MOV64x(s3, c);
        SDxw(s3, xEmu, offsetof(x64emu_t, op2));
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, rex.w ? d_shrd64 : d_shrd32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    IFX (X_CF) {
        if (c > 1) {
            SRAI(s3, s1, c - 1);
            ANDI(s3, s3, 1); // LSB == F_CF
        } else {
            ANDI(s3, s1, 1); // LSB == F_CF
        }
        OR(xFlags, xFlags, s3);
    }
    IFX (X_OF) {
        SRLIxw(s3, s1, rex.w ? 63 : 31);
        XOR(s3, s3, s2);
        ANDI(s3, s3, 1);
        SLLI(s3, s3, F_OF2);
        OR(xFlags, xFlags, s3);
    }

    SRLIxw(s3, s1, c);
    SLLIxw(s1, s2, (rex.w ? 64 : 32) - c);
    OR(s1, s1, s3);

    if (!rex.w) {
        ZEROUP(s1);
    }
    IFX (X_SF) {
        SRLIxw(s3, s1, rex.w ? 63 : 31);
        SET_FLAGS_NEZ(s3, F_SF, s5);
    }
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s5);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

void emit_shrd16c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, uint32_t c, int s3, int s4, int s5)
{
    c &= 0x1f;
    CLEAR_FLAGS();

    IFX (X_PEND) {
        if (c) {
            MOV64x(s3, c);
            SH(s3, xEmu, offsetof(x64emu_t, op2));
        } else
            SH(xZR, xEmu, offsetof(x64emu_t, op2));
        SH(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, d_shrd16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    if (!c) {
        IFX (X_PEND) {
            SH(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }
    IFX (X_OF) {
        SRLI(s3, s1, 15);
        XOR(s3, s3, s2);
        ANDI(s3, s3, 1);
        SLLI(s3, s3, F_OF2);
        OR(xFlags, xFlags, s3);
    }

    // create concat first
    SLLI(s5, s2, 16);
    OR(s1, s1, s5);

    IFX (X_CF) {
        if (c > 1) {
            SRAI(s3, s1, c - 1);
            ANDI(s3, s3, 1); // LSB == F_CF
        } else {
            // no need to shift
            ANDI(s3, s1, 1); // LSB == F_CF
        }
        OR(xFlags, xFlags, s3);
    }

    if (cpuext.zbb) {
        RORIW(s1, s1, c);
    } else {
        SRLI(s5, s1, c);
        SLLI(s1, s1, 32 - c);
        OR(s1, s1, s5);
    }
    ZEXTH(s1, s1);

    IFX (X_PEND) {
        SH(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s5);
    }
    IFX (X_SF) {
        SLLIW(s3, s1, 16);
        SET_FLAGS_LTZ(s3, F_SF, s4, s5);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

void emit_shld32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, uint32_t c, int s3, int s4, int s5)
{
    CLEAR_FLAGS();
    IFX (X_PEND) {
        MOV64x(s3, c);
        SDxw(s3, xEmu, offsetof(x64emu_t, op2));
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, rex.w ? d_shld64 : d_shld32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFX (X_CF) {
        SRLI(s3, s1, (rex.w ? 64 : 32) - c);
        ANDI(s4, s3, 1); // F_CF
        OR(xFlags, xFlags, s4);
    }

    IFX (X_OF) {
        SRLIxw(s3, s1, rex.w ? 62 : 30);
        SRLI(s4, s3, 1);
        XOR(s3, s3, s4);
        ANDI(s3, s3, 1);
        SLLI(s3, s3, F_OF2);
        OR(xFlags, xFlags, s3);
    }

    SLLIxw(s3, s1, c);
    SRLIxw(s1, s2, (rex.w ? 64 : 32) - c);
    OR(s1, s1, s3);

    if (!rex.w) ZEROUP(s1);

    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s5);
    }
    IFX (X_SF) {
        SRLIxw(s3, s1, rex.w ? 63 : 31);
        SET_FLAGS_NEZ(s3, F_SF, s4);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}


void emit_shrd32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s5, int s3, int s4, int s6)
{
    int64_t j64;
    CLEAR_FLAGS();
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SDxw(s5, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, rex.w ? d_shrd64 : d_shrd32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }
    IFX (X_CF) {
        SUBI(s3, s5, 1);
        SRA(s3, s1, s3);
        ANDI(s3, s3, 1); // LSB == F_CF
        OR(xFlags, xFlags, s3);
    }
    IFX (X_OF) {
        SRLIxw(s3, s1, rex.w ? 63 : 31);
        XOR(s3, s3, s2);
        ANDI(s3, s3, 1);
        SLLI(s3, s3, F_OF2);
        OR(xFlags, xFlags, s3);
    }
    ADDI(s4, xZR, (rex.w ? 64 : 32));
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
        SET_FLAGS_EQZ(s1, F_ZF, s5);
    }
    IFX (X_SF) {
        SRLIxw(s3, s1, rex.w ? 63 : 31);
        SET_FLAGS_NEZ(s3, F_SF, s5);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

void emit_shld32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s5, int s3, int s4, int s6)
{
    int64_t j64;
    CLEAR_FLAGS();
    IFX (X_PEND) {
        SDxw(s1, xEmu, offsetof(x64emu_t, op1));
        SDxw(s5, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, rex.w ? d_shld64 : d_shld32);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    MOV32w(s3, (rex.w ? 64 : 32));
    SUB(s3, s3, s5);
    IFX (X_CF) {
        SRL(s4, s1, s3);
        ANDI(s4, s4, 1); // LSB == F_CF
        OR(xFlags, xFlags, s4);
    }
    IFX (X_OF) {
        SRLIxw(s6, s1, rex.w ? 62 : 30);
        SRLI(s4, s6, 1);
        XOR(s6, s6, s4);
        ANDI(s6, s6, 1);
        SLLI(s6, s6, F_OF2);
        OR(xFlags, xFlags, s6);
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
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s5);
    }
    IFX (X_SF) {
        SRLIxw(s3, s1, rex.w ? 63 : 31);
        SET_FLAGS_NEZ(s3, F_SF, s5);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

void emit_shld16c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, uint32_t c, int s3, int s4, int s5)
{
    c &= 0x1f;
    CLEAR_FLAGS();
    IFX (X_PEND) {
        if (c) {
            MOV64x(s3, c);
            SH(s3, xEmu, offsetof(x64emu_t, op2));
        } else
            SH(xZR, xEmu, offsetof(x64emu_t, op2));
        SH(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, d_shld16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    if (!c) {
        IFX (X_PEND) {
            SH(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }

    IFX (X_OF) {
        SRLI(s3, s1, 14);
        SRLI(s4, s3, 1);
        XOR(s3, s3, s4);
        ANDI(s3, s3, 1);
        SLLI(s3, s3, F_OF2);
        OR(xFlags, xFlags, s3);
    }

    // create concat first
    SLLI(s5, s2, 16);
    OR(s1, s1, s5);

    IFX (X_CF) {
        if (c <= 16) {
            SRLI(s3, s1, 16 - c);
        } else {
            SRLI(s3, s2, 32 - c);
        }
        ANDI(s5, s3, 1); // LSB == F_CF
        OR(xFlags, xFlags, s5);
    }
    if (cpuext.zbb) {
        RORIW(s1, s1, 32 - c);
    } else {
        SLLI(s3, s1, c);
        SRLI(s1, s1, 32 - c);
        OR(s1, s1, s3);
    }
    ZEXTH(s1, s1);

    IFX (X_PEND) {
        SH(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_SF) {
        SLLIW(s4, s1, 16);
        SET_FLAGS_LTZ(s4, F_SF, s3, s5);
    }
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s3);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

void emit_shld16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5, int s6)
{
    CLEAR_FLAGS();
    IFX (X_PEND) {
        SH(s1, xEmu, offsetof(x64emu_t, op1));
        SH(s3, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, d_shld16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    IFX (X_OF) {
        SRLI(s5, s1, 14);
        SRLI(s4, s5, 1);
        XOR(s5, s5, s4);
        ANDI(s5, s5, 1);
        SLLI(s5, s5, F_OF2);
        OR(xFlags, xFlags, s5);
    }

    MOV32w(s6, 32);
    SUBW(s6, s6, s3);
    IFX (X_CF) {
        SLLI(s5, s1, 16);
        OR(s4, s2, s5);
        SRLW(s5, s4, s6);
        ANDI(s5, s5, 1); // LSB == F_CF
        OR(xFlags, xFlags, s5);
    }
    // create concat first
    SLLI(s5, s2, 16);
    OR(s1, s1, s5);

    if (cpuext.zbb) {
        RORW(s1, s1, s6);
    } else {
        SLLW(s5, s1, s3);
        SRLW(s1, s1, s6);
        OR(s1, s1, s5);
    }
    ZEXTH(s1, s1);

    IFX (X_PEND) {
        SH(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_SF) {
        SLLIW(s4, s1, 16);
        SET_FLAGS_LTZ(s4, F_SF, s4, s5);
    }
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s4);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s4, s5);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s4, xZR);
}

void emit_shrd16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5, int s6)
{
    CLEAR_FLAGS();
    IFX (X_PEND) {
        SH(s1, xEmu, offsetof(x64emu_t, op1));
        SH(s3, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, d_shrd16);
    } else IFXORNAT (X_ALL) {
        SET_DFNONE();
    }

    SLLI(s5, s2, 16);
    OR(s1, s1, s5);
    IFX (X_OF) {
        SRLI(s5, s1, 15);
        XOR(s5, s5, s2);
        ANDI(s5, s5, 1);
        SLLI(s5, s5, F_OF2);
        OR(xFlags, xFlags, s5);
    }

    IFX (X_CF) {
        ADDIW(s5, s3, -1);
        SRLW(s5, s1, s5);
        ANDI(s5, s5, 1); // LSB == F_CF
        OR(xFlags, xFlags, s5);
    }

    if (cpuext.zbb) {
        RORW(s1, s1, s3);
    } else {
        MOV32w(s6, 32);
        SUBW(s6, s6, s3);
        SRLW(s5, s1, s3);
        SLLW(s1, s1, s6);
        OR(s1, s1, s5);
    }
    ZEXTH(s1, s1);

    IFX (X_PEND) {
        SH(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX (X_ZF) {
        SET_FLAGS_EQZ(s1, F_ZF, s5);
    }
    IFX (X_SF) {
        SLLIW(s3, s1, 16);
        SET_FLAGS_LTZ(s3, F_SF, s4, s5);
    }
    IFX (X_PF) {
        emit_pf(dyn, ninst, s1, s4, s5);
    }
    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit RCL16 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_rcl16c(dynarec_rv64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4)
{
    if (!(c % 17)) return;

    SET_DFNONE();

    IFX (X_OF) {
        ANDI(xFlags, xFlags, ~(1UL << F_OF2));
        SRLI(s3, s1, 14);
        SRLI(s4, s3, 1);
        XOR(s3, s3, s4);
        ANDI(s3, s3, 1);
        SLLI(s3, s3, F_OF2);
        OR(xFlags, xFlags, s3);
    }

    c %= 17;

    ANDI(s3, xFlags, 1 << F_CF);
    SLLI(s3, s3, 16);
    OR(s1, s1, s3); // insert CF to bit 16

    SLLI(s4, s1, 47 + c);
    SLLI(s3, s4, 1);
    SRLI(s3, s3, 48);
    SRLI(s1, s1, 17 - c);
    OR(s1, s1, s3);

    IFX (X_CF) {
        ANDI(xFlags, xFlags, ~(1UL << F_CF));
        SRLI(s4, s4, 63);
        OR(xFlags, xFlags, s4);
    }

    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}

// emit RCR16 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_rcr16c(dynarec_rv64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4)
{
    if (!(c % 17)) return;

    SET_DFNONE();

    IFX (X_OF) {
        ANDI(xFlags, xFlags, ~(1UL << F_OF2));
        SRLI(s3, s1, 15);
        XOR(s3, s3, xFlags);
        ANDI(s3, s3, 1);
        SLLI(s3, s3, F_OF2);
        OR(xFlags, xFlags, s3);
    }

    c %= 17;

    ANDI(s3, xFlags, 1 << F_CF);
    SLLI(s3, s3, 16);
    OR(s1, s1, s3); // insert CF to bit 16

    SRLI(s3, s1, c);
    SLLI(s1, s1, 17 - c);
    OR(s1, s1, s3);
    SLLI(s4, s1, 47);
    ZEXTH(s1, s1);

    IFX (X_CF) {
        ANDI(xFlags, xFlags, ~(1UL << F_CF));
        SRLI(s4, s4, 63);
        OR(xFlags, xFlags, s4);
    }

    if (dyn->insts[ninst].nat_flags_fusion) NAT_FLAGS_OPS(s1, xZR, s3, xZR);
}
