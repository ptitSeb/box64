// SPDX-License-Identifier: MIT
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
#include "debug.h"
#include "myalign.h"
#include "callback.h"
#include "emu/x64emu_private.h"

const char* systemdName = "libsystemd.so.0";
#define LIBNAME systemd

#define ADDED_FUNCTIONS()           \

#include "generated/wrappedsystemdtypes.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// sd_bus_message_handler_t ...
#define GO(A)   \
static uintptr_t my_sd_bus_message_handler_t_fct_##A = 0;                               \
static int my_sd_bus_message_handler_t_##A(void* a, void* b, void* c)                   \
{                                                                                       \
    return (int)RunFunctionFmt(my_sd_bus_message_handler_t_fct_##A, "ppp", a, b, c);    \
}
SUPER()
#undef GO
static void* find_sd_bus_message_handler_t_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_sd_bus_message_handler_t_fct_##A == (uintptr_t)fct) return my_sd_bus_message_handler_t_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_sd_bus_message_handler_t_fct_##A == 0) {my_sd_bus_message_handler_t_fct_##A = (uintptr_t)fct; return my_sd_bus_message_handler_t_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for systemd sd_bus_message_handler_t callback\n");
    return NULL;
}
#undef SUPER

EXPORT int my_sd_bus_match_signal(x64emu_t* emu, void* bus, void* slot, void* sender, void* path, void* itf, void* member, void* f, void* data)
{
    return my->sd_bus_match_signal(bus, slot, sender, path, itf, member, find_sd_bus_message_handler_t_Fct(f), data);
}

EXPORT int my_sd_bus_call_methodv(x64emu_t* emu, void* bus, void* dest, void* path, void* itf, void* member, void* ret, void* reply, void* types, x64_va_list_t b)
{
    #ifdef CONVERT_VALIST
    (void)emu;
    CONVERT_VALIST(b);
    #else
    // should create a my align function using types instead, similar to the one for the GVariant one
    CREATE_VALIST_FROM_VALIST(b, emu->scratch);
    #endif
    return my->sd_bus_call_methodv(bus, dest, path, itf, member, ret, reply, types, VARARGS);
}
EXPORT int my_sd_bus_call_method(x64emu_t* emu, void* bus, void* dest, void* path, void* itf, void* member, void* ret, void* reply, void* types, uint64_t* b)
{
    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 8);
    return my->sd_bus_call_methodv(bus, dest, path, itf, member, ret, reply, types, VARARGS);
}

EXPORT int my_sd_bus_message_readv(x64emu_t* emu, void* m, void* types, x64_va_list_t b)
{
    #ifdef CONVERT_VALIST
    (void)emu;
    CONVERT_VALIST(b);
    #else
    // should create a my align function using types instead, similar to the one for the GVariant one
    CREATE_VALIST_FROM_VALIST(b, emu->scratch);
    #endif
    return my->sd_bus_message_readv(m, types, VARARGS);
}
EXPORT int my_sd_bus_message_read(x64emu_t* emu, void* m, void* types, uint64_t* b)
{
    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 2);
    return my->sd_bus_message_readv(m, types, VARARGS);
}

#include "wrappedlib_init.h"

