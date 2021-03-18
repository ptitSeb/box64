//arm epilog for dynarec
//Save stuff, prepare stack and register
//called with pointer to emu as 1st parameter
//and address to jump to as 2nd parameter

.text
.align 4

.global arm64_epilog
arm64_epilog:
    //update register -> emu
    str     x10, [x0, (8 *  0)]
    str     x11, [x0, (8 *  1)]
    str     x12, [x0, (8 *  2)]
    str     x13, [x0, (8 *  3)]
    str     x14, [x0, (8 *  4)]
    str     x15, [x0, (8 *  5)]
    str     x16, [x0, (8 *  6)]
    str     x17, [x0, (8 *  7)]
    str     x18, [x0, (8 *  8)]
    str     x19, [x0, (8 *  9)]
    str     x20, [x0, (8 * 10)]
    str     x21, [x0, (8 * 11)]
    str     x22, [x0, (8 * 12)]
    str     x23, [x0, (8 * 13)]
    str     x24, [x0, (8 * 14)]
    str     x25, [x0, (8 * 15)]
    str     x26, [x0, (8 * 16)]
    str     x27, [x0, (8 * 17)] // put back reg value in emu, including EIP (so x27 must be EIP now)
    //restore all used register
    //vpop     {d8-d15}
    ldr     x19, [sp, (8 * 0)]
    ldr     x20, [sp, (8 * 1)]
    ldr     x21, [sp, (8 * 2)]
    ldr     x22, [sp, (8 * 3)]
    ldr     x23, [sp, (8 * 4)]
    ldr     x24, [sp, (8 * 5)]
    ldr     x25, [sp, (8 * 6)]
    ldr     x26, [sp, (8 * 7)]
    ldr     x27, [sp, (8 * 8)]
    add     sp,  sp, (8 * 10)
    ldp     lr, fp, [sp], 16  // saved lr
    //end, return...
    ret


.global arm64_epilog_fast
arm64_epilog_fast:
    //restore all used register
    //vpop     {d8-d15}
    ldr     x19, [sp, (8 * 0)]
    ldr     x20, [sp, (8 * 1)]
    ldr     x21, [sp, (8 * 2)]
    ldr     x22, [sp, (8 * 3)]
    ldr     x23, [sp, (8 * 4)]
    ldr     x24, [sp, (8 * 5)]
    ldr     x25, [sp, (8 * 6)]
    ldr     x26, [sp, (8 * 7)]
    ldr     x27, [sp, (8 * 8)]
    add     sp,  sp, (8 * 10)
    ldp     lr, fp, [sp], 16  // saved lr
    //end, return...
    ret
