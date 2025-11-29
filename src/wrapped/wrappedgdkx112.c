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
#define ALTNAME "libgdk-x11-2.0.so"

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
static uintptr_t my_filter_fct_##A = 0;                                                    \
static int my_filter_##A(void* xevent, void* event, void* data)                            \
{                                                                                          \
    return (int)RunFunctionFmt(my_filter_fct_##A, "ppp", xevent, event, data); \
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
// GSourceFunc
#define GO(A)   \
static uintptr_t my_GSourceFunc_fct_##A = 0;                                \
static int my_GSourceFunc_##A(void* a)                                      \
{                                                                           \
    return (int)RunFunctionFmt(my_GSourceFunc_fct_##A, "p", a);       \
}
SUPER()
#undef GO
static void* findGSourceFunc(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GSourceFunc_fct_##A == (uintptr_t)fct) return my_GSourceFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GSourceFunc_fct_##A == 0) {my_GSourceFunc_fct_##A = (uintptr_t)fct; return my_GSourceFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gdk2 GSourceFunc callback\n");
    return NULL;
}
// GDestroyNotify
#define GO(A)   \
static uintptr_t my_GDestroyNotify_fct_##A = 0;                         \
static void my_GDestroyNotify_##A(void* data)                           \
{                                                                       \
    RunFunctionFmt(my_GDestroyNotify_fct_##A, "p", data);         \
}
SUPER()
#undef GO
static void* findGDestroyNotifyFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GDestroyNotify_fct_##A == (uintptr_t)fct) return my_GDestroyNotify_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GDestroyNotify_fct_##A == 0) {my_GDestroyNotify_fct_##A = (uintptr_t)fct; return my_GDestroyNotify_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gdk2 GDestroyNotify callback\n");
    return NULL;
}
// EventHandler
#define GO(A)   \
static uintptr_t my_EventHandler_fct_##A = 0;               \
static void my_EventHandler_##A(void* a, void* b)           \
{                                                           \
    RunFunctionFmt(my_EventHandler_fct_##A, "pp", a, b);    \
}
SUPER()
#undef GO
static void* findEventHandlerFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_EventHandler_fct_##A == (uintptr_t)fct) return my_EventHandler_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_EventHandler_fct_##A == 0) {my_EventHandler_fct_##A = (uintptr_t)fct; return my_EventHandler_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gdk2 EventHandler callback\n");
    return NULL;
}
// InputEvent
#define GO(A)   \
static uintptr_t my_InputEvent_fct_##A = 0;                 \
static void my_InputEvent_##A(void* a, int b, int c)        \
{                                                           \
    RunFunctionFmt(my_InputEvent_fct_##A, "pii", a, b, c);  \
}
SUPER()
#undef GO
static void* findInputEventFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_InputEvent_fct_##A == (uintptr_t)fct) return my_InputEvent_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_InputEvent_fct_##A == 0) {my_InputEvent_fct_##A = (uintptr_t)fct; return my_InputEvent_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gdk2 InputEvent callback\n");
    return NULL;
}

#undef SUPER

EXPORT void my_gdk_event_handler_set(x64emu_t* emu, void* func, void* data, void* notify)
{
    return my->gdk_event_handler_set(findEventHandlerFct(func), data, findGDestroyNotifyFct(notify));
}


EXPORT int my_gdk_input_add(x64emu_t* emu, int source, int condition, void* f, void* data)
{
    return my->gdk_input_add(source, condition, findInputEventFct(f), data);
}

EXPORT int my_gdk_input_add_full(x64emu_t* emu, int source, int condition, void* f, void* data, void* notify)
{
    return my->gdk_input_add_full(source, condition, findInputEventFct(f), data, findGDestroyNotifyFct(notify));
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

EXPORT uint32_t my_gdk_threads_add_timeout_full(x64emu_t* emu, int priotity, uint32_t interval, void* f, void* data, void* d)
{
    return my->gdk_threads_add_timeout_full(priotity, interval, findGSourceFunc(f), data, findGDestroyNotifyFct(d));
}

#define PRE_INIT \
    if (BOX64ENV(nogtk)) return -2;

#define CUSTOM_INIT         \
    libname = lib->name;

#define NEEDED_LIBS "libgobject-2.0.so.0", "libgio-2.0.so.0", "libgdk_pixbuf-2.0.so.0"

#include "wrappedlib_init.h"
