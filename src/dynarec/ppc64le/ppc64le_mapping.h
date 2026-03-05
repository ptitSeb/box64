#ifndef __PPC64LE_MAPPING_H__
#define __PPC64LE_MAPPING_H__

// PPC64LE Register Mapping Scheme (ELFv2 ABI)
//
// NOTE: The current mapping places all 16 x86_64 GPRs into callee-saved
// registers (r14-r29).  This simplifies native call ABI boundaries (no x86
// state needs saving/restoring around BCTRL) but means every wrapped native
// call requires 6 extra `mr` instructions to move x86 argument registers
// (RDI,RSI,RDX,RCX,R8,R9) into ELFv2 argument registers (r3-r8).
//
// The LA64 and RV64 dynarecs instead map x86 arg regs directly onto native
// arg regs (a0-a5) for zero-cost argument forwarding.  A future PR should
// evaluate restructuring this mapping to follow the same approach.
//
/*****************************************************************************************
reg     name     mapping      native description              Box64 description       saver
******************************************************************************************
r0      -        -            Special (literal 0 in D-form)   N/A                     -
r1      sp       native sp    Stack pointer                   N/A                     Callee
r2      toc      native toc   TOC pointer (reserved)          N/A                     -
r3      a0       x1           Argument/return value           Scratch                 Caller
r4      a1       x2           Argument                        Scratch                 Caller
r5      a2       x3           Argument                        Scratch                 Caller
r6      a3       x4           Argument                        Scratch                 Caller
r7      a4       x5           Argument                        Scratch                 Caller
r8      a5       x6           Argument                        Scratch                 Caller
r9      a6       x7           Argument                        Scratch                 Caller
r10     a7       -            Argument                        Scratch (env ptr)       Caller
r11     -        -            Scratch (env ptr / plt)         PLT scratch             Caller
r12     -        -            Scratch (func entry ptr)        scratch (caller-saved)  Caller
r13     -        -            TLS pointer (reserved)          N/A                     -
r14     s0       RAX          Saved register                  -                       Callee
r15     s1       RCX          Saved register                  -                       Callee
r16     s2       RDX          Saved register                  -                       Callee
r17     s3       RBX          Saved register                  -                       Callee
r18     s4       RSP          Saved register                  -                       Callee
r19     s5       RBP          Saved register                  -                       Callee
r20     s6       RSI          Saved register                  -                       Callee
r21     s7       RDI          Saved register                  -                       Callee
r22     s8       R8           Saved register                  -                       Callee
r23     s9       R9           Saved register                  -                       Callee
r24     s10      R10          Saved register                  -                       Callee
r25     s11      R11          Saved register                  -                       Callee
r26     s12      R12          Saved register                  -                       Callee
r27     s13      R13          Saved register                  -                       Callee
r28     s14      R14          Saved register                  -                       Callee
r29     s15      R15          Saved register                  -                       Callee
r30     s16      FLAGS        Saved register                  x86 Flags               Callee
r31     s17      xEmu         Saved register                  The Emu struct          Callee

SIMD / VMX / VSX:
vr0-vr19        Volatile (caller-saved)  - scratch/temp
vr20-vr31       Non-volatile (callee-saved) - persistent XMM cache
vs0-vs31        Overlap with FPR f0-f31 (upper halves)
vs32-vs63       Overlap with VMX vr0-vr31

Note: In little-endian mode, SIMD element ordering matches x86 — minimal swizzle needed.
******************************************************************************************/

#ifndef ASM_MAPPING

#include <stdint.h>

// x86 Register mapping (to PPC64LE GPRs)
// Using callee-saved r14-r29 for x86 registers
#define xRAX    14
#define xRCX    15
#define xRDX    16
#define xRBX    17
#define xRSP    18
#define xRBP    19
#define xRSI    20
#define xRDI    21
#define xR8     22
#define xR9     23
#define xR10    24
#define xR11    25
#define xR12    26
#define xR13    27
#define xR14    28
#define xR15    29
#define xFlags  30
#define xRIP    9       // use r9 (caller-saved, loaded at block entry)
// xSavedSP is stored in emu->xSPSave (offset 808), not in a register.
// r12 is caller-saved and gets clobbered by native calls, so it cannot
// hold xSavedSP across BCTRL. See RV64 for the same approach.

// convert a x86 register to native according to the register mapping
#define TO_NAT(A) (((uint8_t[]) { 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29 })[(A)])
#define IS_GPR(A) (((uint8_t[]) { \
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
    0, 0 \
})[(A)])

// scratch registers (caller-saved GPRs)
#define x1      3
#define x2      4
#define x3      5
#define x4      6
#define x5      7
#define x6      8
#define x7      10

#define x87pc   11      // for x87 precision control, can be scratch otherwise

// emu is r31
#define xEmu    31
// PPC64LE LR is in SPR (not a GPR), but we use r0 for BLR patterns
#define xLR     0       // r0 is special: used as literal 0 in D-form, but holds LR after mflr
// PPC64LE SP is r1
#define xSP     1
// PPC64LE has no zero register; r0 acts as 0 only in specific instruction forms
// We define xZR as 0 for compatibility, but it ONLY works in D-form load/store base
#define xZR     0

// PPC64LE ABI registers
#define A0      3
#define A1      4
#define A2      5
#define A3      6
#define A4      7
#define A5      8
#define A6      9
#define A7      10

// PPC64LE Condition Register fields (CR0-CR7)
// Each CR field has 4 bits: LT, GT, EQ, SO
#define CR_LT   0       // bit 0 of a CR field: Less Than / FP Less Than
#define CR_GT   1       // bit 1 of a CR field: Greater Than / FP Greater Than
#define CR_EQ   2       // bit 2 of a CR field: Equal / FP Equal
#define CR_SO   3       // bit 3 of a CR field: Summary Overflow / FP Unordered

// CR field numbers (for use with condition branches)
#define CR0     0
#define CR1     1
#define CR2     2
#define CR3     3
#define CR4     4
#define CR5     5
#define CR6     6
#define CR7     7

// Branch condition encodings (BO field, 5 bits)
// Simplified forms for conditional branches
#define BO_TRUE     12      // 01100 - branch if condition bit is true
#define BO_FALSE    4       // 00100 - branch if condition bit is false
#define BO_ALWAYS   20      // 10100 - branch always (unconditional)

// BI field helpers: CR field * 4 + bit within field
#define BI(cr, bit)  ((cr) * 4 + (bit))

#else

// Assembly-time register mapping for .S files
// Uses %rN prefix syntax (matches GCC output and PPC64LE GAS convention)
#define RAX     %r14
#define RCX     %r15
#define RDX     %r16
#define RBX     %r17
#define RSP     %r18
#define RBP     %r19
#define RSI     %r20
#define RDI     %r21
#define R8      %r22
#define R9      %r23
#define R10     %r24
#define R11     %r25
#define R12     %r26
#define R13     %r27
#define R14     %r28
#define R15     %r29
#define Flags   %r30
#define RIP     %r9
#define Emu     %r31
// SavedSP is no longer a register — stored in emu->xSPSave (offset 808)

#endif // ASM_MAPPING

#endif // __PPC64LE_MAPPING_H__
