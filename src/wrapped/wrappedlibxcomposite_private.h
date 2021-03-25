#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

GO(XCompositeCreateRegionFromBorderClip, LFpL)
GO(XCompositeFindDisplay, pFp)
GO(XCompositeGetOverlayWindow, LFpL)
GO(XCompositeNameWindowPixmap, LFpL)
GO(XCompositeQueryExtension, iFppp)
GO(XCompositeQueryVersion, iFppp)
GO(XCompositeRedirectSubwindows, vFpLi)
GO(XCompositeRedirectWindow, vFpLi)
GO(XCompositeReleaseOverlayWindow, vFpL)
GO(XCompositeUnredirectSubwindows, vFpLi)
GO(XCompositeUnredirectWindow, vFpLi)
GO(XCompositeVersion, iFv)
