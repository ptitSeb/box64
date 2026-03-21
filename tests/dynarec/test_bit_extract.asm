; test_bit_extract.asm - Test PMOVMSKB, MOVMSKPS, MOVMSKPD
; These extract sign bits from SIMD registers - historically buggy (Bug #19)
%include "test_framework.inc"

section .data
    t1_name: db "pmovmskb all-FF", 0
    t2_name: db "pmovmskb all-00", 0
    t3_name: db "pmovmskb alternating 0xAA", 0
    t4_name: db "pmovmskb alternating 0x55", 0
    t5_name: db "pmovmskb low-half-set", 0
    t6_name: db "pmovmskb high-half-set", 0
    t7_name: db "pmovmskb single-byte-set", 0
    t8_name: db "pmovmskb pcmpeqb equal", 0
    t9_name: db "pmovmskb pcmpeqb differ", 0
    t10_name: db "movmskps all-neg", 0
    t11_name: db "movmskps all-pos", 0
    t12_name: db "movmskps mixed 0101", 0
    t13_name: db "movmskps mixed 1010", 0
    t14_name: db "movmskpd all-neg", 0
    t15_name: db "movmskpd all-pos", 0
    t16_name: db "movmskpd low-neg", 0
    t17_name: db "movmskpd high-neg", 0
    t18_name: db "pmovmskb after pcmpeqb partial", 0
    t19_name: db "pmovmskb byte 7 set only", 0
    t20_name: db "pmovmskb byte 15 set only", 0

    align 16
    ; Data patterns for movmskps
    ps_all_neg: dd 0x80000000, 0xFF800000, 0xBF800000, 0xC0000000
    ps_all_pos: dd 0x3F800000, 0x40000000, 0x00000001, 0x7F800000
    ps_0101:    dd 0x3F800000, 0xBF800000, 0x3F800000, 0xBF800000  ; +,-,+,-
    ps_1010:    dd 0xBF800000, 0x3F800000, 0xBF800000, 0x3F800000  ; -,+,-,+
    ; Data patterns for movmskpd
    pd_all_neg: dq 0x8000000000000000, 0xBFF0000000000000
    pd_all_pos: dq 0x3FF0000000000000, 0x0000000000000001
    pd_low_neg: dq 0xBFF0000000000000, 0x3FF0000000000000
    pd_high_neg: dq 0x3FF0000000000000, 0xBFF0000000000000
    ; Byte patterns
    pattern_aa: times 8 db 0xFF, 0x00   ; 0xFF at even bytes -> mask 0xAAAA... wait
                                         ; byte0=FF,byte1=00,byte2=FF,byte3=00...
                                         ; sign bits: 1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0 = 0x5555
    pattern_55: times 8 db 0x00, 0xFF   ; byte0=00,byte1=FF -> sign: 0,1,0,1... = 0xAAAA
    pattern_low: db 0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    pattern_high: db 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80
    pattern_byte7: db 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    pattern_byte15: db 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80
    ; For pcmpeqb partial match test
    pat_a: db 0x41,0x42,0x43,0x44, 0x45,0x46,0x47,0x48, 0x49,0x4A,0x4B,0x4C, 0x4D,0x4E,0x4F,0x50
    pat_b: db 0x41,0x00,0x43,0x00, 0x45,0x00,0x47,0x00, 0x49,0x00,0x4B,0x00, 0x4D,0x00,0x4F,0x00

section .text
global _start

_start:
    INIT_TESTS

    ; ==== Test 1: pmovmskb all-FF ====
    TEST_CASE t1_name
    pcmpeqd xmm0, xmm0         ; all 0xFF
    pmovmskb eax, xmm0
    CHECK_EQ_32 eax, 0xFFFF

    ; ==== Test 2: pmovmskb all-00 ====
    TEST_CASE t2_name
    pxor xmm0, xmm0
    pmovmskb eax, xmm0
    CHECK_EQ_32 eax, 0x0000

    ; ==== Test 3: pmovmskb alternating 0x5555 ====
    TEST_CASE t3_name
    movdqa xmm0, [rel pattern_aa]
    pmovmskb eax, xmm0
    CHECK_EQ_32 eax, 0x5555

    ; ==== Test 4: pmovmskb alternating 0xAAAA ====
    TEST_CASE t4_name
    movdqa xmm0, [rel pattern_55]
    pmovmskb eax, xmm0
    CHECK_EQ_32 eax, 0xAAAA

    ; ==== Test 5: pmovmskb low half set ====
    TEST_CASE t5_name
    movdqa xmm0, [rel pattern_low]
    pmovmskb eax, xmm0
    CHECK_EQ_32 eax, 0x00FF

    ; ==== Test 6: pmovmskb high half set ====
    TEST_CASE t6_name
    movdqa xmm0, [rel pattern_high]
    pmovmskb eax, xmm0
    CHECK_EQ_32 eax, 0xFF00

    ; ==== Test 7: pmovmskb single byte (byte 7) set ====
    TEST_CASE t7_name
    movdqa xmm0, [rel pattern_byte7]
    pmovmskb eax, xmm0
    CHECK_EQ_32 eax, 0x0080

    ; ==== Test 8: pmovmskb after pcmpeqb equal arrays ====
    TEST_CASE t8_name
    movdqa xmm0, [rel pat_a]
    movdqa xmm1, [rel pat_a]   ; same data
    pcmpeqb xmm0, xmm1
    pmovmskb eax, xmm0
    CHECK_EQ_32 eax, 0xFFFF

    ; ==== Test 9: pmovmskb after pcmpeqb all-different ====
    TEST_CASE t9_name
    pxor xmm0, xmm0            ; all zeros
    pcmpeqd xmm1, xmm1         ; all 0xFF
    pcmpeqb xmm0, xmm1
    pmovmskb eax, xmm0
    CHECK_EQ_32 eax, 0x0000

    ; ==== Test 10: movmskps all-negative ====
    TEST_CASE t10_name
    movaps xmm0, [rel ps_all_neg]
    movmskps eax, xmm0
    CHECK_EQ_32 eax, 0x000F

    ; ==== Test 11: movmskps all-positive ====
    TEST_CASE t11_name
    movaps xmm0, [rel ps_all_pos]
    movmskps eax, xmm0
    CHECK_EQ_32 eax, 0x0000

    ; ==== Test 12: movmskps mixed +,-,+,- ====
    TEST_CASE t12_name
    movaps xmm0, [rel ps_0101]
    movmskps eax, xmm0
    ; float[0]=+, float[1]=-, float[2]=+, float[3]=- -> bits: 1010 = 0xA
    CHECK_EQ_32 eax, 0x000A

    ; ==== Test 13: movmskps mixed -,+,-,+ ====
    TEST_CASE t13_name
    movaps xmm0, [rel ps_1010]
    movmskps eax, xmm0
    ; float[0]=-, float[1]=+, float[2]=-, float[3]=+ -> bits: 0101 = 0x5
    CHECK_EQ_32 eax, 0x0005

    ; ==== Test 14: movmskpd all-negative ====
    TEST_CASE t14_name
    movapd xmm0, [rel pd_all_neg]
    movmskpd eax, xmm0
    CHECK_EQ_32 eax, 0x0003

    ; ==== Test 15: movmskpd all-positive ====
    TEST_CASE t15_name
    movapd xmm0, [rel pd_all_pos]
    movmskpd eax, xmm0
    CHECK_EQ_32 eax, 0x0000

    ; ==== Test 16: movmskpd low-neg ====
    TEST_CASE t16_name
    movapd xmm0, [rel pd_low_neg]
    movmskpd eax, xmm0
    ; double[0]=neg, double[1]=pos -> bits: 01 = 1
    CHECK_EQ_32 eax, 0x0001

    ; ==== Test 17: movmskpd high-neg ====
    TEST_CASE t17_name
    movapd xmm0, [rel pd_high_neg]
    movmskpd eax, xmm0
    ; double[0]=pos, double[1]=neg -> bits: 10 = 2
    CHECK_EQ_32 eax, 0x0002

    ; ==== Test 18: pmovmskb after pcmpeqb partial match ====
    TEST_CASE t18_name
    movdqa xmm0, [rel pat_a]
    movdqa xmm1, [rel pat_b]
    pcmpeqb xmm0, xmm1
    pmovmskb eax, xmm0
    ; Equal at positions 0,2,4,6,8,10,12,14 -> mask = 0x5555
    CHECK_EQ_32 eax, 0x5555

    ; ==== Test 19: pmovmskb byte 7 set only ====
    TEST_CASE t19_name
    movdqa xmm0, [rel pattern_byte7]
    pmovmskb eax, xmm0
    CHECK_EQ_32 eax, 0x0080

    ; ==== Test 20: pmovmskb byte 15 set only ====
    TEST_CASE t20_name
    movdqa xmm0, [rel pattern_byte15]
    pmovmskb eax, xmm0
    CHECK_EQ_32 eax, 0x8000

    END_TESTS
