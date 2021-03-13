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

typedef void* (*pFpii_t)(void*, int32_t, int32_t);
typedef void* (*pFpiii_t)(void*, int32_t, int32_t, int32_t);

typedef struct sdl2ttf_my_s {
    pFpii_t     TTF_OpenFontRW;
    pFpiii_t    TTF_OpenFontIndexRW;
} sdl2ttf_my_t;

static void* getSDL2TTFMy(library_t* lib)
{
    sdl2ttf_my_t* my = (sdl2ttf_my_t*)calloc(1, sizeof(sdl2ttf_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    GO(TTF_OpenFontIndexRW,pFpiii_t)
    GO(TTF_OpenFontRW, pFpii_t)
    #undef GO
    return my;
}

static library_t* my_lib = NULL;

void EXPORT *my2_TTF_OpenFontIndexRW(x64emu_t* emu, void* a, int32_t b, int32_t c, int32_t d)
{
    sdl2ttf_my_t *my = (sdl2ttf_my_t *)my_lib->priv.w.p2;
    SDL2_RWops_t* rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    void* r = my->TTF_OpenFontIndexRW(rw, b, c, d);
    if(b==0)
        RWNativeEnd2(rw);
    return r;
}

void EXPORT *my2_TTF_OpenFontRW(x64emu_t* emu, void* a, int32_t b, int32_t c)
{
    sdl2ttf_my_t *my = (sdl2ttf_my_t *)my_lib->priv.w.p2;
    SDL2_RWops_t* rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    void* r = my->TTF_OpenFontRW(rw, b, c);
    if(b==0)
        RWNativeEnd2(rw);
    return r;
}

const char* sdl2ttfName = "libSDL2_ttf-2.0.so.0";
#define LIBNAME sdl2ttf

#define CUSTOM_INIT                     \
    my_lib = lib;                       \
    lib->altmy = strdup("my2_");        \
    lib->priv.w.p2 = getSDL2TTFMy(lib);

#define CUSTOM_FINI                     \
    free(lib->priv.w.p2);               \
    my_lib = NULL;

#include "wrappedlib_init.h"

