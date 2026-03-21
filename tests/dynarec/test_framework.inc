; test_framework.inc - NASM test framework for box64 dynarec testing
; Uses raw syscalls only (no libc dependency)
; Include this file at the top of each test .asm file
;
; Usage:
;   %include "test_framework.inc"
;   section .data
;   test_name: db "My Test", 0
;   section .text
;   global _start
;   _start:
;       INIT_TESTS
;       ; ... test code using TEST_CASE, CHECK_EQ_64, etc. ...
;       END_TESTS
;
; Registers reserved by framework:
;   r12 = pass count
;   r13 = total test count
;   r14 = scratch for checks (restored after)
;   r15 = current test name pointer

BITS 64

;; ============================================================
;; Data section additions
;; ============================================================
section .data
    _fw_pass_msg: db "  PASS", 10, 0
    _fw_fail_msg: db "  FAIL", 10, 0
    _fw_expect_msg: db "  expected=0x", 0
    _fw_got_msg: db " got=0x", 0
    _fw_summary_msg: db "Result: ", 0
    _fw_slash: db "/", 0
    _fw_nl: db 10, 0
    _fw_colon_sp: db ": ", 0
    _fw_hex_chars: db "0123456789abcdef"

section .bss
    _fw_hexbuf: resb 20    ; buffer for hex conversion

section .text

;; ============================================================
;; Print a null-terminated string at [rsi]
;; Clobbers: rax, rdi, rdx, rcx
;; ============================================================
_fw_print_str:
    ; count length
    mov rcx, rsi
.len_loop:
    cmp byte [rcx], 0
    je .len_done
    inc rcx
    jmp .len_loop
.len_done:
    sub rcx, rsi
    mov rdx, rcx       ; length
    mov eax, 1          ; sys_write
    mov edi, 1          ; stdout
    syscall
    ret

;; ============================================================
;; Print 64-bit value in rax as hex
;; Clobbers: rax, rcx, rdx, rsi, rdi
;; ============================================================
_fw_print_hex64:
    push rbx
    mov rbx, rax
    lea rsi, [rel _fw_hexbuf]
    mov ecx, 16         ; 16 hex digits
.hex_loop:
    rol rbx, 4
    mov eax, ebx
    and eax, 0xF
    lea rdx, [rel _fw_hex_chars]
    mov al, byte [rdx + rax]
    mov byte [rsi], al
    inc rsi
    dec ecx
    jnz .hex_loop
    mov byte [rsi], 0   ; null terminate
    lea rsi, [rel _fw_hexbuf]
    pop rbx
    call _fw_print_str
    ret

;; ============================================================
;; Print 32-bit value in eax as hex (8 digits)
;; ============================================================
_fw_print_hex32:
    push rbx
    mov ebx, eax
    lea rsi, [rel _fw_hexbuf]
    mov ecx, 8
.hex_loop:
    rol ebx, 4
    mov eax, ebx
    and eax, 0xF
    lea rdx, [rel _fw_hex_chars]
    mov al, byte [rdx + rax]
    mov byte [rsi], al
    inc rsi
    dec ecx
    jnz .hex_loop
    mov byte [rsi], 0
    lea rsi, [rel _fw_hexbuf]
    pop rbx
    call _fw_print_str
    ret

;; ============================================================
;; Print decimal value in eax (up to 999)
;; ============================================================
_fw_print_dec:
    push rbx
    lea rsi, [rel _fw_hexbuf + 10]
    mov byte [rsi], 0
    dec rsi
    mov ebx, eax
    test ebx, ebx
    jnz .dec_loop
    ; handle zero
    mov byte [rsi], '0'
    jmp .dec_done
.dec_loop:
    test ebx, ebx
    jz .dec_done_inc
    xor edx, edx
    mov eax, ebx
    mov ecx, 10
    div ecx
    add edx, '0'
    mov byte [rsi], dl
    dec rsi
    mov ebx, eax
    jmp .dec_loop
.dec_done_inc:
    inc rsi
.dec_done:
    pop rbx
    call _fw_print_str
    ret

;; ============================================================
;; Macros
;; ============================================================

;; Initialize test framework
%macro INIT_TESTS 0
    xor r12d, r12d      ; pass count
    xor r13d, r13d      ; total count
%endmacro

;; Start a named test case. name_label should be a label to a db string.
;; TEST_CASE test_name_label
%macro TEST_CASE 1
    inc r13d
    lea rsi, [rel %1]
    call _fw_print_str
%endmacro

;; CHECK_EQ_64 actual_reg, expected_value
;; Compares reg64 to immediate, prints PASS/FAIL
%macro CHECK_EQ_64 2
    push rax
    push rsi
    mov rax, %2
    cmp %1, rax
    jne %%fail
    ; PASS
    lea rsi, [rel _fw_pass_msg]
    call _fw_print_str
    inc r12d
    jmp %%done
%%fail:
    ; FAIL - show expected/got
    lea rsi, [rel _fw_fail_msg]
    call _fw_print_str
    lea rsi, [rel _fw_expect_msg]
    call _fw_print_str
    mov rax, %2
    call _fw_print_hex64
    lea rsi, [rel _fw_got_msg]
    call _fw_print_str
    mov rax, %1
    call _fw_print_hex64
    lea rsi, [rel _fw_nl]
    call _fw_print_str
%%done:
    pop rsi
    pop rax
%endmacro

;; CHECK_EQ_32 actual_reg, expected_value
;; Compares reg32 to immediate, prints PASS/FAIL
%macro CHECK_EQ_32 2
    push rax
    push rsi
    cmp %1, %2
    jne %%fail
    lea rsi, [rel _fw_pass_msg]
    call _fw_print_str
    inc r12d
    jmp %%done
%%fail:
    lea rsi, [rel _fw_fail_msg]
    call _fw_print_str
    lea rsi, [rel _fw_expect_msg]
    call _fw_print_str
    mov eax, %2
    call _fw_print_hex32
    lea rsi, [rel _fw_got_msg]
    call _fw_print_str
    mov eax, %1
    call _fw_print_hex32
    lea rsi, [rel _fw_nl]
    call _fw_print_str
%%done:
    pop rsi
    pop rax
%endmacro

;; CHECK_EQ_XMM_LOW64 xmm_reg, expected_value
;; Moves low 64 bits of XMM to rax, compares to expected
%macro CHECK_EQ_XMM_LOW64 2
    push rax
    push rsi
    movq rax, %1
    mov rsi, %2
    cmp rax, rsi
    jne %%fail
    lea rsi, [rel _fw_pass_msg]
    call _fw_print_str
    inc r12d
    jmp %%done
%%fail:
    lea rsi, [rel _fw_fail_msg]
    call _fw_print_str
    lea rsi, [rel _fw_expect_msg]
    call _fw_print_str
    mov rax, %2
    call _fw_print_hex64
    lea rsi, [rel _fw_got_msg]
    call _fw_print_str
    movq rax, %1
    call _fw_print_hex64
    lea rsi, [rel _fw_nl]
    call _fw_print_str
%%done:
    pop rsi
    pop rax
%endmacro

;; CHECK_FLAGS expected_flags_mask
;; Saves RFLAGS, ANDs with mask, compares. Call immediately after the instruction
;; that sets flags. The actual flags value is in r14 after this macro.
%macro CHECK_FLAGS 1
    pushfq
    pop r14                  ; r14 = actual flags
    push rax
    push rsi
    mov rax, r14
    and rax, %1             ; mask relevant bits
    mov rsi, %1
    ;; We just check that the flags & mask matches expected
    ;; Actually, this should compare flags&mask == expected
    ;; Let the caller specify the exact expected value instead
    ;; This macro is too simplistic - use CHECK_FLAGS_EQ instead
    pop rsi
    pop rax
%endmacro

;; CHECK_FLAGS_EQ mask, expected_masked_value
;; Gets RFLAGS, ANDs with mask, compares to expected
;; Must be called with RFLAGS still reflecting the instruction under test
;; (i.e., no cmp/test between the instruction and this macro)
%macro SAVE_FLAGS 0
    pushfq
    pop r14
%endmacro

%macro CHECK_FLAGS_EQ 2
    push rax
    push rsi
    push rcx
    mov rax, r14
    and rax, %1             ; mask
    mov rcx, %2             ; expected
    cmp rax, rcx
    jne %%fail
    lea rsi, [rel _fw_pass_msg]
    call _fw_print_str
    inc r12d
    jmp %%done
%%fail:
    lea rsi, [rel _fw_fail_msg]
    call _fw_print_str
    lea rsi, [rel _fw_expect_msg]
    call _fw_print_str
    mov rax, %2
    call _fw_print_hex64
    lea rsi, [rel _fw_got_msg]
    call _fw_print_str
    mov rax, r14
    and rax, %1
    call _fw_print_hex64
    lea rsi, [rel _fw_nl]
    call _fw_print_str
%%done:
    pop rcx
    pop rsi
    pop rax
%endmacro

;; Print summary and exit
%macro END_TESTS 0
    ; Print summary
    lea rsi, [rel _fw_summary_msg]
    call _fw_print_str
    mov eax, r12d
    call _fw_print_dec
    lea rsi, [rel _fw_slash]
    call _fw_print_str
    mov eax, r13d
    call _fw_print_dec
    lea rsi, [rel _fw_nl]
    call _fw_print_str
    ; Exit with 0 if all passed, 1 otherwise
    cmp r12d, r13d
    je %%all_pass
    mov edi, 1
    jmp %%do_exit
%%all_pass:
    xor edi, edi
%%do_exit:
    mov eax, 60         ; sys_exit
    syscall
%endmacro

;; Flag bit constants
%define CF 0x0001
%define PF 0x0004
%define AF 0x0010
%define ZF 0x0040
%define SF 0x0080
%define OF 0x0800
%define FLAGS_MASK_ARITH (CF|PF|AF|ZF|SF|OF)
%define FLAGS_MASK_LOGIC (CF|PF|ZF|SF|OF)
