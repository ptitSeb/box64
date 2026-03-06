#ifndef __DYNAREC_PPC64LE_ARCH_H__
#define __DYNAREC_PPC64LE_ARCH_H__

#include <stddef.h>

#include "x64emu.h"
#include "box64context.h"
#include "box64cpu.h"
#include "dynarec_ppc64le_private.h"

// get size of arch specific info (can be 0)
size_t get_size_arch(dynarec_ppc64le_t* dyn);
//populate the array
void* populate_arch(dynarec_ppc64le_t* dyn, void* p, size_t sz);
#include <ucontext.h>
//adjust flags and more
void adjust_arch(dynablock_t* db, x64emu_t* emu, ucontext_t* p, uintptr_t x64pc);
// get if instruction can be regenerated for unaligned access
int arch_unaligned(dynablock_t* db, uintptr_t x64pc);
#endif // __DYNAREC_PPC64LE_ARCH_H__
