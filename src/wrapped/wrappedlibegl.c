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
#include "box64context.h"
#include "librarian.h"
#include "callback.h"
#include "gltools.h"

const char* libeglName = "libEGL.so.1";
#define LIBNAME libegl

#include "generated/wrappedlibegltypes.h"

#include "wrappercallback.h"

EXPORT void* my_eglGetProcAddress(x64emu_t* emu, void* name) 
{
    khint_t k;
    const char* rname = (const char*)name;
    return getGLProcAddress(emu, NULL, (glprocaddress_t)my->eglGetProcAddress, rname);
}

#include "wrappedlib_init.h"
