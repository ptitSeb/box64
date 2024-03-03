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
    addi.d $sp, $sp, -(8 * 12)
    st.d   $a0, $sp, 0
    st.d   $a1, $sp, 8
    st.d   $r11, $sp, 16
    st.d   $r12, $sp, 24
    st.d   $r13, $sp, 32
    st.d   $r14, $sp, 40
    st.d   $r15, $sp, 48
    st.d   $r16, $sp, 56
    st.d   $r17, $sp, 64
    st.d   $r18, $sp, 72
    st.d   $r19, $sp, 80
    st.d   $r20, $sp, 88 // also save r20(rip) to allow change in LinkNext

    move   $a2, $ra      // "from" is in ra, so put in a2
    addi.d $a3, $sp, 88  // a3 is address to change rip
    // call the function
    bl LinkNext
    // preserve return value
    move   $a3, $a0
    // pop regs
    ld.d   $a0, $sp, 0
    ld.d   $a1, $sp, 8
    ld.d   $r11, $sp, 16
    ld.d   $r12, $sp, 24
    ld.d   $r13, $sp, 32
    ld.d   $r14, $sp, 40
    ld.d   $r15, $sp, 48
    ld.d   $r16, $sp, 56
    ld.d   $r17, $sp, 64
    ld.d   $r18, $sp, 72
    ld.d   $r19, $sp, 80
    ld.d   $r20, $sp, 88
    addi.d $sp,  $sp, (8 * 12)
    // return offset is jump address
    jr     $a3