#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "callback.h"
#include "librarian.h"
#include "box64context.h"
#include "emu/x64emu_private.h"

const char* libxtName = "libXt.so.6";
#define LIBNAME libxt

typedef void  (*vFpuipp_t)(void*, uint32_t, int32_t, void*, void*);

#define SUPER() \
    GO(XtAddEventHandler, vFpuipp_t)

typedef struct libxt_my_s {
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
    // functions
} libxt_my_t;

void* getXtMy(library_t* lib)
{
    libxt_my_t* my = (libxt_my_t*)calloc(1, sizeof(libxt_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeXtMy(void* lib)
{
    (void)lib;
    //libxt_my_t *my = (libxt_my_t *)lib;
}

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)   \
GO(5)   \
GO(6)   \
GO(7)

#define GO(A)   \
static uintptr_t my_Event_fct_##A = 0;   \
static void my_Event_##A(void* w, void* data, void* event)     \
{                                       \
    RunFunction(my_context, my_Event_fct_##A, 3, w, data, event);\
}
SUPER()
#undef GO
static void* findEventFct(void* fct)
{
    if(!fct) return NULL;
    #define GO(A) if(my_Event_fct_##A == (uintptr_t)fct) return my_Event_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_Event_fct_##A == 0) {my_Event_fct_##A = (uintptr_t)fct; return my_Event_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libXt Event callback\n");
    return NULL;
}
#undef SUPER


EXPORT void my_XtAddEventHandler(x64emu_t* emu, void* w, uint32_t mask, int32_t maskable, void* cb, void* data)
{
    (void)emu;
    void* fct = findEventFct(cb);
    library_t* lib = GetLibInternal(libxtName);
    libxt_my_t* my = (libxt_my_t*)lib->priv.w.p2;

    my->XtAddEventHandler(w, mask, maskable, fct, data);
}

#define CUSTOM_INIT \
    lib->priv.w.p2 = getXtMy(lib);   \
    lib->priv.w.needed = 2; \
    lib->priv.w.neededlibs = (char**)calloc(lib->priv.w.needed, sizeof(char*)); \
    lib->priv.w.neededlibs[0] = strdup("libX11.so.6"); \
    lib->priv.w.neededlibs[1] = strdup("libXext.so.6");

#define CUSTOM_FINI \
    freeXtMy(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"
