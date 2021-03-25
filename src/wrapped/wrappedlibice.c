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

const char* libiceName = "libICE.so.6";
#define LIBNAME libice

#if 0
#define SUPER()
    GO(BZ2_bzCompressInit, iFpiii_t)

typedef struct libice_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} libice_my_t;

void* getICEMy(library_t* lib)
{
    libice_my_t* my = (libice_my_t*)calloc(1, sizeof(libice_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeICEMy(void* lib)
{
    libice_my_t *my = (libice_my_t *)lib;
}

#define CUSTOM_INIT \
    lib->priv.w.p2 = getICEMy(lib);

#define CUSTOM_FINI \
    freeICEMy(lib->priv.w.p2); \
    free(lib->priv.w.p2);
#endif

#include "wrappedlib_init.h"

