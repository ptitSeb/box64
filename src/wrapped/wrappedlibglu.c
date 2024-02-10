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

const char* libgluName = "libGLU.so.1";
#define LIBNAME libglu

#include "generated/wrappedlibglutypes.h"

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

// glu_callback
#define GO(A)   \
static uintptr_t my_glu_callback_fct_##A = 0;                       \
static void my_glu_callback_##A(void* a, void* b)                   \
{                                                                   \
    RunFunctionFmt(my_glu_callback_fct_##A, "pp", a, b);      \
}
SUPER()
#undef GO
static void* findglu_callbackFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_glu_callback_fct_##A == (uintptr_t)fct) return my_glu_callback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_glu_callback_fct_##A == 0) {my_glu_callback_fct_##A = (uintptr_t)fct; return my_glu_callback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Jpeg glu_callback callback\n");
    return NULL;
}
// glu_callback4
#define GO(A)   \
static uintptr_t my_glu_callback4_fct_##A = 0;                              \
static void my_glu_callback4_##A(void* a, void* b, void* c, void* d)        \
{                                                                           \
    RunFunctionFmt(my_glu_callback4_fct_##A, "pppp", a, b, c, d);     \
}
SUPER()
#undef GO
static void* findglu_callback4Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_glu_callback4_fct_##A == (uintptr_t)fct) return my_glu_callback4_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_glu_callback4_fct_##A == 0) {my_glu_callback4_fct_##A = (uintptr_t)fct; return my_glu_callback4_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Jpeg glu_callback4 callback\n");
    return NULL;
}
// glu_callback5
#define GO(A)   \
static uintptr_t my_glu_callback5_fct_##A = 0;                                  \
static void my_glu_callback5_##A(void* a, void* b, void* c, void* d, void* e)   \
{                                                                               \
    RunFunctionFmt(my_glu_callback5_fct_##A, "ppppp", a, b, c, d, e);     \
}
SUPER()
#undef GO
static void* findglu_callback5Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_glu_callback5_fct_##A == (uintptr_t)fct) return my_glu_callback5_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_glu_callback5_fct_##A == 0) {my_glu_callback5_fct_##A = (uintptr_t)fct; return my_glu_callback5_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Jpeg glu_callback5 callback\n");
    return NULL;
}

#undef SUPER

// There are many callback signatures: vFv, vFi, vFp, vFip and vFpp... so a generic wrapping to vFpp works
// except for GLU_TESS_COMBINE and GLU_TESS_COMBINE_DATA
#define GLU_TESS_COMBINE                   100105
#define GLU_TESS_COMBINE_DATA              100111
void EXPORT my_gluQuadricCallback(x64emu_t* emu, void* a, int32_t b, void* cb)
{
    (void)emu;
    my->gluQuadricCallback(a, b, findglu_callbackFct(cb));
}
void EXPORT my_gluTessCallback(x64emu_t* emu, void* a, int32_t b, void* cb)
{
    (void)emu;
    if(b==GLU_TESS_COMBINE)
        my->gluTessCallback(a, b, findglu_callback4Fct(cb));
    else if(b==GLU_TESS_COMBINE_DATA)
        my->gluTessCallback(a, b, findglu_callback5Fct(cb));
    else
        my->gluTessCallback(a, b, findglu_callbackFct(cb));
}
void EXPORT my_gluNurbsCallback(x64emu_t* emu, void* a, int32_t b, void* cb)
{
    (void)emu;
    my->gluNurbsCallback(a, b, findglu_callbackFct(cb));
}

#define NEEDED_LIBS "libGL.so.1"

#include "wrappedlib_init.h"
