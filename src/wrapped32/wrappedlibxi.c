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
#include "converter32.h"

#ifdef ANDROID
    static const char* libxiName = "libXi.so";
#else
    static const char* libxiName = "libXi.so.6";
#endif

#define LIBNAME libxi

#ifdef ANDROID
#define NEEDED_LIBS "libX11.so", "libXext.so"
#else
#define NEEDED_LIBS "libX11.so.6", "libXext.so.6"
#endif

#include "libtools/my_x11_conv.h"

#include "generated/wrappedlibxitypes32.h"

#include "wrappercallback32.h"

EXPORT void* my32_XIQueryDevice(x64emu_t* emu, void* dpy, int deviceid, int* ndevices)
{
    void* ret = my->XIQueryDevice(dpy, deviceid, ndevices);
    inplace_XIDeviceInfo_shrink(ret, *ndevices);
    return ret;
}

EXPORT void my32_XIFreeDeviceInfo(x64emu_t* emu, void* d)
{
    inplace_XIDeviceInfo_enlarge(d);
    my->XIFreeDeviceInfo(d);
}

EXPORT int my32_XISelectEvents(x64emu_t* emu, void* dpy, XID window, my_XIEventMask_32_t* masks, int num)
{
    my_XIEventMask_t masks_l[num];
    for(int i=0; i<num; ++i) {
        masks_l[i].deviceid = masks[i].deviceid;
        masks_l[i].mask_len = masks[i].mask_len;
        masks_l[i].mask = from_ptrv(masks[i].mask);
    }
    return my->XISelectEvents(dpy, window, masks_l, num);
}

EXPORT void* my32_XIGetSelectedEvents(x64emu_t* emu, void* dpy, XID window, int* num)
{
    void* ret = my->XIGetSelectedEvents(dpy, window, num);
    if(!ret) return NULL;
    //inplace shrink
    my_XIEventMask_t* src = ret;
    my_XIEventMask_32_t* dst = ret;
    for(int i=0; i<*num; ++i, ++src, ++dst) {
        dst->deviceid = src->deviceid;
        dst->mask_len = src->mask_len;
        dst->mask = to_ptrv(src->mask);
    }
    return ret;
}

EXPORT void* my32_XOpenDevice(x64emu_t* emu, void* dpy, XID id)
{
    void* ret = my->XOpenDevice(dpy, id);
    register_XDevice_events(ret);
    inplace_XDevice_shrink(ret);
    return ret;
}

EXPORT int my32_XCloseDevice(x64emu_t* emu, void* dpy, void* d)
{
    inplace_XDevice_enlarge(d);
    unregister_XDevice_events(d);
    return my->XCloseDevice(dpy, d);
}

EXPORT int my32_XGetDeviceButtonMapping(x64emu_t* emu, void* dpy, void* d, void* map, int nmap)
{
    inplace_XDevice_enlarge(d);
    return my->XGetDeviceButtonMapping(dpy, d, map, nmap);
    inplace_XDevice_shrink(d);
}

#include "wrappedlib_init32.h"
