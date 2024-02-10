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
#include "callback.h"

const char* event21Name = "libevent-2.1.so.7";
#define LIBNAME event21

#include "generated/wrappedevent21types.h"

#include "wrappercallback.h"

#define SUPER() \
    GO(0)   \
    GO(1)   \
    GO(2)   \
    GO(3)

// event_set
#define GO(A)   \
static uintptr_t my_event_set_fct_##A = 0;                  \
static void my_event_set_##A(int a, short b, void* c)       \
{                                                           \
    RunFunctionFmt(my_event_set_fct_##A, "iwp", a, b, c);   \
}
SUPER()
#undef GO
static void* findevent_setFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_event_set_fct_##A == (uintptr_t)fct) return my_event_set_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_event_set_fct_##A == 0) {my_event_set_fct_##A = (uintptr_t)fct; return my_event_set_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libevent-2.1 event_set callback\n");
    return NULL;
}

#undef SUPER

EXPORT void my_event_set(x64emu_t* emu, void* evt, int sock, short i, void* f, void* data)
{
    my->event_set(evt, sock, i, findevent_setFct(f), data);
}

#include "wrappedlib_init.h"
