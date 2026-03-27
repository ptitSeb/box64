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

// Fake the lib for now, don't load it
const char* smbclientName = "libsmbclient.so.0";
#define LIBNAME smbclient

#include "generated/wrappedsmbclienttypes.h"

#include "wrappercallback.h"

#undef SUPER

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// smbc_get_auth_data_fn ...
#define GO(A)   \
static uintptr_t my_smbc_get_auth_data_fn_fct_##A = 0;                                                      \
static void my_smbc_get_auth_data_fn_##A(void* a, void* b, void* c, int d, void* e, int f, void* g, int h)  \
{                                                                                                           \
    RunFunctionFmt(my_smbc_get_auth_data_fn_fct_##A, "pppipipi", a, b, c, d, e, f, g, h);                   \
}
SUPER()
#undef GO
static void* find_smbc_get_auth_data_fn_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_smbc_get_auth_data_fn_fct_##A == (uintptr_t)fct) return my_smbc_get_auth_data_fn_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_smbc_get_auth_data_fn_fct_##A == 0) {my_smbc_get_auth_data_fn_fct_##A = (uintptr_t)fct; return my_smbc_get_auth_data_fn_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for smbclien smbc_get_auth_data_fn callback\n");
    return NULL;
}
// smbc_get_auth_data_with_context_fn ...
#define GO(A)   \
static uintptr_t my_smbc_get_auth_data_with_context_fn_fct_##A = 0;                                                                 \
static void my_smbc_get_auth_data_with_context_fn_##A(void* a, void* b, void* c, void * d, int e, void* f, int g, void* h, int i)   \
{                                                                                                                                   \
    RunFunctionFmt(my_smbc_get_auth_data_with_context_fn_fct_##A, "ppppipipi", a, b, c, d, e, f, g, h, i);                          \
}
SUPER()
#undef GO
static void* find_smbc_get_auth_data_with_context_fn_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_smbc_get_auth_data_with_context_fn_fct_##A == (uintptr_t)fct) return my_smbc_get_auth_data_with_context_fn_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_smbc_get_auth_data_with_context_fn_fct_##A == 0) {my_smbc_get_auth_data_with_context_fn_fct_##A = (uintptr_t)fct; return my_smbc_get_auth_data_with_context_fn_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for smbclien smbc_get_auth_data_with_context_fn callback\n");
    return NULL;
}

#undef SUPER

EXPORT int my_smbc_init(x64emu_t* emu, void* f, int debug)
{
    return my->smbc_init(find_smbc_get_auth_data_fn_Fct(f), debug);
}

EXPORT void my_smbc_setFunctionAuthDataWithContext(x64emu_t* emu, void* ctx, void* f)
{
    my->smbc_setFunctionAuthDataWithContext(ctx, find_smbc_get_auth_data_with_context_fn_Fct(f));
}

#include "wrappedlib_init.h"
