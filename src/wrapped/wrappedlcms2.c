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

const char* lcms2Name = "liblcms2.so.2";
#define LIBNAME lcms2

#define ADDED_FUNCTIONS()       \

#include "generated/wrappedlcms2types.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)


// cmsLogErrorHandlerFunction
#define GO(A)   \
static uintptr_t my_cmsLogErrorHandlerFunction_fct_##A = 0;                     \
static void my_cmsLogErrorHandlerFunction_##A(void* a, uint32_t b, void* c)     \
{                                                                               \
    RunFunctionFmt(my_cmsLogErrorHandlerFunction_fct_##A, "pup", a, b, c);      \
}
SUPER()
#undef GO
static void* findcmsLogErrorHandlerFunctionFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_cmsLogErrorHandlerFunction_fct_##A == (uintptr_t)fct) return my_cmsLogErrorHandlerFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_cmsLogErrorHandlerFunction_fct_##A == 0) {my_cmsLogErrorHandlerFunction_fct_##A = (uintptr_t)fct; return my_cmsLogErrorHandlerFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for liblcms2 cmsLogErrorHandlerFunction callback\n");
    return NULL;
}

#undef SUPER

EXPORT void my_cmsSetLogErrorHandlerTHR(x64emu_t* emu, void* ctx, void* f)
{
    my->cmsSetLogErrorHandlerTHR(ctx, findcmsLogErrorHandlerFunctionFct(f));
}

#include "wrappedlib_init.h"

