#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "dynarec.h"
#include "emu/x64emu_private.h"
#include "emu/x64run_private.h"
#include "x64run.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"

#include "arm64_printer.h"
#include "dynarec_arm64_private.h"
#include "dynarec_arm64_functions.h"
#include "dynarec_arm64_helper.h"

// emit OR32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_or32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4)
{
    MAYUSE(s2);
    IFX(X_PEND) {
        SET_DF(s4, rex.w?d_or64:d_or32);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    ORRxw_REG(s1, s1, s2);
    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_CF | X_AF | X_OF) {
        MOV32w(s3, (1<<F_CF)|(1<<F_AF)|(1<<F_OF));
        BICw(xFlags, xFlags, s3);
    }
    IFX(X_ZF) {
        TSTxw_REG(s1, s1);
        CSETw(s3, cEQ);
        BFIw(xFlags, s3, F_ZF, 1);
    }
    IFX(X_SF) {
        LSRxw(s3, s1, (rex.w)?63:31);
        BFIw(xFlags, s3, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit OR32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_or32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4)
{
    IFX(X_PEND) {
        SET_DF(s4, rex.w?d_or64:d_or32);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    int mask = convert_bitmask_xw(c);
    if(mask) {
        ORRxw_mask(s1, s1, (mask>>12)&1, mask&0x3F, (mask>>6)&0x3F);
    } else {
        MOV64xw(s3, c);
        ORRxw_REG(s1, s1, s3);
    }
    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_CF | X_AF | X_OF) {
        MOV32w(s3, (1<<F_CF)|(1<<F_AF)|(1<<F_OF));
        BICw(xFlags, xFlags, s3);
    }
    IFX(X_ZF) {
        TSTxw_REG(s1, s1);
        CSETw(s3, cEQ);
        BFIw(xFlags, s3, F_ZF, 1);
    }
    IFX(X_SF) {
        LSRxw(s3, s1, (rex.w)?63:31);
        BFIw(xFlags, s3, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit XOR32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_xor32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4)
{
    MAYUSE(s2);
    IFX(X_PEND) {
        SET_DF(s4, rex.w?d_xor64:d_xor32);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    EORxw_REG(s1, s1, s2);
    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_CF | X_AF | X_OF) {
        MOV32w(s3, (1<<F_CF)|(1<<F_AF)|(1<<F_OF));
        BICw(xFlags, xFlags, s3);
    }
    IFX(X_ZF) {
        TSTxw_REG(s1, s1);
        CSETw(s3, cEQ);
        BFIw(xFlags, s3, F_ZF, 1);
    }
    IFX(X_SF) {
        LSRxw(s3, s1, (rex.w)?63:31);
        BFIw(xFlags, s3, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit XOR32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_xor32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4)
{
    IFX(X_PEND) {
        SET_DF(s4, rex.w?d_xor64:d_xor32);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    int mask = convert_bitmask_xw(c);
    if(mask) {
        EORxw_mask(s1, s1, (mask>>12)&1, mask&0x3F, (mask>>6)&0x3F);
    } else {
        MOV64xw(s3, c);
        EORxw_REG(s1, s1, s3);
    }
    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_CF | X_AF | X_OF) {
        MOV32w(s3, (1<<F_CF)|(1<<F_AF)|(1<<F_OF));
        BICw(xFlags, xFlags, s3);
    }
    IFX(X_ZF) {
        TSTxw_REG(s1, s1);
        CSETw(s3, cEQ);
        BFIw(xFlags, s3, F_ZF, 1);
    }
    IFX(X_SF) {
        LSRxw(s3, s1, (rex.w)?63:31);
        BFIw(xFlags, s3, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit AND32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_and32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4)
{
    MAYUSE(s2);
    IFX(X_PEND) {
        SET_DF(s4, rex.w?d_and64:d_and32);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    IFX(X_ZF) {
        ANDSxw_REG(s1, s1, s2);
    } else {
        ANDxw_REG(s1, s1, s2);
    }
    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_CF | X_AF | X_OF) {
        MOV32w(s3, (1<<F_CF)|(1<<F_AF)|(1<<F_OF));
        BICw(xFlags, xFlags, s3);
    }
    IFX(X_ZF) {
        CSETw(s3, cEQ);
        BFIw(xFlags, s3, F_ZF, 1);
    }
    IFX(X_SF) {
        LSRxw(s3, s1, (rex.w)?63:31);
        BFIw(xFlags, s3, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit AND32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_and32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4)
{
    IFX(X_PEND) {
        SET_DF(s4, rex.w?d_and64:d_and32);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    int mask = convert_bitmask_xw(c);
    if(mask) {
        IFX(X_ALL) {
            ANDSxw_mask(s1, s1, (mask>>12)&1, mask&0x3F, (mask>>6)&0x3F);
        } else {
            ANDxw_mask(s1, s1, (mask>>12)&1, mask&0x3F, (mask>>6)&0x3F);
        }
    } else {
        MOV64xw(s3, c);
        IFX(X_ALL) {
            ANDSxw_REG(s1, s1, s3);
        } else {
            ANDxw_REG(s1, s1, s3);
        }
    }
    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_CF | X_AF | X_OF) {
        MOV32w(s3, (1<<F_CF)|(1<<F_AF)|(1<<F_OF));
        BICw(xFlags, xFlags, s3);
    }
    IFX(X_ZF) {
        CSETw(s3, cEQ);
        BFIw(xFlags, s3, F_ZF, 1);
    }
    IFX(X_SF) {
        LSRxw(s3, s1, (rex.w)?63:31);
        BFIw(xFlags, s3, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit OR8 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch, s4 can be same as s2 (and so s2 destroyed)
void emit_or8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    MAYUSE(s2);
    IFX(X_PEND) {
        SET_DF(s3, d_or8);
    } else IFX(X_ALL) {
        SET_DFNONE(s3);
    }
    ORRw_REG(s1, s1, s2);
    IFX(X_PEND) {
        STRB_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_CF | X_AF | X_OF) {
        MOV32w(s3, (1<<F_CF)|(1<<F_AF)|(1<<F_OF));
        BICw(xFlags, xFlags, s3);
    }
    COMP_ZFSF(s1, 8)
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit OR8 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_or8c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    IFX(X_PEND) {
        SET_DF(s4, d_or8);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    int mask = convert_bitmask_w(c);
    if(mask) {
        ORRw_mask(s1, s1, mask&0x3F, (mask>>6)&0x3F);
    } else {
        MOV32w(s3, c&0xff);
        ORRw_REG(s1, s1, s3);
    }
    IFX(X_PEND) {
        STRB_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_CF | X_AF | X_OF) {
        MOV32w(s3, (1<<F_CF)|(1<<F_AF)|(1<<F_OF));
        BICw(xFlags, xFlags, s3);
    }
    COMP_ZFSF(s1, 8)
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit XOR8 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch, s4 can be same as s2 (and so s2 destroyed)
void emit_xor8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    MAYUSE(s2);
    IFX(X_PEND) {
        SET_DF(s3, d_xor8);
    } else IFX(X_ALL) {
        SET_DFNONE(s3);
    }
    EORx_REG(s1, s1, s2);
    IFX(X_PEND) {
        STRB_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_CF | X_AF | X_OF) {
        MOV32w(s3, (1<<F_CF)|(1<<F_AF)|(1<<F_OF));
        BICw(xFlags, xFlags, s3);
    }
    COMP_ZFSF(s1, 8)
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit XOR8 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_xor8c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    IFX(X_PEND) {
        SET_DF(s4, d_xor8);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    int mask = convert_bitmask_w(c);
    if(mask) {
        EORw_mask(s1, s1, mask&0x3F, (mask>>6)&0x3F);
    } else {
        MOV32w(s3, c&0xff);
        EORw_REG(s1, s1, s3);
    }
    IFX(X_PEND) {
        STRB_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_CF | X_AF | X_OF) {
        MOV32w(s3, (1<<F_CF)|(1<<F_AF)|(1<<F_OF));
        BICw(xFlags, xFlags, s3);
    }
    COMP_ZFSF(s1, 8)
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit AND8 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch, s4 can be same as s2 (and so s2 destroyed)
void emit_and8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    MAYUSE(s2);
    IFX(X_PEND) {
        SET_DF(s3, d_and8);
    } else IFX(X_ALL) {
        SET_DFNONE(s3);
    }
    IFX(X_ZF) {
        ANDSw_REG(s1, s1, s2);
    } else {
        ANDw_REG(s1, s1, s2);
    }
    IFX(X_PEND) {
        STRB_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_CF | X_AF | X_OF) {
        MOV32w(s3, (1<<F_CF)|(1<<F_AF)|(1<<F_OF));
        BICw_REG(xFlags, xFlags, s3);
    }
    IFX(X_ZF) {
        CSETw(s3, cEQ);
        BFIw(xFlags, s3, F_ZF, 1);
    }
    IFX(X_SF) {
        LSRw(s3, s1, 7);
        BFIw(xFlags, s3, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit AND8 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_and8c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    IFX(X_PEND) {
        SET_DF(s4, d_and8);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    int mask = convert_bitmask_w(c);
    if(mask) {
        IFX(X_ZF) {
            ANDSw_mask(s1, s1, mask&0x3F, (mask>>6)&0x3F);
        } else {
            ANDw_mask(s1, s1, mask&0x3F, (mask>>6)&0x3F);
        }
    } else {
        MOV32w(s3, c&0xff);
        IFX(X_ZF) {
            ANDSw_REG(s1, s1, s3);
        } else {
            ANDw_REG(s1, s1, s3);
        }
    }
    IFX(X_PEND) {
        STRB_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_CF | X_AF | X_OF) {
        MOV32w(s3, (1<<F_CF)|(1<<F_AF)|(1<<F_OF));
        BICw(xFlags, xFlags, s3);
    }
    IFX(X_ZF) {
        CSETw(s3, cEQ);
        BFIw(xFlags, s3, F_ZF, 1);
    }
    IFX(X_SF) {
        LSRw(s3, s1, 7);
        BFIw(xFlags, s3, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}


// emit OR16 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch, s4 can be same as s2 (and so s2 destroyed)
void emit_or16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    MAYUSE(s2);
    IFX(X_PEND) {
        SET_DF(s3, d_or16);
    } else IFX(X_ALL) {
        SET_DFNONE(s3);
    }
    ORRw_REG(s1, s1, s2);
    IFX(X_PEND) {
        STRH_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_CF | X_AF | X_OF) {
        MOV32w(s3, (1<<F_CF)|(1<<F_AF)|(1<<F_OF));
        BICw(xFlags, xFlags, s3);
    }
    COMP_ZFSF(s1, 16)
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit OR16 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
//void emit_or16c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
//{
//    IFX(X_PEND) {
//        MOVW(s3, c);
//        STR_IMM9(s1, xEmu, offsetof(x64emu_t, op1));
//        STR_IMM9(s3, xEmu, offsetof(x64emu_t, op2));
//        SET_DF(s4, d_or16);
//    } else IFX(X_ALL) {
//        SET_DFNONE(s4);
//    }
//    if(c>=0 && c<256) {
//        IFX(X_ALL) {
//            ORRS_IMM8(s1, s1, c, 0);
//        } else {
//            ORR_IMM8(s1, s1, c, 0);
//        }
//    } else {
//        IFX(X_PEND) {} else {MOVW(s3, c);}
//        IFX(X_ALL) {
//            ORRS_REG_LSL_IMM5(s1, s1, s3, 0);
//        } else {
//            ORR_REG_LSL_IMM5(s1, s1, s3, 0);
//        }
//    }
//    IFX(X_PEND) {
//        STR_IMM9(s1, xEmu, offsetof(x64emu_t, res));
//    }
//    IFX(X_CF | X_AF | X_ZF) {
//        BIC_IMM8(xFlags, xFlags, (1<<F_CF)|(1<<F_AF)|(1<<F_ZF), 0);
//    }
//    IFX(X_OF) {
//        BIC_IMM8(xFlags, xFlags, 0b10, 0x0b);
//    }
//    IFX(X_ZF) {
//        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
//    }
//    IFX(X_SF) {
//        MOV_REG_LSR_IMM5(s3, s1, 15);
//        BFI(xFlags, s3, F_SF, 1);
//    }
//    IFX(X_PF) {
//        emit_pf(dyn, ninst, s1, s3, s4);
//    }
//}

// emit XOR16 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch, s4 can be same as s2 (and so s2 destroyed)
void emit_xor16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    MAYUSE(s2);
    IFX(X_PEND) {
        SET_DF(s3, d_xor16);
    } else IFX(X_ALL) {
        SET_DFNONE(s3);
    }
    EORw_REG(s1, s1, s2);
    IFX(X_PEND) {
        STRH_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_CF | X_AF | X_OF) {
        MOV32w(s3, (1<<F_CF)|(1<<F_AF)|(1<<F_OF));
        BICw(xFlags, xFlags, s3);
    }
    COMP_ZFSF(s1, 16)
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit XOR16 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
//void emit_xor16c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
//{
//    IFX(X_PEND) {
//        MOVW(s3, c);
//        STR_IMM9(s1, xEmu, offsetof(x64emu_t, op1));
//        STR_IMM9(s3, xEmu, offsetof(x64emu_t, op2));
//        SET_DF(s4, d_xor16);
//    } else IFX(X_ALL) {
//        SET_DFNONE(s4);
//    }
//    if(c>=0 && c<256) {
//        IFX(X_ALL) {
//            XORS_IMM8(s1, s1, c);
//        } else {
//            XOR_IMM8(s1, s1, c);
//        }
//    } else {
//        IFX(X_PEND) {} else {MOVW(s3, c);}
//        IFX(X_ALL) {
//            XORS_REG_LSL_IMM5(s1, s1, s3, 0);
//        } else {
//            XOR_REG_LSL_IMM5(s1, s1, s3, 0);
//        }
//    }
//    IFX(X_PEND) {
//        STR_IMM9(s1, xEmu, offsetof(x64emu_t, res));
//    }
//    IFX(X_CF | X_AF | X_ZF) {
//        BIC_IMM8(xFlags, xFlags, (1<<F_CF)|(1<<F_AF)|(1<<F_ZF), 0);
//    }
//    IFX(X_OF) {
//        BIC_IMM8(xFlags, xFlags, 0b10, 0x0b);
//    }
//    IFX(X_ZF) {
//        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
//    }
//    IFX(X_SF) {
//        MOV_REG_LSR_IMM5(s3, s1, 15);
//        BFI(xFlags, s3, F_SF, 1);
//    }
//    IFX(X_PF) {
//        emit_pf(dyn, ninst, s1, s3, s4);
//    }
//}

// emit AND16 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch, s4 can be same as s2 (and so s2 destroyed)
void emit_and16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    MAYUSE(s2);
    IFX(X_PEND) {
        SET_DF(s3, d_and16);
    } else IFX(X_ALL) {
        SET_DFNONE(s3);
    }
    IFX(X_ALL) {
        ANDSw_REG(s1, s1, s2);
    } else {
        ANDw_REG(s1, s1, s2);
    }
    IFX(X_PEND) {
        STRH_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_CF | X_AF | X_OF) {
        MOV32w(s3, (1<<F_CF)|(1<<F_AF)|(1<<F_OF));
        BICw_REG(xFlags, xFlags, s3);
    }
    IFX(X_ZF) {
        CSETw(s3, cEQ);
        BFIw(xFlags, s3, F_ZF, 1);
    }
    IFX(X_SF) {
        LSRw(s3, s1, 15);
        BFIw(xFlags, s3, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit AND16 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
//void emit_and16c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
//{
//    IFX(X_PEND) {
//        MOVW(s3, c);
//        STR_IMM9(s1, xEmu, offsetof(x64emu_t, op1));
//        STR_IMM9(s3, xEmu, offsetof(x64emu_t, op2));
//        SET_DF(s4, d_and16);
//    } else IFX(X_ALL) {
//        SET_DFNONE(s4);
//    }
//    if(c>=0 && c<256) {
//        IFX(X_ALL) {
//            ANDS_IMM8(s1, s1, c);
//        } else {
//            AND_IMM8(s1, s1, c);
//        }
//    } else {
//        IFX(X_PEND) {} else {MOVW(s3, c);}
//        IFX(X_ALL) {
//            ANDS_REG_LSL_IMM5(s1, s1, s3, 0);
//        } else {
//            AND_REG_LSL_IMM5(s1, s1, s3, 0);
//        }
//    }
//    IFX(X_PEND) {
//        STR_IMM9(s1, xEmu, offsetof(x64emu_t, res));
//    }
//    IFX(X_CF | X_AF | X_ZF) {
//        BIC_IMM8(xFlags, xFlags, (1<<F_CF)|(1<<F_AF)|(1<<F_ZF), 0);
//    }
//    IFX(X_OF) {
//        BIC_IMM8(xFlags, xFlags, 0b10, 0x0b);
//    }
//    IFX(X_ZF) {
//        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
//    }
//    IFX(X_SF) {
//        MOV_REG_LSR_IMM5(s3, s1, 15);
//        BFI(xFlags, s3, F_SF, 1);
//    }
//    IFX(X_PF) {
//        emit_pf(dyn, ninst, s1, s3, s4);
//    }
//}
