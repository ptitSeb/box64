; test_x87.asm - Test x87 FPU operations
; FLD, FST, FADD, FSUB, FMUL, FDIV, FSQRT, FABS, FCHS, FXCH,
; FILD, FISTP, FCOM, FUCOM, FCOMI, FLDZ, FLD1, FLDPI, FRNDINT
; We verify results by storing to memory as doubles (via FST/FSTP)
; and comparing the IEEE 754 bit pattern.
%include "test_framework.inc"

section .data
    t1_name:  db "fld/fstp f64 round", 0
    t2_name:  db "fld1", 0
    t3_name:  db "fldz", 0
    t4_name:  db "fadd 1+2=3", 0
    t5_name:  db "fsub 5-3=2", 0
    t6_name:  db "fmul 3*4=12", 0
    t7_name:  db "fdiv 12/4=3", 0
    t8_name:  db "fsqrt 16=4", 0
    t9_name:  db "fabs -3 -> 3", 0
    t10_name: db "fchs 5 -> -5", 0
    t11_name: db "fild i32 42", 0
    t12_name: db "fild i64 100000", 0
    t13_name: db "fistp i32", 0
    t14_name: db "fistp i64", 0
    t15_name: db "fxch st0<->st1", 0
    t16_name: db "faddp st1,st0", 0
    t17_name: db "fsubp st1,st0", 0
    t18_name: db "fmulp st1,st0", 0
    t19_name: db "fdivp st1,st0", 0
    t20_name: db "fcomi eq", 0
    t21_name: db "fcomi lt", 0
    t22_name: db "fcomi gt", 0
    t23_name: db "frndint 3.7->4", 0
    t24_name: db "fld f32", 0
    t25_name: db "fadd mem64", 0
    t26_name: db "fmul mem32", 0
    t27_name: db "fild i16", 0
    t28_name: db "fist i16", 0
    t29_name: db "fld/fstp f64 neg", 0
    t30_name: db "fchs twice = id", 0

    align 8
    ; f64 values
    val_1d:    dq 0x3FF0000000000000  ; 1.0
    val_2d:    dq 0x4000000000000000  ; 2.0
    val_3d:    dq 0x4008000000000000  ; 3.0
    val_4d:    dq 0x4010000000000000  ; 4.0
    val_5d:    dq 0x4014000000000000  ; 5.0
    val_12d:   dq 0x4028000000000000  ; 12.0
    val_16d:   dq 0x4030000000000000  ; 16.0
    val_neg3d: dq 0xC008000000000000  ; -3.0
    val_neg5d: dq 0xC014000000000000  ; -5.0
    val_3_7d:  dq 0x400D99999999999A  ; 3.7

    ; f32 values
    val_3f:    dd 0x40400000           ; 3.0f
    val_4f:    dd 0x40800000           ; 4.0f

    ; Integer values for FILD/FISTP
    val_i32_42:    dd 42
    val_i64_100k:  dq 100000
    val_i16_7:     dw 7

section .bss
    alignb 8
    result_f64: resq 1
    result_f32: resd 1
    result_i32: resd 1
    result_i64: resq 1
    result_i16: resw 1

section .text
global _start

_start:
    INIT_TESTS

    ; Initialize x87 FPU
    finit

    ; ==== Test 1: fld f64 / fstp f64 roundtrip ====
    TEST_CASE t1_name
    fld qword [rel val_3d]         ; push 3.0 to ST(0)
    fstp qword [rel result_f64]    ; pop to memory
    mov rax, [rel result_f64]
    CHECK_EQ_64 rax, 0x4008000000000000  ; 3.0

    ; ==== Test 2: fld1 ====
    TEST_CASE t2_name
    fld1                           ; push 1.0
    fstp qword [rel result_f64]
    mov rax, [rel result_f64]
    CHECK_EQ_64 rax, 0x3FF0000000000000  ; 1.0

    ; ==== Test 3: fldz ====
    TEST_CASE t3_name
    fldz                           ; push 0.0
    fstp qword [rel result_f64]
    mov rax, [rel result_f64]
    CHECK_EQ_64 rax, 0x0000000000000000  ; +0.0

    ; ==== Test 4: fadd 1.0 + 2.0 = 3.0 ====
    TEST_CASE t4_name
    fld qword [rel val_1d]         ; ST(0) = 1.0
    fld qword [rel val_2d]         ; ST(0) = 2.0, ST(1) = 1.0
    faddp st1, st0                 ; ST(0) = 3.0
    fstp qword [rel result_f64]
    mov rax, [rel result_f64]
    CHECK_EQ_64 rax, 0x4008000000000000  ; 3.0

    ; ==== Test 5: fsub 5.0 - 3.0 = 2.0 ====
    TEST_CASE t5_name
    fld qword [rel val_5d]         ; ST(0) = 5.0
    fld qword [rel val_3d]         ; ST(0) = 3.0, ST(1) = 5.0
    fsubp st1, st0                 ; ST(0) = ST(1) - ST(0) = 5.0 - 3.0 = 2.0
    fstp qword [rel result_f64]
    mov rax, [rel result_f64]
    CHECK_EQ_64 rax, 0x4000000000000000  ; 2.0

    ; ==== Test 6: fmul 3.0 * 4.0 = 12.0 ====
    TEST_CASE t6_name
    fld qword [rel val_3d]         ; ST(0) = 3.0
    fld qword [rel val_4d]         ; ST(0) = 4.0, ST(1) = 3.0
    fmulp st1, st0                 ; ST(0) = 12.0
    fstp qword [rel result_f64]
    mov rax, [rel result_f64]
    CHECK_EQ_64 rax, 0x4028000000000000  ; 12.0

    ; ==== Test 7: fdiv 12.0 / 4.0 = 3.0 ====
    TEST_CASE t7_name
    fld qword [rel val_12d]        ; ST(0) = 12.0
    fld qword [rel val_4d]         ; ST(0) = 4.0, ST(1) = 12.0
    fdivp st1, st0                 ; ST(0) = ST(1) / ST(0) = 12.0 / 4.0 = 3.0
    fstp qword [rel result_f64]
    mov rax, [rel result_f64]
    CHECK_EQ_64 rax, 0x4008000000000000  ; 3.0

    ; ==== Test 8: fsqrt(16.0) = 4.0 ====
    TEST_CASE t8_name
    fld qword [rel val_16d]        ; ST(0) = 16.0
    fsqrt                          ; ST(0) = 4.0
    fstp qword [rel result_f64]
    mov rax, [rel result_f64]
    CHECK_EQ_64 rax, 0x4010000000000000  ; 4.0

    ; ==== Test 9: fabs(-3.0) = 3.0 ====
    TEST_CASE t9_name
    fld qword [rel val_neg3d]      ; ST(0) = -3.0
    fabs                           ; ST(0) = 3.0
    fstp qword [rel result_f64]
    mov rax, [rel result_f64]
    CHECK_EQ_64 rax, 0x4008000000000000  ; 3.0

    ; ==== Test 10: fchs(5.0) = -5.0 ====
    TEST_CASE t10_name
    fld qword [rel val_5d]         ; ST(0) = 5.0
    fchs                           ; ST(0) = -5.0
    fstp qword [rel result_f64]
    mov rax, [rel result_f64]
    CHECK_EQ_64 rax, 0xC014000000000000  ; -5.0

    ; ==== Test 11: fild i32 42 -> 42.0 ====
    TEST_CASE t11_name
    fild dword [rel val_i32_42]    ; ST(0) = 42.0
    fstp qword [rel result_f64]
    mov rax, [rel result_f64]
    CHECK_EQ_64 rax, 0x4045000000000000  ; 42.0

    ; ==== Test 12: fild i64 100000 -> 100000.0 ====
    TEST_CASE t12_name
    fild qword [rel val_i64_100k]  ; ST(0) = 100000.0
    fstp qword [rel result_f64]
    mov rax, [rel result_f64]
    ; 100000.0 = 0x40F86A0000000000
    CHECK_EQ_64 rax, 0x40F86A0000000000

    ; ==== Test 13: fistp i32: 42.0 -> 42 ====
    TEST_CASE t13_name
    fld qword [rel val_3d]         ; ST(0) = 3.0
    fistp dword [rel result_i32]
    mov eax, [rel result_i32]
    CHECK_EQ_32 eax, 3

    ; ==== Test 14: fistp i64: 12.0 -> 12 ====
    TEST_CASE t14_name
    fld qword [rel val_12d]        ; ST(0) = 12.0
    fistp qword [rel result_i64]
    mov rax, [rel result_i64]
    CHECK_EQ_64 rax, 12

    ; ==== Test 15: fxch st0 <-> st1 ====
    TEST_CASE t15_name
    fld qword [rel val_3d]         ; ST(0) = 3.0
    fld qword [rel val_5d]         ; ST(0) = 5.0, ST(1) = 3.0
    fxch st1                       ; ST(0) = 3.0, ST(1) = 5.0
    fstp qword [rel result_f64]    ; store 3.0, ST(0) = 5.0
    mov rax, [rel result_f64]
    push rax
    fstp qword [rel result_f64]    ; store 5.0
    mov rax, [rel result_f64]
    pop rcx
    ; rcx should be 3.0 (0x4008000000000000)
    ; rax should be 5.0 (0x4014000000000000)
    ; NOTE: can't use cmp r64, large_imm; x86 only sign-extends 32-bit imm
    mov rbx, 0x4008000000000000
    cmp rcx, rbx
    jne .t15_fail
    mov rbx, 0x4014000000000000
    cmp rax, rbx
    jne .t15_fail
    push rsi
    lea rsi, [rel _fw_pass_msg]
    call _fw_print_str
    inc r12d
    pop rsi
    jmp .t15_done
.t15_fail:
    push rsi
    lea rsi, [rel _fw_fail_msg]
    call _fw_print_str
    pop rsi
.t15_done:

    ; ==== Test 16: faddp st1, st0 ====
    TEST_CASE t16_name
    fld qword [rel val_4d]         ; ST(0) = 4.0
    fld qword [rel val_5d]         ; ST(0) = 5.0, ST(1) = 4.0
    faddp st1, st0                 ; ST(0) = 9.0
    fstp qword [rel result_f64]
    mov rax, [rel result_f64]
    ; 9.0 = 0x4022000000000000
    CHECK_EQ_64 rax, 0x4022000000000000

    ; ==== Test 17: fsubp st1, st0 (st1 - st0) ====
    TEST_CASE t17_name
    fld qword [rel val_12d]        ; ST(0) = 12.0
    fld qword [rel val_4d]         ; ST(0) = 4.0, ST(1) = 12.0
    fsubp st1, st0                 ; ST(0) = 12.0 - 4.0 = 8.0
    fstp qword [rel result_f64]
    mov rax, [rel result_f64]
    ; 8.0 = 0x4020000000000000
    CHECK_EQ_64 rax, 0x4020000000000000

    ; ==== Test 18: fmulp st1, st0 ====
    TEST_CASE t18_name
    fld qword [rel val_2d]         ; ST(0) = 2.0
    fld qword [rel val_5d]         ; ST(0) = 5.0, ST(1) = 2.0
    fmulp st1, st0                 ; ST(0) = 10.0
    fstp qword [rel result_f64]
    mov rax, [rel result_f64]
    ; 10.0 = 0x4024000000000000
    CHECK_EQ_64 rax, 0x4024000000000000

    ; ==== Test 19: fdivp st1, st0 (st1 / st0) ====
    TEST_CASE t19_name
    fld qword [rel val_16d]        ; ST(0) = 16.0
    fld qword [rel val_4d]         ; ST(0) = 4.0, ST(1) = 16.0
    fdivp st1, st0                 ; ST(0) = 16.0 / 4.0 = 4.0
    fstp qword [rel result_f64]
    mov rax, [rel result_f64]
    CHECK_EQ_64 rax, 0x4010000000000000  ; 4.0

    ; ==== Test 20: fcomi equal -> ZF=1 ====
    TEST_CASE t20_name
    fld qword [rel val_3d]         ; ST(0) = 3.0
    fld qword [rel val_3d]         ; ST(0) = 3.0, ST(1) = 3.0
    fcomi st0, st1
    SAVE_FLAGS
    fstp qword [rel result_f64]    ; pop ST(0)
    fstp qword [rel result_f64]    ; pop ST(1)
    CHECK_FLAGS_EQ (ZF|CF|PF), ZF

    ; ==== Test 21: fcomi less than -> CF=1 ====
    TEST_CASE t21_name
    fld qword [rel val_5d]         ; ST(1) = 5.0
    fld qword [rel val_3d]         ; ST(0) = 3.0
    fcomi st0, st1                 ; 3.0 < 5.0 -> CF=1
    SAVE_FLAGS
    fstp qword [rel result_f64]
    fstp qword [rel result_f64]
    CHECK_FLAGS_EQ (ZF|CF|PF), CF

    ; ==== Test 22: fcomi greater than -> CF=0 ZF=0 ====
    TEST_CASE t22_name
    fld qword [rel val_3d]         ; ST(1) = 3.0
    fld qword [rel val_5d]         ; ST(0) = 5.0
    fcomi st0, st1                 ; 5.0 > 3.0 -> CF=0, ZF=0
    SAVE_FLAGS
    fstp qword [rel result_f64]
    fstp qword [rel result_f64]
    CHECK_FLAGS_EQ (ZF|CF|PF), 0

    ; ==== Test 23: frndint 3.7 -> 4.0 (round to nearest even, default) ====
    TEST_CASE t23_name
    fld qword [rel val_3_7d]       ; ST(0) = 3.7
    frndint                        ; ST(0) = 4.0 (round to nearest)
    fstp qword [rel result_f64]
    mov rax, [rel result_f64]
    CHECK_EQ_64 rax, 0x4010000000000000  ; 4.0

    ; ==== Test 24: fld f32 -> extended -> fstp f64 ====
    TEST_CASE t24_name
    fld dword [rel val_3f]         ; load 3.0f as 80-bit extended
    fstp qword [rel result_f64]    ; store as double
    mov rax, [rel result_f64]
    CHECK_EQ_64 rax, 0x4008000000000000  ; 3.0

    ; ==== Test 25: fadd with mem64: ST(0) += [mem] ====
    TEST_CASE t25_name
    fld qword [rel val_1d]         ; ST(0) = 1.0
    fadd qword [rel val_4d]        ; ST(0) += 4.0 = 5.0
    fstp qword [rel result_f64]
    mov rax, [rel result_f64]
    CHECK_EQ_64 rax, 0x4014000000000000  ; 5.0

    ; ==== Test 26: fmul with mem32: ST(0) *= [mem32] ====
    TEST_CASE t26_name
    fld qword [rel val_3d]         ; ST(0) = 3.0
    fmul dword [rel val_4f]        ; ST(0) *= 4.0f = 12.0
    fstp qword [rel result_f64]
    mov rax, [rel result_f64]
    CHECK_EQ_64 rax, 0x4028000000000000  ; 12.0

    ; ==== Test 27: fild i16 ====
    TEST_CASE t27_name
    fild word [rel val_i16_7]      ; ST(0) = 7.0
    fstp qword [rel result_f64]
    mov rax, [rel result_f64]
    ; 7.0 = 0x401C000000000000
    CHECK_EQ_64 rax, 0x401C000000000000

    ; ==== Test 28: fistp i16: 5.0 -> 5 ====
    TEST_CASE t28_name
    fld qword [rel val_5d]         ; ST(0) = 5.0
    fistp word [rel result_i16]
    movzx eax, word [rel result_i16]
    CHECK_EQ_32 eax, 5

    ; ==== Test 29: fld/fstp negative: -5.0 ====
    TEST_CASE t29_name
    fld qword [rel val_neg5d]      ; ST(0) = -5.0
    fstp qword [rel result_f64]
    mov rax, [rel result_f64]
    CHECK_EQ_64 rax, 0xC014000000000000  ; -5.0

    ; ==== Test 30: fchs twice = identity ====
    TEST_CASE t30_name
    fld qword [rel val_3d]         ; ST(0) = 3.0
    fchs                           ; ST(0) = -3.0
    fchs                           ; ST(0) = 3.0
    fstp qword [rel result_f64]
    mov rax, [rel result_f64]
    CHECK_EQ_64 rax, 0x4008000000000000  ; 3.0

    END_TESTS
