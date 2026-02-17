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

#ifdef ANDROID
    static const char* libxrandrName = "libXrandr.so";
#else
    static const char* libxrandrName = "libXrandr.so.2";
#endif

#define LIBNAME libxrandr

#include "libtools/my_x11_conv.h"

#include "generated/wrappedlibxrandrtypes32.h"

#include "wrappercallback32.h"

EXPORT void* my32_XRRGetScreenResources(x64emu_t* emu, void* dpy, XID window)
{
    void* ret = my->XRRGetScreenResources(dpy, window);
    inplace_XRRScreenResources_shrink(ret);
    return ret;
}

EXPORT void* my32_XRRGetScreenResourcesCurrent(x64emu_t* emu, void* dpy, XID window)
{
    void* ret = my->XRRGetScreenResourcesCurrent(dpy, window);
    inplace_XRRScreenResources_shrink(ret);
    return ret;
}

EXPORT int my32_XRRSetCrtcConfig(x64emu_t* emu, void* dpy, void* res, XID crtc, unsigned long timestamp, int x, int y, XID mode, uint16_t rotation, XID_32* outputs, int noutputs)
{
    XID outputs_l[noutputs];
    if(outputs)
        for(int i=0; i<noutputs; ++i)
            outputs_l[i] = from_ulong(outputs[i]);
    inplace_XRRScreenResources_enlarge(res);
    int ret = my->XRRSetCrtcConfig(dpy, res, crtc, timestamp, x, y, mode, rotation, outputs?(&outputs_l):NULL, noutputs);
    inplace_XRRScreenResources_shrink(res);
    return ret;
}

EXPORT void* my32_XRRGetPanning(x64emu_t* emu, void* dpy, void* res, XID crtc)
{
    inplace_XRRScreenResources_enlarge(res);
    void* ret = my->XRRGetPanning(dpy, res, crtc);
    inplace_XRRScreenResources_shrink(res);
    if(ret) {
        // shrink XRRPanning: L and 12i
        *(ulong_t*)ret = to_ulong(*(unsigned long*)ret);
        memmove(ret+4, ret+8, 12*4);
    }
    return ret;
}

EXPORT int my32_XRRSetPanning(x64emu_t* emu, void* dpy, void* res, XID crtc, void* panning)
{
    inplace_XRRScreenResources_enlarge(res);
    // enlarge panning
    {
        unsigned long timestamp = from_ulong(*(ulong_t*)panning);
        memmove(panning+8, panning+4, 12*4);
        *(unsigned long*)panning = timestamp;
    }
    int ret = my->XRRSetPanning(dpy, res, crtc, panning);
    inplace_XRRScreenResources_shrink(res);
    {
        // shrink XRRPanning: L and 12i
        *(ulong_t*)panning = to_ulong(*(unsigned long*)panning);
        memmove(panning+4, panning+8, 12*4);
    }
    return ret;
}

EXPORT void* my32_XRRGetCrtcInfo(x64emu_t* emu, void* dpy, void* res, XID crtc)
{
    inplace_XRRScreenResources_enlarge(res);
    void* ret = my->XRRGetCrtcInfo(dpy, res, crtc);
    inplace_XRRScreenResources_shrink(res);
    inplace_XRRCrtcInfo_shrink(ret);
    return ret;
}

EXPORT void* my32_XRRGetOutputInfo(x64emu_t* emu, void* dpy, void* res, XID window)
{
    inplace_XRRScreenResources_enlarge(res);
    void* ret = my->XRRGetOutputInfo(dpy, res, window);
    inplace_XRRScreenResources_shrink(res);
    inplace_XRROutputInfo_shrink(ret);
    return ret;
}

EXPORT void* my32_XRRGetProviderInfo(x64emu_t* emu, void* dpy, void* res, XID provider)
{
    inplace_XRRScreenResources_enlarge(res);
    void* ret = my->XRRGetProviderInfo(dpy, res, provider);
    inplace_XRRScreenResources_shrink(res);
    inplace_XRRProviderInfo_shrink(ret);
    return ret;
}

EXPORT void my32_XRRFreeProviderInfo(x64emu_t* emu, void* r)
{
    inplace_XRRProviderInfo_enlarge(r);
    my->XRRFreeProviderInfo(r);
}

EXPORT void* my32_XRRGetProviderResources(x64emu_t* emu, void* dpy, XID window)
{
    void* ret = my->XRRGetProviderResources(dpy, window);
    inplace_XRRProviderResources_shrink(ret);
    return ret;
}

EXPORT void my32_XRRFreeProviderResources(x64emu_t* emu, void* r)
{
    inplace_XRRProviderResources_enlarge(r);
    my->XRRFreeProviderResources(r);
}

EXPORT void* my32_XRRListOutputProperties(x64emu_t* emu, void* dpy, XID out, int* num)
{
    XID* ret = my->XRRListOutputProperties(dpy, out, num);
    if(!ret) return NULL;
    XID_32* ret_s = (XID_32*)ret;
    for(int i=0; i<*num; ++i)
        ret_s[i] = to_ulong(ret[i]);
    return ret;
}

EXPORT void* my32_XRRQueryOutputProperty(x64emu_t* emu, void* dpy, XID output, XID prop)
{
    void* ret = my->XRRQueryOutputProperty(dpy, output, prop);
    return inplace_XRRPropertyInfo_shrink(ret);
}

EXPORT int my32_XRRQueryExtension(x64emu_t* emu, void* dpy, int* event_base, int* error_base)
{
    int fallabck;
    int *event = event_base?event_base:&fallabck;
    int ret = my->XRRQueryExtension(dpy, event, error_base);
    if(!ret) return ret;
    register_XRandR_events(*event);
    return ret;
}

EXPORT int my32_XRRUpdateConfiguration(x64emu_t* emu, my_XEvent_32_t* evt)
{
    my_XEvent_t evt_l = {0};
    unconvertXEvent(&evt_l, evt);
    return my->XRRUpdateConfiguration(&evt_l);
}

EXPORT void* my32_XRRGetMonitors(x64emu_t* emu, void* dpy, XID window, int get_active, int* nmonitors)
{
    void* ret = my->XRRGetMonitors(dpy, window, get_active, nmonitors);
    return inplace_XRRMonitorInfo_shrink(ret, *nmonitors);
}

EXPORT void my32_XRRFreeMonitors(x64emu_t* emu, void* monitors)
{
    if(!monitors) return;
    int n = 0;
    while(((my_XRRMonitorInfo_32_t*)monitors)[n].name) ++n;
    my->XRRFreeMonitors(inplace_XRRMonitorInfo_enlarge(monitors, n));
}

EXPORT int my32_XRRGetCrtcTransform(x64emu_t* emu, void* dpy, XID crtc, ptr_t* attributes)
{
    void* attributes_l = NULL;
    int ret = my->XRRGetCrtcTransform(dpy, crtc, &attributes_l);
    inplace_XRRCrtcTransformAttributes_shrink(attributes_l);
    *attributes = to_ptrv(attributes_l);
    return ret;
}

#ifdef ANDROID
#define NEEDED_LIBS "libX11.so", "libXext.so", "libXrender.so"
#else
#define NEEDED_LIBS "libX11.so.6", "libXext.so.6", "libXrender.so.1"
#endif

#define CUSTOM_FINI \
    unregister_XRandR_events();

#include "wrappedlib_init32.h"
