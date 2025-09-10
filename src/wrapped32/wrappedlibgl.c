#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper32.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "box32context.h"
#include "librarian.h"
#include "callback.h"
#include "gltools.h"
#include "libtools/my_x11_conv.h"

extern const char* libglName;
#define LIBNAME libgl

#include "generated/wrappedlibgltypes32.h"

#include "wrappercallback32.h"

void* getDisplay(void* d); // define in 32bits wrappedx11.c

// FIXME: old wrapped* type of file, cannot use generated/wrappedlibgltypes.h
void* getGLProcAddress32(x64emu_t* emu, glprocaddress_t procaddr, const char* rname);
EXPORT void* my32_glXGetProcAddress(x64emu_t* emu, void* name)
{
    khint_t k;
    const char* rname = (const char*)name;
    return getGLProcAddress32(emu, my_lib->w.priv, rname);
}
EXPORT void* my32_glXGetProcAddressARB(x64emu_t* emu, void* name) __attribute__((alias("my32_glXGetProcAddress")));

typedef int  (*iFi_t)(int);
typedef void (*vFpp_t)(void*, void*);
typedef void*(*pFpp_t)(void*, void*);
typedef void (*vFip_t)(int, void*);
typedef void (*vFppp_t)(void*, void*, void*);
typedef void (*vFppi_t)(void*, void*, int);
typedef void*(*pFpip_t)(void*, int, void*);
typedef void (*vFuip_t)(uint32_t, int, void*);
typedef void*(*pFp_t)(void*);
typedef void (*vFuipp_t)(uint32_t, int, void*, void*);
typedef void*(*pFpipp_t)(void*, int, void*, void*);
typedef void*(*pFpppi_t)(void*, void*, void*, int);
typedef void (*vFuipu_t)(uint32_t, int, void*, uint32_t);
typedef void (*vFupupi_t)(uint32_t, void*, uint32_t, void*, int);
typedef void (*vFuippp_t)(uint32_t, int, void*, void*, void*);
typedef void (*vFuuippp_t)(uint32_t, uint32_t, int, void*, void*, void*);
typedef void (*vFupupip_t)(uint32_t, void*, uint32_t, void*, int, void*);
typedef void (*vFppupui_t)(void*, void*, uint32_t, void*, uint32_t, int);
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

// glDebugMessageCallback ...
#define GO(A)                                                                       \
static vFpp_t my32_glDebugMessageCallback_fct_##A = NULL;                             \
static void my32_glDebugMessageCallback_##A(x64emu_t* emu, void* prod, void* param)   \
{                                                                                   \
    if(!my32_glDebugMessageCallback_fct_##A)                                          \
        return;                                                                     \
    my32_glDebugMessageCallback_fct_##A(find_debug_callback_Fct(prod), param);        \
}
SUPER()
#undef GO
static void* find_glDebugMessageCallback_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my32_glDebugMessageCallback_fct_##A == (vFpp_t)fct) return my32_glDebugMessageCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_glDebugMessageCallback_fct_##A == 0) {my32_glDebugMessageCallback_fct_##A = (vFpp_t)fct; return my32_glDebugMessageCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libGL glDebugMessageCallback callback\n");
    return NULL;
}
// glDebugMessageCallbackARB ...
#define GO(A)                                                                       \
static vFpp_t my32_glDebugMessageCallbackARB_fct_##A = NULL;                             \
static void my32_glDebugMessageCallbackARB_##A(x64emu_t* emu, void* prod, void* param)   \
{                                                                                   \
    if(!my32_glDebugMessageCallbackARB_fct_##A)                                          \
        return;                                                                     \
    my32_glDebugMessageCallbackARB_fct_##A(find_debug_callback_Fct(prod), param);        \
}
SUPER()
#undef GO
static void* find_glDebugMessageCallbackARB_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my32_glDebugMessageCallbackARB_fct_##A == (vFpp_t)fct) return my32_glDebugMessageCallbackARB_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_glDebugMessageCallbackARB_fct_##A == 0) {my32_glDebugMessageCallbackARB_fct_##A = (vFpp_t)fct; return my32_glDebugMessageCallbackARB_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libGL glDebugMessageCallbackARB callback\n");
    return NULL;
}
// glDebugMessageCallbackAMD ...
#define GO(A)                                                                       \
static vFpp_t my32_glDebugMessageCallbackAMD_fct_##A = NULL;                             \
static void my32_glDebugMessageCallbackAMD_##A(x64emu_t* emu, void* prod, void* param)   \
{                                                                                   \
    if(!my32_glDebugMessageCallbackAMD_fct_##A)                                          \
        return;                                                                     \
    my32_glDebugMessageCallbackAMD_fct_##A(find_debug_callback_Fct(prod), param);        \
}
SUPER()
#undef GO
static void* find_glDebugMessageCallbackAMD_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my32_glDebugMessageCallbackAMD_fct_##A == (vFpp_t)fct) return my32_glDebugMessageCallbackAMD_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_glDebugMessageCallbackAMD_fct_##A == 0) {my32_glDebugMessageCallbackAMD_fct_##A = (vFpp_t)fct; return my32_glDebugMessageCallbackAMD_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libGL glDebugMessageCallbackAMD callback\n");
    return NULL;
}
// glDebugMessageCallbackKHR ...
#define GO(A)                                                                       \
static vFpp_t my32_glDebugMessageCallbackKHR_fct_##A = NULL;                             \
static void my32_glDebugMessageCallbackKHR_##A(x64emu_t* emu, void* prod, void* param)   \
{                                                                                   \
    if(!my32_glDebugMessageCallbackKHR_fct_##A)                                          \
        return;                                                                     \
    my32_glDebugMessageCallbackKHR_fct_##A(find_debug_callback_Fct(prod), param);        \
}
SUPER()
#undef GO
static void* find_glDebugMessageCallbackKHR_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my32_glDebugMessageCallbackKHR_fct_##A == (vFpp_t)fct) return my32_glDebugMessageCallbackKHR_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_glDebugMessageCallbackKHR_fct_##A == 0) {my32_glDebugMessageCallbackKHR_fct_##A = (vFpp_t)fct; return my32_glDebugMessageCallbackKHR_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libGL glDebugMessageCallbackKHR callback\n");
    return NULL;
}
// eglDebugMessageControlKHR ...
#define GO(A)                                                                           \
static vFpp_t my_eglDebugMessageControlKHR_fct_##A = NULL;                             \
static void my_eglDebugMessageControlKHR_##A(x64emu_t* emu, void* prod, void* param)   \
{                                                                                       \
    if(!my_eglDebugMessageControlKHR_fct_##A)                                          \
        return;                                                                         \
    my_eglDebugMessageControlKHR_fct_##A(find_egl_debug_callback_Fct(prod), param);    \
}
SUPER()
#undef GO
static void* find_eglDebugMessageControlKHR_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my_eglDebugMessageControlKHR_fct_##A == (vFpp_t)fct) return my_eglDebugMessageControlKHR_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_eglDebugMessageControlKHR_fct_##A == 0) {my_eglDebugMessageControlKHR_fct_##A = (vFpp_t)fct; return my_eglDebugMessageControlKHR_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libGL eglDebugMessageControlKHR callback\n");
    return NULL;
}
// eglSetBlobCacheFuncsANDROID ...
#define GO(A)                                                                                               \
static vFppp_t my_eglSetBlobCacheFuncsANDROID_fct_##A = NULL;                                               \
static void my_eglSetBlobCacheFuncsANDROID_##A(x64emu_t* emu, void* dpy, void* set, void* get)              \
{                                                                                                           \
    if(!my_eglSetBlobCacheFuncsANDROID_fct_##A)                                                             \
        return;                                                                                             \
    my_eglSetBlobCacheFuncsANDROID_fct_##A(dpy, find_set_blob_func_Fct(set), find_get_blob_func_Fct(get));  \
}
SUPER()
#undef GO
static void* find_eglSetBlobCacheFuncsANDROID_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my_eglSetBlobCacheFuncsANDROID_fct_##A == (vFppp_t)fct) return my_eglSetBlobCacheFuncsANDROID_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_eglSetBlobCacheFuncsANDROID_fct_##A == 0) {my_eglSetBlobCacheFuncsANDROID_fct_##A = (vFppp_t)fct; return my_eglSetBlobCacheFuncsANDROID_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libGL eglSetBlobCacheFuncsANDROID callback\n");
    return NULL;
}
// glXSwapIntervalMESA ...
#define GO(A)                                           \
static iFi_t my32_glXSwapIntervalMESA_fct_##A = NULL;     \
static int my32_glXSwapIntervalMESA_##A(int interval)     \
{                                                       \
    if(!my32_glXSwapIntervalMESA_fct_##A)                 \
        return 0;                                       \
    return my32_glXSwapIntervalMESA_fct_##A(interval);    \
}
SUPER()
#undef GO

static int my_dummy32_glXSwapIntervalMESA(int interval)
{
    return 5; // GLX_BAD_CONTEXT
}

static void* find_glXSwapIntervalMESA_Fct(void* fct)
{
    if(!fct) return my_dummy32_glXSwapIntervalMESA;
    #define GO(A) if(my32_glXSwapIntervalMESA_fct_##A == (iFi_t)fct) return my32_glXSwapIntervalMESA_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_glXSwapIntervalMESA_fct_##A == 0) {my32_glXSwapIntervalMESA_fct_##A = (iFi_t)fct; return my32_glXSwapIntervalMESA_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libGL glXSwapIntervalMESA callback\n");
    return NULL;
}

// glXSwapIntervalEXT ...
#define GO(A)                                                                   \
static vFppi_t my32_glXSwapIntervalEXT_fct_##A = NULL;                            \
static void my32_glXSwapIntervalEXT_##A(void* dpy, void* drawable, int interval)  \
{                                                                               \
    if (!my32_glXSwapIntervalEXT_fct_##A)                                         \
        return;                                                                 \
    my32_glXSwapIntervalEXT_fct_##A(dpy, drawable, interval);                     \
}
SUPER()
#undef GO

static void my_dummy32_glXSwapIntervalEXT(void* dpy, void* drawable, int interval) {}

static void* find_glXSwapIntervalEXT_Fct(void* fct)
{
    if(!fct) return my_dummy32_glXSwapIntervalEXT;
    #define GO(A) if(my32_glXSwapIntervalEXT_fct_##A == (vFppi_t)fct) return my32_glXSwapIntervalEXT_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_glXSwapIntervalEXT_fct_##A == 0) {my32_glXSwapIntervalEXT_fct_##A = (vFppi_t)fct; return my32_glXSwapIntervalEXT_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libGL glXSwapIntervalEXT callback\n");
    return NULL;
}

// glProgramCallbackMESA ...
#define GO(A)                                                               \
static vFpp_t my32_glProgramCallbackMESA_fct_##A = NULL;                      \
static void my32_glProgramCallbackMESA_##A(x64emu_t* emu, void* f, void* data)\
{                                                                           \
    if(!my32_glProgramCallbackMESA_fct_##A)                                   \
        return;                                                             \
    my32_glProgramCallbackMESA_fct_##A(find_program_callback_Fct(f), data);   \
}
SUPER()
#undef GO
static void* find_glProgramCallbackMESA_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my32_glProgramCallbackMESA_fct_##A == (vFpp_t)fct) return my32_glProgramCallbackMESA_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_glProgramCallbackMESA_fct_##A == 0) {my32_glProgramCallbackMESA_fct_##A = (vFpp_t)fct; return my32_glProgramCallbackMESA_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libGL glProgramCallbackMESA callback\n");
    return NULL;
}
void* my_GetVkProcAddr(x64emu_t* emu, void* name, void*(*getaddr)(void*));  // defined in wrappedvulkan.c
// glGetVkProcAddrNV ...
#define GO(A)                                                           \
static pFp_t my32_glGetVkProcAddrNV_fct_##A = NULL;                       \
static void* my32_glGetVkProcAddrNV_##A(x64emu_t* emu, void* name)        \
{                                                                       \
    if(!my32_glGetVkProcAddrNV_fct_##A)                                   \
        return NULL;                                                    \
    return my_GetVkProcAddr(emu, name, my32_glGetVkProcAddrNV_fct_##A);   \
}
SUPER()
#undef GO
static void* find_glGetVkProcAddrNV_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my32_glGetVkProcAddrNV_fct_##A == (pFp_t)fct) return my32_glGetVkProcAddrNV_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_glGetVkProcAddrNV_fct_##A == 0) {my32_glGetVkProcAddrNV_fct_##A = (pFp_t)fct; return my32_glGetVkProcAddrNV_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libGL glGetVkProcAddrNV callback\n");
    return NULL;
}
// glShaderSource ...
#define GO(A)                                                                                               \
static vFuipp_t my32_glShaderSource_fct_##A = NULL;                                                         \
static void my32_glShaderSource_##A(x64emu_t* emu, uint32_t shader, int count, ptr_t* string, int* length)  \
{                                                                                                           \
    if(!my32_glShaderSource_fct_##A)                                                                        \
        return;                                                                                             \
    char* str[count];                                                                                       \
    if(string) for(int i=0; i<count; ++i) str[i] = from_ptrv(string[i]);                                    \
    my32_glShaderSource_fct_##A (shader, count, string?str:NULL, length);                                   \
}
SUPER()
#undef GO
static void* find_glShaderSource_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my32_glShaderSource_fct_##A == (vFuipp_t)fct) return my32_glShaderSource_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_glShaderSource_fct_##A == 0) {my32_glShaderSource_fct_##A = (vFuipp_t)fct; return my32_glShaderSource_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libGL glShaderSource callback\n");
    return NULL;
}
static void* find_glShaderSourceARB_Fct(void* fct)
{
    return find_glShaderSource_Fct(fct);
}
// glXChooseFBConfig ...
#define GO(A)                                                                                               \
static pFpipp_t my32_glXChooseFBConfig_fct_##A = NULL;                                                      \
static void* my32_glXChooseFBConfig_##A(x64emu_t* emu, void* dpy, int screen, int* list, int* nelement)     \
{                                                                                                           \
    if(!my32_glXChooseFBConfig_fct_##A)                                                                     \
        return NULL;                                                                                        \
    void** res = my32_glXChooseFBConfig_fct_##A (dpy, screen, list, nelement);                              \
    if(!res)                                                                                                \
        return NULL;                                                                                        \
    ptr_t* fbconfig = (ptr_t*)res;                                                                          \
    for(int i=0; i<*nelement; ++i)                                                                          \
        fbconfig[i] = to_ptrv(res[i]);                                                                      \
    return res;                                                                                             \
}
SUPER()
#undef GO
static void* find_glXChooseFBConfig_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my32_glXChooseFBConfig_fct_##A == (pFpipp_t)fct) return my32_glXChooseFBConfig_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_glXChooseFBConfig_fct_##A == 0) {my32_glXChooseFBConfig_fct_##A = (pFpipp_t)fct; return my32_glXChooseFBConfig_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libGL glXChooseFBConfig callback\n");
    return NULL;
}
// glXChooseFBConfigSGIX ...
#define GO(A)                                                                                               \
static pFpipp_t my32_glXChooseFBConfigSGIX_fct_##A = NULL;                                                      \
static void* my32_glXChooseFBConfigSGIX_##A(x64emu_t* emu, void* dpy, int screen, int* list, int* nelement)     \
{                                                                                                           \
    if(!my32_glXChooseFBConfigSGIX_fct_##A)                                                                     \
        return NULL;                                                                                        \
    void** res = my32_glXChooseFBConfigSGIX_fct_##A (dpy, screen, list, nelement);                              \
    if(!res)                                                                                                \
        return NULL;                                                                                        \
    ptr_t* fbconfig = (ptr_t*)res;                                                                          \
    for(int i=0; i<*nelement; ++i)                                                                          \
        fbconfig[i] = to_ptrv(res[i]);                                                                      \
    return res;                                                                                             \
}
SUPER()
#undef GO
static void* find_glXChooseFBConfigSGIX_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my32_glXChooseFBConfigSGIX_fct_##A == (pFpipp_t)fct) return my32_glXChooseFBConfigSGIX_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_glXChooseFBConfigSGIX_fct_##A == 0) {my32_glXChooseFBConfigSGIX_fct_##A = (pFpipp_t)fct; return my32_glXChooseFBConfigSGIX_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libGL glXChooseFBConfigSGIX callback\n");
    return NULL;
}
// glXGetVisualFromFBConfig ...
#define GO(A)                                                                                               \
static pFpp_t my32_glXGetVisualFromFBConfig_fct_##A = NULL;                                                 \
static void* my32_glXGetVisualFromFBConfig_##A(x64emu_t* emu, void* dpy, void* config)                      \
{                                                                                                           \
    if(!my32_glXGetVisualFromFBConfig_fct_##A)                                                              \
        return NULL;                                                                                        \
    void* res = my32_glXGetVisualFromFBConfig_fct_##A (dpy, config);                                        \
    if(!res)                                                                                                \
        return NULL;                                                                                        \
    my_XVisualInfo_32_t* vinfo = (my_XVisualInfo_32_t*)res;                                                 \
    convert_XVisualInfo_to_32(dpy, vinfo, res);                                                                  \
    return vinfo;                                                                                           \
}
SUPER()
#undef GO
static void* find_glXGetVisualFromFBConfig_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my32_glXGetVisualFromFBConfig_fct_##A == (pFpp_t)fct) return my32_glXGetVisualFromFBConfig_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_glXGetVisualFromFBConfig_fct_##A == 0) {my32_glXGetVisualFromFBConfig_fct_##A = (pFpp_t)fct; return my32_glXGetVisualFromFBConfig_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libGL glXGetVisualFromFBConfig callback\n");
    return NULL;
}
// glXGetVisualFromFBConfigSGIX ...
#define GO(A)                                                                                           \
static pFpp_t my32_glXGetVisualFromFBConfigSGIX_fct_##A = NULL;                                         \
static void* my32_glXGetVisualFromFBConfigSGIX_##A(x64emu_t* emu, void* dpy, void* config)              \
{                                                                                                       \
    if(!my32_glXGetVisualFromFBConfigSGIX_fct_##A)                                                      \
        return NULL;                                                                                    \
    void* res = my32_glXGetVisualFromFBConfigSGIX_fct_##A (dpy, config);                                \
    if(!res)                                                                                            \
        return NULL;                                                                                    \
    my_XVisualInfo_32_t* vinfo = (my_XVisualInfo_32_t*)res;                                             \
    convert_XVisualInfo_to_32(dpy, vinfo, res);                                                         \
    return vinfo;                                                                                       \
}
SUPER()
#undef GO
static void* find_glXGetVisualFromFBConfigSGIX_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my32_glXGetVisualFromFBConfigSGIX_fct_##A == (pFpp_t)fct) return my32_glXGetVisualFromFBConfigSGIX_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_glXGetVisualFromFBConfigSGIX_fct_##A == 0) {my32_glXGetVisualFromFBConfigSGIX_fct_##A = (pFpp_t)fct; return my32_glXGetVisualFromFBConfigSGIX_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libGL glXGetVisualFromFBConfigSGIX callback\n");
    return NULL;
}
// glXChooseVisual ...
#define GO(A)                                                                                      \
static pFpip_t my32_glXChooseVisual_fct_##A = NULL;                                                \
static void* my32_glXChooseVisual_##A(x64emu_t* emu, void* dpy, int screen, int* attr)             \
{                                                                                                  \
    if(!my32_glXChooseVisual_fct_##A)                                                              \
        return NULL;                                                                               \
    void* res = my32_glXChooseVisual_fct_##A (dpy, screen, attr);                                  \
    if(!res)                                                                                       \
        return NULL;                                                                               \
    my_XVisualInfo_32_t* vinfo = (my_XVisualInfo_32_t*)res;                                        \
    convert_XVisualInfo_to_32(dpy, vinfo, res);                                                         \
    return vinfo;                                                                                  \
}
SUPER()
#undef GO
static void* find_glXChooseVisual_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my32_glXChooseVisual_fct_##A == (pFpip_t)fct) return my32_glXChooseVisual_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_glXChooseVisual_fct_##A == 0) {my32_glXChooseVisual_fct_##A = (pFpip_t)fct; return my32_glXChooseVisual_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libGL glXChooseVisual callback\n");
    return NULL;
}
// glXCreateContext ...
#define GO(A)                                                                                                           \
static pFpppi_t my32_glXCreateContext_fct_##A = NULL;                                                                   \
static void* my32_glXCreateContext_##A(x64emu_t* emu, void* dpy, my_XVisualInfo_32_t* info, void* shared, int direct)   \
{                                                                                                                       \
    if(!my32_glXCreateContext_fct_##A)                                                                                  \
        return NULL;                                                                                                    \
    my_XVisualInfo_t info_l = {0};                                                                                      \
    convert_XVisualInfo_to_64(dpy, &info_l, info);                                                                           \
    return my32_glXCreateContext_fct_##A (dpy, &info_l, shared, direct);                                                \
}
SUPER()
#undef GO
static void* find_glXCreateContext_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my32_glXCreateContext_fct_##A == (pFpppi_t)fct) return my32_glXCreateContext_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_glXCreateContext_fct_##A == 0) {my32_glXCreateContext_fct_##A = (pFpppi_t)fct; return my32_glXCreateContext_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libGL glXCreateContext callback\n");
    return NULL;
}
// glXGetFBConfigFromVisualSGIX ...
#define GO(A)                                                                                           \
static pFpp_t my32_glXGetFBConfigFromVisualSGIX_fct_##A = NULL;                                       \
static void* my32_glXGetFBConfigFromVisualSGIX_##A(x64emu_t* emu, void* dpy, my_XVisualInfo_32_t* info) \
{                                                                                                       \
    if(!my32_glXGetFBConfigFromVisualSGIX_fct_##A)                                                      \
        return NULL;                                                                                    \
    my_XVisualInfo_t info_l = {0};                                                                      \
    convert_XVisualInfo_to_64(dpy, &info_l, info);                                                      \
    return my32_glXGetFBConfigFromVisualSGIX_fct_##A (dpy, &info_l);                                    \
}
SUPER()
#undef GO
static void* find_glXGetFBConfigFromVisualSGIX_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my32_glXGetFBConfigFromVisualSGIX_fct_##A == (pFpp_t)fct) return my32_glXGetFBConfigFromVisualSGIX_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_glXGetFBConfigFromVisualSGIX_fct_##A == 0) {my32_glXGetFBConfigFromVisualSGIX_fct_##A = (pFpp_t)fct; return my32_glXGetFBConfigFromVisualSGIX_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libGL glXGetFBConfigFromVisualSGIX callback\n");
    return NULL;
}
// glMultiDrawElements ...
#define GO(A)                                                                                                                       \
static vFupupi_t my32_glMultiDrawElements_fct_##A = NULL;                                                                           \
static void my32_glMultiDrawElements_##A(x64emu_t* emu, uint32_t mode, void* count, uint32_t type, ptr_t* indices, int drawcount)   \
{                                                                                                                                   \
    if(!my32_glMultiDrawElements_fct_##A)                                                                                           \
        return;                                                                                                                     \
    void* indices_l[drawcount];                                                                                                     \
    for(int i=0; i<drawcount; ++i)                                                                                                  \
        indices_l[i] = from_ptrv(indices[i]);                                                                                       \
    my32_glMultiDrawElements_fct_##A (mode, count, type, indices_l, drawcount);                                                     \
}
SUPER()
#undef GO
static void* find_glMultiDrawElements_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my32_glMultiDrawElements_fct_##A == (vFupupi_t)fct) return my32_glMultiDrawElements_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_glMultiDrawElements_fct_##A == 0) {my32_glMultiDrawElements_fct_##A = (vFupupi_t)fct; return my32_glMultiDrawElements_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libGL glMultiDrawElements callback\n");
    return NULL;
}
static void* find_glMultiDrawElementsEXT_Fct(void* fct)
{
    return find_glMultiDrawElements_Fct(fct);
}
// glMultiModeDrawElementsIBM ...
#define GO(A)                                                                                                                                               \
static vFppupui_t my32_glMultiModeDrawElementsIBM_fct_##A = NULL;                                                                                           \
static void my32_glMultiModeDrawElementsIBM_##A(x64emu_t* emu, void* mode, void* count, uint32_t type, ptr_t* indices, uint32_t primcount, int modestride)  \
{                                                                                                                                                           \
    if(!my32_glMultiModeDrawElementsIBM_fct_##A)                                                                                                            \
        return;                                                                                                                                             \
    void* indices_l[primcount];                                                                                                                             \
    for(uint32_t i=0; i<primcount; ++i)                                                                                                                     \
        indices_l[i] = from_ptrv(indices[i]);                                                                                                               \
    my32_glMultiModeDrawElementsIBM_fct_##A (mode, count, type, indices_l, primcount, modestride);                                                          \
}
SUPER()
#undef GO
static void* find_glMultiModeDrawElementsIBM_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my32_glMultiModeDrawElementsIBM_fct_##A == (vFppupui_t)fct) return my32_glMultiModeDrawElementsIBM_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_glMultiModeDrawElementsIBM_fct_##A == 0) {my32_glMultiModeDrawElementsIBM_fct_##A = (vFppupui_t)fct; return my32_glMultiModeDrawElementsIBM_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libGL glMultiModeDrawElementsIBM callback\n");
    return NULL;
}
static void* find_glMultiModeDrawElementsIBMEXT_Fct(void* fct)
{
    return find_glMultiModeDrawElementsIBM_Fct(fct);
}
// glTransformFeedbackVaryings ...
#define GO(A)                                                                                                               \
static vFuipu_t my32_glTransformFeedbackVaryings_fct_##A = NULL;                                                            \
static void my32_glTransformFeedbackVaryings_##A(x64emu_t* emu, uint32_t prog, int count, ptr_t* varyings, uint32_t mode)   \
{                                                                                                                           \
    if(!my32_glTransformFeedbackVaryings_fct_##A)                                                                           \
        return;                                                                                                             \
    void* varyings_l[count];                                                                                                \
    for(int i=0; i<count; ++i)                                                                                              \
        varyings_l[i] = from_ptrv(varyings[i]);                                                                             \
    my32_glTransformFeedbackVaryings_fct_##A (prog, count, varyings_l, mode);                                               \
}
SUPER()
#undef GO
static void* find_glTransformFeedbackVaryings_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my32_glTransformFeedbackVaryings_fct_##A == (vFuipu_t)fct) return my32_glTransformFeedbackVaryings_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_glTransformFeedbackVaryings_fct_##A == 0) {my32_glTransformFeedbackVaryings_fct_##A = (vFuipu_t)fct; return my32_glTransformFeedbackVaryings_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libGL glTransformFeedbackVaryings callback\n");
    return NULL;
}
static void* find_glTransformFeedbackVaryingsEXT_Fct(void* fct)
{
    return find_glTransformFeedbackVaryings_Fct(fct);
}
// glBindBuffersRange ...
#define GO(A)                                                                                                                                       \
static vFuuippp_t my32_glBindBuffersRange_fct_##A = NULL;                                                                                           \
static void my32_glBindBuffersRange_##A(x64emu_t* emu, uint32_t target, uint32_t first, int count, void* buffers, ptr_t* offsets, ptr_t* sizes)     \
{                                                                                                                                                   \
    if(!my32_glBindBuffersRange_fct_##A)                                                                                                            \
        return;                                                                                                                                     \
    long offsets_l[count];                                                                                                                          \
    long sizes_l[count];                                                                                                                            \
    for(int i=0; i<count; ++i) {                                                                                                                    \
        offsets_l[i] = from_long(offsets[i]);                                                                                                       \
        sizes_l[i] = from_long(sizes[i]);                                                                                                           \
    }                                                                                                                                               \
    my32_glBindBuffersRange_fct_##A (target, first, count, buffers, offsets_l, sizes);                                                              \
}
SUPER()
#undef GO
static void* find_glBindBuffersRange_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my32_glBindBuffersRange_fct_##A == (vFuuippp_t)fct) return my32_glBindBuffersRange_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_glBindBuffersRange_fct_##A == 0) {my32_glBindBuffersRange_fct_##A = (vFuuippp_t)fct; return my32_glBindBuffersRange_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libGL glBindBuffersRange callback\n");
    return NULL;
}
// glBindVertexBuffers ...
#define GO(A)                                                                                                                       \
static vFuippp_t my32_glBindVertexBuffers_fct_##A = NULL;                                                                           \
static void my32_glBindVertexBuffers_##A(x64emu_t* emu, uint32_t first, int count, void* buffers, ptr_t* offsets, void* strides)    \
{                                                                                                                                   \
    if(!my32_glBindVertexBuffers_fct_##A)                                                                                           \
        return;                                                                                                                     \
    long offsets_l[count];                                                                                                          \
    for(int i=0; i<count; ++i)                                                                                                      \
        offsets_l[i] = from_long(offsets[i]);                                                                                       \
    my32_glBindVertexBuffers_fct_##A (first, count, buffers, offsets_l, strides);                                                   \
}
SUPER()
#undef GO
static void* find_glBindVertexBuffers_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my32_glBindVertexBuffers_fct_##A == (vFuippp_t)fct) return my32_glBindVertexBuffers_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_glBindVertexBuffers_fct_##A == 0) {my32_glBindVertexBuffers_fct_##A = (vFuippp_t)fct; return my32_glBindVertexBuffers_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libGL glBindVertexBuffers callback\n");
    return NULL;
}
// glVertexArrayVertexBuffers ...
#define GO(A)                                                                                                                                           \
static vFuuippp_t my32_glVertexArrayVertexBuffers_fct_##A = NULL;                                                                                       \
static void my32_glVertexArrayVertexBuffers_##A(x64emu_t* emu, uint32_t vaobj, uint32_t first, int count, void* buffers, ptr_t* offsets, void* strides) \
{                                                                                                                                                       \
    if(!my32_glVertexArrayVertexBuffers_fct_##A)                                                                                                        \
        return;                                                                                                                                         \
    long offsets_l[count];                                                                                                                              \
    for(int i=0; i<count; ++i)                                                                                                                          \
        offsets_l[i] = from_long(offsets[i]);                                                                                                           \
    my32_glVertexArrayVertexBuffers_fct_##A (vaobj, first, count, buffers, offsets_l, strides);                                                         \
}
SUPER()
#undef GO
static void* find_glVertexArrayVertexBuffers_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my32_glVertexArrayVertexBuffers_fct_##A == (vFuuippp_t)fct) return my32_glVertexArrayVertexBuffers_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_glVertexArrayVertexBuffers_fct_##A == 0) {my32_glVertexArrayVertexBuffers_fct_##A = (vFuuippp_t)fct; return my32_glVertexArrayVertexBuffers_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libGL glVertexArrayVertexBuffers callback\n");
    return NULL;
}
// glMultiDrawElementsBaseVertex ...
#define GO(A)                                                                                                                                                   \
static vFupupip_t my32_glMultiDrawElementsBaseVertex_fct_##A = NULL;                                                                                            \
static void my32_glMultiDrawElementsBaseVertex_##A(x64emu_t* emu, uint32_t mode, void* count, uint32_t type, ptr_t* indices, int drawcount, void* basevertex)   \
{                                                                                                                                                               \
    if(!my32_glMultiDrawElementsBaseVertex_fct_##A)                                                                                                             \
        return;                                                                                                                                                 \
    void* indices_l[drawcount];                                                                                                                                 \
    for(int i=0; i<drawcount; ++i)                                                                                                                              \
        indices_l[i] = from_ptrv(indices[i]);                                                                                                                   \
    my32_glMultiDrawElementsBaseVertex_fct_##A (mode, count, type, indices_l, drawcount, basevertex);                                                           \
}
SUPER()
#undef GO
static void* find_glMultiDrawElementsBaseVertex_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my32_glMultiDrawElementsBaseVertex_fct_##A == (vFupupip_t)fct) return my32_glMultiDrawElementsBaseVertex_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_glMultiDrawElementsBaseVertex_fct_##A == 0) {my32_glMultiDrawElementsBaseVertex_fct_##A = (vFupupip_t)fct; return my32_glMultiDrawElementsBaseVertex_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libGL glMultiDrawElementsBaseVertex callback\n");
    return NULL;
}
// glCreateShaderProgramv ...
#define GO(A)                                                                                           \
static vFuip_t my32_glCreateShaderProgramv_fct_##A = NULL;                                              \
static void my32_glCreateShaderProgramv_##A(x64emu_t* emu, uint32_t shader, int count, ptr_t* string)   \
{                                                                                                       \
    if(!my32_glCreateShaderProgramv_fct_##A)                                                            \
        return;                                                                                         \
    char* str[count];                                                                                   \
    if(string) for(int i=0; i<count; ++i) str[i] = from_ptrv(string[i]);                                \
    my32_glCreateShaderProgramv_fct_##A (shader, count, string?str:NULL);                               \
}
SUPER()
#undef GO
static void* find_glCreateShaderProgramv_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my32_glCreateShaderProgramv_fct_##A == (vFuip_t)fct) return my32_glCreateShaderProgramv_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_glCreateShaderProgramv_fct_##A == 0) {my32_glCreateShaderProgramv_fct_##A = (vFuip_t)fct; return my32_glCreateShaderProgramv_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libGL glCreateShaderProgramv callback\n");
    return NULL;
}
// glCompileShaderIncludeARB ...
#define GO(A)                                                                                                       \
static vFuipp_t my32_glCompileShaderIncludeARB_fct_##A = NULL;                                                         \
static void my32_glCompileShaderIncludeARB_##A(x64emu_t* emu, uint32_t shader, int count, ptr_t* string, int* length)  \
{                                                                                                                   \
    if(!my32_glCompileShaderIncludeARB_fct_##A)                                                                        \
        return;                                                                                                     \
    char* str[count];                                                                                               \
    if(string) for(int i=0; i<count; ++i) str[i] = from_ptrv(string[i]);                                            \
    my32_glCompileShaderIncludeARB_fct_##A (shader, count, string?str:NULL, length);                                   \
}
SUPER()
#undef GO
static void* find_glCompileShaderIncludeARB_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my32_glCompileShaderIncludeARB_fct_##A == (vFuipp_t)fct) return my32_glCompileShaderIncludeARB_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_glCompileShaderIncludeARB_fct_##A == 0) {my32_glCompileShaderIncludeARB_fct_##A = (vFuipp_t)fct; return my32_glCompileShaderIncludeARB_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libGL glCompileShaderIncludeARB callback\n");
    return NULL;
}
// glXGetFBConfigs ...
#define GO(A)                                                                       \
static pFpip_t my32_glXGetFBConfigs_fct_##A = NULL;                                 \
static void* my32_glXGetFBConfigs_##A(x64emu_t* emu, void* dpy, int screen, int* n) \
{                                                                                   \
    if(!my32_glXGetFBConfigs_fct_##A)                                               \
        return NULL;                                                                \
    void* ret = my32_glXGetFBConfigs_fct_##A (dpy, screen, n);                      \
    if(!ret) return NULL;                                                           \
    void** src = ret;                                                               \
    ptr_t* dst = ret;                                                               \
    for(int i=0; i<*n; ++i)                                                         \
        dst[i] = to_ptrv(src[i]);                                                   \
    return ret;                                                                     \
}
SUPER()
#undef GO
static void* find_glXGetFBConfigs_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my32_glXGetFBConfigs_fct_##A == (pFpip_t)fct) return my32_glXGetFBConfigs_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_glXGetFBConfigs_fct_##A == 0) {my32_glXGetFBConfigs_fct_##A = (pFpip_t)fct; return my32_glXGetFBConfigs_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libGL glXGetFBConfigs callback\n");
    return NULL;
}
// glGetUniformIndices ...
#define GO(A)                                                                                                   \
static vFuipp_t my32_glGetUniformIndices_fct_##A = NULL;                                                        \
static void my32_glGetUniformIndices_##A(x64emu_t* emu, uint32_t prog, int count, ptr_t* names, void* indices)  \
{                                                                                                               \
    if(!my32_glGetUniformIndices_fct_##A)                                                                       \
        return;                                                                                                 \
    void* names_l[count];                                                                                       \
    my32_glGetUniformIndices_fct_##A (prog, count, names?names_l:NULL, indices);                                \
    if(names) for(int i=0; i<count; ++i) names[i] = to_ptrv(names_l[i]);                                        \
}
SUPER()
#undef GO
static void* find_glGetUniformIndices_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my32_glGetUniformIndices_fct_##A == (vFuipp_t)fct) return my32_glGetUniformIndices_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_glGetUniformIndices_fct_##A == 0) {my32_glGetUniformIndices_fct_##A = (vFuipp_t)fct; return my32_glGetUniformIndices_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libGL glGetUniformIndices callback\n");
    return NULL;
}
// glVDPAUMapSurfacesNV ...
#define GO(A)                                                                                   \
static vFip_t my32_glVDPAUMapSurfacesNV_fct_##A = NULL;                                         \
static void my32_glVDPAUMapSurfacesNV_##A(x64emu_t* emu, int count, long_t* surfaces)           \
{                                                                                               \
    if(!my32_glVDPAUMapSurfacesNV_fct_##A)                                                      \
        return;                                                                                 \
    long surfaces_l[count];                                                                     \
    if(surfaces) for(int i=0; i<count; ++i) surfaces_l[i] = from_long(surfaces[i]);             \
    my32_glVDPAUMapSurfacesNV_fct_##A (count, surfaces?surfaces_l:NULL);                        \
}
SUPER()
#undef GO
static void* find_glVDPAUMapSurfacesNV_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my32_glVDPAUMapSurfacesNV_fct_##A == (vFip_t)fct) return my32_glVDPAUMapSurfacesNV_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_glVDPAUMapSurfacesNV_fct_##A == 0) {my32_glVDPAUMapSurfacesNV_fct_##A = (vFip_t)fct; return my32_glVDPAUMapSurfacesNV_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libGL glVDPAUMapSurfacesNV callback\n");
    return NULL;
}
// glVDPAUUnmapSurfacesNV ...
#define GO(A)                                                                                   \
static vFip_t my32_glVDPAUUnmapSurfacesNV_fct_##A = NULL;                                       \
static void my32_glVDPAUUnmapSurfacesNV_##A(x64emu_t* emu, int count, long_t* surfaces)         \
{                                                                                               \
    if(!my32_glVDPAUUnmapSurfacesNV_fct_##A)                                                    \
        return;                                                                                 \
    long surfaces_l[count];                                                                     \
    if(surfaces) for(int i=0; i<count; ++i) surfaces_l[i] = from_long(surfaces[i]);             \
    my32_glVDPAUUnmapSurfacesNV_fct_##A (count, surfaces?surfaces_l:NULL);                      \
}
SUPER()
#undef GO
static void* find_glVDPAUUnmapSurfacesNV_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my32_glVDPAUUnmapSurfacesNV_fct_##A == (vFip_t)fct) return my32_glVDPAUUnmapSurfacesNV_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_glVDPAUUnmapSurfacesNV_fct_##A == 0) {my32_glVDPAUUnmapSurfacesNV_fct_##A = (vFip_t)fct; return my32_glVDPAUUnmapSurfacesNV_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libGL glVDPAUUnmapSurfacesNV callback\n");
    return NULL;
}

#undef SUPER

#define PRE_INIT                                                                \
    if(BOX64ENV(libgl)) {                                                           \
        lib->w.lib = dlopen(BOX64ENV(libgl), RTLD_LAZY | RTLD_GLOBAL);              \
        lib->path = strdup(BOX64ENV(libgl));                                        \
    } else if(strstr(lib->name, "libGLX_nvidia.so.0")) {                        \
        lib->w.lib = dlopen("libGLX_nvidia.so.0", RTLD_LAZY | RTLD_GLOBAL);     \
        if(lib->w.lib) lib->path = strdup("libGLX_nvidia.so.0");                \
    }
#define CUSTOM_INIT \
    my_lib = lib;                                                               \
    lib->w.priv = dlsym(lib->w.lib, "glXGetProcAddress");                       \
    void* symb = dlsym(lib->w.lib, "glDebugMessageCallback");                   \
    if(symb) {                                                                  \
        k = kh_get(symbolmap, lib->w.mysymbolmap, "glDebugMessageCallback");    \
        symbol1_t *s = &kh_value(lib->w.mysymbolmap, k);                        \
        s->resolved = 1;                                                        \
        s->addr = AddBridge(lib->w.bridge, s->w, find_glDebugMessageCallback_Fct(symb), 0, "glDebugMessageCallback"); \
    }                                                                           \
    symb = dlsym(lib->w.lib, "glXSwapIntervalMESA");                            \
    if(symb) {                                                                  \
        k = kh_get(symbolmap, lib->w.mysymbolmap, "glXSwapIntervalMESA");       \
        symbol1_t *s = &kh_value(lib->w.mysymbolmap, k);                        \
        s->resolved = 1;                                                        \
        s->addr = AddBridge(lib->w.bridge, s->w, find_glXSwapIntervalMESA_Fct(symb), 0, "glXSwapIntervalMESA"); \
    }                                                                           \
    symb = dlsym(lib->w.lib, "glXSwapIntervalEXT");                             \
    if(symb) {                                                                  \
        k = kh_get(symbolmap, lib->w.mysymbolmap, "glXSwapIntervalEXT");        \
        symbol1_t *s = &kh_value(lib->w.mysymbolmap, k);                        \
        s->resolved = 1;                                                        \
        s->addr = AddBridge(lib->w.bridge, s->w, find_glXSwapIntervalEXT_Fct(symb), 0, "glXSwapIntervalEXT"); \
    }                                                                           \

// creating function for direct access, just in case
EXPORT void my32_glShaderSource(x64emu_t* emu, uint32_t shader, int count, ptr_t* string, int* length)
{
    char* str[count];
    if(string) for(int i=0; i<count; ++i) str[i] = from_ptrv(string[i]);
    my->glShaderSource(shader, count, string?str:NULL, length);
}
EXPORT void my32_glShaderSourceARB(x64emu_t* emu, uint32_t shader, int count, ptr_t* string, int* length)
{
    my32_glShaderSource(emu, shader, count, string, length);
}

EXPORT void* my32_glXChooseFBConfig(x64emu_t* emu, void* dpy, int screen, int* list, int* nelement)
{
    void** res = my->glXChooseFBConfig(dpy, screen, list, nelement);
    if(!res)
        return NULL;
    ptr_t *fbconfig = (ptr_t*)res;
    for(int i=0; i<*nelement; ++i)
        fbconfig[i] = to_ptrv(res[i]);
    return res;
}

EXPORT void* my32_glXChooseFBConfigSGIX(x64emu_t* emu, void* dpy, int screen, int* list, int* nelement)
{
    void** res = my->glXChooseFBConfigSGIX(dpy, screen, list, nelement);
    if(!res)
        return NULL;
    ptr_t *fbconfig = (ptr_t*)res;
    for(int i=0; i<*nelement; ++i)
        fbconfig[i] = to_ptrv(res[i]);
    return res;
}

EXPORT void* my32_glXGetVisualFromFBConfig(x64emu_t* emu, void* dpy, void* config)
{
    void* res = my->glXGetVisualFromFBConfig(dpy, config);
    if(!res) return NULL;
    my_XVisualInfo_32_t* vinfo = (my_XVisualInfo_32_t*)res;
    convert_XVisualInfo_to_32(dpy, vinfo, res);
    return vinfo;
}

EXPORT void* my32_glXGetVisualFromFBConfigSGIX(x64emu_t* emu, void* dpy, void* config)
{
    void* res = my->glXGetVisualFromFBConfigSGIX(dpy, config);
    if(!res) return NULL;
    my_XVisualInfo_32_t* vinfo = (my_XVisualInfo_32_t*)res;
    convert_XVisualInfo_to_32(dpy, vinfo, res);
    return vinfo;
}

EXPORT void* my32_glXChooseVisual(x64emu_t* emu, void* dpy, int screen, int* attr)
{
    void* res = my->glXChooseVisual(dpy, screen, attr);
    if(!res) return NULL;
    my_XVisualInfo_32_t* vinfo = (my_XVisualInfo_32_t*)res;
    convert_XVisualInfo_to_32(dpy, vinfo, res);
    return vinfo;
}

EXPORT void* my32_glXCreateContext(x64emu_t* emu, void* dpy, my_XVisualInfo_32_t* info, void* shared, int direct)
{
    my_XVisualInfo_t info_l = {0};
    convert_XVisualInfo_to_64(dpy, &info_l, info);
    return my->glXCreateContext(dpy, &info_l, shared, direct);
}

EXPORT void* my32_glXGetFBConfigFromVisualSGIX(x64emu_t* emu, void* dpy, my_XVisualInfo_32_t* info)
{
    my_XVisualInfo_t info_l = {0};
    convert_XVisualInfo_to_64(dpy, &info_l, info);
    return my->glXGetFBConfigFromVisualSGIX(dpy, &info_l);
}

EXPORT void my32_glMultiDrawElements(x64emu_t* emu, uint32_t mode, void* count, uint32_t type, ptr_t* indices, int drawcount)
{
    void* indices_l[drawcount];
    for(int i=0; i<drawcount; ++i)
        indices_l[i] = from_ptrv(indices[i]);
    my->glMultiDrawElements(mode, count, type, indices_l, drawcount);
}
EXPORT void my32_glMultiDrawElementsExt(x64emu_t* emu, uint32_t mode, void* count, uint32_t type, ptr_t* indices, int drawcount)
{
    my32_glMultiDrawElements(emu, mode, count, type, indices, drawcount);
}

EXPORT void my32_glMultiModeDrawElementsIBM(x64emu_t* emu, void* mode, void* count, uint32_t type, ptr_t* indices, uint32_t primcount, int modestride)
{
    void* indices_l[primcount];
    for(uint32_t i=0; i<primcount; ++i)
        indices_l[i] = from_ptrv(indices[i]);
    my->glMultiModeDrawElementsIBM(mode, count, type, indices_l, primcount, modestride);
}

EXPORT void my32_glTransformFeedbackVaryings(x64emu_t* emu, uint32_t prog, int count, ptr_t* varyings, uint32_t mode)
{
    void* varyings_l[count];
    for(int i=0; i<count; ++i)
        varyings_l[i] = from_ptrv(varyings[i]);
    my->glTransformFeedbackVaryings(prog, count, varyings_l, mode);
}
EXPORT void my32_glTransformFeedbackVaryingsEXT(x64emu_t* emu, uint32_t prog, int count, ptr_t* varyings, uint32_t mode)
{
    my32_glTransformFeedbackVaryings(emu, prog, count, varyings, mode);
}

EXPORT void my32_glBindBuffersRange(x64emu_t* emu, uint32_t target, uint32_t first, int count, void* buffers, long_t* offsets, long_t* sizes)
{
    long offsets_l[count];
    long sizes_l[count];
    for(int i=0; i<count; ++i) {
        offsets_l[i] = from_long(offsets[i]);
        sizes_l[i] = from_long(sizes_l[i]);
    }
    my->glBindBuffersRange(target, first, count, buffers, offsets_l, sizes_l);
}

EXPORT void my32_glBindVertexBuffers(x64emu_t* emu, uint32_t first, int count, void* buffers, long_t* offsets, void* strides)
{
    long offsets_l[count];
    for(int i=0; i<count; ++i)
        offsets_l[i] = from_long(offsets[i]);
    my->glBindVertexBuffers(first, count, buffers, offsets_l, strides);
}

EXPORT void my32_glVertexArrayVertexBuffers(x64emu_t* emu, uint32_t vaobj, uint32_t first, int count, void* buffers, long_t* offsets, void* strides)
{
    long offsets_l[count];
    for(int i=0; i<count; ++i)
        offsets_l[i] = from_long(offsets[i]);
    my->glVertexArrayVertexBuffers(vaobj, first, count, buffers, offsets_l, strides);
}

EXPORT void my32_glMultiDrawElementsBaseVertex(x64emu_t* emu, uint32_t mode, void* count, uint32_t type, ptr_t* indices, int drawcount, void* basevertex)
{
    void* indices_l[drawcount];
    for(int i=0; i<drawcount; ++i)
        indices_l[i] = from_ptrv(indices[i]);
    my->glMultiDrawElementsBaseVertex(mode, count, type, indices_l, drawcount, basevertex);
}

EXPORT void my32_glCreateShaderProgramv(x64emu_t* emu, uint32_t shader, int count, ptr_t* string)
{
    char* str[count];
    if(string) for(int i=0; i<count; ++i) str[i] = from_ptrv(string[i]);
    my->glCreateShaderProgramv(shader, count, string?str:NULL);
}

EXPORT void my32_glCompileShaderIncludeARB(x64emu_t* emu, uint32_t shader, int count, ptr_t* string, int* length)
{
    char* str[count];
    if(string) for(int i=0; i<count; ++i) str[i] = from_ptrv(string[i]);
    my->glCompileShaderIncludeARB(shader, count, string?str:NULL, length);
}

EXPORT void* my32_glXGetFBConfigs(x64emu_t* emu, void* dpy, int screen, int* n)
{
    void* ret = my->glXGetFBConfigs(dpy, screen, n);
    if(!ret) return NULL;
    ptr_t* dst = ret;
    void** src = ret;
    for(int i=0; i<*n; ++i)
        dst[i] = to_ptrv(src[i]);
    return ret;
}

EXPORT void my32_glGetUniformIndices(x64emu_t* emu, uint32_t prog, int count, ptr_t* names, void* indices)
{
    void* names_l[count];
    my->glGetUniformIndices(prog, count, names?names_l:NULL, indices);
    if(names)
        for(int i=0; i<count; ++i)
            names[i] = to_ptrv(names_l[i]);
}

#include "wrappedlib_init32.h"

#define SUPER()                                 \
 GO(vFpp_t, glDebugMessageCallback)             \
 GO(vFpp_t, glDebugMessageCallbackARB)          \
 GO(vFpp_t, glDebugMessageCallbackAMD)          \
 GO(vFpp_t, glDebugMessageCallbackKHR)          \
 GO(vFpp_t, eglDebugMessageControlKHR)          \
 GO(iFi_t, glXSwapIntervalMESA)                 \
 GO(vFppi_t, glXSwapIntervalEXT)                \
 GO(vFpp_t, glProgramCallbackMESA)              \
 GO(pFp_t, glGetVkProcAddrNV)                   \
 GO(vFppp_t, eglSetBlobCacheFuncsANDROID)       \
 GO(vFuipp_t, glShaderSource)                   \
 GO(vFuipp_t, glShaderSourceARB)                \
 GO(pFpipp_t, glXChooseFBConfig)                \
 GO(pFpipp_t, glXChooseFBConfigSGIX)            \
 GO(pFpp_t, glXGetVisualFromFBConfig)           \
 GO(pFpp_t, glXGetVisualFromFBConfigSGIX)       \
 GO(pFpp_t, glXChooseVisual)                    \
 GO(pFpppi_t, glXCreateContext)                 \
 GO(pFpp_t, glXGetFBConfigFromVisualSGIX)       \
 GO(vFupupi_t, glMultiDrawElements)             \
 GO(vFppupui_t, glMultiModeDrawElementsIBM)     \
 GO(vFuipu_t, glTransformFeedbackVaryings)      \
 GO(vFuuippp_t, glBindBuffersRange)             \
 GO(vFuippp_t, glBindVertexBuffers)             \
 GO(vFuuippp_t, glVertexArrayVertexBuffers)     \
 GO(vFupupip_t, glMultiDrawElementsBaseVertex)  \
 GO(vFuip_t, glCreateShaderProgramv)            \
 GO(vFuipp_t, glCompileShaderIncludeARB)        \
 GO(vFupupi_t, glMultiDrawElementsEXT)          \
 GO(vFuipu_t, glTransformFeedbackVaryingsEXT)   \
 GO(pFpip_t, glXGetFBConfigs)                   \
 GO(vFuipp_t, glGetUniformIndices)              \
 GO(vFuipp_t, glVDPAUMapSurfacesNV)             \
 GO(vFuipp_t, glVDPAUUnmapSurfacesNV)           \

gl_wrappers_t* getGLProcWrapper32(box64context_t* context, glprocaddress_t procaddress)
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

void* getGLProcAddress32(x64emu_t* emu, glprocaddress_t procaddr, const char* rname)
{
    khint_t k;
    printf_dlsym(LOG_DEBUG, "Calling getGLProcAddress32[%p](\"%s\") => ", procaddr, rname);
    gl_wrappers_t* wrappers = getGLProcWrapper32(emu->context, procaddr);
    // check if glxprocaddress is filled, and search for lib and fill it if needed
    // get proc adress using actual glXGetProcAddress
    k = kh_get(symbolmap, wrappers->glmymap, rname);
    int is_my = (k==kh_end(wrappers->glmymap))?0:1;
    void* symbol;
    if(is_my) {
        // try again, by using custom "my_" now...
        #define GO(A, B) else if(!strcmp(rname, #B)) symbol = find_##B##_Fct(procaddr(rname));
        if(0) {}
        SUPER()
        else {
            if(strcmp(rname, "glXGetProcAddress") && strcmp(rname, "glXGetProcAddressARB")) {
                printf_log(LOG_NONE, "Warning, %s defined as GOM, but find_%s_Fct not defined\n", rname, rname);
            }
            char tmp[200];
            strcpy(tmp, "my_");
            strcat(tmp, rname);
            symbol = dlsym(emu->context->box64lib, tmp);
        }
        #undef GO
        #undef SUPER
    } else
        symbol = procaddr(rname);
    if(!symbol) {
        printf_dlsym_prefix(0, LOG_DEBUG, "%p\n", NULL);
        return NULL;    // easy
    }
    // check if alread bridged
    uintptr_t ret = CheckBridged(emu->context->system, symbol);
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
    if(!s->resolved) {
        const char* constname = kh_key(wrappers->glwrappers, k);
        AddOffsetSymbol(emu->context->maplib, symbol, rname);
        s->addr = AddCheckBridge(emu->context->system, s->w, symbol, 0, constname);
        s->resolved = 1;
    }
    ret = s->addr;
    printf_dlsym_prefix(0, LOG_DEBUG, "%p\n", (void*)ret);
    return (void*)ret;
}
