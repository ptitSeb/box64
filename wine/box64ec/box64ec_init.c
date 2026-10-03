// SPDX-License-Identifier: MIT
#define WIN32_NO_STATUS
#include <windows.h>
#undef WIN32_NO_STATUS
#include <ntstatus.h>
#include <string.h>

#include "env.h"
#include "sysinfo.h"
#include "box64context.h"
#include "custommem.h"
#include "hostext.h"
#include "box64ec_private.h"
#include "box64ec_syscalls.h"
#include "wine/compiler.h"

uintptr_t box64_pagesize = 4096;
uint32_t default_fs = 0;
int box64_is32bits = 0;
int box64_unittest_mode = 0;
uint8_t box64_rdtsc_shift = 0;
int box64_wine = 1;
sysinfo_t box64_sysinfo = {0};
cpu_ext_t cpuext = {0};
unsigned int va_bits = 39;
uint64_t Box64EC_EcCodeBitmapLimit = 1ull << 39;
box64context_t Box64EC_Context;
box64context_t* my_context = &Box64EC_Context;

NTSTATUS WINAPI ProcessInit(void)
{
    char filename[MAX_PATH];

    LoadEnvVariables();
    init_custommem_helper(&Box64EC_Context);
    InitializeEnvFiles();

    DWORD length = GetModuleFileNameA(NULL, filename, sizeof(filename));
    if (length && length < sizeof(filename)) {
        const char* profile = strrchr(filename, '\\');
        ApplyEnvFileEntry(profile ? profile + 1 : filename);
    }

    InitializeSystemInfo();
    DetectHostCpuFeatures();

    NTSTATUS status = Box64EC_InitSyscalls();
    if (status)
        return status;
    UNICODE_STRING name;
    HMODULE ntdll;
    RtlInitUnicodeString(&name, L"ntdll.dll");
    status = LdrGetDllHandle(NULL, 0, &name, &ntdll);
    if (status)
        return status;
    status = Box64EC_TransitionProcessInit(ntdll);
    if (status)
        return status;
    if ((uintptr_t)&va_bits > (1ull << 39))
        va_bits = 48;
    Box64EC_EcCodeBitmapLimit = 1ull << (va_bits < 47 ? va_bits : 47);
    RtlInitializeCriticalSection(&Box64EC_Context.mutex_dyndump);
    RtlInitializeCriticalSection(&Box64EC_Context.mutex_trace);
    RtlInitializeCriticalSection(&Box64EC_Context.mutex_tls);
    RtlInitializeCriticalSection(&Box64EC_Context.mutex_thread);
    RtlInitializeCriticalSection(&Box64EC_Context.mutex_bridge);
    RtlInitializeCriticalSection(&Box64EC_Context.mutex_lock);
    return STATUS_SUCCESS;
}
