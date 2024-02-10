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

const char* alureName = "libalure.so.1";
#define LIBNAME alure

#define NEEDED_LIBS "libopenal.so.1"

#include "wrappedlib_init.h"
