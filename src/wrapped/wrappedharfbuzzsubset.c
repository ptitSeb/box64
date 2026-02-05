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
#include "callback.h"
#include "debug.h"

const char* harfbuzzsubsetName = "libharfbuzz-subset.so.0";
#define LIBNAME harfbuzzsubset

#include "wrappedlib_init.h"

