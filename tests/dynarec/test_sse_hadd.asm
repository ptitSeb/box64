; test_sse_hadd.asm - Test SSE3/SSSE3 horizontal add/sub operations
; HADDPS, HADDPD, HSUBPS, HSUBPD (SSE3)
; ADDSUBPS, ADDSUBPD (SSE3)
; PHADDW, PHADDD, PHADDSW (SSSE3)
; PHSUBW, PHSUBD, PHSUBSW (SSSE3)
%include "test_framework.inc"

section .data
    t1_name:  db "haddps basic", 0
    t2_name:  db "haddpd basic", 0
    t3_name:  db "hsubps basic", 0
    t4_name:  db "hsubpd basic", 0
    t5_name:  db "addsubps basic", 0
    t6_name:  db "addsubpd basic", 0
    t7_name:  db "phaddw basic", 0
    t8_name:  db "phaddd basic", 0
    t9_name:  db "phsubw basic", 0
    t10_name: db "phsubd basic", 0
    t11_name: db "phaddsw basic", 0
    t12_name: db "phsubsw basic", 0
    t13_name: db "haddps self", 0
    t14_name: db "haddpd self", 0
    t15_name: db "phaddw saturate", 0
    t16_name: db "hsubps negative", 0
    t17_name: db "addsubps mixed", 0
    t18_name: db "phaddd large", 0
    t19_name: db "phsubsw saturate", 0
    t20_name: db "haddps zeros", 0
    t21_name: db "addsubpd negative", 0
    t22_name: db "phaddw self", 0
    t23_name: db "hsubpd self", 0
    t24_name: db "haddps high check", 0
    t25_name: db "phaddd self", 0

    align 16
    ; Float packs
    f_1234:   dd 1.0, 2.0, 3.0, 4.0
    f_5678:   dd 5.0, 6.0, 7.0, 8.0
    f_zeros:  dd 0.0, 0.0, 0.0, 0.0
    f_neg:    dd -1.0, 3.0, -2.0, 5.0

    ; Double packs
    d_12:     dq 1.0, 2.0
    d_34:     dq 3.0, 4.0
    d_n1_2:   dq -1.0, 2.0

    ; Word data for PHADDW/PHSUBW
    w_1234:   dw 10, 20, 30, 40, 50, 60, 70, 80
    w_5678:   dw 100, 200, 300, 400, 500, 600, 700, 800
    ; For saturation test
    w_sat:    dw 0x7F00, 0x0100, 0x7FFF, 0x0001, 0x0010, 0x0020, 0x0030, 0x0040
    w_nsat:   dw 0x8000, 0x8000, 0x7FFF, 0x7FFF, 0x0001, 0xFFFF, 0x0002, 0xFFFE

    ; Dword data for PHADDD/PHSUBD
    d_i1234:  dd 100, 200, 300, 400
    d_i5678:  dd 500, 600, 700, 800
    d_ilarge: dd 0x7FFFFFF0, 0x00000010, 0x40000000, 0x40000000

section .text
global _start

_start:
    INIT_TESTS

    ; ==== Test 1: HADDPS basic ====
    ; haddps xmm0, xmm1
    ; dst[0] = src1[0]+src1[1], dst[1] = src1[2]+src1[3]
    ; dst[2] = src2[0]+src2[1], dst[3] = src2[2]+src2[3]
    ; src1={1,2,3,4}, src2={5,6,7,8}
    ; Result: {3, 7, 11, 15}
    ; 3.0f=0x40400000, 7.0f=0x40E00000
    TEST_CASE t1_name
    movdqa xmm0, [rel f_1234]
    movdqa xmm1, [rel f_5678]
    haddps xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x40E0000040400000

    ; ==== Test 2: HADDPD basic ====
    ; haddpd xmm0, xmm1
    ; dst[0] = src1[0]+src1[1], dst[1] = src2[0]+src2[1]
    ; src1={1.0, 2.0}, src2={3.0, 4.0}
    ; Result: {3.0, 7.0}
    TEST_CASE t2_name
    movdqa xmm0, [rel d_12]
    movdqa xmm1, [rel d_34]
    haddpd xmm0, xmm1
    movq rax, xmm0
    ; 3.0 = 0x4008000000000000
    CHECK_EQ_64 rax, 0x4008000000000000

    ; ==== Test 3: HSUBPS basic ====
    ; dst[0] = src1[0]-src1[1], dst[1] = src1[2]-src1[3]
    ; src1={1,2,3,4}, src2={5,6,7,8}
    ; Result: {-1, -1, -1, -1}
    ; -1.0f = 0xBF800000
    TEST_CASE t3_name
    movdqa xmm0, [rel f_1234]
    movdqa xmm1, [rel f_5678]
    hsubps xmm0, xmm1
    movd eax, xmm0
    CHECK_EQ_32 eax, 0xBF800000

    ; ==== Test 4: HSUBPD basic ====
    ; dst[0] = src1[0]-src1[1], dst[1] = src2[0]-src2[1]
    ; src1={1.0, 2.0}, src2={3.0, 4.0}
    ; Result: {-1.0, -1.0}
    TEST_CASE t4_name
    movdqa xmm0, [rel d_12]
    movdqa xmm1, [rel d_34]
    hsubpd xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xBFF0000000000000  ; -1.0

    ; ==== Test 5: ADDSUBPS basic ====
    ; addsubps: odd dwords ADD, even dwords SUB
    ; dst[0] = src1[0]-src2[0], dst[1] = src1[1]+src2[1]
    ; dst[2] = src1[2]-src2[2], dst[3] = src1[3]+src2[3]
    ; src1={1,2,3,4}, src2={5,6,7,8}
    ; Result: {1-5, 2+6, 3-7, 4+8} = {-4, 8, -4, 12}
    ; -4.0f=0xC0800000, 8.0f=0x41000000
    TEST_CASE t5_name
    movdqa xmm0, [rel f_1234]
    movdqa xmm1, [rel f_5678]
    addsubps xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x41000000C0800000

    ; ==== Test 6: ADDSUBPD basic ====
    ; dst[0] = src1[0]-src2[0], dst[1] = src1[1]+src2[1]
    ; src1={1.0, 2.0}, src2={3.0, 4.0}
    ; Result: {-2.0, 6.0}
    TEST_CASE t6_name
    movdqa xmm0, [rel d_12]
    movdqa xmm1, [rel d_34]
    addsubpd xmm0, xmm1
    movq rax, xmm0
    ; -2.0 = 0xC000000000000000
    CHECK_EQ_64 rax, 0xC000000000000000

    ; ==== Test 7: PHADDW basic ====
    ; Horizontal add adjacent pairs of 16-bit integers
    ; dst[0..3] = src1 pairs added, dst[4..7] = src2 pairs added
    ; src1: {10,20, 30,40, 50,60, 70,80}
    ; src2: {100,200, 300,400, 500,600, 700,800}
    ; Result: {30, 70, 110, 150, 300, 700, 1100, 1500}
    ; Low 64: words 30, 70, 110, 150
    ; = 0x0096006E0046001E
    TEST_CASE t7_name
    movdqa xmm0, [rel w_1234]
    movdqa xmm1, [rel w_5678]
    phaddw xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0096006E0046001E

    ; ==== Test 8: PHADDD basic ====
    ; Horizontal add adjacent pairs of 32-bit integers
    ; src1: {100, 200, 300, 400} -> {300, 700}
    ; src2: {500, 600, 700, 800} -> {1100, 1500}
    ; Result: {300, 700, 1100, 1500}
    ; Low 64: dw0=300=0x12C, dw1=700=0x2BC -> 0x000002BC0000012C
    TEST_CASE t8_name
    movdqa xmm0, [rel d_i1234]
    movdqa xmm1, [rel d_i5678]
    phaddd xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x000002BC0000012C

    ; ==== Test 9: PHSUBW basic ====
    ; Horizontal sub adjacent pairs: a[0]-a[1], a[2]-a[3], ...
    ; src1: {10,20, 30,40, 50,60, 70,80} -> {-10, -10, -10, -10}
    ; src2: {100,200, 300,400, 500,600, 700,800} -> {-100, -100, -100, -100}
    ; Low 64: words -10, -10, -10, -10 = 0xFFF6, 0xFFF6, 0xFFF6, 0xFFF6
    ; = 0xFFF6FFF6FFF6FFF6
    TEST_CASE t9_name
    movdqa xmm0, [rel w_1234]
    movdqa xmm1, [rel w_5678]
    phsubw xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xFFF6FFF6FFF6FFF6

    ; ==== Test 10: PHSUBD basic ====
    ; src1: {100, 200, 300, 400} -> {-100, -100}
    ; src2: {500, 600, 700, 800} -> {-100, -100}
    ; Low 64: dw0=-100=0xFFFFFF9C, dw1=-100=0xFFFFFF9C
    TEST_CASE t10_name
    movdqa xmm0, [rel d_i1234]
    movdqa xmm1, [rel d_i5678]
    phsubd xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xFFFFFF9CFFFFFF9C

    ; ==== Test 11: PHADDSW basic (saturating horizontal add) ====
    ; src1: {10,20, 30,40, 50,60, 70,80} -> {30, 70, 110, 150} (no saturation needed)
    ; Low 64: same as PHADDW for small values
    TEST_CASE t11_name
    movdqa xmm0, [rel w_1234]
    movdqa xmm1, [rel w_5678]
    phaddsw xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0096006E0046001E

    ; ==== Test 12: PHSUBSW basic (saturating horizontal sub) ====
    ; src1: {10,20, 30,40, 50,60, 70,80} -> {-10, -10, -10, -10}
    TEST_CASE t12_name
    movdqa xmm0, [rel w_1234]
    movdqa xmm1, [rel w_5678]
    phsubsw xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xFFF6FFF6FFF6FFF6

    ; ==== Test 13: HADDPS self (xmm0, xmm0) ====
    ; src={1,2,3,4}: dst = {1+2, 3+4, 1+2, 3+4} = {3, 7, 3, 7}
    TEST_CASE t13_name
    movdqa xmm0, [rel f_1234]
    haddps xmm0, xmm0
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x40E0000040400000

    ; ==== Test 14: HADDPD self ====
    ; src={1.0, 2.0}: dst = {3.0, 3.0}
    TEST_CASE t14_name
    movdqa xmm0, [rel d_12]
    haddpd xmm0, xmm0
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x4008000000000000  ; 3.0

    ; ==== Test 15: PHADDSW saturation ====
    ; w_sat: {0x7F00, 0x0100, 0x7FFF, 0x0001, ...}
    ; pair0: 0x7F00 + 0x0100 = 0x8000 -> saturates to 0x7FFF
    ; pair1: 0x7FFF + 0x0001 = 0x8000 -> saturates to 0x7FFF
    TEST_CASE t15_name
    movdqa xmm0, [rel w_sat]
    movdqa xmm1, [rel w_1234]   ; doesn't matter much, just need something
    phaddsw xmm0, xmm1
    ; word0 = saturate(0x7F00+0x0100) = 0x7FFF
    ; word1 = saturate(0x7FFF+0x0001) = 0x7FFF
    movd eax, xmm0
    ; Low dword = word0 | (word1 << 16) = 0x7FFF7FFF
    CHECK_EQ_32 eax, 0x7FFF7FFF

    ; ==== Test 16: HSUBPS with negative ====
    ; f_neg = {-1.0, 3.0, -2.0, 5.0}
    ; dst[0] = -1.0 - 3.0 = -4.0, dst[1] = -2.0 - 5.0 = -7.0
    ; -4.0f = 0xC0800000
    TEST_CASE t16_name
    movdqa xmm0, [rel f_neg]
    movdqa xmm1, [rel f_1234]
    hsubps xmm0, xmm1
    movd eax, xmm0
    CHECK_EQ_32 eax, 0xC0800000

    ; ==== Test 17: ADDSUBPS with mixed signs ====
    ; f_neg = {-1, 3, -2, 5}, f_1234 = {1, 2, 3, 4}
    ; dst[0] = -1-1 = -2, dst[1] = 3+2 = 5, dst[2] = -2-3 = -5, dst[3] = 5+4 = 9
    ; -2.0f = 0xC0000000, 5.0f = 0x40A00000
    TEST_CASE t17_name
    movdqa xmm0, [rel f_neg]
    movdqa xmm1, [rel f_1234]
    addsubps xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x40A00000C0000000

    ; ==== Test 18: PHADDD with large values (overflow wraps) ====
    ; d_ilarge: {0x7FFFFFF0, 0x10, 0x40000000, 0x40000000}
    ; pair0: 0x7FFFFFF0 + 0x10 = 0x80000000 (wraps to negative in signed)
    ; pair1: 0x40000000 + 0x40000000 = 0x80000000
    TEST_CASE t18_name
    movdqa xmm0, [rel d_ilarge]
    movdqa xmm1, [rel d_i1234]
    phaddd xmm0, xmm1
    movd eax, xmm0
    CHECK_EQ_32 eax, 0x80000000

    ; ==== Test 19: PHSUBSW saturation ====
    ; w_nsat: {0x8000, 0x8000, 0x7FFF, 0x7FFF, ...}
    ; pair0: 0x8000 - 0x8000 = 0 (no saturation)
    ; pair1: 0x7FFF - 0x7FFF = 0
    TEST_CASE t19_name
    movdqa xmm0, [rel w_nsat]
    movdqa xmm1, [rel w_1234]
    phsubsw xmm0, xmm1
    movd eax, xmm0
    ; word0 = 0, word1 = 0 -> dword = 0
    CHECK_EQ_32 eax, 0x00000000

    ; ==== Test 20: HADDPS with zeros ====
    TEST_CASE t20_name
    movdqa xmm0, [rel f_zeros]
    movdqa xmm1, [rel f_1234]
    haddps xmm0, xmm1
    movd eax, xmm0
    ; dw0 = 0+0 = 0
    CHECK_EQ_32 eax, 0x00000000

    ; ==== Test 21: ADDSUBPD with negative ====
    ; d_n1_2 = {-1.0, 2.0}, d_34 = {3.0, 4.0}
    ; dst[0] = -1.0 - 3.0 = -4.0, dst[1] = 2.0 + 4.0 = 6.0
    ; -4.0 = 0xC010000000000000
    TEST_CASE t21_name
    movdqa xmm0, [rel d_n1_2]
    movdqa xmm1, [rel d_34]
    addsubpd xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xC010000000000000

    ; ==== Test 22: PHADDW self ====
    ; w_1234 = {10,20,30,40,50,60,70,80}
    ; Self: {30,70,110,150, 30,70,110,150}
    TEST_CASE t22_name
    movdqa xmm0, [rel w_1234]
    phaddw xmm0, xmm0
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0096006E0046001E

    ; ==== Test 23: HSUBPD self ====
    ; d_12 = {1.0, 2.0}
    ; Self: dst[0] = 1.0-2.0 = -1.0, dst[1] = 1.0-2.0 = -1.0
    TEST_CASE t23_name
    movdqa xmm0, [rel d_12]
    hsubpd xmm0, xmm0
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xBFF0000000000000  ; -1.0

    ; ==== Test 24: HADDPS check high dwords ====
    ; src1={1,2,3,4}, src2={5,6,7,8}
    ; Result: {3, 7, 11, 15}
    ; High 64: dw2=11.0=0x41300000, dw3=15.0=0x41700000 -> 0x4170000041300000
    TEST_CASE t24_name
    movdqa xmm0, [rel f_1234]
    movdqa xmm1, [rel f_5678]
    haddps xmm0, xmm1
    movhlps xmm2, xmm0
    movq rax, xmm2
    CHECK_EQ_64 rax, 0x4170000041300000

    ; ==== Test 25: PHADDD self ====
    ; d_i1234 = {100, 200, 300, 400}
    ; Self: {300, 700, 300, 700}
    TEST_CASE t25_name
    movdqa xmm0, [rel d_i1234]
    phaddd xmm0, xmm0
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x000002BC0000012C

    END_TESTS
