#include <stddef.h>
#include <windows.h>
#include <ntstatus.h>
#include <winternl.h>

#include "debug.h"
#include "wine/compiler.h"
#include "wine/debug.h"


#define FILE_ATTRIBUTE_VALID_FLAGS 0x00007fb7

static UINT get_nt_file_options(DWORD attributes)
{
    UINT options = 0;

    if (attributes & FILE_FLAG_BACKUP_SEMANTICS)
        options |= FILE_OPEN_FOR_BACKUP_INTENT;
    else
        options |= FILE_NON_DIRECTORY_FILE;
    if (attributes & FILE_FLAG_DELETE_ON_CLOSE)
        options |= FILE_DELETE_ON_CLOSE;
    if (attributes & FILE_FLAG_NO_BUFFERING)
        options |= FILE_NO_INTERMEDIATE_BUFFERING;
    if (!(attributes & FILE_FLAG_OVERLAPPED))
        options |= FILE_SYNCHRONOUS_IO_NONALERT;
    if (attributes & FILE_FLAG_RANDOM_ACCESS)
        options |= FILE_RANDOM_ACCESS;
    if (attributes & FILE_FLAG_SEQUENTIAL_SCAN)
        options |= FILE_SEQUENTIAL_ONLY;
    if (attributes & FILE_FLAG_WRITE_THROUGH)
        options |= FILE_WRITE_THROUGH;
    return options;
}

HANDLE WINAPI CreateFileW(LPCWSTR filename, DWORD access, DWORD sharing, LPSECURITY_ATTRIBUTES sa, DWORD creation, DWORD attributes, HANDLE template)
{
    UNICODE_STRING filenameU;
    OBJECT_ATTRIBUTES attr;
    IO_STATUS_BLOCK io;
    HANDLE ret;

    static const UINT nt_disposition[5] = { FILE_CREATE, FILE_OVERWRITE_IF, FILE_OPEN, FILE_OPEN_IF, FILE_OVERWRITE };
    if (!RtlDosPathNameToNtPathName_U(filename, &filenameU, NULL, NULL)) {
        return INVALID_HANDLE_VALUE;
    }
    if (attributes & FILE_FLAG_DELETE_ON_CLOSE) access |= DELETE;
    InitializeObjectAttributes(&attr, &filenameU, OBJ_CASE_INSENSITIVE, 0, NULL);
    NTSTATUS status = NtCreateFile(&ret, access | GENERIC_READ | SYNCHRONIZE, &attr, &io, NULL, attributes & FILE_ATTRIBUTE_VALID_FLAGS, sharing,
        nt_disposition[creation - CREATE_NEW], get_nt_file_options(attributes), NULL, 0);
    RtlFreeUnicodeString(&filenameU);
    return status ? INVALID_HANDLE_VALUE : ret;
}

HANDLE WINAPI CreateFileA(LPCSTR name, DWORD access, DWORD sharing, LPSECURITY_ATTRIBUTES sa, DWORD creation, DWORD attributes, HANDLE template)
{
    UNICODE_STRING nameU;
    RtlCreateUnicodeStringFromAsciiz(&nameU, name);
    HANDLE handle = CreateFileW(nameU.Buffer, access, sharing, sa, creation, attributes, template);
    RtlFreeUnicodeString(&nameU);
    return handle;
}


BOOL WINAPI CloseHandle(HANDLE handle)
{
    return !NtClose(handle);
}

DWORD WINAPI GetEnvironmentVariableA(LPCSTR name, LPSTR value, DWORD size)
{
    UNICODE_STRING nameU, valueU;
    PWSTR valueW;
    NTSTATUS status;
    DWORD len, ret;

    if (!(valueW = RtlAllocateHeap(GetProcessHeap(), 0, size * sizeof(WCHAR)))) return 0;

    RtlCreateUnicodeStringFromAsciiz(&nameU, name);
    valueU.Length = 0;
    valueU.MaximumLength = (size ? size - 1 : 0) * sizeof(WCHAR);
    valueU.Buffer = valueW;

    status = RtlQueryEnvironmentVariable_U(NULL, &nameU, &valueU);
    len = valueU.Length / sizeof(WCHAR);
    if (status == STATUS_BUFFER_TOO_SMALL)
        ret = len + 1;
    else if (status)
        ret = 0;
    else if (!size)
        ret = len + 1;
    else {
        if (len) RtlUnicodeToMultiByteN(value, size, &ret, valueW, len * sizeof(WCHAR));
    }

    RtlFreeUnicodeString(&nameU);
    RtlFreeHeap(GetProcessHeap(), 0, valueW);
    return ret;
}

DWORD WINAPI GetFileAttributesW(LPCWSTR name)
{
    FILE_BASIC_INFORMATION info;
    UNICODE_STRING nameU;
    OBJECT_ATTRIBUTES attr;
    NTSTATUS status;

    if (!RtlDosPathNameToNtPathName_U(name, &nameU, NULL, NULL)) {
        return INVALID_FILE_ATTRIBUTES;
    }

    InitializeObjectAttributes(&attr, &nameU, OBJ_CASE_INSENSITIVE, 0, NULL);
    status = NtQueryAttributesFile(&attr, &info);
    RtlFreeUnicodeString(&nameU);

    if (status == STATUS_SUCCESS) return info.FileAttributes;
    if (RtlIsDosDeviceName_U(name)) return FILE_ATTRIBUTE_ARCHIVE;
    return INVALID_FILE_ATTRIBUTES;
}

DWORD WINAPI GetFileAttributesA(LPCSTR name)
{
    UNICODE_STRING nameU;
    RtlCreateUnicodeStringFromAsciiz(&nameU, name);
    DWORD ret = GetFileAttributesW(nameU.Buffer);
    RtlFreeUnicodeString(&nameU);
    return ret;
}

HANDLE WINAPI GetProcessHeap(void)
{
    return ((HANDLE**)NtCurrentTeb())[12][6];
}


DWORD WINAPI GetModuleFileNameW(HMODULE module, LPWSTR filename, DWORD size)
{
    ULONG len = 0;
    UNICODE_STRING filenameU;
    NTSTATUS status;

    filenameU.Buffer = filename;
    filenameU.MaximumLength = min(size, UNICODE_STRING_MAX_CHARS) * sizeof(WCHAR);
    status = LdrGetDllFullName(module, &filenameU);
    if (!status || status == STATUS_BUFFER_TOO_SMALL) len = filenameU.Length / sizeof(WCHAR);
    return len;
}


DWORD WINAPI GetModuleFileNameA(HMODULE module, LPSTR filename, DWORD size)
{
    LPWSTR filenameW = RtlAllocateHeap(GetProcessHeap(), 0, size * sizeof(WCHAR));
    DWORD len, ret = 0;

    if (!filenameW) return 0;

    if ((len = GetModuleFileNameW(module, filenameW, size))) {
        RtlUnicodeToMultiByteN(filename, size, &ret, filenameW, len * sizeof(WCHAR));
        if (ret < size) filename[ret] = 0;
    }
    RtlFreeHeap(GetProcessHeap(), 0, filenameW);
    return ret;
}

BOOL WINAPI ReadFile(HANDLE file, LPVOID buffer, DWORD count, LPDWORD result, LPOVERLAPPED overlapped)
{
    LARGE_INTEGER offset;
    PLARGE_INTEGER poffset = NULL;
    IO_STATUS_BLOCK iosb;
    PIO_STATUS_BLOCK io_status = &iosb;
    HANDLE event = 0;
    NTSTATUS status;
    LPVOID cvalue = NULL;

    if (result) *result = 0;

    if (overlapped) {
        printf_log(LOG_NONE, "unimplemented overlapped in ReadFile\n");
        return FALSE;
    } else
        io_status->Information = 0;
    io_status->Status = STATUS_PENDING;

    status = NtReadFile(file, event, NULL, cvalue, io_status, buffer, count, poffset, NULL);

    if (status == STATUS_PENDING && !overlapped) {
        printf_log(LOG_NONE, "unimplemented pending in ReadFile\n");
    }

    if (result) *result = overlapped && status ? 0 : io_status->Information;

    if (status == STATUS_END_OF_FILE) {
        if (overlapped != NULL) {
            return FALSE;
        }
    } else if (status && status != STATUS_TIMEOUT) {
        return FALSE;
    }
    return TRUE;
}


static void fillSystemInfo(SYSTEM_INFO* si, const SYSTEM_BASIC_INFORMATION__* basic_info, const SYSTEM_CPU_INFORMATION* cpu_info)
{
    si->wProcessorArchitecture = cpu_info->ProcessorArchitecture;
    si->wReserved = 0;
    si->dwPageSize = basic_info->PageSize;
    si->lpMinimumApplicationAddress = basic_info->LowestUserAddress;
    si->lpMaximumApplicationAddress = basic_info->HighestUserAddress;
    si->dwActiveProcessorMask = basic_info->ActiveProcessorsAffinityMask;
    si->dwNumberOfProcessors = basic_info->NumberOfProcessors;
    si->dwAllocationGranularity = basic_info->AllocationGranularity;
    si->wProcessorLevel = cpu_info->ProcessorLevel;
    si->wProcessorRevision = cpu_info->ProcessorRevision;

    switch (cpu_info->ProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_ARM64:
            si->dwProcessorType = 0;
            break;
        default:
            printf_log(LOG_NONE, "Unknown processor architecture %x\n", cpu_info->ProcessorArchitecture);
            si->dwProcessorType = 0;
            break;
    }
}

void WINAPI GetSystemInfo(SYSTEM_INFO* si)
{
    SYSTEM_BASIC_INFORMATION__ basic_info;
    SYSTEM_CPU_INFORMATION cpu_info;

    if (NtQuerySystemInformation(0 /* SystemBasicInformation */, &basic_info, sizeof(basic_info), NULL)
        || NtQuerySystemInformation(1 /* SystemCpuInformation */, &cpu_info, sizeof(cpu_info), NULL))
        return;

    fillSystemInfo(si, &basic_info, &cpu_info);
}

BOOL WINAPI IsProcessorFeaturePresent(DWORD feature)
{
    return RtlIsProcessorFeaturePresent(feature);
}

LSTATUS WINAPI RegOpenKeyExA(HKEY hkey, LPCSTR name, DWORD options, REGSAM access, PHKEY retkey)
{
    if (hkey != HKEY_LOCAL_MACHINE) {
        printf_log(LOG_NONE, "Unsupported registry key %p\n", hkey);
        return ERROR_INVALID_HANDLE;
    }

    UNICODE_STRING rootkeyU;
    UNICODE_STRING nameU;
    OBJECT_ATTRIBUTES attr;
    NTSTATUS status;
    HANDLE handle;
    RtlInitUnicodeString(&rootkeyU, L"\\Registry\\Machine");
    InitializeObjectAttributes(&attr, &rootkeyU, OBJ_CASE_INSENSITIVE, 0, NULL);
    if (NtOpenKey(&handle, access, &attr)) return RtlNtStatusToDosError(status);

    RtlCreateUnicodeStringFromAsciiz(&nameU, name);
    InitializeObjectAttributes(&attr, &nameU, OBJ_CASE_INSENSITIVE, handle, NULL);
    status = NtOpenKey((HANDLE*)retkey, access, &attr);
    RtlFreeUnicodeString(&nameU);
    NtClose(handle);
    return RtlNtStatusToDosError(status);
}

LSTATUS WINAPI RegQueryValueExA(HKEY hkey, LPCSTR name, LPDWORD reserved, LPDWORD type, LPBYTE data, LPDWORD count)
{
    NTSTATUS status;
    ANSI_STRING nameA;
    UNICODE_STRING nameW;
    DWORD total_size, datalen = 0;
    char buffer[256];
    KEY_VALUE_PARTIAL_INFORMATION* info = (KEY_VALUE_PARTIAL_INFORMATION*)buffer;
    static const int info_size = offsetof(KEY_VALUE_PARTIAL_INFORMATION, Data);

    if ((data && !count) || reserved) return ERROR_INVALID_PARAMETER;

    if (count) datalen = *count;
    if (!data && count) *count = 0;

    RtlInitAnsiString(&nameA, name);
    if ((status = RtlAnsiStringToUnicodeString(&nameW, &nameA, TRUE)))
        return RtlNtStatusToDosError(status);

    status = NtQueryValueKey(hkey, &nameW, KeyValuePartialInformation, buffer, sizeof(buffer), &total_size);
    if (status) {
        RtlFreeUnicodeString(&nameW);
        return RtlNtStatusToDosError(status);
    }

    if (data) {
        if (total_size - info_size > datalen)
            status = STATUS_BUFFER_OVERFLOW;
        else
            memcpy(data, buffer + info_size, total_size - info_size);
    } else
        status = STATUS_SUCCESS;

    if (type) *type = info->Type;
    if (count) *count = total_size - info_size;

    RtlFreeUnicodeString(&nameW);
    return RtlNtStatusToDosError(status);
}

LSTATUS WINAPI RegCloseKey(HKEY hkey)
{
    if (!hkey) return ERROR_INVALID_HANDLE;
    return RtlNtStatusToDosError(NtClose(hkey));
}
