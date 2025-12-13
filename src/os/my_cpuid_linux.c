#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sched.h>

#include "my_cpuid.h"
#include "build_info.h"
#include "debug.h"
#include "x64emu.h"

int get_cpuMhz()
{
    int MHz = 0;
    char *p = NULL;
    if((p=getenv("BOX64_CPUMHZ"))) {
        MHz = atoi(p);
        return MHz;
    }
    char cpumhz[200];
    sprintf(cpumhz, "%d", MHz?:1000);
    setenv("BOX64_CPUMHZ", cpumhz, 1);  // set temp value incase box64 gets recursively called

    int cpucore = 0;
    while(cpucore!=-1) {
        char cpufreq[4096];
        sprintf(cpufreq, "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_max_freq", cpucore);
        FILE *f = fopen(cpufreq, "r");
        if(f) {
            int r;
            if(1==fscanf(f, "%d", &r)) {
                r /= 1000;
                if(MHz<r)
                    MHz = r;
            }
            fclose(f);
            ++cpucore;
        }
        else
            cpucore = -1;
    }
    // try with /procs/cpuinfo
    if (!MHz) {
        FILE* f = fopen("/proc/cpuinfo", "r");
        if (f) {
            char line[256];
            while (fgets(line, sizeof(line), f)) {
                if (strcasestr(line, "cpu mhz") != NULL) {
                    char* colon = strchr(line, ':');
                    if (colon != NULL) {
                        float mhz;
                        if (sscanf(colon + 1, "%f", &mhz) == 1) {
                            MHz = (int)mhz;
                            break;
                        }
                    }
                }
            }
            fclose(f);
        }
    }
#ifndef STATICBUILD
    if(!MHz) {
        // try with lscpu, grabbing the max frequency
        FILE* f = popen("lscpu | grep \"CPU max MHz:\" | sed -r 's/CPU max MHz:\\s{1,}//g'", "r");
        if(f) {
            char tmp[200] = "";
            ssize_t s = fread(tmp, 1, 200, f);
            pclose(f);
            if(s>0) {
                // worked! (unless it's saying "lscpu: command not found" or something like that)
                if(!strstr(tmp, "lscpu")) {
                    // trim ending
                    while(strlen(tmp) && tmp[strlen(tmp)-1]=='\n')
                        tmp[strlen(tmp)-1] = 0;
                    // incase multiple cpu type are present, there will be multiple lines
                    while(strchr(tmp, '\n'))
                        *strchr(tmp,'\n') = ' ';
                    // cut the float part (so '.' or ','), it's not needed
                    if(strchr(tmp, '.'))
                        *strchr(tmp, '.')= '\0';
                    if(strchr(tmp, ','))
                        *strchr(tmp, ',')= '\0';
                    int mhz;
                    if(sscanf(tmp, "%d", &mhz)==1)
                        MHz = mhz;
                }
            }
        }
    }
    #endif
    if(!MHz)
        MHz = 1000; // default to 1Ghz...
    sprintf(cpumhz, "%d", MHz);
    setenv("BOX64_CPUMHZ", cpumhz, 1);  // set actual value
    return MHz;
}
static int nCPU = 0;
static double bogoMips = 100.;
static int read_ncpu = 0;

void grabNCpu() {
    nCPU = 1;  // default number of CPU to 1
    FILE *f = fopen("/proc/cpuinfo", "r");
    ssize_t dummy;
    if(f) {
        nCPU = 0;
        int bogo = 0;
        size_t len = 500;
        char* line = malloc(len);
        while ((dummy = getline(&line, &len, f)) != (ssize_t)-1) {
            if(!strncmp(line, "processor\t", strlen("processor\t")))
                ++nCPU;
            if(!bogo && !strncmp(line, "BogoMIPS\t", strlen("BogoMIPS\t"))) {
                // grab 1st BogoMIPS
                float tmp;
                if(sscanf(line, "BogoMIPS\t: %g", &tmp)==1) {
                    bogoMips = tmp;
                    bogo = 1;
                }
            }
        }
        free(line);
        fclose(f);
        if(!nCPU) nCPU=1;
    }
}
int getNCpu()
{
    if(!nCPU)
        grabNCpu();
    read_ncpu = 1;
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

double getBogoMips()
{
    if(!nCPU)
        grabNCpu();
    return bogoMips;
}

const char* getCpuName()
{
    static char name[200] = "Unknown CPU";
    static int done = 0;
    if(done)
        return name;
    done = 1;
    char *p = NULL;
    if((p=getenv("BOX64_CPUNAME"))) {
        strcpy(name, p);
        return name;
    }
    setenv("BOX64_CPUNAME", name, 1);   // temporary set
    #ifndef STATICBUILD
    FILE* f = popen("LC_ALL=C lscpu | grep -i \"model name:\" | head -n 1 | sed -r 's/(model name:)\\s{1,}//gi'", "r");
    if(f) {
        char tmp[200] = "";
        ssize_t s = fread(tmp, 1, 200, f);
        pclose(f);
        if(s>0) {
            // worked! (unless it's saying "lscpu: command not found" or something like that)
            if(!strstr(tmp, "lscpu")) {
                // trim ending
                while(strlen(tmp) && tmp[strlen(tmp)-1]=='\n')
                    tmp[strlen(tmp)-1] = 0;
                strncpy(name, tmp, 199);
            }
            setenv("BOX64_CPUNAME", name, 1);
            return name;
        }
    }
    // failed, try to get architecture at least
    f = popen("uname -m", "r");
    if(f) {
        char tmp[200] = "";
        ssize_t s = fread(tmp, 1, 200, f);
        pclose(f);
        if(s>0) {
            // worked!
            // trim ending
            while(strlen(tmp) && tmp[strlen(tmp)-1]=='\n')
                tmp[strlen(tmp)-1] = 0;
            snprintf(name, 199, "unknown %s cpu", tmp);
            setenv("BOX64_CPUNAME", name, 1);
            return name;
        }
    }
    #endif
    // Nope, bye
    return name;
}

const char* getBoxCpuName()
{
    static char branding[3*4*4+1] = "";
    static int done = 0;
    if(!done) {
        done = 1;
        const char* name = getCpuName();
        if(strstr(name, "MHz") || strstr(name, "GHz")) {
            // name already have the speed in it
            snprintf(branding, sizeof(branding), BOX64_BUILD_INFO_STRING_SHORT " on %.*s", 39, name);
        } else {
            unsigned int MHz = get_cpuMhz();
            if(MHz>1500) { // swiches to GHz display...
                snprintf(branding, sizeof(branding), BOX64_BUILD_INFO_STRING_SHORT " on %.*s @%1.2f GHz", 28, name, MHz / 1000.);
            } else {
                snprintf(branding, sizeof(branding), BOX64_BUILD_INFO_STRING_SHORT " on %.*s @%04d MHz", 28, name, MHz);
            }
        }
    }
    return branding;
}

uint32_t helper_getcpu(x64emu_t* emu) {
    #if defined(__GLIBC__) && defined(__GLIBC_MINOR__) && !defined(ANDROID)
    #if __GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ > 28)
    uint32_t cpu, node;
    if(!getcpu(&cpu, &node))
        return (node&0xff)<<12 | (cpu&0xff);
    #endif
    #endif
    return 0;
}

uint32_t fallback_random32()
{
    return random() ^ (random()<<1);
}

uint32_t get_random32()
{
    uint32_t ret;
    FILE* f = fopen("/dev/urandom", "rb");
    if(f) {
        if(fread(&ret, sizeof(ret), 1, f)!=1)
            ret = fallback_random32();
        fclose(f);
    } else
        ret = fallback_random32();
    return ret;
}
uint64_t fallback_random64()
{
    return random() ^ (((uint64_t)random())<<18) ^ (((uint64_t)random())<<41);
}

uint64_t get_random64()
{
    uint64_t ret;
    FILE* f = fopen("/dev/urandom", "rb");
    if(f) {
        if(fread(&ret, sizeof(ret), 1, f)!=1)
            ret = fallback_random64();
        fclose(f);
    } else
        ret = fallback_random64();
    return ret;
}
