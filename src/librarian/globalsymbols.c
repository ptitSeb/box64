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

#define GLOB(A, B) \
    if (GetGlobalNoWeakSymbolStartEnd(my_context->maplib, #A, &globoffs, &globend, -1, NULL, 0, NULL)) {  \
        printf_log(LOG_DEBUG, "Global " #A " workaround, @%p <- %p\n", (void*)globoffs, &A);              \
        memcpy((void*)globoffs, &A, sizeof(A));                                                           \
    }                                                                                                     \
    if (B && GetGlobalNoWeakSymbolStartEnd(my_context->maplib, #A, &globoffs, &globend, 2, B, 1, NULL)) { \
        printf_log(LOG_DEBUG, "Global " #A " workaround, @%p <- %p\n", (void*)globoffs, &A);              \
        memcpy((void*)globoffs, &A, sizeof(A));                                                           \
    }

#define TOGLOB(A, B) \
    if (GetGlobalNoWeakSymbolStartEnd(my_context->maplib, #A, &globoffs, &globend, -1, NULL, 0, NULL)) {  \
        printf_log(LOG_DEBUG, "Global " #A " workaround, @%p -> %p\n", (void*)globoffs, &A);              \
        memcpy(&A, (void*)globoffs, sizeof(A));                                                           \
    }                                                                                                     \
    if (B && GetGlobalNoWeakSymbolStartEnd(my_context->maplib, #A, &globoffs, &globend, 2, B, 1, NULL)) { \
        printf_log(LOG_DEBUG, "Global " #A " workaround, @%p -> %p\n", (void*)globoffs, &A);              \
        memcpy(&A, (void*)globoffs, sizeof(A));                                                           \
    }


// *********** GTK *****************
EXPORT void* gdk_display = NULL;   // in case it's used...

void my_checkGlobalGdkDisplay()
{
    uintptr_t globoffs, globend;
    GLOB(gdk_display, NULL)
}

void my_setGlobalGThreadsInit()
{
    int val = 1;
    uintptr_t globoffs, globend;
    if (GetGlobalNoWeakSymbolStartEnd(my_context->maplib, "g_threads_got_initialized", &globoffs, &globend, -1, NULL, 0, NULL)) {
        printf_log(LOG_DEBUG, "Global g_threads_got_initialized workaround, @%p <= %d\n", (void*)globoffs, val);
        memcpy((void*)globoffs, &val, sizeof(val));
    }
}

char* getGDKX11LibName();
void** my_GetGTKDisplay()
{
    if(gdk_display)
        return &gdk_display;
    
    char* name = getGDKX11LibName();
    library_t * lib = GetLibInternal(name?name:"libgdk-x11-2.0.so.0");
    if(!lib) return &gdk_display;   // mmm, that will crash later probably
    void* s = dlsym(GetHandle(lib), "gdk_display");
    gdk_display = *(void**)s;
    return s;
}

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
    GLOB(COLS, NULL)
    GLOB(LINES, NULL)
    GLOB(TABSIZE, NULL)
    GLOB(curscr, NULL)
    GLOB(newscr, NULL)
    GLOB(stdscr, NULL)
    GLOB(acs_map, NULL)
    GLOB(UP, NULL)
    GLOB(BC, NULL)
    GLOB(PC, NULL)
    GLOB(ospeed, NULL)
    GLOB(ttytype, NULL)
}

void my_updateGlobalTInfo()
{
    uintptr_t globoffs, globend;
    TOGLOB(COLS, NULL)
    TOGLOB(LINES, NULL)
    TOGLOB(TABSIZE, NULL)
    TOGLOB(curscr, NULL)
    TOGLOB(newscr, NULL)
    TOGLOB(stdscr, NULL)
    TOGLOB(acs_map, NULL)
    TOGLOB(UP, NULL)
    TOGLOB(BC, NULL)
    TOGLOB(PC, NULL)
    TOGLOB(ospeed, NULL)
    TOGLOB(ttytype, NULL)
}

// **************** getopts ****************
EXPORT char *optarg;
EXPORT int optind, opterr, optopt;

void my_updateGlobalOpt()
{
    uintptr_t globoffs, globend;
    TOGLOB(optarg, "GLIBC_2.2.5");
    TOGLOB(optind, "GLIBC_2.2.5");
    TOGLOB(opterr, "GLIBC_2.2.5");
    TOGLOB(optopt, "GLIBC_2.2.5");
}

void my_checkGlobalOpt()
{
    uintptr_t globoffs, globend;
    GLOB(optarg, "GLIBC_2.2.5");
    GLOB(optind, "GLIBC_2.2.5");
    GLOB(opterr, "GLIBC_2.2.5");
    GLOB(optopt, "GLIBC_2.2.5");
}