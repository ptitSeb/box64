#define _GNU_SOURCE /* See feature_test_macros(7) */
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
#include "box64context.h"
#include "librarian.h"
#include "myalign.h"

const char* libsqlite3Name = "libsqlite3.so.0";
#define LIBNAME libsqlite3


#define ADDED_FUNCTIONS()

#include "generated/wrappedlibsqlite3types.h"

//#include "wrappercallback.h"

// Insert my_* functions here...

#include "wrappedlib_init.h"
