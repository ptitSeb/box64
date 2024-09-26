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

#include "libtools/my_x11_defs.h"
#include "libtools/my_x11_defs_32.h"

void convertXEvent(my_XEvent_32_t* dst, my_XEvent_t* src);
void unconvertXEvent(my_XEvent_t* dst, my_XEvent_32_t* src);
typedef int (*XErrorHandler)(void *, void *);
void* my32_XSetErrorHandler(x64emu_t* t, XErrorHandler handler);
#if 0
typedef int (*XIOErrorHandler)(void *);
void* my32_XSetIOErrorHandler(x64emu_t* t, XIOErrorHandler handler);
void* my32_XESetCloseDisplay(x64emu_t* emu, void* display, int32_t extension, void* handler);
typedef int (*WireToEventProc)(void*, void*, void*);
#endif
typedef int(*EventHandler) (void*,void*,void*);
int32_t my32_XIfEvent(x64emu_t* emu, void* d,void* ev, EventHandler h, void* arg);

void UnwrapXImage(void* d, void* s);
void WrapXImage(void* d, void* s);

typedef void (*vFp_t)(void*);
typedef uint32_t (*uFv_t)(void);
typedef int32_t (*iFpl_t)(void*, intptr_t);
typedef uintptr_t (*LFpii_t)(void*, int32_t, int32_t);
typedef int32_t (*iFpiiL_t)(void*, int32_t, int32_t, uintptr_t);
typedef void* (*pFpiiuu_t)(void*, int32_t, int32_t, uint32_t, uint32_t);

#define ADDED_FUNCTIONS()       \
    GO(XInitThreads, uFv_t)     \
    GO(XLockDisplay, vFp_t)     \
    GO(XUnlockDisplay, vFp_t)

#include "generated/wrappedlibx11types32.h"

#include "wrappercallback32.h"

void convert_Screen_to_32(void* d, void* s);
void* FindDisplay(void* d);
void* getDisplay(void* d);
void convert_XErrorEvent_to_32(void* d, void* s)
{
    my_XErrorEvent_t* src = s;
    my_XErrorEvent_32_t* dst = d;
    dst->type = src->type;
    dst->display = to_ptrv(FindDisplay(src->display));
    dst->resourceid = to_ulong(src->resourceid);
    dst->serial = to_ulong(src->serial);
    dst->error_code = src->error_code;
    dst->request_code = src->request_code;
    dst->minor_code = src->minor_code;
}

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
#if 0
// wire_to_event
#define GO(A)   \
static uintptr_t my32_wire_to_event_fct_##A = 0;                      \
static int my32_wire_to_event_##A(void* dpy, void* re, void* event)   \
{                                                                   \
    return (int)RunFunctionFmt(my32_wire_to_event_fct_##A, "ppp", dpy, re, event);\
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
    if(!fct) return fct;
    if(CheckBridged(lib->w.bridge, fct))
        return (void*)CheckBridged(lib->w.bridge, fct);
    #define GO(A) if(my32_wire_to_event_##A == fct) return (void*)my32_wire_to_event_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(lib->w.bridge, iFppp, fct, 0, NULL);
}

// event_to_wire
#define GO(A)   \
static uintptr_t my32_event_to_wire_fct_##A = 0;                      \
static int my32_event_to_wire_##A(void* dpy, void* re, void* event)   \
{                                                                   \
    return (int)RunFunctionFmt(my32_event_to_wire_fct_##A, "ppp", dpy, re, event);\
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
    if(!fct) return fct;
    if(CheckBridged(lib->w.bridge, fct))
        return (void*)CheckBridged(lib->w.bridge, fct);
    #define GO(A) if(my32_event_to_wire_##A == fct) return (void*)my32_event_to_wire_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(lib->w.bridge, iFppp, fct, 0, NULL);
}
#endif
// error_handler
#define GO(A)   \
static uintptr_t my32_error_handler_fct_##A = 0;                                            \
static int my32_error_handler_##A(void* dpy, void* error)                                   \
{                                                                                           \
    static my_XErrorEvent_32_t evt = {0};                                                   \
    convert_XErrorEvent_to_32(&evt, error);                                                 \
    return (int)RunFunctionFmt(my32_error_handler_fct_##A, "pp", getDisplay(dpy), &evt);   \
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
static void* reverse_error_handlerFct(library_t* lib, void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(lib->w.bridge, fct))
        return (void*)CheckBridged(lib->w.bridge, fct);
    #define GO(A) if(my32_error_handler_##A == fct) return (void*)my32_error_handler_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(lib->w.bridge, iFpp_32, fct, 0, NULL);
}
#if 0
// ioerror_handler
#define GO(A)   \
static uintptr_t my32_ioerror_handler_fct_##A = 0;                      \
static int my32_ioerror_handler_##A(void* dpy)   \
{                                                                   \
    return (int)RunFunctionFmt(my32_ioerror_handler_fct_##A, "p", dpy);\
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
static void* reverse_ioerror_handlerFct(library_t* lib, void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(lib->w.bridge, fct))
        return (void*)CheckBridged(lib->w.bridge, fct);
    #define GO(A) if(my32_ioerror_handler_##A == fct) return (void*)my32_ioerror_handler_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(lib->w.bridge, iFp, fct, 0, NULL);
}

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

// close_display
#define GO(A)   \
static uintptr_t my32_close_display_fct_##A = 0;                      \
static int my32_close_display_##A(void* dpy, void* codes)   \
{                                                                   \
    return (int)RunFunctionFmt(my32_close_display_fct_##A, "pp", dpy, codes);\
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
    return (void*)AddBridge(lib->w.bridge, iFpp, fct, 0, NULL);
}

// register_im
#define GO(A)   \
static uintptr_t my32_register_im_fct_##A = 0;                        \
static void my32_register_im_##A(void* dpy, void* u, void* d)         \
{                                                                   \
    RunFunctionFmt(my32_register_im_fct_##A, "ppp", dpy, u, d);  \
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
    return (void*)AddBridge(lib->w.bridge, iFppp, fct, 0, NULL);
}

// XConnectionWatchProc
#define GO(A)   \
static uintptr_t my32_XConnectionWatchProc_fct_##A = 0;                               \
static void my32_XConnectionWatchProc_##A(void* dpy, void* data, int op, void* d)     \
{                                                                                   \
    RunFunctionFmt(my32_XConnectionWatchProc_fct_##A, "ppip", dpy, data, op, d);  \
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
#endif
// xifevent
#define GO(A)   \
static uintptr_t my32_xifevent_fct_##A = 0;                                 \
static int my32_xifevent_##A(void* dpy, my_XEvent_t* event, void* d)        \
{                                                                           \
    static my_XEvent_32_t evt = {0};                                        \
    convertXEvent(&evt, event);                                             \
    return RunFunctionFmt(my32_xifevent_fct_##A, "ppp", dpy, &evt, d);      \
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
#if 0
// XInternalAsyncHandler
#define GO(A)   \
static uintptr_t my32_XInternalAsyncHandler_fct_##A = 0;                                              \
static int my32_XInternalAsyncHandler_##A(void* dpy, void* rep, void* buf, int len, void* data)       \
{                                                                                                   \
    return RunFunctionFmt(my32_XInternalAsyncHandler_fct_##A, "pppip", dpy, rep, buf, len, data);  \
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
    return (void*)AddBridge(lib->w.bridge, iFppp, fct, 0, NULL);
}

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
// async_handler
#define GO(A)   \
static uintptr_t my32_async_handler_fct_##A = 0;                                      \
static int my32_async_handler_##A(void* a, void* b, void* c, int d, void* e)          \
{                                                                                   \
    return (int)RunFunctionFmt(my32_async_handler_fct_##A, "pppip", a, b, c, d, e);   \
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
#endif
// XImage function wrappers
// create_image
#define GO(A)   \
static uintptr_t my32_create_image_fct_##A = 0;                                                                                     \
static void* my32_create_image_##A(void* a, void* b, uint32_t c, int d, int e, void* f, uint32_t g, uint32_t h, int i, int j)       \
{                                                                                                                                   \
    void* ret = (void*)RunFunctionFmt(my32_create_image_fct_##A, "ppuiipuuii", FindDisplay(a), b, c, d, e, f, g, h, i, j);          \
    UnwrapXImage(ret, ret);                                                                                                         \
    return ret;                                                                                                                     \
}                                                                                                                                   \
static pFXpuiipuuii_t my32_rev_create_image_fct_##A = NULL;                                                                         \
static void* my32_rev_create_image_##A(void* a, void* b, uint32_t c, int d, int e, void* f, uint32_t g, uint32_t h, int i, int j)   \
{                                                                                                                                   \
    void* ret = my32_rev_create_image_fct_##A (FindDisplay(a), b, c, d, e, f, g, h, i, j);                                          \
    WrapXImage(ret, ret);                                                                                                           \
    return ret;                                                                                                                     \
}
SUPER()
#undef GO
static void* find_create_image_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
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
    #define GO(A) if(my32_create_image_fct_##A == (uintptr_t)fct) return my32_create_image_##A;
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
        return (void*)AddCheckBridge(lib->w.bridge, pFXpuiipuuii_32, f, 0, "ximage_create_image");
    printf_log(LOG_NONE, "Warning, no more slot for reverse 32bits libX11 create_image callback\n");
    return fct;
}
// destroy_image
#define GO(A)   \
static uintptr_t my32_destroy_image_fct_##A = 0;                    \
static int my32_destroy_image_##A(void* a)                          \
{                                                                   \
    WrapXImage(a, a);                                               \
    return (int)RunFunctionFmt(my32_destroy_image_fct_##A, "p", a); \
}                                                                   \
static iFp_t my32_rev_destroy_image_fct_##A = NULL;                 \
static int my32_rev_destroy_image_##A(void* a)                      \
{                                                                   \
    UnwrapXImage(a, a);                                             \
    return my32_rev_destroy_image_fct_##A (a);                      \
}
SUPER()
#undef GO
static void* find_destroy_image_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
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
    #define GO(A) if(my32_destroy_image_fct_##A == (uintptr_t)fct) return my32_destroy_image_##A;
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
        return (void*)AddCheckBridge(lib->w.bridge, iFp_32, f, 0, "ximage_destroy_image");
    printf_log(LOG_NONE, "Warning, no more slot for reverse 32bits libX11 destroy_image callback\n");
    return fct;
}
// get_pixel
#define GO(A)   \
static uintptr_t my32_get_pixel_fct_##A = 0;                                    \
static unsigned long my32_get_pixel_##A(void* a, int b, int c)                  \
{                                                                               \
    WrapXImage(a, a);                                                           \
    uint32_t ret = RunFunctionFmt(my32_get_pixel_fct_##A, "pii", a, b, c);      \
    UnwrapXImage(a, a);                                                         \
    return from_ulong(ret);                                                     \
}                                                                               \
static LFpii_t my32_rev_get_pixel_fct_##A = NULL;                               \
static ulong_t my32_rev_get_pixel_##A(void* a, int b, int c)                    \
{                                                                               \
    UnwrapXImage(a, a);                                                         \
    ulong_t ret = to_ulong(my32_rev_get_pixel_fct_##A (a, b, c));               \
    WrapXImage(a, a);                                                           \
    return ret;                                                                 \
}
SUPER()
#undef GO
static void* find_get_pixel_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
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
    #define GO(A) if(my32_get_pixel_fct_##A == (uintptr_t)fct) return my32_get_pixel_##A;
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
        return (void*)AddCheckBridge(lib->w.bridge, LFpii_32, f, 0, "ximage_get_pixel");
    printf_log(LOG_NONE, "Warning, no more slot for reverse 32bits libX11 get_pixel callback\n");
    return fct;
}
// put_pixel
#define GO(A)   \
static uintptr_t my32_put_pixel_fct_##A = 0;                                    \
static int my32_put_pixel_##A(void* a, int b, int c,unsigned long d)            \
{                                                                               \
    WrapXImage(a, a);                                                           \
    int ret =  (int)RunFunctionFmt(my32_put_pixel_fct_##A, "piiL", a, b, c, d); \
    UnwrapXImage(a, a);                                                         \
    return ret;                                                                 \
}                                                                               \
static iFpiiL_t my32_rev_put_pixel_fct_##A = NULL;                              \
static int my32_rev_put_pixel_##A(void* a, int b, int c, ulong_t d)             \
{                                                                               \
    UnwrapXImage(a, a);                                                         \
    int ret = to_ulong(my32_rev_put_pixel_fct_##A (a, b, c, from_ulong(d)));    \
    WrapXImage(a, a);                                                           \
    return ret;                                                                 \
}
SUPER()
#undef GO
static void* find_put_pixel_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
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
    #define GO(A) if(my32_put_pixel_fct_##A == (uintptr_t)fct) return my32_put_pixel_##A;
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
        return (void*)AddCheckBridge(lib->w.bridge, iFpiiL_32, f, 0, "ximage_put_pixel");
    printf_log(LOG_NONE, "Warning, no more slot for reverse 32bits libX11 put_pixel callback\n");
    return fct;
}
// sub_image
#define GO(A)   \
static uintptr_t my32_sub_image_fct_##A = 0;                                        \
static void* my32_sub_image_##A(void* a, int b, int c, uint32_t d, uint32_t e)      \
{                                                                                   \
    WrapXImage(a, a);                                                               \
    void* ret = (void*)RunFunctionFmt(my32_sub_image_fct_##A, "piiuu", a, b, c, d, e);\
    if(ret!=a) UnwrapXImage(ret, ret);                                              \
    UnwrapXImage(a, a);                                                             \
    return ret;                                                                     \
}                                                                                   \
static pFpiiuu_t my32_rev_sub_image_fct_##A = NULL;                                 \
static void* my32_rev_sub_image_##A(void* a, int b, int c, uint32_t d, uint32_t e)  \
{                                                                                   \
    UnwrapXImage(a, a);                                                             \
    void* ret = my32_rev_sub_image_fct_##A (a, b, c, d, e);                         \
    if(ret!=a)                                                                      \
        WrapXImage(ret, ret);                                                       \
    WrapXImage(a, a);                                                               \
    return ret;                                                                     \
}
SUPER()
#undef GO
static void* find_sub_image_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
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
    #define GO(A) if(my32_sub_image_fct_##A == (uintptr_t)fct) return my32_sub_image_##A;
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
    WrapXImage(a, a);                                               \
    int ret =  (int)RunFunctionFmt(my32_add_pixel_fct_##A, "pl", a, b); \
    UnwrapXImage(a, a);                                             \
    return ret;                                                     \
}                                                                   \
static iFpl_t my32_rev_add_pixel_fct_##A = NULL;                    \
static int my32_rev_add_pixel_##A(void* a, long_t b)                \
{                                                                   \
    UnwrapXImage(a, a);                                             \
    int ret = my32_rev_add_pixel_fct_##A (a, from_long(b));         \
    WrapXImage(a, a);                                               \
    return ret;                                                     \
}
SUPER()
#undef GO
static void* find_add_pixel_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
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
    #define GO(A) if(my32_add_pixel_fct_##A == (uintptr_t)fct) return my32_add_pixel_##A;
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

void* my32_XGetImage(x64emu_t* emu, void* disp, size_t drawable, int32_t x, int32_t y
                    , uint32_t w, uint32_t h, uint32_t plane, int32_t fmt);

int32_t my32_XPutImage(x64emu_t* emu, void* disp, size_t drawable, void* gc, void* image
                    , int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y
                    , uint32_t w, uint32_t h);

void* my32_XGetSubImage(x64emu_t* emu, void* disp, size_t drawable
                    , int32_t x, int32_t y
                    , uint32_t w, uint32_t h, size_t plane, int32_t fmt
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
    RESULT = FUNC(FIRST_ARG, va[0], va[1], NULL);       \
    break;                                                      \
case 4:                                                         \
    RESULT = FUNC(FIRST_ARG, va[0], va[1], va[2], va[3], NULL);     \
    break;                                                                          \
case 6:                                                                             \
    RESULT = FUNC(FIRST_ARG, va[0], va[1], va[2], va[3], va[4], va[5], NULL);   \
    break;                                                                                              \
case 8:                                                                                                 \
    RESULT = FUNC(FIRST_ARG, va[0], va[1], va[2], va[3], va[4], va[5], va[6], va[7], NULL); \
    break;                                                                                                                  \
case 10:                                                                                                                    \
    RESULT = FUNC(FIRST_ARG, va[0], va[1], va[2], va[3], va[4], va[5], va[6], va[7], va[8], va[9], NULL);   \
    break;                                                                                                                                          \
case 12:                                                                                                                                            \
    RESULT = FUNC(FIRST_ARG, va[0], va[1], va[2], va[3], va[4], va[5], va[6], va[7], va[8], va[9],  va[10], va[11], NULL);  \
    break;                                                                                                                                                                  \
case 14:                                                                                                                                                                    \
    RESULT = FUNC(FIRST_ARG, va[0], va[1], va[2], va[3], va[4], va[5], va[6], va[7], va[8], va[9],  va[10], va[11], va[12], va[13], NULL);  \
    break;                                                                                                                                                                                          \
case 16:                                                                                                                                                                                            \
    RESULT = FUNC(FIRST_ARG, va[0], va[1], va[2], va[3], va[4], va[5], va[6], va[7], va[8], va[9],  va[10], va[11], va[12], va[13], va[14], va[15], NULL);  \
    break;                                                                                                                                                                                                                  \
case 18:                                                                                                                                                                                                                    \
    RESULT = FUNC(FIRST_ARG, va[0], va[1], va[2], va[3], va[4], va[5], va[6], va[7], va[8], va[9],  va[10], va[11], va[12], va[13], va[14], va[15], va[16], va[17], NULL);  \
    break;                                                                                                                                                                                                                                          \
case 20:                                                                                                                                                                                                                                            \
    RESULT = FUNC(FIRST_ARG, va[0], va[1], va[2], va[3], va[4], va[5], va[6], va[7], va[8], va[9],  va[10], va[11], va[12], va[13], va[14], va[15], va[16], va[17], va[18], va[19], NULL);  \
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

#if 0
EXPORT void* my32_XVaCreateNestedList(x64emu_t* emu, int unused, uintptr_t* va) {
    int n = 0;
    while (getVArgs(emu, 1, va, n)) n+=2 ;

    for (int i = 0; i < n; i += 2) {
        SUPER()
    }

    void* res = NULL;
    VA_CALL(my->XVaCreateNestedList, unused, 1, n, res);
    return res;
}

EXPORT void* my32_XSetICValues(x64emu_t* emu, void* xic, uintptr_t* va) {
    int n = 0;
    while (getVArgs(emu, 1, va, n)) n+=2;

    for (int i = 0; i < n; i += 2) {
        SUPER()
    }

    void* res = NULL;
    VA_CALL(my->XSetICValues, xic, 1, n, res);
    return res;
}
#endif
#undef GO
#if 0

EXPORT void* my32_XSetIMValues(x64emu_t* emu, void* xim, uintptr_t* va) {
    int n = 0;
    while (getVArgs(emu, 1, va, n)) n+=2;

    #define GO(A)                                                                       \
    if (getVArgs(emu, 1, va, i) && strcmp((char*)getVArgs(emu, 1, va, i), A) == 0) {    \
        XIMCallback* origin = (XIMCallback*)getVArgs(emu, 1, va, i+1);                  \
        setVArgs(emu, 1, va, i+1, (uintptr_t)find##A##Fct(origin));                     \
    }
    for (int i = 0; i < n; i += 2) {
        SUPER()
    }
    #undef GO

    void* res = NULL;
    VA_CALL(my->XSetIMValues, xim, 1, n, res)
    return res;
}
#endif
#undef VA_CALL
#undef SUPER
EXPORT void* my32_XSetErrorHandler(x64emu_t* emu, XErrorHandler handler)
{
    void* ret = my->XSetErrorHandler(finderror_handlerFct(handler));
    return reverse_error_handlerFct(my_lib, ret);
}
#if 0

EXPORT void* my32_XSetIOErrorHandler(x64emu_t* emu, XIOErrorHandler handler)
{
    void* ret = my->XSetIOErrorHandler(findioerror_handlerFct(handler));
    return reverse_ioerror_handlerFct(my_lib, ret);
}

EXPORT void* my32_XESetError(x64emu_t* emu, void* display, int32_t extension, void* handler)
{
    void* ret = my->XESetError(display, extension, findexterror_handlerFct(handler));
    return reverse_exterror_handlerFct(my_lib, ret);
}

EXPORT void* my32_XESetCloseDisplay(x64emu_t* emu, void* display, int32_t extension, void* handler)
{
    void* ret = my->XESetCloseDisplay(display, extension, findclose_displayFct(handler));
    return reverse_close_displayFct(my_lib, ret);
}
#endif
EXPORT int32_t my32_XIfEvent(x64emu_t* emu, void* d,void* ev, EventHandler h, void* arg)
{
    my_XEvent_t event = {0};
    int32_t ret = my->XIfEvent(d, &event, findxifeventFct(h), arg);
    convertXEvent(ev, &event);
    return ret;
}
#if 0
EXPORT int32_t my32_XCheckIfEvent(x64emu_t* emu, void* d,void* ev, EventHandler h, void* arg)
{
    int32_t ret = my->XCheckIfEvent(d, ev, findxifeventFct(h), arg);
    return ret;
}

EXPORT int32_t my32_XPeekIfEvent(x64emu_t* emu, void* d,void* ev, EventHandler h, void* arg)
{
    int32_t ret = my->XPeekIfEvent(d, ev, findxifeventFct(h), arg);
    return ret;
}
#endif

EXPORT int my32_XFilterEvent(x64emu_t* emu, my_XEvent_32_t* evt, XID window)
{
    my_XEvent_t event = {0};
    unconvertXEvent(&event, evt);
    return my->XFilterEvent(&event, window);
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
    dst->obdata = to_ptrv(src->obdata);

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

    dst->obdata = from_ptrv(src->obdata);
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

    XImage *img = my->XCreateImage(disp, vis, depth, fmt, off, data, w, h, pad, bpl);
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

EXPORT void* my32_XGetImage(x64emu_t* emu, void* disp, size_t drawable, int32_t x, int32_t y
                    , uint32_t w, uint32_t h, uint32_t plane, int32_t fmt)
{

    XImage *img = my->XGetImage(disp, drawable, x, y, w, h, plane, fmt);
    if(!img)
        return img;
    // bridge all access functions...
    WrapXImage(img, img);
    return img;
}

EXPORT void my32__XInitImageFuncPtrs(x64emu_t* emu, XImage* img)
{
    my->_XInitImageFuncPtrs(img);
    WrapXImage(emu, img);
}

EXPORT int32_t my32_XPutImage(x64emu_t* emu, void* disp, size_t drawable, void* gc, void* image
                    , int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y
                    , uint32_t w, uint32_t h)
{
    UnwrapXImage(image, image); // what if the image was created on x86 side and is smaller?
    int32_t r = my->XPutImage(disp, drawable, gc, image, src_x, src_y, dst_x, dst_y, w, h);
    // bridge all access functions...
    WrapXImage(image, image);
    return r;
}

EXPORT void* my32_XGetSubImage(x64emu_t* emu, void* disp, size_t drawable
                    , int32_t x, int32_t y
                    , uint32_t w, uint32_t h, size_t plane, int32_t fmt
                    , void* image, int32_t dst_x, int32_t dst_y)
{

    UnwrapXImage(image, image);
    XImage *img = my->XGetSubImage(disp, drawable, x, y, w, h, plane, fmt, image, dst_x, dst_y);
    if(img && img!=image)
        WrapXImage(img, img);

    WrapXImage(image, image);
    return img;
}

EXPORT void my32_XDestroyImage(x64emu_t* emu, void* image)
{

    UnwrapXImage(image, image);
    my->XDestroyImage(image);
}
#if 0
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
#endif
EXPORT int my32_XQueryExtension(x64emu_t* emu, void* display, char* name, int* major, int* first_event, int* first_error)
{
    int ret = my->XQueryExtension(display, name, major, first_event, first_error);
    if(!ret && name && !strcmp(name, "GLX") && box64_x11glx) {
        // hack to force GLX to be accepted, even if not present
        // left major and first_XXX to default...
        ret = 1;
    }
    return ret;
}
#if 0
EXPORT int my32_XAddConnectionWatch(x64emu_t* emu, void* display, char* f, void* data)
{
    return my->XAddConnectionWatch(display, findXConnectionWatchProcFct(f), data);
}

EXPORT void my32_XRemoveConnectionWatch(x64emu_t* emu, void* display, char* f, void* data)
{
    my->XRemoveConnectionWatch(display, findXConnectionWatchProcFct(f), data);
}

EXPORT void* my32_XSetAfterFunction(x64emu_t* emu, void* display, void* f)
{

    return reverse_XSynchronizeProcFct(my_lib, my->XSetAfterFunction(display, findXSynchronizeProcFct(f)));
}

EXPORT void* my32_XSynchronize(x64emu_t* emu, void* display, int onoff)
{
    return reverse_XSynchronizeProcFct(my_lib, my->XSynchronize(display, onoff));
}
#endif

#define N_DISPLAY 4
#define N_SCREENS 16
my_XDisplay_t* my32_Displays_64[N_DISPLAY] = {0};
struct my_XFreeFuncs_32 my32_free_funcs_32[N_DISPLAY] = {0};
struct my_XLockPtrs_32 my32_lock_fns_32[N_DISPLAY] = {0};
my_Screen_32_t my32_screens[N_DISPLAY*N_SCREENS] = {0};
int n_screeens = 0;
my_XDisplay_32_t my32_Displays_32[N_DISPLAY] = {0};

void* getDisplay(void* d)
{
    if(!d) return d;
    for(int i=0; i<N_DISPLAY; ++i)
        if(&my32_Displays_32[i]==d || my32_Displays_64[i]==d)
            return my32_Displays_64[i];
        printf_log(LOG_INFO, "BOX32: Warning, 32bits Display %p not found\n", d);
    return d;
}

void* FindDisplay(void* d)
{
    if(!d) return d;
    for(int i=0; i<N_DISPLAY; ++i)
        if(my32_Displays_64[i]==d || &my32_Displays_32[i]==d)
            return &my32_Displays_32[i];
    return d;
}

void convert_Screen_to_32(void* d, void* s)
{
    my_Screen_t* src = s;
    my_Screen_32_t* dst = d;
    dst->ext_data = to_ptrv(src->ext_data);
    dst->display = to_ptrv(FindDisplay(src->display));
    dst->root = to_ulong(src->root);
    dst->width = src->width;
    dst->mwidth = src->mwidth;
    dst->ndepths = src->ndepths;
    dst->depths = to_ptrv(src->depths);
    dst->root_depth = src->root_depth;
    dst->root_visual = to_ptrv(src->root_visual);
    dst->default_gc = to_ptrv(src->default_gc);
    dst->cmap = to_ulong(src->cmap);
    dst->white_pixel = to_ulong(src->white_pixel);
    dst->black_pixel = to_ulong(src->black_pixel);
    dst->max_maps = src->max_maps;
    dst->backing_store = src->backing_store;
    dst->save_unders = src->save_unders;
    dst->root_input_mask = to_long(src->root_input_mask);
}

EXPORT void* my32_XOpenDisplay(x64emu_t* emu, void* d)
{
    void* r = my->XOpenDisplay(d);
    // Added automatic bridge because of thos macro from Xlibint.h
    //#define LockDisplay(d)       if ((d)->lock_fns) (*(d)->lock_fns->lock_display)(d)
    //#define UnlockDisplay(d)     if ((d)->lock_fns) (*(d)->lock_fns->unlock_display)(d)
    if(!r)
        return r;

    my_XDisplay_t* dpy = (my_XDisplay_t*)r;
    // look for a free slot, or a display already there
    my_XDisplay_32_t* ret = NULL;
    struct my_XFreeFuncs_32 *free_funcs = NULL;
    struct my_XLockPtrs_32 *lock_fns = NULL;
    for(int i=0; i<N_DISPLAY && !ret; ++i) {
        if(my32_Displays_64[i]==dpy || !my32_Displays_64[i]) {
            my32_Displays_64[i] = dpy;
            ret = &my32_Displays_32[i];
            free_funcs = &my32_free_funcs_32[i];
            ret->free_funcs = to_ptrv(free_funcs);
            lock_fns = &my32_lock_fns_32[i];
            ret->lock_fns = to_ptrv(lock_fns);
        }
    }
    if(!ret) {
        printf_log(LOG_INFO, "BOX32: No more slot available for libX11 Display!");
        return r;
    }

    bridge_t* system = my_lib->w.bridge;

    // partial copy...

    #define GO(A, W)\
    if(dpy->A)      \
        if(!CheckBridged(system, dpy->A)) \
            ret->A = AddCheckBridge(system, W, dpy->A, 0, #A); \

    #define GO2(A, B, W) \
    if(dpy->A && dpy->A->B)  \
        if(!CheckBridged(system, dpy->A->B)) \
            A->B = AddCheckBridge(system, W, dpy->A->B, 0, #B "_" #A); \

    ret->vendor = to_cstring(dpy->vendor);
    ret->fd = dpy->fd;
    ret->conn_checker = dpy->conn_checker;
    ret->proto_major_version = dpy->proto_major_version;
    ret->proto_minor_version = dpy->proto_minor_version;
    ret->xdefaults = to_cstring(dpy->xdefaults);
    ret->display_name = to_cstring(dpy->display_name);
    ret->default_screen = dpy->default_screen;
    ret->nscreens = dpy->nscreens;
    if(dpy->screens) {
        ret->screens = to_ptrv(&my32_screens[n_screeens]);
        for(int i=0; i<dpy->nscreens; ++i) {
            if(n_screeens==N_DISPLAY*N_SCREENS) {
                printf_log(LOG_INFO, "BOX32: Warning, no more libX11 Screen slots!");
                break;
            }
            convert_Screen_to_32(&my32_screens[n_screeens++], &dpy->screens[i]);
        }
    } else
        ret->screens = 0;

    GO2(free_funcs, atoms, vFp_32)
    GO2(free_funcs, modifiermap, iFp_32)
    GO2(free_funcs, key_bindings, vFp_32)
    GO2(free_funcs, context_db, vFp_32)
    GO2(free_funcs, defaultCCCs, vFp_32)
    GO2(free_funcs, clientCmaps, vFp_32)
    GO2(free_funcs, intensityMaps, vFp_32)
    GO2(free_funcs, im_filters, vFp_32)
    GO2(free_funcs, xkb, vFp_32)
    GO(resource_alloc, LFp_32)
    GO(synchandler, iFp_32)
    //TODO: ext_procs?
    //TODO: event_vec?
    //TODO: wire_vec?
    //TODO: async_handlers?
    GO2(lock_fns, lock_display, vFp_32)
    GO2(lock_fns, unlock_display, vFp_32)
    GO(idlist_alloc, vFppi_32)
    //TODO: error_vec?
    //TODO: flushes
    //TODO: im_fd_info?
    //TODO: conn_watchers
    GO(savedsynchandler, iFp_32)
    //TODO: generic_event_vec?
    //TODO: generic_event_copy_vec?


    #undef GO
    #undef GO2

    return ret;
}

EXPORT int my32_XCloseDisplay(x64emu_t* emu, void* dpy)
{
    int ret = my->XCloseDisplay(dpy);
    if(ret)
        for(int i=0; i<N_DISPLAY; ++i) {
            // crude free of ressources... not perfect
            if(my32_Displays_64[i]==dpy) {
                my32_Displays_64[i] = NULL;
                return ret;
            }
        }
    return ret;
}


EXPORT XID my32_XCreateWindow(x64emu_t* emu, void* d, XID Window, int x, int y, uint32_t width, uint32_t height, uint32_t border_width, int depth, uint32_t cl, void* visual,  unsigned long mask, my_XSetWindowAttributes_32_t* attr)
{
    my_XSetWindowAttributes_t attrib;
    if(attr) {
        attrib.background_pixmap = from_ulong(attr->background_pixmap);
        attrib.background_pixel = from_ulong(attr->background_pixel);
        attrib.border_pixmap = from_ulong(attr->border_pixmap);
        attrib.border_pixel = from_ulong(attr->border_pixel);
        attrib.bit_gravity = attr->bit_gravity;
        attrib.win_gravity = attr->win_gravity;
        attrib.backing_store = attr->backing_store;
        attrib.backing_planes = from_ulong(attr->backing_planes);
        attrib.backing_pixel = from_ulong(attr->backing_pixel);
        attrib.save_under = attr->save_under;
        attrib.event_mask = from_long(attr->event_mask);
        attrib.do_not_propagate_mask = from_long(attr->do_not_propagate_mask);
        attrib.override_redirect = attr->override_redirect;
        attrib.colormap = from_ulong(attr->colormap);
        attrib.cursor = from_ulong(attr->cursor);
    }
    return my->XCreateWindow(d, Window, x, y, width, height, border_width, depth, cl, visual, mask, attr?(&attrib):NULL);
}

void convertXEvent(my_XEvent_32_t* dst, my_XEvent_t* src)
{
    // convert the XAnyEvent first, as it's a common set
    dst->type = src->type;
    dst->xany.display = to_ptrv(FindDisplay(src->xany.display));
    dst->xany.window = to_ulong(src->xany.window);
    dst->xany.send_event = src->xany.serial;
    dst->xany.serial = to_ulong(src->xany.serial);
    switch(src->type) {
        case XEVT_KeyPress:
        case XEVT_KeyRelease:
            dst->xkey.root = to_ulong(src->xkey.root);
            dst->xkey.subwindow = to_ulong(src->xkey.subwindow);
            dst->xkey.time = to_ulong(src->xkey.time);
            dst->xkey.x = src->xkey.x;
            dst->xkey.y = src->xkey.y;
            dst->xkey.x_root = src->xkey.x_root;
            dst->xkey.y_root = src->xkey.y_root;
            dst->xkey.state = src->xkey.state;
            dst->xkey.keycode = src->xkey.keycode;
            dst->xkey.same_screen = src->xkey.same_screen;
            break;
        case XEVT_ButtonPress:
        case XEVT_ButtonRelease:
            dst->xbutton.root = to_ulong(src->xbutton.root);
            dst->xbutton.subwindow = to_ulong(src->xbutton.subwindow);
            dst->xbutton.time = to_ulong(src->xbutton.time);
            dst->xbutton.x = src->xbutton.x;
            dst->xbutton.y = src->xbutton.y;
            dst->xbutton.x_root = src->xbutton.x_root;
            dst->xbutton.y_root = src->xbutton.y_root;
            dst->xbutton.state = src->xbutton.state;
            dst->xbutton.button = src->xbutton.button;
            dst->xbutton.same_screen = src->xbutton.same_screen;
            break;
        case XEVT_MotionNotify:
            dst->xmotion.root = to_ulong(src->xmotion.root);
            dst->xmotion.subwindow = to_ulong(src->xmotion.subwindow);
            dst->xmotion.time = to_ulong(src->xmotion.time);
            dst->xmotion.x = src->xmotion.x;
            dst->xmotion.y = src->xmotion.y;
            dst->xmotion.x_root = src->xmotion.x_root;
            dst->xmotion.y_root = src->xmotion.y_root;
            dst->xmotion.state = src->xmotion.state;
            dst->xmotion.is_hint = src->xmotion.is_hint;
            dst->xmotion.same_screen = src->xmotion.same_screen;
            break;
        case XEVT_EnterNotify:
        case XEVT_LeaveNotify:
            dst->xcrossing.root = to_ulong(src->xcrossing.root);
            dst->xcrossing.subwindow = to_ulong(src->xcrossing.subwindow);
            dst->xcrossing.time = to_ulong(src->xcrossing.time);
            dst->xcrossing.x = src->xcrossing.x;
            dst->xcrossing.y = src->xcrossing.y;
            dst->xcrossing.x_root = src->xcrossing.x_root;
            dst->xcrossing.y_root = src->xcrossing.y_root;
            dst->xcrossing.mode = src->xcrossing.mode;
            dst->xcrossing.detail = src->xcrossing.detail;
            dst->xcrossing.same_screen = src->xcrossing.same_screen;
            dst->xcrossing.focus = src->xcrossing.focus;
            dst->xcrossing.state = src->xcrossing.state;
            break;
        case XEVT_FocusIn:
        case XEVT_FocusOut:
            dst->xfocus.mode = src->xfocus.mode;
            dst->xfocus.detail = src->xfocus.detail;
            break;
        case XEVT_KeymapNotify:
            memcpy(dst->xkeymap.key_vector, src->xkeymap.key_vector, 32);
            break;
        case XEVT_Expose:
            dst->xexpose.x = src->xexpose.x;
            dst->xexpose.y = src->xexpose.y;
            dst->xexpose.width = src->xexpose.width;
            dst->xexpose.height = src->xexpose.height;
            dst->xexpose.count = src->xexpose.count;
            break;
        case XEVT_GraphicsExpose:
            dst->xgraphicsexpose.x = src->xgraphicsexpose.x;
            dst->xgraphicsexpose.y = src->xgraphicsexpose.y;
            dst->xgraphicsexpose.width = src->xgraphicsexpose.width;
            dst->xgraphicsexpose.height = src->xgraphicsexpose.height;
            dst->xgraphicsexpose.count = src->xgraphicsexpose.count;
            dst->xgraphicsexpose.major_code = src->xgraphicsexpose.major_code;
            dst->xgraphicsexpose.minor_code = src->xgraphicsexpose.minor_code;
            break;
        case XEVT_NoExpose:
            dst->xnoexpose.major_code = src->xnoexpose.major_code;
            dst->xnoexpose.minor_code = src->xnoexpose.minor_code;
            break;
        case XEVT_VisibilityNotify:
            dst->xvisibility.state = src->xvisibility.state;
            break;
        case XEVT_CreateNotify:
            dst->xcreatewindow.window = to_ulong(src->xcreatewindow.window);
            dst->xcreatewindow.x = src->xcreatewindow.x;
            dst->xcreatewindow.y = src->xcreatewindow.y;
            dst->xcreatewindow.width = src->xcreatewindow.width;
            dst->xcreatewindow.height = src->xcreatewindow.height;
            dst->xcreatewindow.border_width = src->xcreatewindow.border_width;
            dst->xcreatewindow.override_redirect = src->xcreatewindow.override_redirect;
            break;
        case XEVT_DestroyNotify:
            dst->xdestroywindow.window = to_ulong(src->xdestroywindow.window);
            break;
        case XEVT_UnmapNotify:
            dst->xunmap.window = to_ulong(src->xunmap.window);
            dst->xunmap.from_configure = src->xunmap.from_configure;
            break;
        case XEVT_MapNotify:
            dst->xmap.window = to_ulong(src->xmap.window);
            dst->xmap.override_redirect = src->xmap.override_redirect;
            break;
        case XEVT_MapRequest:
            dst->xmaprequest.window = to_ulong(src->xmaprequest.window);
            break;
        case XEVT_ReparentNotify:
            dst->xreparent.window = to_ulong(src->xreparent.window);
            dst->xreparent.parent = to_ulong(src->xreparent.parent);
            dst->xreparent.x = src->xreparent.x;
            dst->xreparent.y = src->xreparent.y;
            dst->xreparent.override_redirect = src->xreparent.override_redirect;
            break;
        case XEVT_ConfigureNotify:
            dst->xconfigure.window = to_ulong(src->xconfigure.window);
            dst->xconfigure.x = src->xconfigure.x;
            dst->xconfigure.y = src->xconfigure.y;
            dst->xconfigure.width = src->xconfigure.width;
            dst->xconfigure.height = src->xconfigure.height;
            dst->xconfigure.border_width = src->xconfigure.border_width;
            dst->xconfigure.above = to_ulong(src->xconfigure.above);
            dst->xconfigure.override_redirect = src->xconfigure.override_redirect;
            break;
        case XEVT_ConfigureRequest:
            dst->xconfigurerequest.window = to_ulong(src->xconfigurerequest.window);
            dst->xconfigurerequest.x = src->xconfigurerequest.x;
            dst->xconfigurerequest.y = src->xconfigurerequest.y;
            dst->xconfigurerequest.width = src->xconfigurerequest.width;
            dst->xconfigurerequest.height = src->xconfigurerequest.height;
            dst->xconfigurerequest.border_width = src->xconfigurerequest.border_width;
            dst->xconfigurerequest.above = to_ulong(src->xconfigurerequest.above);
            dst->xconfigurerequest.detail = src->xconfigurerequest.detail;
            dst->xconfigurerequest.value_mask = to_ulong(src->xconfigurerequest.value_mask);
            break;
        case XEVT_GravityNotify:
            dst->xgravity.window = to_ulong(src->xgravity.window);
            dst->xgravity.x = src->xgravity.x;
            dst->xgravity.y = src->xgravity.y;
            break;
        case XEVT_ResizeRequest:
            dst->xresizerequest.width = src->xresizerequest.width;
            dst->xresizerequest.height = src->xresizerequest.height;
            break;
        case XEVT_CirculateNotify:
            dst->xcirculate.window = to_ulong(src->xcirculate.window);
            dst->xcirculate.place = src->xcirculate.place;
            break;
        case XEVT_CirculateRequest:
            dst->xcirculaterequest.window = to_ulong(src->xcirculaterequest.window);
            dst->xcirculaterequest.place = src->xcirculaterequest.place;
            break;
        case XEVT_PropertyNotify:
            dst->xproperty.atom = to_ulong(src->xproperty.atom);
            dst->xproperty.time = to_ulong(src->xproperty.time);
            dst->xproperty.state = src->xproperty.state;
            break;
        case XEVT_SelectionClear:
            dst->xselectionclear.selection = to_ulong(src->xselectionclear.selection);
            dst->xselectionclear.time = to_ulong(src->xselectionclear.time);
            break;
        case XEVT_SelectionRequest:
            dst->xselectionrequest.requestor = to_ulong(src->xselectionrequest.requestor);
            dst->xselectionrequest.selection = to_ulong(src->xselectionrequest.selection);
            dst->xselectionrequest.target = to_ulong(src->xselectionrequest.target);
            dst->xselectionrequest.property = to_ulong(src->xselectionrequest.property);
            dst->xselectionrequest.time = to_ulong(src->xselectionrequest.time);
            break;
        case XEVT_SelectionNotify:
            dst->xselection.selection = to_ulong(src->xselection.selection);
            dst->xselection.target = to_ulong(src->xselection.target);
            dst->xselection.property = to_ulong(src->xselection.property);
            dst->xselection.time = to_ulong(src->xselection.time);
            break;
        case XEVT_ColormapNotify:
            dst->xcolormap.colormap = to_ulong(src->xcolormap.colormap);
            dst->xcolormap.c_new = src->xcolormap.c_new;
            dst->xcolormap.state = src->xcolormap.state;
            break;
        case XEVT_ClientMessage:
            dst->xclient.message_type = to_ulong(src->xclient.message_type);
            dst->xclient.format = src->xclient.format;
            if(src->xclient.format==32)
                for(int i=0; i<5; ++i) {
                    if(((src->xclient.data.l[i]&0xffffffff80000000LL))==0xffffffff80000000LL)
                        dst->xclient.data.l[i] = to_ulong(src->xclient.data.l[i]&0xffffffff);   // negative value...
                    else
                        dst->xclient.data.l[i] = to_ulong(src->xclient.data.l[i]);
                }
            else
                memcpy(dst->xclient.data.b, src->xclient.data.b, 20);
            break;
        case XEVT_MappingNotify:
            dst->xmapping.request = src->xmapping.request;
            dst->xmapping.first_keycode = src->xmapping.first_keycode;
            dst->xmapping.count = src->xmapping.count;
            break;
        case XEVT_GenericEvent:
            dst->xgeneric.extension = src->xgeneric.extension;
            dst->xgeneric.evtype = src->xgeneric.evtype;
            break;

        default:
            printf_log(LOG_INFO, "Warning, unsupported 32bits XEvent type=%d\n", src->type);
    }
}
void unconvertXEvent(my_XEvent_t* dst, my_XEvent_32_t* src)
{
    // convert the XAnyEvent first, as it's a common set
    dst->type = src->type;
    dst->xany.display = getDisplay(from_ptrv(src->xany.display));
    dst->xany.window = from_ulong(src->xany.window);
    dst->xany.send_event = src->xany.serial;
    dst->xany.serial = from_ulong(src->xany.serial);
    switch(src->type) {
        case XEVT_KeyPress:
        case XEVT_KeyRelease:
            dst->xkey.root = from_ulong(src->xkey.root);
            dst->xkey.subwindow = from_ulong(src->xkey.subwindow);
            dst->xkey.time = from_ulong(src->xkey.time);
            dst->xkey.x = src->xkey.x;
            dst->xkey.y = src->xkey.y;
            dst->xkey.x_root = src->xkey.x_root;
            dst->xkey.y_root = src->xkey.y_root;
            dst->xkey.state = src->xkey.state;
            dst->xkey.keycode = src->xkey.keycode;
            dst->xkey.same_screen = src->xkey.same_screen;
            break;
        case XEVT_ButtonPress:
        case XEVT_ButtonRelease:
            dst->xbutton.root = from_ulong(src->xbutton.root);
            dst->xbutton.subwindow = from_ulong(src->xbutton.subwindow);
            dst->xbutton.time = from_ulong(src->xbutton.time);
            dst->xbutton.x = src->xbutton.x;
            dst->xbutton.y = src->xbutton.y;
            dst->xbutton.x_root = src->xbutton.x_root;
            dst->xbutton.y_root = src->xbutton.y_root;
            dst->xbutton.state = src->xbutton.state;
            dst->xbutton.button = src->xbutton.button;
            dst->xbutton.same_screen = src->xbutton.same_screen;
            break;
        case XEVT_MotionNotify:
            dst->xmotion.root = from_ulong(src->xmotion.root);
            dst->xmotion.subwindow = from_ulong(src->xmotion.subwindow);
            dst->xmotion.time = from_ulong(src->xmotion.time);
            dst->xmotion.x = src->xmotion.x;
            dst->xmotion.y = src->xmotion.y;
            dst->xmotion.x_root = src->xmotion.x_root;
            dst->xmotion.y_root = src->xmotion.y_root;
            dst->xmotion.state = src->xmotion.state;
            dst->xmotion.is_hint = src->xmotion.is_hint;
            dst->xmotion.same_screen = src->xmotion.same_screen;
            break;
        case XEVT_EnterNotify:
        case XEVT_LeaveNotify:
            dst->xcrossing.root = from_ulong(src->xcrossing.root);
            dst->xcrossing.subwindow = from_ulong(src->xcrossing.subwindow);
            dst->xcrossing.time = from_ulong(src->xcrossing.time);
            dst->xcrossing.x = src->xcrossing.x;
            dst->xcrossing.y = src->xcrossing.y;
            dst->xcrossing.x_root = src->xcrossing.x_root;
            dst->xcrossing.y_root = src->xcrossing.y_root;
            dst->xcrossing.mode = src->xcrossing.mode;
            dst->xcrossing.detail = src->xcrossing.detail;
            dst->xcrossing.same_screen = src->xcrossing.same_screen;
            dst->xcrossing.focus = src->xcrossing.focus;
            dst->xcrossing.state = src->xcrossing.state;
            break;
        case XEVT_FocusIn:
        case XEVT_FocusOut:
            dst->xfocus.mode = src->xfocus.mode;
            dst->xfocus.detail = src->xfocus.detail;
            break;
        case XEVT_KeymapNotify:
            memcpy(dst->xkeymap.key_vector, src->xkeymap.key_vector, 32);
            break;
        case XEVT_Expose:
            dst->xexpose.x = src->xexpose.x;
            dst->xexpose.y = src->xexpose.y;
            dst->xexpose.width = src->xexpose.width;
            dst->xexpose.height = src->xexpose.height;
            dst->xexpose.count = src->xexpose.count;
            break;
        case XEVT_GraphicsExpose:
            dst->xgraphicsexpose.x = src->xgraphicsexpose.x;
            dst->xgraphicsexpose.y = src->xgraphicsexpose.y;
            dst->xgraphicsexpose.width = src->xgraphicsexpose.width;
            dst->xgraphicsexpose.height = src->xgraphicsexpose.height;
            dst->xgraphicsexpose.count = src->xgraphicsexpose.count;
            dst->xgraphicsexpose.major_code = src->xgraphicsexpose.major_code;
            dst->xgraphicsexpose.minor_code = src->xgraphicsexpose.minor_code;
            break;
        case XEVT_NoExpose:
            dst->xnoexpose.major_code = src->xnoexpose.major_code;
            dst->xnoexpose.minor_code = src->xnoexpose.minor_code;
            break;
        case XEVT_VisibilityNotify:
            dst->xvisibility.state = src->xvisibility.state;
            break;
        case XEVT_CreateNotify:
            dst->xcreatewindow.window = from_ulong(src->xcreatewindow.window);
            dst->xcreatewindow.x = src->xcreatewindow.x;
            dst->xcreatewindow.y = src->xcreatewindow.y;
            dst->xcreatewindow.width = src->xcreatewindow.width;
            dst->xcreatewindow.height = src->xcreatewindow.height;
            dst->xcreatewindow.border_width = src->xcreatewindow.border_width;
            dst->xcreatewindow.override_redirect = src->xcreatewindow.override_redirect;
            break;
        case XEVT_DestroyNotify:
            dst->xdestroywindow.window = from_ulong(src->xdestroywindow.window);
            break;
        case XEVT_UnmapNotify:
            dst->xunmap.window = from_ulong(src->xunmap.window);
            dst->xunmap.from_configure = src->xunmap.from_configure;
            break;
        case XEVT_MapNotify:
            dst->xmap.window = from_ulong(src->xmap.window);
            dst->xmap.override_redirect = src->xmap.override_redirect;
            break;
        case XEVT_MapRequest:
            dst->xmaprequest.window = from_ulong(src->xmaprequest.window);
            break;
        case XEVT_ReparentNotify:
            dst->xreparent.window = from_ulong(src->xreparent.window);
            dst->xreparent.parent = from_ulong(src->xreparent.parent);
            dst->xreparent.x = src->xreparent.x;
            dst->xreparent.y = src->xreparent.y;
            dst->xreparent.override_redirect = src->xreparent.override_redirect;
            break;
        case XEVT_ConfigureNotify:
            dst->xconfigure.window = from_ulong(src->xconfigure.window);
            dst->xconfigure.x = src->xconfigure.x;
            dst->xconfigure.y = src->xconfigure.y;
            dst->xconfigure.width = src->xconfigure.width;
            dst->xconfigure.height = src->xconfigure.height;
            dst->xconfigure.border_width = src->xconfigure.border_width;
            dst->xconfigure.above = from_ulong(src->xconfigure.above);
            dst->xconfigure.override_redirect = src->xconfigure.override_redirect;
            break;
        case XEVT_ConfigureRequest:
            dst->xconfigurerequest.window = from_ulong(src->xconfigurerequest.window);
            dst->xconfigurerequest.x = src->xconfigurerequest.x;
            dst->xconfigurerequest.y = src->xconfigurerequest.y;
            dst->xconfigurerequest.width = src->xconfigurerequest.width;
            dst->xconfigurerequest.height = src->xconfigurerequest.height;
            dst->xconfigurerequest.border_width = src->xconfigurerequest.border_width;
            dst->xconfigurerequest.above = from_ulong(src->xconfigurerequest.above);
            dst->xconfigurerequest.detail = src->xconfigurerequest.detail;
            dst->xconfigurerequest.value_mask = from_ulong(src->xconfigurerequest.value_mask);
            break;
        case XEVT_GravityNotify:
            dst->xgravity.window = from_ulong(src->xgravity.window);
            dst->xgravity.x = src->xgravity.x;
            dst->xgravity.y = src->xgravity.y;
            break;
        case XEVT_ResizeRequest:
            dst->xresizerequest.width = src->xresizerequest.width;
            dst->xresizerequest.height = src->xresizerequest.height;
            break;
        case XEVT_CirculateNotify:
            dst->xcirculate.window = from_ulong(src->xcirculate.window);
            dst->xcirculate.place = src->xcirculate.place;
            break;
        case XEVT_CirculateRequest:
            dst->xcirculaterequest.window = from_ulong(src->xcirculaterequest.window);
            dst->xcirculaterequest.place = src->xcirculaterequest.place;
            break;
        case XEVT_PropertyNotify:
            dst->xproperty.atom = from_ulong(src->xproperty.atom);
            dst->xproperty.time = from_ulong(src->xproperty.time);
            dst->xproperty.state = src->xproperty.state;
            break;
        case XEVT_SelectionClear:
            dst->xselectionclear.selection = from_ulong(src->xselectionclear.selection);
            dst->xselectionclear.time = from_ulong(src->xselectionclear.time);
            break;
        case XEVT_SelectionRequest:
            dst->xselectionrequest.requestor = from_ulong(src->xselectionrequest.requestor);
            dst->xselectionrequest.selection = from_ulong(src->xselectionrequest.selection);
            dst->xselectionrequest.target = from_ulong(src->xselectionrequest.target);
            dst->xselectionrequest.property = from_ulong(src->xselectionrequest.property);
            dst->xselectionrequest.time = from_ulong(src->xselectionrequest.time);
            break;
        case XEVT_SelectionNotify:
            dst->xselection.selection = from_ulong(src->xselection.selection);
            dst->xselection.target = from_ulong(src->xselection.target);
            dst->xselection.property = from_ulong(src->xselection.property);
            dst->xselection.time = from_ulong(src->xselection.time);
            break;
        case XEVT_ColormapNotify:
            dst->xcolormap.colormap = from_ulong(src->xcolormap.colormap);
            dst->xcolormap.c_new = src->xcolormap.c_new;
            dst->xcolormap.state = src->xcolormap.state;
            break;
        case XEVT_ClientMessage:
            dst->xclient.message_type = from_ulong(src->xclient.message_type);
            dst->xclient.format = src->xclient.format;
            if(src->xclient.format==32)
                for(int i=0; i<5; ++i) dst->xclient.data.l[i] = from_ulong(src->xclient.data.l[i]);
            else
                memcpy(dst->xclient.data.b, src->xclient.data.b, 20);
            break;
        case XEVT_MappingNotify:
            dst->xmapping.request = src->xmapping.request;
            dst->xmapping.first_keycode = src->xmapping.first_keycode;
            dst->xmapping.count = src->xmapping.count;
            break;
        case XEVT_GenericEvent:
            dst->xgeneric.extension = src->xgeneric.extension;
            dst->xgeneric.evtype = src->xgeneric.evtype;
            break;

        default:
            printf_log(LOG_INFO, "Warning, unsupported 32bits (un)XEvent type=%d\n", src->type);
    }
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

EXPORT int my32_XSendEvent(x64emu_t* emu, void* dpy, XID window, int propagate, long mask, my_XEvent_32_t* evt)
{
    my_XEvent_t event = {0};
    if(evt) unconvertXEvent(&event, evt);
    return my->XSendEvent(dpy, window, propagate, mask, evt?(&event):NULL);
}

EXPORT unsigned long my32_XLookupKeysym(x64emu_t* emu, my_XEvent_32_t* evt, int index)
{
    my_XEvent_t event = {0};
    if(evt) unconvertXEvent(&event, evt);
    return my->XLookupKeysym(evt?(&event):NULL, index);
}

EXPORT int my32_XLookupString(x64emu_t* emu, my_XEvent_32_t* evt, void* buff, int len, void* keysym, void* status)
{
    my_XEvent_t event = {0};
    if(evt) unconvertXEvent(&event, evt);
    return my->XLookupString(evt?(&event):NULL, buff, len, keysym, status);
}

EXPORT int my32_XSetWMProtocols(x64emu_t* emu, void* dpy, XID window, XID_32* protocol, int count)
{
    XID list[count];
    if(protocol)
        for(int i=0; i<count; ++i)
            list[i] = from_ulong(protocol[i]);
    return my->XSetWMProtocols(dpy, window, protocol?list:NULL, count);
}

void convert_XWMints_to_64(void* d, void* s)
{
    my_XWMHints_t* dst = d;
    my_XWMHints_32_t* src = s;
    long flags = from_long(src->flags);
    // reverse order
    if(flags&XWMHint_WindowGroupHint)   dst->window_group = from_ulong(src->window_group);
    if(flags&XWMHint_IconMaskHint)      dst->icon_mask = from_ulong(src->icon_mask);
    if(flags&XWMHint_IconPositionHint)  {dst->icon_y = src->icon_y; dst->icon_x = src->icon_x;}
    if(flags&XWMHint_IconWindowHint)    dst->icon_window = from_ulong(src->icon_window);
    if(flags&XWMHint_IconPixmapHint)    dst->icon_pixmap = from_ulong(src->icon_pixmap);
    if(flags&XWMHint_StateHint)         dst->initial_state = src->initial_state;
    if(flags&XWMHint_InputHint)         dst->input = src->input;

    dst->flags = flags;
}
void inplace_enlarge_wmhints(void* hints)
{
    if(!hints) return;
    my_XWMHints_32_t* src = hints;
    my_XWMHints_t* dst = hints;
    long flags = from_long(src->flags);
    // reverse order
    if(flags&XWMHint_WindowGroupHint)   dst->window_group = from_ulong(src->window_group);
    if(flags&XWMHint_IconMaskHint)      dst->icon_mask = from_ulong(src->icon_mask);
    if(flags&XWMHint_IconPositionHint)  {dst->icon_y = src->icon_y; dst->icon_x = src->icon_x;}
    if(flags&XWMHint_IconWindowHint)    dst->icon_window = from_ulong(src->icon_window);
    if(flags&XWMHint_IconPixmapHint)    dst->icon_pixmap = from_ulong(src->icon_pixmap);
    if(flags&XWMHint_StateHint)         dst->initial_state = src->initial_state;
    if(flags&XWMHint_InputHint)         dst->input = src->input;

    dst->flags = flags;
}
void inplace_shrink_wmhints(void* hints)
{
    if(!hints) return;
    my_XWMHints_t* src = hints;
    my_XWMHints_32_t* dst = hints;
    long_t flags = to_long(src->flags);
    // forward order
    if(flags&XWMHint_InputHint)         dst->input = src->input;
    if(flags&XWMHint_StateHint)         dst->initial_state = src->initial_state;
    if(flags&XWMHint_IconPixmapHint)    dst->icon_pixmap = to_ulong(src->icon_pixmap);
    if(flags&XWMHint_IconWindowHint)    dst->icon_window = to_ulong(src->icon_window);
    if(flags&XWMHint_IconPositionHint)  {dst->icon_y = src->icon_y; dst->icon_x = src->icon_x;}
    if(flags&XWMHint_IconMaskHint)      dst->icon_mask = to_ulong(src->icon_mask);
    if(flags&XWMHint_WindowGroupHint)   dst->window_group = to_ulong(src->window_group);

    dst->flags = flags;
}

void convert_XSizeHints_to_64(void* d, void *s)
{
    //XSizeHints is a long flag and 17*int...
    long flags = to_long(*(long_t*)s);
    memcpy(d+8, s+4, 17*4);
    *(long*)d = flags;
}
void inplace_enlarge_wmsizehints(void* hints)
{
    //XSizeHints is a long flag and 17*int...
    long flags = to_long(*(long_t*)hints);
    memmove(hints+8, hints+4, 17*4);
    *(long*)hints = flags;
}
void inplace_shrink_wmsizehints(void* hints)
{
    //XSizeHints is a long flag and 17*int...
    long_t flags = from_long(*(long*)hints);
    memmove(hints+4, hints+8, 17*4);
    *(long_t*)hints = flags;
}

EXPORT int my32_XSetWMHints(x64emu_t* emu, void* dpy, XID window, void* hints)
{
    inplace_enlarge_wmhints(hints);
    int ret = my->XSetWMHints(dpy, window, hints);
    inplace_shrink_wmhints(hints);
    return ret;
}

EXPORT int my32_XSetWMNormalHints(x64emu_t* emu, void* dpy, XID window, void* hints)
{
    inplace_enlarge_wmsizehints(hints);
    my->XSetWMNormalHints(dpy, window, hints);
    inplace_shrink_wmsizehints(hints);
}
#if 0
EXPORT void* my32__XGetRequest(x64emu_t* emu, my_XDisplay_t* dpy, uint8_t type, size_t len)
{
    // check if asynchandler needs updated wrapping
    struct my32_XInternalAsync * p = dpy->async_handlers;
    while(p) {
        if(GetNativeFnc((uintptr_t)p->handler)!=p->handler) {
            // needs wrapping and autobridge!
            void* new_handler = find_async_handler_Fct(p->handler);
            AddAutomaticBridge(my_lib->w.bridge, iFpppip, new_handler, 0, "async_handler");
            p->handler = new_handler;
        }
        p = p->next;
    }

    return my->_XGetRequest(dpy, type, len);
}
#endif

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

void convert_XWindowAttributes_to_32(void* d, void* s)
{
    my_XWindowAttributes_t* src = s;
    my_XWindowAttributes_32_t* dst = d;
    dst->x = src->x;
    dst->y = src->y;
    dst->width = src->width;
    dst->height = src->height;
    dst->border_width = src->border_width;
    dst->depth = src->depth;
    dst->visual = to_ptrv(src->visual);
    dst->root = to_ulong(src->root);
    dst->c_class = src->c_class;
    dst->bit_gravity = src->bit_gravity;
    dst->win_gravity = src->win_gravity;
    dst->backing_store = src->backing_store;
    dst->backing_planes = to_ulong(src->backing_planes);
    dst->backing_pixel = to_ulong(src->backing_pixel);
    dst->save_under = src->save_under;
    dst->colormap = to_ulong(src->colormap);
    dst->map_installed = src->map_installed;
    dst->map_state = src->map_state;
    dst->all_event_masks = to_long(src->all_event_masks);
    dst->your_event_mask = to_long(src->your_event_mask);
    dst->do_not_propagate_mask = to_long(src->do_not_propagate_mask);
    dst->override_redirect = src->override_redirect;
    dst->screen = to_ptrv(src->screen);
}

EXPORT int my32_XGetWindowAttributes(x64emu_t* emu, void* dpy, XID window, my_XWindowAttributes_32_t* attr)
{
    static my_Screen_32_t screen32 = {0};
    my_XWindowAttributes_t l_attr = {0};
    int ret = my->XGetWindowAttributes(dpy, window, &l_attr);
    convert_XWindowAttributes_to_32(attr, &l_attr);
    attr->screen = to_ptrv(&screen32);
    convert_Screen_to_32(&screen32, l_attr.screen);
    return ret;
}

EXPORT int my32_XChangeProperty(x64emu_t* emu, void* dpy, XID window, XID prop, XID type, int fmt, int mode, void* data, int n)
{
    unsigned long data_l[n];
    if(fmt==32) {
        for(int i=0; i<n; ++i)
            data_l[i] = from_ulong(((ulong_t*)data)[i]);
        data = data_l;
    }
    return my->XChangeProperty(dpy, window, prop, type, fmt, mode, data, n);
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

#define CUSTOM_INIT                 \
    AddAutomaticBridge(lib->w.bridge, vFp_32, *(void**)dlsym(lib->w.lib, "_XLockMutex_fn"), 0, "_XLockMutex_fn"); \
    AddAutomaticBridge(lib->w.bridge, vFp_32, *(void**)dlsym(lib->w.lib, "_XUnlockMutex_fn"), 0, "_XUnlockMutex_fn"); \
    if(box64_x11threads) my->XInitThreads();
#if 0
#ifdef ANDROID
#define NEEDED_LIBS "libxcb.so"
#else
#define NEEDED_LIBS "libxcb.so.1"
#endif
#endif

#include "wrappedlib_init32.h"
