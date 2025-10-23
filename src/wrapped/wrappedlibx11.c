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

const char* libx11Name = "libX11.so.6";
#define ALTNAME "libX11.so"

#define LIBNAME libx11

typedef int (*XErrorHandler)(void *, void *);
void* my_XSetErrorHandler(x64emu_t* t, XErrorHandler handler);
typedef int (*XIOErrorHandler)(void *);
void* my_XSetIOErrorHandler(x64emu_t* t, XIOErrorHandler handler);
void* my_XESetCloseDisplay(x64emu_t* emu, void* display, int32_t extension, void* handler);
typedef int (*WireToEventProc)(void*, void*, void*);
typedef int(*EventHandler) (void*,void*,void*);
int32_t my_XIfEvent(x64emu_t* emu, void* d,void* ev, EventHandler h, void* arg);

typedef struct XImageSave_s {
    int   anyEmu;
    void* create;
    void* destroy;
    void* get;
    void* put;
    void* sub;
    void* add;
} XImageSave_t;

typedef struct ximage_s {
    void*(*create_image)(
            void*           /* display */,
            void*           /* visual */,
            uint32_t        /* depth */,
            int32_t         /* format */,
            int32_t         /* offset */,
            void*           /* data */,
            uint32_t        /* width */,
            uint32_t        /* height */,
            int32_t         /* bitmap_pad */,
            int32_t         /* bytes_per_line */);
    int32_t (*destroy_image)        (void*);
    uintptr_t (*get_pixel)           (void*, int32_t, int32_t);
    int32_t (*put_pixel)            (void*, int32_t, int32_t, uintptr_t);
    void*(*sub_image)    (void*, int32_t, int32_t, uint32_t, uint32_t); //sub_image return a new XImage that need bridging => custom wrapper
    int32_t (*add_pixel)            (void*, intptr_t);
} ximage_t;

typedef struct _XImage {
    int32_t width, height;          /* size of image */
    int32_t xoffset;                /* number of pixels offset in X direction */
    int32_t format;                 /* XYBitmap, XYPixmap, ZPixmap */
    void*   data;                   /* pointer to image data */
    int32_t byte_order;             /* data byte order, LSBFirst, MSBFirst */
    int32_t bitmap_unit;            /* quant. of scanline 8, 16, 32 */
    int32_t bitmap_bit_order;       /* LSBFirst, MSBFirst */
    int32_t bitmap_pad;             /* 8, 16, 32 either XY or ZPixmap */
    int32_t depth;                  /* depth of image */
    int32_t bytes_per_line;         /* accelarator to next line */
    int32_t bits_per_pixel;         /* bits per pixel (ZPixmap) */
    uintptr_t red_mask;             /* bits in z arrangment */
    uintptr_t green_mask;
    uintptr_t blue_mask;
    void*    obdata;                 /* hook for the object routines to hang on */
    ximage_t f;
} XImage;

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

#include "generated/wrappedlibx11types.h"

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
GO(13)  \
GO(14)  \
GO(15)

// wire_to_event
#define GO(A)   \
static uintptr_t my_wire_to_event_fct_##A = 0;                      \
static int my_wire_to_event_##A(void* dpy, void* re, void* event)   \
{                                                                   \
    return (int)RunFunctionFmt(my_wire_to_event_fct_##A, "ppp", dpy, re, event);\
}
SUPER()
#undef GO
static void* findwire_to_eventFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_wire_to_event_fct_##A == (uintptr_t)fct) return my_wire_to_event_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_wire_to_event_fct_##A == 0) {my_wire_to_event_fct_##A = (uintptr_t)fct; return my_wire_to_event_##A; }
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
    #define GO(A) if(my_wire_to_event_##A == fct) return (void*)my_wire_to_event_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(lib->w.bridge, iFppp, fct, 0, NULL);
}

// event_to_wire
#define GO(A)   \
static uintptr_t my_event_to_wire_fct_##A = 0;                      \
static int my_event_to_wire_##A(void* dpy, void* re, void* event)   \
{                                                                   \
    return (int)RunFunctionFmt(my_event_to_wire_fct_##A, "ppp", dpy, re, event);\
}
SUPER()
#undef GO
static void* findevent_to_wireFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_event_to_wire_fct_##A == (uintptr_t)fct) return my_event_to_wire_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_event_to_wire_fct_##A == 0) {my_event_to_wire_fct_##A = (uintptr_t)fct; return my_event_to_wire_##A; }
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
    #define GO(A) if(my_event_to_wire_##A == fct) return (void*)my_event_to_wire_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(lib->w.bridge, iFppp, fct, 0, NULL);
}

// error_handler
#define GO(A)   \
static uintptr_t my_error_handler_fct_##A = 0;                      \
static int my_error_handler_##A(void* dpy, void* error)   \
{                                                                   \
    return (int)RunFunctionFmt(my_error_handler_fct_##A, "pp", dpy, error);\
}
SUPER()
#undef GO
static void* finderror_handlerFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_error_handler_fct_##A == (uintptr_t)fct) return my_error_handler_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_error_handler_fct_##A == 0) {my_error_handler_fct_##A = (uintptr_t)fct; return my_error_handler_##A; }
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
    #define GO(A) if(my_error_handler_##A == fct) return (void*)my_error_handler_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(lib->w.bridge, iFpp, fct, 0, NULL);
}

// ioerror_handler
#define GO(A)   \
static uintptr_t my_ioerror_handler_fct_##A = 0;                      \
static int my_ioerror_handler_##A(void* dpy)   \
{                                                                   \
    return (int)RunFunctionFmt(my_ioerror_handler_fct_##A, "p", dpy);\
}
SUPER()
#undef GO
static void* findioerror_handlerFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_ioerror_handler_fct_##A == (uintptr_t)fct) return my_ioerror_handler_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_ioerror_handler_fct_##A == 0) {my_ioerror_handler_fct_##A = (uintptr_t)fct; return my_ioerror_handler_##A; }
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
    #define GO(A) if(my_ioerror_handler_##A == fct) return (void*)my_ioerror_handler_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(lib->w.bridge, iFp, fct, 0, NULL);
}

// exterror_handler
#define GO(A)   \
static uintptr_t my_exterror_handler_fct_##A = 0;                      \
static int my_exterror_handler_##A(void* dpy, void* err, void* codes, int* ret_code)   \
{                                                                   \
    return (int)RunFunctionFmt(my_exterror_handler_fct_##A, "pppp", dpy, err, codes, ret_code);\
}
SUPER()
#undef GO
static void* findexterror_handlerFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_exterror_handler_fct_##A == (uintptr_t)fct) return my_exterror_handler_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_exterror_handler_fct_##A == 0) {my_exterror_handler_fct_##A = (uintptr_t)fct; return my_exterror_handler_##A; }
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
    #define GO(A) if(my_exterror_handler_##A == fct) return (void*)my_exterror_handler_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(lib->w.bridge, iFpppp, fct, 0, NULL);
}

// close_display
#define GO(A)   \
static uintptr_t my_close_display_fct_##A = 0;                      \
static int my_close_display_##A(void* dpy, void* codes)   \
{                                                                   \
    return (int)RunFunctionFmt(my_close_display_fct_##A, "pp", dpy, codes);\
}
SUPER()
#undef GO
static void* findclose_displayFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_close_display_fct_##A == (uintptr_t)fct) return my_close_display_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_close_display_fct_##A == 0) {my_close_display_fct_##A = (uintptr_t)fct; return my_close_display_##A; }
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
    #define GO(A) if(my_close_display_##A == fct) return (void*)my_close_display_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(lib->w.bridge, iFpp, fct, 0, NULL);
}

// register_im
#define GO(A)   \
static uintptr_t my_register_im_fct_##A = 0;                        \
static void my_register_im_##A(void* dpy, void* u, void* d)         \
{                                                                   \
    RunFunctionFmt(my_register_im_fct_##A, "ppp", dpy, u, d);  \
}
SUPER()
#undef GO
static void* findregister_imFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_register_im_fct_##A == (uintptr_t)fct) return my_register_im_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_register_im_fct_##A == 0) {my_register_im_fct_##A = (uintptr_t)fct; return my_register_im_##A; }
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
    #define GO(A) if(my_register_im_##A == fct) return (void*)my_register_im_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(lib->w.bridge, iFppp, fct, 0, NULL);
}

// XConnectionWatchProc
#define GO(A)   \
static uintptr_t my_XConnectionWatchProc_fct_##A = 0;                               \
static void my_XConnectionWatchProc_##A(void* dpy, void* data, int op, void* d)     \
{                                                                                   \
    RunFunctionFmt(my_XConnectionWatchProc_fct_##A, "ppip", dpy, data, op, d);  \
}
SUPER()
#undef GO
static void* findXConnectionWatchProcFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_XConnectionWatchProc_fct_##A == (uintptr_t)fct) return my_XConnectionWatchProc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_XConnectionWatchProc_fct_##A == 0) {my_XConnectionWatchProc_fct_##A = (uintptr_t)fct; return my_XConnectionWatchProc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 XConnectionWatchProc callback\n");
    return NULL;
}
// xifevent
#define GO(A)   \
static uintptr_t my_xifevent_fct_##A = 0;                                   \
static int my_xifevent_##A(void* dpy, void* event, void* d)                 \
{                                                                           \
    return RunFunctionFmt(my_xifevent_fct_##A, "ppp", dpy, event, d);  \
}
SUPER()
#undef GO
static void* findxifeventFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xifevent_fct_##A == (uintptr_t)fct) return my_xifevent_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xifevent_fct_##A == 0) {my_xifevent_fct_##A = (uintptr_t)fct; return my_xifevent_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 xifevent callback\n");
    return NULL;
}
// XInternalAsyncHandler
#define GO(A)   \
static uintptr_t my_XInternalAsyncHandler_fct_##A = 0;                                              \
static int my_XInternalAsyncHandler_##A(void* dpy, void* rep, void* buf, int len, void* data)       \
{                                                                                                   \
    return RunFunctionFmt(my_XInternalAsyncHandler_fct_##A, "pppip", dpy, rep, buf, len, data);  \
}
SUPER()
#undef GO
static void* findXInternalAsyncHandlerFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_XInternalAsyncHandler_fct_##A == (uintptr_t)fct) return my_XInternalAsyncHandler_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_XInternalAsyncHandler_fct_##A == 0) {my_XInternalAsyncHandler_fct_##A = (uintptr_t)fct; return my_XInternalAsyncHandler_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 XInternalAsyncHandler callback\n");
    return NULL;
}

// XSynchronizeProc
#define GO(A)   \
static uintptr_t my_XSynchronizeProc_fct_##A = 0;                       \
static int my_XSynchronizeProc_##A()                                    \
{                                                                       \
    return (int)RunFunctionFmt(my_XSynchronizeProc_fct_##A, "");\
}
SUPER()
#undef GO
static void* findXSynchronizeProcFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_XSynchronizeProc_fct_##A == (uintptr_t)fct) return my_XSynchronizeProc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_XSynchronizeProc_fct_##A == 0) {my_XSynchronizeProc_fct_##A = (uintptr_t)fct; return my_XSynchronizeProc_##A; }
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
    #define GO(A) if(my_XSynchronizeProc_##A == fct) return (void*)my_XSynchronizeProc_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(lib->w.bridge, iFppp, fct, 0, NULL);
}

// XLockDisplay
#define GO(A)   \
static uintptr_t my_XLockDisplay_fct_##A = 0;                   \
static void my_XLockDisplay_##A(void* dpy)                      \
{                                                               \
    RunFunctionFmt(my_XLockDisplay_fct_##A, "p", dpy);   \
}
SUPER()
#undef GO
static void* findXLockDisplayFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_XLockDisplay_fct_##A == (uintptr_t)fct) return my_XLockDisplay_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_XLockDisplay_fct_##A == 0) {my_XLockDisplay_fct_##A = (uintptr_t)fct; return my_XLockDisplay_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 XLockDisplay callback\n");
    return NULL;
}
// XUnlockDisplay
#define GO(A)   \
static uintptr_t my_XUnlockDisplay_fct_##A = 0;                 \
static void my_XUnlockDisplay_##A(void* dpy)                    \
{                                                               \
    RunFunctionFmt(my_XUnlockDisplay_fct_##A, "p", dpy); \
}
SUPER()
#undef GO
static void* findXUnlockDisplayFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_XUnlockDisplay_fct_##A == (uintptr_t)fct) return my_XUnlockDisplay_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_XUnlockDisplay_fct_##A == 0) {my_XUnlockDisplay_fct_##A = (uintptr_t)fct; return my_XUnlockDisplay_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 XUnlockDisplay callback\n");
    return NULL;
}
// XImage function wrappers
// create_image
#define GO(A)   \
static uintptr_t my_create_image_fct_##A = 0;                                                                                   \
static void* my_create_image_##A(void* a, void* b, uint32_t c, int d, int e, void* f, uint32_t g, uint32_t h, int i, int j)     \
{                                                                                                                               \
    return (void*)RunFunctionFmt(my_create_image_fct_##A, "ppuiipuuii", a, b, c, d, e, f, g, h, i, j);                          \
}
SUPER()
#undef GO
static void* find_create_image_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_create_image_fct_##A == (uintptr_t)fct) return my_create_image_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_create_image_fct_##A == 0) {my_create_image_fct_##A = (uintptr_t)fct; return my_create_image_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 create_image callback\n");
    return NULL;
}
static void* reverse_create_image_Fct(library_t* lib, void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(lib->w.bridge, fct))
        return (void*)CheckBridged(lib->w.bridge, fct);
    #define GO(A) if(my_create_image_##A == fct) return (void*)my_create_image_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddAutomaticBridge(lib->w.bridge, pFppuiipuuii, fct, 0, NULL);
}
// destroy_image
#define GO(A)   \
static uintptr_t my_destroy_image_fct_##A = 0;                      \
static int my_destroy_image_##A(void* a)                            \
{                                                                   \
    return (int)RunFunctionFmt(my_destroy_image_fct_##A, "p", a);   \
}
SUPER()
#undef GO
static void* find_destroy_image_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_destroy_image_fct_##A == (uintptr_t)fct) return my_destroy_image_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_destroy_image_fct_##A == 0) {my_destroy_image_fct_##A = (uintptr_t)fct; return my_destroy_image_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 destroy_image callback\n");
    return NULL;
}
static void* reverse_destroy_image_Fct(library_t* lib, void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(lib->w.bridge, fct))
        return (void*)CheckBridged(lib->w.bridge, fct);
    #define GO(A) if(my_destroy_image_##A == fct) return (void*)my_destroy_image_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddAutomaticBridge(lib->w.bridge, iFp, fct, 0, NULL);
}
// get_pixel
#define GO(A)   \
static uintptr_t my_get_pixel_fct_##A = 0;                                      \
static unsigned long my_get_pixel_##A(void* a, int b, int c)                    \
{                                                                               \
    return (unsigned long)RunFunctionFmt(my_get_pixel_fct_##A, "pii", a, b, c); \
}
SUPER()
#undef GO
static void* find_get_pixel_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_get_pixel_fct_##A == (uintptr_t)fct) return my_get_pixel_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_get_pixel_fct_##A == 0) {my_get_pixel_fct_##A = (uintptr_t)fct; return my_get_pixel_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 get_pixel callback\n");
    return NULL;
}
static void* reverse_get_pixel_Fct(library_t* lib, void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(lib->w.bridge, fct))
        return (void*)CheckBridged(lib->w.bridge, fct);
    #define GO(A) if(my_get_pixel_##A == fct) return (void*)my_get_pixel_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddAutomaticBridge(lib->w.bridge, LFpii, fct, 0, NULL);
}
// put_pixel
#define GO(A)   \
static uintptr_t my_put_pixel_fct_##A = 0;                                  \
static int my_put_pixel_##A(void* a, int b, int c,unsigned long d)          \
{                                                                           \
    return (int)RunFunctionFmt(my_put_pixel_fct_##A, "piiL", a, b, c, d);   \
}
SUPER()
#undef GO
static void* find_put_pixel_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_put_pixel_fct_##A == (uintptr_t)fct) return my_put_pixel_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_put_pixel_fct_##A == 0) {my_put_pixel_fct_##A = (uintptr_t)fct; return my_put_pixel_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 put_pixel callback\n");
    return NULL;
}
static void* reverse_put_pixel_Fct(library_t* lib, void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(lib->w.bridge, fct))
        return (void*)CheckBridged(lib->w.bridge, fct);
    #define GO(A) if(my_put_pixel_##A == fct) return (void*)my_put_pixel_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddAutomaticBridge(lib->w.bridge, iFpiiL, fct, 0, NULL);
}
// sub_image
#define GO(A)   \
static uintptr_t my_sub_image_fct_##A = 0;                                      \
static void* my_sub_image_##A(void* a, int b, int c, uint32_t d, uint32_t e)    \
{                                                                               \
    return (void*)RunFunctionFmt(my_sub_image_fct_##A, "piiuu", a, b, c, d, e); \
}
SUPER()
#undef GO
static void* find_sub_image_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_sub_image_fct_##A == (uintptr_t)fct) return my_sub_image_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_sub_image_fct_##A == 0) {my_sub_image_fct_##A = (uintptr_t)fct; return my_sub_image_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 sub_image callback\n");
    return NULL;
}
static void* reverse_sub_image_Fct(library_t* lib, void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(lib->w.bridge, fct))
        return (void*)CheckBridged(lib->w.bridge, fct);
    #define GO(A) if(my_sub_image_##A == fct) return (void*)my_sub_image_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddAutomaticBridge(lib->w.bridge, pFpiiuu, fct, 0, NULL);
}
// add_pixel
#define GO(A)   \
static uintptr_t my_add_pixel_fct_##A = 0;                          \
static int my_add_pixel_##A(void* a, long b)                        \
{                                                                   \
    return (int)RunFunctionFmt(my_add_pixel_fct_##A, "pl", a, b);   \
}
SUPER()
#undef GO
static void* find_add_pixel_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_add_pixel_fct_##A == (uintptr_t)fct) return my_add_pixel_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_add_pixel_fct_##A == 0) {my_add_pixel_fct_##A = (uintptr_t)fct; return my_add_pixel_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 add_pixel callback\n");
    return NULL;
}
static void* reverse_add_pixel_Fct(library_t* lib, void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(lib->w.bridge, fct))
        return (void*)CheckBridged(lib->w.bridge, fct);
    #define GO(A) if(my_add_pixel_##A == fct) return (void*)my_add_pixel_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddAutomaticBridge(lib->w.bridge, iFpl, fct, 0, NULL);
}
// end of XImage functions callbacks
// async_handler
#define GO(A)   \
static uintptr_t my_async_handler_fct_##A = 0;                                      \
static int my_async_handler_##A(void* a, void* b, void* c, int d, void* e)          \
{                                                                                   \
    return (int)RunFunctionFmt(my_async_handler_fct_##A, "pppip", a, b, c, d, e);   \
}
SUPER()
#undef GO
static void* find_async_handler_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_async_handler_fct_##A == (uintptr_t)fct) return my_async_handler_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_async_handler_fct_##A == 0) {my_async_handler_fct_##A = (uintptr_t)fct; return my_async_handler_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 async_handler callback\n");
    return NULL;
}

// ResourceAlloc
typedef unsigned long XID;
#define GO(A)   \
static uintptr_t my_ResourceAlloc_fct_##A = 0;                      \
static XID my_ResourceAlloc_##A(void* dpy)                          \
{                                                                   \
    return (XID)RunFunctionFmt(my_ResourceAlloc_fct_##A, "p", dpy); \
}
SUPER()
#undef GO
static void* findResourceAllocFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_ResourceAlloc_fct_##A == (uintptr_t)fct) return my_ResourceAlloc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_ResourceAlloc_fct_##A == 0) {my_ResourceAlloc_fct_##A = (uintptr_t)fct; return my_ResourceAlloc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 ResourceAlloc callback\n");
    return NULL;
}

#undef SUPER

void* my_XCreateImage(x64emu_t* emu, void* disp, void* vis, uint32_t depth, int32_t fmt, int32_t off
                    , void* data, uint32_t w, uint32_t h, int32_t pad, int32_t bpl);

int32_t my_XInitImage(x64emu_t* emu, void* img);

void* my_XGetImage(x64emu_t* emu, void* disp, size_t drawable, int32_t x, int32_t y
                    , uint32_t w, uint32_t h, uint32_t plane, int32_t fmt);

int32_t my_XPutImage(x64emu_t* emu, void* disp, size_t drawable, void* gc, void* image
                    , int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y
                    , uint32_t w, uint32_t h);

void* my_XGetSubImage(x64emu_t* emu, void* disp, size_t drawable
                    , int32_t x, int32_t y
                    , uint32_t w, uint32_t h, size_t plane, int32_t fmt
                    , void* image, int32_t dst_x, int32_t dst_y);

void my_XDestroyImage(x64emu_t* emu, void* image);

#ifdef PANDORA
void* my_XLoadQueryFont(x64emu_t* emu, void* d, void* name);
#endif

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
static uintptr_t my_XNGeometryCallback_fct_##A = 0;                 \
static void my_XNGeometryCallback_##A(void* a, void* b, void* c)    \
{                                                                   \
    RunFunctionFmt(my_XNGeometryCallback_fct_##A, "ppp", a, b);     \
}
SUPER()
#undef GO
static void* findXNGeometryCallbackFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_XNGeometryCallback_fct_##A == (uintptr_t)fct) return my_XNGeometryCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_XNGeometryCallback_fct_##A == 0) {my_XNGeometryCallback_fct_##A = (uintptr_t)fct; return my_XNGeometryCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 XNGeometryCallback callback\n");
    return NULL;
}
// XNDestroyCallback
#define GO(A)   \
static uintptr_t my_XNDestroyCallback_fct_##A = 0;              \
static void my_XNDestroyCallback_##A(void* a, void* b, void* c) \
{                                                               \
    RunFunctionFmt(my_XNDestroyCallback_fct_##A, "ppp", a, b);  \
}
SUPER()
#undef GO
static void* findXNDestroyCallbackFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_XNDestroyCallback_fct_##A == (uintptr_t)fct) return my_XNDestroyCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_XNDestroyCallback_fct_##A == 0) {my_XNDestroyCallback_fct_##A = (uintptr_t)fct; return my_XNDestroyCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 XNDestroyCallback callback\n");
    return NULL;
}
// XNPreeditStartCallback
#define GO(A)   \
static uintptr_t my_XNPreeditStartCallback_fct_##A = 0;                 \
static void my_XNPreeditStartCallback_##A(void* a, void* b, void* c)    \
{                                                                       \
    RunFunctionFmt(my_XNPreeditStartCallback_fct_##A, "ppp", a, b);     \
}
SUPER()
#undef GO
static void* findXNPreeditStartCallbackFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_XNPreeditStartCallback_fct_##A == (uintptr_t)fct) return my_XNPreeditStartCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_XNPreeditStartCallback_fct_##A == 0) {my_XNPreeditStartCallback_fct_##A = (uintptr_t)fct; return my_XNPreeditStartCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 XNPreeditStartCallback callback\n");
    return NULL;
}
// XNPreeditDoneCallback
#define GO(A)   \
static uintptr_t my_XNPreeditDoneCallback_fct_##A = 0;              \
static void my_XNPreeditDoneCallback_##A(void* a, void* b, void* c) \
{                                                                   \
    RunFunctionFmt(my_XNPreeditDoneCallback_fct_##A, "ppp", a, b);  \
}
SUPER()
#undef GO
static void* findXNPreeditDoneCallbackFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_XNPreeditDoneCallback_fct_##A == (uintptr_t)fct) return my_XNPreeditDoneCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_XNPreeditDoneCallback_fct_##A == 0) {my_XNPreeditDoneCallback_fct_##A = (uintptr_t)fct; return my_XNPreeditDoneCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 XNPreeditDoneCallback callback\n");
    return NULL;
}
// XNPreeditDrawCallback
#define GO(A)   \
static uintptr_t my_XNPreeditDrawCallback_fct_##A = 0;              \
static void my_XNPreeditDrawCallback_##A(void* a, void* b, void* c) \
{                                                                   \
    RunFunctionFmt(my_XNPreeditDrawCallback_fct_##A, "ppp", a, b);  \
}
SUPER()
#undef GO
static void* findXNPreeditDrawCallbackFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_XNPreeditDrawCallback_fct_##A == (uintptr_t)fct) return my_XNPreeditDrawCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_XNPreeditDrawCallback_fct_##A == 0) {my_XNPreeditDrawCallback_fct_##A = (uintptr_t)fct; return my_XNPreeditDrawCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 XNPreeditDrawCallback callback\n");
    return NULL;
}
// XNPreeditCaretCallback
#define GO(A)   \
static uintptr_t my_XNPreeditCaretCallback_fct_##A = 0;                 \
static void my_XNPreeditCaretCallback_##A(void* a, void* b, void* c)    \
{                                                                       \
    RunFunctionFmt(my_XNPreeditCaretCallback_fct_##A, "ppp", a, b);     \
}
SUPER()
#undef GO
static void* findXNPreeditCaretCallbackFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_XNPreeditCaretCallback_fct_##A == (uintptr_t)fct) return my_XNPreeditCaretCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_XNPreeditCaretCallback_fct_##A == 0) {my_XNPreeditCaretCallback_fct_##A = (uintptr_t)fct; return my_XNPreeditCaretCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 XNPreeditCaretCallback callback\n");
    return NULL;
}
// XNPreeditStateNotifyCallback
#define GO(A)   \
static uintptr_t my_XNPreeditStateNotifyCallback_fct_##A = 0;               \
static void my_XNPreeditStateNotifyCallback_##A(void* a, void* b, void* c)  \
{                                                                           \
    RunFunctionFmt(my_XNPreeditStateNotifyCallback_fct_##A, "ppp", a, b);   \
}
SUPER()
#undef GO
static void* findXNPreeditStateNotifyCallbackFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_XNPreeditStateNotifyCallback_fct_##A == (uintptr_t)fct) return my_XNPreeditStateNotifyCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_XNPreeditStateNotifyCallback_fct_##A == 0) {my_XNPreeditStateNotifyCallback_fct_##A = (uintptr_t)fct; return my_XNPreeditStateNotifyCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 XNPreeditStateNotifyCallback callback\n");
    return NULL;
}
// XNStatusStartCallback
#define GO(A)   \
static uintptr_t my_XNStatusStartCallback_fct_##A = 0;                  \
static void my_XNStatusStartCallback_##A(void* a, void* b, void* c)     \
{                                                                       \
    RunFunctionFmt(my_XNStatusStartCallback_fct_##A, "ppp", a, b);      \
}
SUPER()
#undef GO
static void* findXNStatusStartCallbackFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_XNStatusStartCallback_fct_##A == (uintptr_t)fct) return my_XNStatusStartCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_XNStatusStartCallback_fct_##A == 0) {my_XNStatusStartCallback_fct_##A = (uintptr_t)fct; return my_XNStatusStartCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 XNStatusStartCallback callback\n");
    return NULL;
}
// XNStatusDoneCallback
#define GO(A)   \
static uintptr_t my_XNStatusDoneCallback_fct_##A = 0;               \
static void my_XNStatusDoneCallback_##A(void* a, void* b, void* c)  \
{                                                                   \
    RunFunctionFmt(my_XNStatusDoneCallback_fct_##A, "ppp", a, b);   \
}
SUPER()
#undef GO
static void* findXNStatusDoneCallbackFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_XNStatusDoneCallback_fct_##A == (uintptr_t)fct) return my_XNStatusDoneCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_XNStatusDoneCallback_fct_##A == 0) {my_XNStatusDoneCallback_fct_##A = (uintptr_t)fct; return my_XNStatusDoneCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 XNStatusDoneCallback callback\n");
    return NULL;
}
// XNStatusDrawCallback
#define GO(A)   \
static uintptr_t my_XNStatusDrawCallback_fct_##A = 0;               \
static void my_XNStatusDrawCallback_##A(void* a, void* b, void* c)  \
{                                                                   \
    RunFunctionFmt(my_XNStatusDrawCallback_fct_##A, "ppp", a, b);   \
}
SUPER()
#undef GO
static void* findXNStatusDrawCallbackFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_XNStatusDrawCallback_fct_##A == (uintptr_t)fct) return my_XNStatusDrawCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_XNStatusDrawCallback_fct_##A == 0) {my_XNStatusDrawCallback_fct_##A = (uintptr_t)fct; return my_XNStatusDrawCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 XNStatusDrawCallback callback\n");
    return NULL;
}
// XNR6PreeditCallback
#define GO(A)   \
static uintptr_t my_XNR6PreeditCallback_fct_##A = 0;                \
static void my_XNR6PreeditCallback_##A(void* a, void* b, void* c)   \
{                                                                   \
    RunFunctionFmt(my_XNR6PreeditCallback_fct_##A, "ppp", a, b);    \
}
SUPER()
#undef GO
static void* findXNR6PreeditCallbackFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_XNR6PreeditCallback_fct_##A == (uintptr_t)fct) return my_XNR6PreeditCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_XNR6PreeditCallback_fct_##A == 0) {my_XNR6PreeditCallback_fct_##A = (uintptr_t)fct; return my_XNR6PreeditCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 XNR6PreeditCallback callback\n");
    return NULL;
}
// XNStringConversionCallback
#define GO(A)   \
static uintptr_t my_XNStringConversionCallback_fct_##A = 0;                 \
static void my_XNStringConversionCallback_##A(void* a, void* b, void* c)    \
{                                                                           \
    RunFunctionFmt(my_XNStringConversionCallback_fct_##A, "ppp", a, b);     \
}
SUPER()
#undef GO
static void* findXNStringConversionCallbackFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_XNStringConversionCallback_fct_##A == (uintptr_t)fct) return my_XNStringConversionCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_XNStringConversionCallback_fct_##A == 0) {my_XNStringConversionCallback_fct_##A = (uintptr_t)fct; return my_XNStringConversionCallback_##A; }
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
    RESULT = FUNC(FIRST_ARG, getVArgs(emu, N, va, 0), getVArgs(emu, N, va, 1), NULL);       \
    break;                                                      \
case 4:                                                         \
    RESULT = FUNC(FIRST_ARG, getVArgs(emu, N, va, 0), getVArgs(emu, N, va, 1), getVArgs(emu, N, va, 2), getVArgs(emu, N, va, 3), NULL);     \
    break;                                                                          \
case 6:                                                                             \
    RESULT = FUNC(FIRST_ARG, getVArgs(emu, N, va, 0), getVArgs(emu, N, va, 1), getVArgs(emu, N, va, 2), getVArgs(emu, N, va, 3), getVArgs(emu, N, va, 4), getVArgs(emu, N, va, 5), NULL);   \
    break;                                                                                              \
case 8:                                                                                                 \
    RESULT = FUNC(FIRST_ARG, getVArgs(emu, N, va, 0), getVArgs(emu, N, va, 1), getVArgs(emu, N, va, 2), getVArgs(emu, N, va, 3), getVArgs(emu, N, va, 4), getVArgs(emu, N, va, 5), getVArgs(emu, N, va, 6), getVArgs(emu, N, va, 7), NULL); \
    break;                                                                                                                  \
case 10:                                                                                                                    \
    RESULT = FUNC(FIRST_ARG, getVArgs(emu, N, va, 0), getVArgs(emu, N, va, 1), getVArgs(emu, N, va, 2), getVArgs(emu, N, va, 3), getVArgs(emu, N, va, 4), getVArgs(emu, N, va, 5), getVArgs(emu, N, va, 6), getVArgs(emu, N, va, 7), getVArgs(emu, N, va, 8), getVArgs(emu, N, va, 9), NULL);   \
    break;                                                                                                                                          \
case 12:                                                                                                                                            \
    RESULT = FUNC(FIRST_ARG, getVArgs(emu, N, va, 0), getVArgs(emu, N, va, 1), getVArgs(emu, N, va, 2), getVArgs(emu, N, va, 3), getVArgs(emu, N, va, 4), getVArgs(emu, N, va, 5), getVArgs(emu, N, va, 6), getVArgs(emu, N, va, 7), getVArgs(emu, N, va, 8), getVArgs(emu, N, va, 9),  getVArgs(emu, N, va, 10), getVArgs(emu, N, va, 11), NULL);  \
    break;                                                                                                                                                                  \
case 14:                                                                                                                                                                    \
    RESULT = FUNC(FIRST_ARG, getVArgs(emu, N, va, 0), getVArgs(emu, N, va, 1), getVArgs(emu, N, va, 2), getVArgs(emu, N, va, 3), getVArgs(emu, N, va, 4), getVArgs(emu, N, va, 5), getVArgs(emu, N, va, 6), getVArgs(emu, N, va, 7), getVArgs(emu, N, va, 8), getVArgs(emu, N, va, 9),  getVArgs(emu, N, va, 10), getVArgs(emu, N, va, 11), getVArgs(emu, N, va, 12), getVArgs(emu, N, va, 13), NULL);  \
    break;                                                                                                                                                                                          \
case 16:                                                                                                                                                                                            \
    RESULT = FUNC(FIRST_ARG, getVArgs(emu, N, va, 0), getVArgs(emu, N, va, 1), getVArgs(emu, N, va, 2), getVArgs(emu, N, va, 3), getVArgs(emu, N, va, 4), getVArgs(emu, N, va, 5), getVArgs(emu, N, va, 6), getVArgs(emu, N, va, 7), getVArgs(emu, N, va, 8), getVArgs(emu, N, va, 9),  getVArgs(emu, N, va, 10), getVArgs(emu, N, va, 11), getVArgs(emu, N, va, 12), getVArgs(emu, N, va, 13), getVArgs(emu, N, va, 14), getVArgs(emu, N, va, 15), NULL);  \
    break;                                                                                                                                                                                                                  \
case 18:                                                                                                                                                                                                                    \
    RESULT = FUNC(FIRST_ARG, getVArgs(emu, N, va, 0), getVArgs(emu, N, va, 1), getVArgs(emu, N, va, 2), getVArgs(emu, N, va, 3), getVArgs(emu, N, va, 4), getVArgs(emu, N, va, 5), getVArgs(emu, N, va, 6), getVArgs(emu, N, va, 7), getVArgs(emu, N, va, 8), getVArgs(emu, N, va, 9),  getVArgs(emu, N, va, 10), getVArgs(emu, N, va, 11), getVArgs(emu, N, va, 12), getVArgs(emu, N, va, 13), getVArgs(emu, N, va, 14), getVArgs(emu, N, va, 15), getVArgs(emu, N, va, 16), getVArgs(emu, N, va, 17), NULL);  \
    break;                                                                                                                                                                                                                                          \
case 20:                                                                                                                                                                                                                                            \
    RESULT = FUNC(FIRST_ARG, getVArgs(emu, N, va, 0), getVArgs(emu, N, va, 1), getVArgs(emu, N, va, 2), getVArgs(emu, N, va, 3), getVArgs(emu, N, va, 4), getVArgs(emu, N, va, 5), getVArgs(emu, N, va, 6), getVArgs(emu, N, va, 7), getVArgs(emu, N, va, 8), getVArgs(emu, N, va, 9),  getVArgs(emu, N, va, 10), getVArgs(emu, N, va, 11), getVArgs(emu, N, va, 12), getVArgs(emu, N, va, 13), getVArgs(emu, N, va, 14), getVArgs(emu, N, va, 15), getVArgs(emu, N, va, 16), getVArgs(emu, N, va, 17), getVArgs(emu, N, va, 18), getVArgs(emu, N, va, 19), NULL);  \
    break;                                                                                                                                                                                                                                                                  \
default:                                                                                                                \
    printf_log(LOG_NONE, "warning: %s's vasize (%d) is too large, need create new call case!\n", __func__, VAARGSZ);    \
    break;                                                                                                              \
}

#define GO(A)                                                                       \
if (getVArgs(emu, 1, va, i) && strcmp((char*)getVArgs(emu, 1, va, i), A) == 0) {    \
    XICCallback* origin = (XICCallback*)getVArgs(emu, 1, va, i+1);                  \
    setVArgs(emu, 1, va, i+1, (uintptr_t)find##A##Fct(origin));                     \
}

EXPORT void* my_XVaCreateNestedList(x64emu_t* emu, int unused, uintptr_t* va) {
    int n = 0;
    while (getVArgs(emu, 1, va, n)) n+=2 ;

    for (int i = 0; i < n; i += 2) {
        SUPER()
    }

    void* res = NULL;
    VA_CALL(my->XVaCreateNestedList, unused, 1, n, res);
    return res;
}

EXPORT void* my_XCreateIC(x64emu_t* emu, void* xim, uintptr_t* va) {
    int n = 0;
    while (getVArgs(emu, 1, va, n)) n+=2;

    for (int i = 0; i < n; i += 2) {
        SUPER()
    }

    void* res = NULL;
    VA_CALL(my->XCreateIC, xim, 1, n, res);
    return res;
}

EXPORT void* my_XSetICValues(x64emu_t* emu, void* xic, uintptr_t* va) {
    int n = 0;
    while (getVArgs(emu, 1, va, n)) n+=2;

    for (int i = 0; i < n; i += 2) {
        SUPER()
    }

    void* res = NULL;
    VA_CALL(my->XSetICValues, xic, 1, n, res);
    return res;
}
#undef GO

EXPORT void* my_XSetIMValues(x64emu_t* emu, void* xim, uintptr_t* va) {
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
#undef VA_CALL
#undef SUPER

EXPORT void* my_XSetErrorHandler(x64emu_t* emu, XErrorHandler handler)
{
    void* ret = my->XSetErrorHandler(finderror_handlerFct(handler));
    return reverse_error_handlerFct(my_lib, ret);
}

EXPORT void* my_XSetIOErrorHandler(x64emu_t* emu, XIOErrorHandler handler)
{
    void* ret = my->XSetIOErrorHandler(findioerror_handlerFct(handler));
    return reverse_ioerror_handlerFct(my_lib, ret);
}

EXPORT void* my_XESetError(x64emu_t* emu, void* display, int32_t extension, void* handler)
{
    void* ret = my->XESetError(display, extension, findexterror_handlerFct(handler));
    return reverse_exterror_handlerFct(my_lib, ret);
}

EXPORT void* my_XESetCloseDisplay(x64emu_t* emu, void* display, int32_t extension, void* handler)
{
    void* ret = my->XESetCloseDisplay(display, extension, findclose_displayFct(handler));
    return reverse_close_displayFct(my_lib, ret);
}

EXPORT int32_t my_XIfEvent(x64emu_t* emu, void* d,void* ev, EventHandler h, void* arg)
{
    int32_t ret = my->XIfEvent(d, ev, findxifeventFct(h), arg);
    return ret;
}

EXPORT int32_t my_XCheckIfEvent(x64emu_t* emu, void* d,void* ev, EventHandler h, void* arg)
{
    int32_t ret = my->XCheckIfEvent(d, ev, findxifeventFct(h), arg);
    return ret;
}

EXPORT int32_t my_XPeekIfEvent(x64emu_t* emu, void* d,void* ev, EventHandler h, void* arg)
{
    int32_t ret = my->XPeekIfEvent(d, ev, findxifeventFct(h), arg);
    return ret;
}

void sub_image_wrapper(x64emu_t *emu, uintptr_t fnc);
typedef void* (*sub_image_wrapper_t)(void*, int32_t, int32_t, uint32_t, uint32_t);


void BridgeImageFunc(x64emu_t *emu, XImage *img)
{
    bridge_t* system = emu->context->system;

    #define GO(A, W) \
    img->f.A = (W##_t)reverse_##A##_Fct(my_lib, img->f.A);

    GO(create_image, pFppuiipuuii)
    GO(destroy_image, iFp)
    GO(get_pixel, LFpii)
    GO(put_pixel, iFpiiL)
    GO(sub_image, pFpiiuu)
    GO(add_pixel, iFpl)
    #undef GO
}

void UnbridgeImageFunc(x64emu_t *emu, XImage *img)
{
    (void)emu;
    #define GO(A, W)                            \
    img->f.A = (W##_t)find_##A##_Fct(img->f.A);

    GO(create_image, pFppuiipuuii)
    GO(destroy_image, iFp)
    GO(get_pixel, LFpii)
    GO(put_pixel, iFpiiL)
    GO(sub_image, pFpiiuu)
    GO(add_pixel, iFpl)
    #undef GO
}

void sub_image_wrapper(x64emu_t *emu, uintptr_t fnc)
{
    pFpiiuu_t fn = (pFpiiuu_t)fnc;
    void* img = fn(*(void**)(R_RDI), *(int32_t*)(R_RSI), *(int32_t*)(R_RDX), *(uint32_t*)(R_RCX), *(uint32_t*)(R_R8));
    BridgeImageFunc(emu, (XImage*)img);
    R_EAX=(uintptr_t)img;
}


EXPORT void* my_XCreateImage(x64emu_t* emu, void* disp, void* vis, uint32_t depth, int32_t fmt, int32_t off
                    , void* data, uint32_t w, uint32_t h, int32_t pad, int32_t bpl)
{

    XImage *img = my->XCreateImage(disp, vis, depth, fmt, off, data, w, h, pad, bpl);
    if(!img)
        return img;
    // bridge all access functions...
    BridgeImageFunc(emu, img);
    return img;
}

EXPORT int32_t my_XInitImage(x64emu_t* emu, void* img)
{

    int ret = my->XInitImage(img);
    // bridge all access functions...
    BridgeImageFunc(emu, img);
    return ret;
}

EXPORT void* my_XGetImage(x64emu_t* emu, void* disp, size_t drawable, int32_t x, int32_t y
                    , uint32_t w, uint32_t h, uint32_t plane, int32_t fmt)
{

    XImage *img = my->XGetImage(disp, drawable, x, y, w, h, plane, fmt);
    if(!img)
        return img;
    // bridge all access functions...
    BridgeImageFunc(emu, img);
    return img;
}

EXPORT void my__XInitImageFuncPtrs(x64emu_t* emu, XImage* img)
{
    my->_XInitImageFuncPtrs(img);
    BridgeImageFunc(emu, img);
}

EXPORT int32_t my_XPutImage(x64emu_t* emu, void* disp, size_t drawable, void* gc, void* image
                    , int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y
                    , uint32_t w, uint32_t h)
{
    UnbridgeImageFunc(emu, (XImage*)image);
    int32_t r = my->XPutImage(disp, drawable, gc, image, src_x, src_y, dst_x, dst_y, w, h);
    // bridge all access functions...
    BridgeImageFunc(emu, (XImage*)image);
    return r;
}

EXPORT void* my_XGetSubImage(x64emu_t* emu, void* disp, size_t drawable
                    , int32_t x, int32_t y
                    , uint32_t w, uint32_t h, size_t plane, int32_t fmt
                    , void* image, int32_t dst_x, int32_t dst_y)
{

    UnbridgeImageFunc(emu, (XImage*)image);
    XImage *img = my->XGetSubImage(disp, drawable, x, y, w, h, plane, fmt, image, dst_x, dst_y);
    if(img && img!=image)
        BridgeImageFunc(emu, img);

    BridgeImageFunc(emu, (XImage*)image);
    return img;
}

EXPORT void my_XDestroyImage(x64emu_t* emu, void* image)
{

    UnbridgeImageFunc(emu, (XImage*)image);
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

EXPORT void my__XDeqAsyncHandler(x64emu_t* emu, void* cb, void* data)
{
    my->_XDeqAsyncHandler(findXInternalAsyncHandlerFct(cb), data);
}

EXPORT void* my_XESetWireToEvent(x64emu_t* emu, void* display, int32_t event_number, void* proc)
{
    void* ret = NULL;

    ret = my->XESetWireToEvent(display, event_number, findwire_to_eventFct(proc));

    return reverse_wire_to_eventFct(my_lib, ret);
}
EXPORT void* my_XESetEventToWire(x64emu_t* emu, void* display, int32_t event_number, void* proc)
{
    void* ret = NULL;

    ret = my->XESetEventToWire(display, event_number, findevent_to_wireFct(proc));

    return reverse_event_to_wireFct(my_lib, ret);
}

EXPORT int my_XRegisterIMInstantiateCallback(x64emu_t* emu, void* d, void* db, void* res_name, void* res_class, void* cb, void* data)
{
    return my->XRegisterIMInstantiateCallback(d, db, res_name, res_class, findregister_imFct(cb), data);
}

EXPORT int my_XUnregisterIMInstantiateCallback(x64emu_t* emu, void* d, void* db, void* res_name, void* res_class, void* cb, void* data)
{
    return my->XUnregisterIMInstantiateCallback(d, db, res_name, res_class, reverse_register_imFct(my_lib, cb), data);
}

EXPORT int my_XQueryExtension(x64emu_t* emu, void* display, char* name, int* major, int* first_event, int* first_error)
{
    int ret = my->XQueryExtension(display, name, major, first_event, first_error);
    if(!ret && name && !strcmp(name, "GLX") && BOX64ENV(x11glx)) {
        // hack to force GLX to be accepted, even if not present
        // left major and first_XXX to default...
        ret = 1;
    }
    return ret;
}

EXPORT int my_XAddConnectionWatch(x64emu_t* emu, void* display, char* f, void* data)
{
    return my->XAddConnectionWatch(display, findXConnectionWatchProcFct(f), data);
}

EXPORT void my_XRemoveConnectionWatch(x64emu_t* emu, void* display, char* f, void* data)
{
    my->XRemoveConnectionWatch(display, findXConnectionWatchProcFct(f), data);
}

EXPORT void* my_XSetAfterFunction(x64emu_t* emu, void* display, void* f)
{

    return reverse_XSynchronizeProcFct(my_lib, my->XSetAfterFunction(display, findXSynchronizeProcFct(f)));
}

EXPORT void* my_XSynchronize(x64emu_t* emu, void* display, int onoff)
{
    return reverse_XSynchronizeProcFct(my_lib, my->XSynchronize(display, onoff));
}

struct my_XFreeFuncs {
    void* atoms;
    void* modifiermap;
    void* key_bindings;
    void* context_db;
    void* defaultCCCs;
    void* clientCmaps;
    void* intensityMaps;
    void* im_filters;
    void* xkb;
};

struct my_XExten {
        struct my_XExten *next;
        void* codes;    // XExtCodes
        void* create_GC;    // CreateGCType
        void* copy_GC;  // CopyGCType
        void* flush_GC; // FlushGCType
        void* free_GC;  // FreeGCType
        void* create_Font;  // CreateFontType
        void* free_Font;    // FreeFontType
        void* close_display;    // CloseDisplayType
        void* error;    // ErrorType
        void* error_string; // ErrorStringType
        char *name;
        void* error_values; // PrintErrorType
        void* before_flush; // BeforeFlushType
        struct my_XExten *next_flush;
};

struct my_XInternalAsync {
    struct my_XInternalAsync *next;
    int (*handler)(void*, void*, char*, int, void*);
    void* data;
};

struct my_XLockPtrs {
    void (*lock_display)(void* dpy);
    void (*unlock_display)(void *dpy);
};

struct my_XConnectionInfo {
    int fd;
    void* read_callback;    // _XInternalConnectionProc
    void* call_data;
    void* *watch_data;
    struct my_XConnectionInfo *next;
};

struct my_XConnWatchInfo {
    void* fn;   // XConnectionWatchProc
    void* client_data;
    struct _XConnWatchInfo *next;
};

typedef struct my_XDisplay_s
{
        void *ext_data;
        struct my_XFreeFuncs *free_funcs;
        int fd;
        int conn_checker;
        int proto_major_version;
        int proto_minor_version;
        char *vendor;
        XID resource_base;
        XID resource_mask;
        XID resource_id;
        int resource_shift;
        XID (*resource_alloc)(void*);
        int byte_order;
        int bitmap_unit;
        int bitmap_pad;
        int bitmap_bit_order;
        int nformats;
        void *pixmap_format;
        int vnumber;
        int release;
        void *head, *tail;
        int qlen;
        unsigned long last_request_read;
        unsigned long request;
        char *last_req;
        char *buffer;
        char *bufptr;
        char *bufmax;
        unsigned max_request_size;
        void* *db;
        int (*synchandler)(void*);
        char *display_name;
        int default_screen;
        int nscreens;
        void *screens;
        unsigned long motion_buffer;
        volatile unsigned long flags;
        int min_keycode;
        int max_keycode;
        void *keysyms;
        void *modifiermap;
        int keysyms_per_keycode;
        char *xdefaults;
        char *scratch_buffer;
        unsigned long scratch_length;
        int ext_number;
        struct my_XExten *ext_procs;
        int (*event_vec[128])(void *, void *, void *);
        int (*wire_vec[128])(void *, void *, void *);
        XID lock_meaning;
        void* lock;
        struct my_XInternalAsync *async_handlers;
        unsigned long bigreq_size;
        struct my_XLockPtrs *lock_fns;
        void (*idlist_alloc)(void *, void *, int);
        void* key_bindings;
        XID cursor_font;
        void* *atoms;
        unsigned int mode_switch;
        unsigned int num_lock;
        void* context_db;
        int (**error_vec)(void*, void*, void*);
        struct {
           void* defaultCCCs;
           void* clientCmaps;
           void* perVisualIntensityMaps;
        } cms;
        void* im_filters;
        void* qfree;
        unsigned long next_event_serial_num;
        struct my_XExten *flushes;
        struct my_XConnectionInfo *im_fd_info;
        int im_fd_length;
        struct my_XConnWatchInfo *conn_watchers;
        int watcher_count;
        void* filedes;
        int (*savedsynchandler)(void *);
        XID resource_max;
        int xcmisc_opcode;
        void* *xkb_info;
        void* *trans_conn;
        void* *xcb;
        unsigned int next_cookie;
        int (*generic_event_vec[128])(void*, void*, void*);
        int (*generic_event_copy_vec[128])(void*, void*, void*);
        void *cookiejar;
        unsigned long last_request_read_upper32bit; // 64bits only
        unsigned long request_upper32bit;   // 64bits only
        void* error_threads;
        void* exit_handler;
        void* exit_handler_data;
} my_XDisplay_t;

EXPORT void* my_XOpenDisplay(x64emu_t* emu, void* d)
{
    void* ret = my->XOpenDisplay(d);
    // Added automatic bridge because of thos macro from Xlibint.h
    //#define LockDisplay(d)       if ((d)->lock_fns) (*(d)->lock_fns->lock_display)(d)
    //#define UnlockDisplay(d)     if ((d)->lock_fns) (*(d)->lock_fns->unlock_display)(d)

    my_XDisplay_t* dpy = (my_XDisplay_t*)ret;
    if(!ret)
        return ret;

    if(BOX64ENV(x11sync)) {my->XSynchronize(ret, 1); printf_log(LOG_INFO, "Forcing Syncronized opration on Display %p\n", ret);}
    bridge_t* system = emu->context->system;

    #define GO(A, W)\
    if(dpy->A)      \
        if(!CheckBridged(system, dpy->A)) \
            AddAutomaticBridge(system, W, dpy->A, 0, #A); \

    #define GO2(A, B, W) \
    if(dpy->A && dpy->A->B)  \
        if(!CheckBridged(system, dpy->A->B)) \
            AddAutomaticBridge(system, W, dpy->A->B, 0, #B "_" #A); \


    GO2(free_funcs, atoms, vFp)
    GO2(free_funcs, modifiermap, iFp)
    GO2(free_funcs, key_bindings, vFp)
    GO2(free_funcs, context_db, vFp)
    GO2(free_funcs, defaultCCCs, vFp)
    GO2(free_funcs, clientCmaps, vFp)
    GO2(free_funcs, intensityMaps, vFp)
    GO2(free_funcs, im_filters, vFp)
    GO2(free_funcs, xkb, vFp)
    GO(resource_alloc, LFp)
    GO(synchandler, iFp)
    //TODO: ext_procs?
    //TODO: event_vec?
    //TODO: wire_vec?
    //TODO: async_handlers?
    GO2(lock_fns, lock_display, vFp);
    GO2(lock_fns, unlock_display, vFp);
    GO(idlist_alloc, vFppi)
    //TODO: error_vec?
    //TODO: flushes
    //TODO: im_fd_info?
    //TODO: conn_watchers
    GO(savedsynchandler, iFp)
    //TODO: generic_event_vec?
    //TODO: generic_event_copy_vec?


    #undef GO
    #undef GO2

    return ret;
}

EXPORT void* my__XGetRequest(x64emu_t* emu, my_XDisplay_t* dpy, uint8_t type, size_t len)
{
    // check if asynchandler needs updated wrapping
    struct my_XInternalAsync * p = dpy->async_handlers;
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

EXPORT uintptr_t my_XCreateWindow(x64emu_t* emu, my_XDisplay_t* dpy, uintptr_t v2, int32_t v3, int32_t v4, uint32_t v5, uint32_t v6, uint32_t v7, int32_t v8, uint32_t v9, void* v10, uintptr_t v11, void* v12)
{
    dpy->resource_alloc = findResourceAllocFct(dpy->resource_alloc);
    uintptr_t ret = my->XCreateWindow(dpy, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12);
    return ret;
}

EXPORT void* my_XOpenIM(x64emu_t* emu, my_XDisplay_t* dpy, void* v2, void* v3, void* v4)
{
    void* ret = my->XOpenIM(dpy, v2, v3, v4);
    bridge_t* system = emu->context->system;

    #define GO(A, W)\
    if(dpy->A)      \
        if(!CheckBridged(system, dpy->A)) \
            AddAutomaticBridge(system, W, dpy->A, 0, #A); \

    GO(resource_alloc, LFp)
    #undef GO

    return ret;
}

EXPORT int my_XCloseDisplay(x64emu_t* emu, void* dpy)
{
    int ret = my->XCloseDisplay(dpy);
    if(!ret) unregister_xcb_display(dpy);
    return ret;
}

#define CUSTOM_INIT                 \
    AddAutomaticBridge(lib->w.bridge, vFp, *(void**)dlsym(lib->w.lib, "_XLockMutex_fn"), 0, "_XLockMutex_fn"); \
    AddAutomaticBridge(lib->w.bridge, vFp, *(void**)dlsym(lib->w.lib, "_XUnlockMutex_fn"), 0, "_XUnlockMutex_fn"); \
    if(BOX64ENV(x11threads)) my->XInitThreads();

#define NEEDED_LIBS "libxcb.so.1"

#include "wrappedlib_init.h"
