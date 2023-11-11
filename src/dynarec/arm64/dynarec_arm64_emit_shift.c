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
    IFX(X_OF) {
        CMPSxw_U12(s2, 0);
        Bcond(cNE, +8+((dyn->insts[ninst].x64.gen_flags&X_PEND)?4:0));
        BFCw(xFlags, F_OF, 1);
        IFX(X_PEND) {
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
        CSETw(s4, cEQ);
        BFIw(xFlags, s4, F_ZF, 1);
    }
    IFX(X_SF) {
        LSRxw(s4, s1, (rex.w)?63:31);
        BFIx(xFlags, s4, F_SF, 1);
    }
    IFX(X_OF) {
        CMPSxw_U12(s2, 1);   // if s2==1
            IFX(X_SF) {} else {LSRxw(s4, s1, (rex.w)?63:31);}
            EORxw_REG(s4, s4, xFlags);  // CF is set if OF is asked
            CSELw(s4, s4, wZR, cEQ);
            BFIw(xFlags, s4, F_OF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit SHL32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_shl32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4)
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
        IFX(X_OF) {
            BFCw(xFlags, F_OF, 1);
        }
        IFX(X_PEND) {
            STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }
    IFX(X_CF|X_OF) {
        LSRxw(s3, s1, (rex.w?64:32)-c);
        BFIxw(xFlags, s3, F_CF, 1);
    }
    LSLxw(s1, s1, c);

    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_ZF) {
        TSTxw_REG(s1, s1);
        CSETw(s4, cEQ);
        BFIw(xFlags, s4, F_ZF, 1);
    }
    IFX(X_SF) {
        LSRxw(s4, s1, (rex.w)?63:31);
        BFIx(xFlags, s4, F_SF, 1);
    }
    IFX(X_OF) {
        if(c==1) {
            IFX(X_SF) {} else {LSRxw(s4, s1, (rex.w)?63:31);}
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
    IFX(X_CF) {
        SUBxw_U12(s3, s2, 1);
        LSRxw_REG(s3, s1, s3);
        BFIw(xFlags, s3, 0, 1);
    }
    IFX(X_OF) {
        CMPSxw_U12(s2, 1);   // if s2==1
            Bcond(cNE, 4+2*4);
            LSRxw(s4, s1, rex.w?63:31);
            BFIw(xFlags, s4, F_OF, 1);
    }
    LSRxw_REG(s1, s1, s2);
    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_ZF) {
        TSTxw_REG(s1, s1);
        CSETw(s4, cEQ);
        BFIw(xFlags, s4, F_ZF, 1);
    }
    IFX(X_SF) {
        LSRxw(s4, s1, (rex.w)?63:31);
        BFIx(xFlags, s4, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit SHR32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_shr32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4)
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
        if(c==1) {
            BFIw(xFlags, s1, 0, 1);
        } else {
            LSRxw(s3, s1, c-1);
            BFIw(xFlags, s3, 0, 1);
        }
    }
    IFX(X_OF) {
        if(c==1) {
            LSRxw(s4, s1, rex.w?63:31);
            BFIw(xFlags, s4, F_OF, 1);
        }
    }
    LSRxw(s1, s1, c);
    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_ZF) {
        TSTxw_REG(s1, s1);
        CSETw(s4, cEQ);
        BFIw(xFlags, s4, F_ZF, 1);
    }
    IFX(X_SF) {
        LSRxw(s4, s1, (rex.w)?63:31);
        BFIx(xFlags, s4, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit SAR32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_sar32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4)
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
        TSTxw_REG(s1, s1);
        CSETw(s4, cEQ);
        BFIw(xFlags, s4, F_ZF, 1);
    }
    IFX(X_SF) {
        LSRxw(s4, s1, (rex.w)?63:31);
        BFIx(xFlags, s4, F_SF, 1);
    }
    IFX(X_OF)
        if(c==1) {
            BFCw(xFlags, F_OF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit SHL8 instruction, from s1 , shift s2, store result in s1 using s3 and s4 as scratch. s3 can be same as s2
void emit_shl8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    MAYUSE(s2);
    int64_t j64;
    MAYUSE(j64);

    IFX(X_PEND) {
        STRB_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRB_U12(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, d_shl8);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    IFX(X_CF | X_OF) {
        MOV32w(s4, 8);
        SUBw_REG(s4, s4, s2);
        LSRw_REG(s4, s1, s4);
        BFIw(xFlags, s4, F_CF, 1);
    }
    LSLw_REG(s1, s1, s2);
    IFX(X_PEND) {
        STRB_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_ZF) {
        TSTw_mask(s1, 0, 7);
        CSETw(s4, cEQ);
        BFIw(xFlags, s4, F_ZF, 1);
    }
    IFX(X_SF) {
        LSRw(s4, s1, 7);
        BFIw(xFlags, s4, F_SF, 1);
    }
    IFX(X_OF) {
        CMPSw_U12(s2, 1);   // if s2==1
            IFX(X_SF) {} else {LSRw(s4, s1, 7);}
            EORw_REG(s4, s4, xFlags);  // CF is set if OF is asked
            CSELw(s4, s4, wZR, cEQ);
            BFIw(xFlags, s4, F_OF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit SHL8 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_shl8c(dynarec_arm_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4)
{
    IFX(X_PEND) {
        MOV32w(s3, c);
        STRB_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRB_U12(s3, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, d_shl8);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    if(c==0) {
        IFX(X_OF) {
            BFCw(xFlags, F_OF, 1);
        }
        IFX(X_PEND) {
            STRB_U12(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }
    IFX(X_CF|X_OF) {
        LSRw(s3, s1, 8-c);
        BFIw(xFlags, s3, F_CF, 1);
    }
    LSLw(s1, s1, c);

    IFX(X_PEND) {
        STRB_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_ZF) {
        TSTw_mask(s1, 0, 7);
        CSETw(s4, cEQ);
        BFIw(xFlags, s4, F_ZF, 1);
    }
    IFX(X_SF) {
        LSRw(s4, s1, 7);
        BFIw(xFlags, s4, F_SF, 1);
    }
    IFX(X_OF) {
        if(c==1) {
            IFX(X_SF) {} else {LSRw(s4, s1, 7);}
            EORw_REG(s4, s4, xFlags);  // CF is set if OF is asked
            BFIw(xFlags, s4, F_OF, 1);
        } else {
            BFCw(xFlags, F_OF, 1);
        }
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit SHR8 instruction, from s1 , s2, store result in s1 using s3 and s4 as scratch, s2 can be same as s3
void emit_shr8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    MAYUSE(s2);
    int64_t j64;
    MAYUSE(j64);

    IFX(X_PEND) {
        STRB_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRB_U12(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, d_shr8);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    IFX(X_CF) {
        SUBw_U12(s3, s2, 1);
        LSRw_REG(s3, s1, s3);
        BFIw(xFlags, s3, 0, 1);
    }
    IFX(X_OF) {
        CMPSw_U12(s2, 1);   // if s2==1
            Bcond(cNE, 4+2*4);
            LSRw(s4, s1, 7);
            BFIw(xFlags, s4, F_OF, 1);
    }
    LSRw_REG(s1, s1, s2);
    IFX(X_PEND) {
        STRB_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_ZF) {
        TSTw_mask(s1, 0, 7);
        CSETw(s4, cEQ);
        BFIw(xFlags, s4, F_ZF, 1);
    }
    IFX(X_SF) {
        LSRw(s4, s1, 7);
        BFIw(xFlags, s4, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit ROL32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_rol32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4)
{
    MAYUSE(rex); MAYUSE(s1); MAYUSE(s3); MAYUSE(s4);
    IFX(X_PEND) {
        MOV32w(s3, c);
        STRxw_U12(s3, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, rex.w?d_rol64:d_rol32);
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
void emit_ror32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4)
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
        BFXILxw(xFlags, s1, rex.w?63:31, 1);
    }
    IFX(X_OF) {
        if(c==1) {
            LSRxw(s3, s1, rex.w?62:30);
            EORxw_REG_LSR(s3, s3, s3, 1);
            BFIw(xFlags, s3, F_OF, 1);
        }
    }
}

// emit ROL8 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_rol8c(dynarec_arm_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4)
{
    MAYUSE(s1); MAYUSE(s3); MAYUSE(s4);
    IFX(X_PEND) {
        MOV32w(s3, c);
        STRB_U12(s3, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, d_rol8);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    if(!c) {
        IFX(X_PEND) {
            STRB_U12(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }
    int rc = 8-(c&7);
    ORRw_REG_LSL(s1, s1, s1, 8);
    LSRw(s1, s1, rc);
    IFX(X_PEND) {
        STRB_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_CF) {
        BFIw(xFlags, s1, F_CF, 1);
    }
    IFX(X_OF) {
        if(c==1) {
            EORw_REG_LSR(s3, s1, s1, 7);
            BFIw(xFlags, s3, F_OF, 1);
        }
    }
}

// emit ROR8 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_ror8c(dynarec_arm_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4)
{
    MAYUSE(s1); MAYUSE(s3); MAYUSE(s4);
    IFX(X_PEND) {
        MOV32w(s3, c);
        STRB_U12(s3, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, d_ror8);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    if(!c) {
        IFX(X_PEND) {
            STRB_U12(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }
    ORRw_REG_LSL(s1, s1, s1, 8);
    LSRw(s1, s1, c&7);
    IFX(X_PEND) {
        STRB_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_CF) {
        BFXILw(xFlags, s1, 7, 1);
    }
    IFX(X_OF) {
        if(c==1) {
            LSRw(s3, s1, 6);
            EORw_REG_LSR(s3, s3, s3, 1);
            BFIw(xFlags, s3, F_OF, 1);
        }
    }
}

// emit ROL16 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_rol16c(dynarec_arm_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4)
{
    MAYUSE(s1); MAYUSE(s3); MAYUSE(s4);
    IFX(X_PEND) {
        MOV32w(s3, c);
        STRH_U12(s3, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, d_rol16);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    if(!c) {
        IFX(X_PEND) {
            STRH_U12(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }
    int rc = 16-(c&15);
    ORRw_REG_LSL(s1, s1, s1, 16);
    LSRw(s1, s1, rc);
    IFX(X_PEND) {
        STRH_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_CF) {
        BFIw(xFlags, s1, F_CF, 1);
    }
    IFX(X_OF) {
        if(c==1) {
            EORw_REG_LSR(s3, s1, s1, 15);
            BFIw(xFlags, s3, F_OF, 1);
        }
    }
}

// emit ROR16 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_ror16c(dynarec_arm_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4)
{
    MAYUSE(s1); MAYUSE(s3); MAYUSE(s4);
    IFX(X_PEND) {
        MOV32w(s3, c);
        STRH_U12(s3, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, d_ror16);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    if(!c) {
        IFX(X_PEND) {
            STRH_U12(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }
    ORRw_REG_LSL(s1, s1, s1, 16);
    LSRw(s1, s1, c&15);
    IFX(X_PEND) {
        STRH_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_CF) {
        BFXILw(xFlags, s1, 15, 1);
    }
    IFX(X_OF) {
        if(c==1) {
            LSRw(s3, s1, 14);
            EORw_REG_LSR(s3, s3, s3, 1);
            BFIw(xFlags, s3, F_OF, 1);
        }
    }
}

// emit SHRD32 instruction, from s1, fill s2 , constant c, store result in s1 using s3 and s4 as scratch
void emit_shrd32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, uint32_t c, int s3, int s4)
{
    c&=(rex.w?0x3f:0x1f);
    IFX(X_PEND) {
        MOV32w(s3, c);
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRxw_U12(s3, xEmu, offsetof(x64emu_t, op2));
        // same flags computation as with shl64/shl32
        SET_DF(s4, rex.w?d_shl64:d_shl32);
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
        LSRxw(s3, s1, c-1);
        BFIw(xFlags, s3, 0, 1);
    }
    LSRxw(s3, s1, c);
    ORRxw_REG_LSL(s1, s3, s2, (rex.w?64:32)-c);
    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_ZF) {
        TSTxw_REG(s1, s1);
        CSETw(s4, cEQ);
        BFIw(xFlags, s4, F_ZF, 1);
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

void emit_shld32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, uint32_t c, int s3, int s4)
{
    c&=(rex.w?0x3f:0x1f);
    IFX(X_PEND) {
        MOV32w(s3, c);
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRxw_U12(s3, xEmu, offsetof(x64emu_t, op2));
        // same flags computation as with shl64/shl32
        SET_DF(s4, rex.w?d_shl64:d_shl32);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    if(c==0) {
        IFX(X_OF) {
            BFCw(xFlags, F_OF, 1);
        }
        IFX(X_PEND) {
            STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }
    IFX(X_CF|X_OF) {
        LSRxw(s3, s1, (rex.w?64:32)-c);
        BFIxw(xFlags, s3, F_CF, 1);
    }
    LSLxw(s3, s1, c);
    ORRxw_REG_LSR(s1, s3, s2, (rex.w?64:32)-c);

    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_ZF) {
        TSTxw_REG(s1, s1);
        CSETw(s4, cEQ);
        BFIw(xFlags, s4, F_ZF, 1);
    }
    IFX(X_SF) {
        LSRxw(s4, s1, (rex.w)?63:31);
        BFIx(xFlags, s4, F_SF, 1);
    }
    IFX(X_OF) {
        if(c==1) {
            UBFXxw(s3, s1, rex.w?63:31, 1);
            EORxw_REG(s3, s3, xFlags);  // CF is set if OF is asked
            BFIw(xFlags, s3, F_OF, 1);
        } else {
            BFCw(xFlags, F_OF, 1);
        }
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}
