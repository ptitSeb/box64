#ifndef __DYNAREC_RV64_FUNCTIONS_H__
#define __DYNAREC_RV64_FUNCTIONS_H__

#include "../dynarec_native_functions.h"

typedef struct x64emu_s x64emu_t;
typedef struct dynarec_rv64_s dynarec_rv64_t;

#define SCRATCH0    2

// Get an FPU scratch reg
int fpu_get_scratch(dynarec_rv64_t* dyn);
// Reset scratch regs counter
void fpu_reset_scratch(dynarec_rv64_t* dyn);
// Get an x87 double reg
int fpu_get_reg_x87(dynarec_rv64_t* dyn, int t, int n);
// Get an MMX double reg
int fpu_get_reg_emm(dynarec_rv64_t* dyn, int emm);
// Get an XMM quad reg
int fpu_get_reg_xmm(dynarec_rv64_t* dyn, int t, int xmm);
// Free a FPU/MMX/XMM reg
void fpu_free_reg(dynarec_rv64_t* dyn, int reg);
// Reset fpu regs counter
void fpu_reset_reg(dynarec_rv64_t* dyn);

// ---- Neon cache functions
// Get type for STx
int extcache_get_st(dynarec_rv64_t* dyn, int ninst, int a);
// Get if STx is FLOAT or DOUBLE
int extcache_get_st_f(dynarec_rv64_t* dyn, int ninst, int a);
// Get if STx is FLOAT or I64
int extcache_get_st_f_i64(dynarec_rv64_t* dyn, int ninst, int a);
// Get actual type for STx
int extcache_get_current_st(dynarec_rv64_t* dyn, int ninst, int a);
// Get actual STx is FLOAT or DOUBLE
int extcache_get_current_st_f(dynarec_rv64_t* dyn, int a);
// Get actual STx is FLOAT or I64
int extcache_get_current_st_f_i64(dynarec_rv64_t* dyn, int a);
// Back-propagate a change float->double
void extcache_promote_double(dynarec_rv64_t* dyn, int ninst, int a);
// Combine and propagate if needed (pass 1 only)
int extcache_combine_st(dynarec_rv64_t* dyn, int ninst, int a, int b);  // with stack current dyn->n_stack*
// Do not allow i64 type
int extcache_no_i64(dynarec_rv64_t* dyn, int ninst, int st, int a);

// FPU Cache transformation (for loops) // Specific, need to be written par backend
int fpuCacheNeedsTransform(dynarec_rv64_t* dyn, int ninst);

// Undo the changes of a extcache to get the status before the instruction
void extcacheUnwind(extcache_t* cache);

const char* getCacheName(int t, int n);

void inst_name_pass3(dynarec_native_t* dyn, int ninst, const char* name, rex_t rex);
void print_opcode(dynarec_native_t* dyn, int ninst, uint32_t opcode);
void print_newinst(dynarec_native_t* dyn, int ninst);

// reset the cache
void fpu_reset(dynarec_native_t* dyn);
void fpu_reset_ninst(dynarec_native_t* dyn, int ninst);

// is st freed
int fpu_is_st_freed(dynarec_native_t* dyn, int ninst, int st);
#endif //__DYNAREC_RV64_FUNCTIONS_H__
