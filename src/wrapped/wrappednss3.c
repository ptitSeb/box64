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

const char* nss3Name = "libnss3.so";
#define LIBNAME nss3
static library_t *my_lib = NULL;

#include "generated/wrappednss3types.h"

typedef struct nss3_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} nss3_my_t;

void* getNss3My(library_t* lib)
{
    my_lib = lib;
    nss3_my_t* my = (nss3_my_t*)calloc(1, sizeof(nss3_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}

void freeNss3My(void* lib)
{
    //nss3_my_t *my = (nss3_my_t *)lib;
}

#undef SUPER

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// PK11PasswordFunc ...
#define GO(A)   \
static uintptr_t my_PK11PasswordFunc_fct_##A = 0;                                   \
static void* my_PK11PasswordFunc_##A(void* a, int b, void* c)                       \
{                                                                                   \
    return (void*)RunFunction(my_context, my_PK11PasswordFunc_fct_##A, 3, a, b, c); \
}
SUPER()
#undef GO
static void* find_PK11PasswordFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_PK11PasswordFunc_fct_##A == (uintptr_t)fct) return my_PK11PasswordFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_PK11PasswordFunc_fct_##A == 0) {my_PK11PasswordFunc_fct_##A = (uintptr_t)fct; return my_PK11PasswordFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for nss3 PK11PasswordFunc callback\n");
    return NULL;
}

#undef SUPER

EXPORT void my_PK11_SetPasswordFunc(x64emu_t* emu, void* f)
{
    nss3_my_t* my = (nss3_my_t*)my_lib->priv.w.p2;

    my->PK11_SetPasswordFunc(find_PK11PasswordFunc_Fct(f));
}

#define CUSTOM_INIT \
    lib->priv.w.p2 = getNss3My(lib);

#define CUSTOM_FINI \
    freeNss3My(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"

