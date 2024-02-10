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
#include "sdl1rwops.h"

const char* sdl1soundName = "libSDL_sound-1.0.so.1";
#define LIBNAME sdl1sound

#include "generated/wrappedsdl1soundtypes.h"

#include "wrappercallback.h"

EXPORT void* my_Sound_NewSample(x64emu_t* emu, void* a, void* ext, void* desired, uint32_t buffersize)
{
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    void* r = my->Sound_NewSample(rw, ext, desired, buffersize);
    //RWNativeEnd(rw);  // will be closed automatically
    return r;
}

#define NEEDED_LIBS "libSDL-1.2.so.0"

#include "wrappedlib_init.h"
