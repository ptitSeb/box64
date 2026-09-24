// SPDX-License-Identifier: MIT
#ifndef BOX64EC_H
#define BOX64EC_H

#include <windows.h>
#include <winternl.h>

/* Wine / Windows CHPE v2 CPU area (TEB+0x1788) */
typedef struct _CHPE_V2_CPU_AREA_INFO {
    BOOLEAN             InSimulation;         /* 000 */
    BOOLEAN             InSyscallCallback;    /* 001 */
    ULONG64             EmulatorStackBase;    /* 008 */
    ULONG64             EmulatorStackLimit;   /* 010 */
    ARM64EC_NT_CONTEXT *ContextAmd64;         /* 018 */
    ULONG              *SuspendDoorbell;      /* 020 */
    ULONG64             LoadingModuleModflag; /* 028 */
    void               *EmulatorData[4];      /* 030 */
    ULONG64             EmulatorDataInline;   /* 050 */
} CHPE_V2_CPU_AREA_INFO, *PCHPE_V2_CPU_AREA_INFO;

#define TEB_CHPE_V2_CPU_AREA_OFFSET 0x1788

#define EC_DATA_EMU 0
#define EC_DATA_THR 1

static inline CHPE_V2_CPU_AREA_INFO* Box64EC_GetCpuArea(void)
{
    return *(CHPE_V2_CPU_AREA_INFO**)((char*)NtCurrentTeb() + TEB_CHPE_V2_CPU_AREA_OFFSET);
}

#endif /* BOX64EC_H */
