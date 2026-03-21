; test_sse_insert_extract.asm - Test SSE4.1 insert/extract instructions and PTEST
; PINSRB, PINSRW, PINSRD, PINSRQ, PEXTRB, PEXTRW, PEXTRD, PEXTRQ, PTEST, INSERTPS
%include "test_framework.inc"

section .data
    ; ---- Test name strings ----
    tn_pinsrb_p0_lo:    db "pinsrb pos0 lo64", 0
    tn_pinsrb_p0_hi:    db "pinsrb pos0 hi64", 0
    tn_pinsrb_p7_lo:    db "pinsrb pos7 lo64", 0
    tn_pinsrb_p15_hi:   db "pinsrb pos15 hi64", 0

    tn_pinsrw_p0_lo:    db "pinsrw pos0 lo64", 0
    tn_pinsrw_p3_lo:    db "pinsrw pos3 lo64", 0
    tn_pinsrw_p7_hi:    db "pinsrw pos7 hi64", 0

    tn_pinsrd_p0_lo:    db "pinsrd pos0 lo64", 0
    tn_pinsrd_p1_lo:    db "pinsrd pos1 lo64", 0
    tn_pinsrd_p2_hi:    db "pinsrd pos2 hi64", 0
    tn_pinsrd_p3_hi:    db "pinsrd pos3 hi64", 0

    tn_pinsrq_p0_lo:    db "pinsrq pos0 lo64", 0
    tn_pinsrq_p1_hi:    db "pinsrq pos1 hi64", 0

    tn_pextrb_p0:       db "pextrb pos0", 0
    tn_pextrb_p5:       db "pextrb pos5", 0
    tn_pextrb_p15:      db "pextrb pos15", 0

    tn_pextrw_p0:       db "pextrw pos0", 0
    tn_pextrw_p3:       db "pextrw pos3", 0
    tn_pextrw_p7:       db "pextrw pos7", 0

    tn_pextrd_p0:       db "pextrd pos0", 0
    tn_pextrd_p1:       db "pextrd pos1", 0
    tn_pextrd_p2:       db "pextrd pos2", 0
    tn_pextrd_p3:       db "pextrd pos3", 0

    tn_pextrq_p0:       db "pextrq pos0", 0
    tn_pextrq_p1:       db "pextrq pos1", 0

    tn_ptest_zero:      db "ptest both zero", 0
    tn_ptest_disjoint:  db "ptest disjoint", 0
    tn_ptest_subset:    db "ptest subset", 0
    tn_ptest_overlap:   db "ptest overlap", 0

    tn_insertps_nz_lo:  db "insertps no-zero lo64", 0
    tn_insertps_nz_hi:  db "insertps no-zero hi64", 0
    tn_insertps_zm_lo:  db "insertps zmask lo64", 0
    tn_insertps_zm_hi:  db "insertps zmask hi64", 0

    align 16
    ; ---- Test vectors ----

    ; Base XMM: bytes 0-15 = their index value
    ; Memory layout: 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
    ; lo64 = 0x0706050403020100, hi64 = 0x0F0E0D0C0B0A0908
    vec_base:   db 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07
                db 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F

    ; PTEST vectors
    vec_zeros:      dq 0x0000000000000000, 0x0000000000000000
    vec_0f_pat:     dq 0x0F0F0F0F0F0F0F0F, 0x0F0F0F0F0F0F0F0F
    vec_f0_pat:     dq 0xF0F0F0F0F0F0F0F0, 0xF0F0F0F0F0F0F0F0
    vec_all_ones:   dq 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF

    ; INSERTPS float vectors
    ; dst = {1.0f, 2.0f, 3.0f, 4.0f}  (dword0=1.0, dword1=2.0, dword2=3.0, dword3=4.0)
    ; 1.0f=0x3F800000, 2.0f=0x40000000, 3.0f=0x40400000, 4.0f=0x40800000
    vec_fps_dst:    dd 0x3F800000, 0x40000000, 0x40400000, 0x40800000

    ; src = {5.0f, 6.0f, 7.0f, 8.0f}
    ; 5.0f=0x40A00000, 6.0f=0x40C00000, 7.0f=0x40E00000, 8.0f=0x41000000
    vec_fps_src:    dd 0x40A00000, 0x40C00000, 0x40E00000, 0x41000000

section .text
global _start

_start:
    INIT_TESTS

    ; ================================================================
    ; PINSRB - insert byte into XMM
    ; ================================================================

    ; PINSRB position 0: insert 0xFF at byte 0
    ; Base lo64 = 0x0706050403020100 -> byte 0 becomes 0xFF
    ; Result lo64 = 0x07060504030201FF
    ; hi64 unchanged = 0x0F0E0D0C0B0A0908
    movdqa xmm0, [rel vec_base]
    mov eax, 0xFF
    pinsrb xmm0, eax, 0

    TEST_CASE tn_pinsrb_p0_lo
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x07060504030201FF

    TEST_CASE tn_pinsrb_p0_hi
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x0F0E0D0C0B0A0908

    ; PINSRB position 7: insert 0xAB at byte 7
    ; Base lo64 = 0x0706050403020100 -> byte 7 becomes 0xAB
    ; Result lo64 = 0xAB06050403020100
    movdqa xmm0, [rel vec_base]
    mov eax, 0xAB
    pinsrb xmm0, eax, 7

    TEST_CASE tn_pinsrb_p7_lo
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xAB06050403020100

    ; PINSRB position 15: insert 0x42 at byte 15
    ; Base hi64 = 0x0F0E0D0C0B0A0908 -> byte 15 (topmost byte of hi64) becomes 0x42
    ; Result hi64 = 0x420E0D0C0B0A0908
    movdqa xmm0, [rel vec_base]
    mov eax, 0x42
    pinsrb xmm0, eax, 15

    TEST_CASE tn_pinsrb_p15_hi
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x420E0D0C0B0A0908

    ; ================================================================
    ; PINSRW - insert word into XMM
    ; ================================================================

    ; PINSRW position 0: insert 0xBEEF at word 0 (bytes 0-1)
    ; Base lo64 = 0x0706050403020100 -> word 0 becomes 0xBEEF
    ; Result lo64 = 0x070605040302BEEF
    movdqa xmm0, [rel vec_base]
    mov eax, 0xBEEF
    pinsrw xmm0, eax, 0

    TEST_CASE tn_pinsrw_p0_lo
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x070605040302BEEF

    ; PINSRW position 3: insert 0xCAFE at word 3 (bytes 6-7)
    ; Base lo64 = 0x0706050403020100 -> word 3 (top word of lo64) becomes 0xCAFE
    ; Result lo64 = 0xCAFE050403020100
    movdqa xmm0, [rel vec_base]
    mov eax, 0xCAFE
    pinsrw xmm0, eax, 3

    TEST_CASE tn_pinsrw_p3_lo
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xCAFE050403020100

    ; PINSRW position 7: insert 0xDEAD at word 7 (bytes 14-15)
    ; Base hi64 = 0x0F0E0D0C0B0A0908 -> word 7 (top word of hi64) becomes 0xDEAD
    ; Result hi64 = 0xDEAD0D0C0B0A0908
    movdqa xmm0, [rel vec_base]
    mov eax, 0xDEAD
    pinsrw xmm0, eax, 7

    TEST_CASE tn_pinsrw_p7_hi
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0xDEAD0D0C0B0A0908

    ; ================================================================
    ; PINSRD - insert dword into XMM
    ; ================================================================

    ; PINSRD position 0: insert 0xDEADBEEF at dword 0 (bytes 0-3)
    ; Base lo64 = 0x0706050403020100 -> dword 0 becomes 0xDEADBEEF
    ; Result lo64 = 0x07060504DEADBEEF
    movdqa xmm0, [rel vec_base]
    mov eax, 0xDEADBEEF
    pinsrd xmm0, eax, 0

    TEST_CASE tn_pinsrd_p0_lo
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x07060504DEADBEEF

    ; PINSRD position 1: insert 0xCAFEBABE at dword 1 (bytes 4-7)
    ; Base lo64 = 0x0706050403020100 -> dword 1 becomes 0xCAFEBABE
    ; Result lo64 = 0xCAFEBABE03020100
    movdqa xmm0, [rel vec_base]
    mov eax, 0xCAFEBABE
    pinsrd xmm0, eax, 1

    TEST_CASE tn_pinsrd_p1_lo
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xCAFEBABE03020100

    ; PINSRD position 2: insert 0x12345678 at dword 2 (bytes 8-11)
    ; Base hi64 = 0x0F0E0D0C0B0A0908 -> dword 2 (lo dword of hi64) becomes 0x12345678
    ; Result hi64 = 0x0F0E0D0C12345678
    movdqa xmm0, [rel vec_base]
    mov eax, 0x12345678
    pinsrd xmm0, eax, 2

    TEST_CASE tn_pinsrd_p2_hi
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x0F0E0D0C12345678

    ; PINSRD position 3: insert 0xFFFFFFFF at dword 3 (bytes 12-15)
    ; Base hi64 = 0x0F0E0D0C0B0A0908 -> dword 3 (hi dword of hi64) becomes 0xFFFFFFFF
    ; Result hi64 = 0xFFFFFFFF0B0A0908
    movdqa xmm0, [rel vec_base]
    mov eax, 0xFFFFFFFF
    pinsrd xmm0, eax, 3

    TEST_CASE tn_pinsrd_p3_hi
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0xFFFFFFFF0B0A0908

    ; ================================================================
    ; PINSRQ - insert qword into XMM
    ; ================================================================

    ; PINSRQ position 0: insert 0xDEADBEEFCAFEBABE at qword 0
    ; Result lo64 = 0xDEADBEEFCAFEBABE
    movdqa xmm0, [rel vec_base]
    mov rax, 0xDEADBEEFCAFEBABE
    pinsrq xmm0, rax, 0

    TEST_CASE tn_pinsrq_p0_lo
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xDEADBEEFCAFEBABE

    ; PINSRQ position 1: insert 0x123456789ABCDEF0 at qword 1
    ; Result hi64 = 0x123456789ABCDEF0
    movdqa xmm0, [rel vec_base]
    mov rax, 0x123456789ABCDEF0
    pinsrq xmm0, rax, 1

    TEST_CASE tn_pinsrq_p1_hi
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x123456789ABCDEF0

    ; ================================================================
    ; PEXTRB - extract byte from XMM (zero-extended to 64-bit reg)
    ; Base: bytes 0-15 = their index
    ; ================================================================

    ; PEXTRB position 0: byte 0 = 0x00
    movdqa xmm0, [rel vec_base]
    pextrb eax, xmm0, 0

    TEST_CASE tn_pextrb_p0
    CHECK_EQ_64 rax, 0x0000000000000000

    ; PEXTRB position 5: byte 5 = 0x05
    movdqa xmm0, [rel vec_base]
    pextrb eax, xmm0, 5

    TEST_CASE tn_pextrb_p5
    CHECK_EQ_64 rax, 0x0000000000000005

    ; PEXTRB position 15: byte 15 = 0x0F
    movdqa xmm0, [rel vec_base]
    pextrb eax, xmm0, 15

    TEST_CASE tn_pextrb_p15
    CHECK_EQ_64 rax, 0x000000000000000F

    ; ================================================================
    ; PEXTRW - extract word from XMM (zero-extended to 32-bit reg)
    ; Word layout: w0=0x0100, w1=0x0302, w2=0x0504, w3=0x0706
    ;              w4=0x0908, w5=0x0B0A, w6=0x0D0C, w7=0x0F0E
    ; ================================================================

    ; PEXTRW position 0: word 0 = 0x0100
    movdqa xmm0, [rel vec_base]
    pextrw eax, xmm0, 0

    TEST_CASE tn_pextrw_p0
    CHECK_EQ_32 eax, 0x00000100

    ; PEXTRW position 3: word 3 = 0x0706
    movdqa xmm0, [rel vec_base]
    pextrw eax, xmm0, 3

    TEST_CASE tn_pextrw_p3
    CHECK_EQ_32 eax, 0x00000706

    ; PEXTRW position 7: word 7 = 0x0F0E
    movdqa xmm0, [rel vec_base]
    pextrw eax, xmm0, 7

    TEST_CASE tn_pextrw_p7
    CHECK_EQ_32 eax, 0x00000F0E

    ; ================================================================
    ; PEXTRD - extract dword from XMM
    ; Dword layout: d0=0x03020100, d1=0x07060504
    ;               d2=0x0B0A0908, d3=0x0F0E0D0C
    ; ================================================================

    ; PEXTRD position 0: dword 0 = 0x03020100
    movdqa xmm0, [rel vec_base]
    pextrd eax, xmm0, 0

    TEST_CASE tn_pextrd_p0
    CHECK_EQ_32 eax, 0x03020100

    ; PEXTRD position 1: dword 1 = 0x07060504
    movdqa xmm0, [rel vec_base]
    pextrd eax, xmm0, 1

    TEST_CASE tn_pextrd_p1
    CHECK_EQ_32 eax, 0x07060504

    ; PEXTRD position 2: dword 2 = 0x0B0A0908
    movdqa xmm0, [rel vec_base]
    pextrd eax, xmm0, 2

    TEST_CASE tn_pextrd_p2
    CHECK_EQ_32 eax, 0x0B0A0908

    ; PEXTRD position 3: dword 3 = 0x0F0E0D0C
    movdqa xmm0, [rel vec_base]
    pextrd eax, xmm0, 3

    TEST_CASE tn_pextrd_p3
    CHECK_EQ_32 eax, 0x0F0E0D0C

    ; ================================================================
    ; PEXTRQ - extract qword from XMM
    ; ================================================================

    ; PEXTRQ position 0: qword 0 = 0x0706050403020100
    movdqa xmm0, [rel vec_base]
    pextrq rax, xmm0, 0

    TEST_CASE tn_pextrq_p0
    CHECK_EQ_64 rax, 0x0706050403020100

    ; PEXTRQ position 1: qword 1 = 0x0F0E0D0C0B0A0908
    movdqa xmm0, [rel vec_base]
    pextrq rax, xmm0, 1

    TEST_CASE tn_pextrq_p1
    CHECK_EQ_64 rax, 0x0F0E0D0C0B0A0908

    ; ================================================================
    ; PTEST - logical compare setting ZF and CF
    ; ZF = 1 if (xmm1 AND xmm2) == 0
    ; CF = 1 if (NOT(xmm1) AND xmm2) == 0
    ; ================================================================

    ; Test 1: both zero -> AND=0 (ZF=1), NOT(0)=all_ones AND 0 = 0 (CF=1)
    ; Expected: ZF=1, CF=1 -> flags & (ZF|CF) = 0x0041
    movdqa xmm0, [rel vec_zeros]
    movdqa xmm1, [rel vec_zeros]
    ptest xmm0, xmm1
    SAVE_FLAGS

    TEST_CASE tn_ptest_zero
    CHECK_FLAGS_EQ (ZF|CF), 0x0041

    ; Test 2: disjoint bits -> xmm1=0x0F0F..., xmm2=0xF0F0...
    ; AND = 0 (ZF=1), NOT(0x0F0F...)=0xF0F0..., AND 0xF0F0... = 0xF0F0... != 0 (CF=0)
    ; Expected: ZF=1, CF=0 -> flags & (ZF|CF) = 0x0040
    movdqa xmm0, [rel vec_0f_pat]
    movdqa xmm1, [rel vec_f0_pat]
    ptest xmm0, xmm1
    SAVE_FLAGS

    TEST_CASE tn_ptest_disjoint
    CHECK_FLAGS_EQ (ZF|CF), 0x0040

    ; Test 3: subset -> xmm1=all_ones, xmm2=0x0F0F...
    ; AND = 0x0F0F... != 0 (ZF=0), NOT(all_ones)=0, AND 0x0F0F... = 0 (CF=1)
    ; Expected: ZF=0, CF=1 -> flags & (ZF|CF) = 0x0001
    movdqa xmm0, [rel vec_all_ones]
    movdqa xmm1, [rel vec_0f_pat]
    ptest xmm0, xmm1
    SAVE_FLAGS

    TEST_CASE tn_ptest_subset
    CHECK_FLAGS_EQ (ZF|CF), 0x0001

    ; Test 4: overlapping -> xmm1=0x0F0F..., xmm2=all_ones
    ; AND = 0x0F0F... != 0 (ZF=0), NOT(0x0F0F...)=0xF0F0..., AND all_ones = 0xF0F0... != 0 (CF=0)
    ; Expected: ZF=0, CF=0 -> flags & (ZF|CF) = 0x0000
    movdqa xmm0, [rel vec_0f_pat]
    movdqa xmm1, [rel vec_all_ones]
    ptest xmm0, xmm1
    SAVE_FLAGS

    TEST_CASE tn_ptest_overlap
    CHECK_FLAGS_EQ (ZF|CF), 0x0000

    ; ================================================================
    ; INSERTPS - insert single-precision float with zero mask
    ; imm8: [7:6]=src_dword_index, [5:4]=dst_dword_index, [3:0]=zero_mask
    ; ================================================================

    ; Test 1: insert src[0] into dst[2], zero_mask=0 (no zeroing)
    ; imm8 = (0 << 6) | (2 << 4) | 0x0 = 0x20
    ; dst = {1.0, 2.0, 3.0, 4.0}, src = {5.0, 6.0, 7.0, 8.0}
    ; Result: {1.0, 2.0, 5.0, 4.0}
    ; dword0=0x3F800000 (1.0), dword1=0x40000000 (2.0), dword2=0x40A00000 (5.0), dword3=0x40800000 (4.0)
    ; lo64 = 0x400000003F800000, hi64 = 0x4080000040A00000
    movdqa xmm0, [rel vec_fps_dst]
    movdqa xmm1, [rel vec_fps_src]
    insertps xmm0, xmm1, 0x20

    TEST_CASE tn_insertps_nz_lo
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x400000003F800000

    TEST_CASE tn_insertps_nz_hi
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x4080000040A00000

    ; Test 2: insert src[1] into dst[0], zero_mask=0b0110 (zero dword1 and dword2)
    ; imm8 = (1 << 6) | (0 << 4) | 0x6 = 0x46
    ; dst = {1.0, 2.0, 3.0, 4.0}, src = {5.0, 6.0, 7.0, 8.0}
    ; dst[0] = src[1] = 6.0f = 0x40C00000
    ; dst[1] = 0 (zeroed by bit 1 of zmask)
    ; dst[2] = 0 (zeroed by bit 2 of zmask)
    ; dst[3] = 4.0f = 0x40800000 (not zeroed)
    ; lo64 = 0x0000000040C00000, hi64 = 0x4080000000000000
    movdqa xmm0, [rel vec_fps_dst]
    movdqa xmm1, [rel vec_fps_src]
    insertps xmm0, xmm1, 0x46

    TEST_CASE tn_insertps_zm_lo
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0000000040C00000

    TEST_CASE tn_insertps_zm_hi
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x4080000000000000

    END_TESTS
