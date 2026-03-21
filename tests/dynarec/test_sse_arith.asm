; test_sse_arith.asm - Test SSE floating-point arithmetic operations
; ADDPS/PD/SS/SD, SUBPS/PD/SS/SD, MULPS/PD/SS/SD, DIVPS/PD/SS/SD,
; SQRTPS/PD/SS/SD, MINPS/PD/SS/SD, MAXPS/PD/SS/SD, COMISS/COMISD,
; RCPPS, RSQRTPS, ANDPS/ANDPD/ORPS/ORPD/XORPS/XORPD
%include "test_framework.inc"

section .data
    t1_name:  db "addss 1.0+2.0", 0
    t2_name:  db "addsd 1.0+2.0", 0
    t3_name:  db "addps [1,2,3,4]+[5,6,7,8]", 0
    t4_name:  db "addpd [1,2]+[3,4]", 0
    t5_name:  db "subss 5.0-3.0", 0
    t6_name:  db "subsd 5.0-3.0", 0
    t7_name:  db "subps [5,6,7,8]-[1,2,3,4]", 0
    t8_name:  db "subpd [5,6]-[1,2]", 0
    t9_name:  db "mulss 3.0*4.0", 0
    t10_name: db "mulsd 3.0*4.0", 0
    t11_name: db "mulps [1,2,3,4]*[5,6,7,8]", 0
    t12_name: db "mulpd [2,3]*[4,5]", 0
    t13_name: db "divss 12.0/4.0", 0
    t14_name: db "divsd 12.0/4.0", 0
    t15_name: db "divps [12,24,36,48]/[4,8,12,16]", 0
    t16_name: db "divpd [12,24]/[4,8]", 0
    t17_name: db "sqrtss 16.0", 0
    t18_name: db "sqrtsd 16.0", 0
    t19_name: db "sqrtps [4,9,16,25]", 0
    t20_name: db "sqrtpd [4,16]", 0
    t21_name: db "minss 3,7", 0
    t22_name: db "minsd 3,7", 0
    t23_name: db "minps [3,1,7,2],[5,0,4,8]", 0
    t24_name: db "minpd [3,7],[5,1]", 0
    t25_name: db "maxss 3,7", 0
    t26_name: db "maxsd 3,7", 0
    t27_name: db "maxps [3,1,7,2],[5,0,4,8]", 0
    t28_name: db "maxpd [3,7],[5,1]", 0
    t29_name: db "comiss eq", 0
    t30_name: db "comiss lt", 0
    t31_name: db "comiss gt", 0
    t32_name: db "comisd eq", 0
    t33_name: db "comisd lt", 0
    t34_name: db "comisd gt", 0
    t35_name: db "andps", 0
    t36_name: db "andpd", 0
    t37_name: db "orps", 0
    t38_name: db "xorps zero self", 0
    t39_name: db "addss neg+pos", 0
    t40_name: db "mulsd neg*neg", 0
    t41_name: db "subps [0,0,0,0]-[1,2,3,4]", 0
    t42_name: db "divsd 1.0/0.25", 0
    t43_name: db "sqrtsd 0.25", 0
    t44_name: db "minss neg", 0
    t45_name: db "maxsd neg", 0

    align 16
    ; Single-precision vectors (32-bit floats, little-endian)
    ; IEEE 754: 1.0f = 0x3F800000, 2.0f = 0x40000000, 3.0f = 0x40400000
    ; 4.0f = 0x40800000, 5.0f = 0x40A00000, 6.0f = 0x40C00000
    ; 7.0f = 0x40E00000, 8.0f = 0x41000000
    vec_ps_1234:  dd 0x3F800000, 0x40000000, 0x40400000, 0x40800000  ; 1,2,3,4
    vec_ps_5678:  dd 0x40A00000, 0x40C00000, 0x40E00000, 0x41000000  ; 5,6,7,8
    vec_ps_zero:  dd 0, 0, 0, 0
    ; 3.0f = 0x40400000, 1.0f, 7.0f = 0x40E00000, 2.0f
    vec_ps_3172:  dd 0x40400000, 0x3F800000, 0x40E00000, 0x40000000  ; 3,1,7,2
    ; 5.0f, 0.0f, 4.0f = 0x40800000, 8.0f
    vec_ps_5048:  dd 0x40A00000, 0x00000000, 0x40800000, 0x41000000  ; 5,0,4,8
    ; 12.0f=0x41400000, 24.0f=0x41C00000, 36.0f=0x42100000, 48.0f=0x42400000
    vec_ps_div_a: dd 0x41400000, 0x41C00000, 0x42100000, 0x42400000  ; 12,24,36,48
    ; 4.0f, 8.0f, 12.0f=0x41400000, 16.0f=0x41800000
    vec_ps_div_b: dd 0x40800000, 0x41000000, 0x41400000, 0x41800000  ; 4,8,12,16
    ; 4.0f, 9.0f=0x41100000, 16.0f=0x41800000, 25.0f=0x41C80000
    vec_ps_sqrt:  dd 0x40800000, 0x41100000, 0x41800000, 0x41C80000  ; 4,9,16,25

    ; Double-precision vectors (64-bit doubles, little-endian)
    ; IEEE 754: 1.0 = 0x3FF0000000000000, 2.0 = 0x4000000000000000
    ; 3.0 = 0x4008000000000000, 4.0 = 0x4010000000000000
    ; 5.0 = 0x4014000000000000, 6.0 = 0x4018000000000000
    ; 7.0 = 0x401C000000000000, 8.0 = 0x4020000000000000
    vec_pd_12:    dq 0x3FF0000000000000, 0x4000000000000000  ; 1.0, 2.0
    vec_pd_34:    dq 0x4008000000000000, 0x4010000000000000  ; 3.0, 4.0
    vec_pd_56:    dq 0x4014000000000000, 0x4018000000000000  ; 5.0, 6.0
    vec_pd_23:    dq 0x4000000000000000, 0x4008000000000000  ; 2.0, 3.0
    vec_pd_45:    dq 0x4010000000000000, 0x4014000000000000  ; 4.0, 5.0
    vec_pd_37:    dq 0x4008000000000000, 0x401C000000000000  ; 3.0, 7.0
    vec_pd_51:    dq 0x4014000000000000, 0x3FF0000000000000  ; 5.0, 1.0
    ; 12.0 = 0x4028000000000000, 24.0 = 0x4038000000000000
    vec_pd_div_a: dq 0x4028000000000000, 0x4038000000000000  ; 12.0, 24.0
    ; 4.0, 8.0 = 0x4020000000000000
    vec_pd_div_b: dq 0x4010000000000000, 0x4020000000000000  ; 4.0, 8.0
    vec_pd_4_16:  dq 0x4010000000000000, 0x4030000000000000  ; 4.0, 16.0

    ; Scalar values (loaded via movss/movsd or from memory)
    ; 1.0f = 0x3F800000
    val_1f:  dd 0x3F800000
    ; 2.0f
    val_2f:  dd 0x40000000
    ; 3.0f
    val_3f:  dd 0x40400000
    ; 4.0f
    val_4f:  dd 0x40800000
    ; 5.0f
    val_5f:  dd 0x40A00000
    ; 7.0f
    val_7f:  dd 0x40E00000
    ; 12.0f
    val_12f: dd 0x41400000
    ; 16.0f
    val_16f: dd 0x41800000
    ; -3.0f = 0xC0400000
    val_neg3f: dd 0xC0400000
    ; -5.0f = 0xC0A00000
    val_neg5f: dd 0xC0A00000

    ; 1.0 = 0x3FF0000000000000
    val_1d:  dq 0x3FF0000000000000
    ; 2.0
    val_2d:  dq 0x4000000000000000
    ; 3.0
    val_3d:  dq 0x4008000000000000
    ; 4.0
    val_4d:  dq 0x4010000000000000
    ; 5.0
    val_5d:  dq 0x4014000000000000
    ; 7.0
    val_7d:  dq 0x401C000000000000
    ; 12.0
    val_12d: dq 0x4028000000000000
    ; 16.0
    val_16d: dq 0x4030000000000000
    ; 0.25 = 0x3FD0000000000000
    val_025d: dq 0x3FD0000000000000
    ; -3.0 = 0xC008000000000000
    val_neg3d: dq 0xC008000000000000
    ; -5.0 = 0xC014000000000000
    val_neg5d: dq 0xC014000000000000

    ; ANDPS/ORPS masks
    align 16
    vec_ps_allones: dd 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF
    vec_pd_allones: dq 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF
    ; Sign mask for single: bit 31 set
    vec_ps_signmask: dd 0x80000000, 0x80000000, 0x80000000, 0x80000000
    ; Absolute value mask for double: clear bit 63
    vec_pd_absmask:  dq 0x7FFFFFFFFFFFFFFF, 0x7FFFFFFFFFFFFFFF

section .text
global _start

_start:
    INIT_TESTS

    ; ==== Test 1: addss 1.0f + 2.0f = 3.0f ====
    TEST_CASE t1_name
    movss xmm0, [rel val_1f]
    movss xmm1, [rel val_2f]
    addss xmm0, xmm1
    movd eax, xmm0
    CHECK_EQ_32 eax, 0x40400000       ; 3.0f

    ; ==== Test 2: addsd 1.0 + 2.0 = 3.0 ====
    TEST_CASE t2_name
    movsd xmm0, [rel val_1d]
    movsd xmm1, [rel val_2d]
    addsd xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x4008000000000000 ; 3.0

    ; ==== Test 3: addps [1,2,3,4]+[5,6,7,8]=[6,8,10,12] ====
    TEST_CASE t3_name
    movaps xmm0, [rel vec_ps_1234]
    movaps xmm1, [rel vec_ps_5678]
    addps xmm0, xmm1
    ; 6.0f=0x40C00000, 8.0f=0x41000000 -> low 64: 0x4100000040C00000
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x4100000040C00000

    ; ==== Test 4: addpd [1,2]+[3,4]=[4,6] ====
    TEST_CASE t4_name
    movapd xmm0, [rel vec_pd_12]
    movapd xmm1, [rel vec_pd_34]
    addpd xmm0, xmm1
    ; 4.0 = 0x4010000000000000
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x4010000000000000 ; low element = 4.0

    ; ==== Test 5: subss 5.0f - 3.0f = 2.0f ====
    TEST_CASE t5_name
    movss xmm0, [rel val_5f]
    movss xmm1, [rel val_3f]
    subss xmm0, xmm1
    movd eax, xmm0
    CHECK_EQ_32 eax, 0x40000000       ; 2.0f

    ; ==== Test 6: subsd 5.0 - 3.0 = 2.0 ====
    TEST_CASE t6_name
    movsd xmm0, [rel val_5d]
    movsd xmm1, [rel val_3d]
    subsd xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x4000000000000000 ; 2.0

    ; ==== Test 7: subps [5,6,7,8]-[1,2,3,4]=[4,4,4,4] ====
    TEST_CASE t7_name
    movaps xmm0, [rel vec_ps_5678]
    movaps xmm1, [rel vec_ps_1234]
    subps xmm0, xmm1
    ; 4.0f=0x40800000 -> low 64: 0x4080000040800000
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x4080000040800000

    ; ==== Test 8: subpd [5,6]-[1,2]=[4,4] ====
    TEST_CASE t8_name
    movapd xmm0, [rel vec_pd_56]
    movapd xmm1, [rel vec_pd_12]
    subpd xmm0, xmm1
    ; 4.0 = 0x4010000000000000
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x4010000000000000

    ; ==== Test 9: mulss 3.0f * 4.0f = 12.0f ====
    TEST_CASE t9_name
    movss xmm0, [rel val_3f]
    movss xmm1, [rel val_4f]
    mulss xmm0, xmm1
    movd eax, xmm0
    CHECK_EQ_32 eax, 0x41400000       ; 12.0f

    ; ==== Test 10: mulsd 3.0 * 4.0 = 12.0 ====
    TEST_CASE t10_name
    movsd xmm0, [rel val_3d]
    movsd xmm1, [rel val_4d]
    mulsd xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x4028000000000000 ; 12.0

    ; ==== Test 11: mulps [1,2,3,4]*[5,6,7,8]=[5,12,21,32] ====
    TEST_CASE t11_name
    movaps xmm0, [rel vec_ps_1234]
    movaps xmm1, [rel vec_ps_5678]
    mulps xmm0, xmm1
    ; 5.0f=0x40A00000, 12.0f=0x41400000 -> low 64: 0x4140000040A00000
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x4140000040A00000

    ; ==== Test 12: mulpd [2,3]*[4,5]=[8,15] ====
    TEST_CASE t12_name
    movapd xmm0, [rel vec_pd_23]
    movapd xmm1, [rel vec_pd_45]
    mulpd xmm0, xmm1
    ; 8.0 = 0x4020000000000000
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x4020000000000000 ; low element = 8.0

    ; ==== Test 13: divss 12.0f / 4.0f = 3.0f ====
    TEST_CASE t13_name
    movss xmm0, [rel val_12f]
    movss xmm1, [rel val_4f]
    divss xmm0, xmm1
    movd eax, xmm0
    CHECK_EQ_32 eax, 0x40400000       ; 3.0f

    ; ==== Test 14: divsd 12.0 / 4.0 = 3.0 ====
    TEST_CASE t14_name
    movsd xmm0, [rel val_12d]
    movsd xmm1, [rel val_4d]
    divsd xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x4008000000000000 ; 3.0

    ; ==== Test 15: divps [12,24,36,48]/[4,8,12,16]=[3,3,3,3] ====
    TEST_CASE t15_name
    movaps xmm0, [rel vec_ps_div_a]
    movaps xmm1, [rel vec_ps_div_b]
    divps xmm0, xmm1
    ; 3.0f=0x40400000 -> low 64: 0x4040000040400000
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x4040000040400000

    ; ==== Test 16: divpd [12,24]/[4,8]=[3,3] ====
    TEST_CASE t16_name
    movapd xmm0, [rel vec_pd_div_a]
    movapd xmm1, [rel vec_pd_div_b]
    divpd xmm0, xmm1
    ; 3.0 = 0x4008000000000000
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x4008000000000000

    ; ==== Test 17: sqrtss 16.0 = 4.0 ====
    TEST_CASE t17_name
    movss xmm0, [rel val_16f]
    sqrtss xmm1, xmm0
    movd eax, xmm1
    CHECK_EQ_32 eax, 0x40800000       ; 4.0f

    ; ==== Test 18: sqrtsd 16.0 = 4.0 ====
    TEST_CASE t18_name
    movsd xmm0, [rel val_16d]
    sqrtsd xmm1, xmm0
    movq rax, xmm1
    CHECK_EQ_64 rax, 0x4010000000000000 ; 4.0

    ; ==== Test 19: sqrtps [4,9,16,25]=[2,3,4,5] ====
    TEST_CASE t19_name
    movaps xmm0, [rel vec_ps_sqrt]
    sqrtps xmm1, xmm0
    ; 2.0f=0x40000000, 3.0f=0x40400000 -> low 64: 0x4040000040000000
    movq rax, xmm1
    CHECK_EQ_64 rax, 0x4040000040000000

    ; ==== Test 20: sqrtpd [4,16]=[2,4] ====
    TEST_CASE t20_name
    movapd xmm0, [rel vec_pd_4_16]
    sqrtpd xmm1, xmm0
    ; 2.0 = 0x4000000000000000
    movq rax, xmm1
    CHECK_EQ_64 rax, 0x4000000000000000 ; low element = 2.0

    ; ==== Test 21: minss min(3.0, 7.0) = 3.0 ====
    TEST_CASE t21_name
    movss xmm0, [rel val_3f]
    movss xmm1, [rel val_7f]
    minss xmm0, xmm1
    movd eax, xmm0
    CHECK_EQ_32 eax, 0x40400000       ; 3.0f

    ; ==== Test 22: minsd min(3.0, 7.0) = 3.0 ====
    TEST_CASE t22_name
    movsd xmm0, [rel val_3d]
    movsd xmm1, [rel val_7d]
    minsd xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x4008000000000000 ; 3.0

    ; ==== Test 23: minps min([3,1,7,2],[5,0,4,8])=[3,0,4,2] ====
    TEST_CASE t23_name
    movaps xmm0, [rel vec_ps_3172]
    movaps xmm1, [rel vec_ps_5048]
    minps xmm0, xmm1
    ; min(3,5)=3=0x40400000, min(1,0)=0=0x00000000 -> low 64: 0x0000000040400000
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0000000040400000

    ; ==== Test 24: minpd min([3,7],[5,1])=[3,1] ====
    TEST_CASE t24_name
    movapd xmm0, [rel vec_pd_37]
    movapd xmm1, [rel vec_pd_51]
    minpd xmm0, xmm1
    ; min(3,5)=3 = 0x4008000000000000
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x4008000000000000

    ; ==== Test 25: maxss max(3.0, 7.0) = 7.0 ====
    TEST_CASE t25_name
    movss xmm0, [rel val_3f]
    movss xmm1, [rel val_7f]
    maxss xmm0, xmm1
    movd eax, xmm0
    CHECK_EQ_32 eax, 0x40E00000       ; 7.0f

    ; ==== Test 26: maxsd max(3.0, 7.0) = 7.0 ====
    TEST_CASE t26_name
    movsd xmm0, [rel val_3d]
    movsd xmm1, [rel val_7d]
    maxsd xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x401C000000000000 ; 7.0

    ; ==== Test 27: maxps max([3,1,7,2],[5,0,4,8])=[5,1,7,8] ====
    TEST_CASE t27_name
    movaps xmm0, [rel vec_ps_3172]
    movaps xmm1, [rel vec_ps_5048]
    maxps xmm0, xmm1
    ; max(3,5)=5=0x40A00000, max(1,0)=1=0x3F800000 -> low 64: 0x3F80000040A00000
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x3F80000040A00000

    ; ==== Test 28: maxpd max([3,7],[5,1])=[5,7] ====
    TEST_CASE t28_name
    movapd xmm0, [rel vec_pd_37]
    movapd xmm1, [rel vec_pd_51]
    maxpd xmm0, xmm1
    ; max(3,5)=5 = 0x4014000000000000
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x4014000000000000

    ; ==== Test 29: comiss equal -> ZF=1 CF=0 ====
    TEST_CASE t29_name
    movss xmm0, [rel val_3f]
    movss xmm1, [rel val_3f]
    comiss xmm0, xmm1
    SAVE_FLAGS
    ; Equal: ZF=1, PF=0, CF=0
    CHECK_FLAGS_EQ (ZF|CF|PF), ZF

    ; ==== Test 30: comiss less than -> CF=1 ====
    TEST_CASE t30_name
    movss xmm0, [rel val_3f]
    movss xmm1, [rel val_7f]
    comiss xmm0, xmm1
    SAVE_FLAGS
    ; 3.0 < 7.0: CF=1, ZF=0, PF=0
    CHECK_FLAGS_EQ (ZF|CF|PF), CF

    ; ==== Test 31: comiss greater than -> CF=0 ZF=0 ====
    TEST_CASE t31_name
    movss xmm0, [rel val_7f]
    movss xmm1, [rel val_3f]
    comiss xmm0, xmm1
    SAVE_FLAGS
    ; 7.0 > 3.0: CF=0, ZF=0, PF=0
    CHECK_FLAGS_EQ (ZF|CF|PF), 0

    ; ==== Test 32: comisd equal -> ZF=1 CF=0 ====
    TEST_CASE t32_name
    movsd xmm0, [rel val_3d]
    movsd xmm1, [rel val_3d]
    comisd xmm0, xmm1
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|PF), ZF

    ; ==== Test 33: comisd less than -> CF=1 ====
    TEST_CASE t33_name
    movsd xmm0, [rel val_3d]
    movsd xmm1, [rel val_7d]
    comisd xmm0, xmm1
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|PF), CF

    ; ==== Test 34: comisd greater than -> CF=0 ZF=0 ====
    TEST_CASE t34_name
    movsd xmm0, [rel val_7d]
    movsd xmm1, [rel val_3d]
    comisd xmm0, xmm1
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|PF), 0

    ; ==== Test 35: andps ====
    TEST_CASE t35_name
    ; Use andps to clear sign bit (absolute value for single)
    ; -3.0f = 0xC0400000, abs mask = 0x7FFFFFFF -> 0x40400000 = 3.0f
    movss xmm0, [rel val_neg3f]
    ; Create abs mask: 0x7FFFFFFF in all lanes
    movaps xmm1, [rel vec_ps_allones]
    psrld xmm1, 1                ; 0x7FFFFFFF in each dword
    andps xmm0, xmm1
    movd eax, xmm0
    CHECK_EQ_32 eax, 0x40400000       ; 3.0f (abs of -3.0f)

    ; ==== Test 36: andpd ====
    TEST_CASE t36_name
    ; -3.0 = 0xC008000000000000, abs -> 0x4008000000000000 = 3.0
    movsd xmm0, [rel val_neg3d]
    movapd xmm1, [rel vec_pd_absmask]
    andpd xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x4008000000000000 ; 3.0

    ; ==== Test 37: orps - set sign bit (negate) ====
    TEST_CASE t37_name
    ; 3.0f = 0x40400000, OR with 0x80000000 -> 0xC0400000 = -3.0f
    movss xmm0, [rel val_3f]
    movaps xmm1, [rel vec_ps_signmask]
    orps xmm0, xmm1
    movd eax, xmm0
    CHECK_EQ_32 eax, 0xC0400000       ; -3.0f

    ; ==== Test 38: xorps zero self ====
    TEST_CASE t38_name
    movaps xmm0, [rel vec_ps_1234]
    xorps xmm0, xmm0
    movq rax, xmm0
    CHECK_EQ_64 rax, 0

    ; ==== Test 39: addss negative + positive: -5.0 + 3.0 = -2.0 ====
    TEST_CASE t39_name
    movss xmm0, [rel val_neg5f]
    movss xmm1, [rel val_3f]
    addss xmm0, xmm1
    movd eax, xmm0
    ; -2.0f = 0xC0000000
    CHECK_EQ_32 eax, 0xC0000000

    ; ==== Test 40: mulsd negative * negative: (-3.0)*(-5.0) = 15.0 ====
    TEST_CASE t40_name
    movsd xmm0, [rel val_neg3d]
    movsd xmm1, [rel val_neg5d]
    mulsd xmm0, xmm1
    movq rax, xmm0
    ; 15.0 = 0x402E000000000000
    CHECK_EQ_64 rax, 0x402E000000000000

    ; ==== Test 41: subps [0,0,0,0]-[1,2,3,4]=[-1,-2,-3,-4] ====
    TEST_CASE t41_name
    movaps xmm0, [rel vec_ps_zero]
    movaps xmm1, [rel vec_ps_1234]
    subps xmm0, xmm1
    ; -1.0f=0xBF800000, -2.0f=0xC0000000 -> low 64: 0xC0000000BF800000
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xC0000000BF800000

    ; ==== Test 42: divsd 1.0 / 0.25 = 4.0 ====
    TEST_CASE t42_name
    movsd xmm0, [rel val_1d]
    movsd xmm1, [rel val_025d]
    divsd xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x4010000000000000 ; 4.0

    ; ==== Test 43: sqrtsd 0.25 = 0.5 ====
    TEST_CASE t43_name
    movsd xmm0, [rel val_025d]
    sqrtsd xmm1, xmm0
    movq rax, xmm1
    ; 0.5 = 0x3FE0000000000000
    CHECK_EQ_64 rax, 0x3FE0000000000000

    ; ==== Test 44: minss with negative: min(-3.0, 3.0) = -3.0 ====
    TEST_CASE t44_name
    movss xmm0, [rel val_neg3f]
    movss xmm1, [rel val_3f]
    minss xmm0, xmm1
    movd eax, xmm0
    CHECK_EQ_32 eax, 0xC0400000       ; -3.0f

    ; ==== Test 45: maxsd with negative: max(-5.0, -3.0) = -3.0 ====
    TEST_CASE t45_name
    movsd xmm0, [rel val_neg5d]
    movsd xmm1, [rel val_neg3d]
    maxsd xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xC008000000000000 ; -3.0

    END_TESTS
