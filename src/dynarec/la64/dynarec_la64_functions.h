#ifndef __DYNAREC_LA64_FUNCTIONS_H__
#define __DYNAREC_LA64_FUNCTIONS_H__

#include "../dynarec_native_functions.h"

#define SCRATCH0 24

typedef struct x64emu_s x64emu_t;
typedef struct dynarec_la64_s dynarec_la64_t;

// Get an FPU scratch reg
int fpu_get_scratch(dynarec_la64_t* dyn);
// Reset scratch regs counter
void fpu_reset_scratch(dynarec_la64_t* dyn);
// Get an XMM quad reg
int fpu_get_reg_xmm(dynarec_la64_t* dyn, int t, int xmm);
// Free a FPU/MMX/XMM reg
void fpu_free_reg(dynarec_la64_t* dyn, int reg);
// Reset fpu regs counter
void fpu_reset_reg(dynarec_la64_t* dyn);
// Get an MMX double reg
int fpu_get_reg_emm(dynarec_la64_t* dyn, int emm);

// FPU Cache transformation (for loops) // Specific, need to be written by backend
int fpuCacheNeedsTransform(dynarec_la64_t* dyn, int ninst);

// Undo the changes of a lsxcache to get the status before the instruction
void lsxcacheUnwind(lsxcache_t* cache);
void fpu_save_and_unwind(dynarec_la64_t* dyn, int ninst, lsxcache_t* cache);
void fpu_unwind_restore(dynarec_la64_t* dyn, int ninst, lsxcache_t* cache);

const char* getCacheName(int t, int n);

void inst_name_pass3(dynarec_native_t* dyn, int ninst, const char* name, rex_t rex);
void print_opcode(dynarec_native_t* dyn, int ninst, uint32_t opcode);

// reset the cache
void fpu_reset(dynarec_native_t* dyn);
void fpu_reset_ninst(dynarec_native_t* dyn, int ninst);

void updateNativeFlags(dynarec_la64_t* dyn);
void get_free_scratch(dynarec_la64_t* dyn, int ninst, uint8_t* tmp1, uint8_t* tmp2, uint8_t* tmp3, uint8_t s1, uint8_t s2, uint8_t s3, uint8_t s4, uint8_t s5);
#endif //__DYNAREC_LA64_FUNCTIONS_H__
