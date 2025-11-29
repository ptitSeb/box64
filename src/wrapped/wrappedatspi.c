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

const char* atspiName = "libatspi.so.0";
#define LIBNAME atspi

#define ADDED_FUNCTIONS()                   \

#include "generated/wrappedatspitypes.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// GDestroyNotify ...
#define GO(A)   \
static uintptr_t my_GDestroyNotify_fct_##A = 0;                 \
static void my_GDestroyNotify_##A(void* a)                      \
{                                                               \
    RunFunctionFmt(my_GDestroyNotify_fct_##A, "p", a);    \
}
SUPER()
#undef GO
static void* find_GDestroyNotify_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GDestroyNotify_fct_##A == (uintptr_t)fct) return my_GDestroyNotify_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GDestroyNotify_fct_##A == 0) {my_GDestroyNotify_fct_##A = (uintptr_t)fct; return my_GDestroyNotify_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for atspi GDestroyNotify callback\n");
    return NULL;
}
// AtspiEventListenerCB ...
#define GO(A)   \
static uintptr_t my_AtspiEventListenerCB_fct_##A = 0;                   \
static void my_AtspiEventListenerCB_##A(void* a, void* b)               \
{                                                                       \
    RunFunctionFmt(my_AtspiEventListenerCB_fct_##A, "pp", a, b);  \
}
SUPER()
#undef GO
static void* find_AtspiEventListenerCB_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_AtspiEventListenerCB_fct_##A == (uintptr_t)fct) return my_AtspiEventListenerCB_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_AtspiEventListenerCB_fct_##A == 0) {my_AtspiEventListenerCB_fct_##A = (uintptr_t)fct; return my_AtspiEventListenerCB_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for atspi AtspiEventListenerCB callback\n");
    return NULL;
}

#undef SUPER

EXPORT void* my_atspi_event_listener_new(x64emu_t* emu, void* f, void* data, void* d)
{
    return my->atspi_event_listener_new(find_AtspiEventListenerCB_Fct(f), data, find_GDestroyNotify_Fct(d));
}

#define PRE_INIT \
    if (BOX64ENV(nogtk)) return -2;

#include "wrappedlib_init.h"
