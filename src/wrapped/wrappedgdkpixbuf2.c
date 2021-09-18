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

const char* gdkpixbuf2Name = "libgdk_pixbuf-2.0.so.0";
#define LIBNAME gdkpixbuf2

typedef void* (*pFpiiiiiipp_t)(void*, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, void*, void*);

#define SUPER() \
    GO(gdk_pixbuf_new_from_data, pFpiiiiiipp_t)

typedef struct gdkpixbuf2_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} gdkpixbuf2_my_t;

void* getGdkpixbuf2My(library_t* lib)
{
    gdkpixbuf2_my_t* my = (gdkpixbuf2_my_t*)calloc(1, sizeof(gdkpixbuf2_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeGdkpixbuf2My(void* lib)
{
    //gdkpixbuf2_my_t *my = (gdkpixbuf2_my_t *)lib;
}

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)

// destroy_pixbuf
#define GO(A)   \
static uintptr_t my_destroy_pixbuf_fct_##A = 0;                             \
static void my_destroy_pixbuf_##A(void* pixels, void* data)                 \
{                                                                           \
    RunFunction(my_context, my_destroy_pixbuf_fct_##A, 2, pixels, data);    \
}
SUPER()
#undef GO
static void* finddestroy_pixbufFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_destroy_pixbuf_fct_##A == (uintptr_t)fct) return my_destroy_pixbuf_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_destroy_pixbuf_fct_##A == 0) {my_destroy_pixbuf_fct_##A = (uintptr_t)fct; return my_destroy_pixbuf_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gdk-pixbuf2 destroy_pixbuf callback\n");
    return NULL;
}

#undef SUPER

EXPORT void* my_gdk_pixbuf_new_from_data(x64emu_t* emu, void* data, int32_t colorspace, int32_t has_alpha, int32_t bpp, int32_t w, int32_t h, int32_t stride, void* destroy_func, void* destroy_data)
{
    library_t * lib = GetLibInternal(gdkpixbuf2Name);
    gdkpixbuf2_my_t *my = (gdkpixbuf2_my_t*)lib->priv.w.p2;

    return my->gdk_pixbuf_new_from_data(data, colorspace, has_alpha, bpp, w, h, stride, finddestroy_pixbufFct(destroy_func), destroy_data);
}


#define PRE_INIT    \
    if(box64_nogtk) \
        return -1;

#define CUSTOM_INIT \
    lib->priv.w.p2 = getGdkpixbuf2My(lib);

#define CUSTOM_FINI \
    freeGdkpixbuf2My(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"
