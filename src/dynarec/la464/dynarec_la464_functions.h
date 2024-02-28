#ifndef __DYNAREC_LA464_FUNCTIONS_H__
#define __DYNAREC_LA464_FUNCTIONS_H__

#include "../dynarec_native_functions.h"

typedef struct x64emu_s x64emu_t;
typedef struct dynarec_rv64_s dynarec_rv64_t;

// Reset scratch regs counter
void fpu_reset_scratch(dynarec_la464_t* dyn);

void inst_name_pass3(dynarec_native_t* dyn, int ninst, const char* name, rex_t rex);
void print_opcode(dynarec_native_t* dyn, int ninst, uint32_t opcode);

#endif //__DYNAREC_LA464_FUNCTIONS_H__
