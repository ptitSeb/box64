// RV64 lock helper
// there is 2 part: read and write
// write return 0 on success, 1 on fail (value has been changed)

.text
.align 4

.global rv64_lock_read_b
.global rv64_lock_write_b
.global rv64_lock_read_h
.global rv64_lock_write_h
.global rv64_lock_read_d
.global rv64_lock_write_d
.global rv64_lock_read_dd
.global rv64_lock_write_dd
.global rv64_lock_read_dq
.global rv64_lock_write_dq
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

rv64_lock_read_b:
    // address is a0, return is a0
    lr.w.aqrl   a0, (a0)
    andi        a0, a0, 0xff
    ret

rv64_lock_write_b:
    // address is a0, value is a1, return is a0
    andi        a1, a1, 0xff
    sc.w.aqrl   a0, a1, (a0)
    snez        a0, a0
    ret

rv64_lock_read_h:
    // address is a0, return is a0
    lr.w.aqrl   a0, (a0)
    slli        a0, a0, 48
    srli        a0, a0, 48
    ret

rv64_lock_write_h:
    // address is a0, value is a1, return is a0
    slli        a1, a1, 48
    srli        a1, a1, 48
    sc.w.aqrl   a0, a1, (a0)
    snez        a0, a0
    ret

rv64_lock_read_d:
    // address is a0, return is a0
    lr.w.aqrl   a0, (a0)
    ret


rv64_lock_write_d:
    // address is a0, value is a1, return is a0
    sc.w.aqrl   a0, a1, (a0)
    snez        a0, a0
    ret

rv64_lock_read_dd:
    // address is a0, return is a0
    lr.d.aqrl   a0, (a0)
    ret

rv64_lock_write_dd:
    // address is a0, value is a1, return is a0
    sc.d.aqrl   a0, a1, (a0)
    snez        a0, a0
    ret

rv64_lock_read_dq:
    // address is a2, return a0, a1
    fence   rw, rw
    lr.d    a3, (a2)
    sd      a3, (a0)
    addi    a2, a2, 8
    lr.d    a3, (a2)
    sd      a3, (a1)
    ret

rv64_lock_write_dq:
    // address is a2, value is a0, a1, return is a0
    sc.d    a3, a0, (a2)
    addi    a2, a2, 8
    sc.d    a4, a1, (a2)
    fence   rw, rw
    add     a0, a3, a4
    snez    a0, a0
    ret

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
