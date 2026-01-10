#include <string.h>

#include "auxval.h"
#include "debug.h"


#ifdef ARM64
#include <linux/auxvec.h>
#include <asm/hwcap.h>
#endif

#ifdef RV64
#include <setjmp.h>
#include <signal.h>
#include <sys/mman.h>
#include "dynarec/rv64/rv64_emitter.h"


// Detect RV64 extensions, by executing on of the opcode with a SIGILL signal handler

static sigjmp_buf sigbuf = {0};
typedef int (*iFiip_t)(int, int, void*);
static void detect_sigill(int sig)
{
    siglongjmp(sigbuf, 1);
}

static int Check(void* block)
{
    static uint64_t buf[2] = {0};
    // Clear instruction cache
    __clear_cache(block, block+box64_pagesize);
    // Setup SIGILL signal handler
    __sighandler_t old = signal(SIGILL, detect_sigill);
    if(sigsetjmp(sigbuf, 1)) {
        // didn't work, extension not present
        signal(SIGILL, old);
        return 0;
    }
    int result = ((iFiip_t)block)(0, 1, buf);
    if (result != 42) {
        // wrong result, extension not present
        signal(SIGILL, old);
        return 0;
    }
    // done...
    signal(SIGILL, old);
    return 1;
}

void rv64Detect(void)
{
    // Alloc memory to execute stuffs
    void* my_block = mmap(NULL, box64_pagesize, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if(my_block==(void*)-1) {
        return;
    }
    uint32_t* block;
    #define EMIT(A) *block = (A); ++block


    // THead vendor extensions
    block = (uint32_t*)my_block;
    ADDI(A0, xZR, 40);
    ADDI(A1, xZR, 1);
    TH_ADDSL(A0, A0, A1, 1);
    BR(xRA);
    cpuext.xtheadba
        = cpuext.xtheadbb
        = cpuext.xtheadbs
        = cpuext.xtheadcondmov
        = cpuext.xtheadmemidx
        = cpuext.xtheadmempair
        = cpuext.xtheadfmemidx
        = cpuext.xtheadmac
        = cpuext.xtheadfmv = Check(my_block);

    // Official extensions

    if (!cpuext.xtheadba) {
        // Test Zba with ADDUW
        block = (uint32_t*)my_block;
        ADDUW(A0, A0, A1);
        ADDI(A0, xZR, 42);
        BR(xRA);
        cpuext.zba = Check(my_block);
        // Test Zbb with ANDN
        block = (uint32_t*)my_block;
        ANDN(A0, A0, A1);
        ADDI(A0, xZR, 42);
        BR(xRA);
        cpuext.zbb = Check(my_block);
        // Test Zbc with CLMUL
        block = (uint32_t*)my_block;
        CLMUL(A0, A0, A1);
        ADDI(A0, xZR, 42);
        BR(xRA);
        cpuext.zbc = Check(my_block);
        // Test Zbs with BCLR
        block = (uint32_t*)my_block;
        BCLR(A0, A0, A1);
        ADDI(A0, xZR, 42);
        BR(xRA);
        cpuext.zbs = Check(my_block);
    }

    block = (uint32_t*)my_block;
    CSRRS(xZR, xZR, 0xc22 /* vlenb */);
    ADDI(A0, xZR, 42);
    BR(xRA);
    cpuext.vector = Check(my_block);

    if (cpuext.vector) {
        block = (uint32_t*)my_block;
        CSRRS(xZR, xZR, 0x00f /* vcsr */); // vcsr does not exists in xtheadvector
        ADDI(A0, xZR, 42);
        BR(xRA);
        cpuext.xtheadvector = !Check(my_block);
    }

    if (cpuext.vector) {
        int vlenb = 0;
        asm volatile("csrr %0, 0xc22" : "=r"(vlenb));
        cpuext.vlen = vlenb;
        if (vlenb < 16) {
            // we need vlen >= 128
            cpuext.vector = 0;
        }
    }

    // Finish
    // Free the memory my_block
    munmap(my_block, box64_pagesize);
}
#endif

#ifdef DYNAREC
int DetectHostCpuFeatures(void)
{
#ifdef ARM64
    unsigned long hwcap = real_getauxval(AT_HWCAP);
    if(!hwcap)
        hwcap = HWCAP_ASIMD;
    // first, check all needed extensions, lif half, edsp and fastmult
    if ((hwcap & HWCAP_ASIMD) == 0) return 0;
    if(hwcap&HWCAP_CRC32)
        cpuext.crc32 = 1;
    if(hwcap&HWCAP_PMULL)
        cpuext.pmull = 1;
    if(hwcap&HWCAP_AES)
        cpuext.aes = 1;
    if(hwcap&HWCAP_ATOMICS)
        cpuext.atomics = 1;
    #ifdef HWCAP_SHA1
    if(hwcap&HWCAP_SHA1)
        cpuext.sha1 = 1;
    #endif
    #ifdef HWCAP_SHA2
    if(hwcap&HWCAP_SHA2)
        cpuext.sha2 = 1;
    #endif
    #ifdef HWCAP_USCAT
    if(hwcap&HWCAP_USCAT)
        cpuext.uscat = 1;
    #endif
    #ifdef HWCAP_FLAGM
    if(hwcap&HWCAP_FLAGM)
        cpuext.flagm = 1;
    #endif
    unsigned long hwcap2 = real_getauxval(AT_HWCAP2);
    #ifdef HWCAP2_FLAGM2
    if(hwcap2&HWCAP2_FLAGM2)
        cpuext.flagm2 = 1;
    #endif
    #ifdef HWCAP2_FRINT
    if(hwcap2&HWCAP2_FRINT)
        cpuext.frintts = 1;
    #endif
    #ifdef HWCAP2_AFP
    if(hwcap2&HWCAP2_AFP)
        cpuext.afp = 1;
    #endif
    #ifdef HWCAP2_RNG
    if(hwcap2&HWCAP2_RNG)
        cpuext.rndr = 1;
    #endif
#elif defined(LA64)
    char* p = GetEnv("BOX64_DYNAREC_LA64NOEXT");
    if (p == NULL || p[0] != '1') {
        uint32_t cpucfg2 = 0, idx = 2;
        asm volatile("cpucfg %0, %1" : "=r"(cpucfg2) : "r"(idx));
        if (!((cpucfg2 >> 6) & 0b1)) return 0; // LSX must present

        cpuext.lasx = (cpucfg2 >> 7) & 0b1;
        cpuext.lbt = (cpucfg2 >> 18) & 0b1;
        cpuext.frecipe = (cpucfg2 >> 25) & 0b1;
        cpuext.lam_bh = (cpucfg2 >> 27) & 0b1;
        cpuext.lamcas = (cpucfg2 >> 28) & 0b1;
        cpuext.scq = (cpucfg2 >> 30) & 0b1;
        if (p) {
            p = strtok(p, ",");
            while (p) {
                if (!strcasecmp(p, "lasx")) cpuext.lasx = 0;
                if (!strcasecmp(p, "lbt")) cpuext.lbt = 0;
                if (!strcasecmp(p, "frecipe")) cpuext.frecipe = 0;
                if (!strcasecmp(p, "lam_bh")) cpuext.lam_bh = 0;
                if (!strcasecmp(p, "lamcas")) cpuext.lamcas = 0;
                if (!strcasecmp(p, "scq")) cpuext.scq = 0;
                p = strtok(NULL, ",");
            }
        }
    }
#elif defined(RV64)
    // private env. variable for the developer ;)
    char *p = GetEnv("BOX64_DYNAREC_RV64NOEXT");
    if(p == NULL || strcasecmp(p, "1")) {
        rv64Detect();
        if (p) {
            p = strtok(p, ",");
            while (p) {
                if (!strcasecmp(p, "zba")) cpuext.zba = 0;
                if (!strcasecmp(p, "zbb")) cpuext.zbb = 0;
                if (!strcasecmp(p, "zbc")) cpuext.zbc = 0;
                if (!strcasecmp(p, "zbs")) cpuext.zbs = 0;
                if (!strcasecmp(p, "vector")) {
                    cpuext.vector = 0;
                    cpuext.xtheadvector = 0;
                }
                if (!strcasecmp(p, "xtheadba")) cpuext.xtheadba = 0;
                if (!strcasecmp(p, "xtheadbb")) cpuext.xtheadbb = 0;
                if (!strcasecmp(p, "xtheadbs")) cpuext.xtheadbs = 0;
                if (!strcasecmp(p, "xtheadmemidx")) cpuext.xtheadmemidx = 0;
                // if (!strcasecmp(p, "xtheadfmemidx")) cpuext.xtheadfmemidx = 0;
                // if (!strcasecmp(p, "xtheadmac")) cpuext.xtheadmac = 0;
                // if (!strcasecmp(p, "xtheadfmv")) cpuext.xtheadfmv = 0;
                if (!strcasecmp(p, "xtheadmempair")) cpuext.xtheadmempair = 0;
                if (!strcasecmp(p, "xtheadcondmov")) cpuext.xtheadcondmov = 0;
                p = strtok(NULL, ",");
            }
        }
    }
#endif
    return 1;
}
#endif