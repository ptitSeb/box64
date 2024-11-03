//riscv update linker table for dynarec
//called with pointer to emu as 1st parameter
//and address of table to as 2nd parameter
//ip is at r12

.text
.align 4

.extern LinkNext

.global rv64_next

    .8byte  0   // NULL pointer before rv64_next, for getDB
rv64_next:
    // emu is a0
    // IP address is a1
    addi    sp,  sp,  -(8 * 2)
    sd      a0, (sp)
    sd      a1, 8(sp)
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

    mv      a2, ra      // "from" is in ra, so put in a2

    // call the function
1:
    auipc   a4, %pcrel_hi(LinkNext)
    jalr    a4, %pcrel_lo(1b)
    // preserve return value
    mv      a3, a0
    // pop regs
    ld      a0, (sp)
    ld      a1, 8(sp)
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
    addi    sp,  sp,  (8 * 2)
    // return offset is jump address
    jr      a3

