; test_aes_crc.asm - Test AES-NI, PCLMULQDQ, and CRC32 instructions
; AESENC, AESENCLAST, AESDEC, AESDECLAST, AESIMC, AESKEYGENASSIST,
; PCLMULQDQ (all imm8 selectors), CRC32 (byte, dword, qword operands)
%include "test_framework.inc"

section .data
    ; ---- Test name strings ----
    tn_aesenc_z_lo:        db "aesenc(0,0) lo64", 0
    tn_aesenc_z_hi:        db "aesenc(0,0) hi64", 0
    tn_aesenc_63_lo:       db "aesenc(0x63..,0x63..) lo64", 0
    tn_aesenc_63_hi:       db "aesenc(0x63..,0x63..) hi64", 0
    tn_aesenc_zk63_lo:     db "aesenc(0,0x63..) lo64", 0
    tn_aesenc_zk63_hi:     db "aesenc(0,0x63..) hi64", 0
    tn_aesenc_mem_lo:      db "aesenc(0,mem 0) lo64", 0
    tn_aesenc_mem_hi:      db "aesenc(0,mem 0) hi64", 0
    tn_aesenclast_z_lo:    db "aesenclast(0,0) lo64", 0
    tn_aesenclast_z_hi:    db "aesenclast(0,0) hi64", 0
    tn_aesenclast_63_lo:   db "aesenclast(0x63..,0) lo64", 0
    tn_aesenclast_63_hi:   db "aesenclast(0x63..,0) hi64", 0
    tn_aesdec_z_lo:        db "aesdec(0,0) lo64", 0
    tn_aesdec_z_hi:        db "aesdec(0,0) hi64", 0
    tn_aesdeclast_z_lo:    db "aesdeclast(0,0) lo64", 0
    tn_aesdeclast_z_hi:    db "aesdeclast(0,0) hi64", 0
    tn_aesimc_63_lo:       db "aesimc(0x63..) lo64", 0
    tn_aesimc_63_hi:       db "aesimc(0x63..) hi64", 0
    tn_aesimc_mem_lo:      db "aesimc(mem 0x63..) lo64", 0
    tn_aesimc_mem_hi:      db "aesimc(mem 0x63..) hi64", 0
    tn_keygen_z_lo:        db "aeskeygenassist(0,0x01) lo64", 0
    tn_keygen_z_hi:        db "aeskeygenassist(0,0x01) hi64", 0
    tn_clmul_00_lo:        db "pclmulqdq(1,1,0x00) lo64", 0
    tn_clmul_00_hi:        db "pclmulqdq(1,1,0x00) hi64", 0
    tn_clmul_3x5_lo:       db "pclmulqdq(3,5,0x00) lo64", 0
    tn_clmul_3x5_hi:       db "pclmulqdq(3,5,0x00) hi64", 0
    tn_clmul_hi_lo:        db "pclmulqdq(bit63,bit63,0x00) lo64", 0
    tn_clmul_hi_hi:        db "pclmulqdq(bit63,bit63,0x00) hi64", 0
    tn_clmul_11_lo:        db "pclmulqdq(3,5,0x11) lo64", 0
    tn_clmul_11_hi:        db "pclmulqdq(3,5,0x11) hi64", 0
    tn_clmul_01_lo:        db "pclmulqdq(7,1,0x01) lo64", 0
    tn_clmul_01_hi:        db "pclmulqdq(7,1,0x01) hi64", 0
    tn_crc32_b_zero:       db "crc32 r32,r/m8 (0,0x00)", 0
    tn_crc32_b_one:        db "crc32 r32,r/m8 (0,0x01)", 0
    tn_crc32_d_zero:       db "crc32 r32,r/m32 (0,0)", 0
    tn_crc32_q_zero:       db "crc32 r64,r/m64 (0,0)", 0

    align 16
    ; ---- Test vectors ----
    vec_zero:    times 4 dd 0x00000000
    vec_all_63:  times 4 dd 0x63636363
    ; PCLMULQDQ: xmm = [lo64=1, hi64=0]
    vec_one:     dq 0x0000000000000001, 0x0000000000000000
    ; PCLMULQDQ: xmm = [lo64=3, hi64=0]
    vec_three:   dq 0x0000000000000003, 0x0000000000000000
    ; PCLMULQDQ: xmm = [lo64=5, hi64=0]
    vec_five:    dq 0x0000000000000005, 0x0000000000000000
    ; PCLMULQDQ: xmm = [lo64=bit63, hi64=0]
    vec_bit63:   dq 0x8000000000000000, 0x0000000000000000
    ; PCLMULQDQ: xmm = [lo64=X, hi64=3] for imm=0x11 test
    vec_hi3:     dq 0x0000000000000000, 0x0000000000000003
    ; PCLMULQDQ: xmm = [lo64=X, hi64=5] for imm=0x11 test
    vec_hi5:     dq 0x0000000000000000, 0x0000000000000005
    ; PCLMULQDQ: xmm = [lo64=1, hi64=7] for imm=0x01 cross test
    vec_lo1_hi7: dq 0x0000000000000001, 0x0000000000000007

section .text
global _start

_start:
    INIT_TESTS

    ; ================================================================
    ; AESENC: state=all-zero, key=all-zero
    ; SubBytes(0x00) = 0x63 for every byte -> all-0x63 state
    ; ShiftRows on constant state -> unchanged
    ; MixColumns on constant column [c,c,c,c] -> [c,c,c,c]
    ;   (because (2 XOR 3 XOR 1 XOR 1) = 1 in GF(2^8))
    ; AddRoundKey with zero key -> all-0x63
    ; Result: 0x63636363 63636363 63636363 63636363
    ; ================================================================
    pxor xmm0, xmm0                ; state = 0
    pxor xmm1, xmm1                ; key = 0
    aesenc xmm0, xmm1

    TEST_CASE tn_aesenc_z_lo
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x6363636363636363

    TEST_CASE tn_aesenc_z_hi
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x6363636363636363

    ; ================================================================
    ; AESENC: state=all-0x63, key=all-0x63
    ; SubBytes(0x63) = 0xFB -> all-0xFB state
    ; ShiftRows/MixColumns on constant -> unchanged (all-0xFB)
    ; AddRoundKey: 0xFB XOR 0x63 = 0x98 -> all-0x98
    ; ================================================================
    movdqa xmm0, [rel vec_all_63]  ; state = all 0x63
    movdqa xmm1, [rel vec_all_63]  ; key = all 0x63
    aesenc xmm0, xmm1

    TEST_CASE tn_aesenc_63_lo
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x9898989898989898

    TEST_CASE tn_aesenc_63_hi
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x9898989898989898

    ; ================================================================
    ; AESENC: state=all-zero, key=all-0x63
    ; SubBytes(0x00) = 0x63 -> ShiftRows/MixColumns -> all-0x63
    ; AddRoundKey: 0x63 XOR 0x63 = 0x00 -> all-zero
    ; ================================================================
    pxor xmm0, xmm0                ; state = 0
    movdqa xmm1, [rel vec_all_63]  ; key = all 0x63
    aesenc xmm0, xmm1

    TEST_CASE tn_aesenc_zk63_lo
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0000000000000000

    TEST_CASE tn_aesenc_zk63_hi
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x0000000000000000

    ; ================================================================
    ; AESENC with memory operand: state=0, key=mem(0)
    ; Same as first AESENC test -> all-0x63
    ; ================================================================
    pxor xmm0, xmm0
    aesenc xmm0, [rel vec_zero]

    TEST_CASE tn_aesenc_mem_lo
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x6363636363636363

    TEST_CASE tn_aesenc_mem_hi
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x6363636363636363

    ; ================================================================
    ; AESENCLAST: state=all-zero, key=all-zero
    ; SubBytes(0x00) = 0x63 -> all-0x63
    ; ShiftRows on constant -> unchanged
    ; NO MixColumns (last round)
    ; AddRoundKey with zero key -> all-0x63
    ; ================================================================
    pxor xmm0, xmm0
    pxor xmm1, xmm1
    aesenclast xmm0, xmm1

    TEST_CASE tn_aesenclast_z_lo
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x6363636363636363

    TEST_CASE tn_aesenclast_z_hi
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x6363636363636363

    ; ================================================================
    ; AESENCLAST: state=all-0x63, key=all-zero
    ; SubBytes(0x63) = 0xFB -> all-0xFB
    ; ShiftRows on constant -> unchanged
    ; NO MixColumns
    ; AddRoundKey with zero key -> all-0xFB
    ; ================================================================
    movdqa xmm0, [rel vec_all_63]
    pxor xmm1, xmm1
    aesenclast xmm0, xmm1

    TEST_CASE tn_aesenclast_63_lo
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xFBFBFBFBFBFBFBFB

    TEST_CASE tn_aesenclast_63_hi
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0xFBFBFBFBFBFBFBFB

    ; ================================================================
    ; AESDEC: state=all-zero, key=all-zero
    ; InvSubBytes(0x00) = 0x52 -> all-0x52
    ; InvShiftRows on constant -> unchanged
    ; InvMixColumns on constant column [c,c,c,c] -> [c,c,c,c]
    ;   (because (0E XOR 0B XOR 0D XOR 09) = 01 in GF(2^8))
    ; AddRoundKey with zero key -> all-0x52
    ; ================================================================
    pxor xmm0, xmm0
    pxor xmm1, xmm1
    aesdec xmm0, xmm1

    TEST_CASE tn_aesdec_z_lo
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x5252525252525252

    TEST_CASE tn_aesdec_z_hi
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x5252525252525252

    ; ================================================================
    ; AESDECLAST: state=all-zero, key=all-zero
    ; InvSubBytes(0x00) = 0x52 -> all-0x52
    ; InvShiftRows on constant -> unchanged
    ; NO InvMixColumns (last round)
    ; AddRoundKey with zero key -> all-0x52
    ; ================================================================
    pxor xmm0, xmm0
    pxor xmm1, xmm1
    aesdeclast xmm0, xmm1

    TEST_CASE tn_aesdeclast_z_lo
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x5252525252525252

    TEST_CASE tn_aesdeclast_z_hi
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x5252525252525252

    ; ================================================================
    ; AESIMC: input=all-0x63
    ; InvMixColumns on constant column [c,c,c,c] -> [c,c,c,c]
    ; Result: all-0x63
    ; ================================================================
    movdqa xmm0, [rel vec_all_63]
    aesimc xmm1, xmm0

    TEST_CASE tn_aesimc_63_lo
    movq rax, xmm1
    CHECK_EQ_64 rax, 0x6363636363636363

    TEST_CASE tn_aesimc_63_hi
    pextrq rax, xmm1, 1
    CHECK_EQ_64 rax, 0x6363636363636363

    ; ================================================================
    ; AESIMC with memory operand: same test from memory
    ; ================================================================
    aesimc xmm1, [rel vec_all_63]

    TEST_CASE tn_aesimc_mem_lo
    movq rax, xmm1
    CHECK_EQ_64 rax, 0x6363636363636363

    TEST_CASE tn_aesimc_mem_hi
    pextrq rax, xmm1, 1
    CHECK_EQ_64 rax, 0x6363636363636363

    ; ================================================================
    ; AESKEYGENASSIST: input=all-zero, imm8=0x01
    ; For input XMM [X0, X1, X2, X3] (dwords, X0=lowest):
    ;   Dst[31:0]   = SubWord(X1) = SubWord(0) = 0x63636363
    ;   Dst[63:32]  = RotWord(SubWord(X1)) XOR RCON
    ;               = RotWord(0x63636363) XOR 0x00000001
    ;               = 0x63636363 XOR 0x00000001 = 0x63636362
    ;   Dst[95:64]  = SubWord(X3) = 0x63636363
    ;   Dst[127:96] = RotWord(SubWord(X3)) XOR RCON = 0x63636362
    ; lo64 = 0x6363636263636363
    ; hi64 = 0x6363636263636363
    ; ================================================================
    pxor xmm0, xmm0
    aeskeygenassist xmm1, xmm0, 0x01

    TEST_CASE tn_keygen_z_lo
    movq rax, xmm1
    CHECK_EQ_64 rax, 0x6363636263636363

    TEST_CASE tn_keygen_z_hi
    pextrq rax, xmm1, 1
    CHECK_EQ_64 rax, 0x6363636263636363

    ; ================================================================
    ; PCLMULQDQ: carry-less multiply of lo64 * lo64 (imm8=0x00)
    ; a_lo = 1, b_lo = 1 -> clmul(1,1) = 1
    ; lo64 = 1, hi64 = 0
    ; ================================================================
    movdqa xmm0, [rel vec_one]
    movdqa xmm1, [rel vec_one]
    pclmulqdq xmm0, xmm1, 0x00

    TEST_CASE tn_clmul_00_lo
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0000000000000001

    TEST_CASE tn_clmul_00_hi
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x0000000000000000

    ; ================================================================
    ; PCLMULQDQ: clmul(3, 5) with imm8=0x00
    ; 0b11 * 0b101 (carry-less):
    ;   bit0 of 3: 1 -> 101
    ;   bit1 of 3: 1 -> 1010
    ;   XOR: 0101 XOR 1010 = 1111 = 0x0F
    ; lo64 = 0x0F, hi64 = 0
    ; ================================================================
    movdqa xmm0, [rel vec_three]
    movdqa xmm1, [rel vec_five]
    pclmulqdq xmm0, xmm1, 0x00

    TEST_CASE tn_clmul_3x5_lo
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x000000000000000F

    TEST_CASE tn_clmul_3x5_hi
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x0000000000000000

    ; ================================================================
    ; PCLMULQDQ: clmul(0x8000000000000000, 0x8000000000000000) imm8=0x00
    ; bit63 * bit63 = bit126 of 128-bit result
    ; bit126 is in hi64 at bit position 62 -> hi64 = 0x4000000000000000
    ; lo64 = 0
    ; ================================================================
    movdqa xmm0, [rel vec_bit63]
    movdqa xmm1, [rel vec_bit63]
    pclmulqdq xmm0, xmm1, 0x00

    TEST_CASE tn_clmul_hi_lo
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0000000000000000

    TEST_CASE tn_clmul_hi_hi
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x4000000000000000

    ; ================================================================
    ; PCLMULQDQ: imm8=0x11 (hi64_of_a * hi64_of_b)
    ; a_hi = 3, b_hi = 5 -> clmul(3,5) = 0x0F (same as above)
    ; lo64 = 0x0F, hi64 = 0
    ; ================================================================
    movdqa xmm0, [rel vec_hi3]
    movdqa xmm1, [rel vec_hi5]
    pclmulqdq xmm0, xmm1, 0x11

    TEST_CASE tn_clmul_11_lo
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x000000000000000F

    TEST_CASE tn_clmul_11_hi
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x0000000000000000

    ; ================================================================
    ; PCLMULQDQ: imm8=0x01 (hi64_of_a * lo64_of_b)
    ; a = [lo64=1, hi64=7], b = [lo64=1, hi64=X]
    ; Selects: hi64_of_a=7, lo64_of_b=1
    ; clmul(7, 1) = 7
    ; lo64 = 7, hi64 = 0
    ; ================================================================
    movdqa xmm0, [rel vec_lo1_hi7]
    movdqa xmm1, [rel vec_one]
    pclmulqdq xmm0, xmm1, 0x01

    TEST_CASE tn_clmul_01_lo
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0000000000000007

    TEST_CASE tn_clmul_01_hi
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x0000000000000000

    ; ================================================================
    ; CRC32 r32, r/m8: initial=0, byte=0x00 -> 0
    ; Trivially correct: CRC of zero data with zero initial = 0
    ; ================================================================
    xor eax, eax
    xor ecx, ecx
    crc32 eax, cl

    TEST_CASE tn_crc32_b_zero
    CHECK_EQ_32 eax, 0x00000000

    ; ================================================================
    ; CRC32 r32, r/m8: initial=0, byte=0x01 -> 0xF26B8303
    ; Well-known CRC-32C (Castagnoli) result for single byte 0x01
    ; ================================================================
    xor eax, eax
    mov cl, 0x01
    crc32 eax, cl

    TEST_CASE tn_crc32_b_one
    CHECK_EQ_32 eax, 0xF26B8303

    ; ================================================================
    ; CRC32 r32, r/m32: initial=0, dword=0 -> 0
    ; ================================================================
    xor eax, eax
    xor ecx, ecx
    crc32 eax, ecx

    TEST_CASE tn_crc32_d_zero
    CHECK_EQ_32 eax, 0x00000000

    ; ================================================================
    ; CRC32 r64, r/m64: initial=0, qword=0 -> 0
    ; ================================================================
    xor eax, eax
    xor ecx, ecx
    crc32 rax, rcx

    TEST_CASE tn_crc32_q_zero
    CHECK_EQ_64 rax, 0x0000000000000000

    END_TESTS
