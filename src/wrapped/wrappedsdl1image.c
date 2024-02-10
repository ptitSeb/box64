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
#include "box64context.h"
#include "sdl1rwops.h"

const char* sdl1imageName = "libSDL_image-1.2.so.0";
#define LIBNAME sdl1image

#define ADDED_FUNCTIONS()           \

#include "generated/wrappedsdl1imagetypes.h"

#include "wrappercallback.h"

#define GO(A)   \
void EXPORT *my_##A(x64emu_t* emu, void* a) \
{ \
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a); \
    void* r = my->A(rw); \
    RWNativeEnd(rw); \
    return r; \
}
GO(IMG_LoadBMP_RW)
GO(IMG_LoadCUR_RW)
GO(IMG_LoadGIF_RW)
GO(IMG_LoadICO_RW)
GO(IMG_LoadJPG_RW)
GO(IMG_LoadLBM_RW)
GO(IMG_LoadPCX_RW)
GO(IMG_LoadPNG_RW)
GO(IMG_LoadPNM_RW)
GO(IMG_LoadTGA_RW)
GO(IMG_LoadTIF_RW)
GO(IMG_LoadWEBP_RW)
GO(IMG_LoadXCF_RW)
GO(IMG_LoadXPM_RW)
GO(IMG_LoadXV_RW)
#undef GO

void EXPORT *my_IMG_LoadTyped_RW(x64emu_t* emu, void* a, int32_t b, void* c)
{
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    void* r = my->IMG_LoadTyped_RW(rw, b, c);
    if(b==0)
        RWNativeEnd(rw);
    return r;
}
void EXPORT *my_IMG_Load_RW(x64emu_t* emu, void* a, int32_t b)
{
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    void* r = my->IMG_Load_RW(rw, b);
    if(b==0)
        RWNativeEnd(rw);
    return r;
}

#define NEEDED_LIBS "libSDL-1.2.so.0", "libz.so.1"

#include "wrappedlib_init.h"
