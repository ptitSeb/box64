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

const char* pangoft2Name = "libpangoft2-1.0.so.0";
#define LIBNAME pangoft2

#define CUSTOM_INIT \
    setNeededLibs(&lib->priv.w, 2, "libfontconfig.so.1", "libfreetype.so.6");

#include "wrappedlib_init.h"
