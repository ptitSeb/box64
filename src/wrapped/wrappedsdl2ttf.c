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

const char* sdl2ttfName = "libSDL2_ttf-2.0.so.0";
#define LIBNAME sdl2ttf

#define ADDED_FUNCTIONS()           \

#include "generated/wrappedsdl2ttftypes.h"

#include "wrappercallback.h"

void EXPORT *my2_TTF_OpenFontIndexRW(x64emu_t* emu, void* a, int32_t b, int32_t c, int32_t d)
{
    SDL2_RWops_t* rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    void* r = my->TTF_OpenFontIndexRW(rw, b, c, d);
    if(b==0)
        RWNativeEnd2(rw);
    return r;
}

void EXPORT *my2_TTF_OpenFontRW(x64emu_t* emu, void* a, int32_t b, int32_t c)
{
    SDL2_RWops_t* rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    void* r = my->TTF_OpenFontRW(rw, b, c);
    if(b==0)
        RWNativeEnd2(rw);
    return r;
}

#define ALTMY my2_

#define NEEDED_LIBS "libSDL2-2.0.so.0"

#include "wrappedlib_init.h"
