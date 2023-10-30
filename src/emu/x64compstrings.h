#ifndef __X64_CMPSTRINGS_H__
#define __X64_CMPSTRINGS_H__

#include <stdint.h>

#include "regs.h"

typedef struct x64emu_s x64emu_t;

uint32_t sse42_compare_string_explicit_len(x64emu_t* emu, sse_regs_t* a, int la, sse_regs_t* b, int lb, uint8_t imm8);
uint32_t sse42_compare_string_implicit_len(x64emu_t* emu, sse_regs_t* a, sse_regs_t* b, uint8_t imm8);

#endif //__X64_CMPSTRINGS_H__