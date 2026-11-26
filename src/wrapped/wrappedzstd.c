// SPDX-License-Identifier: MIT
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "callback.h"
#include "debug.h"
#include "emu/x64emu_private.h"

const char* zstdName = "libzstd.so.1";
#define LIBNAME zstd
#define ALTNAME "libzstd.so"


typedef void* (*my_ZSTD_allocFunction)(void*, size_t);
typedef void (*my_ZSTD_freeFunction)(void*, void*);

typedef struct
{
    my_ZSTD_allocFunction customAlloc;
    my_ZSTD_freeFunction  customFree;
    void* opaque;
} my_ZSTD_customMem;

#include "generated/wrappedzstdtypes.h"
#include "wrappercallback.h"

#undef SUPER

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

#define GO(A)                                            \
static uintptr_t my_alloc_mem_fct_##A = 0;               \
static uintptr_t my_free_mem_fct_##A = 0;                \
static void* my_alloc_mem_##A(void* opaque, size_t size) \
{                                                        \
    return (void*)RunFunctionFmt(my_alloc_mem_fct_##A, "pL", opaque, size); \
}                                                        \
static void my_free_mem_##A(void* opaque, void* address) \
{                                                        \
    RunFunctionFmt(my_free_mem_fct_##A, "pp", opaque, address); \
}

SUPER()
#undef GO

static void* find_alloc_mem_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_alloc_mem_fct_##A == (uintptr_t)fct) return my_alloc_mem_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_alloc_mem_fct_##A == 0) {my_alloc_mem_fct_##A = (uintptr_t)fct; return my_alloc_mem_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for zstd alloc callback\n");
    return NULL;
}

static void* find_free_mem_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_free_mem_fct_##A == (uintptr_t)fct) return my_free_mem_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_free_mem_fct_##A == 0) {my_free_mem_fct_##A = (uintptr_t)fct; return my_free_mem_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for zstd free callback\n");
    return NULL;
}

#undef SUPER

EXPORT void* my_ZSTD_createCStream_advanced(x64emu_t* emu, void* customAlloc, void* customFree, void* opaque)
{
    my_ZSTD_customMem mem;
    mem.customAlloc = find_alloc_mem_Fct(customAlloc);
    mem.customFree  = find_free_mem_Fct(customFree);
    mem.opaque = opaque;

    return my->ZSTD_createCStream_advanced(mem);
}

EXPORT void* my_ZSTD_createDStream_advanced(x64emu_t* emu, void* customAlloc, void* customFree, void* opaque)
{
    my_ZSTD_customMem mem;
    mem.customAlloc = find_alloc_mem_Fct(customAlloc);
    mem.customFree  = find_free_mem_Fct(customFree);
    mem.opaque = opaque;

    return my->ZSTD_createDStream_advanced(mem);
}

#include "wrappedlib_init.h"
