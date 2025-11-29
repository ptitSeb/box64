#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>
#include <stdarg.h>
#define __STDC_WANT_LIB_EXT2__ 1 // for vasprintf
#include <stdio.h>

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

const char* libavutil56Name = "libavutil.so.56";

#define LIBNAME libavutil56

#define ADDED_FUNCTIONS()                   \
    GO(av_malloc, pFL_t)                    \
    GO(av_mallocz, pFL_t)                   \
    GO(av_realloc, pFpL_t)                  \

#include "generated/wrappedlibavutil56types.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// log_callback ...
#define GO(A)   \
static uintptr_t my_log_callback_fct_##A = 0;                           \
static void my_log_callback_##A(void* a, int b, void* c, va_list d)     \
{                                                                       \
    x64_va_list_t null_va = {0};                                        \
    char* p = NULL;                                                     \
    (void)!vasprintf(&p, c, d);                                         \
    RunFunctionFmt(my_log_callback_fct_##A, "pipp", a, b, d, null_va);  \
    free(p);                                                            \
}
SUPER()
#undef GO
static void* find_log_callback_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_log_callback_fct_##A == (uintptr_t)fct) return my_log_callback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_log_callback_fct_##A == 0) {my_log_callback_fct_##A = (uintptr_t)fct; return my_log_callback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libavutil56 log_callback callback\n");
    return NULL;
}
// alloc ...
#define GO(A)   \
static uintptr_t my_alloc_fct_##A = 0;                      \
static void* my_alloc_##A(int a)                            \
{                                                           \
    return (void*)RunFunctionFmt(my_alloc_fct_##A, "i", a); \
}
SUPER()
#undef GO
static void* find_alloc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_alloc_fct_##A == (uintptr_t)fct) return my_alloc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_alloc_fct_##A == 0) {my_alloc_fct_##A = (uintptr_t)fct; return my_alloc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libavutil56 alloc callback\n");
    return NULL;
}
// func1 ...
#define GO(A)   \
static uintptr_t my_func1_fct_##A = 0;                      \
static double my_func1_##A(void* a, double b)               \
{                                                           \
    return RunFunctionFmtD(my_func1_fct_##A, "pd", a, b);   \
}
SUPER()
#undef GO
static void* find_func1_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_func1_fct_##A == (uintptr_t)fct) return my_func1_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_func1_fct_##A == 0) {my_func1_fct_##A = (uintptr_t)fct; return my_func1_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libavutil56 func1 callback\n");
    return NULL;
}
// func2 ...
#define GO(A)   \
static uintptr_t my_func2_fct_##A = 0;                          \
static double my_func2_##A(void* a, double b, double c)         \
{                                                               \
    return RunFunctionFmtD(my_func2_fct_##A, "pdd", a, b, c);   \
}
SUPER()
#undef GO
static void* find_func2_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_func2_fct_##A == (uintptr_t)fct) return my_func2_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_func2_fct_##A == 0) {my_func2_fct_##A = (uintptr_t)fct; return my_func2_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libavutil56 func2 callback\n");
    return NULL;
}
// worker ...
#define GO(A)   \
static uintptr_t my_worker_fct_##A = 0;                         \
static void my_worker_##A(void* a, int b, int c, int d, int e)  \
{                                                               \
    RunFunctionFmt(my_worker_fct_##A, "piiii", a, b, c, d, e);  \
}
SUPER()
#undef GO
static void* find_worker_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_worker_fct_##A == (uintptr_t)fct) return my_worker_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_worker_fct_##A == 0) {my_worker_fct_##A = (uintptr_t)fct; return my_worker_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libavutil56 worker callback\n");
    return NULL;
}
// main ...
#define GO(A)   \
static uintptr_t my_main_fct_##A = 0;           \
static void my_main_##A(void* a)                \
{                                               \
    RunFunctionFmt(my_main_fct_##A, "p", a);    \
}
SUPER()
#undef GO
static void* find_main_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_main_fct_##A == (uintptr_t)fct) return my_main_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_main_fct_##A == 0) {my_main_fct_##A = (uintptr_t)fct; return my_main_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libavutil56 main callback\n");
    return NULL;
}
// free ...
#define GO(A)   \
static uintptr_t my_free_fct_##A = 0;               \
static void my_free_##A(void* a, void* b)           \
{                                                   \
    RunFunctionFmt(my_free_fct_##A, "pp", a, b);    \
}
SUPER()
#undef GO
static void* find_free_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_free_fct_##A == (uintptr_t)fct) return my_free_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_free_fct_##A == 0) {my_free_fct_##A = (uintptr_t)fct; return my_free_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libavutil56 free callback\n");
    return NULL;
}

#undef SUPER

EXPORT void my_av_log_set_callback(x64emu_t* emu, void* f)
{
    my->av_log_set_callback(find_log_callback_Fct(f));
}

EXPORT void* my_av_asprintf(x64emu_t* emu, void * fmt, uint64_t * b) {
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 1);
    PREPARE_VALIST;
    char* buff = NULL;
    vasprintf(&buff, (char*)fmt, VARARGS);
    void* ret = my->av_asprintf("%s", buff);
    free(buff);
    return ret;
}

EXPORT void my_av_vbprintf(x64emu_t* emu, void* buff, void* fmt, x64_va_list_t b)
{
    (void)emu;
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(b);
    #else
    myStackAlignValist(emu, (const char*)fmt, emu->scratch, b);
    PREPARE_VALIST;
    #endif
    my->av_vbprintf(buff, fmt, VARARGS);
}
EXPORT void my_av_bprintf(x64emu_t* emu, void** buff, void * fmt, uint64_t * b) {
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 2);
    PREPARE_VALIST;
    my->av_vbprintf((char**)buff, (char*)fmt, VARARGS);
}

EXPORT void* my_av_buffer_pool_init(x64emu_t* emu, int size, void* alloc)
{
    return my->av_buffer_pool_init(size, find_alloc_Fct(alloc));
}

EXPORT int my_av_expr_parse(x64emu_t* emu, void* expr, void* s, void** const_names, void** func1_names, void** funcs1, void** func2_names, void** funcs2, int offset, void* log)
{
    int n_f1 = 0, n_f2 = 0;
    // find n of f1 and f2 first
    while(funcs1[n_f1]) ++n_f1;
    while(funcs2[n_f2]) ++n_f2;
    n_f1++; n_f2++; // include NULL marker
    void* funcs1_[n_f1];
    void* funcs2_[n_f2];
    for(int i=0; i<n_f1; ++i) funcs1_[i] = find_func1_Fct(funcs1[i]);
    for(int i=0; i<n_f2; ++i) funcs2_[i] = find_func2_Fct(funcs2[i]);
    return my->av_expr_parse(expr, s, const_names, func1_names, funcs1_, func2_names, funcs2_, offset, log);
}

EXPORT void my_av_log(x64emu_t* emu, void* avcl, int lvl, void* fmt, uint64_t* b)
{
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 3);
    PREPARE_VALIST;
    char* buff = NULL;
    vasprintf(&buff, (char*)fmt, VARARGS);
    my->av_log(avcl, lvl, "%s", buff);
    free(buff);
}

EXPORT void* my_av_malloc_tracked(x64emu_t* emu, size_t size)
{
    return my->av_malloc_tracked?my->av_malloc_tracked(size):my->av_malloc(size);
}

EXPORT void* my_av_mallocz_tracked(x64emu_t* emu, size_t size)
{
    return my->av_mallocz_tracked?my->av_mallocz_tracked(size):my->av_mallocz(size);
}

EXPORT void* my_av_realloc_tracked(x64emu_t* emu, void* p, size_t size)
{
    return my->av_realloc_tracked?my->av_realloc_tracked(p, size):my->av_realloc(p, size);
}

EXPORT size_t my_av_strlcatf(x64emu_t* emu, void* dst, size_t size, void* fmt, uint64_t* b)
{
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 3);
    PREPARE_VALIST;
    char* buff = NULL;
    vasprintf(&buff, (char*)fmt, VARARGS);
    size_t ret = my->av_strlcatf(dst, size, "%s", buff);
    free(buff);
    return ret;
}

EXPORT int my_avpriv_slicethread_create(x64emu_t* emu, void* pctx, void* priv, void* worker, void* main_func, int nb)
{
    return my->avpriv_slicethread_create(pctx, priv, find_worker_Fct(worker), find_main_Fct(main_func), nb);
}

EXPORT void* my_av_buffer_create(x64emu_t* emu, void* data, int size, void* f, void* opaque, int flags)
{
    return my->av_buffer_create(data, size, find_free_Fct(f), opaque, flags);
}

#define PRE_INIT \
    if (BOX64ENV(nogtk)) return -2;

#include "wrappedlib_init.h"
