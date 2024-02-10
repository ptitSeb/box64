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
#define ALTNAME "libldap-2.5.so.0"
#define LIBNAME ldapr

#define ADDED_FUNCTIONS() \

#include "wrappedldaprtypes.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// LDAP_SASL_INTERACT_PROC ...
#define GO(A)   \
static uintptr_t my_LDAP_SASL_INTERACT_PROC_fct_##A = 0;                                        \
static int my_LDAP_SASL_INTERACT_PROC_##A(void* a, unsigned b, void* c, void* d)                \
{                                                                                               \
    return RunFunctionFmt(my_LDAP_SASL_INTERACT_PROC_fct_##A, "pupp", a, b, c, d);        \
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
    return my->ldap_sasl_interactive_bind_s(ld, dn, mechs, sctrls, cctrls, flags, find_LDAP_SASL_INTERACT_PROC_Fct(f), defaults);
}

#ifdef ANDROID
#define NEEDED_LIBS "liblber-2.4.so"
#else
#define NEEDED_LIBS "liblber-2.4.so.2"
#endif

#include "wrappedlib_init.h"
