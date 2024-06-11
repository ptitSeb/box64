#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>
#include <stdarg.h>

#include "wrappedlibs.h"

#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "debug.h"
#include "myalign.h"
#include "callback.h"
#include "emu/x64emu_private.h"

const char* udev0Name = "libudev.so.0";
#define LIBNAME udev0

#define ADDED_FUNCTIONS()           \

//#include "generated/wrappedudev0types.h"

//#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

#undef SUPER

#define ALTMY my0_

#define NEEDED_LIBS "libudev.so.1"

#include "wrappedlib_init.h"
