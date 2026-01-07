#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <string.h>

#include "os.h"
#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "emu/x64run_private.h"
#include "x64emu.h"
#include "box64stack.h"
#include "x64trace.h"
#include "dynablock.h"
#include "dynarec_native.h"
#include "../dynablock_private.h"
#include "custommem.h"
#include "x64test.h"
#include "pe_tools.h"

#include "../dynarec_arch.h"

#if STEP == 0
    #define EMIT(A)         dyn->native_size+=4
    #define SETMARK(A)      jmp_df[A] = 1
#elif STEP == 1
    #define EMIT(A)         do {} while (0)
    #define SETMARK(A)      jmp_df[A] = 0
#elif STEP == 2
    #define EMIT(A)         dyn->native_size+=4
    #define SETMARK(A)      jmp_df[A] = dyn->native_size
#elif STEP == 3
    #define MESSAGE(A, ...)                                                   \
        do {                                                                  \
            if (dyn->need_dump) dynarec_log_prefix(0, LOG_NONE, __VA_ARGS__); \
        } while (0)
    #define EMIT(A)                                         \
        do{                                                 \
            if(dyn->need_dump) print_opcode(dyn, ninst, (uint32_t)(A)); \
            *(uint32_t*)(dyn->block) = (uint32_t)(A);       \
            dyn->block += 4; dyn->native_size += 4;         \
            dyn->insts[ninst].size2 += 4;                   \
        }while(0)
    #define SETMARK(A)      MESSAGE(LOG_DUMP, "Mark(%d)=%p\n", A, dyn->block)
#else
#error Meh!
#endif
#define STEP_PASS
#include "../dynarec_helper.h"

/*
    Will generate a dynablock that does UpdateFlags. x0 = x64emu_t, x1 = df
    So read the current df, set df to None, and jump to the correct function
    using a static jump table.
    Only x1..x5 regs will be used. No saving of SIMD regs needed.
    LR will be used for return, and x0 needs to be corrctly setup as xEmu
    flags will be changed
*/

void updateflags_pass(dynarec_arm_t* dyn, uint64_t jmp_df[])
{
    int ninst = 0;
    rex_t rex = {0};
    LDRw_U12(x1, xEmu, offsetof(x64emu_t, df));
    STRw_U12(xZR, xEmu, offsetof(x64emu_t, df));
    CMPSw_U12(x1, d_unknown);
    Bcond(cLT, 4+4);
    RET(xLR);
    ADR_S20(x2, 4+8);
    ADDx_REG_LSL(x1, x2, x1, 2);
    BR(x1);
    for(int i=d_none; i<d_unknown; ++i)
        B(jmp_df[i] - dyn->native_size);
SETMARK(d_none);
    RET(xLR);
SETMARK(d_add8);
SETMARK(d_add8b);
    LDRB_U12(x1, xEmu, offsetof(x64emu_t, op1));
    LDRB_U12(x2, xEmu, offsetof(x64emu_t, op2));
    emit_add8(dyn, ninst, x1, x2, x3, x4);
    RET(xLR);
SETMARK(d_add16);
SETMARK(d_add16b);
    LDRH_U12(x1, xEmu, offsetof(x64emu_t, op1));
    LDRH_U12(x2, xEmu, offsetof(x64emu_t, op2));
    emit_add16(dyn, ninst, x1, x2, x3, x4);
    RET(xLR);
SETMARK(d_add32);
SETMARK(d_add32b);
    LDRw_U12(x1, xEmu, offsetof(x64emu_t, op1));
    LDRw_U12(x2, xEmu, offsetof(x64emu_t, op2));
    rex.w = 0;
    emit_add32(dyn, ninst, rex, x1, x2, x3, x4);
    RET(xLR);
SETMARK(d_add64);
    LDRx_U12(x1, xEmu, offsetof(x64emu_t, op1));
    LDRx_U12(x2, xEmu, offsetof(x64emu_t, op2));
    rex.w = 1;
    emit_add32(dyn, ninst, rex, x1, x2, x3, x4);
    rex.w = 0;
    RET(xLR);
SETMARK(d_and8);
    LDRB_U12(x1, xEmu, offsetof(x64emu_t, res));
    MOV32w(x2, 0xff);
    emit_and8(dyn, ninst, x1, x2, x3, x4);
    RET(xLR);
SETMARK(d_and16);
    LDRH_U12(x1, xEmu, offsetof(x64emu_t, res));
    MOV32w(x2, 0xffff);
    emit_and16(dyn, ninst, x1, x2, x3, x4);
    RET(xLR);
SETMARK(d_and32);
    LDRw_U12(x1, xEmu, offsetof(x64emu_t, res));
    MOV32w(x2, 0xffffffff);
    rex.w = 0;
    emit_and32(dyn, ninst, rex, x1, x2, x3, x4);
    RET(xLR);
SETMARK(d_and64);
    LDRx_U12(x1, xEmu, offsetof(x64emu_t, res));
    MOV64x(x2, 0xffffffffffffffffULL);
    rex.w = 1;
    emit_and32(dyn, ninst, rex, x1, x2, x3, x4);
    rex.w = 0;
    RET(xLR);
SETMARK(d_dec8);
    LDRB_U12(x1, xEmu, offsetof(x64emu_t, op1));
    emit_dec8(dyn, ninst, x1, x3, x4);
    RET(xLR);
SETMARK(d_dec16);
    LDRH_U12(x1, xEmu, offsetof(x64emu_t, op1));
    emit_dec16(dyn, ninst, x1, x3, x4);
    RET(xLR);
SETMARK(d_dec32);
    LDRw_U12(x1, xEmu, offsetof(x64emu_t, op1));
    rex.w = 0;
    emit_dec32(dyn, ninst, rex, x1, x3, x4);
    RET(xLR);
SETMARK(d_dec64);
    LDRx_U12(x1, xEmu, offsetof(x64emu_t, op1));
    rex.w = 1;
    emit_dec32(dyn, ninst, rex, x1, x3, x4);
    rex.w = 0;
    RET(xLR);
SETMARK(d_inc8);
    LDRB_U12(x1, xEmu, offsetof(x64emu_t, op1));
    emit_inc8(dyn, ninst, x1, x3, x4);
    RET(xLR);
SETMARK(d_inc16);
    LDRH_U12(x1, xEmu, offsetof(x64emu_t, op1));
    emit_inc16(dyn, ninst, x1, x3, x4);
    RET(xLR);
SETMARK(d_inc32);
    LDRw_U12(x1, xEmu, offsetof(x64emu_t, op1));
    rex.w = 0;
    emit_inc32(dyn, ninst, rex, x1, x3, x4);
    RET(xLR);
SETMARK(d_inc64);
    LDRx_U12(x1, xEmu, offsetof(x64emu_t, op1));
    rex.w = 1;
    emit_inc32(dyn, ninst, rex, x1, x3, x4);
    rex.w = 0;
    RET(xLR);
SETMARK(d_imul8);
    LDRSH_U12(x1, xEmu, offsetof(x64emu_t, res));
    ASRxw(x2, x1, 8);
    CMPSw_REG_ASR(x2, x1, 16);
    CSETw(x3, cNE);
    BFIw(xFlags, x3, F_CF, 1);
    BFIw(xFlags, x3, F_OF, 1);
    if(!BOX64ENV(cputype)) {
        LSRw(x2, x1, 7);
        BFIw(xFlags, x2, F_SF, 1);
        BFCw(xFlags, F_ZF, 1);
        BFCw(xFlags, F_AF, 1);
        emit_pf(dyn, ninst, x1, x4);
    }
    RET(xLR);
SETMARK(d_imul16);
    LDRw_U12(x1, xEmu, offsetof(x64emu_t, res));
    ASRw(x2, x1, 16);
    CMPSw_REG_ASR(x2, x1, 31);
    CSETw(x3, cNE);
    BFIw(xFlags, x3, F_CF, 1);
    BFIw(xFlags, x3, F_OF, 1);
    if(!BOX64ENV(cputype)) {
        LSRw(x2, x1, 15);
        BFIw(xFlags, x2, F_SF, 1);
        BFCw(xFlags, F_ZF, 1);
        BFCw(xFlags, F_AF, 1);
        emit_pf(dyn, ninst, x1, x4);
    }
    RET(xLR);
SETMARK(d_imul32);
    LDRw_U12(x1, xEmu, offsetof(x64emu_t, res));
    LDRw_U12(x2, xEmu, offsetof(x64emu_t, op1));
    CMPSw_REG_ASR(x2, x1, 31);
    CSETw(x3, cNE);
    BFIw(xFlags, x3, F_CF, 1);
    BFIw(xFlags, x3, F_OF, 1);
    if(!BOX64ENV(cputype)) {
        LSRw(x2, x1, 31);
        BFIw(xFlags, x2, F_SF, 1);
        BFCw(xFlags, F_ZF, 1);
        BFCw(xFlags, F_AF, 1);
        emit_pf(dyn, ninst, x1, x4);
    }
    RET(xLR);
SETMARK(d_imul64);
    LDRx_U12(x1, xEmu, offsetof(x64emu_t, res));
    LDRx_U12(x2, xEmu, offsetof(x64emu_t, op1));
    CMPSx_REG_ASR(x2, x1, 63);
    CSETw(x3, cNE);
    BFIw(xFlags, x3, F_CF, 1);
    BFIw(xFlags, x3, F_OF, 1);
    if(!BOX64ENV(cputype)) {
        LSRx(x2, x1, 63);
        BFIw(xFlags, x2, F_SF, 1);
        BFCw(xFlags, F_ZF, 1);
        BFCw(xFlags, F_AF, 1);
        emit_pf(dyn, ninst, x1, x4);
    }
    RET(xLR);
SETMARK(d_or8);
    LDRB_U12(x1, xEmu, offsetof(x64emu_t, res));
    MOV32w(x2, 0);
    emit_or8(dyn, ninst, x1, x2, x3, x4);
    RET(xLR);
SETMARK(d_or16);
    LDRH_U12(x1, xEmu, offsetof(x64emu_t, res));
    MOV32w(x2, 0);
    emit_or16(dyn, ninst, x1, x2, x3, x4);
    RET(xLR);
SETMARK(d_or32);
    LDRw_U12(x1, xEmu, offsetof(x64emu_t, res));
    MOV32w(x2, 0);
    rex.w = 0;
    emit_or32(dyn, ninst, rex, x1, x2, x3, x4);
    RET(xLR);
SETMARK(d_or64);
    LDRx_U12(x1, xEmu, offsetof(x64emu_t, res));
    MOV64x(x2, 0);
    rex.w = 1;
    emit_or32(dyn, ninst, rex, x1, x2, x3, x4);
    rex.w = 0;
    RET(xLR);
SETMARK(d_mul8);
    LDRH_U12(x1, xEmu, offsetof(x64emu_t, res));
    CMPSw_REG_LSR(xZR, x1, 8);
    CSETw(x3, cNE);
    BFIw(xFlags, x3, F_CF, 1);
    BFIw(xFlags, x3, F_OF, 1);
    if(!BOX64ENV(cputype)) {
        LSRw(x2, x1, 7);
        BFIw(xFlags, x2, F_SF, 1);
        BFCw(xFlags, F_ZF, 1);
        BFCw(xFlags, F_AF, 1);
        emit_pf(dyn, ninst, x1, x4);
    }
    RET(xLR);
SETMARK(d_mul16);
    LDRw_U12(x1, xEmu, offsetof(x64emu_t, res));
    CMPSw_REG_LSR(xZR, x1, 16);
    CSETw(x3, cNE);
    BFIw(xFlags, x3, F_CF, 1);
    BFIw(xFlags, x3, F_OF, 1);
    if(!BOX64ENV(cputype)) {
        LSRw(x2, x1, 15);
        BFIw(xFlags, x2, F_SF, 1);
        BFCw(xFlags, F_ZF, 1);
        BFCw(xFlags, F_AF, 1);
        emit_pf(dyn, ninst, x1, x4);
    }
    RET(xLR);
SETMARK(d_mul32);
    LDRw_U12(x2, xEmu, offsetof(x64emu_t, op1));
    CMPSw_U12(x2, 0);
    CSETw(x3, cNE);
    BFIw(xFlags, x3, F_CF, 1);
    BFIw(xFlags, x3, F_OF, 1);
    if(!BOX64ENV(cputype)) {
        LDRw_U12(x1, xEmu, offsetof(x64emu_t, res));
        LSRw(x2, x1, 31);
        BFIw(xFlags, x2, F_SF, 1);
        BFCw(xFlags, F_ZF, 1);
        BFCw(xFlags, F_AF, 1);
        emit_pf(dyn, ninst, x1, x4);
    }
    RET(xLR);
SETMARK(d_mul64);
    LDRx_U12(x2, xEmu, offsetof(x64emu_t, op1));
    CMPSx_U12(x2, 0);
    CSETw(x3, cNE);
    BFIw(xFlags, x3, F_CF, 1);
    BFIw(xFlags, x3, F_OF, 1);
    if(!BOX64ENV(cputype)) {
        LDRx_U12(x1, xEmu, offsetof(x64emu_t, res));
        LSRx(x2, x1, 63);
        BFIw(xFlags, x2, F_SF, 1);
        BFCw(xFlags, F_ZF, 1);
        BFCw(xFlags, F_AF, 1);
        emit_pf(dyn, ninst, x1, x4);
    }
    RET(xLR);
SETMARK(d_neg8);
    LDRB_U12(x1, xEmu, offsetof(x64emu_t, op1));
    emit_neg8(dyn, ninst, x1, x3, x4);
    RET(xLR);
SETMARK(d_neg16);
    LDRH_U12(x1, xEmu, offsetof(x64emu_t, op1));
    emit_neg16(dyn, ninst, x1, x3, x4);
    RET(xLR);
SETMARK(d_neg32);
    LDRw_U12(x1, xEmu, offsetof(x64emu_t, op1));
    rex.w = 0;
    emit_neg32(dyn, ninst, rex, x1, x3, x4);
    RET(xLR);
SETMARK(d_neg64);
    LDRx_U12(x1, xEmu, offsetof(x64emu_t, op1));
    rex.w = 1;
    emit_neg32(dyn, ninst, rex, x1, x3, x4);
    rex.w = 0;
    RET(xLR);
SETMARK(d_shl8);
    LDRB_U12(x1, xEmu, offsetof(x64emu_t, op1));
    LDRB_U12(x2, xEmu, offsetof(x64emu_t, op2));
    ANDSw_mask(x2, x2, 0, 4);
    Bcond(cNE, 4+4);
    RET(xLR);
    emit_shl8(dyn, ninst, x1, x2, x3, x4);
    RET(xLR);
SETMARK(d_shl16);
    LDRH_U12(x1, xEmu, offsetof(x64emu_t, op1));
    LDRH_U12(x2, xEmu, offsetof(x64emu_t, op2));
    ANDSw_mask(x2, x2, 0, 4);
    Bcond(cNE, 4+4);
    RET(xLR);
    emit_shl16(dyn, ninst, x1, x2, x3, x4);
    RET(xLR);
SETMARK(d_shl32);
    LDRw_U12(x1, xEmu, offsetof(x64emu_t, op1));
    LDRw_U12(x2, xEmu, offsetof(x64emu_t, op2));
    ANDSw_mask(x2, x2, 0, 4);
    Bcond(cNE, 4+4);
    RET(xLR);
    rex.w = 0;
    emit_shl32(dyn, ninst, rex, x1, x2, x3, x4);
    RET(xLR);
SETMARK(d_shl64);
    LDRx_U12(x1, xEmu, offsetof(x64emu_t, op1));
    LDRx_U12(x2, xEmu, offsetof(x64emu_t, op2));
    ANDSw_mask(x2, x2, 0, 5);
    Bcond(cNE, 4+4);
    RET(xLR);
    rex.w = 1;
    emit_shl32(dyn, ninst, rex, x1, x2, x3, x4);
    rex.w = 0;
    RET(xLR);
SETMARK(d_shr8);
    LDRB_U12(x1, xEmu, offsetof(x64emu_t, op1));
    LDRB_U12(x2, xEmu, offsetof(x64emu_t, op2));
    ANDSw_mask(x2, x2, 0, 4);
    Bcond(cNE, 4+4);
    RET(xLR);
    emit_shr8(dyn, ninst, x1, x2, x3, x4);
    RET(xLR);
SETMARK(d_shr16);
    LDRH_U12(x1, xEmu, offsetof(x64emu_t, op1));
    LDRH_U12(x2, xEmu, offsetof(x64emu_t, op2));
    ANDSw_mask(x2, x2, 0, 4);
    Bcond(cNE, 4+4);
    RET(xLR);
    emit_shr16(dyn, ninst, x1, x2, x3, x4);
    RET(xLR);
SETMARK(d_shr32);
    LDRw_U12(x1, xEmu, offsetof(x64emu_t, op1));
    LDRw_U12(x2, xEmu, offsetof(x64emu_t, op2));
    ANDSw_mask(x2, x2, 0, 4);
    Bcond(cNE, 4+4);
    RET(xLR);
    rex.w = 0;
    emit_shr32(dyn, ninst, rex, x1, x2, x3, x4);
    RET(xLR);
SETMARK(d_shr64);
    LDRx_U12(x1, xEmu, offsetof(x64emu_t, op1));
    LDRx_U12(x2, xEmu, offsetof(x64emu_t, op2));
    ANDSw_mask(x2, x2, 0, 5);
    Bcond(cNE, 4+4);
    RET(xLR);
    rex.w = 1;
    emit_shr32(dyn, ninst, rex, x1, x2, x3, x4);
    rex.w = 0;
    RET(xLR);
SETMARK(d_sar8);
    LDRSB_U12(x1, xEmu, offsetof(x64emu_t, op1));
    LDRB_U12(x2, xEmu, offsetof(x64emu_t, op2));
    ANDSw_mask(x2, x2, 0, 4);
    Bcond(cNE, 4+4);
    RET(xLR);
    emit_sar8(dyn, ninst, x1, x2, x3, x4);
    RET(xLR);
SETMARK(d_sar16);
    LDRSH_U12(x1, xEmu, offsetof(x64emu_t, op1));
    LDRH_U12(x2, xEmu, offsetof(x64emu_t, op2));
    ANDSw_mask(x2, x2, 0, 4);
    Bcond(cNE, 4+4);
    RET(xLR);
    emit_sar16(dyn, ninst, x1, x2, x3, x4);
    RET(xLR);
SETMARK(d_sar32);
    LDRw_U12(x1, xEmu, offsetof(x64emu_t, op1));
    LDRw_U12(x2, xEmu, offsetof(x64emu_t, op2));
    ANDSw_mask(x2, x2, 0, 4);
    Bcond(cNE, 4+4);
    RET(xLR);
    rex.w = 0;
    emit_sar32(dyn, ninst, rex, x1, x2, x3, x4);
    RET(xLR);
SETMARK(d_sar64);
    LDRx_U12(x1, xEmu, offsetof(x64emu_t, op1));
    LDRx_U12(x2, xEmu, offsetof(x64emu_t, op2));
    ANDSw_mask(x2, x2, 0, 5);
    Bcond(cNE, 4+4);
    RET(xLR);
    rex.w = 1;
    emit_sar32(dyn, ninst, rex, x1, x2, x3, x4);
    rex.w = 0;
    RET(xLR);
SETMARK(d_sub8);
    LDRB_U12(x1, xEmu, offsetof(x64emu_t, op1));
    LDRB_U12(x2, xEmu, offsetof(x64emu_t, op2));
    emit_sub8(dyn, ninst, x1, x2, x3, x4);
    RET(xLR);
SETMARK(d_sub16);
    LDRH_U12(x1, xEmu, offsetof(x64emu_t, op1));
    LDRH_U12(x2, xEmu, offsetof(x64emu_t, op2));
    emit_sub16(dyn, ninst, x1, x2, x3, x4);
    RET(xLR);
SETMARK(d_sub32);
    LDRw_U12(x1, xEmu, offsetof(x64emu_t, op1));
    LDRw_U12(x2, xEmu, offsetof(x64emu_t, op2));
    rex.w = 0;
    emit_sub32(dyn, ninst, rex, x1, x2, x3, x4);
    RET(xLR);
SETMARK(d_sub64);
    LDRx_U12(x1, xEmu, offsetof(x64emu_t, op1));
    LDRx_U12(x2, xEmu, offsetof(x64emu_t, op2));
    rex.w = 1;
    emit_sub32(dyn, ninst, rex, x1, x2, x3, x4);
    rex.w = 0;
    RET(xLR);
SETMARK(d_xor8);
    LDRB_U12(x1, xEmu, offsetof(x64emu_t, res));
    MOV32w(x2, 0);
    emit_xor8(dyn, ninst, x1, x2, x3, x4);
    RET(xLR);
SETMARK(d_xor16);
    LDRH_U12(x1, xEmu, offsetof(x64emu_t, res));
    MOV32w(x2, 0);
    emit_xor16(dyn, ninst, x1, x2, x3, x4);
    RET(xLR);
SETMARK(d_xor32);
    LDRw_U12(x1, xEmu, offsetof(x64emu_t, res));
    MOV32w(x2, 0);
    rex.w = 0;
    emit_xor32(dyn, ninst, rex, x1, x2, x3, x4);
    RET(xLR);
SETMARK(d_xor64);
    LDRx_U12(x1, xEmu, offsetof(x64emu_t, res));
    MOV64x(x2, 0);
    rex.w = 1;
    emit_xor32(dyn, ninst, rex, x1, x2, x3, x4);
    rex.w = 0;
    RET(xLR);
SETMARK(d_cmp8);
    LDRB_U12(x1, xEmu, offsetof(x64emu_t, op1));
    LDRB_U12(x2, xEmu, offsetof(x64emu_t, op2));
    emit_cmp8(dyn, ninst, x1, x2, x3, x4, x5);
    RET(xLR);
SETMARK(d_cmp16);
    LDRH_U12(x1, xEmu, offsetof(x64emu_t, op1));
    LDRH_U12(x2, xEmu, offsetof(x64emu_t, op2));
    emit_cmp16(dyn, ninst, x1, x2, x3, x4, x5);
    RET(xLR);
SETMARK(d_cmp32);
    LDRw_U12(x1, xEmu, offsetof(x64emu_t, op1));
    LDRw_U12(x2, xEmu, offsetof(x64emu_t, op2));
    rex.w = 0;
    emit_cmp32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    RET(xLR);
SETMARK(d_cmp64);
    LDRx_U12(x1, xEmu, offsetof(x64emu_t, op1));
    LDRx_U12(x2, xEmu, offsetof(x64emu_t, op2));
    rex.w = 1;
    emit_cmp32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    rex.w = 0;
    RET(xLR);
SETMARK(d_tst8);
    LDRB_U12(x1, xEmu, offsetof(x64emu_t, res));
    MOV32w(x2, 0xff);
    emit_test8(dyn, ninst, x1, x2, x3, x4, x5);
    RET(xLR);
SETMARK(d_tst16);
    LDRH_U12(x1, xEmu, offsetof(x64emu_t, res));
    MOV32w(x2, 0xffff);
    emit_test16(dyn, ninst, x1, x2, x3, x4, x5);
    RET(xLR);
SETMARK(d_tst32);
    LDRw_U12(x1, xEmu, offsetof(x64emu_t, res));
    MOV32w(x2, 0xffffffff);
    rex.w = 0;
    emit_test32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    RET(xLR);
SETMARK(d_tst64);
    LDRx_U12(x1, xEmu, offsetof(x64emu_t, res));
    MOV64x(x2, 0xffffffffffffffffULL);
    rex.w = 1;
    emit_test32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    rex.w = 0;
    RET(xLR);
// for ADC & SBB, the emit_adcX cannot be used because the CF state is not saved
SETMARK(d_adc8);
    LDRH_U12(x1, xEmu, offsetof(x64emu_t, res));
    BFXILw(xFlags, x1, 8, 1);   //F_CF
    LSRw(x2, x1, 7);
    BFIw(xFlags, x2, F_SF, 1);
    TSTw_mask(x1, 0, 7);    // mask 0xff
    CSETw(x2, cEQ);
    BFIw(xFlags, x2, F_ZF, 1);
    LDRB_U12(x2, xEmu, offsetof(x64emu_t, op1));
    LDRB_U12(x3, xEmu, offsetof(x64emu_t, op2));
    ANDw_REG(x4, x2, x3);   // op1 & op2
    ORRw_REG(x2, x2, x3);   // op1 | op2
    BICw_REG(x2, x2, x1);   // ~res & (op1 | op2)
    ORRw_REG(x2, x2, x4); // CC
    LSRw(x3, x2, 3);
    BFIw(xFlags, x3, F_AF, 1);
    LSRw(x3, x2, 6);
    EORw_REG_LSR(x3, x3, x3, 1);
    BFIw(xFlags, x3, F_OF, 1);
    emit_pf(dyn, ninst, x1, x4);
    RET(xLR);
SETMARK(d_adc8b);
    LDRB_U12(x1, xEmu, offsetof(x64emu_t, res));
    LDRB_U12(x2, xEmu, offsetof(x64emu_t, op1));
    LDRB_U12(x3, xEmu, offsetof(x64emu_t, op2));
    ADDw_REG(x4, x2, x3);
    SUBw_UXTB(x4, x1, x4);
    CBZw(x4, 4+4);
    MOVZw(x4, 1);
    ADDw_REG(x1, x2, x3);
    ADDw_REG(x1, x1, x4);   // recompute res as 16bits
    BFXILw(xFlags, x1, 8, 1);   //F_CF
    LSRw(x4, x1, 7);
    BFIw(xFlags, x4, F_SF, 1);
    TSTw_mask(x1, 0, 7);    // mask 0xff
    CSETw(x4, cEQ);
    BFIw(xFlags, x4, F_ZF, 1);
    ANDw_REG(x4, x2, x3);   // op1 & op2
    ORRw_REG(x2, x2, x3);   // op1 | op2
    BICw_REG(x2, x2, x1);   // ~res & (op1 | op2)
    ORRw_REG(x2, x2, x4); // CC
    LSRw(x3, x2, 3);
    BFIw(xFlags, x3, F_AF, 1);
    LSRw(x3, x2, 6);
    EORw_REG_LSR(x3, x3, x3, 1);
    BFIw(xFlags, x3, F_OF, 1);
    emit_pf(dyn, ninst, x1, x4);
    RET(xLR);
SETMARK(d_adc16);
    LDRw_U12(x1, xEmu, offsetof(x64emu_t, res));
    BFXILw(xFlags, x1, 16, 1);   //F_CF
    LSRw(x2, x1, 15);
    BFIw(xFlags, x2, F_SF, 1);
    TSTw_mask(x1, 0, 15);    // mask 0xffff
    CSETw(x2, cEQ);
    BFIw(xFlags, x2, F_ZF, 1);
    LDRH_U12(x2, xEmu, offsetof(x64emu_t, op1));
    LDRH_U12(x3, xEmu, offsetof(x64emu_t, op2));
    ANDw_REG(x4, x2, x3);   // op1 & op2
    ORRw_REG(x2, x2, x3);   // op1 | op2
    BICw_REG(x2, x2, x1);   // ~res & (op1 | op2)
    ORRw_REG(x2, x2, x4); // CC
    LSRw(x3, x2, 3);
    BFIw(xFlags, x3, F_AF, 1);
    LSRw(x3, x2, 14);
    EORw_REG_LSR(x3, x3, x3, 1);
    BFIw(xFlags, x3, F_OF, 1);
    emit_pf(dyn, ninst, x1, x4);
    RET(xLR);
SETMARK(d_adc16b);
    LDRH_U12(x1, xEmu, offsetof(x64emu_t, res));
    LDRH_U12(x2, xEmu, offsetof(x64emu_t, op1));
    LDRH_U12(x3, xEmu, offsetof(x64emu_t, op2));
    ADDw_REG(x4, x2, x3);
    SUBw_UXTH(x4, x1, x4);
    CBZw(x4, 4+4);
    MOVZw(x4, 1);
    ADDw_REG(x1, x2, x3);
    ADDw_REG(x1, x1, x4);   // recompute res as 16bits
    BFXILw(xFlags, x1, 16, 1);   //F_CF
    LSRw(x4, x1, 15);
    BFIw(xFlags, x4, F_SF, 1);
    TSTw_mask(x1, 0, 15);    // mask 0xffff
    CSETw(x4, cEQ);
    BFIw(xFlags, x4, F_ZF, 1);
    ANDw_REG(x4, x2, x3);   // op1 & op2
    ORRw_REG(x2, x2, x3);   // op1 | op2
    BICw_REG(x2, x2, x1);   // ~res & (op1 | op2)
    ORRw_REG(x2, x2, x4); // CC
    LSRw(x3, x2, 3);
    BFIw(xFlags, x3, F_AF, 1);
    LSRw(x3, x2, 14);
    EORw_REG_LSR(x3, x3, x3, 1);
    BFIw(xFlags, x3, F_OF, 1);
    emit_pf(dyn, ninst, x1, x4);
    RET(xLR);
SETMARK(d_adc32);
    LDRx_U12(x1, xEmu, offsetof(x64emu_t, res));
    BFXILx(xFlags, x1, 32, 1);   //F_CF
    LSRw(x2, x1, 31);
    BFIw(xFlags, x2, F_SF, 1);
    TSTw_REG(x1, x1);
    CSETw(x2, cEQ);
    BFIw(xFlags, x2, F_ZF, 1);
    LDRw_U12(x2, xEmu, offsetof(x64emu_t, op1));
    LDRw_U12(x3, xEmu, offsetof(x64emu_t, op2));
    ANDw_REG(x4, x2, x3);   // op1 & op2
    ORRw_REG(x2, x2, x3);   // op1 | op2
    BICw_REG(x2, x2, x1);   // ~res & (op1 | op2)
    ORRw_REG(x2, x2, x4); // CC
    LSRw(x3, x2, 3);
    BFIw(xFlags, x3, F_AF, 1);
    LSRw(x3, x2, 30);
    EORw_REG_LSR(x3, x3, x3, 1);
    BFIw(xFlags, x3, F_OF, 1);
    emit_pf(dyn, ninst, x1, x4);
    RET(xLR);
SETMARK(d_adc32b);
    LDRw_U12(x1, xEmu, offsetof(x64emu_t, res));
    LSRw(x2, x1, 31);
    BFIw(xFlags, x2, F_SF, 1);
    TSTw_REG(x1, x1);
    CSETw(x2, cEQ);
    BFIw(xFlags, x2, F_ZF, 1);
    LDRw_U12(x2, xEmu, offsetof(x64emu_t, op1));
    LDRw_U12(x3, xEmu, offsetof(x64emu_t, op2));
    ADDw_REG(x4, x2, x3);
    CMPSw_REG(x1, x4);
    CSETw(x4, cNE);
    ADDx_REG(x4, x4, x2);
    ADDx_REG(x4, x4, x3);
    BFXILx(xFlags, x4, 32, 1);   //F_CF
    ANDw_REG(x4, x2, x3);   // op1 & op2
    ORRw_REG(x2, x2, x3);   // op1 | op2
    BICw_REG(x2, x2, x1);   // ~res & (op1 | op2)
    ORRw_REG(x2, x2, x4); // CC
    LSRw(x3, x2, 3);
    BFIw(xFlags, x3, F_AF, 1);
    LSRw(x3, x2, 30);
    EORw_REG_LSR(x3, x3, x3, 1);
    BFIw(xFlags, x3, F_OF, 1);
    emit_pf(dyn, ninst, x1, x4);
    RET(xLR);
SETMARK(d_adc64);
    LDRx_U12(x1, xEmu, offsetof(x64emu_t, res));
    LSRx(x2, x1, 63);
    BFIw(xFlags, x2, F_SF, 1);
    TSTx_REG(x1, x1);
    CSETw(x2, cEQ);
    BFIw(xFlags, x2, F_ZF, 1);
    LDRx_U12(x2, xEmu, offsetof(x64emu_t, op1));
    LDRx_U12(x3, xEmu, offsetof(x64emu_t, op2));
    ADDx_REG(x4, x2, x3);
    CMPSx_REG(x1, x4);
    CSETw(x4, cNE);
    ADDx_UXTW(x4, x4, x2);
    ADDx_UXTW(x4, x4, x3);  // x4 = lo
    LSRx(x4, x4, 32);
    ADDx_REG_LSR(x4, x4, x2, 32);
    ADDx_REG_LSR(x4, x4, x3, 32);   // hi
    BFXILx(xFlags, x4, 32, 1);   //F_CF
    ANDx_REG(x4, x2, x3);   // op1 & op2
    ORRx_REG(x2, x2, x3);   // op1 | op2
    BICx_REG(x2, x2, x1);   // ~res & (op1 | op2)
    ORRx_REG(x2, x2, x4); // CC
    LSRx(x3, x2, 3);
    BFIx(xFlags, x3, F_AF, 1);
    LSRx(x3, x2, 62);
    EORw_REG_LSR(x3, x3, x3, 1);
    BFIw(xFlags, x3, F_OF, 1);
    emit_pf(dyn, ninst, x1, x4);
    RET(xLR);
SETMARK(d_sbb8);
    LDRB_U12(x1, xEmu, offsetof(x64emu_t, res));
    LSRw(x2, x1, 7);
    BFIw(xFlags, x2, F_SF, 1);
    TSTw_mask(x1, 0, 7);    // mask 0xff
    CSETw(x2, cEQ);
    BFIw(xFlags, x2, F_ZF, 1);
    LDRB_U12(x2, xEmu, offsetof(x64emu_t, op1));
    LDRB_U12(x3, xEmu, offsetof(x64emu_t, op2));
    BICw_REG(x4, x3, x2);   // ~op1 & op2
    ORNw_REG(x2, x3, x2);   // ~op1 | op2
    ANDw_REG(x2, x2, x1);   // res & (~op1 | op2)
    ORRw_REG(x2, x2, x4); // CC
    BFXILw(xFlags, x2, 7, 1);
    LSRw(x3, x2, 3);
    BFIw(xFlags, x3, F_AF, 1);
    LSRw(x3, x2, 6);
    EORw_REG_LSR(x3, x3, x3, 1);
    BFIw(xFlags, x3, F_OF, 1);
    emit_pf(dyn, ninst, x1, x4);
    RET(xLR);
SETMARK(d_sbb16);
    LDRH_U12(x1, xEmu, offsetof(x64emu_t, res));
    LSRw(x2, x1, 15);
    BFIw(xFlags, x2, F_SF, 1);
    TSTw_mask(x1, 0, 15);    // mask 0xffff
    CSETw(x2, cEQ);
    BFIw(xFlags, x2, F_ZF, 1);
    LDRH_U12(x2, xEmu, offsetof(x64emu_t, op1));
    LDRH_U12(x3, xEmu, offsetof(x64emu_t, op2));
    BICw_REG(x4, x3, x2);   // ~op1 & op2
    ORNw_REG(x2, x3, x2);   // ~op1 | op2
    ANDw_REG(x2, x2, x1);   // res & (~op1 | op2)
    ORRw_REG(x2, x2, x4); // CC
    BFXILw(xFlags, x2, 15, 1);
    LSRw(x3, x2, 3);
    BFIw(xFlags, x3, F_AF, 1);
    LSRw(x3, x2, 14);
    EORw_REG_LSR(x3, x3, x3, 1);
    BFIw(xFlags, x3, F_OF, 1);
    emit_pf(dyn, ninst, x1, x4);
    RET(xLR);
SETMARK(d_sbb32);
    LDRw_U12(x1, xEmu, offsetof(x64emu_t, res));
    LSRw(x2, x1, 31);
    BFIw(xFlags, x2, F_SF, 1);
    TSTw_REG(x1, x1);
    CSETw(x2, cEQ);
    BFIw(xFlags, x2, F_ZF, 1);
    LDRw_U12(x2, xEmu, offsetof(x64emu_t, op1));
    LDRw_U12(x3, xEmu, offsetof(x64emu_t, op2));
    BICw_REG(x4, x3, x2);   // ~op1 & op2
    ORNw_REG(x2, x3, x2);   // ~op1 | op2
    ANDw_REG(x2, x2, x1);   // res & (~op1 | op2)
    ORRw_REG(x2, x2, x4); // CC
    BFXILw(xFlags, x2, 31, 1);
    LSRw(x3, x2, 3);
    BFIw(xFlags, x3, F_AF, 1);
    LSRw(x3, x2, 30);
    EORw_REG_LSR(x3, x3, x3, 1);
    BFIw(xFlags, x3, F_OF, 1);
    emit_pf(dyn, ninst, x1, x4);
    RET(xLR);
SETMARK(d_sbb64);
    LDRx_U12(x1, xEmu, offsetof(x64emu_t, res));
    LSRx(x2, x1, 63);
    BFIw(xFlags, x2, F_SF, 1);
    TSTx_REG(x1, x1);
    CSETw(x2, cEQ);
    BFIw(xFlags, x2, F_ZF, 1);
    LDRx_U12(x2, xEmu, offsetof(x64emu_t, op1));
    LDRx_U12(x3, xEmu, offsetof(x64emu_t, op2));
    BICx_REG(x4, x3, x2);   // ~op1 & op2
    ORNx_REG(x2, x3, x2);   // ~op1 | op2
    ANDx_REG(x2, x2, x1);   // res & (~op1 | op2)
    ORRx_REG(x2, x2, x4); // CC
    BFXILx(xFlags, x2, 63, 1);
    LSRw(x3, x2, 3);
    BFIw(xFlags, x3, F_AF, 1);
    LSRx(x3, x2, 62);
    EORw_REG_LSR(x3, x3, x3, 1);
    BFIw(xFlags, x3, F_OF, 1);
    emit_pf(dyn, ninst, x1, x4);
    RET(xLR);
SETMARK(d_rol8);
    LDRB_U12(x1, xEmu, offsetof(x64emu_t, res));
    if(BOX64ENV(cputype)) {
        EORw_REG_LSR(x2, x1, x1, 7);
        BFIw(xFlags, x2, F_OF, 1);
    } else {
        LDRB_U12(x2, xEmu, offsetof(x64emu_t, op1));
        LSLw_IMM(x3, x2, 6);
        EORw_REG_LSR(x3, x3, x3, 1);
        BFIw(xFlags, x3, F_OF, 1);
    }
    BFXILw(xFlags, x1, 0, 1);
    RET(xLR);
SETMARK(d_rol16);
    LDRH_U12(x1, xEmu, offsetof(x64emu_t, res));
    if(BOX64ENV(cputype)) {
        EORw_REG_LSR(x2, x1, x1, 15);
        BFIw(xFlags, x2, F_OF, 1);
    } else {
        LDRH_U12(x2, xEmu, offsetof(x64emu_t, op1));
        LSLw_IMM(x3, x2, 14);
        EORw_REG_LSR(x3, x3, x3, 1);
        BFIw(xFlags, x3, F_OF, 1);
    }
    BFXILw(xFlags, x1, 0, 1);
    RET(xLR);
SETMARK(d_rol32);
    LDRw_U12(x1, xEmu, offsetof(x64emu_t, res));
    if(BOX64ENV(cputype)) {
        EORw_REG_LSR(x2, x1, x1, 31);
        BFIw(xFlags, x2, F_OF, 1);
    } else {
        LDRw_U12(x2, xEmu, offsetof(x64emu_t, op1));
        LSLw_IMM(x3, x2, 30);
        EORw_REG_LSR(x3, x3, x3, 1);
        BFIw(xFlags, x3, F_OF, 1);
    }
    BFXILw(xFlags, x1, 0, 1);
    RET(xLR);
SETMARK(d_rol64);
    LDRx_U12(x1, xEmu, offsetof(x64emu_t, res));
    if(BOX64ENV(cputype)) {
        EORx_REG_LSR(x2, x1, x1, 63);
        BFIw(xFlags, x2, F_OF, 1);
    } else {
        LDRx_U12(x2, xEmu, offsetof(x64emu_t, op1));
        LSLx_IMM(x3, x2, 62);
        EORw_REG_LSR(x3, x3, x3, 1);
        BFIw(xFlags, x3, F_OF, 1);
    }
    BFXILw(xFlags, x1, 0, 1);
    RET(xLR);
SETMARK(d_ror8);
    LDRB_U12(x1, xEmu, offsetof(x64emu_t, res));
    if(BOX64ENV(cputype)) {
        LSLw_IMM(x2, x1, 6);
        EORw_REG_LSR(x3, x2, x2, 1);
        BFIw(xFlags, x3, F_OF, 1);
    } else {
        LDRB_U12(x2, xEmu, offsetof(x64emu_t, op1));
        EORw_REG_LSR(x3, x2, x2, 7);
        BFIw(xFlags, x3, F_OF, 1);
    }
    BFXILw(xFlags, x1, 7, 1);
    RET(xLR);
SETMARK(d_ror16);
    LDRH_U12(x1, xEmu, offsetof(x64emu_t, res));
    if(BOX64ENV(cputype)) {
        LSLw_IMM(x2, x1, 14);
        EORw_REG_LSR(x3, x2, x2, 1);
        BFIw(xFlags, x3, F_OF, 1);
    } else {
        LDRH_U12(x2, xEmu, offsetof(x64emu_t, op1));
        EORw_REG_LSR(x3, x2, x2, 15);
        BFIw(xFlags, x3, F_OF, 1);
    }
    BFXILw(xFlags, x1, 15, 1);
    RET(xLR);
SETMARK(d_ror32);
    LDRw_U12(x1, xEmu, offsetof(x64emu_t, res));
    if(BOX64ENV(cputype)) {
        LSLw_IMM(x2, x1, 30);
        EORw_REG_LSR(x3, x2, x2, 1);
        BFIw(xFlags, x3, F_OF, 1);
    } else {
        LDRw_U12(x2, xEmu, offsetof(x64emu_t, op1));
        EORw_REG_LSR(x3, x2, x2, 31);
        BFIw(xFlags, x3, F_OF, 1);
    }
    BFXILw(xFlags, x1, 31, 1);
    RET(xLR);
SETMARK(d_ror64);
    LDRx_U12(x1, xEmu, offsetof(x64emu_t, res));
    if(BOX64ENV(cputype)) {
        LSLx_IMM(x2, x1, 62);
        EORw_REG_LSR(x3, x2, x2, 1);
        BFIw(xFlags, x3, F_OF, 1);
    } else {
        LDRx_U12(x2, xEmu, offsetof(x64emu_t, op1));
        EORx_REG_LSR(x3, x2, x2, 63);
        BFIw(xFlags, x3, F_OF, 1);
    }
    BFXILx(xFlags, x1, 63, 1);
    RET(xLR);
SETMARK(d_shrd16);
    LDRH_U12(x1, xEmu, offsetof(x64emu_t, res));
    LDRH_U12(x2, xEmu, offsetof(x64emu_t, op1));
    LDRH_U12(x3, xEmu, offsetof(x64emu_t, op2));
    if(BOX64ENV(cputype)) {
        LSRw(x4, x1, 14);
        EORw_REG_LSR(x4, x4, x4, 1);
        BFIw(xFlags, x4, F_OF, 1);
        ORRw_mask(xFlags, xFlags, 28, 0); // mask = 0x10
    } else {
        ANDw_mask(x4, x3, 0, 3);    // mask = 0x0f
        SUBw_U12(x4, x4, 16);
        NEGw_REG(x4, x4);
        LSRw_REG(x4, x1, x4);
        EORw_REG_LSR(x4, x4, x2, 15);
        BFIw(xFlags, x4, F_OF, 1);
        BFCw(xFlags, F_AF, 1);
    }
    CBZw(x3, 4+4);
    RET(xLR);
    SUBw_U12(x4, x3, 1);
    LSRw_REG(x4, x2, x4);
    if(BOX64ENV(cputype)) {
        CMPSw_U12(x3, 15);
        CSELw(x4, x4, xZR, cGT);
    }
    BFIw(xFlags, x4, F_CF, 1);
    LSRw(x4, x1, 15);
    BFIw(xFlags, x4, F_SF, 1);
    TSTw_mask(x1, 0, 15);
    CSETw(x4, cEQ);
    BFIw(xFlags, x4, F_ZF, 1);
    emit_pf(dyn, ninst, x1, x4);
    RET(xLR);
SETMARK(d_shrd32);
    LDRw_U12(x1, xEmu, offsetof(x64emu_t, res));
    LDRw_U12(x2, xEmu, offsetof(x64emu_t, op1));
    LDRw_U12(x3, xEmu, offsetof(x64emu_t, op2));
    CBNZw(x3, 4+4);
    RET(xLR);
     if(BOX64ENV(cputype)) {
        LSRw(x4, x1, 30);
        EORw_REG_LSR(x4, x4, x4, 1);
        BFIw(xFlags, x4, F_OF, 1);
        ORRw_mask(xFlags, xFlags, 28, 0); // mask = 0x10
    } else {
        SUBw_U12(x4, x3, 32);
        NEGw_REG(x4, x4);
        LSRw_REG(x4, x1, x4);
        EORw_REG_LSR(x4, x4, x2, 31);
        BFIw(xFlags, x4, F_OF, 1);
        BFCw(xFlags, F_AF, 1);
    }
    SUBw_U12(x4, x3, 1);
    LSRw_REG(x4, x2, x4);
    BFIw(xFlags, x4, F_CF, 1);
    LSRw(x4, x1, 31);
    BFIw(xFlags, x4, F_SF, 1);
    TSTw_REG(x1, x1);
    CSETw(x4, cEQ);
    BFIw(xFlags, x4, F_ZF, 1);
    emit_pf(dyn, ninst, x1, x4);
    RET(xLR);
SETMARK(d_shrd64);
    LDRx_U12(x1, xEmu, offsetof(x64emu_t, res));
    LDRx_U12(x2, xEmu, offsetof(x64emu_t, op1));
    LDRx_U12(x3, xEmu, offsetof(x64emu_t, op2));
    CBNZw(x3, 4+4);
    RET(xLR);
     if(BOX64ENV(cputype)) {
        LSRx(x4, x1, 62);
        EORw_REG_LSR(x4, x4, x4, 1);
        BFIw(xFlags, x4, F_OF, 1);
        ORRw_mask(xFlags, xFlags, 28, 0); // mask = 0x10
    } else {
        SUBw_U12(x4, x3, 64);
        NEGw_REG(x4, x4);
        LSRx_REG(x4, x1, x4);
        EORx_REG_LSR(x4, x4, x2, 63);
        BFIw(xFlags, x4, F_OF, 1);
        BFCw(xFlags, F_AF, 1);
    }
    SUBx_U12(x4, x3, 1);
    LSRx_REG(x4, x2, x4);
    BFIw(xFlags, x4, F_CF, 1);
    LSRx(x4, x1, 63);
    BFIw(xFlags, x4, F_SF, 1);
    TSTx_REG(x1, x1);
    CSETw(x4, cEQ);
    BFIw(xFlags, x4, F_ZF, 1);
    emit_pf(dyn, ninst, x1, x4);
    RET(xLR);
SETMARK(d_shld16);
    LDRH_U12(x1, xEmu, offsetof(x64emu_t, res));
    LDRH_U12(x2, xEmu, offsetof(x64emu_t, op1));
    LDRH_U12(x3, xEmu, offsetof(x64emu_t, op2));
    CBNZw(x3, 4+4);
    RET(xLR);
    SUBw_U12(x4, x3, 16);
    NEGw_REG(x4, x4);
    LSRw_REG(x4, x2, x4);
    BFIw(xFlags, x4, F_CF, 1);
    if(BOX64ENV(cputype)) {
        EORw_REG_LSR(x4, xFlags, x1, 15);
        CMPSw_U12(x3, 15);
        CSELw(x4, x4, xFlags, cGT);
        BFIw(xFlags, x4, F_OF, 1);
        ORRw_mask(xFlags, xFlags, 28, 0); // mask = 0x10
    } else {
        LSRw(x4, x2, 14);
        EORw_REG_LSR(x4, x4, x4, 1);
        BFIw(xFlags, x4, F_OF, 1);
        BFCw(xFlags, F_AF, 1);
    }
    LSRw(x4, x1, 15);
    BFIw(xFlags, x4, F_SF, 1);
    TSTw_mask(x1, 0, 15);
    CSETw(x4, cEQ);
    BFIw(xFlags, x4, F_ZF, 1);
    emit_pf(dyn, ninst, x1, x4);
    RET(xLR);
SETMARK(d_shld32);
    LDRw_U12(x1, xEmu, offsetof(x64emu_t, res));
    LDRw_U12(x2, xEmu, offsetof(x64emu_t, op1));
    LDRw_U12(x3, xEmu, offsetof(x64emu_t, op2));
    CBNZw(x3, 4+4);
    RET(xLR);
    SUBw_U12(x4, x3, 32);
    NEGw_REG(x4, x4);
    LSRw_REG(x4, x2, x4);
    BFIw(xFlags, x4, F_CF, 1);
    if(BOX64ENV(cputype)) {
        EORw_REG_LSR(x4, xFlags, x1, 31);
        BFIw(xFlags, x4, F_OF, 1);
        ORRw_mask(xFlags, xFlags, 28, 0); // mask = 0x10
    } else {
        LSRw(x4, x2, 30);
        EORw_REG_LSR(x4, x4, x4, 1);
        BFIw(xFlags, x4, F_OF, 1);
        BFCw(xFlags, F_AF, 1);
    }
    LSRw(x4, x1, 31);
    BFIw(xFlags, x4, F_SF, 1);
    TSTw_REG(x1, x1);
    CSETw(x4, cEQ);
    BFIw(xFlags, x4, F_ZF, 1);
    emit_pf(dyn, ninst, x1, x4);
    RET(xLR);
SETMARK(d_shld64);
    LDRx_U12(x1, xEmu, offsetof(x64emu_t, res));
    LDRx_U12(x2, xEmu, offsetof(x64emu_t, op1));
    LDRx_U12(x3, xEmu, offsetof(x64emu_t, op2));
    CBNZx(x3, 4+4);
    RET(xLR);
    MOV32w(x4, 64);
    SUBw_REG(x4, x4, x3);
    LSRx_REG(x4, x2, x4);
    BFIw(xFlags, x4, F_CF, 1);
    if(BOX64ENV(cputype)) {
        EORx_REG_LSR(x4, xFlags, x1, 63);
        BFIw(xFlags, x4, F_OF, 1);
        ORRw_mask(xFlags, xFlags, 28, 0); // mask = 0x10
    } else {
        LSRx(x4, x2, 62);
        EORw_REG_LSR(x4, x4, x4, 1);
        BFIw(xFlags, x4, F_OF, 1);
        BFCw(xFlags, F_AF, 1);
    }
    LSRx(x4, x1, 63);
    BFIw(xFlags, x4, F_SF, 1);
    TSTx_REG(x1, x1);
    CSETw(x4, cEQ);
    BFIw(xFlags, x4, F_ZF, 1);
    emit_pf(dyn, ninst, x1, x4);
    RET(xLR);
// all done!
}