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
    sub     sp,  sp,  (8 * 12)
    stp     x0,  x1,  [sp, (8 *  0)]
    stp     x10, x11, [sp, (8 *  2)]
    stp     x12, x13, [sp, (8 *  4)]
    stp     x14, x15, [sp, (8 *  6)]
    stp     x16, x17, [sp, (8 *  8)]
    str     x18, [sp, (8 * 10)]
    // call the function
    bl      LinkNext
    // preserve return value
    mov     x3, x0
    // pop regs
    ldp     x0, x1,   [sp, (8 *  0)]
    ldp     x10, x11, [sp, (8 *  2)]
    ldp     x12, x13, [sp, (8 *  4)]
    ldp     x14, x15, [sp, (8 *  6)]
    ldp     x16, x17, [sp, (8 *  8)]
    ldr     x18, [sp, (8 * 10)]
    add     sp,  sp, (8 * 12)
    // return offset is jump address
    br      x3

