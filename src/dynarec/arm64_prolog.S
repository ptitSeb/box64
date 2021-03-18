//arm prologue for dynarec
//Save stuff, prepare stack and register
//called with pointer to emu as 1st parameter
//and address to jump to as 2nd parameter

.text
.align 4

.global arm64_prolog
arm64_prolog:
    //save all 18 used register
    stp     lr, fp, [sp, -16]!  // save lr
    sub     sp,  sp, (8 * 10)
    str     x19, [sp, (8 * 0)]
    str     x20, [sp, (8 * 1)]
    str     x21, [sp, (8 * 2)]
    str     x22, [sp, (8 * 3)]
    str     x23, [sp, (8 * 4)]
    str     x24, [sp, (8 * 5)]
    str     x25, [sp, (8 * 6)]
    str     x26, [sp, (8 * 7)]
    str     x27, [sp, (8 * 8)]
    //vpush     {d8-d15}    // save NEON regs?
    //setup emu -> register
    ldr     x10, [x0, (8 *  0)]
    ldr     x11, [x0, (8 *  1)]
    ldr     x12, [x0, (8 *  2)]
    ldr     x13, [x0, (8 *  3)]
    ldr     x14, [x0, (8 *  4)]
    ldr     x15, [x0, (8 *  5)]
    ldr     x16, [x0, (8 *  6)]
    ldr     x17, [x0, (8 *  7)]
    ldr     x18, [x0, (8 *  8)]
    ldr     x19, [x0, (8 *  9)]
    ldr     x20, [x0, (8 * 10)]
    ldr     x21, [x0, (8 * 11)]
    ldr     x22, [x0, (8 * 12)]
    ldr     x23, [x0, (8 * 13)]
    ldr     x24, [x0, (8 * 14)]
    ldr     x25, [x0, (8 * 15)]
    ldr     x26, [x0, (8 * 16)]
    ldr     x27, [x0, (8 * 17)]
    //jump to function
    br       x1
