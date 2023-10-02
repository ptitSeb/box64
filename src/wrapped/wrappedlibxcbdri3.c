#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "callback.h"
#include "librarian.h"
#include "box64context.h"
#include "emu/x64emu_private.h"

#ifdef ANDROID
	const char* libxcbdri3Name = "libxcb-dri3.so";
#else
	const char* libxcbdri3Name = "libxcb-dri3.so.0";
#endif

#define LIBNAME libxcbdri3

#include "wrappedlib_init.h"
