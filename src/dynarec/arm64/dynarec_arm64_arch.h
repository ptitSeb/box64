#ifndef __DYNAREC_ARM_ARCH_H__
#define __DYNAREC_ARM_ARCH_H__

#include <stddef.h>
#include <ucontext.h>

#include "x64emu.h"
#include "box64context.h"
#include "dynarec.h"
#include "dynarec_arm64_private.h"

typedef struct arch_flags_s
{
    uint8_t defered:1;
    uint8_t nf:1;
    uint8_t eq:1;
    uint8_t vf:1;
    uint8_t cf:1;
    uint8_t inv_cf:1;
} arch_flags_t;

// get size of arch specific info (can be 0)
size_t get_size_arch(dynarec_arm_t* dyn);
//populate the array
void populate_arch(dynarec_arm_t* dyn, void* p);
//adjust flags and more
void adjust_arch(dynablock_t* db, x64emu_t* emu, ucontext_t* p, uintptr_t native_addr);
#endif // __DYNAREC_ARM_ARCH_H__
