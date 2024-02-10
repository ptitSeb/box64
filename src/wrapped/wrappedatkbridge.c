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

const char* atkbridgeName = "libatk-bridge-2.0.so.0";
#define LIBNAME atkbridge

EXPORT int my_atk_bridge_adaptor_init(void* argc, void** argv)
{
    return 0;
}

#define PRE_INIT    \
    if(box64_nogtk) \
        return -1;

#define NEEDED_LIBS "libatk-1.0.so.0", "libSM.so.6", "libICE.so.6", "libXau.so.6", "libxcb.so.1"

#include "wrappedlib_init.h"
