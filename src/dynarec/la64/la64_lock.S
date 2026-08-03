#define ASM_MAPPING 1
#include "la64_mapping.h"
#undef ASM_MAPPING

// LA64 lock helper
// there is 2 part: read and write
// write return 0 on success, 1 on fail (value has been changed)

.text
.align 4

.global la64_lock_xchg_b
.global la64_lock_xchg_dd
.global la64_lock_xchg_d
.global la64_lock_storeifnull
.global la64_lock_storeifnull_d
.global la64_lock_storeifref
.global la64_lock_storeifref2
.global la64_lock_storeifref_d
.global la64_lock_storeifref2_d
.global la64_lock_storeb
.global la64_lock_incif0
.global la64_lock_decifnot0
.global la64_lock_store
.global la64_lock_store_dd
.global la64_lock_cas_b
.global la64_lock_cas_h
.global la64_lock_cas_d
.global la64_lock_cas_dd
.global la64_lock_cas_dq
.global la64_lock_get_b
.global la64_lock_get_d
.global la64_lock_get_dd
.global la64_crc
.global la64_crc_autocrc

la64_lock_xchg_b:
    .word 0x385e1486 // amswap_db.b $a2, $a1, $a0
    move        $a0, $a2
    ret

la64_lock_xchg_dd:
    // address is a0, value is a1, return old value in a0
    amswap_db.d $a2, $a1, $a0
    move        $a0, $a2
    ret

la64_lock_xchg_d:
    // address is a0, value is a1, return old value in a0
    amswap_db.w $a2, $a1, $a0
    move        $a0, $a2
    ret

la64_lock_storeifnull:
    // address is a0, value is a1, a1 store to a0 only if [a0] is 0. return old [a0] value
    dbar 0
1:
    move $a3, $a1
    ll.d $a2, $a0, 0
    bnez $a2, 2f
    sc.d $a3, $a0, 0
    beqz $a3, 1b
2:
    move $a0, $a2
    ret

la64_lock_storeifnull_d:
    // address is a0, value is a1, a1 store to a0 only if [a0] is 0. return old [a0] value
    dbar 0
    move $a3, $a1
1:
    ll.w $a2, $a0, 0
    bnez $a2, 2f
    sc.w $a3, $a0, 0
    beqz $a3, 1b
2:
    move $a0, $a2
    ret

la64_lock_storeifref:
    // address is a0, value is a1, a1 store to a0 only if [a0] is a2. return new [a0] value (so a1 or old value)
    dbar 0
1:
    move $a4, $a1
    ll.d $a3, $a0, 0
    bne  $a2, $a3, 2f
    sc.d $a4, $a0, 0
    beqz $a4, 1b
    move $a0, $a1
    ret
2:
    move $a0, $a3
    ret

la64_lock_storeifref_d:
    // address is a0, value is a1, a1 store to a0 only if [a0] is a2. return new [a0] value (so a1 or old value)
    dbar 0
1:
    move $a4, $a1
    ll.w $a3, $a0, 0
    bne  $a2, $a3, 2f
    sc.w $a4, $a0, 0
    beqz $a4, 1b
    move $a0, $a1
    ret
2:
    move $a0, $a3
    ret

la64_lock_storeifref2_d:
    // address is a0, value is a1, a1 store to a0 only if [a0] is a2. return old [a0] value
    dbar 0
1:
    move $a4, $a1
    ll.w $a3, $a0, 0
    bne  $a2, $a3, 2f
    sc.w $a4, $a0, 0
    beqz $a4, 1b
2:
    move $a0, $a3
    ret

la64_lock_storeifref2:
    // address is a0, value is a1, a1 store to a0 only if [a0] is a2. return old [a0] value
    dbar 0
1:
    move $a4, $a1
    ll.d $a3, $a0, 0
    bne  $a2, $a3, 2f
    sc.d $a4, $a0, 0
    beqz $a4, 1b
2:
    move $a0, $a3
    ret

la64_lock_storeb:
    st.b $a1, $a0, 0
    dbar 0
    ret

la64_lock_decifnot0:
    dbar   0
1:
    ll.w   $a1, $a0, 0
    beqz   $a1, 2f
    addi.d $a1, $a1, -1
    move   $a2, $a1
    sc.w   $a2, $a0, 0
    beqz   $a2, 1b
2:
    move   $a0, $a1
    ret

la64_lock_incif0:
    dbar   0
1:
    ll.w   $a1, $a0, 0
    bnez   $a1, 2f
    addi.d $a1, $a1, 1
    move   $a2, $a1
    sc.w   $a2, $a0, 0
    beqz   $a2, 1b
2:
    move   $a0, $a1
    ret

la64_lock_store:
    st.w $a1, $a0, 0
    dbar 0
    ret

la64_lock_store_dd:
    st.d $a1, $a0, 0
    dbar 0
    ret

la64_lock_cas_b:
    ext.w.b     $a3, $a1
    .word 0x385a1885 // amcas_db.b  $a1, $a2, $a0
    xor         $a0, $a1, $a3
    sltu        $a0, $zero, $a0
    ret

la64_lock_cas_h:
    ext.w.h     $a3, $a1
    .word 0x385a9885 // amcas_db.h  $a1, $a2, $a0
    xor         $a0, $a1, $a3
    sltu        $a0, $zero, $a0
    ret

la64_lock_cas_d:
    ll.w $a3, $a0, 0
    bne  $a3, $a1, 1f
    sc.w $a2, $a0, 0
    xori $a0, $a2, 1 // sc return success in $a2, overwriting it! 0 = fail, 1 = success
    ret
1:
    li.d $a0, 1
    ret

la64_lock_cas_dd:
    ll.d $a3, $a0, 0
    bne  $a3, $a1, 1f
    sc.d $a2, $a0, 0
    xori $a0, $a2, 1
1:
    ret
    li.d $a0, 1
    ret

la64_lock_cas_dq:
    ll.d $a4, $a0, 0
    bne  $a4, $a2, 1f
    .word 0x38571c85 // sc.q $a1, $a3, $a0
    beqz $a1, 1f
    xori $a0, $a1, 1
    ret
1:
    li.d $a0, 1
    ret

la64_lock_get_b:
    dbar 0
    ld.b $a0, $a0, 0
    ret

la64_lock_get_d:
    dbar 0
    ld.w $a0, $a0, 0
    ret

la64_lock_get_dd:
    dbar 0
    ld.d $a0, $a0, 0
    ret

.macro crc6 src, len
    xor         $t5, $t5, $t5
    xor         $t6, $t6, $t6
    xor         $t7, $t7, $t7
    xor         $t8, $t8, $t8
    xor         $a3, $a3, $a3
    xor         $a4, $a4, $a4
    li.d        $a6, 48
1:
    bltu        $\len, $a6, 2f
    ld.d        $a5, $\src, 0
    crc.w.d.w   $t5, $a5, $t5
    ld.d        $a5, $\src, 8
    crc.w.d.w   $t6, $a5, $t6
    ld.d        $a5, $\src, 16
    crc.w.d.w   $t7, $a5, $t7
    ld.d        $a5, $\src, 24
    crc.w.d.w   $t8, $a5, $t8
    ld.d        $a5, $\src, 32
    crc.w.d.w   $a3, $a5, $a3
    ld.d        $a5, $\src, 40
    crc.w.d.w   $a4, $a5, $a4
    addi.d      $\src, $\src, 48
    addi.w      $\len, $\len, -48
    b           1b
2:
    li.d        $a6, 8
    bltu        $\len, $a6, 6f
    ld.d        $a5, $\src, 0
    crc.w.d.w   $t5, $a5, $t5
    addi.d      $\src, $\src, 8
    addi.w      $\len, $\len, -8
    bltu        $\len, $a6, 7f
    ld.d        $a5, $\src, 0
    crc.w.d.w   $t6, $a5, $t6
    addi.d      $\src, $\src, 8
    addi.w      $\len, $\len, -8
    bltu        $\len, $a6, 8f
    ld.d        $a5, $\src, 0
    crc.w.d.w   $t7, $a5, $t7
    addi.d      $\src, $\src, 8
    addi.w      $\len, $\len, -8
    bltu        $\len, $a6, 9f
    ld.d        $a5, $\src, 0
    crc.w.d.w   $t8, $a5, $t8
    addi.d      $\src, $\src, 8
    addi.w      $\len, $\len, -8
    bltu        $\len, $a6, 12f
    ld.d        $a5, $\src, 0
    crc.w.d.w   $a3, $a5, $a3
    addi.d      $\src, $\src, 8
    addi.w      $\len, $\len, -8
    b           13f
6:
    beqz        $\len, 10f
11:
    ld.bu       $a5, $\src, 0
    crc.w.b.w   $t5, $a5, $t5
    addi.d      $\src, $\src, 1
    addi.w      $\len, $\len, -1
    bnez        $\len, 11b
    b           10f
7:
    beqz        $\len, 10f
11:
    ld.bu       $a5, $\src, 0
    crc.w.b.w   $t6, $a5, $t6
    addi.d      $\src, $\src, 1
    addi.w      $\len, $\len, -1
    bnez        $\len, 11b
    b           10f
8:
    beqz        $\len, 10f
11:
    ld.bu       $a5, $\src, 0
    crc.w.b.w   $t7, $a5, $t7
    addi.d      $\src, $\src, 1
    addi.w      $\len, $\len, -1
    bnez        $\len, 11b
    b           10f
9:
    beqz        $\len, 10f
11:
    ld.bu       $a5, $\src, 0
    crc.w.b.w   $t8, $a5, $t8
    addi.d      $\src, $\src, 1
    addi.w      $\len, $\len, -1
    bnez        $\len, 11b
    b           10f
12:
    beqz        $\len, 10f
11:
    ld.bu       $a5, $\src, 0
    crc.w.b.w   $a3, $a5, $a3
    addi.d      $\src, $\src, 1
    addi.w      $\len, $\len, -1
    bnez        $\len, 11b
    b           10f
13:
    beqz        $\len, 10f
11:
    ld.bu       $a5, $\src, 0
    crc.w.b.w   $a4, $a5, $a4
    addi.d      $\src, $\src, 1
    addi.w      $\len, $\len, -1
    bnez        $\len, 11b
10:
    xor         $t5, $t5, $t6
    xor         $t5, $t5, $t7
    xor         $t5, $t5, $t8
    xor         $t5, $t5, $a3
    xor         $t5, $t5, $a4
.endm

// a0=source, a1=len, return combined crc32 in a0. 6-way crc32 streams xoring together at the end.
la64_crc:
    crc6 a0, a1
    move        $a0, $t5
    ret

// t2=source, t3=stored hash, t4=len; t2=0 if equal.
la64_crc_autocrc:
    st.d        $a3, $r31, 8*1    // save RCX
    st.d        $a4, $r31, 8*8    // save R8
    st.d        $a5, $r31, 8*9    // save R9
    st.d        $a6, $r31, 8*3    // save RBX
    crc6 t2, t4
    bstrpick.d  $t5, $t5, 31, 0
    xor         $t5, $t5, $t3     // compare with stored hash
    ld.d        $a3, $r31, 8*1    // restore RCX
    ld.d        $a4, $r31, 8*8    // restore R8
    ld.d        $a5, $r31, 8*9    // restore R9
    ld.d        $a6, $r31, 8*3    // restore RBX
    move        $t2, $t5
    ret
