#ifndef __DYNAREC_LA_ARCH_H__
#define __DYNAREC_LA_ARCH_H__

#include <stddef.h>
#include <ucontext.h>

#include "x64emu.h"
#include "box64context.h"
#include "dynarec_la64_private.h"

// get size of arch specific info (can be 0)
size_t get_size_arch(dynarec_la64_t* dyn);
//populate the array
void* populate_arch(dynarec_la64_t* dyn, void* p, size_t sz);
//adjust flags and more (needed for signal handling)
void adjust_arch(dynablock_t* db, x64emu_t* emu, ucontext_t* p, uintptr_t x64pc);
// get if instruction can be regenerated for unaligned access
int arch_unaligned(dynablock_t* db, uintptr_t x64pc);
#endif // __DYNAREC_LA_ARCH_H__
