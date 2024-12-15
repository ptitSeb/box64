#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>
#include <netdb.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper32.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "callback.h"
#include "librarian.h"
#include "box64context.h"
#include "emu/x64emu_private.h"
#include "box32.h"
#include "myalign32.h"

static const char* libresolvName = "libresolv.so.2";
#define LIBNAME libresolv

#include "generated/wrappedlibresolvtypes32.h"

#include "wrappercallback32.h"

#include "wrappedlib_init32.h"
