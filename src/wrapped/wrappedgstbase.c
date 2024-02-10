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
#include "myalign.h"
#include "gtkclass.h"
#include "fileutils.h"

#ifdef ANDROID
    const char* gstbaseName = "libgstbase-1.0.so";
#else
    const char* gstbaseName = "libgstbase-1.0.so.0";
#endif

#define LIBNAME gstbase

typedef size_t  (*LFv_t)();

#define ADDED_FUNCTIONS()                   \
    GO(gst_base_transform_get_type, LFv_t)  \
    GO(gst_base_sink_get_type, LFv_t)       \
    GO(gst_aggregator_get_type, LFv_t)      \
    GO(gst_push_src_get_type, LFv_t)        \
    GO(gst_base_src_get_type, LFv_t)        \
    GO(gst_aggregator_pad_get_type, LFv_t)  \

#include "generated/wrappedgstbasetypes.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)

// GstCollectPadsFunction ...
#define GO(A)   \
static uintptr_t my_GstCollectPadsFunction_fct_##A = 0;                     \
static int my_GstCollectPadsFunction_##A(void* a, void* b)                  \
{                                                                           \
    return RunFunctionFmt(my_GstCollectPadsFunction_fct_##A, "pp", a, b);   \
}
SUPER()
#undef GO
static void* findGstCollectPadsFunctionFct(void* fct)
{
    if(!fct) return fct;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GstCollectPadsFunction_fct_##A == (uintptr_t)fct) return my_GstCollectPadsFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstCollectPadsFunction_fct_##A == 0) {my_GstCollectPadsFunction_fct_##A = (uintptr_t)fct; return my_GstCollectPadsFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstbase GstCollectPadsFunction callback\n");
    return NULL;
}
// GstCollectDataDestroyNotify ...
#define GO(A)   \
static uintptr_t my_GstCollectDataDestroyNotify_fct_##A = 0;        \
static void my_GstCollectDataDestroyNotify_##A(void* a)             \
{                                                                   \
    RunFunctionFmt(my_GstCollectDataDestroyNotify_fct_##A, "p", a); \
}
SUPER()
#undef GO
static void* findGstCollectDataDestroyNotifyFct(void* fct)
{
    if(!fct) return fct;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GstCollectDataDestroyNotify_fct_##A == (uintptr_t)fct) return my_GstCollectDataDestroyNotify_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstCollectDataDestroyNotify_fct_##A == 0) {my_GstCollectDataDestroyNotify_fct_##A = (uintptr_t)fct; return my_GstCollectDataDestroyNotify_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstbase GstCollectDataDestroyNotify callback\n");
    return NULL;
}
// GstCollectPadsEventFunction ...
#define GO(A)   \
static uintptr_t my_GstCollectPadsEventFunction_fct_##A = 0;                            \
static int my_GstCollectPadsEventFunction_##A(void* a, void* b, void* c, void* d)       \
{                                                                                       \
    return RunFunctionFmt(my_GstCollectPadsEventFunction_fct_##A, "pppp", a, b, c, d);  \
}
SUPER()
#undef GO
static void* findGstCollectPadsEventFunctionFct(void* fct)
{
    if(!fct) return fct;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GstCollectPadsEventFunction_fct_##A == (uintptr_t)fct) return my_GstCollectPadsEventFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstCollectPadsEventFunction_fct_##A == 0) {my_GstCollectPadsEventFunction_fct_##A = (uintptr_t)fct; return my_GstCollectPadsEventFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstbase GstCollectPadsEventFunction callback\n");
    return NULL;
}
// GstCollectPadsQueryFunction ...
#define GO(A)   \
static uintptr_t my_GstCollectPadsQueryFunction_fct_##A = 0;                            \
static int my_GstCollectPadsQueryFunction_##A(void* a, void* b, void* c, void* d)       \
{                                                                                       \
    return RunFunctionFmt(my_GstCollectPadsQueryFunction_fct_##A, "pppp", a, b, c, d);  \
}
SUPER()
#undef GO
static void* findGstCollectPadsQueryFunctionFct(void* fct)
{
    if(!fct) return fct;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GstCollectPadsQueryFunction_fct_##A == (uintptr_t)fct) return my_GstCollectPadsQueryFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstCollectPadsQueryFunction_fct_##A == 0) {my_GstCollectPadsQueryFunction_fct_##A = (uintptr_t)fct; return my_GstCollectPadsQueryFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstbase GstCollectPadsQueryFunction callback\n");
    return NULL;
}
// GstCollectPadsClipFunction ...
#define GO(A)   \
static uintptr_t my_GstCollectPadsClipFunction_fct_##A = 0;                                 \
static int my_GstCollectPadsClipFunction_##A(void* a, void* b, void* c, void* d, void* e)   \
{                                                                                           \
    return RunFunctionFmt(my_GstCollectPadsClipFunction_fct_##A, "ppppp", a, b, c, d, e);   \
}
SUPER()
#undef GO
static void* findGstCollectPadsClipFunctionFct(void* fct)
{
    if(!fct) return fct;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GstCollectPadsClipFunction_fct_##A == (uintptr_t)fct) return my_GstCollectPadsClipFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstCollectPadsClipFunction_fct_##A == 0) {my_GstCollectPadsClipFunction_fct_##A = (uintptr_t)fct; return my_GstCollectPadsClipFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstbase GstCollectPadsClipFunction callback\n");
    return NULL;
}
// GstCollectPadsBufferFunction ...
#define GO(A)   \
static uintptr_t my_GstCollectPadsBufferFunction_fct_##A = 0;                           \
static int my_GstCollectPadsBufferFunction_##A(void* a, void* b, void* c, void* d)      \
{                                                                                       \
    return RunFunctionFmt(my_GstCollectPadsBufferFunction_fct_##A, "pppp", a, b, c, d); \
}
SUPER()
#undef GO
static void* findGstCollectPadsBufferFunctionFct(void* fct)
{
    if(!fct) return fct;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GstCollectPadsBufferFunction_fct_##A == (uintptr_t)fct) return my_GstCollectPadsBufferFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstCollectPadsBufferFunction_fct_##A == 0) {my_GstCollectPadsBufferFunction_fct_##A = (uintptr_t)fct; return my_GstCollectPadsBufferFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstbase GstCollectPadsBufferFunction callback\n");
    return NULL;
}

#undef SUPER

EXPORT void my_gst_collect_pads_set_function(x64emu_t* emu, void* p, void* f, void* data)
{
    my->gst_collect_pads_set_function(p, findGstCollectPadsFunctionFct(f), data);
}

EXPORT void* my_gst_collect_pads_add_pad(x64emu_t* emu, void* pads, void* pad, uint32_t size, void* d, int lock)
{
    return my->gst_collect_pads_add_pad(pads, pad, size, findGstCollectDataDestroyNotifyFct(d), lock);
}

EXPORT void my_gst_collect_pads_set_event_function(x64emu_t* emu, void* pads, void* f, void* data)
{
    my->gst_collect_pads_set_event_function(pads, findGstCollectPadsEventFunctionFct(f), data);
}

EXPORT void my_gst_collect_pads_set_query_function(x64emu_t* emu, void* pads, void* f, void* data)
{
    my->gst_collect_pads_set_query_function(pads, findGstCollectPadsQueryFunctionFct(f), data);
}

EXPORT void my_gst_collect_pads_set_clip_function(x64emu_t* emu, void* pads, void* f, void* data)
{
    my->gst_collect_pads_set_clip_function(pads, findGstCollectPadsClipFunctionFct(f), data);
}

EXPORT void my_gst_collect_pads_set_buffer_function(x64emu_t* emu, void* pads, void* f, void* data)
{
    my->gst_collect_pads_set_buffer_function(pads, findGstCollectPadsBufferFunctionFct(f), data);
}

#define PRE_INIT    \
    if(box64_nogtk) \
        return -1;

#define CUSTOM_INIT \
    SetGstBaseTransformID(my->gst_base_transform_get_type());\
    SetGstBaseSinkID(my->gst_base_sink_get_type());\
    SetGstAggregatorID(my->gst_aggregator_get_type());\
    SetGstPushSrcID(my->gst_push_src_get_type());\
    SetGstBaseSrcID(my->gst_base_src_get_type());\
    SetGstAggregatorPadID(my->gst_aggregator_pad_get_type());

#ifdef ANDROID
#define NEEDED_LIBS "libgstreamer-1.0.so"
#else
#define NEEDED_LIBS "libgstreamer-1.0.so.0"
#endif

#include "wrappedlib_init.h"
