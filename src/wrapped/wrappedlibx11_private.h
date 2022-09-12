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
// _Utf8GetConvByName
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
GO(XAllocColorCells, iFppipipi)
//GO(XAllocColorPlanes
//GO(XAllocIconSize
GO(_XAllocID, LFp)
GO(_XAllocIDs, vFppi)
GO(XAllocNamedColor, iFppppp)
// _XAllocScratch
GO(XAllocSizeHints, pFv)
//GO(XAllocStandardColormap
// _XAllocTemp
GO(XAllocWMHints, pFv)
GO(XAllowEvents, iFpiL)
GO(XAllPlanes, LFv)
// _XAsyncErrorHandler
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
GO(XChangeActivePointerGrab, vFpupp)
GO(XChangeGC, iFppLp)
GO(XChangeKeyboardControl, iFpLp)
GO(XChangeKeyboardMapping, iFpiipi)
GO(XChangePointerControl, iFpiiiii)
GO(XChangeProperty, iFppppiipi)
//GO(XChangeSaveSet
GO(XChangeWindowAttributes, iFppLp)
GOM(XCheckIfEvent, iFEpppp)
GO(XCheckMaskEvent, iFplp)
GO(XCheckTypedEvent, iFpip)
GO(XCheckTypedWindowEvent, iFppip)
GO(XCheckWindowEvent, iFpplp)
//GO(XCirculateSubwindows
//GO(XCirculateSubwindowsDown
//GO(XCirculateSubwindowsUp
GO(XClearArea, iFppiiuui)
GO(XClearWindow, iFpp)
GO(XClipBox, iFpp)
GO(XCloseDisplay, iFp)
GO(XCloseIM, iFp)
// _XCloseLC
//GO(XCloseOM
// _XcmsAddCmapRec
//GO(XcmsAddColorSpace
//GO(XcmsAddFunctionSet
//GO(XcmsAllocColor
//GO(XcmsAllocNamedColor
// _XcmsArcTangent
//GO(XcmsCCCOfColormap
//GO(XcmsCIELabClipab
//GO(XcmsCIELabClipL
//GO(XcmsCIELabClipLab
DATA(XcmsCIELabColorSpace, 6*sizeof(void*))
// _XcmsCIELab_prefix   // type r
//GO(XcmsCIELabQueryMaxC
//GO(XcmsCIELabQueryMaxL
//GO(XcmsCIELabQueryMaxLC
// _XcmsCIELabQueryMaxLCRGB
//GO(XcmsCIELabQueryMinL
//GO(XcmsCIELabToCIEXYZ
//GO(XcmsCIELabWhiteShiftColors
//GO(XcmsCIELuvClipL
//GO(XcmsCIELuvClipLuv
//GO(XcmsCIELuvClipuv
DATA(XcmsCIELuvColorSpace, 6*sizeof(void*))
// _XcmsCIELuv_prefix   // type r
//GO(XcmsCIELuvQueryMaxC
//GO(XcmsCIELuvQueryMaxL
//GO(XcmsCIELuvQueryMaxLC
// _XcmsCIELuvQueryMaxLCRGB
//GO(XcmsCIELuvQueryMinL
//GO(XcmsCIELuvToCIEuvY
//GO(XcmsCIELuvWhiteShiftColors
DATA(XcmsCIEuvYColorSpace, 6*sizeof(void*))
// _XcmsCIEuvY_prefix   // type r
//GO(XcmsCIEuvYToCIELuv
//GO(XcmsCIEuvYToCIEXYZ
//GO(XcmsCIEuvYToTekHVC
// _XcmsCIEuvY_ValidSpec
DATA(XcmsCIExyYColorSpace, 6*sizeof(void*)) // probably need some wrapping
// _XcmsCIExyY_prefix   // type r
//GO(XcmsCIExyYToCIEXYZ
DATA(XcmsCIEXYZColorSpace, 6*sizeof(void*))
// _XcmsCIEXYZ_prefix   // type r
//GO(XcmsCIEXYZToCIELab
//GO(XcmsCIEXYZToCIEuvY
//GO(XcmsCIEXYZToCIExyY
//GO(XcmsCIEXYZToRGBi
// _XcmsCIEXYZ_ValidSpec
//GO(XcmsClientWhitePointOfCCC
//GO(XcmsConvertColors
// _XcmsConvertColorsWithWhitePt
// _XcmsCopyCmapRecAndFree
// _XcmsCopyISOLatin1Lowered
// _XcmsCopyPointerArray
// _XcmsCosine
//GO(XcmsCreateCCC
// _XcmsCubeRoot
DATA(_XcmsDDColorSpaces, 6*sizeof(void*))
DATA(_XcmsDDColorSpacesInit, 6*sizeof(void*))
// _XcmsDDConvertColors
//GO(XcmsDefaultCCC
// _XcmsDeleteCmapRec
DATA(_XcmsDIColorSpaces, 6*sizeof(void*))
DATA(_XcmsDIColorSpacesInit, 6*sizeof(void*))
// _XcmsDIConvertColors
//GO(XcmsDisplayOfCCC
// _XcmsEqualWhitePts
//GO(XcmsFormatOfPrefix
//GO(XcmsFreeCCC
// _XcmsFreeIntensityMaps
// _XcmsFreePointerArray
// _XcmsGetElement
// _XcmsGetIntensityMap
// _XcmsGetProperty
// _XcmsInitDefaultCCCs
// _XcmsInitScrnInfo
DATA(XcmsLinearRGBFunctionSet, 3*sizeof(void*)) // probably needs some wrapping
//GO(XcmsLookupColor
// _XcmsLRGB_InitScrnDefault
//GO(XcmsPrefixOfFormat
// _XcmsPushPointerArray
//GO(XcmsQueryBlack
//GO(XcmsQueryBlue
//GO(XcmsQueryColor
//GO(XcmsQueryColors
//GO(XcmsQueryGreen
//GO(XcmsQueryRed
//GO(XcmsQueryWhite
DATA(_XcmsRegColorSpaces, 6*sizeof(void*))
// _XcmsRegFormatOfPrefix
// _XcmsResolveColor
// _XcmsResolveColorString
DATA(XcmsRGBColorSpace, 6*sizeof(void*))
DATA(XcmsRGBiColorSpace, 6*sizeof(void*))
// _XcmsRGBi_prefix // type r
//GO(XcmsRGBiToCIEXYZ
//GO(XcmsRGBiToRGB
// _XcmsRGB_prefix  // type r
//GO(XcmsRGBToRGBi
// _XcmsRGB_to_XColor
DATA(_XcmsSCCFuncSets, 3*sizeof(void*))
DATA(_XcmsSCCFuncSetsInit, 3*sizeof(void*))
//GO(XcmsScreenNumberOfCCC
//GO(XcmsScreenWhitePointOfCCC
//GO(XcmsSetCCCOfColormap
//GO(XcmsSetCompressionProc
// _XcmsSetGetColor
// _XcmsSetGetColors
//GO(XcmsSetWhiteAdjustProc
//GO(XcmsSetWhitePoint
// _XcmsSine
// _XcmsSquareRoot
//GO(XcmsStoreColor
//GO(XcmsStoreColors
// _XcmsTekHVC_CheckModify
//GO(XcmsTekHVCClipC
//GO(XcmsTekHVCClipV
//GO(XcmsTekHVCClipVC
DATA(XcmsTekHVCColorSpace, 6*sizeof(void*))
// _XcmsTekHVC_prefix   // type r
//GO(XcmsTekHVCQueryMaxC
//GO(XcmsTekHVCQueryMaxV
//GO(XcmsTekHVCQueryMaxVC
// _XcmsTekHVCQueryMaxVCRGB
//GO(XcmsTekHVCQueryMaxVSamples
//GO(XcmsTekHVCQueryMinV
//GO(XcmsTekHVCToCIEuvY
//GO(XcmsTekHVCWhiteShiftColors
DATA(XcmsUNDEFINEDColorSpace, 6*sizeof(void*))
// _XcmsUnresolveColor
//GO(XcmsVisualOfCCC
// _XColor_to_XcmsRGB
GO(XConfigureWindow, iFppup)
GO(XConnectionNumber, iFp)
// _XConnectXCB
//GO(XContextDependentDrawing
//GO(XContextualDrawing
GO(XConvertCase, vFLpp)
GO(XConvertSelection, iFpppppL)
GO(XCopyArea, iFppppiiuuii)
GO(XCopyColormapAndFree, uFpp)
// _XCopyEventCookie
GO(XCopyGC, iFpppL)
GO(XCopyPlane, iFppppiiuuiiL)
// _XCopyToArg
GO(XCreateBitmapFromData, pFpppuu)
GO(XCreateColormap, pFpppi)
GO(XCreateFontCursor, pFpu)
GO(XCreateFontSet, pFppppp)
GO(XCreateGC, pFppup)
//GO(XCreateGlyphCursor
GOM(XCreateIC, pFEpV)     // use vararg
GOM(XCreateImage, pFEppuiipuuii)
GO(dummy_XCreateImage, pFppuiipuuii)    // to have the wrapper
DATAB(_XCreateMutex_fn, sizeof(void*))
//GO(XCreateOC
GO(XCreatePixmap, pFppuuu)
GO(XCreatePixmapCursor, pFpppppuu)
GO(XCreatePixmapFromBitmapData, pFpppuuLLu)
GO(XCreateRegion, pFv)
GO(XCreateSimpleWindow, pFppiiuuuLL)
GO(XCreateWindow, pFppiiuuuiupLp)
DATAB(_Xdebug, sizeof(void*))
GO(XDefaultColormap, pFpi)
GO(XDefaultColormapOfScreen, pFp)
GO(XDefaultDepth, iFpi)
GO(XDefaultDepthOfScreen, iFp)
// _XDefaultError
GO(XDefaultGC, pFpi)
GO(XDefaultGCOfScreen, pFp)
GO(_XDefaultIOError, iFp)
// _XDefaultOpenIM
// _XDefaultOpenOM
GO(XDefaultRootWindow, pFp)
GO(XDefaultScreen, iFp)
GO(XDefaultScreenOfDisplay, pFp)
GO(XDefaultString, pFv)
GO(XDefaultVisual, pFpi)
GO(XDefaultVisualOfScreen, pFp)
// _XDefaultWireError
GO(XDefineCursor, iFppp)
GO(XDeleteContext, iFpLi)
//GO(XDeleteModifiermapEntry
GO(XDeleteProperty, iFppp)
// _XDeq
GOM(_XDeqAsyncHandler, vFEpp)
GO(XDestroyIC, vFp)
GOM(XDestroyImage, iFEp)  //need to unbridge
//GO(XDestroyOC
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
//GO(XDisplayOfOM
//GO(XDisplayOfScreen
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
GO(XDrawLine, iFpppiiii)
GO(XDrawLines, iFppppii)
GO(XDrawPoint, iFpppii)
GO(XDrawPoints, iFppppii)
GO(XDrawRectangle, iFpppiiuu)
GO(XDrawRectangles, iFppppi)
GO(XDrawSegments, iFppppi)
GO(XDrawString, iFpppiipi)
GO(XDrawString16, iFpppiipi)
//GO(XDrawText
//GO(XDrawText16
GO(_XEatData, vFpL)
GO(_XEatDataWords, vFpL)
//GO(XEHeadOfExtensionList
GO(XEmptyRegion, iFp)
GO(XEnableAccessControl, vFp)
// _XEnq
GO(XEqualRegion, iFpp)
// _XError
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
//GO(XEventMaskOfScreen
GO(XEventsQueued, iFpi)
// _XEventsQueued
// _Xevent_to_mask  // type r
// _XEventToWire
GO(XExtendedMaxRequestSize, lFp)
//GO(XExtentsOfFontSet
// _XF86BigfontFreeFontMetrics
// _XF86LoadQueryLocaleFont
//GO(XFetchBuffer
//GO(XFetchBytes
// _XFetchEventCookie
//GO(XFetchName
GO(XFillArc, iFpppiiuuii)
//GO(XFillArcs
GO(XFillPolygon, iFppppiii)
GO(XFillRectangle, iFpppiiuu)
GO(XFillRectangles, iFppppi)
GO(XFilterEvent, iFpp)
GO(XFindContext, iFpLip)
//GO(XFindOnExtensionList
GO(XFlush, iFp)
GO(_XFlush, vFp)
GO(XFlushGC, vFpp)
GO(_XFlushGCCache, vFpp)
GO(XFontsOfFontSet, iFppp)
GO(XForceScreenSaver, iFpi)
GO(XFree, iFp)
// _XFreeAtomTable
GO(XFreeColormap, iFpp)
GO(XFreeColors, iFpppiL)
GO(XFreeCursor, iFpp)
DATAB(_XFreeDisplayLock_fn, sizeof(void*))
// _XFreeDisplayStructure
// _XFreeEventCookies
GO(XFreeEventData, vFpp)
// _XFreeExtData
GO(XFreeExtensionList, iFp)
GO(XFreeFont, iFpp)
GO(XFreeFontInfo, iFppi)
GO(XFreeFontNames, iFp)
GO(XFreeFontPath, iFp)
GO(XFreeFontSet, vFpp)
GO(XFreeGC, iFpp)
GO(XFreeModifiermap, iFp)
DATAB(_XFreeMutex_fn, sizeof(void*))
GO(XFreePixmap, iFpp)
GO(XFreeStringList, vFp)
// _XFreeTemp
// _XFreeX11XCBStructure
GO(XGContextFromGC, pFp)
GO(XGeometry, iFpippuuuiipppp)
GO(_XGetAsyncData, vFpppiiii)
// _XGetAsyncReply
GO(XGetAtomName, pFpp)
GO(XGetAtomNames, iFppip)
// _XGetBitsPerPixel
GO(XGetClassHint, iFppp)
GO(XGetCommand, iFpppp)
GO(XGetDefault, pFppp)
GO(XGetErrorDatabaseText, iFpppppi)
GO(XGetErrorText, iFpipi)
GO(XGetEventData, iFpp)
GO(XGetFontPath, pFpp)
GO(XGetFontProperty, iFppp)
GO(XGetGCValues, iFppLp)
GO(XGetGeometry, iFppppppppp)
// _XGetHostname
GO(XGetIconName, iFppp)
//GO(XGetIconSizes
GO(XGetICValues, pFpppppppppp)      // use varargs...
GOM(XGetImage, pFEppiiuuLi)     // return an XImage with callbacks that needs wrapping
GO(XGetIMValues, pFppppppp)     // use varargs
GO(XGetInputFocus, iFppp)
GO(XGetKeyboardControl, iFpp)
GO(XGetKeyboardMapping, pFppip)
// _XGetLCValues
GO(XGetModifierMapping, pFp)
GO(XGetMotionEvents, pFppLLp)
//GO(XGetNormalHints
//GO(XGetOCValues
//GO(XGetOMValues
//GOM(XGetPixel, LFEpii)  // need unbridging
GO(dummy_XGetPixel, LFpii)     // for the wrapper
GO(XGetPointerControl, iFpppp)
GO(XGetPointerMapping, iFppi)
GO(_XGetRequest, pFpuL)
GO(XGetRGBColormaps, iFppppp)
// _XGetScanlinePad
GO(XGetScreenSaver, iFppppp)
GO(XGetSelectionOwner, pFpp)
//GO(XGetSizeHints
//GO(XGetStandardColormap
GOM(XGetSubImage, pFEppiiuuuipii)
GO(XGetTextProperty, iFpppp)
GO(XGetTransientForHint, iFppp)
GO(XGetVisualInfo, pFplpp)
GO(XGetWindowAttributes, iFppp)
// _XGetWindowAttributes
GO(XGetWindowProperty, iFpppllipppppp)
//GO(XGetWMClientMachine
GO(XGetWMColormapWindows, iFpppp)
GO(XGetWMHints, pFpp)
GO(XGetWMIconName, iFppp)
GO(XGetWMName, iFppp)
GO(XGetWMNormalHints, iFpppp)
//GO(XGetWMProtocols
GO(XGetWMSizeHints, iFppppp)
//GO(XGetZoomHints
DATAB(_Xglobal_lock, sizeof(void*))
GO(XGrabButton, iFpuupiuiipp)
GO(XGrabKey, iFpiupiii)
GO(XGrabKeyboard, iFppuiiL)
GO(XGrabPointer, iFppiuiippL)
GO(XGrabServer, iFp)
DATAB(_XHeadOfDisplayList, sizeof(void*))
GO(XHeightMMOfScreen, iFp)
GO(XHeightOfScreen, iFp)
DATAB(_Xi18n_lock, sizeof(void*))
GO(XIconifyWindow, iFppi)
GOM(XIfEvent, iFEpppp)
GO(XImageByteOrder, iFp)
// _XIMCompileResourceList
// _XimGetCharCode
// _XimGetLocaleCode
// _XimLookupMBText
// _XimLookupUTF8Text
// _XimLookupWCText
GO(XIMOfIC, pFp)
// _XimXTransBytesReadable
// _XimXTransClose
// _XimXTransCloseForCloning
// _XimXTransConnect
// _XimXTransDisconnect
// _XimXTransFreeConnInfo
// _XimXTransGetConnectionNumber
// _XimXTransGetHostname
// _XimXTransGetMyAddr
// _XimXTransGetPeerAddr
// _XimXTransIsLocal
// _XimXTransOpenCLTSClient
// _XimXTransOpenCOTSClient
// _XimXTransRead
// _XimXTransReadv
// _XimXTransSetOption
DATA(_XimXTransSocketINET6Funcs, sizeof(void*))
DATA(_XimXTransSocketINETFuncs, sizeof(void*))
DATA(_XimXTransSocketLocalFuncs, sizeof(void*))
DATA(_XimXTransSocketTCPFuncs, sizeof(void*))
DATA(_XimXTransSocketUNIXFuncs, sizeof(void*))
// _XimXTransWrite
// _XimXTransWritev
// _XInitDefaultIM
// _XInitDefaultOM
DATAB(_XInitDisplayLock_fn, sizeof(void*))
// _XInitDynamicIM
// _XInitDynamicOM
GO(XInitExtension, pFpp)
GOM(XInitImage, iFEp)
// _XInitImageFuncPtrs
// _XInitKeysymDB
GO(XInitThreads, iFv)
//GO(XInsertModifiermapEntry
GO(XInstallColormap, iFpp)
GO(XInternalConnectionNumbers, iFppp)
GO(XInternAtom, pFppi)
GO(XInternAtoms, iFppiip)
GO(XIntersectRegion, iFppp)
GO(_XIOError, iFp)
DATAB(_XIOErrorFunction, sizeof(void*))
// _XIsEventCookie
//GO(XkbAddDeviceLedInfo
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
//GO(XkbAddKeyType
//GO(XkbAllocClientMap
//GO(XkbAllocCompatMap
//GO(XkbAllocControls
//GO(XkbAllocDeviceInfo
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
//GO(XkbAllocIndicatorMaps
//GO(XkbAllocKeyboard
//GO(XkbAllocNames
//GO(XkbAllocServerMap
//GO(XkbApplyCompatMapToKey
//GO(XkbApplyVirtualModChanges
//GO(XkbBell
//GO(XkbBellEvent
//GO(XkbChangeDeviceInfo
//GO(XkbChangeEnabledControls
//GO(XkbChangeKeycodeRange
//GO(XkbChangeMap
//GO(XkbChangeNames
//GO(XkbChangeTypesOfKey
//GO(XkbComputeEffectiveMap
//GO(XkbComputeRowBounds
//GO(XkbComputeSectionBounds
//GO(XkbComputeShapeBounds
//GO(XkbComputeShapeTop
// _XkbCopyFromReadBuffer
//GO(XkbCopyKeyType
//GO(XkbCopyKeyTypes
//GO(XkbDeviceBell
//GO(XkbDeviceBellEvent
//GO(XkbFindOverlayForKey
//GO(XkbForceBell
//GO(XkbForceDeviceBell
GO(XkbFreeClientMap, vFpui)
//GO(XkbFreeCompatMap
//GO(XkbFreeComponentList
//GO(XkbFreeControls
//GO(XkbFreeDeviceInfo
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
//GO(XkbFreeIndicatorMaps
GO(XkbFreeKeyboard, vFpui)
GO(XkbFreeNames, vFpui)
// _XkbFreeReadBuffer
//GO(XkbFreeServerMap
DATA(_XkbGetAtomNameFunc, sizeof(void*))
//GO(XkbGetAutoRepeatRate
//GO(XkbGetAutoResetControls
// _XkbGetCharset
//GO(XkbGetCompatMap
//GO(XkbGetControls
// _XkbGetConverters
//GO(XkbGetDetectableAutoRepeat
//GO(XkbGetDeviceButtonActions
//GO(XkbGetDeviceInfo
//GO(XkbGetDeviceInfoChanges
//GO(XkbGetDeviceLedInfo
//GO(XkbGetGeometry
//GO(XkbGetIndicatorMap
//GO(XkbGetIndicatorState
//GO(XkbGetKeyActions
//GO(XkbGetKeyBehaviors
GO(XkbGetKeyboard, pFpuu)
//GO(XkbGetKeyboardByName
//GO(XkbGetKeyExplicitComponents
//GO(XkbGetKeyModifierMap
//GO(XkbGetKeySyms
//GO(XkbGetKeyTypes
//GO(XkbGetKeyVirtualModMap
GO(XkbGetMap, pFpuu)
//GO(XkbGetMapChanges
//GO(XkbGetNamedDeviceIndicator
//GO(XkbGetNamedGeometry
GO(XkbGetNamedIndicator, iFpuppppp)
GO(XkbGetNames, iFpup)
//GO(XkbGetPerClientControls
// _XkbGetReadBufferCountedString
// _XkbGetReadBufferPtr
GO(XkbGetState, iFpup)
GO(XkbGetUpdatedMap, iFpup)
//GO(XkbGetVirtualMods
//GO(XkbGetXlibControls
GO(XkbIgnoreExtension, iFp)
//GO(XkbInitCanonicalKeyTypes
// _XkbInitReadBuffer
DATA(_XkbInternAtomFunc, sizeof(void*))
GO(XkbKeycodeToKeysym, LFpuuu)
GO(XkbKeysymToModifiers, uFpp)
//GO(XkbKeyTypesForCoreSymbols
//GO(XkbLatchGroup
//GO(XkbLatchModifiers
GO(XkbLibraryVersion, iFpp)
//GO(XkbListComponents
//GO(XkbLockGroup
//GO(XkbLockModifiers
//GO(XkbLookupKeyBinding
GO(XkbLookupKeySym, iFpuupp)
//GO(XkbNoteControlsChanges
// _XkbNoteCoreMapChanges
//GO(XkbNoteDeviceChanges
//GO(XkbNoteMapChanges
//GO(XkbNoteNameChanges
//GO(XkbOpenDisplay
// _XkbPeekAtReadBuffer
GO(XkbQueryExtension, iFpppppp)
// _XkbReadBufferCopyKeySyms
// _XkbReadCopyKeySyms
// _XkbReadGetCompatMapReply
// _XkbReadGetGeometryReply
// _XkbReadGetIndicatorMapReply
// _XkbReadGetMapReply
// _XkbReadGetNamesReply
//GO(XkbRefreshKeyboardMapping
// _XkbReloadDpy
//GO(XkbResizeDeviceButtonActions
//GO(XkbResizeKeyActions
//GO(XkbResizeKeySyms
//GO(XkbResizeKeyType
GO(XkbSelectEventDetails, iFpuuLL)
GO(XkbSelectEvents, iFpuLL)
//GO(XkbSetAtomFuncs
//GO(XkbSetAutoRepeatRate
//GO(XkbSetAutoResetControls
//GO(XkbSetCompatMap
//GO(XkbSetControlslib
//GO(XkbSetDebuggingFlags
GO(XkbSetDetectableAutoRepeat, iFpip)
//GO(XkbSetDeviceButtonActions
//GO(XkbSetDeviceInfo
//GO(XkbSetDeviceLedInfo
//GO(XkbSetGeometry
//GO(XkbSetIgnoreLockMods
//GO(XkbSetIndicatorMap
//GO(XkbSetMap
//GO(XkbSetNamedDeviceIndicator
//GO(XkbSetNamedIndicator
//GO(XkbSetNames
GO(XkbSetPerClientControls, iFpup)
//GO(XkbSetServerInternalMods
//GO(XkbSetXlibControls
// _XkbSkipReadBufferData
//GO(XkbToControl
//GO(XkbTranslateKey
//GO(XkbTranslateKeyCode
GO(XkbTranslateKeySym, iFppupip)
//GO(XkbUpdateActionVirtualMods
//GO(XkbUpdateKeyTypeVirtualMods
//GO(XkbUpdateMapFromCore
GO(XkbUseExtension, iFppp)
//GO(XkbVirtualModsToReal
// _XkbWriteCopyKeySyms
//GO(XkbXlibControlsImplemented
GO(XKeycodeToKeysym, pFppi)
// _XKeycodeToKeysym
// _XKeyInitialize
GO(XKeysymToKeycode, uFpp)
// _XKeysymToKeycode
// _XKeysymToModifiers
GO(XKeysymToString, pFp)
// _XkeyTable   // type r
GO(XKillClient, iFpL)
GO(XLastKnownRequestProcessed, LFp)
// _XlcAddCharSet
// _XlcAddCT
// _XlcAddGB18030LocaleConverters
// _XlcAddLoader
// _XlcAddUtf8Converters
// _XlcAddUtf8LocaleConverters
// _XlcCloseConverter
// _XlcCompareISOLatin1
// _XlcCompileResourceList
// _XlcConvert
// _XlcCopyFromArg
// _XlcCopyToArg
// _XlcCountVaList
// _XlcCreateDefaultCharSet
// _XlcCreateLC
// _XlcCreateLocaleDataBase
// _XlcCurrentLC
// _XlcDbg_printValue
// _XlcDefaultMapModifiers
// _XlcDeInitLoader
// _XlcDestroyLC
// _XlcDestroyLocaleDataBase
// _XlcDynamicLoad
// _XlcFileName
DATA(_XlcGenericMethods, sizeof(void*))
// _XlcGetCharSet
// _XlcGetCharSetWithSide
// _XlcGetCSValues
// _XlcGetLocaleDataBase
// _XlcGetResource
// _XlcGetValues
// _XlcInitCTInfo
// _XlcInitLoader
// _XlcLocaleDirName
// _XlcLocaleLibDirName
// _XlcMapOSLocaleName
// _Xlcmbstoutf8
// _Xlcmbstowcs
// _Xlcmbtowc
// _XlcNCompareISOLatin1
// _XlcOpenConverter
// _XlcParseCharSet
// _XlcParse_scopemaps
DATA(_XlcPublicMethods, sizeof(void*))
// _XlcRemoveLoader
// _XlcResetConverter
// _XlcResolveI18NPath
// _XlcResolveLocaleName
// _XlcSetConverter
// _XlcSetValues
// _XlcValidModSyntax
// _XlcVaToArgList
// _Xlcwcstombs
// _Xlcwctomb
GO(XListDepths, pFpip)
GO(XListExtensions, pFpp)
GO(XListFonts, pFppip)
GO(XListFontsWithInfo, pFppipp)
GO(XListHosts, pFppp)
//GO(XListInstalledColormaps
GO(XListPixmapFormats, pFpp)
GO(XListProperties, pFppp)
GO(XLoadFont, pFpp)
GO(XLoadQueryFont, pFpp)
// xlocaledir
//GO(XLocaleOfFontSet
GO(XLocaleOfIM, pFp)
//GO(XLocaleOfOM
GO(XLockDisplay, vFp)
DATAB(_XLockMutex_fn, sizeof(void*))
GO(XLookupColor, iFppppp)
GO(XLookupKeysym, pFpi)
// _XLookupKeysym
GO(XLookupString, iFppipp)
// _XLookupString
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
// _Xmblen
GO(XmbLookupString, iFpppipp)
GO(XmbResetIC, pFp)
GO(XmbSetWMProperties, vFpppppippp)
// _Xmbstoutf8
// _Xmbstowcs
GO(XmbTextEscapement, iFppi)
GO(XmbTextExtents, iFppipp)
GO(XmbTextListToTextProperty, iFppiip)
// _XmbTextListToTextProperty
//GO(XmbTextPerCharExtents
GO(XmbTextPropertyToTextList, iFpppp)
// _XmbTextPropertyToTextList
// _Xmbtowc
//GO(XMinCmapsOfScreen
GO(XMoveResizeWindow, iFppiiuu)
GO(XMoveWindow, iFppii)
//GO(XNewModifiermap
GO(XNextEvent, iFpp)
GO(XNextRequest, LFp)
GO(XNoOp, iFp)
// _XNoticeCreateBitmap
// _XNoticePutBitmap
GO(XOffsetRegion, iFpii)
//GO(XOMOfOC
GOM(XOpenDisplay, pFEp)
GO(XOpenIM, pFpppp)
// _XOpenLC
//GO(XOpenOM
// _XParseBaseFontNameList
GO(XParseColor, iFpppp)
GO(XParseGeometry, iFppppp)
GO(XPeekEvent, iFpp)
GOM(XPeekIfEvent, iFEpppp)
GO(XPending, iFp)
//GO(Xpermalloc
GO(XPlanesOfScreen, iFp)
GO(XPointInRegion, iFpp)
// _XPollfdCacheAdd
// _XPollfdCacheDel
// _XPollfdCacheInit
GO(XPolygonRegion, pFpii)
GO(XProcessInternalConnection, vFpi)
// _XProcessInternalConnection
// _XProcessWindowAttributes
GO(XProtocolRevision, iFp)
GO(XProtocolVersion, iFp)
GO(XPutBackEvent, iFpp)
// _XPutBackEvent
GOM(XPutImage, iFEppppiiiiuu)
//GO(XPutPixel
GO(XQLength, iFp)
GO(XQueryBestCursor, iFppuupp)
//GO(XQueryBestSize
//GO(XQueryBestStipple
//GO(XQueryBestTile
GO(XQueryColor, iFppp)
GO(XQueryColors, iFpppi)
GOM(XQueryExtension, iFEppppp)
GO(XQueryFont, pFpL)
GO(XQueryKeymap, iFpp)
GO(XQueryPointer, iFppppppppp)
GO(XQueryTextExtents, iFpLpipppp)
GO(XQueryTextExtents16, iFpLpipppp)
GO(XQueryTree, pFpppppp)
GO(XRaiseWindow, iFpp)
GO(_XRead, iFppi)
//GO(XReadBitmapFile
//GO(XReadBitmapFileData
// _XReadEvents
GO(_XReadPad, vFppi)
GO(XRebindKeysym, iFpppipi)
GO(XRecolorCursor, iFpppp)
GO(XReconfigureWMWindow, iFppiup)
GO(XRectInRegion, iFpiiuu)
GO(XRefreshKeyboardMapping, iFp)
// _XRefreshKeyboardMapping
// _XRegisterFilterByMask
// _XRegisterFilterByType
GOM(XRegisterIMInstantiateCallback, iFEpppppp)
// _XRegisterInternalConnection
GOM(XRemoveConnectionWatch, iFEppp)
//GO(XRemoveFromSaveSet
GO(XRemoveHost, vFpp)
GO(XRemoveHosts, vFppi)
GO(XReparentWindow, iFpppii)
GO(_XReply, iFppii)
GO(XResetScreenSaver, iFp)
GO(XResizeWindow, iFppuu)
GO(XResourceManagerString, pFp)
GO(XRestackWindows, iFppi)
// _XReverse_Bytes
GO(XrmCombineDatabase, vFppi)
GO(XrmCombineFileDatabase, iFppi)
// _XrmDefaultInitParseInfo
GO(XrmDestroyDatabase, vFp)
//GOM(XrmEnumerateDatabase
GO(XrmGetDatabase, pFp)
GO(XrmGetFileDatabase, pFp)
GO(XrmGetResource, iFppppp)
GO(XrmGetStringDatabase, pFp)
GO(XrmInitialize, vFv)
// _XrmInitParseInfo
// _XrmInternalStringToQuark
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
GO(XRootWindow, pFpi)
GO(XRootWindowOfScreen, pFp)
GO(XRotateBuffers, iFpi)
GO(XRotateWindowProperties, iFpppii)
GO(XSaveContext, iFpLip)
GO(XScreenCount, iFp)
GO(XScreenNumberOfScreen, iFp)
GO(XScreenOfDisplay, pFpi)
// _XScreenOfWindow
//GO(XScreenResourceString
GO(XSelectInput, iFppi)
GO(_XSend, vFppi)
GO(XSendEvent, uFppiip)
GO(XServerVendor, pFp)
GO(XSetAccessControl, vFpi)
GOM(XSetAfterFunction, pFEpp)
GO(XSetArcMode, iFppi)
//GO(XSetAuthorization
GO(XSetBackground, iFppL)
GO(XSetClassHint, iFppp)
GO(XSetClipMask, iFppp)
GO(XSetClipOrigin, iFppii)
GO(XSetClipRectangles, iFppiipii)
// _XSetClipRectangles
GO(XSetCloseDownMode, iFpi)
GO(XSetCommand, iFpppi)
GO(XSetDashes, iFppipi)
GOM(XSetErrorHandler, pFEp)
GO(XSetFillRule, iFppi)
GO(XSetFillStyle, iFppp)
GO(XSetFont, iFppp)
GO(XSetFontPath, iFppi)
GO(XSetForeground, iFppL)
GO(XSetFunction, iFppi)
GO(XSetGraphicsExposures, iFppi)
GO(XSetICFocus, vFp)
GO(XSetIconName, iFppp)
//GO(XSetIconSizes
GOM(XSetICValues, pFEpV)         // use vaarg
// _XSetImage
GOM(XSetIMValues, pFEpV)	// use vaarg
GO(XSetInputFocus, iFppiL)
GOM(XSetIOErrorHandler, pFEp)
GO(_XSetLastRequestRead, uFpp)
GO(XSetLineAttributes, iFppuiii)
GO(XSetLocaleModifiers, pFp)
//GO(XSetModifierMapping
GO(XSetNormalHints, iFpppp)
//GO(XSetOCValues
//GO(XSetOMValues
//GO(XSetPlaneMask
//GO(XSetPointerMapping
GO(XSetRegion, iFppp)
//GO(XSetRGBColormaps
GO(XSetScreenSaver, iFpiiii)
GO(XSetSelectionOwner, iFpppL)
//GO(XSetSizeHints
//GO(XSetStandardColormap
GO(XSetStandardProperties, iFpppppppp)
//GO(XSetState
GO(XSetStipple, iFppp)
GO(XSetSubwindowMode, iFppi)
GO(XSetTextProperty, vFpppp)
GO(XSetTile, iFppp)
GO(XSetTransientForHint, iFppp)
GO(XSetTSOrigin, iFppii)
GO(XSetWindowBackground, iFppu)
GO(XSetWindowBackgroundPixmap, iFppp)
//GO(XSetWindowBorder
//GO(XSetWindowBorderPixmap
GO(XSetWindowBorderWidth, iFppu)
GO(XSetWindowColormap, iFppp)
GO(XSetWMClientMachine, vFppp)
GO(XSetWMColormapWindows, iFpppi)
GO(XSetWMHints, iFppp)
GO(XSetWMIconName, vFppp)
GO(XSetWMName, vFppp)
GO(XSetWMNormalHints, iFppp)
GO(XSetWMProperties, vFpppppippp)
GO(XSetWMProtocols, iFpppi)
GO(XSetWMSizeHints, vFpppu)
//GO(XSetZoomHints
GO(XShrinkRegion, iFpii)
//GO(XStoreBuffer
//GO(XStoreBytes
GO(XStoreColor, iFppp)
GO(XStoreColors, iFpppi)
// _XStoreEventCookie
GO(XStoreName, iFppp)
//GO(XStoreNamedColor
GO(XStringListToTextProperty, iFpip)
GO(XStringToKeysym, pFp)
//GOM(XSubImage, pFEpiiuu)    // need unbridging
GO(dummy_XSubImage, pFpiiuu)    // for the wrapper
GO(XSubtractRegion, iFppp)
GO(XSupportsLocale, iFv)
GO(XSync, iFpu)
GOM(XSynchronize, pFEpi)
GO(XTextExtents, iFppipppp)
GO(XTextExtents16, iFppipppp)
// _XTextHeight
// _XTextHeight16
//GO(XTextPropertyToStringList
GO(XTextWidth, iFppi)
GO(XTextWidth16, iFppi)
DATAB(_Xthread_self_fn, sizeof(void*))
GO(XTranslateCoordinates, iFpppiippp)
// _XTranslateKey
// _XTranslateKeySym
// _XTryShapeBitmapCursor
GO(XUndefineCursor, iFpp)
//GO(XUngrabButton
GO(XUngrabKey, iFpiup)
GO(XUngrabKeyboard, iFpL)
GO(XUngrabPointer, iFpL)
GO(XUngrabServer, iFp)
GO(XUninstallColormap, iFpp)
GO(XUnionRectWithRegion, iFppp)
GO(XUnionRegion, iFppp)
// _XUnknownCopyEventCookie
// _XUnknownNativeEvent
// _XUnknownWireEvent
// _XUnknownWireEventCookie
GO(XUnloadFont, iFpp)
GO(XUnlockDisplay, vFp)
DATAB(_XUnlockMutex_fn, sizeof(void*))
GO(XUnmapSubwindows, iFpp)
GO(XUnmapWindow, iFpp)
// _XUnregisterFilter
GOM(XUnregisterIMInstantiateCallback, iFEpppppp)
// _XUnregisterInternalConnection
// _XUnresolveColor
GO(XUnsetICFocus, vFp)
// _XUpdateAtomCache
// _XUpdateGCCache
//GO(Xutf8DrawImageString
GO(Xutf8DrawString, vFppppiipi)
//GO(Xutf8DrawText
GO(Xutf8LookupString, iFLppipp)
GO(Xutf8ResetIC, pFL)
GO(Xutf8SetWMProperties, vFpppppippp)
//GO(Xutf8TextEscapement
GO(Xutf8TextExtents, iFppipp)
GO(Xutf8TextListToTextProperty, iFppiup)
// _Xutf8TextListToTextProperty
//GO(Xutf8TextPerCharExtents
GO(Xutf8TextPropertyToTextList, iFpppp)
// _Xutf8TextPropertyToTextList
GOM(XVaCreateNestedList, pFEiV)
GO(XVendorRelease, iFp)
// _XVIDtoVisual
GO(XVisualIDFromVisual, LFp)
GO(XWarpPointer, iFpppiiuuii)
//GO(XwcDrawImageString
GO(XwcDrawString, vFppppiipi)
//GO(XwcDrawText
GO(XwcFreeStringList, vFp)
// _XwcFreeStringList
GO(XwcLookupString, iFpppipp)
//GO(XwcResetIC
// _Xwcscmp
// _Xwcscpy
// _Xwcslen
// _Xwcsncmp
// _Xwcsncpy
// _Xwcstombs
GO(XwcTextEscapement, iFppi)
GO(XwcTextExtents, iFppipp)
GO(XwcTextListToTextProperty, iFppiip)
// _XwcTextListToTextProperty
//GO(XwcTextPerCharExtents
GO(XwcTextPropertyToTextList, iFpppp)
// _XwcTextPropertyToTextList
// _Xwctomb
//GO(XWhitePixel
//GO(XWhitePixelOfScreen
//GO(XWidthMMOfScreen
GO(XWidthOfScreen, iFp)
GO(XWindowEvent, iFpplp)
GO(_XWireToEvent, iFppp)
GO(XWithdrawWindow, iFppi)
//GO(XWMGeometry
//GO(XWriteBitmapFile
GO(XXorRegion, iFppp)

GO(_XData32, iFppu)
GO(_XRead32, iFppL)

GO(dummy_putpixel, iFpiiL)
GO(dummy_addpixel, iFpl)
