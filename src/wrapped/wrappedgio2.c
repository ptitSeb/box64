#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <stdarg.h>

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
#include "gtkclass.h"

const char* gio2Name = "libgio-2.0.so.0";
#define ALTNAME "libgio-2.0.so"

#define LIBNAME gio2

typedef size_t(*LFv_t)(void);
typedef void(*vFppp_t)(void*, void*, void*);

typedef struct my_GActionEntry_s
{
  void*     name;
  vFppp_t   activate;
  void*     parameter_type;
  void*     state;
  vFppp_t   change_state;
  size_t    padding[3];
} my_GActionEntry_t;

#define ADDED_FUNCTIONS() \
 GO(g_application_get_type, LFv_t)                  \
 GO(g_dbus_proxy_get_type, LFv_t)                   \
 GO(g_dbus_object_manager_client_get_type, LFv_t)   \
 GO(g_dbus_interface_skeleton_get_type, LFv_t)      \

#include "wrappedgio2types.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)

// GAsyncReadyCallback
#define GO(A)   \
static uintptr_t my_GAsyncReadyCallback_fct_##A = 0;                                      \
static void my_GAsyncReadyCallback_##A(void* source, void* res, void* data)               \
{                                                                                         \
    RunFunctionFmt(my_GAsyncReadyCallback_fct_##A, "ppp", source, res, data); \
}
SUPER()
#undef GO
static void* findGAsyncReadyCallbackFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GAsyncReadyCallback_fct_##A == (uintptr_t)fct) return my_GAsyncReadyCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GAsyncReadyCallback_fct_##A == 0) {my_GAsyncReadyCallback_fct_##A = (uintptr_t)fct; return my_GAsyncReadyCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gio2 GAsyncReadyCallback callback\n");
    return NULL;
}

// GDestroyNotify
#define GO(A)   \
static uintptr_t my_GDestroyNotify_fct_##A = 0;                       \
static void my_GDestroyNotify_##A(void* data)                         \
{                                                                     \
    RunFunctionFmt(my_GDestroyNotify_fct_##A, "p", data); \
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
    printf_log(LOG_NONE, "Warning, no more slot for gio2 GDestroyNotify callback\n");
    return NULL;
}

// GTaskThreadFunc
#define GO(A)                                                                                               \
    static uintptr_t my_GTaskThreadFunc_fct_##A = 0;                                                        \
    static void my_GTaskThreadFunc_##A(void* task, void* source_object, void* task_data, void* cancellable) \
    {                                                                                                       \
        RunFunctionFmt(my_GTaskThreadFunc_fct_##A, "pppp", task, source_object, task_data, cancellable);    \
    }
SUPER()
#undef GO
static void* findGTaskThreadFuncFct(void* fct)
{
    if (!fct) return fct;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
#define GO(A) \
    if (my_GTaskThreadFunc_fct_##A == (uintptr_t)fct) return my_GTaskThreadFunc_##A;
    SUPER()
#undef GO
#define GO(A)                                        \
    if (my_GTaskThreadFunc_fct_##A == 0) {           \
        my_GTaskThreadFunc_fct_##A = (uintptr_t)fct; \
        return my_GTaskThreadFunc_##A;               \
    }
    SUPER()
#undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gio2 GTaskThreadFunc callback\n");
    return NULL;
}

// GDBusProxyTypeFunc
#define GO(A)   \
static uintptr_t my_GDBusProxyTypeFunc_fct_##A = 0;                                                           \
static int my_GDBusProxyTypeFunc_##A(void* manager, void* path, void* name, void* data)                       \
{                                                                                                             \
    return (int)RunFunctionFmt(my_GDBusProxyTypeFunc_fct_##A, "pppp", manager, path, name, data); \
}
SUPER()
#undef GO
static void* findGDBusProxyTypeFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GDBusProxyTypeFunc_fct_##A == (uintptr_t)fct) return my_GDBusProxyTypeFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GDBusProxyTypeFunc_fct_##A == 0) {my_GDBusProxyTypeFunc_fct_##A = (uintptr_t)fct; return my_GDBusProxyTypeFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gio2 GDBusProxyTypeFunc callback\n");
    return NULL;
}

// GSimpleAsyncThreadFunc
#define GO(A)   \
static uintptr_t my_GSimpleAsyncThreadFunc_fct_##A = 0;                                             \
static void my_GSimpleAsyncThreadFunc_##A(void* res, void* object, void* cancellable)               \
{                                                                                                   \
    RunFunctionFmt(my_GSimpleAsyncThreadFunc_fct_##A, "ppp", res, object, cancellable); \
}
SUPER()
#undef GO
static void* findGSimpleAsyncThreadFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GSimpleAsyncThreadFunc_fct_##A == (uintptr_t)fct) return my_GSimpleAsyncThreadFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GSimpleAsyncThreadFunc_fct_##A == 0) {my_GSimpleAsyncThreadFunc_fct_##A = (uintptr_t)fct; return my_GSimpleAsyncThreadFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gio2 GSimpleAsyncThreadFunc callback\n");
    return NULL;
}

// GCallback
#define GO(A)   \
static uintptr_t my_GCallback_fct_##A = 0;                                \
static void my_GCallback_##A(void* a, void* b, void* c, void* d)          \
{                                                                         \
    RunFunctionFmt(my_GCallback_fct_##A, "pppp", a, b, c, d); \
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
    printf_log(LOG_NONE, "Warning, no more slot for gio2 GCallback callback\n");
    return NULL;
}

// GDBusSignalCallback
#define GO(A)   \
static uintptr_t my_GDBusSignalCallback_fct_##A = 0;                                                                                         \
static void my_GDBusSignalCallback_##A(void* connection, void* sender, void* object, void* interface, void* signal, void* param, void* data) \
{                                                                                                                                            \
    RunFunctionFmt(my_GDBusSignalCallback_fct_##A, "ppppppp", connection, sender, object, interface, signal, param, data);       \
}
SUPER()
#undef GO
static void* findGDBusSignalCallbackFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GDBusSignalCallback_fct_##A == (uintptr_t)fct) return my_GDBusSignalCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GDBusSignalCallback_fct_##A == 0) {my_GDBusSignalCallback_fct_##A = (uintptr_t)fct; return my_GDBusSignalCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gio2 GDBusSignalCallback callback\n");
    return NULL;
}

// GDBusMessageFilterFunction
#define GO(A)   \
static uintptr_t my_GDBusMessageFilterFunction_fct_##A = 0;                                                         \
static void my_GDBusMessageFilterFunction_##A(void* connection, void* message, int incoming, void* data)            \
{                                                                                                                   \
    RunFunctionFmt(my_GDBusMessageFilterFunction_fct_##A, "ppip", connection, message, incoming, data); \
}
SUPER()
#undef GO
static void* findGDBusMessageFilterFunctionFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GDBusMessageFilterFunction_fct_##A == (uintptr_t)fct) return my_GDBusMessageFilterFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GDBusMessageFilterFunction_fct_##A == 0) {my_GDBusMessageFilterFunction_fct_##A = (uintptr_t)fct; return my_GDBusMessageFilterFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gio2 GDBusMessageFilterFunction callback\n");
    return NULL;
}

// GBusNameAppearedCallback
#define GO(A)   \
static uintptr_t my_GBusNameAppearedCallback_fct_##A = 0;                                                   \
static void my_GBusNameAppearedCallback_##A(void* connection, void* name, void* owner, void* data)          \
{                                                                                                           \
    RunFunctionFmt(my_GBusNameAppearedCallback_fct_##A, "pppp", connection, name, owner, data); \
}
SUPER()
#undef GO
static void* findGBusNameAppearedCallbackFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GBusNameAppearedCallback_fct_##A == (uintptr_t)fct) return my_GBusNameAppearedCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GBusNameAppearedCallback_fct_##A == 0) {my_GBusNameAppearedCallback_fct_##A = (uintptr_t)fct; return my_GBusNameAppearedCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gio2 GBusNameAppearedCallback callback\n");
    return NULL;
}

// GBusNameVanishedCallback
#define GO(A)   \
static uintptr_t my_GBusNameVanishedCallback_fct_##A = 0;                                           \
static void my_GBusNameVanishedCallback_##A(void* connection, void* name, void* data)               \
{                                                                                                   \
    RunFunctionFmt(my_GBusNameVanishedCallback_fct_##A, "ppp", connection, name, data); \
}
SUPER()
#undef GO
static void* findGBusNameVanishedCallbackFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GBusNameVanishedCallback_fct_##A == (uintptr_t)fct) return my_GBusNameVanishedCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GBusNameVanishedCallback_fct_##A == 0) {my_GBusNameVanishedCallback_fct_##A = (uintptr_t)fct; return my_GBusNameVanishedCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gio2 GBusNameVanishedCallback callback\n");
    return NULL;
}

// GBusAcquiredCallback
#define GO(A)   \
static uintptr_t my_GBusAcquiredCallback_fct_##A = 0;                                           \
static void my_GBusAcquiredCallback_##A(void* connection, void* name, void* data)               \
{                                                                                               \
    RunFunctionFmt(my_GBusAcquiredCallback_fct_##A, "ppp", connection, name, data); \
}
SUPER()
#undef GO
static void* findGBusAcquiredCallbackFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GBusAcquiredCallback_fct_##A == (uintptr_t)fct) return my_GBusAcquiredCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GBusAcquiredCallback_fct_##A == 0) {my_GBusAcquiredCallback_fct_##A = (uintptr_t)fct; return my_GBusAcquiredCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gio2 GBusAcquiredCallback callback\n");
    return NULL;
}

// GBusNameAcquiredCallback
#define GO(A)   \
static uintptr_t my_GBusNameAcquiredCallback_fct_##A = 0;                                           \
static void my_GBusNameAcquiredCallback_##A(void* connection, void* name, void* data)               \
{                                                                                                   \
    RunFunctionFmt(my_GBusNameAcquiredCallback_fct_##A, "ppp", connection, name, data); \
}
SUPER()
#undef GO
static void* findGBusNameAcquiredCallbackFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GBusNameAcquiredCallback_fct_##A == (uintptr_t)fct) return my_GBusNameAcquiredCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GBusNameAcquiredCallback_fct_##A == 0) {my_GBusNameAcquiredCallback_fct_##A = (uintptr_t)fct; return my_GBusNameAcquiredCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gio2 GBusNameAcquiredCallback callback\n");
    return NULL;
}

// GBusNameLostCallback
#define GO(A)   \
static uintptr_t my_GBusNameLostCallback_fct_##A = 0;                                           \
static void my_GBusNameLostCallback_##A(void* connection, void* name, void* data)               \
{                                                                                               \
    RunFunctionFmt(my_GBusNameLostCallback_fct_##A, "ppp", connection, name, data); \
}
SUPER()
#undef GO
static void* findGBusNameLostCallbackFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GBusNameLostCallback_fct_##A == (uintptr_t)fct) return my_GBusNameLostCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GBusNameLostCallback_fct_##A == 0) {my_GBusNameLostCallback_fct_##A = (uintptr_t)fct; return my_GBusNameLostCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gio2 GBusNameLostCallback callback\n");
    return NULL;
}
// vFppp
#define GO(A)   \
static uintptr_t my_vFppp_fct_##A = 0;                  \
static void my_vFppp_##A(void* a, void* b, void* c)     \
{                                                       \
    RunFunctionFmt(my_vFppp_fct_##A, "ppp", a, b, c);   \
}
SUPER()
#undef GO
static void* findvFpppFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_vFppp_fct_##A == (uintptr_t)fct) return my_vFppp_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_vFppp_fct_##A == 0) {my_vFppp_fct_##A = (uintptr_t)fct; return my_vFppp_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gio2 vFppp callback\n");
    return NULL;
}

// GDBusInterfaceVTable....
// First the structure GDBusInterfaceVTable statics, with paired x64 source pointer
#define GO(A)   \
static my_GDBusInterfaceVTable_t     my_GDBusInterfaceVTable_##A = {0};   \
static my_GDBusInterfaceVTable_t   *ref_GDBusInterfaceVTable_##A = NULL;
SUPER()
#undef GO
// then the static functions callback that may be used with the structure, but dispatch also have a callback...
#define GO(A)   \
static uintptr_t fct_funcs_method_call_##A = 0; \
static void my_funcs_method_call_##A(void* connection, void* sender, void* object_path, void* interface_name, void* method_name, void* parameters, void* invocation, void* user_data) { \
    RunFunctionFmt(fct_funcs_method_call_##A, "pppppppp", connection, sender, object_path, interface_name, method_name, parameters, invocation, user_data); \
} \
static uintptr_t fct_funcs_get_property_##A = 0; \
static void* my_funcs_get_property_##A(void* connection, void* sender, void* object_path, void* interface_name, void* property_name, void* error, void* user_data) { \
    return (void*)RunFunctionFmt(fct_funcs_get_property_##A, "ppppppp", connection, sender, object_path, interface_name, property_name, error, user_data); \
} \
static uintptr_t fct_funcs_set_property_##A = 0; \
static int my_funcs_set_property_##A(void* connection, void* sender, void* object_path, void* interface_name, void* property_name, void* value, void* error, void* user_data) { \
    return (int)RunFunctionFmt(fct_funcs_set_property_##A, "pppppppp", connection, sender, object_path, interface_name, property_name, value, error, user_data); \
}

SUPER()
#undef GO
// and now the get slot / assign... Taking into account that the desired callback may already be a wrapped one (so unwrapping it)
my_GDBusInterfaceVTable_t* findFreeGDBusInterfaceVTable(my_GDBusInterfaceVTable_t* fcts)
{
    if(!fcts) return fcts;
    #define GO(A) if(ref_GDBusInterfaceVTable_##A == fcts) return &my_GDBusInterfaceVTable_##A;
    SUPER()
    #undef GO
    #define GO(A) if(ref_GDBusInterfaceVTable_##A == 0) {   \
        ref_GDBusInterfaceVTable_##A = fcts;                 \
        my_GDBusInterfaceVTable_##A.method_call = (fcts->method_call)?((GetNativeFnc((uintptr_t)fcts->method_call))?GetNativeFnc((uintptr_t)fcts->method_call):(void*)my_funcs_method_call_##A):NULL;    \
        fct_funcs_method_call_##A = (uintptr_t)fcts->method_call;                            \
        my_GDBusInterfaceVTable_##A.get_property = (fcts->get_property)?((GetNativeFnc((uintptr_t)fcts->get_property))?GetNativeFnc((uintptr_t)fcts->get_property):(void*)my_funcs_get_property_##A):NULL;    \
        fct_funcs_get_property_##A = (uintptr_t)fcts->get_property;                            \
        my_GDBusInterfaceVTable_##A.set_property = (fcts->set_property)?((GetNativeFnc((uintptr_t)fcts->set_property))?GetNativeFnc((uintptr_t)fcts->set_property):(void*)my_funcs_set_property_##A):NULL;    \
        fct_funcs_set_property_##A = (uintptr_t)fcts->set_property;                            \
        return &my_GDBusInterfaceVTable_##A;                \
    }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gio2 GDBusInterfaceVTable callback\n");
    return NULL;
}
#undef SUPER

EXPORT void* my_g_task_new(x64emu_t* emu, void* source_object, void* cancellable, void* cb, void* data)
{
    return my->g_task_new(source_object, cancellable, findGAsyncReadyCallbackFct(cb), data);
}

EXPORT void my_g_task_return_pointer(x64emu_t* emu, void* task, void* result, void* destroy)
{
    my->g_task_return_pointer(task, result, findGDestroyNotifyFct(destroy));
}

EXPORT void my_g_task_set_task_data(x64emu_t* emu, void* task, void* data, void* destroy)
{
    my->g_task_set_task_data(task, data, findGDestroyNotifyFct(destroy));
}

EXPORT void my_g_task_run_in_thread(x64emu_t* emu, void* task, void* func)
{
    my->g_task_run_in_thread(task, findGTaskThreadFuncFct(func));
}

EXPORT void my_g_task_run_in_thread_sync(x64emu_t* emu, void* task, void* func)
{
    my->g_task_run_in_thread_sync(task, findGTaskThreadFuncFct(func));
}

EXPORT void my_g_dbus_proxy_new(x64emu_t* emu, void* connection, uint32_t flags, void* info, void* name, void* path, void* interface, void* cancellable, void* cb, void* data)
{
    my->g_dbus_proxy_new(connection, flags, info, name, path, interface, cancellable, findGAsyncReadyCallbackFct(cb), data);
}

EXPORT void my_g_dbus_proxy_new_for_bus(x64emu_t* emu, int bus_type, uint32_t flags, void* info, void* name, void* path, void* interface, void* cancellable, void* cb, void* data)
{
    my->g_dbus_proxy_new_for_bus(bus_type, flags, info, name, path, interface, cancellable, findGAsyncReadyCallbackFct(cb), data);
}

EXPORT void my_g_dbus_proxy_call(x64emu_t* emu, void* proxy, void* name, void* param, uint32_t flags, int timeout, void* cancellable, void* cb, void* data)
{
    my->g_dbus_proxy_call(proxy, name, param, flags, timeout, cancellable, findGAsyncReadyCallbackFct(cb), data);
}

EXPORT void my_g_dbus_proxy_call_with_unix_fd_list(x64emu_t* emu, void* proxy, void* name, void* param, uint32_t flags, int timeout, void* fd_list, void* cancellable, void* cb, void* data)
{
    my->g_dbus_proxy_call_with_unix_fd_list(proxy, name, param, flags, timeout, fd_list, cancellable, findGAsyncReadyCallbackFct(cb), data);
}

EXPORT void* my_g_dbus_object_manager_client_new_for_bus_sync(x64emu_t* emu, size_t bus, uint32_t flags, void* name, void* path, void* cb, void* data, void* destroy, void* cancellable, void* error)
{
    return my->g_dbus_object_manager_client_new_for_bus_sync(bus, flags, name, path, findGDBusProxyTypeFuncFct(cb), data, findGDestroyNotifyFct(destroy), cancellable, error);
}

EXPORT void* my_g_simple_async_result_new(x64emu_t* emu, void* source, void* cb, void* data, void* tag)
{
    return my->g_simple_async_result_new(source, findGAsyncReadyCallbackFct(cb), data, tag);
}

EXPORT void* my_g_simple_async_result_new_error(x64emu_t* emu, void* source, void* cb, void* data, uint32_t domain, int code, void* fmt, va_list b)
{
    char* tmp;
    int dummy = vasprintf(&tmp, fmt, b);
    (void)dummy;
    void* ret = my->g_simple_async_result_new_error(source, findGAsyncReadyCallbackFct(cb), data, domain, code, tmp);
    free(tmp);
    return ret;
}

EXPORT void* my_g_simple_async_result_new_from_error(x64emu_t* emu, void* source, void* cb, void* data, void* error)
{
    return my->g_simple_async_result_new_from_error(source, findGAsyncReadyCallbackFct(cb), data, error);
}

EXPORT void* my_g_simple_async_result_new_take_error(x64emu_t* emu, void* source, void* cb, void* data, void* error)
{
    return my->g_simple_async_result_new_take_error(source, findGAsyncReadyCallbackFct(cb), data, error);
}

EXPORT void my_g_simple_async_result_set_op_res_gpointer(x64emu_t* emu, void* simple, void* op, void* notify)
{
    my->g_simple_async_result_set_op_res_gpointer(simple, op, findGDestroyNotifyFct(notify));
}

EXPORT void my_g_simple_async_result_run_in_thread(x64emu_t* emu, void* simple, void* fnc, int priority, void* cancellable)
{
    return my->g_simple_async_result_run_in_thread(simple, findGSimpleAsyncThreadFuncFct(fnc), priority, cancellable);
}

EXPORT void my_g_simple_async_report_error_in_idle(x64emu_t* emu, void* object, void* cb, void* data, uint32_t domain, int code, void* fmt, va_list b)
{
    char* tmp;
    int dummy = vasprintf(&tmp, fmt, b);
    (void)dummy;
    my->g_simple_async_report_error_in_idle(object, findGAsyncReadyCallbackFct(cb), data, domain, code, tmp);
    free(tmp);
}

EXPORT void my_g_simple_async_report_gerror_in_idle(x64emu_t* emu, void* object, void* cb, void* data, void* error)
{
    my->g_simple_async_report_gerror_in_idle(object, findGAsyncReadyCallbackFct(cb), data, error);
}

EXPORT void my_g_simple_async_report_take_gerror_in_idle(x64emu_t* emu, void* object, void* cb, void* data, void* error)
{
    my->g_simple_async_report_take_gerror_in_idle(object, findGAsyncReadyCallbackFct(cb), data, error);
}

EXPORT unsigned long my_g_cancellable_connect(x64emu_t* emu, void* cancellable, void* cb, void* data, void* notify)
{
    return my->g_cancellable_connect(cancellable, findGCallbackFct(cb), data, findGDestroyNotifyFct(notify));
}

EXPORT void my_g_async_initable_init_async(x64emu_t* emu, void* initable, int priority, void* cancellable, void* cb, void* data)
{
    my->g_async_initable_init_async(initable, priority, cancellable, findGAsyncReadyCallbackFct(cb), data);
}

EXPORT void my_g_async_initable_new_valist_async(x64emu_t* emu, size_t type, void* first, x64_va_list_t var_args, int priority, void* cancellable, void* cb, void* data)
{
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(var_args);
    #else
    CREATE_VALIST_FROM_VALIST(var_args, emu->scratch);
    #endif
    my->g_async_initable_new_valist_async(type, first, VARARGS, priority, cancellable, findGAsyncReadyCallbackFct(cb), data);
}

EXPORT void my_g_async_initable_new_async(x64emu_t* emu, size_t type, int priority, void* cancellable, void* cb, void* data, void* first, uintptr_t* V)
{
    CREATE_VALIST_FROM_VAARG(V, emu->scratch, 6);
    my->g_async_initable_new_valist_async(type, first, VARARGS, priority, cancellable, findGAsyncReadyCallbackFct(cb), data);
}

EXPORT void my_g_async_initable_newv_async(x64emu_t* emu, size_t type, uint32_t n, void* params, int priority, void* cancellable, void* cb, void* data)
{
    my->g_async_initable_newv_async(type, n, params, priority, cancellable, findGAsyncReadyCallbackFct(cb), data);
}

EXPORT void my_g_bus_get(x64emu_t* emu, size_t type, void* cancellable, void* cb, void* data)
{
    my->g_bus_get(type, cancellable, findGAsyncReadyCallbackFct(cb), data);
}

EXPORT void my_g_dbus_connection_new(x64emu_t* emu, void* stream, void* guid, uint32_t flags, void* observer, void* cancellable, void* cb, void* data)
{
    my->g_dbus_connection_new(stream, guid, flags, observer, cancellable, findGAsyncReadyCallbackFct(cb), data);
}

EXPORT void my_g_dbus_connection_new_for_address(x64emu_t* emu, void* address, uint32_t flags, void* observer, void* cancellable, void* cb, void* data)
{
    my->g_dbus_connection_new_for_address(address, flags, observer, cancellable, findGAsyncReadyCallbackFct(cb), data);
}

EXPORT void my_g_dbus_connection_close(x64emu_t* emu, void* connection, void* cancellable, void* cb, void* data)
{
    my->g_dbus_connection_close(connection, cancellable, findGAsyncReadyCallbackFct(cb), data);
}

EXPORT void my_g_dbus_connection_flush(x64emu_t* emu, void* connection, void* cancellable, void* cb, void* data)
{
    my->g_dbus_connection_flush(connection, cancellable, findGAsyncReadyCallbackFct(cb), data);
}

EXPORT void my_g_dbus_connection_call(x64emu_t* emu, void* connection, void* bus, void* object, void* interface, void* method, void* param, void* reply, uint32_t flags, int timeout, void* cancellable, void* cb, void* data)
{
    my->g_dbus_connection_call(connection, bus, object, interface, method, param, reply, flags, timeout, cancellable, findGAsyncReadyCallbackFct(cb), data);
}

EXPORT uint32_t my_g_dbus_connection_signal_subscribe(x64emu_t* emu, void* connection, void* sender, void* interface, void* member, void* object, void* arg0, uint32_t flags, void* cb, void* data, void* notify)
{
    return my->g_dbus_connection_signal_subscribe(connection, sender, interface, member, object, arg0, flags, findGDBusSignalCallbackFct(cb), data, findGDestroyNotifyFct(notify));
}

EXPORT void my_g_dbus_connection_send_message_with_reply(x64emu_t* emu, void* connection, void* message, uint32_t flags, int timeout, void* serial, void* cancellable, void* cb, void* data)
{
    my->g_dbus_connection_send_message_with_reply(connection, message, flags, timeout, serial, cancellable, findGAsyncReadyCallbackFct(cb), data);
}

EXPORT uint32_t my_g_dbus_connection_add_filter(x64emu_t* emu, void* connection, void* cb, void* data, void* notify)
{
    return my->g_dbus_connection_add_filter(connection, findGDBusMessageFilterFunctionFct(cb), data, findGDestroyNotifyFct(notify));
}

EXPORT uint32_t my_g_dbus_connection_register_object(x64emu_t* emu, void* connection, void* object, void* info, my_GDBusInterfaceVTable_t* vtable, void* data, void* notify, void* error)
{
    return my->g_dbus_connection_register_object(connection, object, info, findFreeGDBusInterfaceVTable(vtable), data, findGDestroyNotifyFct(notify), error);
}

EXPORT uint32_t my_g_bus_watch_name(x64emu_t* emu, int type, void* name, uint32_t flags, void* appeared, void* vanished, void* data, void* notify)
{
    return my->g_bus_watch_name(type, name, flags, findGBusNameAppearedCallbackFct(appeared), findGBusNameVanishedCallbackFct(vanished), data, findGDestroyNotifyFct(notify));
}

EXPORT uint32_t my_g_bus_watch_name_on_connection(x64emu_t* emu, void* connection, void* name, uint32_t flags, void* appeared, void* vanished, void* data, void* notify)
{
    return my->g_bus_watch_name_on_connection(connection, name, flags, findGBusNameAppearedCallbackFct(appeared), findGBusNameVanishedCallbackFct(vanished), data, findGDestroyNotifyFct(notify));
}

EXPORT uint32_t my_g_bus_own_name(x64emu_t* emu, size_t type, void* name, uint32_t flags, void* bus_acquired, void* name_acquired, void* name_lost, void* data, void* notify)
{
    return my->g_bus_own_name(type, name, flags, findGBusAcquiredCallbackFct(bus_acquired), findGBusNameAcquiredCallbackFct(name_acquired), findGBusNameLostCallbackFct(name_lost), data, findGDestroyNotifyFct(notify));
}

EXPORT uint32_t my_g_bus_own_name_on_connection(x64emu_t* emu, void* connection, void* name, uint32_t flags, void* name_acquired, void* name_lost, void* data, void* notify)
{
    return my->g_bus_own_name_on_connection(connection, name, flags, findGBusNameAcquiredCallbackFct(name_acquired), findGBusNameLostCallbackFct(name_lost), data, findGDestroyNotifyFct(notify));
}

EXPORT void my_g_simple_async_result_set_error_va(x64emu_t* emu, void* simple, uint32_t domain, int code, void* fmt, x64_va_list_t V)
{
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(V);
    #else
    CREATE_VALIST_FROM_VALIST(V, emu->scratch);
    #endif
    my->g_simple_async_result_set_error_va(simple, domain, code, fmt, VARARGS);
}

EXPORT void my_g_simple_async_result_set_error(x64emu_t* emu, void* simple, uint32_t domain, int code, void* fmt, uintptr_t* b)
{
    myStackAlign(emu, fmt, b, emu->scratch, R_EAX, 4);
    PREPARE_VALIST;
    my->g_simple_async_result_set_error_va(simple, domain, code, fmt, VARARGS);
}

EXPORT void* my_g_initable_new(x64emu_t* emu, size_t type, void* cancel, void* err, void* first, uintptr_t* b)
{
    #if 0
    // look for number of pairs
    int n = 1;
    emu->scratch[0] = (uint64_t)first;
    emu->scratch[1] = getVArgs(emu, 4, b, 0);
    while(getVArgs(emu, 4, b, n)) {
        emu->scratch[n+1] = getVArgs(emu, 4, b, n);
        emu->scratch[n+2] = getVArgs(emu, 4, b, n+1);
        n+=2;
    }
    emu->scratch[n+1] = 0;
    emu->scratch[n+2] = 0;
    PREPARE_VALIST;
    #else
    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 4);
    #endif
    return my->g_initable_new_valist(type, first, VARARGS, cancel, err);
}

EXPORT void* my_g_initable_new_valist(x64emu_t* emu, size_t type, void* first, x64_va_list_t V, void* cancel, void* err)
{
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(V);
    #else
    CREATE_VALIST_FROM_VALIST(V, emu->scratch);
    #endif
    return my->g_initable_new_valist(type, first, VARARGS, cancel, err);
}

EXPORT void my_g_task_return_new_error(x64emu_t* emu, void* task, uint32_t domain, int code, void *fmt, va_list b)
{
    char* tmp;
    int dummy = vasprintf(&tmp, fmt, b);
    (void)dummy;
    my->g_task_return_new_error(task, domain, code, tmp);
    free(tmp);
}

EXPORT void my_g_input_stream_read_async(x64emu_t* emu, void* stream, void* buffer, size_t count, int io_prio, void* cancel, void* f, void* data)
{
    my->g_input_stream_read_async(stream, buffer, count, io_prio, cancel, findGAsyncReadyCallbackFct(f), data);
}

EXPORT void my_g_dbus_method_invocation_return_error_valist(x64emu_t* emu, void* invocation, uint32_t domain, int code, void* fmt, x64_va_list_t b)
{
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(b);
    #else
    myStackAlignValist(emu, (const char*)fmt, emu->scratch, b);
    PREPARE_VALIST;
    #endif
    my->g_dbus_method_invocation_return_error_valist(invocation, domain, code, fmt, VARARGS);
}

EXPORT void my_g_dbus_method_invocation_return_error(x64emu_t* emu, void* invocation, uint32_t domain, int code, void* fmt, uintptr_t* b)
{
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 4);
    PREPARE_VALIST;
    my->g_dbus_method_invocation_return_error_valist(invocation, domain, code, fmt, VARARGS);
}

EXPORT void my_g_action_map_add_action_entries(x64emu_t* emu, void* map, my_GActionEntry_t* entries, int n, void* data)
{
    my_GActionEntry_t entries_[n];
    memcpy(entries_, entries, sizeof(my_GActionEntry_t)*n);
    for(int i=0; i<n; ++i) {
        entries_[i].activate = findvFpppFct(entries[i].activate);
        entries_[i].change_state = findvFpppFct(entries[i].change_state);
    }
    my->g_action_map_add_action_entries(map, entries_, n, data);
}

#define PRE_INIT \
    if (BOX64ENV(nogtk)) return -2;

#define CUSTOM_INIT \
    SetGApplicationID(my->g_application_get_type());                            \
    SetGDBusProxyID(my->g_dbus_proxy_get_type());                               \
    SetGDBusObjectManagerClientID(my->g_dbus_object_manager_client_get_type()); \
    SetGDBusInterfaceSkeletonID(my->g_dbus_interface_skeleton_get_type());

#define NEEDED_LIBS "libgmodule-2.0.so.0"

#include "wrappedlib_init.h"
