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
#include "box32.h"

#ifdef ANDROID
    static const char* xineramaName = "libXinerama.so";
#else
    static const char* xineramaName = "libXinerama.so.1";
#endif

#define LIBNAME xinerama

#ifdef ANDROID
#define NEEDED_LIBS "libX11.so", "libXext.so"
#else
#define NEEDED_LIBS "libX11.so.6", "libXext.so.6"
#endif

#include "wrappedlib_init32.h"
