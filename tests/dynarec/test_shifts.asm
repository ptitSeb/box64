; test_shifts.asm - Test SHL, SHR, SAR, ROL, ROR with immediate and CL
; Including flag behavior. Shift masking (Bug #17) is critical.
%include "test_framework.inc"

section .data
    t1_name:  db "shl eax,1", 0
    t2_name:  db "shl eax,cl (cl=4)", 0
    t3_name:  db "shl rax,cl (cl=63)", 0
    t4_name:  db "shl rax,cl (cl=64 masked to 0)", 0
    t5_name:  db "shr eax,1", 0
    t6_name:  db "shr eax,cl (cl=4)", 0
    t7_name:  db "shr rax,cl (cl=63)", 0
    t8_name:  db "shr rax,cl (cl=64 masked to 0)", 0
    t9_name:  db "sar eax,1 (negative)", 0
    t10_name: db "sar eax,cl (cl=31, negative)", 0
    t11_name: db "sar rax,cl (cl=63, negative)", 0
    t12_name: db "sar eax,1 (positive)", 0
    t13_name: db "rol eax,1", 0
    t14_name: db "rol eax,cl (cl=4)", 0
    t15_name: db "rol rax,cl (cl=1)", 0
    t16_name: db "ror eax,1", 0
    t17_name: db "ror eax,cl (cl=4)", 0
    t18_name: db "ror rax,cl (cl=1)", 0
    t19_name: db "shl eax,0 (no-op)", 0
    t20_name: db "shr eax,0 (no-op)", 0
    t21_name: db "shl rax,cl (cl=32)", 0
    t22_name: db "shr rax,cl (cl=32)", 0
    t23_name: db "sar rax,cl (cl=32, negative)", 0
    t24_name: db "shl eax,31", 0
    t25_name: db "shr eax,31", 0
    t26_name: db "rol eax,cl (cl=32 masked to 0)", 0
    t27_name: db "ror eax,cl (cl=32 masked to 0)", 0
    t28_name: db "shl flags CF", 0
    t29_name: db "shr flags CF", 0
    t30_name: db "double shift shld eax,ebx,cl", 0
    t31_name: db "double shift shrd eax,ebx,cl", 0
    t32_name: db "shld eax,ebx,4", 0
    t33_name: db "shrd eax,ebx,4", 0

section .text
global _start

_start:
    INIT_TESTS

    ; ==== Test 1: shl eax, 1 ====
    TEST_CASE t1_name
    mov eax, 0x12345678
    shl eax, 1
    CHECK_EQ_32 eax, 0x2468ACF0

    ; ==== Test 2: shl eax, cl (cl=4) ====
    TEST_CASE t2_name
    mov eax, 0x12345678
    mov ecx, 4
    shl eax, cl
    CHECK_EQ_32 eax, 0x23456780

    ; ==== Test 3: shl rax, cl (cl=63) ====
    TEST_CASE t3_name
    mov rax, 0x12345678ABCDEF01
    mov ecx, 63
    shl rax, cl
    CHECK_EQ_64 rax, 0x8000000000000000

    ; ==== Test 4: shl rax, cl (cl=64, should mask to 0 = no shift) ====
    TEST_CASE t4_name
    mov rax, 0x12345678ABCDEF01
    mov ecx, 64
    shl rax, cl
    ; x86 masks cl & 0x3F = 0, so no shift
    CHECK_EQ_64 rax, 0x12345678ABCDEF01

    ; ==== Test 5: shr eax, 1 ====
    TEST_CASE t5_name
    mov eax, 0x80000002
    shr eax, 1
    CHECK_EQ_32 eax, 0x40000001

    ; ==== Test 6: shr eax, cl (cl=4) ====
    TEST_CASE t6_name
    mov eax, 0xFEDCBA98
    mov ecx, 4
    shr eax, cl
    CHECK_EQ_32 eax, 0x0FEDCBA9

    ; ==== Test 7: shr rax, cl (cl=63) ====
    TEST_CASE t7_name
    mov rax, 0x8000000000000000
    mov ecx, 63
    shr rax, cl
    CHECK_EQ_64 rax, 1

    ; ==== Test 8: shr rax, cl (cl=64, mask to 0) ====
    TEST_CASE t8_name
    mov rax, 0xFEDCBA9876543210
    mov ecx, 64
    shr rax, cl
    ; Mask to 0, no shift
    CHECK_EQ_64 rax, 0xFEDCBA9876543210

    ; ==== Test 9: sar eax, 1 (negative) ====
    TEST_CASE t9_name
    mov eax, 0x80000000
    sar eax, 1
    CHECK_EQ_32 eax, 0xC0000000

    ; ==== Test 10: sar eax, cl (cl=31, negative) ====
    TEST_CASE t10_name
    mov eax, 0x80000000
    mov ecx, 31
    sar eax, cl
    CHECK_EQ_32 eax, 0xFFFFFFFF

    ; ==== Test 11: sar rax, cl (cl=63, negative) ====
    TEST_CASE t11_name
    mov rax, 0x8000000000000000
    mov ecx, 63
    sar rax, cl
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFF

    ; ==== Test 12: sar eax, 1 (positive) ====
    TEST_CASE t12_name
    mov eax, 0x7FFFFFFE
    sar eax, 1
    CHECK_EQ_32 eax, 0x3FFFFFFF

    ; ==== Test 13: rol eax, 1 ====
    TEST_CASE t13_name
    mov eax, 0x80000001
    rol eax, 1
    ; bit 31 goes to bit 0, bit 0 goes to bit 1
    CHECK_EQ_32 eax, 0x00000003

    ; ==== Test 14: rol eax, cl (cl=4) ====
    TEST_CASE t14_name
    mov eax, 0xF0000001
    mov ecx, 4
    rol eax, cl
    ; top 4 bits (F) rotate to bottom
    CHECK_EQ_32 eax, 0x0000001F

    ; ==== Test 15: rol rax, cl (cl=1) ====
    TEST_CASE t15_name
    mov rax, 0x8000000000000001
    mov ecx, 1
    rol rax, cl
    CHECK_EQ_64 rax, 0x0000000000000003

    ; ==== Test 16: ror eax, 1 ====
    TEST_CASE t16_name
    mov eax, 0x00000003
    ror eax, 1
    ; bit 0 goes to bit 31
    CHECK_EQ_32 eax, 0x80000001

    ; ==== Test 17: ror eax, cl (cl=4) ====
    TEST_CASE t17_name
    mov eax, 0x0000001F
    mov ecx, 4
    ror eax, cl
    CHECK_EQ_32 eax, 0xF0000001

    ; ==== Test 18: ror rax, cl (cl=1) ====
    TEST_CASE t18_name
    mov rax, 0x0000000000000003
    mov ecx, 1
    ror rax, cl
    CHECK_EQ_64 rax, 0x8000000000000001

    ; ==== Test 19: shl eax, 0 (no-op) ====
    TEST_CASE t19_name
    mov eax, 0xDEADBEEF
    shl eax, 0
    CHECK_EQ_32 eax, 0xDEADBEEF

    ; ==== Test 20: shr eax, 0 (no-op) ====
    TEST_CASE t20_name
    mov eax, 0xDEADBEEF
    shr eax, 0
    CHECK_EQ_32 eax, 0xDEADBEEF

    ; ==== Test 21: shl rax, cl (cl=32) ====
    TEST_CASE t21_name
    mov rax, 0x00000001FFFFFFFF
    mov ecx, 32
    shl rax, cl
    CHECK_EQ_64 rax, 0xFFFFFFFF00000000

    ; ==== Test 22: shr rax, cl (cl=32) ====
    TEST_CASE t22_name
    mov rax, 0xFFFFFFFF00000001
    mov ecx, 32
    shr rax, cl
    CHECK_EQ_64 rax, 0x00000000FFFFFFFF

    ; ==== Test 23: sar rax, cl (cl=32, negative) ====
    TEST_CASE t23_name
    mov rax, 0x8000000000000000
    mov ecx, 32
    sar rax, cl
    CHECK_EQ_64 rax, 0xFFFFFFFF80000000

    ; ==== Test 24: shl eax, 31 ====
    TEST_CASE t24_name
    mov eax, 1
    shl eax, 31
    CHECK_EQ_32 eax, 0x80000000

    ; ==== Test 25: shr eax, 31 ====
    TEST_CASE t25_name
    mov eax, 0x80000000
    shr eax, 31
    CHECK_EQ_32 eax, 1

    ; ==== Test 26: rol eax, cl (cl=32 masks to 0 for 32-bit) ====
    TEST_CASE t26_name
    mov eax, 0x12345678
    mov ecx, 32
    rol eax, cl
    ; x86 masks ROL by 31 for 32-bit: 32 & 0x1F = 0, no rotation
    CHECK_EQ_32 eax, 0x12345678

    ; ==== Test 27: ror eax, cl (cl=32 masks to 0) ====
    TEST_CASE t27_name
    mov eax, 0x12345678
    mov ecx, 32
    ror eax, cl
    CHECK_EQ_32 eax, 0x12345678

    ; ==== Test 28: shl flags CF ====
    ; shl sets CF to last bit shifted out
    TEST_CASE t28_name
    mov eax, 0x80000000
    shl eax, 1
    SAVE_FLAGS
    ; Bit 31 was set, shifted out -> CF=1
    CHECK_FLAGS_EQ CF, CF

    ; ==== Test 29: shr flags CF ====
    ; shr sets CF to last bit shifted out
    TEST_CASE t29_name
    mov eax, 0x00000001
    shr eax, 1
    SAVE_FLAGS
    ; Bit 0 was set, shifted out -> CF=1
    CHECK_FLAGS_EQ CF, CF

    ; ==== Test 30: shld eax, ebx, cl ====
    TEST_CASE t30_name
    mov eax, 0x12345678
    mov ebx, 0xABCDEF01
    mov ecx, 8
    shld eax, ebx, cl
    ; Shifts eax left by 8, fills from top 8 bits of ebx
    ; eax = (eax << 8) | (ebx >> 24) = 0x345678AB
    CHECK_EQ_32 eax, 0x345678AB

    ; ==== Test 31: shrd eax, ebx, cl ====
    TEST_CASE t31_name
    mov eax, 0x12345678
    mov ebx, 0xABCDEF01
    mov ecx, 8
    shrd eax, ebx, cl
    ; Shifts eax right by 8, fills from bottom 8 bits of ebx
    ; eax = (ebx_low8 << 24) | (eax >> 8) = 0x01123456
    CHECK_EQ_32 eax, 0x01123456

    ; ==== Test 32: shld eax, ebx, 4 (immediate) ====
    TEST_CASE t32_name
    mov eax, 0x12345678
    mov ebx, 0xABCDEF01
    shld eax, ebx, 4
    ; eax = (eax << 4) | (ebx >> 28) = 0x2345678A
    CHECK_EQ_32 eax, 0x2345678A

    ; ==== Test 33: shrd eax, ebx, 4 (immediate) ====
    TEST_CASE t33_name
    mov eax, 0x12345678
    mov ebx, 0xABCDEF01
    shrd eax, ebx, 4
    ; eax = (ebx_low4 << 28) | (eax >> 4) = 0x11234567
    CHECK_EQ_32 eax, 0x11234567

    END_TESTS
