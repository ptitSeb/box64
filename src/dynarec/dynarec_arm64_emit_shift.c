#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
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
#include "dynarec_arm64.h"
#include "dynarec_arm64_private.h"
#include "arm64_printer.h"
#include "../tools/bridge_private.h"

#include "dynarec_arm64_functions.h"
#include "dynarec_arm64_helper.h"

// emit SHL32 instruction, from s1 , shift s2, store result in s1 using s3 and s4 as scratch. s3 can be same as s2
void emit_shl32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4)
{
    MAYUSE(s2);
    int64_t j64;
    MAYUSE(j64);

    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRxw_U12(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, rex.w?d_shl64:d_shl32);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    IFX(F_OF) {
        CMPSxw_U12(s2, 0);
        IFX(F_OF) {
            Bcond(cNE, +8);
            BFCx(xFlags, F_OF, 1);
        }
        IFX(X_PEND) {
            Bcond(cNE, +8);
            STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
        }
        B_NEXT(cEQ);
    }
    IFX(X_CF | X_OF) {
        MOV32w(s4, rex.w?64:32);
        SUBxw_REG(s4, s4, s2);
        LSRxw_REG(s4, s1, s4);
        BFIw(xFlags, s4, F_CF, 1);
    }
    LSLxw_REG(s1, s1, s2);
    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_ZF) {
        TSTxw_REG(s1, s1);
        BFCx(xFlags, F_ZF, 1);
        Bcond(cNE, +8);
        ORRw_mask(xFlags, xFlags, 0b011010, 0); // mask=0x40
    }
    IFX(X_SF) {
        LSRxw(s4, s1, (rex.w)?63:31);
        BFIx(xFlags, s4, F_SF, 1);
    }
    IFX(X_OF) {
        CMPSxw_U12(s2, 1);   // if s2==1
            IFX(X_SF) {} else {LSRxw(s4, s1, (rex.w)?63:31);}
            Bcond(cNE, 4+3*4);
            BFCw(xFlags, F_OF, 1);
            EORxw_REG(s4, s4, xFlags);  // CF is set if OF is asked
            BFIw(xFlags, s4, F_OF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit SHL32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_shl32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int32_t c, int s3, int s4)
{
    IFX(X_PEND) {
        MOV32w(s3, c);
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRxw_U12(s3, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, rex.w?d_shl64:d_shl32);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    if(c==0) {
        IFX(F_OF) {
            BFCx(xFlags, F_OF, 1);
        }
        IFX(X_PEND) {
            STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }
    IFX(X_CF) {
        LSRxw(s3, s1, (rex.w?64:32)-c);
        BFIxw(xFlags, s3, F_CF, 1);
    }
    LSLxw(s1, s1, c);

    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_ZF) {
        TSTxw_REG(s1, s1);
        BFCx(xFlags, F_ZF, 1);
        Bcond(cNE, +8);
        ORRw_mask(xFlags, xFlags, 0b011010, 0); // mask=0x40
    }
    IFX(X_SF) {
        LSRxw(s4, s1, (rex.w)?63:31);
        BFIx(xFlags, s4, F_SF, 1);
    }
    IFX(X_OF) {
        if(c==1) {
            IFX(X_SF) {} else {LSRxw(s4, s1, (rex.w)?63:31);}
            Bcond(cNE, +12);
            EORxw_REG(s4, s4, xFlags);  // CF is set if OF is asked
            BFIw(xFlags, s4, F_OF, 1);
        } else {
            BFCw(xFlags, F_OF, 1);
        }
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit SHR32 instruction, from s1 , s2, store result in s1 using s3 and s4 as scratch, s2 can be same as s3
void emit_shr32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4)
{
    MAYUSE(s2);
    int64_t j64;
    MAYUSE(j64);

    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRxw_U12(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, rex.w?d_shr64:d_shr32);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    IFX(X_ALL) {
        CMPSxw_U12(s2, 0); //if(!c)
            IFX(X_PEND) {
                Bcond(cNE, +12);
                STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
            }
            B_NEXT(cEQ);
    }
    IFX(X_CF) {
        SUBxw_U12(s3, s2, 1);
        LSRxw_REG(s3, s1, s3);
        BFIw(xFlags, s3, 0, 1);
    }
    LSRxw_REG(s1, s1, s2);
    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_ZF) {
        TSTxw_REG(s1, s1);
        BFCx(xFlags, F_ZF, 1);
        Bcond(cNE, +8);
        ORRw_mask(xFlags, xFlags, 0b011010, 0); // mask=0x40
    }
    IFX(X_SF) {
        LSRxw(s4, s1, (rex.w)?63:31);
        BFIx(xFlags, s4, F_SF, 1);
    }
    IFX(X_OF) {
        CMPSxw_U12(s2, 1);   // if s2==1
            Bcond(cNE, 4+3*4);
            if(rex.w) {
                LSRx(s4, s1, 62);
            } else {
                LSRw(s4, s1, 30);
            }
            EORw_REG_LSR(s4, s4, s4, 1);
            BFIw(xFlags, s4, F_OF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit SHR32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_shr32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int32_t c, int s3, int s4)
{
    IFX(X_PEND) {
        MOV32w(s3, c);
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRxw_U12(s3, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, rex.w?d_shr64:d_shr32);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    if(!c) {
        IFX(X_PEND) {
            STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }
    IFX(X_CF) {
        LSRxw_REG(s3, s1, c-1);
        BFIw(xFlags, s3, 0, 1);
    }
    LSRxw(s1, s1, c);
    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_ZF) {
        TSTxw_REG(s1, s1);
        BFCx(xFlags, F_ZF, 1);
        Bcond(cNE, +8);
        ORRw_mask(xFlags, xFlags, 0b011010, 0); // mask=0x40
    }
    IFX(X_SF) {
        LSRxw(s4, s1, (rex.w)?63:31);
        BFIx(xFlags, s4, F_SF, 1);
    }
    IFX(X_OF) {
        if(c==1) {
            LSRxw(s4, s1, rex.w?62:30);
            EORw_REG_LSR(s4, s4, s4, 1);
            BFIw(xFlags, s4, F_OF, 1);
        }
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit SAR32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_sar32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int32_t c, int s3, int s4)
{
    IFX(X_PEND) {
        MOV32w(s3, c);
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRxw_U12(s3, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, rex.w?d_sar64:d_sar32);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    if(!c) {
        IFX(X_PEND) {
            STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }
    IFX(X_CF) {
        ASRxw(s3, s1, c-1);
        BFIw(xFlags, s3, 0, 1);
    }
    ASRxw(s1, s1, c);
    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_ZF) {
        TSTw_REG(s1, s1);
        BFCx(xFlags, F_ZF, 1);
        Bcond(cNE, +8);
        ORRw_mask(xFlags, xFlags, 0b011010, 0); // mask=0x40
    }
    IFX(X_SF) {
        LSRxw(s4, s1, (rex.w)?63:31);
        BFIx(xFlags, s4, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit ROL32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_rol32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int32_t c, int s3, int s4)
{
    MAYUSE(rex); MAYUSE(s1); MAYUSE(s3); MAYUSE(s4);
    IFX(X_PEND) {
        MOV32w(s3, c);
        STRxw_U12(s3, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, d_rol32);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    if(!c) {
        IFX(X_PEND) {
            STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }
    RORxw(s1, s1, (rex.w?64:32)-c);
    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_CF) {
        BFIw(xFlags, s1, F_CF, 1);
    }
    IFX(X_OF) {
        if(c==1) {
            ADDxw_REG_LSR(s3, s1, s1, rex.w?63:31);
            BFIw(xFlags, s3, F_OF, 1);
        }
    }
}

// emit ROR32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_ror32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int32_t c, int s3, int s4)
{
    MAYUSE(s1); MAYUSE(s3); MAYUSE(s4);
    IFX(X_PEND) {
        MOV32w(s3, c);
        STRxw_U12(s3, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, rex.w?d_ror64:d_ror32);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    if(!c) {
        IFX(X_PEND) {
            STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }
    RORxw(s1, s1, c);
    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_CF) {
        LSRxw(s3, s1, rex.w?63:31);
        BFIw(xFlags, s3, F_CF, 1);
    }
    IFX(X_OF) {
        if(c==1) {
            LSRxw(s3, s1, rex.w?62:30);
            EORxw_REG_LSR(s3, s3, s3, 1);
            BFIw(xFlags, s4, F_OF, 1);
        }
    }
}

// emit SHRD32 instruction, from s1, fill s2 , constant c, store result in s1 using s3 and s4 as scratch
//void emit_shrd32c(dynarec_arm_t* dyn, int ninst, int s1, int s2, int32_t c, int s3, int s4)
//{
//    c&=0x1f;
//    IFX(X_PEND) {
//        MOVW(s3, c);
//        STR_IMM9(s1, xEmu, offsetof(x64emu_t, op1));
//        STR_IMM9(s3, xEmu, offsetof(x64emu_t, op2));
//        // same flags calc as shr32
//        SET_DF(s4, d_shr32);
//    } else IFX(X_ALL) {
//        SET_DFNONE(s4);
//    }
//    if(!c) {
//        IFX(X_PEND) {
//            STR_IMM9(s1, xEmu, offsetof(x64emu_t, res));
//        }
//        return;
//    }
//    IFX(X_CF) {
//        MOVS_REG_LSR_IMM5(s1, s1, c);
//    } else {
//        MOV_REG_LSR_IMM5(s1, s1, c);
//    }
//    IFX(X_ZF|X_CF) {
//        BIC_IMM8(xFlags, xFlags, (1<<F_ZF)|(1<<F_CF), 0);
//    }
//    IFX(X_CF) {
//        ORR_IMM8_COND(cCS, xFlags, xFlags, 1<<F_CF, 0);
//    }
//    IFX(X_ZF) {
//        ORRS_REG_LSL_IMM5(s1, s1, s2, 32-c);
//    } else {
//        ORR_REG_LSL_IMM5(s1, s1, s2, 32-c);
//    }
//    IFX(X_PEND) {
//        STR_IMM9(s1, xEmu, offsetof(x64emu_t, res));
//    }
//    IFX(X_ZF) {
//        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
//    }
//    IFX(X_SF) {
//        MOV_REG_LSR_IMM5(s3, s1, 31);
//        BFI(xFlags, s3, F_SF, 1);
//    }
//    IFX(X_OF) {
//        if(c==1) {
//            MOV_REG_LSR_IMM5(s4, s1, 30);
//            XOR_REG_LSR_IMM8(s4, s4, s4, 1);
//            BFI(xFlags, s4, F_OF, 1);
//        }
//    }
//    IFX(X_PF) {
//        emit_pf(dyn, ninst, s1, s3, s4);
//    }
//}

//void emit_shld32c(dynarec_arm_t* dyn, int ninst, int s1, int s2, int32_t c, int s3, int s4)
//{
//    c&=0x1f;
//    IFX(X_PEND) {
//        MOVW(s3, c);
//        STR_IMM9(s1, xEmu, offsetof(x64emu_t, op1));
//        STR_IMM9(s3, xEmu, offsetof(x64emu_t, op2));
//        // same flags computation as with shl32
//        SET_DF(s4, d_shl32);
//    } else IFX(X_ALL) {
//        SET_DFNONE(s4);
//    }
//    if(c==0) {
//        IFX(F_OF) {
//            BFC(xFlags, F_OF, 1);
//        }
//        IFX(X_PEND) {
//            STR_IMM9(s1, xEmu, offsetof(x64emu_t, res));
//        }
//        return;
//    }
//    IFX(X_CF) {
//        MOV_REG_LSR_IMM5(s3, s1, 32-c);
//        BFI(xFlags, s3, F_CF, 1);
//    }
//    IFX(X_OF) {
//        MOVS_REG_LSL_IMM5(s1, s1, c);
//    } else {
//        MOV_REG_LSL_IMM5(s1, s1, c);
//    }
//    IFX(X_OF) {
//        if(c==1) {
//            UBFX(s3, s2, 0, 1);
//            XOR_IMM8_COND(cCS, s3, s3, 1);
//            BFI(xFlags, s3, F_OF, 1);
//        } else {
//            BFC(xFlags, F_OF, 1);
//        }
//    }
//    IFX(X_ZF) {
//        ORRS_REG_LSR_IMM5(s1, s1, s2, 32-c);
//    } else {
//        ORR_REG_LSR_IMM5(s1, s1, s2, 32-c);
//    }
//
//    IFX(X_PEND) {
//        STR_IMM9(s1, xEmu, offsetof(x64emu_t, res));
//    }
//    IFX(X_ZF) {
//        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
//        BIC_IMM8_COND(cNE, xFlags, xFlags, 1<<F_ZF, 0);
//    }
//    IFX(X_SF) {
//        MOV_REG_LSR_IMM5(s3, s1, 31);
//        BFI(xFlags, s3, F_SF, 1);
//    }
//    IFX(X_PF) {
//        emit_pf(dyn, ninst, s1, s3, s4);
//    }
//}
