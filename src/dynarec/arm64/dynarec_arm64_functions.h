#ifndef __DYNAREC_ARM_FUNCTIONS_H__
#define __DYNAREC_ARM_FUNCTIONS_H__

typedef struct x64emu_s x64emu_t;

void arm_fstp(x64emu_t* emu, void* p);

void arm_print_armreg(x64emu_t* emu, uintptr_t reg, uintptr_t n);

void arm_f2xm1(x64emu_t* emu);
void arm_fyl2x(x64emu_t* emu);
void arm_ftan(x64emu_t* emu);
void arm_fpatan(x64emu_t* emu);
void arm_fxtract(x64emu_t* emu);
void arm_fprem(x64emu_t* emu);
void arm_fyl2xp1(x64emu_t* emu);
void arm_fsincos(x64emu_t* emu);
void arm_frndint(x64emu_t* emu);
void arm_fscale(x64emu_t* emu);
void arm_fsin(x64emu_t* emu);
void arm_fcos(x64emu_t* emu);
void arm_fbld(x64emu_t* emu, uint8_t* ed);
void arm_fild64(x64emu_t* emu, int64_t* ed);
void arm_fbstp(x64emu_t* emu, uint8_t* ed);
void arm_fistp64(x64emu_t* emu, int64_t* ed);
void arm_fistt64(x64emu_t* emu, int64_t* ed);
void arm_fld(x64emu_t* emu, uint8_t* ed);
void arm_fsave(x64emu_t* emu, uint8_t* ed);
void arm_frstor(x64emu_t* emu, uint8_t* ed);
void arm_fprem1(x64emu_t* emu);

void arm_aesd(x64emu_t* emu, int xmm);
void arm_aese(x64emu_t* emu, int xmm);
void arm_aesdlast(x64emu_t* emu, int xmm);
void arm_aeselast(x64emu_t* emu, int xmm);
void arm_aesimc(x64emu_t* emu, int xmm);


void arm_ud(x64emu_t* emu);

// Get an FPU scratch reg
int fpu_get_scratch(dynarec_arm_t* dyn);
// Reset scratch regs counter
void fpu_reset_scratch(dynarec_arm_t* dyn);
// Get an x87 double reg
int fpu_get_reg_x87(dynarec_arm_t* dyn, int t, int n);
// Get an MMX double reg
int fpu_get_reg_emm(dynarec_arm_t* dyn, int emm);
// Get an XMM quad reg
int fpu_get_reg_xmm(dynarec_arm_t* dyn, int t, int xmm);
// Free a FPU/MMX/XMM reg
void fpu_free_reg(dynarec_arm_t* dyn, int reg);
// Reset fpu regs counter
void fpu_reset_reg(dynarec_arm_t* dyn);

// ---- Neon cache functions
// Get type for STx
int neoncache_get_st(dynarec_arm_t* dyn, int ninst, int a);
// Get if STx is FLOAT or DOUBLE
int neoncache_get_st_f(dynarec_arm_t* dyn, int ninst, int a);
// Get actual type for STx
int neoncache_get_current_st(dynarec_arm_t* dyn, int ninst, int a);
// Get actual STx is FLOAT or DOUBLE
int neoncache_get_current_st_f(dynarec_arm_t* dyn, int a);
// Back-propagate a change float->double
void neoncache_promote_double(dynarec_arm_t* dyn, int ninst, int a);
// Combine and propagate if needed (pass 1 only)
int neoncache_combine_st(dynarec_arm_t* dyn, int ninst, int a, int b);  // with stack current dyn->n_stack*

// FPU Cache transformation (for loops)
int fpuCacheNeedsTransform(dynarec_arm_t* dyn, int i1);

// Undo the changes of a neoncache to get the status before the instruction
void neoncacheUnwind(neoncache_t* cache);

// is inst clean for a son branch?
int isInstClean(dynarec_arm_t* dyn, int ninst);

// predecessor access
int isPred(dynarec_arm_t* dyn, int ninst, int pred);
int getNominalPred(dynarec_arm_t* dyn, int ninst);

// Get if ED will have the correct parity. Not emiting anything. Parity is 2 for DWORD or 3 for QWORD
int getedparity(dynarec_arm_t* dyn, int ninst, uintptr_t addr, uint8_t nextop, int parity, int delta);
// Do the GETED, but don't emit anything...
uintptr_t fakeed(dynarec_arm_t* dyn, uintptr_t addr, int ninst, uint8_t nextop);

// Is what pointed at addr a native call? And if yes, to what function?
int isNativeCall(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t* calladdress, int* retn);

const char* getCacheName(int t, int n);

#endif //__DYNAREC_ARM_FUNCTIONS_H__