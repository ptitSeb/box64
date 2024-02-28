#ifndef __DYNAREC_HELPER__H_
#define __DYNAREC_HELPER__H_

#ifdef ARM64
#include "arm64/dynarec_arm64_helper.h"
#elif defined(LA64)
#include "la64/dynarec_la64_helper.h"
#elif defined(RV64)
#include "rv64/dynarec_rv64_helper.h"
#else
#error Unsupported architecture
#endif

#endif //__DYNAREC_HELPER__H_