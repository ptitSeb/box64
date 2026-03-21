# PPC64LE Dynarec Upstreaming Roadmap

Status tracker for upstreaming opcodes from the `ppc64le-dynarec` branch to `upstream/main` (ptitSeb/box64).

## Current State (as of 2026-03-14)

### Infrastructure: 100% Upstream
All dynarec scaffolding, emitter, helper macros, functions, printer, assembly stubs are upstream.
The upstream `helper.h` already declares ALL emit functions - only implementations are missing.

### What's Upstream in `_00.c` (40 case labels)

| PR | Opcodes | Description |
|----|---------|-------------|
| #3592 | Infrastructure | Full dynarec scaffolding |
| #3620 | 0x50-0x5F, 0xC9 | PUSH/POP reg, LEAVE |
| #3633 | 0x63, 0x88-0x8B, 0x8D, 0x90-0x99, 0xB0-0xBF, 0xC6-0xC7 | MOV, NOP/XCHG, MOVSXD, CWDE/CDQ, LEA |
| #3652 | 0x00-0x05 | ADD Eb/Gb, ADD Ed/Gd, ADD AL/Ib, ADD EAX/Id |
| #3668 | 0x08-0x0D, 0x20-0x25, 0x28-0x2D, 0x30-0x35 | OR, AND, SUB, XOR (all Eb/Gb..EAX/Id forms) |
| #3677 | (refactor) | GETED/GETEB/GETGB/GETGBEB/WBACK/GBBACK/EBBACK helper macros + refactor _00.c ALU opcodes |
| #3678 | 0x38-0x3D, 0x84-0x85, 0xA8-0xA9 | CMP, TEST (depends on #3677) |

### Emit Files Upstream
- `dynarec_ppc64le_emit_math.c` — `emit_add8`, `emit_add8c`, `emit_add32`, `emit_add32c`, `emit_sub8`, `emit_sub8c`, `emit_sub32`, `emit_sub32c` (8 functions)
- `dynarec_ppc64le_emit_logic.c` — `emit_or8`, `emit_or8c`, `emit_or32`, `emit_or32c`, `emit_and8`, `emit_and8c`, `emit_and32`, `emit_and32c`, `emit_xor8`, `emit_xor8c`, `emit_xor32`, `emit_xor32c` (12 functions)
- `dynarec_ppc64le_emit_tests.c` — `emit_cmp8`, `emit_cmp8_0`, `emit_cmp16`, `emit_cmp16_0`, `emit_cmp32`, `emit_cmp32_0`, `emit_test8`, `emit_test8c`, `emit_test16`, `emit_test32`, `emit_test32c` (11 functions) *(PR #3678, pending)*

### Opcode Dispatch Files Upstream (stubs only, minimal implementations)
- `dynarec_ppc64le_66.c`
- `dynarec_ppc64le_66f0.c`
- `dynarec_ppc64le_f0.c`

## Upstreaming Plan

### Phase 1: Remaining ALU Operations in `_00.c`

Each batch follows the ADD pattern (6 opcodes per group: Eb/Gb, Ed/Gd, Gb/Eb, Gd/Ed, AL/Ib, EAX/Id).

**Style note**: Batches 1a/1b used manual inline byte register handling to match upstream's existing style. Batch 1-macros will upstream the GETGB/GETEB/GETED/WBACK/GBBACK/EBBACK/GETGBEB macros (matching ARM64/RV64/LA64) AND refactor all existing opcode cases (ADD/OR/SUB/AND/XOR/MOV) to use them. All code in `_00.c` will be macro-style after that point.

#### Batch 1a: OR + SUB — ✅ Merged
- **PR**: [#3668](https://github.com/ptitSeb/box64/pull/3668)
- **Opcodes**: 0x08-0x0D (OR), 0x28-0x2D (SUB) — 12 opcodes
- **Emit functions added**:
  - `emit_or8`, `emit_or8c`, `emit_or32`, `emit_or32c` (in new `emit_logic.c`)
  - `emit_sub8`, `emit_sub8c`, `emit_sub32`, `emit_sub32c` (added to `emit_math.c`)
- **New file**: `dynarec_ppc64le_emit_logic.c`
- **CMakeLists.txt**: added `emit_logic.c` to DYNAREC_PASS
- **Verified**: Build clean (0 warnings), ctest 33/33 pass, NASM test_int_arith 40/40 pass (dynarec + interpreter)

#### Batch 1b: AND + XOR — ✅ Merged (folded into #3668)
- **PR**: [#3669](https://github.com/ptitSeb/box64/pull/3669) (closed; content merged via [#3668](https://github.com/ptitSeb/box64/pull/3668))
- **Branch**: `ppc64le-and-xor-opcodes` (based on `ppc64le-or-sub-opcodes`)
- **Opcodes**: 0x20-0x25 (AND), 0x30-0x35 (XOR) — 12 opcodes
- **Emit functions added**:
  - `emit_and8`, `emit_and8c`, `emit_and32`, `emit_and32c` (in `emit_logic.c`)
  - `emit_xor8`, `emit_xor8c`, `emit_xor32`, `emit_xor32c` (in `emit_logic.c`)
- **Dependencies**: Batch 1a (emit_logic.c must exist)
- **Bug found & fixed**: All four XOR emitters used `SET_DF(s4, ...)` which clobbered `x2` (the address register from `geted()`) in the memory path of 0x30/0x31. Fixed to `SET_DF(s3, ...)` matching AND/OR emitters. The bug caused SIGSEGV accessing address `0x34` (the df constant value) when writing back the XOR result.
- **Verified**: Build clean (0 warnings), ctest 33/33 pass, NASM test_and_xor 30/30 pass (dynarec + interpreter), NASM test_int_arith 40/40 pass

#### Batch 1-macros: Upstream GETGB/GETEB/GETED/WBACK/GBBACK/EBBACK/GETGBEB Macros + Refactor Existing Opcodes — ✅ PR Submitted
- **PR**: [#3677](https://github.com/ptitSeb/box64/pull/3677)
- **Branch**: `ppc64le-helper-macros`
- **Files**: `dynarec_ppc64le_helper.h`, `dynarec_ppc64le_00.c`
- **Part A — Add macros to `helper.h`**:
  - `GETED`, `GETEB`, `GETGB`, `GETGBEB`, `WBACK`, `GBBACK`, `EBBACK`
  - Source: Already implemented on `ppc64le-dynarec` branch
- **Part B — Refactor all existing opcode cases in `_00.c` to use macros**:
  - ADD 0x00-0x05 (from PR #3652) — replace manual inline byte register handling with GETGBEB/EBBACK/GBBACK
  - OR 0x08-0x0D (from Batch 1a) — same refactor
  - SUB 0x28-0x2D (from Batch 1a) — same refactor
  - AND 0x20-0x25 (from Batch 1b) — same refactor
  - XOR 0x30-0x35 (from Batch 1b) — same refactor
  - MOV 0x88-0x8B (from PR #3633) — refactor to use GETED/GETEB/GETGB/WBACK where applicable
- **Justification**: All three other backends (ARM64, RV64, LA64) define these macros in their respective `helper.h` files. PPC64LE is the only backend that lacks them. Adding them and refactoring:
  1. Makes the PPC64LE backend consistent with ARM64/RV64/LA64
  2. Eliminates the scratch register conflict risk class (SET_DF clobbering address registers)
  3. Keeps `_00.c` internally consistent — no mix of manual inline vs macro style
  4. Reduces review burden for future PRs
- **Dependencies**: #3668 (OR/AND/SUB/XOR) must be merged first so the diff is clean — ✅ done
- **Verification**: Must pass all existing tests (ctest 33/33, NASM test_int_arith 40/40, NASM test_and_xor 30/30) since this is a pure refactor with no behavioral change
- **Benchmark (500 invocations x 3 runs)**:
  | Test | Mode | BEFORE (inline) avg | AFTER (macro) avg | Delta |
  |------|------|---------------------|-------------------|-------|
  | test_int_arith | DYNAREC | 11.72s | 11.87s | +1.3% |
  | test_and_xor | DYNAREC | 11.07s | 11.89s | +7.4% |
  | test_int_arith | INTERP | 8.79s | 8.86s | +0.8% |
  | test_and_xor | INTERP | 8.92s | 8.93s | +0.1% |
  Conclusion: Macros are compile-time only (C preprocessor) and generate identical PPC64LE machine code. Interpreter mode confirms no regression (<1%). Dynarec variance is system load noise — these micro-benchmarks are dominated by 500x box64 process startup/teardown overhead.

#### Batch 1c: CMP + TEST (0x84-0x85, 0xA8-0xA9) — ✅ PR Submitted
- **PR**: [#3678](https://github.com/ptitSeb/box64/pull/3678)
- **Branch**: `ppc64le-cmp-test-opcodes` (based on `ppc64le-helper-macros`)
- **Opcodes**: 0x38-0x3D (CMP), 0x84-0x85 (TEST Eb/Ed), 0xA8-0xA9 (TEST AL/EAX) — 10 opcodes
- **Emit functions added** (in new `dynarec_ppc64le_emit_tests.c`, 535 lines, 11 functions):
  - `emit_cmp8`, `emit_cmp8_0`, `emit_cmp16`, `emit_cmp16_0`, `emit_cmp32`, `emit_cmp32_0`
  - `emit_test8`, `emit_test8c`, `emit_test16`, `emit_test32`, `emit_test32c`
- **New file**: `dynarec_ppc64le_emit_tests.c`
- **CMakeLists.txt**: added `emit_tests.c` to DYNAREC_PASS
- **Special**: CMP never writes back; CMP 0x3C/0x3D has zero-optimized paths via `emit_cmp8_0`/`emit_cmp32_0`
- **Dependencies**: Batch 1-macros (#3677) must merge first
- **Style**: Uses GETGBEB/EBBACK/GBBACK/WBACK macros (matching ARM64/RV64/LA64 style)
- **NASM tests**: `test_cmp_test.asm` — 35 test cases covering register, memory, 64-bit, hi-byte, zero-optimized, overflow, SF, PF, and TEST clearing CF/OF. **Note**: NASM test files were excluded from PR #3678 since upstream has no `tests/dynarec/` directory — test framework upstreaming is tracked separately in Phase 8. Tests remain on `ppc64le-dynarec` branch for local validation.
- **Verified**: Build clean (0 warnings), ctest 33/33 pass, NASM test_cmp_test 35/35 pass (dynarec + interpreter match native x86_64), all existing NASM tests still pass

#### Batch 1d: ADC + SBB
- **PR**: [#3684](https://github.com/ptitSeb/box64/pull/3684)
- **Opcodes**: 0x10-0x15 (ADC), 0x18-0x1D (SBB) — 12 opcodes
- **New emit functions needed**:
  - `emit_adc8`, `emit_adc8c`, `emit_adc32` (add to `emit_math.c`)
  - `emit_sbb8`, `emit_sbb8c`, `emit_sbb32` (add to `emit_math.c`)
- **Special**: ADC/SBB require `READFLAGS(X_CF)` before `SETFLAGS`
- **Dependencies**: Batch 1-macros
- **Style**: Macro style (GETGBEB/EBBACK/GBBACK/WBACK)

#### Batch 1e: Group 1 ALU Immediate (0x80-0x83)
- **Opcodes**: 0x80/0x82 (Group 1 Eb, Ib), 0x81 (Group 1 Ed, Id), 0x83 (Group 1 Ed, Ib) — 4 opcodes, 8 sub-operations each
- **Dependencies**: ALL emit functions from Batches 1a-1d must be upstream
- **Special**: These use switch on `(nextop>>3)&7` to dispatch ADD/OR/ADC/SBB/AND/SUB/XOR/CMP

### Phase 2: Control Flow + Shifts + Misc in `_00.c`

#### Batch 2a: Conditional Jumps + JMP/CALL
- **Opcodes**: 0x70-0x7F (Jcc short), 0xE8 (CALL), 0xE9/0xEB (JMP), 0xC2/0xC3 (RET)
- **Dependencies**: GOCOND macro in helper.h (already upstream)

#### Batch 2b: Shift/Rotate (Group 2)
- **Opcodes**: 0xC0/0xC1, 0xD0/0xD1, 0xD2/0xD3
- **New file**: `dynarec_ppc64le_emit_shift.c`
- **Dependencies**: none beyond infrastructure

#### Batch 2c: PUSH/POP immediate, IMUL, XCHG memory
- **Opcodes**: 0x68/0x6A (PUSH imm), 0x69/0x6B (IMUL), 0x86/0x87 (XCHG mem)

#### Batch 2d: String Operations
- **Opcodes**: 0xA4-0xAF (MOVS, CMPS, STOS, LODS, SCAS with REP)

#### Batch 2e: Group 3-5 (F6/F7/FE/FF)
- **Opcodes**: 0xF6/0xF7 (TEST/NOT/NEG/MUL/IMUL/DIV/IDIV), 0xFE/0xFF (INC/DEC/CALL/JMP/PUSH)
- **New emit functions**: `emit_neg8/16/32`, `emit_inc8/16/32`, `emit_dec8/16/32`

#### Batch 2f: Flags + Misc
- **Opcodes**: 0x9C/0x9D (PUSHF/POPF), 0x9E/0x9F (SAHF/LAHF), 0xF5/0xF8-0xFD (CMC/CLC/STC/CLD/STD), 0xCC/0xCD (INT)

### Phase 3: Two-Byte Opcodes (`_0f.c`)
- **New file**: `dynarec_ppc64le_0f.c` (2,712 lines)
- Split into sub-batches by opcode group (CMOVcc, SETcc, BSF/BSR, MOVZX/MOVSX, SSE, etc.)

### Phase 4: SSE/SSE2 Prefix Opcodes
- `_660f.c` (4,022 lines) — SSE2 integer/packed operations
- `_f20f.c` (722 lines) — SSE2 scalar double
- `_f30f.c` (1,014 lines) — SSE scalar float

### Phase 5: x87 FPU
- `_d8.c` through `_df.c` (8 files, ~2,200 lines total)

### Phase 6: LOCK Prefix Operations
- Expand `_f0.c` (1,790 lines), `_66f0.c` (388 lines)

### Phase 7: AVX
- 11 AVX files (~8,800 lines total)
- Lowest priority; many applications don't require AVX

### Phase 8: NASM Dynarec Test Framework

Upstream the `tests/dynarec/` NASM test infrastructure for opcode-level regression testing. This is orthogonal to opcode upstreaming (Phases 1-7) and can be proposed at any time. All backends (ARM64, RV64, LA64, PPC64LE) benefit equally since these tests exercise the interpreter and all dynarec backends.

**Current state**: 37 `.asm` test files + `test_framework.inc` + `run_tests.sh` exist on the `ppc64le-dynarec` branch. Upstream has no `tests/dynarec/` directory — its existing tests use pre-compiled x86_64 ELF binaries + `.txt` reference outputs compared via `runTest.cmake`.

#### Approach: Hybrid (pre-compiled binaries + optional NASM recompilation)

**Files to upstream**:
- `tests/dynarec/test_framework.inc` — macro framework (INIT_TESTS, TEST_CASE, CHECK_EQ_32/64, SAVE_FLAGS, CHECK_FLAGS_EQ, END_TESTS); raw syscalls only, no libc dependency
- 37 `.asm` source files (one per opcode group / feature area)
- Pre-compiled x86_64 ELF static binaries for each (committed to repo, like upstream's existing `tests/testNN` binaries)
- `.txt` reference output files for each (generated once on native x86_64, committed to repo)
- `tests/dynarec/run_tests.sh` — convenience script for manual testing outside of ctest

**CMake integration**:
```cmake
# Optional NASM detection
find_program(NASM nasm)

# If NASM found, add custom commands to recompile .asm files
# This allows editing tests without an external toolchain
if(NASM)
    foreach(TEST_ASM ${DYNAREC_TEST_ASM_FILES})
        get_filename_component(TEST_NAME ${TEST_ASM} NAME_WE)
        add_custom_command(
            OUTPUT ${CMAKE_BINARY_DIR}/tests/dynarec/${TEST_NAME}
            COMMAND ${NASM} -f elf64 ${TEST_ASM} -o ${CMAKE_BINARY_DIR}/tests/dynarec/${TEST_NAME}.o
            COMMAND ld -o ${CMAKE_BINARY_DIR}/tests/dynarec/${TEST_NAME} ${CMAKE_BINARY_DIR}/tests/dynarec/${TEST_NAME}.o
            DEPENDS ${TEST_ASM} ${CMAKE_SOURCE_DIR}/tests/dynarec/test_framework.inc
        )
    endforeach()
endif()

# Tests always use pre-compiled binaries (or NASM-recompiled if available)
# Following upstream's existing runTest.cmake pattern
if(NOT ANDROID)
    add_test(dynarec_add_or_sub ${CMAKE_COMMAND}
        -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX64}
        -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/dynarec/test_add_or_sub
        -D TEST_OUTPUT=tmpfile_dynarec_add_or_sub.txt
        -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/dynarec/test_add_or_sub.txt
        -P ${CMAKE_SOURCE_DIR}/runTest.cmake)
    # ... repeat for each test
endif()
```

**Key design decisions**:
- Pre-compiled binaries guarantee tests work on CI without NASM installed
- Optional NASM recompilation allows contributors to edit/add tests locally
- `.txt` reference files are generated once on native x86_64 and committed — no need to run natively during CI
- Tests follow upstream's existing `add_test()` + `runTest.cmake` pattern exactly
- All test binaries are static (no libc dependency) — they use raw syscalls via `test_framework.inc`

**Test inventory** (37 files on `ppc64le-dynarec`):

| Test file | Description |
|-----------|-------------|
| `test_add_or_sub.asm` | ADD/OR/SUB: all opcode forms, mem paths, hi-byte, 64-bit, flags |
| `test_and_xor.asm` | AND/XOR: all opcode forms, mem paths, hi-byte, 64-bit, flags |
| `test_cmp_test.asm` | CMP/TEST: all opcode forms, mem, zero-optimized, overflow, flags |
| `test_int_arith.asm` | Integer arithmetic: ADD, SUB, CMP, TEST, ADC, SBB, INC, DEC, NEG, MUL, IMUL, DIV, IDIV |
| `test_mov.asm` | MOV: reg-reg, mem, imm, hi-byte, RIP-relative, flags preservation |
| `test_push_pop_misc.asm` | PUSH/POP, XCHG, NOP, LEAVE, MOVSXD, CWDE/CDQ |
| `test_shifts.asm` | Shift/rotate operations |
| `test_updateflags.asm` | General flags update verification |
| `test_updateflags_arith.asm` | Arithmetic flags (CF, AF, OF) edge cases |
| `test_updateflags_carry.asm` | Carry flag propagation (ADC/SBB chains) |
| `test_updateflags_logic.asm` | Logic flags (CF=0, OF=0 for AND/OR/XOR/TEST) |
| `test_updateflags_mul.asm` | MUL/IMUL flags |
| `test_updateflags_shift.asm` | Shift flags (CF from last bit shifted) |
| `test_updateflags_shld.asm` | SHLD/SHRD flags |
| `test_misc_flags.asm` | Misc flag operations (CMC, CLC, STC, SAHF, LAHF) |
| `test_misc_int.asm` | Misc integer (BSF, BSR, BT, BTC, BTS, BTR, BSWAP, MOVZX, MOVSX) |
| `test_bit_extract.asm` | BMI1/BMI2 bit extraction |
| `test_conversions.asm` | Type conversions (CBW, CWD, CDQ, CQO, MOVSX, MOVZX) |
| `test_movs_stos.asm` | String operations (MOVS, STOS, CMPS, SCAS with REP) |
| `test_mov_push_imm.asm` | MOV/PUSH with immediate operands |
| `test_lock_atomic.asm` | LOCK prefix atomic operations |
| `test_sse_arith.asm` | SSE arithmetic (ADDPS/SUBPS/MULPS/DIVPS etc.) |
| `test_sse_cmp.asm` | SSE comparison (CMPPS, COMISS, UCOMISS) |
| `test_sse_hadd.asm` | SSE horizontal add/sub |
| `test_sse_int.asm` | SSE integer (PADDB/W/D, PSUBB/W/D, PMULL, etc.) |
| `test_sse_misc_int.asm` | SSE misc integer (PSHUFB, PSADBW, etc.) |
| `test_sse_shuffle.asm` | SSE shuffle (SHUFPS, PSHUFD, PUNPCK, etc.) |
| `test_sse_insert_extract.asm` | SSE4.1 insert/extract |
| `test_sse_round_blend.asm` | SSE4.1 round/blend |
| `test_sse_string.asm` | SSE4.2 string (PCMPISTRI, PCMPISTRM) |
| `test_sse3_move.asm` | SSE3 move (MOVDDUP, MOVSHDUP, MOVSLDUP, LDDQU) |
| `test_pmaddubsw.asm` | SSSE3 PMADDUBSW |
| `test_shufps.asm` | SHUFPS detailed testing |
| `test_aes_crc.asm` | AES-NI and CRC32 instructions |
| `test_x87.asm` | x87 FPU operations |

**Batching strategy**:
- **Batch 8a**: Framework + integer tests (test_framework.inc, test_add_or_sub, test_and_xor, test_cmp_test, test_int_arith, test_mov, test_push_pop_misc, test_shifts, all updateflags tests, test_misc_flags, test_misc_int, test_bit_extract, test_conversions, test_movs_stos, test_mov_push_imm, test_lock_atomic) — ~22 tests
- **Batch 8b**: SSE/SSE2/SSE3/SSE4/AES tests (all test_sse_*, test_pmaddubsw, test_shufps, test_aes_crc, test_sse3_move) — ~13 tests
- **Batch 8c**: x87 FPU tests (test_x87) — 1 test
- **Batch 8d**: run_tests.sh convenience script + any remaining tests

**Prerequisites**: Discuss with maintainer (ptitSeb) before submitting. This introduces a new testing paradigm for the project. Key selling points:
1. Opcode-level regression tests catch dynarec bugs that higher-level C tests miss (e.g., the EBBACK STB argument swap bug was only caught by memory-path NASM tests)
2. Tests are backend-agnostic — they benefit ARM64/RV64/LA64 equally, not just PPC64LE
3. Hybrid approach requires zero new CI dependencies (pre-compiled binaries always work)
4. Follows upstream's existing `add_test()` + `runTest.cmake` pattern exactly

## Upstream Style Reference

### Byte Opcode Pattern (manual inline, matching upstream ADD)
```c
case 0xNN:
    INST_NAME("OP Eb, Gb");
    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
    nextop = F8;
    SCRATCH_USAGE(0);
    // Get Gb (source byte register)
    gd = ((nextop & 0x38) >> 3) + (rex.r << 3);
    if (rex.rex) {
        gb2 = 0;
        gb1 = TO_NAT(gd);
    } else {
        gb2 = ((gd & 4) << 1);
        gb1 = TO_NAT(gd & 3);
    }
    if (gb2) {
        gd = x4;
        BF_EXTRACT(gd, gb1, gb2 + 7, gb2);
    } else {
        gd = gb1;
    }
    if (MODREG) {
        ed = (nextop & 7) + (rex.b << 3);
        if (rex.rex) {
            eb1 = TO_NAT(ed);
            eb2 = 0;
        } else {
            eb1 = TO_NAT(ed & 3);
            eb2 = ((ed & 4) << 1);
        }
        if (eb2) {
            ed = x5;
            BF_EXTRACT(ed, eb1, eb2 + 7, eb2);
        } else {
            ed = eb1;
        }
        emit_OP8(dyn, ninst, ed, gd, x1, x2, ...);
        BF_INSERT(eb1, ed, eb2 + 7, eb2);
    } else {
        addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, DS_DISP, 0);
        SMREADLOCK(lock);
        LBZ(x5, fixedaddress, ed);
        emit_OP8(dyn, ninst, x5, gd, x1, x2, ...);
        STB(x5, fixedaddress, ed);
        SMWRITELOCK(lock);
    }
    break;
```

### Dword Opcode Pattern (manual inline, matching upstream ADD)
```c
case 0xNN:
    INST_NAME("OP Ed, Gd");
    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
    nextop = F8;
    GETGD;
    SCRATCH_USAGE(0);
    if (MODREG) {
        ed = TO_NAT((nextop & 7) + (rex.b << 3));
        emit_OP32(dyn, ninst, rex, ed, gd, x3, x4, x5, ...);
    } else {
        addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, &lock, DS_DISP, 0);
        SMREADLOCK(lock);
        LDxw(x5, ed, fixedaddress);
        emit_OP32(dyn, ninst, rex, x5, gd, x3, x4, x1, ...);
        SDxw(x5, ed, fixedaddress);
        SMWRITELOCK(lock);
    }
    break;
```

### Notes
- `SCRATCH_USAGE(0)` is a no-op currently but signals register allocation intent
- Byte operations: Gb uses `x4` for extract, Eb uses `x5` for extract; `x1`/`x2` for emit scratch
- Dword operations: `GETGD` for Gd; `x5` for memory loads; `x3`/`x4` for emit scratch
- Memory paths use `geted()` + `SMREADLOCK`/`SMWRITELOCK` for atomic support
- All emit function signatures are already declared in upstream `helper.h`
- **Emit function scratch registers**: `SET_DF()` must use `s3` (not `s4`) to avoid clobbering the address register (`x2`/`ed`) in memory paths. `s3` is safe because `CLEAR_FLAGS(s3)` reuses it immediately after. This risk class is eliminated once GETGB/GETEB/WBACK macros are upstream (Batch 1-macros), as the macros encapsulate register allocation.

## Build & Test Workflow

### Remote Infrastructure

| Machine | Address | Purpose |
|---------|---------|---------|
| PPC64LE build/test box | `tle@192.168.1.247` (port 22) | Build box64, run ctest, run NASM tests under box64 |
| x86_64 compile box | `tle@192.168.1.148` (port 2222) | Compile NASM test `.asm` files into x86_64 ELF binaries |
| Local Mac | localhost | Source editing, rsync hub between machines |

### Step 1: Rsync Source to PPC64LE Box

```bash
rsync -az --exclude='.git' --exclude='build' /Users/tle/Work/box64/ tle@192.168.1.247:/tmp/box64-build/
```

Note: Without `.git`, the build generates `git_head.h` with empty GITREV (this is fine). `build_info.c` is regenerated by cmake.

### Step 2: Build on PPC64LE Box

```bash
ssh tle@192.168.1.247
cd /tmp/box64-build/build
cmake .. -DPPC64LE_DYNAREC=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo
make -j$(nproc)
```

Verify: 0 errors, 0 warnings (warnings in the dynarec files indicate real issues).

### Step 3: Run ctest

```bash
ssh tle@192.168.1.247 "cd /tmp/box64-build/build && ctest --output-on-failure"
```

Expected: `100% tests passed, 0 tests failed out of 33`.

### Step 4: Write NASM Test

NASM tests live in `tests/dynarec/` on the `ppc64le-dynarec` branch. Each test file:
- Includes `test_framework.inc` at the top
- Uses `INIT_TESTS` / `TEST_CASE` / `CHECK_EQ_32` / `CHECK_EQ_64` / `SAVE_FLAGS` / `CHECK_FLAGS_EQ` / `END_TESTS` macros
- Uses raw syscalls only (no libc dependency) — produces a static ELF binary
- Tests register results AND flags (CF, PF, AF, ZF, SF, OF)

Flag constants defined in framework:
```nasm
%define CF 0x0001
%define PF 0x0004
%define AF 0x0010
%define ZF 0x0040
%define SF 0x0080
%define OF 0x0800
%define FLAGS_MASK_ARITH (CF|PF|AF|ZF|SF|OF)
%define FLAGS_MASK_LOGIC (CF|PF|ZF|SF|OF)
```

Each batch should have a dedicated NASM test covering:
- All opcode forms: Eb/Gb, Ed/Gd, Gb/Eb, Gd/Ed, AL/Ib, EAX/Id
- Register-register AND memory paths
- Hi-byte registers (AH, CH) for byte operations (non-REX)
- 64-bit (REX.W) variants
- Flag verification (ZF, SF, CF=0/OF=0 for logic ops)

### Step 5: Compile NASM Test on x86_64 Box

```bash
# Copy .asm and framework to x86_64 box
scp -P 2222 test_file.asm tle@192.168.1.148:/tmp/nasm_tests/
scp -P 2222 tests/dynarec/test_framework.inc tle@192.168.1.148:/tmp/nasm_tests/

# Compile on x86_64
ssh -p 2222 tle@192.168.1.148 "cd /tmp/nasm_tests && nasm -f elf64 test_file.asm -o test_file.o && ld -o test_file test_file.o"

# Verify on native x86_64 first
ssh -p 2222 tle@192.168.1.148 "/tmp/nasm_tests/test_file"
# Expected: all PASS, Result: N/N
```

### Step 6: Transfer Binary to PPC64LE Box

Two-hop transfer (x86_64 -> local Mac -> PPC64LE):
```bash
scp -P 2222 tle@192.168.1.148:/tmp/nasm_tests/test_file /tmp/test_file
scp /tmp/test_file tle@192.168.1.247:/tmp/test_file
```

### Step 7: Run NASM Test Under box64 (Dynarec + Interpreter)

```bash
# Dynarec mode
ssh tle@192.168.1.247 'BOX64_DYNAREC=1 /tmp/box64-build/build/box64 /tmp/test_file > /tmp/dynarec_out.txt 2>/dev/null; echo "Exit: $status"'

# Interpreter mode
ssh tle@192.168.1.247 'BOX64_DYNAREC=0 /tmp/box64-build/build/box64 /tmp/test_file > /tmp/interp_out.txt 2>/dev/null; echo "Exit: $status"'

# Compare outputs (must match)
ssh tle@192.168.1.247 'diff /tmp/dynarec_out.txt /tmp/interp_out.txt && echo "MATCH" || echo "DIFFER"'
```

Both modes must: exit 0, show all PASS, Result: N/N, and outputs must match.

Note: The remote PPC64LE box runs **fish shell**, so use `$status` instead of `$?` for exit codes.

### Step 8: Debug Crashes (if needed)

Enable debug logging to diagnose dynarec segfaults:
```bash
ssh tle@192.168.1.247 'BOX64_DYNAREC=1 BOX64_LOG=1 BOX64_DYNAREC_LOG=1 BOX64_DYNAREC_DUMP=1 /tmp/box64-build/build/box64 /tmp/test_file > /tmp/debug_out.txt 2>&1'
```

Key things to look for in crash output:
- `SIGSEGV ... for accessing 0xNN` — the faulting address
- `x64opcode=NN NN` — the x64 instruction being emulated
- The dynarec dump shows generated PPC64LE instructions for each x64 instruction
- Look for scratch register conflicts where `SET_DF`/`CLEAR_FLAGS`/`emit_pf` clobber registers still needed by the caller

### Existing NASM Test Binaries on PPC64LE Box

| Binary | Tests | Description |
|--------|-------|-------------|
| `/tmp/test_int_arith` | 40/40 | ADD, SUB, CMP, TEST, ADC, SBB, INC, DEC, NEG, MUL, IMUL, DIV, IDIV, XADD, CMPXCHG, BSWAP, LEA, CDQ, CQO, MOVSX |
| `/tmp/test_and_xor` | 30/30 | AND (0x20-0x25) + XOR (0x30-0x35): all opcode forms, mem paths, hi-byte regs, 64-bit, flags |
| `/tmp/test_add_or_sub` | 40/40 | ADD (0x00-0x05), OR (0x08-0x0D), SUB (0x28-0x2D): all 6 opcode forms per group, mem paths, hi-byte regs, 64-bit, flags |
| `/tmp/test_mov` | 24/24 | MOV (0x88-0x8B, 0xB0-0xBF, 0xC6-0xC7): reg-reg, mem, imm8/32/64, hi-byte, RIP-rel, flags preservation |
| `/tmp/test_push_pop_misc` | 34/34 | PUSH/POP (0x50-0x5F), XCHG (0x86-0x87, 0x90+r), NOP, LEAVE, MOVSXD (0x63), CWDE/CDQ/CDQE/CQO/CBW |

NASM test sources are committed to `ppc64le-dynarec` branch at `tests/dynarec/`.

---

## Lessons Learned / Pitfalls

### 1. EBBACK STB Argument Swap Bug (2026-03-14)

**What happened**: When porting the `EBBACK()` macro from `ppc64le-dynarec` to `ppc64le-helper-macros`,
the arguments to `STB(ed, fixedaddress, wback)` were accidentally written as `STB(ed, wback, fixedaddress)`.
This swapped the immediate offset and base register, causing a segfault when accessing tiny addresses like `0x5`.

**Root cause**: Manual re-typing instead of exact copy-paste. Both argument orders compile without error —
PPC64LE `STB(Rs, offset, Ra)` treats the second arg as an immediate and the third as a register, so
swapping them silently generates wrong code.

**How it was caught**: The `test_and_xor` NASM test included memory operand test cases (`and Eb,Gb mem`).
Register-only tests all passed; the bug only manifested with memory operands.

**Mandatory process going forward**:
1. **Never manually re-type code from `ppc64le-dynarec`**. Always extract with `git show ppc64le-dynarec:<path>` and copy verbatim.
2. **Always diff after porting**: `diff <(git show ppc64le-dynarec:<path>) <path>` to verify only intended changes.
3. **Every NASM test MUST include memory operand variants** — register-only tests are insufficient to catch address computation bugs.
4. **Run NASM tests before ctest** — ctest's existing tests may not exercise the specific opcode forms being ported.

### 2. NASM Test Gotchas

- **Cannot use `cmp reg64, imm64`** — x86_64 only supports 32-bit sign-extended immediates for `cmp`. Use `mov` to a temp register first, or use `CHECK_EQ_64` macro which handles this.
- **Framework reserves r12-r15** — don't clobber these in test cases.
- **Always test both `BOX64_DYNAREC=1` and `BOX64_DYNAREC=0`** to confirm dynarec matches interpreter.
