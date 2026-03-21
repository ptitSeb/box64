; test_sse_shuffle.asm - Test SSE shuffle/permute/interleave instructions
; SHUFPS, SHUFPD, PSHUFD, PSHUFLW, PSHUFHW, PSHUFB
; UNPCKLPS, UNPCKHPS, UNPCKLPD, UNPCKHPD
; MOVHLPS, MOVLHPS
; INSERTPS, EXTRACTPS (SSE4.1)
; PUNPCKLBW/WD/DQ/QDQ, PUNPCKHBW/WD/DQ/QDQ
; PALIGNR (SSSE3)
%include "test_framework.inc"

section .data
    t1_name:  db "shufps imm=0x00", 0
    t2_name:  db "shufps imm=0xE4", 0
    t3_name:  db "shufps imm=0x1B", 0
    t4_name:  db "shufpd imm=0x0", 0
    t5_name:  db "shufpd imm=0x3", 0
    t6_name:  db "pshufd imm=0x1B", 0
    t7_name:  db "pshufd imm=0x00", 0
    t8_name:  db "pshuflw imm=0x1B", 0
    t9_name:  db "pshufhw imm=0x1B", 0
    t10_name: db "unpcklps", 0
    t11_name: db "unpckhps", 0
    t12_name: db "unpcklpd", 0
    t13_name: db "unpckhpd", 0
    t14_name: db "movhlps", 0
    t15_name: db "movlhps", 0
    t16_name: db "pshufb zero mask", 0
    t17_name: db "pshufb shuffle", 0
    t18_name: db "insertps", 0
    t19_name: db "extractps", 0
    t20_name: db "punpcklbw", 0
    t21_name: db "punpckhbw", 0
    t22_name: db "punpckldq", 0
    t23_name: db "punpcklqdq", 0
    t24_name: db "punpckhqdq", 0
    t25_name: db "palignr 4", 0
    t26_name: db "palignr 8", 0
    t27_name: db "pshufd broadcast", 0
    t28_name: db "shufps cross", 0
    t29_name: db "punpcklwd", 0
    t30_name: db "punpckhwd", 0
    t31_name: db "pshufb reverse", 0
    t32_name: db "shufpd imm=0x1", 0
    t33_name: db "palignr 1", 0
    t34_name: db "insertps zero", 0
    t35_name: db "unpcklps self", 0

    align 16
    ; Float data (as dwords for easy verification)
    ; {1.0, 2.0, 3.0, 4.0}
    f_1234:    dd 0x3F800000, 0x40000000, 0x40400000, 0x40800000
    ; {5.0, 6.0, 7.0, 8.0}
    f_5678:    dd 0x40A00000, 0x40C00000, 0x40E00000, 0x41000000

    ; Double data
    ; {1.0, 2.0}
    d_12:      dq 0x3FF0000000000000, 0x4000000000000000
    ; {3.0, 4.0}
    d_34:      dq 0x4008000000000000, 0x4010000000000000

    ; Integer data (dwords)
    i_1234:    dd 0x11111111, 0x22222222, 0x33333333, 0x44444444
    i_5678:    dd 0x55555555, 0x66666666, 0x77777777, 0x88888888

    ; Byte data for pshufb/punpck
    b_data1:   db 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77
               db 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF
    b_data2:   db 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7
               db 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF

    ; pshufb control masks
    ; All high bits set -> zero out all bytes
    shuf_zero: db 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80
               db 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80
    ; Reverse bytes within each 128-bit lane: 15,14,...,1,0
    shuf_rev:  db 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
    ; Simple shuffle: take bytes 0,0,0,0,...
    shuf_bcast: db 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    ; Identity shuffle
    shuf_ident: db 3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12

    ; Word data for punpcklw/punpckhw
    w_data1:   dw 0x1111, 0x2222, 0x3333, 0x4444, 0x5555, 0x6666, 0x7777, 0x8888
    w_data2:   dw 0xAAAA, 0xBBBB, 0xCCCC, 0xDDDD, 0xEEEE, 0xFFFF, 0x9999, 0x0000

section .text
global _start

_start:
    INIT_TESTS

    ; ==== Test 1: SHUFPS imm=0x00 - broadcast dword0 from each source ====
    ; shufps xmm0, xmm1, imm8
    ; Result: [src1[imm[1:0]], src1[imm[3:2]], src2[imm[5:4]], src2[imm[7:6]]]
    ; imm=0x00: [src1[0], src1[0], src2[0], src2[0]]
    ; src1={1.0,2.0,3.0,4.0}, src2={5.0,6.0,7.0,8.0}
    ; Result: {1.0, 1.0, 5.0, 5.0}
    ; Low 64: dw0=0x3F800000(1.0), dw1=0x3F800000(1.0) -> 0x3F8000003F800000
    TEST_CASE t1_name
    movdqa xmm0, [rel f_1234]
    movdqa xmm1, [rel f_5678]
    shufps xmm0, xmm1, 0x00
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x3F8000003F800000

    ; ==== Test 2: SHUFPS imm=0xE4 - identity (no shuffle) ====
    ; imm=0xE4=11_10_01_00: [src1[0], src1[1], src2[2], src2[3]]
    ; Result: {1.0, 2.0, 7.0, 8.0}
    ; Low 64: dw0=0x3F800000, dw1=0x40000000 -> 0x400000003F800000
    TEST_CASE t2_name
    movdqa xmm0, [rel f_1234]
    movdqa xmm1, [rel f_5678]
    shufps xmm0, xmm1, 0xE4
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x400000003F800000

    ; ==== Test 3: SHUFPS imm=0x1B - reverse within each pair ====
    ; imm=0x1B=00_01_10_11: [src1[3], src1[2], src2[1], src2[0]]
    ; Result: {4.0, 3.0, 6.0, 5.0}
    ; Low 64: dw0=0x40800000(4.0), dw1=0x40400000(3.0) -> 0x4040000040800000
    TEST_CASE t3_name
    movdqa xmm0, [rel f_1234]
    movdqa xmm1, [rel f_5678]
    shufps xmm0, xmm1, 0x1B
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x4040000040800000

    ; ==== Test 4: SHUFPD imm=0x0 - low from both ====
    ; shufpd xmm0, xmm1, imm8
    ; Result: [src1[imm[0]], src2[imm[1]]]
    ; imm=0: [src1[0], src2[0]] = {1.0, 3.0}
    ; Low 64: 1.0 = 0x3FF0000000000000
    TEST_CASE t4_name
    movdqa xmm0, [rel d_12]
    movdqa xmm1, [rel d_34]
    shufpd xmm0, xmm1, 0x0
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x3FF0000000000000

    ; ==== Test 5: SHUFPD imm=0x3 - high from both ====
    ; imm=0x3=11: [src1[1], src2[1]] = {2.0, 4.0}
    ; Low 64: 2.0 = 0x4000000000000000
    TEST_CASE t5_name
    movdqa xmm0, [rel d_12]
    movdqa xmm1, [rel d_34]
    shufpd xmm0, xmm1, 0x3
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x4000000000000000

    ; ==== Test 6: PSHUFD imm=0x1B - reverse dwords ====
    ; pshufd xmm0, xmm1, imm: dst[i] = src[imm field i]
    ; imm=0x1B=00_01_10_11: [src[3], src[2], src[1], src[0]]
    ; src = {0x11111111, 0x22222222, 0x33333333, 0x44444444}
    ; Result: {0x44444444, 0x33333333, 0x22222222, 0x11111111}
    ; Low 64: dw0=0x44444444, dw1=0x33333333 -> 0x3333333344444444
    TEST_CASE t6_name
    movdqa xmm1, [rel i_1234]
    pshufd xmm0, xmm1, 0x1B
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x3333333344444444

    ; ==== Test 7: PSHUFD imm=0x00 - broadcast dword 0 ====
    ; imm=0x00: [src[0], src[0], src[0], src[0]]
    ; Result: {0x11111111, 0x11111111, 0x11111111, 0x11111111}
    TEST_CASE t7_name
    movdqa xmm1, [rel i_1234]
    pshufd xmm0, xmm1, 0x00
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x1111111111111111

    ; ==== Test 8: PSHUFLW imm=0x1B - reverse low 4 words, high unchanged ====
    ; pshuflw: shuffles words 0-3 (low 64 bits), words 4-7 unchanged
    ; imm=0x1B=00_01_10_11: [w3, w2, w1, w0, w4, w5, w6, w7]
    ; src words: 0x1111, 0x2222, 0x3333, 0x4444, 0x5555, 0x6666, 0x7777, 0x8888
    ; Low 4 reversed: 0x4444, 0x3333, 0x2222, 0x1111
    ; Low 64: 0x1111222233334444
    TEST_CASE t8_name
    movdqa xmm1, [rel w_data1]
    pshuflw xmm0, xmm1, 0x1B
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x1111222233334444

    ; ==== Test 9: PSHUFHW imm=0x1B - reverse high 4 words, low unchanged ====
    ; pshufhw: shuffles words 4-7 (high 64 bits), words 0-3 unchanged
    ; imm=0x1B: high words reversed: [w0, w1, w2, w3, w7, w6, w5, w4]
    ; Low 64 unchanged: 0x4444333322221111 (original low qword of w_data1)
    TEST_CASE t9_name
    movdqa xmm1, [rel w_data1]
    pshufhw xmm0, xmm1, 0x1B
    movq rax, xmm0
    ; Low 64 bits are unchanged: words 0x1111, 0x2222, 0x3333, 0x4444
    ; As LE qword: 0x4444333322221111
    CHECK_EQ_64 rax, 0x4444333322221111

    ; ==== Test 10: UNPCKLPS - interleave low dwords ====
    ; unpcklps xmm0, xmm1: dst = [src1[0], src2[0], src1[1], src2[1]]
    ; src1={1.0,2.0,3.0,4.0}, src2={5.0,6.0,7.0,8.0}
    ; Result: {1.0, 5.0, 2.0, 6.0}
    ; Low 64: dw0=0x3F800000(1.0), dw1=0x40A00000(5.0) -> 0x40A000003F800000
    TEST_CASE t10_name
    movdqa xmm0, [rel f_1234]
    movdqa xmm1, [rel f_5678]
    unpcklps xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x40A000003F800000

    ; ==== Test 11: UNPCKHPS - interleave high dwords ====
    ; unpckhps xmm0, xmm1: dst = [src1[2], src2[2], src1[3], src2[3]]
    ; Result: {3.0, 7.0, 4.0, 8.0}
    ; Low 64: dw0=0x40400000(3.0), dw1=0x40E00000(7.0) -> 0x40E0000040400000
    TEST_CASE t11_name
    movdqa xmm0, [rel f_1234]
    movdqa xmm1, [rel f_5678]
    unpckhps xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x40E0000040400000

    ; ==== Test 12: UNPCKLPD - interleave low qwords ====
    ; unpcklpd xmm0, xmm1: dst = [src1[0], src2[0]]
    ; src1={1.0, 2.0}, src2={3.0, 4.0}
    ; Result: {1.0, 3.0}
    ; Low 64: 1.0 = 0x3FF0000000000000
    TEST_CASE t12_name
    movdqa xmm0, [rel d_12]
    movdqa xmm1, [rel d_34]
    unpcklpd xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x3FF0000000000000

    ; ==== Test 13: UNPCKHPD - interleave high qwords ====
    ; unpckhpd xmm0, xmm1: dst = [src1[1], src2[1]]
    ; Result: {2.0, 4.0}
    ; Low 64: 2.0 = 0x4000000000000000
    TEST_CASE t13_name
    movdqa xmm0, [rel d_12]
    movdqa xmm1, [rel d_34]
    unpckhpd xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x4000000000000000

    ; ==== Test 14: MOVHLPS - move high to low ====
    ; movhlps xmm0, xmm1: xmm0[63:0] = xmm1[127:64], xmm0[127:64] unchanged
    ; xmm0 = {1.0, 2.0, 3.0, 4.0}, xmm1 = {5.0, 6.0, 7.0, 8.0}
    ; xmm0 low = xmm1 high = {7.0, 8.0} as dwords
    ; Low 64: 0x4100000040E00000
    TEST_CASE t14_name
    movdqa xmm0, [rel f_1234]
    movdqa xmm1, [rel f_5678]
    movhlps xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x4100000040E00000

    ; ==== Test 15: MOVLHPS - move low to high ====
    ; movlhps xmm0, xmm1: xmm0[127:64] = xmm1[63:0], xmm0[63:0] unchanged
    ; xmm0 = {1.0, 2.0, 3.0, 4.0}, xmm1 = {5.0, 6.0, 7.0, 8.0}
    ; xmm0 low unchanged = {1.0, 2.0}, xmm0 high = xmm1 low = {5.0, 6.0}
    ; Low 64: unchanged = 0x400000003F800000
    TEST_CASE t15_name
    movdqa xmm0, [rel f_1234]
    movdqa xmm1, [rel f_5678]
    movlhps xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x400000003F800000

    ; ==== Test 16: PSHUFB all zero mask (high bit set) ====
    ; When mask byte has bit 7 set, result byte = 0
    TEST_CASE t16_name
    movdqa xmm0, [rel b_data1]
    movdqa xmm1, [rel shuf_zero]
    pshufb xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0

    ; ==== Test 17: PSHUFB identity-like shuffle ====
    ; shuf_ident: {3,2,1,0, 7,6,5,4, 11,10,9,8, 15,14,13,12} - reverse within dwords
    ; b_data1: 00 11 22 33 44 55 66 77 88 99 AA BB CC DD EE FF
    ; Result byte[i] = src[mask[i] & 0xF]
    ; byte0 = src[3] = 0x33
    ; byte1 = src[2] = 0x22
    ; byte2 = src[1] = 0x11
    ; byte3 = src[0] = 0x00
    ; byte4 = src[7] = 0x77
    ; byte5 = src[6] = 0x66
    ; byte6 = src[5] = 0x55
    ; byte7 = src[4] = 0x44
    ; Low 64: 0x4455667700112233
    TEST_CASE t17_name
    movdqa xmm0, [rel b_data1]
    movdqa xmm1, [rel shuf_ident]
    pshufb xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x4455667700112233

    ; ==== Test 18: INSERTPS ====
    ; insertps xmm0, xmm1, imm8
    ; imm8[7:6] = count_s (source select from xmm1)
    ; imm8[5:4] = count_d (destination select in xmm0)
    ; imm8[3:0] = zmask (zero destination dwords)
    ; Insert xmm1 dword[1] into xmm0 dword[2], no zeroing
    ; imm = 01_10_0000 = 0x60
    ; xmm0 = {0x11111111, 0x22222222, 0x33333333, 0x44444444}
    ; xmm1 = {0x55555555, 0x66666666, 0x77777777, 0x88888888}
    ; xmm0[2] = xmm1[1] = 0x66666666
    ; Result: {0x11111111, 0x22222222, 0x66666666, 0x44444444}
    ; Low 64: 0x2222222211111111
    TEST_CASE t18_name
    movdqa xmm0, [rel i_1234]
    movdqa xmm1, [rel i_5678]
    insertps xmm0, xmm1, 0x60
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x2222222211111111

    ; ==== Test 19: EXTRACTPS ====
    ; extractps r/m32, xmm, imm8 -> extract dword[imm8[1:0]] to r32
    ; xmm0 = {0x11111111, 0x22222222, 0x33333333, 0x44444444}
    ; Extract dword[2] = 0x33333333
    TEST_CASE t19_name
    movdqa xmm0, [rel i_1234]
    extractps eax, xmm0, 2
    CHECK_EQ_32 eax, 0x33333333

    ; ==== Test 20: PUNPCKLBW - interleave low bytes ====
    ; punpcklbw xmm0, xmm1: interleave bytes from low 8 bytes of each
    ; xmm0 bytes 0-7: 00 11 22 33 44 55 66 77
    ; xmm1 bytes 0-7: A0 A1 A2 A3 A4 A5 A6 A7
    ; Result: 00 A0 11 A1 22 A2 33 A3 44 A4 55 A5 66 A6 77 A7
    ; Low 64: bytes 00 A0 11 A1 22 A2 33 A3 -> 0xA333A222A111A000
    TEST_CASE t20_name
    movdqa xmm0, [rel b_data1]
    movdqa xmm1, [rel b_data2]
    punpcklbw xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xA333A222A111A000

    ; ==== Test 21: PUNPCKHBW - interleave high bytes ====
    ; punpckhbw xmm0, xmm1: interleave bytes from high 8 bytes of each
    ; xmm0 bytes 8-15: 88 99 AA BB CC DD EE FF
    ; xmm1 bytes 8-15: A8 A9 AA AB AC AD AE AF
    ; Result: 88 A8 99 A9 AA AA BB AB CC AC DD AD EE AE FF AF
    ; Low 64: 88 A8 99 A9 AA AA BB AB -> 0xABBBAAAAA999A888
    TEST_CASE t21_name
    movdqa xmm0, [rel b_data1]
    movdqa xmm1, [rel b_data2]
    punpckhbw xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xABBBAAAAA999A888

    ; ==== Test 22: PUNPCKLDQ - interleave low dwords ====
    ; punpckldq xmm0, xmm1: [src1[0], src2[0], src1[1], src2[1]]
    ; Same as unpcklps but for integer
    ; src1 = {0x11111111, 0x22222222, 0x33333333, 0x44444444}
    ; src2 = {0x55555555, 0x66666666, 0x77777777, 0x88888888}
    ; Result: {0x11111111, 0x55555555, 0x22222222, 0x66666666}
    ; Low 64: 0x5555555511111111
    TEST_CASE t22_name
    movdqa xmm0, [rel i_1234]
    movdqa xmm1, [rel i_5678]
    punpckldq xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x5555555511111111

    ; ==== Test 23: PUNPCKLQDQ - interleave low qwords ====
    ; punpcklqdq xmm0, xmm1: [src1_low64, src2_low64]
    ; src1 low64 = 0x2222222211111111, src2 low64 = 0x6666666655555555
    ; Low 64 of result: 0x2222222211111111
    TEST_CASE t23_name
    movdqa xmm0, [rel i_1234]
    movdqa xmm1, [rel i_5678]
    punpcklqdq xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x2222222211111111

    ; ==== Test 24: PUNPCKHQDQ - interleave high qwords ====
    ; punpckhqdq xmm0, xmm1: [src1_high64, src2_high64]
    ; src1 high64 = 0x4444444433333333, src2 high64 = 0x8888888877777777
    ; Low 64 of result: 0x4444444433333333
    TEST_CASE t24_name
    movdqa xmm0, [rel i_1234]
    movdqa xmm1, [rel i_5678]
    punpckhqdq xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x4444444433333333

    ; ==== Test 25: PALIGNR shift=4 ====
    ; palignr xmm0, xmm1, imm8
    ; Concatenate [xmm0 : xmm1] (32 bytes), shift right by imm8 bytes, take low 16 bytes
    ; xmm0 = i_1234 = {0x11111111, 0x22222222, 0x33333333, 0x44444444}
    ; xmm1 = i_5678 = {0x55555555, 0x66666666, 0x77777777, 0x88888888}
    ; Concat (high to low): xmm0_hi...xmm0_lo xmm1_hi...xmm1_lo
    ; = 44444444 33333333 22222222 11111111 | 88888888 77777777 66666666 55555555
    ; Shift right by 4 bytes: take bytes [4..19]
    ; Byte 4 of xmm1 starts at 0x66666666
    ; Result bytes [4..19] of concat: 66666666 77777777 88888888 11111111
    ; Wait: bytes 0-3 = 55555555 (xmm1 dw0), bytes 4-7 = 66666666 (xmm1 dw1)...
    ; Shift right by 4: take bytes 4..19 = xmm1 bytes 4..15 + xmm0 bytes 0..3
    ; = 66666666 77777777 88888888 11111111
    ; Low 64: 0x7777777766666666
    TEST_CASE t25_name
    movdqa xmm0, [rel i_1234]
    movdqa xmm1, [rel i_5678]
    palignr xmm0, xmm1, 4
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x7777777766666666

    ; ==== Test 26: PALIGNR shift=8 ====
    ; Shift right by 8 bytes -> take bytes 8..23
    ; = xmm1 bytes 8..15 + xmm0 bytes 0..7
    ; = 77777777 88888888 + 11111111 22222222 -> dwords
    ; Wait: bytes 8-15 of xmm1 = high qword = 0x8888888877777777
    ; bytes 0-7 of xmm0 = low qword = 0x2222222211111111
    ; Result low 64: 0x8888888877777777
    TEST_CASE t26_name
    movdqa xmm0, [rel i_1234]
    movdqa xmm1, [rel i_5678]
    palignr xmm0, xmm1, 8
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x8888888877777777

    ; ==== Test 27: PSHUFD broadcast dword2 ====
    ; imm=0xAA=10_10_10_10: [src[2], src[2], src[2], src[2]]
    ; src = {0x11111111, 0x22222222, 0x33333333, 0x44444444}
    ; Result: all 0x33333333
    TEST_CASE t27_name
    movdqa xmm1, [rel i_1234]
    pshufd xmm0, xmm1, 0xAA
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x3333333333333333

    ; ==== Test 28: SHUFPS cross - src1 dwords 2,3 and src2 dwords 0,1 ====
    ; imm=0x4E=01_00_11_10: [src1[2], src1[3], src2[0], src2[1]]
    ; Result: {3.0, 4.0, 5.0, 6.0}
    ; Low 64: dw0=0x40400000(3.0), dw1=0x40800000(4.0) -> 0x4080000040400000
    TEST_CASE t28_name
    movdqa xmm0, [rel f_1234]
    movdqa xmm1, [rel f_5678]
    shufps xmm0, xmm1, 0x4E
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x4080000040400000

    ; ==== Test 29: PUNPCKLWD - interleave low words ====
    ; w_data1 low 4 words: 0x1111, 0x2222, 0x3333, 0x4444
    ; w_data2 low 4 words: 0xAAAA, 0xBBBB, 0xCCCC, 0xDDDD
    ; Result: 0x1111, 0xAAAA, 0x2222, 0xBBBB, 0x3333, 0xCCCC, 0x4444, 0xDDDD
    ; Low 64: 0x1111 0xAAAA 0x2222 0xBBBB -> 0xBBBB2222AAAA1111
    TEST_CASE t29_name
    movdqa xmm0, [rel w_data1]
    movdqa xmm1, [rel w_data2]
    punpcklwd xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xBBBB2222AAAA1111

    ; ==== Test 30: PUNPCKHWD - interleave high words ====
    ; w_data1 high 4 words: 0x5555, 0x6666, 0x7777, 0x8888
    ; w_data2 high 4 words: 0xEEEE, 0xFFFF, 0x9999, 0x0000
    ; Result: 0x5555, 0xEEEE, 0x6666, 0xFFFF, 0x7777, 0x9999, 0x8888, 0x0000
    ; Low 64: 0xFFFF6666EEEE5555
    TEST_CASE t30_name
    movdqa xmm0, [rel w_data1]
    movdqa xmm1, [rel w_data2]
    punpckhwd xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xFFFF6666EEEE5555

    ; ==== Test 31: PSHUFB reverse all bytes ====
    ; shuf_rev: {15,14,...,1,0}
    ; b_data1: 00 11 22 33 44 55 66 77 88 99 AA BB CC DD EE FF
    ; Reversed: FF EE DD CC BB AA 99 88 77 66 55 44 33 22 11 00
    ; Low 64: 77 66 55 44 33 22 11 00 -> 0x7766554433221100
    TEST_CASE t31_name
    movdqa xmm0, [rel b_data1]
    movdqa xmm1, [rel shuf_rev]
    pshufb xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x7766554433221100

    ; ==== Test 32: SHUFPD imm=0x1 - swap low/high between sources ====
    ; imm=0x1=01: [src1[1], src2[0]]
    ; src1={1.0, 2.0}, src2={3.0, 4.0}
    ; Result: {2.0, 3.0}
    ; Low 64: 2.0 = 0x4000000000000000
    TEST_CASE t32_name
    movdqa xmm0, [rel d_12]
    movdqa xmm1, [rel d_34]
    shufpd xmm0, xmm1, 0x1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x4000000000000000

    ; ==== Test 33: PALIGNR shift=1 (byte-level shift) ====
    ; xmm0 = b_data1 = 00 11 22 33 44 55 66 77 88 99 AA BB CC DD EE FF
    ; xmm1 = b_data2 = A0 A1 A2 A3 A4 A5 A6 A7 A8 A9 AA AB AC AD AE AF
    ; Concat: [xmm0 : xmm1], shift right 1 byte
    ; Byte 0 of result = byte 1 of xmm1 = A1
    ; Byte 1 = byte 2 of xmm1 = A2
    ; ...
    ; Byte 14 = byte 15 of xmm1 = AF
    ; Byte 15 = byte 0 of xmm0 = 00
    ; Low 64 = bytes 0-7 of result: A1 A2 A3 A4 A5 A6 A7 A8
    ; = 0xA8A7A6A5A4A3A2A1
    TEST_CASE t33_name
    movdqa xmm0, [rel b_data1]
    movdqa xmm1, [rel b_data2]
    palignr xmm0, xmm1, 1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xA8A7A6A5A4A3A2A1

    ; ==== Test 34: INSERTPS with zero mask ====
    ; imm = 00_00_1111 = 0x0F: count_s=0, count_d=0, zmask=0xF (zero ALL dwords)
    ; Result: all zeros regardless of input
    TEST_CASE t34_name
    movdqa xmm0, [rel i_1234]
    movdqa xmm1, [rel i_5678]
    insertps xmm0, xmm1, 0x0F
    movq rax, xmm0
    CHECK_EQ_64 rax, 0

    ; ==== Test 35: UNPCKLPS with self (duplicate low dwords) ====
    ; unpcklps xmm0, xmm0: [dw0, dw0, dw1, dw1]
    ; src = {0x11111111, 0x22222222, 0x33333333, 0x44444444}
    ; Result: {0x11111111, 0x11111111, 0x22222222, 0x22222222}
    ; Low 64: 0x1111111111111111
    TEST_CASE t35_name
    movdqa xmm0, [rel i_1234]
    unpcklps xmm0, xmm0
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x1111111111111111

    END_TESTS
