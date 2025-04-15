#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>
#include <stdarg.h>
#define __STDC_WANT_LIB_EXT2__ 1 // for vasprintf
#include <stdio.h>

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

const char* libavutil56Name = "libavutil.so.56";

#define LIBNAME libavutil56

#define ADDED_FUNCTIONS()                   \

#include "generated/wrappedlibavutil56types.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// log_callback ...
#define GO(A)   \
static uintptr_t my_log_callback_fct_##A = 0;                           \
static void my_log_callback_##A(void* a, int b, void* c, va_list d)     \
{                                                                       \
    x64_va_list_t null_va = {0};                                        \
    char* p = NULL;                                                     \
    (void)!vasprintf(&p, c, d);                                         \
    RunFunctionFmt(my_log_callback_fct_##A, "pipp", a, b, d, null_va);  \
    free(p);                                                            \
}
SUPER()
#undef GO
static void* find_log_callback_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_log_callback_fct_##A == (uintptr_t)fct) return my_log_callback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_log_callback_fct_##A == 0) {my_log_callback_fct_##A = (uintptr_t)fct; return my_log_callback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libavutil56 log_callback callback\n");
    return NULL;
}

#undef SUPER

EXPORT void my_av_log_set_callback(x64emu_t* emu, void* f)
{
    my->av_log_set_callback(find_log_callback_Fct(f));
}

#include "wrappedlib_init.h"
