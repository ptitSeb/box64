//arm update linker table for dynarec
//called with pointer to emu as 1st parameter
//and address of table to as 2nd parameter
//ip is at r12

.text
.align 4

.extern LinkNext

.global arm64_next
arm64_next:
    // emu is r0
    // IP address is r1
    sub     sp,  sp, (8 * 12)
    str     x0,  [sp, (8 *  0)]
    str     x1,  [sp, (8 *  1)]
    str     x10, [sp, (8 *  2)]
    str     x11, [sp, (8 *  3)]
    str     x12, [sp, (8 *  4)]
    str     x13, [sp, (8 *  5)]
    str     x14, [sp, (8 *  6)]
    str     x15, [sp, (8 *  7)]
    str     x16, [sp, (8 *  8)]
    str     x17, [sp, (8 *  9)]
    str     x18, [sp, (8 * 10)]
    // call the function
    bl      LinkNext
    // preserve return value
    mov     x3, x0
    // pop regs
    ldr     x0,  [sp, (8 *  0)]
    ldr     x1,  [sp, (8 *  1)]
    ldr     x10, [sp, (8 *  2)]
    ldr     x11, [sp, (8 *  3)]
    ldr     x12, [sp, (8 *  4)]
    ldr     x13, [sp, (8 *  5)]
    ldr     x14, [sp, (8 *  6)]
    ldr     x15, [sp, (8 *  7)]
    ldr     x16, [sp, (8 *  8)]
    ldr     x17, [sp, (8 *  9)]
    ldr     x18, [sp, (8 * 10)]
    add     sp,  sp, (8 * 12)
    // return offset is jump address
    br      x3

