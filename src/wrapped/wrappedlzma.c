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
#include "callback.h"
#include "librarian.h"
#include "box64context.h"
#include "emu/x64emu_private.h"
#include "myalign.h"

const char* lzmaName = "liblzma.so.5";
#define LIBNAME lzma

#include "generated/wrappedlzmatypes.h"

#include "wrappercallback.h"

typedef struct lzma_allocator_s {
    void *(*alloc)(void *opaque, size_t nmemb, size_t size);
    void (*free)(void *opaque, void *ptr);
    void *opaque;
} lzma_allocator_t;

typedef struct lzma_stream_s {
    const uint8_t *next_in;
    size_t avail_in;
    uint64_t total_in;
    uint8_t *next_out;
    size_t avail_out;
    uint64_t total_out;
    lzma_allocator_t *allocator;
    void* internal;
    void *reserved_ptr1;
    void *reserved_ptr2;
    void *reserved_ptr3;
    void *reserved_ptr4;
    uint64_t reserved_int1;
    uint64_t reserved_int2;
    size_t reserved_int3;
    size_t reserved_int4;
    int reserved_enum1;
    int reserved_enum2;
} lzma_stream_t;

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// alloc ...
#define GO(A)   \
static uintptr_t my_alloc_fct_##A = 0;                                              \
static void* my_alloc_##A(void* opaque, size_t items, size_t size)                  \
{                                                                                   \
    return (void*)RunFunctionFmt(my_alloc_fct_##A, "pLL", opaque, items, size);\
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

static void wrap_alloc_struct(lzma_allocator_t* a)
{
    if(!a)
        return;
    a->alloc = find_alloc_Fct(a->alloc);
    a->free = find_free_Fct(a->free);

}
static void unwrap_alloc_struct(lzma_allocator_t* a)
{
    if(!a)
        return;
    a->alloc = reverse_alloc_Fct(a->alloc);
    a->free = reverse_free_Fct(a->free);

}

EXPORT int my_lzma_index_buffer_decode(x64emu_t* emu, void* i, void* memlimit, lzma_allocator_t* alloc, void* in_, void* in_pos, size_t in_size)
{
    wrap_alloc_struct(alloc);
    int ret = my->lzma_index_buffer_decode(i, memlimit, alloc, in_, in_pos, in_size);
    unwrap_alloc_struct(alloc);
    return ret;
}

EXPORT void my_lzma_index_end(x64emu_t* emu, void* i, lzma_allocator_t* alloc)
{
    wrap_alloc_struct(alloc);
    my->lzma_index_end(i,alloc);
    unwrap_alloc_struct(alloc);
}

EXPORT int my_lzma_stream_buffer_decode(x64emu_t* emu, void* memlimit, uint32_t flags, lzma_allocator_t* alloc, void* in_, void* in_pos, size_t in_size, void* out_, void* out_pos, size_t out_size)
{
    wrap_alloc_struct(alloc);
    int ret = my->lzma_stream_buffer_decode(memlimit, flags, alloc, in_, in_pos, in_size, out_, out_pos, out_size);
    unwrap_alloc_struct(alloc);
    return ret;
}

EXPORT int my_lzma_stream_decoder(x64emu_t* emu, lzma_stream_t* stream, uint64_t memlimit, uint32_t flags)
{
    wrap_alloc_struct(stream->allocator);
    int ret = my->lzma_stream_decoder(stream, memlimit, flags);
    unwrap_alloc_struct(stream->allocator);
    return ret;
}

EXPORT int my_lzma_stream_encoder(x64emu_t* emu, lzma_stream_t* stream, void* filters, int check)
{
    wrap_alloc_struct(stream->allocator);
    int ret = my->lzma_stream_encoder(stream, filters, check);
    unwrap_alloc_struct(stream->allocator);
    return ret;
}

EXPORT int my_lzma_easy_encoder(x64emu_t* emu, lzma_stream_t* stream, uint32_t precheck, uint32_t check)
{
    wrap_alloc_struct(stream->allocator);
    int ret = my->lzma_easy_encoder(stream, precheck, check);
    unwrap_alloc_struct(stream->allocator);
    return ret;
}

EXPORT int my_lzma_raw_encoder(x64emu_t* emu, lzma_stream_t* stream, void* filters)
{
    wrap_alloc_struct(stream->allocator);
    int ret = my->lzma_raw_encoder(stream, filters);
    unwrap_alloc_struct(stream->allocator);
    return ret;
}

EXPORT int my_lzma_raw_decoder(x64emu_t* emu, lzma_stream_t* stream, void* filters)
{
    wrap_alloc_struct(stream->allocator);
    int ret = my->lzma_raw_decoder(stream, filters);
    unwrap_alloc_struct(stream->allocator);
    return ret;
}

EXPORT int my_lzma_properties_decode(x64emu_t* emu, void* filters, lzma_allocator_t* allocator, void* props, size_t size)
{
    wrap_alloc_struct(allocator);
    int ret = my->lzma_properties_decode(filters, allocator, props, size);
    unwrap_alloc_struct(allocator);
    return ret;
}

EXPORT int my_lzma_alone_decoder(x64emu_t* emu, lzma_stream_t* stream, uint64_t memlimit)
{
    wrap_alloc_struct(stream->allocator);
    int ret = my->lzma_alone_decoder(stream, memlimit);
    unwrap_alloc_struct(stream->allocator);
    return ret;
}

EXPORT int my_lzma_alone_encoder(x64emu_t* emu, lzma_stream_t* stream, void* options)
{
    wrap_alloc_struct(stream->allocator);
    int ret = my->lzma_alone_encoder(stream, options);
    unwrap_alloc_struct(stream->allocator);
    return ret;
}

EXPORT int my_lzma_stream_encoder_mt(x64emu_t* emu, lzma_stream_t* stream, void* options)
{
    wrap_alloc_struct(stream->allocator);
    int ret = my->lzma_stream_encoder_mt(stream, options);
    unwrap_alloc_struct(stream->allocator);
    return ret;
}

EXPORT int my_lzma_stream_decoder_mt(x64emu_t* emu, lzma_stream_t* stream, const void* options)
{
    wrap_alloc_struct(stream->allocator);
    int ret = my->lzma_stream_decoder_mt(stream, options);
    unwrap_alloc_struct(stream->allocator);
    return ret;
}

EXPORT int my_lzma_code(x64emu_t* emu, lzma_stream_t* stream, int a)
{
    wrap_alloc_struct(stream->allocator);
    int ret = my->lzma_code(stream, a);
    unwrap_alloc_struct(stream->allocator);
    return ret;
}

EXPORT void my_lzma_end(x64emu_t* emu, lzma_stream_t* stream)
{
    wrap_alloc_struct(stream->allocator);
    my->lzma_end(stream);
}

#include "wrappedlib_init.h"
