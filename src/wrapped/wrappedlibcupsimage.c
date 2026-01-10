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

const char* libcupsimageName = "libcupsimage.so.2";
#define ALTNAME "libcupsimage.so"

#define LIBNAME libcupsimage

#define ADDED_FUNCTIONS()                   \

#include "generated/wrappedlibcupsimagetypes.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// cups_raster_iocb_t ...
#define GO(A)   \
static uintptr_t my_cups_raster_iocb_t_fct_##A = 0;                                 \
static ssize_t my_cups_raster_iocb_t_##A(void* a, void* b, size_t c)                \
{                                                                                   \
    return (ssize_t)RunFunctionFmt(my_cups_raster_iocb_t_fct_##A, "ppL", a, b, c);  \
}
SUPER()
#undef GO
static void* find_cups_raster_iocb_t_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_cups_raster_iocb_t_fct_##A == (uintptr_t)fct) return my_cups_raster_iocb_t_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_cups_raster_iocb_t_fct_##A == 0) {my_cups_raster_iocb_t_fct_##A = (uintptr_t)fct; return my_cups_raster_iocb_t_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcupsimage cups_raster_iocb_t callback\n");
    return NULL;
}
// cups_interpret_cb_t ...
#define GO(A)   \
static uintptr_t my_cups_interpret_cb_t_fct_##A = 0;                        \
static int my_cups_interpret_cb_t_##A(void* a, int b)                       \
{                                                                           \
    return (int)RunFunctionFmt(my_cups_interpret_cb_t_fct_##A, "pi", a, b); \
}
SUPER()
#undef GO
static void* find_cups_interpret_cb_t_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_cups_interpret_cb_t_fct_##A == (uintptr_t)fct) return my_cups_interpret_cb_t_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_cups_interpret_cb_t_fct_##A == 0) {my_cups_interpret_cb_t_fct_##A = (uintptr_t)fct; return my_cups_interpret_cb_t_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcupsimage cups_interpret_cb_t callback\n");
    return NULL;
}

#undef SUPER

EXPORT void* my_cupsRasterOpenIO(x64emu_t* emu, void* f, void* ctx, uint32_t mode)
{
    return my->cupsRasterOpenIO(find_cups_raster_iocb_t_Fct(f), ctx, mode);
}

EXPORT int my_cupsRasterInterpretPPD(x64emu_t* emu, void* h, void* ppd, int noptions, void* options, void* f)
{
    return my->cupsRasterInterpretPPD(h, ppd, noptions, options, find_cups_interpret_cb_t_Fct(f));
}

#include "wrappedlib_init.h"
