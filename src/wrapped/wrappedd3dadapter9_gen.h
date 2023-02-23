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
    GO(QueryInterface, iWppp);
    GO(AddRef, uWp);
    GO(Release, uWp);

    GO(GetAdapterIdentifier, iWpup);
    GO(CheckDeviceType, iWpiiii);
    GO(CheckDeviceFormat, iWpiiuii);
    GO(CheckDeviceMultiSampleType, iWpiiiip);
    GO(CheckDepthStencilMatch, iWpiiii);
    GO(CheckDeviceFormatConversion, iWpiii);
    GO(GetDeviceCaps, iWpip);
    GO(CreateDevice, iWEpuipupppp);
    GO(CreateDeviceEx, iWEpuipuppppp);
}

static void make_vtable_IDirect3D9Vtbl(x64emu_t* emu, IDirect3D9Vtbl* new, IDirect3D9Vtbl* real)
{
    GO(QueryInterface, iWppp);
    GO(AddRef, uWp);
    GO(Release, uWp);
	
    GO(RegisterSoftwareDevice, iWpp);
    GO(GetAdapterCount, uWp);
    GO(GetAdapterIdentifier, iWpuup);
    GO(GetAdapterModeCount, uWpui);
    GO(EnumAdapterModes, iWpuiup);
    GO(GetAdapterDisplayMode, iWpup);
    GO(CheckDeviceType, iWpuiiii);
    GO(CheckDeviceFormat, iWpuiiuii);
    GO(CheckDeviceMultiSampleType, iWpuiiiip);
    GO(CheckDepthStencilMatch, iWpuiiii);
    GO(CheckDeviceFormatConversion, iWpuiii);
    GO(GetDeviceCaps, iWpuip);
    GO(GetAdapterMonitor, pWpu);
    GO(CreateDevice, iWpuipupp);
}

static void make_vtable_IDirect3D9ExVtbl(x64emu_t* emu, IDirect3D9ExVtbl* new, IDirect3D9ExVtbl* real)
{
    GO(QueryInterface, iWppp);
    GO(AddRef, uWp);
    GO(Release, uWp);
	
    GO(RegisterSoftwareDevice, iWpp);
    GO(GetAdapterCount, uWp);
    GO(GetAdapterIdentifier, iWpuup);
    GO(GetAdapterModeCount, uWpui);
    GO(EnumAdapterModes, iWpuiup);
    GO(GetAdapterDisplayMode, iWpup);
    GO(CheckDeviceType, iWpuiiii);
    GO(CheckDeviceFormat, iWpuiiuii);
    GO(CheckDeviceMultiSampleType, iWpuiiiip);
    GO(CheckDepthStencilMatch, iWpuiiii);
    GO(CheckDeviceFormatConversion, iWpuiii);
    GO(GetDeviceCaps, iWpuip);
    GO(GetAdapterMonitor, pWpu);
    GO(CreateDevice, iWpuipupp);
	
    GO(GetAdapterModeCountEx, uWpup);
    GO(EnumAdapterModesEx, iWpupup);
    GO(GetAdapterDisplayModeEx, iWpupp);
    GO(CreateDeviceEx, iWpuipuppp);
    GO(GetAdapterLUID, iWpup);
}

static void make_vtable_IDirect3D9ExOverlayExtensionVtbl(x64emu_t* emu, IDirect3D9ExOverlayExtensionVtbl* new, IDirect3D9ExOverlayExtensionVtbl* real)
{
    GO(QueryInterface, iWppp);
    GO(AddRef, uWp);
    GO(Release, uWp);
	
    GO(CheckDeviceOverlayType, iWpuiuuipip);
}

static void make_vtable_IDirect3DAuthenticatedChannel9Vtbl(x64emu_t* emu, IDirect3DAuthenticatedChannel9Vtbl* new, IDirect3DAuthenticatedChannel9Vtbl* real)
{
    GO(QueryInterface, iWppp);
    GO(AddRef, uWp);
    GO(Release, uWp);
	
    GO(GetCertificateSize, iWpp);
    GO(GetCertificate, iWpup);
    GO(NegotiateKeyExchange, iWpup);
    GO(Query, iWpupup);
    GO(Configure, iWpupp);
}

static void make_vtable_IDirect3DBaseTexture9Vtbl(x64emu_t* emu, IDirect3DBaseTexture9Vtbl* new, IDirect3DBaseTexture9Vtbl* real)
{
    GO(QueryInterface, iWppp);
    GO(AddRef, uWp);
    GO(Release, uWp);
	
    GO(GetDevice, iWpp);
    GO(SetPrivateData, iWpppuu);
    GO(GetPrivateData, iWpppp);
    GO(FreePrivateData, iWpp);
    GO(SetPriority, uWpu);
    GO(GetPriority, uWp);
    GO(PreLoad, vWp);
    GO(GetType, iWp);
	
    GO(SetLOD, uWpu);
    GO(GetLOD, uWp);
    GO(GetLevelCount, uWp);
    GO(SetAutoGenFilterType, iWpi);
    GO(GetAutoGenFilterType, iWp);
    GO(GenerateMipSubLevels, vWp);
}

static void make_vtable_IDirect3DCryptoSession9Vtbl(x64emu_t* emu, IDirect3DCryptoSession9Vtbl* new, IDirect3DCryptoSession9Vtbl* real)
{
    GO(QueryInterface, iWppp);
    GO(AddRef, uWp);
    GO(Release, uWp);
	
    GO(GetCertificateSize, iWpp);
    GO(GetCertificate, iWpup);
    GO(NegotiateKeyExchange, iWpup);
    GO(EncryptionBlt, iWpppup);
    GO(DecryptionBlt, iWpppuppp);
    GO(GetSurfacePitch, iWppp);
    GO(StartSessionKeyRefresh, iWppu);
    GO(FinishSessionKeyRefresh, iWp);
    GO(GetEncryptionBltKey, iWppu);
}

static void make_vtable_IDirect3DCubeTexture9Vtbl(x64emu_t* emu, IDirect3DCubeTexture9Vtbl* new, IDirect3DCubeTexture9Vtbl* real)
{
    GO(QueryInterface, iWppp);
    GO(AddRef, uWp);
    GO(Release, uWp);
	
    GO(GetDevice, iWpp);
    GO(SetPrivateData, iWpppuu);
    GO(GetPrivateData, iWpppp);
    GO(FreePrivateData, iWpp);
    GO(SetPriority, uWpu);
    GO(GetPriority, uWp);
    GO(PreLoad, vWp);
    GO(GetType, iWp);
	
    GO(SetLOD, uWpu);
    GO(GetLOD, uWp);
    GO(GetLevelCount, uWp);
    GO(SetAutoGenFilterType, iWpi);
    GO(GetAutoGenFilterType, iWp);
    GO(GenerateMipSubLevels, vWp);
	
    GO(GetLevelDesc, iWpup);
    GOM(GetCubeMapSurface, iWEpiup);
    GO(LockRect, iWpiuppu);
    GO(UnlockRect, iWpiu);
    GO(AddDirtyRect, iWpip);
}

static void make_vtable_IDirect3DDevice9Vtbl(x64emu_t* emu, IDirect3DDevice9Vtbl* new, IDirect3DDevice9Vtbl* real)
{
    GO(QueryInterface, iWppp);
    GO(AddRef, uWp);
    GO(Release, uWp);
	
    GO(TestCooperativeLevel, iWp);
    GO(GetAvailableTextureMem, uWp);
    GO(EvictManagedResources, iWp);
    GOM(GetDirect3D, iWEpp);
    GO(GetDeviceCaps, iWpp);
    GO(GetDisplayMode, iWpup);
    GO(GetCreationParameters, iWpp);
    GO(SetCursorProperties, iWpuup);
    GO(SetCursorPosition, vWpiiu);
    GO(ShowCursor, iWpi);
    GOM(CreateAdditionalSwapChain, iWEppp);
    GOM(GetSwapChain, iWEpup);
    GO(GetNumberOfSwapChains, uWp);
    GO(Reset, iWpp);
    GO(Present, iWppppp);
    GOM(GetBackBuffer, iWEpuuip);
    GO(GetRasterStatus, iWpup);
    GO(SetDialogBoxMode, iWpi);
    GO(SetGammaRamp, vWpuup);
    GO(GetGammaRamp, vWpup);
    GOM(CreateTexture, iWEpuuuuiipp);
    GOM(CreateVolumeTexture, iWEpuuuuuiipp);
    GOM(CreateCubeTexture, iWEpuuuiipp);
    GOM(CreateVertexBuffer, iWEpuuuipp);
    GOM(CreateIndexBuffer, iWEpuuiipp);
    GOM(CreateRenderTarget, iWEpuuiiuipp);
    GOM(CreateDepthStencilSurface, iWEpuuiiuipp);
    GO(UpdateSurface, iWppppp);
    GO(UpdateTexture, iWppp);
    GO(GetRenderTargetData, iWppp);
    GO(GetFrontBufferData, iWpup);
    GO(StretchRect, iWpppppi);
    GO(ColorFill, iWpppu);
    GOM(CreateOffscreenPlainSurface, iWEpuuiipp);
    GO(SetRenderTarget, iWpup);
    GOM(GetRenderTarget, iWEpup);
    GO(SetDepthStencilSurface, iWpp);
    GOM(GetDepthStencilSurface, iWEpp);
    GO(BeginScene, iWp);
    GO(EndScene, iWp);
    GO(Clear, iWpupuufu);
    GO(SetTransform, iWpip);
    GO(GetTransform, iWpip);
    GO(MultiplyTransform, iWpip);
    GO(SetViewport, iWpp);
    GO(GetViewport, iWpp);
    GO(SetMaterial, iWpp);
    GO(GetMaterial, iWpp);
    GO(SetLight, iWpup);
    GO(GetLight, iWpup);
    GO(LightEnable, iWpui);
    GO(GetLightEnable, iWpup);
    GO(SetClipPlane, iWpup);
    GO(GetClipPlane, iWpup);
    GO(SetRenderState, iWpiu);
    GO(GetRenderState, iWpip);
    GOM(CreateStateBlock, iWEpip);
    GO(BeginStateBlock, iWp);
    GOM(EndStateBlock, iWEpp);
    GO(SetClipStatus, iWpp);
    GO(GetClipStatus, iWpp);
    GO(GetTexture, iWpup);
    GO(SetTexture, iWpup);
    GO(GetTextureStageState, iWpuip);
    GO(SetTextureStageState, iWpuiu);
    GO(GetSamplerState, iWpuip);
    GO(SetSamplerState, iWpuiu);
    GO(ValidateDevice, iWpp);
    GO(SetPaletteEntries, iWpup);
    GO(GetPaletteEntries, iWpup);
    GO(SetCurrentTexturePalette, iWpu);
    GO(GetCurrentTexturePalette, iWpp);
    GO(SetScissorRect, iWpp);
    GO(GetScissorRect, iWpp);
    GO(SetSoftwareVertexProcessing, iWpi);
    GO(GetSoftwareVertexProcessing, iWp);
    GO(SetNPatchMode, iWpf);
    GO(GetNPatchMode, fWp);
    GO(DrawPrimitive, iWpiuu);
    GO(DrawIndexedPrimitive, iWpiiuuuu);
    GO(DrawPrimitiveUP, iWpiupu);
    GO(DrawIndexedPrimitiveUP, iWpiuuupipu);
    GO(ProcessVertices, iWpuuuppu);
    GOM(CreateVertexDeclaration, iWEppp);
    GO(SetVertexDeclaration, iWpp);
    GO(GetVertexDeclaration, iWpp);
    GO(SetFVF, iWpu);
    GO(GetFVF, iWpp);
    GOM(CreateVertexShader, iWEppp);
    GO(SetVertexShader, iWpp);
    GO(GetVertexShader, iWpp);
    GO(SetVertexShaderConstantF, iWpupu);
    GO(GetVertexShaderConstantF, iWpupu);
    GO(SetVertexShaderConstantI, iWpupu);
    GO(GetVertexShaderConstantI, iWpupu);
    GO(SetVertexShaderConstantB, iWpupu);
    GO(GetVertexShaderConstantB, iWpupu);
    GO(SetStreamSource, iWpupuu);
    GO(GetStreamSource, iWpuppp);
    GO(SetStreamSourceFreq, iWpuu);
    GO(GetStreamSourceFreq, iWpup);
    GO(SetIndices, iWpp);
    GO(GetIndices, iWpp);
    GOM(CreatePixelShader, iWEppp);
    GO(SetPixelShader, iWpp);
    GO(GetPixelShader, iWpp);
    GO(SetPixelShaderConstantF, iWpupu);
    GO(GetPixelShaderConstantF, iWpupu);
    GO(SetPixelShaderConstantI, iWpupu);
    GO(GetPixelShaderConstantI, iWpupu);
    GO(SetPixelShaderConstantB, iWpupu);
    GO(GetPixelShaderConstantB, iWpupu);
    GO(DrawRectPatch, iWpupp);
    GO(DrawTriPatch, iWpupp);
    GO(DeletePatch, iWpu);
    GOM(CreateQuery, iWEpip);
}

static void make_vtable_IDirect3DDevice9ExVtbl(x64emu_t* emu, IDirect3DDevice9ExVtbl* new, IDirect3DDevice9ExVtbl* real)
{
    GO(QueryInterface, iWppp);
    GO(AddRef, uWp);
    GO(Release, uWp);
	
    GO(TestCooperativeLevel, iWp);
    GO(GetAvailableTextureMem, uWp);
    GO(EvictManagedResources, iWp);
    GO(GetDirect3D, iWpp);
    GO(GetDeviceCaps, iWpp);
    GO(GetDisplayMode, iWpup);
    GO(GetCreationParameters, iWpp);
    GO(SetCursorProperties, iWpuup);
    GO(SetCursorPosition, vWpiiu);
    GO(ShowCursor, iWpi);
    GOM(CreateAdditionalSwapChain, iWEppp);
    GOM(GetSwapChain, iWEpup);
    GO(GetNumberOfSwapChains, uWp);
    GO(Reset, iWpp);
    GO(Present, iWppppp);
    GOM(GetBackBuffer, iWEpuuip);
    GO(GetRasterStatus, iWpup);
    GO(SetDialogBoxMode, iWpi);
    GO(SetGammaRamp, vWpuup);
    GO(GetGammaRamp, vWpup);
    GOM(CreateTexture, iWEpuuuuiipp);
    GOM(CreateVolumeTexture, iWEpuuuuuiipp);
    GOM(CreateCubeTexture, iWEpuuuiipp);
    GOM(CreateVertexBuffer, iWEpuuuipp);
    GOM(CreateIndexBuffer, iWEpuuiipp);
    GOM(CreateRenderTarget, iWEpuuiiuipp);
    GOM(CreateDepthStencilSurface, iWEpuuiiuipp);
    GO(UpdateSurface, iWppppp);
    GO(UpdateTexture, iWppp);
    GO(GetRenderTargetData, iWppp);
    GO(GetFrontBufferData, iWpup);
    GO(StretchRect, iWpppppi);
    GO(ColorFill, iWpppu);
    GOM(CreateOffscreenPlainSurface, iWEpuuiipp);
    GO(SetRenderTarget, iWpup);
    GOM(GetRenderTarget, iWEpup);
    GO(SetDepthStencilSurface, iWpp);
    GOM(GetDepthStencilSurface, iWEpp);
    GO(BeginScene, iWp);
    GO(EndScene, iWp);
    GO(Clear, iWpupuufu);
    GO(SetTransform, iWpip);
    GO(GetTransform, iWpip);
    GO(MultiplyTransform, iWpip);
    GO(SetViewport, iWpp);
    GO(GetViewport, iWpp);
    GO(SetMaterial, iWpp);
    GO(GetMaterial, iWpp);
    GO(SetLight, iWpup);
    GO(GetLight, iWpup);
    GO(LightEnable, iWpui);
    GO(GetLightEnable, iWpup);
    GO(SetClipPlane, iWpup);
    GO(GetClipPlane, iWpup);
    GO(SetRenderState, iWpiu);
    GO(GetRenderState, iWpip);
    GOM(CreateStateBlock, iWEpip);
    GO(BeginStateBlock, iWp);
    GOM(EndStateBlock, iWEpp);
    GO(SetClipStatus, iWpp);
    GO(GetClipStatus, iWpp);
    GO(GetTexture, iWpup);
    GO(SetTexture, iWpup);
    GO(GetTextureStageState, iWpuip);
    GO(SetTextureStageState, iWpuiu);
    GO(GetSamplerState, iWpuip);
    GO(SetSamplerState, iWpuiu);
    GO(ValidateDevice, iWpp);
    GO(SetPaletteEntries, iWpup);
    GO(GetPaletteEntries, iWpup);
    GO(SetCurrentTexturePalette, iWpu);
    GO(GetCurrentTexturePalette, iWpp);
    GO(SetScissorRect, iWpp);
    GO(GetScissorRect, iWpp);
    GO(SetSoftwareVertexProcessing, iWpi);
    GO(GetSoftwareVertexProcessing, iWp);
    GO(SetNPatchMode, iWpf);
    GO(GetNPatchMode, fWp);
    GO(DrawPrimitive, iWpiuu);
    GO(DrawIndexedPrimitive, iWpiiuuuu);
    GO(DrawPrimitiveUP, iWpiupu);
    GO(DrawIndexedPrimitiveUP, iWpiuuupipu);
    GO(ProcessVertices, iWpuuuppu);
    GOM(CreateVertexDeclaration, iWEppp);
    GO(SetVertexDeclaration, iWpp);
    GO(GetVertexDeclaration, iWpp);
    GO(SetFVF, iWpu);
    GO(GetFVF, iWpp);
    GOM(CreateVertexShader, iWEppp);
    GO(SetVertexShader, iWpp);
    GO(GetVertexShader, iWpp);
    GO(SetVertexShaderConstantF, iWpupu);
    GO(GetVertexShaderConstantF, iWpupu);
    GO(SetVertexShaderConstantI, iWpupu);
    GO(GetVertexShaderConstantI, iWpupu);
    GO(SetVertexShaderConstantB, iWpupu);
    GO(GetVertexShaderConstantB, iWpupu);
    GO(SetStreamSource, iWpupuu);
    GO(GetStreamSource, iWpuppp);
    GO(SetStreamSourceFreq, iWpuu);
    GO(GetStreamSourceFreq, iWpup);
    GO(SetIndices, iWpp);
    GO(GetIndices, iWpp);
    GOM(CreatePixelShader, iWEppp);
    GO(SetPixelShader, iWpp);
    GO(GetPixelShader, iWpp);
    GO(SetPixelShaderConstantF, iWpupu);
    GO(GetPixelShaderConstantF, iWpupu);
    GO(SetPixelShaderConstantI, iWpupu);
    GO(GetPixelShaderConstantI, iWpupu);
    GO(SetPixelShaderConstantB, iWpupu);
    GO(GetPixelShaderConstantB, iWpupu);
    GO(DrawRectPatch, iWpupp);
    GO(DrawTriPatch, iWpupp);
    GO(DeletePatch, iWpu);
    GOM(CreateQuery, iWEpip);
	
    GO(SetConvolutionMonoKernel, iWpuupp);
    GO(ComposeRects, iWppppupiii);
    GO(PresentEx, iWpppppu);
    GO(GetGPUThreadPriority, iWpp);
    GO(SetGPUThreadPriority, iWpi);
    GO(WaitForVBlank, iWpu);
    GOM(CheckResourceResidency, iWEppu);
    GO(SetMaximumFrameLatency, iWpu);
    GO(GetMaximumFrameLatency, iWpp);
    GO(CheckDeviceState, iWpp);
    GOM(CreateRenderTargetEx, iWEpuuiiuippu);
    GOM(CreateOffscreenPlainSurfaceEx, iWEpuuiippu);
    GOM(CreateDepthStencilSurfaceEx, iWEpuuiiuippu);
    GO(ResetEx, iWppp);
    GO(GetDisplayModeEx, iWpupp);
}

static void make_vtable_IDirect3DDevice9VideoVtbl(x64emu_t* emu, IDirect3DDevice9VideoVtbl* new, IDirect3DDevice9VideoVtbl* real)
{
    GO(QueryInterface, iWppp);
    GO(AddRef, uWp);
    GO(Release, uWp);
	
    GO(GetContentProtectionCaps, iWpppp);
    GOM(CreateAuthenticatedChannel, iWEpipp);
    GOM(CreateCryptoSession, iWEppppp);
}

static void make_vtable_IDirect3DIndexBuffer9Vtbl(x64emu_t* emu, IDirect3DIndexBuffer9Vtbl* new, IDirect3DIndexBuffer9Vtbl* real)
{
    GO(QueryInterface, iWppp);
    GO(AddRef, uWp);
    GO(Release, uWp);
	
    GO(GetDevice, iWpp);
    GO(SetPrivateData, iWpppuu);
    GO(GetPrivateData, iWpppp);
    GO(FreePrivateData, iWpp);
    GO(SetPriority, uWpu);
    GO(GetPriority, uWp);
    GO(PreLoad, vWp);
    GO(GetType, iWp);
	
    GO(Lock, iWpuupu);
    GO(Unlock, iWp);
    GO(GetDesc, iWpp);
}

static void make_vtable_IDirect3DPixelShader9Vtbl(x64emu_t* emu, IDirect3DPixelShader9Vtbl* new, IDirect3DPixelShader9Vtbl* real)
{
    GO(QueryInterface, iWppp);
    GO(AddRef, uWp);
    GO(Release, uWp);
	
    GO(GetDevice, iWpp);
    GO(GetFunction, iWppp);
}

static void make_vtable_IDirect3DQuery9Vtbl(x64emu_t* emu, IDirect3DQuery9Vtbl* new, IDirect3DQuery9Vtbl* real)
{
    GO(QueryInterface, iWppp);
    GO(AddRef, uWp);
    GO(Release, uWp);
	
    GO(GetDevice, iWpp);
    GO(GetType, iWp);
    GO(GetDataSize, uWp);
    GO(Issue, iWpu);
    GO(GetData, iWppuu);
}

static void make_vtable_IDirect3DResource9Vtbl(x64emu_t* emu, IDirect3DResource9Vtbl* new, IDirect3DResource9Vtbl* real)
{
    GO(QueryInterface, iWppp);
    GO(AddRef, uWp);
    GO(Release, uWp);
	
    GO(GetDevice, iWpp);
    GO(SetPrivateData, iWpppuu);
    GO(GetPrivateData, iWpppp);
    GO(FreePrivateData, iWpp);
    GO(SetPriority, uWpu);
    GO(GetPriority, uWp);
    GO(PreLoad, vWp);
    GO(GetType, iWp);
}

static void make_vtable_IDirect3DStateBlock9Vtbl(x64emu_t* emu, IDirect3DStateBlock9Vtbl* new, IDirect3DStateBlock9Vtbl* real)
{
    GO(QueryInterface, iWppp);
    GO(AddRef, uWp);
    GO(Release, uWp);
	
    GO(GetDevice, iWpp);
    GO(Capture, iWp);
    GO(Apply, iWp);
}

static void make_vtable_IDirect3DSurface9Vtbl(x64emu_t* emu, IDirect3DSurface9Vtbl* new, IDirect3DSurface9Vtbl* real)
{
    GO(QueryInterface, iWppp);
    GO(AddRef, uWp);
    GO(Release, uWp);
	
    GO(GetDevice, iWpp);
    GO(SetPrivateData, iWpppuu);
    GO(GetPrivateData, iWpppp);
    GO(FreePrivateData, iWpp);
    GO(SetPriority, uWpu);
    GO(GetPriority, uWp);
    GO(PreLoad, vWp);
    GO(GetType, iWp);
	
    GO(GetContainer, iWppp);
    GO(GetDesc, iWpp);
    GO(LockRect, iWpppu);
    GO(UnlockRect, iWp);
    GO(GetDC, iWpp);
    GO(ReleaseDC, iWpp);
}

static void make_vtable_IDirect3DSwapChain9Vtbl(x64emu_t* emu, IDirect3DSwapChain9Vtbl* new, IDirect3DSwapChain9Vtbl* real)
{
    GO(QueryInterface, iWppp);
    GO(AddRef, uWp);
    GO(Release, uWp);
	
    GO(Present, iWpppppu);
    GO(GetFrontBufferData, iWpp);
    GO(GetBackBuffer, iWpuip); // XXX
    GO(GetRasterStatus, iWpp);
    GO(GetDisplayMode, iWpp);
    GO(GetDevice, iWpp);
    GO(GetPresentParameters, iWpp);
}

static void make_vtable_IDirect3DSwapChain9ExVtbl(x64emu_t* emu, IDirect3DSwapChain9ExVtbl* new, IDirect3DSwapChain9ExVtbl* real)
{
    GO(QueryInterface, iWppp);
    GO(AddRef, uWp);
    GO(Release, uWp);
	
    GO(Present, iWpppppu);
    GO(GetFrontBufferData, iWpp);
    GO(GetBackBuffer, iWpuip); // XXX
    GO(GetRasterStatus, iWpp);
    GO(GetDisplayMode, iWpp);
    GO(GetDevice, iWpp);
    GO(GetPresentParameters, iWpp);
	
    GO(GetLastPresentCount, iWpp);
    GO(GetPresentStats, iWpp);
    GO(GetDisplayModeEx, iWppp);
}

static void make_vtable_IDirect3DTexture9Vtbl(x64emu_t* emu, IDirect3DTexture9Vtbl* new, IDirect3DTexture9Vtbl* real)
{
    GO(QueryInterface, iWppp);
    GO(AddRef, uWp);
    GO(Release, uWp);
	
    GO(GetDevice, iWpp);
    GO(SetPrivateData, iWpppuu);
    GO(GetPrivateData, iWpppp);
    GO(FreePrivateData, iWpp);
    GO(SetPriority, uWpu);
    GO(GetPriority, uWp);
    GO(PreLoad, vWp);
    GO(GetType, iWp);
	
    GO(SetLOD, uWpu);
    GO(GetLOD, uWp);
    GO(GetLevelCount, uWp);
    GO(SetAutoGenFilterType, iWpi);
    GO(GetAutoGenFilterType, iWp);
    GO(GenerateMipSubLevels, vWp);
	
    GO(GetLevelDesc, iWpup);
    GOM(GetSurfaceLevel, iWEpup);
    GO(LockRect, iWpuppu);
    GO(UnlockRect, iWpu);
    GO(AddDirtyRect, iWpp);
}

static void make_vtable_IDirect3DVertexBuffer9Vtbl(x64emu_t* emu, IDirect3DVertexBuffer9Vtbl* new, IDirect3DVertexBuffer9Vtbl* real)
{
    GO(QueryInterface, iWppp);
    GO(AddRef, uWp);
    GO(Release, uWp);
	
    GO(GetDevice, iWpp);
    GO(SetPrivateData, iWpppuu);
    GO(GetPrivateData, iWpppp);
    GO(FreePrivateData, iWpp);
    GO(SetPriority, uWpu);
    GO(GetPriority, uWp);
    GO(PreLoad, vWp);
    GO(GetType, iWp);
	
    GO(Lock, iWpuupu);
    GO(Unlock, iWp);
    GO(GetDesc, iWpp);
}

static void make_vtable_IDirect3DVertexDeclaration9Vtbl(x64emu_t* emu, IDirect3DVertexDeclaration9Vtbl* new, IDirect3DVertexDeclaration9Vtbl* real)
{
    GO(QueryInterface, iWppp);
    GO(AddRef, uWp);
    GO(Release, uWp);
	
    GO(GetDevice, iWpp);
    GO(GetDeclaration, iWppp);
}

static void make_vtable_IDirect3DVertexShader9Vtbl(x64emu_t* emu, IDirect3DVertexShader9Vtbl* new, IDirect3DVertexShader9Vtbl* real)
{
    GO(QueryInterface, iWppp);
    GO(AddRef, uWp);
    GO(Release, uWp);
	
    GO(GetDevice, iWpp);
    GO(GetFunction, iWppp);
}

static void make_vtable_IDirect3DVolume9Vtbl(x64emu_t* emu, IDirect3DVolume9Vtbl* new, IDirect3DVolume9Vtbl* real)
{
    GO(QueryInterface, iWppp);
    GO(AddRef, uWp);
    GO(Release, uWp);
	
    GO(GetDevice, iWpp);
    GO(SetPrivateData, iWpppuu);
    GO(GetPrivateData, iWpppp);
    GO(FreePrivateData, iWpp);
    GO(GetContainer, iWppp);
    GO(GetDesc, iWpp);
    GO(LockBox, iWpppu);
    GO(UnlockBox, iWp);
}

static void make_vtable_IDirect3DVolumeTexture9Vtbl(x64emu_t* emu, IDirect3DVolumeTexture9Vtbl* new, IDirect3DVolumeTexture9Vtbl* real)
{
    GO(QueryInterface, iWppp);
    GO(AddRef, uWp);
    GO(Release, uWp);
	
    GO(GetDevice, iWpp);
    GO(SetPrivateData, iWpppuu);
    GO(GetPrivateData, iWpppp);
    GO(FreePrivateData, iWpp);
    GO(SetPriority, uWpu);
    GO(GetPriority, uWp);
    GO(PreLoad, vWp);
    GO(GetType, iWp);
	
    GO(SetLOD, uWpu);
    GO(GetLOD, uWp);
    GO(GetLevelCount, uWp);
    GO(SetAutoGenFilterType, iWpi);
    GO(GetAutoGenFilterType, iWp);
    GO(GenerateMipSubLevels, vWp);
	
    GO(GetLevelDesc, iWpup);
    GOM(GetVolumeLevel, iWEpup);
    GO(LockBox, iWpuppu);
    GO(UnlockBox, iWpu);
    GO(AddDirtyBox, iWpp);
}
