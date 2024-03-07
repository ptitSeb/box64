#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

#ifdef ANDROID
    const char* fontconfigName = "libfontconfig.so";
#else
    const char* fontconfigName = "libfontconfig.so.1";
#endif

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

EXPORT void* my_FcPatternVaBuild(x64emu_t* emu, void* pattern, x64_va_list_t V)
{
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(V);
    #else
    CREATE_VALIST_FROM_VALIST(V, emu->scratch);
    #endif
    return my->FcPatternVaBuild(pattern, VARARGS);
}
EXPORT void* my_FcPatternBuild(x64emu_t* emu, void* pattern, uint64_t* b)
{
    if(!pattern)    
        return my->FcPatternBuild(pattern, NULL);
    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 1);
    return my->FcPatternVaBuild(pattern, VARARGS);
}

#define NEEDED_LIBS "libexpat.so.1", "libfreetype.so.6"

#include "wrappedlib_init.h"
