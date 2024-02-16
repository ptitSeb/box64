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

#ifdef ANDROID
    const char* libxtName = "libXt.so";
#else
    const char* libxtName = "libXt.so.6";
#endif

#define LIBNAME libxt

#include "generated/wrappedlibxttypes.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)   \
GO(5)   \
GO(6)   \
GO(7)

// Event
#define GO(A)   \
static uintptr_t my_Event_fct_##A = 0;   \
static void my_Event_##A(void* w, void* data, void* event)     \
{                                       \
    RunFunctionFmt(my_Event_fct_##A, "ppp", w, data, event);\
}
SUPER()
#undef GO
static void* findEventFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_Event_fct_##A == (uintptr_t)fct) return my_Event_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_Event_fct_##A == 0) {my_Event_fct_##A = (uintptr_t)fct; return my_Event_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libXt Event callback\n");
    return NULL;
}
// WorkProc
#define GO(A)   \
static uintptr_t my_WorkProc_fct_##A = 0;   \
static int my_WorkProc_##A(void* p)         \
{                                           \
    return (int)RunFunctionFmt(my_WorkProc_fct_##A, "p", p);\
}
SUPER()
#undef GO
static void* findWorkProcFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_WorkProc_fct_##A == (uintptr_t)fct) return my_WorkProc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_WorkProc_fct_##A == 0) {my_WorkProc_fct_##A = (uintptr_t)fct; return my_WorkProc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libXt WorkProc callback\n");
    return NULL;
}
// InputCallback
#define GO(A)   \
static uintptr_t my_InputCallback_fct_##A = 0;                      \
static void my_InputCallback_##A(void* p, void* s, void* id)        \
{                                                                   \
    RunFunctionFmt(my_InputCallback_fct_##A, "ppp", p, s, id); \
}
SUPER()
#undef GO
static void* findInputCallbackFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_InputCallback_fct_##A == (uintptr_t)fct) return my_InputCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_InputCallback_fct_##A == 0) {my_InputCallback_fct_##A = (uintptr_t)fct; return my_InputCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libXt InputCallback callback\n");
    return NULL;
}
// XtErrorMsgHandler
#define GO(A)   \
static uintptr_t my_XtErrorMsgHandler_fct_##A = 0;                                          \
static void my_XtErrorMsgHandler_##A(void* a, void* b, void* c, void* d, void* e, void* f)  \
{                                                                                           \
    RunFunctionFmt(my_XtErrorMsgHandler_fct_##A, "pppppp", a, b, c, d, e, f);               \
}
SUPER()
#undef GO
static void* findXtErrorMsgHandlerFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_XtErrorMsgHandler_fct_##A == (uintptr_t)fct) return my_XtErrorMsgHandler_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_XtErrorMsgHandler_fct_##A == 0) {my_XtErrorMsgHandler_fct_##A = (uintptr_t)fct; return my_XtErrorMsgHandler_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libXt XtErrorMsgHandler callback\n");
    return NULL;
}
// XtErrorHandler
#define GO(A)   \
static uintptr_t my_XtErrorHandler_fct_##A = 0;         \
static void my_XtErrorHandler_##A(void* a)              \
{                                                       \
    RunFunctionFmt(my_XtErrorHandler_fct_##A, "p", a);  \
}
SUPER()
#undef GO
static void* findXtErrorHandlerFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_XtErrorHandler_fct_##A == (uintptr_t)fct) return my_XtErrorHandler_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_XtErrorHandler_fct_##A == 0) {my_XtErrorHandler_fct_##A = (uintptr_t)fct; return my_XtErrorHandler_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libXt XtErrorHandler callback\n");
    return NULL;
}
// XtEventHandler
#define GO(A)   \
static uintptr_t my_XtEventHandler_fct_##A = 0;                         \
static void my_XtEventHandler_##A(void* a, void* b, void* c, void* d)   \
{                                                                       \
    RunFunctionFmt(my_XtEventHandler_fct_##A, "pppp", a, b, c, d);      \
}
SUPER()
#undef GO
static void* findXtEventHandlerFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_XtEventHandler_fct_##A == (uintptr_t)fct) return my_XtEventHandler_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_XtEventHandler_fct_##A == 0) {my_XtEventHandler_fct_##A = (uintptr_t)fct; return my_XtEventHandler_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libXt XtEventHandler callback\n");
    return NULL;
}

#undef SUPER


EXPORT void my_XtAddEventHandler(x64emu_t* emu, void* w, uint32_t mask, int32_t maskable, void* cb, void* data)
{
    (void)emu;
    void* fct = findEventFct(cb);
    my->XtAddEventHandler(w, mask, maskable, fct, data);
}

EXPORT long my_XtAppAddWorkProc(x64emu_t* emu, void* context, void* proc, void* data)
{
    (void)emu;
    return my->XtAppAddWorkProc(context, findWorkProcFct(proc), data);
}

EXPORT long my_XtAppAddInput(x64emu_t* emu, void* context, int source, void* cond, void* proc, void* data)
{
    (void)emu;
    return my->XtAppAddInput(context, source, cond, findInputCallbackFct(proc), data);
}

EXPORT void my_XtAppSetWarningMsgHandler(x64emu_t* emu, void* ctx, void* f)
{
    my->XtAppSetWarningMsgHandler(ctx, findXtErrorMsgHandlerFct(f));
}

EXPORT void my_XtAppSetErrorMsgHandler(x64emu_t* emu, void* ctx, void* f)
{
    my->XtAppSetErrorMsgHandler(ctx, findXtErrorMsgHandlerFct(f));
}

EXPORT void my_XtAppSetWarningHandler(x64emu_t* emu, void* ctx, void* f)
{
    my->XtAppSetWarningHandler(ctx, findXtErrorHandlerFct(f));
}

EXPORT void my_XtAppSetErrorHandler(x64emu_t* emu, void* ctx, void* f)
{
    my->XtAppSetErrorHandler(ctx, findXtErrorHandlerFct(f));
}

EXPORT void my_XtAddRawEventHandler(x64emu_t* emu, void* w, uint32_t mask, int nonmaskable, void* f, void* data)
{
    my->XtAddRawEventHandler(w, mask, nonmaskable, findXtEventHandlerFct(f), data);
}

#ifdef ANDROID
#define NEEDED_LIBS "libX11.so", "libXext.so"
#else
#define NEEDED_LIBS "libX11.so.6", "libXext.so.6"
#endif

#include "wrappedlib_init.h"
