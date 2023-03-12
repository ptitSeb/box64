//riscv epilog for dynarec
//Save stuff, prepare stack and register
//called with pointer to emu as 1st parameter
//and address to jump to as 2nd parameter

.text
.align 4

.global rv64_epilog
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
    sd      x5, 128(a0)     //xFlags
    sd      x6, 136(a0)     // put back reg value in emu, including EIP (so x27 must be EIP now)
    //restore all used register
    ld      ra, (sp)  // save ra
    ld      x8, 8(sp) // save fp
    ld      x18, 16(sp)
    ld      x19, 24(sp)
    ld      x20, 32(sp)
    ld      x21, 40(sp)
    ld      x22, 48(sp)
    ld      x23, 56(sp)
    ld      x24, 64(sp)
    ld      x25, 72(sp)
    ld      x26, 80(sp)
    ld      x27, 88(sp)
    fsd     f8, 96(sp)
    fsd     f9, 104(sp)
    addi    sp,  sp, (8 * 14)
    //end, return...
    ret


.global rv64_epilog_fast
rv64_epilog_fast:
    //restore all used register
    ld      ra, (sp)  // save ra
    ld      x8, 8(sp) // save fp
    ld      x16, 16(sp)
    ld      x17, 24(sp)
    fld     f8, 32(sp)
    fld     f9, 40(sp)
    addi    sp,  sp, (8 * (2+4))
    //end, return...
    ret
