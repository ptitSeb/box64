#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>
#include <stdarg.h>

#include "wrappedlibs.h"

#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "debug.h"
#include "myalign.h"
#include "callback.h"
#include "emu/x64emu_private.h"

const char* udev0Name = "libudev.so.0";
#define LIBNAME udev0
// fallback to 1 version... Not sure if really correct (probably not)
#define ALTNAME "libudev.so.1"

#define ADDED_FUNCTIONS()           \

#include "generated/wrappedudev0types.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// log_fn ...
#ifdef CONVERT_VALIST
#define GO(A)   \
static uintptr_t my_log_fn_fct_##A = 0;                                                                 \
static void my_log_fn_##A(void* udev, int p, void *f, int l, void* fn, void* fmt, x64_va_list_t args)   \
{                                                                                                       \
    CONVERT_VALIST(args)                                                                                \
    RunFunction(my_log_fn_fct_##A, 7, udev, p, f, l, fn, fmt, VARARGS);                                 \
}
#else
#define GO(A)   \
static uintptr_t my_log_fn_fct_##A = 0;                                                                 \
static void my_log_fn_##A(void* udev, int p, void *f, int l, void* fn, void* fmt, x64_va_list_t args)   \
{                                                                                                       \
    CREATE_VALIST_FROM_VALIST(args, thread_get_emu()->scratch);                                         \
    RunFunction(my_log_fn_fct_##A, 7, udev, p, f, l, fn, fmt, VARARGS);                                 \
}
#endif
SUPER()
#undef GO
static void* find_log_fn_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_log_fn_fct_##A == (uintptr_t)fct) return my_log_fn_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_log_fn_fct_##A == 0) {my_log_fn_fct_##A = (uintptr_t)fct; return my_log_fn_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for udev1 log_fn callback\n");
    return NULL;
}
#undef SUPER

EXPORT void my0_udev_set_log_fn(x64emu_t* emu, void* udev, void* f)
{
    my->udev_set_log_fn(udev, find_log_fn_Fct(f));
}

#define ALTMY my0_

#define NEEDED_LIBS "libudev.so.1"

#include "wrappedlib_init.h"
