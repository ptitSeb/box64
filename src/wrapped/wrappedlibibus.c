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

const char* libibusName = "libibus-1.0.so.5";
#define LIBNAME libibus

#define ADDED_FUNCTIONS() \

#include "wrappedlibibustypes.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)

// GAsyncReadyCallback
#define GO(A)   \
static uintptr_t my_GAsyncReadyCallback_fct_##A = 0;   \
static void my_GAsyncReadyCallback_##A(void* source, void* res, void* data)     \
{                                       \
    RunFunctionFmt(my_GAsyncReadyCallback_fct_##A, "ppp", source, res, data);\
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
    printf_log(LOG_NONE, "Warning, no more slot for libibus GAsyncReadyCallback callback\n");
    return NULL;
}

#undef SUPER

EXPORT void my_ibus_bus_create_input_context_async(x64emu_t* emu, void* bus, void* name, int timeout, void* cancel, void* f, void* data)
{
    my->ibus_bus_create_input_context_async(bus, name, timeout, cancel, findGAsyncReadyCallbackFct(f), data);
}

EXPORT void my_ibus_bus_current_input_context_async(x64emu_t* emu, void* bus, int timeout, void* cancel, void* f, void* data)
{
    my->ibus_bus_current_input_context_async(bus, timeout, cancel, findGAsyncReadyCallbackFct(f), data);
}

EXPORT void my_ibus_bus_get_global_engine_async(x64emu_t* emu, void* bus, int timeout, void* cancel, void* f, void* data)
{
    my->ibus_bus_get_global_engine_async(bus, timeout, cancel, findGAsyncReadyCallbackFct(f), data);
}

EXPORT void my_ibus_bus_list_active_engines_async(x64emu_t* emu, void* bus, int timeout, void* cancel, void* f, void* data)
{
    my->ibus_bus_list_active_engines_async(bus, timeout, cancel, findGAsyncReadyCallbackFct(f), data);
}

EXPORT void my_ibus_bus_list_engines_async(x64emu_t* emu, void* bus, int timeout, void* cancel, void* f, void* data)
{
    my->ibus_bus_list_engines_async(bus, timeout, cancel, findGAsyncReadyCallbackFct(f), data);
}

EXPORT void my_ibus_bus_set_global_engine_async(x64emu_t* emu, void* bus, void* engine, int timeout, void* cancel, void* f, void* data)
{
    my->ibus_bus_set_global_engine_async(bus, engine, timeout, cancel, findGAsyncReadyCallbackFct(f), data);
}

EXPORT void my_ibus_config_get_value_async(x64emu_t* emu, void* bus, void* section, void* name, int timeout, void* cancel, void* f, void* data)
{
    my->ibus_config_get_value_async(bus, section, name, timeout, cancel, findGAsyncReadyCallbackFct(f), data);
}

EXPORT void my_ibus_config_get_values_async(x64emu_t* emu, void* bus, void* section, int timeout, void* cancel, void* f, void* data)
{
    my->ibus_config_get_values_async(bus, section, timeout, cancel, findGAsyncReadyCallbackFct(f), data);
}

EXPORT void my_ibus_config_set_value_async(x64emu_t* emu, void* bus, void* section, void* name, void* value, int timeout, void* cancel, void* f, void* data)
{
    my->ibus_config_set_value_async(bus, section, name, value, timeout, cancel, findGAsyncReadyCallbackFct(f), data);
}

EXPORT void my_ibus_input_context_get_engine_async(x64emu_t* emu, void* bus, int timeout, void* cancel, void* f, void* data)
{
    my->ibus_input_context_get_engine_async(bus, timeout, cancel, findGAsyncReadyCallbackFct(f), data);
}

EXPORT void my_ibus_input_context_process_key_event_async(x64emu_t* emu, void* bus, uint32_t keyval, uint32_t keycode, uint32_t state, int timeout, void* cancel, void* f, void* data)
{
    my->ibus_input_context_process_key_event_async(bus, keyval, keycode, state, timeout, cancel, findGAsyncReadyCallbackFct(f), data);
}

#define PRE_INIT \
    if (BOX64ENV(nogtk)) return -2;

#define NEEDED_LIBS "libgio-2.0.so.0", "libgobject-2.0.so.0", "libglib-2.0.so.0"

#include "wrappedlib_init.h"
