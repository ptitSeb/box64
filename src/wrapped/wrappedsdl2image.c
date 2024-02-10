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
#include "sdl2rwops.h"

const char* sdl2imageName = "libSDL2_image-2.0.so.0";
#define LIBNAME sdl2image

#define ADDED_FUNCTIONS()           \

#include "generated/wrappedsdl2imagetypes.h"

#include "wrappercallback.h"

#define GO(A)   \
EXPORT void *my2_##A(x64emu_t* emu, void* a) \
{ \
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a); \
    void* r = my->A(rw); \
    RWNativeEnd2(rw); \
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

 EXPORT void *my2_IMG_LoadTyped_RW(x64emu_t* emu, void* a, int32_t b, void* c)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    void* r = my->IMG_LoadTyped_RW(rw, b, c);
    if(b==0)
        RWNativeEnd2(rw);
    return r;
}
EXPORT void *my2_IMG_Load_RW(x64emu_t* emu, void* a, int32_t b)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    void* r = my->IMG_Load_RW(rw, b);
    if(b==0)
        RWNativeEnd2(rw);
    return r;
}
EXPORT int32_t my2_IMG_SavePNG_RW(x64emu_t* emu, void* s, void* a, int32_t b)
{
    // some old? fuction signature use IMG_SavePNG_RW(dst, surf, compression) instead of the IMG_SavePNG_RW(surf, dst, freedst)
    // need to try detect if s is in fact a RWops
    int32_t r;
    if(isRWops((SDL2_RWops_t*)s) && !isRWops((SDL2_RWops_t*)a)) {
        SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)s);
        r = my->IMG_SavePNG_RW(a, rw, 0);
        RWNativeEnd2(rw);
    } else {
        SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
        r = my->IMG_SavePNG_RW(s, rw, b);
        if(b==0)
            RWNativeEnd2(rw);
    }
    return r;
}

EXPORT void* my2_IMG_LoadTexture_RW(x64emu_t* emu, void* rend, void* a, int32_t b)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    void* r = my->IMG_LoadTexture_RW(rend, rw, b);
    if(b==0)
        RWNativeEnd2(rw);
    return r;
}

EXPORT void* my2_IMG_LoadTextureTyped_RW(x64emu_t* emu, void* rend, void* a, int32_t b, void* type)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    void* r = my->IMG_LoadTextureTyped_RW(rend, rw, b, type);
    if(b==0)
        RWNativeEnd2(rw);
    return r;
}

#define ALTMY my2_

#define NEEDED_LIBS "libSDL2-2.0.so.0"

#include "wrappedlib_init.h"
