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
    if(box64_dynarec_test)
        IFX(X_AF) {
            BFCw(xFlags, F_AF, 1);
        }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit SHL32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_shl32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4)
{
    if (!c) return;

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
    if(box64_dynarec_test)
        IFX(X_AF) {
            BFCw(xFlags, F_AF, 1);
        }
    IFX(X_PF) {
        if(c>7) {
            // the 0xff area will be 0, so PF is known
            MOV32w(s3, 1);
            BFIw(xFlags, s3, F_PF, 1);
        } else
            emit_pf(dyn, ninst, s1, s4);
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
    if(box64_dynarec_test)
        IFX(X_AF) {
            BFCw(xFlags, F_AF, 1);
        }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit SHR32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_shr32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4)
{
    if (!c) return;

    IFX(X_PEND) {
        MOV32w(s3, c);
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRxw_U12(s3, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, rex.w?d_shr64:d_shr32);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
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
        // no sign if c>0
        BFCw(xFlags, F_SF, 1);
    }
    if(box64_dynarec_test)
        IFX(X_AF) {
            BFCw(xFlags, F_AF, 1);
        }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit SAR32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_sar32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4)
{
    if (!c) return;

    IFX(X_PEND) {
        MOV32w(s3, c);
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRxw_U12(s3, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, rex.w?d_sar64:d_sar32);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
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
    if(box64_dynarec_test)
        IFX(X_AF) {
            BFCw(xFlags, F_AF, 1);
        }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
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
    COMP_ZFSF(s1, 8)
    IFX(X_OF) {
        CMPSw_U12(s2, 1);   // if s2==1
            IFX2(X_SF, && !arm64_flagm) {} else {LSRw(s3, s1, 7);}
            EORw_REG(s4, s3, xFlags);  // CF is set if OF is asked
            CSELw(s4, s4, wZR, cEQ);
            BFIw(xFlags, s4, F_OF, 1);
    }
    if(box64_dynarec_test)
        IFX(X_AF) {
            BFCw(xFlags, F_AF, 1);
        }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit SHL8 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_shl8c(dynarec_arm_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4)
{
    if(!c)
        return;
    IFX(X_PEND) {
        MOV32w(s3, c);
        STRB_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRB_U12(s3, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, d_shl8);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    if(c<8) {
        IFX(X_CF|X_OF) {
            BFXILw(xFlags, s1, 8-c, 1);
        }
        LSLw(s1, s1, c);

        IFX(X_PEND) {
            STRB_U12(s1, xEmu, offsetof(x64emu_t, res));
        }
        COMP_ZFSF(s1, 8)
        IFX(X_OF) {
            if(c==1) {
                IFX2(X_SF, && !arm64_flagm) {} else {LSRw(s3, s1, 7);}
                EORw_REG(s4, s3, xFlags);  // CF is set if OF is asked
                BFIw(xFlags, s4, F_OF, 1);
            } else {
                BFCw(xFlags, F_OF, 1);
            }
        }
        if(box64_dynarec_test) IFX(X_AF) {
            BFCw(xFlags, F_AF, 1);
        }
        IFX(X_PF) {
            emit_pf(dyn, ninst, s1, s4);
        }
    } else {
        IFX(X_CF) {
            LSLw(s3, s1, c-1);
            BFXILw(xFlags, s3, 7, 1);   // insert F_CF from s3[7:1]
        }
        MOVw_REG(s1, xZR);
        IFX(X_OF) {
            BFCw(xFlags, F_OF, 1);
        }
        IFX(X_SF) {
            BFCw(xFlags, F_SF, 1);
        }
        if(box64_dynarec_test) IFX(X_AF) {
            BFCw(xFlags, F_AF, 1);
        }
        IFX(X_PF | X_ZF) {
            MOV32w(s3, 1);
            IFX(X_ZF) {
                BFIw(xFlags, s3, F_ZF, 1);
            }
            IFX(X_PF) {
                BFIw(xFlags, s3, F_PF, 1);
            }
        }
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
        SUBw_U12(s4, s2, 1);
        LSRw_REG(s4, s1, s4);
        BFIw(xFlags, s4, 0, 1);
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
    COMP_ZFSF(s1, 8)
    if(box64_dynarec_test)
        IFX(X_AF) {
            BFCw(xFlags, F_AF, 1);
        }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit SHR8 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_shr8c(dynarec_arm_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4)
{
    if (!c) return;
    IFX(X_PEND) {
        MOV32w(s3, c);
        STRB_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRB_U12(s3, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, d_shr8);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    IFX(X_CF) {
        if(c==1) {
            BFIw(xFlags, s1, 0, 1);
        } else {
            LSRw(s3, s1, c-1);
            BFIw(xFlags, s3, 0, 1);
        }
    }
    IFX(X_OF) {
        if(c==1) {
            LSRw(s4, s1, 7);
            BFIw(xFlags, s4, F_OF, 1);
        }
    }
    LSRw(s1, s1, c);
    IFX(X_PEND) {
        STRB_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    COMP_ZFSF(s1, 8)
    if(box64_dynarec_test)
        IFX(X_AF) {
            BFCw(xFlags, F_AF, 1);
        }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit SAR8 instruction, from s1 , shift s2, store result in s1 using s3 and s4 as scratch, s2 can be same as s3
void emit_sar8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX(X_PEND) {
        STRB_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRB_U12(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, d_sar8);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    IFX(X_CF) {
        SUBw_U12(s4, s2, 1);
        ASRw_REG(s4, s1, s4);
        BFIw(xFlags, s4, 0, 1);
    }
    ASRw_REG(s1, s1, s2);
    IFX(X_PEND) {
        STRB_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    COMP_ZFSF(s1, 8)
    IFX(X_OF) {
        CMPSw_U12(s2, 1);
        Bcond(cNE, 4+4);
            BFCw(xFlags, F_OF, 1);
    }
    if(box64_dynarec_test)
        IFX(X_AF) {
            BFCw(xFlags, F_AF, 1);
        }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit SAR8 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_sar8c(dynarec_arm_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4)
{
    if (!c) return;
    IFX(X_PEND) {
        MOV32w(s3, c);
        STRB_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRB_U12(s3, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, d_sar8);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    if(c<8) IFX(X_CF) {
        ASRw(s3, s1, c-1);
        BFIw(xFlags, s3, 0, 1);
    }
    ASRw(s1, s1, c);
    IFX(X_PEND) {
        STRB_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    if(c<8) {
        COMP_ZFSF(s1, 8)
        IFX(X_OF)
            if(c==1) {
                BFCw(xFlags, F_OF, 1);
        }
        IFX(X_PF) {
            emit_pf(dyn, ninst, s1, s4);
        }
    } else {
        LSRw(s3, s1, 7);
        BFIw(xFlags, s3, F_CF, 1);
        BFIw(wFlags, s3, F_SF, 1);
        EORw_mask(s3, s3, 0, 0);    //1
        BFIw(xFlags, s3, F_ZF, 1);
        MOV32w(s3, 1);
        BFIw(xFlags, s3, F_PF, 1);
    }
    if(box64_dynarec_test)
        IFX(X_AF) {
            BFCw(xFlags, F_AF, 1);
        }
}

// emit SHL16 instruction, from s1 , shift s2, store result in s1 using s3 and s4 as scratch. s3 can be same as s2
void emit_shl16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX(X_PEND) {
        STRH_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRH_U12(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, d_shl16);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    IFX(X_CF | X_OF) {
        MOV32w(s4, 16);
        SUBw_REG(s4, s4, s2);
        LSRw_REG(s4, s1, s4);
        BFIw(xFlags, s4, F_CF, 1);
    }
    LSLw_REG(s1, s1, s2);
    IFX(X_PEND) {
        STRH_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    COMP_ZFSF(s1, 16)
    IFX(X_OF) {
        CMPSw_U12(s2, 1);   // if s2==1
            IFX2(X_SF, && !arm64_flagm) {} else {LSRw(s3, s1, 15);}
            EORw_REG(s4, s3, xFlags);  // CF is set if OF is asked
            CSELw(s4, s4, wZR, cEQ);
            BFIw(xFlags, s4, F_OF, 1);
    }
    if(box64_dynarec_test)
        IFX(X_AF) {
            BFCw(xFlags, F_AF, 1);
        }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit SHL16 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_shl16c(dynarec_arm_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4)
{
    if (!c) return;
    IFX(X_PEND) {
        MOV32w(s3, c);
        STRH_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRH_U12(s3, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, d_shl16);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    if(c<16) {
        IFX(X_CF|X_OF) {
            LSRw(s3, s1, 16-c);
            BFIw(xFlags, s3, F_CF, 1);
        }
        LSLw(s1, s1, c);

        IFX(X_PEND) {
            STRH_U12(s1, xEmu, offsetof(x64emu_t, res));
        }
        COMP_ZFSF(s1, 16)
        IFX(X_OF) {
            if(c==1) {
                IFX2(X_SF, && !arm64_flagm) {} else {LSRw(s3, s1, 15);}
                EORw_REG(s4, s3, xFlags);  // CF is set if OF is asked
                BFIw(xFlags, s4, F_OF, 1);
            } else {
                BFCw(xFlags, F_OF, 1);
            }
        }
        if(box64_dynarec_test) IFX(X_AF) {
            BFCw(xFlags, F_AF, 1);
        }
        IFX(X_PF) {
            if(c>7) {
                // the 0xff area will be 0, so PF is known
                MOV32w(s3, 1);
                BFIw(xFlags, s3, F_PF, 1);
            } else
                emit_pf(dyn, ninst, s1, s4);
        }
    } else {
        IFX(X_CF) {
            LSLw(s3, s1, c-1);
            BFXILw(xFlags, s3, 15, 1);   // insert F_CF from s3[15:1]
        }
        MOVw_REG(s1, xZR);
        IFX(X_OF) {
            BFCw(xFlags, F_OF, 1);
        }
        IFX(X_SF) {
            BFCw(xFlags, F_SF, 1);
        }
        IFX(X_PF | X_ZF) {
            MOV32w(s3, 1);
            IFX(X_ZF) {
                BFIw(xFlags, s3, F_ZF, 1);
            }
            IFX(X_PF) {
                BFIw(xFlags, s3, F_PF, 1);
            }
        }
    }
}

// emit SHR16 instruction, from s1 , s2, store result in s1 using s3 and s4 as scratch, s2 can be same as s3
void emit_shr16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX(X_PEND) {
        STRH_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRH_U12(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, d_shr16);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    IFX(X_CF) {
        SUBw_U12(s4, s2, 1);
        LSRw_REG(s4, s1, s4);
        BFIw(xFlags, s4, 0, 1);
    }
    IFX(X_OF) {
        CMPSw_U12(s2, 1);   // if s2==1
            Bcond(cNE, 4+2*4);
            LSRw(s4, s1, 15);
            BFIw(xFlags, s4, F_OF, 1);
    }
    LSRw_REG(s1, s1, s2);
    IFX(X_PEND) {
        STRH_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    COMP_ZFSF(s1, 16)
    if(box64_dynarec_test)
        IFX(X_AF) {
            BFCw(xFlags, F_AF, 1);
        }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit SHR16 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_shr16c(dynarec_arm_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4)
{
    if(!c)
        return;
    IFX(X_PEND) {
        MOV32w(s3, c);
        STRH_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRH_U12(s3, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, d_shr16);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    IFX(X_CF) {
        if(c==1) {
            BFIw(xFlags, s1, 0, 1);
        } else {
            LSRw(s3, s1, c-1);
            BFIw(xFlags, s3, 0, 1);
        }
    }
    IFX(X_OF) {
        if(c==1) {
            LSRw(s4, s1, 15);
            BFIw(xFlags, s4, F_OF, 1);
        }
    }
    LSRw(s1, s1, c);
    IFX(X_PEND) {
        STRH_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    COMP_ZFSF(s1, 16)
    if(box64_dynarec_test)
        IFX(X_AF) {
            BFCw(xFlags, F_AF, 1);
        }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit SAR16 instruction, from s1 , shift s2, store result in s1 using s3 and s4 as scratch, s2 can be same as s3
void emit_sar16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX(X_PEND) {
        STRH_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRH_U12(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, d_sar16);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    IFX(X_CF) {
        SUBw_U12(s4, s2, 1);
        ASRw_REG(s4, s1, s4);
        BFIw(xFlags, s4, 0, 1);
    }
    ASRw_REG(s1, s1, s2);
    IFX(X_PEND) {
        STRH_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    COMP_ZFSF(s1, 16)
    IFX(X_OF) {
        CMPSw_U12(s2, 1);
        Bcond(cNE, 4+4);
            BFCw(xFlags, F_OF, 1);
    }
    if(box64_dynarec_test)
        IFX(X_AF) {
            BFCw(xFlags, F_AF, 1);
        }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit SAR16 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_sar16c(dynarec_arm_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4)
{
    if (!c) return;
    IFX(X_PEND) {
        MOV32w(s3, c);
        STRH_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRH_U12(s3, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, d_sar16);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    IFX(X_CF) {
        ASRw(s3, s1, c-1);
        BFIw(xFlags, s3, 0, 1);
    }
    ASRw(s1, s1, c);
    IFX(X_PEND) {
        STRH_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    COMP_ZFSF(s1, 16)
    IFX(X_OF)
        if(c==1) {
            BFCw(xFlags, F_OF, 1);
    }
    if(box64_dynarec_test)
        IFX(X_AF) {
            BFCw(xFlags, F_AF, 1);
        }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}
// emit ROL32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_rol32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4)
{
    MAYUSE(rex); MAYUSE(s1); MAYUSE(s3); MAYUSE(s4);

    if (!c) return;

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

    if (!c) return;

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

    if (!c) return;

    IFX(X_PEND) {
        MOV32w(s3, c);
        STRB_U12(s3, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, d_rol8);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    if(c&7) {
        int rc = 8-(c&7);
        ORRw_REG_LSL(s1, s1, s1, 8);
        LSRw(s1, s1, rc);
    }
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

    if (!c) return;

    IFX(X_PEND) {
        MOV32w(s3, c);
        STRB_U12(s3, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, d_ror8);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    if(c&7) {
        ORRw_REG_LSL(s1, s1, s1, 8);
        LSRw(s1, s1, c&7);
    }
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

    if (!c) return;

    IFX(X_PEND) {
        MOV32w(s3, c);
        STRH_U12(s3, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, d_rol16);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    if(c&15) {
        int rc = 16-(c&15);
        ORRw_REG_LSL(s1, s1, s1, 16);
        LSRw(s1, s1, rc);
    }
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

    if (!c) return;

    IFX(X_PEND) {
        MOV32w(s3, c);
        STRH_U12(s3, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, d_ror16);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    if(c&15) {
        ORRw_REG_LSL(s1, s1, s1, 16);
        LSRw(s1, s1, c&15);
    }
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

// emit RCL8 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_rcl8c(dynarec_arm_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4)
{
    MAYUSE(s1); MAYUSE(s3); MAYUSE(s4);

    if (!(c%9)) return;

    SET_DFNONE(s4);

    c%=9;
    BFIw(s1, xFlags, 8, 1); // insert cf
    IFX(X_OF|X_CF) {
        if(c!=8) {
            LSRw_IMM(s3, s1, 8-c);
        } else {
            MOVw_REG(s3, s1);
        }
    }
    ORRw_REG_LSL(s1, s1, s1, 9);    // insert s1 again
    LSRw_IMM(s1, s1, 9-c); // do the rcl
    IFX(X_PEND) {
        STRB_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_OF|X_CF) {
        IFX(X_CF) {
            BFIw(xFlags, s3, F_CF, 1);
        }
        IFX(X_OF) {
            if(c==1) {
                EORw_REG_LSR(s3, s3, s1, 7);
                BFIw(xFlags, s3, F_OF, 1);
            }
        }
    }
}

// emit RCR8 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_rcr8c(dynarec_arm_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4)
{
    MAYUSE(s1); MAYUSE(s3); MAYUSE(s4);

    if (!(c%9)) return;

    SET_DFNONE(s4);

    c%=9;
    IFX(X_OF) {
        MOVw_REG(s3, wFlags);
    }
    BFIw(s1, xFlags, 8, 1); // insert cf
    IFX(X_CF) {
        BFXILw(xFlags, s1, c-1, 1);
    }
    if(c>1) {
        ORRw_REG_LSL(s1, s1, s1, 9);    // insert s1 again
    }
    LSRw_IMM(s1, s1, c); // do the rcr
    IFX(X_OF) {
        if(c==1) {
            EORw_REG_LSR(s3, s3, s1, 7);
            BFIw(xFlags, s3, F_OF, 1);
        }
    }
    IFX(X_PEND) {
        STRB_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
}

// emit RCL16 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_rcl16c(dynarec_arm_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4)
{
    MAYUSE(s1); MAYUSE(s3); MAYUSE(s4);

    if (!(c%17)) return;

    SET_DFNONE(s4);

    c%=17;
    BFIw(s1, xFlags, 16, 1); // insert cf
    IFX(X_OF|X_CF) {
        if(c!=16) {
            LSRw_IMM(s3, s1, 16-c);
        } else {
            MOVw_REG(s3, s1);
        }
    }
    ORRx_REG_LSL(s1, s1, s1, 17);    // insert s1 again
    LSRx_IMM(s1, s1, 17-c); // do the rcl
    IFX(X_PEND) {
        STRH_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_CF) {
        BFIw(xFlags, s3, F_CF, 1);
    }
    IFX(X_OF) {
        if(c==1) {
            EORw_REG_LSR(s3, s3, s1, 15);
            BFIw(xFlags, s3, F_OF, 1);
        }
    }
}

// emit RCR16 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_rcr16c(dynarec_arm_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4)
{
    MAYUSE(s1); MAYUSE(s3); MAYUSE(s4);

    if (!(c%17)) return;

    SET_DFNONE(s4);

    c%=17;
    BFIw(s1, xFlags, 16, 1); // insert cf
    IFX(X_OF) {
        MOVw_REG(s3, wFlags);
    }
    IFX(X_CF) {
        BFXILx(xFlags, s1, c-1, 1);
    }
    if(c>1) {
        ORRx_REG_LSL(s1, s1, s1, 17);    // insert s1 again
    }
    LSRx_IMM(s1, s1, c); // do the rcr
    IFX(X_OF) {
        if(c==1) {
            EORw_REG_LSR(s3, s3, s1, 15);
            BFIw(xFlags, s3, F_OF, 1);
        }
    }
    IFX(X_PEND) {
        STRH_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
}

// emit RCL32/RCL64 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_rcl32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4)
{
    MAYUSE(s1); MAYUSE(s3); MAYUSE(s4);

    SET_DFNONE(s4);

    IFX(X_OF|X_CF) {
        LSRxw_IMM(s3, s1, (rex.w?64:32)-c);
    }
    if(c==1) {
        LSLxw(s1, s1, 1);
        BFIxw(s1, xFlags, 0, 1);
    } else {
        LSLxw(s4, s1, c);
        BFIxw(s4, xFlags, c-1, 1);
        ORRxw_REG_LSR(s1, s4, s1, (rex.w?65:33)-c);
    }
    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_CF) {
        BFIw(xFlags, s3, F_CF, 1);
    }
    IFX(X_OF) {
        if(c==1) {
            EORxw_REG_LSR(s3, s3, s1, rex.w?63:31);
            BFIw(xFlags, s3, F_OF, 1);
        }
    }
}
// emit RCR32/RCR64 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_rcr32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4)
{
    MAYUSE(s1); MAYUSE(s3); MAYUSE(s4);

    SET_DFNONE(s4);

    IFX(X_OF) {
        if(c==1) {
            EORxw_REG_LSR(s3, xFlags, s1, rex.w?63:31);
            BFIw(xFlags, s3, F_OF, 1);
        }
    }
    IFX(X_CF) {
        BFXILxw(s3, s1, c-1, 1);
    }
    if(c==1) {
        LSRxw(s1, s1, 1);
        BFIxw(s1, xFlags, rex.w?63:31, 1);
    } else {
        LSRxw(s4, s1, c);
        BFIxw(s4, xFlags, (rex.w?64:32)-c, 1);
        ORRxw_REG_LSL(s1, s4, s1, (rex.w?65:33)-c);
    }
    IFX(X_CF) {
        BFIw(wFlags, s3, 0, 1);
    }
    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
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
        SET_DF(s4, rex.w?d_shrd64:d_shrd32);
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
        BFXILxw(xFlags, s1, c-1, 1);    // set CF
    }
    IFX(X_OF) {
        if(c==1) {
            LSRxw(s4, s1, rex.w?63:31);
            BFIw(xFlags, s4, F_OF, 1);  // store sign for later use
        }
    }
    EXTRxw(s1, s2, s1, c);
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
            UBFXx(s3, s1, rex.w?63:31, 1);
            EORw_REG_LSL(xFlags, xFlags, s3, F_OF);  // OF is set if sign changed
        }
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

void emit_shld32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, uint32_t c, int s3, int s4)
{
    c&=(rex.w?0x3f:0x1f);
    IFX(X_PEND) {
        MOV32w(s3, c);
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRxw_U12(s3, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, rex.w?d_shld64:d_shld32);
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
    IFX(X_CF) {
        LSRxw(s3, s1, (rex.w?64:32)-c);
        BFIxw(xFlags, s3, F_CF, 1);
    }
    IFX(X_OF) {
        LSRxw(s3, s1, rex.w?63:31);
        BFIw(xFlags, s3, F_OF, 1);  // store current sign for later use
    }
    EXTRxw(s1, s1, s2, (rex.w?64:32)-c);

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
            UBFXx(s3, s1, rex.w?63:31, 1);
            EORw_REG_LSL(xFlags, xFlags, s3, F_OF);  // OF is set if sign changed
        } else {
            BFCw(xFlags, F_OF, 1);
        }
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit SHRD32 instruction, from s1, fill s2 , shift s5, store result in s1 using s3 and s4 as scratch
void emit_shrd32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int s5, int s3, int s4)
{
    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRxw_U12(s5, xEmu, offsetof(x64emu_t, op2));
        // same flags computation as with shl64/shl32
        SET_DF(s4, rex.w?d_shrd64:d_shrd32);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    IFX(X_CF) {
        SUBw_U12(s3, s5, 1);
        LSRxw_REG(s3, s1, s3);
        BFIw(xFlags, s3, F_CF, 1);
    }
    IFX(X_OF) {
        LSRxw(s4, s1, rex.w?63:31);
        BFIw(xFlags, s4, F_OF, 1);  // store sign fr now
    }
    LSRxw_REG(s3, s1, s5);
    SUBxw_U12(s4, s5, rex.w?64:32);
    NEGxw_REG(s4, s4);
    LSLxw_REG(s4, s2, s4);
    ORRxw_REG(s1, s3, s4);
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
        BFIw(xFlags, s4, F_SF, 1);
    }
    IFX(X_OF) {
        CMPSw_U12(s5, 1);
        Bcond(cNE, 4+2*4);
            UBFXx(s3, s1, rex.w?63:31, 1);
            EORw_REG_LSL(xFlags, xFlags, s3, F_OF);  // OF is set if sign changed
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

void emit_shld32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int s5, int s3, int s4)
{
    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRxw_U12(s5, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, rex.w?d_shld64:d_shld32);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    MOV32w(s3, (rex.w?64:32));
    SUBw_REG(s3, s3, s5);
    IFX(X_CF) {
        LSRxw_REG(s4, s1, s3);
        BFIxw(xFlags, s4, F_CF, 1);
    }
    IFX(X_OF) {
        LSRxw(s4, s1, rex.w?63:31);
        BFIw(xFlags, s4, F_OF, 1);  // store current sign for later use
    }
    LSLxw_REG(s4, s1, s5);
    LSRxw_REG(s3, s2, s3);
    ORRxw_REG(s1, s3, s4);

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
        CMPSw_U12(s5, 1);
        Bcond(cNE, 4+2*4);
            UBFXx(s3, s1, rex.w?63:31, 1);
            EORw_REG_LSL(xFlags, xFlags, s3, F_OF);  // OF is set if sign changed
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit SHRD16 instruction, from s1, fill s2 , constant c, store result in s1 using s3 and s4 as scratch
void emit_shrd16c(dynarec_arm_t* dyn, int ninst, int s1, int s2, uint32_t c, int s3, int s4)
{
    c&=0x1f;
    IFX(X_PEND) {
        MOV32w(s3, c);
        STRH_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRH_U12(s3, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, d_shrd16);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    if(!c) {
        IFX(X_PEND) {
            STRH_U12(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }
    ORRw_REG_LSL(s1, s1, s2, 16);   // create concat first
    IFX(X_CF) {
        BFXILw(xFlags, s1, c-1, 1);    // set CF
    }
    IFX(X_OF) {
        if(c==1) {
            LSRw(s4, s1, 15);
            BFIw(xFlags, s4, F_OF, 1);  // store sign for later use
        }
    }
    RORw(s1, s1, c);
    IFX(X_PEND) {
        STRH_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    COMP_ZFSF(s1, 16)
    IFX(X_OF) {
        if(c==1) {
            UBFXw(s3, s1, 15, 1);
            EORw_REG_LSL(xFlags, xFlags, s3, F_OF);  // OF is set if sign changed
        }
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit SHRD16 instruction, from s1, fill s2 , shift s5, store result in s1 using s3 and s4 as scratch
void emit_shrd16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s5, int s3, int s4)
{
    IFX(X_PEND) {
        STRH_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRH_U12(s5, xEmu, offsetof(x64emu_t, op2));
        // same flags computation as with shl64/shl32
        SET_DF(s4, d_shrd16);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    ORRw_REG_LSL(s1, s1, s2, 16);   // create concat first
    IFX(X_CF) {
        SUBw_U12(s3, s5, 1);
        LSRw_REG(s3, s1, s3);
        BFIw(xFlags, s3, F_CF, 1);
    }
    IFX(X_OF) {
        LSRw(s4, s1, 15);
        BFIw(xFlags, s4, F_OF, 1);  // store sign fr now
    }
    RORw_REG(s1, s1, s5);
    IFX(X_PEND) {
        STRH_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    COMP_ZFSF(s1, 16)
    IFX(X_OF) {
        CMPSw_U12(s5, 1);
        Bcond(cNE, 4+2*4);
            UBFXw(s3, s1, 15, 1);
            EORw_REG_LSL(xFlags, xFlags, s3, F_OF);  // OF is set if sign changed
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

void emit_shld16c(dynarec_arm_t* dyn, int ninst, int s1, int s2, uint32_t c, int s3, int s4)
{
    c&=0x1f;
    IFX(X_PEND) {
        MOV32w(s3, c);
        STRH_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRH_U12(s3, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, d_shld16);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    if(c==0) {
        IFX(X_OF) {
            BFCw(xFlags, F_OF, 1);
        }
        IFX(X_PEND) {
            STRH_U12(s1, xEmu, offsetof(x64emu_t, res));
        }
        return;
    }
    ORRw_REG_LSL(s1, s1, s2, 16);   // create concat first
    IFX(X_CF) {
        if(c<16)
            LSRw(s3, s1, 16-c);
        else
            LSRw(s3, s2, 32-c);
        BFIw(xFlags, s3, F_CF, 1);
    }
    IFX(X_OF) {
        LSRw(s3, s1, 15);
        BFIw(xFlags, s3, F_OF, 1);  // store current sign for later use
    }
    RORw(s1, s1, 32-c);

    IFX(X_PEND) {
        STRH_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    COMP_ZFSF(s1, 16)
    IFX(X_OF) {
        if(c==1) {
            UBFXw(s3, s1, 15, 1);
            EORw_REG_LSL(xFlags, xFlags, s3, F_OF);  // OF is set if sign changed
        } else {
            BFCw(xFlags, F_OF, 1);
        }
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

void emit_shld16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s5, int s3, int s4)
{
    IFX(X_PEND) {
        STRH_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRH_U12(s5, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, d_shld16);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    IFX(X_CF) {
        ORRw_REG_LSL(s4, s2, s1, 16);
        MOV32w(s3, 32);
        SUBw_REG(s3, s3, s5);
        LSRw_REG(s3, s4, s3);
        BFIw(xFlags, s3, F_CF, 1);
    }
    IFX(X_OF) {
        LSRw(s3, s1, 15);
        BFIw(xFlags, s3, F_OF, 1);  // store current sign for later use
    }
    ORRw_REG_LSL(s1, s1, s2, 16);   // create concat first
    MOV32w(s3, 32);
    SUBw_REG(s3, s3, s5);
    RORw_REG(s1, s1, s3);

    IFX(X_PEND) {
        STRH_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    COMP_ZFSF(s1, 16)
    IFX(X_OF) {
        CMPSw_U12(s5, 1);
        Bcond(cNE, 4+2*4);
            UBFXw(s3, s1, 15, 1);
            EORw_REG_LSL(xFlags, xFlags, s3, F_OF);  // OF is set if sign changed
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}
