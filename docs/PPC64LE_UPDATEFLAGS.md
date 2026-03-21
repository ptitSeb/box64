# PPC64LE Native UpdateFlags — Technical Reference

## Overview

The PPC64LE native UpdateFlags block replaces the C `UpdateFlags()` function
(`src/emu/x64emu.c:712`) with a pre-generated native PPC64LE code block that
handles all 89 deferred flag (df) types inline. This eliminates the overhead of
calling into C from JIT-generated code when deferred x86 flags need to be
materialized at basic block boundaries.

### Performance Impact

Profiling Pillars of Eternity on PPC64LE showed `UpdateFlags` consuming 2.95%
of total CPU time (~21M calls, 96.99% of which were `d_cmp32`). After the
native block was deployed, `UpdateFlags` dropped to 0.00% in perf profiles.
Menu FPS improved from ~40 to 45-50.

## Architecture

### Why UpdateFlags Exists

The PPC64LE dynarec already has comprehensive `emit_*` functions (103 total)
that compute x86 flags inline during opcode translation. However, at **basic
block boundaries** (in `flagsCacheTransform` in `dynarec_ppc64le_helper.c`),
when the flag status is `status_unk`, the dynarec must resolve any pending
deferred flags before the next block can consume them. Previously this was done
by calling the C `UpdateFlags()` function. The native block replaces that call.

### File Layout

| File | Compiled | Purpose |
|------|----------|---------|
| `updateflags_ppc64le.c` | Once (in `DYNAREC_SRC`) | Builder: allocates dynablock, runs 4 passes, caches result |
| `updateflags_ppc64le_pass.c` | 4x (in `DYNAREC_PASS`, with `-DSTEP=0..3`) | Code generation: prologue, dispatch, all 89 handlers |

### Multi-Pass Compilation

The pass file is compiled 4 times with different `STEP` values (0-3), producing
4 functions: `updateflags_pass0()` through `updateflags_pass3()`. This reuses
the same multi-pass pipeline as normal dynablock generation:

- **Pass 0**: Size counting. `EMIT(A)` increments `dyn->native_size += 4`.
  `SETMARK(A)` records which df types are handled (`jmp_df[A] = 1`).
- **Pass 1**: No-op. `EMIT(A)` does nothing. `SETMARK(A)` clears (`jmp_df[A] = 0`).
- **Pass 2**: Offset recording. `EMIT(A)` increments size. `SETMARK(A)` records
  the byte offset (`jmp_df[A] = dyn->native_size`).
- **Pass 3**: Code emission. `EMIT(A)` writes the instruction word to the
  allocated block. `SETMARK(A)` prints debug info (via `MESSAGE`).

The function name mangling is handled by `#define updateflags_pass STEPNAME(updateflags_pass)`
in `dynarec_ppc64le_helper.h`, where `STEPNAME` concatenates the step number.

### Critical Build Guards

The pass file defines its own `EMIT`, `SETMARK`, and `MESSAGE` macros that
differ from the standard dynarec pass macros. A `#define STEP_PASS` /
`#ifndef STEP_PASS` guard in `dynarec_ppc64le_helper.h` prevents the standard
pass headers (`dynarec_ppc64le_pass3.h`, etc.) from being included and
overwriting these custom macros.

### ResetTable64

The `Table64()` mechanism uses a global hash table for 64-bit constant pooling.
`ResetTable64(&helper)` must be called between passes (before pass 2 and pass 3)
to reset the table state. Missing these calls was a root cause of early crashes.

## Wiring Points

The native block is wired into the dynarec at these locations:

1. **`dynarec_ppc64le_consts.h`**: `const_updateflags_ppc64le` enum value
2. **`dynarec_ppc64le_consts.c`**: `case const_updateflags_ppc64le: return (uintptr_t)create_updateflags();`
3. **`dynarec_ppc64le_helper.h`**: `READFLAGS` and `GRABFLAGS` macros use
   `TABLE64C(x6, const_updateflags_ppc64le); MTCTR(x6); BCTRL();`
4. **`dynarec_ppc64le_helper.c`**: `flagsCacheTransform` uses the same pattern
5. **`CMakeLists.txt`**: `updateflags_ppc64le.c` in `DYNAREC_SRC`,
   `updateflags_ppc64le_pass.c` in `DYNAREC_PASS`

## Dispatch Mechanism

### Prologue

```
MFLR(x7)                              // Save caller's LR to x7
LWZ(x1, offsetof(x64emu_t, df), xEmu) // Load deferred flag type
CMPWI(x1, d_unknown)                  // Bounds check
BLT(+8)                               // If valid, skip return
MTLR(x7)                              // Restore LR
BLR()                                 // Return (invalid df)
```

### Branch Table

```
BCL(20, 31, 4)     // LR = addr(BCL) + 4
MFLR(x2)           // x2 = addr(this instruction)
SLWI(x1, x1, 2)    // x1 = df * 4 (each table entry is 4 bytes)
ADD(x1, x2, x1)    // x1 = x2 + df*4
ADDI(x1, x1, 24)   // Skip past: MFLR + SLWI + ADD + ADDI + MTCTR + BCTR = 6 insns = 24 bytes
MTCTR(x1)
BCTR()              // Jump to table[df]
// table[0] = B(offset_to_d_none_handler)
// table[1] = B(offset_to_d_add8_handler)
// ...up to table[88]
```

Each table entry is a 4-byte `B(offset)` instruction that branches to the
corresponding handler.

### LR Clobbering Issue (PPC64LE-Specific)

On PPC64LE, `BCL(20,31,4)` clobbers LR. The caller's return address (set by
`BCTRL`) would be lost. The fix is to save LR to x7 at the very start of the
block (`MFLR(x7)`) and all handlers end with `MTLR(x7); BLR()`.

This differs from ARM64 where `TABLE64C` does NOT clobber LR, so ARM64 handlers
can simply use `RET(xLR)`.

## Register Conventions

| Register | PPC64LE | Role |
|----------|---------|------|
| x1 | r3 | Scratch, also holds df initially |
| x2 | r4 | Scratch |
| x3 | r5 | Scratch |
| x4 | r6 | Scratch |
| x5 | r7 | Scratch |
| x6 | r8 | Scratch |
| x7 | r10 | Scratch, also holds saved LR |
| xEmu | r31 | x64emu_t pointer (preserved) |
| xFlags | r30 | Current eflags register (modified by handlers) |
| xRIP | r9 | **Caller-saved** — must be saved/restored around C calls |

### xRIP Save/Restore

xRIP (r9) is the ONLY x86 register mapped to a caller-saved PPC64LE register.
The fallback handler saves it to `emu->ip` before calling C `UpdateFlags()` and
restores it after. This matches the `call_c()` pattern used elsewhere.

## Handler Categories

### Handlers Using emit_* Functions (Batches 1-4)

These handlers load operands from the emu struct, clear `emu->df`, then call the
corresponding `emit_*` function with `gen_flags = X_ALL`:

- **CMP/TST** (Batch 1): `emit_cmp8/16/32`, `emit_test8/16/32` — load op1+op2
  (CMP) or op1+res (TST)
- **Arithmetic** (Batch 2): `emit_add/sub/inc/dec/neg` variants — load
  op1+op2+res (add/sub), op1+res (inc/dec), or op1 (neg)
- **Logic** (Batch 3): `emit_and/or/xor` variants — load only res; second
  operand is a constant (all-ones for AND, zero for OR/XOR)
- **Shift** (Batch 4): `emit_shl/shr/sar` variants — load op1+op2; SAR 8/16
  loads op1 sign-extended

#### gen_flags = X_ALL Behavior

With `gen_flags = X_ALL` (0x3F):
- `IFX_PENDOR0` is FALSE (no X_PEND bit), so emit functions do NOT store
  op1/op2/res to emu and do NOT call SET_DF
- All `IFX(X_*)` flag computation blocks execute (compute all 6 flags)
- `CLEAR_FLAGS` clears flag bits in xFlags before recomputing
- **Therefore**: Native handlers must explicitly clear emu->df since emit
  functions won't do it when called with X_ALL

#### df Clearing Strategy

- **Native handlers**: Clear df explicitly (`LI(x3,0); STW(x3, offsetof(df), xEmu)`)
  before calling emit_* function
- **Fallback handler**: Call C `UpdateFlags()` which reads and clears df itself

### Fully Inlined Handlers (Batch 5)

These handlers do NOT call emit_* functions. All flag computation is done inline
with direct bit manipulation on xFlags:

#### MUL/IMUL (d_mul8/16/32/64, d_imul8/16/32/64)

- **MUL**: CF/OF = high bits nonzero. Optional SF/ZF/AF/PF when `!BOX64ENV(cputype)`.
- **IMUL**: CF/OF = sign extension of low result differs from actual high bits.

#### ROL/ROR (d_rol8/16/32/64, d_ror8/16/32/64)

Only set CF and OF (no SF/ZF/AF/PF):
- **ROL**: CF = bit 0 of result; OF = CF XOR MSB of result
- **ROR**: CF = MSB of result; OF = XOR of two MSBs of result

#### SBB (d_sbb8/16/32/64)

Borrow chain: `CC = (~op1 & op2) | (res & (~op1 | op2))`
- CF = CC[MSB], AF = CC[3], OF = XOR of top two CC bits
- SF from res MSB, ZF from res==0, PF via emit_pf

#### ADC (d_adc8/8b/16/16b/32/32b/64)

Carry chain: `CC = (op1 & op2) | (~res & (op1 | op2))`

**Non-b variants** (d_adc8, d_adc16, d_adc32): res is stored wider
(16/32/64 bit), carry bit directly accessible at bit 8/16/32.

**b variants** (d_adc8b, d_adc16b, d_adc32b): res is stored truncated, carry
must be reconstructed by detecting whether `carry_in` was 0 or 1:
- `carry_in = (res != (uint_N_t)(op1 + op2)) ? 1 : 0`
- Recompute wider result: `wider = op1 + op2 + carry_in`
- CF from overflow bit of wider result

**d_adc64**: Always reconstructs carry via hi/lo split (no 128-bit register).

#### SHLD/SHRD (d_shld16/32/64, d_shrd16/32/64)

**SHRD** (double-precision shift right): `dest = (dest >> count) | (src << (N-count))`
- CF = `(op1 >> (cnt-1)) & 1`
- OF depends on `BOX64ENV(cputype)`:
  - cputype: `XOR2(res >> (N-2))` — XOR of top two result bits
  - !cputype: `((res >> (N-cnt)) ^ (op1 >> (N-1))) & 1`

**SHLD** (double-precision shift left): `dest = (dest << count) | (src >> (N-count))`
- CF = `(op1 >> (N - cnt)) & 1`
- OF depends on `BOX64ENV(cputype)`:
  - cputype: `(CF ^ (res >> (N-1))) & 1`
  - !cputype: `XOR2(op1 >> (N-2))`

**d_shrd16 semantics note**: The ARM64 implementation has a confirmed bug
(inverted CBZw/CBNZw, reported as Issue #3670). The PPC64LE implementation uses
correct semantics matching the C reference: OF/AF are unconditional (outside
`if(cnt>0)`), CF/SF/ZF/PF only when cnt>0.

**Early-return pattern for cnt==0**:
```
CMPWI(x3, 0);
BNE(12);       // if cnt!=0, skip past return
MTLR(x7);
BLR();         // early return when cnt==0
```

## PPC64LE Flag Manipulation Primitives

### Flag Bit Positions (same as x86 EFLAGS)

| Flag | Bit | Constant |
|------|-----|----------|
| CF | 0 | F_CF |
| PF | 2 | F_PF |
| AF | 4 | F_AF |
| ZF | 6 | F_ZF |
| SF | 7 | F_SF |
| OF | 11 | F_OF |

### Key Operations

- **Insert bit into xFlags**: `BF_INSERT(xFlags, src, N, N)` — inserts bit 0 of
  `src` into bit N of xFlags. Implemented as `RLDIMI`.
- **Clear flag bit**: `BF_INSERT(xFlags, xZR, N, N)` — but since r0 is NOT the
  zero register on PPC64LE, this loads 0 into a temp register first.
- **Clear all flags**: `CLEAR_FLAGS(s)` — `MOV64x(s, mask); ANDC(xFlags, xFlags, s)`
- **Set AF**: `ORI(xFlags, xFlags, 1 << F_AF)`
- **Parity flag**: `emit_pf(dyn, ninst, s1, s3, s4)` — computes parity of low
  byte of s1.

### PPC64LE Conditional Select

`ISEL(Rt, Ra, Rb, BC)` — if CR bit BC is set: RT=RA (RA=0 means literal 0);
else RT=RB. This is the PPC64LE equivalent of ARM64's CSEL/CSET.

Pattern for CSET (set to 1 if condition true):
```
LI(rd, 1);
ISEL(rd, rd, 0, BI(0, CR_EQ));  // rd = 1 if EQ, else 0
```

## TABLE64C in the Pass File

The fallback handler uses `TABLE64C` to load the C function address. Because the
pass file has its own EMIT/SETMARK macros (not the standard pass headers),
TABLE64C is redefined within the pass file for each STEP:

- **Pass 0**: Counts 16 bytes (4 instructions: BCL + MFLR + ADDIS + LD)
- **Pass 1**: No-op
- **Pass 2**: Registers constant with `Table64()` and counts 16 bytes
- **Pass 3**: Full BCL/MFLR/ADDIS/LD emission with `Table64()` for offset
  computation. Uses `PPC64_HI16` and `PPC64_LO16` helper macros.

## b-Variant Aliases

Several df types come in pairs (e.g., d_add8 / d_add8b). In the ARM64
implementation, both variants share identical handler code — both SETMARKs point
to the same code address. PPC64LE replicates this pattern.

## Deferred Flag Types (complete enum, src/include/regs.h)

d_none=0 through d_shld64=88, d_unknown=89.

All 89 types (0-88) have native SETMARK handlers. The fallback path (calling C
`UpdateFlags()`) is retained as a safety net but is effectively dead code.

## Testing

Seven NASM test suites cover all handler categories:

| Test File | Handlers Covered | Test Count |
|-----------|-----------------|------------|
| `test_updateflags.asm` | CMP, TST | 44 |
| `test_updateflags_arith.asm` | ADD, SUB, INC, DEC, NEG | 48 |
| `test_updateflags_logic.asm` | AND, OR, XOR | 33 |
| `test_updateflags_shift.asm` | SHL, SHR, SAR, ROL, ROR | 37 |
| `test_updateflags_carry.asm` | SBB, ADC | 31 |
| `test_updateflags_mul.asm` | MUL, IMUL | 22 |
| `test_updateflags_shld.asm` | SHLD, SHRD | 34 |
| **Total** | | **249** |

All tests are run in both dynarec (`BOX64_DYNAREC=1`) and interpreter
(`BOX64_DYNAREC=0`) modes. Both must produce identical output and exit 0.

The test framework (`test_framework.inc`) provides macros for flag checking that
force flag materialization across basic block boundaries, ensuring the native
UpdateFlags block is exercised rather than inline emit_* functions.
