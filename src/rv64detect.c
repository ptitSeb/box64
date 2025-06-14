#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <sys/mman.h>
#include <setjmp.h>

#include "debug.h"
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

void RV64_Detect_Function()
{
    // Alloc memory to execute stuffs
    void* my_block = mmap(NULL, box64_pagesize, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
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
