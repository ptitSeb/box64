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

#include "wrappedlib_init.h"