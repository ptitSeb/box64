#include <stdio.h>
#include "debug.h"
#include "box64version.h"
#include "build_info.h"

#if defined(DYNAREC)
#define WITH_DYNAREC_STR " with Dynarec"
#else
#define WITH_DYNAREC_STR ""
#endif

#ifdef HAVE_TRACE
#define WITH_TRACE_STR " with trace"
#else
#define WITH_TRACE_STR ""
#endif

void PrintBox64Version(int prefix)
{
    PrintfFtrace(prefix, BOX64_BUILD_INFO_STRING WITH_DYNAREC_STR WITH_TRACE_STR " built on %s %s\n",
        __DATE__, __TIME__);
}

#undef WITH_TRACE
#undef WITH_DYNAREC
