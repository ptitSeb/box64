; test_sse_misc_int.asm - Test miscellaneous SSE integer multiply, pack, abs, sign
; PMADDWD, PMADDUBSW, PMULHRSW, PABS, PSIGN, PACKUSDW, PMULHW, PMULHUW, PMULUDQ
%include "test_framework.inc"

section .data
    t1_name:  db "pmaddwd basic lo", 0
    t2_name:  db "pmaddwd basic hi", 0
    t3_name:  db "pmaddwd neg lo", 0
    t4_name:  db "pmaddwd neg hi", 0
    t5_name:  db "pmaddubsw basic lo", 0
    t6_name:  db "pmaddubsw basic hi", 0
    t7_name:  db "pmaddubsw sat lo", 0
    t8_name:  db "pmaddubsw sat hi", 0
    t9_name:  db "pmulhrsw basic lo", 0
    t10_name: db "pmulhrsw basic hi", 0
    t11_name: db "pabsb lo", 0
    t12_name: db "pabsb hi", 0
    t13_name: db "pabsw lo", 0
    t14_name: db "pabsw hi", 0
    t15_name: db "pabsd lo", 0
    t16_name: db "pabsd hi", 0
    t17_name: db "psignb lo", 0
    t18_name: db "psignb hi", 0
    t19_name: db "psignw lo", 0
    t20_name: db "psignw hi", 0
    t21_name: db "psignd lo", 0
    t22_name: db "psignd hi", 0
    t23_name: db "packusdw lo", 0
    t24_name: db "packusdw hi", 0
    t25_name: db "pmulhw lo", 0
    t26_name: db "pmulhw hi", 0
    t27_name: db "pmulhuw lo", 0
    t28_name: db "pmulhuw hi", 0
    t29_name: db "pmuludq lo", 0
    t30_name: db "pmuludq hi", 0
    t31_name: db "pmaddwd max lo", 0
    t32_name: db "pmaddwd max hi", 0
    t33_name: db "pmulhrsw neg lo", 0
    t34_name: db "pmulhrsw neg hi", 0

    align 16
    ; PMADDWD basic: A=[1,2,3,4,5,6,7,8] B=[10,20,30,40,50,60,70,80]
    vec_madd_a:     dw 1, 2, 3, 4, 5, 6, 7, 8
    vec_madd_b:     dw 10, 20, 30, 40, 50, 60, 70, 80

    ; PMADDWD negative: A=[0x7FFF,0x8000,1,1, 0x7FFF,0x8000,1,1]
    ;                   B=[2,2,2,2, 2,2,2,2]
    align 16
    vec_madd_neg_a: dw 0x7FFF, 0x8000, 0x0001, 0x0001, 0x7FFF, 0x8000, 0x0001, 0x0001
    vec_madd_neg_b: dw 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002

    ; PMADDWD max: A=[0x7FFF,0x7FFF,0x8000,0x8000, 0,0,1,0]
    ;             B=[0x7FFF,0x7FFF,0x8000,0x8000, 0,0,0,0]
    ; dword[0] = 32767*32767 + 32767*32767 = 1073676289 + 1073676289 = 2147352578 = 0x7FFE0002
    ; dword[1] = (-32768)*(-32768) + (-32768)*(-32768) = 1073741824 + 1073741824 = 2147483648 = 0x80000000
    ; dword[2] = 0*0 + 0*0 = 0
    ; dword[3] = 1*0 + 0*0 = 0
    align 16
    vec_madd_max_a: dw 0x7FFF, 0x7FFF, 0x8000, 0x8000, 0x0000, 0x0000, 0x0001, 0x0000
    vec_madd_max_b: dw 0x7FFF, 0x7FFF, 0x8000, 0x8000, 0x0000, 0x0000, 0x0000, 0x0000

    ; PMADDUBSW basic:
    ; A (unsigned): [0xFF,0x01, 0x80,0x02, 0x10,0x10, 0x00,0x00, 0xFF,0x01, 0x80,0x02, 0x10,0x10, 0x00,0x00]
    ; B (signed):   [0x7F,0x7F, 0x01,0x7F, 0x01,0x01, 0x01,0x01, 0x7F,0x7F, 0x01,0x7F, 0x01,0x01, 0x01,0x01]
    align 16
    vec_maddubs_a:  db 0xFF, 0x01, 0x80, 0x02, 0x10, 0x10, 0x00, 0x00, 0xFF, 0x01, 0x80, 0x02, 0x10, 0x10, 0x00, 0x00
    vec_maddubs_b:  db 0x7F, 0x7F, 0x01, 0x7F, 0x01, 0x01, 0x01, 0x01, 0x7F, 0x7F, 0x01, 0x7F, 0x01, 0x01, 0x01, 0x01

    ; PMADDUBSW saturation: A=all 0xFF, B=all 0x7F
    align 16
    vec_maddubs_sat_a: times 16 db 0xFF
    vec_maddubs_sat_b: times 16 db 0x7F

    ; PMULHRSW: A=[0x4000,0x7FFF,0x0100,0x0001, 0x4000,0x7FFF,0x0100,0x0001]
    ; B same as A (squaring)
    align 16
    vec_pmulhrsw_a: dw 0x4000, 0x7FFF, 0x0100, 0x0001, 0x4000, 0x7FFF, 0x0100, 0x0001

    ; PMULHRSW negative test: A=[0xC000(-16384), 0x8001(-32767), 0xFF00(-256), 0xFFFF(-1),
    ;                            0xC000, 0x8001, 0xFF00, 0xFFFF]
    ; B = A (squaring)
    ; word[0]: (-16384)*(-16384) = 268435456. >>14=16384. +1=16385. >>1=8192=0x2000
    ; word[1]: (-32767)*(-32767) = 1073676289. >>14=65532. +1=65533. >>1=32766=0x7FFE
    ; word[2]: (-256)*(-256) = 65536. >>14=4. +1=5. >>1=2=0x0002
    ; word[3]: (-1)*(-1) = 1. >>14=0. +1=1. >>1=0=0x0000
    align 16
    vec_pmulhrsw_neg: dw 0xC000, 0x8001, 0xFF00, 0xFFFF, 0xC000, 0x8001, 0xFF00, 0xFFFF

    ; PABSB input: [0x00,0x01,0x7F,0x80, 0xFF,0xFE,0x01,0x81, 0x02,0xFD,0x40,0xC0, 0x10,0xF0,0x7E,0x82]
    align 16
    vec_absb_in:    db 0x00, 0x01, 0x7F, 0x80, 0xFF, 0xFE, 0x01, 0x81, 0x02, 0xFD, 0x40, 0xC0, 0x10, 0xF0, 0x7E, 0x82

    ; PABSW input: [0x0001, 0xFFFF, 0x7FFF, 0x8000, 0x8001, 0xFFFE, 0x0000, 0x0100]
    align 16
    vec_absw_in:    dw 0x0001, 0xFFFF, 0x7FFF, 0x8000, 0x8001, 0xFFFE, 0x0000, 0x0100

    ; PABSD input: [1, -1, 0x7FFFFFFF, 0x80000000]
    align 16
    vec_absd_in:    dd 0x00000001, 0xFFFFFFFF, 0x7FFFFFFF, 0x80000000

    ; PSIGNB inputs
    align 16
    vec_psignb_a:   db 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10
    vec_psignb_b:   db 0x01, 0xFF, 0x00, 0x7F, 0x80, 0x01, 0xFF, 0x00, 0x01, 0xFF, 0x00, 0x7F, 0x80, 0x01, 0xFF, 0x00

    ; PSIGNW inputs: A=[1,2,3,4,5,6,7,8], B=[1,-1,0,0x7FFF, 0x8000,1,-1,0]
    align 16
    vec_psignw_a:   dw 1, 2, 3, 4, 5, 6, 7, 8
    vec_psignw_b:   dw 1, 0xFFFF, 0, 0x7FFF, 0x8000, 1, 0xFFFF, 0

    ; PSIGND inputs: A=[1,2,3,4], B=[1,-1,0,0x7FFFFFFF]
    align 16
    vec_psignd_a:   dd 1, 2, 3, 4
    vec_psignd_b:   dd 1, 0xFFFFFFFF, 0, 0x7FFFFFFF

    ; PACKUSDW inputs (SSE4.1)
    ; A: [1, 0xFFFF, -1(0xFFFFFFFF), 0x10000]
    ; B: [0, 0x7FFF, 0x80000000, 0x0000FFFE]
    align 16
    vec_packusdw_a: dd 0x00000001, 0x0000FFFF, 0xFFFFFFFF, 0x00010000
    vec_packusdw_b: dd 0x00000000, 0x00007FFF, 0x80000000, 0x0000FFFE

    ; PMULHW / PMULHUW inputs (squaring)
    align 16
    vec_pmulh_a:    dw 0x0100, 0x7FFF, 0x8000, 0xFFFF, 0x0001, 0x1000, 0x4000, 0x2000

    ; PMULUDQ inputs
    ; A: [2, 0xDEAD, 0xFFFFFFFF, 0xBEEF]  (dwords 0 and 2 matter)
    ; B: [3, 0xCAFE, 0xFFFFFFFF, 0xFACE]
    align 16
    vec_pmuludq_a:  dd 0x00000002, 0x0000DEAD, 0xFFFFFFFF, 0x0000BEEF
    vec_pmuludq_b:  dd 0x00000003, 0x0000CAFE, 0xFFFFFFFF, 0x0000FACE

section .text
global _start

_start:
    INIT_TESTS

    ; ==== Test 1: pmaddwd basic lo64 ====
    ; dword[0] = 1*10+2*20 = 50 = 0x32
    ; dword[1] = 3*30+4*40 = 250 = 0xFA
    ; lo64 = 0x000000FA00000032
    TEST_CASE t1_name
    movdqa xmm0, [rel vec_madd_a]
    movdqa xmm1, [rel vec_madd_b]
    pmaddwd xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x000000FA00000032

    ; ==== Test 2: pmaddwd basic hi64 ====
    ; dword[2] = 5*50+6*60 = 610 = 0x262
    ; dword[3] = 7*70+8*80 = 1130 = 0x46A
    ; hi64 = 0x0000046A00000262
    TEST_CASE t2_name
    movdqa xmm0, [rel vec_madd_a]
    movdqa xmm1, [rel vec_madd_b]
    pmaddwd xmm0, xmm1
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x0000046A00000262

    ; ==== Test 3: pmaddwd negative lo64 ====
    ; dword[0] = 0x7FFF*2 + 0x8000(=-32768)*2 = 65534 + (-65536) = -2 = 0xFFFFFFFE
    ; dword[1] = 1*2 + 1*2 = 4
    ; lo64 = 0x00000004FFFFFFFE
    TEST_CASE t3_name
    movdqa xmm0, [rel vec_madd_neg_a]
    movdqa xmm1, [rel vec_madd_neg_b]
    pmaddwd xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x00000004FFFFFFFE

    ; ==== Test 4: pmaddwd negative hi64 ====
    ; Same pattern repeats: hi64 = 0x00000004FFFFFFFE
    TEST_CASE t4_name
    movdqa xmm0, [rel vec_madd_neg_a]
    movdqa xmm1, [rel vec_madd_neg_b]
    pmaddwd xmm0, xmm1
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x00000004FFFFFFFE

    ; ==== Test 5: pmaddubsw basic lo64 ====
    ; word[0] = 0xFF*0x7F + 0x01*0x7F = 32385+127 = 32512 = 0x7F00
    ; word[1] = 0x80*0x01 + 0x02*0x7F = 128+254 = 382 = 0x017E
    ; word[2] = 0x10*0x01 + 0x10*0x01 = 16+16 = 32 = 0x0020
    ; word[3] = 0x00*0x01 + 0x00*0x01 = 0 = 0x0000
    ; lo64 = 0x00000020017E7F00
    TEST_CASE t5_name
    movdqa xmm0, [rel vec_maddubs_a]
    movdqa xmm1, [rel vec_maddubs_b]
    pmaddubsw xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x00000020017E7F00

    ; ==== Test 6: pmaddubsw basic hi64 ====
    ; Same pattern repeats: hi64 = 0x00000020017E7F00
    TEST_CASE t6_name
    movdqa xmm0, [rel vec_maddubs_a]
    movdqa xmm1, [rel vec_maddubs_b]
    pmaddubsw xmm0, xmm1
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x00000020017E7F00

    ; ==== Test 7: pmaddubsw saturation lo64 ====
    ; Each pair: 255*127 + 255*127 = 64770 > 32767 -> saturates to 0x7FFF
    ; All words = 0x7FFF
    ; lo64 = 0x7FFF7FFF7FFF7FFF
    TEST_CASE t7_name
    movdqa xmm0, [rel vec_maddubs_sat_a]
    movdqa xmm1, [rel vec_maddubs_sat_b]
    pmaddubsw xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x7FFF7FFF7FFF7FFF

    ; ==== Test 8: pmaddubsw saturation hi64 ====
    TEST_CASE t8_name
    movdqa xmm0, [rel vec_maddubs_sat_a]
    movdqa xmm1, [rel vec_maddubs_sat_b]
    pmaddubsw xmm0, xmm1
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x7FFF7FFF7FFF7FFF

    ; ==== Test 9: pmulhrsw basic lo64 ====
    ; word[0]: 0x4000*0x4000: (16384*16384>>14+1)>>1 = (16384+1)>>1 = 8192 = 0x2000
    ; word[1]: 0x7FFF*0x7FFF: (32767*32767>>14+1)>>1 = (65532+1)>>1 = 32766 = 0x7FFE
    ; word[2]: 0x0100*0x0100: (256*256>>14+1)>>1 = (4+1)>>1 = 2 = 0x0002
    ; word[3]: 0x0001*0x0001: (1*1>>14+1)>>1 = (0+1)>>1 = 0 = 0x0000
    ; lo64 = 0x000000027FFE2000
    TEST_CASE t9_name
    movdqa xmm0, [rel vec_pmulhrsw_a]
    movdqa xmm1, xmm0
    pmulhrsw xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x000000027FFE2000

    ; ==== Test 10: pmulhrsw basic hi64 ====
    ; Same pattern: hi64 = 0x000000027FFE2000
    TEST_CASE t10_name
    movdqa xmm0, [rel vec_pmulhrsw_a]
    movdqa xmm1, xmm0
    pmulhrsw xmm0, xmm1
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x000000027FFE2000

    ; ==== Test 11: pabsb lo64 ====
    ; Input: [0x00,0x01,0x7F,0x80, 0xFF,0xFE,0x01,0x81, ...]
    ; abs:   [0x00,0x01,0x7F,0x80, 0x01,0x02,0x01,0x7F]
    ; lo64 = byte[7]..byte[0] = 0x7F_01_02_01_80_7F_01_00
    TEST_CASE t11_name
    movdqa xmm0, [rel vec_absb_in]
    pabsb xmm0, xmm0
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x7F010201807F0100

    ; ==== Test 12: pabsb hi64 ====
    ; Input hi: [0x02,0xFD,0x40,0xC0, 0x10,0xF0,0x7E,0x82]
    ; abs:      [0x02,0x03,0x40,0x40, 0x10,0x10,0x7E,0x7E]
    ; hi64 = 0x7E_7E_10_10_40_40_03_02
    TEST_CASE t12_name
    movdqa xmm0, [rel vec_absb_in]
    pabsb xmm0, xmm0
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x7E7E101040400302

    ; ==== Test 13: pabsw lo64 ====
    ; Input words: [0x0001, 0xFFFF, 0x7FFF, 0x8000]
    ; abs:         [0x0001, 0x0001, 0x7FFF, 0x8000]
    ; lo64 = 0x80007FFF00010001
    TEST_CASE t13_name
    movdqa xmm0, [rel vec_absw_in]
    pabsw xmm0, xmm0
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x80007FFF00010001

    ; ==== Test 14: pabsw hi64 ====
    ; Input words: [0x8001, 0xFFFE, 0x0000, 0x0100]
    ; abs:         [0x7FFF, 0x0002, 0x0000, 0x0100]
    ; hi64 = 0x0100000000027FFF
    TEST_CASE t14_name
    movdqa xmm0, [rel vec_absw_in]
    pabsw xmm0, xmm0
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x0100000000027FFF

    ; ==== Test 15: pabsd lo64 ====
    ; Input dwords: [1, -1]
    ; abs:          [1, 1]
    ; lo64 = 0x0000000100000001
    TEST_CASE t15_name
    movdqa xmm0, [rel vec_absd_in]
    pabsd xmm0, xmm0
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0000000100000001

    ; ==== Test 16: pabsd hi64 ====
    ; Input dwords: [0x7FFFFFFF, 0x80000000]
    ; abs:          [0x7FFFFFFF, 0x80000000]
    ; hi64 = 0x800000007FFFFFFF
    TEST_CASE t16_name
    movdqa xmm0, [rel vec_absd_in]
    pabsd xmm0, xmm0
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x800000007FFFFFFF

    ; ==== Test 17: psignb lo64 ====
    ; A: [01,02,03,04,05,06,07,08, ...]
    ; B: [01,FF,00,7F,80,01,FF,00, ...]
    ; R: [01,FE,00,04,FB,06,F9,00]
    ; lo64 = 0x00F906FB0400FE01
    TEST_CASE t17_name
    movdqa xmm0, [rel vec_psignb_a]
    movdqa xmm1, [rel vec_psignb_b]
    psignb xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x00F906FB0400FE01

    ; ==== Test 18: psignb hi64 ====
    ; A: [09,0A,0B,0C,0D,0E,0F,10]
    ; B: [01,FF,00,7F,80,01,FF,00]
    ; R: [09,F6,00,0C,F3,0E,F1,00]
    ; hi64 = 0x00F10EF30C00F609
    TEST_CASE t18_name
    movdqa xmm0, [rel vec_psignb_a]
    movdqa xmm1, [rel vec_psignb_b]
    psignb xmm0, xmm1
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x00F10EF30C00F609

    ; ==== Test 19: psignw lo64 ====
    ; A words: [1, 2, 3, 4]
    ; B words: [1, -1, 0, 0x7FFF]
    ; R words: [1, -2(FFFE), 0, 4]
    ; lo64 = 0x00040000FFFE0001
    TEST_CASE t19_name
    movdqa xmm0, [rel vec_psignw_a]
    movdqa xmm1, [rel vec_psignw_b]
    psignw xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x00040000FFFE0001

    ; ==== Test 20: psignw hi64 ====
    ; A words: [5, 6, 7, 8]
    ; B words: [0x8000(-), 1, -1, 0]
    ; R words: [-5(FFFB), 6, -7(FFF9), 0]
    ; hi64 = 0x0000FFF90006FFFB
    TEST_CASE t20_name
    movdqa xmm0, [rel vec_psignw_a]
    movdqa xmm1, [rel vec_psignw_b]
    psignw xmm0, xmm1
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x0000FFF90006FFFB

    ; ==== Test 21: psignd lo64 ====
    ; A dwords: [1, 2]
    ; B dwords: [1, -1]
    ; R dwords: [1, -2(FFFFFFFE)]
    ; lo64 = 0xFFFFFFFE00000001
    TEST_CASE t21_name
    movdqa xmm0, [rel vec_psignd_a]
    movdqa xmm1, [rel vec_psignd_b]
    psignd xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xFFFFFFFE00000001

    ; ==== Test 22: psignd hi64 ====
    ; A dwords: [3, 4]
    ; B dwords: [0, 0x7FFFFFFF]
    ; R dwords: [0, 4]
    ; hi64 = 0x0000000400000000
    TEST_CASE t22_name
    movdqa xmm0, [rel vec_psignd_a]
    movdqa xmm1, [rel vec_psignd_b]
    psignd xmm0, xmm1
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x0000000400000000

    ; ==== Test 23: packusdw lo64 (SSE4.1) ====
    ; A dwords: [1, 0xFFFF, -1, 0x10000]
    ; Pack to unsigned words with saturation:
    ;   1->0x0001, 0xFFFF->0xFFFF, -1->0x0000(neg sat), 0x10000->0xFFFF(>65535 sat)
    ; lo64 = words [0x0001, 0xFFFF, 0x0000, 0xFFFF] = 0xFFFF0000FFFF0001
    TEST_CASE t23_name
    movdqa xmm0, [rel vec_packusdw_a]
    movdqa xmm1, [rel vec_packusdw_b]
    packusdw xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xFFFF0000FFFF0001

    ; ==== Test 24: packusdw hi64 ====
    ; B dwords: [0, 0x7FFF, 0x80000000, 0xFFFE]
    ; Pack: 0->0x0000, 0x7FFF->0x7FFF, -2147483648->0x0000(neg sat), 0xFFFE->0xFFFE
    ; hi64 = words [0x0000, 0x7FFF, 0x0000, 0xFFFE] = 0xFFFE00007FFF0000
    TEST_CASE t24_name
    movdqa xmm0, [rel vec_packusdw_a]
    movdqa xmm1, [rel vec_packusdw_b]
    packusdw xmm0, xmm1
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0xFFFE00007FFF0000

    ; ==== Test 25: pmulhw lo64 ====
    ; A=B=[0x0100, 0x7FFF, 0x8000, 0xFFFF, ...]
    ; Signed high multiply:
    ;   256*256=65536 >> 16 = 1 = 0x0001
    ;   32767*32767=1073676289 >> 16 = 16383 = 0x3FFF
    ;   (-32768)*(-32768)=1073741824 >> 16 = 16384 = 0x4000
    ;   (-1)*(-1)=1 >> 16 = 0 = 0x0000
    ; lo64 = 0x000040003FFF0001
    TEST_CASE t25_name
    movdqa xmm0, [rel vec_pmulh_a]
    movdqa xmm1, xmm0
    pmulhw xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x000040003FFF0001

    ; ==== Test 26: pmulhw hi64 ====
    ; words: [0x0001, 0x1000, 0x4000, 0x2000]
    ;   1*1=1 >> 16 = 0 = 0x0000
    ;   0x1000*0x1000=16777216 >> 16 = 256 = 0x0100
    ;   0x4000*0x4000=268435456 >> 16 = 4096 = 0x1000
    ;   0x2000*0x2000=67108864 >> 16 = 1024 = 0x0400
    ; hi64 = 0x0400100001000000
    TEST_CASE t26_name
    movdqa xmm0, [rel vec_pmulh_a]
    movdqa xmm1, xmm0
    pmulhw xmm0, xmm1
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x0400100001000000

    ; ==== Test 27: pmulhuw lo64 ====
    ; Same inputs, unsigned high multiply:
    ;   256*256=65536 >> 16 = 1 (same)
    ;   32767*32767 >> 16 = 16383 (same)
    ;   0x8000*0x8000=32768*32768=1073741824 >> 16 = 16384 = 0x4000 (same)
    ;   0xFFFF*0xFFFF=65535*65535=4294836225 >> 16 = 65534 = 0xFFFE
    ; lo64 = 0xFFFE40003FFF0001
    TEST_CASE t27_name
    movdqa xmm0, [rel vec_pmulh_a]
    movdqa xmm1, xmm0
    pmulhuw xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xFFFE40003FFF0001

    ; ==== Test 28: pmulhuw hi64 ====
    ; Same as signed for these values since all are positive unsigned
    ; hi64 = 0x0400100001000000
    TEST_CASE t28_name
    movdqa xmm0, [rel vec_pmulh_a]
    movdqa xmm1, xmm0
    pmulhuw xmm0, xmm1
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x0400100001000000

    ; ==== Test 29: pmuludq lo64 ====
    ; A dword[0]=2, B dword[0]=3: 2*3 = 6
    ; lo64 (qword result of dword[0]*dword[0]) = 6
    TEST_CASE t29_name
    movdqa xmm0, [rel vec_pmuludq_a]
    movdqa xmm1, [rel vec_pmuludq_b]
    pmuludq xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0000000000000006

    ; ==== Test 30: pmuludq hi64 ====
    ; A dword[2]=0xFFFFFFFF, B dword[2]=0xFFFFFFFF
    ; 0xFFFFFFFF * 0xFFFFFFFF = 0xFFFFFFFE00000001
    TEST_CASE t30_name
    movdqa xmm0, [rel vec_pmuludq_a]
    movdqa xmm1, [rel vec_pmuludq_b]
    pmuludq xmm0, xmm1
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0xFFFFFFFE00000001

    ; ==== Test 31: pmaddwd max lo64 ====
    ; dword[0] = 32767*32767 + 32767*32767 = 2*1073676289 = 2147352578 = 0x7FFE0002
    ; dword[1] = (-32768)*(-32768) + (-32768)*(-32768) = 2*1073741824 = 2147483648 = 0x80000000
    ; lo64 = 0x800000007FFE0002
    TEST_CASE t31_name
    movdqa xmm0, [rel vec_madd_max_a]
    movdqa xmm1, [rel vec_madd_max_b]
    pmaddwd xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x800000007FFE0002

    ; ==== Test 32: pmaddwd max hi64 ====
    ; dword[2] = 0*0 + 0*0 = 0
    ; dword[3] = 1*0 + 0*0 = 0
    ; hi64 = 0x0000000000000000
    TEST_CASE t32_name
    movdqa xmm0, [rel vec_madd_max_a]
    movdqa xmm1, [rel vec_madd_max_b]
    pmaddwd xmm0, xmm1
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x0000000000000000

    ; ==== Test 33: pmulhrsw negative lo64 ====
    ; Squaring negative values gives same result as positive equivalents
    ; word[0]: (-16384)*(-16384) -> 8192 = 0x2000
    ; word[1]: (-32767)*(-32767) -> 32766 = 0x7FFE
    ; word[2]: (-256)*(-256) -> 2 = 0x0002
    ; word[3]: (-1)*(-1) -> 0 = 0x0000
    ; lo64 = 0x000000027FFE2000
    TEST_CASE t33_name
    movdqa xmm0, [rel vec_pmulhrsw_neg]
    movdqa xmm1, xmm0
    pmulhrsw xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x000000027FFE2000

    ; ==== Test 34: pmulhrsw negative hi64 ====
    ; Same pattern: hi64 = 0x000000027FFE2000
    TEST_CASE t34_name
    movdqa xmm0, [rel vec_pmulhrsw_neg]
    movdqa xmm1, xmm0
    pmulhrsw xmm0, xmm1
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x000000027FFE2000

    END_TESTS
