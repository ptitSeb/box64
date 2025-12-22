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
#include "gtkclass.h"

const char* gdk3Name = "libgdk-3.so.0";
#define ALTNAME "libgdk-3.so"

#define LIBNAME gdk3

//#define ADDED_FUNCTIONS()

#include "generated/wrappedgdk3types.h"

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
    return (int)RunFunctionFmt(my_filter_fct_##A, "ppp", xevent, event, data);        \
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
    printf_log(LOG_NONE, "Warning, no more slot for gdk-3 GdkFilterFunc callback\n");
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
    printf_log(LOG_NONE, "Warning, no more slot for gdk-3 GSourceFunc callback\n");
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
    printf_log(LOG_NONE, "Warning, no more slot for gdk-3 GDestroyNotify callback\n");
    return NULL;
}
// GCallback  (generic function with 6 arguments, hopefully it's enough)
#define GO(A)   \
static uintptr_t my_GCallback_fct_##A = 0;                                                      \
static void* my_GCallback_##A(void* a, void* b, void* c, void* d)             \
{                                                                                               \
    return (void*)RunFunctionFmt(my_GCallback_fct_##A, "pppp", a, b, c, d); \
}
SUPER()
#undef GO
static void* findGCallbackFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GCallback_fct_##A == (uintptr_t)fct) return my_GCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GCallback_fct_##A == 0) {my_GCallback_fct_##A = (uintptr_t)fct; return my_GCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gdk3 generic GCallback\n");
    return NULL;
}
// EventHandler
#define GO(A)   \
static uintptr_t my_EventHandler_fct_##A = 0;                                       \
static void* my_EventHandler_##A(void* a, void* b, void* c, void* d, void* e)       \
{                                                                                   \
    return (void*)RunFunctionFmt(my_EventHandler_fct_##A, "ppppp", a, b, c, d, e);  \
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
    printf_log(LOG_NONE, "Warning, no more slot for gdk3 generic EventHandler\n");
    return NULL;
}

// GdkSeatGrabPrepareFunc
#define GO(A)                                                                            \
    static uintptr_t my_GdkSeatGrabPrepareFunc_fct_##A = 0;                              \
    static void* my_GdkSeatGrabPrepareFunc_##A(void* a, void* b, void* c)                \
    {                                                                                    \
        return (void*)RunFunctionFmt(my_GdkSeatGrabPrepareFunc_fct_##A, "ppp", a, b, c); \
    }
SUPER()
#undef GO
static void* findGdkSeatGrabPrepareFuncFct(void* fct)
{
    if (!fct) return fct;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
#define GO(A) \
    if (my_GdkSeatGrabPrepareFunc_fct_##A == (uintptr_t)fct) return my_GdkSeatGrabPrepareFunc_##A;
    SUPER()
#undef GO
#define GO(A)                                               \
    if (my_GdkSeatGrabPrepareFunc_fct_##A == 0) {           \
        my_GdkSeatGrabPrepareFunc_fct_##A = (uintptr_t)fct; \
        return my_GdkSeatGrabPrepareFunc_##A;               \
    }
    SUPER()
#undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gdk3 GdkSeatGrabPrepareFunc\n");
    return NULL;
}

#undef SUPER


EXPORT void my3_gdk_event_handler_set(x64emu_t* emu, void* func, void* data, void* notify)
{
    return my->gdk_event_handler_set(findEventHandlerFct(func), data, findGDestroyNotifyFct(notify));
}


EXPORT void my3_gdk_init(x64emu_t* emu, void* argc, void* argv)
{
    my->gdk_init(argc, argv);
    my_checkGlobalGdkDisplay();
}

EXPORT int my3_gdk_init_check(x64emu_t* emu, void* argc, void* argv)
{
    int ret = my->gdk_init_check(argc, argv);
    my_checkGlobalGdkDisplay();
    return ret;
}

EXPORT void my3_gdk_window_add_filter(x64emu_t* emu, void* window, void* f, void* data)
{
    my->gdk_window_add_filter(window, findFilterFct(f), data);
}

EXPORT void my3_gdk_window_remove_filter(x64emu_t* emu, void* window, void* f, void* data)
{
    my->gdk_window_remove_filter(window, findFilterFct(f), data);
}

EXPORT uint32_t my3_gdk_threads_add_idle_full(x64emu_t* emu, int priority, void* f, void* data, void* d)
{
    return my->gdk_threads_add_idle_full(priority, findGSourceFunc(f), data, findGDestroyNotifyFct(d));
}

EXPORT uint32_t my3_gdk_threads_add_idle(x64emu_t* emu, void* f, void* data)
{
    return my->gdk_threads_add_idle(findGSourceFunc(f), data);
}

EXPORT uint32_t my3_gdk_threads_add_timeout_full(x64emu_t* emu, int priotity, uint32_t interval, void* f, void* data, void* d)
{
    return my->gdk_threads_add_timeout_full(priotity, interval, findGSourceFunc(f), data, findGDestroyNotifyFct(d));
}

EXPORT void my3_gdk_threads_set_lock_functions(x64emu_t* emu, void* enter_fn, void* leave_fn)
{
    my->gdk_threads_set_lock_functions(findGCallbackFct(enter_fn), findGCallbackFct(leave_fn));
}

EXPORT uint32_t my3_gdk_seat_grab(x64emu_t* emu, void* seat, void* window, uint32_t cap, int oes, void* cursor, void* ev, void* func, void* data)
{
    return my->gdk_seat_grab(seat, window, cap, oes, cursor, ev, findGdkSeatGrabPrepareFuncFct(func), data);
}

#define PRE_INIT \
    if (BOX64ENV(nogtk)) return -2;

#define ALTMY my3_

#define NEEDED_LIBS "libgobject-2.0.so.0", "libgio-2.0.so.0", "libgdk_pixbuf-2.0.so.0"

#include "wrappedlib_init.h"
