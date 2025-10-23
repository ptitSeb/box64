#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper32.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "callback.h"
#include "librarian.h"
#include "box32context.h"
#include "emu/x64emu_private.h"
#include "myalign32.h"
#include "libtools/my_x11_conv.h"

static const char* libx11xcbName = "libX11-xcb.so.1";
#define ALTNAME "libX11-xcb.so"
#define LIBNAME libx11xcb

#define ADDED_FUNCTIONS()       \

#include "generated/wrappedlibx11xcbtypes32.h"

#include "wrappercallback32.h"

EXPORT void* my32_XGetXCBConnection(x64emu_t* emu, void* a)
{
    void* ret = add_xcb_connection32(my->XGetXCBConnection(a));
    regXCBDisplay(a, ret);
    return ret;
}

#include "wrappedlib_init32.h"
