; test_lock_atomic.asm - Test LOCK-prefixed atomic instructions and CMPXCHG/XADD/XCHG
; LOCK ADD/SUB/AND/OR/XOR, LOCK INC/DEC, LOCK BTS/BTR/BTC,
; LOCK CMPXCHG, LOCK CMPXCHG8B, LOCK CMPXCHG16B, LOCK XADD, XCHG [mem]
%include "test_framework.inc"

section .data
    t1_name:  db "lock add byte", 0
    t2_name:  db "lock add dword", 0
    t3_name:  db "lock add qword", 0
    t4_name:  db "lock sub dword", 0
    t5_name:  db "lock sub qword", 0
    t6_name:  db "lock and dword", 0
    t7_name:  db "lock and qword", 0
    t8_name:  db "lock or dword", 0
    t9_name:  db "lock or qword", 0
    t10_name: db "lock xor dword", 0
    t11_name: db "lock xor qword", 0
    t12_name: db "lock inc byte", 0
    t13_name: db "lock inc dword", 0
    t14_name: db "lock inc qword", 0
    t15_name: db "lock dec dword", 0
    t16_name: db "lock dec qword", 0
    t17_name: db "lock bts set bit CF=0", 0
    t18_name: db "lock bts set bit CF=1", 0
    t19_name: db "lock bts mem result", 0
    t20_name: db "lock btr clear CF=1", 0
    t21_name: db "lock btr clear CF=0", 0
    t22_name: db "lock btc compl CF=1", 0
    t23_name: db "lock btc compl CF=0", 0
    t24_name: db "lock cmpxchg match", 0
    t25_name: db "lock cmpxchg mismatch", 0
    t26_name: db "lock cmpxchg64 match", 0
    t27_name: db "lock cmpxchg8b match", 0
    t28_name: db "lock cmpxchg8b fail", 0
    t29_name: db "lock cmpxchg16b match", 0
    t30_name: db "lock cmpxchg16b fail", 0
    t31_name: db "lock xadd dword", 0
    t32_name: db "lock xadd qword", 0
    t33_name: db "xchg [mem] dword", 0
    t34_name: db "xchg [mem] qword", 0
    t35_name: db "lock add dword imm", 0

section .bss
    alignb 8
    val8:    resb 8        ; general 8-byte scratch
    val8b:   resb 8        ; second scratch
    alignb 16
    val16:   resb 16       ; 16-byte aligned for cmpxchg16b

section .text
global _start

_start:
    INIT_TESTS

    ; ==== Test 1: LOCK ADD byte [mem], reg ====
    ; 0x10 + 0x20 = 0x30
    TEST_CASE t1_name
    lea rbx, [rel val8]
    mov byte [rbx], 0x10
    mov al, 0x20
    lock add byte [rbx], al
    movzx eax, byte [rbx]
    CHECK_EQ_32 eax, 0x30

    ; ==== Test 2: LOCK ADD dword [mem], reg ====
    ; 0x10000 + 0x20000 = 0x30000
    TEST_CASE t2_name
    lea rbx, [rel val8]
    mov dword [rbx], 0x10000
    mov ecx, 0x20000
    lock add dword [rbx], ecx
    mov eax, [rbx]
    CHECK_EQ_32 eax, 0x30000

    ; ==== Test 3: LOCK ADD qword [mem], reg ====
    ; 0x100000000 + 0x200000000 = 0x300000000
    TEST_CASE t3_name
    lea rbx, [rel val8]
    mov rax, 0x100000000
    mov [rbx], rax
    mov rcx, 0x200000000
    lock add qword [rbx], rcx
    mov rax, [rbx]
    CHECK_EQ_64 rax, 0x300000000

    ; ==== Test 4: LOCK SUB dword [mem], reg ====
    ; 100 - 30 = 70
    TEST_CASE t4_name
    lea rbx, [rel val8]
    mov dword [rbx], 100
    mov ecx, 30
    lock sub dword [rbx], ecx
    mov eax, [rbx]
    CHECK_EQ_32 eax, 70

    ; ==== Test 5: LOCK SUB qword [mem], reg ====
    ; 0x300000000 - 0x100000000 = 0x200000000
    TEST_CASE t5_name
    lea rbx, [rel val8]
    mov rax, 0x300000000
    mov [rbx], rax
    mov rcx, 0x100000000
    lock sub qword [rbx], rcx
    mov rax, [rbx]
    CHECK_EQ_64 rax, 0x200000000

    ; ==== Test 6: LOCK AND dword [mem], reg ====
    ; 0xFF00FF00 & 0x0F0F0F0F = 0x0F000F00
    TEST_CASE t6_name
    lea rbx, [rel val8]
    mov dword [rbx], 0xFF00FF00
    mov ecx, 0x0F0F0F0F
    lock and dword [rbx], ecx
    mov eax, [rbx]
    CHECK_EQ_32 eax, 0x0F000F00

    ; ==== Test 7: LOCK AND qword [mem], reg ====
    ; 0xFF00FF00FF00FF00 & 0x0F0F0F0F0F0F0F0F = 0x0F000F000F000F00
    TEST_CASE t7_name
    lea rbx, [rel val8]
    mov rax, 0xFF00FF00FF00FF00
    mov [rbx], rax
    mov rcx, 0x0F0F0F0F0F0F0F0F
    lock and qword [rbx], rcx
    mov rax, [rbx]
    CHECK_EQ_64 rax, 0x0F000F000F000F00

    ; ==== Test 8: LOCK OR dword [mem], reg ====
    ; 0xF0F0F0F0 | 0x0F0F0F0F = 0xFFFFFFFF
    TEST_CASE t8_name
    lea rbx, [rel val8]
    mov dword [rbx], 0xF0F0F0F0
    mov ecx, 0x0F0F0F0F
    lock or dword [rbx], ecx
    mov eax, [rbx]
    CHECK_EQ_32 eax, 0xFFFFFFFF

    ; ==== Test 9: LOCK OR qword [mem], reg ====
    ; 0xF0F0F0F0F0F0F0F0 | 0x0F0F0F0F0F0F0F0F = 0xFFFFFFFFFFFFFFFF
    TEST_CASE t9_name
    lea rbx, [rel val8]
    mov rax, 0xF0F0F0F0F0F0F0F0
    mov [rbx], rax
    mov rcx, 0x0F0F0F0F0F0F0F0F
    lock or qword [rbx], rcx
    mov rax, [rbx]
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFF

    ; ==== Test 10: LOCK XOR dword [mem], reg ====
    ; 0xAAAAAAAA ^ 0x55555555 = 0xFFFFFFFF
    TEST_CASE t10_name
    lea rbx, [rel val8]
    mov dword [rbx], 0xAAAAAAAA
    mov ecx, 0x55555555
    lock xor dword [rbx], ecx
    mov eax, [rbx]
    CHECK_EQ_32 eax, 0xFFFFFFFF

    ; ==== Test 11: LOCK XOR qword [mem], reg ====
    ; 0xAAAAAAAAAAAAAAAA ^ 0xFFFFFFFFFFFFFFFF = 0x5555555555555555
    TEST_CASE t11_name
    lea rbx, [rel val8]
    mov rax, 0xAAAAAAAAAAAAAAAA
    mov [rbx], rax
    mov rcx, 0xFFFFFFFFFFFFFFFF
    lock xor qword [rbx], rcx
    mov rax, [rbx]
    CHECK_EQ_64 rax, 0x5555555555555555

    ; ==== Test 12: LOCK INC byte [mem] ====
    ; 0x41 -> 0x42
    TEST_CASE t12_name
    lea rbx, [rel val8]
    mov byte [rbx], 0x41
    lock inc byte [rbx]
    movzx eax, byte [rbx]
    CHECK_EQ_32 eax, 0x42

    ; ==== Test 13: LOCK INC dword [mem] ====
    ; 99 -> 100
    TEST_CASE t13_name
    lea rbx, [rel val8]
    mov dword [rbx], 99
    lock inc dword [rbx]
    mov eax, [rbx]
    CHECK_EQ_32 eax, 100

    ; ==== Test 14: LOCK INC qword [mem] ====
    ; 0xFFFFFFFF -> 0x100000000  (carry across 32-bit boundary)
    TEST_CASE t14_name
    lea rbx, [rel val8]
    mov rax, 0xFFFFFFFF
    mov [rbx], rax
    lock inc qword [rbx]
    mov rax, [rbx]
    CHECK_EQ_64 rax, 0x100000000

    ; ==== Test 15: LOCK DEC dword [mem] ====
    ; 100 -> 99
    TEST_CASE t15_name
    lea rbx, [rel val8]
    mov dword [rbx], 100
    lock dec dword [rbx]
    mov eax, [rbx]
    CHECK_EQ_32 eax, 99

    ; ==== Test 16: LOCK DEC qword [mem] ====
    ; 0x100000000 -> 0xFFFFFFFF  (borrow across 32-bit boundary)
    TEST_CASE t16_name
    lea rbx, [rel val8]
    mov rax, 0x100000000
    mov [rbx], rax
    lock dec qword [rbx]
    mov rax, [rbx]
    CHECK_EQ_64 rax, 0xFFFFFFFF

    ; ==== Test 17: LOCK BTS [mem], reg - bit was 0, CF=0 ====
    ; mem = 0x00000000, set bit 5 -> CF=0 (old bit), mem becomes 0x20
    TEST_CASE t17_name
    lea rbx, [rel val8]
    mov dword [rbx], 0
    mov ecx, 5
    lock bts dword [rbx], ecx
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, 0          ; old bit was 0

    ; ==== Test 18: LOCK BTS [mem], reg - bit was 1, CF=1 ====
    ; mem still = 0x20 from test 17, set bit 5 again -> CF=1
    TEST_CASE t18_name
    lea rbx, [rel val8]
    mov dword [rbx], 0x00000020   ; bit 5 is set
    mov ecx, 5
    lock bts dword [rbx], ecx
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF         ; old bit was 1

    ; ==== Test 19: LOCK BTS verify memory result ====
    ; mem = 0, set bit 10 -> mem = 0x400
    TEST_CASE t19_name
    lea rbx, [rel val8]
    mov dword [rbx], 0
    mov ecx, 10
    lock bts dword [rbx], ecx
    mov eax, [rbx]
    CHECK_EQ_32 eax, 0x00000400

    ; ==== Test 20: LOCK BTR [mem], reg - bit was 1, CF=1 ====
    ; mem = 0xFF, clear bit 3 -> CF=1, mem = 0xF7
    TEST_CASE t20_name
    lea rbx, [rel val8]
    mov dword [rbx], 0xFF
    mov ecx, 3
    lock btr dword [rbx], ecx
    SAVE_FLAGS
    ; Check CF=1 (old bit was set) and check memory
    CHECK_FLAGS_EQ CF, CF

    ; ==== Test 21: LOCK BTR [mem], reg - bit was 0, CF=0 ====
    ; mem = 0xF7 (bit 3 clear), clear bit 3 -> CF=0
    TEST_CASE t21_name
    lea rbx, [rel val8]
    mov dword [rbx], 0xF7         ; bit 3 already clear
    mov ecx, 3
    lock btr dword [rbx], ecx
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, 0          ; old bit was 0

    ; ==== Test 22: LOCK BTC [mem], reg - bit was 1, CF=1 ====
    ; mem = 0xFF, complement bit 7 -> CF=1 (was set), mem = 0x7F
    TEST_CASE t22_name
    lea rbx, [rel val8]
    mov dword [rbx], 0xFF
    mov ecx, 7
    lock btc dword [rbx], ecx
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF         ; old bit was 1

    ; ==== Test 23: LOCK BTC [mem], reg - bit was 0, CF=0 ====
    ; mem = 0x7F (bit 7 is 0), complement bit 7 -> CF=0, mem = 0xFF
    TEST_CASE t23_name
    lea rbx, [rel val8]
    mov dword [rbx], 0x7F         ; bit 7 is clear
    mov ecx, 7
    lock btc dword [rbx], ecx
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, 0          ; old bit was 0

    ; ==== Test 24: LOCK CMPXCHG dword match (swap happens) ====
    ; [mem]=42, eax=42, ecx=99 -> match: [mem]=99, ZF=1
    TEST_CASE t24_name
    lea rbx, [rel val8]
    mov dword [rbx], 42
    mov eax, 42                   ; comparand
    mov ecx, 99                   ; new value
    lock cmpxchg dword [rbx], ecx
    ; Verify memory got the new value
    mov eax, [rbx]
    CHECK_EQ_32 eax, 99

    ; ==== Test 25: LOCK CMPXCHG dword mismatch (no swap) ====
    ; [mem]=43, eax=42, ecx=99 -> mismatch: eax=43, [mem] unchanged
    TEST_CASE t25_name
    lea rbx, [rel val8]
    mov dword [rbx], 43
    mov eax, 42                   ; comparand (doesn't match)
    mov ecx, 99
    lock cmpxchg dword [rbx], ecx
    ; eax should be loaded with [mem] = 43
    CHECK_EQ_32 eax, 43

    ; ==== Test 26: LOCK CMPXCHG qword match ====
    ; [mem]=0xDEADBEEF12345678, rax=same, rcx=0x1111111122222222 -> swap
    TEST_CASE t26_name
    lea rbx, [rel val8]
    mov rax, 0xDEADBEEF12345678
    mov [rbx], rax
    mov rcx, 0x1111111122222222
    lock cmpxchg qword [rbx], rcx
    mov rax, [rbx]
    CHECK_EQ_64 rax, 0x1111111122222222

    ; ==== Test 27: LOCK CMPXCHG8B match ====
    ; [mem] = 0x0000000200000001 (edx:eax = 00000002:00000001)
    ; Set edx:eax to match, ecx:ebx = new value 00000004:00000003
    ; After: [mem] = 0x0000000400000003
    TEST_CASE t27_name
    push rbx                      ; save - cmpxchg8b uses ebx
    lea rdi, [rel val8]
    mov dword [rdi], 0x00000001   ; low dword
    mov dword [rdi+4], 0x00000002 ; high dword
    mov eax, 0x00000001           ; expected low
    mov edx, 0x00000002           ; expected high
    mov ebx, 0x00000003           ; new low
    mov ecx, 0x00000004           ; new high
    lock cmpxchg8b [rdi]
    ; Match -> store ecx:ebx to [mem]
    ; Check low dword
    mov eax, [rdi]
    pop rbx
    CHECK_EQ_32 eax, 0x00000003

    ; ==== Test 28: LOCK CMPXCHG8B mismatch ====
    ; [mem] = 0x00000006_00000005, edx:eax = 0x00000002_00000001 (no match)
    ; After: edx:eax loaded from [mem] = 0x00000006:00000005
    TEST_CASE t28_name
    push rbx
    lea rdi, [rel val8]
    mov dword [rdi], 0x00000005   ; low dword
    mov dword [rdi+4], 0x00000006 ; high dword
    mov eax, 0x00000001           ; expected low (wrong)
    mov edx, 0x00000002           ; expected high (wrong)
    mov ebx, 0x00000099           ; new low (should not be stored)
    mov ecx, 0x000000AA           ; new high (should not be stored)
    lock cmpxchg8b [rdi]
    ; Mismatch -> edx:eax loaded from [mem]
    pop rbx
    ; edx should be 6, eax should be 5
    CHECK_EQ_32 eax, 0x00000005

    ; ==== Test 29: LOCK CMPXCHG16B match ====
    ; [mem] = rdx:rax = 0x22222222:0x11111111 (128 bits)
    ; new = rcx:rbx = 0x44444444:0x33333333
    ; After match: [mem] = rcx:rbx
    TEST_CASE t29_name
    push rbx
    lea rdi, [rel val16]
    mov rax, 0x1111111111111111
    mov [rdi], rax                ; low qword
    mov rax, 0x2222222222222222
    mov [rdi+8], rax              ; high qword
    mov rax, 0x1111111111111111   ; expected low
    mov rdx, 0x2222222222222222   ; expected high
    mov rbx, 0x3333333333333333   ; new low
    mov rcx, 0x4444444444444444   ; new high
    lock cmpxchg16b [rdi]
    ; Match -> store rcx:rbx
    mov rax, [rdi]                ; low qword of result
    pop rbx
    CHECK_EQ_64 rax, 0x3333333333333333

    ; ==== Test 30: LOCK CMPXCHG16B mismatch ====
    ; [mem] = high:0xBBBBBBBB low:0xAAAAAAAA
    ; rdx:rax = 0x22222222:0x11111111 (doesn't match)
    ; After: rdx:rax loaded from [mem]
    TEST_CASE t30_name
    push rbx
    lea rdi, [rel val16]
    mov rax, 0xAAAAAAAAAAAAAAAA
    mov [rdi], rax                ; low qword
    mov rax, 0xBBBBBBBBBBBBBBBB
    mov [rdi+8], rax              ; high qword
    mov rax, 0x1111111111111111   ; expected low (wrong)
    mov rdx, 0x2222222222222222   ; expected high (wrong)
    mov rbx, 0x9999999999999999   ; new (should not be stored)
    mov rcx, 0x8888888888888888
    lock cmpxchg16b [rdi]
    ; Mismatch -> rdx:rax loaded from [mem]
    pop rbx
    ; rax should be the low qword of [mem]
    CHECK_EQ_64 rax, 0xAAAAAAAAAAAAAAAA

    ; ==== Test 31: LOCK XADD dword [mem], reg ====
    ; [mem]=10, ecx=20 -> [mem]=30, ecx=old [mem]=10
    TEST_CASE t31_name
    lea rbx, [rel val8]
    mov dword [rbx], 10
    mov ecx, 20
    lock xadd dword [rbx], ecx
    ; ecx gets old value (10), [mem] = 10+20 = 30
    CHECK_EQ_32 ecx, 10

    ; ==== Test 32: LOCK XADD qword [mem], reg ====
    ; [mem]=0x100, rcx=0x200 -> [mem]=0x300, rcx=0x100
    TEST_CASE t32_name
    lea rbx, [rel val8]
    mov rax, 0x100
    mov [rbx], rax
    mov rcx, 0x200
    lock xadd qword [rbx], rcx
    ; Verify [mem] = 0x300
    mov rax, [rbx]
    CHECK_EQ_64 rax, 0x300

    ; ==== Test 33: XCHG [mem], reg dword (implicit LOCK) ====
    ; [mem]=0xAABBCCDD, ecx=0x11223344 -> [mem]=0x11223344, ecx=0xAABBCCDD
    TEST_CASE t33_name
    lea rbx, [rel val8]
    mov dword [rbx], 0xAABBCCDD
    mov ecx, 0x11223344
    xchg dword [rbx], ecx
    ; ecx should have old [mem] value
    CHECK_EQ_32 ecx, 0xAABBCCDD

    ; ==== Test 34: XCHG [mem], reg qword (implicit LOCK) ====
    ; [mem]=0xDEADBEEFCAFEBABE, rcx=0x1234567890ABCDEF
    ; -> [mem]=0x1234567890ABCDEF, rcx=0xDEADBEEFCAFEBABE
    TEST_CASE t34_name
    lea rbx, [rel val8]
    mov rax, 0xDEADBEEFCAFEBABE
    mov [rbx], rax
    mov rcx, 0x1234567890ABCDEF
    xchg qword [rbx], rcx
    ; Check that [mem] got the new value
    mov rax, [rbx]
    CHECK_EQ_64 rax, 0x1234567890ABCDEF

    ; ==== Test 35: LOCK ADD dword [mem], imm (sign-extended) ====
    ; 0x7FFFFFF0 + 0x10 = 0x80000000
    TEST_CASE t35_name
    lea rbx, [rel val8]
    mov dword [rbx], 0x7FFFFFF0
    lock add dword [rbx], 0x10
    mov eax, [rbx]
    CHECK_EQ_32 eax, 0x80000000

    END_TESTS
