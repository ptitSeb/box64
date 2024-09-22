#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "wrapper32.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"

#ifdef ANDROID
    static const char* libxrenderName = "libXrender.so";
#else
    static const char* libxrenderName = "libXrender.so.1";
#endif

#define LIBNAME libxrender

#ifdef ANDROID
#define NEEDED_LIBS "libX11.so"
#else
#define NEEDED_LIBS "libX11.so.6"
#endif

#include "wrappedlib_init32.h"
