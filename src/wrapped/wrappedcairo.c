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

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// destroy ...
#define GO(A)   \
static uintptr_t my_destroy_fct_##A = 0;        \
static void my_destroy_##A(void* a)             \
{                                               \
    RunFunctionFmt(my_destroy_fct_##A, "p", a); \
}
SUPER()
#undef GO
static void* find_destroy_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_destroy_fct_##A == (uintptr_t)fct) return my_destroy_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_destroy_fct_##A == 0) {my_destroy_fct_##A = (uintptr_t)fct; return my_destroy_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for cairo destroy callback\n");
    return NULL;
}
static void* reverse_destroy_Fct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_destroy_##A == fct) return (void*)my_destroy_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, pFpii, fct, 0, NULL);
}
// cairo_write ...
#define GO(A)   \
static uintptr_t my_cairo_write_fct_##A = 0;                                    \
static uint32_t my_cairo_write_##A(void* a, void* b, uint32_t c)                \
{                                                                               \
    return (uint32_t)RunFunctionFmt(my_cairo_write_fct_##A, "ppu", a, b, c);    \
}
SUPER()
#undef GO
static void* find_cairo_write_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_cairo_write_fct_##A == (uintptr_t)fct) return my_cairo_write_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_cairo_write_fct_##A == 0) {my_cairo_write_fct_##A = (uintptr_t)fct; return my_cairo_write_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for cairo cairo_write callback\n");
    return NULL;
}

#undef SUPER

EXPORT void* my_cairo_xcb_device_get_connection(x64emu_t* emu, void* a)
{
    return add_xcb_connection(my->cairo_xcb_device_get_connection(a));
}

EXPORT int my_cairo_surface_set_user_data(x64emu_t* emu, void* surf, void* key, void* data, void* d)
{
    return my->cairo_surface_set_user_data(surf, key, data, find_destroy_Fct(d));
}

EXPORT int my_cairo_set_user_data(x64emu_t* emu, void* cr, void* key, void* data, void* d)
{
    return my->cairo_set_user_data(cr, key, data, find_destroy_Fct(d));
}

EXPORT int my_cairo_surface_set_mime_data(x64emu_t* emu, void* surf, void* mime_type, void* data, size_t len, void* destroy, void* closure)
{
    return my->cairo_surface_set_mime_data(surf, mime_type, data, len, find_destroy_Fct(destroy), closure);
}

EXPORT void* my_cairo_pdf_surface_create_for_stream(x64emu_t* emu, void* f, void* c, double w, double h)
{
    return my->cairo_pdf_surface_create_for_stream(find_cairo_write_Fct(f), c, w, h);
}

EXPORT uint32_t my_cairo_surface_write_to_png_stream(x64emu_t* emu, void* surf, void* f, void* c)
{
    return my->cairo_surface_write_to_png_stream(surf, find_cairo_write_Fct(f), c);
}

#include "wrappedlib_init.h"