// SPDX-License-Identifier: MIT
#include <windows.h>
#include <winternl.h>

NTSYSAPI NTSTATUS NTAPI LdrDisableThreadCalloutsForDll(HMODULE);

BOOL WINAPI DllMainCRTStartup(HINSTANCE inst, DWORD reason, void* reserved)
{
    (void)reserved;
    if (reason == DLL_PROCESS_ATTACH)
        LdrDisableThreadCalloutsForDll(inst);
    return TRUE;
}
