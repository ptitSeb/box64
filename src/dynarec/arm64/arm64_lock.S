//arm lock helper
//there is 2 part: read and write
// write return 0 on success, 1 on fail (value has been changed)

.text
.align 4

.global arm64_lock_read_b
.global arm64_lock_write_b
.global arm64_lock_read_h
.global arm64_lock_write_h
.global arm64_lock_read_d
.global arm64_lock_write_d
.global arm64_lock_read_dd
.global arm64_lock_write_dd
.global arm64_lock_read_dq
.global arm64_lock_write_dq
.global arm64_lock_xchg
.global arm64_lock_xchg_d
.global arm64_lock_storeifnull
.global arm64_lock_storeifref
.global arm64_lock_decifnot0b
.global arm64_lock_storeb
.global arm64_lock_incif0
.global arm64_lock_decifnot0
.global arm64_lock_store

arm64_lock_read_b:
    dmb     ish
    // address is x0, return is x0
    ldaxrb  w0, [x0]
    ret

arm64_lock_write_b:
    // address is x0, value is x1, return is x0
    mov     x2, x0
    stlxrb  w0, w1, [x2]
    dmb     ish
    ret

arm64_lock_read_h:
    dmb     ish
    // address is x0, return is x0
    ldaxrh  w0, [x0]
    ret

arm64_lock_write_h:
    // address is x0, value is x1, return is x0
    mov     x2, x0
    stlxrh  w0, w1, [x2]
    dmb     ish
    ret

arm64_lock_read_d:
    dmb     ish
    // address is x0, return is x0
    ldaxr    w0, [x0]
    ret

arm64_lock_write_d:
    // address is x0, value is w1, return is x0
    mov     x2, x0
    stlxr   w0, w1, [x2]
    dmb     ish
    ret

arm64_lock_read_dd:
    dmb     ish
    // address is x0, return is x0
    ldaxr   x0, [x0]
    ret

arm64_lock_write_dd:
    // address is x0, value is x1, return is x0
    mov     x2, x0
    stlxr   w0, x1, [x2]
    dmb     ish
    ret

arm64_lock_read_dq:
    dmb     ish
    // address is r2, return is r0, r1
    ldaxp   x4, x3, [x2]
    str     x4, [x0]
    str     x3, [x1]
    ret

arm64_lock_write_dq:
    // address is r2, value is r0, r1, return is r0
    // r0 needs to be aligned
    stlxp   w3, x0, x1, [x2]
    mov     w0, w3
    dmb     ish
    ret


arm64_lock_xchg:
    dmb     ish
arm64_lock_xchg_0:
    // address is x0, value is x1, return old value in x0
    ldaxr   x2, [x0]
    stlxr   w3, x1, [x0]
    cbnz    w3, arm64_lock_xchg_0
    dmb     ish
    mov     x0, x2
    ret

arm64_lock_xchg_d:
    dmb     ish
arm64_lock_xchg_d_0:
    // address is x0, value is x1, return old value in x0
    ldaxr   w2, [x0]
    stlxr   w3, w1, [x0]
    cbnz    w3, arm64_lock_xchg_d_0
    dmb     ish
    mov     w0, w2
    ret

arm64_lock_storeifnull:
    dmb     ish
arm64_lock_storeifnull_0:
    // address is x0, value is x1, x1 store to x0 only if [x0] is 0. return new [x0] value (so x1 or old value)
    ldaxr   x2, [x0]
    cbnz    x2, arm64_lock_storeifnull_exit
    mov     x2, x1
    stlxr   w3, x2, [x0]
    cbnz    w3, arm64_lock_storeifnull_0
arm64_lock_storeifnull_exit:
    dmb     ish
    mov     x0, x2
    ret

arm64_lock_storeifref:
    dmb     ish
arm64_lock_storeifref_0:
    // address is x0, value is x1, x1 store to x0 only if [x0] is x3. return new [x0] value (so x1 or old value)
    ldaxr   x3, [x0]
    cmp     x2, x3
    bne     arm64_lock_storeifref_exit
    stlxr   w4, x1, [x0]
    cbnz    w4, arm64_lock_storeifref_0
    mov     x0, x1
    ret
arm64_lock_storeifref_exit:
    dmb     ish
    mov     x0, x3
    ret

arm64_lock_decifnot0b:
    dmb     ish
arm64_lock_decifnot0b_0:
    ldaxrb  w1, [x0]
    cmp     w1, #0
    beq     arm64_lock_decifnot0b_exit
    sub     w1, w1, #1
    stlxrb  w2, w1, [x0]
    cbnz    w2, arm64_lock_decifnot0b_0
arm64_lock_decifnot0b_exit:
    dmb     ish
    ret

arm64_lock_storeb:
    dmb     ish
    strb    w1, [x0]
    dmb     ish
    ret

arm64_lock_decifnot0:
    dmb     ish
arm64_lock_decifnot0_0:
    ldaxr   w1, [x0]
    cmp     w1, #0
    bne     arm64_lock_decifnot0_exit
    sub     w3, w1, #1
    stlxr   w2, w3, [x0]
    cbnz    w2, arm64_lock_decifnot0_0
arm64_lock_decifnot0_exit:
    dmb     ish
    mov     w0, w1
    ret

arm64_lock_incif0:
    dmb     ish
arm64_lock_incif0_0:
    ldaxr   w1, [x0]
    cmp     w1, #0
    beq     arm64_lock_incif0_exit
    add     w3, w1, #1
    stlxr   w2, w3, [x0]
    cbnz    w2, arm64_lock_incif0_0
arm64_lock_incif0_exit:
    dmb     ish
    mov     w0, w1
    ret

arm64_lock_store:
    dmb     ish
    str     w1, [x0]
    dmb     ish
    ret
