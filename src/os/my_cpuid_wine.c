#include <windows.h>

#include "my_cpuid.h"

NTSYSAPI ULONG WINAPI NtGetTickCount(VOID);
NTSYSAPI ULONG NTAPI RtlRandom(ULONG *seed);

const char* getBoxCpuName()
{
    return NULL;
}

void my_cpuid(x64emu_t* emu, uint32_t tmp32u)
{
    // FIXME
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
