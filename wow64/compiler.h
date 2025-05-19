#ifndef __COMPILER_H_
#define __COMPILER_H_

#include <stdint.h>

/* Things missing from mingw64 right now */
#define ThreadWow64Context (29)
#define WOW64_TLS_CPURESERVED (1)
#define WOW64_TLS_MAX_NUMBER (19)

typedef struct _WOW64_CPURESERVED
{
    USHORT Flags;
    USHORT Machine;
} WOW64_CPURESERVED;

NTSTATUS WINAPI RtlWow64GetCurrentCpuArea(USHORT *, void **, void **);
NTSYSAPI NTSTATUS WINAPI LdrGetDllHandle(LPCWSTR, ULONG, const UNICODE_STRING*, HMODULE*);
NTSYSAPI void* WINAPI RtlFindExportedRoutineByName(HMODULE, const char*);

static inline uintptr_t calculate_fs(void)
{
/* until mingw64 has WowTebOffset in the TEB struct */
uint8_t* teb = (uint8_t*)NtCurrentTeb();
return (uintptr_t)(teb + *(int32_t*)(teb + 0x180c));
}

#endif //__COMPILER_H_
