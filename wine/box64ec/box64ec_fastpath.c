// SPDX-License-Identifier: MIT
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <windows.h>
#include <winternl.h>

#include "box64ec.h"
#include "box64ec_layout.h"
#include "box64ec_private.h"
#include "box64cpu.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "wine/compiler.h"

_Static_assert(offsetof(box64ec_thr_t, guest_exception_pending) == BOX64EC_THR_EXCEPTION_PENDING_OFFSET,
               "guest_exception_pending");
_Static_assert(offsetof(box64ec_thr_t, pending_exit) == BOX64EC_THR_PENDING_EXIT_OFFSET, "pending_exit");
_Static_assert(offsetof(box64ec_thr_t, pending_native_sp) == BOX64EC_THR_PENDING_NATIVE_SP_OFFSET,
               "pending_native_sp");
_Static_assert(offsetof(box64ec_thr_t, entry.sp) == BOX64EC_THR_ENTRY_SP_OFFSET &&
               offsetof(box64ec_thr_t, entry.pc) == BOX64EC_THR_ENTRY_SP_OFFSET + 8, "entry");
_Static_assert(offsetof(box64ec_thr_t, enter_depth) == BOX64EC_THR_ENTER_DEPTH_OFFSET, "enter_depth");
_Static_assert(offsetof(box64ec_thr_t, pending_callback_depth) == BOX64EC_THR_PENDING_CALLBACK_DEPTH_OFFSET,
               "pending_callback_depth");
_Static_assert(offsetof(box64ec_thr_t, pending_callback_seq) == BOX64EC_THR_PENDING_CALLBACK_SEQ_OFFSET,
               "pending_callback_seq");
_Static_assert(offsetof(x64emu_t, xmm) == BOX64EC_EMU_XMM_OFFSET, "xmm");
_Static_assert(offsetof(x64emu_t, mxcsr) == BOX64EC_EMU_MXCSR_OFFSET, "mxcsr");
_Static_assert(offsetof(x64emu_t, live_mxcsr) == BOX64EC_EMU_LIVE_MXCSR_OFFSET, "live_mxcsr");
_Static_assert(offsetof(x64emu_t, live_fpcr) == BOX64EC_EMU_LIVE_FPCR_OFFSET, "live_fpcr");
_Static_assert(offsetof(x64emu_t, live_fpsr) == BOX64EC_EMU_LIVE_FPSR_OFFSET, "live_fpsr");

/* Functions that never return or switch stacks, as exports and x64 stub targets */
enum { SLOW_RAISE_EXCEPTION, SLOW_RTL_RAISE_EXCEPTION, SLOW_SWITCH_TO_FIBER, SLOW_COUNT };
static uintptr_t slow_exports[SLOW_COUNT];
static uintptr_t slow_targets[SLOW_COUNT];
static int slow_resolved;

/* kernelbase is not loaded yet when Box64EC starts: look again until it is */
static void resolve_slow_targets(void)
{
    UNICODE_STRING name;
    HMODULE ntdll = NULL, kernelbase = NULL;

    RtlInitUnicodeString(&name, L"kernelbase.dll");
    if (LdrGetDllHandle(NULL, 0, &name, &kernelbase) || !kernelbase)
        return;
    RtlInitUnicodeString(&name, L"ntdll.dll");
    if (LdrGetDllHandle(NULL, 0, &name, &ntdll) || !ntdll)
        return;
    slow_exports[SLOW_RAISE_EXCEPTION] = (uintptr_t)RtlFindExportedRoutineByName(kernelbase, "RaiseException");
    slow_exports[SLOW_RTL_RAISE_EXCEPTION] = (uintptr_t)RtlFindExportedRoutineByName(ntdll, "RtlRaiseException");
    slow_exports[SLOW_SWITCH_TO_FIBER] = (uintptr_t)RtlFindExportedRoutineByName(kernelbase, "SwitchToFiber");
    for (int i = 0; i < SLOW_COUNT; ++i)
        slow_targets[i] = fast_ffs_target(slow_exports[i]);
    __atomic_store_n(&slow_resolved, 1, __ATOMIC_RELEASE);   // after the tables, for other threads
}

static int is_slow_target(uintptr_t addr)
{
    if (!__atomic_load_n(&slow_resolved, __ATOMIC_ACQUIRE))
        resolve_slow_targets();
    if (addr == rtl_unwind_ex || addr == rtl_unwind_target)
        return 1;
    for (int i = 0; i < SLOW_COUNT; ++i)
        if (addr == slow_exports[i] || addr == slow_targets[i])
            return 1;
    return 0;
}

/* Decided once per target and cached in the jump table, the call state is checked in asm */
int Box64EC_TryFastForward(x64emu_t* emu, uintptr_t addr)
{
    CHPE_V2_CPU_AREA_INFO* area;
    uintptr_t ret;
    uint32_t before_target;

    if (!emu || !addr || (addr & 3))
        return BOX64EC_FAST_FORWARD_REJECT;
    /* A BLR right before the target is an exit thunk continuation, not a function entry */
    memcpy(&before_target, (const void*)(addr - 4), sizeof(before_target));
    if (before_target == 0xd63f0200u || is_slow_target(addr))
        return BOX64EC_FAST_FORWARD_NATIVE_EXIT;
    if (!__atomic_load_n(&slow_resolved, __ATOMIC_ACQUIRE))
        return BOX64EC_FAST_FORWARD_REJECT;     // could still be one of them: do not cache it

    if (Box64EC_PollSuspend() || !R_RSP || Box64EC_IsEmulatorStackAddress(R_RSP))
        return BOX64EC_FAST_FORWARD_REJECT;
    ret = *(volatile uintptr_t*)(uintptr_t)R_RSP;
    if (ret < 0x10000ull || (ret >> BOX64EC_VA_BITS) || Box64EC_IsEcCode(ret))
        return BOX64EC_FAST_FORWARD_REJECT;
    area = Box64EC_GetCpuArea();
    if (!area || !area->ContextAmd64 || area->EmulatorData[EC_DATA_EMU] != emu)
        return BOX64EC_FAST_FORWARD_REJECT;
    return BOX64EC_FAST_FORWARD_ACCEPT;
}
