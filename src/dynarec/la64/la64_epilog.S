#define ASM_MAPPING 1
#include "la64_mapping.h"
#undef ASM_MAPPING

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
    st.d   RAX, Emu, (8 * 0)
    st.d   RCX, Emu, (8 * 1)
    st.d   RDX, Emu, (8 * 2)
    st.d   RBX, Emu, (8 * 3)
    st.d   RSP, Emu, (8 * 4)
    st.d   RBP, Emu, (8 * 5)
    st.d   RSI, Emu, (8 * 6)
    st.d   RDI, Emu, (8 * 7)
    st.d   R8,  Emu, (8 * 8)
    st.d   R9,  Emu, (8 * 9)
    st.d   R10, Emu, (8 * 10)
    st.d   R11, Emu, (8 * 11)
    st.d   R12, Emu, (8 * 12)
    st.d   R13, Emu, (8 * 13)
    st.d   R14, Emu, (8 * 14)
    st.d   R15, Emu, (8 * 15)
    // restore xFlags from LBT.eflags
    la.global $r19, cpuext
    ldptr.d   $r19, $r19, 0
    andi      $r19, $r19, 1
    beqz      $r19, 1f
    ori       $r19, $r0, 0b100011010101
    andn      Flags, Flags, $r19
    x86mfflag $r19, 0b111111
    or        Flags, Flags, $r19
1:
    st.d   Flags, Emu, (8 * 16) // xFlags
    st.d   RIP,   Emu, (8 * 17) // put back reg value in emu, including EIP (so $r28 must be EIP now)
    // fallback to epilog_fast now, just restoring saved regs
la64_epilog_fast:
    // restore saved sp from emu->xSPSave
    ld.d   $sp, Emu, 808
    ld.d   $r19, $sp, -8
    // put back old value
    st.d   $r19, Emu, 808
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
