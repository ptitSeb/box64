; test_updateflags_arith.asm - Deferred flag tests for remaining arithmetic ops
; Covers: add8/16/64, sub8/16/64, inc8/16/32/64, dec8/16/32/64, neg8/16/32/64
; Each test: instruction -> call flag_barrier_ret -> check flags
%include "test_framework.inc"

section .data
    ; --- add8 ---
    ta8_1:  db "add8 carry (CF) via call", 0
    ta8_2:  db "add8 zero (ZF) via call", 0
    ta8_3:  db "add8 overflow (OF) via call", 0
    ; --- add16 ---
    ta16_1: db "add16 carry (CF) via call", 0
    ta16_2: db "add16 zero (ZF) via call", 0
    ta16_3: db "add16 overflow (OF) via call", 0
    ; --- add64 ---
    ta64_1: db "add64 carry (CF) via call", 0
    ta64_2: db "add64 zero (ZF) via call", 0
    ta64_3: db "add64 overflow (OF) via call", 0
    ; --- sub8 ---
    ts8_1:  db "sub8 borrow (CF) via call", 0
    ts8_2:  db "sub8 zero (ZF) via call", 0
    ts8_3:  db "sub8 overflow (OF) via call", 0
    ; --- sub16 ---
    ts16_1: db "sub16 borrow (CF) via call", 0
    ts16_2: db "sub16 zero (ZF) via call", 0
    ts16_3: db "sub16 overflow (OF) via call", 0
    ; --- sub64 ---
    ts64_1: db "sub64 borrow (CF) via call", 0
    ts64_2: db "sub64 zero (ZF) via call", 0
    ts64_3: db "sub64 overflow (OF) via call", 0
    ; --- inc8 ---
    ti8_1:  db "inc8 zero (ZF) via call", 0
    ti8_2:  db "inc8 overflow (OF) via call", 0
    ti8_3:  db "inc8 no CF change via call", 0
    ; --- inc16 ---
    ti16_1: db "inc16 zero (ZF) via call", 0
    ti16_2: db "inc16 overflow (OF) via call", 0
    ; --- inc32 ---
    ti32_1: db "inc32 zero (ZF) via call", 0
    ti32_2: db "inc32 overflow (OF) via call", 0
    ; --- inc64 ---
    ti64_1: db "inc64 zero (ZF) via call", 0
    ti64_2: db "inc64 overflow (OF) via call", 0
    ; --- dec8 ---
    td8_1:  db "dec8 zero (ZF) via call", 0
    td8_2:  db "dec8 sign (SF) via call", 0
    td8_3:  db "dec8 overflow (OF) via call", 0
    ; --- dec16 ---
    td16_1: db "dec16 zero (ZF) via call", 0
    td16_2: db "dec16 overflow (OF) via call", 0
    ; --- dec32 ---
    td32_1: db "dec32 zero (ZF) via call", 0
    td32_2: db "dec32 overflow (OF) via call", 0
    ; --- dec64 ---
    td64_1: db "dec64 zero (ZF) via call", 0
    td64_2: db "dec64 overflow (OF) via call", 0
    ; --- neg8 ---
    tn8_1:  db "neg8 zero (ZF,CF=0) via call", 0
    tn8_2:  db "neg8 nonzero (CF=1) via call", 0
    tn8_3:  db "neg8 overflow (OF) via call", 0
    ; --- neg16 ---
    tn16_1: db "neg16 zero (ZF) via call", 0
    tn16_2: db "neg16 nonzero (CF=1) via call", 0
    tn16_3: db "neg16 overflow (OF) via call", 0
    ; --- neg32 ---
    tn32_1: db "neg32 zero (ZF) via call", 0
    tn32_2: db "neg32 nonzero (CF=1) via call", 0
    tn32_3: db "neg32 overflow (OF) via call", 0
    ; --- neg64 ---
    tn64_1: db "neg64 zero (ZF) via call", 0
    tn64_2: db "neg64 nonzero (CF=1) via call", 0
    tn64_3: db "neg64 overflow (OF) via call", 0

section .text
global _start

flag_barrier_ret:
    ret

_start:
    INIT_TESTS

    ;; ================================================================
    ;; ADD 8-bit
    ;; ================================================================

    ; add8 carry: 0xFF + 1 = 0x00 -> CF=1, ZF=1
    TEST_CASE ta8_1
    mov al, 0xFF
    add al, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|ZF), (CF|ZF)

    ; add8 zero: 0 + 0 = 0 -> ZF=1, CF=0
    TEST_CASE ta8_2
    mov al, 0
    add al, 0
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF), ZF

    ; add8 overflow: 0x7F + 1 = 0x80 -> OF=1, SF=1
    TEST_CASE ta8_3
    mov al, 0x7F
    add al, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (OF|SF), (OF|SF)

    ;; ================================================================
    ;; ADD 16-bit
    ;; ================================================================

    ; add16 carry: 0xFFFF + 1 = 0x0000 -> CF=1, ZF=1
    TEST_CASE ta16_1
    mov ax, 0xFFFF
    add ax, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|ZF), (CF|ZF)

    ; add16 zero: 0 + 0 = 0 -> ZF=1, CF=0
    TEST_CASE ta16_2
    mov ax, 0
    add ax, 0
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF), ZF

    ; add16 overflow: 0x7FFF + 1 = 0x8000 -> OF=1, SF=1
    TEST_CASE ta16_3
    mov ax, 0x7FFF
    add ax, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (OF|SF), (OF|SF)

    ;; ================================================================
    ;; ADD 64-bit
    ;; ================================================================

    ; add64 carry: 0xFFFFFFFFFFFFFFFF + 1 = 0 -> CF=1, ZF=1
    TEST_CASE ta64_1
    mov rax, 0xFFFFFFFFFFFFFFFF
    add rax, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|ZF), (CF|ZF)

    ; add64 zero: 0 + 0 = 0 -> ZF=1
    TEST_CASE ta64_2
    xor rax, rax
    add rax, 0
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF), ZF

    ; add64 overflow: 0x7FFFFFFFFFFFFFFF + 1 -> OF=1, SF=1
    TEST_CASE ta64_3
    mov rax, 0x7FFFFFFFFFFFFFFF
    add rax, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (OF|SF), (OF|SF)

    ;; ================================================================
    ;; SUB 8-bit
    ;; ================================================================

    ; sub8 borrow: 0 - 1 = 0xFF -> CF=1, SF=1
    TEST_CASE ts8_1
    mov al, 0
    sub al, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|SF), (CF|SF)

    ; sub8 zero: 5 - 5 = 0 -> ZF=1
    TEST_CASE ts8_2
    mov al, 5
    sub al, 5
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|SF|OF), ZF

    ; sub8 overflow: 0x80 - 1 = 0x7F -> OF=1 (neg - pos = pos)
    TEST_CASE ts8_3
    mov al, 0x80
    sub al, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ OF, OF

    ;; ================================================================
    ;; SUB 16-bit
    ;; ================================================================

    ; sub16 borrow: 0 - 1 -> CF=1, SF=1
    TEST_CASE ts16_1
    mov ax, 0
    sub ax, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|SF), (CF|SF)

    ; sub16 zero: 0x1234 - 0x1234 = 0 -> ZF=1
    TEST_CASE ts16_2
    mov ax, 0x1234
    mov bx, 0x1234
    sub ax, bx
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|SF|OF), ZF

    ; sub16 overflow: 0x8000 - 1 = 0x7FFF -> OF=1
    TEST_CASE ts16_3
    mov ax, 0x8000
    sub ax, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ OF, OF

    ;; ================================================================
    ;; SUB 64-bit
    ;; ================================================================

    ; sub64 borrow: 0 - 1 -> CF=1, SF=1
    TEST_CASE ts64_1
    xor rax, rax
    mov rbx, 1
    sub rax, rbx
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|SF), (CF|SF)

    ; sub64 zero: same - same = 0 -> ZF=1
    TEST_CASE ts64_2
    mov rax, 0x123456789ABCDEF0
    mov rbx, 0x123456789ABCDEF0
    sub rax, rbx
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|SF|OF), ZF

    ; sub64 overflow: 0x8000000000000000 - 1 -> OF=1
    TEST_CASE ts64_3
    mov rax, 0x8000000000000000
    mov rbx, 1
    sub rax, rbx
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ OF, OF

    ;; ================================================================
    ;; INC 8-bit (INC does NOT affect CF)
    ;; ================================================================

    ; inc8 zero: 0xFF + 1 = 0x00 -> ZF=1, CF unchanged
    TEST_CASE ti8_1
    mov al, 0xFF
    inc al
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ ZF, ZF

    ; inc8 overflow: 0x7F + 1 = 0x80 -> OF=1, SF=1
    TEST_CASE ti8_2
    mov al, 0x7F
    inc al
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (OF|SF), (OF|SF)

    ; inc8 preserves CF: set CF first, inc should not clear it
    TEST_CASE ti8_3
    stc                          ; set CF=1
    mov al, 5
    inc al                       ; should not touch CF
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ;; ================================================================
    ;; INC 16-bit
    ;; ================================================================

    ; inc16 zero: 0xFFFF + 1 = 0x0000 -> ZF=1
    TEST_CASE ti16_1
    mov ax, 0xFFFF
    inc ax
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ ZF, ZF

    ; inc16 overflow: 0x7FFF + 1 = 0x8000 -> OF=1, SF=1
    TEST_CASE ti16_2
    mov ax, 0x7FFF
    inc ax
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (OF|SF), (OF|SF)

    ;; ================================================================
    ;; INC 32-bit
    ;; ================================================================

    ; inc32 zero: 0xFFFFFFFF + 1 = 0 -> ZF=1
    TEST_CASE ti32_1
    mov eax, 0xFFFFFFFF
    inc eax
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ ZF, ZF

    ; inc32 overflow: 0x7FFFFFFF + 1 = 0x80000000 -> OF=1, SF=1
    TEST_CASE ti32_2
    mov eax, 0x7FFFFFFF
    inc eax
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (OF|SF), (OF|SF)

    ;; ================================================================
    ;; INC 64-bit
    ;; ================================================================

    ; inc64 zero: 0xFFFFFFFFFFFFFFFF + 1 = 0 -> ZF=1
    TEST_CASE ti64_1
    mov rax, 0xFFFFFFFFFFFFFFFF
    inc rax
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ ZF, ZF

    ; inc64 overflow: 0x7FFFFFFFFFFFFFFF + 1 -> OF=1, SF=1
    TEST_CASE ti64_2
    mov rax, 0x7FFFFFFFFFFFFFFF
    inc rax
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (OF|SF), (OF|SF)

    ;; ================================================================
    ;; DEC 8-bit (DEC does NOT affect CF)
    ;; ================================================================

    ; dec8 zero: 1 - 1 = 0 -> ZF=1
    TEST_CASE td8_1
    mov al, 1
    dec al
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ ZF, ZF

    ; dec8 sign: 0 - 1 = 0xFF -> SF=1
    TEST_CASE td8_2
    mov al, 0
    dec al
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ SF, SF

    ; dec8 overflow: 0x80 - 1 = 0x7F -> OF=1
    TEST_CASE td8_3
    mov al, 0x80
    dec al
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ OF, OF

    ;; ================================================================
    ;; DEC 16-bit
    ;; ================================================================

    ; dec16 zero: 1 - 1 = 0 -> ZF=1
    TEST_CASE td16_1
    mov ax, 1
    dec ax
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ ZF, ZF

    ; dec16 overflow: 0x8000 - 1 = 0x7FFF -> OF=1
    TEST_CASE td16_2
    mov ax, 0x8000
    dec ax
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ OF, OF

    ;; ================================================================
    ;; DEC 32-bit
    ;; ================================================================

    ; dec32 zero: 1 - 1 = 0 -> ZF=1
    TEST_CASE td32_1
    mov eax, 1
    dec eax
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ ZF, ZF

    ; dec32 overflow: 0x80000000 - 1 = 0x7FFFFFFF -> OF=1
    TEST_CASE td32_2
    mov eax, 0x80000000
    dec eax
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ OF, OF

    ;; ================================================================
    ;; DEC 64-bit
    ;; ================================================================

    ; dec64 zero: 1 - 1 = 0 -> ZF=1
    TEST_CASE td64_1
    mov rax, 1
    dec rax
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ ZF, ZF

    ; dec64 overflow: 0x8000000000000000 - 1 -> OF=1
    TEST_CASE td64_2
    mov rax, 0x8000000000000000
    dec rax
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ OF, OF

    ;; ================================================================
    ;; NEG 8-bit  (neg x = 0 - x; CF=1 unless x==0)
    ;; ================================================================

    ; neg8 zero: neg 0 = 0 -> ZF=1, CF=0
    TEST_CASE tn8_1
    mov al, 0
    neg al
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF), ZF

    ; neg8 nonzero: neg 1 = 0xFF -> CF=1, SF=1
    TEST_CASE tn8_2
    mov al, 1
    neg al
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|SF), (CF|SF)

    ; neg8 overflow: neg 0x80 = 0x80 -> OF=1, CF=1, SF=1
    TEST_CASE tn8_3
    mov al, 0x80
    neg al
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (OF|CF|SF), (OF|CF|SF)

    ;; ================================================================
    ;; NEG 16-bit
    ;; ================================================================

    ; neg16 zero: neg 0 = 0 -> ZF=1, CF=0
    TEST_CASE tn16_1
    mov ax, 0
    neg ax
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF), ZF

    ; neg16 nonzero: neg 1 = 0xFFFF -> CF=1, SF=1
    TEST_CASE tn16_2
    mov ax, 1
    neg ax
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|SF), (CF|SF)

    ; neg16 overflow: neg 0x8000 = 0x8000 -> OF=1, CF=1, SF=1
    TEST_CASE tn16_3
    mov ax, 0x8000
    neg ax
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (OF|CF|SF), (OF|CF|SF)

    ;; ================================================================
    ;; NEG 32-bit
    ;; ================================================================

    ; neg32 zero: neg 0 = 0 -> ZF=1, CF=0
    TEST_CASE tn32_1
    mov eax, 0
    neg eax
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF), ZF

    ; neg32 nonzero: neg 1 = 0xFFFFFFFF -> CF=1, SF=1
    TEST_CASE tn32_2
    mov eax, 1
    neg eax
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|SF), (CF|SF)

    ; neg32 overflow: neg 0x80000000 -> OF=1, CF=1, SF=1
    TEST_CASE tn32_3
    mov eax, 0x80000000
    neg eax
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (OF|CF|SF), (OF|CF|SF)

    ;; ================================================================
    ;; NEG 64-bit
    ;; ================================================================

    ; neg64 zero: neg 0 = 0 -> ZF=1, CF=0
    TEST_CASE tn64_1
    xor rax, rax
    neg rax
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF), ZF

    ; neg64 nonzero: neg 1 -> CF=1, SF=1
    TEST_CASE tn64_2
    mov rax, 1
    neg rax
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|SF), (CF|SF)

    ; neg64 overflow: neg 0x8000000000000000 -> OF=1, CF=1, SF=1
    TEST_CASE tn64_3
    mov rax, 0x8000000000000000
    neg rax
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (OF|CF|SF), (OF|CF|SF)

    ;; ================================================================
    END_TESTS
