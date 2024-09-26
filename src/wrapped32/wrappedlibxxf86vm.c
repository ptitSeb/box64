#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper32.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "callback.h"
#include "librarian.h"
#include "box32context.h"
#include "emu/x64emu_private.h"

#ifdef ANDROID
    static const char* libxxf86vmName = "libXxf86vm.so";
#else
    static const char* libxxf86vmName = "libXxf86vm.so.1";
#endif

#define LIBNAME libxxf86vm
#if 0
#ifdef ANDROID
#define NEEDED_LIBS "libX11.so", "libXext.so"
#else
#define NEEDED_LIBS "libX11.so.6", "libXext.so.6"
#endif
#endif
#include "wrappedlib_init32.h"
