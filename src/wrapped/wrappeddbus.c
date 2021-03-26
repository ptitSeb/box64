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
#include "myalign.h"

const char* dbusName = "libdbus-1.so.3";
#define LIBNAME dbus
static library_t* my_lib = NULL;

typedef void (*vFppp_t)(void*, void*, void*);
typedef void (*vFpppp_t)(void*, void*, void*, void*);
typedef int32_t (*iFpppp_t)(void*, void*, void*, void*);
typedef int32_t (*iFppiA_t)(void*, void*, int32_t, va_list);
typedef int32_t (*iFpipp_t)(void*, int32_t, void*, void*);
typedef int32_t (*iFppppp_t)(void*, void*, void*, void*, void*);
typedef int32_t (*iFpppppp_t)(void*, void*, void*, void*, void*, void*);

#define SUPER()                                                 \
    GO(dbus_timeout_set_data, vFppp_t)                          \
    GO(dbus_connection_set_timeout_functions, iFpppppp_t)       \
    GO(dbus_connection_add_filter, iFpppp_t)                    \
    GO(dbus_connection_remove_filter, vFppp_t)                  \
    GO(dbus_message_get_args_valist, iFppiA_t)                  \
    GO(dbus_message_set_data, iFpipp_t)                         \
    GO(dbus_pending_call_set_notify, iFpppp_t)                  \
    GO(dbus_pending_call_set_data, iFpipp_t)                    \
    GO(dbus_watch_set_data, vFppp_t)                            \
    GO(dbus_connection_set_dispatch_status_function, vFpppp_t)  \
    GO(dbus_connection_set_watch_functions, iFpppppp_t)         \
    GO(dbus_connection_try_register_object_path, iFppppp_t)     \
    GO(dbus_connection_set_data, iFpipp_t)                      \
    GO(dbus_connection_set_wakeup_main_function, vFpppp_t)      \
    GO(dbus_connection_try_register_fallback, iFppppp_t)        \


typedef struct dbus_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} dbus_my_t;

static void* getDBusMy(library_t* lib)
{
    dbus_my_t* my = (dbus_my_t*)calloc(1, sizeof(dbus_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

static void freeDBusMy(void* lib)
{
    //dbus_my_t *my = (dbus_my_t *)lib;
}

#define NF 4
#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)

// DBusFreeFunction
#define GO(A)   \
static uintptr_t my_DBusFreeFunction_fct_##A = 0;               \
static void my_DBusFreeFunction_##A(void* p)                    \
{                                                               \
    RunFunction(my_context, my_DBusFreeFunction_fct_##A, 1, p); \
}
SUPER()
#undef GO
static void* find_DBusFreeFunction_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_DBusFreeFunction_fct_##A == (uintptr_t)fct) return my_DBusFreeFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_DBusFreeFunction_fct_##A == 0) {my_DBusFreeFunction_fct_##A = (uintptr_t)fct; return my_DBusFreeFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libdbus-1.so DBusFreeFunction callback\n");
    return NULL;
}
// DBusHandleMessageFunction
#define GO(A)   \
static uintptr_t my_DBusHandleMessageFunction_fct_##A = 0;                              \
static int my_DBusHandleMessageFunction_##A(void* a, void* b, void* c)                  \
{                                                                                       \
    return RunFunction(my_context, my_DBusHandleMessageFunction_fct_##A, 3, a, b, c);   \
}
SUPER()
#undef GO
static void* find_DBusHandleMessageFunction_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_DBusHandleMessageFunction_fct_##A == (uintptr_t)fct) return my_DBusHandleMessageFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_DBusHandleMessageFunction_fct_##A == 0) {my_DBusHandleMessageFunction_fct_##A = (uintptr_t)fct; return my_DBusHandleMessageFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libdbus-1.so DBusHandleMessageFunction callback\n");
    return NULL;
}
// DBusAddTimeoutFunction
#define GO(A)   \
static uintptr_t my_DBusAddTimeoutFunction_fct_##A = 0;                         \
static int my_DBusAddTimeoutFunction_##A(void* a, void* b)                      \
{                                                                               \
    return RunFunction(my_context, my_DBusAddTimeoutFunction_fct_##A, 2, a, b); \
}
SUPER()
#undef GO
static void* find_DBusAddTimeoutFunction_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_DBusAddTimeoutFunction_fct_##A == (uintptr_t)fct) return my_DBusAddTimeoutFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_DBusAddTimeoutFunction_fct_##A == 0) {my_DBusAddTimeoutFunction_fct_##A = (uintptr_t)fct; return my_DBusAddTimeoutFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libdbus-1.so DBusAddTimeoutFunction callback\n");
    return NULL;
}
// DBusRemoveTimeoutFunction
#define GO(A)   \
static uintptr_t my_DBusRemoveTimeoutFunction_fct_##A = 0;                  \
static void my_DBusRemoveTimeoutFunction_##A(void* a, void* b)              \
{                                                                           \
    RunFunction(my_context, my_DBusRemoveTimeoutFunction_fct_##A, 2, a, b); \
}
SUPER()
#undef GO
static void* find_DBusRemoveTimeoutFunction_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_DBusRemoveTimeoutFunction_fct_##A == (uintptr_t)fct) return my_DBusRemoveTimeoutFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_DBusRemoveTimeoutFunction_fct_##A == 0) {my_DBusRemoveTimeoutFunction_fct_##A = (uintptr_t)fct; return my_DBusRemoveTimeoutFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libdbus-1.so DBusRemoveTimeoutFunction callback\n");
    return NULL;
}
// DBusTimeoutToggledFunction
#define GO(A)   \
static uintptr_t my_DBusTimeoutToggledFunction_fct_##A = 0;                     \
static void my_DBusTimeoutToggledFunction_##A(void* a, void* b)                 \
{                                                                               \
    RunFunction(my_context, my_DBusTimeoutToggledFunction_fct_##A, 2, a, b);    \
}
SUPER()
#undef GO
static void* find_DBusTimeoutToggledFunction_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_DBusTimeoutToggledFunction_fct_##A == (uintptr_t)fct) return my_DBusTimeoutToggledFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_DBusTimeoutToggledFunction_fct_##A == 0) {my_DBusTimeoutToggledFunction_fct_##A = (uintptr_t)fct; return my_DBusTimeoutToggledFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libdbus-1.so DBusTimeoutToggledFunction callback\n");
    return NULL;
}
// DBusWakeupMainFunction
#define GO(A)   \
static uintptr_t my_DBusWakeupMainFunction_fct_##A = 0;                 \
static void my_DBusWakeupMainFunction_##A(void* a)                      \
{                                                                       \
    RunFunction(my_context, my_DBusWakeupMainFunction_fct_##A, 1, a);   \
}
SUPER()
#undef GO
static void* find_DBusWakeupMainFunction_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_DBusWakeupMainFunction_fct_##A == (uintptr_t)fct) return my_DBusWakeupMainFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_DBusWakeupMainFunction_fct_##A == 0) {my_DBusWakeupMainFunction_fct_##A = (uintptr_t)fct; return my_DBusWakeupMainFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libdbus-1.so DBusWakeupMainFunction callback\n");
    return NULL;
}

// DBusPendingCallNotifyFunction
#define GO(A)   \
static uintptr_t my_DBusPendingCallNotifyFunction_fct_##A = 0;   \
static void my_DBusPendingCallNotifyFunction_##A(void* pending, void* data)     \
{                                       \
    RunFunction(my_context, my_DBusPendingCallNotifyFunction_fct_##A, 2, pending, data);\
}
SUPER()
#undef GO
static void* findDBusPendingCallNotifyFunctionFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_DBusPendingCallNotifyFunction_fct_##A == (uintptr_t)fct) return my_DBusPendingCallNotifyFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_DBusPendingCallNotifyFunction_fct_##A == 0) {my_DBusPendingCallNotifyFunction_fct_##A = (uintptr_t)fct; return my_DBusPendingCallNotifyFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for dbus DBusPendingCallNotifyFunction callback\n");
    return NULL;
}

// DBusDispatchStatusFunction
#define GO(A)   \
static uintptr_t my_DBusDispatchStatusFunction_fct_##A = 0;   \
static void my_DBusDispatchStatusFunction_##A(void* connection, int new_status, void* data)     \
{                                       \
    RunFunction(my_context, my_DBusDispatchStatusFunction_fct_##A, 3, connection, new_status, data);\
}
SUPER()
#undef GO
static void* findDBusDispatchStatusFunctionFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_DBusDispatchStatusFunction_fct_##A == (uintptr_t)fct) return my_DBusDispatchStatusFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_DBusDispatchStatusFunction_fct_##A == 0) {my_DBusDispatchStatusFunction_fct_##A = (uintptr_t)fct; return my_DBusDispatchStatusFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for dbus DBusDispatchStatusFunction callback\n");
    return NULL;
}

// DBusAddWatchFunction
#define GO(A)   \
static uintptr_t my_DBusAddWatchFunction_fct_##A = 0;   \
static int my_DBusAddWatchFunction_##A(void* watch, void* data)     \
{                                       \
    return (int)RunFunction(my_context, my_DBusAddWatchFunction_fct_##A, 2, watch, data);\
}
SUPER()
#undef GO
static void* findDBusAddWatchFunctionFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_DBusAddWatchFunction_fct_##A == (uintptr_t)fct) return my_DBusAddWatchFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_DBusAddWatchFunction_fct_##A == 0) {my_DBusAddWatchFunction_fct_##A = (uintptr_t)fct; return my_DBusAddWatchFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for dbus DBusAddWatchFunction callback\n");
    return NULL;
}

// DBusRemoveWatchFunction
#define GO(A)   \
static uintptr_t my_DBusRemoveWatchFunction_fct_##A = 0;   \
static void my_DBusRemoveWatchFunction_##A(void* watch, void* data)     \
{                                       \
    RunFunction(my_context, my_DBusRemoveWatchFunction_fct_##A, 2, watch, data);\
}
SUPER()
#undef GO
static void* findDBusRemoveWatchFunctionFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_DBusRemoveWatchFunction_fct_##A == (uintptr_t)fct) return my_DBusRemoveWatchFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_DBusRemoveWatchFunction_fct_##A == 0) {my_DBusRemoveWatchFunction_fct_##A = (uintptr_t)fct; return my_DBusRemoveWatchFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for dbus DBusRemoveWatchFunction callback\n");
    return NULL;
}

// DBusWatchToggledFunction
#define GO(A)   \
static uintptr_t my_DBusWatchToggledFunction_fct_##A = 0;   \
static void my_DBusWatchToggledFunction_##A(void* watch, void* data)     \
{                                       \
    RunFunction(my_context, my_DBusWatchToggledFunction_fct_##A, 2, watch, data);\
}
SUPER()
#undef GO
static void* findDBusWatchToggledFunctionFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_DBusWatchToggledFunction_fct_##A == (uintptr_t)fct) return my_DBusWatchToggledFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_DBusWatchToggledFunction_fct_##A == 0) {my_DBusWatchToggledFunction_fct_##A = (uintptr_t)fct; return my_DBusWatchToggledFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for dbus DBusWatchToggledFunction callback\n");
    return NULL;
}

// DBusObjectPathUnregisterFunction
#define GO(A)   \
static uintptr_t my_DBusObjectPathUnregisterFunction_fct_##A = 0;   \
static void my_DBusObjectPathUnregisterFunction_##A(void* connection, void* data)     \
{                                       \
    RunFunction(my_context, my_DBusObjectPathUnregisterFunction_fct_##A, 2, connection, data);\
}
SUPER()
#undef GO
static void* findDBusObjectPathUnregisterFunctionFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_DBusObjectPathUnregisterFunction_fct_##A == (uintptr_t)fct) return my_DBusObjectPathUnregisterFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_DBusObjectPathUnregisterFunction_fct_##A == 0) {my_DBusObjectPathUnregisterFunction_fct_##A = (uintptr_t)fct; return my_DBusObjectPathUnregisterFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for dbus DBusObjectPathUnregisterFunction callback\n");
    return NULL;
}

// DBusObjectPathMessageFunction
#define GO(A)   \
static uintptr_t my_DBusObjectPathMessageFunction_fct_##A = 0;   \
static void my_DBusObjectPathMessageFunction_##A(void* connection, void* message, void* data)     \
{                                       \
    RunFunction(my_context, my_DBusObjectPathMessageFunction_fct_##A, 3, connection, message, data);\
}
SUPER()
#undef GO
static void* findDBusObjectPathMessageFunctionFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_DBusObjectPathMessageFunction_fct_##A == (uintptr_t)fct) return my_DBusObjectPathMessageFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_DBusObjectPathMessageFunction_fct_##A == 0) {my_DBusObjectPathMessageFunction_fct_##A = (uintptr_t)fct; return my_DBusObjectPathMessageFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for dbus DBusObjectPathMessageFunction callback\n");
    return NULL;
}

// dbus_internal_pad
#define GO(A)   \
static uintptr_t my_dbus_internal_pad_fct_##A = 0;   \
static void my_dbus_internal_pad_##A(void* a, void* b, void* c, void* d)     \
{                                       \
    RunFunction(my_context, my_dbus_internal_pad_fct_##A, 4, a, b, c, d);\
}
SUPER()
#undef GO
static void* finddbus_internal_padFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_dbus_internal_pad_fct_##A == (uintptr_t)fct) return my_dbus_internal_pad_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_dbus_internal_pad_fct_##A == 0) {my_dbus_internal_pad_fct_##A = (uintptr_t)fct; return my_dbus_internal_pad_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for dbus dbus_internal_pad callback\n");
    return NULL;
}
#undef SUPER

EXPORT void my_dbus_timeout_set_data(x64emu_t* emu, void* e, void* p, void* f)
{
    dbus_my_t *my = (dbus_my_t*)my_lib->priv.w.p2;
    my->dbus_timeout_set_data(e, p, find_DBusFreeFunction_Fct(f));
}


EXPORT int32_t my_dbus_connection_set_timeout_functions(x64emu_t* emu, void* c, void* a, void* r, void* t, void* d, void* f)
{
    dbus_my_t *my = (dbus_my_t*)my_lib->priv.w.p2;

    return my->dbus_connection_set_timeout_functions(c, 
            find_DBusAddTimeoutFunction_Fct(a), 
            find_DBusRemoveTimeoutFunction_Fct(r), 
            find_DBusTimeoutToggledFunction_Fct(t), 
            d, find_DBusFreeFunction_Fct(f));
}

EXPORT int my_dbus_connection_add_filter(x64emu_t* emu, void* connection, void* fnc, void* data, void* fr)
{
    dbus_my_t *my = (dbus_my_t*)my_lib->priv.w.p2;
    return my->dbus_connection_add_filter(connection, find_DBusHandleMessageFunction_Fct(fnc), data, find_DBusFreeFunction_Fct(fr));
}

EXPORT void my_dbus_connection_remove_filter(x64emu_t* emu, void* connection, void* fnc, void* data)
{
    dbus_my_t *my = (dbus_my_t*)my_lib->priv.w.p2;
    my->dbus_connection_remove_filter(connection, find_DBusHandleMessageFunction_Fct(fnc), data);
}

EXPORT int my_dbus_message_get_args_valist(x64emu_t* emu, void* message, void* e, int arg, x64_va_list_t b)
{
    dbus_my_t *my = (dbus_my_t*)my_lib->priv.w.p2;
    CONVERT_VALIST(b);
    return my->dbus_message_get_args_valist(message, e, arg, VARARGS);
}

/*EXPORT int my_dbus_message_get_args(x64emu_t* emu, void* message, void* e, int arg, uint64_t* V)
{
    dbus_my_t *my = (dbus_my_t*)my_lib->priv.w.p2;

    return my->dbus_message_get_args_valist(message, e, arg, V);
}*/

EXPORT int my_dbus_message_set_data(x64emu_t* emu, void* message, int32_t slot, void* data, void* free_func)
{
    dbus_my_t *my = (dbus_my_t*)my_lib->priv.w.p2;

    return my->dbus_message_set_data(message, slot, data, find_DBusFreeFunction_Fct(free_func));
}

EXPORT int my_dbus_pending_call_set_notify(x64emu_t* emu, void* pending, void* func, void* data, void* free_func)
{
    dbus_my_t *my = (dbus_my_t*)my_lib->priv.w.p2;

    return my->dbus_pending_call_set_notify(pending, findDBusPendingCallNotifyFunctionFct(func), data, find_DBusFreeFunction_Fct(free_func));
}

EXPORT int my_dbus_pending_call_set_data(x64emu_t* emu, void* pending, int32_t slot, void* data, void* free_func)
{
    dbus_my_t *my = (dbus_my_t*)my_lib->priv.w.p2;

    return my->dbus_pending_call_set_data(pending, slot, data, find_DBusFreeFunction_Fct(free_func));
}

EXPORT void my_dbus_watch_set_data(x64emu_t* emu, void* watch, void* data, void* free_func)
{
    dbus_my_t *my = (dbus_my_t*)my_lib->priv.w.p2;

    my->dbus_watch_set_data(watch, data, find_DBusFreeFunction_Fct(free_func));
}

EXPORT void my_dbus_connection_set_dispatch_status_function(x64emu_t* emu, void* connection, void* dispatch, void* data, void* free_func)
{
    dbus_my_t *my = (dbus_my_t*)my_lib->priv.w.p2;

    my->dbus_connection_set_dispatch_status_function(connection, findDBusDispatchStatusFunctionFct(dispatch), data, find_DBusFreeFunction_Fct(free_func));
}

EXPORT int my_dbus_connection_set_watch_functions(x64emu_t* emu, void* connection, void* add, void* remove, void* toggled, void* data, void* free_func)
{
    dbus_my_t *my = (dbus_my_t*)my_lib->priv.w.p2;

    return my->dbus_connection_set_watch_functions(connection, findDBusAddWatchFunctionFct(add), findDBusRemoveWatchFunctionFct(remove), findDBusWatchToggledFunctionFct(toggled), data, find_DBusFreeFunction_Fct(free_func));
}

typedef struct my_DBusObjectPathVTable_s
{
   void*    unregister_function; 
   void*    message_function; 
   void*    pad1; 
   void*    pad2; 
   void*    pad3; 
   void*    pad4; 
} my_DBusObjectPathVTable_t;

EXPORT int my_dbus_connection_try_register_object_path(x64emu_t* emu, void* connection, void* path, my_DBusObjectPathVTable_t* vtable, void* data, void* error)
{
    dbus_my_t *my = (dbus_my_t*)my_lib->priv.w.p2;

    my_DBusObjectPathVTable_t vt = {0};
    if(vtable) {
        vt.unregister_function = findDBusObjectPathUnregisterFunctionFct(vtable->unregister_function);
        vt.message_function = findDBusObjectPathMessageFunctionFct(vtable->message_function);
        vt.pad1 = finddbus_internal_padFct(vtable->pad1);
        vt.pad2 = finddbus_internal_padFct(vtable->pad2);
        vt.pad3 = finddbus_internal_padFct(vtable->pad3);
        vt.pad4 = finddbus_internal_padFct(vtable->pad4);
    }

    return my->dbus_connection_try_register_object_path(connection, path, vtable?&vt:NULL, data, error);
}

EXPORT int my_dbus_connection_set_data(x64emu_t* emu, void* connection, int slot, void* data, void* free_func)
{
    dbus_my_t *my = (dbus_my_t*)my_lib->priv.w.p2;

    return my->dbus_connection_set_data(connection, slot, data, find_DBusFreeFunction_Fct(free_func));
}

EXPORT void my_dbus_connection_set_wakeup_main_function(x64emu_t* emu, void* connection, void* wakeup, void* data, void* free_func)
{
    dbus_my_t *my = (dbus_my_t*)my_lib->priv.w.p2;

    my->dbus_connection_set_wakeup_main_function(connection, find_DBusWakeupMainFunction_Fct(wakeup), data, find_DBusFreeFunction_Fct(free_func));
}

EXPORT int my_dbus_connection_try_register_fallback(x64emu_t* emu, void* connection, void* path, my_DBusObjectPathVTable_t* vtable, void* data, void* error)
{
    dbus_my_t *my = (dbus_my_t*)my_lib->priv.w.p2;

    my_DBusObjectPathVTable_t vt = {0};
    if(vtable) {
        vt.unregister_function = findDBusObjectPathUnregisterFunctionFct(vtable->unregister_function);
        vt.message_function = findDBusObjectPathMessageFunctionFct(vtable->message_function);
        vt.pad1 = finddbus_internal_padFct(vtable->pad1);
        vt.pad2 = finddbus_internal_padFct(vtable->pad2);
        vt.pad3 = finddbus_internal_padFct(vtable->pad3);
        vt.pad4 = finddbus_internal_padFct(vtable->pad4);
    }

    return my->dbus_connection_try_register_fallback(connection, path, vtable?&vt:NULL, data, error);
}



#define CUSTOM_INIT \
    lib->priv.w.p2 = getDBusMy(lib);    \
    my_lib = lib;

#define CUSTOM_FINI \
    freeDBusMy(lib->priv.w.p2); \
    free(lib->priv.w.p2);       \
    my_lib = NULL;

#include "wrappedlib_init.h"

