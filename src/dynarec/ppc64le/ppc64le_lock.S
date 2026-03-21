// PPC64LE lock helper functions
// Atomic read/write operations using Load-and-Reserve / Store-Conditional
// PPC64LE uses: lbarx/stbcx. (byte), lharx/sthcx. (halfword),
//               lwarx/stwcx. (word), ldarx/stdcx. (doubleword)
// All with lwsync/sync barriers for memory ordering.

.text
.align 4

.global ppc64le_lock_read_b
.global ppc64le_lock_write_b
.global ppc64le_lock_read_h
.global ppc64le_lock_write_h
.global ppc64le_lock_read_d
.global ppc64le_lock_write_d
.global ppc64le_lock_read_dd
.global ppc64le_lock_write_dd
.global ppc64le_lock_xchg_dd
.global ppc64le_lock_xchg_d
.global ppc64le_lock_xchg_h
.global ppc64le_lock_xchg_b
.global ppc64le_lock_storeifnull
.global ppc64le_lock_storeifnull_d
.global ppc64le_lock_storeifref
.global ppc64le_lock_storeifref2
.global ppc64le_lock_storeifref_d
.global ppc64le_lock_storeifref2_d
.global ppc64le_lock_decifnot0b
.global ppc64le_lock_storeb
.global ppc64le_lock_incif0
.global ppc64le_lock_decifnot0
.global ppc64le_lock_store
.global ppc64le_lock_store_dd
.global ppc64le_lock_get_b
.global ppc64le_lock_get_d
.global ppc64le_lock_get_dd
.global ppc64le_fast_hash

// ============================================================================
// Byte operations
// ============================================================================
.type ppc64le_lock_read_b, @function
ppc64le_lock_read_b:
    // address is r3, return is r3
    lwsync
    lbarx   %r3, 0, %r3
    blr
.size ppc64le_lock_read_b, .-ppc64le_lock_read_b

.type ppc64le_lock_write_b, @function
ppc64le_lock_write_b:
    // address is r3, value is r4, return is r3 (0=success, 1=fail)
    mr      %r5, %r3
    stbcx.  %r4, 0, %r5
    mfcr    %r3
    rlwinm  %r3, %r3, 3, 31, 31     // extract CR0[EQ]: 1 if success
    xori    %r3, %r3, 1              // invert: 0 = success, 1 = fail
    lwsync
    blr
.size ppc64le_lock_write_b, .-ppc64le_lock_write_b

// ============================================================================
// Halfword operations
// ============================================================================
.type ppc64le_lock_read_h, @function
ppc64le_lock_read_h:
    // address is r3, return is r3
    lwsync
    lharx   %r3, 0, %r3
    blr
.size ppc64le_lock_read_h, .-ppc64le_lock_read_h

.type ppc64le_lock_write_h, @function
ppc64le_lock_write_h:
    // address is r3, value is r4, return is r3
    mr      %r5, %r3
    sthcx.  %r4, 0, %r5
    mfcr    %r3
    rlwinm  %r3, %r3, 3, 31, 31
    xori    %r3, %r3, 1
    lwsync
    blr
.size ppc64le_lock_write_h, .-ppc64le_lock_write_h

// ============================================================================
// Word (32-bit) operations
// ============================================================================
.type ppc64le_lock_read_d, @function
ppc64le_lock_read_d:
    // address is r3, return is r3
    lwsync
    lwarx   %r3, 0, %r3
    blr
.size ppc64le_lock_read_d, .-ppc64le_lock_read_d

.type ppc64le_lock_write_d, @function
ppc64le_lock_write_d:
    // address is r3, value is r4, return is r3
    mr      %r5, %r3
    stwcx.  %r4, 0, %r5
    mfcr    %r3
    rlwinm  %r3, %r3, 3, 31, 31
    xori    %r3, %r3, 1
    lwsync
    blr
.size ppc64le_lock_write_d, .-ppc64le_lock_write_d

// ============================================================================
// Doubleword (64-bit) operations
// ============================================================================
.type ppc64le_lock_read_dd, @function
ppc64le_lock_read_dd:
    // address is r3, return is r3
    lwsync
    ldarx   %r3, 0, %r3
    blr
.size ppc64le_lock_read_dd, .-ppc64le_lock_read_dd

.type ppc64le_lock_write_dd, @function
ppc64le_lock_write_dd:
    // address is r3, value is r4, return is r3
    mr      %r5, %r3
    stdcx.  %r4, 0, %r5
    mfcr    %r3
    rlwinm  %r3, %r3, 3, 31, 31
    xori    %r3, %r3, 1
    lwsync
    blr
.size ppc64le_lock_write_dd, .-ppc64le_lock_write_dd

// ============================================================================
// Exchange operations (atomic swap)
// ============================================================================
.type ppc64le_lock_xchg_dd, @function
ppc64le_lock_xchg_dd:
    // address is r3, value is r4, return old value in r3
    lwsync
1:
    ldarx   %r5, 0, %r3
    stdcx.  %r4, 0, %r3
    bne     1b
    lwsync
    mr      %r3, %r5
    blr
.size ppc64le_lock_xchg_dd, .-ppc64le_lock_xchg_dd

.type ppc64le_lock_xchg_d, @function
ppc64le_lock_xchg_d:
    // address is r3, value is r4, return old value in r3
    lwsync
1:
    lwarx   %r5, 0, %r3
    stwcx.  %r4, 0, %r3
    bne     1b
    lwsync
    mr      %r3, %r5
    blr
.size ppc64le_lock_xchg_d, .-ppc64le_lock_xchg_d

.type ppc64le_lock_xchg_h, @function
ppc64le_lock_xchg_h:
    // address is r3, value is r4, return old value in r3
    lwsync
1:
    lharx   %r5, 0, %r3
    sthcx.  %r4, 0, %r3
    bne     1b
    lwsync
    mr      %r3, %r5
    blr
.size ppc64le_lock_xchg_h, .-ppc64le_lock_xchg_h

.type ppc64le_lock_xchg_b, @function
ppc64le_lock_xchg_b:
    // address is r3, value is r4, return old value in r3
    lwsync
1:
    lbarx   %r5, 0, %r3
    stbcx.  %r4, 0, %r3
    bne     1b
    lwsync
    mr      %r3, %r5
    blr
.size ppc64le_lock_xchg_b, .-ppc64le_lock_xchg_b

// ============================================================================
// Conditional store operations
// ============================================================================
.type ppc64le_lock_storeifnull, @function
ppc64le_lock_storeifnull:
    // address is r3, value is r4
    // store r4 to [r3] only if [r3] is 0. return old [r3] value
    lwsync
1:
    ldarx   %r5, 0, %r3
    cmpdi   %r5, 0
    bne     2f
    stdcx.  %r4, 0, %r3
    bne     1b
2:
    lwsync
    mr      %r3, %r5
    blr
.size ppc64le_lock_storeifnull, .-ppc64le_lock_storeifnull

.type ppc64le_lock_storeifnull_d, @function
ppc64le_lock_storeifnull_d:
    // address is r3, value is r4
    // store r4 (word) to [r3] only if [r3] is 0. return old [r3] value
    lwsync
1:
    lwarx   %r5, 0, %r3
    cmpwi   %r5, 0
    bne     2f
    stwcx.  %r4, 0, %r3
    bne     1b
2:
    lwsync
    mr      %r3, %r5
    blr
.size ppc64le_lock_storeifnull_d, .-ppc64le_lock_storeifnull_d

.type ppc64le_lock_storeifref, @function
ppc64le_lock_storeifref:
    // address is r3, value is r4, ref is r5
    // store r4 to [r3] only if [r3] == r5. return new [r3] value (r4 or old)
    lwsync
1:
    ldarx   %r6, 0, %r3
    cmpd    %r5, %r6
    bne     2f
    stdcx.  %r4, 0, %r3
    bne     1b
    mr      %r3, %r4
    blr
2:
    mr      %r3, %r6
    blr
.size ppc64le_lock_storeifref, .-ppc64le_lock_storeifref

.type ppc64le_lock_storeifref2, @function
ppc64le_lock_storeifref2:
    // address is r3, value is r4, ref is r5
    // store r4 to [r3] only if [r3] == r5. return old [r3] value
    lwsync
1:
    ldarx   %r6, 0, %r3
    cmpd    %r5, %r6
    bne     2f
    stdcx.  %r4, 0, %r3
    bne     1b
2:
    mr      %r3, %r6
    blr
.size ppc64le_lock_storeifref2, .-ppc64le_lock_storeifref2

.type ppc64le_lock_storeifref_d, @function
ppc64le_lock_storeifref_d:
    // address is r3, value is r4 (word), ref is r5 (word)
    // store r4 to [r3] only if [r3] == r5. return new [r3] value
    lwsync
1:
    lwarx   %r6, 0, %r3
    cmpw    %r5, %r6
    bne     2f
    stwcx.  %r4, 0, %r3
    bne     1b
    mr      %r3, %r4
    blr
2:
    mr      %r3, %r6
    blr
.size ppc64le_lock_storeifref_d, .-ppc64le_lock_storeifref_d

.type ppc64le_lock_storeifref2_d, @function
ppc64le_lock_storeifref2_d:
    // address is r3, value is r4 (word), ref is r5 (word)
    // store r4 to [r3] only if [r3] == r5. return old [r3] value
    lwsync
1:
    lwarx   %r6, 0, %r3
    cmpw    %r5, %r6
    bne     2f
    stwcx.  %r4, 0, %r3
    bne     1b
2:
    mr      %r3, %r6
    blr
.size ppc64le_lock_storeifref2_d, .-ppc64le_lock_storeifref2_d

// ============================================================================
// Misc lock operations
// ============================================================================
.type ppc64le_lock_decifnot0b, @function
ppc64le_lock_decifnot0b:
    // address is r3, decrement byte at [r3] if not 0
    lwsync
1:
    lbarx   %r4, 0, %r3
    cmpwi   %r4, 0
    beq     2f
    addi    %r4, %r4, -1
    stbcx.  %r4, 0, %r3
    bne     1b
2:
    blr
.size ppc64le_lock_decifnot0b, .-ppc64le_lock_decifnot0b

.type ppc64le_lock_storeb, @function
ppc64le_lock_storeb:
    // address is r3, value is r4
    stb     %r4, 0(%r3)
    lwsync
    blr
.size ppc64le_lock_storeb, .-ppc64le_lock_storeb

.type ppc64le_lock_decifnot0, @function
ppc64le_lock_decifnot0:
    // address is r3, return old value in r3
    lwsync
1:
    lwarx   %r4, 0, %r3
    cmpwi   %r4, 0
    beq     2f
    addi    %r5, %r4, -1
    stwcx.  %r5, 0, %r3
    bne     1b
2:
    mr      %r3, %r4
    blr
.size ppc64le_lock_decifnot0, .-ppc64le_lock_decifnot0

.type ppc64le_lock_incif0, @function
ppc64le_lock_incif0:
    // address is r3, increment word at [r3] if 0. return old value in r3
    lwsync
1:
    lwarx   %r4, 0, %r3
    cmpwi   %r4, 0
    bne     2f
    addi    %r5, %r4, 1
    stwcx.  %r5, 0, %r3
    bne     1b
2:
    mr      %r3, %r4
    blr
.size ppc64le_lock_incif0, .-ppc64le_lock_incif0

.type ppc64le_lock_store, @function
ppc64le_lock_store:
    // address is r3, value is r4 (word)
    stw     %r4, 0(%r3)
    lwsync
    blr
.size ppc64le_lock_store, .-ppc64le_lock_store

.type ppc64le_lock_store_dd, @function
ppc64le_lock_store_dd:
    // address is r3, value is r4 (doubleword)
    std     %r4, 0(%r3)
    lwsync
    blr
.size ppc64le_lock_store_dd, .-ppc64le_lock_store_dd

.type ppc64le_lock_get_b, @function
ppc64le_lock_get_b:
    // address is r3, return byte in r3
    lbarx   %r3, 0, %r3
    blr
.size ppc64le_lock_get_b, .-ppc64le_lock_get_b

.type ppc64le_lock_get_d, @function
ppc64le_lock_get_d:
    // address is r3, return word in r3
    lwarx   %r3, 0, %r3
    blr
.size ppc64le_lock_get_d, .-ppc64le_lock_get_d

.type ppc64le_lock_get_dd, @function
ppc64le_lock_get_dd:
    // address is r3, return doubleword in r3
    ldarx   %r3, 0, %r3
    blr
.size ppc64le_lock_get_dd, .-ppc64le_lock_get_dd

// ============================================================================
// Fast hash for block change detection on PPC64LE.
//
// Uses XOR-rotate-accumulate with 4 independent accumulators for ILP.
// Processes 32 bytes per iteration in the main loop, with 8-byte tail.
// No multiplies — only ld/xor/rotldi (all 1-cycle on POWER9).
//
// POWER9 lacks hardware CRC32 instructions (unlike ARM64's crc32x or
// LA64's crc.w.d.w), so we use this multiply-free approach instead.
// The hash is used purely for equality-based change detection
// (not hash table distribution), so cryptographic quality is not needed.
//
// Note: vpmsumd (carryless multiply) was also tried with 2 vector
// accumulators and optimized finalization, but measured ~17% of CPU
// vs ~16.4% for this XOR-rotate approach.  The ~7-cycle vpmsumd
// latency and vector-to-GPR extraction cost outweigh its better
// mixing on the small block sizes typical of dynarec code blocks.
//
// Registers:
//   r3  = address (input), then return value
//   r4  = length in bytes (input)
//   r5  = data pointer
//   r6  = accumulator h0
//   r7  = accumulator h1
//   r8  = accumulator h2
//   r9  = accumulator h3
//   r10 = temp (loaded data)
//   r11 = temp (loaded data)
//   r12 = temp (loaded data)
//   r0  = temp (loaded data)
// ============================================================================
.type ppc64le_fast_hash, @function
ppc64le_fast_hash:
    mr      5, 3                    // r5 = data pointer
    li      3, 0                    // return 0 for zero length
    cmpwi   4, 0
    beqlr

    // Initialize 4 accumulators with different seeds incorporating length.
    // Using odd constants ensures different initial states.
    // r6 = h0, r7 = h1, r8 = h2, r9 = h3
    lis     6, 0x9E37               // r6 = 0x9E3779B97F4A7C15 (golden ratio)
    ori     6, 6, 0x79B9
    sldi    6, 6, 32
    oris    6, 6, 0x7F4A
    ori     6, 6, 0x7C15
    xor     6, 6, 4                 // mix in length

    lis     7, 0x517C               // r7 = 0x517CC1B727220A95
    ori     7, 7, 0xC1B7
    sldi    7, 7, 32
    oris    7, 7, 0x2722
    ori     7, 7, 0x0A95
    xor     7, 7, 4

    lis     8, 0x6C62               // r8 = 0x6C62272E07BB0142
    ori     8, 8, 0x272E
    sldi    8, 8, 32
    oris    8, 8, 0x07BB
    ori     8, 8, 0x0142
    xor     8, 8, 4

    lis     9, 0x2E7D               // r9 = 0x2E7D2C03A4507D35
    ori     9, 9, 0x2C03
    sldi    9, 9, 32
    oris    9, 9, 0xA450
    ori     9, 9, 0x7D35
    xor     9, 9, 4

    // Main loop: process 32 bytes (4 x 8 bytes) per iteration
    cmplwi  4, 32
    blt     .Lhash_tail8

.Lhash_32byte_loop:
    // Load 4 doublewords — all loads are independent (good ILP)
    ld      10, 0(5)
    ld      11, 8(5)
    ld      12, 16(5)
    ld      0, 24(5)

    // XOR data into accumulators
    xor     6, 6, 10
    xor     7, 7, 11
    xor     8, 8, 12
    xor     9, 9, 0

    // Rotate accumulators by different prime amounts for avalanche
    // All rotldi are 1-cycle, independent of each other
    rotldi  6, 6, 31
    rotldi  7, 7, 29
    rotldi  8, 8, 23
    rotldi  9, 9, 19

    // Cross-mix: each accumulator gets bits from another
    // This provides avalanche without multiplies
    xor     6, 6, 7
    xor     8, 8, 9
    rotldi  7, 7, 7
    rotldi  9, 9, 11
    xor     7, 7, 6
    xor     9, 9, 8

    addi    5, 5, 32
    addi    4, 4, -32
    cmplwi  4, 32
    bge     .Lhash_32byte_loop

.Lhash_tail8:
    // Process remaining 8-byte chunks
    cmplwi  4, 8
    blt     .Lhash_tail_partial

.Lhash_8byte_loop:
    ld      10, 0(5)
    xor     6, 6, 10
    rotldi  6, 6, 31
    xor     6, 6, 7
    addi    5, 5, 8
    addi    4, 4, -8
    cmplwi  4, 8
    bge     .Lhash_8byte_loop

.Lhash_tail_partial:
    // Process remaining 1-7 bytes.
    // Instead of a byte-at-a-time loop (which has serial dependencies),
    // load a partial word/halfword/byte and combine in fewer steps.
    cmpwi   4, 0
    beq     .Lhash_finalize

    // Handle 4+ remaining bytes: load a word
    cmplwi  4, 4
    blt     .Lhash_tail_lt4
    lwz     10, 0(5)                // load 4 bytes
    xor     6, 6, 10
    rotldi  6, 6, 13
    addi    5, 5, 4
    addi    4, 4, -4
    cmpwi   4, 0
    beq     .Lhash_finalize

.Lhash_tail_lt4:
    // Handle 2+ remaining bytes: load a halfword
    cmplwi  4, 2
    blt     .Lhash_tail_lt2
    lhz     10, 0(5)                // load 2 bytes
    xor     6, 6, 10
    rotldi  6, 6, 7
    addi    5, 5, 2
    addi    4, 4, -2
    cmpwi   4, 0
    beq     .Lhash_finalize

.Lhash_tail_lt2:
    // Handle last remaining byte
    lbz     10, 0(5)
    xor     6, 6, 10
    rotldi  6, 6, 5

.Lhash_finalize:
    // Fold 4 accumulators into one
    xor     6, 6, 7
    xor     8, 8, 9
    xor     6, 6, 8

    // Final 64->32 bit mix (shift-xor, no multiply)
    srdi    10, 6, 33
    xor     6, 6, 10
    srdi    10, 6, 17
    xor     6, 6, 10
    rotldi  6, 6, 13
    srdi    10, 6, 29
    xor     3, 6, 10               // result in r3

    blr
.size ppc64le_fast_hash, .-ppc64le_fast_hash
