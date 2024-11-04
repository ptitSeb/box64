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

const char* libxfixesName = "libXfixes.so.3";
#define ALTNAME "libXfixes.so"

#define LIBNAME libxfixes

#define NEEDED_LIBS "libX11.so.6", "libxcb.so.1", "libXau.so.6", "libXdmcp.so.6"

#include "wrappedlib_init.h"
