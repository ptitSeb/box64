// SPDX-License-Identifier: MIT
/*
 * Wine direct syscalls for box64ec.
 *
 * Exported NtContinue converts through ARM64EC_NT_CONTEXT and drops EC-disallowed
 * host regs. Resume from the emulator stack needs a native ARM64_NT_CONTEXT
 * continue via __wine_syscall_dispatcher.
 */
#include <string.h>
#include <stddef.h>
#include <windows.h>
#include <ntstatus.h>
#include <winternl.h>
#include <winnt.h>

#include "box64ec.h"
#include "box64ec_syscalls.h"
#include "debug.h"
#include "wine/compiler.h"

NTSYSAPI void* WINAPI RtlImageDirectoryEntryToData(HMODULE, BOOLEAN, USHORT, ULONG*);

void* Box64EC_WineSyscallDispatcher;
uint64_t Box64EC_WineNtContinueSyscallId = 0x43; /* Windows default; overwritten under Wine */
uintptr_t Box64EC_KiUserExceptionDispatcher;

extern NTSTATUS Box64EC_NtContinueNative(ARM64_NT_CONTEXT* ctx, BOOLEAN alert);

/* mingw may lack these ARM64EC image metadata definitions. */
typedef struct {
    ULONG Version;
    ULONG CodeMap;
    ULONG CodeMapCount;
    ULONG CodeRangesToEntryPoints;
    ULONG RedirectionMetadata;
    ULONG __os_arm64x_dispatch_call_no_redirect;
    ULONG __os_arm64x_dispatch_ret;
    ULONG __os_arm64x_dispatch_call;
    ULONG __os_arm64x_dispatch_icall;
    ULONG __os_arm64x_dispatch_icall_cfg;
    ULONG AlternateEntryPoint;
    ULONG AuxiliaryIAT;
    ULONG CodeRangesToEntryPointsCount;
    ULONG RedirectionMetadataCount;
} BOX64EC_IMAGE_ARM64EC_METADATA;

typedef struct {
    ULONG Source;
    ULONG Destination;
} BOX64EC_IMAGE_ARM64EC_REDIRECTION_ENTRY;

static uintptr_t resolve_ki_user_exception_dispatcher(HMODULE ntdll, uintptr_t ntdll_base)
{
    uintptr_t ffs, native;
    ULONG size = 0;
    IMAGE_LOAD_CONFIG_DIRECTORY64* lc;
    BOX64EC_IMAGE_ARM64EC_METADATA* meta;
    BOX64EC_IMAGE_ARM64EC_REDIRECTION_ENTRY* tab;
    ULONG i;

    ffs = (uintptr_t)RtlFindExportedRoutineByName(ntdll, "KiUserExceptionDispatcher");
    if (!ffs)
        return 0;

    native = ffs;
    lc = (IMAGE_LOAD_CONFIG_DIRECTORY64*)RtlImageDirectoryEntryToData(
        ntdll, TRUE, IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG, &size);
    if (lc && lc->CHPEMetadataPointer) {
        meta = (BOX64EC_IMAGE_ARM64EC_METADATA*)(uintptr_t)lc->CHPEMetadataPointer;
        if (meta->RedirectionMetadata && meta->RedirectionMetadataCount) {
            tab = (BOX64EC_IMAGE_ARM64EC_REDIRECTION_ENTRY*)(ntdll_base + meta->RedirectionMetadata);
            for (i = 0; i < meta->RedirectionMetadataCount; ++i) {
                if (ntdll_base + tab[i].Source == ffs) {
                    native = ntdll_base + tab[i].Destination;
                    break;
                }
            }
            /* Redirection entries use the source RVA returned by GetProcAddress. */
            if (native == ffs && ffs >= ntdll_base) {
                ULONG rva = (ULONG)(ffs - ntdll_base);
                for (i = 0; i < meta->RedirectionMetadataCount; ++i) {
                    if (tab[i].Source == rva) {
                        native = ntdll_base + tab[i].Destination;
                        break;
                    }
                }
            }
        }
    }
    return native;
}

static int name_is_nt_syscall(const char* name)
{
    if (!name || name[0] != 'N' || name[1] != 't')
        return 0;
    if (strcmp(name, "NtGetTickCount") == 0)
        return 0;
    return 1;
}

static void parse_wine_syscall_numbers(HMODULE ntdll, uintptr_t ntdll_base)
{
    ULONG size = 0;
    IMAGE_EXPORT_DIRECTORY* exports;
    uint32_t* names;
    uint32_t* funcs;
    uint16_t* ords;
    uint32_t i, n = 0;
    /* Stack table: name RVA pairs sorted by function RVA ⇒ syscall id order */
    struct { uint32_t rva; const char* name; } table[0x200];

    exports = (IMAGE_EXPORT_DIRECTORY*)RtlImageDirectoryEntryToData(
        ntdll, TRUE, IMAGE_DIRECTORY_ENTRY_EXPORT, &size);
    if (!exports)
        return;

    names = (uint32_t*)(ntdll_base + exports->AddressOfNames);
    funcs = (uint32_t*)(ntdll_base + exports->AddressOfFunctions);
    ords  = (uint16_t*)(ntdll_base + exports->AddressOfNameOrdinals);

    for (i = 0; i < exports->NumberOfNames && n < 0x200; ++i) {
        const char* name = (const char*)(ntdll_base + names[i]);
        if (name_is_nt_syscall(name)) {
            table[n].name = name;
            table[n].rva = funcs[ords[i]];
            ++n;
        }
    }

    /* Insertion sort by RVA */
    for (i = 1; i < n; ++i) {
        uint32_t j = i;
        uint32_t tmp_rva = table[i].rva;
        const char* tmp_name = table[i].name;
        while (j > 0 && table[j - 1].rva > tmp_rva) {
            table[j] = table[j - 1];
            --j;
        }
        table[j].rva = tmp_rva;
        table[j].name = tmp_name;
    }

    for (i = 0; i < n; ++i) {
        if (strcmp(table[i].name, "NtContinue") == 0) {
            Box64EC_WineNtContinueSyscallId = i;
            printf_log(LOG_DEBUG, "box64ec: NtContinue syscall id=%u\n", i);
            break;
        }
    }
}

NTSTATUS Box64EC_InitSyscalls(void)
{
    UNICODE_STRING name;
    HMODULE ntdll = NULL;
    void** disp;
    NTSTATUS status;

    RtlInitUnicodeString(&name, L"ntdll.dll");
    status = LdrGetDllHandle(NULL, 0, &name, &ntdll);
    if (status || !ntdll) {
        printf_log(LOG_DEBUG, "box64ec: LdrGetDllHandle(ntdll) failed %08x\n", (unsigned)status);
        return status ? status : STATUS_NOT_FOUND;
    }

    disp = (void**)RtlFindExportedRoutineByName(ntdll, "__wine_syscall_dispatcher");
    if (disp && *disp) {
        Box64EC_WineSyscallDispatcher = *disp;
        printf_log(LOG_DEBUG, "box64ec: WineSyscallDispatcher=%p\n", *disp);
        parse_wine_syscall_numbers(ntdll, (uintptr_t)ntdll);
    } else {
        printf_log(LOG_DEBUG, "box64ec: no __wine_syscall_dispatcher (native SVC fallback)\n");
        Box64EC_WineSyscallDispatcher = NULL;
    }

    Box64EC_KiUserExceptionDispatcher =
        resolve_ki_user_exception_dispatcher(ntdll, (uintptr_t)ntdll);
    printf_log(LOG_DEBUG, "box64ec: KiUserExceptionDispatcher=%p\n",
               (void*)Box64EC_KiUserExceptionDispatcher);

    return STATUS_SUCCESS;
}

NTSTATUS Box64EC_ContinueNative(ARM64_NT_CONTEXT* ctx, BOOLEAN alert)
{
    if (!ctx)
        return STATUS_INVALID_PARAMETER;
    return Box64EC_NtContinueNative(ctx, alert);
}
