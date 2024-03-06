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
#include "emu/x64emu_private.h"
#include "callback.h"
#include "librarian.h"
#include "box64context.h"
#include "emu/x64emu_private.h"
#include "myalign.h"

const char* cairoName = "libcairo.so.2";
#define LIBNAME cairo

#include "generated/wrappedcairotypes.h"

#include "wrappercallback.h"

#include "wrappedlib_init.h"


EXPORT void* my_cairo_xcb_device_get_connection(x64emu_t* emu, void* a)
{
    return add_xcb_connection(my->cairo_xcb_device_get_connection(a));
}