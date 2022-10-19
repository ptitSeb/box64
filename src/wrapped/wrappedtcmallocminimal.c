#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"

// Fake the lib for now, don't load it
const char* tcmallocminimalName = "libtcmalloc_minimal.so.4";
#define LIBNAME tcmallocminimal

// this preinit basically open "box86" as dlopen (because libtcmalloc_minimal needs to be LD_PRELOAD for it to work)
#define PRE_INIT\
    lib->w.lib = dlopen(NULL, RTLD_LAZY | RTLD_GLOBAL);    \
    if(0)

#include "wrappedlib_init.h"
