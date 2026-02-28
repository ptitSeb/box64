#include <stdint.h>

#include "ppc64le_emitter.h"

#define EMIT(A)       \
    do {              \
        *block = (A); \
        ++block;      \
    } while (0)
void CreateJmpNext(void* addr, void* next)
{
    uint32_t* block = (uint32_t*)addr;
    // PC-relative load of the target address stored at 'next'.
    // BCL 20,31,.+4 sets LR = address of the next instruction (addr+4).
    // We compute the offset from LR to 'next' and use LD to load it.
    //   BCL 20,31,.+4       -> LR = addr+4
    //   MFLR r12            -> r12 = addr+4
    //   LD r12, offset(r12) -> r12 = *(addr+4+offset) = *(next) = target
    //   MTCTR r12
    //   BCTR
    // Total: 5 instructions = 20 bytes, fits in JMPNEXT_SIZE (40 bytes)
    // with 8 bytes for block ptr at start and 8 bytes for target ptr at end.
    int reg = 12;   // r12 is scratch
    intptr_t offset = (intptr_t)next - ((intptr_t)addr + 4);
    BCL(20, 31, 4);
    MFLR(reg);
    LD(reg, (int16_t)offset, reg);
    MTCTR(reg);
    BCTR();
}
