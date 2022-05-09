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
    setNeededLibs(&lib->priv.w, 5,      \
        "libatk-1.0.so.0",              \
        "libSM.so.6",                   \
        "libICE.so.6",                  \
        "libXau.so.6",                  \
        "libxcb.so.1");

#include "wrappedlib_init.h"
