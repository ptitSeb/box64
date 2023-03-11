#include <stdint.h>

#include "rv64_emitter.h"

#define EMIT(A) *block = (A); ++block
void CreateJmpNext(void* addr, void* next)
{
    uint32_t* block = (uint32_t*)addr;
    uintptr_t diff = (intptr_t)next - (intptr_t)addr;
    AUIPC(x2, diff>>12);
    LD_I12(x2, x2, diff&0b111111111111);
    BR(x2);
}