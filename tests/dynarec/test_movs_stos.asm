; test_movs_stos.asm - Test x86 string instructions (REP prefix)
; REP MOVSB/MOVSW/MOVSD/MOVSQ
; REP STOSB/STOSW/STOSD/STOSQ
; REPE CMPSB/CMPSW/CMPSD
; REPNE SCASB/SCASW
; LODSB/LODSW/LODSD/LODSQ
; Tests DF (direction flag) forward and backward
%include "test_framework.inc"

section .data
    t1_name:  db "rep movsb forward 8", 0
    t2_name:  db "rep stosb fill 16", 0
    t3_name:  db "repe cmpsb equal", 0
    t4_name:  db "repe cmpsb diff at 3", 0
    t5_name:  db "repne scasb find byte", 0
    t6_name:  db "rep movsw forward 4", 0
    t7_name:  db "rep stosd fill 4", 0
    t8_name:  db "rep movsq forward 2", 0
    t9_name:  db "rep stosq fill 2", 0
    t10_name: db "lodsb", 0
    t11_name: db "lodsw", 0
    t12_name: db "lodsd", 0
    t13_name: db "lodsq", 0
    t14_name: db "rep movsb backward", 0
    t15_name: db "repne scasb not found", 0
    t16_name: db "repe cmpsw equal 4", 0
    t17_name: db "repe cmpsd diff at 1", 0
    t18_name: db "rep movsb count=0", 0
    t19_name: db "rep stosb count=0", 0
    t20_name: db "repne scasb first byte", 0
    t21_name: db "rep movsd forward 4", 0
    t22_name: db "rep movsb overlap fwd", 0
    t23_name: db "rep stosb large", 0
    t24_name: db "repe cmpsb ZF after eq", 0
    t25_name: db "repe cmpsb ZF after ne", 0

    align 16
    src_bytes: db "ABCDEFGH", 0, 0, 0, 0, 0, 0, 0, 0
    src_hello: db "Hello, World!", 0, 0, 0
    src_diff:  db "Hello, Xorld!", 0, 0, 0
    src_words: dw 0x1111, 0x2222, 0x3333, 0x4444, 0, 0, 0, 0
    src_dwords: dd 0xAABBCCDD, 0x11223344, 0x55667788, 0x99AABBCC
    src_qwords: dq 0x1122334455667788, 0xAABBCCDDEEFF0011
    search_str: db "Find the X in here!", 0

section .bss
    alignb 8
    dst_buf: resb 128
    dst_buf2: resb 128
    cmp_buf: resb 64

section .text
global _start

_start:
    INIT_TESTS

    ; ==== Test 1: REP MOVSB forward - copy 8 bytes ====
    TEST_CASE t1_name
    ; Clear dst first
    lea rdi, [rel dst_buf]
    xor eax, eax
    mov ecx, 16
    rep stosb
    ; Now copy
    cld                        ; clear DF (forward)
    lea rsi, [rel src_bytes]   ; source
    lea rdi, [rel dst_buf]     ; destination
    mov ecx, 8
    rep movsb
    ; Check first 8 bytes: "ABCDEFGH"
    lea rax, [rel dst_buf]
    mov rax, [rax]
    mov rbx, 0x4847464544434241  ; "ABCDEFGH" in LE
    CHECK_EQ_64 rax, 0x4847464544434241

    ; ==== Test 2: REP STOSB - fill 16 bytes with 0xCC ====
    TEST_CASE t2_name
    cld
    lea rdi, [rel dst_buf]
    mov al, 0xCC
    mov ecx, 16
    rep stosb
    ; Check first 8 bytes: all 0xCC
    lea rax, [rel dst_buf]
    mov rax, [rax]
    CHECK_EQ_64 rax, 0xCCCCCCCCCCCCCCCC

    ; ==== Test 3: REPE CMPSB - compare equal strings ====
    ; Compares "Hello, World!\0" with itself
    ; Should compare all 14 bytes (including null) and ECX = 0
    TEST_CASE t3_name
    cld
    lea rsi, [rel src_hello]
    lea rdi, [rel src_hello]
    mov ecx, 14
    repe cmpsb
    ; All equal, ECX should be 0
    CHECK_EQ_32 ecx, 0

    ; ==== Test 4: REPE CMPSB - strings differ at position 7 ====
    ; "Hello, World!" vs "Hello, Xorld!"
    ; Differ at byte 7: 'W' (0x57) vs 'X' (0x58)
    ; REPE stops AFTER the mismatch byte is compared
    ; ECX = initial - number_compared = 14 - 8 = 6
    TEST_CASE t4_name
    cld
    lea rsi, [rel src_hello]
    lea rdi, [rel src_diff]
    mov ecx, 14
    repe cmpsb
    CHECK_EQ_32 ecx, 6

    ; ==== Test 5: REPNE SCASB - find 'X' in string ====
    ; "Find the X in here!" -> 'X' is at position 9
    ; REPNE stops when match is found (after comparing the matching byte)
    ; ECX = 20 - 10 = 10 (compared 10 bytes: positions 0-9)
    TEST_CASE t5_name
    cld
    lea rdi, [rel search_str]
    mov al, 'X'
    mov ecx, 20
    repne scasb
    CHECK_EQ_32 ecx, 10

    ; ==== Test 6: REP MOVSW - copy 4 words ====
    TEST_CASE t6_name
    ; Clear
    lea rdi, [rel dst_buf]
    xor eax, eax
    mov ecx, 16
    rep stosb
    ; Copy 4 words
    cld
    lea rsi, [rel src_words]
    lea rdi, [rel dst_buf]
    mov ecx, 4
    rep movsw
    ; Check: 0x1111, 0x2222, 0x3333, 0x4444
    ; Low 64 bits: 0x4444333322221111
    lea rax, [rel dst_buf]
    mov rax, [rax]
    CHECK_EQ_64 rax, 0x4444333322221111

    ; ==== Test 7: REP STOSD - fill 4 dwords ====
    TEST_CASE t7_name
    cld
    lea rdi, [rel dst_buf]
    mov eax, 0xDEADBEEF
    mov ecx, 4
    rep stosd
    ; Check first 8 bytes
    lea rax, [rel dst_buf]
    mov rax, [rax]
    CHECK_EQ_64 rax, 0xDEADBEEFDEADBEEF

    ; ==== Test 8: REP MOVSQ - copy 2 qwords ====
    TEST_CASE t8_name
    ; Clear
    lea rdi, [rel dst_buf]
    xor eax, eax
    mov ecx, 32
    rep stosb
    ; Copy
    cld
    lea rsi, [rel src_qwords]
    lea rdi, [rel dst_buf]
    mov ecx, 2
    rep movsq
    ; First qword = 0x1122334455667788
    lea rax, [rel dst_buf]
    mov rax, [rax]
    CHECK_EQ_64 rax, 0x1122334455667788

    ; ==== Test 9: REP STOSQ - fill 2 qwords ====
    TEST_CASE t9_name
    cld
    lea rdi, [rel dst_buf]
    mov rax, 0x0123456789ABCDEF
    mov ecx, 2
    rep stosq
    lea rax, [rel dst_buf]
    mov rax, [rax]
    CHECK_EQ_64 rax, 0x0123456789ABCDEF

    ; ==== Test 10: LODSB - load single byte ====
    TEST_CASE t10_name
    cld
    lea rsi, [rel src_bytes]   ; 'A' = 0x41
    xor eax, eax
    lodsb
    CHECK_EQ_32 eax, 0x41

    ; ==== Test 11: LODSW - load single word ====
    TEST_CASE t11_name
    cld
    lea rsi, [rel src_words]   ; 0x1111
    xor eax, eax
    lodsw
    CHECK_EQ_32 eax, 0x1111

    ; ==== Test 12: LODSD - load single dword ====
    TEST_CASE t12_name
    cld
    lea rsi, [rel src_dwords]  ; 0xAABBCCDD
    xor eax, eax
    lodsd
    CHECK_EQ_32 eax, 0xAABBCCDD

    ; ==== Test 13: LODSQ - load single qword ====
    TEST_CASE t13_name
    cld
    lea rsi, [rel src_qwords]  ; 0x1122334455667788
    xor rax, rax
    lodsq
    CHECK_EQ_64 rax, 0x1122334455667788

    ; ==== Test 14: REP MOVSB backward (DF=1) ====
    TEST_CASE t14_name
    ; Clear dst
    cld
    lea rdi, [rel dst_buf]
    xor eax, eax
    mov ecx, 32
    rep stosb
    ; Copy 8 bytes backward: src starts at byte 7, dst starts at byte 7
    std                        ; set DF (backward)
    lea rsi, [rel src_bytes + 7]    ; point to last byte 'H'
    lea rdi, [rel dst_buf + 7]      ; point to dst byte 7
    mov ecx, 8
    rep movsb
    cld                        ; restore DF
    ; dst[0..7] should be "ABCDEFGH" (same as forward, just copied in reverse order)
    lea rax, [rel dst_buf]
    mov rax, [rax]
    CHECK_EQ_64 rax, 0x4847464544434241

    ; ==== Test 15: REPNE SCASB - byte not found ====
    ; Search for 'Z' in "Find the X in here!" (not present in first 20 bytes)
    TEST_CASE t15_name
    cld
    lea rdi, [rel search_str]
    mov al, 'Z'
    mov ecx, 20
    repne scasb
    ; Not found -> ECX = 0 (exhausted count)
    CHECK_EQ_32 ecx, 0

    ; ==== Test 16: REPE CMPSW - compare equal words ====
    TEST_CASE t16_name
    cld
    lea rsi, [rel src_words]
    lea rdi, [rel src_words]
    mov ecx, 4
    repe cmpsw
    CHECK_EQ_32 ecx, 0

    ; ==== Test 17: REPE CMPSD - differ at dword 1 ====
    ; src_dwords: 0xAABBCCDD, 0x11223344, 0x55667788, 0x99AABBCC
    ; Compare with itself but modify one dword in dst
    TEST_CASE t17_name
    ; Copy src_dwords to dst_buf, then change dword 1
    cld
    lea rsi, [rel src_dwords]
    lea rdi, [rel dst_buf]
    mov ecx, 4
    rep movsd
    ; Modify dword 1 in dst
    lea rax, [rel dst_buf]
    mov dword [rax + 4], 0x99999999
    ; Now compare
    cld
    lea rsi, [rel src_dwords]
    lea rdi, [rel dst_buf]
    mov ecx, 4
    repe cmpsd
    ; Differ at dword 1 (0x11223344 vs 0x99999999)
    ; Compared dword 0 (match) + dword 1 (mismatch, stop) = 2 compared
    ; ECX = 4 - 2 = 2
    CHECK_EQ_32 ecx, 2

    ; ==== Test 18: REP MOVSB with count=0 (should be no-op) ====
    TEST_CASE t18_name
    ; Fill dst with 0xFF first
    cld
    lea rdi, [rel dst_buf]
    mov al, 0xFF
    mov ecx, 8
    rep stosb
    ; REP MOVSB with ECX=0
    cld
    lea rsi, [rel src_bytes]
    lea rdi, [rel dst_buf]
    xor ecx, ecx              ; count = 0
    rep movsb
    ; dst should still be 0xFF
    lea rax, [rel dst_buf]
    mov rax, [rax]
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFF

    ; ==== Test 19: REP STOSB with count=0 (should be no-op) ====
    TEST_CASE t19_name
    ; Fill dst with 0xAA
    cld
    lea rdi, [rel dst_buf]
    mov al, 0xAA
    mov ecx, 8
    rep stosb
    ; REP STOSB with ECX=0
    cld
    lea rdi, [rel dst_buf]
    mov al, 0x00
    xor ecx, ecx
    rep stosb
    ; dst should still be 0xAA
    lea rax, [rel dst_buf]
    mov rax, [rax]
    CHECK_EQ_64 rax, 0xAAAAAAAAAAAAAAAA

    ; ==== Test 20: REPNE SCASB - find byte at position 0 ====
    ; Search for 'F' at start of "Find..."
    TEST_CASE t20_name
    cld
    lea rdi, [rel search_str]
    mov al, 'F'
    mov ecx, 20
    repne scasb
    ; Found at position 0, compared 1 byte. ECX = 20 - 1 = 19
    CHECK_EQ_32 ecx, 19

    ; ==== Test 21: REP MOVSD - copy 4 dwords ====
    TEST_CASE t21_name
    ; Clear
    cld
    lea rdi, [rel dst_buf]
    xor eax, eax
    mov ecx, 32
    rep stosb
    ; Copy
    cld
    lea rsi, [rel src_dwords]
    lea rdi, [rel dst_buf]
    mov ecx, 4
    rep movsd
    ; Check first 8 bytes: dw0=0xAABBCCDD, dw1=0x11223344
    ; LE 64: 0x11223344AABBCCDD
    lea rax, [rel dst_buf]
    mov rax, [rax]
    CHECK_EQ_64 rax, 0x11223344AABBCCDD

    ; ==== Test 22: REP MOVSB overlapping forward ====
    ; Copy within same buffer: dst = src + 1 (forward overlap)
    ; This replicates byte 0 across the buffer
    TEST_CASE t22_name
    cld
    lea rdi, [rel dst_buf]
    mov al, 0x42               ; 'B'
    mov ecx, 1
    rep stosb
    ; Fill rest with 0
    xor al, al
    mov ecx, 15
    rep stosb
    ; Now REP MOVSB: src = dst_buf, dst = dst_buf+1, count = 7
    ; This copies byte-by-byte forward: each byte copies the previous
    ; Result: dst_buf[0..7] = all 0x42
    cld
    lea rsi, [rel dst_buf]
    lea rdi, [rel dst_buf + 1]
    mov ecx, 7
    rep movsb
    ; Check 8 bytes: all 0x42
    lea rax, [rel dst_buf]
    mov rax, [rax]
    CHECK_EQ_64 rax, 0x4242424242424242

    ; ==== Test 23: REP STOSB larger fill (64 bytes) ====
    TEST_CASE t23_name
    cld
    lea rdi, [rel dst_buf]
    mov al, 0x55
    mov ecx, 64
    rep stosb
    ; Check bytes at offset 56
    lea rax, [rel dst_buf + 56]
    mov rax, [rax]
    CHECK_EQ_64 rax, 0x5555555555555555

    ; ==== Test 24: REPE CMPSB - check ZF after all-equal comparison ====
    ; When all bytes match and ECX reaches 0, ZF should be 1 (last comparison was equal)
    TEST_CASE t24_name
    cld
    lea rsi, [rel src_hello]
    lea rdi, [rel src_hello]
    mov ecx, 5                 ; compare "Hello" with "Hello"
    repe cmpsb
    SAVE_FLAGS
    CHECK_FLAGS_EQ ZF, ZF      ; ZF should be set (last cmp was equal)

    ; ==== Test 25: REPE CMPSB - check ZF after mismatch ====
    ; When mismatch found, ZF should be 0
    TEST_CASE t25_name
    cld
    lea rsi, [rel src_hello]
    lea rdi, [rel src_diff]
    mov ecx, 14
    repe cmpsb
    SAVE_FLAGS
    CHECK_FLAGS_EQ ZF, 0       ; ZF should be clear (last cmp was not equal)

    END_TESTS
