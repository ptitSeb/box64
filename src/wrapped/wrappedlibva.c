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
#include "callback.h"
#include "librarian.h"
#include "box64context.h"
#include "emu/x64emu_private.h"
#include "myalign.h"

const char* libvaName = "libva.so.2";
#define LIBNAME libva

#define ADDED_FUNCTIONS()       \

#include "generated/wrappedlibvatypes.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)

// VAMessageCallback
#define GO(A)   \
static uintptr_t my_VAMessageCallback_fct_##A = 0;                          \
static void my_VAMessageCallback_##A(void* a, void* b)                      \
{                                                                           \
    RunFunctionFmt(my_VAMessageCallback_fct_##A, "pp", a, b);   \
}
SUPER()
#undef GO
static void* findVAMessageCallbackFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_VAMessageCallback_fct_##A == (uintptr_t)fct) return my_VAMessageCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_VAMessageCallback_fct_##A == 0) {my_VAMessageCallback_fct_##A = (uintptr_t)fct; return my_VAMessageCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxa VAMessageCallback callback\n");
    return NULL;
}
static void* reverse_VAMessageCallbackFct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_VAMessageCallback_##A == fct) return (void*)my_VAMessageCallback_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, vFpp, fct, 0, NULL);
}

#undef SUPER

EXPORT void* my_vaSetErrorCallback(x64emu_t* emu, void* dpy, void* f, void* ctx)
{
    return reverse_VAMessageCallbackFct(my->vaSetErrorCallback(dpy, findVAMessageCallbackFct(f), ctx));
}

EXPORT void* my_vaSetInfoCallback(x64emu_t* emu, void* dpy, void* f, void* ctx)
{
    return reverse_VAMessageCallbackFct(my->vaSetInfoCallback(dpy, findVAMessageCallbackFct(f), ctx));
}

#include "wrappedlib_init.h"
