#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

GO(XineramaIsActive, iFp)
GO(XineramaQueryExtension, iFppp)
GO(XineramaQueryScreens, pFpp)
GO(XineramaQueryVersion, iFppp)
GO(XPanoramiXAllocInfo, pFv)
GO(XPanoramiXGetScreenCount, iFppp)
GO(XPanoramiXGetScreenSize, iFppip)
GO(XPanoramiXGetState, iFppp)
GO(XPanoramiXQueryExtension, iFppp)
GO(XPanoramiXQueryVersion, iFppp)
