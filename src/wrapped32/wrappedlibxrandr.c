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

#include "libtools/my_x11_defs.h"
#include "libtools/my_x11_defs_32.h"

#include "generated/wrappedlibxrandrtypes32.h"

#include "wrappercallback32.h"

void convert_XRRModeInfo_to_32(void* d, const void* s)
{
    my_XRRModeInfo_32_t *dst = d;
    const my_XRRModeInfo_t *src = s;
    dst->id = to_ulong(src->id);
    dst->width = src->width;
    dst->height = src->height;
    dst->dotClock = to_ulong(src->dotClock);
    dst->hSyncStart = src->hSyncStart;
    dst->hSyncEnd = src->hSyncEnd;
    dst->hTotal = src->hTotal;
    dst->hSkew = src->hSkew;
    dst->vSyncStart = src->vSyncStart;
    dst->vSyncEnd = src->vSyncEnd;
    dst->vTotal = src->vTotal;
    dst->name = to_ptrv(src->name);
    dst->nameLength = src->nameLength;
    dst->modeFlags = to_ulong(src->modeFlags);
}

void convert_XRRModeInfo_to_64(void* d, const void* s)
{
    my_XRRModeInfo_t *dst = d;
    const my_XRRModeInfo_32_t *src = s;
    dst->modeFlags = from_ulong(src->modeFlags);
    dst->nameLength = src->nameLength;
    dst->name = from_ptrv(src->name);
    dst->vTotal = src->vTotal;
    dst->vSyncEnd = src->vSyncEnd;
    dst->vSyncStart = src->vSyncStart;
    dst->hSkew = src->hSkew;
    dst->hTotal = src->hTotal;
    dst->hSyncEnd = src->hSyncEnd;
    dst->hSyncStart = src->hSyncStart;
    dst->dotClock = from_ulong(src->dotClock);
    dst->height = src->height;
    dst->width = src->width;
    dst->id = from_ulong(src->id);
}

void inplace_XRRScreenResources_shrink(void* s)
{
    if(!s) return;
    my_XRRScreenResources_32_t *dst = s;
    my_XRRScreenResources_t *src = s;
    // shrinking, so forward...
    dst->timestamp = to_ulong(src->timestamp);
    dst->configTimestamp = to_ulong(src->configTimestamp);
    dst->ncrtc = src->ncrtc;
    for(int i=0; i<dst->ncrtc; ++i)
        ((XID_32*)src->crtcs)[i] = to_ulong(src->crtcs[i]);
    dst->crtcs = to_ptrv(src->crtcs);
    dst->noutput = src->noutput;
    for(int i=0; i<dst->noutput; ++i)
        ((XID_32*)src->outputs)[i] = to_ulong(src->outputs[i]);
    dst->outputs = to_ptrv(src->outputs);
    dst->nmode = src->nmode;
    for(int i=0; i<dst->noutput; ++i)
        convert_XRRModeInfo_to_32(&((my_XRRModeInfo_32_t*)src->modes)[i], &src->modes[i]);
    dst->modes = to_ptrv(src->modes);
}

void inplace_XRRScreenResources_enlarge(void* s)
{
    if(!s) return;
    my_XRRScreenResources_t *dst = s;
    my_XRRScreenResources_32_t *src = s;
    // enlarge, so backward...
    int nmode = src->nmode;
    int noutput = src->noutput;
    int ncrtc = src->ncrtc;
    dst->modes = from_ptrv(src->modes);
    for(int i=nmode-1; i>=0; --i)
        convert_XRRModeInfo_to_64(&dst->modes[i], &((my_XRRModeInfo_32_t*)dst->modes)[i]);
    dst->nmode = src->nmode;
    dst->outputs = from_ptrv(src->outputs);
    for(int i=noutput-1; i>=0; --i)
        dst->outputs[i] = from_ulong(((XID_32*)dst->outputs)[i]);
    dst->noutput = src->noutput;
    dst->crtcs = from_ptrv(src->crtcs);
    for(int i=ncrtc-1; i>=0; --i)
        dst->crtcs[i] = from_ulong(((XID_32*)dst->crtcs)[i]);
    dst->ncrtc = src->ncrtc;
    dst->configTimestamp = to_ulong(src->configTimestamp);
    dst->timestamp = to_ulong(src->timestamp);
}

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
    memset(outputs_l, 0, sizeof(outputs_l));
    inplace_XRRScreenResources_enlarge(res);
    int ret = my->XRRSetCrtcConfig(dpy, res, crtc, timestamp, x, y, mode, rotation, &outputs_l, noutputs);
    inplace_XRRScreenResources_shrink(res);
    for(int i=0; i<noutputs; ++i)
        outputs[i] = to_ulong(outputs_l[i]);
    return ret;
}

EXPORT void* my32_XRRGetPanning(x64emu_t* emu, void* dpy, void* res, XID crtc)
{
    inplace_XRRScreenResources_enlarge(res);
    void* ret = my->XRRGetPanning(dpy, res, crtc);
    inplace_XRRScreenResources_shrink(res);
    if(ret) {
        // shrink XRRPanning: L and 12i
        *(ulong_t*)res = to_ulong(*(unsigned long*)res);
        memmove(res+4, res+8, 12*4);
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

void inplace_XRRCrtcInfo_shrink(void* s)
{
    if(!s) return;
    my_XRRCrtcInfo_32_t *dst = s;
    my_XRRCrtcInfo_t *src = s;
    dst->timestamp = to_ulong(src->timestamp);
    dst->x = src->x;
    dst->y = src->y;
    dst->width = src->width;
    dst->height = src->height;
    dst->mode = to_ulong(src->mode);
    dst->rotation = src->rotation;
    for(int i=0; i<src->noutput; ++i)
        ((XID_32*)src->outputs)[i] = to_ulong(src->outputs[i]);
    dst->noutput = src->noutput;
    dst->outputs = to_ptrv(src->outputs);
    dst->rotations = src->rotations;
    dst->npossible = src->npossible;
    for(int i=0; i<dst->npossible; ++i)
        ((XID_32*)src->possible)[i] = to_ulong(src->possible[i]);
    dst->possible = to_ptrv(src->possible);
}

EXPORT void* my32_XRRGetCrtcInfo(x64emu_t* emu, void* dpy, void* res, XID crtc)
{
    inplace_XRRScreenResources_enlarge(res);
    void* ret = my->XRRGetCrtcInfo(dpy, res, crtc);
    inplace_XRRScreenResources_shrink(res);
    inplace_XRRCrtcInfo_shrink(ret);
    return ret;
}

void inplace_XRROutputInfo_shrink(void* s)
{
    if(!s) return;
    my_XRROutputInfo_32_t *dst = s;
    my_XRROutputInfo_t *src = s;
    dst->timestamp = to_ulong(src->timestamp);
    dst->crtc = src->crtc;
    dst->name = to_ptrv(src->name);
    dst->nameLen = src->nameLen;
    dst->mm_width = to_ulong(src->mm_width);
    dst->mm_height = to_ulong(src->mm_height);
    dst->connection = src->connection;
    dst->subpixel_order = src->subpixel_order;
    dst->ncrtc = src->ncrtc;
    for(int i=0; i<dst->ncrtc; ++i)
        ((XID_32*)src->crtcs)[i] = to_ulong(src->crtcs[i]);
    dst->crtcs = to_ptrv(src->crtcs);
    dst->nclone = src->nclone;
    for(int i=0; i<dst->nclone; ++i)
        ((XID_32*)src->clones)[i] = to_ulong(src->clones[i]);
    dst->clones = to_ptrv(src->clones);
    dst->nmode = src->nmode;
    dst->npreferred = src->npreferred;
    for(int i=0; i<dst->nmode; ++i)
        ((XID_32*)src->modes)[i] = to_ulong(src->modes[i]);
    dst->modes = to_ptrv(src->modes);
}

EXPORT void* my32_XRRGetOutputInfo(x64emu_t* emu, void* dpy, void* res, XID window)
{
    inplace_XRRScreenResources_enlarge(res);
    void* ret = my->XRRGetOutputInfo(dpy, res, window);
    inplace_XRRScreenResources_shrink(res);
    inplace_XRROutputInfo_shrink(ret);
    return ret;
}

void inplace_XRRProviderInfo_shrink(void* a)
{
    if(!a) return;
    my_XRRProviderInfo_32_t *dst = a;
    my_XRRProviderInfo_t* src = a;

    for(int i=0; i<src->ncrtcs; ++i)
        ((ulong_t*)src->crtcs)[i] = to_ulong(src->crtcs[i]);
    for(int i=0; i<src->noutputs; ++i)
        ((ulong_t*)src->outputs)[i] = to_ulong(src->outputs[i]);
    for(int i=0; i<src->nassociatedproviders; ++i)
        ((ulong_t*)src->associated_providers)[i] = to_ulong(src->associated_providers[i]);
    dst->capabilities = src->capabilities;
    dst->ncrtcs = src->ncrtcs;
    dst->crtcs = to_ptrv(src->crtcs);
    dst->noutputs = src->noutputs;
    dst->outputs = to_ptrv(src->outputs);
    dst->name = to_ptrv(src->name);
    dst->nassociatedproviders = src->nassociatedproviders;
    dst->associated_providers = to_ptrv(src->associated_providers);
    dst->associated_capability = to_ptrv(src->associated_capability);
    dst->nameLen = src->nameLen;
}
void inplace_XRRProviderInfo_enlarge(void* a)
{
    if(!a) return;
    my_XRRProviderInfo_t *dst = a;
    my_XRRProviderInfo_32_t* src = a;

    dst->nameLen = src->nameLen;
    dst->associated_capability = from_ptrv(src->associated_capability);
    dst->associated_providers = from_ptrv(src->associated_providers);
    dst->nassociatedproviders = src->nassociatedproviders;
    dst->name = from_ptrv(src->name);
    dst->outputs = from_ptrv(src->outputs);
    dst->noutputs = src->noutputs;
    dst->crtcs = from_ptrv(src->crtcs);
    dst->ncrtcs = src->ncrtcs;
    dst->capabilities = src->capabilities;
    for(int i=dst->ncrtcs-1; i>=0; --i)
        dst->crtcs[i] = from_ulong(((ulong_t*)dst->crtcs)[i]);
    for(int i=dst->noutputs-1; i>=0; --i)
        dst->outputs[i] = from_ulong(((ulong_t*)dst->outputs)[i]);
    for(int i=dst->nassociatedproviders-1; i>=0; --i)
        dst->associated_providers[i] = from_ulong(((ulong_t*)dst->associated_providers)[i]);
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

void inplace_XRRProviderResources_shrink(void* a)
{
    if(!a) return;
    my_XRRProviderResources_32_t* dst = a;
    my_XRRProviderResources_t* src = a;

    for(int i=0; i<src->nproviders; ++i)
        ((ulong_t*)src->providers)[i] = to_ulong(src->providers[i]);
    dst->timestamp = to_long(src->timestamp);
    dst->nproviders = src->nproviders;
    dst->providers = to_ptrv(src->providers);
}
void inplace_XRRProviderResources_enlarge(void* a)
{
    if(!a) return;
    my_XRRProviderResources_t* dst = a;
    my_XRRProviderResources_32_t* src = a;

    dst->timestamp = from_long(src->timestamp);
    dst->nproviders = src->nproviders;
    dst->providers = from_ptrv(src->providers);
    for(int i=dst->nproviders-1; i>=0; --i)
        dst->providers[i] = from_ulong(((ulong_t*)dst->providers)[i]);
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


#ifdef ANDROID
#define NEEDED_LIBS "libX11.so", "libXext.so", "libXrender.so"
#else
#define NEEDED_LIBS "libX11.so.6", "libXext.so.6", "libXrender.so.1"
#endif

#include "wrappedlib_init32.h"
