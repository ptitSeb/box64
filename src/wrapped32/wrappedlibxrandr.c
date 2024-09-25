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
    dst->noutput = src->noutput;
    for(int i=0; i<dst->noutput; ++i)
        ((XID_32*)src->outputs)[i] = to_ulong(src->outputs[i]);
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

#if 0
#ifdef ANDROID
#define NEEDED_LIBS "libX11.so", "libXext.so", "libXrender.so"
#else
#define NEEDED_LIBS "libX11.so.6", "libXext.so.6", "libXrender.so.1"
#endif
#endif
#include "wrappedlib_init32.h"
