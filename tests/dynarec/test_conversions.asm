; test_conversions.asm - Test SSE/x86 conversion operations
; CVTSI2SS, CVTSI2SD, CVTTSS2SI, CVTTSD2SI, CVTSS2SD, CVTSD2SS,
; CVTPS2PD, CVTPD2PS, CVTDQ2PS, CVTPS2DQ, CVTTPS2DQ, CVTDQ2PD, CVTPD2DQ,
; MOVZX, MOVSX, CBW/CWDE/CDQE, MOVSXD
%include "test_framework.inc"

section .data
    t1_name:  db "cvtsi2ss i32->f32", 0
    t2_name:  db "cvtsi2ss i64->f32", 0
    t3_name:  db "cvtsi2sd i32->f64", 0
    t4_name:  db "cvtsi2sd i64->f64", 0
    t5_name:  db "cvttss2si f32->i32", 0
    t6_name:  db "cvttss2si f32->i64", 0
    t7_name:  db "cvttsd2si f64->i32", 0
    t8_name:  db "cvttsd2si f64->i64", 0
    t9_name:  db "cvtss2sd f32->f64", 0
    t10_name: db "cvtsd2ss f64->f32", 0
    t11_name: db "cvtps2pd 2xf32->2xf64", 0
    t12_name: db "cvtpd2ps 2xf64->2xf32", 0
    t13_name: db "cvtdq2ps 4xi32->4xf32", 0
    t14_name: db "cvtps2dq 4xf32->4xi32", 0
    t15_name: db "cvttps2dq truncate", 0
    t16_name: db "cvtdq2pd 2xi32->2xf64", 0
    t17_name: db "cvtpd2dq 2xf64->2xi32", 0
    t18_name: db "cvtsi2ss neg", 0
    t19_name: db "cvttsd2si neg trunc", 0
    t20_name: db "cvtss2sd neg", 0
    t21_name: db "movzx byte->32", 0
    t22_name: db "movzx word->32", 0
    t23_name: db "movzx byte->64", 0
    t24_name: db "movsx byte->32", 0
    t25_name: db "movsx word->32", 0
    t26_name: db "movsx byte->64", 0
    t27_name: db "movsxd i32->i64", 0
    t28_name: db "movsxd neg", 0
    t29_name: db "cvttss2si large", 0
    t30_name: db "cvtsi2sd large i64", 0
    t31_name: db "cvtps2dq neg round", 0
    t32_name: db "cvttps2dq neg trunc", 0
    t33_name: db "cbw/cwde/cdqe chain", 0
    t34_name: db "cvtsd2ss precision", 0
    t35_name: db "cvtss2si round", 0

    align 16
    ; Float values for conversion tests
    ; 42.0f = 0x42280000
    val_42f:    dd 0x42280000
    ; 42.0 = 0x4045000000000000
    val_42d:    dq 0x4045000000000000
    ; 3.7f = 0x406CCCCD
    val_3_7f:   dd 0x406CCCCD
    ; 3.7 = 0x400D999999999A (double)
    val_3_7d:   dq 0x400D99999999999A
    ; -7.9f = 0xC0FCCCD
    ; Actually: -7.9f = 0xC0FCCCCD
    val_neg7_9f: dd 0xC0FCCCCD
    ; -7.9 = 0xC01F999999999A (double)
    ; Actually: -7.9 = 0xC01F99999999999A
    val_neg7_9d: dq 0xC01F99999999999A
    ; 100000.0f = 0x47C35000
    val_100kf:  dd 0x47C35000
    ; -3.0f = 0xC0400000
    val_neg3f:  dd 0xC0400000
    ; -3.0 = 0xC008000000000000
    val_neg3d:  dq 0xC008000000000000
    ; 2.5f = 0x40200000
    val_2_5f:   dd 0x40200000
    ; -2.5f = 0xC0200000
    val_neg2_5f: dd 0xC0200000
    ; 1000000000 (10^9) as double = 0x41CDCD6500000000
    ; Actually: 1e9 = 0x41CDCD6500000000
    val_1e9d:   dq 0x41CDCD6500000000

    align 16
    ; Packed vectors for conversion
    ; [1.0f, 2.0f, 3.0f, 4.0f]
    vec_ps_1234: dd 0x3F800000, 0x40000000, 0x40400000, 0x40800000
    ; [1, 2, 3, 4] as dwords
    vec_dq_1234: dd 1, 2, 3, 4
    ; [1.0, 2.0] as doubles
    vec_pd_12:   dq 0x3FF0000000000000, 0x4000000000000000
    ; [-1.5f, 2.5f, -3.5f, 4.5f]
    ; -1.5f=0xBFC00000, 2.5f=0x40200000, -3.5f=0xC0600000, 4.5f=0x40900000
    vec_ps_neg:  dd 0xBFC00000, 0x40200000, 0xC0600000, 0x40900000
    ; [3.0, 4.0] as doubles
    vec_pd_34:   dq 0x4008000000000000, 0x4010000000000000

section .text
global _start

_start:
    INIT_TESTS

    ; ==== Test 1: cvtsi2ss i32 -> f32: 42 -> 42.0f ====
    TEST_CASE t1_name
    mov eax, 42
    xorps xmm0, xmm0
    cvtsi2ss xmm0, eax
    movd eax, xmm0
    CHECK_EQ_32 eax, 0x42280000      ; 42.0f

    ; ==== Test 2: cvtsi2ss i64 -> f32: 42 -> 42.0f ====
    TEST_CASE t2_name
    mov rax, 42
    xorps xmm0, xmm0
    cvtsi2ss xmm0, rax
    movd eax, xmm0
    CHECK_EQ_32 eax, 0x42280000      ; 42.0f

    ; ==== Test 3: cvtsi2sd i32 -> f64: 42 -> 42.0 ====
    TEST_CASE t3_name
    mov eax, 42
    xorpd xmm0, xmm0
    cvtsi2sd xmm0, eax
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x4045000000000000  ; 42.0

    ; ==== Test 4: cvtsi2sd i64 -> f64: 42 -> 42.0 ====
    TEST_CASE t4_name
    mov rax, 42
    xorpd xmm0, xmm0
    cvtsi2sd xmm0, rax
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x4045000000000000  ; 42.0

    ; ==== Test 5: cvttss2si f32 -> i32 (truncate): 3.7f -> 3 ====
    TEST_CASE t5_name
    movss xmm0, [rel val_3_7f]
    cvttss2si eax, xmm0
    CHECK_EQ_32 eax, 3

    ; ==== Test 6: cvttss2si f32 -> i64 (truncate): 3.7f -> 3 ====
    TEST_CASE t6_name
    movss xmm0, [rel val_3_7f]
    cvttss2si rax, xmm0
    CHECK_EQ_64 rax, 3

    ; ==== Test 7: cvttsd2si f64 -> i32 (truncate): 42.0 -> 42 ====
    TEST_CASE t7_name
    movsd xmm0, [rel val_42d]
    cvttsd2si eax, xmm0
    CHECK_EQ_32 eax, 42

    ; ==== Test 8: cvttsd2si f64 -> i64 (truncate): 42.0 -> 42 ====
    TEST_CASE t8_name
    movsd xmm0, [rel val_42d]
    cvttsd2si rax, xmm0
    CHECK_EQ_64 rax, 42

    ; ==== Test 9: cvtss2sd f32 -> f64: 42.0f -> 42.0 ====
    TEST_CASE t9_name
    movss xmm0, [rel val_42f]
    cvtss2sd xmm1, xmm0
    movq rax, xmm1
    CHECK_EQ_64 rax, 0x4045000000000000  ; 42.0

    ; ==== Test 10: cvtsd2ss f64 -> f32: 42.0 -> 42.0f ====
    TEST_CASE t10_name
    movsd xmm0, [rel val_42d]
    cvtsd2ss xmm1, xmm0
    movd eax, xmm1
    CHECK_EQ_32 eax, 0x42280000      ; 42.0f

    ; ==== Test 11: cvtps2pd low 2 floats -> 2 doubles ====
    TEST_CASE t11_name
    ; [1.0f, 2.0f, ...] -> [1.0, 2.0]
    movaps xmm0, [rel vec_ps_1234]
    cvtps2pd xmm1, xmm0
    ; low double should be 1.0 = 0x3FF0000000000000
    movq rax, xmm1
    CHECK_EQ_64 rax, 0x3FF0000000000000

    ; ==== Test 12: cvtpd2ps 2 doubles -> low 2 floats ====
    TEST_CASE t12_name
    ; [3.0, 4.0] -> [3.0f, 4.0f, 0, 0]
    movapd xmm0, [rel vec_pd_34]
    cvtpd2ps xmm1, xmm0
    ; low 64 = [3.0f=0x40400000, 4.0f=0x40800000]
    movq rax, xmm1
    CHECK_EQ_64 rax, 0x4080000040400000

    ; ==== Test 13: cvtdq2ps 4xi32 -> 4xf32 ====
    TEST_CASE t13_name
    movdqa xmm0, [rel vec_dq_1234]
    cvtdq2ps xmm1, xmm0
    ; [1,2,3,4] -> [1.0f, 2.0f, 3.0f, 4.0f]
    ; low 64: [1.0f=0x3F800000, 2.0f=0x40000000]
    movq rax, xmm1
    CHECK_EQ_64 rax, 0x400000003F800000

    ; ==== Test 14: cvtps2dq 4xf32 -> 4xi32 (round to nearest) ====
    TEST_CASE t14_name
    movaps xmm0, [rel vec_ps_1234]
    cvtps2dq xmm1, xmm0
    ; [1.0f, 2.0f, 3.0f, 4.0f] -> [1, 2, 3, 4]
    movq rax, xmm1
    CHECK_EQ_64 rax, 0x0000000200000001

    ; ==== Test 15: cvttps2dq truncate ====
    TEST_CASE t15_name
    ; Use 3.7f -> 3 (truncation)
    movss xmm0, [rel val_3_7f]
    cvttps2dq xmm1, xmm0
    movd eax, xmm1
    CHECK_EQ_32 eax, 3

    ; ==== Test 16: cvtdq2pd low 2 i32 -> 2 doubles ====
    TEST_CASE t16_name
    movdqa xmm0, [rel vec_dq_1234]
    cvtdq2pd xmm1, xmm0
    ; [1, 2, ...] -> [1.0, 2.0]
    movq rax, xmm1
    CHECK_EQ_64 rax, 0x3FF0000000000000  ; 1.0

    ; ==== Test 17: cvtpd2dq 2 doubles -> low 2 i32 ====
    TEST_CASE t17_name
    movapd xmm0, [rel vec_pd_34]
    cvtpd2dq xmm1, xmm0
    ; [3.0, 4.0] -> [3, 4, 0, 0]
    movq rax, xmm1
    CHECK_EQ_64 rax, 0x0000000400000003

    ; ==== Test 18: cvtsi2ss negative: -42 -> -42.0f ====
    TEST_CASE t18_name
    mov eax, -42
    xorps xmm0, xmm0
    cvtsi2ss xmm0, eax
    movd eax, xmm0
    ; -42.0f = 0xC2280000
    CHECK_EQ_32 eax, 0xC2280000

    ; ==== Test 19: cvttsd2si negative truncate: -7.9 -> -7 ====
    TEST_CASE t19_name
    movsd xmm0, [rel val_neg7_9d]
    cvttsd2si eax, xmm0
    ; truncate toward zero: -7.9 -> -7
    CHECK_EQ_32 eax, -7

    ; ==== Test 20: cvtss2sd negative: -3.0f -> -3.0 ====
    TEST_CASE t20_name
    movss xmm0, [rel val_neg3f]
    cvtss2sd xmm1, xmm0
    movq rax, xmm1
    CHECK_EQ_64 rax, 0xC008000000000000  ; -3.0

    ; ==== Test 21: movzx byte -> 32: 0xFF -> 0x000000FF ====
    TEST_CASE t21_name
    mov eax, 0xDEADBEEF
    mov al, 0xFF
    movzx ecx, al
    CHECK_EQ_32 ecx, 0xFF

    ; ==== Test 22: movzx word -> 32: 0xBEEF -> 0x0000BEEF ====
    TEST_CASE t22_name
    mov eax, 0xBEEF
    movzx ecx, ax
    CHECK_EQ_32 ecx, 0xBEEF

    ; ==== Test 23: movzx byte -> 64 ====
    TEST_CASE t23_name
    mov al, 0x80
    movzx rcx, al
    CHECK_EQ_64 rcx, 0x80

    ; ==== Test 24: movsx byte -> 32: 0x80 -> 0xFFFFFF80 ====
    TEST_CASE t24_name
    mov al, 0x80
    movsx ecx, al
    CHECK_EQ_32 ecx, 0xFFFFFF80

    ; ==== Test 25: movsx word -> 32: 0x8000 -> 0xFFFF8000 ====
    TEST_CASE t25_name
    mov ax, 0x8000
    movsx ecx, ax
    CHECK_EQ_32 ecx, 0xFFFF8000

    ; ==== Test 26: movsx byte -> 64: 0x80 -> 0xFFFFFFFFFFFFFF80 ====
    TEST_CASE t26_name
    mov al, 0x80
    movsx rcx, al
    CHECK_EQ_64 rcx, 0xFFFFFFFFFFFFFF80

    ; ==== Test 27: movsxd i32 -> i64: 42 -> 42 ====
    TEST_CASE t27_name
    mov eax, 42
    movsxd rcx, eax
    CHECK_EQ_64 rcx, 42

    ; ==== Test 28: movsxd negative: 0xFFFFFFFC (-4) -> 0xFFFFFFFFFFFFFFFC ====
    TEST_CASE t28_name
    mov eax, -4
    movsxd rcx, eax
    CHECK_EQ_64 rcx, 0xFFFFFFFFFFFFFFFC

    ; ==== Test 29: cvttss2si large float: 100000.0f -> 100000 ====
    TEST_CASE t29_name
    movss xmm0, [rel val_100kf]
    cvttss2si eax, xmm0
    CHECK_EQ_32 eax, 100000

    ; ==== Test 30: cvtsi2sd large i64: 1000000000 -> 1e9 ====
    TEST_CASE t30_name
    mov rax, 1000000000
    xorpd xmm0, xmm0
    cvtsi2sd xmm0, rax
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x41CDCD6500000000  ; 1e9

    ; ==== Test 31: cvtps2dq negative round: -1.5f -> -2 (round to nearest even) ====
    TEST_CASE t31_name
    movaps xmm0, [rel vec_ps_neg]
    cvtps2dq xmm1, xmm0
    ; -1.5f rounds to -2 (banker's rounding), 2.5f rounds to 2
    ; -3.5f rounds to -4, 4.5f rounds to 4
    movd eax, xmm1
    ; -2 as i32 = 0xFFFFFFFE
    CHECK_EQ_32 eax, 0xFFFFFFFE

    ; ==== Test 32: cvttps2dq negative truncate: -1.5f -> -1 ====
    TEST_CASE t32_name
    movaps xmm0, [rel vec_ps_neg]
    cvttps2dq xmm1, xmm0
    ; -1.5f truncates to -1
    movd eax, xmm1
    CHECK_EQ_32 eax, 0xFFFFFFFF      ; -1

    ; ==== Test 33: cbw/cwde/cdqe chain: 0x80 -> sign-extend through ====
    TEST_CASE t33_name
    mov al, 0x80           ; -128 in AL
    cbw                     ; AL -> AX: 0xFF80
    cwde                    ; AX -> EAX: 0xFFFFFF80
    cdqe                    ; EAX -> RAX: 0xFFFFFFFFFFFFFF80
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFF80

    ; ==== Test 34: cvtsd2ss precision: 3.0 -> 3.0f (exact) ====
    TEST_CASE t34_name
    movsd xmm0, [rel val_neg3d]
    cvtsd2ss xmm1, xmm0
    movd eax, xmm1
    CHECK_EQ_32 eax, 0xC0400000      ; -3.0f

    ; ==== Test 35: cvtss2si round: 2.5f -> 2 (round to nearest even) ====
    TEST_CASE t35_name
    movss xmm0, [rel val_2_5f]
    cvtss2si eax, xmm0
    ; 2.5 rounds to 2 (banker's rounding: ties to even)
    CHECK_EQ_32 eax, 2

    END_TESTS
