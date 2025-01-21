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

const char* pulsesimpleName = "libpulse-simple.so.0";
#define ALTNAME "libpulse-simple.so"

#define LIBNAME pulsesimple

#define PRE_INIT          \
    if(BOX64ENV(nopulse)) \
        return -1;

#define NEEDED_LIBS "libpulse.so.0"

#include "wrappedlib_init.h"
