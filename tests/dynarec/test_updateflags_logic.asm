; test_updateflags_logic.asm - Deferred flag tests for remaining logic ops
; Covers: and8/16/64, or8/16/64, xor8/16/64, test16
; Each test: instruction -> call flag_barrier_ret -> check flags
; Logic ops always clear CF and OF, set ZF/SF/PF based on result
%include "test_framework.inc"

section .data
    ; --- and8 ---
    tand8_1:  db "and8 zero result via call", 0
    tand8_2:  db "and8 nonzero result via call", 0
    tand8_3:  db "and8 sign result via call", 0
    ; --- and16 ---
    tand16_1: db "and16 zero result via call", 0
    tand16_2: db "and16 nonzero result via call", 0
    tand16_3: db "and16 sign result via call", 0
    ; --- and64 ---
    tand64_1: db "and64 zero result via call", 0
    tand64_2: db "and64 nonzero result via call", 0
    tand64_3: db "and64 sign result via call", 0
    ; --- or8 ---
    tor8_1:   db "or8 zero result via call", 0
    tor8_2:   db "or8 nonzero result via call", 0
    tor8_3:   db "or8 sign result via call", 0
    ; --- or16 ---
    tor16_1:  db "or16 zero result via call", 0
    tor16_2:  db "or16 nonzero result via call", 0
    tor16_3:  db "or16 sign result via call", 0
    ; --- or64 ---
    tor64_1:  db "or64 zero result via call", 0
    tor64_2:  db "or64 nonzero result via call", 0
    tor64_3:  db "or64 sign result via call", 0
    ; --- xor8 ---
    txor8_1:  db "xor8 zero result via call", 0
    txor8_2:  db "xor8 nonzero result via call", 0
    txor8_3:  db "xor8 sign result via call", 0
    ; --- xor16 ---
    txor16_1: db "xor16 zero result via call", 0
    txor16_2: db "xor16 nonzero result via call", 0
    txor16_3: db "xor16 sign result via call", 0
    ; --- xor64 ---
    txor64_1: db "xor64 zero result via call", 0
    txor64_2: db "xor64 nonzero result via call", 0
    txor64_3: db "xor64 sign result via call", 0
    ; --- test16 ---
    ttst16_1: db "test16 zero result via call", 0
    ttst16_2: db "test16 nonzero result via call", 0
    ttst16_3: db "test16 sign result via call", 0
    ; --- CF/OF cleared by logic ops ---
    tlogic_cf1: db "and8 clears CF via call", 0
    tlogic_cf2: db "or16 clears CF via call", 0
    tlogic_cf3: db "xor64 clears CF via call", 0

section .text
global _start

flag_barrier_ret:
    ret

_start:
    INIT_TESTS

    ;; ================================================================
    ;; AND 8-bit
    ;; ================================================================

    ; and8 zero: 0xF0 & 0x0F = 0 -> ZF=1, CF=0, OF=0
    TEST_CASE tand8_1
    mov al, 0xF0
    and al, 0x0F
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|OF|SF), ZF

    ; and8 nonzero: 0xFF & 0x0F = 0x0F -> ZF=0, SF=0
    TEST_CASE tand8_2
    mov al, 0xFF
    and al, 0x0F
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|OF|SF), 0

    ; and8 sign: 0xFF & 0x80 = 0x80 -> SF=1
    TEST_CASE tand8_3
    mov al, 0xFF
    and al, 0x80
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|OF|SF), SF

    ;; ================================================================
    ;; AND 16-bit
    ;; ================================================================

    ; and16 zero: 0xFF00 & 0x00FF = 0 -> ZF=1
    TEST_CASE tand16_1
    mov ax, 0xFF00
    and ax, 0x00FF
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|OF|SF), ZF

    ; and16 nonzero: 0xFFFF & 0x1234 = 0x1234 -> ZF=0, SF=0
    TEST_CASE tand16_2
    mov ax, 0xFFFF
    mov bx, 0x1234
    and ax, bx
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|OF|SF), 0

    ; and16 sign: 0xFFFF & 0x8000 = 0x8000 -> SF=1
    TEST_CASE tand16_3
    mov ax, 0xFFFF
    and ax, 0x8000
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|OF|SF), SF

    ;; ================================================================
    ;; AND 64-bit
    ;; ================================================================

    ; and64 zero: 0xFFFF0000 & 0x0000FFFF = 0 -> ZF=1
    TEST_CASE tand64_1
    mov rax, 0xFFFF000000000000
    mov rbx, 0x0000FFFFFFFFFFFF
    and rax, rbx
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|OF|SF), ZF

    ; and64 nonzero: large & mask -> nonzero
    TEST_CASE tand64_2
    mov rax, 0x123456789ABCDEF0
    mov rbx, 0x00000000FFFFFFFF
    and rax, rbx
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|OF|SF), 0

    ; and64 sign: set bit 63
    TEST_CASE tand64_3
    mov rax, 0xFFFFFFFFFFFFFFFF
    mov rbx, 0x8000000000000000
    and rax, rbx
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|OF|SF), SF

    ;; ================================================================
    ;; OR 8-bit
    ;; ================================================================

    ; or8 zero: 0 | 0 = 0 -> ZF=1
    TEST_CASE tor8_1
    mov al, 0
    or al, 0
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|OF|SF), ZF

    ; or8 nonzero: 0x01 | 0x02 = 0x03 -> ZF=0, SF=0
    TEST_CASE tor8_2
    mov al, 0x01
    or al, 0x02
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|OF|SF), 0

    ; or8 sign: 0x00 | 0x80 = 0x80 -> SF=1
    TEST_CASE tor8_3
    mov al, 0x00
    or al, 0x80
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|OF|SF), SF

    ;; ================================================================
    ;; OR 16-bit
    ;; ================================================================

    ; or16 zero: 0 | 0 = 0 -> ZF=1
    TEST_CASE tor16_1
    mov ax, 0
    or ax, 0
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|OF|SF), ZF

    ; or16 nonzero: 0x00FF | 0xFF00 = 0xFFFF -> SF=1
    TEST_CASE tor16_2
    mov ax, 0x00FF
    or ax, 0xFF00
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|OF|SF), SF

    ; or16 sign: 0x0000 | 0x8000 = 0x8000 -> SF=1
    TEST_CASE tor16_3
    mov ax, 0x0000
    or ax, 0x8000
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|OF|SF), SF

    ;; ================================================================
    ;; OR 64-bit
    ;; ================================================================

    ; or64 zero: 0 | 0 = 0 -> ZF=1
    TEST_CASE tor64_1
    xor rax, rax
    xor rbx, rbx
    or rax, rbx
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|OF|SF), ZF

    ; or64 nonzero: small values -> nonzero, no sign
    TEST_CASE tor64_2
    mov rax, 0x00000000DEADBEEF
    mov rbx, 0x00000000CAFEBABE
    or rax, rbx
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|OF|SF), 0

    ; or64 sign: set bit 63
    TEST_CASE tor64_3
    mov rax, 0x8000000000000000
    xor rbx, rbx
    or rax, rbx
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|OF|SF), SF

    ;; ================================================================
    ;; XOR 8-bit
    ;; ================================================================

    ; xor8 zero: same ^ same = 0 -> ZF=1
    TEST_CASE txor8_1
    mov al, 0xAA
    xor al, 0xAA
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|OF|SF), ZF

    ; xor8 nonzero: 0xFF ^ 0x0F = 0xF0 -> SF=1 (bit 7 set)
    TEST_CASE txor8_2
    mov al, 0xFF
    xor al, 0x0F
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|OF|SF), SF

    ; xor8 sign: 0x00 ^ 0x80 = 0x80 -> SF=1
    TEST_CASE txor8_3
    mov al, 0x00
    xor al, 0x80
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|OF|SF), SF

    ;; ================================================================
    ;; XOR 16-bit
    ;; ================================================================

    ; xor16 zero: same ^ same = 0 -> ZF=1
    TEST_CASE txor16_1
    mov ax, 0x1234
    mov bx, 0x1234
    xor ax, bx
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|OF|SF), ZF

    ; xor16 nonzero: 0xFFFF ^ 0x00FF = 0xFF00 -> SF=1
    TEST_CASE txor16_2
    mov ax, 0xFFFF
    xor ax, 0x00FF
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|OF|SF), SF

    ; xor16 sign: 0x0000 ^ 0x8000 = 0x8000 -> SF=1
    TEST_CASE txor16_3
    mov ax, 0x0000
    mov bx, 0x8000
    xor ax, bx
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|OF|SF), SF

    ;; ================================================================
    ;; XOR 64-bit
    ;; ================================================================

    ; xor64 zero: same ^ same = 0 -> ZF=1
    TEST_CASE txor64_1
    mov rax, 0x123456789ABCDEF0
    mov rbx, 0x123456789ABCDEF0
    xor rax, rbx
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|OF|SF), ZF

    ; xor64 nonzero: different values -> nonzero
    TEST_CASE txor64_2
    mov rax, 0x00000000FFFFFFFF
    mov rbx, 0x000000000000FFFF
    xor rax, rbx
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|OF|SF), 0

    ; xor64 sign: set bit 63
    TEST_CASE txor64_3
    mov rax, 0x0000000000000001
    mov rbx, 0x8000000000000001
    xor rax, rbx
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|OF|SF), SF

    ;; ================================================================
    ;; TEST 16-bit  (test = AND without storing result)
    ;; ================================================================

    ; test16 zero: 0xF0F0 & 0x0F0F = 0 -> ZF=1
    TEST_CASE ttst16_1
    mov ax, 0xF0F0
    test ax, 0x0F0F
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|OF|SF), ZF

    ; test16 nonzero: 0xFFFF & 0x1234 = 0x1234 -> ZF=0
    TEST_CASE ttst16_2
    mov ax, 0xFFFF
    mov bx, 0x1234
    test ax, bx
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|OF|SF), 0

    ; test16 sign: 0x8000 & 0x8000 = 0x8000 -> SF=1
    TEST_CASE ttst16_3
    mov ax, 0x8000
    test ax, 0x8000
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|OF|SF), SF

    ;; ================================================================
    ;; Verify logic ops clear CF (even if CF was set before)
    ;; ================================================================

    ; and8 clears CF: set CF first, and should clear it
    TEST_CASE tlogic_cf1
    stc                     ; CF=1
    mov al, 0xFF
    and al, 0x0F            ; should clear CF
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, 0

    ; or16 clears CF: set CF first, or should clear it
    TEST_CASE tlogic_cf2
    stc                     ; CF=1
    mov ax, 0x1234
    or ax, 0x0000           ; should clear CF
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, 0

    ; xor64 clears CF: set CF first, xor should clear it
    TEST_CASE tlogic_cf3
    stc                     ; CF=1
    mov rax, 1
    mov rbx, 2
    xor rax, rbx            ; should clear CF
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, 0

    ;; ================================================================
    END_TESTS
