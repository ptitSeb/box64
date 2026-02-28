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
.global ppc64le_crc

// ============================================================================
// Byte operations
// ============================================================================
.type ppc64le_lock_read_b, @function
ppc64le_lock_read_b:
    // address is r3, return is r3
    lwsync
    lbarx   3, 0, 3
    blr
.size ppc64le_lock_read_b, .-ppc64le_lock_read_b

.type ppc64le_lock_write_b, @function
ppc64le_lock_write_b:
    // address is r3, value is r4, return is r3 (0=success, 1=fail)
    mr      5, 3
    stbcx.  4, 0, 5
    mfcr    3
    rlwinm  3, 3, 3, 31, 31     // extract CR0[EQ]: 1 if success
    xori    3, 3, 1              // invert: 0 = success, 1 = fail
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
    lharx   3, 0, 3
    blr
.size ppc64le_lock_read_h, .-ppc64le_lock_read_h

.type ppc64le_lock_write_h, @function
ppc64le_lock_write_h:
    // address is r3, value is r4, return is r3
    mr      5, 3
    sthcx.  4, 0, 5
    mfcr    3
    rlwinm  3, 3, 3, 31, 31
    xori    3, 3, 1
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
    lwarx   3, 0, 3
    blr
.size ppc64le_lock_read_d, .-ppc64le_lock_read_d

.type ppc64le_lock_write_d, @function
ppc64le_lock_write_d:
    // address is r3, value is r4, return is r3
    mr      5, 3
    stwcx.  4, 0, 5
    mfcr    3
    rlwinm  3, 3, 3, 31, 31
    xori    3, 3, 1
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
    ldarx   3, 0, 3
    blr
.size ppc64le_lock_read_dd, .-ppc64le_lock_read_dd

.type ppc64le_lock_write_dd, @function
ppc64le_lock_write_dd:
    // address is r3, value is r4, return is r3
    mr      5, 3
    stdcx.  4, 0, 5
    mfcr    3
    rlwinm  3, 3, 3, 31, 31
    xori    3, 3, 1
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
    ldarx   5, 0, 3
    stdcx.  4, 0, 3
    bne     1b
    lwsync
    mr      3, 5
    blr
.size ppc64le_lock_xchg_dd, .-ppc64le_lock_xchg_dd

.type ppc64le_lock_xchg_d, @function
ppc64le_lock_xchg_d:
    // address is r3, value is r4, return old value in r3
    lwsync
1:
    lwarx   5, 0, 3
    stwcx.  4, 0, 3
    bne     1b
    lwsync
    mr      3, 5
    blr
.size ppc64le_lock_xchg_d, .-ppc64le_lock_xchg_d

.type ppc64le_lock_xchg_h, @function
ppc64le_lock_xchg_h:
    // address is r3, value is r4, return old value in r3
    lwsync
1:
    lharx   5, 0, 3
    sthcx.  4, 0, 3
    bne     1b
    lwsync
    mr      3, 5
    blr
.size ppc64le_lock_xchg_h, .-ppc64le_lock_xchg_h

.type ppc64le_lock_xchg_b, @function
ppc64le_lock_xchg_b:
    // address is r3, value is r4, return old value in r3
    lwsync
1:
    lbarx   5, 0, 3
    stbcx.  4, 0, 3
    bne     1b
    lwsync
    mr      3, 5
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
    ldarx   5, 0, 3
    cmpdi   5, 0
    bne     2f
    stdcx.  4, 0, 3
    bne     1b
2:
    lwsync
    mr      3, 5
    blr
.size ppc64le_lock_storeifnull, .-ppc64le_lock_storeifnull

.type ppc64le_lock_storeifnull_d, @function
ppc64le_lock_storeifnull_d:
    // address is r3, value is r4
    // store r4 (word) to [r3] only if [r3] is 0. return old [r3] value
    lwsync
1:
    lwarx   5, 0, 3
    cmpwi   5, 0
    bne     2f
    stwcx.  4, 0, 3
    bne     1b
2:
    lwsync
    mr      3, 5
    blr
.size ppc64le_lock_storeifnull_d, .-ppc64le_lock_storeifnull_d

.type ppc64le_lock_storeifref, @function
ppc64le_lock_storeifref:
    // address is r3, value is r4, ref is r5
    // store r4 to [r3] only if [r3] == r5. return new [r3] value (r4 or old)
    lwsync
1:
    ldarx   6, 0, 3
    cmpd    5, 6
    bne     2f
    stdcx.  4, 0, 3
    bne     1b
    mr      3, 4
    blr
2:
    mr      3, 6
    blr
.size ppc64le_lock_storeifref, .-ppc64le_lock_storeifref

.type ppc64le_lock_storeifref2, @function
ppc64le_lock_storeifref2:
    // address is r3, value is r4, ref is r5
    // store r4 to [r3] only if [r3] == r5. return old [r3] value
    lwsync
1:
    ldarx   6, 0, 3
    cmpd    5, 6
    bne     2f
    stdcx.  4, 0, 3
    bne     1b
2:
    mr      3, 6
    blr
.size ppc64le_lock_storeifref2, .-ppc64le_lock_storeifref2

.type ppc64le_lock_storeifref_d, @function
ppc64le_lock_storeifref_d:
    // address is r3, value is r4 (word), ref is r5 (word)
    // store r4 to [r3] only if [r3] == r5. return new [r3] value
    lwsync
1:
    lwarx   6, 0, 3
    cmpw    5, 6
    bne     2f
    stwcx.  4, 0, 3
    bne     1b
    mr      3, 4
    blr
2:
    mr      3, 6
    blr
.size ppc64le_lock_storeifref_d, .-ppc64le_lock_storeifref_d

.type ppc64le_lock_storeifref2_d, @function
ppc64le_lock_storeifref2_d:
    // address is r3, value is r4 (word), ref is r5 (word)
    // store r4 to [r3] only if [r3] == r5. return old [r3] value
    lwsync
1:
    lwarx   6, 0, 3
    cmpw    5, 6
    bne     2f
    stwcx.  4, 0, 3
    bne     1b
2:
    mr      3, 6
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
    lbarx   4, 0, 3
    cmpwi   4, 0
    beq     2f
    addi    4, 4, -1
    stbcx.  4, 0, 3
    bne     1b
2:
    blr
.size ppc64le_lock_decifnot0b, .-ppc64le_lock_decifnot0b

.type ppc64le_lock_storeb, @function
ppc64le_lock_storeb:
    // address is r3, value is r4
    stb     4, 0(3)
    lwsync
    blr
.size ppc64le_lock_storeb, .-ppc64le_lock_storeb

.type ppc64le_lock_decifnot0, @function
ppc64le_lock_decifnot0:
    // address is r3, return old value in r3
    lwsync
1:
    lwarx   4, 0, 3
    cmpwi   4, 0
    beq     2f
    addi    5, 4, -1
    stwcx.  5, 0, 3
    bne     1b
2:
    mr      3, 4
    blr
.size ppc64le_lock_decifnot0, .-ppc64le_lock_decifnot0

.type ppc64le_lock_incif0, @function
ppc64le_lock_incif0:
    // address is r3, increment word at [r3] if 0. return old value in r3
    lwsync
1:
    lwarx   4, 0, 3
    cmpwi   4, 0
    bne     2f
    addi    5, 4, 1
    stwcx.  5, 0, 3
    bne     1b
2:
    mr      3, 4
    blr
.size ppc64le_lock_incif0, .-ppc64le_lock_incif0

.type ppc64le_lock_store, @function
ppc64le_lock_store:
    // address is r3, value is r4 (word)
    stw     4, 0(3)
    lwsync
    blr
.size ppc64le_lock_store, .-ppc64le_lock_store

.type ppc64le_lock_store_dd, @function
ppc64le_lock_store_dd:
    // address is r3, value is r4 (doubleword)
    std     4, 0(3)
    lwsync
    blr
.size ppc64le_lock_store_dd, .-ppc64le_lock_store_dd

.type ppc64le_lock_get_b, @function
ppc64le_lock_get_b:
    // address is r3, return byte in r3
    lbarx   3, 0, 3
    blr
.size ppc64le_lock_get_b, .-ppc64le_lock_get_b

.type ppc64le_lock_get_d, @function
ppc64le_lock_get_d:
    // address is r3, return word in r3
    lwarx   3, 0, 3
    blr
.size ppc64le_lock_get_d, .-ppc64le_lock_get_d

.type ppc64le_lock_get_dd, @function
ppc64le_lock_get_dd:
    // address is r3, return doubleword in r3
    ldarx   3, 0, 3
    blr
.size ppc64le_lock_get_dd, .-ppc64le_lock_get_dd

// ============================================================================
// CRC32 — slicing-by-4 software implementation
// PPC64LE has no hardware CRC32 instruction, so we use 4 lookup tables (4KB).
// Uses polynomial 0x1EDC6F41 (CRC-32C / Castagnoli), bit-reversed as 0x82F63B78.
// Processes 4 bytes per iteration with 4 independent table lookups (good ILP),
// with byte-at-a-time tail using tab0.
// ============================================================================
.type ppc64le_crc, @function
ppc64le_crc:
    // r3 = address, r4 = length (bytes)
    // return CRC in r3
    // Registers:
    //   r3  = crc (return value)
    //   r4  = remaining length
    //   r5  = address pointer
    //   r6  = tab0 base
    //   r7  = tab1 base (tab0 + 1024)
    //   r8  = tab2 base (tab0 + 2048)
    //   r9  = tab3 base (tab0 + 3072)
    //   r10 = temp (loaded word)
    //   r11 = temp (table lookup result)
    //   r12 = temp (table lookup result)
    //   r0  = temp (table lookup result)
    mr      5, 3                // address in r5
    li      3, 0                // crc = 0
    cmpwi   4, 0
    beqlr                       // return 0 if length == 0

    // Load table base address (PC-relative via bcl/mflr trick)
    mflr    10                  // save LR in r10
    bcl     20, 31, .+4         // branch-and-link to next insn (sets LR = here+4)
.Lcrc_here:
    mflr    6                   // r6 = address of .Lcrc_here
    mtlr    10                  // restore LR
    addis   6, 6, (.Lcrc32c_tab0 - .Lcrc_here)@ha
    addi    6, 6, (.Lcrc32c_tab0 - .Lcrc_here)@l

    // Set up table base pointers: tab1 = tab0+1024, tab2 = tab0+2048, tab3 = tab0+3072
    addi    7, 6, 1024          // r7 = tab1
    addi    8, 6, 2048          // r8 = tab2
    addi    9, 6, 3072          // r9 = tab3

    // Main loop: slicing-by-4, process 4 bytes per iteration
    cmplwi  4, 4
    blt     .Lcrc_tail

.Lcrc_4byte_loop:
    // Load 4 bytes as a little-endian word and XOR with crc
    lwz     10, 0(5)            // r10 = *(uint32_t*)addr (LE on LE machine)
    xor     10, 3, 10           // r10 = crc ^ word

    // 4 independent table lookups — good ILP on superscalar POWER9
    // tab3[(r10 >> 0) & 0xFF]
    clrldi  11, 10, 56          // r11 = r10 & 0xFF (byte 0)
    sldi    11, 11, 2           // r11 *= 4
    lwzx    11, 9, 11           // r11 = tab3[byte0]

    // tab2[(r10 >> 8) & 0xFF]
    srwi    12, 10, 8
    clrldi  12, 12, 56          // r12 = (r10 >> 8) & 0xFF (byte 1)
    sldi    12, 12, 2           // r12 *= 4
    lwzx    12, 8, 12           // r12 = tab2[byte1]

    // tab1[(r10 >> 16) & 0xFF]
    srwi    0, 10, 16
    clrldi  0, 0, 56            // r0 = (r10 >> 16) & 0xFF (byte 2)
    sldi    0, 0, 2             // r0 *= 4
    lwzx    0, 7, 0             // r0 = tab1[byte2]

    // tab0[(r10 >> 24) & 0xFF]
    srwi    10, 10, 24          // r10 = (r10 >> 24) & 0xFF (byte 3)
    sldi    10, 10, 2           // r10 *= 4
    lwzx    10, 6, 10           // r10 = tab0[byte3]

    // XOR all 4 results together
    xor     3, 11, 12           // crc = tab3[b0] ^ tab2[b1]
    xor     3, 3, 0             // crc ^= tab1[b2]
    xor     3, 3, 10            // crc ^= tab0[b3]

    addi    5, 5, 4
    addi    4, 4, -4
    cmplwi  4, 4
    bge     .Lcrc_4byte_loop

.Lcrc_tail:
    // Process remaining 0-3 bytes one at a time using tab0
    cmpwi   4, 0
    beqlr

.Lcrc_tail_loop:
    lbz     10, 0(5)
    xor     11, 3, 10
    clrldi  11, 11, 56
    sldi    11, 11, 2
    lwzx    11, 6, 11
    srwi    3, 3, 8
    xor     3, 3, 11
    addi    5, 5, 1
    addi    4, 4, -1
    cmpwi   4, 0
    bne     .Lcrc_tail_loop

    blr
.size ppc64le_crc, .-ppc64le_crc

// CRC32C slicing-by-4 lookup tables (polynomial 0x82F63B78)
// 4 tables x 256 entries x 4 bytes = 4KB total
.section .rodata
.align 4
.Lcrc32c_tab0:
    .long 0x00000000, 0xF26B8303, 0xE13B70F7, 0x1350F3F4
    .long 0xC79A971F, 0x35F1141C, 0x26A1E7E8, 0xD4CA64EB
    .long 0x8AD958CF, 0x78B2DBCC, 0x6BE22838, 0x9989AB3B
    .long 0x4D43CFD0, 0xBF284CD3, 0xAC78BF27, 0x5E133C24
    .long 0x105EC76F, 0xE235446C, 0xF165B798, 0x030E349B
    .long 0xD7C45070, 0x25AFD373, 0x36FF2087, 0xC494A384
    .long 0x9A879FA0, 0x68EC1CA3, 0x7BBCEF57, 0x89D76C54
    .long 0x5D1D08BF, 0xAF768BBC, 0xBC267848, 0x4E4DFB4B
    .long 0x20BD8EDE, 0xD2D60DDD, 0xC186FE29, 0x33ED7D2A
    .long 0xE72719C1, 0x154C9AC2, 0x061C6936, 0xF477EA35
    .long 0xAA64D611, 0x580F5512, 0x4B5FA6E6, 0xB93425E5
    .long 0x6DFE410E, 0x9F95C20D, 0x8CC531F9, 0x7EAEB2FA
    .long 0x30E349B1, 0xC288CAB2, 0xD1D83946, 0x23B3BA45
    .long 0xF779DEAE, 0x05125DAD, 0x1642AE59, 0xE4292D5A
    .long 0xBA3A117E, 0x4851927D, 0x5B016189, 0xA96AE28A
    .long 0x7DA08661, 0x8FCB0562, 0x9C9BF696, 0x6EF07595
    .long 0x417B1DBC, 0xB3109EBF, 0xA0406D4B, 0x522BEE48
    .long 0x86E18AA3, 0x748A09A0, 0x67DAFA54, 0x95B17957
    .long 0xCBA24573, 0x39C9C670, 0x2A993584, 0xD8F2B687
    .long 0x0C38D26C, 0xFE53516F, 0xED03A29B, 0x1F682198
    .long 0x5125DAD3, 0xA34E59D0, 0xB01EAA24, 0x42752927
    .long 0x96BF4DCC, 0x64D4CECF, 0x77843D3B, 0x85EFBE38
    .long 0xDBFC821C, 0x2997011F, 0x3AC7F2EB, 0xC8AC71E8
    .long 0x1C661503, 0xEE0D9600, 0xFD5D65F4, 0x0F36E6F7
    .long 0x61C69362, 0x93AD1061, 0x80FDE395, 0x72966096
    .long 0xA65C047D, 0x5437877E, 0x4767748A, 0xB50CF789
    .long 0xEB1FCBAD, 0x197448AE, 0x0A24BB5A, 0xF84F3859
    .long 0x2C855CB2, 0xDEEEDFB1, 0xCDBE2C45, 0x3FD5AF46
    .long 0x7198540D, 0x83F3D70E, 0x90A324FA, 0x62C8A7F9
    .long 0xB602C312, 0x44694011, 0x5739B3E5, 0xA55230E6
    .long 0xFB410CC2, 0x092A8FC1, 0x1A7A7C35, 0xE811FF36
    .long 0x3CDB9BDD, 0xCEB018DE, 0xDDE0EB2A, 0x2F8B6829
    .long 0x82F63B78, 0x709DB87B, 0x63CD4B8F, 0x91A6C88C
    .long 0x456CAC67, 0xB7072F64, 0xA457DC90, 0x563C5F93
    .long 0x082F63B7, 0xFA44E0B4, 0xE9141340, 0x1B7F9043
    .long 0xCFB5F4A8, 0x3DDE77AB, 0x2E8E845F, 0xDCE5075C
    .long 0x92A8FC17, 0x60C37F14, 0x73938CE0, 0x81F80FE3
    .long 0x55326B08, 0xA759E80B, 0xB4091BFF, 0x466298FC
    .long 0x1871A4D8, 0xEA1A27DB, 0xF94AD42F, 0x0B21572C
    .long 0xDFEB33C7, 0x2D80B0C4, 0x3ED04330, 0xCCBBC033
    .long 0xA24BB5A6, 0x502036A5, 0x4370C551, 0xB11B4652
    .long 0x65D122B9, 0x97BAA1BA, 0x84EA524E, 0x7681D14D
    .long 0x2892ED69, 0xDAF96E6A, 0xC9A99D9E, 0x3BC21E9D
    .long 0xEF087A76, 0x1D63F975, 0x0E330A81, 0xFC588982
    .long 0xB21572C9, 0x407EF1CA, 0x532E023E, 0xA145813D
    .long 0x758FE5D6, 0x87E466D5, 0x94B49521, 0x66DF1622
    .long 0x38CC2A06, 0xCAA7A905, 0xD9F75AF1, 0x2B9CD9F2
    .long 0xFF56BD19, 0x0D3D3E1A, 0x1E6DCDEE, 0xEC064EED
    .long 0xC38D26C4, 0x31E6A5C7, 0x22B65633, 0xD0DDD530
    .long 0x0417B1DB, 0xF67C32D8, 0xE52CC12C, 0x1747422F
    .long 0x49547E0B, 0xBB3FFD08, 0xA86F0EFC, 0x5A048DFF
    .long 0x8ECEE914, 0x7CA56A17, 0x6FF599E3, 0x9D9E1AE0
    .long 0xD3D3E1AB, 0x21B862A8, 0x32E8915C, 0xC083125F
    .long 0x144976B4, 0xE622F5B7, 0xF5720643, 0x07198540
    .long 0x590AB964, 0xAB613A67, 0xB831C993, 0x4A5A4A90
    .long 0x9E902E7B, 0x6CFBAD78, 0x7FAB5E8C, 0x8DC0DD8F
    .long 0xE330A81A, 0x115B2B19, 0x020BD8ED, 0xF0605BEE
    .long 0x24AA3F05, 0xD6C1BC06, 0xC5914FF2, 0x37FACCF1
    .long 0x69E9F0D5, 0x9B8273D6, 0x88D28022, 0x7AB90321
    .long 0xAE7367CA, 0x5C18E4C9, 0x4F48173D, 0xBD23943E
    .long 0xF36E6F75, 0x0105EC76, 0x12551F82, 0xE03E9C81
    .long 0x34F4F86A, 0xC69F7B69, 0xD5CF889D, 0x27A40B9E
    .long 0x79B737BA, 0x8BDCB4B9, 0x988C474D, 0x6AE7C44E
    .long 0xBE2DA0A5, 0x4C4623A6, 0x5F16D052, 0xAD7D5351

.Lcrc32c_tab1:
    .long 0x00000000, 0x13A29877, 0x274530EE, 0x34E7A899
    .long 0x4E8A61DC, 0x5D28F9AB, 0x69CF5132, 0x7A6DC945
    .long 0x9D14C3B8, 0x8EB65BCF, 0xBA51F356, 0xA9F36B21
    .long 0xD39EA264, 0xC03C3A13, 0xF4DB928A, 0xE7790AFD
    .long 0x3FC5F181, 0x2C6769F6, 0x1880C16F, 0x0B225918
    .long 0x714F905D, 0x62ED082A, 0x560AA0B3, 0x45A838C4
    .long 0xA2D13239, 0xB173AA4E, 0x859402D7, 0x96369AA0
    .long 0xEC5B53E5, 0xFFF9CB92, 0xCB1E630B, 0xD8BCFB7C
    .long 0x7F8BE302, 0x6C297B75, 0x58CED3EC, 0x4B6C4B9B
    .long 0x310182DE, 0x22A31AA9, 0x1644B230, 0x05E62A47
    .long 0xE29F20BA, 0xF13DB8CD, 0xC5DA1054, 0xD6788823
    .long 0xAC154166, 0xBFB7D911, 0x8B507188, 0x98F2E9FF
    .long 0x404E1283, 0x53EC8AF4, 0x670B226D, 0x74A9BA1A
    .long 0x0EC4735F, 0x1D66EB28, 0x298143B1, 0x3A23DBC6
    .long 0xDD5AD13B, 0xCEF8494C, 0xFA1FE1D5, 0xE9BD79A2
    .long 0x93D0B0E7, 0x80722890, 0xB4958009, 0xA737187E
    .long 0xFF17C604, 0xECB55E73, 0xD852F6EA, 0xCBF06E9D
    .long 0xB19DA7D8, 0xA23F3FAF, 0x96D89736, 0x857A0F41
    .long 0x620305BC, 0x71A19DCB, 0x45463552, 0x56E4AD25
    .long 0x2C896460, 0x3F2BFC17, 0x0BCC548E, 0x186ECCF9
    .long 0xC0D23785, 0xD370AFF2, 0xE797076B, 0xF4359F1C
    .long 0x8E585659, 0x9DFACE2E, 0xA91D66B7, 0xBABFFEC0
    .long 0x5DC6F43D, 0x4E646C4A, 0x7A83C4D3, 0x69215CA4
    .long 0x134C95E1, 0x00EE0D96, 0x3409A50F, 0x27AB3D78
    .long 0x809C2506, 0x933EBD71, 0xA7D915E8, 0xB47B8D9F
    .long 0xCE1644DA, 0xDDB4DCAD, 0xE9537434, 0xFAF1EC43
    .long 0x1D88E6BE, 0x0E2A7EC9, 0x3ACDD650, 0x296F4E27
    .long 0x53028762, 0x40A01F15, 0x7447B78C, 0x67E52FFB
    .long 0xBF59D487, 0xACFB4CF0, 0x981CE469, 0x8BBE7C1E
    .long 0xF1D3B55B, 0xE2712D2C, 0xD69685B5, 0xC5341DC2
    .long 0x224D173F, 0x31EF8F48, 0x050827D1, 0x16AABFA6
    .long 0x6CC776E3, 0x7F65EE94, 0x4B82460D, 0x5820DE7A
    .long 0xFBC3FAF9, 0xE861628E, 0xDC86CA17, 0xCF245260
    .long 0xB5499B25, 0xA6EB0352, 0x920CABCB, 0x81AE33BC
    .long 0x66D73941, 0x7575A136, 0x419209AF, 0x523091D8
    .long 0x285D589D, 0x3BFFC0EA, 0x0F186873, 0x1CBAF004
    .long 0xC4060B78, 0xD7A4930F, 0xE3433B96, 0xF0E1A3E1
    .long 0x8A8C6AA4, 0x992EF2D3, 0xADC95A4A, 0xBE6BC23D
    .long 0x5912C8C0, 0x4AB050B7, 0x7E57F82E, 0x6DF56059
    .long 0x1798A91C, 0x043A316B, 0x30DD99F2, 0x237F0185
    .long 0x844819FB, 0x97EA818C, 0xA30D2915, 0xB0AFB162
    .long 0xCAC27827, 0xD960E050, 0xED8748C9, 0xFE25D0BE
    .long 0x195CDA43, 0x0AFE4234, 0x3E19EAAD, 0x2DBB72DA
    .long 0x57D6BB9F, 0x447423E8, 0x70938B71, 0x63311306
    .long 0xBB8DE87A, 0xA82F700D, 0x9CC8D894, 0x8F6A40E3
    .long 0xF50789A6, 0xE6A511D1, 0xD242B948, 0xC1E0213F
    .long 0x26992BC2, 0x353BB3B5, 0x01DC1B2C, 0x127E835B
    .long 0x68134A1E, 0x7BB1D269, 0x4F567AF0, 0x5CF4E287
    .long 0x04D43CFD, 0x1776A48A, 0x23910C13, 0x30339464
    .long 0x4A5E5D21, 0x59FCC556, 0x6D1B6DCF, 0x7EB9F5B8
    .long 0x99C0FF45, 0x8A626732, 0xBE85CFAB, 0xAD2757DC
    .long 0xD74A9E99, 0xC4E806EE, 0xF00FAE77, 0xE3AD3600
    .long 0x3B11CD7C, 0x28B3550B, 0x1C54FD92, 0x0FF665E5
    .long 0x759BACA0, 0x663934D7, 0x52DE9C4E, 0x417C0439
    .long 0xA6050EC4, 0xB5A796B3, 0x81403E2A, 0x92E2A65D
    .long 0xE88F6F18, 0xFB2DF76F, 0xCFCA5FF6, 0xDC68C781
    .long 0x7B5FDFFF, 0x68FD4788, 0x5C1AEF11, 0x4FB87766
    .long 0x35D5BE23, 0x26772654, 0x12908ECD, 0x013216BA
    .long 0xE64B1C47, 0xF5E98430, 0xC10E2CA9, 0xD2ACB4DE
    .long 0xA8C17D9B, 0xBB63E5EC, 0x8F844D75, 0x9C26D502
    .long 0x449A2E7E, 0x5738B609, 0x63DF1E90, 0x707D86E7
    .long 0x0A104FA2, 0x19B2D7D5, 0x2D557F4C, 0x3EF7E73B
    .long 0xD98EEDC6, 0xCA2C75B1, 0xFECBDD28, 0xED69455F
    .long 0x97048C1A, 0x84A6146D, 0xB041BCF4, 0xA3E32483

.Lcrc32c_tab2:
    .long 0x00000000, 0xA541927E, 0x4F6F520D, 0xEA2EC073
    .long 0x9EDEA41A, 0x3B9F3664, 0xD1B1F617, 0x74F06469
    .long 0x38513EC5, 0x9D10ACBB, 0x773E6CC8, 0xD27FFEB6
    .long 0xA68F9ADF, 0x03CE08A1, 0xE9E0C8D2, 0x4CA15AAC
    .long 0x70A27D8A, 0xD5E3EFF4, 0x3FCD2F87, 0x9A8CBDF9
    .long 0xEE7CD990, 0x4B3D4BEE, 0xA1138B9D, 0x045219E3
    .long 0x48F3434F, 0xEDB2D131, 0x079C1142, 0xA2DD833C
    .long 0xD62DE755, 0x736C752B, 0x9942B558, 0x3C032726
    .long 0xE144FB14, 0x4405696A, 0xAE2BA919, 0x0B6A3B67
    .long 0x7F9A5F0E, 0xDADBCD70, 0x30F50D03, 0x95B49F7D
    .long 0xD915C5D1, 0x7C5457AF, 0x967A97DC, 0x333B05A2
    .long 0x47CB61CB, 0xE28AF3B5, 0x08A433C6, 0xADE5A1B8
    .long 0x91E6869E, 0x34A714E0, 0xDE89D493, 0x7BC846ED
    .long 0x0F382284, 0xAA79B0FA, 0x40577089, 0xE516E2F7
    .long 0xA9B7B85B, 0x0CF62A25, 0xE6D8EA56, 0x43997828
    .long 0x37691C41, 0x92288E3F, 0x78064E4C, 0xDD47DC32
    .long 0xC76580D9, 0x622412A7, 0x880AD2D4, 0x2D4B40AA
    .long 0x59BB24C3, 0xFCFAB6BD, 0x16D476CE, 0xB395E4B0
    .long 0xFF34BE1C, 0x5A752C62, 0xB05BEC11, 0x151A7E6F
    .long 0x61EA1A06, 0xC4AB8878, 0x2E85480B, 0x8BC4DA75
    .long 0xB7C7FD53, 0x12866F2D, 0xF8A8AF5E, 0x5DE93D20
    .long 0x29195949, 0x8C58CB37, 0x66760B44, 0xC337993A
    .long 0x8F96C396, 0x2AD751E8, 0xC0F9919B, 0x65B803E5
    .long 0x1148678C, 0xB409F5F2, 0x5E273581, 0xFB66A7FF
    .long 0x26217BCD, 0x8360E9B3, 0x694E29C0, 0xCC0FBBBE
    .long 0xB8FFDFD7, 0x1DBE4DA9, 0xF7908DDA, 0x52D11FA4
    .long 0x1E704508, 0xBB31D776, 0x511F1705, 0xF45E857B
    .long 0x80AEE112, 0x25EF736C, 0xCFC1B31F, 0x6A802161
    .long 0x56830647, 0xF3C29439, 0x19EC544A, 0xBCADC634
    .long 0xC85DA25D, 0x6D1C3023, 0x8732F050, 0x2273622E
    .long 0x6ED23882, 0xCB93AAFC, 0x21BD6A8F, 0x84FCF8F1
    .long 0xF00C9C98, 0x554D0EE6, 0xBF63CE95, 0x1A225CEB
    .long 0x8B277743, 0x2E66E53D, 0xC448254E, 0x6109B730
    .long 0x15F9D359, 0xB0B84127, 0x5A968154, 0xFFD7132A
    .long 0xB3764986, 0x1637DBF8, 0xFC191B8B, 0x595889F5
    .long 0x2DA8ED9C, 0x88E97FE2, 0x62C7BF91, 0xC7862DEF
    .long 0xFB850AC9, 0x5EC498B7, 0xB4EA58C4, 0x11ABCABA
    .long 0x655BAED3, 0xC01A3CAD, 0x2A34FCDE, 0x8F756EA0
    .long 0xC3D4340C, 0x6695A672, 0x8CBB6601, 0x29FAF47F
    .long 0x5D0A9016, 0xF84B0268, 0x1265C21B, 0xB7245065
    .long 0x6A638C57, 0xCF221E29, 0x250CDE5A, 0x804D4C24
    .long 0xF4BD284D, 0x51FCBA33, 0xBBD27A40, 0x1E93E83E
    .long 0x5232B292, 0xF77320EC, 0x1D5DE09F, 0xB81C72E1
    .long 0xCCEC1688, 0x69AD84F6, 0x83834485, 0x26C2D6FB
    .long 0x1AC1F1DD, 0xBF8063A3, 0x55AEA3D0, 0xF0EF31AE
    .long 0x841F55C7, 0x215EC7B9, 0xCB7007CA, 0x6E3195B4
    .long 0x2290CF18, 0x87D15D66, 0x6DFF9D15, 0xC8BE0F6B
    .long 0xBC4E6B02, 0x190FF97C, 0xF321390F, 0x5660AB71
    .long 0x4C42F79A, 0xE90365E4, 0x032DA597, 0xA66C37E9
    .long 0xD29C5380, 0x77DDC1FE, 0x9DF3018D, 0x38B293F3
    .long 0x7413C95F, 0xD1525B21, 0x3B7C9B52, 0x9E3D092C
    .long 0xEACD6D45, 0x4F8CFF3B, 0xA5A23F48, 0x00E3AD36
    .long 0x3CE08A10, 0x99A1186E, 0x738FD81D, 0xD6CE4A63
    .long 0xA23E2E0A, 0x077FBC74, 0xED517C07, 0x4810EE79
    .long 0x04B1B4D5, 0xA1F026AB, 0x4BDEE6D8, 0xEE9F74A6
    .long 0x9A6F10CF, 0x3F2E82B1, 0xD50042C2, 0x7041D0BC
    .long 0xAD060C8E, 0x08479EF0, 0xE2695E83, 0x4728CCFD
    .long 0x33D8A894, 0x96993AEA, 0x7CB7FA99, 0xD9F668E7
    .long 0x9557324B, 0x3016A035, 0xDA386046, 0x7F79F238
    .long 0x0B899651, 0xAEC8042F, 0x44E6C45C, 0xE1A75622
    .long 0xDDA47104, 0x78E5E37A, 0x92CB2309, 0x378AB177
    .long 0x437AD51E, 0xE63B4760, 0x0C158713, 0xA954156D
    .long 0xE5F54FC1, 0x40B4DDBF, 0xAA9A1DCC, 0x0FDB8FB2
    .long 0x7B2BEBDB, 0xDE6A79A5, 0x3444B9D6, 0x91052BA8

.Lcrc32c_tab3:
    .long 0x00000000, 0xDD45AAB8, 0xBF672381, 0x62228939
    .long 0x7B2231F3, 0xA6679B4B, 0xC4451272, 0x1900B8CA
    .long 0xF64463E6, 0x2B01C95E, 0x49234067, 0x9466EADF
    .long 0x8D665215, 0x5023F8AD, 0x32017194, 0xEF44DB2C
    .long 0xE964B13D, 0x34211B85, 0x560392BC, 0x8B463804
    .long 0x924680CE, 0x4F032A76, 0x2D21A34F, 0xF06409F7
    .long 0x1F20D2DB, 0xC2657863, 0xA047F15A, 0x7D025BE2
    .long 0x6402E328, 0xB9474990, 0xDB65C0A9, 0x06206A11
    .long 0xD725148B, 0x0A60BE33, 0x6842370A, 0xB5079DB2
    .long 0xAC072578, 0x71428FC0, 0x136006F9, 0xCE25AC41
    .long 0x2161776D, 0xFC24DDD5, 0x9E0654EC, 0x4343FE54
    .long 0x5A43469E, 0x8706EC26, 0xE524651F, 0x3861CFA7
    .long 0x3E41A5B6, 0xE3040F0E, 0x81268637, 0x5C632C8F
    .long 0x45639445, 0x98263EFD, 0xFA04B7C4, 0x27411D7C
    .long 0xC805C650, 0x15406CE8, 0x7762E5D1, 0xAA274F69
    .long 0xB327F7A3, 0x6E625D1B, 0x0C40D422, 0xD1057E9A
    .long 0xABA65FE7, 0x76E3F55F, 0x14C17C66, 0xC984D6DE
    .long 0xD0846E14, 0x0DC1C4AC, 0x6FE34D95, 0xB2A6E72D
    .long 0x5DE23C01, 0x80A796B9, 0xE2851F80, 0x3FC0B538
    .long 0x26C00DF2, 0xFB85A74A, 0x99A72E73, 0x44E284CB
    .long 0x42C2EEDA, 0x9F874462, 0xFDA5CD5B, 0x20E067E3
    .long 0x39E0DF29, 0xE4A57591, 0x8687FCA8, 0x5BC25610
    .long 0xB4868D3C, 0x69C32784, 0x0BE1AEBD, 0xD6A40405
    .long 0xCFA4BCCF, 0x12E11677, 0x70C39F4E, 0xAD8635F6
    .long 0x7C834B6C, 0xA1C6E1D4, 0xC3E468ED, 0x1EA1C255
    .long 0x07A17A9F, 0xDAE4D027, 0xB8C6591E, 0x6583F3A6
    .long 0x8AC7288A, 0x57828232, 0x35A00B0B, 0xE8E5A1B3
    .long 0xF1E51979, 0x2CA0B3C1, 0x4E823AF8, 0x93C79040
    .long 0x95E7FA51, 0x48A250E9, 0x2A80D9D0, 0xF7C57368
    .long 0xEEC5CBA2, 0x3380611A, 0x51A2E823, 0x8CE7429B
    .long 0x63A399B7, 0xBEE6330F, 0xDCC4BA36, 0x0181108E
    .long 0x1881A844, 0xC5C402FC, 0xA7E68BC5, 0x7AA3217D
    .long 0x52A0C93F, 0x8FE56387, 0xEDC7EABE, 0x30824006
    .long 0x2982F8CC, 0xF4C75274, 0x96E5DB4D, 0x4BA071F5
    .long 0xA4E4AAD9, 0x79A10061, 0x1B838958, 0xC6C623E0
    .long 0xDFC69B2A, 0x02833192, 0x60A1B8AB, 0xBDE41213
    .long 0xBBC47802, 0x6681D2BA, 0x04A35B83, 0xD9E6F13B
    .long 0xC0E649F1, 0x1DA3E349, 0x7F816A70, 0xA2C4C0C8
    .long 0x4D801BE4, 0x90C5B15C, 0xF2E73865, 0x2FA292DD
    .long 0x36A22A17, 0xEBE780AF, 0x89C50996, 0x5480A32E
    .long 0x8585DDB4, 0x58C0770C, 0x3AE2FE35, 0xE7A7548D
    .long 0xFEA7EC47, 0x23E246FF, 0x41C0CFC6, 0x9C85657E
    .long 0x73C1BE52, 0xAE8414EA, 0xCCA69DD3, 0x11E3376B
    .long 0x08E38FA1, 0xD5A62519, 0xB784AC20, 0x6AC10698
    .long 0x6CE16C89, 0xB1A4C631, 0xD3864F08, 0x0EC3E5B0
    .long 0x17C35D7A, 0xCA86F7C2, 0xA8A47EFB, 0x75E1D443
    .long 0x9AA50F6F, 0x47E0A5D7, 0x25C22CEE, 0xF8878656
    .long 0xE1873E9C, 0x3CC29424, 0x5EE01D1D, 0x83A5B7A5
    .long 0xF90696D8, 0x24433C60, 0x4661B559, 0x9B241FE1
    .long 0x8224A72B, 0x5F610D93, 0x3D4384AA, 0xE0062E12
    .long 0x0F42F53E, 0xD2075F86, 0xB025D6BF, 0x6D607C07
    .long 0x7460C4CD, 0xA9256E75, 0xCB07E74C, 0x16424DF4
    .long 0x106227E5, 0xCD278D5D, 0xAF050464, 0x7240AEDC
    .long 0x6B401616, 0xB605BCAE, 0xD4273597, 0x09629F2F
    .long 0xE6264403, 0x3B63EEBB, 0x59416782, 0x8404CD3A
    .long 0x9D0475F0, 0x4041DF48, 0x22635671, 0xFF26FCC9
    .long 0x2E238253, 0xF36628EB, 0x9144A1D2, 0x4C010B6A
    .long 0x5501B3A0, 0x88441918, 0xEA669021, 0x37233A99
    .long 0xD867E1B5, 0x05224B0D, 0x6700C234, 0xBA45688C
    .long 0xA345D046, 0x7E007AFE, 0x1C22F3C7, 0xC167597F
    .long 0xC747336E, 0x1A0299D6, 0x782010EF, 0xA565BA57
    .long 0xBC65029D, 0x6120A825, 0x0302211C, 0xDE478BA4
    .long 0x31035088, 0xEC46FA30, 0x8E647309, 0x5321D9B1
    .long 0x4A21617B, 0x9764CBC3, 0xF54642FA, 0x2803E842
