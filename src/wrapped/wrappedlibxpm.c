#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"

#ifdef ANDROID
const char* libxpmName = "libXpm.so";
#else
const char* libxpmName = "libXpm.so.4";
#endif

#define LIBNAME libxpm

#ifdef ANDROID
#define NEEDED_LIBS "libX11.so", "libXext.so"
#else
#define NEEDED_LIBS "libX11.so.6", "libXext.so.6"
#endif

#include "wrappedlib_init.h"
