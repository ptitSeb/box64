#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

GO(vaAcquireBufferHandle, iFpup)
//GO(vaAssociateSubpicture, 
//GO(vaAttachProtectedSession, 
GO(vaBeginPicture, iFpup)
//GO(vaBufferInfo, 
//GO(vaBufferSetNumElements, 
//GO(vaBufferTypeStr, 
GO(vaConfigAttribTypeStr, pFu)
//GO(vaCopy, 
GO(vaCreateBuffer, iFpuiuupp)
//GO(vaCreateBuffer2, 
GO(vaCreateConfig, iFpiipip)
GO(vaCreateContext, iFppiiipip)
GO(vaCreateImage, iFppiip)
//GO(vaCreateMFContext, 
//GO(vaCreateProtectedSession, 
//GO(vaCreateSubpicture, 
//GO(vaCreateSurfaces@VA_API_0.32.0, 
GO(vaCreateSurfaces, iFpuuupupu)
//GO(vaDeassociateSubpicture, 
GO(vaDeriveImage, iFppp)
GO(vaDestroyBuffer, iFpu)
GO(vaDestroyConfig, iFpp)
GO(vaDestroyContext, iFpu)
GO(vaDestroyImage, iFpp)
//GO(vaDestroyProtectedSession, 
//GO(vaDestroySubpicture, 
GO(vaDestroySurfaces, iFppi)
//GO(vaDetachProtectedSession, 
GO(vaDisplayIsValid, iFp)
GO(vaEndPicture, iFpu)
GO(vaEntrypointStr, pFi)
//GO(va_errorMessage, 
GO(vaErrorStr, pFi)
GO(vaExportSurfaceHandle, iFppuup)
GO(vaGetConfigAttributes, iFpiipi)
//GO(vaGetDisplayAttributes, 
GO(vaGetImage, iFppiiuup)
//GO(vaGetLibFunc, 
//GO(va_infoMessage, 
GO(vaInitialize, iFppp)
//GO(vaLockSurface, 
GO(vaMapBuffer, iFpup)
GO(vaMaxNumConfigAttributes, iFp)
//GO(vaMaxNumDisplayAttributes, 
GO(vaMaxNumEntrypoints, iFp)
GO(vaMaxNumImageFormats, iFp)
GO(vaMaxNumProfiles, iFp)
GO(vaMaxNumSubpictureFormats, iFp)
//GO(vaMFAddContext, 
//GO(vaMFReleaseContext, 
//GO(vaMFSubmit, 
//GO(va_newDisplayContext, 
//GO(va_newDriverContext, 
//GO(va_parseConfig, 
GO(vaProfileStr, pFu)
//GO(vaProtectedSessionExecute, 
GO(vaPutImage, iFpppiiuuiiuu)
GO(vaQueryConfigAttributes, iFpppppp)
GO(vaQueryConfigEntrypoints, iFpipp)
GO(vaQueryConfigProfiles, iFppp)
//GO(vaQueryDisplayAttributes, 
GO(vaQueryImageFormats, iFppp)
GO(vaQueryProcessingRate, iFpppp)
GO(vaQuerySubpictureFormats, iFpppp)
GO(vaQuerySurfaceAttributes, iFpppp)
GO(vaQuerySurfaceError, iFpppp)
GO(vaQuerySurfaceStatus, iFppp)
GO(vaQueryVendorString, pFp)
//GO(vaQueryVideoProcFilterCaps, 
//GO(vaQueryVideoProcFilters, 
GO(vaQueryVideoProcPipelineCaps, iFpupup)
GO(vaReleaseBufferHandle, iFpu)
GO(vaRenderPicture, iFpupi)
//GO(vaSetDisplayAttributes, 
GO(vaSetDriverName, iFpp)
GOM(vaSetErrorCallback, pFEppp)
//GO(vaSetImagePalette, 
GOM(vaSetInfoCallback, pFEppp)
//GO(vaSetSubpictureChromakey, 
//GO(vaSetSubpictureGlobalAlpha, 
//GO(vaSetSubpictureImage, 
//GO(vaStatusStr, 
//GO(vaSyncBuffer, 
GO(vaSyncSurface, iFpp)
GO(vaSyncSurface2, iFppU)
GO(vaTerminate, iFp)
//DATAB(va_trace_flag, 
//GO(va_TracePutSurface, 
//GO(va_TraceStatus, 
//GO(vaUnlockSurface, 
GO(vaUnmapBuffer, iFpu)
