#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

GO(XScreenSaverAllocInfo, pFv)
GO(XScreenSaverGetRegistered, iFXipp)
GO(XScreenSaverQueryExtension, iFXpp)
GO(XScreenSaverQueryInfo, iFXLp)
GO(XScreenSaverQueryVersion, iFXpp)
GO(XScreenSaverRegister, iFXiLL)
GO(XScreenSaverSelectInput, vFXLL)
//GO(XScreenSaverSetAttributes, vFXLiiuuuiupLp)
GO(XScreenSaverSuspend, vFXi)
GO(XScreenSaverUnregister, iFXi)
GO(XScreenSaverUnsetAttributes, vFXL)
