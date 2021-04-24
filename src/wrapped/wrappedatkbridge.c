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

const char* atkbridgeName = "libatk-bridge-2.0.so.0";
#define LIBNAME atkbridge

#define PRE_INIT    \
    if(box64_nogtk) \
        return -1;

#define CUSTOM_INIT \
    lib->priv.w.needed = 5;                                                     \
    lib->priv.w.neededlibs = (char**)calloc(lib->priv.w.needed, sizeof(char*)); \
    lib->priv.w.neededlibs[0] = strdup("libatk-1.0.so.0");                      \
    lib->priv.w.neededlibs[1] = strdup("libSM.so.6");                           \
    lib->priv.w.neededlibs[2] = strdup("libICE.so.6");                          \
    lib->priv.w.neededlibs[3] = strdup("libXau.so.6");                          \
    lib->priv.w.neededlibs[4] = strdup("libxcb.so.1");                          \

#include "wrappedlib_init.h"
