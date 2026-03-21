; test_add_or_sub.asm - Test ADD (0x00-0x05), OR (0x08-0x0D), SUB (0x28-0x2D) all forms
%include "test_framework.inc"

section .data
    ; ADD tests
    t1_name:  db "add Eb,Gb reg-reg (0x00)", 0
    t2_name:  db "add Ed,Gd reg-reg (0x01)", 0
    t3_name:  db "add Gb,Eb reg-reg (0x02)", 0
    t4_name:  db "add Gd,Ed reg-reg (0x03)", 0
    t5_name:  db "add AL,Ib (0x04)", 0
    t6_name:  db "add EAX,Id (0x05)", 0
    t7_name:  db "add Eb,Gb mem (0x00 mem)", 0
    t8_name:  db "add Ed,Gd mem (0x01 mem)", 0
    t9_name:  db "add Gb,Eb mem (0x02 mem)", 0
    t10_name: db "add Gd,Ed mem (0x03 mem)", 0
    t11_name: db "add 64-bit (REX.W 0x01)", 0
    t12_name: db "add hi-byte AH (0x00)", 0

    ; OR tests
    t20_name: db "or Eb,Gb reg-reg (0x08)", 0
    t21_name: db "or Ed,Gd reg-reg (0x09)", 0
    t22_name: db "or Gb,Eb reg-reg (0x0A)", 0
    t23_name: db "or Gd,Ed reg-reg (0x0B)", 0
    t24_name: db "or AL,Ib (0x0C)", 0
    t25_name: db "or EAX,Id (0x0D)", 0
    t26_name: db "or Eb,Gb mem (0x08 mem)", 0
    t27_name: db "or Ed,Gd mem (0x09 mem)", 0
    t28_name: db "or Gb,Eb mem (0x0A mem)", 0
    t29_name: db "or Gd,Ed mem (0x0B mem)", 0
    t30_name: db "or 64-bit (REX.W 0x09)", 0
    t31_name: db "or hi-byte AH (0x08)", 0

    ; SUB tests
    t40_name: db "sub Eb,Gb reg-reg (0x28)", 0
    t41_name: db "sub Ed,Gd reg-reg (0x29)", 0
    t42_name: db "sub Gb,Eb reg-reg (0x2A)", 0
    t43_name: db "sub Gd,Ed reg-reg (0x2B)", 0
    t44_name: db "sub AL,Ib (0x2C)", 0
    t45_name: db "sub EAX,Id (0x2D)", 0
    t46_name: db "sub Eb,Gb mem (0x28 mem)", 0
    t47_name: db "sub Ed,Gd mem (0x29 mem)", 0
    t48_name: db "sub Gb,Eb mem (0x2A mem)", 0
    t49_name: db "sub Gd,Ed mem (0x2B mem)", 0
    t50_name: db "sub 64-bit (REX.W 0x29)", 0
    t51_name: db "sub hi-byte AH (0x28)", 0

    ; Flag tests
    t60_name: db "add sets CF on overflow", 0
    t61_name: db "or clears CF/OF", 0
    t62_name: db "sub sets CF on borrow", 0
    t63_name: db "add sets ZF on zero", 0

section .bss
    scratch: resq 1

section .text
global _start

_start:
    INIT_TESTS

    ;; ============ ADD ============

    TEST_CASE t1_name
    mov eax, 0x10
    mov ecx, 0x20
    add al, cl
    CHECK_EQ_32 eax, 0x30

    TEST_CASE t2_name
    mov eax, 0x11111111
    mov ebx, 0x22222222
    add eax, ebx
    CHECK_EQ_32 eax, 0x33333333

    TEST_CASE t3_name
    mov ecx, 0x05
    mov edx, 0x03
    add dl, cl
    CHECK_EQ_32 edx, 0x08

    TEST_CASE t4_name
    mov eax, 0x10000000
    mov ecx, 0x20000000
    and ecx, eax        ; zero ecx approach won't work, just set directly
    mov ecx, 0x20000000
    add ecx, eax
    CHECK_EQ_32 ecx, 0x30000000

    TEST_CASE t5_name
    mov eax, 0x10
    add al, 0x25
    CHECK_EQ_32 eax, 0x35

    TEST_CASE t6_name
    mov eax, 0x10000000
    add eax, 0x05000000
    CHECK_EQ_32 eax, 0x15000000

    TEST_CASE t7_name
    mov byte [rel scratch], 0x40
    mov cl, 0x02
    add byte [rel scratch], cl
    movzx eax, byte [rel scratch]
    CHECK_EQ_32 eax, 0x42

    TEST_CASE t8_name
    mov dword [rel scratch], 0xAAAA0000
    mov ebx, 0x00005555
    add dword [rel scratch], ebx
    mov eax, dword [rel scratch]
    CHECK_EQ_32 eax, 0xAAAA5555

    TEST_CASE t9_name
    mov byte [rel scratch], 0x11
    mov cl, 0x22
    add cl, byte [rel scratch]
    movzx eax, cl
    CHECK_EQ_32 eax, 0x33

    TEST_CASE t10_name
    mov dword [rel scratch], 0x00001111
    mov ecx, 0x00002222
    add ecx, dword [rel scratch]
    CHECK_EQ_32 ecx, 0x00003333

    TEST_CASE t11_name
    mov rax, 0x00000001FFFFFFFF
    mov rbx, 0x0000000000000001
    add rax, rbx
    CHECK_EQ_64 rax, 0x0000000200000000

    TEST_CASE t12_name
    mov eax, 0x1000
    mov ecx, 0x0500
    add ah, ch
    CHECK_EQ_32 eax, 0x1500

    ;; ============ OR ============

    TEST_CASE t20_name
    mov eax, 0xF0
    mov ecx, 0x0F
    or al, cl
    CHECK_EQ_32 eax, 0xFF

    TEST_CASE t21_name
    mov eax, 0xFF000000
    mov ebx, 0x00FF0000
    or eax, ebx
    CHECK_EQ_32 eax, 0xFFFF0000

    TEST_CASE t22_name
    mov ecx, 0xA0
    mov edx, 0x05
    or dl, cl
    CHECK_EQ_32 edx, 0xA5

    TEST_CASE t23_name
    mov eax, 0x0000FFFF
    mov ecx, 0xFFFF0000
    or ecx, eax
    CHECK_EQ_32 ecx, 0xFFFFFFFF

    TEST_CASE t24_name
    mov eax, 0xA0
    or al, 0x05
    CHECK_EQ_32 eax, 0xA5

    TEST_CASE t25_name
    mov eax, 0xFF000000
    or eax, 0x000000FF
    CHECK_EQ_32 eax, 0xFF0000FF

    TEST_CASE t26_name
    mov byte [rel scratch], 0xF0
    mov cl, 0x0F
    or byte [rel scratch], cl
    movzx eax, byte [rel scratch]
    CHECK_EQ_32 eax, 0xFF

    TEST_CASE t27_name
    mov dword [rel scratch], 0xAAAA0000
    mov ebx, 0x00005555
    or dword [rel scratch], ebx
    mov eax, dword [rel scratch]
    CHECK_EQ_32 eax, 0xAAAA5555

    TEST_CASE t28_name
    mov byte [rel scratch], 0x0F
    mov cl, 0xA0
    or cl, byte [rel scratch]
    movzx eax, cl
    CHECK_EQ_32 eax, 0xAF

    TEST_CASE t29_name
    mov dword [rel scratch], 0x00FF0000
    mov ecx, 0xFF000000
    or ecx, dword [rel scratch]
    CHECK_EQ_32 ecx, 0xFFFF0000

    TEST_CASE t30_name
    mov rax, 0x00000000FFFF0000
    mov rbx, 0xFFFF000000000000
    or rax, rbx
    CHECK_EQ_64 rax, 0xFFFF0000FFFF0000

    TEST_CASE t31_name
    mov eax, 0xA000
    mov ecx, 0x0500
    or ah, ch
    CHECK_EQ_32 eax, 0xA500

    ;; ============ SUB ============

    TEST_CASE t40_name
    mov eax, 0x30
    mov ecx, 0x10
    sub al, cl
    CHECK_EQ_32 eax, 0x20

    TEST_CASE t41_name
    mov eax, 0x55555555
    mov ebx, 0x11111111
    sub eax, ebx
    CHECK_EQ_32 eax, 0x44444444

    TEST_CASE t42_name
    mov ecx, 0x05
    mov edx, 0x30
    sub dl, cl
    CHECK_EQ_32 edx, 0x2B

    TEST_CASE t43_name
    mov eax, 0x11111111
    mov ecx, 0x55555555
    sub ecx, eax
    CHECK_EQ_32 ecx, 0x44444444

    TEST_CASE t44_name
    mov eax, 0x50
    sub al, 0x15
    CHECK_EQ_32 eax, 0x3B

    TEST_CASE t45_name
    mov eax, 0x80000000
    sub eax, 0x00000001
    CHECK_EQ_32 eax, 0x7FFFFFFF

    TEST_CASE t46_name
    mov byte [rel scratch], 0x80
    mov cl, 0x01
    sub byte [rel scratch], cl
    movzx eax, byte [rel scratch]
    CHECK_EQ_32 eax, 0x7F

    TEST_CASE t47_name
    mov dword [rel scratch], 0xAAAA5555
    mov ebx, 0x00005555
    sub dword [rel scratch], ebx
    mov eax, dword [rel scratch]
    CHECK_EQ_32 eax, 0xAAAA0000

    TEST_CASE t48_name
    mov byte [rel scratch], 0x10
    mov cl, 0x50
    sub cl, byte [rel scratch]
    movzx eax, cl
    CHECK_EQ_32 eax, 0x40

    TEST_CASE t49_name
    mov dword [rel scratch], 0x11111111
    mov ecx, 0x55555555
    sub ecx, dword [rel scratch]
    CHECK_EQ_32 ecx, 0x44444444

    TEST_CASE t50_name
    mov rax, 0x0000000200000000
    mov rbx, 0x0000000000000001
    sub rax, rbx
    CHECK_EQ_64 rax, 0x00000001FFFFFFFF

    TEST_CASE t51_name
    mov eax, 0x5000
    mov ecx, 0x1000
    sub ah, ch
    CHECK_EQ_32 eax, 0x4000

    ;; ============ Flag tests ============

    TEST_CASE t60_name
    mov al, 0xFF
    add al, 1
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    TEST_CASE t61_name
    stc
    mov eax, 0xF0
    or eax, 0x0F
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|OF), 0

    TEST_CASE t62_name
    mov al, 0x00
    sub al, 1
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    TEST_CASE t63_name
    mov al, 0x80
    add al, 0x80
    SAVE_FLAGS
    CHECK_FLAGS_EQ ZF, ZF

    END_TESTS
