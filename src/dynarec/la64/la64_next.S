//la64 update linker table for dynarec
//called with pointer to emu as 1st parameter
//and address of table to as 2nd parameter
//ip is at r12

.text
.align 4

.extern LinkNext

.global la64_next

    .8byte  0   // NULL pointer before la64_next, for getDB
la64_next:
    // emu is a0
    // IP address is a1
    addi.d $sp, $sp, -(8 * 2)
    st.d   $a0, $sp, 0
    st.d   $a1, $sp, 8
    st.d   $r12, $a0, (8 * 0)
    st.d   $r13, $a0, (8 * 1)
    st.d   $r14, $a0, (8 * 2)
    st.d   $r15, $a0, (8 * 3)
    st.d   $r16, $a0, (8 * 4)
    st.d   $r17, $a0, (8 * 5)
    st.d   $r18, $a0, (8 * 6)
    st.d   $r19, $a0, (8 * 7)
    st.d   $r23, $a0, (8 * 8)
    st.d   $r24, $a0, (8 * 9)
    st.d   $r25, $a0, (8 * 10)
    st.d   $r26, $a0, (8 * 11)
    st.d   $r27, $a0, (8 * 12)
    st.d   $r28, $a0, (8 * 13)
    st.d   $r29, $a0, (8 * 14)
    st.d   $r30, $a0, (8 * 15)
    // restore xFlags from LBT.eflags
    la.global $r12, la64_lbt
    ldptr.d   $r12, $r12, 0
    beqz      $r12, 1f
    ori       $r13, $r0, 0b100011010101
    andn      $r31, $r31, $r13
    x86mfflag $r13, 0b111111
    or        $r31, $r31, $r13
1:
    st.d   $r31, $a0, (8 * 16) // xFlags
    st.d   $r20, $a0, (8 * 17) // put back reg value in emu, including EIP (so $r20 must be EIP now)

    move   $a2, $ra      // "from" is in ra, so put in a2
    // call the function
    bl LinkNext
    // preserve return value
    move   $a3, $a0
    // pop regs
    ld.d   $a0, $sp, 0
    ld.d   $a1, $sp, 8
    ld.d   $r12, $a0, (8 * 0)
    ld.d   $r13, $a0, (8 * 1)
    ld.d   $r14, $a0, (8 * 2)
    ld.d   $r15, $a0, (8 * 3)
    ld.d   $r16, $a0, (8 * 4)
    ld.d   $r17, $a0, (8 * 5)
    ld.d   $r18, $a0, (8 * 6)
    ld.d   $r19, $a0, (8 * 7)
    ld.d   $r23, $a0, (8 * 8)
    ld.d   $r24, $a0, (8 * 9)
    ld.d   $r25, $a0, (8 * 10)
    ld.d   $r26, $a0, (8 * 11)
    ld.d   $r27, $a0, (8 * 12)
    ld.d   $r28, $a0, (8 * 13)
    ld.d   $r29, $a0, (8 * 14)
    ld.d   $r30, $a0, (8 * 15)
    ld.d   $r31, $a0, (8 * 16)  // xFlags
    ld.d   $r20, $a0, (8 * 17)  // xRIP
    addi.d $sp,  $sp, (8 * 2)
    // return offset is jump address
    jr     $a3