#ifndef __DYNAREC_PPC64LE_FUNCTIONS_H__
#define __DYNAREC_PPC64LE_FUNCTIONS_H__

#include "../dynarec_native_functions.h"

// First VMX scratch register index
#define SCRATCH0    24

// Map cache index to VSX hardware register number (vs0-vs63).
// XMM indices 0-15     → vs32-vs47  (VMX vr0-vr15), enabling VMX integer SIMD ops.
// MMX indices 16-23    → vs56-vs63  (VMX vr24-vr31), enabling VMX integer SIMD ops.
// Scratch indices 24-31 → vs48-vs55 (VMX vr16-vr23), enabling VMX ops on scratch.
//
// The scratch mapping is critical: VMX (VX/VA-form) instructions encode a 5-bit
// VR number (vr0-vr31 = vs32-vs63).  Placing scratch in vs48-vs55 (vr16-vr23)
// allows VRREG() to produce the correct VR number for both XMM and scratch regs.
// Note: vr20-vr23 (vs52-vs55) are callee-saved per ELFv2 ABI.  We save vr20 in
// the prolog since the maximum simultaneous scratch count is 5 (indices 24-28).
//
// x87 and MMX are mutually exclusive (EMMS separates them), but on PPC64LE their
// hardware register requirements differ: x87 needs FPR (scalar FP) while MMX needs
// VR (VMX integer SIMD).  Since FPR and VR are NOT aliased on POWER (unlike LA64),
// VSXREG routes MMX indices 16-23 to VR space (vs56-vs63), while VSXREG_X87 routes
// them to FPR space (vs16-vs23).  x87 code must use VSXREG_X87 instead of VSXREG.
// vr24-vr31 (vs56-vs63) are callee-saved; the prolog/epilog saves/restores them.
#define VSXREG(idx)      ((idx) < 16 ? (idx) + 32 : (idx) < 24 ? (idx) + 40 : (idx) + 24)
#define VSXREG_X87(idx)  (idx)  /* x87 idx 16-23 → vs16-vs23 (FPR f16-f23) */

// Extract VR register number for VMX integer instructions (5-bit, vr0-vr31).
// XMM cache indices 0-15    → vr0-vr15  (vs32-vs47).
// MMX cache indices 16-23   → vr24-vr31 (vs56-vs63).
// Scratch cache indices 24-31 → vr16-vr23 (vs48-vs55).
#define VRREG(idx)   ((idx) < 16 ? (idx) : (idx) < 24 ? (idx) + 8 : (idx) - 8)

typedef struct x64emu_s x64emu_t;
typedef struct dynarec_ppc64le_s dynarec_ppc64le_t;

// Get an FPU scratch reg
int fpu_get_scratch(dynarec_ppc64le_t* dyn);
// Reset scratch regs counter
void fpu_reset_scratch(dynarec_ppc64le_t* dyn);
// Get an x87 double reg
int fpu_get_reg_x87(dynarec_ppc64le_t* dyn, int t, int n);
// Get an XMM quad reg
int fpu_get_reg_xmm(dynarec_ppc64le_t* dyn, int t, int xmm);
// Get an YMM quad reg
int fpu_get_reg_ymm(dynarec_ppc64le_t* dyn, int t, int ymm);
// Free a FPU/MMX/XMM reg
void fpu_free_reg(dynarec_ppc64le_t* dyn, int reg);
// Reset fpu regs counter
void fpu_reset_reg(dynarec_ppc64le_t* dyn);
// Get an MMX double reg
int fpu_get_reg_emm(dynarec_ppc64le_t* dyn, int emm);

// ---- VMX cache functions
// Get type for STx
int vmxcache_get_st(dynarec_ppc64le_t* dyn, int ninst, int a);
// Get if STx is FLOAT or DOUBLE
int vmxcache_get_st_f(dynarec_ppc64le_t* dyn, int ninst, int a);
// Get if STx is FLOAT or I64
int vmxcache_get_st_f_i64(dynarec_ppc64le_t* dyn, int ninst, int a);
// Get actual type for STx
int vmxcache_get_current_st(dynarec_ppc64le_t* dyn, int ninst, int a);
// Get actual STx is FLOAT or DOUBLE
int vmxcache_get_current_st_f(dynarec_ppc64le_t* dyn, int a);
// Get actual STx is FLOAT or I64
int vmxcache_get_current_st_f_i64(dynarec_ppc64le_t* dyn, int a);
// Back-propagate a change float->double
void vmxcache_promote_double(dynarec_ppc64le_t* dyn, int ninst, int a);
// Combine and propagate if needed (pass 1 only)
int vmxcache_combine_st(dynarec_ppc64le_t* dyn, int ninst, int a, int b);  // with stack current dyn->v_stack*
// Do not allow i64 type
int vmxcache_no_i64(dynarec_ppc64le_t* dyn, int ninst, int st, int a);

// FPU Cache transformation (for loops) // Specific, need to be written by backend
int fpuCacheNeedsTransform(dynarec_ppc64le_t* dyn, int ninst);

// Undo the changes of a vmxcache to get the status before the instruction
void vmxcacheUnwind(vmxcache_t* cache);
void fpu_save_and_unwind(dynarec_ppc64le_t* dyn, int ninst, vmxcache_t* cache);
void fpu_unwind_restore(dynarec_ppc64le_t* dyn, int ninst, vmxcache_t* cache);

const char* getCacheName(int t, int n);

void inst_name_pass3(dynarec_native_t* dyn, int ninst, const char* name, rex_t rex);
void print_opcode(dynarec_native_t* dyn, int ninst, uint32_t opcode);

// reset the cache
void fpu_reset(dynarec_native_t* dyn);
void fpu_reset_ninst(dynarec_native_t* dyn, int ninst);
// is st freed
int fpu_is_st_freed(dynarec_native_t* dyn, int ninst, int st);

// propagate FPU_BARRIER to trigger it as soon as possible (avoiding fetching an FPU reg if it's unused)
void propagateFpuBarrier(dynarec_ppc64le_t* dyn);

// propagate the unneeded flags on XMM/YMM regs (done between step 0 and step 1)
void updateYmm0s(dynarec_ppc64le_t* dyn, int ninst, int max_ninst_reached);

// Update native flags fusion info
void updateNativeFlags(dynarec_ppc64le_t* dyn);

// Get free scratch registers avoiding native flag operands
void get_free_scratch(dynarec_ppc64le_t* dyn, int ninst, uint8_t* tmp1, uint8_t* tmp2, uint8_t* tmp3, uint8_t s1, uint8_t s2, uint8_t s3, uint8_t s4, uint8_t s5);

// Try to put FPU barrier earlier
void tryEarlyFpuBarrier(dynarec_ppc64le_t* dyn, int last_fpu_used, int ninst);

#endif //__DYNAREC_PPC64LE_FUNCTIONS_H__
