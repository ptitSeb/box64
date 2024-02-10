#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

const char* libtinfo6Name = "libtinfo.so.6";
#define LIBNAME libtinfo6

#include "generated/wrappedlibtinfo6types.h"

#include "wrappercallback.h"

// utility functions
#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// putc
#define GO(A)   \
static uintptr_t my_putc_fct_##A = 0;                           \
static int my_putc_##A(char c)                                  \
{                                                               \
    return (int)RunFunctionFmt(my_putc_fct_##A, "c", c);  \
}
SUPER()
#undef GO
static void* find_putc_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_putc_fct_##A == (uintptr_t)fct) return my_putc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_putc_fct_##A == 0) {my_putc_fct_##A = (uintptr_t)fct; return my_putc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libSSL putc callback\n");
    return NULL;
}

#undef SUPER

EXPORT int my6_tputs(x64emu_t* emu, void* str, int affcnt, void* f)
{
    return my->tputs(str, affcnt, find_putc_Fct(f));
}

#define ALTMY my6_

#include "wrappedlib_init.h"
