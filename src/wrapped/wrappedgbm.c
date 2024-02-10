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

const char* gbmName = "libgbm.so.1";
#define LIBNAME gbm

#define ADDED_FUNCTIONS()                   \

#include "generated/wrappedgbmtypes.h"

#include "wrappercallback.h"

// utility functions
#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// destroy_user_data
#define GO(A)   \
static uintptr_t my_destroy_user_data_fct_##A = 0;              \
static void my_destroy_user_data_##A(void* a, void* b)          \
{                                                               \
    RunFunctionFmt(my_destroy_user_data_fct_##A, "pp", a, b);   \
}
SUPER()
#undef GO
static void* find_destroy_user_data_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_destroy_user_data_fct_##A == (uintptr_t)fct) return my_destroy_user_data_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_destroy_user_data_fct_##A == 0) {my_destroy_user_data_fct_##A = (uintptr_t)fct; return my_destroy_user_data_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libgbm destroy_user_data callback\n");
    return NULL;
}

#undef SUPER

EXPORT void my_gbm_bo_set_user_data(x64emu_t* emu, void* bo, void* data, void *f)
{
    my->gbm_bo_set_user_data(bo, data, find_destroy_user_data_Fct(f));
}

#include "wrappedlib_init.h"
