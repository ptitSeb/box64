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
#include "box64context.h"
#include "librarian.h"
#include "callback.h"

const char* ldaprName =
#ifdef ANDROID
    "libldap_r-2.4.so"
#else
    "libldap_r-2.4.so.2"
#endif
    ;
#define LIBNAME ldapr
static library_t *my_lib = NULL;

typedef int     (*iFpppppupp_t)      (void*, void*, void*, void* , void*, unsigned, void*, void*);

#define SUPER()                                     \
    GO(ldap_sasl_interactive_bind_s, iFpppppupp_t)  \

typedef struct ldapr_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} ldapr_my_t;

void* getLdaprMy(library_t* lib)
{
    ldapr_my_t* my = (ldapr_my_t*)calloc(1, sizeof(ldapr_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeLdaprMy(void* lib)
{
    //ldapr_my_t *my = (ldapr_my_t *)lib;
}

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// LDAP_SASL_INTERACT_PROC ...
#define GO(A)   \
static uintptr_t my_LDAP_SASL_INTERACT_PROC_fct_##A = 0;                                \
static int my_LDAP_SASL_INTERACT_PROC_##A(void* a, unsigned b, void* c, void* d)        \
{                                                                                       \
    return RunFunction(my_context, my_LDAP_SASL_INTERACT_PROC_fct_##A, 4, a, b, c, d);  \
}
SUPER()
#undef GO
static void* find_LDAP_SASL_INTERACT_PROC_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_LDAP_SASL_INTERACT_PROC_fct_##A == (uintptr_t)fct) return my_LDAP_SASL_INTERACT_PROC_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_LDAP_SASL_INTERACT_PROC_fct_##A == 0) {my_LDAP_SASL_INTERACT_PROC_fct_##A = (uintptr_t)fct; return my_LDAP_SASL_INTERACT_PROC_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libldap_r LDAP_SASL_INTERACT_PROC callback\n");
    return NULL;
}
#undef SUPER

EXPORT int my_ldap_sasl_interactive_bind_s(x64emu_t* emu, void* ld, void* dn, void* mechs, void* sctrls, void* cctrls, unsigned flags, void* f, void* defaults)
{
    ldapr_my_t* my = (ldapr_my_t*)my_lib->priv.w.p2;

    return my->ldap_sasl_interactive_bind_s(ld, dn, mechs, sctrls, cctrls, flags, find_LDAP_SASL_INTERACT_PROC_Fct(f), defaults);
}

#ifdef ANDROID
#define NEEDED_LIB "liblber-2.4.so"
#else
#define NEEDED_LIB "liblber-2.4.so.2"
#endif

#define CUSTOM_INIT \
    lib->priv.w.p2 = getLdaprMy(lib);                                           \
    lib->priv.w.needed = 1;                                                     \
    lib->priv.w.neededlibs = (char**)calloc(lib->priv.w.needed, sizeof(char*)); \
    lib->priv.w.neededlibs[0] = strdup(NEEDED_LIB);                             \
    my_lib = lib;

#define CUSTOM_FINI \
    freeLdaprMy(lib->priv.w.p2); \
    free(lib->priv.w.p2);       \
    my_lib = NULL;

#include "wrappedlib_init.h"

