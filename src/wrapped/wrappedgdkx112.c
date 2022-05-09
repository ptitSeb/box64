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
#include "gtkclass.h"

const char* gdkx112Name = "libgdk-x11-2.0.so.0";
#define LIBNAME gdkx112

static char* libname = NULL;

char* getGDKX11LibName() {return libname;}

#include "generated/wrappedgdkx112types.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)

// GdkFilterFunc
#define GO(A)   \
static uintptr_t my_filter_fct_##A = 0;   \
static int my_filter_##A(void* xevent, void* event, void* data)     \
{                                       \
    return (int)RunFunction(my_context, my_filter_fct_##A, 3, xevent, event, data);\
}
SUPER()
#undef GO
static void* findFilterFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_filter_fct_##A == (uintptr_t)fct) return my_filter_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_filter_fct_##A == 0) {my_filter_fct_##A = (uintptr_t)fct; return my_filter_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-2 GdkFilterFunc callback\n");
    return NULL;
}


static void my_event_handler(void* event, my_signal_t* sig)
{
    RunFunction(my_context, sig->c_handler, 2, event, sig->data);
}

EXPORT void my_gdk_event_handler_set(x64emu_t* emu, void* func, void* data, void* notify)
{
    if(!func)
        return my->gdk_event_handler_set(func, data, notify);

    my_signal_t* sig = new_mysignal(func, data, notify);
    my->gdk_event_handler_set(my_event_handler, sig, my_signal_delete);
}


static void my_input_function(my_signal_t* sig, int source, int condition)
{
    RunFunction(my_context, sig->c_handler, 3, sig->data, source, condition);
}

EXPORT int my_gdk_input_add(x64emu_t* emu, int source, int condition, void* f, void* data)
{
    if(!f)
        return my->gdk_input_add_full(source, condition, f, data, NULL);

    my_signal_t* sig = new_mysignal(f, data, NULL);
    return my->gdk_input_add_full(source, condition, my_input_function, sig, my_signal_delete);
}

EXPORT int my_gdk_input_add_full(x64emu_t* emu, int source, int condition, void* f, void* data, void* notify)
{
    if(!f)
        return my->gdk_input_add_full(source, condition, f, data, notify);
    
    my_signal_t* sig = new_mysignal(f, data, notify);
    return my->gdk_input_add_full(source, condition, my_input_function, sig, my_signal_delete);
}

EXPORT void my_gdk_init(x64emu_t* emu, void* argc, void* argv)
{
    my->gdk_init(argc, argv);
    my_checkGlobalGdkDisplay();
}

EXPORT int my_gdk_init_check(x64emu_t* emu, void* argc, void* argv)
{
    int ret = my->gdk_init_check(argc, argv);
    my_checkGlobalGdkDisplay();
    return ret;
}

EXPORT void my_gdk_window_add_filter(x64emu_t* emu, void* window, void* f, void* data)
{
    my->gdk_window_add_filter(window, findFilterFct(f), data);
}

EXPORT void my_gdk_window_remove_filter(x64emu_t* emu, void* window, void* f, void* data)
{
    my->gdk_window_remove_filter(window, findFilterFct(f), data);
}

#define PRE_INIT    \
    if(box64_nogtk) \
        return -1;

#define CUSTOM_INIT         \
    libname = lib->name;    \
    getMy(lib);             \
    setNeededLibs(&lib->priv.w, 3, "libgobject-2.0.so.0", "libgio-2.0.so.0", "libgdk_pixbuf-2.0.so.0");

#define CUSTOM_FINI \
    freeMy(); \


#include "wrappedlib_init.h"
