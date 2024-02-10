#define _GNU_SOURCE
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

const char* mpg123Name = 
#ifdef ANDROID
    "libmpg123.so"
#else
    "libmpg123.so.0"
#endif
    ;
#define LIBNAME mpg123

#define ADDED_FUNCTIONS()           \

#include "generated/wrappedmpg123types.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// r_read ...
#define GO(A)   \
static uintptr_t my_r_read_fct_##A = 0;                                 \
static ssize_t my_r_read_##A(void* a, void* b, size_t n)             \
{                                                                                       \
    return (ssize_t)RunFunctionFmt(my_r_read_fct_##A, "ppL", a, b, n);        \
}
SUPER()
#undef GO
static void* find_r_read_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_r_read_fct_##A == (uintptr_t)fct) return my_r_read_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_r_read_fct_##A == 0) {my_r_read_fct_##A = (uintptr_t)fct; return my_r_read_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for mpg123 r_read callback\n");
    return NULL;
}
// r_lseek ...
#define GO(A)   \
static uintptr_t my_r_lseek_fct_##A = 0;                                 \
static int64_t my_r_lseek_##A(void* a, int64_t b, int n)             \
{                                                                                       \
    return (int64_t)RunFunctionFmt(my_r_lseek_fct_##A, "pIi", a, b, n);       \
}
SUPER()
#undef GO
static void* find_r_lseek_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_r_lseek_fct_##A == (uintptr_t)fct) return my_r_lseek_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_r_lseek_fct_##A == 0) {my_r_lseek_fct_##A = (uintptr_t)fct; return my_r_lseek_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for mpg123 r_lseek callback\n");
    return NULL;
}
// cleanup ...
#define GO(A)   \
static uintptr_t my_cleanup_fct_##A = 0;                \
static void my_cleanup_##A(void* a)                     \
{                                                       \
    RunFunctionFmt(my_cleanup_fct_##A, "p", a);   \
}
SUPER()
#undef GO
static void* find_cleanup_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_cleanup_fct_##A == (uintptr_t)fct) return my_cleanup_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_cleanup_fct_##A == 0) {my_cleanup_fct_##A = (uintptr_t)fct; return my_cleanup_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for mpg123 cleanup callback\n");
    return NULL;
}

#undef SUPER

EXPORT int my_mpg123_replace_reader_handle(x64emu_t* emu, void* mh, void* r_read, void* r_lseek, void* cleanup)
{
    return my->mpg123_replace_reader_handle(mh, find_r_read_Fct(r_read), find_r_lseek_Fct(r_lseek), find_cleanup_Fct(cleanup));
}

EXPORT int my_mpg123_replace_reader_handle_64(x64emu_t* emu, void* mh, void* r_read, void* r_lseek, void* cleanup)
{
    return my->mpg123_replace_reader_handle_64(mh, find_r_read_Fct(r_read), find_r_lseek_Fct(r_lseek), find_cleanup_Fct(cleanup));
}

#include "wrappedlib_init.h"
