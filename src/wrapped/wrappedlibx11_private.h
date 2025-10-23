#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

DATAB(_qfree, 4) // Warning: failed to confirm

GO(XActivateScreenSaver, iFp)
GOM(XAddConnectionWatch, iFEppp)
GO(XAddExtension, pFp)
GO(XAddHost, iFpp)
GO(XAddHosts, iFppi)
GO(XAddPixel, vFpl)
GO(XAddToExtensionList, iFpp)
GO(XAddToSaveSet, iFpL)
GO(XAllocClassHint, pFv)
GO(XAllocColor, iFpLp)
GO(XAllocColorCells, iFpLipupu)
GO(XAllocColorPlanes, iFpLipiiiippp)
GO(XAllocIconSize, pFv)
GO(_XAllocID, LFp)
GO(_XAllocIDs, vFppi)
GO(XAllocNamedColor, iFpLppp)
GO(_XAllocScratch, pFpL)
GO(XAllocSizeHints, pFv)
GO(XAllocStandardColormap, pFv)
GO(_XAllocTemp, pFpL)
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
GO(XBlackPixelOfScreen, LFp)
GO(XCellsOfScreen, iFp)
GO(XChangeActivePointerGrab, iFpuLL)
GO(XChangeGC, iFppLp)
GO(XChangeKeyboardControl, iFpLp)
GO(XChangeKeyboardMapping, iFpiipi)
GO(XChangePointerControl, iFpiiiii)
GO(XChangeProperty, iFpLLLiipi)
GO(XChangeSaveSet, iFpLi)
GO(XChangeWindowAttributes, iFpLLp)
GOM(XCheckIfEvent, iFEpppp)
GO(XCheckMaskEvent, iFplp)
GO(XCheckTypedEvent, iFpip)
GO(XCheckTypedWindowEvent, iFpLip)
GO(XCheckWindowEvent, iFpLlp)
GO(XCirculateSubwindows, iFpLi)
GO(XCirculateSubwindowsDown, iFpL)
GO(XCirculateSubwindowsUp, iFpL)
GO(XClearArea, iFpLiiuui)
GO(XClearWindow, iFpL)
GO(XClipBox, iFpp)
GOM(XCloseDisplay, iFEp)
GO(XCloseIM, iFp)
//GO(_XCloseLC, 
GO(XCloseOM, iFp)
//GO(_XcmsAddCmapRec, 
//GOM(XcmsAddColorSpace, iFEp)
GO(XcmsAddFunctionSet, iFp)
GO(XcmsAllocColor, iFpLpL)
GO(XcmsAllocNamedColor, iFpLpppL)
//GO(_XcmsArcTangent, 
//GOM(XcmsCCCOfColormap, pFEpL)
//GOM(XcmsCIELabClipab, iFEppuup)
//GOM(XcmsCIELabClipL, iFEppuup)
//GOM(XcmsCIELabClipLab, iFEppuup)
DATA(XcmsCIELabColorSpace, 6*sizeof(void*))
//DATA(_XcmsCIELab_prefix, 
//GOM(XcmsCIELabQueryMaxC, iFEpddp)
//GOM(XcmsCIELabQueryMaxL, iFEpddp)
//GOM(XcmsCIELabQueryMaxLC, iFEpdp)
//GO(_XcmsCIELabQueryMaxLCRGB, 
//GOM(XcmsCIELabQueryMinL, iFEpddp)
//GOM(XcmsCIELabToCIEXYZ, iFEpppu)
//GOM(XcmsCIELabWhiteShiftColors, iFEpppLpup)
//GOM(XcmsCIELuvClipL, iFEppuup)
//GOM(XcmsCIELuvClipLuv, iFEppuup)
//GOM(XcmsCIELuvClipuv, iFEppuup)
DATA(XcmsCIELuvColorSpace, 6*sizeof(void*))
//DATA(_XcmsCIELuv_prefix, 
//GOM(XcmsCIELuvQueryMaxC, iFEpddp)
//GOM(XcmsCIELuvQueryMaxL, iFEpddp)
//GOM(XcmsCIELuvQueryMaxLC, iFEpdp)
//GO(_XcmsCIELuvQueryMaxLCRGB, 
//GOM(XcmsCIELuvQueryMinL, iFEpddp)
//GOM(XcmsCIELuvToCIEuvY, iFEpppu)
//GOM(XcmsCIELuvWhiteShiftColors, iFEpppLpup)
DATA(XcmsCIEuvYColorSpace, 6*sizeof(void*))
//DATA(_XcmsCIEuvY_prefix, 
//GOM(XcmsCIEuvYToCIELuv, iFEpppu)
//GOM(XcmsCIEuvYToCIEXYZ, iFEpppu)
//GOM(XcmsCIEuvYToTekHVC, iFEpppu)
//GO(_XcmsCIEuvY_ValidSpec, 
DATA(XcmsCIExyYColorSpace, 6*sizeof(void*)) // probably need some wrapping
//DATA(_XcmsCIExyY_prefix, 
//GOM(XcmsCIExyYToCIEXYZ, iFEpppu)
DATA(XcmsCIEXYZColorSpace, 6*sizeof(void*))
//DATA(_XcmsCIEXYZ_prefix, 
//GOM(XcmsCIEXYZToCIELab, iFEpppu)
//GOM(XcmsCIEXYZToCIEuvY, iFEpppu)
//GOM(XcmsCIEXYZToCIExyY, iFEpppu)
//GOM(XcmsCIEXYZToRGBi, iFEppup)
//GO(_XcmsCIEXYZ_ValidSpec, 
//GOM(XcmsClientWhitePointOfCCC, pFEp)
//GOM(XcmsConvertColors, iFEppuLp)
//GO(_XcmsConvertColorsWithWhitePt, 
//GO(_XcmsCopyCmapRecAndFree, 
//GO(_XcmsCopyISOLatin1Lowered, 
//GO(_XcmsCopyPointerArray, 
//GO(_XcmsCosine, 
//GOM(XcmsCreateCCC, pFEpipppppp)
//GO(_XcmsCubeRoot, 
DATA(_XcmsDDColorSpaces, 6*sizeof(void*))
DATA(_XcmsDDColorSpacesInit, 6*sizeof(void*))
//GO(_XcmsDDConvertColors, 
//GOM(XcmsDefaultCCC, pFEpi)
//GO(_XcmsDeleteCmapRec, 
DATA(_XcmsDIColorSpaces, 6*sizeof(void*))
DATA(_XcmsDIColorSpacesInit, 6*sizeof(void*))
//GO(_XcmsDIConvertColors, 
//GOM(XcmsDisplayOfCCC, pFEp)
//GO(_XcmsEqualWhitePts, 
GO(XcmsFormatOfPrefix, LFp)
GOM(XcmsFreeCCC, vFp)
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
//GOM(XcmsQueryBlack, iFEpLp)
//GOM(XcmsQueryBlue, iFEpLp)
GO(XcmsQueryColor, iFpLpL)
GO(XcmsQueryColors, iFpLpuL)
//GOM(XcmsQueryGreen, iFEpLp)
//GOM(XcmsQueryRed, iFEpLp)
//GOM(XcmsQueryWhite, iFEpLp)
DATA(_XcmsRegColorSpaces, 6*sizeof(void*))
//GO(_XcmsRegFormatOfPrefix, 
//GO(_XcmsResolveColor, 
//GO(_XcmsResolveColorString, 
DATA(XcmsRGBColorSpace, 6*sizeof(void*))
DATA(XcmsRGBiColorSpace, 6*sizeof(void*))
//DATA(_XcmsRGBi_prefix, 
//GOM(XcmsRGBiToCIEXYZ, iFEppup)
//GOM(XcmsRGBiToRGB, iFEppup)
//DATA(_XcmsRGB_prefix, 
//GOM(XcmsRGBToRGBi, iFEppup)
//GO(_XcmsRGB_to_XColor, 
DATA(_XcmsSCCFuncSets, 3*sizeof(void*))
DATA(_XcmsSCCFuncSetsInit, 3*sizeof(void*))
//GOM(XcmsScreenNumberOfCCC, iFEp)
//GOM(XcmsScreenWhitePointOfCCC, pFEp)
//GOM(XcmsSetCCCOfColormap, pFEpLp)
//GOM(XcmsSetCompressionProc, pFEppp)
//GO(_XcmsSetGetColor, 
//GO(_XcmsSetGetColors, 
//GOM(XcmsSetWhiteAdjustProc, pFEppp)
//GOM(XcmsSetWhitePoint, iFEpp)
//GO(_XcmsSine, 
//GO(_XcmsSquareRoot, 
GO(XcmsStoreColor, iFpLp)
GO(XcmsStoreColors, iFpLpup)
//GO(_XcmsTekHVC_CheckModify, 
//GOM(XcmsTekHVCClipC, iFEppuup)
//GOM(XcmsTekHVCClipV, iFEppuup)
//GOM(XcmsTekHVCClipVC, iFEppuup)
DATA(XcmsTekHVCColorSpace, 6*sizeof(void*))
//DATA(_XcmsTekHVC_prefix, 
//GOM(XcmsTekHVCQueryMaxC, iFEpddp)
//GOM(XcmsTekHVCQueryMaxV, iFEpddp)
//GOM(XcmsTekHVCQueryMaxVC, iFEpdp)
//GO(_XcmsTekHVCQueryMaxVCRGB, 
//GOM(XcmsTekHVCQueryMaxVSamples, iFEpdpu)
//GOM(XcmsTekHVCQueryMinV, iFEpddp)
//GOM(XcmsTekHVCToCIEuvY, iFEpppu)
//GOM(XcmsTekHVCWhiteShiftColors, iFEpppLpup)
DATA(XcmsUNDEFINEDColorSpace, 6*sizeof(void*))
//GO(_XcmsUnresolveColor, 
//GOM(XcmsVisualOfCCC, pFEp)
//GO(_XColor_to_XcmsRGB, 
GO(XConfigureWindow, iFpLup)
GO(XConnectionNumber, iFp)
//GO(_XConnectXCB, 
GO(XContextDependentDrawing, iFp)
GO(XContextualDrawing, iFp)
GO(XConvertCase, vFLpp)
GO(XConvertSelection, iFpLLLLL)
GO(XCopyArea, iFpLLpiiuuii)
GO(XCopyColormapAndFree, LFpL)
GO(_XCopyEventCookie, iFppp)
GO(XCopyGC, iFppLp)
GO(XCopyPlane, iFpLLpiiuuiiL)
//GO(_XCopyToArg, 
GO(XCreateBitmapFromData, LFpLpuu)
GO(XCreateColormap, LFpLpi)
GO(XCreateFontCursor, LFpu)
GO(XCreateFontSet, pFppppp)
GO(XCreateGC, pFpLLp)
GO(XCreateGlyphCursor, LFpLLuupp)
GOM(XCreateIC, pFEpV)     // use vararg
GOM(XCreateImage, pFEppuiipuuii)
GO(dummy_XCreateImage, pFppuiipuuii)    // to have the wrapper
DATAB(_XCreateMutex_fn, sizeof(void*))
GO(XCreateOC, pFppppppppppppppppppp)    // is pFpV
GO(XCreatePixmap, LFpLuuu)
GO(XCreatePixmapCursor, LFpLLppuu)
GO(XCreatePixmapFromBitmapData, LFpLpuuLLu)
GO(XCreateRegion, pFv)
GO(XCreateSimpleWindow, LFpLiiuuuLL)
GOM(XCreateWindow, LFEpLiiuuuiupLp)
DATAB(_Xdebug, sizeof(void*))
GO(XDefaultColormap, LFpi)
GO(XDefaultColormapOfScreen, LFp)
GO(XDefaultDepth, iFpi)
GO(XDefaultDepthOfScreen, iFp)
GO(_XDefaultError, iFpp)
GO(XDefaultGC, pFpi)
GO(XDefaultGCOfScreen, pFp)
GO(_XDefaultIOError, iFp)
GO(_XDefaultIOErrorExit, vFpp)
//GO(_XDefaultOpenIM, 
//GO(_XDefaultOpenOM, 
GO(XDefaultRootWindow, LFp)
GO(XDefaultScreen, iFp)
GO(XDefaultScreenOfDisplay, pFp)
GO(XDefaultString, pFv)
GO(XDefaultVisual, pFpi)
GO(XDefaultVisualOfScreen, pFp)
GO(_XDefaultWireError, iFppp)
GO(XDefineCursor, iFpLL)
GO(XDeleteContext, iFpLi)
GO(XDeleteModifiermapEntry, pFpCi)
GO(XDeleteProperty, iFpLL)
GO(_XDeq, vFppp)
GOM(_XDeqAsyncHandler, vFEpp)
GO(XDestroyIC, vFp)
GOM(XDestroyImage, iFEp)  //need to unbridge
GO(XDestroyOC, vFp)
GO(XDestroyRegion, iFp)
GO(XDestroySubwindows, iFpL)
GO(XDestroyWindow, iFpL)
GO(XDirectionalDependentDrawing, iFp)
GO(XDisableAccessControl, iFp)
GO(XDisplayCells, iFpi)
GO(XDisplayHeight, iFpi)
GO(XDisplayHeightMM, iFpi)
GO(XDisplayKeycodes, iFppp)
GO(XDisplayMotionBufferSize, LFp)
GO(XDisplayName, pFp)
GO(XDisplayOfIM, pFp)
GO(XDisplayOfOM, pFp)
GO(XDisplayOfScreen, pFp)
GO(XDisplayPlanes, iFpi)
GO(XDisplayString, pFp)
GO(XDisplayWidth, iFpi)
GO(XDisplayWidthMM, iFpi)
GO(XDoesBackingStore, iFp)
GO(XDoesSaveUnders, iFp)
GO(XDrawArc, iFpLpiiuuii)
GO(XDrawArcs, iFpLppi)
GO(XDrawImageString, iFpLpiipi)
GO(XDrawImageString16, iFpLpiipi)
GO(XDrawLine, iFpLpiiii)
GO(XDrawLines, iFpLppii)
GO(XDrawPoint, iFpLpii)
GO(XDrawPoints, iFpLppii)
GO(XDrawRectangle, iFpLpiiuu)
GO(XDrawRectangles, iFpLppi)
GO(XDrawSegments, iFpLppi)
GO(XDrawString, iFpLpiipi)
GO(XDrawString16, iFpLpiipi)
GO(XDrawText, iFpLpiipi)
GO(XDrawText16, iFpLpiipi)
GO(_XEatData, vFpL)
GO(_XEatDataWords, vFpL)
//GO(XEHeadOfExtensionList, 
GO(XEmptyRegion, iFp)
GO(XEnableAccessControl, iFp)
GO(_XEnq, vFpp)
GO(XEqualRegion, iFpp)
GO(_XError, iFpp)
DATAB(_XErrorFunction, sizeof(void*))
//GOM(XESetBeforeFlush, pFEpip)
GOM(XESetCloseDisplay, pFEpip)
//GOM(XESetCopyEventCookie, pFEpip)
//GOM(XESetCopyGC, pFEpip)
//GOM(XESetCreateFont, pFEpip)
//GOM(XESetCreateGC, pFEpip)
GOM(XESetError, pFEpip)
//GOM(XESetErrorString, pFEpip)
GOM(XESetEventToWire, pFEpip)
//GOM(XESetFlushGC, pFEpip)
//GOM(XESetFreeFont, pFEpip)
//GOM(XESetFreeGC, pFEpip)
//GOM(XESetPrintErrorValues, pFEpip)
//GOM(XESetWireToError, pFEpip)
GOM(XESetWireToEvent, pFEpip)
//GOM(XESetWireToEventCookie, pFEpip)
GO(XEventMaskOfScreen, lFp)
GO(_XEventsQueued, iFpi)
GO(XEventsQueued, iFpi)
//DATA(_Xevent_to_mask, 
GO(_XEventToWire, iFppp)
GO(XExtendedMaxRequestSize, lFp)
GO(XExtentsOfFontSet, pFp)
//GO(_XF86BigfontFreeFontMetrics, 
GO(_XF86LoadQueryLocaleFont, iFpppp)
GO(XFetchBuffer, pFppi)
GO(XFetchBytes, pFpp)
GO(_XFetchEventCookie, iFpp)
GO(XFetchName, iFpLp)
GO(XFillArc, iFpLpiiuuii)
GO(XFillArcs, iFpLppi)
GO(XFillPolygon, iFpLppiii)
GO(XFillRectangle, iFpLpiiuu)
GO(XFillRectangles, iFpLppi)
GO(XFilterEvent, iFpL)
GO(XFindContext, iFpLip)
GO(XFindOnExtensionList, pFpi)
GO(_XFlush, vFp)
GO(XFlush, iFp)
GO(XFlushGC, vFpp)
GO(_XFlushGCCache, vFpp)
GO(XFontsOfFontSet, iFppp)
GO(XForceScreenSaver, iFpi)
GO(XFree, iFp)
//GO(_XFreeAtomTable, 
GO(XFreeColormap, iFpL)
GO(XFreeColors, iFpLpiL)
GO(XFreeCursor, iFpL)
DATAB(_XFreeDisplayLock_fn, sizeof(void*))
//GO(_XFreeDisplayStructure, 
GO(_XFreeEventCookies, vFp)
GO(XFreeEventData, vFpp)
GO(_XFreeExtData, iFp)
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
GO(_XFreeTemp, vFppL)
GO(XFreeThreads, iFv)
//GO(_XFreeX11XCBStructure, 
GO(XGContextFromGC, LFp)
GO(XGeometry, iFpippuuuiipppp)
GO(_XGetAsyncData, vFpppiiii)
GO(_XGetAsyncReply, pFppppiii)
GO(XGetAtomName, pFpL)
GO(XGetAtomNames, iFppip)
//GO(_XGetBitsPerPixel, 
GO(XGetClassHint, iFpLp)
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
GOM(XGetImage, pFEpLiiuuLi)     // return an XImage with callbacks that needs wrapping
GO(XGetIMValues, pFppppppp)     // use varargs
GO(XGetInputFocus, iFppp)
GO(XGetKeyboardControl, iFpp)
GO(XGetKeyboardMapping, pFpCip)
//GO(_XGetLCValues, 
GO(XGetModifierMapping, pFp)
GO(XGetMotionEvents, pFpLLLp)
GO(XGetNormalHints, iFpLp)
GO(XGetOCValues, pFpppppppppp) // use varargs
GO(XGetOMValues, pFp)
//GOM(XGetPixel, LFEpii)  // need unbridging
GO(dummy_XGetPixel, LFpii)     // for the wrapper
GO(XGetPointerControl, iFpppp)
GO(XGetPointerMapping, iFppi)
GOM(_XGetRequest, pFEpCL)
GO(XGetRGBColormaps, iFpLppL)
//GO(_XGetScanlinePad, 
GO(XGetScreenSaver, iFppppp)
GO(XGetSelectionOwner, LFpL)
GO(XGetSizeHints, iFpLpL)
GO(XGetStandardColormap, iFpLpL)
GOM(XGetSubImage, pFEpLiiuuLipii)
GO(XGetTextProperty, iFpLpL)
GO(XGetTransientForHint, iFpLp)
GO(XGetVisualInfo, pFplpp)
GO(_XGetWindowAttributes, iFpLp)
GO(XGetWindowAttributes, iFpLp)
GO(XGetWindowProperty, iFpLLlliLppppp)
GO(XGetWMClientMachine, iFpLp)
GO(XGetWMColormapWindows, iFpLpp)
GO(XGetWMHints, pFpL)
GO(XGetWMIconName, iFpLp)
GO(XGetWMName, iFpLp)
GO(XGetWMNormalHints, iFpLpp)
GO(XGetWMProtocols, iFpLpp)
GO(XGetWMSizeHints, iFpLppL)
GO(XGetZoomHints, iFpLp)
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
GO(XIconifyWindow, iFpLi)
GOM(XIfEvent, iFEpppp)
GO(XImageByteOrder, iFp)
//GO(_XimCbDispatch, 
//GO(_XimCheckCreateICValues, 
//GO(_XimCheckDataSize, 
//GO(_XimCheckICMode, 
//GO(_XimCheckIfLocalProcessing, 
//GO(_XimCheckIfThaiProcessing, 
//GO(_XimCheckIMMode, 
//GO(_XimCheckLocalInputStyle, 
//GO(_XimCommitCallback, 
//GO(_XIMCompileResourceList, 
//GO(_XimConnect, 
//GO(_Ximctstombs, 
//GO(_Ximctstoutf8, 
//GO(_Ximctstowcs, 
//GO(_XimDecodeICATTRIBUTE, 
//GO(_XimDecodeIMATTRIBUTE, 
//GO(_XimDecodeLocalICAttr, 
//GO(_XimDecodeLocalIMAttr, 
//GO(_XimDestroyIMStructureList, 
//GO(_XimDispatchInit, 
//GO(_XimEncodeICATTRIBUTE, 
//GO(_XimEncodeIMATTRIBUTE, 
//GO(_XimEncodeLocalICAttr, 
//GO(_XimEncodeLocalIMAttr, 
//GO(_XimError, 
//GO(_XimErrorCallback, 
//GO(_XimExtension, 
//GO(_XimFilterWaitEvent, 
//GO(_XimFlush, 
//GO(_XimForwardEvent, 
//GO(_XimForwardEventCallback, 
//GO(_XimFreeCommitInfo, 
//GO(_XimFreeProtoIntrCallback, 
//GO(_XimFreeTransIntrCallback, 
//GO(_XimGetAttributeID, 
//GO(_XimGetCharCode, 
//GO(_XimGetCurrentICValues, 
//GO(_XimGetCurrentIMValues, 
//GO(_XimGetICValueData, 
//GO(_XimGetIMValueData, 
//GO(_XimGetLocaleCode, 
//GO(_XimGetMyEndian, 
//GO(_XimGetResourceListRec, 
//GO(_XimGetResourceListRecByQuark, 
//GO(_XimGetWindowEventmask, 
//GO(_XimICOfXICID, 
//DATA(_XimImSportRec, 
//GO(_XimInitialResourceInfo, 
//GO(_XimLcctstombs, 
//GO(_XimLcctstoutf8, 
//GO(_XimLcctstowcs, 
//GO(_XimLocalCreateIC, 
//GO(_XimLocalFilter, 
//GO(_XimLocalGetICValues, 
//GO(_XimLocalGetIMValues, 
//GO(_XimLocalIMFree, 
//GO(_XimLocalMbLookupString, 
//GO(_XimLocalOpenIM, 
//GO(_XimLocalSetICValues, 
//GO(_XimLocalSetIMValues, 
//GO(_XimLocalUtf8LookupString, 
//GO(_XimLocalWcLookupString, 
//GO(_XimLookupMBText, 
//GO(_XimLookupUTF8Text, 
//GO(_XimLookupWCText, 
//GO(_XimMakeICAttrIDList, 
//GO(_XimMakeIMAttrIDList, 
GO(XIMOfIC, pFp)
//GO(_XimOpenIM, 
//GO(_XimParseStringFile, 
//GO(_XimProcError, 
//GO(_XimProcSyncReply, 
//GO(_XimProtoCreateIC, 
//GO(_XimProtoEventToWire, 
//GO(_XimProtoIMFree, 
//GO(_XimProtoMbLookupString, 
//GO(_XimProtoOpenIM, 
//GO(_XimProtoUtf8LookupString, 
//GO(_XimProtoWcLookupString, 
//GO(_XimProtoWireToEvent, 
//GO(_XimRead, 
//GO(_XimRegisterDispatcher, 
//GO(_XimRegisterFilter, 
//GO(_XimRegisterIMInstantiateCallback, 
//GO(_XimRegisterServerFilter, 
//GO(_XimRegisterTriggerKeysCallback, 
//GO(_XimRegProtoIntrCallback, 
//GO(_XimReregisterFilter, 
//GO(_XimResetIMInstantiateCallback, 
//GO(_XimRespSyncReply, 
//GO(_XimServerDestroy, 
//GO(_XimSetCurrentICValues, 
//GO(_XimSetCurrentIMValues, 
//GO(_XimSetEventMaskCallback, 
//GO(_XimSetHeader, 
//GO(_XimSetICDefaults, 
//GO(_XimSetICMode, 
//GO(_XimSetICResourceList, 
//GO(_XimSetICValueData, 
//GO(_XimSetIMMode, 
//GO(_XimSetIMResourceList, 
//GO(_XimSetIMValueData, 
//GO(_XimSetInnerICResourceList, 
//GO(_XimSetInnerIMResourceList, 
//GO(_XimSetLocalIMDefaults, 
//GO(_XimShutdown, 
//GO(_XimSync, 
//GO(_XimSyncCallback, 
//GO(_XimThaiCloseIM, 
//GO(_XimThaiCreateIC, 
//GO(_XimThaiFilter, 
//GO(_XimThaiIMFree, 
//GO(_XimThaiOpenIM, 
//GO(_XimTransCallDispatcher, 
//GO(_XimTransConf, 
//GO(_XimTransFilterWaitEvent, 
//GO(_XimTransFlush, 
//GO(_XimTransInternalConnection, 
//DATA(_XimTransportRec, 
//GO(_XimTransRead, 
//GO(_XimTransRegisterDispatcher, 
//GO(_XimTransWrite, 
//GO(_XimTriggerNotify, 
//GO(_XimUnregisterFilter, 
//GO(_XimUnRegisterIMInstantiateCallback, 
//GO(_XimUnregisterServerFilter, 
//GO(_XimWrite, 
//GO(_XimXConf, 
//GO(_XimXTransBytesReadable, 
//GO(_XimXTransClose, 
//GO(_XimXTransCloseForCloning, 
//GO(_XimXTransConnect, 
//GO(_XimXTransDisconnect, 
//GO(_XimXTransFreeConnInfo, 
//GO(_XimXTransGetConnectionNumber, 
//GO(_XimXTransGetHostname, 
//GO(_XimXTransGetPeerAddr, 
//GO(_XimXTransIsLocal, 
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
DATAB(_XInitDisplayLock_fn, sizeof(void*))
GO(XInitExtension, pFpp)
//GO(_XInitIM, 
GOM(XInitImage, iFEp)
GOM(_XInitImageFuncPtrs, iFEp)
//GO(_XInitKeysymDB, 
//GO(_XInitOM, 
GO(XInitThreads, iFv)
GO(XInsertModifiermapEntry, pFpCi)
GO(XInstallColormap, iFpL)
GO(XInternalConnectionNumbers, iFppp)
GO(XInternAtom, LFppi)
GO(XInternAtoms, iFppiip)
GO(XIntersectRegion, iFppp)
GO(_XIOError, iFp)
DATAB(_XIOErrorFunction, sizeof(void*))
GO(_XIsEventCookie, iFpp)
GO(XkbAddDeviceLedInfo, pFpuu)
GO(XkbAddGeomColor, pFppu)
GO(XkbAddGeomDoodad, pFppL)
GO(XkbAddGeomKey, pFp)
GO(XkbAddGeomKeyAlias, pFppp)
GO(XkbAddGeomOutline, pFpi)
GO(XkbAddGeomOverlay, pFpLi)
GO(XkbAddGeomOverlayKey, pFpppp)
GO(XkbAddGeomOverlayRow, pFpii)
GO(XkbAddGeomProperty, pFppp)
GO(XkbAddGeomRow, pFpi)
GO(XkbAddGeomSection, pFpLiii)
GO(XkbAddGeomShape, pFpLi)
GO(XkbAddKeyType, pFpLiii)
GO(XkbAllocClientMap, iFpuu)
GO(XkbAllocCompatMap, iFpuu)
GO(XkbAllocControls, iFpu)
GO(XkbAllocDeviceInfo, pFuuu)
GO(XkbAllocGeomColors, iFpi)
GO(XkbAllocGeomDoodads, iFpi)
GO(XkbAllocGeometry, iFpp)
GO(XkbAllocGeomKeyAliases, iFpi)
GO(XkbAllocGeomKeys, iFpi)
GO(XkbAllocGeomOutlines, iFpi)
GO(XkbAllocGeomOverlayKeys, iFpi)
GO(XkbAllocGeomOverlayRows, iFpi)
GO(XkbAllocGeomOverlays, iFpi)
GO(XkbAllocGeomPoints, iFpi)
GO(XkbAllocGeomProps, iFpi)
GO(XkbAllocGeomRows, iFpi)
GO(XkbAllocGeomSectionDoodads, iFpi)
GO(XkbAllocGeomSections, iFpi)
GO(XkbAllocGeomShapes, iFpi)
GO(XkbAllocIndicatorMaps, iFp)
GO(XkbAllocKeyboard, pFv)
GO(XkbAllocNames, iFpuii)
GO(XkbAllocServerMap, iFpuu)
GO(XkbApplyCompatMapToKey, iFpCp)
GO(XkbApplyVirtualModChanges, iFpup)
GO(XkbBell, iFpLiL)
GO(XkbBellEvent, iFpLiL)
GO(XkbChangeDeviceInfo, iFppp)
GO(XkbChangeEnabledControls, iFpuuu)
GO(XkbChangeKeycodeRange, iFpiip)
GO(XkbChangeMap, iFppp)
GO(XkbChangeNames, iFppp)
GO(XkbChangeTypesOfKey, iFpiiupp)
GO(XkbComputeEffectiveMap, iFppp)
GO(XkbComputeRowBounds, iFppp)
GO(XkbComputeSectionBounds, iFpp)
GO(XkbComputeShapeBounds, iFp)
GO(XkbComputeShapeTop, iFpp)
//GO(_XkbCopyFromReadBuffer, 
GO(XkbCopyKeyType, iFpp)
GO(XkbCopyKeyTypes, iFppi)
GO(XkbDeviceBell, iFpLiiiiL)
GO(XkbDeviceBellEvent, iFpLiiiiL)
GO(XkbFindOverlayForKey, pFppp)
GO(XkbForceBell, iFpi)
GO(XkbForceDeviceBell, iFpiiii)
GO(XkbFreeClientMap, vFpui)
GO(XkbFreeCompatMap, vFpui)
GO(XkbFreeComponentList, vFp)
GO(XkbFreeControls, vFpui)
GO(XkbFreeDeviceInfo, vFpui)
GO(XkbFreeGeomColors, vFpiii)
GO(XkbFreeGeomDoodads, vFpii)
GO(XkbFreeGeometry, vFpui)
GO(XkbFreeGeomKeyAliases, vFpiii)
GO(XkbFreeGeomKeys, vFpiii)
GO(XkbFreeGeomOutlines, vFpiii)
GO(XkbFreeGeomOverlayKeys, vFpiii)
GO(XkbFreeGeomOverlayRows, vFpiii)
GO(XkbFreeGeomOverlays, vFpiii)
GO(XkbFreeGeomPoints, vFpiii)
GO(XkbFreeGeomProperties, vFpiii)
GO(XkbFreeGeomRows, vFpiii)
GO(XkbFreeGeomSections, vFpiii)
GO(XkbFreeGeomShapes, vFpiii)
GO(XkbFreeIndicatorMaps, vFp)
GO(XkbFreeKeyboard, vFpui)
GO(XkbFreeNames, vFpui)
//GO(_XkbFreeReadBuffer, 
GO(XkbFreeServerMap, vFpui)
DATA(_XkbGetAtomNameFunc, sizeof(void*))
GO(XkbGetAutoRepeatRate, iFpupp)
GO(XkbGetAutoResetControls, iFppp)
//GO(_XkbGetCharset, 
GO(XkbGetCompatMap, iFpup)
GO(XkbGetControls, iFpLp)
//GO(_XkbGetConverters, 
GO(XkbGetDetectableAutoRepeat, iFpp)
GO(XkbGetDeviceButtonActions, iFppiuu)
GO(XkbGetDeviceInfo, pFpuuuu)
GO(XkbGetDeviceInfoChanges, iFppp)
GO(XkbGetDeviceLedInfo, iFppuuu)
GO(XkbGetGeometry, iFpp)
GO(XkbGetIndicatorMap, iFpLp)
GO(XkbGetIndicatorState, iFpup)
GO(XkbGetKeyActions, iFpuup)
GO(XkbGetKeyBehaviors, iFpuup)
GO(XkbGetKeyboard, pFpuu)
GO(XkbGetKeyboardByName, pFpupuui)
GO(XkbGetKeyExplicitComponents, iFpuup)
GO(XkbGetKeyModifierMap, iFpuup)
GO(XkbGetKeySyms, iFpuup)
GO(XkbGetKeyTypes, iFpuup)
GO(XkbGetKeyVirtualModMap, iFpuup)
GO(XkbGetMap, pFpuu)
GO(XkbGetMapChanges, iFppp)
GO(XkbGetNamedDeviceIndicator, iFpuuuLpppp)
GO(XkbGetNamedGeometry, iFppL)
GO(XkbGetNamedIndicator, iFpLpppp)
GO(XkbGetNames, iFpup)
GO(XkbGetPerClientControls, iFpp)
//GO(_XkbGetReadBufferCountedString, 
//GO(_XkbGetReadBufferPtr, 
GO(XkbGetState, iFpup)
GO(XkbGetUpdatedMap, iFpup)
GO(XkbGetVirtualMods, iFpup)
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
GO(XkbLockGroup, iFpuu)
GO(XkbLockModifiers, iFpuuu)
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
//GO(_XkbReadBufferCopy32, 
//GO(_XkbReadBufferCopyKeySyms, 
//GO(_XkbReadCopyData32, 
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
GO(XkbSelectEvents, iFpuuu)
//GOM(XkbSetAtomFuncs, vFEpp)
GO(XkbSetAutoRepeatRate, iFpuuu)
GO(XkbSetAutoResetControls, iFpupp)
GO(XkbSetCompatMap, iFpupi)
GO(XkbSetControls, iFpLp)
GO(XkbSetDebuggingFlags, iFpuupuupp)
GO(XkbSetDetectableAutoRepeat, iFpip)
GO(XkbSetDeviceButtonActions, iFppuu)
GO(XkbSetDeviceInfo, iFpup)
GO(XkbSetDeviceLedInfo, iFppuuu)
GO(XkbSetGeometry, iFpup)
GO(XkbSetIgnoreLockMods, iFpuuuuu)
GO(XkbSetIndicatorMap, iFpLp)
GO(XkbSetMap, iFpup)
GO(XkbSetNamedDeviceIndicator, iFpuuuLiiip)
GO(XkbSetNamedIndicator, iFpLiiip)
GO(XkbSetNames, iFpuuup)
GO(XkbSetPerClientControls, iFpup)
GO(XkbSetServerInternalMods, iFpuuuuu)
GO(XkbSetXlibControls, uFpuu)
//GO(_XkbSkipReadBufferData, 
GO(XkbToControl, cFc)
//GO(XkbTranslateKey, 
GO(XkbTranslateKeyCode, iFpCupp)
GO(XkbTranslateKeySym, iFppupip)
GO(XkbUpdateActionVirtualMods, iFppu)
GO(XkbUpdateKeyTypeVirtualMods, vFppup)
GO(XkbUpdateMapFromCore, iFpCiipp)
GO(XkbUseExtension, iFppp)
GO(XkbVirtualModsToReal, iFpup)
//GO(_XkbWriteCopyData32, 
//GO(_XkbWriteCopyKeySyms, 
GO(XkbXlibControlsImplemented, uFv)
//GO(_XKeycodeToKeysym, 
GO(XKeycodeToKeysym, LFpCi)
//GO(_XKeyInitialize, 
//GO(_XKeysymToKeycode, 
GO(XKeysymToKeycode, CFpL)
//GO(_XKeysymToModifiers, 
GO(XKeysymToString, pFL)
//DATA(_XkeyTable, 
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
//GO(_XlcDefaultLoader, 
//GO(_XlcDefaultMapModifiers, 
//GO(_XlcDeInitLoader, 
//GO(_XlcDestroyLC, 
//GO(_XlcDestroyLocaleDataBase, 
//GO(_XlcFileName, 
//GO(_XlcGenericLoader, 
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
//GO(_XlcUtf8Loader, 
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
GO(xlocaledir, vFpi)
GO(XLocaleOfFontSet, pFp)
GO(XLocaleOfIM, pFp)
GO(XLocaleOfOM, pFp)
GO(XLockDisplay, vFp)
DATAB(_XLockMutex_fn, sizeof(void*))
GO(XLookupColor, iFpLppp)
//GO(_XLookupKeysym, 
GO(XLookupKeysym, LFpi)
//GO(_XLookupString, 
GO(XLookupString, iFppipp)
GO(XLowerWindow, iFpL)
GO(XMapRaised, iFpL)
GO(XMapSubwindows, iFpL)
GO(XMapWindow, iFpL)
GO(XMaskEvent, iFplp)
GO(XMatchVisualInfo, iFpiiip)
GO(XMaxCmapsOfScreen, iFp)
GO(XMaxRequestSize, lFp)
//GO(_XmbDefaultDrawImageString, 
//GO(_XmbDefaultDrawString, 
//GO(_XmbDefaultTextEscapement, 
//GO(_XmbDefaultTextExtents, 
//GO(_XmbDefaultTextPerCharExtents, 
GO(XmbDrawImageString, vFpLppiipi)
GO(XmbDrawString, vFpLppiipi)
GO(XmbDrawText, vFpLpiipi)
//GO(_XmbGenericDrawImageString, 
//GO(_XmbGenericDrawString, 
//GO(_XmbGenericTextEscapement, 
//GO(_XmbGenericTextExtents, 
//GO(_XmbGenericTextPerCharExtents, 
GO(_Xmblen, iFpi)
GO(XmbLookupString, iFpppipp)
GO(XmbResetIC, pFp)
GO(XmbSetWMProperties, vFpLpppippp)
//GO(_Xmbstoutf8, 
//GO(_Xmbstowcs, 
GO(XmbTextEscapement, iFppi)
GO(XmbTextExtents, iFppipp)
//GO(_XmbTextListToTextProperty, 
GO(XmbTextListToTextProperty, iFppiup)
GO(XmbTextPerCharExtents, iFppippippp)
//GO(_XmbTextPropertyToTextList, 
GO(XmbTextPropertyToTextList, iFpppp)
GO(_Xmbtowc, iFppi)
GO(XMinCmapsOfScreen, iFp)
GO(XMoveResizeWindow, iFpLiiuu)
GO(XMoveWindow, iFpLii)
GO(XNewModifiermap, pFi)
GO(XNextEvent, iFpp)
GO(XNextRequest, LFp)
GO(XNoOp, iFp)
//GO(_XNoticeCreateBitmap, 
//GO(_XNoticePutBitmap, 
GO(XOffsetRegion, iFpii)
//GO(_XomConvert, 
//GO(_XomGenericDrawString, 
//GO(_XomGenericOpenOM, 
//GO(_XomGenericTextExtents, 
//GO(_XomGetFontDataFromFontSet, 
//GO(_XomInitConverter, 
GO(XOMOfOC, pFp)
GOM(XOpenDisplay, pFEp)
GOM(XOpenIM, pFEpppp)
//GO(_XOpenLC, 
GO(XOpenOM, pFpppp)
//GO(_XParseBaseFontNameList, 
GO(XParseColor, iFpLpp)
GO(XParseGeometry, iFppppp)
GO(XPeekEvent, iFpp)
GOM(XPeekIfEvent, iFEpppp)
GO(XPending, iFp)
GO(Xpermalloc, pFu)
GO(XPlanesOfScreen, iFp)
GO(XPointInRegion, iFpii)
GO(_XPollfdCacheAdd, vFpi)
GO(_XPollfdCacheDel, vFpi)
GO(_XPollfdCacheInit, iFp)
GO(XPolygonRegion, pFpii)
//GOM(_XProcessInternalConnection, vFEpp)
GO(XProcessInternalConnection, vFpi)
GO(_XProcessWindowAttributes, vFppLp)
GO(XProtocolRevision, iFp)
GO(XProtocolVersion, iFp)
GO(_XPutBackEvent, iFpp)
GO(XPutBackEvent, iFpp)
GOM(XPutImage, iFEpLppiiiiuu)
//GO(XPutPixel, 
GO(XQLength, iFp)
GO(XQueryBestCursor, iFpLuupp)
GO(XQueryBestSize, iFpiLuupp)
GO(XQueryBestStipple, iFpLuupp)
GO(XQueryBestTile, iFpLuupp)
GO(XQueryColor, iFpLp)
GO(XQueryColors, iFpLpi)
GOM(XQueryExtension, iFEppppp)
GO(XQueryFont, pFpL)
GO(XQueryKeymap, iFpp)
GO(XQueryPointer, iFpLppppppp)
GO(XQueryTextExtents, iFpLpipppp)
GO(XQueryTextExtents16, iFpLpipppp)
GO(XQueryTree, iFpLpppp)
GO(XRaiseWindow, iFpL)
GO(_XRead, iFppl)
GO(XReadBitmapFile, iFpLpppppp)
GO(XReadBitmapFileData, iFpppppp)
GO(_XReadEvents, vFp)
GO(_XReadPad, vFppl)
GO(XRebindKeysym, iFpLpipi)
GO(XRecolorCursor, iFpLpp)
GO(XReconfigureWMWindow, iFpLiup)
GO(XRectInRegion, iFpiiuu)
//GO(_XRefreshKeyboardMapping, 
GO(XRefreshKeyboardMapping, iFp)
//GO(_XRegisterFilterByMask, 
//GO(_XRegisterFilterByType, 
GOM(XRegisterIMInstantiateCallback, iFEpppppp)
//GOM(_XRegisterInternalConnection, iFEpipp)
GOM(XRemoveConnectionWatch, vFEppp)
GO(XRemoveFromSaveSet, iFpL)
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
//GOM(XrmEnumerateDatabase, iFEpppipp)
GO(XrmGetDatabase, pFp)
GO(XrmGetFileDatabase, pFp)
GO(XrmGetResource, iFppppp)
GO(XrmGetStringDatabase, pFp)
GO(XrmInitialize, vFv)
//GO(_XrmInitParseInfo, 
//GO(_XrmInternalStringToQuark, 
GO(XrmLocaleOfDatabase, pFp)
GO(XrmMergeDatabases, vFpp)
GO(XrmParseCommand, vFppippp)
GO(XrmPermStringToQuark, iFp)
GO(XrmPutFileDatabase, vFpp)
GO(XrmPutLineResource, vFpp)
GO(XrmPutResource, vFpppp)
GO(XrmPutStringResource, vFppp)
GO(XrmQGetResource, iFppppp)
GO(XrmQGetSearchList, iFppppi)
GO(XrmQGetSearchResource, iFpiipp)
GO(XrmQPutResource, vFpppip)
GO(XrmQPutStringResource, vFpppp)
GO(XrmQuarkToString, pFi)
GO(XrmSetDatabase, vFpp)
GO(XrmStringToBindingQuarkList, vFppp)
GO(XrmStringToQuark, iFp)
GO(XrmStringToQuarkList, vFpp)
GO(XrmUniqueQuark, iFv)
GO(XRootWindow, LFpi)
GO(XRootWindowOfScreen, LFp)
GO(XRotateBuffers, iFpi)
GO(XRotateWindowProperties, iFpLpii)
GO(XSaveContext, iFpLip)
GO(XScreenCount, iFp)
GO(XScreenNumberOfScreen, iFp)
GO(XScreenOfDisplay, pFpi)
GO(_XScreenOfWindow, pFpL)
GO(XScreenResourceString, pFp)
GO(XSelectInput, iFpLl)
GO(_XSend, vFppl)
GO(XSendEvent, iFpLilp)
GO(XServerVendor, pFp)
GO(XSetAccessControl, iFpi)
GOM(XSetAfterFunction, pFEpp)
GO(XSetArcMode, iFppi)
GO(XSetAuthorization, vFpipi)
GO(XSetBackground, iFppL)
GO(XSetClassHint, iFpLp)
GO(XSetClipMask, iFppL)
GO(XSetClipOrigin, iFppii)
GO(_XSetClipRectangles, vFppiipii)
GO(XSetClipRectangles, iFppiipii)
GO(XSetCloseDownMode, iFpi)
GO(XSetCommand, iFpLpi)
GO(XSetDashes, iFppipi)
GOM(XSetErrorHandler, pFEp)
GO(XSetFillRule, iFppi)
GO(XSetFillStyle, iFppi)
GO(XSetFont, iFppL)
GO(XSetFontPath, iFppi)
GO(XSetForeground, iFppL)
GO(XSetFunction, iFppi)
GO(XSetGraphicsExposures, iFppi)
GO(XSetICFocus, vFp)
GO(XSetIconName, iFpLp)
GO(XSetIconSizes, iFpLpi)
GOM(XSetICValues, pFEpV)
//GO(_XSetImage, 
GOM(XSetIMValues, pFEpV)
GO(XSetInputFocus, iFpLiL)
//GOM(XSetIOErrorExitHandler, vFEppp)
GOM(XSetIOErrorHandler, pFEp)
GO(_XSetLastRequestRead, LFpp)
GO(XSetLineAttributes, iFppuiii)
GO(XSetLocaleModifiers, pFp)
GO(XSetModifierMapping, iFpp)
GO(XSetNormalHints, iFpLp)
GO(XSetOCValues, pFpppppppppppppppp) // use vaarg
GO(XSetOMValues, pFpppppppppppppppp) // use vaarg
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
GO(XSetStipple, iFppL)
GO(XSetSubwindowMode, iFppi)
GO(XSetTextProperty, vFpLpL)
GO(XSetTile, iFppL)
GO(XSetTransientForHint, iFpLL)
GO(XSetTSOrigin, iFppii)
GO(XSetWindowBackground, iFpLL)
GO(XSetWindowBackgroundPixmap, iFpLL)
GO(XSetWindowBorder, iFpLL)
GO(XSetWindowBorderPixmap, iFpLL)
GO(XSetWindowBorderWidth, iFpLu)
GO(XSetWindowColormap, iFpLL)
GO(XSetWMClientMachine, vFpLp)
GO(XSetWMColormapWindows, iFpLpi)
GO(XSetWMHints, iFpLp)
GO(XSetWMIconName, vFpLp)
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
GO(_XStoreEventCookie, vFpp)
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
GO(XUndefineCursor, iFpL)
GO(XUngrabButton, iFpuuL)
GO(XUngrabKey, iFpiuL)
GO(XUngrabKeyboard, iFpL)
GO(XUngrabPointer, iFpL)
GO(XUngrabServer, iFp)
GO(XUninstallColormap, iFpL)
GO(XUnionRectWithRegion, iFppp)
GO(XUnionRegion, iFppp)
GO(_XUnknownCopyEventCookie, iFppp)
GO(_XUnknownNativeEvent, iFppp)
GO(_XUnknownWireEvent, iFppp)
GO(_XUnknownWireEventCookie, iFppp)
GO(XUnloadFont, iFpL)
GO(XUnlockDisplay, vFp)
DATA(_XUnlockMutex_fn, sizeof(void*))
GO(XUnmapSubwindows, iFpL)
GO(XUnmapWindow, iFpL)
//GO(_XUnregisterFilter, 
GOM(XUnregisterIMInstantiateCallback, iFEpppppp)
GO(_XUnregisterInternalConnection, vFpi)
//GO(_XUnresolveColor, 
GO(XUnsetICFocus, vFp)
//GO(_XUpdateAtomCache, 
//GO(_XUpdateGCCache, 
//GO(_Xutf8DefaultDrawImageString, 
//GO(_Xutf8DefaultDrawString, 
//GO(_Xutf8DefaultTextEscapement, 
//GO(_Xutf8DefaultTextExtents, 
//GO(_Xutf8DefaultTextPerCharExtents, 
GO(Xutf8DrawImageString, vFpLppiipi)
GO(Xutf8DrawString, vFpLppiipi)
GO(Xutf8DrawText, vFpLpiipi)
//GO(_Xutf8GenericDrawImageString, 
//GO(_Xutf8GenericDrawString, 
//GO(_Xutf8GenericTextEscapement, 
//GO(_Xutf8GenericTextExtents, 
//GO(_Xutf8GenericTextPerCharExtents, 
GO(Xutf8LookupString, iFpppipp)
GO(Xutf8ResetIC, pFp)
GO(Xutf8SetWMProperties, vFpLpppippp)
GO(Xutf8TextEscapement, iFppi)
GO(Xutf8TextExtents, iFppipp)
//GO(_Xutf8TextListToTextProperty, 
GO(Xutf8TextListToTextProperty, iFppiup)
GO(Xutf8TextPerCharExtents, iFppippippp)
//GO(_Xutf8TextPropertyToTextList, 
GO(Xutf8TextPropertyToTextList, iFpppp)
GOM(XVaCreateNestedList, pFEiV)
GO(XVendorRelease, iFp)
GO(_XVIDtoVisual, pFpL)
GO(XVisualIDFromVisual, LFp)
GO(XWarpPointer, iFpLLiiuuii)
//GO(_XwcDefaultDrawImageString, 
//GO(_XwcDefaultDrawString, 
//GO(_XwcDefaultTextEscapement, 
//GO(_XwcDefaultTextExtents, 
//GO(_XwcDefaultTextPerCharExtents, 
GO(XwcDrawImageString, vFpLppiipi)
GO(XwcDrawString, vFpLppiipi)
GO(XwcDrawText, vFpLpiipi)
//GO(_XwcFreeStringList, 
GO(XwcFreeStringList, vFp)
//GO(_XwcGenericDrawImageString, 
//GO(_XwcGenericDrawString, 
//GO(_XwcGenericTextEscapement, 
//GO(_XwcGenericTextExtents, 
//GO(_XwcGenericTextPerCharExtents, 
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
//GO(_XwcTextListToTextProperty, 
GO(XwcTextListToTextProperty, iFppiup)
GO(XwcTextPerCharExtents, iFppippippp)
//GO(_XwcTextPropertyToTextList, 
GO(XwcTextPropertyToTextList, iFpppp)
GO(_Xwctomb, iFpi)
GO(XWhitePixel, LFpi)
GO(XWhitePixelOfScreen, LFp)
GO(XWidthMMOfScreen, iFp)
GO(XWidthOfScreen, iFp)
GO(XWindowEvent, iFpLlp)
GO(_XWireToEvent, iFppp)
GO(XWithdrawWindow, iFpLi)
GO(XWMGeometry, iFpippupppppp)
GO(XWriteBitmapFile, iFppLuuii)
GO(XXorRegion, iFppp)

GO(_XData32, iFppu)
GO(_XRead32, vFppl)

GO(dummy_putpixel, iFpiiL)
GO(dummy_addpixel, iFpl)
