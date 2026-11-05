// SPDX-License-Identifier: MIT
// ARM64EC dynarec prolog
// x0 = emu*, x1 = native block entry

.text
.align 4

.global arm64_prolog

.seh_proc arm64_prolog
arm64_prolog:
    /* The JIT chunks register unwind codes for this same 160-byte frame, keep
     * both in sync. The lowest 16 bytes are not used. */
    sub     sp, sp, #160
    .seh_stackalloc 160
    stp     x29, x30, [sp, #144]
    .seh_save_fplr 144
    stp     x19, x20, [sp, #16]
    .seh_save_regp x19, 16
    stp     x21, x22, [sp, #32]
    .seh_save_regp x21, 32
    stp     x25, x26, [sp, #48]
    .seh_save_regp x25, 48
    str     x27, [sp, #64]
    .seh_save_reg x27, 64
    str     x23, [sp, #72]               /* guest R10, an ARM64EC blocked register */
    .seh_save_any_reg x23, 72
    stp     d8,  d9,  [sp, #80]
    .seh_save_fregp d8, 80
    stp     d10, d11, [sp, #96]
    .seh_save_fregp d10, 96
    stp     d12, d13, [sp, #112]
    .seh_save_fregp d12, 112
    stp     d14, d15, [sp, #128]
    .seh_save_fregp d14, 128
    .seh_endprologue

    /* The epilog restores SP from x29: guest RBP is in x21 in the JIT map. */
    mov     x29, sp

    /* Load guest GPRs: R8 in x15, R9 in x6, R10 in x23, R11 in x9, x18 stays the TEB */
    ldr     x10, [x0, (8 *  0)]          /* RAX */
    ldr     x11, [x0, (8 *  1)]          /* RCX */
    ldr     x12, [x0, (8 *  2)]          /* RDX */
    ldr     x8,  [x0, (8 *  3)]          /* RBX */
    ldr     x20, [x0, (8 *  4)]          /* RSP */
    ldr     x21, [x0, (8 *  5)]          /* RBP */
    ldr     x16, [x0, (8 *  6)]          /* RSI */
    ldr     x17, [x0, (8 *  7)]          /* RDI */
    ldr     x15, [x0, (8 *  8)]          /* R8  */
    ldr     x6,  [x0, (8 *  9)]          /* R9  */
    ldr     x23, [x0, (8 * 10)]          /* R10 */
    ldr     x9,  [x0, (8 * 11)]          /* R11 */
    ldr     x22, [x0, (8 * 12)]          /* R12 */
    ldr     x25, [x0, (8 * 13)]          /* R13 */
    ldr     x26, [x0, (8 * 14)]          /* R14 */
    ldr     x19, [x0, (8 * 15)]          /* R15 */
    ldr     x7,  [x0, (8 * 16)]          /* Flags */
    ldr     x27, [x0, (8 * 17)]          /* RIP */

    br      x1
.seh_endproc
