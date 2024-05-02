// RV64 lock helper
// there is 2 part: read and write
// write return 0 on success, 1 on fail (value has been changed)

.text
.align 4

.global la64_lock_xchg_dd
.global la64_lock_xchg_d
.global la64_lock_storeifnull
.global la64_lock_storeifnull_d
.global la64_lock_storeifref
.global la64_lock_storeifref_d
.global la64_lock_storeifref2_d
.global la64_lock_decifnot0b
.global la64_lock_storeb
.global la64_lock_incif0
.global la64_lock_decifnot0
.global la64_lock_store
.global la64_lock_store_dd
.global la64_lock_get_b
.global la64_lock_get_d
.global la64_lock_get_dd
.global la64_lock_cas_d
.global la64_lock_cas_dd
.global la64_lock_cas_dq

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
    move $a3, $a1
    ll.d $a2, $a0, 0
    bnez $a2, 12
    sc.d $a3, $a0, 0
    beqz $a3, -16
    move $a0, $a2
    ret

la64_lock_storeifnull_d:
    // address is a0, value is a1, a1 store to a0 only if [a0] is 0. return old [a0] value
    dbar 0
    move $a3, $a1
    ll.w $a2, $a0, 0
    bnez $a2, 12
    sc.w $a3, $a0, 0
    beqz $a3, -16
    move $a0, $a2
    ret

la64_lock_storeifref:
    // address is a0, value is a1, a1 store to a0 only if [a0] is a2. return new [a0] value (so a1 or old value)
    dbar 0
    move $a4, $a1
    ll.d $a3, $a0, 0
    bne  $a2, $a3, 24
    sc.d $a4, $a0, 0
    beqz $a4, -16
    dbar 0
    move $a0, $a1
    ret
    dbar 0
    move $a0, $a3
    ret

la64_lock_storeifref_d:
    // address is a0, value is a1, a1 store to a0 only if [a0] is a2. return new [a0] value (so a1 or old value)
    dbar 0
    move $a4, $a1  
    ll.w $a3, $a0, 0
    bne  $a2, $a3, 24
    sc.w $a4, $a0, 0
    beqz $a4, -16
    dbar 0
    move $a0, $a1
    ret
    dbar 0
    move $a0, $a3
    ret

la64_lock_storeifref2_d:
    // address is a0, value is a1, a1 store to a0 only if [a0] is a2. return old [a0] value
    dbar 0
    move $a4, $a1
    ll.w $a3, $a0, 0
    bne  $a2, $a3, 12
    sc.w $a4, $a0, 0
    beqz $a4, -16
    move $a0, $a3
    ret

la64_lock_decifnot0b:
    dbar       0
    // TODO
    ret

la64_lock_storeb:
    st.b $a1, $a0, 0
    dbar 0
    ret

la64_lock_decifnot0:
    dbar   0
    ll.w   $a1, $a0, 0
    beqz   $a1, 20
    addi.d $a1, $a1, -1
    move   $a2, $a1
    sc.w   $a2, $a0, 0
    beqz   $a2, -20
    move   $a0, $a1
    ret

la64_lock_incif0:
    dbar   0
    ll.w   $a1, $a0, 0
    bnez   $a1, 20
    addi.d $a1, $a1, 1
    move   $a2, $a1
    sc.w   $a2, $a0, 0
    beqz   $a2, -20
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

la64_lock_cas_d:
    ll.w $a3, $a0, 0
    bne  $a3, $a1, 16
    sc.w $a2, $a0, 0
    li.d $a0, 0
    ret
    li.d $a0, 1
    ret

la64_lock_cas_dd:
    ll.d $a3, $a0, 0
    bne  $a3, $a1, 1f
    sc.d $a2, $a0, 0
    li.d $a0, 0
1:
    ret
    li.d $a0, 1
    ret

la64_lock_cas_dq:
    ll.d $a4, $a0, 0
    bne  $a4, $a2, 1f
    sc.d $a1, $a0, 0
    st.d $a3, $a0, 8
    li.d $a0, 0
    ret
1:
    li.d $a0, 1
    ret
