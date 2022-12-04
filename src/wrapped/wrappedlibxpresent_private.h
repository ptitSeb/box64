#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

DATAB(XPresentExtensionInfo, 3*sizeof(void*))
DATA(XPresentExtensionName, sizeof(void*))
GO(XPresentFindDisplay, pFp)
GO(XPresentFreeInput, vFppp)
GO(XPresentNotifyMSC, vFppuUUU)
GO(XPresentPixmap, vFpppuppiipppuUUUpi)
GO(XPresentQueryCapabilities, uFpp)
GO(XPresentQueryExtension, iFpppp)
GO(XPresentQueryVersion, iFppp)
GO(XPresentSelectInput, pFppu)
GO(XPresentVersion, iFv)
