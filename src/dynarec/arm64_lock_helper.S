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
.global arm64_lock_xchg
.global arm64_lock_storeifnull


arm64_lock_read_b:
    // address is x0, return is x0
    ldaxrb  w0, [x0]
    ret

arm64_lock_write_b:
    // address is x0, value is x1, return is x0
    mov     x2, x0
    stlxrb  w0, w1, [x2]
    ret

arm64_lock_read_h:
    // address is x0, return is x0
    ldaxrh  w0, [x0]
    ret

arm64_lock_write_h:
    // address is x0, value is x1, return is x0
    mov     x2, x0
    stlxrh  w0, w1, [x2]
    ret

arm64_lock_read_d:
    // address is x0, return is x0
    #ldaxr   w0, [x0]
    ldr     w0,[x0]
    ret

arm64_lock_write_d:
    // address is x0, value is w1, return is x0
    mov     x2, x0
    #stlxr   w0, w1, [x2]
    str     w1, [x2]
    mov     w0, 0
    ret

arm64_lock_read_dd:
    // address is x0, return is x0
    ldaxr   x0, [x0]
    ret

arm64_lock_write_dd:
    // address is x0, value is x1, return is x0
    mov     x2, x0
    stlxr   w0, x1, [x2]
    ret

arm64_lock_xchg:
    // address is x0, value is x1, return old value in x0
    ldaxr   x2, [x0]
    stlxr   w3, x1, [x0]
    cmp     w3, #1
    beq     arm64_lock_xchg
    mov     x0, x2
    ret

arm64_lock_storeifnull:
    // address is x0, value is x1, x1 store to x0 only if [x0] is 0. return new [x0] value (so x1 or old value)
    ldaxr   x2, [x0]
    cmp     x2, #0
    bne     arm64_lock_storeifnull_exit
    mov     x2, x1
    stlxr   w3, x2, [x0]
    cmp     w3, #1
    beq     arm64_lock_storeifnull
arm64_lock_storeifnull_exit:
    mov     x0, x2
    ret
