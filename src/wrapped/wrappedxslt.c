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
#include "box64context.h"
#include "librarian.h"
#include "callback.h"

const char *xsltName =
#ifdef ANDROID
    "libxslt.so"
#else
    "libxslt.so.1"
#endif
    ;
#define LIBNAME xslt

#define ADDED_FUNCTIONS() \

#include "generated/wrappedxslttypes.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// xmlXPathFunction ...
#define GO(A)   \
static uintptr_t my_xmlXPathFunction_fct_##A = 0;                           \
static void my_xmlXPathFunction_##A(void* a, int b)                         \
{                                                                           \
    RunFunctionFmt(my_xmlXPathFunction_fct_##A, "pi", a, b);          \
}
SUPER()
#undef GO
static void* find_xmlXPathFunction_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlXPathFunction_fct_##A == (uintptr_t)fct) return my_xmlXPathFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlXPathFunction_fct_##A == 0) {my_xmlXPathFunction_fct_##A = (uintptr_t)fct; return my_xmlXPathFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxslt xmlXPathFunction callback\n");
    return NULL;
}
// xsltDocLoaderFunc ...
#define GO(A)   \
static uintptr_t my_xsltDocLoaderFunc_fct_##A = 0;                              \
static void* my_xsltDocLoaderFunc_##A(void* a, void* b, int c, void* d, int e)  \
{                                                                               \
    return (void*)RunFunctionFmt(my_xsltDocLoaderFunc_fct_##A, "ppipi", a, b, c, d, e);       \
}
SUPER()
#undef GO
static void* find_xsltDocLoaderFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xsltDocLoaderFunc_fct_##A == (uintptr_t)fct) return my_xsltDocLoaderFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xsltDocLoaderFunc_fct_##A == 0) {my_xsltDocLoaderFunc_fct_##A = (uintptr_t)fct; return my_xsltDocLoaderFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxslt xsltDocLoaderFunc callback\n");
    return NULL;
}
// xsltSecurityCheck ...
#define GO(A)   \
static uintptr_t my_xsltSecurityCheck_fct_##A = 0;                                          \
static int my_xsltSecurityCheck_##A(void* a, void* b, void* c)                              \
{                                                                                           \
    return (int)RunFunctionFmt(my_xsltSecurityCheck_fct_##A, "ppp", a, b, c);         \
}
SUPER()
#undef GO
static void* find_xsltSecurityCheck_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xsltSecurityCheck_fct_##A == (uintptr_t)fct) return my_xsltSecurityCheck_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xsltSecurityCheck_fct_##A == 0) {my_xsltSecurityCheck_fct_##A = (uintptr_t)fct; return my_xsltSecurityCheck_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxslt xsltSecurityCheck callback\n");
    return NULL;
}
// xsltSortFunc ...
#define GO(A)   \
static uintptr_t my_xsltSortFunc_fct_##A = 0;                               \
static void my_xsltSortFunc_##A(void* a, void* b, int c)                    \
{                                                                           \
    RunFunctionFmt(my_xsltSortFunc_fct_##A, "ppi", a, b, c);          \
}
SUPER()
#undef GO
static void* find_xsltSortFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xsltSortFunc_fct_##A == (uintptr_t)fct) return my_xsltSortFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xsltSortFunc_fct_##A == 0) {my_xsltSortFunc_fct_##A = (uintptr_t)fct; return my_xsltSortFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxslt xsltSortFunc callback\n");
    return NULL;
}
#undef SUPER

EXPORT int my_xsltRegisterExtModuleFunction(x64emu_t* emu, void* name, void* URI, void* f)
{
    return my->xsltRegisterExtModuleFunction(name, URI, find_xmlXPathFunction_Fct(f));
}

EXPORT void my_xsltSetLoaderFunc(x64emu_t* emu, void* f)
{
    my->xsltSetLoaderFunc(find_xsltDocLoaderFunc_Fct(f));
}

EXPORT int my_xsltRegisterExtFunction(x64emu_t* emu, void* ctx, void* name, void* uri, void* f)
{
    return my->xsltRegisterExtFunction(ctx, name, uri, find_xmlXPathFunction_Fct(f));
}

EXPORT int my_xsltSetSecurityPrefs(x64emu_t* emu, void* sec, void* option, void* f)
{
    return my->xsltSetSecurityPrefs(sec, option, find_xsltSecurityCheck_Fct(f));
}

EXPORT void my_xsltSetCtxtSortFunc(x64emu_t* emu, void* ctx, void* handler)
{
    return my->xsltSetCtxtSortFunc(ctx, find_xsltSortFunc_Fct(handler));
}

#include "wrappedlib_init.h"
