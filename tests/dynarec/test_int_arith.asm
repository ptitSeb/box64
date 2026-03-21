; test_int_arith.asm - Test integer arithmetic with flags
; ADD, SUB, CMP, TEST, ADC, SBB, INC, DEC, NEG, MUL, IMUL, DIV, IDIV
%include "test_framework.inc"

section .data
    t1_name:  db "add eax,ebx", 0
    t2_name:  db "add overflow", 0
    t3_name:  db "add carry", 0
    t4_name:  db "sub basic", 0
    t5_name:  db "sub borrow", 0
    t6_name:  db "cmp equal ZF", 0
    t7_name:  db "cmp less SF", 0
    t8_name:  db "test zero ZF", 0
    t9_name:  db "test nonzero", 0
    t10_name: db "adc basic", 0
    t11_name: db "adc with carry", 0
    t12_name: db "sbb basic", 0
    t13_name: db "sbb with borrow", 0
    t14_name: db "inc basic", 0
    t15_name: db "inc no CF change", 0
    t16_name: db "dec basic", 0
    t17_name: db "dec to zero ZF", 0
    t18_name: db "neg basic", 0
    t19_name: db "neg zero", 0
    t20_name: db "mul 32x32", 0
    t21_name: db "mul 64x64 low", 0
    t22_name: db "imul reg,reg", 0
    t23_name: db "imul reg,reg,imm", 0
    t24_name: db "div 32", 0
    t25_name: db "idiv 32 positive", 0
    t26_name: db "idiv 32 negative", 0
    t27_name: db "add 64-bit", 0
    t28_name: db "sub 64-bit", 0
    t29_name: db "imul 64-bit", 0
    t30_name: db "xadd basic", 0
    t31_name: db "cmpxchg succeed", 0
    t32_name: db "cmpxchg fail", 0
    t33_name: db "bswap 32", 0
    t34_name: db "bswap 64", 0
    t35_name: db "add flags PF", 0
    t36_name: db "lea basic", 0
    t37_name: db "lea sib", 0
    t38_name: db "cdq sign extend", 0
    t39_name: db "cqo sign extend", 0
    t40_name: db "movsx 8->32", 0

section .text
global _start

_start:
    INIT_TESTS

    ; ==== Test 1: add eax, ebx ====
    TEST_CASE t1_name
    mov eax, 100
    mov ebx, 200
    add eax, ebx
    CHECK_EQ_32 eax, 300

    ; ==== Test 2: add overflow ====
    TEST_CASE t2_name
    mov eax, 0x7FFFFFFF
    add eax, 1
    SAVE_FLAGS
    CHECK_FLAGS_EQ OF, OF       ; overflow set

    ; ==== Test 3: add carry ====
    TEST_CASE t3_name
    mov eax, 0xFFFFFFFF
    add eax, 1
    SAVE_FLAGS
    ; Result is 0, so ZF is set and CF is set
    CHECK_FLAGS_EQ (CF|ZF), (CF|ZF)

    ; ==== Test 4: sub basic ====
    TEST_CASE t4_name
    mov eax, 300
    mov ebx, 100
    sub eax, ebx
    CHECK_EQ_32 eax, 200

    ; ==== Test 5: sub borrow ====
    TEST_CASE t5_name
    mov eax, 0
    sub eax, 1
    SAVE_FLAGS
    ; 0 - 1 = 0xFFFFFFFF, CF set (borrow), SF set
    CHECK_FLAGS_EQ (CF|SF), (CF|SF)

    ; ==== Test 6: cmp equal -> ZF ====
    TEST_CASE t6_name
    mov eax, 42
    cmp eax, 42
    SAVE_FLAGS
    CHECK_FLAGS_EQ ZF, ZF

    ; ==== Test 7: cmp less -> SF ====
    TEST_CASE t7_name
    mov eax, 10
    cmp eax, 20
    SAVE_FLAGS
    ; 10 - 20 = negative -> SF set, CF set (borrow)
    CHECK_FLAGS_EQ (SF|CF), (SF|CF)

    ; ==== Test 8: test zero -> ZF ====
    TEST_CASE t8_name
    mov eax, 0xF0
    test eax, 0x0F
    SAVE_FLAGS
    ; 0xF0 & 0x0F = 0 -> ZF set
    CHECK_FLAGS_EQ ZF, ZF

    ; ==== Test 9: test nonzero ====
    TEST_CASE t9_name
    mov eax, 0xFF
    test eax, 0x0F
    SAVE_FLAGS
    ; 0xFF & 0x0F = 0x0F != 0 -> ZF clear
    CHECK_FLAGS_EQ ZF, 0

    ; ==== Test 10: adc basic (no carry in) ====
    TEST_CASE t10_name
    clc                     ; clear carry
    mov eax, 100
    adc eax, 200
    CHECK_EQ_32 eax, 300

    ; ==== Test 11: adc with carry in ====
    TEST_CASE t11_name
    stc                     ; set carry
    mov eax, 100
    adc eax, 200
    CHECK_EQ_32 eax, 301

    ; ==== Test 12: sbb basic (no borrow) ====
    TEST_CASE t12_name
    clc
    mov eax, 300
    sbb eax, 100
    CHECK_EQ_32 eax, 200

    ; ==== Test 13: sbb with borrow ====
    TEST_CASE t13_name
    stc
    mov eax, 300
    sbb eax, 100
    CHECK_EQ_32 eax, 199

    ; ==== Test 14: inc basic ====
    TEST_CASE t14_name
    mov eax, 41
    inc eax
    CHECK_EQ_32 eax, 42

    ; ==== Test 15: inc does not affect CF ====
    TEST_CASE t15_name
    stc                     ; set CF
    mov eax, 0
    inc eax                 ; should NOT clear CF
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ; ==== Test 16: dec basic ====
    TEST_CASE t16_name
    mov eax, 43
    dec eax
    CHECK_EQ_32 eax, 42

    ; ==== Test 17: dec to zero -> ZF ====
    TEST_CASE t17_name
    mov eax, 1
    dec eax
    SAVE_FLAGS
    CHECK_FLAGS_EQ ZF, ZF

    ; ==== Test 18: neg basic ====
    TEST_CASE t18_name
    mov eax, 42
    neg eax
    CHECK_EQ_32 eax, 0xFFFFFFD6   ; -42

    ; ==== Test 19: neg zero -> ZF, CF=0 ====
    TEST_CASE t19_name
    xor eax, eax
    neg eax
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF), ZF     ; ZF=1, CF=0

    ; ==== Test 20: mul 32x32 ====
    TEST_CASE t20_name
    mov eax, 100000
    mov ecx, 100000
    mul ecx             ; edx:eax = eax * ecx = 10,000,000,000
    ; 10000000000 = 0x2540BE400
    ; eax = 0x540BE400, edx = 0x2
    CHECK_EQ_32 eax, 0x540BE400

    ; ==== Test 21: mul 64x64 low bits ====
    TEST_CASE t21_name
    mov rax, 0x100000000      ; 2^32
    mov rcx, 3
    mul rcx                     ; rdx:rax = 3 * 2^32
    ; rax = 0x300000000, rdx = 0
    CHECK_EQ_64 rax, 0x300000000

    ; ==== Test 22: imul reg, reg ====
    TEST_CASE t22_name
    mov eax, -7
    mov ebx, 6
    imul eax, ebx
    CHECK_EQ_32 eax, 0xFFFFFFD6    ; -42

    ; ==== Test 23: imul reg, reg, imm ====
    TEST_CASE t23_name
    mov ebx, -7
    imul eax, ebx, 6
    CHECK_EQ_32 eax, 0xFFFFFFD6    ; -42

    ; ==== Test 24: div 32-bit ====
    TEST_CASE t24_name
    xor edx, edx
    mov eax, 100
    mov ecx, 7
    div ecx
    ; 100 / 7 = 14 remainder 2
    CHECK_EQ_32 eax, 14

    ; ==== Test 25: idiv 32-bit positive ====
    TEST_CASE t25_name
    xor edx, edx
    mov eax, 100
    mov ecx, 7
    idiv ecx
    CHECK_EQ_32 eax, 14

    ; ==== Test 26: idiv 32-bit negative ====
    TEST_CASE t26_name
    mov eax, -100
    cdq                     ; sign-extend eax into edx:eax
    mov ecx, 7
    idiv ecx
    ; -100 / 7 = -14 remainder -2
    CHECK_EQ_32 eax, 0xFFFFFFF2    ; -14

    ; ==== Test 27: add 64-bit ====
    TEST_CASE t27_name
    mov rax, 0x100000000
    mov rbx, 0x200000000
    add rax, rbx
    CHECK_EQ_64 rax, 0x300000000

    ; ==== Test 28: sub 64-bit ====
    TEST_CASE t28_name
    mov rax, 0x300000000
    mov rbx, 0x100000000
    sub rax, rbx
    CHECK_EQ_64 rax, 0x200000000

    ; ==== Test 29: imul 64-bit ====
    TEST_CASE t29_name
    mov rax, 0x100000000
    mov rbx, 3
    imul rax, rbx
    CHECK_EQ_64 rax, 0x300000000

    ; ==== Test 30: xadd ====
    TEST_CASE t30_name
    mov eax, 10
    mov ebx, 20
    xadd eax, ebx
    ; After xadd: eax = 10+20 = 30, ebx = old eax = 10
    ; Wait, xadd dst, src: temp=dst+src, src=old_dst, dst=temp
    ; So eax = 30, ebx = 10
    CHECK_EQ_32 eax, 30

    ; ==== Test 31: cmpxchg succeed ====
    TEST_CASE t31_name
    mov eax, 42            ; comparand
    mov ebx, 42            ; destination (matches eax)
    mov ecx, 99            ; new value
    cmpxchg ebx, ecx       ; if ebx==eax, ebx=ecx
    CHECK_EQ_32 ebx, 99

    ; ==== Test 32: cmpxchg fail ====
    TEST_CASE t32_name
    mov eax, 42
    mov ebx, 43            ; doesn't match eax
    mov ecx, 99
    cmpxchg ebx, ecx       ; ebx != eax, so eax = ebx
    CHECK_EQ_32 eax, 43    ; eax loaded with ebx's value

    ; ==== Test 33: bswap 32 ====
    TEST_CASE t33_name
    mov eax, 0x12345678
    bswap eax
    CHECK_EQ_32 eax, 0x78563412

    ; ==== Test 34: bswap 64 ====
    TEST_CASE t34_name
    mov rax, 0x0102030405060708
    bswap rax
    CHECK_EQ_64 rax, 0x0807060504030201

    ; ==== Test 35: add flags PF (parity) ====
    TEST_CASE t35_name
    mov eax, 0
    add eax, 0             ; result=0, has even parity -> PF set
    SAVE_FLAGS
    CHECK_FLAGS_EQ PF, PF

    ; ==== Test 36: lea basic ====
    TEST_CASE t36_name
    mov rax, 100
    lea rbx, [rax + 50]
    CHECK_EQ_64 rbx, 150

    ; ==== Test 37: lea SIB ====
    TEST_CASE t37_name
    mov rax, 10
    mov rbx, 20
    lea rcx, [rax + rbx*4 + 8]
    ; 10 + 20*4 + 8 = 10 + 80 + 8 = 98
    CHECK_EQ_64 rcx, 98

    ; ==== Test 38: cdq ====
    TEST_CASE t38_name
    mov eax, 0x80000000
    cdq
    ; edx should be 0xFFFFFFFF (sign extension of negative eax)
    CHECK_EQ_32 edx, 0xFFFFFFFF

    ; ==== Test 39: cqo ====
    TEST_CASE t39_name
    mov rax, 0x8000000000000000
    cqo
    CHECK_EQ_64 rdx, 0xFFFFFFFFFFFFFFFF

    ; ==== Test 40: movsx 8->32 ====
    TEST_CASE t40_name
    mov bl, 0x80            ; -128
    movsx eax, bl
    CHECK_EQ_32 eax, 0xFFFFFF80

    END_TESTS
