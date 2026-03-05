// PPC64LE prolog for dynarec
// Save callee-saved registers, load x86 state from emu struct,
// then jump to the dynarec block.
// Called with:
//   r3 = pointer to x64emu_t (emu)
//   r4 = address to jump to (dynarec block entry)
//
// ELFv2 ABI: r1=SP, r2=TOC (preserved), r13=TLS (preserved)
// Callee-saved GPRs: r14-r31
// Callee-saved VMX: vr20-vr31

#define ASM_MAPPING
#include "ppc64le_mapping.h"

.text
.align 4

.global ppc64le_prolog
.type ppc64le_prolog, @function
ppc64le_prolog:
    // Save LR
    mflr    %r0
    std     %r0, 16(%r1)

    // Stack frame: 352 bytes (16-byte aligned)
    // Layout (from new SP):
    //   0: back chain
    //   8: CR save
    //  16: LR save (for functions called from dynarec)
    //  24: TOC save
    //  32: r14-r31 save area (144 bytes = 18*8)
    // 176: jump_next slot (16 bytes = 2*8)
    // 192: old xSPSave (8 bytes, for nested dynarec)
    // 200: padding (8 bytes, for alignment)
    // 208: vr20 save area (16 bytes, callee-saved VMX)
    // 224: vr24 save area (16 bytes, callee-saved VMX, MMX0)
    // 240: vr25 save area (16 bytes, callee-saved VMX, MMX1)
    // 256: vr26 save area (16 bytes, callee-saved VMX, MMX2)
    // 272: vr27 save area (16 bytes, callee-saved VMX, MMX3)
    // 288: vr28 save area (16 bytes, callee-saved VMX, MMX4)
    // 304: vr29 save area (16 bytes, callee-saved VMX, MMX5)
    // 320: vr30 save area (16 bytes, callee-saved VMX, MMX6)
    // 336: vr31 save area (16 bytes, callee-saved VMX, MMX7)

    stdu    %r1, -352(%r1)          // allocate stack frame

    // Save callee-saved GPRs
    std     %r14, 32(%r1)
    std     %r15, 40(%r1)
    std     %r16, 48(%r1)
    std     %r17, 56(%r1)
    std     %r18, 64(%r1)
    std     %r19, 72(%r1)
    std     %r20, 80(%r1)
    std     %r21, 88(%r1)
    std     %r22, 96(%r1)
    std     %r23, 104(%r1)
    std     %r24, 112(%r1)
    std     %r25, 120(%r1)
    std     %r26, 128(%r1)
    std     %r27, 136(%r1)
    std     %r28, 144(%r1)
    std     %r29, 152(%r1)
    std     %r30, 160(%r1)
    std     %r31, 168(%r1)

    // Save CR
    mfcr    %r0
    stw     %r0, 8(%r1)

    // Save callee-saved VMX registers
    // vr20 (= vs52): scratch register index 28
    stxv    52, 208(%r1)
    // vr24-vr31 (= vs56-vs63): MMX registers 0-7
    stxv    56, 224(%r1)
    stxv    57, 240(%r1)
    stxv    58, 256(%r1)
    stxv    59, 272(%r1)
    stxv    60, 288(%r1)
    stxv    61, 304(%r1)
    stxv    62, 320(%r1)
    stxv    63, 336(%r1)

    // Save TOC (r2)
    std     %r2, 24(%r1)

    // Setup: load x86 register state from emu struct
    // emu struct layout: regs[0..15] = RAX,RCX,RDX,RBX,RSP,RBP,RSI,RDI,R8..R15
    // Each is 8 bytes, starting at offset 0 in x64emu_t
    // r3 = emu pointer

    // Store emu pointer first (needed for xSPSave below)
    mr      Emu, %r3                // Emu = r31

    // Save the current frame SP to emu->xSPSave so the epilog can find it.
    // r12 (xSavedSP) is caller-saved and gets clobbered by native calls,
    // so we store the frame base in the emu struct instead (like RV64).
    // Also save the old xSPSave value on the stack for nested dynarec calls.
    ld      %r0, 808(Emu)           // old xSPSave value
    std     %r1, 808(Emu)           // emu->xSPSave = current frame SP
    std     %r0, 192(%r1)           // save old xSPSave at frame offset 192

    // Load x86 registers from emu into mapped PPC64LE registers
    ld      RAX, (8 *  0)(Emu)      // RAX = r14
    ld      RCX, (8 *  1)(Emu)      // RCX = r15
    ld      RDX, (8 *  2)(Emu)      // RDX = r16
    ld      RBX, (8 *  3)(Emu)      // RBX = r17
    ld      RSP, (8 *  4)(Emu)      // RSP = r18
    ld      RBP, (8 *  5)(Emu)      // RBP = r19
    ld      RSI, (8 *  6)(Emu)      // RSI = r20
    ld      RDI, (8 *  7)(Emu)      // RDI = r21
    ld      R8,  (8 *  8)(Emu)      // R8  = r22
    ld      R9,  (8 *  9)(Emu)      // R9  = r23
    ld      R10, (8 * 10)(Emu)      // R10 = r24
    ld      R11, (8 * 11)(Emu)      // R11 = r25
    ld      R12, (8 * 12)(Emu)      // R12 = r26
    ld      R13, (8 * 13)(Emu)      // R13 = r27
    ld      R14, (8 * 14)(Emu)      // R14 = r28
    ld      R15, (8 * 15)(Emu)      // R15 = r29

    // Load flags (at offset 16*8 = 128)
    ld      Flags, (8 * 16)(Emu)    // Flags = r30

    // Load RIP into r9 (xRIP)
    ld      RIP, (8 * 17)(Emu)      // RIP = r9

    // Push a jump_next slot on stack (2 doublewords of zeros)
    li      %r0, 0
    std     %r0, 176(%r1)           // jump_next[0] = 0
    std     %r0, 184(%r1)           // jump_next[1] = 0

    // Jump to dynarec block (address in r4)
    mtctr   %r4
    bctr                             // branch to CTR (no link)
.size ppc64le_prolog, .-ppc64le_prolog
