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
// Limit here is defined by LD litteral, that is 19bits
#define MAXBLOCK_SIZE ((1<<19)-200)

#define RAZ_SPECIFIC(A, N)      rasNativeState(A, N)
#define UPDATE_SPECIFICS(A)     updateNativeFlags(A)
#define PREUPDATE_SPECIFICS(A)

#define ARCH_SIZE(A)    get_size_arch(A)
#define ARCH_FILL(A, B) populate_arch(A, B)
#define ARCH_ADJUST(A, B, C, D) adjust_arch(A, B, C, D)
#define STOP_NATIVE_FLAGS(A, B)   A->insts[B].nat_flags_op = NAT_FLAG_OP_UNUSABLE
#define ARCH_UNALIGNED(A, B) arch_unaligned(A, B)
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

#define ARCH_SIZE(A)    0
#define ARCH_FILL(A, B)  {}
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

#define ARCH_SIZE(A)    get_size_arch(A)
#define ARCH_FILL(A, B) populate_arch(A, B)
#define ARCH_ADJUST(A, B, C, D) {}
#define STOP_NATIVE_FLAGS(A, B) {}
#define ARCH_UNALIGNED(A, B) arch_unaligned(A, B)
#else
#error Unsupported platform
#endif

#endif //__DYNAREC_ARCH__H_
