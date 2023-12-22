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

// emit ADD32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_add32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4)
{
    MAYUSE(s2);
    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRxw_U12(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, rex.w?d_add64:d_add32b);
    } else IFX(X_ALL) {
        SET_DFNONE(s3);
    }
    IFX(X_AF) {
        ORRxw_REG(s3, s1, s2);    // s3 = op1 | op2
        ANDxw_REG(s4, s1, s2);    // s4 = op1 & op2
    }
    IFX(X_ALL) {
        ADDSxw_REG(s1, s1, s2);
    } else {
        ADDxw_REG(s1, s1, s2);
    }
    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_AF) {
        BICxw_REG(s3, s3, s1);   // s3 = (op1 | op2) & ~ res
        ORRxw_REG(s3, s3, s4);   // s3 = (op1 & op2) | ((op1 | op2) & ~ res)
        LSRxw(s4, s3, 3);
        BFIxw(xFlags, s4, F_AF, 1);    // AF: bc & 0x08
    }
    IFX(X_ZF) {
        CSETw(s4, cEQ);
        BFIw(xFlags, s4, F_ZF, 1);
    }
    IFX(X_CF) {
        CSETw(s4, cCS);
        BFIw(xFlags, s4, F_CF, 1);
    }
    IFX(X_OF) {
        CSETw(s4, cVS);
        BFIw(xFlags, s4, F_OF, 1);
    }
    IFX(X_SF) {
        LSRxw(s3, s1, (rex.w)?63:31);
        BFIx(xFlags, s3, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit ADD32 instruction, from s1, constant c, store result in s1 using s3 and s4 as scratch
void emit_add32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4, int s5)
{
    MAYUSE(s5);
    if(s1==xRSP && (!dyn->insts || dyn->insts[ninst].x64.gen_flags==X_PEND))
    {
        // special case when doing math on ESP and only PEND is needed: ignoring it!
        if(c>=0 && c<0x1000) {
            ADDx_U12(s1, s1, c);
        } else {
            MOV64x(s3, c);
            ADDx_REG(s1, s1, s3);
        }
        return;
    }
    IFX(X_PEND) {
        MOV64xw(s5, c);
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRxw_U12(s5, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, rex.w?d_add64:d_add32b);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    IFX(X_AF) {
        IFX(X_PEND) {} else {MOV64xw(s5, c);}
        ORRxw_REG(s3, s1, s5);      // s3 = op1 | op2
        ANDxw_REG(s4, s1, s5);      // s4 = op1 & op2
    }
    if(c>=0 && c<0x1000) {
        IFX(X_ALL) {
            ADDSxw_U12(s1, s1, c);
        } else {
            ADDxw_U12(s1, s1, c);
        }
    } else {
        IFX(X_PEND|X_AF) {} else {MOV64xw(s5, c);}
        IFX(X_ALL) {
            ADDSxw_REG(s1, s1, s5);
        } else {
            ADDxw_REG(s1, s1, s5);
        }
    }
    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_AF) {
        BICxw_REG(s3, s3, s1);   // s3 = (op1 | op2) & ~ res
        ORRxw_REG(s3, s3, s4);   // s3 = (op1 & op2) | ((op1 | op2) & ~ res)
        LSRxw(s4, s3, 3);
        BFIxw(xFlags, s4, F_AF, 1);    // AF: bc & 0x08
    }
    IFX(X_ZF) {
        CSETw(s4, cEQ);
        BFIw(xFlags, s4, F_ZF, 1);
    }
    IFX(X_CF) {
        CSETw(s4, cCS);
        BFIw(xFlags, s4, F_CF, 1);
    }
    IFX(X_OF) {
        CSETw(s4, cVS);
        BFIw(xFlags, s4, F_OF, 1);
    }
    IFX(X_SF) {
        LSRxw(s3, s1, (rex.w)?63:31);
        BFIx(xFlags, s3, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit SUB32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_sub32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4)
{
    MAYUSE(s2);
    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRxw_U12(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, rex.w?d_sub64:d_sub32);
    } else IFX(X_ALL) {
        SET_DFNONE(s3);
    }
    IFX(X_AF) {
        ORNxw_REG(s3, s2, s1);  // s3 = ~op1 | op2
        BICxw(s4, s2, s1);      // s4 = ~op1 & op2
    }
    IFX(X_ALL) {
        SUBSxw_REG(s1, s1, s2);
    } else {
        SUBxw_REG(s1, s1, s2);
    }
    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_AF) {
        ANDxw_REG(s3, s3, s1);   // s3 = (~op1 | op2) & res
        ORRxw_REG(s3, s3, s4);   // s3 = (~op1 & op2) | ((~op1 | op2) & res)
        LSRxw(s4, s3, 3);
        BFIx(xFlags, s4, F_AF, 1);    // AF: bc & 0x08
    }
    IFX(X_ZF) {
        CSETw(s4, cEQ);
        BFIw(xFlags, s4, F_ZF, 1);
    }
    IFX(X_CF) {
        // inverted carry
        CSETw(s4, cCC);
        BFIw(xFlags, s4, F_CF, 1);
    }
    IFX(X_OF) {
        CSETw(s4, cVS);
        BFIw(xFlags, s4, F_OF, 1);
    }
    IFX(X_SF) {
        LSRxw(s3, s1, (rex.w)?63:31);
        BFIx(xFlags, s3, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit SUB32 instruction, from s1, constant c, store result in s1 using s3 and s4 as scratch
void emit_sub32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4, int s5)
{
    MAYUSE(s5);
    if(s1==xRSP && (!dyn->insts || dyn->insts[ninst].x64.gen_flags==X_PEND))
    {
        // special case when doing math on RSP and only PEND is needed: ignoring it!
        if(c>=0 && c<0x1000) {
            SUBxw_U12(s1, s1, c);
        } else {
            MOV64xw(s5, c);
            SUBxw_REG(s1, s1, s5);
        }
        return;
    }
    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, op1));
        MOV64xw(s5, c);
        STRxw_U12(s5, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, rex.w?d_sub64:d_sub32);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    IFX(X_AF) {
        IFX(X_PEND) {} else {MOV64xw(s5, c);}
        ORNxw_REG(s3, s5, s1);      // s3 = ~op1 | op2
        BICxw_REG(s4, s5, s1);      // s4 = ~op1 & op2
    }
    if(c>=0 && c<0x1000) {
        IFX(X_ALL) {
            SUBSxw_U12(s1, s1, c);
        } else {
            SUBxw_U12(s1, s1, c);
        }
    } else {
        IFX(X_PEND|X_AF) {} else {MOV64xw(s5, c);}
        IFX(X_ALL) {
            SUBSxw_REG(s1, s1, s5);
        } else {
            SUBxw_REG(s1, s1, s5);
        }
    }
    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_AF) {
        ANDxw_REG(s3, s3, s1);   // s3 = (~op1 | op2) & res
        ORRxw_REG(s3, s3, s4);   // s3 = (~op1 & op2) | ((~op1 | op2) & res)
        LSRxw(s4, s3, 3);
        BFIw(xFlags, s4, F_AF, 1);    // AF: bc & 0x08
    }
    IFX(X_ZF) {
        CSETw(s4, cEQ);
        BFIw(xFlags, s4, F_ZF, 1);
    }
    IFX(X_CF) {
        // inverted carry
        CSETw(s4, cCC);
        BFIw(xFlags, s4, F_CF, 1);
    }
    IFX(X_OF) {
        CSETw(s4, cVS);
        BFIw(xFlags, s4, F_OF, 1);
    }
    IFX(X_SF) {
        LSRxw(s3, s1, (rex.w)?63:31);
        BFIx(xFlags, s3, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit ADD8 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_add8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    MAYUSE(s2);
    IFX(X_PEND) {
        STRB_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRB_U12(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, d_add8);
    } else IFX(X_ALL) {
        SET_DFNONE(s3);
    }
    IFX(X_AF | X_OF) {
        ORRw_REG(s3, s1, s2);    // s3 = op1 | op2
        ANDw_REG(s4, s1, s2);    // s4 = op1 & op2
    }
    ADDw_REG(s1, s1, s2);
    IFX(X_AF|X_OF) {
        BICw_REG(s3, s3, s1);   // s3 = (op1 | op2) & ~ res
        ORRw_REG(s3, s3, s4);   // s3 = (op1 & op2) | ((op1 | op2) & ~ res)
        IFX(X_AF) {
            LSRw(s4, s3, 3);
            BFIw(xFlags, s4, F_AF, 1);    // AF: bc & 0x08
        }
        IFX(X_OF) {
            LSRw(s4, s3, 6);
            EORw_REG_LSR(s4, s4, s4, 1);
            BFIw(xFlags, s4, F_OF, 1);    // OF: ((bc >> 6) ^ ((bc>>6)>>1)) & 1
        }
    }
    IFX(X_CF) {
        BFXILw(xFlags, s1, 8, 1);
    }
    IFX(X_PEND) {
        STRH_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    COMP_ZFSF(s1, 8)
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit ADD8 instruction, from s1, const c, store result in s1 using s3 and s4 as scratch
void emit_add8c(dynarec_arm_t* dyn, int ninst, int s1, int c, int s3, int s4)
{
    IFX(X_PEND) {
        MOV32w(s4, c&0xff);
        STRB_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRB_U12(s4, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, d_add8);
    } else IFX(X_ALL) {
        SET_DFNONE(s3);
    }
    IFX(X_AF | X_OF) {
        IFX(X_PEND) {} else {MOV32w(s4, c&0xff);}
        ORRw_REG(s3, s1, s4);       // s3 = op1 | op2
        ANDw_REG(s4, s1, s4);       // s4 = op1 & op2
    }
    ADDw_U12(s1, s1, c);

    IFX(X_AF|X_OF) {
        BICw_REG(s3, s3, s1);   // s3 = (op1 | op2) & ~ res
        ORRw_REG(s3, s3, s4);   // s4 = (op1 & op2) | ((op1 | op2) & ~ res)
        IFX(X_AF) {
            LSRw(s4, s3, 3);
            BFIw(xFlags, s4, F_AF, 1);    // AF: bc & 0x08
        }
        IFX(X_OF) {
            LSRw(s4, s3, 6);
            EORw_REG_LSR(s4, s4, s4, 1);
            BFIw(xFlags, s4, F_OF, 1);    // OF: ((bc >> 6) ^ ((bc>>6)>>1)) & 1
        }
    }
    IFX(X_CF) {
        BFXILw(xFlags, s1, 8, 1);
    }
    IFX(X_PEND) {
        STRH_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    COMP_ZFSF(s1, 8)
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit SUB8 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_sub8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    MAYUSE(s2);
    IFX(X_PEND) {
        STRB_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRB_U12(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, d_sub8);
    } else IFX(X_ALL) {
        SET_DFNONE(s3);
    }
    IFX(X_AF|X_OF|X_CF) {
        MVNw_REG(s3, s1);
        ORRw_REG(s3, s3, s2);    // s3 = ~op1 | op2
        BICw_REG(s4, s2, s1);    // s4 = ~op1 & op2
    }

    SUBw_REG(s1, s1, s2);
    IFX(X_PEND) {
        STRB_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_AF|X_OF|X_CF) {
        ANDw_REG(s3, s3, s1);   // s3 = (~op1 | op2) & res
        ORRw_REG(s3, s3, s4);   // s3 = (~op1 & op2) | ((~op1 | op2) & res)
        IFX(X_CF) {
            LSRw(s4, s3, 7);
            BFIw(xFlags, s4, F_CF, 1);    // CF : bc & 0x80
        }
        IFX(X_AF) {
            LSRw(s4, s3, 3);
            BFIw(xFlags, s4, F_AF, 1);    // AF: bc & 0x08
        }
        IFX(X_OF) {
            LSRw(s4, s3, 6);
            EORw_REG_LSR(s4, s4, s4, 1);
            BFIw(xFlags, s4, F_OF, 1);    // OF: ((bc >> 6) ^ ((bc>>6)>>1)) & 1
        }
    }
    COMP_ZFSF(s1, 8)
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit SUB8 instruction, from s1, constant c, store result in s1 using s3 and s4 as scratch
void emit_sub8c(dynarec_arm_t* dyn, int ninst, int s1, int c, int s3, int s4, int s5)
{
    MAYUSE(s5);
    IFX(X_ALL|X_PEND) {
        MOV32w(s5, c&0xff);
    }
    IFX(X_PEND) {
        STRB_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRB_U12(s5, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, d_sub8);
    } else IFX(X_ALL) {
        SET_DFNONE(s3);
    }
    IFX(X_AF|X_OF|X_CF) {
        MVNw_REG(s3, s1);
        ORRw_REG(s3, s3, s5);    // s3 = ~op1 | op2
        BICw_REG(s4, s5, s1);    // s4 = ~op1 & op2
    }
    IFX(X_ALL) {
        SUBw_REG(s1, s1, s5);
    } else {
        SUBw_U12(s1, s1, c&0xff);
    }
    IFX(X_PEND) {
        STRB_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_AF|X_OF|X_CF) {
        ANDw_REG(s3, s3, s1);   // s3 = (~op1 | op2) & res
        ORRw_REG(s3, s3, s4);   // s3 = (~op1 & op2) | ((~op1 | op2) & res)
        IFX(X_CF) {
            LSRw(s4, s3, 7);
            BFIw(xFlags, s4, F_CF, 1);    // CF : bc & 0x80
        }
        IFX(X_AF) {
            LSRw(s4, s3, 3);
            BFIw(xFlags, s4, F_AF, 1);    // AF: bc & 0x08
        }
        IFX(X_OF) {
            LSRw(s4, s3, 6);
            EORw_REG_LSR(s4, s4, s4, 1);
            BFIw(xFlags, s4, F_OF, 1);    // OF: ((bc >> 6) ^ ((bc>>6)>>1)) & 1
        }
    }
    COMP_ZFSF(s1, 8)
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit ADD16 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_add16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    MAYUSE(s2);
    IFX(X_PEND) {
        STRH_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRH_U12(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, d_add16);
    } else IFX(X_ALL) {
        SET_DFNONE(s3);
    }
    IFX(X_AF | X_OF) {
        ORRw_REG(s3, s1, s2);    // s3 = op1 | op2
        ANDw_REG(s4, s1, s2);    // s4 = op1 & op2
    }
    ADDw_REG(s1, s1, s2);

    IFX(X_AF|X_OF) {
        BICw_REG(s3, s3, s1);   // s3 = (op1 | op2) & ~ res
        ORRw_REG(s3, s3, s4);   // s3 = (op1 & op2) | ((op1 | op2) & ~ res)
        IFX(X_AF) {
            LSRw(s4, s3, 3);
            BFIw(xFlags, s4, F_AF, 1);    // AF: bc & 0x08
        }
        IFX(X_OF) {
            LSRw(s4, s3, 14);
            EORw_REG_LSR(s4, s4, s4, 1);
            BFIw(xFlags, s4, F_OF, 1);    // OF: ((bc >> 14) ^ ((bc>>14)>>1)) & 1
        }
    }
    IFX(X_CF) {
        BFXILw(xFlags, s1, 16, 1);
    }
    IFX(X_PEND) {
        STRw_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    COMP_ZFSF(s1, 16)
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit ADD16 instruction, from s1, const c, store result in s1 using s3 and s4 as scratch
//void emit_add16c(dynarec_arm_t* dyn, int ninst, int s1, int c, int s3, int s4)
//{
//    IFX(X_PEND) {
//        MOVW(s3, c);
//        STR_IMM9(s1, xEmu, offsetof(x64emu_t, op1));
//        STR_IMM9(s3, xEmu, offsetof(x64emu_t, op2));
//        SET_DF(s4, d_add16);
//    } else IFX(X_ALL) {
//        SET_DFNONE(s4);
//    }
//    IFX(X_AF | X_OF) {
//        MOV_REG(s4, s1);
//    }
//    if(c>=0 && c<256) {
//        ADD_IMM8(s1, s1, c);
//    } else {
//        IFX(X_PEND) {} else {MOVW(s3, c);}
//        ADD_REG_LSL_IMM5(s1, s1, s3, 0);
//    }
//
//    IFX(X_AF|X_OF) {
//        if(c>=0 && c<256) {
//            ORR_IMM8(s3, s4, c, 0);     // s3 = op1 | op2
//            AND_IMM8(s4, s4, c);        // s4 = op1 & op2
//        } else {
//            ORR_REG_LSL_IMM5(s3, s3, s4, 0);    // s3 = op1 | op2
//            PUSH(xSP, 1<<s3);
//            MOVW(s3, c);
//            AND_REG_LSL_IMM5(s4, s4, s3, 0);    // s4 = op1 & op2
//            POP(xSP, 1<<s3);
//        }
//
//        BIC_REG_LSL_IMM5(s3, s3, s1, 0);   // s3 = (op1 | op2) & ~ res
//        ORR_REG_LSL_IMM5(s3, s3, s4, 0);   // s3 = (op1 & op2) | ((op1 | op2) & ~ res)
//        IFX(X_AF) {
//            MOV_REG_LSR_IMM5(s4, s3, 3);
//            BFI(xFlags, s4, F_AF, 1);    // AF: bc & 0x08
//        }
//        IFX(X_OF) {
//            MOV_REG_LSR_IMM5(s4, s3, 14);
//            XOR_REG_LSR_IMM8(s4, s4, s4, 1);
//            BFI(xFlags, s4, F_OF, 1);    // OF: ((bc >> 14) ^ ((bc>>14)>>1)) & 1
//        }
//    }
//    IFX(X_CF) {
//        MOV_REG_LSR_IMM5(s3, s1, 16);
//        BFI(xFlags, s3, F_CF, 1);
//    }
//    IFX(X_PEND) {
//        STR_IMM9(s1, xEmu, offsetof(x64emu_t, res));
//    }
//    IFX(X_ZF) {
//        UXTH(s1, s1, 0);
//        TSTS_REG_LSL_IMM5(s1, s1, 0);
//        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
//        BIC_IMM8_COND(cNE, xFlags, xFlags, 1<<F_ZF, 0);
//    }
//    IFX(X_SF) {
//        MOV_REG_LSR_IMM5(s3, s1, 15);
//        BFI(xFlags, s3, F_SF, 1);
//    }
//    IFX(X_PF) {
//        emit_pf(dyn, ninst, s1, s3, s4);
//    }
//}

// emit SUB16 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_sub16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    MAYUSE(s2);
    IFX(X_PEND) {
        STRH_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRH_U12(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, d_sub16);
    } else IFX(X_ALL) {
        SET_DFNONE(s3);
    }
    IFX(X_AF|X_OF|X_CF) {
        ORNw_REG(s3, s2, s1);    // s3 = ~op1 | op2
        BICw_REG(s4, s2, s1);    // s4 = ~op1 & op2
    }

    SUBw_REG(s1, s1, s2);
    IFX(X_PEND) {
        STRH_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_AF|X_OF|X_CF) {
        ANDw_REG(s3, s3, s1);   // s3 = (~op1 | op2) & res
        ORRw_REG(s3, s3, s4);   // s3 = (~op1 & op2) | ((~op1 | op2) & res)
        IFX(X_CF) {
            LSRw(s4, s3, 15);
            BFIw(xFlags, s4, F_CF, 1);    // CF : bc & 0x8000
        }
        IFX(X_AF) {
            LSRw(s4, s3, 3);
            BFIw(xFlags, s4, F_AF, 1);    // AF: bc & 0x08
        }
        IFX(X_OF) {
            LSRw(s4, s3, 14);
            EORw_REG_LSR(s4, s4, s4, 1);
            BFIw(xFlags, s4, F_OF, 1);    // OF: ((bc >> 14) ^ ((bc>>14)>>1)) & 1
        }
    }
    COMP_ZFSF(s1, 16)
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit SUB16 instruction, from s1, constant c, store result in s1 using s3 and s4 as scratch
//void emit_sub16c(dynarec_arm_t* dyn, int ninst, int s1, int c, int s3, int s4)
//{
//    IFX(X_PEND) {
//        MOVW(s3, c);
//        STR_IMM9(s1, xEmu, offsetof(x64emu_t, op1));
//        STR_IMM9(s3, xEmu, offsetof(x64emu_t, op2));
//        SET_DF(s4, d_sub16);
//    } else IFX(X_ALL) {
//        SET_DFNONE(s4);
//    }
//    IFX(X_AF|X_OF|X_CF) {
//        MVN_REG_LSL_IMM5(s4, s1, 0);
//    }
//    if(c>=0 && c<255) {
//        SUB_IMM8(s1, s1, c);
//    } else {
//        IFX(X_PEND) {} else {MOVW(s3, c);}
//        SUB_REG_LSL_IMM5(s1, s1, s3, 0);
//    }
//    IFX(X_PEND) {
//        STR_IMM9(s1, xEmu, offsetof(x64emu_t, res));
//    }
//    IFX(X_AF|X_OF|X_CF) {
//        if(c>=0 && c<256) {
//            ORR_IMM8(s3, s4, c, 0);     // s3 = ~op1 | op2
//            AND_IMM8(s4, s4, c);        // s4 = ~op1 & op2
//        } else {
//            ORR_REG_LSL_IMM5(s3, s3, s4, 0);    // s3 = ~op1 | op2
//            PUSH(xSP, 1<<s3);
//            MOVW(s3, c);
//            AND_REG_LSL_IMM5(s4, s4, s3, 0);    // s4 = ~op1 & op2
//            POP(xSP, 1<<s3);
//        }
//        AND_REG_LSL_IMM5(s3, s3, s1, 0);   // s3 = (~op1 | op2) & res
//        ORR_REG_LSL_IMM5(s3, s3, s4, 0);   // s3 = (~op1 & op2) | ((~op1 | op2) & res)
//        IFX(X_CF) {
//            MOV_REG_LSR_IMM5(s4, s3, 15);
//            BFI(xFlags, s4, F_CF, 1);    // CF : bc & 0x8000
//        }
//        IFX(X_AF) {
//            MOV_REG_LSR_IMM5(s4, s3, 3);
//            BFI(xFlags, s4, F_AF, 1);    // AF: bc & 0x08
//        }
//        IFX(X_OF) {
//            MOV_REG_LSR_IMM5(s4, s3, 14);
//            XOR_REG_LSR_IMM8(s4, s4, s4, 1);
//            BFI(xFlags, s4, F_OF, 1);    // OF: ((bc >> 14) ^ ((bc>>14)>>1)) & 1
//        }
//    }
//    IFX(X_ZF) {
//        UXTH(s1, s1, 0);
//        TSTS_REG_LSL_IMM5(s1, s1, 0);
//        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
//        BIC_IMM8_COND(cNE, xFlags, xFlags, 1<<F_ZF, 0);
//    }
//    IFX(X_SF) {
//        MOV_REG_LSR_IMM5(s3, s1, 15);
//        BFI(xFlags, s3, F_SF, 1);
//    }
//    IFX(X_PF) {
//        emit_pf(dyn, ninst, s1, s3, s4);
//    }
//}

// emit INC32 instruction, from s1, store result in s1 using s3 and s4 as scratch
void emit_inc32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s3, int s4)
{
    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, rex.w?d_inc64:d_inc32);
    } else IFX(X_ZF|X_OF|X_AF|X_SF|X_PF) {
        SET_DFNONE(s4);
    }
    IFX(X_AF) {
        if(rex.w) {
            ORRx_mask(s3, s1, 1, 0, 0);          // s3 = op1 | op2
            ANDx_mask(s4, s1, 1, 0, 0);          // s4 = op1 & op2
        } else {
            ORRw_mask(s3, s1, 0, 0);             // s3 = op1 | op2
            ANDw_mask(s4, s1, 0, 0);             // s4 = op1 & op2
        }
    }
    IFX(X_ZF|X_OF) {
        ADDSxw_U12(s1, s1, 1);
    } else {
        ADDxw_U12(s1, s1, 1);
    }
    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_AF) {
        BICxw_REG(s3, s3, s1);   // s3 = (op1 | op2) & ~ res
        ORRxw_REG(s3, s3, s4);   // s3 = (op1 & op2) | ((op1 | op2) & ~ res)
        LSRxw(s4, s3, 3);
        BFIxw(xFlags, s4, F_AF, 1);    // AF: bc & 0x08
    }
    IFX(X_ZF) {
        CSETw(s4, cEQ);
        BFIw(xFlags, s4, F_ZF, 1);
    }
    IFX(X_OF) {
        CSETw(s4, cVS);
        BFIw(xFlags, s4, F_OF, 1);
    }
    IFX(X_SF) {
        LSRxw(s3, s1, rex.w?63:31);
        BFIxw(xFlags, s3, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit INC8 instruction, from s1, store result in s1 using s3 and s4 as scratch
void emit_inc8(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4)
{
    IFX(X_PEND) {
        STRB_U12(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s3, d_inc8);
    } else IFX(X_ZF|X_OF|X_AF|X_SF|X_PF) {
        SET_DFNONE(s3);
    }
    IFX(X_AF | X_OF) {
        ORRw_mask(s3, s1, 0, 0);     // s3 = op1 | op2
        ANDw_mask(s4, s1, 0, 0);        // s4 = op1 & op2
    }
    ADDw_U12(s1, s1, 1);
    IFX(X_PEND) {
        STRB_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_AF|X_OF) {
        BICw_REG(s3, s3, s1);   // s3 = (op1 | op2) & ~ res
        ORRw_REG(s3, s3, s4);   // s4 = (op1 & op2) | ((op1 | op2) & ~ res)
        IFX(X_AF) {
            LSRw(s4, s3, 3);
            BFIw(xFlags, s4, F_AF, 1);    // AF: bc & 0x08
        }
        IFX(X_OF) {
            LSRw(s4, s3, 6);
            EORw_REG_LSR(s4, s4, s4, 1);
            BFIw(xFlags, s4, F_OF, 1);    // OF: ((bc >> 6) ^ ((bc>>6)>>1)) & 1
        }
    }
    COMP_ZFSF(s1, 8)
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit INC16 instruction, from s1, store result in s1 using s3 and s4 as scratch
void emit_inc16(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4)
{
    IFX(X_PEND) {
        STRH_U12(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s3, d_inc16);
    } else IFX(X_ZF|X_OF|X_AF|X_SF|X_PF) {
        SET_DFNONE(s3);
    }
    IFX(X_AF | X_OF) {
        MOVw_REG(s4, s1);
    }
    ADDw_U12(s1, s1, 1);
    IFX(X_PEND) {
        STRH_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_AF|X_OF) {
        ORRw_mask(s3, s4, 0, 0);    // s3 = op1 | op2
        ANDw_mask(s4, s4, 0, 0);    // s4 = op1 & op2
        BICw_REG(s3, s3, s1);       // s3 = (op1 | op2) & ~ res
        ORRw_REG(s3, s3, s4);       // s3 = (op1 & op2) | ((op1 | op2) & ~ res)
        IFX(X_AF) {
            LSRw(s4, s3, 3);
            BFIw(xFlags, s4, F_AF, 1);    // AF: bc & 0x08
        }
        IFX(X_OF) {
            LSRw(s4, s3, 14);
            EORw_REG_LSR(s4, s4, s4, 1);
            BFIw(xFlags, s4, F_OF, 1);    // OF: ((bc >> 14) ^ ((bc>>14)>>1)) & 1
        }
    }
    COMP_ZFSF(s1, 16)
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit DEC32 instruction, from s1, store result in s1 using s3 and s4 as scratch
void emit_dec32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s3, int s4)
{
    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s4, rex.w?d_dec64:d_dec32);
    } else IFX(X_ZF|X_OF|X_AF|X_SF|X_PF) {
        SET_DFNONE(s4);
    }
    IFX(X_AF) {
        MVNxw_REG(s3, s1);
        if(rex.w) {
            ANDx_mask(s4, s3, 1, 0, 0);          // s4 = ~op1 & op2
            ORRx_mask(s3, s3, 1, 0, 0);          // s3 = ~op1 | op2
        } else {
            ANDw_mask(s4, s3, 0, 0);             // s4 = ~op1 & op2
            ORRw_mask(s3, s3, 0, 0);             // s3 = ~op1 | op2
        }
    }
    IFX(X_ZF|X_OF) {
        SUBSxw_U12(s1, s1, 1);
    } else {
        SUBxw_U12(s1, s1, 1);
    }
    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_AF) {
        ANDxw_REG(s3, s3, s1);   // s3 = (~op1 | op2) & res
        ORRxw_REG(s3, s3, s4);   // s4 = (~op1 & op2) | ((~op1 | op2) & ~ res)
        LSRxw(s4, s3, 3);
        BFIw(xFlags, s4, F_AF, 1);    // AF: bc & 0x08
    }
    IFX(X_ZF) {
        CSETw(s4, cEQ);
        BFIw(xFlags, s4, F_ZF, 1);
    }
    IFX(X_OF) {
        CSETw(s4, cVS);
        BFIw(xFlags, s4, F_OF, 1);
    }
    IFX(X_SF) {
        LSRxw(s3, s1, rex.w?63:31);
        BFIxw(xFlags, s3, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit DEC8 instruction, from s1, store result in s1 using s3 and s4 as scratch
void emit_dec8(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4)
{
    IFX(X_PEND) {
        STRB_U12(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s3, d_dec8);
    } else IFX(X_ZF|X_OF|X_AF|X_SF|X_PF) {
        SET_DFNONE(s3);
    }
    IFX(X_AF|X_OF) {
        MVNw_REG(s3, s1);
        ANDw_mask(s4, s3, 0, 0);        // s4 = ~op1 & op2
        ORRw_mask(s3, s3, 0, 0);     // s3 = ~op1 | op2
    }
    IFX(X_ZF) {
        SUBSw_U12(s1, s1, 1);
    } else {
        SUBw_U12(s1, s1, 1);
    }
    IFX(X_PEND) {
        STRB_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_AF|X_OF) {
        ANDw_REG(s3, s3, s1);   // s3 = (~op1 | op2) & res
        ORRw_REG(s3, s3, s4);   // s3 = (~op1 & op2) | ((~op1 | op2) & res)
        IFX(X_AF) {
            LSRw(s4, s3, 3);
            BFIw(xFlags, s4, F_AF, 1);    // AF: bc & 0x08
        }
        IFX(X_OF) {
            LSRw(s4, s3, 6);
            EORw_REG_LSR(s4, s4, s4, 1);
            BFIw(xFlags, s4, F_OF, 1);    // OF: ((bc >> 6) ^ ((bc>>6)>>1)) & 1
        }
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

// emit DEC16 instruction, from s1, store result in s1 using s3 and s4 as scratch
void emit_dec16(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4)
{
    IFX(X_PEND) {
        STRH_U12(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s3, d_dec16);
    } else IFX(X_ZF|X_OF|X_AF|X_SF|X_PF) {
        SET_DFNONE(s3);
    }
    IFX(X_AF|X_OF) {
        MVNw_REG(s4, s1);
    }
    IFX(X_ZF) {
        SUBSw_U12(s1, s1, 1);
    } else {
        SUBw_U12(s1, s1, 1);
    }
    IFX(X_PEND) {
        STRH_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_AF|X_OF) {
        ORRw_mask(s3, s4, 0, 0);    // s3 = ~op1 | op2
        ANDw_mask(s4, s4, 0, 0);    // s4 = ~op1 & op2
        ANDw_REG(s3, s3, s1);       // s3 = (~op1 | op2) & res
        ORRw_REG(s3, s3, s4);       // s3 = (~op1 & op2) | ((~op1 | op2) & res)
        IFX(X_AF) {
            LSRw(s4, s3, 3);
            BFIw(xFlags, s4, F_AF, 1);    // AF: bc & 0x08
        }
        IFX(X_OF) {
            LSRw(s4, s3, 14);
            EORw_REG_LSR(s4, s4, s4, 1);
            BFIw(xFlags, s4, F_OF, 1);    // OF: ((bc >> 14) ^ ((bc>>14)>>1)) & 1
        }
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

// emit ADC32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_adc32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4)
{
    MAYUSE(s2);
    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRxw_U12(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, rex.w?d_adc64:d_adc32b);
    } else IFX(X_ALL) {
        SET_DFNONE(s3);
    }
    IFX(X_AF) {
        MOVxw_REG(s4, s1);
    }
    MRS_nzvc(s3);
    BFIx(s3, xFlags, 29, 1); // set C
    MSR_nzvc(s3);      // load CC into ARM CF
    IFX(X_ZF|X_CF|X_OF) {
        ADCSxw_REG(s1, s1, s2);
    } else {
        ADCxw_REG(s1, s1, s2);
    }
    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_AF) {
        ORRxw_REG(s3, s4, s2);    // s3 = op1 | op2
        ANDxw_REG(s4, s4, s2);    // s4 = op1 & op2
        BICxw_REG(s3, s3, s1);   // s3 = (op1 | op2) & ~ res
        ORRxw_REG(s3, s3, s4);   // s4 = (op1 & op2) | ((op1 | op2) & ~ res)
        LSRxw(s4, s3, 3);
        BFIw(xFlags, s4, F_AF, 1);    // AF: bc & 0x08
    }
    IFX(X_ZF) {
        CSETw(s3, cEQ);
        BFIw(xFlags, s3, F_ZF, 1);
    }
    IFX(X_CF) {
        CSETw(s3, cCS);
        BFIw(xFlags, s3, F_CF, 1);
    }
    IFX(X_OF) {
        CSETw(s3, cVS);
        BFIw(xFlags, s3, F_OF, 1);
    }
    IFX(X_SF) {
        LSRx(s3, s1, rex.w?63:31);
        BFIw(xFlags, s3, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit ADC32 instruction, from s1, constant c, store result in s1 using s3 and s4 as scratch
//void emit_adc32c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
//{
//    IFX(X_PEND) {
//        MOV32(s3, c);
//        STR_IMM9(s1, xEmu, offsetof(x64emu_t, op1));
//        STR_IMM9(s3, xEmu, offsetof(x64emu_t, op2));
//        SET_DF(s4, d_adc32);
//    } else IFX(X_ALL) {
//        SET_DFNONE(s4);
//    }
//    IFX(X_AF) {
//        MOV_REG(s4, s1);
//    }
//    MOVS_REG_LSR_IMM5(s3, xFlags, 1);    // load CC into ARM CF
//    if(c>=0 && c<256) {
//        IFX(X_ZF|X_CF|X_OF) {
//            ADCS_IMM8(s1, s1, c);
//        } else {
//            ADC_IMM8(s1, s1, c);
//        }
//    } else {
//        MOV32(s3, c);
//        IFX(X_ZF|X_CF|X_OF) {
//            ADCS_REG_LSL_IMM5(s1, s1, s3, 0);
//        } else {
//            ADC_REG_LSL_IMM5(s1, s1, s3, 0);
//        }
//    }
//    IFX(X_PEND) {
//        STR_IMM9(s1, xEmu, offsetof(x64emu_t, res));
//    }
//    IFX(X_AF) {
//        if(c>=0 && c<256) {
//            ORR_IMM8(s3, s4, c, 0);     // s3 = op1 | op2
//            AND_IMM8(s4, s4, c);        // s4 = op1 & op2
//        } else {
//            ORR_REG_LSL_IMM5(s3, s3, s4, 0);    // s3 = op1 | op2
//            PUSH(xSP, 1<<s3);
//            MOVW(s3, c);
//            AND_REG_LSL_IMM5(s4, s4, s3, 0);    // s4 = op1 & op2
//            POP(xSP, 1<<s3);
//        }
//        BIC_REG_LSL_IMM5(s3, s3, s1, 0);   // s3 = (op1 | op2) & ~ res
//        ORR_REG_LSL_IMM5(s3, s3, s4, 0);   // s4 = (op1 & op2) | ((op1 | op2) & ~ res)
//        MOV_REG_LSR_IMM5(s4, s3, 3);
//        BFI(xFlags, s4, F_AF, 1);    // AF: bc & 0x08
//    }
//    IFX(X_ZF|X_CF) {
//        BIC_IMM8(xFlags, xFlags, (1<<F_ZF)|(1<<F_CF), 0);
//    }
//    IFX(X_ZF) {
//        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
//    }
//    IFX(X_CF) {
//        ORR_IMM8_COND(cCS, xFlags, xFlags, 1<<F_CF, 0);
//    }
//    IFX(X_OF) {
//        ORR_IMM8_COND(cVS, xFlags, xFlags, 0b10, 0x0b);
//        BIC_IMM8_COND(cVC, xFlags, xFlags, 0b10, 0x0b);
//    }
//    IFX(X_SF) {
//        MOV_REG_LSR_IMM5(s3, s1, 31);
//        BFI(xFlags, s3, F_SF, 1);
//    }
//    IFX(X_PF) {
//        emit_pf(dyn, ninst, s1, s3, s4);
//    }
//}

// emit ADC8 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_adc8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    MAYUSE(s2);
    IFX(X_PEND) {
        STRB_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRB_U12(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, d_adc8);
    } else IFX(X_ALL) {
        SET_DFNONE(s3);
    }
    IFX(X_AF | X_OF) {
        MOVw_REG(s4, s1);
    }
    MRS_nzvc(s3);
    BFIx(s3, xFlags, 29, 1); // set C
    MSR_nzvc(s3);      // load CC into ARM CF
    ADCw_REG(s1, s1, s2);
    IFX(X_PEND) {
        STRH_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_AF|X_OF) {
        ORRw_REG(s3, s4, s2);    // s3 = op1 | op2
        ANDw_REG(s4, s4, s2);    // s4 = op1 & op2
        BICw_REG(s3, s3, s1);   // s3 = (op1 | op2) & ~ res
        ORRw_REG(s3, s3, s4);   // s3 = (op1 & op2) | ((op1 | op2) & ~ res)
        IFX(X_AF) {
            LSRw(s4, s3, 3);
            BFIw(xFlags, s4, F_AF, 1);    // AF: bc & 0x08
        }
        IFX(X_OF) {
            LSRw(s4, s3, 6);
            EORw_REG_LSR(s4, s4, s4, 1);
            BFIw(xFlags, s4, F_OF, 1);    // OF: ((bc >> 6) ^ ((bc>>6)>>1)) & 1
        }
    }
    IFX(X_CF) {
        LSRw(s3, s1, 8);
        BFIw(xFlags, s3, F_CF, 1);
    }
    COMP_ZFSF(s1, 8)
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit ADC8 instruction, from s1, const c, store result in s1 using s3 and s4 as scratch
void emit_adc8c(dynarec_arm_t* dyn, int ninst, int s1, int c, int s3, int s4, int s5)
{
    MAYUSE(s5);
    MOV32w(s5, c&0xff);
    emit_adc8(dyn, ninst, s1, s5, s3, s4);
}

// emit ADC16 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_adc16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    MAYUSE(s2);
    IFX(X_PEND) {
        STRH_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRH_U12(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, d_adc16);
    } else IFX(X_ALL) {
        SET_DFNONE(s3);
    }
    IFX(X_AF | X_OF) {
        MOVw_REG(s4, s1);
    }
    MRS_nzvc(s3);
    BFIx(s3, xFlags, 29, 1); // set C
    MSR_nzvc(s3);      // load CC into ARM CF
    ADCw_REG(s1, s1, s2);
    IFX(X_PEND) {
        STRw_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_AF|X_OF) {
        ORRw_REG(s3, s4, s2);    // s3 = op1 | op2
        ANDw_REG(s4, s4, s2);    // s4 = op1 & op2
        BICw_REG(s3, s3, s1);   // s3 = (op1 | op2) & ~ res
        ORRw_REG(s3, s3, s4);   // s3 = (op1 & op2) | ((op1 | op2) & ~ res)
        IFX(X_AF) {
            LSRw(s4, s3, 3);
            BFIw(xFlags, s4, F_AF, 1);    // AF: bc & 0x08
        }
        IFX(X_OF) {
            LSRw(s4, s3, 14);
            EORw_REG_LSR(s4, s4, s4, 1);
            BFIw(xFlags, s4, F_OF, 1);    // OF: ((bc >> 14) ^ ((bc>>14)>>1)) & 1
        }
    }
    IFX(X_CF) {
        LSRw(s3, s1, 16);
        BFIw(xFlags, s3, F_CF, 1);
    }
    COMP_ZFSF(s1, 16)
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit ADC16 instruction, from s1, const c, store result in s1 using s3 and s4 as scratch
//void emit_adc16c(dynarec_arm_t* dyn, int ninst, int s1, int c, int s3, int s4)
//{
//    IFX(X_PEND) {
//        MOVW(s3, c);
//        STR_IMM9(s1, xEmu, offsetof(x64emu_t, op1));
//        STR_IMM9(s3, xEmu, offsetof(x64emu_t, op2));
//        SET_DF(s3, d_adc16);
//    } else IFX(X_ALL) {
//        SET_DFNONE(s3);
//    }
//    IFX(X_AF | X_OF) {
//        MOV_REG(s4, s1);
//    }
//    MOVS_REG_LSR_IMM5(s3, xFlags, 1);    // load CC into ARM CF
//    if(c>=0 && c<256) {
//        ADC_IMM8(s1, s1, c);
//    } else {
//        MOVW(s3, c);
//        ADC_REG_LSL_IMM5(s1, s1, s3, 0);
//    }
//    IFX(X_PEND) {
//        STR_IMM9(s1, xEmu, offsetof(x64emu_t, res));
//    }
//    IFX(X_AF|X_OF) {
//        if(c>=0 && c<256) {
//            ORR_IMM8(s3, s4, c, 0);     // s3 = op1 | op2
//            AND_IMM8(s4, s4, c);        // s4 = op1 & op2
//        } else {
//            ORR_REG_LSL_IMM5(s3, s3, s4, 0);    // s3 = op1 | op2
//            PUSH(xSP, 1<<s3);
//            MOVW(s3, c);
//            AND_REG_LSL_IMM5(s4, s4, s3, 0);    // s4 = op1 & op2
//            POP(xSP, 1<<s3);
//        }
//
//        BIC_REG_LSL_IMM5(s3, s3, s1, 0);   // s3 = (op1 | op2) & ~ res
//        ORR_REG_LSL_IMM5(s3, s3, s4, 0);   // s3 = (op1 & op2) | ((op1 | op2) & ~ res)
//        IFX(X_AF) {
//            MOV_REG_LSR_IMM5(s4, s3, 3);
//            BFI(xFlags, s4, F_AF, 1);    // AF: bc & 0x08
//        }
//        IFX(X_OF) {
//            MOV_REG_LSR_IMM5(s4, s3, 14);
//            XOR_REG_LSR_IMM8(s4, s4, s4, 1);
//            BFI(xFlags, s4, F_OF, 1);    // OF: ((bc >> 14) ^ ((bc>>14)>>1)) & 1
//        }
//    }
//    IFX(X_CF) {
//        MOV_REG_LSR_IMM5(s3, s1, 16);
//        BFI(xFlags, s3, F_CF, 1);
//    }
//    IFX(X_ZF) {
//        UXTH(s1, s1, 0);
//        TSTS_REG_LSL_IMM5(s1, s1, 0);
//        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
//        BIC_IMM8_COND(cNE, xFlags, xFlags, 1<<F_ZF, 0);
//    }
//    IFX(X_SF) {
//        MOV_REG_LSR_IMM5(s3, s1, 15);
//        BFI(xFlags, s3, F_SF, 1);
//    }
//    IFX(X_PF) {
//        emit_pf(dyn, ninst, s1, s3, s4);
//    }
//}

// emit SBB32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_sbb32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4)
{
    MAYUSE(s2);
    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRxw_U12(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, rex.w?d_sbb64:d_sbb32);
    } else IFX(X_ALL) {
        SET_DFNONE(s3);
    }
    EORw_mask(s4, xFlags, 0, 0);        // invert CC because it's reverted for SUB on ARM
    MRS_nzvc(s3);
    BFIx(s3, s4, 29, 1); // set C
    MSR_nzvc(s3);      // load CC into ARM CF
    IFX(X_AF) {
        MVNxw_REG(s4, s1);
    }
    IFX(X_ZF|X_CF|X_OF) {
        SBCSxw_REG(s1, s1, s2);
    } else {
        SBCxw_REG(s1, s1, s2);
    }
    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_AF) {
        ORRxw_REG(s3, s4, s2);    // s3 = ~op1 | op2
        ANDxw_REG(s4, s2, s4);    // s4 = ~op1 & op2
        ANDxw_REG(s3, s3, s1);   // s3 = (~op1 | op2) & res
        ORRxw_REG(s3, s3, s4);   // s4 = (~op1 & op2) | ((~op1 | op2) & res)
        UBFXw(s4, s3, 3, 1);
        BFIw(xFlags, s4, F_AF, 1);    // AF: bc & 0x08
    }
    IFX(X_ZF) {
        CSETw(s3, cEQ);
        BFIw(xFlags, s3, F_ZF, 1);
    }
    IFX(X_CF) {
        // Inverted carry
        CSETw(s3, cCC);
        BFIw(xFlags, s3, F_CF, 1);
    }
    IFX(X_OF) {
        CSETw(s3, cVS);
        BFIw(xFlags, s3, F_OF, 1);
    }
    IFX(X_SF) {
        LSRxw(s3, s1, rex.w?63:31);
        BFIw(xFlags, s3, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit SBB32 instruction, from s1, constant c, store result in s1 using s3 and s4 as scratch
//void emit_sbb32c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
//{
//    IFX(X_PEND) {
//        MOV32(s3, c);
//        STR_IMM9(s1, xEmu, offsetof(x64emu_t, op1));
//        STR_IMM9(s3, xEmu, offsetof(x64emu_t, op2));
//        SET_DF(s4, d_sbb32);
//    } else IFX(X_ALL) {
//        SET_DFNONE(s4);
//    }
//    IFX(X_AF) {
//        MVN_REG_LSL_IMM5(s4, s1, 0);
//    }
//    XOR_IMM8(s3, xFlags, 1);            // invert CC because it's reverted for SUB on ARM
//    MOVS_REG_LSR_IMM5(s3, s3, 1);       // load into ARM CF
//    if(c>=0 && c<256) {
//        IFX(X_ZF|X_CF|X_OF) {
//            SBCS_IMM8(s1, s1, c);
//        } else {
//            SBC_IMM8(s1, s1, c);
//        }
//    } else {
//        MOV32(s3, c);
//        IFX(X_ZF|X_CF|X_OF) {
//            SBCS_REG_LSL_IMM5(s1, s1, s3, 0);
//        } else {
//            SBC_REG_LSL_IMM5(s1, s1, s3, 0);
//        }
//    }
//    IFX(X_PEND) {
//        STR_IMM9(s1, xEmu, offsetof(x64emu_t, res));
//    }
//    IFX(X_AF) {
//        if(c>=0 && c<256) {
//            AND_IMM8(s4, s3, c);                // s4 = ~op1 & op2
//            ORR_IMM8(s3, s3, c, 0);             // s3 = ~op1 | op2
//        } else {
//            ORR_REG_LSL_IMM5(s3, s4, s3, 0);
//            PUSH(xSP, 1<<s3);
//            MOV32(s3, c);
//            AND_REG_LSL_IMM5(s4, s3, s4, 0);
//            POP(xSP, 1<<s3);
//        }
//        AND_REG_LSL_IMM5(s3, s3, s1, 0);   // s3 = (~op1 | op2) & res
//        ORR_REG_LSL_IMM5(s3, s3, s4, 0);   // s4 = (~op1 & op2) | ((~op1 | op2) & ~ res)
//        UBFX(s4, s3, 3, 1);
//        BFI(xFlags, s4, F_AF, 1);    // AF: bc & 0x08
//    }
//    IFX(X_ZF|X_CF) {
//        BIC_IMM8(xFlags, xFlags, (1<<F_ZF)|(1<<F_CF), 0);
//    }
//    IFX(X_ZF) {
//        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
//    }
//    IFX(X_CF) {
//        // Inverted carry
//        ORR_IMM8_COND(cCC, xFlags, xFlags, 1<<F_CF, 0);
//    }
//    IFX(X_OF) {
//        ORR_IMM8_COND(cVS, xFlags, xFlags, 0b10, 0x0b);
//        BIC_IMM8_COND(cVC, xFlags, xFlags, 0b10, 0x0b);
//    }
//    IFX(X_SF) {
//        MOV_REG_LSR_IMM5(s3, s1, 31);
//        BFI(xFlags, s3, F_SF, 1);
//    }
//    IFX(X_PF) {
//        emit_pf(dyn, ninst, s1, s3, s4);
//    }
//}

// emit SBB8 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_sbb8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    MAYUSE(s2);
    IFX(X_PEND) {
        STRB_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRB_U12(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, d_sbb8);
    } else IFX(X_ALL) {
        SET_DFNONE(s3);
    }
    EORw_mask(s4, xFlags, 0, 0);        // invert CC because it's reverted for SUB on ARM
    MRS_nzvc(s3);
    BFIx(s3, s4, 29, 1); // set C
    MSR_nzvc(s3);      // load CC into ARM CF
    IFX(X_AF|X_OF|X_CF) {
        MVNw_REG(s4, s1);
    }
    SBCw_REG(s1, s1, s2);
    IFX(X_PEND) {
        STRB_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_AF|X_OF|X_CF) {
        ORRw_REG(s3, s4, s2);    // s3 = ~op1 | op2
        ANDw_REG(s4, s2, s4);    // s4 = ~op1 & op2
        ANDw_REG(s3, s3, s1);   // s3 = (~op1 | op2) & res
        ORRw_REG(s3, s3, s4);   // s3 = (~op1 & op2) | ((~op1 | op2) & res)
        IFX(X_CF) {
            LSRw(s4, s3, 7);
            BFIw(xFlags, s4, F_CF, 1);    // CF : bc & 0x80
        }
        IFX(X_AF) {
            LSRw(s4, s3, 3);
            BFIw(xFlags, s4, F_AF, 1);    // AF: bc & 0x08
        }
        IFX(X_OF) {
            LSRw(s4, s3, 6);
            EORw_REG_LSR(s4, s4, s4, 1);
            BFIw(xFlags, s4, F_OF, 1);    // OF: ((bc >> 6) ^ ((bc>>6)>>1)) & 1
        }
    }
    COMP_ZFSF(s1, 8)
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit SBB8 instruction, from s1, constant c, store result in s1 using s3 and s4 as scratch
void emit_sbb8c(dynarec_arm_t* dyn, int ninst, int s1, int c, int s3, int s4, int s5)
{
    MAYUSE(s5);
    MOV32w(s5, c&0xff);
    emit_sbb8(dyn, ninst, s1, s5, s3, s4);
}

// emit SBB16 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_sbb16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    MAYUSE(s2);
    IFX(X_PEND) {
        STRH_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRH_U12(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, d_sbb16);
    } else IFX(X_ALL) {
        SET_DFNONE(s3);
    }
    EORw_mask(s4, xFlags, 0, 0);            // invert CC because it's reverted for SUB on ARM
    MRS_nzvc(s3);
    BFIx(s3, s4, 29, 1); // set C, bit 29
    MSR_nzvc(s3);      // load CC into ARM CF
    IFX(X_AF|X_OF|X_CF) {
        MVNw_REG(s4, s1);
    }
    SBCw_REG(s1, s1, s2);
    IFX(X_PEND) {
        STRH_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_AF|X_OF|X_CF) {
        ORRw_REG(s3, s4, s2);    // s3 = ~op1 | op2
        ANDw_REG(s4, s2, s4);    // s4 = ~op1 & op2
        ANDw_REG(s3, s3, s1);   // s3 = (~op1 | op2) & res
        ORRw_REG(s3, s3, s4);   // s3 = (~op1 & op2) | ((~op1 | op2) & res)
        IFX(X_CF) {
            LSRw(s4, s3, 15);
            BFIw(xFlags, s4, F_CF, 1);    // CF : bc & 0x8000
        }
        IFX(X_AF) {
            LSRw(s4, s3, 3);
            BFIw(xFlags, s4, F_AF, 1);    // AF: bc & 0x08
        }
        IFX(X_OF) {
            LSRw(s4, s3, 14);
            EORw_REG_LSR(s4, s4, s4, 1);
            BFIw(xFlags, s4, F_OF, 1);    // OF: ((bc >> 14) ^ ((bc>>14)>>1)) & 1
        }
    }
    COMP_ZFSF(s1, 16)
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit SBB16 instruction, from s1, constant c, store result in s1 using s3 and s4 as scratch
//void emit_sbb16c(dynarec_arm_t* dyn, int ninst, int s1, int c, int s3, int s4)
//{
//    IFX(X_PEND) {
//        MOVW(s3, c);
//        STR_IMM9(s1, xEmu, offsetof(x64emu_t, op1));
//        STR_IMM9(s3, xEmu, offsetof(x64emu_t, op2));
//        SET_DF(s3, d_sbb16);
//    } else IFX(X_ALL) {
//        SET_DFNONE(s3);
//    }
//    IFX(X_AF|X_OF|X_CF) {
//        MVN_REG_LSL_IMM5(s4, s1, 0);
//    }
//    XOR_IMM8(s3, xFlags, 1);            // invert CC because it's reverted for SUB on ARM
//    MOVS_REG_LSR_IMM5(s3, s3, 1);       // load into ARM CF
//    if(c>=0 && c<255) {
//        SBC_IMM8(s1, s1, c);
//    } else {
//        MOVW(s3, c);
//        SBC_REG_LSL_IMM5(s1, s1, s3, 0);
//    }
//    IFX(X_PEND) {
//        STR_IMM9(s1, xEmu, offsetof(x64emu_t, res));
//    }
//    IFX(X_AF|X_OF|X_CF) {
//        if(c>=0 && c<256) {
//            ORR_IMM8(s3, s4, c, 0);     // s3 = ~op1 | op2
//            AND_IMM8(s4, s4, c);        // s4 = ~op1 & op2
//        } else {
//            ORR_REG_LSL_IMM5(s3, s3, s4, 0);    // s3 = ~op1 | op2
//            PUSH(xSP, 1<<s3);
//            MOVW(s3, c);
//            AND_REG_LSL_IMM5(s4, s4, s3, 0);    // s4 = ~op1 & op2
//            POP(xSP, 1<<s3);
//        }
//        AND_REG_LSL_IMM5(s3, s3, s1, 0);   // s3 = (~op1 | op2) & res
//        ORR_REG_LSL_IMM5(s3, s3, s4, 0);   // s3 = (~op1 & op2) | ((~op1 | op2) & res)
//        IFX(X_CF) {
//            MOV_REG_LSR_IMM5(s4, s3, 15);
//            BFI(xFlags, s4, F_CF, 1);    // CF : bc & 0x8000
//        }
//        IFX(X_AF) {
//            MOV_REG_LSR_IMM5(s4, s3, 3);
//            BFI(xFlags, s4, F_AF, 1);    // AF: bc & 0x08
//        }
//        IFX(X_OF) {
//            MOV_REG_LSR_IMM5(s4, s3, 14);
//            XOR_REG_LSR_IMM8(s4, s4, s4, 1);
//            BFI(xFlags, s4, F_OF, 1);    // OF: ((bc >> 14) ^ ((bc>>14)>>1)) & 1
//        }
//    }
//    IFX(X_ZF) {
//        UXTH(s1, s1, 0);
//        TSTS_REG_LSL_IMM5(s1, s1, 0);
//        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
//        BIC_IMM8_COND(cNE, xFlags, xFlags, 1<<F_ZF, 0);
//    }
//    IFX(X_SF) {
//        MOV_REG_LSR_IMM5(s3, s1, 15);
//        BFI(xFlags, s3, F_SF, 1);
//    }
//    IFX(X_PF) {
//        emit_pf(dyn, ninst, s1, s3, s4);
//    }
//}

// emit NEG32 instruction, from s1, store result in s1 using s3 and s4 as scratch
void emit_neg32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s3, int s4)
{
    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s3, rex.w?d_neg64:d_neg32);
    } else IFX(X_ALL) {
        SET_DFNONE(s3);
    }
    IFX(X_CF) {
        TSTxw_REG(s1, s1);
        CSETw(s4, cNE);
        BFIw(xFlags, s4, F_CF, 1);
    }
    IFX(X_AF) {
        MOVxw_REG(s3, s1);
    }
    IFX(X_ZF|X_OF) {
        NEGSxw_REG(s1, s1);
    } else {
        NEGxw_REG(s1, s1);
    }
    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_ZF) {
        CSETw(s4, cEQ);
        BFIw(xFlags, s4, F_ZF, 1);
    }
    IFX(X_OF) {
        CSETw(s4, cVS);
        BFIw(xFlags, s4, F_OF, 1);
    }
    IFX(X_AF) {
        ORRxw_REG(s3, s3, s1);                        // bc = op1 | res
        LSRxw(s4, s3, 3);
        BFIw(xFlags, s4, F_AF, 1);    // AF: bc & 0x08
    }
    IFX(X_SF) {
        LSRxw(s3, s1, rex.w?63:31);
        BFIw(xFlags, s3, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit NEG16 instruction, from s1, store result in s1 using s3 and s4 as scratch
void emit_neg16(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4)
{
    IFX(X_PEND) {
        STRH_U12(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s3, d_neg16);
    } else IFX(X_ALL) {
        SET_DFNONE(s3);
    }
    IFX(X_CF) {
        TSTw_REG(s1, s1);
        CSETw(s4, cNE);
        BFIw(xFlags, s4, F_CF, 1);
    }
    IFX(X_AF|X_OF) {
        MOVw_REG(s3, s1);
    }
    IFX(X_ZF) {
        NEGSw_REG(s1, s1);
    } else {
        NEGw_REG(s1, s1);
    }
    IFX(X_PEND) {
        STRH_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_AF|X_OF) {
        ORRw_REG(s3, s3, s1);                        // bc = op1 | res
        IFX(X_AF) {
            LSRw(s4, s3, 3);
            BFIw(xFlags, s4, F_AF, 1);    // AF: bc & 0x08
        }
        IFX(X_OF) {
            LSRw(s4, s3, 14);
            EORw_REG_LSR(s4, s4, s4, 1);
            BFIw(xFlags, s4, F_OF, 1);    // OF: ((bc >> 14) ^ ((bc>>14)>>1)) & 1
        }
    }
    IFX(X_ZF) {
        CSETw(s4, cEQ);
        BFIw(xFlags, s4, F_ZF, 1);
    }
    IFX(X_SF) {
        LSRw(s3, s1, 15);
        BFIw(xFlags, s3, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit NEG8 instruction, from s1, store result in s1 using s3 and s4 as scratch
void emit_neg8(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4)
{
    IFX(X_PEND) {
        STRB_U12(s1, xEmu, offsetof(x64emu_t, op1));
        SET_DF(s3, d_neg8);
    } else IFX(X_ALL) {
        SET_DFNONE(s3);
    }
    IFX(X_CF) {
        TSTw_REG(s1, s1);
        CSETw(s4, cNE);
        BFIw(xFlags, s4, F_CF, 1);
    }
    IFX(X_AF|X_OF) {
        MOVw_REG(s3, s1);
    }
    NEGw_REG(s1, s1);
    IFX(X_ZF) {
        ANDSw_mask(s1, s1, 0, 7);   // mask 0xff
    }
    IFX(X_PEND) {
        STRB_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_AF|X_OF) {
        ORRw_REG(s3, s3, s1);                        // bc = op1 | res
        IFX(X_AF) {
            LSRw(s4, s3, 3);
            BFIw(xFlags, s4, F_AF, 1);    // AF: bc & 0x08
        }
        IFX(X_OF) {
            LSRw(s4, s3, 6);
            EORx_REG_LSR(s4, s4, s4, 1);
            BFIw(xFlags, s4, F_OF, 1);    // OF: ((bc >> 6) ^ ((bc>>6)>>1)) & 1
        }
    }
    IFX(X_ZF) {
        CSETw(s4, cEQ);
        BFIw(xFlags, s4, F_ZF, 1);
    }
    IFX(X_SF) {
        LSRw(s3, s1, 7);
        BFIw(xFlags, s3, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}
