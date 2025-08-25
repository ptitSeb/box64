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
#define ALTNAME "libpulse.so"

#define LIBNAME pulse

// TODO: check my_pa_proplist_setf (not using generated/...)

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

typedef void (*vFpi_t)(void*, int32_t);
typedef int (*iFppp_t)(void*, void*, void*);
typedef void* (*pFpiipp_t)(void*, int32_t, int32_t, void*, void*);

#if 0
#ifdef NOALIGN
typedef void (*vFipippV_t)(int, void*, int, void*, void*, va_list);
#else
typedef void (*vFipippV_t)(int, void*, int, void*, void*, void*);
#endif
    GO(pa_log_level_meta, vFipippV_t)
#endif

#define ADDED_FUNCTIONS() \


#include "generated/wrappedpulsetypes.h"

#include "wrappercallback.h"

// TODO: change that static for a map ptr2ptr?
static my_pa_mainloop_api_t my_mainloop_api = {0};
static void UpdateautobridgeMainloopAPI(x64emu_t* emu, bridge_t* bridge, my_pa_mainloop_api_t* api);

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
    RunFunctionFmt(my_free_fct_##A, "p", data);\
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
    RunFunctionFmt(my_free_api_fct_##A, "ppp", api, p, data);\
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

#define GO(A)                                                                                       \
static uintptr_t my_io_event_fct_##A = 0;                                                           \
static void my_io_event_##A(my_pa_mainloop_api_t* api, void* e, int fd, int events, void* data)     \
{                                                                                                   \
    UpdateautobridgeMainloopAPI(thread_get_emu(), my_lib->w.bridge, api);                           \
    RunFunctionFmt(my_io_event_fct_##A, "ppiip", api, e, fd, events, data);                         \
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

#define GO(A)                                                                               \
static uintptr_t my_time_event_fct_##A = 0;                                                 \
static void my_time_event_##A(my_pa_mainloop_api_t* api, void* e, void* tv, void* data)     \
{                                                                                           \
    UpdateautobridgeMainloopAPI(thread_get_emu(), my_lib->w.bridge, api);                   \
    RunFunctionFmt(my_time_event_fct_##A, "pppp", api, e, tv, data);                        \
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

#define GO(A)                                                                       \
static uintptr_t my_defer_event_fct_##A = 0;                                        \
static void my_defer_event_##A(my_pa_mainloop_api_t* api, void* e, void* data)      \
{                                                                                   \
    UpdateautobridgeMainloopAPI(thread_get_emu(), my_lib->w.bridge, api);           \
    RunFunctionFmt(my_defer_event_fct_##A, "ppp", api, e, data);                    \
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
    return (int)RunFunctionFmt(my_poll_fct_##A, "pLip", ufds, nfds, timeout, data); \
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
    UpdateautobridgeMainloopAPI(thread_get_emu(), my_lib->w.bridge, api);           \
    RunFunctionFmt(my_signal_fct_##A, "ppip", api, e, sig, data);                   \
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
    UpdateautobridgeMainloopAPI(thread_get_emu(), my_lib->w.bridge, api);           \
    RunFunctionFmt(my_signal_destroy_fct_##A, "ppp", api, e, data);                 \
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
    RunFunctionFmt(my_prefork_fct_##A, ""); \
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
    RunFunctionFmt(my_postfork_fct_##A, "");\
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
    RunFunctionFmt(my_atfork_fct_##A, "");  \
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
    RunFunctionFmt(my_state_context_fct_##A, "pp", context, data);    \
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
    RunFunctionFmt(my_notify_context_fct_##A, "pp", context, data);   \
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
    RunFunctionFmt(my_success_context_fct_##A, "pip", context, success, data); \
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
    RunFunctionFmt(my_event_context_fct_##A, "pppp", context, name, p, data);   \
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
    RunFunctionFmt(my_module_info_fct_##A, "ppip", context, i, eol, data);  \
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
    RunFunctionFmt(my_server_info_fct_##A, "ppp", context, i, data);   \
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
    RunFunctionFmt(my_client_info_fct_##A, "ppip", context, i, eol, data);  \
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
    RunFunctionFmt(my_context_index_fct_##A, "pup", context, idx, data);   \
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
    RunFunctionFmt(my_subscribe_context_fct_##A, "piup", context, evt, idx, data);  \
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
    RunFunctionFmt(my_stream_state_fct_##A, "pp", s, data);   \
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
    RunFunctionFmt(my_stream_success_fct_##A, "pip", s, success, data);    \
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
    RunFunctionFmt(my_stream_notify_fct_##A, "pp", s, data);  \
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
    RunFunctionFmt(my_stream_event_fct_##A, "pppp", s, name, pl, data); \
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
    RunFunctionFmt(my_stream_request_fct_##A, "pLp", s, nbytes, data); \
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
// device_restore_read_device_formats
#define GO(A)                                                                               \
static uintptr_t my_device_restore_read_device_formats_fct_##A = 0;                         \
static void my_device_restore_read_device_formats_##A(void* a, void* b, int c, void* d)     \
{                                                                                           \
    RunFunctionFmt(my_device_restore_read_device_formats_fct_##A, "ppip", a, b, c, d);  \
}
SUPER()
#undef GO
static void* find_device_restore_read_device_formats_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_device_restore_read_device_formats_fct_##A == (uintptr_t)fct) return my_device_restore_read_device_formats_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_device_restore_read_device_formats_fct_##A == 0) {my_device_restore_read_device_formats_fct_##A = (uintptr_t)fct; return my_device_restore_read_device_formats_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio device_restore_read_device_formats callback\n");
    return NULL;
}
// card_info
#define GO(A)                                                      \
static uintptr_t my_card_info_fct_##A = 0;                         \
static void my_card_info_##A(void* a, void* b, int c, void* d)     \
{                                                                  \
    RunFunctionFmt(my_card_info_fct_##A, "ppip", a, b, c, d);  \
}
SUPER()
#undef GO
static void* find_card_info_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_card_info_fct_##A == (uintptr_t)fct) return my_card_info_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_card_info_fct_##A == 0) {my_card_info_fct_##A = (uintptr_t)fct; return my_card_info_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio card_info callback\n");
    return NULL;
}
// source_output_info
#define GO(A)                                                               \
static uintptr_t my_source_output_info_fct_##A = 0;                         \
static void my_source_output_info_##A(void* a, void* b, int c, void* d)     \
{                                                                           \
    RunFunctionFmt(my_source_output_info_fct_##A, "ppip", a, b, c, d);      \
}
SUPER()
#undef GO
static void* find_source_output_info_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_source_output_info_fct_##A == (uintptr_t)fct) return my_source_output_info_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_source_output_info_fct_##A == 0) {my_source_output_info_fct_##A = (uintptr_t)fct; return my_source_output_info_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio source_output_info callback\n");
    return NULL;
}
// device_restore_subscribe
#define GO(A)                                                                       \
static uintptr_t my_device_restore_subscribe_fct_##A = 0;                           \
static void my_device_restore_subscribe_##A(void* a, int b, uint32_t c, void* d)    \
{                                                                                   \
    RunFunctionFmt(my_device_restore_subscribe_fct_##A, "piup", a, b, c, d);        \
}
SUPER()
#undef GO
static void* find_device_restore_subscribe_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_device_restore_subscribe_fct_##A == (uintptr_t)fct) return my_device_restore_subscribe_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_device_restore_subscribe_fct_##A == 0) {my_device_restore_subscribe_fct_##A = (uintptr_t)fct; return my_device_restore_subscribe_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio device_restore_subscribe callback\n");
    return NULL;
}
// mainloop_once
#define GO(A)                                                               \
static uintptr_t my_mainloop_once_fct_##A = 0;                              \
static void my_mainloop_once_##A(void* api, void* b)                        \
{                                                                           \
    UpdateautobridgeMainloopAPI(thread_get_emu(), my_lib->w.bridge, api);   \
    RunFunctionFmt(my_mainloop_once_fct_##A, "pp", api, b);                 \
}
SUPER()
#undef GO
static void* find_mainloop_once_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_mainloop_once_fct_##A == (uintptr_t)fct) return my_mainloop_once_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_mainloop_once_fct_##A == 0) {my_mainloop_once_fct_##A = (uintptr_t)fct; return my_mainloop_once_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio mainloop_once callback\n");
    return NULL;
}
// io_new
#define GO(A)                                                                                                                                           \
static uintptr_t my_io_new_fct_##A = 0;                                                                                                                 \
static void* my_io_new_##A(void* api, int fd, int events, void* cb, void* data)                                                                         \
{                                                                                                                                                       \
    return (void*)RunFunctionFmt(my_io_new_fct_##A, "piipp", api, fd, events, AddCheckBridge(my_lib->w.bridge, vFppiip, cb, 0, "my_io_new_cb"),data);   \
}
SUPER()
#undef GO
static void* find_io_new_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_io_new_fct_##A == (uintptr_t)fct) return my_io_new_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_io_new_fct_##A == 0) {my_io_new_fct_##A = (uintptr_t)fct; return my_io_new_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio io_new callback\n");
    return NULL;
}
// io_enable
#define GO(A)                                                   \
static uintptr_t my_io_enable_fct_##A = 0;                      \
static void my_io_enable_##A(void* api, int events)             \
{                                                               \
    RunFunctionFmt(my_io_enable_fct_##A, "pi", api, events);    \
}
SUPER()
#undef GO
static void* find_io_enable_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_io_enable_fct_##A == (uintptr_t)fct) return my_io_enable_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_io_enable_fct_##A == 0) {my_io_enable_fct_##A = (uintptr_t)fct; return my_io_enable_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio io_enable callback\n");
    return NULL;
}
// io_free
#define GO(A)                                   \
static uintptr_t my_io_free_fct_##A = 0;        \
static void my_io_free_##A(void* e)             \
{                                               \
    RunFunctionFmt(my_io_free_fct_##A, "p", e); \
}
SUPER()
#undef GO
static void* find_io_free_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_io_free_fct_##A == (uintptr_t)fct) return my_io_free_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_io_free_fct_##A == 0) {my_io_free_fct_##A = (uintptr_t)fct; return my_io_free_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio io_free callback\n");
    return NULL;
}
// io_set_destroy
#define GO(A)                                                                                                                   \
static uintptr_t my_io_set_destroy_fct_##A = 0;                                                                                 \
static void my_io_set_destroy_##A(void* e, void* cb)                                                                            \
{                                                                                                                               \
    RunFunctionFmt(my_io_set_destroy_fct_##A, "pp", e, AddCheckBridge(my_lib->w.bridge, vFppp, cb, 0, "my_io_set_destroy_cb")); \
}
SUPER()
#undef GO
static void* find_io_set_destroy_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_io_set_destroy_fct_##A == (uintptr_t)fct) return my_io_set_destroy_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_io_set_destroy_fct_##A == 0) {my_io_set_destroy_fct_##A = (uintptr_t)fct; return my_io_set_destroy_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio io_set_destroy callback\n");
    return NULL;
}
// time_new
#define GO(A)                                                                                                                                   \
static uintptr_t my_time_new_fct_##A = 0;                                                                                                       \
static void* my_time_new_##A(void* api, void* t, void* cb, void* data)                                                                          \
{                                                                                                                                               \
    return (void*)RunFunctionFmt(my_time_new_fct_##A, "pppp", api, t, AddCheckBridge(my_lib->w.bridge, vFpppp, cb, 0, "my_time_new_cb"), data); \
}
SUPER()
#undef GO
static void* find_time_new_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_time_new_fct_##A == (uintptr_t)fct) return my_time_new_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_time_new_fct_##A == 0) {my_time_new_fct_##A = (uintptr_t)fct; return my_time_new_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio time_new callback\n");
    return NULL;
}
// time_restart
#define GO(A)                                               \
static uintptr_t my_time_restart_fct_##A = 0;               \
static void my_time_restart_##A(void* e, void* t)           \
{                                                           \
    RunFunctionFmt(my_time_restart_fct_##A, "pp", e, t);    \
}
SUPER()
#undef GO
static void* find_time_restart_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_time_restart_fct_##A == (uintptr_t)fct) return my_time_restart_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_time_restart_fct_##A == 0) {my_time_restart_fct_##A = (uintptr_t)fct; return my_time_restart_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio time_restart callback\n");
    return NULL;
}
// time_free
#define GO(A)                                       \
static uintptr_t my_time_free_fct_##A = 0;          \
static void my_time_free_##A(void* e)               \
{                                                   \
    RunFunctionFmt(my_time_free_fct_##A, "p", e);   \
}
SUPER()
#undef GO
static void* find_time_free_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_time_free_fct_##A == (uintptr_t)fct) return my_time_free_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_time_free_fct_##A == 0) {my_time_free_fct_##A = (uintptr_t)fct; return my_time_free_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio time_free callback\n");
    return NULL;
}
// pa_time_event_destroy_cb
#define GO(A)                                                               \
static uintptr_t my_pa_time_event_destroy_cb_fct_##A = 0;                   \
static void my_pa_time_event_destroy_cb_##A(void* a, void* e, void* u)      \
{                                                                           \
    RunFunctionFmt(my_pa_time_event_destroy_cb_fct_##A, "ppp", a, e, u);    \
}
SUPER()
#undef GO
static void* find_pa_time_event_destroy_cb_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_pa_time_event_destroy_cb_fct_##A == (uintptr_t)fct) return my_pa_time_event_destroy_cb_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_pa_time_event_destroy_cb_fct_##A == 0) {my_pa_time_event_destroy_cb_fct_##A = (uintptr_t)fct; return my_pa_time_event_destroy_cb_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio pa_time_event_destroy_cb callback\n");
    return NULL;
}
// time_set_destroy
#define GO(A)                                                                                                                       \
static uintptr_t my_time_set_destroy_fct_##A = 0;                                                                                   \
static void my_time_set_destroy_##A(void* e, void* cb)                                                                              \
{                                                                                                                                   \
    RunFunctionFmt(my_time_set_destroy_fct_##A, "pp", e, AddCheckBridge(my_lib->w.bridge, vFppp, cb, 0, "my_time_set_destroy_cb")); \
}
SUPER()
#undef GO
static void* find_time_set_destroy_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_time_set_destroy_fct_##A == (uintptr_t)fct) return my_time_set_destroy_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_time_set_destroy_fct_##A == 0) {my_time_set_destroy_fct_##A = (uintptr_t)fct; return my_time_set_destroy_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio time_set_destroy callback\n");
    return NULL;
}
#define GO(A)                                                   \
static vFpp_t my_time_set_destroy_native_fct_##A = NULL;        \
static void my_time_set_destroy_native_##A(void* e, void* cb)   \
{                                                               \
    cb = find_pa_time_event_destroy_cb_Fct(cb);                 \
    my_time_set_destroy_native_fct_##A (e, cb);                 \
}
SUPER()
#undef GO
static void* find_time_set_destroy_native_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my_time_set_destroy_native_fct_##A == fct) return my_time_set_destroy_native_##A;
    SUPER()
    #undef GO
    #define GO(A) if(!my_time_set_destroy_native_fct_##A) {my_time_set_destroy_native_fct_##A = fct; return my_time_set_destroy_native_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio time_set_destroy native wrapper\n");
    return NULL;
}
// defer_new
#define GO(A)                                                                                                                                   \
static uintptr_t my_defer_new_fct_##A = 0;                                                                                                      \
static void* my_defer_new_##A(void* api, void* cb, void* data)                                                                                  \
{                                                                                                                                               \
    return (void*)RunFunctionFmt(my_defer_new_fct_##A, "ppp", api, AddCheckBridge(my_lib->w.bridge, vFpppp, cb, 0, "my_defer_new_cb"), data);   \
}
SUPER()
#undef GO
static void* find_defer_new_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_defer_new_fct_##A == (uintptr_t)fct) return my_defer_new_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_defer_new_fct_##A == 0) {my_defer_new_fct_##A = (uintptr_t)fct; return my_defer_new_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio defer_new callback\n");
    return NULL;
}
// defer_enable
#define GO(A)                                               \
static uintptr_t my_defer_enable_fct_##A = 0;               \
static void my_defer_enable_##A(void* e, int b)             \
{                                                           \
    RunFunctionFmt(my_defer_enable_fct_##A, "pi", e, b);    \
}
SUPER()
#undef GO
static void* find_defer_enable_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_defer_enable_fct_##A == (uintptr_t)fct) return my_defer_enable_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_defer_enable_fct_##A == 0) {my_defer_enable_fct_##A = (uintptr_t)fct; return my_defer_enable_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio defer_enable callback\n");
    return NULL;
}
// defer_free
#define GO(A)                                       \
static uintptr_t my_defer_free_fct_##A = 0;         \
static void my_defer_free_##A(void* e)              \
{                                                   \
    RunFunctionFmt(my_defer_free_fct_##A, "p", e);  \
}
SUPER()
#undef GO
static void* find_defer_free_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_defer_free_fct_##A == (uintptr_t)fct) return my_defer_free_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_defer_free_fct_##A == 0) {my_defer_free_fct_##A = (uintptr_t)fct; return my_defer_free_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio defer_free callback\n");
    return NULL;
}
// defer_set_destroy
#define GO(A)                                                                                                                           \
static uintptr_t my_defer_set_destroy_fct_##A = 0;                                                                                      \
static void my_defer_set_destroy_##A(void* e, void* cb)                                                                                 \
{                                                                                                                                       \
    RunFunctionFmt(my_defer_set_destroy_fct_##A, "pp", e, AddCheckBridge(my_lib->w.bridge, vFppp, cb, 0, "my_defer_set_destroy_cb"));   \
}
SUPER()
#undef GO
static void* find_defer_set_destroy_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_defer_set_destroy_fct_##A == (uintptr_t)fct) return my_defer_set_destroy_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_defer_set_destroy_fct_##A == 0) {my_defer_set_destroy_fct_##A = (uintptr_t)fct; return my_defer_set_destroy_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio defer_set_destroy callback\n");
    return NULL;
}
// quit
#define GO(A)                                           \
static uintptr_t my_quit_fct_##A = 0;                   \
static void my_quit_##A(void* e, int retval)            \
{                                                       \
    RunFunctionFmt(my_quit_fct_##A, "pi", e, retval);   \
}
SUPER()
#undef GO
static void* find_quit_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_quit_fct_##A == (uintptr_t)fct) return my_quit_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_quit_fct_##A == 0) {my_quit_fct_##A = (uintptr_t)fct; return my_quit_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio quit callback\n");
    return NULL;
}

#undef SUPER

static void UpdateautobridgeMainloopAPI(x64emu_t* emu, bridge_t* bridge, my_pa_mainloop_api_t* api)
{
    if(!api) {
        return;
    }
    #define GO(A, W) if(api->A!=my_mainloop_api.A) {api->A=find_##A##_Fct(api->A); AddAutomaticBridge(bridge, W, api->A, 0, "pulse_" #A);}
    #define GO2(A, W) if(api->A!=my_mainloop_api.A) {api->A=find_##A##_Fct(api->A); AddAutomaticBridgeAlt(bridge, W, api->A, find_##A##_native_Fct(api->A), 0, "pulse_" #A);}
    GO(io_new, pFpiipp);
    GO(io_enable, vFpi);
    GO(io_free, vFp);
    GO(io_set_destroy, vFpp);
    GO(time_new, pFpppp);
    GO(time_restart, vFpp);
    GO(time_free, vFp);
    GO2(time_set_destroy, vFpp);
    GO(defer_new, pFppp);
    GO(defer_enable, vFpi);
    GO(defer_free, vFp);
    GO(defer_set_destroy, vFpp);
    GO(quit, vFpi);
    #undef GO
    #undef GO2
    memcpy(&my_mainloop_api, api, sizeof(my_mainloop_api));
    return;
}

static void autobridgeMainloopAPI(x64emu_t* emu, bridge_t* bridge, my_pa_mainloop_api_t* api)
{
    if(!api) {
        return;
    }
    #define GO(A, W) if(api->A) AddAutomaticBridge(bridge, W, api->A, 0, "pulse_" #A)
    #define GO2(A, W) if(api->A) AddAutomaticBridgeAlt(bridge, W, api->A, find_##A##_native_Fct(api->A), 0, "pulse_" #A)
    GO(io_new, pFpiipp);
    GO(io_enable, vFpi);
    GO(io_free, vFp);
    GO(io_set_destroy, vFpp);
    GO(time_new, pFpppp);
    GO(time_restart, vFpp);
    GO(time_free, vFp);
    GO2(time_set_destroy, vFpp);
    GO(defer_new, pFppp);
    GO(defer_enable, vFpi);
    GO(defer_free, vFp);
    GO(defer_set_destroy, vFpp);
    GO(quit, vFpi);
    #undef GO
    #undef GO2
    memcpy(&my_mainloop_api, api, sizeof(my_mainloop_api));
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
    my->pa_mainloop_free(mainloop);
}
EXPORT void* my_pa_mainloop_get_api(x64emu_t* emu, void* mainloop)
{
    my_pa_mainloop_api_t* api = my->pa_mainloop_get_api(mainloop);
    autobridgeMainloopAPI(emu, my_lib->w.bridge, api);
    return api;
}

EXPORT void my_pa_threaded_mainloop_free(x64emu_t* emu, void* mainloop)
{
    my->pa_threaded_mainloop_free(mainloop);
}
EXPORT void* my_pa_threaded_mainloop_get_api(x64emu_t* emu, void* mainloop)
{
    my_pa_mainloop_api_t* api = my->pa_threaded_mainloop_get_api(mainloop);
    autobridgeMainloopAPI(emu, my_lib->w.bridge, api);
    return api;
}

// Context functions
EXPORT void* my_pa_context_new(x64emu_t* emu, my_pa_mainloop_api_t* mainloop, void* name)
{
    UpdateautobridgeMainloopAPI(emu, my_lib->w.bridge, mainloop);
    return my->pa_context_new(mainloop, name);
}

EXPORT void* my_pa_context_new_with_proplist(x64emu_t* emu, my_pa_mainloop_api_t* mainloop, void* name, void* proplist)
{
    UpdateautobridgeMainloopAPI(emu, my_lib->w.bridge, mainloop);
    return my->pa_context_new_with_proplist(mainloop, name, proplist);
}

EXPORT int my_pa_signal_init(x64emu_t* emu, my_pa_mainloop_api_t* mainloop)
{
    UpdateautobridgeMainloopAPI(emu, my_lib->w.bridge, mainloop);
    return my->pa_signal_init(mainloop);
}

EXPORT void* my_pa_signal_new(x64emu_t* emu, int sig, void* cb, void* data)
{
    return my->pa_signal_new(sig, find_signal_Fct(cb), data);
}

EXPORT void my_pa_signal_set_destroy(x64emu_t* emu, void* e, void* cb)
{
    return my->pa_signal_set_destroy(e, find_signal_destroy_Fct(cb));
}

typedef struct my_pa_spawn_api_s {
    void* prefork;
    void* postfork;
    void* atfork;
} my_pa_spawn_api_t;

EXPORT int my_pa_context_connect(x64emu_t* emu, void* context, void* server, int flags, my_pa_spawn_api_t* api)
{
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
    my->pa_context_set_state_callback(context, find_state_context_Fct(cb), data);
}

EXPORT void my_pa_context_set_default_sink(x64emu_t* emu, void* context, void* name, void* cb, void* data)
{
    my->pa_context_set_default_sink(context, name, find_success_context_Fct(cb), data);
}

EXPORT void my_pa_context_set_default_source(x64emu_t* emu, void* context, void* name, void* cb, void* data)
{
    my->pa_context_set_default_source(context, name, find_success_context_Fct(cb), data);
}

EXPORT void* my_pa_context_move_sink_input_by_index(x64emu_t* emu, void* context, uint32_t idx, uint32_t sink_idx, void* cb, void* data)
{
    return my->pa_context_move_sink_input_by_index(context, idx, sink_idx, find_success_context_Fct(cb), data);
}

EXPORT void* my_pa_context_get_module_info_list(x64emu_t* emu, void* context, void* cb, void* data)
{
    return my->pa_context_get_module_info_list(context, find_module_info_Fct(cb), data);
}

EXPORT void* my_pa_context_get_server_info(x64emu_t* emu, void* context, void* cb, void* data)
{
    return my->pa_context_get_server_info(context, find_server_info_Fct(cb), data);
}

EXPORT void* my_pa_context_get_client_info_list(x64emu_t* emu, void* context, void* cb, void* data)
{
    return my->pa_context_get_client_info_list(context, find_client_info_Fct(cb), data);
}

EXPORT void* my_pa_context_get_sink_input_info(x64emu_t* emu, void* context, uint32_t idx, void* cb, void* data)
{
    return my->pa_context_get_sink_input_info(context, idx, find_module_info_Fct(cb), data);
}
EXPORT void* my_pa_context_get_sink_input_info_list(x64emu_t* emu, void* context, void* cb, void* data)
{
    return my->pa_context_get_sink_input_info_list(context, find_module_info_Fct(cb), data);
}

EXPORT void* my_pa_context_get_sink_info_list(x64emu_t* emu, void* context, void* cb, void* data)
{
    return my->pa_context_get_sink_info_list(context, find_module_info_Fct(cb), data);
}

EXPORT void* my_pa_context_get_sink_info_by_name(x64emu_t* emu, void* context, void* name, void* cb, void* data)
{
    return my->pa_context_get_sink_info_by_name(context, name, find_module_info_Fct(cb), data);
}

EXPORT void* my_pa_context_get_source_info_list(x64emu_t* emu, void* context, void* cb, void* data)
{
    return my->pa_context_get_source_info_list(context, find_module_info_Fct(cb), data);
}

EXPORT void my_pa_context_set_sink_input_mute(x64emu_t* emu, void* context, uint32_t idx, int mute, void* cb, void* data)
{
    my->pa_context_set_sink_input_mute(context, idx, mute, find_success_context_Fct(cb), data);
}

EXPORT void my_pa_context_set_sink_input_volume(x64emu_t* emu, void* context, uint32_t idx, void* volume, void* cb, void* data)
{
    my->pa_context_set_sink_input_volume(context, idx, volume, find_success_context_Fct(cb), data);
}

EXPORT void* my_pa_context_get_sink_info_by_index(x64emu_t* emu, void* context, uint32_t idx, void* cb, void* data)
{
    return my->pa_context_get_sink_info_by_index(context, idx, find_module_info_Fct(cb), data);
}

EXPORT void* my_pa_context_get_source_info_by_index(x64emu_t* emu, void* context, uint32_t idx, void* cb, void* data)
{
    return my->pa_context_get_source_info_by_index(context, idx, find_module_info_Fct(cb), data);
}

EXPORT void my_pa_context_set_source_volume_by_index(x64emu_t* emu, void* context, uint32_t idx, void* volume, void* cb, void* data)
{
    my->pa_context_set_source_volume_by_index(context, idx, volume, find_success_context_Fct(cb), data);
}

EXPORT void my_pa_context_set_source_mute_by_index(x64emu_t* emu, void* context, uint32_t idx, int mute, void* cb, void* data)
{
    my->pa_context_set_source_mute_by_index(context, idx, mute, find_success_context_Fct(cb), data);
}

EXPORT void my_pa_context_set_sink_volume_by_index(x64emu_t* emu, void* context, uint32_t idx, void* volume, void* cb, void* data)
{
    my->pa_context_set_sink_volume_by_index(context, idx, volume, find_success_context_Fct(cb), data);
}

EXPORT void* my_pa_context_unload_module(x64emu_t* emu, void* context, uint32_t idx, void* cb, void* data)
{
    return my->pa_context_unload_module(context, idx, find_success_context_Fct(cb), data);
}

EXPORT void* my_pa_context_load_module(x64emu_t* emu, void* context, void* name, void* arg, void* cb, void* data)
{
    return my->pa_context_load_module(context, name, arg, find_context_index_Fct(cb), data);
}

EXPORT void* my_pa_context_subscribe(x64emu_t* emu, void* context, uint32_t m, void* cb, void* data)
{
    return my->pa_context_subscribe(context, m, find_success_context_Fct(cb), data);
}

EXPORT void my_pa_context_set_subscribe_callback(x64emu_t* emu, void* context, void* cb, void* data)
{
    return my->pa_context_set_subscribe_callback(context, find_subscribe_context_Fct(cb), data);
}

EXPORT void* my_pa_context_drain(x64emu_t* emu, void* context, void* cb, void* data)
{
    return my->pa_context_drain(context, find_notify_context_Fct(cb), data);
}

EXPORT void* my_pa_context_exit_daemon(x64emu_t* emu, void* context, void* cb, void* data)
{
    return my->pa_context_exit_daemon(context, find_success_context_Fct(cb), data);
}

EXPORT void* my_pa_context_proplist_remove(x64emu_t* emu, void* context, void* keys, void* cb, void* data)
{
    return my->pa_context_proplist_remove(context, keys, find_success_context_Fct(cb), data);
}

EXPORT void* my_pa_context_proplist_update(x64emu_t* emu, void* context, int mode, void* p, void* cb, void* data)
{
    return my->pa_context_proplist_update(context, mode, p, find_success_context_Fct(cb), data);
}

EXPORT void my_pa_context_set_event_callback(x64emu_t* emu, void* context, void* cb, void* data)
{
    my->pa_context_set_event_callback(context, find_event_context_Fct(cb), data);
}

EXPORT void* my_pa_context_set_name(x64emu_t* emu, void* context, void* name, void* cb, void* data)
{
    return my->pa_context_set_name(context, name, find_success_context_Fct(cb), data);
}

EXPORT void* my_pa_context_set_source_volume_by_name(x64emu_t* emu, void* context, void* name,void* volume, void* cb, void* data)
{
    return my->pa_context_set_source_volume_by_name(context, name, volume, find_success_context_Fct(cb), data);
}

EXPORT void* my_pa_context_get_source_info_by_name(x64emu_t* emu, void* context, void* name, void* cb, void* data)
{
    return my->pa_context_get_source_info_by_name(context, name, find_module_info_Fct(cb), data);
}

// Stream functions

EXPORT void* my_pa_stream_drain(x64emu_t* emu, void* stream, void* cb, void* data)
{
    return my->pa_stream_drain(stream, find_stream_success_Fct(cb), data);
}

EXPORT void* my_pa_stream_flush(x64emu_t* emu, void* stream, void* cb, void* data)
{
    return my->pa_stream_flush(stream, find_stream_success_Fct(cb), data);
}

EXPORT void my_pa_stream_set_latency_update_callback(x64emu_t* emu, void* stream, void* cb, void* data)
{
    my->pa_stream_set_latency_update_callback(stream, find_stream_notify_Fct(cb), data);
}

EXPORT void my_pa_stream_set_read_callback(x64emu_t* emu, void* stream, void* cb, void* data)
{
    my->pa_stream_set_read_callback(stream, find_stream_request_Fct(cb), data);
}

EXPORT int my_pa_stream_write(x64emu_t* emu, void* stream, void* d, size_t nbytes, void* cb, int64_t offset, int seek)
{
    if(!my_lib)
        return 0;
    if(!my)
        return 0;
    return my->pa_stream_write(stream, d, nbytes, findFreeFct(cb), offset, seek);
}

EXPORT void* my_pa_stream_update_timing_info(x64emu_t* emu, void* stream, void* cb, void* data)
{
    return my->pa_stream_update_timing_info(stream, find_stream_success_Fct(cb), data);
}

EXPORT void* my_pa_stream_prebuf(x64emu_t* emu, void* stream, void* cb, void* data)
{
    return my->pa_stream_prebuf(stream, find_stream_success_Fct(cb), data);
}

EXPORT void* my_pa_stream_proplist_remove(x64emu_t* emu, void* stream, void* keys, void* cb, void* data)
{
    return my->pa_stream_proplist_remove(stream, keys, find_stream_success_Fct(cb), data);
}

EXPORT void* my_pa_stream_proplist_update(x64emu_t* emu, void* stream, int32_t mode, void* p, void* cb, void* data)
{
    return my->pa_stream_proplist_update(stream, mode, p, find_stream_success_Fct(cb), data);
}

EXPORT void* my_pa_stream_set_buffer_attr(x64emu_t* emu, void* stream, void* attr, void* cb, void* data)
{
    return my->pa_stream_set_buffer_attr(stream, attr, find_stream_success_Fct(cb), data);
}

EXPORT void my_pa_stream_set_buffer_attr_callback(x64emu_t* emu, void* stream, void* cb, void* data)
{
    my->pa_stream_set_buffer_attr_callback(stream, find_stream_notify_Fct(cb), data);
}

EXPORT void my_pa_stream_set_event_callback(x64emu_t* emu, void* stream, void* cb, void* data)
{
    my->pa_stream_set_event_callback(stream, find_stream_event_Fct(cb), data);
}

EXPORT void my_pa_stream_set_moved_callback(x64emu_t* emu, void* stream, void* cb, void* data)
{
    my->pa_stream_set_moved_callback(stream, find_stream_notify_Fct(cb), data);
}

EXPORT void* my_pa_stream_set_name(x64emu_t* emu, void* stream, void* name, void* cb, void* data)
{
    return my->pa_stream_set_name(stream, name, find_stream_success_Fct(cb), data);
}

EXPORT void my_pa_stream_set_overflow_callback(x64emu_t* emu, void* stream, void* cb, void* data)
{
    my->pa_stream_set_overflow_callback(stream, find_stream_notify_Fct(cb), data);
}

EXPORT void my_pa_stream_set_started_callback(x64emu_t* emu, void* stream, void* cb, void* data)
{
    my->pa_stream_set_started_callback(stream, find_stream_notify_Fct(cb), data);
}

EXPORT void my_pa_stream_set_state_callback(x64emu_t* emu, void* stream, void* cb, void* data)
{
    my->pa_stream_set_state_callback(stream, find_stream_state_Fct(cb), data);
}

EXPORT void my_pa_stream_set_suspended_callback(x64emu_t* emu, void* stream, void* cb, void* data)
{
    my->pa_stream_set_suspended_callback(stream, find_stream_notify_Fct(cb), data);
}

EXPORT void my_pa_stream_set_underflow_callback(x64emu_t* emu, void* stream, void* cb, void* data)
{
    my->pa_stream_set_underflow_callback(stream, find_stream_notify_Fct(cb), data);
}

EXPORT void my_pa_stream_set_write_callback(x64emu_t* emu, void* stream, void* cb, void* data)
{
    my->pa_stream_set_write_callback(stream, find_stream_request_Fct(cb), data);
}

EXPORT void* my_pa_stream_trigger(x64emu_t* emu, void* stream, void* cb, void* data)
{
    return my->pa_stream_trigger(stream, find_stream_success_Fct(cb), data);
}

EXPORT void* my_pa_stream_update_sample_rate(x64emu_t* emu, void* stream, uint32_t rate, void* cb, void* data)
{
    return my->pa_stream_update_sample_rate(stream, rate, find_stream_success_Fct(cb), data);
}

EXPORT void* my_pa_stream_cork(x64emu_t* emu, void* stream, int32_t b, void* cb, void* data)
{
    return my->pa_stream_cork(stream, b, find_stream_success_Fct(cb), data);
}

EXPORT int my_pa_proplist_setf(x64emu_t* emu, void* p, void* key, void* fmt, uint64_t* b)
{
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

    my->pa_mainloop_set_poll_func(m, find_poll_Fct(f), data);
}

#if 0
EXPORT void my_pa_log_level_meta(x64emu_t* emu, int level, void* file, int line, void* func, void* format, void* b, va_list V)
{
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

EXPORT void* my_pa_ext_device_restore_save_formats(x64emu_t* emu, void* c, int t, uint32_t idx, uint8_t n, void* f, void* cb, void* data)
{
    return my->pa_ext_device_restore_save_formats(c, t, idx, n, f, find_success_context_Fct(cb), data);
}

EXPORT void* my_pa_ext_device_restore_read_formats(x64emu_t* emu, void* c, int t, uint32_t idx, void* cb, void* data)
{
    return my->pa_ext_device_restore_read_formats(c, t, idx, find_device_restore_read_device_formats_Fct(cb), data);
}

EXPORT void* my_pa_context_set_source_port_by_index(x64emu_t* emu, void* c, uint32_t idx, void* p, void* cb, void* data)
{
    return my->pa_context_set_source_port_by_index(c, idx, p, find_success_context_Fct(cb), data);
}

EXPORT void* my_pa_context_set_card_profile_by_index(x64emu_t* emu, void* c, uint32_t idx, void* p, void* cb, void* data)
{
    return my->pa_context_set_card_profile_by_index(c, idx, p, find_success_context_Fct(cb), data);
}

EXPORT void* my_pa_context_set_sink_port_by_index(x64emu_t* emu, void* c, uint32_t idx, void* p, void* cb, void* data)
{
    return my->pa_context_set_sink_port_by_index(c, idx, p, find_success_context_Fct(cb), data);
}

EXPORT void* my_pa_context_get_card_info_list(x64emu_t* emu, void* c, void* cb, void* data)
{
    return my->pa_context_get_card_info_list(c, find_card_info_Fct(cb), data);
}

EXPORT void* my_pa_context_get_source_output_info_list(x64emu_t* emu, void* c, void* cb, void* data)
{
    return my->pa_context_get_source_output_info_list(c, find_source_output_info_Fct(cb), data);
}

EXPORT void* my_pa_ext_device_restore_read_formats_all(x64emu_t* emu, void* c, void* cb, void* data)
{
    return my->pa_ext_device_restore_read_formats_all(c, find_device_restore_read_device_formats_Fct(cb), data);
}

EXPORT void* my_pa_ext_device_restore_set_subscribe_cb(x64emu_t* emu, void* c, void* cb, void* data)
{
    return my->pa_ext_device_restore_set_subscribe_cb(c, find_device_restore_subscribe_Fct(cb), data);
}

EXPORT void* my_pa_ext_device_restore_subscribe(x64emu_t* emu, void* c, int e, void* cb, void* data)
{
    return my->pa_ext_device_restore_subscribe(c, e, find_success_context_Fct(cb), data);
}

EXPORT void* my_pa_context_set_sink_mute_by_index(x64emu_t* emu, void* c, uint32_t idx, int m, void* cb, void* data)
{
    return my->pa_context_set_sink_mute_by_index(c, idx, m, find_success_context_Fct(cb), data);
}

EXPORT void* my_pa_context_set_sink_port_by_name(x64emu_t* emu, void* c, void* n, void* p, void* cb, void* data)
{
    return my->pa_context_set_sink_port_by_name(c, n, p, find_success_context_Fct(cb), data);
}

EXPORT void* my_pa_context_move_source_output_by_index(x64emu_t* emu, void* c, uint32_t idx, uint32_t s, void* cb, void* data)
{
    return my->pa_context_move_source_output_by_index(c, idx, s, find_success_context_Fct(cb), data);
}

EXPORT void* my_pa_context_get_card_info_by_index(x64emu_t* emu, void* c, uint32_t idx, void* cb, void* data)
{
    return my->pa_context_get_card_info_by_index(c, idx, find_card_info_Fct(cb), data);
}

EXPORT void* my_pa_context_get_client_info(x64emu_t* emu, void* c, uint32_t idx, void* cb, void* data)
{
    return my->pa_context_get_client_info(c, idx, find_client_info_Fct(cb), data);
}

EXPORT void* my_pa_context_get_source_output_info(x64emu_t* emu, void* c, uint32_t idx, void* cb, void* data)
{
    return my->pa_context_get_source_output_info(c, idx, find_source_output_info_Fct(cb), data);
}

EXPORT void* my_pa_context_rttime_new(x64emu_t* emu, void* c, uint64_t usec, void* cb, void* data)
{
    return my->pa_context_rttime_new(c, usec, findTimeEventFct(cb) ,data);
}

EXPORT void my_pa_mainloop_api_once(x64emu_t* emu, void* mainloop, void* cb, void* data)
{
    UpdateautobridgeMainloopAPI(emu, my_lib->w.bridge, mainloop);
    my->pa_mainloop_api_once(mainloop, find_mainloop_once_Fct(cb), data);
}

void my_autobridge_mainloop_api(x64emu_t* emu, void* api)
{
    UpdateautobridgeMainloopAPI(emu, my_lib->w.bridge, api);
}

#define PRE_INIT        \
    if(BOX64ENV(nopulse))   \
        return -1;

#include "wrappedlib_init.h"
