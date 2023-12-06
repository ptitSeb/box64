#ifndef __DYNAREC_NATIVE_FUNCTIONS_H__
#define __DYNAREC_NATIVE_FUNCTIONS_H__

#include <stdint.h>

#include "dynarec_arch.h"

typedef struct x64emu_s x64emu_t;

void native_fstp(x64emu_t* emu, void* p);

void native_print_armreg(x64emu_t* emu, uintptr_t reg, uintptr_t n);

void native_f2xm1(x64emu_t* emu);
void native_fyl2x(x64emu_t* emu);
void native_ftan(x64emu_t* emu);
void native_fpatan(x64emu_t* emu);
void native_fxtract(x64emu_t* emu);
void native_fprem(x64emu_t* emu);
void native_fyl2xp1(x64emu_t* emu);
void native_fsincos(x64emu_t* emu);
void native_frndint(x64emu_t* emu);
void native_fscale(x64emu_t* emu);
void native_fsin(x64emu_t* emu);
void native_fcos(x64emu_t* emu);
void native_fbld(x64emu_t* emu, uint8_t* ed);
void native_fild64(x64emu_t* emu, int64_t* ed);
void native_fbstp(x64emu_t* emu, uint8_t* ed);
void native_fistp64(x64emu_t* emu, int64_t* ed);
void native_fistt64(x64emu_t* emu, int64_t* ed);
void native_fld(x64emu_t* emu, uint8_t* ed);
void native_fsave(x64emu_t* emu, uint8_t* ed);
void native_frstor(x64emu_t* emu, uint8_t* ed);
void native_fprem1(x64emu_t* emu);

void native_aesd(x64emu_t* emu, int xmm);
void native_aese(x64emu_t* emu, int xmm);
void native_aesdlast(x64emu_t* emu, int xmm);
void native_aeselast(x64emu_t* emu, int xmm);
void native_aesimc(x64emu_t* emu, int xmm);
void native_aeskeygenassist(x64emu_t* emu, int gx, int ex, void* p, uint32_t u8);
void native_pclmul(x64emu_t* emu, int gx, int ex, void* p, uint32_t u8);

void native_clflush(x64emu_t* emu, void* p);

void native_ud(x64emu_t* emu);
void native_priv(x64emu_t* emu);
void native_singlestep(x64emu_t* emu);
void native_int3(x64emu_t* emu);
void native_int(x64emu_t* emu, int num);
void native_div0(x64emu_t* emu);

// Caches transformation (for loops) // Specific, need to be written par backend
int CacheNeedsTransform(dynarec_native_t* dyn, int i1);

// predecessor access
int isPred(dynarec_native_t* dyn, int ninst, int pred);
int getNominalPred(dynarec_native_t* dyn, int ninst);

// Do the GETED, but don't emit anything...
uintptr_t fakeed(dynarec_native_t* dyn, uintptr_t addr, int ninst, uint8_t nextop);
// return Ib on a mod/rm opcode without emiting anything
uint8_t geted_ib(dynarec_native_t* dyn, uintptr_t addr, int ninst, uint8_t nextop);

// Is what pointed at addr a native call? And if yes, to what function?
int isNativeCall(dynarec_native_t* dyn, uintptr_t addr, uintptr_t* calladdress, int* retn);

ADDITIONNAL_DEFINITION()

#endif //__DYNAREC_NATIVE_FUNCTIONS_H__
