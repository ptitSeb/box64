#define ASM_MAPPING 1
#include "rv64_mapping.h"
#undef ASM_MAPPING

.text
.align 4

.global rv64_prolog

// rv64_prolog(emu, jump_address)
rv64_prolog:

    // 16 bytes aligned
    addi    sp,   sp, -(8 * 24)

    // save callee-saved registers
    sd      ra,   (0*8)(sp)
    sd      fp,   (1*8)(sp)
    sd      s1,   (2*8)(sp)
    sd      s2,   (3*8)(sp)
    sd      s3,   (4*8)(sp)
    sd      s4,   (5*8)(sp)
    sd      s5,   (6*8)(sp)
    sd      s6,   (7*8)(sp)
    sd      s7,   (8*8)(sp)
    sd      s8,   (9*8)(sp)
    sd      s9,   (10*8)(sp)
    sd      s10,  (11*8)(sp)
    sd      s11,  (12*8)(sp)
    fsd     fs2,  (13*8)(sp)
    fsd     fs3,  (14*8)(sp)
    fsd     fs4,  (15*8)(sp)
    fsd     fs5,  (16*8)(sp)
    fsd     fs6,  (17*8)(sp)
    fsd     fs7,  (18*8)(sp)
    fsd     fs8,  (19*8)(sp)
    fsd     fs9,  (20*8)(sp)
    fsd     fs10, (21*8)(sp)
    fsd     fs11, (22*8)(sp)

    // save a1
    mv      t6,  a1

    /*** switch to box64 register naming convection ***/

    // load x86 registers from emu
    mv      Emu, a0
    ld      RAX,   0(Emu)
    ld      RCX,   8(Emu)
    ld      RDX,   16(Emu)
    ld      RBX,   24(Emu)
    ld      RSP,   32(Emu)
    ld      RBP,   40(Emu)
    ld      RSI,   48(Emu)
    ld      RDI,   56(Emu)
    ld      R8,    64(Emu)
    ld      R9,    72(Emu)
    ld      R10,   80(Emu)
    ld      R11,   88(Emu)
    ld      R12,   96(Emu)
    ld      R13,   104(Emu)
    ld      R14,   112(Emu)
    ld      R15,   120(Emu)
    ld      Flags, 128(Emu)
    ld      RIP,   136(Emu)

    // adjust flags bit 11 -> bit 5
    andi    Flags, Flags, ~(1<<5)   // probably not usefull?
    srli    t3, Flags, 11-5
    andi    t3, t3, 1<<5
    or      Flags, Flags, t3

    ld      t3, 808(Emu) // grab an old value of emu->xSPSave
    sd      sp, 808(Emu) // save current sp to emu->xSPSave

    // push sentinel onto the stack
    sd      t3, -16(sp)
    sd      zero, -8(sp)
    addi    sp, sp, -16

    // jump to block
    jr      t6
