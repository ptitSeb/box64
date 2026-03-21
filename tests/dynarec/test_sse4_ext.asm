; test_sse4_ext.asm - Test SSE4.1 sign/zero extend and min/max instructions
; PMOVSXBW/BD/BQ/WD/WQ/DQ, PMOVZXBW/BD/BQ/WD/WQ/DQ,
; PMINSB/PMINSD/PMINUD/PMINUW, PMAXSB/PMAXSD/PMAXUD/PMAXUW, PHMINPOSUW
%include "test_framework.inc"

section .data
    ; ---- Test name strings ----
    ; Each CHECK gets its own TEST_CASE for correct pass/total counting.
    tn_sxbw_pos_lo:   db "pmovsxbw pos lo64", 0
    tn_sxbw_pos_hi:   db "pmovsxbw pos hi64", 0
    tn_sxbw_neg_lo:   db "pmovsxbw neg lo64", 0
    tn_sxbw_neg_hi:   db "pmovsxbw neg hi64", 0
    tn_sxbd_lo:        db "pmovsxbd lo64", 0
    tn_sxbd_hi:        db "pmovsxbd hi64", 0
    tn_sxbq_lo:        db "pmovsxbq lo64", 0
    tn_sxbq_hi:        db "pmovsxbq hi64", 0
    tn_sxwd_pos_lo:   db "pmovsxwd pos lo64", 0
    tn_sxwd_pos_hi:   db "pmovsxwd pos hi64", 0
    tn_sxwd_neg_lo:   db "pmovsxwd neg lo64", 0
    tn_sxwd_neg_hi:   db "pmovsxwd neg hi64", 0
    tn_sxwq_lo:        db "pmovsxwq lo64", 0
    tn_sxwq_hi:        db "pmovsxwq hi64", 0
    tn_sxdq_lo:        db "pmovsxdq lo64", 0
    tn_sxdq_hi:        db "pmovsxdq hi64", 0
    tn_zxbw_pos_lo:   db "pmovzxbw pos lo64", 0
    tn_zxbw_pos_hi:   db "pmovzxbw pos hi64", 0
    tn_zxbw_hi_lo:    db "pmovzxbw high lo64", 0
    tn_zxbw_hi_hi:    db "pmovzxbw high hi64", 0
    tn_zxbd_lo:        db "pmovzxbd lo64", 0
    tn_zxbd_hi:        db "pmovzxbd hi64", 0
    tn_zxbq_lo:        db "pmovzxbq lo64", 0
    tn_zxbq_hi:        db "pmovzxbq hi64", 0
    tn_zxwd_lo:        db "pmovzxwd lo64", 0
    tn_zxwd_hi:        db "pmovzxwd hi64", 0
    tn_zxwq_lo:        db "pmovzxwq lo64", 0
    tn_zxwq_hi:        db "pmovzxwq hi64", 0
    tn_zxdq_lo:        db "pmovzxdq lo64", 0
    tn_zxdq_hi:        db "pmovzxdq hi64", 0
    tn_minsb_mix_lo:  db "pminsb mix lo64", 0
    tn_minsb_mix_hi:  db "pminsb mix hi64", 0
    tn_minsb_pos_lo:  db "pminsb pos lo64", 0
    tn_minsb_pos_hi:  db "pminsb pos hi64", 0
    tn_minsd_lo:       db "pminsd lo64", 0
    tn_minsd_hi:       db "pminsd hi64", 0
    tn_minud_lo:       db "pminud lo64", 0
    tn_minud_hi:       db "pminud hi64", 0
    tn_minuw_lo:       db "pminuw lo64", 0
    tn_minuw_hi:       db "pminuw hi64", 0
    tn_maxsb_mix_lo:  db "pmaxsb mix lo64", 0
    tn_maxsb_mix_hi:  db "pmaxsb mix hi64", 0
    tn_maxsb_pos_lo:  db "pmaxsb pos lo64", 0
    tn_maxsb_pos_hi:  db "pmaxsb pos hi64", 0
    tn_maxsd_lo:       db "pmaxsd lo64", 0
    tn_maxsd_hi:       db "pmaxsd hi64", 0
    tn_maxud_lo:       db "pmaxud lo64", 0
    tn_maxud_hi:       db "pmaxud hi64", 0
    tn_maxuw_lo:       db "pmaxuw lo64", 0
    tn_maxuw_hi:       db "pmaxuw hi64", 0
    tn_phmin1_lo:      db "phminposuw basic lo64", 0
    tn_phmin1_hi:      db "phminposuw basic hi64", 0
    tn_phmin2_lo:      db "phminposuw zero lo64", 0
    tn_phmin2_hi:      db "phminposuw zero hi64", 0

    align 16
    ; ---- Test vectors ----

    ; PMOVSXBW/PMOVZXBW positive: bytes 01 02 03 04 05 06 07 08
    vec_sx_bw_pos: db 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
                   db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

    ; PMOVSXBW/PMOVZXBW negative: bytes FF 80 7F FE 01 00 81 82
    vec_sx_bw_neg: db 0xFF, 0x80, 0x7F, 0xFE, 0x01, 0x00, 0x81, 0x82
                   db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

    ; PMOVSXBD: bytes 01 FE 7F 80
    vec_sx_bd:     db 0x01, 0xFE, 0x7F, 0x80, 0x00, 0x00, 0x00, 0x00
                   db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

    ; PMOVSXBQ: bytes 01 FE
    vec_sx_bq:     db 0x01, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                   db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

    ; PMOVSXWD positive: words 0001 0002 0003 0004
    vec_sx_wd_pos: dw 0x0001, 0x0002, 0x0003, 0x0004, 0x0000, 0x0000, 0x0000, 0x0000

    ; PMOVSXWD negative: words FFFF 8000 7FFF FE00
    vec_sx_wd_neg: dw 0xFFFF, 0x8000, 0x7FFF, 0xFE00, 0x0000, 0x0000, 0x0000, 0x0000

    ; PMOVSXWQ: words 0001 FFFE
    vec_sx_wq:     dw 0x0001, 0xFFFE, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000

    ; PMOVSXDQ: dwords 00000001 FFFFFFFE
    vec_sx_dq:     dd 0x00000001, 0xFFFFFFFE, 0x00000000, 0x00000000

    ; PMOVZXBD: bytes FF 80 7F 01
    vec_zx_bd:     db 0xFF, 0x80, 0x7F, 0x01, 0x00, 0x00, 0x00, 0x00
                   db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

    ; PMOVZXBQ: bytes FF 80
    vec_zx_bq:     db 0xFF, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                   db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

    ; PMOVZXWD: words FFFF 8000 7FFF 0001
    vec_zx_wd:     dw 0xFFFF, 0x8000, 0x7FFF, 0x0001, 0x0000, 0x0000, 0x0000, 0x0000

    ; PMOVZXWQ: words FFFF 8000
    vec_zx_wq:     dw 0xFFFF, 0x8000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000

    ; PMOVZXDQ: dwords FFFFFFFF 80000001
    vec_zx_dq:     dd 0xFFFFFFFF, 0x80000001, 0x00000000, 0x00000000

    ; PMINSB/PMAXSB test 1: mixed signed bytes
    vec_minmax_a:  db 0x05, 0xFF, 0x00, 0x80, 0x7F, 0x01, 0xFE, 0x10
                   db 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90

    vec_minmax_b:  db 0x10, 0x01, 0xFF, 0x7F, 0x80, 0xFF, 0x02, 0x20
                   db 0x10, 0x40, 0x30, 0x60, 0x50, 0x80, 0x70, 0xA0

    ; PMINSB/PMAXSB test 2: all positive bytes
    vec_pos_a:     db 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
                   db 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10

    vec_pos_b:     db 0x10, 0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09
                   db 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01

    ; PMINSD/PMAXSD and PMINUD/PMAXUD: dwords
    vec_sd_a:      dd 0x00000001, 0xFFFFFFFF, 0x7FFFFFFF, 0x80000000
    vec_sd_b:      dd 0xFFFFFFFF, 0x00000001, 0x80000000, 0x7FFFFFFF

    ; PMINUW/PMAXUW: unsigned words
    vec_uw_a:      dw 0x0001, 0xFFFF, 0x7FFF, 0x8000, 0x0100, 0xFF00, 0x00FF, 0x8001
    vec_uw_b:      dw 0xFFFF, 0x0001, 0x8000, 0x7FFF, 0xFF00, 0x0100, 0x8001, 0x00FF

    ; PHMINPOSUW test 1: words 0005 0003 0001 0004 0002 0006 0007 0008
    vec_phmin1:    dw 0x0005, 0x0003, 0x0001, 0x0004, 0x0002, 0x0006, 0x0007, 0x0008

    ; PHMINPOSUW test 2: words FFFF 0080 00FF 0001 0100 0000 8000 7FFF
    vec_phmin2:    dw 0xFFFF, 0x0080, 0x00FF, 0x0001, 0x0100, 0x0000, 0x8000, 0x7FFF

section .text
global _start

_start:
    INIT_TESTS

    ; ================================================================
    ; PMOVSXBW positive bytes
    ; bytes: 01 02 03 04 05 06 07 08
    ; -> words: 0001 0002 0003 0004 0005 0006 0007 0008
    ; lo64 = 0004_0003_0002_0001, hi64 = 0008_0007_0006_0005
    ; ================================================================
    movdqa xmm0, [rel vec_sx_bw_pos]
    pmovsxbw xmm1, xmm0

    TEST_CASE tn_sxbw_pos_lo
    movq rax, xmm1
    CHECK_EQ_64 rax, 0x0004000300020001

    TEST_CASE tn_sxbw_pos_hi
    pextrq rax, xmm1, 1
    CHECK_EQ_64 rax, 0x0008000700060005

    ; ================================================================
    ; PMOVSXBW negative/mixed bytes
    ; bytes: FF 80 7F FE 01 00 81 82
    ; FF=-1->FFFF, 80=-128->FF80, 7F=127->007F, FE=-2->FFFE
    ; 01=1->0001, 00=0->0000, 81=-127->FF81, 82=-126->FF82
    ; lo64 = FFFE_007F_FF80_FFFF, hi64 = FF82_FF81_0000_0001
    ; ================================================================
    movdqa xmm0, [rel vec_sx_bw_neg]
    pmovsxbw xmm1, xmm0

    TEST_CASE tn_sxbw_neg_lo
    movq rax, xmm1
    CHECK_EQ_64 rax, 0xFFFE007FFF80FFFF

    TEST_CASE tn_sxbw_neg_hi
    pextrq rax, xmm1, 1
    CHECK_EQ_64 rax, 0xFF82FF8100000001

    ; ================================================================
    ; PMOVSXBD: bytes 01 FE 7F 80
    ; 01->00000001, FE=-2->FFFFFFFE, 7F->0000007F, 80=-128->FFFFFF80
    ; lo64 = FFFFFFFE_00000001, hi64 = FFFFFF80_0000007F
    ; ================================================================
    movdqa xmm0, [rel vec_sx_bd]
    pmovsxbd xmm1, xmm0

    TEST_CASE tn_sxbd_lo
    movq rax, xmm1
    CHECK_EQ_64 rax, 0xFFFFFFFE00000001

    TEST_CASE tn_sxbd_hi
    pextrq rax, xmm1, 1
    CHECK_EQ_64 rax, 0xFFFFFF800000007F

    ; ================================================================
    ; PMOVSXBQ: bytes 01 FE
    ; 01->0000000000000001, FE=-2->FFFFFFFFFFFFFFFE
    ; ================================================================
    movdqa xmm0, [rel vec_sx_bq]
    pmovsxbq xmm1, xmm0

    TEST_CASE tn_sxbq_lo
    movq rax, xmm1
    CHECK_EQ_64 rax, 0x0000000000000001

    TEST_CASE tn_sxbq_hi
    pextrq rax, xmm1, 1
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFE

    ; ================================================================
    ; PMOVSXWD positive: words 0001 0002 0003 0004
    ; -> dwords: 00000001 00000002 00000003 00000004
    ; lo64 = 00000002_00000001, hi64 = 00000004_00000003
    ; ================================================================
    movdqa xmm0, [rel vec_sx_wd_pos]
    pmovsxwd xmm1, xmm0

    TEST_CASE tn_sxwd_pos_lo
    movq rax, xmm1
    CHECK_EQ_64 rax, 0x0000000200000001

    TEST_CASE tn_sxwd_pos_hi
    pextrq rax, xmm1, 1
    CHECK_EQ_64 rax, 0x0000000400000003

    ; ================================================================
    ; PMOVSXWD negative: words FFFF 8000 7FFF FE00
    ; FFFF=-1->FFFFFFFF, 8000=-32768->FFFF8000
    ; 7FFF=32767->00007FFF, FE00=-512->FFFFFE00
    ; lo64 = FFFF8000_FFFFFFFF, hi64 = FFFFFE00_00007FFF
    ; ================================================================
    movdqa xmm0, [rel vec_sx_wd_neg]
    pmovsxwd xmm1, xmm0

    TEST_CASE tn_sxwd_neg_lo
    movq rax, xmm1
    CHECK_EQ_64 rax, 0xFFFF8000FFFFFFFF

    TEST_CASE tn_sxwd_neg_hi
    pextrq rax, xmm1, 1
    CHECK_EQ_64 rax, 0xFFFFFE0000007FFF

    ; ================================================================
    ; PMOVSXWQ: words 0001 FFFE
    ; 0001->0000000000000001, FFFE=-2->FFFFFFFFFFFFFFFE
    ; ================================================================
    movdqa xmm0, [rel vec_sx_wq]
    pmovsxwq xmm1, xmm0

    TEST_CASE tn_sxwq_lo
    movq rax, xmm1
    CHECK_EQ_64 rax, 0x0000000000000001

    TEST_CASE tn_sxwq_hi
    pextrq rax, xmm1, 1
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFE

    ; ================================================================
    ; PMOVSXDQ: dwords 00000001 FFFFFFFE
    ; 00000001->0000000000000001, FFFFFFFE=-2->FFFFFFFFFFFFFFFE
    ; ================================================================
    movdqa xmm0, [rel vec_sx_dq]
    pmovsxdq xmm1, xmm0

    TEST_CASE tn_sxdq_lo
    movq rax, xmm1
    CHECK_EQ_64 rax, 0x0000000000000001

    TEST_CASE tn_sxdq_hi
    pextrq rax, xmm1, 1
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFE

    ; ================================================================
    ; PMOVZXBW positive bytes (same as sign-extend for positive)
    ; bytes: 01 02 03 04 05 06 07 08
    ; lo64 = 0004_0003_0002_0001, hi64 = 0008_0007_0006_0005
    ; ================================================================
    movdqa xmm0, [rel vec_sx_bw_pos]
    pmovzxbw xmm1, xmm0

    TEST_CASE tn_zxbw_pos_lo
    movq rax, xmm1
    CHECK_EQ_64 rax, 0x0004000300020001

    TEST_CASE tn_zxbw_pos_hi
    pextrq rax, xmm1, 1
    CHECK_EQ_64 rax, 0x0008000700060005

    ; ================================================================
    ; PMOVZXBW high bytes (zero-extend, NOT sign-extend)
    ; bytes: FF 80 7F FE 01 00 81 82
    ; FF->00FF, 80->0080, 7F->007F, FE->00FE
    ; 01->0001, 00->0000, 81->0081, 82->0082
    ; lo64 = 00FE_007F_0080_00FF, hi64 = 0082_0081_0000_0001
    ; ================================================================
    movdqa xmm0, [rel vec_sx_bw_neg]
    pmovzxbw xmm1, xmm0

    TEST_CASE tn_zxbw_hi_lo
    movq rax, xmm1
    CHECK_EQ_64 rax, 0x00FE007F008000FF

    TEST_CASE tn_zxbw_hi_hi
    pextrq rax, xmm1, 1
    CHECK_EQ_64 rax, 0x0082008100000001

    ; ================================================================
    ; PMOVZXBD: bytes FF 80 7F 01
    ; FF->000000FF, 80->00000080, 7F->0000007F, 01->00000001
    ; lo64 = 00000080_000000FF, hi64 = 00000001_0000007F
    ; ================================================================
    movdqa xmm0, [rel vec_zx_bd]
    pmovzxbd xmm1, xmm0

    TEST_CASE tn_zxbd_lo
    movq rax, xmm1
    CHECK_EQ_64 rax, 0x00000080000000FF

    TEST_CASE tn_zxbd_hi
    pextrq rax, xmm1, 1
    CHECK_EQ_64 rax, 0x000000010000007F

    ; ================================================================
    ; PMOVZXBQ: bytes FF 80
    ; FF->00000000000000FF, 80->0000000000000080
    ; ================================================================
    movdqa xmm0, [rel vec_zx_bq]
    pmovzxbq xmm1, xmm0

    TEST_CASE tn_zxbq_lo
    movq rax, xmm1
    CHECK_EQ_64 rax, 0x00000000000000FF

    TEST_CASE tn_zxbq_hi
    pextrq rax, xmm1, 1
    CHECK_EQ_64 rax, 0x0000000000000080

    ; ================================================================
    ; PMOVZXWD: words FFFF 8000 7FFF 0001
    ; FFFF->0000FFFF, 8000->00008000, 7FFF->00007FFF, 0001->00000001
    ; lo64 = 00008000_0000FFFF, hi64 = 00000001_00007FFF
    ; ================================================================
    movdqa xmm0, [rel vec_zx_wd]
    pmovzxwd xmm1, xmm0

    TEST_CASE tn_zxwd_lo
    movq rax, xmm1
    CHECK_EQ_64 rax, 0x000080000000FFFF

    TEST_CASE tn_zxwd_hi
    pextrq rax, xmm1, 1
    CHECK_EQ_64 rax, 0x0000000100007FFF

    ; ================================================================
    ; PMOVZXWQ: words FFFF 8000
    ; FFFF->000000000000FFFF, 8000->0000000000008000
    ; ================================================================
    movdqa xmm0, [rel vec_zx_wq]
    pmovzxwq xmm1, xmm0

    TEST_CASE tn_zxwq_lo
    movq rax, xmm1
    CHECK_EQ_64 rax, 0x000000000000FFFF

    TEST_CASE tn_zxwq_hi
    pextrq rax, xmm1, 1
    CHECK_EQ_64 rax, 0x0000000000008000

    ; ================================================================
    ; PMOVZXDQ: dwords FFFFFFFF 80000001
    ; FFFFFFFF->00000000FFFFFFFF, 80000001->0000000080000001
    ; ================================================================
    movdqa xmm0, [rel vec_zx_dq]
    pmovzxdq xmm1, xmm0

    TEST_CASE tn_zxdq_lo
    movq rax, xmm1
    CHECK_EQ_64 rax, 0x00000000FFFFFFFF

    TEST_CASE tn_zxdq_hi
    pextrq rax, xmm1, 1
    CHECK_EQ_64 rax, 0x0000000080000001

    ; ================================================================
    ; PMINSB signed mix
    ; a: 05 FF 00 80 7F 01 FE 10  20 30 40 50 60 70 80 90
    ; b: 10 01 FF 7F 80 FF 02 20  10 40 30 60 50 80 70 A0
    ; Signed byte-by-byte min:
    ;   min(05,10)=05  min(-1,01)=FF   min(00,-1)=FF   min(-128,7F)=80
    ;   min(7F,-128)=80 min(01,-1)=FF  min(-2,02)=FE   min(16,32)=10
    ; lo64 bytes[7..0] = 10 FE FF 80 80 FF FF 05 = 0x10FEFF8080FFFF05
    ;   min(32,16)=10  min(48,64)=30   min(64,48)=30   min(80,96)=50
    ;   min(96,80)=50  min(112,-128)=80 min(-128,112)=80 min(-112,-96)=90
    ; hi64 bytes[15..8] = 90 80 80 50 50 30 30 10 = 0x9080805050303010
    ; ================================================================
    movdqa xmm0, [rel vec_minmax_a]
    movdqa xmm1, [rel vec_minmax_b]
    pminsb xmm0, xmm1

    TEST_CASE tn_minsb_mix_lo
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x10FEFF8080FFFF05

    TEST_CASE tn_minsb_mix_hi
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x9080805050303010

    ; ================================================================
    ; PMINSB all positive
    ; a: 01 02 03 04 05 06 07 08  09 0A 0B 0C 0D 0E 0F 10
    ; b: 10 0F 0E 0D 0C 0B 0A 09  08 07 06 05 04 03 02 01
    ; lo64 = 08 07 06 05 04 03 02 01 = 0x0807060504030201
    ; hi64 = 01 02 03 04 05 06 07 08 = 0x0102030405060708
    ; ================================================================
    movdqa xmm0, [rel vec_pos_a]
    movdqa xmm1, [rel vec_pos_b]
    pminsb xmm0, xmm1

    TEST_CASE tn_minsb_pos_lo
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0807060504030201

    TEST_CASE tn_minsb_pos_hi
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x0102030405060708

    ; ================================================================
    ; PMINSD signed dwords
    ; a: 00000001 FFFFFFFF 7FFFFFFF 80000000
    ; b: FFFFFFFF 00000001 80000000 7FFFFFFF
    ; Signed: min(1,-1)=-1  min(-1,1)=-1  min(MAX,MIN)=MIN  min(MIN,MAX)=MIN
    ; lo64 = FFFFFFFF_FFFFFFFF, hi64 = 80000000_80000000
    ; ================================================================
    movdqa xmm0, [rel vec_sd_a]
    movdqa xmm1, [rel vec_sd_b]
    pminsd xmm0, xmm1

    TEST_CASE tn_minsd_lo
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFF

    TEST_CASE tn_minsd_hi
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x8000000080000000

    ; ================================================================
    ; PMINUD unsigned dwords
    ; a: 00000001 FFFFFFFF 7FFFFFFF 80000000
    ; b: FFFFFFFF 00000001 80000000 7FFFFFFF
    ; Unsigned: min(1,big)=1  min(big,1)=1  min(7F..,80..)=7F..  min(80..,7F..)=7F..
    ; lo64 = 00000001_00000001, hi64 = 7FFFFFFF_7FFFFFFF
    ; ================================================================
    movdqa xmm0, [rel vec_sd_a]
    movdqa xmm1, [rel vec_sd_b]
    pminud xmm0, xmm1

    TEST_CASE tn_minud_lo
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0000000100000001

    TEST_CASE tn_minud_hi
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x7FFFFFFF7FFFFFFF

    ; ================================================================
    ; PMINUW unsigned words
    ; a: 0001 FFFF 7FFF 8000  0100 FF00 00FF 8001
    ; b: FFFF 0001 8000 7FFF  FF00 0100 8001 00FF
    ; lo64 = 7FFF_7FFF_0001_0001 = 0x7FFF7FFF00010001
    ; hi64 = 00FF_00FF_0100_0100 = 0x00FF00FF01000100
    ; ================================================================
    movdqa xmm0, [rel vec_uw_a]
    movdqa xmm1, [rel vec_uw_b]
    pminuw xmm0, xmm1

    TEST_CASE tn_minuw_lo
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x7FFF7FFF00010001

    TEST_CASE tn_minuw_hi
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x00FF00FF01000100

    ; ================================================================
    ; PMAXSB signed mix (same vectors as pminsb mix)
    ; Signed byte-by-byte max:
    ;   max(05,10)=10  max(-1,01)=01   max(00,-1)=00   max(-128,7F)=7F
    ;   max(7F,-128)=7F max(01,-1)=01  max(-2,02)=02   max(16,32)=20
    ; lo64 = 20 02 01 7F 7F 00 01 10 = 0x2002017F7F000110
    ;   max(32,16)=20  max(48,64)=40   max(64,48)=40   max(80,96)=60
    ;   max(96,80)=60  max(112,-128)=70 max(-128,112)=70 max(-112,-96)=A0
    ; hi64 = A0 70 70 60 60 40 40 20 = 0xA070706060404020
    ; ================================================================
    movdqa xmm0, [rel vec_minmax_a]
    movdqa xmm1, [rel vec_minmax_b]
    pmaxsb xmm0, xmm1

    TEST_CASE tn_maxsb_mix_lo
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x2002017F7F000110

    TEST_CASE tn_maxsb_mix_hi
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0xA070706060404020

    ; ================================================================
    ; PMAXSB all positive
    ; lo64 = 09 0A 0B 0C 0D 0E 0F 10 = 0x090A0B0C0D0E0F10
    ; hi64 = 10 0F 0E 0D 0C 0B 0A 09 = 0x100F0E0D0C0B0A09
    ; ================================================================
    movdqa xmm0, [rel vec_pos_a]
    movdqa xmm1, [rel vec_pos_b]
    pmaxsb xmm0, xmm1

    TEST_CASE tn_maxsb_pos_lo
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x090A0B0C0D0E0F10

    TEST_CASE tn_maxsb_pos_hi
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x100F0E0D0C0B0A09

    ; ================================================================
    ; PMAXSD signed dwords
    ; Signed: max(1,-1)=1  max(-1,1)=1  max(MAX,MIN)=MAX  max(MIN,MAX)=MAX
    ; lo64 = 00000001_00000001, hi64 = 7FFFFFFF_7FFFFFFF
    ; ================================================================
    movdqa xmm0, [rel vec_sd_a]
    movdqa xmm1, [rel vec_sd_b]
    pmaxsd xmm0, xmm1

    TEST_CASE tn_maxsd_lo
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0000000100000001

    TEST_CASE tn_maxsd_hi
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x7FFFFFFF7FFFFFFF

    ; ================================================================
    ; PMAXUD unsigned dwords
    ; Unsigned: max(1,big)=big  max(big,1)=big  max(7F..,80..)=80..  max(80..,7F..)=80..
    ; lo64 = FFFFFFFF_FFFFFFFF, hi64 = 80000000_80000000
    ; ================================================================
    movdqa xmm0, [rel vec_sd_a]
    movdqa xmm1, [rel vec_sd_b]
    pmaxud xmm0, xmm1

    TEST_CASE tn_maxud_lo
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFF

    TEST_CASE tn_maxud_hi
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x8000000080000000

    ; ================================================================
    ; PMAXUW unsigned words
    ; lo64 = 8000_8000_FFFF_FFFF = 0x80008000FFFFFFFF
    ; hi64 = 8001_8001_FF00_FF00 = 0x80018001FF00FF00
    ; ================================================================
    movdqa xmm0, [rel vec_uw_a]
    movdqa xmm1, [rel vec_uw_b]
    pmaxuw xmm0, xmm1

    TEST_CASE tn_maxuw_lo
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x80008000FFFFFFFF

    TEST_CASE tn_maxuw_hi
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x80018001FF00FF00

    ; ================================================================
    ; PHMINPOSUW basic
    ; words: 0005 0003 0001 0004 0002 0006 0007 0008
    ; Min unsigned word = 0001 at index 2
    ; Result: word[0]=min=0001, word[1]=index=0002, rest=0
    ; lo64 = 0x0000000000020001, hi64 = 0
    ; ================================================================
    movdqa xmm0, [rel vec_phmin1]
    phminposuw xmm0, xmm0

    TEST_CASE tn_phmin1_lo
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0000000000020001

    TEST_CASE tn_phmin1_hi
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x0000000000000000

    ; ================================================================
    ; PHMINPOSUW with zero
    ; words: FFFF 0080 00FF 0001 0100 0000 8000 7FFF
    ; Min unsigned word = 0000 at index 5
    ; Result: word[0]=0000, word[1]=0005, rest=0
    ; lo64 = 0x0000000000050000, hi64 = 0
    ; ================================================================
    movdqa xmm0, [rel vec_phmin2]
    phminposuw xmm0, xmm0

    TEST_CASE tn_phmin2_lo
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0000000000050000

    TEST_CASE tn_phmin2_hi
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x0000000000000000

    END_TESTS
