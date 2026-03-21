; test_updateflags.asm - Test deferred flag materialization across basic block boundaries
;
; PURPOSE: Verify that UpdateFlags (or native updateflags) correctly computes
; x86 flags from deferred op1/op2/res values when flags must be materialized
; at basic block boundaries.
;
; TECHNIQUE: Each test uses one of these patterns to force a block boundary
; between the flag-setting instruction and the flag-reading instruction:
;   1. Indirect call through register (call rax) — forces new dynablock
;   2. Indirect jump through register (jmp rax) — forces new dynablock
;   3. Function call (call label) — basic block boundary at call/ret
;
; The called function does nothing that modifies flags (just ret or push/pop),
; so the flags must survive across the boundary via the deferred mechanism.
;
; COVERAGE: Tests the top df types from PoE profiling:
;   - d_cmp32  (96.99%) — CMP r32, r/m32 and CMP r32, imm
;   - d_tst32  ( 1.35%) — TEST r32, r/m32
;   - d_cmp64  ( 0.48%) — CMP r64, r/m64
;   - d_tst8   ( 0.33%) — TEST r8, r/m8
;   - d_tst64  ( 0.25%) — TEST r64, r/m64
;   - d_cmp8   ( 0.19%) — CMP r8, imm8
;   - d_cmp16  ( 0.16%) — CMP r16, r/m16
;   - d_sub32  (bonus)  — SUB r32, r/m32 (sets flags like CMP but modifies dst)
;   - d_add32  (bonus)  — ADD r32, r/m32
;   - d_and32  (bonus)  — AND r32, r/m32 (logic flags)
;   - d_or32   (bonus)  — OR  r32, r/m32
;   - d_xor32  (bonus)  — XOR r32, r/m32
;
; For each operation, we test multiple flag scenarios:
;   - ZF=1 (zero result)
;   - SF=1 (negative/sign result)
;   - CF=1 (carry/borrow)
;   - OF=1 (overflow)
;   - PF (parity)
;   - Combined flags
;
%include "test_framework.inc"

section .data
    ; --- d_cmp32 tests ---
    tc32_1: db "cmp32 equal (ZF) via call", 0
    tc32_2: db "cmp32 less (CF+SF) via call", 0
    tc32_3: db "cmp32 greater (no CF/ZF) via call", 0
    tc32_4: db "cmp32 overflow (OF) via call", 0
    tc32_5: db "cmp32 zero vs neg via call", 0
    tc32_6: db "cmp32 equal via indirect call", 0
    tc32_7: db "cmp32 carry via indirect jmp", 0
    tc32_8: db "cmp32 all flags check via call", 0

    ; --- d_tst32 tests ---
    tt32_1: db "test32 zero (ZF) via call", 0
    tt32_2: db "test32 nonzero via call", 0
    tt32_3: db "test32 sign (SF) via call", 0
    tt32_4: db "test32 parity (PF) via call", 0

    ; --- d_cmp64 tests ---
    tc64_1: db "cmp64 equal (ZF) via call", 0
    tc64_2: db "cmp64 less (CF) via call", 0
    tc64_3: db "cmp64 overflow (OF) via call", 0

    ; --- d_tst8 tests ---
    tt8_1:  db "test8 zero (ZF) via call", 0
    tt8_2:  db "test8 sign (SF) via call", 0
    tt8_3:  db "test8 nonzero via call", 0

    ; --- d_tst64 tests ---
    tt64_1: db "test64 zero (ZF) via call", 0
    tt64_2: db "test64 sign (SF) via call", 0

    ; --- d_cmp8 tests ---
    tc8_1:  db "cmp8 equal (ZF) via call", 0
    tc8_2:  db "cmp8 less (CF) via call", 0
    tc8_3:  db "cmp8 sign (SF) via call", 0

    ; --- d_cmp16 tests ---
    tc16_1: db "cmp16 equal (ZF) via call", 0
    tc16_2: db "cmp16 less (CF) via call", 0
    tc16_3: db "cmp16 overflow (OF) via call", 0

    ; --- d_sub32 tests ---
    ts32_1: db "sub32 zero (ZF) via call", 0
    ts32_2: db "sub32 borrow (CF) via call", 0
    ts32_3: db "sub32 overflow (OF) via call", 0

    ; --- d_add32 tests ---
    ta32_1: db "add32 carry (CF) via call", 0
    ta32_2: db "add32 overflow (OF) via call", 0
    ta32_3: db "add32 zero (ZF) via call", 0

    ; --- d_and32/or32/xor32 tests ---
    tl32_1: db "and32 zero (ZF) via call", 0
    tl32_2: db "or32 sign (SF) via call", 0
    tl32_3: db "xor32 zero (ZF) via call", 0
    tl32_4: db "and32 parity (PF) via call", 0

    ; --- cross-boundary conditional branch tests ---
    ; These test that jcc works correctly when flags were deferred
    txb_1:  db "cmp32 -> call -> jz taken", 0
    txb_2:  db "cmp32 -> call -> jz not taken", 0
    txb_3:  db "cmp32 -> call -> jl taken", 0
    txb_4:  db "cmp32 -> call -> jg taken", 0
    txb_5:  db "test32 -> call -> js taken", 0
    txb_6:  db "cmp32 -> call -> jo taken", 0

    ; --- loop / multi-iteration tests ---
    ; Ensure UpdateFlags is consistent across many iterations
    tml_1:  db "cmp32 loop 1000 iters via call", 0
    tml_2:  db "test32 loop 1000 iters via call", 0

section .bss
    scratch: resq 4
    loop_counter: resd 1
    jmp_target_addr: resq 1

section .text
global _start

;; ============================================================
;; Helper functions that create basic block boundaries
;; These do NOT modify flags (no arithmetic/logic/cmp/test)
;; ============================================================

; Simple return - forces call/ret block boundary
; Uses only stack manipulation which doesn't affect RFLAGS arithmetic flags
flag_barrier_ret:
    ret

; A slightly larger function that still preserves flags
; Push/pop don't affect arithmetic flags
flag_barrier_nop:
    push rbx
    mov rbx, rbx        ; nop-like, doesn't affect flags
    pop rbx
    ret

; Function that does memory store without affecting flags
; (mov to memory doesn't affect flags)
flag_barrier_store:
    push rbx
    lea rbx, [rel scratch]
    mov [rbx], rdi       ; store arg, no flag change
    pop rbx
    ret


_start:
    INIT_TESTS

    ;; ================================================================
    ;; d_cmp32 tests — CMP r32, r/m32 (96.99% of UpdateFlags calls)
    ;; ================================================================

    ; --- Test: cmp32 equal → ZF=1, CF=0, SF=0, OF=0 ---
    TEST_CASE tc32_1
    mov eax, 42
    cmp eax, 42                  ; 42 - 42 = 0 → ZF=1
    call flag_barrier_ret        ; ← basic block boundary
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|SF|OF), ZF

    ; --- Test: cmp32 less → CF=1, SF=1 (unsigned: src < dst, signed: positive < bigger positive) ---
    TEST_CASE tc32_2
    mov eax, 10
    cmp eax, 20                  ; 10 - 20 = -10 → CF=1, SF=1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|SF), (CF|SF)

    ; --- Test: cmp32 greater → CF=0, ZF=0 ---
    TEST_CASE tc32_3
    mov eax, 100
    cmp eax, 50                  ; 100 - 50 = 50 → CF=0, ZF=0, SF=0
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|ZF|SF), 0

    ; --- Test: cmp32 signed overflow → OF=1 ---
    ; 0x80000000 - 1 = 0x7FFFFFFF → signed overflow (neg - pos = pos)
    TEST_CASE tc32_4
    mov eax, 0x80000000
    cmp eax, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ OF, OF

    ; --- Test: cmp32 zero vs negative ---
    ; 0 - 0xFFFFFFFF: unsigned: 0 < 0xFFFFFFFF → CF=1
    TEST_CASE tc32_5
    mov eax, 0
    mov ebx, 0xFFFFFFFF
    cmp eax, ebx
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ; --- Test: cmp32 equal via indirect call ---
    TEST_CASE tc32_6
    lea rax, [rel flag_barrier_ret]
    mov ecx, 99
    cmp ecx, 99                  ; ZF=1
    call rax                     ; ← indirect call, stronger block boundary
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|SF|OF), ZF

    ; --- Test: cmp32 carry via indirect jump ---
    TEST_CASE tc32_7
    lea rax, [rel .tc32_7_target]
    mov ecx, 5
    cmp ecx, 100                 ; 5 - 100 → CF=1, SF=1
    jmp rax                      ; ← indirect jump
.tc32_7_target:
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|SF), (CF|SF)

    ; --- Test: cmp32 comprehensive all-flags check ---
    ; 0xFF - 0x01 = 0xFE (as 32-bit: 0xFF - 0x01 = 0xFE)
    ; CF=0, ZF=0, SF=0, OF=0, AF=1 (borrow from bit 4), PF=0 (0xFE has 7 bits → odd)
    TEST_CASE tc32_8
    mov eax, 0xFF
    cmp eax, 0x01
    call flag_barrier_nop
    SAVE_FLAGS
    CHECK_FLAGS_EQ FLAGS_MASK_ARITH, 0


    ;; ================================================================
    ;; d_tst32 tests — TEST r32, r/m32 (1.35%)
    ;; ================================================================

    ; --- Test: test32 zero → ZF=1, SF=0, CF=0, OF=0 ---
    TEST_CASE tt32_1
    mov eax, 0xF0F0F0F0
    mov ebx, 0x0F0F0F0F
    test eax, ebx                ; AND = 0 → ZF=1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|SF|CF|OF), ZF

    ; --- Test: test32 nonzero → ZF=0 ---
    TEST_CASE tt32_2
    mov eax, 0xFF
    test eax, 0x0F               ; AND = 0x0F → ZF=0, SF=0
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|SF|CF|OF), 0

    ; --- Test: test32 sign → SF=1 ---
    TEST_CASE tt32_3
    mov eax, 0x80000000
    test eax, eax                ; AND = 0x80000000 → SF=1, ZF=0
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (SF|ZF|CF|OF), SF

    ; --- Test: test32 parity → PF check ---
    ; 0x03 AND 0x03 = 0x03 (2 bits set → even parity → PF=1)
    TEST_CASE tt32_4
    mov eax, 0x03
    test eax, 0x03
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ PF, PF


    ;; ================================================================
    ;; d_cmp64 tests — CMP r64, r/m64 (0.48%)
    ;; ================================================================

    ; --- Test: cmp64 equal → ZF=1 ---
    TEST_CASE tc64_1
    mov rax, 0x123456789ABCDEF0
    mov rbx, 0x123456789ABCDEF0
    cmp rax, rbx
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|SF|OF), ZF

    ; --- Test: cmp64 less → CF=1 ---
    TEST_CASE tc64_2
    mov rax, 1
    mov rbx, 0x7FFFFFFFFFFFFFFF
    cmp rax, rbx                 ; 1 - huge → CF=1, SF=1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|SF), (CF|SF)

    ; --- Test: cmp64 signed overflow → OF=1 ---
    ; 0x8000000000000000 - 1 → signed overflow
    TEST_CASE tc64_3
    mov rax, 0x8000000000000000
    mov rbx, 1
    cmp rax, rbx
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ OF, OF


    ;; ================================================================
    ;; d_tst8 tests — TEST r8, r/m8 (0.33%)
    ;; ================================================================

    ; --- Test: test8 zero → ZF=1 ---
    TEST_CASE tt8_1
    mov al, 0xF0
    test al, 0x0F                ; AND = 0 → ZF=1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|SF|CF|OF), ZF

    ; --- Test: test8 sign → SF=1 ---
    TEST_CASE tt8_2
    mov al, 0x80
    test al, al                  ; AND = 0x80 → SF=1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (SF|ZF|CF|OF), SF

    ; --- Test: test8 nonzero → ZF=0 ---
    TEST_CASE tt8_3
    mov al, 0x55
    test al, 0x0F                ; AND = 0x05 → ZF=0
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|OF), 0


    ;; ================================================================
    ;; d_tst64 tests — TEST r64, r/m64 (0.25%)
    ;; ================================================================

    ; --- Test: test64 zero → ZF=1 ---
    TEST_CASE tt64_1
    mov rax, 0xAAAAAAAAAAAAAAAA
    mov rbx, 0x5555555555555555
    test rax, rbx                ; AND = 0 → ZF=1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|SF|CF|OF), ZF

    ; --- Test: test64 sign → SF=1 ---
    TEST_CASE tt64_2
    mov rax, 0x8000000000000001
    test rax, rax                ; SF=1, ZF=0
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (SF|ZF|CF|OF), SF


    ;; ================================================================
    ;; d_cmp8 tests — CMP r8, imm8 (0.19%)
    ;; ================================================================

    ; --- Test: cmp8 equal → ZF=1 ---
    TEST_CASE tc8_1
    mov al, 0x7F
    cmp al, 0x7F
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|SF|OF), ZF

    ; --- Test: cmp8 less → CF=1 ---
    TEST_CASE tc8_2
    mov al, 0x10
    cmp al, 0x80                 ; 0x10 - 0x80 → CF=1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ; --- Test: cmp8 sign → SF=1 ---
    ; 0x01 - 0x02 = 0xFF → SF=1
    TEST_CASE tc8_3
    mov al, 0x01
    cmp al, 0x02
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ SF, SF


    ;; ================================================================
    ;; d_cmp16 tests — CMP r16, r/m16 (0.16%)
    ;; ================================================================

    ; --- Test: cmp16 equal → ZF=1 ---
    TEST_CASE tc16_1
    mov ax, 0x1234
    cmp ax, 0x1234
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|SF|OF), ZF

    ; --- Test: cmp16 less → CF=1 ---
    TEST_CASE tc16_2
    mov ax, 0x0010
    mov bx, 0xFF00
    cmp ax, bx                   ; 0x10 - 0xFF00 → CF=1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ; --- Test: cmp16 overflow → OF=1 ---
    ; 0x8000 - 1 → signed: -32768 - 1 = 32767, signed overflow
    TEST_CASE tc16_3
    mov ax, 0x8000
    cmp ax, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ OF, OF


    ;; ================================================================
    ;; d_sub32 tests — SUB r32, r/m32
    ;; ================================================================

    ; --- Test: sub32 zero result → ZF=1 ---
    TEST_CASE ts32_1
    mov eax, 42
    sub eax, 42                  ; 42 - 42 = 0 → ZF=1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|SF|OF), ZF

    ; --- Test: sub32 borrow → CF=1 ---
    TEST_CASE ts32_2
    mov eax, 0
    sub eax, 1                   ; 0 - 1 → CF=1, SF=1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|SF), (CF|SF)

    ; --- Test: sub32 signed overflow → OF=1 ---
    ; 0x7FFFFFFF - (-1) = 0x7FFFFFFF + 1 = 0x80000000 → OF=1
    TEST_CASE ts32_3
    mov eax, 0x7FFFFFFF
    mov ebx, 0xFFFFFFFF          ; -1 in signed
    sub eax, ebx
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ OF, OF


    ;; ================================================================
    ;; d_add32 tests — ADD r32, r/m32
    ;; ================================================================

    ; --- Test: add32 carry → CF=1 ---
    TEST_CASE ta32_1
    mov eax, 0xFFFFFFFF
    add eax, 1                   ; wraps to 0 → CF=1, ZF=1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (CF|ZF), (CF|ZF)

    ; --- Test: add32 signed overflow → OF=1 ---
    ; 0x7FFFFFFF + 1 = 0x80000000 → OF=1
    TEST_CASE ta32_2
    mov eax, 0x7FFFFFFF
    add eax, 1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ OF, OF

    ; --- Test: add32 zero result (impossible without CF, but 0+0=0) ---
    TEST_CASE ta32_3
    xor eax, eax
    xor ebx, ebx
    add eax, ebx                 ; 0 + 0 = 0 → ZF=1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|SF|OF), ZF


    ;; ================================================================
    ;; d_and32/or32/xor32 tests — logic operations
    ;; CF=0 and OF=0 always for logic ops
    ;; ================================================================

    ; --- Test: and32 zero → ZF=1, CF=0, OF=0 ---
    TEST_CASE tl32_1
    mov eax, 0xFF00FF00
    and eax, 0x00FF00FF          ; result = 0 → ZF=1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|OF), ZF

    ; --- Test: or32 sign → SF=1, CF=0, OF=0 ---
    TEST_CASE tl32_2
    mov eax, 0x80000000
    or eax, 0x00000001           ; result has bit 31 set → SF=1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (SF|CF|OF), SF

    ; --- Test: xor32 self → ZF=1, CF=0, OF=0 ---
    TEST_CASE tl32_3
    mov eax, 0xDEADBEEF
    xor eax, eax                 ; result = 0 → ZF=1
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF|OF), ZF

    ; --- Test: and32 parity check ---
    ; 0xFF AND 0x0F = 0x0F (4 bits set → even parity → PF=1)
    TEST_CASE tl32_4
    mov eax, 0xFF
    and eax, 0x0F
    call flag_barrier_ret
    SAVE_FLAGS
    CHECK_FLAGS_EQ PF, PF


    ;; ================================================================
    ;; Cross-boundary conditional branch tests
    ;; These test: flag_set → call → jcc (flag used in different block)
    ;; ================================================================

    ; --- Test: cmp32 → call → jz taken (equal) ---
    TEST_CASE txb_1
    mov eax, 50
    cmp eax, 50                  ; ZF=1
    call flag_barrier_ret
    jz .txb1_taken               ; should be taken
    ; not taken path — FAIL
    SAVE_FLAGS
    CHECK_FLAGS_EQ 0, 0xFFFF     ; force fail
    jmp .txb1_done
.txb1_taken:
    ; correct path — pass
    SAVE_FLAGS
    CHECK_FLAGS_EQ ZF, ZF
.txb1_done:

    ; --- Test: cmp32 → call → jz not taken (not equal) ---
    TEST_CASE txb_2
    mov eax, 50
    cmp eax, 51                  ; ZF=0
    call flag_barrier_ret
    jz .txb2_bad                 ; should NOT be taken
    ; correct path — not taken
    SAVE_FLAGS
    CHECK_FLAGS_EQ ZF, 0
    jmp .txb2_done
.txb2_bad:
    SAVE_FLAGS
    CHECK_FLAGS_EQ 0, 0xFFFF     ; force fail
.txb2_done:

    ; --- Test: cmp32 → call → jl taken (less, signed) ---
    TEST_CASE txb_3
    mov eax, -5
    cmp eax, 10                  ; -5 < 10, signed → jl taken
    call flag_barrier_ret
    jl .txb3_taken
    SAVE_FLAGS
    CHECK_FLAGS_EQ 0, 0xFFFF     ; force fail
    jmp .txb3_done
.txb3_taken:
    SAVE_FLAGS
    ; SF != OF for jl
    ; -5 (0xFFFFFFFB) - 10 = 0xFFFFFFF1, SF=1, OF=0 → SF!=OF → jl taken
    CHECK_FLAGS_EQ SF, SF
.txb3_done:

    ; --- Test: cmp32 → call → jg taken (greater, signed) ---
    TEST_CASE txb_4
    mov eax, 100
    cmp eax, 50                  ; 100 > 50, signed → jg taken
    call flag_barrier_ret
    jg .txb4_taken
    SAVE_FLAGS
    CHECK_FLAGS_EQ 0, 0xFFFF     ; force fail
    jmp .txb4_done
.txb4_taken:
    SAVE_FLAGS
    ; ZF=0, SF==OF for jg
    CHECK_FLAGS_EQ (ZF|SF|OF), 0
.txb4_done:

    ; --- Test: test32 → call → js taken (sign bit set) ---
    TEST_CASE txb_5
    mov eax, 0x80000000
    test eax, eax                ; SF=1
    call flag_barrier_ret
    js .txb5_taken
    SAVE_FLAGS
    CHECK_FLAGS_EQ 0, 0xFFFF     ; force fail
    jmp .txb5_done
.txb5_taken:
    SAVE_FLAGS
    CHECK_FLAGS_EQ SF, SF
.txb5_done:

    ; --- Test: cmp32 → call → jo taken (overflow) ---
    TEST_CASE txb_6
    mov eax, 0x80000000
    cmp eax, 1                   ; signed overflow → OF=1
    call flag_barrier_ret
    jo .txb6_taken
    SAVE_FLAGS
    CHECK_FLAGS_EQ 0, 0xFFFF     ; force fail
    jmp .txb6_done
.txb6_taken:
    SAVE_FLAGS
    CHECK_FLAGS_EQ OF, OF
.txb6_done:


    ;; ================================================================
    ;; Loop tests — verify UpdateFlags consistency over many iterations
    ;; If the native updateflags has a subtle bug (e.g., register
    ;; corruption), it may only manifest after many calls
    ;; ================================================================

    ; --- Test: cmp32 across call in tight loop, 1000 iterations ---
    ; Each iteration: set ecx to loop var, cmp ecx, 500
    ;   - When ecx < 500: CF=1, SF=1, ZF=0
    ;   - When ecx = 500: CF=0, SF=0, ZF=1
    ;   - When ecx > 500: CF=0, SF=0, ZF=0
    ; We count how many times ZF=1 (should be exactly 1: when ecx=500)
    TEST_CASE tml_1
    xor ebp, ebp                 ; ebp = ZF-true counter
    mov ecx, 0                   ; loop counter
.tml1_loop:
    push rcx                     ; save loop counter
    cmp ecx, 500                 ; flag-setting instruction
    call flag_barrier_ret        ; basic block boundary
    SAVE_FLAGS
    mov rax, r14
    and rax, ZF
    test rax, rax
    jz .tml1_no_zf
    inc ebp                      ; count ZF=1 hits
.tml1_no_zf:
    pop rcx
    inc ecx
    cmp ecx, 1000
    jl .tml1_loop
    ; ebp should be exactly 1 (only when ecx was 500)
    CHECK_EQ_32 ebp, 1

    ; --- Test: test32 across call in tight loop, 1000 iterations ---
    ; Each iteration: test (iter & 0x80000000)
    ;   - For iter 0..999, bit 31 is never set, so SF=0 always
    ;   - ZF=1 when (iter & 0xFF) == 0 (i.e., iter=0,256,512,768 → 4 times)
    TEST_CASE tml_2
    xor ebp, ebp                 ; ZF counter
    mov ecx, 0
.tml2_loop:
    push rcx
    mov eax, ecx
    test eax, 0xFF               ; test low byte
    call flag_barrier_ret
    SAVE_FLAGS
    mov rax, r14
    and rax, ZF
    test rax, rax
    jz .tml2_no_zf
    inc ebp
.tml2_no_zf:
    pop rcx
    inc ecx
    cmp ecx, 1000
    jl .tml2_loop
    ; ZF=1 when (iter & 0xFF) == 0: iter=0,256,512,768 → 4 times
    CHECK_EQ_32 ebp, 4


    ;; ================================================================
    ;; Done
    ;; ================================================================
    END_TESTS
