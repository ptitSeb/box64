#include "box64version.h"
#ifdef NOGIT
#define GITREV "nogit"
#else
#include "git_head.h"
#endif

#ifndef __BUILD_INFO_H__
#define __BUILD_INFO_H__

void PrintBox64Version(void);

#define BOX64_BUILD_INFO_STR_HELPER(x) #x
#define BOX64_BUILD_INFO_STR(x) BOX64_BUILD_INFO_STR_HELPER(x)

#define BOX64_BUILD_INFO_STRING \
    "Box64" \
    " v" BOX64_BUILD_INFO_STR(BOX64_MAJOR) "." BOX64_BUILD_INFO_STR(BOX64_MINOR) "." BOX64_BUILD_INFO_STR(BOX64_REVISION) \
    " " GITREV

#endif //__BUILD_INFO_H__