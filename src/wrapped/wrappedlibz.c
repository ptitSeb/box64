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

const char* libzName = "libz.so.1";
#define LIBNAME libz

#include "generated/wrappedlibztypes.h"

typedef struct libz_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} libz_my_t;


void* getZMy(library_t* lib)
{
    libz_my_t* my = (libz_my_t*)calloc(1, sizeof(libz_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeZMy(void* lib)
{
    (void)lib;
    //libz_my_t *my = (libz_my_t *)lib;
}

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
    return (void*)RunFunction(my_context, my_alloc_fct_##A, 3, opaque, items, size);    \
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
// free ...
#define GO(A)   \
static uintptr_t my_free_fct_##A = 0;                               \
static void my_free_##A(void* opaque, void* address)                \
{                                                                   \
    RunFunction(my_context, my_free_fct_##A, 2, opaque, address);   \
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

EXPORT int my_inflateInit_(x64emu_t* emu, void* str, void* version, int size)
{
    libz_my_t *my = (libz_my_t *)emu->context->zlib->priv.w.p2;
    wrapper_stream_z(emu, str);
    return my->inflateInit_(str, version, size);
}

EXPORT int my_inflateInit2_(x64emu_t* emu, void* str, int windowBits, void* version, int stream_size)
{
    libz_my_t *my = (libz_my_t *)emu->context->zlib->priv.w.p2;
    wrapper_stream_z(emu, str);
    return my->inflateInit2_(str, windowBits, version, stream_size);
}

EXPORT int my_inflateBackInit_(x64emu_t* emu, void* strm, int windowBits, void *window, void* version, int size)
{
    libz_my_t *my = (libz_my_t *)emu->context->zlib->priv.w.p2;
    wrapper_stream_z(emu, strm);
    return my->inflateBackInit_(strm, windowBits, window, version, size);
}

// TODO: remove this?
EXPORT int my_inflateEnd(x64emu_t* emu, void* str)
{
    libz_my_t *my = (libz_my_t *)emu->context->zlib->priv.w.p2;
    int r = my->inflateEnd(str);
    return r;
}

EXPORT int my_deflateInit_(x64emu_t* emu, void* str, int level, void* version, int stream_size)
{
    libz_my_t *my = (libz_my_t *)emu->context->zlib->priv.w.p2;
    wrapper_stream_z(emu, str);
    return my->deflateInit_(str, level, version, stream_size);
}

EXPORT int my_deflateInit2_(x64emu_t* emu, void* str, int level, int method, int windowBits, int memLevel, int strategy, void* version, int stream_size)
{
    libz_my_t *my = (libz_my_t *)emu->context->zlib->priv.w.p2;
    wrapper_stream_z(emu, str);
    return my->deflateInit2_(str, level, method, windowBits, memLevel, strategy, version, stream_size);
}

// TODO: remove this?
EXPORT int my_deflateEnd(x64emu_t* emu, void* str)
{
    libz_my_t *my = (libz_my_t *)emu->context->zlib->priv.w.p2;
    int r = my->deflateEnd(str);
    return r;
}


#define CUSTOM_INIT \
    box64->zlib = lib; \
    lib->priv.w.p2 = getZMy(lib);

#define CUSTOM_FINI \
    freeZMy(lib->priv.w.p2); \
    free(lib->priv.w.p2); \
    ((box64context_t*)(lib->context))->zlib = NULL;

#include "wrappedlib_init.h"
