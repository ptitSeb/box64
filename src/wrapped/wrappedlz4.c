#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"

const char* lz4Name = "liblz4.so.1";
#define LIBNAME lz4
#define ALTNAME "liblz4.so"

#include "wrappedlib_init.h"
