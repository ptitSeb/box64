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

const char* libxtstName = "libXtst.so.6";
#define LIBNAME libxtst

typedef int (*iFpppp_t)(void*, void*, void*, void*);

#define SUPER() \
    GO(XRecordEnableContextAsync, iFpppp_t) \
    GO(XRecordEnableContext, iFpppp_t)      \

typedef struct libxtst_my_s {
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
    // functions
} libxtst_my_t;

void* getXtstMy(library_t* lib)
{
    libxtst_my_t* my = (libxtst_my_t*)calloc(1, sizeof(libxtst_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeXtstMy(void* lib)
{
    (void)lib;
    //libxtst_my_t *my = (libxtst_my_t *)lib;
}

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// XRecordInterceptProc ...
#define GO(A)   \
static uintptr_t my_XRecordInterceptProc_fct_##A = 0;                   \
static void my_XRecordInterceptProc_##A(void* a, void* b)               \
{                                                                       \
    RunFunction(my_context, my_XRecordInterceptProc_fct_##A, 2, a, b);  \
}
SUPER()
#undef GO
static void* find_XRecordInterceptProc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_XRecordInterceptProc_fct_##A == (uintptr_t)fct) return my_XRecordInterceptProc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_XRecordInterceptProc_fct_##A == 0) {my_XRecordInterceptProc_fct_##A = (uintptr_t)fct; return my_XRecordInterceptProc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxtst XRecordInterceptProc callback\n");
    return NULL;
}

EXPORT int my_XRecordEnableContextAsync(x64emu_t* emu, void* display, void* context, void* cb, void* closure)
{
    (void)emu;
    library_t* lib = GetLibInternal(libxtstName);
    libxtst_my_t* my = (libxtst_my_t*)lib->priv.w.p2;
    return my->XRecordEnableContextAsync(display, context, find_XRecordInterceptProc_Fct(cb), closure);
}

EXPORT int my_XRecordEnableContext(x64emu_t* emu, void* display, void* context, void* cb, void* closure)
{
    (void)emu;
    library_t* lib = GetLibInternal(libxtstName);
    libxtst_my_t* my = (libxtst_my_t*)lib->priv.w.p2;
    return my->XRecordEnableContext(display, context, find_XRecordInterceptProc_Fct(cb), closure);
}

#define CUSTOM_INIT \
    lib->priv.w.p2 = getXtstMy(lib);   \
    lib->priv.w.needed = 2; \
    lib->priv.w.neededlibs = (char**)calloc(lib->priv.w.needed, sizeof(char*)); \
    lib->priv.w.neededlibs[0] = strdup("libX11.so.6"); \
    lib->priv.w.neededlibs[1] = strdup("libXext.so.6");

#define CUSTOM_FINI \
    freeXtstMy(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"
