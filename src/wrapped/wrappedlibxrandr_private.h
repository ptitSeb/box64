#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

GO(XRRQueryVersion,iFppp)
GO(XRRFreeScreenConfigInfo,vFp)
GO(XRRGetCrtcInfo, pFppu)
GO(XRRListOutputProperties, pFpup)
GO(XRRQueryExtension, iFppp)
//GO(XRRAllocModeInfo
//GO(XRRGetProviderProperty
//GO(XRRListProviderProperties
GO(XRRRotations,uFpip)
//GO(XRRSetCrtcTransform
GO(XRRGetCrtcGammaSize, iFpu)
GO(XRRConfigRotations,uFpp)
GO(XRRGetProviderInfo, pFppp)
//GO(XRRConfigureOutputProperty
//GO(XRRSetOutputPrimary
GO(XRRFreeOutputInfo, vFp)
GO(XRRUpdateConfiguration,iFp)
GO(XRRGetScreenResources, pFpp)
GO(XRRConfigSizes,pFpp)
//GO(XRRQueryProviderProperty
//GO(XRRDeleteOutputProperty
//GO(XRRDeleteProviderProperty
GO(XRRSetScreenConfig,iFpppiuu)
GO(XRRAllocGamma, pFi)
GO(XRRSetScreenSize, vFppiiii)
GO(XRRSetScreenConfigAndRate,iFpppiuwu)
GO(XRRFreeScreenResources, vFp)
GO(XRRGetOutputPrimary, uFpp)
//GO(XRRCreateMode
GO(XRRConfigCurrentRate,wFp)
//GO(XRRDestroyMode
GO(XRRSetCrtcConfig, iFppuuiiuupi)
GO(XRRConfigCurrentConfiguration,uFpp)
GO(XRRSizes,pFpip)
//GO(XRRAddOutputMode
GO(XRRFreePanning, vFp)
GO(XRRFreeProviderResources, vFp)
//GO(XRRChangeProviderProperty
GO(XRRGetPanning, pFppu)
//GO(XRRSetProviderOffloadSink
GO(XRRGetScreenResourcesCurrent, pFpp)
GO(XRRConfigTimes,uFpp)
GO(XRRSetCrtcGamma, vFppp)
//GO(XRRSetProviderOutputSource
GO(XRRGetScreenSizeRange, iFpppppp)
GO(XRRRates,pFpiip)
GO(XRRFreeProviderInfo, vFp)
GO(XRRConfigRates,pFpip)
GO(XRRQueryOutputProperty, pFpup)
GO(XRRGetOutputProperty, iFpupiiiipppppp)
GO(XRRFreeGamma, vFp)
GO(XRRRootToScreen,iFpp)
GO(XRRGetScreenInfo,pFpp)
GO(XRRFreeCrtcInfo, vFp)
GO(XRRGetProviderResources, pFpp)
//GO(XRRFreeModeInfo
//GO(XRRChangeOutputProperty
GO(XRRGetCrtcGamma, pFpu)
GO(XRRSetPanning, pFppu)
GO(XRRSelectInput,vFppi)
GO(XRRGetCrtcTransform, iFppp)
GO(XRRTimes,uFpip)
//GO(XRRDeleteOutputMode
GO(XRRGetOutputInfo, pFppu)
//GO(XRRConfigureProviderProperty
GO(XRRAllocateMonitor, pFpi)
GO(XRRGetMonitors, pFppip)
GO(XRRSetMonitor, vFppp)
GO(XRRFreeMonitors, vFp)