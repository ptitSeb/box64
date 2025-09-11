#define ASM_MAPPING 1
#include "la64_mapping.h"
#undef ASM_MAPPING

//loongarch prologue for dynarec
//Save stuff, prepare stack and register
//called with pointer to emu as 1st parameter
//and address to jump to as 2nd parameter

.text
.align 4

.extern cpuext

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
    // save a1
    move   $r16,  $a1
    // setup emu -> register
    move   Emu,   $a0
    ld.d   RAX,   Emu, (8 * 0)
    ld.d   RCX,   Emu, (8 * 1)
    ld.d   RDX,   Emu, (8 * 2)
    ld.d   RBX,   Emu, (8 * 3)
    ld.d   RSP,   Emu, (8 * 4)
    ld.d   RBP,   Emu, (8 * 5)
    ld.d   RSI,   Emu, (8 * 6)
    ld.d   RDI,   Emu, (8 * 7)
    ld.d   R8,    Emu, (8 * 8)
    ld.d   R9,    Emu, (8 * 9)
    ld.d   R10,   Emu, (8 * 10)
    ld.d   R11,   Emu, (8 * 11)
    ld.d   R12,   Emu, (8 * 12)
    ld.d   R13,   Emu, (8 * 13)
    ld.d   R14,   Emu, (8 * 14)
    ld.d   R15,   Emu, (8 * 15)
    ld.d   Flags, Emu, (8 * 16)  // xFlags
    ld.d   RIP,   Emu, (8 * 17)  // xRIP
    // spill xFlags to LBT.eflags
    la.global $r19, cpuext
    ldptr.d   $r19, $r19, 0
    andi      $r19, $r19, 1
    beqz      $r19, 1f
    x86mtflag Flags, 0b111111
1:
    ld.d   $r19, Emu, 808  // grab an old value of emu->xSPSave
    st.d   $sp,  Emu, 808  // save current sp to emu->xSPSave
    // push sentinel onto the stack
    st.d   $r19, $sp, -16
    st.d   $r0,  $sp, -8
    addi.d $sp,  $sp, -16
    //jump to function
    jirl   $r0,  $r16, 0
