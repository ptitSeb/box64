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
#include "myalign32.h"
#include "converter32.h"
#include "libtools/my_x11_conv.h"

static const char* cairoName = "libcairo.so.2";
#define LIBNAME cairo

#include "generated/wrappedcairotypes32.h"

#include "wrappercallback32.h"

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
    return (void*)AddBridge(my_lib->w.bridge, vFp_32, fct, 0, NULL);
}
// cairo_write_func ...
#define GO(A)   \
static uintptr_t my_cairo_write_func_fct_##A = 0;                       \
static uint32_t my_cairo_write_func_##A(void* a, void* b, uint32_t c)   \
{                                                                       \
    return RunFunctionFmt(my_cairo_write_func_fct_##A, "ppu", a, b, c); \
}
SUPER()
#undef GO
static void* find_cairo_write_func_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_cairo_write_func_fct_##A == (uintptr_t)fct) return my_cairo_write_func_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_cairo_write_func_fct_##A == 0) {my_cairo_write_func_fct_##A = (uintptr_t)fct; return my_cairo_write_func_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for cairo cairo_write_func callback\n");
    return NULL;
}
#undef SUPER

EXPORT void* my32_cairo_xcb_device_get_connection(x64emu_t* emu, void* a)
{
    return add_xcb_connection32(my->cairo_xcb_device_get_connection(a));
}

EXPORT uint32_t my32_cairo_surface_set_user_data(x64emu_t* emu, void* surf, void* key, void* data, void* d)
{
    return my->cairo_surface_set_user_data(surf, key, data, find_destroy_Fct(d));
}

EXPORT uint32_t my32_cairo_set_user_data(x64emu_t* emu, void* cr, void* key, void* data, void* d)
{
    return my->cairo_set_user_data(cr, key, data, find_destroy_Fct(d));
}

void inplace_FT_FaceRec_shrink(void* a);
EXPORT void* my32_cairo_ft_scaled_font_lock_face(x64emu_t* emu, void* font)
{
    void* ret = my->cairo_ft_scaled_font_lock_face(font);
    inplace_FT_FaceRec_shrink(ret);
    return ret;
}

EXPORT void* my32_cairo_pdf_surface_create_for_stream(x64emu_t* emu, void* f, void* closure, double width, double height)
{
    return my->cairo_pdf_surface_create_for_stream(find_cairo_write_func_Fct(f), closure, width, height);
}

EXPORT uint32_t my32_cairo_pattern_set_user_data(x64emu_t* emu, void* pat, void* key, void* data, void* d)
{
    return my->cairo_pattern_set_user_data(pat, key, data, find_destroy_Fct(d));
}

EXPORT void* my32_cairo_xlib_surface_create(x64emu_t* emu, void* dpy, XID d, void* vis, int w, int h)
{
    return my->cairo_xlib_surface_create(dpy, d, convert_Visual_to_64(dpy, vis), w, h);
}

EXPORT void* my32_cairo_xlib_surface_create_for_bitmap(x64emu_t* emu, void* dpy, XID d, void* screen, int w, int h)
{

    return my->cairo_xlib_surface_create_for_bitmap(dpy, d, getScreen64(dpy, screen), w, h);
}

#include "wrappedlib_init32.h"