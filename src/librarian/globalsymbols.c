#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "callback.h"
#include "box64context.h"
#include "librarian.h"
#include "library.h"

// workaround for Globals symbols

#define GLOB(A) \
    if (GetGlobalNoWeakSymbolStartEnd(my_context->maplib, #A, &globoffs, &globend, -1, NULL)) {     \
        printf_log(LOG_DEBUG, "Global " #A " workaround, @%p <= %p\n", (void*)globoffs, &A);        \
        memcpy((void*)globoffs, &A, sizeof(A));                                                     \
    }


// *********** GTK *****************
#if 0
EXPORT void* gdk_display = NULL;   // in case it's used...

void my_checkGlobalGdkDisplay()
{
    uintptr_t globoffs, globend;
    GLOB(gdk_display)
}

void my_setGlobalGThreadsInit()
{
    int val = 1;
    uintptr_t globoffs, globend;
    if (GetGlobalNoWeakSymbolStartEnd(my_context->maplib, "g_threads_got_initialized", &globoffs, &globend, -1, NULL)) {
        printf_log(LOG_DEBUG, "Global g_threads_got_initialized workaround, @%p <= %p\n", (void*)globoffs, (void*)val);
        memcpy((void*)globoffs, &val, sizeof(val));
    }
}

char* getGDKX11LibName();
void** my_GetGTKDisplay()
{
    if(gdk_display)
        return &gdk_display;
    
    char* name = getGDKX11LibName();
    library_t * lib = GetLibInternal(name?name:"libgtk-1.2.so.0");
    if(!lib) return &gdk_display;   // mmm, that will crash later probably
    void* s = dlsym(GetHandle(lib), "gdk_display");
    gdk_display = *(void**)s;
    return s;
}

#endif
// **************** NCurses ****************
EXPORT int COLS;
EXPORT int LINES;
EXPORT int TABSIZE;
EXPORT void* curscr;
EXPORT void* newscr;
EXPORT void* stdscr;
EXPORT void* acs_map[128];
EXPORT void* UP;
EXPORT void* BC;
EXPORT uint8_t PC;
EXPORT uint16_t ospeed;
EXPORT void* ttytype;

void my_checkGlobalTInfo()
{
    uintptr_t globoffs, globend;
    GLOB(COLS)
    GLOB(LINES)
    GLOB(TABSIZE)
    GLOB(curscr)
    GLOB(newscr)
    GLOB(stdscr)
    GLOB(acs_map)
    GLOB(UP)
    GLOB(BC)
    GLOB(PC)
    GLOB(ospeed)
    GLOB(ttytype)
}

// **************** getopts ****************
EXPORT char *optarg;
EXPORT int optind, opterr, optopt;

void my_checkGlobalOpt()
{
    uintptr_t globoffs, globend;
    GLOB(optarg);
    GLOB(optind);
    GLOB(opterr);
    GLOB(optopt);
}