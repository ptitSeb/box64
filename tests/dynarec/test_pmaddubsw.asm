; test_pmaddubsw.asm - Test PMADDUBSW (SSSE3) for both MMX and SSE forms
; Opcode 0F 38 04 (MMX) and 66 0F 38 04 (SSE)
; result[i] = sat_s16(src1_u8[2i]*src2_s8[2i] + src1_u8[2i+1]*src2_s8[2i+1])
%include "test_framework.inc"

section .data
    t1_name:  db "pmaddubsw mm basic", 0
    t2_name:  db "pmaddubsw mm signed", 0
    t3_name:  db "pmaddubsw mm pos sat", 0
    t4_name:  db "pmaddubsw mm neg sat", 0
    t5_name:  db "pmaddubsw mm zeros", 0
    t6_name:  db "pmaddubsw mm same reg", 0
    t7_name:  db "pmaddubsw xmm basic", 0
    t8_name:  db "pmaddubsw xmm sat", 0
    t9_name:  db "pmaddubsw xmm same reg", 0
    t10_name: db "pmaddubsw mm max u8*s8", 0

    align 8
    ; MMX test data (64-bit / 8 bytes each)
    ; Test 1: basic, no saturation
    ; Gm = [1, 2, 3, 4, 5, 6, 7, 8] (unsigned bytes)
    ; Em = [1, 2, 3, 4, 5, 6, 7, 8] (signed bytes)
    ; result[0] = 1*1 + 2*2 = 5
    ; result[1] = 3*3 + 4*4 = 25
    ; result[2] = 5*5 + 6*6 = 61
    ; result[3] = 7*7 + 8*8 = 113
    ; expected = 0x0071003D00190005
    mm_basic_a: db 1, 2, 3, 4, 5, 6, 7, 8
    mm_basic_b: db 1, 2, 3, 4, 5, 6, 7, 8

    ; Test 2: signed second operand
    ; Gm = [100, 200, 50, 150, 10, 20, 30, 40] (unsigned)
    ; Em = [-1, 2, -3, 4, -5, 6, -7, 8] (signed)
    ; result[0] = 100*(-1) + 200*2 = -100+400 = 300 = 0x012C
    ; result[1] = 50*(-3) + 150*4 = -150+600 = 450 = 0x01C2
    ; result[2] = 10*(-5) + 20*6 = -50+120 = 70 = 0x0046
    ; result[3] = 30*(-7) + 40*8 = -210+320 = 110 = 0x006E
    mm_signed_a: db 100, 200, 50, 150, 10, 20, 30, 40
    mm_signed_b: db 0xFF, 2, 0xFD, 4, 0xFB, 6, 0xF9, 8

    ; Test 3: positive saturation
    ; Gm = [255, 255, 0, 0, 0, 0, 0, 0] (unsigned)
    ; Em = [127, 127, 0, 0, 0, 0, 0, 0] (signed)
    ; result[0] = 255*127 + 255*127 = 32385+32385 = 64770 → sat to 32767 = 0x7FFF
    ; result[1] = 0
    ; result[2] = 0
    ; result[3] = 0
    mm_possat_a: db 255, 255, 0, 0, 0, 0, 0, 0
    mm_possat_b: db 127, 127, 0, 0, 0, 0, 0, 0

    ; Test 4: negative saturation
    ; Gm = [255, 255, 0, 0, 0, 0, 0, 0] (unsigned)
    ; Em = [-128, -128, 0, 0, 0, 0, 0, 0] (signed = 0x80)
    ; result[0] = 255*(-128) + 255*(-128) = -32640+(-32640) = -65280 → sat to -32768 = 0x8000
    ; result[1..3] = 0
    mm_negsat_a: db 255, 255, 0, 0, 0, 0, 0, 0
    mm_negsat_b: db 0x80, 0x80, 0, 0, 0, 0, 0, 0

    ; Test 5: all zeros
    mm_zeros: dq 0

    ; Test 10: max unsigned * max positive signed
    ; Gm = [255, 0, 0, 255, 128, 128, 1, 1]
    ; Em = [127, 0, 0, 127, 1, -1, 127, -128]
    ; result[0] = 255*127 + 0*0 = 32385 = 0x7E81
    ; result[1] = 0*0 + 255*127 = 32385 = 0x7E81
    ; result[2] = 128*1 + 128*(-1) = 128+(-128) = 0
    ; result[3] = 1*127 + 1*(-128) = 127+(-128) = -1 = 0xFFFF
    mm_mix_a: db 255, 0, 0, 255, 128, 128, 1, 1
    mm_mix_b: db 127, 0, 0, 127, 1, 0xFF, 127, 0x80

    align 16
    ; SSE test data (128-bit / 16 bytes each)
    ; Test 7: basic SSE
    ; Gx = [1,2, 3,4, 5,6, 7,8, 9,10, 11,12, 13,14, 15,16]
    ; Ex = [1,1, 1,1, 1,1, 1,1, 2,2,  2,2,   2,2,   2,2]
    ; result[0] = 1*1+2*1 = 3, [1]=3+4=7, [2]=5+6=11, [3]=7+8=15
    ; result[4] = 9*2+10*2=38, [5]=11*2+12*2=46, [6]=13*2+14*2=54, [7]=15*2+16*2=62
    ; low64  = 0x000F000B00070003
    ; high64 = 0x003E003600260026  -- wait let me recalculate
    ; high64: 38=0x26, 46=0x2E, 54=0x36, 62=0x3E
    ; high64 = 0x003E003600260026 -- no, rethink byte order
    ; In memory (LE): result[0]=3 at offset 0-1, result[1]=7 at offset 2-3, etc.
    ; low 64 bits (qword 0):  result[0..3] = 3,7,11,15 → 0x000F000B00070003
    ; high 64 bits (qword 1): result[4..7] = 38,46,54,62 → 0x003E00360002E0026
    ; oops typo: 38=0x0026, 46=0x002E, 54=0x0036, 62=0x003E
    ; high64 = 0x003E0036002E0026
    xmm_basic_a: db 1,2, 3,4, 5,6, 7,8, 9,10, 11,12, 13,14, 15,16
    xmm_basic_b: db 1,1, 1,1, 1,1, 1,1, 2,2,  2,2,   2,2,   2,2

    ; Test 8: SSE with saturation in both directions
    ; Gx = [255,255, 255,255, 0,0, 0,0, 128,128, 1,1, 10,20, 200,100]
    ; Ex = [127,127, -128,-128, 0,0, 0,0, 1,-1, 127,-128, 5,5, -1,-1]
    ; result[0] = 255*127+255*127 = 64770 → sat 32767 = 0x7FFF
    ; result[1] = 255*(-128)+255*(-128) = -65280 → sat -32768 = 0x8000
    ; result[2] = 0
    ; result[3] = 0
    ; result[4] = 128*1+128*(-1) = 0
    ; result[5] = 1*127+1*(-128) = -1 = 0xFFFF
    ; result[6] = 10*5+20*5 = 150 = 0x0096
    ; result[7] = 200*(-1)+100*(-1) = -300 = 0xFED4
    xmm_sat_a: db 255,255, 255,255, 0,0, 0,0, 128,128, 1,1, 10,20, 200,100
    xmm_sat_b: db 127,127, 0x80,0x80, 0,0, 0,0, 1,0xFF, 127,0x80, 5,5, 0xFF,0xFF

section .text
global _start
_start:
    INIT_TESTS

    ; ===== Test 1: MMX basic =====
    TEST_CASE t1_name
    movq mm0, [rel mm_basic_a]
    movq mm1, [rel mm_basic_b]
    pmaddubsw mm0, mm1
    movq rax, mm0
    CHECK_EQ_64 rax, 0x0071003D00190005

    ; ===== Test 2: MMX signed =====
    TEST_CASE t2_name
    movq mm0, [rel mm_signed_a]
    movq mm1, [rel mm_signed_b]
    pmaddubsw mm0, mm1
    movq rax, mm0
    CHECK_EQ_64 rax, 0x006E004601C2012C

    ; ===== Test 3: MMX positive saturation =====
    TEST_CASE t3_name
    movq mm0, [rel mm_possat_a]
    movq mm1, [rel mm_possat_b]
    pmaddubsw mm0, mm1
    movq rax, mm0
    CHECK_EQ_64 rax, 0x000000000000_7FFF

    ; ===== Test 4: MMX negative saturation =====
    TEST_CASE t4_name
    movq mm0, [rel mm_negsat_a]
    movq mm1, [rel mm_negsat_b]
    pmaddubsw mm0, mm1
    movq rax, mm0
    CHECK_EQ_64 rax, 0x000000000000_8000

    ; ===== Test 5: MMX zeros =====
    TEST_CASE t5_name
    movq mm0, [rel mm_zeros]
    movq mm1, [rel mm_zeros]
    pmaddubsw mm0, mm1
    movq rax, mm0
    CHECK_EQ_64 rax, 0

    ; ===== Test 6: MMX same register =====
    ; mm0 = mm0 op mm0: Gm unsigned, Em signed interpretation of same data
    ; data = [1, 2, 3, 4, 5, 6, 7, 8]
    ; As unsigned: [1,2,3,4,5,6,7,8]
    ; As signed:   [1,2,3,4,5,6,7,8] (all positive, same values)
    ; result = same as test 1
    TEST_CASE t6_name
    movq mm0, [rel mm_basic_a]
    pmaddubsw mm0, mm0
    movq rax, mm0
    CHECK_EQ_64 rax, 0x0071003D00190005

    ; ===== Test 7: SSE basic =====
    TEST_CASE t7_name
    movdqa xmm0, [rel xmm_basic_a]
    movdqa xmm1, [rel xmm_basic_b]
    pmaddubsw xmm0, xmm1
    ; Check low 64 bits
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x000F000B00070003

    ; ===== Test 8: SSE saturation =====
    TEST_CASE t8_name
    movdqa xmm0, [rel xmm_sat_a]
    movdqa xmm1, [rel xmm_sat_b]
    pmaddubsw xmm0, xmm1
    ; Check low 64 bits: [0x7FFF, 0x8000, 0, 0]
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x000000008000_7FFF

    ; ===== Test 9: SSE same register =====
    TEST_CASE t9_name
    movdqa xmm0, [rel xmm_basic_a]
    pmaddubsw xmm0, xmm0
    ; data = [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16]
    ; As u8: same. As s8: same (all <= 127).
    ; result[0]=1*1+2*2=5, [1]=3*3+4*4=25, [2]=5*5+6*6=61, [3]=7*7+8*8=113
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0071003D00190005

    ; ===== Test 10: MMX mixed values =====
    TEST_CASE t10_name
    movq mm0, [rel mm_mix_a]
    movq mm1, [rel mm_mix_b]
    pmaddubsw mm0, mm1
    movq rax, mm0
    ; result[0]=32385=0x7E81, [1]=32385=0x7E81, [2]=0, [3]=-1=0xFFFF
    CHECK_EQ_64 rax, 0xFFFF00007E817E81

    emms
    END_TESTS
