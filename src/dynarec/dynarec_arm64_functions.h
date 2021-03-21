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

void arm_ud(x64emu_t* emu);

// Get an FPU scratch reg
int fpu_get_scratch(dynarec_arm_t* dyn);
// Reset scratch regs counter
void fpu_reset_scratch(dynarec_arm_t* dyn);
// Get an x87 double reg
int fpu_get_reg_x87(dynarec_arm_t* dyn);
// Get an MMX double reg
int fpu_get_reg_emm(dynarec_arm_t* dyn, int emm);
// Get an XMM quad reg
int fpu_get_reg_xmm(dynarec_arm_t* dyn, int xmm);
// Free a FPU/MMX/XMM reg
void fpu_free_reg(dynarec_arm_t* dyn, int reg);
// Reset fpu regs counter
void fpu_reset_reg(dynarec_arm_t* dyn);

// Get if ED will have the correct parity. Not emiting anything. Parity is 2 for DWORD or 3 for QWORD
int getedparity(dynarec_arm_t* dyn, int ninst, uintptr_t addr, uint8_t nextop, int parity);
// Do the GETED, but don't emit anything...
uintptr_t fakeed(dynarec_arm_t* dyn, uintptr_t addr, int ninst, uint8_t nextop);

// Is what pointed at addr a native call? And if yes, to what function?
int isNativeCall(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t* calladdress, int* retn);

#endif //__DYNAREC_ARM_FUNCTIONS_H__