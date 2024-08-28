#include <stdint.h>

#include "arm64_emitter.h"

#define EMIT(A)       \
    do {              \
        *block = (A); \
        ++block;      \
    } while (0)
void CreateJmpNext(void* addr, void* next)
{
    uint32_t* block = (uint32_t*)addr;
    LDRx_literal(x2, (intptr_t)next - (intptr_t)addr);
    BR(x2);
}
