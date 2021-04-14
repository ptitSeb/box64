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


typedef struct sdl2net_my_s {
} sdl2net_my_t;

static void* getSDL2NetMy(library_t* lib)
{
    sdl2net_my_t* my = (sdl2net_my_t*)calloc(1, sizeof(sdl2net_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    (void)lib; // So many wrapped functions here
    #undef GO
    return my;
}

static library_t* my_lib = NULL;


const char* sdl2netName = "libSDL2_net-2.0.so.0";
#define LIBNAME sdl2net

#define CUSTOM_INIT                     \
    my_lib = lib;                       \
    lib->altmy = strdup("my2_");        \
    lib->priv.w.p2 = getSDL2NetMy(lib);

#define CUSTOM_FINI                     \
    free(lib->priv.w.p2);               \
    my_lib = NULL;

#include "wrappedlib_init.h"
