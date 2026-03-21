; test_sse3_move.asm - Test SSE3 move/duplicate instructions and
; reciprocal/reciprocal-sqrt approximation instructions
; MOVDDUP, MOVSLDUP, MOVSHDUP, LDDQU, MOVNTDQ, MOVNTPS,
; RCPSS, RCPPS, RSQRTSS, RSQRTPS
; Uses raw Linux syscalls only (no libc dependency)
%include "test_framework.inc"

section .data
    t1_name:  db "movddup xmm,xmm lo64", 0
    t2_name:  db "movddup xmm,xmm hi64", 0
    t3_name:  db "movddup xmm,m64 lo64", 0
    t4_name:  db "movddup xmm,m64 hi64", 0
    t5_name:  db "movsldup xmm,xmm lo64", 0
    t6_name:  db "movsldup xmm,xmm hi64", 0
    t7_name:  db "movsldup xmm,m128 lo64", 0
    t8_name:  db "movsldup xmm,m128 hi64", 0
    t9_name:  db "movshdup xmm,xmm lo64", 0
    t10_name: db "movshdup xmm,xmm hi64", 0
    t11_name: db "movshdup xmm,m128 lo64", 0
    t12_name: db "movshdup xmm,m128 hi64", 0
    t13_name: db "lddqu lo64", 0
    t14_name: db "lddqu hi64", 0
    t15_name: db "movntdq lo64", 0
    t16_name: db "movntdq hi64", 0
    t17_name: db "movntps lo64", 0
    t18_name: db "movntps hi64", 0
    t19_name: db "rcpss(1.0) lo32", 0
    t20_name: db "rcpss(1.0) preserves hi96", 0
    t21_name: db "rcpps([1,1,1,1]) lo64", 0
    t22_name: db "rcpps([1,1,1,1]) hi64", 0
    t23_name: db "rsqrtss(1.0) lo32", 0
    t24_name: db "rsqrtss(1.0) preserves hi96", 0
    t25_name: db "rsqrtps([1,1,1,1]) lo64", 0
    t26_name: db "rsqrtps([1,1,1,1]) hi64", 0
    t27_name: db "movddup zero lo64", 0
    t28_name: db "movddup zero hi64", 0
    t29_name: db "movsldup ident lo64", 0
    t30_name: db "movshdup ident lo64", 0

    align 16
    ; Double-precision data for MOVDDUP tests
    ; {2.0, 3.0} = {0x4000000000000000, 0x4008000000000000}
    vec_pd_23:  dq 0x4000000000000000, 0x4008000000000000

    ; Single-precision data for MOVSLDUP/MOVSHDUP tests
    ; {1.0f, 2.0f, 3.0f, 4.0f} = {0x3F800000, 0x40000000, 0x40400000, 0x40800000}
    vec_ps_1234: dd 0x3F800000, 0x40000000, 0x40400000, 0x40800000

    ; Data for LDDQU test (known 16-byte pattern)
    align 16
    lddqu_src: dq 0xDEADBEEFCAFEBABE, 0x0123456789ABCDEF

    ; Data for non-temporal store tests (source pattern)
    align 16
    nt_src_int: dq 0xAAAABBBBCCCCDDDD, 0x1111222233334444
    nt_src_ps:  dq 0x40000000_3F800000, 0x40800000_40400000  ; {1.0f,2.0f,3.0f,4.0f}

    ; All 1.0f for RCPPS/RSQRTPS tests
    align 16
    vec_ps_ones: dd 0x3F800000, 0x3F800000, 0x3F800000, 0x3F800000

    ; Destination value for RCPSS/RSQRTSS preserve-upper tests
    ; {junk, 0xDEAD, 0xBEEF, 0xCAFE} -- we set specific upper dwords
    align 16
    vec_rcpss_dst: dd 0x3F800000, 0x11111111, 0x22222222, 0x33333333

    ; Scalar 1.0f for RCPSS/RSQRTSS
    val_1f: dd 0x3F800000

    ; Zero double for MOVDDUP zero test
    val_zero_d: dq 0x0000000000000000

    ; Identical-pair test data for MOVSLDUP/MOVSHDUP
    ; {5.0f, 5.0f, 7.0f, 7.0f} = each pair same
    align 16
    vec_ps_5577: dd 0x40A00000, 0x40A00000, 0x40E00000, 0x40E00000

section .bss
    ; 16-byte aligned buffers for non-temporal store tests
    alignb 16
    nt_dst_int: resb 16
    nt_dst_ps:  resb 16

section .text
global _start

_start:
    INIT_TESTS

    ; ==== Test 1-2: MOVDDUP xmm, xmm ====
    ; Input xmm: lo64=0x4000000000000000 (2.0), hi64=0x4008000000000000 (3.0)
    ; Result: both halves = lo64 of source = 0x4000000000000000
    TEST_CASE t1_name
    movapd xmm0, [rel vec_pd_23]
    movddup xmm1, xmm0
    movq rax, xmm1
    CHECK_EQ_64 rax, 0x4000000000000000

    TEST_CASE t2_name
    pextrq rax, xmm1, 1
    CHECK_EQ_64 rax, 0x4000000000000000

    ; ==== Test 3-4: MOVDDUP xmm, m64 ====
    ; Load 64-bit from memory, duplicate to both halves
    ; Source memory = 0x4000000000000000 (first qword of vec_pd_23)
    TEST_CASE t3_name
    movddup xmm2, [rel vec_pd_23]
    movq rax, xmm2
    CHECK_EQ_64 rax, 0x4000000000000000

    TEST_CASE t4_name
    pextrq rax, xmm2, 1
    CHECK_EQ_64 rax, 0x4000000000000000

    ; ==== Test 5-6: MOVSLDUP xmm, xmm ====
    ; MOVSLDUP selects [0,0,2,2] (duplicates even-indexed dwords)
    ; Input: [0x3F800000, 0x40000000, 0x40400000, 0x40800000] = [1.0, 2.0, 3.0, 4.0]
    ; Result: [0x3F800000, 0x3F800000, 0x40400000, 0x40400000]
    ; lo64 = 0x3F800000_3F800000, hi64 = 0x40400000_40400000
    TEST_CASE t5_name
    movaps xmm0, [rel vec_ps_1234]
    movsldup xmm1, xmm0
    movq rax, xmm1
    CHECK_EQ_64 rax, 0x3F8000003F800000

    TEST_CASE t6_name
    pextrq rax, xmm1, 1
    CHECK_EQ_64 rax, 0x4040000040400000

    ; ==== Test 7-8: MOVSLDUP xmm, m128 ====
    TEST_CASE t7_name
    movsldup xmm2, [rel vec_ps_1234]
    movq rax, xmm2
    CHECK_EQ_64 rax, 0x3F8000003F800000

    TEST_CASE t8_name
    pextrq rax, xmm2, 1
    CHECK_EQ_64 rax, 0x4040000040400000

    ; ==== Test 9-10: MOVSHDUP xmm, xmm ====
    ; MOVSHDUP selects [1,1,3,3] (duplicates odd-indexed dwords)
    ; Input: [1.0, 2.0, 3.0, 4.0]
    ; Result: [0x40000000, 0x40000000, 0x40800000, 0x40800000]
    ; lo64 = 0x40000000_40000000, hi64 = 0x40800000_40800000
    TEST_CASE t9_name
    movaps xmm0, [rel vec_ps_1234]
    movshdup xmm1, xmm0
    movq rax, xmm1
    CHECK_EQ_64 rax, 0x4000000040000000

    TEST_CASE t10_name
    pextrq rax, xmm1, 1
    CHECK_EQ_64 rax, 0x4080000040800000

    ; ==== Test 11-12: MOVSHDUP xmm, m128 ====
    TEST_CASE t11_name
    movshdup xmm2, [rel vec_ps_1234]
    movq rax, xmm2
    CHECK_EQ_64 rax, 0x4000000040000000

    TEST_CASE t12_name
    pextrq rax, xmm2, 1
    CHECK_EQ_64 rax, 0x4080000040800000

    ; ==== Test 13-14: LDDQU xmm, m128 ====
    ; Load 16 bytes from aligned memory, verify contents
    ; Source: lo64=0xDEADBEEFCAFEBABE, hi64=0x0123456789ABCDEF
    TEST_CASE t13_name
    lddqu xmm0, [rel lddqu_src]
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xDEADBEEFCAFEBABE

    TEST_CASE t14_name
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x0123456789ABCDEF

    ; ==== Test 15-16: MOVNTDQ m128, xmm ====
    ; Store xmm to 16-byte aligned memory with non-temporal hint, load back and verify
    TEST_CASE t15_name
    movaps xmm0, [rel nt_src_int]
    movntdq [rel nt_dst_int], xmm0
    ; Fence to ensure the non-temporal store is visible
    sfence
    ; Load back to verify
    movaps xmm1, [rel nt_dst_int]
    movq rax, xmm1
    CHECK_EQ_64 rax, 0xAAAABBBBCCCCDDDD

    TEST_CASE t16_name
    pextrq rax, xmm1, 1
    CHECK_EQ_64 rax, 0x1111222233334444

    ; ==== Test 17-18: MOVNTPS m128, xmm ====
    ; Store packed singles with non-temporal hint, load back and verify
    TEST_CASE t17_name
    movaps xmm0, [rel nt_src_ps]
    movntps [rel nt_dst_ps], xmm0
    sfence
    movaps xmm1, [rel nt_dst_ps]
    movq rax, xmm1
    CHECK_EQ_64 rax, 0x400000003F800000

    TEST_CASE t18_name
    pextrq rax, xmm1, 1
    CHECK_EQ_64 rax, 0x4080000040400000

    ; ==== Test 19-20: RCPSS xmm, xmm ====
    ; RCPSS: approximate reciprocal of lowest single, upper 3 dwords from dest unchanged
    ; Set dest xmm0 = [0x3F800000, 0x11111111, 0x22222222, 0x33333333]
    ; Set src  xmm1 = [1.0f, ...]
    ; rcpss xmm0, xmm1 -> xmm0 lo32 = rcp(1.0) = 1.0 = 0x3F800000
    ; Upper 96 bits remain: [0x11111111, 0x22222222, 0x33333333]
    TEST_CASE t19_name
    movaps xmm0, [rel vec_rcpss_dst]
    movss xmm1, [rel val_1f]
    rcpss xmm0, xmm1
    movd eax, xmm0
    CHECK_EQ_32 eax, 0x3F800000

    TEST_CASE t20_name
    ; Check upper 96 bits preserved: dwords [1]=0x11111111, [2]=0x22222222, [3]=0x33333333
    ; hi64 should be 0x33333333_22222222
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x3333333322222222

    ; ==== Test 21-22: RCPPS xmm, xmm ====
    ; All four lanes: rcp(1.0) = 1.0
    ; Input: [1.0, 1.0, 1.0, 1.0]
    ; Expected: [0x3F800000, 0x3F800000, 0x3F800000, 0x3F800000]
    ; lo64 = 0x3F800000_3F800000, hi64 = 0x3F800000_3F800000
    TEST_CASE t21_name
    movaps xmm0, [rel vec_ps_ones]
    rcpps xmm1, xmm0
    movq rax, xmm1
    CHECK_EQ_64 rax, 0x3F8000003F800000

    TEST_CASE t22_name
    pextrq rax, xmm1, 1
    CHECK_EQ_64 rax, 0x3F8000003F800000

    ; ==== Test 23-24: RSQRTSS xmm, xmm ====
    ; rsqrt(1.0) = 1/sqrt(1.0) = 1.0 = 0x3F800000
    ; Set dest xmm0 = [0x3F800000, 0x11111111, 0x22222222, 0x33333333]
    ; rsqrtss xmm0, xmm1 where xmm1 lo32 = 1.0
    ; Result: xmm0 lo32 = 0x3F800000, upper 96 unchanged
    TEST_CASE t23_name
    movaps xmm0, [rel vec_rcpss_dst]
    movss xmm1, [rel val_1f]
    rsqrtss xmm0, xmm1
    movd eax, xmm0
    CHECK_EQ_32 eax, 0x3F800000

    TEST_CASE t24_name
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x3333333322222222

    ; ==== Test 25-26: RSQRTPS xmm, xmm ====
    ; All four lanes: rsqrt(1.0) = 1.0
    ; Input: [1.0, 1.0, 1.0, 1.0]
    ; Expected: [0x3F800000, 0x3F800000, 0x3F800000, 0x3F800000]
    TEST_CASE t25_name
    movaps xmm0, [rel vec_ps_ones]
    rsqrtps xmm1, xmm0
    movq rax, xmm1
    CHECK_EQ_64 rax, 0x3F8000003F800000

    TEST_CASE t26_name
    pextrq rax, xmm1, 1
    CHECK_EQ_64 rax, 0x3F8000003F800000

    ; ==== Test 27-28: MOVDDUP with zero ====
    ; Source: lo64=0, hi64=don't care
    ; Result: both halves = 0
    TEST_CASE t27_name
    pxor xmm3, xmm3           ; zero xmm3
    mov rax, 0xFFFFFFFFFFFFFFFF
    movq xmm0, rax             ; lo64 = all ones
    pinsrq xmm0, rax, 1       ; hi64 = all ones, xmm0 = all ones
    movddup xmm1, xmm3        ; duplicate lo64(0) to both halves
    movq rax, xmm1
    CHECK_EQ_64 rax, 0x0000000000000000

    TEST_CASE t28_name
    pextrq rax, xmm1, 1
    CHECK_EQ_64 rax, 0x0000000000000000

    ; ==== Test 29: MOVSLDUP with identical pairs ====
    ; Input: [5.0, 5.0, 7.0, 7.0] -> MOVSLDUP -> [5.0, 5.0, 7.0, 7.0] (same)
    ; lo64 = 0x40A00000_40A00000
    TEST_CASE t29_name
    movaps xmm0, [rel vec_ps_5577]
    movsldup xmm1, xmm0
    movq rax, xmm1
    CHECK_EQ_64 rax, 0x40A0000040A00000

    ; ==== Test 30: MOVSHDUP with identical pairs ====
    ; Input: [5.0, 5.0, 7.0, 7.0] -> MOVSHDUP -> [5.0, 5.0, 7.0, 7.0] (same)
    ; lo64 = 0x40A00000_40A00000
    TEST_CASE t30_name
    movaps xmm0, [rel vec_ps_5577]
    movshdup xmm1, xmm0
    movq rax, xmm1
    CHECK_EQ_64 rax, 0x40A0000040A00000

    END_TESTS
