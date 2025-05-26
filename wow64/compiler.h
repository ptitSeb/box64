#ifndef __COMPILER_H_
#define __COMPILER_H_

#include <stdint.h>

/* Things missing from mingw64 right now */
#define ThreadWow64Context (29)
#define WOW64_TLS_CPURESERVED (1)
#define WOW64_TLS_MAX_NUMBER (19)
#define WOW64_CPURESERVED_FLAG_RESET_STATE (1)

typedef struct _WOW64_CPURESERVED
{
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

NTSTATUS WINAPI RtlWow64GetCurrentCpuArea(USHORT *, void **, void **);
NTSTATUS  WINAPI Wow64SystemServiceEx(UINT, UINT*);
NTSYSAPI NTSTATUS WINAPI LdrGetDllHandle(LPCWSTR, ULONG, const UNICODE_STRING*, HMODULE*);
NTSYSAPI NTSTATUS WINAPI NtContinue(PCONTEXT, BOOLEAN);
NTSYSAPI void* WINAPI RtlFindExportedRoutineByName(HMODULE, const char*);
NTSYSAPI void DECLSPEC_NORETURN WINAPI RtlRaiseStatus(NTSTATUS);

static inline uintptr_t calculate_fs(void)
{
/* until mingw64 has WowTebOffset in the TEB struct */
uint8_t* teb = (uint8_t*)NtCurrentTeb();
return (uintptr_t)(teb + *(int32_t*)(teb + 0x180c));
}

#endif //__COMPILER_H_
