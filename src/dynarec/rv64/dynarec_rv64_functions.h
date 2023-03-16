#ifndef __DYNAREC_RV64_FUNCTIONS_H__
#define __DYNAREC_RV64_FUNCTIONS_H__

#include "../dynarec_native_functions.h"

typedef struct x64emu_s x64emu_t;
typedef struct dynarec_rv64_s dynarec_rv64_t;

// Reset scratch regs counter
void fpu_reset_scratch(dynarec_rv64_t* dyn);

#endif //__DYNAREC_RV64_FUNCTIONS_H__