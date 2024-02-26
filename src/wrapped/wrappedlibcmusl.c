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
#include "elfloader.h"
#include "box64context.h"
#include "x64tls.h"


// don't try to load the actual ld-linux (because name is variable), just use box64 itself, as it's linked to ld-linux
const char* libcmuslName = "libc.musl-x86_64.so.1";
#define LIBNAME libcmusl

#ifndef STATICBUILD
#define PRE_INIT\
    if(1)                                                           \
        lib->w.lib = dlopen(NULL, RTLD_LAZY | RTLD_GLOBAL);    \
    else
#endif

#define CUSTOM_INIT \
    box64_musl = 1;

#define NEEDED_LIBS "libc.so.6", "libdl.so.2", "libm.so.6", "libpthread.so.0"

// define all standard library functions
#include "wrappedlib_init.h"
