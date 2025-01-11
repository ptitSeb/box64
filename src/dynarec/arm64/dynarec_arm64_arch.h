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

typedef struct arch_x87_s
{
    int8_t dela;        //up to +/-7
    uint8_t x87;        // 1bit is STx present
    uint32_t x87_pos;   //4bits per STx position
} arch_x87_t;

typedef struct arch_mmx_s
{
    uint8_t mmx;    //1bit for each mmx reg present
} arch_mmx_t;

typedef struct arch_sse_s
{
    uint16_t sse;   //1bit for each sse reg present
} arch_sse_t;

typedef struct arch_ymm_s
{
    uint16_t ymm0;      // 1bit for ymm0
    uint16_t ymm;       // 1bit for each ymm present
    uint64_t ymm_pos;   // 4bits for position of each ymm present
} arch_ymm_t;

typedef struct arch_arch_s
{
    uint16_t flags:1;
    uint16_t x87:1;
    uint16_t mmx:1;
    uint16_t sse:1;
    uint16_t ymm:1;
    uint16_t unaligned:1;
    uint16_t seq:10;    // how many instruction on the same values
} arch_arch_t;

// get size of arch specific info (can be 0)
size_t get_size_arch(dynarec_arm_t* dyn);
//populate the array
void populate_arch(dynarec_arm_t* dyn, void* p);
//adjust flags and more
void adjust_arch(dynablock_t* db, x64emu_t* emu, ucontext_t* p, uintptr_t native_addr);
// get if instruction can be regenerated for unaligned access
int arch_unaligned(dynablock_t* db, uintptr_t x64pc);
#endif // __DYNAREC_ARM_ARCH_H__
