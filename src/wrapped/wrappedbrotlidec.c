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

const char* brotlidecName = "libbrotlidec.so.1";
#define LIBNAME brotlidec

#define ADDED_FUNCTIONS()           \

//#include "generated/wrappedbrotlidectypes.h"

//#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

#undef SUPER

#include "wrappedlib_init.h"
