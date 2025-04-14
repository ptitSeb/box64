#include "freq.h"

// TODO: box64_rdtsc?

uint64_t ReadTSC(x64emu_t* emu) {
    uint64_t val;
    asm volatile("mrs %0, cntvct_el0"
                 : "=r"(val));
    return val;
}

uint64_t ReadTSCFrequency(x64emu_t* emu) {
    uint64_t val;
    asm volatile("mrs %0, cntfrq_el0"
                 : "=r"(val));
    return val;
}