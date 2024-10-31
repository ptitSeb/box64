#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>
#include <stdarg.h>

#include "wrappedlibs.h"

#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "debug.h"
#include "myalign.h"
#include "callback.h"
#include "emu/x64emu_private.h"

const char* brotlidecName = "libbrotlidec.so.1";
#define LIBNAME brotlidec

#define ADDED_FUNCTIONS()           \

#include "generated/wrappedbrotlidectypes.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// brotli_alloc_func ...
#define GO(A)   \
static uintptr_t my_brotli_alloc_func_fct_##A = 0;                          \
static void* my_brotli_alloc_func_##A(void* a, size_t b)                    \
{                                                                           \
    return (void*)RunFunctionFmt(my_brotli_alloc_func_fct_##A, "pL", a, b); \
}
SUPER()
#undef GO
static void* find_brotli_alloc_func_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_brotli_alloc_func_fct_##A == (uintptr_t)fct) return my_brotli_alloc_func_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_brotli_alloc_func_fct_##A == 0) {my_brotli_alloc_func_fct_##A = (uintptr_t)fct; return my_brotli_alloc_func_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for brotidec brotli_alloc_func callback\n");
    return NULL;
}
// brotli_free_func ...
#define GO(A)   \
static uintptr_t my_brotli_free_func_fct_##A = 0;               \
static void my_brotli_free_func_##A(void* a, void* b)           \
{                                                               \
    RunFunctionFmt(my_brotli_free_func_fct_##A, "pp", a, b);    \
}
SUPER()
#undef GO
static void* find_brotli_free_func_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_brotli_free_func_fct_##A == (uintptr_t)fct) return my_brotli_free_func_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_brotli_free_func_fct_##A == 0) {my_brotli_free_func_fct_##A = (uintptr_t)fct; return my_brotli_free_func_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for brotidec brotli_free_func callback\n");
    return NULL;
}
// brotli_decoder_metadata_start_func ...
#define GO(A)   \
static uintptr_t my_brotli_decoder_metadata_start_func_fct_##A = 0;             \
static void my_brotli_decoder_metadata_start_func_##A(void* a, size_t b)        \
{                                                                               \
    RunFunctionFmt(my_brotli_decoder_metadata_start_func_fct_##A, "pL", a, b);  \
}
SUPER()
#undef GO
static void* find_brotli_decoder_metadata_start_func_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_brotli_decoder_metadata_start_func_fct_##A == (uintptr_t)fct) return my_brotli_decoder_metadata_start_func_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_brotli_decoder_metadata_start_func_fct_##A == 0) {my_brotli_decoder_metadata_start_func_fct_##A = (uintptr_t)fct; return my_brotli_decoder_metadata_start_func_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for brotidec brotli_decoder_metadata_start_func callback\n");
    return NULL;
}
// brotli_decoder_metadata_chunk_func ...
#define GO(A)   \
static uintptr_t my_brotli_decoder_metadata_chunk_func_fct_##A = 0;                 \
static void my_brotli_decoder_metadata_chunk_func_##A(void* a, void* b, size_t c)   \
{                                                                                   \
    RunFunctionFmt(my_brotli_decoder_metadata_chunk_func_fct_##A, "ppL", a, b, c);  \
}
SUPER()
#undef GO
static void* find_brotli_decoder_metadata_chunk_func_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_brotli_decoder_metadata_chunk_func_fct_##A == (uintptr_t)fct) return my_brotli_decoder_metadata_chunk_func_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_brotli_decoder_metadata_chunk_func_fct_##A == 0) {my_brotli_decoder_metadata_chunk_func_fct_##A = (uintptr_t)fct; return my_brotli_decoder_metadata_chunk_func_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for brotidec brotli_decoder_metadata_chunk_func callback\n");
    return NULL;
}
#undef SUPER

EXPORT void* my_BrotliDecoderCreateInstance(x64emu_t* emu, void* m, void* f, void* d)
{
    return my->BrotliDecoderCreateInstance(find_brotli_alloc_func_Fct(m), find_brotli_free_func_Fct(f), d);
}

EXPORT void my_BrotliDecoderSetMetadataCallbacks(x64emu_t* emu, void* state, void* start, void* chunk, void* d)
{
    my->BrotliDecoderSetMetadataCallbacks(state, find_brotli_decoder_metadata_start_func_Fct(start), find_brotli_decoder_metadata_chunk_func_Fct(chunk), d);
}

#include "wrappedlib_init.h"
