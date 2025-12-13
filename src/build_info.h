#include "box64version.h"
#ifdef NOGIT
#define GITREV "nogit"
#else
#include "git_head.h"
#endif

#ifndef __BUILD_INFO_H__
#define __BUILD_INFO_H__


#if defined(DYNAREC) && (defined(ARM64) || defined(RV64) || defined(LA64))
#if defined(ARM64)
#define ARCH_STR " arm64"
#elif defined(RV64)
#define ARCH_STR " riscv64"
#elif defined(LA64)
#define ARCH_STR " loongarch64"
#endif
#else
#define ARCH_STR ""
#endif

void PrintBox64Version(int prefix);

#define BOX64_BUILD_INFO_STR_HELPER(x) #x
#define BOX64_BUILD_INFO_STR(x) BOX64_BUILD_INFO_STR_HELPER(x)

#ifdef _WIN32
#define BOX64_STR "WowBox64"
#else
#define BOX64_STR "Box64"
#endif

#define BOX64_BUILD_INFO_STRING \
    BOX64_STR ARCH_STR          \
        " v" BOX64_BUILD_INFO_STR(BOX64_MAJOR) "." BOX64_BUILD_INFO_STR(BOX64_MINOR) "." BOX64_BUILD_INFO_STR(BOX64_REVISION) " " GITREV

#define BOX64_BUILD_INFO_STRING_SHORT \
    BOX64_STR                         \
    " v" BOX64_BUILD_INFO_STR(BOX64_MAJOR) "." BOX64_BUILD_INFO_STR(BOX64_MINOR) "." BOX64_BUILD_INFO_STR(BOX64_REVISION)

#endif //__BUILD_INFO_H__
