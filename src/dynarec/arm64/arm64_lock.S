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
.global arm64_lock_xchg_dd
.global arm64_lock_xchg_d
.global arm64_lock_xchg_h
.global arm64_lock_xchg_b
.global arm64_lock_storeifnull
.global arm64_lock_storeifnull_d
.global arm64_lock_storeifref
.global arm64_lock_storeifref_d
.global arm64_lock_storeifref2_d
.global arm64_lock_decifnot0b
.global arm64_lock_storeb
.global arm64_lock_incif0
.global arm64_lock_decifnot0
.global arm64_lock_store
.global arm64_lock_store_dd
.global arm64_lock_get_b
.global arm64_lock_get_d
.global arm64_lock_get_dd

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


arm64_lock_xchg_dd:
    dmb     ish
arm64_lock_xchg_dd_0:
    // address is x0, value is x1, return old value in x0
    ldaxr   x2, [x0]
    stlxr   w3, x1, [x0]
    cbnz    w3, arm64_lock_xchg_dd_0
    mov     x0, x2
    ret

arm64_lock_xchg_d:
    dmb     ish
arm64_lock_xchg_d_0:
    // address is x0, value is x1, return old value in x0
    ldaxr   w2, [x0]
    stlxr   w3, w1, [x0]
    cbnz    w3, arm64_lock_xchg_d_0
    mov     w0, w2
    ret

arm64_lock_xchg_h:
    dmb     ish
arm64_lock_xchg_h_0:
    // address is x0, value is x1, return old value in x0
    ldaxrh  w2, [x0]
    stlxrh  w3, w1, [x0]
    cbnz    w3, arm64_lock_xchg_h_0
    mov     w0, w2
    ret

arm64_lock_xchg_b:
    dmb     ish
arm64_lock_xchg_b_0:
    // address is x0, value is x1, return old value in x0
    ldaxrb   w2, [x0]
    stlxrb   w3, w1, [x0]
    cbnz    w3, arm64_lock_xchg_b_0
    mov     w0, w2
    ret

arm64_lock_storeifnull:
    dmb     ish
1:
    // address is x0, value is x1, x1 store to x0 only if [x0] is 0. return old [x0] value
    ldaxr   x2, [x0]
    cbnz    x2, 2f
    stlxr   w3, x1, [x0]
    cbnz    w3, 1b
2:
    mov     x0, x2
    ret

arm64_lock_storeifnull_d:
    dmb     ish
1:
    // address is x0, value is w1, w1 store to x0 only if [x0] is 0. return old [x0] value
    ldaxr   w2, [x0]
    cbnz    w2, 2f
    stlxr   w3, w1, [x0]
    cbnz    w3, 1b
2:
    dmb     ish
    mov     w0, w2
    ret

arm64_lock_storeifref:
    dmb     ish
1:
    // address is x0, value is x1, x1 store to x0 only if [x0] is x2. return new [x0] value (so x1 or old value)
    ldaxr   x3, [x0]
    cmp     x2, x3
    bne     2f
    stlxr   w4, x1, [x0]
    cbnz    w4, 1b
    mov     x0, x1
    ret
2:
    mov     x0, x3
    ret

arm64_lock_storeifref_d:
    dmb     ish
1:
    // address is x0, value is w1, w1 store to x0 only if [x0] is w2. return new [x0] value (so x1 or old value)
    ldaxr   w3, [x0]
    cmp     w2, w3
    bne     2f
    stlxr   w4, w1, [x0]
    cbnz    w4, 1b
    mov     w0, w1
    ret
2:
    mov     w0, w3
    ret

arm64_lock_storeifref2_d:
    dmb     ish
1:
    // address is x0, value is w1, w1 store to x0 only if [x0] is w2. return old [x0] value
    ldaxr   w3, [x0]
    cmp     w2, w3
    bne     2f
    stlxr   w4, w1, [x0]
    cbnz    w4, 1b
2:
    mov     w0, w3
    ret

arm64_lock_decifnot0b:
    dmb     ish
1:
    ldaxrb  w1, [x0]
    cmp     w1, #0
    beq     2f
    sub     w1, w1, #1
    stlxrb  w2, w1, [x0]
    cbnz    w2, 1b
2:
    ret

arm64_lock_storeb:
    strb    w1, [x0]
    dmb     ish
    ret

arm64_lock_decifnot0:
    dmb     ish
1:
    ldaxr   w1, [x0]
    cmp     w1, #0
    beq     2f
    sub     w3, w1, #1
    stlxr   w2, w3, [x0]
    cbnz    w2, 1b
2:
    mov     w0, w1
    ret

arm64_lock_incif0:
    dmb     ish
1:
    ldaxr   w1, [x0]
    cmp     w1, #0
    bne     2f
    add     w3, w1, #1
    stlxr   w2, w3, [x0]
    cbnz    w2, 1b
2:
    mov     w0, w1
    ret

arm64_lock_store:
    str     w1, [x0]
    dmb     ish
    ret

arm64_lock_store_dd:
    str     x1, [x0]
    dmb     ish
    ret

arm64_lock_get_b:
    ldaxrb  w0, [x0]
    ret

arm64_lock_get_d:
    ldaxr   w0, [x0]
    ret

arm64_lock_get_dd:
    ldaxr   x0, [x0]
    ret
