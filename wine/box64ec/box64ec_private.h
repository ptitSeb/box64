// SPDX-License-Identifier: MIT
#ifndef BOX64EC_PRIVATE_H
#define BOX64EC_PRIVATE_H

#include "box64ec.h"
#include "box64context.h"

typedef struct {
    uintptr_t lr;
    uintptr_t guest_sp;
    uintptr_t native_resume_sp;
    uint64_t seq;
    unsigned route;
} box64ec_callback_activation_t;

#define BOX64EC_CALLBACK_SLOTS 128

typedef struct {
    uintptr_t pending_exit;
    uintptr_t pending_native_sp;
    Box64EC_EntryFrame entry;
    int enter_depth;
    unsigned internal_map_depth;
    unsigned callback_depth;
    unsigned pending_callback_depth;
    uint64_t callback_seq;
    uint64_t pending_callback_seq;
    volatile LONG guest_exception_pending;
    ULONG guest_exception_code;
    uintptr_t guest_exception_address;
    uintptr_t guest_exception_rip;
    uintptr_t guest_exception_rsp;
    box64ec_callback_activation_t callbacks[BOX64EC_CALLBACK_SLOTS];
} box64ec_thr_t;

extern box64context_t Box64EC_Context;

void context_to_emu(ARM64EC_NT_CONTEXT* context, x64emu_t* emu);
uint32_t Box64EC_GetEflags(x64emu_t* emu);
void emu_to_context(x64emu_t* emu, ARM64EC_NT_CONTEXT* context);
void jit_arm_context_to_emu(ARM64_NT_CONTEXT* context, x64emu_t* emu);
void emu_to_jit_arm_context(x64emu_t* emu, ARM64_NT_CONTEXT* context);
void emu_to_arm64_ec_packed(x64emu_t* emu, ARM64_NT_CONTEXT* context);
void merge_eflags_from_emu(x64emu_t* emu, ARM64EC_NT_CONTEXT* context);
void merge_unmapped_context_from_emu(x64emu_t* emu, ARM64EC_NT_CONTEXT* context);
box64ec_thr_t* Box64EC_GetThreadState(void);
void process_pending_cross_process_work(void);
void finish_enter(box64ec_thr_t* state);
extern int Box64EC_FastCallback;
void Box64EC_FastCallbackLeave(void);
void Box64EC_ResetAbandonedEmuRun(box64ec_thr_t* state, x64emu_t* emu);
void Box64EC_InternalMapEnter(void);
void Box64EC_InternalMapLeave(void);
NTSTATUS Box64EC_InitCallretStack(x64emu_t* emu);
void Box64EC_FreeCallretStack(x64emu_t* emu);
void Box64EC_ResetCallretStack(x64emu_t* emu);
int Box64EC_HandleCallretStackFault(x64emu_t* emu, uintptr_t address,
                                    ARM64_NT_CONTEXT* arm_ctx);
NTSTATUS Box64EC_TransitionProcessInit(HMODULE ntdll);
uintptr_t fast_ffs_target(uintptr_t stub);
extern uintptr_t rtl_unwind_ex, rtl_unwind_target;
extern void ExitToX64(void);
extern void BeginSimulation(void);
extern char ExitFunctionSuspendPoint;
extern char ExitFunctionSuspendResumePoint;

#endif
