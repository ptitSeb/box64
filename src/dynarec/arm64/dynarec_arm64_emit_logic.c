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

// emit OR32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_or32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4)
{
    MAYUSE(s2);
    IFX(X_PEND) {
        SET_DF(s4, rex.w?d_or64:d_or32);
    } else {
        SET_DFNONE();
    }
    ORRxw_REG(s1, s1, s2);
    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    int need_tst = 0;
    IFX(X_ZF) need_tst = 1;
    IFXNATIVE(X_SF, NF_SF) need_tst = 1;
    IFXNATIVE(X_OF, NF_VF) need_tst = 1;
    IFXNATIVE(X_CF, NF_CF) need_tst = 1;
    if(need_tst) TSTxw_REG(s1, s1);
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
    IFX(X_ZF) {
        IFNATIVE(NF_EQ) {} else {
            CSETw(s3, cEQ);
            BFIw(xFlags, s3, F_ZF, 1);
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

// emit OR32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_or32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4)
{
    int mask = convert_bitmask_xw(c);
    if(!mask) {
        MOV64xw(s3, c);
        emit_or32(dyn, ninst, rex, s1, s3, s3, s4);
        return;
    }
    IFX(X_PEND) {
        SET_DF(s4, rex.w?d_or64:d_or32);
    } else {
        SET_DFNONE();
    }
    ORRxw_mask(s1, s1, (mask>>12)&1, mask&0x3F, (mask>>6)&0x3F);
    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    int need_tst = 0;
    IFX(X_ZF) need_tst = 1;
    IFXNATIVE(X_SF, NF_SF) need_tst = 1;
    IFXNATIVE(X_OF, NF_VF) need_tst = 1;
    IFXNATIVE(X_CF, NF_CF) need_tst = 1;
    if(need_tst) TSTxw_REG(s1, s1);
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
    IFX(X_ZF) {
        IFNATIVE(NF_EQ) {} else {
            CSETw(s3, cEQ);
            BFIw(xFlags, s3, F_ZF, 1);
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

// emit XOR32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_xor32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4)
{
    MAYUSE(s2);
    IFX(X_PEND) {
        SET_DF(s4, rex.w?d_xor64:d_xor32);
    } else {
        SET_DFNONE();
    }
    EORxw_REG(s1, s1, s2);
    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    int need_tst = 0;
    IFX(X_ZF) need_tst = 1;
    IFXNATIVE(X_SF, NF_SF) need_tst = 1;
    IFXNATIVE(X_OF, NF_VF) need_tst = 1;
    IFXNATIVE(X_CF, NF_CF) need_tst = 1;
    if(need_tst) TSTxw_REG(s1, s1);
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
    IFX(X_ZF) {
        IFNATIVE(NF_EQ) {} else {
            CSETw(s3, cEQ);
            BFIw(xFlags, s3, F_ZF, 1);
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

// emit XOR32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_xor32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4)
{
    int mask = convert_bitmask_xw(c);
    if(!mask && !((!rex.w && c==0xffffffffLL) || (rex.w && c==0xffffffffffffffffLL))) {
        MOV64xw(s3, c);
        emit_xor32(dyn, ninst, rex, s1, s3, s3, s4);
        return;
    }
    IFX(X_PEND) {
        SET_DF(s4, rex.w?d_xor64:d_xor32);
    } else {
        SET_DFNONE();
    }
    if(!mask) {
        MVNxw_REG(s1, s1);
    } else {
        EORxw_mask(s1, s1, (mask>>12)&1, mask&0x3F, (mask>>6)&0x3F);
    }
    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    int need_tst = 0;
    IFX(X_ZF) need_tst = 1;
    IFXNATIVE(X_SF, NF_SF) need_tst = 1;
    IFXNATIVE(X_OF, NF_VF) need_tst = 1;
    IFXNATIVE(X_CF, NF_CF) need_tst = 1;
    if(need_tst) TSTxw_REG(s1, s1);
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
    IFX(X_ZF) {
        IFNATIVE(NF_EQ) {} else {
            CSETw(s3, cEQ);
            BFIw(xFlags, s3, F_ZF, 1);
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

// emit AND32 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch
void emit_and32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4)
{
    MAYUSE(s2);
    IFX(X_PEND) {
        SET_DF(s4, rex.w?d_and64:d_and32);
    } else {
        SET_DFNONE();
    }
    IFX(X_ZF|X_SF|X_CF|X_OF) {
        ANDSxw_REG(s1, s1, s2);
    } else {
        ANDxw_REG(s1, s1, s2);
    }
    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
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
    IFX(X_ZF) {
        IFNATIVE(NF_EQ) {} else {
            CSETw(s3, cEQ);
            BFIw(xFlags, s3, F_ZF, 1);
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

// emit AND32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_and32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4)
{
    int mask = convert_bitmask_xw(c);
    if(!mask) {
        MOV64xw(s3, c);
        emit_and32(dyn, ninst, rex, s1, s3, s3, s4);
        return;
    }
    IFX(X_PEND) {
        SET_DF(s4, rex.w?d_and64:d_and32);
    } else {
        SET_DFNONE();
    }
    IFX(X_ZF|X_SF|X_CF|X_OF) {
        ANDSxw_mask(s1, s1, (mask>>12)&1, mask&0x3F, (mask>>6)&0x3F);
    } else {
        ANDxw_mask(s1, s1, (mask>>12)&1, mask&0x3F, (mask>>6)&0x3F);
    }
    IFX(X_PEND) {
        STRxw_U12(s1, xEmu, offsetof(x64emu_t, res));
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
    IFX(X_ZF) {
        IFNATIVE(NF_EQ) {} else {
            CSETw(s3, cEQ);
            BFIw(xFlags, s3, F_ZF, 1);
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

// emit OR8 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch, s4 can be same as s2 (and so s2 destroyed)
void emit_or8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    MAYUSE(s2);
    IFX(X_PEND) {
        SET_DF(s4, d_or8);
    } else {
        SET_DFNONE();
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
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit OR8 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_or8c(dynarec_arm_t* dyn, int ninst, int s1, uint8_t c, int s3, int s4)
{
    int mask = convert_bitmask_w(c);
    if(!mask) {
        MOV32w(s3, c);
        emit_or8(dyn, ninst, s1, s3, s3, s4);
        return;
    }
    IFX(X_PEND) {
        SET_DF(s4, d_or8);
    } else {
        SET_DFNONE();
    }
    ORRw_mask(s1, s1, mask&0x3F, (mask>>6)&0x3F);
    IFX(X_PEND) {
        STRB_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_CF | X_AF | X_OF) {
        MOV32w(s3, (1<<F_CF)|(1<<F_AF)|(1<<F_OF));
        BICw(xFlags, xFlags, s3);
    }
    COMP_ZFSF(s1, 8)
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit XOR8 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch, s4 can be same as s2 (and so s2 destroyed)
void emit_xor8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    MAYUSE(s2);
    IFX(X_PEND) {
        SET_DF(s4, d_xor8);
    } else {
        SET_DFNONE();
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
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit XOR8 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_xor8c(dynarec_arm_t* dyn, int ninst, int s1, uint8_t c, int s3, int s4)
{
    int mask = convert_bitmask_w(c);
    if(!mask) {
        MOV32w(s3, c&0xff);
        emit_xor8(dyn, ninst, s1, s3, s3, s4);
        return;
    }
    IFX(X_PEND) {
        SET_DF(s4, d_xor8);
    } else {
        SET_DFNONE();
    }
    EORw_mask(s1, s1, mask&0x3F, (mask>>6)&0x3F);
    IFX(X_PEND) {
        STRB_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_CF | X_AF | X_OF) {
        MOV32w(s3, (1<<F_CF)|(1<<F_AF)|(1<<F_OF));
        BICw(xFlags, xFlags, s3);
    }
    COMP_ZFSF(s1, 8)
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit AND8 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch, s4 can be same as s2 (and so s2 destroyed)
void emit_and8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    MAYUSE(s2);
    IFX(X_PEND) {
        SET_DF(s4, d_and8);
    } else {
        SET_DFNONE();
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
    if(arm64_flagm) {
        COMP_ZFSF(s1, 8)
    } else {
        IFX(X_ZF) {
            IFNATIVE(NF_EQ) {} else {
                CSETw(s3, cEQ);
                BFIw(xFlags, s3, F_ZF, 1);
            }
        }
        IFX(X_SF) {
            LSRw(s3, s1, 7);
            BFIw(xFlags, s3, F_SF, 1);
        }
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit AND8 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_and8c(dynarec_arm_t* dyn, int ninst, int s1, uint8_t c, int s3, int s4)
{
    int mask = convert_bitmask_w(c);
    if(!mask) {
        MOV32w(s3, c);
        emit_and8(dyn, ninst, s1, s3, s3, s4);
        return;
    }
    IFX(X_PEND) {
        SET_DF(s4, d_and8);
    } else {
        SET_DFNONE();
    }
    IFX(X_ZF) {
        ANDSw_mask(s1, s1, mask&0x3F, (mask>>6)&0x3F);
    } else {
        ANDw_mask(s1, s1, mask&0x3F, (mask>>6)&0x3F);
    }
    IFX(X_PEND) {
        STRB_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_CF | X_AF | X_OF) {
        MOV32w(s3, (1<<F_CF)|(1<<F_AF)|(1<<F_OF));
        BICw(xFlags, xFlags, s3);
    }
    if(arm64_flagm) {
        COMP_ZFSF(s1, 8)
    } else {
        IFX(X_ZF) {
            IFNATIVE(NF_EQ) {} else {
                CSETw(s3, cEQ);
                BFIw(xFlags, s3, F_ZF, 1);
            }
        }
        IFX(X_SF) {
            LSRw(s3, s1, 7);
            BFIw(xFlags, s3, F_SF, 1);
        }
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit OR16 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch, s4 can be same as s2 (and so s2 destroyed)
void emit_or16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    MAYUSE(s2);
    IFX(X_PEND) {
        SET_DF(s4, d_or16);
    } else {
        SET_DFNONE();
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
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit OR16 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_or16c(dynarec_arm_t* dyn, int ninst, int s1, int16_t c, int s3, int s4)
{
    int mask = convert_bitmask_w((uint16_t)c);
    if(!mask) {
        MOV32w(s3, (uint16_t)c);
        emit_or16(dyn, ninst, s1, s3, s3, s4);
        return;
    }
    IFX(X_PEND) {
        SET_DF(s4, d_or16);
    } else {
        SET_DFNONE();
    }
    ORRw_mask(s1, s1, mask&0x3F, (mask>>6)&0x3F);
    IFX(X_PEND) {
        STRH_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_CF | X_AF | X_OF) {
        MOV32w(s3, (1<<F_CF)|(1<<F_AF)|(1<<F_OF));
        BICw(xFlags, xFlags, s3);
    }
    COMP_ZFSF(s1, 16)
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit XOR16 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch, s4 can be same as s2 (and so s2 destroyed)
void emit_xor16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    MAYUSE(s2);
    IFX(X_PEND) {
        SET_DF(s4, d_xor16);
    } else {
        SET_DFNONE();
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
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit XOR16 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_xor16c(dynarec_arm_t* dyn, int ninst, int s1, int16_t c, int s3, int s4)
{
    int mask = convert_bitmask_w((uint16_t)c);
    if(!mask) {
        MOV32w(s3, (uint16_t)c);
        emit_xor16(dyn, ninst, s1, s3, s3, s4);
        return;
    }
    IFX(X_PEND) {
        SET_DF(s4, d_xor16);
    } else {
        SET_DFNONE();
    }
    EORw_mask(s1, s1, mask&0x3F, (mask>>6)&0x3F);
    IFX(X_PEND) {
        STRH_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_CF | X_AF | X_OF) {
        MOV32w(s3, (1<<F_CF)|(1<<F_AF)|(1<<F_OF));
        BICw(xFlags, xFlags, s3);
    }
    COMP_ZFSF(s1, 16)
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}


// emit AND16 instruction, from s1, s2, store result in s1 using s3 and s4 as scratch, s4 can be same as s2 (and so s2 destroyed)
void emit_and16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    MAYUSE(s2);
    IFX(X_PEND) {
        SET_DF(s4, d_and16);
    } else {
        SET_DFNONE();
    }
    IFX(X_ZF) {
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
    if(arm64_flagm) {
        COMP_ZFSF(s1, 16)
    } else {
        IFX(X_ZF) {
            IFNATIVE(NF_EQ) {} else {
                CSETw(s3, cEQ);
                BFIw(xFlags, s3, F_ZF, 1);
            }
        }
        IFX(X_SF) {
            LSRw(s3, s1, 15);
            BFIw(xFlags, s3, F_SF, 1);
        }
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit AND16 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_and16c(dynarec_arm_t* dyn, int ninst, int s1, int16_t c, int s3, int s4)
{
    int mask = convert_bitmask_w((uint16_t)c);
    if(!mask) {
        MOV32w(s3, (uint16_t)c);
        emit_and16(dyn, ninst, s1, s3, s3, s4);
        return;
    }
    IFX(X_PEND) {
        SET_DF(s4, d_and16);
    } else {
        SET_DFNONE();
    }
    IFX(X_ZF) {
        ANDSw_mask(s1, s1, mask&0x3F, (mask>>6)&0x3F);
    } else {
        ANDw_mask(s1, s1, mask&0x3F, (mask>>6)&0x3F);
    }
    IFX(X_PEND) {
        STRH_U12(s1, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_CF | X_AF | X_OF) {
        MOV32w(s3, (1<<F_CF)|(1<<F_AF)|(1<<F_OF));
        BICw(xFlags, xFlags, s3);
    }
    if(arm64_flagm) {
        COMP_ZFSF(s1, 16)
    } else {
        IFX(X_ZF) {
            IFNATIVE(NF_EQ) {} else {
                CSETw(s3, cEQ);
                BFIw(xFlags, s3, F_ZF, 1);
            }
        }
        IFX(X_SF) {
            LSRw(s3, s1, 15);
            BFIw(xFlags, s3, F_SF, 1);
        }
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}
