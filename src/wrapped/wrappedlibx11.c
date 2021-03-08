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

const char* libx11Name = "libX11.so.6";
#define LIBNAME libx11

extern int x11threads;
extern int x11glx;

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

typedef uint32_t (*uFv_t)(void);
typedef void (*vFp_t)(void*);
typedef void* (*pFp_t)(void*);
typedef void (*vFpp_t)(void*, void*);
typedef void* (*pFpp_t)(void*, void*);
typedef void* (*pFpip_t)(void*, int32_t, void*);
typedef int32_t (*iFp_t)(void*);
typedef int32_t (*iFpi_t)(void*, int32_t);
typedef int32_t (*iFpl_t)(void*, intptr_t);
typedef int32_t (*iFppp_t)(void*, void*, void*);
typedef int32_t (*iFppu_t)(void*, void*, uint32_t);
typedef int32_t (*iFpppp_t)(void*, void*, void*, void*);
typedef uint32_t (*uFpii_t)(void*, int32_t, int32_t);
typedef uintptr_t (*LFpii_t)(void*, int32_t, int32_t);
typedef int32_t (*iFpiiu_t)(void*, int32_t, int32_t, uint32_t);
typedef int32_t (*iFpiiL_t)(void*, int32_t, int32_t, uintptr_t);
typedef void* (*pFppup_t)(void*, void*, uint32_t, void*);
typedef void* (*pFpiiuu_t)(void*, int32_t, int32_t, uint32_t, uint32_t);
typedef void* (*pFppiiuuui_t)(void*, void*, int32_t, int32_t, uint32_t, uint32_t, uint32_t, int32_t);
typedef void* (*pFppuiipuuii_t)(void*, void*, uint32_t, int32_t, int32_t, void*, uint32_t, uint32_t, int32_t, int32_t);
typedef void* (*pFppiiuuuipii_t)(void*, void*, int32_t, int32_t, uint32_t, uint32_t, uint32_t, int32_t, void*, int32_t, int32_t);
typedef int32_t (*iFppppiiiiuu_t)(void*, void*, void*, void*, int32_t, int32_t, int32_t, int32_t, uint32_t, uint32_t);
typedef int (*iFppppp_t)(void*, void*, void*, void*, void*);
typedef int (*iFpppppp_t)(void*, void*, void*, void*, void*, void*);

#define SUPER() \
    GO(XSetErrorHandler, pFp_t)             \
    GO(XSetIOErrorHandler, pFp_t)           \
    GO(XESetError, pFpip_t)                 \
    GO(XESetCloseDisplay, pFpip_t)          \
    GO(XIfEvent, iFpppp_t)                  \
    GO(XCheckIfEvent, iFpppp_t)             \
    GO(XPeekIfEvent, iFpppp_t)              \
    GO(XCreateImage, pFppuiipuuii_t)        \
    GO(XInitImage, iFp_t)                   \
    GO(XGetImage, pFppiiuuui_t)             \
    GO(XPutImage, iFppppiiiiuu_t)           \
    GO(XGetSubImage, pFppiiuuuipii_t)       \
    GO(XDestroyImage, vFp_t)                \
    GO(_XDeqAsyncHandler, vFpp_t)           \
    GO(XLoadQueryFont, pFpp_t)              \
    GO(XCreateGC, pFppup_t)                 \
    GO(XSetBackground, iFppu_t)             \
    GO(XSetForeground, iFppu_t)             \
    GO(XESetWireToEvent, pFpip_t)           \
    GO(XESetEventToWire, pFpip_t)           \
    GO(XCloseDisplay, iFp_t)                \
    GO(XOpenDisplay, pFp_t)                 \
    GO(XInitThreads, uFv_t)                 \
    GO(XRegisterIMInstantiateCallback, iFpppppp_t)      \
    GO(XUnregisterIMInstantiateCallback, iFpppppp_t)    \
    GO(XQueryExtension, iFppppp_t)          \
    GO(XAddConnectionWatch, iFppp_t)        \
    GO(XRemoveConnectionWatch, iFppp_t)     \

typedef struct x11_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} x11_my_t;

void* getX11My(library_t* lib)
{
    x11_my_t* my = (x11_my_t*)calloc(1, sizeof(x11_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}

void freeX11My(void* lib)
{
    // x11_my_t *my = (x11_my_t *)lib;
}
#undef SUPER

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
    return (int)RunFunction(my_context, my_wire_to_event_fct_##A, 3, dpy, re, event);\
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
    if(CheckBridged(lib->priv.w.bridge, fct))
        return (void*)CheckBridged(lib->priv.w.bridge, fct);
    #define GO(A) if(my_wire_to_event_##A == fct) return (void*)my_wire_to_event_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(lib->priv.w.bridge, iFppp, fct, 0);
}

// event_to_wire
#define GO(A)   \
static uintptr_t my_event_to_wire_fct_##A = 0;                      \
static int my_event_to_wire_##A(void* dpy, void* re, void* event)   \
{                                                                   \
    return (int)RunFunction(my_context, my_event_to_wire_fct_##A, 3, dpy, re, event);\
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
    if(CheckBridged(lib->priv.w.bridge, fct))
        return (void*)CheckBridged(lib->priv.w.bridge, fct);
    #define GO(A) if(my_event_to_wire_##A == fct) return (void*)my_event_to_wire_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(lib->priv.w.bridge, iFppp, fct, 0);
}

// error_handler
#define GO(A)   \
static uintptr_t my_error_handler_fct_##A = 0;                      \
static int my_error_handler_##A(void* dpy, void* error)   \
{                                                                   \
    return (int)RunFunction(my_context, my_error_handler_fct_##A, 2, dpy, error);\
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
    if(CheckBridged(lib->priv.w.bridge, fct))
        return (void*)CheckBridged(lib->priv.w.bridge, fct);
    #define GO(A) if(my_error_handler_##A == fct) return (void*)my_error_handler_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(lib->priv.w.bridge, iFpp, fct, 0);
}

// ioerror_handler
#define GO(A)   \
static uintptr_t my_ioerror_handler_fct_##A = 0;                      \
static int my_ioerror_handler_##A(void* dpy)   \
{                                                                   \
    return (int)RunFunction(my_context, my_ioerror_handler_fct_##A, 1, dpy);\
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
    if(CheckBridged(lib->priv.w.bridge, fct))
        return (void*)CheckBridged(lib->priv.w.bridge, fct);
    #define GO(A) if(my_ioerror_handler_##A == fct) return (void*)my_ioerror_handler_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(lib->priv.w.bridge, iFp, fct, 0);
}

// exterror_handler
#define GO(A)   \
static uintptr_t my_exterror_handler_fct_##A = 0;                      \
static int my_exterror_handler_##A(void* dpy, void* err, void* codes, int* ret_code)   \
{                                                                   \
    return (int)RunFunction(my_context, my_exterror_handler_fct_##A, 4, dpy, err, codes, ret_code);\
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
    if(CheckBridged(lib->priv.w.bridge, fct))
        return (void*)CheckBridged(lib->priv.w.bridge, fct);
    #define GO(A) if(my_exterror_handler_##A == fct) return (void*)my_exterror_handler_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(lib->priv.w.bridge, iFpppp, fct, 0);
}

// close_display
#define GO(A)   \
static uintptr_t my_close_display_fct_##A = 0;                      \
static int my_close_display_##A(void* dpy, void* codes)   \
{                                                                   \
    return (int)RunFunction(my_context, my_close_display_fct_##A, 2, dpy, codes);\
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
    if(CheckBridged(lib->priv.w.bridge, fct))
        return (void*)CheckBridged(lib->priv.w.bridge, fct);
    #define GO(A) if(my_close_display_##A == fct) return (void*)my_close_display_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(lib->priv.w.bridge, iFpp, fct, 0);
}

// register_im
#define GO(A)   \
static uintptr_t my_register_im_fct_##A = 0;                        \
static void my_register_im_##A(void* dpy, void* u, void* d)         \
{                                                                   \
    RunFunction(my_context, my_register_im_fct_##A, 3, dpy, u, d);  \
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
    if(CheckBridged(lib->priv.w.bridge, fct))
        return (void*)CheckBridged(lib->priv.w.bridge, fct);
    #define GO(A) if(my_register_im_##A == fct) return (void*)my_register_im_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(lib->priv.w.bridge, iFppp, fct, 0);
}

// XConnectionWatchProc
#define GO(A)   \
static uintptr_t my_XConnectionWatchProc_fct_##A = 0;                               \
static void my_XConnectionWatchProc_##A(void* dpy, void* data, int op, void* d)     \
{                                                                                   \
    RunFunction(my_context, my_XConnectionWatchProc_fct_##A, 4, dpy, data, op, d);  \
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
    return RunFunction(my_context, my_xifevent_fct_##A, 3, dpy, event, d);  \
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
    return RunFunction(my_context, my_XInternalAsyncHandler_fct_##A, 5, dpy, rep, buf, len, data);  \
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
#undef SUPER

void* my_XCreateImage(x64emu_t* emu, void* disp, void* vis, uint32_t depth, int32_t fmt, int32_t off
                    , void* data, uint32_t w, uint32_t h, int32_t pad, int32_t bpl);

int32_t my_XInitImage(x64emu_t* emu, void* img);

void* my_XGetImage(x64emu_t* emu, void* disp, void* drawable, int32_t x, int32_t y
                    , uint32_t w, uint32_t h, uint32_t plane, int32_t fmt);

int32_t my_XPutImage(x64emu_t* emu, void* disp, void* drawable, void* gc, void* image
                    , int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y
                    , uint32_t w, uint32_t h);

void* my_XGetSubImage(x64emu_t* emu, void* disp, void* drawable
                    , int32_t x, int32_t y
                    , uint32_t w, uint32_t h, uint32_t plane, int32_t fmt
                    , void* image, int32_t dst_x, int32_t dst_y);

void my_XDestroyImage(x64emu_t* emu, void* image);

#ifdef PANDORA
void* my_XLoadQueryFont(x64emu_t* emu, void* d, void* name);
#endif

void* my_XVaCreateNestedList(int dummy, void* p);

EXPORT void* my_XSetErrorHandler(x64emu_t* emu, XErrorHandler handler)
{
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;
    void* ret = my->XSetErrorHandler(finderror_handlerFct(handler));
    return reverse_error_handlerFct(lib, ret);
}

EXPORT void* my_XSetIOErrorHandler(x64emu_t* emu, XIOErrorHandler handler)
{
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;
    void* ret = my->XSetIOErrorHandler(findioerror_handlerFct(handler));
    return reverse_ioerror_handlerFct(lib, ret);
}

EXPORT void* my_XESetError(x64emu_t* emu, void* display, int32_t extension, void* handler)
{
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;
    void* ret = my->XESetError(display, extension, findexterror_handlerFct(handler));
    return reverse_exterror_handlerFct(lib, ret);
}

EXPORT void* my_XESetCloseDisplay(x64emu_t* emu, void* display, int32_t extension, void* handler)
{
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;
    void* ret = my->XESetCloseDisplay(display, extension, findclose_displayFct(handler));
    return reverse_close_displayFct(lib, ret);
}

EXPORT int32_t my_XIfEvent(x64emu_t* emu, void* d,void* ev, EventHandler h, void* arg)
{
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;
    int32_t ret = my->XIfEvent(d, ev, findxifeventFct(h), arg);
    return ret;
}

EXPORT int32_t my_XCheckIfEvent(x64emu_t* emu, void* d,void* ev, EventHandler h, void* arg)
{
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;
    int32_t ret = my->XCheckIfEvent(d, ev, findxifeventFct(h), arg);
    return ret;
}

EXPORT int32_t my_XPeekIfEvent(x64emu_t* emu, void* d,void* ev, EventHandler h, void* arg)
{
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;
    int32_t ret = my->XPeekIfEvent(d, ev, findxifeventFct(h), arg);
    return ret;
}

void sub_image_wrapper(x64emu_t *emu, uintptr_t fnc);
typedef void* (*sub_image_wrapper_t)(void*, int32_t, int32_t, uint32_t, uint32_t);


void BridgeImageFunc(x64emu_t *emu, XImage *img)
{
    bridge_t* system = emu->context->system;

    #define GO(A, W) \
    fnc = CheckBridged(system, img->f.A); \
    if(!fnc) fnc = AddAutomaticBridge(emu, system, W, img->f.A, 0); \
    img->f.A = (W##_t)fnc;

    uintptr_t fnc;

    GO(create_image, pFppuiipuuii)
    GO(destroy_image, iFp)
    GO(get_pixel, LFpii)
    GO(put_pixel, iFpiiL)
    GO(sub_image, sub_image_wrapper)
    GO(add_pixel, iFpl)
    #undef GO
}

void UnbridgeImageFunc(x64emu_t *emu, XImage *img)
{
    #define GO(A, W) \
    fnc = GetNativeFnc((uintptr_t)img->f.A); \
    if(fnc) \
        img->f.A = (W##_t)fnc;

    void* fnc;

    GO(create_image, pFppuiipuuii)
    GO(destroy_image, iFp)
    GO(get_pixel, LFpii)
    GO(put_pixel, iFpiiL)
    GO(sub_image, sub_image_wrapper)
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
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;

    XImage *img = my->XCreateImage(disp, vis, depth, fmt, off, data, w, h, pad, bpl);
    if(!img)
        return img;
    // bridge all access functions...
    BridgeImageFunc(emu, img);
    return img;
}

EXPORT int32_t my_XInitImage(x64emu_t* emu, void* img)
{
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;

    int ret = my->XInitImage(img);
    // bridge all access functions...
    BridgeImageFunc(emu, img);
    return ret;
}

EXPORT void* my_XGetImage(x64emu_t* emu, void* disp, void* drawable, int32_t x, int32_t y
                    , uint32_t w, uint32_t h, uint32_t plane, int32_t fmt)
{
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;

    XImage *img = my->XGetImage(disp, drawable, x, y, w, h, plane, fmt);
    if(!img)
        return img;
    // bridge all access functions...
    BridgeImageFunc(emu, img);
    return img;
}

EXPORT int32_t my_XPutImage(x64emu_t* emu, void* disp, void* drawable, void* gc, void* image
                    , int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y
                    , uint32_t w, uint32_t h)
{
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;
    UnbridgeImageFunc(emu, (XImage*)image);
    int32_t r = my->XPutImage(disp, drawable, gc, image, src_x, src_y, dst_x, dst_y, w, h);
    // bridge all access functions...
    BridgeImageFunc(emu, (XImage*)image);
    return r;
}

EXPORT void* my_XGetSubImage(x64emu_t* emu, void* disp, void* drawable
                    , int32_t x, int32_t y
                    , uint32_t w, uint32_t h, uint32_t plane, int32_t fmt
                    , void* image, int32_t dst_x, int32_t dst_y)
{
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;

    UnbridgeImageFunc(emu, (XImage*)image);
    XImage *img = my->XGetSubImage(disp, drawable, x, y, w, h, plane, fmt, image, dst_x, dst_y);
    if(img)
        BridgeImageFunc(emu, img);

    BridgeImageFunc(emu, (XImage*)image);
    return img;
}

EXPORT void my_XDestroyImage(x64emu_t* emu, void* image)
{
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;

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
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;
    my->_XDeqAsyncHandler(findXInternalAsyncHandlerFct(cb), data);
}
#if 0
typedef struct my_XIMArg_s {
    char    *name;
    void    *value;
} my_XIMArg_t;
#define my_XNVaNestedList                       "XNVaNestedList"

EXPORT void* my_XVaCreateNestedList(int dummy, void* b)
{
    // need to create a similar function here...
    void* p = b;
    int n = 0;
    while(p++) ++n;
    void** ret = (void**)malloc(sizeof(void*)*n);
    p = b;
    n = 0;
    while(p++)
        ret[n++] = p;
    return ret;
}
#endif

EXPORT void* my_XESetWireToEvent(x64emu_t* emu, void* display, int32_t event_number, void* proc)
{
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;
    void* ret = NULL;

    ret = my->XESetWireToEvent(display, event_number, findwire_to_eventFct(proc));

    return reverse_wire_to_eventFct(lib, ret);
}
EXPORT void* my_XESetEventToWire(x64emu_t* emu, void* display, int32_t event_number, void* proc)
{
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;
    void* ret = NULL;

    ret = my->XESetEventToWire(display, event_number, findevent_to_wireFct(proc));

    return reverse_event_to_wireFct(lib, ret);
}

EXPORT int my_XRegisterIMInstantiateCallback(x64emu_t* emu, void* d, void* db, void* res_name, void* res_class, void* cb, void* data)
{
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;

    return my->XRegisterIMInstantiateCallback(d, db, res_name, res_class, findregister_imFct(cb), data);
}
    
EXPORT int my_XUnregisterIMInstantiateCallback(x64emu_t* emu, void* d, void* db, void* res_name, void* res_class, void* cb, void* data)
{
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;

    return my->XUnregisterIMInstantiateCallback(d, db, res_name, res_class, reverse_register_imFct(lib, cb), data);
}

EXPORT int my_XQueryExtension(x64emu_t* emu, void* display, char* name, int* major, int* first_event, int* first_error)
{
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;

    int ret = my->XQueryExtension(display, name, major, first_event, first_error);
    if(!ret && name && !strcmp(name, "GLX") && x11glx) {
        // hack to force GLX to be accepted, even if not present
        // left major and first_XXX to default...
        ret = 1;
    }
    return ret;
}

EXPORT int my_XAddConnectionWatch(x64emu_t* emu, void* display, char* f, void* data)
{
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;

    return my->XAddConnectionWatch(display, findXConnectionWatchProcFct(f), data);
}

EXPORT int my_XRemoveConnectionWatch(x64emu_t* emu, void* display, char* f, void* data)
{
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;

    return my->XRemoveConnectionWatch(display, findXConnectionWatchProcFct(f), data);
}

#define CUSTOM_INIT                 \
    box64->x11lib = lib;            \
    lib->priv.w.p2 = getX11My(lib); \
    if(x11threads) ((x11_my_t*)lib->priv.w.p2)->XInitThreads();

#define CUSTOM_FINI \
    freeX11My(lib->priv.w.p2); \
    free(lib->priv.w.p2);   \
    ((box64context_t*)(lib->context))->x11lib = NULL; \

#include "wrappedlib_init.h"
