#ifndef __DYNAREC_ARM_FUNCTIONS_H__
#define __DYNAREC_ARM_FUNCTIONS_H__

#include "../dynarec_native_functions.h"

#define SCRATCH0    24
#define XMM0    0
#define XMM8    16
#define X870    8
#define EMM0    8


// Get an FPU scratch reg
int fpu_get_scratch(dynarec_arm_t* dyn, int ninst);
// Get 2 consecutive FPU scratch reg
int fpu_get_double_scratch(dynarec_arm_t* dyn, int ninst);
// Reset scratch regs counter
void fpu_reset_scratch(dynarec_arm_t* dyn);
// Get an x87 double reg
int fpu_get_reg_x87(dynarec_arm_t* dyn, int ninst, int t, int n);
// Get an MMX double reg
int fpu_get_reg_emm(dynarec_arm_t* dyn, int ninst, int emm);
// Get an XMM quad reg
int fpu_get_reg_xmm(dynarec_arm_t* dyn, int t, int xmm);
// Free a FPU/MMX/XMM reg
void fpu_free_reg(dynarec_arm_t* dyn, int reg);
// Reset fpu regs counter
void fpu_reset_reg(dynarec_arm_t* dyn);
// internal YMM handling
int internal_mark_ymm(dynarec_arm_t* dyn, int t, int ymm, int reg);
// is ymm neoncache[reg] one of k1, k2, k3?
int is_ymm_to_keep(dynarec_arm_t* dyn, int reg, int k1, int k2, int k3);

// ---- Neon cache functions
// Get type for STx
int neoncache_get_st(dynarec_arm_t* dyn, int ninst, int a);
// Get if STx is FLOAT
int neoncache_get_st_f(dynarec_arm_t* dyn, int ninst, int a);
// Get if STx is FLOAT or I64
int neoncache_get_st_f_i64(dynarec_arm_t* dyn, int ninst, int a);
// Get actual type for STx
int neoncache_get_current_st(dynarec_arm_t* dyn, int ninst, int a);
// Get actual STx is FLOAT
int neoncache_get_current_st_f(dynarec_arm_t* dyn, int a);
// Get actual STx is FLOAT or I64
int neoncache_get_current_st_f_i64(dynarec_arm_t* dyn, int a);
// Back-propagate a change float->double
void neoncache_promote_double(dynarec_arm_t* dyn, int ninst, int a);
// Combine and propagate if needed (pass 1 only)
int neoncache_combine_st(dynarec_arm_t* dyn, int ninst, int a, int b);  // with stack current dyn->n_stack*
// Do not allow i64 type
int neoncache_no_i64(dynarec_arm_t* dyn, int ninst, int st, int a);

// transform x86 flags to native flags
uint8_t flag2native(uint8_t flags);
// mark a instruction as using/generating flags. return flag
uint8_t mark_natflag(dynarec_arm_t* dyn, int ninst, uint8_t flag, int before);
// propage the use of nativeflags or not (done between step 0 and step 1)
void updateNativeFlags(dynarec_arm_t* dyn);
// raz arm speicifc state when an opcode is unused
void rasNativeState(dynarec_arm_t* dyn, int ninst);
// check if natives flags needs some tranform to/from x86 flags
int nativeFlagsNeedsTransform(dynarec_arm_t* dyn, int ninst);

// FPU Cache transformation (for loops) // Specific, need to be written by backend
int fpuCacheNeedsTransform(dynarec_arm_t* dyn, int ninst);

// Undo the changes of a neoncache to get the status before the instruction
void neoncacheUnwind(neoncache_t* cache);
void fpu_save_and_unwind(dynarec_arm_t* dyn, int ninst, neoncache_t* cache);
void fpu_unwind_restore(dynarec_arm_t* dyn, int ninst, neoncache_t* cache);


// Get if ED will have the correct parity. Not emitting anything. Parity is 2 for DWORD or 3 for QWORD
int getedparity(dynarec_native_t* dyn, int ninst, uintptr_t addr, uint8_t nextop, int parity, int delta);

const char* getCacheName(int t, int n);

void inst_name_pass3(dynarec_native_t* dyn, int ninst, const char* name, rex_t rex);
void print_opcode(dynarec_native_t* dyn, int ninst, uint32_t opcode);

// reset the cache
void fpu_reset(dynarec_native_t* dyn);
void fpu_reset_ninst(dynarec_native_t* dyn, int ninst);

// is st freed
int fpu_is_st_freed(dynarec_native_t* dyn, int ninst, int st);

// propage FPU_BARRIER to trigger it as soon as possible (avoiding fetching an FPU reg if it's unused)
void propagateFpuBarrier(dynarec_arm_t* dyn);
// propage the uneeded flags on XMM/YMM regs (done between step 0 and step 1)
void updateUneeded(dynarec_arm_t* dyn);

void updateYmm0s(dynarec_arm_t* dyn, int ninst, int max_ninst_reached);

// add a preload of SSE/AVX register preload before i2, for a jump in ninst. Might not do it if there is jump in between that goes outside this ninst-i2 interval
void addSSEPreload(dynarec_arm_t* dyn, int ninst, int i2, uint32_t preload);

// AVX helpers
void avx_mark_zero(dynarec_arm_t* dyn, int ninst, int reg);
int is_avx_zero(dynarec_arm_t* dyn, int ninst, int reg);
int is_avx_zero_unset(dynarec_arm_t* dyn, int ninst, int reg);
void avx_mark_zero_reset(dynarec_arm_t* dyn, int ninst);

#endif //__DYNAREC_ARM_FUNCTIONS_H__
