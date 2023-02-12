/* Based on include/D3D9/ and include/d3dadapter/ in Mesa */

/*
 * Copyright 2011 Joakim Sindholt <opensource@zhasha.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE. */

static void make_vtable_ID3DAdapter9Vtbl(x64emu_t* emu, ID3DAdapter9Vtbl* new, ID3DAdapter9Vtbl* real)
{
    GO(QueryInterface, iFppp, 3);
    GO(AddRef, uFp, 1);
    GO(Release, uFp, 1);

    GO(GetAdapterIdentifier, iFpup, 3);
    GO(CheckDeviceType, iFpiiii, 5);
    GO(CheckDeviceFormat, iFpiiuii, 6);
    GO(CheckDeviceMultiSampleType, iFpiiiip, 6);
    GO(CheckDepthStencilMatch, iFpiiii, 5);
    GO(CheckDeviceFormatConversion, iFpiii, 4);
    GO(GetDeviceCaps, iFpip, 3);
    GO(CreateDevice, iFEpuipupppp, 9);
    GO(CreateDeviceEx, iFpuipuppppp, 10);
}

static void make_vtable_IDirect3D9Vtbl(x64emu_t* emu, IDirect3D9Vtbl* new, IDirect3D9Vtbl* real)
{
    GO(QueryInterface, iFppp, 3);
    GO(AddRef, uFp, 1);
    GO(Release, uFp, 1);
	
    GO(RegisterSoftwareDevice, iFpp, 2);
    GO(GetAdapterCount, uFp, 1);
    GO(GetAdapterIdentifier, iFpuup, 4);
    GO(GetAdapterModeCount, uFpui, 3);
    GO(EnumAdapterModes, iFpuiup, 5);
    GO(GetAdapterDisplayMode, iFpup, 3);
    GO(CheckDeviceType, iFpuiiii, 6);
    GO(CheckDeviceFormat, iFpuiiuii, 7);
    GO(CheckDeviceMultiSampleType, iFpuiiiip, 7);
    GO(CheckDepthStencilMatch, iFpuiiii, 6);
    GO(CheckDeviceFormatConversion, iFpuiii, 5);
    GO(GetDeviceCaps, iFpuip, 4);
    GO(GetAdapterMonitor, pFpu, 2);
    GO(CreateDevice, iFpuipupp, 7);
}

static void make_vtable_IDirect3D9ExVtbl(x64emu_t* emu, IDirect3D9ExVtbl* new, IDirect3D9ExVtbl* real)
{
    GO(QueryInterface, iFppp, 3);
    GO(AddRef, uFp, 1);
    GO(Release, uFp, 1);
	
    GO(RegisterSoftwareDevice, iFpp, 2);
    GO(GetAdapterCount, uFp, 1);
    GO(GetAdapterIdentifier, iFpuup, 4);
    GO(GetAdapterModeCount, uFpui, 3);
    GO(EnumAdapterModes, iFpuiup, 5);
    GO(GetAdapterDisplayMode, iFpup, 3);
    GO(CheckDeviceType, iFpuiiii, 6);
    GO(CheckDeviceFormat, iFpuiiuii, 7);
    GO(CheckDeviceMultiSampleType, iFpuiiiip, 7);
    GO(CheckDepthStencilMatch, iFpuiiii, 6);
    GO(CheckDeviceFormatConversion, iFpuiii, 5);
    GO(GetDeviceCaps, iFpuip, 4);
    GO(GetAdapterMonitor, pFpu, 2);
    GO(CreateDevice, iFpuipupp, 7);
	
    GO(GetAdapterModeCountEx, uFpup, 3);
    GO(EnumAdapterModesEx, iFpupup, 5);
    GO(GetAdapterDisplayModeEx, iFpupp, 4);
    GO(CreateDeviceEx, iFpuipuppp, 8);
    GO(GetAdapterLUID, iFpup, 3);
}

static void make_vtable_IDirect3D9ExOverlayExtensionVtbl(x64emu_t* emu, IDirect3D9ExOverlayExtensionVtbl* new, IDirect3D9ExOverlayExtensionVtbl* real)
{
    GO(QueryInterface, iFppp, 3);
    GO(AddRef, uFp, 1);
    GO(Release, uFp, 1);
	
    GO(CheckDeviceOverlayType, iFpuiuuipip, 9);
}

static void make_vtable_IDirect3DAuthenticatedChannel9Vtbl(x64emu_t* emu, IDirect3DAuthenticatedChannel9Vtbl* new, IDirect3DAuthenticatedChannel9Vtbl* real)
{
    GO(QueryInterface, iFppp, 3);
    GO(AddRef, uFp, 1);
    GO(Release, uFp, 1);
	
    GO(GetCertificateSize, iFpp, 2);
    GO(GetCertificate, iFpup, 3);
    GO(NegotiateKeyExchange, iFpup, 3);
    GO(Query, iFpupup, 5);
    GO(Configure, iFpupp, 4);
}

static void make_vtable_IDirect3DBaseTexture9Vtbl(x64emu_t* emu, IDirect3DBaseTexture9Vtbl* new, IDirect3DBaseTexture9Vtbl* real)
{
    GO(QueryInterface, iFppp, 3);
    GO(AddRef, uFp, 1);
    GO(Release, uFp, 1);
	
    GO(GetDevice, iFpp, 2);
    GO(SetPrivateData, iFpppuu, 5);
    GO(GetPrivateData, iFpppp, 4);
    GO(FreePrivateData, iFpp, 2);
    GO(SetPriority, uFpu, 2);
    GO(GetPriority, uFp, 1);
    GO(PreLoad, vFp, 1);
    GO(GetType, iFp, 1);
	
    GO(SetLOD, uFpu, 2);
    GO(GetLOD, uFp, 1);
    GO(GetLevelCount, uFp, 1);
    GO(SetAutoGenFilterType, iFpi, 2);
    GO(GetAutoGenFilterType, iFp, 1);
    GO(GenerateMipSubLevels, vFp, 1);
}

static void make_vtable_IDirect3DCryptoSession9Vtbl(x64emu_t* emu, IDirect3DCryptoSession9Vtbl* new, IDirect3DCryptoSession9Vtbl* real)
{
    GO(QueryInterface, iFppp, 3);
    GO(AddRef, uFp, 1);
    GO(Release, uFp, 1);
	
    GO(GetCertificateSize, iFpp, 2);
    GO(GetCertificate, iFpup, 3);
    GO(NegotiateKeyExchange, iFpup, 3);
    GO(EncryptionBlt, iFpppup, 5);
    GO(DecryptionBlt, iFpppuppp, 7);
    GO(GetSurfacePitch, iFppp, 3);
    GO(StartSessionKeyRefresh, iFppu, 3);
    GO(FinishSessionKeyRefresh, iFp, 1);
    GO(GetEncryptionBltKey, iFppu, 3);
}

static void make_vtable_IDirect3DCubeTexture9Vtbl(x64emu_t* emu, IDirect3DCubeTexture9Vtbl* new, IDirect3DCubeTexture9Vtbl* real)
{
    GO(QueryInterface, iFppp, 3);
    GO(AddRef, uFp, 1);
    GO(Release, uFp, 1);
	
    GO(GetDevice, iFpp, 2);
    GO(SetPrivateData, iFpppuu, 5);
    GO(GetPrivateData, iFpppp, 4);
    GO(FreePrivateData, iFpp, 2);
    GO(SetPriority, uFpu, 2);
    GO(GetPriority, uFp, 1);
    GO(PreLoad, vFp, 1);
    GO(GetType, iFp, 1);
	
    GO(SetLOD, uFpu, 2);
    GO(GetLOD, uFp, 1);
    GO(GetLevelCount, uFp, 1);
    GO(SetAutoGenFilterType, iFpi, 2);
    GO(GetAutoGenFilterType, iFp, 1);
    GO(GenerateMipSubLevels, vFp, 1);
	
    GO(GetLevelDesc, iFpup, 3);
    GOM(GetCubeMapSurface, iFEpiup, 4);
    GO(LockRect, iFpiuppu, 6);
    GO(UnlockRect, iFpiu, 3);
    GO(AddDirtyRect, iFpip, 3);
}

static void make_vtable_IDirect3DDevice9Vtbl(x64emu_t* emu, IDirect3DDevice9Vtbl* new, IDirect3DDevice9Vtbl* real)
{
    GO(QueryInterface, iFppp, 3);
    GO(AddRef, uFp, 1);
    GO(Release, uFp, 1);
	
    GO(TestCooperativeLevel, iFp, 1);
    GO(GetAvailableTextureMem, uFp, 1);
    GO(EvictManagedResources, iFp, 1);
    GOM(GetDirect3D, iFEpp, 2);
    GO(GetDeviceCaps, iFpp, 2);
    GO(GetDisplayMode, iFpup, 3);
    GO(GetCreationParameters, iFpp, 2);
    GO(SetCursorProperties, iFpuup, 4);
    GO(SetCursorPosition, vFpiiu, 4);
    GO(ShowCursor, iFpi, 2);
    GOM(CreateAdditionalSwapChain, iFEppp, 3);
    GOM(GetSwapChain, iFEpup, 3);
    GO(GetNumberOfSwapChains, uFp, 1);
    GO(Reset, iFpp, 2);
    GO(Present, iFppppp, 5);
    GOM(GetBackBuffer, iFEpuuip, 5);
    GO(GetRasterStatus, iFpup, 3);
    GO(SetDialogBoxMode, iFpi, 2);
    GO(SetGammaRamp, vFpuup, 4);
    GO(GetGammaRamp, vFpup, 3);
    GOM(CreateTexture, iFEpuuuuiipp, 9);
    GOM(CreateVolumeTexture, iFEpuuuuuiipp, 10);
    GOM(CreateCubeTexture, iFEpuuuiipp, 8);
    GOM(CreateVertexBuffer, iFEpuuuipp, 7);
    GOM(CreateIndexBuffer, iFEpuuiipp, 7);
    GOM(CreateRenderTarget, iFEpuuiiuipp, 9);
    GOM(CreateDepthStencilSurface, iFEpuuiiuipp, 9);
    GO(UpdateSurface, iFppppp, 5);
    GO(UpdateTexture, iFppp, 3);
    GO(GetRenderTargetData, iFppp, 3);
    GO(GetFrontBufferData, iFpup, 3);
    GO(StretchRect, iFpppppi, 6);
    GO(ColorFill, iFpppu, 4);
    GOM(CreateOffscreenPlainSurface, iFEpuuiipp, 7);
    GO(SetRenderTarget, iFpup, 3);
    GOM(GetRenderTarget, iFEpup, 3);
    GO(SetDepthStencilSurface, iFpp, 2);
    GOM(GetDepthStencilSurface, iFEpp, 2);
    GO(BeginScene, iFp, 1);
    GO(EndScene, iFp, 1);
    GO(Clear, iFpupuufu, 7);
    GO(SetTransform, iFpip, 3);
    GO(GetTransform, iFpip, 3);
    GO(MultiplyTransform, iFpip, 3);
    GO(SetViewport, iFpp, 2);
    GO(GetViewport, iFpp, 2);
    GO(SetMaterial, iFpp, 2);
    GO(GetMaterial, iFpp, 2);
    GO(SetLight, iFpup, 3);
    GO(GetLight, iFpup, 3);
    GO(LightEnable, iFpui, 3);
    GO(GetLightEnable, iFpup, 3);
    GO(SetClipPlane, iFpup, 3);
    GO(GetClipPlane, iFpup, 3);
    GO(SetRenderState, iFpiu, 3);
    GO(GetRenderState, iFpip, 3);
    GOM(CreateStateBlock, iFEpip, 3);
    GO(BeginStateBlock, iFp, 1);
    GOM(EndStateBlock, iFEpp, 2);
    GO(SetClipStatus, iFpp, 2);
    GO(GetClipStatus, iFpp, 2);
    GO(GetTexture, iFpup, 3);
    GO(SetTexture, iFpup, 3);
    GO(GetTextureStageState, iFpuip, 4);
    GO(SetTextureStageState, iFpuiu, 4);
    GO(GetSamplerState, iFpuip, 4);
    GO(SetSamplerState, iFpuiu, 4);
    GO(ValidateDevice, iFpp, 2);
    GO(SetPaletteEntries, iFpup, 3);
    GO(GetPaletteEntries, iFpup, 3);
    GO(SetCurrentTexturePalette, iFpu, 2);
    GO(GetCurrentTexturePalette, iFpp, 2);
    GO(SetScissorRect, iFpp, 2);
    GO(GetScissorRect, iFpp, 2);
    GO(SetSoftwareVertexProcessing, iFpi, 2);
    GO(GetSoftwareVertexProcessing, iFp, 1);
    GO(SetNPatchMode, iFpf, 2);
    GO(GetNPatchMode, fFp, 1);
    GO(DrawPrimitive, iFpiuu, 4);
    GO(DrawIndexedPrimitive, iFpiiuuuu, 7);
    GO(DrawPrimitiveUP, iFpiupu, 5);
    GO(DrawIndexedPrimitiveUP, iFpiuuupipu, 9);
    GO(ProcessVertices, iFpuuuppu, 7);
    GOM(CreateVertexDeclaration, iFEppp, 3);
    GO(SetVertexDeclaration, iFpp, 2);
    GO(GetVertexDeclaration, iFpp, 2);
    GO(SetFVF, iFpu, 2);
    GO(GetFVF, iFpp, 2);
    GOM(CreateVertexShader, iFEppp, 3);
    GO(SetVertexShader, iFpp, 2);
    GO(GetVertexShader, iFpp, 2);
    GO(SetVertexShaderConstantF, iFpupu, 4);
    GO(GetVertexShaderConstantF, iFpupu, 4);
    GO(SetVertexShaderConstantI, iFpupu, 4);
    GO(GetVertexShaderConstantI, iFpupu, 4);
    GO(SetVertexShaderConstantB, iFpupu, 4);
    GO(GetVertexShaderConstantB, iFpupu, 4);
    GO(SetStreamSource, iFpupuu, 5);
    GO(GetStreamSource, iFpuppp, 5);
    GO(SetStreamSourceFreq, iFpuu, 3);
    GO(GetStreamSourceFreq, iFpup, 3);
    GO(SetIndices, iFpp, 2);
    GO(GetIndices, iFpp, 2);
    GOM(CreatePixelShader, iFEppp, 3);
    GO(SetPixelShader, iFpp, 2);
    GO(GetPixelShader, iFpp, 2);
    GO(SetPixelShaderConstantF, iFpupu, 4);
    GO(GetPixelShaderConstantF, iFpupu, 4);
    GO(SetPixelShaderConstantI, iFpupu, 4);
    GO(GetPixelShaderConstantI, iFpupu, 4);
    GO(SetPixelShaderConstantB, iFpupu, 4);
    GO(GetPixelShaderConstantB, iFpupu, 4);
    GO(DrawRectPatch, iFpupp, 4);
    GO(DrawTriPatch, iFpupp, 4);
    GO(DeletePatch, iFpu, 2);
    GOM(CreateQuery, iFEpip, 3);
}

static void make_vtable_IDirect3DDevice9ExVtbl(x64emu_t* emu, IDirect3DDevice9ExVtbl* new, IDirect3DDevice9ExVtbl* real)
{
    GO(QueryInterface, iFppp, 3);
    GO(AddRef, uFp, 1);
    GO(Release, uFp, 1);
	
    GO(TestCooperativeLevel, iFp, 1);
    GO(GetAvailableTextureMem, uFp, 1);
    GO(EvictManagedResources, iFp, 1);
    GO(GetDirect3D, iFpp, 2);
    GO(GetDeviceCaps, iFpp, 2);
    GO(GetDisplayMode, iFpup, 3);
    GO(GetCreationParameters, iFpp, 2);
    GO(SetCursorProperties, iFpuup, 4);
    GO(SetCursorPosition, vFpiiu, 4);
    GO(ShowCursor, iFpi, 2);
    GOM(CreateAdditionalSwapChain, iFEppp, 3);
    GOM(GetSwapChain, iFEpup, 3);
    GO(GetNumberOfSwapChains, uFp, 1);
    GO(Reset, iFpp, 2);
    GO(Present, iFppppp, 5);
    GOM(GetBackBuffer, iFEpuuip, 5);
    GO(GetRasterStatus, iFpup, 3);
    GO(SetDialogBoxMode, iFpi, 2);
    GO(SetGammaRamp, vFpuup, 4);
    GO(GetGammaRamp, vFpup, 3);
    GOM(CreateTexture, iFEpuuuuiipp, 9);
    GOM(CreateVolumeTexture, iFEpuuuuuiipp, 10);
    GOM(CreateCubeTexture, iFEpuuuiipp, 8);
    GOM(CreateVertexBuffer, iFEpuuuipp, 7);
    GOM(CreateIndexBuffer, iFEpuuiipp, 7);
    GOM(CreateRenderTarget, iFEpuuiiuipp, 9);
    GOM(CreateDepthStencilSurface, iFEpuuiiuipp, 9);
    GO(UpdateSurface, iFppppp, 5);
    GO(UpdateTexture, iFppp, 3);
    GO(GetRenderTargetData, iFppp, 3);
    GO(GetFrontBufferData, iFpup, 3);
    GO(StretchRect, iFpppppi, 6);
    GO(ColorFill, iFpppu, 4);
    GOM(CreateOffscreenPlainSurface, iFEpuuiipp, 7);
    GO(SetRenderTarget, iFpup, 3);
    GOM(GetRenderTarget, iFEpup, 3);
    GO(SetDepthStencilSurface, iFpp, 2);
    GOM(GetDepthStencilSurface, iFEpp, 2);
    GO(BeginScene, iFp, 1);
    GO(EndScene, iFp, 1);
    GO(Clear, iFpupuufu, 7);
    GO(SetTransform, iFpip, 3);
    GO(GetTransform, iFpip, 3);
    GO(MultiplyTransform, iFpip, 3);
    GO(SetViewport, iFpp, 2);
    GO(GetViewport, iFpp, 2);
    GO(SetMaterial, iFpp, 2);
    GO(GetMaterial, iFpp, 2);
    GO(SetLight, iFpup, 3);
    GO(GetLight, iFpup, 3);
    GO(LightEnable, iFpui, 3);
    GO(GetLightEnable, iFpup, 3);
    GO(SetClipPlane, iFpup, 3);
    GO(GetClipPlane, iFpup, 3);
    GO(SetRenderState, iFpiu, 3);
    GO(GetRenderState, iFpip, 3);
    GOM(CreateStateBlock, iFEpip, 3);
    GO(BeginStateBlock, iFp, 1);
    GOM(EndStateBlock, iFEpp, 2);
    GO(SetClipStatus, iFpp, 2);
    GO(GetClipStatus, iFpp, 2);
    GO(GetTexture, iFpup, 3);
    GO(SetTexture, iFpup, 3);
    GO(GetTextureStageState, iFpuip, 4);
    GO(SetTextureStageState, iFpuiu, 4);
    GO(GetSamplerState, iFpuip, 4);
    GO(SetSamplerState, iFpuiu, 4);
    GO(ValidateDevice, iFpp, 2);
    GO(SetPaletteEntries, iFpup, 3);
    GO(GetPaletteEntries, iFpup, 3);
    GO(SetCurrentTexturePalette, iFpu, 2);
    GO(GetCurrentTexturePalette, iFpp, 2);
    GO(SetScissorRect, iFpp, 2);
    GO(GetScissorRect, iFpp, 2);
    GO(SetSoftwareVertexProcessing, iFpi, 2);
    GO(GetSoftwareVertexProcessing, iFp, 1);
    GO(SetNPatchMode, iFpf, 2);
    GO(GetNPatchMode, fFp, 1);
    GO(DrawPrimitive, iFpiuu, 4);
    GO(DrawIndexedPrimitive, iFpiiuuuu, 7);
    GO(DrawPrimitiveUP, iFpiupu, 5);
    GO(DrawIndexedPrimitiveUP, iFpiuuupipu, 9);
    GO(ProcessVertices, iFpuuuppu, 7);
    GOM(CreateVertexDeclaration, iFEppp, 3);
    GO(SetVertexDeclaration, iFpp, 2);
    GO(GetVertexDeclaration, iFpp, 2);
    GO(SetFVF, iFpu, 2);
    GO(GetFVF, iFpp, 2);
    GOM(CreateVertexShader, iFEppp, 3);
    GO(SetVertexShader, iFpp, 2);
    GO(GetVertexShader, iFpp, 2);
    GO(SetVertexShaderConstantF, iFpupu, 4);
    GO(GetVertexShaderConstantF, iFpupu, 4);
    GO(SetVertexShaderConstantI, iFpupu, 4);
    GO(GetVertexShaderConstantI, iFpupu, 4);
    GO(SetVertexShaderConstantB, iFpupu, 4);
    GO(GetVertexShaderConstantB, iFpupu, 4);
    GO(SetStreamSource, iFpupuu, 5);
    GO(GetStreamSource, iFpuppp, 5);
    GO(SetStreamSourceFreq, iFpuu, 3);
    GO(GetStreamSourceFreq, iFpup, 3);
    GO(SetIndices, iFpp, 2);
    GO(GetIndices, iFpp, 2);
    GOM(CreatePixelShader, iFEppp, 3);
    GO(SetPixelShader, iFpp, 2);
    GO(GetPixelShader, iFpp, 2);
    GO(SetPixelShaderConstantF, iFpupu, 4);
    GO(GetPixelShaderConstantF, iFpupu, 4);
    GO(SetPixelShaderConstantI, iFpupu, 4);
    GO(GetPixelShaderConstantI, iFpupu, 4);
    GO(SetPixelShaderConstantB, iFpupu, 4);
    GO(GetPixelShaderConstantB, iFpupu, 4);
    GO(DrawRectPatch, iFpupp, 4);
    GO(DrawTriPatch, iFpupp, 4);
    GO(DeletePatch, iFpu, 2);
    GOM(CreateQuery, iFEpip, 3);
	
    GO(SetConvolutionMonoKernel, iFpuupp, 5);
    GO(ComposeRects, iFppppupiii, 9);
    GO(PresentEx, iFpppppu, 6);
    GO(GetGPUThreadPriority, iFpp, 2);
    GO(SetGPUThreadPriority, iFpi, 2);
    GO(WaitForVBlank, iFpu, 2);
    GOM(CheckResourceResidency, iFEppu, 3);
    GO(SetMaximumFrameLatency, iFpu, 2);
    GO(GetMaximumFrameLatency, iFpp, 2);
    GO(CheckDeviceState, iFpp, 2);
    GOM(CreateRenderTargetEx, iFEpuuiiuippu, 10);
    GOM(CreateOffscreenPlainSurfaceEx, iFEpuuiippu, 8);
    GOM(CreateDepthStencilSurfaceEx, iFEpuuiiuippu, 10);
    GO(ResetEx, iFppp, 3);
    GO(GetDisplayModeEx, iFpupp, 4);
}

static void make_vtable_IDirect3DDevice9VideoVtbl(x64emu_t* emu, IDirect3DDevice9VideoVtbl* new, IDirect3DDevice9VideoVtbl* real)
{
    GO(QueryInterface, iFppp, 3);
    GO(AddRef, uFp, 1);
    GO(Release, uFp, 1);
	
    GO(GetContentProtectionCaps, iFpppp, 4);
    GOM(CreateAuthenticatedChannel, iFEpipp, 4);
    GOM(CreateCryptoSession, iFEppppp, 5);
}

static void make_vtable_IDirect3DIndexBuffer9Vtbl(x64emu_t* emu, IDirect3DIndexBuffer9Vtbl* new, IDirect3DIndexBuffer9Vtbl* real)
{
    GO(QueryInterface, iFppp, 3);
    GO(AddRef, uFp, 1);
    GO(Release, uFp, 1);
	
    GO(GetDevice, iFpp, 2);
    GO(SetPrivateData, iFpppuu, 5);
    GO(GetPrivateData, iFpppp, 4);
    GO(FreePrivateData, iFpp, 2);
    GO(SetPriority, uFpu, 2);
    GO(GetPriority, uFp, 1);
    GO(PreLoad, vFp, 1);
    GO(GetType, iFp, 1);
	
    GO(Lock, iFpuupu, 5);
    GO(Unlock, iFp, 1);
    GO(GetDesc, iFpp, 2);
}

static void make_vtable_IDirect3DPixelShader9Vtbl(x64emu_t* emu, IDirect3DPixelShader9Vtbl* new, IDirect3DPixelShader9Vtbl* real)
{
    GO(QueryInterface, iFppp, 3);
    GO(AddRef, uFp, 1);
    GO(Release, uFp, 1);
	
    GO(GetDevice, iFpp, 2);
    GO(GetFunction, iFppp, 3);
}

static void make_vtable_IDirect3DQuery9Vtbl(x64emu_t* emu, IDirect3DQuery9Vtbl* new, IDirect3DQuery9Vtbl* real)
{
    GO(QueryInterface, iFppp, 3);
    GO(AddRef, uFp, 1);
    GO(Release, uFp, 1);
	
    GO(GetDevice, iFpp, 2);
    GO(GetType, iFp, 1);
    GO(GetDataSize, uFp, 1);
    GO(Issue, iFpu, 2);
    GO(GetData, iFppuu, 4);
}

static void make_vtable_IDirect3DResource9Vtbl(x64emu_t* emu, IDirect3DResource9Vtbl* new, IDirect3DResource9Vtbl* real)
{
    GO(QueryInterface, iFppp, 3);
    GO(AddRef, uFp, 1);
    GO(Release, uFp, 1);
	
    GO(GetDevice, iFpp, 2);
    GO(SetPrivateData, iFpppuu, 5);
    GO(GetPrivateData, iFpppp, 4);
    GO(FreePrivateData, iFpp, 2);
    GO(SetPriority, uFpu, 2);
    GO(GetPriority, uFp, 1);
    GO(PreLoad, vFp, 1);
    GO(GetType, iFp, 1);
}

static void make_vtable_IDirect3DStateBlock9Vtbl(x64emu_t* emu, IDirect3DStateBlock9Vtbl* new, IDirect3DStateBlock9Vtbl* real)
{
    GO(QueryInterface, iFppp, 3);
    GO(AddRef, uFp, 1);
    GO(Release, uFp, 1);
	
    GO(GetDevice, iFpp, 2);
    GO(Capture, iFp, 1);
    GO(Apply, iFp, 1);
}

static void make_vtable_IDirect3DSurface9Vtbl(x64emu_t* emu, IDirect3DSurface9Vtbl* new, IDirect3DSurface9Vtbl* real)
{
    GO(QueryInterface, iFppp, 3);
    GO(AddRef, uFp, 1);
    GO(Release, uFp, 1);
	
    GO(GetDevice, iFpp, 2);
    GO(SetPrivateData, iFpppuu, 5);
    GO(GetPrivateData, iFpppp, 4);
    GO(FreePrivateData, iFpp, 2);
    GO(SetPriority, uFpu, 2);
    GO(GetPriority, uFp, 1);
    GO(PreLoad, vFp, 1);
    GO(GetType, iFp, 1);
	
    GO(GetContainer, iFppp, 3);
    GO(GetDesc, iFpp, 2);
    GO(LockRect, iFpppu, 4);
    GO(UnlockRect, iFp, 1);
    GO(GetDC, iFpp, 2);
    GO(ReleaseDC, iFpp, 2);
}

static void make_vtable_IDirect3DSwapChain9Vtbl(x64emu_t* emu, IDirect3DSwapChain9Vtbl* new, IDirect3DSwapChain9Vtbl* real)
{
    GO(QueryInterface, iFppp, 3);
    GO(AddRef, uFp, 1);
    GO(Release, uFp, 1);
	
    GO(Present, iFpppppu, 6);
    GO(GetFrontBufferData, iFpp, 2);
    GO(GetBackBuffer, iFpuip, 4); // XXX
    GO(GetRasterStatus, iFpp, 2);
    GO(GetDisplayMode, iFpp, 2);
    GO(GetDevice, iFpp, 2);
    GO(GetPresentParameters, iFpp, 2);
}

static void make_vtable_IDirect3DSwapChain9ExVtbl(x64emu_t* emu, IDirect3DSwapChain9ExVtbl* new, IDirect3DSwapChain9ExVtbl* real)
{
    GO(QueryInterface, iFppp, 3);
    GO(AddRef, uFp, 1);
    GO(Release, uFp, 1);
	
    GO(Present, iFpppppu, 6);
    GO(GetFrontBufferData, iFpp, 2);
    GO(GetBackBuffer, iFpuip, 4); // XXX
    GO(GetRasterStatus, iFpp, 2);
    GO(GetDisplayMode, iFpp, 2);
    GO(GetDevice, iFpp, 2);
    GO(GetPresentParameters, iFpp, 2);
	
    GO(GetLastPresentCount, iFpp, 2);
    GO(GetPresentStats, iFpp, 2);
    GO(GetDisplayModeEx, iFppp, 3);
}

static void make_vtable_IDirect3DTexture9Vtbl(x64emu_t* emu, IDirect3DTexture9Vtbl* new, IDirect3DTexture9Vtbl* real)
{
    GO(QueryInterface, iFppp, 3);
    GO(AddRef, uFp, 1);
    GO(Release, uFp, 1);
	
    GO(GetDevice, iFpp, 2);
    GO(SetPrivateData, iFpppuu, 5);
    GO(GetPrivateData, iFpppp, 4);
    GO(FreePrivateData, iFpp, 2);
    GO(SetPriority, uFpu, 2);
    GO(GetPriority, uFp, 1);
    GO(PreLoad, vFp, 1);
    GO(GetType, iFp, 1);
	
    GO(SetLOD, uFpu, 2);
    GO(GetLOD, uFp, 1);
    GO(GetLevelCount, uFp, 1);
    GO(SetAutoGenFilterType, iFpi, 2);
    GO(GetAutoGenFilterType, iFp, 1);
    GO(GenerateMipSubLevels, vFp, 1);
	
    GO(GetLevelDesc, iFpup, 3);
    GOM(GetSurfaceLevel, iFEpup, 3);
    GO(LockRect, iFpuppu, 5);
    GO(UnlockRect, iFpu, 2);
    GO(AddDirtyRect, iFpp, 2);
}

static void make_vtable_IDirect3DVertexBuffer9Vtbl(x64emu_t* emu, IDirect3DVertexBuffer9Vtbl* new, IDirect3DVertexBuffer9Vtbl* real)
{
    GO(QueryInterface, iFppp, 3);
    GO(AddRef, uFp, 1);
    GO(Release, uFp, 1);
	
    GO(GetDevice, iFpp, 2);
    GO(SetPrivateData, iFpppuu, 5);
    GO(GetPrivateData, iFpppp, 4);
    GO(FreePrivateData, iFpp, 2);
    GO(SetPriority, uFpu, 2);
    GO(GetPriority, uFp, 1);
    GO(PreLoad, vFp, 1);
    GO(GetType, iFp, 1);
	
    GO(Lock, iFpuupu, 5);
    GO(Unlock, iFp, 1);
    GO(GetDesc, iFpp, 2);
}

static void make_vtable_IDirect3DVertexDeclaration9Vtbl(x64emu_t* emu, IDirect3DVertexDeclaration9Vtbl* new, IDirect3DVertexDeclaration9Vtbl* real)
{
    GO(QueryInterface, iFppp, 3);
    GO(AddRef, uFp, 1);
    GO(Release, uFp, 1);
	
    GO(GetDevice, iFpp, 2);
    GO(GetDeclaration, iFppp, 3);
}

static void make_vtable_IDirect3DVertexShader9Vtbl(x64emu_t* emu, IDirect3DVertexShader9Vtbl* new, IDirect3DVertexShader9Vtbl* real)
{
    GO(QueryInterface, iFppp, 3);
    GO(AddRef, uFp, 1);
    GO(Release, uFp, 1);
	
    GO(GetDevice, iFpp, 2);
    GO(GetFunction, iFppp, 3);
}

static void make_vtable_IDirect3DVolume9Vtbl(x64emu_t* emu, IDirect3DVolume9Vtbl* new, IDirect3DVolume9Vtbl* real)
{
    GO(QueryInterface, iFppp, 3);
    GO(AddRef, uFp, 1);
    GO(Release, uFp, 1);
	
    GO(GetDevice, iFpp, 2);
    GO(SetPrivateData, iFpppuu, 5);
    GO(GetPrivateData, iFpppp, 4);
    GO(FreePrivateData, iFpp, 2);
    GO(GetContainer, iFppp, 3);
    GO(GetDesc, iFpp, 2);
    GO(LockBox, iFpppu, 4);
    GO(UnlockBox, iFp, 1);
}

static void make_vtable_IDirect3DVolumeTexture9Vtbl(x64emu_t* emu, IDirect3DVolumeTexture9Vtbl* new, IDirect3DVolumeTexture9Vtbl* real)
{
    GO(QueryInterface, iFppp, 3);
    GO(AddRef, uFp, 1);
    GO(Release, uFp, 1);
	
    GO(GetDevice, iFpp, 2);
    GO(SetPrivateData, iFpppuu, 5);
    GO(GetPrivateData, iFpppp, 4);
    GO(FreePrivateData, iFpp, 2);
    GO(SetPriority, uFpu, 2);
    GO(GetPriority, uFp, 1);
    GO(PreLoad, vFp, 1);
    GO(GetType, iFp, 1);
	
    GO(SetLOD, uFpu, 2);
    GO(GetLOD, uFp, 1);
    GO(GetLevelCount, uFp, 1);
    GO(SetAutoGenFilterType, iFpi, 2);
    GO(GetAutoGenFilterType, iFp, 1);
    GO(GenerateMipSubLevels, vFp, 1);
	
    GO(GetLevelDesc, iFpup, 3);
    GOM(GetVolumeLevel, iFEpup, 3);
    GO(LockBox, iFpuppu, 5);
    GO(UnlockBox, iFpu, 2);
    GO(AddDirtyBox, iFpp, 2);
}
