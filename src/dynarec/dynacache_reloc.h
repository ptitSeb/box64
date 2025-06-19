#ifndef __DYNACACHE_RELOC_H__
#define __DYNACACHE_RELOC_H__

#ifdef ARM64
#include "dynarec/arm64/dynarec_arm64_consts.h"
#define native_consts_t             arm64_consts_t
#else
typedef enum native_consts_s {
    const_none,
    const_last
} native_consts_t;
#endif

void AddRelocTable64Const(dynarec_native_t* dyn, int ninst, native_consts_t C, int pass);
void AddRelocTable64Addr(dynarec_native_t* dyn, int ninst, uintptr_t addr, int pass);
void AddRelocTable64RetEndBlock(dynarec_native_t* dyn, int ninst, uintptr_t addr, int pass);
void AddRelocTable64JmpTbl(dynarec_native_t* dyn, int ninst, uintptr_t addr, int pass);
void AddRelocCancelBlock(dynarec_native_t* dyn, int ninst, int pass);

int ApplyRelocs(dynablock_t* block, intptr_t delta_block, intptr_t delta_map, uintptr_t mapping_start);
int RelocsHaveCancel(dynablock_t* block);
uintptr_t RelocGetNext();

#endif