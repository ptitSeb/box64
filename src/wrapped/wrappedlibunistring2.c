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

const char* libunistring2Name = "libunistring.so.2";
#define ALTNAME "libunistring.so"
#define ALTNAME2 "libunistring.so.5"
#define LIBNAME libunistring2

#include "wrappedlib_init.h"
