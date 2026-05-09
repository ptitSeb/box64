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
#define EMIT(A)    dyn->native_size += 4
#define SETMARK(A) jmp_df[A] = 1
#elif STEP == 1
#define EMIT(A)    do {} while (0)
#define SETMARK(A) jmp_df[A] = 0
#elif STEP == 2
#define EMIT(A)    dyn->native_size += 4
#define SETMARK(A) jmp_df[A] = dyn->native_size
#elif STEP == 3
#define MESSAGE(A, ...)                                                   \
    do {                                                                  \
        if (dyn->need_dump) dynarec_log_prefix(0, LOG_NONE, __VA_ARGS__); \
    } while (0)
#define EMIT(A)                                                        \
    do {                                                               \
        if (dyn->need_dump) print_opcode(dyn, ninst, (uint32_t)(A));   \
        *(uint32_t*)(dyn->block) = (uint32_t)(A);                      \
        dyn->block += 4;                                               \
        dyn->native_size += 4;                                         \
        dyn->insts[ninst].size2 += 4;                                  \
    } while (0)
#define SETMARK(A) MESSAGE(LOG_DUMP, "Mark(%d)=%p\n", A, dyn->block)
#else
#error Meh!
#endif

#define STEP_PASS
#include "../dynarec_helper.h"

void updateflags_pass(dynarec_la64_t* dyn, uint64_t jmp_df[])
{
    int ninst = 0;
    rex_t rex = {0};

    RESTORE_EFLAGS(x2);
    LD_WU(x1, xEmu, offsetof(x64emu_t, df));
    ST_W(xZR, xEmu, offsetof(x64emu_t, df));
    SLTUI(x2, x1, d_unknown);
    BNEZ(x2, 8);
    BR(xRA);
    PCADDU12I(x2, SPLIT20(4*4));
    ADDI_D(x2, x2, SPLIT12(4*4));
    ALSL_D(x1, x1, x2, 2);
    BR(x1);
    for (int i = d_none; i < d_unknown; ++i)
        B(jmp_df[i] - dyn->native_size);

SETMARK(d_none);
    BR(xRA);

SETMARK(d_add8);
SETMARK(d_add8b);
    LD_BU(x1, xEmu, offsetof(x64emu_t, op1));
    LD_BU(x2, xEmu, offsetof(x64emu_t, op2));
    emit_add8(dyn, ninst, x1, x2, x3, x4);
    BR(xRA);

SETMARK(d_add16);
SETMARK(d_add16b);
    LD_HU(x1, xEmu, offsetof(x64emu_t, op1));
    LD_HU(x2, xEmu, offsetof(x64emu_t, op2));
    emit_add16(dyn, ninst, x1, x2, x3, x4, x5);
    BR(xRA);

SETMARK(d_add32);
SETMARK(d_add32b);
    LD_WU(x1, xEmu, offsetof(x64emu_t, op1));
    LD_WU(x2, xEmu, offsetof(x64emu_t, op2));
    rex.w = 0;
    emit_add32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    BR(xRA);

SETMARK(d_add64);
    LD_D(x1, xEmu, offsetof(x64emu_t, op1));
    LD_D(x2, xEmu, offsetof(x64emu_t, op2));
    rex.w = 1;
    emit_add32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    rex.w = 0;
    BR(xRA);

SETMARK(d_and8);
    LD_BU(x1, xEmu, offsetof(x64emu_t, res));
    MOV32w(x2, 0xff);
    emit_and8(dyn, ninst, x1, x2, x3, x4);
    BR(xRA);

SETMARK(d_and16);
    LD_HU(x1, xEmu, offsetof(x64emu_t, res));
    MOV32w(x2, 0xffff);
    emit_and16(dyn, ninst, x1, x2, x3, x4);
    BR(xRA);

SETMARK(d_and32);
    LD_WU(x1, xEmu, offsetof(x64emu_t, res));
    MOV32w(x2, 0xffffffff);
    rex.w = 0;
    emit_and32(dyn, ninst, rex, x1, x2, x3, x4);
    BR(xRA);

SETMARK(d_and64);
    LD_D(x1, xEmu, offsetof(x64emu_t, res));
    MOV64x(x2, 0xffffffffffffffffULL);
    rex.w = 1;
    emit_and32(dyn, ninst, rex, x1, x2, x3, x4);
    rex.w = 0;
    BR(xRA);

SETMARK(d_dec8);
    LD_BU(x1, xEmu, offsetof(x64emu_t, op1));
    emit_dec8(dyn, ninst, x1, x2, x3, x4);
    BR(xRA);

SETMARK(d_dec16);
    LD_HU(x1, xEmu, offsetof(x64emu_t, op1));
    emit_dec16(dyn, ninst, x1, x2, x3, x4, x5);
    BR(xRA);

SETMARK(d_dec32);
    LD_WU(x1, xEmu, offsetof(x64emu_t, op1));
    rex.w = 0;
    emit_dec32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    BR(xRA);

SETMARK(d_dec64);
    LD_D(x1, xEmu, offsetof(x64emu_t, op1));
    rex.w = 1;
    emit_dec32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    rex.w = 0;
    BR(xRA);

SETMARK(d_inc8);
    LD_BU(x1, xEmu, offsetof(x64emu_t, op1));
    emit_inc8(dyn, ninst, x1, x2, x3, x4);
    BR(xRA);

SETMARK(d_inc16);
    LD_HU(x1, xEmu, offsetof(x64emu_t, op1));
    emit_inc16(dyn, ninst, x1, x2, x3, x4);
    BR(xRA);

SETMARK(d_inc32);
    LD_WU(x1, xEmu, offsetof(x64emu_t, op1));
    rex.w = 0;
    emit_inc32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    BR(xRA);

SETMARK(d_inc64);
    LD_D(x1, xEmu, offsetof(x64emu_t, op1));
    rex.w = 1;
    emit_inc32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    rex.w = 0;
    BR(xRA);

SETMARK(d_imul8);
    LD_H(x1, xEmu, offsetof(x64emu_t, res));
    EXT_W_H(x2, x1);
    EXT_W_B(x3, x1);
    XOR(x3, x3, x2);
    SNEZ(x3, x3);
    BSTRINS_D(xFlags, x3, F_CF, F_CF);
    BSTRINS_D(xFlags, x3, F_OF, F_OF);
    SRLI_D(x2, x1, 7);
    BSTRINS_D(xFlags, x2, F_SF, F_SF);
    BSTRINS_D(xFlags, xZR, F_ZF, F_ZF);
    BSTRINS_D(xFlags, xZR, F_AF, F_AF);
    ANDI(x4, x1, 0xff);
    emit_pf(dyn, ninst, x4, x5, x6);
    SPILL_EFLAGS();
    BR(xRA);

SETMARK(d_imul16);
    LD_W(x1, xEmu, offsetof(x64emu_t, res));
    SRAI_W(x2, x1, 15);
    SRAI_W(x3, x1, 31);
    XOR(x3, x3, x2);
    SNEZ(x3, x3);
    BSTRINS_D(xFlags, x3, F_CF, F_CF);
    BSTRINS_D(xFlags, x3, F_OF, F_OF);
    SRLI_D(x2, x1, 15);
    BSTRINS_D(xFlags, x2, F_SF, F_SF);
    BSTRINS_D(xFlags, xZR, F_ZF, F_ZF);
    BSTRINS_D(xFlags, xZR, F_AF, F_AF);
    ANDI(x4, x1, 0xff);
    emit_pf(dyn, ninst, x4, x5, x6);
    SPILL_EFLAGS();
    BR(xRA);

SETMARK(d_imul32);
    LD_WU(x1, xEmu, offsetof(x64emu_t, res));
    LD_W(x2, xEmu, offsetof(x64emu_t, op1));
    SRAI_W(x3, x1, 31);
    XOR(x3, x3, x2);
    SNEZ(x3, x3);
    BSTRINS_D(xFlags, x3, F_CF, F_CF);
    BSTRINS_D(xFlags, x3, F_OF, F_OF);
    SRLI_D(x2, x1, 31);
    BSTRINS_D(xFlags, x2, F_SF, F_SF);
    BSTRINS_D(xFlags, xZR, F_ZF, F_ZF);
    BSTRINS_D(xFlags, xZR, F_AF, F_AF);
    ANDI(x4, x1, 0xff);
    emit_pf(dyn, ninst, x4, x5, x6);
    SPILL_EFLAGS();
    BR(xRA);

SETMARK(d_imul64);
    LD_D(x1, xEmu, offsetof(x64emu_t, res));
    LD_D(x2, xEmu, offsetof(x64emu_t, op1));
    SRAI_D(x3, x1, 63);
    XOR(x3, x3, x2);
    SNEZ(x3, x3);
    BSTRINS_D(xFlags, x3, F_CF, F_CF);
    BSTRINS_D(xFlags, x3, F_OF, F_OF);
    SRLI_D(x2, x1, 63);
    BSTRINS_D(xFlags, x2, F_SF, F_SF);
    BSTRINS_D(xFlags, xZR, F_ZF, F_ZF);
    BSTRINS_D(xFlags, xZR, F_AF, F_AF);
    ANDI(x4, x1, 0xff);
    emit_pf(dyn, ninst, x4, x5, x6);
    SPILL_EFLAGS();
    BR(xRA);

SETMARK(d_or8);
    LD_BU(x1, xEmu, offsetof(x64emu_t, res));
    OR(x2, xZR, xZR);
    emit_or8(dyn, ninst, x1, x2, x3, x4);
    BR(xRA);

SETMARK(d_or16);
    LD_HU(x1, xEmu, offsetof(x64emu_t, res));
    OR(x2, xZR, xZR);
    emit_or16(dyn, ninst, x1, x2, x3, x4);
    BR(xRA);

SETMARK(d_or32);
    LD_WU(x1, xEmu, offsetof(x64emu_t, res));
    OR(x2, xZR, xZR);
    rex.w = 0;
    emit_or32(dyn, ninst, rex, x1, x2, x3, x4);
    BR(xRA);

SETMARK(d_or64);
    LD_D(x1, xEmu, offsetof(x64emu_t, res));
    OR(x2, xZR, xZR);
    rex.w = 1;
    emit_or32(dyn, ninst, rex, x1, x2, x3, x4);
    rex.w = 0;
    BR(xRA);

SETMARK(d_mul8);
    LD_HU(x1, xEmu, offsetof(x64emu_t, res));
    SRLI_D(x2, x1, 8);
    SNEZ(x3, x2);
    BSTRINS_D(xFlags, x3, F_CF, F_CF);
    BSTRINS_D(xFlags, x3, F_OF, F_OF);
    SRLI_D(x2, x1, 7);
    BSTRINS_D(xFlags, x2, F_SF, F_SF);
    BSTRINS_D(xFlags, xZR, F_ZF, F_ZF);
    BSTRINS_D(xFlags, xZR, F_AF, F_AF);
    ANDI(x4, x1, 0xff);
    emit_pf(dyn, ninst, x4, x5, x6);
    SPILL_EFLAGS();
    BR(xRA);

SETMARK(d_mul16);
    LD_WU(x1, xEmu, offsetof(x64emu_t, res));
    SRLI_D(x2, x1, 16);
    SNEZ(x3, x2);
    BSTRINS_D(xFlags, x3, F_CF, F_CF);
    BSTRINS_D(xFlags, x3, F_OF, F_OF);
    SRLI_D(x2, x1, 15);
    BSTRINS_D(xFlags, x2, F_SF, F_SF);
    BSTRINS_D(xFlags, xZR, F_ZF, F_ZF);
    BSTRINS_D(xFlags, xZR, F_AF, F_AF);
    ANDI(x4, x1, 0xff);
    emit_pf(dyn, ninst, x4, x5, x6);
    SPILL_EFLAGS();
    BR(xRA);

SETMARK(d_mul32);
    LD_WU(x2, xEmu, offsetof(x64emu_t, op1));
    SNEZ(x3, x2);
    BSTRINS_D(xFlags, x3, F_CF, F_CF);
    BSTRINS_D(xFlags, x3, F_OF, F_OF);
    LD_WU(x1, xEmu, offsetof(x64emu_t, res));
    SRLI_D(x2, x1, 31);
    BSTRINS_D(xFlags, x2, F_SF, F_SF);
    BSTRINS_D(xFlags, xZR, F_ZF, F_ZF);
    BSTRINS_D(xFlags, xZR, F_AF, F_AF);
    ANDI(x4, x1, 0xff);
    emit_pf(dyn, ninst, x4, x5, x6);
    SPILL_EFLAGS();
    BR(xRA);

SETMARK(d_mul64);
    LD_D(x2, xEmu, offsetof(x64emu_t, op1));
    SNEZ(x3, x2);
    BSTRINS_D(xFlags, x3, F_CF, F_CF);
    BSTRINS_D(xFlags, x3, F_OF, F_OF);
    LD_D(x1, xEmu, offsetof(x64emu_t, res));
    SRLI_D(x2, x1, 63);
    BSTRINS_D(xFlags, x2, F_SF, F_SF);
    BSTRINS_D(xFlags, xZR, F_ZF, F_ZF);
    BSTRINS_D(xFlags, xZR, F_AF, F_AF);
    ANDI(x4, x1, 0xff);
    emit_pf(dyn, ninst, x4, x5, x6);
    SPILL_EFLAGS();
    BR(xRA);

SETMARK(d_neg8);
    LD_BU(x1, xEmu, offsetof(x64emu_t, op1));
    emit_neg8(dyn, ninst, x1, x3, x4);
    BR(xRA);

SETMARK(d_neg16);
    LD_HU(x1, xEmu, offsetof(x64emu_t, op1));
    emit_neg16(dyn, ninst, x1, x3, x4);
    BR(xRA);

SETMARK(d_neg32);
    LD_WU(x1, xEmu, offsetof(x64emu_t, op1));
    rex.w = 0;
    emit_neg32(dyn, ninst, rex, x1, x3, x4);
    BR(xRA);

SETMARK(d_neg64);
    LD_D(x1, xEmu, offsetof(x64emu_t, op1));
    rex.w = 1;
    emit_neg32(dyn, ninst, rex, x1, x3, x4);
    rex.w = 0;
    BR(xRA);

SETMARK(d_shl8);
    LD_BU(x1, xEmu, offsetof(x64emu_t, op1));
    LD_BU(x2, xEmu, offsetof(x64emu_t, op2));
    ANDI(x2, x2, 0x1f);
    BEQZ(x2, 8);
    BR(xRA);
    emit_shl8(dyn, ninst, x1, x2, x3, x4, x5);
    BR(xRA);

SETMARK(d_shl16);
    LD_HU(x1, xEmu, offsetof(x64emu_t, op1));
    LD_HU(x2, xEmu, offsetof(x64emu_t, op2));
    ANDI(x2, x2, 0x1f);
    BEQZ(x2, 8);
    BR(xRA);
    emit_shl16(dyn, ninst, x1, x2, x3, x4, x5);
    BR(xRA);

SETMARK(d_shl32);
    LD_WU(x1, xEmu, offsetof(x64emu_t, op1));
    LD_WU(x2, xEmu, offsetof(x64emu_t, op2));
    ANDI(x2, x2, 0x1f);
    BEQZ(x2, 8);
    BR(xRA);
    rex.w = 0;
    emit_shl32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    BR(xRA);

SETMARK(d_shl64);
    LD_D(x1, xEmu, offsetof(x64emu_t, op1));
    LD_D(x2, xEmu, offsetof(x64emu_t, op2));
    ANDI(x2, x2, 0x3f);
    BEQZ(x2, 8);
    BR(xRA);
    rex.w = 1;
    emit_shl32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    rex.w = 0;
    BR(xRA);

SETMARK(d_shr8);
    LD_BU(x1, xEmu, offsetof(x64emu_t, op1));
    LD_BU(x2, xEmu, offsetof(x64emu_t, op2));
    ANDI(x2, x2, 0x1f);
    BEQZ(x2, 8);
    BR(xRA);
    emit_shr8(dyn, ninst, x1, x2, x3, x4, x5);
    BR(xRA);

SETMARK(d_shr16);
    LD_HU(x1, xEmu, offsetof(x64emu_t, op1));
    LD_HU(x2, xEmu, offsetof(x64emu_t, op2));
    ANDI(x2, x2, 0x1f);
    BEQZ(x2, 8);
    BR(xRA);
    emit_shr16(dyn, ninst, x1, x2, x3, x4, x5);
    BR(xRA);

SETMARK(d_shr32);
    LD_WU(x1, xEmu, offsetof(x64emu_t, op1));
    LD_WU(x2, xEmu, offsetof(x64emu_t, op2));
    ANDI(x2, x2, 0x1f);
    BEQZ(x2, 8);
    BR(xRA);
    rex.w = 0;
    emit_shr32(dyn, ninst, rex, x1, x2, x3, x4);
    BR(xRA);

SETMARK(d_shr64);
    LD_D(x1, xEmu, offsetof(x64emu_t, op1));
    LD_D(x2, xEmu, offsetof(x64emu_t, op2));
    ANDI(x2, x2, 0x3f);
    BEQZ(x2, 8);
    BR(xRA);
    rex.w = 1;
    emit_shr32(dyn, ninst, rex, x1, x2, x3, x4);
    rex.w = 0;
    BR(xRA);

SETMARK(d_sar8);
    LD_B(x1, xEmu, offsetof(x64emu_t, op1));
    LD_BU(x2, xEmu, offsetof(x64emu_t, op2));
    ANDI(x2, x2, 0x1f);
    BEQZ(x2, 8);
    BR(xRA);
    emit_sar8(dyn, ninst, x1, x2, x3, x4, x5);
    BR(xRA);

SETMARK(d_sar16);
    LD_H(x1, xEmu, offsetof(x64emu_t, op1));
    LD_HU(x2, xEmu, offsetof(x64emu_t, op2));
    ANDI(x2, x2, 0x1f);
    BEQZ(x2, 8);
    BR(xRA);
    emit_sar16(dyn, ninst, x1, x2, x3, x4, x5);
    BR(xRA);

SETMARK(d_sar32);
    LD_W(x1, xEmu, offsetof(x64emu_t, op1));
    LD_WU(x2, xEmu, offsetof(x64emu_t, op2));
    ANDI(x2, x2, 0x1f);
    BEQZ(x2, 8);
    BR(xRA);
    rex.w = 0;
    emit_sar32(dyn, ninst, rex, x1, x2, x3, x4);
    BR(xRA);

SETMARK(d_sar64);
    LD_D(x1, xEmu, offsetof(x64emu_t, op1));
    LD_D(x2, xEmu, offsetof(x64emu_t, op2));
    ANDI(x2, x2, 0x3f);
    BEQZ(x2, 8);
    BR(xRA);
    rex.w = 1;
    emit_sar32(dyn, ninst, rex, x1, x2, x3, x4);
    rex.w = 0;
    BR(xRA);

SETMARK(d_sub8);
    LD_BU(x1, xEmu, offsetof(x64emu_t, op1));
    LD_BU(x2, xEmu, offsetof(x64emu_t, op2));
    emit_sub8(dyn, ninst, x1, x2, x3, x4, x5);
    BR(xRA);

SETMARK(d_sub16);
    LD_HU(x1, xEmu, offsetof(x64emu_t, op1));
    LD_HU(x2, xEmu, offsetof(x64emu_t, op2));
    emit_sub16(dyn, ninst, x1, x2, x3, x4, x5);
    BR(xRA);

SETMARK(d_sub32);
    LD_WU(x1, xEmu, offsetof(x64emu_t, op1));
    LD_WU(x2, xEmu, offsetof(x64emu_t, op2));
    rex.w = 0;
    emit_sub32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    BR(xRA);

SETMARK(d_sub64);
    LD_D(x1, xEmu, offsetof(x64emu_t, op1));
    LD_D(x2, xEmu, offsetof(x64emu_t, op2));
    rex.w = 1;
    emit_sub32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    rex.w = 0;
    BR(xRA);

SETMARK(d_xor8);
    LD_BU(x1, xEmu, offsetof(x64emu_t, res));
    OR(x2, xZR, xZR);
    emit_xor8(dyn, ninst, x1, x2, x3, x4);
    BR(xRA);

SETMARK(d_xor16);
    LD_HU(x1, xEmu, offsetof(x64emu_t, res));
    OR(x2, xZR, xZR);
    emit_xor16(dyn, ninst, x1, x2, x3, x4, x5);
    BR(xRA);

SETMARK(d_xor32);
    LD_WU(x1, xEmu, offsetof(x64emu_t, res));
    OR(x2, xZR, xZR);
    rex.w = 0;
    emit_xor32(dyn, ninst, rex, x1, x2, x3, x4);
    BR(xRA);

SETMARK(d_xor64);
    LD_D(x1, xEmu, offsetof(x64emu_t, res));
    OR(x2, xZR, xZR);
    rex.w = 1;
    emit_xor32(dyn, ninst, rex, x1, x2, x3, x4);
    rex.w = 0;
    BR(xRA);

SETMARK(d_cmp8);
    LD_BU(x1, xEmu, offsetof(x64emu_t, op1));
    LD_BU(x2, xEmu, offsetof(x64emu_t, op2));
    emit_cmp8(dyn, ninst, x1, x2, x3, x4, x5, x6);
    BR(xRA);

SETMARK(d_cmp16);
    LD_HU(x1, xEmu, offsetof(x64emu_t, op1));
    LD_HU(x2, xEmu, offsetof(x64emu_t, op2));
    emit_cmp16(dyn, ninst, x1, x2, x3, x4, x5, x6);
    BR(xRA);

SETMARK(d_cmp32);
    LD_WU(x1, xEmu, offsetof(x64emu_t, op1));
    LD_WU(x2, xEmu, offsetof(x64emu_t, op2));
    rex.w = 0;
    emit_cmp32(dyn, ninst, rex, x1, x2, x3, x4, x5, x6);
    BR(xRA);

SETMARK(d_cmp64);
    LD_D(x1, xEmu, offsetof(x64emu_t, op1));
    LD_D(x2, xEmu, offsetof(x64emu_t, op2));
    rex.w = 1;
    emit_cmp32(dyn, ninst, rex, x1, x2, x3, x4, x5, x6);
    rex.w = 0;
    BR(xRA);

SETMARK(d_tst8);
    LD_BU(x1, xEmu, offsetof(x64emu_t, res));
    MOV32w(x2, 0xff);
    emit_test8(dyn, ninst, x1, x2, x3, x4, x5);
    BR(xRA);

SETMARK(d_tst16);
    LD_HU(x1, xEmu, offsetof(x64emu_t, res));
    MOV32w(x2, 0xffff);
    emit_test16(dyn, ninst, x1, x2, x3, x4, x5);
    BR(xRA);

SETMARK(d_tst32);
    LD_WU(x1, xEmu, offsetof(x64emu_t, res));
    MOV32w(x2, 0xffffffff);
    rex.w = 0;
    emit_test32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    BR(xRA);

SETMARK(d_tst64);
    LD_D(x1, xEmu, offsetof(x64emu_t, res));
    MOV64x(x2, 0xffffffffffffffffULL);
    rex.w = 1;
    emit_test32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    rex.w = 0;
    BR(xRA);

SETMARK(d_adc8);
    LD_HU(x1, xEmu, offsetof(x64emu_t, res));
    SRLI_D(x2, x1, 8);
    BSTRINS_D(xFlags, x2, F_CF, F_CF);
    SRLI_D(x2, x1, 7);
    BSTRINS_D(xFlags, x2, F_SF, F_SF);
    ANDI(x2, x1, 0xff);
    SEQZ(x2, x2);
    BSTRINS_D(xFlags, x2, F_ZF, F_ZF);
    LD_BU(x2, xEmu, offsetof(x64emu_t, op1));
    LD_BU(x3, xEmu, offsetof(x64emu_t, op2));
    AND(x4, x2, x3);
    OR(x2, x2, x3);
    ANDN(x2, x2, x1);
    OR(x2, x2, x4);
    SRLI_D(x3, x2, 3);
    BSTRINS_D(xFlags, x3, F_AF, F_AF);
    SRLI_D(x3, x2, 6);
    SRLI_D(x4, x3, 1);
    XOR(x3, x3, x4);
    BSTRINS_D(xFlags, x3, F_OF, F_OF);
    ANDI(x4, x1, 0xff);
    emit_pf(dyn, ninst, x4, x5, x6);
    SPILL_EFLAGS();
    BR(xRA);

SETMARK(d_adc8b);
    LD_BU(x1, xEmu, offsetof(x64emu_t, res));
    LD_BU(x2, xEmu, offsetof(x64emu_t, op1));
    LD_BU(x3, xEmu, offsetof(x64emu_t, op2));
    ADD_D(x4, x2, x3);
    SUB_D(x4, x1, x4);
    SNEZ(x4, x4);
    ADD_D(x1, x2, x3);
    ADD_D(x1, x1, x4);
    SRLI_D(x4, x1, 8);
    BSTRINS_D(xFlags, x4, F_CF, F_CF);
    SRLI_D(x4, x1, 7);
    BSTRINS_D(xFlags, x4, F_SF, F_SF);
    ANDI(x4, x1, 0xff);
    SEQZ(x4, x4);
    BSTRINS_D(xFlags, x4, F_ZF, F_ZF);
    AND(x4, x2, x3);
    OR(x2, x2, x3);
    ANDN(x2, x2, x1);
    OR(x2, x2, x4);
    SRLI_D(x3, x2, 3);
    BSTRINS_D(xFlags, x3, F_AF, F_AF);
    SRLI_D(x3, x2, 6);
    SRLI_D(x4, x3, 1);
    XOR(x3, x3, x4);
    BSTRINS_D(xFlags, x3, F_OF, F_OF);
    ANDI(x4, x1, 0xff);
    emit_pf(dyn, ninst, x4, x5, x6);
    SPILL_EFLAGS();
    BR(xRA);

SETMARK(d_adc16);
    LD_WU(x1, xEmu, offsetof(x64emu_t, res));
    SRLI_D(x2, x1, 16);
    BSTRINS_D(xFlags, x2, F_CF, F_CF);
    SRLI_D(x2, x1, 15);
    BSTRINS_D(xFlags, x2, F_SF, F_SF);
    ANDI(x2, x1, 0xffff);
    SEQZ(x2, x2);
    BSTRINS_D(xFlags, x2, F_ZF, F_ZF);
    LD_HU(x2, xEmu, offsetof(x64emu_t, op1));
    LD_HU(x3, xEmu, offsetof(x64emu_t, op2));
    AND(x4, x2, x3);
    OR(x2, x2, x3);
    ANDN(x2, x2, x1);
    OR(x2, x2, x4);
    SRLI_D(x3, x2, 3);
    BSTRINS_D(xFlags, x3, F_AF, F_AF);
    SRLI_D(x3, x2, 14);
    SRLI_D(x4, x3, 1);
    XOR(x3, x3, x4);
    BSTRINS_D(xFlags, x3, F_OF, F_OF);
    ANDI(x4, x1, 0xff);
    emit_pf(dyn, ninst, x4, x5, x6);
    SPILL_EFLAGS();
    BR(xRA);

SETMARK(d_adc16b);
    LD_HU(x1, xEmu, offsetof(x64emu_t, res));
    LD_HU(x2, xEmu, offsetof(x64emu_t, op1));
    LD_HU(x3, xEmu, offsetof(x64emu_t, op2));
    ADD_D(x4, x2, x3);
    SUB_D(x4, x1, x4);
    SNEZ(x4, x4);
    ADD_D(x1, x2, x3);
    ADD_D(x1, x1, x4);
    SRLI_D(x4, x1, 16);
    BSTRINS_D(xFlags, x4, F_CF, F_CF);
    SRLI_D(x4, x1, 15);
    BSTRINS_D(xFlags, x4, F_SF, F_SF);
    ANDI(x4, x1, 0xffff);
    SEQZ(x4, x4);
    BSTRINS_D(xFlags, x4, F_ZF, F_ZF);
    AND(x4, x2, x3);
    OR(x2, x2, x3);
    ANDN(x2, x2, x1);
    OR(x2, x2, x4);
    SRLI_D(x3, x2, 3);
    BSTRINS_D(xFlags, x3, F_AF, F_AF);
    SRLI_D(x3, x2, 14);
    SRLI_D(x4, x3, 1);
    XOR(x3, x3, x4);
    BSTRINS_D(xFlags, x3, F_OF, F_OF);
    ANDI(x4, x1, 0xff);
    emit_pf(dyn, ninst, x4, x5, x6);
    SPILL_EFLAGS();
    BR(xRA);

SETMARK(d_adc32);
    LD_D(x1, xEmu, offsetof(x64emu_t, res));
    SRLI_D(x2, x1, 32);
    BSTRINS_D(xFlags, x2, F_CF, F_CF);
    SRLI_D(x2, x1, 31);
    BSTRINS_D(xFlags, x2, F_SF, F_SF);
    SEQZ(x2, x1);
    BSTRINS_D(xFlags, x2, F_ZF, F_ZF);
    LD_WU(x2, xEmu, offsetof(x64emu_t, op1));
    LD_WU(x3, xEmu, offsetof(x64emu_t, op2));
    AND(x4, x2, x3);
    OR(x2, x2, x3);
    ANDN(x2, x2, x1);
    OR(x2, x2, x4);
    SRLI_D(x3, x2, 3);
    BSTRINS_D(xFlags, x3, F_AF, F_AF);
    SRLI_D(x3, x2, 30);
    SRLI_D(x4, x3, 1);
    XOR(x3, x3, x4);
    BSTRINS_D(xFlags, x3, F_OF, F_OF);
    ANDI(x4, x1, 0xff);
    emit_pf(dyn, ninst, x4, x5, x6);
    SPILL_EFLAGS();
    BR(xRA);

SETMARK(d_adc32b);
    LD_WU(x1, xEmu, offsetof(x64emu_t, res));
    SRLI_D(x2, x1, 31);
    BSTRINS_D(xFlags, x2, F_SF, F_SF);
    SEQZ(x2, x1);
    BSTRINS_D(xFlags, x2, F_ZF, F_ZF);
    LD_WU(x2, xEmu, offsetof(x64emu_t, op1));
    LD_WU(x3, xEmu, offsetof(x64emu_t, op2));
    ADD_W(x4, x2, x3);
    SUB_W(x4, x1, x4);
    SNEZ(x4, x4);
    BSTRPICK_D(x5, x2, 15, 0);
    BSTRPICK_D(x6, x3, 15, 0);
    ADD_D(x4, x4, x5);
    ADD_D(x4, x4, x6);
    SRLI_D(x4, x4, 16);
    SRLI_D(x5, x2, 16);
    SRLI_D(x6, x3, 16);
    ADD_D(x4, x4, x5);
    ADD_D(x4, x4, x6);
    SRLI_D(x5, x4, 16);
    BSTRINS_D(xFlags, x5, F_CF, F_CF);
    AND(x4, x2, x3);
    OR(x2, x2, x3);
    ANDN(x2, x2, x1);
    OR(x2, x2, x4);
    SRLI_D(x3, x2, 3);
    BSTRINS_D(xFlags, x3, F_AF, F_AF);
    SRLI_D(x3, x2, 30);
    SRLI_D(x4, x3, 1);
    XOR(x3, x3, x4);
    BSTRINS_D(xFlags, x3, F_OF, F_OF);
    ANDI(x4, x1, 0xff);
    emit_pf(dyn, ninst, x4, x5, x6);
    SPILL_EFLAGS();
    BR(xRA);

SETMARK(d_adc64);
    LD_D(x1, xEmu, offsetof(x64emu_t, res));
    SRLI_D(x2, x1, 63);
    BSTRINS_D(xFlags, x2, F_SF, F_SF);
    SEQZ(x2, x1);
    BSTRINS_D(xFlags, x2, F_ZF, F_ZF);
    LD_D(x2, xEmu, offsetof(x64emu_t, op1));
    LD_D(x3, xEmu, offsetof(x64emu_t, op2));
    ADD_D(x4, x2, x3);
    SUB_D(x4, x1, x4);
    SNEZ(x4, x4);
    BSTRPICK_D(x5, x2, 31, 0);
    BSTRPICK_D(x6, x3, 31, 0);
    ADD_D(x4, x4, x5);
    ADD_D(x4, x4, x6);
    SRLI_D(x4, x4, 32);
    SRLI_D(x5, x2, 32);
    SRLI_D(x6, x3, 32);
    ADD_D(x4, x4, x5);
    ADD_D(x4, x4, x6);
    SRLI_D(x5, x4, 32);
    BSTRINS_D(xFlags, x5, F_CF, F_CF);
    AND(x4, x2, x3);
    OR(x2, x2, x3);
    ANDN(x2, x2, x1);
    OR(x2, x2, x4);
    SRLI_D(x3, x2, 3);
    BSTRINS_D(xFlags, x3, F_AF, F_AF);
    SRLI_D(x3, x2, 62);
    SRLI_D(x4, x3, 1);
    XOR(x3, x3, x4);
    BSTRINS_D(xFlags, x3, F_OF, F_OF);
    ANDI(x4, x1, 0xff);
    emit_pf(dyn, ninst, x4, x5, x6);
    SPILL_EFLAGS();
    BR(xRA);

SETMARK(d_sbb8);
    LD_BU(x1, xEmu, offsetof(x64emu_t, res));
    SRLI_D(x2, x1, 7);
    BSTRINS_D(xFlags, x2, F_SF, F_SF);
    SEQZ(x2, x1);
    BSTRINS_D(xFlags, x2, F_ZF, F_ZF);
    LD_BU(x2, xEmu, offsetof(x64emu_t, op1));
    LD_BU(x3, xEmu, offsetof(x64emu_t, op2));
    ANDN(x4, x3, x2);
    ORN(x2, x3, x2);
    AND(x2, x2, x1);
    OR(x2, x2, x4);
    SRLI_D(x3, x2, 7);
    BSTRINS_D(xFlags, x3, F_CF, F_CF);
    SRLI_D(x3, x2, 3);
    BSTRINS_D(xFlags, x3, F_AF, F_AF);
    SRLI_D(x3, x2, 6);
    SRLI_D(x4, x3, 1);
    XOR(x3, x3, x4);
    BSTRINS_D(xFlags, x3, F_OF, F_OF);
    ANDI(x4, x1, 0xff);
    emit_pf(dyn, ninst, x4, x5, x6);
    SPILL_EFLAGS();
    BR(xRA);

SETMARK(d_sbb16);
    LD_HU(x1, xEmu, offsetof(x64emu_t, res));
    SRLI_D(x2, x1, 15);
    BSTRINS_D(xFlags, x2, F_SF, F_SF);
    SEQZ(x2, x1);
    BSTRINS_D(xFlags, x2, F_ZF, F_ZF);
    LD_HU(x2, xEmu, offsetof(x64emu_t, op1));
    LD_HU(x3, xEmu, offsetof(x64emu_t, op2));
    ANDN(x4, x3, x2);
    ORN(x2, x3, x2);
    AND(x2, x2, x1);
    OR(x2, x2, x4);
    SRLI_D(x3, x2, 15);
    BSTRINS_D(xFlags, x3, F_CF, F_CF);
    SRLI_D(x3, x2, 3);
    BSTRINS_D(xFlags, x3, F_AF, F_AF);
    SRLI_D(x3, x2, 14);
    SRLI_D(x4, x3, 1);
    XOR(x3, x3, x4);
    BSTRINS_D(xFlags, x3, F_OF, F_OF);
    ANDI(x4, x1, 0xff);
    emit_pf(dyn, ninst, x4, x5, x6);
    SPILL_EFLAGS();
    BR(xRA);

SETMARK(d_sbb32);
    LD_WU(x1, xEmu, offsetof(x64emu_t, res));
    SRLI_D(x2, x1, 31);
    BSTRINS_D(xFlags, x2, F_SF, F_SF);
    SEQZ(x2, x1);
    BSTRINS_D(xFlags, x2, F_ZF, F_ZF);
    LD_WU(x2, xEmu, offsetof(x64emu_t, op1));
    LD_WU(x3, xEmu, offsetof(x64emu_t, op2));
    ANDN(x4, x3, x2);
    ORN(x2, x3, x2);
    AND(x2, x2, x1);
    OR(x2, x2, x4);
    SRLI_D(x3, x2, 31);
    BSTRINS_D(xFlags, x3, F_CF, F_CF);
    SRLI_D(x3, x2, 3);
    BSTRINS_D(xFlags, x3, F_AF, F_AF);
    SRLI_D(x3, x2, 30);
    SRLI_D(x4, x3, 1);
    XOR(x3, x3, x4);
    BSTRINS_D(xFlags, x3, F_OF, F_OF);
    ANDI(x4, x1, 0xff);
    emit_pf(dyn, ninst, x4, x5, x6);
    SPILL_EFLAGS();
    BR(xRA);

SETMARK(d_sbb64);
    LD_D(x1, xEmu, offsetof(x64emu_t, res));
    SRLI_D(x2, x1, 63);
    BSTRINS_D(xFlags, x2, F_SF, F_SF);
    SEQZ(x2, x1);
    BSTRINS_D(xFlags, x2, F_ZF, F_ZF);
    LD_D(x2, xEmu, offsetof(x64emu_t, op1));
    LD_D(x3, xEmu, offsetof(x64emu_t, op2));
    ANDN(x4, x3, x2);
    ORN(x2, x3, x2);
    AND(x2, x2, x1);
    OR(x2, x2, x4);
    SRLI_D(x3, x2, 63);
    BSTRINS_D(xFlags, x3, F_CF, F_CF);
    SRLI_D(x3, x2, 3);
    BSTRINS_D(xFlags, x3, F_AF, F_AF);
    SRLI_D(x3, x2, 62);
    SRLI_D(x4, x3, 1);
    XOR(x3, x3, x4);
    BSTRINS_D(xFlags, x3, F_OF, F_OF);
    ANDI(x4, x1, 0xff);
    emit_pf(dyn, ninst, x4, x5, x6);
    SPILL_EFLAGS();
    BR(xRA);

SETMARK(d_rol8);
    LD_BU(x1, xEmu, offsetof(x64emu_t, res));
    LD_BU(x2, xEmu, offsetof(x64emu_t, op1));
    SRLI_D(x3, x2, 6);
    SRLI_D(x4, x3, 1);
    XOR(x3, x3, x4);
    BSTRINS_D(xFlags, x3, F_OF, F_OF);
    BSTRINS_D(xFlags, x1, F_CF, F_CF);
    SPILL_EFLAGS();
    BR(xRA);

SETMARK(d_rol16);
    LD_HU(x1, xEmu, offsetof(x64emu_t, res));
    LD_HU(x2, xEmu, offsetof(x64emu_t, op1));
    SRLI_D(x3, x2, 14);
    SRLI_D(x4, x3, 1);
    XOR(x3, x3, x4);
    BSTRINS_D(xFlags, x3, F_OF, F_OF);
    BSTRINS_D(xFlags, x1, F_CF, F_CF);
    SPILL_EFLAGS();
    BR(xRA);

SETMARK(d_rol32);
    LD_WU(x1, xEmu, offsetof(x64emu_t, res));
    LD_WU(x2, xEmu, offsetof(x64emu_t, op1));
    SRLI_D(x3, x2, 30);
    SRLI_D(x4, x3, 1);
    XOR(x3, x3, x4);
    BSTRINS_D(xFlags, x3, F_OF, F_OF);
    BSTRINS_D(xFlags, x1, F_CF, F_CF);
    SPILL_EFLAGS();
    BR(xRA);

SETMARK(d_rol64);
    LD_D(x1, xEmu, offsetof(x64emu_t, res));
    LD_D(x2, xEmu, offsetof(x64emu_t, op1));
    SRLI_D(x3, x2, 62);
    SRLI_D(x4, x3, 1);
    XOR(x3, x3, x4);
    BSTRINS_D(xFlags, x3, F_OF, F_OF);
    BSTRINS_D(xFlags, x1, F_CF, F_CF);
    SPILL_EFLAGS();
    BR(xRA);

SETMARK(d_ror8);
    LD_BU(x1, xEmu, offsetof(x64emu_t, res));
    LD_BU(x2, xEmu, offsetof(x64emu_t, op1));
    SRLI_D(x3, x2, 7);
    XOR(x3, x3, x2);
    BSTRINS_D(xFlags, x3, F_OF, F_OF);
    SRLI_D(x2, x1, 7);
    BSTRINS_D(xFlags, x2, F_CF, F_CF);
    SPILL_EFLAGS();
    BR(xRA);

SETMARK(d_ror16);
    LD_HU(x1, xEmu, offsetof(x64emu_t, res));
    LD_HU(x2, xEmu, offsetof(x64emu_t, op1));
    SRLI_D(x3, x2, 15);
    XOR(x3, x3, x2);
    BSTRINS_D(xFlags, x3, F_OF, F_OF);
    SRLI_D(x2, x1, 15);
    BSTRINS_D(xFlags, x2, F_CF, F_CF);
    SPILL_EFLAGS();
    BR(xRA);

SETMARK(d_ror32);
    LD_WU(x1, xEmu, offsetof(x64emu_t, res));
    LD_WU(x2, xEmu, offsetof(x64emu_t, op1));
    SRLI_D(x3, x2, 31);
    XOR(x3, x3, x2);
    BSTRINS_D(xFlags, x3, F_OF, F_OF);
    SRLI_D(x2, x1, 31);
    BSTRINS_D(xFlags, x2, F_CF, F_CF);
    SPILL_EFLAGS();
    BR(xRA);

SETMARK(d_ror64);
    LD_D(x1, xEmu, offsetof(x64emu_t, res));
    LD_D(x2, xEmu, offsetof(x64emu_t, op1));
    SRLI_D(x3, x2, 63);
    XOR(x3, x3, x2);
    BSTRINS_D(xFlags, x3, F_OF, F_OF);
    SRLI_D(x2, x1, 63);
    BSTRINS_D(xFlags, x2, F_CF, F_CF);
    SPILL_EFLAGS();
    BR(xRA);

SETMARK(d_shrd16);
    LD_HU(x1, xEmu, offsetof(x64emu_t, res));
    LD_HU(x2, xEmu, offsetof(x64emu_t, op1));
    LD_HU(x3, xEmu, offsetof(x64emu_t, op2));
    BNEZ(x3, 8);
    SPILL_EFLAGS();
    BR(xRA);
    ANDI(x4, x3, 0x0f);
    ADDI_D(x4, x4, -16);
    SUB_D(x4, xZR, x4);
    SRL_D(x4, x1, x4);
    SRLI_D(x5, x2, 15);
    XOR(x4, x4, x5);
    BSTRINS_D(xFlags, x4, F_OF, F_OF);
    BSTRINS_D(xFlags, xZR, F_AF, F_AF);
    ADDI_D(x4, x3, -1);
    SRL_D(x4, x2, x4);
    BSTRINS_D(xFlags, x4, F_CF, F_CF);
    SRLI_D(x4, x1, 15);
    BSTRINS_D(xFlags, x4, F_SF, F_SF);
    SEQZ(x4, x1);
    BSTRINS_D(xFlags, x4, F_ZF, F_ZF);
    ANDI(x4, x1, 0xff);
    emit_pf(dyn, ninst, x4, x5, x6);
    SPILL_EFLAGS();
    BR(xRA);

SETMARK(d_shrd32);
    LD_WU(x1, xEmu, offsetof(x64emu_t, res));
    LD_WU(x2, xEmu, offsetof(x64emu_t, op1));
    LD_WU(x3, xEmu, offsetof(x64emu_t, op2));
    BNEZ(x3, 8);
    SPILL_EFLAGS();
    BR(xRA);
    ADDI_D(x4, x3, -32);
    SUB_D(x4, xZR, x4);
    SRL_D(x4, x1, x4);
    SRLI_D(x5, x2, 31);
    XOR(x4, x4, x5);
    BSTRINS_D(xFlags, x4, F_OF, F_OF);
    BSTRINS_D(xFlags, xZR, F_AF, F_AF);
    ADDI_D(x4, x3, -1);
    SRL_D(x4, x2, x4);
    BSTRINS_D(xFlags, x4, F_CF, F_CF);
    SRLI_D(x4, x1, 31);
    BSTRINS_D(xFlags, x4, F_SF, F_SF);
    SEQZ(x4, x1);
    BSTRINS_D(xFlags, x4, F_ZF, F_ZF);
    ANDI(x4, x1, 0xff);
    emit_pf(dyn, ninst, x4, x5, x6);
    SPILL_EFLAGS();
    BR(xRA);

SETMARK(d_shrd64);
    LD_D(x1, xEmu, offsetof(x64emu_t, res));
    LD_D(x2, xEmu, offsetof(x64emu_t, op1));
    LD_D(x3, xEmu, offsetof(x64emu_t, op2));
    BNEZ(x3, 8);
    SPILL_EFLAGS();
    BR(xRA);
    ADDI_D(x4, x3, -64);
    SUB_D(x4, xZR, x4);
    SRL_D(x4, x1, x4);
    SRLI_D(x5, x2, 63);
    XOR(x4, x4, x5);
    BSTRINS_D(xFlags, x4, F_OF, F_OF);
    BSTRINS_D(xFlags, xZR, F_AF, F_AF);
    ADDI_D(x4, x3, -1);
    SRL_D(x4, x2, x4);
    BSTRINS_D(xFlags, x4, F_CF, F_CF);
    SRLI_D(x4, x1, 63);
    BSTRINS_D(xFlags, x4, F_SF, F_SF);
    SEQZ(x4, x1);
    BSTRINS_D(xFlags, x4, F_ZF, F_ZF);
    ANDI(x4, x1, 0xff);
    emit_pf(dyn, ninst, x4, x5, x6);
    SPILL_EFLAGS();
    BR(xRA);

SETMARK(d_shld16);
    LD_HU(x1, xEmu, offsetof(x64emu_t, res));
    LD_HU(x2, xEmu, offsetof(x64emu_t, op1));
    LD_HU(x3, xEmu, offsetof(x64emu_t, op2));
    BNEZ(x3, 8);
    SPILL_EFLAGS();
    BR(xRA);
    ADDI_D(x4, x3, -16);
    SUB_D(x4, xZR, x4);
    SRL_D(x4, x2, x4);
    BSTRINS_D(xFlags, x4, F_CF, F_CF);
    SRLI_D(x4, x2, 14);
    SRLI_D(x5, x4, 1);
    XOR(x4, x4, x5);
    BSTRINS_D(xFlags, x4, F_OF, F_OF);
    BSTRINS_D(xFlags, xZR, F_AF, F_AF);
    SRLI_D(x4, x1, 15);
    BSTRINS_D(xFlags, x4, F_SF, F_SF);
    SEQZ(x4, x1);
    BSTRINS_D(xFlags, x4, F_ZF, F_ZF);
    ANDI(x4, x1, 0xff);
    emit_pf(dyn, ninst, x4, x5, x6);
    SPILL_EFLAGS();
    BR(xRA);

SETMARK(d_shld32);
    LD_WU(x1, xEmu, offsetof(x64emu_t, res));
    LD_WU(x2, xEmu, offsetof(x64emu_t, op1));
    LD_WU(x3, xEmu, offsetof(x64emu_t, op2));
    BNEZ(x3, 8);
    SPILL_EFLAGS();
    BR(xRA);
    ADDI_D(x4, x3, -32);
    SUB_D(x4, xZR, x4);
    SRL_D(x4, x2, x4);
    BSTRINS_D(xFlags, x4, F_CF, F_CF);
    SRLI_D(x4, x2, 30);
    SRLI_D(x5, x4, 1);
    XOR(x4, x4, x5);
    BSTRINS_D(xFlags, x4, F_OF, F_OF);
    BSTRINS_D(xFlags, xZR, F_AF, F_AF);
    SRLI_D(x4, x1, 31);
    BSTRINS_D(xFlags, x4, F_SF, F_SF);
    SEQZ(x4, x1);
    BSTRINS_D(xFlags, x4, F_ZF, F_ZF);
    ANDI(x4, x1, 0xff);
    emit_pf(dyn, ninst, x4, x5, x6);
    SPILL_EFLAGS();
    BR(xRA);

SETMARK(d_shld64);
    LD_D(x1, xEmu, offsetof(x64emu_t, res));
    LD_D(x2, xEmu, offsetof(x64emu_t, op1));
    LD_D(x3, xEmu, offsetof(x64emu_t, op2));
    BNEZ(x3, 8);
    SPILL_EFLAGS();
    BR(xRA);
    MOV32w(x4, 64);
    SUB_D(x4, x4, x3);
    SRL_D(x4, x2, x4);
    BSTRINS_D(xFlags, x4, F_CF, F_CF);
    SRLI_D(x4, x2, 62);
    SRLI_D(x5, x4, 1);
    XOR(x4, x4, x5);
    BSTRINS_D(xFlags, x4, F_OF, F_OF);
    BSTRINS_D(xFlags, xZR, F_AF, F_AF);
    SRLI_D(x4, x1, 63);
    BSTRINS_D(xFlags, x4, F_SF, F_SF);
    SEQZ(x4, x1);
    BSTRINS_D(xFlags, x4, F_ZF, F_ZF);
    ANDI(x4, x1, 0xff);
    emit_pf(dyn, ninst, x4, x5, x6);
    SPILL_EFLAGS();
    BR(xRA);
}
