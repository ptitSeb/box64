#ifndef __SYSINFO_H_
#define __SYSINFO_H_
#include <stdio.h>
#include <stdint.h>

typedef struct x64emu_s x64emu_t;
typedef struct sysinfo_s {
    uint64_t frequency;
    uint64_t ncpu;
    uint64_t bogomips;
    char* cpuname;

    uint64_t box64_ncpu;
    char* box64_cpuname;

    uint32_t emulated_frequency : 1;
    uint32_t read_frequency : 1;
    uint32_t read_ncpu : 1;
    uint32_t read_bogomips : 1;
    uint32_t read_cpuname : 1;
} sysinfo_t;

void InitializeSystemInfo(void);
uint32_t helper_getcpu(x64emu_t* emu);
#endif //__SYSINFO_H_