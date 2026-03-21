; test_sse_string.asm - Test SSE4.2 string comparison operations
; PCMPISTRI, PCMPISTRM (implicit-length string compare)
; PCMPESTRI, PCMPESTRM (explicit-length string compare)
;
; imm8 encoding:
;   [1:0] = source data format: 00=unsigned bytes, 01=unsigned words, 10=signed bytes, 11=signed words
;   [3:2] = aggregation: 00=equal-any, 01=ranges, 10=equal-each, 11=equal-ordered
;   [5:4] = polarity: 00=positive, 01=negative, 10=masked+, 11=masked-
;   [6]   = output: 0=least significant index, 1=most significant index (PCMPISTRI)
;            or 0=bit mask, 1=byte/word mask (PCMPISTRM)
%include "test_framework.inc"

section .data
    t1_name:  db "pcmpistri equal-each found", 0
    t2_name:  db "pcmpistri equal-each not found", 0
    t3_name:  db "pcmpistri equal-any strchr", 0
    t4_name:  db "pcmpistri ranges alpha", 0
    t5_name:  db "pcmpistri equal-ordered substr", 0
    t6_name:  db "pcmpistri equal-each MSB index", 0
    t7_name:  db "pcmpistrm equal-each bitmask", 0
    t8_name:  db "pcmpistrm equal-each bytemask", 0
    t9_name:  db "pcmpistri equal-each ZF/SF", 0
    t10_name: db "pcmpistri null at pos 5", 0
    t11_name: db "pcmpestri equal-each", 0
    t12_name: db "pcmpestrm equal-each bitmask", 0
    t13_name: db "pcmpistri ranges digit", 0
    t14_name: db "pcmpistri equal-any charset", 0
    t15_name: db "pcmpistri equal-ordered no match", 0
    t16_name: db "pcmpistrm equal-any bitmask", 0
    t17_name: db "pcmpistri neg polarity", 0
    t18_name: db "pcmpistri all-null src", 0
    t19_name: db "pcmpestri len=0", 0
    t20_name: db "pcmpistri ranges multi", 0
    t21_name: db "pcmpistrm ranges bytemask", 0
    t22_name: db "pcmpistri equal-each partial", 0
    t23_name: db "pcmpestri equal-ordered", 0
    t24_name: db "pcmpistrm neg polarity", 0
    t25_name: db "pcmpistri equal-each CF/ZF", 0

    align 16
    ; Test strings (null-terminated, 16 bytes each padded with zeros)
    ; "Hello World!\0\0\0\0"
    str_hello:   db "Hello World!", 0, 0, 0, 0
    ; "Hello World!\0\0\0\0" (same for equal-each)
    str_hello2:  db "Hello World!", 0, 0, 0, 0
    ; "Goodbye!!!!!\0\0\0\0"
    str_goodbye: db "Goodbye!!!!!", 0, 0, 0, 0
    ; "abcdefghijklmnop" (full 16 bytes, no null)
    str_abcd16:  db "abcdefghijklmnop"
    ; "abcdef\0\0\0\0\0\0\0\0\0\0"
    str_abcdef:  db "abcdef", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    ; "xyzab\0..." - contains 'a' at position 3 and 'b' at position 4
    str_xyzab:   db "xyzab", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    ; Search chars for equal-any: "aeiou\0..."
    str_vowels:  db "aeiou", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    ; Range pair for alpha check: "azAZ\0..."
    ; Ranges: [a-z] [A-Z] -> bytes: 'a','z','A','Z'
    str_az_range: db "azAZ", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    ; Range pair for digit check: "09\0..."
    str_09_range: db "09", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    ; Substring to search: "lo W\0..."
    str_substr:  db "lo W", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    ; Substring not present: "xyz\0..."
    str_xyz:     db "xyz", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    ; "12345\0..."
    str_12345:   db "12345", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    ; "abc12xyz\0..."
    str_abc12:   db "abc12xyz", 0, 0, 0, 0, 0, 0, 0, 0
    ; All null
    str_null16:  db 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    ; "HeLLo\0..." (partial match with Hello)
    str_heLLo:   db "HeLLo World!", 0, 0, 0, 0
    ; Charset "!@#$%\0..."
    str_charset: db "!@#$%", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    ; "Hello!\0..." for charset test
    str_hellob:  db "Hello!", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    ; "abcXYZ123\0..."
    str_mixed:   db "abcXYZ123", 0, 0, 0, 0, 0, 0, 0
    ; Range for alpha+digit: "azAZ09\0..."
    str_alnum_range: db "azAZ09", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

section .text
global _start

_start:
    INIT_TESTS

    ; ==== Test 1: pcmpistri equal-each, strings match -> ECX = 16 (no mismatch) ====
    ; imm8 = 0x18: unsigned bytes (00), equal-each (10 << 2 = 0x08), positive polarity (00), LSB index (0)
    ; Actually: equal-each = bits[3:2]=10 -> 0x08. Unsigned bytes = 00. Polarity positive = 00. Output LSB = 0.
    ; imm8 = 0x08
    TEST_CASE t1_name
    movdqa xmm0, [rel str_hello]
    movdqa xmm1, [rel str_hello2]
    pcmpistri xmm0, xmm1, 0x18    ; equal-each, unsigned bytes, neg polarity -> find first MISMATCH
    ; All bytes match (including nulls after pos 12), so ECX = 16 (no mismatch found)
    ; Wait - polarity 01 (negative) inverts results. Let me use polarity 00 (positive) for equal-each:
    ; Actually let me reconsider. pcmpistri with equal-each + positive polarity:
    ;   IntRes1 bit[i] = 1 if src1[i] == src2[i]
    ;   ECX = index of least significant set bit in IntRes2
    ;   With positive polarity, matching bytes give 1.
    ;   First match is at position 0 (both are 'H')
    ; So ECX should be 0 for the first match position.
    ; But that's not useful for strcmp. For strcmp we want first MISMATCH (negative polarity).
    ; Let me just use 0x18 (equal-each, negative polarity) to find first mismatch:
    ;   imm8 = 0x18: [1:0]=00 (unsigned bytes), [3:2]=10 (equal-each), [5:4]=01 (negative), [6]=0 (LSB)
    ;   IntRes2[i] = NOT IntRes1[i] = 1 where bytes differ
    ;   All match -> IntRes2 = all 0 -> ECX = 16
    CHECK_EQ_32 ecx, 16

    ; ==== Test 2: pcmpistri equal-each mismatch at pos 0 ====
    ; "Hello..." vs "Goodbye..." -> first diff at position 0
    TEST_CASE t2_name
    movdqa xmm0, [rel str_hello]
    movdqa xmm1, [rel str_goodbye]
    pcmpistri xmm0, xmm1, 0x18    ; equal-each, neg polarity, unsigned bytes, LSB
    ; H != G at position 0, so IntRes2 bit 0 = 1, ECX = 0
    CHECK_EQ_32 ecx, 0

    ; ==== Test 3: pcmpistri equal-any (strchr-like): find first vowel ====
    ; imm8 = 0x00: unsigned bytes, equal-any, positive polarity, LSB index
    ; xmm0 = charset (vowels), xmm1 = string to search
    TEST_CASE t3_name
    movdqa xmm0, [rel str_vowels]      ; "aeiou"
    movdqa xmm1, [rel str_hello]       ; "Hello World!"
    pcmpistri xmm0, xmm1, 0x00    ; equal-any, positive, LSB
    ; Searching "Hello World!" for any of {a,e,i,o,u}
    ; H(0) no, e(1) YES -> first match at index 1
    CHECK_EQ_32 ecx, 1

    ; ==== Test 4: pcmpistri ranges - check for alpha chars ====
    ; imm8 = 0x04: unsigned bytes, ranges, positive polarity, LSB
    ; Range pairs in xmm0: 'a','z','A','Z' -> matches [a-z] and [A-Z]
    TEST_CASE t4_name
    movdqa xmm0, [rel str_az_range]    ; "azAZ"
    movdqa xmm1, [rel str_12345]       ; "12345"
    pcmpistri xmm0, xmm1, 0x44    ; ranges, unsigned bytes, neg polarity, LSB
    ; "12345" - none are in [a-z] or [A-Z], so with positive polarity all bits 0
    ; With negative polarity (0x14): IntRes2 = NOT IntRes1 (but only up to valid length)
    ; Wait, let me reconsider. 0x44 = bits: [6]=1(MSB), [5:4]=00(pos), [3:2]=01(ranges), [1:0]=00(ubytes)
    ; Let me use 0x04: ranges, positive polarity, LSB index
    ; "12345": '1'=0x31, not in [0x61-0x7A] or [0x41-0x5A], so no match
    ; ECX = 16 (no match found in valid range)
    ; Actually let me just test with alpha string:
    movdqa xmm0, [rel str_az_range]    ; "azAZ"
    movdqa xmm1, [rel str_abc12]       ; "abc12xyz"
    pcmpistri xmm0, xmm1, 0x04    ; ranges, positive, LSB
    ; a(0) in [a-z]? yes. b(1) yes. c(2) yes. 1(3) no. 2(4) no. x(5) yes. y(6) yes. z(7) yes.
    ; First match = position 0
    CHECK_EQ_32 ecx, 0

    ; ==== Test 5: pcmpistri equal-ordered (substring search) ====
    ; imm8 = 0x0C: unsigned bytes, equal-ordered, positive polarity, LSB
    ; xmm0 = needle ("lo W"), xmm1 = haystack ("Hello World!")
    TEST_CASE t5_name
    movdqa xmm0, [rel str_substr]      ; "lo W"
    movdqa xmm1, [rel str_hello]       ; "Hello World!"
    pcmpistri xmm0, xmm1, 0x0C    ; equal-ordered, positive, LSB
    ; Looking for "lo W" in "Hello World!"
    ; At pos 3: 'l' matches 'l', 'o' matches 'o', ' ' matches ' ', 'W' matches 'W' -> match at 3
    CHECK_EQ_32 ecx, 3

    ; ==== Test 6: pcmpistri equal-each with MSB index ====
    ; imm8 = 0x58: unsigned bytes, equal-each, neg polarity, MSB index
    ; Find LAST mismatch position
    TEST_CASE t6_name
    movdqa xmm0, [rel str_hello]       ; "Hello World!\0\0\0\0"
    movdqa xmm1, [rel str_heLLo]       ; "HeLLo World!\0\0\0\0"
    pcmpistri xmm0, xmm1, 0x58    ; equal-each, neg polarity, MSB
    ; Mismatches: pos 2 ('l' vs 'L'), pos 3 ('l' vs 'L')
    ; IntRes2 bits 2 and 3 set. MSB index = 3
    CHECK_EQ_32 ecx, 3

    ; ==== Test 7: pcmpistrm equal-each bit mask ====
    ; imm8 = 0x08: unsigned bytes, equal-each, positive polarity, bit mask
    TEST_CASE t7_name
    movdqa xmm0, [rel str_hello]
    movdqa xmm1, [rel str_hello2]
    pcmpistrm xmm0, xmm1, 0x08    ; equal-each, positive, bit mask
    ; All bytes match -> all bits set for valid positions (0-12), null terminator and beyond also match
    ; Actually implicit-length: valid up to first null in either string
    ; Both have null at pos 12. Bytes 0-11 are 'Hello World!' and match.
    ; Byte 12 is null in both -> still compared? Yes, implicit null at same position means match.
    ; Bits 0-12 should be 1 (matching), bits 13-15 forced to 0 (past null)
    ; Wait: SSE4.2 implicit-length: valid bytes are those before the first null.
    ; If null at pos 12 in both, valid range is [0..11] for both.
    ; Byte 12 and beyond: IntRes1 forced to 0.
    ; Actually the spec says: for equal-each, if both are invalid (past null), result is forced to 1... no.
    ; Let me check: for equal-each:
    ;   If xmm1[i] is invalid AND xmm0[i] is invalid: IntRes1[i] = 1 (both past end)
    ;   If only one is invalid: IntRes1[i] = 0
    ;   If both valid: IntRes1[i] = (xmm0[i] == xmm1[i])
    ; So bits 0-11: all 1 (both valid, match). Bit 12: both have null -> both invalid after, but pos 12 is the null itself.
    ; Hmm, "valid" means the byte is part of the string (before null terminator). The null byte at pos 12 is actually the terminator.
    ; In Intel's definition, a byte is "valid" if it comes before the null terminator.
    ; So pos 0-11 are valid, pos 12-15 are invalid (pos 12 is the null, which terminates the string).
    ; Both invalid at 12-15 -> IntRes1[12..15] = 1 for equal-each.
    ; Result mask = 0xFFFF for equal-each when strings match including past-end.
    ; Let me just check low 16 bits:
    movq rax, xmm0
    ; Bit mask is in the low 16 bits of xmm0
    ; Should be 0xFFFF
    and eax, 0xFFFF
    CHECK_EQ_32 eax, 0xFFFF

    ; ==== Test 8: pcmpistrm equal-each byte mask ====
    ; imm8 = 0x48: unsigned bytes, equal-each, positive polarity, byte mask
    TEST_CASE t8_name
    movdqa xmm0, [rel str_hello]
    movdqa xmm1, [rel str_heLLo]
    pcmpistrm xmm0, xmm1, 0x48    ; equal-each, positive, byte mask
    ; Matches at 0,1,4,5,6,7,8,9,10,11 (and past-null: 12,13,14,15)
    ; Mismatches at 2,3 ('l' vs 'L', 'l' vs 'L')
    ; Byte mask: matching positions get 0xFF, non-matching get 0x00
    ; Byte 2 = 0x00, byte 3 = 0x00
    ; Low 64 bits: bytes 0,1 = 0xFF, byte 2 = 0x00, byte 3 = 0x00, bytes 4-7 = 0xFF
    ; = FF FF 00 00 FF FF FF FF -> as little-endian qword: 0xFFFFFFFF0000FFFF
    movq rax, xmm0
    mov rbx, 0xFFFFFFFF0000FFFF
    cmp rax, rbx
    jne .t8_fail
    push rsi
    lea rsi, [rel _fw_pass_msg]
    call _fw_print_str
    inc r12d
    pop rsi
    jmp .t8_done
.t8_fail:
    push rsi
    lea rsi, [rel _fw_fail_msg]
    call _fw_print_str
    lea rsi, [rel _fw_expect_msg]
    call _fw_print_str
    mov rax, 0xFFFFFFFF0000FFFF
    call _fw_print_hex64
    lea rsi, [rel _fw_got_msg]
    call _fw_print_str
    movq rax, xmm0
    call _fw_print_hex64
    lea rsi, [rel _fw_nl]
    call _fw_print_str
    pop rsi
.t8_done:

    ; ==== Test 9: pcmpistri flags: ZF set when null found in xmm1 ====
    ; ZF = 1 if any byte in xmm1 is null
    ; SF = 1 if any byte in xmm0 is null
    TEST_CASE t9_name
    movdqa xmm0, [rel str_hello]       ; has null at pos 12 -> SF=1
    movdqa xmm1, [rel str_abcdef]      ; has null at pos 6 -> ZF=1
    pcmpistri xmm0, xmm1, 0x18    ; equal-each, neg polarity
    SAVE_FLAGS
    ; Both have nulls, so ZF=1 and SF=1
    CHECK_FLAGS_EQ (ZF|SF), (ZF|SF)

    ; ==== Test 10: pcmpistri equal-each - null at position 5 in xmm1 ====
    TEST_CASE t10_name
    movdqa xmm0, [rel str_abcd16]      ; "abcdefghijklmnop" (no null, all 16 valid)
    movdqa xmm1, [rel str_xyzab]       ; "xyzab\0..." (null at pos 5)
    pcmpistri xmm0, xmm1, 0x18    ; equal-each, neg polarity, LSB
    ; Compare: a!=x(0), b!=y(1), c!=z(2), d!=a(3), e!=b(4), then xmm1 null at 5
    ; After neg polarity: find first mismatch
    ; All 5 valid bytes are different -> IntRes1 all 0 for pos 0-4
    ; Neg polarity inverts -> IntRes2 bits 0-4 set
    ; Positions 5-15: xmm1 invalid, xmm0 valid -> IntRes1[i] = 0 -> neg -> IntRes2[i] = 1
    ; But wait, positions past xmm1's null are handled specially.
    ; For equal-each when xmm1[i] is invalid but xmm0[i] is valid: IntRes1[i] = 0
    ; After neg: IntRes2[i] = 1
    ; LSB index of IntRes2 = 0 (first mismatch at position 0)
    CHECK_EQ_32 ecx, 0

    ; ==== Test 11: pcmpestri equal-each with explicit lengths ====
    ; EAX = length of xmm0, EDX = length of xmm1
    TEST_CASE t11_name
    movdqa xmm0, [rel str_hello]       ; "Hello World!"
    movdqa xmm1, [rel str_hello2]      ; "Hello World!"
    mov eax, 5             ; only compare first 5 chars "Hello"
    mov edx, 5             ; only compare first 5 chars "Hello"
    pcmpestri xmm0, xmm1, 0x18    ; equal-each, neg polarity, LSB
    ; First 5 bytes match -> neg polarity -> IntRes2 bits 0-4 = 0
    ; Positions 5+: both invalid -> IntRes1[i] = 1 -> neg -> IntRes2[i] = 0
    ; No mismatch found -> ECX = 16
    CHECK_EQ_32 ecx, 16

    ; ==== Test 12: pcmpestrm equal-each bit mask ====
    TEST_CASE t12_name
    movdqa xmm0, [rel str_hello]
    movdqa xmm1, [rel str_heLLo]
    mov eax, 5             ; "Hello" (5 bytes)
    mov edx, 5             ; "HeLLo" (5 bytes)
    pcmpestrm xmm0, xmm1, 0x08    ; equal-each, positive, bit mask
    ; Compare "Hello" vs "HeLLo" (5 bytes each)
    ; pos 0: H==H yes, pos 1: e==e yes, pos 2: l!=L no, pos 3: l!=L no, pos 4: o==o yes
    ; Bits 0-4: 1,1,0,0,1 = 0x13
    ; Bits 5-15: both invalid -> IntRes1 = 1 -> 0x13 | 0xFFE0 = 0xFFF3
    movd eax, xmm0
    and eax, 0xFFFF
    CHECK_EQ_32 eax, 0xFFF3

    ; ==== Test 13: pcmpistri ranges - digit detection ====
    ; Range "09" matches [0x30-0x39] i.e. '0'-'9'
    TEST_CASE t13_name
    movdqa xmm0, [rel str_09_range]    ; "09"
    movdqa xmm1, [rel str_abc12]       ; "abc12xyz"
    pcmpistri xmm0, xmm1, 0x04    ; ranges, positive, LSB
    ; a(0) in ['0'-'9']? no. b(1) no. c(2) no. 1(3) yes -> first digit at pos 3
    CHECK_EQ_32 ecx, 3

    ; ==== Test 14: pcmpistri equal-any (charset search) ====
    TEST_CASE t14_name
    movdqa xmm0, [rel str_charset]     ; "!@#$%"
    movdqa xmm1, [rel str_hellob]      ; "Hello!"
    pcmpistri xmm0, xmm1, 0x00    ; equal-any, positive, LSB
    ; Search "Hello!" for any of {!, @, #, $, %}
    ; H(0) no, e(1) no, l(2) no, l(3) no, o(4) no, !(5) YES -> match at 5
    CHECK_EQ_32 ecx, 5

    ; ==== Test 15: pcmpistri equal-ordered - substring not present ====
    TEST_CASE t15_name
    movdqa xmm0, [rel str_xyz]         ; "xyz"
    movdqa xmm1, [rel str_hello]       ; "Hello World!"
    pcmpistri xmm0, xmm1, 0x0C    ; equal-ordered, positive, LSB
    ; "xyz" not found in "Hello World!" -> ECX = 16
    CHECK_EQ_32 ecx, 16

    ; ==== Test 16: pcmpistrm equal-any bitmask ====
    TEST_CASE t16_name
    movdqa xmm0, [rel str_vowels]      ; "aeiou"
    movdqa xmm1, [rel str_hello]       ; "Hello World!"
    pcmpistrm xmm0, xmm1, 0x00    ; equal-any, positive, bit mask
    ; Positions with vowels in "Hello World!":
    ; H(0)no, e(1)yes, l(2)no, l(3)no, o(4)yes, (5)no, W(6)no, o(7)yes, r(8)no, l(9)no, d(10)no, !(11)no
    ; Bits set: 1, 4, 7 -> mask = 0b10010010 = 0x0092
    movd eax, xmm0
    and eax, 0xFFFF
    CHECK_EQ_32 eax, 0x0092

    ; ==== Test 17: pcmpistri equal-each with negative polarity ====
    ; Negative polarity inverts -> finds MISmatches
    TEST_CASE t17_name
    movdqa xmm0, [rel str_hello]
    movdqa xmm1, [rel str_heLLo]       ; "HeLLo World!"
    pcmpistri xmm0, xmm1, 0x18    ; equal-each, neg polarity, LSB
    ; Mismatches at pos 2, 3. First mismatch = pos 2
    CHECK_EQ_32 ecx, 2

    ; ==== Test 18: pcmpistri all-null source ====
    TEST_CASE t18_name
    movdqa xmm0, [rel str_null16]      ; all zeros
    movdqa xmm1, [rel str_hello]       ; "Hello World!"
    pcmpistri xmm0, xmm1, 0x18    ; equal-each, neg polarity
    ; xmm0 has null at pos 0 -> all positions invalid for xmm0
    ; For equal-each: if xmm0[i] invalid and xmm1[i] valid -> IntRes1[i] = 0
    ; After neg polarity -> IntRes2[i] = 1 for all valid positions in xmm1
    ; ECX should be 0 (first bit set at position 0)
    CHECK_EQ_32 ecx, 0

    ; ==== Test 19: pcmpestri with length 0 ====
    TEST_CASE t19_name
    movdqa xmm0, [rel str_hello]
    movdqa xmm1, [rel str_hello2]
    mov eax, 0             ; xmm0 has 0 valid bytes
    mov edx, 12            ; xmm1 has 12 valid bytes
    pcmpestri xmm0, xmm1, 0x18    ; equal-each, neg polarity
    ; xmm0 length 0 -> all xmm0 bytes invalid
    ; For equal-each: both invalid -> IntRes1 = 1. xmm0 invalid, xmm1 valid -> IntRes1 = 0
    ; Positions 0-11: xmm0 invalid, xmm1 valid -> IntRes1 = 0 -> neg -> IntRes2 = 1
    ; First mismatch/invalid at pos 0
    CHECK_EQ_32 ecx, 0

    ; ==== Test 20: pcmpistri ranges multiple ranges ====
    ; Check alphanumeric: ranges "azAZ09" = [a-z][A-Z][0-9]
    TEST_CASE t20_name
    movdqa xmm0, [rel str_alnum_range] ; "azAZ09"
    movdqa xmm1, [rel str_mixed]       ; "abcXYZ123"
    pcmpistri xmm0, xmm1, 0x04    ; ranges, positive, LSB
    ; All chars a,b,c,X,Y,Z,1,2,3 are alphanumeric -> all match
    ; First match at pos 0
    CHECK_EQ_32 ecx, 0

    ; ==== Test 21: pcmpistrm ranges byte mask ====
    TEST_CASE t21_name
    movdqa xmm0, [rel str_09_range]    ; "09" -> range ['0'-'9']
    movdqa xmm1, [rel str_abc12]       ; "abc12xyz"
    pcmpistrm xmm0, xmm1, 0x44    ; ranges, positive, byte mask
    ; Digits at pos 3('1') and 4('2') -> byte mask: bytes 3,4 = 0xFF, rest = 0x00
    ; Low 64 bits: 00 00 00 FF FF 00 00 00 (LE) = 0x0000FFFF00000000
    ; Wait: byte 0='a' no, byte 1='b' no, byte 2='c' no, byte 3='1' yes, byte 4='2' yes,
    ; byte 5='x' no, byte 6='y' no, byte 7='z' no
    ; Little-endian qword of bytes [0..7]: byte0 is LSB
    ; = 0x00_00_00_FF_FF_00_00_00... no wait:
    ; byte0=0x00, byte1=0x00, byte2=0x00, byte3=0xFF, byte4=0xFF, byte5=0x00, byte6=0x00, byte7=0x00
    ; As 64-bit LE: 0x000000FFFF000000... wait, byte order in LE qword:
    ; address 0 -> LSByte. So byte0 at bit[7:0], byte1 at bit[15:8], etc.
    ; byte3=0xFF at bits [31:24], byte4=0xFF at bits [39:32]
    ; = 0x000000FFFF000000
    movq rax, xmm0
    mov rbx, 0x000000FFFF000000
    cmp rax, rbx
    jne .t21_fail
    push rsi
    lea rsi, [rel _fw_pass_msg]
    call _fw_print_str
    inc r12d
    pop rsi
    jmp .t21_done
.t21_fail:
    push rsi
    lea rsi, [rel _fw_fail_msg]
    call _fw_print_str
    lea rsi, [rel _fw_expect_msg]
    call _fw_print_str
    mov rax, 0x000000FFFF000000
    call _fw_print_hex64
    lea rsi, [rel _fw_got_msg]
    call _fw_print_str
    movq rax, xmm0
    call _fw_print_hex64
    lea rsi, [rel _fw_nl]
    call _fw_print_str
    pop rsi
.t21_done:

    ; ==== Test 22: pcmpistri equal-each partial match ====
    TEST_CASE t22_name
    movdqa xmm0, [rel str_abcdef]      ; "abcdef"
    movdqa xmm1, [rel str_xyzab]       ; "xyzab"
    pcmpistri xmm0, xmm1, 0x08    ; equal-each, positive, LSB
    ; pos0: a!=x, pos1: b!=y, pos2: c!=z, pos3: d!=a, pos4: e!=b
    ; No matches in valid range -> check past-null behavior
    ; Both have null at pos 5 and 6: both invalid -> IntRes1 = 1
    ; First match at pos 5? Both invalid -> result is 1 for equal-each
    ; Actually: "abcdef" null at pos 6, "xyzab" null at pos 5
    ; pos 5: xmm0 valid ('f'), xmm1 invalid -> IntRes1[5] = 0
    ; pos 6-15: both invalid -> IntRes1 = 1
    ; First set bit = 6
    CHECK_EQ_32 ecx, 6

    ; ==== Test 23: pcmpestri equal-ordered ====
    TEST_CASE t23_name
    movdqa xmm0, [rel str_substr]      ; "lo W"
    movdqa xmm1, [rel str_hello]       ; "Hello World!"
    mov eax, 4             ; needle length
    mov edx, 12            ; haystack length
    pcmpestri xmm0, xmm1, 0x0C    ; equal-ordered, positive, LSB
    ; Same as implicit test: "lo W" found at pos 3
    CHECK_EQ_32 ecx, 3

    ; ==== Test 24: pcmpistrm equal-each with negative polarity bitmask ====
    TEST_CASE t24_name
    movdqa xmm0, [rel str_hello]
    movdqa xmm1, [rel str_heLLo]       ; "HeLLo World!"
    pcmpistrm xmm0, xmm1, 0x18    ; equal-each, neg polarity, bit mask
    ; Mismatches at pos 2, 3 only
    ; IntRes2: bits 2,3 set = 0x000C
    movd eax, xmm0
    and eax, 0xFFFF
    CHECK_EQ_32 eax, 0x000C

    ; ==== Test 25: pcmpistri CF/ZF flags ====
    ; CF = 1 if IntRes2 is not all zeros (i.e., there's a result)
    ; ZF = 1 if any byte in xmm1 is null (end of string found)
    TEST_CASE t25_name
    movdqa xmm0, [rel str_vowels]      ; "aeiou" (has null)
    movdqa xmm1, [rel str_hello]       ; "Hello World!" (has null)
    pcmpistri xmm0, xmm1, 0x00    ; equal-any, positive, LSB
    SAVE_FLAGS
    ; IntRes2 is not all-zero (vowels found) -> CF=1
    ; xmm1 has null -> ZF=1
    ; xmm0 has null -> SF=1
    CHECK_FLAGS_EQ (CF|ZF), (CF|ZF)

    END_TESTS
