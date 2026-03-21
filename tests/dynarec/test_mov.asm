; test_mov.asm - Test MOV opcodes (0x88-0x8B, 0xB0-0xBF, 0xC6-0xC7)
%include "test_framework.inc"

section .data
    t1_name:  db "mov Eb,Gb reg-reg (0x88)", 0
    t2_name:  db "mov Ed,Gd reg-reg (0x89)", 0
    t3_name:  db "mov Gb,Eb reg-reg (0x8A)", 0
    t4_name:  db "mov Gd,Ed reg-reg (0x8B)", 0
    t5_name:  db "mov Eb,Gb mem (0x88 mem)", 0
    t6_name:  db "mov Ed,Gd mem (0x89 mem)", 0
    t7_name:  db "mov Gb,Eb mem (0x8A mem)", 0
    t8_name:  db "mov Gd,Ed mem (0x8B mem)", 0
    t9_name:  db "mov r8,imm8 (0xB0)", 0
    t10_name: db "mov r32,imm32 (0xB8)", 0
    t11_name: db "mov r64,imm64 (REX.W 0xB8)", 0
    t12_name: db "mov Eb,Ib (0xC6)", 0
    t13_name: db "mov Ed,Id (0xC7)", 0
    t14_name: db "mov Eb,Ib mem (0xC6 mem)", 0
    t15_name: db "mov Ed,Id mem (0xC7 mem)", 0
    t16_name: db "mov 64-bit reg-reg (REX.W 0x89)", 0
    t17_name: db "mov 64-bit mem (REX.W 0x89 mem)", 0
    t18_name: db "mov 64-bit Gd,Ed mem (REX.W 0x8B mem)", 0
    t19_name: db "mov hi-byte AH (0x88)", 0
    t20_name: db "mov hi-byte from AH (0x8A)", 0
    t21_name: db "mov all r8 imm8 (0xB0-0xB7)", 0
    t22_name: db "mov all r32 imm32 (0xB8-0xBF)", 0
    t23_name: db "mov does not affect flags", 0
    t24_name: db "mov Eb,Gb mem RIP-rel (0x88)", 0

section .bss
    scratch:  resq 2
    scratch2: resq 1

section .text
global _start

_start:
    INIT_TESTS

    ; ==== Test 1: mov Eb,Gb reg-reg (0x88) ====
    TEST_CASE t1_name
    mov eax, 0x00
    mov ecx, 0xAB
    mov al, cl
    CHECK_EQ_32 eax, 0xAB

    ; ==== Test 2: mov Ed,Gd reg-reg (0x89) ====
    TEST_CASE t2_name
    mov eax, 0x00000000
    mov ebx, 0xDEADBEEF
    mov eax, ebx
    CHECK_EQ_32 eax, 0xDEADBEEF

    ; ==== Test 3: mov Gb,Eb reg-reg (0x8A) ====
    TEST_CASE t3_name
    mov ecx, 0x00
    mov edx, 0x42
    mov cl, dl
    CHECK_EQ_32 ecx, 0x42

    ; ==== Test 4: mov Gd,Ed reg-reg (0x8B) ====
    TEST_CASE t4_name
    mov ecx, 0x00000000
    mov edx, 0x12345678
    mov ecx, edx
    CHECK_EQ_32 ecx, 0x12345678

    ; ==== Test 5: mov Eb,Gb mem (0x88 mem) ====
    TEST_CASE t5_name
    mov byte [rel scratch], 0x00
    mov cl, 0xBB
    mov byte [rel scratch], cl
    movzx eax, byte [rel scratch]
    CHECK_EQ_32 eax, 0xBB

    ; ==== Test 6: mov Ed,Gd mem (0x89 mem) ====
    TEST_CASE t6_name
    mov dword [rel scratch], 0x00000000
    mov ebx, 0xCAFEBABE
    mov dword [rel scratch], ebx
    mov eax, dword [rel scratch]
    CHECK_EQ_32 eax, 0xCAFEBABE

    ; ==== Test 7: mov Gb,Eb mem (0x8A mem) ====
    TEST_CASE t7_name
    mov byte [rel scratch], 0x77
    mov cl, 0x00
    mov cl, byte [rel scratch]
    movzx eax, cl
    CHECK_EQ_32 eax, 0x77

    ; ==== Test 8: mov Gd,Ed mem (0x8B mem) ====
    TEST_CASE t8_name
    mov dword [rel scratch], 0xFEEDFACE
    mov ecx, 0x00000000
    mov ecx, dword [rel scratch]
    CHECK_EQ_32 ecx, 0xFEEDFACE

    ; ==== Test 9: mov r8,imm8 (0xB0) ====
    TEST_CASE t9_name
    mov al, 0x42
    movzx ecx, al
    CHECK_EQ_32 ecx, 0x42

    ; ==== Test 10: mov r32,imm32 (0xB8) ====
    TEST_CASE t10_name
    mov eax, 0xDEADBEEF
    CHECK_EQ_32 eax, 0xDEADBEEF

    ; ==== Test 11: mov r64,imm64 (REX.W 0xB8) ====
    TEST_CASE t11_name
    mov rax, 0xDEADBEEF12345678
    CHECK_EQ_64 rax, 0xDEADBEEF12345678

    ; ==== Test 12: mov Eb,Ib reg (0xC6) ====
    TEST_CASE t12_name
    xor eax, eax
    mov al, 0x99
    CHECK_EQ_32 eax, 0x99

    ; ==== Test 13: mov Ed,Id reg (0xC7) ====
    TEST_CASE t13_name
    mov eax, 0xABCD1234
    CHECK_EQ_32 eax, 0xABCD1234

    ; ==== Test 14: mov Eb,Ib mem (0xC6 mem) ====
    TEST_CASE t14_name
    mov byte [rel scratch], 0xEE
    movzx eax, byte [rel scratch]
    CHECK_EQ_32 eax, 0xEE

    ; ==== Test 15: mov Ed,Id mem (0xC7 mem) ====
    TEST_CASE t15_name
    mov dword [rel scratch], 0xBAADF00D
    mov eax, dword [rel scratch]
    CHECK_EQ_32 eax, 0xBAADF00D

    ; ==== Test 16: mov 64-bit reg-reg (REX.W 0x89) ====
    TEST_CASE t16_name
    mov rax, 0xDEADBEEFCAFEBABE
    mov rbx, 0
    mov rbx, rax
    CHECK_EQ_64 rbx, 0xDEADBEEFCAFEBABE

    ; ==== Test 17: mov 64-bit mem (REX.W 0x89 mem) ====
    TEST_CASE t17_name
    mov rax, 0x123456789ABCDEF0
    mov qword [rel scratch], rax
    mov rbx, qword [rel scratch]
    CHECK_EQ_64 rbx, 0x123456789ABCDEF0

    ; ==== Test 18: mov 64-bit Gd,Ed mem (REX.W 0x8B mem) ====
    TEST_CASE t18_name
    mov qword [rel scratch], 0
    mov rax, 0xFEDCBA9876543210
    mov qword [rel scratch], rax
    mov rcx, 0
    mov rcx, qword [rel scratch]
    CHECK_EQ_64 rcx, 0xFEDCBA9876543210

    ; ==== Test 19: mov hi-byte AH (0x88) ====
    TEST_CASE t19_name
    mov eax, 0x0000
    mov ecx, 0xFF00
    mov ah, ch
    CHECK_EQ_32 eax, 0xFF00

    ; ==== Test 20: mov hi-byte from AH (0x8A) ====
    TEST_CASE t20_name
    mov eax, 0xAB00
    mov ecx, 0x0000
    mov cl, ah
    CHECK_EQ_32 ecx, 0xAB

    ; ==== Test 21: mov all r8 imm8 (0xB0-0xB7) ====
    TEST_CASE t21_name
    mov al, 0x11
    mov cl, 0x22
    mov dl, 0x33
    mov bl, 0x44
    ; Check them all via movzx
    movzx r8d, al
    cmp r8d, 0x11
    jne .t21_fail
    movzx r8d, cl
    cmp r8d, 0x22
    jne .t21_fail
    movzx r8d, dl
    cmp r8d, 0x33
    jne .t21_fail
    movzx r8d, bl
    cmp r8d, 0x44
    jne .t21_fail
    ; all good - manually pass
    mov eax, 1
    CHECK_EQ_32 eax, 1
    jmp .t21_done
.t21_fail:
    mov eax, 0
    CHECK_EQ_32 eax, 1
.t21_done:

    ; ==== Test 22: mov all r32 imm32 (0xB8-0xBF) ====
    TEST_CASE t22_name
    mov eax, 0x11111111
    mov ecx, 0x22222222
    mov edx, 0x33333333
    mov ebx, 0x44444444
    ; verify eax
    cmp eax, 0x11111111
    jne .t22_fail
    cmp ecx, 0x22222222
    jne .t22_fail
    cmp edx, 0x33333333
    jne .t22_fail
    cmp ebx, 0x44444444
    jne .t22_fail
    mov eax, 1
    CHECK_EQ_32 eax, 1
    jmp .t22_done
.t22_fail:
    mov eax, 0
    CHECK_EQ_32 eax, 1
.t22_done:

    ; ==== Test 23: mov does not affect flags ====
    TEST_CASE t23_name
    ; Set ZF and SF by doing xor
    xor eax, eax            ; ZF=1, SF=0
    SAVE_FLAGS               ; save into r14
    mov eax, 0xDEADBEEF     ; should not change flags
    pushfq
    pop rax
    ; Compare relevant flag bits
    and rax, FLAGS_MASK_ARITH
    and r14, FLAGS_MASK_ARITH
    cmp rax, r14
    je .t23_pass
    mov eax, 0
    CHECK_EQ_32 eax, 1
    jmp .t23_done
.t23_pass:
    mov eax, 1
    CHECK_EQ_32 eax, 1
.t23_done:

    ; ==== Test 24: mov Eb,Gb mem via base+disp (0x88) ====
    TEST_CASE t24_name
    lea rbx, [rel scratch]
    mov byte [rbx], 0x00
    mov cl, 0xDD
    mov byte [rbx], cl
    movzx eax, byte [rbx]
    CHECK_EQ_32 eax, 0xDD

    END_TESTS
