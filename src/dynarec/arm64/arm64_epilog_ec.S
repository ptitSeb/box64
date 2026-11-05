// SPDX-License-Identifier: MIT
// ARM64EC dynarec epilog
// x0 = emu*

.text
.align 4

.global arm64_epilog

arm64_epilog:
    str     x10, [x0, (8 *  0)]          /* RAX */
    str     x11, [x0, (8 *  1)]          /* RCX */
    str     x12, [x0, (8 *  2)]          /* RDX */
    str     x8,  [x0, (8 *  3)]          /* RBX */
    str     x20, [x0, (8 *  4)]          /* RSP */
    str     x21, [x0, (8 *  5)]          /* RBP */
    str     x16, [x0, (8 *  6)]          /* RSI */
    str     x17, [x0, (8 *  7)]          /* RDI */
    str     x15, [x0, (8 *  8)]          /* R8 */
    str     x6,  [x0, (8 *  9)]          /* R9 */
    str     x23, [x0, (8 * 10)]          /* R10 */
    str     x9,  [x0, (8 * 11)]          /* R11 */
    str     x22, [x0, (8 * 12)]          /* R12 */
    str     x25, [x0, (8 * 13)]          /* R13 */
    str     x26, [x0, (8 * 14)]          /* R14 */
    str     x19, [x0, (8 * 15)]          /* R15 */
    str     x7,  [x0, (8 * 16)]          /* Flags */
    str     x27, [x0, (8 * 17)]          /* RIP */
arm64_epilog_scope:
    mov     sp, x29
    ldp     x19, x20, [sp, #16]
    ldp     x21, x22, [sp, #32]
    ldp     x25, x26, [sp, #48]
    ldr     x27, [sp, #64]
    ldr     x23, [sp, #72]
    ldp     d8,  d9,  [sp, #80]
    ldp     d10, d11, [sp, #96]
    ldp     d12, d13, [sp, #112]
    ldp     d14, d15, [sp, #128]
    ldp     x29, x30, [sp, #144]
    add     sp, sp, #160
    ret
arm64_epilog_end:

/* The epilog runs in the arm64_prolog frame (the JIT jumps here), so its
 * prolog codes are those of arm64_prolog, and the explicit epilog scope first
 * gets SP back from x29. */
.section .xdata,"dr"
.p2align 2
.Lunwind_arm64_epilog:
    .long ((arm64_epilog_end-arm64_epilog)/4) | (1<<22) | (14<<27)
    .long ((arm64_epilog_scope-arm64_epilog)/4) | (24<<22)
    /* Parent phantom prolog. */
    .byte 0xe5, 0xe1
    .byte 0xd9, 0x90, 0xd9, 0x0e, 0xd8, 0x8c, 0xd8, 0x0a
    .byte 0xe7, 0x17, 0x09, 0xd2, 0x08, 0xc9, 0x86, 0xc8
    .byte 0x84, 0xc8, 0x02, 0x52, 0x0a, 0xe4
    /* Explicit epilog starts by recovering SP from the private anchor. */
    .byte 0xe1
    .byte 0xc8, 0x02, 0xc8, 0x84, 0xc9, 0x86, 0xd2, 0x08
    .byte 0xe7, 0x17, 0x09, 0xd8, 0x0a, 0xd8, 0x8c, 0xd9
    .byte 0x0e, 0xd9, 0x90, 0x52, 0x0a, 0xe4
    .byte 0xe3, 0xe3, 0xe3, 0xe3, 0xe3, 0xe3, 0xe3, 0xe3, 0xe3

.section .pdata,"dr"
.p2align 2
    .rva arm64_epilog
    .rva .Lunwind_arm64_epilog
