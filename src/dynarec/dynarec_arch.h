#ifndef __DYNAREC_ARCH__H_
#define __DYNAREC_ARCH__H_

#ifdef ARM64
#include "arm64/dynarec_arm64_private.h"
#include "arm64/dynarec_arm64_functions.h"

#define instruction_native_t        instruction_arm64_t

uintptr_t arm_pass0(dynarec_arm_t* dyn, uintptr_t addr);
uintptr_t arm_pass1(dynarec_arm_t* dyn, uintptr_t addr);
uintptr_t arm_pass2(dynarec_arm_t* dyn, uintptr_t addr);
uintptr_t arm_pass3(dynarec_arm_t* dyn, uintptr_t addr);

#define native_pass0    arm_pass0
#define native_pass1    arm_pass1
#define native_pass2    arm_pass2
#define native_pass3    arm_pass3

#else
#error Unsupported platform
#endif

#endif //__DYNAREC_ARCH__H_