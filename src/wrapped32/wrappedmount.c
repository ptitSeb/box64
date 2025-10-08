#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper32.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "callback.h"
#include "librarian.h"
#include "box32context.h"
#include "emu/x64emu_private.h"
#include "converter32.h"

static const char* mountName = "libmount.so.1";
#define LIBNAME mount

#include "generated/wrappedmounttypes32.h"

#include "wrappercallback32.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// cmp ...
#define GO(A)   \
static uintptr_t my32_cmp_fct_##A = 0;                              \
static int my32_cmp_##A(void* a, void* b, void* c)                  \
{                                                                   \
    return (int)RunFunctionFmt(my32_cmp_fct_##A, "ppp", a, b, c);   \
}
SUPER()
#undef GO
static void* find_cmp_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my32_cmp_fct_##A == (uintptr_t)fct) return my32_cmp_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_cmp_fct_##A == 0) {my32_cmp_fct_##A = (uintptr_t)fct; return my32_cmp_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libmount cmp callback\n");
    return NULL;
}

#undef SUPER

typedef struct my_libmnt_optmap_s
{
        /*const*/ char *name;
        int             id;
        int             mask;
} my_libmnt_optmap_t;
typedef struct my_libmnt_optmap_32_s
{
        ptr_t           name;   //const char      *
        int             id;
        int             mask;
} my_libmnt_optmap_32_t;

void convert_libmnt_optmap_32(void* d, void* s)
{
    if(!s || !d) return;
    my_libmnt_optmap_t* src = s;
    my_libmnt_optmap_32_t* dst = d;
    dst->name = to_ptrv(src->name);
    dst->id = src->id;
    dst->mask = src->mask;
}
void convert_libmnt_optmap_64(void* d, void* s)
{
    if(!s || !d) return;
    my_libmnt_optmap_32_t* src = s;
    my_libmnt_optmap_t* dst = d;
    dst->mask = src->mask;
    dst->id = src->id;
    dst->name = from_ptrv(src->name);
}

#define N 4
static my_libmnt_optmap_t* optmap_64[N] = {0};
static my_libmnt_optmap_32_t optmap_32[N] = {0};
my_libmnt_optmap_32_t* shrink_libmnt_optmap(void* a)
{
    if(!a) return NULL;
    for(int i; i<N; ++i) {
        if(optmap_64[i]==a)
            return &optmap_32[i];
    }
    // look for free slot
    for(int i; i<N; ++i)
        if(!optmap_64[i]) {
            optmap_64[i] = a;
            convert_libmnt_optmap_32(&optmap_32[i], a);
            return &optmap_32[i];
        }
    // no more slot... use a local static and print a warning
    static int warned = 0;
    if(!warned) {
        printf_log(LOG_INFO, "Warning: no more slot for libmnt_optmap\n");
        warned = 1;
    }
    static my_libmnt_optmap_32_t ret = {0};
    convert_libmnt_optmap_32(&ret, a);
    return &ret;
}
my_libmnt_optmap_t* enlarge_libmnt_optmap(void* a)
{
    if(!a) return NULL;
    for(int i; i<N; ++i) {
        if(&optmap_32[i]==a)
            return optmap_64[i];
    }
    static int warned = 0;
    if(!warned) {
        printf_log(LOG_INFO, "Warning: 32bit version of libmnt_optmap not found\n");
        warned = 1;
    }
    static my_libmnt_optmap_t ret = {0};
    convert_libmnt_optmap_64(&ret, a);
    return &ret;
}

EXPORT void* my32_mnt_get_builtin_optmap(x64emu_t* emu, int a)
{
    return shrink_libmnt_optmap(my->mnt_get_builtin_optmap(a));
}

EXPORT int my32_mnt_optstr_get_flags(x64emu_t* emu, void* optstr, void* flag, void* map)
{
    return my->mnt_optstr_get_flags(optstr, flag, enlarge_libmnt_optmap(map));
}

EXPORT int my32_mnt_table_uniq_fs(void* tb, int flags, void* f)
{
    return my->mnt_table_uniq_fs(tb, flags, find_cmp_Fct(f));
}

#include "wrappedlib_init32.h"

