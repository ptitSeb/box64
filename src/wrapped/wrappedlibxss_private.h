#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh....
#endif

GO(XScreenSaverAllocInfo, pFv)
GO(XScreenSaverGetRegistered, uFpipp)
GO(XScreenSaverQueryExtension, iFppp)
GO(XScreenSaverQueryInfo, iFppp)
GO(XScreenSaverQueryVersion, iFppp)
GO(XScreenSaverRegister, vFpiup)
GO(XScreenSaverSelectInput, vFppu)
GO(XScreenSaverSetAttributes, vFppiiuuuiupup)
GO(XScreenSaverSuspend, vFpi)
GO(XScreenSaverUnregister, iFpi)
GO(XScreenSaverUnsetAttributes, vFpp)
