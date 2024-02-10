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

const char* libcupsName = 
#ifdef ANDROID
    "libcups.so"
#else
    "libcups.so.2"
#endif
    ;

#define LIBNAME libcups

#define ADDED_FUNCTIONS()                   \

#include "generated/wrappedlibcupstypes.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// cups_dest_cb_t ...
#define GO(A)   \
static uintptr_t my_cups_dest_cb_t_fct_##A = 0;                                         \
static int my_cups_dest_cb_t_##A(void* a, uint32_t b, void* c)                          \
{                                                                                       \
    return (int)RunFunctionFmt(my_cups_dest_cb_t_fct_##A, "pup", a, b, c);        \
}
SUPER()
#undef GO
static void* find_cups_dest_cb_t_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_cups_dest_cb_t_fct_##A == (uintptr_t)fct) return my_cups_dest_cb_t_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_cups_dest_cb_t_fct_##A == 0) {my_cups_dest_cb_t_fct_##A = (uintptr_t)fct; return my_cups_dest_cb_t_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcups cups_dest_cb_t callback\n");
    return NULL;
}

#undef SUPER

EXPORT int my_cupsEnumDests(x64emu_t* emu, uint32_t flags, int msec, int* cancel, uint32_t type, uint32_t mask, void* f, void* data)
{
    return my->cupsEnumDests(flags, msec, cancel, type, mask, find_cups_dest_cb_t_Fct(f), data);
}

#include "wrappedlib_init.h"
