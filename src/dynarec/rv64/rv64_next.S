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
    addi    sp,  sp,  -(8 * 10)
    sd      a0, (sp)
    sd      a1, 8(sp)
    sd      x5, 16(sp)
    sd      x7, 24(sp)
    sd      x16, 32(sp)
    sd      x17, 40(sp)
    sd      x28, 48(sp)
    sd      x29, 56(sp)
    sd      x30, 64(sp)
    sd      x31, 72(sp)

    mv      a2, ra      // "from" is in ra, so put in a2
    addi    a3, sp, 24   // a3 is address to change rip
    // call the function
1:
    auipc   a4, %pcrel_hi(LinkNext)
    jalr    a4, %pcrel_lo(1b)
    // preserve return value
    mv      a3, a0
    // pop regs
    ld      a0, (sp)
    ld      a1, 8(sp)
    ld      x5, 16(sp)
    ld      x7, 24(sp)
    ld      x16, 32(sp)
    ld      x17, 40(sp)
    ld      x28, 48(sp)
    ld      x29, 56(sp)
    ld      x30, 64(sp)
    ld      x31, 72(sp)
    addi    sp,  sp,  (8 * 10)
    // return offset is jump address
    jr      a3

