#define ASM_MAPPING 1
#include "la64_mapping.h"
#undef ASM_MAPPING

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
    // move emu to a0
    // move IP address to a1
    addi.d $sp, $sp, -(8 * 11)
    st.d   RDI, $sp, 0
    st.d   RSI, $sp, 8
    st.d   RDX, $sp, 16
    st.d   RCX, $sp, 24
    st.d   R8,  $sp, 32
    st.d   R9,  $sp, 40
    st.d   RAX, $sp, 48
    st.d   RBX, $sp, 56
    st.d   RSP, $sp, 64
    st.d   RBP, $sp, 72
    st.d   RIP, $sp, 80 // also save r29(rip) to allow change in LinkNext

    move   $a0, Emu
    move   $a1, RIP
    move   $a2, $ra      // "from" is in ra, so put in a2
    addi.d $a3, $sp, 80  // a3 is address to change rip
    // call the function
    bl LinkNext
    // preserve return value
    move   $r16, $a0
    // pop regs
    ld.d   RDI, $sp, 0
    ld.d   RSI, $sp, 8
    ld.d   RDX, $sp, 16
    ld.d   RCX, $sp, 24
    ld.d   R8,  $sp, 32
    ld.d   R9,  $sp, 40
    ld.d   RAX, $sp, 48
    ld.d   RBX, $sp, 56
    ld.d   RSP, $sp, 64
    ld.d   RBP, $sp, 72
    ld.d   RIP, $sp, 80
    addi.d $sp, $sp, (8 * 11)
    // return offset is jump address
    jr     $r16
