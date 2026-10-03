// SPDX-License-Identifier: MIT
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <ntstatus.h>
#include <winternl.h>
#include <winnt.h>

#include "box64ec.h"
#include "box64ec_private.h"
#include "box64ec_syscalls.h"
#include "box64cpu.h"
#include "debug.h"
#include "os.h"
#include "custommem.h"
#include "env.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "box64context.h"
#include "wine/compiler.h"

NTSYSAPI void NTAPI RtlExitUserProcess(NTSTATUS status);
NTSYSAPI NTSTATUS NTAPI NtTerminateProcess(HANDLE process, NTSTATUS status);
#ifdef DYNAREC
extern void arm64_epilog(void);
#endif

static void (WINAPI *pProcessPendingCrossProcessEmulatorWork)(void);
static uintptr_t rtl_unwind_ex, rtl_unwind_target;

static const uint8_t fast_ffs_prefix[9] = {
    0x48, 0x8b, 0xc4,             /* mov rax,rsp */
    0x48, 0x89, 0x58, 0x20,       /* mov [rax+20h],rbx */
    0x55, 0x5d                    /* push rbp; pop rbp */
};


void process_pending_cross_process_work(void)
{
    if (pProcessPendingCrossProcessEmulatorWork)
        pProcessPendingCrossProcessEmulatorWork();
}

box64ec_thr_t* Box64EC_GetThreadState(void)
{
    CHPE_V2_CPU_AREA_INFO* area = Box64EC_GetCpuArea();
    if (!area)
        return NULL;
    return (box64ec_thr_t*)area->EmulatorData[EC_DATA_THR];
}

void finish_enter(box64ec_thr_t* t)
{
    if (!t || t->enter_depth <= 0)
        return;
    --t->enter_depth;
}

static void discard_callback_activations(box64ec_thr_t* t, unsigned depth)
{
    if (!t || depth >= t->callback_depth)
        return;
    memset(&t->callbacks[depth], 0,
           (t->callback_depth - depth) * sizeof(t->callbacks[0]));
    t->callback_depth = depth;
}

static void note_callback_activation(box64ec_thr_t* t, uintptr_t lr,
                                     uintptr_t guest_sp,
                                     uintptr_t native_resume_sp,
                                     unsigned route)
{
    box64ec_callback_activation_t* active;

    if (!t || (route != 1 && route != 2) ||
        !Box64EC_IsEmulatorStackAddress(native_resume_sp) ||
        !Box64EC_IsEmulatorStackAddress(native_resume_sp - 8) ||
        (native_resume_sp & 15))
        return;

    while (t->callback_depth &&
           t->callbacks[t->callback_depth - 1].native_resume_sp <=
               native_resume_sp)
        discard_callback_activations(t, t->callback_depth - 1);
    if (t->callback_depth >= BOX64EC_CALLBACK_SLOTS)
        return;

    active = &t->callbacks[t->callback_depth++];
    active->lr = lr;
    active->guest_sp = guest_sp;
    active->native_resume_sp = native_resume_sp;
    active->seq = ++t->callback_seq;
    active->route = route;
}

void* Box64EC_X64ReturnInstr;



static NTSTATUS alloc_x64_return_instr(void)
{
    PVOID base = NULL;
    SIZE_T size = 0x1000;
    NTSTATUS status = NtAllocateVirtualMemory(
        NtCurrentProcess(), &base, 0, &size,
        MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (status)
        return status;
    *(uint8_t*)base = 0xc3;
    Box64EC_X64ReturnInstr = base;
    return STATUS_SUCCESS;
}

static uintptr_t fast_ffs_target(uintptr_t stub)
{
    uintptr_t jmp = stub + sizeof(fast_ffs_prefix);
    int32_t rel;

    if (!stub || memcmp((const void*)stub, fast_ffs_prefix,
                        sizeof(fast_ffs_prefix)) ||
        *(const uint8_t*)jmp != 0xe9)
        return 0;
    memcpy(&rel, (const void*)(jmp + 1), sizeof(rel));
    return jmp + 5 + (intptr_t)rel;
}

NTSTATUS Box64EC_TransitionProcessInit(HMODULE ntdll)
{
    NTSTATUS status;

    if (ntdll)
        pProcessPendingCrossProcessEmulatorWork =
            (void*)RtlFindExportedRoutineByName(
                ntdll, "ProcessPendingCrossProcessEmulatorWork");
    rtl_unwind_ex = (uintptr_t)RtlFindExportedRoutineByName(ntdll, "RtlUnwindEx");
    rtl_unwind_target = fast_ffs_target(rtl_unwind_ex);
    status = alloc_x64_return_instr();
    if (status)
        return status;
    printf_log(LOG_DEBUG, "box64ec X64ReturnInstr=%p\n", Box64EC_X64ReturnInstr);
    return STATUS_SUCCESS;
}

void Box64EC_NoteEntryFrame(uint64_t sp, uint64_t pc)
{
    box64ec_thr_t* t = Box64EC_GetThreadState();
    if (!t)
        return;
    t->entry.sp = sp;
    t->entry.pc = pc;
}

void Box64EC_NoteEnterBoundary(uint64_t route,
                               uint64_t guest_sp, uint64_t lr,
                               uint64_t emulator_sp, uint64_t pushed_slot)
{
    CHPE_V2_CPU_AREA_INFO* area = Box64EC_GetCpuArea();
    x64emu_t* emu = area ? (x64emu_t*)area->EmulatorData[EC_DATA_EMU] : NULL;
    box64ec_thr_t* t = Box64EC_GetThreadState();
    uintptr_t native_resume_sp = 0;

    if (!emu || !t)
        return;

    if ((route == 1 || route == 2) && pushed_slot == lr &&
        Box64EC_IsEmulatorStackAddress(emulator_sp))
        native_resume_sp = (uintptr_t)emulator_sp + 8;
    if (native_resume_sp)
        note_callback_activation(t, (uintptr_t)lr, (uintptr_t)guest_sp,
                                 native_resume_sp, (unsigned)route);
}

static box64ec_callback_activation_t* find_callback_activation(
    box64ec_thr_t* t, uintptr_t target, uintptr_t rsp, int exit_thunk,
    unsigned* depth)
{
    unsigned count;

    if (!t || !t->callback_depth)
        return NULL;
    count = t->callback_depth;

    for (unsigned n = count; n; --n) {
        box64ec_callback_activation_t* active = &t->callbacks[n - 1];
        if (active->route && active->lr == target &&
            rsp == active->guest_sp + 8 &&
            *(volatile uintptr_t*)(active->native_resume_sp - 8) == active->lr) {
            if (depth)
                *depth = n;
            return active;
        }
    }
    if (!exit_thunk)
        return NULL;
    for (unsigned n = count; n; --n) {
        box64ec_callback_activation_t* active = &t->callbacks[n - 1];
        if (active->route && rsp == active->guest_sp + 8 &&
            *(volatile uintptr_t*)(active->native_resume_sp - 8) == active->lr) {
            if (depth)
                *depth = n;
            return active;
        }
    }
    return NULL;
}

void Box64EC_ClearInSimulationIfOutermost(void)
{
    CHPE_V2_CPU_AREA_INFO* area = Box64EC_GetCpuArea();
    box64ec_thr_t* t = Box64EC_GetThreadState();
    if (!area)
        return;
    /* Inner EnterX64 has already decremented depth on return. */
    if (t && t->enter_depth > 0)
        return;
    area->InSimulation = FALSE;
}

int Box64EC_IsEmulatorStackAddress(uint64_t addr)
{
    CHPE_V2_CPU_AREA_INFO* area = Box64EC_GetCpuArea();
    if (!area || !area->EmulatorStackBase || !area->EmulatorStackLimit)
        return 0;
    return addr <= area->EmulatorStackBase && addr >= area->EmulatorStackLimit;
}

int Box64EC_PollSuspend(void)
{
    CHPE_V2_CPU_AREA_INFO* area = Box64EC_GetCpuArea();
    if (!area || !area->SuspendDoorbell)
        return 0;
    return *area->SuspendDoorbell != 0;
}

void Box64EC_HandleSuspend(x64emu_t* emu)
{
    CHPE_V2_CPU_AREA_INFO* area = Box64EC_GetCpuArea();
    ARM64_NT_CONTEXT context;
    uint64_t fpcr, fpsr;
    NTSTATUS status;

    if (!area || !emu)
        return;
    emu_to_arm64_ec_packed(emu, &context);
    /* LDMXCSR updates emulator state without necessarily updating host state. */
    Box64EC_ApplyLiveMxcsr(emu);
    __asm__ __volatile__("mrs %0, fpcr" : "=r"(fpcr));
    __asm__ __volatile__("mrs %0, fpsr" : "=r"(fpsr));
    context.Fpcr = fpcr;
    context.Fpsr = fpsr;
    area->InSimulation = FALSE;
    if (area->SuspendDoorbell)
        *area->SuspendDoorbell = 0;
    status = Box64EC_ContinueNative(&context, FALSE);
    RtlRaiseStatus(status ? status : STATUS_UNSUCCESSFUL);
}

void Box64EC_RequestExitToNative(x64emu_t* emu, uintptr_t addr)
{
    box64ec_thr_t* t = Box64EC_GetThreadState();
    box64ec_callback_activation_t* active;
    uint32_t target_m4 = 0;
    unsigned callback_depth = 0;

    if (t) {
        t->pending_native_sp = 0;
        t->pending_callback_depth = 0;
        t->pending_callback_seq = 0;
    }

    /* An ARM64EC exit thunk returns to the instruction after `blr x16`.
     * Native Wine continuations require an aligned SP. */
    if (addr >= 4) memcpy(&target_m4, (const void*)(addr - 4), sizeof(target_m4));

    /* Pair a native->x64 ExitToX64 frame with the x64->native continuation.
     * ExitToX64 pushes LR on the guest stack, so a correct x64 RET restores
     * exactly guest_sp + 8.  Keep this independent of the exit-thunk shape:
     * it remains useful even when Wine reaches the continuation by another
     * thunk shape. */
    if (t && emu) {
        active = find_callback_activation(t, addr, (uintptr_t)R_RSP,
                                          target_m4 == 0xd63f0200u,
                                          &callback_depth);
        if (active) {
            t->pending_native_sp = active->native_resume_sp;
            t->pending_callback_depth = callback_depth;
            t->pending_callback_seq = active->seq;
        }
    }

    /* Do NOT clear InSimulation here. enter_jit refuses nesting only while
     * InSimulation=1; clearing early lets a re-entered EnterX64 wipe
     * pending_exit (Caps: without-pending at x64 RIP → NtTerminate).
     * Asm clears InSimulation after TakePendingNativeExit. */

    if (t)
        t->pending_exit = addr;
    if (emu) {
        R_RIP = addr;
        emu->quit = 1;
    }
}

uintptr_t Box64EC_TakePendingNativeExit(uintptr_t* native_sp)
{
    box64ec_thr_t* t = Box64EC_GetThreadState();
    CHPE_V2_CPU_AREA_INFO* area;
    x64emu_t* emu;
    uintptr_t v, sp;
    unsigned callback_depth;
    uint64_t callback_seq;

    if (!t) {
        if (native_sp)
            *native_sp = 0;
        return 0;
    }
    v = t->pending_exit;
    sp = t->pending_native_sp;
    callback_depth = t->pending_callback_depth;
    callback_seq = t->pending_callback_seq;
    t->pending_exit = 0;
    t->pending_native_sp = 0;
    t->pending_callback_depth = 0;
    t->pending_callback_seq = 0;
    if (v && sp && callback_depth &&
        callback_depth <= t->callback_depth &&
        t->callbacks[callback_depth - 1].seq == callback_seq &&
        t->callbacks[callback_depth - 1].native_resume_sp == sp) {
        discard_callback_activations(t, callback_depth - 1);
    } else if (sp) {
        sp = 0;
    }
    if (native_sp)
        *native_sp = sp;
    if ((v && (v == rtl_unwind_ex || v == rtl_unwind_target))) {
        area = Box64EC_GetCpuArea();
        emu = area ? (x64emu_t*)area->EmulatorData[EC_DATA_EMU] : NULL;
        printf_log(LOG_DEBUG, "box64ec: reset abandoned emulator state before "
                         "RtlUnwindEx target=%p\n", (void*)v);
        Box64EC_ResetAbandonedEmuRun(t, emu);
    }
    return v;
}
