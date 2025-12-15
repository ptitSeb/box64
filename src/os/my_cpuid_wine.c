#include <windows.h>

#include "my_cpuid.h"
#include "debug.h"
#include "build_info.h"

NTSYSAPI ULONG WINAPI NtGetTickCount(VOID);
NTSYSAPI ULONG NTAPI RtlRandom(ULONG *seed);

static int nCPU = 0;
static int read_ncpu = 0;

void grabNCpu() {
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    nCPU = sysinfo.dwNumberOfProcessors;
}
int get_cpuMhz() {
    return 2000;    //TODO: find a way to get realy value instead of arbitrary default to 2.0GHz
}
int getNCpu()
{
    if(!nCPU)
        grabNCpu();
    if(BOX64ENV(maxcpu) && nCPU>BOX64ENV(maxcpu))
        return BOX64ENV(maxcpu);
    return nCPU;
}
int getNCpuUnmasked()
{
    if(!nCPU)
        grabNCpu();
    return nCPU;
}
int canNCpuBeChanged()
{
    return read_ncpu?0:1;
}

const char* getBoxCpuName()
{
    return BOX64_STR;
}

uint32_t helper_getcpu(x64emu_t* emu) {
    return 0;
}

uint32_t get_random32(void)
{
    ULONG seed = NtGetTickCount();
    return RtlRandom(&seed);
}

uint64_t get_random64(void)
{
    ULONG seed = NtGetTickCount();
    uint64_t tmp = RtlRandom(&seed);
    return RtlRandom(&seed) | (tmp << 32);
}
