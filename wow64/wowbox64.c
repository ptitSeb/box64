/*
 * Copyright 2022-2025 André Zwing
 * Copyright 2023 Alexandre Julliard
 */
#include <stdio.h>
#include <windows.h>
#include <ntstatus.h>
#include <winternl.h>
#include <winnt.h>

#include "compiler.h"
#include "os.h"
#include "custommem.h"
#include "env.h"
#include "emu/x64emu_private.h"
#include "emu/x87emu_private.h"
#include "x64trace.h"
#include "box64context.h"
#include "wine/debug.h"

uintptr_t box64_pagesize = 4096;

uint32_t default_gs = 0x2b;
uint32_t default_fs = 0;

int box64_rdtsc = 0;
uint8_t box64_rdtsc_shift = 0;
int box64_is32bits = 0;
int box64_wine = 0; // this is for the emulated x86 Wine.

static uint32_t x86emu_parity_tab[8] =
{
    0x96696996,
    0x69969669,
    0x69969669,
    0x96696996,
    0x69969669,
    0x96696996,
    0x96696996,
    0x69969669,
};

static UINT16 DECLSPEC_ALIGN(4096) bopcode[4096/sizeof(UINT16)];
static UINT16 DECLSPEC_ALIGN(4096) unxcode[4096/sizeof(UINT16)];

typedef UINT64 unixlib_handle_t;
NTSTATUS (WINAPI *__wine_unix_call_dispatcher)( unixlib_handle_t, unsigned int, void * );

int is_addr_unaligned(uintptr_t addr)
{
    // FIXME
    return 0;
}

typedef void (*wrapper_t)(x64emu_t* emu, uintptr_t fnc);
int isSimpleWrapper(wrapper_t fun)
{
    return 0;
}

int isRetX87Wrapper(wrapper_t fun)
{
    return 0;
}

int arm64_asimd = 0;
int arm64_aes = 0;
int arm64_pmull = 0;
int arm64_crc32 = 0;
int arm64_atomics = 0;
int arm64_sha1 = 0;
int arm64_sha2 = 0;
int arm64_uscat = 0;
int arm64_flagm = 0;
int arm64_flagm2 = 0;
int arm64_frintts = 0;
int arm64_afp = 0;
int arm64_rndr = 0;

static box64context_t box64_context;
box64context_t *my_context = &box64_context;


void WINAPI BTCpuFlushInstructionCache2(LPCVOID addr, SIZE_T size)
{
    // NYI
    // invalidate all paged interleaved with this range.
    unprotectDB((uintptr_t)addr, (size_t)size, 1);
}

void* WINAPI BTCpuGetBopCode(void)
{
    return (UINT32*)&bopcode;
}

void* WINAPI __wine_get_unix_opcode(void)
{
    return (UINT32*)&unxcode;
}

NTSTATUS WINAPI BTCpuGetContext(HANDLE thread, HANDLE process, void* unknown, WOW64_CONTEXT* ctx)
{
    return NtQueryInformationThread( thread, ThreadWow64Context, ctx, sizeof(*ctx), NULL );
}

void WINAPI BTCpuNotifyMemoryFree(PVOID addr, SIZE_T size, ULONG free_type)
{
    // NYI
}

void WINAPI BTCpuNotifyMemoryProtect(PVOID addr, SIZE_T size, DWORD new_protect)
{
    // NYI
}

void WINAPI BTCpuNotifyUnmapViewOfSection(PVOID addr, ULONG flags)
{
    // NYI
}

NTSTATUS WINAPI BTCpuProcessInit(void)
{
    HMODULE module;
    UNICODE_STRING str;
    void **p__wine_unix_call_dispatcher;
    __wine_dbg_output("[BOX64] BTCpuProcessInit\n");

    LoadEnvVariables();

    memset(bopcode, 0xc3, sizeof(bopcode));
    memset(unxcode, 0xc3, sizeof(unxcode));
    bopcode[0] = 0x2ecd;
    unxcode[0] = 0x2ecd;

    init_custommem_helper(&box64_context);

    if ((ULONG_PTR)bopcode >> 32 || (ULONG_PTR)unxcode >> 32)
    {
        __wine_dbg_output( "box64cpu loaded above 4G, disabling\n" );
        return STATUS_INVALID_ADDRESS;
    }

    RtlInitUnicodeString( &str, L"ntdll.dll" );
    LdrGetDllHandle( NULL, 0, &str, &module );
    p__wine_unix_call_dispatcher = RtlFindExportedRoutineByName( module, "__wine_unix_call_dispatcher" );
    __wine_unix_call_dispatcher = *p__wine_unix_call_dispatcher;

    RtlInitializeCriticalSection(&box64_context.mutex_dyndump);
    RtlInitializeCriticalSection(&box64_context.mutex_trace);
    RtlInitializeCriticalSection(&box64_context.mutex_tls);
    RtlInitializeCriticalSection(&box64_context.mutex_thread);
    RtlInitializeCriticalSection(&box64_context.mutex_bridge);
    RtlInitializeCriticalSection(&box64_context.mutex_lock);

    InitX64Trace(&box64_context);

    __wine_dbg_output("[BOX64] BTCpuProcessInit done\n");
    return STATUS_SUCCESS;
}

NTSTATUS WINAPI BTCpuResetToConsistentState(EXCEPTION_POINTERS* ptrs)
{
    // NYI
    return STATUS_SUCCESS;
}

NTSTATUS WINAPI BTCpuSetContext(HANDLE thread, HANDLE process, void* unknown, WOW64_CONTEXT* ctx)
{
    return NtSetInformationThread( thread, ThreadWow64Context, ctx, sizeof(*ctx) );
}

void WINAPI BTCpuSimulate(void)
{
    // NYI
}

NTSTATUS WINAPI BTCpuThreadInit(void)
{
    WOW64_CONTEXT *ctx;
    x64emu_t *emu = RtlAllocateHeap( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*emu) );
    __wine_dbg_output("[BOX64] BTCpuThreadInit\n");

    RtlWow64GetCurrentCpuArea( NULL, (void **)&ctx, NULL );
    emu->context = &box64_context;

    // setup cpu helpers
    for (int i=0; i<16; ++i)
        emu->sbiidx[i] = &emu->regs[i];
    emu->sbiidx[4] = &emu->zero;
    emu->x64emu_parity_tab = x86emu_parity_tab;

    reset_fpu(emu);

    NtCurrentTeb()->TlsSlots[0] = emu;  // FIXME
    return STATUS_SUCCESS;
}

NTSTATUS WINAPI BTCpuTurboThunkControl(ULONG enable)
{
    // NYI
    return STATUS_SUCCESS;
}

NTSTATUS WINAPI LdrDisableThreadCalloutsForDll(HMODULE);

BOOL WINAPI DllMainCRTStartup(HINSTANCE inst, DWORD reason, void* reserved)
{
    if (reason == DLL_PROCESS_ATTACH) LdrDisableThreadCalloutsForDll(inst);
    return TRUE;
}
