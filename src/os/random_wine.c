#include <windows.h>

#include "my_cpuid.h"
#include "debug.h"
#include "build_info.h"

NTSYSAPI ULONG WINAPI NtGetTickCount(VOID);
NTSYSAPI ULONG NTAPI RtlRandom(ULONG *seed);

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
