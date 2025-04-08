#include <stdio.h>
#include <windows.h>
#include <ntstatus.h>
#include <winnt.h>

#include "os.h"
#include "custommem.h"

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
