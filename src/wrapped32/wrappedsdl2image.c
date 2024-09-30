#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper32.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "box32context.h"
#include "sdl2rwops.h"
#include "sdl2align32.h"

static const char* sdl2imageName = "libSDL2_image-2.0.so.0";
#define LIBNAME sdl2image

#define ADDED_FUNCTIONS()           \

#include "generated/wrappedsdl2imagetypes32.h"

#include "wrappercallback32.h"

#define GO(A)   \
EXPORT void *my32_2_##A(x64emu_t* emu, void* a) \
{ \
    inplace_SDL2_RWops_to_64(a); \
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a); \
    void* r = my->A(rw); \
    RWNativeEnd2(rw); \
    inplace_SDL2_RWops_to_32(a); \
    inplace_SDL2_Surface_to_32(r); \
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

EXPORT void *my32_2_IMG_Load(x64emu_t* emu, void* path)
{
    void* r = my->IMG_Load(path);
    inplace_SDL2_Surface_to_32(r);
    return r;
}

EXPORT void *my32_2_IMG_LoadTyped_RW(x64emu_t* emu, void* a, int32_t b, void* c)
{
    inplace_SDL2_RWops_to_64(a);
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    void* r = my->IMG_LoadTyped_RW(rw, b, c);
    if(b==0) {
        RWNativeEnd2(rw);
        inplace_SDL2_RWops_to_32(a);
    }
    inplace_SDL2_Surface_to_32(r);
    return r;
}
EXPORT void *my32_2_IMG_Load_RW(x64emu_t* emu, void* a, int32_t b)
{
    inplace_SDL2_RWops_to_64(a);
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    void* r = my->IMG_Load_RW(rw, b);
    if(b==0) {
        RWNativeEnd2(rw);
        inplace_SDL2_RWops_to_32(a);
    }
    inplace_SDL2_Surface_to_32(r);
    return r;
}
//EXPORT int32_t my32_2_IMG_SavePNG_RW(x64emu_t* emu, void* s, void* a, int32_t b)
//{
//    // some old? fuction signature use IMG_SavePNG_RW(dst, surf, compression) instead of the IMG_SavePNG_RW(surf, dst, freedst)
//    // need to try detect if s is in fact a RWops
//    int32_t r;
//    if(isRWops32((my_SDL2_RWops_32_t*)s) && !isRWops32((my_SDL2_RWops_32_t*)a)) {
//        inplace_SDL2_RWops_to_64(s);
//        SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)s);
//        r = my->IMG_SavePNG_RW(a, rw, 0);
//        RWNativeEnd2(rw);
//    } else {
//        SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
//        r = my->IMG_SavePNG_RW(s, rw, b);
//        if(b==0)
//            RWNativeEnd2(rw);
//    }
//    return r;
//}

EXPORT void* my32_2_IMG_LoadTexture_RW(x64emu_t* emu, void* rend, void* a, int32_t b)
{
    inplace_SDL2_RWops_to_64(a);
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    void* r = my->IMG_LoadTexture_RW(rend, rw, b);
    if(b==0) {
        RWNativeEnd2(rw);
        inplace_SDL2_RWops_to_32(a);
    }
    return r;
}

EXPORT void* my32_2_IMG_LoadTextureTyped_RW(x64emu_t* emu, void* rend, void* a, int32_t b, void* type)
{
    inplace_SDL2_RWops_to_64(a);
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    void* r = my->IMG_LoadTextureTyped_RW(rend, rw, b, type);
    if(b==0) {
        RWNativeEnd2(rw);
        inplace_SDL2_RWops_to_32(a);
    }
    return r;
}

#define ALTMY my32_2_

#define NEEDED_LIBS "libSDL2-2.0.so.0"

#include "wrappedlib_init32.h"
