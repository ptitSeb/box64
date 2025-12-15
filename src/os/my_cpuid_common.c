#define _GNU_SOURCE
#include <string.h>
#include <sched.h>

#include "my_cpuid.h"
#include "../emu/x64emu_private.h"
#include "debug.h"
#include "freq.h"

void my_cpuid(x64emu_t* emu, uint32_t tmp32u)
{
    emu->regs[_AX].dword[1] = emu->regs[_DX].dword[1] = emu->regs[_CX].dword[1] = emu->regs[_BX].dword[1] = 0;
    int ncpu = getNCpu();
    if(!ncpu) ncpu = 1;
    int ncluster = 1;
    static int warned = 10;
    if(BOX64ENV(cputype)) {
        while(ncpu>256) {
            ncluster++; // do cluster of 256 cpus...
            if(ncpu>=256)
                ncpu-=256;
            else
                ncpu=256;
        }
    } else {
        while(ncpu>64) {
            ncluster++; // do cluster of 64 cpus...
            if(ncpu>=128)
                ncpu-=64;
            else
                ncpu=64;
        }
    }
    static char branding[3*4*4+1] = "";
    if(!branding[0]) {
        strcpy(branding, getBoxCpuName());
        while(strlen(branding)<3*4*4) {
            memmove(branding+1, branding, strlen(branding)+1);
            branding[0] = ' ';
        }
    }
    uint32_t subleaf = R_ECX;
    //printf_log(LOG_INFO, "%04d|%p: cpuid leaf=0x%x (subleaf=0x%x)", GetTID(), (void*)R_RIP, tmp32u, subleaf);
    switch(tmp32u) {
        case 0x0:
            // emulate a P4. TODO: Emulate a Core2?
            R_RAX = BOX64ENV(cputype)?0x0000000f:0x00000016;
            if(BOX64ENV(cputype)) {
                // return AuthenticAMD
                R_RBX = 0x68747541;
                R_RCX = 0x444d4163;
                R_RDX = 0x69746E65;
            } else {
                // return GenuineIntel
                R_RBX = 0x756E6547;
                R_RCX = 0x6C65746E;
                R_RDX = 0x49656E69;
            }
            break;
        case 0x1:
            if(BOX64ENV(cputype)) {
                R_RAX = (0xc<<0) | // stepping 0-3
                        (0x1<<4) | // base model 4-7
                        (0xf<<8) | // base familly 8-11
                        (0x0<<12)| // reserved 12-15
                        (0x7<<16)| // Extented model 16-19
                        (0x8<<20)| // extended familly 20-27
                        (0x0<<28)| // reserved 28-31
                        0 ; // family and all, simulating a Ryzen 5 type of cpu
            } else {
                R_RAX = (0x1<<0) | // stepping
                        (0x6<<4) | // model
                        (0x6<<8) | // familly
                        (0x0<<12)| // Processor type
                        (0x0<<14)| // reserved
                        (0x4<<16)| // extended model
                        (0x0<<20)| // extended familly
                        0 ; // family and all, simulating Haswell type of cpu
            }
            R_RBX = 0 | (8<<0x8) | ((BOX64ENV(cputype)?0:ncluster)<<16);          // Brand index, CLFlush (8), Max APIC ID (16-23), Local APIC ID (24-31)
            /*{
                int cpu = sched_getcpu();
                if(cpu<0) cpu=0;
                R_RAX |= cpu<<24;
            }*/
            R_RDX =   1         // fpu
                    | 1<<1      // vme
                    | 1<<2      // debugging extension
                    | 1<<3      // pse
                    | 1<<4      // rdtsc
                    | 1<<5      // msr
                    | 1<<6      // pae
                    | 1<<7      // mcheck extension
                    | 1<<8      // cmpxchg8
                    | 1<<11     // sep (sysenter & sysexit)
                    | 1<<12     // mtrr
                    | 1<<13     // pgb
                    | 1<<14     // mcheck arch
                    | 1<<15     // cmov
                    | 1<<16     // pat
                    | 1<<17     // pse36
                    | 1<<19     // clflush (seems to be with SSE2)
                    | 1<<21     // DS, used with VMX, is that usefull?
                    | 1<<23     // mmx
                    | 1<<24     // fxsr (fxsave, fxrestore)
                    | 1<<25     // SSE
                    | 1<<26     // SSE2
                    | (BOX64ENV(cputype)?0:1)<<28     // HT / Multi-core
                    ;
            R_RCX =   1<<0      // SSE3
                    | BOX64ENV(pclmulqdq)<<1      // PCLMULQDQ
                    | (BOX64ENV(cputype)?0:1)<<2      // DS 64bits
                    | 1<<3      // Monitor/MWait (priviledge instructions)
                    | (BOX64ENV(cputype)?0:1)<<5      // VMX  //is that usefull
                    | 1<<9      // SSSE3
                    | BOX64ENV(avx2)<<12     // fma
                    | 1<<13     // cx16 (cmpxchg16)
                    | 1<<19     // SSE4_1
                    | BOX64ENV(sse42)<<20     // SSE4_2 can be hiden
                    | 1<<22     // MOVBE
                    | 1<<23     // POPCOUNT
                    | BOX64ENV(aes)<<25     // aesni
                    | BOX64ENV(avx)<<26 // xsave
                    | BOX64ENV(avx)<<27 // osxsave
                    | BOX64ENV(avx)<<28 // AVX
                    | BOX64ENV(avx)<<29 // F16C
                    | BOX64ENV(avx2)<<30     // RDRAND
                    | 0<<31     // Hypervisor guest running
                    ;
            break;
        case 0x2:
            if(BOX64ENV(cputype)) {
                // reserved
                R_RAX = R_RBX = R_RCX = R_RDX = 0 ;
            } else {
                // TLB and Cache info. Sending 9th gen i3 info...
                R_RAX = 0x76036301;
                R_RBX = 0x00f0b6ff;
                R_RCX = 0x00000000;
                R_RDX = 0x00c30000;
            }
            break;

        case 0x4:
            if(BOX64ENV(cputype)) {
                // reserved
                R_RAX = R_RBX = R_RCX = R_RDX = 0 ;
            } else {
                // Cache info
                switch (subleaf) {
                    case 0: // L1 data cache
                        R_RAX = 0x4121 | ((ncpu-1)<<26);   //type + (26-31):max cores per packages-1
                        R_RBX = 0x01c0003f; // size
                        R_RCX = 63;
                        R_RDX = 0;
                        break;
                    case 1: // L1 inst cache
                        R_RAX = 0x4122 | ((ncpu-1)<<26); //type
                        R_RBX = 0x01c0003f; // size
                        R_RCX = 63;
                        R_RDX = 0;
                        break;
                    case 2: // L2 cache
                        R_RAX = 0x4143 | ((ncpu-1)<<26); //type
                        R_RBX = 0x00c0003f;    // size
                        R_RCX = 1023;
                        R_RDX = 0;
                        break;
                    case 3: // L3 cache
                        R_RAX = 0x3c63 | ((ncpu-1)<<26); //type
                        R_RBX = 0x02c0003f;    // size
                        R_RCX = 8191;
                        R_RDX = 6;
                        break;
                    default:
                        R_RAX = 0x00000000;
                        R_RBX = 0x00000000;
                        R_RCX = 0x00000000;
                        R_RDX = 0x00000000;
                        break;
                }
            }
            break;
        case 0x5:   //mwait info
            R_RAX = 0x40;
            R_RBX = 0x40;
            R_RCX = 1 | 2;
            R_RDX = 0x11142120;
            break;
        case 0x3:   // PSN
        case 0x6:   // thermal
        case 0x8:   // more extended capabilities
        case 0x9:   // direct cache access
        case 0xA:   // Architecture performance monitor
            R_RAX = 0;
            R_RBX = 0;
            R_RCX = 0;
            R_RDX = 0;
            break;
        case 0x7:
            // extended bits...
            if(subleaf==0) {
                R_RAX = 0;
                R_RBX = 1<<0 // FSGSBASE
                        | BOX64ENV(avx)<<3  // BMI1
                        | BOX64ENV(avx2)<<5  //AVX2
                        | (BOX64ENV(cputype)?0:1)<<6 // FDP_EXCPTN_ONLY
                        | 1<<7 // SMEP
                        | BOX64ENV(avx2)<<8 //BMI2
                        | (BOX64ENV(cputype)?0:1)<<9    // Enhanced REP MOVSB   // is it a good idea?
                        | 1<<10 //INVPCID (priviledge instruction
                        | (BOX64ENV(cputype)?0:1)<<13 // Deprecates FPU CS and FPU DS
                        | 0<<18 // RDSEED
                        | BOX64ENV(avx2)<<19 //ADX
                        | 1<<23 // CLFLUSHOPT
                        | 1<<24 // CLWB
                        | BOX64ENV(shaext)<<29 // SHA extension
                        ;
                R_RCX =
                        (BOX64ENV(avx)&&BOX64ENV(aes))<<9   | //VAES
                        (BOX64ENV(avx2)&&BOX64ENV(pclmulqdq))<<10 | //VPCLMULQDQ.
                        1<<22 | // RDPID
                        0;
                R_RDX = 0;

            } else {R_RAX = R_RCX = R_RBX = R_RDX = 0;}
            break;
        case 0xB:
            if(BOX64ENV(cputype)) {
                // reserved
                R_RAX = R_RBX = R_RCX = R_RDX = 0 ;
            } else {
                // Extended Topology Enumeration Leaf
                R_RAX = 0;
                R_RBX = (subleaf==0)?1:((subleaf==1)?ncpu:0);
                R_RCX |= (subleaf==0)?0x100:((subleaf==1)?0x200:0);
                #ifdef WIN32
                int cpu = 0;
                #else
                int cpu = sched_getcpu();
                if(cpu<0) cpu=0;
                #endif
                R_RDX = cpu;
            }
            break;
        case 0xC:
            if(BOX64ENV(cputype)) {
                // reserved
                R_RAX = R_RBX = R_RCX = R_RDX = 0;
            } else {
                //?
                R_RAX = R_RBX = R_RCX = R_RDX = 0;
            }
            break;
        case 0xD:
            // Processor Extended State Enumeration Main Leaf / Sub Leaf
            switch(subleaf) {
            case 0:
                R_RAX = 0b111;          // x87 SSE AVX saved
                R_RBX = 512+64+16*16;     // size of xsave/xrstor
                R_RCX = 512+64+16*16;     // maximum size of xsave area
                R_RDX = 0;              // more bits
                break;
            case 1:
                R_RAX = 0;      // XSAVEOPT (0) and XSAVEC (1), XGETBV with ECX=1 (2) XSAVES (3) and XFD (4) not supported yet
                R_RCX = R_RBX = R_RDX = 0;
                break;
            case 2:
                // componant 2: avx
                R_RAX = 16*16; // size of the avx block
                R_RBX = 512+64;  // offset
                R_RCX = 0;
                R_RDX = 0;
                break;
            default:
                R_RAX = R_RCX = R_RBX = R_RDX = 0;
                break;
            }
            break;
        case 0xE:   //?
            R_RAX = R_RCX = R_RBX = R_RDX = 0;
            break;
        case 0xF:
            if(BOX64ENV(cputype)) {
                // reserved
                R_RAX = R_RBX = R_RCX = R_RDX = 0 ;
            } else {
                //L3 Cache
                switch(subleaf) {
                    case 0:
                        R_RAX = 0;
                        R_RBX = 0; // maximum range of RMID of physical processor
                        R_RCX = 0;
                        R_RDX = 0;  // bit 1 support L3 RDT Cache monitoring
                        break;
                    case 1:
                        R_RAX = 0;
                        R_RCX = 0;
                        R_RBX = 0;  // Conversion factor
                        R_RDX = 0;  // bit 0 = occupency monitoring
                        break;
                    default: R_RAX = R_RBX = R_RCX = R_RDX = 0;
                }
            }
            break;
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
            R_RAX = R_RBX = R_RCX = R_RDX = 0;
            break;
        case 0x14:
            if(BOX64ENV(cputype)) {
                // reserved
                R_RAX = R_RBX = R_RCX = R_RDX = 0;
            } else {
                // Processor Trace Enumeration Main Leaf
                switch(subleaf) {
                    case 0: // main leaf
                        R_RAX = 1;  // max sub-leaf
                        R_RBX = 0xf;
                        R_RCX = 0x7;
                        R_RDX = 0;
                        break;
                    case 1:
                        R_RAX = 0x02490002;
                        R_RBX = 0x003f3fff;
                        R_RCX = 0;
                        R_RDX = 0;
                        break;
                    default: R_RAX = R_RBX = R_RCX = R_RDX = 0;
                }
            }
            break;
        case 0x15:
            if(BOX64ENV(cputype)) {
                // reserved
                R_RAX = R_RBX = R_RCX = R_RDX = 0;
            } else {
                // TSC core frenquency
                R_RAX = 0x02;  // denominator
                R_RBX = 0x120;  // numerator
                /*{
                    uint64_t freq = ReadTSCFrequency(emu);
                    while(freq>100000000) {
                        freq/=10;
                        R_RAX *= 10;
                    }
                    R_RCX = freq;  // nominal frequency in Hz
                }*/
                R_RCX = 0;
                R_RDX = 0;
            }
            break;
        case 0x16:
            if(BOX64ENV(cputype)) {
                R_RAX = R_RBX = R_RCX = R_RDX = 0;
            } else {
                R_RAX = get_cpuMhz();
                R_RBX = get_cpuMhz();
                R_RCX = 100;
                R_RDX = 0;

            }
            break;
        case 0x40000000 ... 0x400000FF:
            // the Hypervisor interface, yeah we don't do this, see also the 31bit of ECX in leaf 0x1.
            R_RAX = 0;
            R_RBX = 0;
            R_RCX = 0;
            R_RDX = 0;
            break;
        case 0x80000000:        // max extended
            if(BOX64ENV(cputype)) {
                R_RAX = 0x8000001a;
                R_RBX = 0x68747541;
                R_RCX = 0x444d4163;
                R_RDX = 0x69746E65;
            } else {
                R_RAX = 0x80000008;
                R_RBX = 0;
                R_RCX = 0;
                R_RDX = 0;
            }
            break;
        case 0x80000001:        //Extended Processor Signature and Feature Bits
            if(BOX64ENV(cputype)) {
                R_RAX = (0xc<<0) | // stepping 0-3
                        (0x1<<4) | // base model 4-7
                        (0xf<<8) | // base familly 8-11
                        (0x0<<12)| // reserved 12-15
                        (0x7<<16)| // Extented model 16-19
                        (0x8<<20)| // extended familly 20-27
                        (0x0<<28)| // reserved 28-31
                        0 ; // family and all, simulating a Ryzen 5 type of cpu
                R_RBX = 0; //0-15 brandID, 16-27 reserved, 28-31 PkgType
                R_RCX =   1<<0      // LAHF/SAHF
                        | 1<<1      // cmplegacy?
                        | 1<<2      // securevm
                        | 1<<5      // ABM (LZCNT)
                        | 1<<6      // SSE4a (extrq, instrq, movntss, movntsd)
                        //| 1<<7      // misaligned SSE
                        | 1<<8      // 3DNowPrefetch
                        //| 1<<10     // IBS
                        //| 1<<11     // XOP
                        //| 1<<16     // FMA4
                        ;
                R_RDX =   1         // fpu
                        | 1<<2      // debugging extension
                        | 1<<3      // pse
                        | 1<<4      // rdtsc
                        | 1<<5      // msr
                        | 1<<6      // pae
                        | 1<<8      // cmpxchg8
                        | 1<<11     // sep (sysenter & sysexit)
                        | 1<<12     // mtrr
                        | 1<<13     // pge
                        | 1<<15     // cmov
                        | 1<<16     // pat
                        | 1<<17     // pse36
                        | 1<<19     // clflush (seems to be with SSE2)
                        | 1<<20     // NX
                        | 1<<21     // DS, used with VMX, is that usefull?
                        | 1<<22     // MMXext
                        | 1<<23     // mmx
                        | 1<<24     // fxsr (fxsave, fxrestore)
                        //| 1<<25     // FFXSR
                        //| 1<<26     // Page1GB
                        | 1<<27     // RDTSCP
                        | 1<<29     //LM
                        //| 1<<30     //3DNowExt
                        //| 1<<31     //3DNow!
                        ;
            } else {
                R_RAX = 0;  // reserved
                R_RBX = 0;  // reserved
                R_RCX = (1<<0)  // LAHF_LM
                    | (1<<5)    // LZCNT
                    | (1<<8)   // PREFETCHW
                    ;
                R_RDX = 1       // x87 FPU
                    | (1<<8)    // cx8: cmpxchg8b opcode
                    | (1<<11)   // syscall
                    | (1<<15)   // cmov: FCMOV opcodes
                    | (1<<20)   // NX
                    | (1<<23)   // mmx: MMX available
                    | (1<<24)   // fxsave
                    | (1<<27)   // rdtscp
                    | (1<<29);  // long mode 64bits available
            }
            break;
        case 0x80000002:    // Brand part 1 (branding signature)
            R_RAX = ((uint32_t*)branding)[0];
            R_RBX = ((uint32_t*)branding)[1];
            R_RCX = ((uint32_t*)branding)[2];
            R_RDX = ((uint32_t*)branding)[3];
            break;
        case 0x80000003:    // Brand part 2
            R_RAX = ((uint32_t*)branding)[4];
            R_RBX = ((uint32_t*)branding)[5];
            R_RCX = ((uint32_t*)branding)[6];
            R_RDX = ((uint32_t*)branding)[7];
            break;
        case 0x80000004:    // Brand part 3, with frequency
            R_RAX = ((uint32_t*)branding)[8];
            R_RBX = ((uint32_t*)branding)[9];
            R_RCX = ((uint32_t*)branding)[10];
            R_RDX = ((uint32_t*)branding)[11];
            break;
        case 0x80000005:
            if(BOX64ENV(cputype)) {
                //L1 cache and TLB
                R_RAX = 0;
                R_RBX = 0;
                R_RCX = 0;
                R_RDX = 0;
            } else {
                R_RAX = 0;
                R_RBX = 0;
                R_RCX = 0;
                R_RDX = 0;
            }
            break;
        case 0x80000006:    // L2 cache line size and associativity
            if(BOX64ENV(cputype)) {
                R_RAX = 0;
                R_RBX = 0;
                R_RCX = 64 | (0x6<<12) | (256<<16); // bits: 0-7 line size, 15-12: assoc (using special encoding), 31-16: size in K    //TODO: read info from /sys/devices/system/cpu/cpuX/cache/index2
                R_RDX = 0;
            } else {
                R_RAX = 0;
                R_RBX = 0;
                R_RCX = 64 | (0x6<<12) | (256<<16); // bits: 0-7 line size, 15-12: assoc (using special encoding), 31-16: size in K    //TODO: read info from /sys/devices/system/cpu/cpuX/cache/index2
                R_RDX = 0;
            }
            break;
        case 0x80000007:
            if(BOX64ENV(cputype)) {
                // Advanced Power Management Information
                R_RAX = 0;
                R_RBX = 0;
                R_RCX = 0;
                R_RDX = 0 | (1<<8); // Invariant TSC
            } else {
                // Invariant TSC
                R_RAX = 0;
                R_RBX = 0;
                R_RCX = 0;
                R_RDX = 0 /*| (1<<8)*/; // Invariant TSC
            }
            break;
        case 0x80000008:
            if(BOX64ENV(cputype)) {
                // Address Size And Physical Core Count Information
                R_RAX = 0;  // 23-16 guest / 15-8 linear / 7-0 phys
                R_RBX = 0;  // reserved
                R_RCX = ncpu-1;
                R_RDX = 0;
            } else {
                // ?
                R_RAX = 0x0000302e;//?
                R_RBX = 0;
                R_RCX = 0;
                R_RDX = 0;
            }
            break;
        case 0x8000000a:
            if(BOX64ENV(cputype)) {
                // SVM Revision and Feature Identification
                R_RAX = 0;
                R_RBX = 0;
                R_RCX = 0;
                R_RDX = 0;
            } else {
                // ?
                R_RAX = 0;
                R_RBX = 0;
                R_RCX = 0;
                R_RDX = 0;
            }
            break;
        case 0x8000001a:
            if(BOX64ENV(cputype)) {
                // Performance Optimization Identifiers
                R_RAX =   1<<0  // FP128
                        | 1<<1  // MOVU
                        | 0;
                R_RBX = 0;
                R_RCX = 0;
                R_RDX = 0;
            } else {
                // ?
                R_RAX = 0;
                R_RBX = 0;
                R_RCX = 0;
                R_RDX = 0;
            }
            break;
        default:
            if(warned) {
                printf_log(LOG_INFO, "Warning, CPUID command %X unsupported (ECX=%08x)\n", tmp32u, R_RCX);
                --warned;
                if(!warned)
                    printf_log(LOG_INFO, "Stopped logging CPUID issues\n");
            }
            R_RAX = 0;
            R_RBX = 0;
            R_RCX = 0;
            R_RDX = 0;
    }
    //printf_log_prefix(0, LOG_INFO, " => EAX=%08x EBX=%08x ECX=%08x EDX=%08x\n", R_RAX, R_RBX, R_RCX, R_RDX);
}
