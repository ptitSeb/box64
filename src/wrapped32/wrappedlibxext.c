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
#include "emu/x64emu_private.h"
#include "callback.h"
#include "librarian.h"
#include "box32context.h"
#include "emu/x64emu_private.h"
#include "converter32.h"

#ifdef ANDROID
    static const char* libxextName = "libXext.so";
#else
    static const char* libxextName = "libXext.so.6";
#endif

#define LIBNAME libxext

#include "libtools/my_x11_conv.h"

typedef struct _XImage XImage;
void BridgeImageFunc(x64emu_t *emu, XImage *img);
void UnbridgeImageFunc(x64emu_t *emu, XImage *img);
typedef int (*XextErrorHandler)(void *, void *, void*);

#include "generated/wrappedlibxexttypes32.h"

#include "wrappercallback32.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// exterrorhandle ...
#define GO(A)   \
static uintptr_t my_exterrorhandle_fct_##A = 0;                                                     \
static int my_exterrorhandle_##A(void* display, void* ext_name, void* reason)                       \
{                                                                                                   \
    return RunFunctionFmt(my_exterrorhandle_fct_##A, "ppp", getDisplay(display), ext_name, reason); \
}
SUPER()
#undef GO
static void* find_exterrorhandle_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_exterrorhandle_fct_##A == (uintptr_t)fct) return my_exterrorhandle_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_exterrorhandle_fct_##A == 0) {my_exterrorhandle_fct_##A = (uintptr_t)fct; return my_exterrorhandle_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libXext exterrorhandle callback\n");
    return NULL;
}
static void* reverse_exterrorhandleFct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_exterrorhandle_##A == fct) return (void*)my_exterrorhandle_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, iFppp_32, fct, 0, NULL);
}
// create_gc ...
#define GO(A)   \
static uintptr_t my_create_gc_fct_##A = 0;                                      \
static int my_create_gc_##A(void* a, uint32_t b, void* c)                       \
{                                                                               \
    return RunFunctionFmt(my_create_gc_fct_##A, "pup", getDisplay(a), b, c);    \
}
SUPER()
#undef GO
static void* find_create_gc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_create_gc_fct_##A == (uintptr_t)fct) return my_create_gc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_create_gc_fct_##A == 0) {my_create_gc_fct_##A = (uintptr_t)fct; return my_create_gc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libXext create_gc callback\n");
    return NULL;
}
// copy_gc ...
#define GO(A)   \
static uintptr_t my_copy_gc_fct_##A = 0;                                    \
static int my_copy_gc_##A(void* a, uint32_t b, void* c)                     \
{                                                                           \
    return RunFunctionFmt(my_copy_gc_fct_##A, "pup", getDisplay(a), b, c);  \
}
SUPER()
#undef GO
static void* find_copy_gc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_copy_gc_fct_##A == (uintptr_t)fct) return my_copy_gc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_copy_gc_fct_##A == 0) {my_copy_gc_fct_##A = (uintptr_t)fct; return my_copy_gc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libXext copy_gc callback\n");
    return NULL;
}
// flush_gc ...
#define GO(A)   \
static uintptr_t my_flush_gc_fct_##A = 0;                                   \
static int my_flush_gc_##A(void* a, uint32_t b, void* c)                    \
{                                                                           \
    return RunFunctionFmt(my_flush_gc_fct_##A, "pup", getDisplay(a), b, c); \
}
SUPER()
#undef GO
static void* find_flush_gc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_flush_gc_fct_##A == (uintptr_t)fct) return my_flush_gc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_flush_gc_fct_##A == 0) {my_flush_gc_fct_##A = (uintptr_t)fct; return my_flush_gc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libXext flush_gc callback\n");
    return NULL;
}
// free_gc ...
#define GO(A)   \
static uintptr_t my_free_gc_fct_##A = 0;                                    \
static int my_free_gc_##A(void* a, uint32_t b, void* c)                     \
{                                                                           \
    return RunFunctionFmt(my_free_gc_fct_##A, "pup", getDisplay(a), b, c);  \
}
SUPER()
#undef GO
static void* find_free_gc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_free_gc_fct_##A == (uintptr_t)fct) return my_free_gc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_free_gc_fct_##A == 0) {my_free_gc_fct_##A = (uintptr_t)fct; return my_free_gc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libXext free_gc callback\n");
    return NULL;
}
// create_font ...
#define GO(A)   \
static uintptr_t my_create_font_fct_##A = 0;                                        \
static int my_create_font_##A(void* a, void* b, void* c)                            \
{                                                                                   \
    inplace_XFontStruct_shrink(b);                                                  \
    int ret = RunFunctionFmt(my_create_font_fct_##A, "ppp", getDisplay(a), b, c);   \
    inplace_XFontStruct_enlarge(b);                                                 \
    return ret;                                                                     \
}
SUPER()
#undef GO
static void* find_create_font_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_create_font_fct_##A == (uintptr_t)fct) return my_create_font_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_create_font_fct_##A == 0) {my_create_font_fct_##A = (uintptr_t)fct; return my_create_font_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libXext create_font callback\n");
    return NULL;
}
// free_font ...
#define GO(A)   \
static uintptr_t my_free_font_fct_##A = 0;                                      \
static int my_free_font_##A(void* a, void* b, void* c)                          \
{                                                                               \
    inplace_XFontStruct_shrink(b);                                              \
    int ret = RunFunctionFmt(my_free_font_fct_##A, "ppp", getDisplay(a), b, c); \
    inplace_XFontStruct_enlarge(b);                                             \
    return ret;                                                                 \
}
SUPER()
#undef GO
static void* find_free_font_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_free_font_fct_##A == (uintptr_t)fct) return my_free_font_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_free_font_fct_##A == 0) {my_free_font_fct_##A = (uintptr_t)fct; return my_free_font_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libXext free_font callback\n");
    return NULL;
}
// close_display ...
#define GO(A)   \
static uintptr_t my_close_display_fct_##A = 0;                                  \
static int my_close_display_##A(void* a, void* b)                               \
{                                                                               \
    return RunFunctionFmt(my_close_display_fct_##A, "pp", getDisplay(a), b);    \
}
SUPER()
#undef GO
static void* find_close_display_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_close_display_fct_##A == (uintptr_t)fct) return my_close_display_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_close_display_fct_##A == 0) {my_close_display_fct_##A = (uintptr_t)fct; return my_close_display_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libXext close_display callback\n");
    return NULL;
}
// wire_to_event ...
#define GO(A)   \
static uintptr_t my_wire_to_event_fct_##A = 0;                                          \
static int my_wire_to_event_##A(void* a, void* b, void* c)                              \
{                                                                                       \
    static my_XEvent_32_t evt;                                                          \
    int ret = RunFunctionFmt(my_wire_to_event_fct_##A, "ppp", getDisplay(a), &evt, c);  \
    unconvertXEvent(b, &evt);                                                           \
}
SUPER()
#undef GO
static void* find_wire_to_event_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_wire_to_event_fct_##A == (uintptr_t)fct) return my_wire_to_event_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_wire_to_event_fct_##A == 0) {my_wire_to_event_fct_##A = (uintptr_t)fct; return my_wire_to_event_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libXext wire_to_event callback\n");
    return NULL;
}
// event_to_wire ...
#define GO(A)   \
static uintptr_t my_event_to_wire_fct_##A = 0;                                      \
static int my_event_to_wire_##A(void* a, void* b, void* c)                          \
{                                                                                   \
    static my_XEvent_32_t evt;                                                      \
    convertXEvent(&evt, b);                                                         \
    return RunFunctionFmt(my_event_to_wire_fct_##A, "ppp", getDisplay(a), &evt, c); \
}
SUPER()
#undef GO
static void* find_event_to_wire_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_event_to_wire_fct_##A == (uintptr_t)fct) return my_event_to_wire_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_event_to_wire_fct_##A == 0) {my_event_to_wire_fct_##A = (uintptr_t)fct; return my_event_to_wire_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libXext event_to_wire callback\n");
    return NULL;
}
// error ...
#define GO(A)   \
static uintptr_t my_error_fct_##A = 0;                                          \
static int my_error_##A(void* a, void* b, void* c, int* d)                      \
{                                                                               \
    return RunFunctionFmt(my_error_fct_##A, "pppp", getDisplay(a), b, c, d);    \
}
SUPER()
#undef GO
static void* find_error_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_error_fct_##A == (uintptr_t)fct) return my_error_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_error_fct_##A == 0) {my_error_fct_##A = (uintptr_t)fct; return my_error_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libXext error callback\n");
    return NULL;
}
// error_string ...
#define GO(A)   \
static uintptr_t my_error_string_fct_##A = 0;                                           \
static int my_error_string_##A(void* a, int b, void* c, void* d, int e)                 \
{                                                                                       \
    return RunFunctionFmt(my_error_string_fct_##A, "pippi", getDisplay(a), b, c, d, e); \
}
SUPER()
#undef GO
static void* find_error_string_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_error_string_fct_##A == (uintptr_t)fct) return my_error_string_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_error_string_fct_##A == 0) {my_error_string_fct_##A = (uintptr_t)fct; return my_error_string_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libXext error_string callback\n");
    return NULL;
}

#undef SUPER

EXPORT void* my32_XShmCreateImage(x64emu_t* emu, void* disp, void* vis, uint32_t depth, int32_t fmt
                    , void* data, void* shminfo, uint32_t w, uint32_t h)
{
    XImage *img = my->XShmCreateImage(disp, vis, depth, fmt, data, shminfo, w, h);
    if(!img)
        return img;
    // bridge all access functions...
    BridgeImageFunc(emu, img);
    return img;
}

EXPORT int32_t my32_XShmPutImage(x64emu_t* emu, void* disp, size_t drawable, void* gc, void* image
                    , int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y
                    , uint32_t w, uint32_t h, int32_t sendevt)
{
    UnbridgeImageFunc(emu, (XImage*)image);
    int32_t r = my->XShmPutImage(disp, drawable, gc, image, src_x, src_y, dst_x, dst_y, w, h, sendevt);
    // bridge all access functions...
    BridgeImageFunc(emu, (XImage*)image);
    return r;
}

EXPORT int32_t my32_XShmGetImage(x64emu_t* emu, void* disp, size_t drawable, void* image, int32_t x, int32_t y, size_t plane)
{
    UnbridgeImageFunc(emu, (XImage*)image);
    int32_t r = my->XShmGetImage(disp, drawable, image, x, y, plane);
    // bridge all access functions...
    BridgeImageFunc(emu, (XImage*)image);
    return r;
}

EXPORT void* my32_XSetExtensionErrorHandler(x64emu_t* emu, void* handler)
{
    (void)emu;
    return reverse_exterrorhandleFct(my->XSetExtensionErrorHandler(find_exterrorhandle_Fct(handler)));
}

//EXPORT void* my32_XextAddDisplay(x64emu_t* emu, void* extinfo, void* dpy, void* extname, my32_XExtensionHooks* hooks, int nevents, void* data)
//{
//    my32_XExtensionHooks natives = {0};
//    #define GO(A) natives.A = find_##A##_Fct(hooks->A);
//    GO(create_gc)
//    GO(copy_gc)
//    GO(flush_gc)
//    GO(free_gc)
//    GO(create_font)
//    GO(free_font)
//    GO(close_display)
//    GO(wire_to_event)
//    GO(event_to_wire)
//    GO(error)
//    GO(error_string)
//    #undef GO
//    void *ret = my->XextAddDisplay(extinfo, dpy, extname, &natives, nevents, data);
//    return ret;
//}

EXPORT void* my32_XdbeGetVisualInfo(x64emu_t* emu, void* dpy, XID_32* draws, int* num)
{
    XID draws_l[*num];
    if(*num)
        for(int i=0; i<*num; ++i)
            draws_l[i] = from_ulong(draws[i]);
    my_XdbeScreenVisualInfo_t* ret = my->XdbeGetVisualInfo(dpy, draws_l, num);
    inplace_XdbeScreenVisualInfo_shrink(ret);
    return ret;
}

EXPORT void my32_XdbeFreeVisualInfo(x64emu_t* emu, void* infos)
{
    inplace_XdbeScreenVisualInfo_enlarge(infos);
    my->XdbeFreeVisualInfo(infos);
}

EXPORT void* my32_XextCreateExtension(x64emu_t* emu)
{
    return  inplace_XExtensionInfo_shrink(my->XextCreateExtension());
}

EXPORT void my32_XextDestroyExtension(x64emu_t* emu, void* ext)
{
    my->XextDestroyExtension(inplace_XExtensionInfo_enlarge(ext));
}

EXPORT void* my32_XextAddDisplay(x64emu_t* emu, void* ext, void* dpy, void* name, my_XExtensionHooks_32_t* hooks, int nevents, void* data)
{
    my_XExtensionHooks_t hooks_l = {0};
    if(hooks) {
        #define GO(A) hooks_l.A = find_##A##_Fct(from_ptrv(hooks->A))
        GO(create_gc);
        GO(copy_gc);
        GO(flush_gc);
        GO(free_gc);
        GO(create_font);
        GO(free_font);
        GO(close_display);
        GO(wire_to_event);
        GO(event_to_wire);
        GO(error);
        GO(error_string);
        #undef GO
    }
    inplace_XExtensionInfo_shrink(my->XextAddDisplay(inplace_XExtensionInfo_enlarge(ext), dpy, name, hooks?(&hooks_l):NULL, nevents, data));
}

EXPORT void* my32_XextFindDisplay(x64emu_t* emu, void* ext, void* dpy)
{
    return inplace_XExtensionInfo_shrink(my->XextFindDisplay(inplace_XExtensionInfo_enlarge(ext), dpy));
}

EXPORT int my32_XextRemoveDisplay(x64emu_t* emu, void* ext, void* dpy)
{
    return my->XextRemoveDisplay(inplace_XExtensionInfo_enlarge(ext), dpy);
}

#if 0
#ifdef ANDROID
#define NEEDED_LIBS "libX11.so", "libxcb.so", "libXau.so", "libdl.so", "libXdmcp.so"
#else
#define NEEDED_LIBS "libX11.so.6", "libxcb.so.1", "libXau.so.6", "libdl.so.2", "libXdmcp.so.6"
#endif
#endif
#include "wrappedlib_init32.h"
