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

const char* nvmlName = "libnvidia-ml.so.1";
#define ALTNAME "libnvidia-ml.so"

#define LIBNAME nvml

#define PRE_INIT \
    if (BOX64ENV(nogtk)) return -2;

#include "wrappedlib_init.h"
