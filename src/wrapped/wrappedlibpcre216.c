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

const char* libpcre216Name = "libpcre2-16.so.0";
#define LIBNAME libpcre216

#define ADDED_FUNCTIONS()           \

#include "generated/wrappedlibpcre216types.h"

#include "wrappercallback.h"

// utility functions
#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// pcre2_jit_callback
#define GO(A)   \
static uintptr_t my_pcre2_jit_callback_fct_##A = 0;                         \
static void* my_pcre2_jit_callback_##A(void* a)                             \
{                                                                           \
    return (void*)RunFunctionFmt(my_pcre2_jit_callback_fct_##A, "p", a);    \
}
SUPER()
#undef GO
static void* find_pcre2_jit_callback_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_pcre2_jit_callback_fct_##A == (uintptr_t)fct) return my_pcre2_jit_callback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_pcre2_jit_callback_fct_##A == 0) {my_pcre2_jit_callback_fct_##A = (uintptr_t)fct; return my_pcre2_jit_callback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcre216 pcre2_jit_callback callback\n");
    return NULL;
}
// callback_enumerate
#define GO(A)   \
static uintptr_t my_callback_enumerate_fct_##A = 0;                         \
static int my_callback_enumerate_##A(void* a, void* b)                      \
{                                                                           \
    return (int)RunFunctionFmt(my_callback_enumerate_fct_##A, "pp", a, b);  \
}
SUPER()
#undef GO
static void* find_callback_enumerate_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_callback_enumerate_fct_##A == (uintptr_t)fct) return my_callback_enumerate_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_callback_enumerate_fct_##A == 0) {my_callback_enumerate_fct_##A = (uintptr_t)fct; return my_callback_enumerate_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcre216 callback_enumerate callback\n");
    return NULL;
}
// private_malloc
#define GO(A)   \
static uintptr_t my_private_malloc_fct_##A = 0;                             \
static void* my_private_malloc_##A(size_t a, void* b)                       \
{                                                                           \
    return (void*)RunFunctionFmt(my_private_malloc_fct_##A, "Lp", a, b);    \
}
SUPER()
#undef GO
static void* find_private_malloc_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_private_malloc_fct_##A == (uintptr_t)fct) return my_private_malloc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_private_malloc_fct_##A == 0) {my_private_malloc_fct_##A = (uintptr_t)fct; return my_private_malloc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcre216 private_malloc callback\n");
    return NULL;
}
// private_free
#define GO(A)   \
static uintptr_t my_private_free_fct_##A = 0;               \
static void my_private_free_##A(void* a, void* b)           \
{                                                           \
    RunFunctionFmt(my_private_free_fct_##A, "pp", a, b);    \
}
SUPER()
#undef GO
static void* find_private_free_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_private_free_fct_##A == (uintptr_t)fct) return my_private_free_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_private_free_fct_##A == 0) {my_private_free_fct_##A = (uintptr_t)fct; return my_private_free_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcre216 private_free callback\n");
    return NULL;
}
// callout_function
#define GO(A)   \
static uintptr_t my_callout_function_fct_##A = 0;                       \
static int my_callout_function_##A(void* a)                             \
{                                                                       \
    return (int)RunFunctionFmt(my_callout_function_fct_##A, "p", a);    \
}
SUPER()
#undef GO
static void* find_callout_function_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_callout_function_fct_##A == (uintptr_t)fct) return my_callout_function_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_callout_function_fct_##A == 0) {my_callout_function_fct_##A = (uintptr_t)fct; return my_callout_function_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcre216 callout_function callback\n");
    return NULL;
}
// guard_function
#define GO(A)   \
static uintptr_t my_guard_function_fct_##A = 0;                         \
static int my_guard_function_##A(uint32_t a, void* b)                   \
{                                                                       \
    return (int)RunFunctionFmt(my_guard_function_fct_##A, "up", a, b);  \
}
SUPER()
#undef GO
static void* find_guard_function_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_guard_function_fct_##A == (uintptr_t)fct) return my_guard_function_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_guard_function_fct_##A == 0) {my_guard_function_fct_##A = (uintptr_t)fct; return my_guard_function_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcre216 guard_function callback\n");
    return NULL;
}
// substitute
#define GO(A)   \
static uintptr_t my_substitute_fct_##A = 0;                                             \
static size_t my_substitute_##A(void* a, size_t b, void* c, size_t d, int e, void* f)   \
{                                                                                       \
    return (size_t)RunFunctionFmt(my_substitute_fct_##A, "pLpLip", a, b, c, d, e, f);   \
}
SUPER()
#undef GO
static void* find_substitute_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_substitute_fct_##A == (uintptr_t)fct) return my_substitute_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_substitute_fct_##A == 0) {my_substitute_fct_##A = (uintptr_t)fct; return my_substitute_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcre216 substitute callback\n");
    return NULL;
}

#undef SUPER

EXPORT void my_pcre2_jit_stack_assign_16(x64emu_t* emu, void* ctx, void* f, void* data)
{
    my->pcre2_jit_stack_assign_16(ctx, find_pcre2_jit_callback_Fct(f), data);
}

EXPORT int my_pcre2_callout_enumerate_16(x64emu_t* emu, void* code, void* f, void* data)
{
    return my->pcre2_callout_enumerate_16(code, find_callback_enumerate_Fct(f), data);
}

EXPORT void* my_pcre2_general_context_create_16(x64emu_t* emu, void* mc, void* fr, void* data)
{
    return my->pcre2_general_context_create_16(find_private_malloc_Fct(mc), find_private_free_Fct(fr), data);
}

EXPORT int my_pcre2_set_callout_16(x64emu_t* emu, void* ctx, void* f, void* data)
{
    return my->pcre2_set_callout_16(ctx, find_callout_function_Fct(f), data);
}

EXPORT int my_pcre2_set_compile_recursion_guard_16(x64emu_t* emu, void* ctx, void* f, void* data)
{
    return my->pcre2_set_compile_recursion_guard_16(ctx, find_guard_function_Fct(f), data);
}

EXPORT int my_pcre2_set_recursion_memory_management_16(x64emu_t* emu, void* ctx, void* mc, void* fr, void* data)
{
    return my->pcre2_set_recursion_memory_management_16(ctx, find_private_malloc_Fct(mc), find_private_free_Fct(fr), data);
}

EXPORT int my_pcre2_set_substitute_callout_16(x64emu_t* emu, void* ctx, void* f, void* data)
{
    return my->pcre2_set_substitute_callout_16(ctx, find_callback_enumerate_Fct(f), data);
}

EXPORT int my_pcre2_set_substitute_case_callout_16(x64emu_t* emu, void* ctx, void* f, void* data)
{
    return my->pcre2_set_substitute_case_callout_16(ctx, find_substitute_Fct(f), data);
}

#include "wrappedlib_init.h"

