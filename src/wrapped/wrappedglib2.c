#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <stdarg.h>

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
#define ALTNAME "libglib-2.0.so"

#define LIBNAME glib2

typedef void  (*vFppip_t)(void*, void*, int, void*);

#ifdef HAVE_LD80LIBS
#define ADDED_FUNCTIONS_2()
#else
typedef void (*vFppippDpDC_t)(void*, void*, int32_t, void*, void*, double, void*, double, uint8_t);
#define ADDED_FUNCTIONS_2() \
    GO(g_assertion_message_cmpnum, vFppippDpDC_t)
#endif

#define ADDED_FUNCTIONS() \
    GO(g_build_filenamev, pFp_t)                \
    GO(g_variant_get_va, vFpppp_t)              \
    GO(g_build_pathv, pFpp_t)                   \
    GO(g_set_error_literal, vFppip_t)           \
    GO(g_variant_builder_add_value, vFpp_t)     \
    ADDED_FUNCTIONS_2()

#include "wrappedglib2types.h"

#include "wrappercallback.h"

typedef int (*GSourceFunc) (void* user_data);

typedef struct my_GSourceFuncs_s {
  int  (*prepare)  (void* source, int* timeout_);
  int  (*check)    (void* source);
  int  (*dispatch) (void* source, GSourceFunc callback,void* user_data);
  void (*finalize) (void* source);
  GSourceFunc closure;
  void* marshal;
} my_GSourceFuncs_t;

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)   \
GO(5)   \
GO(6)   \
GO(7)   \
GO(8)   \
GO(9)   \
GO(10)  \
GO(11)  \
GO(12)  \
GO(13)  \
GO(14)  \

// GCopyFct
#define GO(A)   \
static uintptr_t my_copy_fct_##A = 0;                                     \
static void* my_copy_##A(void* data)                                      \
{                                                                         \
    return (void*)RunFunctionFmt(my_copy_fct_##A, "p", data); \
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
}
// GFreeFct
#define GO(A)   \
static uintptr_t my_free_fct_##A = 0;                       \
static void my_free_##A(void* data)                         \
{                                                           \
    RunFunctionFmt(my_free_fct_##A, "p", data); \
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
static uintptr_t my_duplicate_fct_##A = 0;                                     \
static void* my_duplicate_##A(void* data)                                      \
{                                                                              \
    return (void*)RunFunctionFmt(my_duplicate_fct_##A, "p", data); \
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
#define GO(A)   \
static my_GSourceFuncs_t     my_gsourcefuncs_##A = {0};   \
static my_GSourceFuncs_t   *ref_gsourcefuncs_##A = NULL;
SUPER()
#undef GO
// then the static functions callback that may be used with the structure, but dispatch also have a callback...
#define GO(A)   \
static int my_funcs_prepare_##A(void* source, int *timeout_) {                                      \
    return (int)RunFunctionFmt((uintptr_t)ref_gsourcefuncs_##A->prepare, "pp", source, timeout_);   \
}                                                                                                   \
static uintptr_t fct_funcs_check_##A = 0;                                                           \
static int my_funcs_check_##A(void* source) {                                                       \
    return (int)RunFunctionFmt((uintptr_t)ref_gsourcefuncs_##A->check, "p", source);                \
}                                                                                                   \
static uintptr_t fct_funcs_dispatch_cb_##A = 0;                                                     \
static int my_funcs_dispatch_cb_##A(void* a, void* b, void* c, void* d) {                           \
    return (int)RunFunctionFmt(fct_funcs_dispatch_cb_##A, "pppp", a, b, c, d);                      \
}                                                                                                   \
static uintptr_t fct_funcs_dispatch_##A = 0;                                                        \
static int my_funcs_dispatch_##A(void* source, void* cb, void* data) {                              \
    uintptr_t old = fct_funcs_dispatch_cb_##A;                                                      \
    fct_funcs_dispatch_cb_##A = (uintptr_t)cb;                                                      \
    return (int)RunFunctionFmt((uintptr_t)ref_gsourcefuncs_##A->dispatch, "ppp", source, cb?my_funcs_dispatch_cb_##A:NULL, data); \
    fct_funcs_dispatch_cb_##A = old;                                                                \
}                                                                                                   \
static uintptr_t fct_funcs_finalize_##A = 0;                                                        \
static int my_funcs_finalize_##A(void* source) {                                                    \
    return (int)RunFunctionFmt((uintptr_t)ref_gsourcefuncs_##A->finalize, "p", source);             \
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
    #define GO(A) if(ref_gsourcefuncs_##A == 0) {                                       \
        ref_gsourcefuncs_##A = fcts;                                                    \
        my_gsourcefuncs_##A.closure = fcts->closure;                                    \
        my_gsourcefuncs_##A.marshal = fcts->marshal;                                    \
        my_gsourcefuncs_##A.prepare = (fcts->prepare)?GetNativeOrAlt(fcts->prepare, my_funcs_prepare_##A):NULL;     \
        my_gsourcefuncs_##A.check = (fcts->check)?GetNativeOrAlt(fcts->check, my_funcs_check_##A):NULL;             \
        my_gsourcefuncs_##A.dispatch = (fcts->dispatch)?GetNativeOrAlt(fcts->dispatch, my_funcs_dispatch_##A):NULL; \
        my_gsourcefuncs_##A.finalize = (fcts->finalize)?GetNativeOrAlt(fcts->finalize, my_funcs_finalize_##A):NULL; \
        return &my_gsourcefuncs_##A;                \
    }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GSourceFuncs callback\n");
    return NULL;
}

// PollFunc ...
#define GO(A)   \
static uintptr_t my_poll_fct_##A = 0;                                                \
static int my_poll_##A(void* ufds, uint32_t nfsd, int32_t timeout_)                  \
{                                                                                    \
    return RunFunctionFmt(my_poll_fct_##A, "pui", ufds, nfsd, timeout_); \
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
    return (void*)AddCheckBridge(my_lib->w.bridge, iFpui, fct, 0, "GPollFunc");
}

// GHashFunc ...
#define GO(A)   \
static uintptr_t my_hashfunc_fct_##A = 0;                                       \
static uint32_t my_hashfunc_##A(void* key)                                      \
{                                                                               \
    return (uint32_t)RunFunctionFmt(my_hashfunc_fct_##A, "p", key); \
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
static uintptr_t my_equalfunc_fct_##A = 0;                               \
static int my_equalfunc_##A(void* a, void* b)                            \
{                                                                        \
    return RunFunctionFmt(my_equalfunc_fct_##A, "pp", a, b); \
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
static uintptr_t my_destroyfunc_fct_##A = 0;                               \
static int my_destroyfunc_##A(void* a, void* b)                            \
{                                                                          \
    return RunFunctionFmt(my_destroyfunc_fct_##A, "pp", a, b); \
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
static uintptr_t my_spwnchildsetup_fct_##A = 0;                       \
static void my_spwnchildsetup_##A(void* data)                         \
{                                                                     \
    RunFunctionFmt(my_spwnchildsetup_fct_##A, "p", data); \
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
static uintptr_t my_GSourceFunc_fct_##A = 0;                                \
static void my_GSourceFunc_##A(void* a, void* b, void* c, void* d)          \
{                                                                           \
    RunFunctionFmt(my_GSourceFunc_fct_##A, "pppp", a, b, c, d); \
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
static uintptr_t my_GCompareFunc_fct_##A = 0;                                    \
static int my_GCompareFunc_##A(void* a, void* b)                                 \
{                                                                                \
    return (int)RunFunctionFmt(my_GCompareFunc_fct_##A, "pp", a, b); \
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
static uintptr_t my_GCompareDataFunc_fct_##A = 0;                                           \
static int my_GCompareDataFunc_##A(void* a, void* b, void* data)                            \
{                                                                                           \
    return (int)RunFunctionFmt(my_GCompareDataFunc_fct_##A, "ppp", a, b, data); \
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
static uintptr_t my_GCompletionFunc_fct_##A = 0;                                  \
static void* my_GCompletionFunc_##A(void* a)                                      \
{                                                                                 \
    return (void*)RunFunctionFmt(my_GCompletionFunc_fct_##A, "p", a); \
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
static uintptr_t my_GCompletionStrncmpFunc_fct_##A = 0;                                        \
static int my_GCompletionStrncmpFunc_##A(void* a, void* b, unsigned long n)                    \
{                                                                                              \
    return (int)RunFunctionFmt(my_GCompletionStrncmpFunc_fct_##A, "ppL", a, b, n); \
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
static uintptr_t my_GIOFunc_fct_##A = 0;                                        \
static int my_GIOFunc_##A(void* a, int b, void* c)                              \
{                                                                               \
    return (int)RunFunctionFmt(my_GIOFunc_fct_##A, "pip", a, b, c); \
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
static uintptr_t my_GDestroyNotify_fct_##A = 0;                    \
static void my_GDestroyNotify_##A(void* a)                         \
{                                                                  \
    RunFunctionFmt(my_GDestroyNotify_fct_##A, "p", a); \
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
static uintptr_t my_GFunc_fct_##A = 0;                        \
static void my_GFunc_##A(void* a, void* b)                    \
{                                                             \
    RunFunctionFmt(my_GFunc_fct_##A, "pp", a, b); \
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
static uintptr_t my_GHFunc_fct_##A = 0;                            \
static void my_GHFunc_##A(void* a, void* b, void* c)               \
{                                                                  \
    RunFunctionFmt(my_GHFunc_fct_##A, "ppp", a, b, c); \
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
static uintptr_t my_GHRFunc_fct_##A = 0;                                 \
static int my_GHRFunc_##A(void* a, void* b, void* c)                     \
{                                                                        \
    return RunFunctionFmt(my_GHRFunc_fct_##A, "ppp", a, b, c); \
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
static uintptr_t my_GChildWatchFunc_fct_##A = 0;                            \
static void my_GChildWatchFunc_##A(int a, int b, void* c)                   \
{                                                                           \
    RunFunctionFmt(my_GChildWatchFunc_fct_##A, "iip", a, b, c); \
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
static uintptr_t my_GLogFunc_fct_##A = 0;                                \
static void my_GLogFunc_##A(void* a, int b, void* c, void* d)            \
{                                                                        \
    RunFunctionFmt(my_GLogFunc_fct_##A, "pipp", a, b, c, d); \
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
    return (void*)AddCheckBridge(my_lib->w.bridge, vFpipp, fct, 0, "GLogFunc");
}
// GPrintFunc ...
#define GO(A)   \
static uintptr_t my_GPrintFunc_fct_##A = 0;                    \
static void my_GPrintFunc_##A(void* a)                         \
{                                                              \
    RunFunctionFmt(my_GPrintFunc_fct_##A, "p", a); \
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
// GOptionArg ...
#define GO(A)   \
static uintptr_t my_GOptionArg_fct_##A = 0;                                            \
static int my_GOptionArg_##A(void* a, void* b, void* c, void* d)                       \
{                                                                                      \
    return (int)RunFunctionFmt(my_GOptionArg_fct_##A, "pppp", a, b, c, d); \
}
SUPER()
#undef GO
static void* findGOptionArgFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GOptionArg_fct_##A == (uintptr_t)fct) return my_GOptionArg_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GOptionArg_fct_##A == 0) {my_GOptionArg_fct_##A = (uintptr_t)fct; return my_GOptionArg_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GOptionArg callback\n");
    return NULL;
}
static void* reverseGOptionArgFct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if((uintptr_t)fct == my_GOptionArg_fct_##A) return (void*)my_GOptionArg_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddCheckBridge(my_lib->w.bridge, iFpppp, fct, 0, "GOptionArgFunc");
}
// GOptionParse ...
#define GO(A)   \
static uintptr_t my_GOptionParse_fct_##A = 0;                                            \
static int my_GOptionParse_##A(void* a, void* b, void* c, void* d)                       \
{                                                                                      \
    return (int)RunFunctionFmt(my_GOptionParse_fct_##A, "pppp", a, b, c, d); \
}
SUPER()
#undef GO
static void* findGOptionParseFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GOptionParse_fct_##A == (uintptr_t)fct) return my_GOptionParse_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GOptionParse_fct_##A == 0) {my_GOptionParse_fct_##A = (uintptr_t)fct; return my_GOptionParse_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GOptionParse callback\n");
    return NULL;
}
// GNodeTraverseFunc ...
#define GO(A)   \
static uintptr_t my_GNodeTraverseFunc_fct_##A = 0;                                    \
static int my_GNodeTraverseFunc_##A(void* a, void* b)                                 \
{                                                                                     \
    return (int)RunFunctionFmt(my_GNodeTraverseFunc_fct_##A, "pp", a, b); \
}
SUPER()
#undef GO
static void* findGNodeTraverseFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GNodeTraverseFunc_fct_##A == (uintptr_t)fct) return my_GNodeTraverseFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GNodeTraverseFunc_fct_##A == 0) {my_GNodeTraverseFunc_fct_##A = (uintptr_t)fct; return my_GNodeTraverseFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GNodeTraverseFunc callback\n");
    return NULL;
}
// GThreadFunc ...
#define GO(A)   \
static uintptr_t my_GThreadFunc_fct_##A = 0;                                  \
static void* my_GThreadFunc_##A(void* a)                                      \
{                                                                             \
    return (void*)RunFunctionFmt(my_GThreadFunc_fct_##A, "p", a); \
}
SUPER()
#undef GO
static void* findGThreadFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GThreadFunc_fct_##A == (uintptr_t)fct) return my_GThreadFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GThreadFunc_fct_##A == 0) {my_GThreadFunc_fct_##A = (uintptr_t)fct; return my_GThreadFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GThreadFunc callback\n");
    return NULL;
}
// TimeOut
#define GO(A)   \
static uintptr_t my_TimeOut_fct_##A = 0;            \
static void my_TimeOut_##A(void* a)                 \
{                                                   \
    RunFunctionFmt(my_TimeOut_fct_##A, "p", a);     \
}
SUPER()
#undef GO
static void* findTimeOutFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_TimeOut_fct_##A == (uintptr_t)fct) return my_TimeOut_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_TimeOut_fct_##A == 0) {my_TimeOut_fct_##A = (uintptr_t)fct; return my_TimeOut_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 TimeOut callback\n");
    return NULL;
}
// GTraverseFunc ...
#define GO(A)   \
static uintptr_t my_GTraverseFunc_fct_##A = 0;                              \
static int my_GTraverseFunc_##A(void* a, void* b, void* c)                  \
{                                                                           \
    return (int)RunFunctionFmt(my_GTraverseFunc_fct_##A, "ppp", a, b, c);   \
}
SUPER()
#undef GO
static void* findGTraverseFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GTraverseFunc_fct_##A == (uintptr_t)fct) return my_GTraverseFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GTraverseFunc_fct_##A == 0) {my_GTraverseFunc_fct_##A = (uintptr_t)fct; return my_GTraverseFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GTraverseFunc callback\n");
    return NULL;
}

#undef SUPER

EXPORT void* my_g_markup_vprintf_escaped(x64emu_t *emu, void* fmt, void* b) {
    // need to align on arm
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 1);
    PREPARE_VALIST;
    return my->g_markup_vprintf_escaped(fmt, VARARGS);
}

EXPORT void* my_g_build_filename(x64emu_t* emu, void* first, uintptr_t* b)
{
    int n = 0;
    while (getVArgs(emu, 1, b, n++));
    void* array[n+1];   // +1 for 1st (NULL terminal already included)
    array[0] = first;
    for(int i=0; i<n; ++i)
        array[i+1] = (void*)getVArgs(emu, 1, b, i);
    void* ret = my->g_build_filenamev(array);
    return ret;
}

EXPORT uint32_t my_g_timeout_add(x64emu_t* emu, uint32_t interval, void* func, void* data)
{
    return my->g_timeout_add(interval, findTimeOutFct(func), data);
}

EXPORT void my_g_list_free_full(x64emu_t* emu, void* list, void* free_func)
{
    my->g_list_free_full(list, findFreeFct(free_func));
}

EXPORT void* my_g_markup_printf_escaped(x64emu_t *emu, void* fmt, void* b) {
    // need to align on arm
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 1);
    PREPARE_VALIST;
    return my->g_markup_vprintf_escaped(fmt, VARARGS);
}


EXPORT void my_g_datalist_id_set_data_full(x64emu_t* emu, void* datalist, uintptr_t key, void* data, void* freecb)
{
    void* fc = findFreeFct(freecb);
    my->g_datalist_id_set_data_full(datalist, key, data, fc);
}

EXPORT void* my_g_datalist_id_dup_data(x64emu_t* emu, void* datalist, uintptr_t key, void* dupcb, void* data)
{
    void* cc = findDuplicateFct(dupcb);
    return my->g_datalist_id_dup_data(datalist, key, cc, data);
}

EXPORT int my_g_datalist_id_replace_data(x64emu_t* emu, void* datalist, uintptr_t key, void* oldval, void* newval, void* oldfree, void* newfree)
{
    void* oldfc = findFreeFct(oldfree);
    void* newfc = findFreeFct(newfree);
    return my->g_datalist_id_replace_data(datalist, key, oldval, newval, oldfc, newfc);
}

EXPORT void* my_g_variant_new_from_data(x64emu_t* emu, void* type, void* data, size_t size, int trusted, void* freecb, void* datacb)
{
    void* fc = findFreeFct(freecb);
    return my->g_variant_new_from_data(type, data, size, trusted, fc, datacb);
}

EXPORT void* my_g_variant_new_parsed_va(x64emu_t* emu, void* fmt, x64_va_list_t b)
{
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(b);
    #else
    CREATE_VALIST_FROM_VALIST(b, emu->scratch);
    #endif
    return my->g_variant_new_parsed_va(fmt, &VARARGS);
}

EXPORT void my_g_variant_get(x64emu_t* emu, void* value, void* fmt, uint64_t* V)
{
    CREATE_VALIST_FROM_VAARG(V, emu->scratch, 2);
    my->g_variant_get_va(value, fmt, NULL, &VARARGS);
}

EXPORT void* my_g_strdup_vprintf(x64emu_t* emu, void* fmt, void* b)
{
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 1);
    PREPARE_VALIST;
    return my->g_strdup_vprintf(fmt, VARARGS);
}

EXPORT int my_g_vprintf(x64emu_t* emu, void* fmt, void* b)
{
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 1);
    PREPARE_VALIST;
    return my->g_vprintf(fmt, VARARGS);
}

EXPORT int my_g_vfprintf(x64emu_t* emu, void* F, void* fmt, void* b)
{
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 2);
    PREPARE_VALIST;
    return my->g_vfprintf(F, fmt, VARARGS);
}

EXPORT int my_g_vsprintf(x64emu_t* emu, void* s, void* fmt, void* b)
{
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 2);
    PREPARE_VALIST;
    return my->g_vsprintf(s, fmt, VARARGS);
}

EXPORT int my_g_vsnprintf(x64emu_t* emu, void* s, unsigned long n, void* fmt, void* b)
{
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 3);
    PREPARE_VALIST;
    return my->g_vsnprintf(s, n, fmt, VARARGS);
}

EXPORT int my_g_vasprintf(x64emu_t* emu, void* s, void* fmt, void* b)
{
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 2);
    PREPARE_VALIST;
    return my->g_vasprintf(s, fmt, VARARGS);
}

EXPORT uint32_t my_g_printf_string_upper_bound(x64emu_t* emu, void* fmt, void* b)
{
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 1);
    PREPARE_VALIST;
    return my->g_printf_string_upper_bound(fmt, VARARGS);
}

EXPORT void my_g_print(x64emu_t* emu, void* fmt, void* b)
{
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
    char* buf = NULL;
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 1);
    PREPARE_VALIST;
    int ret = vasprintf(&buf, fmt, VARARGS);
    (void)ret;
    my->g_printerr(buf);
    free(buf);
}

EXPORT void* my_g_source_new(x64emu_t* emu, my_GSourceFuncs_t* source_funcs, size_t struct_size)
{
    return my->g_source_new(findFreeGSourceFuncs(source_funcs), struct_size);
}

EXPORT void my_g_source_set_funcs(x64emu_t* emu, void* source, my_GSourceFuncs_t* source_funcs)
{

    my->g_source_set_funcs(source, findFreeGSourceFuncs(source_funcs));
}


EXPORT int my_g_source_remove_by_funcs_user_data(x64emu_t* emu, my_GSourceFuncs_t* source_funcs, void* data)
{
    return my->g_source_remove_by_funcs_user_data(findFreeGSourceFuncs(source_funcs), data);
}

EXPORT void* my_g_main_context_get_poll_func(x64emu_t* emu, void* context)
{

    void* ret = my->g_main_context_get_poll_func(context);
    if(!ret) return ret;
    void* r = reversePollFct(ret);
    if(r) return r;
    // needs to bridge....
    return (void*)AddCheckBridge(my_lib->w.bridge, iFpui, ret, 0, NULL);
}

EXPORT void my_g_main_context_set_poll_func(x64emu_t* emu, void* context, void* func)
{
    my->g_main_context_set_poll_func(context, findPollFct(func));
}

EXPORT uint32_t my_g_idle_add_full(x64emu_t* emu, int priority, void* f, void* data, void* notify)
{
    return my->g_idle_add_full(priority, findTimeOutFct(f), data, findGDestroyNotifyFct(notify));
}

EXPORT void* my_g_hash_table_new(x64emu_t* emu, void* hash, void* equal)
{
    return my->g_hash_table_new(findHashFct(hash), findEqualFct(equal));
}

EXPORT void* my_g_hash_table_new_full(x64emu_t* emu, void* hash, void* equal, void* destroy_key, void* destroy_val)
{
    return my->g_hash_table_new_full(findHashFct(hash), findEqualFct(equal), findDestroyFct(destroy_key), findDestroyFct(destroy_val));
}

EXPORT void my_g_hash_table_foreach(x64emu_t* emu, void* table, void* f, void* data)
{
    my->g_hash_table_foreach(table, findGHFuncFct(f), data);
}

EXPORT uint32_t my_g_hash_table_foreach_remove(x64emu_t* emu, void* table, void* f, void* data)
{

    return my->g_hash_table_foreach_remove(table, findGHRFuncFct(f), data);
}
EXPORT uint32_t my_g_hash_table_foreach_steal(x64emu_t* emu, void* table, void* f, void* data)
{

    return my->g_hash_table_foreach_steal(table, findGHRFuncFct(f), data);
}
EXPORT void* my_g_hash_table_find(x64emu_t* emu, void* table, void* f, void* data)
{

    return my->g_hash_table_find(table, findGHRFuncFct(f), data);
}

EXPORT int my_g_spawn_async_with_pipes(x64emu_t* emu, void* dir, void* argv, void* envp, uint32_t flags, void* f, void* data, void* child, void* input, void* output, void* err, void* error)
{
    return my->g_spawn_async_with_pipes(dir, argv, envp, flags, findSpawnChildSetupFct(f), data, child, input, output, err, error);
}

EXPORT int my_g_spawn_async(x64emu_t* emu, void* dir, void* argv, void* envp, uint32_t flags, void* f, void* data, void* child, void* error)
{
    return my->g_spawn_async(dir, argv, envp, flags, findSpawnChildSetupFct(f), data, child, error);
}

EXPORT int my_g_spawn_sync(x64emu_t* emu, void* dir, void* argv, void* envp, uint32_t flags, void* f, void* data, void* input, void* output, void* status, void* error)
{
    return my->g_spawn_sync(dir, argv, envp, flags, findSpawnChildSetupFct(f), data, input, output, status, error);
}

EXPORT uint32_t my_g_child_watch_add(x64emu_t* emu, int pid, void* f, void* data)
{
    return my->g_child_watch_add(pid, findGChildWatchFuncFct(f), data);
}

EXPORT uint32_t my_g_child_watch_add_full(x64emu_t* emu, int priority, int pid, void* f, void* data, void* notify)
{
    return my->g_idle_add_full(priority, findGChildWatchFuncFct(f), data, findGDestroyNotifyFct(notify));
}

EXPORT void* my_g_private_new(x64emu_t* emu, void* notify)
{
    return my->g_private_new(findFreeFct(notify));
}

EXPORT void my_g_static_private_set(x64emu_t* emu, void* private, void* data, void* notify)
{
    my->g_static_private_set(private, data, findFreeFct(notify));
}

EXPORT void* my_g_ptr_array_new_with_free_func(x64emu_t* emu, void* notify)
{
    return my->g_ptr_array_new_with_free_func(findFreeFct(notify));
}

EXPORT void* my_g_ptr_array_new_full(x64emu_t* emu, uint32_t size, void* notify)
{
    return my->g_ptr_array_new_full(size, findFreeFct(notify));
}

EXPORT void my_g_ptr_array_set_free_func(x64emu_t* emu, void* array, void* notify)
{
    my->g_ptr_array_set_free_func(array, findFreeFct(notify));
}

EXPORT void my_g_ptr_array_sort(x64emu_t* emu, void* array, void* comp)
{
    my->g_ptr_array_sort(array, findGCompareFuncFct(comp));
}

EXPORT void my_g_ptr_array_sort_with_data(x64emu_t* emu, void* array, void* comp, void* data)
{
    my->g_ptr_array_sort_with_data(array, findGCompareDataFuncFct(comp), data);
}

EXPORT void my_g_qsort_with_data(x64emu_t* emu, void* pbase, int total, unsigned long size, void* comp, void* data)
{
    my->g_qsort_with_data(pbase, total, size, findGCompareDataFuncFct(comp), data);
}

EXPORT void my_g_ptr_array_foreach(x64emu_t* emu, void* array, void* func, void* data)
{
    my->g_ptr_array_foreach(array, findGFuncFct(func), data);
}

EXPORT void* my_g_thread_create(x64emu_t* emu, void* func, void* data, int joinable, void* error)
{
    void* et = NULL;
    return my->g_thread_create(my_prepare_thread(emu, func, data, 0, &et), et, joinable, error);
}

EXPORT void* my_g_thread_create_full(x64emu_t* emu, void* func, void* data, unsigned long stack, int joinable, int bound, uint32_t priority, void* error)
{
    void* et = NULL;
    return my->g_thread_create_full(my_prepare_thread(emu, func, data, stack, &et), et, stack, joinable, bound, priority, error);
}

EXPORT void my_g_thread_foreach(x64emu_t* emu, void* func, void* data)
{
    my->g_thread_foreach(findGFuncFct(func), data);
}

EXPORT void my_g_array_sort(x64emu_t* emu, void* array, void* comp)
{
    my->g_array_sort(array, findGCompareFuncFct(comp));
}

EXPORT void my_g_array_sort_with_data(x64emu_t* emu, void* array, void* comp, void* data)
{
    my->g_array_sort_with_data(array, findGCompareDataFuncFct(comp), data);
}

EXPORT void my_g_array_set_clear_func(x64emu_t* emu, void* array, void* notify)
{
    my->g_array_set_clear_func(array, findFreeFct(notify));
}

EXPORT void my_g_source_set_callback(x64emu_t* emu, void* source, void* func, void* data, void* notify)
{
    my->g_source_set_callback(source, findGSourceFuncFct(func), data, findFreeFct(notify));
}

EXPORT void my_g_main_context_invoke(x64emu_t* emu, void* context, void* func, void* data)
{
    my->g_main_context_invoke(context, findGSourceFuncFct(func), data);
}

EXPORT void* my_g_slist_insert_sorted(x64emu_t* emu, void* list, void* d, void* comp)
{

    return my->g_slist_insert_sorted(list, d, findGCompareFuncFct(comp));
}
EXPORT void* my_g_slist_insert_sorted_with_data(x64emu_t* emu, void* list, void* d, void* comp, void* data)
{
    return my->g_slist_insert_sorted_with_data(list, d, findGCompareDataFuncFct(comp), data);
}

EXPORT void my_g_slist_foreach(x64emu_t* emu, void* list, void* func, void* data)
{
    my->g_slist_foreach(list, findGFuncFct(func), data);
}

EXPORT void* my_g_slist_find_custom(x64emu_t* emu, void* list, void* data, void* comp)
{
    return my->g_slist_find_custom(list, data, findGCompareFuncFct(comp));
}

EXPORT uint32_t my_g_idle_add(x64emu_t* emu, void* func, void* data)
{
    return my->g_idle_add(findGSourceFuncFct(func), data);
}

EXPORT void* my_g_variant_new_va(x64emu_t* emu, char* fmt, void* endptr, x64_va_list_t* b)
{
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(*b);
    #else
      #if defined(__loongarch64) || defined(__riscv)
        va_list sysv_varargs;
        myStackAlignGVariantNewVa(emu, fmt, emu->scratch, b);
        sysv_varargs = (va_list)emu->scratch;
      #else
        CREATE_VALIST_FROM_VALIST(*b, emu->scratch);
      #endif
    #endif
    return my->g_variant_new_va(fmt, endptr, &sysv_varargs);
}

EXPORT void* my_g_variant_new(x64emu_t* emu, char* fmt, uint64_t* V)
{
#if defined(__loongarch64) || defined(__riscv)
    myStackAlignGVariantNew(emu, fmt, V, emu->scratch, R_EAX);
    PREPARE_VALIST;
#else
    CREATE_VALIST_FROM_VAARG(V, emu->scratch, 1);
#endif
    return my->g_variant_new_va(fmt, NULL, &VARARGS);
}

EXPORT  void my_g_variant_builder_add(x64emu_t* emu, void* builder, void* fmt, uint64_t* V)
{
    // equivalent to calling g_variant_new and g_variant_builder_add_value
    CREATE_VALIST_FROM_VAARG(V, emu->scratch, 2);
    void* val = my->g_variant_new_va(fmt, NULL, &VARARGS);
    my->g_variant_builder_add_value(builder, val);
}

EXPORT void* my_g_completion_new(x64emu_t* emu, void* f)
{
    return my->g_completion_new(findGCompletionFct(f));
}

EXPORT void my_g_completion_set_compare(x64emu_t *emu, void* cmp, void* f)
{
    my->g_completion_set_compare(cmp, findGCompletionStrncmpFuncFct(f));
}

EXPORT void* my_g_log_set_default_handler(x64emu_t *emu, void* f, void* data)
{
    return reverseGLogFuncFct(my->g_log_set_default_handler(findGLogFuncFct(f), data));
}

EXPORT uint32_t my_g_io_add_watch_full(x64emu_t* emu, void* channel, int priority, uint32_t cond, void* f, void* data, void* notify)
{
    return my->g_io_add_watch_full(channel, priority, cond, findGIOFuncFct(f), data, findDestroyFct(notify));
}

EXPORT uint32_t my_g_io_add_watch(x64emu_t* emu, void* channel, uint32_t cond, void* f, void* data)
{
    return my->g_io_add_watch(channel, cond, findGIOFuncFct(f), data);
}

EXPORT void* my_g_set_print_handler(x64emu_t *emu, void* f)
{
    return reverseGPrintFuncFct(my->g_set_print_handler(findGPrintFuncFct(f)));
}

EXPORT void* my_g_set_printerr_handler(x64emu_t *emu, void* f)
{
    return reverseGPrintFuncFct(my->g_set_printerr_handler(findGPrintFuncFct(f)));
}

EXPORT void* my_g_slist_sort(x64emu_t *emu, void* list, void* f)
{
    return my->g_slist_sort(list, findGCompareFuncFct(f));
}

EXPORT void* my_g_slist_sort_with_data(x64emu_t *emu, void* list, void* f, void* data)
{
    return my->g_slist_sort_with_data(list, findGCompareDataFuncFct(f), data);
}

EXPORT void* my_g_build_path(x64emu_t *emu, void* sep, void* first, uintptr_t* data)
{
    int n = (first)?1:0;
    void* p = n?((void*)getVArgs(emu, 2, data, 0)):NULL;
    while(p) {
        p = (void*)getVArgs(emu, 2, data, n++);
    }
    ++n;    // final NULL
    void** args = (void**)box_malloc((n+1) *sizeof(void*));
    args[0] = first;
    for(int i=0; i<n; ++i)
        args[i+1] = (void*)getVArgs(emu, 2, data, i);
    p = my->g_build_pathv(sep, args);
    box_free(args);
    return p;
}

EXPORT void* my_g_list_sort(x64emu_t *emu, void* list, void* f)
{
    return my->g_list_sort(list, findGCompareFuncFct(f));
}

EXPORT void* my_g_list_sort_with_data(x64emu_t *emu, void* list, void* f, void* data)
{
    return my->g_list_sort_with_data(list, findGCompareDataFuncFct(f), data);
}

EXPORT void* my_g_queue_find_custom(x64emu_t *emu, void* queue, void* data, void* f)
{
    return my->g_queue_find_custom(queue, data, findGCompareFuncFct(f));
}

EXPORT void* my_g_list_find_custom(x64emu_t *emu, void* list, void* data, void* f)
{
    return my->g_list_find_custom(list, data, findGCompareFuncFct(f));
}

EXPORT uint32_t my_g_timeout_add_full(x64emu_t *emu, int priority, uint32_t interval, void* f, void* data, void* notify)
{
    return my->g_timeout_add_full(priority, interval, findGSourceFuncFct(f), data, findDestroyFct(notify));
}

EXPORT uint32_t my_g_timeout_add_seconds(x64emu_t *emu, uint32_t interval, void* f, void* data)
{
    return my->g_timeout_add_seconds(interval, findGSourceFuncFct(f), data);
}

EXPORT uint32_t my_g_timeout_add_seconds_full(x64emu_t *emu, int priority, uint32_t interval, void* f, void* data, void* notify)
{
    return my->g_timeout_add_seconds_full(priority, interval, findGSourceFuncFct(f), data, findDestroyFct(notify));
}

EXPORT uint32_t my_g_log_set_handler(x64emu_t *emu, void* domain, int level, void* f, void* data)
{
    return my->g_log_set_handler(domain, level, findGLogFuncFct(f), data);
}

EXPORT void my_g_set_error(x64emu_t *emu, void* err, void* domain, uint32_t code, void* fmt, uintptr_t* stack)
{
    char buf[1000];
    myStackAlign(emu, fmt, stack, emu->scratch, R_EAX, 4);
    PREPARE_VALIST;
    vsnprintf(buf, sizeof(buf), fmt, VARARGS);
    my->g_set_error_literal(err, domain, code, buf);
}

EXPORT void* my_g_error_new(x64emu_t* emu, uint32_t domain, int code, void* fmt, uintptr_t* b)
{
    myStackAlign(emu, fmt, b, emu->scratch, R_EAX, 3);
    PREPARE_VALIST;
    return my->g_error_new_valist(domain, code, fmt, VARARGS);
}
EXPORT void* my_g_error_new_valist(x64emu_t* emu, uint32_t domain, int code, void* fmt, x64_va_list_t V)
{
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(V);
    #else
    CREATE_VALIST_FROM_VALIST(V, emu->scratch);
    #endif
    return my->g_error_new_valist(domain, code, fmt, VARARGS);
}

EXPORT int my_g_fprintf(x64emu_t* emu, void* f, void* fmt, uintptr_t* b)
{
    myStackAlign(emu, fmt, b, emu->scratch, R_EAX, 3);
    PREPARE_VALIST;
    return my->g_vfprintf(f, fmt, VARARGS);
}

EXPORT void my_g_logv(x64emu_t* emu, void* domain, int level, void* fmt, x64_va_list_t V)
{
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(V);
    #else
    CREATE_VALIST_FROM_VALIST(V, emu->scratch);
    #endif
    my->g_logv(domain, level, fmt, VARARGS);
}
EXPORT void my_g_log(x64emu_t* emu, void* domain, int level, void* fmt, uintptr_t* b)
{
    myStackAlign(emu, fmt, b, emu->scratch, R_EAX, 3);
    PREPARE_VALIST;
    my->g_logv(domain, level, fmt, VARARGS);
}

EXPORT int my_g_printf(x64emu_t* emu, void* fmt, uintptr_t* b)
{
    myStackAlign(emu, fmt, b, emu->scratch, R_EAX, 1);
    PREPARE_VALIST;
    return my->g_vprintf(fmt, VARARGS);
}

EXPORT int my_g_snprintf(x64emu_t* emu, void* buf, size_t l, void* fmt, uintptr_t* b)
{
    myStackAlign(emu, fmt, b, emu->scratch, R_EAX, 3);
    PREPARE_VALIST;
    return my->g_vsnprintf(buf, l, fmt, VARARGS);
}

EXPORT int my_g_sprintf(x64emu_t* emu, void* buf, void* fmt, uintptr_t* b)
{
    myStackAlign(emu, fmt, b, emu->scratch, R_EAX, 2);
    PREPARE_VALIST;
    return my->g_vsprintf(buf, fmt, VARARGS);
}

EXPORT void* my_g_strdup_printf(x64emu_t* emu, void* fmt, uintptr_t* b)
{
    myStackAlign(emu, fmt, b, emu->scratch, R_EAX, 1);
    PREPARE_VALIST;
    return my->g_strdup_vprintf(fmt, VARARGS);
}

EXPORT void my_g_string_append_printf(x64emu_t* emu, void* string, void* fmt, uintptr_t* b)
{
    myStackAlign(emu, fmt, b, emu->scratch, R_EAX, 2);
    PREPARE_VALIST;
    return my->g_string_append_vprintf(string, fmt, VARARGS);
}

EXPORT void my_g_string_append_vprintf(x64emu_t* emu, void* string, void* fmt, x64_va_list_t V)
{
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(V);
    #else
    myStackAlignValist(emu, (const char*)fmt, emu->scratch, V);
    PREPARE_VALIST;
    #endif
    return my->g_string_append_vprintf(string, fmt, VARARGS);
}

EXPORT void my_g_string_printf(x64emu_t* emu, void* string, void* fmt, uintptr_t* b)
{
    myStackAlign(emu, fmt, b, emu->scratch, R_EAX, 2);
    PREPARE_VALIST;
    return my->g_string_vprintf(string, fmt, VARARGS);
}

EXPORT void my_g_string_vprintf(x64emu_t* emu, void* string, void* fmt, x64_va_list_t V)
{
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(V);
    #else
    myStackAlignValist(emu, (const char*)fmt, emu->scratch, V);
    PREPARE_VALIST;
    #endif
    return my->g_string_vprintf(string, fmt, VARARGS);
}

EXPORT void* my_g_strjoin(x64emu_t* emu, void* sep, uintptr_t* data)
{
    int n = 0;
    void* p = (void*)getVArgs(emu, 1, data, 0);
    while(p) {
        p = (void*)getVArgs(emu, 1, data, n++);
    }
    ++n;    // final NULL
    void** args = (void**)box_malloc(n *sizeof(void*));
    for(int i=0; i<n; ++i)
        args[i] = (void*)getVArgs(emu, 1, data, i);
    p = my->g_strjoinv(sep, args);
    box_free(args);
    return p;
}

EXPORT void* my_g_strjoinv(x64emu_t* emu, void* a, void** V)
{
    return my->g_strjoinv(a, V);
}

EXPORT void* my_g_option_group_new(x64emu_t* emu, void* name, void* desc, void* help, void* data, void* destroy)
{
    return my->g_option_group_new(name, desc, help, data, findDestroyFct(destroy));
}

typedef struct my_GOptionEntry_s {
  void*     long_name;
  char      short_name;
  int       flags;
  int       arg;
  void*     arg_data;
  void*     description;
  void*     arg_description;
} my_GOptionEntry_t;

EXPORT void my_g_option_context_add_main_entries(x64emu_t* emu, void* context, my_GOptionEntry_t* entries, void* domain)
{
    my_GOptionEntry_t* p = entries;
    int idx = 0;
    while (p && p->long_name) {
        // wrap Callbacks
        ++p;
        ++idx;
    }
    p = entries;
    my_GOptionEntry_t my_entries[idx+1];
    idx = 0;
    while (p && p->long_name) {
        // wrap Callbacks
        my_entries[idx] = *p;
        if (p->arg == 3)
            my_entries[idx].arg_data = findGOptionArgFct(p->arg_data);
        ++p;
        ++idx;
    }
    if(p) my_entries[idx] = *p;
    my->g_option_context_add_main_entries(context, entries?my_entries:NULL, domain);
}

EXPORT void* my_g_strconcat(x64emu_t* emu, void* first, uintptr_t* data)
{
    int n = (first)?1:0;
    void* p = n?((void*)getVArgs(emu, 1, data, 0)):NULL;
    while(p) {
        p = (void*)getVArgs(emu, 1, data, n++);
    }
    ++n;    // final NULL
    void** args = (void**)box_malloc((n+1) *sizeof(void*));
    args[0] = first;
    for(int i=0; i<n; ++i)
        args[i+1] = (void*)getVArgs(emu, 1, data, i);
    p = my->g_strjoinv(NULL, args);
    box_free(args);
    return p;
}

EXPORT void* my_g_markup_parse_context_new(x64emu_t* emu, void* parser, uint32_t flags, void* data, void* destroy)
{
    return my->g_markup_parse_context_new(parser, flags, data, findDestroyFct(destroy));
}

EXPORT void my_g_list_foreach(x64emu_t* emu, void* list, void* func, void* data)
{
    my->g_list_foreach(list, findGFuncFct(func), data);
}

EXPORT void* my_g_list_insert_sorted(x64emu_t* emu, void* list, void* data, void* f)
{
    return my->g_list_insert_sorted(list, data, findGCompareFuncFct(f));
}

EXPORT void my_g_node_traverse(x64emu_t* emu, void* node, int order, int flags, int depth, void* f, void* data)
{
    my->g_node_traverse(node, order, flags, depth, findGNodeTraverseFuncFct(f), data);
}

EXPORT void* my_g_node_copy_deep(x64emu_t* emu, void* node, void* f, void* data)
{
    return my->g_node_copy_deep(node, findCopyFct(f), data);
}

EXPORT void* my_g_thread_try_new(x64emu_t* emu, void* name, void* f, void* data, void* err)
{
    return my->g_thread_try_new(name, findGThreadFuncFct(f), data, err);
}

EXPORT void my_g_slist_free_full(x64emu_t* emu, void* list, void* f)
{
    my->g_slist_free_full(list, findDestroyFct(f));
}

EXPORT void* my_g_list_insert_sorted_with_data(x64emu_t* emu, void* list, void* data, void* f, void* user)
{
    return my->g_list_insert_sorted_with_data(list, data, findGCompareDataFuncFct(f), user);
}

EXPORT void my_g_option_group_set_parse_hooks(x64emu_t* emu, void* group, void* preparse, void* postparse)
{
    my->g_option_group_set_parse_hooks(group, findGOptionParseFct(preparse), findGOptionParseFct(postparse));
}

EXPORT void* my_g_thread_new(x64emu_t* emu, void* name, void* f, void* data)
{
    return my->g_thread_new(name, findGThreadFuncFct(f), data);
}

EXPORT void my_g_queue_foreach(x64emu_t* emu, void* queue, void* f, void* data)
{
    my->g_queue_foreach(queue, findGFuncFct(f), data);
}

EXPORT void* my_g_once_impl(x64emu_t* emu, void* once, void* f, void* arg)
{
    return my->g_once_impl(once, findGThreadFuncFct(f), arg);
}

EXPORT void* my_g_bytes_new_with_free_func(x64emu_t* emu, void* data, unsigned long n, void* notify, void* user)
{
    return my->g_bytes_new_with_free_func(data, n, findGDestroyNotifyFct(notify), user);
}

#ifndef HAVE_LD80BITS
EXPORT void my_g_assertion_message_cmpnum(void* domain, void* file, int32_t line, void* func, void* expr, double arg1, void* comp, double arg2, uint8_t numtype)
{
    my->g_assertion_message_cmpnum(domain, file, line, func, expr, arg1, comp, arg2, numtype);
}
#endif

EXPORT void* my_g_sequence_new(x64emu_t* emu, void* d)
{
    return my->g_sequence_new(findGDestroyNotifyFct(d));
}

EXPORT void* my_g_sequence_lookup(x64emu_t* emu, void* seq, void* data, void* f, void* cmp_data)
{
    return my->g_sequence_lookup(seq, data, findGCompareDataFuncFct(f), cmp_data);
}

EXPORT void* my_g_sequence_insert_sorted(x64emu_t* emu, void* seq, void* data, void* f, void* cmp_data)
{
    return my->g_sequence_insert_sorted(seq, data, findGCompareDataFuncFct(f), cmp_data);
}

EXPORT void* my_g_tree_new(x64emu_t* emu, void* f)
{
    return my->g_tree_new(findGCompareFuncFct(f));
}

EXPORT void* my_g_tree_new_full(x64emu_t* emu, void* f, void* data, void* d1, void* d2)
{
    return my->g_tree_new_full(findGCompareFuncFct(f), data, findGDestroyNotifyFct(d1), findGDestroyNotifyFct(d2));
}

EXPORT void my_g_tree_foreach(x64emu_t* emu, void* tree, void* f, void* data)
{
    my->g_tree_foreach(tree, findGTraverseFuncFct(f), data);
}

EXPORT void my_g_queue_insert_sorted(x64emu_t* emu, void* queue, void* data, void* f, void* user_data)
{
    my->g_queue_insert_sorted(queue, data, findGCompareDataFuncFct(f), user_data);
}

EXPORT void* my_g_async_queue_new_full(x64emu_t* emu, void* item_free_func)
{
    return my->g_async_queue_new_full(findGDestroyNotifyFct(item_free_func));
}

EXPORT void* my_g_thread_pool_new(x64emu_t* emu, void* func, void* user_data, int32_t max_threads, int32_t exclusive, void* error)
{
    return my->g_thread_pool_new(findGFuncFct(func), user_data, max_threads, exclusive, error);
}

EXPORT void my_g_async_queue_push_sorted(x64emu_t* emu, void* queue, void* data, void* func, void* user_data)
{
    my->g_async_queue_push_sorted(queue, data, findGCompareDataFuncFct(func), user_data);
}

EXPORT void my_g_thread_pool_set_sort_function(x64emu_t* emu, void* pool, void* func, void* user_data)
{
    my->g_thread_pool_set_sort_function(pool, findGCompareDataFuncFct(func), user_data);
}

#define PRE_INIT    \
    if(BOX64ENV(nogtk)) \
        return -1;

#include "wrappedlib_init.h"
