; test_misc_flags.asm - Test miscellaneous flag/register manipulation instructions
; LAHF, SAHF, PUSHF/POPF, ENTER/LEAVE, XLAT, MOVBE, CBW/CWDE/CDQE, CWD/CDQ/CQO
%include "test_framework.inc"

section .data
    t1_name:  db "lahf after xor eax,eax", 0
    t2_name:  db "lahf after stc", 0
    t3_name:  db "lahf after sub (SF=1)", 0
    t4_name:  db "sahf all flags set", 0
    t5_name:  db "sahf all flags clear", 0
    t6_name:  db "sahf CF only", 0
    t7_name:  db "pushf/popf restore", 0
    t8_name:  db "pushf/popf OF+SF+AF", 0
    t9_name:  db "enter/leave rbp", 0
    t10_name: db "enter/leave rsp", 0
    t11_name: db "enter 0,0 frame", 0
    t12_name: db "xlatb basic", 0
    t13_name: db "xlatb index 0", 0
    t14_name: db "xlatb index 255", 0
    t15_name: db "movbe r32,m32", 0
    t16_name: db "movbe r64,m64", 0
    t17_name: db "movbe m32,r32", 0
    t18_name: db "movbe m64,r64", 0
    t19_name: db "cbw positive", 0
    t20_name: db "cbw negative", 0
    t21_name: db "cwde positive", 0
    t22_name: db "cwde negative", 0
    t23_name: db "cdqe positive", 0
    t24_name: db "cdqe negative", 0
    t25_name: db "cwd positive", 0
    t26_name: db "cwd negative", 0
    t27_name: db "cdq positive", 0
    t28_name: db "cdq negative", 0
    t29_name: db "cqo positive", 0
    t30_name: db "cqo negative", 0

    ; XLAT lookup table: table[i] = 0xFF - i
    align 16
    xlat_table:
    %assign i 0
    %rep 256
        db (0xFF - i)
    %assign i i+1
    %endrep

    ; MOVBE test data
    align 8
    movbe_dword_src: dd 0x01020304
    movbe_qword_src: dq 0x0102030405060708

section .bss
    alignb 8
    scratch1: resq 1
    scratch2: resq 1
    movbe_dword_dst: resd 1
    movbe_qword_dst: resq 1

section .text
global _start

_start:
    INIT_TESTS

    ; ================================================================
    ; LAHF tests
    ; ================================================================

    ; ==== Test 1: lahf after xor eax,eax ====
    ; xor eax,eax -> ZF=1, PF=1, SF=0, CF=0, AF=0
    ; FLAGS low byte = 0b01000110 = 0x46
    ; LAHF loads FLAGS[7:0] into AH
    TEST_CASE t1_name
    xor eax, eax           ; sets ZF=1, PF=1, SF=0, CF=0, AF=0
    lahf                    ; AH = FLAGS[7:0] = 0x46
    mov cl, ah
    movzx ecx, cl
    CHECK_EQ_32 ecx, 0x46

    ; ==== Test 2: lahf after stc ====
    ; stc sets CF=1, other flags unchanged from xor above
    ; FLAGS low byte = 0x46 | 0x01 = 0x47
    TEST_CASE t2_name
    xor eax, eax           ; ZF=1, PF=1 => FLAGS[7:0] = 0x46
    stc                     ; CF=1 => FLAGS[7:0] = 0x47
    lahf                    ; AH = 0x47
    mov cl, ah
    movzx ecx, cl
    CHECK_EQ_32 ecx, 0x47

    ; ==== Test 3: lahf after sub producing SF=1 ====
    ; mov al, 1; sub al, 2 -> result = 0xFF (-1 as i8)
    ; SF=1 (bit7=1), ZF=0, CF=1 (borrow), AF=1 (nibble borrow), PF=1 (0xFF has 8 bits=even parity)
    ; FLAGS[7:0] = SF(0x80)|AF(0x10)|PF(0x04)|CF(0x01)|reserved_bit1(0x02)
    ;            = 0x80|0x10|0x04|0x02|0x01 = 0x97
    TEST_CASE t3_name
    mov al, 1
    sub al, 2               ; AL = 0xFF, SF=1, CF=1, AF=1, PF=1, ZF=0
    lahf
    mov cl, ah
    movzx ecx, cl
    CHECK_EQ_32 ecx, 0x97

    ; ================================================================
    ; SAHF tests
    ; ================================================================

    ; ==== Test 4: sahf with all flags set ====
    ; AH = 0xD7 = 1101_0111 => SF=1, ZF=1, AF=1, PF=1, CF=1
    ; (bit5 and bit3 are ignored by CPU in flags, bit1 is always 1)
    ; Check SF|ZF|AF|PF|CF mask = 0x80|0x40|0x10|0x04|0x01 = 0xD5
    TEST_CASE t4_name
    mov ah, 0xD7
    sahf
    SAVE_FLAGS
    CHECK_FLAGS_EQ 0xD5, 0xD5

    ; ==== Test 5: sahf all flags clear ====
    ; AH = 0x02 (only reserved bit1 set, all user flags = 0)
    ; SF=0, ZF=0, AF=0, PF=0, CF=0
    TEST_CASE t5_name
    mov ah, 0x02
    sahf
    SAVE_FLAGS
    CHECK_FLAGS_EQ 0xD5, 0x00

    ; ==== Test 6: sahf CF only ====
    ; AH = 0x03 => CF=1, others clear (bit1 reserved = 1)
    TEST_CASE t6_name
    mov ah, 0x03
    sahf
    SAVE_FLAGS
    CHECK_FLAGS_EQ 0xD5, 0x01

    ; ================================================================
    ; PUSHF / POPF tests
    ; ================================================================

    ; ==== Test 7: pushf/popf restore flags ====
    ; Set CF=1 and ZF=1 via arithmetic: xor eax,eax sets ZF=1,PF=1,CF=0
    ; then stc sets CF=1
    ; pushf, modify flags (sub to clear ZF/CF), popf, verify original restored
    TEST_CASE t7_name
    xor eax, eax           ; ZF=1, PF=1, CF=0
    stc                     ; CF=1
    pushfq
    ; Now clobber flags
    mov eax, 5
    sub eax, 3              ; changes flags: ZF=0, CF=0, etc.
    popfq                   ; restore original flags
    SAVE_FLAGS
    ; Should have ZF=1 and CF=1 restored
    CHECK_FLAGS_EQ (ZF|CF), (ZF|CF)

    ; ==== Test 8: pushf/popf with OF+SF+AF ====
    ; 0x7FFFFFFF + 1 = 0x80000000
    ; OF=1, SF=1, ZF=0, CF=0, AF=1, PF=0
    TEST_CASE t8_name
    mov eax, 0x7FFFFFFF
    add eax, 1              ; OF=1, SF=1, AF=1, CF=0, ZF=0, PF=0
    pushfq
    xor eax, eax            ; clobber flags
    popfq
    SAVE_FLAGS
    ; Check OF|SF|AF = 0x800|0x80|0x10 = 0x890
    ; Mask = CF|PF|AF|ZF|SF|OF = 0x8D5
    CHECK_FLAGS_EQ 0x8D5, 0x890

    ; ================================================================
    ; ENTER / LEAVE tests
    ; ================================================================

    ; ==== Test 9: enter 32,0 / leave — check rbp ====
    ; ENTER 32,0: push rbp; mov rbp,rsp; sub rsp,32
    ; After enter: new_rbp = old_rsp - 8
    ; LEAVE: mov rsp,rbp; pop rbp => restores original rbp
    TEST_CASE t9_name
    mov rax, rsp            ; save original rsp
    mov rbx, rbp            ; save original rbp
    enter 32, 0
    mov rcx, rbp            ; new rbp = old_rsp - 8
    leave
    ; Verify new_rbp == old_rsp - 8
    sub rcx, rax            ; rcx = new_rbp - old_rsp = -8
    CHECK_EQ_64 rcx, 0xFFFFFFFFFFFFFFF8

    ; ==== Test 10: enter 32,0 / leave — check rsp ====
    ; After enter: new_rsp = new_rbp - 32 = old_rsp - 8 - 32 = old_rsp - 40
    TEST_CASE t10_name
    mov rax, rsp            ; save original rsp
    mov rbx, rbp            ; save original rbp
    enter 32, 0
    mov rdx, rsp            ; new rsp
    leave
    ; Verify new_rsp == old_rsp - 40
    sub rdx, rax            ; rdx = new_rsp - old_rsp = -40
    CHECK_EQ_64 rdx, 0xFFFFFFFFFFFFFFD8

    ; ==== Test 11: enter 0,0 / leave — zero-size frame ====
    ; ENTER 0,0: push rbp; mov rbp,rsp; sub rsp,0
    ; So new_rbp = old_rsp - 8 and new_rsp = new_rbp = old_rsp - 8
    TEST_CASE t11_name
    mov rax, rsp            ; save original rsp
    mov rbx, rbp            ; save original rbp
    enter 0, 0
    mov rcx, rsp            ; new rsp should equal new rbp
    mov rdx, rbp            ; new rbp
    leave
    ; new_rsp == new_rbp
    sub rcx, rdx            ; should be 0
    CHECK_EQ_64 rcx, 0

    ; ================================================================
    ; XLATB tests
    ; ================================================================

    ; ==== Test 12: xlatb basic (index=5) ====
    ; table[5] = 0xFF - 5 = 0xFA
    TEST_CASE t12_name
    lea rbx, [rel xlat_table]
    mov al, 5
    xlatb                   ; AL = [RBX + AL] = table[5] = 0xFA
    movzx ecx, al
    CHECK_EQ_32 ecx, 0xFA

    ; ==== Test 13: xlatb index 0 ====
    ; table[0] = 0xFF
    TEST_CASE t13_name
    lea rbx, [rel xlat_table]
    mov al, 0
    xlatb
    movzx ecx, al
    CHECK_EQ_32 ecx, 0xFF

    ; ==== Test 14: xlatb index 255 ====
    ; table[255] = 0xFF - 255 = 0x00
    TEST_CASE t14_name
    lea rbx, [rel xlat_table]
    mov al, 255
    xlatb
    movzx ecx, al
    CHECK_EQ_32 ecx, 0x00

    ; ================================================================
    ; MOVBE tests
    ; ================================================================

    ; ==== Test 15: movbe r32, [mem] — byte swap dword load ====
    ; Memory contains 0x01020304 (LE bytes: 04 03 02 01)
    ; MOVBE loads and byte-swaps: result = 0x04030201
    TEST_CASE t15_name
    movbe eax, [rel movbe_dword_src]
    CHECK_EQ_32 eax, 0x04030201

    ; ==== Test 16: movbe r64, [mem] — byte swap qword load ====
    ; Memory contains 0x0102030405060708 (LE bytes: 08 07 06 05 04 03 02 01)
    ; MOVBE loads and byte-swaps: result = 0x0807060504030201
    TEST_CASE t16_name
    movbe rax, [rel movbe_qword_src]
    CHECK_EQ_64 rax, 0x0807060504030201

    ; ==== Test 17: movbe [mem], r32 — byte swap dword store ====
    ; Put 0x01020304 in eax, MOVBE [mem], eax => stores byte-swapped
    ; Then normal load should read 0x04030201
    TEST_CASE t17_name
    mov eax, 0x01020304
    movbe [rel movbe_dword_dst], eax
    mov ecx, [rel movbe_dword_dst]
    CHECK_EQ_32 ecx, 0x04030201

    ; ==== Test 18: movbe [mem], r64 — byte swap qword store ====
    ; Put 0x0102030405060708 in rax, MOVBE [mem], rax => stores byte-swapped
    ; Normal load should read 0x0807060504030201
    TEST_CASE t18_name
    mov rax, 0x0102030405060708
    movbe [rel movbe_qword_dst], rax
    mov rcx, [rel movbe_qword_dst]
    CHECK_EQ_64 rcx, 0x0807060504030201

    ; ================================================================
    ; CBW / CWDE / CDQE tests
    ; ================================================================

    ; ==== Test 19: cbw positive (AL=0x7F -> AX=0x007F) ====
    TEST_CASE t19_name
    xor eax, eax
    mov al, 0x7F
    cbw                     ; sign-extend AL to AX
    movzx ecx, ax
    CHECK_EQ_32 ecx, 0x007F

    ; ==== Test 20: cbw negative (AL=0x80 -> AX=0xFF80) ====
    TEST_CASE t20_name
    xor eax, eax
    mov al, 0x80
    cbw                     ; sign-extend AL to AX: 0xFF80
    movzx ecx, ax
    CHECK_EQ_32 ecx, 0xFF80

    ; ==== Test 21: cwde positive (AX=0x7FFF -> EAX=0x00007FFF) ====
    TEST_CASE t21_name
    xor eax, eax
    mov ax, 0x7FFF
    cwde                    ; sign-extend AX to EAX
    CHECK_EQ_32 eax, 0x00007FFF

    ; ==== Test 22: cwde negative (AX=0x8000 -> EAX=0xFFFF8000) ====
    TEST_CASE t22_name
    xor eax, eax
    mov ax, 0x8000
    cwde                    ; sign-extend AX to EAX: 0xFFFF8000
    CHECK_EQ_32 eax, 0xFFFF8000

    ; ==== Test 23: cdqe positive (EAX=0x7FFFFFFF -> RAX=0x000000007FFFFFFF) ====
    TEST_CASE t23_name
    mov eax, 0x7FFFFFFF
    cdqe                    ; sign-extend EAX to RAX
    CHECK_EQ_64 rax, 0x000000007FFFFFFF

    ; ==== Test 24: cdqe negative (EAX=0x80000000 -> RAX=0xFFFFFFFF80000000) ====
    TEST_CASE t24_name
    mov eax, 0x80000000
    cdqe                    ; sign-extend EAX to RAX: 0xFFFFFFFF80000000
    CHECK_EQ_64 rax, 0xFFFFFFFF80000000

    ; ================================================================
    ; CWD / CDQ / CQO tests
    ; ================================================================

    ; ==== Test 25: cwd positive (AX=0x7FFF -> DX=0x0000) ====
    TEST_CASE t25_name
    xor edx, edx
    mov ax, 0x7FFF
    cwd                     ; sign-extend AX into DX:AX
    movzx ecx, dx
    CHECK_EQ_32 ecx, 0x0000

    ; ==== Test 26: cwd negative (AX=0x8000 -> DX=0xFFFF) ====
    TEST_CASE t26_name
    xor edx, edx
    mov ax, 0x8000
    cwd                     ; DX = 0xFFFF
    movzx ecx, dx
    CHECK_EQ_32 ecx, 0xFFFF

    ; ==== Test 27: cdq positive (EAX=0x7FFFFFFF -> EDX=0x00000000) ====
    TEST_CASE t27_name
    mov eax, 0x7FFFFFFF
    cdq                     ; sign-extend EAX into EDX:EAX
    CHECK_EQ_32 edx, 0x00000000

    ; ==== Test 28: cdq negative (EAX=0x80000000 -> EDX=0xFFFFFFFF) ====
    TEST_CASE t28_name
    mov eax, 0x80000000
    cdq                     ; EDX = 0xFFFFFFFF
    CHECK_EQ_32 edx, 0xFFFFFFFF

    ; ==== Test 29: cqo positive (RAX=0x7FFFFFFFFFFFFFFF -> RDX=0) ====
    TEST_CASE t29_name
    mov rax, 0x7FFFFFFFFFFFFFFF
    cqo                     ; sign-extend RAX into RDX:RAX
    CHECK_EQ_64 rdx, 0x0000000000000000

    ; ==== Test 30: cqo negative (RAX=0x8000000000000000 -> RDX=0xFFFFFFFFFFFFFFFF) ====
    TEST_CASE t30_name
    mov rax, 0x8000000000000000
    cqo                     ; RDX = 0xFFFFFFFFFFFFFFFF
    CHECK_EQ_64 rdx, 0xFFFFFFFFFFFFFFFF

    END_TESTS
