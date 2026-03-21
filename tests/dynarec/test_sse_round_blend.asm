; test_sse_round_blend.asm - Test SSE4.1 rounding, blending, and dot product
; ROUNDSS, ROUNDSD, ROUNDPS, ROUNDPD (imm8: 0=nearest, 1=floor, 2=ceil, 3=trunc)
; BLENDPS, BLENDPD, PBLENDW
; BLENDVPS, BLENDVPD, PBLENDVB (variable blend using XMM0 as mask)
; DPPS, DPPD
%include "test_framework.inc"

section .data
    t1_name:  db "roundss nearest 2.5", 0
    t2_name:  db "roundss floor 2.7", 0
    t3_name:  db "roundss ceil 2.3", 0
    t4_name:  db "roundss trunc -2.7", 0
    t5_name:  db "roundsd nearest 3.5", 0
    t6_name:  db "roundsd floor 3.7", 0
    t7_name:  db "roundsd ceil -1.2", 0
    t8_name:  db "roundsd trunc -3.9", 0
    t9_name:  db "roundps floor packed", 0
    t10_name: db "roundpd ceil packed", 0
    t11_name: db "blendps imm=0x5", 0
    t12_name: db "blendps imm=0xF", 0
    t13_name: db "blendpd imm=0x2", 0
    t14_name: db "pblendw imm=0xF0", 0
    t15_name: db "dpps full dot", 0
    t16_name: db "dppd dot product", 0
    t17_name: db "blendvps xmm0 mask", 0
    t18_name: db "blendvpd xmm0 mask", 0
    t19_name: db "pblendvb xmm0 mask", 0
    t20_name: db "roundss nearest -0.5", 0
    t21_name: db "roundss nearest 0.5", 0
    t22_name: db "roundps trunc packed", 0
    t23_name: db "dpps partial mask", 0
    t24_name: db "blendps imm=0x0", 0
    t25_name: db "pblendw imm=0xAA", 0
    t26_name: db "roundsd nearest 4.5", 0
    t27_name: db "roundpd floor packed", 0
    t28_name: db "blendvps all select", 0
    t29_name: db "dppd single lane", 0
    t30_name: db "roundss floor -0.1", 0

    align 16
    ; Single floats
    f_2_5:    dd 2.5, 0.0, 0.0, 0.0
    f_2_7:    dd 2.7, 0.0, 0.0, 0.0
    f_2_3:    dd 2.3, 0.0, 0.0, 0.0
    f_n2_7:   dd 0xC02CCCCD, 0, 0, 0          ; -2.7f
    f_n0_5:   dd 0xBF000000, 0, 0, 0          ; -0.5f
    f_0_5:    dd 0x3F000000, 0, 0, 0          ; 0.5f
    f_n0_1:   dd 0xBDCCCCCD, 0, 0, 0          ; -0.1f
    f_pack1:  dd 1.3, 2.7, -0.5, 3.9
    f_pack2:  dd 10.0, 20.0, 30.0, 40.0
    f_pack3:  dd 100.0, 200.0, 300.0, 400.0

    ; Doubles
    d_3_5:    dq 3.5, 0.0
    d_3_7:    dq 3.7, 0.0
    d_n1_2:   dq -1.2, 0.0
    d_n3_9:   dq -3.9, 0.0
    d_4_5:    dq 4.5, 0.0
    d_pack1:  dq 1.7, -2.3
    d_pack2:  dq 3.0, 4.0
    d_pack3:  dq 5.0, 6.0

    ; For dot product
    f_dp1:    dd 1.0, 2.0, 3.0, 4.0
    f_dp2:    dd 5.0, 6.0, 7.0, 8.0
    d_dp1:    dq 3.0, 4.0
    d_dp2:    dq 5.0, 6.0

    ; Blend masks (for variable blend, sign bit selects)
    ; High bit set = select from src2
    blend_mask_ps: dd 0x80000000, 0x00000000, 0x80000000, 0x00000000  ; select 0,2 from src2
    blend_mask_pd: dq 0x0000000000000000, 0x8000000000000000          ; select qword 1 from src2
    blend_mask_pb: db 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00
                   db 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00
    blend_mask_all: dd 0x80000000, 0x80000000, 0x80000000, 0x80000000

    ; For pblendw
    w_src1:   dw 0x1111, 0x2222, 0x3333, 0x4444, 0x5555, 0x6666, 0x7777, 0x8888
    w_src2:   dw 0xAAAA, 0xBBBB, 0xCCCC, 0xDDDD, 0xEEEE, 0xFFFF, 0x9999, 0x0000

    ; Byte data for pblendvb
    b_src1:   db 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
              db 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10
    b_src2:   db 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8
              db 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xB0

section .text
global _start

_start:
    INIT_TESTS

    ; ==== Test 1: ROUNDSS nearest, 2.5 -> 2.0 (banker's rounding) ====
    TEST_CASE t1_name
    movss xmm1, [rel f_2_5]
    roundss xmm0, xmm1, 0        ; nearest
    movd eax, xmm0
    ; 2.5 rounds to 2.0 (banker's rounding: round to even)
    ; 2.0f = 0x40000000
    CHECK_EQ_32 eax, 0x40000000

    ; ==== Test 2: ROUNDSS floor, 2.7 -> 2.0 ====
    TEST_CASE t2_name
    movss xmm1, [rel f_2_7]
    roundss xmm0, xmm1, 1        ; floor
    movd eax, xmm0
    CHECK_EQ_32 eax, 0x40000000  ; 2.0f

    ; ==== Test 3: ROUNDSS ceil, 2.3 -> 3.0 ====
    TEST_CASE t3_name
    movss xmm1, [rel f_2_3]
    roundss xmm0, xmm1, 2        ; ceil
    movd eax, xmm0
    CHECK_EQ_32 eax, 0x40400000  ; 3.0f

    ; ==== Test 4: ROUNDSS trunc, -2.7 -> -2.0 ====
    TEST_CASE t4_name
    movss xmm1, [rel f_n2_7]
    roundss xmm0, xmm1, 3        ; trunc
    movd eax, xmm0
    CHECK_EQ_32 eax, 0xC0000000  ; -2.0f

    ; ==== Test 5: ROUNDSD nearest, 3.5 -> 4.0 (banker's: round to even) ====
    TEST_CASE t5_name
    movsd xmm1, [rel d_3_5]
    roundsd xmm0, xmm1, 0
    movq rax, xmm0
    ; 3.5 rounds to 4.0 (nearest even)
    CHECK_EQ_64 rax, 0x4010000000000000  ; 4.0

    ; ==== Test 6: ROUNDSD floor, 3.7 -> 3.0 ====
    TEST_CASE t6_name
    movsd xmm1, [rel d_3_7]
    roundsd xmm0, xmm1, 1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x4008000000000000  ; 3.0

    ; ==== Test 7: ROUNDSD ceil, -1.2 -> -1.0 ====
    TEST_CASE t7_name
    movsd xmm1, [rel d_n1_2]
    roundsd xmm0, xmm1, 2
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xBFF0000000000000  ; -1.0

    ; ==== Test 8: ROUNDSD trunc, -3.9 -> -3.0 ====
    TEST_CASE t8_name
    movsd xmm1, [rel d_n3_9]
    roundsd xmm0, xmm1, 3
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xC008000000000000  ; -3.0

    ; ==== Test 9: ROUNDPS floor packed {1.3, 2.7, -0.5, 3.9} -> {1.0, 2.0, -1.0, 3.0} ====
    TEST_CASE t9_name
    movdqa xmm1, [rel f_pack1]
    roundps xmm0, xmm1, 1        ; floor
    movd eax, xmm0
    ; dword0 = floor(1.3) = 1.0 = 0x3F800000
    CHECK_EQ_32 eax, 0x3F800000

    ; ==== Test 10: ROUNDPD ceil packed {1.7, -2.3} -> {2.0, -2.0} ====
    TEST_CASE t10_name
    movdqa xmm1, [rel d_pack1]
    roundpd xmm0, xmm1, 2        ; ceil
    movq rax, xmm0
    ; qword0 = ceil(1.7) = 2.0 = 0x4000000000000000
    CHECK_EQ_64 rax, 0x4000000000000000

    ; ==== Test 11: BLENDPS imm=0x5 (select dwords 0,2 from src2) ====
    ; src1={10,20,30,40}, src2={100,200,300,400}
    ; imm=0101: dw0 from src2, dw1 from src1, dw2 from src2, dw3 from src1
    ; Result: {100, 20, 300, 40}
    ; Low 64: dw0=100.0=0x42C80000, dw1=20.0=0x41A00000 -> 0x41A0000042C80000
    TEST_CASE t11_name
    movdqa xmm0, [rel f_pack2]   ; {10,20,30,40}
    movdqa xmm1, [rel f_pack3]   ; {100,200,300,400}
    blendps xmm0, xmm1, 0x5
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x41A0000042C80000

    ; ==== Test 12: BLENDPS imm=0xF (all from src2) ====
    TEST_CASE t12_name
    movdqa xmm0, [rel f_pack2]
    movdqa xmm1, [rel f_pack3]
    blendps xmm0, xmm1, 0xF
    movq rax, xmm0
    ; Should be {100,200,...} = 0x4348000042C80000
    CHECK_EQ_64 rax, 0x4348000042C80000

    ; ==== Test 13: BLENDPD imm=0x2 (select qword 1 from src2) ====
    ; src1={3.0, 4.0}, src2={5.0, 6.0}
    ; imm=10: qw0 from src1, qw1 from src2
    ; Result: {3.0, 6.0}
    TEST_CASE t13_name
    movdqa xmm0, [rel d_pack2]
    movdqa xmm1, [rel d_pack3]
    blendpd xmm0, xmm1, 0x2
    movq rax, xmm0
    ; qw0 = 3.0 = 0x4008000000000000
    CHECK_EQ_64 rax, 0x4008000000000000

    ; ==== Test 14: PBLENDW imm=0xF0 (high 4 words from src2) ====
    ; w_src1 = {1111,2222,3333,4444,5555,6666,7777,8888}
    ; w_src2 = {AAAA,BBBB,CCCC,DDDD,EEEE,FFFF,9999,0000}
    ; imm=0xF0: words 0-3 from src1, words 4-7 from src2
    ; Result: {1111,2222,3333,4444,EEEE,FFFF,9999,0000}
    ; Low 64: 0x4444333322221111
    TEST_CASE t14_name
    movdqa xmm0, [rel w_src1]
    movdqa xmm1, [rel w_src2]
    pblendw xmm0, xmm1, 0xF0
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x4444333322221111

    ; ==== Test 15: DPPS full dot product ====
    ; dpps xmm0, xmm1, imm8
    ; imm8[7:4] = multiply mask, imm8[3:0] = result store mask
    ; imm=0xFF: multiply all 4 lanes, store result in all 4 dwords
    ; {1,2,3,4} . {5,6,7,8} = 1*5+2*6+3*7+4*8 = 5+12+21+32 = 70
    ; 70.0f = 0x428C0000
    TEST_CASE t15_name
    movdqa xmm0, [rel f_dp1]
    movdqa xmm1, [rel f_dp2]
    dpps xmm0, xmm1, 0xFF
    movd eax, xmm0
    CHECK_EQ_32 eax, 0x428C0000

    ; ==== Test 16: DPPD dot product ====
    ; {3.0, 4.0} . {5.0, 6.0} = 15+24 = 39.0
    ; imm=0x33: multiply both qwords, store in both qwords
    ; 39.0 = 0x4043800000000000
    TEST_CASE t16_name
    movdqa xmm0, [rel d_dp1]
    movdqa xmm1, [rel d_dp2]
    dppd xmm0, xmm1, 0x33
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x4043800000000000

    ; ==== Test 17: BLENDVPS (variable blend using xmm0 as mask) ====
    ; Mask = blend_mask_ps: select dwords 0,2 from src2
    ; src1 (xmm1) = {10,20,30,40}, src2 (xmm2) = {100,200,300,400}
    ; Need xmm0 = mask, then blendvps xmm1, xmm2, xmm0
    ; Intel syntax: blendvps xmm1, xmm2 (implicit xmm0 as mask)
    TEST_CASE t17_name
    movdqa xmm0, [rel blend_mask_ps]
    movdqa xmm1, [rel f_pack2]   ; dst
    movdqa xmm2, [rel f_pack3]   ; src
    blendvps xmm1, xmm2          ; implicit xmm0 mask
    ; dw0: mask bit set -> from src2 = 100.0, dw1: clear -> from src1 = 20.0
    ; dw2: set -> 300.0, dw3: clear -> 40.0
    movq rax, xmm1
    ; Low 64: dw0=100.0=0x42C80000, dw1=20.0=0x41A00000
    CHECK_EQ_64 rax, 0x41A0000042C80000

    ; ==== Test 18: BLENDVPD (variable blend using xmm0 as mask) ====
    TEST_CASE t18_name
    movdqa xmm0, [rel blend_mask_pd]
    movdqa xmm1, [rel d_pack2]   ; {3.0, 4.0}
    movdqa xmm2, [rel d_pack3]   ; {5.0, 6.0}
    blendvpd xmm1, xmm2          ; qw0: clear -> 3.0, qw1: set -> 6.0
    movq rax, xmm1
    CHECK_EQ_64 rax, 0x4008000000000000  ; 3.0

    ; ==== Test 19: PBLENDVB (variable byte blend using xmm0) ====
    TEST_CASE t19_name
    movdqa xmm0, [rel blend_mask_pb]  ; alternating: select even bytes from src2
    movdqa xmm1, [rel b_src1]
    movdqa xmm2, [rel b_src2]
    pblendvb xmm1, xmm2
    ; byte0: mask 0x80 -> src2=0xA1, byte1: mask 0x00 -> src1=0x02
    ; byte2: mask 0x80 -> src2=0xA3, byte3: mask 0x00 -> src1=0x04
    ; etc. Low 8 bytes: A1 02 A3 04 A5 06 A7 08
    ; LE qword: 0x08A706A504A302A1
    movq rax, xmm1
    CHECK_EQ_64 rax, 0x08A706A504A302A1

    ; ==== Test 20: ROUNDSS nearest, -0.5 -> 0.0 (banker's: round to even) ====
    TEST_CASE t20_name
    movss xmm1, [rel f_n0_5]
    roundss xmm0, xmm1, 0
    movd eax, xmm0
    ; -0.5 rounds to 0.0 (nearest even = 0) -> 0x00000000 or 0x80000000 (-0.0)
    ; Intel says: round to nearest even integer. -0.5 -> 0 (even). Result is -0.0 or 0.0.
    ; On x86, roundss(-0.5, nearest) = -0.0 = 0x80000000
    CHECK_EQ_32 eax, 0x80000000

    ; ==== Test 21: ROUNDSS nearest, 0.5 -> 0.0 (banker's: round to even) ====
    TEST_CASE t21_name
    movss xmm1, [rel f_0_5]
    roundss xmm0, xmm1, 0
    movd eax, xmm0
    ; 0.5 rounds to 0.0 (nearest even = 0)
    CHECK_EQ_32 eax, 0x00000000

    ; ==== Test 22: ROUNDPS trunc packed ====
    ; {1.3, 2.7, -0.5, 3.9} -> {1.0, 2.0, -0.0, 3.0}
    TEST_CASE t22_name
    movdqa xmm1, [rel f_pack1]
    roundps xmm0, xmm1, 3        ; trunc
    movd eax, xmm0
    ; dword0 = trunc(1.3) = 1.0 = 0x3F800000
    CHECK_EQ_32 eax, 0x3F800000

    ; ==== Test 23: DPPS partial mask ====
    ; imm=0x71: multiply dwords 0,1,2 only (bits 7,6,5 set, bit 4 clear), store to dword 0 only
    ; {1,2,3,4} . {5,6,7,_} = 1*5+2*6+3*7 = 5+12+21 = 38
    ; 38.0f = 0x42180000
    TEST_CASE t23_name
    movdqa xmm0, [rel f_dp1]
    movdqa xmm1, [rel f_dp2]
    dpps xmm0, xmm1, 0x71
    movd eax, xmm0
    CHECK_EQ_32 eax, 0x42180000

    ; ==== Test 24: BLENDPS imm=0x0 (all from src1, no blend) ====
    TEST_CASE t24_name
    movdqa xmm0, [rel f_pack2]   ; {10,20,30,40}
    movdqa xmm1, [rel f_pack3]
    blendps xmm0, xmm1, 0x0
    movq rax, xmm0
    ; All from src1: dw0=10.0=0x41200000, dw1=20.0=0x41A00000
    CHECK_EQ_64 rax, 0x41A0000041200000

    ; ==== Test 25: PBLENDW imm=0xAA (alternate words from src2) ====
    ; bits: 10101010 -> words 1,3,5,7 from src2
    ; Result: w0=1111, w1=BBBB, w2=3333, w3=DDDD, w4=5555, w5=FFFF, w6=7777, w7=0000
    ; Low 64: 0xDDDD3333BBBB1111
    TEST_CASE t25_name
    movdqa xmm0, [rel w_src1]
    movdqa xmm1, [rel w_src2]
    pblendw xmm0, xmm1, 0xAA
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xDDDD3333BBBB1111

    ; ==== Test 26: ROUNDSD nearest, 4.5 -> 4.0 (banker's) ====
    TEST_CASE t26_name
    movsd xmm1, [rel d_4_5]
    roundsd xmm0, xmm1, 0
    movq rax, xmm0
    ; 4.5 rounds to 4.0 (nearest even)
    CHECK_EQ_64 rax, 0x4010000000000000  ; 4.0

    ; ==== Test 27: ROUNDPD floor packed {1.7, -2.3} -> {1.0, -3.0} ====
    TEST_CASE t27_name
    movdqa xmm1, [rel d_pack1]
    roundpd xmm0, xmm1, 1        ; floor
    movq rax, xmm0
    ; qw0 = floor(1.7) = 1.0 = 0x3FF0000000000000
    CHECK_EQ_64 rax, 0x3FF0000000000000

    ; ==== Test 28: BLENDVPS all selected from src2 ====
    TEST_CASE t28_name
    movdqa xmm0, [rel blend_mask_all]
    movdqa xmm1, [rel f_pack2]
    movdqa xmm2, [rel f_pack3]
    blendvps xmm1, xmm2
    movq rax, xmm1
    ; All from src2: {100,200,300,400}
    CHECK_EQ_64 rax, 0x4348000042C80000

    ; ==== Test 29: DPPD single lane ====
    ; imm=0x11: multiply qw0 only, store in qw0 only
    ; {3.0, 4.0} . {5.0, _} = 3*5 = 15.0
    ; 15.0 = 0x402E000000000000
    TEST_CASE t29_name
    movdqa xmm0, [rel d_dp1]
    movdqa xmm1, [rel d_dp2]
    dppd xmm0, xmm1, 0x11
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x402E000000000000

    ; ==== Test 30: ROUNDSS floor, -0.1 -> -1.0 ====
    TEST_CASE t30_name
    movss xmm1, [rel f_n0_1]
    roundss xmm0, xmm1, 1        ; floor
    movd eax, xmm0
    ; floor(-0.1) = -1.0 = 0xBF800000
    CHECK_EQ_32 eax, 0xBF800000

    END_TESTS
