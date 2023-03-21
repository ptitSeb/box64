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
#elif defined(LA464)

#define instruction_native_t        instruction_la464_t
#define dynarec_native_t            dynarec_la464_t

#define ADDITIONNAL_DEFINITION()

#define OTHER_CACHE()

#include "la464/la464_printer.h"
#include "la464/dynarec_la464_private.h"
#include "la464/dynarec_la464_functions.h"
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
#else
#error Unsupported platform
#endif

#endif //__DYNAREC_ARCH__H_
