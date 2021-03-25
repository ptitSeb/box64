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

const char* fontconfigName = "libfontconfig.so.1";
#define LIBNAME fontconfig

#define SUPER() \

typedef struct fontconfig_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} fontconfig_my_t;

void* getFontconfigMy(library_t* lib)
{
    fontconfig_my_t* my = (fontconfig_my_t*)calloc(1, sizeof(fontconfig_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeFontconfigMy(void* lib)
{
    //fontconfig_my_t *my = (fontconfig_my_t *)lib;
}

#define CUSTOM_INIT \
    lib->priv.w.p2 = getFontconfigMy(lib);

#define CUSTOM_FINI \
    freeFontconfigMy(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"

