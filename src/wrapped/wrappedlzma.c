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

const char* lzmaName = "liblzma.so.5";
#define LIBNAME lzma
static library_t* my_lib = NULL;


#define SUPER() \

typedef struct lzma_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} lzma_my_t;

void* getLzmaMy(library_t* lib)
{
    lzma_my_t* my = (lzma_my_t*)calloc(1, sizeof(lzma_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeLzmaMy(void* lib)
{
    //lzma_my_t *my = (lzma_my_t *)lib;
}

#define CUSTOM_INIT \
    lib->priv.w.p2 = getLzmaMy(lib); \
    my_lib = lib;

#define CUSTOM_FINI \
    freeLzmaMy(lib->priv.w.p2);  \
    free(lib->priv.w.p2);       \
    my_lib = NULL;

#include "wrappedlib_init.h"

