#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "wrapper32.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"

#ifdef LA64
static const char* nslName = "libnsl.so.2";
#else
static const char* nslName = "libnsl.so.1";
#endif
#define LIBNAME nsl

#include "wrappedlib_init32.h"

