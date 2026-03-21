; test_updateflags_mul.asm - Deferred flag tests for mul/imul
; Covers: mul8/16/32/64, imul8/16/32/64 (single-operand form)
; Each test: instruction -> call flag_barrier_ret -> check flags
;
; MUL/IMUL flag behavior:
;   CF=OF=1 if upper half of result is nonzero (result doesn't fit in lower half)
;   CF=OF=0 if upper half is zero (result fits in lower half)
;   SF, ZF, AF, PF are undefined after MUL/IMUL
%include "test_framework.inc"

section .data
    ; --- mul8 ---
    tmul8_1:  db "mul8 small (no overflow) via call", 0
    tmul8_2:  db "mul8 overflow (CF=OF=1) via call", 0
    tmul8_3:  db "mul8 zero result via call", 0
    ; --- mul16 ---
    tmul16_1: db "mul16 small (no overflow) via call", 0
    tmul16_2: db "mul16 overflow (CF=OF=1) via call", 0
    tmul16_3: db "mul16 zero result via call", 0
    ; --- mul32 ---
    tmul32_1: db "mul32 small (no overflow) via call", 0
    tmul32_2: db "mul32 overflow (CF=OF=1) via call", 0
    tmul32_3: db "mul32 zero result via call", 0
    ; --- mul64 ---
    tmul64_1: db "mul64 small (no overflow) via call", 0
    tmul64_2: db "mul64 overflow (CF=OF=1) via call", 0
    tmul64_3: db "mul64 zero result via call", 0
    ; --- imul8 ---
    timul8_1: db "imul8 small pos (no overflow)", 0
    timul8_2: db "imul8 overflow (CF=OF=1)", 0
    timul8_3: db "imul8 neg*neg no overflow", 0
    timul8_4: db "imul8 neg*pos overflow", 0
    ; --- imul16 ---
    timul16_1: db "imul16 small (no overflow)", 0
    timul16_2: db "imul16 overflow (CF=OF=1)", 0
    ; --- imul32 ---
    timul32_1: db "imul32 small (no overflow)", 0
    timul32_2: db "imul32 overflow (CF=OF=1)", 0
    ; --- imul64 ---
    timul64_1: db "imul64 small (no overflow)", 0
    timul64_2: db "imul64 overflow (CF=OF=1)", 0

section .bss
    mul_op8:  resb 1
    mul_op16: resw 1
    mul_op32: resd 1
    mul_op64: resq 1

section .text
global _start

flag_barrier_ret:
    ret

_start:
    INIT_TESTS

    ;; ================================================================
    ;; MUL 8-bit: AX = AL * r/m8
    ;; CF=OF=0 if AH is zero, CF=OF=1 if AH is nonzero
    ;; ================================================================

    ; mul8 small: 3 * 4 = 12, fits in AL -> CF=OF=0
    TEST_CASE tmul8_1
    mov al, 3
    mov byte [rel mul_op8], 4
    mul byte [rel mul_op8]
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|OF), 0

    ; mul8 overflow: 16 * 16 = 256, doesn't fit in AL -> CF=OF=1
    TEST_CASE tmul8_2
    mov al, 16
    mov byte [rel mul_op8], 16
    mul byte [rel mul_op8]
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|OF), (CF|OF)

    ; mul8 zero: 0 * 255 = 0 -> CF=OF=0
    TEST_CASE tmul8_3
    mov al, 0
    mov byte [rel mul_op8], 255
    mul byte [rel mul_op8]
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|OF), 0

    ;; ================================================================
    ;; MUL 16-bit: DX:AX = AX * r/m16
    ;; CF=OF=0 if DX is zero, CF=OF=1 if DX is nonzero
    ;; ================================================================

    ; mul16 small: 100 * 200 = 20000, fits in AX -> CF=OF=0
    TEST_CASE tmul16_1
    mov ax, 100
    mov word [rel mul_op16], 200
    mul word [rel mul_op16]
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|OF), 0

    ; mul16 overflow: 0x100 * 0x100 = 0x10000, DX=1 -> CF=OF=1
    TEST_CASE tmul16_2
    mov ax, 0x100
    mov word [rel mul_op16], 0x100
    mul word [rel mul_op16]
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|OF), (CF|OF)

    ; mul16 zero: 0 * 0xFFFF = 0 -> CF=OF=0
    TEST_CASE tmul16_3
    mov ax, 0
    mov word [rel mul_op16], 0xFFFF
    mul word [rel mul_op16]
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|OF), 0

    ;; ================================================================
    ;; MUL 32-bit: EDX:EAX = EAX * r/m32
    ;; CF=OF=0 if EDX is zero, CF=OF=1 if EDX is nonzero
    ;; ================================================================

    ; mul32 small: 1000 * 2000 = 2000000, fits in EAX -> CF=OF=0
    TEST_CASE tmul32_1
    mov eax, 1000
    mov dword [rel mul_op32], 2000
    mul dword [rel mul_op32]
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|OF), 0

    ; mul32 overflow: 0x10000 * 0x10000 = 0x100000000, EDX=1 -> CF=OF=1
    TEST_CASE tmul32_2
    mov eax, 0x10000
    mov dword [rel mul_op32], 0x10000
    mul dword [rel mul_op32]
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|OF), (CF|OF)

    ; mul32 zero: 0 * anything = 0 -> CF=OF=0
    TEST_CASE tmul32_3
    mov eax, 0
    mov dword [rel mul_op32], 0xFFFFFFFF
    mul dword [rel mul_op32]
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|OF), 0

    ;; ================================================================
    ;; MUL 64-bit: RDX:RAX = RAX * r/m64
    ;; CF=OF=0 if RDX is zero, CF=OF=1 if RDX is nonzero
    ;; ================================================================

    ; mul64 small: 1000000 * 2000000 = 2e12, fits in RAX -> CF=OF=0
    TEST_CASE tmul64_1
    mov rax, 1000000
    mov qword [rel mul_op64], 2000000
    mul qword [rel mul_op64]
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|OF), 0

    ; mul64 overflow: 0x100000000 * 0x100000000, RDX=1 -> CF=OF=1
    TEST_CASE tmul64_2
    mov rax, 0x100000000
    mov rbx, 0x100000000
    mov qword [rel mul_op64], rbx
    mul qword [rel mul_op64]
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|OF), (CF|OF)

    ; mul64 zero: 0 * anything = 0 -> CF=OF=0
    TEST_CASE tmul64_3
    xor rax, rax
    mov qword [rel mul_op64], 0xFFFFFFFFFFFFFFFF
    mul qword [rel mul_op64]
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|OF), 0

    ;; ================================================================
    ;; IMUL 8-bit (single-operand): AX = AL * r/m8 (signed)
    ;; CF=OF=0 if result sign-extends from AL to AX (AH = sign-extension of AL)
    ;; CF=OF=1 otherwise
    ;; ================================================================

    ; imul8 small positive: 3 * 4 = 12, fits in signed AL -> CF=OF=0
    TEST_CASE timul8_1
    mov al, 3
    mov byte [rel mul_op8], 4
    imul byte [rel mul_op8]
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|OF), 0

    ; imul8 overflow: 64 * 4 = 256, doesn't fit in signed AL -> CF=OF=1
    TEST_CASE timul8_2
    mov al, 64
    mov byte [rel mul_op8], 4
    imul byte [rel mul_op8]
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|OF), (CF|OF)

    ; imul8 neg*neg no overflow: (-1) * (-1) = 1, fits -> CF=OF=0
    TEST_CASE timul8_3
    mov al, 0xFF          ; -1 signed
    mov byte [rel mul_op8], 0xFF  ; -1 signed
    imul byte [rel mul_op8]
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|OF), 0

    ; imul8 neg*pos overflow: (-128) * 2 = -256, doesn't fit in AL -> CF=OF=1
    TEST_CASE timul8_4
    mov al, 0x80          ; -128 signed
    mov byte [rel mul_op8], 2
    imul byte [rel mul_op8]
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|OF), (CF|OF)

    ;; ================================================================
    ;; IMUL 16-bit (single-operand): DX:AX = AX * r/m16 (signed)
    ;; CF=OF=0 if DX is sign-extension of AX, CF=OF=1 otherwise
    ;; ================================================================

    ; imul16 small: 100 * (-2) = -200, fits in AX -> CF=OF=0
    TEST_CASE timul16_1
    mov ax, 100
    mov word [rel mul_op16], 0xFFFE  ; -2 signed
    imul word [rel mul_op16]
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|OF), 0

    ; imul16 overflow: 0x100 * 0x100 = 0x10000, doesn't fit -> CF=OF=1
    TEST_CASE timul16_2
    mov ax, 0x100
    mov word [rel mul_op16], 0x100
    imul word [rel mul_op16]
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|OF), (CF|OF)

    ;; ================================================================
    ;; IMUL 32-bit (single-operand): EDX:EAX = EAX * r/m32 (signed)
    ;; CF=OF=0 if EDX is sign-extension of EAX, CF=OF=1 otherwise
    ;; ================================================================

    ; imul32 small: 1000 * (-2) = -2000, fits in EAX -> CF=OF=0
    TEST_CASE timul32_1
    mov eax, 1000
    mov dword [rel mul_op32], 0xFFFFFFFE  ; -2 signed
    imul dword [rel mul_op32]
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|OF), 0

    ; imul32 overflow: 0x10000 * 0x10000 = 0x100000000, doesn't fit -> CF=OF=1
    TEST_CASE timul32_2
    mov eax, 0x10000
    mov dword [rel mul_op32], 0x10000
    imul dword [rel mul_op32]
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|OF), (CF|OF)

    ;; ================================================================
    ;; IMUL 64-bit (single-operand): RDX:RAX = RAX * r/m64 (signed)
    ;; CF=OF=0 if RDX is sign-extension of RAX, CF=OF=1 otherwise
    ;; ================================================================

    ; imul64 small: 1000000 * (-2) = -2000000, fits in RAX -> CF=OF=0
    TEST_CASE timul64_1
    mov rax, 1000000
    mov qword [rel mul_op64], -2
    imul qword [rel mul_op64]
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|OF), 0

    ; imul64 overflow: 0x100000000 * 0x100000000, doesn't fit -> CF=OF=1
    TEST_CASE timul64_2
    mov rax, 0x100000000
    mov rbx, 0x100000000
    mov qword [rel mul_op64], rbx
    imul qword [rel mul_op64]
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|OF), (CF|OF)

    ;; ================================================================
    END_TESTS
