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
static library_t *my_lib = NULL;

typedef void    (*vFp_t)        (void*);
typedef int     (*iFppp_t)      (void*, void*, void*);

#define SUPER()                                 \
    GO(xsltRegisterExtModuleFunction, iFppp_t)  \
    GO(xsltSetLoaderFunc, vFp_t)                \

typedef struct xslt_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} xslt_my_t;

void* getXsltMy(library_t* lib)
{
    xslt_my_t* my = (xslt_my_t*)calloc(1, sizeof(xslt_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeXsltMy(void* lib)
{
    //xslt_my_t *my = (xslt_my_t *)lib;
}

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// xmlXPathFunction ...
#define GO(A)   \
static uintptr_t my_xmlXPathFunction_fct_##A = 0;                   \
static void my_xmlXPathFunction_##A(void* a, int b)                 \
{                                                                   \
    RunFunction(my_context, my_xmlXPathFunction_fct_##A, 2, a, b);  \
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
static uintptr_t my_xsltDocLoaderFunc_fct_##A = 0;                                          \
static void* my_xsltDocLoaderFunc_##A(void* a, void* b, int c, void* d, int e)              \
{                                                                                           \
    return (void*)RunFunction(my_context, my_xsltDocLoaderFunc_fct_##A, 5, a, b, c, d, e);  \
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

#undef SUPER

EXPORT int my_xsltRegisterExtModuleFunction(x64emu_t* emu, void* name, void* URI, void* f)
{
    xslt_my_t* my = (xslt_my_t*)my_lib->priv.w.p2;

    return my->xsltRegisterExtModuleFunction(name, URI, find_xmlXPathFunction_Fct(f));
}

EXPORT void my_xsltSetLoaderFunc(x64emu_t* emu, void* f)
{
    xslt_my_t* my = (xslt_my_t*)my_lib->priv.w.p2;

    my->xsltSetLoaderFunc(find_xsltDocLoaderFunc_Fct(f));
}

#define CUSTOM_INIT \
    lib->priv.w.p2 = getXsltMy(lib);    \
    my_lib = lib;

#define CUSTOM_FINI \
    freeXsltMy(lib->priv.w.p2); \
    free(lib->priv.w.p2);       \
    my_lib = NULL;

#include "wrappedlib_init.h"
