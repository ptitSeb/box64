#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "debug.h"
#include "callback.h"
#include "emu/x64emu_private.h"
#include "box64context.h"

#ifdef ANDROID
    const char* libzName = "libz.so";
#else
    const char* libzName = "libz.so.1";
#endif

#define LIBNAME libz

#include "generated/wrappedlibztypes.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// alloc ...
#define GO(A)   \
static uintptr_t my_alloc_fct_##A = 0;                                          \
static void* my_alloc_##A(void* opaque, uint32_t items, uint32_t size)                  \
{                                                                                       \
    return (void*)RunFunctionFmt(my_alloc_fct_##A, "puu", opaque, items, size);    \
}
SUPER()
#undef GO
static void* find_alloc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_alloc_fct_##A == (uintptr_t)fct) return my_alloc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_alloc_fct_##A == 0) {my_alloc_fct_##A = (uintptr_t)fct; return my_alloc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for zlib alloc callback\n");
    return NULL;
}
static void* reverse_alloc_Fct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_alloc_##A == fct) return (void*)my_alloc_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, pFpuu, fct, 0, NULL);
}
// free ...
#define GO(A)   \
static uintptr_t my_free_fct_##A = 0;                               \
static void my_free_##A(void* opaque, void* address)                \
{                                                                   \
    RunFunctionFmt(my_free_fct_##A, "pp", opaque, address);   \
}
SUPER()
#undef GO
static void* find_free_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_free_fct_##A == (uintptr_t)fct) return my_free_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_free_fct_##A == 0) {my_free_fct_##A = (uintptr_t)fct; return my_free_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for zlib free callback\n");
    return NULL;
}
static void* reverse_free_Fct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_free_##A == fct) return (void*)my_free_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, vFpp, fct, 0, NULL);
}
#undef SUPER

typedef struct z_stream_s {
    void *next_in;
    uint32_t     avail_in;
    uintptr_t    total_in;
    void    *next_out;
    uint32_t     avail_out;
    uintptr_t    total_out;
    char *msg;
    void *state;
    void* zalloc;
    void*  zfree;
    void*     opaque;
    int32_t     data_type;
    uintptr_t   adler;
    uintptr_t   reserved;
} z_stream;

static void wrapper_stream_z(x64emu_t* emu, void* str)
{
    (void)emu;
    z_stream *stream = (z_stream*)str;
    stream->zalloc = find_alloc_Fct(stream->zalloc);
    stream->zfree = find_free_Fct(stream->zfree);
}
static void unwrapper_stream_z(x64emu_t* emu, void* str)
{
    (void)emu;
    z_stream *stream = (z_stream*)str;
    stream->zalloc = reverse_alloc_Fct(stream->zalloc);
    stream->zfree = reverse_free_Fct(stream->zfree);
}

EXPORT int my_inflateInit_(x64emu_t* emu, void* str, void* version, int size)
{
    wrapper_stream_z(emu, str);
    int ret = my->inflateInit_(str, version, size);
    unwrapper_stream_z(emu, str);
    return ret;
}

EXPORT int my_inflateInit2_(x64emu_t* emu, void* str, int windowBits, void* version, int stream_size)
{
    wrapper_stream_z(emu, str);
    int ret = my->inflateInit2_(str, windowBits, version, stream_size);
    unwrapper_stream_z(emu, str);
    return ret;
}

EXPORT int my_inflateBackInit_(x64emu_t* emu, void* str, int windowBits, void *window, void* version, int size)
{
    wrapper_stream_z(emu, str);
    int ret = my->inflateBackInit_(str, windowBits, window, version, size);
    unwrapper_stream_z(emu, str);
    return ret;
}

EXPORT int my_inflateEnd(x64emu_t* emu, void* str)
{
    wrapper_stream_z(emu, str);
    int r = my->inflateEnd(str);
    return r;
}

EXPORT int my_deflateInit_(x64emu_t* emu, void* str, int level, void* version, int stream_size)
{
    wrapper_stream_z(emu, str);
    int ret = my->deflateInit_(str, level, version, stream_size);
    unwrapper_stream_z(emu, str);
    return ret;
}

EXPORT int my_deflateInit2_(x64emu_t* emu, void* str, int level, int method, int windowBits, int memLevel, int strategy, void* version, int stream_size)
{
    wrapper_stream_z(emu, str);
    int ret = my->deflateInit2_(str, level, method, windowBits, memLevel, strategy, version, stream_size);
    unwrapper_stream_z(emu, str);
    return ret;
}

// TODO: remove this?
EXPORT int my_deflateEnd(x64emu_t* emu, void* str)
{
    wrapper_stream_z(emu, str);
    int r = my->deflateEnd(str);
    return r;
}

EXPORT int my_inflate(x64emu_t* emu, void* str, int flush)
{
    wrapper_stream_z(emu, str);
    int ret = my->inflate(str, flush);
    unwrapper_stream_z(emu, str);
    return ret;
}

#include "wrappedlib_init.h"
