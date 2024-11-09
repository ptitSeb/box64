#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

//GO(XawAddPixmapLoader, 
GO(XawAsciiSave, iFp)
GO(XawAsciiSaveAsFile, iFpp)
GO(XawAsciiSourceChanged, iFp)
GO(XawAsciiSourceFreeString, vFp)
//GO(_Xaw_atowc, 
//GO(XawBooleanExpression, 
//GO(XawCallProcAction, 
//GO(XawCreateDisplayList, 
//GO(XawCreateDisplayListClass, 
//GO(XawDeclareAction, 
//GO(XawDeclareDisplayListProc, 
//DATA(_XawDefaultTextTranslations, 
//GO(XawDestroyDisplayList, 
//GOM(XawDialogAddButton, vFEpppp)
GO(XawDialogGetValueString, pFp)
//GO(XawDisplayListInitialize, 
//GO(XawDisplayListString, 
//GO(XawFindArgVal, 
//DATAB(XawFmt8Bit, 8)
//DATAB(XawFmtWide, 8)
GO(XawFormDoLayout, vFpc)
//GO(XawFreeParamsStruct, 
//GO(XawGetActionResList, 
//GO(XawGetActionVarList, 
//GO(XawGetDisplayListClass, 
//GO(_XawGetPageSize, 
//GO(XawGetValuesAction, 
GO(_XawImCallVendorShellExtResize, vFp)
GO(_XawImDestroy, vFpp)
GO(_XawImGetImAreaHeight, iFp)
GO(_XawImGetShellHeight, WFp)
GO(_XawImInitialize, vFpp)
GO(_XawImRealize, vFp)
GO(_XawImReconnect, vFp)
GO(_XawImRegister, vFp)
GO(_XawImResizeVendorShell, vFp)
GO(_XawImSetFocusValues, vFppu)
GO(_XawImSetValues, vFppu)
GO(_XawImUnregister, vFp)
GO(_XawImUnsetFocus, vFp)
GO(_XawImWcLookupString, iFpppip)
GO(XawInitializeDefaultConverters, vFv)
GO(XawInitializeWidgetSet, vFv)
//GO(_Xaw_iswalnum, 
GO(XawListChange, vFppiic)
GO(XawListHighlight, vFpi)
GO(XawListShowCurrent, pFp)
GO(XawListUnhighlight, vFp)
//GO(XawLoadPixmap, 
GO(_XawLookupString, iFpppip)
GO(_XawMultiSave, iFp)
GO(_XawMultiSaveAsFile, iFpp)
GO(_XawMultiSinkPosToXY, vFplpp)
GO(_XawMultiSourceFreeString, vFp)
//GOM(XawOpenApplication, pFEpppppppp)
GO(XawPanedAllowResize, vFpc)
GO(XawPanedGetMinMax, vFppp)
GO(XawPanedGetNumSub, iFp)
GO(XawPanedSetMinMax, vFpii)
GO(XawPanedSetRefigureMode, vFpc)
//GO(XawParseBoolean, 
//GO(XawParseParamsString, 
//GO(XawPixmapFromXPixmap, 
//GO(XawPixmapsInitialize, 
//GO(XawPrintActionErrorMsg, 
//GO(XawReshapeWidget, 
//GO(XawRunDisplayList, 
GO(XawScrollbarSetThumb, vFpff)
//GO(XawSetValuesAction, 
GO(XawSimpleMenuAddGlobalActions, vFp)
GO(XawSimpleMenuClearActiveEntry, vFp)
GO(XawSimpleMenuGetActiveEntry, pFp)
//GO(_XawSourceAddText, 
//GO(_XawSourceRemoveText, 
//GO(_XawSourceSetUndoErase, 
//GO(_XawSourceSetUndoMerge, 
//DATA(_XawTextActionsTable, 
//DATA(_XawTextActionsTableCount, 4)
//GO(_XawTextAlterSelection, 
//GOM(_XawTextBuildLineTable, vFEplc)
//GO(_XawTextCheckResize, 
//GO(_XawTextClearAndCenterDisplay, 
GO(XawTextDisableRedisplay, vFp)
GO(XawTextDisplay, vFp)
GO(XawTextDisplayCaret, vFpc)
//GO(_XawTextDoReplaceAction, 
//GO(_XawTextDoSearchAction, 
GO(XawTextEnableRedisplay, vFp)
//GO(_XawTextExecuteUpdate, 
//GOM(_XawTextFormat, iFEp)
GO(XawTextGetInsertionPoint, lFp)
GO(XawTextGetSelectionPos, vFppp)
GO(XawTextGetSink, pFp)
GO(XawTextGetSource, pFp)
//GOM(_XawTextGetSTRING, pFEpll)
//GO(_XawTextGetText, 
//GO(_XawTextInsertFile, 
//GO(_XawTextInsertFileAction, 
GO(XawTextInvalidate, vFpll)
//DATAB(xaw_text_kill_ring, 8)
GO(XawTextLastPosition, lFp)
GO(_XawTextMBToWC, pFppp)
//GOM(_XawTextNeedsUpdating, vFEpll)
//GO(_XawTextPopdownSearchAction, 
GO(_XawTextPosToXY, vFplpp)
//GO(_XawTextPrepareToUpdate, 
//GO(_XawTextReplace, 
GO(XawTextReplace, iFpllp)
//GOM(_XawTextSaltAwaySelection, vFEppi)
//GO(XawTextScroll, 
//GO(_XawTextSearch, 
GO(XawTextSearch, lFpup)
//GO(_XawTextSelectionList, 
//GO(_XawTextSetField, 
GO(XawTextSetInsertionPoint, vFpl)
//GO(_XawTextSetLineAndColumnNumber, 
//GO(_XawTextSetScrollBars, 
//GO(_XawTextSetSelection, 
GO(XawTextSetSelection, vFpll)
GO(XawTextSetSelectionArray, vFpp)
//GO(_XawTextSetSource, 
GO(XawTextSetSource, vFppl)
//GO(_XawTextShowPosition, 
//GOM(XawTextSinkAddProperty, pFEpp)
GO(XawTextSinkBeginPaint, iFp)
//GO(_XawTextSinkClearToBackground, 
GO(XawTextSinkClearToBackground, vFpwwWW)
//GOM(XawTextSinkCombineProperty, pFEpppi)
//GOM(XawTextSinkConvertPropertyList, pFEpppLi)
//GOM(XawTextSinkCopyProperty, pFEpi)
//GO(_XawTextSinkDisplayText, 
GO(XawTextSinkDisplayText, vFpwwllc)
GO(XawTextSinkDoPaint, vFp)
GO(XawTextSinkEndPaint, iFp)
GO(XawTextSinkFindDistance, vFplilppp)
GO(XawTextSinkFindPosition, vFpliicppp)
GO(XawTextSinkGetCursorBounds, vFpp)
//GOM(XawTextSinkGetProperty, pFEpi)
GO(XawTextSinkInsertCursor, vFpwwu)
GO(XawTextSinkMaxHeight, iFpi)
GO(XawTextSinkMaxLines, iFpW)
GO(XawTextSinkPreparePaint, vFpiilli)
GO(XawTextSinkResolve, vFpliip)
GO(XawTextSinkSetTabs, vFpip)
GO(XawTextSourceAddAnchor, pFpl)
GO(XawTextSourceAddEntity, pFpiiplui)
GO(XawTextSourceAnchorAndEntity, iFplpp)
//GO(_XawTextSourceChanged, 
GO(XawTextSourceClearEntities, vFpll)
GO(XawTextSourceConvertSelection, cFppppppp)
//GO(_XawTextSourceFindAnchor, 
GO(XawTextSourceFindAnchor, pFpl)
//GO(_XawTextSourceNewLineAtEOF, 
GO(XawTextSourceNextAnchor, pFpp)
GO(XawTextSourcePrevAnchor, pFpp)
GO(XawTextSourceRead, lFplpi)
GO(XawTextSourceRemoveAnchor, pFpp)
GO(XawTextSourceReplace, iFpllp)
GO(XawTextSourceScan, lFpluuic)
GO(XawTextSourceSearch, lFplup)
GO(XawTextSourceSetSelection, vFpllL)
//GO(_XawTextSrcToggleUndo, 
//GO(_XawTextSrcUndo, 
GO(XawTextTopPosition, lFp)
GO(XawTextUnsetSelection, vFp)
//GO(_XawTextVScroll, 
GO(_XawTextWCToMB, pFppp)
//GO(_XawTextZapSelection, 
GO(XawTipDisable, vFp)
GO(XawTipEnable, vFp)
GO(XawToggleChangeRadioGroup, vFpp)
GO(XawToggleGetCurrent, pFp)
GO(XawToggleSetCurrent, vFpp)
GO(XawToggleUnsetCurrent, vFp)
GO(XawTreeForceLayout, vFp)
//GO(XawTypeToStringWarning, 
//DATA(xawvendorShellExtClassRec, 
//GO(XawVendorShellExtResize, 
//DATA(xawvendorShellExtWidgetClass, 
//GO(XawVendorStructureNotifyHandler, 
GO(XawViewportSetCoordinates, vFpww)
GO(XawViewportSetLocation, vFpff)
//DATA(XawWidgetArray, 
//DATA(XawWidgetCount, 4)
