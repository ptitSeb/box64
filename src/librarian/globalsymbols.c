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
#include "globalsymbols.h"

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


void addGlobalRef(void* p, const char* symname)
{
    if(!strcmp(symname, "gdk_display") && !BOX64ENV(nogtk))
        addGlobalGdkDisplayRef(p);
}

// *********** GTK *****************
EXPORT void* gdk_display = NULL;   // in case it's used...

static void** gdk_display_refs = NULL;
static size_t gdk_display_cap = 0;
static size_t gdk_display_size = 0;

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

void my_checkGlobalGdkDisplay()
{
    uintptr_t globoffs, globend;
    void** p_gdk_display = &gdk_display;
    my_GetGTKDisplay(); // make sure to refresh gdk_display
    printf_log(LOG_DEBUG, "Refreshing Glbal gdk_diplay with %p\n", gdk_display);
    GLOB(p_gdk_display, NULL)
    // also check referenced addresses
    for(size_t i=0; i<gdk_display_size; ++i)
        memcpy(gdk_display_refs[i], &p_gdk_display, sizeof(p_gdk_display));
}

void addGlobalGdkDisplayRef(void* p)
{
    // check if address is already there
    for(size_t i=0; i<gdk_display_size; ++i)
        if(gdk_display_refs[i] == p)
            return;
    // resize if needed
    if(gdk_display_size==gdk_display_cap) {
        gdk_display_cap += 4;
        gdk_display_refs = box_realloc(gdk_display_refs, gdk_display_cap*sizeof(void*));
    }
    // add
    gdk_display_refs[gdk_display_size++] = p;
    printf_log(LOG_DEBUG, "Added %p ref to gdk_display\n", p);
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

// **************** NCurses ****************
EXPORT int COLS = 0;
EXPORT int LINES = 0;
EXPORT int TABSIZE = 0;
EXPORT void* curscr = NULL;
EXPORT void* newscr = NULL;
EXPORT void* stdscr = NULL;
EXPORT void* acs_map[64] = {0};
EXPORT void* UP = NULL;
EXPORT void* BC = NULL;
EXPORT uint8_t PC = 0;
EXPORT uint16_t ospeed = 0;
EXPORT void* ttytype[32] = {0};
EXPORT void* cur_term = NULL;

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
    GLOB(cur_term, NULL)
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
    TOGLOB(cur_term, NULL)
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

// **************** libxslt ****************
EXPORT int xsltMaxVars;
void my_updateGlobalXslt()
{
    uintptr_t globoffs, globend;
    TOGLOB(xsltMaxVars, "LIBXML2_1.0.24");
}

void my_checkGlobalXslt()
{
    uintptr_t globoffs, globend;
    GLOB(xsltMaxVars, "LIBXML2_1.0.24");
}
