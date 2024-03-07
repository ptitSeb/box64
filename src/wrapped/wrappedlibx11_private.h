#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

// all those bss stuff are suspicious
//DATAB(__bss_end__, 4)
//DATAB(_bss_end__, 4)
//DATAB(__bss_start, 4)
//DATAB(__bss_start__, 4)
//DATA(__data_start, 4)
//DATA(_edata, 4)
//DATAB(_end, 4)
//DATAB(__end__, 4)
// __exidx_end  // type r
// __exidx_start    // type r
// _fini
// _init
// KeySymToUcs4
DATAB(_qfree, 4)
//GO(_Utf8GetConvByName, 
//GO(XActivateScreenSaver
GOM(XAddConnectionWatch, iFEppp)
GO(XAddExtension, pFp)
GO(XAddHost, vFpp)
GO(XAddHosts, vFppi)
GO(XAddPixel, vFpl)
GO(XAddToExtensionList, vFpp)
//GO(XAddToSaveSet
GO(XAllocClassHint, pFv)
GO(XAllocColor, iFppp)
GO(XAllocColorCells, iFpLipupu)
GO(XAllocColorPlanes, iFpLipiiiippp)
GO(XAllocIconSize, pFv)
GO(_XAllocID, LFp)
GO(_XAllocIDs, vFppi)
GO(XAllocNamedColor, iFppppp)
//GO(_XAllocScratch, 
GO(XAllocSizeHints, pFv)
GO(XAllocStandardColormap, pFv)
//GO(_XAllocTemp, 
GO(XAllocWMHints, pFv)
GO(XAllowEvents, iFpiL)
GO(XAllPlanes, LFv)
GO(_XAsyncErrorHandler, iFpppip)
GO(XAutoRepeatOff, iFp)
GO(XAutoRepeatOn, iFp)
GO(XBaseFontNameListOfFontSet, pFp)
GO(XBell, iFpi)
GO(XBitmapBitOrder, iFp)
GO(XBitmapPad, iFp)
GO(XBitmapUnit, iFp)
GO(XBlackPixel, LFpi)
//GO(XBlackPixelOfScreen
//GO(XCellsOfScreen
GO(XChangeActivePointerGrab, iFpuLL)
GO(XChangeGC, iFppLp)
GO(XChangeKeyboardControl, iFpLp)
GO(XChangeKeyboardMapping, iFpiipi)
GO(XChangePointerControl, iFpiiiii)
GO(XChangeProperty, iFpLLLiipi)
//GO(XChangeSaveSet
GO(XChangeWindowAttributes, iFpLLp)
GOM(XCheckIfEvent, iFEpppp)
GO(XCheckMaskEvent, iFplp)
GO(XCheckTypedEvent, iFpip)
GO(XCheckTypedWindowEvent, iFpLip)
GO(XCheckWindowEvent, iFpplp)
GO(XCirculateSubwindows, iFpLi)
//GO(XCirculateSubwindowsDown
//GO(XCirculateSubwindowsUp
GO(XClearArea, iFpLiiuui)
GO(XClearWindow, iFpp)
GO(XClipBox, iFpp)
GO(XCloseDisplay, iFp)
GO(XCloseIM, iFp)
//GO(_XCloseLC, 
GO(XCloseOM, iFp)
//GO(_XcmsAddCmapRec, 
//GO(XcmsAddColorSpace
//GO(XcmsAddFunctionSet
//GO(XcmsAllocColor
//GO(XcmsAllocNamedColor
//GO(_XcmsArcTangent, 
//GO(XcmsCCCOfColormap
//GO(XcmsCIELabClipab
//GO(XcmsCIELabClipL
//GO(XcmsCIELabClipLab
DATA(XcmsCIELabColorSpace, 6*sizeof(void*))
//GO(_XcmsCIELab_prefix   // type r, 
//GO(XcmsCIELabQueryMaxC
//GO(XcmsCIELabQueryMaxL
//GO(XcmsCIELabQueryMaxLC
//GO(_XcmsCIELabQueryMaxLCRGB, 
//GO(XcmsCIELabQueryMinL
//GO(XcmsCIELabToCIEXYZ
//GO(XcmsCIELabWhiteShiftColors
//GO(XcmsCIELuvClipL
//GO(XcmsCIELuvClipLuv
//GO(XcmsCIELuvClipuv
DATA(XcmsCIELuvColorSpace, 6*sizeof(void*))
//GO(_XcmsCIELuv_prefix   // type r, 
//GO(XcmsCIELuvQueryMaxC
//GO(XcmsCIELuvQueryMaxL
//GO(XcmsCIELuvQueryMaxLC
//GO(_XcmsCIELuvQueryMaxLCRGB, 
//GO(XcmsCIELuvQueryMinL
//GO(XcmsCIELuvToCIEuvY
//GO(XcmsCIELuvWhiteShiftColors
DATA(XcmsCIEuvYColorSpace, 6*sizeof(void*))
//GO(_XcmsCIEuvY_prefix   // type r, 
//GO(XcmsCIEuvYToCIELuv
//GO(XcmsCIEuvYToCIEXYZ
//GO(XcmsCIEuvYToTekHVC
//GO(_XcmsCIEuvY_ValidSpec, 
DATA(XcmsCIExyYColorSpace, 6*sizeof(void*)) // probably need some wrapping
//GO(_XcmsCIExyY_prefix   // type r, 
//GO(XcmsCIExyYToCIEXYZ
DATA(XcmsCIEXYZColorSpace, 6*sizeof(void*))
//GO(_XcmsCIEXYZ_prefix   // type r, 
//GO(XcmsCIEXYZToCIELab
//GO(XcmsCIEXYZToCIEuvY
//GO(XcmsCIEXYZToCIExyY
//GO(XcmsCIEXYZToRGBi
//GO(_XcmsCIEXYZ_ValidSpec, 
//GO(XcmsClientWhitePointOfCCC
//GO(XcmsConvertColors
//GO(_XcmsConvertColorsWithWhitePt, 
//GO(_XcmsCopyCmapRecAndFree, 
//GO(_XcmsCopyISOLatin1Lowered, 
//GO(_XcmsCopyPointerArray, 
//GO(_XcmsCosine, 
//GO(XcmsCreateCCC
//GO(_XcmsCubeRoot, 
DATA(_XcmsDDColorSpaces, 6*sizeof(void*))
DATA(_XcmsDDColorSpacesInit, 6*sizeof(void*))
//GO(_XcmsDDConvertColors, 
//GO(XcmsDefaultCCC
//GO(_XcmsDeleteCmapRec, 
DATA(_XcmsDIColorSpaces, 6*sizeof(void*))
DATA(_XcmsDIColorSpacesInit, 6*sizeof(void*))
//GO(_XcmsDIConvertColors, 
//GO(XcmsDisplayOfCCC
//GO(_XcmsEqualWhitePts, 
GO(XcmsFormatOfPrefix, LFp)
//GO(XcmsFreeCCC
//GO(_XcmsFreeIntensityMaps, 
//GO(_XcmsFreePointerArray, 
//GO(_XcmsGetElement, 
//GO(_XcmsGetIntensityMap, 
//GO(_XcmsGetProperty, 
//GO(_XcmsInitDefaultCCCs, 
//GO(_XcmsInitScrnInfo, 
DATA(XcmsLinearRGBFunctionSet, 3*sizeof(void*)) // probably needs some wrapping
GO(XcmsLookupColor, iFpLpppL)
//GO(_XcmsLRGB_InitScrnDefault, 
GO(XcmsPrefixOfFormat, pFL)
//GO(_XcmsPushPointerArray, 
//GO(XcmsQueryBlack
//GO(XcmsQueryBlue
GO(XcmsQueryColor, iFpLpL)
GO(XcmsQueryColors, iFpLpuL)
//GO(XcmsQueryGreen
//GO(XcmsQueryRed
//GO(XcmsQueryWhite
DATA(_XcmsRegColorSpaces, 6*sizeof(void*))
//GO(_XcmsRegFormatOfPrefix, 
//GO(_XcmsResolveColor, 
//GO(_XcmsResolveColorString, 
DATA(XcmsRGBColorSpace, 6*sizeof(void*))
DATA(XcmsRGBiColorSpace, 6*sizeof(void*))
//GO(_XcmsRGBi_prefix // type r, 
//GO(XcmsRGBiToCIEXYZ
//GO(XcmsRGBiToRGB
//GO(_XcmsRGB_prefix  // type r, 
//GO(XcmsRGBToRGBi
//GO(_XcmsRGB_to_XColor, 
DATA(_XcmsSCCFuncSets, 3*sizeof(void*))
DATA(_XcmsSCCFuncSetsInit, 3*sizeof(void*))
//GO(XcmsScreenNumberOfCCC
//GO(XcmsScreenWhitePointOfCCC
//GO(XcmsSetCCCOfColormap
//GO(XcmsSetCompressionProc
//GO(_XcmsSetGetColor, 
//GO(_XcmsSetGetColors, 
//GO(XcmsSetWhiteAdjustProc
//GO(XcmsSetWhitePoint
//GO(_XcmsSine, 
//GO(_XcmsSquareRoot, 
GO(XcmsStoreColor, iFpLp)
GO(XcmsStoreColors, iFpLpup)
//GO(_XcmsTekHVC_CheckModify, 
//GO(XcmsTekHVCClipC
//GO(XcmsTekHVCClipV
//GO(XcmsTekHVCClipVC
DATA(XcmsTekHVCColorSpace, 6*sizeof(void*))
//GO(_XcmsTekHVC_prefix   // type r, 
//GO(XcmsTekHVCQueryMaxC
//GO(XcmsTekHVCQueryMaxV
//GO(XcmsTekHVCQueryMaxVC
//GO(_XcmsTekHVCQueryMaxVCRGB, 
//GO(XcmsTekHVCQueryMaxVSamples
//GO(XcmsTekHVCQueryMinV
//GO(XcmsTekHVCToCIEuvY
//GO(XcmsTekHVCWhiteShiftColors
DATA(XcmsUNDEFINEDColorSpace, 6*sizeof(void*))
//GO(_XcmsUnresolveColor, 
//GO(XcmsVisualOfCCC
//GO(_XColor_to_XcmsRGB, 
GO(XConfigureWindow, iFpLup)
GO(XConnectionNumber, iFp)
//GO(_XConnectXCB, 
//GO(XContextDependentDrawing
GO(XContextualDrawing, iFp)
GO(XConvertCase, vFLpp)
GO(XConvertSelection, iFpLLLLL)
GO(XCopyArea, iFpLLpiiuuii)
GO(XCopyColormapAndFree, LFpL)
//GO(_XCopyEventCookie, 
GO(XCopyGC, iFppLp)
GO(XCopyPlane, iFpLLpiiuuiiL)
//GO(_XCopyToArg, 
GO(XCreateBitmapFromData, LFpLpuu)
GO(XCreateColormap, pFpppi)
GO(XCreateFontCursor, LFpu)
GO(XCreateFontSet, pFppppp)
GO(XCreateGC, pFpLLp)
GO(XCreateGlyphCursor, LFpLLuupp)
GOM(XCreateIC, pFEpV)     // use vararg
GOM(XCreateImage, pFEppuiipuuii)
GO(dummy_XCreateImage, pFppuiipuuii)    // to have the wrapper
DATAB(_XCreateMutex_fn, sizeof(void*))
GO(XCreateOC, pFp)
GO(XCreatePixmap, LFpLuuu)
GO(XCreatePixmapCursor, LFpLLppuu)
GO(XCreatePixmapFromBitmapData, LFpLpuuLLu)
GO(XCreateRegion, pFv)
GO(XCreateSimpleWindow, LFpLiiuuuLL)
GO(XCreateWindow, pFppiiuuuiupLp)
DATAB(_Xdebug, sizeof(void*))
GO(XDefaultColormap, LFpi)
GO(XDefaultColormapOfScreen, LFp)
GO(XDefaultDepth, iFpi)
GO(XDefaultDepthOfScreen, iFp)
//GO(_XDefaultError, 
GO(XDefaultGC, pFpi)
GO(XDefaultGCOfScreen, pFp)
GO(_XDefaultIOError, iFp)
//GO(_XDefaultOpenIM, 
//GO(_XDefaultOpenOM, 
GO(XDefaultRootWindow, LFp)
GO(XDefaultScreen, iFp)
GO(XDefaultScreenOfDisplay, pFp)
GO(XDefaultString, pFv)
GO(XDefaultVisual, pFpi)
GO(XDefaultVisualOfScreen, pFp)
//GO(_XDefaultWireError, 
GO(XDefineCursor, iFpLL)
GO(XDeleteContext, iFpLi)
//GO(XDeleteModifiermapEntry
GO(XDeleteProperty, iFpLL)
//GO(_XDeq, 
GOM(_XDeqAsyncHandler, vFEpp)
GO(XDestroyIC, vFp)
GOM(XDestroyImage, iFEp)  //need to unbridge
GO(XDestroyOC, vFp)
GO(XDestroyRegion, iFp)
GO(XDestroySubwindows, iFpp)
GO(XDestroyWindow, iFpp)
//GO(XDirectionalDependentDrawing
GO(XDisableAccessControl, vFp)
GO(XDisplayCells, iFpi)
GO(XDisplayHeight, iFpi)
GO(XDisplayHeightMM, iFpi)
GO(XDisplayKeycodes, iFppp)
//GO(XDisplayMotionBufferSize
GO(XDisplayName, pFp)
GO(XDisplayOfIM, pFp)
GO(XDisplayOfOM, pFp)
GO(XDisplayOfScreen, pFp)
//GO(XDisplayPlanes
GO(XDisplayString, pFp)
GO(XDisplayWidth, iFpi)
GO(XDisplayWidthMM, iFpi)
//GO(XDoesBackingStore
//GO(XDoesSaveUnders
GO(XDrawArc, iFpppiiuuii)
//GO(XDrawArcs
//GO(XDrawImageString
//GO(XDrawImageString16
GO(XDrawLine, iFpLpiiii)
GO(XDrawLines, iFppppii)
GO(XDrawPoint, iFpLpii)
GO(XDrawPoints, iFpLppii)
GO(XDrawRectangle, iFpppiiuu)
GO(XDrawRectangles, iFppppi)
GO(XDrawSegments, iFpLppi)
GO(XDrawString, iFpLpiipi)
GO(XDrawString16, iFpLpiipi)
GO(XDrawText, iFpLpiipi)
GO(XDrawText16, iFpLpiipi)
GO(_XEatData, vFpL)
GO(_XEatDataWords, vFpL)
//GO(XEHeadOfExtensionList
GO(XEmptyRegion, iFp)
GO(XEnableAccessControl, vFp)
GO(_XEnq, vFpp)
GO(XEqualRegion, iFpp)
//GO(_XError, 
DATAB(_XErrorFunction, sizeof(void*))
//GO(XESetBeforeFlush
GOM(XESetCloseDisplay, pFEpip)
//GO(XESetCopyEventCookie
//GO(XESetCopyGC
//GO(XESetCreateFont
//GO(XESetCreateGC
GOM(XESetError, pFEpip)
//GO(XESetErrorString
GOM(XESetEventToWire, pFEpip)
//GO(XESetFlushGC
//GO(XESetFreeFont
//GO(XESetFreeGC
//GO(XESetPrintErrorValues
//GO(XESetWireToError
GOM(XESetWireToEvent, pFEpip)
//GO(XESetWireToEventCookie
GO(XEventMaskOfScreen, lFp)
GO(XEventsQueued, iFpi)
//GO(_XEventsQueued, 
//GO(_Xevent_to_mask  // type r, 
//GO(_XEventToWire, 
GO(XExtendedMaxRequestSize, lFp)
GO(XExtentsOfFontSet, pFp)
//GO(_XF86BigfontFreeFontMetrics, 
//GO(_XF86LoadQueryLocaleFont, 
GO(XFetchBuffer, pFppi)
GO(XFetchBytes, pFpp)
//GO(_XFetchEventCookie, 
GO(XFetchName, iFppp)
GO(XFillArc, iFpLpiiuuii)
GO(XFillArcs, iFpLppi)
GO(XFillPolygon, iFpLppiii)
GO(XFillRectangle, iFpLpiiuu)
GO(XFillRectangles, iFpLppi)
GO(XFilterEvent, iFpL)
GO(XFindContext, iFpLip)
//GO(XFindOnExtensionList
GO(XFlush, iFp)
GO(_XFlush, vFp)
GO(XFlushGC, vFpp)
GO(_XFlushGCCache, vFpp)
GO(XFontsOfFontSet, iFppp)
GO(XForceScreenSaver, iFpi)
GO(XFree, iFp)
//GO(_XFreeAtomTable, 
GO(XFreeColormap, iFpp)
GO(XFreeColors, iFpLpiL)
GO(XFreeCursor, iFpL)
DATAB(_XFreeDisplayLock_fn, sizeof(void*))
//GO(_XFreeDisplayStructure, 
//GO(_XFreeEventCookies, 
GO(XFreeEventData, vFpp)
//GO(_XFreeExtData, 
GO(XFreeExtensionList, iFp)
GO(XFreeFont, iFpp)
GO(XFreeFontInfo, iFppi)
GO(XFreeFontNames, iFp)
GO(XFreeFontPath, iFp)
GO(XFreeFontSet, vFpp)
GO(XFreeGC, iFpp)
GO(XFreeModifiermap, iFp)
DATAB(_XFreeMutex_fn, sizeof(void*))
GO(XFreePixmap, iFpL)
GO(XFreeStringList, vFp)
//GO(_XFreeTemp, 
//GO(_XFreeX11XCBStructure, 
GO(XGContextFromGC, LFp)
GO(XGeometry, iFpippuuuiipppp)
GO(_XGetAsyncData, vFpppiiii)
GO(_XGetAsyncReply, pFppppiii)
GO(XGetAtomName, pFpL)
GO(XGetAtomNames, iFppip)
//GO(_XGetBitsPerPixel, 
GO(XGetClassHint, iFppp)
GO(XGetCommand, iFpLpp)
GO(XGetDefault, pFppp)
GO(XGetErrorDatabaseText, iFpppppi)
GO(XGetErrorText, iFpipi)
GO(XGetEventData, iFpp)
GO(XGetFontPath, pFpp)
GO(XGetFontProperty, iFpLp)
GO(XGetGCValues, iFppLp)
GO(XGetGeometry, iFpLppppppp)
GO(_XGetHostname, iFpi)
GO(XGetIconName, iFpLp)
GO(XGetIconSizes, iFpLpp)
GO(XGetICValues, pFpppppppppp)      // use varargs...
GOM(XGetImage, pFEppiiuuLi)     // return an XImage with callbacks that needs wrapping
GO(XGetIMValues, pFppppppp)     // use varargs
GO(XGetInputFocus, iFppp)
GO(XGetKeyboardControl, iFpp)
GO(XGetKeyboardMapping, pFpCip)
//GO(_XGetLCValues, 
GO(XGetModifierMapping, pFp)
GO(XGetMotionEvents, pFpLLLp)
GO(XGetNormalHints, iFppp)
GO(XGetOCValues, pFpppppppppp) // use varargs
GO(XGetOMValues, pFp)
//GOM(XGetPixel, LFEpii)  // need unbridging
GO(dummy_XGetPixel, LFpii)     // for the wrapper
GO(XGetPointerControl, iFpppp)
GO(XGetPointerMapping, iFppi)
GOM(_XGetRequest, pFEpuL)
GO(XGetRGBColormaps, iFpLppL)
//GO(_XGetScanlinePad, 
GO(XGetScreenSaver, iFppppp)
GO(XGetSelectionOwner, LFpL)
//GO(XGetSizeHints
GO(XGetStandardColormap, iFpLpL)
GOM(XGetSubImage, pFEppiiuuuipii)
GO(XGetTextProperty, iFpLpL)
GO(XGetTransientForHint, iFpLp)
GO(XGetVisualInfo, pFplpp)
GO(XGetWindowAttributes, iFppp)
//GO(_XGetWindowAttributes, 
GO(XGetWindowProperty, iFpLLlliLppppp)
//GO(XGetWMClientMachine
GO(XGetWMColormapWindows, iFpLpp)
GO(XGetWMHints, pFpL)
GO(XGetWMIconName, iFppp)
GO(XGetWMName, iFpLp)
GO(XGetWMNormalHints, iFpLpp)
GO(XGetWMProtocols, iFpLpp)
GO(XGetWMSizeHints, iFpLppL)
//GO(XGetZoomHints
DATAB(_Xglobal_lock, sizeof(void*))
GO(XGrabButton, iFpuuLiuiiLL)
GO(XGrabKey, iFpiuLiii)
GO(XGrabKeyboard, iFpLiiiL)
GO(XGrabPointer, iFpLiuiiLLL)
GO(XGrabServer, iFp)
DATAB(_XHeadOfDisplayList, sizeof(void*))
GO(XHeightMMOfScreen, iFp)
GO(XHeightOfScreen, iFp)
DATAB(_Xi18n_lock, sizeof(void*))
GO(XIconifyWindow, iFppi)
GOM(XIfEvent, iFEpppp)
GO(XImageByteOrder, iFp)
//GO(_XIMCompileResourceList, 
//GO(_XimGetCharCode, 
//GO(_XimGetLocaleCode, 
//GO(_XimLookupMBText, 
//GO(_XimLookupUTF8Text, 
//GO(_XimLookupWCText, 
GO(XIMOfIC, pFp)
//GO(_XimXTransBytesReadable, 
//GO(_XimXTransClose, 
//GO(_XimXTransCloseForCloning, 
//GO(_XimXTransConnect, 
//GO(_XimXTransDisconnect, 
//GO(_XimXTransFreeConnInfo, 
//GO(_XimXTransGetConnectionNumber, 
//GO(_XimXTransGetHostname, 
//GO(_XimXTransGetMyAddr, 
//GO(_XimXTransGetPeerAddr, 
//GO(_XimXTransIsLocal, 
//GO(_XimXTransOpenCLTSClient, 
//GO(_XimXTransOpenCOTSClient, 
//GO(_XimXTransRead, 
//GO(_XimXTransReadv, 
//GO(_XimXTransSetOption, 
DATA(_XimXTransSocketINET6Funcs, sizeof(void*))
DATA(_XimXTransSocketINETFuncs, sizeof(void*))
DATA(_XimXTransSocketLocalFuncs, sizeof(void*))
DATA(_XimXTransSocketTCPFuncs, sizeof(void*))
DATA(_XimXTransSocketUNIXFuncs, sizeof(void*))
//GO(_XimXTransWrite, 
//GO(_XimXTransWritev, 
//GO(_XInitDefaultIM, 
//GO(_XInitDefaultOM, 
DATAB(_XInitDisplayLock_fn, sizeof(void*))
//GO(_XInitDynamicIM, 
//GO(_XInitDynamicOM, 
GO(XInitExtension, pFpp)
GOM(XInitImage, iFEp)
//GO(_XInitImageFuncPtrs, 
//GO(_XInitKeysymDB, 
GO(XInitThreads, iFv)
GO(XInsertModifiermapEntry, pFpCi)
GO(XInstallColormap, iFpp)
GO(XInternalConnectionNumbers, iFppp)
GO(XInternAtom, LFppi)
GO(XInternAtoms, iFppiip)
GO(XIntersectRegion, iFppp)
GO(_XIOError, iFp)
DATAB(_XIOErrorFunction, sizeof(void*))
//GO(_XIsEventCookie, 
GO(XkbAddDeviceLedInfo, pFpuu)
//GO(XkbAddGeomColor
//GO(XkbAddGeomDoodad
//GO(XkbAddGeomKey
//GO(XkbAddGeomKeyAlias
//GO(XkbAddGeomOutline
//GO(XkbAddGeomOverlay
//GO(XkbAddGeomOverlayKey
//GO(XkbAddGeomOverlayRow
//GO(XkbAddGeomProperty
//GO(XkbAddGeomRow
//GO(XkbAddGeomSection
//GO(XkbAddGeomShape
GO(XkbAddKeyType, pFpLiii)
//GO(XkbAllocClientMap
GO(XkbAllocCompatMap, iFpuu)
GO(XkbAllocControls, iFpu)
GO(XkbAllocDeviceInfo, pFuuu)
//GO(XkbAllocGeomColors
//GO(XkbAllocGeomDoodads
//GO(XkbAllocGeometry
//GO(XkbAllocGeomKeyAliases
//GO(XkbAllocGeomKeys
//GO(XkbAllocGeomOutlines
//GO(XkbAllocGeomOverlayKeys
//GO(XkbAllocGeomOverlayRows
//GO(XkbAllocGeomOverlays
//GO(XkbAllocGeomPoints
//GO(XkbAllocGeomProps
//GO(XkbAllocGeomRows
//GO(XkbAllocGeomSectionDoodads
//GO(XkbAllocGeomSections
//GO(XkbAllocGeomShapes
GO(XkbAllocIndicatorMaps, iFp)
GO(XkbAllocKeyboard, pFv)
GO(XkbAllocNames, iFpuii)
//GO(XkbAllocServerMap
GO(XkbApplyCompatMapToKey, iFpCp)
GO(XkbApplyVirtualModChanges, iFpup)
GO(XkbBell, iFppip)
GO(XkbBellEvent, iFpLiL)
GO(XkbChangeDeviceInfo, iFppp)
//GO(XkbChangeEnabledControls
GO(XkbChangeKeycodeRange, iFpiip)
GO(XkbChangeMap, iFppp)
GO(XkbChangeNames, iFppp)
GO(XkbChangeTypesOfKey, iFpiiupp)
GO(XkbComputeEffectiveMap, iFppp)
//GO(XkbComputeRowBounds
//GO(XkbComputeSectionBounds
//GO(XkbComputeShapeBounds
//GO(XkbComputeShapeTop
//GO(_XkbCopyFromReadBuffer, 
GO(XkbCopyKeyType, iFpp)
GO(XkbCopyKeyTypes, iFppi)
//GO(XkbDeviceBell
GO(XkbDeviceBellEvent, iFpLiiiiL)
//GO(XkbFindOverlayForKey
GO(XkbForceBell, iFpi)
GO(XkbForceDeviceBell, iFpiiii)
GO(XkbFreeClientMap, vFpui)
//GO(XkbFreeCompatMap
GO(XkbFreeComponentList, vFp)
//GO(XkbFreeControls
GO(XkbFreeDeviceInfo, vFpui)
//GO(XkbFreeGeomColors
//GO(XkbFreeGeomDoodads
//GO(XkbFreeGeometry
//GO(XkbFreeGeomKeyAliases
//GO(XkbFreeGeomKeys
//GO(XkbFreeGeomOutlines
//GO(XkbFreeGeomOverlayKeys
//GO(XkbFreeGeomOverlayRows
//GO(XkbFreeGeomOverlays
//GO(XkbFreeGeomPoints
//GO(XkbFreeGeomProperties
//GO(XkbFreeGeomRows
//GO(XkbFreeGeomSections
//GO(XkbFreeGeomShapes
GO(XkbFreeIndicatorMaps, vFp)
GO(XkbFreeKeyboard, vFpui)
GO(XkbFreeNames, vFpui)
//GO(_XkbFreeReadBuffer, 
//GO(XkbFreeServerMap
DATA(_XkbGetAtomNameFunc, sizeof(void*))
//GO(XkbGetAutoRepeatRate
GO(XkbGetAutoResetControls, iFppp)
//GO(_XkbGetCharset, 
//GO(XkbGetCompatMap
GO(XkbGetControls, iFpLp)
//GO(_XkbGetConverters, 
GO(XkbGetDetectableAutoRepeat, iFpp)
GO(XkbGetDeviceButtonActions, iFppiuu)
GO(XkbGetDeviceInfo, pFpuuuu)
//GO(XkbGetDeviceInfoChanges
//GO(XkbGetDeviceLedInfo
//GO(XkbGetGeometry
//GO(XkbGetIndicatorMap
GO(XkbGetIndicatorState, iFpup)
//GO(XkbGetKeyActions
//GO(XkbGetKeyBehaviors
GO(XkbGetKeyboard, pFpuu)
GO(XkbGetKeyboardByName, pFpupuui)
//GO(XkbGetKeyExplicitComponents
//GO(XkbGetKeyModifierMap
//GO(XkbGetKeySyms
//GO(XkbGetKeyTypes
GO(XkbGetKeyVirtualModMap, iFpuup)
GO(XkbGetMap, pFpuu)
//GO(XkbGetMapChanges
GO(XkbGetNamedDeviceIndicator, iFpuuuLpppp)
//GO(XkbGetNamedGeometry
GO(XkbGetNamedIndicator, iFpLpppp)
GO(XkbGetNames, iFpup)
GO(XkbGetPerClientControls, iFpp)
//GO(_XkbGetReadBufferCountedString, 
//GO(_XkbGetReadBufferPtr, 
GO(XkbGetState, iFpup)
GO(XkbGetUpdatedMap, iFpup)
//GO(XkbGetVirtualMods
GO(XkbGetXlibControls, uFp)
GO(XkbIgnoreExtension, iFi)
GO(XkbInitCanonicalKeyTypes, iFpui)
//GO(_XkbInitReadBuffer, 
DATA(_XkbInternAtomFunc, sizeof(void*))
GO(XkbKeycodeToKeysym, LFpCii)
GO(XkbKeysymToModifiers, uFpL)
GO(XkbKeyTypesForCoreSymbols, iFpipupp)
GO(XkbLatchGroup, iFpuu)
GO(XkbLatchModifiers, iFpuuu)
GO(XkbLibraryVersion, iFpp)
GO(XkbListComponents, pFpupp)
//GO(XkbLockGroup
//GO(XkbLockModifiers
GO(XkbLookupKeyBinding, iFpLupip)
GO(XkbLookupKeySym, iFpCupp)
GO(XkbNoteControlsChanges, vFppu)
//GO(_XkbNoteCoreMapChanges, 
GO(XkbNoteDeviceChanges, vFppu)
GO(XkbNoteMapChanges, vFppu)
GO(XkbNoteNameChanges, vFppu)
GO(XkbOpenDisplay, pFpppppp)
//GO(_XkbPeekAtReadBuffer, 
GO(XkbQueryExtension, iFpppppp)
//GO(_XkbReadBufferCopyKeySyms, 
//GO(_XkbReadCopyKeySyms, 
//GO(_XkbReadGetCompatMapReply, 
//GO(_XkbReadGetGeometryReply, 
//GO(_XkbReadGetIndicatorMapReply, 
//GO(_XkbReadGetMapReply, 
//GO(_XkbReadGetNamesReply, 
GO(XkbRefreshKeyboardMapping, iFp)
//GO(_XkbReloadDpy, 
GO(XkbResizeDeviceButtonActions, iFpu)
GO(XkbResizeKeyActions, pFpii)
GO(XkbResizeKeySyms, pFpii)
GO(XkbResizeKeyType, iFpiiii)
GO(XkbSelectEventDetails, iFpuuLL)
GO(XkbSelectEvents, iFpuLL)
//GO(XkbSetAtomFuncs
//GO(XkbSetAutoRepeatRate
GO(XkbSetAutoResetControls, iFpupp)
GO(XkbSetCompatMap, iFpupi)
//GO(XkbSetControlslib
GO(XkbSetDebuggingFlags, iFpuupuupp)
GO(XkbSetDetectableAutoRepeat, iFpip)
GO(XkbSetDeviceButtonActions, iFppuu)
GO(XkbSetDeviceInfo, iFpup)
GO(XkbSetDeviceLedInfo, iFppuuu)
//GO(XkbSetGeometry
GO(XkbSetIgnoreLockMods, iFpuuuuu)
//GO(XkbSetIndicatorMap
GO(XkbSetMap, iFpup)
GO(XkbSetNamedDeviceIndicator, iFpuuuLiiip)
GO(XkbSetNamedIndicator, iFpLiiip)
GO(XkbSetNames, iFpuuup)
GO(XkbSetPerClientControls, iFpup)
//GO(XkbSetServerInternalMods
GO(XkbSetXlibControls, uFpuu)
//GO(_XkbSkipReadBufferData, 
GO(XkbToControl, CFC)
//GO(XkbTranslateKey
GO(XkbTranslateKeyCode, iFpCupp)
GO(XkbTranslateKeySym, iFppupip)
GO(XkbUpdateActionVirtualMods, iFppu)
GO(XkbUpdateKeyTypeVirtualMods, vFppup)
GO(XkbUpdateMapFromCore, iFpCiipp)
GO(XkbUseExtension, iFppp)
GO(XkbVirtualModsToReal, iFpup)
//GO(_XkbWriteCopyKeySyms, 
GO(XkbXlibControlsImplemented, uFv)
GO(XKeycodeToKeysym, LFpCi)
//GO(_XKeycodeToKeysym, 
//GO(_XKeyInitialize, 
GO(XKeysymToKeycode, CFpL)
//GO(_XKeysymToKeycode, 
//GO(_XKeysymToModifiers, 
GO(XKeysymToString, pFL)
//GO(_XkeyTable   // type r, 
GO(XKillClient, iFpL)
GO(XLastKnownRequestProcessed, LFp)
//GO(_XlcAddCharSet, 
//GO(_XlcAddCT, 
//GO(_XlcAddGB18030LocaleConverters, 
//GO(_XlcAddLoader, 
//GO(_XlcAddUtf8Converters, 
//GO(_XlcAddUtf8LocaleConverters, 
//GO(_XlcCloseConverter, 
//GO(_XlcCompareISOLatin1, 
//GO(_XlcCompileResourceList, 
//GO(_XlcConvert, 
//GO(_XlcCopyFromArg, 
//GO(_XlcCopyToArg, 
//GO(_XlcCountVaList, 
//GO(_XlcCreateDefaultCharSet, 
//GO(_XlcCreateLC, 
//GO(_XlcCreateLocaleDataBase, 
//GO(_XlcCurrentLC, 
//GO(_XlcDbg_printValue, 
//GO(_XlcDefaultMapModifiers, 
//GO(_XlcDeInitLoader, 
//GO(_XlcDestroyLC, 
//GO(_XlcDestroyLocaleDataBase, 
//GO(_XlcDynamicLoad, 
//GO(_XlcFileName, 
DATA(_XlcGenericMethods, sizeof(void*))
//GO(_XlcGetCharSet, 
//GO(_XlcGetCharSetWithSide, 
//GO(_XlcGetCSValues, 
//GO(_XlcGetLocaleDataBase, 
//GO(_XlcGetResource, 
//GO(_XlcGetValues, 
//GO(_XlcInitCTInfo, 
//GO(_XlcInitLoader, 
//GO(_XlcLocaleDirName, 
//GO(_XlcLocaleLibDirName, 
//GO(_XlcMapOSLocaleName, 
//GO(_Xlcmbstoutf8, 
//GO(_Xlcmbstowcs, 
//GO(_Xlcmbtowc, 
//GO(_XlcNCompareISOLatin1, 
//GO(_XlcOpenConverter, 
//GO(_XlcParseCharSet, 
//GO(_XlcParse_scopemaps, 
DATA(_XlcPublicMethods, sizeof(void*))
//GO(_XlcRemoveLoader, 
//GO(_XlcResetConverter, 
//GO(_XlcResolveI18NPath, 
//GO(_XlcResolveLocaleName, 
//GO(_XlcSetConverter, 
//GO(_XlcSetValues, 
//GO(_XlcValidModSyntax, 
//GO(_XlcVaToArgList, 
//GO(_Xlcwcstombs, 
//GO(_Xlcwctomb, 
GO(XListDepths, pFpip)
GO(XListExtensions, pFpp)
GO(XListFonts, pFppip)
GO(XListFontsWithInfo, pFppipp)
GO(XListHosts, pFppp)
GO(XListInstalledColormaps, pFpLp)
GO(XListPixmapFormats, pFpp)
GO(XListProperties, pFpLp)
GO(XLoadFont, LFpp)
GO(XLoadQueryFont, pFpp)
// xlocaledir
GO(XLocaleOfFontSet, pFp)
GO(XLocaleOfIM, pFp)
GO(XLocaleOfOM, pFp)
GO(XLockDisplay, vFp)
DATAB(_XLockMutex_fn, sizeof(void*))
GO(XLookupColor, iFpLppp)
GO(XLookupKeysym, LFpi)
//GO(_XLookupKeysym, 
GO(XLookupString, iFppipp)
//GO(_XLookupString, 
GO(XLowerWindow, iFpp)
GO(XMapRaised, iFpp)
GO(XMapSubwindows, iFpp)
GO(XMapWindow, iFpp)
GO(XMaskEvent, iFplp)
GO(XMatchVisualInfo, iFpiiip)
//GO(XMaxCmapsOfScreen
GO(XMaxRequestSize, iFp)
//GO(XmbDrawImageString
GO(XmbDrawString, vFppppiipi)
//GO(XmbDrawText
GO(_Xmblen, iFpi)
GO(XmbLookupString, iFpppipp)
GO(XmbResetIC, pFp)
GO(XmbSetWMProperties, vFpppppippp)
//GO(_Xmbstoutf8, 
//GO(_Xmbstowcs, 
GO(XmbTextEscapement, iFppi)
GO(XmbTextExtents, iFppipp)
GO(XmbTextListToTextProperty, iFppiip)
//GO(_XmbTextListToTextProperty, 
GO(XmbTextPerCharExtents, iFppippppp)
GO(XmbTextPropertyToTextList, iFpppp)
//GO(_XmbTextPropertyToTextList, 
GO(_Xmbtowc, iFppi)
//GO(XMinCmapsOfScreen
GO(XMoveResizeWindow, iFpLiiuu)
GO(XMoveWindow, iFpLii)
GO(XNewModifiermap, pFi)
GO(XNextEvent, iFpp)
GO(XNextRequest, LFp)
GO(XNoOp, iFp)
//GO(_XNoticeCreateBitmap, 
//GO(_XNoticePutBitmap, 
GO(XOffsetRegion, iFpii)
GO(XOMOfOC, pFp)
GOM(XOpenDisplay, pFEp)
GO(XOpenIM, pFpppp)
//GO(_XOpenLC, 
GO(XOpenOM, pFpppp)
//GO(_XParseBaseFontNameList, 
GO(XParseColor, iFpLpp)
GO(XParseGeometry, iFppppp)
GO(XPeekEvent, iFpp)
GOM(XPeekIfEvent, iFEpppp)
GO(XPending, iFp)
//GO(Xpermalloc
GO(XPlanesOfScreen, iFp)
GO(XPointInRegion, iFpp)
//GO(_XPollfdCacheAdd, 
//GO(_XPollfdCacheDel, 
//GO(_XPollfdCacheInit, 
GO(XPolygonRegion, pFpii)
GO(XProcessInternalConnection, vFpi)
//GO(_XProcessInternalConnection, 
//GO(_XProcessWindowAttributes, 
GO(XProtocolRevision, iFp)
GO(XProtocolVersion, iFp)
GO(XPutBackEvent, iFpp)
//GO(_XPutBackEvent, 
GOM(XPutImage, iFEppppiiiiuu)
//GO(XPutPixel
GO(XQLength, iFp)
GO(XQueryBestCursor, iFppuupp)
GO(XQueryBestSize, iFpiLuupp)
//GO(XQueryBestStipple
GO(XQueryBestTile, iFpLuupp)
GO(XQueryColor, iFppp)
GO(XQueryColors, iFpppi)
GOM(XQueryExtension, iFEppppp)
GO(XQueryFont, pFpL)
GO(XQueryKeymap, iFpp)
GO(XQueryPointer, iFpLppppppp)
GO(XQueryTextExtents, iFpLpipppp)
GO(XQueryTextExtents16, iFpLpipppp)
GO(XQueryTree, iFpLpppp)
GO(XRaiseWindow, iFpp)
GO(_XRead, iFppi)
GO(XReadBitmapFile, iFpLpppppp)
GO(XReadBitmapFileData, iFpppppp)
GO(_XReadEvents, vFp)
GO(_XReadPad, vFppi)
GO(XRebindKeysym, iFpLpipi)
GO(XRecolorCursor, iFpLpp)
GO(XReconfigureWMWindow, iFpLiup)
GO(XRectInRegion, iFpiiuu)
GO(XRefreshKeyboardMapping, iFp)
//GO(_XRefreshKeyboardMapping, 
//GO(_XRegisterFilterByMask, 
//GO(_XRegisterFilterByType, 
GOM(XRegisterIMInstantiateCallback, iFEpppppp)
//GO(_XRegisterInternalConnection, 
GOM(XRemoveConnectionWatch, iFEppp)
//GO(XRemoveFromSaveSet
GO(XRemoveHost, iFpp)
GO(XRemoveHosts, iFppi)
GO(XReparentWindow, iFpLLii)
GO(_XReply, iFppii)
GO(XResetScreenSaver, iFp)
GO(XResizeWindow, iFpLuu)
GO(XResourceManagerString, pFp)
GO(XRestackWindows, iFppi)
//GO(_XReverse_Bytes, 
GO(XrmCombineDatabase, vFppi)
GO(XrmCombineFileDatabase, iFppi)
//GO(_XrmDefaultInitParseInfo, 
GO(XrmDestroyDatabase, vFp)
//GOM(XrmEnumerateDatabase
GO(XrmGetDatabase, pFp)
GO(XrmGetFileDatabase, pFp)
GO(XrmGetResource, iFppppp)
GO(XrmGetStringDatabase, pFp)
GO(XrmInitialize, vFv)
//GO(_XrmInitParseInfo, 
//GO(_XrmInternalStringToQuark, 
//GO(XrmLocaleOfDatabase
//GO(XrmMergeDatabases
//GO(XrmParseCommand
//GO(XrmPermStringToQuark
//GO(XrmPutFileDatabase
//GO(XrmPutLineResource
//GO(XrmPutResource
//GO(XrmPutStringResource
//GO(XrmQGetResource
//GO(XrmQGetSearchList
//GO(XrmQGetSearchResource
//GO(XrmQPutResource
//GO(XrmQPutStringResource
//GO(XrmQuarkToString
//GO(XrmSetDatabase
//GO(XrmStringToBindingQuarkList
//GO(XrmStringToQuark
//GO(XrmStringToQuarkList
GO(XrmUniqueQuark, iFv) //typedef int XrmQuark, *XrmQuarkList;
GO(XRootWindow, LFpi)
GO(XRootWindowOfScreen, LFp)
GO(XRotateBuffers, iFpi)
GO(XRotateWindowProperties, iFpLpii)
GO(XSaveContext, iFpLip)
GO(XScreenCount, iFp)
GO(XScreenNumberOfScreen, iFp)
GO(XScreenOfDisplay, pFpi)
//GO(_XScreenOfWindow, 
GO(XScreenResourceString, pFp)
GO(XSelectInput, iFpLl)
GO(_XSend, vFppi)
GO(XSendEvent, iFpLilp)
GO(XServerVendor, pFp)
GO(XSetAccessControl, vFpi)
GOM(XSetAfterFunction, pFEpp)
GO(XSetArcMode, iFppi)
GO(XSetAuthorization, vFpipi)
GO(XSetBackground, iFppL)
GO(XSetClassHint, iFpLp)
GO(XSetClipMask, iFppp)
GO(XSetClipOrigin, iFppii)
GO(XSetClipRectangles, iFppiipii)
//GO(_XSetClipRectangles, 
GO(XSetCloseDownMode, iFpi)
GO(XSetCommand, iFpLpi)
GO(XSetDashes, iFppipi)
GOM(XSetErrorHandler, pFEp)
GO(XSetFillRule, iFppi)
GO(XSetFillStyle, iFppp)
GO(XSetFont, iFppL)
GO(XSetFontPath, iFppi)
GO(XSetForeground, iFppL)
GO(XSetFunction, iFppi)
GO(XSetGraphicsExposures, iFppi)
GO(XSetICFocus, vFp)
GO(XSetIconName, iFppp)
GO(XSetIconSizes, iFpLpi)
GOM(XSetICValues, pFEpV)         // use vaarg
//GO(_XSetImage, 
GOM(XSetIMValues, pFEpV)	// use vaarg
GO(XSetInputFocus, iFpLiL)
GOM(XSetIOErrorHandler, pFEp)
GO(_XSetLastRequestRead, uFpp)
GO(XSetLineAttributes, iFppuiii)
GO(XSetLocaleModifiers, pFp)
GO(XSetModifierMapping, iFpp)
GO(XSetNormalHints, iFpppp)
GO(XSetOCValues, pFpppppppppppppppp) // use vaarg
GO(XSetOMValues, pFp)
GO(XSetPlaneMask, iFppL)
GO(XSetPointerMapping, iFppi)
GO(XSetRegion, iFppp)
GO(XSetRGBColormaps, vFpLpiL)
GO(XSetScreenSaver, iFpiiii)
GO(XSetSelectionOwner, iFpLLL)
GO(XSetSizeHints, iFpLpL)
GO(XSetStandardColormap, vFpLpL)
GO(XSetStandardProperties, iFpLppLpip)
GO(XSetState, iFppLLiL)
GO(XSetStipple, iFppp)
GO(XSetSubwindowMode, iFppi)
GO(XSetTextProperty, vFpLpL)
GO(XSetTile, iFppL)
GO(XSetTransientForHint, iFpLL)
GO(XSetTSOrigin, iFppii)
GO(XSetWindowBackground, iFppu)
GO(XSetWindowBackgroundPixmap, iFppp)
GO(XSetWindowBorder, iFpLL)
GO(XSetWindowBorderPixmap, iFpLL)
GO(XSetWindowBorderWidth, iFpLu)
GO(XSetWindowColormap, iFpLL)
GO(XSetWMClientMachine, vFppp)
GO(XSetWMColormapWindows, iFpLpi)
GO(XSetWMHints, iFpLp)
GO(XSetWMIconName, vFppp)
GO(XSetWMName, vFpLp)
GO(XSetWMNormalHints, vFpLp)
GO(XSetWMProperties, vFpLpppippp)
GO(XSetWMProtocols, iFpLpi)
GO(XSetWMSizeHints, vFpLpL)
GO(XSetZoomHints, iFpLp)
GO(XShrinkRegion, iFpii)
GO(XStoreBuffer, iFppii)
GO(XStoreBytes, iFppi)
GO(XStoreColor, iFpLp)
GO(XStoreColors, iFpLpi)
//GO(_XStoreEventCookie, 
GO(XStoreName, iFpLp)
GO(XStoreNamedColor, iFpLpLi)
GO(XStringListToTextProperty, iFpip)
GO(XStringToKeysym, LFp)
//GOM(XSubImage, pFEpiiuu)    // need unbridging
GO(dummy_XSubImage, pFpiiuu)    // for the wrapper
GO(XSubtractRegion, iFppp)
GO(XSupportsLocale, iFv)
GO(XSync, iFpi)
GOM(XSynchronize, pFEpi)
GO(XTextExtents, iFppipppp)
GO(XTextExtents16, iFppipppp)
GO(_XTextHeight, iFppi)
GO(_XTextHeight16, iFppi)
GO(XTextPropertyToStringList, iFppp)
GO(XTextWidth, iFppi)
GO(XTextWidth16, iFppi)
DATAB(_Xthread_self_fn, sizeof(void*))
GO(XTranslateCoordinates, iFpLLiippp)
//GO(_XTranslateKey, 
//GO(_XTranslateKeySym, 
//GO(_XTryShapeBitmapCursor, 
GO(XUndefineCursor, iFpp)
GO(XUngrabButton, iFpuuL)
GO(XUngrabKey, iFpiuL)
GO(XUngrabKeyboard, iFpL)
GO(XUngrabPointer, iFpL)
GO(XUngrabServer, iFp)
GO(XUninstallColormap, iFpL)
GO(XUnionRectWithRegion, iFppp)
GO(XUnionRegion, iFppp)
//GO(_XUnknownCopyEventCookie, 
//GO(_XUnknownNativeEvent, 
//GO(_XUnknownWireEvent, 
//GO(_XUnknownWireEventCookie, 
GO(XUnloadFont, iFpL)
GO(XUnlockDisplay, vFp)
DATAB(_XUnlockMutex_fn, sizeof(void*))
GO(XUnmapSubwindows, iFpp)
GO(XUnmapWindow, iFpL)
//GO(_XUnregisterFilter, 
GOM(XUnregisterIMInstantiateCallback, iFEpppppp)
//GO(_XUnregisterInternalConnection, 
//GO(_XUnresolveColor, 
GO(XUnsetICFocus, vFp)
//GO(_XUpdateAtomCache, 
//GO(_XUpdateGCCache, 
GO(Xutf8DrawImageString, vFpLppiipi)
GO(Xutf8DrawString, vFppppiipi)
GO(Xutf8DrawText, vFpLpiipi)
GO(Xutf8LookupString, iFpppipp)
GO(Xutf8ResetIC, pFp)
GO(Xutf8SetWMProperties, vFpLpppippp)
GO(Xutf8TextEscapement, iFppi)
GO(Xutf8TextExtents, iFppipp)
GO(Xutf8TextListToTextProperty, iFppiup)
//GO(_Xutf8TextListToTextProperty, 
GO(Xutf8TextPerCharExtents, iFppippippp)
GO(Xutf8TextPropertyToTextList, iFpppp)
//GO(_Xutf8TextPropertyToTextList, 
GOM(XVaCreateNestedList, pFEiV)
GO(XVendorRelease, iFp)
//GO(_XVIDtoVisual, 
GO(XVisualIDFromVisual, LFp)
GO(XWarpPointer, iFpLLiiuuii)
GO(XwcDrawImageString, vFpLppiipi)
GO(XwcDrawString, vFppppiipi)
GO(XwcDrawText, vFpLpiipi)
GO(XwcFreeStringList, vFp)
//GO(_XwcFreeStringList, 
GO(XwcLookupString, iFpppipp)
GO(XwcResetIC, pFp)
//GO(_Xwcscmp, 
//GO(_Xwcscpy, 
//GO(_Xwcslen, 
//GO(_Xwcsncmp, 
//GO(_Xwcsncpy, 
//GO(_Xwcstombs, 
GO(XwcTextEscapement, iFppi)
GO(XwcTextExtents, iFppipp)
GO(XwcTextListToTextProperty, iFppiup)
//GO(_XwcTextListToTextProperty, 
GO(XwcTextPerCharExtents, iFppippippp)
GO(XwcTextPropertyToTextList, iFpppp)
//GO(_XwcTextPropertyToTextList, 
GO(_Xwctomb, iFpu)
GO(XWhitePixel, LFpi)
GO(XWhitePixelOfScreen, LFp)
//GO(XWidthMMOfScreen
GO(XWidthOfScreen, iFp)
GO(XWindowEvent, iFpLlp)
GO(_XWireToEvent, iFppp)
GO(XWithdrawWindow, iFppi)
GO(XWMGeometry, iFpippupppppp)
GO(XWriteBitmapFile, iFppLuuii)
GO(XXorRegion, iFppp)

GO(_XData32, iFppu)
GO(_XRead32, iFppL)

GO(dummy_putpixel, iFpiiL)
GO(dummy_addpixel, iFpl)
