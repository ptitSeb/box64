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

const char* libxrenderName = "libXrender.so.1";
#define LIBNAME libxrender

#define CUSTOM_INIT \
    setNeededLibs(lib, 4,           \
        "libX11.so.6",              \
        "libxcb.so.1",              \
        "libXau.so.6",              \
        "libXdmcp.so.6");

#include "wrappedlib_init.h"

