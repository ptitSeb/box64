// SPDX-License-Identifier: MIT
#include <stdint.h>
#include <windows.h>
#include <ntstatus.h>
#include <winternl.h>

#include "box64ec.h"
#include "box64ec_private.h"
#include "box64cpu.h"
#include "debug.h"
#include "env.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "wine/compiler.h"

NTSYSAPI void NTAPI RtlExitUserProcess(NTSTATUS status);
NTSYSAPI NTSTATUS NTAPI NtTerminateProcess(HANDLE process, NTSTATUS status);

int Box64EC_IsPrivilegedInstruction(void* addr)
{
    uintptr_t rip = addr ? (uintptr_t)addr : 0;
    const uint8_t* code;
    uint8_t op;
    unsigned length = 0;

    if (rip < 0x10000ull || (rip >> BOX64EC_VA_BITS))
        return 0;
    code = (const uint8_t*)rip;
    do {
        op = *(volatile const uint8_t*)(code + length++);
    } while (length < 15 &&
             (op == 0x26 || op == 0x2e || op == 0x36 || op == 0x3e ||
              (op >= 0x40 && op <= 0x4f) || op == 0x64 || op == 0x65 ||
              op == 0x66 || op == 0x67 || op == 0xf0 || op == 0xf2 ||
              op == 0xf3));

    if ((op >= 0x6c && op <= 0x6f) ||
        (op >= 0xe4 && op <= 0xef) ||
        op == 0xf4 || op == 0xfa || op == 0xfb)
        return 1;
    if (op != 0x0f || length == 15)
        return 0;
    op = *(volatile const uint8_t*)(code + length);
    return op == 0x06 || op == 0x08 || op == 0x09 ||
           (op >= 0x20 && op <= 0x23) || op == 0x30 || op == 0x32 ||
           op == 0x34 || op == 0x35;
}

uintptr_t Box64EC_ResolveNoPendingExit(uintptr_t rip)
{
    CHPE_V2_CPU_AREA_INFO* area = Box64EC_GetCpuArea();
    x64emu_t* emu = area && area->EmulatorData[EC_DATA_EMU]
                        ? (x64emu_t*)area->EmulatorData[EC_DATA_EMU]
                        : NULL;
    int is_ec = rip && Box64EC_IsEcCode(rip);

    printf_log(LOG_DEBUG, "box64ec: EnterX64 returned without pending native exit "
               "(rip=%p ec=%d rsp=%p rax=%p)%s\n",
               (void*)rip, is_ec,
               emu ? (void*)(uintptr_t)R_RSP : NULL,
               emu ? (void*)(uintptr_t)R_RAX : NULL,
               is_ec ? " - treating RIP as EC exit" : "");
    if (is_ec)
        return rip;
    /* Avoid brk → Wine virtual_unwind infinite loop on enter_jit frames. */
    Box64EC_FatalEnterJit(0xec02);
    return 0;
}

/* Refuse nested/failed enter_jit without brk (brk #0xec0x → SEH flood / hang). */
void Box64EC_FatalEnterJit(unsigned reason)
{
    CHPE_V2_CPU_AREA_INFO* area = Box64EC_GetCpuArea();
    box64ec_thr_t* t = Box64EC_GetThreadState();
    static unsigned last_reason;
    static int flood;

    /* ExitToX64 pushes LR with an 8-byte store - SP may be 8-mod-16 here.
     * ARM64 C ABI needs 16-byte SP or the prologue faults MISALIGNMENT. */
    __asm__ __volatile__("mov x16, sp\n\tbic sp, x16, #0xf" ::: "x16", "memory");

    if (reason == last_reason && flood > 0) {
        /* Already logged this reason - terminate quietly. */
    } else {
        last_reason = reason;
        flood = 1;
        printf_log(LOG_DEBUG, "box64ec: fatal enter_jit reason=#0x%x InSimulation=%d "
                         "entry_sp=%p entry_pc=%p - terminating (no brk)\n",
                         reason,
                         area ? (int)area->InSimulation : -1,
                         t ? (void*)(uintptr_t)t->entry.sp : NULL,
                         t ? (void*)(uintptr_t)t->entry.pc : NULL);
    }
    if (area)
        area->InSimulation = FALSE;
    NtTerminateProcess((HANDLE)(intptr_t)-1, STATUS_ILLEGAL_INSTRUCTION);
    RtlExitUserProcess(STATUS_ILLEGAL_INSTRUCTION);
}

static void run_emu(x64emu_t* emu)
{
    if (Box64EC_PollSuspend()) {
        Box64EC_HandleSuspend(emu);
        return;
    }

    Box64EC_ApplyLiveMxcsr(emu);
    Run(emu, 0);
}

void Box64EC_ResetAbandonedEmuRun(box64ec_thr_t* state, x64emu_t* emu)
{
    if (state)
        state->enter_depth = 0;
    if (!emu)
        return;
    emu->jmpbuf = NULL;
    emu->flags.jmpbuf_ready = 0;
}

void Box64EC_EnterX64(uint64_t rip)
{
    CHPE_V2_CPU_AREA_INFO* area = Box64EC_GetCpuArea();
    x64emu_t* emu = area ? (x64emu_t*)area->EmulatorData[EC_DATA_EMU] : NULL;
    box64ec_thr_t* thr;
    int nested;

    if (!area || !emu || !area->ContextAmd64) {
        RtlRaiseStatus(STATUS_INVALID_PARAMETER);
        return;
    }

    thr = Box64EC_GetThreadState();
    nested = thr ? thr->enter_depth : 0;
    if (thr) {
        thr->enter_depth++;
        /* Only the outermost entry clears pending - nested SEH/CALL_S must
         * not wipe an in-flight ExitFunctionEC target. */
        if (!nested) {
            thr->pending_exit = 0;
            thr->pending_native_sp = 0;
            thr->pending_callback_depth = 0;
            thr->pending_callback_seq = 0;
        }
    }
    area->InSimulation = TRUE;

    process_pending_cross_process_work();
    context_to_emu(area->ContextAmd64, emu);
    R_RIP = rip;

    emu->quit = 0;

    if (!R_RSP) {
        printf_log(LOG_DEBUG, "box64ec EnterX64: guest RSP is NULL, aborting\n");
        finish_enter(thr);
        RtlRaiseStatus(STATUS_INVALID_PARAMETER);
        return;
    }
    if (Box64EC_IsEmulatorStackAddress(R_RSP)) {
        /* RtlRaiseStatus here nests SEH on the emu stack → virtual_unwind hang. */
        printf_log(LOG_DEBUG, "box64ec EnterX64: guest RSP %p is emulator stack - abort\n",
                         (void*)(uintptr_t)R_RSP);
        finish_enter(thr);
        NtTerminateProcess((HANDLE)(intptr_t)-1, STATUS_ACCESS_VIOLATION);
        RtlExitUserProcess(STATUS_ACCESS_VIOLATION);
        return;
    }

    run_emu(emu);


    /* Keep InSimulation=1 until asm TakePending + ExitFunctionEC / enter_jit
     * restore path clears it - prevents nested EnterX64 wiping pending_exit. */
    emu_to_context(emu, area->ContextAmd64);
    finish_enter(thr);
}

void Box64EC_BeginSimulation(void)
{
    CHPE_V2_CPU_AREA_INFO* area = Box64EC_GetCpuArea();
    x64emu_t* emu = area ? (x64emu_t*)area->EmulatorData[EC_DATA_EMU] : NULL;
    box64ec_thr_t* t = Box64EC_GetThreadState();
    uintptr_t native_sp = 0;
    uintptr_t target;

    if (!area || !emu || !area->ContextAmd64) {
        RtlRaiseStatus(STATUS_INVALID_PARAMETER);
        RtlExitUserProcess(STATUS_INVALID_PARAMETER);
    }

    if (t && InterlockedExchange(&t->guest_exception_pending, 0) == 1)
        Box64EC_ResetAbandonedEmuRun(t, emu);

    process_pending_cross_process_work();
    if (t) {
        t->pending_exit = 0;
        t->pending_native_sp = 0;
        t->pending_callback_depth = 0;
        t->pending_callback_seq = 0;
    }
    area->InSimulation = TRUE;
    /* Wine filled ContextAmd64; merge sticky EFlags from prior emu state. */
    merge_eflags_from_emu(emu, area->ContextAmd64);
    context_to_emu(area->ContextAmd64, emu);
    emu->quit = 0;
    run_emu(emu);
    /* Keep InSimulation until the assembly restore tail clears it. */
    emu_to_context(emu, area->ContextAmd64);
    target = Box64EC_TakePendingNativeExit(&native_sp);
    if (!target)
        target = Box64EC_ResolveNoPendingExit((uintptr_t)R_RIP);
    Box64EC_ApplyContextMxcsr(area->ContextAmd64);
    Box64EC_FinishBeginSimulation(area->ContextAmd64, target, native_sp);
}
