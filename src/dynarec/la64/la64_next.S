#define ASM_MAPPING 1
#include "la64_mapping.h"
#undef ASM_MAPPING

//la64 update linker table for dynarec
//called with pointer to emu as 1st parameter
//and address of table to as 2nd parameter
//ip is at r12

.text
.align 4

.extern cpuext
.extern LinkNext
.extern LinkNextInvalid

.macro restore_vzero
    .word 0x71277bde // vxor.v $vr30, $vr30, $vr30
    la.global $r19, cpuext
    ldptr.d   $r19, $r19, 0
    andi      $r19, $r19, 0x20
    beqz      $r19, 1f
    .word 0x75277bde // xvxor.v $xr30, $xr30, $xr30
1:
.endm

.global la64_next
.global la64_next_invalid

    .8byte  0   // NULL pointer before la64_next, for getDB
la64_next:
    // move emu to a0
    // move IP address to a1
    addi.d $sp, $sp, -(8 * 12)
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
    // Save LBT.eflags too if lbt is enabled
    la.global $r19, cpuext
    ldptr.d   $r19, $r19, 0
    andi      $r19, $r19, 1
    beqz      $r19, 1f
    x86mfflag $r19, 0b111111
    st.d      $r19, $sp, 88
1:
    move   $a0, Emu
    move   $a1, RIP
    move   $a2, $ra      // "from" is in ra, so put in a2
    addi.d $a3, $sp, 80  // a3 is address to change rip
    // call the function
    bl LinkNext
    // preserve return value
    move   $r16, $a0
    restore_vzero
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
    // restore LBT.eflags
    la.global $r19, cpuext
    ldptr.d   $r19, $r19, 0
    andi      $r19, $r19, 1
    beqz      $r19, 2f
    ld.d      $r19, $sp, 88
    x86mtflag $r19, 0b111111
2:
    addi.d $sp, $sp, (8 * 12)
    // return offset is jump address
    jr     $r16

la64_next_invalid:
    // move emu to a0
    // move IP address to a1
    addi.d $sp, $sp, -(8 * 12)
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
    // Save LBT.eflags too if lbt is enabled
    la.global $r19, cpuext
    ldptr.d   $r19, $r19, 0
    andi      $r19, $r19, 1
    beqz      $r19, 1f
    x86mfflag $r19, 0b111111
    st.d      $r19, $sp, 88
1:
    move   $a0, Emu
    move   $a1, RIP
    move   $a2, $ra      // "from" is in ra, so put in a2
    addi.d $a3, $sp, 80  // a3 is address to change rip
    // call the function
    bl LinkNextInvalid
    // preserve return value
    move   $r16, $a0
    restore_vzero
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
    // restore LBT.eflags
    la.global $r19, cpuext
    ldptr.d   $r19, $r19, 0
    andi      $r19, $r19, 1
    beqz      $r19, 2f
    ld.d      $r19, $sp, 88
    x86mtflag $r19, 0b111111
2:
    addi.d $sp, $sp, (8 * 12)
    // return offset is jump address
    jr     $r16
