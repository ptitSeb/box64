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
#include "box64context.h"
#include "sdl1rwops.h"

typedef void* (*pFpi_t)(void*, int32_t);
typedef void* (*pFp_t)(void*);
typedef void* (*pFpip_t)(void*, int32_t, void*);

typedef struct sdl1image_my_s {
    pFp_t       IMG_LoadBMP_RW;
    pFp_t       IMG_LoadCUR_RW;
    pFp_t       IMG_LoadGIF_RW;
    pFp_t       IMG_LoadICO_RW;
    pFp_t       IMG_LoadJPG_RW;
    pFp_t       IMG_LoadLBM_RW;
    pFp_t       IMG_LoadPCX_RW;
    pFp_t       IMG_LoadPNG_RW;
    pFp_t       IMG_LoadPNM_RW;
    pFp_t       IMG_LoadTGA_RW;
    pFp_t       IMG_LoadTIF_RW;
    pFpip_t     IMG_LoadTyped_RW;
    pFp_t       IMG_LoadWEBP_RW;
    pFp_t       IMG_LoadXCF_RW;
    pFp_t       IMG_LoadXPM_RW;
    pFp_t       IMG_LoadXV_RW;
    pFpi_t      IMG_Load_RW;
} sdl1image_my_t;

static library_t* my_lib = NULL;

static void* getSDL1ImageMy(library_t* lib)
{
    sdl1image_my_t* my = (sdl1image_my_t*)calloc(1, sizeof(sdl1image_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    GO(IMG_LoadBMP_RW,pFp_t)
    GO(IMG_LoadCUR_RW,pFp_t)
    GO(IMG_LoadGIF_RW,pFp_t)
    GO(IMG_LoadICO_RW,pFp_t)
    GO(IMG_LoadJPG_RW,pFp_t)
    GO(IMG_LoadLBM_RW,pFp_t)
    GO(IMG_LoadPCX_RW,pFp_t)
    GO(IMG_LoadPNG_RW,pFp_t)
    GO(IMG_LoadPNM_RW,pFp_t)
    GO(IMG_LoadTGA_RW,pFp_t)
    GO(IMG_LoadTIF_RW,pFp_t)
    GO(IMG_LoadTyped_RW,pFpip_t)
    GO(IMG_LoadWEBP_RW,pFp_t)
    GO(IMG_LoadXCF_RW,pFp_t)
    GO(IMG_LoadXPM_RW,pFp_t)
    GO(IMG_LoadXV_RW,pFp_t)
    GO(IMG_Load_RW,pFpi_t)
    #undef GO
    return my;
}

#define GO(A) \
void EXPORT *my_##A(x64emu_t* emu, void* a) \
{ \
    sdl1image_my_t *my = (sdl1image_my_t *)my_lib->priv.w.p2; \
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a); \
    void* r = my->A(rw); \
    RWNativeEnd(rw); \
    return r; \
}
GO(IMG_LoadBMP_RW)
GO(IMG_LoadCUR_RW)
GO(IMG_LoadGIF_RW)
GO(IMG_LoadICO_RW)
GO(IMG_LoadJPG_RW)
GO(IMG_LoadLBM_RW)
GO(IMG_LoadPCX_RW)
GO(IMG_LoadPNG_RW)
GO(IMG_LoadPNM_RW)
GO(IMG_LoadTGA_RW)
GO(IMG_LoadTIF_RW)
GO(IMG_LoadWEBP_RW)
GO(IMG_LoadXCF_RW)
GO(IMG_LoadXPM_RW)
GO(IMG_LoadXV_RW)
#undef GO

void EXPORT *my_IMG_LoadTyped_RW(x64emu_t* emu, void* a, int32_t b, void* c)
{
    sdl1image_my_t *my = (sdl1image_my_t *)my_lib->priv.w.p2;
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    void* r = my->IMG_LoadTyped_RW(rw, b, c);
    if(b==0)
        RWNativeEnd(rw);
    return r;
}
void EXPORT *my_IMG_Load_RW(x64emu_t* emu, void* a, int32_t b)
{
    sdl1image_my_t *my = (sdl1image_my_t *)my_lib->priv.w.p2;
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    void* r = my->IMG_Load_RW(rw, b);
    if(b==0)
        RWNativeEnd(rw);
    return r;
}

const char* sdl1imageName = "libSDL_image-1.2.so.0";
#define LIBNAME sdl1image

#define CUSTOM_INIT \
    my_lib = lib; \
    lib->priv.w.p2 = getSDL1ImageMy(lib);   \
    lib->priv.w.needed = 2; \
    lib->priv.w.neededlibs = (char**)calloc(lib->priv.w.needed, sizeof(char*)); \
    lib->priv.w.neededlibs[0] = strdup("libSDL-1.2.so.0");                      \
    lib->priv.w.neededlibs[1] = strdup("libz.so.1");

#define CUSTOM_FINI \
    free(lib->priv.w.p2); \
    my_lib = NULL;

#include "wrappedlib_init.h"

