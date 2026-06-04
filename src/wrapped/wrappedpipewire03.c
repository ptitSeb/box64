#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
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
#include "box64context.h"
#include "librarian.h"
#include "myalign.h"

const char* pipewire03Name = "libpipewire-0.3.so";

#define LIBNAME pipewire03

typedef int (*iFppp_t)(void*, void*, void*);
typedef int (*iFpppA_t)(void*, void*, void*, va_list);

#define ADDED_FUNCTIONS()                   \
    GO(pw_properties_set, iFppp_t)          \
    GO(pw_properties_setva, iFpppA_t)       \

#include "generated/wrappedpipewire03types.h"

#include "wrappercallback.h"

EXPORT void* my_pw_properties_new(x64emu_t* emu, void* key, uintptr_t* b)
{
    void* props = my->pw_properties_new(NULL, NULL);
    if(!props || !key)
        return props;

    int n = 0;
    while(key) {
        void* value = (void*)getVArgs(emu, 1, b, n++);
        my->pw_properties_set(props, key, value);
        key = (void*)getVArgs(emu, 1, b, n++);
    }

    return props;
}

EXPORT int my_pw_properties_setf(x64emu_t* emu, void* props, void* key, void* fmt, uintptr_t* b)
{
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 3);
    PREPARE_VALIST;
    return my->pw_properties_setva(props, key, fmt, VARARGS);
}

#include "wrappedlib_init.h"
