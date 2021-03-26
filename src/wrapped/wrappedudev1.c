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

const char* udev1Name = "libudev.so.1";
#define LIBNAME udev1
// fallback to 0 version... Not sure if really correct (probably not)
#define ALTNAME "libudev.so.0"

#include "wrappedlib_init.h"

