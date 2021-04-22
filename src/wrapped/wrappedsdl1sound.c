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

const char* sdl1soundName = "libSDL_sound-1.0.so.1";
#define LIBNAME sdl1sound

#include "generated/wrappedsdl1soundtypes.h"

typedef struct sdl1sound_my_s {
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} sdl1sound_my_t;

static library_t* my_lib = NULL;

static void* getSDL1SoundMy(library_t* lib)
{
    sdl1sound_my_t* my = (sdl1sound_my_t*)calloc(1, sizeof(sdl1sound_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}

EXPORT void* my_Sound_NewSample(x64emu_t* emu, void* a, void* ext, void* desired, uint32_t buffersize)
{
    sdl1sound_my_t *my = (sdl1sound_my_t *)my_lib->priv.w.p2;
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    void* r = my->Sound_NewSample(rw, ext, desired, buffersize);
    //RWNativeEnd(rw);  // will be closed automatically
    return r;
}

#define CUSTOM_INIT \
    my_lib = lib; \
    lib->priv.w.p2 = getSDL1SoundMy(lib);   \
    lib->priv.w.needed = 1; \
    lib->priv.w.neededlibs = (char**)calloc(lib->priv.w.needed, sizeof(char*)); \
    lib->priv.w.neededlibs[0] = strdup("libSDL-1.2.so.0");

#define CUSTOM_FINI \
    free(lib->priv.w.p2); \
    my_lib = NULL;

#include "wrappedlib_init.h"

