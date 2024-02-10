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
#include "librarian.h"
#include "x64emu.h"
#include "callback.h"
#include "box64context.h"
#include "sdl1rwops.h"
#include "myalign.h"

const char* smpegName = "libsmpeg-0.4.so.0";
#define LIBNAME smpeg

#define ADDED_FUNCTIONS()           \

#include "generated/wrappedsmpegtypes.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// dispcallback ...
#define GO(A)   \
static uintptr_t my_dispcallback_fct_##A = 0;                                                   \
static void my_dispcallback_##A(void* dst, int32_t x, int32_t y, unsigned int w, unsigned int h)\
{                                                                                               \
    RunFunctionFmt(my_dispcallback_fct_##A, "piiuu", dst, x, y, w, h);                       \
}
SUPER()
#undef GO
static void* find_dispcallback_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_dispcallback_fct_##A == (uintptr_t)fct) return my_dispcallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_dispcallback_fct_##A == 0) {my_dispcallback_fct_##A = (uintptr_t)fct; return my_dispcallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libsmpeg dispcallback callback\n");
    return NULL;
}
#undef SUPER

EXPORT void my_SMPEG_setdisplay(x64emu_t* emu, void* mpeg, void* surf, void* lock, void* cb)
{
    (void)emu;
    my->SMPEG_setdisplay(mpeg, surf, lock, find_dispcallback_Fct(cb));
}

EXPORT void* my_SMPEG_new_rwops(x64emu_t* emu, void* src, void* info, int32_t sdl_audio)
{
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)src);
    void* ret = my->SMPEG_new_rwops(rw, info, sdl_audio);
    RWNativeEnd(rw);
    return ret;
}

#include "wrappedlib_init.h"
