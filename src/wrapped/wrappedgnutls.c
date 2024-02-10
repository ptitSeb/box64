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

const char* gnutlsName = 
#if ANDROID
    "libgnutls.so"
#else
    "libgnutls.so.30"
#endif
    ;
#define LIBNAME gnutls

#include "generated/wrappedgnutlstypes.h"

#include "wrappercallback.h"

// utility functions
#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// gnutls_log
#define GO(A)   \
static uintptr_t my_gnutls_log_fct_##A = 0;                            \
static void my_gnutls_log_##A(int level, const char* p)                \
{                                                                      \
    RunFunctionFmt(my_gnutls_log_fct_##A, "ip", level, p);        \
}
SUPER()
#undef GO
static void* find_gnutls_log_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_gnutls_log_fct_##A == (uintptr_t)fct) return my_gnutls_log_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_gnutls_log_fct_##A == 0) {my_gnutls_log_fct_##A = (uintptr_t)fct; return my_gnutls_log_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libgnutls.so.30 gnutls_log callback\n");
    return NULL;
}

// pullpush
#define GO(A)   \
static uintptr_t my_pullpush_fct_##A = 0;                                         \
static long my_pullpush_##A(void* p, void* d, size_t l)                           \
{                                                                                 \
    return (long)RunFunctionFmt(my_pullpush_fct_##A, "ppL", p, d, l);         \
}
SUPER()
#undef GO
static void* find_pullpush_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_pullpush_fct_##A == (uintptr_t)fct) return my_pullpush_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_pullpush_fct_##A == 0) {my_pullpush_fct_##A = (uintptr_t)fct; return my_pullpush_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libgnutls.so.30 pullpush callback\n");
    return NULL;
}

// timeout
#define GO(A)   \
static uintptr_t my_timeout_fct_##A = 0;                                    \
static int my_timeout_##A(void* p, uint32_t t)                              \
{                                                                           \
    return (int)RunFunctionFmt(my_timeout_fct_##A, "pu", p, t);       \
}
SUPER()
#undef GO
static void* find_timeout_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_timeout_fct_##A == (uintptr_t)fct) return my_timeout_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_timeout_fct_##A == 0) {my_timeout_fct_##A = (uintptr_t)fct; return my_timeout_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libgnutls.so.30 timeout callback\n");
    return NULL;
}

#undef SUPER


EXPORT void my_gnutls_global_set_log_function(x64emu_t* emu, void* f)
{
    (void)emu;
    my->gnutls_global_set_log_function(find_gnutls_log_Fct(f));
}

EXPORT void my_gnutls_transport_set_pull_function(x64emu_t* emu, void* session, void* f)
{
    (void)emu;
    my->gnutls_transport_set_pull_function(session, find_pullpush_Fct(f));
}
EXPORT void my_gnutls_transport_set_push_function(x64emu_t* emu, void* session, void* f)
{
    (void)emu;
    my->gnutls_transport_set_push_function(session, find_pullpush_Fct(f));
}

EXPORT void my_gnutls_transport_set_pull_timeout_function(x64emu_t* emu, void* session, void* f)
{
    (void)emu;
    my->gnutls_transport_set_pull_timeout_function(session, find_timeout_Fct(f));
}

#include "wrappedlib_init.h"
