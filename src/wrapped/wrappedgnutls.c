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

const char* gnutlsName = "libgnutls.so.30";
#define LIBNAME gnutls

static library_t *my_lib = NULL;

typedef void        (*vFp_t)        (void*);
typedef void        (*vFpp_t)       (void*, void*);

#define SUPER() \
    GO(gnutls_global_set_log_function, vFp_t)       \
    GO(gnutls_transport_set_pull_function, vFpp_t)  \
    GO(gnutls_transport_set_push_function, vFpp_t)  \

typedef struct gnutls_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} gnutls_my_t;

void* getGnutlsMy(library_t* lib)
{
    gnutls_my_t* my = (gnutls_my_t*)calloc(1, sizeof(gnutls_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeGnutlsMy(void* lib)
{
    //gnutls_my_t *my = (gnutls_my_t *)lib;
}

// utility functions
#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// gnutls_log
#define GO(A)   \
static uintptr_t my_gnutls_log_fct_##A = 0;                       \
static void my_gnutls_log_##A(int level, const char* p)           \
{                                                                 \
    RunFunction(my_context, my_gnutls_log_fct_##A, 2, level, p);  \
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
static uintptr_t my_pullpush_fct_##A = 0;                                   \
static long my_pullpush_##A(void* p, void* d, size_t l)                     \
{                                                                           \
    return (long)RunFunction(my_context, my_pullpush_fct_##A, 3, p, d, l);  \
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

#undef SUPER


EXPORT void my_gnutls_global_set_log_function(x64emu_t* emu, void* f)
{
    gnutls_my_t *my = (gnutls_my_t*)my_lib->priv.w.p2;

    my->gnutls_global_set_log_function(find_gnutls_log_Fct(f));
}

EXPORT void my_gnutls_transport_set_pull_function(x64emu_t* emu, void* session, void* f)
{
    gnutls_my_t *my = (gnutls_my_t*)my_lib->priv.w.p2;

    my->gnutls_transport_set_pull_function(session, find_pullpush_Fct(f));
}
EXPORT void my_gnutls_transport_set_push_function(x64emu_t* emu, void* session, void* f)
{
    gnutls_my_t *my = (gnutls_my_t*)my_lib->priv.w.p2;

    my->gnutls_transport_set_push_function(session, find_pullpush_Fct(f));
}

#define CUSTOM_INIT \
    my_lib = lib;   \
    lib->priv.w.p2 = getGnutlsMy(lib);

#define CUSTOM_FINI \
    my_lib = NULL;  \
    freeGnutlsMy(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"

