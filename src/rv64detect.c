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
typedef void(*vFii_t)(int, int);
static void detect_sigill(int sig)
{
    siglongjmp(sigbuf, 1);
}

static int Check(void* block)
{
    // Clear instruction cache
    __clear_cache(block, block+box64_pagesize);
    // Setup SIGILL signal handler
    signal(SIGILL, detect_sigill);
    if(sigsetjmp(sigbuf, 1)) {
        // didn't work, extension not present
        signal(SIGILL, SIG_DFL);
        return 0;
    }
    ((vFii_t)block)(0, 1);
    // done...
    signal(SIGILL, SIG_DFL);
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

    // Finish
    // Free the memory my_block
    munmap(my_block, box64_pagesize);
}