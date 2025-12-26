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

const char* libglxnvidiaName = "libGLX_nvidia.so.0";
#define ALTNAME "libGLX_nvidia.so"
#define LIBNAME libglxnvidia

#include "generated/wrappedlibglxnvidiatypes.h"

#include "wrappercallback.h"

// FIXME: old wrapped* type of file, cannot use generated/wrappedlibgltypes.h

EXPORT void* mynv_glXGetProcAddress(x64emu_t* emu, void* name)
{
    pFp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glXGetProcAddress;
    khint_t k;
    const char* rname = (const char*)name;
    return getGLProcAddress(emu, "mynv_", (void*)fnc, rname);
}
EXPORT void* mynv_glXGetProcAddressARB(x64emu_t* emu, void* name) __attribute__((alias("mynv_glXGetProcAddress")));

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
#define ALTMY mynv
#include "wrappedlib_init.h"

// glDebugMessageCallback
static void mynv_glDebugMessageCallback(x64emu_t* emu, void* prod, void* param)
{
    vFpp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glDebugMessageCallback;
    fnc(find_debug_callback_Fct(prod), param);
}
// glDebugMessageCallbackARB
static void mynv_glDebugMessageCallbackARB(x64emu_t* emu, void* prod, void* param)
{
    vFpp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glDebugMessageCallbackARB;
    fnc(find_debug_callback_Fct(prod), param);
}
// glDebugMessageCallbackAMD
static void mynv_glDebugMessageCallbackAMD(x64emu_t* emu, void* prod, void* param)
{
    vFpp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glDebugMessageCallbackAMD;
    fnc(find_debug_callback_Fct(prod), param);
}
// glDebugMessageCallbackKHR
static void mynv_glDebugMessageCallbackKHR(x64emu_t* emu, void* prod, void* param)
{
    vFpp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glDebugMessageCallbackKHR;
    fnc(find_debug_callback_Fct(prod), param);
}
// eglDebugMessageControlKHR
static int mynv_eglDebugMessageControlKHR(x64emu_t* emu, void* prod, void* param)
{
    iFpp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->eglDebugMessageControlKHR;
    return fnc(find_debug_callback_Fct(prod), param);
}
// eglSetBlobCacheFuncsANDROID ...
static void mynv_eglSetBlobCacheFuncsANDROID(x64emu_t* emu, void* dpy, void* set, void* get)              \
{                                                                                                           \
    vFppp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->eglSetBlobCacheFuncsANDROID;
    fnc(dpy, find_set_blob_func_Fct(set), find_get_blob_func_Fct(get));
}
// glXSwapIntervalMESA ...
static int mynv_dummy_glXSwapIntervalMESA(int interval)
{
    return 5; // GLX_BAD_CONTEXT
}
static int mynv_glXSwapIntervalMESA(x64emu_t* emu, int interval)
{
    iFi_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glXSwapIntervalMESA;
    if(!fnc) fnc=mynv_dummy_glXSwapIntervalMESA;
    return fnc(interval);
}
// glXSwapIntervalEXT ...
static void mynv_dummy_glXSwapIntervalEXT(void* dpy, void* drawable, int interval) {}
static void mynv_glXSwapIntervalEXT(x64emu_t* emu, void* dpy, void* drawable, int interval)
{
    vFppi_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glXSwapIntervalEXT;
    if(!fnc) fnc=mynv_dummy_glXSwapIntervalEXT;
    fnc(dpy, drawable, interval);
}
// glProgramCallbackMESA ...
static void mynv_glProgramCallbackMESA(x64emu_t* emu, int t, void* f, void* data)
{
    vFipp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glProgramCallbackMESA;
    fnc(t, find_program_callback_Fct(f), data);
}
void* my_GetVkProcAddr(x64emu_t* emu, void* name, void*(*getaddr)(void*));  // defined in wrappedvulkan.c
// glGetVkProcAddrNV ...
static void* mynv_glGetVkProcAddrNV(x64emu_t* emu, void* name)
{
    pFp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glGetVkProcAddrNV;
    return my_GetVkProcAddr(emu, name, fnc);
}
