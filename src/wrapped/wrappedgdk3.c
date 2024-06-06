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

#ifdef ANDROID
    const char* gdk3Name = "libgdk-3.so";
#else
    const char* gdk3Name = "libgdk-3.so.0";
#endif
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

#undef SUPER


static void my3_event_handler(void* event, my_signal_t* sig)
{
    RunFunctionFmt(sig->c_handler, "pp", event, sig->data)        ;
}

EXPORT void my3_gdk_event_handler_set(x64emu_t* emu, void* func, void* data, void* notify)
{
    if(!func)
        return my->gdk_event_handler_set(func, data, notify);

    my_signal_t* sig = new_mysignal(func, data, notify);
    my->gdk_event_handler_set(my3_event_handler, sig, my_signal_delete);
}


static void my3_input_function(my_signal_t* sig, int source, int condition)
{
    RunFunctionFmt(sig->c_handler, "pii", sig->data, source, condition)       ;
}

EXPORT int my3_gdk_input_add(x64emu_t* emu, int source, int condition, void* f, void* data)
{
    if(!f)
        return my->gdk_input_add_full(source, condition, f, data, NULL);

    my_signal_t* sig = new_mysignal(f, data, NULL);
    return my->gdk_input_add_full(source, condition, my3_input_function, sig, my_signal_delete);
}

EXPORT int my3_gdk_input_add_full(x64emu_t* emu, int source, int condition, void* f, void* data, void* notify)
{
    if(!f)
        return my->gdk_input_add_full(source, condition, f, data, notify);

    my_signal_t* sig = new_mysignal(f, data, notify);
    return my->gdk_input_add_full(source, condition, my3_input_function, sig, my_signal_delete);
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

#define PRE_INIT    \
    if(box64_nogtk) \
        return -1;

#define ALTMY my3_

#ifdef ANDROID
#define NEEDED_LIBS "libgobject-2.0.so", "libgio-2.0.so", "libgdk_pixbuf-2.0.so"
#else
#define NEEDED_LIBS "libgobject-2.0.so.0", "libgio-2.0.so.0", "libgdk_pixbuf-2.0.so.0"
#endif

#include "wrappedlib_init.h"
