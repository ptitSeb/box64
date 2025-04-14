#include <windows.h>

#include "my_cpuid.h"

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
    // FIXME
    return 0;
}

uint64_t get_random64(void)
{
    // FIXME
    return 0;
}