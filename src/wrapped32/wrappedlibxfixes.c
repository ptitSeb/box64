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
    static const char* libxfixesName = "libXfixes.so";
#else
    static const char* libxfixesName = "libXfixes.so.3";
#endif

#define LIBNAME libxfixes

#if 0
#ifdef ANDROID
#define NEEDED_LIBS "libX11.so", "libxcb.so", "libXau.so", "libXdmcp.so"
#else
#define NEEDED_LIBS "libX11.so.6", "libxcb.so.1", "libXau.so.6", "libXdmcp.so.6"
#endif
#endif

#include "wrappedlib_init32.h"
