#define ASM_MAPPING 1
#include "rv64_mapping.h"
#undef ASM_MAPPING

.text
.align 4

.global rv64_epilog
.global rv64_epilog_fast

// rv64_epilog(void)
rv64_epilog:

    // adjust flags bit 5 -> bit 11
    lui     t3, 0xfffff
    addiw   t3, t3, 0x7df
    and     t3, Flags, t3
    andi    Flags, Flags, 1<<5
    slli    Flags, Flags, 11-5
    or      Flags, Flags, t3

    // spill x86 registers to emu
    sd      RAX,   0(Emu)
    sd      RCX,   8(Emu)
    sd      RDX,   16(Emu)
    sd      RBX,   24(Emu)
    sd      RSP,   32(Emu)
    sd      RBP,   40(Emu)
    sd      RSI,   48(Emu)
    sd      RDI,   56(Emu)
    sd      R8,    64(Emu)
    sd      R9,    72(Emu)
    sd      R10,   80(Emu)
    sd      R11,   88(Emu)
    sd      R12,   96(Emu)
    sd      R13,   104(Emu)
    sd      R14,   112(Emu)
    sd      R15,   120(Emu)
    sd      Flags, 128(Emu)
    sd      RIP,   136(Emu)

    /*** switch to native register naming convection ***/

    // fallback to epilog_fast now, just restoring saved regs
rv64_epilog_fast:
    // restore saved sp from emu->xSPSave
    ld      sp, 808(Emu)
    ld      t3, -8(sp)
    // put back old value
    sd      t3, 808(Emu)

    ld      ra,   (0*8)(sp)
    ld      fp,   (1*8)(sp)
    ld      s1,   (2*8)(sp)
    ld      s2,   (3*8)(sp)
    ld      s3,   (4*8)(sp)
    ld      s4,   (5*8)(sp)
    ld      s5,   (6*8)(sp)
    ld      s6,   (7*8)(sp)
    ld      s7,   (8*8)(sp)
    ld      s8,   (9*8)(sp)
    ld      s9,   (10*8)(sp)
    ld      s10,  (11*8)(sp)
    ld      s11,  (12*8)(sp)
    fld     fs2,  (13*8)(sp)
    fld     fs3,  (14*8)(sp)
    fld     fs4,  (15*8)(sp)
    fld     fs5,  (16*8)(sp)
    fld     fs6,  (17*8)(sp)
    fld     fs7,  (19*8)(sp)
    fld     fs8,  (19*8)(sp)
    fld     fs9,  (20*8)(sp)
    fld     fs10, (21*8)(sp)
    fld     fs11, (22*8)(sp)

    // 16 bytes aligned
    addi    sp,  sp, (8 * 24)
    ret
