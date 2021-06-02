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

static library_t* my_lib = NULL;

typedef int    (*iFpip_t)       (void*, int, void*);

#define SUPER() \
    GO(tputs, iFpip_t)       \

typedef struct libtinfo6_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} libtinfo6_my_t;

void* getTinfo6My(library_t* lib)
{
    libtinfo6_my_t* my = (libtinfo6_my_t*)calloc(1, sizeof(libtinfo6_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeTinfo6My(void* lib)
{
    //libtinfo6_my_t *my = (libtinfo6_my_t *)lib;
}

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
    return (int)RunFunction(my_context, my_putc_fct_##A, 1, c); \
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
    libtinfo6_my_t* my = (libtinfo6_my_t*)my_lib->priv.w.p2;

    return my->tputs(str, affcnt, find_putc_Fct(f));
}

#define CUSTOM_INIT \
    my_lib = lib;   \
    lib->altmy = strdup("my6_"); \
    lib->priv.w.p2 = getTinfo6My(lib);

#define CUSTOM_FINI \
    my_lib = NULL;              \
    freeTinfo6My(lib->priv.w.p2);  \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"
