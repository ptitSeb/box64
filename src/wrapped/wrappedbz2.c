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

const char* bz2Name = "libbz2.so.1";
#define LIBNAME bz2

#include "generated/wrappedbz2types.h"

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
static void* my_alloc_##A(void* opaque, int m, int n)                           \
{                                                                               \
    return (void*)RunFunctionFmt(my_alloc_fct_##A, "pii", opaque, m, n);  \
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
    printf_log(LOG_NONE, "Warning, no more slot for bz2 alloc callback\n");
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
    return (void*)AddBridge(my_lib->w.bridge, pFpii, fct, 0, NULL);
}
// free ...
#define GO(A)   \
static uintptr_t my_free_fct_##A = 0;                       \
static void my_free_##A(void* opaque, void* p)              \
{                                                           \
    RunFunctionFmt(my_free_fct_##A, "pp", opaque, p); \
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
    printf_log(LOG_NONE, "Warning, no more slot for bz2 free callback\n");
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

typedef struct {
    char *next_in;
    unsigned int avail_in;
    unsigned int total_in_lo32;
    unsigned int total_in_hi32;

    char *next_out;
    unsigned int avail_out;
    unsigned int total_out_lo32;
    unsigned int total_out_hi32;

    void *state;

    void *(*bzalloc)(void *,int,int);
    void (*bzfree)(void *,void *);
    void *opaque;
} my_bz_stream_t;


#define WRAP_BZ(A) \
    A->bzalloc = find_alloc_Fct(A->bzalloc); \
    A->bzfree = find_free_Fct(A->bzfree);

#define UNWRAP_BZ(A) \
    if(A->bzalloc) A->bzalloc = reverse_alloc_Fct(A->bzalloc); \
    if(A->bzfree) A->bzfree = reverse_free_Fct(A->bzfree);

EXPORT int my_BZ2_bzCompressInit(x64emu_t* emu, my_bz_stream_t* strm, int blocksize, int verbosity, int work)
{
    (void)emu;
    WRAP_BZ(strm);
    int ret = my->BZ2_bzCompressInit(strm, blocksize, verbosity, work);
    UNWRAP_BZ(strm);
    return ret;
}

EXPORT int my_BZ2_bzCompress(x64emu_t* emu, my_bz_stream_t* strm, int action)
{
    (void)emu;
    WRAP_BZ(strm);
    int ret = my->BZ2_bzCompress(strm, action);
    UNWRAP_BZ(strm);
    return ret;
}

EXPORT int my_BZ2_bzCompressEnd(x64emu_t* emu, my_bz_stream_t* strm)
{
    (void)emu;
    WRAP_BZ(strm);
    int ret = my->BZ2_bzCompressEnd(strm);
    UNWRAP_BZ(strm);
    return ret;
}

EXPORT int my_BZ2_bzDecompressInit(x64emu_t* emu, my_bz_stream_t* strm, int verbosity, int small)
{
    (void)emu;
    WRAP_BZ(strm);
    int ret = my->BZ2_bzDecompressInit(strm, verbosity, small);
    UNWRAP_BZ(strm);
    return ret;
}

EXPORT int my_BZ2_bzDecompress(x64emu_t* emu, my_bz_stream_t* strm)
{
    (void)emu;
    WRAP_BZ(strm);
    int ret = my->BZ2_bzDecompress(strm);
    UNWRAP_BZ(strm);
    return ret;
}

EXPORT int my_BZ2_bzDecompressEnd(x64emu_t* emu, my_bz_stream_t* strm)
{
    (void)emu;
    WRAP_BZ(strm);
    int ret = my->BZ2_bzDecompressEnd(strm);
    UNWRAP_BZ(strm);
    return ret;
}

#include "wrappedlib_init.h"
