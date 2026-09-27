// SPDX-License-Identifier: MIT

/* ARM64EC dispatch bridges for box64ec.dll
 * Implements documented __os_arm64x_* helpers exported as ExitToX64 / DispatchJump /
 * RetToEntryThunk / BeginSimulation
 */

    .text
    .balign 16

    .extern Box64EC_EnterX64
    .extern Box64EC_BeginSimulation
    .extern Box64EC_TakePendingNativeExit
    .extern Box64EC_ResolveNoPendingExit
    .extern Box64EC_FatalEnterJit
    .extern Box64EC_X64ReturnInstr
    .extern Box64EC_WineSyscallDispatcher
    .extern Box64EC_WineNtContinueSyscallId
    .extern Box64EC_CaptureNeonToContext
    .extern Box64EC_ApplyContextMxcsr
    .extern Box64EC_NoteEntryFrame
    .extern Box64EC_NoteEnterBoundary
    .extern Box64EC_PrepareNestedEnter
    .extern Box64EC_ClearInSimulationIfOutermost
    .extern Box64EC_EcCodeBitmapLimit


    /* ARM64EC_NT_CONTEXT field offsets (Wine winnt.h) */
    .equ CTX_EFLAGS, 0x44
    .equ CTX_X8,     0x78   /* Rax */
    .equ CTX_X0,     0x80   /* Rcx */
    .equ CTX_X1,     0x88   /* Rdx */
    .equ CTX_X27,    0x90   /* Rbx */
    .equ CTX_SP,     0x98   /* Rsp */
    .equ CTX_FP,     0xa0   /* Rbp */
    .equ CTX_X25,    0xa8   /* Rsi */
    .equ CTX_X26,    0xb0   /* Rdi */
    .equ CTX_X2,     0xb8   /* R8  */
    .equ CTX_X3,     0xc0   /* R9  */
    .equ CTX_X4,     0xc8   /* R10 */
    .equ CTX_X5,     0xd0   /* R11 */
    .equ CTX_X19,    0xd8   /* R12 */
    .equ CTX_X20,    0xe0   /* R13 */
    .equ CTX_X21,    0xe8   /* R14 */
    .equ CTX_X22,    0xf0   /* R15 */
    .equ CTX_PC,     0xf8   /* Rip */

/* ------------------------------------------------------------------ */
/* Shared enter path. x9 = RIP. SP = guest stack (LR already pushed).  */
/* ------------------------------------------------------------------ */
enter_jit:
    ldr     x17, [x18, #0x1788]          /* TEB->ChpeV2CpuAreaInfo */
    /* On nested ExitToX64, do not snapshot emu SP into Context Sp/Fp. */
    ldrb    w16, [x17]                   /* InSimulation */
    mov     w7, w16                      /* preserve pre-entry state for trace */
    cbnz    w16, .Lenter_jit_nested
    mov     w16, #1
    strb    w16, [x17]                   /* InSimulation = 1 */
    ldr     x16, [x17, #0x18]            /* ContextAmd64 */
    cbz     x16, .Lenter_jit_fail

    /* Snapshot live EC buddy GPRs → ContextAmd64 (Sp/Fp last; see below). */
    str     x8,  [x16, #CTX_X8]
    str     x0,  [x16, #CTX_X0]
    str     x1,  [x16, #CTX_X1]
    str     x27, [x16, #CTX_X27]
    str     x25, [x16, #CTX_X25]
    str     x26, [x16, #CTX_X26]
    str     x2,  [x16, #CTX_X2]
    str     x3,  [x16, #CTX_X3]
    str     x4,  [x16, #CTX_X4]
    str     x5,  [x16, #CTX_X5]
    str     x19, [x16, #CTX_X19]
    str     x20, [x16, #CTX_X20]
    str     x21, [x16, #CTX_X21]
    str     x22, [x16, #CTX_X22]
    str     x9,  [x16, #CTX_PC]
    /* Never write EmulatorStack* into Context Sp (poisons guest RSP).
     * Happens if RTS cleared InSimulation then Wine ExitToX64's a VEH
     * while still sitting on the emu stack. */
    ldr     x1,  [x17, #0x8]             /* EmulatorStackBase */
    ldr     x3,  [x17, #0x10]            /* EmulatorStackLimit */
    mov     x0,  sp
    mov     w5,  #0                      /* SP is on the guest stack */
    cmp     x0,  x3
    b.lo    2f                           /* sp < Limit → guest */
    cmp     x0,  x1
    b.hi    2f                           /* sp > Base → guest */
    /* Native callback on the emulator stack: make the x64 call on the
     * saved guest stack instead. */
    mov     w5,  #1
    cbz     w6, 3f
    cmp     w6, #2
    b.hi    3f
    ldr     x0, [x16, #CTX_SP]
    sub     x0, x0, #8
    str     x10, [x0]
    str     x0, [x16, #CTX_SP]
    b       3f
2:
    str     x0,  [x16, #CTX_SP]
    str     x29, [x16, #CTX_FP]
3:
    ldr     w0,  [x16, #CTX_EFLAGS]
    cbnz    w0,  1f
    mov     w0,  #0x202
    str     w0,  [x16, #CTX_EFLAGS]
1:
    b       .Lenter_jit_run

.Lenter_jit_nested:
    ldr     x16, [x17, #0x18]
    cbz     x16, .Lenter_jit_fail

    /* Snapshot buddy GPRs + PC BEFORE any C call (volatiles). */
    str     x8,  [x16, #CTX_X8]
    str     x0,  [x16, #CTX_X0]
    str     x1,  [x16, #CTX_X1]
    str     x27, [x16, #CTX_X27]
    str     x25, [x16, #CTX_X25]
    str     x26, [x16, #CTX_X26]
    str     x2,  [x16, #CTX_X2]
    str     x3,  [x16, #CTX_X3]
    str     x4,  [x16, #CTX_X4]
    str     x5,  [x16, #CTX_X5]
    str     x19, [x16, #CTX_X19]
    str     x20, [x16, #CTX_X20]
    str     x21, [x16, #CTX_X21]
    str     x22, [x16, #CTX_X22]
    str     x9,  [x16, #CTX_PC]

    /* SP/FP only when host SP is guest stack (not EmulatorStack*). */
    ldr     x1,  [x17, #0x8]             /* EmulatorStackBase */
    ldr     x2,  [x17, #0x10]            /* EmulatorStackLimit */
    mov     x0,  sp
    mov     w5,  #0                      /* SP is on the guest stack */
    cmp     x0,  x2
    b.lo    1f                           /* sp < Limit → guest */
    cmp     x0,  x1
    b.hi    1f                           /* sp > Base → guest */
    /* Native callback on the emulator stack: synthesize its x64 call on
     * the saved guest stack. */
    mov     w5, #1
    cbz     w6, 2f
    cmp     w6, #2
    b.hi    2f
    ldr     x0, [x16, #CTX_SP]
    sub     x0, x0, #8
    str     x10, [x0]
    str     x0, [x16, #CTX_SP]
    b       2f
1:
    str     x0,  [x16, #CTX_SP]
    str     x29, [x16, #CTX_FP]
2:
    /* Do NOT call C here: guest SP is 8-mod-16 after ExitToX64's LR push.
     * Nest log runs after the emu-stack switch below. */
    b       .Lenter_jit_run

.Lenter_jit_run:
    /* Switch to Wine emulator stack BEFORE any C call (guest SP is often
     * 8-mod-16 after the LR push; C ABI needs 16-byte SP). */
    ldr     x1,  [x17, #0x8]             /* EmulatorStackBase */
    cbz     x1,  .Lenter_jit_fail
    mov     x4,  x1                      /* keep stack base for range check */

     /* Experimental: keep this activation below a live Wine callback frame. */
    ldr     x3,  [x17, #0x10]            /* EmulatorStackLimit */
    cmp     x11, x3
    b.lo    .Lenter_jit_stack_base       /* physical SP is not EmulatorStack */
    cmp     x11, x4
    b.hi    .Lenter_jit_stack_base
    bic     x2,  x11, #0xf
    cmp     x2,  x1
    csel    x1,  x2, x1, lo              /* stack grows toward lower addresses */
.Lenter_jit_stack_base:
    sub     x2,  x1, #0x90
    cmp     x2,  x3
    b.ls    .Lenter_jit_fail             /* no committed stack remains */
    ldr     x2,  [x16, #CTX_SP]          /* parent x64 stack */
    mov     sp,  x1
    b       enter_jit_frame

/* The machine frame makes the stack switch visible to Windows unwinding.
 * Its parent is the x64 state saved in ContextAmd64. */
    .seh_proc enter_jit_frame
enter_jit_frame:
    stp     x2,  x9,  [sp, #-16]!        /* parent x64 SP, PC */
    .seh_pushframe
    stp     x19, x20, [sp, #-0x80]!
    .seh_save_r19r20_x 0x80
    stp     x21, x22, [sp, #0x10]
    .seh_save_regp x21, 0x10
    stp     x29, x30, [sp, #0x20]
    .seh_save_fplr 0x20
    mov     x29, sp
    .seh_set_fp
    .seh_endprologue

    /* Save entry-only values after the guest nonvolatiles are protected. */
    stp     x10, x11, [sp, #0x30]        /* pushed slot, physical entry SP */
    stp     x6,  x7,  [sp, #0x40]        /* entry route, prior InSimulation */
    stp     x5,  xzr, [sp, #0x50]        /* entry was already on emu stack */
    stp     x9,  x30, [sp, #0x60]        /* RIP, entry LR */
    mov     x19, x16                     /* keep ContextAmd64* */
    cmp     w7, #0
    cset    w20, eq                      /* cold enter: NoteEntryFrame */
    mov     w21, w5
    /* Nested path: log only (SP/FP already decided). Still on emu stack. */
    cbnz    w20, 4f
    ldr     x0,  [x19, #CTX_SP]          /* guest Sp (or prior if on_emu) */
    /* If on_emu, CTX_SP was left alone - pass EmulatorStackBase for log. */
    cbz     w21, 5f
    ldr     x17, [x18, #0x1788]
    ldr     x0,  [x17, #0x8]
5:
    ldr     x1,  [sp, #0x60]             /* RIP */
    bl      Box64EC_PrepareNestedEnter
    ldr     x17, [x18, #0x1788]
4:
    /* Entry frame = guest Sp + LR (for SEH trampoline); skip if nested. */
    cbz     w20, 6f
    ldr     x0, [x19, #CTX_SP]
    ldr     x1, [sp, #0x68]              /* entry LR */
    bl      Box64EC_NoteEntryFrame
6:
    /* Capture live NEON volatiles (XMM0-5) into Context. x0 must be
     * ContextAmd64* - NoteEntryFrame/PrepareNestedEnter leave SP or 0/1. */
    mov     x0, x19                      /* ContextAmd64* */
    bl      Box64EC_CaptureNeonToContext

    /* Record the boundary only after volatile NEON state is safe in Context. */
    ldr     x0, [sp, #0x40]              /* route */
    ldr     x1, [sp, #0x60]              /* x64 RIP */
    ldr     x2, [x19, #CTX_SP]           /* guest SP */
    ldr     x3, [sp, #0x68]              /* incoming LR */
    ldr     x4, [x2]                     /* return slot at x64 entry */
    ldr     x5, [sp, #0x48]              /* prior InSimulation */
    ldr     x6, [sp, #0x38]              /* physical SP at thunk entry */
    ldr     x7, [sp, #0x30]              /* slot immediately after push */
    bl      Box64EC_NoteEnterBoundary

    ldr     x0,  [sp, #0x60]             /* RIP */
    bl      Box64EC_EnterX64
    sub     sp, sp, #16
    mov     x0, sp
    bl      Box64EC_TakePendingNativeExit
    ldr     x12, [sp]
    add     sp, sp, #16
    mov     x15, x0                      /* pending native target (or 0) */
    str     x12, [sp, #0x70]

    /* Resolve the target while the unwindable emulator frame is active. */
    cbnz    x15, 7f
    ldr     x0, [x19, #CTX_PC]
    bl      Box64EC_ResolveNoPendingExit
    mov     x15, x0
    cbnz    x15, 7f
    mov     w0, #0xec02
    bl      Box64EC_FatalEnterJit
    b       .
7:
    str     x15, [sp, #0x78]

    /* Restore guest state onto live EC buddies */
    ldr     x17, [x18, #0x1788]
    ldr     x16, [x17, #0x18]
    /* Still on emu stack - safe for C (must clear BEFORE guest SP restore:
     * guest SP is typically 8-mod-16 → DATATYPE_MISALIGNMENT on AAPCS64). */
    mov     x0, x16
    bl      Box64EC_ApplyContextMxcsr
    bl      Box64EC_ClearInSimulationIfOutermost
    ldp     x12, x15, [sp, #0x70]
    mov     x16, x19

    /* Restore SIMD after the last C call. */
    ldp     q0,  q1,  [x16, #0x1a0]
    ldp     q2,  q3,  [x16, #0x1c0]
    ldp     q4,  q5,  [x16, #0x1e0]
    ldp     q6,  q7,  [x16, #0x200]
    ldp     q8,  q9,  [x16, #0x220]
    ldp     q10, q11, [x16, #0x240]
    ldp     q12, q13, [x16, #0x260]
    ldp     q14, q15, [x16, #0x280]

    ldr     x8,  [x16, #CTX_X8]
    ldr     x0,  [x16, #CTX_X0]
    ldr     x1,  [x16, #CTX_X1]
    ldr     x27, [x16, #CTX_X27]
    ldr     x25, [x16, #CTX_X25]
    ldr     x26, [x16, #CTX_X26]
    ldr     x2,  [x16, #CTX_X2]
    ldr     x3,  [x16, #CTX_X3]
    ldr     x4,  [x16, #CTX_X4]
    ldr     x5,  [x16, #CTX_X5]
    ldr     x6,  [x16, #CTX_FP]
    ldr     x19, [x16, #CTX_X19]
    ldr     x20, [x16, #CTX_X20]
    ldr     x21, [x16, #CTX_X21]
    ldr     x22, [x16, #CTX_X22]
    ldr     x9,  [x16, #CTX_PC]
    ldr     x17, [x16, #CTX_SP]
    mov     x29, x6
    cbz     x12, 8f
    mov     sp, x12                      /* resume Wine callback stack */
    b       9f
8:
    mov     sp, x17                      /* back to guest stack */
9:
    mov     x9,  x15
    b       ExitFunctionEC
    .seh_endproc

.Lenter_jit_fail:
    mov     x16, sp
    bic     sp, x16, #0xf
    mov     w0, #0xec01
    bl      Box64EC_FatalEnterJit
    b       .

    /* __os_arm64x_dispatch_call_no_redirect - exit thunk enters x64 (RIP in x9) */
    .globl ExitToX64
ExitToX64:
    /* Push LR as the x64 return address. */
    mov     w6, #1                       /* trace route: ExitToX64 */
    str     x30, [sp, #-8]!
    mov     x11, sp                      /* physical SP after native push */
    mov     x10, sp
    ldr     x10, [x10]
    b       enter_jit

    /* __os_arm64x_x64_jump - RIP in x9; check EcCodeBitMap */
    .globl DispatchJump
DispatchJump:
    mov     w6, #2                       /* trace route: DispatchJump */
    str     x30, [sp, #-8]!
    mov     x11, sp                      /* physical SP after native push */
    mov     x10, sp
    ldr     x10, [x10]
    adrp    x16, Box64EC_EcCodeBitmapLimit
    ldr     x16, [x16, :lo12:Box64EC_EcCodeBitmapLimit]
    cmp     x9, x16
    b.hs    enter_jit                     /* outside the EC bitmap */
    ldr     x16, [x18, #0x60]            /* TEB->PEB */
    ldr     x16, [x16, #0x368]           /* PEB->EcCodeBitMap */
    cbz     x16, enter_jit               /* no bitmap → treat as x64 */
    lsr     x17, x9, #18
    ldr     x16, [x16, x17, lsl #3]
    lsr     x17, x9, #12
    lsr     x16, x16, x17
    tbnz    x16, #0, .Ldispatch_ec
    b       enter_jit

.Ldispatch_ec:
    ldr     x30, [sp], #8
    /* Direct DispatchJump-to-EC bypasses ExitFunctionEC.  An exit-thunk
     * continuation (the instruction after `blr x16`) requires native ARM64
     * stack alignment, so stop at the boundary before USER32 faults later. */
    ldursw  x16, [x9, #-4]
    mov     w17, #0x200
    movk    w17, #0xd63f, lsl #16        /* encoding of blr x16 */
    cmp     w16, w17
    b.ne    1f
    mov     x16, sp
    tbz     x16, #3, 1f
    mov     x0, x9                       /* bad native continuation */
    mov     x1, sp
    mov     x2, x30
    brk     #0xec06
1:
    br      x9

    /* __os_arm64x_dispatch_ret - return to x64 (addr in LR).
     * SP is already correct for post-CALL x64 (ExitFunctionEC popped the
     * retaddr into LR). Do NOT push again - that desyncs guest RSP and
     * breaks later stack args (CreateFileA → ERROR_INVALID_PARAMETER).
     * Check the target before entering the JIT. */
    .globl RetToEntryThunk
RetToEntryThunk:
    mov     w6, #3                       /* trace route: RetToEntryThunk */
    mov     x9, x30
    mov     x11, sp                      /* physical SP, no push on this route */
    mov     x10, sp
    ldr     x10, [x10]
    mov     w6, #3

     /* Shared: x9 = target; SP unchanged. */
.Lcheck_target_ec:
    adrp    x16, Box64EC_EcCodeBitmapLimit
    ldr     x16, [x16, :lo12:Box64EC_EcCodeBitmapLimit]
    cmp     x9, x16
    b.hs    enter_jit                     /* outside the EC bitmap */
    ldr     x16, [x18, #0x60]            /* TEB->PEB */
    ldr     x16, [x16, #0x368]           /* PEB->EcCodeBitMap */
    cbz     x16, enter_jit               /* no bitmap → treat as x64 */
    lsr     x17, x9, #18
    ldr     x16, [x16, x17, lsl #3]
    lsr     x17, x9, #12
    lsr     x16, x16, x17
    tbnz    x16, #0, .Ldispatch_ec_ret
    b       enter_jit

.Ldispatch_ec_ret:
    /* Exit-thunk continuations require a 16-byte-aligned native stack. */
    ldursw  x16, [x9, #-4]
    mov     w17, #0x200
    movk    w17, #0xd63f, lsl #16        /* encoding of blr x16 */
    cmp     w16, w17
    b.ne    ExitFunctionEC
    mov     x16, sp
    tbz     x16, #3, ExitFunctionEC
    mov     x0, x9                       /* bad native continuation */
    mov     x1, sp                       /* x64-aligned guest SP */
    mov     x2, x30                      /* RetToEntryThunk incoming LR */
    brk     #0xec04
    b       ExitFunctionEC

    /* BeginSimulation - KiUserEmulationDispatcher resume (ContextAmd64 filled) */
    .globl BeginSimulation
BeginSimulation:
    ldr     x17, [x18, #0x1788]
    mov     w16, #1
    strb    w16, [x17]
    ldr     x16, [x17, #0x8]             /* EmulatorStackBase */
    cbz     x16, 1f
    mov     sp, x16
1:
    /* Entry frame for SEH while on emulator stack */
    mov     x0, sp
    mov     x1, x30
    bl      Box64EC_NoteEntryFrame

    /* The dispatcher never returns across this stack-switching boundary. */
    b       Box64EC_BeginSimulation

    /* Complete BeginSimulation without another call after restoring guest SP.
     * x0 = ContextAmd64, x1 = native target, x2 = native callback SP. */
    .globl "#Box64EC_FinishBeginSimulation"
"#Box64EC_FinishBeginSimulation":
    mov     x16, x0
    mov     x15, x1
    mov     x12, x2
    ldr     x17, [x18, #0x1788]
    cbz     x16, .Lbegin_finish_invalid
    /* Keep the ContextAmd64 restore tail free of calls.  Wine can resume an
     * interrupted ARM64EC call boundary with the guest SP, so a saved x16 on
     * the emulator stack is no longer reachable. */
    ldp     q0,  q1,  [x16, #0x1a0]
    ldp     q2,  q3,  [x16, #0x1c0]
    ldp     q4,  q5,  [x16, #0x1e0]
    ldp     q6,  q7,  [x16, #0x200]
    ldp     q8,  q9,  [x16, #0x220]
    ldp     q10, q11, [x16, #0x240]
    ldp     q12, q13, [x16, #0x260]
    ldp     q14, q15, [x16, #0x280]

    ldr     x8,  [x16, #CTX_X8]
    ldr     x0,  [x16, #CTX_X0]
    ldr     x1,  [x16, #CTX_X1]
    ldr     x27, [x16, #CTX_X27]
    ldr     x29, [x16, #CTX_FP]
    ldr     x25, [x16, #CTX_X25]
    ldr     x26, [x16, #CTX_X26]
    ldr     x2,  [x16, #CTX_X2]
    ldr     x3,  [x16, #CTX_X3]
    ldr     x4,  [x16, #CTX_X4]
    ldr     x5,  [x16, #CTX_X5]
    ldr     x19, [x16, #CTX_X19]
    ldr     x20, [x16, #CTX_X20]
    ldr     x21, [x16, #CTX_X21]
    ldr     x22, [x16, #CTX_X22]
    ldr     x9,  [x16, #CTX_PC]
    ldr     x17, [x16, #CTX_SP]
    cbz     x12, 4f
    mov     sp, x12
    b       2f
4:
    mov     sp, x17
2:
    ldr     x17, [x18, #0x1788]
    strb    wzr, [x17]

    cbz     x15, 3f
    mov     x9, x15
    b       ExitFunctionEC
3:
    /* BeginSimulation is a one-way dispatcher entry. */
.Lbegin_finish_invalid:
    mov     x16, sp
    bic     sp, x16, #0xf
    mov     w0, #0xec07
    bl      Box64EC_FatalEnterJit
    b       .

    /* Leave simulation to native EC (entry-thunk or exit-thunk return).
     * Expects target in x9; guest SP already restored.
     * SP alignment: x64 CALL leaves SP 8-mod-16; ARM64EC needs 16-byte SP.
     * Check the doorbell before SP mutation so
     * suspend resume can safely re-run target resolution. */
    .globl ExitFunctionEC
    .globl ExitFunctionSuspendPoint
    .globl ExitFunctionSuspendResumePoint
ExitFunctionEC:
    /* Clear AFP NEP+AH - native EC code will not expect them */
    mrs     x17, fpcr
    and     x17, x17, #~6
    msr     fpcr, x17

    ldr     x17, [x18, #0x1788]
    strb    wzr, [x17]                   /* InSimulation = 0 */

    /* Cooperative suspend: Wine may set the doorbell around exit.
     * Must run before SP/LR mutation. */
    ldr     x16, [x17, #0x20]            /* SuspendDoorbell* */
    cbz     x16, ExitFunctionSuspendResumePoint
    ldr     w16, [x16]
    cbnz    w16, ExitFunctionSuspendPoint

ExitFunctionSuspendResumePoint:
    mov     x17, x9                      /* native target */

    mov     w16, #0x200
    movk    w16, #0xd63f, lsl #16        /* encoding of blr x16 */
    ldursw  x6, [x17, #-4]
    cmp     w6, w16
    b.ne    .Lexit_ec_export

    /* Return into an exit-thunk continuation. */
    mov     x6, sp
    tbz     x6, #3, .Lexit_ec_br
    mov     x0, x17                      /* bad native continuation */
    mov     x1, sp
    mov     x2, x30
    brk     #0xec05
    b       .Lexit_ec_br

    /* Exported EC function - entry thunk offset is s32 at target-4 */
.Lexit_ec_export:
    and     x6, x6, #~3
    add     x17, x17, x6

    mov     x4, sp
    tbz     x4, #3, .Lexit_ec_misaligned /* SP 16-aligned ⇒ x64 had odd alignment */
    /* Normal after x64 CALL: SP is 8-mod-16. Pop retaddr into LR, SP now 16-aligned. */
    ldr     x30, [x4], #8
    mov     sp, x4
    b       .Lexit_ec_br

.Lexit_ec_misaligned:
    /* Leave retaddr on stack; EC returns to a single x64 `ret` (0xC3). */
    adrp    x16, Box64EC_X64ReturnInstr
    ldr     x30, [x16, :lo12:Box64EC_X64ReturnInstr]

.Lexit_ec_br:
    br      x17

    /* Suspend trap - ResetToConsistentState redirects Pc to resume point */
ExitFunctionSuspendPoint:
    brk     #0xCAFE

    /* Continue into KiUser (or any PC) with SP = frame.
     * x0 = PC, x1 = SP. Used by STATUS_EMULATION_SYSCALL (H62). */
    .globl Box64EC_JumpSetStack
Box64EC_JumpSetStack:
    mov     sp, x1
    br      x0

    /* ------------------------------------------------------------------ */
    /* NtContinue with full ARM64_NT_CONTEXT (Wine dispatcher or SVC).     */
    /* x0 = ARM64_NT_CONTEXT*, x1 = Alert                                  */
    /* ------------------------------------------------------------------ */
    .globl Box64EC_NtContinueNative
Box64EC_NtContinueNative:
    adrp    x16, Box64EC_WineSyscallDispatcher
    ldr     x16, [x16, :lo12:Box64EC_WineSyscallDispatcher]
    cbz     x16, 1f
    mov     x9, x30
    adrp    x8, Box64EC_WineNtContinueSyscallId
    ldr     x8, [x8, :lo12:Box64EC_WineNtContinueSyscallId]
    blr     x16                          /* Wine dispatcher; retaddr in x9 */
    ret
1:
    mov     x8, #0x43                    /* Windows NtContinue */
    svc     #0
    ret
