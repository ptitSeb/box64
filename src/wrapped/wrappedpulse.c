#define _GNU_SOURCE         /* See feature_test_macros(7) */
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
#include "box64context.h"
#include "librarian.h"
#include "myalign.h"

const char* pulseName = "libpulse.so.0";
#define LIBNAME pulse

typedef struct my_pa_mainloop_api_s {
    void*   data;
    void*   io_new;
    void*   io_enable;
    void*   io_free;
    void*   io_set_destroy;
    void*   time_new;
    void*   time_restart;
    void*   time_free;
    void*   time_set_destroy;
    void*   defer_new;
    void*   defer_enable;
    void*   defer_free;
    void*   defer_set_destroy;
    void*   quit;
} my_pa_mainloop_api_t;

typedef void (*vFp_t)(void*);
typedef void* (*pFp_t)(void*);
typedef int (*iFp_t)(void*);
typedef void (*vFpi_t)(void*, int32_t);
typedef void* (*pFpp_t)(void*, void*);
typedef void (*vFpp_t)(void*, void*);
typedef int (*iFppp_t)(void*, void*, void*);
typedef void* (*pFipp_t)(int32_t, void*, void*);
typedef void* (*pFppp_t)(void*, void*, void*);
typedef void (*vFppp_t)(void*, void*, void*);
typedef void* (*pFpipp_t)(void*, int32_t, void*, void*);
typedef int (*iFppip_t)(void*, void*, int, void*);
typedef void* (*pFpppp_t)(void*, void*, void*, void*);
typedef void* (*pFpupp_t)(void*, uint32_t, void*, void*);
typedef void* (*pFpiipp_t)(void*, int32_t, int32_t, void*, void*);
typedef void* (*pFppppp_t)(void*, void*, void*, void*, void*);
typedef void* (*pFpippp_t)(void*, int32_t, void*, void*, void*);
typedef void* (*pFpuipp_t)(void*, uint32_t, int, void*, void*);
typedef void* (*pFpuppp_t)(void*, uint32_t, void*, void*, void*);
typedef void* (*pFpuupp_t)(void*, uint32_t, uint32_t, void*, void*);
typedef int (*iFppLpIi_t)(void*, void*, size_t, void*, int64_t, int32_t);

#if 0
#ifdef NOALIGN
typedef void (*vFipippV_t)(int, void*, int, void*, void*, va_list);
#else
typedef void (*vFipippV_t)(int, void*, int, void*, void*, void*);
#endif
    GO(pa_log_level_meta, vFipippV_t)           
#endif

#define SUPER() \
    GO(pa_mainloop_free, vFp_t)                 \
    GO(pa_mainloop_get_api, pFp_t)              \
    GO(pa_threaded_mainloop_free, vFp_t)        \
    GO(pa_threaded_mainloop_get_api, pFp_t)     \
    GO(pa_signal_init, iFp_t)                   \
    GO(pa_signal_new, pFipp_t)                  \
    GO(pa_signal_set_destroy, vFpp_t)           \
    GO(pa_context_new, pFpp_t)                  \
    GO(pa_context_new_with_proplist, pFppp_t)   \
    GO(pa_context_get_state, iFp_t)             \
    GO(pa_context_exit_daemon, pFppp_t)         \
    GO(pa_context_set_state_callback, vFppp_t)  \
    GO(pa_context_set_default_sink, pFpppp_t)   \
    GO(pa_context_set_default_source, pFpppp_t) \
    GO(pa_context_move_sink_input_by_index, pFpuupp_t)  \
    GO(pa_context_get_module_info_list, pFppp_t)        \
    GO(pa_context_get_client_info_list, pFppp_t)        \
    GO(pa_context_get_server_info, pFppp_t)     \
    GO(pa_context_get_sink_input_info, pFpupp_t)\
    GO(pa_context_get_sink_input_info_list, pFppp_t)    \
    GO(pa_context_get_sink_info_list, pFppp_t)  \
    GO(pa_context_get_sink_info_by_name, pFpppp_t)      \
    GO(pa_context_set_sink_input_mute, pFpuipp_t)       \
    GO(pa_context_set_sink_input_volume, pFpuppp_t)     \
    GO(pa_context_get_source_info_list, pFppp_t)\
    GO(pa_context_get_source_info_by_index, pFpupp_t)   \
    GO(pa_context_get_sink_info_by_index, pFpupp_t)     \
    GO(pa_context_set_sink_volume_by_index, pFpuppp_t)  \
    GO(pa_context_set_source_volume_by_index, pFpuppp_t)\
    GO(pa_context_set_source_mute_by_index, pFpuipp_t)  \
    GO(pa_context_unload_module, pFpupp_t)      \
    GO(pa_context_load_module, pFppppp_t)       \
    GO(pa_context_connect, iFppip_t)            \
    GO(pa_context_subscribe, pFpupp_t)          \
    GO(pa_context_set_subscribe_callback, vFppp_t)      \
    GO(pa_context_drain, pFppp_t)               \
    GO(pa_context_proplist_remove, pFpppp_t)    \
    GO(pa_context_proplist_update, pFpippp_t)   \
    GO(pa_context_set_event_callback, vFppp_t)  \
    GO(pa_context_set_name, pFpppp_t)           \
    GO(pa_context_set_source_volume_by_name, pFppppp_t) \
    GO(pa_context_get_source_info_by_name, pFpppp_t)    \
    GO(pa_stream_get_state, iFp_t)              \
    GO(pa_stream_drain, pFppp_t)                \
    GO(pa_stream_flush, pFppp_t)                \
    GO(pa_stream_set_latency_update_callback, vFppp_t)  \
    GO(pa_stream_set_read_callback, vFppp_t)    \
    GO(pa_stream_set_state_callback, vFppp_t)   \
    GO(pa_stream_write, iFppLpIi_t)             \
    GO(pa_stream_update_timing_info, pFppp_t)   \
    GO(pa_stream_prebuf, pFppp_t)               \
    GO(pa_stream_proplist_remove, pFpppp_t)     \
    GO(pa_stream_proplist_update, pFpippp_t)    \
    GO(pa_stream_set_buffer_attr, pFpppp_t)     \
    GO(pa_stream_set_buffer_attr_callback, vFppp_t)     \
    GO(pa_stream_set_event_callback, vFppp_t)   \
    GO(pa_stream_set_moved_callback, vFppp_t)   \
    GO(pa_stream_set_name, pFpppp_t)            \
    GO(pa_stream_set_overflow_callback, vFppp_t)\
    GO(pa_stream_set_started_callback, vFppp_t) \
    GO(pa_stream_set_suspended_callback, vFppp_t)       \
    GO(pa_stream_set_underflow_callback, vFppp_t)       \
    GO(pa_stream_set_write_callback, vFppp_t)   \
    GO(pa_stream_trigger, pFppp_t)              \
    GO(pa_stream_update_sample_rate, pFpupp_t)  \
    GO(pa_stream_cork, pFpipp_t)                \
    GO(pa_proplist_setf, iFppp_t)               \
    GO(pa_mainloop_set_poll_func, vFppp_t)      


typedef struct pulse_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} pulse_my_t;

void* getPulseMy(library_t* lib)
{
    pulse_my_t* my = (pulse_my_t*)calloc(1, sizeof(pulse_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO

    return my;
}
#undef SUPER

void freePulseMy(void* lib)
{
    //pulse_my_t *my = (pulse_my_t *)lib;

}

// TODO: change that static for a map ptr2ptr?
static my_pa_mainloop_api_t my_mainloop_api = {0};
static my_pa_mainloop_api_t my_mainloop_native = {0};
static int mainloop_inited = 0;
static my_pa_mainloop_api_t* my_mainloop_ref = NULL;
static my_pa_mainloop_api_t* my_mainloop_orig = NULL;


// utility functions
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
GO(15)  \


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
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio free callback\n");
    return NULL;
}

#define GO(A)   \
static uintptr_t my_free_api_fct_##A = 0;   \
static void my_free_api_##A(my_pa_mainloop_api_t* api, void* p, void* data)     \
{                                       \
    RunFunction(my_context, my_free_api_fct_##A, 3, api, p, data);\
}
SUPER()
#undef GO
static void* findFreeAPIFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_free_api_fct_##A == (uintptr_t)fct) return my_free_api_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_free_api_fct_##A == 0) {my_free_api_fct_##A = (uintptr_t)fct; return my_free_api_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio free api callback\n");
    return NULL;
}

#define GO(A)   \
static uintptr_t my_io_event_fct_##A = 0;   \
static void my_io_event_##A(my_pa_mainloop_api_t* api, void* e, int fd, int events, void* data)     \
{                                       \
    if(api==my_mainloop_orig) api=my_mainloop_ref;                  \
    RunFunction(my_context, my_io_event_fct_##A, 5, api, e, fd, events, data);  \
}
SUPER()
#undef GO
static void* findIOEventFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_io_event_fct_##A == (uintptr_t)fct) return my_io_event_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_io_event_fct_##A == 0) {my_io_event_fct_##A = (uintptr_t)fct; return my_io_event_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio io_event api callback\n");
    return NULL;
}

#define GO(A)   \
static uintptr_t my_time_event_fct_##A = 0;   \
static void my_time_event_##A(my_pa_mainloop_api_t* api, void* e, void* tv, void* data)     \
{                                       \
    if(api==my_mainloop_orig) api=my_mainloop_ref;                  \
    RunFunction(my_context, my_time_event_fct_##A, 4, api, e, tv, data);  \
}
SUPER()
#undef GO
static void* findTimeEventFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    if(!fct) return NULL;
    #define GO(A) if(my_time_event_fct_##A == (uintptr_t)fct) return my_time_event_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_time_event_fct_##A == 0) {my_time_event_fct_##A = (uintptr_t)fct; return my_time_event_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio time_event api callback\n");
    return NULL;
}

#define GO(A)   \
static uintptr_t my_defer_event_fct_##A = 0;   \
static void my_defer_event_##A(my_pa_mainloop_api_t* api, void* e, void* data)     \
{                                       \
    if(api==my_mainloop_orig) api=my_mainloop_ref;                  \
    RunFunction(my_context, my_defer_event_fct_##A, 3, api, e, data);  \
}
SUPER()
#undef GO
static void* findDeferEventFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    if(!fct) return NULL;
    #define GO(A) if(my_defer_event_fct_##A == (uintptr_t)fct) return my_defer_event_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_defer_event_fct_##A == 0) {my_defer_event_fct_##A = (uintptr_t)fct; return my_defer_event_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio defer_event api callback\n");
    return NULL;
}
// poll
#define GO(A)   \
static uintptr_t my_poll_fct_##A = 0;   \
static int my_poll_##A(void* ufds, unsigned long nfds, int timeout, void* data)         \
{                                       \
    return (int)RunFunction(my_context, my_poll_fct_##A, 4, ufds, nfds, timeout, data); \
}
SUPER()
#undef GO
static void* find_poll_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_poll_fct_##A == (uintptr_t)fct) return my_poll_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_poll_fct_##A == 0) {my_poll_fct_##A = (uintptr_t)fct; return my_poll_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio poll callback\n");
    return NULL;
}
// signal
#define GO(A)   \
static uintptr_t my_signal_fct_##A = 0;   \
static void my_signal_##A(my_pa_mainloop_api_t* api, void* e, int sig, void *data)  \
{                                                                                   \
    if(api==my_mainloop_orig) api=my_mainloop_ref;                                  \
    RunFunction(my_context, my_signal_fct_##A, 4, api, e, sig, data);               \
}
SUPER()
#undef GO
static void* find_signal_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_signal_fct_##A == (uintptr_t)fct) return my_signal_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_signal_fct_##A == 0) {my_signal_fct_##A = (uintptr_t)fct; return my_signal_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio signal callback\n");
    return NULL;
}
// signal_destroy
#define GO(A)   \
static uintptr_t my_signal_destroy_fct_##A = 0;   \
static void my_signal_destroy_##A(my_pa_mainloop_api_t* api, void* e, void *data)   \
{                                                                                   \
    if(api==my_mainloop_orig) api=my_mainloop_ref;                                  \
    RunFunction(my_context, my_signal_destroy_fct_##A, 3, api, e, data);            \
}
SUPER()
#undef GO
static void* find_signal_destroy_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_signal_destroy_fct_##A == (uintptr_t)fct) return my_signal_destroy_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_signal_destroy_fct_##A == 0) {my_signal_destroy_fct_##A = (uintptr_t)fct; return my_signal_destroy_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio signal_destroy callback\n");
    return NULL;
}

// prefork
#define GO(A)   \
static uintptr_t my_prefork_fct_##A = 0;            \
static void my_prefork_##A()                        \
{                                                   \
    RunFunction(my_context, my_prefork_fct_##A, 0); \
}
SUPER()
#undef GO
static void* find_prefork_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_prefork_fct_##A == (uintptr_t)fct) return my_prefork_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_prefork_fct_##A == 0) {my_prefork_fct_##A = (uintptr_t)fct; return my_prefork_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio prefork callback\n");
    return NULL;
}
// postfork
#define GO(A)   \
static uintptr_t my_postfork_fct_##A = 0;           \
static void my_postfork_##A()                       \
{                                                   \
    RunFunction(my_context, my_postfork_fct_##A, 0);\
}
SUPER()
#undef GO
static void* find_postfork_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_postfork_fct_##A == (uintptr_t)fct) return my_postfork_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_postfork_fct_##A == 0) {my_postfork_fct_##A = (uintptr_t)fct; return my_postfork_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio postfork callback\n");
    return NULL;
}
// atfork
#define GO(A)   \
static uintptr_t my_atfork_fct_##A = 0;             \
static void my_atfork_##A()                         \
{                                                   \
    RunFunction(my_context, my_atfork_fct_##A, 0);  \
}
SUPER()
#undef GO
static void* find_atfork_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_atfork_fct_##A == (uintptr_t)fct) return my_atfork_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_atfork_fct_##A == 0) {my_atfork_fct_##A = (uintptr_t)fct; return my_atfork_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio atfork callback\n");
    return NULL;
}

// state_context
#define GO(A)                                                               \
static uintptr_t my_state_context_fct_##A = 0;                              \
static void my_state_context_##A(void* context, void* data)                 \
{                                                                           \
    RunFunction(my_context, my_state_context_fct_##A, 2, context, data);    \
}
SUPER()
#undef GO
static void* find_state_context_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_state_context_fct_##A == (uintptr_t)fct) return my_state_context_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_state_context_fct_##A == 0) {my_state_context_fct_##A = (uintptr_t)fct; return my_state_context_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio state_context callback\n");
    return NULL;
}
// notify_context
#define GO(A)                                                               \
static uintptr_t my_notify_context_fct_##A = 0;                             \
static void my_notify_context_##A(void* context, void* data)                \
{                                                                           \
    RunFunction(my_context, my_notify_context_fct_##A, 2, context, data);   \
}
SUPER()
#undef GO
static void* find_notify_context_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_notify_context_fct_##A == (uintptr_t)fct) return my_notify_context_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_notify_context_fct_##A == 0) {my_notify_context_fct_##A = (uintptr_t)fct; return my_notify_context_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio notify_context callback\n");
    return NULL;
}
// success_context
#define GO(A)                                                                       \
static uintptr_t my_success_context_fct_##A = 0;                                    \
static void my_success_context_##A(void* context, int success, void* data)          \
{                                                                                   \
    RunFunction(my_context, my_success_context_fct_##A, 3, context, success, data); \
}
SUPER()
#undef GO
static void* find_success_context_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_success_context_fct_##A == (uintptr_t)fct) return my_success_context_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_success_context_fct_##A == 0) {my_success_context_fct_##A = (uintptr_t)fct; return my_success_context_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio success_context callback\n");
    return NULL;
}
// event_context
#define GO(A)                                                                       \
static uintptr_t my_event_context_fct_##A = 0;                                      \
static void my_event_context_##A(void* context, void* name, void* p, void* data)    \
{                                                                                   \
    RunFunction(my_context, my_event_context_fct_##A, 4, context, name, p, data);   \
}
SUPER()
#undef GO
static void* find_event_context_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_event_context_fct_##A == (uintptr_t)fct) return my_event_context_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_event_context_fct_##A == 0) {my_event_context_fct_##A = (uintptr_t)fct; return my_event_context_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio event_context callback\n");
    return NULL;
}
// module_info
#define GO(A)                                                                   \
static uintptr_t my_module_info_fct_##A = 0;                                    \
static void my_module_info_##A(void* context, void* i, int eol, void* data)     \
{                                                                               \
    RunFunction(my_context, my_module_info_fct_##A, 4, context, i, eol, data);  \
}
SUPER()
#undef GO
static void* find_module_info_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_module_info_fct_##A == (uintptr_t)fct) return my_module_info_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_module_info_fct_##A == 0) {my_module_info_fct_##A = (uintptr_t)fct; return my_module_info_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio module_info callback\n");
    return NULL;
}
// server_info
#define GO(A)                                                               \
static uintptr_t my_server_info_fct_##A = 0;                                \
static void my_server_info_##A(void* context, void* i, void* data)          \
{                                                                           \
    RunFunction(my_context, my_server_info_fct_##A, 3, context, i, data);   \
}
SUPER()
#undef GO
static void* find_server_info_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_server_info_fct_##A == (uintptr_t)fct) return my_server_info_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_server_info_fct_##A == 0) {my_server_info_fct_##A = (uintptr_t)fct; return my_server_info_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio server_info callback\n");
    return NULL;
}
// client_info
#define GO(A)                                                                   \
static uintptr_t my_client_info_fct_##A = 0;                                    \
static void my_client_info_##A(void* context, void* i, int eol, void* data)     \
{                                                                               \
    RunFunction(my_context, my_client_info_fct_##A, 4, context, i, eol, data);  \
}
SUPER()
#undef GO
static void* find_client_info_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_client_info_fct_##A == (uintptr_t)fct) return my_client_info_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_client_info_fct_##A == 0) {my_client_info_fct_##A = (uintptr_t)fct; return my_client_info_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio client_info callback\n");
    return NULL;
}
// context_index
#define GO(A)                                                                   \
static uintptr_t my_context_index_fct_##A = 0;                                  \
static void my_context_index_##A(void* context, uint32_t idx, void* data)       \
{                                                                               \
    RunFunction(my_context, my_context_index_fct_##A, 3, context, idx, data);   \
}
SUPER()
#undef GO
static void* find_context_index_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_context_index_fct_##A == (uintptr_t)fct) return my_context_index_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_context_index_fct_##A == 0) {my_context_index_fct_##A = (uintptr_t)fct; return my_context_index_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio context_index callback\n");
    return NULL;
}
// subscribe_context
#define GO(A)                                                                           \
static uintptr_t my_subscribe_context_fct_##A = 0;                                      \
static void my_subscribe_context_##A(void* context, int evt, uint32_t idx, void* data)  \
{                                                                                       \
    RunFunction(my_context, my_subscribe_context_fct_##A, 4, context, evt, idx, data);  \
}
SUPER()
#undef GO
static void* find_subscribe_context_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_subscribe_context_fct_##A == (uintptr_t)fct) return my_subscribe_context_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_subscribe_context_fct_##A == 0) {my_subscribe_context_fct_##A = (uintptr_t)fct; return my_subscribe_context_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio subscribe_context callback\n");
    return NULL;
}

// stream_state
#define GO(A)                                                       \
static uintptr_t my_stream_state_fct_##A = 0;                       \
static void my_stream_state_##A(void* s, void* data)                \
{                                                                   \
    RunFunction(my_context, my_stream_state_fct_##A, 2, s, data);   \
}
SUPER()
#undef GO
static void* find_stream_state_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_stream_state_fct_##A == (uintptr_t)fct) return my_stream_state_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_stream_state_fct_##A == 0) {my_stream_state_fct_##A = (uintptr_t)fct; return my_stream_state_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio stream_state callback\n");
    return NULL;
}
// stream_success
#define GO(A)                                                                   \
static uintptr_t my_stream_success_fct_##A = 0;                                 \
static void my_stream_success_##A(void* s, int success, void* data)             \
{                                                                               \
    RunFunction(my_context, my_stream_success_fct_##A, 3, s, success, data);    \
}
SUPER()
#undef GO
static void* find_stream_success_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_stream_success_fct_##A == (uintptr_t)fct) return my_stream_success_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_stream_success_fct_##A == 0) {my_stream_success_fct_##A = (uintptr_t)fct; return my_stream_success_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio stream_success callback\n");
    return NULL;
}
// stream_notify
#define GO(A)                                                       \
static uintptr_t my_stream_notify_fct_##A = 0;                      \
static void my_stream_notify_##A(void* s, void* data)               \
{                                                                   \
    RunFunction(my_context, my_stream_notify_fct_##A, 2, s, data);  \
}
SUPER()
#undef GO
static void* find_stream_notify_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_stream_notify_fct_##A == (uintptr_t)fct) return my_stream_notify_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_stream_notify_fct_##A == 0) {my_stream_notify_fct_##A = (uintptr_t)fct; return my_stream_notify_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio stream_notify callback\n");
    return NULL;
}
// stream_event
#define GO(A)                                                               \
static uintptr_t my_stream_event_fct_##A = 0;                               \
static void my_stream_event_##A(void* s, void* name, void* pl, void* data)  \
{                                                                           \
    RunFunction(my_context, my_stream_event_fct_##A, 4, s, name, pl, data); \
}
SUPER()
#undef GO
static void* find_stream_event_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_stream_event_fct_##A == (uintptr_t)fct) return my_stream_event_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_stream_event_fct_##A == 0) {my_stream_event_fct_##A = (uintptr_t)fct; return my_stream_event_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio stream_event callback\n");
    return NULL;
}
// stream_request
#define GO(A)                                                               \
static uintptr_t my_stream_request_fct_##A = 0;                             \
static void my_stream_request_##A(void* s, size_t nbytes, void* data)       \
{                                                                           \
    RunFunction(my_context, my_stream_request_fct_##A, 3, s, nbytes, data); \
}
SUPER()
#undef GO
static void* find_stream_request_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_stream_request_fct_##A == (uintptr_t)fct) return my_stream_request_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_stream_request_fct_##A == 0) {my_stream_request_fct_##A = (uintptr_t)fct; return my_stream_request_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio stream_request callback\n");
    return NULL;
}

#undef SUPER


// Mainloop functions
//  but first, all cb from the pa_mainloop_api (serioulsy, how many callback and callback inside callback there is in pulse audio?!!!)

// Native version of the mailoop_api. Called from x64 space

static void* native_io_new(void* api, int fd, int events, void* cb, void *data)
{
    if(api==my_mainloop_ref) api=my_mainloop_orig;    // need native version
    // need to wrap the callback!
    void* c = findIOEventFct(cb);
    return ((pFpiipp_t)my_mainloop_native.io_new)(api, fd, events, c, data);
}
static void native_io_enable(void* e, int events)
{
    return ((vFpi_t)my_mainloop_native.io_enable)(e, events);
}
static void native_io_free(void* e)
{
    return ((vFp_t)my_mainloop_native.io_free)(e);
}
static void native_io_set_destroy(void* e, void* cb)
{
    // need to wrap the callback!
    void* c = findFreeAPIFct(cb);
    return ((vFpp_t)my_mainloop_native.io_set_destroy)(e, c);
}

static void* native_time_new(void* api, void* tv, void* cb, void* data)
{
    if(api==my_mainloop_ref) api=my_mainloop_orig;    // need native version
    // need to wrap the callback!
    void* c = findTimeEventFct(cb);
    return ((pFpppp_t)my_mainloop_native.time_new)(api, tv, c, data);
}
static void native_time_restart(void* e, void* tv)
{
    return ((vFpp_t)my_mainloop_native.time_restart)(e, tv);
}
static void native_time_free(void* e)
{
    return ((vFp_t)my_mainloop_native.time_free)(e);
}
static void native_time_set_destroy(void* e, void* cb)
{
    // need to wrap the callback!
    void* c = findFreeAPIFct(cb);
    return ((vFpp_t)my_mainloop_native.time_set_destroy)(e, c);
}

static void* native_defer_new(void* api, void* cb, void* data)
{
    if(api==my_mainloop_ref) api=my_mainloop_orig;    // need native version
    // need to wrap the callback!
    void* c = findDeferEventFct(cb);
    return ((pFppp_t)my_mainloop_native.defer_new)(api, c, data);
}
static void native_defer_enable(void* e, int b)
{
    return ((vFpi_t)my_mainloop_native.defer_enable)(e, b);
}
static void native_defer_free(void* e)
{
    return ((vFp_t)my_mainloop_native.defer_free)(e);
}
static void native_defer_set_destroy(void* e, void* cb)
{
    // need to wrap the callback!
    void* c = findFreeAPIFct(cb);
    return ((vFpp_t)my_mainloop_native.defer_set_destroy)(e, c);
}

static void native_quit(void* api, int retval)
{
    if(api==my_mainloop_ref) api=my_mainloop_orig;    // need native version
    return ((vFpi_t)my_mainloop_native.quit)(api, retval);
}

// Emulated version. Called from native space (meh, crossing stuff are hard to follow)
static void* my_io_new(void* api, int fd, int events, void* cb, void *userdata)
{
    uintptr_t b = (uintptr_t)cb;
    //pulse_my_t* my = (pulse_my_t*)my_context->pulse->priv.w.p2;

    void* fnc = GetNativeFnc((uintptr_t)my_mainloop_ref->io_new);
    if(fnc) {
        if(fnc==native_io_new) fnc=my_mainloop_native.io_new;
        if(api==my_mainloop_ref) api=my_mainloop_orig;    // need native version
        return ((pFpiipp_t)fnc)(api, fd, events, cb, userdata);
    }

    bridge_t* bridge = my_context->pulse->priv.w.bridge;
    if(cb)
        b = AddCheckBridge(bridge, vFppiip, cb, 0, NULL);
    if(api==my_mainloop_orig) api=my_mainloop_ref;    // need emulated version
    return (void*)RunFunction(my_context, (uintptr_t)my_mainloop_ref->io_new, 5, api, fd, events, b, userdata);
}
static void my_io_enable(void* e, int events)
{
    void* fnc = GetNativeFnc((uintptr_t)my_mainloop_ref->io_enable);
    if(fnc==native_io_enable) fnc=my_mainloop_native.io_enable;
    if(fnc)
        return ((vFpi_t)fnc)(e, events);

    RunFunction(my_context, (uintptr_t)my_mainloop_ref->io_enable, 2, e, events);
}
static void my_io_free(void* e)
{
    void* fnc = GetNativeFnc((uintptr_t)my_mainloop_ref->io_free);
    if(fnc==native_io_free) fnc=my_mainloop_native.io_free;
    if(fnc)
        return ((vFp_t)fnc)(e);

    RunFunction(my_context, (uintptr_t)my_mainloop_ref->io_free, 1, e);
}
static void my_io_set_destroy(void* e, void* cb)
{
    void* fnc = GetNativeFnc((uintptr_t)my_mainloop_ref->io_set_destroy);
    if(fnc==native_io_set_destroy) fnc=my_mainloop_native.io_set_destroy;
    if(fnc)
        return ((vFpp_t)fnc)(e, cb);

    bridge_t* bridge = my_context->pulse->priv.w.bridge;
    uintptr_t b = 0;
    if(cb) {
        b = CheckBridged(bridge, cb);
        if(!b)
            b = AddBridge(bridge, vFppp, cb, 0, NULL);
    }
    RunFunction(my_context, (uintptr_t)my_mainloop_ref->io_set_destroy, 2, e, b);
}

static void* my_time_new(void* api, void* tv, void* cb, void* data)
{
    uintptr_t b = (uintptr_t)cb;
    //pulse_my_t* my = (pulse_my_t*)my_context->pulse->priv.w.p2;

    void* fnc = GetNativeFnc((uintptr_t)my_mainloop_ref->time_new);
    if(fnc) {
        if(fnc==native_time_new) fnc=my_mainloop_native.time_new;
        if(api==my_mainloop_ref) api=my_mainloop_orig;    // need native version
        return ((pFpppp_t)fnc)(api, tv, (void*)b, data);
    }

    // need to bridge the callback!
    bridge_t* bridge = my_context->pulse->priv.w.bridge;
    if(cb)
        b = AddCheckBridge(bridge, vFpppp, cb, 0, NULL);
    if(api==my_mainloop_orig) api=my_mainloop_ref;    // need emulated version
    return (void*)RunFunction(my_context, (uintptr_t)my_mainloop_ref->time_new, 4, api, tv, b, data);
}
static void my_time_restart(void* e, void* tv)
{
    void* fnc = GetNativeFnc((uintptr_t)my_mainloop_ref->time_restart);
    if(fnc==native_time_restart) fnc=my_mainloop_native.time_restart;
    if(fnc)
        return ((vFpp_t)fnc)(e, tv);

    RunFunction(my_context, (uintptr_t)my_mainloop_ref->time_restart, 2, e, tv);
}
static void my_time_free(void* e)
{
    void* fnc = GetNativeFnc((uintptr_t)my_mainloop_ref->time_free);
    if(fnc==native_time_free) fnc=my_mainloop_native.time_free;
    if(fnc)
        return ((vFp_t)fnc)(e);

    RunFunction(my_context, (uintptr_t)my_mainloop_ref->time_free, 1, e);
}
static void my_time_set_destroy(void* e, void* cb)
{
    void* fnc = GetNativeFnc((uintptr_t)my_mainloop_ref->time_set_destroy);
    if(fnc==native_time_set_destroy) fnc=my_mainloop_native.time_set_destroy;
    if(fnc)
        return ((vFpp_t)fnc)(e, cb);

    bridge_t* bridge = my_context->pulse->priv.w.bridge;
    uintptr_t b = 0;
    if(cb)
            b = AddCheckBridge(bridge, vFppp, cb, 0, NULL);
    RunFunction(my_context, (uintptr_t)my_mainloop_ref->time_set_destroy, 2, e, b);
}

static void* my_defer_new(void* api, void* cb, void* data)
{
    uintptr_t b = (uintptr_t)cb;
    //pulse_my_t* my = (pulse_my_t*)my_context->pulse->priv.w.p2;

    void* fnc = GetNativeFnc((uintptr_t)my_mainloop_ref->defer_new);
    if(fnc) {
        if(api==my_mainloop_ref) api=my_mainloop_orig;    // need native version
        if(fnc==native_defer_new) fnc=my_mainloop_native.defer_new;
        return ((pFppp_t)fnc)(api, cb, data);
    }

    // need to bridge the callback!
    bridge_t* bridge = my_context->pulse->priv.w.bridge;
    if(cb) {
        b = CheckBridged(bridge, cb);
        if(!b)
            b = AddBridge(bridge, vFppp, cb, 0, NULL);
    }
    if(api==my_mainloop_orig) api=my_mainloop_ref;    // need emulated version
    return (void*)RunFunction(my_context, (uintptr_t)my_mainloop_ref->defer_new, 3, api, b, data);
}
static void my_defer_enable(void* e, int b)
{
    void* fnc = GetNativeFnc((uintptr_t)my_mainloop_ref->defer_enable);
    if(fnc==native_defer_enable) fnc=my_mainloop_native.defer_enable;
    if(fnc)
        return ((vFpi_t)fnc)(e, b);

    RunFunction(my_context, (uintptr_t)my_mainloop_ref->defer_enable, 2, e, b);
}
static void my_defer_free(void* e)
{
    void* fnc = GetNativeFnc((uintptr_t)my_mainloop_ref->defer_free);
    if(fnc==native_defer_free) fnc=my_mainloop_native.defer_free;
    if(fnc)
        return ((vFp_t)fnc)(e);

    RunFunction(my_context, (uintptr_t)my_mainloop_ref->defer_free, 1, e);
}
static void my_defer_set_destroy(void* e, void* cb)
{
    void* fnc = GetNativeFnc((uintptr_t)my_mainloop_ref->defer_set_destroy);
    if(fnc==native_defer_set_destroy) fnc=my_mainloop_native.defer_set_destroy;
    if(fnc)
        return ((vFpp_t)fnc)(e, cb);

    bridge_t* bridge = my_context->pulse->priv.w.bridge;
    uintptr_t b = 0;
    if(cb)
        b = AddCheckBridge(bridge, vFppp, cb, 0, NULL);
    RunFunction(my_context, (uintptr_t)my_mainloop_ref->defer_set_destroy, 2, e, b);
}

static void my_quit(void* api, int retval)
{
    void* fnc = GetNativeFnc((uintptr_t)my_mainloop_ref->quit);
    if(fnc) {
        if(fnc==native_quit) fnc=my_mainloop_native.quit;
        if(api==my_mainloop_ref) api=my_mainloop_orig;    // need native version
        return ((vFpi_t)fnc)(api, retval);
    }

    if(api==my_mainloop_orig) api=my_mainloop_ref;    // need emulated version
    RunFunction(my_context, (uintptr_t)my_mainloop_ref->quit, 2, api, retval);
}

static void bridgeMainloopAPI(bridge_t* bridge, my_pa_mainloop_api_t* api)
{
    if(!api) {
        return;
    }
    #define GO(A, W) my_mainloop_native.A = api->A; if(api->A) {my_mainloop_api.A = (void*)AddCheckBridge(bridge, W, native_##A, 0, NULL); api->A=my_##A;} else my_mainloop_api.A = NULL
    GO(io_new, pFpiipp);
    GO(io_enable, vFpi);
    GO(io_free, vFp);
    GO(io_set_destroy, vFpp);
    GO(time_new, pFpppp);
    GO(time_restart, vFpp);
    GO(time_free, vFp);
    GO(time_set_destroy, vFpp);
    GO(defer_new, pFppp);
    GO(defer_enable, vFpi);
    GO(defer_free, vFp);
    GO(defer_set_destroy, vFpp);
    GO(quit, vFpi);
    #undef GO
    my_mainloop_api.data = api->data;
    my_mainloop_orig = api;
    my_mainloop_ref = &my_mainloop_api;
    return;
}
/*static my_pa_mainloop_api_t* backMainloopAPI(my_pa_mainloop_api_t* mainloop)
{
    if(my_mainloop_ref!=mainloop) {
        printf_log(LOG_NONE, "Warning, Pulse mainloop_api is not expected value\n");
        return mainloop;
    }
    return my_mainloop_orig;
}*/

// mainloop_api: all the functions are wrapped, with custom function used...
// and a copy is sent to the emulated software. copy use wrapped function
// only one mainloop can be active at a given time!
EXPORT void my_pa_mainloop_free(x64emu_t* emu, void* mainloop)
{
    library_t* lib = emu->context->pulse;
    pulse_my_t* my = lib->priv.w.p2;
    my->pa_mainloop_free(mainloop);
    mainloop_inited = 0;
    /*my_mainloop_ref =*/ my_mainloop_orig = NULL;
}
EXPORT void* my_pa_mainloop_get_api(x64emu_t* emu, void* mainloop)
{
    library_t* lib = emu->context->pulse;
    pulse_my_t* my = lib->priv.w.p2;
    my_pa_mainloop_api_t* api = my->pa_mainloop_get_api(mainloop);
    bridgeMainloopAPI(lib->priv.w.bridge, api);
    return my_mainloop_ref;
}

EXPORT void my_pa_threaded_mainloop_free(x64emu_t* emu, void* mainloop)
{
    library_t* lib = emu->context->pulse;
    pulse_my_t* my = lib->priv.w.p2;
    my->pa_threaded_mainloop_free(mainloop);
    mainloop_inited = 0;
    /*my_mainloop_ref =*/ my_mainloop_orig = NULL;
}
EXPORT void* my_pa_threaded_mainloop_get_api(x64emu_t* emu, void* mainloop)
{
    library_t* lib = emu->context->pulse;
    pulse_my_t* my = lib->priv.w.p2;
    my_pa_mainloop_api_t* api = my->pa_threaded_mainloop_get_api(mainloop);
    bridgeMainloopAPI(lib->priv.w.bridge, api);
    return my_mainloop_ref;
}

// Context functions
EXPORT void* my_pa_context_new(x64emu_t* emu, my_pa_mainloop_api_t* mainloop, void* name)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    if(mainloop==my_mainloop_ref) mainloop=my_mainloop_orig;    // need native version
    return my->pa_context_new(mainloop, name);
}

EXPORT void* my_pa_context_new_with_proplist(x64emu_t* emu, my_pa_mainloop_api_t* mainloop, void* name, void* proplist)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    if(mainloop==my_mainloop_ref) mainloop=my_mainloop_orig;    // need native version
    return my->pa_context_new_with_proplist(mainloop, name, proplist);
}

EXPORT int my_pa_signal_init(x64emu_t* emu, my_pa_mainloop_api_t* mainloop)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    if(mainloop==my_mainloop_ref) mainloop=my_mainloop_orig;    // need native version
    return my->pa_signal_init(mainloop);
}

EXPORT void* my_pa_signal_new(x64emu_t* emu, int sig, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    return my->pa_signal_new(sig, find_signal_Fct(cb), data);
}

EXPORT void my_pa_signal_set_destroy(x64emu_t* emu, void* e, void* cb)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    return my->pa_signal_set_destroy(e, find_signal_destroy_Fct(cb));
}

typedef struct my_pa_spawn_api_s {
    void* prefork;
    void* postfork;
    void* atfork;
} my_pa_spawn_api_t;

EXPORT int my_pa_context_connect(x64emu_t* emu, void* context, void* server, int flags, my_pa_spawn_api_t* api)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    if(!api) {
        return my->pa_context_connect(context, server, flags, api);
    }
    static my_pa_spawn_api_t a = {0};
    #define GO(A) a.A = find_##A##_Fct(api->A)
    GO(prefork);
    GO(postfork);
    GO(atfork);
    #undef GO
    return my->pa_context_connect(context, server, flags, &a);
}

EXPORT void my_pa_context_set_state_callback(x64emu_t* emu, void* context, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my->pa_context_set_state_callback(context, find_state_context_Fct(cb), data);
}

EXPORT void my_pa_context_set_default_sink(x64emu_t* emu, void* context, void* name, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my->pa_context_set_default_sink(context, name, find_success_context_Fct(cb), data);
}

EXPORT void my_pa_context_set_default_source(x64emu_t* emu, void* context, void* name, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my->pa_context_set_default_source(context, name, find_success_context_Fct(cb), data);
}

EXPORT void* my_pa_context_move_sink_input_by_index(x64emu_t* emu, void* context, uint32_t idx, uint32_t sink_idx, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    return my->pa_context_move_sink_input_by_index(context, idx, sink_idx, find_success_context_Fct(cb), data);
}

EXPORT void* my_pa_context_get_module_info_list(x64emu_t* emu, void* context, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    return my->pa_context_get_module_info_list(context, find_module_info_Fct(cb), data);
}

EXPORT void* my_pa_context_get_server_info(x64emu_t* emu, void* context, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    return my->pa_context_get_server_info(context, find_server_info_Fct(cb), data);
}

EXPORT void* my_pa_context_get_client_info_list(x64emu_t* emu, void* context, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    return my->pa_context_get_client_info_list(context, find_client_info_Fct(cb), data);
}

EXPORT void* my_pa_context_get_sink_input_info(x64emu_t* emu, void* context, uint32_t idx, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    return my->pa_context_get_sink_input_info(context, idx, find_module_info_Fct(cb), data);
}
EXPORT void* my_pa_context_get_sink_input_info_list(x64emu_t* emu, void* context, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    return my->pa_context_get_sink_input_info_list(context, find_module_info_Fct(cb), data);
}

EXPORT void* my_pa_context_get_sink_info_list(x64emu_t* emu, void* context, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    return my->pa_context_get_sink_info_list(context, find_module_info_Fct(cb), data);
}

EXPORT void* my_pa_context_get_sink_info_by_name(x64emu_t* emu, void* context, void* name, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    return my->pa_context_get_sink_info_by_name(context, name, find_module_info_Fct(cb), data);
}

EXPORT void* my_pa_context_get_source_info_list(x64emu_t* emu, void* context, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    return my->pa_context_get_source_info_list(context, find_module_info_Fct(cb), data);
}

EXPORT void my_pa_context_set_sink_input_mute(x64emu_t* emu, void* context, uint32_t idx, int mute, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my->pa_context_set_sink_input_mute(context, idx, mute, find_success_context_Fct(cb), data);
}

EXPORT void my_pa_context_set_sink_input_volume(x64emu_t* emu, void* context, uint32_t idx, void* volume, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my->pa_context_set_sink_input_volume(context, idx, volume, find_success_context_Fct(cb), data);
}

EXPORT void* my_pa_context_get_sink_info_by_index(x64emu_t* emu, void* context, uint32_t idx, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    return my->pa_context_get_sink_info_by_index(context, idx, find_module_info_Fct(cb), data);
}

EXPORT void* my_pa_context_get_source_info_by_index(x64emu_t* emu, void* context, uint32_t idx, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    return my->pa_context_get_source_info_by_index(context, idx, find_module_info_Fct(cb), data);
}

EXPORT void my_pa_context_set_source_volume_by_index(x64emu_t* emu, void* context, uint32_t idx, void* volume, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my->pa_context_set_source_volume_by_index(context, idx, volume, find_success_context_Fct(cb), data);
}

EXPORT void my_pa_context_set_source_mute_by_index(x64emu_t* emu, void* context, uint32_t idx, int mute, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my->pa_context_set_source_mute_by_index(context, idx, mute, find_success_context_Fct(cb), data);
}

EXPORT void my_pa_context_set_sink_volume_by_index(x64emu_t* emu, void* context, uint32_t idx, void* volume, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my->pa_context_set_sink_volume_by_index(context, idx, volume, find_success_context_Fct(cb), data);
}

EXPORT void* my_pa_context_unload_module(x64emu_t* emu, void* context, uint32_t idx, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    return my->pa_context_unload_module(context, idx, find_success_context_Fct(cb), data);
}

EXPORT void* my_pa_context_load_module(x64emu_t* emu, void* context, void* name, void* arg, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    return my->pa_context_load_module(context, name, arg, find_context_index_Fct(cb), data);
}

EXPORT void* my_pa_context_subscribe(x64emu_t* emu, void* context, uint32_t m, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    return my->pa_context_subscribe(context, m, find_success_context_Fct(cb), data);
}

EXPORT void my_pa_context_set_subscribe_callback(x64emu_t* emu, void* context, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    return my->pa_context_set_subscribe_callback(context, find_subscribe_context_Fct(cb), data);
}

EXPORT void* my_pa_context_drain(x64emu_t* emu, void* context, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    return my->pa_context_drain(context, find_notify_context_Fct(cb), data);
}

EXPORT void* my_pa_context_exit_daemon(x64emu_t* emu, void* context, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    return my->pa_context_exit_daemon(context, find_success_context_Fct(cb), data);
}

EXPORT void* my_pa_context_proplist_remove(x64emu_t* emu, void* context, void* keys, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    return my->pa_context_proplist_remove(context, keys, find_success_context_Fct(cb), data);
}

EXPORT void* my_pa_context_proplist_update(x64emu_t* emu, void* context, int mode, void* p, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    return my->pa_context_proplist_update(context, mode, p, find_success_context_Fct(cb), data);
}

EXPORT void my_pa_context_set_event_callback(x64emu_t* emu, void* context, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my->pa_context_set_event_callback(context, find_event_context_Fct(cb), data);
}

EXPORT void* my_pa_context_set_name(x64emu_t* emu, void* context, void* name, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    return my->pa_context_set_name(context, name, find_success_context_Fct(cb), data);
}

EXPORT void* my_pa_context_set_source_volume_by_name(x64emu_t* emu, void* context, void* name,void* volume, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    return my->pa_context_set_source_volume_by_name(context, name, volume, find_success_context_Fct(cb), data);
}

EXPORT void* my_pa_context_get_source_info_by_name(x64emu_t* emu, void* context, void* name, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    return my->pa_context_get_source_info_by_name(context, name, find_module_info_Fct(cb), data);
}

// Stream functions

EXPORT void* my_pa_stream_drain(x64emu_t* emu, void* stream, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    return my->pa_stream_drain(stream, find_stream_success_Fct(cb), data);
}

EXPORT void* my_pa_stream_flush(x64emu_t* emu, void* stream, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    return my->pa_stream_flush(stream, find_stream_success_Fct(cb), data);
}

EXPORT void my_pa_stream_set_latency_update_callback(x64emu_t* emu, void* stream, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my->pa_stream_set_latency_update_callback(stream, find_stream_notify_Fct(cb), data);
}

EXPORT void my_pa_stream_set_read_callback(x64emu_t* emu, void* stream, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my->pa_stream_set_read_callback(stream, find_stream_request_Fct(cb), data);
}

EXPORT int my_pa_stream_write(x64emu_t* emu, void* stream, void* d, size_t nbytes, void* cb, int64_t offset, int seek)
{
    if(!emu->context->pulse)
        return 0;
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    if(!my)
        return 0;
    return my->pa_stream_write(stream, d, nbytes, findFreeFct(cb), offset, seek);
}

EXPORT void* my_pa_stream_update_timing_info(x64emu_t* emu, void* stream, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    return my->pa_stream_update_timing_info(stream, find_stream_success_Fct(cb), data);
}

EXPORT void* my_pa_stream_prebuf(x64emu_t* emu, void* stream, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    return my->pa_stream_prebuf(stream, find_stream_success_Fct(cb), data);
}

EXPORT void* my_pa_stream_proplist_remove(x64emu_t* emu, void* stream, void* keys, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    return my->pa_stream_proplist_remove(stream, keys, find_stream_success_Fct(cb), data);
}

EXPORT void* my_pa_stream_proplist_update(x64emu_t* emu, void* stream, int32_t mode, void* p, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    return my->pa_stream_proplist_update(stream, mode, p, find_stream_success_Fct(cb), data);
}

EXPORT void* my_pa_stream_set_buffer_attr(x64emu_t* emu, void* stream, void* attr, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    return my->pa_stream_set_buffer_attr(stream, attr, find_stream_success_Fct(cb), data);
}

EXPORT void my_pa_stream_set_buffer_attr_callback(x64emu_t* emu, void* stream, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my->pa_stream_set_buffer_attr_callback(stream, find_stream_notify_Fct(cb), data);
}

EXPORT void my_pa_stream_set_event_callback(x64emu_t* emu, void* stream, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my->pa_stream_set_event_callback(stream, find_stream_event_Fct(cb), data);
}

EXPORT void my_pa_stream_set_moved_callback(x64emu_t* emu, void* stream, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my->pa_stream_set_moved_callback(stream, find_stream_notify_Fct(cb), data);
}

EXPORT void* my_pa_stream_set_name(x64emu_t* emu, void* stream, void* name, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    return my->pa_stream_set_name(stream, name, find_stream_success_Fct(cb), data);
}

EXPORT void my_pa_stream_set_overflow_callback(x64emu_t* emu, void* stream, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my->pa_stream_set_overflow_callback(stream, find_stream_notify_Fct(cb), data);
}

EXPORT void my_pa_stream_set_started_callback(x64emu_t* emu, void* stream, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my->pa_stream_set_started_callback(stream, find_stream_notify_Fct(cb), data);
}

EXPORT void my_pa_stream_set_state_callback(x64emu_t* emu, void* stream, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my->pa_stream_set_state_callback(stream, find_stream_state_Fct(cb), data);
}

EXPORT void my_pa_stream_set_suspended_callback(x64emu_t* emu, void* stream, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my->pa_stream_set_suspended_callback(stream, find_stream_notify_Fct(cb), data);
}

EXPORT void my_pa_stream_set_underflow_callback(x64emu_t* emu, void* stream, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my->pa_stream_set_underflow_callback(stream, find_stream_notify_Fct(cb), data);
}

EXPORT void my_pa_stream_set_write_callback(x64emu_t* emu, void* stream, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my->pa_stream_set_write_callback(stream, find_stream_request_Fct(cb), data);
}

EXPORT void* my_pa_stream_trigger(x64emu_t* emu, void* stream, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    return my->pa_stream_trigger(stream, find_stream_success_Fct(cb), data);
}

EXPORT void* my_pa_stream_update_sample_rate(x64emu_t* emu, void* stream, uint32_t rate, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    return my->pa_stream_update_sample_rate(stream, rate, find_stream_success_Fct(cb), data);
}

EXPORT void* my_pa_stream_cork(x64emu_t* emu, void* stream, int32_t b, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    return my->pa_stream_cork(stream, b, find_stream_success_Fct(cb), data);
}

EXPORT int my_pa_proplist_setf(x64emu_t* emu, void* p, void* key, void* fmt, uint64_t* b)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    char* format;
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 3);
    PREPARE_VALIST;
    int dummy = vasprintf(&format, fmt, VARARGS);
    (void)dummy;
    int ret = my->pa_proplist_setf(p, key, format);
    free(format);
    return ret;
}

EXPORT void my_pa_mainloop_set_poll_func(x64emu_t* emu, void* m, void* f, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;

    my->pa_mainloop_set_poll_func(m, find_poll_Fct(f), data);
}

#if 0
EXPORT void my_pa_log_level_meta(x64emu_t* emu, int level, void* file, int line, void* func, void* format, void* b, va_list V)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign((const char*)format, b, emu->scratch);
    PREPARE_VALIST;
    return my->pa_log_level_meta(level, file, line, func, format, emu->scratch);
    #else
    return my->pa_log_level_meta(level, file, line, func, format, V);
    #endif
}
#endif

#define PRE_INIT        \
    if(box64_nopulse)   \
        return -1;

#define CUSTOM_INIT \
    lib->priv.w.p2 = getPulseMy(lib);   \
    box64->pulse = lib;                 \


#define CUSTOM_FINI \
    lib->context->pulse = NULL;     \
    freePulseMy(lib->priv.w.p2);    \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"

