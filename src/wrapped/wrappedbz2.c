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
static library_t* my_lib = NULL;

typedef int  (*iFp_t)(void*);
typedef int  (*iFpi_t)(void*, int);
typedef int  (*iFpii_t)(void*, int, int);
typedef int  (*iFpiii_t)(void*, int, int, int);

#define SUPER() \
    GO(BZ2_bzCompressInit, iFpiii_t)    \
    GO(BZ2_bzCompress, iFpi_t)          \
    GO(BZ2_bzCompressEnd, iFp_t)        \
    GO(BZ2_bzDecompressInit, iFpii_t)   \
    GO(BZ2_bzDecompress, iFp_t)         \
    GO(BZ2_bzDecompressEnd, iFp_t)

typedef struct bz2_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} bz2_my_t;

void* getBz2My(library_t* lib)
{
    bz2_my_t* my = (bz2_my_t*)calloc(1, sizeof(bz2_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeBz2My(void* lib)
{
    (void)lib;
    //bz2_my_t *my = (bz2_my_t *)lib;
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
static void* my_alloc_##A(void* opaque, int m, int n)                           \
{                                                                               \
    return (void*)RunFunction(my_context, my_alloc_fct_##A, 3, opaque, m, n);   \
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
    if(CheckBridged(my_lib->priv.w.bridge, fct))
        return (void*)CheckBridged(my_lib->priv.w.bridge, fct);
    #define GO(A) if(my_alloc_##A == fct) return (void*)my_alloc_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->priv.w.bridge, pFpii, fct, 0, NULL);
}
// free ...
#define GO(A)   \
static uintptr_t my_free_fct_##A = 0;                       \
static void my_free_##A(void* opaque, void* p)              \
{                                                           \
    RunFunction(my_context, my_free_fct_##A, 2, opaque, p); \
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
    if(CheckBridged(my_lib->priv.w.bridge, fct))
        return (void*)CheckBridged(my_lib->priv.w.bridge, fct);
    #define GO(A) if(my_free_##A == fct) return (void*)my_free_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->priv.w.bridge, vFpp, fct, 0, NULL);
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
    A->bzalloc = find_alloc_Fct(A->bzalloc);        \
    A->bzfree = find_free_Fct(A->bzfree);

#define UNWRAP_BZ(A) if(A->bzalloc || A->bzfree)    \
    A->bzalloc = reverse_alloc_Fct(A->bzalloc);     \
    A->bzfree = reverse_free_Fct(A->bzfree);

EXPORT int my_BZ2_bzCompressInit(x64emu_t* emu, my_bz_stream_t* strm, int blocksize, int verbosity, int work)
{
    (void)emu;
    library_t * lib = GetLibInternal(bz2Name);
    bz2_my_t *my = (bz2_my_t*)lib->priv.w.p2;
    WRAP_BZ(strm);
    int ret = my->BZ2_bzCompressInit(strm, blocksize, verbosity, work);
    UNWRAP_BZ(strm);
    return ret;
}

EXPORT int my_BZ2_bzCompress(x64emu_t* emu, my_bz_stream_t* strm, int action)
{
    (void)emu;
    library_t * lib = GetLibInternal(bz2Name);
    bz2_my_t *my = (bz2_my_t*)lib->priv.w.p2;
    WRAP_BZ(strm);
    int ret = my->BZ2_bzCompress(strm, action);
    UNWRAP_BZ(strm);
    return ret;
}

EXPORT int my_BZ2_bzCompressEnd(x64emu_t* emu, my_bz_stream_t* strm)
{
    (void)emu;
    library_t * lib = GetLibInternal(bz2Name);
    bz2_my_t *my = (bz2_my_t*)lib->priv.w.p2;
    WRAP_BZ(strm);
    int ret = my->BZ2_bzCompressEnd(strm);
    UNWRAP_BZ(strm);
    return ret;
}

EXPORT int my_BZ2_bzDecompressInit(x64emu_t* emu, my_bz_stream_t* strm, int verbosity, int small)
{
    (void)emu;
    library_t * lib = GetLibInternal(bz2Name);
    bz2_my_t *my = (bz2_my_t*)lib->priv.w.p2;
    WRAP_BZ(strm);
    int ret = my->BZ2_bzDecompressInit(strm, verbosity, small);
    UNWRAP_BZ(strm);
    return ret;
}

EXPORT int my_BZ2_bzDecompress(x64emu_t* emu, my_bz_stream_t* strm)
{
    (void)emu;
    library_t * lib = GetLibInternal(bz2Name);
    bz2_my_t *my = (bz2_my_t*)lib->priv.w.p2;
    WRAP_BZ(strm);
    int ret = my->BZ2_bzDecompress(strm);
    UNWRAP_BZ(strm);
    return ret;
}

EXPORT int my_BZ2_bzDecompressEnd(x64emu_t* emu, my_bz_stream_t* strm)
{
    (void)emu;
    library_t * lib = GetLibInternal(bz2Name);
    bz2_my_t *my = (bz2_my_t*)lib->priv.w.p2;
    WRAP_BZ(strm);
    int ret = my->BZ2_bzDecompressEnd(strm);
    UNWRAP_BZ(strm);
    return ret;
}

#define CUSTOM_INIT \
    lib->priv.w.p2 = getBz2My(lib); \
    my_lib = lib;

#define CUSTOM_FINI \
    freeBz2My(lib->priv.w.p2);  \
    free(lib->priv.w.p2);       \
    my_lib = NULL;

#include "wrappedlib_init.h"
