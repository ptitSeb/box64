// SPDX-License-Identifier: MIT
#include <stdint.h>
#include <string.h>
#include <windows.h>
#include <ntstatus.h>
#include <winternl.h>

#include "box64ec.h"
#include "box64ec_exception.h"
#include "box64ec_private.h"
#include "box64ec_syscalls.h"
#include "debug.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "wine/compiler.h"

NTSYSAPI void NTAPI RtlExitUserProcess(NTSTATUS status);
NTSYSAPI NTSTATUS NTAPI NtRaiseException(EXCEPTION_RECORD* rec,
                                         CONTEXT* context,
                                         BOOL first_chance);
NTSYSAPI NTSTATUS NTAPI NtTerminateProcess(HANDLE process, NTSTATUS status);

void Box64EC_RaiseGuestException(EXCEPTION_RECORD* rec)
{
    CHPE_V2_CPU_AREA_INFO* area = Box64EC_GetCpuArea();
    x64emu_t* emu = area ? (x64emu_t*)area->EmulatorData[EC_DATA_EMU] : NULL;
    box64ec_thr_t* t = Box64EC_GetThreadState();
    ULONG context_flags;
    BOOL first_chance;
    int was_in_simulation;
    NTSTATUS status;

    if (!rec)
        return;

    printf_log(LOG_DEBUG, "box64ec RaiseGuestException code=%08x flags=%x addr=%p n=%u\n",
               (unsigned)rec->ExceptionCode, (unsigned)rec->ExceptionFlags,
               rec->ExceptionAddress, (unsigned)rec->NumberParameters);

    first_chance = rec->ExceptionCode != (NTSTATUS)STATUS_STACK_BUFFER_OVERRUN &&
                   rec->ExceptionCode != (NTSTATUS)0xC0000602; /* STATUS_FAIL_FAST_EXCEPTION */
    if (!first_chance)
        rec->ExceptionFlags |= EXCEPTION_NONCONTINUABLE;

    if (!area || !emu || !area->ContextAmd64 || !t || !R_RSP ||
        Box64EC_IsEmulatorStackAddress(R_RSP)) {
        printf_log(LOG_DEBUG, "box64ec RaiseGuestException has no guest context\n");
        NtTerminateProcess((HANDLE)(intptr_t)-1, STATUS_INVALID_PARAMETER);
        RtlExitUserProcess(STATUS_INVALID_PARAMETER);
        return;
    }

    context_flags = area->ContextAmd64->ContextFlags;
    area->ContextAmd64->ContextFlags = CONTEXT_FULL;
    if ((context_flags & CONTEXT_XSTATE) == CONTEXT_XSTATE)
        area->ContextAmd64->ContextFlags |= CONTEXT_XSTATE;
    emu_to_context(emu, area->ContextAmd64);
    t->guest_exception_code = rec->ExceptionCode;
    t->guest_exception_address = (uintptr_t)rec->ExceptionAddress;
    t->guest_exception_rip = (uintptr_t)R_RIP;
    t->guest_exception_rsp = (uintptr_t)R_RSP;
    InterlockedExchange(&t->guest_exception_pending, 1);

    was_in_simulation = area->InSimulation;
    area->InSimulation = FALSE;
    status = NtRaiseException(rec, (CONTEXT*)area->ContextAmd64, first_chance);
    InterlockedCompareExchange(&t->guest_exception_pending, 0, 1);
    area->InSimulation = was_in_simulation;

    printf_log(LOG_DEBUG, "box64ec NtRaiseException returned status=%08x code=%08x\n",
                     (unsigned)status, (unsigned)rec->ExceptionCode);
    NtTerminateProcess((HANDLE)(intptr_t)-1,
                       status ? status : rec->ExceptionCode);
    RtlExitUserProcess(status ? status : rec->ExceptionCode);
}

static int raise_emulation_syscall_kiuser(x64emu_t* emu, int advance_rip)
{
    /* Must outlive JumpSetStack into KiUser (SP becomes &args). */
    KiUserExceptionDispatcherStackLayout args;
    uint64_t fpcr = 0, fpsr = 0;
    CHPE_V2_CPU_AREA_INFO* area = Box64EC_GetCpuArea();
    box64ec_thr_t* thr;

    if (!emu || !Box64EC_KiUserExceptionDispatcher)
        return 0;
    if (!R_RSP || Box64EC_IsEmulatorStackAddress(R_RSP))
        return 0;

    if (advance_rip)
        R_RIP += 2;

    memset(&args, 0, sizeof(args));
    emu_to_arm64_ec_packed(emu, &args.Context);
    __asm__ __volatile__("mrs %0, fpcr" : "=r"(fpcr));
    __asm__ __volatile__("mrs %0, fpsr" : "=r"(fpsr));
    args.Context.Fpcr = fpcr;
    args.Context.Fpsr = fpsr;
    args.Rec.ExceptionCode = (NTSTATUS)0x40000039; /* STATUS_EMULATION_SYSCALL */

    if (area && area->ContextAmd64)
        emu_to_context(emu, area->ContextAmd64);

    thr = Box64EC_GetThreadState();
    /* Abandoned EnterX64 frames must not keep nesting state. */
    if (thr) {
        thr->enter_depth = 0;
        thr->pending_exit = 0;
        thr->pending_native_sp = 0;
        thr->pending_callback_depth = 0;
        thr->pending_callback_seq = 0;
    }
    if (area)
        area->InSimulation = FALSE;
    emu->quit = 1;

    Box64EC_JumpSetStack(Box64EC_KiUserExceptionDispatcher,
                         (uintptr_t)(void*)&args);
    /* noreturn */
    return 0;
}

void Box64EC_EmulateSyscall(void* opaque)
{
    x64emu_t* emu = opaque;
    EXCEPTION_RECORD rec;

    raise_emulation_syscall_kiuser(emu, 0);
    memset(&rec, 0, sizeof(rec));
    rec.ExceptionCode = (NTSTATUS)0x40000039;
    rec.ExceptionAddress = emu && R_RIP >= 2
        ? (void*)(uintptr_t)(R_RIP - 2) : NULL;
    Box64EC_RaiseGuestException(&rec);
}

void EmitInterruption(x64emu_t* emu, int num, void* addr)
{
    CHPE_V2_CPU_AREA_INFO* area = Box64EC_GetCpuArea();
    EXCEPTION_RECORD rec;
    void* rip = addr ? addr : (void*)(uintptr_t)R_RIP;

    printf_log(LOG_DEBUG, "box64ec EmitInterruption int=0x%x at %p RIP=%p RCX=%p\n",
               num, addr, (void*)R_RIP, (void*)(uintptr_t)R_RCX);

    if (num == 0x2d)
        R_RIP = (uintptr_t)rip + 3;
    if (area && area->ContextAmd64)
        emu_to_context(emu, area->ContextAmd64);
    if (area)
        area->InSimulation = FALSE;

    memset(&rec, 0, sizeof(rec));
    rec.ExceptionAddress = rip;

    switch (num) {
    case 0x2e: /* NT syscall */
    case 0x80: /* Linux-ish syscall - Wine remux via STATUS_EMULATION_SYSCALL */
        raise_emulation_syscall_kiuser(emu, 1);
        rec.ExceptionCode = (NTSTATUS)0x40000039;
        break;
    case 0x29: /* __fastfail(RCX) - Wine uses STATUS_STACK_BUFFER_OVERRUN */
        rec.ExceptionCode = STATUS_STACK_BUFFER_OVERRUN;
        rec.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
        rec.NumberParameters = 1;
        rec.ExceptionInformation[0] = (ULONG_PTR)R_RCX;
        break;
    case 0x2c: /* assertion failure */
        rec.ExceptionCode = STATUS_ASSERTION_FAILURE;
        break;
    case 0x2d: /* debug service - skip harmless print/symbol ops like Wine */
        if (R_RAX == 1 || R_RAX == 3 || R_RAX == 4 || R_RAX == 5) {
            printf_log(LOG_DEBUG, "box64ec: ignoring INT 0x2d service RAX=%llu\n",
                       (unsigned long long)R_RAX);
            if (area)
                area->InSimulation = TRUE;
            return;
        }
        rec.ExceptionAddress = (void*)(uintptr_t)R_RIP;
        /* Wine delivers the handler context one byte before the debug-service address. */
        --R_RIP;
        rec.ExceptionCode = STATUS_BREAKPOINT;
        rec.NumberParameters = 1;
        rec.ExceptionInformation[0] = (ULONG_PTR)R_RAX;
        break;
    default:
        /* Match Wine wow64 default: AV with 2 params, not a naked 0-param raise. */
        printf_log(LOG_DEBUG, "box64ec: unhandled INT 0x%x at RIP=%p\n", num, rip);
        rec.ExceptionCode = STATUS_ACCESS_VIOLATION;
        rec.NumberParameters = 2;
        rec.ExceptionInformation[0] = 0;
        rec.ExceptionInformation[1] = (ULONG_PTR)-1;
        break;
    }

    Box64EC_RaiseGuestException(&rec);
}
