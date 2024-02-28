#include <stdint.h>

#include "la64_emitter.h"

#define EMIT(A) *block = (A); ++block;
void CreateJmpNext(void* addr, void* next)
{
    uint32_t* block = (uint32_t*)addr;
    uintptr_t diff = (intptr_t)next - (intptr_t)addr;
    PCADDU12I(x2, SPLIT20(diff));
    LD_D(x2, x2, SPLIT12(diff));
    BR(x2);
}