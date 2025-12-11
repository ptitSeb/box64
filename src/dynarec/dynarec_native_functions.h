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
void native_fsave16(x64emu_t* emu, uint8_t* ed);
void native_frstor(x64emu_t* emu, uint8_t* ed);
void native_frstor16(x64emu_t* emu, uint8_t* ed);
void native_fprem1(x64emu_t* emu);

double direct_f2xm1(x64emu_t* emu, double a);
double direct_fyl2x(x64emu_t* emu, double a, double b);
double direct_fyl2xp1(x64emu_t* emu, double a, double b);
double direct_fpatan(x64emu_t* emu, double a, double b);
double direct_fsin(x64emu_t* emu, double a);
double direct_fcos(x64emu_t* emu, double a);
double direct_ftan(x64emu_t* emu, double a);
double direct_fscale(x64emu_t* emu, double a, double b);

void native_aesd(x64emu_t* emu, int xmm);
void native_aesd_y(x64emu_t* emu, int ymm);
void native_aese(x64emu_t* emu, int xmm);
void native_aese_y(x64emu_t* emu, int ymm);
void native_aesdlast(x64emu_t* emu, int xmm);
void native_aesdlast_y(x64emu_t* emu, int ymm);
void native_aeselast(x64emu_t* emu, int xmm);
void native_aeselast_y(x64emu_t* emu, int ymm);
void native_aesimc(x64emu_t* emu, int xmm);
void native_aesimc_y(x64emu_t* emu, int ymm);
void native_aeskeygenassist(x64emu_t* emu, int gx, int ex, void* p, uint32_t u8);
void native_pclmul(x64emu_t* emu, int gx, int ex, void* p, uint32_t u8);
void native_pclmul_x(x64emu_t* emu, int gx, int vx, void* p, uint32_t u8);
void native_pclmul_y(x64emu_t* emu, int gy, int vy, void* p, uint32_t u8);

void native_ud(x64emu_t* emu);
void native_br(x64emu_t* emu);
void native_priv(x64emu_t* emu);
void native_gpf(x64emu_t* emu);
void native_int3(x64emu_t* emu);
void native_int(x64emu_t* emu, int num);
void native_wineint(x64emu_t* emu, int num);
void native_div0(x64emu_t* emu);

// Caches transformation (for loops) // Specific, need to be written par backend
int CacheNeedsTransform(dynarec_native_t* dyn, int i1);
// propagete defererd to unknow, as state is not needed
void propagate_nodf(dynarec_native_t* dyn, int ninst);

// predecessor access
int isPred(dynarec_native_t* dyn, int ninst, int pred);
int getNominalPred(dynarec_native_t* dyn, int ninst);

// Do the GETED, but don't emit anything...
uintptr_t fakeed(dynarec_native_t* dyn, uintptr_t addr, int ninst, uint8_t nextop);
// return Ib on a mod/rm opcode without emitting anything
uint8_t geted_ib(dynarec_native_t* dyn, uintptr_t addr, int ninst, uint8_t nextop);

typedef struct register_mapping_s {
    const char* name;
    const char* native;
} register_mapping_t;

void x64disas_add_register_mapping_annotations(char* buf, const char* disas, const register_mapping_t* mappings, size_t mappings_sz);

ADDITIONNAL_DEFINITION()

#endif //__DYNAREC_NATIVE_FUNCTIONS_H__
