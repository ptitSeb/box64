#ifndef __COMPILER_H_
#define __COMPILER_H_

#include <stdint.h>

/* Things missing from mingw64 right now */
#define ThreadWow64Context                 (29)
#define WOW64_TLS_CPURESERVED              (1)
#define WOW64_TLS_MAX_NUMBER               (19)
#define WOW64_CPURESERVED_FLAG_RESET_STATE (1)

typedef enum _MEMORY_INFORMATION_CLASS {
    MemoryBasicInformation,
    MemoryWorkingSetInformation,
    MemoryMappedFilenameInformation,
    MemoryRegionInformation,
    MemoryWorkingSetExInformation,
    MemorySharedCommitInformation,
    MemoryImageInformation,
    MemoryRegionInformationEx,
    MemoryPrivilegedBasicInformation,
    MemoryEnclaveImageInformation,
    MemoryBasicInformationCapped,
    MemoryPhysicalContiguityInformation,
    MemoryBadInformation,
    MemoryBadInformationAllProcesses,
} MEMORY_INFORMATION_CLASS;

typedef struct _WOW64_CPURESERVED {
    USHORT Flags;
    USHORT Machine;
} WOW64_CPURESERVED;

typedef struct _XMM_SAVE_AREA32 {
    WORD  ControlWord;
    WORD  StatusWord;
    BYTE  TagWord;
    BYTE  Reserved1;
    WORD  ErrorOpcode;
    DWORD ErrorOffset;
    WORD  ErrorSelector;
    WORD  Reserved2;
    DWORD DataOffset;
    WORD  DataSelector;
    WORD  Reserved3;
    DWORD MxCsr;
    DWORD MxCsr_Mask;
    M128A FloatRegisters[8];
    M128A XmmRegisters[16];
    BYTE  Reserved4[96];
} XMM_SAVE_AREA32;

typedef struct _SYSTEM_CPU_INFORMATION {
    USHORT ProcessorArchitecture;
    USHORT ProcessorLevel;
    USHORT ProcessorRevision;
    USHORT MaximumProcessors;
    ULONG ProcessorFeatureBits;
} SYSTEM_CPU_INFORMATION, *PSYSTEM_CPU_INFORMATION;

typedef struct _SYSTEM_BASIC_INFORMATION__ {
    DWORD unknown;
    ULONG KeMaximumIncrement;
    ULONG PageSize;
    ULONG MmNumberOfPhysicalPages;
    ULONG MmLowestPhysicalPage;
    ULONG MmHighestPhysicalPage;
    ULONG_PTR AllocationGranularity;
    PVOID LowestUserAddress;
    PVOID HighestUserAddress;
    ULONG_PTR ActiveProcessorsAffinityMask;
    BYTE NumberOfProcessors;
} SYSTEM_BASIC_INFORMATION__, *PSYSTEM_BASIC_INFORMATION__;

typedef struct _KEY_VALUE_PARTIAL_INFORMATION {
    ULONG TitleIndex;
    ULONG Type;
    ULONG DataLength;
    UCHAR Data[1];
} KEY_VALUE_PARTIAL_INFORMATION, *PKEY_VALUE_PARTIAL_INFORMATION;

typedef enum _KEY_VALUE_INFORMATION_CLASS {
    KeyValueBasicInformation,
    KeyValueFullInformation,
    KeyValuePartialInformation,
    KeyValueFullInformationAlign64,
    KeyValuePartialInformationAlign64,
    KeyValueLayerInformation,
} KEY_VALUE_INFORMATION_CLASS;

#define NtCurrentProcess() ((HANDLE)(LONG_PTR) - 1)

NTSTATUS WINAPI RtlWow64GetCurrentCpuArea(USHORT*, void**, void**);
NTSTATUS WINAPI Wow64SystemServiceEx(UINT, UINT*);
NTSYSAPI NTSTATUS WINAPI LdrGetDllHandle(LPCWSTR, ULONG, const UNICODE_STRING*, HMODULE*);
NTSYSAPI NTSTATUS WINAPI LdrGetDllFullName(HMODULE, UNICODE_STRING*);
NTSYSAPI NTSTATUS WINAPI NtContinue(PCONTEXT, BOOLEAN);
NTSYSAPI NTSTATUS WINAPI NtQueryVirtualMemory(HANDLE, LPCVOID, MEMORY_INFORMATION_CLASS, PVOID, SIZE_T, SIZE_T*);
NTSYSAPI NTSTATUS WINAPI NtReadFile(HANDLE, HANDLE, PIO_APC_ROUTINE, PVOID, PIO_STATUS_BLOCK, PVOID, ULONG, PLARGE_INTEGER, PULONG);
NTSYSAPI void* WINAPI RtlFindExportedRoutineByName(HMODULE, const char*);
NTSYSAPI void DECLSPEC_NORETURN WINAPI RtlRaiseStatus(NTSTATUS);
NTSYSAPI void WINAPI RtlRaiseException(EXCEPTION_RECORD*);
NTSYSAPI NTSTATUS WINAPI RtlQueryEnvironmentVariable_U(PWSTR, PUNICODE_STRING, PUNICODE_STRING);
NTSYSAPI NTSTATUS WINAPI NtQueryAttributesFile(const OBJECT_ATTRIBUTES*, FILE_BASIC_INFORMATION*);
NTSYSAPI ULONG WINAPI RtlIsDosDeviceName_U(PCWSTR);
NTSYSAPI NTSTATUS WINAPI RtlUnicodeToMultiByteN(LPSTR, DWORD, LPDWORD, LPCWSTR, DWORD);
NTSTATUS WINAPI NtProtectVirtualMemory(HANDLE, PVOID*, SIZE_T*, ULONG, ULONG*);
NTSTATUS WINAPI NtAllocateVirtualMemory(HANDLE, PVOID*, ULONG_PTR, SIZE_T*, ULONG, ULONG);
PVOID WINAPI RtlReAllocateHeap(HANDLE, ULONG, PVOID, SIZE_T);
NTSTATUS WINAPI NtFreeVirtualMemory(HANDLE, PVOID*, SIZE_T*, ULONG);
BOOLEAN WINAPI RtlIsProcessorFeaturePresent(UINT feature);
NTSTATUS WINAPI NtOpenKey(HANDLE*, ACCESS_MASK, const OBJECT_ATTRIBUTES*);
NTSYSAPI NTSTATUS WINAPI NtQueryValueKey(HANDLE, const UNICODE_STRING*, KEY_VALUE_INFORMATION_CLASS, void*, DWORD, DWORD*);

static inline uintptr_t calculate_fs(void)
{
    /* until mingw64 has WowTebOffset in the TEB struct */
    uint8_t* teb = (uint8_t*)NtCurrentTeb();
    return (uintptr_t)(teb + *(int32_t*)(teb + 0x180c));
}

#endif //__COMPILER_H_
