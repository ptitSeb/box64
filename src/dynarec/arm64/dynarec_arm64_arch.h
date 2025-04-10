#ifndef __DYNAREC_ARM_ARCH_H__
#define __DYNAREC_ARM_ARCH_H__

#include <stddef.h>

#include "x64emu.h"
#include "box64context.h"
#include "box64cpu.h"
#include "dynarec_arm64_private.h"

// get size of arch specific info (can be 0)
size_t get_size_arch(dynarec_arm_t* dyn);
//populate the array
void* populate_arch(dynarec_arm_t* dyn, void* p, size_t sz);
#ifndef _WIN32
#include <ucontext.h>
//adjust flags and more
void adjust_arch(dynablock_t* db, x64emu_t* emu, ucontext_t* p, uintptr_t x64pc);
#else
#define adjust_arch(db, emu, p, x64pc)
#endif
// get if instruction can be regenerated for unaligned access
int arch_unaligned(dynablock_t* db, uintptr_t x64pc);
#endif // __DYNAREC_ARM_ARCH_H__
