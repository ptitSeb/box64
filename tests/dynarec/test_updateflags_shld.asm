; test_updateflags_shld.asm - Deferred flag tests for SHLD/SHRD
; Covers: shld/shrd for 16/32/64 bit widths
; Each test: instruction -> call flag_barrier_ret -> check flags
;
; SHLD: Double-precision shift left.  dest = (dest << count) | (src >> (N-count))
;   CF = last bit shifted out of dest (bit N-count of original dest)
;   SF/ZF/PF set from result. AF undefined.
;   OF defined only for count=1.
;
; SHRD: Double-precision shift right. dest = (dest >> count) | (src << (N-count))
;   CF = last bit shifted out of dest (bit count-1 of original dest)
;   SF/ZF/PF set from result. AF undefined.
;   OF defined only for count=1.
;
; When count=0, no flags are modified.
%include "test_framework.inc"

section .data
    ; --- shrd16 ---
    tshrd16_1:  db "shrd16 basic shift CF=1", 0
    tshrd16_2:  db "shrd16 zero result ZF=1", 0
    tshrd16_3:  db "shrd16 count=0 no flags change", 0
    tshrd16_4:  db "shrd16 CF from bit(cnt-1)", 0
    tshrd16_5:  db "shrd16 sign bit result SF=1", 0
    ; --- shrd32 ---
    tshrd32_1:  db "shrd32 basic CF=1", 0
    tshrd32_2:  db "shrd32 zero result ZF=1", 0
    tshrd32_3:  db "shrd32 count=0 no flags change", 0
    tshrd32_4:  db "shrd32 shift by 4 CF check", 0
    tshrd32_5:  db "shrd32 high bits from src", 0
    ; --- shrd64 ---
    tshrd64_1:  db "shrd64 basic CF=1", 0
    tshrd64_2:  db "shrd64 zero result ZF=1", 0
    tshrd64_3:  db "shrd64 count=0 no flags change", 0
    tshrd64_4:  db "shrd64 shift by 8 CF check", 0
    ; --- shld16 ---
    tshld16_1:  db "shld16 basic CF=1", 0
    tshld16_2:  db "shld16 zero result ZF=1", 0
    tshld16_3:  db "shld16 count=0 no flags change", 0
    tshld16_4:  db "shld16 CF from bit(16-cnt)", 0
    tshld16_5:  db "shld16 sign bit result SF=1", 0
    ; --- shld32 ---
    tshld32_1:  db "shld32 basic CF=1", 0
    tshld32_2:  db "shld32 zero result ZF=1", 0
    tshld32_3:  db "shld32 count=0 no flags change", 0
    tshld32_4:  db "shld32 shift by 4 CF check", 0
    tshld32_5:  db "shld32 high bits from src", 0
    ; --- shld64 ---
    tshld64_1:  db "shld64 basic CF=1", 0
    tshld64_2:  db "shld64 zero result ZF=1", 0
    tshld64_3:  db "shld64 count=0 no flags change", 0
    tshld64_4:  db "shld64 shift by 8 CF check", 0
    ; --- result checks ---
    tshrd16_r1: db "shrd16 result check", 0
    tshrd32_r1: db "shrd32 result check", 0
    tshrd64_r1: db "shrd64 result check", 0
    tshld16_r1: db "shld16 result check", 0
    tshld32_r1: db "shld32 result check", 0
    tshld64_r1: db "shld64 result check", 0

section .text
global _start

flag_barrier_ret:
    ret

_start:
    INIT_TESTS

    ;; ================================================================
    ;; SHRD16 tests
    ;; ================================================================

    ; shrd ax, bx, 1: ax=0x0003, bx=0x0000, count=1
    ; res = (0x0003 >> 1) | (0x0000 << 15) = 0x0001
    ; CF = bit 0 of original dest = 1
    TEST_CASE tshrd16_1
    mov ax, 0x0003
    mov bx, 0x0000
    shrd ax, bx, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ; shrd ax, bx, 1: ax=0x0001, bx=0x0000, count=1
    ; res = 0x0000, CF=1, ZF=1
    TEST_CASE tshrd16_2
    mov ax, 0x0001
    mov bx, 0x0000
    shrd ax, bx, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|ZF), (CF|ZF)

    ; shrd ax, bx, 0: no flags change
    ; Set CF first with stc, then shrd with count=0 should NOT clear CF
    TEST_CASE tshrd16_3
    stc                     ; set CF
    mov ax, 0x1234
    mov bx, 0x5678
    shrd ax, bx, 0
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF   ; CF should still be set

    ; shrd ax, bx, 4: ax=0x00F0, bx=0xABCD, count=4
    ; CF = bit 3 of 0x00F0 = (0x00F0 >> 3) & 1 = 0x1E & 1 = 0
    TEST_CASE tshrd16_4
    mov ax, 0x00F0
    mov bx, 0xABCD
    shrd ax, bx, 4
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, 0

    ; shrd ax, bx, 1: ax=0x0000, bx=0xFFFF, count=1
    ; res = (0x0000 >> 1) | (0xFFFF << 15) = 0x8000 => SF=1, CF=0
    TEST_CASE tshrd16_5
    mov ax, 0x0000
    mov bx, 0xFFFF
    shrd ax, bx, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (SF|CF), SF   ; SF=1, CF=0

    ; shrd16 result check: ax=0x8001, bx=0x0003, count=1
    ; res = (0x8001 >> 1) | (0x0003 << 15) = 0xC000 | 0x8000 = 0xC000
    ; Wait: 0x8001 >> 1 = 0x4000, 0x0003 << 15 = 0x8000, result = 0xC000
    TEST_CASE tshrd16_r1
    mov ax, 0x8001
    mov bx, 0x0003
    shrd ax, bx, 1
    call flag_barrier_ret
    CHECK_EQ_32 eax, 0xC000

    ;; ================================================================
    ;; SHRD32 tests
    ;; ================================================================

    ; shrd eax, ebx, 1: eax=0x00000003, ebx=0, count=1
    ; res = 1, CF=1
    TEST_CASE tshrd32_1
    mov eax, 0x00000003
    xor ebx, ebx
    shrd eax, ebx, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ; shrd eax, ebx, 1: eax=1, ebx=0, count=1 => res=0, CF=1, ZF=1
    TEST_CASE tshrd32_2
    mov eax, 0x00000001
    xor ebx, ebx
    shrd eax, ebx, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|ZF), (CF|ZF)

    ; count=0: no flags change
    TEST_CASE tshrd32_3
    stc
    mov eax, 0x12345678
    mov ebx, 0xABCDEF01
    shrd eax, ebx, 0
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ; shrd eax, ebx, 4: eax=0x000000F0, ebx=0xDEADBEEF, count=4
    ; CF = bit 3 of 0xF0 = (0xF0 >> 3) & 1 = 0x1E & 1 = 0
    TEST_CASE tshrd32_4
    mov eax, 0x000000F0
    mov ebx, 0xDEADBEEF
    shrd eax, ebx, 4
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, 0

    ; shrd eax, ebx, 1: eax=0, ebx=0x80000001, count=1
    ; res = (0 >> 1) | (0x80000001 << 31) = 0x80000000, SF=1
    TEST_CASE tshrd32_5
    xor eax, eax
    mov ebx, 0x80000001
    shrd eax, ebx, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (SF|CF), SF

    ; Result check
    TEST_CASE tshrd32_r1
    mov eax, 0x80000001
    mov ebx, 0x00000003
    shrd eax, ebx, 1
    call flag_barrier_ret
    CHECK_EQ_32 eax, 0xC0000000

    ;; ================================================================
    ;; SHRD64 tests
    ;; ================================================================

    ; shrd rax, rbx, 1: rax=3, rbx=0, count=1 => res=1, CF=1
    TEST_CASE tshrd64_1
    mov rax, 3
    xor rbx, rbx
    shrd rax, rbx, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ; shrd rax, rbx, 1: rax=1, rbx=0 => res=0, CF=1, ZF=1
    TEST_CASE tshrd64_2
    mov rax, 1
    xor rbx, rbx
    shrd rax, rbx, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|ZF), (CF|ZF)

    ; count=0: no flags change
    TEST_CASE tshrd64_3
    stc
    mov rax, 0x123456789ABCDEF0
    mov rbx, 0xFEDCBA9876543210
    shrd rax, rbx, 0
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ; shrd rax, rbx, 8: rax=0xFF, rbx=0xAB, count=8
    ; CF = bit 7 of 0xFF = 1
    TEST_CASE tshrd64_4
    mov rax, 0xFF
    mov rbx, 0xAB
    shrd rax, rbx, 8
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ; Result check
    TEST_CASE tshrd64_r1
    mov rax, 0x8000000000000001
    mov rbx, 0x0000000000000003
    shrd rax, rbx, 1
    call flag_barrier_ret
    CHECK_EQ_64 rax, 0xC000000000000000

    ;; ================================================================
    ;; SHLD16 tests
    ;; ================================================================

    ; shld ax, bx, 1: ax=0x8000, bx=0x0001, count=1
    ; res = (0x8000 << 1) | (0x0001 >> 15) = 0x0000 | 0x0001 = 0x0001
    ; CF = bit 15 of original dest = 1
    TEST_CASE tshld16_1
    mov ax, 0x8000
    mov bx, 0x0001
    shld ax, bx, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ; shld ax, bx, 1: ax=0x8000, bx=0, count=1
    ; res = 0x0000, CF=1, ZF=1
    TEST_CASE tshld16_2
    mov ax, 0x8000
    xor bx, bx
    shld ax, bx, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|ZF), (CF|ZF)

    ; count=0: no flags change
    TEST_CASE tshld16_3
    stc
    mov ax, 0x1234
    mov bx, 0x5678
    shld ax, bx, 0
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ; shld ax, bx, 4: ax=0x1234, bx=0xABCD, count=4
    ; CF = bit(16-4) = bit 12 of 0x1234 = (0x1234 >> 12) & 1 = 1
    TEST_CASE tshld16_4
    mov ax, 0x1234
    mov bx, 0xABCD
    shld ax, bx, 4
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ; shld ax, bx, 1: ax=0x4000, bx=0, count=1
    ; res = 0x8000, CF=0, SF=1
    TEST_CASE tshld16_5
    mov ax, 0x4000
    xor bx, bx
    shld ax, bx, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (SF|CF), SF

    ; Result check: shld ax, bx, 1: ax=0x8001, bx=0xC000, count=1
    ; res = (0x8001 << 1) | (0xC000 >> 15) = 0x0002 | 0x0001 = 0x0003
    TEST_CASE tshld16_r1
    mov ax, 0x8001
    mov bx, 0xC000
    shld ax, bx, 1
    call flag_barrier_ret
    CHECK_EQ_32 eax, 0x0003

    ;; ================================================================
    ;; SHLD32 tests
    ;; ================================================================

    ; shld eax, ebx, 1: eax=0x80000000, ebx=1, count=1
    ; res = 0x00000001, CF=1
    TEST_CASE tshld32_1
    mov eax, 0x80000000
    mov ebx, 0x00000001
    shld eax, ebx, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ; shld eax, ebx, 1: eax=0x80000000, ebx=0 => res=0, CF=1, ZF=1
    TEST_CASE tshld32_2
    mov eax, 0x80000000
    xor ebx, ebx
    shld eax, ebx, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|ZF), (CF|ZF)

    ; count=0: no flags change
    TEST_CASE tshld32_3
    stc
    mov eax, 0x12345678
    mov ebx, 0xABCDEF01
    shld eax, ebx, 0
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ; shld eax, ebx, 4: eax=0x12345678, ebx=0xABCDEF01, count=4
    ; CF = bit(32-4) = bit 28 of 0x12345678 = (0x12345678 >> 28) & 1 = 1
    TEST_CASE tshld32_4
    mov eax, 0x12345678
    mov ebx, 0xABCDEF01
    shld eax, ebx, 4
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ; shld eax, ebx, 1: eax=0, ebx=0x80000001
    ; res = (0 << 1) | (0x80000001 >> 31) = 0x00000001
    TEST_CASE tshld32_5
    xor eax, eax
    mov ebx, 0x80000001
    shld eax, ebx, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|ZF), 0    ; CF=0 (bit 31 of 0 = 0), ZF=0 (res=1)

    ; Result check
    TEST_CASE tshld32_r1
    mov eax, 0x80000001
    mov ebx, 0xC0000000
    shld eax, ebx, 1
    call flag_barrier_ret
    CHECK_EQ_32 eax, 0x00000003

    ;; ================================================================
    ;; SHLD64 tests
    ;; ================================================================

    ; shld rax, rbx, 1: rax=0x8000000000000000, rbx=1 => res=1, CF=1
    TEST_CASE tshld64_1
    mov rax, 0x8000000000000000
    mov rbx, 1
    shld rax, rbx, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ; shld rax, rbx, 1: rax=0x8000000000000000, rbx=0 => res=0, CF=1, ZF=1
    TEST_CASE tshld64_2
    mov rax, 0x8000000000000000
    xor rbx, rbx
    shld rax, rbx, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|ZF), (CF|ZF)

    ; count=0: no flags change
    TEST_CASE tshld64_3
    stc
    mov rax, 0x123456789ABCDEF0
    mov rbx, 0xFEDCBA9876543210
    shld rax, rbx, 0
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ; shld rax, rbx, 8: rax=0xFF00000000000000, rbx=0xAB
    ; CF = bit(64-8)=bit 56 of rax = (0xFF00000000000000 >> 56) & 1 = 0xFF & 1 = 1
    TEST_CASE tshld64_4
    mov rax, 0xFF00000000000000
    mov rbx, 0xAB
    shld rax, rbx, 8
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ; Result check
    TEST_CASE tshld64_r1
    mov rax, 0x8000000000000001
    mov rbx, 0xC000000000000000
    shld rax, rbx, 1
    call flag_barrier_ret
    CHECK_EQ_64 rax, 0x0000000000000003

    END_TESTS
