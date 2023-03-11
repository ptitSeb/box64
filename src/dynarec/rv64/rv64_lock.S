// RV64 lock helper
// there is 2 part: read and write
// write return 0 on success, 1 on fail (value has been changed)

.text
.align 4

.global rv64_lock_xchg
.global rv64_lock_xchg_d
.global rv64_lock_storeifnull
.global rv64_lock_storeifnull_d
.global rv64_lock_storeifref
.global rv64_lock_storeifref_d
.global rv64_lock_storeifref2_d
.global rv64_lock_decifnot0b
.global rv64_lock_storeb
.global rv64_lock_incif0
.global rv64_lock_decifnot0
.global rv64_lock_store
.global rv64_lock_cas_d
.global rv64_lock_cas_dd
.global rv64_lock_cas_dq

rv64_lock_xchg:
    // address is a0, value is a1, return old value in a0
    amoswap.d.aqrl  a0, a1, (a0)
    ret

rv64_lock_xchg_d:
    // address is a0, value is a1, return old value in a0
    amoswap.w.aqrl  a0, a1, (a0)
    ret

rv64_lock_storeifnull:
    // address is a0, value is a1, a1 store to a0 only if [a0] is 0. return old [a0] value
    fence   rw, rw
1:    
    lr.d    a2, (a0)
    bnez    a2, 2f
    sc.d    a3, a1, (a0)
    bnez    a3, 1b
2:
    mv      a0, a2
    ret

rv64_lock_storeifnull_d:
    // address is a0, value is a1, a1 store to a0 only if [a0] is 0. return old [a0] value
    fence   rw, rw
1:    
    lr.w    a2, (a0)
    bnez    a2, 2f
    sc.w    a3, a1, (a0)
    bnez    a3, 1b
2:
    mv      a0, a2
    ret

rv64_lock_storeifref:
    // address is a0, value is a1, a1 store to a0 only if [a0] is a2. return new [a0] value (so a1 or old value)
    fence   rw, rw
1:    
    lr.d    a3, (a0)
    bne     a2, a3, 2f
    sc.d    a4, a1, (a0)
    bnez    a4, 1b
    fence   rw, rw
    mv      a0, a1
    ret
2:
    fence   rw, rw
    mv      a0, a3
    ret

rv64_lock_storeifref_d:
    // address is a0, value is a1, a1 store to a0 only if [a0] is a2. return new [a0] value (so a1 or old value)
    fence   rw, rw
1:    
    lr.w    a3, (a0)
    bne     a2, a3, 2f
    sc.w    a4, a1, (a0)
    bnez    a4, 1b
    mv      a0, a1
    ret
2:
    mv      a0, a3
    ret

rv64_lock_storeifref2_d:
    // address is a0, value is a1, a1 store to a0 only if [a0] is a2. return old [a0] value
    fence   rw, rw
1:    
    lr.w    a3, (a0)
    bne     a2, a3, 2f
    sc.w    a4, a1, (a0)
    bnez    a4, 1b
2:
    mv      a0, a3
    ret

rv64_lock_decifnot0b:
    fence   rw, rw
1:
    lr.w    a1, (a0)
    andi    a1, a1, 0xff
    beqz    a1, 2f
    addi    a1, a1, -1
    sc.w    a2, a1, (a0)
    bnez    a2, 1b
2:
    ret

rv64_lock_storeb:
    sb      a1, 0(a0)
    fence   rw, rw
    ret

rv64_lock_decifnot0:
    fence   rw, rw
1:
    lr.w    a1, (a0)
    beqz    a1, 2f
    addi    a1, a1, -1
    sc.w    a2, a1, (a0)
    bnez    a2, 1b
2:
    mv      a0, a1
    ret

rv64_lock_incif0:
    fence   rw, rw
1:
    lr.w    a1, (a0)
    bnez    a1, 2f
    addi    a1, a1, 1
    sc.w    a2, a1, (a0)
    bnez    a2, 1b
2:
    mv      a0, a1
    ret

rv64_lock_store:
    sw      a1, 0(a0)
    fence   rw, rw
    ret

rv64_lock_cas_d:
    lr.w t0, (a0)
    bne t0, a1, 1f
    sc.w a0, a2, (a0)
    ret
1:
    li a0, 1
    ret

rv64_lock_cas_dd:
    lr.d t0, (a0)
    bne t0, a1, 1f
    sc.d a0, a2, (a0)
    ret
1:
    li a0, 1
    ret

rv64_lock_cas_dq:
    mv a4, a0
    lr.d t0, (a0)
    bne t0, a1, 1f
    sc.d a0, a2, (a0)
    sd a3,(a4)
    ret
1:
    li a0, 1
    ret