/* Taken from include/ and include/d3dadapter/ in Mesa, with some
 * typedefs and defines removed. */

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

typedef struct ID3DAdapter9Vtbl
{
	int (*QueryInterface)(void *This, void *riid, void **ppvObject);
	unsigned (*AddRef)(void *This);
	unsigned (*Release)(void *This);
	/* ID3DAdapter9 */
	int (*GetAdapterIdentifier)(void *This, unsigned Flags, void *pIdentifier);
	int (*CheckDeviceType)(void *This, int DevType, int AdapterFormat, int BackBufferFormat, int bWindowed);
	int (*CheckDeviceFormat)(void *This, int DeviceType, int AdapterFormat, unsigned Usage, int RType, int CheckFormat);
	int (*CheckDeviceMultiSampleType)(void *This, int DeviceType, int SurfaceFormat, int Windowed, int MultiSampleType, void *pQualityLevels);
	int (*CheckDepthStencilMatch)(void *This, int DeviceType, int AdapterFormat, int RenderTargetFormat, int DepthStencilFormat);
	int (*CheckDeviceFormatConversion)(void *This, int DeviceType, int SourceFormat, int TargetFormat);
	int (*GetDeviceCaps)(void *This, int DeviceType, void *pCaps);
	int (*CreateDevice)(void *This, unsigned RealAdapter, int DeviceType, void *hFocusWindow, unsigned BehaviorFlags, void *pPresentationParameters, void *pD3D9, void *pPresentationFactory, void **ppReturnedDeviceInterface);
	int (*CreateDeviceEx)(void *This, unsigned RealAdapter, int DeviceType, void *hFocusWindow, unsigned BehaviorFlags, void *pPresentationParameters, void *pFullscreenDisplayMode, void *pD3D9Ex, void *pPresentationFactory, void **ppReturnedDeviceInterface);
} ID3DAdapter9Vtbl;

typedef struct ID3DPresentVtbl
{
	int (*QueryInterface)(void *This, void *riid, void **ppvObject);
	unsigned (*AddRef)(void *This);
	unsigned (*Release)(void *This);

	/* ID3DPresent */
	int (*SetPresentParameters)(void *This, void *pPresentationParameters, void *pFullscreenDisplayMode);
	int (*NewD3DWindowBufferFromDmaBuf)(void *This, int dmaBufFd, int width, int height, int stride, int depth, int bpp, void **out);
	int (*DestroyD3DWindowBuffer)(void *This, void *buffer);
	int (*WaitBufferReleased)(void *This, void *buffer);
	int (*FrontBufferCopy)(void *This, void *buffer);
	int (*PresentBuffer)(void *This, void *buffer, void *hWndOverride, const void *pSourceRect, const void *pDestRect, const void *pDirtyRegion, unsigned Flags);
	int (*GetRasterStatus)(void *This, void *pRasterStatus);
	int (*GetDisplayMode)(void *This, void *pMode, void *pRotation);
	int (*GetPresentStats)(void *This, void *pStats);
	int (*GetCursorPos)(void *This, void *pPoint);
	int (*SetCursorPos)(void *This, void *pPoint);
	int (*SetCursor)(void *This, void *pBitmap, void *pHotspot, int bShow);
	int (*SetGammaRamp)(void *This, const void *pRamp, void *hWndOverride);
	int (*GetWindowInfo)(void *This,  void *hWnd, void *width, void *height, void *depth);
	int (*GetWindowOccluded)(void *This);
	int (*ResolutionMismatch)(void *This);
	void *(*CreateThread)(void *This, void *pThreadfunc, void *pParam);
	int (*WaitForThread)(void *This, void *thread);
	int (*SetPresentParameters2)(void *This, void *pParameters);
	int (*IsBufferReleased)(void *This, void *buffer);
	int (*WaitBufferReleaseEvent)(void *This);
} ID3DPresentVtbl;

typedef struct ID3DPresentGroupVtbl
{
	int (*QueryInterface)(void *This, void *riid, void **ppvObject);
	unsigned (*AddRef)(void *This);
	unsigned (*Release)(void *This);

	/* ID3DPresentGroup */
	unsigned (*GetMultiheadCount)(void *This);
	int (*GetPresent)(void *This, unsigned Index, void **ppPresent);
	int (*CreateAdditionalPresent)(void *This, void *pPresentationParameters, void **ppPresent);
        /*void*/ int (*GetVersion) (void *This, void *major, void *minor);
} ID3DPresentGroupVtbl;



typedef struct IDirect3D9Vtbl
{
	int (*QueryInterface)(void *This, void *riid, void **ppvObject);
	unsigned (*AddRef)(void *This);
	unsigned (*Release)(void *This);
	/* IDirect3D9 */
	int (*RegisterSoftwareDevice)(void *This, void *pInitializeFunction);
	unsigned (*GetAdapterCount)(void *This);
	int (*GetAdapterIdentifier)(void *This, unsigned Adapter, unsigned Flags, void *pIdentifier);
	unsigned (*GetAdapterModeCount)(void *This, unsigned Adapter, int Format);
	int (*EnumAdapterModes)(void *This, unsigned Adapter, int Format, unsigned Mode, void *pMode);
	int (*GetAdapterDisplayMode)(void *This, unsigned Adapter, void *pMode);
	int (*CheckDeviceType)(void *This, unsigned Adapter, int DevType, int AdapterFormat, int BackBufferFormat, int bWindowed);
	int (*CheckDeviceFormat)(void *This, unsigned Adapter, int DeviceType, int AdapterFormat, unsigned Usage, int RType, int CheckFormat);
	int (*CheckDeviceMultiSampleType)(void *This, unsigned Adapter, int DeviceType, int SurfaceFormat, int Windowed, int MultiSampleType, void *pQualityLevels);
	int (*CheckDepthStencilMatch)(void *This, unsigned Adapter, int DeviceType, int AdapterFormat, int RenderTargetFormat, int DepthStencilFormat);
	int (*CheckDeviceFormatConversion)(void *This, unsigned Adapter, int DeviceType, int SourceFormat, int TargetFormat);
	int (*GetDeviceCaps)(void *This, unsigned Adapter, int DeviceType, void *pCaps);
	void *(*GetAdapterMonitor)(void *This, unsigned Adapter);
	int (*CreateDevice)(void *This, unsigned Adapter, int DeviceType, void *hFocusWindow, unsigned BehaviorFlags, void *pPresentationParameters, void **ppReturnedDeviceInterface);
} IDirect3D9Vtbl;

typedef struct IDirect3D9ExVtbl
{
	int (*QueryInterface)(void *This, void *riid, void **ppvObject);
	unsigned (*AddRef)(void *This);
	unsigned (*Release)(void *This);
	/* IDirect3D9 */
	int (*RegisterSoftwareDevice)(void *This, void *pInitializeFunction);
	unsigned (*GetAdapterCount)(void *This);
	int (*GetAdapterIdentifier)(void *This, unsigned Adapter, unsigned Flags, void *pIdentifier);
	unsigned (*GetAdapterModeCount)(void *This, unsigned Adapter, int Format);
	int (*EnumAdapterModes)(void *This, unsigned Adapter, int Format, unsigned Mode, void *pMode);
	int (*GetAdapterDisplayMode)(void *This, unsigned Adapter, void *pMode);
	int (*CheckDeviceType)(void *This, unsigned Adapter, int DevType, int AdapterFormat, int BackBufferFormat, int bWindowed);
	int (*CheckDeviceFormat)(void *This, unsigned Adapter, int DeviceType, int AdapterFormat, unsigned Usage, int RType, int CheckFormat);
	int (*CheckDeviceMultiSampleType)(void *This, unsigned Adapter, int DeviceType, int SurfaceFormat, int Windowed, int MultiSampleType, void *pQualityLevels);
	int (*CheckDepthStencilMatch)(void *This, unsigned Adapter, int DeviceType, int AdapterFormat, int RenderTargetFormat, int DepthStencilFormat);
	int (*CheckDeviceFormatConversion)(void *This, unsigned Adapter, int DeviceType, int SourceFormat, int TargetFormat);
	int (*GetDeviceCaps)(void *This, unsigned Adapter, int DeviceType, void *pCaps);
	void *(*GetAdapterMonitor)(void *This, unsigned Adapter);
	int (*CreateDevice)(void *This, unsigned Adapter, int DeviceType, void *hFocusWindow, unsigned BehaviorFlags, void *pPresentationParameters, void **ppReturnedDeviceInterface);
	/* IDirect3D9Ex */
	unsigned (*GetAdapterModeCountEx)(void *This, unsigned Adapter, const void *pFilter);
	int (*EnumAdapterModesEx)(void *This, unsigned Adapter, const void *pFilter, unsigned Mode, void *pMode);
	int (*GetAdapterDisplayModeEx)(void *This, unsigned Adapter, void *pMode, void *pRotation);
	int (*CreateDeviceEx)(void *This, unsigned Adapter, int DeviceType, void *hFocusWindow, unsigned BehaviorFlags, void *pPresentationParameters, void *pFullscreenDisplayMode, void **ppReturnedDeviceInterface);
	int (*GetAdapterLUID)(void *This, unsigned Adapter, void *pLUID);
} IDirect3D9ExVtbl;

typedef struct IDirect3D9ExOverlayExtensionVtbl
{
	int (*QueryInterface)(void *This, void *riid, void **ppvObject);
	unsigned (*AddRef)(void *This);
	unsigned (*Release)(void *This);
	/* IDirect3D9ExOverlayExtension */
	int (*CheckDeviceOverlayType)(void *This, unsigned Adapter, int DevType, unsigned OverlayWidth, unsigned OverlayHeight, int OverlayFormat, void *pDisplayMode, int DisplayRotation, void *pOverlayCaps);
} IDirect3D9ExOverlayExtensionVtbl;

typedef struct IDirect3DAuthenticatedChannel9Vtbl
{
	int (*QueryInterface)(void *This, void *riid, void **ppvObject);
	unsigned (*AddRef)(void *This);
	unsigned (*Release)(void *This);
	/* IDirect3DAuthenticatedChannel9 */
	int (*GetCertificateSize)(void *This, void *pCertificateSize);
	int (*GetCertificate)(void *This, unsigned CertifacteSize, void *ppCertificate);
	int (*NegotiateKeyExchange)(void *This, unsigned DataSize, void *pData);
	int (*Query)(void *This, unsigned InputSize, const void *pInput, unsigned OutputSize, void *pOutput);
	int (*Configure)(void *This, unsigned InputSize, const void *pInput, void *pOutput);
} IDirect3DAuthenticatedChannel9Vtbl;

typedef struct IDirect3DBaseTexture9Vtbl
{
	int (*QueryInterface)(void *This, void *riid, void **ppvObject);
	unsigned (*AddRef)(void *This);
	unsigned (*Release)(void *This);
	/* IDirect3DResource9 */
	int (*GetDevice)(void *This, void **ppDevice);
	int (*SetPrivateData)(void *This, void *refguid, const void *pData, unsigned SizeOfData, unsigned Flags);
	int (*GetPrivateData)(void *This, void *refguid, void *pData, void *pSizeOfData);
	int (*FreePrivateData)(void *This, void *refguid);
	unsigned (*SetPriority)(void *This, unsigned PriorityNew);
	unsigned (*GetPriority)(void *This);
	void (*PreLoad)(void *This);
	int (*GetType)(void *This);
	/* IDirect3DBaseTexture9 */
	unsigned (*SetLOD)(void *This, unsigned LODNew);
	unsigned (*GetLOD)(void *This);
	unsigned (*GetLevelCount)(void *This);
	int (*SetAutoGenFilterType)(void *This, int FilterType);
	int (*GetAutoGenFilterType)(void *This);
	void (*GenerateMipSubLevels)(void *This);
} IDirect3DBaseTexture9Vtbl;

typedef struct IDirect3DCryptoSession9Vtbl
{
	int (*QueryInterface)(void *This, void *riid, void **ppvObject);
	unsigned (*AddRef)(void *This);
	unsigned (*Release)(void *This);
	/* IDirect3DCryptoSession9 */
	int (*GetCertificateSize)(void *This, void *pCertificateSize);
	int (*GetCertificate)(void *This, unsigned CertifacteSize, void *ppCertificate);
	int (*NegotiateKeyExchange)(void *This, unsigned DataSize, void *pData);
	int (*EncryptionBlt)(void *This, void *pSrcSurface, void *pDstSurface, unsigned DstSurfaceSize, void *pIV);
	int (*DecryptionBlt)(void *This, void *pSrcSurface, void *pDstSurface, unsigned SrcSurfaceSize, void *pEncryptedBlockInfo, void *pContentKey, void *pIV);
	int (*GetSurfacePitch)(void *This, void *pSrcSurface, void *pSurfacePitch);
	int (*StartSessionKeyRefresh)(void *This, void *pRandomNumber, unsigned RandomNumberSize);
	int (*FinishSessionKeyRefresh)(void *This);
	int (*GetEncryptionBltKey)(void *This, void *pReadbackKey, unsigned KeySize);
} IDirect3DCryptoSession9Vtbl;

typedef struct IDirect3DCubeTexture9Vtbl
{
	int (*QueryInterface)(void *This, void *riid, void **ppvObject);
	unsigned (*AddRef)(void *This);
	unsigned (*Release)(void *This);
	/* IDirect3DResource9 */
	int (*GetDevice)(void *This, void **ppDevice);
	int (*SetPrivateData)(void *This, void *refguid, const void *pData, unsigned SizeOfData, unsigned Flags);
	int (*GetPrivateData)(void *This, void *refguid, void *pData, void *pSizeOfData);
	int (*FreePrivateData)(void *This, void *refguid);
	unsigned (*SetPriority)(void *This, unsigned PriorityNew);
	unsigned (*GetPriority)(void *This);
	void (*PreLoad)(void *This);
	int (*GetType)(void *This);
	/* IDirect3DBaseTexture9 */
	unsigned (*SetLOD)(void *This, unsigned LODNew);
	unsigned (*GetLOD)(void *This);
	unsigned (*GetLevelCount)(void *This);
	int (*SetAutoGenFilterType)(void *This, int FilterType);
	int (*GetAutoGenFilterType)(void *This);
	void (*GenerateMipSubLevels)(void *This);
	/* IDirect3DCubeTexture9 */
	int (*GetLevelDesc)(void *This, unsigned Level, void *pDesc);
	int (*GetCubeMapSurface)(void *This, int FaceType, unsigned Level, void **ppCubeMapSurface);
	int (*LockRect)(void *This, int FaceType, unsigned Level, void *pLockedRect, const void *pRect, unsigned Flags);
	int (*UnlockRect)(void *This, int FaceType, unsigned Level);
	int (*AddDirtyRect)(void *This, int FaceType, const void *pDirtyRect);
} IDirect3DCubeTexture9Vtbl;

typedef struct IDirect3DDevice9Vtbl
{
	int (*QueryInterface)(void *This, void *riid, void **ppvObject);
	unsigned (*AddRef)(void *This);
	unsigned (*Release)(void *This);
	/* IDirect3DDevice9 */
	int (*TestCooperativeLevel)(void *This);
	unsigned (*GetAvailableTextureMem)(void *This);
	int (*EvictManagedResources)(void *This);
	int (*GetDirect3D)(void *This, void **ppD3D9);
	int (*GetDeviceCaps)(void *This, void *pCaps);
	int (*GetDisplayMode)(void *This, unsigned iSwapChain, void *pMode);
	int (*GetCreationParameters)(void *This, void *pParameters);
	int (*SetCursorProperties)(void *This, unsigned XHotSpot, unsigned YHotSpot, void *pCursorBitmap);
	void (*SetCursorPosition)(void *This, int X, int Y, unsigned Flags);
	int (*ShowCursor)(void *This, int bShow);
	int (*CreateAdditionalSwapChain)(void *This, void *pPresentationParameters, void **pSwapChain);
	int (*GetSwapChain)(void *This, unsigned iSwapChain, void **pSwapChain);
	unsigned (*GetNumberOfSwapChains)(void *This);
	int (*Reset)(void *This, void *pPresentationParameters);
	int (*Present)(void *This, const void *pSourceRect, const void *pDestRect, void *hDestWindowOverride, const void *pDirtyRegion);
	int (*GetBackBuffer)(void *This, unsigned iSwapChain, unsigned iBackBuffer, int Type, void **ppBackBuffer);
	int (*GetRasterStatus)(void *This, unsigned iSwapChain, void *pRasterStatus);
	int (*SetDialogBoxMode)(void *This, int bEnableDialogs);
	void (*SetGammaRamp)(void *This, unsigned iSwapChain, unsigned Flags, const void *pRamp);
	void (*GetGammaRamp)(void *This, unsigned iSwapChain, void *pRamp);
	int (*CreateTexture)(void *This, unsigned Width, unsigned Height, unsigned Levels, unsigned Usage, int Format, int Pool, void **ppTexture, void *pSharedHandle);
	int (*CreateVolumeTexture)(void *This, unsigned Width, unsigned Height, unsigned Depth, unsigned Levels, unsigned Usage, int Format, int Pool, void **ppVolumeTexture, void *pSharedHandle);
	int (*CreateCubeTexture)(void *This, unsigned EdgeLength, unsigned Levels, unsigned Usage, int Format, int Pool, void **ppCubeTexture, void *pSharedHandle);
	int (*CreateVertexBuffer)(void *This, unsigned Length, unsigned Usage, unsigned FVF, int Pool, void **ppVertexBuffer, void *pSharedHandle);
	int (*CreateIndexBuffer)(void *This, unsigned Length, unsigned Usage, int Format, int Pool, void **ppIndexBuffer, void *pSharedHandle);
	int (*CreateRenderTarget)(void *This, unsigned Width, unsigned Height, int Format, int MultiSample, unsigned MultisampleQuality, int Lockable, void **ppSurface, void *pSharedHandle);
	int (*CreateDepthStencilSurface)(void *This, unsigned Width, unsigned Height, int Format, int MultiSample, unsigned MultisampleQuality, int Discard, void **ppSurface, void *pSharedHandle);
	int (*UpdateSurface)(void *This, void *pSourceSurface, const void *pSourceRect, void *pDestinationSurface, const void *pDestPoint);
	int (*UpdateTexture)(void *This, void *pSourceTexture, void *pDestinationTexture);
	int (*GetRenderTargetData)(void *This, void *pRenderTarget, void *pDestSurface);
	int (*GetFrontBufferData)(void *This, unsigned iSwapChain, void *pDestSurface);
	int (*StretchRect)(void *This, void *pSourceSurface, const void *pSourceRect, void *pDestSurface, const void *pDestRect, int Filter);
	int (*ColorFill)(void *This, void *pSurface, const void *pRect, unsigned color);
	int (*CreateOffscreenPlainSurface)(void *This, unsigned Width, unsigned Height, int Format, int Pool, void **ppSurface, void *pSharedHandle);
	int (*SetRenderTarget)(void *This, unsigned RenderTargetIndex, void *pRenderTarget);
	int (*GetRenderTarget)(void *This, unsigned RenderTargetIndex, void **ppRenderTarget);
	int (*SetDepthStencilSurface)(void *This, void *pNewZStencil);
	int (*GetDepthStencilSurface)(void *This, void **ppZStencilSurface);
	int (*BeginScene)(void *This);
	int (*EndScene)(void *This);
	int (*Clear)(void *This, unsigned Count, const void *pRects, unsigned Flags, unsigned Color, float Z, unsigned Stencil);
	int (*SetTransform)(void *This, int State, const void *pMatrix);
	int (*GetTransform)(void *This, int State, void *pMatrix);
	int (*MultiplyTransform)(void *This, int State, const void *pMatrix);
	int (*SetViewport)(void *This, const void *pViewport);
	int (*GetViewport)(void *This, void *pViewport);
	int (*SetMaterial)(void *This, const void *pMaterial);
	int (*GetMaterial)(void *This, void *pMaterial);
	int (*SetLight)(void *This, unsigned Index, const void *pLight);
	int (*GetLight)(void *This, unsigned Index, void *pLight);
	int (*LightEnable)(void *This, unsigned Index, int Enable);
	int (*GetLightEnable)(void *This, unsigned Index, void *pEnable);
	int (*SetClipPlane)(void *This, unsigned Index, const void *pPlane);
	int (*GetClipPlane)(void *This, unsigned Index, void *pPlane);
	int (*SetRenderState)(void *This, int State, unsigned Value);
	int (*GetRenderState)(void *This, int State, void *pValue);
	int (*CreateStateBlock)(void *This, int Type, void **ppSB);
	int (*BeginStateBlock)(void *This);
	int (*EndStateBlock)(void *This, void **ppSB);
	int (*SetClipStatus)(void *This, const void *pClipStatus);
	int (*GetClipStatus)(void *This, void *pClipStatus);
	int (*GetTexture)(void *This, unsigned Stage, void **ppTexture);
	int (*SetTexture)(void *This, unsigned Stage, void *pTexture);
	int (*GetTextureStageState)(void *This, unsigned Stage, int Type, void *pValue);
	int (*SetTextureStageState)(void *This, unsigned Stage, int Type, unsigned Value);
	int (*GetSamplerState)(void *This, unsigned Sampler, int Type, void *pValue);
	int (*SetSamplerState)(void *This, unsigned Sampler, int Type, unsigned Value);
	int (*ValidateDevice)(void *This, void *pNumPasses);
	int (*SetPaletteEntries)(void *This, unsigned PaletteNumber, const void *pEntries);
	int (*GetPaletteEntries)(void *This, unsigned PaletteNumber, void *pEntries);
	int (*SetCurrentTexturePalette)(void *This, unsigned PaletteNumber);
	int (*GetCurrentTexturePalette)(void *This, void *PaletteNumber);
	int (*SetScissorRect)(void *This, const void *pRect);
	int (*GetScissorRect)(void *This, void *pRect);
	int (*SetSoftwareVertexProcessing)(void *This, int bSoftware);
	int (*GetSoftwareVertexProcessing)(void *This);
	int (*SetNPatchMode)(void *This, float nSegments);
	float (*GetNPatchMode)(void *This);
	int (*DrawPrimitive)(void *This, int PrimitiveType, unsigned StartVertex, unsigned PrimitiveCount);
	int (*DrawIndexedPrimitive)(void *This, int PrimitiveType, int BaseVertexIndex, unsigned MinVertexIndex, unsigned NumVertices, unsigned startIndex, unsigned primCount);
	int (*DrawPrimitiveUP)(void *This, int PrimitiveType, unsigned PrimitiveCount, const void *pVertexStreamZeroData, unsigned VertexStreamZeroStride);
	int (*DrawIndexedPrimitiveUP)(void *This, int PrimitiveType, unsigned MinVertexIndex, unsigned NumVertices, unsigned PrimitiveCount, const void *pIndexData, int IndexDataFormat, const void *pVertexStreamZeroData, unsigned VertexStreamZeroStride);
	int (*ProcessVertices)(void *This, unsigned SrcStartIndex, unsigned DestIndex, unsigned VertexCount, void *pDestBuffer, void *pVertexDecl, unsigned Flags);
	int (*CreateVertexDeclaration)(void *This, const void *pVertexElements, void **ppDecl);
	int (*SetVertexDeclaration)(void *This, void *pDecl);
	int (*GetVertexDeclaration)(void *This, void **ppDecl);
	int (*SetFVF)(void *This, unsigned FVF);
	int (*GetFVF)(void *This, void *pFVF);
	int (*CreateVertexShader)(void *This, const void *pFunction, void **ppShader);
	int (*SetVertexShader)(void *This, void *pShader);
	int (*GetVertexShader)(void *This, void **ppShader);
	int (*SetVertexShaderConstantF)(void *This, unsigned StartRegister, const void *pConstantData, unsigned Vector4fCount);
	int (*GetVertexShaderConstantF)(void *This, unsigned StartRegister, void *pConstantData, unsigned Vector4fCount);
	int (*SetVertexShaderConstantI)(void *This, unsigned StartRegister, const void *pConstantData, unsigned Vector4iCount);
	int (*GetVertexShaderConstantI)(void *This, unsigned StartRegister, void *pConstantData, unsigned Vector4iCount);
	int (*SetVertexShaderConstantB)(void *This, unsigned StartRegister, const void *pConstantData, unsigned BoolCount);
	int (*GetVertexShaderConstantB)(void *This, unsigned StartRegister, void *pConstantData, unsigned BoolCount);
	int (*SetStreamSource)(void *This, unsigned StreamNumber, void *pStreamData, unsigned OffsetInBytes, unsigned Stride);
	int (*GetStreamSource)(void *This, unsigned StreamNumber, void **ppStreamData, void *pOffsetInBytes, void *pStride);
	int (*SetStreamSourceFreq)(void *This, unsigned StreamNumber, unsigned Setting);
	int (*GetStreamSourceFreq)(void *This, unsigned StreamNumber, void *pSetting);
	int (*SetIndices)(void *This, void *pIndexData);
	int (*GetIndices)(void *This, void **ppIndexData);
	int (*CreatePixelShader)(void *This, const void *pFunction, void **ppShader);
	int (*SetPixelShader)(void *This, void *pShader);
	int (*GetPixelShader)(void *This, void **ppShader);
	int (*SetPixelShaderConstantF)(void *This, unsigned StartRegister, const void *pConstantData, unsigned Vector4fCount);
	int (*GetPixelShaderConstantF)(void *This, unsigned StartRegister, void *pConstantData, unsigned Vector4fCount);
	int (*SetPixelShaderConstantI)(void *This, unsigned StartRegister, const void *pConstantData, unsigned Vector4iCount);
	int (*GetPixelShaderConstantI)(void *This, unsigned StartRegister, void *pConstantData, unsigned Vector4iCount);
	int (*SetPixelShaderConstantB)(void *This, unsigned StartRegister, const void *pConstantData, unsigned BoolCount);
	int (*GetPixelShaderConstantB)(void *This, unsigned StartRegister, void *pConstantData, unsigned BoolCount);
	int (*DrawRectPatch)(void *This, unsigned Handle, const void *pNumSegs, const void *pRectPatchInfo);
	int (*DrawTriPatch)(void *This, unsigned Handle, const void *pNumSegs, const void *pTriPatchInfo);
	int (*DeletePatch)(void *This, unsigned Handle);
	int (*CreateQuery)(void *This, int Type, void **ppQuery);
} IDirect3DDevice9Vtbl;

typedef struct IDirect3DDevice9ExVtbl
{
	int (*QueryInterface)(void *This, void *riid, void **ppvObject);
	unsigned (*AddRef)(void *This);
	unsigned (*Release)(void *This);
	/* IDirect3DDevice9 */
	int (*TestCooperativeLevel)(void *This);
	unsigned (*GetAvailableTextureMem)(void *This);
	int (*EvictManagedResources)(void *This);
	int (*GetDirect3D)(void *This, void **ppD3D9);
	int (*GetDeviceCaps)(void *This, void *pCaps);
	int (*GetDisplayMode)(void *This, unsigned iSwapChain, void *pMode);
	int (*GetCreationParameters)(void *This, void *pParameters);
	int (*SetCursorProperties)(void *This, unsigned XHotSpot, unsigned YHotSpot, void *pCursorBitmap);
	void (*SetCursorPosition)(void *This, int X, int Y, unsigned Flags);
	int (*ShowCursor)(void *This, int bShow);
	int (*CreateAdditionalSwapChain)(void *This, void *pPresentationParameters, void **pSwapChain);
	int (*GetSwapChain)(void *This, unsigned iSwapChain, void **pSwapChain);
	unsigned (*GetNumberOfSwapChains)(void *This);
	int (*Reset)(void *This, void *pPresentationParameters);
	int (*Present)(void *This, const void *pSourceRect, const void *pDestRect, void *hDestWindowOverride, const void *pDirtyRegion);
	int (*GetBackBuffer)(void *This, unsigned iSwapChain, unsigned iBackBuffer, int Type, void **ppBackBuffer);
	int (*GetRasterStatus)(void *This, unsigned iSwapChain, void *pRasterStatus);
	int (*SetDialogBoxMode)(void *This, int bEnableDialogs);
	void (*SetGammaRamp)(void *This, unsigned iSwapChain, unsigned Flags, const void *pRamp);
	void (*GetGammaRamp)(void *This, unsigned iSwapChain, void *pRamp);
	int (*CreateTexture)(void *This, unsigned Width, unsigned Height, unsigned Levels, unsigned Usage, int Format, int Pool, void **ppTexture, void *pSharedHandle);
	int (*CreateVolumeTexture)(void *This, unsigned Width, unsigned Height, unsigned Depth, unsigned Levels, unsigned Usage, int Format, int Pool, void **ppVolumeTexture, void *pSharedHandle);
	int (*CreateCubeTexture)(void *This, unsigned EdgeLength, unsigned Levels, unsigned Usage, int Format, int Pool, void **ppCubeTexture, void *pSharedHandle);
	int (*CreateVertexBuffer)(void *This, unsigned Length, unsigned Usage, unsigned FVF, int Pool, void **ppVertexBuffer, void *pSharedHandle);
	int (*CreateIndexBuffer)(void *This, unsigned Length, unsigned Usage, int Format, int Pool, void **ppIndexBuffer, void *pSharedHandle);
	int (*CreateRenderTarget)(void *This, unsigned Width, unsigned Height, int Format, int MultiSample, unsigned MultisampleQuality, int Lockable, void **ppSurface, void *pSharedHandle);
	int (*CreateDepthStencilSurface)(void *This, unsigned Width, unsigned Height, int Format, int MultiSample, unsigned MultisampleQuality, int Discard, void **ppSurface, void *pSharedHandle);
	int (*UpdateSurface)(void *This, void *pSourceSurface, const void *pSourceRect, void *pDestinationSurface, const void *pDestPoint);
	int (*UpdateTexture)(void *This, void *pSourceTexture, void *pDestinationTexture);
	int (*GetRenderTargetData)(void *This, void *pRenderTarget, void *pDestSurface);
	int (*GetFrontBufferData)(void *This, unsigned iSwapChain, void *pDestSurface);
	int (*StretchRect)(void *This, void *pSourceSurface, const void *pSourceRect, void *pDestSurface, const void *pDestRect, int Filter);
	int (*ColorFill)(void *This, void *pSurface, const void *pRect, unsigned color);
	int (*CreateOffscreenPlainSurface)(void *This, unsigned Width, unsigned Height, int Format, int Pool, void **ppSurface, void *pSharedHandle);
	int (*SetRenderTarget)(void *This, unsigned RenderTargetIndex, void *pRenderTarget);
	int (*GetRenderTarget)(void *This, unsigned RenderTargetIndex, void **ppRenderTarget);
	int (*SetDepthStencilSurface)(void *This, void *pNewZStencil);
	int (*GetDepthStencilSurface)(void *This, void **ppZStencilSurface);
	int (*BeginScene)(void *This);
	int (*EndScene)(void *This);
	int (*Clear)(void *This, unsigned Count, const void *pRects, unsigned Flags, unsigned Color, float Z, unsigned Stencil);
	int (*SetTransform)(void *This, int State, const void *pMatrix);
	int (*GetTransform)(void *This, int State, void *pMatrix);
	int (*MultiplyTransform)(void *This, int State, const void *pMatrix);
	int (*SetViewport)(void *This, const void *pViewport);
	int (*GetViewport)(void *This, void *pViewport);
	int (*SetMaterial)(void *This, const void *pMaterial);
	int (*GetMaterial)(void *This, void *pMaterial);
	int (*SetLight)(void *This, unsigned Index, const void *pLight);
	int (*GetLight)(void *This, unsigned Index, void *pLight);
	int (*LightEnable)(void *This, unsigned Index, int Enable);
	int (*GetLightEnable)(void *This, unsigned Index, void *pEnable);
	int (*SetClipPlane)(void *This, unsigned Index, const void *pPlane);
	int (*GetClipPlane)(void *This, unsigned Index, void *pPlane);
	int (*SetRenderState)(void *This, int State, unsigned Value);
	int (*GetRenderState)(void *This, int State, void *pValue);
	int (*CreateStateBlock)(void *This, int Type, void **ppSB);
	int (*BeginStateBlock)(void *This);
	int (*EndStateBlock)(void *This, void **ppSB);
	int (*SetClipStatus)(void *This, const void *pClipStatus);
	int (*GetClipStatus)(void *This, void *pClipStatus);
	int (*GetTexture)(void *This, unsigned Stage, void **ppTexture);
	int (*SetTexture)(void *This, unsigned Stage, void *pTexture);
	int (*GetTextureStageState)(void *This, unsigned Stage, int Type, void *pValue);
	int (*SetTextureStageState)(void *This, unsigned Stage, int Type, unsigned Value);
	int (*GetSamplerState)(void *This, unsigned Sampler, int Type, void *pValue);
	int (*SetSamplerState)(void *This, unsigned Sampler, int Type, unsigned Value);
	int (*ValidateDevice)(void *This, void *pNumPasses);
	int (*SetPaletteEntries)(void *This, unsigned PaletteNumber, const void *pEntries);
	int (*GetPaletteEntries)(void *This, unsigned PaletteNumber, void *pEntries);
	int (*SetCurrentTexturePalette)(void *This, unsigned PaletteNumber);
	int (*GetCurrentTexturePalette)(void *This, void *PaletteNumber);
	int (*SetScissorRect)(void *This, const void *pRect);
	int (*GetScissorRect)(void *This, void *pRect);
	int (*SetSoftwareVertexProcessing)(void *This, int bSoftware);
	int (*GetSoftwareVertexProcessing)(void *This);
	int (*SetNPatchMode)(void *This, float nSegments);
	float (*GetNPatchMode)(void *This);
	int (*DrawPrimitive)(void *This, int PrimitiveType, unsigned StartVertex, unsigned PrimitiveCount);
	int (*DrawIndexedPrimitive)(void *This, int PrimitiveType, int BaseVertexIndex, unsigned MinVertexIndex, unsigned NumVertices, unsigned startIndex, unsigned primCount);
	int (*DrawPrimitiveUP)(void *This, int PrimitiveType, unsigned PrimitiveCount, const void *pVertexStreamZeroData, unsigned VertexStreamZeroStride);
	int (*DrawIndexedPrimitiveUP)(void *This, int PrimitiveType, unsigned MinVertexIndex, unsigned NumVertices, unsigned PrimitiveCount, const void *pIndexData, int IndexDataFormat, const void *pVertexStreamZeroData, unsigned VertexStreamZeroStride);
	int (*ProcessVertices)(void *This, unsigned SrcStartIndex, unsigned DestIndex, unsigned VertexCount, void *pDestBuffer, void *pVertexDecl, unsigned Flags);
	int (*CreateVertexDeclaration)(void *This, const void *pVertexElements, void **ppDecl);
	int (*SetVertexDeclaration)(void *This, void *pDecl);
	int (*GetVertexDeclaration)(void *This, void **ppDecl);
	int (*SetFVF)(void *This, unsigned FVF);
	int (*GetFVF)(void *This, void *pFVF);
	int (*CreateVertexShader)(void *This, const void *pFunction, void **ppShader);
	int (*SetVertexShader)(void *This, void *pShader);
	int (*GetVertexShader)(void *This, void **ppShader);
	int (*SetVertexShaderConstantF)(void *This, unsigned StartRegister, const void *pConstantData, unsigned Vector4fCount);
	int (*GetVertexShaderConstantF)(void *This, unsigned StartRegister, void *pConstantData, unsigned Vector4fCount);
	int (*SetVertexShaderConstantI)(void *This, unsigned StartRegister, const void *pConstantData, unsigned Vector4iCount);
	int (*GetVertexShaderConstantI)(void *This, unsigned StartRegister, void *pConstantData, unsigned Vector4iCount);
	int (*SetVertexShaderConstantB)(void *This, unsigned StartRegister, const void *pConstantData, unsigned BoolCount);
	int (*GetVertexShaderConstantB)(void *This, unsigned StartRegister, void *pConstantData, unsigned BoolCount);
	int (*SetStreamSource)(void *This, unsigned StreamNumber, void *pStreamData, unsigned OffsetInBytes, unsigned Stride);
	int (*GetStreamSource)(void *This, unsigned StreamNumber, void **ppStreamData, void *pOffsetInBytes, void *pStride);
	int (*SetStreamSourceFreq)(void *This, unsigned StreamNumber, unsigned Setting);
	int (*GetStreamSourceFreq)(void *This, unsigned StreamNumber, void *pSetting);
	int (*SetIndices)(void *This, void *pIndexData);
	int (*GetIndices)(void *This, void **ppIndexData);
	int (*CreatePixelShader)(void *This, const void *pFunction, void **ppShader);
	int (*SetPixelShader)(void *This, void *pShader);
	int (*GetPixelShader)(void *This, void **ppShader);
	int (*SetPixelShaderConstantF)(void *This, unsigned StartRegister, const void *pConstantData, unsigned Vector4fCount);
	int (*GetPixelShaderConstantF)(void *This, unsigned StartRegister, void *pConstantData, unsigned Vector4fCount);
	int (*SetPixelShaderConstantI)(void *This, unsigned StartRegister, const void *pConstantData, unsigned Vector4iCount);
	int (*GetPixelShaderConstantI)(void *This, unsigned StartRegister, void *pConstantData, unsigned Vector4iCount);
	int (*SetPixelShaderConstantB)(void *This, unsigned StartRegister, const void *pConstantData, unsigned BoolCount);
	int (*GetPixelShaderConstantB)(void *This, unsigned StartRegister, void *pConstantData, unsigned BoolCount);
	int (*DrawRectPatch)(void *This, unsigned Handle, const void *pNumSegs, const void *pRectPatchInfo);
	int (*DrawTriPatch)(void *This, unsigned Handle, const void *pNumSegs, const void *pTriPatchInfo);
	int (*DeletePatch)(void *This, unsigned Handle);
	int (*CreateQuery)(void *This, int Type, void **ppQuery);
	/* IDirect3DDevice9Ex */
	int (*SetConvolutionMonoKernel)(void *This, unsigned width, unsigned height, void *rows, void *columns);
	int (*ComposeRects)(void *This, void *pSrc, void *pDst, void *pSrcRectDescs, unsigned NumRects, void *pDstRectDescs, int Operation, int Xoffset, int Yoffset);
	int (*PresentEx)(void *This, const void *pSourceRect, const void *pDestRect, void *hDestWindowOverride, const void *pDirtyRegion, unsigned dwFlags);
	int (*GetGPUThreadPriority)(void *This, void *pPriority);
	int (*SetGPUThreadPriority)(void *This, int Priority);
	int (*WaitForVBlank)(void *This, unsigned iSwapChain);
	int (*CheckResourceResidency)(void *This, void **pResourceArray, unsigned NumResources);
	int (*SetMaximumFrameLatency)(void *This, unsigned MaxLatency);
	int (*GetMaximumFrameLatency)(void *This, void *pMaxLatency);
	int (*CheckDeviceState)(void *This, void *hDestinationWindow);
	int (*CreateRenderTargetEx)(void *This, unsigned Width, unsigned Height, int Format, int MultiSample, unsigned MultisampleQuality, int Lockable, void **ppSurface, void *pSharedHandle, unsigned Usage);
	int (*CreateOffscreenPlainSurfaceEx)(void *This, unsigned Width, unsigned Height, int Format, int Pool, void **ppSurface, void *pSharedHandle, unsigned Usage);
	int (*CreateDepthStencilSurfaceEx)(void *This, unsigned Width, unsigned Height, int Format, int MultiSample, unsigned MultisampleQuality, int Discard, void **ppSurface, void *pSharedHandle, unsigned Usage);
	int (*ResetEx)(void *This, void *pPresentationParameters, void *pFullscreenDisplayMode);
	int (*GetDisplayModeEx)(void *This, unsigned iSwapChain, void *pMode, void *pRotation);
} IDirect3DDevice9ExVtbl;

typedef struct IDirect3DDevice9VideoVtbl
{
	int (*QueryInterface)(void *This, void *riid, void **ppvObject);
	unsigned (*AddRef)(void *This);
	unsigned (*Release)(void *This);
	/* IDirect3DDevice9Video */
	int (*GetContentProtectionCaps)(void *This, const void *pCryptoType, const void *pDecodeProfile, void *pCaps);
	int (*CreateAuthenticatedChannel)(void *This, int ChannelType, void **ppAuthenticatedChannel, void *pChannelHandle);
	int (*CreateCryptoSession)(void *This, const void *pCryptoType, const void *pDecodeProfile, void **ppCryptoSession, void *pCryptoHandle);
} IDirect3DDevice9VideoVtbl;

typedef struct IDirect3DIndexBuffer9Vtbl
{
	int (*QueryInterface)(void *This, void *riid, void **ppvObject);
	unsigned (*AddRef)(void *This);
	unsigned (*Release)(void *This);
	/* IDirect3DResource9 */
	int (*GetDevice)(void *This, void **ppDevice);
	int (*SetPrivateData)(void *This, void *refguid, const void *pData, unsigned SizeOfData, unsigned Flags);
	int (*GetPrivateData)(void *This, void *refguid, void *pData, void *pSizeOfData);
	int (*FreePrivateData)(void *This, void *refguid);
	unsigned (*SetPriority)(void *This, unsigned PriorityNew);
	unsigned (*GetPriority)(void *This);
	void (*PreLoad)(void *This);
	int (*GetType)(void *This);
	/* IDirect3DIndexBuffer9 */
	int (*Lock)(void *This, unsigned OffsetToLock, unsigned SizeToLock, void **ppbData, unsigned Flags);
	int (*Unlock)(void *This);
	int (*GetDesc)(void *This, void *pDesc);
} IDirect3DIndexBuffer9Vtbl;

typedef struct IDirect3DPixelShader9Vtbl
{
	int (*QueryInterface)(void *This, void *riid, void **ppvObject);
	unsigned (*AddRef)(void *This);
	unsigned (*Release)(void *This);
	/* IDirect3DPixelShader9 */
	int (*GetDevice)(void *This, void **ppDevice);
	int (*GetFunction)(void *This, void *pData, void *pSizeOfData);
} IDirect3DPixelShader9Vtbl;

typedef struct IDirect3DQuery9Vtbl
{
	int (*QueryInterface)(void *This, void *riid, void **ppvObject);
	unsigned (*AddRef)(void *This);
	unsigned (*Release)(void *This);
	/* IDirect3DQuery9 */
	int (*GetDevice)(void *This, void **ppDevice);
	int (*GetType)(void *This);
	unsigned (*GetDataSize)(void *This);
	int (*Issue)(void *This, unsigned dwIssueFlags);
	int (*GetData)(void *This, void *pData, unsigned dwSize, unsigned dwGetDataFlags);
} IDirect3DQuery9Vtbl;

typedef struct IDirect3DResource9Vtbl
{
	int (*QueryInterface)(void *This, void *riid, void **ppvObject);
	unsigned (*AddRef)(void *This);
	unsigned (*Release)(void *This);
	/* IDirect3DResource9 */
	int (*GetDevice)(void *This, void **ppDevice);
	int (*SetPrivateData)(void *This, void *refguid, const void *pData, unsigned SizeOfData, unsigned Flags);
	int (*GetPrivateData)(void *This, void *refguid, void *pData, void *pSizeOfData);
	int (*FreePrivateData)(void *This, void *refguid);
	unsigned (*SetPriority)(void *This, unsigned PriorityNew);
	unsigned (*GetPriority)(void *This);
	void (*PreLoad)(void *This);
	int (*GetType)(void *This);
} IDirect3DResource9Vtbl;

typedef struct IDirect3DStateBlock9Vtbl
{
	int (*QueryInterface)(void *This, void *riid, void **ppvObject);
	unsigned (*AddRef)(void *This);
	unsigned (*Release)(void *This);
	/* IDirect3DStateBlock9 */
	int (*GetDevice)(void *This, void **ppDevice);
	int (*Capture)(void *This);
	int (*Apply)(void *This);
} IDirect3DStateBlock9Vtbl;

typedef struct IDirect3DSurface9Vtbl
{
	int (*QueryInterface)(void *This, void *riid, void **ppvObject);
	unsigned (*AddRef)(void *This);
	unsigned (*Release)(void *This);
	/* IDirect3DResource9 */
	int (*GetDevice)(void *This, void **ppDevice);
	int (*SetPrivateData)(void *This, void *refguid, const void *pData, unsigned SizeOfData, unsigned Flags);
	int (*GetPrivateData)(void *This, void *refguid, void *pData, void *pSizeOfData);
	int (*FreePrivateData)(void *This, void *refguid);
	unsigned (*SetPriority)(void *This, unsigned PriorityNew);
	unsigned (*GetPriority)(void *This);
	void (*PreLoad)(void *This);
	int (*GetType)(void *This);
	/* IDirect3DSurface9 */
	int (*GetContainer)(void *This, void *riid, void **ppContainer);
	int (*GetDesc)(void *This, void *pDesc);
	int (*LockRect)(void *This, void *pLockedRect, const void *pRect, unsigned Flags);
	int (*UnlockRect)(void *This);
	int (*GetDC)(void *This, void *phdc);
	int (*ReleaseDC)(void *This, void *hdc);
} IDirect3DSurface9Vtbl;

typedef struct IDirect3DSwapChain9Vtbl
{
	int (*QueryInterface)(void *This, void *riid, void **ppvObject);
	unsigned (*AddRef)(void *This);
	unsigned (*Release)(void *This);
	/* IDirect3DSwapChain9 */
	int (*Present)(void *This, const void *pSourceRect, const void *pDestRect, void *hDestWindowOverride, const void *pDirtyRegion, unsigned dwFlags);
	int (*GetFrontBufferData)(void *This, void *pDestSurface);
	int (*GetBackBuffer)(void *This, unsigned iBackBuffer, int Type, void **ppBackBuffer);
	int (*GetRasterStatus)(void *This, void *pRasterStatus);
	int (*GetDisplayMode)(void *This, void *pMode);
	int (*GetDevice)(void *This, void **ppDevice);
	int (*GetPresentParameters)(void *This, void *pPresentationParameters);
} IDirect3DSwapChain9Vtbl;

typedef struct IDirect3DSwapChain9ExVtbl
{
	int (*QueryInterface)(void *This, void *riid, void **ppvObject);
	unsigned (*AddRef)(void *This);
	unsigned (*Release)(void *This);
	/* IDirect3DSwapChain9 */
	int (*Present)(void *This, const void *pSourceRect, const void *pDestRect, void *hDestWindowOverride, const void *pDirtyRegion, unsigned dwFlags);
	int (*GetFrontBufferData)(void *This, void *pDestSurface);
	int (*GetBackBuffer)(void *This, unsigned iBackBuffer, int Type, void **ppBackBuffer);
	int (*GetRasterStatus)(void *This, void *pRasterStatus);
	int (*GetDisplayMode)(void *This, void *pMode);
	int (*GetDevice)(void *This, void **ppDevice);
	int (*GetPresentParameters)(void *This, void *pPresentationParameters);
	/* IDirect3DSwapChain9Ex */
	int (*GetLastPresentCount)(void *This, void *pLastPresentCount);
	int (*GetPresentStats)(void *This, void *pPresentationStatistics);
	int (*GetDisplayModeEx)(void *This, void *pMode, void *pRotation);
} IDirect3DSwapChain9ExVtbl;

typedef struct IDirect3DTexture9Vtbl
{
	int (*QueryInterface)(void *This, void *riid, void **ppvObject);
	unsigned (*AddRef)(void *This);
	unsigned (*Release)(void *This);
	/* IDirect3DResource9 */
	int (*GetDevice)(void *This, void **ppDevice);
	int (*SetPrivateData)(void *This, void *refguid, const void *pData, unsigned SizeOfData, unsigned Flags);
	int (*GetPrivateData)(void *This, void *refguid, void *pData, void *pSizeOfData);
	int (*FreePrivateData)(void *This, void *refguid);
	unsigned (*SetPriority)(void *This, unsigned PriorityNew);
	unsigned (*GetPriority)(void *This);
	void (*PreLoad)(void *This);
	int (*GetType)(void *This);
	/* IDirect3DBaseTexture9 */
	unsigned (*SetLOD)(void *This, unsigned LODNew);
	unsigned (*GetLOD)(void *This);
	unsigned (*GetLevelCount)(void *This);
	int (*SetAutoGenFilterType)(void *This, int FilterType);
	int (*GetAutoGenFilterType)(void *This);
	void (*GenerateMipSubLevels)(void *This);
	/* IDirect3DTexture9 */
	int (*GetLevelDesc)(void *This, unsigned Level, void *pDesc);
	int (*GetSurfaceLevel)(void *This, unsigned Level, void **ppSurfaceLevel);
	int (*LockRect)(void *This, unsigned Level, void *pLockedRect, const void *pRect, unsigned Flags);
	int (*UnlockRect)(void *This, unsigned Level);
	int (*AddDirtyRect)(void *This, const void *pDirtyRect);
} IDirect3DTexture9Vtbl;

typedef struct IDirect3DVertexBuffer9Vtbl
{
	int (*QueryInterface)(void *This, void *riid, void **ppvObject);
	unsigned (*AddRef)(void *This);
	unsigned (*Release)(void *This);
	/* IDirect3DResource9 */
	int (*GetDevice)(void *This, void **ppDevice);
	int (*SetPrivateData)(void *This, void *refguid, const void *pData, unsigned SizeOfData, unsigned Flags);
	int (*GetPrivateData)(void *This, void *refguid, void *pData, void *pSizeOfData);
	int (*FreePrivateData)(void *This, void *refguid);
	unsigned (*SetPriority)(void *This, unsigned PriorityNew);
	unsigned (*GetPriority)(void *This);
	void (*PreLoad)(void *This);
	int (*GetType)(void *This);
	/* IDirect3DVertexBuffer9 */
	int (*Lock)(void *This, unsigned OffsetToLock, unsigned SizeToLock, void **ppbData, unsigned Flags);
	int (*Unlock)(void *This);
	int (*GetDesc)(void *This, void *pDesc);
} IDirect3DVertexBuffer9Vtbl;

typedef struct IDirect3DVertexDeclaration9Vtbl
{
	int (*QueryInterface)(void *This, void *riid, void **ppvObject);
	unsigned (*AddRef)(void *This);
	unsigned (*Release)(void *This);
	/* IDirect3DVertexDeclaration9 */
	int (*GetDevice)(void *This, void **ppDevice);
	int (*GetDeclaration)(void *This, void *pElement, void *pNumElements);
} IDirect3DVertexDeclaration9Vtbl;

typedef struct IDirect3DVertexShader9Vtbl
{
	int (*QueryInterface)(void *This, void *riid, void **ppvObject);
	unsigned (*AddRef)(void *This);
	unsigned (*Release)(void *This);
	/* IDirect3DVertexShader9 */
	int (*GetDevice)(void *This, void **ppDevice);
	int (*GetFunction)(void *This, void *pData, void *pSizeOfData);
} IDirect3DVertexShader9Vtbl;

typedef struct IDirect3DVolume9Vtbl
{
	int (*QueryInterface)(void *This, void *riid, void **ppvObject);
	unsigned (*AddRef)(void *This);
	unsigned (*Release)(void *This);
	/* IDirect3DVolume9 */
	int (*GetDevice)(void *This, void **ppDevice);
	int (*SetPrivateData)(void *This, void *refguid, const void *pData, unsigned SizeOfData, unsigned Flags);
	int (*GetPrivateData)(void *This, void *refguid, void *pData, void *pSizeOfData);
	int (*FreePrivateData)(void *This, void *refguid);
	int (*GetContainer)(void *This, void *riid, void **ppContainer);
	int (*GetDesc)(void *This, void *pDesc);
	int (*LockBox)(void *This, void *pLockedVolume, const void *pBox, unsigned Flags);
	int (*UnlockBox)(void *This);
} IDirect3DVolume9Vtbl;

typedef struct IDirect3DVolumeTexture9Vtbl
{
	int (*QueryInterface)(void *This, void *riid, void **ppvObject);
	unsigned (*AddRef)(void *This);
	unsigned (*Release)(void *This);
	/* IDirect3DResource9 */
	int (*GetDevice)(void *This, void **ppDevice);
	int (*SetPrivateData)(void *This, void *refguid, const void *pData, unsigned SizeOfData, unsigned Flags);
	int (*GetPrivateData)(void *This, void *refguid, void *pData, void *pSizeOfData);
	int (*FreePrivateData)(void *This, void *refguid);
	unsigned (*SetPriority)(void *This, unsigned PriorityNew);
	unsigned (*GetPriority)(void *This);
	void (*PreLoad)(void *This);
	int (*GetType)(void *This);
	/* IDirect3DBaseTexture9 */
	unsigned (*SetLOD)(void *This, unsigned LODNew);
	unsigned (*GetLOD)(void *This);
	unsigned (*GetLevelCount)(void *This);
	int (*SetAutoGenFilterType)(void *This, int FilterType);
	int (*GetAutoGenFilterType)(void *This);
	void (*GenerateMipSubLevels)(void *This);
	/* IDirect3DVolumeTexture9 */
	int (*GetLevelDesc)(void *This, unsigned Level, void *pDesc);
	int (*GetVolumeLevel)(void *This, unsigned Level, void **ppVolumeLevel);
	int (*LockBox)(void *This, unsigned Level, void *pLockedVolume, const void *pBox, unsigned Flags);
	int (*UnlockBox)(void *This, unsigned Level);
	int (*AddDirtyBox)(void *This, const void *pDirtyBox);
} IDirect3DVolumeTexture9Vtbl;

