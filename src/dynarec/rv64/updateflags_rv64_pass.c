// SPDX-License-Identifier: MIT
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
#define SETMARK(A) jmp_df[A] = 1
#elif STEP == 1
#define SETMARK(A) jmp_df[A] = 0
#elif STEP == 2
#define SETMARK(A) jmp_df[A] = dyn->native_size
#elif STEP == 3
#define SETMARK(A) MESSAGE(LOG_DUMP, "Mark(%d)=%p\n", A, dyn->block)
#else
#error Meh!
#endif

#define STEP_PASS
#include "../dynarec_helper.h"

void updateflags_pass(dynarec_rv64_t* dyn, uint64_t jmp_df[])
{
    int ninst = 0;
    rex_t rex = { 0 };

    LWU(x1, xEmu, offsetof(x64emu_t, df));
    SW(xZR, xEmu, offsetof(x64emu_t, df));
    SLTIU(x2, x1, d_unknown);
    BNEZ(x2, 8); // df < d_unknown? skip the return and dispatch
    BR(xRA);
    SLLI(x1, x1, 2);
    AUIPC(x2, 0);
    ADDI(x2, x2, 4 * 4); // offset of the branch table from the AUIPC
    ADD(x1, x1, x2);
    BR(x1);
    for (int i = d_none; i < d_unknown; ++i)
        B(jmp_df[i] - dyn->native_size);

SETMARK(d_none);
    BR(xRA);

SETMARK(d_add8);
SETMARK(d_add8b);
    LBU(x1, xEmu, offsetof(x64emu_t, op1));
    LBU(x2, xEmu, offsetof(x64emu_t, op2));
    emit_add8(dyn, ninst, x1, x2, x3, x4, x5);
    BR(xRA);

SETMARK(d_add16);
SETMARK(d_add16b);
    LHU(x1, xEmu, offsetof(x64emu_t, op1));
    LHU(x2, xEmu, offsetof(x64emu_t, op2));
    emit_add16(dyn, ninst, x1, x2, x3, x4, x5);
    BR(xRA);

SETMARK(d_add32);
SETMARK(d_add32b);
    LWU(x1, xEmu, offsetof(x64emu_t, op1));
    LWU(x2, xEmu, offsetof(x64emu_t, op2));
    rex.w = 0;
    emit_add32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    BR(xRA);

SETMARK(d_add64);
    LD(x1, xEmu, offsetof(x64emu_t, op1));
    LD(x2, xEmu, offsetof(x64emu_t, op2));
    rex.w = 1;
    emit_add32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    rex.w = 0;
    BR(xRA);

SETMARK(d_and8);
    LBU(x1, xEmu, offsetof(x64emu_t, res));
    ADDI(x2, xZR, 0xff);
    emit_and8(dyn, ninst, x1, x2, x3, x4);
    BR(xRA);

SETMARK(d_and16);
    LHU(x1, xEmu, offsetof(x64emu_t, res));
    MOV32w(x2, 0xffff);
    emit_and16(dyn, ninst, x1, x2, x3, x4);
    BR(xRA);

SETMARK(d_and32);
    LWU(x1, xEmu, offsetof(x64emu_t, res));
    MOV32w(x2, 0xffffffff);
    rex.w = 0;
    emit_and32(dyn, ninst, rex, x1, x2, x3, x4);
    BR(xRA);

SETMARK(d_and64);
    LD(x1, xEmu, offsetof(x64emu_t, res));
    ADDI(x2, xZR, -1);
    rex.w = 1;
    emit_and32(dyn, ninst, rex, x1, x2, x3, x4);
    rex.w = 0;
    BR(xRA);

SETMARK(d_dec8);
    LBU(x1, xEmu, offsetof(x64emu_t, op1));
    emit_dec8(dyn, ninst, x1, x2, x3, x4);
    BR(xRA);

SETMARK(d_dec16);
    LHU(x1, xEmu, offsetof(x64emu_t, op1));
    emit_dec16(dyn, ninst, x1, x2, x3, x4, x5);
    BR(xRA);

SETMARK(d_dec32);
    LWU(x1, xEmu, offsetof(x64emu_t, op1));
    rex.w = 0;
    emit_dec32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    BR(xRA);

SETMARK(d_dec64);
    LD(x1, xEmu, offsetof(x64emu_t, op1));
    rex.w = 1;
    emit_dec32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    rex.w = 0;
    BR(xRA);

SETMARK(d_inc8);
    LBU(x1, xEmu, offsetof(x64emu_t, op1));
    emit_inc8(dyn, ninst, x1, x2, x3, x4);
    BR(xRA);

SETMARK(d_inc16);
    LHU(x1, xEmu, offsetof(x64emu_t, op1));
    emit_inc16(dyn, ninst, x1, x2, x3, x4, x5);
    BR(xRA);

SETMARK(d_inc32);
    LWU(x1, xEmu, offsetof(x64emu_t, op1));
    rex.w = 0;
    emit_inc32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    BR(xRA);

SETMARK(d_inc64);
    LD(x1, xEmu, offsetof(x64emu_t, op1));
    rex.w = 1;
    emit_inc32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    rex.w = 0;
    BR(xRA);

SETMARK(d_imul8);
    LH(x1, xEmu, offsetof(x64emu_t, res));
    SRAI(x2, x1, 7);
    SRAI(x3, x1, 15);
    XOR(x2, x2, x3);
    SNEZ(x2, x2); // CF=OF if the byte product does not sign-extend
    ANDI(xFlags, xFlags, ~((1 << F_CF) | (1 << F_OF2)));
    OR(xFlags, xFlags, x2);
    SLLI(x2, x2, F_OF2);
    OR(xFlags, xFlags, x2);
    if (!BOX64ENV(cputype)) {
        SRLI(x2, x1, 7);
        PUTFLAG(x2, F_SF);
        ANDI(xFlags, xFlags, ~((1 << F_ZF) | (1 << F_AF) | (1 << F_PF)));
        ANDI(x3, x1, 0xff);
        emit_pf(dyn, ninst, x3, x4, x5);
    }
    BR(xRA);

SETMARK(d_imul16);
    LW(x1, xEmu, offsetof(x64emu_t, res));
    SRAI(x2, x1, 15);
    SRAI(x3, x1, 31);
    XOR(x2, x2, x3);
    SNEZ(x2, x2);
    ANDI(xFlags, xFlags, ~((1 << F_CF) | (1 << F_OF2)));
    OR(xFlags, xFlags, x2);
    SLLI(x2, x2, F_OF2);
    OR(xFlags, xFlags, x2);
    if (!BOX64ENV(cputype)) {
        SRLI(x2, x1, 15);
        PUTFLAG(x2, F_SF);
        ANDI(xFlags, xFlags, ~((1 << F_ZF) | (1 << F_AF) | (1 << F_PF)));
        ANDI(x3, x1, 0xff);
        emit_pf(dyn, ninst, x3, x4, x5);
    }
    BR(xRA);

SETMARK(d_imul32);
    LWU(x1, xEmu, offsetof(x64emu_t, res));
    LW(x2, xEmu, offsetof(x64emu_t, op1));
    SRAI(x3, x1, 31);
    XOR(x3, x3, x2);
    SNEZ(x3, x3);
    ANDI(xFlags, xFlags, ~((1 << F_CF) | (1 << F_OF2)));
    OR(xFlags, xFlags, x3);
    SLLI(x3, x3, F_OF2);
    OR(xFlags, xFlags, x3);
    if (!BOX64ENV(cputype)) {
        SRLI(x2, x1, 31);
        PUTFLAG(x2, F_SF);
        ANDI(xFlags, xFlags, ~((1 << F_ZF) | (1 << F_AF) | (1 << F_PF)));
        ANDI(x3, x1, 0xff);
        emit_pf(dyn, ninst, x3, x4, x5);
    }
    BR(xRA);

SETMARK(d_imul64);
    LD(x1, xEmu, offsetof(x64emu_t, res));
    LD(x2, xEmu, offsetof(x64emu_t, op1));
    SRAI(x3, x1, 63);
    XOR(x3, x3, x2);
    SNEZ(x3, x3);
    ANDI(xFlags, xFlags, ~((1 << F_CF) | (1 << F_OF2)));
    OR(xFlags, xFlags, x3);
    SLLI(x3, x3, F_OF2);
    OR(xFlags, xFlags, x3);
    if (!BOX64ENV(cputype)) {
        SRLI(x2, x1, 63);
        PUTFLAG(x2, F_SF);
        ANDI(xFlags, xFlags, ~((1 << F_ZF) | (1 << F_AF) | (1 << F_PF)));
        ANDI(x3, x1, 0xff);
        emit_pf(dyn, ninst, x3, x4, x5);
    }
    BR(xRA);

SETMARK(d_or8);
    LBU(x1, xEmu, offsetof(x64emu_t, res));
    MV(x2, xZR);
    emit_or8(dyn, ninst, x1, x2, x3, x4);
    BR(xRA);

SETMARK(d_or16);
    LHU(x1, xEmu, offsetof(x64emu_t, res));
    MV(x2, xZR);
    emit_or16(dyn, ninst, x1, x2, x3, x4);
    BR(xRA);

SETMARK(d_or32);
    LWU(x1, xEmu, offsetof(x64emu_t, res));
    MV(x2, xZR);
    rex.w = 0;
    emit_or32(dyn, ninst, rex, x1, x2, x3, x4);
    BR(xRA);

SETMARK(d_or64);
    LD(x1, xEmu, offsetof(x64emu_t, res));
    MV(x2, xZR);
    rex.w = 1;
    emit_or32(dyn, ninst, rex, x1, x2, x3, x4);
    rex.w = 0;
    BR(xRA);

SETMARK(d_mul8);
    LHU(x1, xEmu, offsetof(x64emu_t, res));
    SRLI(x2, x1, 8);
    SNEZ(x2, x2); // CF=OF if high byte is not 0
    ANDI(xFlags, xFlags, ~((1 << F_CF) | (1 << F_OF2)));
    OR(xFlags, xFlags, x2);
    SLLI(x2, x2, F_OF2);
    OR(xFlags, xFlags, x2);
    if (!BOX64ENV(cputype)) {
        SRLI(x2, x1, 7);
        PUTFLAG(x2, F_SF);
        ANDI(xFlags, xFlags, ~((1 << F_ZF) | (1 << F_AF) | (1 << F_PF)));
        ANDI(x3, x1, 0xff);
        emit_pf(dyn, ninst, x3, x4, x5);
    }
    BR(xRA);

SETMARK(d_mul16);
    LWU(x1, xEmu, offsetof(x64emu_t, res));
    SRLI(x2, x1, 16);
    SNEZ(x2, x2);
    ANDI(xFlags, xFlags, ~((1 << F_CF) | (1 << F_OF2)));
    OR(xFlags, xFlags, x2);
    SLLI(x2, x2, F_OF2);
    OR(xFlags, xFlags, x2);
    if (!BOX64ENV(cputype)) {
        SRLI(x2, x1, 15);
        PUTFLAG(x2, F_SF);
        ANDI(xFlags, xFlags, ~((1 << F_ZF) | (1 << F_AF) | (1 << F_PF)));
        ANDI(x3, x1, 0xff);
        emit_pf(dyn, ninst, x3, x4, x5);
    }
    BR(xRA);

SETMARK(d_mul32);
    LWU(x2, xEmu, offsetof(x64emu_t, op1));
    SNEZ(x3, x2); // CF=OF if op1 is not 0
    ANDI(xFlags, xFlags, ~((1 << F_CF) | (1 << F_OF2)));
    OR(xFlags, xFlags, x3);
    SLLI(x3, x3, F_OF2);
    OR(xFlags, xFlags, x3);
    if (!BOX64ENV(cputype)) {
        LWU(x1, xEmu, offsetof(x64emu_t, res));
        SRLI(x2, x1, 31);
        PUTFLAG(x2, F_SF);
        ANDI(xFlags, xFlags, ~((1 << F_ZF) | (1 << F_AF) | (1 << F_PF)));
        ANDI(x3, x1, 0xff);
        emit_pf(dyn, ninst, x3, x4, x5);
    }
    BR(xRA);

SETMARK(d_mul64);
    LD(x2, xEmu, offsetof(x64emu_t, op1));
    SNEZ(x3, x2);
    ANDI(xFlags, xFlags, ~((1 << F_CF) | (1 << F_OF2)));
    OR(xFlags, xFlags, x3);
    SLLI(x3, x3, F_OF2);
    OR(xFlags, xFlags, x3);
    if (!BOX64ENV(cputype)) {
        LD(x1, xEmu, offsetof(x64emu_t, res));
        SRLI(x2, x1, 63);
        PUTFLAG(x2, F_SF);
        ANDI(xFlags, xFlags, ~((1 << F_ZF) | (1 << F_AF) | (1 << F_PF)));
        ANDI(x3, x1, 0xff);
        emit_pf(dyn, ninst, x3, x4, x5);
    }
    BR(xRA);

SETMARK(d_neg8);
    LBU(x1, xEmu, offsetof(x64emu_t, op1));
    emit_neg8(dyn, ninst, x1, x2, x3, x4);
    BR(xRA);

SETMARK(d_neg16);
    LHU(x1, xEmu, offsetof(x64emu_t, op1));
    emit_neg16(dyn, ninst, x1, x2, x3, x4);
    BR(xRA);

SETMARK(d_neg32);
    LWU(x1, xEmu, offsetof(x64emu_t, op1));
    rex.w = 0;
    emit_neg32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    BR(xRA);

SETMARK(d_neg64);
    LD(x1, xEmu, offsetof(x64emu_t, op1));
    rex.w = 1;
    emit_neg32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    rex.w = 0;
    BR(xRA);

SETMARK(d_shl8);
    LBU(x1, xEmu, offsetof(x64emu_t, op1));
    LBU(x2, xEmu, offsetof(x64emu_t, op2));
    ANDI(x2, x2, 0x1f);
    BNEZ(x2, 8);
    BR(xRA); // count == 0, nothing to do
    emit_shl8(dyn, ninst, x1, x2, x3, x4, x5);
    BR(xRA);

SETMARK(d_shl16);
    LHU(x1, xEmu, offsetof(x64emu_t, op1));
    LHU(x2, xEmu, offsetof(x64emu_t, op2));
    ANDI(x2, x2, 0x1f);
    BNEZ(x2, 8);
    BR(xRA);
    emit_shl16(dyn, ninst, x1, x2, x3, x4, x5);
    BR(xRA);

SETMARK(d_shl32);
    LWU(x1, xEmu, offsetof(x64emu_t, op1));
    LWU(x2, xEmu, offsetof(x64emu_t, op2));
    ANDI(x2, x2, 0x1f);
    BNEZ(x2, 8);
    BR(xRA);
    rex.w = 0;
    emit_shl32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    BR(xRA);

SETMARK(d_shl64);
    LD(x1, xEmu, offsetof(x64emu_t, op1));
    LD(x2, xEmu, offsetof(x64emu_t, op2));
    ANDI(x2, x2, 0x3f);
    BNEZ(x2, 8);
    BR(xRA);
    rex.w = 1;
    emit_shl32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    rex.w = 0;
    BR(xRA);

SETMARK(d_shr8);
    LBU(x1, xEmu, offsetof(x64emu_t, op1));
    LBU(x2, xEmu, offsetof(x64emu_t, op2));
    ANDI(x2, x2, 0x1f);
    BNEZ(x2, 8);
    BR(xRA);
    emit_shr8(dyn, ninst, x1, x2, x3, x4, x5);
    BR(xRA);

SETMARK(d_shr16);
    LHU(x1, xEmu, offsetof(x64emu_t, op1));
    LHU(x2, xEmu, offsetof(x64emu_t, op2));
    ANDI(x2, x2, 0x1f);
    BNEZ(x2, 8);
    BR(xRA);
    emit_shr16(dyn, ninst, x1, x2, x3, x4, x5);
    BR(xRA);

SETMARK(d_shr32);
    LWU(x1, xEmu, offsetof(x64emu_t, op1));
    LWU(x2, xEmu, offsetof(x64emu_t, op2));
    ANDI(x2, x2, 0x1f);
    BNEZ(x2, 8);
    BR(xRA);
    rex.w = 0;
    emit_shr32(dyn, ninst, rex, x1, x2, x3, x4);
    BR(xRA);

SETMARK(d_shr64);
    LD(x1, xEmu, offsetof(x64emu_t, op1));
    LD(x2, xEmu, offsetof(x64emu_t, op2));
    ANDI(x2, x2, 0x3f);
    BNEZ(x2, 8);
    BR(xRA);
    rex.w = 1;
    emit_shr32(dyn, ninst, rex, x1, x2, x3, x4);
    rex.w = 0;
    BR(xRA);

SETMARK(d_sar8);
    LB(x1, xEmu, offsetof(x64emu_t, op1));
    LBU(x2, xEmu, offsetof(x64emu_t, op2));
    ANDI(x2, x2, 0x1f);
    BNEZ(x2, 8);
    BR(xRA);
    emit_sar8(dyn, ninst, x1, x2, x3, x4, x5);
    BR(xRA);

SETMARK(d_sar16);
    LH(x1, xEmu, offsetof(x64emu_t, op1));
    LHU(x2, xEmu, offsetof(x64emu_t, op2));
    ANDI(x2, x2, 0x1f);
    BNEZ(x2, 8);
    BR(xRA);
    emit_sar16(dyn, ninst, x1, x2, x3, x4, x5);
    BR(xRA);

SETMARK(d_sar32);
    LW(x1, xEmu, offsetof(x64emu_t, op1));
    LWU(x2, xEmu, offsetof(x64emu_t, op2));
    ANDI(x2, x2, 0x1f);
    BNEZ(x2, 8);
    BR(xRA);
    rex.w = 0;
    emit_sar32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    BR(xRA);

SETMARK(d_sar64);
    LD(x1, xEmu, offsetof(x64emu_t, op1));
    LD(x2, xEmu, offsetof(x64emu_t, op2));
    ANDI(x2, x2, 0x3f);
    BNEZ(x2, 8);
    BR(xRA);
    rex.w = 1;
    emit_sar32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    rex.w = 0;
    BR(xRA);

SETMARK(d_sub8);
    LBU(x1, xEmu, offsetof(x64emu_t, op1));
    LBU(x2, xEmu, offsetof(x64emu_t, op2));
    emit_sub8(dyn, ninst, x1, x2, x3, x4, x5);
    BR(xRA);

SETMARK(d_sub16);
    LHU(x1, xEmu, offsetof(x64emu_t, op1));
    LHU(x2, xEmu, offsetof(x64emu_t, op2));
    emit_sub16(dyn, ninst, x1, x2, x3, x4, x5);
    BR(xRA);

SETMARK(d_sub32);
    LWU(x1, xEmu, offsetof(x64emu_t, op1));
    LWU(x2, xEmu, offsetof(x64emu_t, op2));
    rex.w = 0;
    emit_sub32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    BR(xRA);

SETMARK(d_sub64);
    LD(x1, xEmu, offsetof(x64emu_t, op1));
    LD(x2, xEmu, offsetof(x64emu_t, op2));
    rex.w = 1;
    emit_sub32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    rex.w = 0;
    BR(xRA);

SETMARK(d_xor8);
    LBU(x1, xEmu, offsetof(x64emu_t, res));
    MV(x2, xZR);
    emit_xor8(dyn, ninst, x1, x2, x3, x4);
    BR(xRA);

SETMARK(d_xor16);
    LHU(x1, xEmu, offsetof(x64emu_t, res));
    MV(x2, xZR);
    emit_xor16(dyn, ninst, x1, x2, x3, x4, x5);
    BR(xRA);

SETMARK(d_xor32);
    LWU(x1, xEmu, offsetof(x64emu_t, res));
    MV(x2, xZR);
    rex.w = 0;
    emit_xor32(dyn, ninst, rex, x1, x2, x3, x4);
    BR(xRA);

SETMARK(d_xor64);
    LD(x1, xEmu, offsetof(x64emu_t, res));
    MV(x2, xZR);
    rex.w = 1;
    emit_xor32(dyn, ninst, rex, x1, x2, x3, x4);
    rex.w = 0;
    BR(xRA);

SETMARK(d_cmp8);
    LBU(x1, xEmu, offsetof(x64emu_t, op1));
    LBU(x2, xEmu, offsetof(x64emu_t, op2));
    emit_cmp8(dyn, ninst, x1, x2, x3, x4, x5, x6);
    BR(xRA);

SETMARK(d_cmp16);
    LHU(x1, xEmu, offsetof(x64emu_t, op1));
    LHU(x2, xEmu, offsetof(x64emu_t, op2));
    emit_cmp16(dyn, ninst, x1, x2, x3, x4, x5, x6);
    BR(xRA);

SETMARK(d_cmp32);
    LWU(x1, xEmu, offsetof(x64emu_t, op1));
    LWU(x2, xEmu, offsetof(x64emu_t, op2));
    rex.w = 0;
    emit_cmp32(dyn, ninst, rex, x1, x2, x3, x4, x5, x6);
    BR(xRA);

SETMARK(d_cmp64);
    LD(x1, xEmu, offsetof(x64emu_t, op1));
    LD(x2, xEmu, offsetof(x64emu_t, op2));
    rex.w = 1;
    emit_cmp32(dyn, ninst, rex, x1, x2, x3, x4, x5, x6);
    rex.w = 0;
    BR(xRA);

SETMARK(d_tst8);
    LBU(x1, xEmu, offsetof(x64emu_t, res));
    ADDI(x2, xZR, 0xff);
    emit_test8(dyn, ninst, x1, x2, x3, x4, x5);
    BR(xRA);

SETMARK(d_tst16);
    LHU(x1, xEmu, offsetof(x64emu_t, res));
    MOV32w(x2, 0xffff);
    emit_test16(dyn, ninst, x1, x2, x3, x4, x5);
    BR(xRA);

SETMARK(d_tst32);
    LWU(x1, xEmu, offsetof(x64emu_t, res));
    MOV32w(x2, 0xffffffff);
    rex.w = 0;
    emit_test32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    BR(xRA);

SETMARK(d_tst64);
    LD(x1, xEmu, offsetof(x64emu_t, res));
    ADDI(x2, xZR, -1);
    rex.w = 1;
    emit_test32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    rex.w = 0;
    BR(xRA);

// for ADC & SBB, the emit_adcX/emit_sbbX cannot be used because the carry state
// in xFlags is not guaranteed to be the one from before the deferred instruction
SETMARK(d_adc8);
    LHU(x1, xEmu, offsetof(x64emu_t, res));
    LBU(x2, xEmu, offsetof(x64emu_t, op1));
    LBU(x3, xEmu, offsetof(x64emu_t, op2));
    SRLI(x4, x1, 8);
    PUTFLAG(x4, F_CF); // CF from bit 8 of the 9 bits result
    ANDI(x4, x1, 0xff);
    SRLI(x5, x4, 7);
    PUTFLAG(x5, F_SF);
    SEQZ(x5, x4);
    PUTFLAG(x5, F_ZF);
    AND(x4, x2, x3); // op1 & op2
    OR(x5, x2, x3); // op1 | op2
    NOT(x6, x1);
    AND(x6, x6, x5); // ~res & (op1 | op2)
    OR(x4, x4, x6); // cc
    SRLI(x5, x4, 3);
    PUTFLAG(x5, F_AF);
    SRLI(x5, x4, 6);
    SRLI(x6, x5, 1);
    XOR(x5, x5, x6);
    PUTFLAG(x5, F_OF2);
    ANDI(xFlags, xFlags, ~(1 << F_PF));
    emit_pf(dyn, ninst, x1, x5, x6);
    BR(xRA);

SETMARK(d_adc8b);
    LBU(x1, xEmu, offsetof(x64emu_t, res));
    LBU(x2, xEmu, offsetof(x64emu_t, op1));
    LBU(x3, xEmu, offsetof(x64emu_t, op2));
    ADD(x4, x2, x3);
    ANDI(x4, x4, 0xff);
    SUB(x4, x1, x4);
    SNEZ(x4, x4); // detect the carry-in
    ADD(x1, x2, x3);
    ADD(x1, x1, x4); // recompute res as 9 bits
    SRLI(x4, x1, 8);
    PUTFLAG(x4, F_CF);
    ANDI(x4, x1, 0xff);
    SRLI(x5, x4, 7);
    PUTFLAG(x5, F_SF);
    SEQZ(x5, x4);
    PUTFLAG(x5, F_ZF);
    AND(x4, x2, x3);
    OR(x5, x2, x3);
    NOT(x6, x1);
    AND(x6, x6, x5);
    OR(x4, x4, x6); // cc
    SRLI(x5, x4, 3);
    PUTFLAG(x5, F_AF);
    SRLI(x5, x4, 6);
    SRLI(x6, x5, 1);
    XOR(x5, x5, x6);
    PUTFLAG(x5, F_OF2);
    ANDI(xFlags, xFlags, ~(1 << F_PF));
    emit_pf(dyn, ninst, x1, x5, x6);
    BR(xRA);

SETMARK(d_adc16);
    LWU(x1, xEmu, offsetof(x64emu_t, res));
    LHU(x2, xEmu, offsetof(x64emu_t, op1));
    LHU(x3, xEmu, offsetof(x64emu_t, op2));
    SRLI(x4, x1, 16);
    PUTFLAG(x4, F_CF); // CF from bit 16 of the 17 bits result
    ANDI(x4, x1, 0xffff);
    SRLI(x5, x4, 15);
    PUTFLAG(x5, F_SF);
    SEQZ(x5, x4);
    PUTFLAG(x5, F_ZF);
    AND(x4, x2, x3);
    OR(x5, x2, x3);
    NOT(x6, x1);
    AND(x6, x6, x5);
    OR(x4, x4, x6); // cc
    SRLI(x5, x4, 3);
    PUTFLAG(x5, F_AF);
    SRLI(x5, x4, 14);
    SRLI(x6, x5, 1);
    XOR(x5, x5, x6);
    PUTFLAG(x5, F_OF2);
    ANDI(xFlags, xFlags, ~(1 << F_PF));
    emit_pf(dyn, ninst, x1, x5, x6);
    BR(xRA);

SETMARK(d_adc16b);
    LHU(x1, xEmu, offsetof(x64emu_t, res));
    LHU(x2, xEmu, offsetof(x64emu_t, op1));
    LHU(x3, xEmu, offsetof(x64emu_t, op2));
    ADD(x4, x2, x3);
    ZEXTH(x4, x4);
    SUB(x4, x1, x4);
    SNEZ(x4, x4); // detect the carry-in
    ADD(x1, x2, x3);
    ADD(x1, x1, x4); // recompute res as 17 bits
    SRLI(x4, x1, 16);
    PUTFLAG(x4, F_CF);
    ANDI(x4, x1, 0xffff);
    SRLI(x5, x4, 15);
    PUTFLAG(x5, F_SF);
    SEQZ(x5, x4);
    PUTFLAG(x5, F_ZF);
    AND(x4, x2, x3);
    OR(x5, x2, x3);
    NOT(x6, x1);
    AND(x6, x6, x5);
    OR(x4, x4, x6); // cc
    SRLI(x5, x4, 3);
    PUTFLAG(x5, F_AF);
    SRLI(x5, x4, 14);
    SRLI(x6, x5, 1);
    XOR(x5, x5, x6);
    PUTFLAG(x5, F_OF2);
    ANDI(xFlags, xFlags, ~(1 << F_PF));
    emit_pf(dyn, ninst, x1, x5, x6);
    BR(xRA);

SETMARK(d_adc32);
    LD(x1, xEmu, offsetof(x64emu_t, res));
    LWU(x2, xEmu, offsetof(x64emu_t, op1));
    LWU(x3, xEmu, offsetof(x64emu_t, op2));
    SRLI(x4, x1, 32);
    PUTFLAG(x4, F_CF); // CF from bit 32 of the 33 bits result
    SLLI(x4, x1, 32);
    SRLI(x4, x4, 32); // res32
    SRLI(x5, x4, 31);
    PUTFLAG(x5, F_SF);
    SEQZ(x5, x4);
    PUTFLAG(x5, F_ZF);
    AND(x4, x2, x3);
    OR(x5, x2, x3);
    NOT(x6, x1);
    AND(x6, x6, x5);
    OR(x4, x4, x6); // cc
    SRLI(x5, x4, 3);
    PUTFLAG(x5, F_AF);
    SRLI(x5, x4, 30);
    SRLI(x6, x5, 1);
    XOR(x5, x5, x6);
    PUTFLAG(x5, F_OF2);
    ANDI(xFlags, xFlags, ~(1 << F_PF));
    emit_pf(dyn, ninst, x1, x5, x6);
    BR(xRA);

SETMARK(d_adc32b);
    LWU(x1, xEmu, offsetof(x64emu_t, res));
    LWU(x2, xEmu, offsetof(x64emu_t, op1));
    LWU(x3, xEmu, offsetof(x64emu_t, op2));
    SRLI(x4, x1, 31);
    PUTFLAG(x4, F_SF);
    SEQZ(x4, x1);
    PUTFLAG(x4, F_ZF);
    ADDW(x4, x2, x3);
    SUBW(x4, x1, x4);
    SNEZ(x4, x4); // detect the carry-in
    ZEXTH(x5, x2);
    ZEXTH(x6, x3);
    ADD(x4, x4, x5);
    ADD(x4, x4, x6); // lo
    SRLI(x4, x4, 16); // carry out of the low 16 bits
    SRLI(x5, x2, 16);
    SRLI(x6, x3, 16);
    ADD(x4, x4, x5);
    ADD(x4, x4, x6); // hi
    SRLI(x4, x4, 16);
    PUTFLAG(x4, F_CF);
    AND(x4, x2, x3);
    OR(x5, x2, x3);
    NOT(x6, x1);
    AND(x6, x6, x5);
    OR(x4, x4, x6); // cc
    SRLI(x5, x4, 3);
    PUTFLAG(x5, F_AF);
    SRLI(x5, x4, 30);
    SRLI(x6, x5, 1);
    XOR(x5, x5, x6);
    PUTFLAG(x5, F_OF2);
    ANDI(xFlags, xFlags, ~(1 << F_PF));
    emit_pf(dyn, ninst, x1, x5, x6);
    BR(xRA);

SETMARK(d_adc64);
    LD(x1, xEmu, offsetof(x64emu_t, res));
    LD(x2, xEmu, offsetof(x64emu_t, op1));
    LD(x3, xEmu, offsetof(x64emu_t, op2));
    SRLI(x4, x1, 63);
    PUTFLAG(x4, F_SF);
    SEQZ(x4, x1);
    PUTFLAG(x4, F_ZF);
    ADD(x4, x2, x3);
    SUB(x4, x1, x4);
    SNEZ(x4, x4); // detect the carry-in
    ZEXTW2(x5, x2);
    ZEXTW2(x6, x3);
    ADD(x4, x4, x5);
    ADD(x4, x4, x6); // lo
    SRLI(x4, x4, 32); // carry out of the low 32 bits
    SRLI(x5, x2, 32);
    SRLI(x6, x3, 32);
    ADD(x4, x4, x5);
    ADD(x4, x4, x6); // hi
    SRLI(x4, x4, 32);
    PUTFLAG(x4, F_CF);
    AND(x4, x2, x3);
    OR(x5, x2, x3);
    NOT(x6, x1);
    AND(x6, x6, x5);
    OR(x4, x4, x6); // cc
    SRLI(x5, x4, 3);
    PUTFLAG(x5, F_AF);
    SRLI(x5, x4, 62);
    SRLI(x6, x5, 1);
    XOR(x5, x5, x6);
    PUTFLAG(x5, F_OF2);
    ANDI(xFlags, xFlags, ~(1 << F_PF));
    emit_pf(dyn, ninst, x1, x5, x6);
    BR(xRA);

SETMARK(d_sbb8);
    LBU(x1, xEmu, offsetof(x64emu_t, res));
    LBU(x2, xEmu, offsetof(x64emu_t, op1));
    LBU(x3, xEmu, offsetof(x64emu_t, op2));
    SRLI(x4, x1, 7);
    PUTFLAG(x4, F_SF);
    SEQZ(x4, x1);
    PUTFLAG(x4, F_ZF);
    NOT(x4, x2);
    OR(x4, x4, x3); // ~op1 | op2
    AND(x4, x4, x1); // res & (~op1 | op2)
    NOT(x5, x2);
    AND(x5, x5, x3); // ~op1 & op2
    OR(x4, x4, x5); // bc
    SRLI(x5, x4, 7);
    PUTFLAG(x5, F_CF);
    SRLI(x5, x4, 3);
    PUTFLAG(x5, F_AF);
    SRLI(x5, x4, 6);
    SRLI(x6, x5, 1);
    XOR(x5, x5, x6);
    PUTFLAG(x5, F_OF2);
    ANDI(xFlags, xFlags, ~(1 << F_PF));
    emit_pf(dyn, ninst, x1, x5, x6);
    BR(xRA);

SETMARK(d_sbb16);
    LHU(x1, xEmu, offsetof(x64emu_t, res));
    LHU(x2, xEmu, offsetof(x64emu_t, op1));
    LHU(x3, xEmu, offsetof(x64emu_t, op2));
    SRLI(x4, x1, 15);
    PUTFLAG(x4, F_SF);
    SEQZ(x4, x1);
    PUTFLAG(x4, F_ZF);
    NOT(x4, x2);
    OR(x4, x4, x3);
    AND(x4, x4, x1);
    NOT(x5, x2);
    AND(x5, x5, x3);
    OR(x4, x4, x5); // bc
    SRLI(x5, x4, 15);
    PUTFLAG(x5, F_CF);
    SRLI(x5, x4, 3);
    PUTFLAG(x5, F_AF);
    SRLI(x5, x4, 14);
    SRLI(x6, x5, 1);
    XOR(x5, x5, x6);
    PUTFLAG(x5, F_OF2);
    ANDI(xFlags, xFlags, ~(1 << F_PF));
    emit_pf(dyn, ninst, x1, x5, x6);
    BR(xRA);

SETMARK(d_sbb32);
    LWU(x1, xEmu, offsetof(x64emu_t, res));
    LWU(x2, xEmu, offsetof(x64emu_t, op1));
    LWU(x3, xEmu, offsetof(x64emu_t, op2));
    SRLI(x4, x1, 31);
    PUTFLAG(x4, F_SF);
    SEQZ(x4, x1);
    PUTFLAG(x4, F_ZF);
    NOT(x4, x2);
    OR(x4, x4, x3);
    AND(x4, x4, x1);
    NOT(x5, x2);
    AND(x5, x5, x3);
    OR(x4, x4, x5); // bc
    SRLI(x5, x4, 31);
    PUTFLAG(x5, F_CF);
    SRLI(x5, x4, 3);
    PUTFLAG(x5, F_AF);
    SRLI(x5, x4, 30);
    SRLI(x6, x5, 1);
    XOR(x5, x5, x6);
    PUTFLAG(x5, F_OF2);
    ANDI(xFlags, xFlags, ~(1 << F_PF));
    emit_pf(dyn, ninst, x1, x5, x6);
    BR(xRA);

SETMARK(d_sbb64);
    LD(x1, xEmu, offsetof(x64emu_t, res));
    LD(x2, xEmu, offsetof(x64emu_t, op1));
    LD(x3, xEmu, offsetof(x64emu_t, op2));
    SRLI(x4, x1, 63);
    PUTFLAG(x4, F_SF);
    SEQZ(x4, x1);
    PUTFLAG(x4, F_ZF);
    NOT(x4, x2);
    OR(x4, x4, x3);
    AND(x4, x4, x1);
    NOT(x5, x2);
    AND(x5, x5, x3);
    OR(x4, x4, x5); // bc
    SRLI(x5, x4, 63);
    PUTFLAG(x5, F_CF);
    SRLI(x5, x4, 3);
    PUTFLAG(x5, F_AF);
    SRLI(x5, x4, 62);
    SRLI(x6, x5, 1);
    XOR(x5, x5, x6);
    PUTFLAG(x5, F_OF2);
    ANDI(xFlags, xFlags, ~(1 << F_PF));
    emit_pf(dyn, ninst, x1, x5, x6);
    BR(xRA);

SETMARK(d_rol8);
    LBU(x1, xEmu, offsetof(x64emu_t, res));
    if (BOX64ENV(cputype)) {
        SRLI(x2, x1, 7);
        XOR(x2, x2, x1);
        PUTFLAG(x2, F_OF2);
    } else {
        LBU(x2, xEmu, offsetof(x64emu_t, op1));
        SRLI(x3, x2, 6);
        SRLI(x4, x3, 1);
        XOR(x3, x3, x4);
        PUTFLAG(x3, F_OF2);
    }
    PUTFLAG(x1, F_CF);
    BR(xRA);

SETMARK(d_rol16);
    LHU(x1, xEmu, offsetof(x64emu_t, res));
    if (BOX64ENV(cputype)) {
        SRLI(x2, x1, 15);
        XOR(x2, x2, x1);
        PUTFLAG(x2, F_OF2);
    } else {
        LHU(x2, xEmu, offsetof(x64emu_t, op1));
        SRLI(x3, x2, 14);
        SRLI(x4, x3, 1);
        XOR(x3, x3, x4);
        PUTFLAG(x3, F_OF2);
    }
    PUTFLAG(x1, F_CF);
    BR(xRA);

SETMARK(d_rol32);
    LWU(x1, xEmu, offsetof(x64emu_t, res));
    if (BOX64ENV(cputype)) {
        SRLI(x2, x1, 31);
        XOR(x2, x2, x1);
        PUTFLAG(x2, F_OF2);
    } else {
        LWU(x2, xEmu, offsetof(x64emu_t, op1));
        SRLI(x3, x2, 30);
        SRLI(x4, x3, 1);
        XOR(x3, x3, x4);
        PUTFLAG(x3, F_OF2);
    }
    PUTFLAG(x1, F_CF);
    BR(xRA);

SETMARK(d_rol64);
    LD(x1, xEmu, offsetof(x64emu_t, res));
    if (BOX64ENV(cputype)) {
        SRLI(x2, x1, 63);
        XOR(x2, x2, x1);
        PUTFLAG(x2, F_OF2);
    } else {
        LD(x2, xEmu, offsetof(x64emu_t, op1));
        SRLI(x3, x2, 62);
        SRLI(x4, x3, 1);
        XOR(x3, x3, x4);
        PUTFLAG(x3, F_OF2);
    }
    PUTFLAG(x1, F_CF);
    BR(xRA);

SETMARK(d_ror8);
    LBU(x1, xEmu, offsetof(x64emu_t, res));
    if (BOX64ENV(cputype)) {
        SRLI(x2, x1, 6);
        SRLI(x3, x2, 1);
        XOR(x2, x2, x3);
        PUTFLAG(x2, F_OF2);
    } else {
        LBU(x2, xEmu, offsetof(x64emu_t, op1));
        SRLI(x3, x2, 7);
        XOR(x3, x3, x2);
        PUTFLAG(x3, F_OF2);
    }
    SRLI(x2, x1, 7);
    PUTFLAG(x2, F_CF);
    BR(xRA);

SETMARK(d_ror16);
    LHU(x1, xEmu, offsetof(x64emu_t, res));
    if (BOX64ENV(cputype)) {
        SRLI(x2, x1, 14);
        SRLI(x3, x2, 1);
        XOR(x2, x2, x3);
        PUTFLAG(x2, F_OF2);
    } else {
        LHU(x2, xEmu, offsetof(x64emu_t, op1));
        SRLI(x3, x2, 15);
        XOR(x3, x3, x2);
        PUTFLAG(x3, F_OF2);
    }
    SRLI(x2, x1, 15);
    PUTFLAG(x2, F_CF);
    BR(xRA);

SETMARK(d_ror32);
    LWU(x1, xEmu, offsetof(x64emu_t, res));
    if (BOX64ENV(cputype)) {
        SRLI(x2, x1, 30);
        SRLI(x3, x2, 1);
        XOR(x2, x2, x3);
        PUTFLAG(x2, F_OF2);
    } else {
        LWU(x2, xEmu, offsetof(x64emu_t, op1));
        SRLI(x3, x2, 31);
        XOR(x3, x3, x2);
        PUTFLAG(x3, F_OF2);
    }
    SRLI(x2, x1, 31);
    PUTFLAG(x2, F_CF);
    BR(xRA);

SETMARK(d_ror64);
    LD(x1, xEmu, offsetof(x64emu_t, res));
    if (BOX64ENV(cputype)) {
        SRLI(x2, x1, 62);
        SRLI(x3, x2, 1);
        XOR(x2, x2, x3);
        PUTFLAG(x2, F_OF2);
    } else {
        LD(x2, xEmu, offsetof(x64emu_t, op1));
        SRLI(x3, x2, 63);
        XOR(x3, x3, x2);
        PUTFLAG(x3, F_OF2);
    }
    SRLI(x2, x1, 63);
    PUTFLAG(x2, F_CF);
    BR(xRA);

SETMARK(d_shrd16);
    LHU(x1, xEmu, offsetof(x64emu_t, res));
    LHU(x2, xEmu, offsetof(x64emu_t, op1));
    LHU(x3, xEmu, offsetof(x64emu_t, op2));
    BNEZ(x3, 8);
    BR(xRA); // count == 0, nothing to do
    if (BOX64ENV(cputype)) {
        SRLI(x4, x1, 14);
        SRLI(x5, x4, 1);
        XOR(x4, x4, x5);
        PUTFLAG(x4, F_OF2);
        ORI(xFlags, xFlags, 1 << F_AF);
    } else {
        ANDI(x4, x3, 0x0f);
        ADDI(x4, x4, -16);
        NEG(x4, x4);
        SRL(x4, x1, x4);
        SRLI(x5, x2, 15);
        XOR(x4, x4, x5);
        PUTFLAG(x4, F_OF2);
        ANDI(xFlags, xFlags, ~(1 << F_AF));
    }
    ADDI(x4, x3, -1);
    SRLW(x4, x2, x4); // CF = bit (count-1) of op1
    if (BOX64ENV(cputype)) {
        SLTI(x5, x3, 16);
        BNEZ(x5, 8);
        MV(x4, xZR); // count > 15: CF = 0
    }
    PUTFLAG(x4, F_CF);
    SRLI(x4, x1, 15);
    PUTFLAG(x4, F_SF);
    SEQZ(x4, x1);
    PUTFLAG(x4, F_ZF);
    ANDI(xFlags, xFlags, ~(1 << F_PF));
    emit_pf(dyn, ninst, x1, x5, x6);
    BR(xRA);

SETMARK(d_shrd32);
    LWU(x1, xEmu, offsetof(x64emu_t, res));
    LWU(x2, xEmu, offsetof(x64emu_t, op1));
    LWU(x3, xEmu, offsetof(x64emu_t, op2));
    BNEZ(x3, 8);
    BR(xRA);
    if (BOX64ENV(cputype)) {
        SRLI(x4, x1, 30);
        SRLI(x5, x4, 1);
        XOR(x4, x4, x5);
        PUTFLAG(x4, F_OF2);
        ORI(xFlags, xFlags, 1 << F_AF);
    } else {
        ADDI(x4, x3, -32);
        NEG(x4, x4);
        SRLW(x4, x1, x4);
        SRLI(x5, x2, 31);
        XOR(x4, x4, x5);
        PUTFLAG(x4, F_OF2);
        ANDI(xFlags, xFlags, ~(1 << F_AF));
    }
    ADDI(x4, x3, -1);
    SRLW(x4, x2, x4); // CF = bit (count-1) of op1
    PUTFLAG(x4, F_CF);
    SRLI(x4, x1, 31);
    PUTFLAG(x4, F_SF);
    SEQZ(x4, x1);
    PUTFLAG(x4, F_ZF);
    ANDI(xFlags, xFlags, ~(1 << F_PF));
    emit_pf(dyn, ninst, x1, x5, x6);
    BR(xRA);

SETMARK(d_shrd64);
    LD(x1, xEmu, offsetof(x64emu_t, res));
    LD(x2, xEmu, offsetof(x64emu_t, op1));
    LD(x3, xEmu, offsetof(x64emu_t, op2));
    BNEZ(x3, 8);
    BR(xRA);
    if (BOX64ENV(cputype)) {
        SRLI(x4, x1, 62);
        SRLI(x5, x4, 1);
        XOR(x4, x4, x5);
        PUTFLAG(x4, F_OF2);
        ORI(xFlags, xFlags, 1 << F_AF);
    } else {
        ADDI(x4, x3, -64);
        NEG(x4, x4);
        SRL(x4, x1, x4);
        SRLI(x5, x2, 63);
        XOR(x4, x4, x5);
        PUTFLAG(x4, F_OF2);
        ANDI(xFlags, xFlags, ~(1 << F_AF));
    }
    ADDI(x4, x3, -1);
    SRL(x4, x2, x4); // CF = bit (count-1) of op1
    PUTFLAG(x4, F_CF);
    SRLI(x4, x1, 63);
    PUTFLAG(x4, F_SF);
    SEQZ(x4, x1);
    PUTFLAG(x4, F_ZF);
    ANDI(xFlags, xFlags, ~(1 << F_PF));
    emit_pf(dyn, ninst, x1, x5, x6);
    BR(xRA);

SETMARK(d_shld16);
    LHU(x1, xEmu, offsetof(x64emu_t, res));
    LHU(x2, xEmu, offsetof(x64emu_t, op1));
    LHU(x3, xEmu, offsetof(x64emu_t, op2));
    BNEZ(x3, 8);
    BR(xRA);
    ADDI(x4, x3, -16);
    NEG(x4, x4);
    SRLW(x4, x2, x4); // CF = bit (16-count) of op1
    PUTFLAG(x4, F_CF);
    if (BOX64ENV(cputype)) {
        SRLI(x4, x1, 15);
        XOR(x4, x4, xFlags);
        SLTI(x5, x3, 16);
        BNEZ(x5, 8);
        MV(x4, xFlags); // count > 15: OF = CF
        PUTFLAG(x4, F_OF2);
        ORI(xFlags, xFlags, 1 << F_AF);
    } else {
        SRLI(x4, x2, 14);
        SRLI(x5, x4, 1);
        XOR(x4, x4, x5);
        PUTFLAG(x4, F_OF2);
        ANDI(xFlags, xFlags, ~(1 << F_AF));
    }
    SRLI(x4, x1, 15);
    PUTFLAG(x4, F_SF);
    SEQZ(x4, x1);
    PUTFLAG(x4, F_ZF);
    ANDI(xFlags, xFlags, ~(1 << F_PF));
    emit_pf(dyn, ninst, x1, x5, x6);
    BR(xRA);

SETMARK(d_shld32);
    LWU(x1, xEmu, offsetof(x64emu_t, res));
    LWU(x2, xEmu, offsetof(x64emu_t, op1));
    LWU(x3, xEmu, offsetof(x64emu_t, op2));
    BNEZ(x3, 8);
    BR(xRA);
    ADDI(x4, x3, -32);
    NEG(x4, x4);
    SRLW(x4, x2, x4); // CF = bit (32-count) of op1
    PUTFLAG(x4, F_CF);
    if (BOX64ENV(cputype)) {
        SRLI(x4, x1, 31);
        XOR(x4, x4, xFlags);
        PUTFLAG(x4, F_OF2);
        ORI(xFlags, xFlags, 1 << F_AF);
    } else {
        SRLI(x4, x2, 30);
        SRLI(x5, x4, 1);
        XOR(x4, x4, x5);
        PUTFLAG(x4, F_OF2);
        ANDI(xFlags, xFlags, ~(1 << F_AF));
    }
    SRLI(x4, x1, 31);
    PUTFLAG(x4, F_SF);
    SEQZ(x4, x1);
    PUTFLAG(x4, F_ZF);
    ANDI(xFlags, xFlags, ~(1 << F_PF));
    emit_pf(dyn, ninst, x1, x5, x6);
    BR(xRA);

SETMARK(d_shld64);
    LD(x1, xEmu, offsetof(x64emu_t, res));
    LD(x2, xEmu, offsetof(x64emu_t, op1));
    LD(x3, xEmu, offsetof(x64emu_t, op2));
    BNEZ(x3, 8);
    BR(xRA);
    ADDI(x4, x3, -64);
    NEG(x4, x4);
    SRL(x4, x2, x4); // CF = bit (64-count) of op1
    PUTFLAG(x4, F_CF);
    if (BOX64ENV(cputype)) {
        SRLI(x4, x1, 63);
        XOR(x4, x4, xFlags);
        PUTFLAG(x4, F_OF2);
        ORI(xFlags, xFlags, 1 << F_AF);
    } else {
        SRLI(x4, x2, 62);
        SRLI(x5, x4, 1);
        XOR(x4, x4, x5);
        PUTFLAG(x4, F_OF2);
        ANDI(xFlags, xFlags, ~(1 << F_AF));
    }
    SRLI(x4, x1, 63);
    PUTFLAG(x4, F_SF);
    SEQZ(x4, x1);
    PUTFLAG(x4, F_ZF);
    ANDI(xFlags, xFlags, ~(1 << F_PF));
    emit_pf(dyn, ninst, x1, x5, x6);
    BR(xRA);
    // all done!
}
