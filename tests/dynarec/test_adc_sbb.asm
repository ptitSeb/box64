; test_adc_sbb.asm - Test ADC (0x10-0x15) and SBB (0x18-0x1D) all forms
; ADC/SBB consume CF as input, so we must set/clear CF explicitly before each test.
%include "test_framework.inc"

section .data
    ; ADC register tests
    t1_name:  db "adc Eb,Gb reg CF=0 (0x10)", 0
    t2_name:  db "adc Eb,Gb reg CF=1 (0x10)", 0
    t3_name:  db "adc Ed,Gd reg CF=0 (0x11)", 0
    t4_name:  db "adc Ed,Gd reg CF=1 (0x11)", 0
    t5_name:  db "adc Gb,Eb reg CF=0 (0x12)", 0
    t6_name:  db "adc Gb,Eb reg CF=1 (0x12)", 0
    t7_name:  db "adc Gd,Ed reg CF=0 (0x13)", 0
    t8_name:  db "adc Gd,Ed reg CF=1 (0x13)", 0
    t9_name:  db "adc AL,Ib CF=0 (0x14)", 0
    t10_name: db "adc AL,Ib CF=1 (0x14)", 0
    t11_name: db "adc EAX,Id CF=0 (0x15)", 0
    t12_name: db "adc EAX,Id CF=1 (0x15)", 0

    ; ADC memory tests
    t13_name: db "adc Eb,Gb mem CF=1 (0x10 mem)", 0
    t14_name: db "adc Ed,Gd mem CF=1 (0x11 mem)", 0
    t15_name: db "adc Gb,Eb mem CF=1 (0x12 mem)", 0
    t16_name: db "adc Gd,Ed mem CF=1 (0x13 mem)", 0

    ; ADC 64-bit and special
    t17_name: db "adc 64-bit CF=1 (REX.W 0x11)", 0
    t18_name: db "adc hi-byte AH CF=1 (0x10)", 0

    ; SBB register tests
    t20_name: db "sbb Eb,Gb reg CF=0 (0x18)", 0
    t21_name: db "sbb Eb,Gb reg CF=1 (0x18)", 0
    t22_name: db "sbb Ed,Gd reg CF=0 (0x19)", 0
    t23_name: db "sbb Ed,Gd reg CF=1 (0x19)", 0
    t24_name: db "sbb Gb,Eb reg CF=0 (0x1A)", 0
    t25_name: db "sbb Gb,Eb reg CF=1 (0x1A)", 0
    t26_name: db "sbb Gd,Ed reg CF=0 (0x1B)", 0
    t27_name: db "sbb Gd,Ed reg CF=1 (0x1B)", 0
    t28_name: db "sbb AL,Ib CF=0 (0x1C)", 0
    t29_name: db "sbb AL,Ib CF=1 (0x1C)", 0
    t30_name: db "sbb EAX,Id CF=0 (0x1D)", 0
    t31_name: db "sbb EAX,Id CF=1 (0x1D)", 0

    ; SBB memory tests
    t32_name: db "sbb Eb,Gb mem CF=1 (0x18 mem)", 0
    t33_name: db "sbb Ed,Gd mem CF=1 (0x19 mem)", 0
    t34_name: db "sbb Gb,Eb mem CF=1 (0x1A mem)", 0
    t35_name: db "sbb Gd,Ed mem CF=1 (0x1B mem)", 0

    ; SBB 64-bit and special
    t36_name: db "sbb 64-bit CF=1 (REX.W 0x19)", 0
    t37_name: db "sbb hi-byte AH CF=1 (0x18)", 0

    ; Flag tests
    t40_name: db "adc CF=1 sets CF on 8-bit overflow", 0
    t41_name: db "adc CF=1 sets ZF on zero result", 0
    t42_name: db "sbb CF=1 sets CF on borrow", 0
    t43_name: db "sbb CF=0 clears CF no borrow", 0
    t44_name: db "adc CF=1 sets OF on signed overflow", 0
    t45_name: db "sbb CF=1 sets OF on signed overflow", 0

section .bss
    scratch: resq 1

section .text
global _start

_start:
    INIT_TESTS

    ;; ===== ADC register tests =====

    ; --- t1: adc Eb,Gb reg CF=0 (0x10) ---
    ; 0x30 + 0x12 + CF(0) = 0x42
    TEST_CASE t1_name
    clc
    mov al, 0x30
    mov bl, 0x12
    adc al, bl
    SAVE_FLAGS
    movzx r8, al
    CHECK_EQ_64 r8, 0x42

    ; --- t2: adc Eb,Gb reg CF=1 (0x10) ---
    ; 0x30 + 0x12 + CF(1) = 0x43
    TEST_CASE t2_name
    stc
    mov al, 0x30
    mov bl, 0x12
    adc al, bl
    SAVE_FLAGS
    movzx r8, al
    CHECK_EQ_64 r8, 0x43

    ; --- t3: adc Ed,Gd reg CF=0 (0x11) ---
    ; 0x10000 + 0x20000 + CF(0) = 0x30000
    TEST_CASE t3_name
    clc
    mov eax, 0x10000
    mov ebx, 0x20000
    adc eax, ebx
    SAVE_FLAGS
    CHECK_EQ_64 rax, 0x30000

    ; --- t4: adc Ed,Gd reg CF=1 (0x11) ---
    ; 0x10000 + 0x20000 + CF(1) = 0x30001
    TEST_CASE t4_name
    stc
    mov eax, 0x10000
    mov ebx, 0x20000
    adc eax, ebx
    SAVE_FLAGS
    CHECK_EQ_64 rax, 0x30001

    ; --- t5: adc Gb,Eb reg CF=0 (0x12) ---
    ; 0x55 + 0x11 + CF(0) = 0x66
    TEST_CASE t5_name
    clc
    mov bl, 0x55
    mov cl, 0x11
    adc bl, cl
    SAVE_FLAGS
    movzx r8, bl
    CHECK_EQ_64 r8, 0x66

    ; --- t6: adc Gb,Eb reg CF=1 (0x12) ---
    ; 0x55 + 0x11 + CF(1) = 0x67
    TEST_CASE t6_name
    stc
    mov bl, 0x55
    mov cl, 0x11
    adc bl, cl
    SAVE_FLAGS
    movzx r8, bl
    CHECK_EQ_64 r8, 0x67

    ; --- t7: adc Gd,Ed reg CF=0 (0x13) ---
    ; 0xAABB + 0x1100 + CF(0) = 0xBBBB
    TEST_CASE t7_name
    clc
    mov eax, 0xAABB
    mov ecx, 0x1100
    adc eax, ecx
    SAVE_FLAGS
    CHECK_EQ_64 rax, 0xBBBB

    ; --- t8: adc Gd,Ed reg CF=1 (0x13) ---
    ; 0xAABB + 0x1100 + CF(1) = 0xBBBC
    TEST_CASE t8_name
    stc
    mov eax, 0xAABB
    mov ecx, 0x1100
    adc eax, ecx
    SAVE_FLAGS
    CHECK_EQ_64 rax, 0xBBBC

    ; --- t9: adc AL,Ib CF=0 (0x14) ---
    ; 0x10 + 0x20 + CF(0) = 0x30
    TEST_CASE t9_name
    clc
    mov al, 0x10
    adc al, 0x20
    SAVE_FLAGS
    movzx r8, al
    CHECK_EQ_64 r8, 0x30

    ; --- t10: adc AL,Ib CF=1 (0x14) ---
    ; 0x10 + 0x20 + CF(1) = 0x31
    TEST_CASE t10_name
    stc
    mov al, 0x10
    adc al, 0x20
    SAVE_FLAGS
    movzx r8, al
    CHECK_EQ_64 r8, 0x31

    ; --- t11: adc EAX,Id CF=0 (0x15) ---
    ; 0x100000 + 0x200000 + CF(0) = 0x300000
    TEST_CASE t11_name
    clc
    mov eax, 0x100000
    adc eax, 0x200000
    SAVE_FLAGS
    CHECK_EQ_64 rax, 0x300000

    ; --- t12: adc EAX,Id CF=1 (0x15) ---
    ; 0x100000 + 0x200000 + CF(1) = 0x300001
    TEST_CASE t12_name
    stc
    mov eax, 0x100000
    adc eax, 0x200000
    SAVE_FLAGS
    CHECK_EQ_64 rax, 0x300001

    ;; ===== ADC memory tests =====

    ; --- t13: adc Eb,Gb mem CF=1 (0x10 mem) ---
    ; [scratch]=0x40 + bl=0x05 + CF(1) = 0x46
    TEST_CASE t13_name
    mov byte [rel scratch], 0x40
    mov bl, 0x05
    stc
    adc byte [rel scratch], bl
    SAVE_FLAGS
    movzx r8, byte [rel scratch]
    CHECK_EQ_64 r8, 0x46

    ; --- t14: adc Ed,Gd mem CF=1 (0x11 mem) ---
    ; [scratch]=0x1000 + ebx=0x2000 + CF(1) = 0x3001
    TEST_CASE t14_name
    mov dword [rel scratch], 0x1000
    mov ebx, 0x2000
    stc
    adc dword [rel scratch], ebx
    SAVE_FLAGS
    mov r8d, dword [rel scratch]
    CHECK_EQ_64 r8, 0x3001

    ; --- t15: adc Gb,Eb mem CF=1 (0x12 mem) ---
    ; bl=0x30 + [scratch]=0x0F + CF(1) = 0x40
    TEST_CASE t15_name
    mov byte [rel scratch], 0x0F
    mov bl, 0x30
    stc
    adc bl, byte [rel scratch]
    SAVE_FLAGS
    movzx r8, bl
    CHECK_EQ_64 r8, 0x40

    ; --- t16: adc Gd,Ed mem CF=1 (0x13 mem) ---
    ; eax=0x5000 + [scratch]=0x3000 + CF(1) = 0x8001
    TEST_CASE t16_name
    mov dword [rel scratch], 0x3000
    mov eax, 0x5000
    stc
    adc eax, dword [rel scratch]
    SAVE_FLAGS
    CHECK_EQ_64 rax, 0x8001

    ; --- t17: adc 64-bit CF=1 (REX.W 0x11) ---
    ; 0x100000000 + 0x200000000 + CF(1) = 0x300000001
    TEST_CASE t17_name
    stc
    mov rax, 0x100000000
    mov rbx, 0x200000000
    adc rax, rbx
    SAVE_FLAGS
    CHECK_EQ_64 rax, 0x300000001

    ; --- t18: adc hi-byte AH CF=1 (0x10) ---
    ; AH=0x30 + BH=0x11 + CF(1) = 0x42 in AH
    TEST_CASE t18_name
    stc
    mov ax, 0x3055       ; AH=0x30, AL=0x55
    mov bx, 0x1100       ; BH=0x11
    adc ah, bh
    ; Save AH before SAVE_FLAGS (can't mix hi-byte regs with REX)
    movzx ecx, ah
    SAVE_FLAGS
    movzx r8, cl
    CHECK_EQ_64 r8, 0x42

    ;; ===== SBB register tests =====

    ; --- t20: sbb Eb,Gb reg CF=0 (0x18) ---
    ; 0x50 - 0x12 - CF(0) = 0x3E
    TEST_CASE t20_name
    clc
    mov al, 0x50
    mov bl, 0x12
    sbb al, bl
    SAVE_FLAGS
    movzx r8, al
    CHECK_EQ_64 r8, 0x3E

    ; --- t21: sbb Eb,Gb reg CF=1 (0x18) ---
    ; 0x50 - 0x12 - CF(1) = 0x3D
    TEST_CASE t21_name
    stc
    mov al, 0x50
    mov bl, 0x12
    sbb al, bl
    SAVE_FLAGS
    movzx r8, al
    CHECK_EQ_64 r8, 0x3D

    ; --- t22: sbb Ed,Gd reg CF=0 (0x19) ---
    ; 0x50000 - 0x20000 - CF(0) = 0x30000
    TEST_CASE t22_name
    clc
    mov eax, 0x50000
    mov ebx, 0x20000
    sbb eax, ebx
    SAVE_FLAGS
    CHECK_EQ_64 rax, 0x30000

    ; --- t23: sbb Ed,Gd reg CF=1 (0x19) ---
    ; 0x50000 - 0x20000 - CF(1) = 0x2FFFF
    TEST_CASE t23_name
    stc
    mov eax, 0x50000
    mov ebx, 0x20000
    sbb eax, ebx
    SAVE_FLAGS
    CHECK_EQ_64 rax, 0x2FFFF

    ; --- t24: sbb Gb,Eb reg CF=0 (0x1A) ---
    ; 0x80 - 0x30 - CF(0) = 0x50
    TEST_CASE t24_name
    clc
    mov bl, 0x80
    mov cl, 0x30
    sbb bl, cl
    SAVE_FLAGS
    movzx r8, bl
    CHECK_EQ_64 r8, 0x50

    ; --- t25: sbb Gb,Eb reg CF=1 (0x1A) ---
    ; 0x80 - 0x30 - CF(1) = 0x4F
    TEST_CASE t25_name
    stc
    mov bl, 0x80
    mov cl, 0x30
    sbb bl, cl
    SAVE_FLAGS
    movzx r8, bl
    CHECK_EQ_64 r8, 0x4F

    ; --- t26: sbb Gd,Ed reg CF=0 (0x1B) ---
    ; 0xBBBB - 0x1100 - CF(0) = 0xAABB
    TEST_CASE t26_name
    clc
    mov eax, 0xBBBB
    mov ecx, 0x1100
    sbb eax, ecx
    SAVE_FLAGS
    CHECK_EQ_64 rax, 0xAABB

    ; --- t27: sbb Gd,Ed reg CF=1 (0x1B) ---
    ; 0xBBBB - 0x1100 - CF(1) = 0xAABA
    TEST_CASE t27_name
    stc
    mov eax, 0xBBBB
    mov ecx, 0x1100
    sbb eax, ecx
    SAVE_FLAGS
    CHECK_EQ_64 rax, 0xAABA

    ; --- t28: sbb AL,Ib CF=0 (0x1C) ---
    ; 0x50 - 0x20 - CF(0) = 0x30
    TEST_CASE t28_name
    clc
    mov al, 0x50
    sbb al, 0x20
    SAVE_FLAGS
    movzx r8, al
    CHECK_EQ_64 r8, 0x30

    ; --- t29: sbb AL,Ib CF=1 (0x1C) ---
    ; 0x50 - 0x20 - CF(1) = 0x2F
    TEST_CASE t29_name
    stc
    mov al, 0x50
    sbb al, 0x20
    SAVE_FLAGS
    movzx r8, al
    CHECK_EQ_64 r8, 0x2F

    ; --- t30: sbb EAX,Id CF=0 (0x1D) ---
    ; 0x500000 - 0x200000 - CF(0) = 0x300000
    TEST_CASE t30_name
    clc
    mov eax, 0x500000
    sbb eax, 0x200000
    SAVE_FLAGS
    CHECK_EQ_64 rax, 0x300000

    ; --- t31: sbb EAX,Id CF=1 (0x1D) ---
    ; 0x500000 - 0x200000 - CF(1) = 0x2FFFFF
    TEST_CASE t31_name
    stc
    mov eax, 0x500000
    sbb eax, 0x200000
    SAVE_FLAGS
    CHECK_EQ_64 rax, 0x2FFFFF

    ;; ===== SBB memory tests =====

    ; --- t32: sbb Eb,Gb mem CF=1 (0x18 mem) ---
    ; [scratch]=0x60 - bl=0x10 - CF(1) = 0x4F
    TEST_CASE t32_name
    mov byte [rel scratch], 0x60
    mov bl, 0x10
    stc
    sbb byte [rel scratch], bl
    SAVE_FLAGS
    movzx r8, byte [rel scratch]
    CHECK_EQ_64 r8, 0x4F

    ; --- t33: sbb Ed,Gd mem CF=1 (0x19 mem) ---
    ; [scratch]=0x5000 - ebx=0x2000 - CF(1) = 0x2FFF
    TEST_CASE t33_name
    mov dword [rel scratch], 0x5000
    mov ebx, 0x2000
    stc
    sbb dword [rel scratch], ebx
    SAVE_FLAGS
    mov r8d, dword [rel scratch]
    CHECK_EQ_64 r8, 0x2FFF

    ; --- t34: sbb Gb,Eb mem CF=1 (0x1A mem) ---
    ; bl=0x70 - [scratch]=0x20 - CF(1) = 0x4F
    TEST_CASE t34_name
    mov byte [rel scratch], 0x20
    mov bl, 0x70
    stc
    sbb bl, byte [rel scratch]
    SAVE_FLAGS
    movzx r8, bl
    CHECK_EQ_64 r8, 0x4F

    ; --- t35: sbb Gd,Ed mem CF=1 (0x1B mem) ---
    ; eax=0x9000 - [scratch]=0x3000 - CF(1) = 0x5FFF
    TEST_CASE t35_name
    mov dword [rel scratch], 0x3000
    mov eax, 0x9000
    stc
    sbb eax, dword [rel scratch]
    SAVE_FLAGS
    CHECK_EQ_64 rax, 0x5FFF

    ; --- t36: sbb 64-bit CF=1 (REX.W 0x19) ---
    ; 0x500000000 - 0x200000000 - CF(1) = 0x2FFFFFFFF
    TEST_CASE t36_name
    stc
    mov rax, 0x500000000
    mov rbx, 0x200000000
    sbb rax, rbx
    SAVE_FLAGS
    CHECK_EQ_64 rax, 0x2FFFFFFFF

    ; --- t37: sbb hi-byte AH CF=1 (0x18) ---
    ; AH=0x80 - BH=0x30 - CF(1) = 0x4F in AH
    TEST_CASE t37_name
    stc
    mov ax, 0x8055       ; AH=0x80, AL=0x55
    mov bx, 0x3000       ; BH=0x30
    sbb ah, bh
    ; Save AH before SAVE_FLAGS (can't mix hi-byte regs with REX)
    movzx ecx, ah
    SAVE_FLAGS
    movzx r8, cl
    CHECK_EQ_64 r8, 0x4F

    ;; ===== Flag tests =====

    ; --- t40: adc CF=1 sets CF on 8-bit overflow ---
    ; 0xFF + 0x00 + CF(1) = 0x100 -> truncated to 0x00, CF=1
    TEST_CASE t40_name
    stc
    mov al, 0xFF
    mov bl, 0x00
    adc al, bl
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF     ; CF should be set

    ; --- t41: adc CF=1 sets ZF on zero result ---
    ; 0xFF + 0x00 + CF(1) = 0x100 -> truncated to 0x00, ZF=1
    TEST_CASE t41_name
    stc
    mov al, 0xFF
    mov bl, 0x00
    adc al, bl
    SAVE_FLAGS
    CHECK_FLAGS_EQ ZF, ZF     ; ZF should be set

    ; --- t42: sbb CF=1 sets CF on borrow ---
    ; 0x00 - 0x00 - CF(1) = -1 -> 0xFF, CF=1
    TEST_CASE t42_name
    stc
    mov al, 0x00
    mov bl, 0x00
    sbb al, bl
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF     ; CF should be set (borrow occurred)

    ; --- t43: sbb CF=0 clears CF no borrow ---
    ; 0x50 - 0x20 - CF(0) = 0x30, CF=0
    TEST_CASE t43_name
    clc
    mov al, 0x50
    mov bl, 0x20
    sbb al, bl
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, 0      ; CF should be clear

    ; --- t44: adc CF=1 sets OF on signed overflow ---
    ; 0x7F + 0x00 + CF(1) = 0x80 -> signed overflow (positive became negative), OF=1
    TEST_CASE t44_name
    stc
    mov al, 0x7F
    mov bl, 0x00
    adc al, bl
    SAVE_FLAGS
    CHECK_FLAGS_EQ OF, OF     ; OF should be set

    ; --- t45: sbb CF=1 sets OF on signed overflow ---
    ; 0x80 - 0x00 - CF(1) = 0x7F -> signed overflow (negative became positive), OF=1
    TEST_CASE t45_name
    stc
    mov al, 0x80
    mov bl, 0x00
    sbb al, bl
    SAVE_FLAGS
    CHECK_FLAGS_EQ OF, OF     ; OF should be set

    END_TESTS
