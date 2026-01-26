#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#include "freq.h"
#include "debug.h"

#if defined(RV64)
static size_t readBinarySizeFromFile(const char* fname)
{
    if (access(fname, R_OK) != 0) return -1;
    FILE* fp = fopen(fname, "r");
    if (fp == NULL) return -1;

    char b[sizeof(uint64_t)] = { 0 }, tmp;
    ssize_t n = fread(b, 1, sizeof(b), fp);
    if (n <= 0) return -1;

    for (ssize_t i = 0; i < n / 2; i++) {
        tmp = b[n - i - 1];
        b[n - i - 1] = b[i];
        b[i] = tmp;
    }
    return *(uint64_t*)b;
}

static inline uint64_t readCycleCounter()
{
    uint64_t val;
    asm volatile("rdtime %0"
                 : "=r"(val));
    return val;
}

static inline uint64_t readFreq()
{
    static size_t val = -1;

    val = readBinarySizeFromFile("/sys/firmware/devicetree/base/cpus/timebase-frequency");
    if (val != (size_t)-1) return val;

    // fallback to rdtime + sleep
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 50000000; // 50 milliseconds
    uint64_t cycles = readCycleCounter();
    nanosleep(&ts, NULL);
    // round to MHz
    val = (size_t)round(((double)(readCycleCounter() - cycles) * 20) / 1e6) * 1e6;
    return (uint64_t)val;
}
#elif defined(ARM64)
static inline uint64_t readCycleCounter()
{
    uint64_t val;
    asm volatile("mrs %0, cntvct_el0"
                 : "=r"(val));
    return val;
}
static inline uint64_t readFreq()
{
    uint64_t val;
    asm volatile("mrs %0, cntfrq_el0"
                 : "=r"(val));
    return val;
}
#elif defined(LA64)
static inline uint64_t readCycleCounter()
{
    uint64_t val;
    asm volatile("rdtime.d %0, %1"
                 : "=r"(val) : "r"(0));
    return val;
}

static inline uint64_t readFreq()
{
    static size_t val = -1;

    if (box64_sysinfo.read_frequency) return box64_sysinfo.frequency;

    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 50000000; // 50 milliseconds
    uint64_t cycles = readCycleCounter();
    nanosleep(&ts, NULL);
    // round to MHz
    val = (size_t)round(((double)(readCycleCounter() - cycles) * 20) / 1e6) * 1e6;
    return (uint64_t)val;
}
#endif

uint64_t ReadTSC(x64emu_t* emu)
{
    (void)emu;

    // Hardware counter, per architecture
#if defined(ARM64) || defined(RV64) || defined(LA64)
    if (!box64_rdtsc) return readCycleCounter();
#endif
    // fall back to gettime...
#if !defined(NOGETCLOCK)
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);
    return (uint64_t)(ts.tv_sec) * 1000000000LL + ts.tv_nsec;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)(tv.tv_sec) * 1000000 + tv.tv_usec;
#endif
}

uint64_t ReadTSCFrequency(x64emu_t* emu)
{
    (void)emu;
    // Hardware counter, per architecture
#if defined(ARM64) || defined(RV64) || defined(LA64)
    if (!box64_rdtsc) return readFreq();
#endif
    // fall back to get time
#if !defined(NOGETCLOCK)
    return 1000000000LL;
#else
    return 1000000;
#endif
}
