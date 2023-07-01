#include <stdint.h>

#include "arm64_emitter.h"

#define EMIT(A) *block = (A); ++block
void CreateJmpNext(void* addr, void* next)
{
    uint32_t* block = (uint32_t*)addr;
    LDRx_literal(x2, (intptr_t)next - (intptr_t)addr);
    BR(x2);
}

int CreateEnter(void* addr, void* enter)
{
    if(enter) {
        uint32_t* block = (uint32_t*)addr;
        ADR_S20(x2, (intptr_t)enter - (intptr_t)addr);
        LDAXRw(x1, x2);
        ADDw_U12(x1, x1, 1);
        STLXRw(x3, x1, x2);
        CBNZw(x3, 4-4*4);
    }
    return 4*5;
}