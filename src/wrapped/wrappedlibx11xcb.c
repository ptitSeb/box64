#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "callback.h"
#include "librarian.h"
#include "box64context.h"
#include "emu/x64emu_private.h"
#include "myalign.h"

const char* libx11xcbName = "libX11-xcb.so.1";
#define ALTNAME "libX11-xcb.so"
#define LIBNAME libx11xcb

#define ADDED_FUNCTIONS()       \

#include "generated/wrappedlibx11xcbtypes.h"

#include "wrappercallback.h"

EXPORT void* my_XGetXCBConnection(x64emu_t* emu, void* a)
{
    void* ret = add_xcb_connection(my->XGetXCBConnection(a));
    register_xcb_display(a, ret);
    return ret;
}

#include "wrappedlib_init.h"
