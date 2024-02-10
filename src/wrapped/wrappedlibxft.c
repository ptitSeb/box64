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

#ifdef ANDROID
    const char* libxftName = "libXft.so";
#else
    const char* libxftName = "libXft.so.2";
#endif

#define LIBNAME libxft

#ifdef ANDROID
#define NEEDED_LIBS "libX11.so", "libfontconfig.so", "libXrender.so", "libfreetype.so"
#else
#define NEEDED_LIBS "libX11.so.6", "libfontconfig.so.1", "libXrender.so.1", "libfreetype.so.6"
#endif

#include "wrappedlib_init.h"
