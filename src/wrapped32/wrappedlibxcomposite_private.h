#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

GO(XCompositeCreateRegionFromBorderClip, LFXL)
//GO(XCompositeFindDisplay, pFX)
GO(XCompositeGetOverlayWindow, LFXL)
GO(XCompositeNameWindowPixmap, LFXL)
GO(XCompositeQueryExtension, iFXpp)
GO(XCompositeQueryVersion, iFXpp)
GO(XCompositeRedirectSubwindows, vFXLi)
GO(XCompositeRedirectWindow, vFXLi)
GO(XCompositeReleaseOverlayWindow, vFXL)
GO(XCompositeUnredirectSubwindows, vFXLi)
GO(XCompositeUnredirectWindow, vFXLi)
GO(XCompositeVersion, iFv)
