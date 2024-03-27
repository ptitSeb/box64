#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"

#ifdef ANDROID
const char* gstfftName = "libgstfft-1.0.so";
#else
const char* gstfftName = "libgstfft-1.0.so.0";
#endif

#define LIBNAME gstfft

#define PRE_INIT     \
    if (box64_nogtk) \
        return -1;

#include "wrappedlib_init.h"
