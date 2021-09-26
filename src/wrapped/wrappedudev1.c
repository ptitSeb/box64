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

const char* udev1Name = "libudev.so.1";
#define LIBNAME udev1
// fallback to 0 version... Not sure if really correct (probably not)
#define ALTNAME "libudev.so.0"

static library_t* my_lib = NULL;

typedef void (*vFpp_t)(void*, void*);

#define SUPER()                 \
GO(udev_set_log_fn, vFpp_t)     \

typedef struct udev1_my_s {
    // functions
    #define GO(A, W)    W A;
    SUPER()
    #undef GO
} udev1_my_t;

static void* getUdev1My(library_t* lib)
{
    udev1_my_t* my = (udev1_my_t*)calloc(1, sizeof(udev1_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}

static void freeUdev1My(void* lib)
{
    (void)lib;
    //udev1_my_t *my = (udev1_my_t *)lib;
}
#undef SUPER

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
    RunFunction(my_context, my_log_fn_fct_##A, 7, udev, p, f, l, fn, fmt, VARARGS);                     \
}
#else
#define GO(A)   \
static uintptr_t my_log_fn_fct_##A = 0;                                                                 \
static void my_log_fn_##A(void* udev, int p, void *f, int l, void* fn, void* fmt, x64_va_list_t args)   \
{                                                                                                       \
    CREATE_VALIST_FROM_VALIST(args, thread_get_emu()->scratch);                                         \
    RunFunction(my_context, my_log_fn_fct_##A, 7, udev, p, f, l, fn, fmt, VARARGS);                     \
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

EXPORT void my_udev_set_log_fn(x64emu_t* emu, void* udev, void* f)
{
    udev1_my_t* my = (udev1_my_t*)my_lib->priv.w.p2;

    my->udev_set_log_fn(udev, find_log_fn_Fct(f));
}

#define CUSTOM_INIT \
    lib->priv.w.p2 = getUdev1My(lib); \
    my_lib = lib;

#define CUSTOM_FINI \
    freeUdev1My(lib->priv.w.p2);\
    free(lib->priv.w.p2);       \
    my_lib = NULL;

#include "wrappedlib_init.h"

