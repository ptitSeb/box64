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
#include "emu/x64emu_private.h"
#include "callback.h"
#include "librarian.h"
#include "box64context.h"
#include "emu/x64emu_private.h"

#ifdef ANDROID
    const char* libxrandrName = "libXrandr.so";
#else
    const char* libxrandrName = "libXrandr.so.2";
#endif

#define LIBNAME libxrandr

#ifdef ANDROID
#define NEEDED_LIBS "libX11.so", "libXext.so", "libXrender.so"
#else
#define NEEDED_LIBS "libX11.so.6", "libXext.so.6", "libXrender.so.1"
#endif

#include "wrappedlib_init.h"
