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

#include "wrappedd3dadapter9_vtable.h"

static int (*my_CreateAuthenticatedChannel_real)(void* This, int ChannelType, void*** ppAuthenticatedChannel, void* pChannelHandle);
static int my_CreateAuthenticatedChannel(x64emu_t* emu, void* This, int ChannelType, void*** ppAuthenticatedChannel, void* pChannelHandle);
static int (*my_CreateCryptoSession_real)(void* This, const void* pCryptoType, const void* pDecodeProfile, void*** ppCryptoSession, void* pCryptoHandle);
static int my_CreateCryptoSession(x64emu_t* emu, void* This, const void* pCryptoType, const void* pDecodeProfile, void*** ppCryptoSession, void* pCryptoHandle);
static int (*my_CreateCubeTexture_real)(void* This, unsigned EdgeLength, unsigned Levels, unsigned Usage, int Format, int Pool, void*** ppCubeTexture, void* pSharedHandle);
static int my_CreateCubeTexture(x64emu_t* emu, void* This, unsigned EdgeLength, unsigned Levels, unsigned Usage, int Format, int Pool, void*** ppCubeTexture, void* pSharedHandle);
static int (*my_CreateIndexBuffer_real)(void* This, unsigned Length, unsigned Usage, int Format, int Pool, void*** ppIndexBuffer, void* pSharedHandle);
static int my_CreateIndexBuffer(x64emu_t* emu, void* This, unsigned Length, unsigned Usage, int Format, int Pool, void*** ppIndexBuffer, void* pSharedHandle);
static int (*my_CreatePixelShader_real)(void* This, const void* pFunction, void*** ppShader);
static int my_CreatePixelShader(x64emu_t* emu, void* This, const void* pFunction, void*** ppShader);
static int (*my_CreateQuery_real)(void* This, int Type, void*** ppQuery);
static int my_CreateQuery(x64emu_t* emu, void* This, int Type, void*** ppQuery);
static int (*my_CheckResourceResidency_real)(void* This, void*** pResourceArray, unsigned NumResources);
static int my_CheckResourceResidency(x64emu_t* emu, void* This, void*** pResourceArray, unsigned NumResources);
static int (*my_CreateStateBlock_real)(void* This, int Type, void*** ppSB);
static int my_CreateStateBlock(x64emu_t* emu, void* This, int Type, void*** ppSB);
static int (*my_EndStateBlock_real)(void* This, void*** ppSB);
static int my_EndStateBlock(x64emu_t* emu, void* This, void*** ppSB);
static int (*my_CreateDepthStencilSurfaceEx_real)(void* This, unsigned Width, unsigned Height, int Format, int MultiSample, unsigned MultisampleQuality, int Discard, void*** ppSurface, void* pSharedHandle, unsigned Usage);
static int my_CreateDepthStencilSurfaceEx(x64emu_t* emu, void* This, unsigned Width, unsigned Height, int Format, int MultiSample, unsigned MultisampleQuality, int Discard, void*** ppSurface, void* pSharedHandle, unsigned Usage);
static int (*my_CreateDepthStencilSurface_real)(void* This, unsigned Width, unsigned Height, int Format, int MultiSample, unsigned MultisampleQuality, int Discard, void*** ppSurface, void* pSharedHandle);
static int my_CreateDepthStencilSurface(x64emu_t* emu, void* This, unsigned Width, unsigned Height, int Format, int MultiSample, unsigned MultisampleQuality, int Discard, void*** ppSurface, void* pSharedHandle);
static int (*my_CreateOffscreenPlainSurfaceEx_real)(void* This, unsigned Width, unsigned Height, int Format, int Pool, void*** ppSurface, void* pSharedHandle, unsigned Usage);
static int my_CreateOffscreenPlainSurfaceEx(x64emu_t* emu, void* This, unsigned Width, unsigned Height, int Format, int Pool, void*** ppSurface, void* pSharedHandle, unsigned Usage);
static int (*my_CreateOffscreenPlainSurface_real)(void* This, unsigned Width, unsigned Height, int Format, int Pool, void*** ppSurface, void* pSharedHandle);
static int my_CreateOffscreenPlainSurface(x64emu_t* emu, void* This, unsigned Width, unsigned Height, int Format, int Pool, void*** ppSurface, void* pSharedHandle);
static int (*my_CreateRenderTargetEx_real)(void* This, unsigned Width, unsigned Height, int Format, int MultiSample, unsigned MultisampleQuality, int Lockable, void*** ppSurface, void* pSharedHandle, unsigned Usage);
static int my_CreateRenderTargetEx(x64emu_t* emu, void* This, unsigned Width, unsigned Height, int Format, int MultiSample, unsigned MultisampleQuality, int Lockable, void*** ppSurface, void* pSharedHandle, unsigned Usage);
static int (*my_CreateRenderTarget_real)(void* This, unsigned Width, unsigned Height, int Format, int MultiSample, unsigned MultisampleQuality, int Lockable, void*** ppSurface, void* pSharedHandle);
static int my_CreateRenderTarget(x64emu_t* emu, void* This, unsigned Width, unsigned Height, int Format, int MultiSample, unsigned MultisampleQuality, int Lockable, void*** ppSurface, void* pSharedHandle);
static int (*my_GetCubeMapSurface_real)(void* This, int FaceType, unsigned Level, void*** ppCubeMapSurface);
static int my_GetCubeMapSurface(x64emu_t* emu, void* This, int FaceType, unsigned Level, void*** ppCubeMapSurface);
static int (*my_GetDepthStencilSurface_real)(void* This, void*** ppZStencilSurface);
static int my_GetDepthStencilSurface(x64emu_t* emu, void* This, void*** ppZStencilSurface);
static int (*my_GetBackBuffer_real)(void* This, unsigned iSwapChain, unsigned iBackBuffer, int Type, void*** ppBackBuffer);
static int my_GetBackBuffer(x64emu_t* emu, void* This, unsigned iSwapChain, unsigned iBackBuffer, int Type, void*** ppBackBuffer);
static int (*my_GetRenderTarget_real)(void* This, unsigned RenderTargetIndex, void*** ppRenderTarget);
static int my_GetRenderTarget(x64emu_t* emu, void* This, unsigned RenderTargetIndex, void*** ppRenderTarget);
static int (*my_GetSurfaceLevel_real)(void* This, unsigned Level, void*** ppSurfaceLevel);
static int my_GetSurfaceLevel(x64emu_t* emu, void* This, unsigned Level, void*** ppSurfaceLevel);
static int (*my_CreateAdditionalSwapChain_real)(void* This, void* pPresentationParameters, void*** pSwapChain);
static int my_CreateAdditionalSwapChain(x64emu_t* emu, void* This, void* pPresentationParameters, void*** pSwapChain);
static int (*my_GetSwapChain_real)(void* This, unsigned iSwapChain, void*** pSwapChain);
static int my_GetSwapChain(x64emu_t* emu, void* This, unsigned iSwapChain, void*** pSwapChain);
static int (*my_CreateTexture_real)(void* This, unsigned Width, unsigned Height, unsigned Levels, unsigned Usage, int Format, int Pool, void*** ppTexture, void* pSharedHandle);
static int my_CreateTexture(x64emu_t* emu, void* This, unsigned Width, unsigned Height, unsigned Levels, unsigned Usage, int Format, int Pool, void*** ppTexture, void* pSharedHandle);
static int (*my_CreateVertexBuffer_real)(void* This, unsigned Length, unsigned Usage, unsigned FVF, int Pool, void*** ppVertexBuffer, void* pSharedHandle);
static int my_CreateVertexBuffer(x64emu_t* emu, void* This, unsigned Length, unsigned Usage, unsigned FVF, int Pool, void*** ppVertexBuffer, void* pSharedHandle);
static int (*my_CreateVertexDeclaration_real)(void* This, const void* pVertexElements, void*** ppDecl);
static int my_CreateVertexDeclaration(x64emu_t* emu, void* This, const void* pVertexElements, void*** ppDecl);
static int (*my_CreateVertexShader_real)(void* This, const void* pFunction, void*** ppShader);
static int my_CreateVertexShader(x64emu_t* emu, void* This, const void* pFunction, void*** ppShader);
static int (*my_GetVolumeLevel_real)(void* This, unsigned Level, void*** ppVolumeLevel);
static int my_GetVolumeLevel(x64emu_t* emu, void* This, unsigned Level, void*** ppVolumeLevel);
static int (*my_CreateVolumeTexture_real)(void* This, unsigned Width, unsigned Height, unsigned Depth, unsigned Levels, unsigned Usage, int Format, int Pool, void*** ppVolumeTexture, void* pSharedHandle);
static int my_CreateVolumeTexture(x64emu_t* emu, void* This, unsigned Width, unsigned Height, unsigned Depth, unsigned Levels, unsigned Usage, int Format, int Pool, void*** ppVolumeTexture, void* pSharedHandle);

static int (*my_GetDirect3D_real)(void* This, void*** ppD3D9);
static int my_GetDirect3D(x64emu_t* emu, void* This, void*** ppD3D9);

#define UNPACK(...) __VA_ARGS__

#define GO(name, type) \
    new->name = (void*)AddCheckBridge(emu->context->system, type, real->name, 0, #name)

#define GOM(name, type) \
    my_##name##_real = (void*)real->name; \
    new->name = (void*)AddCheckBridge(emu->context->system, type, my_##name, 0, "my_" #name)

#define GO2(name, type) \
    my_##name##_real2 = (void*)real->name; \
    new->name = (void*)AddCheckBridge(emu->context->system, type, my_##name2, 0, "my_" #name)

#include "wrappedd3dadapter9_gen.h"

#undef GO
#undef GOM
#undef GO2


const char* d3dadapter9Name = "d3dadapter9.so.1";
#define LIBNAME d3dadapter9
#define ALTNAME "/usr/lib/aarch64-linux-gnu/d3d/d3dadapter9.so.1"

typedef void* (*pFp_t)(void*);

typedef struct my_D3DAdapter9DRM {
    uint32_t major_version;
    uint32_t minor_version;
    void *create_adapter;
} my_D3DAdapter9DRM;

typedef struct d3d_my_s {
    pFp_t D3DAdapter9GetProc;
    my_D3DAdapter9DRM adapter;
    int (*create_adapter)(int, ID3DAdapter9Vtbl ***);

    int (*CreateDevice)(void*, unsigned, int, void*, unsigned, void*, void*, void*, IDirect3DDevice9Vtbl ***);
    int (*CreateDeviceEx)(void*, unsigned, int, void*, unsigned, void*, void*, void*, void*, IDirect3DDevice9ExVtbl ***);

    int presentgroup_init;

    struct {
        ID3DAdapter9Vtbl adapter;
        int adapter_init;

        IDirect3DDevice9ExVtbl device;
        int device_init;

        IDirect3DAuthenticatedChannel9Vtbl my_IDirect3DAuthenticatedChannel9Vtbl;
        int my_IDirect3DAuthenticatedChannel9Vtbl_init;
        IDirect3DCryptoSession9Vtbl my_IDirect3DCryptoSession9Vtbl;
        int my_IDirect3DCryptoSession9Vtbl_init;
        IDirect3DCubeTexture9Vtbl my_IDirect3DCubeTexture9Vtbl;
        int my_IDirect3DCubeTexture9Vtbl_init;
        IDirect3DIndexBuffer9Vtbl my_IDirect3DIndexBuffer9Vtbl;
        int my_IDirect3DIndexBuffer9Vtbl_init;
        IDirect3DPixelShader9Vtbl my_IDirect3DPixelShader9Vtbl;
        int my_IDirect3DPixelShader9Vtbl_init;
        IDirect3DQuery9Vtbl my_IDirect3DQuery9Vtbl;
        int my_IDirect3DQuery9Vtbl_init;
        IDirect3DResource9Vtbl my_IDirect3DResource9Vtbl;
        int my_IDirect3DResource9Vtbl_init;
        IDirect3DStateBlock9Vtbl my_IDirect3DStateBlock9Vtbl;
        int my_IDirect3DStateBlock9Vtbl_init;
        IDirect3DSurface9Vtbl my_IDirect3DSurface9Vtbl;
        int my_IDirect3DSurface9Vtbl_init;
        IDirect3DSwapChain9Vtbl my_IDirect3DSwapChain9Vtbl;
        int my_IDirect3DSwapChain9Vtbl_init;
        IDirect3DTexture9Vtbl my_IDirect3DTexture9Vtbl;
        int my_IDirect3DTexture9Vtbl_init;
        IDirect3DVertexBuffer9Vtbl my_IDirect3DVertexBuffer9Vtbl;
        int my_IDirect3DVertexBuffer9Vtbl_init;
        IDirect3DVertexDeclaration9Vtbl my_IDirect3DVertexDeclaration9Vtbl;
        int my_IDirect3DVertexDeclaration9Vtbl_init;
        IDirect3DVertexShader9Vtbl my_IDirect3DVertexShader9Vtbl;
        int my_IDirect3DVertexShader9Vtbl_init;
        IDirect3DVolume9Vtbl my_IDirect3DVolume9Vtbl;
        int my_IDirect3DVolume9Vtbl_init;
        IDirect3DVolumeTexture9Vtbl my_IDirect3DVolumeTexture9Vtbl;
        int my_IDirect3DVolumeTexture9Vtbl_init;
    } vtables;
} d3d_my_t;

static d3d_my_t my_d3d = {0};
static d3d_my_t * const my = &my_d3d;

#define GOR(retcnd, name, args, call, ret, vtbl, a) \
    static ret name(UNPACK args) \
    { \
        int r = name##_real(UNPACK call); \
        if (retcnd) return r; \
        if (!my->vtables.my_##vtbl##_init) { \
            make_vtable_##vtbl(emu, &my->vtables.my_##vtbl, **a); \
            my->vtables.my_##vtbl##_init = 1; \
        } \
        (*a)[0] = &my->vtables.my_##vtbl; \
        (*a)[1] = &my->vtables.my_##vtbl; \
        return 0; \
    }

#define GO(name, args, call, ret, vtbl, a) \
    GOR(r, name, args, call, ret, vtbl, a)

GO(my_CreateAuthenticatedChannel, (x64emu_t* emu, void* This, int ChannelType, void*** ppAuthenticatedChannel, void* pChannelHandle), (This, ChannelType, ppAuthenticatedChannel, pChannelHandle), int, IDirect3DAuthenticatedChannel9Vtbl, ppAuthenticatedChannel)
GO(my_CreateCryptoSession, (x64emu_t* emu, void* This, const void* pCryptoType, const void* pDecodeProfile, void*** ppCryptoSession, void* pCryptoHandle), (This, pCryptoType, pDecodeProfile, ppCryptoSession, pCryptoHandle), int, IDirect3DCryptoSession9Vtbl, ppCryptoSession)
GO(my_CreateCubeTexture, (x64emu_t* emu, void* This, unsigned EdgeLength, unsigned Levels, unsigned Usage, int Format, int Pool, void*** ppCubeTexture, void* pSharedHandle), (This, EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle), int, IDirect3DCubeTexture9Vtbl, ppCubeTexture)
GO(my_CreateIndexBuffer, (x64emu_t* emu, void* This, unsigned Length, unsigned Usage, int Format, int Pool, void*** ppIndexBuffer, void* pSharedHandle), (This, Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle), int, IDirect3DIndexBuffer9Vtbl, ppIndexBuffer)
GO(my_CreatePixelShader, (x64emu_t* emu, void* This, const void* pFunction, void*** ppShader), (This, pFunction, ppShader), int, IDirect3DPixelShader9Vtbl, ppShader)
GO(my_CheckResourceResidency, (x64emu_t* emu, void* This, void*** pResourceArray, unsigned NumResources), (This, pResourceArray, NumResources), int, IDirect3DResource9Vtbl, pResourceArray)
GO(my_CreateStateBlock, (x64emu_t* emu, void* This, int Type, void*** ppSB), (This, Type, ppSB), int, IDirect3DStateBlock9Vtbl, ppSB)
GO(my_EndStateBlock, (x64emu_t* emu, void* This, void*** ppSB), (This, ppSB), int, IDirect3DStateBlock9Vtbl, ppSB)
GO(my_CreateDepthStencilSurfaceEx, (x64emu_t* emu, void* This, unsigned Width, unsigned Height, int Format, int MultiSample, unsigned MultisampleQuality, int Discard, void*** ppSurface, void* pSharedHandle, unsigned Usage), (This, Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle, Usage), int, IDirect3DSurface9Vtbl, ppSurface)
GO(my_CreateDepthStencilSurface, (x64emu_t* emu, void* This, unsigned Width, unsigned Height, int Format, int MultiSample, unsigned MultisampleQuality, int Discard, void*** ppSurface, void* pSharedHandle), (This, Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle), int, IDirect3DSurface9Vtbl, ppSurface)
GO(my_CreateOffscreenPlainSurfaceEx, (x64emu_t* emu, void* This, unsigned Width, unsigned Height, int Format, int Pool, void*** ppSurface, void* pSharedHandle, unsigned Usage), (This, Width, Height, Format, Pool, ppSurface, pSharedHandle, Usage), int, IDirect3DSurface9Vtbl, ppSurface)
GO(my_CreateOffscreenPlainSurface, (x64emu_t* emu, void* This, unsigned Width, unsigned Height, int Format, int Pool, void*** ppSurface, void* pSharedHandle), (This, Width, Height, Format, Pool, ppSurface, pSharedHandle), int, IDirect3DSurface9Vtbl, ppSurface)
GO(my_CreateRenderTargetEx, (x64emu_t* emu, void* This, unsigned Width, unsigned Height, int Format, int MultiSample, unsigned MultisampleQuality, int Lockable, void*** ppSurface, void* pSharedHandle, unsigned Usage), (This, Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle, Usage), int, IDirect3DSurface9Vtbl, ppSurface)
GO(my_CreateRenderTarget, (x64emu_t* emu, void* This, unsigned Width, unsigned Height, int Format, int MultiSample, unsigned MultisampleQuality, int Lockable, void*** ppSurface, void* pSharedHandle), (This, Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle), int, IDirect3DSurface9Vtbl, ppSurface)
GO(my_GetCubeMapSurface, (x64emu_t* emu, void* This, int FaceType, unsigned Level, void*** ppCubeMapSurface), (This, FaceType, Level, ppCubeMapSurface), int, IDirect3DSurface9Vtbl, ppCubeMapSurface)
GO(my_GetDepthStencilSurface, (x64emu_t* emu, void* This, void*** ppZStencilSurface), (This, ppZStencilSurface), int, IDirect3DSurface9Vtbl, ppZStencilSurface)
GO(my_GetBackBuffer, (x64emu_t* emu, void* This, unsigned iSwapChain, unsigned iBackBuffer, int Type, void*** ppBackBuffer), (This, iSwapChain, iBackBuffer, Type, ppBackBuffer), int, IDirect3DSurface9Vtbl, ppBackBuffer)
GO(my_GetRenderTarget, (x64emu_t* emu, void* This, unsigned RenderTargetIndex, void*** ppRenderTarget), (This, RenderTargetIndex, ppRenderTarget), int, IDirect3DSurface9Vtbl, ppRenderTarget)
GO(my_GetSurfaceLevel, (x64emu_t* emu, void* This, unsigned Level, void*** ppSurfaceLevel), (This, Level, ppSurfaceLevel), int, IDirect3DSurface9Vtbl, ppSurfaceLevel)
GO(my_CreateAdditionalSwapChain, (x64emu_t* emu, void* This, void* pPresentationParameters, void*** pSwapChain), (This, pPresentationParameters, pSwapChain), int, IDirect3DSwapChain9Vtbl, pSwapChain)
GO(my_GetSwapChain, (x64emu_t* emu, void* This, unsigned iSwapChain, void*** pSwapChain), (This, iSwapChain, pSwapChain), int, IDirect3DSwapChain9Vtbl, pSwapChain)
GO(my_CreateTexture, (x64emu_t* emu, void* This, unsigned Width, unsigned Height, unsigned Levels, unsigned Usage, int Format, int Pool, void*** ppTexture, void* pSharedHandle), (This, Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle), int, IDirect3DTexture9Vtbl, ppTexture)
GO(my_CreateVertexBuffer, (x64emu_t* emu, void* This, unsigned Length, unsigned Usage, unsigned FVF, int Pool, void*** ppVertexBuffer, void* pSharedHandle), (This, Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle), int, IDirect3DVertexBuffer9Vtbl, ppVertexBuffer)
GO(my_CreateVertexDeclaration, (x64emu_t* emu, void* This, const void* pVertexElements, void*** ppDecl), (This, pVertexElements, ppDecl), int, IDirect3DVertexDeclaration9Vtbl, ppDecl)
GO(my_CreateVertexShader, (x64emu_t* emu, void* This, const void* pFunction, void*** ppShader), (This, pFunction, ppShader), int, IDirect3DVertexShader9Vtbl, ppShader)
GO(my_GetVolumeLevel, (x64emu_t* emu, void* This, unsigned Level, void*** ppVolumeLevel), (This, Level, ppVolumeLevel), int, IDirect3DVolume9Vtbl, ppVolumeLevel)
GO(my_CreateVolumeTexture, (x64emu_t* emu, void* This, unsigned Width, unsigned Height, unsigned Depth, unsigned Levels, unsigned Usage, int Format, int Pool, void*** ppVolumeTexture, void* pSharedHandle), (This, Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle), int, IDirect3DVolumeTexture9Vtbl, ppVolumeTexture)

GOR((r || !ppQuery), my_CreateQuery, (x64emu_t* emu, void* This, int Type, void*** ppQuery), (This, Type, ppQuery), int, IDirect3DQuery9Vtbl, ppQuery)

#undef GO
#undef GOR

static void getMy(library_t* lib)
{
    my->D3DAdapter9GetProc = (pFp_t)dlsym(lib->w.lib, "D3DAdapter9GetProc");
}

static void freeMy()
{
}

#define GOV(ns, ret, fn, args, call) \
    static uintptr_t my_##ns##_##fn##_fct = 0; \
    static ret my_##ns##_##fn(UNPACK args) { \
        ret r = (ret)RunFunctionWindows(my_##ns##_##fn##_fct, UNPACK call); \
/* no closing brace */

#define GOV_1(ns, ret, fn, t1) \
    GOV(ns, ret, fn, (t1 a), (1, a)) return r; }
#define GOV_2(ns, ret, fn, t1, t2) \
    GOV(ns, ret, fn, (t1 a, t2 b), (2, a, b)) return r; }
#define GOV_3(ns, ret, fn, t1, t2, t3) \
    GOV(ns, ret, fn, (t1 a, t2 b, t3 c), (3, a, b, c)) return r; }
#define GOV_4(ns, ret, fn, t1, t2, t3, t4) \
    GOV(ns, ret, fn, (t1 a, t2 b, t3 c, t4 d), (4, a, b, c, d)) return r; }
#define GOV_5(ns, ret, fn, t1, t2, t3, t4, t5) \
    GOV(ns, ret, fn, (t1 a, t2 b, t3 c, t4 d, t5 e), (5, a, b, c, d, e)) return r; }
#define GOV_6(ns, ret, fn, t1, t2, t3, t4, t5, t6) \
    GOV(ns, ret, fn, (t1 a, t2 b, t3 c, t4 d, t5 e, t6 f), (6, a, b, c, d, e, f)) return r; }
#define GOV_7(ns, ret, fn, t1, t2, t3, t4, t5, t6, t7) \
    GOV(ns, ret, fn, (t1 a, t2 b, t3 c, t4 d, t5 e, t6 f, t7 g), (7, a, b, c, d, e, f, g)) return r; }
#define GOV_8(ns, ret, fn, t1, t2, t3, t4, t5, t6, t7, t8) \
    GOV(ns, ret, fn, (t1 a, t2 b, t3 c, t4 d, t5 e, t6 f, t7 g, t8 h), (8, a, b, c, d, e, f, g, h)) return r; }

#define GOS(ns, ret, fn, ...) \
    my_##ns##_##fn##_fct = (uintptr_t)vtbl->fn; \
    vtbl->fn = my_##ns##_##fn;

#define SUPER(ns, X1, X2, X3, X4, X5, X6, X7, X8) \
        X3(ns, int, QueryInterface, void*, void*, void**) \
        X1(ns, unsigned, AddRef, void*) \
        X1(ns, unsigned, Release, void*) \
        X3(ns, int, SetPresentParameters, void*, void*, void*) \
        X8(ns, int, NewD3DWindowBufferFromDmaBuf, void*, int, int, int, int, int, int, void**) \
        X2(ns, int, DestroyD3DWindowBuffer, void*, void*) \
        X2(ns, int, WaitBufferReleased, void*, void*) \
        X2(ns, int, FrontBufferCopy, void*, void*) \
        X7(ns, int, PresentBuffer, void*, void*, void*, const void*, const void*, const void*, unsigned) \
        X2(ns, int, GetRasterStatus, void*, void*) \
        X3(ns, int, GetDisplayMode, void*, void*, void*) \
        X2(ns, int, GetPresentStats, void*, void*) \
        X2(ns, int, GetCursorPos, void*, void*) \
        X2(ns, int, SetCursorPos, void*, void*) \
        X4(ns, int, SetCursor, void*, void*, void*, int) \
        X3(ns, int, SetGammaRamp, void*, const void*, void*) \
        X5(ns, int, GetWindowInfo, void*, void*, void*, void*, void*) \
        X1(ns, int, GetWindowOccluded, void*) \
        X1(ns, int, ResolutionMismatch, void*) \
        X3(ns, void*, CreateThread, void*, void*, void*) \
        X2(ns, int, WaitForThread, void*, void*) \
        X2(ns, int, SetPresentParameters2, void*, void*) \
        X2(ns, int, IsBufferReleased, void*, void*) \
        X1(ns, int, WaitBufferReleaseEvent, void*) \

SUPER(Present, GOV_1, GOV_2, GOV_3, GOV_4, GOV_5, GOV_6, GOV_7, GOV_8)

static void fixup_PresentVtbl(ID3DPresentVtbl *vtbl)
{
    SUPER(Present, GOS, GOS, GOS, GOS, GOS, GOS, GOS, GOS)
}
#undef SUPER

static int pres_init = 0;

#define GOV_PRES(ns, ret, fn, t1, t2, t3) \
        GOV(ns, ret, fn, (t1 a, t2 b, t3 c), (3, a, b, c)) \
        ID3DPresentVtbl*** vtbl = (void*)c; \
        if (!pres_init) fixup_PresentVtbl(**vtbl); \
        pres_init = 1; \
        return r; \
    }

#define SUPER(ns, X1, X2, X3, XPRES) \
        X3(ns, int, QueryInterface, void*, void*, void**) \
        X1(ns, unsigned, AddRef, void*) \
        X1(ns, unsigned, Release, void*) \
        X1(ns, unsigned, GetMultiheadCount, void*) \
        XPRES(ns, int, GetPresent, void*, unsigned, void**) \
        X3(ns, int, CreateAdditionalPresent, void*, void*, void**) \
        X3(ns, /*void*/ int, GetVersion, void*, void*, void*) \

SUPER(PresentGroup, GOV_1, GOV_2, GOV_3, GOV_PRES)

static void fixup_PresentGroupVtbl(ID3DPresentGroupVtbl *vtbl)
{
    SUPER(PresentGroup, GOS, GOS, GOS, GOS)
}
#undef SUPER
#undef GOS

typedef struct my_Direct3D9 {
        IDirect3D9Vtbl *vtbl;
        IDirect3D9Vtbl **real;
} my_Direct3D9;

unsigned my_Direct3D9_AddRef(void *This)
{
    my_Direct3D9 *my = This;
    return RunFunctionWindows((uintptr_t)(*my->real)->AddRef, 1, my->real);
}

unsigned my_Direct3D9_Release(void *This)
{
    my_Direct3D9 *my = This;
    return RunFunctionWindows((uintptr_t)(*my->real)->Release, 1, my->real);
}

IDirect3D9Vtbl my_Direct3D9_vtbl = {
    .AddRef = my_Direct3D9_AddRef,
    .Release = my_Direct3D9_Release,
};

typedef struct my_Direct3D9Ex {
        IDirect3D9ExVtbl *vtbl;
        IDirect3D9ExVtbl **real;
} my_Direct3D9Ex;

unsigned my_Direct3D9Ex_AddRef(void *This)
{
    my_Direct3D9Ex *my = This;
    return RunFunctionFmt((uintptr_t)(*my->real)->AddRef, "p", my->real);
}

unsigned my_Direct3D9Ex_Release(void *This)
{
    my_Direct3D9Ex *my = This;
    return RunFunctionFmt((uintptr_t)(*my->real)->Release, "p", my->real);
}

IDirect3D9ExVtbl my_Direct3D9Ex_vtbl = {
    .AddRef = my_Direct3D9Ex_AddRef,
    .Release = my_Direct3D9Ex_Release,
};


static int my_GetDirect3D(x64emu_t* emu, void* This, void*** ppD3D9)
{
    int r = my_GetDirect3D_real(This, ppD3D9);
    if (r) return r;
    *ppD3D9 = (void**)((my_Direct3D9*)*ppD3D9)->real;
    return 0;
}

int my_create_device(x64emu_t* emu, void *This, unsigned RealAdapter, int DeviceType, void *hFocusWindow, unsigned BehaviorFlags, void *pPresent, IDirect3D9Vtbl **pD3D9, ID3DPresentGroupVtbl **pPresentationFactory, IDirect3DDevice9Vtbl ***ppReturnedDeviceInterface)
{
    my_Direct3D9 *my_pD3D9 = malloc(sizeof(my_Direct3D9));

    my_pD3D9->vtbl = &my_Direct3D9_vtbl;
    my_pD3D9->real = pD3D9;

    if (!my->presentgroup_init) {
        fixup_PresentGroupVtbl(*pPresentationFactory);
        my->presentgroup_init = 1;
    }

    IDirect3DDevice9Vtbl **ret;
    int r = my->CreateDevice(This, RealAdapter, DeviceType, hFocusWindow, BehaviorFlags, pPresent, my_pD3D9, pPresentationFactory, &ret);

    if (r) return r;

    if (!my->vtables.device_init) {
        make_vtable_IDirect3DDevice9Vtbl(emu, (IDirect3DDevice9Vtbl*)&my->vtables.device, *ret);
        my->vtables.device_init = 1;
    }

    ret[0] = (IDirect3DDevice9Vtbl*)&my->vtables.device;
    ret[1] = (IDirect3DDevice9Vtbl*)&my->vtables.device;
    *ppReturnedDeviceInterface = ret;

    return 0;
}

int my_create_device_ex(x64emu_t* emu, void *This, unsigned RealAdapter, int DeviceType, void *hFocusWindow, unsigned BehaviorFlags, void *pPresent, void *pFullscreenDisplayMode, IDirect3D9ExVtbl **pD3D9Ex, ID3DPresentGroupVtbl **pPresentationFactory, IDirect3DDevice9ExVtbl ***ppReturnedDeviceInterface)
{
    my_Direct3D9Ex *my_pD3D9Ex = malloc(sizeof(my_Direct3D9Ex));

    my_pD3D9Ex->vtbl = &my_Direct3D9Ex_vtbl;
    my_pD3D9Ex->real = pD3D9Ex;

    if (!my->presentgroup_init) {
        fixup_PresentGroupVtbl(*pPresentationFactory);
        my->presentgroup_init = 1;
    }

    IDirect3DDevice9ExVtbl **ret;
    int r = my->CreateDeviceEx(This, RealAdapter, DeviceType, hFocusWindow, BehaviorFlags, pPresent, pFullscreenDisplayMode, my_pD3D9Ex, pPresentationFactory, &ret);

    if (r) return r;

    if (!my->vtables.device_init) {
        make_vtable_IDirect3DDevice9ExVtbl(emu, &my->vtables.device, *ret);
        my->vtables.device_init = 1;
    }

    ret[0] = &my->vtables.device;
    ret[1] = &my->vtables.device;
    *ppReturnedDeviceInterface = ret;

    return 0;
}

int my_create_adapter(x64emu_t* emu, int fd, ID3DAdapter9Vtbl ***x_adapter)
{

    ID3DAdapter9Vtbl **adapter;
    int r = my->create_adapter(fd, &adapter);
    if (r) return r;

    if (!my->vtables.adapter_init) {
        my->CreateDevice = (void*)(*adapter)->CreateDevice;
        (*adapter)->CreateDevice = (void*)my_create_device;

        my->CreateDeviceEx = (void*)(*adapter)->CreateDeviceEx;
        (*adapter)->CreateDeviceEx = (void*)my_create_device_ex;

        make_vtable_ID3DAdapter9Vtbl(emu, &my->vtables.adapter, *adapter);
        my->vtables.adapter_init = 1;
    }

    adapter[0] = &my->vtables.adapter;
    adapter[1] = &my->vtables.adapter;
    *x_adapter = adapter;

    return 0 /* D3D_OK */;
}

EXPORT void* my_D3DAdapter9GetProc(x64emu_t* emu, void *ptr)
{

    /* stdcall, 1st param is in RCX */

    my_D3DAdapter9DRM* adapter = my->D3DAdapter9GetProc("drm");

    my->create_adapter = adapter->create_adapter;

    my->adapter.major_version = adapter->major_version;
    my->adapter.minor_version = adapter->minor_version;
    my->adapter.create_adapter = (void*)AddBridge(emu->context->system, iWEip, my_create_adapter, 0, "my_create_adapter");

    return &my->adapter;
}

#define CUSTOM_INIT \
    getMy(lib);

#define CUSTOM_FINI \
    freeMy();

#include "wrappedlib_init.h"
