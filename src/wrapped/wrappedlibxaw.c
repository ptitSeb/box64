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

const char* libxawName = "libXaw.so.7";
#define ALTNAME "libXaw.so"

#define LIBNAME libxaw

#define NEEDED_LIBS "libX11.so.6", "libXext.so.6", "libXmu.so.6", "libXt.so.6"

#include "wrappedlib_init.h"
