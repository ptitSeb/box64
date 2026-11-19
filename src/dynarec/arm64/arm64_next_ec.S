// SPDX-License-Identifier: MIT
// ARM64EC update linker table for dynarec
// called with emu in x0 and the x64 target in x1 (and xRIP)

#include "box64ec_layout.h"

.text
.align 4

.extern LinkNext
.extern LinkNextInvalid
.extern Box64EC_RequestExitToNative
.extern Box64EC_ApplyLiveMxcsr
.extern Box64EC_CaptureLiveMxcsr
.extern process_pending_cross_process_work
.extern Box64EC_EcCodeBitmapLimit
.extern ExitFunctionEC
.extern arm64_epilog_restore
.extern box64_ec_jmptable48

#ifdef SAVE_MEM
.equ JMPTABL_SHIFT3, 16
.equ JMPTABL_SHIFT2, 12
.equ JMPTABL_SHIFT1, 12
.equ JMPTABL_SHIFT0, 10
#else
.equ JMPTABL_SHIFT2, 16
.equ JMPTABL_SHIFT1, 18
.equ JMPTABL_SHIFT0, 14
#endif
.equ JMPTABL_START0, 0
.equ JMPTABL_START1, (JMPTABL_START0 + JMPTABL_SHIFT0)
.equ JMPTABL_START2, (JMPTABL_START1 + JMPTABL_SHIFT1)
#ifdef SAVE_MEM
.equ JMPTABL_START3, (JMPTABL_START2 + JMPTABL_SHIFT2)
#endif

.global arm64_next
.global arm64_next_invalid
.global arm64_ec_native_exit_direct
.global arm64_ec_fast_forward
.global arm64_ec_fast_forward_direct
.global arm64_ec_fast_resume

/* Spill the guest registers of the JIT map to emu (x0) */
.macro save_guest_regs
    stp     x10, x11, [x0, (8 *  0)]     /* RAX, RCX */
    stp     x12, x8,  [x0, (8 *  2)]     /* RDX, RBX */
    stp     x20, x21, [x0, (8 *  4)]     /* RSP, RBP */
    stp     x16, x17, [x0, (8 *  6)]     /* RSI, RDI */
    stp     x15, x6,  [x0, (8 *  8)]     /* R8, R9 */
    stp     x23, x9,  [x0, (8 * 10)]     /* R10, R11 */
    stp     x22, x25, [x0, (8 * 12)]     /* R12, R13 */
    stp     x26, x19, [x0, (8 * 14)]     /* R14, R15 */
    stp     x7,  x27, [x0, (8 * 16)]     /* Flags, RIP */
.endm

.macro next_linker name, linker
\name:
    stp     x29, x30, [sp, #-16]!
    sub     sp,  sp,  (8 * 16)
    /* the guest registers in caller-saved hosts, and RIP that the linker can change */
    stp     x0,  x1,  [sp, (8 *  0)]
    stp     x10, x11, [sp, (8 *  2)]
    stp     x12, x8,  [sp, (8 *  4)]
    stp     x23, x9,  [sp, (8 *  6)]     /* R10, R11 */
    stp     x16, x17, [sp, (8 *  8)]
    stp     x15, x7,  [sp, (8 * 10)]     /* R8, Flags */
    stp     x6,  x27, [sp, (8 * 12)]     /* R9, RIP */
    /* The linker can build a block, which can fault: a fault there is raised
     * to the guest from the emu state, so it must be complete. */
    stp     x10, x11, [x0, (8 *  0)]     /* RAX, RCX */
    stp     x12, x8,  [x0, (8 *  2)]     /* RDX, RBX */
    stp     x20, x21, [x0, (8 *  4)]     /* RSP, RBP */
    stp     x16, x17, [x0, (8 *  6)]     /* RSI, RDI */
    stp     x15, x6,  [x0, (8 *  8)]     /* R8, R9 */
    stp     x23, x9,  [x0, (8 * 10)]     /* R10, R11 */
    stp     x22, x25, [x0, (8 * 12)]     /* R12, R13 */
    stp     x26, x19, [x0, (8 * 14)]     /* R14, R15 */
    stp     x7,  x27, [x0, (8 * 16)]     /* Flags, RIP */
    /* nested activations start below it, and Wine's context conversions zero x28 */
    str     x28, [x0, #BOX64EC_EMU_CALLRET_SP_OFFSET]
    mov     x1, x27                      /* xRIP */
    mov     x2, x30                      /* "from" is in lr */
    add     x3, sp, (8 * 13)             /* address to change RIP */
    bl      \linker
    mov     x3, x0
    ldp     x0,  x1,  [sp, (8 *  0)]
    ldr     x28, [x0, #BOX64EC_EMU_CALLRET_SP_OFFSET]
    ldp     x10, x11, [sp, (8 *  2)]
    ldp     x12, x8,  [sp, (8 *  4)]
    ldp     x23, x9,  [sp, (8 *  6)]
    ldp     x16, x17, [sp, (8 *  8)]
    ldp     x15, x7,  [sp, (8 * 10)]
    ldp     x6,  x27, [sp, (8 * 12)]
    add     sp,  sp,  (8 * 16)
    ldp     x29, x30, [sp], #16
    // return offset is jump address
    br      x3
\name\()_end:
.endm

    .8byte  0   // NULL pointer before arm64_next, for getDB
next_linker arm64_next, LinkNext
next_linker arm64_next_invalid, LinkNextInvalid

/* Jump table entry of a native target: leave the JIT without LinkNext */
    .8byte  0   // NULL pointer before the entry, for getDB
arm64_ec_native_exit_direct:
    stp     x29, x30, [sp, #-16]!
    sub     sp,  sp,  #16
    save_guest_regs
    str     x28, [x0, #BOX64EC_EMU_CALLRET_SP_OFFSET]
    str     x0,  [sp]
    mov     x1,  x27                     /* xRIP */
    bl      Box64EC_RequestExitToNative
    ldr     x0,  [sp]
    add     sp,  sp,  #16
    ldp     x29, x30, [sp], #16
    b       arm64_epilog_restore         /* the helper clobbered the guest registers */
arm64_ec_native_exit_direct_end:

/* Call a native function without leaving the JIT: a tagged record on the CALLRET
 * stack marks the call, RetToEntryThunk matches it and resumes at arm64_ec_fast_resume. */
    .8byte  0   // NULL pointer before the entry, for getDB
arm64_ec_fast_forward_direct:
    save_guest_regs
arm64_ec_fast_forward_direct_end:
arm64_ec_fast_forward:
    /* every call: x64 alignment, return address not EC code, nothing pending */
    tbz     x20, #3, arm64_ec_native_exit_direct
    ldr     x1, [x20]
    adrp    x2, Box64EC_EcCodeBitmapLimit
    ldr     x2, [x2, :lo12:Box64EC_EcCodeBitmapLimit]
    cmp     x1, x2
    b.hs    arm64_ec_native_exit_direct
    ldr     x2, [x18, #0x60]             /* PEB */
    cbz     x2, arm64_ec_native_exit_direct
    ldr     x2, [x2, #0x368]             /* EcCodeBitMap */
    cbz     x2, arm64_ec_native_exit_direct
    lsr     x3, x1, #18
    ldr     x2, [x2, x3, lsl #3]
    lsr     x3, x1, #12
    lsr     x2, x2, x3
    tbnz    x2, #0, arm64_ec_native_exit_direct
    ldr     x1, [x18, #0x1788]
    cbz     x1, arm64_ec_native_exit_direct
    /* room for callbacks nested below this frame */
    ldr     x2, [x1, #0x10]              /* EmulatorStackLimit */
    mov     x3, sp
    sub     x3, x3, x2
    cmp     x3, #0x10, lsl #12           /* 64 KiB */
    b.lo    arm64_ec_native_exit_direct
    ldr     x2, [x1, #0x20]              /* SuspendDoorbell */
    cbz     x2, 1f
    ldr     w2, [x2]
    cbnz    w2, arm64_ec_native_exit_direct
1:  ldr     x1, [x1, #0x38]              /* EmulatorData[EC_DATA_THR] */
    cbz     x1, 2f
    ldr     w1, [x1, #BOX64EC_THR_EXCEPTION_PENDING_OFFSET]
    cbnz    w1, arm64_ec_native_exit_direct
2:
    /* push the record, CALLRET bit set if the CALL pushed a pair for this return */
    ldr     x10, [x28, #8]
    ldr     x6, [x20]
    cmp     x10, x6
    cset    w10, eq
    mov     w16, #BOX64EC_FAST_RECORD_TAG_LOW
    movk    w16, #BOX64EC_FAST_RECORD_TAG_HIGH, lsl #16
    orr     w16, w16, w10
    lsl     x16, x16, #32
    mov     w11, w7
    orr     x16, x16, x11
    sub     x28, x28, #16
    stp     x16, x20, [x28]
    str     x28, [x0, #BOX64EC_EMU_CALLRET_SP_OFFSET]

    bl      Box64EC_ApplyLiveMxcsr       /* x0 = emu, returned */
    mov     x12, x20                     /* guest RSP, 8 mod 16 */
    mov     x15, x27                     /* native target */

    add     x17, x0, #BOX64EC_EMU_XMM_OFFSET
    ldp     q0,  q1,  [x17, #0x00]
    ldp     q2,  q3,  [x17, #0x20]
    ldp     q4,  q5,  [x17, #0x40]
    ldp     q6,  q7,  [x17, #0x60]
    ldp     q8,  q9,  [x17, #0x80]
    ldp     q10, q11, [x17, #0xa0]
    ldp     q12, q13, [x17, #0xc0]
    ldp     q14, q15, [x17, #0xe0]
    ldr     x8,  [x0, (8 *  0)]          /* RAX */
    ldr     x1,  [x0, (8 *  2)]          /* RDX */
    ldr     x27, [x0, (8 *  3)]          /* RBX */
    ldr     x29, [x0, (8 *  5)]          /* RBP */
    ldr     x25, [x0, (8 *  6)]          /* RSI */
    ldr     x26, [x0, (8 *  7)]          /* RDI */
    ldr     x2,  [x0, (8 *  8)]          /* R8  */
    ldr     x3,  [x0, (8 *  9)]          /* R9  */
    ldr     x4,  [x0, (8 * 10)]          /* R10 */
    ldr     x5,  [x0, (8 * 11)]          /* R11 */
    ldr     x19, [x0, (8 * 12)]          /* R12 */
    ldr     x20, [x0, (8 * 13)]          /* R13 */
    ldr     x21, [x0, (8 * 14)]          /* R14 */
    ldr     x22, [x0, (8 * 15)]          /* R15 */
    ldr     x0,  [x0, (8 *  1)]          /* RCX, last: x0 was emu */
    mov     x9,  x15                     /* ExitFunctionEC target */
    mov     x30, x15                     /* unwind destination after the stack switch */
arm64_ec_fast_forward_stack_switch:
    mov     sp,  x12
    b       ExitFunctionEC

/* RetToEntryThunk matched the record: x9 = guest return RIP, x12 = record */
arm64_ec_fast_resume:
    ldr     x6, [x18, #0x1788]
    ldr     x6, [x6, #0x30]              /* EmulatorData[EC_DATA_EMU] */
    stp     x8,  x0,  [x6, (8 *  0)]     /* RAX, RCX */
    stp     x1,  x27, [x6, (8 *  2)]     /* RDX, RBX */
    mov     x10, sp
    stp     x10, x29, [x6, (8 *  4)]     /* RSP, RBP */
    stp     x25, x26, [x6, (8 *  6)]     /* RSI, RDI */
    stp     x2,  x3,  [x6, (8 *  8)]     /* R8, R9 */
    stp     x4,  x5,  [x6, (8 * 10)]     /* R10, R11 */
    stp     x19, x20, [x6, (8 * 12)]     /* R12, R13 */
    stp     x21, x22, [x6, (8 * 14)]     /* R14, R15 */
    str     x9,  [x6, (8 * 17)]          /* RIP */
    /* all 16: a callback during the call may have run the JIT on this emu */
    add     x7, x6, #BOX64EC_EMU_XMM_OFFSET
    stp     q0,  q1,  [x7, #0x00]
    stp     q2,  q3,  [x7, #0x20]
    stp     q4,  q5,  [x7, #0x40]
    stp     q6,  q7,  [x7, #0x60]
    stp     q8,  q9,  [x7, #0x80]
    stp     q10, q11, [x7, #0xa0]
    stp     q12, q13, [x7, #0xc0]
    stp     q14, q15, [x7, #0xe0]
    mov     x19, x12                     /* the record survives the helper call */
    ldr     x15, [x12]
    add     x16, x12, #16                /* pop the record */
    tbz     x15, #32, 1f
    add     x16, x16, #16                /* and the CALLRET pair of the call */
1:  str     x16, [x6, #BOX64EC_EMU_CALLRET_SP_OFFSET]
    ldr     x29, [x6, #BOX64EC_EMU_JIT_FRAME_OFFSET]
    mov     sp,  x29
arm64_ec_fast_resume_stable:
    mov     x0,  x6
    bl      Box64EC_CaptureLiveMxcsr     /* x0 = emu, returned */
    mov     x6,  x0
    /* cross-process code changes queued during the call, as in EnterX64 */
    ldr     x1,  [x18, #0x60]            /* PEB */
    ldr     x1,  [x1, #BOX64EC_PEB_CHPEV2_INFO_OFFSET]
    cbz     x1,  4f
    ldr     x1,  [x1, #BOX64EC_CHPEV2_WORK_LIST_OFFSET]
    cbz     x1,  4f
    ldr     w1,  [x1, #BOX64EC_WORK_LIST_FIRST_OFFSET]
    cbz     w1,  4f
    mov     x20, x6                      /* emu survives the call */
    bl      process_pending_cross_process_work
    mov     x6,  x20
4:
    ldr     x7,  [x19]                   /* guest flags */
    mov     w7,  w7
    ldr     x16, [x18, #0x1788]
    mov     w17, #1
    strb    w17, [x16]                   /* InSimulation = 1 */

    /* back to the JIT register map */
    ldr     x10, [x6, (8 *  0)]
    ldr     x11, [x6, (8 *  1)]
    ldr     x12, [x6, (8 *  2)]
    ldr     x8,  [x6, (8 *  3)]
    ldr     x20, [x6, (8 *  4)]
    ldr     x21, [x6, (8 *  5)]
    ldr     x16, [x6, (8 *  6)]
    ldr     x17, [x6, (8 *  7)]
    ldr     x15, [x6, (8 *  8)]
    ldr     x23, [x6, (8 * 10)]
    ldr     x9,  [x6, (8 * 11)]
    ldr     x22, [x6, (8 * 12)]
    ldr     x25, [x6, (8 * 13)]
    ldr     x26, [x6, (8 * 14)]
    ldr     x19, [x6, (8 * 15)]
    ldr     x27, [x6, (8 * 17)]
    mov     x0,  x6
    ldr     x6,  [x0, (8 *  9)]
    ldr     x28, [x0, #BOX64EC_EMU_CALLRET_SP_OFFSET]
    mov     x1,  x27
    /* jump table entry of the return RIP, the default one above 48 bits */
    adr     x2,  arm64_next
    lsr     x3,  x27, #48
    cbnz    x3,  3f
    adrp    x2,  box64_ec_jmptable48
    ldr     x2,  [x2, :lo12:box64_ec_jmptable48]
#ifdef SAVE_MEM
    ubfm    x3,  x27, #JMPTABL_START3, #(JMPTABL_START3 + JMPTABL_SHIFT3 - 1)
    ldr     x2,  [x2, x3, lsl #3]
#endif
    ubfm    x3,  x27, #JMPTABL_START2, #(JMPTABL_START2 + JMPTABL_SHIFT2 - 1)
    ldr     x2,  [x2, x3, lsl #3]
    ubfm    x3,  x27, #JMPTABL_START1, #(JMPTABL_START1 + JMPTABL_SHIFT1 - 1)
    ldr     x2,  [x2, x3, lsl #3]
    ubfm    x3,  x27, #JMPTABL_START0, #(JMPTABL_START0 + JMPTABL_SHIFT0 - 1)
    ldr     x2,  [x2, x3, lsl #3]
3:  br      x2
arm64_ec_fast_resume_end:

/* These are noncontiguous children of arm64_prolog: end_c separates the local
 * prolog from the inherited 160-byte dynarec frame. */
.section .xdata,"dr"
.p2align 2
.Lunwind_arm64_next:
    .long ((arm64_next_end-arm64_next)/4) | (1<<21) | (8<<27)
    .byte 0x08, 0x81, 0xe5, 0xe1
    .byte 0xd9, 0x90, 0xd9, 0x0e, 0xd8, 0x8c, 0xd8, 0x0a
    .byte 0xe7, 0x17, 0x09, 0xd2, 0x08, 0xc9, 0x86, 0xc8
    .byte 0x84, 0xc8, 0x02, 0x52, 0x0a, 0xe4
    .byte 0xe3, 0xe3, 0xe3, 0xe3, 0xe3, 0xe3
.Lunwind_arm64_next_invalid:
    .long ((arm64_next_invalid_end-arm64_next_invalid)/4) | (1<<21) | (8<<27)
    .byte 0x08, 0x81, 0xe5, 0xe1
    .byte 0xd9, 0x90, 0xd9, 0x0e, 0xd8, 0x8c, 0xd8, 0x0a
    .byte 0xe7, 0x17, 0x09, 0xd2, 0x08, 0xc9, 0x86, 0xc8
    .byte 0x84, 0xc8, 0x02, 0x52, 0x0a, 0xe4
    .byte 0xe3, 0xe3, 0xe3, 0xe3, 0xe3, 0xe3

.Lunwind_arm64_ec_native_exit_direct:
    .long ((arm64_ec_native_exit_direct_end-arm64_ec_native_exit_direct)/4) | (1<<21) | (7<<27)
    .byte 0x01, 0x81, 0xe5, 0xe1
    .byte 0xd9, 0x90, 0xd9, 0x0e, 0xd8, 0x8c, 0xd8, 0x0a
    .byte 0xe7, 0x17, 0x09, 0xd2, 0x08, 0xc9, 0x86, 0xc8
    .byte 0x84, 0xc8, 0x02, 0x52, 0x0a, 0xe4
    .byte 0xe3, 0xe3

/* SP based: x29 may hold the EC buddy RBP in these fragments */
.Lunwind_arm64_fast_forward_direct:
    .long ((arm64_ec_fast_forward_direct_end-arm64_ec_fast_forward_direct)/4) | (6<<27)
    .byte 0xe5, 0xd9, 0x90, 0xd9, 0x0e, 0xd8, 0x8c, 0xd8
    .byte 0x0a, 0xe7, 0x17, 0x09, 0xd2, 0x08, 0xc9, 0x86
    .byte 0xc8, 0x84, 0xc8, 0x02, 0x52, 0x0a, 0xe4, 0xe3
.Lunwind_arm64_fast_forward:
    .long ((arm64_ec_fast_forward_stack_switch-arm64_ec_fast_forward)/4) | (6<<27)
    .byte 0xe5, 0xd9, 0x90, 0xd9, 0x0e, 0xd8, 0x8c, 0xd8
    .byte 0x0a, 0xe7, 0x17, 0x09, 0xd2, 0x08, 0xc9, 0x86
    .byte 0xc8, 0x84, 0xc8, 0x02, 0x52, 0x0a, 0xe4, 0xe3
.Lunwind_arm64_fast_resume:
    .long ((arm64_ec_fast_resume_end-arm64_ec_fast_resume_stable)/4) | (6<<27)
    .byte 0xe5, 0xd9, 0x90, 0xd9, 0x0e, 0xd8, 0x8c, 0xd8
    .byte 0x0a, 0xe7, 0x17, 0x09, 0xd2, 0x08, 0xc9, 0x86
    .byte 0xc8, 0x84, 0xc8, 0x02, 0x52, 0x0a, 0xe4, 0xe3

.section .pdata,"dr"
.p2align 2
    .rva arm64_next
    .rva .Lunwind_arm64_next
    .rva arm64_next_invalid
    .rva .Lunwind_arm64_next_invalid
    .rva arm64_ec_native_exit_direct
    .rva .Lunwind_arm64_ec_native_exit_direct
    .rva arm64_ec_fast_forward_direct
    .rva .Lunwind_arm64_fast_forward_direct
    .rva arm64_ec_fast_forward
    .rva .Lunwind_arm64_fast_forward
    .rva arm64_ec_fast_resume_stable
    .rva .Lunwind_arm64_fast_resume
