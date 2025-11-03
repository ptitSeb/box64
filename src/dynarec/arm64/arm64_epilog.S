//arm epilog for dynarec
//Save stuff, prepare stack and register
//called with pointer to emu as 1st parameter
//and address to jump to as 2nd parameter

.text
.align 4

.global arm64_epilog
arm64_epilog:
    //update register -> emu
    stp     x10, x11, [x0, (8 *  0)]
    stp     x12, x13, [x0, (8 *  2)]
    stp     x14, x15, [x0, (8 *  4)]
    stp     x16, x17, [x0, (8 *  6)]
    stp     x18, x19, [x0, (8 *  8)]
    stp     x20, x21, [x0, (8 * 10)]
    stp     x22, x23, [x0, (8 * 12)]
    stp     x24, x25, [x0, (8 * 14)]
    stp     x26, x27, [x0, (8 * 16)] // put back reg value in emu, including EIP (so x27 must be EIP now)
    //restore all used register
#ifdef _WIN32
    ldr     x18, [x0, 3088]
#endif
    add     sp, x28, 0
    //vpop     {d8-d15}
    ldp     x19, x20, [sp, (8 * 0)]
    ldp     x21, x22, [sp, (8 * 2)]
    ldp     x23, x24, [sp, (8 * 4)]
    ldp     x25, x26, [sp, (8 * 6)]
    ldp     x27, x28, [sp, (8 * 8)]
    ldp     d8, d9,   [sp, (8 *10)]
    ldp     d10, d11, [sp, (8 *12)]
    ldp     d12, d13, [sp, (8 *14)]
    ldp     d14, d15, [sp, (8 *16)]
    add     sp,  sp, (8 * 18)
    ldp     x30, x29, [sp], 16  // saved lr
    //end, return...
    ret
