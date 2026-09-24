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
#include "custommem.h"
#include "debug.h"
#include "env.h"
#include "os.h"
#include "sysinfo.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "emu/x87emu_private.h"
#include "wine/compiler.h"

NTSYSAPI NTSTATUS NTAPI NtDuplicateObject(HANDLE source_process,
                                           HANDLE source_handle,
                                           HANDLE target_process,
                                           PHANDLE target_handle,
                                           ACCESS_MASK access,
                                           ULONG attributes,
                                           ULONG options);
NTSYSAPI NTSTATUS NTAPI NtSuspendThread(HANDLE thread, PULONG previous_count);
NTSYSAPI BOOLEAN WINAPI RtlIsCurrentThread(HANDLE thread);

static uint32_t x86emu_parity_tab[8] = {
    0x96696996, 0x69969669, 0x69969669, 0x96696996,
    0x69969669, 0x96696996, 0x96696996, 0x69969669,
};

NTSTATUS WINAPI ThreadInit(void)
{
    CHPE_V2_CPU_AREA_INFO* area = Box64EC_GetCpuArea();
    ARM64EC_NT_CONTEXT* ctx;
    x64emu_t* emu;
    box64ec_thr_t* thr;

    printf_log(LOG_DEBUG, "box64ec ThreadInit()\n");
    if (!area) {
        return STATUS_UNSUCCESSFUL;
    }

    emu = WinCalloc(1, sizeof(*emu));
    if (!emu) {
        return STATUS_NO_MEMORY;
    }
    thr = WinCalloc(1, sizeof(*thr));
    if (!thr) {
        WinFree(emu);
        return STATUS_NO_MEMORY;
    }
    area->EmulatorData[EC_DATA_THR] = thr;
    area->SuspendDoorbell = &thr->doorbell;

    emu->context = &Box64EC_Context;
    for (int i = 0; i < 16; ++i)
        emu->sbiidx[i] = &emu->regs[i];
    emu->sbiidx[4] = &emu->zero;
    emu->x64emu_parity_tab = x86emu_parity_tab;
    reset_fpu(emu);
    emu->win64_teb = (uint64_t)NtCurrentTeb();
    emu->segs_offs[_GS] = (uintptr_t)emu->win64_teb;

    if (!area->ContextAmd64)
        area->ContextAmd64 = (ARM64EC_NT_CONTEXT*)&area->EmulatorDataInline;
    ctx = area->ContextAmd64;

    if (!ctx->AMD64_SegCs) {
        ctx->ContextFlags = CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_FLOATING_POINT;
        ctx->AMD64_SegCs = 0x33;
        ctx->AMD64_SegDs = 0x2b;
        ctx->AMD64_SegEs = 0x2b;
        ctx->AMD64_SegFs = 0x53;
        ctx->AMD64_SegGs = 0x2b;
        ctx->AMD64_SegSs = 0x2b;
        ctx->AMD64_EFlags = 0x202;
        ctx->AMD64_MxCsr = 0x1f80;
        ctx->AMD64_MxCsr_copy = 0x1f80;
        ctx->AMD64_ControlWord = 0x27f;
    }

    printf_log(LOG_DEBUG, "box64ec ThreadInit: emu_stack=%p..%p ctx=%p\n",
               (void*)(uintptr_t)area->EmulatorStackLimit,
               (void*)(uintptr_t)area->EmulatorStackBase,
               (void*)ctx);

    area->EmulatorData[EC_DATA_EMU] = emu;

    return STATUS_SUCCESS;
}

static void free_thread_state(CHPE_V2_CPU_AREA_INFO* area)
{
    x64emu_t* emu;
    void* thr;

    if (!area)
        return;
    emu = (x64emu_t*)area->EmulatorData[EC_DATA_EMU];
    thr = area->EmulatorData[EC_DATA_THR];
    area->SuspendDoorbell = NULL;
    area->EmulatorData[EC_DATA_EMU] = NULL;
    if (emu) {
        FreeX64Emu(&emu);
    }
    area->EmulatorData[EC_DATA_THR] = NULL;
    if (thr)
        WinFree(thr);
}

void WINAPI ThreadTerm(HANDLE handle, LONG exit_code)
{
    typedef struct {
        NTSTATUS ExitStatus;
        PVOID TebBaseAddress;
        CLIENT_ID ClientId;
        ULONG_PTR AffinityMask;
        LONG Priority;
        LONG BasePriority;
    } thread_basic_information_t;

    CHPE_V2_CPU_AREA_INFO* area;
    x64emu_t* emu;
    thread_basic_information_t info;
    OBJECT_BASIC_INFORMATION access;
    HANDLE thread = NULL;
    NTSTATUS status;

    /* Wine invokes this callback before checking the original handle's access. */
    status = NtQueryObject(handle, ObjectBasicInformation, &access, sizeof(access), NULL);
    if (status || !(access.GrantedAccess & THREAD_TERMINATE))
        return;

    if (RtlIsCurrentThread(handle)) {
        area = Box64EC_GetCpuArea();
        emu = area ? (x64emu_t*)area->EmulatorData[EC_DATA_EMU] : NULL;
        printf_log(LOG_DEBUG, "box64ec ThreadTerm handle=%p status=%08x area=%p rip=%p rsp=%p\n",
                         handle, (unsigned)exit_code, area,
                         emu ? (void*)(uintptr_t)R_RIP : NULL,
                         emu ? (void*)(uintptr_t)R_RSP : NULL);
        free_thread_state(area);
        return;
    }

    status = NtDuplicateObject(NtCurrentProcess(), handle,
                               NtCurrentProcess(), &thread,
                               THREAD_QUERY_INFORMATION |
                               THREAD_SUSPEND_RESUME, 0, 0);
    if (status) {
        printf_log(LOG_DEBUG, "box64ec ThreadTerm handle=%p duplicate failed=%08x\n",
                         handle, (unsigned)status);
        return;
    }

    status = NtQueryInformationThread(thread, ThreadBasicInformation,
                                      &info, sizeof(info), NULL);
    if (status) {
        printf_log(LOG_DEBUG, "box64ec ThreadTerm handle=%p query failed=%08x\n",
                         handle, (unsigned)status);
        NtClose(thread);
        return;
    }
    if (!info.TebBaseAddress) {
        printf_log(LOG_DEBUG, "box64ec ThreadTerm handle=%p has no TEB\n", handle);
        NtClose(thread);
        return;
    }

    status = NtSuspendThread(thread, NULL);
    if (status) {
        printf_log(LOG_DEBUG, "box64ec ThreadTerm handle=%p suspend failed=%08x\n",
                         handle, (unsigned)status);
        NtClose(thread);
        return;
    }

    area = *(CHPE_V2_CPU_AREA_INFO**)(
        (char*)info.TebBaseAddress + TEB_CHPE_V2_CPU_AREA_OFFSET);
    emu = area ? (x64emu_t*)area->EmulatorData[EC_DATA_EMU] : NULL;
    printf_log(LOG_DEBUG, "box64ec ThreadTerm handle=%p status=%08x area=%p rip=%p rsp=%p\n",
                     handle, (unsigned)exit_code, area,
                     emu ? (void*)(uintptr_t)R_RIP : NULL,
                     emu ? (void*)(uintptr_t)R_RSP : NULL);

    if (!area) {
        NtClose(thread);
        return;
    }
    free_thread_state(area);
    NtClose(thread);
}
