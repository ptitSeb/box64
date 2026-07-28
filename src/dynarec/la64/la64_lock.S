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
.global la64_snapshot_match
.global la64_snapshot_match_lasx

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

la64_crc:
    move        $a2, $a0
    xor         $a0, $a0, $a0
    li.d        $a3, 8
1:
    bltu        $a1, $a3, 2f
    ld.d        $a4, $a2, 0
    crc.w.d.w   $a0, $a4, $a0
    addi.d      $a2, $a2, 8
    addi.w      $a1, $a1, -8
    bnez        $a1, 1b
2:
    beqz        $a1, 4f
3:
    ld.bu       $a4, $a2, 0
    crc.w.b.w   $a0, $a4, $a0
    addi.d      $a2, $a2, 1
    addi.w      $a1, $a1, -1
    bnez        $a1, 3b
4:
    ret

// t2=current source, t3=snapshot, t4=len; t2=0 if equal. v24-v29 are scratch registers.
la64_snapshot_match:
    li.d        $t5, 64
1:
    bltu        $t4, $t5, 2f
    vld         $vr24, $t2, 0
    vld         $vr25, $t2, 16
    vld         $vr26, $t2, 32
    vld         $vr27, $t2, 48
    vld         $vr28, $t3, 0
    vld         $vr29, $t3, 16
    vxor.v      $vr24, $vr24, $vr28
    vxor.v      $vr25, $vr25, $vr29
    vld         $vr28, $t3, 32
    vld         $vr29, $t3, 48
    vxor.v      $vr26, $vr26, $vr28
    vxor.v      $vr27, $vr27, $vr29
    vor.v       $vr24, $vr24, $vr25
    vor.v       $vr26, $vr26, $vr27
    vor.v       $vr24, $vr24, $vr26
    vsetnez.v   $fcc0, $vr24
    bcnez       $fcc0, 8f
    addi.d      $t2, $t2, 64
    addi.d      $t3, $t3, 64
    addi.w      $t4, $t4, -64
    b           1b
2:
    li.d        $t5, 16
3:
    bltu        $t4, $t5, 4f
    vld         $vr24, $t2, 0
    vld         $vr25, $t3, 0
    vxor.v      $vr24, $vr24, $vr25
    vsetnez.v   $fcc0, $vr24
    bcnez       $fcc0, 8f
    addi.d      $t2, $t2, 16
    addi.d      $t3, $t3, 16
    addi.w      $t4, $t4, -16
    b           3b
4:
    li.d        $t5, 8
5:
    bltu        $t4, $t5, 6f
    ld.d        $t5, $t2, 0
    ld.d        $t6, $t3, 0
    bne         $t5, $t6, 8f
    addi.d      $t2, $t2, 8
    addi.d      $t3, $t3, 8
    addi.w      $t4, $t4, -8
    li.d        $t5, 8
    b           5b
6:
    beqz        $t4, 7f
    ld.bu       $t5, $t2, 0
    ld.bu       $t6, $t3, 0
    bne         $t5, $t6, 8f
    addi.d      $t2, $t2, 1
    addi.d      $t3, $t3, 1
    addi.w      $t4, $t4, -1
    b           6b
7:
    move        $t2, $zero
    ret
8:
    li.d        $t2, 1
    ret

// t2=current source, t3=snapshot, t4=len; t2=0 if equal. v24-v29 are scratch registers.
la64_snapshot_match_lasx:
    li.d        $t5, 128
1:
    bltu        $t4, $t5, 2f
    xvld        $xr24, $t2, 0
    xvld        $xr25, $t2, 32
    xvld        $xr26, $t2, 64
    xvld        $xr27, $t2, 96
    xvld        $xr28, $t3, 0
    xvld        $xr29, $t3, 32
    xvxor.v     $xr24, $xr24, $xr28
    xvxor.v     $xr25, $xr25, $xr29
    xvld        $xr28, $t3, 64
    xvld        $xr29, $t3, 96
    xvxor.v     $xr26, $xr26, $xr28
    xvxor.v     $xr27, $xr27, $xr29
    xvor.v      $xr24, $xr24, $xr25
    xvor.v      $xr26, $xr26, $xr27
    xvor.v      $xr24, $xr24, $xr26
    xvsetnez.v  $fcc0, $xr24
    bcnez       $fcc0, 8f
    addi.d      $t2, $t2, 128
    addi.d      $t3, $t3, 128
    addi.w      $t4, $t4, -128
    b           1b
2:
    li.d        $t5, 32
3:
    bltu        $t4, $t5, 4f
    xvld        $xr24, $t2, 0
    xvld        $xr25, $t3, 0
    xvxor.v     $xr24, $xr24, $xr25
    xvsetnez.v  $fcc0, $xr24
    bcnez       $fcc0, 8f
    addi.d      $t2, $t2, 32
    addi.d      $t3, $t3, 32
    addi.w      $t4, $t4, -32
    b           3b
4:
    li.d        $t5, 8
5:
    bltu        $t4, $t5, 6f
    ld.d        $t5, $t2, 0
    ld.d        $t6, $t3, 0
    bne         $t5, $t6, 8f
    addi.d      $t2, $t2, 8
    addi.d      $t3, $t3, 8
    addi.w      $t4, $t4, -8
    li.d        $t5, 8
    b           5b
6:
    beqz        $t4, 7f
    ld.bu       $t5, $t2, 0
    ld.bu       $t6, $t3, 0
    bne         $t5, $t6, 8f
    addi.d      $t2, $t2, 1
    addi.d      $t3, $t3, 1
    addi.w      $t4, $t4, -1
    b           6b
7:
    move        $t2, $zero
    ret
8:
    li.d        $t2, 1
    ret
