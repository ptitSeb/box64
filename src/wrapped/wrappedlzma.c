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
    return (void*)RunFunction(my_context, my_alloc_fct_##A, 3, opaque, items, size);\
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

static void wrap_alloc_struct(lzma_allocator_t* dst, lzma_allocator_t* src)
{
    if(!src)
        return;
    dst->opaque = src->opaque;
    dst->alloc = find_alloc_Fct(src->alloc);
    dst->free = find_free_Fct(src->free);
    
}

EXPORT int my_lzma_index_buffer_decode(x64emu_t* emu, void* i, void* memlimit, lzma_allocator_t* alloc, void* in_, void* in_pos, size_t in_size)
{
    lzma_allocator_t allocator = {0};
    wrap_alloc_struct(&allocator, alloc);
    return my->lzma_index_buffer_decode(i, memlimit, alloc?&allocator:NULL, in_, in_pos, in_size);
}

EXPORT void my_lzma_index_end(x64emu_t* emu, void* i, lzma_allocator_t* alloc)
{
    lzma_allocator_t allocator = {0};
    wrap_alloc_struct(&allocator, alloc);
    return my->lzma_index_end(i,alloc?&allocator:NULL);
}

EXPORT int my_lzma_stream_buffer_decode(x64emu_t* emu, void* memlimit, uint32_t flags, lzma_allocator_t* alloc, void* in_, void* in_pos, size_t in_size, void* out_, void* out_pos, size_t out_size)
{
    lzma_allocator_t allocator = {0};
    wrap_alloc_struct(&allocator, alloc);
    return my->lzma_stream_buffer_decode(memlimit, flags, alloc?&allocator:NULL, in_, in_pos, in_size, out_, out_pos, out_size);
}

EXPORT int my_lzma_stream_decoder(x64emu_t* emu, lzma_stream_t* stream, uint64_t memlimit, uint32_t flags)
{
    // not restoring the allocator after, so lzma_code and lzma_end can be used without "GOM" wrapping
    if(stream->allocator)
        wrap_alloc_struct(stream->allocator, stream->allocator);
    return my->lzma_stream_decoder(stream, memlimit, flags);
}

#define CUSTOM_INIT \
    getMy(lib);

#define CUSTOM_FINI \
    freeMy();

#include "wrappedlib_init.h"
