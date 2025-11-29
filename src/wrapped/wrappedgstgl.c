#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include "gtkclass.h"
#include "fileutils.h"
#include "gltools.h"

const char* gstglName = "libgstgl-1.0.so.0";
#define ALTNAME "libgstgl-1.0.so"

#define LIBNAME gstgl

typedef size_t  (*LFv_t)();

#define ADDED_FUNCTIONS()                   \
    GO(gst_gl_base_filter_get_type, LFv_t)  \
    GO(gst_gl_filter_get_type, LFv_t)       \
    GO(gst_gl_base_src_get_type, LFv_t)     \

#include "generated/wrappedgstgltypes.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)   \

// getprocaddress_with_platform ...
#define GO(A)   \
static uint64_t my_getprocaddress_platform_##A = 0;                                                 \
static void* my_getprocaddress_with_platform_##A(const char* name)                                  \
{                                                                                                   \
    int type = my_getprocaddress_platform_##A >> 32;                                                \
    int api = my_getprocaddress_platform_##A & 0xffffffff;                                          \
    return my->gst_gl_context_get_proc_address_with_platform(type, api, (void*)name);               \
}
SUPER()
#undef GO
static glprocaddress_t findGetProcAddressPlatformFct(int type, int gl_api)
{
    uint64_t key = (((uint64_t)type)<<32) | (gl_api);
    #define GO(A) if(my_getprocaddress_platform_##A == key) return my_getprocaddress_with_platform_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_getprocaddress_platform_##A == 0) {my_getprocaddress_platform_##A = key; return my_getprocaddress_with_platform_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstgl gst_gl_context_get_proc_address_with_platform callback\n");
    return NULL;
}

// getprocaddress_with_api ...
#define GO(A)   \
static int my_getprocaddress_api_##A = 0;                                                           \
static void* my_getprocaddress_with_api_##A(const char* name)                                       \
{                                                                                                   \
    return my->gst_gl_context_default_get_proc_address(my_getprocaddress_api_##A, (void*)name);     \
}
SUPER()
#undef GO
static glprocaddress_t findGetProcAddressAPIFct(int api)
{
    #define GO(A) if(my_getprocaddress_api_##A == api) return my_getprocaddress_with_api_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_getprocaddress_api_##A == 0) {my_getprocaddress_api_##A = api; return my_getprocaddress_with_api_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstgl gst_gl_context_default_get_proc_address callback\n");
    return NULL;
}
// GDestroyFunc ...
#define GO(A)   \
static uintptr_t my_destroyfunc_fct_##A = 0;                               \
static int my_destroyfunc_##A(void* a, void* b)                            \
{                                                                          \
    return RunFunctionFmt(my_destroyfunc_fct_##A, "pp", a, b); \
}
SUPER()
#undef GO
static void* findDestroyFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_destroyfunc_fct_##A == (uintptr_t)fct) return my_destroyfunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_destroyfunc_fct_##A == 0) {my_destroyfunc_fct_##A = (uintptr_t)fct; return my_destroyfunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstgl GDestroyNotify callback\n");
    return NULL;
}
//GstGLContextThreadFunc
#define GO(A)   \
static uintptr_t my_GstGLContextThreadFunc_fct_##A = 0;             \
static void my_GstGLContextThreadFunc_##A(void* a, void* b)         \
{                                                                   \
    RunFunctionFmt(my_GstGLContextThreadFunc_fct_##A, "pp", a, b);  \
}
SUPER()
#undef GO
static void* findGstGLContextThreadFuncFct(void* fct)
{
    if(!fct) return fct;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_GstGLContextThreadFunc_fct_##A == (uintptr_t)fct) return my_GstGLContextThreadFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstGLContextThreadFunc_fct_##A == 0) {my_GstGLContextThreadFunc_fct_##A = (uintptr_t)fct; return my_GstGLContextThreadFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstgl GstGLContextThreadFunc callback\n");
    return NULL;
}
//GstGLFilterRenderFunc
#define GO(A)   \
static uintptr_t my_GstGLFilterRenderFunc_fct_##A = 0;                              \
static int my_GstGLFilterRenderFunc_##A(void* a, void* b, void* c)                  \
{                                                                                   \
    return (int)RunFunctionFmt(my_GstGLFilterRenderFunc_fct_##A, "ppp", a, b, c);   \
}
SUPER()
#undef GO
static void* findGstGLFilterRenderFuncFct(void* fct)
{
    if(!fct) return fct;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_GstGLFilterRenderFunc_fct_##A == (uintptr_t)fct) return my_GstGLFilterRenderFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstGLFilterRenderFunc_fct_##A == 0) {my_GstGLFilterRenderFunc_fct_##A = (uintptr_t)fct; return my_GstGLFilterRenderFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstgl GstGLFilterRenderFunc callback\n");
    return NULL;
}
//GstGLFramebufferFunc
#define GO(A)   \
static uintptr_t my_GstGLFramebufferFunc_fct_##A = 0;                       \
static int my_GstGLFramebufferFunc_##A(void* a)                             \
{                                                                           \
    return (int)RunFunctionFmt(my_GstGLFramebufferFunc_fct_##A, "p", a);    \
}
SUPER()
#undef GO
static void* findGstGLFramebufferFuncFct(void* fct)
{
    if(!fct) return fct;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_GstGLFramebufferFunc_fct_##A == (uintptr_t)fct) return my_GstGLFramebufferFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstGLFramebufferFunc_fct_##A == 0) {my_GstGLFramebufferFunc_fct_##A = (uintptr_t)fct; return my_GstGLFramebufferFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstgl GstGLFramebufferFunc callback\n");
    return NULL;
}
//GstGLWindowCB
#define GO(A)   \
static uintptr_t my_GstGLWindowCB_fct_##A = 0;          \
static void my_GstGLWindowCB_##A(void* a)               \
{                                                       \
    RunFunctionFmt(my_GstGLWindowCB_fct_##A, "p", a);   \
}
SUPER()
#undef GO
static void* findGstGLWindowCBFct(void* fct)
{
    if(!fct) return fct;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_GstGLWindowCB_fct_##A == (uintptr_t)fct) return my_GstGLWindowCB_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstGLWindowCB_fct_##A == 0) {my_GstGLWindowCB_fct_##A = (uintptr_t)fct; return my_GstGLWindowCB_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstgl GstGLWindowCB callback\n");
    return NULL;
}
#undef SUPER

EXPORT void* my_gst_gl_context_get_proc_address_with_platform(x64emu_t* emu, int type, int gl_api, const char* name)
{
    return getGLProcAddress(emu, findGetProcAddressPlatformFct(type, gl_api), name);
}

EXPORT void* my_gst_gl_context_default_get_proc_address(x64emu_t* emu, int gl_api, const char* name)
{
    return getGLProcAddress(emu, findGetProcAddressAPIFct(gl_api), name);
}

EXPORT void my_gst_gl_context_thread_add(x64emu_t* emu, void* context, void* f, void* data)
{
    my->gst_gl_context_thread_add(context, findGstGLContextThreadFuncFct(f), data);
}

EXPORT int my_gst_gl_filter_render_to_target(x64emu_t* emu, void* filter, void* input, void* output, void* f, void* data)
{
    return my->gst_gl_filter_render_to_target(filter, input, output, findGstGLFilterRenderFuncFct(f), data);
}

EXPORT int my_gst_gl_framebuffer_draw_to_texture(x64emu_t* emu, void* fb, void* mem, void* f, void* data)
{
    return my->gst_gl_framebuffer_draw_to_texture(fb, mem, findGstGLFramebufferFuncFct(f), data);
}

EXPORT void my_gst_gl_insert_debug_marker(x64emu_t* emu, void* context, void* fmt, uintptr_t* b)
{
    #ifdef PREFER_CONVERT_VAARG
    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 2);
    #else
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 2);
    PREPARE_VALIST;
    #endif
    char* buff = NULL;
    if(vasprintf(&buff, (const char*)fmt, VARARGS)<0)
        return; // meh
    my->gst_gl_insert_debug_marker(context, buff);
    free(buff);
}

EXPORT void my_gst_gl_window_send_message(x64emu_t* emu, void* window, void* f, void* data)
{
    my->gst_gl_window_send_message(window, findGstGLWindowCBFct(f), data);
}

EXPORT void my_gst_gl_window_set_close_callback(x64emu_t* emu, void* window, void* f, void* data, void* d)
{
    my->gst_gl_window_set_close_callback(window, findGstGLWindowCBFct(f), data, findDestroyFct(d));
}

EXPORT void my_gst_gl_window_set_draw_callback(x64emu_t* emu, void* window, void* f, void* data, void* d)
{
    my->gst_gl_window_set_draw_callback(window, findGstGLWindowCBFct(f), data, findDestroyFct(d));
}

EXPORT void my_gst_gl_window_set_resize_callback(x64emu_t* emu, void* window, void* f, void* data, void* d)
{
    my->gst_gl_window_set_resize_callback(window, findGstGLWindowCBFct(f), data, findDestroyFct(d));
}

#define PRE_INIT \
    if (BOX64ENV(nogtk)) return -2;

#define CUSTOM_INIT \
    SetGstGLBaseFilterID(my->gst_gl_base_filter_get_type());        \
    SetGstGLFilterID(my->gst_gl_filter_get_type());                 \
    SetGstGLBaseSrcID(my->gst_gl_base_src_get_type());

#define NEEDED_LIBS "libGL.so.1", "libgstreamer-1.0.so.0"

#include "wrappedlib_init.h"
