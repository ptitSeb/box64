#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

const char* libtiff5Name = "libtiff.so.5";
#define LIBNAME libtiff5
#define ALTNAME "libtiff.so.6"

#include "generated/wrappedlibtiff5types.h"

#include "wrappercallback.h"

// utility functions
#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// TIFFReadWriteProc
#define GO(A)   \
static uintptr_t my_TIFFReadWriteProc_fct_##A = 0;                                  \
static size_t my_TIFFReadWriteProc_##A(void* a, void* b, size_t c)                  \
{                                                                                   \
    return (size_t)RunFunctionFmt(my_TIFFReadWriteProc_fct_##A, "ppL", a, b, c);    \
}
SUPER()
#undef GO
static void* find_TIFFReadWriteProc_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_TIFFReadWriteProc_fct_##A == (uintptr_t)fct) return my_TIFFReadWriteProc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_TIFFReadWriteProc_fct_##A == 0) {my_TIFFReadWriteProc_fct_##A = (uintptr_t)fct; return my_TIFFReadWriteProc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libTIFF TIFFReadWriteProc callback\n");
    return NULL;
}
// TIFFSeekProc
#define GO(A)   \
static uintptr_t my_TIFFSeekProc_fct_##A = 0;                               \
static ssize_t my_TIFFSeekProc_##A(void* a, ssize_t b)                      \
{                                                                           \
    return (ssize_t)RunFunctionFmt(my_TIFFSeekProc_fct_##A, "pl", a, b);    \
}
SUPER()
#undef GO
static void* find_TIFFSeekProc_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_TIFFSeekProc_fct_##A == (uintptr_t)fct) return my_TIFFSeekProc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_TIFFSeekProc_fct_##A == 0) {my_TIFFSeekProc_fct_##A = (uintptr_t)fct; return my_TIFFSeekProc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libTIFF TIFFSeekProc callback\n");
    return NULL;
}
// TIFFCloseProc
#define GO(A)   \
static uintptr_t my_TIFFCloseProc_fct_##A = 0;                      \
static int my_TIFFCloseProc_##A(void* a)                            \
{                                                                   \
    return (int)RunFunctionFmt(my_TIFFCloseProc_fct_##A, "p", a);   \
}
SUPER()
#undef GO
static void* find_TIFFCloseProc_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_TIFFCloseProc_fct_##A == (uintptr_t)fct) return my_TIFFCloseProc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_TIFFCloseProc_fct_##A == 0) {my_TIFFCloseProc_fct_##A = (uintptr_t)fct; return my_TIFFCloseProc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libTIFF TIFFCloseProc callback\n");
    return NULL;
}
// TIFFSizeProc
#define GO(A)   \
static uintptr_t my_TIFFSizeProc_fct_##A = 0;                           \
static ssize_t my_TIFFSizeProc_##A(void* a)                             \
{                                                                       \
    return (ssize_t)RunFunctionFmt(my_TIFFSizeProc_fct_##A, "p", a);    \
}
SUPER()
#undef GO
static void* find_TIFFSizeProc_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_TIFFSizeProc_fct_##A == (uintptr_t)fct) return my_TIFFSizeProc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_TIFFSizeProc_fct_##A == 0) {my_TIFFSizeProc_fct_##A = (uintptr_t)fct; return my_TIFFSizeProc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libTIFF TIFFSizeProc callback\n");
    return NULL;
}
// TIFFMapFileProc
#define GO(A)   \
static uintptr_t my_TIFFMapFileProc_fct_##A = 0;                            \
static int my_TIFFMapFileProc_##A(void* a, void* b, void* c)                \
{                                                                           \
    return (int)RunFunctionFmt(my_TIFFMapFileProc_fct_##A, "ppp", a, b, c); \
}
SUPER()
#undef GO
static void* find_TIFFMapFileProc_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_TIFFMapFileProc_fct_##A == (uintptr_t)fct) return my_TIFFMapFileProc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_TIFFMapFileProc_fct_##A == 0) {my_TIFFMapFileProc_fct_##A = (uintptr_t)fct; return my_TIFFMapFileProc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libTIFF TIFFMapFileProc callback\n");
    return NULL;
}
// TIFFUnmapFileProc
#define GO(A)   \
static uintptr_t my_TIFFUnmapFileProc_fct_##A = 0;                  \
static void my_TIFFUnmapFileProc_##A(void* a, void* b, ssize_t c)   \
{                                                                   \
    RunFunctionFmt(my_TIFFUnmapFileProc_fct_##A, "ppl", a, b, c);   \
}
SUPER()
#undef GO
static void* find_TIFFUnmapFileProc_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_TIFFUnmapFileProc_fct_##A == (uintptr_t)fct) return my_TIFFUnmapFileProc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_TIFFUnmapFileProc_fct_##A == 0) {my_TIFFUnmapFileProc_fct_##A = (uintptr_t)fct; return my_TIFFUnmapFileProc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libTIFF TIFFUnmapFileProc callback\n");
    return NULL;
}

#undef SUPER

EXPORT int my_TIFFSetField(x64emu_t* emu, void* tiff, uint32_t tag, uintptr_t* b)
{
    // most tage just use between 1 & 3 args of scalar type
    // according to the doc, there is the TIFFTAG_PHOTOSHOP that takes ? and not 2 args?? -> not handled
    // and a few tag that have 1 arg of type double or float.
    // so handling the double/float as a special case, then the other as a generic 3 params call...
    // as float are handled as double in VarArg, there treated the same way
    // some also set some callback function: unhadnled for now
    switch(tag) {
        case 341:   //TIFFTAG_SMAXSAMPLEVALUE
        case 340:   //TIFFTAG_SMINSAMPLEVALUE
        case 37439: //TIFFTAG_STONITS
        case 286:   //TIFFTAG_XPOSITION
        case 282:   //TIFFTAG_XRESOLUTION
        case 287:   //TIFFTAG_YPOSITION
        case 283:   //TIFFTAG_YRESOLUTION
            if(R_EAX) return my->TIFFSetField(tiff, tag, emu->xmm[0].d[0]);
            else {
                uint64_t val = getVArgs(emu, 2, b, 0);
                return my->TIFFSetField(tiff, tag, *(double*)&val);
            }

        default:
            return  my->TIFFSetField(tiff, tag, getVArgs(emu, 2, b, 0), getVArgs(emu, 2, b, 1), getVArgs(emu, 2, b, 2));
    }
}

EXPORT void my_TIFFError(x64emu_t* emu, void* module, const char* fmt, uintptr_t* b)
{
    #ifdef PREFER_CONVERT_VAARG
    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 2);
    #else
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 2);
    PREPARE_VALIST;
    #endif
    char* tmp;
    int dummy = vasprintf(&tmp, fmt, VARARGS);
    (void)dummy;
    my->TIFFError(module, tmp);
    free(tmp);
}

EXPORT void* my_TIFFClientOpen(x64emu_t* emu, void* filename, void* mode, void* data, void* readproc, void* writeproc, void* seekproc, void* closeproc, void* sizeproc, void* mapproc, void* unmapproc)
{
    return my->TIFFClientOpen(filename, mode, data,
        find_TIFFReadWriteProc_Fct(readproc), find_TIFFReadWriteProc_Fct(writeproc),
        find_TIFFSeekProc_Fct(seekproc), find_TIFFCloseProc_Fct(closeproc), find_TIFFSizeProc_Fct(sizeproc),
        find_TIFFMapFileProc_Fct(mapproc), find_TIFFUnmapFileProc_Fct(unmapproc)
    );
}

#include "wrappedlib_init.h"