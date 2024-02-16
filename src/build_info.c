#include <stdio.h>
#include "debug.h"
#include "box64version.h"
#ifdef NOGIT
#define GITREV "nogit"
#else
#include "git_head.h"
#endif

void PrintBox64Version()
{
    printf_log(LOG_NONE, "Box64%s%s v%d.%d.%d %s built on %s %s\n", 
    #ifdef HAVE_TRACE
        " with trace",
    #else
        "",
    #endif
    #ifdef DYNAREC
        " with Dynarec",
    #else
        "",
    #endif
        BOX64_MAJOR, BOX64_MINOR, BOX64_REVISION,
        GITREV,
        __DATE__, __TIME__);
}

