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
#include "box64context.h"
#include "librarian.h"
#include "myalign.h"


const char* libvorbisName = "libvorbis.so.0";
#define LIBNAME libvorbis


typedef struct vorbis_my_s {
} vorbis_my_t;

void* getVorbisMy(library_t* lib)
{
    vorbis_my_t* my = (vorbis_my_t*)calloc(1, sizeof(vorbis_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    (void)lib; // So many wrapped functions here too
    #undef GO
    return my;
}

void freeVorbisMy(void* lib)
{
    (void)lib;
    //vorbis_my_t *my = (vorbis_my_t *)lib;
}

#define CUSTOM_INIT \
    box64->vorbis = lib; \
    lib->priv.w.p2 = getVorbisMy(lib);

#define CUSTOM_FINI \
    freeVorbisMy(lib->priv.w.p2); \
    free(lib->priv.w.p2); \
    lib->context->vorbis = NULL;

#include "wrappedlib_init.h"
