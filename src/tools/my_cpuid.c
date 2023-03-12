#define _GNU_SOURCE 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sched.h>

#include "my_cpuid.h"
#include "../emu/x64emu_private.h"
#include "debug.h"

int get_cpuMhz()
{
	int MHz = 0;
#ifdef __arm__
	FILE *f = fopen("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq", "r");
	if(f) {
		int r;
		if(1==fscanf(f, "%d", &r))
			MHz = r/1000;
		fclose(f);
	}
#endif
	if(!MHz)
		MHz = 1000; // default to 1Ghz...
	return MHz;
}
int getNCpu();  // defined in wrappedlibc.c
const char* getCpuName();   // same

void my_cpuid(x64emu_t* emu, uint32_t tmp32u)
{
    emu->regs[_AX].dword[1] = emu->regs[_DX].dword[1] = emu->regs[_CX].dword[1] = emu->regs[_BX].dword[1] = 0;
    int ncpu = getNCpu();
    static char branding[3*4*4+1] = "";
    static int done = 0;
    if(!done) {
        done = 1;
        const char* name = getCpuName();
        if(strstr(name, "MHz") || strstr(name, "GHz")) {
            // name already have the speed in it
            snprintf(branding, 3*4*4, "Box64 on %s", name);
        } else {
            int MHz = get_cpuMhz();
            if(MHz>15000) { // swiches to GHz display...
                snprintf(branding, 3*4*4, "Box64 on %s @%1.2f GHz", name, MHz/1000.);
            } else {
                snprintf(branding, 3*4*4, "Box64 on %s @%04d MHz", name, MHz);
            }
        }
        while(strlen(branding)<3*4*4) {
            memmove(branding+1, branding, strlen(branding));
            branding[0] = ' ';
        }
    }
    if(ncpu>255) ncpu = 255;
    if(!ncpu) ncpu = 1;
    switch(tmp32u) {
        case 0x0:
            // emulate a P4. TODO: Emulate a Core2?
            R_EAX = 0x0000000D;//0x80000004;
            // return GenuineIntel
            R_EBX = 0x756E6547;
            R_EDX = 0x49656E69;
            R_ECX = 0x6C65746E;
            break;
        case 0x1:
            R_EAX = 0x00000601; // family and all
            R_EBX = 0 | (8<<0x8) | (ncpu<<16);          // Brand index, CLFlush (8), Max APIC ID (16-23), Local APIC ID (24-31)
            {
                int cpu = sched_getcpu();
                if(cpu<0) cpu=0;
                R_EAX |= cpu<<24;
            }
            R_EDX =   1         // fpu 
                    | 1<<4      // rdtsc
                    | 1<<8      // cmpxchg8
                    | 1<<11     // sep (sysenter & sysexit)
                    | 1<<15     // cmov
                    | 1<<19     // clflush (seems to be with SSE2)
                    | 1<<23     // mmx
                    | 1<<24     // fxsr (fxsave, fxrestore)
                    | 1<<25     // SSE
                    | 1<<26     // SSE2
                    ;
            R_ECX =   1<<0      // SSE3
                    | 1<<1      // PCLMULQDQ
                    | 1<<9      // SSSE3
                    | 1<<12     // fma
                    | 1<<13     // cx16 (cmpxchg16)
                    | 1<<19     // SSE4_1
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
            if(R_ECX==1)    R_EAX = 0; // Bit 5 is avx512_bf16
            else R_EAX = R_ECX = R_EBX = R_EDX = 0; // TODO
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
            
        case 0x80000000:        // max extended
            R_EAX = 0x80000005;
            break;
        case 0x80000001:        //Extended Processor Signature and Feature Bits
            R_EAX = 0;  // reserved
            R_EBX = 0;  // reserved
            R_ECX = (1<<5) | (1<<8); // LZCNT | PREFETCHW
            R_EDX = 1 | (1<<29); // x87 FPU? bit 29 is 64bits available
            //AMD flags?
            //R_EDX = 1 | (1<<8) | (1<<11) | (1<<15) | (1<<23) | (1<<29); // fpu+cmov+cx8+syscall+mmx+lm (mmxext=22, 3dnow=31, 3dnowext=30)
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
        default:
            printf_log(LOG_INFO, "Warning, CPUID command %X unsupported (ECX=%08x)\n", tmp32u, R_ECX);
            R_EAX = 0;
            R_EBX = 0;
            R_ECX = 0;
            R_EDX = 0;
    }   
}
