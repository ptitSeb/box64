// PPC64LE epilog for dynarec
// Store x86 register state back to emu struct,
// restore callee-saved registers, return to caller.
// Called with:
//   Emu (r31) = pointer to x64emu_t
//
// Must store back all x86 registers including RIP.

#define ASM_MAPPING
#include "ppc64le_mapping.h"

.text
.align 4

.global ppc64le_epilog
.type ppc64le_epilog, @function
ppc64le_epilog:
    // Store x86 registers back to emu struct
    // Emu (r31) points to x64emu_t
    std     RAX, (8 *  0)(Emu)      // r14
    std     RCX, (8 *  1)(Emu)      // r15
    std     RDX, (8 *  2)(Emu)      // r16
    std     RBX, (8 *  3)(Emu)      // r17
    std     RSP, (8 *  4)(Emu)      // r18
    std     RBP, (8 *  5)(Emu)      // r19
    std     RSI, (8 *  6)(Emu)      // r20
    std     RDI, (8 *  7)(Emu)      // r21
    std     R8,  (8 *  8)(Emu)      // r22
    std     R9,  (8 *  9)(Emu)      // r23
    std     R10, (8 * 10)(Emu)      // r24
    std     R11, (8 * 11)(Emu)      // r25
    std     R12, (8 * 12)(Emu)      // r26
    std     R13, (8 * 13)(Emu)      // r27
    std     R14, (8 * 14)(Emu)      // r28
    std     R15, (8 * 15)(Emu)      // r29

    // Store flags
    std     Flags, (8 * 16)(Emu)    // r30

    // Store RIP (r9 = xRIP, must contain current IP)
    std     RIP, (8 * 17)(Emu)      // r9

    // Restore stack pointer from emu->xSPSave
    // xSPSave stores the frame base SP saved by the prolog
    ld      %r1, 808(Emu)

    // Restore old xSPSave for nested dynarec calls
    ld      %r0, 192(%r1)
    std     %r0, 808(Emu)

    // Restore TOC
    ld      %r2, 24(%r1)

    // Restore CR
    lwz     %r0, 8(%r1)
    mtcrf   0xFF, %r0

    // Restore callee-saved VMX registers
    // vr20 (= vs52): scratch register index 28
    lxv     52, 208(%r1)
    // vr24-vr31 (= vs56-vs63): MMX registers 0-7
    lxv     56, 224(%r1)
    lxv     57, 240(%r1)
    lxv     58, 256(%r1)
    lxv     59, 272(%r1)
    lxv     60, 288(%r1)
    lxv     61, 304(%r1)
    lxv     62, 320(%r1)
    lxv     63, 336(%r1)

    // Restore callee-saved GPRs
    ld      %r14, 32(%r1)
    ld      %r15, 40(%r1)
    ld      %r16, 48(%r1)
    ld      %r17, 56(%r1)
    ld      %r18, 64(%r1)
    ld      %r19, 72(%r1)
    ld      %r20, 80(%r1)
    ld      %r21, 88(%r1)
    ld      %r22, 96(%r1)
    ld      %r23, 104(%r1)
    ld      %r24, 112(%r1)
    ld      %r25, 120(%r1)
    ld      %r26, 128(%r1)
    ld      %r27, 136(%r1)
    ld      %r28, 144(%r1)
    ld      %r29, 152(%r1)
    ld      %r30, 160(%r1)
    ld      %r31, 168(%r1)

    // Deallocate frame
    addi    %r1, %r1, 352

    // Restore LR and return
    ld      %r0, 16(%r1)
    mtlr    %r0
    blr
.size ppc64le_epilog, .-ppc64le_epilog
