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

// Fake the lib for now, don't load it
const char* tbbmallocproxyName = "libtbbmalloc_proxy.so.2";
#define LIBNAME tbbmallocproxy

// this preinit basically open "box64" as dlopen
#define PRE_INIT\
    lib->w.lib = dlopen(NULL, RTLD_LAZY | RTLD_GLOBAL);    \
    if(0)

#include "wrappedlib_init.h"
