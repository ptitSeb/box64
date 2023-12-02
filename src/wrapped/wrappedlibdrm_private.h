#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh....
#endif

//GO(drmAddBufs, 
//GO(drmAddContextPrivateMapping, 
//GO(drmAddContextTag, 
//GO(drmAddMap, 
//GO(drmAgpAcquire, 
//GO(drmAgpAlloc, 
//GO(drmAgpBase, 
//GO(drmAgpBind, 
//GO(drmAgpDeviceId, 
//GO(drmAgpEnable, 
//GO(drmAgpFree, 
//GO(drmAgpGetMode, 
//GO(drmAgpMemoryAvail, 
//GO(drmAgpMemoryUsed, 
//GO(drmAgpRelease, 
//GO(drmAgpSize, 
//GO(drmAgpUnbind, 
//GO(drmAgpVendorId, 
//GO(drmAgpVersionMajor, 
//GO(drmAgpVersionMinor, 
GO(drmAuthMagic, iFiu)
//GO(drmAvailable, 
//GO(drmCheckModesettingSupported, 
//GO(drmClose, 
//GO(drmCloseOnce, 
//GO(drmCommandNone, 
GO(drmCommandRead, iFiLpL)
GO(drmCommandWrite, iFiLpL)
GO(drmCommandWriteRead, iFiLpL)
//GO(drmCreateContext, 
//GO(drmCreateDrawable, 
//GO(drmCrtcGetSequence, 
//GO(drmCrtcQueueSequence, 
//GO(drmCtlInstHandler, 
//GO(drmCtlUninstHandler, 
//GO(drmDelContextTag, 
//GO(drmDestroyContext, 
//GO(drmDestroyDrawable, 
//GO(drmDevicesEqual, 
//GO(drmDMA, 
//GO(drmDropMaster, 
//GO(drmError, 
//GO(drmFinish, 
//GO(drmFree, 
//GO(drmFreeBufs, 
//GO(drmFreeBusid, 
//GO(drmFreeDevice, 
GO(drmFreeDevices, vFpi)
//GO(drmFreeReservedContextList, 
GO(drmFreeVersion, vFp)
//GO(drmGetBufInfo, 
//GO(drmGetBusid, 
//GO(drmGetCap, 
//GO(drmGetClient, 
//GO(drmGetContextFlags, 
//GO(drmGetContextPrivateMapping, 
//GO(drmGetContextTag, 
//GO(drmGetDevice, 
//GO(drmGetDevice2, 
GO(drmGetDeviceNameFromFd, pFi)
//GO(drmGetDeviceNameFromFd2, 
GO(drmGetDevices, iFpi)
GO(drmGetDevices2, iFupi)
//GO(drmGetEntry, 
//GO(drmGetHashTable, 
//GO(drmGetInterruptFromBusID, 
//GO(drmGetLibVersion, 
//GO(drmGetLock, 
GO(drmGetMagic, iFip)
//GO(drmGetMap, 
GO(drmGetNodeTypeFromFd, iFi)
GO(drmGetPrimaryDeviceNameFromFd, pFi)
//GO(drmGetRenderDeviceNameFromFd, 
//GO(drmGetReservedContextList, 
//GO(drmGetStats, 
GO(drmGetVersion, pFi)
GO(drmHandleEvent, iFip)    //warning, p is *drmHandleEvent that needs wrapping, plenty of function pointer there
GO(drmHashCreate, pFv)
GO(drmHashDelete, iFpL)
GO(drmHashDestroy, iFp)
GO(drmHashFirst, iFppp)
GO(drmHashInsert, iFpLL)
GO(drmHashLookup, iFpLp)
GO(drmHashNext, iFppp)
GO(drmIoctl, iFiLp)
//GO(drmIsMaster, 
//GO(drmMalloc, 
//GO(drmMap, 
//GO(drmMapBufs, 
//GO(drmMarkBufs, 
GO(drmModeAddFB, iFiuuCCuup)
//GO(drmModeAddFB2, 
//GO(drmModeAddFB2WithModifiers, 
//GO(drmModeAtomicAddProperty, 
//GO(drmModeAtomicAlloc, 
//GO(drmModeAtomicCommit, 
//GO(drmModeAtomicDuplicate, 
//GO(drmModeAtomicFree, 
//GO(drmModeAtomicGetCursor, 
//GO(drmModeAtomicMerge, 
//GO(drmModeAtomicSetCursor, 
//GO(drmModeAttachMode, 
//GO(drmModeConnectorSetProperty, 
//GO(drmModeCreateLease, 
//GO(drmModeCreatePropertyBlob, 
//GO(drmModeCrtcGetGamma, 
//GO(drmModeCrtcSetGamma, 
//GO(drmModeDestroyPropertyBlob, 
//GO(drmModeDetachMode, 
//GO(drmModeDirtyFB, 
GO(drmModeFreeConnector, vFp)
GO(drmModeFreeCrtc, vFp)
GO(drmModeFreeEncoder, vFp)
//GO(drmModeFreeFB, 
//GO(drmModeFreeModeInfo, 
//GO(drmModeFreeObjectProperties, 
//GO(drmModeFreePlane, 
//GO(drmModeFreePlaneResources, 
//GO(drmModeFreeProperty, 
//GO(drmModeFreePropertyBlob, 
GO(drmModeFreeResources, vFp)
GO(drmModeGetConnector, pFiu)
//GO(drmModeGetConnectorCurrent, 
GO(drmModeGetCrtc, pFiu)
GO(drmModeGetEncoder, pFiu)
//GO(drmModeGetFB, 
//GO(drmModeGetLease, 
//GO(drmModeGetPlane, 
//GO(drmModeGetPlaneResources, 
//GO(drmModeGetProperty, 
//GO(drmModeGetPropertyBlob, 
GO(drmModeGetResources, pFi)
//GO(drmModeListLessees, 
//GO(drmModeMoveCursor, 
//GO(drmModeObjectGetProperties, 
//GO(drmModeObjectSetProperty, 
GO(drmModePageFlip, iFiuuup)
//GO(drmModePageFlipTarget, 
//GO(drmModeRevokeLease, 
GO(drmModeRmFB, iFiu)
GO(drmModeSetCrtc, iFiuuuupip)
//GO(drmModeSetCursor, 
//GO(drmModeSetCursor2, 
//GO(drmModeSetPlane, 
GOM(drmMsg, vFEpV)
//GO(drmOpen, 
//GO(drmOpenControl, 
//GO(drmOpenOnce, 
//GO(drmOpenOnceWithType, 
//GO(drmOpenRender, 
//GO(drmOpenWithType, 
GO(drmPrimeFDToHandle, iFiip)
GO(drmPrimeHandleToFD, iFiuup)
//GO(drmRandom, 
//GO(drmRandomCreate, 
//GO(drmRandomDestroy, 
//GO(drmRandomDouble, 
//GO(drmRmMap, 
//GO(drmScatterGatherAlloc, 
//GO(drmScatterGatherFree, 
//GO(drmSetBusid, 
//GO(drmSetClientCap, 
//GO(drmSetContextFlags, 
//GO(drmSetInterfaceVersion, 
//GO(drmSetMaster, 
//GO(drmSetServerInfo, 
//GO(drmSLCreate, 
//GO(drmSLDelete, 
//GO(drmSLDestroy, 
//GO(drmSLDump, 
//GO(drmSLFirst, 
//GO(drmSLInsert, 
//GO(drmSLLookup, 
//GO(drmSLLookupNeighbors, 
//GO(drmSLNext, 
//GO(drmSwitchToContext, 
GO(drmSyncobjCreate, iFiup)
GO(drmSyncobjDestroy, iFiu)
GO(drmSyncobjExportSyncFile, iFiup)
GO(drmSyncobjFDToHandle, iFiip)
GO(drmSyncobjHandleToFD, iFiup)
GO(drmSyncobjImportSyncFile, iFiui)
GO(drmSyncobjQuery, iFippu)
GO(drmSyncobjQuery2, iFippuu)
GO(drmSyncobjReset, iFipu)
GO(drmSyncobjSignal, iFipu)
GO(drmSyncobjTimelineSignal, iFippu)
GO(drmSyncobjTimelineWait, iFippuIup)
GO(drmSyncobjTransfer, iFiuUuUu)
GO(drmSyncobjWait, iFipuIup)
//GO(drmUnlock, 
//GO(drmUnmap, 
//GO(drmUnmapBufs, 
//GO(drmUpdateDrawableInfo, 
//GO(drmWaitVBlank, 
