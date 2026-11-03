// SPDX-License-Identifier: MIT
// RV64 lock helper
// there is 2 part: read and write
// write return 0 on success, 1 on fail (value has been changed)

.text
.align 4

.global rv64_lock_xchg_dd
.global rv64_lock_xchg_d
.global rv64_lock_storeifnull
.global rv64_lock_storeifnull_d
.global rv64_lock_storeifref
.global rv64_lock_storeifref2
.global rv64_lock_storeifref_d
.global rv64_lock_storeifref2_d
.global rv64_lock_decifnot0b
.global rv64_lock_storeb
.global rv64_lock_incif0
.global rv64_lock_inc
.global rv64_lock_decifnot0
.global rv64_lock_store
.global rv64_lock_store_dd
.global rv64_lock_cas_d
.global rv64_lock_cas_dd
.global rv64_lock_get_b
.global rv64_lock_get_d
.global rv64_lock_get_dd
.global rv64_crc
.global rv64_crc_autocrc
.global rv64_crc_zbc
.global rv64_crc_autocrc_zbc

rv64_lock_xchg_dd:
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

rv64_lock_storeifref2:
    // address is a0, value is a1, a1 store to a0 only if [a0] is a2. return old [a0] value
    fence   rw, rw
1:    
    lr.d    a3, (a0)
    bne     a2, a3, 2f
    sc.d    a4, a1, (a0)
    bnez    a4, 1b
2:
    mv      a0, a3
    ret

rv64_lock_decifnot0b:
    fence   rw, rw
    andi    a3, a0, 3
    andi    a0, a0, ~3
    slli    a3, a3, 1   //TODO: make a jump table less hacky!
    la      a4, table
    add     a4, a4, a3
    jr      a4, 10
table:
    j       1f
    j       rv64_lock_decifnot0b_1  // all those are compressed
    j       rv64_lock_decifnot0b_2
    j       rv64_lock_decifnot0b_3
1:
    lr.w    a1, (a0)
    andi    a3, a1, 0xff
    beqz    a3, 2f
    addi    a3, a3, -1
    andi    a1, a1, ~0xff
    or      a1, a1, a3
    sc.w    a2, a1, (a0)
    bnez    a2, 1b
2:
    ret
rv64_lock_decifnot0b_1:
    li      a4, 0xffff00ff
1:
    lr.w    a1, (a0)
    srli    a3, a1, 8
    andi    a3, a3, 0xff
    beqz    a3, 2f
    addi    a3, a3, -1
    and     a1, a1, a4
    slli    a3, a3, 8
    or      a1, a1, a3
    sc.w    a2, a1, (a0)
    bnez    a2, 1b
2:
    ret
rv64_lock_decifnot0b_2:
    li      a4, 0xff00ffff
1:
    lr.w    a1, (a0)
    srli    a3, a1, 16
    andi    a3, a3, 0xff
    beqz    a3, 2f
    addi    a3, a3, -1
    and     a1, a1, a4
    slli    a3, a3, 16
    or      a1, a1, a3
    sc.w    a2, a1, (a0)
    bnez    a2, 1b
2:
    ret
rv64_lock_decifnot0b_3:
    li      a4, 0x00ffffff
1:
    lr.w    a1, (a0)
    srli    a3, a1, 24
    andi    a3, a3, 0xff
    beqz    a3, 2f
    addi    a3, a3, -1
    and     a1, a1, a4
    slli    a3, a3, 24
    or      a1, a1, a3
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

rv64_lock_inc:
    fence   rw, rw
1:
    lr.w    a1, (a0)
    addi    a1, a1, 1
    sc.w    a2, a1, (a0)
    bnez    a2, 1b
    mv      a0, a1
    ret

rv64_lock_store:
    sw      a1, 0(a0)
    fence   rw, rw
    ret

rv64_lock_store_dd:
    sd      a1, 0(a0)
    fence   rw, rw
    ret

rv64_lock_cas_d:
    fence   rw, rw
    lr.w    t0, (a0)
    bne     t0, a1, 1f
    sc.w    a0, a2, (a0)
    fence   rw, rw
    ret
1:
    li      a0, 1
    ret

rv64_lock_cas_dd:
    fence   rw, rw
    lr.d    t0, (a0)
    bne     t0, a1, 1f
    sc.d    a0, a2, (a0)
    fence   rw, rw
    ret
1:
    li      a0, 1
    ret

rv64_lock_get_b:
    fence   rw, rw
    lbu     a0, 0(a0)
    ret

rv64_lock_get_d:
    fence   rw, rw
    lw      a0, 0(a0)
    ret

rv64_lock_get_dd:
    fence   rw, rw
    ld      a0, 0(a0)
    ret

rv64_crc:
    // a0 = address, a1 = length, returns uint32_t hash in a0
    li      a2, 0xcbf29ce484222325
    li      a7, 0x100000001b3
    li      a3, 8
1:
    bltu    a1, a3, 2f
    ld      a4, 0(a0)
    xor     a2, a2, a4
    mul     a2, a2, a7
    addi    a0, a0, 8
    addi    a1, a1, -8
    bnez    a1, 1b
2:
    beqz    a1, 3f
    lbu     a4, 0(a0)
    xor     a2, a2, a4
    mul     a2, a2, a7
    addi    a0, a0, 1
    addi    a1, a1, -1
    bnez    a1, 2b
3:
    mv      a0, a2
    addiw   a0, a0, 0
    ret

rv64_crc_autocrc:
    // t3 = address, t4 = length, t2 = hash, t6 is preserved
    li      t2, 0xcbf29ce484222325
    li      a7, 0x100000001b3
    li      t5, 8
1:
    bltu    t4, t5, 2f
    ld      t1, 0(t3)
    xor     t2, t2, t1
    mul     t2, t2, a7
    addi    t3, t3, 8
    addi    t4, t4, -8
    bnez    t4, 1b
2:
    beqz    t4, 3f
    lbu     t1, 0(t3)
    xor     t2, t2, t1
    mul     t2, t2, a7
    addi    t3, t3, 1
    addi    t4, t4, -1
    bnez    t4, 2b
3:
    slli    t2, t2, 32
    srli    t2, t2, 32
    ret

    .option arch, +zbc
rv64_crc_zbc:
    // a0 = address, a1 = length, returns uint32_t CRC32C in a0
    li      a2, 0
    li      a3, 8
1:
    bltu    a1, a3, 2f
    slli    a2, a2, 32
    srli    a2, a2, 32
    ld      a4, 0(a0)
    xor     a4, a4, a2
    li      a7, 0x4869EC38DEA713F1
    clmul   a4, a4, a7
    li      a7, 0x105EC76F1
    clmulh  a4, a4, a7
    mv      a2, a4
    addi    a0, a0, 8
    addi    a1, a1, -8
    bnez    a1, 1b
2:
    beqz    a1, 3f
    slli    a2, a2, 32
    srli    a2, a2, 32
    lbu     a4, 0(a0)
    xor     a4, a4, a2
    slli    a4, a4, 56
    li      a7, 0x4869EC38DEA713F1
    clmul   a4, a4, a7
    li      a7, 0x105EC76F1
    clmulh  a4, a4, a7
    srli    a3, a2, 8
    xor     a2, a4, a3
    addi    a0, a0, 1
    addi    a1, a1, -1
    bnez    a1, 2b
3:
    mv      a0, a2
    addiw   a0, a0, 0
    ret

rv64_crc_autocrc_zbc:
    // t3 = address, t4 = length, t2 = CRC32C, t6 is preserved
    li      t2, 0
    li      t5, 8
1:
    bltu    t4, t5, 2f
    slli    t2, t2, 32
    srli    t2, t2, 32
    ld      t1, 0(t3)
    xor     t1, t1, t2
    li      a7, 0x4869EC38DEA713F1
    clmul   t1, t1, a7
    li      a7, 0x105EC76F1
    clmulh  t1, t1, a7
    mv      t2, t1
    addi    t3, t3, 8
    addi    t4, t4, -8
    bnez    t4, 1b
2:
    beqz    t4, 3f
    slli    t2, t2, 32
    srli    t2, t2, 32
    lbu     t1, 0(t3)
    xor     t1, t1, t2
    slli    t1, t1, 56
    li      a7, 0x4869EC38DEA713F1
    clmul   t1, t1, a7
    li      a7, 0x105EC76F1
    clmulh  t1, t1, a7
    srli    t5, t2, 8
    xor     t2, t1, t5
    addi    t3, t3, 1
    addi    t4, t4, -1
    bnez    t4, 2b
3:
    slli    t2, t2, 32
    srli    t2, t2, 32
    ret
