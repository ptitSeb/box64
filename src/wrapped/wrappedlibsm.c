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

const char* libsmName = "libSM.so.6";
#define LIBNAME libsm

#include "generated/wrappedlibsmtypes.h"

#include "wrappercallback.h"

typedef struct my_SmcCallbacks_s {
    struct {
	void*	 callback;
	void*	 client_data;
    } save_yourself;

    struct {
	void*	 callback;
	void*	 client_data;
    } die;

    struct {
	void*	 callback;
	void*		 client_data;
    } save_complete;

    struct {
	void* callback;
	void*		 client_data;
    } shutdown_cancelled;
} my_SmcCallbacks_t;
#define SmcSaveYourselfProcMask		    (1L << 0)
#define SmcDieProcMask			        (1L << 1)
#define SmcSaveCompleteProcMask		    (1L << 2)
#define SmcShutdownCancelledProcMask	(1L << 3)

static uintptr_t my_save_yourself_fct = 0;
static void my_save_yourself(void* smcConn, void* clientData, int saveType, int shutdown, int interactStyle, int fast)
{
    RunFunctionFmt(my_save_yourself_fct, "ppiiii", smcConn, clientData, saveType, shutdown, interactStyle, fast);
}

static uintptr_t my_die_fct = 0;
static void my_die(void* smcConn, void* clientData)
{
    RunFunctionFmt(my_die_fct, "pp", smcConn, clientData);
}

static uintptr_t my_shutdown_cancelled_fct = 0;
static void my_shutdown_cancelled(void* smcConn, void* clientData)
{
    RunFunctionFmt(my_shutdown_cancelled_fct, "pp", smcConn, clientData);
}

static uintptr_t my_save_complete_fct = 0;
static void my_save_complete(void* smcConn, void* clientData)
{
    RunFunctionFmt(my_save_complete_fct, "pp", smcConn, clientData);
}


EXPORT void* my_SmcOpenConnection(x64emu_t* emu, void* networkIdsList, void* context, int major, int minor, unsigned long mask, my_SmcCallbacks_t* cb, void* previousId, void* clientIdRet, int errorLength, void* errorRet)
{
    (void)emu;
    my_SmcCallbacks_t nat = {0};
    #define GO(A, B) if(mask&A) {my_##B##_fct = (uintptr_t)cb->B.callback; nat.B.callback = my_##B; nat.B.client_data=cb->B.client_data;}
    GO(SmcSaveYourselfProcMask, save_yourself)
    GO(SmcDieProcMask, die)
    GO(SmcSaveCompleteProcMask, save_complete)
    GO(SmcShutdownCancelledProcMask, shutdown_cancelled)
    #undef GO
    return my->SmcOpenConnection(networkIdsList, context, major, minor, mask, &nat, previousId, clientIdRet, errorLength, errorRet);
}

// utility functions
#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// Request
#define GO(A)   \
static uintptr_t my_Request_fct_##A = 0;        \
static void my_Request_##A(void* a, void* b)     \
{                                               \
    RunFunctionFmt(my_Request_fct_##A, "pp", a, b);\
}
SUPER()
#undef GO
static void* findRequestFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_Request_fct_##A == (uintptr_t)fct) return my_Request_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_Request_fct_##A == 0) {my_Request_fct_##A = (uintptr_t)fct; return my_Request_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libSM Request callback\n");
    return NULL;
}
// SmcErrorHandler
#define GO(A)   \
static uintptr_t my_SmcErrorHandler_fct_##A = 0;        \
static void my_SmcErrorHandler_##A(void* a, int b, int c, unsigned long d, int e, int f, void* g)     \
{                                               \
    RunFunctionFmt(my_SmcErrorHandler_fct_##A, "piiLiip", a, b, c, d, e, f, g);\
}
SUPER()
#undef GO
static void* findSmcErrorHandlerFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_SmcErrorHandler_fct_##A == (uintptr_t)fct) return my_SmcErrorHandler_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_SmcErrorHandler_fct_##A == 0) {my_SmcErrorHandler_fct_##A = (uintptr_t)fct; return my_SmcErrorHandler_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libSM SmcErrorHandler callback\n");
    return NULL;
}
static void* reverse_SmcErrorHandler_Fct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_SmcErrorHandler_##A == fct) return (void*)my_SmcErrorHandler_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, vFpiiLiip, fct, 0, NULL);
}

#undef SUPER

EXPORT int my_SmcInteractRequest(x64emu_t* emu, void* smcConn, int f, void* cb, void* data)
{
    (void)emu;
    return my->SmcInteractRequest(smcConn, f, findRequestFct(cb), data);
}

EXPORT int my_SmcRequestSaveYourselfPhase2(x64emu_t* emu, void* smcConn, void* cb, void* data)
{
    (void)emu;
    return my->SmcRequestSaveYourselfPhase2(smcConn, findRequestFct(cb), data);
}

EXPORT void* my_SmcSetErrorHandler(x64emu_t* emu, void* f)
{
    return reverse_SmcErrorHandler_Fct(my->SmcSetErrorHandler(findSmcErrorHandlerFct(f)));
}

#include "wrappedlib_init.h"
