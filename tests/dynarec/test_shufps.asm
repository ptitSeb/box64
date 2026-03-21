; test_shufps.asm - Comprehensive SHUFPS test for PPC64LE dynarec optimization
; Tests all optimization tiers:
;   Tier 0: Identity (same-reg, imm=0xE4)
;   Tier 1: Broadcast (same-reg, all selectors identical)
;   Tier 2: XXPERMDI (doubleword-aligned halves)
;   Tier 3a: General single-source (same-reg, GPR path)
;   Tier 3b: General two-source (VPERM path)
; Also tests memory operand (Ex from memory)
%include "test_framework.inc"

section .data
    ; ---- Tier 0: Identity ----
    tn_id_same:     db "shufps id same 0xE4", 0

    ; ---- Tier 1: Broadcast (same-reg) ----
    tn_bcast_w0:    db "shufps bcast w0 0x00", 0
    tn_bcast_w1:    db "shufps bcast w1 0x55", 0
    tn_bcast_w2:    db "shufps bcast w2 0xAA", 0
    tn_bcast_w3:    db "shufps bcast w3 0xFF", 0

    ; ---- Tier 2: XXPERMDI (same-reg) ----
    tn_pdi_44_s:    db "shufps 0x44 same", 0
    tn_pdi_4E_s:    db "shufps 0x4E same", 0
    tn_pdi_E4_s:    db "shufps 0xE4 same(id)", 0
    tn_pdi_EE_s:    db "shufps 0xEE same", 0

    ; ---- Tier 2: XXPERMDI (two-source) ----
    tn_pdi_44_2:    db "shufps 0x44 two-src", 0
    tn_pdi_4E_2:    db "shufps 0x4E two-src", 0
    tn_pdi_E4_2:    db "shufps 0xE4 two-src", 0
    tn_pdi_EE_2:    db "shufps 0xEE two-src", 0

    ; ---- Tier 3a: General single-source ----
    tn_gen_1B_s:    db "shufps 0x1B same", 0
    tn_gen_39_s:    db "shufps 0x39 same", 0
    tn_gen_C0_s:    db "shufps 0xC0 same", 0
    tn_gen_D5_s:    db "shufps 0xD5 same", 0
    tn_gen_DB_s:    db "shufps 0xDB same", 0
    tn_gen_EA_s:    db "shufps 0xEA same", 0
    tn_gen_F1_s:    db "shufps 0xF1 same", 0
    tn_gen_B1_s:    db "shufps 0xB1 same", 0
    tn_gen_93_s:    db "shufps 0x93 same", 0
    tn_gen_E0_s:    db "shufps 0xE0 same", 0
    tn_gen_E5_s:    db "shufps 0xE5 same", 0

    ; ---- Tier 3b: General two-source ----
    tn_gen_00_2:    db "shufps 0x00 two-src", 0
    tn_gen_1B_2:    db "shufps 0x1B two-src", 0
    tn_gen_24_2:    db "shufps 0x24 two-src", 0
    tn_gen_20_2:    db "shufps 0x20 two-src", 0
    tn_gen_C0_2:    db "shufps 0xC0 two-src", 0
    tn_gen_D5_2:    db "shufps 0xD5 two-src", 0
    tn_gen_DB_2:    db "shufps 0xDB two-src", 0
    tn_gen_EA_2:    db "shufps 0xEA two-src", 0
    tn_gen_F1_2:    db "shufps 0xF1 two-src", 0
    tn_gen_FF_2:    db "shufps 0xFF two-src", 0

    ; ---- Memory operand tests ----
    tn_mem_E4:      db "shufps 0xE4 mem", 0
    tn_mem_00:      db "shufps 0x00 mem", 0
    tn_mem_1B:      db "shufps 0x1B mem", 0
    tn_mem_24:      db "shufps 0x24 mem", 0

    align 16
    ; Source data: Gx = {1.0, 2.0, 3.0, 4.0}
    ;   word0=0x3F800000(1.0) word1=0x40000000(2.0) word2=0x40400000(3.0) word3=0x40800000(4.0)
    gx_data:    dd 0x3F800000, 0x40000000, 0x40400000, 0x40800000

    ; Source data: Ex = {5.0, 6.0, 7.0, 8.0}
    ;   word0=0x40A00000(5.0) word1=0x40C00000(6.0) word2=0x40E00000(7.0) word3=0x41000000(8.0)
    ex_data:    dd 0x40A00000, 0x40C00000, 0x40E00000, 0x41000000

    ; For memory operand tests, keep a copy
    ex_mem:     dd 0x40A00000, 0x40C00000, 0x40E00000, 0x41000000

; ============================================================
; Helper: define word constants for readability
; Gx words: w0=0x3F800000, w1=0x40000000, w2=0x40400000, w3=0x40800000
; Ex words: w0=0x40A00000, w1=0x40C00000, w2=0x40E00000, w3=0x41000000
; ============================================================
; Result is packed as: low64 = (word1 << 32) | word0, high64 = (word3 << 32) | word2
; CHECK_EQ_XMM_LOW64 checks low 64 bits, pextrq checks high 64 bits

; Macro to check full 128-bit XMM result (low64, high64)
%macro CHECK_XMM128 3   ; %1=xmm, %2=expected_low64, %3=expected_high64
    ; Check low 64 bits
    CHECK_EQ_XMM_LOW64 %1, %2
    ; Check high 64 bits
    inc r13d
    push rax
    push rsi
    pextrq rax, %1, 1
    mov rsi, %3
    cmp rax, rsi
    jne %%fail_hi
    lea rsi, [rel _fw_pass_msg]
    call _fw_print_str
    inc r12d
    jmp %%done_hi
%%fail_hi:
    lea rsi, [rel _fw_fail_msg]
    call _fw_print_str
    lea rsi, [rel _fw_expect_msg]
    call _fw_print_str
    mov rax, %3
    call _fw_print_hex64
    lea rsi, [rel _fw_got_msg]
    call _fw_print_str
    pextrq rax, %1, 1
    call _fw_print_hex64
    lea rsi, [rel _fw_nl]
    call _fw_print_str
%%done_hi:
    pop rsi
    pop rax
%endmacro

section .text
global _start

_start:
    INIT_TESTS

    ; ================================================================
    ; Tier 0: Identity (same-reg, imm=0xE4)
    ; shufps xmm0, xmm0, 0xE4 → [w0,w1,w2,w3] = no change
    ; ================================================================
    TEST_CASE tn_id_same
    movdqa xmm0, [rel gx_data]
    shufps xmm0, xmm0, 0xE4
    ; Expected: {1.0, 2.0, 3.0, 4.0} = original
    CHECK_XMM128 xmm0, 0x400000003F800000, 0x4080000040400000

    ; ================================================================
    ; Tier 1: Broadcast word0 (imm=0x00)
    ; All 4 selectors = 0 → [w0,w0,w0,w0]
    ; ================================================================
    TEST_CASE tn_bcast_w0
    movdqa xmm0, [rel gx_data]
    shufps xmm0, xmm0, 0x00
    ; {1.0, 1.0, 1.0, 1.0}
    CHECK_XMM128 xmm0, 0x3F8000003F800000, 0x3F8000003F800000

    ; Tier 1: Broadcast word1 (imm=0x55)
    TEST_CASE tn_bcast_w1
    movdqa xmm0, [rel gx_data]
    shufps xmm0, xmm0, 0x55
    ; {2.0, 2.0, 2.0, 2.0}
    CHECK_XMM128 xmm0, 0x4000000040000000, 0x4000000040000000

    ; Tier 1: Broadcast word2 (imm=0xAA)
    TEST_CASE tn_bcast_w2
    movdqa xmm0, [rel gx_data]
    shufps xmm0, xmm0, 0xAA
    ; {3.0, 3.0, 3.0, 3.0}
    CHECK_XMM128 xmm0, 0x4040000040400000, 0x4040000040400000

    ; Tier 1: Broadcast word3 (imm=0xFF)
    TEST_CASE tn_bcast_w3
    movdqa xmm0, [rel gx_data]
    shufps xmm0, xmm0, 0xFF
    ; {4.0, 4.0, 4.0, 4.0}
    CHECK_XMM128 xmm0, 0x4080000040800000, 0x4080000040800000

    ; ================================================================
    ; Tier 2: XXPERMDI same-reg cases
    ; ================================================================

    ; 0x44 same: sel0=0,sel1=1,sel2=0,sel3=1 → [w0,w1,w0,w1] = dup low half
    TEST_CASE tn_pdi_44_s
    movdqa xmm0, [rel gx_data]
    shufps xmm0, xmm0, 0x44
    CHECK_XMM128 xmm0, 0x400000003F800000, 0x400000003F800000

    ; 0x4E same: sel0=2,sel1=3,sel2=0,sel3=1 → [w2,w3,w0,w1] = swap halves
    TEST_CASE tn_pdi_4E_s
    movdqa xmm0, [rel gx_data]
    shufps xmm0, xmm0, 0x4E
    CHECK_XMM128 xmm0, 0x4080000040400000, 0x400000003F800000

    ; 0xE4 same: identity (already tested as Tier 0, but check via Tier 2 path)
    TEST_CASE tn_pdi_E4_s
    movdqa xmm0, [rel gx_data]
    shufps xmm0, xmm0, 0xE4
    CHECK_XMM128 xmm0, 0x400000003F800000, 0x4080000040400000

    ; 0xEE same: sel0=2,sel1=3,sel2=2,sel3=3 → [w2,w3,w2,w3] = dup high half
    TEST_CASE tn_pdi_EE_s
    movdqa xmm0, [rel gx_data]
    shufps xmm0, xmm0, 0xEE
    CHECK_XMM128 xmm0, 0x4080000040400000, 0x4080000040400000

    ; ================================================================
    ; Tier 2: XXPERMDI two-source cases
    ; Gx={1,2,3,4}, Ex={5,6,7,8}
    ; shufps: low from Gx, high from Ex
    ; ================================================================

    ; 0x44 two-src: [Gx.w0,Gx.w1,Ex.w0,Ex.w1] = {1,2,5,6}
    TEST_CASE tn_pdi_44_2
    movdqa xmm0, [rel gx_data]
    movdqa xmm1, [rel ex_data]
    shufps xmm0, xmm1, 0x44
    CHECK_XMM128 xmm0, 0x400000003F800000, 0x40C0000040A00000

    ; 0x4E two-src: [Gx.w2,Gx.w3,Ex.w0,Ex.w1] = {3,4,5,6}
    TEST_CASE tn_pdi_4E_2
    movdqa xmm0, [rel gx_data]
    movdqa xmm1, [rel ex_data]
    shufps xmm0, xmm1, 0x4E
    CHECK_XMM128 xmm0, 0x4080000040400000, 0x40C0000040A00000

    ; 0xE4 two-src: [Gx.w0,Gx.w1,Ex.w2,Ex.w3] = {1,2,7,8}
    TEST_CASE tn_pdi_E4_2
    movdqa xmm0, [rel gx_data]
    movdqa xmm1, [rel ex_data]
    shufps xmm0, xmm1, 0xE4
    CHECK_XMM128 xmm0, 0x400000003F800000, 0x4100000040E00000

    ; 0xEE two-src: [Gx.w2,Gx.w3,Ex.w2,Ex.w3] = {3,4,7,8}
    TEST_CASE tn_pdi_EE_2
    movdqa xmm0, [rel gx_data]
    movdqa xmm1, [rel ex_data]
    shufps xmm0, xmm1, 0xEE
    CHECK_XMM128 xmm0, 0x4080000040400000, 0x4100000040E00000

    ; ================================================================
    ; Tier 3a: General single-source (Gx==Ex)
    ; ================================================================

    ; 0x1B same: reverse [w3,w2,w1,w0]
    TEST_CASE tn_gen_1B_s
    movdqa xmm0, [rel gx_data]
    shufps xmm0, xmm0, 0x1B
    ; {4.0, 3.0, 2.0, 1.0}
    CHECK_XMM128 xmm0, 0x4040000040800000, 0x3F80000040000000

    ; 0x39 same: rotate left by 1 [w1,w2,w3,w0]
    ; sel0=1,sel1=2,sel2=3,sel3=0
    TEST_CASE tn_gen_39_s
    movdqa xmm0, [rel gx_data]
    shufps xmm0, xmm0, 0x39
    CHECK_XMM128 xmm0, 0x4040000040000000, 0x3F80000040800000

    ; 0xC0 same: sel0=0,sel1=0,sel2=0,sel3=3 → [w0,w0,w0,w3]
    TEST_CASE tn_gen_C0_s
    movdqa xmm0, [rel gx_data]
    shufps xmm0, xmm0, 0xC0
    CHECK_XMM128 xmm0, 0x3F8000003F800000, 0x408000003F800000

    ; 0xD5 same: sel0=1,sel1=1,sel2=1,sel3=3 → [w1,w1,w1,w3]
    TEST_CASE tn_gen_D5_s
    movdqa xmm0, [rel gx_data]
    shufps xmm0, xmm0, 0xD5
    CHECK_XMM128 xmm0, 0x4000000040000000, 0x4080000040000000

    ; 0xDB same: sel0=3,sel1=2,sel2=1,sel3=3 → [w3,w2,w1,w3]
    TEST_CASE tn_gen_DB_s
    movdqa xmm0, [rel gx_data]
    shufps xmm0, xmm0, 0xDB
    CHECK_XMM128 xmm0, 0x4040000040800000, 0x4080000040000000

    ; 0xEA same: sel0=2,sel1=2,sel2=2,sel3=3 → [w2,w2,w2,w3]
    TEST_CASE tn_gen_EA_s
    movdqa xmm0, [rel gx_data]
    shufps xmm0, xmm0, 0xEA
    CHECK_XMM128 xmm0, 0x4040000040400000, 0x4080000040400000

    ; 0xF1 same: sel0=1,sel1=0,sel2=3,sel3=3 → [w1,w0,w3,w3]
    TEST_CASE tn_gen_F1_s
    movdqa xmm0, [rel gx_data]
    shufps xmm0, xmm0, 0xF1
    CHECK_XMM128 xmm0, 0x3F80000040000000, 0x4080000040800000

    ; 0xB1 same: sel0=1,sel1=0,sel2=3,sel3=2 → [w1,w0,w3,w2]  (swap within halves)
    TEST_CASE tn_gen_B1_s
    movdqa xmm0, [rel gx_data]
    shufps xmm0, xmm0, 0xB1
    CHECK_XMM128 xmm0, 0x3F80000040000000, 0x4040000040800000

    ; 0x93 same: sel0=3,sel1=0,sel2=1,sel3=2 → [w3,w0,w1,w2]
    TEST_CASE tn_gen_93_s
    movdqa xmm0, [rel gx_data]
    shufps xmm0, xmm0, 0x93
    CHECK_XMM128 xmm0, 0x3F80000040800000, 0x4040000040000000

    ; 0xE0 same: sel0=0,sel1=0,sel2=2,sel3=3 → [w0,w0,w2,w3]
    TEST_CASE tn_gen_E0_s
    movdqa xmm0, [rel gx_data]
    shufps xmm0, xmm0, 0xE0
    CHECK_XMM128 xmm0, 0x3F8000003F800000, 0x4080000040400000

    ; 0xE5 same: sel0=1,sel1=1,sel2=2,sel3=3 → [w1,w1,w2,w3]
    TEST_CASE tn_gen_E5_s
    movdqa xmm0, [rel gx_data]
    shufps xmm0, xmm0, 0xE5
    CHECK_XMM128 xmm0, 0x4000000040000000, 0x4080000040400000

    ; ================================================================
    ; Tier 3b: General two-source (VPERM path)
    ; Gx={1,2,3,4}  Ex={5,6,7,8}
    ; shufps: result[0,1] from Gx, result[2,3] from Ex
    ; ================================================================

    ; 0x00 two-src: [Gx.w0,Gx.w0,Ex.w0,Ex.w0] = {1,1,5,5}
    TEST_CASE tn_gen_00_2
    movdqa xmm0, [rel gx_data]
    movdqa xmm1, [rel ex_data]
    shufps xmm0, xmm1, 0x00
    CHECK_XMM128 xmm0, 0x3F8000003F800000, 0x40A0000040A00000

    ; 0x1B two-src: [Gx.w3,Gx.w2,Ex.w1,Ex.w0] = {4,3,6,5}
    TEST_CASE tn_gen_1B_2
    movdqa xmm0, [rel gx_data]
    movdqa xmm1, [rel ex_data]
    shufps xmm0, xmm1, 0x1B
    CHECK_XMM128 xmm0, 0x4040000040800000, 0x40A0000040C00000

    ; 0x24 two-src: sel0=0,sel1=1,sel2=2,sel3=0 → [Gx.w0,Gx.w1,Ex.w2,Ex.w0] = {1,2,7,5}
    TEST_CASE tn_gen_24_2
    movdqa xmm0, [rel gx_data]
    movdqa xmm1, [rel ex_data]
    shufps xmm0, xmm1, 0x24
    CHECK_XMM128 xmm0, 0x400000003F800000, 0x40A0000040E00000

    ; 0x20 two-src: sel0=0,sel1=0,sel2=2,sel3=0 → [Gx.w0,Gx.w0,Ex.w2,Ex.w0] = {1,1,7,5}
    TEST_CASE tn_gen_20_2
    movdqa xmm0, [rel gx_data]
    movdqa xmm1, [rel ex_data]
    shufps xmm0, xmm1, 0x20
    CHECK_XMM128 xmm0, 0x3F8000003F800000, 0x40A0000040E00000

    ; 0xC0 two-src: sel0=0,sel1=0,sel2=0,sel3=3 → [Gx.w0,Gx.w0,Ex.w0,Ex.w3] = {1,1,5,8}
    TEST_CASE tn_gen_C0_2
    movdqa xmm0, [rel gx_data]
    movdqa xmm1, [rel ex_data]
    shufps xmm0, xmm1, 0xC0
    CHECK_XMM128 xmm0, 0x3F8000003F800000, 0x4100000040A00000

    ; 0xD5 two-src: sel0=1,sel1=1,sel2=1,sel3=3 → [Gx.w1,Gx.w1,Ex.w1,Ex.w3] = {2,2,6,8}
    TEST_CASE tn_gen_D5_2
    movdqa xmm0, [rel gx_data]
    movdqa xmm1, [rel ex_data]
    shufps xmm0, xmm1, 0xD5
    CHECK_XMM128 xmm0, 0x4000000040000000, 0x4100000040C00000

    ; 0xDB two-src: sel0=3,sel1=2,sel2=1,sel3=3 → [Gx.w3,Gx.w2,Ex.w1,Ex.w3] = {4,3,6,8}
    TEST_CASE tn_gen_DB_2
    movdqa xmm0, [rel gx_data]
    movdqa xmm1, [rel ex_data]
    shufps xmm0, xmm1, 0xDB
    CHECK_XMM128 xmm0, 0x4040000040800000, 0x4100000040C00000

    ; 0xEA two-src: sel0=2,sel1=2,sel2=2,sel3=3 → [Gx.w2,Gx.w2,Ex.w2,Ex.w3] = {3,3,7,8}
    TEST_CASE tn_gen_EA_2
    movdqa xmm0, [rel gx_data]
    movdqa xmm1, [rel ex_data]
    shufps xmm0, xmm1, 0xEA
    CHECK_XMM128 xmm0, 0x4040000040400000, 0x4100000040E00000

    ; 0xF1 two-src: sel0=1,sel1=0,sel2=3,sel3=3 → [Gx.w1,Gx.w0,Ex.w3,Ex.w3] = {2,1,8,8}
    TEST_CASE tn_gen_F1_2
    movdqa xmm0, [rel gx_data]
    movdqa xmm1, [rel ex_data]
    shufps xmm0, xmm1, 0xF1
    CHECK_XMM128 xmm0, 0x3F80000040000000, 0x4100000041000000

    ; 0xFF two-src: sel0=3,sel1=3,sel2=3,sel3=3 → [Gx.w3,Gx.w3,Ex.w3,Ex.w3] = {4,4,8,8}
    TEST_CASE tn_gen_FF_2
    movdqa xmm0, [rel gx_data]
    movdqa xmm1, [rel ex_data]
    shufps xmm0, xmm1, 0xFF
    CHECK_XMM128 xmm0, 0x4080000040800000, 0x4100000041000000

    ; ================================================================
    ; Memory operand tests (Ex loaded from memory)
    ; ================================================================

    ; 0xE4 mem: [Gx.w0,Gx.w1,mem.w2,mem.w3] = {1,2,7,8}
    TEST_CASE tn_mem_E4
    movdqa xmm0, [rel gx_data]
    shufps xmm0, [rel ex_mem], 0xE4
    CHECK_XMM128 xmm0, 0x400000003F800000, 0x4100000040E00000

    ; 0x00 mem: [Gx.w0,Gx.w0,mem.w0,mem.w0] = {1,1,5,5}
    TEST_CASE tn_mem_00
    movdqa xmm0, [rel gx_data]
    shufps xmm0, [rel ex_mem], 0x00
    CHECK_XMM128 xmm0, 0x3F8000003F800000, 0x40A0000040A00000

    ; 0x1B mem: [Gx.w3,Gx.w2,mem.w1,mem.w0] = {4,3,6,5}
    TEST_CASE tn_mem_1B
    movdqa xmm0, [rel gx_data]
    shufps xmm0, [rel ex_mem], 0x1B
    CHECK_XMM128 xmm0, 0x4040000040800000, 0x40A0000040C00000

    ; 0x24 mem: [Gx.w0,Gx.w1,mem.w2,mem.w0] = {1,2,7,5}
    TEST_CASE tn_mem_24
    movdqa xmm0, [rel gx_data]
    shufps xmm0, [rel ex_mem], 0x24
    CHECK_XMM128 xmm0, 0x400000003F800000, 0x40A0000040E00000

    END_TESTS
