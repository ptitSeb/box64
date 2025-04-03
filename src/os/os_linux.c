#include <sys/syscall.h>
#include <sched.h>
#include <unistd.h>
#include <stdint.h>

#include "os.h"
#include "signals.h"
#include "emu/x64int_private.h"
#include "bridge.h"

int GetTID(void)
{
    return syscall(SYS_gettid);
}

int SchedYield(void)
{
    return sched_yield();
}

void EmitSignal(void* emu, int sig, void* addr, int code)
{
    return emit_signal((x64emu_t*)emu, sig, addr, code);
}

void EmitDiv0(void* emu, void* addr, int code)
{
    return emit_div0((x64emu_t*)emu, addr, code);
}

void EmitInterruption(void* emu, int num, void* addr)
{
    return emit_interruption((x64emu_t*)emu, num, addr);
}

int IsBridgeSignature(char s, char c)
{
    return s == 'S' && c == 'C';
}

void EmuInt3(void* emu, void* addr)
{
    return x64Int3((x64emu_t*)emu, (uintptr_t*)addr);
}

int IsNativeCall(uintptr_t addr, int is32bits, uintptr_t* calladdress, uint16_t* retn)
{
    return isNativeCallInternal(addr, is32bits, calladdress, retn);
}

void* EmuFork(void* emu, int forktype)
{
    return x64emu_fork((x64emu_t*)emu, forktype);
}

void EmuX64Syscall(void* emu)
{
    x64Syscall((x64emu_t*)emu);
}

void EmuX86Syscall(void* emu)
{
    x86Syscall((x64emu_t*)emu);
}
