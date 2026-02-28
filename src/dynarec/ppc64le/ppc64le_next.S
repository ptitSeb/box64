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
    mflr    0

    // Allocate save area on stack
    stdu    1, -128(1)

    // Save registers
    std     0,   16(1)        // LR (the "from" address)
    std     3,   32(1)        // x1 / A0
    std     4,   40(1)        // x2 / A1
    std     5,   48(1)        // x3 / A2
    std     6,   56(1)        // x4 / A3
    std     7,   64(1)        // x5 / A4
    std     8,   72(1)        // x6 / A5
    std     9,   80(1)        // xRIP - also save to allow change in LinkNext
    std     10,  88(1)        // x7 / A7

    // Call LinkNext(emu, ip, from, &rip_on_stack)
    mr      3, Emu             // arg0 = emu (r31)
    mr      4, RIP             // arg1 = xRIP (r9)
    ld      5, 16(1)           // arg2 = "from" (saved LR)
    addi    6, 1, 80           // arg3 = address of saved RIP on stack

    // Restore TOC for calling C code
    // We need to load the TOC for LinkNext - use the saved TOC
    // Note: In ELFv2, r12 must point to the function entry for local calls
    // For external calls via PLT, the linker handles it
    bl      LinkNext
    nop                          // TOC restore slot (linker fills if needed)

    // Preserve return value (jump target) in r12
    mr      12, 3

    // Restore registers
    ld      3,   32(1)
    ld      4,   40(1)
    ld      5,   48(1)
    ld      6,   56(1)
    ld      7,   64(1)
    ld      8,   72(1)
    ld      9,   80(1)        // RIP may have been modified by LinkNext
    ld      10,  88(1)

    // Deallocate save area
    addi    1, 1, 128

    // Jump to resolved target
    mtctr   12
    bctr
.size ppc64le_next, .-ppc64le_next
