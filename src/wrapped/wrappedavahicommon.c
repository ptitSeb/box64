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
#include "callback.h"
#include "librarian.h"
#include "box64context.h"
#include "emu/x64emu_private.h"
#include "myalign.h"

const char* avahicommonName = "libavahi-common.so.3";
#define LIBNAME avahicommon

typedef void* (*pFpi_t)(void*, int);

#define ADDED_FUNCTIONS()       \
    GO(avahi_string_list_new_from_array, pFpi_t)

#include "generated/wrappedavahicommontypes.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)

typedef struct my_AvahiAllocator_s {
    void* (*malloc)(size_t size);
    void (*free)(void *p);
    void* (*realloc)(void *p, size_t size);
    void* (*calloc)(size_t nmemb, size_t size);
} my_AvahiAllocator_t;

// malloc
#define GO(A)   \
static uintptr_t my_malloc_fct_##A = 0;                         \
static void* my_malloc_##A(size_t a)                            \
{                                                               \
    return (void*)RunFunctionFmt(my_malloc_fct_##A, "L", a);    \
}
SUPER()
#undef GO
static void* findmallocFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_malloc_fct_##A == (uintptr_t)fct) return my_malloc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_malloc_fct_##A == 0) {my_malloc_fct_##A = (uintptr_t)fct; return my_malloc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libavahi-common malloc callback\n");
    return NULL;
}
// free
#define GO(A)   \
static uintptr_t my_free_fct_##A = 0;           \
static void my_free_##A(void* a)                \
{                                               \
    RunFunctionFmt(my_free_fct_##A, "p", a);    \
}
SUPER()
#undef GO
static void* findfreeFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_free_fct_##A == (uintptr_t)fct) return my_free_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_free_fct_##A == 0) {my_free_fct_##A = (uintptr_t)fct; return my_free_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libavahi-common free callback\n");
    return NULL;
}
// realloc
#define GO(A)   \
static uintptr_t my_realloc_fct_##A = 0;                            \
static void* my_realloc_##A(void* a, size_t b)                      \
{                                                                   \
    return (void*)RunFunctionFmt(my_realloc_fct_##A, "pL", a, b);   \
}
SUPER()
#undef GO
static void* findreallocFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_realloc_fct_##A == (uintptr_t)fct) return my_realloc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_realloc_fct_##A == 0) {my_realloc_fct_##A = (uintptr_t)fct; return my_realloc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libavahi-common realloc callback\n");
    return NULL;
}
// calloc
#define GO(A)   \
static uintptr_t my_calloc_fct_##A = 0;                             \
static void* my_calloc_##A(size_t a, size_t b)                      \
{                                                                   \
    return (void*)RunFunctionFmt(my_calloc_fct_##A, "LL", a, b);    \
}
SUPER()
#undef GO
static void* findcallocFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_calloc_fct_##A == (uintptr_t)fct) return my_calloc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_calloc_fct_##A == 0) {my_calloc_fct_##A = (uintptr_t)fct; return my_calloc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libavahi-common calloc callback\n");
    return NULL;
}
// AvahiPollFunc
#define GO(A)   \
static uintptr_t my_AvahiPollFunc_fct_##A = 0;                                  \
static int my_AvahiPollFunc_##A(void* a, uint32_t b, int c, void* d)            \
{                                                                               \
    return (int)RunFunctionFmt(my_AvahiPollFunc_fct_##A, "puip", a, b, c, d);   \
}
SUPER()
#undef GO
static void* findAvahiPollFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_AvahiPollFunc_fct_##A == (uintptr_t)fct) return my_AvahiPollFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_AvahiPollFunc_fct_##A == 0) {my_AvahiPollFunc_fct_##A = (uintptr_t)fct; return my_AvahiPollFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libavahi-common AvahiPollFunc callback\n");
    return NULL;
}

#undef SUPER

EXPORT void my_avahi_set_allocator(x64emu_t* emu, my_AvahiAllocator_t* p)
{
    my_AvahiAllocator_t _p = {0};
    if(p) {
        _p.malloc = findmallocFct(p->malloc);
        _p.free = findfreeFct(p->free);
        _p.realloc = findreallocFct(p->realloc);
        _p.calloc = findcallocFct(p->calloc);
    }
    my->avahi_set_allocator(p?&_p:NULL);
}

EXPORT void my_avahi_simple_poll_set_func(x64emu_t* emu, void* s, void* f, void* d)
{
    my->avahi_simple_poll_set_func(s, findAvahiPollFuncFct(f), d);
}

EXPORT void* my_avahi_strdup_printf(x64emu_t* emu, void* fmt, uint64_t* b)
{
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 1);
    PREPARE_VALIST;
    return my->avahi_strdup_vprintf(fmt, VARARGS);
}
EXPORT void* my_avahi_strdup_vprintf(x64emu_t* emu, void* fmt, x64_va_list_t b)
{
    (void)emu;
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(b);
    #else
    myStackAlignValist(emu, (const char*)fmt, emu->scratch, b);
    PREPARE_VALIST;
    #endif
    return my->avahi_strdup_vprintf(fmt, VARARGS);
}

EXPORT void* my_avahi_string_list_add_many(x64emu_t* emu, void* list, uint64_t* b)
{
    CREATE_SYSV_VALIST(b);
    return my->avahi_string_list_add_many_va(list, VARARGS);
}
EXPORT void* my_avahi_string_list_add_many_va(x64emu_t* emu, void* list, x64_va_list_t b)
{
    #ifdef CONVERT_VALIST
    (void)emu;
    CONVERT_VALIST(b);
    #else
    CREATE_VALIST_FROM_VALIST(b, emu->scratch);
    #endif
    return my->avahi_string_list_add_many_va(list, VARARGS);
}

EXPORT void* my_avahi_string_list_add_printf(x64emu_t* emu, void* list, void* fmt, uint64_t* b)
{
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 2);
    PREPARE_VALIST;
    return my->avahi_string_list_add_vprintf(list, fmt, VARARGS);
}
EXPORT void* my_avahi_string_list_add_vprintf(x64emu_t* emu, void* list, void* fmt, x64_va_list_t b)
{
    (void)emu;
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(b);
    #else
    myStackAlignValist(emu, (const char*)fmt, emu->scratch, b);
    PREPARE_VALIST;
    #endif
    return my->avahi_string_list_add_vprintf(list, fmt, VARARGS);
}

EXPORT void* my_avahi_string_list_new(x64emu_t* emu, void* p, uintptr_t* b)
{
    int cnt = 0;
    while(getVArgs(emu, 1, b, cnt)) ++cnt;
    void* arr[cnt+1];
    arr[0] = p;
    for(int i=0; i<cnt; ++i)
        arr[i+1] = (void*)getVArgs(emu, 1, b, i);
    return my->avahi_string_list_new_from_array(arr, cnt+1);
}

EXPORT void* my_avahi_string_list_new_va(x64emu_t* emu, x64_va_list_t b)
{
    #ifdef CONVERT_VALIST
    (void)emu;
    CONVERT_VALIST(b);
    #else
    CREATE_VALIST_FROM_VALIST(b, emu->scratch);
    #endif
    return my->avahi_string_list_new_va(VARARGS);
}

#include "wrappedlib_init.h"
