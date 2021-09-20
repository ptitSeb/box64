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
#include "myalign.h"
#include "gtkclass.h"
#include "threads.h"

const char* glib2Name = "libglib-2.0.so.0";
#define LIBNAME glib2
static library_t* my_lib = NULL;

static char* libname = NULL;

typedef void (*vFp_t)(void*);
typedef void* (*pFp_t)(void*);
typedef void  (*vFpp_t)(void*, void*);
typedef void* (*pFup_t)(uint32_t, void*);
typedef int  (*iFpp_t)(void*, void*);
typedef int  (*iFpA_t)(void*, va_list);
typedef size_t(*LFpA_t)(void*, va_list);
typedef void* (*pFpp_t)(void*, void*);
typedef void* (*pFpA_t)(void*, va_list);
typedef void* (*pFpu_t)(void*, uint32_t);
typedef uint32_t  (*uFpp_t)(void*, void*);
typedef int  (*iFppp_t)(void*, void*, void*);
typedef int  (*iFppA_t)(void*, void*, va_list);
typedef uint32_t (*uFipp_t)(int, void*, void*);
typedef uint32_t  (*uFppp_t)(void*, void*, void*);
typedef void  (*vFppp_t)(void*, void*, void*);
typedef uint32_t (*uFupp_t)(uint32_t, void*, void*);
typedef void* (*pFppp_t)(void*, void*, void*);
typedef void  (*vFppip_t)(void*, void*, int, void*);
typedef void* (*pFppip_t)(void*, void*, int, void*);
typedef uint32_t (*uFpipp_t)(void*, int, void*, void*);
typedef uint32_t (*uFuppp_t)(uint32_t, void*, void*, void*);
typedef uint32_t (*uFippp_t)(int, void*, void*, void*);
typedef uint32_t (*uFiuppp_t)(int, uint32_t, void*, void*, void*);
typedef void (*vFpiLpp_t)(void*, int, unsigned long, void*, void*);
typedef void* (*pFpppp_t)(void*, void*, void*, void*);
typedef void (*vFpppp_t)(void*, void*, void*, void*);
typedef void (*vFpupp_t)(void*, uint32_t, void*, void*);
typedef int (*iFpLpp_t)(void*, unsigned long, void*, void*);
typedef int (*iFpLpA_t)(void*, unsigned long, void*, va_list);
typedef void* (*pFpupp_t)(void*, uint32_t, void*, void*);
typedef uint32_t (*uFiippp_t)(int, int, void*, void*, void*);
typedef uint32_t (*uFpiippp_t)(void*, int, int, void*, void*, void*);
typedef int (*iFpupppp_t)(void*, uint32_t, void*, void*, void*, void*);
typedef void* (*pFppuipp_t)(void*, void*, uint32_t, int32_t, void*, void*);
typedef void* (*pFppLiiip_t)(void*, void*, unsigned long, int, int, int, void*);
typedef int (*iFpppipppp_t)(void*, void*, void*, int, void*, void*, void*, void*);
typedef int (*iFpppipppppp_t)(void*, void*, void*, int, void*, void*, void*, void*, void*, void*);
typedef int (*iFpppippppppp_t)(void*, void*, void*, int, void*, void*, void*, void*, void*, void*, void*);
typedef void* (*pFpipA_t)(void*, int, void*, va_list);
typedef void (*vFpipA_t)(void*, int, void*, va_list);
typedef void (*vFppA_t)(void*, void*, va_list);

#define SUPER() \
    GO(g_list_free_full, vFpp_t)                \
    GO(g_markup_vprintf_escaped, pFpA_t)        \
    GO(g_build_filenamev, pFp_t)                \
    GO(g_timeout_add_full, uFiuppp_t)           \
    GO(g_datalist_id_set_data_full, vFpupp_t)   \
    GO(g_datalist_id_dup_data, pFpupp_t)        \
    GO(g_datalist_id_replace_data, iFpupppp_t)  \
    GO(g_variant_new_from_data, pFppuipp_t)     \
    GO(g_variant_new_parsed_va, pFpp_t)         \
    GO(g_variant_get_va, vFpppp_t)              \
    GO(g_strdup_vprintf, pFpA_t)                \
    GO(g_vprintf, iFpA_t)                       \
    GO(g_vfprintf, iFppA_t)                     \
    GO(g_vsprintf, iFppA_t)                     \
    GO(g_vsnprintf, iFpLpA_t)                   \
    GO(g_vasprintf, iFppA_t)                    \
    GO(g_printf_string_upper_bound, LFpA_t)     \
    GO(g_source_new, pFpu_t)                    \
    GO(g_source_set_funcs, vFpp_t)              \
    GO(g_source_remove_by_funcs_user_data, iFpp_t) \
    GO(g_main_context_get_poll_func, pFp_t)     \
    GO(g_main_context_set_poll_func, vFpp_t)    \
    GO(g_print, vFp_t)                          \
    GO(g_printerr, vFp_t)                       \
    GO(g_idle_add_full, uFippp_t)               \
    GO(g_hash_table_new, pFpp_t)                \
    GO(g_hash_table_new_full, pFpppp_t)         \
    GO(g_hash_table_foreach, vFppp_t)           \
    GO(g_hash_table_foreach_remove, uFppp_t)    \
    GO(g_hash_table_foreach_steal, uFppp_t)     \
    GO(g_hash_table_find, pFppp_t)              \
    GO(g_spawn_async_with_pipes, iFpppippppppp_t)\
    GO(g_spawn_async, iFpppipppp_t)             \
    GO(g_spawn_sync, iFpppipppppp_t)            \
    GO(g_child_watch_add, uFipp_t)              \
    GO(g_child_watch_add_full, uFiippp_t)       \
    GO(g_private_new, pFp_t)                    \
    GO(g_static_private_set, vFppp_t)           \
    GO(g_ptr_array_new_with_free_func, pFp_t)   \
    GO(g_ptr_array_new_full, pFup_t)            \
    GO(g_ptr_array_set_free_func, vFpp_t)       \
    GO(g_ptr_array_sort, vFpp_t)                \
    GO(g_ptr_array_sort_with_data, vFppp_t)     \
    GO(g_ptr_array_foreach, vFppp_t)            \
    GO(g_qsort_with_data, vFpiLpp_t)            \
    GO(g_thread_create, pFppip_t)               \
    GO(g_thread_create_full, pFppLiiip_t)       \
    GO(g_thread_foreach, vFpp_t)                \
    GO(g_array_sort, vFpp_t)                    \
    GO(g_array_sort_with_data, vFppp_t)         \
    GO(g_array_set_clear_func, vFpp_t)          \
    GO(g_source_set_callback, vFpppp_t)         \
    GO(g_slist_insert_sorted, pFppp_t)          \
    GO(g_slist_insert_sorted_with_data, pFpppp_t)   \
    GO(g_slist_foreach, pFppp_t)                \
    GO(g_slist_find_custom, pFppp_t)            \
    GO(g_idle_add, uFpp_t)                      \
    GO(g_variant_new_va, pFppp_t)               \
    GO(g_completion_new, pFp_t)                 \
    GO(g_completion_set_compare, vFpp_t)        \
    GO(g_log_set_default_handler, pFpp_t)       \
    GO(g_io_add_watch, uFpipp_t)                \
    GO(g_io_add_watch_full, uFpiippp_t)         \
    GO(g_set_print_handler, pFp_t)              \
    GO(g_set_printerr_handler, pFp_t)           \
    GO(g_slist_sort, pFpp_t)                    \
    GO(g_slist_sort_with_data, pFppp_t)         \
    GO(g_build_pathv, pFpp_t)                   \
    GO(g_list_sort, pFpp_t)                     \
    GO(g_list_sort_with_data, pFppp_t)          \
    GO(g_queue_find_custom, pFppp_t)            \
    GO(g_list_find_custom, pFppp_t)             \
    GO(g_timeout_add_seconds, uFupp_t)          \
    GO(g_timeout_add_seconds_full, uFiuppp_t)   \
    GO(g_log_set_handler, uFpipp_t)             \
    GO(g_set_error_literal, vFppip_t)           \
    GO(g_error_new_valist, pFpipA_t)            \
    GO(g_logv, vFpipA_t)                        \
    GO(g_string_append_vprintf, vFppA_t)        \
    GO(g_string_vprintf, vFppA_t)               \
    GO(g_strjoinv, pFpA_t)                      \


typedef struct glib2_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} glib2_my_t;

void* getGlib2My(library_t* lib)
{
    glib2_my_t* my = (glib2_my_t*)calloc(1, sizeof(glib2_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeGlib2My(void* lib)
{
    //glib2_my_t *my = (glib2_my_t *)lib;
}

EXPORT void* my_g_markup_vprintf_escaped(x64emu_t *emu, void* fmt, void* b) {
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;
    // need to align on arm
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 1);
    PREPARE_VALIST;
    return my->g_markup_vprintf_escaped(fmt, VARARGS);
}

EXPORT void* my_g_build_filename(x64emu_t* emu, void* first, uintptr_t* b)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;
    int n = 0;
    while (getVArgs(emu, 1, b, n++));
    void* array[n+1];   // +1 for 1st (NULL terminal already included)
    array[0] = first;
    for(int i=0; i<n; ++i)
        array[i+1] = (void*)getVArgs(emu, 1, b, i);
    void* ret = my->g_build_filenamev(array);
    return ret;
}

static int my_timeout_cb(my_signal_t* sig)
{
    return (int)RunFunction(my_context, sig->c_handler, 1, sig->data);
}
EXPORT uint32_t my_g_timeout_add(x64emu_t* emu, uint32_t interval, void* func, void* data)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    my_signal_t *sig = new_mysignal(func, data, NULL);
    return my->g_timeout_add_full(0, interval, my_timeout_cb, sig, my_signal_delete);
}
typedef int (*GSourceFunc) (void* user_data);

typedef struct my_GSourceFuncs_s {
  int  (*prepare)  (void* source, int* timeout_);
  int  (*check)    (void* source);
  int  (*dispatch) (void* source, GSourceFunc callback,void* user_data);
  void (*finalize) (void* source);
} my_GSourceFuncs_t;

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)   \
GO(5)   \
GO(6)   \
GO(7)

// GCopyFct
/*#define GO(A)   \
static uintptr_t my_copy_fct_##A = 0;   \
static void* my_copy_##A(void* data)     \
{                                       \
    return (void*)RunFunction(my_context, my_copy_fct_##A, 1, data);\
}
SUPER()
#undef GO
static void* findCopyFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_copy_fct_##A == (uintptr_t)fct) return my_copy_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_copy_fct_##A == 0) {my_copy_fct_##A = (uintptr_t)fct; return my_copy_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 Copy callback\n");
    return NULL;
}*/
// GFreeFct
#define GO(A)   \
static uintptr_t my_free_fct_##A = 0;   \
static void my_free_##A(void* data)     \
{                                       \
    RunFunction(my_context, my_free_fct_##A, 1, data);\
}
SUPER()
#undef GO
static void* findFreeFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_free_fct_##A == (uintptr_t)fct) return my_free_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_free_fct_##A == 0) {my_free_fct_##A = (uintptr_t)fct; return my_free_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 Free callback\n");
    return NULL;
}
// GDuplicateFct
#define GO(A)   \
static uintptr_t my_duplicate_fct_##A = 0;   \
static void* my_duplicate_##A(void* data)     \
{                                       \
    return (void*)RunFunction(my_context, my_duplicate_fct_##A, 1, data);\
}
SUPER()
#undef GO
static void* findDuplicateFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_duplicate_fct_##A == (uintptr_t)fct) return my_duplicate_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_duplicate_fct_##A == 0) {my_duplicate_fct_##A = (uintptr_t)fct; return my_duplicate_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 Duplicate callback\n");
    return NULL;
}
// GSourceFuncs....
// g_source_new callback. First the structure GSourceFuncs statics, with paired x64 source pointer
#define GO(A) \
static my_GSourceFuncs_t     my_gsourcefuncs_##A = {0};   \
static my_GSourceFuncs_t   *ref_gsourcefuncs_##A = NULL;
SUPER()
#undef GO
// then the static functions callback that may be used with the structure, but dispatch also have a callback...
#define GO(A)   \
static uintptr_t fct_funcs_prepare_##A = 0;  \
static int my_funcs_prepare_##A(void* source, int timeout_) {   \
    return (int)RunFunction(my_context, fct_funcs_prepare_##A, 2, source, timeout_);    \
}   \
static uintptr_t fct_funcs_check_##A = 0;  \
static int my_funcs_check_##A(void* source) {   \
    return (int)RunFunction(my_context, fct_funcs_check_##A, 1, source);    \
}   \
static uintptr_t fct_funcs_dispatch_cb_##A = 0; \
static int my_funcs_dispatch_cb_##A(void* a, void* b, void* c, void* d) {   \
    return (int)RunFunction(my_context, fct_funcs_dispatch_cb_##A, 4, a, b, c, d);    \
}   \
static uintptr_t fct_funcs_dispatch_##A = 0;  \
static int my_funcs_dispatch_##A(void* source, void* cb, void* data) {   \
    uintptr_t old = fct_funcs_dispatch_cb_##A;  \
    fct_funcs_dispatch_cb_##A = (uintptr_t)cb;  \
    return (int)RunFunction(my_context, fct_funcs_dispatch_##A, 3, source, cb?my_funcs_dispatch_cb_##A:NULL, data);    \
    fct_funcs_dispatch_cb_##A = old;    \
}   \
static uintptr_t fct_funcs_finalize_##A = 0;  \
static int my_funcs_finalize_##A(void* source) {   \
    return (int)RunFunction(my_context, fct_funcs_finalize_##A, 1, source);    \
}
SUPER()
#undef GO
// and now the get slot / assign... Taking into account that the desired callback may already be a wrapped one (so unwrapping it)
static my_GSourceFuncs_t* findFreeGSourceFuncs(my_GSourceFuncs_t* fcts)
{
    if(!fcts) return fcts;
    #define GO(A) if(ref_gsourcefuncs_##A == fcts) return &my_gsourcefuncs_##A;
    SUPER()
    #undef GO
    #define GO(A) if(ref_gsourcefuncs_##A == 0) {   \
        ref_gsourcefuncs_##A = fcts;                 \
        my_gsourcefuncs_##A.prepare = (fcts->prepare)?((GetNativeFnc((uintptr_t)fcts->prepare))?GetNativeFnc((uintptr_t)fcts->prepare):my_funcs_prepare_##A):NULL;    \
        fct_funcs_prepare_##A = (uintptr_t)fcts->prepare;                            \
        my_gsourcefuncs_##A.check = (fcts->check)?((GetNativeFnc((uintptr_t)fcts->check))?GetNativeFnc((uintptr_t)fcts->check):my_funcs_check_##A):NULL;    \
        fct_funcs_check_##A = (uintptr_t)fcts->check;                            \
        my_gsourcefuncs_##A.dispatch = (fcts->dispatch)?((GetNativeFnc((uintptr_t)fcts->dispatch))?GetNativeFnc((uintptr_t)fcts->dispatch):my_funcs_dispatch_##A):NULL;    \
        fct_funcs_dispatch_##A = (uintptr_t)fcts->dispatch;                            \
        my_gsourcefuncs_##A.finalize = (fcts->finalize)?((GetNativeFnc((uintptr_t)fcts->finalize))?GetNativeFnc((uintptr_t)fcts->finalize):my_funcs_finalize_##A):NULL;    \
        fct_funcs_finalize_##A = (uintptr_t)fcts->finalize;                            \
        return &my_gsourcefuncs_##A;                \
    }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GSourceFuncs callback\n");
    return NULL;
}

// PollFunc ...
#define GO(A)   \
static uintptr_t my_poll_fct_##A = 0;   \
static int my_poll_##A(void* ufds, uint32_t nfsd, int32_t timeout_)     \
{                                       \
    return RunFunction(my_context, my_poll_fct_##A, 3, ufds, nfsd, timeout_);\
}
SUPER()
#undef GO
static void* findPollFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_poll_fct_##A == (uintptr_t)fct) return my_poll_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_poll_fct_##A == 0) {my_poll_fct_##A = (uintptr_t)fct; return my_poll_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 Poll callback\n");
    return NULL;
}

static void* reversePollFct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if((uintptr_t)fct == my_poll_fct_##A) return (void*)my_poll_fct_##A;
    SUPER()
    #undef GO
    return NULL;
}

// GHashFunc ...
#define GO(A)   \
static uintptr_t my_hashfunc_fct_##A = 0;   \
static uint32_t my_hashfunc_##A(void* key)     \
{                                       \
    return (uint32_t)RunFunction(my_context, my_hashfunc_fct_##A, 1, key);\
}
SUPER()
#undef GO
static void* findHashFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_hashfunc_fct_##A == (uintptr_t)fct) return my_hashfunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_hashfunc_fct_##A == 0) {my_hashfunc_fct_##A = (uintptr_t)fct; return my_hashfunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GHashFunc callback\n");
    return NULL;
}
// GEqualFunc ...
#define GO(A)   \
static uintptr_t my_equalfunc_fct_##A = 0;   \
static int my_equalfunc_##A(void* a, void* b)     \
{                                       \
    return RunFunction(my_context, my_equalfunc_fct_##A, 2, a, b);\
}
SUPER()
#undef GO
static void* findEqualFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_equalfunc_fct_##A == (uintptr_t)fct) return my_equalfunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_equalfunc_fct_##A == 0) {my_equalfunc_fct_##A = (uintptr_t)fct; return my_equalfunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GEqualFunc callback\n");
    return NULL;
}
// GDestroyFunc ...
#define GO(A)   \
static uintptr_t my_destroyfunc_fct_##A = 0;   \
static int my_destroyfunc_##A(void* a, void* b)     \
{                                       \
    return RunFunction(my_context, my_destroyfunc_fct_##A, 2, a, b);\
}
SUPER()
#undef GO
static void* findDestroyFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_destroyfunc_fct_##A == (uintptr_t)fct) return my_destroyfunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_destroyfunc_fct_##A == 0) {my_destroyfunc_fct_##A = (uintptr_t)fct; return my_destroyfunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GDestroyNotify callback\n");
    return NULL;
}
// GSpawnChildSetupFunc ...
#define GO(A)   \
static uintptr_t my_spwnchildsetup_fct_##A = 0;   \
static void my_spwnchildsetup_##A(void* data)     \
{                                       \
    RunFunction(my_context, my_spwnchildsetup_fct_##A, 1, data);\
}
SUPER()
#undef GO
static void* findSpawnChildSetupFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_spwnchildsetup_fct_##A == (uintptr_t)fct) return my_spwnchildsetup_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_spwnchildsetup_fct_##A == 0) {my_spwnchildsetup_fct_##A = (uintptr_t)fct; return my_spwnchildsetup_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GSpawnChildSetup callback\n");
    return NULL;
}
// GSourceFunc ...
#define GO(A)   \
static uintptr_t my_GSourceFunc_fct_##A = 0;   \
static void my_GSourceFunc_##A(void* a, void* b, void* c, void* d)     \
{                                       \
    RunFunction(my_context, my_GSourceFunc_fct_##A, 4, a, b, c, d);\
}
SUPER()
#undef GO
static void* findGSourceFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GSourceFunc_fct_##A == (uintptr_t)fct) return my_GSourceFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GSourceFunc_fct_##A == 0) {my_GSourceFunc_fct_##A = (uintptr_t)fct; return my_GSourceFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GSourceFunc callback\n");
    return NULL;
}
// GCompareFunc ...
#define GO(A)   \
static uintptr_t my_GCompareFunc_fct_##A = 0;   \
static int my_GCompareFunc_##A(void* a, void* b)     \
{                                       \
    return (int)RunFunction(my_context, my_GCompareFunc_fct_##A, 2, a, b);\
}
SUPER()
#undef GO
static void* findGCompareFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GCompareFunc_fct_##A == (uintptr_t)fct) return my_GCompareFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GCompareFunc_fct_##A == 0) {my_GCompareFunc_fct_##A = (uintptr_t)fct; return my_GCompareFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GCompareFunc callback\n");
    return NULL;
}
// GCompareDataFunc ...
#define GO(A)   \
static uintptr_t my_GCompareDataFunc_fct_##A = 0;                   \
static int my_GCompareDataFunc_##A(void* a, void* b, void* data)    \
{                                       \
    return (int)RunFunction(my_context, my_GCompareDataFunc_fct_##A, 3, a, b, data);\
}
SUPER()
#undef GO
static void* findGCompareDataFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GCompareDataFunc_fct_##A == (uintptr_t)fct) return my_GCompareDataFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GCompareDataFunc_fct_##A == 0) {my_GCompareDataFunc_fct_##A = (uintptr_t)fct; return my_GCompareDataFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GCompareDataFunc callback\n");
    return NULL;
}
// GCompletionFunc ...
#define GO(A)   \
static uintptr_t my_GCompletionFunc_fct_##A = 0;                            \
static void* my_GCompletionFunc_##A(void* a)                                \
{                                                                           \
    return (void*)RunFunction(my_context, my_GCompletionFunc_fct_##A, 1, a);\
}
SUPER()
#undef GO
static void* findGCompletionFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GCompletionFunc_fct_##A == (uintptr_t)fct) return my_GCompletionFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GCompletionFunc_fct_##A == 0) {my_GCompletionFunc_fct_##A = (uintptr_t)fct; return my_GCompletionFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GCompletionFunc callback\n");
    return NULL;
}
// GCompletionStrncmpFunc ...
#define GO(A)   \
static uintptr_t my_GCompletionStrncmpFunc_fct_##A = 0;                                 \
static int my_GCompletionStrncmpFunc_##A(void* a, void* b, unsigned long n)             \
{                                                                                       \
    return (int)RunFunction(my_context, my_GCompletionStrncmpFunc_fct_##A, 3, a, b, n); \
}
SUPER()
#undef GO
static void* findGCompletionStrncmpFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GCompletionStrncmpFunc_fct_##A == (uintptr_t)fct) return my_GCompletionStrncmpFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GCompletionStrncmpFunc_fct_##A == 0) {my_GCompletionStrncmpFunc_fct_##A = (uintptr_t)fct; return my_GCompletionStrncmpFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GCompletionStrncmpFunc callback\n");
    return NULL;
}
// GIOFunc ...
#define GO(A)   \
static uintptr_t my_GIOFunc_fct_##A = 0;                                 \
static int my_GIOFunc_##A(void* a, int b, void* c)             \
{                                                                                       \
    return (int)RunFunction(my_context, my_GIOFunc_fct_##A, 3, a, b, c); \
}
SUPER()
#undef GO
static void* findGIOFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GIOFunc_fct_##A == (uintptr_t)fct) return my_GIOFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GIOFunc_fct_##A == 0) {my_GIOFunc_fct_##A = (uintptr_t)fct; return my_GIOFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GIOFunc callback\n");
    return NULL;
}
// GDestroyNotify ...
#define GO(A)   \
static uintptr_t my_GDestroyNotify_fct_##A = 0;                 \
static void my_GDestroyNotify_##A(void* a)                      \
{                                                               \
    RunFunction(my_context, my_GDestroyNotify_fct_##A, 1, a);   \
}
SUPER()
#undef GO
static void* findGDestroyNotifyFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GDestroyNotify_fct_##A == (uintptr_t)fct) return my_GDestroyNotify_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GDestroyNotify_fct_##A == 0) {my_GDestroyNotify_fct_##A = (uintptr_t)fct; return my_GDestroyNotify_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GDestroyNotify callback\n");
    return NULL;
}
// GFunc ...
#define GO(A)   \
static uintptr_t my_GFunc_fct_##A = 0;                  \
static void my_GFunc_##A(void* a, void* b)              \
{                                                       \
    RunFunction(my_context, my_GFunc_fct_##A, 2, a, b); \
}
SUPER()
#undef GO
static void* findGFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GFunc_fct_##A == (uintptr_t)fct) return my_GFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GFunc_fct_##A == 0) {my_GFunc_fct_##A = (uintptr_t)fct; return my_GFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GFunc callback\n");
    return NULL;
}
// GHFunc ...
#define GO(A)   \
static uintptr_t my_GHFunc_fct_##A = 0;                     \
static void my_GHFunc_##A(void* a, void* b, void* c)        \
{                                                           \
    RunFunction(my_context, my_GHFunc_fct_##A, 3, a, b, c); \
}
SUPER()
#undef GO
static void* findGHFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GHFunc_fct_##A == (uintptr_t)fct) return my_GHFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GHFunc_fct_##A == 0) {my_GHFunc_fct_##A = (uintptr_t)fct; return my_GHFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GHFunc callback\n");
    return NULL;
}
// GHRFunc ...
#define GO(A)   \
static uintptr_t my_GHRFunc_fct_##A = 0;                            \
static int my_GHRFunc_##A(void* a, void* b, void* c)                \
{                                                                   \
    return RunFunction(my_context, my_GHRFunc_fct_##A, 3, a, b, c); \
}
SUPER()
#undef GO
static void* findGHRFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GHRFunc_fct_##A == (uintptr_t)fct) return my_GHRFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GHRFunc_fct_##A == 0) {my_GHRFunc_fct_##A = (uintptr_t)fct; return my_GHRFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GHRFunc callback\n");
    return NULL;
}
// GChildWatchFunc ...
#define GO(A)   \
static uintptr_t my_GChildWatchFunc_fct_##A = 0;                        \
static void my_GChildWatchFunc_##A(int a, int b, void* c)               \
{                                                                       \
    RunFunction(my_context, my_GChildWatchFunc_fct_##A, 3, a, b, c);    \
}
SUPER()
#undef GO
static void* findGChildWatchFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GChildWatchFunc_fct_##A == (uintptr_t)fct) return my_GChildWatchFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GChildWatchFunc_fct_##A == 0) {my_GChildWatchFunc_fct_##A = (uintptr_t)fct; return my_GChildWatchFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GChildWatchFunc callback\n");
    return NULL;
}
// GLogFunc ...
#define GO(A)   \
static uintptr_t my_GLogFunc_fct_##A = 0;                           \
static void my_GLogFunc_##A(void* a, int b, void* c, void* d)       \
{                                                                   \
    RunFunction(my_context, my_GLogFunc_fct_##A, 4, a, b, c, d);    \
}
SUPER()
#undef GO
static void* findGLogFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GLogFunc_fct_##A == (uintptr_t)fct) return my_GLogFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GLogFunc_fct_##A == 0) {my_GLogFunc_fct_##A = (uintptr_t)fct; return my_GLogFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GLogFunc callback\n");
    return NULL;
}
static void* reverseGLogFuncFct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if((uintptr_t)fct == my_GLogFunc_fct_##A) return (void*)my_GLogFunc_fct_##A;
    SUPER()
    #undef GO
    return NULL;
}
// GPrintFunc ...
#define GO(A)   \
static uintptr_t my_GPrintFunc_fct_##A = 0;                     \
static void my_GPrintFunc_##A(void* a)                          \
{                                                               \
    RunFunction(my_context, my_GPrintFunc_fct_##A, 1, a);       \
}
SUPER()
#undef GO
static void* findGPrintFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GPrintFunc_fct_##A == (uintptr_t)fct) return my_GPrintFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GPrintFunc_fct_##A == 0) {my_GPrintFunc_fct_##A = (uintptr_t)fct; return my_GPrintFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GPrintFunc callback\n");
    return NULL;
}
static void* reverseGPrintFuncFct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if((uintptr_t)fct == my_GPrintFunc_fct_##A) return (void*)my_GPrintFunc_fct_##A;
    SUPER()
    #undef GO
    return NULL;
}

#undef SUPER

EXPORT void my_g_list_free_full(x64emu_t* emu, void* list, void* free_func)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;
    my->g_list_free_full(list, findFreeFct(free_func));
}

EXPORT void* my_g_markup_printf_escaped(x64emu_t *emu, void* fmt, void* b) {
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;
    // need to align on arm
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 1);
    PREPARE_VALIST;
    return my->g_markup_vprintf_escaped(fmt, VARARGS);
}


EXPORT void my_g_datalist_id_set_data_full(x64emu_t* emu, void* datalist, uint32_t key, void* data, void* freecb)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;
    void* fc = findFreeFct(freecb);
    my->g_datalist_id_set_data_full(datalist, key, data, fc);
}

EXPORT void* my_g_datalist_id_dup_data(x64emu_t* emu, void* datalist, uint32_t key, void* dupcb, void* data)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;
    void* cc = findDuplicateFct(dupcb);
    return my->g_datalist_id_dup_data(datalist, key, cc, data);
}

EXPORT int my_g_datalist_id_replace_data(x64emu_t* emu, void* datalist, uint32_t key, void* oldval, void* newval, void* oldfree, void* newfree)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;
    void* oldfc = findFreeFct(oldfree);
    void* newfc = findFreeFct(newfree);
    return my->g_datalist_id_replace_data(datalist, key, oldval, newval, oldfc, newfc);
}

EXPORT void* my_g_variant_new_from_data(x64emu_t* emu, void* type, void* data, uint32_t size, int trusted, void* freecb, void* datacb)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;
    void* fc = findFreeFct(freecb);
    return my->g_variant_new_from_data(type, data, size, trusted, fc, datacb);
}

EXPORT void* my_g_variant_new_parsed_va(x64emu_t* emu, void* fmt, x64_va_list_t b)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;
    CONVERT_VALIST(b);
    return my->g_variant_new_parsed_va(fmt, &VARARGS);
}

EXPORT void my_g_variant_get(x64emu_t* emu, void* value, void* fmt, uint64_t* V)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;
    CREATE_VALIST_FROM_VAARG(V, emu->scratch, 2);
    my->g_variant_get_va(value, fmt, NULL, &VARARGS);
}

EXPORT void* my_g_strdup_vprintf(x64emu_t* emu, void* fmt, void* b)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 1);
    PREPARE_VALIST;
    return my->g_strdup_vprintf(fmt, VARARGS);
}

EXPORT int my_g_vprintf(x64emu_t* emu, void* fmt, void* b)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 1);
    PREPARE_VALIST;
    return my->g_vprintf(fmt, VARARGS);
}

EXPORT int my_g_vfprintf(x64emu_t* emu, void* F, void* fmt, void* b)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 2);
    PREPARE_VALIST;
    return my->g_vfprintf(F, fmt, VARARGS);
}

EXPORT int my_g_vsprintf(x64emu_t* emu, void* s, void* fmt, void* b)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 2);
    PREPARE_VALIST;
    return my->g_vsprintf(s, fmt, VARARGS);
}

EXPORT int my_g_vsnprintf(x64emu_t* emu, void* s, unsigned long n, void* fmt, void* b)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 3);
    PREPARE_VALIST;
    return my->g_vsnprintf(s, n, fmt, VARARGS);
}

EXPORT int my_g_vasprintf(x64emu_t* emu, void* s, void* fmt, void* b)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 2);
    PREPARE_VALIST;
    return my->g_vasprintf(s, fmt, VARARGS);
}

EXPORT uint32_t my_g_printf_string_upper_bound(x64emu_t* emu, void* fmt, void* b)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 1);
    PREPARE_VALIST;
    return my->g_printf_string_upper_bound(fmt, VARARGS);
}

EXPORT void my_g_print(x64emu_t* emu, void* fmt, void* b)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    char* buf = NULL;
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 1);
    PREPARE_VALIST;
    int ret = vasprintf(&buf, fmt, VARARGS);
    (void)ret;
    my->g_print(buf);
    free(buf);
}

EXPORT void my_g_printerr(x64emu_t* emu, void* fmt, void* b)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    char* buf = NULL;
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 1);
    PREPARE_VALIST;
    int ret = vasprintf(&buf, fmt, VARARGS);
    (void)ret;
    my->g_printerr(buf);
    free(buf);
}

EXPORT void* my_g_source_new(x64emu_t* emu, my_GSourceFuncs_t* source_funcs, uint32_t struct_size)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    return my->g_source_new(findFreeGSourceFuncs(source_funcs), struct_size);
}

EXPORT void my_g_source_set_funcs(x64emu_t* emu, void* source, my_GSourceFuncs_t* source_funcs)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    my->g_source_set_funcs(source, findFreeGSourceFuncs(source_funcs));
}


EXPORT int my_g_source_remove_by_funcs_user_data(x64emu_t* emu, my_GSourceFuncs_t* source_funcs, void* data)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    return my->g_source_remove_by_funcs_user_data(findFreeGSourceFuncs(source_funcs), data);
}

EXPORT void* my_g_main_context_get_poll_func(x64emu_t* emu, void* context)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    void* ret = my->g_main_context_get_poll_func(context);
    if(!ret) return ret;
    void* r = reversePollFct(ret);
    if(r) return r;
    // needs to bridge....
    return (void*)AddCheckBridge(my_lib->priv.w.bridge, iFpui, ret, 0, NULL);
}
    
EXPORT void my_g_main_context_set_poll_func(x64emu_t* emu, void* context, void* func)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    my->g_main_context_set_poll_func(context, findPollFct(func));
}

EXPORT uint32_t my_g_idle_add_full(x64emu_t* emu, int priority, void* f, void* data, void* notify)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    if(!f)
        return my->g_idle_add_full(priority, f, data, notify);

    my_signal_t *sig = new_mysignal(f, data, notify);
    printf_log(LOG_DEBUG, "glib2 Idle CB with priority %d created for %p, sig=%p\n", priority, f, sig);
    return my->g_idle_add_full(priority, my_timeout_cb, sig, my_signal_delete);
}

EXPORT void* my_g_hash_table_new(x64emu_t* emu, void* hash, void* equal)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    return my->g_hash_table_new(findHashFct(hash), findEqualFct(equal));
}

EXPORT void* my_g_hash_table_new_full(x64emu_t* emu, void* hash, void* equal, void* destroy_key, void* destroy_val)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    return my->g_hash_table_new_full(findHashFct(hash), findEqualFct(equal), findDestroyFct(destroy_key), findDestroyFct(destroy_val));
}

EXPORT void my_g_hash_table_foreach(x64emu_t* emu, void* table, void* f, void* data)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    my->g_hash_table_foreach(table, findGHFuncFct(f), data);
}

EXPORT uint32_t my_g_hash_table_foreach_remove(x64emu_t* emu, void* table, void* f, void* data)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;
    
    return my->g_hash_table_foreach_remove(table, findGHRFuncFct(f), data);
}
EXPORT uint32_t my_g_hash_table_foreach_steal(x64emu_t* emu, void* table, void* f, void* data)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;
    
    return my->g_hash_table_foreach_steal(table, findGHRFuncFct(f), data);
}
EXPORT void* my_g_hash_table_find(x64emu_t* emu, void* table, void* f, void* data)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;
    
    return my->g_hash_table_find(table, findGHRFuncFct(f), data);
}

EXPORT int my_g_spawn_async_with_pipes(x64emu_t* emu, void* dir, void* argv, void* envp, int flags, void* f, void* data, void* child, void* input, void* output, void* err, void* error)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    return my->g_spawn_async_with_pipes(dir, argv, envp, flags, findSpawnChildSetupFct(f), data, child, input, output, err, error);
}

EXPORT int my_g_spawn_async(x64emu_t* emu, void* dir, void* argv, void* envp, int flags, void* f, void* data, void* child, void* error)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    return my->g_spawn_async(dir, argv, envp, flags, findSpawnChildSetupFct(f), data, child, error);
}

EXPORT int my_g_spawn_sync(x64emu_t* emu, void* dir, void* argv, void* envp, int flags, void* f, void* data, void* input, void* output, void* status, void* error)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    return my->g_spawn_sync(dir, argv, envp, flags, findSpawnChildSetupFct(f), data, input, output, status, error);
}

EXPORT uint32_t my_g_child_watch_add(x64emu_t* emu, int pid, void* f, void* data)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    return my->g_child_watch_add(pid, findGChildWatchFuncFct(f), data);
}

EXPORT uint32_t my_g_child_watch_add_full(x64emu_t* emu, int priority, int pid, void* f, void* data, void* notify)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    return my->g_idle_add_full(priority, findGChildWatchFuncFct(f), data, findGDestroyNotifyFct(notify));
}

EXPORT void* my_g_private_new(x64emu_t* emu, void* notify)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    return my->g_private_new(findFreeFct(notify));
}

EXPORT void my_g_static_private_set(x64emu_t* emu, void* private, void* data, void* notify)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    my->g_static_private_set(private, data, findFreeFct(notify));
}

EXPORT void* my_g_ptr_array_new_with_free_func(x64emu_t* emu, void* notify)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    return my->g_ptr_array_new_with_free_func(findFreeFct(notify));
}

EXPORT void* my_g_ptr_array_new_full(x64emu_t* emu, uint32_t size, void* notify)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    return my->g_ptr_array_new_full(size, findFreeFct(notify));
}

EXPORT void my_g_ptr_array_set_free_func(x64emu_t* emu, void* array, void* notify)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    my->g_ptr_array_set_free_func(array, findFreeFct(notify));
}

EXPORT void my_g_ptr_array_sort(x64emu_t* emu, void* array, void* comp)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    my->g_ptr_array_sort(array, findGCompareFuncFct(comp));
}

EXPORT void my_g_ptr_array_sort_with_data(x64emu_t* emu, void* array, void* comp, void* data)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    my->g_ptr_array_sort_with_data(array, findGCompareDataFuncFct(comp), data);
}

EXPORT void my_g_qsort_with_data(x64emu_t* emu, void* pbase, int total, unsigned long size, void* comp, void* data)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    my->g_qsort_with_data(pbase, total, size, findGCompareDataFuncFct(comp), data);
}

EXPORT void my_g_ptr_array_foreach(x64emu_t* emu, void* array, void* func, void* data)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    my->g_ptr_array_foreach(array, findGFuncFct(func), data);
}

EXPORT void* my_g_thread_create(x64emu_t* emu, void* func, void* data, int joinable, void* error)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    void* et = NULL;
    return my->g_thread_create(my_prepare_thread(emu, func, data, 0, &et), et, joinable, error);
}

EXPORT void* my_g_thread_create_full(x64emu_t* emu, void* func, void* data, unsigned long stack, int joinable, int bound, int priority, void* error)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    void* et = NULL;
    return my->g_thread_create_full(my_prepare_thread(emu, func, data, stack, &et), et, stack, joinable, bound, priority, error);
}

EXPORT void my_g_thread_foreach(x64emu_t* emu, void* func, void* data)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    my->g_thread_foreach(findGFuncFct(func), data);
}

EXPORT void my_g_array_sort(x64emu_t* emu, void* array, void* comp)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    my->g_array_sort(array, findGCompareFuncFct(comp));
}

EXPORT void my_g_array_sort_with_data(x64emu_t* emu, void* array, void* comp, void* data)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    my->g_array_sort_with_data(array, findGCompareDataFuncFct(comp), data);
}

EXPORT void my_g_array_set_clear_func(x64emu_t* emu, void* array, void* notify)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    my->g_array_set_clear_func(array, findFreeFct(notify));
}

EXPORT void my_g_source_set_callback(x64emu_t* emu, void* source, void* func, void* data, void* notify)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    my->g_source_set_callback(source, findGSourceFuncFct(func), data, findFreeFct(notify));
}

EXPORT void* my_g_slist_insert_sorted(x64emu_t* emu, void* list, void* d, void* comp)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    return my->g_slist_insert_sorted(list, d, findGCompareFuncFct(comp));
}
EXPORT void* my_g_slist_insert_sorted_with_data(x64emu_t* emu, void* list, void* d, void* comp, void* data)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    return my->g_slist_insert_sorted_with_data(list, d, findGCompareDataFuncFct(comp), data);
}

EXPORT void my_g_slist_foreach(x64emu_t* emu, void* list, void* func, void* data)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    my->g_slist_foreach(list, findGFuncFct(func), data);
}

EXPORT void* my_g_slist_find_custom(x64emu_t* emu, void* list, void* data, void* comp)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    return my->g_slist_find_custom(list, data, findGCompareFuncFct(comp));
}

EXPORT uint32_t my_g_idle_add(x64emu_t* emu, void* func, void* data)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    return my->g_idle_add(findGSourceFuncFct(func), data);
}

EXPORT void* my_g_variant_new_va(x64emu_t* emu, char* fmt, void* endptr, x64_va_list_t* b)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    CONVERT_VALIST(*b);
    va_list* aligned = &VARARGS;
    return my->g_variant_new_va(fmt, endptr, &aligned);
}

EXPORT void* my_g_variant_new(x64emu_t* emu, char* fmt, uint64_t* V)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    CREATE_VALIST_FROM_VAARG(V, emu->scratch, 1);
    return my->g_variant_new_va(fmt, NULL, &VARARGS);
}

EXPORT void* my_g_completion_new(x64emu_t* emu, void* f)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    return my->g_completion_new(findGCompletionFct(f));
}

EXPORT void my_g_completion_set_compare(x64emu_t *emu, void* cmp, void* f)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    my->g_completion_set_compare(cmp, findGCompletionStrncmpFuncFct(f));
}

EXPORT void* my_g_log_set_default_handler(x64emu_t *emu, void* f, void* data)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    return reverseGLogFuncFct(my->g_log_set_default_handler(findGLogFuncFct(f), data));
}

EXPORT uint32_t my_g_io_add_watch_full(x64emu_t* emu, void* channel, int priority, int cond, void* f, void* data, void* notify)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    return my->g_io_add_watch_full(channel, priority, cond, findGIOFuncFct(f), data, findDestroyFct(notify));
}

EXPORT uint32_t my_g_io_add_watch(x64emu_t* emu, void* channel, int cond, void* f, void* data)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    return my->g_io_add_watch(channel, cond, findGIOFuncFct(f), data);
}

EXPORT void* my_g_set_print_handler(x64emu_t *emu, void* f)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    return reverseGPrintFuncFct(my->g_set_print_handler(findGPrintFuncFct(f)));
}

EXPORT void* my_g_set_printerr_handler(x64emu_t *emu, void* f)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    return reverseGPrintFuncFct(my->g_set_printerr_handler(findGPrintFuncFct(f)));
}

EXPORT void* my_g_slist_sort(x64emu_t *emu, void* list, void* f)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    return my->g_slist_sort(list, findGCompareFuncFct(f));
}

EXPORT void* my_g_slist_sort_with_data(x64emu_t *emu, void* list, void* f, void* data)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    return my->g_slist_sort_with_data(list, findGCompareDataFuncFct(f), data);
}

EXPORT void* my_g_build_path(x64emu_t *emu, void* sep, void* first, uintptr_t* data)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    int n = (first)?1:0;
    void* p = n?((void*)getVArgs(emu, 2, data, 0)):NULL;
    while(p) {
        p = (void*)getVArgs(emu, 2, data, n++);
    }
    ++n;    // final NULL
    void** args = (void**)malloc(n *sizeof(void*));
    args[0] = first;
    for(int i=1; i<n; ++i)
        args[i] = (void*)getVArgs(emu, 2, data, i-1);
    p = my->g_build_pathv(sep, args);
    free(args);
    return p;
}

EXPORT void* my_g_list_sort(x64emu_t *emu, void* list, void* f)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    return my->g_list_sort(list, findGCompareFuncFct(f));
}

EXPORT void* my_g_list_sort_with_data(x64emu_t *emu, void* list, void* f, void* data)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    return my->g_list_sort_with_data(list, findGCompareDataFuncFct(f), data);
}

EXPORT void* my_g_queue_find_custom(x64emu_t *emu, void* queue, void* data, void* f)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    return my->g_queue_find_custom(queue, data, findGCompareFuncFct(f));
}

EXPORT void* my_g_list_find_custom(x64emu_t *emu, void* list, void* data, void* f)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    return my->g_list_find_custom(list, data, findGCompareFuncFct(f));
}

EXPORT uint32_t my_g_timeout_add_full(x64emu_t *emu, int priority, uint32_t interval, void* f, void* data, void* notify)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    return my->g_timeout_add_full(priority, interval, findGSourceFuncFct(f), data, findDestroyFct(notify));
}

EXPORT uint32_t my_g_timeout_add_seconds(x64emu_t *emu, uint32_t interval, void* f, void* data)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    return my->g_timeout_add_seconds(interval, findGSourceFuncFct(f), data);
}

EXPORT uint32_t my_g_timeout_add_seconds_full(x64emu_t *emu, int priority, uint32_t interval, void* f, void* data, void* notify)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    return my->g_timeout_add_seconds_full(priority, interval, findGSourceFuncFct(f), data, findDestroyFct(notify));
}

EXPORT uint32_t my_g_log_set_handler(x64emu_t *emu, void* domain, int level, void* f, void* data)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;

    return my->g_log_set_handler(domain, level, findGLogFuncFct(f), data);
}

EXPORT void my_g_set_error(x64emu_t *emu, void* err, void* domain, int code, void* fmt, uintptr_t* stack)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;
    char buf[1000];
    myStackAlign(emu, fmt, stack, emu->scratch, R_EAX, 4);
    PREPARE_VALIST;
    vsnprintf(buf, sizeof(buf), fmt, VARARGS);
    my->g_set_error_literal(err, domain, code, buf);
}

EXPORT void* my_g_error_new(x64emu_t* emu, void* domain, int code, void* fmt, uintptr_t* b)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;
    myStackAlign(emu, fmt, b, emu->scratch, R_EAX, 3);
    PREPARE_VALIST;
    return my->g_error_new_valist(domain, code, fmt, VARARGS);
}
EXPORT void* my_g_error_new_valist(x64emu_t* emu, void* domain, int code, void* fmt, x64_va_list_t V)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;
    CONVERT_VALIST(V);
    return my->g_error_new_valist(domain, code, fmt, VARARGS);
}

EXPORT int my_g_fprintf(x64emu_t* emu, void* f, void* fmt, uintptr_t* b)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;
    myStackAlign(emu, fmt, b, emu->scratch, R_EAX, 3);
    PREPARE_VALIST;
    return my->g_vfprintf(f, fmt, VARARGS);
}

EXPORT void my_g_logv(x64emu_t* emu, void* domain, int level, void* fmt, x64_va_list_t V)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;
    CONVERT_VALIST(V);
    my->g_logv(domain, level, fmt, VARARGS);
}
EXPORT void my_g_log(x64emu_t* emu, void* domain, int level, void* fmt, uintptr_t* b)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;
    myStackAlign(emu, fmt, b, emu->scratch, R_EAX, 3);
    PREPARE_VALIST;
    my->g_logv(domain, level, fmt, VARARGS);
}

EXPORT int my_g_printf(x64emu_t* emu, void* fmt, uintptr_t* b)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;
    myStackAlign(emu, fmt, b, emu->scratch, R_EAX, 1);
    PREPARE_VALIST;
    return my->g_vprintf(fmt, VARARGS);
}

EXPORT int my_g_snprintf(x64emu_t* emu, void* buf, size_t l, void* fmt, uintptr_t* b)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;
    myStackAlign(emu, fmt, b, emu->scratch, R_EAX, 3);
    PREPARE_VALIST;
    return my->g_vsnprintf(buf, l, fmt, VARARGS);
}

EXPORT int my_g_sprintf(x64emu_t* emu, void* buf, void* fmt, uintptr_t* b)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;
    myStackAlign(emu, fmt, b, emu->scratch, R_EAX, 2);
    PREPARE_VALIST;
    my->g_vsprintf(buf, fmt, VARARGS);
}

EXPORT void* my_g_strdup_printf(x64emu_t* emu, void* fmt, uintptr_t* b)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;
    myStackAlign(emu, fmt, b, emu->scratch, R_EAX, 1);
    PREPARE_VALIST;
    return my->g_strdup_vprintf(fmt, VARARGS);
}

EXPORT void my_g_string_append_printf(x64emu_t* emu, void* string, void* fmt, uintptr_t* b)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;
    myStackAlign(emu, fmt, b, emu->scratch, R_EAX, 2);
    PREPARE_VALIST;
    return my->g_string_append_vprintf(string, fmt, VARARGS);
}

EXPORT void my_g_string_append_vprintf(x64emu_t* emu, void* string, void* fmt, x64_va_list_t V)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;
    CONVERT_VALIST(V);
    return my->g_string_append_vprintf(string, fmt, VARARGS);
}

EXPORT void my_g_string_printf(x64emu_t* emu, void* string, void* fmt, uintptr_t* b)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;
    myStackAlign(emu, fmt, b, emu->scratch, R_EAX, 2);
    PREPARE_VALIST;
    return my->g_string_vprintf(string, fmt, VARARGS);
}

EXPORT void my_g_string_vprintf(x64emu_t* emu, void* string, void* fmt, x64_va_list_t V)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;
    CONVERT_VALIST(V);
    return my->g_string_vprintf(string, fmt, VARARGS);
}

EXPORT void* my_g_strjoin(x64emu_t* emu, void* a, uintptr_t* b)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;
    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 1);
    return my->g_strjoinv(a, VARARGS);
}

EXPORT void* my_g_strjoinv(x64emu_t* emu, void* a, x64_va_list_t V)
{
    glib2_my_t *my = (glib2_my_t*)my_lib->priv.w.p2;
    CONVERT_VALIST(V);
    return my->g_strjoinv(a, VARARGS);
}


#define PRE_INIT    \
    if(box64_nogtk) \
        return -1;

#define CUSTOM_INIT \
    libname = lib->name;\
    my_lib = lib;       \
    lib->priv.w.p2 = getGlib2My(lib);

#define CUSTOM_FINI \
    freeGlib2My(lib->priv.w.p2);\
    free(lib->priv.w.p2);       \
    my_lib = NULL;

#include "wrappedlib_init.h"

