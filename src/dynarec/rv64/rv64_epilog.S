//riscv epilog for dynarec
//Save stuff, prepare stack and register
//called with pointer to emu as 1st parameter
//and address to jump to as 2nd parameter

.text
.align 4

.global rv64_epilog
.global rv64_epilog_fast

rv64_epilog:
    //update register -> emu
    sd      x16, (a0)
    sd      x17, 8(a0)
    sd      x18, 16(a0)
    sd      x19, 24(a0)
    sd      x20, 32(a0)
    sd      x21, 40(a0)
    sd      x22, 48(a0)
    sd      x23, 56(a0)
    sd      x24, 64(a0)
    sd      x25, 72(a0)
    sd      x26, 80(a0)
    sd      x27, 88(a0)
    sd      x28, 96(a0)
    sd      x29, 104(a0)
    sd      x30, 112(a0)
    sd      x31, 120(a0)
    // adjust flags bit 5 -> bit 11
    li      x5, ~(1<<11)
    and     x8, x8, x5
    andi    x5, x8, 1<<5
    slli    x5, x5, 11-5
    or      x8, x8, x5
    sd      x8, 128(a0)     //xFlags
    sd      x7, 136(a0)     // put back reg value in emu, including EIP (so x7 must be EIP now)
    // fallback to epilog_fast now, just restoring saved regs
rv64_epilog_fast:
    ld      sp, 552(a0) // restore saved sp from emu->xSPSave, see rv64_prolog
    ld      x9, -8(sp)
    sd      x9, 552(a0) // put back old value
    ld      ra, (sp)  // save ra
    ld      x8, 8(sp) // save fp
    ld      x18, (2*8)(sp)
    ld      x19, (3*8)(sp)
    ld      x20, (4*8)(sp)
    ld      x21, (5*8)(sp)
    ld      x22, (6*8)(sp)
    ld      x23, (7*8)(sp)
    ld      x24, (8*8)(sp)
    ld      x25, (9*8)(sp)
    ld      x26, (10*8)(sp)
    ld      x27, (11*8)(sp)
    ld      x9,  (12*8)(sp)
    fld     f18, (13*8)(sp)
    fld     f19, (14*8)(sp)
    fld     f20, (15*8)(sp)
    fld     f21, (16*8)(sp)
    fld     f22, (17*8)(sp)
    fld     f23, (19*8)(sp)
    fld     f24, (19*8)(sp)
    fld     f25, (20*8)(sp)
    fld     f26, (21*8)(sp)
    fld     f27, (22*8)(sp)
    addi    sp,  sp, (8 * 24)
    //end, return...
    ret
