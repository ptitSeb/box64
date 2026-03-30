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

const char* libvpxName = "libvpx.so.9";
#define ALTNAME "libvpx.so"

#define LIBNAME libvpx

#define ADDED_FUNCTIONS()       \

#include "generated/wrappedlibvpxtypes.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)


// vpx_get_frame_buffer_cb_fn_t
#define GO(A)   \
static uintptr_t my_vpx_get_frame_buffer_cb_fn_t_fct_##A = 0;                               \
static int my_vpx_get_frame_buffer_cb_fn_t_##A(void* a, size_t b, void* c)                  \
{                                                                                           \
    return (int)RunFunctionFmt(my_vpx_get_frame_buffer_cb_fn_t_fct_##A, "pLp", a, b, c);    \
}
SUPER()
#undef GO
static void* findvpx_get_frame_buffer_cb_fn_tFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_vpx_get_frame_buffer_cb_fn_t_fct_##A == (uintptr_t)fct) return my_vpx_get_frame_buffer_cb_fn_t_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_vpx_get_frame_buffer_cb_fn_t_fct_##A == 0) {my_vpx_get_frame_buffer_cb_fn_t_fct_##A = (uintptr_t)fct; return my_vpx_get_frame_buffer_cb_fn_t_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libvpx vpx_get_frame_buffer_cb_fn_t callback\n");
    return NULL;
}
// vpx_release_frame_buffer_cb_fn_t
#define GO(A)   \
static uintptr_t my_vpx_release_frame_buffer_cb_fn_t_fct_##A = 0;                           \
static int my_vpx_release_frame_buffer_cb_fn_t_##A(void* a, void* b)                        \
{                                                                                           \
    return (int)RunFunctionFmt(my_vpx_release_frame_buffer_cb_fn_t_fct_##A, "pp", a, b);    \
}
SUPER()
#undef GO
static void* findvpx_release_frame_buffer_cb_fn_tFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_vpx_release_frame_buffer_cb_fn_t_fct_##A == (uintptr_t)fct) return my_vpx_release_frame_buffer_cb_fn_t_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_vpx_release_frame_buffer_cb_fn_t_fct_##A == 0) {my_vpx_release_frame_buffer_cb_fn_t_fct_##A = (uintptr_t)fct; return my_vpx_release_frame_buffer_cb_fn_t_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libvpx vpx_release_frame_buffer_cb_fn_t callback\n");
    return NULL;
}

#undef SUPER

EXPORT int my_vpx_codec_set_frame_buffer_functions(x64emu_t* emu, void* ctx, void* cb_get, void* cb_release, void* data)
{
    return my->vpx_codec_set_frame_buffer_functions(ctx, findvpx_get_frame_buffer_cb_fn_tFct(cb_get), findvpx_release_frame_buffer_cb_fn_tFct(cb_release), data);
}

#include "wrappedlib_init.h"