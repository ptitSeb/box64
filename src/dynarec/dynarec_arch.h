#ifndef __DYNAREC_ARCH__H_
#define __DYNAREC_ARCH__H_

#ifdef ARM64
#include "arm64/arm64_printer.h"
#include "arm64/dynarec_arm64_private.h"
#include "arm64/dynarec_arm64_functions.h"

#define instruction_native_t        instruction_arm64_t
#define dynarec_native_t            dynarec_arm_t

#else
#error Unsupported platform
#endif

#endif //__DYNAREC_ARCH__H_
