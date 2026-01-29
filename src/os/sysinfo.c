#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <sched.h>

#include "debug.h"
#include "sysinfo.h"
#include "build_info.h"

#ifdef _WIN32
#include <windows.h>
#include "freq.h"
#endif

#ifndef _WIN32
static void readCpuinfo(sysinfo_t* info)
{
    memset(info, 0, sizeof(sysinfo_t));
    info->emulated_frequency = 1;
    int cpucore = 0;
    while (1) {
        char cpufreq[4096] = { 0 };
        sprintf(cpufreq, "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_max_freq", cpucore);
        FILE* f = fopen(cpufreq, "r");
        if (!f) break;
        int r;
        if (1 == fscanf(f, "%d", &r)) {
            info->frequency = (uint64_t)r * 1000;
            info->emulated_frequency = 0;
            info->read_frequency = 1;
        }
        fclose(f);
        ++cpucore;
    }

    char line[4096] = { 0 };
    FILE* f = fopen("/proc/cpuinfo", "r");
    if (!f) goto lscpu;

    while (fgets(line, sizeof(line), f)) {
        if (info->ncpu <= 1 && !info->read_frequency && strstr(line, "CPU MHz") != NULL) {
            char* colon = strchr(line, ':');
            if (colon != NULL) {
                float mhz;
                if (sscanf(colon + 1, "%f", &mhz) == 1) {
                    info->frequency = (uint64_t)(mhz * 1e6);
                    info->emulated_frequency = 0;
                    info->read_frequency = 1;
                }
            }
        } else if (info->ncpu <= 1 && strstr(line, "Model Name") != NULL) { // TODO: big.LITTLE handling?
            char* colon = strchr(line, ':');
            if (colon != NULL) {
                while (colon[1] == ' ' || colon[1] == '\t')
                    ++colon;
                size_t len = strlen(colon);
                info->cpuname = (char*)malloc(len + 1);
                strcpy(info->cpuname, colon + 2);
                char* newline = strchr(info->cpuname, '\n');
                if (newline) *newline = '\0';
                info->read_cpuname = 1;
            }
        } else if (!info->read_bogomips && strstr(line, "BogoMIPS") != NULL) {
            char* colon = strchr(line, ':');
            if (colon != NULL) {
                float bm;
                if (sscanf(colon + 1, "%f", &bm) == 1) {
                    info->bogomips = (uint64_t)(bm * 1e6);
                    info->read_bogomips = 1;
                }
            }
        } else if (strstr(line, "processor") != NULL) {
            info->ncpu++;
        }
    }
    if (info->ncpu) info->read_ncpu = 1;
    fclose(f);

lscpu:
    if (getenv("BOX64_LSCPU_EXECUTED")) {
        // in case it's the x86_64 lscpu, prevent infinite loop
        goto fallback;
    }
    setenv("BOX64_LSCPU_EXECUTED", "1", 1);
    if (!info->read_cpuname || !info->read_ncpu || !info->read_frequency) {
        FILE* f = popen("lscpu", "r");
        if (!f) goto fallback;
        while (fgets(line, sizeof(line), f)) {
            if (!info->read_ncpu && strstr(line, "CPU(s):") != NULL) {
                char* colon = strchr(line, ':');
                if (colon != NULL) {
                    int ncpu;
                    if (sscanf(colon + 1, "%d", &ncpu) == 1) {
                        info->ncpu = (uint64_t)ncpu;
                        info->read_ncpu = 1;
                    }
                }
            } else if (!info->read_cpuname && strstr(line, "Model name:") != NULL) {
                char* colon = strchr(line, ':');
                if (colon != NULL) {
                    while (colon[1] == ' ' || colon[1] == '\t')
                        ++colon;
                    size_t len = strlen(colon);
                    info->cpuname = (char*)malloc(len + 1);
                    strcpy(info->cpuname, colon + 2);
                    char* newline = strchr(info->cpuname, '\n');
                    if (newline) *newline = '\0';
                    info->read_cpuname = 1;
                }
            } else if (!info->read_frequency && strstr(line, "CPU max MHz:") != NULL) {
                char* colon = strchr(line, ':');
                if (colon != NULL) {
                    float mhz;
                    if (sscanf(colon + 1, "%f", &mhz) == 1) {
                        info->frequency = (uint64_t)(mhz * 1e6);
                        info->emulated_frequency = 0;
                        info->read_frequency = 1;
                    }
                }
            }
        }
        pclose(f);
    }

fallback:
    if (!info->read_ncpu && !getenv("BOX64_NPROC_EXECUTED") /* in case it's the x86 version, prevent infinite loop */) {
        setenv("BOX64_NPROC_EXECUTED", "1", 1);
        FILE* f = popen("nproc", "r");
        if (f) {
            int ncpu;
            if (fscanf(f, "%d", &ncpu) == 1) {
                info->ncpu = (uint64_t)ncpu;
            }
            pclose(f);
        } else {
            info->ncpu = 1;
        }
    }
    if (!info->read_cpuname) {
        info->cpuname = (char*)malloc(20);
        strcpy(info->cpuname, "Unknown CPU");
    }
    if (!info->read_frequency) {
        info->frequency = 1000000000; // 1GHz default
        info->emulated_frequency = 1;
    }

    if (!info->read_bogomips) {
        info->bogomips = info->frequency;
    }

    return;
}
#endif

void InitializeSystemInfo(void)
{
#ifndef _WIN32
    sysinfo_t info = { 0 };
    readCpuinfo(&box64_sysinfo);

    char branding[3 * 4 * 4 + 1];
    if (strstr(box64_sysinfo.cpuname, "MHz") || strstr(box64_sysinfo.cpuname, "GHz")) {
        snprintf(branding, sizeof(branding), BOX64_BUILD_INFO_STRING_SHORT " on %.*s", 39, box64_sysinfo.cpuname);
    } else {
        if (box64_sysinfo.frequency > 1500000) {
            snprintf(branding, sizeof(branding), BOX64_BUILD_INFO_STRING_SHORT " on %.*s @%1.2f GHz", 28, box64_sysinfo.cpuname, box64_sysinfo.frequency / 1000000000.);
        } else {
            snprintf(branding, sizeof(branding), BOX64_BUILD_INFO_STRING_SHORT " on %.*s @%04d MHz", 28, box64_sysinfo.cpuname, box64_sysinfo.frequency / 1000000);
        }
    }
    box64_sysinfo.box64_cpuname = (char*)calloc(strlen(branding) + 1, 1);
    strcpy(box64_sysinfo.box64_cpuname, branding);

#else
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    box64_sysinfo.ncpu = sysinfo.dwNumberOfProcessors;
    box64_sysinfo.frequency = ReadTSCFrequency(NULL);
    box64_sysinfo.bogomips = box64_sysinfo.frequency;
    box64_sysinfo.cpuname = (char*)WinMalloc(64);
    strcpy(box64_sysinfo.cpuname, "Box64 Virtual CPU");
    box64_sysinfo.box64_cpuname = (char*)WinCalloc(strlen(BOX64_STR) + 1, 1);
    strcpy(box64_sysinfo.box64_cpuname, BOX64_STR);

    box64_sysinfo.emulated_frequency = 0;
#endif

    box64_sysinfo.box64_ncpu = box64_sysinfo.ncpu;
    if (BOX64ENV(maxcpu) && box64_sysinfo.ncpu > (uint64_t)BOX64ENV(maxcpu)) {
        box64_sysinfo.box64_ncpu = (uint64_t)BOX64ENV(maxcpu);
    }
}

uint32_t helper_getcpu(x64emu_t* emu)
{
#ifndef _WIN32
#if defined(__GLIBC__) && defined(__GLIBC_MINOR__) && !defined(ANDROID)
#if __GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ > 28)
    uint32_t cpu, node;
    if (!getcpu(&cpu, &node))
        return (node & 0xff) << 12 | (cpu & 0xff);
#endif
#endif
#endif
    return box64_sysinfo.ncpu;
}