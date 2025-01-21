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
#include "box64context.h"
#include "librarian.h"
#include "myalign.h"

const char* pulsemainloopglibName = "libpulse-mainloop-glib.so.0";
#define ALTNAME "libpulse-mainloop-glib.so"

#define LIBNAME pulsemainloopglib

#include "generated/wrappedpulsemainloopglibtypes.h"

#include "wrappercallback.h"

void my_autobridge_mainloop_api(x64emu_t* emu, void* api);  // defined in pulse.c
EXPORT void* my_pa_glib_mainloop_get_api(x64emu_t* emu, void* mainloop)
{
    void* ret = my->pa_glib_mainloop_get_api(mainloop);
    my_autobridge_mainloop_api(emu, ret);
    return ret;
}

#define PRE_INIT        \
    if(BOX64ENV(nopulse))   \
        return -1;

#define NEEDED_LIBS "libpulse.so.0"

#include "wrappedlib_init.h"
