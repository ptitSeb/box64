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
#include "myalign32.h"
#include "gltools.h"
#include "libtools/my_x11_conv.h"

static const char* libglxName = "libGLX.so.0";
#define LIBNAME libglx

#include "generated/wrappedlibglxtypes32.h"

#include "wrappercallback32.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \


#undef SUPER

EXPORT void* myx32_glXGetProcAddress(x64emu_t* emu, void* name) 
{
    khint_t k;
    const char* rname = (const char*)name;
    return getGLProcAddress32(emu, NULL, (glprocaddress_t)my->glXGetProcAddress, rname);
}
EXPORT void* myx32_glXGetProcAddressARB(x64emu_t* emu, void* name) __attribute__((alias("myx32_glXGetProcAddress")));

EXPORT void* myx32_glXChooseFBConfig(x64emu_t* emu, void* dpy, int screen, int* list, int* nelement)
{
    void** res = my->glXChooseFBConfig(dpy, screen, list, nelement);
    if(!res)
        return NULL;
    ptr_t *fbconfig = (ptr_t*)res;
    for(int i=0; i<*nelement; ++i)
        fbconfig[i] = to_ptrv(res[i]);
    return res;
}

EXPORT void* myx32_glXGetVisualFromFBConfig(x64emu_t* emu, void* dpy, void* config)
{
    void* res = my->glXGetVisualFromFBConfig(dpy, config);
    if(!res) return NULL;
    my_XVisualInfo_32_t* vinfo = (my_XVisualInfo_32_t*)res;
    convert_XVisualInfo_to_32(dpy, vinfo, res);
    return vinfo;
}

EXPORT void* myx32_glXChooseVisual(x64emu_t* emu, void* dpy, int screen, int* attr)
{
    void* res = my->glXChooseVisual(dpy, screen, attr);
    if(!res) return NULL;
    my_XVisualInfo_32_t* vinfo = (my_XVisualInfo_32_t*)res;
    convert_XVisualInfo_to_32(dpy, vinfo, res);
    return vinfo;
}

EXPORT void* myx32_glXCreateContext(x64emu_t* emu, void* dpy, my_XVisualInfo_32_t* info, void* shared, int direct)
{
    my_XVisualInfo_t info_l = {0};
    convert_XVisualInfo_to_64(dpy, &info_l, info);
    return my->glXCreateContext(dpy, &info_l, shared, direct);
}

EXPORT void* myx32_glXGetFBConfigs(x64emu_t* emu, void* dpy, int screen, int* n)
{
    void* ret = my->glXGetFBConfigs(dpy, screen, n);
    if(!ret) return NULL;
    ptr_t* dst = ret;
    void** src = ret;
    for(int i=0; i<*n; ++i)
        dst[i] = to_ptrv(src[i]);
    return ret;
}


#define ALTMY myx32_

#include "wrappedlib_init32.h"
