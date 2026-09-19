// SPDX-License-Identifier: MIT
#define WIN32_NO_STATUS
#include <windows.h>
#undef WIN32_NO_STATUS
#include <ntstatus.h>
#include <winternl.h>

NTSYSAPI NTSTATUS NTAPI LdrDisableThreadCalloutsForDll(HMODULE);

BOOL WINAPI DllMainCRTStartup(HINSTANCE inst, DWORD reason, void* reserved)
{
    (void)reserved;
    if (reason == DLL_PROCESS_ATTACH)
        LdrDisableThreadCalloutsForDll(inst);
    return TRUE;
}

NTSTATUS WINAPI ProcessInit(void)
{
    /* The CPU runtime will be introduced separately. */
    return STATUS_NOT_IMPLEMENTED;
}
