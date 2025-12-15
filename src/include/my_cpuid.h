#ifndef __MY_CPUID_H__
#define __MY_CPUID_H__
#include <stdint.h>
typedef struct x64emu_s x64emu_t;

void my_cpuid(x64emu_t* emu, uint32_t tmp32u);
uint32_t helper_getcpu(x64emu_t* emu);  // get the numa/cpu id actually running
uint32_t get_random32();
uint64_t get_random64();
int get_cpuMhz();
int getNCpu();
int getNCpuUnmasked();
int canNCpuBeChanged();
const char* getBoxCpuName();
#endif //__MY_CPUID_H__
