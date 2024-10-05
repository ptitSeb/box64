#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

GO(XineramaIsActive, iFX)
GO(XineramaQueryExtension, iFXpp)
GO(XineramaQueryScreens, pFXp)
GO(XineramaQueryVersion, iFXpp)
//GO(XPanoramiXAllocInfo, bLiiiiiL_Fv)
//GO(XPanoramiXGetScreenCount, iFXLbLiiiiiL_)
//GO(XPanoramiXGetScreenSize, iFXLibLiiiiiL_)
//GO(XPanoramiXGetState, iFXLbLiiiiiL_)
GO(XPanoramiXQueryExtension, iFXpp)
GO(XPanoramiXQueryVersion, iFXpp)
