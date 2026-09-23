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

uintptr_t box64_pagesize = 4096;
int box64_is32bits = 0;
int box64_wine = 1;
sysinfo_t box64_sysinfo = {0};
cpu_ext_t cpuext = {0};
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

    /* The CPU runtime will be introduced separately. */
    return STATUS_NOT_IMPLEMENTED;
}
