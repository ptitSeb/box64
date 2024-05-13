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
#include "myalign.h"

const char* libiceName = "libICE.so.6";
#define LIBNAME libice

#define ADDED_FUNCTIONS() \

#include "wrappedlibicetypes.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)

// WatchProc
#define GO(A)   \
static uintptr_t my_WatchProc_fct_##A = 0;                      \
static void my_WatchProc_##A(void* a, void* b, int c, void* d)  \
{                                                               \
    RunFunctionFmt(my_WatchProc_fct_##A, "ppip", a, b, c, d);   \
}
SUPER()
#undef GO
static void* findWatchProcFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_WatchProc_fct_##A == (uintptr_t)fct) return my_WatchProc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_WatchProc_fct_##A == 0) {my_WatchProc_fct_##A = (uintptr_t)fct; return my_WatchProc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libice WatchProc callback\n");
    return NULL;
}
// IceErrorHandler
#define GO(A)   \
static uintptr_t my_IceErrorHandler_fct_##A = 0;        \
static void my_IceErrorHandler_##A(void* a, int b, int c, unsigned long d, int e, int f, void* g)     \
{                                               \
    RunFunctionFmt(my_IceErrorHandler_fct_##A, "piiLiip", a, b, c, d, e, f, g);\
}
SUPER()
#undef GO
static void* findIceErrorHandlerFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_IceErrorHandler_fct_##A == (uintptr_t)fct) return my_IceErrorHandler_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_IceErrorHandler_fct_##A == 0) {my_IceErrorHandler_fct_##A = (uintptr_t)fct; return my_IceErrorHandler_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libICE IceErrorHandler callback\n");
    return NULL;
}
static void* reverse_IceErrorHandler_Fct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_IceErrorHandler_##A == fct) return (void*)my_IceErrorHandler_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, vFpiiLiip, fct, 0, NULL);
}
// IceIOErrorHandler
#define GO(A)   \
static uintptr_t my_IceIOErrorHandler_fct_##A = 0;        \
static void my_IceIOErrorHandler_##A(void* a)     \
{                                               \
    RunFunctionFmt(my_IceIOErrorHandler_fct_##A, "p", a);\
}
SUPER()
#undef GO
static void* findIceIOErrorHandlerFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_IceIOErrorHandler_fct_##A == (uintptr_t)fct) return my_IceIOErrorHandler_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_IceIOErrorHandler_fct_##A == 0) {my_IceIOErrorHandler_fct_##A = (uintptr_t)fct; return my_IceIOErrorHandler_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libICE IceIOErrorHandler callback\n");
    return NULL;
}
static void* reverse_IceIOErrorHandler_Fct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_IceIOErrorHandler_##A == fct) return (void*)my_IceIOErrorHandler_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, vFp, fct, 0, NULL);
}

#undef SUPER

EXPORT int my_IceAddConnectionWatch(x64emu_t* emu, void* f, void* data)
{
    return my->IceAddConnectionWatch(findWatchProcFct(f), data);
}

EXPORT int my_IceRemoveConnectionWatch(x64emu_t* emu, void* f, void* data)
{
    return my->IceRemoveConnectionWatch(findWatchProcFct(f), data);
}

EXPORT void* my_IceSetErrorHandler(x64emu_t* emu, void* f)
{
    return reverse_IceErrorHandler_Fct(my->IceSetErrorHandler(findIceErrorHandlerFct(f)));
}

EXPORT void* my_IceSetIOErrorHandler(x64emu_t* emu, void* f)
{
    return reverse_IceIOErrorHandler_Fct(my->IceSetIOErrorHandler(findIceIOErrorHandlerFct(f)));
}

#include "wrappedlib_init.h"

