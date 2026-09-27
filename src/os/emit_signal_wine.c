/*
 * Copyright 2022-2025 André Zwing
 * Copyright 2023 Alexandre Julliard
 */
#include <windows.h>
#include <winternl.h>
#include <ntstatus.h>
#include <string.h>
#include <stdint.h>

#include "x64_signals.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "regs.h"
#include "debug.h"
#include "custommem.h"
#include "wine/compiler.h"
#ifdef BOX64EC
#include "box64ec.h"
#include "os.h"
#include "emu/x64run_private.h"

#endif

void EmitSignal(x64emu_t* emu, int sig, void* addr, int code)
{
#ifdef BOX64EC
    EXCEPTION_RECORD rec;

    memset(&rec, 0, sizeof(rec));
    rec.ExceptionAddress = addr;

    switch (sig) {
        case X64_SIGILL:
            printf_log(LOG_DEBUG, "box64ec SIGILL at %p code=%d\n", addr, code);
            rec.ExceptionCode = STATUS_ILLEGAL_INSTRUCTION;
            break;
        case X64_SIGSEGV:
            printf_log(LOG_DEBUG, "box64ec SIGSEGV at %p code=0x%x\n", addr, code);
            if (code == 0xbad0 && Box64EC_IsPrivilegedInstruction(addr)) {
                rec.ExceptionCode = STATUS_PRIVILEGED_INSTRUCTION;
                break;
            }
            /* Real AV shape (2 params). Do NOT mark NONCONTINUABLE - that
             * forces Wine into a second-chance path while still on the
             * emulator stack and nests past EmulatorStackBase. */
            rec.ExceptionCode = STATUS_ACCESS_VIOLATION;
            rec.NumberParameters = 2;
            rec.ExceptionInformation[0] = 0; /* read */
            rec.ExceptionInformation[1] = code == 0xbad0
                ? (ULONG_PTR)-1 : (ULONG_PTR)addr;
            break;
        case X64_SIGTRAP:
            printf_log(LOG_DEBUG, "box64ec SIGTRAP at %p code=%d\n", addr, code);
            rec.ExceptionCode = code == 1 ? STATUS_SINGLE_STEP : STATUS_BREAKPOINT;
            if (code == 1)
                emu->eflags.x64 &= ~(1u << 8);
            break;
        default:
            printf_log(LOG_DEBUG, "box64ec unknown signal %d at %p code=%d\n",
                             sig, addr, code);
            rec.ExceptionCode = STATUS_ILLEGAL_INSTRUCTION;
            break;
    }
    Box64EC_RaiseGuestException(&rec);
#else
    EXCEPTION_RECORD rec;

    switch (sig) {
        case X64_SIGILL:
            printf_log(LOG_DEBUG, "SIGILL at %p with code %d\n", addr, code);
            rec.ExceptionCode = STATUS_ILLEGAL_INSTRUCTION;
            break;
        case X64_SIGSEGV:
            printf_log(LOG_DEBUG, "SIGSEGV at %p with code %d\n", addr, code);
            rec.ExceptionCode = STATUS_ACCESS_VIOLATION;
            break;
        default:
            printf_log(LOG_INFO, "Warning, unknown signal %d at %p with code %d\n", sig, addr, code);
            rec.ExceptionCode = STATUS_ACCESS_VIOLATION;
            break;
    }
    rec.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
    rec.ExceptionRecord = NULL;
    rec.ExceptionAddress = addr;
    rec.NumberParameters = 0;
    RtlRaiseException(&rec);
#endif
}

void CheckExec(x64emu_t* emu, uintptr_t addr)
{
#ifdef BOX64EC
    /* Hitting ARM64EC DLL code while simulating → leave the interpreter. */
    if (Box64EC_IsEcCode(addr)) {
        printf_log(LOG_DEBUG, "box64ec interpreter-ec-exit addr=%p rip=%p rsp=%p\n",
                    (void*)addr, (void*)(uintptr_t)R_RIP,
                    (void*)(uintptr_t)R_RSP);
        Box64EC_RequestExitToNative(emu, addr);
    }
#else
    (void)emu;
    (void)addr;
#endif
}

void EmitDiv0(x64emu_t* emu, void* addr, int code)
{
#ifdef BOX64EC
    EXCEPTION_RECORD rec;
    (void)emu; (void)code;
    memset(&rec, 0, sizeof(rec));
    printf_log(LOG_DEBUG, "box64ec DIV0 at %p\n", addr);
    rec.ExceptionCode = EXCEPTION_INT_DIVIDE_BY_ZERO;
    rec.ExceptionAddress = addr;
    Box64EC_RaiseGuestException(&rec);
#else
    EXCEPTION_RECORD rec;
    rec.ExceptionCode = EXCEPTION_INT_DIVIDE_BY_ZERO;
    rec.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
    rec.ExceptionRecord = NULL;
    rec.ExceptionAddress = addr;
    rec.NumberParameters = 0;
    RtlRaiseException(&rec);
#endif
}

void EmuInt3(void* emu, void* addr)
{
#ifdef BOX64EC
    EXCEPTION_RECORD rec;
    (void)emu;

    memset(&rec, 0, sizeof(rec));
    printf_log(LOG_DEBUG, "box64ec INT3 at %p\n", addr);
    rec.ExceptionCode = STATUS_BREAKPOINT;
    rec.ExceptionAddress = addr;
    Box64EC_RaiseGuestException(&rec);
#else
    EXCEPTION_RECORD rec;

    rec.ExceptionCode = STATUS_BREAKPOINT;
    rec.ExceptionFlags = 0;
    rec.ExceptionRecord = NULL;
    rec.ExceptionAddress = addr;
    rec.NumberParameters = 0;
    RtlRaiseException(&rec);
#endif
}
