#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>
#include "wrappedlibs.h"
#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include "debug.h"
#include "callback.h"

const char* libharfbuzzName = "libharfbuzz.so.0";
#define LIBNAME libharfbuzz

#include "generated/wrappedlibharfbuzztypes.h"

#include "wrappercallback.h"

#define SUPER() \
    GO(0)   \
    GO(1)   \
    GO(2)   \
    GO(3)

// destroy
#define GO(A)                                               \
static uintptr_t my_destroy_fct_##A = 0;                    \
static void my_destroy_##A(void* a)                         \
{                                                           \
    RunFunctionFmt(my_context, my_destroy_fct_##A, "p", a); \
}
SUPER()
#undef GO
static void* find_destroy_Fct(void* fct)
{
    if (!fct) return NULL;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if (my_destroy_fct_##A == (uintptr_t)fct) return my_destroy_##A;
    SUPER()
    #undef GO
    #define GO(A) if (my_destroy_##A == 0) {my_destroy_fct_##A = (uintptr_t)fct; return my_destroy_##A;}
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libharfbuzz destroy callback\n");
    return NULL;
}

#undef SUPER

EXPORT void* my_hb_blob_create(x64emu_t* emu, void* data, uint32_t length, uint32_t mode, void* user_data, void* destroy)
{
    (void)emu;
    return my->hb_blob_create(data, length, mode, user_data, find_destroy_Fct(destroy));
}

#include "wrappedlib_init.h"
