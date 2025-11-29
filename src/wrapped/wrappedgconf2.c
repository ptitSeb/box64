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
#include "gtkclass.h"

const char* gconf2Name = "libgconf-2.so.4";
#define LIBNAME gconf2

#include "generated/wrappedgconf2types.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)

// GFreeFct
#define GO(A)   \
static uintptr_t my_GFreeFct_fct_##A = 0;                    \
static void my_GFreeFct_##A(void* a)                         \
{                                                            \
    RunFunctionFmt(my_GFreeFct_fct_##A, "p", a);      \
}
SUPER()
#undef GO
static void* findGFreeFctFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GFreeFct_fct_##A == (uintptr_t)fct) return my_GFreeFct_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GFreeFct_fct_##A == 0) {my_GFreeFct_fct_##A = (uintptr_t)fct; return my_GFreeFct_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gconf2 GFreeFct callback\n");
    return NULL;
}
// GConfClientNotifyFunc
#define GO(A)   \
static uintptr_t my_GConfClientNotifyFunc_fct_##A = 0;                                \
static void my_GConfClientNotifyFunc_##A(void* a, uint32_t b, void* c, void* d)       \
{                                                                                     \
    RunFunctionFmt(my_GConfClientNotifyFunc_fct_##A, "pupp", a, b, c, d);         \
}
SUPER()
#undef GO
static void* findGConfClientNotifyFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GConfClientNotifyFunc_fct_##A == (uintptr_t)fct) return my_GConfClientNotifyFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GConfClientNotifyFunc_fct_##A == 0) {my_GConfClientNotifyFunc_fct_##A = (uintptr_t)fct; return my_GConfClientNotifyFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gconf2 GConfClientNotifyFunc callback\n");
    return NULL;
}

#undef SUPER

EXPORT uint32_t my_gconf_client_notify_add(x64emu_t* emu, void* client, void* section, void* f, void* data, void* d, void* error)
{
    return my->gconf_client_notify_add(client, section, findGConfClientNotifyFuncFct(f), data, findGFreeFctFct(d), error);
}

#define PRE_INIT \
    if (BOX64ENV(nogtk)) return -2;

#include "wrappedlib_init.h"
