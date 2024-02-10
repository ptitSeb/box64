//arm prologue for dynarec
//Save stuff, prepare stack and register
//called with pointer to emu as 1st parameter
//and address to jump to as 2nd parameter

.text
.align 4

.global rv64_prolog
rv64_prolog:
    //save all 18 used register
    addi    sp,  sp, -(8 * 24)  // 16 bytes aligned
    sd      ra, (sp)  // save ra
    sd      x8, 8(sp) // save fp
    sd      x18, (2*8)(sp)
    sd      x19, (3*8)(sp)
    sd      x20, (4*8)(sp)
    sd      x21, (5*8)(sp)
    sd      x22, (6*8)(sp)
    sd      x23, (7*8)(sp)
    sd      x24, (8*8)(sp)
    sd      x25, (9*8)(sp)
    sd      x26, (10*8)(sp)
    sd      x27, (11*8)(sp)
    sd      x9,  (12*8)(sp)
    fsd     f18, (13*8)(sp)
    fsd     f19, (14*8)(sp)
    fsd     f20, (15*8)(sp)
    fsd     f21, (16*8)(sp)
    fsd     f22, (17*8)(sp)
    fsd     f23, (18*8)(sp)
    fsd     f24, (19*8)(sp)
    fsd     f25, (20*8)(sp)
    fsd     f26, (21*8)(sp)
    fsd     f27, (22*8)(sp)
    //setup emu -> register
    ld      x16, (a0)
    ld      x17, 8(a0)
    ld      x18, 16(a0)
    ld      x19, 24(a0)
    ld      x20, 32(a0)
    ld      x21, 40(a0)
    ld      x22, 48(a0)
    ld      x23, 56(a0)
    ld      x24, 64(a0)
    ld      x25, 72(a0)
    ld      x26, 80(a0)
    ld      x27, 88(a0)
    ld      x28, 96(a0)
    ld      x29, 104(a0)
    ld      x30, 112(a0)
    ld      x31, 120(a0)
    ld      x8, 128(a0)     //xFlags
    ld      x7, 136(a0)     // xRIP
    // // adjust flags bit 11 -> bit 5
    andi    x8, x8, ~(1<<5)    // probably not usefull?
    srli    x5, x8, 11-5
    andi    x5, x5, 1<<5
    or      x8, x8, x5
    ld      x5, 552(a0) // grab an old value of emu->xSPSave
    sd      sp, 552(a0) // save current sp to emu->xSPSave
    // push sentinel onto the stack
    sd      x5, -16(sp)
    sd      zero, -8(sp)
    addi    sp, sp, -16
    // setup xMASK
    xori    x5, x0, -1
    srli    x5, x5, 32
    // jump to block
    jalr    a1
