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

const char* libxftName = "libXft.so.2";
#define ALTNAME "libXft.so"

#define LIBNAME libxft

typedef void(*vFp_t)(void*);
typedef void*(*pFpp_t)(void*, void*);
typedef void*(*pFpipp_t)(void*, int, void*, void*);

#define ADDED_FUNCTIONS()                   \
    GO(XftFontMatch, pFpipp_t)              \
    GO(XftFontOpenPattern, pFpp_t)          \
    GO(FcPatternDestroy, vFp_t)

#include "generated/wrappedlibxfttypes.h"

#include "wrappercallback.h"

void* my_FcPatternBuild(x64emu_t* emu, void* pattern, uint64_t* b); // from fontconfig
EXPORT void* my_XftFontOpen(x64emu_t* emu, void* dpy, int screen, uintptr_t* b)
{
    int result;
    void* pat = my_FcPatternBuild(emu, NULL, b);
    void* match = my->XftFontMatch(dpy, screen, pat, &result);
    my->FcPatternDestroy(pat);
    if(!match)
        return NULL;
    void* ret = my->XftFontOpenPattern(dpy, match);
    if(!ret)
        my->FcPatternDestroy(match);
    return ret;
}

#define NEEDED_LIBS "libX11.so.6", "libfontconfig.so.1", "libXrender.so.1", "libfreetype.so.6"

#include "wrappedlib_init.h"
