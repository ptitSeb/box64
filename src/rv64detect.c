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
typedef void(*vFiip_t)(int, int, void*);
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
    ((vFiip_t)block)(0, 1, buf);
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

    // Official extensions

    // Test Zba with ADDUW
    block = (uint32_t*)my_block;
    ADDUW(A0, A0, A1);
    BR(xRA);
    rv64_zba = Check(my_block);
    // Test Zbb with ANDN
    block = (uint32_t*)my_block;
    ANDN(A0, A0, A1);
    BR(xRA);
    rv64_zbb = Check(my_block);
    // Test Zbc with CLMUL
    block = (uint32_t*)my_block;
    CLMUL(A0, A0, A1);
    BR(xRA);
    rv64_zbc = Check(my_block);
    // Test Zbs with BCLR
    block = (uint32_t*)my_block;
    BCLR(A0, A0, A1);
    BR(xRA);
    rv64_zbs = Check(my_block);

    // THead vendor extensions
    if (!rv64_zba) {
        // Test XTheadBa with TH_ADDSL
        block = (uint32_t*)my_block;
        TH_ADDSL(A0, A0, A1, 1);
        BR(xRA);
        rv64_xtheadba = Check(my_block);

        // Test XTheadBb with TH_SRRI
        block = (uint32_t*)my_block;
        TH_SRRI(A0, A1, 1);
        BR(xRA);
        rv64_xtheadbb = Check(my_block);

        // Test XTheadBs with TH_TST
        block = (uint32_t*)my_block;
        TH_TST(A0, A1, 1);
        BR(xRA);
        rv64_xtheadbs = Check(my_block);

        // Test XTheadCondMov with TH_MVEQZ
        block = (uint32_t*)my_block;
        TH_MVEQZ(A0, A0, A1);
        BR(xRA);
        rv64_xtheadcondmov = Check(my_block);

        // Test XTheadMemIdx with TH_LBIA
        block = (uint32_t*)my_block;
        TH_LBIA(A0, A2, 1, 1);
        BR(xRA);
        rv64_xtheadmemidx = Check(my_block);

        // Test XTheadMemPair with TH_LDD
        block = (uint32_t*)my_block;
        TH_LDD(A0, A1, A2, 0);
        BR(xRA);
        rv64_xtheadmempair = Check(my_block);

        // Test XTheadFMemIdx with TH_FLRD
        block = (uint32_t*)my_block;
        TH_FLRD(A0, A2, xZR, 0);
        BR(xRA);
        rv64_xtheadfmemidx = Check(my_block);

        // Test XTheadMac with TH_MULA
        block = (uint32_t*)my_block;
        TH_MULA(A0, A0, A1);
        BR(xRA);
        rv64_xtheadmac = Check(my_block);

        // Test XTheadFmv with TH_FMV_X_HW
        block = (uint32_t*)my_block;
        TH_FMV_X_HW(A0, A1);
        BR(xRA);
        rv64_xtheadfmv = Check(my_block);
    }

    // Finish
    // Free the memory my_block
    munmap(my_block, box64_pagesize);
}
