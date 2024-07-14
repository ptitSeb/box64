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

const char* p11kitName = "libp11-kit.so.0";
#define LIBNAME p11kit

#define ADDED_FUNCTIONS()

#include "generated/wrappedp11kittypes.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)

// p11_kit_pin_destroy_func
#define GO(A)   \
static uintptr_t my_p11_kit_pin_destroy_func_fct_##A = 0;           \
static void my_p11_kit_pin_destroy_func_##A(void* a)                \
{                                                                   \
    RunFunctionFmt(my_p11_kit_pin_destroy_func_fct_##A, "p", a);    \
}
SUPER()
#undef GO
static void* findp11_kit_pin_destroy_funcFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_p11_kit_pin_destroy_func_fct_##A == (uintptr_t)fct) return my_p11_kit_pin_destroy_func_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_p11_kit_pin_destroy_func_fct_##A == 0) {my_p11_kit_pin_destroy_func_fct_##A = (uintptr_t)fct; return my_p11_kit_pin_destroy_func_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libp11-kit.so.0 p11_kit_pin_destroy_func callback\n");
    return NULL;
}

// p11_kit_pin_callback
#define GO(A)   \
static uintptr_t my_p11_kit_pin_callback_fct_##A = 0;                                       \
static void* my_p11_kit_pin_callback_##A(void* a, void* b, void* c, uint32_t d, void* e)    \
{                                                                                           \
    return (void*)RunFunctionFmt(my_p11_kit_pin_callback_fct_##A, "pppup", a, b, c, d, e);  \
}
SUPER()
#undef GO
static void* findp11_kit_pin_callbackFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_p11_kit_pin_callback_fct_##A == (uintptr_t)fct) return my_p11_kit_pin_callback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_p11_kit_pin_callback_fct_##A == 0) {my_p11_kit_pin_callback_fct_##A = (uintptr_t)fct; return my_p11_kit_pin_callback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libp11-kit.so.0 p11_kit_pin_callback callback\n");
    return NULL;
}

#undef SUPER

EXPORT int my_p11_kit_pin_register_callback(x64emu_t* emu, void* source, void* f, void* data, void* d)
{
    return my->p11_kit_pin_register_callback(source, findp11_kit_pin_callbackFct(f), data, findp11_kit_pin_destroy_funcFct(d));
}

EXPORT void my_p11_kit_pin_unregister_callback(x64emu_t* emu, void* source, void* f, void* data)
{
    my->p11_kit_pin_unregister_callback(source, findp11_kit_pin_callbackFct(f), data);
}

#include "wrappedlib_init.h"
