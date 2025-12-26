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
#include "gltools.h"

const char* libglxName = "libGLX.so.0";
#define LIBNAME libglx

#include "generated/wrappedlibglxtypes.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \


#undef SUPER

EXPORT void* myx_glXGetProcAddress(x64emu_t* emu, void* name) 
{
    khint_t k;
    const char* rname = (const char*)name;
    return getGLProcAddress(emu, NULL, (glprocaddress_t)my->glXGetProcAddress, rname);
}

EXPORT void* myx_glXGetProcAddressARB(x64emu_t* emu, void* name) 
{
    khint_t k;
    const char* rname = (const char*)name;
    return getGLProcAddress(emu, NULL, (glprocaddress_t)my->glXGetProcAddressARB, rname);
}

#define ALTMY myx_

#include "wrappedlib_init.h"
