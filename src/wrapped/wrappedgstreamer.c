#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <dirent.h> 

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
#include "fileutils.h"

const char* gstreamerName = "libgstreamer-1.0.so.0";
#define ALTNAME "libgstreamer-1.0.so"

#define LIBNAME gstreamer

typedef void    (*vFv_t)();
typedef void*   (*pFppA_t)(void*, void*, va_list);
typedef void*   (*pFv_t)();
typedef size_t  (*LFv_t)();
typedef void*   (*pFp_t)(void*);
typedef void    (*vFpp_t)(void*, void*);
typedef int     (*iFpp_t)(void*, void*);
typedef void    (*vFpip_t)(void*, int, void*);

void* my_dlopen(x64emu_t* emu, void *filename, int flag);
int my_dlclose(x64emu_t* emu, void *handle);
void* my_dlsym(x64emu_t* emu, void *handle, void *symbol);

#ifndef MAX_PATH
#define MAX_PATH 4096
#endif

#define ADDED_FUNCTIONS()                   \
    GO(gst_object_get_type, LFv_t)          \
    GO(gst_allocator_get_type, LFv_t)       \
    GO(gst_task_pool_get_type, LFv_t)       \
    GO(gst_element_get_type, LFv_t)         \
    GO(gst_bin_get_type, LFv_t)             \
    GO(gst_pad_get_type, LFv_t)             \
    GO(gst_uri_handler_get_type, LFv_t)     \
    GO(gst_buffer_pool_get_type, LFv_t)     \
    GO(gst_structure_new_empty, pFp_t)      \
    GO(gst_caps_new_empty, pFv_t)           \
    GO(gst_caps_replace, iFpp_t)            \
    GO(gst_caps_append_structure, vFpp_t)   \
    GO(gst_bin_add, iFpp_t)                 \
    GO(gst_element_link, iFpp_t)            \
    GO(gst_query_set_formatsv, vFpip_t)     \

typedef struct my_gst_plugin_s {
    void* handle;
    int   is_native;
} my_gst_plugin_t;

#define ADDED_STRUCT()  \
    int plugin_cnt;     \
    int plugin_cap;     \
    my_gst_plugin_t* plugins;     \

#define ADDED_FINI()                                    \
    for(int i=0; i<my->plugin_cnt; ++i) {               \
        if(my->plugins[i].is_native)                    \
            dlclose(my->plugins[i].handle);             \
        else                                            \
            my_dlclose(thread_get_emu(), my->plugins[i].handle);   \
    }                                                   \
    if(my->plugins) box_free(my->plugins);              \


#include "generated/wrappedgstreamertypes.h"

#include "wrappercallback.h"

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

// GDestroyFunc ...
#define GO(A)   \
static uintptr_t my_destroyfunc_fct_##A = 0;                                \
static int my_destroyfunc_##A(void* a, void* b)                             \
{                                                                           \
    return RunFunctionFmt(my_destroyfunc_fct_##A, "pp", a, b);  \
}
SUPER()
#undef GO
static void* findDestroyFct(void* fct)
{
    if(!fct) return fct;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_destroyfunc_fct_##A == (uintptr_t)fct) return my_destroyfunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_destroyfunc_fct_##A == 0) {my_destroyfunc_fct_##A = (uintptr_t)fct; return my_destroyfunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GDestroyNotify callback\n");
    return NULL;
}
//GstPadActivateModeFunction
#define GO(A)   \
static uintptr_t my_GstPadActivateModeFunction_fct_##A = 0;                                             \
static int my_GstPadActivateModeFunction_##A(void* a, void* b, int c, int d)                            \
{                                                                                                       \
    return (int)RunFunctionFmt(my_GstPadActivateModeFunction_fct_##A, "ppii", a, b, c, d);  \
}
SUPER()
#undef GO
static void* findGstPadActivateModeFunctionFct(void* fct)
{
    if(!fct) return fct;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_GstPadActivateModeFunction_fct_##A == (uintptr_t)fct) return my_GstPadActivateModeFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstPadActivateModeFunction_fct_##A == 0) {my_GstPadActivateModeFunction_fct_##A = (uintptr_t)fct; return my_GstPadActivateModeFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GstPadActivateModeFunction callback\n");
    return NULL;
}
//GstPadQueryFunction
#define GO(A)   \
static uintptr_t my_GstPadQueryFunction_fct_##A = 0;                                        \
static int my_GstPadQueryFunction_##A(void* a, void* b, void* c)                            \
{                                                                                           \
    return (int)RunFunctionFmt(my_GstPadQueryFunction_fct_##A, "ppp", a, b, c); \
}
SUPER()
#undef GO
static void* findGstPadQueryFunctionFct(void* fct)
{
    if(!fct) return fct;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_GstPadQueryFunction_fct_##A == (uintptr_t)fct) return my_GstPadQueryFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstPadQueryFunction_fct_##A == 0) {my_GstPadQueryFunction_fct_##A = (uintptr_t)fct; return my_GstPadQueryFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GstPadQueryFunction callback\n");
    return NULL;
}
//GstPadGetRangeFunction
#define GO(A)   \
static uintptr_t my_GstPadGetRangeFunction_fct_##A = 0;                                                 \
static int my_GstPadGetRangeFunction_##A(void* a, void* b, uint64_t c, uint32_t d, void* e)             \
{                                                                                                       \
    return (int)RunFunctionFmt(my_GstPadGetRangeFunction_fct_##A, "ppUup", a, b, c, d, e);  \
}
SUPER()
#undef GO
static void* findGstPadGetRangeFunctionFct(void* fct)
{
    if(!fct) return fct;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_GstPadGetRangeFunction_fct_##A == (uintptr_t)fct) return my_GstPadGetRangeFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstPadGetRangeFunction_fct_##A == 0) {my_GstPadGetRangeFunction_fct_##A = (uintptr_t)fct; return my_GstPadGetRangeFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GstPadGetRangeFunction callback\n");
    return NULL;
}
//GstPadChainFunction
#define GO(A)   \
static uintptr_t my_GstPadChainFunction_fct_##A = 0;                                        \
static int my_GstPadChainFunction_##A(void* a, void* b, void* c)                            \
{                                                                                           \
    return (int)RunFunctionFmt(my_GstPadChainFunction_fct_##A, "ppp", a, b, c); \
}
SUPER()
#undef GO
static void* findGstPadChainFunctionFct(void* fct)
{
    if(!fct) return fct;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_GstPadChainFunction_fct_##A == (uintptr_t)fct) return my_GstPadChainFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstPadChainFunction_fct_##A == 0) {my_GstPadChainFunction_fct_##A = (uintptr_t)fct; return my_GstPadChainFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GstPadChainFunction callback\n");
    return NULL;
}
//GstPadEventFunction
#define GO(A)   \
static uintptr_t my_GstPadEventFunction_fct_##A = 0;                                        \
static int my_GstPadEventFunction_##A(void* a, void* b, void* c)                            \
{                                                                                           \
    return (int)RunFunctionFmt(my_GstPadEventFunction_fct_##A, "ppp", a, b, c); \
}
SUPER()
#undef GO
static void* findGstPadEventFunctionFct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my_GstPadEventFunction_fct_##A == (uintptr_t)fct) return my_GstPadEventFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstPadEventFunction_fct_##A == 0) {my_GstPadEventFunction_fct_##A = (uintptr_t)fct; return my_GstPadEventFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GstPadEventFunction callback\n");
    return NULL;
}
//GstBusSyncHandler
#define GO(A)   \
static uintptr_t my_GstBusSyncHandler_fct_##A = 0;                                          \
static int my_GstBusSyncHandler_##A(void* a, void* b, void* c)                              \
{                                                                                           \
    return (int)RunFunctionFmt(my_GstBusSyncHandler_fct_##A, "ppp", a, b, c);   \
}
SUPER()
#undef GO
static void* findGstBusSyncHandlerFct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my_GstBusSyncHandler_fct_##A == (uintptr_t)fct) return my_GstBusSyncHandler_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstBusSyncHandler_fct_##A == 0) {my_GstBusSyncHandler_fct_##A = (uintptr_t)fct; return my_GstBusSyncHandler_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GstBusSyncHandler callback\n");
    return NULL;
}
//GstBusFunc
#define GO(A)   \
static uintptr_t my_GstBusFunc_fct_##A = 0;                             \
static int my_GstBusFunc_##A(void* a, void* b, void* c)                 \
{                                                                       \
    return (int)RunFunctionFmt(my_GstBusFunc_fct_##A, "ppp", a, b, c);  \
}
SUPER()
#undef GO
static void* findGstBusFuncFct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my_GstBusFunc_fct_##A == (uintptr_t)fct) return my_GstBusFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstBusFunc_fct_##A == 0) {my_GstBusFunc_fct_##A = (uintptr_t)fct; return my_GstBusFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GstBusFunc callback\n");
    return NULL;
}

//GstPluginFeatureFilter
#define GO(A)   \
static uintptr_t my_GstPluginFeatureFilter_fct_##A = 0;                                     \
static int my_GstPluginFeatureFilter_##A(void* a, void* b)                                  \
{                                                                                           \
    return (int)RunFunctionFmt(my_GstPluginFeatureFilter_fct_##A, "pp", a, b);  \
}
SUPER()
#undef GO
static void* findGstPluginFeatureFilterFct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my_GstPluginFeatureFilter_fct_##A == (uintptr_t)fct) return my_GstPluginFeatureFilter_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstPluginFeatureFilter_fct_##A == 0) {my_GstPluginFeatureFilter_fct_##A = (uintptr_t)fct; return my_GstPluginFeatureFilter_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GstPluginFeatureFilter callback\n");
    return NULL;
}

//GstCapsFilterMapFunc
#define GO(A)   \
static uintptr_t my_GstCapsFilterMapFunc_fct_##A = 0;                                           \
static int my_GstCapsFilterMapFunc_##A(void* a, void* b, void* c)                               \
{                                                                                               \
    return (int)RunFunctionFmt(my_GstCapsFilterMapFunc_fct_##A, "ppp", a, b, c);    \
}
SUPER()
#undef GO
static void* findGstCapsFilterMapFuncFct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my_GstCapsFilterMapFunc_fct_##A == (uintptr_t)fct) return my_GstCapsFilterMapFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstCapsFilterMapFunc_fct_##A == 0) {my_GstCapsFilterMapFunc_fct_##A = (uintptr_t)fct; return my_GstCapsFilterMapFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GstCapsFilterMapFunc callback\n");
    return NULL;
}
//GstPluginInit
#define GO(A)   \
static uintptr_t my_GstPluginInit_fct_##A = 0;                      \
static int my_GstPluginInit_##A(void* a)                            \
{                                                                   \
    return (int)RunFunctionFmt(my_GstPluginInit_fct_##A, "p", a);   \
}
SUPER()
#undef GO
static void* findGstPluginInitFct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my_GstPluginInit_fct_##A == (uintptr_t)fct) return my_GstPluginInit_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstPluginInit_fct_##A == 0) {my_GstPluginInit_fct_##A = (uintptr_t)fct; return my_GstPluginInit_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GstPluginInit callback\n");
    return NULL;
}
//GstIteratorFoldFunction
#define GO(A)   \
static uintptr_t my_GstIteratorFoldFunction_fct_##A = 0;                            \
static int my_GstIteratorFoldFunction_##A(void* a, void* b, void* c)                \
{                                                                                   \
    return (int)RunFunctionFmt(my_GstIteratorFoldFunction_fct_##A, "ppp", a, b, c); \
}
SUPER()
#undef GO
static void* findGstIteratorFoldFunctionFct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my_GstIteratorFoldFunction_fct_##A == (uintptr_t)fct) return my_GstIteratorFoldFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstIteratorFoldFunction_fct_##A == 0) {my_GstIteratorFoldFunction_fct_##A = (uintptr_t)fct; return my_GstIteratorFoldFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GstIteratorFoldFunction callback\n");
    return NULL;
}
//GCompareFunc
#define GO(A)   \
static uintptr_t my_GCompareFunc_fct_##A = 0;                           \
static int my_GCompareFunc_##A(void* a, void* b)                        \
{                                                                       \
    return (int)RunFunctionFmt(my_GCompareFunc_fct_##A, "pp", a, b);    \
}
SUPER()
#undef GO
static void* findGCompareFuncFct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my_GCompareFunc_fct_##A == (uintptr_t)fct) return my_GCompareFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GCompareFunc_fct_##A == 0) {my_GCompareFunc_fct_##A = (uintptr_t)fct; return my_GCompareFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GCompareFunc callback\n");
    return NULL;
}
//GCompareDataFunc
#define GO(A)   \
static uintptr_t my_GCompareDataFunc_fct_##A = 0;                            \
static int my_GCompareDataFunc_##A(void* a, void* b, void* c)                \
{                                                                            \
    return (int)RunFunctionFmt(my_GCompareDataFunc_fct_##A, "ppp", a, b, c); \
}
SUPER()
#undef GO
static void* findGCompareDataFuncFct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my_GCompareDataFunc_fct_##A == (uintptr_t)fct) return my_GCompareDataFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GCompareDataFunc_fct_##A == 0) {my_GCompareDataFunc_fct_##A = (uintptr_t)fct; return my_GCompareDataFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GCompareDataFunc callback\n");
    return NULL;
}
//GstTaskFunction
#define GO(A)   \
static uintptr_t my_GstTaskFunction_fct_##A = 0;        \
static void my_GstTaskFunction_##A(void* a)             \
{                                                       \
    RunFunctionFmt(my_GstTaskFunction_fct_##A, "p", a); \
}
SUPER()
#undef GO
static void* findGstTaskFunctionFct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my_GstTaskFunction_fct_##A == (uintptr_t)fct) return my_GstTaskFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstTaskFunction_fct_##A == 0) {my_GstTaskFunction_fct_##A = (uintptr_t)fct; return my_GstTaskFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GstTaskFunction callback\n");
    return NULL;
}
//GstTagForeachFunc
#define GO(A)   \
static uintptr_t my_GstTagForeachFunc_fct_##A = 0;                  \
static void my_GstTagForeachFunc_##A(void* a, void* b, void* c)     \
{                                                                   \
    RunFunctionFmt(my_GstTagForeachFunc_fct_##A, "ppp", a, b, c);   \
}
SUPER()
#undef GO
static void* findGstTagForeachFuncFct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my_GstTagForeachFunc_fct_##A == (uintptr_t)fct) return my_GstTagForeachFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstTagForeachFunc_fct_##A == 0) {my_GstTagForeachFunc_fct_##A = (uintptr_t)fct; return my_GstTagForeachFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GstTagForeachFunc callback\n");
    return NULL;
}
//GstPadActivateFunction
#define GO(A)   \
static uintptr_t my_GstPadActivateFunction_fct_##A = 0;                         \
static int my_GstPadActivateFunction_##A(void* a, void* b)                      \
{                                                                               \
    return (int)RunFunctionFmt(my_GstPadActivateFunction_fct_##A, "pp", a, b);  \
}
SUPER()
#undef GO
static void* findGstPadActivateFunctionFct(void* fct)
{
    if(!fct) return fct;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_GstPadActivateFunction_fct_##A == (uintptr_t)fct) return my_GstPadActivateFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstPadActivateFunction_fct_##A == 0) {my_GstPadActivateFunction_fct_##A = (uintptr_t)fct; return my_GstPadActivateFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GstPadActivateFunction callback\n");
    return NULL;
}
//GstPadProbeCallback
#define GO(A)   \
static uintptr_t my_GstPadProbeCallback_fct_##A = 0;                            \
static int my_GstPadProbeCallback_##A(void* a, void* b, void* c)                \
{                                                                               \
    return (int)RunFunctionFmt(my_GstPadProbeCallback_fct_##A, "ppp", a, b, c); \
}
SUPER()
#undef GO
static void* findGstPadProbeCallbackFct(void* fct)
{
    if(!fct) return fct;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_GstPadProbeCallback_fct_##A == (uintptr_t)fct) return my_GstPadProbeCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstPadProbeCallback_fct_##A == 0) {my_GstPadProbeCallback_fct_##A = (uintptr_t)fct; return my_GstPadProbeCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GstPadProbeCallback callback\n");
    return NULL;
}
//GstStructureForeachFunc
#define GO(A)   \
static uintptr_t my_GstStructureForeachFunc_fct_##A = 0;                            \
static int my_GstStructureForeachFunc_##A(void* a, void* b, void* c)                \
{                                                                                   \
    return (int)RunFunctionFmt(my_GstStructureForeachFunc_fct_##A, "ppp", a, b, c); \
}
SUPER()
#undef GO
static void* findGstStructureForeachFuncFct(void* fct)
{
    if(!fct) return fct;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_GstStructureForeachFunc_fct_##A == (uintptr_t)fct) return my_GstStructureForeachFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstStructureForeachFunc_fct_##A == 0) {my_GstStructureForeachFunc_fct_##A = (uintptr_t)fct; return my_GstStructureForeachFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GstStructureForeachFunc callback\n");
    return NULL;
}
//GstPadLinkFunction
#define GO(A)   \
static uintptr_t my_GstPadLinkFunction_fct_##A = 0;                             \
static int my_GstPadLinkFunction_##A(void* a, void* b, void* c)                 \
{                                                                               \
    return (int)RunFunctionFmt(my_GstPadLinkFunction_fct_##A, "ppp", a, b, c);  \
}
SUPER()
#undef GO
static void* findGstPadLinkFunctionFct(void* fct)
{
    if(!fct) return fct;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_GstPadLinkFunction_fct_##A == (uintptr_t)fct) return my_GstPadLinkFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstPadLinkFunction_fct_##A == 0) {my_GstPadLinkFunction_fct_##A = (uintptr_t)fct; return my_GstPadLinkFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GstPadLinkFunction callback\n");
    return NULL;
}
//GstStructureFilterMapFunc
#define GO(A)   \
static uintptr_t my_GstStructureFilterMapFunc_fct_##A = 0;                              \
static int my_GstStructureFilterMapFunc_##A(void* a, void* b, void* c)                  \
{                                                                                       \
    return (int)RunFunctionFmt(my_GstStructureFilterMapFunc_fct_##A, "ppp", a, b, c);   \
}
SUPER()
#undef GO
static void* findGstStructureFilterMapFuncFct(void* fct)
{
    if(!fct) return fct;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_GstStructureFilterMapFunc_fct_##A == (uintptr_t)fct) return my_GstStructureFilterMapFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstStructureFilterMapFunc_fct_##A == 0) {my_GstStructureFilterMapFunc_fct_##A = (uintptr_t)fct; return my_GstStructureFilterMapFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GstStructureFilterMapFunc callback\n");
    return NULL;
}
//GstElementForeachPadFunc
#define GO(A)   \
static uintptr_t my_GstElementForeachPadFunc_fct_##A = 0;                               \
static int my_GstElementForeachPadFunc_##A(void* a, void* b, void* c)                   \
{                                                                                       \
    return (int)RunFunctionFmt(my_GstElementForeachPadFunc_fct_##A, "ppp", a, b, c);    \
}
SUPER()
#undef GO
static void* findGstElementForeachPadFuncFct(void* fct)
{
    if(!fct) return fct;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_GstElementForeachPadFunc_fct_##A == (uintptr_t)fct) return my_GstElementForeachPadFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstElementForeachPadFunc_fct_##A == 0) {my_GstElementForeachPadFunc_fct_##A = (uintptr_t)fct; return my_GstElementForeachPadFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GstElementForeachPadFunc callback\n");
    return NULL;
}
//GstTypeFindFunction
#define GO(A)   \
static uintptr_t my_GstTypeFindFunction_fct_##A = 0;                \
static void my_GstTypeFindFunction_##A(void* a, void* b)            \
{                                                                   \
    RunFunctionFmt(my_GstTypeFindFunction_fct_##A, "pp", a, b);     \
}
SUPER()
#undef GO
static void* findGstTypeFindFunctionFct(void* fct)
{
    if(!fct) return fct;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_GstTypeFindFunction_fct_##A == (uintptr_t)fct) return my_GstTypeFindFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstTypeFindFunction_fct_##A == 0) {my_GstTypeFindFunction_fct_##A = (uintptr_t)fct; return my_GstTypeFindFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GstTypeFindFunction callback\n");
    return NULL;
}
//GstPadIterIntLinkFunction
#define GO(A)   \
static uintptr_t my_GstPadIterIntLinkFunction_fct_##A = 0;                          \
static void* my_GstPadIterIntLinkFunction_##A(void* a, void* b)                     \
{                                                                                   \
    return (void*)RunFunctionFmt(my_GstPadIterIntLinkFunction_fct_##A, "pp", a, b); \
}
SUPER()
#undef GO
static void* findGstPadIterIntLinkFunctionFct(void* fct)
{
    if(!fct) return fct;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_GstPadIterIntLinkFunction_fct_##A == (uintptr_t)fct) return my_GstPadIterIntLinkFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstPadIterIntLinkFunction_fct_##A == 0) {my_GstPadIterIntLinkFunction_fct_##A = (uintptr_t)fct; return my_GstPadIterIntLinkFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GstPadIterIntLinkFunction callback\n");
    return NULL;
}
//GstPadStickyEventsForeachFunction
#define GO(A)   \
static uintptr_t my_GstPadStickyEventsForeachFunction_fct_##A = 0;                              \
static int my_GstPadStickyEventsForeachFunction_##A(void* a, void* b, void* c)                  \
{                                                                                               \
    return (int)RunFunctionFmt(my_GstPadStickyEventsForeachFunction_fct_##A, "ppp", a, b, c);   \
}
SUPER()
#undef GO
static void* findGstPadStickyEventsForeachFunctionFct(void* fct)
{
    if(!fct) return fct;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_GstPadStickyEventsForeachFunction_fct_##A == (uintptr_t)fct) return my_GstPadStickyEventsForeachFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstPadStickyEventsForeachFunction_fct_##A == 0) {my_GstPadStickyEventsForeachFunction_fct_##A = (uintptr_t)fct; return my_GstPadStickyEventsForeachFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GstPadStickyEventsForeachFunction callback\n");
    return NULL;
}
//GstBufferForeachMetaFunc
#define GO(A)   \
static uintptr_t my_GstBufferForeachMetaFunc_fct_##A = 0;                              \
static int my_GstBufferForeachMetaFunc_##A(void* a, void* b, void* c)                  \
{                                                                                      \
    return (int)RunFunctionFmt(my_GstBufferForeachMetaFunc_fct_##A, "ppp", a, b, c);   \
}
SUPER()
#undef GO
static void* findGstBufferForeachMetaFuncFct(void* fct)
{
    if(!fct) return fct;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_GstBufferForeachMetaFunc_fct_##A == (uintptr_t)fct) return my_GstBufferForeachMetaFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstBufferForeachMetaFunc_fct_##A == 0) {my_GstBufferForeachMetaFunc_fct_##A = (uintptr_t)fct; return my_GstBufferForeachMetaFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GstBufferForeachMetaFunc callback\n");
    return NULL;
}
//GstMiniObjectCopyFunction
#define GO(A)   \
static uintptr_t my_GstMiniObjectCopyFunction_fct_##A = 0;                          \
static void* my_GstMiniObjectCopyFunction_##A(void* a)                              \
{                                                                                   \
    return (void*)RunFunctionFmt(my_GstMiniObjectCopyFunction_fct_##A, "p", a);     \
}
SUPER()
#undef GO
static void* findGstMiniObjectCopyFunctionFct(void* fct)
{
    if(!fct) return fct;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_GstMiniObjectCopyFunction_fct_##A == (uintptr_t)fct) return my_GstMiniObjectCopyFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstMiniObjectCopyFunction_fct_##A == 0) {my_GstMiniObjectCopyFunction_fct_##A = (uintptr_t)fct; return my_GstMiniObjectCopyFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GstMiniObjectCopyFunction callback\n");
    return NULL;
}
//GstMiniObjectDisposeFunction
#define GO(A)   \
static uintptr_t my_GstMiniObjectDisposeFunction_fct_##A = 0;                       \
static int my_GstMiniObjectDisposeFunction_##A(void* a)                             \
{                                                                                   \
    return (int)RunFunctionFmt(my_GstMiniObjectDisposeFunction_fct_##A, "p", a);    \
}
SUPER()
#undef GO
static void* findGstMiniObjectDisposeFunctionFct(void* fct)
{
    if(!fct) return fct;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_GstMiniObjectDisposeFunction_fct_##A == (uintptr_t)fct) return my_GstMiniObjectDisposeFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstMiniObjectDisposeFunction_fct_##A == 0) {my_GstMiniObjectDisposeFunction_fct_##A = (uintptr_t)fct; return my_GstMiniObjectDisposeFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GstMiniObjectDisposeFunction callback\n");
    return NULL;
}
//GstMiniObjectFreeFunction
#define GO(A)   \
static uintptr_t my_GstMiniObjectFreeFunction_fct_##A = 0;          \
static void my_GstMiniObjectFreeFunction_##A(void* a)               \
{                                                                   \
    RunFunctionFmt(my_GstMiniObjectFreeFunction_fct_##A, "p", a);   \
}
SUPER()
#undef GO
static void* findGstMiniObjectFreeFunctionFct(void* fct)
{
    if(!fct) return fct;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_GstMiniObjectFreeFunction_fct_##A == (uintptr_t)fct) return my_GstMiniObjectFreeFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstMiniObjectFreeFunction_fct_##A == 0) {my_GstMiniObjectFreeFunction_fct_##A = (uintptr_t)fct; return my_GstMiniObjectFreeFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GstMiniObjectFreeFunction callback\n");
    return NULL;
}
//GstClockCallback
#define GO(A)   \
static uintptr_t my_GstClockCallback_fct_##A = 0;                                   \
static int my_GstClockCallback_##A(void* a, uint64_t b, void* c, void* d)           \
{                                                                                   \
    return (int)RunFunctionFmt(my_GstClockCallback_fct_##A, "pUpp", a, b, c, d);    \
}
SUPER()
#undef GO
static void* findGstClockCallbackFct(void* fct)
{
    if(!fct) return fct;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_GstClockCallback_fct_##A == (uintptr_t)fct) return my_GstClockCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstClockCallback_fct_##A == 0) {my_GstClockCallback_fct_##A = (uintptr_t)fct; return my_GstClockCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GstClockCallback callback\n");
    return NULL;
}
//GstPadChainListFunction
#define GO(A)   \
static uintptr_t my_GstPadChainListFunction_fct_##A = 0;                            \
static int my_GstPadChainListFunction_##A(void* a, void* b, void* c)                \
{                                                                                   \
    return (int)RunFunctionFmt(my_GstPadChainListFunction_fct_##A, "ppp", a, b, c); \
}
SUPER()
#undef GO
static void* findGstPadChainListFunctionFct(void* fct)
{
    if(!fct) return fct;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_GstPadChainListFunction_fct_##A == (uintptr_t)fct) return my_GstPadChainListFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstPadChainListFunction_fct_##A == 0) {my_GstPadChainListFunction_fct_##A = (uintptr_t)fct; return my_GstPadChainListFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GstPadChainListFunction callback\n");
    return NULL;
}
//GstCapsMapFunc
#define GO(A)   \
static uintptr_t my_GstCapsMapFunc_fct_##A = 0;                             \
static int my_GstCapsMapFunc_##A(void* a, void* b, void* c)                 \
{                                                                           \
    return (int)RunFunctionFmt(my_GstCapsMapFunc_fct_##A, "ppp", a, b, c);  \
}
SUPER()
#undef GO
static void* findGstCapsMapFuncFct(void* fct)
{
    if(!fct) return fct;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_GstCapsMapFunc_fct_##A == (uintptr_t)fct) return my_GstCapsMapFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstCapsMapFunc_fct_##A == 0) {my_GstCapsMapFunc_fct_##A = (uintptr_t)fct; return my_GstCapsMapFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GstCapsMapFunc callback\n");
    return NULL;
}
//GstTagMergeFunc
#define GO(A)   \
static uintptr_t my_GstTagMergeFunc_fct_##A = 0;            \
static void my_GstTagMergeFunc_##A(void* a, void* b)        \
{                                                           \
    RunFunctionFmt(my_GstTagMergeFunc_fct_##A, "pp", a, b); \
}
SUPER()
#undef GO
static void* findGstTagMergeFuncFct(void* fct)
{
    if(!fct) return fct;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_GstTagMergeFunc_fct_##A == (uintptr_t)fct) return my_GstTagMergeFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstTagMergeFunc_fct_##A == 0) {my_GstTagMergeFunc_fct_##A = (uintptr_t)fct; return my_GstTagMergeFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GstTagMergeFunc callback\n");
    return NULL;
}
//GstIteratorForeachFunction
#define GO(A)   \
static uintptr_t my_GstIteratorForeachFunction_fct_##A = 0;             \
static void my_GstIteratorForeachFunction_##A(void* a, void* b)         \
{                                                                       \
    RunFunctionFmt(my_GstIteratorForeachFunction_fct_##A, "pp", a, b);  \
}
SUPER()
#undef GO
static void* findGstIteratorForeachFunctionFct(void* fct)
{
    if(!fct) return fct;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_GstIteratorForeachFunction_fct_##A == (uintptr_t)fct) return my_GstIteratorForeachFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstIteratorForeachFunction_fct_##A == 0) {my_GstIteratorForeachFunction_fct_##A = (uintptr_t)fct; return my_GstIteratorForeachFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GstIteratorForeachFunction callback\n");
    return NULL;
}
//GstPadEventFullFunction
#define GO(A)   \
static uintptr_t my_GstPadEventFullFunction_fct_##A = 0;                            \
static int my_GstPadEventFullFunction_##A(void* a, void* b, void* c)                \
{                                                                                   \
    return (int)RunFunctionFmt(my_GstPadEventFullFunction_fct_##A, "ppp", a, b, c); \
}
SUPER()
#undef GO
static void* findGstPadEventFullFunctionFct(void* fct)
{
    if(!fct) return fct;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_GstPadEventFullFunction_fct_##A == (uintptr_t)fct) return my_GstPadEventFullFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstPadEventFullFunction_fct_##A == 0) {my_GstPadEventFullFunction_fct_##A = (uintptr_t)fct; return my_GstPadEventFullFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GstPadEventFullFunction callback\n");
    return NULL;
}
//GstBufferListFunc
#define GO(A)   \
static uintptr_t my_GstBufferListFunc_fct_##A = 0;                              \
static int my_GstBufferListFunc_##A(void* a, uint32_t b, void* c)               \
{                                                                               \
    return (int)RunFunctionFmt(my_GstBufferListFunc_fct_##A, "pup", a, b, c);   \
}
SUPER()
#undef GO
static void* findGstBufferListFuncFct(void* fct)
{
    if(!fct) return fct;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_GstBufferListFunc_fct_##A == (uintptr_t)fct) return my_GstBufferListFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstBufferListFunc_fct_##A == 0) {my_GstBufferListFunc_fct_##A = (uintptr_t)fct; return my_GstBufferListFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GstBufferListFunc callback\n");
    return NULL;
}

#undef SUPER

EXPORT void my_gst_caps_set_simple(x64emu_t* emu, void* caps, void* field, void* b) {

    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 2);
    my->gst_caps_set_simple_valist(caps, field, VARARGS);
}

EXPORT void my_gst_caps_set_simple_valist(x64emu_t* emu, void* caps, void* field, x64_va_list_t V) {
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(V);
    #else
    CREATE_VALIST_FROM_VALIST(V, emu->scratch);
    #endif
    my->gst_caps_set_simple_valist(caps, field, VARARGS);
}

EXPORT void my_gst_structure_remove_fields(x64emu_t* emu, void* structure, void* field, void* b) {

    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 2);
    my->gst_structure_remove_fields_valist(structure, field, VARARGS);
}

EXPORT void my_gst_structure_remove_fields_valist(x64emu_t* emu, void* structure, void* field, x64_va_list_t V) {
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(V);
    #else
    CREATE_VALIST_FROM_VALIST(V, emu->scratch);
    #endif
    my->gst_structure_remove_fields_valist(structure, field, VARARGS);
}

EXPORT void my_gst_debug_log(x64emu_t* emu, void* cat, uint32_t level, void* file, void* func, int line, void* obj, void* fmt, void* b) {

    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 7);
    PREPARE_VALIST;
    my->gst_debug_log_valist(cat, level, file, func, line, obj, fmt, VARARGS);
}

EXPORT void my_gst_debug_log_valist(x64emu_t* emu, void* cat, uint32_t level, void* file, void* func, int line, void* obj, void* fmt, x64_va_list_t V) {

    #ifdef CONVERT_VALIST
    CONVERT_VALIST(V);
    #else
    myStackAlignValist(emu, (const char*)fmt, emu->scratch, V);
    PREPARE_VALIST;
    #endif
    my->gst_debug_log_valist(cat, level, file, func, line, obj, fmt, VARARGS);
}

EXPORT int my_gst_structure_get(x64emu_t* emu, void* structure, void* field, void* b) {

    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 2);
    return my->gst_structure_get_valist(structure, field, VARARGS);
}

EXPORT int my_gst_structure_get_valist(x64emu_t* emu, void* structure, void* field, x64_va_list_t V) {
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(V);
    #else
    CREATE_VALIST_FROM_VALIST(V, emu->scratch);
    #endif
    return my->gst_structure_get_valist(structure, field, VARARGS);
}


EXPORT void my_gst_pad_set_activatemode_function_full(x64emu_t* emu, void* pad, void* f, void* data, void* d)
{
    my->gst_pad_set_activatemode_function_full(pad, findGstPadActivateModeFunctionFct(f), data, findDestroyFct(d));
}

EXPORT void my_gst_pad_set_query_function_full(x64emu_t* emu, void* pad, void* f, void* data, void* d)
{
    my->gst_pad_set_query_function_full(pad, findGstPadQueryFunctionFct(f), data, findDestroyFct(d));
}

EXPORT void my_gst_pad_set_getrange_function_full(x64emu_t* emu, void* pad, void* f, void* data, void* d)
{
    my->gst_pad_set_getrange_function_full(pad, findGstPadGetRangeFunctionFct(f), data, findDestroyFct(d));
}

EXPORT void my_gst_pad_set_chain_function_full(x64emu_t* emu, void* pad, void* f, void* data, void* d)
{
    my->gst_pad_set_chain_function_full(pad, findGstPadChainFunctionFct(f), data, findDestroyFct(d));
}

EXPORT void my_gst_pad_set_event_function_full(x64emu_t* emu, void* pad, void* f, void* data, void* d)
{
    my->gst_pad_set_event_function_full(pad, findGstPadEventFunctionFct(f), data, findDestroyFct(d));
}

EXPORT void my_gst_bus_set_sync_handler(x64emu_t* emu, void* bus, void* f, void* data, void* d)
{
    my->gst_bus_set_sync_handler(bus, findGstBusSyncHandlerFct(f), data, findDestroyFct(d));
}

EXPORT void* my_gst_buffer_new_wrapped_full(x64emu_t* emu, uint32_t f, void* data, size_t maxsize, size_t offset, size_t size, void* user, void* d)
{
    return my->gst_buffer_new_wrapped_full(f, data, maxsize, offset, size, user, findDestroyFct(d));
}

EXPORT void* my_gst_structure_new(x64emu_t* emu, void* name, void* first, uint64_t* b)
{
    if(!first)
        return my->gst_structure_new_empty(name);
    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 2);
    return my->gst_structure_new_valist(name, first, VARARGS);
}

EXPORT void my_gst_mini_object_set_qdata(x64emu_t* emu, void* object, uint32_t quark, void* data, void* d)
{
    my->gst_mini_object_set_qdata(object, quark, data, findDestroyFct(d));
}

EXPORT void* my_gst_caps_new_simple(x64emu_t* emu, void* type, void* name, void* b)
{
    // need to unroll the function here, there is no direct VA equivalent
    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 2);
    void* caps = my->gst_caps_new_empty();
    void* structure = my->gst_structure_new_valist(type, name, VARARGS);
    if (structure)
        my->gst_caps_append_structure(caps, structure);
    else
        my->gst_caps_replace(&caps, NULL);

    return caps;
}

EXPORT void* my_gst_registry_feature_filter(x64emu_t* emu, void* reg, void* filter, int first, void* data)
{
    return my->gst_registry_feature_filter(reg, findGstPluginFeatureFilterFct(filter), first, data);
}

EXPORT int my_gst_caps_foreach(x64emu_t* emu, void* caps, void* f, void* data)
{
    return my->gst_caps_foreach(caps, findGstCapsFilterMapFuncFct(f), data);
}

EXPORT uint32_t my_gst_bus_add_watch(x64emu_t* emu, void* bus, void* f, void* data)
{
    return my->gst_bus_add_watch(bus, findGstBusFuncFct(f), data);
}

EXPORT uint32_t my_gst_bus_add_watch_full(x64emu_t* emu, void* bus, int priority, void* f, void* data, void* d)
{
    return my->gst_bus_add_watch_full(bus, priority, findGstBusFuncFct(f), data, findDestroyFct(d));
}

EXPORT int my_gst_bin_add_many(x64emu_t* emu, void* bin, void* first, void** b)
{
    int ret = my->gst_bin_add(bin, first);
    while(ret && *b) {
        ret = my->gst_bin_add(bin, *b);
        ++b;
    }
    return ret;
}

EXPORT int my_gst_element_link_many(x64emu_t* emu, void* e1, void* e2, void** b)
{
    int ret = my->gst_element_link(e1, e2);
    void* a = e2;
    while(ret && *b) {
        ret = my->gst_element_link(a, *b);
        a = *b;
        ++b;
    }
    return ret;
}

EXPORT void* my_gst_plugin_load_file(x64emu_t* emu, const char* filename, void** error)
{
    printf_log(LOG_INFO, "using gst_plugin_load_file, file %s (is x86_64=%d)\n", filename, FileIsX64ELF(filename));
    return my->gst_plugin_load_file((void*)filename, error);
}

static void register_plugins_from_folder(x64emu_t* emu, const char* folder)
{
    if(!folder) {
        printf_log(/*LOG_DEBUG*/ LOG_INFO, "BOX64 didn't detect any custom gstreamer-1.0 folder\n");
        return;
    }
    DIR *d;
    struct dirent *dir;
    // get folder where gstreamer came from
    char native_folder[MAX_PATH+1] = {0};
    Dl_info dli;
    if(dladdr(my->gst_init_check, &dli)) {
        strcpy(native_folder, dli.dli_fname);
        char* p = strrchr(native_folder, '/');
        *p = '\0';
        strcat(native_folder, "/gstreamer-1.0/");
        printf_log(/*LOG_DEBUG*/ LOG_INFO, "BOX64 Will look for native gstreamer plugin in %s\n", native_folder);
    }
    d = opendir(folder);
    if(!d)
        return;
    while((dir = readdir(d)) != NULL) {
        if(strstr(dir->d_name, "libgst")==dir->d_name && strstr(dir->d_name, ".so")) {
            // handling this one, stripping to get the plugin name
            char name[500];
            int is_native = 0;
            void* handle = NULL;
            char filename[MAX_PATH];
            char regfunc_name[500];
            // get the name of the function
            strcpy(name, dir->d_name + strlen("libgst"));
            *strrchr(name, '.') = '\0';
            snprintf(regfunc_name, sizeof(regfunc_name), "gst_plugin_%s_register", name);
            // check if native version exist
            if(native_folder[0] && !FindInCollection(dir->d_name, &my_context->box64_emulated_libs)) {
                strcpy(filename, native_folder);
                strcat(filename, dir->d_name);
                handle = dlopen(filename, 2);
            }
            if(handle) 
                is_native = 1;
            else {
                strcpy(filename, folder);
                strcat(filename, "/");
                strcat(filename, dir->d_name);
                handle = my_dlopen(emu, filename, 2);
            }
            void* f_init = handle?(is_native?dlsym(handle, regfunc_name):my_dlsym(emu, handle, regfunc_name)):NULL;
            if(f_init) {
                printf_log(LOG_DEBUG, "Will registering %sgstplugin %s\n", is_native?"native ":"", filename);
                if(is_native)
                    ((vFv_t)(f_init))();
                else
                    RunFunctionFmt((uintptr_t)f_init, "");
                if(my->plugin_cnt==my->plugin_cap) {
                    my->plugin_cap += 8;
                    my->plugins = (my_gst_plugin_t*)box_realloc(my->plugins, my->plugin_cap*sizeof(my_gst_plugin_t));
                }
                my->plugins[my->plugin_cnt].is_native = is_native;
                my->plugins[my->plugin_cnt++].handle = handle;
            } else {
                printf_log(LOG_DEBUG, "Failled to register %sgstplugin %s, name=%s, handle=%p\n", is_native?"native ":"", filename, name, handle);
            }
            if(handle && !f_init) {
                is_native?dlclose(handle):my_dlclose(emu, handle);
                handle = NULL;
            }
        }
    }
    closedir(d);
}

extern const char* box64_custom_gstreamer;
EXPORT int my_gst_init_check(x64emu_t* emu, int* argc, char*** argv, void** error)
{
    int ret = my->gst_init_check(argc, argv, error);
    register_plugins_from_folder(emu, box64_custom_gstreamer);
    return ret;
}

EXPORT void my_gst_init(x64emu_t* emu, int* argc, char*** argv)
{
    my->gst_init(argc, argv);
    register_plugins_from_folder(emu, box64_custom_gstreamer);
}

EXPORT int my_gst_plugin_register_static(x64emu_t* emu, int maj_v, int min_v, void* name, void* desc, void* init_f, void* ver, void* lic, void* source, void* pack, void* orig)
{
    return my->gst_plugin_register_static(maj_v, min_v, name, desc, findGstPluginInitFct(init_f), ver, lic, source, pack, orig);
}

EXPORT int my_gst_info_vasprintf(x64emu_t* emu, void* res, void* fmt, x64_va_list_t V)
{
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(V);
    #else
    CREATE_VALIST_FROM_VALIST(V, emu->scratch);
    #endif
    return my->gst_info_vasprintf(res, fmt, VARARGS);
}

EXPORT void* my__gst_element_error_printf(x64emu_t* emu, void* fmt, uintptr_t* b)
{
    char* buffer;
    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 1);
    int len = my->gst_info_vasprintf(&buffer, fmt, VARARGS);
    if(len<0)
        buffer = NULL;
    return buffer;
}

EXPORT void my_gst_structure_set_valist(x64emu_t* emu, void* st, void* fieldname, x64_va_list_t V)
{
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(V);
    #else
    CREATE_VALIST_FROM_VALIST(V, emu->scratch);
    #endif
    my->gst_structure_set_valist(st, fieldname, VARARGS);
}

EXPORT void my_gst_structure_set(x64emu_t* emu, void* st, void* fieldname, uintptr_t* b)
{
    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 2);
    my->gst_structure_set_valist(st, fieldname, VARARGS);
}

EXPORT uint32_t my_gst_iterator_fold(x64emu_t* emu, void* it, void* f, void* ret, void* data)
{
    return my->gst_iterator_fold(it, findGstIteratorFoldFunctionFct(f), ret, data);
}

EXPORT void* my_gst_util_array_binary_search(x64emu_t* emu, void* array, uint32_t num, size_t size, void* f, uint32_t mode, void* search, void* data)
{
    return my->gst_util_array_binary_search(array, num, size, findGCompareDataFuncFct(f), mode, search, data);
}

EXPORT void* my_gst_structure_new_valist(x64emu_t* emu, void* name, void* first, x64_va_list_t V)
{
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(V);
    #else
    CREATE_VALIST_FROM_VALIST(V, emu->scratch);
    #endif
    return my->gst_structure_new_valist(name, first, VARARGS);
}

EXPORT void* my_gst_make_element_message_details(x64emu_t* emu, void* name, uintptr_t* b)
{
    if(!name)
        return NULL;
    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 1);
    return my->gst_structure_new_valist("detail", name, VARARGS);
}

EXPORT int my_gst_pad_start_task(x64emu_t* emu, void* pad, void* f, void* data, void* d)
{
    return my->gst_pad_start_task(pad, findGstTaskFunctionFct(f), data, findDestroyFct(d));
}

EXPORT void my_gst_tag_list_foreach(x64emu_t* emu, void* list, void* f, void* data)
{
    my->gst_tag_list_foreach(list, findGstTagForeachFuncFct(f), data);
}

EXPORT void* my_gst_memory_new_wrapped(x64emu_t* emu, uint32_t flags, void* data, size_t maxsz, size_t offset, size_t size, void* user_data, void* d)
{
    return my->gst_memory_new_wrapped(flags, data, maxsz, offset, size, user_data, findDestroyFct(d));
}

EXPORT void* my_gst_pad_create_stream_id_printf_valist(x64emu_t* emu, void* pad, void* parent, void* id, x64_va_list_t V)
{
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(V);
    #else
    CREATE_VALIST_FROM_VALIST(V, emu->scratch);
    #endif
    return my->gst_pad_create_stream_id_printf_valist(pad, parent, id, VARARGS);
}

EXPORT void* my_gst_pad_create_stream_id_printf(x64emu_t* emu, void* pad, void* parent, void* id, uintptr_t* b)
{
    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 3);
    return my->gst_pad_create_stream_id_printf_valist(pad, parent, id, VARARGS);
}

EXPORT void* my_gst_caps_features_new_id_valist(x64emu_t* emu, uint32_t id, x64_va_list_t V)
{
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(V);
    #else
    CREATE_VALIST_FROM_VALIST(V, emu->scratch);
    #endif
    return my->gst_caps_features_new_id_valist(id, VARARGS);
}

EXPORT void* my_gst_caps_features_new_id(x64emu_t* emu, uint32_t id, uintptr_t* b)
{
    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 1);
    return my->gst_caps_features_new_id_valist(id, VARARGS);
}


EXPORT void my_gst_pad_set_activate_function_full(x64emu_t* emu, void* pad, void* f, void* data, void* d)
{
    my->gst_pad_set_activate_function_full(pad, findGstPadActivateFunctionFct(f), data, findDestroyFct(d));
}

EXPORT void my_gst_tag_list_add_valist(x64emu_t* emu, void* list, int mode, void* tag, x64_va_list_t V)
{
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(V);
    #else
    CREATE_VALIST_FROM_VALIST(V, emu->scratch);
    #endif
    my->gst_tag_list_add_valist(list, mode, tag, VARARGS);
}
EXPORT void my_gst_tag_list_add(x64emu_t* emu, void* list, int mode, void* tag, uintptr_t* b)
{
    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 3);
    my->gst_tag_list_add_valist(list, mode, tag, VARARGS);
}

EXPORT void* my_gst_tag_list_new_valist(x64emu_t* emu, x64_va_list_t V)
{
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(V);
    #else
    CREATE_VALIST_FROM_VALIST(V, emu->scratch);
    #endif
    return my->gst_tag_list_new_valist(VARARGS);
}

EXPORT void* my_gst_tag_list_new(x64emu_t* emu, void* arg, uintptr_t* b)
{
    // construct VARARGS with arg[0] too, because gst_tag_list_new_valist have just the va_list
    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 0);
    return my->gst_tag_list_new_valist(VARARGS);
}

EXPORT void* my_gst_caps_features_new_valist(x64emu_t* emu, void* feat1, x64_va_list_t V)
{
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(V);
    #else
    CREATE_VALIST_FROM_VALIST(V, emu->scratch);
    #endif
    return my->gst_caps_features_new_valist(feat1, VARARGS);
}

EXPORT void* my_gst_caps_features_new(x64emu_t* emu, void* feat1, uintptr_t* b)
{
    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 1);
    return my->gst_caps_features_new_valist(feat1, VARARGS);
}

EXPORT unsigned long my_gst_pad_add_probe(x64emu_t* emu, void* pad, uint32_t mask, void* f, void* data, void* d)
{
    return my->gst_pad_add_probe(pad, mask, findGstPadProbeCallbackFct(f), data, findDestroyFct(d));
}

EXPORT int my_gst_structure_foreach(x64emu_t* emu, void* st, void* f, void* data)
{
    return my->gst_structure_foreach(st, findGstStructureForeachFuncFct(f), data);
}

EXPORT void my_gst_pad_set_link_function_full(x64emu_t* emu, void* pad, void* f, void* data, void *d)
{
    my->gst_pad_set_link_function_full(pad, findGstPadLinkFunctionFct(f), data, findDestroyFct(d));
}

EXPORT void my_gst_tag_list_add_valist_values(x64emu_t* emu, void* list, int mode, void* tag, x64_va_list_t V)
{
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(V);
    #else
    CREATE_VALIST_FROM_VALIST(V, emu->scratch);
    #endif
    my->gst_tag_list_add_valist_values(list, mode, tag, VARARGS);
}

EXPORT void my_gst_tag_list_add_values(x64emu_t* emu, void* list, int mode, void* tag, uintptr_t* b)
{
    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 3);
    my->gst_tag_list_add_valist_values(list, mode, tag, VARARGS);
}

EXPORT void my_gst_structure_filter_and_map_in_place(x64emu_t* emu, void* s, void* f, void* data)
{
    my->gst_structure_filter_and_map_in_place(s, findGstStructureFilterMapFuncFct(f), data);
}

EXPORT int my_gst_element_foreach_sink_pad(x64emu_t* emu, void* element, void* f, void* data)
{
    return my->gst_element_foreach_sink_pad(element, findGstElementForeachPadFuncFct(f),data);
}

EXPORT void* my_gst_task_new(x64emu_t* emu, void* f, void* data, void* d)
{
    return my->gst_task_new(findGstTaskFunctionFct(f), data, findDestroyFct(d));
}

EXPORT int my_gst_type_find_register(x64emu_t* emu, void* plugin, void* name, uint32_t rank, void* f, void* ext, void* caps, void* data, void* d)
{
    return my->gst_type_find_register(plugin, name, rank, findGstTypeFindFunctionFct(f), ext, caps, data, findDestroyFct(f));
}

EXPORT void my_gst_pad_set_iterate_internal_links_function_full(x64emu_t* emu, void* pad, void* f, void* data, void* d)
{
    my->gst_pad_set_iterate_internal_links_function_full(pad, findGstPadIterIntLinkFunctionFct(f), data, findDestroyFct(d));
}

EXPORT void my_gst_pad_sticky_events_foreach(x64emu_t* emu, void* pad, void* f, void* data)
{
    my->gst_pad_sticky_events_foreach(pad, findGstPadStickyEventsForeachFunctionFct(f), data);
}

EXPORT int my_gst_buffer_foreach_meta(x64emu_t* emu, void* buff, void* f, void* data)
{
    return my->gst_buffer_foreach_meta(buff, findGstBufferForeachMetaFuncFct(f), data);
}

EXPORT void my_gst_mini_object_init(x64emu_t* emu, void* obj, uint32_t flags, size_t type, void* copy_f, void* disp_f, void* free_f)
{
    my->gst_mini_object_init(obj, flags, type, findGstMiniObjectCopyFunctionFct(copy_f), findGstMiniObjectDisposeFunctionFct(disp_f), findGstMiniObjectFreeFunctionFct(free_f));
}

EXPORT int my_gst_iterator_find_custom(x64emu_t* emu, void* it, void* f, void* elem, void* data)
{
    return my->gst_iterator_find_custom(it, findGCompareFuncFct(f), elem, data);
}

EXPORT int my_gst_clock_id_wait_async(x64emu_t* emu, void* id, void* f, void* data, void* d)
{
    return my->gst_clock_id_wait_async(id, findGstClockCallbackFct(f), data, findDestroyFct(d));
}

EXPORT void my_gst_pad_set_chain_list_function_full(x64emu_t* emu, void* pad, void *f, void* data, void* d)
{
    return my->gst_pad_set_chain_list_function_full(pad, findGstPadChainListFunctionFct(f), data, findDestroyFct(d));
}

EXPORT int my_gst_caps_map_in_place(x64emu_t* emu, void* caps, void* f, void* data)
{
    return my->gst_caps_map_in_place(caps, findGstCapsMapFuncFct(f), data);
}

EXPORT void my_gst_tag_register(x64emu_t* emu, void* name, uint32_t flag, size_t type, void* nick, void* blurb, void* f)
{
    my->gst_tag_register(name, flag, type, nick, blurb, findGstTagMergeFuncFct(f));
}

EXPORT uint32_t my_gst_iterator_foreach(x64emu_t* emu, void* it, void* f, void* data)
{
    return my->gst_iterator_foreach(it, findGstIteratorForeachFunctionFct(f), data);
}

EXPORT void my_gst_pad_set_event_full_function_full(x64emu_t* emu, void* pad, void* f, void* data, void* d)
{
    my->gst_pad_set_event_full_function_full(pad, findGstPadEventFullFunctionFct(f), data, findDestroyFct(d));
}

EXPORT int my_gst_buffer_list_foreach(x64emu_t* emu, void* list, void* f, void* data)
{
    return my->gst_buffer_list_foreach(list, findGstBufferListFuncFct(f), data);
}

EXPORT void my_gst_query_set_formats(x64emu_t* emu, void* query, int n, uint64_t* b)
{
    uint32_t a[n];
    for(int i=0; i<n; ++i)
        a[i] = getVArgs(emu, 2, b, i);
    my->gst_query_set_formatsv(query, n, a);
}

EXPORT void my_gst_debug_log_id_valist(x64emu_t* emu, void* cat, uint32_t level, void* f, void* func, int l, void* id, void* fmt, x64_va_list_t V)
{
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(V);
    #else
    CREATE_VALIST_FROM_VALIST(V, emu->scratch);
    #endif
    my->gst_debug_log_id_valist(cat, level, f, func, l, id, fmt, VARARGS);
}

EXPORT void my_gst_debug_log_id(x64emu_t* emu, void* cat, uint32_t level, void* f, void* func, int l, void* id, void* fmt, void* b)
{
    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 7);
    my->gst_debug_log_id_valist(cat, level, f, func, l, id, fmt, VARARGS);
}

#define PRE_INIT \
    if (BOX64ENV(nogtk)) return -2;

#define CUSTOM_INIT \
    SetGstObjectID(my->gst_object_get_type());                 \
    SetGstAllocatorID(my->gst_allocator_get_type());           \
    SetGstTaskPoolID(my->gst_task_pool_get_type());            \
    SetGstElementID(my->gst_element_get_type());               \
    SetGstBinID(my->gst_bin_get_type());                       \
    SetGstPadID(my->gst_pad_get_type());                       \
    SetGstURIHandlerID(my->gst_uri_handler_get_type());        \
    SetGstBufferPoolID(my->gst_buffer_pool_get_type());        \

#define NEEDED_LIBS "libgtk-3.so.0"

#include "wrappedlib_init.h"
