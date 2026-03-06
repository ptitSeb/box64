#ifndef __DYNACACHE_RELOC_H__
#define __DYNACACHE_RELOC_H__

#ifdef ARM64
#include "dynarec/arm64/dynarec_arm64_consts.h"
#define native_consts_t arm64_consts_t
#elif defined(RV64)
#include "dynarec/rv64/dynarec_rv64_consts.h"
#define native_consts_t rv64_consts_t
#elif defined(LA64)
#include "dynarec/la64/dynarec_la64_consts.h"
#define native_consts_t la64_consts_t
#elif defined(PPC64LE)
#include "dynarec/ppc64le/dynarec_ppc64le_consts.h"
#define native_consts_t ppc64le_consts_t
#else
#error Unsupported architecture
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