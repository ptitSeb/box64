// SPDX-License-Identifier: MIT
// ARM64EC update linker table for dynarec
// called with emu in x0 and the x64 target in x1 (and xRIP)

.text
.align 4

.extern LinkNext
.extern LinkNextInvalid

.global arm64_next
.global arm64_next_invalid

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
    mov     x1, x27                      /* xRIP */
    mov     x2, x30                      /* "from" is in lr */
    add     x3, sp, (8 * 13)             /* address to change RIP */
    bl      \linker
    mov     x3, x0
    ldp     x0,  x1,  [sp, (8 *  0)]
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

.section .pdata,"dr"
.p2align 2
    .rva arm64_next
    .rva .Lunwind_arm64_next
    .rva arm64_next_invalid
    .rva .Lunwind_arm64_next_invalid
