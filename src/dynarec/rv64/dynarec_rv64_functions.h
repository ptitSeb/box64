#ifndef __DYNAREC_RV64_FUNCTIONS_H__
#define __DYNAREC_RV64_FUNCTIONS_H__
#include <stdint.h>

typedef struct x64emu_s x64emu_t;
typedef struct dynarec_rv64_s dynarec_rv64_t;

// Reset scratch regs counter
void fpu_reset_scratch(dynarec_rv64_t* dyn);

// Get if ED will have the correct parity. Not emiting anything. Parity is 2 for DWORD or 3 for QWORD
int getedparity(dynarec_rv64_t* dyn, int ninst, uintptr_t addr, uint8_t nextop, int parity, int delta);
// Do the GETED, but don't emit anything...
uintptr_t fakeed(dynarec_rv64_t* dyn, uintptr_t addr, int ninst, uint8_t nextop);

// Is what pointed at addr a native call? And if yes, to what function?
int isNativeCall(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t* calladdress, int* retn);

// FPU Cache transformation (for loops)
int CacheNeedsTransform(dynarec_rv64_t* dyn, int i1);

// predecessor access
int isPred(dynarec_rv64_t* dyn, int ninst, int pred);
int getNominalPred(dynarec_rv64_t* dyn, int ninst);

#endif //__DYNAREC_RV64_FUNCTIONS_H__