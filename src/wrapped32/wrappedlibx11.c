#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper32.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "callback.h"
#include "librarian.h"
#include "box32context.h"
#include "emu/x64emu_private.h"
#include "myalign32.h"
#include "elfloader.h"
#include "converter32.h"

#ifdef ANDROID
    static const char* libx11Name = "libX11.so";
#else
    static const char* libx11Name = "libX11.so.6";
#endif

#define LIBNAME libx11

#include "libtools/my_x11_conv.h"

typedef int (*XErrorHandler)(void *, void *);
void* my32_XSetErrorHandler(x64emu_t* t, XErrorHandler handler);
typedef int (*XIOErrorHandler)(void *);
void* my32_XSetIOErrorHandler(x64emu_t* t, XIOErrorHandler handler);
void* my32_XESetCloseDisplay(x64emu_t* emu, void* display, int32_t extension, void* handler);
typedef int (*WireToEventProc)(void*, void*, void*);
typedef int(*EventHandler) (void*,void*,void*);
int32_t my32_XIfEvent(x64emu_t* emu, void* d,void* ev, EventHandler h, void* arg);

void delShmInfo(my_XShmSegmentInfo_t* a);   // edfine in Xext, to remove a saved ShmInfo

typedef void (*vFp_t)(void*);
typedef int  (*iFp_t)(void*);
typedef uint32_t (*uFv_t)(void);
typedef int  (*iFpp_t)(void*, void*);
typedef int32_t (*iFpl_t)(void*, intptr_t);
typedef int  (*iFppp_t)(void*, void*, void*);
typedef uintptr_t (*LFpii_t)(void*, int32_t, int32_t);
typedef int32_t (*iFpiiL_t)(void*, int32_t, int32_t, uintptr_t);
typedef void* (*pFpiiuu_t)(void*, int32_t, int32_t, uint32_t, uint32_t);

#define ADDED_FUNCTIONS()       \
    GO(XInitThreads, uFv_t)     \
    GO(XUnlockDisplay, vFp_t)

#include "generated/wrappedlibx11types32.h"

#include "wrappercallback32.h"

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
GO(15)

// wire_to_event
#define GO(A)   \
static uintptr_t my32_wire_to_event_fct_##A = 0;                                                    \
static int my32_wire_to_event_##A(void* dpy, void* re, void* event)                                 \
{                                                                                                   \
    static my_XEvent_32_t re_s = {0};                                                               \
    int ret = (int)RunFunctionFmt(my32_wire_to_event_fct_##A, "ppp", getDisplay(dpy), &re_s, event);\
    unconvertXEvent(re, &re_s);                                                                     \
    return ret;                                                                                     \
}
SUPER()
#undef GO
#define GO(A)   \
static iFppp_t my32_rev_wire_to_event_fct_##A = NULL;                                               \
static int my32_rev_wire_to_event_##A(void* dpy, void* re, void* event)                             \
{                                                                                                   \
    static my_XEvent_t re_l = {0};                                                                  \
    int ret = my32_rev_wire_to_event_fct_##A (getDisplay(dpy), &re_l, event);                       \
    convertXEvent(re, &re_l);                                                                       \
}
SUPER()
#undef GO
static void* findwire_to_eventFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my32_wire_to_event_fct_##A == (uintptr_t)fct) return my32_wire_to_event_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_wire_to_event_fct_##A == 0) {my32_wire_to_event_fct_##A = (uintptr_t)fct; return my32_wire_to_event_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 wire_to_event callback\n");
    return NULL;
}
static void* reverse_wire_to_eventFct(library_t* lib, void* fct)
{
    //Callsed from x86 world -> native world
    if(!fct) return fct;
    // first check if it's a wrapped function, that could be easy
    #define GO(A) if(my32_wire_to_event_##A == fct) return (void*)my32_wire_to_event_fct_##A;
    SUPER()
    #undef GO
    if(FindElfAddress(my_context, (uintptr_t)fct))
        return fct;
    // it's a naitve one... so bridge it, but need transform XImage32 to XImage
    void* f = NULL;
    #define GO(A) if(!f && my32_rev_wire_to_event_fct_##A == fct) f = (void*)my32_rev_wire_to_event_##A;
    SUPER()
    #undef GO
    #define GO(A) if(!f && !my32_rev_wire_to_event_fct_##A) {my32_rev_wire_to_event_fct_##A = fct; f = my32_rev_wire_to_event_##A;}
    SUPER()
    #undef GO
    if(f)
        return (void*)AddCheckBridge(lib->w.bridge, iFppp_32, f, 0, "X11_wire_to_event");
    printf_log(LOG_NONE, "Warning, no more slot for reverse 32bits libX11 wire_to_event callback\n");
    return fct;
}

// event_to_wire
#define GO(A)   \
static uintptr_t my32_event_to_wire_fct_##A = 0;                                                    \
static int my32_event_to_wire_##A(void* dpy, void* re, void* event)                                 \
{                                                                                                   \
    my_XEvent_32_t re_s = {0};                                                                      \
    convertXEvent(&re_s, re);                                                                       \
    return (int)RunFunctionFmt(my32_event_to_wire_fct_##A, "ppp", getDisplay(dpy), &re_s, event);   \
}
SUPER()
#undef GO
#define GO(A)   \
static iFppp_t my32_rev_event_to_wire_fct_##A = NULL;                                               \
static int my32_rev_event_to_wire_##A(void* dpy, void* re, void* event)                             \
{                                                                                                   \
    static my_XEvent_t re_l = {0};                                                                  \
    unconvertXEvent(&re_l, re);                                                                     \
    return my32_rev_event_to_wire_fct_##A (getDisplay(dpy), &re_l, event);                          \
}
SUPER()
#undef GO
static void* findevent_to_wireFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my32_event_to_wire_fct_##A == (uintptr_t)fct) return my32_event_to_wire_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_event_to_wire_fct_##A == 0) {my32_event_to_wire_fct_##A = (uintptr_t)fct; return my32_event_to_wire_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 event_to_wire callback\n");
    return NULL;
}
static void* reverse_event_to_wireFct(library_t* lib, void* fct)
{
    //Callsed from x86 world -> native world
    if(!fct) return fct;
    // first check if it's a wrapped function, that could be easy
    #define GO(A) if(my32_event_to_wire_##A == fct) return (void*)my32_event_to_wire_fct_##A;
    SUPER()
    #undef GO
    if(FindElfAddress(my_context, (uintptr_t)fct))
        return fct;
    // it's a naitve one... so bridge it, but need transform XImage32 to XImage
    void* f = NULL;
    #define GO(A) if(!f && my32_rev_event_to_wire_fct_##A == fct) f = (void*)my32_rev_event_to_wire_##A;
    SUPER()
    #undef GO
    #define GO(A) if(!f && !my32_rev_event_to_wire_fct_##A) {my32_rev_event_to_wire_fct_##A = fct; f = my32_rev_event_to_wire_##A;}
    SUPER()
    #undef GO
    if(f)
        return (void*)AddCheckBridge(lib->w.bridge, iFppp_32, f, 0, "X11_event_to_wire");
    printf_log(LOG_NONE, "Warning, no more slot for reverse 32bits libX11 event_to_wire callback\n");
    return fct;
}
// error_handler
#define GO(A)   \
static uintptr_t my32_error_handler_fct_##A = 0;                                            \
static int my32_error_handler_##A(void* dpy, void* error)                                   \
{                                                                                           \
    static my_XErrorEvent_32_t evt = {0};                                                   \
    convert_XErrorEvent_to_32(&evt, error);                                                 \
    printf_log(LOG_DEBUG, "Calling Xerrorhandler(%p, %p), err=%hhu/%hhu/%hhu\n",            \
        dpy, error, evt.error_code, evt.request_code, evt.minor_code);                      \
    return (int)RunFunctionFmt(my32_error_handler_fct_##A, "pp", FindDisplay(dpy), &evt);   \
}
SUPER()
#undef GO
#define GO(A)   \
static iFpp_t my32_rev_error_handler_fct_##A = NULL;                                        \
static int my32_rev_error_handler_##A(void* dpy, void* error)                               \
{                                                                                           \
    my_XErrorEvent_t evt = {0};                                                             \
    convert_XErrorEvent_to_64(&evt, error);                                                 \
    return my32_rev_error_handler_fct_##A (getDisplay(dpy), &evt);                          \
}
SUPER()
#undef GO
static void* finderror_handlerFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my32_error_handler_fct_##A == (uintptr_t)fct) return my32_error_handler_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_error_handler_fct_##A == 0) {my32_error_handler_fct_##A = (uintptr_t)fct; return my32_error_handler_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 error_handler callback\n");
    return NULL;
}
static void* reverse_error_handler_Fct(library_t* lib, void* fct)
{
    //Callsed from x86 world -> native world
    if(!fct) return fct;
    // first check if it's a wrapped function, that could be easy
    #define GO(A) if(my32_error_handler_##A == fct) return (void*)my32_error_handler_fct_##A;
    SUPER()
    #undef GO
    if(FindElfAddress(my_context, (uintptr_t)fct))
        return fct;
    // it's a naitve one... so bridge it, but need transform XImage32 to XImage
    void* f = NULL;
    #define GO(A) if(!f && my32_rev_error_handler_fct_##A == fct) f = (void*)my32_rev_error_handler_##A;
    SUPER()
    #undef GO
    #define GO(A) if(!f && !my32_rev_error_handler_fct_##A) {my32_rev_error_handler_fct_##A = fct; f = my32_rev_error_handler_##A;}
    SUPER()
    #undef GO
    if(f)
        return (void*)AddCheckBridge(lib->w.bridge, iFpp_32, f, 0, "X11_error_handler");
    printf_log(LOG_NONE, "Warning, no more slot for reverse 32bits libX11 error_handler callback\n");
    return fct;
}

// ioerror_handler
#define GO(A)   \
static uintptr_t my32_ioerror_handler_fct_##A = 0;                                  \
static int my32_ioerror_handler_##A(void* dpy)                                      \
{                                                                                   \
    return (int)RunFunctionFmt(my32_ioerror_handler_fct_##A, "p", FindDisplay(dpy));\
}
SUPER()
#undef GO
#define GO(A)   \
static iFp_t my32_rev_ioerror_handler_fct_##A = NULL;                               \
static int my32_rev_ioerror_handler_##A(void* dpy)                                  \
{                                                                                   \
    return my32_rev_ioerror_handler_fct_##A (getDisplay(dpy));                      \
}
SUPER()
#undef GO
static void* findioerror_handlerFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my32_ioerror_handler_fct_##A == (uintptr_t)fct) return my32_ioerror_handler_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_ioerror_handler_fct_##A == 0) {my32_ioerror_handler_fct_##A = (uintptr_t)fct; return my32_ioerror_handler_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 ioerror_handler callback\n");
    return NULL;
}
static void* reverse_ioerror_handler_Fct(library_t* lib, void* fct)
{
    //Callsed from x86 world -> native world
    if(!fct) return fct;
    // first check if it's a wrapped function, that could be easy
    #define GO(A) if(my32_ioerror_handler_##A == fct) return (void*)my32_ioerror_handler_fct_##A;
    SUPER()
    #undef GO
    if(FindElfAddress(my_context, (uintptr_t)fct))
        return fct;
    // it's a naitve one... so bridge it, but need transform XImage32 to XImage
    void* f = NULL;
    #define GO(A) if(!f && my32_rev_ioerror_handler_fct_##A == fct) f = (void*)my32_rev_ioerror_handler_##A;
    SUPER()
    #undef GO
    #define GO(A) if(!f && !my32_rev_ioerror_handler_fct_##A) {my32_rev_ioerror_handler_fct_##A = fct; f = my32_rev_ioerror_handler_##A;}
    SUPER()
    #undef GO
    if(f)
        return (void*)AddCheckBridge(lib->w.bridge, iFp_32, f, 0, "X11_ioerror_handler");
    printf_log(LOG_NONE, "Warning, no more slot for reverse 32bits libX11 ioerror_handler callback\n");
    return fct;
}

#if 0
// exterror_handler
#define GO(A)   \
static uintptr_t my32_exterror_handler_fct_##A = 0;                      \
static int my32_exterror_handler_##A(void* dpy, void* err, void* codes, int* ret_code)   \
{                                                                   \
    return (int)RunFunctionFmt(my32_exterror_handler_fct_##A, "pppp", dpy, err, codes, ret_code);\
}
SUPER()
#undef GO
static void* findexterror_handlerFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my32_exterror_handler_fct_##A == (uintptr_t)fct) return my32_exterror_handler_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_exterror_handler_fct_##A == 0) {my32_exterror_handler_fct_##A = (uintptr_t)fct; return my32_exterror_handler_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 exterror_handler callback\n");
    return NULL;
}
static void* reverse_exterror_handlerFct(library_t* lib, void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(lib->w.bridge, fct))
        return (void*)CheckBridged(lib->w.bridge, fct);
    #define GO(A) if(my32_exterror_handler_##A == fct) return (void*)my32_exterror_handler_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(lib->w.bridge, iFpppp, fct, 0, NULL);
}
#endif
// close_display
#define GO(A)   \
static uintptr_t my32_close_display_fct_##A = 0;                                            \
static int my32_close_display_##A(void* dpy, void* codes)                                   \
{                                                                                           \
    return (int)RunFunctionFmt(my32_close_display_fct_##A, "pp", getDisplay(dpy), codes);   \
}
SUPER()
#undef GO
static void* findclose_displayFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my32_close_display_fct_##A == (uintptr_t)fct) return my32_close_display_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_close_display_fct_##A == 0) {my32_close_display_fct_##A = (uintptr_t)fct; return my32_close_display_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 close_display callback\n");
    return NULL;
}
static void* reverse_close_displayFct(library_t* lib, void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(lib->w.bridge, fct))
        return (void*)CheckBridged(lib->w.bridge, fct);
    #define GO(A) if(my32_close_display_##A == fct) return (void*)my32_close_display_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(lib->w.bridge, iFpp_32, fct, 0, NULL);
}

// register_im
#define GO(A)   \
static uintptr_t my32_register_im_fct_##A = 0;                              \
static void my32_register_im_##A(void* dpy, void* u, void* d)               \
{                                                                           \
    RunFunctionFmt(my32_register_im_fct_##A, "ppp", getDisplay(dpy), u, d); \
}
SUPER()
#undef GO
static void* findregister_imFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my32_register_im_fct_##A == (uintptr_t)fct) return my32_register_im_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_register_im_fct_##A == 0) {my32_register_im_fct_##A = (uintptr_t)fct; return my32_register_im_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 register_im callback\n");
    return NULL;
}
static void* reverse_register_imFct(library_t* lib, void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(lib->w.bridge, fct))
        return (void*)CheckBridged(lib->w.bridge, fct);
    #define GO(A) if(my32_register_im_##A == fct) return (void*)my32_register_im_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(lib->w.bridge, iFppp_32, fct, 0, NULL);
}
// XConnectionWatchProc
#define GO(A)   \
static uintptr_t my32_XConnectionWatchProc_fct_##A = 0;                                         \
static void my32_XConnectionWatchProc_##A(void* dpy, void* data, int op, void* d)               \
{                                                                                               \
    RunFunctionFmt(my32_XConnectionWatchProc_fct_##A, "ppip", FindDisplay(dpy), data, op, d);   \
}
SUPER()
#undef GO
static void* findXConnectionWatchProcFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my32_XConnectionWatchProc_fct_##A == (uintptr_t)fct) return my32_XConnectionWatchProc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_XConnectionWatchProc_fct_##A == 0) {my32_XConnectionWatchProc_fct_##A = (uintptr_t)fct; return my32_XConnectionWatchProc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 XConnectionWatchProc callback\n");
    return NULL;
}
// xifevent
#define GO(A)   \
static uintptr_t my32_xifevent_fct_##A = 0;                                             \
static int my32_xifevent_##A(void* dpy, my_XEvent_t* event, void* d)                    \
{                                                                                       \
    static my_XEvent_32_t evt[16] = {0};                                                \
    static int idx = 0;                                                                 \
    idx=(idx+1)&15;                                                                     \
    convertXEvent(evt+idx, event);                                                      \
    return RunFunctionFmt(my32_xifevent_fct_##A, "ppp", getDisplay(dpy), evt+idx, d);   \
}
SUPER()
#undef GO
static void* findxifeventFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my32_xifevent_fct_##A == (uintptr_t)fct) return my32_xifevent_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_xifevent_fct_##A == 0) {my32_xifevent_fct_##A = (uintptr_t)fct; return my32_xifevent_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 xifevent callback\n");
    return NULL;
}
// XInternalAsyncHandler
#define GO(A)   \
static uintptr_t my32_XInternalAsyncHandler_fct_##A = 0;                                                        \
static int my32_XInternalAsyncHandler_##A(void* dpy, void* rep, void* buf, int len, void* data)                 \
{                                                                                                               \
    return RunFunctionFmt(my32_XInternalAsyncHandler_fct_##A, "pppip", FindDisplay(dpy), rep, buf, len, data);  \
}
SUPER()
#undef GO
static void* findXInternalAsyncHandlerFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my32_XInternalAsyncHandler_fct_##A == (uintptr_t)fct) return my32_XInternalAsyncHandler_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_XInternalAsyncHandler_fct_##A == 0) {my32_XInternalAsyncHandler_fct_##A = (uintptr_t)fct; return my32_XInternalAsyncHandler_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 XInternalAsyncHandler callback\n");
    return NULL;
}
// XSynchronizeProc
#define GO(A)   \
static uintptr_t my32_XSynchronizeProc_fct_##A = 0;                       \
static int my32_XSynchronizeProc_##A()                                    \
{                                                                       \
    return (int)RunFunctionFmt(my32_XSynchronizeProc_fct_##A, "");\
}
SUPER()
#undef GO
static void* findXSynchronizeProcFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my32_XSynchronizeProc_fct_##A == (uintptr_t)fct) return my32_XSynchronizeProc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_XSynchronizeProc_fct_##A == 0) {my32_XSynchronizeProc_fct_##A = (uintptr_t)fct; return my32_XSynchronizeProc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 XSynchronizeProc callback\n");
    return NULL;
}
static void* reverse_XSynchronizeProcFct(library_t* lib, void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(lib->w.bridge, fct))
        return (void*)CheckBridged(lib->w.bridge, fct);
    #define GO(A) if(my32_XSynchronizeProc_##A == fct) return (void*)my32_XSynchronizeProc_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(lib->w.bridge, iFppp_32, fct, 0, NULL);
}
#if 0
// XLockDisplay
#define GO(A)   \
static uintptr_t my32_XLockDisplay_fct_##A = 0;                   \
static void my32_XLockDisplay_##A(void* dpy)                      \
{                                                               \
    RunFunctionFmt(my32_XLockDisplay_fct_##A, "p", dpy);   \
}
SUPER()
#undef GO
static void* findXLockDisplayFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my32_XLockDisplay_fct_##A == (uintptr_t)fct) return my32_XLockDisplay_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_XLockDisplay_fct_##A == 0) {my32_XLockDisplay_fct_##A = (uintptr_t)fct; return my32_XLockDisplay_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 XLockDisplay callback\n");
    return NULL;
}
// XUnlockDisplay
#define GO(A)   \
static uintptr_t my32_XUnlockDisplay_fct_##A = 0;                 \
static void my32_XUnlockDisplay_##A(void* dpy)                    \
{                                                               \
    RunFunctionFmt(my32_XUnlockDisplay_fct_##A, "p", dpy); \
}
SUPER()
#undef GO
static void* findXUnlockDisplayFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my32_XUnlockDisplay_fct_##A == (uintptr_t)fct) return my32_XUnlockDisplay_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_XUnlockDisplay_fct_##A == 0) {my32_XUnlockDisplay_fct_##A = (uintptr_t)fct; return my32_XUnlockDisplay_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 XUnlockDisplay callback\n");
    return NULL;
}
#endif
// async_handler
#define GO(A)   \
static uintptr_t my32_async_handler_fct_##A = 0;                                                    \
static int my32_async_handler_##A(void* a, void* b, void* c, int d, void* e)                        \
{                                                                                                   \
    return (int)RunFunctionFmt(my32_async_handler_fct_##A, "pppip", FindDisplay(a), b, c, d, e);    \
}
SUPER()
#undef GO
static void* find_async_handler_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my32_async_handler_fct_##A == (uintptr_t)fct) return my32_async_handler_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_async_handler_fct_##A == 0) {my32_async_handler_fct_##A = (uintptr_t)fct; return my32_async_handler_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 async_handler callback\n");
    return NULL;
}
// XImage function wrappers
// create_image
#define GO(A)   \
static uintptr_t my32_create_image_fct_##A = 0;                                                                                     \
static void* my32_create_image_##A(void* a, void* b, uint32_t c, int d, int e, void* f, uint32_t g, uint32_t h, int i, int j)       \
{                                                                                                                                   \
    void* ret = (void*)RunFunctionFmt(my32_create_image_fct_##A, "ppuiipuuii", FindDisplay(a), convert_Visual_to_32(a, b), c, d, e, f, g, h, i, j);          \
    UnwrapXImage(ret, ret);                                                                                                         \
    return ret;                                                                                                                     \
}                                                                                                                                   \
static pFXpuiipuuii_t my32_rev_create_image_fct_##A = NULL;                                                                         \
static void* my32_rev_create_image_##A(void* a, void* b, uint32_t c, int d, int e, void* f, uint32_t g, uint32_t h, int i, int j)   \
{                                                                                                                                   \
    void* ret = my32_rev_create_image_fct_##A (getDisplay(a), convert_Visual_to_64(a, b), c, d, e, f, g, h, i, j);                  \
    WrapXImage(ret, ret);                                                                                                           \
    return ret;                                                                                                                     \
}
SUPER()
#undef GO
static void* find_create_image_Fct(void* fct)
{
    if(!fct) return fct;
    void* n_fct = GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my32_rev_create_image_##A == n_fct) return (void*)my32_rev_create_image_fct_##A;
    SUPER()
    #undef GO
    if(n_fct)  return n_fct;
    #define GO(A) if(my32_create_image_fct_##A == (uintptr_t)fct) return my32_create_image_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_create_image_fct_##A == 0) {my32_create_image_fct_##A = (uintptr_t)fct; return my32_create_image_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 create_image callback\n");
    return NULL;
}
static void* reverse_create_image_Fct(library_t* lib, void* fct)
{
    //Callsed from x86 world -> native world
    if(!fct) return fct;
    // first check if it's a wrapped function, that could be easy
    #define GO(A) if(my32_create_image_##A == fct) return (void*)my32_create_image_fct_##A;
    SUPER()
    #undef GO
    if(FindElfAddress(my_context, (uintptr_t)fct))
        return fct;
    // it's a naitve one... so bridge it, but need transform XImage32 to XImage
    void* f = NULL;
    #define GO(A) if(!f && my32_rev_create_image_fct_##A == fct) f = (void*)my32_rev_create_image_##A;
    SUPER()
    #undef GO
    #define GO(A) if(!f && !my32_rev_create_image_fct_##A) {my32_rev_create_image_fct_##A = fct; f = my32_rev_create_image_##A;}
    SUPER()
    #undef GO
    if(f)
        return (void*)AddCheckBridge(lib->w.bridge, pFXpuiipuuii_32, f, 0, "Ximage_create_image");
    printf_log(LOG_NONE, "Warning, no more slot for reverse 32bits libX11 create_image callback\n");
    return fct;
}
// destroy_image
#define GO(A)   \
static uintptr_t my32_destroy_image_fct_##A = 0;                        \
static int my32_destroy_image_##A(void* a)                              \
{                                                                       \
    void* obdata = ((XImage*)a)->obdata;                                \
    inplace_XImage_shrink(a);                                           \
    int ret = (int)RunFunctionFmt(my32_destroy_image_fct_##A, "p", a);  \
    to_hash_d((uintptr_t)obdata);                                       \
    if(obdata) delShmInfo(obdata);                                      \
    return ret;                                                         \
}                                                                       \
static iFp_t my32_rev_destroy_image_fct_##A = NULL;                     \
static int my32_rev_destroy_image_##A(void* a)                          \
{                                                                       \
    inplace_XImage_enlarge(a);                                          \
    to_hash_d((uintptr_t)((XImage*)a)->obdata);                         \
    void* obdata = ((XImage*)a)->obdata;                                \
    int ret = my32_rev_destroy_image_fct_##A (a);                       \
    if(obdata) delShmInfo(obdata);                                      \
    return ret;                                                         \
}
SUPER()
#undef GO
static void* find_destroy_image_Fct(void* fct)
{
    if(!fct) return fct;
    void* n_fct = GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my32_rev_destroy_image_##A == n_fct) return (void*)my32_rev_destroy_image_fct_##A;
    SUPER()
    #undef GO
    if(n_fct)  return n_fct;
    #define GO(A) if(my32_destroy_image_fct_##A == (uintptr_t)fct) return my32_destroy_image_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_destroy_image_fct_##A == 0) {my32_destroy_image_fct_##A = (uintptr_t)fct; return my32_destroy_image_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for 32bits libX11 destroy_image callback\n");
    return NULL;
}
static void* reverse_destroy_image_Fct(library_t* lib, void* fct)
{
    //Callsed from x86 world -> native world
    if(!fct) return fct;
    // first check if it's a wrapped function, that could be easy
    #define GO(A) if(my32_destroy_image_##A == fct) return (void*)my32_destroy_image_fct_##A;
    SUPER()
    #undef GO
    if(FindElfAddress(my_context, (uintptr_t)fct))
        return fct;
    // it's a naitve one... so bridge it, but need transform XImage32 to XImage
    void* f = NULL;
    #define GO(A) if(!f && my32_rev_destroy_image_fct_##A == fct) f = (void*)my32_rev_destroy_image_##A;
    SUPER()
    #undef GO
    #define GO(A) if(!f && !my32_rev_destroy_image_fct_##A) {my32_rev_destroy_image_fct_##A = fct; f = my32_rev_destroy_image_##A;}
    SUPER()
    #undef GO
    if(f)
        return (void*)AddCheckBridge(lib->w.bridge, iFp_32, f, 0, "Ximage_destroy_image");
    printf_log(LOG_NONE, "Warning, no more slot for reverse 32bits libX11 destroy_image callback\n");
    return fct;
}
// get_pixel
#define GO(A)   \
static uintptr_t my32_get_pixel_fct_##A = 0;                                    \
static unsigned long my32_get_pixel_##A(void* a, int b, int c)                  \
{                                                                               \
    inplace_XImage_shrink(a);                                                   \
    uint32_t ret = RunFunctionFmt(my32_get_pixel_fct_##A, "pii", a, b, c);      \
    inplace_XImage_enlarge(a);                                                  \
    return from_ulong(ret);                                                     \
}                                                                               \
static LFpii_t my32_rev_get_pixel_fct_##A = NULL;                               \
static unsigned long my32_rev_get_pixel_##A(void* a, int b, int c)              \
{                                                                               \
    inplace_XImage_enlarge(a);                                                  \
    unsigned long ret = my32_rev_get_pixel_fct_##A (a, b, c);                   \
    inplace_XImage_shrink(a);                                                   \
    return ret;                                                                 \
}
SUPER()
#undef GO
static void* find_get_pixel_Fct(void* fct)
{
    if(!fct) return fct;
    void* n_fct = GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my32_rev_get_pixel_##A == n_fct) return (void*)my32_rev_get_pixel_fct_##A;
    SUPER()
    #undef GO
    if(n_fct)  return n_fct;
    #define GO(A) if(my32_get_pixel_fct_##A == (uintptr_t)fct) return my32_get_pixel_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_get_pixel_fct_##A == 0) {my32_get_pixel_fct_##A = (uintptr_t)fct; return my32_get_pixel_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 get_pixel callback\n");
    return NULL;
}
static void* reverse_get_pixel_Fct(library_t* lib, void* fct)
{
    //Callsed from x86 world -> native world
    if(!fct) return fct;
    // first check if it's a wrapped function, that could be easy
    #define GO(A) if(my32_get_pixel_##A == fct) return (void*)my32_get_pixel_fct_##A;
    SUPER()
    #undef GO
    if(FindElfAddress(my_context, (uintptr_t)fct))
        return fct;
    // it's a naitve one... so bridge it, but need transform XImage32 to XImage
    void* f = NULL;
    #define GO(A) if(!f && my32_rev_get_pixel_fct_##A == fct) f = (void*)my32_rev_get_pixel_##A;
    SUPER()
    #undef GO
    #define GO(A) if(!f && !my32_rev_get_pixel_fct_##A) {my32_rev_get_pixel_fct_##A = fct; f = my32_rev_get_pixel_##A;}
    SUPER()
    #undef GO
    if(f)
        return (void*)AddCheckBridge(lib->w.bridge, LFpii_32, f, 0, "Ximage_get_pixel");
    printf_log(LOG_NONE, "Warning, no more slot for reverse 32bits libX11 get_pixel callback\n");
    return fct;
}
// put_pixel
#define GO(A)   \
static uintptr_t my32_put_pixel_fct_##A = 0;                                    \
static int my32_put_pixel_##A(void* a, int b, int c,unsigned long d)            \
{                                                                               \
    inplace_XImage_shrink(a);                                                   \
    int ret =  (int)RunFunctionFmt(my32_put_pixel_fct_##A, "piiL", a, b, c, d); \
    inplace_XImage_enlarge(a);                                                  \
    return ret;                                                                 \
}                                                                               \
static iFpiiL_t my32_rev_put_pixel_fct_##A = NULL;                              \
static int my32_rev_put_pixel_##A(void* a, int b, int c, ulong_t d)             \
{                                                                               \
    inplace_XImage_enlarge(a);                                                  \
    int ret = my32_rev_put_pixel_fct_##A (a, b, c, from_ulong(d));              \
    inplace_XImage_shrink(a);                                                   \
    return ret;                                                                 \
}
SUPER()
#undef GO
static void* find_put_pixel_Fct(void* fct)
{
    if(!fct) return fct;
    void* n_fct = GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my32_rev_put_pixel_##A == n_fct) return (void*)my32_rev_put_pixel_fct_##A;
    SUPER()
    #undef GO
    if(n_fct)  return n_fct;
    #define GO(A) if(my32_put_pixel_fct_##A == (uintptr_t)fct) return my32_put_pixel_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_put_pixel_fct_##A == 0) {my32_put_pixel_fct_##A = (uintptr_t)fct; return my32_put_pixel_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 put_pixel callback\n");
    return NULL;
}
static void* reverse_put_pixel_Fct(library_t* lib, void* fct)
{
    //Callsed from x86 world -> native world
    if(!fct) return fct;
    // first check if it's a wrapped function, that could be easy
    #define GO(A) if(my32_put_pixel_##A == fct) return (void*)my32_put_pixel_fct_##A;
    SUPER()
    #undef GO
    if(FindElfAddress(my_context, (uintptr_t)fct))
        return fct;
    // it's a naitve one... so bridge it, but need transform XImage32 to XImage
    void* f = NULL;
    #define GO(A) if(!f && my32_rev_put_pixel_fct_##A == fct) f = (void*)my32_rev_put_pixel_##A;
    SUPER()
    #undef GO
    #define GO(A) if(!f && !my32_rev_put_pixel_fct_##A) {my32_rev_put_pixel_fct_##A = fct; f = my32_rev_put_pixel_##A;}
    SUPER()
    #undef GO
    if(f)
        return (void*)AddCheckBridge(lib->w.bridge, iFpiiL_32, f, 0, "Ximage_put_pixel");
    printf_log(LOG_NONE, "Warning, no more slot for reverse 32bits libX11 put_pixel callback\n");
    return fct;
}
// sub_image
#define GO(A)   \
static uintptr_t my32_sub_image_fct_##A = 0;                                        \
static void* my32_sub_image_##A(void* a, int b, int c, uint32_t d, uint32_t e)      \
{                                                                                   \
    inplace_XImage_shrink(a);                                                       \
    void* ret = (void*)RunFunctionFmt(my32_sub_image_fct_##A, "piiuu", a, b, c, d, e);\
    if(ret!=a) UnwrapXImage(ret, ret);                                              \
    inplace_XImage_enlarge(a);                                                      \
    return ret;                                                                     \
}                                                                                   \
static pFpiiuu_t my32_rev_sub_image_fct_##A = NULL;                                 \
static void* my32_rev_sub_image_##A(void* a, int b, int c, uint32_t d, uint32_t e)  \
{                                                                                   \
    inplace_XImage_enlarge(a);                                                      \
    void* ret = my32_rev_sub_image_fct_##A (a, b, c, d, e);                         \
    if(ret!=a)                                                                      \
        WrapXImage(ret, ret);                                                       \
    inplace_XImage_shrink(a);                                                       \
    return ret;                                                                     \
}
SUPER()
#undef GO
static void* find_sub_image_Fct(void* fct)
{
    if(!fct) return fct;
    void* n_fct = GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my32_rev_sub_image_##A == n_fct) return (void*)my32_rev_sub_image_fct_##A;
    SUPER()
    #undef GO
    if(n_fct)  return n_fct;
    #define GO(A) if(my32_sub_image_fct_##A == (uintptr_t)fct) return my32_sub_image_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_sub_image_fct_##A == 0) {my32_sub_image_fct_##A = (uintptr_t)fct; return my32_sub_image_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 sub_image callback\n");
    return NULL;
}
static void* reverse_sub_image_Fct(library_t* lib, void* fct)
{
    //Callsed from x86 world -> native world
    if(!fct) return fct;
    // first check if it's a wrapped function, that could be easy
    #define GO(A) if(my32_sub_image_##A == fct) return (void*)my32_sub_image_fct_##A;
    SUPER()
    #undef GO
    if(FindElfAddress(my_context, (uintptr_t)fct))
        return fct;
    // it's a naitve one... so bridge it, but need transform XImage32 to XImage
    void* f = NULL;
    #define GO(A) if(!f && my32_rev_sub_image_fct_##A == fct) f = (void*)my32_rev_sub_image_##A;
    SUPER()
    #undef GO
    #define GO(A) if(!f && !my32_rev_sub_image_fct_##A) {my32_rev_sub_image_fct_##A = fct; f = my32_rev_sub_image_##A;}
    SUPER()
    #undef GO
    if(f)
        return (void*)AddCheckBridge(lib->w.bridge, pFpiiuu_32, f, 0, "ximage_sub_image");
    printf_log(LOG_NONE, "Warning, no more slot for reverse 32bits libX11 sub_image callback\n");
    return fct;
}
// add_pixel
#define GO(A)   \
static uintptr_t my32_add_pixel_fct_##A = 0;                        \
static int my32_add_pixel_##A(void* a, long b)                      \
{                                                                   \
    inplace_XImage_shrink(a);                                       \
    int ret =  (int)RunFunctionFmt(my32_add_pixel_fct_##A, "pl", a, b); \
    inplace_XImage_enlarge(a);                                      \
    return ret;                                                     \
}                                                                   \
static iFpl_t my32_rev_add_pixel_fct_##A = NULL;                    \
static int my32_rev_add_pixel_##A(void* a, long_t b)                \
{                                                                   \
    inplace_XImage_enlarge(a);                                      \
    int ret = my32_rev_add_pixel_fct_##A (a, from_long(b));         \
    inplace_XImage_shrink(a);                                       \
    return ret;                                                     \
}
SUPER()
#undef GO
static void* find_add_pixel_Fct(void* fct)
{
    if(!fct) return fct;
    void* n_fct = GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my32_rev_add_pixel_##A == n_fct) return (void*)my32_rev_add_pixel_fct_##A;
    SUPER()
    #undef GO
    if(n_fct)  return n_fct;
    #define GO(A) if(my32_add_pixel_fct_##A == (uintptr_t)fct) return my32_add_pixel_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_add_pixel_fct_##A == 0) {my32_add_pixel_fct_##A = (uintptr_t)fct; return my32_add_pixel_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 add_pixel callback\n");
    return NULL;
}
static void* reverse_add_pixel_Fct(library_t* lib, void* fct)
{
    //Callsed from x86 world -> native world
    if(!fct) return fct;
    // first check if it's a wrapped function, that could be easy
    #define GO(A) if(my32_add_pixel_##A == fct) return (void*)my32_add_pixel_fct_##A;
    SUPER()
    #undef GO
    if(FindElfAddress(my_context, (uintptr_t)fct))
        return fct;
    // it's a naitve one... so bridge it, but need transform XImage32 to XImage
    void* f = NULL;
    #define GO(A) if(!f && my32_rev_add_pixel_fct_##A == fct) f = (void*)my32_rev_add_pixel_##A;
    SUPER()
    #undef GO
    #define GO(A) if(!f && !my32_rev_add_pixel_fct_##A) {my32_rev_add_pixel_fct_##A = fct; f = my32_rev_add_pixel_##A;}
    SUPER()
    #undef GO
    if(f)
        return (void*)AddCheckBridge(lib->w.bridge, iFpl_32, f, 0, "ximage_add_pixel");
    printf_log(LOG_NONE, "Warning, no more slot for reverse 32bits libX11 add_pixel callback\n");
    return fct;
}
// end of XImage functions callbacks

#undef SUPER

void* my32_XCreateImage(x64emu_t* emu, void* disp, void* vis, uint32_t depth, int32_t fmt, int32_t off
                    , void* data, uint32_t w, uint32_t h, int32_t pad, int32_t bpl);

int32_t my32_XInitImage(x64emu_t* emu, void* img);

void* my32_XGetImage(x64emu_t* emu, void* disp, XID drawable, int32_t x, int32_t y
                    , uint32_t w, uint32_t h, uint32_t plane, int32_t fmt);

int32_t my32_XPutImage(x64emu_t* emu, void* disp, XID drawable, void* gc, void* image
                    , int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y
                    , uint32_t w, uint32_t h);

void* my32_XGetSubImage(x64emu_t* emu, void* disp, XID drawable
                    , int32_t x, int32_t y
                    , uint32_t w, uint32_t h, XID plane, int32_t fmt
                    , void* image, int32_t dst_x, int32_t dst_y);

void my32_XDestroyImage(x64emu_t* emu, void* image);

typedef void (*XIMProc)(void*, void*, void*);
typedef int (*XICProc)(void*, void*, void*);
typedef struct {
    void* client_data;
    XIMProc callback;
} XIMCallback;

typedef struct {
    void* client_data;
    XICProc callback;
} XICCallback;

#define XNGeometryCallback "geometryCallback"
#define XNDestroyCallback "destroyCallback"
#define XNPreeditStartCallback "preeditStartCallback"
#define XNPreeditDoneCallback "preeditDoneCallback"
#define XNPreeditDrawCallback "preeditDrawCallback"
#define XNPreeditCaretCallback "preeditCaretCallback"
#define XNPreeditStateNotifyCallback "preeditStateNotifyCallback"
#define XNStatusStartCallback "statusStartCallback"
#define XNStatusDoneCallback "statusDoneCallback"
#define XNStatusDrawCallback "statusDrawCallback"
#define XNR6PreeditCallback "r6PreeditCallback"
#define XNStringConversionCallback "stringConversionCallback"

// utility functions
#include "super100.h"

// XNGeometryCallback
#define GO(A)   \
static uintptr_t my32_XNGeometryCallback_fct_##A = 0;                 \
static void my32_XNGeometryCallback_##A(void* a, void* b, void* c)    \
{                                                                   \
    RunFunctionFmt(my32_XNGeometryCallback_fct_##A, "ppp", a, b);     \
}
SUPER()
#undef GO
static void* findXNGeometryCallbackFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my32_XNGeometryCallback_fct_##A == (uintptr_t)fct) return my32_XNGeometryCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_XNGeometryCallback_fct_##A == 0) {my32_XNGeometryCallback_fct_##A = (uintptr_t)fct; return my32_XNGeometryCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 XNGeometryCallback callback\n");
    return NULL;
}
// XNDestroyCallback
#define GO(A)   \
static uintptr_t my32_XNDestroyCallback_fct_##A = 0;              \
static void my32_XNDestroyCallback_##A(void* a, void* b, void* c) \
{                                                               \
    RunFunctionFmt(my32_XNDestroyCallback_fct_##A, "ppp", a, b);  \
}
SUPER()
#undef GO
static void* findXNDestroyCallbackFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my32_XNDestroyCallback_fct_##A == (uintptr_t)fct) return my32_XNDestroyCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_XNDestroyCallback_fct_##A == 0) {my32_XNDestroyCallback_fct_##A = (uintptr_t)fct; return my32_XNDestroyCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 XNDestroyCallback callback\n");
    return NULL;
}
// XNPreeditStartCallback
#define GO(A)   \
static uintptr_t my32_XNPreeditStartCallback_fct_##A = 0;                 \
static void my32_XNPreeditStartCallback_##A(void* a, void* b, void* c)    \
{                                                                       \
    RunFunctionFmt(my32_XNPreeditStartCallback_fct_##A, "ppp", a, b);     \
}
SUPER()
#undef GO
static void* findXNPreeditStartCallbackFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my32_XNPreeditStartCallback_fct_##A == (uintptr_t)fct) return my32_XNPreeditStartCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_XNPreeditStartCallback_fct_##A == 0) {my32_XNPreeditStartCallback_fct_##A = (uintptr_t)fct; return my32_XNPreeditStartCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 XNPreeditStartCallback callback\n");
    return NULL;
}
// XNPreeditDoneCallback
#define GO(A)   \
static uintptr_t my32_XNPreeditDoneCallback_fct_##A = 0;              \
static void my32_XNPreeditDoneCallback_##A(void* a, void* b, void* c) \
{                                                                   \
    RunFunctionFmt(my32_XNPreeditDoneCallback_fct_##A, "ppp", a, b);  \
}
SUPER()
#undef GO
static void* findXNPreeditDoneCallbackFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my32_XNPreeditDoneCallback_fct_##A == (uintptr_t)fct) return my32_XNPreeditDoneCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_XNPreeditDoneCallback_fct_##A == 0) {my32_XNPreeditDoneCallback_fct_##A = (uintptr_t)fct; return my32_XNPreeditDoneCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 XNPreeditDoneCallback callback\n");
    return NULL;
}
// XNPreeditDrawCallback
#define GO(A)   \
static uintptr_t my32_XNPreeditDrawCallback_fct_##A = 0;              \
static void my32_XNPreeditDrawCallback_##A(void* a, void* b, void* c) \
{                                                                   \
    RunFunctionFmt(my32_XNPreeditDrawCallback_fct_##A, "ppp", a, b);  \
}
SUPER()
#undef GO
static void* findXNPreeditDrawCallbackFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my32_XNPreeditDrawCallback_fct_##A == (uintptr_t)fct) return my32_XNPreeditDrawCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_XNPreeditDrawCallback_fct_##A == 0) {my32_XNPreeditDrawCallback_fct_##A = (uintptr_t)fct; return my32_XNPreeditDrawCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 XNPreeditDrawCallback callback\n");
    return NULL;
}
// XNPreeditCaretCallback
#define GO(A)   \
static uintptr_t my32_XNPreeditCaretCallback_fct_##A = 0;                 \
static void my32_XNPreeditCaretCallback_##A(void* a, void* b, void* c)    \
{                                                                       \
    RunFunctionFmt(my32_XNPreeditCaretCallback_fct_##A, "ppp", a, b);     \
}
SUPER()
#undef GO
static void* findXNPreeditCaretCallbackFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my32_XNPreeditCaretCallback_fct_##A == (uintptr_t)fct) return my32_XNPreeditCaretCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_XNPreeditCaretCallback_fct_##A == 0) {my32_XNPreeditCaretCallback_fct_##A = (uintptr_t)fct; return my32_XNPreeditCaretCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 XNPreeditCaretCallback callback\n");
    return NULL;
}
// XNPreeditStateNotifyCallback
#define GO(A)   \
static uintptr_t my32_XNPreeditStateNotifyCallback_fct_##A = 0;               \
static void my32_XNPreeditStateNotifyCallback_##A(void* a, void* b, void* c)  \
{                                                                           \
    RunFunctionFmt(my32_XNPreeditStateNotifyCallback_fct_##A, "ppp", a, b);   \
}
SUPER()
#undef GO
static void* findXNPreeditStateNotifyCallbackFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my32_XNPreeditStateNotifyCallback_fct_##A == (uintptr_t)fct) return my32_XNPreeditStateNotifyCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_XNPreeditStateNotifyCallback_fct_##A == 0) {my32_XNPreeditStateNotifyCallback_fct_##A = (uintptr_t)fct; return my32_XNPreeditStateNotifyCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 XNPreeditStateNotifyCallback callback\n");
    return NULL;
}
// XNStatusStartCallback
#define GO(A)   \
static uintptr_t my32_XNStatusStartCallback_fct_##A = 0;                  \
static void my32_XNStatusStartCallback_##A(void* a, void* b, void* c)     \
{                                                                       \
    RunFunctionFmt(my32_XNStatusStartCallback_fct_##A, "ppp", a, b);      \
}
SUPER()
#undef GO
static void* findXNStatusStartCallbackFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my32_XNStatusStartCallback_fct_##A == (uintptr_t)fct) return my32_XNStatusStartCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_XNStatusStartCallback_fct_##A == 0) {my32_XNStatusStartCallback_fct_##A = (uintptr_t)fct; return my32_XNStatusStartCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 XNStatusStartCallback callback\n");
    return NULL;
}
// XNStatusDoneCallback
#define GO(A)   \
static uintptr_t my32_XNStatusDoneCallback_fct_##A = 0;               \
static void my32_XNStatusDoneCallback_##A(void* a, void* b, void* c)  \
{                                                                   \
    RunFunctionFmt(my32_XNStatusDoneCallback_fct_##A, "ppp", a, b);   \
}
SUPER()
#undef GO
static void* findXNStatusDoneCallbackFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my32_XNStatusDoneCallback_fct_##A == (uintptr_t)fct) return my32_XNStatusDoneCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_XNStatusDoneCallback_fct_##A == 0) {my32_XNStatusDoneCallback_fct_##A = (uintptr_t)fct; return my32_XNStatusDoneCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 XNStatusDoneCallback callback\n");
    return NULL;
}
// XNStatusDrawCallback
#define GO(A)   \
static uintptr_t my32_XNStatusDrawCallback_fct_##A = 0;               \
static void my32_XNStatusDrawCallback_##A(void* a, void* b, void* c)  \
{                                                                   \
    RunFunctionFmt(my32_XNStatusDrawCallback_fct_##A, "ppp", a, b);   \
}
SUPER()
#undef GO
static void* findXNStatusDrawCallbackFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my32_XNStatusDrawCallback_fct_##A == (uintptr_t)fct) return my32_XNStatusDrawCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_XNStatusDrawCallback_fct_##A == 0) {my32_XNStatusDrawCallback_fct_##A = (uintptr_t)fct; return my32_XNStatusDrawCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 XNStatusDrawCallback callback\n");
    return NULL;
}
// XNR6PreeditCallback
#define GO(A)   \
static uintptr_t my32_XNR6PreeditCallback_fct_##A = 0;                \
static void my32_XNR6PreeditCallback_##A(void* a, void* b, void* c)   \
{                                                                   \
    RunFunctionFmt(my32_XNR6PreeditCallback_fct_##A, "ppp", a, b);    \
}
SUPER()
#undef GO
static void* findXNR6PreeditCallbackFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my32_XNR6PreeditCallback_fct_##A == (uintptr_t)fct) return my32_XNR6PreeditCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_XNR6PreeditCallback_fct_##A == 0) {my32_XNR6PreeditCallback_fct_##A = (uintptr_t)fct; return my32_XNR6PreeditCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 XNR6PreeditCallback callback\n");
    return NULL;
}
// XNStringConversionCallback
#define GO(A)   \
static uintptr_t my32_XNStringConversionCallback_fct_##A = 0;                 \
static void my32_XNStringConversionCallback_##A(void* a, void* b, void* c)    \
{                                                                           \
    RunFunctionFmt(my32_XNStringConversionCallback_fct_##A, "ppp", a, b);     \
}
SUPER()
#undef GO
static void* findXNStringConversionCallbackFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my32_XNStringConversionCallback_fct_##A == (uintptr_t)fct) return my32_XNStringConversionCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_XNStringConversionCallback_fct_##A == 0) {my32_XNStringConversionCallback_fct_##A = (uintptr_t)fct; return my32_XNStringConversionCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 XNStringConversionCallback callback\n");
    return NULL;
}

#undef SUPER

#define SUPER()                     \
GO(XNGeometryCallback)              \
GO(XNDestroyCallback)               \
GO(XNPreeditStartCallback)          \
GO(XNPreeditDoneCallback)           \
GO(XNPreeditDrawCallback)           \
GO(XNPreeditCaretCallback)          \
GO(XNPreeditStateNotifyCallback)    \
GO(XNStatusStartCallback)           \
GO(XNStatusDoneCallback)            \
GO(XNStatusDrawCallback)            \
GO(XNR6PreeditCallback)             \
GO(XNStringConversionCallback)

#define VA_CALL(FUNC, FIRST_ARG, N, VAARGSZ, RESULT)       \
switch (VAARGSZ)                                                \
{                                                               \
case 2:                                                         \
    RESULT = FUNC(FIRST_ARG, from_ulong(va[0]), from_ulong(va[1]), NULL);       \
    break;                                                      \
case 4:                                                         \
    RESULT = FUNC(FIRST_ARG, from_ulong(va[0]), from_ulong(va[1]), from_ulong(va[2]), from_ulong(va[3]), NULL);     \
    break;                                                                          \
case 6:                                                                             \
    RESULT = FUNC(FIRST_ARG, from_ulong(va[0]), from_ulong(va[1]), from_ulong(va[2]), from_ulong(va[3]), from_ulong(va[4]), from_ulong(va[5]), NULL);   \
    break;                                                                                              \
case 8:                                                                                                 \
    RESULT = FUNC(FIRST_ARG, from_ulong(va[0]), from_ulong(va[1]), from_ulong(va[2]), from_ulong(va[3]), from_ulong(va[4]), from_ulong(va[5]), from_ulong(va[6]), from_ulong(va[7]), NULL); \
    break;                                                                                                                  \
case 10:                                                                                                                    \
    RESULT = FUNC(FIRST_ARG, from_ulong(va[0]), from_ulong(va[1]), from_ulong(va[2]), from_ulong(va[3]), from_ulong(va[4]), from_ulong(va[5]), from_ulong(va[6]), from_ulong(va[7]), from_ulong(va[8]), from_ulong(va[9]), NULL);   \
    break;                                                                                                                                          \
case 12:                                                                                                                                            \
    RESULT = FUNC(FIRST_ARG, from_ulong(va[0]), from_ulong(va[1]), from_ulong(va[2]), from_ulong(va[3]), from_ulong(va[4]), from_ulong(va[5]), from_ulong(va[6]), from_ulong(va[7]), from_ulong(va[8]), from_ulong(va[9]),  from_ulong(va[10]), from_ulong(va[11]), NULL);  \
    break;                                                                                                                                                                  \
case 14:                                                                                                                                                                    \
    RESULT = FUNC(FIRST_ARG, from_ulong(va[0]), from_ulong(va[1]), from_ulong(va[2]), from_ulong(va[3]), from_ulong(va[4]), from_ulong(va[5]), from_ulong(va[6]), from_ulong(va[7]), from_ulong(va[8]), from_ulong(va[9]),  from_ulong(va[10]), from_ulong(va[11]), from_ulong(va[12]), from_ulong(va[13]), NULL);  \
    break;                                                                                                                                                                                          \
case 16:                                                                                                                                                                                            \
    RESULT = FUNC(FIRST_ARG, from_ulong(va[0]), from_ulong(va[1]), from_ulong(va[2]), from_ulong(va[3]), from_ulong(va[4]), from_ulong(va[5]), from_ulong(va[6]), from_ulong(va[7]), from_ulong(va[8]), from_ulong(va[9]),  from_ulong(va[10]), from_ulong(va[11]), from_ulong(va[12]), from_ulong(va[13]), from_ulong(va[14]), from_ulong(va[15]), NULL);  \
    break;                                                                                                                                                                                                                  \
case 18:                                                                                                                                                                                                                    \
    RESULT = FUNC(FIRST_ARG, from_ulong(va[0]), from_ulong(va[1]), from_ulong(va[2]), from_ulong(va[3]), from_ulong(va[4]), from_ulong(va[5]), from_ulong(va[6]), from_ulong(va[7]), from_ulong(va[8]), from_ulong(va[9]),  from_ulong(va[10]), from_ulong(va[11]), from_ulong(va[12]), from_ulong(va[13]), from_ulong(va[14]), from_ulong(va[15]), from_ulong(va[16]), from_ulong(va[17]), NULL);  \
    break;                                                                                                                                                                                                                                          \
case 20:                                                                                                                                                                                                                                            \
    RESULT = FUNC(FIRST_ARG, from_ulong(va[0]), from_ulong(va[1]), from_ulong(va[2]), from_ulong(va[3]), from_ulong(va[4]), from_ulong(va[5]), from_ulong(va[6]), from_ulong(va[7]), from_ulong(va[8]), from_ulong(va[9]),  from_ulong(va[10]), from_ulong(va[11]), from_ulong(va[12]), from_ulong(va[13]), from_ulong(va[14]), from_ulong(va[15]), from_ulong(va[16]), from_ulong(va[17]), from_ulong(va[18]), from_ulong(va[19]), NULL);  \
    break;                                                                                                                                                                                                                                                                  \
default:                                                                                                                \
    printf_log(LOG_NONE, "warning: %s's vasize (%d) is too large, need create new call case!\n", __func__, VAARGSZ);    \
    break;                                                                                                              \
}

#define GO(A)                                                                       \
if (va[i] && strcmp((char*)from_ptrv(va[i]), A) == 0) {                             \
    XICCallback* origin = (XICCallback*)from_ptrv(va[i+1]);                         \
    va[i+1] = to_ptrv(find##A##Fct(origin));                                        \
}

EXPORT void* my32_XCreateIC(x64emu_t* emu, void* xim, ptr_t* va) {
    int n = 0;
    while (va[n]) n+=2;

    for (int i = 0; i < n; i += 2) {
        SUPER()
    }

    void* res = NULL;
    VA_CALL(my->XCreateIC, xim, 1, n, res);
    return res;
}

EXPORT void* my32_XVaCreateNestedList(x64emu_t* emu, int unused, ptr_t* va) {
    int n = 0;
    while (va[n]) n+=2 ;

    for (int i = 0; i < n; i += 2) {
        SUPER()
    }

    void* res = NULL;
    VA_CALL(my->XVaCreateNestedList, unused, 1, n, res);
    return res;
}

EXPORT void* my32_XSetICValues(x64emu_t* emu, void* xic, ptr_t* va) {
    int n = 0;
    while (va[n]) n+=2;

    for (int i = 0; i < n; i += 2) {
        SUPER()
    }

    void* res = NULL;
    VA_CALL(my->XSetICValues, xic, 1, n, res);
    return res;
}
#undef GO

EXPORT void* my32_XSetIMValues(x64emu_t* emu, void* xim, ptr_t* va) {
    int n = 0;
    while (va[n]) n+=2;

    #define GO(A)                                                       \
    if (va[i] && strcmp((char*)from_ptrv(va[i]), A) == 0) {             \
        XIMCallback* origin = (XIMCallback*)from_ptrv(va[i+1]);         \
        va[i+1] = to_ptrv(find##A##Fct(origin));                        \
    }
    for (int i = 0; i < n; i += 2) {
        SUPER()
    }
    #undef GO

    void* res = NULL;
    VA_CALL(my->XSetIMValues, xim, 1, n, res)
    return res;
}

#undef VA_CALL
#undef SUPER
EXPORT void* my32_XSetErrorHandler(x64emu_t* emu, XErrorHandler handler)
{
    void* ret = my->XSetErrorHandler(finderror_handlerFct(handler));
    return reverse_error_handler_Fct(my_lib, ret);
}

EXPORT void* my32_XSetIOErrorHandler(x64emu_t* emu, XIOErrorHandler handler)
{
    void* ret = my->XSetIOErrorHandler(findioerror_handlerFct(handler));
    return reverse_ioerror_handler_Fct(my_lib, ret);
}

#if 0
EXPORT void* my32_XESetError(x64emu_t* emu, void* display, int32_t extension, void* handler)
{
    void* ret = my->XESetError(display, extension, findexterror_handlerFct(handler));
    return reverse_exterror_handlerFct(my_lib, ret);
}
#endif
EXPORT void* my32_XESetCloseDisplay(x64emu_t* emu, void* display, int32_t extension, void* handler)
{
    void* ret = my->XESetCloseDisplay(display, extension, findclose_displayFct(handler));
    return reverse_close_displayFct(my_lib, ret);
}

EXPORT int32_t my32_XIfEvent(x64emu_t* emu, void* d,void* ev, EventHandler h, void* arg)
{
    my_XEvent_t event = {0};
    int32_t ret = my->XIfEvent(d, &event, findxifeventFct(h), arg);
    convertXEvent(ev, &event);
    return ret;
}

EXPORT int32_t my32_XCheckIfEvent(x64emu_t* emu, void* d,void* ev, EventHandler h, void* arg)
{
    my_XEvent_t event = {0};
    int32_t ret = my->XCheckIfEvent(d, &event, findxifeventFct(h), arg);
    convertXEvent(ev, &event);
    return ret;
}
EXPORT int32_t my32_XPeekIfEvent(x64emu_t* emu, void* d,void* ev, EventHandler h, void* arg)
{
    my_XEvent_t event = {0};
    int32_t ret = my->XPeekIfEvent(d, &event, findxifeventFct(h), arg);
    convertXEvent(ev, &event);
    return ret;
}

EXPORT int my32_XFilterEvent(x64emu_t* emu, my_XEvent_32_t* evt, XID window)
{
    my_XEvent_t event = {0};
    unconvertXEvent(&event, evt);
    return my->XFilterEvent(&event, window);
}

EXPORT int my32_XPutBackEvent(x64emu_t* emu, void* dpy, my_XEvent_32_t* evt)
{
    my_XEvent_t event = {0};
    unconvertXEvent(&event, evt);
    return my->XPutBackEvent(dpy, &event);
}

EXPORT int my32_XCheckMaskEvent(x64emu_t* emu, void* dpy, long mask, my_XEvent_32_t* evt)
{
    my_XEvent_t event = {0};
    int32_t ret = my->XCheckMaskEvent(dpy, mask, &event);
    convertXEvent(evt, &event);
    return ret;
}

void WrapXImage(void* d, void* s)
{
    XImage *src = s;
    XImage_32 *dst = d;

    // inplace "shrink"
    
    dst->width = src->width;
    dst->height = src->height;
    dst->xoffset = src->xoffset;
    dst->format = src->format;
    dst->data = to_ptrv(src->data);
    dst->byte_order = src->byte_order;
    dst->bitmap_unit = src->bitmap_unit;
    dst->bitmap_bit_order = src->bitmap_bit_order;
    dst->bitmap_pad = src->bitmap_pad;
    dst->depth = src->depth;
    dst->bytes_per_line = src->bytes_per_line;
    dst->bits_per_pixel = src->bits_per_pixel;
    dst->red_mask = to_ulong(src->red_mask);
    dst->green_mask = to_ulong(src->green_mask);
    dst->blue_mask = to_ulong(src->blue_mask);
    dst->obdata = to_hash((uintptr_t)src->obdata);

    #define GO(A, W) \
    dst->f.A = to_ptrv((W##_t)reverse_##A##_Fct(my_lib, src->f.A));

    GO(create_image, pFXpuiipuuii)
    GO(destroy_image, iFp)
    GO(get_pixel, LFpii)
    GO(put_pixel, iFpiiL)
    GO(sub_image, pFpiiuu)
    GO(add_pixel, iFpl)
    #undef GO
}

void UnwrapXImage(void* d, void* s)
{
    XImage_32* src = s;
    XImage *dst = d;

    // inplace "enlarge", so reverse order

    #define GO(A, W)                            \
    dst->f.A = (W##_t)find_##A##_Fct(from_ptrv(src->f.A));

    GO(add_pixel, iFpl)
    GO(sub_image, pFpiiuu)
    GO(put_pixel, iFpiiL)
    GO(get_pixel, LFpii)
    GO(destroy_image, iFp)
    GO(create_image, pFXpuiipuuii)
    #undef GO

    dst->obdata = (void*)from_hash(src->obdata);
    dst->blue_mask = from_ulong(src->blue_mask);
    dst->green_mask = from_ulong(src->green_mask);
    dst->red_mask = from_ulong(src->red_mask);
    dst->bits_per_pixel = src->bits_per_pixel;
    dst->bytes_per_line = src->bytes_per_line;
    dst->depth = src->depth;
    dst->bitmap_pad = src->bitmap_pad;
    dst->bitmap_bit_order = src->bitmap_bit_order;
    dst->bitmap_unit = src->bitmap_unit;
    dst->byte_order = src->byte_order;
    dst->data = from_ptrv(src->data);
    dst->format = src->format;
    dst->xoffset = src->xoffset;
    dst->height = src->height;
    dst->width = src->width;
}

EXPORT void* my32_XCreateImage(x64emu_t* emu, void* disp, void* vis, uint32_t depth, int32_t fmt, int32_t off
                    , void* data, uint32_t w, uint32_t h, int32_t pad, int32_t bpl)
{

    XImage *img = my->XCreateImage(disp, convert_Visual_to_64(disp, vis), depth, fmt, off, data, w, h, pad, bpl);
    if(!img)
        return img;
    // bridge all access functions...
    WrapXImage(img, img);
    return img;
}

EXPORT int32_t my32_XInitImage(x64emu_t* emu, void* img)
{
    XImage l_img = {0};
    UnwrapXImage(&l_img, img);
    int ret = my->XInitImage(&l_img);
    // bridge all access functions...
    WrapXImage(img, &l_img);
    return ret;
}

EXPORT void* my32_XGetImage(x64emu_t* emu, void* disp, XID drawable, int32_t x, int32_t y
                    , uint32_t w, uint32_t h, uint32_t plane, int32_t fmt)
{

    XImage *img = my->XGetImage(disp, drawable, x, y, w, h, plane, fmt);
    if(!img)
        return img;
    // bridge all access functions...
    inplace_XImage_shrink(img);
    return img;
}

EXPORT void my32__XInitImageFuncPtrs(x64emu_t* emu, XImage_32* img)
{
    XImage img_l = {0};
    img->f.add_pixel = img->f.create_image = img->f.destroy_image = img->f.get_pixel = img->f.put_pixel = img->f.sub_image = 0;
    UnwrapXImage(&img_l, img);
    my->_XInitImageFuncPtrs(&img_l);
    WrapXImage(img, &img_l);
}

EXPORT int32_t my32_XPutImage(x64emu_t* emu, void* disp, size_t drawable, void* gc, void* image
                    , int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y
                    , uint32_t w, uint32_t h)
{
    inplace_XImage_enlarge(image); // what if the image was created on x86 side and is smaller?
    int32_t r = my->XPutImage(disp, drawable, gc, image, src_x, src_y, dst_x, dst_y, w, h);
    // bridge all access functions...
    inplace_XImage_shrink(image);
    return r;
}

EXPORT void* my32_XGetSubImage(x64emu_t* emu, void* disp, XID drawable
                    , int32_t x, int32_t y
                    , uint32_t w, uint32_t h, XID plane, int32_t fmt
                    , void* image, int32_t dst_x, int32_t dst_y)
{

    inplace_XImage_enlarge(image);
    XImage *img = my->XGetSubImage(disp, drawable, x, y, w, h, plane, fmt, image, dst_x, dst_y);
    if(img && img!=image)
        inplace_XImage_shrink(img);

    inplace_XImage_shrink(image);
    return img;
}

EXPORT void my32_XDestroyImage(x64emu_t* emu, void* image)
{

    inplace_XImage_enlarge(image);
    to_hash_d((uintptr_t)((XImage*)image)->obdata);
    my->XDestroyImage(image);
}

typedef struct xintasync_s {
    struct xintasync_s *next;
    int (*handler)(
                    void*,
                    void*,
                    void*,
                    int,
                    void*
                    );
    void* data;
} xintasync_t;

EXPORT void my32__XDeqAsyncHandler(x64emu_t* emu, void* cb, void* data)
{
    my->_XDeqAsyncHandler(findXInternalAsyncHandlerFct(cb), data);
}

EXPORT void* my32_XESetWireToEvent(x64emu_t* emu, void* display, int32_t event_number, void* proc)
{
    void* ret = NULL;

    ret = my->XESetWireToEvent(display, event_number, findwire_to_eventFct(proc));

    return reverse_wire_to_eventFct(my_lib, ret);
}
EXPORT void* my32_XESetEventToWire(x64emu_t* emu, void* display, int32_t event_number, void* proc)
{
    void* ret = NULL;

    ret = my->XESetEventToWire(display, event_number, findevent_to_wireFct(proc));

    return reverse_event_to_wireFct(my_lib, ret);
}

EXPORT int my32_XRegisterIMInstantiateCallback(x64emu_t* emu, void* d, void* db, void* res_name, void* res_class, void* cb, void* data)
{
    return my->XRegisterIMInstantiateCallback(d, db, res_name, res_class, findregister_imFct(cb), data);
}

EXPORT int my32_XUnregisterIMInstantiateCallback(x64emu_t* emu, void* d, void* db, void* res_name, void* res_class, void* cb, void* data)
{
    return my->XUnregisterIMInstantiateCallback(d, db, res_name, res_class, reverse_register_imFct(my_lib, cb), data);
}
extern int my32_xinput_opcode;
EXPORT int my32_XQueryExtension(x64emu_t* emu, void* display, char* name, int* major, int* first_event, int* first_error)
{
    int fevent;
    int ret = my->XQueryExtension(display, name, major, &fevent, first_error);
    if(first_event) *first_event = fevent;
    if(!ret && name && !strcmp(name, "GLX") && BOX64ENV(x11glx)) {
        // hack to force GLX to be accepted, even if not present
        // left major and first_XXX to default...
        ret = 1;
    } else if(!strcmp(name, "XInputExtension") && major) {
        my32_xinput_opcode = *major;
    } else if(!strcmp(name, "XFIXES")) {
        register_XFixes_events(fevent);
    } /*else if(ret && first_event) {
        printf_log(LOG_INFO, "X11 Extension \"%s\" first XEvent %d\n", name, *first_event);
    }*/
    return ret;
}
EXPORT int my32_XkbQueryExtension(x64emu_t* emu, void* display, char* opcode, int* event_base, int* error, int* major, int* minor)
{
    int fallback;
    int *event = event_base?event_base:&fallback;
    int ret = my->XkbQueryExtension(display, opcode, event, error, major, minor);
    if(!ret) return ret;
    register_Xkb_events(*event);
    return ret;
}
EXPORT int my32_XAddConnectionWatch(x64emu_t* emu, void* display, char* f, void* data)
{
    return my->XAddConnectionWatch(display, findXConnectionWatchProcFct(f), data);
}
#if 0

EXPORT void my32_XRemoveConnectionWatch(x64emu_t* emu, void* display, char* f, void* data)
{
    my->XRemoveConnectionWatch(display, findXConnectionWatchProcFct(f), data);
}

EXPORT void* my32_XSetAfterFunction(x64emu_t* emu, void* display, void* f)
{

    return reverse_XSynchronizeProcFct(my_lib, my->XSetAfterFunction(display, findXSynchronizeProcFct(f)));
}
#endif

EXPORT void* my32_XSynchronize(x64emu_t* emu, void* display, int onoff)
{
    return reverse_XSynchronizeProcFct(my_lib, my->XSynchronize(display, onoff));
}

EXPORT void* my32_XOpenDisplay(void* name)
{
    void* ret = my->XOpenDisplay(name);
    if(ret && BOX64ENV(x11sync)) {my->XSynchronize(ret, 1); printf_log(LOG_INFO, "Forcing Syncronized operation on Display %p\n", ret);}
    return ret;
}

EXPORT int my32_XCloseDisplay(x64emu_t* emu, void* dpy)
{
    int ret = my->XCloseDisplay(dpy);
    if(!ret) delDisplay(dpy);
    return ret;
}


EXPORT XID my32_XCreateWindow(x64emu_t* emu, void* d, XID Window, int x, int y, uint32_t width, uint32_t height, uint32_t border_width, int depth, uint32_t cl, void* visual,  unsigned long mask, my_XSetWindowAttributes_32_t* attr)
{
    my_XSetWindowAttributes_t attrib;
    if(attr)
        convert_XSetWindowAttributes_to_64(&attrib, attr);
    return my->XCreateWindow(d, Window, x, y, width, height, border_width, depth, cl, convert_Visual_to_64(d, visual), mask, attr?(&attrib):NULL);
}

EXPORT int my32_XNextEvent(x64emu_t* emu, void* dpy, my_XEvent_32_t* evt)
{
    my_XEvent_t event = {0};
    int ret = my->XNextEvent(dpy, &event);
    convertXEvent(evt, &event);
    return ret;
}

EXPORT int my32_XPeekEvent(x64emu_t* emu, void* dpy, my_XEvent_32_t* evt)
{
    my_XEvent_t event = {0};
    int ret = my->XPeekEvent(dpy, &event);
    convertXEvent(evt, &event);
    return ret;
}

EXPORT int my32_XCheckTypedEvent(x64emu_t* emu, void* dpy, int type, my_XEvent_32_t* evt)
{
    my_XEvent_t event = {0};
    int ret = my->XCheckTypedEvent(dpy, type, &event);
    if(ret) convertXEvent(evt, &event);
    return ret;
}

EXPORT int my32_XCheckTypedWindowEvent(x64emu_t* emu, void* dpy, XID window, int type, my_XEvent_32_t* evt)
{
    my_XEvent_t event = {0};
    int ret = my->XCheckTypedWindowEvent(dpy, window, type, &event);
    if(ret) convertXEvent(evt, &event);
    return ret;
}

EXPORT int my32_XCheckWindowEvent(x64emu_t* emu, void* dpy, XID window, long mask, my_XEvent_32_t* evt)
{
    my_XEvent_t event = {0};
    int ret = my->XCheckWindowEvent(dpy, window, mask, &event);
    if(ret) convertXEvent(evt, &event);
    return ret;
}

EXPORT int my32_XWindowEvent(x64emu_t* emu, void* dpy, XID window, long mask, my_XEvent_32_t* evt)
{
    my_XEvent_t event = {0};
    int ret = my->XWindowEvent(dpy, window, mask, &event);
    convertXEvent(evt, &event);
    return ret;
}

EXPORT int my32_XMaskEvent(x64emu_t* emu, void* dpy, long mask, my_XEvent_32_t* evt)
{
    my_XEvent_t event = {0};
    int ret = my->XMaskEvent(dpy, mask, &event);
    if(ret) convertXEvent(evt, &event);
    return ret;
}

EXPORT int my32_XSendEvent(x64emu_t* emu, void* dpy, XID window, int propagate, long mask, my_XEvent_32_t* evt)
{
    my_XEvent_t event = {0};
    if(evt->type==XEVT_ClientMessage && evt->xclient.send_event) {
        evt->xany.display = to_ptrv(dpy);    // some program don't setup this data because the server will
    }
    if(evt) unconvertXEvent(&event, evt);
    return my->XSendEvent(dpy, window, propagate, mask, evt?(&event):NULL);
}

EXPORT int my32_XGetEventData(x64emu_t* emu, void* dpy, my_XEvent_32_t* evt)
{
    my_XEvent_t event = {0};
    if(evt) unconvertXEvent(&event, evt);
    int ret = my->XGetEventData(dpy, &event);
    if(ret) {
        inplace_XEventData_shring(&event);
        convertXEvent(evt, &event);
    }
    return ret;
}

EXPORT void my32_XFreeEventData(x64emu_t* emu, void* dpy, my_XEvent_32_t* evt)
{
    my_XEvent_t event = {0};
    if(evt) {
        unconvertXEvent(&event, evt);
        inplace_XEventData_enlarge(&event);
    }
    my->XFreeEventData(dpy, &event);
    convertXEvent(evt, &event);
}

EXPORT int my32_XRefreshKeyboardMapping(x64emu_t* emu, my_XMappingEvent_32_t* evt)
{
    my_XMappingEvent_t event = {0};
    event.type = evt->type;
    event.serial = from_ulong(evt->serial);
    event.send_event = evt->send_event;
    event.display = getDisplay(from_ptrv(evt->display));
    event.window = from_ulong(evt->window);
    event.request = evt->request;
    event.first_keycode = evt->first_keycode;
    event.count = evt->count;
    return my->XRefreshKeyboardMapping(&event);
}

EXPORT void* my32_XGetKeyboardMapping(x64emu_t* emu, void* dpy, uint8_t first, int count, int* ret_count)
{
    unsigned long* ret = my->XGetKeyboardMapping(dpy, first, count, ret_count);
    if(ret) {
        //inplace reduction of ulong...
        int cnt = count * *ret_count;
        ulong_t* ret_s = (ulong_t*)ret;
        for(int i=0; i<cnt; ++i)
            ret_s[i] = to_ulong(ret[i]);
    }
    return ret;
}

EXPORT int my32_XkbRefreshKeyboardMapping(x64emu_t* emu, my_XkbMapNotifyEvent_32_t* evt)
{
    my_XkbMapNotifyEvent_t event = {0};
    event.type = evt->type;
    event.serial = from_ulong(evt->serial);
    event.send_event = evt->send_event;
    event.display = getDisplay(from_ptrv(evt->display));
    event.time = from_ulong(evt->time);
    event.xkb_type = evt->xkb_type;
    event.device = evt->device;
    event.changed = evt->changed;
    event.flags = evt->flags;
    event.first_type = evt->first_type;
    event.num_types = evt->num_types;
    event.min_key_code = evt->min_key_code;
    event.max_key_code = evt->max_key_code;
    event.first_key_sym = evt->first_key_sym;
    event.first_key_act = evt->first_key_act;
    event.first_key_behavior = evt->first_key_behavior;
    event.first_key_explicit = evt->first_key_explicit;
    event.first_modmap_key = evt->first_modmap_key;
    event.first_vmodmap_key = evt->first_vmodmap_key;
    event.num_key_syms = evt->num_key_syms;
    event.num_key_acts = evt->num_key_acts;
    event.num_key_behaviors = evt->num_key_behaviors;
    event.num_key_explicit = evt->num_key_explicit;
    event.num_modmap_keys = evt->num_modmap_keys;
    event.num_vmodmap_keys = evt->num_vmodmap_keys;
    event.vmods = evt->vmods;
    return my->XkbRefreshKeyboardMapping(&event);
}

EXPORT unsigned long my32_XLookupKeysym(x64emu_t* emu, my_XEvent_32_t* evt, int index)
{
    my_XEvent_t event = {0};
    if(evt) unconvertXEvent(&event, evt);
    return my->XLookupKeysym(evt?(&event):NULL, index);
}

EXPORT int my32_XLookupString(x64emu_t* emu, my_XEvent_32_t* evt, void* buff, int len, ulong_t* keysym, void* status)
{
    my_XEvent_t event = {0};
    XID keysym_l = 0;
    if(evt) unconvertXEvent(&event, evt);
    int ret = my->XLookupString(evt?(&event):NULL, buff, len, keysym?(&keysym_l):NULL, status);
    if(keysym) *keysym = to_ulong(keysym_l);
    return ret;
}

EXPORT int my32_XmbLookupString(x64emu_t* emu, void* xic, my_XEvent_32_t* evt, void* buff, int len, ulong_t* keysym, void* status)
{
    my_XEvent_t event = {0};
    XID keysym_l = 0;
    if(evt) unconvertXEvent(&event, evt);
    int ret = my->XmbLookupString(xic, evt?(&event):NULL, buff, len, keysym?(&keysym_l):NULL, status);
    if(keysym) *keysym = to_ulong(keysym_l);
    return ret;
}

EXPORT int my32_XwcLookupString(x64emu_t* emu, void* xic, my_XEvent_32_t* evt, void* buff, int len, ulong_t* keysym, void* status)
{
    my_XEvent_t event = {0};
    XID keysym_l = 0;
    if(evt) unconvertXEvent(&event, evt);
    int ret = my->XwcLookupString(xic, evt?(&event):NULL, buff, len, keysym?(&keysym_l):NULL, status);
    if(keysym) *keysym = to_ulong(keysym_l);
    return ret;
}

EXPORT int my32_Xutf8LookupString(x64emu_t* emu, void* xic, my_XEvent_32_t* evt, void* buff, int len, ulong_t* keysym, void* status)
{
    my_XEvent_t event = {0};
    XID keysym_l = 0;
    if(evt) unconvertXEvent(&event, evt);
    int ret = my->Xutf8LookupString(xic, evt?(&event):NULL, buff, len, keysym?(&keysym_l):NULL, status);
    if(keysym) *keysym = to_ulong(keysym_l);
    return ret;
}

EXPORT int my32_XSetWMProtocols(x64emu_t* emu, void* dpy, XID window, XID_32* protocol, int count)
{
    XID list[count];
    if(protocol)
        for(int i=0; i<count; ++i)
            list[i] = from_ulong(protocol[i]);
    return my->XSetWMProtocols(dpy, window, protocol?list:NULL, count);
}

EXPORT int my32_XSetWMHints(x64emu_t* emu, void* dpy, XID window, void* hints)
{
    inplace_enlarge_wmhints(hints);
    int ret = my->XSetWMHints(dpy, window, hints);
    inplace_shrink_wmhints(hints);
    return ret;
}

EXPORT void* my32_XGetWMHints(x64emu_t* emu, void* dpy, XID window)
{
    void* ret = my->XGetWMHints(dpy, window);
    inplace_shrink_wmhints(ret);
    return ret;
}

EXPORT int my32_XSetWMNormalHints(x64emu_t* emu, void* dpy, XID window, void* hints)
{
    inplace_enlarge_wmsizehints(hints);
    my->XSetWMNormalHints(dpy, window, hints);
    inplace_shrink_wmsizehints(hints);
}

EXPORT int my32_XGetWMNormalHints(x64emu_t* emu, void* dpy, XID window, void* hints, long_t* supplied)
{
    long supplied_l = 0;
    int hints_l[17+2] = {0};
    int ret = my->XGetWMNormalHints(dpy, window, hints?hints_l:NULL, supplied?(&supplied_l):NULL);
    if(supplied) *supplied = to_long(supplied_l);
    if(hints) {
        *(long_t*)hints = to_long(*(long*)hints_l);
        memcpy(hints+4, hints_l+2, 17*4);
    }
    return ret;
}
EXPORT int my32_XGetWMProtocols(x64emu_t* emu, void* dpy, XID window, void* hints, long_t* supplied)
{
    long supplied_l = 0;
    int hints_l[17+2] = {0};
    int ret = my->XGetWMProtocols(dpy, window, hints?hints_l:NULL, supplied?(&supplied_l):NULL);
    if(supplied) *supplied = to_long(supplied_l);
    if(hints) {
        *(long_t*)hints = to_long(*(long*)hints_l);
        memcpy(hints+4, hints_l+2, 17*4);
    }
    return ret;
}
EXPORT void* my32__XGetRequest(x64emu_t* emu, my_XDisplay_t* dpy, uint8_t type, size_t len)
{
    // check if asynchandler needs updated wrapping
    struct my_XInternalAsync * p = dpy->async_handlers;
    while(p) {
        if(GetNativeFnc((uintptr_t)p->handler)!=p->handler) {
            // needs wrapping and autobridge!
            void* new_handler = find_async_handler_Fct(p->handler);
            AddAutomaticBridge(my_lib->w.bridge, iFpppip_32, new_handler, 0, "async_handler");
            p->handler = new_handler;
        }
        p = p->next;
    }

    return my->_XGetRequest(dpy, type, len);
}

EXPORT int my32_XStringListToTextProperty(x64emu_t* emu, ptr_t* list, int count, void* text)
{
    char* l_list[count];
    if(list)
        for(int i=0; i<count; ++i)
            l_list[i] = from_ptrv(list[i]);
    struct_pLiL_t text_l = {0};
    int ret = my->XStringListToTextProperty(list?(&l_list):NULL, count, &text_l);
    to_struct_pLiL(to_ptrv(text), &text_l);
    return ret;
}

EXPORT int my32_Xutf8TextListToTextProperty(x64emu_t* emu, void* dpy, ptr_t* list, int count, uint32_t style, void* text)
{
    char* l_list[count];
    if(list)
        for(int i=0; i<count; ++i)
            l_list[i] = from_ptrv(list[i]);
    struct_pLiL_t text_l = {0};
    int ret = my->Xutf8TextListToTextProperty(dpy, list?(&l_list):NULL, count, style, &text_l);
    to_struct_pLiL(to_ptrv(text), &text_l);
    return ret;
}

EXPORT int my32_XmbTextListToTextProperty(x64emu_t* emu, void* dpy, ptr_t* list, int count, uint32_t style, void* text)
{
    char* l_list[count];
    if(list)
        for(int i=0; i<count; ++i)
            l_list[i] = from_ptrv(list[i]);
    struct_pLiL_t text_l = {0};
    int ret = my->XmbTextListToTextProperty(dpy, list?(&l_list):NULL, count, style, &text_l);
    to_struct_pLiL(to_ptrv(text), &text_l);
    return ret;
}

EXPORT int my32_XwcTextListToTextProperty(x64emu_t* emu, void* dpy, ptr_t* list, int count, uint32_t style, void* text)
{
    char* l_list[count];
    if(list)
        for(int i=0; i<count; ++i)
            l_list[i] = from_ptrv(list[i]);
    struct_pLiL_t text_l = {0};
    int ret = my->XwcTextListToTextProperty(dpy, list?(&l_list):NULL, count, style, &text_l);
    to_struct_pLiL(to_ptrv(text), &text_l);
    return ret;
}

EXPORT int my32_XGetWindowAttributes(x64emu_t* emu, void* dpy, XID window, my_XWindowAttributes_32_t* attr)
{
    static my_Screen_32_t screen32 = {0};
    my_XWindowAttributes_t l_attr = {0};
    int ret = my->XGetWindowAttributes(dpy, window, &l_attr);
    convert_XWindowAttributes_to_32(dpy, attr, &l_attr);
    if(ret) {
        attr->screen = to_ptrv(&screen32);
        convert_Screen_to_32(&screen32, l_attr.screen);
    }
    return ret;
}

EXPORT int my32_XChangeProperty(x64emu_t* emu, void* dpy, XID window, XID prop, XID type, int fmt, int mode, void* data, int n)
{
    unsigned long data_l[10];
    void* tmp = NULL;
    unsigned long* pdata_l = (unsigned long*)&data_l;
    if(fmt==32) {
        if(n>10) {
            // there can be properties too big to fit on the stack
            tmp = box_malloc(n*sizeof(unsigned long));
            pdata_l = tmp;
        }
        for(int i=0; i<n; ++i)
            pdata_l[i] = from_ulong(((ulong_t*)data)[i]);
        data = pdata_l;
    }
    int ret = my->XChangeProperty(dpy, window, prop, type, fmt, mode, data, n);
    if(tmp)
        box_free(tmp);
}

EXPORT void my32_XSetWMProperties(x64emu_t* emu, void* dpy, XID window, void* window_name, void* icon_name, ptr_t* argv, int argc, void* normal_hints, my_XWMHints_32_t* wm_hints, ptr_t* class_hints)
{
    struct_pLiL_t window_name_l;
    struct_pLiL_t icon_name_l;
    int wm_size_l[17+2] = {0};
    my_XWMHints_t wm_hints_l = {0};
    char* class_hints_l[2] = {0};
    char* argv_l[argc+1];

    if(window_name)
        from_struct_pLiL(&window_name_l, to_ptrv(window_name));
    if(icon_name)
        from_struct_pLiL(&icon_name_l, to_ptrv(icon_name));
    if(normal_hints)
        convert_XSizeHints_to_64(&wm_size_l, normal_hints);
    if(wm_hints)
        convert_XWMints_to_64(&wm_hints_l, wm_hints);
    if(class_hints) {
        class_hints_l[0] = from_ptrv(class_hints[0]);
        class_hints_l[1] = from_ptrv(class_hints[1]);
    }
    if(argv) {
        memset(argv_l, 0, sizeof(argv_l));
        for(int i=0; i<argc; ++i)
            argv_l[i] = from_ptrv(argv[i]);
    }
    my->XSetWMProperties(dpy, window, window_name?(&window_name_l):NULL, icon_name?(&icon_name_l):NULL, argv?argv_l:NULL, argc, normal_hints?(&wm_size_l):NULL, wm_hints?(&wm_hints_l):NULL, class_hints?(&class_hints_l):NULL);
}

EXPORT void my32_XSetWMSizeHints(x64emu_t* emu, void* dpy, XID window, void* hints, XID atom)
{
    int hints_l[17+2] = {0};
    convert_XSizeHints_to_64(&hints_l, hints);
    my->XSetWMSizeHints(dpy, window, &hints_l, atom);
}

EXPORT void my32_Xutf8SetWMProperties(x64emu_t* emu, void* dpy, XID window, void* window_name, void* icon_name, ptr_t* argv, int argc, void* normal_hints, my_XWMHints_32_t* wm_hints, ptr_t* class_hints)
{
    int wm_size_l[17+2] = {0};
    my_XWMHints_t wm_hints_l = {0};
    char* class_hints_l[2] = {0};
    char* argv_l[argc+1];

    if(normal_hints)
        convert_XSizeHints_to_64(&wm_size_l, normal_hints);
    if(wm_hints)
        convert_XWMints_to_64(&wm_hints_l, wm_hints);
    if(class_hints) {
        class_hints_l[0] = from_ptrv(class_hints[0]);
        class_hints_l[1] = from_ptrv(class_hints[1]);
    }
    if(argv) {
        memset(argv_l, 0, sizeof(argv_l));
        for(int i=0; i<argc; ++i)
            argv_l[i] = from_ptrv(argv[i]);
    }
    my->Xutf8SetWMProperties(dpy, window, window_name, icon_name, argv?argv_l:NULL, argc, normal_hints?(&wm_size_l):NULL, wm_hints?(&wm_hints_l):NULL, class_hints?(&class_hints_l):NULL);
}

EXPORT void my32_XmbSetWMProperties(x64emu_t* emu, void* dpy, XID window, void* window_name, void* icon_name, ptr_t* argv, int argc, void* normal_hints, my_XWMHints_32_t* wm_hints, ptr_t* class_hints)
{
    int wm_size_l[17+2] = {0};
    my_XWMHints_t wm_hints_l = {0};
    char* class_hints_l[2] = {0};
    char* argv_l[argc+1];

    if(normal_hints)
        convert_XSizeHints_to_64(&wm_size_l, normal_hints);
    if(wm_hints)
        convert_XWMints_to_64(&wm_hints_l, wm_hints);
    if(class_hints) {
        class_hints_l[0] = from_ptrv(class_hints[0]);
        class_hints_l[1] = from_ptrv(class_hints[1]);
    }
    if(argv) {
        memset(argv_l, 0, sizeof(argv_l));
        for(int i=0; i<argc; ++i)
            argv_l[i] = from_ptrv(argv[i]);
    }
    my->XmbSetWMProperties(dpy, window, window_name, icon_name, argv?argv_l:NULL, argc, normal_hints?(&wm_size_l):NULL, wm_hints?(&wm_hints_l):NULL, class_hints?(&class_hints_l):NULL);
}


EXPORT void* my32_XListExtensions(x64emu_t* emu, void* dpy, int* n)
{
    char** ret = my->XListExtensions(dpy, n);
    if(!ret) return NULL;
    ptr_t* ret_s = (ptr_t*)ret;
    // shrinking
    for(int i=0; i<*n; ++i)
        ret_s[i] = to_ptrv(ret[i]);
    ret_s[*n] = 0;
    return ret;
}

EXPORT int my32_XFreeExtensionList(x64emu_t* emu, ptr_t* list)
{
    // need to expand back the list
    int n = 0;
    //first grab n
    while(list[n]) ++n;
    // now expand, backward order
    void** list_l = (void**)list;
    for(int i=n-1; i>=0; --i)
        list_l[i] = from_ptrv(list[i]);
    return my->XFreeExtensionList(list);
}

EXPORT int my32_XQueryTree(x64emu_t* emu, void* dpy, XID window, XID_32* root, XID_32* parent, ptr_t* children, uint32_t* n)
{
    XID root_l = 0;
    XID parent_l = 0;
    XID* children_l = NULL;
    int ret = my->XQueryTree(dpy, window, &root_l, &parent_l, &children_l, n);
    *root = to_ulong(root_l);
    *parent = to_ulong(parent_l);
    *children = to_ptrv(children_l);
    if(children_l)
        for(int i=0; i<*n; ++i)
            ((XID_32*)children_l)[i] = to_ulong(children_l[i]);
    return ret;
}

EXPORT int my32_Xutf8TextPropertyToTextList(x64emu_t* emu, void* dpy, void* prop, ptr_t* list, int* count)
{
    void** list_l = NULL;
    int ret = my->Xutf8TextPropertyToTextList(dpy, prop, &list_l, count);
    if(list_l && *count) {
        for(int i=0; i<*count; ++i)
            ((ptr_t*)list_l)[i] = to_ptrv(list_l[i]);
    }
    // put end marker, for expansion
    if(list_l)
        ((ptr_t*)list_l)[*count] = 0;
    *list = to_ptrv(list_l);
    return ret;
}

EXPORT int my32_XmbTextPropertyToTextList(x64emu_t* emu, void* dpy, void* prop, ptr_t* list, int* count)
{
    void** list_l = NULL;
    int ret = my->XmbTextPropertyToTextList(dpy, prop, &list_l, count);
    if(list_l && *count) {
        for(int i=0; i<*count; ++i)
            ((ptr_t*)list_l)[i] = to_ptrv(list_l[i]);
    }
    // put end marker, for expansion
    if(list_l)
        ((ptr_t*)list_l)[*count] = 0;
    *list = to_ptrv(list_l);
    return ret;
}

EXPORT int my32_XwcTextPropertyToTextList(x64emu_t* emu, void* dpy, void* prop, ptr_t* list, int* count)
{
    void** list_l = NULL;
    int ret = my->XwcTextPropertyToTextList(dpy, prop, &list_l, count);
    if(list_l && *count) {
        for(int i=0; i<*count; ++i)
            ((ptr_t*)list_l)[i] = to_ptrv(list_l[i]);
    }
    // put end marker, for expansion
    if(list_l)
        ((ptr_t*)list_l)[*count] = 0;
    *list = to_ptrv(list_l);
    return ret;
}

EXPORT void my32_XFreeStringList(x64emu_t* emu, ptr_t* list)
{
    // need to find size of list
    int n = 0;
    while(list[n]) ++n;
    // inplace string list expand
    for(int i=n-1; i>=0; --i)
        ((void**)list)[i] = from_ptrv(list[i]);

    my->XFreeStringList(list);
}

EXPORT void* my32_XListFonts(x64emu_t* emu, void* dpy, void* pat, int maxnames, int* count)
{
    void** ret = my->XListFonts(dpy, pat, maxnames, count);
    if(ret && *count) {
        for(int i=0; i<*count; ++i)
            ((ptr_t*)ret)[i] = to_ptrv(ret[i]);
    }
    // put end marker, for expansion
    if(ret)
        ((ptr_t*)ret)[*count] = 0;
    return ret;
}

EXPORT void my32_XFreeFontNames(x64emu_t* emu, ptr_t* list)
{
    // need to find size of list
    int n = 0;
    while(list[n]) ++n;
    // inplace string list expand
    for(int i=n-1; i>=0; --i)
        ((void**)list)[i] = from_ptrv(list[i]);

    my->XFreeFontNames(list);
}

EXPORT int my32_XFreeColors(x64emu_t* emu, void* dpy, XID map, ulong_t* pixels, int npixels, unsigned long planes)
{
    unsigned long pixels_l[npixels];
    for(int i=0; i<npixels; ++i)
        pixels_l[i] = from_ulong(pixels[i]);
    return my->XFreeColors(dpy, map, pixels_l, npixels, planes);
}

EXPORT void* my32_XGetModifierMapping(x64emu_t* emu, void* dpy)
{
    void *ret = my->XGetModifierMapping(dpy);
    inplace_XModifierKeymap_shrink(ret);
    return ret;
}

EXPORT int my32_XFreeModifiermap(x64emu_t* emu, void* map)
{
    inplace_XModifierKeymap_enlarge(map);
    return my->XFreeModifiermap(map);
}

EXPORT int my32_XInternAtoms(x64emu_t* emu, void* dpy, ptr_t* names, int count, int only, XID_32* atoms)
{
    char* names_l[count];
    XID atoms_l[count];
    for(int i=0; i<count; ++i)
        names_l[i] = from_ptrv(names[i]);
    memset(atoms_l, 0, sizeof(atoms_l));
    int ret = my->XInternAtoms(dpy, names_l, count, only, atoms_l);
    for(int i=0; i<count; ++i)
        atoms[i] = to_ulong(atoms_l[i]);
    return ret;
}

EXPORT void* my32_XGetIMValues(x64emu_t* emu, void* xim, ptr_t* b)
{
    int n = 0;
    void* r;
    while(b[n]) {
        void* ret = my->XGetIMValues(xim, from_ptrv(b[n]), &r, NULL, NULL);
        if(ret)
            return ret;
        b[n+1] = to_ptrv(r);
        n+=2;
    }
    return NULL;
}

EXPORT void* my32_XGetVisualInfo(x64emu_t* emu, void* dpy, long mask, my_XVisualInfo_32_t* template, int* n)
{
    my_XVisualInfo_t template_l = {0};
    if(template) convert_XVisualInfo_to_64_novisual(dpy, &template_l, template);
    void* ret = my->XGetVisualInfo(dpy, mask, template?(&template_l):NULL, n);
    if(ret) {
        my_XVisualInfo_t* src = ret;
        my_XVisualInfo_32_t* dst = ret;
        for(int i=0; i<*n; ++i)
            convert_XVisualInfo_to_32(dpy, &dst[i], &src[i]);
    }
    return ret;
}

EXPORT XID my32_XVisualIDFromVisual(x64emu_t* emu, my_Visual_32_t* v)
{
    return from_ulong(v->visualid);
}

EXPORT XID my32_XCreateColormap(x64emu_t* emu, void* dpy, XID w, my_Visual_32_t* v, int alloc)
{
    return my->XCreateColormap(dpy, w, convert_Visual_to_64(dpy, v), alloc);
}

EXPORT int my32_XQueryColors(x64emu_t* emu, void* dpy, XID map, my_XColor_32_t* defs, int ncolor)
{
    struct_LWWWcc_t defs_l[ncolor];
    for(int i=0; i<ncolor; ++i)
        from_struct_LWWWcc(defs_l+i, to_ptrv(defs+i));
    int ret = my->XQueryColors(dpy, map, defs_l, ncolor);
    for(int i=0; i<ncolor; ++i)
        to_struct_LWWWcc(to_ptrv(defs+i), defs_l+i);
    return ret;
}

EXPORT int my32_XStoreColors(x64emu_t* emu, void* dpy, XID map, my_XColor_32_t* defs, int ncolor)
{
    struct_LWWWcc_t defs_l[ncolor];
    for(int i=0; i<ncolor; ++i)
        from_struct_LWWWcc(defs_l+i, to_ptrv(defs+i));
    int ret = my->XStoreColors(dpy, map, defs_l, ncolor);
    for(int i=0; i<ncolor; ++i)
        to_struct_LWWWcc(to_ptrv(defs+i), defs_l+i);
    return ret;
}

EXPORT int my32_XFreeFont(x64emu_t* emu, void* dpy, void* f)
{
    inplace_XFontStruct_enlarge(f);
    return my->XFreeFont(dpy, f);
}

EXPORT void* my32_XCreateFontSet(x64emu_t* emu, void* dpy, void* name, ptr_t* missing, int* missing_count, ptr_t* string)
{
    void** missing_l = NULL;
    void* string_l = NULL;
    void* ret = my->XCreateFontSet(dpy, name, &missing_l, missing_count, string?(&string_l):NULL);
    if(string) *string = to_ptrv(string_l);
    // inplace string list shrink
    *missing = to_ptrv(missing_l);
    if(missing_l && *missing_count) {
        for(int i=0; i<*missing_count; ++i)
            ((ptr_t*)missing_l)[i] = to_ptrv(missing_l[i]);
    }
    // put end marker, for expansion
    if(missing_l)
        ((ptr_t*)missing_l)[*missing_count] = 0;
    my_XFontSet_32_t* set = box32_calloc(1, sizeof(my_XFontSet_32_t));
    set->fontset = ret;
    return set;
}

EXPORT int my32_XFontsOfFontSet(x64emu_t* emu, my_XFontSet_32_t* set, ptr_t* fonts_ret, ptr_t* names_ret)
{
    void* fonts_ret_l = NULL;
    void* names_ret_l = NULL;
    int ret = my->XFontsOfFontSet(set->fontset, fonts_ret?(&fonts_ret_l):NULL, names_ret?(&names_ret_l):NULL);
    if(ret<=0)
        return ret;
    if(names_ret) {
        int j = set->names_size++;
        set->names = box32_realloc(set->names, sizeof(ptr_t*)*set->names_size);
        set->names[j] = box32_malloc(sizeof(ptr_t)*ret);
        for(int i=0; i<ret; ++i)
            set->names[j][i] = to_ptrv(((void**)names_ret_l)[i]);
        *names_ret = to_ptrv(set->names[j]);
    }
    if(fonts_ret) {
        int j = set->fonts_size++;
        set->fonts = box32_realloc(set->fonts, sizeof(ptr_t*)*set->fonts_size);
        set->fonts[j] = box32_malloc(sizeof(ptr_t)*ret);
        for(int i=0; i<ret; ++i)
            set->fonts[j][i] = to_ptrv(((void**)fonts_ret_l)[i]);
        *fonts_ret = to_ptrv(set->fonts[j]);
    }
}

EXPORT void* my32_XExtentsOfFontSet(x64emu_t* emu, my_XFontSet_32_t* set)
{
    return my->XExtentsOfFontSet(set->fontset);
}

EXPORT void* my32_XLocaleOfFontSet(x64emu_t* emu, my_XFontSet_32_t* set)
{
    return my->XLocaleOfFontSet(set->fontset);
}

EXPORT void* my32_XBaseFontNameListOfFontSet(x64emu_t* emu, my_XFontSet_32_t* set)
{
    return my->XBaseFontNameListOfFontSet(set->fontset);
}

EXPORT int my32_XContextDependentDrawing(x64emu_t* emu, my_XFontSet_32_t* set)
{
    return my->XContextDependentDrawing(set->fontset);
}

EXPORT int my32_XDirectionalDependentDrawing(x64emu_t* emu, my_XFontSet_32_t* set)
{
    return my->XDirectionalDependentDrawing(set->fontset);
}

EXPORT int my32_XContextualDrawing(x64emu_t* emu, my_XFontSet_32_t* set)
{
    return my->XContextualDrawing(set->fontset);
}

EXPORT int my32_XmbTextEscapement(x64emu_t* emu, my_XFontSet_32_t* set, void* text, int sz)
{
    return my->XmbTextEscapement(set->fontset, text, sz);
}

EXPORT int my32_XwcTextEscapement(x64emu_t* emu, my_XFontSet_32_t* set, void* text, int sz)
{
    return my->XwcTextEscapement(set->fontset, text, sz);
}

EXPORT int my32_Xutf8TextEscapement(x64emu_t* emu, my_XFontSet_32_t* set, void* text, int sz)
{
    return my->Xutf8TextEscapement(set->fontset, text, sz);
}

EXPORT int my32_XmbTextExtents(x64emu_t* emu, my_XFontSet_32_t* set, void* text, int sz, void* ink, void* log)
{
    return my->XmbTextExtents(set->fontset, text, sz, ink, log);
}

EXPORT int my32_XwcTextExtents(x64emu_t* emu, my_XFontSet_32_t* set, void* text, int sz, void* ink, void* log)
{
    return my->XwcTextExtents(set->fontset, text, sz, ink, log);
}

EXPORT int my32_Xutf8TextExtents(x64emu_t* emu, my_XFontSet_32_t* set, void* text, int sz, void* ink, void* log)
{
    return my->Xutf8TextExtents(set->fontset, text, sz, ink, log);
}

EXPORT int my32_XmbTextPerCharExtents(x64emu_t* emu, my_XFontSet_32_t* set, void* text, int sz, void* ink, void* log, int buff_sz, void* buff, void* ink_ret, void* log_ret)
{
    return my->XmbTextPerCharExtents(set->fontset, text, sz, ink, log, buff_sz, buff, ink_ret, log_ret);
}

EXPORT int my32_XwcTextPerCharExtents(x64emu_t* emu, my_XFontSet_32_t* set, void* text, int sz, void* ink, void* log, int buff_sz, void* buff, void* ink_ret, void* log_ret)
{
    return my->XwcTextPerCharExtents(set->fontset, text, sz, ink, log, buff_sz, buff, ink_ret, log_ret);
}

EXPORT int my32_Xutf8TextPerCharExtents(x64emu_t* emu, my_XFontSet_32_t* set, void* text, int sz, void* ink, void* log, int buff_sz, void* buff, void* ink_ret, void* log_ret)
{
    return my->Xutf8TextPerCharExtents(set->fontset, text, sz, ink, log, buff_sz, buff, ink_ret, log_ret);
}

EXPORT void my32_XmbDrawString(x64emu_t* emu, void* dpy, XID d, my_XFontSet_32_t* set, void* gc, int x, int y, void* text, int sz)
{
    my->XmbDrawString(dpy, d, set->fontset, gc, x, y, text, sz);
}

EXPORT void my32_XwcDrawString(x64emu_t* emu, void* dpy, XID d, my_XFontSet_32_t* set, void* gc, int x, int y, void* text, int sz)
{
    my->XwcDrawString(dpy, d, set->fontset, gc, x, y, text, sz);
}

EXPORT void my32_Xutf8DrawString(x64emu_t* emu, void* dpy, XID d, my_XFontSet_32_t* set, void* gc, int x, int y, void* text, int sz)
{
    my->Xutf8DrawString(dpy, d, set->fontset, gc, x, y, text, sz);
}

EXPORT void my32_XmbDrawImageString(x64emu_t* emu, void* dpy, XID d, my_XFontSet_32_t* set, void* gc, int x, int y, void* text, int sz)
{
    my->XmbDrawImageString(dpy, d, set->fontset, gc, x, y, text, sz);
}

EXPORT void my32_XwcDrawImageString(x64emu_t* emu, void* dpy, XID d, my_XFontSet_32_t* set, void* gc, int x, int y, void* text, int sz)
{
    my->XwcDrawImageString(dpy, d, set->fontset, gc, x, y, text, sz);
}

EXPORT void my32_Xutf8DrawImageString(x64emu_t* emu, void* dpy, XID d, my_XFontSet_32_t* set, void* gc, int x, int y, void* text, int sz)
{
    my->Xutf8DrawImageString(dpy, d, set->fontset, gc, x, y, text, sz);
}

EXPORT void my32_XmbDrawText(x64emu_t* emu, void* dpy, XID d, void* gc, int x, int y, my_XmbTextItem_32_t* text, int sz)
{
    my_XmbTextItem_t text_l = {0};
    text_l.chars = from_ptrv(text->chars);
    text_l.nchars = text->nchars;
    text_l.delta = text->delta;
    text_l.font_set = ((my_XFontSet_32_t*)from_ptrv(text->font_set))->fontset;
    my->XmbDrawText(dpy, d, gc, x, y, &text_l, sz);
}

EXPORT void my32_XwcDrawText(x64emu_t* emu, void* dpy, XID d, void* gc, int x, int y, my_XwcTextItem_32_t* text, int sz)
{
    my_XwcTextItem_t text_l = {0};
    text_l.chars = from_ptrv(text->chars);
    text_l.nchars = text->nchars;
    text_l.delta = text->delta;
    text_l.font_set = ((my_XFontSet_32_t*)from_ptrv(text->font_set))->fontset;
    my->XwcDrawText(dpy, d, gc, x, y, &text_l, sz);
}

EXPORT void my32_Xutf8DrawText(x64emu_t* emu, void* dpy, XID d, void* gc, int x, int y, my_XmbTextItem_32_t* text, int sz)
{
    my_XmbTextItem_t text_l = {0};
    text_l.chars = from_ptrv(text->chars);
    text_l.nchars = text->nchars;
    text_l.delta = text->delta;
    text_l.font_set = ((my_XFontSet_32_t*)from_ptrv(text->font_set))->fontset;
    my->Xutf8DrawText(dpy, d, gc, x, y, &text_l, sz);
}

EXPORT void my32_XFreeFontSet(x64emu_t* emu,void* dpy, my_XFontSet_32_t* set)
{
    my->XFreeFontSet(dpy, set->fontset);
    for(int i=0; i<set->names_size; ++i)
        box32_free(set->names[i]);
    box32_free(set->names);
    for(int i=0; i<set->fonts_size; ++i)
        box32_free(set->names[i]);
    box32_free(set->fonts);
    box32_free(set);
}

EXPORT int my32_XChangeWindowAttributes(x64emu_t* emu, void* dpy, XID window, unsigned long mask, my_XSetWindowAttributes_32_t* attrs)
{
    my_XSetWindowAttributes_t attrs_l[1];
    convert_XSetWindowAttributes_to_64(attrs_l, attrs);
    return my->XChangeWindowAttributes(dpy, window, mask, attrs_l);
}

EXPORT int my32_XGetWindowProperty(x64emu_t* emu, void* dpy, XID window, XID prop, long offset, long length, int delete, XID req, XID_32* type_return, int* fmt_return, ulong_t* nitems_return, ulong_t* bytes, ptr_t*prop_return)
{
    unsigned long nitems_l = 0, bytes_l = 0, type_return_l = 0;
    void* prop_l = NULL;
    int ret = my->XGetWindowProperty(dpy, window, prop, offset, length, delete, req, &type_return_l, fmt_return, &nitems_l, &bytes_l, &prop_l);
    *nitems_return = to_ulong(nitems_l);
    *bytes = to_ulong(bytes_l);
    *prop_return = to_ptrv(prop_l);
    *type_return = to_ulong(type_return_l);
    if(!ret && *fmt_return==32) {
        // inplace shrink
        unsigned long *src = prop_l;
        ulong_t* dst = prop_l;
        for(int i=0; i<*nitems_return; ++i)
            dst[i] = to_ulong_silent(src[i]);
    }
    return ret;
}

EXPORT int my32_XTextExtents(x64emu_t* emu, my_XFontStruct_32_t* font_struct, void* string, int nchars, int* dir, int* ascent, int* descent, my_XCharStruct_32_t* overall)
{
    //XCharStruct doesn't need any changes
    inplace_XFontStruct_enlarge(font_struct);
    int ret = my->XTextExtents(font_struct, string, nchars, dir, ascent, descent, overall);
    inplace_XFontStruct_shrink(font_struct);
    return ret;
}

EXPORT int my32_XTextExtents16(x64emu_t* emu, my_XFontStruct_32_t* font_struct, void* string, int nchars, int* dir, int* ascent, int* descent, my_XCharStruct_32_t* overall)
{
    //XCharStruct doesn't need any changes
    inplace_XFontStruct_enlarge(font_struct);
    int ret = my->XTextExtents16(font_struct, string, nchars, dir, ascent, descent, overall);
    inplace_XFontStruct_shrink(font_struct);
    return ret;
}

EXPORT void* my32_XLoadQueryFont(x64emu_t* emu, void* dpy, void* name)
{
    void* ret = my->XLoadQueryFont(dpy, name);
    inplace_XFontStruct_shrink(ret);
    return ret;
}

EXPORT void* my32_XQueryFont(x64emu_t* emu, void* dpy, size_t id)
{
    void* ret = my->XQueryFont(dpy, id);
    inplace_XFontStruct_shrink(ret);
    return ret;
}

EXPORT int my32_XFreeFontInfo(x64emu_t* emu, ptr_t* names, void* free_info, int count)
{
    inplace_XFontStruct_enlarge(free_info);
    void** names_l = (void**)names;
    if(names) {
        for(int i=count-1; i>=0; --i)
            ((void**)names)[i] = from_ptrv(names[i]);
    }
    return my->XFreeFontInfo(names_l, free_info, count);
}

EXPORT int my32_XTextWidth(x64emu_t* emu, void* font, void* string, int count)
{
    inplace_XFontStruct_enlarge(font);
    int ret = my->XTextWidth(font, string, count);
    inplace_XFontProp_shrink(font);
    return ret;
}

EXPORT int my32_XTextWidth16(x64emu_t* emu, void* font, void* string, int count)
{
    inplace_XFontStruct_enlarge(font);
    int ret = my->XTextWidth16(font, string, count);
    inplace_XFontProp_shrink(font);
    return ret;
}

EXPORT void my32_XLockDisplay(x64emu_t* emu, void* dpy)
{
    my->XLockDisplay(dpy);
    // update some of the values now that the screen is locked
    refreshDisplay(dpy);
}

EXPORT int my32_XGrabServer(x64emu_t* emu, void* dpy)
{
    int ret = my->XGrabServer(dpy);
    // update some of the values now that the server is grabbed
    refreshDisplay(dpy);
    return ret;
}

EXPORT void* my32_XkbGetMap(x64emu_t* emu, void* dpy, uint32_t which, uint32_t dev)
{
    return inplace_XkbDescRec_shrink(my->XkbGetMap(dpy, which, dev));
}
EXPORT int my32_XkbGetNames(x64emu_t* emu, void* dpy, uint32_t which, my_XkbDescRec_32_t* ptr)
{
    inplace_XkbDescRec_enlarge(ptr);
    int ret = my->XkbGetNames(dpy, which, ptr);
    inplace_XkbDescRec_shrink(ptr);
    return ret;
}
EXPORT int my32_XkbGetUpdatedMap(x64emu_t* emu, void* dpy, uint32_t which, my_XkbDescRec_32_t* ptr)
{
    inplace_XkbDescRec_enlarge(ptr);
    int ret = my->XkbGetUpdatedMap(dpy, which, ptr);
    inplace_XkbDescRec_shrink(ptr);
    return ret;
}

EXPORT int my32_XkbGetControls(x64emu_t* emu, void* dpy, int which, my_XkbDescRec_32_t* ptr)
{
    inplace_XkbDescRec_enlarge(ptr);
    int ret = my->XkbGetControls(dpy, which, ptr);
    inplace_XkbDescRec_shrink(ptr);
    return ret;
}

EXPORT void my32_XkbFreeKeyboard(x64emu_t* emu, void* xkb, int which, int all)
{
    my->XkbFreeKeyboard(inplace_XkbDescRec_enlarge(xkb), which, all);
}

EXPORT void* my32_XGetMotionEvents(x64emu_t* emu, void* dpy, XID w, unsigned long start, unsigned long stop, int* n)
{
    void* ret = my->XGetMotionEvents(dpy, w, start, stop, n);
    if(ret) {
        my_XTimeCoord_t *src = ret;
        my_XTimeCoord_32_t* dst = ret;
        for(int i=0; i<*n; ++i) {
            dst[i].time = to_ulong(src[i].time);
            dst[i].x = src[i].x;
            dst[i].y = src[i].y;
        }
    }
    return ret;
}

EXPORT int my32_XGetWMColormapWindows(x64emu_t* emu, void* dpy, XID w, ptr_t* maps, int* n)
{
    void* maps_l = NULL;
    int ret = my->XGetWMColormapWindows(dpy, w, &maps_l, n);
    *maps = to_ptrv(maps_l);
    XID* src = maps_l;
    XID_32* dst = maps_l;
    for(int i=0; i<*n; ++i) {
        dst[i] = to_ulong(src[i]);
    }
    return ret;
}

EXPORT int my32_XScreenNumberOfScreen(x64emu_t* emu, void* s)
{
    my_Screen_32_t* screen = s;
    void* dpy = getDisplay(from_ptrv(screen->display));
    return my->XScreenNumberOfScreen(getScreen64(dpy, s));
}

EXPORT int my32__XReply(x64emu_t* emu, void* dpy, void* rep, int extra, int discard)
{
    int ret = my->_XReply(dpy, rep, extra, discard);
    printf_log(LOG_DEBUG, " (reply type:%hhu, length:32+%u) ", *(uint8_t*)rep, ((uint32_t*)rep)[1]);
    return ret;
}

EXPORT void* my32_XGetICValues(x64emu_t* emu, void* ic, ptr_t* V)
{
    void* ret = NULL;
    while(!ret && *V) {
        char* name = from_ptrv(V[0]);
        void* val = from_ptrv(V[1]);
        V+=2;
        if(!strcmp(name, "filterEvents")) {
            unsigned long fevent;
            ret = my->XGetICValues(ic, name, &fevent, NULL);
            // I got a value of 0xFFFF00000003, but this seems to be a valid
            // value of KeyPressMask | KeyReleaseMask, so just silently truncate
            if(!ret) *(ulong_t*)val = to_ulong_silent(fevent);
        } else {
            printf_log_prefix(2, LOG_INFO, "Warning, unknown XGetICValues of %s\n", name);
            ret = my->XGetICValues(ic, name, val, NULL);
        }
    }
    return ret;
}

#define CUSTOM_INIT                 \
    AddAutomaticBridge(lib->w.bridge, vFp_32, *(void**)dlsym(lib->w.lib, "_XLockMutex_fn"), 0, "_XLockMutex_fn"); \
    AddAutomaticBridge(lib->w.bridge, vFp_32, *(void**)dlsym(lib->w.lib, "_XUnlockMutex_fn"), 0, "_XUnlockMutex_fn"); \
    if(BOX64ENV(x11threads)) my->XInitThreads();    \
    my_context->libx11 = lib;

#define CUSTOM_FINI             \
    unregister_Xkb_events();    \
    my_context->libx11 = NULL;
#if 0
#ifdef ANDROID
#define NEEDED_LIBS "libxcb.so"
#else
#define NEEDED_LIBS "libxcb.so.1"
#endif
#endif

#include "wrappedlib_init32.h"
