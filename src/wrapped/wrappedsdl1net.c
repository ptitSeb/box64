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


typedef struct sdl1net_my_s {
} sdl1net_my_t;

static void* getSDL1NetMy(library_t* lib)
{
    sdl1net_my_t* my = (sdl1net_my_t*)calloc(1, sizeof(sdl1net_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    #undef GO
    return my;
}

static library_t* my_lib = NULL;


const char* sdl1netName = "libSDL_net-1.2.so.0";
#define LIBNAME sdl1net

#define CUSTOM_INIT                     \
    my_lib = lib;                       \
    lib->altmy = strdup("my_");        \
    lib->priv.w.p2 = getSDL1NetMy(lib);

#define CUSTOM_FINI                     \
    free(lib->priv.w.p2);               \
    my_lib = NULL;

#include "wrappedlib_init.h"

