#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

//XColor is a struct: LWWWcc
//XTextProperty is a struct: pLiL

DATAB(_qfree, 4) // Warning: failed to confirm

GO(XActivateScreenSaver, iFX)
GOM(XAddConnectionWatch, iFEXpp)
GO(XAddExtension, pFX)
GO(XAddHost, iFXbiip_)
GO(XAddHosts, iFXbiip_i)
GO(XAddPixel, vFpl)
//GOM(XAddToExtensionList, iFEbp_p)
GO(XAddToSaveSet, iFXL)
GO(XAllocClassHint, pFv)
GO(XAllocColor, iFXLbLWWWcc_)
GO(XAllocColorCells, iFXLibL_ubL_u)
GO(XAllocColorPlanes, iFXLibL_iiiibL_bL_bL_)
GO(XAllocIconSize, pFv)
GO(_XAllocID, LFX)
GO(_XAllocIDs, vFXbL_i)
GO(XAllocNamedColor, iFXLpbLWWWcc_bLWWWcc_)
GO(_XAllocScratch, pFXL)
GO(XAllocSizeHints, pFv)//GOS(XAllocSizeHints, bliiiiiiiiiippiii_Fv)
//GO(XAllocStandardColormap, bLLLLLLLLLL_Fv)
GO(_XAllocTemp, pFXL)
GO(XAllocWMHints, pFv)//GO(XAllocWMHints, bliiLLiiLL_Fv)
GO(XAllowEvents, iFXiL)
GO(XAllPlanes, LFv)
//GO(_XAsyncErrorHandler, iFpppip)
GO(XAutoRepeatOff, iFX)
GO(XAutoRepeatOn, iFX)
GOM(XBaseFontNameListOfFontSet, pFEp)
GO(XBell, iFXi)
GO(XBitmapBitOrder, iFX)
GO(XBitmapPad, iFX)
GO(XBitmapUnit, iFX)
GO(XBlackPixel, LFXi)
//GOM(XBlackPixelOfScreen, LFEbpXLiiiiipippLLLiiiil_)
//GOM(XCellsOfScreen, iFEbpXLiiiiipippLLLiiiil_)
GO(XChangeActivePointerGrab, iFXuLL)
GO(XChangeGC, iFXpLriLLLiiiiiiiLLiiLiiiiLic_)
GO(XChangeKeyboardControl, iFXLp)
GO(XChangeKeyboardMapping, iFXiibL_i)
GO(XChangePointerControl, iFXiiiii)
GOM(XChangeProperty, iFEXLLLiipi)
GO(XChangeSaveSet, iFXLi)
GOM(XChangeWindowAttributes, iFEXLLp)
GOM(XCheckIfEvent, iFEXppp)
GOM(XCheckMaskEvent, iFEXlp)
GOM(XCheckTypedEvent, iFEXip)
GOM(XCheckTypedWindowEvent, iFEXLip)
GOM(XCheckWindowEvent, iFEXLlp)
GO(XCirculateSubwindows, iFXLi)
GO(XCirculateSubwindowsDown, iFXL)
GO(XCirculateSubwindowsUp, iFXL)
GO(XClearArea, iFXLiiuui)
GO(XClearWindow, iFXL)
GO(XClipBox, iFpp)
GOM(XCloseDisplay, iFEX)
GO(XCloseIM, iFp)
//GO(_XCloseLC, 
GO(XCloseOM, iFp)
//GO(_XcmsAddCmapRec, 
//GOM(XcmsAddColorSpace, iFEbpLpbp_bp_i_)
//GOM(XcmsAddFunctionSet, iFEbbbpLpbp_bp_i__pp_)
//GO(XcmsAllocColor, iFpLpL)
//GO(XcmsAllocNamedColor, iFpLpppL)
//GO(_XcmsArcTangent, 
//GOM(XcmsCCCOfColormap, pFEXL)
//GOM(XcmsCIELabClipab, iFEppuup)
//GOM(XcmsCIELabClipL, iFEppuup)
//GOM(XcmsCIELabClipLab, iFEppuup)
//DATA(XcmsCIELabColorSpace, 6*4)
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
//DATA(XcmsCIELuvColorSpace, 6*4)
//DATA(_XcmsCIELuv_prefix, 
//GOM(XcmsCIELuvQueryMaxC, iFEpddp)
//GOM(XcmsCIELuvQueryMaxL, iFEpddp)
//GOM(XcmsCIELuvQueryMaxLC, iFEpdp)
//GO(_XcmsCIELuvQueryMaxLCRGB, 
//GOM(XcmsCIELuvQueryMinL, iFEpddp)
//GOM(XcmsCIELuvToCIEuvY, iFEpppu)
//GOM(XcmsCIELuvWhiteShiftColors, iFEpppLpup)
//DATA(XcmsCIEuvYColorSpace, 6*4)
//DATA(_XcmsCIEuvY_prefix, 
//GOM(XcmsCIEuvYToCIELuv, iFEpppu)
//GOM(XcmsCIEuvYToCIEXYZ, iFEpppu)
//GOM(XcmsCIEuvYToTekHVC, iFEpppu)
//GO(_XcmsCIEuvY_ValidSpec, 
//DATA(XcmsCIExyYColorSpace, 6*4) // probably need some wrapping
//DATA(_XcmsCIExyY_prefix, 
//GOM(XcmsCIExyYToCIEXYZ, iFEpppu)
//DATA(XcmsCIEXYZColorSpace, 24)
//DATA(_XcmsCIEXYZ_prefix, 
//GOM(XcmsCIEXYZToCIELab, iFEpppu)
//GOM(XcmsCIEXYZToCIEuvY, iFEpppu)
//GOM(XcmsCIEXYZToCIExyY, iFEpppu)
//GOM(XcmsCIEXYZToRGBi, iFEppup)
//GO(_XcmsCIEXYZ_ValidSpec, 
//GOSM(XcmsClientWhitePointOfCCC, bpLL_FEp)
//GOM(XcmsConvertColors, iFEppuLp)
//GO(_XcmsConvertColorsWithWhitePt, 
//GO(_XcmsCopyCmapRecAndFree, 
//GO(_XcmsCopyISOLatin1Lowered, 
//GO(_XcmsCopyPointerArray, 
//GO(_XcmsCosine, 
//GOM(XcmsCreateCCC, pFEpipppppp)
//GO(_XcmsCubeRoot, 
//DATA(_XcmsDDColorSpaces, 6*4)
//DATA(_XcmsDDColorSpacesInit, 6*4)
//GO(_XcmsDDConvertColors, 
//GOM(XcmsDefaultCCC, pFEXi)
//GO(_XcmsDeleteCmapRec, 
//DATA(_XcmsDIColorSpaces, 6*4)
//DATA(_XcmsDIColorSpacesInit, 6*4)
//GO(_XcmsDIConvertColors, 
//GOM(XcmsDisplayOfCCC, XFEp)
//GO(_XcmsEqualWhitePts, 
GO(XcmsFormatOfPrefix, LFp)
GO(XcmsFreeCCC, vFp)
//GO(_XcmsFreeIntensityMaps, 
//GO(_XcmsFreePointerArray, 
//GO(_XcmsGetElement, 
//GO(_XcmsGetIntensityMap, 
//GO(_XcmsGetProperty, 
//GO(_XcmsInitDefaultCCCs, 
//GO(_XcmsInitScrnInfo, 
//DATA(XcmsLinearRGBFunctionSet, 3*4) // probably needs some wrapping
//GO(XcmsLookupColor, iFpLpppL)
//GO(_XcmsLRGB_InitScrnDefault, 
GO(XcmsPrefixOfFormat, pFL)
//GO(_XcmsPushPointerArray, 
//GOM(XcmsQueryBlack, iFEpLp)
//GOM(XcmsQueryBlue, iFEpLp)
//GO(XcmsQueryColor, iFpLpL)
//GO(XcmsQueryColors, iFpLpuL)
//GOM(XcmsQueryGreen, iFEpLp)
//GOM(XcmsQueryRed, iFEpLp)
//GOM(XcmsQueryWhite, iFEpLp)
//DATA(_XcmsRegColorSpaces, 6*4)
//GO(_XcmsRegFormatOfPrefix, 
//GO(_XcmsResolveColor, 
//GO(_XcmsResolveColorString, 
//DATA(XcmsRGBColorSpace, 6*4)
//DATA(XcmsRGBiColorSpace, 6*4)
//DATA(_XcmsRGBi_prefix, 
//GOM(XcmsRGBiToCIEXYZ, iFEppup)
//GOM(XcmsRGBiToRGB, iFEppup)
//DATA(_XcmsRGB_prefix, 
//GOM(XcmsRGBToRGBi, iFEppup)
//GO(_XcmsRGB_to_XColor, 
//DATA(_XcmsSCCFuncSets, 3*4)
//DATA(_XcmsSCCFuncSetsInit, 3*4)
//GOM(XcmsScreenNumberOfCCC, iFEp)
//GOSM(XcmsScreenWhitePointOfCCC, bpLL_FEp)
//GOM(XcmsSetCCCOfColormap, pFEXLp)
//GOM(XcmsSetCompressionProc, pFEppp)
//GO(_XcmsSetGetColor, 
//GO(_XcmsSetGetColors, 
//GOM(XcmsSetWhiteAdjustProc, pFEppp)
//GOM(XcmsSetWhitePoint, iFEpp)
//GO(_XcmsSine, 
//GO(_XcmsSquareRoot, 
//GO(XcmsStoreColor, iFpLp)
//GO(XcmsStoreColors, iFpLpup)
//GO(_XcmsTekHVC_CheckModify, 
//GOM(XcmsTekHVCClipC, iFEppuup)
//GOM(XcmsTekHVCClipV, iFEppuup)
//GOM(XcmsTekHVCClipVC, iFEppuup)
//DATA(XcmsTekHVCColorSpace, 6*4)
//DATA(_XcmsTekHVC_prefix, 
//GOM(XcmsTekHVCQueryMaxC, iFEpddp)
//GOM(XcmsTekHVCQueryMaxV, iFEpddp)
//GOM(XcmsTekHVCQueryMaxVC, iFEpdp)
//GO(_XcmsTekHVCQueryMaxVCRGB, 
//GOM(XcmsTekHVCQueryMaxVSamples, iFEpdpu)
//GOM(XcmsTekHVCQueryMinV, iFEpddp)
//GOM(XcmsTekHVCToCIEuvY, iFEpppu)
//GOM(XcmsTekHVCWhiteShiftColors, iFEpppLpup)
//DATA(XcmsUNDEFINEDColorSpace, 6*4)
//GO(_XcmsUnresolveColor, 
//GOM(XcmsVisualOfCCC, pFEp)
//GO(_XColor_to_XcmsRGB, 
GO(XConfigureWindow, iFXLuriiiiiLi_)
GO(XConnectionNumber, iFX)
//GO(_XConnectXCB, 
GOM(XContextDependentDrawing, iFEp)
GOM(XContextualDrawing, iFEp)
GO(XConvertCase, vFLbL_bL_)
GO(XConvertSelection, iFXLLLLL)
GO(XCopyArea, iFXLLpiiuuii)
GO(XCopyColormapAndFree, LFXL)
//GO(_XCopyEventCookie, iFXbiLiXiiup_biLiXiiup_)
GO(XCopyGC, iFXpLp)
GO(XCopyPlane, iFXLLpiiuuiiL)
//GO(_XCopyToArg, 
GO(XCreateBitmapFromData, LFXLpuu)
GOM(XCreateColormap, LFEXLpi)
GO(XCreateFontCursor, LFXu)
GOM(XCreateFontSet, pFEXpppp)
GO(XCreateGC, pFXLLbiLLLiiiiiiiLLiiLiiiiLic_)
GO(XCreateGlyphCursor, LFXLLuubLWWWcc_bLWWWcc_)
GOM(XCreateIC, pFEpV)
GOM(XCreateImage, pFEXpuiipuuii)
GO(dummy_XCreateImage, pFXpuiipuuii) // Warning: failed to confirm
//DATAB(_XCreateMutex_fn, 4)
//GO(XCreateOC, pFppppppppppppppppppp)    // is pFpV
GO(XCreatePixmap, LFXLuuu)
GO(XCreatePixmapCursor, LFXLLbLWWWcc_bLWWWcc_uu)
GO(XCreatePixmapFromBitmapData, LFXLpuuLLu)
GO(XCreateRegion, pFv)
GO(XCreateSimpleWindow, LFXLiiuuuLL)
GOM(XCreateWindow, LFEXLiiuuuiupLp)
//DATAB(_Xdebug, 4)
GO(XDefaultColormap, LFXi)
//GOM(XDefaultColormapOfScreen, LFEbpXLiiiiipippLLLiiiil_)
GO(XDefaultDepth, iFXi)
//GOM(XDefaultDepthOfScreen, iFEbpXLiiiiipippLLLiiiil_)
//GOM(_XDefaultError, iFEXbiXLLCCC_)
GO(XDefaultGC, pFXi)
//GOM(XDefaultGCOfScreen, pFEbpXLiiiiipippLLLiiiil_)
GO(_XDefaultIOError, iFX)
GO(_XDefaultIOErrorExit, vFXp)
//GO(_XDefaultOpenIM, 
//GO(_XDefaultOpenOM, 
GO(XDefaultRootWindow, LFX)
GO(XDefaultScreen, iFX)
//GOM(XDefaultScreenOfDisplay, bpXLiiiiipippLLLiiiil_FEX)
GO(XDefaultString, pFv)
GO(XDefaultVisual, pFXi)
//GOM(XDefaultVisualOfScreen, pFEbpXLiiiiipippLLLiiiil_)
//GO(_XDefaultWireError, iFppp)
GO(XDefineCursor, iFXLL)
GO(XDeleteContext, iFXLi)
//GO(XDeleteModifiermapEntry, bip_Fbip_Ci)
GO(XDeleteProperty, iFXLL)
//GOM(_XDeq, vFEXpp)
GOM(_XDeqAsyncHandler, vFEXp)
GO(XDestroyIC, vFp)
GOM(XDestroyImage, iFEp)  //need to unbridge
GO(XDestroyOC, vFp)
GO(XDestroyRegion, iFp)
GO(XDestroySubwindows, iFXL)
GO(XDestroyWindow, iFXL)
GOM(XDirectionalDependentDrawing, iFEp)
GO(XDisableAccessControl, iFX)
GO(XDisplayCells, iFXi)
GO(XDisplayHeight, iFXi)
GO(XDisplayHeightMM, iFXi)
GO(XDisplayKeycodes, iFXpp)
GO(XDisplayMotionBufferSize, LFX)
GO(XDisplayName, tFp)
GO(XDisplayOfIM, XFp)
GO(XDisplayOfOM, XFp)
//GOM(XDisplayOfScreen, XFEbpXLiiiiipippLLLiiiil_)
GO(XDisplayPlanes, iFXi)
GO(XDisplayString, pFX)
GO(XDisplayWidth, iFXi)
GO(XDisplayWidthMM, iFXi)
//GOM(XDoesBackingStore, iFEbpXLiiiiipippLLLiiiil_)
//GOM(XDoesSaveUnders, iFEbpXLiiiiipippLLLiiiil_)
GO(XDrawArc, iFXLpiiuuii)
GO(XDrawArcs, iFXLppi)
GO(XDrawImageString, iFXLpiipi)
GO(XDrawImageString16, iFXLpiipi)
GO(XDrawLine, iFXLpiiii)
GO(XDrawLines, iFXLppii)
GO(XDrawPoint, iFXLpii)
GO(XDrawPoints, iFXLppii)
GO(XDrawRectangle, iFXLpiiuu)
GO(XDrawRectangles, iFXLppi)
GO(XDrawSegments, iFXLppi)
GO(XDrawString, iFXLpiipi)
GO(XDrawString16, iFXLpiipi)
GO(XDrawText, iFXLpiibpiiL_i)
GO(XDrawText16, iFXLpiibpiiL_i)
GO(_XEatData, vFXL)
GO(_XEatDataWords, vFXL)
//GO(XEHeadOfExtensionList, 
GO(XEmptyRegion, iFp)
GO(XEnableAccessControl, iFX)
//GO(_XEnq, vFpp)
GO(XEqualRegion, iFpp)
//GO(_XError, iFpp)
//DATAM(_XErrorFunction, 4)
//GOM(XESetBeforeFlush, pFEXip)
GOM(XESetCloseDisplay, pFEXip)
//GOM(XESetCopyEventCookie, pFEXip)
//GOM(XESetCopyGC, pFEXip)
//GOM(XESetCreateFont, pFEXip)
//GOM(XESetCreateGC, pFEXip)
//GOM(XESetError, pFEXip)
//GOM(XESetErrorString, pFEXip)
GOM(XESetEventToWire, pFEXip)
//GOM(XESetFlushGC, pFEXip)
//GOM(XESetFreeFont, pFEXip)
//GOM(XESetFreeGC, pFEXip)
//GOM(XESetPrintErrorValues, pFEXip)
//GOM(XESetWireToError, pFEXip)
GOM(XESetWireToEvent, pFEXip)
//GOM(XESetWireToEventCookie, pFEXip)
//GOM(XEventMaskOfScreen, lFEbpXLiiiiipippLLLiiiil_)
GO(_XEventsQueued, iFXi)
GO(XEventsQueued, iFXi)
//DATA(_Xevent_to_mask, 
//GO(_XEventToWire, iFppp)
GO(XExtendedMaxRequestSize, lFX)
GOM(XExtentsOfFontSet, pFEp)
//GO(_XF86BigfontFreeFontMetrics, 
//GO(_XF86LoadQueryLocaleFont, iFpppp)
GO(XFetchBuffer, pFXpi)
GO(XFetchBytes, pFXp)
//GO(_XFetchEventCookie, iFXbiLiXiiup_)
GO(XFetchName, iFXLbp_)
GO(XFillArc, iFXLpiiuuii)
GO(XFillArcs, iFXLppi)
GO(XFillPolygon, iFXLppiii)
GO(XFillRectangle, iFXLpiiuu)
GO(XFillRectangles, iFXLppi)
GOM(XFilterEvent, iFEpL) // Warning: failed to confirm
GO(XFindContext, iFXLiBp_)
//GOM(XFindOnExtensionList, pFEbp_i)
GO(_XFlush, vFX)
GO(XFlush, iFX)
GO(XFlushGC, vFXp)
GO(_XFlushGCCache, vFXp)
GOM(XFontsOfFontSet, iFEppp)
GO(XForceScreenSaver, iFXi)
GO(XFree, iFp)
//GO(_XFreeAtomTable, 
GO(XFreeColormap, iFXL)
GOM(XFreeColors, iFEXLpiL)
GO(XFreeCursor, iFXL)
//DATAB(_XFreeDisplayLock_fn, 4)
//GO(_XFreeDisplayStructure, 
GO(_XFreeEventCookies, vFX)
GOM(XFreeEventData, vFEXp)
//GOM(_XFreeExtData, iFEp)
GOM(XFreeExtensionList, iFEp)
GOM(XFreeFont, iFEXp)
GOM(XFreeFontInfo, iFEppi)
GOM(XFreeFontNames, iFEp)
//GO(XFreeFontPath, iFbp_)
GOM(XFreeFontSet, vFEXp)
GO(XFreeGC, iFXp)
GOM(XFreeModifiermap, iFEp)
//DATAM(_XFreeMutex_fn, 4)
GO(XFreePixmap, iFXL)
GOM(XFreeStringList, vFEp)
GO(_XFreeTemp, vFXpL)
GO(XFreeThreads, iFv)
//GO(_XFreeX11XCBStructure, 
GO(XGContextFromGC, LFp)
GO(XGeometry, iFXippuuuiipppp)
GO(_XGetAsyncData, vFXppiiii)
GO(_XGetAsyncReply, pFppppiii)
GO(XGetAtomName, pFXL)
GO(XGetAtomNames, iFXbL_ibp_)
//GO(_XGetBitsPerPixel, 
GO(XGetClassHint, iFXLbpp_)
//GO(XGetCommand, iFXLbbp__p)
GO(XGetDefault, pFXpp)
GO(XGetErrorDatabaseText, iFXppppi)
GO(XGetErrorText, iFXipi)
GOM(XGetEventData, iFEXp)
//GO(XGetFontPath, bp_FXp)
//GO(XGetFontProperty, iFpLp)
GO(XGetGCValues, iFXpLbiLLLiiiiiiiLLiiLiiiiLic_)
GO(XGetGeometry, iFXLbL_pppppp)
GO(_XGetHostname, iFpi)
GO(XGetIconName, iFXLbp_)
GO(XGetIconSizes, iFXLbp_p)
GOM(XGetICValues, pFEpV)      // use varargs...
GOM(XGetImage, pFEXLiiuuLi)     // return an XImage with callbacks that needs wrapping
GOM(XGetIMValues, pFEpV)
GO(XGetInputFocus, iFXBL_p)
GO(XGetKeyboardControl, iFXbiiuuLip_)
GOM(XGetKeyboardMapping, pFEXCip) // return a KeySym*, so an unsigned long*
//GO(_XGetLCValues, 
GOM(XGetModifierMapping, pFEX)
GOM(XGetMotionEvents, pFEXLLLp)
//GO(XGetNormalHints, iFpLp)
//GO(XGetOCValues, pFpppppppppp) // use varargs
//GOM(XGetOMValues, pFEpV)
GOM(XGetPixel, LFEXii)  // need unbridging
GO(dummy_XGetPixel, LFpii)     // for the wrapper
GO(XGetPointerControl, iFXppp)
GO(XGetPointerMapping, iFXpi)
GOM(_XGetRequest, pFEXCL)
//GO(XGetRGBColormaps, iFXLbbLLLLLLLLLL__pL)
//GO(_XGetScanlinePad, 
GO(XGetScreenSaver, iFXpppp)
GO(XGetSelectionOwner, LFXL)
//GO(XGetSizeHints, iFpLpL)
GO(XGetStandardColormap, iFXLbLLLLLLLLLL_L)
GOM(XGetSubImage, pFEXLiiuuLipii)
GO(XGetTextProperty, iFXLbpLiL_L)
GO(XGetTransientForHint, iFXLbL_)
GOM(XGetVisualInfo, pFEXlpp)
//GOM(_XGetWindowAttributes, iFEXLbiiiiiipLiiiiLLiLiilllibpXLiiiiipippLLLiiiil__)
GOM(XGetWindowAttributes, iFEXLp)
GOM(XGetWindowProperty, iFEXLLlliLppppp)
GO(XGetWMClientMachine, iFXLbpLiL_)
GOM(XGetWMColormapWindows, iFEXLpp)
GOM(XGetWMHints, pFEXL)
GO(XGetWMIconName, iFXLbpLiL_)
GO(XGetWMName, iFXLbpLiL_)
GOM(XGetWMNormalHints, iFEXLpp)
GOM(XGetWMProtocols, iFEXLpp)
//GO(XGetWMSizeHints, iFpLppL)
//GO(XGetZoomHints, iFpLp)
DATAB(_Xglobal_lock, 4)
GO(XGrabButton, iFXuuLiuiiLL)
GO(XGrabKey, iFXiuLiii)
GO(XGrabKeyboard, iFXLiiiL)
GO(XGrabPointer, iFXLiuiiLLL)
GOM(XGrabServer, iFEX)
//DATAB(_XHeadOfDisplayList, 4)
//GOM(XHeightMMOfScreen, iFEbpXLiiiiipippLLLiiiil_)
//GOM(XHeightOfScreen, iFEbpXLiiiiipippLLLiiiil_)
//DATAB(_Xi18n_lock, 4)
GO(XIconifyWindow, iFXLi)
GOM(XIfEvent, iFEXppp)
GO(XImageByteOrder, iFX)
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
DATA(_XimXTransSocketINET6Funcs, 4)
DATA(_XimXTransSocketINETFuncs, 4)
DATA(_XimXTransSocketLocalFuncs, 4)
DATA(_XimXTransSocketTCPFuncs, 4)
DATA(_XimXTransSocketUNIXFuncs, 4)
//GO(_XimXTransWrite, 
//GO(_XimXTransWritev, 
DATAB(_XInitDisplayLock_fn, 4)
GO(XInitExtension, pFXp)
//GO(_XInitIM, 
GOM(XInitImage, iFEp)
GOM(_XInitImageFuncPtrs, iFEp)
//GO(_XInitKeysymDB, 
//GO(_XInitOM, 
GO(XInitThreads, iFv)
//GO(XInsertModifiermapEntry, bip_Fbip_Ci)
GO(XInstallColormap, iFXL)
GO(XInternalConnectionNumbers, iFXbp_p)
GO(XInternAtom, LFXpi)
GOM(XInternAtoms, iFEXpiip)
GO(XIntersectRegion, iFppp)
GO(_XIOError, iFX)
//DATAM(_XIOErrorFunction, 4)
//GO(_XIsEventCookie, iFpp)
//GO(XkbAddDeviceLedInfo, pFpuu)
//GO(XkbAddGeomColor, pFppu)
//GO(XkbAddGeomDoodad, pFppL)
//GO(XkbAddGeomKey, pFp)
//GO(XkbAddGeomKeyAlias, pFppp)
//GO(XkbAddGeomOutline, pFpi)
//GO(XkbAddGeomOverlay, pFpLi)
//GOM(XkbAddGeomOverlayKey, pFEpbWWWp_pp)
//GOM(XkbAddGeomOverlayRow, bWWWp_FEpii)
//GO(XkbAddGeomProperty, pFppp)
//GO(XkbAddGeomRow, pFpi)
//GO(XkbAddGeomSection, pFpLiii)
//GO(XkbAddGeomShape, pFpLi)
//GO(XkbAddKeyType, pFpLiii)
//GO(XkbAllocClientMap, iFpuu)
//GO(XkbAllocCompatMap, iFpuu)
//GO(XkbAllocControls, iFpu)
//GOS(XkbAllocDeviceInfo, bpLWiWWWpWWWWbWWuuuuLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLpppppppppppppppppppppppppppppppp__Fuuu)
//GO(XkbAllocGeomColors, iFpi)
//GO(XkbAllocGeomDoodads, iFpi)
//GO(XkbAllocGeometry, iFpp)
//GO(XkbAllocGeomKeyAliases, iFpi)
//GO(XkbAllocGeomKeys, iFpi)
//GO(XkbAllocGeomOutlines, iFpi)
//GO(XkbAllocGeomOverlayKeys, iFbWWWp_i)
//GOM(XkbAllocGeomOverlayRows, iFEpi)
//GO(XkbAllocGeomOverlays, iFpi)
//GO(XkbAllocGeomPoints, iFbWWWp_i)
//GO(XkbAllocGeomProps, iFpi)
//GO(XkbAllocGeomRows, iFpi)
//GO(XkbAllocGeomSectionDoodads, iFpi)
//GO(XkbAllocGeomSections, iFpi)
//GO(XkbAllocGeomShapes, iFpi)
//GO(XkbAllocIndicatorMaps, iFp)
//GOSM(XkbAllocKeyboard, bXWWCCpbWWppppCCCCCCCCCCCCCCCCp_bCCbpCCppLbL__WWbL_pp_bLpppppppppppppppppppppppppppppppp_bLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLppbL_LCCW_bbLCCCCp_ppppWW_bLWWpbup_bup_WWWWWWWWWWWWbpp_bup_bLWWbWWWp_bWWWp_bWWWp_p_bLCwwWWwWWWWWWbwwWWipp_bppppp_pp_bppppp_p__FEv)
//GO(XkbAllocNames, iFpuii)
//GO(XkbAllocServerMap, iFpuu)
//GO(XkbApplyCompatMapToKey, iFpCp)
//GO(XkbApplyVirtualModChanges, iFpup)
GO(XkbBell, iFXLiL)
GO(XkbBellEvent, iFXLiL)
//GO(XkbChangeDeviceInfo, iFppp)
GO(XkbChangeEnabledControls, iFXuuu)
//GO(XkbChangeKeycodeRange, iFpiip)
//GO(XkbChangeMap, iFppp)
//GO(XkbChangeNames, iFppp)
//GO(XkbChangeTypesOfKey, iFpiiupp)
//GO(XkbComputeEffectiveMap, iFppp)
//GO(XkbComputeRowBounds, iFppp)
//GO(XkbComputeSectionBounds, iFpp)
//GO(XkbComputeShapeBounds, iFp)
//GO(XkbComputeShapeTop, iFpp)
//GO(_XkbCopyFromReadBuffer, 
//GO(XkbCopyKeyType, iFpp)
//GO(XkbCopyKeyTypes, iFppi)
GO(XkbDeviceBell, iFXLiiiiL)
GO(XkbDeviceBellEvent, iFXLiiiiL)
//GO(XkbFindOverlayForKey, pFppp)
GO(XkbForceBell, iFXi)
GO(XkbForceDeviceBell, iFXiiii)
GO(XkbFreeClientMap, vFpui)
//GO(XkbFreeCompatMap, vFpui)
//GO(XkbFreeComponentList, vFbiiiiiibWp_bWp_bWp_bWp_bWp_bWp__)
//GO(XkbFreeControls, vFpui)
//GO(XkbFreeDeviceInfo, vFpui)
//GO(XkbFreeGeomColors, vFpiii)
//GO(XkbFreeGeomDoodads, vFpii)
//GO(XkbFreeGeometry, vFpui)
//GO(XkbFreeGeomKeyAliases, vFpiii)
//GO(XkbFreeGeomKeys, vFpiii)
//GO(XkbFreeGeomOutlines, vFpiii)
//GO(XkbFreeGeomOverlayKeys, vFbWWWp_iii)
//GOM(XkbFreeGeomOverlayRows, vFEpiii)
//GO(XkbFreeGeomOverlays, vFpiii)
//GO(XkbFreeGeomPoints, vFbWWWp_iii)
//GO(XkbFreeGeomProperties, vFpiii)
//GO(XkbFreeGeomRows, vFpiii)
//GO(XkbFreeGeomSections, vFpiii)
//GO(XkbFreeGeomShapes, vFpiii)
//GO(XkbFreeIndicatorMaps, vFp)
GOM(XkbFreeKeyboard, vFEpui)
//GO(XkbFreeNames, vFpui)
//GO(_XkbFreeReadBuffer, 
//GO(XkbFreeServerMap, vFpui)
//DATA(_XkbGetAtomNameFunc, 4)
GO(XkbGetAutoRepeatRate, iFXupp)
GO(XkbGetAutoResetControls, iFXpp)
//GO(_XkbGetCharset, 
//GO(XkbGetCompatMap, iFpup)
GOM(XkbGetControls, iFEXLp)
//GO(_XkbGetConverters, 
GO(XkbGetDetectableAutoRepeat, iFXp)
//GO(XkbGetDeviceButtonActions, iFppiuu)
//GOS(XkbGetDeviceInfo, bpLWiWWWpWWWWbWWuuuuLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLpppppppppppppppppppppppppppppppp__FXuuuu)
//GO(XkbGetDeviceInfoChanges, iFppp)
//GO(XkbGetDeviceLedInfo, iFppuuu)
//GO(XkbGetGeometry, iFpp)
//GO(XkbGetIndicatorMap, iFpLp)
GO(XkbGetIndicatorState, iFXup)
//GO(XkbGetKeyActions, iFpuup)
//GO(XkbGetKeyBehaviors, iFpuup)
//GOSM(XkbGetKeyboard, bXWWCCpbWWppppCCCCCCCCCCCCCCCCp_bCCbpCCppLbL__WWbL_pp_bLpppppppppppppppppppppppppppppppp_bLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLppbL_LCCW_bbLCCCCp_ppppWW_bLWWpbup_bup_WWWWWWWWWWWWbpp_bup_bLWWbWWWp_bWWWp_bWWWp_p_bLCwwWWwWWWWWWbwwWWipp_bppppp_pp_bppppp_p__FEXuu)
//GOSM(XkbGetKeyboardByName, bXWWCCpbWWppppCCCCCCCCCCCCCCCCp_bCCbpCCppLbL__WWbL_pp_bLpppppppppppppppppppppppppppppppp_bLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLppbL_LCCW_bbLCCCCp_ppppWW_bLWWpbup_bup_WWWWWWWWWWWWbpp_bup_bLWWbWWWp_bWWWp_bWWWp_p_bLCwwWWwWWWWWWbwwWWipp_bppppp_pp_bppppp_p__FEXubpppppp_uui)
//GO(XkbGetKeyExplicitComponents, iFpuup)
//GO(XkbGetKeyModifierMap, iFpuup)
//GO(XkbGetKeySyms, iFpuup)
//GO(XkbGetKeyTypes, iFpuup)
//GO(XkbGetKeyVirtualModMap, iFpuup)
GOM(XkbGetMap, pFEXuu)
//GO(XkbGetMapChanges, iFppp)
GO(XkbGetNamedDeviceIndicator, iFXuuuLpppp)
//GO(XkbGetNamedGeometry, iFppL)
GO(XkbGetNamedIndicator, iFXLpppp)
GOM(XkbGetNames, iFEXup)
GO(XkbGetPerClientControls, iFXp)
//GO(_XkbGetReadBufferCountedString, 
//GO(_XkbGetReadBufferPtr, 
GO(XkbGetState, iFXup)
GOM(XkbGetUpdatedMap, iFEXup)
//GO(XkbGetVirtualMods, iFpup)
GO(XkbGetXlibControls, uFX)
GO(XkbIgnoreExtension, iFi)
//GO(XkbInitCanonicalKeyTypes, iFpui)
//GO(_XkbInitReadBuffer, 
//DATA(_XkbInternAtomFunc, 4)
GO(XkbKeycodeToKeysym, LFXCii)
GO(XkbKeysymToModifiers, uFXL)
//GO(XkbKeyTypesForCoreSymbols, iFpipupp)
GO(XkbLatchGroup, iFXuu)
GO(XkbLatchModifiers, iFXuuu)
GO(XkbLibraryVersion, iFpp)
//GO(XkbListComponents, biiiiiibWp_bWp_bWp_bWp_bWp_bWp__FXubpppppp_p)
GO(XkbLockGroup, iFXuu)
GO(XkbLockModifiers, iFXuuu)
GO(XkbLookupKeyBinding, iFXLupip)
GO(XkbLookupKeySym, iFXCupbL_)
//GO(XkbNoteControlsChanges, vFpbiLiXLiiuuuiCccc_u)
//GO(_XkbNoteCoreMapChanges, 
//GOM(XkbNoteDeviceChanges, vFEpbiLiXLiiuuuiiuuii_u)
//GO(XkbNoteMapChanges, vFpbiLiXLiiuuiiCCCCCCCCiiiiiiu_u)
//GO(XkbNoteNameChanges, vFbuCCCCCCCCWLC_biLiXLiiuiiiiiiuuuii_u)
GO(XkbOpenDisplay, XFpppppp)
//GO(_XkbPeekAtReadBuffer, 
GOM(XkbQueryExtension, iFEXppppp)
//GO(_XkbReadBufferCopy32, 
//GO(_XkbReadBufferCopyKeySyms, 
//GO(_XkbReadCopyData32, 
//GO(_XkbReadCopyKeySyms, 
//GO(_XkbReadGetCompatMapReply, 
//GO(_XkbReadGetGeometryReply, 
//GO(_XkbReadGetIndicatorMapReply, 
//GO(_XkbReadGetMapReply, 
//GO(_XkbReadGetNamesReply, 
GOM(XkbRefreshKeyboardMapping, iEEp)
//GO(_XkbReloadDpy, 
//GO(XkbResizeDeviceButtonActions, iFpu)
//GO(XkbResizeKeyActions, pFpii)
//GO(XkbResizeKeySyms, pFpii)
//GO(XkbResizeKeyType, iFpiiii)
GO(XkbSelectEventDetails, iFXuuLL)
GO(XkbSelectEvents, iFXuuu)
//GOM(XkbSetAtomFuncs, vFEpp)
GO(XkbSetAutoRepeatRate, iFXuuu)
GO(XkbSetAutoResetControls, iFXupp)
//GO(XkbSetCompatMap, iFpupi)
//GO(XkbSetControls, iFpLp)
GO(XkbSetDebuggingFlags, iFXuupuupp)
GO(XkbSetDetectableAutoRepeat, iFXip)
//GO(XkbSetDeviceButtonActions, iFppuu)
//GO(XkbSetDeviceInfo, iFpup)
//GO(XkbSetDeviceLedInfo, iFppuuu)
//GO(XkbSetGeometry, iFpup)
GO(XkbSetIgnoreLockMods, iFXuuuuu)
//GO(XkbSetIndicatorMap, iFpLp)
//GO(XkbSetMap, iFpup)
GO(XkbSetNamedDeviceIndicator, iFXuuuLiiip)
GO(XkbSetNamedIndicator, iFXLiiip)
//GO(XkbSetNames, iFpuuup)
GO(XkbSetPerClientControls, iFXup)
GO(XkbSetServerInternalMods, iFXuuuuu)
GO(XkbSetXlibControls, uFXuu)
//GO(_XkbSkipReadBufferData, 
GO(XkbToControl, cFc)
//GO(XkbTranslateKey, 
//GO(XkbTranslateKeyCode, iFpCupp)
GO(XkbTranslateKeySym, iFXbL_upip)
//GO(XkbUpdateActionVirtualMods, iFppu)
//GO(XkbUpdateKeyTypeVirtualMods, vFppup)
//GO(XkbUpdateMapFromCore, iFpCiipp)
GO(XkbUseExtension, iFXpp)
//GO(XkbVirtualModsToReal, iFpup)
//GO(_XkbWriteCopyData32, 
//GO(_XkbWriteCopyKeySyms, 
GO(XkbXlibControlsImplemented, uFv)
//GO(_XKeycodeToKeysym, 
GO(XKeycodeToKeysym, LFXCi)
//GO(_XKeyInitialize, 
//GO(_XKeysymToKeycode, 
GO(XKeysymToKeycode, CFXL)
//GO(_XKeysymToModifiers, 
GO(XKeysymToString, pFL)
//DATA(_XkeyTable, 
GO(XKillClient, iFXL)
GO(XLastKnownRequestProcessed, LFX)
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
DATA(_XlcGenericMethods, 4)
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
DATA(_XlcPublicMethods, 4)
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
GO(XListDepths, pFXip)
GOM(XListExtensions, pFEXp)//GOM(XListExtensions, bp_FEXp)
GOM(XListFonts, pFEXpip)
//GO(XListFontsWithInfo, pFppipp)
//GO(XListHosts, biip_FXpp)
//GO(XListInstalledColormaps, bL_FXLp)
GO(XListPixmapFormats, pFXp)
//GO(XListProperties, bL_FXLp)
GO(XLoadFont, LFXp)
GOM(XLoadQueryFont, pFEXp)
GO(xlocaledir, vFpi)
GOM(XLocaleOfFontSet, pFEp)
GO(XLocaleOfIM, pFp)
GO(XLocaleOfOM, pFp)
GOM(XLockDisplay, vFEX)
//DATAM(_XLockMutex_fn, 4)
GO(XLookupColor, iFXLpBLWWWcc_BLWWWcc_)
//GO(_XLookupKeysym, 
GOM(XLookupKeysym, LFEpi)
//GO(_XLookupString, 
GOM(XLookupString, iFEppipp)
GO(XLowerWindow, iFXL)
GO(XMapRaised, iFXL)
GO(XMapSubwindows, iFXL)
GO(XMapWindow, iFXL)
GOM(XMaskEvent, iFEXlp)
GO(XMatchVisualInfo, iFXiiiBpLiiiLLLii_)
//GOM(XMaxCmapsOfScreen, iFEbpXLiiiiipippLLLiiiil_)
GO(XMaxRequestSize, lFX)
//GO(_XmbDefaultDrawImageString, 
//GO(_XmbDefaultDrawString, 
//GO(_XmbDefaultTextEscapement, 
//GO(_XmbDefaultTextExtents, 
//GO(_XmbDefaultTextPerCharExtents, 
GOM(XmbDrawImageString, vFEXLppiipi)
GOM(XmbDrawString, vFEXLppiipi)
GOM(XmbDrawText, vFEXLpiipi)
//GO(_XmbGenericDrawImageString, 
//GO(_XmbGenericDrawString, 
//GO(_XmbGenericTextEscapement, 
//GO(_XmbGenericTextExtents, 
//GO(_XmbGenericTextPerCharExtents, 
GO(_Xmblen, iFpi)
GOM(XmbLookupString, iFEpppipp)
GO(XmbResetIC, pFp)
GOM(XmbSetWMProperties, vFEXLpppippp)
//GO(_Xmbstoutf8, 
//GO(_Xmbstowcs, 
GOM(XmbTextEscapement, iFEppi)
GOM(XmbTextExtents, iFEppipp)
//GO(_XmbTextListToTextProperty, 
GOM(XmbTextListToTextProperty, iFEXpiup)
GOM(XmbTextPerCharExtents, iFEppippippp)
//GO(_XmbTextPropertyToTextList, 
GOM(XmbTextPropertyToTextList, iFEXbpLiL_pp)
GO(_Xmbtowc, iFppi)
//GOM(XMinCmapsOfScreen, iFEbpXLiiiiipippLLLiiiil_)
GO(XMoveResizeWindow, iFXLiiuu)
GO(XMoveWindow, iFXLii)
//GO(XNewModifiermap, bip_Fi)
GOM(XNextEvent, iFEXp) // Warning: failed to confirm
GO(XNextRequest, LFX)
GO(XNoOp, iFX)
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
GOM(XOpenDisplay, XFp)  //%noE
GO(XOpenIM, pFXppp)
//GO(_XOpenLC, 
GO(XOpenOM, pFXppp)
//GO(_XParseBaseFontNameList, 
GO(XParseColor, iFXLpbLWWWcc_)
GO(XParseGeometry, iFppppp)
GOM(XPeekEvent, iFEXp) // Warning: failed to confirm
GOM(XPeekIfEvent, iFEpppp)
GO(XPending, iFX)
GO(Xpermalloc, pFu)
//GOM(XPlanesOfScreen, iFEbpXLiiiiipippLLLiiiil_)
GO(XPointInRegion, iFpii)
GO(_XPollfdCacheAdd, vFXi)
GO(_XPollfdCacheDel, vFXi)
GO(_XPollfdCacheInit, iFX)
GO(XPolygonRegion, pFpii)
//GOM(_XProcessInternalConnection, vFEXp)
GO(XProcessInternalConnection, vFXi)
//GO(_XProcessWindowAttributes, vFppLp)
GO(XProtocolRevision, iFX)
GO(XProtocolVersion, iFX)
//GO(_XPutBackEvent, iFpp)
GOM(XPutBackEvent, iFEXp)
GOM(XPutImage, iFEXLppiiiiuu)
GO(XPutPixel, vFXiiL)
GO(XQLength, iFX)
GO(XQueryBestCursor, iFXLuupp)
GO(XQueryBestSize, iFXiLuupp)
GO(XQueryBestStipple, iFXLuupp)
GO(XQueryBestTile, iFXLuupp)
GO(XQueryColor, iFXLbLWWWcc_)
GOM(XQueryColors, iFEXLpi)
GOM(XQueryExtension, iFEXpppp)
GOM(XQueryFont, pFEXL)
GO(XQueryKeymap, iFXp)
GO(XQueryPointer, iFXLbL_bL_ppppp)
GO(XQueryTextExtents, iFXLpipppp)
GO(XQueryTextExtents16, iFXLpipppp)
GOM(XQueryTree, iFEXLpppp)
GO(XRaiseWindow, iFXL)
GO(_XRead, iFXpl)
GO(XReadBitmapFile, iFXLpppbL_pp)
GO(XReadBitmapFileData, iFpppbp_pp)
GO(_XReadEvents, vFX)
GO(_XReadPad, vFXpl)
GO(XRebindKeysym, iFXLbL_ipi)
GO(XRecolorCursor, iFXLbLWWWcc_bLWWWcc_)
GO(XReconfigureWMWindow, iFXLiuriiiiiLi_)
GO(XRectInRegion, iFpiiuu)
//GO(_XRefreshKeyboardMapping, 
GOM(XRefreshKeyboardMapping, iFEp)
//GO(_XRegisterFilterByMask, 
//GO(_XRegisterFilterByType, 
GOM(XRegisterIMInstantiateCallback, iFEXppppp)
//GOM(_XRegisterInternalConnection, iFEXipp)
//GOM(XRemoveConnectionWatch, vFEXpp)
GO(XRemoveFromSaveSet, iFXL)
GO(XRemoveHost, iFXbiip_)
GO(XRemoveHosts, iFXbiip_i)
GO(XReparentWindow, iFXLLii)
GOM(_XReply, iFEXpii)
GO(XResetScreenSaver, iFX)
GO(XResizeWindow, iFXLuu)
GO(XResourceManagerString, pFX)
GO(XRestackWindows, iFXbL_i)
//GO(_XReverse_Bytes, 
GO(XrmCombineDatabase, vFpbp_i)
GO(XrmCombineFileDatabase, iFpbp_i)
//GO(_XrmDefaultInitParseInfo, 
GO(XrmDestroyDatabase, vFp)
//GOM(XrmEnumerateDatabase, iFEpppipp)
GO(XrmGetDatabase, pFX)
GO(XrmGetFileDatabase, pFp)
GO(XrmGetResource, iFpppbp_bup_)
GO(XrmGetStringDatabase, pFp)
GO(XrmInitialize, vFv)
//GO(_XrmInitParseInfo, 
//GO(_XrmInternalStringToQuark, 
GO(XrmLocaleOfDatabase, pFp)
GO(XrmMergeDatabases, vFpbp_)
GO(XrmParseCommand, vFbp_bppup_ippbp_)
GO(XrmPermStringToQuark, iFp)
GO(XrmPutFileDatabase, vFpp)
GO(XrmPutLineResource, vFbp_p)
GO(XrmPutResource, vFbp_ppbup_)
GO(XrmPutStringResource, vFbp_pp)
GO(XrmQGetResource, iFppppbup_)
//GO(XrmQGetSearchList, iFpppbbp__i)
//GO(XrmQGetSearchResource, iFbbp__iipbup_)
GO(XrmQPutResource, vFbp_ppibup_)
GO(XrmQPutStringResource, vFbp_ppp)
GO(XrmQuarkToString, pFi)
GO(XrmSetDatabase, vFXp)
GO(XrmStringToBindingQuarkList, vFppp)
GO(XrmStringToQuark, iFp)
GO(XrmStringToQuarkList, vFpp)
GO(XrmUniqueQuark, iFv)
GO(XRootWindow, LFXi)
//GOM(XRootWindowOfScreen, LFEbpXLiiiiipippLLLiiiil_)
GO(XRotateBuffers, iFXi)
GO(XRotateWindowProperties, iFXLbL_ii)
GO(XSaveContext, iFXLip)
GO(XScreenCount, iFX)
GOM(XScreenNumberOfScreen, iFEp)
//GOM(XScreenOfDisplay, bpXLiiiiipippLLLiiiil_FEXi)
//GOM(_XScreenOfWindow, bpXLiiiiipippLLLiiiil_FEXL)
//GOM(XScreenResourceString, pFEbpXLiiiiipippLLLiiiil_)
GO(XSelectInput, iFXLl)
GO(_XSend, vFXpl)
GOM(XSendEvent, iFEXLilp)
GO(XServerVendor, pFX)
GO(XSetAccessControl, iFXi)
//GOM(XSetAfterFunction, pFEXp)
GO(XSetArcMode, iFXpi)
GO(XSetAuthorization, vFpipi)
GO(XSetBackground, iFXpL)
GO(XSetClassHint, iFXLbpp_)
GO(XSetClipMask, iFXpL)
GO(XSetClipOrigin, iFXpii)
GO(_XSetClipRectangles, vFXpiipii)
GO(XSetClipRectangles, iFXpiipii)
GO(XSetCloseDownMode, iFXi)
GO(XSetCommand, iFXLbp_i)
GO(XSetDashes, iFXpipi)
GOM(XSetErrorHandler, pFEp)
GO(XSetFillRule, iFXpi)
GO(XSetFillStyle, iFXpi)
GO(XSetFont, iFXpL)
GO(XSetFontPath, iFXbp_i)
GO(XSetForeground, iFXpL)
GO(XSetFunction, iFXpi)
GO(XSetGraphicsExposures, iFXpi)
GO(XSetICFocus, vFp)
GO(XSetIconName, iFXLp)
GO(XSetIconSizes, iFXLpi)
GOM(XSetICValues, pFEpV)
//GO(_XSetImage, 
GOM(XSetIMValues, pFEpV)
GO(XSetInputFocus, iFXLiL)
//GOM(XSetIOErrorExitHandler, vFEXpp)
GOM(XSetIOErrorHandler, pFEp)
GO(_XSetLastRequestRead, LFXp)
GO(XSetLineAttributes, iFXpuiii)
GO(XSetLocaleModifiers, pFp)
GO(XSetModifierMapping, iFXbip_)
//GO(XSetNormalHints, iFpLp)
//GOM(XSetOCValues, pFEpV)
//GOM(XSetOMValues, pFEpV)
GO(XSetPlaneMask, iFXpL)
GO(XSetPointerMapping, iFXpi)
GO(XSetRegion, iFXpp)
GO(XSetRGBColormaps, vFXLbLLLLLLLLLL_iL)
GO(XSetScreenSaver, iFXiiii)
GO(XSetSelectionOwner, iFXLLL)
//GO(XSetSizeHints, iFpLpL)
GO(XSetStandardColormap, vFXLbLLLLLLLLLL_L)
GO(XSetStandardProperties, iFXLppLpip)
GO(XSetState, iFXpLLiL)
GO(XSetStipple, iFXpL)
GO(XSetSubwindowMode, iFXpi)
GO(XSetTextProperty, vFXLbpLiL_L)
GO(XSetTile, iFXpL)
GO(XSetTransientForHint, iFXLL)
GO(XSetTSOrigin, iFXpii)
GO(XSetWindowBackground, iFXLL)
GO(XSetWindowBackgroundPixmap, iFXLL)
GO(XSetWindowBorder, iFXLL)
GO(XSetWindowBorderPixmap, iFXLL)
GO(XSetWindowBorderWidth, iFXLu)
GO(XSetWindowColormap, iFXLL)
GO(XSetWMClientMachine, vFXLbpLiL_)
GO(XSetWMColormapWindows, iFXLbL_i)
GOM(XSetWMHints, iFEXLp)
GO(XSetWMIconName, vFXLrpLiL_)
GO(XSetWMName, vFXLrpLiL_)
GOM(XSetWMNormalHints, vFEXLp)
GOM(XSetWMProperties, vFEXLpppippp)
GOM(XSetWMProtocols, iFEXLpi)
GOM(XSetWMSizeHints, vFEXLpL)
//GO(XSetZoomHints, iFpLp)
GO(XShrinkRegion, iFpii)
GO(XStoreBuffer, iFXpii)
GO(XStoreBytes, iFXpi)
GO(XStoreColor, iFXLbLWWWcc_)
GOM(XStoreColors, iFEXLpi)
//GO(_XStoreEventCookie, vFpp)
GO(XStoreName, iFXLp)
GO(XStoreNamedColor, iFXLpLi)
GOM(XStringListToTextProperty, iFEpip)
GO(XStringToKeysym, LFp)
GOM(XSubImage, pFEpiiuu)    // need unbridging  
GO(dummy_XSubImage, pFpiiuu)    // for the wrapper
GO(XSubtractRegion, iFppp)
GO(XSupportsLocale, iFv)
GO(XSync, iFXi)
GOM(XSynchronize, pFEXi)
GOM(XTextExtents, iFEppipppp)
GOM(XTextExtents16, iFEppipppp)
//GO(_XTextHeight, iFppi)
//GO(_XTextHeight16, iFppi)
//GO(XTextPropertyToStringList, iFbpLiL_bbp__p)
GOM(XTextWidth, iFEppi)
GOM(XTextWidth16, iFEppi)
//DATAB(_Xthread_self_fn, 4)
GO(XTranslateCoordinates, iFXLLiippBL_)
//GO(_XTranslateKey, 
//GO(_XTranslateKeySym, 
//GO(_XTryShapeBitmapCursor, 
GO(XUndefineCursor, iFXL)
GO(XUngrabButton, iFXuuL)
GO(XUngrabKey, iFXiuL)
GO(XUngrabKeyboard, iFXL)
GO(XUngrabPointer, iFXL)
GO(XUngrabServer, iFX)
GO(XUninstallColormap, iFXL)
GO(XUnionRectWithRegion, iFppp)
GO(XUnionRegion, iFppp)
//GO(_XUnknownCopyEventCookie, iFXbiLiXiiup_biLiXiiup_)
//GO(_XUnknownNativeEvent, iFppp)
//GO(_XUnknownWireEvent, iFppp)
//GO(_XUnknownWireEventCookie, iFppp)
GO(XUnloadFont, iFXL)
GO(XUnlockDisplay, vFX)
DATA(_XUnlockMutex_fn, 4)
GO(XUnmapSubwindows, iFXL)
GO(XUnmapWindow, iFXL)
//GO(_XUnregisterFilter, 
GOM(XUnregisterIMInstantiateCallback, iFEXppppp)
GO(_XUnregisterInternalConnection, vFXi)
//GO(_XUnresolveColor, 
GO(XUnsetICFocus, vFp)
//GO(_XUpdateAtomCache, 
//GO(_XUpdateGCCache, 
//GO(_Xutf8DefaultDrawImageString, 
//GO(_Xutf8DefaultDrawString, 
//GO(_Xutf8DefaultTextEscapement, 
//GO(_Xutf8DefaultTextExtents, 
//GO(_Xutf8DefaultTextPerCharExtents, 
GOM(Xutf8DrawImageString, vFEXLppiipi)
GOM(Xutf8DrawString, vFEXLppiipi)
GOM(Xutf8DrawText, vFEXLpiipi)
//GO(_Xutf8GenericDrawImageString, 
//GO(_Xutf8GenericDrawString, 
//GO(_Xutf8GenericTextEscapement, 
//GO(_Xutf8GenericTextExtents, 
//GO(_Xutf8GenericTextPerCharExtents, 
GOM(Xutf8LookupString, iFEpppipp)
GO(Xutf8ResetIC, pFp)
GOM(Xutf8SetWMProperties, vFEXLpppippp) // Warning: failed to confirm
GOM(Xutf8TextEscapement, iFEppi)
GOM(Xutf8TextExtents, iFEppipp)
//GO(_Xutf8TextListToTextProperty, 
GOM(Xutf8TextListToTextProperty, iFEXpiup)
GOM(Xutf8TextPerCharExtents, iFEppippippp)
//GO(_Xutf8TextPropertyToTextList, 
GOM(Xutf8TextPropertyToTextList, iFEXbpLiL_pp)
GOM(XVaCreateNestedList, pFEiV)
GO(XVendorRelease, iFX)
GO(_XVIDtoVisual, pFXL)
GOM(XVisualIDFromVisual, LFEp)
GO(XWarpPointer, iFXLLiiuuii)
//GO(_XwcDefaultDrawImageString, 
//GO(_XwcDefaultDrawString, 
//GO(_XwcDefaultTextEscapement, 
//GO(_XwcDefaultTextExtents, 
//GO(_XwcDefaultTextPerCharExtents, 
GOM(XwcDrawImageString, vFEXLppiipi)
GOM(XwcDrawString, vFEXLppiipi)
GOM(XwcDrawText, vFEXLpiipi)
//GO(_XwcFreeStringList, 
GO(XwcFreeStringList, vFbp_)
//GO(_XwcGenericDrawImageString, 
//GO(_XwcGenericDrawString, 
//GO(_XwcGenericTextEscapement, 
//GO(_XwcGenericTextExtents, 
//GO(_XwcGenericTextPerCharExtents, 
GOM(XwcLookupString, iFEpppipp)
GO(XwcResetIC, pFp)
//GO(_Xwcscmp, 
//GO(_Xwcscpy, 
//GO(_Xwcslen, 
//GO(_Xwcsncmp, 
//GO(_Xwcsncpy, 
//GO(_Xwcstombs, 
GOM(XwcTextEscapement, iFEppi)
GOM(XwcTextExtents, iFEppipp)
//GO(_XwcTextListToTextProperty, 
GOM(XwcTextListToTextProperty, iFEXpiup)
GOM(XwcTextPerCharExtents, iFEppippippp)
//GO(_XwcTextPropertyToTextList, 
GOM(XwcTextPropertyToTextList, iFEXbpLiL_pp)
GO(_Xwctomb, iFpi)
GO(XWhitePixel, LFXi)
//GOM(XWhitePixelOfScreen, LFEbpXLiiiiipippLLLiiiil_)
//GOM(XWidthMMOfScreen, iFEbpXLiiiiipippLLLiiiil_)
//GOM(XWidthOfScreen, iFEbpXLiiiiipippLLLiiiil_)
GOM(XWindowEvent, iFEXLlp)
//GO(_XWireToEvent, iFppp)
GO(XWithdrawWindow, iFXLi)
//GO(XWMGeometry, iFpippupppppp)
GO(XWriteBitmapFile, iFXpLuuii)
GO(XXorRegion, iFppp)

//GO(_XData32, iFppu)
//GO(_XRead32, vFppl)

GO(dummy_putpixel, iFpiiL)
GO(dummy_addpixel, iFpl)
GO(dummy_vFp, vFp)
