/*
 * Copyright 2022-2025 André Zwing
 * Copyright 2023 Alexandre Julliard
 */
#include <signal.h>
#include <windows.h>
#include <winternl.h>

#include "x64emu.h"
#include "debug.h"
#include "custommem.h"
#include "wine/compiler.h"

void EmitSignal(x64emu_t* emu, int sig, void* addr, int code)
{
    EXCEPTION_RECORD rec;

    switch (sig) {
        case SIGILL:
            printf_log(LOG_DEBUG, "SIGILL at %p with code %d\n", addr, code);
            rec.ExceptionCode = STATUS_ILLEGAL_INSTRUCTION;
            break;
        case SIGSEGV:
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
}

void CheckExec(x64emu_t* emu, uintptr_t addr)
{
}

void EmitDiv0(x64emu_t* emu, void* addr, int code)
{
    EXCEPTION_RECORD rec;
    rec.ExceptionCode = EXCEPTION_INT_DIVIDE_BY_ZERO;
    rec.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
    rec.ExceptionRecord = NULL;
    rec.ExceptionAddress = addr;
    rec.NumberParameters = 0;
    RtlRaiseException(&rec);
}

void EmuInt3(void* emu, void* addr)
{
    EXCEPTION_RECORD rec;

    rec.ExceptionCode = STATUS_BREAKPOINT;
    rec.ExceptionFlags = 0;
    rec.ExceptionRecord = NULL;
    rec.ExceptionAddress = addr;
    rec.NumberParameters = 0;
    RtlRaiseException(&rec);
}
