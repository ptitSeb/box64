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

#include "arm64_printer.h"
#include "dynarec_arm64_private.h"
#include "dynarec_arm64_functions.h"
#include "../dynarec_helper.h"

// emit CMP32 instruction, from cmp s1, s2, using s3 and s4 as scratch
void emit_cmp32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    MAYUSE(s1); MAYUSE(s2);
    IFX_PENDOR0 {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRxw_U12(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s4, rex.w?d_cmp64:d_cmp32);
    } else {
        SET_DFNONE();
    }
    IFX(X_AF) {
        ORNxw_REG(s3, s2, s1);  // s3 = ~op1 | op2
        BICxw(s4, s2, s1);      // s4 = ~op1 & op2
    }
    SUBSxw_REG(s5, s1, s2);   // res = s1 - s2
    IFX_PENDOR0 {
        STRxw_U12(s5, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_AF) {
        ANDxw_REG(s3, s3, s5);   // s3 = (~op1 | op2) & res
        ORRxw_REG(s3, s3, s4);   // s3 = (~op1 & op2) | ((~op1 | op2) & res)
        LSRxw(s4, s3, 3);
        BFIx(xFlags, s4, F_AF, 1);    // AF: bc & 0x08
    }
    IFX(X_ZF) {
        IFNATIVE(NF_EQ) {} else {
            CSETw(s4, cEQ);
            BFIw(xFlags, s4, F_ZF, 1);
        }
    }
    IFX(X_CF) {
        // inverted carry
        IFNATIVE(NF_CF) {
            GEN_INVERTED_CARRY();
        } else {
            CSETw(s4, cCC);
            BFIw(xFlags, s4, F_CF, 1);
        }
    }
    IFX(X_OF) {
        IFNATIVE(NF_VF) {} else {
            CSETw(s4, cVS);
            BFIw(xFlags, s4, F_OF, 1);
        }
    }
    IFX(X_SF) {
        IFNATIVE(NF_SF) {} else {
            LSRxw(s3, s5, (rex.w)?63:31);
            BFIw(xFlags, s3, F_SF, 1);
        }
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s5, s4);
    }
}

// emit CMP32 instruction, from cmp s1 , 0, using s3 and s4 as scratch
void emit_cmp32_0(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s3, int s4)
{
    IFX_PENDOR0 {
        MOV64xw(s4, 0);
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRxw_U12(s4, xEmu, offsetof(x64emu_t, op2));
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
        SET_DF(s4, rex.w?d_cmp64:d_cmp32);
    } else {
        SET_DFNONE();
    }
    SUBSxw_U12(s3, s1, 0);   // res = s1 - 0
    // and now the tricky ones (and mostly unused), PF and AF
    // bc = (res & (~d | s)) | (~d & s) => is 0 here...
    IFX(X_CF) {
        IFNATIVE(NF_CF) {
            GEN_INVERTED_CARRY();
        } else {
            BFCw(xFlags, F_CF, 1);
        }
    }
    IFX(X_OF) {
        IFNATIVE(NF_VF) {} else {
            BFCw(xFlags, F_OF, 1);
        }
    }
    IFX(X_AF) {
        BFCw(xFlags, F_AF, 1);
    }
    IFX(X_ZF) {
        IFNATIVE(NF_EQ) {} else {
            CSETw(s4, cEQ);
            BFIw(xFlags, s4, F_ZF, 1);
        }
    }
    IFX(X_SF) {
        IFNATIVE(NF_SF) {} else {
            LSRxw(s3, s1, (rex.w)?63:31);
            BFIw(xFlags, s3, F_SF, 1);
        }
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit CMP16 instruction, from cmp s1, s2, using s3 and s4 as scratch
void emit_cmp16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    MAYUSE(s1); MAYUSE(s2);
    IFX_PENDOR0 {
        STRH_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRH_U12(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, d_cmp16);
    } else {
        SET_DFNONE();
    }
    IFX(X_AF) {
        ORNw_REG(s3, s2, s1);  // s3 = ~op1 | op2
        BICw(s4, s2, s1);      // s4 = ~op1 & op2
    }
    IFX(X_CF|X_ZF|X_SF|X_OF) {
        LSLw(s5, s1, 16);
        SUBSw_REG_LSL(s5, s5, s2, 16);
        IFX(X_AF) {
            ANDw_REG_LSR(s3, s3, s5, 16);   // s3 = (~op1 | op2) & res
            ORRw_REG(s3, s3, s4);   // s3 = (~op1 & op2) | ((~op1 | op2) & res)
            LSRw(s4, s3, 3);
            BFIx(xFlags, s4, F_AF, 1);    // AF: bc & 0x08
        }
        IFX(X_ZF) {
            IFNATIVE(NF_EQ) {} else {
                CSETw(s4, cEQ);
                BFIw(xFlags, s4, F_ZF, 1);
            }
        }
        IFX(X_CF) {
            // inverted carry
            IFNATIVE(NF_CF) {
                GEN_INVERTED_CARRY();
            } else {
                CSETw(s4, cCC);
                BFIw(xFlags, s4, F_CF, 1);
            }
        }
        IFX(X_OF) {
            IFNATIVE(NF_VF) {} else {
                CSETw(s4, cVS);
                BFIw(xFlags, s4, F_OF, 1);
            }
        }
        IFX(X_SF) {
            IFNATIVE(NF_SF) {} else {
                CSETw(s4, cMI);
                BFIw(xFlags, s4, F_SF, 1);
            }
        }
        IFX(X_PF|X_PEND) {
            LSRw(s5, s5, 16);
        }
    } else {
        SUBw_REG(s5, s1, s2);   // res = s1 - s2
        IFX(X_AF) {
            ANDw_REG(s3, s3, s5);   // s3 = (~op1 | op2) & res
            ORRw_REG(s3, s3, s4);   // s3 = (~op1 & op2) | ((~op1 | op2) & res)
            LSRw(s4, s3, 3);
            BFIx(xFlags, s4, F_AF, 1);    // AF: bc & 0x08
        }
    }
    IFX_PENDOR0 {
        STRH_U12(s5, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s5, s4);
    }
}

// emit CMP16 instruction, from cmp s1 , #0, using s3 and s4 as scratch
void emit_cmp16_0(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4)
{
    IFX_PENDOR0 {
        MOV32w(s3, 0);
        STRH_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRH_U12(s3, xEmu, offsetof(x64emu_t, op2));
        STRH_U12(s1, xEmu, offsetof(x64emu_t, res));
        SET_DF(s3, d_cmp16);
    } else {
        SET_DFNONE();
    }
    // bc = (res & (~d | s)) | (~d & s) = 0
    IFX(X_CF | X_AF | X_OF) {
        MOV32w(s3, (1<<F_CF)|(1<<F_AF)|(1<<F_OF));
        BICw(xFlags, xFlags, s3);
    }
    COMP_ZFSF(s1, 16)
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}
// emit CMP8 instruction, from cmp s1, s2, using s3 and s4 as scratch
void emit_cmp8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    MAYUSE(s1); MAYUSE(s2);
    IFX_PENDOR0 {
        STRB_U12(s1, xEmu, offsetof(x64emu_t, op1));
        STRB_U12(s2, xEmu, offsetof(x64emu_t, op2));
        SET_DF(s3, d_cmp8);
    } else {
        SET_DFNONE();
    }
    IFX(X_AF) {
        ORNw_REG(s3, s2, s1);  // s3 = ~op1 | op2
        BICw(s4, s2, s1);      // s4 = ~op1 & op2
    }
    IFX(X_CF|X_ZF|X_SF|X_OF) {
        LSLw(s5, s1, 24);
        SUBSw_REG_LSL(s5, s5, s2, 24);
        IFX(X_AF) {
            ANDw_REG_LSR(s3, s3, s5, 24);   // s3 = (~op1 | op2) & res
            ORRw_REG(s3, s3, s4);   // s3 = (~op1 & op2) | ((~op1 | op2) & res)
            LSRw(s4, s3, 3);
            BFIx(xFlags, s4, F_AF, 1);    // AF: bc & 0x08
        }
        IFX(X_ZF) {
            IFNATIVE(NF_EQ) {} else {
                CSETw(s4, cEQ);
                BFIw(xFlags, s4, F_ZF, 1);
            }
        }
        IFX(X_CF) {
            // inverted carry
            IFNATIVE(NF_CF) {
                GEN_INVERTED_CARRY();
            } else {
                CSETw(s4, cCC);
                BFIw(xFlags, s4, F_CF, 1);
            }
        }
        IFX(X_OF) {
            IFNATIVE(NF_VF) {} else {
                CSETw(s4, cVS);
                BFIw(xFlags, s4, F_OF, 1);
            }
        }
        IFX(X_SF) {
            IFNATIVE(NF_SF) {} else {
                CSETw(s4, cMI);
                BFIw(xFlags, s4, F_SF, 1);
            }
        }
        IFX(X_PF|X_PEND) {
            LSRw(s5, s5, 24);
        }
    } else {
        SUBw_REG(s5, s1, s2);   // res = s1 - s2
        IFX(X_AF) {
            ANDw_REG(s3, s3, s5);   // s3 = (~op1 | op2) & res
            ORRw_REG(s3, s3, s4);   // s3 = (~op1 & op2) | ((~op1 | op2) & res)
            LSRw(s4, s3, 3);
            BFIx(xFlags, s4, F_AF, 1);    // AF: bc & 0x08
        }
    }
    IFX_PENDOR0 {
        STRB_U12(s5, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s5, s4);
    }
}
// emit CMP8 instruction, from cmp s1 , 0, using s3 and s4 as scratch
void emit_cmp8_0(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4)
{
    IFX_PENDOR0 {
        STRB_U12(s1, xEmu, offsetof(x64emu_t, op1));
        MOV32w(s4, 0);
        STRB_U12(s4, xEmu, offsetof(x64emu_t, op2));
        STRB_U12(s1, xEmu, offsetof(x64emu_t, res));
        SET_DF(s3, d_cmp8);
    } else {
        SET_DFNONE();
    }
    // bc = (res & (~d | s)) | (~d & s) = 0
    IFX(X_CF | X_AF | X_OF) {
        MOV32w(s3, (1<<F_CF)|(1<<F_AF)|(1<<F_OF));
        BICw(xFlags, xFlags, s3);
    }
    COMP_ZFSF(s1, 8)
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit TEST32 instruction, from test s1, s2, using s3, s4 and s5 as scratch
void emit_test32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5)
{
    MAYUSE(s1); MAYUSE(s2); MAYUSE(s3); MAYUSE(s4);
    IFX_PENDOR0 {
        SET_DF(s3, rex.w?d_tst64:d_tst32);
    } else {
        SET_DFNONE();
    }
    IFX(X_CF) {
        IFNATIVE(NF_CF) {} else {
            BFCw(xFlags, F_CF, 1);
        }
    }
    IFX(X_OF) {
        IFNATIVE(NF_VF) {} else {
            BFCw(xFlags, F_OF, 1);
        }
    }
    IFX(X_AF) {
        BFCw(xFlags, F_AF, 1);
    }
    ANDSxw_REG(s3, s1, s2);   // res = s1 & s2
    IFX_PENDOR0 {
        STRxw_U12(s3, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_ZF) {
        IFNATIVE(NF_EQ) {} else {
            CSETw(s4, cEQ);
            BFIw(xFlags, s4, F_ZF, 1);
        }
    }
    IFX(X_SF) {
        IFNATIVE(NF_SF) {} else {
            LSRxw(s4, s3, rex.w?63:31);
            BFIw(xFlags, s4, F_SF, 1);
        }
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s3, s5);
    }
}

// emit TEST32 instruction, from test s1, s2, using s3, s4 and s5 as scratch
void emit_test32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4, int s5)
{
    MAYUSE(s1); MAYUSE(s3); MAYUSE(s4);
    int mask = convert_bitmask_xw(c);
    IFX_PENDOR0 {
        SET_DF(s3, rex.w?d_tst64:d_tst32);
    } else {
        SET_DFNONE();
    }
    IFX(X_CF) {
        IFNATIVE(NF_CF) {} else {
            BFCw(xFlags, F_CF, 1);
        }
    }
    IFX(X_OF) {
        IFNATIVE(NF_VF) {} else {
            BFCw(xFlags, F_OF, 1);
        }
    }
    IFX(X_AF) {
        BFCw(xFlags, F_AF, 1);
    }
    if(mask) {
        ANDSxw_mask(s3, s1, (mask>>12)&1, mask&0x3F, (mask>>6)&0x3F);
    } else {
        MOV64xw(s3, c);
        ANDSxw_REG(s3, s1, s3);   // res = s1 & s2
    }
    IFX_PENDOR0 {
        STRxw_U12(s3, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_ZF) {
        IFNATIVE(NF_EQ) {} else {
            CSETw(s4, cEQ);
            BFIw(xFlags, s4, F_ZF, 1);
        }
    }
    IFX(X_SF) {
        IFNATIVE(NF_SF) {} else {
            LSRxw(s4, s3, rex.w?63:31);
            BFIw(xFlags, s4, F_SF, 1);
        }
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s3, s5);
    }
}

// emit TEST16 instruction, from test s1, s2, using s3, s4 and s5 as scratch
void emit_test16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    MAYUSE(s1); MAYUSE(s2);
    IFX_PENDOR0 {
        SET_DF(s3, d_tst16);
    } else {
        SET_DFNONE();
    }
    IFX(X_CF|X_ZF|X_SF|X_OF) {
        LSLw(s5, s1, 16);
        ANDSw_REG_LSL(s5, s5, s2, 16);
        IFX(X_ZF) {
            IFNATIVE(NF_EQ) {} else {
                CSETw(s4, cEQ);
                BFIw(xFlags, s4, F_ZF, 1);
            }
        }
        IFX(X_CF) {
            IFNATIVE(NF_CF) {} else {BFCw(xFlags, F_CF, 1);}
        }
        IFX(X_OF) {
            IFNATIVE(NF_VF) {} else {BFCw(xFlags, F_OF, 1);}
        }
        IFX(X_SF) {
            IFNATIVE(NF_SF) {} else {
                CSETw(s4, cMI);
                BFIw(xFlags, s4, F_SF, 1);
            }
        }
        IFX(X_PF|X_PEND) {
            LSRw(s5, s5, 16);
        }
    } else {
        ANDw_REG(s5, s1, s2);   // res = s1 & s2
    }
    IFX(X_AF) {
        BFCw(xFlags, F_AF, 1);
    }
    IFX_PENDOR0 {
        STRH_U12(s5, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s5, s4);
    }
}

// emit TEST16 instruction, from test s1, s2, using s3, s4 and s5 as scratch
void emit_test16c(dynarec_arm_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4, int s5)
{
    MAYUSE(s1);
    int mask = 0;
    IFX_PENDOR0 {
        SET_DF(s3, d_tst16);
    } else {
        SET_DFNONE();
    }
    IFX(X_CF|X_ZF|X_SF|X_OF) {
        LSLw(s5, s1, 16);
        mask = convert_bitmask_w(c<<16);
        if(mask) {
            ANDSw_mask(s5, s5, mask&0x3F, (mask>>6)&0x3F);
        } else {
            MOV32w(s3, c);
            ANDSw_REG_LSL(s5, s5, s3, 16);
        }
        IFX(X_ZF) {
            IFNATIVE(NF_EQ) {} else {
                CSETw(s4, cEQ);
                BFIw(xFlags, s4, F_ZF, 1);
            }
        }
        IFX(X_CF) {
            IFNATIVE(NF_CF) {} else {BFCw(xFlags, F_CF, 1);}
        }
        IFX(X_OF) {
            IFNATIVE(NF_VF) {} else {BFCw(xFlags, F_OF, 1);}
        }
        IFX(X_SF) {
            IFNATIVE(NF_SF) {} else {
                CSETw(s4, cMI);
                BFIw(xFlags, s4, F_SF, 1);
            }
        }
        IFX(X_PF|X_PEND) {
            LSRw(s5, s5, 16);
        }
    } else {
        mask = convert_bitmask_w(c);
        if(mask) {
            ANDw_mask(s5, s1, mask&0x3F, (mask>>6)&0x3F);
        } else {
            MOV32w(s5, c);
            ANDw_REG(s5, s1, s5);   // res = s1 & s2
        }
    }
    IFX(X_AF) {
        BFCw(xFlags, F_AF, 1);
    }
    IFX_PENDOR0 {
        STRH_U12(s5, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s5, s4);
    }
}

// emit TEST8 instruction, from test s1, s2, using s3, s4 and s5 as scratch
void emit_test8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5)
{
    MAYUSE(s1); MAYUSE(s2);
    IFX_PENDOR0 {
        SET_DF(s3, d_tst8);
    } else {
        SET_DFNONE();
    }
    IFX(X_CF|X_ZF|X_SF|X_OF) {
        LSLw(s5, s1, 24);
        ANDSw_REG_LSL(s5, s5, s2, 24);
        IFX(X_ZF) {
            IFNATIVE(NF_EQ) {} else {
                CSETw(s4, cEQ);
                BFIw(xFlags, s4, F_ZF, 1);
            }
        }
        IFX(X_CF) {
            IFNATIVE(NF_CF) {} else {BFCw(xFlags, F_CF, 1);}
        }
        IFX(X_OF) {
            IFNATIVE(NF_VF) {} else {BFCw(xFlags, F_OF, 1);}
        }
        IFX(X_SF) {
            IFNATIVE(NF_SF) {} else {
                CSETw(s4, cMI);
                BFIw(xFlags, s4, F_SF, 1);
            }
        }
        IFX(X_PF|X_PEND) {
            LSRw(s5, s5, 24);
        }
    } else {
        ANDw_REG(s5, s1, s2);   // res = s1 & s2
    }
    IFX(X_AF) {
        BFCw(xFlags, F_AF, 1);
    }
    IFX_PENDOR0 {
        STRB_U12(s5, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s5, s4);
    }
}

// emit TEST8 instruction, from test s1, s2, using s3, s4 and s5 as scratch
void emit_test8c(dynarec_arm_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4, int s5)
{
    MAYUSE(s1);
    int mask = 0;
    IFX_PENDOR0 {
        SET_DF(s3, d_tst8);
    } else {
        SET_DFNONE();
    }
    IFX(X_CF|X_ZF|X_SF|X_OF) {
        LSLw(s5, s1, 24);
        mask = convert_bitmask_w(c<<24);
        if(mask) {
            ANDSw_mask(s5, s5, mask&0x3F, (mask>>6)&0x3F);
        } else {
            MOV32w(s3, c);
            ANDSw_REG_LSL(s5, s5, s3, 24);
        }
        IFX(X_ZF) {
            IFNATIVE(NF_EQ) {} else {
                CSETw(s4, cEQ);
                BFIw(xFlags, s4, F_ZF, 1);
            }
        }
        IFX(X_CF) {
            IFNATIVE(NF_CF) {} else {BFCw(xFlags, F_CF, 1);}
        }
        IFX(X_OF) {
            IFNATIVE(NF_VF) {} else {BFCw(xFlags, F_OF, 1);}
        }
        IFX(X_SF) {
            IFNATIVE(NF_SF) {} else {
                CSETw(s4, cMI);
                BFIw(xFlags, s4, F_SF, 1);
            }
        }
        IFX(X_PF|X_PEND) {
            LSRw(s5, s5, 24);
        }
    } else {
        mask = convert_bitmask_w(c);
        if(mask) {
            ANDw_mask(s5, s1, mask&0x3F, (mask>>6)&0x3F);
        } else {
            MOV32w(s5, c);
            ANDw_REG(s5, s1, s5);   // res = s1 & s2
        }
    }
    IFX(X_AF) {
        BFCw(xFlags, F_AF, 1);
    }
    IFX_PENDOR0 {
        STRB_U12(s5, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s5, s4);
    }
}
