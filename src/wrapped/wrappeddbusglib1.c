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
#include "callback.h"

const char* dbusglib1Name = "libdbus-glib-1.so.2";
#define LIBNAME dbusglib1

#include "generated/wrappeddbusglib1types.h"

#include "wrappercallback.h"

#define SUPER() \
    GO(0)   \
    GO(1)   \
    GO(2)   \
    GO(3)

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
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GDestroyNotify_fct_##A == (uintptr_t)fct) return my_GDestroyNotify_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GDestroyNotify_fct_##A == 0) {my_GDestroyNotify_fct_##A = (uintptr_t)fct; return my_GDestroyNotify_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for dbus-glib1 GDestroyNotify callback\n");
    return NULL;
}

// GClosureNotify
#define GO(A)   \
static uintptr_t my_GClosureNotify_fct_##A = 0;                                 \
static void my_GClosureNotify_##A(void* data, void* closure)                    \
{                                                                               \
    RunFunctionFmt(my_GClosureNotify_fct_##A, "pp", data, closure);       \
}
SUPER()
#undef GO
static void* findGClosureNotifyFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GClosureNotify_fct_##A == (uintptr_t)fct) return my_GClosureNotify_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GClosureNotify_fct_##A == 0) {my_GClosureNotify_fct_##A = (uintptr_t)fct; return my_GClosureNotify_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for dbus-glib1 GClosureNotify callback\n");
    return NULL;
}

// DBusGProxyCallNotify
#define GO(A)   \
static uintptr_t my_DBusGProxyCallNotify_fct_##A = 0;                                           \
static void my_DBusGProxyCallNotify_##A(void* proxy, void* call_id, void* data)                 \
{                                                                                               \
    RunFunctionFmt(my_DBusGProxyCallNotify_fct_##A, "ppp", proxy, call_id, data);         \
}
SUPER()
#undef GO
static void* findDBusGProxyCallNotifyFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_DBusGProxyCallNotify_fct_##A == (uintptr_t)fct) return my_DBusGProxyCallNotify_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_DBusGProxyCallNotify_fct_##A == 0) {my_DBusGProxyCallNotify_fct_##A = (uintptr_t)fct; return my_DBusGProxyCallNotify_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for dbus-glib1 DBusGProxyCallNotify callback\n");
    return NULL;
}

// GCallback
#define GO(A)   \
static uintptr_t my_GCallback_fct_##A = 0;                                  \
static void my_GCallback_##A(void* a, void* b, void* c, void* d)            \
{                                                                           \
    RunFunctionFmt(my_GCallback_fct_##A, "pppp", a, b, c, d);         \
}
SUPER()
#undef GO
static void* findGCallbackFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GCallback_fct_##A == (uintptr_t)fct) return my_GCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GCallback_fct_##A == 0) {my_GCallback_fct_##A = (uintptr_t)fct; return my_GCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for dbus-glib1 GCallback callback\n");
    return NULL;
}

// DBusGTypeSpecializedCollectionIterator
#define GO(A)   \
static uintptr_t my_DBusGTypeSpecializedCollectionIterator_fct_##A = 0;                         \
static void my_DBusGTypeSpecializedCollectionIterator_##A(void* a, void* b)                     \
{                                                                                               \
    RunFunctionFmt(my_DBusGTypeSpecializedCollectionIterator_fct_##A, "pp", a, b);        \
}
SUPER()
#undef GO
static void* findDBusGTypeSpecializedCollectionIteratorFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_DBusGTypeSpecializedCollectionIterator_fct_##A == (uintptr_t)fct) return my_DBusGTypeSpecializedCollectionIterator_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_DBusGTypeSpecializedCollectionIterator_fct_##A == 0) {my_DBusGTypeSpecializedCollectionIterator_fct_##A = (uintptr_t)fct; return my_DBusGTypeSpecializedCollectionIterator_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for dbus-glib1 DBusGTypeSpecializedCollectionIterator callback\n");
    return NULL;
}
// DBusGTypeSpecializedMapIterator
#define GO(A)   \
static uintptr_t my_DBusGTypeSpecializedMapIterator_fct_##A = 0;                            \
static void my_DBusGTypeSpecializedMapIterator_##A(void* a, void* b, void* c)               \
{                                                                                           \
    RunFunctionFmt(my_DBusGTypeSpecializedMapIterator_fct_##A, "ppp", a, b, c);       \
}
SUPER()
#undef GO
static void* findDBusGTypeSpecializedMapIteratorFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_DBusGTypeSpecializedMapIterator_fct_##A == (uintptr_t)fct) return my_DBusGTypeSpecializedMapIterator_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_DBusGTypeSpecializedMapIterator_fct_##A == 0) {my_DBusGTypeSpecializedMapIterator_fct_##A = (uintptr_t)fct; return my_DBusGTypeSpecializedMapIterator_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for dbus-glib1 DBusGTypeSpecializedCollectionIterator callback\n");
    return NULL;
}
#undef SUPER

EXPORT void my_dbus_g_type_collection_value_iterate(x64emu_t* emu, void* value, void* cb, void* data)
{
    my->dbus_g_type_collection_value_iterate(value, findDBusGTypeSpecializedCollectionIteratorFct(cb), data);
}

EXPORT void my_dbus_g_type_map_value_iterate(x64emu_t* emu, void* value, void* cb, void* data)
{
    my->dbus_g_type_map_value_iterate(value, findDBusGTypeSpecializedMapIteratorFct(cb), data);
}

EXPORT void* my_dbus_g_proxy_begin_call(x64emu_t* emu, void* proxy, void* method, void* notify, void* data, void* destroy, int first, int64_t* next)
{
    int narg = 0;
    if(first)
        while(next[narg]) ++narg;
    void* n = findDBusGProxyCallNotifyFct(notify);
    void* d = findGDestroyNotifyFct(destroy);
    switch (narg) {
        case 0: return my->dbus_g_proxy_begin_call(proxy, method, n, data, d, first, 0);    // just in case
        case 1: return my->dbus_g_proxy_begin_call(proxy, method, n, data, d, first, next[0], 0);
        case 2: return my->dbus_g_proxy_begin_call(proxy, method, n, data, d, first, next[0], next[1], 0);
        default: printf_log(LOG_NONE, "Warning, no dbus-glib1 dbus_g_proxy_begin_call wrapper for %d args\n", narg);
    }
    return NULL;    // faillure
}

EXPORT void* my_dbus_g_proxy_begin_call_with_timeout(x64emu_t* emu, void* proxy, void* method, void* notify, void* data, void* destroy, int timeout, int first, int64_t* next)
{
    int narg = 0;
    if(first)
        while(next[narg]) ++narg;
    void* n = findDBusGProxyCallNotifyFct(notify);
    void* d = findGDestroyNotifyFct(destroy);
    switch (narg) {
        case 0: return my->dbus_g_proxy_begin_call_with_timeout(proxy, method, n, data, d, timeout, first, 0);    // just in case
        case 1: return my->dbus_g_proxy_begin_call_with_timeout(proxy, method, n, data, d, timeout, first, next[0], 0);
        case 2: return my->dbus_g_proxy_begin_call_with_timeout(proxy, method, n, data, d, timeout, first, next[0], next[1], 0);
        default: printf_log(LOG_NONE, "Warning, no dbus-glib1 dbus_g_proxy_begin_call_with_timeout wrapper for %d args\n", narg);
    }
    return NULL;    // faillure
}

EXPORT void my_dbus_g_proxy_connect_signal(x64emu_t* emu, void* proxy, void* name, void* handler, void* data, void* free_fnc)
{
    my->dbus_g_proxy_connect_signal(proxy, name, findGCallbackFct(handler), data, findGClosureNotifyFct(free_fnc));
}

EXPORT void my_dbus_g_proxy_disconnect_signal(x64emu_t* emu, void* proxy, void* name, void* handler, void* data)
{
    my->dbus_g_proxy_disconnect_signal(proxy, name, findGCallbackFct(handler), data);
}

#define PRE_INIT \
    if (BOX64ENV(nogtk)) return -2;

#include "wrappedlib_init.h"
