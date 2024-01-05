//arm prologue for dynarec
//Save stuff, prepare stack and register
//called with pointer to emu as 1st parameter
//and address to jump to as 2nd parameter

.text
.align 4

.global arm64_prolog
arm64_prolog:
    //save all 18 used register
    stp     x30, x29, [sp, -16]!  // save lr
    sub     sp,  sp, (8 * 18)
    stp     x19, x20, [sp, (8 * 0)]
    stp     x21, x22, [sp, (8 * 2)]
    stp     x23, x24, [sp, (8 * 4)]
    stp     x25, x26, [sp, (8 * 6)]
    stp     x27, X28, [sp, (8 * 8)]
    stp     d8, d9,   [sp, (8 *10)]
    stp     d10, d11, [sp, (8 *12)]
    stp     d12, d13, [sp, (8 *14)]
    stp     d14, d15, [sp, (8 *16)]
    //vpush     {d8-d15}    // save NEON regs?
    //setup emu -> register
    ldp     x10, x11, [x0, (8 *  0)]
    ldp     x12, x13, [x0, (8 *  2)]
    ldp     x14, x15, [x0, (8 *  4)]
    ldp     x16, x17, [x0, (8 *  6)]
    ldp     x18, x19, [x0, (8 *  8)]
    ldp     x20, x21, [x0, (8 * 10)]
    ldp     x22, x23, [x0, (8 * 12)]
    ldp     x24, x25, [x0, (8 * 14)]
    ldp     x26, x27, [x0, (8 * 16)]
    // Push a jump next on the stack
    stp     xzr, xzr, [sp, -16]!
    // Save old xSP in x28
    add     x28, sp, 16
    //jump to function
    br       x1
