//loongarch prologue for dynarec
//Save stuff, prepare stack and register
//called with pointer to emu as 1st parameter
//and address to jump to as 2nd parameter

.text
.align 4

.extern la64_lbt

.global la64_prolog
la64_prolog:
    //save all 18 used register
    addi.d $sp,  $sp, -(8 * 19)
    st.d   $r1,  $sp, (8 * 0) //save ra
    st.d   $r22, $sp, (8 * 1) //save fp
    // save s0 - s8
    st.d   $r23, $sp, (8 * 2)
    st.d   $r24, $sp, (8 * 3)
    st.d   $r25, $sp, (8 * 4)
    st.d   $r26, $sp, (8 * 5)
    st.d   $r27, $sp, (8 * 6)
    st.d   $r28, $sp, (8 * 7)
    st.d   $r29, $sp, (8 * 8)
    st.d   $r30, $sp, (8 * 9)
    st.d   $r31, $sp, (8 * 10)
    // save fs0 - fs7
    fst.d  $f24, $sp, (8 * 11)
    fst.d  $f25, $sp, (8 * 12)
    fst.d  $f26, $sp, (8 * 13)
    fst.d  $f27, $sp, (8 * 14)
    fst.d  $f28, $sp, (8 * 15)
    fst.d  $f29, $sp, (8 * 16)
    fst.d  $f30, $sp, (8 * 17)
    fst.d  $f31, $sp, (8 * 18)
    // setup emu -> register
    ld.d   $r12, $r4, (8 * 0)
    ld.d   $r13, $r4, (8 * 1)
    ld.d   $r14, $r4, (8 * 2)
    ld.d   $r15, $r4, (8 * 3)
    ld.d   $r16, $r4, (8 * 4)
    ld.d   $r17, $r4, (8 * 5)
    ld.d   $r18, $r4, (8 * 6)
    ld.d   $r19, $r4, (8 * 7)
    ld.d   $r23, $r4, (8 * 8)
    ld.d   $r24, $r4, (8 * 9)
    ld.d   $r25, $r4, (8 * 10)
    ld.d   $r26, $r4, (8 * 11)
    ld.d   $r27, $r4, (8 * 12)
    ld.d   $r28, $r4, (8 * 13)
    ld.d   $r29, $r4, (8 * 14)
    ld.d   $r30, $r4, (8 * 15)
    ld.d   $r31, $r4, (8 * 16)  // xFlags
    ld.d   $r20, $r4, (8 * 17)  // xRIP
    // spill xFlags to LBT.eflags
    la.global $a6, la64_lbt
    ldptr.d   $a6, $a6, 0
    beqz      $a6, 1f
    x86mtflag $r31, 0b111111
1:
    // push sentinel onto the stack
    st.d   $r0, $sp, -16
    st.d   $r0,  $sp, -8
    addi.d $sp,  $sp, -16
    // save old sp into xSavedSP
    addi.d $r22, $sp, 16
    // setup xMASK
    addi.w  $r11, $r0, -1
    lu32i.d $r11, 0
    //jump to function
    jirl   $r0,  $a1, 0
