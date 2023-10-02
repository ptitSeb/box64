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

#ifdef ANDROID
	const char* xshmfenceName = "libxshmfence.so";
#else
	const char* xshmfenceName = "libxshmfence.so.1";
#endif

#define LIBNAME xshmfence

#include "wrappedlib_init.h"

