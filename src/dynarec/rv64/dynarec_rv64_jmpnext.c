#include <stdint.h>

#include "rv64_emitter.h"

#define EMIT(A) *block = (A); ++block
void CreateJmpNext(void* addr, void* next)
{
    uint32_t* block = (uint32_t*)addr;
    uintptr_t diff = (intptr_t)next - (intptr_t)addr;
    AUIPC(x2, SPLIT20(diff));
    #if 1
    LD(x2, x2, SPLIT12(diff));
    #else
    // Probably not usefull, but keeping the code, just in case
    ADDI(x2, x2, SPLIT12(diff));
    LR_D(x2, x2, 1, 1);
    #endif
    BR(x2);
}

void CreateEnter(void* addr, void* enter)
{
    if(enter) {
        uint32_t* block = (uint32_t*)addr;
        uintptr_t diff = (intptr_t)enter - (intptr_t)addr;
        ADDI(x1, xZR, 1);
        AUIPC(x2, SPLIT20(diff));
        ADDI(x2, x2, SPLIT12(diff));
        AMOADD_W(x3, x1, x2, 1, 1);
    }
    return 4*4;
}