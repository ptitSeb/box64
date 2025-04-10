#include <stdio.h>
#include <windows.h>
#include <ntstatus.h>
#include <winnt.h>

#include "os.h"
#include "custommem.h"
#include "env.h"
#include "box64context.h"

uintptr_t box64_pagesize = 4096;

int box64_is32bits = 0;
int box64_wine = 0; // this is for the emulated x86 Wine.

box64env_t box64env = { 0 }; // FIXME: add real env support.

box64env_t* GetCurEnvByAddr(uintptr_t addr) {
    return &box64env;
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
    // unprotectDB((uintptr_t)addr, (size_t)size, 1);
}

void* WINAPI BTCpuGetBopCode(void)
{
    // NYI
    return (UINT32*)NULL;
}

NTSTATUS WINAPI BTCpuGetContext(HANDLE thread, HANDLE process, void* unknown, WOW64_CONTEXT* ctx)
{
    // NYI
    return STATUS_SUCCESS;
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
    // NYI
    return STATUS_SUCCESS;
}

NTSTATUS WINAPI BTCpuResetToConsistentState(EXCEPTION_POINTERS* ptrs)
{
    // NYI
    return STATUS_SUCCESS;
}

NTSTATUS WINAPI BTCpuSetContext(HANDLE thread, HANDLE process, void* unknown, WOW64_CONTEXT* ctx)
{
    // NYI
    return STATUS_SUCCESS;
}

void WINAPI BTCpuSimulate(void)
{
    // NYI
}

NTSTATUS WINAPI BTCpuThreadInit(void)
{
    // NYI
    return STATUS_SUCCESS;
}

NTSTATUS WINAPI BTCpuTurboThunkControl(ULONG enable)
{
    // NYI
    return STATUS_SUCCESS;
}
