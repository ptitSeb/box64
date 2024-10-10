#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

GO(XScreenSaverAllocInfo, pFv)
GO(XScreenSaverGetRegistered, iFpipp)
GO(XScreenSaverQueryExtension, iFppp)
GO(XScreenSaverQueryInfo, iFpLp)
GO(XScreenSaverQueryVersion, iFppp)
GO(XScreenSaverRegister, iFpiLL)
GO(XScreenSaverSelectInput, vFpLL)
GO(XScreenSaverSetAttributes, vFpLiiuuuiupLp)   //need GOM?
GO(XScreenSaverSuspend, vFpi)
GO(XScreenSaverUnregister, iFpi)
GO(XScreenSaverUnsetAttributes, vFpL)
