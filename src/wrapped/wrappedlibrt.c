#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <signal.h>

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

#undef aio_suspend
#undef aio_return
#undef aio_write
#undef aio_read
#undef aio_error

#undef clock_gettime

const char* librtName = "librt.so.1";
#define LIBNAME librt

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)

// sigev_notify
#define GO(A)   \
static uintptr_t my_sigev_notify_fct_##A = 0;   \
static void my_sigev_notify_##A(void* sigval)    \
{                                       \
    RunFunctionFmt(my_context, my_sigev_notify_fct_##A, "p", sigval);\
}
SUPER()
#undef GO
static void* findsigev_notifyFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_sigev_notify_fct_##A == (uintptr_t)fct) return my_sigev_notify_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_sigev_notify_fct_##A == 0) {my_sigev_notify_fct_##A = (uintptr_t)fct; return my_sigev_notify_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libpng12 sigev_notify callback\n");
    return NULL;
}

#undef SUPER

EXPORT int my_timer_create(x64emu_t* emu, uint32_t clockid, void* sevp, timer_t* timerid)
{
    (void)emu;
    struct sigevent sevent;
    memcpy(&sevent, sevp, sizeof(sevent));

    if(sevent.sigev_notify == SIGEV_THREAD) {
        sevent.sigev_notify_function = findsigev_notifyFct(sevent.sigev_notify_function);
    }

    return timer_create(clockid, &sevent, timerid);
}

#include "wrappedlib_init.h"
