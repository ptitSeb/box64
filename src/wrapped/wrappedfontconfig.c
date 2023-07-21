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

const char* fontconfigName = "libfontconfig.so.1";
#define LIBNAME fontconfig

#define ADDED_FUNCTIONS()                   \

#include "generated/wrappedfontconfigtypes.h"

#include "wrappercallback.h"

EXPORT void* my_FcObjectSetVaBuild(x64emu_t* emu, void* first, x64_va_list_t V)
{
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(V);
    #else
    CREATE_VALIST_FROM_VALIST(V, emu->scratch);
    #endif
    return my->FcObjectSetVaBuild(first, VARARGS);
}
EXPORT void* my_FcObjectSetBuild(x64emu_t* emu, void* first, uint64_t* b)
{
    if(!first)    
        return my->FcObjectSetBuild(first, NULL);
    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 1);
    return my->FcObjectSetVaBuild(first, VARARGS);
}

#define CUSTOM_INIT \
    getMy(lib);

#define CUSTOM_FINI \
    freeMy();

#include "wrappedlib_init.h"
