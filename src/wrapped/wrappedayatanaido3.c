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

const char* ayatanaido3Name = "libayatana-ido3-0.4.so.0";

#define LIBNAME ayatanaido3

#define PRE_INIT \
    if (BOX64ENV(nogtk)) return -2;

#define NEEDED_LIBS "libgtk-3.so.0", "libgobject-2.0.so.0", "libglib-2.0.so.0"

#include "wrappedlib_init.h"
