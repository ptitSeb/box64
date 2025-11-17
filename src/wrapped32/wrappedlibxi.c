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

int my32_xinput_opcode = 0; // used to expand XEvenCookie data

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

EXPORT void* my32_XQueryDeviceState(x64emu_t* emu, void* dpy, void* d)
{
    void* ret = inplace_XDeviceState_shrink(my->XQueryDeviceState(dpy, inplace_XDevice_enlarge(d)));
    inplace_XDevice_shrink(d);
    return ret;
}

EXPORT void my32_XFreeDeviceState(x64emu_t* emu, void* ds)
{
    my->XFreeDeviceState(inplace_XDeviceState_enlarge(ds));
}

EXPORT void* my32_XListInputDevices(x64emu_t* emu, void* dpy, int* n)
{
    void* ret  = my->XListInputDevices(dpy, n);
    return inplace_XDeviceInfo_shrink(ret, *n);
}

EXPORT void my32_XFreeDeviceList(x64emu_t* emu, void* l)
{
    my->XFreeDeviceList(inplace_XDeviceInfo_enlarge(l));
}

EXPORT void* my32_XGetDeviceMotionEvents(x64emu_t* emu, void* dpy, void* d, unsigned long start, unsigned long stop, int* n, int* mode, int* axis)
{
    void* ret = my->XGetDeviceMotionEvents(dpy, inplace_XDevice_enlarge(d), start, stop, n, mode, axis);
    inplace_XDevice_shrink(d);
    if(ret) {
        my_XDeviceTimeCoord_t* src = ret;
        my_XDeviceTimeCoord_32_t* dst = ret;
        for(int i=0; i<*n; ++i) {
            dst[i].time = to_ulong(src[i].time);
            dst[i].data = to_ptrv(src[i].data);
        }
        if(*n) {
            dst[*n].time = 0; dst[*n].data = 0; // mark the end
        }
    }
}

EXPORT void my32_XFreeDeviceMotionEvents(x64emu_t* emu, void* l)
{
    int n=0;
    my_XDeviceTimeCoord_32_t* src = l;
    my_XDeviceTimeCoord_t* dst = l;
    // search length first
    while(src[n].time || src[n].data) ++n;
    for(int i=n; i>=0; --i) {
        dst[i].data = from_ptrv(src[i].data);
        dst[i].time = from_ulong(src[i].time);
    }
    my->XFreeDeviceMotionEvents(l);
}

EXPORT int my32_XGrabDevice(x64emu_t* emu, void* dpy, void* d, XID w, int owner, int count, void* evt, int this_mode, int other_modes, unsigned long time)
{
    int ret = my->XGrabDevice(dpy, inplace_XDevice_enlarge(d), w, owner, count, evt, this_mode, other_modes, time);
    inplace_XDevice_shrink(d);
    return ret;
}

#include "wrappedlib_init32.h"
