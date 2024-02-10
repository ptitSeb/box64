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
#include "sdl1rwops.h"


const char* sdl1netName = "libSDL_net-1.2.so.0";
#define LIBNAME sdl1net

#define ALTMY my_

#include "wrappedlib_init.h"
