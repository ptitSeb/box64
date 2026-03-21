; test_updateflags_carry.asm - Deferred flag tests for carry-dependent ops
; Covers: adc8/16/32/64, sbb8/16/32/64, shld16/32/64, shrd16/32/64
; Each test: instruction -> call flag_barrier_ret -> check flags
;
; ADC: dest = dest + src + CF.  Flags set like ADD but including carry-in.
; SBB: dest = dest - src - CF.  Flags set like SUB but including borrow-in.
; SHLD: shift left double - shifts bits from src into dest from the right
; SHRD: shift right double - shifts bits from src into dest from the left
%include "test_framework.inc"

section .data
    ; --- adc8 ---
    tadc8_1:  db "adc8 no carry in, no carry out", 0
    tadc8_2:  db "adc8 carry in, carry out", 0
    tadc8_3:  db "adc8 carry in, overflow", 0
    ; --- adc16 ---
    tadc16_1: db "adc16 no carry in, carry out", 0
    tadc16_2: db "adc16 carry in, zero result", 0
    ; --- adc32 ---
    tadc32_1: db "adc32 carry in, no carry out", 0
    tadc32_2: db "adc32 carry in, carry out", 0
    tadc32_3: db "adc32 carry in, overflow", 0
    ; --- adc64 ---
    tadc64_1: db "adc64 carry in, carry out", 0
    tadc64_2: db "adc64 no carry, overflow", 0
    ; --- sbb8 ---
    tsbb8_1:  db "sbb8 no borrow in, borrow out", 0
    tsbb8_2:  db "sbb8 borrow in, zero result", 0
    tsbb8_3:  db "sbb8 borrow in, overflow", 0
    ; --- sbb16 ---
    tsbb16_1: db "sbb16 borrow in, borrow out", 0
    tsbb16_2: db "sbb16 no borrow, zero result", 0
    ; --- sbb32 ---
    tsbb32_1: db "sbb32 borrow in, borrow out", 0
    tsbb32_2: db "sbb32 borrow in, overflow", 0
    ; --- sbb64 ---
    tsbb64_1: db "sbb64 borrow in, borrow out", 0
    tsbb64_2: db "sbb64 no borrow, zero result", 0
    ; --- shld16 ---
    tshld16_1: db "shld16 basic shift via call", 0
    tshld16_2: db "shld16 CF=last bit out via call", 0
    ; --- shld32 ---
    tshld32_1: db "shld32 basic shift via call", 0
    tshld32_2: db "shld32 CF=last bit out via call", 0
    ; --- shld64 ---
    tshld64_1: db "shld64 basic shift via call", 0
    tshld64_2: db "shld64 CF=last bit out via call", 0
    ; --- shrd16 ---
    tshrd16_1: db "shrd16 basic shift via call", 0
    tshrd16_2: db "shrd16 CF=last bit out via call", 0
    ; --- shrd32 ---
    tshrd32_1: db "shrd32 basic shift via call", 0
    tshrd32_2: db "shrd32 CF=last bit out via call", 0
    ; --- shrd64 ---
    tshrd64_1: db "shrd64 basic shift via call", 0
    tshrd64_2: db "shrd64 CF=last bit out via call", 0

section .text
global _start

flag_barrier_ret:
    ret

_start:
    INIT_TESTS

    ;; ================================================================
    ;; ADC 8-bit
    ;; ================================================================

    ; adc8 no carry in: 5 + 3 + 0 = 8, no flags
    TEST_CASE tadc8_1
    clc
    mov al, 5
    adc al, 3
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|ZF|OF|SF), 0

    ; adc8 carry in, carry out: 0xFF + 0 + 1(CF) = 0x00, CF=1, ZF=1
    TEST_CASE tadc8_2
    stc
    mov al, 0xFF
    adc al, 0
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|ZF), (CF|ZF)

    ; adc8 carry in, overflow: 0x7F + 0 + 1(CF) = 0x80, OF=1, SF=1
    TEST_CASE tadc8_3
    stc
    mov al, 0x7F
    adc al, 0
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (OF|SF), (OF|SF)

    ;; ================================================================
    ;; ADC 16-bit
    ;; ================================================================

    ; adc16 no carry in, carry out: 0xFFFF + 1 + 0 = 0x0000, CF=1, ZF=1
    TEST_CASE tadc16_1
    clc
    mov ax, 0xFFFF
    adc ax, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|ZF), (CF|ZF)

    ; adc16 carry in, zero: 0xFFFF + 0 + 1(CF) = 0x0000, CF=1, ZF=1
    TEST_CASE tadc16_2
    stc
    mov ax, 0xFFFF
    adc ax, 0
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|ZF), (CF|ZF)

    ;; ================================================================
    ;; ADC 32-bit
    ;; ================================================================

    ; adc32 carry in, no carry out: 10 + 20 + 1(CF) = 31
    TEST_CASE tadc32_1
    stc
    mov eax, 10
    adc eax, 20
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|ZF|OF|SF), 0

    ; adc32 carry in, carry out: 0xFFFFFFFF + 0 + 1(CF) = 0, CF=1, ZF=1
    TEST_CASE tadc32_2
    stc
    mov eax, 0xFFFFFFFF
    adc eax, 0
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|ZF), (CF|ZF)

    ; adc32 carry in, overflow: 0x7FFFFFFF + 0 + 1(CF) = 0x80000000, OF=1, SF=1
    TEST_CASE tadc32_3
    stc
    mov eax, 0x7FFFFFFF
    adc eax, 0
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (OF|SF), (OF|SF)

    ;; ================================================================
    ;; ADC 64-bit
    ;; ================================================================

    ; adc64 carry in, carry out: 0xFFFFFFFFFFFFFFFF + 0 + 1(CF) = 0, CF=1, ZF=1
    TEST_CASE tadc64_1
    stc
    mov rax, 0xFFFFFFFFFFFFFFFF
    adc rax, 0
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|ZF), (CF|ZF)

    ; adc64 no carry, overflow: 0x7FFFFFFFFFFFFFFF + 1 + 0 = overflow
    TEST_CASE tadc64_2
    clc
    mov rax, 0x7FFFFFFFFFFFFFFF
    adc rax, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (OF|SF), (OF|SF)

    ;; ================================================================
    ;; SBB 8-bit
    ;; ================================================================

    ; sbb8 no borrow in, borrow out: 0 - 1 - 0 = 0xFF, CF=1, SF=1
    TEST_CASE tsbb8_1
    clc
    mov al, 0
    sbb al, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|SF), (CF|SF)

    ; sbb8 borrow in, zero result: 1 - 0 - 1(CF) = 0, ZF=1
    TEST_CASE tsbb8_2
    stc
    mov al, 1
    sbb al, 0
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|ZF|SF|OF), ZF

    ; sbb8 borrow in, overflow: 0x80 - 0 - 1(CF) = 0x7F, OF=1
    TEST_CASE tsbb8_3
    stc
    mov al, 0x80
    sbb al, 0
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (OF|CF|SF), OF

    ;; ================================================================
    ;; SBB 16-bit
    ;; ================================================================

    ; sbb16 borrow in, borrow out: 0 - 0 - 1(CF) = 0xFFFF, CF=1, SF=1
    TEST_CASE tsbb16_1
    stc
    mov ax, 0
    sbb ax, 0
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|SF), (CF|SF)

    ; sbb16 no borrow, zero result: 0x1234 - 0x1234 - 0 = 0, ZF=1
    TEST_CASE tsbb16_2
    clc
    mov ax, 0x1234
    mov bx, 0x1234
    sbb ax, bx
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|ZF|SF|OF), ZF

    ;; ================================================================
    ;; SBB 32-bit
    ;; ================================================================

    ; sbb32 borrow in, borrow out: 0 - 0 - 1(CF) = 0xFFFFFFFF, CF=1, SF=1
    TEST_CASE tsbb32_1
    stc
    mov eax, 0
    sbb eax, 0
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|SF), (CF|SF)

    ; sbb32 borrow in, overflow: 0x80000000 - 0 - 1(CF) = 0x7FFFFFFF, OF=1
    TEST_CASE tsbb32_2
    stc
    mov eax, 0x80000000
    sbb eax, 0
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (OF|CF|SF), OF

    ;; ================================================================
    ;; SBB 64-bit
    ;; ================================================================

    ; sbb64 borrow in, borrow out: 0 - 0 - 1(CF) = 0xFFFFFFFFFFFFFFFF, CF=1, SF=1
    TEST_CASE tsbb64_1
    mov rax, 0
    stc
    sbb rax, 0
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|SF), (CF|SF)

    ; sbb64 no borrow, zero: large - large - 0 = 0, ZF=1
    TEST_CASE tsbb64_2
    clc
    mov rax, 0x123456789ABCDEF0
    mov rbx, 0x123456789ABCDEF0
    sbb rax, rbx
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|ZF|SF|OF), ZF

    ;; ================================================================
    ;; SHLD 16-bit (shift left double)
    ;; shld dest, src, count: shifts dest left by count, filling from src MSB
    ;; ================================================================

    ; shld16: dest=0x1234, src=0x5678, count=4
    ; Result: dest<<4 | (src>>(16-4)) = 0x2345
    ; CF = last bit shifted out of dest = bit 12 of 0x1234 = 1
    TEST_CASE tshld16_1
    mov ax, 0x1234
    mov bx, 0x5678
    shld ax, bx, 4
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ; shld16: dest=0x0001, src=0x0000, count=1
    ; Result: 0x0002, CF=0
    TEST_CASE tshld16_2
    mov ax, 0x0001
    mov bx, 0x0000
    shld ax, bx, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|ZF), 0

    ;; ================================================================
    ;; SHLD 32-bit
    ;; ================================================================

    ; shld32: dest=0x12345678, src=0x9ABCDEF0, count=4
    ; Result: 0x23456789, CF = bit 28 of 0x12345678 = 1
    TEST_CASE tshld32_1
    mov eax, 0x12345678
    mov ebx, 0x9ABCDEF0
    shld eax, ebx, 4
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ; shld32: dest=0x00000001, src=0x00000000, count=1
    ; Result: 0x00000002, CF=0
    TEST_CASE tshld32_2
    mov eax, 0x00000001
    mov ebx, 0x00000000
    shld eax, ebx, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|ZF), 0

    ;; ================================================================
    ;; SHLD 64-bit
    ;; ================================================================

    ; shld64: dest=0x123456789ABCDEF0, src=0xFEDCBA9876543210, count=4
    ; CF = bit 60 of dest = 1
    TEST_CASE tshld64_1
    mov rax, 0x123456789ABCDEF0
    mov rbx, 0xFEDCBA9876543210
    shld rax, rbx, 4
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ; shld64: dest=0x0000000000000001, src=0, count=1
    ; Result: 0x0000000000000002, CF=0
    TEST_CASE tshld64_2
    mov rax, 0x0000000000000001
    xor rbx, rbx
    shld rax, rbx, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|ZF), 0

    ;; ================================================================
    ;; SHRD 16-bit (shift right double)
    ;; shrd dest, src, count: shifts dest right by count, filling from src LSB
    ;; ================================================================

    ; shrd16: dest=0x1234, src=0x5678, count=4
    ; Result: dest>>4 | (src<<(16-4)) = 0x8123
    ; CF = last bit shifted out of dest = bit 3 of 0x1234 = 0
    TEST_CASE tshrd16_1
    mov ax, 0x1234
    mov bx, 0x5678
    shrd ax, bx, 4
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|ZF), 0

    ; shrd16: dest=0x0001, src=0x0000, count=1
    ; Result: 0x0000, CF=1, ZF=1
    TEST_CASE tshrd16_2
    mov ax, 0x0001
    mov bx, 0x0000
    shrd ax, bx, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|ZF), (CF|ZF)

    ;; ================================================================
    ;; SHRD 32-bit
    ;; ================================================================

    ; shrd32: dest=0x12345678, src=0x9ABCDEF0, count=4
    ; CF = bit 3 of 0x12345678 = 1 (0x8 in low nibble)
    TEST_CASE tshrd32_1
    mov eax, 0x12345678
    mov ebx, 0x9ABCDEF0
    shrd eax, ebx, 4
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ; shrd32: dest=0x00000001, src=0x00000000, count=1
    ; Result: 0x00000000, CF=1, ZF=1
    TEST_CASE tshrd32_2
    mov eax, 0x00000001
    mov ebx, 0x00000000
    shrd eax, ebx, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|ZF), (CF|ZF)

    ;; ================================================================
    ;; SHRD 64-bit
    ;; ================================================================

    ; shrd64: dest=0x123456789ABCDEF8, src=0xFEDCBA9876543210, count=4
    ; CF = bit 3 of dest = 1 (low nibble 8 = 1000, bit 3 = 1)
    TEST_CASE tshrd64_1
    mov rax, 0x123456789ABCDEF8
    mov rbx, 0xFEDCBA9876543210
    shrd rax, rbx, 4
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ; shrd64: dest=0x01, src=0x00, count=1
    ; Result: 0, CF=1, ZF=1
    TEST_CASE tshrd64_2
    mov rax, 1
    xor rbx, rbx
    shrd rax, rbx, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|ZF), (CF|ZF)

    ;; ================================================================
    END_TESTS
