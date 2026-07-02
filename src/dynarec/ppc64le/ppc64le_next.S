// PPC64LE next linker for dynarec
// Called when a dynarec block needs to jump to the next block.
// Saves volatile state, calls LinkNext to resolve the target,
// then branches to the resolved address.
//
// Called with:
//   Emu (r31) = pointer to x64emu_t
//   RIP (r9)  = target x86 IP
//   LR        = "from" address (for linking)

#define ASM_MAPPING
#include "ppc64le_mapping.h"

.text
.align 4

.extern LinkNext

.global ppc64le_next
.type ppc64le_next, @function

    // NULL pointer before ppc64le_next, for getDB
    .8byte  0
ppc64le_next:
    // Save volatile registers that the dynarec uses
    // We need to preserve: r3-r10 (args/scratch used by dynarec)
    // and RIP (r9), plus the LR (return "from" address)
    mflr    %r0

    // Allocate save area on stack
    stdu    %r1, -128(%r1)

    // Save registers
    std     %r0,   16(%r1)        // LR (the "from" address)
    std     %r3,   32(%r1)        // x1 / A0
    std     %r4,   40(%r1)        // x2 / A1
    std     %r5,   48(%r1)        // x3 / A2
    std     %r6,   56(%r1)        // x4 / A3
    std     %r7,   64(%r1)        // x5 / A4
    std     %r8,   72(%r1)        // x6 / A5
    std     %r9,   80(%r1)        // xRIP - also save to allow change in LinkNext
    std     %r10,  88(%r1)        // x7 / A7

    // Call LinkNext(emu, ip, from, &rip_on_stack)
    mr      %r3, Emu               // arg0 = emu (r31)
    mr      %r4, RIP               // arg1 = xRIP (r9)
    ld      %r5, 16(%r1)           // arg2 = "from" (saved LR)
    addi    %r6, %r1, 80           // arg3 = address of saved RIP on stack

    // Restore TOC (r2) before calling C code.
    // JIT code calls native wrapped functions (e.g., libc) which set r2 to
    // their own TOC. Since `bl LinkNext` is resolved as a local call (skipping
    // LinkNext's global entry that reloads r2), we must restore it ourselves.
    // emu->xSPSave (offset 808 from Emu/r31) points to the prolog's stack
    // frame, which has the saved TOC at offset 24.
    ld      %r2, 808(Emu)          // r2 = prolog's frame SP (from xSPSave)
    ld      %r2, 24(%r2)           // r2 = saved TOC from prolog frame

    bl      LinkNext
    nop                            // TOC restore slot (linker may patch)

    // Preserve return value (jump target) in r12
    mr      %r12, %r3

    // Restore registers
    ld      %r3,   32(%r1)
    ld      %r4,   40(%r1)
    ld      %r5,   48(%r1)
    ld      %r6,   56(%r1)
    ld      %r7,   64(%r1)
    ld      %r8,   72(%r1)
    ld      %r9,   80(%r1)        // RIP may have been modified by LinkNext
    ld      %r10,  88(%r1)

    // Deallocate save area
    addi    %r1, %r1, 128

    // Jump to resolved target
    mtctr   %r12
    bctr
.size ppc64le_next, .-ppc64le_next
