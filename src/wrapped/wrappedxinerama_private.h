#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

GO(XineramaIsActive, iFp)
GO(XineramaQueryExtension, iFppp)
GO(XineramaQueryScreens, pFpp)
GO(XineramaQueryVersion, iFppp)
GO(XPanoramiXAllocInfo, pFv)
GO(XPanoramiXGetScreenCount, iFpLp)
GO(XPanoramiXGetScreenSize, iFpLip)
GO(XPanoramiXGetState, iFpLp)
GO(XPanoramiXQueryExtension, iFppp)
GO(XPanoramiXQueryVersion, iFppp)
