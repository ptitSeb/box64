; test_sse_int.asm - Test SSE integer operations
; PCMPEQB/W/D, PMIN/PMAX, PACK/UNPACK, PSHUFD, PSHUFB, etc.
%include "test_framework.inc"

section .data
    t1_name:  db "pcmpeqb equal", 0
    t2_name:  db "pcmpeqb differ", 0
    t3_name:  db "pcmpeqw equal", 0
    t4_name:  db "pcmpeqw partial", 0
    t5_name:  db "pcmpeqd equal", 0
    t6_name:  db "pcmpeqd partial", 0
    t7_name:  db "pcmpgtb positive", 0
    t8_name:  db "pcmpgtb negative", 0
    t9_name:  db "pminub basic", 0
    t10_name: db "pmaxub basic", 0
    t11_name: db "pminsw basic", 0
    t12_name: db "pmaxsw basic", 0
    t13_name: db "pshufd identity", 0
    t14_name: db "pshufd reverse", 0
    t15_name: db "pshufd broadcast", 0
    t16_name: db "pshufb basic", 0
    t17_name: db "pshufb zero mask", 0
    t18_name: db "punpcklbw", 0
    t19_name: db "punpckhbw", 0
    t20_name: db "punpckldq", 0
    t21_name: db "punpckhdq", 0
    t22_name: db "packsswb basic", 0
    t23_name: db "packuswb basic", 0
    t24_name: db "paddb basic", 0
    t25_name: db "paddw basic", 0
    t26_name: db "paddd basic", 0
    t27_name: db "paddq basic", 0
    t28_name: db "psubb basic", 0
    t29_name: db "psubw basic", 0
    t30_name: db "psubd basic", 0
    t31_name: db "pmullw basic", 0
    t32_name: db "pmulld basic", 0
    t33_name: db "pand/pandn/por/pxor", 0
    t34_name: db "psllw imm", 0
    t35_name: db "pslld imm", 0
    t36_name: db "psllq imm", 0
    t37_name: db "psrlw imm", 0
    t38_name: db "psrld imm", 0
    t39_name: db "psrlq imm", 0
    t40_name: db "psraw imm", 0

    align 16
    ; Test vectors
    vec_42:     times 16 db 0x42
    vec_42_alt: db 0x42,0x43,0x42,0x43, 0x42,0x43,0x42,0x43, 0x42,0x43,0x42,0x43, 0x42,0x43,0x42,0x43
    vec_zero:   times 16 db 0
    vec_ff:     times 16 db 0xFF
    vec_01:     times 16 db 0x01
    vec_7f:     times 16 db 0x7F
    vec_80:     times 16 db 0x80
    ; word vectors
    vec_w1234:  dw 1, 2, 3, 4, 5, 6, 7, 8
    vec_w8765:  dw 8, 7, 6, 5, 4, 3, 2, 1
    vec_w_neg:  dw -1, -2, -3, -4, -5, -6, -7, -8
    ; dword vectors
    vec_d1234:  dd 1, 2, 3, 4
    vec_d5678:  dd 5, 6, 7, 8
    vec_d_neg:  dd -1, -2, -3, -4
    ; qword vectors
    vec_q12:    dq 1, 2
    vec_q34:    dq 3, 4
    ; pshufb control
    shuf_identity: db 0,1,2,3, 4,5,6,7, 8,9,10,11, 12,13,14,15
    shuf_zero:     times 16 db 0x80  ; all high bit set = zero output
    shuf_reverse:  db 15,14,13,12, 11,10,9,8, 7,6,5,4, 3,2,1,0
    ; Interleave test patterns
    vec_interleave_a: db 0x01,0x02,0x03,0x04, 0x05,0x06,0x07,0x08, 0x09,0x0A,0x0B,0x0C, 0x0D,0x0E,0x0F,0x10
    vec_interleave_b: db 0xA1,0xA2,0xA3,0xA4, 0xA5,0xA6,0xA7,0xA8, 0xA9,0xAA,0xAB,0xAC, 0xAD,0xAE,0xAF,0xB0
    ; pack test: words that saturate
    vec_pack_w1: dw 0, 127, 128, -1, -128, -129, 32767, -32768
    vec_pack_w2: dw 10, 20, 30, 40, 50, 60, 70, 80
    ; unsigned pack test
    vec_upack_w1: dw 0, 127, 255, 256, 300, -1, 1000, 65535
    vec_upack_w2: dw 0, 1, 128, 255, 256, 512, 1024, 2048
    ; pminub/pmaxub
    vec_min_a: db 0x05,0xFF,0x00,0x80, 0x7F,0x01,0xFE,0x10, 0x20,0x30,0x40,0x50, 0x60,0x70,0x80,0x90
    vec_min_b: db 0x10,0x01,0xFF,0x7F, 0x80,0xFF,0x02,0x20, 0x10,0x40,0x30,0x60, 0x50,0x80,0x70,0xA0
    ; pminsw/pmaxsw
    vec_sw_a: dw 5, -1, 32767, -32768, 0, 100, -100, 1000
    vec_sw_b: dw 10, 1, -1, 32767, -1, -100, 100, -1000
    ; pcmpgtb test
    vec_gt_a: db 0x05,0x80,0x7F,0x00, 0xFF,0x01,0x42,0xBE, 0x05,0x80,0x7F,0x00, 0xFF,0x01,0x42,0xBE
    vec_gt_b: db 0x03,0x7F,0x7E,0x01, 0x00,0xFF,0x42,0xBF, 0x03,0x7F,0x7E,0x01, 0x00,0xFF,0x42,0xBF

    ; pshufd data
    vec_shuf_d: dd 0x11111111, 0x22222222, 0x33333333, 0x44444444

section .text
global _start

_start:
    INIT_TESTS

    ; ==== Test 1: pcmpeqb equal ====
    TEST_CASE t1_name
    movdqa xmm0, [rel vec_42]
    movdqa xmm1, [rel vec_42]
    pcmpeqb xmm0, xmm1
    ; All equal -> all 0xFF
    pmovmskb eax, xmm0
    CHECK_EQ_32 eax, 0xFFFF

    ; ==== Test 2: pcmpeqb differ ====
    TEST_CASE t2_name
    movdqa xmm0, [rel vec_42]
    movdqa xmm1, [rel vec_42_alt]
    pcmpeqb xmm0, xmm1
    ; Equal at positions 0,2,4,6,8,10,12,14 -> 0x5555
    pmovmskb eax, xmm0
    CHECK_EQ_32 eax, 0x5555

    ; ==== Test 3: pcmpeqw equal ====
    TEST_CASE t3_name
    movdqa xmm0, [rel vec_w1234]
    movdqa xmm1, [rel vec_w1234]
    pcmpeqw xmm0, xmm1
    pmovmskb eax, xmm0
    CHECK_EQ_32 eax, 0xFFFF

    ; ==== Test 4: pcmpeqw partial ====
    TEST_CASE t4_name
    movdqa xmm0, [rel vec_w1234]
    movdqa xmm1, [rel vec_w8765]
    pcmpeqw xmm0, xmm1
    ; words: 1vs8=ne, 2vs7=ne, 3vs6=ne, 4vs5=ne, 5vs4=ne, 6vs3=ne, 7vs2=ne, 8vs1=ne
    ; All different -> 0x0000
    pmovmskb eax, xmm0
    CHECK_EQ_32 eax, 0x0000

    ; ==== Test 5: pcmpeqd equal ====
    TEST_CASE t5_name
    movdqa xmm0, [rel vec_d1234]
    movdqa xmm1, [rel vec_d1234]
    pcmpeqd xmm0, xmm1
    pmovmskb eax, xmm0
    CHECK_EQ_32 eax, 0xFFFF

    ; ==== Test 6: pcmpeqd partial ====
    TEST_CASE t6_name
    movdqa xmm0, [rel vec_d1234]
    movdqa xmm1, [rel vec_d5678]
    pcmpeqd xmm0, xmm1
    pmovmskb eax, xmm0
    CHECK_EQ_32 eax, 0x0000

    ; ==== Test 7: pcmpgtb - a > b (signed) ====
    TEST_CASE t7_name
    movdqa xmm0, [rel vec_gt_a]
    movdqa xmm1, [rel vec_gt_b]
    pcmpgtb xmm0, xmm1
    pmovmskb eax, xmm0
    ; Byte comparisons (signed):
    ; 0x05>0x03=Y, 0x80>0x7F=N(signed!), 0x7F>0x7E=Y, 0x00>0x01=N
    ; 0xFF>0x00=N(signed -1>0), 0x01>0xFF=Y(1>-1), 0x42>0x42=N, 0xBE>0xBF=Y(-66>-65? No, -66<-65)
    ; Let me recalc: 0xBE=-66, 0xBF=-65, -66>-65? No.
    ; byte0: 5>3=Y(1), byte1: -128>127=N(0), byte2: 127>126=Y(1), byte3: 0>1=N(0)
    ; byte4: -1>0=N(0), byte5: 1>-1=Y(1), byte6: 66>66=N(0), byte7: -66>-65=N(0)
    ; Same pattern repeats for bytes 8-15
    ; Mask: 0,0,1,0, 0,1,0,0, 1,0,0,0, 0,1,0,0 wait let me redo
    ; bit0=byte0: 5>3=Y=1, bit1=byte1: -128>127=N=0, bit2=byte2: 127>126=Y=1, bit3=byte3: 0>1=N=0
    ; bit4=byte4: -1>0=N=0, bit5=byte5: 1>-1=Y=1, bit6=byte6: 66>66=N=0, bit7=byte7: -66>-65=N=0
    ; low 8 bits: 00100101 = 0x25
    ; Repeats: 0x2525
    CHECK_EQ_32 eax, 0x2525

    ; ==== Test 8: pcmpgtb negative check ====
    TEST_CASE t8_name
    movdqa xmm0, [rel vec_gt_b]
    movdqa xmm1, [rel vec_gt_a]
    pcmpgtb xmm0, xmm1
    pmovmskb eax, xmm0
    ; Inverse of above for strict gt (not equal positions stay 0)
    ; byte0: 3>5=N, byte1: 127>-128=Y, byte2: 126>127=N, byte3: 1>0=Y
    ; byte4: 0>-1=Y, byte5: -1>1=N, byte6: 66>66=N, byte7: -65>-66=Y
    ; low 8: 10011010 = 0x9A (wait: bit7=byte7, bit6=byte6...)
    ; Actually bit order: bit0=byte0=0, bit1=byte1=1, bit2=byte2=0, bit3=byte3=1
    ; bit4=byte4=1, bit5=byte5=0, bit6=byte6=0, bit7=byte7=1
    ; = 10011010 = 0x9A. Repeats: 0x9A9A
    ; But wait: 0x25 | 0x9A should = 0xBF not 0xFF because equal bytes give 0 in both
    ; byte6: 0x42 vs 0x42 is equal -> 0 in both. 0x25 | 0x9A = 0xBF. 
    ; 0xBF = 10111111 -> bit6=0, all others set. Correct (byte6 is equal).
    CHECK_EQ_32 eax, 0x9A9A

    ; ==== Test 9: pminub ====
    TEST_CASE t9_name
    movdqa xmm0, [rel vec_min_a]
    movdqa xmm1, [rel vec_min_b]
    pminub xmm0, xmm1
    ; Check first 8 bytes via movq
    ; min(05,10)=05, min(FF,01)=01, min(00,FF)=00, min(80,7F)=7F
    ; min(7F,80)=7F, min(01,FF)=01, min(FE,02)=02, min(10,20)=10
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x1002017F7F000105

    ; ==== Test 10: pmaxub ====
    TEST_CASE t10_name
    movdqa xmm0, [rel vec_min_a]
    movdqa xmm1, [rel vec_min_b]
    pmaxub xmm0, xmm1
    ; max(05,10)=10, max(FF,01)=FF, max(00,FF)=FF, max(80,7F)=80
    ; max(7F,80)=80, max(01,FF)=FF, max(FE,02)=FE, max(10,20)=20
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x20FEFF8080FFFF10

    ; ==== Test 11: pminsw ====
    TEST_CASE t11_name
    movdqa xmm0, [rel vec_sw_a]
    movdqa xmm1, [rel vec_sw_b]
    pminsw xmm0, xmm1
    ; min(5,10)=5, min(-1,1)=-1, min(32767,-1)=-1, min(-32768,32767)=-32768
    ; Low 64 bits as words: 5, -1, -1, -32768
    ; = 0x0005, 0xFFFF, 0xFFFF, 0x8000
    ; packed LE: 05 00 FF FF FF FF 00 80
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x8000FFFFFFFF0005

    ; ==== Test 12: pmaxsw ====
    TEST_CASE t12_name
    movdqa xmm0, [rel vec_sw_a]
    movdqa xmm1, [rel vec_sw_b]
    pmaxsw xmm0, xmm1
    ; max(5,10)=10, max(-1,1)=1, max(32767,-1)=32767, max(-32768,32767)=32767
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x7FFF7FFF0001000A

    ; ==== Test 13: pshufd identity (imm = 0xE4 = 3,2,1,0) ====
    TEST_CASE t13_name
    movdqa xmm0, [rel vec_shuf_d]
    pshufd xmm1, xmm0, 0xE4    ; identity shuffle
    ; Should be unchanged
    movq rax, xmm1
    CHECK_EQ_64 rax, 0x2222222211111111

    ; ==== Test 14: pshufd reverse (imm = 0x1B = 0,1,2,3) ====
    TEST_CASE t14_name
    movdqa xmm0, [rel vec_shuf_d]
    pshufd xmm1, xmm0, 0x1B    ; reverse
    ; dword[0]=dword[3]=0x44444444, dword[1]=dword[2]=0x33333333
    movq rax, xmm1
    CHECK_EQ_64 rax, 0x3333333344444444

    ; ==== Test 15: pshufd broadcast (imm = 0x00 = 0,0,0,0) ====
    TEST_CASE t15_name
    movdqa xmm0, [rel vec_shuf_d]
    pshufd xmm1, xmm0, 0x00    ; broadcast dword[0]
    movq rax, xmm1
    CHECK_EQ_64 rax, 0x1111111111111111

    ; ==== Test 16: pshufb identity ====
    TEST_CASE t16_name
    movdqa xmm0, [rel vec_interleave_a]
    movdqa xmm1, [rel shuf_identity]
    pshufb xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0807060504030201

    ; ==== Test 17: pshufb zero mask ====
    TEST_CASE t17_name
    movdqa xmm0, [rel vec_interleave_a]
    movdqa xmm1, [rel shuf_zero]
    pshufb xmm0, xmm1
    ; All control bytes have high bit set -> all output bytes = 0
    movq rax, xmm0
    CHECK_EQ_64 rax, 0

    ; ==== Test 18: punpcklbw ====
    TEST_CASE t18_name
    movdqa xmm0, [rel vec_interleave_a]
    movdqa xmm1, [rel vec_interleave_b]
    punpcklbw xmm0, xmm1
    ; Interleaves low 8 bytes: a[0],b[0],a[1],b[1],...a[7],b[7]
    ; = 01,A1,02,A2,03,A3,04,A4,05,A5,06,A6,07,A7,08,A8
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xA404A303A202A101

    ; ==== Test 19: punpckhbw ====
    TEST_CASE t19_name
    movdqa xmm0, [rel vec_interleave_a]
    movdqa xmm1, [rel vec_interleave_b]
    punpckhbw xmm0, xmm1
    ; Interleaves high 8 bytes: a[8],b[8],...a[15],b[15]
    ; = 09,A9,0A,AA,0B,AB,0C,AC,0D,AD,0E,AE,0F,AF,10,B0
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xAC0CAB0BAA0AA909

    ; ==== Test 20: punpckldq ====
    TEST_CASE t20_name
    movdqa xmm0, [rel vec_d1234]
    movdqa xmm1, [rel vec_d5678]
    punpckldq xmm0, xmm1
    ; Interleaves low 2 dwords: d0[0],d1[0],d0[1],d1[1]
    ; = 1,5,2,6
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0000000500000001

    ; ==== Test 21: punpckhdq ====
    TEST_CASE t21_name
    movdqa xmm0, [rel vec_d1234]
    movdqa xmm1, [rel vec_d5678]
    punpckhdq xmm0, xmm1
    ; Interleaves high 2 dwords: d0[2],d1[2],d0[3],d1[3]
    ; = 3,7,4,8
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0000000700000003

    ; ==== Test 22: packsswb ====
    TEST_CASE t22_name
    movdqa xmm0, [rel vec_pack_w1]
    movdqa xmm1, [rel vec_pack_w2]
    packsswb xmm0, xmm1
    ; Signed saturation of words to bytes:
    ; xmm0 words: 0->0, 127->127, 128->127, -1->-1, -128->-128, -129->-128, 32767->127, -32768->-128
    ; xmm1 words: 10,20,30,40,50,60,70,80 -> all fit in signed byte
    ; Result low 8 bytes: 00,7F,7F,FF,80,80,7F,80
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x807F8080FF7F7F00

    ; ==== Test 23: packuswb ====
    TEST_CASE t23_name
    movdqa xmm0, [rel vec_upack_w1]
    movdqa xmm1, [rel vec_upack_w2]
    packuswb xmm0, xmm1
    ; Unsigned saturation of signed words to unsigned bytes:
    ; xmm0: 0->0, 127->127, 255->255, 256->255, 300->255, -1->0, 1000->255, 65535(=-1 signed)->0
    ; xmm1: 0->0, 1->1, 128->128, 255->255, 256->255, 512->255, 1024->255, 2048->255
    ; Low 8 bytes: 00,7F,FF,FF,FF,00,FF,00
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x00FF00FFFF7F0000

    ; ==== Test 24: paddb ====
    TEST_CASE t24_name
    movdqa xmm0, [rel vec_01]
    movdqa xmm1, [rel vec_7f]
    paddb xmm0, xmm1
    ; 0x01 + 0x7F = 0x80 for each byte
    pmovmskb eax, xmm0     ; all have sign bit set
    CHECK_EQ_32 eax, 0xFFFF

    ; ==== Test 25: paddw ====
    TEST_CASE t25_name
    movdqa xmm0, [rel vec_w1234]
    movdqa xmm1, [rel vec_w8765]
    paddw xmm0, xmm1
    ; 1+8=9, 2+7=9, 3+6=9, 4+5=9, 5+4=9, 6+3=9, 7+2=9, 8+1=9
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0009000900090009

    ; ==== Test 26: paddd ====
    TEST_CASE t26_name
    movdqa xmm0, [rel vec_d1234]
    movdqa xmm1, [rel vec_d5678]
    paddd xmm0, xmm1
    ; 1+5=6, 2+6=8, 3+7=10, 4+8=12
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0000000800000006

    ; ==== Test 27: paddq ====
    TEST_CASE t27_name
    movdqa xmm0, [rel vec_q12]
    movdqa xmm1, [rel vec_q34]
    paddq xmm0, xmm1
    ; 1+3=4, 2+4=6
    movq rax, xmm0
    CHECK_EQ_64 rax, 4

    ; ==== Test 28: psubb ====
    TEST_CASE t28_name
    movdqa xmm0, [rel vec_80]
    movdqa xmm1, [rel vec_01]
    psubb xmm0, xmm1
    ; 0x80 - 0x01 = 0x7F for each byte
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x7F7F7F7F7F7F7F7F

    ; ==== Test 29: psubw ====
    TEST_CASE t29_name
    movdqa xmm0, [rel vec_w1234]
    movdqa xmm1, [rel vec_w8765]
    psubw xmm0, xmm1
    ; 1-8=-7(0xFFF9), 2-7=-5(0xFFFB), 3-6=-3(0xFFFD), 4-5=-1(0xFFFF)
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xFFFFFFFDFFFBFFF9

    ; ==== Test 30: psubd ====
    TEST_CASE t30_name
    movdqa xmm0, [rel vec_d1234]
    movdqa xmm1, [rel vec_d5678]
    psubd xmm0, xmm1
    ; 1-5=-4(0xFFFFFFFC), 2-6=-4(0xFFFFFFFC)
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xFFFFFFFCFFFFFFFC

    ; ==== Test 31: pmullw ====
    TEST_CASE t31_name
    movdqa xmm0, [rel vec_w1234]
    movdqa xmm1, [rel vec_w8765]
    pmullw xmm0, xmm1
    ; 1*8=8, 2*7=14, 3*6=18, 4*5=20
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x00140012000E0008

    ; ==== Test 32: pmulld (SSE4.1) ====
    TEST_CASE t32_name
    movdqa xmm0, [rel vec_d1234]
    movdqa xmm1, [rel vec_d5678]
    pmulld xmm0, xmm1
    ; 1*5=5, 2*6=12
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0000000C00000005

    ; ==== Test 33: pand, pandn, por, pxor ====
    TEST_CASE t33_name
    movdqa xmm0, [rel vec_42]       ; all 0x42
    movdqa xmm1, [rel vec_ff]       ; all 0xFF
    movdqa xmm2, xmm0
    pand xmm2, xmm1                  ; 0x42 & 0xFF = 0x42
    movq rax, xmm2
    mov rbx, 0x4242424242424242
    cmp rax, rbx
    jne .t33_fail
    ; pandn: ~xmm0 & xmm1 = ~0x42 & 0xFF = 0xBD
    movdqa xmm2, xmm0
    pandn xmm2, xmm1
    movq rax, xmm2
    mov rbx, 0xBDBDBDBDBDBDBDBD
    cmp rax, rbx
    jne .t33_fail
    ; por: 0x42 | 0x00 = 0x42
    movdqa xmm2, xmm0
    pxor xmm3, xmm3
    por xmm2, xmm3
    movq rax, xmm2
    mov rbx, 0x4242424242424242
    cmp rax, rbx
    jne .t33_fail
    ; pxor: 0x42 ^ 0xFF = 0xBD
    movdqa xmm2, xmm0
    pxor xmm2, xmm1
    movq rax, xmm2
    mov rbx, 0xBDBDBDBDBDBDBDBD
    cmp rax, rbx
    jne .t33_fail
    ; All passed
    push rsi
    lea rsi, [rel _fw_pass_msg]
    call _fw_print_str
    inc r12d
    pop rsi
    jmp .t33_done
.t33_fail:
    push rsi
    lea rsi, [rel _fw_fail_msg]
    call _fw_print_str
    pop rsi
.t33_done:

    ; ==== Test 34: psllw imm ====
    TEST_CASE t34_name
    movdqa xmm0, [rel vec_w1234]
    psllw xmm0, 4
    ; words: 1<<4=16, 2<<4=32, 3<<4=48, 4<<4=64
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0040003000200010

    ; ==== Test 35: pslld imm ====
    TEST_CASE t35_name
    movdqa xmm0, [rel vec_d1234]
    pslld xmm0, 8
    ; dwords: 1<<8=256, 2<<8=512
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0000020000000100

    ; ==== Test 36: psllq imm ====
    TEST_CASE t36_name
    movdqa xmm0, [rel vec_q12]
    psllq xmm0, 16
    ; qwords: 1<<16=65536, 2<<16=131072
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0000000000010000

    ; ==== Test 37: psrlw imm ====
    TEST_CASE t37_name
    movdqa xmm0, [rel vec_w1234]
    movdqa xmm1, xmm0
    psllw xmm1, 8           ; shift left 8 first: 256, 512, 768, 1024
    psrlw xmm1, 8           ; shift right 8: back to 1, 2, 3, 4
    movq rax, xmm1
    CHECK_EQ_64 rax, 0x0004000300020001

    ; ==== Test 38: psrld imm ====
    TEST_CASE t38_name
    movdqa xmm0, [rel vec_d1234]
    movdqa xmm1, xmm0
    pslld xmm1, 16
    psrld xmm1, 16          ; roundtrip
    movq rax, xmm1
    CHECK_EQ_64 rax, 0x0000000200000001

    ; ==== Test 39: psrlq imm ====
    TEST_CASE t39_name
    movdqa xmm0, [rel vec_q12]
    movdqa xmm1, xmm0
    psllq xmm1, 32
    psrlq xmm1, 32
    movq rax, xmm1
    CHECK_EQ_64 rax, 1

    ; ==== Test 40: psraw imm ====
    TEST_CASE t40_name
    movdqa xmm0, [rel vec_w_neg]
    psraw xmm0, 1
    ; -1 >> 1 = -1 (0xFFFF), -2 >> 1 = -1 (0xFFFF), -3 >> 1 = -2, -4 >> 1 = -2
    ; Actually arithmetic: -1 = 0xFFFF >> 1 = 0xFFFF, -2 = 0xFFFE >> 1 = 0xFFFF
    ; -3 = 0xFFFD >> 1 = 0xFFFE, -4 = 0xFFFC >> 1 = 0xFFFE
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xFFFEFFFEFFFFFFFF

    END_TESTS
