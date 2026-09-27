// SPDX-License-Identifier: MIT
#ifndef BOX64EC_H
#define BOX64EC_H

#include <windows.h>
#include <winternl.h>
#include <stdint.h>

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
#define EC_DATA_ENTER 2
#define EC_DATA_ENTER_CTX 3

extern unsigned int va_bits;
#define BOX64EC_VA_BITS va_bits
extern uint64_t Box64EC_EcCodeBitmapLimit;

typedef struct {
    uint64_t sp;
    uint64_t pc;
} Box64EC_EntryFrame;

struct x64emu_s;
void Box64EC_EnterX64(uint64_t rip);
void Box64EC_BeginSimulation(void) __attribute__((noreturn));
void Box64EC_FinishBeginSimulation(ARM64EC_NT_CONTEXT* context,
                                  uintptr_t target, uintptr_t native_sp)
    __attribute__((noreturn));
int Box64EC_IsEcCode(uintptr_t address);
void Box64EC_RequestExitToNative(struct x64emu_s* emu, uintptr_t address);
uintptr_t Box64EC_TakePendingNativeExit(uintptr_t* native_sp);
uintptr_t Box64EC_ResolveNoPendingExit(uintptr_t rip);
void Box64EC_FatalEnterJit(unsigned reason);
int Box64EC_IsPrivilegedInstruction(void* address);
int Box64EC_IsEmulatorStackAddress(uint64_t address);
int Box64EC_PollSuspend(void);
void Box64EC_HandleSuspend(struct x64emu_s* emu);
void Box64EC_RaiseGuestException(EXCEPTION_RECORD* record);
void Box64EC_CaptureContextMxcsr(ARM64EC_NT_CONTEXT* context);
void Box64EC_ApplyContextMxcsr(ARM64EC_NT_CONTEXT* context);
struct x64emu_s* Box64EC_ApplyLiveMxcsr(struct x64emu_s* emu);
struct x64emu_s* Box64EC_CaptureLiveMxcsr(struct x64emu_s* emu);

static inline CHPE_V2_CPU_AREA_INFO* Box64EC_GetCpuArea(void)
{
    return *(CHPE_V2_CPU_AREA_INFO**)((char*)NtCurrentTeb() + TEB_CHPE_V2_CPU_AREA_OFFSET);
}

#endif /* BOX64EC_H */
