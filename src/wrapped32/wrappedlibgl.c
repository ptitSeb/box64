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
EXPORT void* my32_glXGetProcAddress(x64emu_t* emu, void* name)
{
    pFp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glXGetProcAddress;
    khint_t k;
    const char* rname = (const char*)name;
    return getGLProcAddress32(emu, NULL, (void*)fnc, rname);
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

#undef SUPER

#define PRE_INIT                                                                \
    if(BOX64ENV(libgl)) {                                                           \
        lib->w.lib = dlopen(BOX64ENV(libgl), RTLD_LAZY | RTLD_GLOBAL);              \
        lib->path = strdup(BOX64ENV(libgl));                                        \
    } else if(strstr(lib->name, "libGLX_nvidia.so.0")) {                        \
        lib->w.lib = dlopen("libGLX_nvidia.so.0", RTLD_LAZY | RTLD_GLOBAL);     \
        if(lib->w.lib) lib->path = strdup("libGLX_nvidia.so.0");                \
    }

// glDebugMessageCallback ...
EXPORT void my32_glDebugMessageCallback(x64emu_t* emu, void* prod, void* param)
{
    vFpp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glDebugMessageCallback;
    fnc(find_debug_callback_Fct(prod), param);
}
// glDebugMessageCallbackARB ...
EXPORT void my32_glDebugMessageCallbackARB(x64emu_t* emu, void* prod, void* param)
{
    vFpp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glDebugMessageCallbackARB;
    fnc(find_debug_callback_Fct(prod), param);
}
// glDebugMessageCallbackAMD ...
EXPORT void my32_glDebugMessageCallbackAMD(x64emu_t* emu, void* prod, void* param)
{
    vFpp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glDebugMessageCallbackAMD;
    fnc(find_debug_callback_Fct(prod), param);
}
// glDebugMessageCallbackKHR ...
EXPORT void my32_glDebugMessageCallbackKHR(x64emu_t* emu, void* prod, void* param)
{
    vFpp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glDebugMessageCallbackKHR;
    fnc(find_debug_callback_Fct(prod), param);
}
// eglDebugMessageControlKHR ...
EXPORT int my32_eglDebugMessageControlKHR(x64emu_t* emu, void* prod, void* param)
{
    iFpp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->eglDebugMessageControlKHR;
    return fnc(find_egl_debug_callback_Fct(prod), param);
}
// eglSetBlobCacheFuncsANDROID ...
EXPORT void my32_eglSetBlobCacheFuncsANDROID(x64emu_t* emu, void* dpy, void* set, void* get)
{
    vFppp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->eglSetBlobCacheFuncsANDROID;
    fnc(dpy, find_set_blob_func_Fct(set), find_get_blob_func_Fct(get));
}
// glXSwapIntervalMESA ...
EXPORT int my32_dummy_glXSwapIntervalMESA(int interval)
{
    return 5; // GLX_BAD_CONTEXT
}
EXPORT int my32_glXSwapIntervalMESA(x64emu_t* emu, int interval)
{
    iFi_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glXSwapIntervalMESA;
    if(!fnc) fnc=my32_dummy_glXSwapIntervalMESA;
    return fnc(interval);
}
// glXSwapIntervalEXT ...
EXPORT void my32_dummy_glXSwapIntervalEXT(void* dpy, void* drawable, int interval) {}
EXPORT void my32_glXSwapIntervalEXT(x64emu_t* emu, void* dpy, void* drawable, int interval)
{
    vFppi_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glXSwapIntervalEXT;
    if(!fnc) fnc=my32_dummy_glXSwapIntervalEXT;
    fnc(dpy, drawable, interval);
}

// glProgramCallbackMESA ...
EXPORT void my32_glProgramCallbackMESA(x64emu_t* emu, int t, void* f, void* data)
{
    vFipp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glProgramCallbackMESA;
    fnc(t, find_program_callback_Fct(f), data);
}

void* my_GetVkProcAddr(x64emu_t* emu, void* name, void*(*getaddr)(void*));  // defined in wrappedvulkan.c
// glGetVkProcAddrNV ...
EXPORT void* my32_glGetVkProcAddrNV(x64emu_t* emu, void* name)
{
    pFp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glGetVkProcAddrNV;
    return my_GetVkProcAddr(emu, name, fnc);
}
// glShaderSource ...
EXPORT void my32_glShaderSource(x64emu_t* emu, uint32_t shader, int count, ptr_t* string, int* length)
{
    vFuipp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glShaderSource;
    char* str[count];
    if(string) for(int i=0; i<count; ++i) str[i] = from_ptrv(string[i]);
    fnc(shader, count, string?str:NULL, length);
}
EXPORT void my32_glShaderSourceARB(x64emu_t* emu, uint32_t shader, int count, ptr_t* string, int* length) __attribute__((alias("my32_glShaderSource")));
// glXChooseFBConfig ...
EXPORT void* my32_glXChooseFBConfig(x64emu_t* emu, void* dpy, int screen, int* list, int* nelement)
{
    pFpipp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glXChooseFBConfig;
    void** res = fnc(dpy, screen, list, nelement);
    if(!res)
        return NULL;
    ptr_t* fbconfig = (ptr_t*)res;
    for(int i=0; i<*nelement; ++i)
        fbconfig[i] = to_ptrv(res[i]);
    return res;
}
// glXChooseFBConfigSGIX ...
EXPORT void* my32_glXChooseFBConfigSGIX(x64emu_t* emu, void* dpy, int screen, int* list, int* nelement)
{
    pFpipp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glXChooseFBConfigSGIX;
    void** res = fnc(dpy, screen, list, nelement);
    if(!res)
        return NULL;
    ptr_t* fbconfig = (ptr_t*)res;
    for(int i=0; i<*nelement; ++i)
        fbconfig[i] = to_ptrv(res[i]);
    return res;
}
// glXGetVisualFromFBConfig ...
EXPORT void* my32_glXGetVisualFromFBConfig(x64emu_t* emu, void* dpy, void* config)
{
    pFpp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glXGetVisualFromFBConfig;
    void* res = fnc(dpy, config);
    if(!res)
        return NULL;
    my_XVisualInfo_32_t* vinfo = (my_XVisualInfo_32_t*)res;
    convert_XVisualInfo_to_32(dpy, vinfo, res);
    return vinfo;
}
// glXGetVisualFromFBConfigSGIX ...
EXPORT void* my32_glXGetVisualFromFBConfigSGIX(x64emu_t* emu, void* dpy, void* config)
{
    pFpp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glXGetVisualFromFBConfigSGIX;
    void* res = fnc(dpy, config);
    if(!res)
        return NULL;
    my_XVisualInfo_32_t* vinfo = (my_XVisualInfo_32_t*)res;
    convert_XVisualInfo_to_32(dpy, vinfo, res);
    return vinfo;
}
// glXChooseVisual ...
EXPORT void* my32_glXChooseVisual(x64emu_t* emu, void* dpy, int screen, int* attr)
{
    pFpip_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glXChooseVisual;
    void* res = fnc(dpy, screen, attr);
    my_XVisualInfo_32_t* vinfo = (my_XVisualInfo_32_t*)res;
    convert_XVisualInfo_to_32(dpy, vinfo, res);
    return vinfo;
}
// glXCreateContext ...
EXPORT void* my32_glXCreateContext(x64emu_t* emu, void* dpy, my_XVisualInfo_32_t* info, void* shared, int direct) 
{
    pFpppi_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glXCreateContext;
    my_XVisualInfo_t info_l = {0};
    convert_XVisualInfo_to_64(dpy, &info_l, info);
    return fnc(dpy, &info_l, shared, direct);
}
// glXGetFBConfigFromVisualSGIX ...
EXPORT void* my32_glXGetFBConfigFromVisualSGIX(x64emu_t* emu, void* dpy, my_XVisualInfo_32_t* info)
{
    pFpp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glXGetFBConfigFromVisualSGIX;
    my_XVisualInfo_t info_l = {0};
    convert_XVisualInfo_to_64(dpy, &info_l, info);
    return fnc(dpy, &info_l);
}
// glMultiDrawElements ...
EXPORT void my32_glMultiDrawElements(x64emu_t* emu, uint32_t mode, void* count, uint32_t type, ptr_t* indices, int drawcount)
{
    vFupupi_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glMultiDrawElements;
    void* indices_l[drawcount];
    for(int i=0; i<drawcount; ++i)
        indices_l[i] = from_ptrv(indices[i]);
    fnc(mode, count, type, indices_l, drawcount);
}
EXPORT void my32_glMultiDrawElementsExt(x64emu_t* emu, uint32_t mode, void* count, uint32_t type, ptr_t* indices, int drawcount) __attribute__((alias("my32_glMultiDrawElements")));
// glMultiModeDrawElementsIBM ...
EXPORT void my32_glMultiModeDrawElementsIBM(x64emu_t* emu, void* mode, void* count, uint32_t type, ptr_t* indices, uint32_t primcount, int modestride)
{
    vFppupui_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glMultiModeDrawElementsIBM;
    void* indices_l[primcount];
    for(uint32_t i=0; i<primcount; ++i)
        indices_l[i] = from_ptrv(indices[i]);
    fnc(mode, count, type, indices_l, primcount, modestride);
}
// glTransformFeedbackVaryings ...
EXPORT void my32_glTransformFeedbackVaryings(x64emu_t* emu, uint32_t prog, int count, ptr_t* varyings, uint32_t mode)
{
    vFuipu_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glTransformFeedbackVaryings;
    void* varyings_l[count];
    for(int i=0; i<count; ++i)
        varyings_l[i] = from_ptrv(varyings[i]);
    fnc(prog, count, varyings_l, mode);
}
EXPORT void my32_glTransformFeedbackVaryingsEXT(x64emu_t* emu, uint32_t prog, int count, ptr_t* varyings, uint32_t mode) __attribute__((alias("my32_glTransformFeedbackVaryings")));
// glBindBuffersRange ...
EXPORT void my32_glBindBuffersRange(x64emu_t* emu, uint32_t target, uint32_t first, int count, void* buffers, ptr_t* offsets, ptr_t* sizes)
{
    vFuuippp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glBindBuffersRange;
    long offsets_l[count];
    long sizes_l[count];
    for(int i=0; i<count; ++i) {
        offsets_l[i] = from_long(offsets[i]);
        sizes_l[i] = from_long(sizes[i]);
    }
    fnc(target, first, count, buffers, offsets_l, sizes);
}
// glBindVertexBuffers ...
EXPORT void my32_glBindVertexBuffers(x64emu_t* emu, uint32_t first, int count, void* buffers, ptr_t* offsets, void* strides)
{
    vFuippp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glBindVertexBuffers;
    long offsets_l[count];
    for(int i=0; i<count; ++i)
        offsets_l[i] = from_long(offsets[i]);
    fnc(first, count, buffers, offsets_l, strides);
}
// glVertexArrayVertexBuffers ...
EXPORT void my32_glVertexArrayVertexBuffers(x64emu_t* emu, uint32_t vaobj, uint32_t first, int count, void* buffers, ptr_t* offsets, void* strides)
{
    vFuuippp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glVertexArrayVertexBuffers;
    long offsets_l[count];
    for(int i=0; i<count; ++i)
        offsets_l[i] = from_long(offsets[i]);
    fnc(vaobj, first, count, buffers, offsets_l, strides);
}
// glMultiDrawElementsBaseVertex ...
EXPORT void my32_glMultiDrawElementsBaseVertex(x64emu_t* emu, uint32_t mode, void* count, uint32_t type, ptr_t* indices, int drawcount, void* basevertex)
{
    vFupupip_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glMultiDrawElementsBaseVertex;
    void* indices_l[drawcount];
    for(int i=0; i<drawcount; ++i)
        indices_l[i] = from_ptrv(indices[i]);
    fnc(mode, count, type, indices_l, drawcount, basevertex);
}
// glCreateShaderProgramv ...
EXPORT uint32_t my32_glCreateShaderProgramv(x64emu_t* emu, uint32_t shader, int count, ptr_t* string)
{
    uFuip_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glCreateShaderProgramv;
    char* str[count];
    if(string) for(int i=0; i<count; ++i) str[i] = from_ptrv(string[i]);
    return fnc(shader, count, string?str:NULL);
}
// glCompileShaderIncludeARB ...
EXPORT void my32_glCompileShaderIncludeARB(x64emu_t* emu, uint32_t shader, int count, ptr_t* string, int* length)
{
    vFuipp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glCompileShaderIncludeARB;
    char* str[count];
    if(string) for(int i=0; i<count; ++i) str[i] = from_ptrv(string[i]);
    fnc(shader, count, string?str:NULL, length);
}
// glXGetFBConfigs ...
EXPORT void* my32_glXGetFBConfigs(x64emu_t* emu, void* dpy, int screen, int* n)
{
    pFpip_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glXGetFBConfigs;
    void* ret = fnc(dpy, screen, n);
    if(!ret) return NULL;
    void** src = ret;
    ptr_t* dst = ret;
    for(int i=0; i<*n; ++i)
        dst[i] = to_ptrv(src[i]);
    return ret;
}
// glGetUniformIndices ...
EXPORT void my32_glGetUniformIndices(x64emu_t* emu, uint32_t prog, int count, ptr_t* names, void* indices)
{
    vFuipp_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glGetUniformIndices;
    void* names_l[count];
    fnc(prog, count, names?names_l:NULL, indices);
    if(names) for(int i=0; i<count; ++i) names[i] = to_ptrv(names_l[i]);
}
// glVDPAUMapSurfacesNV ...
EXPORT void my32_glVDPAUMapSurfacesNV(x64emu_t* emu, int count, long_t* surfaces)
{
    vFip_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glVDPAUMapSurfacesNV;
    long surfaces_l[count];
    if(surfaces) for(int i=0; i<count; ++i) surfaces_l[i] = from_long(surfaces[i]);
    fnc(count, surfaces?surfaces_l:NULL);                        \
}
// glVDPAUUnmapSurfacesNV ...
EXPORT void my32_glVDPAUUnmapSurfacesNV(x64emu_t* emu, int count, long_t* surfaces)
{
    vFip_t fnc = getBridgeFnc2((void*)R_RIP);
    if(!fnc) fnc=my->glVDPAUUnmapSurfacesNV;
    long surfaces_l[count];
    if(surfaces) for(int i=0; i<count; ++i) surfaces_l[i] = from_long(surfaces[i]);
    fnc(count, surfaces?surfaces_l:NULL);
}

#include "wrappedlib_init32.h"

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

void* getGLProcAddress32(x64emu_t* emu, const char* my, glprocaddress_t procaddr, const char* rname)
{
    if(!my) my = "my32_";
    khint_t k;
    printf_dlsym(LOG_DEBUG, "Calling getGLProcAddress32[%p](\"%s\") => ", procaddr, rname);
    gl_wrappers_t* wrappers = getGLProcWrapper32(emu->context, procaddr);
    // check if glxprocaddress is filled, and search for lib and fill it if needed
    // get proc adress using actual glXGetProcAddress
    k = kh_get(symbolmap, wrappers->glmymap, rname);
    int is_my = (k==kh_end(wrappers->glmymap))?0:1;
    void* symbol = procaddr(rname);
    void* fnc = NULL;
    if(is_my) {
        // try again, by using custom "my_" now...
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
    const char* constname = kh_key(wrappers->glwrappers, k);
    ret = AddCheckBridge2(emu->context->system, s->w, symbol, fnc, 0, constname);

    printf_dlsym_prefix(0, LOG_DEBUG, "%p\n", (void*)ret);
    return (void*)ret;
}
