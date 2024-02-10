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

const char* gmpName = "libgmp.so.10";
#define LIBNAME gmp

#include "generated/wrappedgmptypes.h"

#include "wrappercallback.h"

// utility functions
#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// alloc_func
#define GO(A)   \
static uintptr_t my_alloc_func_fct_##A = 0;                         \
static void* my_alloc_func_##A(size_t a)                            \
{                                                                   \
    return (void*)RunFunctionFmt(my_alloc_func_fct_##A, "L", a);    \
}
SUPER()
#undef GO
static void* find_alloc_func_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_alloc_func_fct_##A == (uintptr_t)fct) return my_alloc_func_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_alloc_func_fct_##A == 0) {my_alloc_func_fct_##A = (uintptr_t)fct; return my_alloc_func_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libgmp.so.10 alloc_func callback\n");
    return NULL;
}
static void* reverse_alloc_func_Fct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_alloc_func_##A == fct) return (void*)my_alloc_func_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, pFL, fct, 0, NULL);
}
// realloc_func
#define GO(A)   \
static uintptr_t my_realloc_func_fct_##A = 0;                           \
static void* my_realloc_func_##A(void* a, size_t b)                     \
{                                                                       \
    return (void*)RunFunctionFmt(my_realloc_func_fct_##A, "pL", a, b);  \
}
SUPER()
#undef GO
static void* find_realloc_func_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_realloc_func_fct_##A == (uintptr_t)fct) return my_realloc_func_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_realloc_func_fct_##A == 0) {my_realloc_func_fct_##A = (uintptr_t)fct; return my_realloc_func_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libgmp.so.10 realloc_func callback\n");
    return NULL;
}
static void* reverse_realloc_func_Fct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_realloc_func_##A == fct) return (void*)my_realloc_func_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, pFpL, fct, 0, NULL);
}
// free_func
#define GO(A)   \
static uintptr_t my_free_func_fct_##A = 0;          \
static void my_free_func_##A(void* a)               \
{                                                   \
    RunFunctionFmt(my_free_func_fct_##A, "p", a);   \
}
SUPER()
#undef GO
static void* find_free_func_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_free_func_fct_##A == (uintptr_t)fct) return my_free_func_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_free_func_fct_##A == 0) {my_free_func_fct_##A = (uintptr_t)fct; return my_free_func_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libgmp.so.10 free_func callback\n");
    return NULL;
}
static void* reverse_free_func_Fct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_free_func_##A == fct) return (void*)my_free_func_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, vFp, fct, 0, NULL);
}

#undef SUPER

EXPORT void my___gmp_get_memory_functions(x64emu_t* emu, void** f_alloc, void** f_realloc, void** f_free)
{
    my->__gmp_get_memory_functions(f_alloc, f_realloc, f_free);
    *f_alloc = reverse_alloc_func_Fct(*f_alloc);
    *f_realloc = reverse_realloc_func_Fct(*f_realloc);
    *f_free = reverse_free_func_Fct(*f_free);
}
EXPORT void my___gmp_set_memory_functions(x64emu_t* emu, void* f_alloc, void* f_realloc, void* f_free)
{
    my->__gmp_set_memory_functions(find_alloc_func_Fct(f_alloc), find_realloc_func_Fct(f_realloc), find_free_func_Fct(f_free));
}

#include "wrappedlib_init.h"
