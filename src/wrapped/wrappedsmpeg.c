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
#include "librarian.h"
#include "x64emu.h"
#include "callback.h"
#include "box64context.h"
#include "sdl1rwops.h"
#include "myalign.h"

const char* smpegName = "libsmpeg-0.4.so.0";
#define LIBNAME smpeg

typedef void (*vFpp_t)(void*, void*);
typedef void (*vFpppp_t)(void*, void*, void*, void*);
typedef void* (*pFppi_t)(void*, void*, int32_t);
typedef void* (*pFipi_t)(int32_t, void*, int32_t);
typedef void* (*pFpipi_t)(void*, int32_t, void*, int32_t);

typedef struct smpeg_my_s {
    // functions
    vFpppp_t    SMPEG_setdisplay;
    pFppi_t     SMPEG_new_rwops;
} smpeg_my_t;

static void* getSMPEGMy(library_t* lib)
{
    smpeg_my_t* my = (smpeg_my_t*)calloc(1, sizeof(smpeg_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    GO(SMPEG_setdisplay, vFpppp_t)
    GO(SMPEG_new_rwops, pFppi_t)
    #undef GO
    return my;
}

static void freeSMPEGMy(void* lib)
{
    //smpeg_my_t *my = (smpeg_my_t *)lib;
}

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// dispcallback ...
#define GO(A)   \
static uintptr_t my_dispcallback_fct_##A = 0;                                                   \
static void my_dispcallback_##A(void* dst, int32_t x, int32_t y, unsigned int w, unsigned int h)\
{                                                                                               \
    RunFunction(my_context, my_dispcallback_fct_##A, 5, dst, x, y, w, h);                       \
}
SUPER()
#undef GO
static void* find_dispcallback_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_dispcallback_fct_##A == (uintptr_t)fct) return my_dispcallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_dispcallback_fct_##A == 0) {my_dispcallback_fct_##A = (uintptr_t)fct; return my_dispcallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libsmpeg dispcallback callback\n");
    return NULL;
}
#undef SUPER

EXPORT void my_SMPEG_setdisplay(x64emu_t* emu, void* mpeg, void* surf, void* lock, void* cb)
{
    library_t* lib = GetLibInternal(smpegName);
    smpeg_my_t* my = (smpeg_my_t*)lib->priv.w.p2;
    my->SMPEG_setdisplay(mpeg, surf, lock, find_dispcallback_Fct(cb));
}

EXPORT void* my_SMPEG_new_rwops(x64emu_t* emu, void* src, void* info, int32_t sdl_audio)
{
    library_t* lib = GetLibInternal(smpegName);
    smpeg_my_t* my = (smpeg_my_t*)lib->priv.w.p2;
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)src);
    void* ret = my->SMPEG_new_rwops(rw, info, sdl_audio);
    RWNativeEnd(rw);
    return ret;
}

#define CUSTOM_INIT \
    lib->priv.w.p2 = getSMPEGMy(lib);

#define CUSTOM_FINI \
    freeSMPEGMy(lib->priv.w.p2); \
    free(lib->priv.w.p2); \

#include "wrappedlib_init.h"

