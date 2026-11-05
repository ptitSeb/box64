// SPDX-License-Identifier: MIT
#include <string.h>
#include <windows.h>
#include <ntstatus.h>
#include <winternl.h>

#include "box64ec_exception.h"
#include "box64ec_private.h"
#include "box64ec_syscalls.h"
#include "emu/x64emu_private.h"
#include "wine/compiler.h"
#ifdef DYNAREC
#include "custommem.h"
#include "dynablock.h"
#include "dynarec/arm64/dynarec_arm64_arch.h"
#include "box64context.h"
#include "os.h"

extern int fillblock_active;
extern void* current_helper;
void adjustregs(x64emu_t* emu, void* pc);
#endif

/* Dispatch a host fault in Run with the guest state, on the guest stack. */
static void rethrow_interpreter_fault(EXCEPTION_RECORD* record,
                                      ARM64_NT_CONTEXT* arm_context,
                                      x64emu_t* emu)
{
    CHPE_V2_CPU_AREA_INFO* area = Box64EC_GetCpuArea();
    box64ec_thr_t* state = Box64EC_GetThreadState();
    KiUserExceptionDispatcherStackLayout* args;
    uint64_t guest_sp = R_RSP & ~(uint64_t)63;

    if (!state || !Box64EC_KiUserExceptionDispatcher || !guest_sp ||
        Box64EC_IsEmulatorStackAddress(guest_sp))
        Box64EC_FatalEnterJit(0xec08);
    args = ((KiUserExceptionDispatcherStackLayout*)(uintptr_t)guest_sp) - 1;
    memset(args, 0, sizeof(*args));
    emu_to_arm64_ec_packed(emu, &args->Context);
    args->Context.Fpcr = arm_context->Fpcr;
    args->Context.Fpsr = arm_context->Fpsr;
    args->Rec = *record;
    args->Rec.ExceptionAddress = (void*)(uintptr_t)R_RIP;
    args->Sp = R_RSP;
    args->Pc = R_RIP;
    arm_context->Sp = (ULONG64)(uintptr_t)args;
    arm_context->Fp = arm_context->Sp;
    arm_context->Pc = Box64EC_KiUserExceptionDispatcher;
    arm_context->X18 = (ULONG64)(uintptr_t)NtCurrentTeb();
    state->guest_exception_code = record->ExceptionCode;
    state->guest_exception_address = (uintptr_t)args->Rec.ExceptionAddress;
    state->guest_exception_rip = R_RIP;
    state->guest_exception_rsp = R_RSP;
    InterlockedExchange(&state->guest_exception_pending, 1);
    area->InSimulation = FALSE;
    Box64EC_ContinueNative(arm_context, FALSE);
    Box64EC_FatalEnterJit(0xec08);
}

void WINAPI ResetToConsistentState(EXCEPTION_RECORD* record, CONTEXT* context,
                                    ARM64_NT_CONTEXT* arm_context)
{
    CHPE_V2_CPU_AREA_INFO* area = Box64EC_GetCpuArea();
    x64emu_t* emu = area ? area->EmulatorData[EC_DATA_EMU] : NULL;
    box64ec_thr_t* state = Box64EC_GetThreadState();

    if (!record || !arm_context || !emu || !state || !context)
        return;
    if (InterlockedCompareExchange(&state->guest_exception_pending, 1, 1) == 1 &&
        record->ExceptionCode == state->guest_exception_code &&
        (uintptr_t)record->ExceptionAddress == state->guest_exception_address &&
        arm_context->Pc == state->guest_exception_rip &&
        arm_context->Sp == state->guest_exception_rsp &&
        InterlockedCompareExchange(&state->guest_exception_pending, 0, 1) == 1) {
        Box64EC_ResetAbandonedEmuRun(state, emu);
        merge_unmapped_context_from_emu(emu, (ARM64EC_NT_CONTEXT*)context);
        area->InSimulation = FALSE;
        return;
    }
    if (record->ExceptionCode == STATUS_ILLEGAL_INSTRUCTION &&
        arm_context->Pc == (uintptr_t)&ExitFunctionSuspendPoint) {
        arm_context->Pc = (uintptr_t)&ExitFunctionSuspendResumePoint;
        area->InSimulation = FALSE;
        if (area->SuspendDoorbell)
            *area->SuspendDoorbell = 0;
        Box64EC_ContinueNative(arm_context, FALSE);
        return;
    }
#ifdef DYNAREC
    /* FillBlock64 can read past the guest code it was given: as the Linux
     * signal handler does, go back into it to cut the block short. */
    if (fillblock_active && current_helper &&
        (uintptr_t)my_context->mutex_dyndump.OwningThread == (uintptr_t)GetTID())
        cancelFillBlock();
    /* In JIT code the guest registers are in the JIT map, and the deferred flags and
     * cached SIMD registers are described by the block. */
    dynablock_t* db = area->InSimulation ? FindDynablockFromNativeAddress((void*)(uintptr_t)arm_context->Pc) : NULL;
    if (db) {
        uintptr_t x64pc = getX64Address(db, arm_context->Pc);
        jit_arm_context_to_emu(arm_context, emu);
        R_RIP = x64pc ? x64pc : arm_context->X27;
        if (x64pc) {
            adjustregs(emu, (void*)(uintptr_t)arm_context->Pc);
            adjust_arch(db, emu, arm_context, x64pc);
        }
        if (record->ExceptionCode == STATUS_ILLEGAL_INSTRUCTION) {
            if (*(uint32_t*)(uintptr_t)arm_context->Pc == BOX64EC_JIT_SUSPEND_OPCODE) {
                Box64EC_ResetAbandonedEmuRun(state, emu);
                Box64EC_HandleSuspend(emu);
                return;
            }
            if (Box64EC_IsPrivilegedInstruction((void*)(uintptr_t)R_RIP)) {
                record->ExceptionCode = STATUS_PRIVILEGED_INSTRUCTION;
                record->NumberParameters = 0;
            }
        }
    }
#endif
    if (area->InSimulation &&
        Box64EC_IsEmulatorStackAddress(arm_context->Sp)) {
        emu_to_context(emu, area->ContextAmd64);
        emu_to_context(emu, (ARM64EC_NT_CONTEXT*)context);
        rethrow_interpreter_fault(record, arm_context, emu);
    }
    merge_unmapped_context_from_emu(emu, (ARM64EC_NT_CONTEXT*)context);
}
