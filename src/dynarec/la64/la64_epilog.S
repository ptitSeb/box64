//la64 epilog for dynarec
//Save stuff, prepare stack and register
//called with pointer to emu as 1st parameter
//and address to jump to as 2nd parameter

.text
.align 4

.global la64_epilog
.global la64_epilog_fast

la64_epilog:
    // update register -> emu
    st.d   $r12, $r4, (8 * 0)
    st.d   $r13, $r4, (8 * 1)
    st.d   $r14, $r4, (8 * 2)
    st.d   $r15, $r4, (8 * 3)
    st.d   $r16, $r4, (8 * 4)
    st.d   $r17, $r4, (8 * 5)
    st.d   $r18, $r4, (8 * 6)
    st.d   $r19, $r4, (8 * 7)
    st.d   $r23, $r4, (8 * 8)
    st.d   $r24, $r4, (8 * 9)
    st.d   $r25, $r4, (8 * 10)
    st.d   $r26, $r4, (8 * 11)
    st.d   $r27, $r4, (8 * 12)
    st.d   $r28, $r4, (8 * 13)
    st.d   $r29, $r4, (8 * 14)
    st.d   $r30, $r4, (8 * 15)
    // restore xFlags from LBT.eflags
    la.global $r12, la64_lbt
    ldptr.d   $r12, $r12, 0
    beqz      $r12, 1f
    ori       $r13, $r0, 0b100011010101
    andn      $r31, $r31, $r13
    x86mfflag $r13, 0b111111
    or        $r31, $r31, $r13
1:
    st.d   $r31, $r4, (8 * 16) // xFlags
    st.d   $r20, $r4, (8 * 17) // put back reg value in emu, including EIP (so $r20 must be EIP now)
    // fallback to epilog_fast now, just restoring saved regs
la64_epilog_fast:
    addi.d $sp, $r22, 0       // restore save sp from xSavedSP
    // restore all used register
    ld.d   $r1,  $sp, (8 * 0) // load ra
    ld.d   $r22, $sp, (8 * 1) // load fp
    ld.d   $r23, $sp, (8 * 2)
    ld.d   $r24, $sp, (8 * 3)
    ld.d   $r25, $sp, (8 * 4)
    ld.d   $r26, $sp, (8 * 5)
    ld.d   $r27, $sp, (8 * 6)
    ld.d   $r28, $sp, (8 * 7)
    ld.d   $r29, $sp, (8 * 8)
    ld.d   $r30, $sp, (8 * 9)
    ld.d   $r31, $sp, (8 * 10)
    fld.d  $f24, $sp, (8 * 11)
    fld.d  $f25, $sp, (8 * 12)
    fld.d  $f26, $sp, (8 * 13)
    fld.d  $f27, $sp, (8 * 14)
    fld.d  $f28, $sp, (8 * 15)
    fld.d  $f29, $sp, (8 * 16)
    fld.d  $f30, $sp, (8 * 17)
    fld.d  $f31, $sp, (8 * 18)
    addi.d $sp,  $sp, (8 * 19)
    // end, return
    ret
