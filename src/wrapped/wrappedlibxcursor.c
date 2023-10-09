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
	const char* libxcursorName = "libXcursor.so";
#else
	const char* libxcursorName = "libXcursor.so.1";
#endif

#define LIBNAME libxcursor

#include "wrappedlib_init.h"

