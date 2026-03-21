; test_and_xor.asm - Test AND (0x20-0x25) and XOR (0x30-0x35) opcodes with flags
%include "test_framework.inc"

section .data
    ; AND tests
    t1_name:  db "and Eb,Gb reg-reg (0x20)", 0
    t2_name:  db "and Ed,Gd reg-reg (0x21)", 0
    t3_name:  db "and Gb,Eb reg-reg (0x22)", 0
    t4_name:  db "and Gd,Ed reg-reg (0x23)", 0
    t5_name:  db "and AL,Ib (0x24)", 0
    t6_name:  db "and EAX,Id (0x25)", 0
    t7_name:  db "and Ed,Gd mem (0x21 mem)", 0
    t8_name:  db "and Gd,Ed mem (0x23 mem)", 0
    t9_name:  db "and flags ZF (result=0)", 0
    t10_name: db "and flags SF (negative)", 0
    t11_name: db "and clears CF/OF", 0
    t12_name: db "and Eb,Gb mem (0x20 mem)", 0
    t13_name: db "and 64-bit (REX.W)", 0

    ; XOR tests
    t20_name: db "xor Eb,Gb reg-reg (0x30)", 0
    t21_name: db "xor Ed,Gd reg-reg (0x31)", 0
    t22_name: db "xor Gb,Eb reg-reg (0x32)", 0
    t23_name: db "xor Gd,Ed reg-reg (0x33)", 0
    t24_name: db "xor AL,Ib (0x34)", 0
    t25_name: db "xor EAX,Id (0x35)", 0
    t26_name: db "xor Ed,Gd mem (0x31 mem)", 0
    t27_name: db "xor Gd,Ed mem (0x33 mem)", 0
    t28_name: db "xor self ZF (reg^reg=0)", 0
    t29_name: db "xor flags SF (negative)", 0
    t30_name: db "xor clears CF/OF", 0
    t31_name: db "xor Eb,Gb mem (0x30 mem)", 0
    t32_name: db "xor 64-bit (REX.W)", 0
    t33_name: db "and Gb,Eb mem (0x22 mem)", 0
    t34_name: db "xor Gb,Eb mem (0x32 mem)", 0
    t35_name: db "and hi-byte reg AH (0x20)", 0
    t36_name: db "xor hi-byte reg AH (0x30)", 0

section .bss
    scratch: resq 1

section .text
global _start

_start:
    INIT_TESTS

    ;; ============================================================
    ;; AND tests
    ;; ============================================================

    ; ==== Test 1: and Eb,Gb reg-reg (0x20) ====
    TEST_CASE t1_name
    mov eax, 0xFF
    mov ecx, 0x0F
    and al, cl                  ; 0xFF & 0x0F = 0x0F
    CHECK_EQ_32 eax, 0x0F

    ; ==== Test 2: and Ed,Gd reg-reg (0x21) ====
    TEST_CASE t2_name
    mov eax, 0xDEADBEEF
    mov ebx, 0xFF00FF00
    and eax, ebx                ; 0xDEADBEEF & 0xFF00FF00 = 0xDE00BE00
    CHECK_EQ_32 eax, 0xDE00BE00

    ; ==== Test 3: and Gb,Eb reg-reg (0x22) ====
    TEST_CASE t3_name
    mov ecx, 0xAB
    mov edx, 0x0F
    and dl, cl                  ; 0x0F & 0xAB = 0x0B
    CHECK_EQ_32 edx, 0x0B

    ; ==== Test 4: and Gd,Ed reg-reg (0x23) ====
    TEST_CASE t4_name
    mov eax, 0x12345678
    mov ecx, 0xFF00FFFF
    and ecx, eax                ; 0xFF00FFFF & 0x12345678 = 0x12005678
    CHECK_EQ_32 ecx, 0x12005678

    ; ==== Test 5: and AL,Ib (0x24) ====
    TEST_CASE t5_name
    mov eax, 0xAB
    and al, 0x0F                ; 0xAB & 0x0F = 0x0B
    CHECK_EQ_32 eax, 0x0B

    ; ==== Test 6: and EAX,Id (0x25) ====
    TEST_CASE t6_name
    mov eax, 0xDEADBEEF
    and eax, 0x0000FFFF         ; 0xDEADBEEF & 0x0000FFFF = 0x0000BEEF
    CHECK_EQ_32 eax, 0x0000BEEF

    ; ==== Test 7: and Ed,Gd mem (0x21 mem) ====
    TEST_CASE t7_name
    mov dword [rel scratch], 0xFFFF0000
    mov ebx, 0x12345678
    and dword [rel scratch], ebx ; 0xFFFF0000 & 0x12345678 = 0x12340000
    mov eax, dword [rel scratch]
    CHECK_EQ_32 eax, 0x12340000

    ; ==== Test 8: and Gd,Ed mem (0x23 mem) ====
    TEST_CASE t8_name
    mov dword [rel scratch], 0x00FF00FF
    mov ecx, 0xABCDEF01
    and ecx, dword [rel scratch] ; 0xABCDEF01 & 0x00FF00FF = 0x00CD0001
    CHECK_EQ_32 ecx, 0x00CD0001

    ; ==== Test 9: and flags ZF (result=0) ====
    TEST_CASE t9_name
    mov eax, 0xFF00
    and eax, 0x00FF             ; result = 0 => ZF set
    SAVE_FLAGS
    CHECK_FLAGS_EQ ZF, ZF

    ; ==== Test 10: and flags SF (negative) ====
    TEST_CASE t10_name
    mov eax, 0xFFFFFFFF
    and eax, 0x80000000         ; result = 0x80000000 => SF set
    SAVE_FLAGS
    CHECK_FLAGS_EQ SF, SF

    ; ==== Test 11: and clears CF/OF ====
    TEST_CASE t11_name
    ; First set CF via add overflow
    mov eax, 0xFFFFFFFF
    add eax, 1                  ; sets CF
    ; Now AND should clear CF and OF
    mov eax, 0xFF
    and eax, 0xFF
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|OF), 0

    ; ==== Test 12: and Eb,Gb mem (0x20 mem) ====
    TEST_CASE t12_name
    mov byte [rel scratch], 0xAB
    mov cl, 0x0F
    and byte [rel scratch], cl  ; 0xAB & 0x0F = 0x0B
    movzx eax, byte [rel scratch]
    CHECK_EQ_32 eax, 0x0B

    ; ==== Test 13: and 64-bit (REX.W) ====
    TEST_CASE t13_name
    mov rax, 0xDEADBEEF12345678
    mov rbx, 0xFFFFFFFF00000000
    and rax, rbx                ; 0xDEADBEEF00000000
    CHECK_EQ_64 rax, 0xDEADBEEF00000000

    ;; ============================================================
    ;; XOR tests
    ;; ============================================================

    ; ==== Test 20: xor Eb,Gb reg-reg (0x30) ====
    TEST_CASE t20_name
    mov eax, 0xFF
    mov ecx, 0x0F
    xor al, cl                  ; 0xFF ^ 0x0F = 0xF0
    CHECK_EQ_32 eax, 0xF0

    ; ==== Test 21: xor Ed,Gd reg-reg (0x31) ====
    TEST_CASE t21_name
    mov eax, 0xDEADBEEF
    mov ebx, 0xFF00FF00
    xor eax, ebx                ; 0xDEADBEEF ^ 0xFF00FF00 = 0x21AD41EF
    CHECK_EQ_32 eax, 0x21AD41EF

    ; ==== Test 22: xor Gb,Eb reg-reg (0x32) ====
    TEST_CASE t22_name
    mov ecx, 0xAB
    mov edx, 0xFF
    xor dl, cl                  ; 0xFF ^ 0xAB = 0x54
    CHECK_EQ_32 edx, 0x54

    ; ==== Test 23: xor Gd,Ed reg-reg (0x33) ====
    TEST_CASE t23_name
    mov eax, 0x12345678
    mov ecx, 0xFFFFFFFF
    xor ecx, eax                ; 0xFFFFFFFF ^ 0x12345678 = 0xEDCBA987
    CHECK_EQ_32 ecx, 0xEDCBA987

    ; ==== Test 24: xor AL,Ib (0x34) ====
    TEST_CASE t24_name
    mov eax, 0xAA
    xor al, 0x55                ; 0xAA ^ 0x55 = 0xFF
    CHECK_EQ_32 eax, 0xFF

    ; ==== Test 25: xor EAX,Id (0x35) ====
    TEST_CASE t25_name
    mov eax, 0xAAAAAAAA
    xor eax, 0x55555555         ; 0xAAAAAAAA ^ 0x55555555 = 0xFFFFFFFF
    CHECK_EQ_32 eax, 0xFFFFFFFF

    ; ==== Test 26: xor Ed,Gd mem (0x31 mem) ====
    TEST_CASE t26_name
    mov dword [rel scratch], 0xFFFF0000
    mov ebx, 0x12345678
    xor dword [rel scratch], ebx ; 0xFFFF0000 ^ 0x12345678 = 0xEDCB5678
    mov eax, dword [rel scratch]
    CHECK_EQ_32 eax, 0xEDCB5678

    ; ==== Test 27: xor Gd,Ed mem (0x33 mem) ====
    TEST_CASE t27_name
    mov dword [rel scratch], 0x00FF00FF
    mov ecx, 0xABCDEF01
    xor ecx, dword [rel scratch] ; 0xABCDEF01 ^ 0x00FF00FF = 0xAB32EFFE
    CHECK_EQ_32 ecx, 0xAB32EFFE

    ; ==== Test 28: xor self ZF (reg^reg=0) ====
    TEST_CASE t28_name
    mov eax, 0xDEADBEEF
    xor eax, eax                ; always 0 => ZF set
    SAVE_FLAGS
    CHECK_FLAGS_EQ ZF, ZF

    ; ==== Test 29: xor flags SF (negative) ====
    TEST_CASE t29_name
    mov eax, 0x00000001
    xor eax, 0x80000001         ; result = 0x80000000 => SF set
    SAVE_FLAGS
    CHECK_FLAGS_EQ SF, SF

    ; ==== Test 30: xor clears CF/OF ====
    TEST_CASE t30_name
    ; First set CF
    mov eax, 0xFFFFFFFF
    add eax, 1                  ; sets CF
    ; XOR should clear CF and OF
    mov eax, 0xFF
    xor eax, 0x0F
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|OF), 0

    ; ==== Test 31: xor Eb,Gb mem (0x30 mem) ====
    TEST_CASE t31_name
    mov byte [rel scratch], 0xAB
    mov cl, 0xFF
    xor byte [rel scratch], cl  ; 0xAB ^ 0xFF = 0x54
    movzx eax, byte [rel scratch]
    CHECK_EQ_32 eax, 0x54

    ; ==== Test 32: xor 64-bit (REX.W) ====
    TEST_CASE t32_name
    mov rax, 0xDEADBEEF12345678
    mov rbx, 0xFFFFFFFFFFFFFFFF
    xor rax, rbx                ; bitwise NOT
    CHECK_EQ_64 rax, 0x2152411FEDCBA987

    ; ==== Test 33: and Gb,Eb mem (0x22 mem) ====
    TEST_CASE t33_name
    mov byte [rel scratch], 0xF0
    mov cl, 0xAB
    and cl, byte [rel scratch]  ; 0xAB & 0xF0 = 0xA0
    movzx eax, cl
    CHECK_EQ_32 eax, 0xA0

    ; ==== Test 34: xor Gb,Eb mem (0x32 mem) ====
    TEST_CASE t34_name
    mov byte [rel scratch], 0xF0
    mov cl, 0xAB
    xor cl, byte [rel scratch]  ; 0xAB ^ 0xF0 = 0x5B
    movzx eax, cl
    CHECK_EQ_32 eax, 0x5B

    ; ==== Test 35: and hi-byte reg AH (0x20) ====
    TEST_CASE t35_name
    mov eax, 0xFF00             ; AH = 0xFF
    mov ecx, 0x0F00             ; CH = 0x0F
    and ah, ch                  ; AH = 0xFF & 0x0F = 0x0F
    CHECK_EQ_32 eax, 0x0F00

    ; ==== Test 36: xor hi-byte reg AH (0x30) ====
    TEST_CASE t36_name
    mov eax, 0xAA00             ; AH = 0xAA
    mov ecx, 0x5500             ; CH = 0x55
    xor ah, ch                  ; AH = 0xAA ^ 0x55 = 0xFF
    CHECK_EQ_32 eax, 0xFF00

    END_TESTS
