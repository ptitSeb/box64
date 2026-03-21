; test_updateflags_shift.asm - Deferred flag tests for shift/rotate ops
; Covers: shl/shr/sar/rol/ror for 8/16/32/64 bit widths
; Each test: instruction -> call flag_barrier_ret -> check flags
;
; Flag behavior for shifts:
;   SHL/SHR/SAR: CF = last bit shifted out, ZF/SF/PF set from result
;                OF defined only for count=1
;   ROL/ROR:     CF = last bit rotated, OF defined only for count=1
;                ZF/SF/PF are NOT modified by rotates
%include "test_framework.inc"

section .data
    ; --- shl ---
    tshl8_1:  db "shl8 CF=last bit out via call", 0
    tshl8_2:  db "shl8 zero result via call", 0
    tshl16_1: db "shl16 CF=1 via call", 0
    tshl16_2: db "shl16 zero result via call", 0
    tshl32_1: db "shl32 CF=1 via call", 0
    tshl32_2: db "shl32 zero result via call", 0
    tshl64_1: db "shl64 CF=1 via call", 0
    tshl64_2: db "shl64 zero result via call", 0
    ; --- shr ---
    tshr8_1:  db "shr8 CF=last bit out via call", 0
    tshr8_2:  db "shr8 zero result via call", 0
    tshr16_1: db "shr16 CF=1 via call", 0
    tshr16_2: db "shr16 zero result via call", 0
    tshr32_1: db "shr32 CF=1 via call", 0
    tshr32_2: db "shr32 zero result via call", 0
    tshr64_1: db "shr64 CF=1 via call", 0
    tshr64_2: db "shr64 zero result via call", 0
    ; --- sar ---
    tsar8_1:  db "sar8 CF=last bit out via call", 0
    tsar8_2:  db "sar8 sign extend CF=0 via call", 0
    tsar16_1: db "sar16 CF=1 via call", 0
    tsar16_2: db "sar16 sign extend CF=0 via call", 0
    tsar32_1: db "sar32 CF=1 via call", 0
    tsar32_2: db "sar32 sign extend CF=0 via call", 0
    tsar64_1: db "sar64 CF=1 via call", 0
    tsar64_2: db "sar64 sign extend CF=0 via call", 0
    ; --- rol ---
    trol8_1:  db "rol8 CF=rotated bit via call", 0
    trol8_2:  db "rol8 OF=1 for count=1 via call", 0
    trol16_1: db "rol16 CF via call", 0
    trol32_1: db "rol32 CF via call", 0
    trol64_1: db "rol64 CF via call", 0
    ; --- ror ---
    tror8_1:  db "ror8 CF=rotated bit via call", 0
    tror8_2:  db "ror8 OF=1 for count=1 via call", 0
    tror16_1: db "ror16 CF via call", 0
    tror32_1: db "ror32 CF via call", 0
    tror64_1: db "ror64 CF via call", 0
    ; --- shift by cl ---
    tshcl_1:  db "shl32 by cl=5 via call", 0
    tshcl_2:  db "shr64 by cl=1 via call", 0
    tshcl_3:  db "sar8 by cl=7 via call", 0

section .text
global _start

flag_barrier_ret:
    ret

_start:
    INIT_TESTS

    ;; ================================================================
    ;; SHL (shift left)
    ;; ================================================================

    ; shl8: 0x81 << 1 = 0x02, CF=1 (bit 7 shifted out)
    TEST_CASE tshl8_1
    mov al, 0x81
    shl al, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ; shl8: 0x80 << 1 = 0x00, CF=1, ZF=1
    TEST_CASE tshl8_2
    mov al, 0x80
    shl al, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|ZF), (CF|ZF)

    ; shl16: 0x8001 << 1 = 0x0002, CF=1
    TEST_CASE tshl16_1
    mov ax, 0x8001
    shl ax, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ; shl16: 0x8000 << 1 = 0x0000, CF=1, ZF=1
    TEST_CASE tshl16_2
    mov ax, 0x8000
    shl ax, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|ZF), (CF|ZF)

    ; shl32: 0x80000001 << 1 = 0x00000002, CF=1
    TEST_CASE tshl32_1
    mov eax, 0x80000001
    shl eax, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ; shl32: 0x80000000 << 1 = 0, CF=1, ZF=1
    TEST_CASE tshl32_2
    mov eax, 0x80000000
    shl eax, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|ZF), (CF|ZF)

    ; shl64: bit63 set << 1, CF=1
    TEST_CASE tshl64_1
    mov rax, 0x8000000000000001
    shl rax, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ; shl64: 0x8000000000000000 << 1 = 0, CF=1, ZF=1
    TEST_CASE tshl64_2
    mov rax, 0x8000000000000000
    shl rax, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|ZF), (CF|ZF)

    ;; ================================================================
    ;; SHR (shift right logical)
    ;; ================================================================

    ; shr8: 0x81 >> 1 = 0x40, CF=1 (bit 0 shifted out)
    TEST_CASE tshr8_1
    mov al, 0x81
    shr al, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ; shr8: 0x01 >> 1 = 0x00, CF=1, ZF=1
    TEST_CASE tshr8_2
    mov al, 0x01
    shr al, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|ZF), (CF|ZF)

    ; shr16: 0x0001 >> 1 = 0, CF=1, ZF=1
    TEST_CASE tshr16_1
    mov ax, 0x0001
    shr ax, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|ZF), (CF|ZF)

    ; shr16: 0x0002 >> 2 = 0, CF=1, ZF=1
    TEST_CASE tshr16_2
    mov ax, 0x0002
    shr ax, 2
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|ZF), (CF|ZF)

    ; shr32: 0x00000001 >> 1 = 0, CF=1, ZF=1
    TEST_CASE tshr32_1
    mov eax, 0x00000001
    shr eax, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|ZF), (CF|ZF)

    ; shr32: 0x80000000 >> 31 = 1, CF=0
    TEST_CASE tshr32_2
    mov eax, 0x80000000
    mov cl, 31
    shr eax, cl
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|ZF), 0

    ; shr64: 0x01 >> 1 = 0, CF=1, ZF=1
    TEST_CASE tshr64_1
    mov rax, 1
    shr rax, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|ZF), (CF|ZF)

    ; shr64: large >> 63 = 1, CF=0
    TEST_CASE tshr64_2
    mov rax, 0x8000000000000000
    mov cl, 63
    shr rax, cl
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|ZF), 0

    ;; ================================================================
    ;; SAR (shift right arithmetic - sign-extending)
    ;; ================================================================

    ; sar8: 0x81 >> 1 = 0xC0, CF=1 (bit 0 out), SF=1 (sign preserved)
    TEST_CASE tsar8_1
    mov al, 0x81
    sar al, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|SF), (CF|SF)

    ; sar8: 0x80 >> 7 = 0xFF, CF=0 (last bit out is 0), SF=1
    TEST_CASE tsar8_2
    mov al, 0x80
    sar al, 7
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|SF|ZF), SF

    ; sar16: 0x8001 >> 1 = 0xC000, CF=1, SF=1
    TEST_CASE tsar16_1
    mov ax, 0x8001
    sar ax, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|SF), (CF|SF)

    ; sar16: 0x8000 >> 15 = 0xFFFF, CF=0, SF=1
    TEST_CASE tsar16_2
    mov ax, 0x8000
    mov cl, 15
    sar ax, cl
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|SF|ZF), SF

    ; sar32: 0x80000001 >> 1 = 0xC0000000, CF=1, SF=1
    TEST_CASE tsar32_1
    mov eax, 0x80000001
    sar eax, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|SF), (CF|SF)

    ; sar32: 0x80000000 >> 31 = 0xFFFFFFFF, CF=0, SF=1
    TEST_CASE tsar32_2
    mov eax, 0x80000000
    mov cl, 31
    sar eax, cl
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|SF|ZF), SF

    ; sar64: 0x8000000000000001 >> 1, CF=1, SF=1
    TEST_CASE tsar64_1
    mov rax, 0x8000000000000001
    sar rax, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|SF), (CF|SF)

    ; sar64: 0x8000000000000000 >> 63 = 0xFFFFFFFFFFFFFFFF, CF=0, SF=1
    TEST_CASE tsar64_2
    mov rax, 0x8000000000000000
    mov cl, 63
    sar rax, cl
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|SF|ZF), SF

    ;; ================================================================
    ;; ROL (rotate left)
    ;; CF = bit 0 of result (last bit rotated into position 0)
    ;; OF defined only for count=1: OF = MSB(result) XOR CF
    ;; ================================================================

    ; rol8: 0x81 rol 1 = 0x03, CF=1 (bit 7 was 1, rotated to bit 0)
    TEST_CASE trol8_1
    mov al, 0x81
    rol al, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ; rol8: 0x40 rol 1 = 0x80, CF=0, OF=1 (MSB changed)
    TEST_CASE trol8_2
    mov al, 0x40
    rol al, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|OF), OF

    ; rol16: 0x8001 rol 1 = 0x0003, CF=1
    TEST_CASE trol16_1
    mov ax, 0x8001
    rol ax, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ; rol32: 0x80000001 rol 1 = 0x00000003, CF=1
    TEST_CASE trol32_1
    mov eax, 0x80000001
    rol eax, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ; rol64: bit63 set, rol 1, CF=1
    TEST_CASE trol64_1
    mov rax, 0x8000000000000001
    rol rax, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ;; ================================================================
    ;; ROR (rotate right)
    ;; CF = MSB of result (last bit rotated into MSB position)
    ;; OF defined only for count=1: OF = MSB(result) XOR next-to-MSB(result)
    ;; ================================================================

    ; ror8: 0x01 ror 1 = 0x80, CF=1 (bit 0 was 1, rotated to bit 7)
    TEST_CASE tror8_1
    mov al, 0x01
    ror al, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ; ror8: 0x01 ror 1 = 0x80 -> MSB=1, next-to-MSB=0 -> OF=1
    TEST_CASE tror8_2
    mov al, 0x01
    ror al, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|OF), (CF|OF)

    ; ror16: 0x0001 ror 1 = 0x8000, CF=1
    TEST_CASE tror16_1
    mov ax, 0x0001
    ror ax, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ; ror32: 0x00000001 ror 1 = 0x80000000, CF=1
    TEST_CASE tror32_1
    mov eax, 0x00000001
    ror eax, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ; ror64: 0x01 ror 1 = 0x8000000000000000, CF=1
    TEST_CASE tror64_1
    mov rax, 1
    ror rax, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ;; ================================================================
    ;; Shifts by CL register (dynamic count)
    ;; ================================================================

    ; shl32 by cl=5: 0x00000001 << 5 = 0x00000020, CF=0
    TEST_CASE tshcl_1
    mov eax, 0x00000001
    mov cl, 5
    shl eax, cl
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|ZF), 0

    ; shr64 by cl=1: 0x03 >> 1 = 0x01, CF=1
    TEST_CASE tshcl_2
    mov rax, 0x03
    mov cl, 1
    shr rax, cl
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|ZF), CF

    ; sar8 by cl=7: 0xFF >> 7 = 0xFF (all 1s sign extended), CF=1, SF=1
    TEST_CASE tshcl_3
    mov al, 0xFF
    mov cl, 7
    sar al, cl
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|SF|ZF), (CF|SF)

    ;; ================================================================
    END_TESTS
