#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh....
#endif

GO(DPMSCapable, iFp)
GO(DPMSDisable, iFp)
GO(DPMSEnable, iFp)
GO(DPMSForceLevel, iFpu)
GO(DPMSGetTimeouts, iFpppp)
GO(DPMSGetVersion, iFppp)
GO(DPMSInfo, iFppp)
GO(DPMSQueryExtension, iFppp)
GO(DPMSSetTimeouts, iFpuuu)
//GO(XagCreateAssociation, 
//GO(XagCreateEmbeddedApplicationGroup, 
//GO(XagCreateNonembeddedApplicationGroup, 
//GO(XagDestroyApplicationGroup, 
//GO(XagDestroyAssociation, 
//GO(XagGetApplicationGroupAttributes, 
//GO(XagQueryApplicationGroup, 
//GO(XagQueryVersion, 
//GO(XcupGetReservedColormapEntries, 
//GO(XcupQueryVersion, 
//GO(XcupStoreColors, 
GO(XdbeAllocateBackBufferName, pFppu)
GO(XdbeBeginIdiom, iFp)
GO(XdbeDeallocateBackBufferName, iFpp)
GO(XdbeEndIdiom, iFp)
GO(XdbeFreeVisualInfo, vFp)
GO(XdbeGetBackBufferAttributes, pFpp)
GO(XdbeGetVisualInfo, pFppp)
GO(XdbeQueryExtension, iFppp)
GO(XdbeSwapBuffers, iFppi)
//GO(XeviGetVisualInfo, 
//GO(XeviQueryExtension, 
//GO(XeviQueryVersion, 
GOM(XextAddDisplay, pFEppppip)
GO(XextCreateExtension, pFv)
GO(XextDestroyExtension, vFp)
DATAB(_XExtensionErrorFunction, 4)
GO(XextFindDisplay, pFpp)
GO(XextRemoveDisplay, iFpp)
//GO(XGEQueryExtension, 
//GO(XGEQueryVersion, 
//GO(XLbxGetEventBase, 
//GO(XLbxQueryExtension, 
//GO(XLbxQueryVersion, 
//GO(XmbufChangeBufferAttributes, 
//GO(XmbufChangeWindowAttributes, 
//GO(XmbufClearBufferArea, 
//GO(XmbufCreateBuffers, 
//GO(XmbufCreateStereoWindow, 
//GO(XmbufDestroyBuffers, 
//GO(XmbufDisplayBuffers, 
//GO(XmbufGetBufferAttributes, 
//GO(XmbufGetScreenInfo, 
//GO(XmbufGetVersion, 
//GO(XmbufGetWindowAttributes, 
//GO(XmbufQueryExtension, 
GO(XMissingExtension, iFpp)
//GO(XMITMiscGetBugMode, 
//GO(XMITMiscQueryExtension, 
//GO(XMITMiscSetBugMode, 
//GO(XSecurityAllocXauth, 
//GO(XSecurityFreeXauth, 
//GO(XSecurityGenerateAuthorization, 
//GO(XSecurityQueryExtension, 
//GO(XSecurityRevokeAuthorization, 
GOM(XSetExtensionErrorHandler, pFEp)
GO(XShapeCombineMask, vFppiiipi)
GO(XShapeCombineRectangles, vFppiiipiii)
GO(XShapeCombineRegion, vFppiiipi)
GO(XShapeCombineShape, vFppiiipii)
GO(XShapeGetRectangles, pFppipp)
GO(XShapeInputSelected, uFpp)
GO(XShapeOffsetShape, vFppiii)
GO(XShapeQueryExtension, iFppp)
GO(XShapeQueryExtents, iFpppppppppppp)
GO(XShapeQueryVersion, iFppp)
GO(XShapeSelectInput, vFppu)
GO(XShmAttach, iFpp)
GOM(XShmCreateImage, pFEppuippuu)       //need brige/unbridge...
GO(XShmCreatePixmap, pFppppuuu)
GO(XShmDetach, iFpp)
GO(XShmGetEventBase, iFp)
GOM(XShmGetImage, iFEpppiiu)        //need brige/unbridge...
GO(XShmPixmapFormat, iFp)
GOM(XShmPutImage, iFEppppiiiiuui)       //need brige/unbridge...
GO(XShmQueryExtension, iFp)
GO(XShmQueryVersion, iFpppp)
GO(XSyncAwait, iFppi)
//GO(XSyncAwaitFence, 
GO(XSyncChangeAlarm, iFpuLp)
GO(XSyncChangeCounter, iFpuU)
GO(XSyncCreateAlarm, uFpLp)     // XSyncAlarm is an XID
GO(XSyncCreateCounter, uFpU)    // XSyncValue is a struct of 2 int
//GO(XSyncCreateFence, 
GO(XSyncDestroyAlarm, iFpu)
GO(XSyncDestroyCounter, iFpu)
//GO(XSyncDestroyFence, 
GO(XSyncFreeSystemCounterList, vFp)
GO(XSyncGetPriority, iFpup)
GO(XSyncInitialize, iFppp)
GO(XSyncIntsToValue, vFpui)
GO(XSyncIntToValue, vFpi)
GO(XSyncListSystemCounters, pFpp)
//GO(XSyncMaxValue, 
//GO(XSyncMinValue, 
GO(XSyncQueryAlarm, iFpup)
GO(XSyncQueryCounter, iFpup)
GO(XSyncQueryExtension, iFppp)
//GO(XSyncQueryFence, 
//GO(XSyncResetFence, 
GO(XSyncSetCounter, iFpuU)      // XSyncCounter is an XID
GO(XSyncSetPriority, iFpui)
//GO(XSyncTriggerFence, 
//GO(XSyncValueAdd, 
//GO(XSyncValueEqual, 
//GO(XSyncValueGreaterOrEqual, 
//GO(XSyncValueGreaterThan, 
//GO(XSyncValueHigh32, 
//GO(XSyncValueIsNegative, 
//GO(XSyncValueIsPositive, 
GO(XSyncValueIsZero, iFp)
//GO(XSyncValueLessOrEqual, 
//GO(XSyncValueLessThan, 
//GO(XSyncValueLow32, 
//GO(XSyncValueSubtract, 
DATA(XTestFakeAckType, sizeof(void*))
//GO(XTestFakeInput, 
//GO(XTestFlush, 
//GO(XTestGetInput, 
DATAB(XTestInputActionType, sizeof(void*))
//GO(XTestMovePointer, 
//GO(XTestPressButton, 
//GO(XTestPressKey, 
//GO(XTestQueryInputSize, 
//GO(XTestReset, 
//GO(XTestStopInput, 
