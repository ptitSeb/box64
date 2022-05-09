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

const char* alutName = "libalut.so.0";
#define LIBNAME alut

#define CUSTOM_INIT \
    setNeededLibs(lib, 1, "libopenal.so.1");

#include "wrappedlib_init.h"

