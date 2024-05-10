#define _GNU_SOURCE 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sched.h>

#include "my_cpuid.h"
#include "../emu/x64emu_private.h"
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
    if(box64_maxcpu && nCPU>box64_maxcpu)
        return box64_maxcpu;
    return nCPU;
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
    FILE* f = popen("lscpu | grep \"Model name:\" | sed -r 's/Model name:\\s{1,}//g'", "r");
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
                strncpy(name, tmp, 199);
            }
            setenv("BOX64_CPUNAME", name, 1);
            return name;
        }
    }
    // failled, try to get architecture at least
    f = popen("lscpu | grep \"Architecture:\" | sed -r 's/Architecture:\\s{1,}//g'", "r");
    if(f) {
        char tmp[200] = "";
        ssize_t s = fread(tmp, 1, 200, f);
        pclose(f);
        if(s>0) {
            // worked!
            // trim ending
            while(strlen(tmp) && tmp[strlen(tmp)-1]=='\n')
                tmp[strlen(tmp)-1] = 0;
            // incase multiple cpu type are present, there will be multiple lines
            while(strchr(tmp, '\n'))
                *strchr(tmp,'\n') = ' ';
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
            snprintf(branding, sizeof(branding), "Box64 on %.*s", 39, name);
        } else {
            unsigned int MHz = get_cpuMhz();
            if(MHz>1500) { // swiches to GHz display...
                snprintf(branding, sizeof(branding), "Box64 on %.*s @%1.2f GHz", 28, name, MHz/1000.);
            } else {
                snprintf(branding, sizeof(branding), "Box64 on %.*s @%04d MHz", 28, name, MHz);
            }
        }
    }
    return branding;
}

void my_cpuid(x64emu_t* emu, uint32_t tmp32u)
{
    emu->regs[_AX].dword[1] = emu->regs[_DX].dword[1] = emu->regs[_CX].dword[1] = emu->regs[_BX].dword[1] = 0;
    int ncpu = getNCpu();
    if(!ncpu) ncpu = 1;
    int ncluster = 1;
    while(ncpu>64) {
        ncluster++; // do cluster of 64 cpus...
        if(ncpu>=128)
            ncpu-=64;
        else
            ncpu=64;
    }
    static char branding[3*4*4+1] = "";
    if(!branding[0]) {
        strcpy(branding, getBoxCpuName());
        while(strlen(branding)<3*4*4) {
            memmove(branding+1, branding, strlen(branding)+1);
            branding[0] = ' ';
        }
    }
    switch(tmp32u) {
        case 0x0:
            // emulate a P4. TODO: Emulate a Core2?
            R_EAX = 0x0000000f;//was 0x15 before, but something seems wrong for leaf 0x15, and cpu-z take that as pure cpu speed...
            // return GenuineIntel
            R_EBX = 0x756E6547;
            R_EDX = 0x49656E69;
            R_ECX = 0x6C65746E;
            break;
        case 0x1:
            R_EAX = 0x00000601; // family and all
            R_EBX = 0 | (8<<0x8) | (ncluster<<16);          // Brand index, CLFlush (8), Max APIC ID (16-23), Local APIC ID (24-31)
            /*{
                int cpu = sched_getcpu();
                if(cpu<0) cpu=0;
                R_EAX |= cpu<<24;
            }*/
            R_EDX =   1         // fpu 
                    | 1<<2      // debugging extension
                    | 1<<4      // rdtsc
                    | 1<<8      // cmpxchg8
                    | 1<<11     // sep (sysenter & sysexit)
                    | 1<<15     // cmov
                    | 1<<19     // clflush (seems to be with SSE2)
                    | 1<<23     // mmx
                    | 1<<24     // fxsr (fxsave, fxrestore)
                    | 1<<25     // SSE
                    | 1<<26     // SSE2
                    | 1<<28     // HT / Multi-core
                    ;
            R_ECX =   1<<0      // SSE3
                    | 1<<1      // PCLMULQDQ
                    | 1<<9      // SSSE3
                    //| 1<<12     // fma    // some games treat FMA as AVX
                    | 1<<13     // cx16 (cmpxchg16)
                    | 1<<19     // SSE4_1
                    | (box64_sse42?(1<<20):0)     // SSE4_2 can be hiden
                    | 1<<22     // MOVBE
                    | 1<<23     // POPCOUNT
                    | 1<<25     // aesni
                    ; 
            break;
        case 0x2:   // TLB and Cache info. Sending 1st gen P4 info...
            R_EAX = 0x665B5001;
            R_EBX = 0x00000000;
            R_ECX = 0x00000000;
            R_EDX = 0x007A7000;
            break;
        
        case 0x4:   // Cache info
            switch (R_ECX) {
                case 0: // L1 data cache
                    R_EAX = (1 | (1<<5) | (1<<8) | ((ncpu-1)<<26));   //type + (26-31):max cores per packages-1
                    R_EBX = (63 | (7<<22)); // size
                    R_ECX = 63;
                    R_EDX = 1;
                    break;
                case 1: // L1 inst cache
                    R_EAX = (2 | (1<<5) | (1<<8)); //type
                    R_EBX = (63 | (7<<22)); // size
                    R_ECX = 63;
                    R_EDX = 1;
                    break;
                case 2: // L2 cache
                    R_EAX = (3 | (2<<5) | (1<<8)); //type
                    R_EBX = (63 | (15<<22));    // size
                    R_ECX = 4095;
                    R_EDX = 1;
                    break;

                default:
                    R_EAX = 0x00000000;
                    R_EBX = 0x00000000;
                    R_ECX = 0x00000000;
                    R_EDX = 0x00000000;
                    break;
            }
            break;
        case 0x5:   //mwait info
            R_EAX = 0;
            R_EBX = 0;
            R_ECX = 1 | 2;
            R_EDX = 0;
            break;
        case 0x3:   // PSN
        case 0x6:   // thermal
        case 0x8:   // more extended capabilities
        case 0x9:   // direct cache access
        case 0xA:   // Architecture performance monitor
            R_EAX = 0;
            R_EBX = 0;
            R_ECX = 0;
            R_EDX = 0;
            break;
        case 0x7:   // extended bits...
            if(R_ECX==0) {
                R_EAX = 0;
                R_EBX = 
                        //1<<3 |  // BMI1 
                        //1<<8 | //BMI2
                        1<<29|  // SHA extension
                        0;
            } else {R_EAX = R_ECX = R_EBX = R_EDX = 0;}
            break;
        case 0xB:   // Extended Topology Enumeration Leaf
            //TODO!
            R_EAX = 0;
            R_EBX = 0;
            break;
        case 0xC:   //?
            R_EAX = 0;
            break;
        case 0xD:   // Processor Extended State Enumeration Main Leaf / Sub Leaf
            if(R_CX==0) {
                R_EAX = 1 | 2;  // x87 SSE saved
                R_EBX = 512;    // size of xsave/xrstor
                R_ECX = 512;    // same
                R_EDX = 0;      // more bits
            } else if(R_CX==1){
                R_EAX = R_ECX = R_EBX = R_EDX = 0;  // XSAVEOPT and co are not available
            } else {
                R_EAX = R_ECX = R_EBX = R_EDX = 0;
            }
            break;
        case 0xE:   //?
            R_EAX = 0;
            break;
        case 0xF:   //L3 Cache
            switch(R_ECX) {
                case 0: 
                    R_EAX = 0;
                    R_EBX = 0; // maximum range of RMID of physical processor
                    R_ECX = 0;
                    R_EDX = 0;  // bit 1 support L3 RDT Cache monitoring
                    break;
                case 1:
                    R_EAX = 0;
                    R_EBX = 0;  // Conversion factor
                    R_EDX = 0;  // bit 0 = occupency monitoring
                    break;
                default: R_EAX = 0;
            }
            break;
        case 0x14:  // Processor Trace Enumeration Main Leaf
            switch(R_ECX) {
                case 0: // main leaf
                    R_EAX = 0;  // max sub-leaf
                    R_EBX = 0;
                    R_ECX = 0;
                    R_EDX = 0;
                    break;
                default: R_EAX = 0;
            }
            break;
        case 0x15:  // TSC core frenquency
            R_EAX = 1;  // denominator
            R_EBX = 1;  // numerator
            {
                uint64_t freq = ReadTSCFrequency(emu);
                while(freq>100000000) {
                    freq/=10;
                    R_EAX *= 10;
                }
                R_ECX = freq;  // nominal frequency in Hz
            }
            R_EDX = 0;
            break;

        case 0x80000000:        // max extended
            R_EAX = 0x80000005; // was 0x80000007 before, but L2 cache description 0x80000006 is not correct and make some AC games to assert about l2 cache value coherency...
            break;
        case 0x80000001:        //Extended Processor Signature and Feature Bits
            R_EAX = 0;  // reserved
            R_EBX = 0;  // reserved
            R_ECX = (1<<0) | (1<<5) | (1<<8); // LAHF_LM | LZCNT | PREFETCHW
            R_EDX = 1       // x87 FPU 
                | (1<<8)    // cx8: cmpxchg8b opcode
                | (1<<11)   // syscall
                | (1<<15)   // cmov: FCMOV opcodes
                | (1<<23)   // mmx: MMX available
                | (1<<24)   // fxsave
                | (1<<27)   // rdtscp
                | (1<<29);  // long mode 64bits available
            break;
        case 0x80000002:    // Brand part 1 (branding signature)
            R_EAX = ((uint32_t*)branding)[0];
            R_EBX = ((uint32_t*)branding)[1];
            R_ECX = ((uint32_t*)branding)[2];
            R_EDX = ((uint32_t*)branding)[3];
            break;
        case 0x80000003:    // Brand part 2
            R_EAX = ((uint32_t*)branding)[4];
            R_EBX = ((uint32_t*)branding)[5];
            R_ECX = ((uint32_t*)branding)[6];
            R_EDX = ((uint32_t*)branding)[7];
            break;
        case 0x80000004:    // Brand part 3, with frequency
            R_EAX = ((uint32_t*)branding)[8];
            R_EBX = ((uint32_t*)branding)[9];
            R_ECX = ((uint32_t*)branding)[10];
            R_EDX = ((uint32_t*)branding)[11];
            break;  
        case 0x80000005:
            R_EAX = 0;
            R_EBX = 0;
            R_ECX = 0;
            R_EDX = 0;
            break;
        case 0x80000006:    // L2 cache line size and associativity
            R_EAX = 0;
            R_EBX = 0;
            R_ECX = 0;
            R_EDX = 0;
            break;
        case 0x80000007:    // Invariant TSC
            R_EAX = 0;
            R_EBX = 0;
            R_ECX = 0;
            R_EDX = 0 | (1<<8); // Invariant TSC
            break;
        default:
            printf_log(LOG_INFO, "Warning, CPUID command %X unsupported (ECX=%08x)\n", tmp32u, R_ECX);
            R_EAX = 0;
            R_EBX = 0;
            R_ECX = 0;
            R_EDX = 0;
    }   
}
