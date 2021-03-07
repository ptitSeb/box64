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

extern char* libGL;

const char* libglName = "libGL.so.1";
#define LIBNAME libgl

void fillGLProcWrapper(box64context_t*);
void freeProcWrapper(kh_symbolmap_t** symbolmap);

EXPORT void* my_glXGetProcAddress(x64emu_t* emu, void* name) 
{
    khint_t k;
    const char* rname = (const char*)name;
    if(dlsym_error && box64_log<LOG_DEBUG) printf_log(LOG_NONE, "Calling glXGetProcAddress(\"%s\") => ", rname);
    if(!emu->context->glwrappers)
        fillGLProcWrapper(emu->context);
    // check if glxprocaddress is filled, and search for lib and fill it if needed
    // get proc adress using actual glXGetProcAddress
    k = kh_get(symbolmap, emu->context->glmymap, rname);
    int is_my = (k==kh_end(emu->context->glmymap))?0:1;
    void* symbol;
    if(is_my) {
        // try again, by using custom "my_" now...
        char tmp[200];
        strcpy(tmp, "my_");
        strcat(tmp, rname);
        symbol = dlsym(emu->context->box64lib, tmp);
    } else 
        symbol = emu->context->glxprocaddress(rname);
    if(!symbol) {
        if(dlsym_error && box64_log<LOG_DEBUG) printf_log(LOG_NONE, "%p\n", NULL);
        return NULL;    // easy
    }
    // check if alread bridged
    uintptr_t ret = CheckBridged(emu->context->system, symbol);
    if(ret) {
        if(dlsym_error && box64_log<LOG_DEBUG) printf_log(LOG_NONE, "%p\n", (void*)ret);
        return (void*)ret; // already bridged
    }
    // get wrapper    
    k = kh_get(symbolmap, emu->context->glwrappers, rname);
    if(k==kh_end(emu->context->glwrappers) && strstr(rname, "ARB")==NULL) {
        // try again, adding ARB at the end if not present
        char tmp[200];
        strcpy(tmp, rname);
        strcat(tmp, "ARB");
        k = kh_get(symbolmap, emu->context->glwrappers, tmp);
    }
    if(k==kh_end(emu->context->glwrappers) && strstr(rname, "EXT")==NULL) {
        // try again, adding EXT at the end if not present
        char tmp[200];
        strcpy(tmp, rname);
        strcat(tmp, "EXT");
        k = kh_get(symbolmap, emu->context->glwrappers, tmp);
    }
    if(k==kh_end(emu->context->glwrappers)) {
        if(dlsym_error && box64_log<LOG_DEBUG) printf_log(LOG_NONE, "%p\n", NULL);
        if(dlsym_error && box64_log<LOG_INFO) printf_log(LOG_NONE, "Warning, no wrapper for %s\n", rname);
        return NULL;
    }
    AddOffsetSymbol(emu->context->maplib, symbol, rname);
    ret = AddBridge(emu->context->system, kh_value(emu->context->glwrappers, k), symbol, 0);
    if(dlsym_error && box64_log<LOG_DEBUG) printf_log(LOG_NONE, "%p\n", (void*)ret);
    return (void*)ret;

}
EXPORT void* my_glXGetProcAddressARB(x64emu_t* emu, void* name) __attribute__((alias("my_glXGetProcAddress")));

typedef int  (*iFi_t)(int);
typedef void (*vFpp_t)(void*, void*);
typedef void (*debugProc_t)(int32_t, int32_t, uint32_t, int32_t, int32_t, void*, void*);

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// debug_callback ...
#define GO(A)   \
static uintptr_t my_debug_callback_fct_##A = 0;                                                                         \
static void my_debug_callback_##A(int32_t a, int32_t b, uint32_t c, int32_t d, int32_t e, const char* f, const void* g) \
{                                                                                                                       \
    RunFunction(my_context, my_debug_callback_fct_##A, 7, a, b, c, d, e, f, g);                                         \
}
SUPER()
#undef GO
static void* find_debug_callback_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_debug_callback_fct_##A == (uintptr_t)fct) return my_debug_callback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_debug_callback_fct_##A == 0) {my_debug_callback_fct_##A = (uintptr_t)fct; return my_debug_callback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libGL debug_callback callback\n");
    return NULL;
}
#undef SUPER

EXPORT void my_glDebugMessageCallback(x64emu_t* emu, void* prod, void* param)
{
    static vFpp_t DebugMessageCallback = NULL;
    static int init = 1;
    if(init) {
        DebugMessageCallback = emu->context->glxprocaddress("glDebugMessageCallback");
        init = 0;
    }
    if(!DebugMessageCallback)
        return;
    DebugMessageCallback(find_debug_callback_Fct(prod), param);
}
EXPORT void my_glDebugMessageCallbackARB(x64emu_t* emu, void* prod, void* param) __attribute__((alias("my_glDebugMessageCallback")));

EXPORT int my_glXSwapIntervalMESA(int interval)
{
    static iFi_t SwapIntervalMESA = NULL;
    static int init = 1;
    if(init) {
        SwapIntervalMESA = my_context->glxprocaddress("glXSwapIntervalMESA");
        init = 0;
    }
    if(!SwapIntervalMESA)
        return 0;
    return SwapIntervalMESA(interval);
}

#define PRE_INIT if(libGL) {lib->priv.w.lib = dlopen(libGL, RTLD_LAZY | RTLD_GLOBAL); lib->path = strdup(libGL);} else
#define CUSTOM_INIT \
    lib->priv.w.priv = dlsym(lib->priv.w.lib, "glXGetProcAddress"); \
    box64->glxprocaddress = lib->priv.w.priv;


#include "wrappedlib_init.h"

void fillGLProcWrapper(box64context_t* context)
{
    int cnt, ret;
    khint_t k;
    kh_symbolmap_t * symbolmap = kh_init(symbolmap);
    // populates maps...
    cnt = sizeof(libglsymbolmap)/sizeof(map_onesymbol_t);
    for (int i=0; i<cnt; ++i) {
        k = kh_put(symbolmap, symbolmap, libglsymbolmap[i].name, &ret);
        kh_value(symbolmap, k) = libglsymbolmap[i].w;
    }
    // and the my_ symbols map
    cnt = sizeof(MAPNAME(mysymbolmap))/sizeof(map_onesymbol_t);
    for (int i=0; i<cnt; ++i) {
        k = kh_put(symbolmap, symbolmap, libglmysymbolmap[i].name, &ret);
        kh_value(symbolmap, k) = libglmysymbolmap[i].w;
    }
    context->glwrappers = symbolmap;
    // my_* map
    symbolmap = kh_init(symbolmap);
    cnt = sizeof(MAPNAME(mysymbolmap))/sizeof(map_onesymbol_t);
    for (int i=0; i<cnt; ++i) {
        k = kh_put(symbolmap, symbolmap, libglmysymbolmap[i].name, &ret);
        kh_value(symbolmap, k) = libglmysymbolmap[i].w;
    }
    context->glmymap = symbolmap;
}
void freeGLProcWrapper(box64context_t* context)
{
    if(!context)
        return;
    if(context->glwrappers)
        kh_destroy(symbolmap, context->glwrappers);
    if(context->glmymap)
        kh_destroy(symbolmap, context->glmymap);
    context->glwrappers = NULL;
    context->glmymap = NULL;
}
