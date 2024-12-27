#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper32.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "callback.h"
#include "librarian.h"
#include "box32context.h"
#include "emu/x64emu_private.h"
#include "converter32.h"

#ifdef ANDROID
    static const char* libxfixesName = "libXfixes.so";
#else
    static const char* libxfixesName = "libXfixes.so.3";
#endif

#define LIBNAME libxfixes

#include "libtools/my_x11_conv.h"

#include "generated/wrappedlibxfixestypes32.h"

#include "wrappercallback32.h"

EXPORT int my32_XFixesQueryExtension(x64emu_t* emu, void* dpy, int* event_base, int* error_base)
{
    int ret = my->XFixesQueryExtension(dpy, event_base, error_base);
    if(!ret) return ret;

    register_XFixes_events(*event_base);
    return ret;
}

EXPORT void* my32_XFixesGetCursorImage(x64emu_t* emu, void* dpy)
{
    void* ret = my->XFixesGetCursorImage(dpy);
    if(ret) {
        my_XFixesCursorImage_t* src = ret;
        my_XFixesCursorImage_32_t* dst = ret;
        for(size_t i=0; i<src->width*src->height; ++i)
            ((ulong_t*)src->pixels)[i] = to_ulong_silent(src->pixels[i]);
        dst->x = src->x;
        dst->y = src->y;
        dst->width = src->width;
        dst->height = src->height;
        dst->xhot = src->xhot;
        dst->cursor_serial = src->cursor_serial;
        dst->pixels = to_ptrv(src->pixels);
        // should check if v2 here!
        dst->atom = to_ulong(src->atom);
        dst->name = to_ptrv(src->name);
    }
    return ret;
}

#if 0
#ifdef ANDROID
#define NEEDED_LIBS "libX11.so", "libxcb.so", "libXau.so", "libXdmcp.so"
#else
#define NEEDED_LIBS "libX11.so.6", "libxcb.so.1", "libXau.so.6", "libXdmcp.so.6"
#endif
#endif

#define CUSTOM_FINI \
    unregister_XFixes_events();

#include "wrappedlib_init32.h"
