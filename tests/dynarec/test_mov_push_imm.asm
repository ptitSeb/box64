; test_mov_push_imm.asm - Test MOV/PUSH immediate opcodes and 16-bit ADD/SUB
; Regression tests for PPC64LE dynarec useless-if removal and xZR bug fixes:
;   0x68  PUSH Id          - zero and nonzero immediates
;   0x6A  PUSH Ib          - zero and nonzero immediates
;   0xC6  MOV Eb, Ib (mem) - zero and nonzero byte immediates
;   0xC7  MOV Ed, Id (mem) - zero and nonzero dword immediates
;   66 05 ADD AX, Iw       - zero immediate (was using xZR = r0, a bug)
;   66 2D SUB AX, Iw       - zero immediate (was using xZR = r0, a bug)
;   66 68 PUSH Iw          - zero and nonzero immediates
;   66 6A PUSH Ib (16-bit) - zero and nonzero immediates
%include "test_framework.inc"

section .data
    t1_name:  db "push 0 (68h)", 0
    t2_name:  db "push 0x12345678 (68h)", 0
    t3_name:  db "push -1 (68h)", 0
    t4_name:  db "push 0 (6Ah)", 0
    t5_name:  db "push 0x7F (6Ah)", 0
    t6_name:  db "push -128 (6Ah)", 0
    t7_name:  db "mov byte [mem],0 (C6h)", 0
    t8_name:  db "mov byte [mem],0x42 (C6h)", 0
    t9_name:  db "mov byte [mem],0xFF (C6h)", 0
    t10_name: db "mov dword [mem],0 (C7h)", 0
    t11_name: db "mov dword [mem],0xDEAD (C7h)", 0
    t12_name: db "mov dword [mem],-1 (C7h)", 0
    t13_name: db "mov qword [mem],0 (REX.W C7h)", 0
    t14_name: db "mov qword [mem],-1 (REX.W C7h)", 0
    t15_name: db "add ax,0 (66 05h)", 0
    t16_name: db "add ax,0x1234 (66 05h)", 0
    t17_name: db "add ax,0 flags (66 05h)", 0
    t18_name: db "sub ax,0 (66 2Dh)", 0
    t19_name: db "sub ax,0x1000 (66 2Dh)", 0
    t20_name: db "sub ax,0 flags (66 2Dh)", 0
    t21_name: db "push word 0 (66 68h)", 0
    t22_name: db "push word 0x1234 (66 68h)", 0
    t23_name: db "push word 0 (66 6Ah)", 0
    t24_name: db "push word 0x7F (66 6Ah)", 0
    t25_name: db "add ax,0 preserves val", 0
    t26_name: db "sub ax,0 preserves val", 0
    t27_name: db "mov byte [mem],0 sequence", 0
    t28_name: db "mov dword [mem],0 seq lo", 0
    t29_name: db "mov dword [mem],0 seq hi", 0

    align 8
    mem_byte:   dq 0xCCCCCCCCCCCCCCCC
    mem_dword:  dq 0xCCCCCCCCCCCCCCCC
    mem_qword:  dq 0xCCCCCCCCCCCCCCCC
    mem_seq_b:  times 4 db 0xCC
    mem_seq_d:  times 2 dd 0xCCCCCCCC

section .text
global _start

_start:
    INIT_TESTS

    ; ==== Test 1: PUSH 0 (opcode 0x68) ====
    TEST_CASE t1_name
    push qword 0
    pop rax
    CHECK_EQ_64 rax, 0

    ; ==== Test 2: PUSH 0x12345678 (opcode 0x68) ====
    TEST_CASE t2_name
    push qword 0x12345678
    pop rax
    CHECK_EQ_64 rax, 0x12345678

    ; ==== Test 3: PUSH -1 (opcode 0x68, sign-extended) ====
    TEST_CASE t3_name
    push qword -1
    pop rax
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFF

    ; ==== Test 4: PUSH 0 (opcode 0x6A) ====
    TEST_CASE t4_name
    push byte 0
    pop rax
    CHECK_EQ_64 rax, 0

    ; ==== Test 5: PUSH 0x7F (opcode 0x6A) ====
    TEST_CASE t5_name
    push byte 0x7F
    pop rax
    CHECK_EQ_64 rax, 0x7F

    ; ==== Test 6: PUSH -128 (opcode 0x6A, sign-extended) ====
    TEST_CASE t6_name
    push byte -128
    pop rax
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFF80

    ; ==== Test 7: MOV byte [mem], 0 (opcode 0xC6, mem path) ====
    TEST_CASE t7_name
    lea rbx, [rel mem_byte]
    mov rax, 0xCCCCCCCCCCCCCCCC
    mov qword [rbx], rax   ; reset
    mov byte [rbx], 0
    movzx eax, byte [rbx]
    CHECK_EQ_32 eax, 0

    ; ==== Test 8: MOV byte [mem], 0x42 (opcode 0xC6) ====
    TEST_CASE t8_name
    lea rbx, [rel mem_byte]
    mov byte [rbx], 0x42
    movzx eax, byte [rbx]
    CHECK_EQ_32 eax, 0x42

    ; ==== Test 9: MOV byte [mem], 0xFF (opcode 0xC6) ====
    TEST_CASE t9_name
    lea rbx, [rel mem_byte]
    mov byte [rbx], 0xFF
    movzx eax, byte [rbx]
    CHECK_EQ_32 eax, 0xFF

    ; ==== Test 10: MOV dword [mem], 0 (opcode 0xC7, mem path) ====
    TEST_CASE t10_name
    lea rbx, [rel mem_dword]
    mov rax, 0xCCCCCCCCCCCCCCCC
    mov qword [rbx], rax   ; reset
    mov dword [rbx], 0
    mov eax, [rbx]
    CHECK_EQ_32 eax, 0

    ; ==== Test 11: MOV dword [mem], 0xDEAD (opcode 0xC7) ====
    TEST_CASE t11_name
    lea rbx, [rel mem_dword]
    mov dword [rbx], 0xDEAD
    mov eax, [rbx]
    CHECK_EQ_32 eax, 0xDEAD

    ; ==== Test 12: MOV dword [mem], -1 (opcode 0xC7) ====
    TEST_CASE t12_name
    lea rbx, [rel mem_dword]
    mov dword [rbx], -1
    mov eax, [rbx]
    CHECK_EQ_32 eax, 0xFFFFFFFF

    ; ==== Test 13: MOV qword [mem], 0 (REX.W + 0xC7) ====
    TEST_CASE t13_name
    lea rbx, [rel mem_qword]
    mov rax, 0xCCCCCCCCCCCCCCCC
    mov qword [rbx], rax   ; reset
    mov qword [rbx], 0
    mov rax, [rbx]
    CHECK_EQ_64 rax, 0

    ; ==== Test 14: MOV qword [mem], -1 (REX.W + 0xC7, sign-extended i32) ====
    TEST_CASE t14_name
    lea rbx, [rel mem_qword]
    mov qword [rbx], -1
    mov rax, [rbx]
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFF

    ; ==== Test 15: ADD AX, 0 (66 05, zero immediate - was xZR bug) ====
    TEST_CASE t15_name
    mov eax, 0x00005678
    db 0x66, 0x05        ; ADD AX, Iw
    dw 0x0000            ; immediate = 0
    ; AX should remain 0x5678, upper bits preserved
    CHECK_EQ_32 eax, 0x00005678

    ; ==== Test 16: ADD AX, 0x1234 (66 05) ====
    TEST_CASE t16_name
    mov eax, 0x00001000
    db 0x66, 0x05
    dw 0x1234
    ; AX = 0x1000 + 0x1234 = 0x2234
    CHECK_EQ_32 eax, 0x00002234

    ; ==== Test 17: ADD AX, 0 flags (66 05, flags must match value) ====
    ; ADD AX, 0 where AX = 0x8000 should set SF, clear ZF
    TEST_CASE t17_name
    mov eax, 0x00008000
    db 0x66, 0x05
    dw 0x0000
    SAVE_FLAGS
    CHECK_FLAGS_EQ SF, SF

    ; ==== Test 18: SUB AX, 0 (66 2D, zero immediate - was xZR bug) ====
    TEST_CASE t18_name
    mov eax, 0x0000ABCD
    db 0x66, 0x2D        ; SUB AX, Iw
    dw 0x0000            ; immediate = 0
    ; AX should remain 0xABCD
    CHECK_EQ_32 eax, 0x0000ABCD

    ; ==== Test 19: SUB AX, 0x1000 (66 2D) ====
    TEST_CASE t19_name
    mov eax, 0x00005000
    db 0x66, 0x2D
    dw 0x1000
    ; AX = 0x5000 - 0x1000 = 0x4000
    CHECK_EQ_32 eax, 0x00004000

    ; ==== Test 20: SUB AX, 0 flags (66 2D, flags must match value) ====
    ; SUB AX, 0 where AX = 0 should set ZF
    TEST_CASE t20_name
    xor eax, eax
    db 0x66, 0x2D
    dw 0x0000
    SAVE_FLAGS
    CHECK_FLAGS_EQ ZF, ZF

    ; ==== Test 21: PUSH word 0 (66 68) ====
    TEST_CASE t21_name
    ; Push 0 as 16-bit, verify
    db 0x66, 0x68        ; PUSH Iw
    dw 0x0000
    pop rax              ; pops 8 bytes but only low 2 were the push
    and eax, 0xFFFF
    CHECK_EQ_32 eax, 0

    ; ==== Test 22: PUSH word 0x1234 (66 68) ====
    TEST_CASE t22_name
    db 0x66, 0x68
    dw 0x1234
    pop rax
    and eax, 0xFFFF
    CHECK_EQ_32 eax, 0x1234

    ; ==== Test 23: PUSH word 0 (66 6A) ====
    TEST_CASE t23_name
    db 0x66, 0x6A        ; PUSH Ib (16-bit)
    db 0x00
    pop rax
    and eax, 0xFFFF
    CHECK_EQ_32 eax, 0

    ; ==== Test 24: PUSH word 0x7F (66 6A) ====
    TEST_CASE t24_name
    db 0x66, 0x6A
    db 0x7F
    pop rax
    and eax, 0xFFFF
    CHECK_EQ_32 eax, 0x007F

    ; ==== Test 25: ADD AX, 0 preserves value with various AX values ====
    ; This specifically catches the xZR bug: if r0 contains garbage,
    ; ADD AX, 0 would corrupt AX
    TEST_CASE t25_name
    mov eax, 0x0000BEEF
    mov ecx, eax         ; save
    db 0x66, 0x05
    dw 0x0000
    cmp ax, cx
    je .t25_pass
    ; If we get here, AX was corrupted
    CHECK_EQ_32 eax, 0x0000BEEF   ; will fail and show actual value
    jmp .t25_done
.t25_pass:
    CHECK_EQ_32 eax, 0x0000BEEF
.t25_done:

    ; ==== Test 26: SUB AX, 0 preserves value with various AX values ====
    TEST_CASE t26_name
    mov eax, 0x0000CAFE
    mov ecx, eax
    db 0x66, 0x2D
    dw 0x0000
    cmp ax, cx
    je .t26_pass
    CHECK_EQ_32 eax, 0x0000CAFE
    jmp .t26_done
.t26_pass:
    CHECK_EQ_32 eax, 0x0000CAFE
.t26_done:

    ; ==== Test 27: MOV byte [mem], 0 then nonzero sequence ====
    ; Ensures mem path works correctly in sequence
    TEST_CASE t27_name
    lea rbx, [rel mem_seq_b]
    mov byte [rbx], 0        ; C6: imm=0
    mov byte [rbx+1], 0xAA   ; C6: imm=0xAA
    mov byte [rbx+2], 0      ; C6: imm=0
    mov byte [rbx+3], 0xBB   ; C6: imm=0xBB
    mov eax, [rbx]
    CHECK_EQ_32 eax, 0xBB00AA00

    ; ==== Test 28: MOV dword [mem], 0 then nonzero sequence (low) ====
    TEST_CASE t28_name
    lea rbx, [rel mem_seq_d]
    mov dword [rbx], 0          ; C7: imm=0
    mov dword [rbx+4], 0x1234   ; C7: imm=0x1234
    mov eax, [rbx]
    CHECK_EQ_32 eax, 0

    ; ==== Test 29: MOV dword [mem], 0 then nonzero sequence (high) ====
    TEST_CASE t29_name
    lea rbx, [rel mem_seq_d]
    mov eax, [rbx+4]
    CHECK_EQ_32 eax, 0x1234

    END_TESTS
