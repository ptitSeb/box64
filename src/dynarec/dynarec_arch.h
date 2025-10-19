#ifndef __DYNAREC_ARCH__H_
#define __DYNAREC_ARCH__H_

#ifdef ARM64

#define instruction_native_t        instruction_arm64_t
#define dynarec_native_t            dynarec_arm_t
#define extcache_native_t           neoncache_t

#define ADDITIONNAL_DEFINITION()  \
    int fpuCacheNeedsTransform(dynarec_native_t* dyn, int ninst);

#define OTHER_CACHE()                                   \
    if (fpuCacheNeedsTransform(dyn, ninst)) ret|=2;     \
    if (nativeFlagsNeedsTransform(dyn, ninst)) ret|=4;

#include "arm64/arm64_printer.h"
#include "arm64/dynarec_arm64_private.h"
#include "arm64/dynarec_arm64_functions.h"
#include "arm64/dynarec_arm64_arch.h"
// TBZ/TBNZ is signed 14bits shifted by 2, but not use to jump from opcode to opcode
// Limit here is defined by LD litteral, that is 19bits signed ledt shifted by 2. So 20bits (positive/negative) address space (used for TABLE64)
// C.cond is also signed 19bits shifted by 2
#define MAXBLOCK_SIZE ((1<<20)-200)

#define RAZ_SPECIFIC(A, N)      rasNativeState(A, N)
#define UPDATE_SPECIFICS(A)     updateYmm0s(dyn, 0, 0); updateNativeFlags(A); propagateFpuBarrier(A)
#define PREUPDATE_SPECIFICS(A)
#define POSTUPDATE_SPECIFICS(A) updateUneeded(A)
#define ARCH_SIZE(A)    get_size_arch(A)
#define ARCH_FILL(A, B, C) populate_arch(A, B, C)
#define ARCH_ADJUST(A, B, C, D) adjust_arch(A, B, C, D)
#define STOP_NATIVE_FLAGS(A, B)   A->insts[B].nat_flags_op = NAT_FLAG_OP_UNUSABLE
#define ARCH_UNALIGNED(A, B) arch_unaligned(A, B)
extern uint32_t arm64_crc(void* p, uint32_t len);
#define ARCH_CRC(A, B)  if(cpuext.crc32) return arm64_crc(A, B)
extern void* create_updateflags();
#define ARCH_UPDATEFLAGS()      create_updateflags()

#define ARCH_NOP    0b11010101000000110010000000011111
#define ARCH_UDF    0xcafe
#elif defined(LA64)

#define instruction_native_t        instruction_la64_t
#define dynarec_native_t            dynarec_la64_t
#define extcache_native_t           lsxcache_t

#define ADDITIONNAL_DEFINITION() \
    int fpuCacheNeedsTransform(dynarec_native_t* dyn, int ninst);

#define OTHER_CACHE() \
    if (fpuCacheNeedsTransform(dyn, ninst)) ret |= 2;

#include "la64/la64_printer.h"
#include "la64/dynarec_la64_private.h"
#include "la64/dynarec_la64_functions.h"
// Limit here is unconditionnal jump, that is signed 28bits
#define MAXBLOCK_SIZE ((1 << 27) - 200)

#define RAZ_SPECIFIC(A, N)
#define UPDATE_SPECIFICS(A)
#define PREUPDATE_SPECIFICS(A) updateNativeFlags(A)
#define POSTUPDATE_SPECIFICS(A)

#define ARCH_SIZE(A)    0
#define ARCH_FILL(A, B, C)  NULL
#define ARCH_ADJUST(A, B, C, D) {}
#define STOP_NATIVE_FLAGS(A, B) {}
#define ARCH_UNALIGNED(A, B) 0

#elif defined(RV64)

#define instruction_native_t        instruction_rv64_t
#define dynarec_native_t            dynarec_rv64_t
#define extcache_native_t           extcache_t

#define ADDITIONNAL_DEFINITION()                                  \
    int fpuCacheNeedsTransform(dynarec_native_t* dyn, int ninst); \
    int sewNeedsTransform(dynarec_rv64_t* dyn, int ninst);

#define OTHER_CACHE()                                 \
    if (fpuCacheNeedsTransform(dyn, ninst)) ret |= 2; \
    if (sewNeedsTransform(dyn, ninst)) ret |= 4;

#include "rv64/rv64_printer.h"
#include "rv64/dynarec_rv64_private.h"
#include "rv64/dynarec_rv64_functions.h"
#include "rv64/dynarec_rv64_arch.h"
// Limit here is unconditionnal jump, that is signed 21bits
#define MAXBLOCK_SIZE ((1<<20)-200)

#define RAZ_SPECIFIC(A, N)
#define UPDATE_SPECIFICS(A)
#define PREUPDATE_SPECIFICS(A) updateNativeFlags(A)
#define POSTUPDATE_SPECIFICS(A)

#define ARCH_SIZE(A)    get_size_arch(A)
#define ARCH_FILL(A, B, C) populate_arch(A, B, C)
#define ARCH_ADJUST(A, B, C, D) {}
#define STOP_NATIVE_FLAGS(A, B) {}
#define ARCH_UNALIGNED(A, B) arch_unaligned(A, B)
#else
#error Unsupported platform
#endif

#include "dynacache_reloc.h"

#endif //__DYNAREC_ARCH__H_
