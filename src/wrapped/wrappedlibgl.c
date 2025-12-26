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
#include "gltools.h"

const char* libglName = "libGL.so.1";
#define ALTNAME "libGL.so"
#define LIBNAME libgl

#include "generated/wrappedlibgltypes.h"

#include "wrappercallback.h"

// FIXME: old wrapped* type of file, cannot use generated/wrappedlibgltypes.h

EXPORT void* my_glXGetProcAddress(x64emu_t* emu, void* name)
{
    pFp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glXGetProcAddress;
    const char* rname = (const char*)name;
    return getGLProcAddress(emu, NULL, (void*)fnc, rname);
}
EXPORT void* my_glXGetProcAddressARB(x64emu_t* emu, void* name)
{
    pFp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glXGetProcAddressARB;
    const char* rname = (const char*)name;
    return getGLProcAddress(emu, NULL, (void*)fnc, rname);
}

typedef int  (*iFi_t)(int);
typedef void (*vFpp_t)(void*, void*);
typedef void (*vFppp_t)(void*, void*, void*);
typedef void (*vFppi_t)(void*, void*, int);
typedef void*(*pFp_t)(void*);
typedef void (*debugProc_t)(int32_t, int32_t, uint32_t, int32_t, int32_t, void*, void*);

typedef struct gl_wrappers_s {
    glprocaddress_t      procaddress;
    kh_symbolmap_t      *glwrappers;    // the map of wrapper for glProcs (for GLX or SDL1/2)
    kh_symbolmap_t      *glmymap;       // link to the mysymbolmap of libGL
} gl_wrappers_t;

KHASH_MAP_INIT_INT64(gl_wrappers, gl_wrappers_t*)

static kh_gl_wrappers_t *gl_wrappers = NULL;

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
    RunFunctionFmt(my_debug_callback_fct_##A, "iiuiipp", a, b, c, d, e, f, g);                                    \
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
// egl_debug_callback ...
#define GO(A)   \
static uintptr_t my_egl_debug_callback_fct_##A = 0;                                                     \
    static void my_egl_debug_callback_##A(int a, void* b, int c, void* d, void* e, const char* f)       \
{                                                                                                       \
    RunFunctionFmt(my_egl_debug_callback_fct_##A, "ipippp", a, b, c, d, e, f);                          \
}
SUPER()
#undef GO
static void* find_egl_debug_callback_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_egl_debug_callback_fct_##A == (uintptr_t)fct) return my_egl_debug_callback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_egl_debug_callback_fct_##A == 0) {my_egl_debug_callback_fct_##A = (uintptr_t)fct; return my_egl_debug_callback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libGL egl_debug_callback callback\n");
    return NULL;
}
// program_callback ...
#define GO(A)                                                       \
static uintptr_t my_program_callback_fct_##A = 0;                   \
static void my_program_callback_##A(int32_t a, void* b)             \
{                                                                   \
    RunFunctionFmt(my_program_callback_fct_##A, "ip", a, b);  \
}
SUPER()
#undef GO
static void* find_program_callback_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_program_callback_fct_##A == (uintptr_t)fct) return my_program_callback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_program_callback_fct_##A == 0) {my_program_callback_fct_##A = (uintptr_t)fct; return my_program_callback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libGL program_callback callback\n");
    return NULL;
}
// set_blob_func ...
#define GO(A)                                                               \
static uintptr_t my_set_blob_func_fct_##A = 0;                              \
static void my_set_blob_func_##A(void* a, ssize_t b, void* c, ssize_t d)    \
{                                                                           \
    RunFunctionFmt(my_set_blob_func_fct_##A, "plpl", a, b, c, d);           \
}
SUPER()
#undef GO
static void* find_set_blob_func_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_set_blob_func_fct_##A == (uintptr_t)fct) return my_set_blob_func_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_set_blob_func_fct_##A == 0) {my_set_blob_func_fct_##A = (uintptr_t)fct; return my_set_blob_func_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libGL set_blob_func callback\n");
    return NULL;
}
// get_blob_func ...
#define GO(A)                                                                       \
static uintptr_t my_get_blob_func_fct_##A = 0;                                      \
static ssize_t my_get_blob_func_##A(void* a, ssize_t b, void* c, ssize_t d)         \
{                                                                                   \
    return (ssize_t)RunFunctionFmt(my_get_blob_func_fct_##A, "plpl", a, b, c, d);   \
}
SUPER()
#undef GO
static void* find_get_blob_func_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_get_blob_func_fct_##A == (uintptr_t)fct) return my_get_blob_func_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_get_blob_func_fct_##A == 0) {my_get_blob_func_fct_##A = (uintptr_t)fct; return my_get_blob_func_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libGL get_blob_func callback\n");
    return NULL;
}
#undef SUPER

#define PRE_INIT                                                                \
    if(BOX64ENV(libgl)) {                                                       \
        lib->w.lib = dlopen(BOX64ENV(libgl), RTLD_LAZY | RTLD_GLOBAL);          \
        lib->path = strdup(BOX64ENV(libgl));                                    \
    }

#include "wrappedlib_init.h"

// glDebugMessageCallback
EXPORT void my_glDebugMessageCallback(x64emu_t* emu, void* prod, void* param)
{
    vFpp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glDebugMessageCallback;
    fnc(find_debug_callback_Fct(prod), param);
}
// glDebugMessageCallbackARB
EXPORT void my_glDebugMessageCallbackARB(x64emu_t* emu, void* prod, void* param)
{
    vFpp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glDebugMessageCallbackARB;
    fnc(find_debug_callback_Fct(prod), param);
}
// glDebugMessageCallbackAMD
EXPORT void my_glDebugMessageCallbackAMD(x64emu_t* emu, void* prod, void* param)
{
    vFpp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glDebugMessageCallbackAMD;
    fnc(find_debug_callback_Fct(prod), param);
}
// glDebugMessageCallbackKHR
EXPORT void my_glDebugMessageCallbackKHR(x64emu_t* emu, void* prod, void* param)
{
    vFpp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glDebugMessageCallbackKHR;
    fnc(find_debug_callback_Fct(prod), param);
}
// eglDebugMessageControlKHR
EXPORT int my_eglDebugMessageControlKHR(x64emu_t* emu, void* prod, void* param)
{
    iFpp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->eglDebugMessageControlKHR;
    return fnc(find_debug_callback_Fct(prod), param);
}
// eglSetBlobCacheFuncsANDROID ...
EXPORT void my_eglSetBlobCacheFuncsANDROID(x64emu_t* emu, void* dpy, void* set, void* get)
{
    vFppp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->eglSetBlobCacheFuncsANDROID;
    fnc(dpy, find_set_blob_func_Fct(set), find_get_blob_func_Fct(get));
}
// glXSwapIntervalMESA ...
EXPORT int my_dummy_glXSwapIntervalMESA(int interval)
{
    return 5; // GLX_BAD_CONTEXT
}
EXPORT int my_glXSwapIntervalMESA(x64emu_t* emu, int interval)
{
    iFi_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glXSwapIntervalMESA;
    if(!fnc) fnc=my_dummy_glXSwapIntervalMESA;
    return fnc(interval);
}
// glXSwapIntervalEXT ...
EXPORT void my_dummy_glXSwapIntervalEXT(void* dpy, void* drawable, int interval) {}
EXPORT void my_glXSwapIntervalEXT(x64emu_t* emu, void* dpy, void* drawable, int interval)
{
    vFppi_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glXSwapIntervalEXT;
    if(!fnc) fnc=my_dummy_glXSwapIntervalEXT;
    fnc(dpy, drawable, interval);
}
// glProgramCallbackMESA ...
EXPORT void my_glProgramCallbackMESA(x64emu_t* emu, int t, void* f, void* data)
{
    vFipp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glProgramCallbackMESA;
    fnc(t, find_program_callback_Fct(f), data);
}
void* my_GetVkProcAddr(x64emu_t* emu, void* name, void*(*getaddr)(void*));  // defined in wrappedvulkan.c
// glGetVkProcAddrNV ...
EXPORT void* my_glGetVkProcAddrNV(x64emu_t* emu, void* name)
{
    pFp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glGetVkProcAddrNV;
    return my_GetVkProcAddr(emu, name, fnc);
}

gl_wrappers_t* getGLProcWrapper(box64context_t* context, glprocaddress_t procaddress)
{
    int cnt, ret;
    khint_t k;
    if(!gl_wrappers) {
        gl_wrappers = kh_init(gl_wrappers);
    }
    k = kh_put(gl_wrappers, gl_wrappers, (uintptr_t)procaddress, &ret);
    if(!ret)
        return kh_value(gl_wrappers, k);
    gl_wrappers_t* wrappers = kh_value(gl_wrappers, k) = (gl_wrappers_t*)calloc(1, sizeof(gl_wrappers_t));

    wrappers->procaddress = procaddress;
    wrappers->glwrappers = kh_init(symbolmap);
    // populates maps...
    cnt = sizeof(libglsymbolmap)/sizeof(map_onesymbol_t);
    for (int i=0; i<cnt; ++i) {
        k = kh_put(symbolmap, wrappers->glwrappers, libglsymbolmap[i].name, &ret);
        kh_value(wrappers->glwrappers, k).w = libglsymbolmap[i].w;
        kh_value(wrappers->glwrappers, k).resolved = 0;
    }
    // and the my_ symbols map
    cnt = sizeof(MAPNAME(mysymbolmap))/sizeof(map_onesymbol_t);
    for (int i=0; i<cnt; ++i) {
        k = kh_put(symbolmap, wrappers->glwrappers, libglmysymbolmap[i].name, &ret);
        kh_value(wrappers->glwrappers, k).w = libglmysymbolmap[i].w;
        kh_value(wrappers->glwrappers, k).resolved = 0;
    }
    // my_* map
    wrappers->glmymap = kh_init(symbolmap);
    cnt = sizeof(MAPNAME(mysymbolmap))/sizeof(map_onesymbol_t);
    for (int i=0; i<cnt; ++i) {
        k = kh_put(symbolmap, wrappers->glmymap, libglmysymbolmap[i].name, &ret);
        kh_value(wrappers->glmymap, k).w = libglmysymbolmap[i].w;
        kh_value(wrappers->glmymap, k).resolved = 0;
    }
    return wrappers;
}
void freeGLProcWrapper(box64context_t* context)
{
    if(!context)
        return;
    if(!gl_wrappers)
        return;
    gl_wrappers_t* wrappers;
    kh_foreach_value(gl_wrappers, wrappers,
        if(wrappers->glwrappers)
            kh_destroy(symbolmap, wrappers->glwrappers);
        if(wrappers->glmymap)
            kh_destroy(symbolmap, wrappers->glmymap);
        wrappers->glwrappers = NULL;
        wrappers->glmymap = NULL;
    );
    kh_destroy(gl_wrappers, gl_wrappers);
    gl_wrappers = NULL;
}

void* getGLProcAddress(x64emu_t* emu, const char* my, glprocaddress_t procaddr, const char* rname)
{
    if(!my) my = "my_";
    khint_t k;
    printf_dlsym(LOG_DEBUG, "Calling getGLProcAddress[%p](\"%s\") => ", procaddr, rname);
    gl_wrappers_t* wrappers = getGLProcWrapper(emu->context, procaddr);
    // check if glxprocaddress is filled, and search for lib and fill it if needed
    // get proc adress using actual glXGetProcAddress
    k = kh_get(symbolmap, wrappers->glmymap, rname);
    int is_my = (k==kh_end(wrappers->glmymap))?0:1;
    void* symbol = procaddr(rname);
    void* fnc = NULL;
    if(is_my) {
        char tmp[200];
        strcpy(tmp, my);
        strcat(tmp, rname);
        fnc = symbol;
        symbol = dlsym(emu->context->box64lib, tmp);
    }
    if(!symbol) {
        printf_dlsym_prefix(0, LOG_DEBUG, "%p\n", NULL);
        return NULL;    // easy
    }
    // check if alread bridged
    uintptr_t ret = CheckBridged2(emu->context->system, symbol, fnc);
    if(ret) {
        printf_dlsym_prefix(0, LOG_DEBUG, "%p\n", (void*)ret);
        return (void*)ret; // already bridged
    }
    // get wrapper
    k = kh_get(symbolmap, wrappers->glwrappers, rname);
    if(k==kh_end(wrappers->glwrappers) && strstr(rname, "ARB")==NULL) {
        // try again, adding ARB at the end if not present
        char tmp[200];
        strcpy(tmp, rname);
        strcat(tmp, "ARB");
        k = kh_get(symbolmap, wrappers->glwrappers, tmp);
    }
    if(k==kh_end(wrappers->glwrappers) && strstr(rname, "EXT")==NULL) {
        // try again, adding EXT at the end if not present
        char tmp[200];
        strcpy(tmp, rname);
        strcat(tmp, "EXT");
        k = kh_get(symbolmap, wrappers->glwrappers, tmp);
    }
    if(k==kh_end(wrappers->glwrappers)) {
        printf_dlsym_prefix(0, LOG_DEBUG, "%p\n", NULL);
        printf_dlsym_prefix(2, LOG_INFO, "Warning, no wrapper for %s\n", rname);
        return NULL;
    }
    symbol1_t* s = &kh_value(wrappers->glwrappers, k);
    const char* constname = kh_key(wrappers->glwrappers, k);
    ret = AddCheckBridge2(emu->context->system, s->w, symbol, fnc, 0, constname);
    printf_dlsym_prefix(0, LOG_DEBUG, "%p\n", (void*)ret);
    return (void*)ret;
}
