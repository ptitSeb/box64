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
#include "box64context.h"
#include "librarian.h"
#include "myalign.h"

#ifdef ANDROID
    const char* pulsesimpleName = "libpulse-simple.so";
#else
    const char* pulsesimpleName = "libpulse-simple.so.0";
#endif

#define LIBNAME pulsesimple

#define PRE_INIT        \
    if(box64_nopulse)   \
        return -1;

#ifdef ANDROID
    #define CUSTOM_INIT \
        setNeededLibs(lib, 1, "libpulse.so");
#else
    #define CUSTOM_INIT \
        setNeededLibs(lib, 1, "libpulse.so.0");
#endif

#include "wrappedlib_init.h"

