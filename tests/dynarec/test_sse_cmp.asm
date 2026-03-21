; test_sse_cmp.asm - Test SSE comparison instructions
; CMPSS, CMPSD, CMPPS, CMPPD with all 8 imm8 predicates:
;   0 = EQ, 1 = LT, 2 = LE, 3 = UNORD, 4 = NEQ, 5 = NLT, 6 = NLE, 7 = ORD
; Also tests MINSS/MINSD/MAXSS/MAXSD, MINPS/MINPD/MAXPS/MAXPD
%include "test_framework.inc"

section .data
    t1_name:  db "cmpss EQ (2.0==2.0)", 0
    t2_name:  db "cmpss LT (1.0<2.0)", 0
    t3_name:  db "cmpss LE (2.0<=2.0)", 0
    t4_name:  db "cmpss UNORD (NaN,1.0)", 0
    t5_name:  db "cmpss NEQ (1.0!=2.0)", 0
    t6_name:  db "cmpss NLT (2.0>=1.0)", 0
    t7_name:  db "cmpss NLE (2.0>1.0)", 0
    t8_name:  db "cmpss ORD (1.0,2.0)", 0
    t9_name:  db "cmpsd EQ (3.0==3.0)", 0
    t10_name: db "cmpsd LT (1.5<2.5)", 0
    t11_name: db "cmpsd NEQ NaN", 0
    t12_name: db "cmpsd UNORD (1.0,NaN)", 0
    t13_name: db "cmpps EQ packed", 0
    t14_name: db "cmpps LT packed", 0
    t15_name: db "cmppd LE packed", 0
    t16_name: db "cmppd NEQ packed", 0
    t17_name: db "cmpss EQ false (1!=2)", 0
    t18_name: db "cmpss LT false (2>1)", 0
    t19_name: db "cmpsd ORD (NaN,NaN)", 0
    t20_name: db "cmpsd NLE (3.0>2.0)", 0
    t21_name: db "minss basic", 0
    t22_name: db "maxss basic", 0
    t23_name: db "minsd basic", 0
    t24_name: db "maxsd basic", 0
    t25_name: db "minps packed", 0
    t26_name: db "maxps packed", 0
    t27_name: db "minpd packed", 0
    t28_name: db "maxpd packed", 0
    t29_name: db "cmpps NLE packed", 0
    t30_name: db "cmppd UNORD packed", 0
    t31_name: db "cmpss LE NaN", 0
    t32_name: db "cmpsd NLT equal", 0
    t33_name: db "cmpps ORD with NaN", 0
    t34_name: db "cmppd LT packed", 0
    t35_name: db "minss NaN handling", 0

    align 16
    ; Float constants (single precision)
    f_1_0:     dd 1.0, 0.0, 0.0, 0.0
    f_2_0:     dd 2.0, 0.0, 0.0, 0.0
    f_3_0:     dd 3.0, 0.0, 0.0, 0.0
    f_nan_s:   dd 0x7FC00000, 0, 0, 0        ; quiet NaN (single)
    ; Packed floats
    f_pack1:   dd 1.0, 2.0, 3.0, 4.0
    f_pack2:   dd 1.0, 3.0, 2.0, 4.0         ; eq, lt, gt, eq (relative to pack1)
    f_pack3:   dd 1.0, 5.0, 0.5, 4.0
    f_pack_nan: dd 1.0, 0x7FC00000, 3.0, 0x7FC00000  ; mixed NaN

    ; Double constants
    d_1_5:     dq 1.5, 0.0
    d_2_5:     dq 2.5, 0.0
    d_3_0:     dq 3.0, 0.0
    d_nan_d:   dq 0x7FF8000000000000, 0      ; quiet NaN (double)
    ; Packed doubles
    d_pack1:   dq 1.0, 2.0
    d_pack2:   dq 1.0, 3.0                    ; eq, lt (relative to pack1)
    d_pack3:   dq 2.0, 1.0                    ; gt, lt (relative to pack1)
    d_pack_nan: dq 1.0, 0x7FF8000000000000    ; [1.0, NaN]

    ; Expected result constants
    all_ones_32: dd 0xFFFFFFFF, 0, 0, 0
    all_zero_32: dd 0, 0, 0, 0

section .text
global _start

_start:
    INIT_TESTS

    ; ==== Test 1: CMPSS EQ - 2.0 == 2.0 -> true (0xFFFFFFFF) ====
    TEST_CASE t1_name
    movss xmm0, [rel f_2_0]
    movss xmm1, [rel f_2_0]
    cmpss xmm0, xmm1, 0          ; EQ
    movd eax, xmm0
    CHECK_EQ_32 eax, 0xFFFFFFFF

    ; ==== Test 2: CMPSS LT - 1.0 < 2.0 -> true ====
    TEST_CASE t2_name
    movss xmm0, [rel f_1_0]
    movss xmm1, [rel f_2_0]
    cmpss xmm0, xmm1, 1          ; LT
    movd eax, xmm0
    CHECK_EQ_32 eax, 0xFFFFFFFF

    ; ==== Test 3: CMPSS LE - 2.0 <= 2.0 -> true ====
    TEST_CASE t3_name
    movss xmm0, [rel f_2_0]
    movss xmm1, [rel f_2_0]
    cmpss xmm0, xmm1, 2          ; LE
    movd eax, xmm0
    CHECK_EQ_32 eax, 0xFFFFFFFF

    ; ==== Test 4: CMPSS UNORD - NaN, 1.0 -> true (unordered) ====
    TEST_CASE t4_name
    movss xmm0, [rel f_nan_s]
    movss xmm1, [rel f_1_0]
    cmpss xmm0, xmm1, 3          ; UNORD
    movd eax, xmm0
    CHECK_EQ_32 eax, 0xFFFFFFFF

    ; ==== Test 5: CMPSS NEQ - 1.0 != 2.0 -> true ====
    TEST_CASE t5_name
    movss xmm0, [rel f_1_0]
    movss xmm1, [rel f_2_0]
    cmpss xmm0, xmm1, 4          ; NEQ
    movd eax, xmm0
    CHECK_EQ_32 eax, 0xFFFFFFFF

    ; ==== Test 6: CMPSS NLT - 2.0 >= 1.0 -> true (not less than) ====
    TEST_CASE t6_name
    movss xmm0, [rel f_2_0]
    movss xmm1, [rel f_1_0]
    cmpss xmm0, xmm1, 5          ; NLT
    movd eax, xmm0
    CHECK_EQ_32 eax, 0xFFFFFFFF

    ; ==== Test 7: CMPSS NLE - 2.0 > 1.0 -> true (not less or equal) ====
    TEST_CASE t7_name
    movss xmm0, [rel f_2_0]
    movss xmm1, [rel f_1_0]
    cmpss xmm0, xmm1, 6          ; NLE
    movd eax, xmm0
    CHECK_EQ_32 eax, 0xFFFFFFFF

    ; ==== Test 8: CMPSS ORD - 1.0, 2.0 -> true (both ordered/non-NaN) ====
    TEST_CASE t8_name
    movss xmm0, [rel f_1_0]
    movss xmm1, [rel f_2_0]
    cmpss xmm0, xmm1, 7          ; ORD
    movd eax, xmm0
    CHECK_EQ_32 eax, 0xFFFFFFFF

    ; ==== Test 9: CMPSD EQ - 3.0 == 3.0 -> true (0xFFFFFFFFFFFFFFFF) ====
    TEST_CASE t9_name
    movsd xmm0, [rel d_3_0]
    movsd xmm1, [rel d_3_0]
    cmpsd xmm0, xmm1, 0          ; EQ
    movq rax, xmm0
    mov rbx, 0xFFFFFFFFFFFFFFFF
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFF

    ; ==== Test 10: CMPSD LT - 1.5 < 2.5 -> true ====
    TEST_CASE t10_name
    movsd xmm0, [rel d_1_5]
    movsd xmm1, [rel d_2_5]
    cmpsd xmm0, xmm1, 1          ; LT
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFF

    ; ==== Test 11: CMPSD NEQ with NaN - NaN != 3.0 -> true (NaN != anything is true) ====
    TEST_CASE t11_name
    movsd xmm0, [rel d_nan_d]
    movsd xmm1, [rel d_3_0]
    cmpsd xmm0, xmm1, 4          ; NEQ
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFF

    ; ==== Test 12: CMPSD UNORD - 1.0 vs NaN -> true (unordered) ====
    TEST_CASE t12_name
    movsd xmm0, [rel d_1_5]
    movsd xmm1, [rel d_nan_d]
    cmpsd xmm0, xmm1, 3          ; UNORD
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFF

    ; ==== Test 13: CMPPS EQ packed ====
    ; pack1 = {1.0, 2.0, 3.0, 4.0}
    ; pack2 = {1.0, 3.0, 2.0, 4.0}
    ; EQ: {T, F, F, T} -> {0xFFFFFFFF, 0, 0, 0xFFFFFFFF}
    ; Low 64 bits: dword0=0xFFFFFFFF, dword1=0x00000000 -> 0x00000000FFFFFFFF
    ; High 64 bits: dword2=0x00000000, dword3=0xFFFFFFFF -> 0xFFFFFFFF00000000
    TEST_CASE t13_name
    movdqa xmm0, [rel f_pack1]
    movdqa xmm1, [rel f_pack2]
    cmpps xmm0, xmm1, 0          ; EQ
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x00000000FFFFFFFF

    ; ==== Test 14: CMPPS LT packed ====
    ; pack1 = {1.0, 2.0, 3.0, 4.0}
    ; pack2 = {1.0, 3.0, 2.0, 4.0}
    ; LT: {F, T, F, F} -> {0, 0xFFFFFFFF, 0, 0}
    ; Low 64: 0xFFFFFFFF00000000
    TEST_CASE t14_name
    movdqa xmm0, [rel f_pack1]
    movdqa xmm1, [rel f_pack2]
    cmpps xmm0, xmm1, 1          ; LT
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xFFFFFFFF00000000

    ; ==== Test 15: CMPPD LE packed ====
    ; pack1 = {1.0, 2.0}
    ; pack2 = {1.0, 3.0}
    ; LE: {T, T} -> {0xFF..FF, 0xFF..FF}
    TEST_CASE t15_name
    movdqa xmm0, [rel d_pack1]
    movdqa xmm1, [rel d_pack2]
    cmppd xmm0, xmm1, 2          ; LE
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFF

    ; ==== Test 16: CMPPD NEQ packed ====
    ; pack1 = {1.0, 2.0}, pack2 = {1.0, 3.0}
    ; NEQ: {F, T} -> {0, 0xFF..FF}
    ; Low 64 = 0x0000000000000000
    TEST_CASE t16_name
    movdqa xmm0, [rel d_pack1]
    movdqa xmm1, [rel d_pack2]
    cmppd xmm0, xmm1, 4          ; NEQ
    movq rax, xmm0
    CHECK_EQ_64 rax, 0

    ; ==== Test 17: CMPSS EQ false - 1.0 != 2.0 -> false (0x00000000) ====
    TEST_CASE t17_name
    movss xmm0, [rel f_1_0]
    movss xmm1, [rel f_2_0]
    cmpss xmm0, xmm1, 0          ; EQ
    movd eax, xmm0
    CHECK_EQ_32 eax, 0

    ; ==== Test 18: CMPSS LT false - 2.0 < 1.0 -> false ====
    TEST_CASE t18_name
    movss xmm0, [rel f_2_0]
    movss xmm1, [rel f_1_0]
    cmpss xmm0, xmm1, 1          ; LT
    movd eax, xmm0
    CHECK_EQ_32 eax, 0

    ; ==== Test 19: CMPSD ORD with NaN,NaN -> false (both unordered) ====
    TEST_CASE t19_name
    movsd xmm0, [rel d_nan_d]
    movsd xmm1, [rel d_nan_d]
    cmpsd xmm0, xmm1, 7          ; ORD
    movq rax, xmm0
    CHECK_EQ_64 rax, 0

    ; ==== Test 20: CMPSD NLE - 3.0 > 2.5 -> true ====
    TEST_CASE t20_name
    movsd xmm0, [rel d_3_0]
    movsd xmm1, [rel d_2_5]
    cmpsd xmm0, xmm1, 6          ; NLE
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFF

    ; ==== Test 21: MINSS - min(2.0, 1.0) = 1.0 ====
    TEST_CASE t21_name
    movss xmm0, [rel f_2_0]
    movss xmm1, [rel f_1_0]
    minss xmm0, xmm1
    movd eax, xmm0
    ; 1.0f = 0x3F800000
    CHECK_EQ_32 eax, 0x3F800000

    ; ==== Test 22: MAXSS - max(1.0, 2.0) = 2.0 ====
    TEST_CASE t22_name
    movss xmm0, [rel f_1_0]
    movss xmm1, [rel f_2_0]
    maxss xmm0, xmm1
    movd eax, xmm0
    ; 2.0f = 0x40000000
    CHECK_EQ_32 eax, 0x40000000

    ; ==== Test 23: MINSD - min(2.5, 1.5) = 1.5 ====
    TEST_CASE t23_name
    movsd xmm0, [rel d_2_5]
    movsd xmm1, [rel d_1_5]
    minsd xmm0, xmm1
    movq rax, xmm0
    ; 1.5 = 0x3FF8000000000000
    CHECK_EQ_64 rax, 0x3FF8000000000000

    ; ==== Test 24: MAXSD - max(1.5, 2.5) = 2.5 ====
    TEST_CASE t24_name
    movsd xmm0, [rel d_1_5]
    movsd xmm1, [rel d_2_5]
    maxsd xmm0, xmm1
    movq rax, xmm0
    ; 2.5 = 0x4004000000000000
    CHECK_EQ_64 rax, 0x4004000000000000

    ; ==== Test 25: MINPS packed ====
    ; pack1 = {1.0, 2.0, 3.0, 4.0}
    ; pack3 = {1.0, 5.0, 0.5, 4.0}
    ; min: {1.0, 2.0, 0.5, 4.0}
    ; Low 64: dword0=1.0=0x3F800000, dword1=2.0=0x40000000 -> 0x400000003F800000
    TEST_CASE t25_name
    movdqa xmm0, [rel f_pack1]
    movdqa xmm1, [rel f_pack3]
    minps xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x400000003F800000

    ; ==== Test 26: MAXPS packed ====
    ; pack1 = {1.0, 2.0, 3.0, 4.0}
    ; pack3 = {1.0, 5.0, 0.5, 4.0}
    ; max: {1.0, 5.0, 3.0, 4.0}
    ; Low 64: dword0=1.0=0x3F800000, dword1=5.0=0x40A00000 -> 0x40A000003F800000
    TEST_CASE t26_name
    movdqa xmm0, [rel f_pack1]
    movdqa xmm1, [rel f_pack3]
    maxps xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x40A000003F800000

    ; ==== Test 27: MINPD packed ====
    ; pack1 = {1.0, 2.0}, pack3 = {2.0, 1.0}
    ; min: {1.0, 1.0}
    ; Low 64: 1.0 = 0x3FF0000000000000
    TEST_CASE t27_name
    movdqa xmm0, [rel d_pack1]
    movdqa xmm1, [rel d_pack3]
    minpd xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x3FF0000000000000

    ; ==== Test 28: MAXPD packed ====
    ; pack1 = {1.0, 2.0}, pack3 = {2.0, 1.0}
    ; max: {2.0, 2.0}
    ; Low 64: 2.0 = 0x4000000000000000
    TEST_CASE t28_name
    movdqa xmm0, [rel d_pack1]
    movdqa xmm1, [rel d_pack3]
    maxpd xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x4000000000000000

    ; ==== Test 29: CMPPS NLE packed ====
    ; pack1 = {1.0, 2.0, 3.0, 4.0}
    ; pack2 = {1.0, 3.0, 2.0, 4.0}
    ; NLE (>): {F, F, T, F} -> {0, 0, 0xFFFFFFFF, 0}
    ; Low 64: 0x0000000000000000
    TEST_CASE t29_name
    movdqa xmm0, [rel f_pack1]
    movdqa xmm1, [rel f_pack2]
    cmpps xmm0, xmm1, 6          ; NLE
    ; Check high 64 bits: dword2=0xFFFFFFFF, dword3=0 -> 0x00000000FFFFFFFF
    ; Need to extract high 64. Use pshufd or movhlps.
    movhlps xmm1, xmm0           ; xmm1 low = xmm0 high
    movq rax, xmm1
    CHECK_EQ_64 rax, 0x00000000FFFFFFFF

    ; ==== Test 30: CMPPD UNORD packed ====
    ; pack_nan = {1.0, NaN}
    ; pack1 = {1.0, 2.0}
    ; UNORD: {F, T} -> {0, 0xFF..FF}
    TEST_CASE t30_name
    movdqa xmm0, [rel d_pack_nan]
    movdqa xmm1, [rel d_pack1]
    cmppd xmm0, xmm1, 3          ; UNORD
    movq rax, xmm0
    ; Low qword: 1.0 vs 1.0 -> both ordered -> false -> 0
    CHECK_EQ_64 rax, 0

    ; ==== Test 31: CMPSS LE with NaN -> false (NaN comparisons are always false for LT/LE/EQ) ====
    TEST_CASE t31_name
    movss xmm0, [rel f_nan_s]
    movss xmm1, [rel f_1_0]
    cmpss xmm0, xmm1, 2          ; LE
    movd eax, xmm0
    CHECK_EQ_32 eax, 0

    ; ==== Test 32: CMPSD NLT with equal values -> true (not less than, i.e. >=) ====
    TEST_CASE t32_name
    movsd xmm0, [rel d_3_0]
    movsd xmm1, [rel d_3_0]
    cmpsd xmm0, xmm1, 5          ; NLT (>=)
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFF

    ; ==== Test 33: CMPPS ORD with NaN in some lanes ====
    ; pack_nan = {1.0, NaN, 3.0, NaN}
    ; pack1    = {1.0, 2.0, 3.0, 4.0}
    ; ORD: {T, F, T, F} -> {0xFFFFFFFF, 0, 0xFFFFFFFF, 0}
    ; Low 64: dword0=0xFFFFFFFF, dword1=0 -> 0x00000000FFFFFFFF
    TEST_CASE t33_name
    movdqa xmm0, [rel f_pack_nan]
    movdqa xmm1, [rel f_pack1]
    cmpps xmm0, xmm1, 7          ; ORD
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x00000000FFFFFFFF

    ; ==== Test 34: CMPPD LT packed ====
    ; pack1 = {1.0, 2.0}, pack2 = {1.0, 3.0}
    ; LT: {F, T} -> {0, 0xFF..FF}
    ; Low 64 = 0
    TEST_CASE t34_name
    movdqa xmm0, [rel d_pack1]
    movdqa xmm1, [rel d_pack2]
    cmppd xmm0, xmm1, 1          ; LT
    movq rax, xmm0
    CHECK_EQ_64 rax, 0

    ; ==== Test 35: MINSS NaN handling ====
    ; Per x86 spec: if either operand is NaN, return the second operand (src2)
    ; minss xmm0, xmm1 -> if xmm0 is NaN, result is xmm1's value
    TEST_CASE t35_name
    movss xmm0, [rel f_nan_s]
    movss xmm1, [rel f_2_0]
    minss xmm0, xmm1
    movd eax, xmm0
    ; NaN vs 2.0 -> returns second operand = 2.0 = 0x40000000
    CHECK_EQ_32 eax, 0x40000000

    END_TESTS
