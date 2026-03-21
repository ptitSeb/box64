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
    #define MESSAGE(A, ...) do {} while (0)
    #define TABLE64C(A, V)  do { EMIT(0); EMIT(0); EMIT(0); EMIT(0); } while(0)
#elif STEP == 1
    #define EMIT(A)         do {} while (0)
    #define SETMARK(A)      jmp_df[A] = 0
    #define MESSAGE(A, ...) do {} while (0)
    #define TABLE64C(A, V)  do {} while (0)
#elif STEP == 2
    #define EMIT(A)         dyn->native_size+=4
    #define SETMARK(A)      jmp_df[A] = dyn->native_size
    #define MESSAGE(A, ...) do {} while (0)
    #define TABLE64C(A, V)  do { Table64(dyn, getConst(V), 2); EMIT(0); EMIT(0); EMIT(0); EMIT(0); } while(0)
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
    #define PPC64_LO16(x) ((int16_t)((x) & 0xFFFF))
    #define PPC64_HI16(x) ((int16_t)((((x) >> 16) + (((x) & 0x8000) ? 1 : 0)) & 0xFFFF))
    #define TABLE64C(A, V) do {                                                                 \
                int val64offset = Table64(dyn, getConst(V), 3);                                 \
                int _delta = val64offset - 4;                                                   \
                MESSAGE(LOG_DUMP, "  Table64C: 0x%lx (offset %d)\n", (V), val64offset);         \
                BCL(20, 31, 4);                                                                 \
                MFLR(A);                                                                        \
                ADDIS(A, A, PPC64_HI16(_delta));                                                \
                LD(A, PPC64_LO16(_delta), A);                                                   \
            } while(0)
#else
#error Meh!
#endif
#define STEP_PASS
#include "../dynarec_helper.h"

/*
    PPC64LE native UpdateFlags block.

    Called from dynarec when deferred flags need to be resolved.
    Entry: xEmu (r31) = x64emu_t*, xFlags (r30) = current eflags.
    LR = return address (set by caller's BCTRL).
    Reads emu->df, dispatches to native handler or falls back to C UpdateFlags().

    Scratch registers: x1(r3)..x6(r8), x7(r10).
    xFlags (r30) is modified by native handlers.
    xEmu (r31) is preserved.

    All 89 df types are handled natively.
    A fallback to C UpdateFlags() is kept as a safety net.

    IMPORTANT: We do NOT clear emu->df in the prologue.
    - Native handlers clear df after computing flags.
    - Fallback handler calls C UpdateFlags() which reads and clears df itself.
*/

void updateflags_pass(dynarec_ppc64le_t* dyn, uint64_t jmp_df[])
{
    int ninst = 0;
    rex_t rex = {0};

    // === Prologue: save caller LR, load df, bounds-check, dispatch ===

    // Save caller's return address (LR set by caller's BCTRL).
    // BCL below will clobber LR, so we must save it first.
    // x7 (r10) is a scratch register not used by emit functions.
    MFLR(x7);

    // Load df from emu struct into x1
    LWZ(x1, offsetof(x64emu_t, df), xEmu);
    // Bounds check: if df >= d_unknown, just return (bogus df)
    CMPWI(x1, d_unknown);
    BLT(12);        // if df < d_unknown, skip MTLR+BLR and go to dispatch
    MTLR(x7);       // restore caller LR before returning
    BLR();          // df >= d_unknown: return without doing anything

    // === Branch table dispatch ===
    // x1 = df (0..d_unknown-1)
    // Compute: target = table_base + df * 4
    // table_base = address of first B instruction after BCTR
    // From MFLR result (= addr of MFLR instruction):
    //   MFLR(x2)  +4 = SLWI
    //   SLWI      +4 = ADD
    //   ADD        +4 = ADDI
    //   ADDI       +4 = MTCTR
    //   MTCTR      +4 = BCTR
    //   BCTR       +4 = table[0]
    // So table[0] = x2 + 24 bytes from x2 (6 instructions * 4 bytes)
    BCL(20, 31, 4);     // LR = addr(next instruction)
    MFLR(x2);           // x2 = addr(this MFLR instruction)
    SLWI(x1, x1, 2);   // x1 = df * 4
    ADD(x1, x2, x1);   // x1 = x2 + df*4
    ADDI(x1, x1, 24);  // x1 = x2 + df*4 + 24 = &table[df]
    MTCTR(x1);
    BCTR();

    // === Branch table: one B(offset) per df type ===
    for (int i = d_none; i < d_unknown; ++i)
        B(jmp_df[i] - dyn->native_size);

    // === d_none: flags are already resolved, just return ===
SETMARK(d_none);
    MTLR(x7);
    BLR();

    // === d_cmp8 ===
SETMARK(d_cmp8);
    LBZ(x1, offsetof(x64emu_t, op1), xEmu);
    LBZ(x2, offsetof(x64emu_t, op2), xEmu);
    // Clear df before emit (emit won't clear it with gen_flags=X_ALL)
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    emit_cmp8(dyn, ninst, x1, x2, x3, x4, x5, x6);
    MTLR(x7);
    BLR();

    // === d_cmp16 ===
SETMARK(d_cmp16);
    LHZ(x1, offsetof(x64emu_t, op1), xEmu);
    LHZ(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    emit_cmp16(dyn, ninst, x1, x2, x3, x4, x5, x6);
    MTLR(x7);
    BLR();

    // === d_cmp32 (THE HOT PATH — 96.99% of all UpdateFlags calls) ===
SETMARK(d_cmp32);
    LWZ(x1, offsetof(x64emu_t, op1), xEmu);
    LWZ(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    rex.w = 0;
    emit_cmp32(dyn, ninst, rex, x1, x2, x3, x4, x5, x6);
    MTLR(x7);
    BLR();

    // === d_cmp64 ===
SETMARK(d_cmp64);
    LD(x1, offsetof(x64emu_t, op1), xEmu);
    LD(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    rex.w = 1;
    emit_cmp32(dyn, ninst, rex, x1, x2, x3, x4, x5, x6);
    rex.w = 0;
    MTLR(x7);
    BLR();

    // === d_tst8 ===
SETMARK(d_tst8);
    LBZ(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    MOV32w(x2, 0xff);
    emit_test8(dyn, ninst, x1, x2, x3, x4, x5);
    MTLR(x7);
    BLR();

    // === d_tst16 ===
SETMARK(d_tst16);
    LHZ(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    MOV32w(x2, 0xffff);
    emit_test16(dyn, ninst, x1, x2, x3, x4, x5);
    MTLR(x7);
    BLR();

    // === d_tst32 ===
SETMARK(d_tst32);
    LWZ(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    MOV32w(x2, 0xffffffff);
    rex.w = 0;
    emit_test32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    MTLR(x7);
    BLR();

    // === d_tst64 ===
SETMARK(d_tst64);
    LD(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    MOV64x(x2, 0xffffffffffffffffULL);
    rex.w = 1;
    emit_test32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    rex.w = 0;
    MTLR(x7);
    BLR();

    // ====================================================================
    // Arithmetic handlers (d_add, d_sub, d_inc, d_dec, d_neg)
    // ====================================================================

    // === d_add8 / d_add8b (aliases — same implementation) ===
SETMARK(d_add8);
SETMARK(d_add8b);
    LBZ(x1, offsetof(x64emu_t, op1), xEmu);
    LBZ(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    emit_add8(dyn, ninst, x1, x2, x3, x4);
    MTLR(x7);
    BLR();

    // === d_add16 / d_add16b ===
SETMARK(d_add16);
SETMARK(d_add16b);
    LHZ(x1, offsetof(x64emu_t, op1), xEmu);
    LHZ(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    emit_add16(dyn, ninst, x1, x2, x3, x4, x5);
    MTLR(x7);
    BLR();

    // === d_add32 / d_add32b ===
SETMARK(d_add32);
SETMARK(d_add32b);
    LWZ(x1, offsetof(x64emu_t, op1), xEmu);
    LWZ(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    rex.w = 0;
    emit_add32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    MTLR(x7);
    BLR();

    // === d_add64 ===
SETMARK(d_add64);
    LD(x1, offsetof(x64emu_t, op1), xEmu);
    LD(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    rex.w = 1;
    emit_add32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    rex.w = 0;
    MTLR(x7);
    BLR();

    // === d_sub8 ===
SETMARK(d_sub8);
    LBZ(x1, offsetof(x64emu_t, op1), xEmu);
    LBZ(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    emit_sub8(dyn, ninst, x1, x2, x3, x4, x5);
    MTLR(x7);
    BLR();

    // === d_sub16 ===
SETMARK(d_sub16);
    LHZ(x1, offsetof(x64emu_t, op1), xEmu);
    LHZ(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    emit_sub16(dyn, ninst, x1, x2, x3, x4, x5);
    MTLR(x7);
    BLR();

    // === d_sub32 ===
SETMARK(d_sub32);
    LWZ(x1, offsetof(x64emu_t, op1), xEmu);
    LWZ(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    rex.w = 0;
    emit_sub32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    MTLR(x7);
    BLR();

    // === d_sub64 ===
SETMARK(d_sub64);
    LD(x1, offsetof(x64emu_t, op1), xEmu);
    LD(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    rex.w = 1;
    emit_sub32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    rex.w = 0;
    MTLR(x7);
    BLR();

    // === d_inc8 ===
SETMARK(d_inc8);
    LBZ(x1, offsetof(x64emu_t, op1), xEmu);
    LI(x2, 0);
    STW(x2, offsetof(x64emu_t, df), xEmu);
    emit_inc8(dyn, ninst, x1, x2, x3, x4);
    MTLR(x7);
    BLR();

    // === d_inc16 ===
SETMARK(d_inc16);
    LHZ(x1, offsetof(x64emu_t, op1), xEmu);
    LI(x2, 0);
    STW(x2, offsetof(x64emu_t, df), xEmu);
    emit_inc16(dyn, ninst, x1, x2, x3, x4);
    MTLR(x7);
    BLR();

    // === d_inc32 ===
SETMARK(d_inc32);
    LWZ(x1, offsetof(x64emu_t, op1), xEmu);
    LI(x2, 0);
    STW(x2, offsetof(x64emu_t, df), xEmu);
    rex.w = 0;
    emit_inc32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    MTLR(x7);
    BLR();

    // === d_inc64 ===
SETMARK(d_inc64);
    LD(x1, offsetof(x64emu_t, op1), xEmu);
    LI(x2, 0);
    STW(x2, offsetof(x64emu_t, df), xEmu);
    rex.w = 1;
    emit_inc32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    rex.w = 0;
    MTLR(x7);
    BLR();

    // === d_dec8 ===
SETMARK(d_dec8);
    LBZ(x1, offsetof(x64emu_t, op1), xEmu);
    LI(x2, 0);
    STW(x2, offsetof(x64emu_t, df), xEmu);
    emit_dec8(dyn, ninst, x1, x2, x3, x4);
    MTLR(x7);
    BLR();

    // === d_dec16 ===
SETMARK(d_dec16);
    LHZ(x1, offsetof(x64emu_t, op1), xEmu);
    LI(x2, 0);
    STW(x2, offsetof(x64emu_t, df), xEmu);
    emit_dec16(dyn, ninst, x1, x2, x3, x4, x5);
    MTLR(x7);
    BLR();

    // === d_dec32 ===
SETMARK(d_dec32);
    LWZ(x1, offsetof(x64emu_t, op1), xEmu);
    LI(x2, 0);
    STW(x2, offsetof(x64emu_t, df), xEmu);
    rex.w = 0;
    emit_dec32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    MTLR(x7);
    BLR();

    // === d_dec64 ===
SETMARK(d_dec64);
    LD(x1, offsetof(x64emu_t, op1), xEmu);
    LI(x2, 0);
    STW(x2, offsetof(x64emu_t, df), xEmu);
    rex.w = 1;
    emit_dec32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    rex.w = 0;
    MTLR(x7);
    BLR();

    // === d_neg8 ===
SETMARK(d_neg8);
    LBZ(x1, offsetof(x64emu_t, op1), xEmu);
    LI(x2, 0);
    STW(x2, offsetof(x64emu_t, df), xEmu);
    emit_neg8(dyn, ninst, x1, x2, x3);
    MTLR(x7);
    BLR();

    // === d_neg16 ===
SETMARK(d_neg16);
    LHZ(x1, offsetof(x64emu_t, op1), xEmu);
    LI(x2, 0);
    STW(x2, offsetof(x64emu_t, df), xEmu);
    emit_neg16(dyn, ninst, x1, x2, x3);
    MTLR(x7);
    BLR();

    // === d_neg32 ===
SETMARK(d_neg32);
    LWZ(x1, offsetof(x64emu_t, op1), xEmu);
    LI(x2, 0);
    STW(x2, offsetof(x64emu_t, df), xEmu);
    rex.w = 0;
    emit_neg32(dyn, ninst, rex, x1, x2, x3);
    MTLR(x7);
    BLR();

    // === d_neg64 ===
SETMARK(d_neg64);
    LD(x1, offsetof(x64emu_t, op1), xEmu);
    LI(x2, 0);
    STW(x2, offsetof(x64emu_t, df), xEmu);
    rex.w = 1;
    emit_neg32(dyn, ninst, rex, x1, x2, x3);
    rex.w = 0;
    MTLR(x7);
    BLR();

    // ====================================================================
    // Logic handlers (d_and, d_or, d_xor)
    // All logic ops load only 'res' from emu. The second operand is a
    // constant mask: all-ones for AND, zero for OR and XOR.
    // ====================================================================

    // === d_and8 ===
SETMARK(d_and8);
    LBZ(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    MOV32w(x2, 0xff);
    emit_and8(dyn, ninst, x1, x2, x3, x4);
    MTLR(x7);
    BLR();

    // === d_and16 ===
SETMARK(d_and16);
    LHZ(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    MOV32w(x2, 0xffff);
    emit_and16(dyn, ninst, x1, x2, x3, x4);
    MTLR(x7);
    BLR();

    // === d_and32 ===
SETMARK(d_and32);
    LWZ(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    MOV32w(x2, 0xffffffff);
    rex.w = 0;
    emit_and32(dyn, ninst, rex, x1, x2, x3, x4);
    MTLR(x7);
    BLR();

    // === d_and64 ===
SETMARK(d_and64);
    LD(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    MOV64x(x2, 0xffffffffffffffffULL);
    rex.w = 1;
    emit_and32(dyn, ninst, rex, x1, x2, x3, x4);
    rex.w = 0;
    MTLR(x7);
    BLR();

    // === d_or8 ===
SETMARK(d_or8);
    LBZ(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    LI(x2, 0);
    emit_or8(dyn, ninst, x1, x2, x3, x4);
    MTLR(x7);
    BLR();

    // === d_or16 ===
SETMARK(d_or16);
    LHZ(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    LI(x2, 0);
    emit_or16(dyn, ninst, x1, x2, x3, x4);
    MTLR(x7);
    BLR();

    // === d_or32 ===
SETMARK(d_or32);
    LWZ(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    LI(x2, 0);
    rex.w = 0;
    emit_or32(dyn, ninst, rex, x1, x2, x3, x4);
    MTLR(x7);
    BLR();

    // === d_or64 ===
SETMARK(d_or64);
    LD(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    LI(x2, 0);
    rex.w = 1;
    emit_or32(dyn, ninst, rex, x1, x2, x3, x4);
    rex.w = 0;
    MTLR(x7);
    BLR();

    // === d_xor8 ===
SETMARK(d_xor8);
    LBZ(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    LI(x2, 0);
    emit_xor8(dyn, ninst, x1, x2, x3, x4);
    MTLR(x7);
    BLR();

    // === d_xor16 ===
SETMARK(d_xor16);
    LHZ(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    LI(x2, 0);
    emit_xor16(dyn, ninst, x1, x2, x3, x4, x5);
    MTLR(x7);
    BLR();

    // === d_xor32 ===
SETMARK(d_xor32);
    LWZ(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    LI(x2, 0);
    rex.w = 0;
    emit_xor32(dyn, ninst, rex, x1, x2, x3, x4);
    MTLR(x7);
    BLR();

    // === d_xor64 ===
SETMARK(d_xor64);
    LD(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    LI(x2, 0);
    rex.w = 1;
    emit_xor32(dyn, ninst, rex, x1, x2, x3, x4);
    rex.w = 0;
    MTLR(x7);
    BLR();

    // ====================================================================
    // Shift handlers (d_shl, d_shr, d_sar)
    // All shift ops load op1 and op2 (shift count).
    // SAR 8/16 loads op1 sign-extended.
    // ====================================================================

    // === d_shl8 ===
SETMARK(d_shl8);
    LBZ(x1, offsetof(x64emu_t, op1), xEmu);
    LBZ(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    emit_shl8(dyn, ninst, x1, x2, x3, x4, x5);
    MTLR(x7);
    BLR();

    // === d_shl16 ===
SETMARK(d_shl16);
    LHZ(x1, offsetof(x64emu_t, op1), xEmu);
    LHZ(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    emit_shl16(dyn, ninst, x1, x2, x3, x4, x5);
    MTLR(x7);
    BLR();

    // === d_shl32 ===
SETMARK(d_shl32);
    LWZ(x1, offsetof(x64emu_t, op1), xEmu);
    LWZ(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    rex.w = 0;
    emit_shl32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    MTLR(x7);
    BLR();

    // === d_shl64 ===
SETMARK(d_shl64);
    LD(x1, offsetof(x64emu_t, op1), xEmu);
    LD(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    rex.w = 1;
    emit_shl32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    rex.w = 0;
    MTLR(x7);
    BLR();

    // === d_shr8 ===
SETMARK(d_shr8);
    LBZ(x1, offsetof(x64emu_t, op1), xEmu);
    LBZ(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    emit_shr8(dyn, ninst, x1, x2, x3, x4, x5);
    MTLR(x7);
    BLR();

    // === d_shr16 ===
SETMARK(d_shr16);
    LHZ(x1, offsetof(x64emu_t, op1), xEmu);
    LHZ(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    emit_shr16(dyn, ninst, x1, x2, x3, x4, x5);
    MTLR(x7);
    BLR();

    // === d_shr32 ===
SETMARK(d_shr32);
    LWZ(x1, offsetof(x64emu_t, op1), xEmu);
    LWZ(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    rex.w = 0;
    emit_shr32(dyn, ninst, rex, x1, x2, x3, x4);
    MTLR(x7);
    BLR();

    // === d_shr64 ===
SETMARK(d_shr64);
    LD(x1, offsetof(x64emu_t, op1), xEmu);
    LD(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    rex.w = 1;
    emit_shr32(dyn, ninst, rex, x1, x2, x3, x4);
    rex.w = 0;
    MTLR(x7);
    BLR();

    // === d_sar8 (sign-extended op1) ===
SETMARK(d_sar8);
    LBZ(x1, offsetof(x64emu_t, op1), xEmu);
    EXTSB(x1, x1);
    LBZ(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    emit_sar8(dyn, ninst, x1, x2, x3, x4, x5);
    MTLR(x7);
    BLR();

    // === d_sar16 (sign-extended op1) ===
SETMARK(d_sar16);
    LHA(x1, offsetof(x64emu_t, op1), xEmu);
    LHZ(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    emit_sar16(dyn, ninst, x1, x2, x3, x4, x5);
    MTLR(x7);
    BLR();

    // === d_sar32 ===
SETMARK(d_sar32);
    LWZ(x1, offsetof(x64emu_t, op1), xEmu);
    LWZ(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    rex.w = 0;
    emit_sar32(dyn, ninst, rex, x1, x2, x3, x4);
    MTLR(x7);
    BLR();

    // === d_sar64 ===
SETMARK(d_sar64);
    LD(x1, offsetof(x64emu_t, op1), xEmu);
    LD(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    rex.w = 1;
    emit_sar32(dyn, ninst, rex, x1, x2, x3, x4);
    rex.w = 0;
    MTLR(x7);
    BLR();

    // ====================================================================
    // MUL handlers (d_mul8/16/32/64)
    // All fully inlined — CF/OF = high bits nonzero.
    // Optional SF/ZF/AF/PF when !BOX64ENV(cputype).
    // ====================================================================

    // === d_mul8: res is 16-bit (AX). CF/OF = (res>>8) != 0 ===
SETMARK(d_mul8);
    LHZ(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    // CF/OF = (x1 >> 8) != 0
    SRWI(x2, x1, 8);
    CMPWI(x2, 0);
    LI(x3, 1);
    // ISEL: if CR0.EQ set → RT=RA(=0); else RT=RB(=x3=1). So x3 = (high!=0)?1:0
    ISEL(x3, 0, x3, BI(0, CR_EQ));
    BF_INSERT(xFlags, x3, F_CF, F_CF);
    BF_INSERT(xFlags, x3, F_OF, F_OF);
    if(!BOX64ENV(cputype)) {
        // SF = bit 7 of res
        SRWI(x2, x1, 7);
        BF_INSERT(xFlags, x2, F_SF, F_SF);
        // ZF = 0 (undefined, but ARM64 clears it)
        BF_INSERT(xFlags, xZR, F_ZF, F_ZF);
        // AF = 0
        BF_INSERT(xFlags, xZR, F_AF, F_AF);
        // PF
        emit_pf(dyn, ninst, x1, x3, x4);
    }
    MTLR(x7);
    BLR();

    // === d_mul16: res is 32-bit (DX:AX). CF/OF = (res>>16) != 0 ===
SETMARK(d_mul16);
    LWZ(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    // CF/OF = (x1 >> 16) != 0
    SRWI(x2, x1, 16);
    CMPWI(x2, 0);
    LI(x3, 1);
    ISEL(x3, 0, x3, BI(0, CR_EQ));  // x3 = (high!=0) ? 1 : 0
    BF_INSERT(xFlags, x3, F_CF, F_CF);
    BF_INSERT(xFlags, x3, F_OF, F_OF);
    if(!BOX64ENV(cputype)) {
        SRWI(x2, x1, 15);
        BF_INSERT(xFlags, x2, F_SF, F_SF);
        BF_INSERT(xFlags, xZR, F_ZF, F_ZF);
        BF_INSERT(xFlags, xZR, F_AF, F_AF);
        emit_pf(dyn, ninst, x1, x3, x4);
    }
    MTLR(x7);
    BLR();

    // === d_mul32: op1=high32. CF/OF = (op1 != 0) ===
SETMARK(d_mul32);
    LWZ(x2, offsetof(x64emu_t, op1), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    CMPWI(x2, 0);
    LI(x3, 1);
    ISEL(x3, 0, x3, BI(0, CR_EQ));  // x3 = (op1!=0) ? 1 : 0
    BF_INSERT(xFlags, x3, F_CF, F_CF);
    BF_INSERT(xFlags, x3, F_OF, F_OF);
    if(!BOX64ENV(cputype)) {
        LWZ(x1, offsetof(x64emu_t, res), xEmu);
        SRWI(x2, x1, 31);
        BF_INSERT(xFlags, x2, F_SF, F_SF);
        BF_INSERT(xFlags, xZR, F_ZF, F_ZF);
        BF_INSERT(xFlags, xZR, F_AF, F_AF);
        emit_pf(dyn, ninst, x1, x3, x4);
    }
    MTLR(x7);
    BLR();

    // === d_mul64: op1=high64. CF/OF = (op1 != 0) ===
SETMARK(d_mul64);
    LD(x2, offsetof(x64emu_t, op1), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    CMPDI(x2, 0);
    LI(x3, 1);
    ISEL(x3, 0, x3, BI(0, CR_EQ));  // x3 = (op1!=0) ? 1 : 0
    BF_INSERT(xFlags, x3, F_CF, F_CF);
    BF_INSERT(xFlags, x3, F_OF, F_OF);
    if(!BOX64ENV(cputype)) {
        LD(x1, offsetof(x64emu_t, res), xEmu);
        SRDI(x2, x1, 63);
        BF_INSERT(xFlags, x2, F_SF, F_SF);
        BF_INSERT(xFlags, xZR, F_ZF, F_ZF);
        BF_INSERT(xFlags, xZR, F_AF, F_AF);
        emit_pf(dyn, ninst, x1, x3, x4);
    }
    MTLR(x7);
    BLR();

    // ====================================================================
    // IMUL handlers (d_imul8/16/32/64)
    // CF/OF = sign extension differs from actual high bits.
    // Optional SF/ZF/AF/PF when !BOX64ENV(cputype).
    // ====================================================================

    // === d_imul8: res is signed 16-bit. CF/OF = (ASR(res,8) != ASR(res,16)) ===
SETMARK(d_imul8);
    // Load res as signed halfword (sign-extended to 32/64-bit)
    LHA(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    // x2 = ASR(x1, 8)
    SRAWI(x2, x1, 8);
    // x4 = ASR(x1, 16)  — but x1 is already sign-extended from 16-bit
    // For a signed 16-bit value in a 32-bit register, ASR by 16 gives sign extension
    SRAWI(x4, x1, 16);
    CMPW(x2, x4);
    LI(x3, 1);
    ISEL(x3, 0, x3, BI(0, CR_EQ));  // x3 = (x2!=x4) ? 1 : 0
    BF_INSERT(xFlags, x3, F_CF, F_CF);
    BF_INSERT(xFlags, x3, F_OF, F_OF);
    if(!BOX64ENV(cputype)) {
        SRWI(x2, x1, 7);
        BF_INSERT(xFlags, x2, F_SF, F_SF);
        BF_INSERT(xFlags, xZR, F_ZF, F_ZF);
        BF_INSERT(xFlags, xZR, F_AF, F_AF);
        emit_pf(dyn, ninst, x1, x3, x4);
    }
    MTLR(x7);
    BLR();

    // === d_imul16: res is 32-bit. CF/OF = (ASR(res,16) != ASR(res,31)) ===
SETMARK(d_imul16);
    LWZ(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    SRAWI(x2, x1, 16);
    SRAWI(x4, x1, 31);
    CMPW(x2, x4);
    LI(x3, 1);
    ISEL(x3, 0, x3, BI(0, CR_EQ));
    BF_INSERT(xFlags, x3, F_CF, F_CF);
    BF_INSERT(xFlags, x3, F_OF, F_OF);
    if(!BOX64ENV(cputype)) {
        SRWI(x2, x1, 15);
        BF_INSERT(xFlags, x2, F_SF, F_SF);
        BF_INSERT(xFlags, xZR, F_ZF, F_ZF);
        BF_INSERT(xFlags, xZR, F_AF, F_AF);
        emit_pf(dyn, ninst, x1, x3, x4);
    }
    MTLR(x7);
    BLR();

    // === d_imul32: res=32-bit, op1=high32. CF/OF = (op1 != ASR(res,31)) ===
SETMARK(d_imul32);
    LWZ(x1, offsetof(x64emu_t, res), xEmu);
    LWZ(x2, offsetof(x64emu_t, op1), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    SRAWI(x4, x1, 31);
    CMPW(x2, x4);
    LI(x3, 1);
    ISEL(x3, 0, x3, BI(0, CR_EQ));
    BF_INSERT(xFlags, x3, F_CF, F_CF);
    BF_INSERT(xFlags, x3, F_OF, F_OF);
    if(!BOX64ENV(cputype)) {
        SRWI(x2, x1, 31);
        BF_INSERT(xFlags, x2, F_SF, F_SF);
        BF_INSERT(xFlags, xZR, F_ZF, F_ZF);
        BF_INSERT(xFlags, xZR, F_AF, F_AF);
        emit_pf(dyn, ninst, x1, x3, x4);
    }
    MTLR(x7);
    BLR();

    // === d_imul64: res=64-bit, op1=high64. CF/OF = (op1 != ASR(res,63)) ===
SETMARK(d_imul64);
    LD(x1, offsetof(x64emu_t, res), xEmu);
    LD(x2, offsetof(x64emu_t, op1), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    SRADI(x4, x1, 63);
    CMPD(x2, x4);
    LI(x3, 1);
    ISEL(x3, 0, x3, BI(0, CR_EQ));
    BF_INSERT(xFlags, x3, F_CF, F_CF);
    BF_INSERT(xFlags, x3, F_OF, F_OF);
    if(!BOX64ENV(cputype)) {
        SRDI(x2, x1, 63);
        BF_INSERT(xFlags, x2, F_SF, F_SF);
        BF_INSERT(xFlags, xZR, F_ZF, F_ZF);
        BF_INSERT(xFlags, xZR, F_AF, F_AF);
        emit_pf(dyn, ninst, x1, x3, x4);
    }
    MTLR(x7);
    BLR();

    // ====================================================================
    // ROL handlers (d_rol8/16/32/64)
    // Only set CF and OF. CF = bit 0 of res.
    // OF: depends on BOX64ENV(cputype).
    // ====================================================================

    // === d_rol8 ===
SETMARK(d_rol8);
    LBZ(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    if(BOX64ENV(cputype)) {
        // OF = res[0] ^ res[7]
        SRWI(x2, x1, 7);
        XOR(x2, x1, x2);
        BF_INSERT(xFlags, x2, F_OF, F_OF);
    } else {
        // OF = op1[6] ^ op1[7]  (top two bits of 8-bit value)
        LBZ(x2, offsetof(x64emu_t, op1), xEmu);
        SLWI(x3, x2, 6);          // x3 = op1 << 6 (bit7 now at bit13, bit6 at bit12)
        SRWI(x4, x3, 1);          // x4 = x3 >> 1
        XOR(x3, x3, x4);          // bit12 = bit13^bit12
        BF_INSERT(xFlags, x3, F_OF, F_OF);
    }
    // CF = bit 0 of res
    BF_INSERT(xFlags, x1, F_CF, F_CF);
    MTLR(x7);
    BLR();

    // === d_rol16 ===
SETMARK(d_rol16);
    LHZ(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    if(BOX64ENV(cputype)) {
        // OF = res[0] ^ res[15]
        SRWI(x2, x1, 15);
        XOR(x2, x1, x2);
        BF_INSERT(xFlags, x2, F_OF, F_OF);
    } else {
        LHZ(x2, offsetof(x64emu_t, op1), xEmu);
        SLWI(x3, x2, 14);
        SRWI(x4, x3, 1);
        XOR(x3, x3, x4);
        BF_INSERT(xFlags, x3, F_OF, F_OF);
    }
    BF_INSERT(xFlags, x1, F_CF, F_CF);
    MTLR(x7);
    BLR();

    // === d_rol32 ===
SETMARK(d_rol32);
    LWZ(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    if(BOX64ENV(cputype)) {
        // OF = res[0] ^ res[31]
        SRWI(x2, x1, 31);
        XOR(x2, x1, x2);
        BF_INSERT(xFlags, x2, F_OF, F_OF);
    } else {
        LWZ(x2, offsetof(x64emu_t, op1), xEmu);
        SLWI(x3, x2, 30);
        SRWI(x4, x3, 1);
        XOR(x3, x3, x4);
        BF_INSERT(xFlags, x3, F_OF, F_OF);
    }
    BF_INSERT(xFlags, x1, F_CF, F_CF);
    MTLR(x7);
    BLR();

    // === d_rol64 ===
SETMARK(d_rol64);
    LD(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    if(BOX64ENV(cputype)) {
        // OF = res[0] ^ res[63]
        SRDI(x2, x1, 63);
        XOR(x2, x1, x2);
        BF_INSERT(xFlags, x2, F_OF, F_OF);
    } else {
        LD(x2, offsetof(x64emu_t, op1), xEmu);
        SLDI(x3, x2, 62);
        SRWI(x4, x3, 1);          // only need low 32 bits for the XOR result bit
        XOR(x3, x3, x4);
        BF_INSERT(xFlags, x3, F_OF, F_OF);
    }
    BF_INSERT(xFlags, x1, F_CF, F_CF);
    MTLR(x7);
    BLR();

    // ====================================================================
    // ROR handlers (d_ror8/16/32/64)
    // Only set CF and OF. CF = MSB of res.
    // OF: depends on BOX64ENV(cputype).
    // ====================================================================

    // === d_ror8 ===
SETMARK(d_ror8);
    LBZ(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    if(BOX64ENV(cputype)) {
        // OF = res[6] ^ res[7] (using res<<6, then XOR with >>1)
        SLWI(x2, x1, 6);
        SRWI(x3, x2, 1);
        XOR(x3, x2, x3);
        BF_INSERT(xFlags, x3, F_OF, F_OF);
    } else {
        // OF = op1[0] ^ op1[7]
        LBZ(x2, offsetof(x64emu_t, op1), xEmu);
        SRWI(x3, x2, 7);
        XOR(x3, x2, x3);
        BF_INSERT(xFlags, x3, F_OF, F_OF);
    }
    // CF = bit 7 of res
    SRWI(x2, x1, 7);
    BF_INSERT(xFlags, x2, F_CF, F_CF);
    MTLR(x7);
    BLR();

    // === d_ror16 ===
SETMARK(d_ror16);
    LHZ(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    if(BOX64ENV(cputype)) {
        SLWI(x2, x1, 14);
        SRWI(x3, x2, 1);
        XOR(x3, x2, x3);
        BF_INSERT(xFlags, x3, F_OF, F_OF);
    } else {
        LHZ(x2, offsetof(x64emu_t, op1), xEmu);
        SRWI(x3, x2, 15);
        XOR(x3, x2, x3);
        BF_INSERT(xFlags, x3, F_OF, F_OF);
    }
    // CF = bit 15 of res
    SRWI(x2, x1, 15);
    BF_INSERT(xFlags, x2, F_CF, F_CF);
    MTLR(x7);
    BLR();

    // === d_ror32 ===
SETMARK(d_ror32);
    LWZ(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    if(BOX64ENV(cputype)) {
        SLWI(x2, x1, 30);
        SRWI(x3, x2, 1);
        XOR(x3, x2, x3);
        BF_INSERT(xFlags, x3, F_OF, F_OF);
    } else {
        LWZ(x2, offsetof(x64emu_t, op1), xEmu);
        SRWI(x3, x2, 31);
        XOR(x3, x2, x3);
        BF_INSERT(xFlags, x3, F_OF, F_OF);
    }
    // CF = bit 31 of res
    SRWI(x2, x1, 31);
    BF_INSERT(xFlags, x2, F_CF, F_CF);
    MTLR(x7);
    BLR();

    // === d_ror64 ===
SETMARK(d_ror64);
    LD(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    if(BOX64ENV(cputype)) {
        SLDI(x2, x1, 62);
        SRWI(x3, x2, 1);
        XOR(x3, x2, x3);
        BF_INSERT(xFlags, x3, F_OF, F_OF);
    } else {
        LD(x2, offsetof(x64emu_t, op1), xEmu);
        SRDI(x3, x2, 63);
        XOR(x3, x2, x3);
        BF_INSERT(xFlags, x3, F_OF, F_OF);
    }
    // CF = bit 63 of res
    SRDI(x2, x1, 63);
    BF_INSERT(xFlags, x2, F_CF, F_CF);
    MTLR(x7);
    BLR();

    // ====================================================================
    // SBB handlers (d_sbb8/16/32/64)
    // Fully inlined borrow chain computation.
    // CC = (~op1 & op2) | (res & (~op1 | op2))
    // CF = CC[MSB], AF = CC[3], OF = XOR2(CC >> (MSB-1))
    // ====================================================================

    // === d_sbb8 ===
SETMARK(d_sbb8);
    LBZ(x1, offsetof(x64emu_t, res), xEmu);
    LI(x5, 0);
    STW(x5, offsetof(x64emu_t, df), xEmu);
    // SF = res[7]
    SRWI(x2, x1, 7);
    BF_INSERT(xFlags, x2, F_SF, F_SF);
    // ZF = (res == 0)
    CMPWI(x1, 0);
    LI(x2, 1);
    ISEL(x2, x2, 0, BI(0, CR_EQ));
    BF_INSERT(xFlags, x2, F_ZF, F_ZF);
    // Load op1, op2
    LBZ(x2, offsetof(x64emu_t, op1), xEmu);
    LBZ(x3, offsetof(x64emu_t, op2), xEmu);
    // CC = (~op1 & op2) | (res & (~op1 | op2))
    ANDC(x4, x3, x2);     // x4 = op2 & ~op1
    ORC(x2, x3, x2);      // x2 = op2 | ~op1
    AND(x2, x2, x1);      // x2 = res & (~op1 | op2)
    OR(x2, x2, x4);       // x2 = CC
    // CF = CC[7]
    SRWI(x3, x2, 7);
    BF_INSERT(xFlags, x3, F_CF, F_CF);
    // AF = CC[3]
    SRWI(x3, x2, 3);
    BF_INSERT(xFlags, x3, F_AF, F_AF);
    // OF = XOR2(CC >> 6) = CC[6] ^ CC[7]
    SRWI(x3, x2, 6);
    SRWI(x5, x3, 1);
    XOR(x3, x3, x5);
    BF_INSERT(xFlags, x3, F_OF, F_OF);
    // PF
    emit_pf(dyn, ninst, x1, x3, x4);
    MTLR(x7);
    BLR();

    // === d_sbb16 ===
SETMARK(d_sbb16);
    LHZ(x1, offsetof(x64emu_t, res), xEmu);
    LI(x5, 0);
    STW(x5, offsetof(x64emu_t, df), xEmu);
    // SF = res[15]
    SRWI(x2, x1, 15);
    BF_INSERT(xFlags, x2, F_SF, F_SF);
    // ZF = (res == 0)
    CMPWI(x1, 0);
    LI(x2, 1);
    ISEL(x2, x2, 0, BI(0, CR_EQ));
    BF_INSERT(xFlags, x2, F_ZF, F_ZF);
    // Load op1, op2
    LHZ(x2, offsetof(x64emu_t, op1), xEmu);
    LHZ(x3, offsetof(x64emu_t, op2), xEmu);
    // CC = (~op1 & op2) | (res & (~op1 | op2))
    ANDC(x4, x3, x2);     // x4 = op2 & ~op1
    ORC(x2, x3, x2);      // x2 = op2 | ~op1
    AND(x2, x2, x1);      // x2 = res & (~op1 | op2)
    OR(x2, x2, x4);       // x2 = CC
    // CF = CC[15]
    SRWI(x3, x2, 15);
    BF_INSERT(xFlags, x3, F_CF, F_CF);
    // AF = CC[3]
    SRWI(x3, x2, 3);
    BF_INSERT(xFlags, x3, F_AF, F_AF);
    // OF = XOR2(CC >> 14) = CC[14] ^ CC[15]
    SRWI(x3, x2, 14);
    SRWI(x5, x3, 1);
    XOR(x3, x3, x5);
    BF_INSERT(xFlags, x3, F_OF, F_OF);
    // PF
    emit_pf(dyn, ninst, x1, x3, x4);
    MTLR(x7);
    BLR();

    // === d_sbb32 ===
SETMARK(d_sbb32);
    LWZ(x1, offsetof(x64emu_t, res), xEmu);
    LI(x5, 0);
    STW(x5, offsetof(x64emu_t, df), xEmu);
    // SF = res[31]
    SRWI(x2, x1, 31);
    BF_INSERT(xFlags, x2, F_SF, F_SF);
    // ZF = (res == 0)
    CMPWI(x1, 0);
    LI(x2, 1);
    ISEL(x2, x2, 0, BI(0, CR_EQ));
    BF_INSERT(xFlags, x2, F_ZF, F_ZF);
    // Load op1, op2
    LWZ(x2, offsetof(x64emu_t, op1), xEmu);
    LWZ(x3, offsetof(x64emu_t, op2), xEmu);
    // CC = (~op1 & op2) | (res & (~op1 | op2))
    ANDC(x4, x3, x2);     // x4 = op2 & ~op1
    ORC(x2, x3, x2);      // x2 = op2 | ~op1
    AND(x2, x2, x1);      // x2 = res & (~op1 | op2)
    OR(x2, x2, x4);       // x2 = CC
    // CF = CC[31]
    SRWI(x3, x2, 31);
    BF_INSERT(xFlags, x3, F_CF, F_CF);
    // AF = CC[3]
    SRWI(x3, x2, 3);
    BF_INSERT(xFlags, x3, F_AF, F_AF);
    // OF = XOR2(CC >> 30) = CC[30] ^ CC[31]
    SRWI(x3, x2, 30);
    SRWI(x5, x3, 1);
    XOR(x3, x3, x5);
    BF_INSERT(xFlags, x3, F_OF, F_OF);
    // PF
    emit_pf(dyn, ninst, x1, x3, x4);
    MTLR(x7);
    BLR();

    // === d_sbb64 ===
SETMARK(d_sbb64);
    LD(x1, offsetof(x64emu_t, res), xEmu);
    LI(x5, 0);
    STW(x5, offsetof(x64emu_t, df), xEmu);
    // SF = res[63]
    SRDI(x2, x1, 63);
    BF_INSERT(xFlags, x2, F_SF, F_SF);
    // ZF = (res == 0)
    CMPDI(x1, 0);
    LI(x2, 1);
    ISEL(x2, x2, 0, BI(0, CR_EQ));
    BF_INSERT(xFlags, x2, F_ZF, F_ZF);
    // Load op1, op2
    LD(x2, offsetof(x64emu_t, op1), xEmu);
    LD(x3, offsetof(x64emu_t, op2), xEmu);
    // CC = (~op1 & op2) | (res & (~op1 | op2))
    ANDC(x4, x3, x2);     // x4 = op2 & ~op1
    ORC(x2, x3, x2);      // x2 = op2 | ~op1
    AND(x2, x2, x1);      // x2 = res & (~op1 | op2)
    OR(x2, x2, x4);       // x2 = CC
    // CF = CC[63]
    SRDI(x3, x2, 63);
    BF_INSERT(xFlags, x3, F_CF, F_CF);
    // AF = CC[3]
    SRWI(x3, x2, 3);
    BF_INSERT(xFlags, x3, F_AF, F_AF);
    // OF = XOR2(CC >> 62) = CC[62] ^ CC[63]
    SRDI(x3, x2, 62);
    SRWI(x5, x3, 1);
    XOR(x3, x3, x5);
    BF_INSERT(xFlags, x3, F_OF, F_OF);
    // PF
    emit_pf(dyn, ninst, x1, x3, x4);
    MTLR(x7);
    BLR();

    // ====================================================================
    // ADC handlers (d_adc8/8b/16/16b/32/32b/64)
    // Carry chain: CC = (op1 & op2) | (~res & (op1 | op2))
    // Non-b variants: res stored wider (carry bit accessible directly)
    // b variants: res stored truncated, carry must be reconstructed
    // ====================================================================

    // === d_adc8: res is 16-bit (carry in bit 8) ===
SETMARK(d_adc8);
    LHZ(x1, offsetof(x64emu_t, res), xEmu);
    LI(x5, 0);
    STW(x5, offsetof(x64emu_t, df), xEmu);
    // CF = bit 8 of res
    SRWI(x2, x1, 8);
    BF_INSERT(xFlags, x2, F_CF, F_CF);
    // SF = bit 7 of res
    SRWI(x2, x1, 7);
    BF_INSERT(xFlags, x2, F_SF, F_SF);
    // ZF = (res & 0xff) == 0
    CMPWI(x1, 0);    // x1 was LBZ-range from LHZ, but low byte could be 0 with high byte nonzero
    ANDId(x5, x1, 0xff);  // x5 = x1 & 0xff, sets CR0
    LI(x2, 1);
    ISEL(x2, x2, 0, BI(0, CR_EQ));
    BF_INSERT(xFlags, x2, F_ZF, F_ZF);
    // Load op1, op2
    LBZ(x2, offsetof(x64emu_t, op1), xEmu);
    LBZ(x3, offsetof(x64emu_t, op2), xEmu);
    // CC = (op1 & op2) | (~res & (op1 | op2))
    AND(x4, x2, x3);      // x4 = op1 & op2
    OR(x2, x2, x3);       // x2 = op1 | op2
    ANDC(x2, x2, x1);     // x2 = (op1 | op2) & ~res
    OR(x2, x2, x4);       // x2 = CC
    // AF = CC[3]
    SRWI(x3, x2, 3);
    BF_INSERT(xFlags, x3, F_AF, F_AF);
    // OF = XOR2(CC >> 6)
    SRWI(x3, x2, 6);
    SRWI(x5, x3, 1);
    XOR(x3, x3, x5);
    BF_INSERT(xFlags, x3, F_OF, F_OF);
    // PF
    emit_pf(dyn, ninst, x1, x3, x4);
    MTLR(x7);
    BLR();

    // === d_adc8b: res stored as 8-bit, must reconstruct carry ===
SETMARK(d_adc8b);
    LBZ(x1, offsetof(x64emu_t, res), xEmu);
    LBZ(x2, offsetof(x64emu_t, op1), xEmu);
    LBZ(x3, offsetof(x64emu_t, op2), xEmu);
    LI(x5, 0);
    STW(x5, offsetof(x64emu_t, df), xEmu);
    // Detect carry_in: if res == (uint8_t)(op1+op2), carry_in=0, else 1
    ADD(x4, x2, x3);              // x4 = op1 + op2 (full)
    RLWINM(x5, x4, 0, 24, 31);   // x5 = (uint8_t)(op1+op2)
    CMPW(x1, x5);
    LI(x4, 0);                    // x4 = carry_in = 0
    BEQ(8);                        // skip LI if equal
    LI(x4, 1);                    // x4 = carry_in = 1
    // Recompute wider res: x1 = op1 + op2 + carry_in (16-bit)
    ADD(x1, x2, x3);
    ADD(x1, x1, x4);
    // CF = bit 8 of wider res
    SRWI(x5, x1, 8);
    BF_INSERT(xFlags, x5, F_CF, F_CF);
    // SF = bit 7 of res
    SRWI(x5, x1, 7);
    BF_INSERT(xFlags, x5, F_SF, F_SF);
    // ZF = (res & 0xff) == 0
    ANDId(x5, x1, 0xff);  // sets CR0
    LI(x5, 1);
    ISEL(x5, x5, 0, BI(0, CR_EQ));
    BF_INSERT(xFlags, x5, F_ZF, F_ZF);
    // CC = (op1 & op2) | (~res & (op1 | op2))
    AND(x4, x2, x3);      // x4 = op1 & op2
    OR(x2, x2, x3);       // x2 = op1 | op2
    ANDC(x2, x2, x1);     // x2 = (op1 | op2) & ~res
    OR(x2, x2, x4);       // x2 = CC
    // AF = CC[3]
    SRWI(x3, x2, 3);
    BF_INSERT(xFlags, x3, F_AF, F_AF);
    // OF = XOR2(CC >> 6)
    SRWI(x3, x2, 6);
    SRWI(x5, x3, 1);
    XOR(x3, x3, x5);
    BF_INSERT(xFlags, x3, F_OF, F_OF);
    // PF
    emit_pf(dyn, ninst, x1, x3, x4);
    MTLR(x7);
    BLR();

    // === d_adc16: res is 32-bit (carry in bit 16) ===
SETMARK(d_adc16);
    LWZ(x1, offsetof(x64emu_t, res), xEmu);
    LI(x5, 0);
    STW(x5, offsetof(x64emu_t, df), xEmu);
    // CF = bit 16 of res
    SRWI(x2, x1, 16);
    BF_INSERT(xFlags, x2, F_CF, F_CF);
    // SF = bit 15 of res
    SRWI(x2, x1, 15);
    BF_INSERT(xFlags, x2, F_SF, F_SF);
    // ZF = (res & 0xffff) == 0
    ANDId(x5, x1, 0xffff);  // sets CR0 — but ANDId only takes 16-bit immediate
    LI(x2, 1);
    ISEL(x2, x2, 0, BI(0, CR_EQ));
    BF_INSERT(xFlags, x2, F_ZF, F_ZF);
    // Load op1, op2
    LHZ(x2, offsetof(x64emu_t, op1), xEmu);
    LHZ(x3, offsetof(x64emu_t, op2), xEmu);
    // CC = (op1 & op2) | (~res & (op1 | op2))
    AND(x4, x2, x3);
    OR(x2, x2, x3);
    ANDC(x2, x2, x1);
    OR(x2, x2, x4);       // x2 = CC
    // AF = CC[3]
    SRWI(x3, x2, 3);
    BF_INSERT(xFlags, x3, F_AF, F_AF);
    // OF = XOR2(CC >> 14)
    SRWI(x3, x2, 14);
    SRWI(x5, x3, 1);
    XOR(x3, x3, x5);
    BF_INSERT(xFlags, x3, F_OF, F_OF);
    // PF
    emit_pf(dyn, ninst, x1, x3, x4);
    MTLR(x7);
    BLR();

    // === d_adc16b: res stored as 16-bit, must reconstruct carry ===
SETMARK(d_adc16b);
    LHZ(x1, offsetof(x64emu_t, res), xEmu);
    LHZ(x2, offsetof(x64emu_t, op1), xEmu);
    LHZ(x3, offsetof(x64emu_t, op2), xEmu);
    LI(x5, 0);
    STW(x5, offsetof(x64emu_t, df), xEmu);
    // Detect carry_in: if res == (uint16_t)(op1+op2), carry_in=0, else 1
    ADD(x4, x2, x3);                 // x4 = op1 + op2 (full)
    RLWINM(x5, x4, 0, 16, 31);      // x5 = (uint16_t)(op1+op2)
    CMPW(x1, x5);
    LI(x4, 0);
    BEQ(8);
    LI(x4, 1);
    // Recompute wider res: x1 = op1 + op2 + carry_in (32-bit)
    ADD(x1, x2, x3);
    ADD(x1, x1, x4);
    // CF = bit 16 of wider res
    SRWI(x5, x1, 16);
    BF_INSERT(xFlags, x5, F_CF, F_CF);
    // SF = bit 15 of res
    SRWI(x5, x1, 15);
    BF_INSERT(xFlags, x5, F_SF, F_SF);
    // ZF = (res & 0xffff) == 0
    ANDId(x5, x1, 0xffff);  // sets CR0
    LI(x5, 1);
    ISEL(x5, x5, 0, BI(0, CR_EQ));
    BF_INSERT(xFlags, x5, F_ZF, F_ZF);
    // CC = (op1 & op2) | (~res & (op1 | op2))
    AND(x4, x2, x3);
    OR(x2, x2, x3);
    ANDC(x2, x2, x1);
    OR(x2, x2, x4);       // x2 = CC
    // AF = CC[3]
    SRWI(x3, x2, 3);
    BF_INSERT(xFlags, x3, F_AF, F_AF);
    // OF = XOR2(CC >> 14)
    SRWI(x3, x2, 14);
    SRWI(x5, x3, 1);
    XOR(x3, x3, x5);
    BF_INSERT(xFlags, x3, F_OF, F_OF);
    // PF
    emit_pf(dyn, ninst, x1, x3, x4);
    MTLR(x7);
    BLR();

    // === d_adc32: res is 64-bit (carry in bit 32) ===
SETMARK(d_adc32);
    LD(x1, offsetof(x64emu_t, res), xEmu);
    LI(x5, 0);
    STW(x5, offsetof(x64emu_t, df), xEmu);
    // CF = bit 32 of res
    SRDI(x2, x1, 32);
    BF_INSERT(xFlags, x2, F_CF, F_CF);
    // SF = bit 31 of res
    SRWI(x2, x1, 31);
    BF_INSERT(xFlags, x2, F_SF, F_SF);
    // ZF = (uint32_t)res == 0
    ZEROUP2(x5, x1);   // x5 = (uint32_t)res
    CMPWI(x5, 0);
    LI(x2, 1);
    ISEL(x2, x2, 0, BI(0, CR_EQ));
    BF_INSERT(xFlags, x2, F_ZF, F_ZF);
    // Load op1, op2 (32-bit)
    LWZ(x2, offsetof(x64emu_t, op1), xEmu);
    LWZ(x3, offsetof(x64emu_t, op2), xEmu);
    // CC = (op1 & op2) | (~res & (op1 | op2))  — 32-bit operations
    AND(x4, x2, x3);
    OR(x2, x2, x3);
    ANDC(x2, x2, x1);     // uses low 32 bits of x1 (= truncated res)
    OR(x2, x2, x4);       // x2 = CC
    // AF = CC[3]
    SRWI(x3, x2, 3);
    BF_INSERT(xFlags, x3, F_AF, F_AF);
    // OF = XOR2(CC >> 30)
    SRWI(x3, x2, 30);
    SRWI(x5, x3, 1);
    XOR(x3, x3, x5);
    BF_INSERT(xFlags, x3, F_OF, F_OF);
    // PF
    emit_pf(dyn, ninst, x1, x3, x4);
    MTLR(x7);
    BLR();

    // === d_adc32b: res stored as 32-bit, must reconstruct carry ===
SETMARK(d_adc32b);
    LWZ(x1, offsetof(x64emu_t, res), xEmu);
    LI(x5, 0);
    STW(x5, offsetof(x64emu_t, df), xEmu);
    // SF = bit 31 of res
    SRWI(x2, x1, 31);
    BF_INSERT(xFlags, x2, F_SF, F_SF);
    // ZF = (res == 0)
    CMPWI(x1, 0);
    LI(x2, 1);
    ISEL(x2, x2, 0, BI(0, CR_EQ));
    BF_INSERT(xFlags, x2, F_ZF, F_ZF);
    // Load op1, op2
    LWZ(x2, offsetof(x64emu_t, op1), xEmu);
    LWZ(x3, offsetof(x64emu_t, op2), xEmu);
    // Detect carry_in: compare (uint32_t)res with (uint32_t)(op1+op2)
    ADD(x4, x2, x3);         // x4 = op1 + op2 (64-bit, but LWZ values fit 32 bits)
    ZEROUP(x4);               // x4 = (uint32_t)(op1+op2)
    CMPW(x1, x4);
    LI(x4, 1);
    ISEL(x4, 0, x4, BI(0, CR_EQ));  // x4 = carry_in: 0 if equal, 1 if not
    // Recompute wider: x4 = carry_in + op1 + op2 (64-bit)
    ADD(x4, x4, x2);
    ADD(x4, x4, x3);
    // CF = bit 32
    SRDI(x5, x4, 32);
    BF_INSERT(xFlags, x5, F_CF, F_CF);
    // CC = (op1 & op2) | (~res & (op1 | op2))
    AND(x4, x2, x3);
    OR(x2, x2, x3);
    ANDC(x2, x2, x1);
    OR(x2, x2, x4);       // x2 = CC
    // AF = CC[3]
    SRWI(x3, x2, 3);
    BF_INSERT(xFlags, x3, F_AF, F_AF);
    // OF = XOR2(CC >> 30)
    SRWI(x3, x2, 30);
    SRWI(x5, x3, 1);
    XOR(x3, x3, x5);
    BF_INSERT(xFlags, x3, F_OF, F_OF);
    // PF
    emit_pf(dyn, ninst, x1, x3, x4);
    MTLR(x7);
    BLR();

    // === d_adc64: must reconstruct carry via hi/lo split ===
SETMARK(d_adc64);
    LD(x1, offsetof(x64emu_t, res), xEmu);
    LI(x5, 0);
    STW(x5, offsetof(x64emu_t, df), xEmu);
    // SF = bit 63 of res
    SRDI(x2, x1, 63);
    BF_INSERT(xFlags, x2, F_SF, F_SF);
    // ZF = (res == 0)
    CMPDI(x1, 0);
    LI(x2, 1);
    ISEL(x2, x2, 0, BI(0, CR_EQ));
    BF_INSERT(xFlags, x2, F_ZF, F_ZF);
    // Load op1, op2
    LD(x2, offsetof(x64emu_t, op1), xEmu);
    LD(x3, offsetof(x64emu_t, op2), xEmu);
    // Detect carry_in: if res == op1+op2, carry_in=0, else 1
    ADD(x4, x2, x3);         // x4 = op1 + op2 (64-bit, may wrap)
    CMPD(x1, x4);
    LI(x4, 1);
    ISEL(x4, 0, x4, BI(0, CR_EQ));  // x4 = carry_in
    // Compute CF via hi/lo split:
    // lo = carry_in + (uint32_t)op1 + (uint32_t)op2
    ZEROUP2(x5, x2);         // x5 = (uint32_t)op1
    ADD(x4, x4, x5);         // x4 = carry_in + lo(op1)
    ZEROUP2(x5, x3);         // x5 = (uint32_t)op2
    ADD(x4, x4, x5);         // x4 = carry_in + lo(op1) + lo(op2) = lo
    // hi = (lo >> 32) + (op1 >> 32) + (op2 >> 32)
    SRDI(x4, x4, 32);        // x4 = lo >> 32 (carry from low half)
    SRDI(x5, x2, 32);        // x5 = op1 >> 32
    ADD(x4, x4, x5);
    SRDI(x5, x3, 32);        // x5 = op2 >> 32
    ADD(x4, x4, x5);         // x4 = hi
    // CF = bit 32 of hi
    SRDI(x5, x4, 32);
    BF_INSERT(xFlags, x5, F_CF, F_CF);
    // CC = (op1 & op2) | (~res & (op1 | op2))
    AND(x4, x2, x3);
    OR(x2, x2, x3);
    ANDC(x2, x2, x1);
    OR(x2, x2, x4);       // x2 = CC
    // AF = CC[3]
    SRWI(x3, x2, 3);
    BF_INSERT(xFlags, x3, F_AF, F_AF);
    // OF = XOR2(CC >> 62)
    SRDI(x3, x2, 62);
    SRWI(x5, x3, 1);
    XOR(x3, x3, x5);
    BF_INSERT(xFlags, x3, F_OF, F_OF);
    // PF
    emit_pf(dyn, ninst, x1, x3, x4);
    MTLR(x7);
    BLR();

    // ====================================================================
    // SHRD handlers (d_shrd16/32/64)
    // CF = (op1 >> (cnt-1)) & 1
    // OF (cputype): XOR2(res >> (N-2))
    // OF (!cputype): ((res >> (N-cnt)) ^ (op1 >> (N-1))) & 1
    // AF: SET (cputype) or CLEAR (!cputype)
    // d_shrd16: OF/AF unconditional, CF/SF/ZF/PF only when cnt>0
    // d_shrd32/64: all flags only when cnt>0
    // ====================================================================

    // === d_shrd16 ===
    // C ref: OF and AF are set unconditionally (outside if(cnt>0)).
    // CF/SF/ZF/PF only when cnt > 0.
SETMARK(d_shrd16);
    LHZ(x1, offsetof(x64emu_t, res), xEmu);
    LHZ(x2, offsetof(x64emu_t, op1), xEmu);
    LHZ(x3, offsetof(x64emu_t, op2), xEmu);
    LI(x5, 0);
    STW(x5, offsetof(x64emu_t, df), xEmu);
    // OF (unconditional)
    if(BOX64ENV(cputype)) {
        // OF = XOR2(res >> 14)
        SRWI(x4, x1, 14);
        SRWI(x5, x4, 1);
        XOR(x4, x4, x5);
        BF_INSERT(xFlags, x4, F_OF, F_OF);
        // AF = 1
        ORI(xFlags, xFlags, 1 << F_AF);
    } else {
        // OF = ((res >> (16 - (cnt & 15))) ^ (op1 >> 15)) & 1
        ANDId(x4, x3, 0x0f);     // x4 = cnt & 0xf, sets CR0
        SUBFIC(x4, x4, 16);      // x4 = 16 - (cnt & 0xf)
        SRW(x4, x1, x4);         // x4 = res >> (16 - (cnt & 0xf))
        SRWI(x5, x2, 15);        // x5 = op1 >> 15
        XOR(x4, x4, x5);
        BF_INSERT(xFlags, x4, F_OF, F_OF);
        // AF = 0
        BF_INSERT(xFlags, xZR, F_AF, F_AF);
    }
    // Early return if cnt == 0
    CMPWI(x3, 0);
    BNE(12);       // if cnt != 0, skip to CF/SF/ZF/PF
    MTLR(x7);
    BLR();
    // CF = (op1 >> (cnt - 1)) & 1
    ADDI(x4, x3, -1);         // x4 = cnt - 1
    SRW(x4, x2, x4);          // x4 = op1 >> (cnt - 1)
    if(BOX64ENV(cputype)) {
        // if cnt > 15, CF = 0
        CMPWI(x3, 15);
        BLE(8);                // if cnt <= 15, skip zeroing
        LI(x4, 0);            // CF = 0 for cnt > 15
    }
    BF_INSERT(xFlags, x4, F_CF, F_CF);
    // SF = bit 15 of res
    SRWI(x4, x1, 15);
    BF_INSERT(xFlags, x4, F_SF, F_SF);
    // ZF = (res & 0xffff) == 0
    ANDId(x4, x1, 0xffff);   // sets CR0
    LI(x4, 1);
    ISEL(x4, x4, 0, BI(0, CR_EQ));
    BF_INSERT(xFlags, x4, F_ZF, F_ZF);
    // PF
    emit_pf(dyn, ninst, x1, x4, x5);
    MTLR(x7);
    BLR();

    // === d_shrd32 ===
SETMARK(d_shrd32);
    LWZ(x1, offsetof(x64emu_t, res), xEmu);
    LWZ(x2, offsetof(x64emu_t, op1), xEmu);
    LWZ(x3, offsetof(x64emu_t, op2), xEmu);
    LI(x5, 0);
    STW(x5, offsetof(x64emu_t, df), xEmu);
    // Early return if cnt == 0
    CMPWI(x3, 0);
    BNE(12);
    MTLR(x7);
    BLR();
    // OF
    if(BOX64ENV(cputype)) {
        // OF = XOR2(res >> 30)
        SRWI(x4, x1, 30);
        SRWI(x5, x4, 1);
        XOR(x4, x4, x5);
        BF_INSERT(xFlags, x4, F_OF, F_OF);
        // AF = 1
        ORI(xFlags, xFlags, 1 << F_AF);
    } else {
        // OF = ((res >> (32 - cnt)) ^ (op1 >> 31)) & 1
        SUBFIC(x4, x3, 32);       // x4 = 32 - cnt
        SRW(x4, x1, x4);          // x4 = res >> (32 - cnt)
        SRWI(x5, x2, 31);         // x5 = op1 >> 31
        XOR(x4, x4, x5);
        BF_INSERT(xFlags, x4, F_OF, F_OF);
        // AF = 0
        BF_INSERT(xFlags, xZR, F_AF, F_AF);
    }
    // CF = (op1 >> (cnt - 1)) & 1
    ADDI(x4, x3, -1);
    SRW(x4, x2, x4);
    BF_INSERT(xFlags, x4, F_CF, F_CF);
    // SF = bit 31 of res
    SRWI(x4, x1, 31);
    BF_INSERT(xFlags, x4, F_SF, F_SF);
    // ZF = (res == 0)
    CMPWI(x1, 0);
    LI(x4, 1);
    ISEL(x4, x4, 0, BI(0, CR_EQ));
    BF_INSERT(xFlags, x4, F_ZF, F_ZF);
    // PF
    emit_pf(dyn, ninst, x1, x4, x5);
    MTLR(x7);
    BLR();

    // === d_shrd64 ===
SETMARK(d_shrd64);
    LD(x1, offsetof(x64emu_t, res), xEmu);
    LD(x2, offsetof(x64emu_t, op1), xEmu);
    LD(x3, offsetof(x64emu_t, op2), xEmu);
    LI(x5, 0);
    STW(x5, offsetof(x64emu_t, df), xEmu);
    // Early return if cnt == 0
    CMPWI(x3, 0);
    BNE(12);
    MTLR(x7);
    BLR();
    // OF
    if(BOX64ENV(cputype)) {
        // OF = XOR2(res >> 62)
        SRDI(x4, x1, 62);
        SRWI(x5, x4, 1);
        XOR(x4, x4, x5);
        BF_INSERT(xFlags, x4, F_OF, F_OF);
        // AF = 1
        ORI(xFlags, xFlags, 1 << F_AF);
    } else {
        // OF = ((res >> (64 - cnt)) ^ (op1 >> 63)) & 1
        SUBFIC(x4, x3, 64);       // x4 = 64 - cnt
        SRD(x4, x1, x4);          // x4 = res >> (64 - cnt)
        SRDI(x5, x2, 63);         // x5 = op1 >> 63
        XOR(x4, x4, x5);
        BF_INSERT(xFlags, x4, F_OF, F_OF);
        // AF = 0
        BF_INSERT(xFlags, xZR, F_AF, F_AF);
    }
    // CF = (op1 >> (cnt - 1)) & 1
    ADDI(x4, x3, -1);
    SRD(x4, x2, x4);
    BF_INSERT(xFlags, x4, F_CF, F_CF);
    // SF = bit 63 of res
    SRDI(x4, x1, 63);
    BF_INSERT(xFlags, x4, F_SF, F_SF);
    // ZF = (res == 0)
    CMPDI(x1, 0);
    LI(x4, 1);
    ISEL(x4, x4, 0, BI(0, CR_EQ));
    BF_INSERT(xFlags, x4, F_ZF, F_ZF);
    // PF
    emit_pf(dyn, ninst, x1, x4, x5);
    MTLR(x7);
    BLR();

    // ====================================================================
    // SHLD handlers (d_shld16/32/64)
    // CF = (op1 >> (N - cnt)) & 1
    // OF (cputype): (CF ^ (res >> (N-1))) & 1
    // OF (!cputype): XOR2(op1 >> (N-2))
    // AF: SET (cputype) or CLEAR (!cputype)
    // All flags only when cnt > 0
    // ====================================================================

    // === d_shld16 ===
SETMARK(d_shld16);
    LHZ(x1, offsetof(x64emu_t, res), xEmu);
    LHZ(x2, offsetof(x64emu_t, op1), xEmu);
    LHZ(x3, offsetof(x64emu_t, op2), xEmu);
    LI(x5, 0);
    STW(x5, offsetof(x64emu_t, df), xEmu);
    // Early return if cnt == 0
    CMPWI(x3, 0);
    BNE(12);
    MTLR(x7);
    BLR();
    // CF = (op1 >> (16 - cnt)) & 1
    SUBFIC(x4, x3, 16);       // x4 = 16 - cnt
    SRW(x4, x2, x4);          // x4 = op1 >> (16 - cnt)
    BF_INSERT(xFlags, x4, F_CF, F_CF);
    // OF
    if(BOX64ENV(cputype)) {
        // OF = (CF ^ (res >> 15)) & 1
        // CF was just inserted into xFlags bit 0
        // But if cnt > 15, OF = CF (i.e., res >> 15 is treated as 0? No, CSEL preserves xFlags)
        // ARM64: EORw_REG_LSR(x4, xFlags, x1, 15); CMPSw_U12(x3, 15); CSELw(x4, x4, xFlags, cGT)
        // When cnt > 15: x4 = xFlags (so OF = xFlags bit F_OF which is preserved)
        // When cnt <= 15: x4 = xFlags ^ (res >> 15), bit 0 = CF ^ sign(res)
        SRWI(x4, x1, 15);         // x4 = res >> 15
        XOR(x4, xFlags, x4);      // x4 = xFlags ^ (res >> 15), bit F_CF = CF ^ sign(res)
        CMPWI(x3, 15);
        // if cnt > 15: use xFlags (preserve existing OF)
        // if cnt <= 15: use x4
        ISEL(x4, x4, xFlags, BI(0, CR_GT));  // GT: x4=x4; else: x4=xFlags
        BF_INSERT(xFlags, x4, F_OF, F_OF);
        // AF = 1
        ORI(xFlags, xFlags, 1 << F_AF);
    } else {
        // OF = XOR2(op1 >> 14)
        SRWI(x4, x2, 14);
        SRWI(x5, x4, 1);
        XOR(x4, x4, x5);
        BF_INSERT(xFlags, x4, F_OF, F_OF);
        // AF = 0
        BF_INSERT(xFlags, xZR, F_AF, F_AF);
    }
    // SF = bit 15 of res
    SRWI(x4, x1, 15);
    BF_INSERT(xFlags, x4, F_SF, F_SF);
    // ZF = (res & 0xffff) == 0
    ANDId(x4, x1, 0xffff);   // sets CR0
    LI(x4, 1);
    ISEL(x4, x4, 0, BI(0, CR_EQ));
    BF_INSERT(xFlags, x4, F_ZF, F_ZF);
    // PF
    emit_pf(dyn, ninst, x1, x4, x5);
    MTLR(x7);
    BLR();

    // === d_shld32 ===
SETMARK(d_shld32);
    LWZ(x1, offsetof(x64emu_t, res), xEmu);
    LWZ(x2, offsetof(x64emu_t, op1), xEmu);
    LWZ(x3, offsetof(x64emu_t, op2), xEmu);
    LI(x5, 0);
    STW(x5, offsetof(x64emu_t, df), xEmu);
    // Early return if cnt == 0
    CMPWI(x3, 0);
    BNE(12);
    MTLR(x7);
    BLR();
    // CF = (op1 >> (32 - cnt)) & 1
    SUBFIC(x4, x3, 32);       // x4 = 32 - cnt
    SRW(x4, x2, x4);          // x4 = op1 >> (32 - cnt)
    BF_INSERT(xFlags, x4, F_CF, F_CF);
    // OF
    if(BOX64ENV(cputype)) {
        // OF = (CF ^ (res >> 31)) & 1
        // CF is in xFlags bit 0
        SRWI(x4, x1, 31);         // x4 = res >> 31
        XOR(x4, xFlags, x4);      // x4 = xFlags ^ (res >> 31), bit 0 = CF ^ sign(res)
        BF_INSERT(xFlags, x4, F_OF, F_OF);
        // AF = 1
        ORI(xFlags, xFlags, 1 << F_AF);
    } else {
        // OF = XOR2(op1 >> 30)
        SRWI(x4, x2, 30);
        SRWI(x5, x4, 1);
        XOR(x4, x4, x5);
        BF_INSERT(xFlags, x4, F_OF, F_OF);
        // AF = 0
        BF_INSERT(xFlags, xZR, F_AF, F_AF);
    }
    // SF = bit 31 of res
    SRWI(x4, x1, 31);
    BF_INSERT(xFlags, x4, F_SF, F_SF);
    // ZF = (res == 0)
    CMPWI(x1, 0);
    LI(x4, 1);
    ISEL(x4, x4, 0, BI(0, CR_EQ));
    BF_INSERT(xFlags, x4, F_ZF, F_ZF);
    // PF
    emit_pf(dyn, ninst, x1, x4, x5);
    MTLR(x7);
    BLR();

    // === d_shld64 ===
SETMARK(d_shld64);
    LD(x1, offsetof(x64emu_t, res), xEmu);
    LD(x2, offsetof(x64emu_t, op1), xEmu);
    LD(x3, offsetof(x64emu_t, op2), xEmu);
    LI(x5, 0);
    STW(x5, offsetof(x64emu_t, df), xEmu);
    // Early return if cnt == 0
    CMPDI(x3, 0);
    BNE(12);
    MTLR(x7);
    BLR();
    // CF = (op1 >> (64 - cnt)) & 1
    MOV32w(x4, 64);
    SUB(x4, x4, x3);          // x4 = 64 - cnt
    SRD(x4, x2, x4);          // x4 = op1 >> (64 - cnt)
    BF_INSERT(xFlags, x4, F_CF, F_CF);
    // OF
    if(BOX64ENV(cputype)) {
        // OF = (CF ^ (res >> 63)) & 1
        SRDI(x4, x1, 63);         // x4 = res >> 63
        XOR(x4, xFlags, x4);      // bit 0 = CF ^ sign(res)
        BF_INSERT(xFlags, x4, F_OF, F_OF);
        // AF = 1
        ORI(xFlags, xFlags, 1 << F_AF);
    } else {
        // OF = XOR2(op1 >> 62)
        SRDI(x4, x2, 62);
        SRWI(x5, x4, 1);
        XOR(x4, x4, x5);
        BF_INSERT(xFlags, x4, F_OF, F_OF);
        // AF = 0
        BF_INSERT(xFlags, xZR, F_AF, F_AF);
    }
    // SF = bit 63 of res
    SRDI(x4, x1, 63);
    BF_INSERT(xFlags, x4, F_SF, F_SF);
    // ZF = (res == 0)
    CMPDI(x1, 0);
    LI(x4, 1);
    ISEL(x4, x4, 0, BI(0, CR_EQ));
    BF_INSERT(xFlags, x4, F_ZF, F_ZF);
    // PF
    emit_pf(dyn, ninst, x1, x4, x5);
    MTLR(x7);
    BLR();

    // Fallback code: call C UpdateFlags(emu)

    // x7 already holds the caller's return LR (saved in prologue).
    // We need to preserve it across the C call.

    // Store current xFlags to emu->eflags so C function sees them
    STD(xFlags, offsetof(x64emu_t, eflags), xEmu);

    // Save xRIP (r9) to emu->ip — it is caller-saved and WILL be clobbered
    // by the C function call. All other x86 regs are in callee-saved r14-r29.
    STD(xRIP, offsetof(x64emu_t, ip), xEmu);

    // Create stack frame: 48 bytes (ELFv2 minimum 32 + 16 for local save)
    // Layout: SP+0=backchain, SP+8=CR, SP+16=callee LR save,
    //         SP+24=saved x7 (our caller's LR), SP+32..47=param save
    STDU(xSP, -48, xSP);       // create frame, store backchain
    STD(x7, 24, xSP);          // save caller's LR in local area

    // Load C UpdateFlags() address via TABLE64C
    TABLE64C(x6, const_updateflags);
    MTCTR(x6);

    // ELFv2 ABI: r12 = function entry address
    MFCTR(12);

    // Set up argument: r3 (A0) = emu pointer
    MV(A0, xEmu);

    // Call C UpdateFlags()
    BCTRL();

    // Reload xFlags from emu->eflags (C function updated it in memory)
    LD(xFlags, offsetof(x64emu_t, eflags), xEmu);

    // Restore xRIP (r9) from emu->ip
    LD(xRIP, offsetof(x64emu_t, ip), xEmu);

    // Restore LR and tear down stack frame
    LD(x7, 24, xSP);           // load saved caller's LR
    ADDI(xSP, xSP, 48);       // tear down frame
    MTLR(x7);                  // restore LR

    // Return to caller
    BLR();
}
