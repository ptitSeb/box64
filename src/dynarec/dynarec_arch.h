#ifndef __DYNAREC_ARCH__H_
#define __DYNAREC_ARCH__H_

#ifdef ARM64

#define instruction_native_t        instruction_arm64_t
#define dynarec_native_t            dynarec_arm_t

#define ADDITIONNAL_DEFINITION()  \
    int fpuCacheNeedsTransform(dynarec_native_t* dyn, int ninst);

#define OTHER_CACHE()   \
    if (fpuCacheNeedsTransform(dyn, ninst)) ret|=2;

#include "arm64/arm64_printer.h"
#include "arm64/dynarec_arm64_private.h"
#include "arm64/dynarec_arm64_functions.h"
// Limit here is defined by LD litteral, that is 19bits
#define MAXBLOCK_SIZE ((1<<19)-200)
#elif defined(LA64)

#define instruction_native_t        instruction_la64_t
#define dynarec_native_t            dynarec_la64_t

#define ADDITIONNAL_DEFINITION() \
    int fpuCacheNeedsTransform(dynarec_native_t* dyn, int ninst);

#define OTHER_CACHE() \
    if (fpuCacheNeedsTransform(dyn, ninst)) ret |= 2;

#include "la64/la64_printer.h"
#include "la64/dynarec_la64_private.h"
#include "la64/dynarec_la64_functions.h"
// Limit here is unconditionnal jump, that is signed 28bits
#define MAXBLOCK_SIZE ((1 << 27) - 200)
#elif defined(RV64)

#define instruction_native_t        instruction_rv64_t
#define dynarec_native_t            dynarec_rv64_t

#define ADDITIONNAL_DEFINITION()  \
    int fpuCacheNeedsTransform(dynarec_native_t* dyn, int ninst);

#define OTHER_CACHE()   \
    if (fpuCacheNeedsTransform(dyn, ninst)) ret|=2;

#include "rv64/rv64_printer.h"
#include "rv64/dynarec_rv64_private.h"
#include "rv64/dynarec_rv64_functions.h"
// Limit here is unconditionnal jump, that is signed 21bits
#define MAXBLOCK_SIZE ((1<<20)-200)
#else
#error Unsupported platform
#endif

#endif //__DYNAREC_ARCH__H_
