#include <sys/syscall.h>
#include <sched.h>
#include <unistd.h>
#include <stdint.h>

#include "os.h"
#include "signals.h"

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
