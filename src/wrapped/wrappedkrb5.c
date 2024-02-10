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

#ifdef ANDROID
    const char* krb5Name = "libkrb5.so";
#else
    const char* krb5Name = "libkrb5.so.3";
#endif

#define LIBNAME krb5

#define ADDED_FUNCTIONS()           \

#include "generated/wrappedkrb5types.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// krb5_prompter ...
#define GO(A)   \
static uintptr_t my_krb5_prompter_fct_##A = 0;                                                  \
static int my_krb5_prompter_##A(void* a, void* b, void* c, void* d, int e, void* f)             \
{                                                                                               \
    return RunFunctionFmt(my_krb5_prompter_fct_##A, "ppppip", a, b, c, d, e, f);          \
}
SUPER()
#undef GO
static void* find_krb5_prompter_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_krb5_prompter_fct_##A == (uintptr_t)fct) return my_krb5_prompter_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_krb5_prompter_fct_##A == 0) {my_krb5_prompter_fct_##A = (uintptr_t)fct; return my_krb5_prompter_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libkrb5 krb5_prompter callback\n");
    return NULL;
}
#undef SUPER

EXPORT int my_krb5_get_init_creds_password(x64emu_t* emu, void* context, void* creds, void* client, void* password, void* f, void* data, int delta, void* service, void* options)
{
    return my->krb5_get_init_creds_password(context, creds, client, password, find_krb5_prompter_Fct(f), data, delta, service, options);
}

#include "wrappedlib_init.h"
