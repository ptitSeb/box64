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
    setNeededLibs(lib, 4,           \
        "libX11.so.6",              \
        "libfontconfig.so.1",       \
        "libXrender.so.1",          \
        "libfreetype.so.6");

#include "wrappedlib_init.h"

