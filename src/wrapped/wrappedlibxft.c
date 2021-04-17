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

const char* libxftName = "libXft.so.2";
#define LIBNAME libxft

#define CUSTOM_INIT \
    lib->priv.w.needed = 4; \
    lib->priv.w.neededlibs = (char**)calloc(lib->priv.w.needed, sizeof(char*)); \
    lib->priv.w.neededlibs[0] = strdup("libX11.so.6");          \
    lib->priv.w.neededlibs[1] = strdup("libfontconfig.so.1");   \
    lib->priv.w.neededlibs[2] = strdup("libXrender.so.1");      \
    lib->priv.w.neededlibs[3] = strdup("libfreetype.so.6");

#include "wrappedlib_init.h"

