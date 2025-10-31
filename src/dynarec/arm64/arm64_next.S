//arm update linker table for dynarec
//called with pointer to emu as 1st parameter
//and address of table to as 2nd parameter
//ip is at r12

.text
.align 4

.extern LinkNext

.global arm64_next

    .8byte  0   // NULL pointer before arm64_next, for getDB
arm64_next:
    // emu is r0
    // IP address is r1
    sub     sp,  sp,  (8 * 12)
    stp     x0,  x1,  [sp, (8 *  0)]
    stp     x10, x11, [sp, (8 *  2)]
    stp     x12, x13, [sp, (8 *  4)]
    stp     x14, x15, [sp, (8 *  6)]
    stp     x16, x17, [sp, (8 *  8)]
    stp     x18, x27, [sp, (8 * 10)]    // also save x27(rip) to allow change in LinkNext

#ifdef _WIN32
    ldr     x18, [x0, 3104]
#endif
    mov     x1, x27     // xRIP, should not be needed most of the time
    mov     x2, x30      // "from" is in lr, so put in x2
    add     x3, sp, 8*11    // x3 is address to change rip
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
    ldp     x18, x27, [sp, (8 * 10)]
    add     sp,  sp, (8 * 12)
    // return offset is jump address
    br      x3

