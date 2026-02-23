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
#include "box64context.h"

const char* sdl3ttfName = "libSDL3_ttf.so.0";
#define LIBNAME sdl3ttf

#define ADDED_FUNCTIONS()           \

#include "generated/wrappedsdl3ttftypes.h"

#include "wrappercallback.h"

#define ALTMY my3_

#define NEEDED_LIBS "libSDL3.so.0"

#include "wrappedlib_init.h"
