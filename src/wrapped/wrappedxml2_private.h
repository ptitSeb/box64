#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

//GO(docbCreateFileParserCtxt, 
//GO(docbCreatePushParserCtxt, 
//GO(__docbDefaultSAXHandler, 
GO(docbDefaultSAXHandlerInit, vFv)
//GO(docbEncodeEntities, 
//GO(docbFreeParserCtxt, 
//GO(docbParseChunk, 
//GO(docbParseDoc, 
//GO(docbParseDocument, 
//GO(docbParseFile, 
//GO(docbSAXParseDoc, 
//GO(docbSAXParseFile, 
//GO(_fini, 
//GO(htmlAttrAllowed, 
//GO(htmlAutoCloseTag, 
GO(htmlCreateFileParserCtxt, pFpp)
//GO(htmlCreateMemoryParserCtxt, 
//GO(htmlCreatePushParserCtxt, 
//GO(htmlCtxtReadDoc, 
//GO(htmlCtxtReadFd, 
//GO(htmlCtxtReadFile, 
//GO(htmlCtxtReadIO, 
//GO(htmlCtxtReadMemory, 
//GO(htmlCtxtReset, 
//GO(htmlCtxtUseOptions, 
//GO(__htmlDefaultSAXHandler, 
GO(htmlDefaultSAXHandlerInit, vFv)
GO(htmlDocContentDumpFormatOutput, vFpppi)
GO(htmlDocContentDumpOutput, vFppp)
GO(htmlDocDump, iFpp)
GO(htmlDocDumpMemory, vFppp)
GO(htmlDocDumpMemoryFormat, vFpppi)
//GO(htmlElementAllowedHere, 
//GO(htmlElementStatusHere, 
//GO(htmlEncodeEntities, 
//GO(htmlEntityLookup, 
//GO(htmlEntityValueLookup, 
//GO(htmlFreeParserCtxt, 
GO(htmlGetMetaEncoding, pFp)
//GO(htmlHandleOmittedElem, 
GO(htmlInitAutoClose, vFv)
//GO(htmlIsAutoClosed, 
GO(htmlIsBooleanAttr, iFp)
//GO(htmlIsScriptAttribute, 
GO(htmlNewDoc, pFpp)
GO(htmlNewDocNoDtD, pFpp)
//GO(htmlNewParserCtxt, 
GO(htmlNodeDump, iFppp)
GO(htmlNodeDumpFile, vFppp)
GO(htmlNodeDumpFileFormat, iFppppi)
GO(htmlNodeDumpFormatOutput, vFppppi)
GO(htmlNodeDumpOutput, vFpppp)
//GO(htmlNodeStatus, 
//GO(htmlParseCharRef, 
//GO(htmlParseChunk, 
//GO(__htmlParseContent, 
//GO(htmlParseDoc, 
//GO(htmlParseDocument, 
//GO(htmlParseElement, 
//GO(htmlParseEntityRef, 
//GO(htmlParseFile, 
//GO(htmlReadDoc, 
//GO(htmlReadFd, 
//GO(htmlReadFile, 
//GO(htmlReadIO, 
//GO(htmlReadMemory, 
GO(htmlSaveFile, iFpp)
GO(htmlSaveFileEnc, iFppp)
GO(htmlSaveFileFormat, uFpppi)
//GO(htmlSAXParseDoc, 
//GO(htmlSAXParseFile, 
GO(htmlSetMetaEncoding, iFpp)
//GO(htmlTagLookup, 
//GO(_init, 
//GO(initGenericErrorDefaultFunc, 
GO(inputPop, pFp)
GO(inputPush, iFpp)
GO(isolat1ToUTF8, iFpppp)
GO(namePop, pFp)
GO(namePush, iFpp)
GO(nodePop, pFp)
GO(nodePush, iFpp)
//GO(__oldXMLWDcompatibility, 
//GO(UTF8ToHtml, 
GO(UTF8Toisolat1, iFpppp)
GO(valuePop, pFp)
GO(valuePush, iFpp)
//GO(xlinkGetDefaultDetect, 
//GO(xlinkGetDefaultHandler, 
//GO(xlinkIsLink, 
//GO(xlinkSetDefaultDetect, 
//GO(xlinkSetDefaultHandler, 
//GO(xmlACatalogAdd, 
//GO(xmlACatalogDump, 
//GO(xmlACatalogRemove, 
//GO(xmlACatalogResolve, 
//GO(xmlACatalogResolvePublic, 
//GO(xmlACatalogResolveSystem, 
//GO(xmlACatalogResolveURI, 
GO(xmlAddAttributeDecl, pFpppppiipp)
GO(xmlAddChild, pFpp)
GO(xmlAddChildList, pFpp)
GO(xmlAddDocEntity, pFppippp)
GO(xmlAddDtdEntity, pFppippp)
GO(xmlAddElementDecl, pFpppip)
GO(xmlAddEncodingAlias, iFpp)
GO(xmlAddID, pFpppp)
GO(xmlAddNextSibling, pFpp)
GO(xmlAddNotationDecl, pFppppp)
GO(xmlAddPrevSibling, pFpp)
GO(xmlAddRef, pFpppp)
//GO(xmlAddSibling, 
GO(xmlAllocOutputBuffer, pFp)
GO(xmlAllocOutputBufferInternal, pFp)
GO(xmlAllocParserInputBuffer, pFi)
GO(xmlAttrSerializeTxtContent, vFpppp)
//GO(xmlAutomataCompile, 
//GO(xmlAutomataGetInitState, 
//GO(xmlAutomataIsDeterminist, 
//GO(xmlAutomataNewAllTrans, 
//GO(xmlAutomataNewCountedTrans, 
//GO(xmlAutomataNewCounter, 
//GO(xmlAutomataNewCounterTrans, 
//GO(xmlAutomataNewCountTrans, 
//GO(xmlAutomataNewCountTrans2, 
//GO(xmlAutomataNewEpsilon, 
//GO(xmlAutomataNewNegTrans, 
//GO(xmlAutomataNewOnceTrans, 
//GO(xmlAutomataNewOnceTrans2, 
//GO(xmlAutomataNewState, 
//GO(xmlAutomataNewTransition, 
//GO(xmlAutomataNewTransition2, 
//GO(xmlAutomataSetFinalState, 
//GO(xmlAutomataSetFlags, 
GO(xmlBufContent, pFp)
GO(xmlBufEnd, pFp)
GO(xmlBufGetNodeContent, iFpp)
GO(xmlBufNodeDump, LFpppii)
GO(xmlBufShrink, LFpL)
GO(xmlBufUse, LFp)
GO(xmlBufferAdd, iFppi)
GO(xmlBufferAddHead, iFppi)
//GO(__xmlBufferAllocScheme, 
GO(xmlBufferCat, iFpp)
GO(xmlBufferCCat, iFpp)
GO(xmlBufferContent, pFp)
GO(xmlBufferCreate, pFv)
GO(xmlBufferCreateSize, pFL)
GO(xmlBufferCreateStatic, pFpp)
GO(xmlBufferDump, iFpp)
GO(xmlBufferEmpty, vFp)
GO(xmlBufferFree, vFp)
GO(xmlBufferGrow, iFpu)
GO(xmlBufferLength, iFp)
GO(xmlBufferResize, iFpu)
GO(xmlBufferSetAllocationScheme, vFpi)
GO(xmlBufferShrink, vFpu)
GO(xmlBufferWriteChar, vFpp)
GO(xmlBufferWriteCHAR, vFpp)
GO(xmlBufferWriteQuotedString, vFpp)
GO(xmlBuildQName, pFpppi)
GO(xmlBuildRelativeURI, pFpp)
GO(xmlBuildURI, pFpp)
GO(xmlByteConsumed, lFp)
//GO(xmlC14NDocDumpMemory, 
//GO(xmlC14NDocSave, 
//GO(xmlC14NDocSaveTo, 
//GO(xmlC14NExecute, 
//GO(xmlCanonicPath, 
//GO(xmlCatalogAdd, 
//GO(xmlCatalogAddLocal, 
//GO(xmlCatalogCleanup, 
//GO(xmlCatalogConvert, 
//GO(xmlCatalogDump, 
//GO(xmlCatalogFreeLocal, 
//GO(xmlCatalogGetDefaults, 
//GO(xmlCatalogGetPublic, 
//GO(xmlCatalogGetSystem, 
//GO(xmlCatalogIsEmpty, 
//GO(xmlCatalogLocalResolve, 
//GO(xmlCatalogLocalResolveURI, 
//GO(xmlCatalogRemove, 
//GO(xmlCatalogResolve, 
//GO(xmlCatalogResolvePublic, 
//GO(xmlCatalogResolveSystem, 
//GO(xmlCatalogResolveURI, 
//GO(xmlCatalogSetDebug, 
//GO(xmlCatalogSetDefaultPrefer, 
GO(xmlCatalogSetDefaults, vFi)
GO(xmlCharEncCloseFunc, iFp)
GO(xmlCharEncFirstLine, iFppp)
//GO(xmlCharEncFirstLineInt, 
GO(xmlCharEncInFunc, iFppp)
GO(xmlCharEncOutFunc, iFppp)
//GO(xmlCharInRange, 
GO(xmlCharStrdup, pFp)
GO(xmlCharStrndup, pFpi)
GO(xmlCheckFilename, iFp)
GO(xmlCheckHTTPInput, pFpp)
GO(xmlCheckLanguageID, iFp)
GO(xmlCheckUTF8, iFp)
//GO(xmlCheckVersion, 
GO(xmlChildElementCount, LFp)
GO(xmlCleanupCharEncodingHandlers, vFv)
GO(xmlCleanupEncodingAliases, vFv)
GO(xmlCleanupGlobals, vFv)
GO(xmlCleanupInputCallbacks, vFv)
//GO(xmlCleanupMemory, 
GO(xmlCleanupOutputCallbacks, vFv)
GO(xmlCleanupParser, vFv)
GO(xmlCleanupPredefinedEntities, vFv)
//GO(xmlCleanupThreads, 
GO(xmlClearNodeInfoSeq, vFp)
GO(xmlClearParserCtxt, vFp)
//GO(xmlConvertSGMLCatalog, 
GO(xmlCopyAttributeTable, pFp)
GO(xmlCopyChar, iFipi)
GO(xmlCopyCharMultiByte, iFpi)
GO(xmlCopyDoc, pFpi)
GO(xmlCopyDocElementContent, pFpp)
GO(xmlCopyDtd, pFp)
GO(xmlCopyElementContent, pFp)
GO(xmlCopyElementTable, pFp)
GO(xmlCopyEntitiesTable, pFp)
GO(xmlCopyEnumeration, pFp)
//GO(xmlCopyError, 
GO(xmlCopyNamespace, pFp)
GO(xmlCopyNamespaceList, pFp)
GO(xmlCopyNode, pFpi)
GO(xmlCopyNodeList, pFp)
GO(xmlCopyNotationTable, pFp)
GO(xmlCopyProp, pFpp)
GO(xmlCopyPropList, pFpp)
GO(xmlCreateDocParserCtxt, pFp)
GO(xmlCreateEntitiesTable, pFv)
GO(xmlCreateEntityParserCtxt, pFppp)
GO(xmlCreateEnumeration, pFp)
GO(xmlCreateFileParserCtxt, pFp)
GO(xmlCreateIntSubset, pFpppp)
GOM(xmlCreateIOParserCtxt, pFEpppppi)
GO(xmlCreateMemoryParserCtxt, pFpi)
GO(xmlCreatePushParserCtxt, pFpppip)
//GO(xmlCreateURI, 
GO(xmlCreateURLParserCtxt, pFpi)
GO(xmlCtxtGetLastError, pFp)
GO(xmlCtxtReadDoc, pFppppi)
GO(xmlCtxtReadFd, pFpippi)
GO(xmlCtxtReadFile, pFpppi)
GOM(xmlCtxtReadIO, pFEppppppi)
GO(xmlCtxtReadMemory, pFppippi)
GO(xmlCtxtReset, vFp)
//GO(xmlCtxtResetLastError, 
GO(xmlCtxtResetPush, iFppipp)
GO(xmlCtxtUseOptions, iFpi)
GO(xmlCurrentChar, iFpp)
GO(xmlDecodeEntities, pFpiiCCC) // deprecated, not always present
//GO(__xmlDefaultBufferSize, 
//GO(__xmlDefaultSAXHandler, 
GO(xmlDefaultSAXHandlerInit, vFv)
//GO(__xmlDefaultSAXLocator, 
GO(xmlDelEncodingAlias, iFp)
//GOM(xmlDeregisterNodeDefault, BFEB)
//GO(__xmlDeregisterNodeDefaultValue, 
GO(xmlDetectCharEncoding, iFpi)
GO(xmlDictCleanup, vFv)
GO(xmlDictCreate, pFv)
GO(xmlDictCreateSub, pFp)
GO(xmlDictExists, pFppi)
GO(xmlDictFree, vFp)
GO(xmlDictGetUsage, LFp)
GO(xmlDictLookup, pFppi)
GO(xmlDictOwns, iFpp)
GO(xmlDictQLookup, pFppp)
GO(xmlDictReference, iFp)
GO(xmlDictSetLimit, LFpL)
GO(xmlDictSize, iFp)
GO(xmlDocCopyNode, pFppi)
GO(xmlDocCopyNodeList, pFpp)
GO(xmlDocDump, iFpp)
GO(xmlDocDumpFormatMemory, vFpppi)
GO(xmlDocDumpFormatMemoryEnc, vFppppi)
GO(xmlDocDumpMemory, vFppp)
GO(xmlDocDumpMemoryEnc, vFpppp)
GO(xmlDocFormatDump, iFppi)
GO(xmlDocGetRootElement, pFp)
GO(xmlDocSetRootElement, pFpp)
GO(xmlDOMWrapAdoptNode, iFpppppi)
GO(xmlDOMWrapCloneNode, iFppppppii)
GO(xmlDOMWrapFreeCtxt, vFp)
GO(xmlDOMWrapNewCtxt, pFv)
GO(xmlDOMWrapReconcileNamespaces, iFppi)
GO(xmlDOMWrapRemoveNode, iFppi)
//GO(__xmlDoValidityCheckingDefaultValue, 
GO(xmlDumpAttributeDecl, vFpp)
GO(xmlDumpAttributeTable, vFpp)
GO(xmlDumpElementDecl, vFpp)
GO(xmlDumpElementTable, vFpp)
GO(xmlDumpEntitiesTable, vFpp)
GO(xmlDumpEntityDecl, vFpp)
GO(xmlDumpNotationDecl, vFpp)
GO(xmlDumpNotationTable, vFpp)
GO(xmlElemDump, vFppp)
GO(xmlEncodeEntities, pFpp)
GO(xmlEncodeEntitiesReentrant, pFpp)
GO(xmlEncodeSpecialChars, pFpp)
//GO(__xmlErrEncoding, 
GO(xmlErrMemory, vFpp)
//GO(xmlExpCtxtNbCons, 
//GO(xmlExpCtxtNbNodes, 
//GO(xmlExpDump, 
//GO(xmlExpExpDerive, 
//GO(xmlExpFree, 
//GO(xmlExpFreeCtxt, 
//GO(xmlExpGetLanguage, 
//GO(xmlExpGetStart, 
//GO(xmlExpIsNillable, 
//GO(xmlExpMaxToken, 
//GO(xmlExpNewAtom, 
//GO(xmlExpNewCtxt, 
//GO(xmlExpNewOr, 
//GO(xmlExpNewRange, 
//GO(xmlExpNewSeq, 
//GO(xmlExpParse, 
//GO(xmlExpRef, 
//GO(xmlExpStringDerive, 
//GO(xmlExpSubsume, 
GO(xmlFileClose, iFp)
GO(xmlFileMatch, iFp)
GO(xmlFileOpen, pFp)
GO(xmlFileRead, iFppi)
GO(xmlFindCharEncodingHandler, pFp)
GO(xmlFirstElementChild, pFp)
GO(xmlFreeAttributeTable, vFp)
//GO(xmlFreeAutomata, 
//GO(xmlFreeCatalog, 
GO(xmlFreeDoc, vFp)
GO(xmlFreeDocElementContent, vFpp)
GO(xmlFreeDtd, vFp)
GO(xmlFreeElementContent, vFp)
GO(xmlFreeElementTable, vFp)
GO(xmlFreeEntitiesTable, vFp)
GO(xmlFreeEnumeration, vFp)
GO(xmlFreeIDTable, vFp)
GO(xmlFreeInputStream, vFp)
//GO(xmlFreeMutex, 
GO(xmlFreeNode, vFp)
GO(xmlFreeNodeList, vFp)
GO(xmlFreeNotationTable, vFp)
GO(xmlFreeNs, vFp)
GO(xmlFreeNsList, vFp)
GO(xmlFreeParserCtxt, vFp)
GO(xmlFreeParserInputBuffer, vFp)
//GO(xmlFreePattern, 
//GO(xmlFreePatternList, 
GO(xmlFreeProp, vFp)
GO(xmlFreePropList, vFp)
GO(xmlFreeRefTable, vFp)
//GO(xmlFreeRMutex, 
//GO(xmlFreeStreamCtxt, 
GO(xmlFreeTextReader, vFp)
GO(xmlFreeTextWriter, vFp)
//GO(xmlFreeURI, 
GO(xmlFreeValidCtxt, vFp)
//GO(xmlGcMemGet, 
//GO(xmlGcMemSetup, 
GOM(__xmlGenericError, pFEv)
GO(__xmlGenericErrorContext, pFv)
//GO(xmlGenericErrorDefaultFunc, 
GO(xmlGetBufferAllocationScheme, iFv)
GO(xmlGetCharEncodingHandler, pFp)
GO(xmlGetCharEncodingName, pFi)
GO(xmlGetCompressMode, iFv)
GO(xmlGetDocCompressMode, iFv)
GO(xmlGetDocEntity, pFpp)
GO(xmlGetDtdAttrDesc, pFppp)
GO(xmlGetDtdElementDesc, pFpp)
GO(xmlGetDtdEntity, pFpp)
GO(xmlGetDtdNotationDesc, pFpp)
GO(xmlGetDtdQAttrDesc, pFpppp)
GO(xmlGetDtdQElementDesc, pFppp)
GO(xmlGetEncodingAlias, pFp)
GOM(xmlGetExternalEntityLoader, pFEv)
GO(xmlGetFeature, iFppp)
GO(xmlGetFeaturesList, iFpp)
//GO(xmlGetGlobalState, 
GO(xmlGetID, pFpp)
GO(xmlGetIntSubset, pFp)
GO(xmlGetLastChild, pFp)
//GO(xmlGetLastError, 
GO(xmlGetLineNo, lFp)
GO(xmlGetNodePath, pFp)
GO(xmlGetNoNsProp, pFpp)
GO(xmlGetNsList, pFpp)
GO(xmlGetNsProp, pFppp)
GO(xmlGetParameterEntity, pFpp)
GO(xmlGetPredefinedEntity, pFp)
GO(xmlGetProp, pFpp)
GO(xmlGetRefs, pFpp)
//GO(xmlGetThreadId, 
GO(xmlGetUTF8Char, iFpp)
//GO(__xmlGetWarningsDefaultValue, 
//GO(__xmlGlobalInitMutexDestroy, 
//GO(__xmlGlobalInitMutexLock, 
//GO(__xmlGlobalInitMutexUnlock, 
GO(xmlHandleEntity, vFpp)   // obsolete, might be removed
GO(xmlHashAddEntry, iFppp)
GO(xmlHashAddEntry2, iFpppp)
GO(xmlHashAddEntry3, iFppppp)
GOM(xmlHashCopy, pFEpp)
GO(xmlHashCreate, pFi)
GO(xmlHashCreateDict, pFip)
GOM(xmlHashFree, vFEpp)
GO(xmlHashLookup, pFpp)
GO(xmlHashLookup2, pFppp)
GO(xmlHashLookup3, pFpppp)
GO(xmlHashQLookup, pFppp)
GO(xmlHashQLookup2, pFppppp)
GO(xmlHashQLookup3, pFppppppp)
GOM(xmlHashRemoveEntry, iFEppp)
GOM(xmlHashRemoveEntry2, iFEpppp)
GOM(xmlHashRemoveEntry3, iFEppppp)
GOM(xmlHashScan, vFEppp)
GOM(xmlHashScan3, vFEpppppp)
GOM(xmlHashScanFull, vFEppp)
GOM(xmlHashScanFull3, vFEpppppp)
GO(xmlHashSize, iFp)
GOM(xmlHashUpdateEntry, iFEpppp)
GOM(xmlHashUpdateEntry2, iFEppppp)
GOM(xmlHashUpdateEntry3, iFEpppppp)
GO(xmlHasFeature, iFi)
GO(xmlHasNsProp, pFppp)
GO(xmlHasProp, pFpp)
GO(__xmlIndentTreeOutput, pFv)
GO(xmlInitCharEncodingHandlers, vFv)
GO(xmlInitializeDict, iFv)
GO(xmlInitGlobals, vFv)
//GO(xmlInitializeCatalog, 
GO(xmlInitializeGlobalState, vFp)
//GO(xmlInitMemory, 
GO(xmlInitializePredefinedEntities, vFv)
GO(xmlInitNodeInfoSeq, vFp)
GO(xmlInitParser, vFv)
GO(xmlInitParserCtxt, iFp)
//GO(xmlInitThreads, 
//GO(__xmlIOErr, 
GO(xmlIOFTPClose, iFp)
GO(xmlIOFTPMatch, iFp)
GO(xmlIOFTPOpen, pFp)
GO(xmlIOFTPRead, iFppi)
GO(xmlIOHTTPClose, iFp)
GO(xmlIOHTTPMatch, iFp)
GO(xmlIOHTTPOpen, pFp)
GO(xmlIOHTTPOpenW, pFpi)
GO(xmlIOHTTPRead, iFppi)
GO(xmlIOParseDTD, pFppp)
//GO(xmlIsBaseChar, 
//GO(xmlIsBlank, 
GO(xmlIsBlankNode, iFp)
//GO(xmlIsChar, 
//GO(xmlIsCombining, 
//GO(xmlIsDigit, 
//GO(xmlIsExtender, 
GO(xmlIsID, iFppp)
//GO(xmlIsIdeographic, 
GO(xmlIsLetter, iFi)
//GO(xmlIsMainThread, 
GO(xmlIsMixedElement, iFpp)
//GO(xmlIsPubidChar, 
GO(xmlIsRef, iFppp)
GO(xmlIsXHTML, iFpp)
GO(xmlKeepBlanksDefault, iFi)
//GO(__xmlKeepBlanksDefaultValue, 
GO(xmlLastElementChild, pFp)
//GO(__xmlLastError, 
GO(xmlLineNumbersDefault, iFi)
//GO(__xmlLineNumbersDefaultValue, 
//GO(xmlLinkGetData, 
//GO(xmlListAppend, 
//GO(xmlListClear, 
//GO(xmlListCopy, 
//GO(xmlListCreate, 
//GO(xmlListDelete, 
//GO(xmlListDup, 
//GO(xmlListEmpty, 
//GO(xmlListEnd, 
//GO(xmlListFront, 
//GO(xmlListInsert, 
//GO(xmlListMerge, 
//GO(xmlListPopBack, 
//GO(xmlListPopFront, 
//GO(xmlListPushBack, 
//GO(xmlListPushFront, 
//GO(xmlListRemoveAll, 
//GO(xmlListRemoveFirst, 
//GO(xmlListRemoveLast, 
//GO(xmlListReverse, 
//GO(xmlListReverseSearch, 
//GO(xmlListReverseWalk, 
//GO(xmlListSearch, 
//GO(xmlListSize, 
//GO(xmlListSort, 
//GO(xmlListWalk, 
//GO(xmlLoadACatalog, 
//GO(xmlLoadCatalog, 
//GO(xmlLoadCatalogs, 
//GO(__xmlLoaderErr, 
//GO(__xmlLoadExtDtdDefaultValue, 
GO(xmlLoadExternalEntity, pFppp)
//GO(xmlLoadSGMLSuperCatalog, 
//GO(xmlLockLibrary, 
//GO(xmlMallocAtomicLoc, 
//GO(xmlMallocBreakpoint, 
//GO(xmlMallocLoc, 
//GO(xmlMemBlocks, 
//GO(xmlMemDisplay, 
//GO(xmlMemDisplayLast, 
//GO(xmlMemFree, 
//GO(xmlMemGet, 
//GO(xmlMemMalloc, 
//GO(xmlMemoryDump, 
//GO(xmlMemoryStrdup, 
//GO(xmlMemRealloc, 
//GO(xmlMemSetup, 
//GO(xmlMemShow, 
//GO(xmlMemStrdupLoc, 
//GO(xmlMemUsed, 
//GO(xmlModuleClose, 
//GO(xmlModuleFree, 
//GO(xmlModuleOpen, 
//GO(xmlModuleSymbol, 
//GO(xmlMutexLock, 
//GO(xmlMutexUnlock, 
GO(xmlNamespaceParseNCName, pFp)
GO(xmlNamespaceParseNSDef, pFp)
GO(xmlNamespaceParseQName, pFpp)
//GO(xmlNanoFTPCheckResponse, 
//GO(xmlNanoFTPCleanup, 
//GO(xmlNanoFTPClose, 
//GO(xmlNanoFTPCloseConnection, 
//GO(xmlNanoFTPConnect, 
//GO(xmlNanoFTPConnectTo, 
//GO(xmlNanoFTPCwd, 
//GO(xmlNanoFTPDele, 
//GO(xmlNanoFTPFreeCtxt, 
//GO(xmlNanoFTPGet, 
//GO(xmlNanoFTPGetConnection, 
//GO(xmlNanoFTPGetResponse, 
//GO(xmlNanoFTPGetSocket, 
//GO(xmlNanoFTPInit, 
//GO(xmlNanoFTPList, 
//GO(xmlNanoFTPNewCtxt, 
//GO(xmlNanoFTPOpen, 
//GO(xmlNanoFTPProxy, 
//GO(xmlNanoFTPQuit, 
//GO(xmlNanoFTPRead, 
//GO(xmlNanoFTPScanProxy, 
//GO(xmlNanoFTPUpdateURL, 
//GO(xmlNanoHTTPAuthHeader, 
//GO(xmlNanoHTTPCleanup, 
//GO(xmlNanoHTTPClose, 
//GO(xmlNanoHTTPContentLength, 
//GO(xmlNanoHTTPEncoding, 
//GO(xmlNanoHTTPFetch, 
//GO(xmlNanoHTTPInit, 
//GO(xmlNanoHTTPMethod, 
//GO(xmlNanoHTTPMethodRedir, 
//GO(xmlNanoHTTPMimeType, 
//GO(xmlNanoHTTPOpen, 
//GO(xmlNanoHTTPOpenRedir, 
//GO(xmlNanoHTTPRead, 
//GO(xmlNanoHTTPRedir, 
//GO(xmlNanoHTTPReturnCode, 
//GO(xmlNanoHTTPSave, 
//GO(xmlNanoHTTPScanProxy, 
//GO(xmlNewAutomata, 
//GO(xmlNewCatalog, 
GO(xmlNewCDataBlock, pFppi)
GOM(xmlNewCharEncodingHandler, pFEppp)
GO(xmlNewCharRef, pFpp)
GO(xmlNewChild, pFpppp)
GO(xmlNewComment, pFp)
GO(xmlNewDoc, pFp)
GO(xmlNewDocComment, pFpp)
GO(xmlNewDocElementContent, pFppi)
GO(xmlNewDocFragment, pFp)
GO(xmlNewDocNode, pFpppp)
GO(xmlNewDocNodeEatName, pFpppp)
GO(xmlNewDocPI, pFppp)
GO(xmlNewDocProp, pFppp)
GO(xmlNewDocRawNode, pFpppp)
GO(xmlNewDocText, pFpp)
GO(xmlNewDocTextLen, pFppi)
GO(xmlNewDtd, pFpppp)
GO(xmlNewElementContent, pFpi)
GO(xmlNewEntity, pFppippp)
GO(xmlNewEntityInputStream, pFpp)
GO(xmlNewGlobalNs, pFppp)
GO(xmlNewInputFromFile, pFpp)
GO(xmlNewInputStream, pFp)
GO(xmlNewIOInputStream, pFppi)
//GO(xmlNewMutex, 
GO(xmlNewNode, pFpp)
GO(xmlNewNodeEatName, pFpp)
GO(xmlNewNs, pFppp)
GO(xmlNewNsProp, pFpppp)
GO(xmlNewNsPropEatName, pFpppp)
GO(xmlNewParserCtxt, pFv)
GO(xmlNewPI, pFpp)
GO(xmlNewProp, pFppp)
GO(xmlNewReference, pFpp)
//GO(xmlNewRMutex, 
GO(xmlNewStringInputStream, pFpp)
GO(xmlNewText, pFp)
GO(xmlNewTextChild, pFpppp)
GO(xmlNewTextLen, pFpi)
//GO(xmlNewTextReader, 
//GO(xmlNewTextReaderFilename, 
//GO(xmlNewTextWriter, 
//GO(xmlNewTextWriterDoc, 
//GO(xmlNewTextWriterFilename, 
GO(xmlNewTextWriterMemory, pFpi)
//GO(xmlNewTextWriterPushParser, 
//GO(xmlNewTextWriterTree, 
GO(xmlNewValidCtxt, pFv)
GO(xmlNextChar, vFp)
GO(xmlNextElementSibling, pFp)
GO(xmlNodeAddContent, vFpp)
GO(xmlNodeAddContentLen, vFppi)
GO(xmlNodeBufGetContent, iFpp)
GO(xmlNodeDump, iFpppii)
GO(xmlNodeDumpOutput, vFpppiip)
GO(xmlNodeGetBase, pFpp)
GO(xmlNodeGetContent, pFp)
GO(xmlNodeGetLang, pFp)
GO(xmlNodeGetSpacePreserve, iFp)
GO(xmlNodeIsText, iFp)
GO(xmlNodeListGetRawString, pFppi)
GO(xmlNodeListGetString, pFppi)
GO(xmlNodeSetBase, vFpp)
GO(xmlNodeSetContent, vFpp)
GO(xmlNodeSetContentLen, vFppi)
GO(xmlNodeSetLang, vFpp)
GO(xmlNodeSetName, vFpp)
GO(xmlNodeSetSpacePreserve, vFpi)
GO(xmlNoNetExternalEntityLoader, pFppp)
//GO(xmlNormalizeURIPath, 
GO(xmlNormalizeWindowsPath, pFp)
//GO(xmlNsListDumpOutput, 
GO(xmlOutputBufferClose, iFp)
GO(xmlOutputBufferCreateBuffer, pFpp)
GO(xmlOutputBufferCreateFd, pFip)
GO(xmlOutputBufferCreateFile, pFpp)
GO(xmlOutputBufferCreateFilename, pFppi)
//GO(__xmlOutputBufferCreateFilename, 
//GOM(xmlOutputBufferCreateFilenameDefault, BFEB)
//GO(__xmlOutputBufferCreateFilenameValue, 
GOM(xmlOutputBufferCreateIO, pFEpppp)
GO(xmlOutputBufferFlush, iFp)
GO(xmlOutputBufferGetContent, pFp)
GO(xmlOutputBufferWrite, iFppp)
GO(xmlOutputBufferWriteEscape, iFppp)
GO(xmlOutputBufferWriteString, iFpp)
GO(xmlParseAttribute, pFpp)
GO(xmlParseAttributeListDecl, vFp)
GO(xmlParseAttributeType, iFpp)
GO(xmlParseAttValue, pFp)
GO(xmlParseBalancedChunkMemory, iFpppipp)
GO(xmlParseBalancedChunkMemoryRecover, iFpppippi)
//GO(xmlParseCatalogFile, 
GO(xmlParseCDSect, vFp)
GO(xmlParseCharData, vFpi)
GO(xmlParseCharEncoding, iFp)
GO(xmlParseCharRef, iFp)
GO(xmlParseChunk, iFppii)
GO(xmlParseComment, vFp)
GO(xmlParseContent, vFp)
GO(xmlParseCtxtExternalEntity, iFpppp)
GO(xmlParseDefaultDecl, iFpp)
GO(xmlParseDoc, pFp)
GO(xmlParseDocTypeDecl, iFpp)
GOM(xmlParseDocument, iFEp)
GO(xmlParseDTD, pFpp)
GO(xmlParseElement, vFp)
GO(xmlParseElementChildrenContentDecl, pFpi)
GO(xmlParseElementContentDecl, iFppp)
GO(xmlParseElementDecl, iFp)
GO(xmlParseElementMixedContentDecl, pFpi)
GO(xmlParseEncName, pFp)
GO(xmlParseEncodingDecl, pFp)
GO(xmlParseEndTag, vFp)
GO(xmlParseEntity, pFp)
GO(xmlParseEntityDecl, vFp)
GO(xmlParseEntityRef, pFp)
GO(xmlParseEntityValue, pFpp)
GO(xmlParseEnumeratedType, iFp)
GO(xmlParseEnumerationType, pFp)
GO(xmlParseExternalEntity, iFpppippp)
GO(xmlParseExternalID, pFppi)
GO(xmlParseExternalSubset, vFppp)
//GO(xmlParseExtParsedEnt, 
GO(xmlParseFile, pFp)
GO(xmlParseInNodeContext, pFppiip)
GO(xmlParseMarkupDecl, vFp)
GO(xmlParseMemory, pFpi)
GO(xmlParseMisc, vFp)
GO(xmlParseName, pFp)
GO(xmlParseNmtoken, pFp)
GO(xmlParseNotationDecl, vFp)
GO(xmlParseNotationType, pFp)
GO(xmlParsePEReference, vFp)
GO(xmlParsePI, vFp)
GO(xmlParsePITarget, pFp)
GO(xmlParsePubidLiteral, pFp)
GO(xmlParserAddNodeInfo, vFpp)
//GO(__xmlParserDebugEntities, 
GO(xmlParseQuotedString, pFp)
GO(xmlParseReference, vFp)
//GO(xmlParserError, 
GO(xmlParserFindNodeInfo, pFpp)
GO(xmlParserFindNodeInfoIndex, LFpp)
GO(xmlParserGetDirectory, pFp)
GO(xmlParserHandlePEReference, vFp)
GO(xmlParserHandleReference, vFp)
GO(xmlParserInputBufferCreateFd, pFii)
GO(xmlParserInputBufferCreateFile, pFpi)
GO(xmlParserInputBufferCreateFilename, pFpi)
//GO(__xmlParserInputBufferCreateFilename, 
//GOM(xmlParserInputBufferCreateFilenameDefault, BFEB)
//GO(__xmlParserInputBufferCreateFilenameValue, 
GOM(xmlParserInputBufferCreateIO, pFEpppi)
GO(xmlParserInputBufferCreateMem, pFpii)
GO(xmlParserInputBufferCreateStatic, pFpii)
GO(xmlParserInputBufferGrow, iFpi)
GO(xmlParserInputBufferPush, iFpip)
GO(xmlParserInputBufferRead, iFpi)
GO(xmlParserInputGrow, iFpi)
GO(xmlParserInputRead, iFpi)
GO(xmlParserInputShrink, vFp)
//GO(xmlParserPrintFileContext, 
//GO(xmlParserPrintFileInfo, 
//GO(xmlParserValidityError, 
//GO(xmlParserValidityWarning, 
//GO(__xmlParserVersion, 
//GO(xmlParserWarning, 
GO(xmlParseSDDecl, iFp)
GO(xmlParseStartTag, pFp)
GO(xmlParseSystemLiteral, pFp)
GO(xmlParseTextDecl, vFp)
//GO(xmlParseURI, 
//GO(xmlParseURIRaw, 
//GO(xmlParseURIReference, 
GO(xmlParseVersionInfo, pFp)
GO(xmlParseVersionNum, pFp)
GO(xmlParseXMLDecl, vFp)
//GO(xmlPathToURI, 
//GO(xmlPatterncompile, 
//GO(xmlPatternFromRoot, 
//GO(xmlPatternGetStreamCtxt, 
//GO(xmlPatternMatch, 
//GO(xmlPatternMaxDepth, 
//GO(xmlPatternMinDepth, 
//GO(xmlPatternStreamable, 
GO(xmlPedanticParserDefault, iFi)
//GO(__xmlPedanticParserDefaultValue, 
GO(xmlPopInput, CFp)
GO(xmlPopInputCallbacks, iFv)
GO(xmlPreviousElementSibling, pFp)
//GO(xmlPrintURI, 
GO(xmlPushInput, iFpi)
//GO(__xmlRaiseError, 
GO(xmlReadDoc, pFpppi)
//GO(xmlReaderForDoc, 
//GO(xmlReaderForFd, 
//GO(xmlReaderForFile, 
GOM(xmlReaderForIO, pFEpppppi)
GO(xmlReaderForMemory, pFpippi)
//GO(xmlReaderNewDoc, 
//GO(xmlReaderNewFd, 
//GO(xmlReaderNewFile, 
//GO(xmlReaderNewIO, 
//GO(xmlReaderNewMemory, 
//GO(xmlReaderNewWalker, 
//GO(xmlReaderWalker, 
GO(xmlReadFd, pFippi)
GO(xmlReadFile, pFppi)
//GOM(xmlReadIO, pFEBBpppi)
GO(xmlReadMemory, pFpippi)
//GO(xmlReallocLoc, 
GO(xmlReconciliateNs, iFpp)
GO(xmlRecoverDoc, pFp)
GO(xmlRecoverFile, pFp)
GO(xmlRecoverMemory, pFpi)
//GO(xmlRegExecErrInfo, 
//GO(xmlRegExecNextValues, 
//GO(xmlRegExecPushString, 
//GO(xmlRegExecPushString2, 
//GO(xmlRegexpCompile, 
//GO(xmlRegexpExec, 
//GO(xmlRegexpIsDeterminist, 
//GO(xmlRegexpPrint, 
//GO(xmlRegFreeExecCtxt, 
//GO(xmlRegFreeRegexp, 
GO(xmlRegisterCharEncodingHandler, vFp)
GO(xmlRegisterDefaultInputCallbacks, vFv)
GO(xmlRegisterDefaultOutputCallbacks, vFv)
GO(xmlRegisterHTTPPostCallbacks, vFv)
GOM(xmlRegisterInputCallbacks, iFEpppp)
//GOM(xmlRegisterNodeDefault, BFEB)
//GO(__xmlRegisterNodeDefaultValue, 
GOM(xmlRegisterOutputCallbacks, iFEpppp)
//GO(xmlRegNewExecCtxt, 
//GO(xmlRelaxNGCleanupTypes, 
//GO(xmlRelaxNGDump, 
//GO(xmlRelaxNGDumpTree, 
//GO(xmlRelaxNGFree, 
//GO(xmlRelaxNGFreeParserCtxt, 
//GO(xmlRelaxNGFreeValidCtxt, 
//GO(xmlRelaxNGGetParserErrors, 
//GO(xmlRelaxNGGetValidErrors, 
//GO(xmlRelaxNGInitTypes, 
//GO(xmlRelaxNGNewDocParserCtxt, 
//GO(xmlRelaxNGNewMemParserCtxt, 
//GO(xmlRelaxNGNewParserCtxt, 
//GO(xmlRelaxNGNewValidCtxt, 
//GO(xmlRelaxNGParse, 
//GO(xmlRelaxNGSetParserErrors, 
//GO(xmlRelaxNGSetParserStructuredErrors, 
//GO(xmlRelaxNGSetValidErrors, 
//GO(xmlRelaxNGSetValidStructuredErrors, 
//GO(xmlRelaxNGValidateDoc, 
//GO(xmlRelaxNGValidateFullElement, 
//GO(xmlRelaxNGValidatePopElement, 
//GO(xmlRelaxNGValidatePushCData, 
//GO(xmlRelaxNGValidatePushElement, 
//GO(xmlRelaxParserSetFlag, 
GO(xmlRemoveID, iFpp)
GO(xmlRemoveProp, iFp)
GO(xmlRemoveRef, iFpp)
GO(xmlReplaceNode, pFpp)
//GO(xmlResetError, 
//GO(xmlResetLastError, 
//GO(xmlRMutexLock, 
//GO(xmlRMutexUnlock, 
GO(xmlSaveClose, iFp)
GO(xmlSaveDoc, lFpp)
GO(xmlSaveFile, iFpp)
GO(xmlSaveFileEnc, iFppp)
GO(xmlSaveFileTo, iFppp)
GO(xmlSaveFlush, iFp)
GO(xmlSaveFormatFile, iFppi)
GO(xmlSaveFormatFileEnc, iFpppi)
GO(xmlSaveFormatFileTo, iFpppi)
//GO(__xmlSaveNoEmptyTags, 
//GOM(xmlSaveSetAttrEscape, iFEpB)
//GOM(xmlSaveSetEscape, iFEpB)
GO(xmlSaveToBuffer, pFppi)
GO(xmlSaveToFd, pFipi)
GO(xmlSaveToFilename, pFppi)
GOM(xmlSaveToIO, pFEppppi)
GO(xmlSaveTree, lFpp)
//GO(xmlSaveUri, 
GO(xmlSAX2AttributeDecl, vFpppiipp)
GO(xmlSAX2CDataBlock, vFppi)
GO(xmlSAX2Characters, vFppi)
GO(xmlSAX2Comment, vFpp)
GO(xmlSAX2ElementDecl, vFppip)
GO(xmlSAX2EndDocument, vFp)
GO(xmlSAX2EndElement, vFpp)
GO(xmlSAX2EndElementNs, vFpppp)
GO(xmlSAX2EntityDecl, vFppippp)
GO(xmlSAX2ExternalSubset, vFpppp)
GO(xmlSAX2GetColumnNumber, iFp)
GO(xmlSAX2GetEntity, pFpp)
GO(xmlSAX2GetLineNumber, iFp)
GO(xmlSAX2GetParameterEntity, pFpp)
GO(xmlSAX2GetPublicId, pFp)
GO(xmlSAX2GetSystemId, pFp)
GO(xmlSAX2HasExternalSubset, iFp)
GO(xmlSAX2HasInternalSubset, iFp)
GO(xmlSAX2IgnorableWhitespace, vFppi)
GO(xmlSAX2InitDefaultSAXHandler, vFpi)
GO(xmlSAX2InitDocbDefaultSAXHandler, vFp)
GO(xmlSAX2InitHtmlDefaultSAXHandler, vFp)
GO(xmlSAX2InternalSubset, vFpppp)
GO(xmlSAX2IsStandalone, iFp)
GO(xmlSAX2NotationDecl, vFpppp)
GO(xmlSAX2ProcessingInstruction, vFppp)
GO(xmlSAX2Reference, vFpp)
GO(xmlSAX2ResolveEntity, pFppp)
GO(xmlSAX2SetDocumentLocator, vFpp)
GO(xmlSAX2StartDocument, vFp)
GO(xmlSAX2StartElement, vFppp)
GO(xmlSAX2StartElementNs, vFppppipiip)
GO(xmlSAX2UnparsedEntityDecl, vFppppp)
GO(xmlSAXDefaultVersion, iFi)
GO(xmlSAXParseDoc, pFppi)
GO(xmlSAXParseDTD, pFppp)
GO(xmlSAXParseEntity, pFpp)
GO(xmlSAXParseFile, pFppi)
GO(xmlSAXParseFileWithData, pFppip)
GO(xmlSAXParseMemory, pFppii)
GO(xmlSAXParseMemoryWithData, pFppiip)
GO(xmlSAXUserParseFile, iFppp)
GO(xmlSAXUserParseMemory, iFpppi)
GO(xmlSAXVersion, iFpi)
GO(xmlScanName, pFp)
//GO(xmlSchemaCheckFacet, 
//GO(xmlSchemaCleanupTypes, 
//GO(xmlSchemaCollapseString, 
//GO(xmlSchemaCompareValues, 
//GO(xmlSchemaCompareValuesWhtsp, 
//GO(xmlSchemaCopyValue, 
GO(xmlSchemaDump, vFpp)
GO(xmlSchemaFree, vFp)
//GO(xmlSchemaFreeFacet, 
GO(xmlSchemaFreeParserCtxt, vFp)
//GO(xmlSchemaFreeType, 
GO(xmlSchemaFreeValidCtxt, vFp)
//GO(xmlSchemaFreeValue, 
//GO(xmlSchemaFreeWildcard, 
//GO(xmlSchemaGetBuiltInListSimpleTypeItemType, 
//GO(xmlSchemaGetBuiltInType, 
//GO(xmlSchemaGetCanonValue, 
//GO(xmlSchemaGetCanonValueWhtsp, 
//GO(xmlSchemaGetFacetValueAsULong, 
//GOM(xmlSchemaGetParserErrors, iFEpBBp)
//GO(xmlSchemaGetPredefinedType, 
//GOM(xmlSchemaGetValidErrors, iFEpBBp)
//GO(xmlSchemaGetValType, 
//GO(xmlSchemaInitTypes, 
//GO(xmlSchemaIsBuiltInTypeFacet, 
GO(xmlSchemaIsValid, iFp)
GO(xmlSchemaNewDocParserCtxt, pFp)
//GO(xmlSchemaNewFacet, 
GO(xmlSchemaNewMemParserCtxt, pFpi)
//GO(xmlSchemaNewNOTATIONValue, 
GO(xmlSchemaNewParserCtxt, pFp)
//GO(xmlSchemaNewQNameValue, 
//GO(xmlSchemaNewStringValue, 
GO(xmlSchemaNewValidCtxt, pFp)
GO(xmlSchemaParse, pFp)
GO(xmlSchemaSAXPlug, pFppp)
GO(xmlSchemaSAXUnplug, iFp)
GOM(xmlSchemaSetParserErrors, vFEpppp)
GOM(xmlSchemaSetParserStructuredErrors, vFEppp)
GOM(xmlSchemaSetValidErrors, vFEpppp)
GO(xmlSchemaSetValidOptions, iFpi)
GOM(xmlSchemaSetValidStructuredErrors, vFEppp)
//GO(xmlSchematronFree, 
//GO(xmlSchematronFreeParserCtxt, 
//GO(xmlSchematronFreeValidCtxt, 
//GO(xmlSchematronNewDocParserCtxt, 
//GO(xmlSchematronNewMemParserCtxt, 
//GO(xmlSchematronNewParserCtxt, 
//GO(xmlSchematronNewValidCtxt, 
//GO(xmlSchematronParse, 
//GO(xmlSchematronSetValidStructuredErrors, 
//GO(xmlSchematronValidateDoc, 
GO(xmlSchemaValidateDoc, iFpp)
//GO(xmlSchemaValidateFacet, 
//GO(xmlSchemaValidateFacetWhtsp, 
GO(xmlSchemaValidateFile, iFppi)
//GO(xmlSchemaValidateLengthFacet, 
//GO(xmlSchemaValidateLengthFacetWhtsp, 
//GO(xmlSchemaValidateListSimpleTypeFacet, 
GO(xmlSchemaValidateOneElement, iFpp)
//GO(xmlSchemaValidatePredefinedType, 
GO(xmlSchemaValidateSetFilename, iFpp)
//GOM(xmlSchemaValidateSetLocator, iFEpBp)
GO(xmlSchemaValidateStream, iFppipp)
GO(xmlSchemaValidCtxtGetOptions, iFp)
GO(xmlSchemaValidCtxtGetParserCtxt, pFp)
//GO(xmlSchemaValPredefTypeNode, 
//GO(xmlSchemaValPredefTypeNodeNoNorm, 
//GO(xmlSchemaValueAppend, 
//GO(xmlSchemaValueGetAsBoolean, 
//GO(xmlSchemaValueGetAsString, 
//GO(xmlSchemaValueGetNext, 
//GO(xmlSchemaWhiteSpaceReplace, 
GO(xmlSearchNs, pFppp)
GO(xmlSearchNsByHref, pFppp)
GO(xmlSetBufferAllocationScheme, vFi)
GO(xmlSetCompressMode, vFi)
GO(xmlSetDocCompressMode, vFpi)
GOM(xmlSetExternalEntityLoader, vFEp)
GO(xmlSetFeature, iFppp)
GOM(xmlSetGenericErrorFunc, vFEpp)
GO(xmlSetListDoc, vFpp)
GO(xmlSetNs, vFpp)
GO(xmlSetNsProp, pFpppp)
GO(xmlSetProp, pFppp)
GOM(xmlSetStructuredErrorFunc, vFEpp)
GO(xmlSetTreeDoc, vFpp)
GO(xmlSetupParserForBuffer, vFppp)
//GO(__xmlSimpleError, 
GO(xmlSkipBlankChars, iFp)
GO(xmlSnprintfElementContent, vFpipi)
GO(xmlSplitQName, pFppp)
GO(xmlSplitQName2, pFpp)
GO(xmlSplitQName3, pFpp)
GO(xmlSprintfElementContent, vFppi)
GO(xmlStopParser, vFp)
GO(xmlStrcasecmp, iFpp)
GO(xmlStrcasestr, pFpp)
GO(xmlStrcat, pFpp)
GO(xmlStrchr, pFpC)
GO(xmlStrcmp, iFpp)
GO(xmlStrdup, pFp)
//GO(xmlStreamPop, 
//GO(xmlStreamPush, 
//GO(xmlStreamPushAttr, 
//GO(xmlStreamPushNode, 
//GO(xmlStreamWantsAnyNode, 
GO(xmlStrEqual, iFpp)
GO(xmlStringCurrentChar, iFppp)
GO(xmlStringDecodeEntities, pFppiCCC)
GO(xmlStringGetNodeList, pFpp)
GO(xmlStringLenDecodeEntities, pFppiiCCC)
GO(xmlStringLenGetNodeList, pFppi)
GO(xmlStrlen, iFp)
GO(xmlStrncasecmp, iFppi)
GO(xmlStrncat, pFppi)
GO(xmlStrncatNew, pFppi)
GO(xmlStrncmp, iFppi)
GO(xmlStrndup, pFpi)
GO2(xmlStrPrintf, iFpipV, xmlStrVPrintf)
GO(xmlStrQEqual, iFppp)
GO(xmlStrstr, pFpp)
GO(xmlStrsub, pFpii)
GOM(__xmlStructuredError, pFEv)
//GO(__xmlStructuredErrorContext, 
GO(xmlStrVPrintf, iFpipp)
GO(xmlSubstituteEntitiesDefault, iFi)
//GO(__xmlSubstituteEntitiesDefaultValue, 
GO(xmlSwitchEncoding, iFpi)
GO(xmlSwitchInputEncoding, iFppp)
GO(xmlSwitchToEncoding, iFpp)
GO(xmlTextConcat, iFppi)
GO(xmlTextMerge, pFpp)
//GO(xmlTextReaderAttributeCount, 
//GO(xmlTextReaderBaseUri, 
//GO(xmlTextReaderByteConsumed, 
//GO(xmlTextReaderClose, 
//GO(xmlTextReaderConstBaseUri, 
//GO(xmlTextReaderConstEncoding, 
GO(xmlTextReaderConstLocalName, pFp)
GO(xmlTextReaderConstName, pFp)
//GO(xmlTextReaderConstNamespaceUri, 
//GO(xmlTextReaderConstPrefix, 
//GO(xmlTextReaderConstString, 
GO(xmlTextReaderConstValue, pFp)
//GO(xmlTextReaderConstXmlLang, 
//GO(xmlTextReaderConstXmlVersion, 
//GO(xmlTextReaderCurrentDoc, 
//GO(xmlTextReaderCurrentNode, 
//GO(xmlTextReaderDepth, 
//GO(xmlTextReaderExpand, 
GO(xmlTextReaderGetAttribute, pFpp)
//GO(xmlTextReaderGetAttributeNo, 
//GO(xmlTextReaderGetAttributeNs, 
//GO(xmlTextReaderGetErrorHandler, 
//GO(xmlTextReaderGetParserColumnNumber, 
//GO(xmlTextReaderGetParserLineNumber, 
//GO(xmlTextReaderGetParserProp, 
//GO(xmlTextReaderGetRemainder, 
GO(xmlTextReaderHasAttributes, iFp)
//GO(xmlTextReaderHasValue, 
//GO(xmlTextReaderIsDefault, 
GO(xmlTextReaderIsEmptyElement, iFp)
GO(xmlTextReaderIsNamespaceDecl, iFp)
//GO(xmlTextReaderIsValid, 
//GO(xmlTextReaderLocalName, 
//GO(xmlTextReaderLocatorBaseURI, 
//GO(xmlTextReaderLocatorLineNumber, 
GO(xmlTextReaderLookupNamespace, pFpp)
//GO(xmlTextReaderMoveToAttribute, 
//GO(xmlTextReaderMoveToAttributeNo, 
//GO(xmlTextReaderMoveToAttributeNs, 
GO(xmlTextReaderMoveToElement, iFp)
GO(xmlTextReaderMoveToFirstAttribute, iFp)
GO(xmlTextReaderMoveToNextAttribute, iFp)
//GO(xmlTextReaderName, 
//GO(xmlTextReaderNamespaceUri, 
//GO(xmlTextReaderNext, 
//GO(xmlTextReaderNextSibling, 
GO(xmlTextReaderNodeType, iFp)
//GO(xmlTextReaderNormalization, 
//GO(xmlTextReaderPrefix, 
//GO(xmlTextReaderPreserve, 
//GO(xmlTextReaderPreservePattern, 
//GO(xmlTextReaderQuoteChar, 
GO(xmlTextReaderRead, iFp)
//GO(xmlTextReaderReadAttributeValue, 
//GO(xmlTextReaderReadInnerXml, 
//GO(xmlTextReaderReadOuterXml, 
//GO(xmlTextReaderReadState, 
//GO(xmlTextReaderReadString, 
//GO(xmlTextReaderRelaxNGSetSchema, 
//GO(xmlTextReaderRelaxNGValidate, 
//GO(xmlTextReaderSchemaValidate, 
//GO(xmlTextReaderSchemaValidateCtxt, 
GOM(xmlTextReaderSetErrorHandler, vFEppp)
//GO(xmlTextReaderSetParserProp, 
//GO(xmlTextReaderSetSchema, 
//GO(xmlTextReaderSetStructuredErrorHandler, 
//GO(xmlTextReaderSetup, 
//GO(xmlTextReaderStandalone, 
//GO(xmlTextReaderValue, 
//GO(xmlTextReaderXmlLang, 
//GO(xmlTextWriterEndAttribute, 
//GO(xmlTextWriterEndCDATA, 
//GO(xmlTextWriterEndComment, 
GO(xmlTextWriterEndDocument, iFp)
//GO(xmlTextWriterEndDTD, 
//GO(xmlTextWriterEndDTDAttlist, 
//GO(xmlTextWriterEndDTDElement, 
//GO(xmlTextWriterEndDTDEntity, 
GO(xmlTextWriterEndElement, iFp)
//GO(xmlTextWriterEndPI, 
//GO(xmlTextWriterFlush, 
//GO(xmlTextWriterFullEndElement, 
GO(xmlTextWriterSetIndent, iFpi)
//GO(xmlTextWriterSetIndentString, 
//GO(xmlTextWriterStartAttribute, 
//GO(xmlTextWriterStartAttributeNS, 
//GO(xmlTextWriterStartCDATA, 
//GO(xmlTextWriterStartComment, 
GO(xmlTextWriterStartDocument, iFpppp)
//GO(xmlTextWriterStartDTD, 
//GO(xmlTextWriterStartDTDAttlist, 
//GO(xmlTextWriterStartDTDElement, 
//GO(xmlTextWriterStartDTDEntity, 
GO(xmlTextWriterStartElement, iFpp)
//GO(xmlTextWriterStartElementNS, 
//GO(xmlTextWriterStartPI, 
GO(xmlTextWriterWriteAttribute, iFppp)
//GO(xmlTextWriterWriteAttributeNS, 
GO(xmlTextWriterWriteBase64, iFppii)
//GO(xmlTextWriterWriteBinHex, 
//GO(xmlTextWriterWriteCDATA, 
//GO(xmlTextWriterWriteComment, 
//GO(xmlTextWriterWriteDTD, 
//GO(xmlTextWriterWriteDTDAttlist, 
//GO(xmlTextWriterWriteDTDElement, 
//GO(xmlTextWriterWriteDTDEntity, 
//GO(xmlTextWriterWriteDTDExternalEntity, 
//GO(xmlTextWriterWriteDTDExternalEntityContents, 
//GO(xmlTextWriterWriteDTDInternalEntity, 
//GO(xmlTextWriterWriteDTDNotation, 
//GO(xmlTextWriterWriteElement, 
//GO(xmlTextWriterWriteElementNS, 
GOM(xmlTextWriterWriteFormatAttribute, iFEpppV)
//GO(xmlTextWriterWriteFormatAttributeNS, 
//GO(xmlTextWriterWriteFormatCDATA, 
//GO(xmlTextWriterWriteFormatComment, 
//GO(xmlTextWriterWriteFormatDTD, 
//GO(xmlTextWriterWriteFormatDTDAttlist, 
//GO(xmlTextWriterWriteFormatDTDElement, 
//GO(xmlTextWriterWriteFormatDTDInternalEntity, 
//GO(xmlTextWriterWriteFormatElement, 
//GO(xmlTextWriterWriteFormatElementNS, 
//GO(xmlTextWriterWriteFormatPI, 
//GO(xmlTextWriterWriteFormatRaw, 
//GO(xmlTextWriterWriteFormatString, 
//GO(xmlTextWriterWritePI, 
//GO(xmlTextWriterWriteRaw, 
//GO(xmlTextWriterWriteRawLen, 
GO(xmlTextWriterWriteString, iFpp)
GOM(xmlTextWriterWriteVFormatAttribute, iFEpppA)
//GO(xmlTextWriterWriteVFormatAttributeNS, 
//GO(xmlTextWriterWriteVFormatCDATA, 
//GO(xmlTextWriterWriteVFormatComment, 
//GO(xmlTextWriterWriteVFormatDTD, 
//GO(xmlTextWriterWriteVFormatDTDAttlist, 
//GO(xmlTextWriterWriteVFormatDTDElement, 
//GO(xmlTextWriterWriteVFormatDTDInternalEntity, 
//GO(xmlTextWriterWriteVFormatElement, 
//GO(xmlTextWriterWriteVFormatElementNS, 
//GO(xmlTextWriterWriteVFormatPI, 
//GO(xmlTextWriterWriteVFormatRaw, 
//GO(xmlTextWriterWriteVFormatString, 
GO(xmlThrDefBufferAllocScheme, iFi)
GO(xmlThrDefDefaultBufferSize, iFi)
//GOM(xmlThrDefDeregisterNodeDefault, BFEB)
GO(xmlThrDefDoValidityCheckingDefaultValue, iFi)
GO(xmlThrDefGetWarningsDefaultValue, iFi)
GO(xmlThrDefIndentTreeOutput, iFi)
GO(xmlThrDefKeepBlanksDefaultValue, iFi)
GO(xmlThrDefLineNumbersDefaultValue, iFi)
GO(xmlThrDefLoadExtDtdDefaultValue, iFi)
//GOM(xmlThrDefOutputBufferCreateFilenameDefault, BFEB)
GO(xmlThrDefParserDebugEntities, iFi)
//GOM(xmlThrDefParserInputBufferCreateFilenameDefault, BFEB)
GO(xmlThrDefPedanticParserDefaultValue, iFi)
//GOM(xmlThrDefRegisterNodeDefault, BFEB)
GO(xmlThrDefSaveNoEmptyTags, iFi)
//GOM(xmlThrDefSetGenericErrorFunc, vFEpB)
//GOM(xmlThrDefSetStructuredErrorFunc, vFEpB)
GO(xmlThrDefSubstituteEntitiesDefaultValue, iFi)
GO(xmlThrDefTreeIndentString, pFp)
GO(__xmlTreeIndentString, pFv)
//GO(xmlUCSIsAegeanNumbers, 
//GO(xmlUCSIsAlphabeticPresentationForms, 
//GO(xmlUCSIsArabic, 
//GO(xmlUCSIsArabicPresentationFormsA, 
//GO(xmlUCSIsArabicPresentationFormsB, 
//GO(xmlUCSIsArmenian, 
//GO(xmlUCSIsArrows, 
//GO(xmlUCSIsBasicLatin, 
//GO(xmlUCSIsBengali, 
//GO(xmlUCSIsBlock, 
//GO(xmlUCSIsBlockElements, 
//GO(xmlUCSIsBopomofo, 
//GO(xmlUCSIsBopomofoExtended, 
//GO(xmlUCSIsBoxDrawing, 
//GO(xmlUCSIsBraillePatterns, 
//GO(xmlUCSIsBuhid, 
//GO(xmlUCSIsByzantineMusicalSymbols, 
//GO(xmlUCSIsCat, 
//GO(xmlUCSIsCatC, 
//GO(xmlUCSIsCatCc, 
//GO(xmlUCSIsCatCf, 
//GO(xmlUCSIsCatCo, 
//GO(xmlUCSIsCatCs, 
//GO(xmlUCSIsCatL, 
//GO(xmlUCSIsCatLl, 
//GO(xmlUCSIsCatLm, 
//GO(xmlUCSIsCatLo, 
//GO(xmlUCSIsCatLt, 
//GO(xmlUCSIsCatLu, 
//GO(xmlUCSIsCatM, 
//GO(xmlUCSIsCatMc, 
//GO(xmlUCSIsCatMe, 
//GO(xmlUCSIsCatMn, 
//GO(xmlUCSIsCatN, 
//GO(xmlUCSIsCatNd, 
//GO(xmlUCSIsCatNl, 
//GO(xmlUCSIsCatNo, 
//GO(xmlUCSIsCatP, 
//GO(xmlUCSIsCatPc, 
//GO(xmlUCSIsCatPd, 
//GO(xmlUCSIsCatPe, 
//GO(xmlUCSIsCatPf, 
//GO(xmlUCSIsCatPi, 
//GO(xmlUCSIsCatPo, 
//GO(xmlUCSIsCatPs, 
//GO(xmlUCSIsCatS, 
//GO(xmlUCSIsCatSc, 
//GO(xmlUCSIsCatSk, 
//GO(xmlUCSIsCatSm, 
//GO(xmlUCSIsCatSo, 
//GO(xmlUCSIsCatZ, 
//GO(xmlUCSIsCatZl, 
//GO(xmlUCSIsCatZp, 
//GO(xmlUCSIsCatZs, 
//GO(xmlUCSIsCherokee, 
//GO(xmlUCSIsCJKCompatibility, 
//GO(xmlUCSIsCJKCompatibilityForms, 
//GO(xmlUCSIsCJKCompatibilityIdeographs, 
//GO(xmlUCSIsCJKCompatibilityIdeographsSupplement, 
//GO(xmlUCSIsCJKRadicalsSupplement, 
//GO(xmlUCSIsCJKSymbolsandPunctuation, 
//GO(xmlUCSIsCJKUnifiedIdeographs, 
//GO(xmlUCSIsCJKUnifiedIdeographsExtensionA, 
//GO(xmlUCSIsCJKUnifiedIdeographsExtensionB, 
//GO(xmlUCSIsCombiningDiacriticalMarks, 
//GO(xmlUCSIsCombiningDiacriticalMarksforSymbols, 
//GO(xmlUCSIsCombiningHalfMarks, 
//GO(xmlUCSIsCombiningMarksforSymbols, 
//GO(xmlUCSIsControlPictures, 
//GO(xmlUCSIsCurrencySymbols, 
//GO(xmlUCSIsCypriotSyllabary, 
//GO(xmlUCSIsCyrillic, 
//GO(xmlUCSIsCyrillicSupplement, 
//GO(xmlUCSIsDeseret, 
//GO(xmlUCSIsDevanagari, 
//GO(xmlUCSIsDingbats, 
//GO(xmlUCSIsEnclosedAlphanumerics, 
//GO(xmlUCSIsEnclosedCJKLettersandMonths, 
//GO(xmlUCSIsEthiopic, 
//GO(xmlUCSIsGeneralPunctuation, 
//GO(xmlUCSIsGeometricShapes, 
//GO(xmlUCSIsGeorgian, 
//GO(xmlUCSIsGothic, 
//GO(xmlUCSIsGreek, 
//GO(xmlUCSIsGreekandCoptic, 
//GO(xmlUCSIsGreekExtended, 
//GO(xmlUCSIsGujarati, 
//GO(xmlUCSIsGurmukhi, 
//GO(xmlUCSIsHalfwidthandFullwidthForms, 
//GO(xmlUCSIsHangulCompatibilityJamo, 
//GO(xmlUCSIsHangulJamo, 
//GO(xmlUCSIsHangulSyllables, 
//GO(xmlUCSIsHanunoo, 
//GO(xmlUCSIsHebrew, 
//GO(xmlUCSIsHighPrivateUseSurrogates, 
//GO(xmlUCSIsHighSurrogates, 
//GO(xmlUCSIsHiragana, 
//GO(xmlUCSIsIdeographicDescriptionCharacters, 
//GO(xmlUCSIsIPAExtensions, 
//GO(xmlUCSIsKanbun, 
//GO(xmlUCSIsKangxiRadicals, 
//GO(xmlUCSIsKannada, 
//GO(xmlUCSIsKatakana, 
//GO(xmlUCSIsKatakanaPhoneticExtensions, 
//GO(xmlUCSIsKhmer, 
//GO(xmlUCSIsKhmerSymbols, 
//GO(xmlUCSIsLao, 
//GO(xmlUCSIsLatin1Supplement, 
//GO(xmlUCSIsLatinExtendedA, 
//GO(xmlUCSIsLatinExtendedAdditional, 
//GO(xmlUCSIsLatinExtendedB, 
//GO(xmlUCSIsLetterlikeSymbols, 
//GO(xmlUCSIsLimbu, 
//GO(xmlUCSIsLinearBIdeograms, 
//GO(xmlUCSIsLinearBSyllabary, 
//GO(xmlUCSIsLowSurrogates, 
//GO(xmlUCSIsMalayalam, 
//GO(xmlUCSIsMathematicalAlphanumericSymbols, 
//GO(xmlUCSIsMathematicalOperators, 
//GO(xmlUCSIsMiscellaneousMathematicalSymbolsA, 
//GO(xmlUCSIsMiscellaneousMathematicalSymbolsB, 
//GO(xmlUCSIsMiscellaneousSymbols, 
//GO(xmlUCSIsMiscellaneousSymbolsandArrows, 
//GO(xmlUCSIsMiscellaneousTechnical, 
//GO(xmlUCSIsMongolian, 
//GO(xmlUCSIsMusicalSymbols, 
//GO(xmlUCSIsMyanmar, 
//GO(xmlUCSIsNumberForms, 
//GO(xmlUCSIsOgham, 
//GO(xmlUCSIsOldItalic, 
//GO(xmlUCSIsOpticalCharacterRecognition, 
//GO(xmlUCSIsOriya, 
//GO(xmlUCSIsOsmanya, 
//GO(xmlUCSIsPhoneticExtensions, 
//GO(xmlUCSIsPrivateUse, 
//GO(xmlUCSIsPrivateUseArea, 
//GO(xmlUCSIsRunic, 
//GO(xmlUCSIsShavian, 
//GO(xmlUCSIsSinhala, 
//GO(xmlUCSIsSmallFormVariants, 
//GO(xmlUCSIsSpacingModifierLetters, 
//GO(xmlUCSIsSpecials, 
//GO(xmlUCSIsSuperscriptsandSubscripts, 
//GO(xmlUCSIsSupplementalArrowsA, 
//GO(xmlUCSIsSupplementalArrowsB, 
//GO(xmlUCSIsSupplementalMathematicalOperators, 
//GO(xmlUCSIsSupplementaryPrivateUseAreaA, 
//GO(xmlUCSIsSupplementaryPrivateUseAreaB, 
//GO(xmlUCSIsSyriac, 
//GO(xmlUCSIsTagalog, 
//GO(xmlUCSIsTagbanwa, 
//GO(xmlUCSIsTags, 
//GO(xmlUCSIsTaiLe, 
//GO(xmlUCSIsTaiXuanJingSymbols, 
//GO(xmlUCSIsTamil, 
//GO(xmlUCSIsTelugu, 
//GO(xmlUCSIsThaana, 
//GO(xmlUCSIsThai, 
//GO(xmlUCSIsTibetan, 
//GO(xmlUCSIsUgaritic, 
//GO(xmlUCSIsUnifiedCanadianAboriginalSyllabics, 
//GO(xmlUCSIsVariationSelectors, 
//GO(xmlUCSIsVariationSelectorsSupplement, 
//GO(xmlUCSIsYijingHexagramSymbols, 
//GO(xmlUCSIsYiRadicals, 
//GO(xmlUCSIsYiSyllables, 
GO(xmlUnlinkNode, vFp)
//GO(xmlUnlockLibrary, 
GO(xmlUnsetNsProp, iFppp)
GO(xmlUnsetProp, iFpp)
//GO(xmlURIEscape, 
//GO(xmlURIEscapeStr, 
//GO(xmlURIUnescapeString, 
GO(xmlUTF8Charcmp, iFpp)
GO(xmlUTF8Size, iFp)
GO(xmlUTF8Strlen, iFp)
GO(xmlUTF8Strloc, iFpp)
GO(xmlUTF8Strndup, pFpi)
GO(xmlUTF8Strpos, pFpi)
GO(xmlUTF8Strsize, iFpi)
GO(xmlUTF8Strsub, pFpii)
GO(xmlValidateAttributeDecl, iFppp)
GO(xmlValidateAttributeValue, iFpp)
GO(xmlValidateDocument, iFpp)
GO(xmlValidateDocumentFinal, iFpp)
GO(xmlValidateDtd, iFpp)
GO(xmlValidateDtdFinal, iFpp)
GO(xmlValidateElement, iFpp)
GO(xmlValidateElementDecl, iFppp)
GO(xmlValidateName, iFpi)
GO(xmlValidateNamesValue, iFp)
GO(xmlValidateNameValue, iFp)
GO(xmlValidateNCName, iFpi)
GO(xmlValidateNMToken, iFpi)
GO(xmlValidateNmtokensValue, iFp)
GO(xmlValidateNmtokenValue, iFp)
GO(xmlValidateNotationDecl, iFppp)
GO(xmlValidateNotationUse, iFppp)
GO(xmlValidateOneAttribute, iFppppp)
GO(xmlValidateOneElement, iFppp)
GO(xmlValidateOneNamespace, iFpppppp)
GO(xmlValidatePopElement, iFpppp)
GO(xmlValidatePushCData, iFppi)
GO(xmlValidatePushElement, iFpppp)
GO(xmlValidateQName, iFpi)
GO(xmlValidateRoot, iFpp)
GO(xmlValidBuildContentModel, iFpp)
GO(xmlValidCtxtNormalizeAttributeValue, pFppppp)
GO(xmlValidGetPotentialChildren, iFpppi)
GO(xmlValidGetValidElements, iFpppi)
GO(xmlValidNormalizeAttributeValue, pFpppp)
//GO(xmlXIncludeFreeContext, 
//GO(xmlXIncludeNewContext, 
//GO(xmlXIncludeProcess, 
//GO(xmlXIncludeProcessFlags, 
//GO(xmlXIncludeProcessFlagsData, 
//GO(xmlXIncludeProcessNode, 
//GO(xmlXIncludeProcessTree, 
//GO(xmlXIncludeProcessTreeFlags, 
//GO(xmlXIncludeProcessTreeFlagsData, 
//GO(xmlXIncludeSetFlags, 
GO(xmlXPathAddValues, vFp)
GO(xmlXPathBooleanFunction, vFpi)
GO(xmlXPathCastBooleanToNumber, dFi)
GO(xmlXPathCastBooleanToString, pFi)
GO(xmlXPathCastNodeSetToBoolean, iFp)
GO(xmlXPathCastNodeSetToNumber, dFp)
GO(xmlXPathCastNodeSetToString, pFp)
GO(xmlXPathCastNodeToNumber, dFp)
GO(xmlXPathCastNodeToString, pFp)
GO(xmlXPathCastNumberToBoolean, iFd)
GO(xmlXPathCastNumberToString, pFd)
GO(xmlXPathCastStringToBoolean, iFp)
GO(xmlXPathCastStringToNumber, dFp)
GO(xmlXPathCastToBoolean, iFp)
GO(xmlXPathCastToNumber, dFp)
GO(xmlXPathCastToString, pFp)
GO(xmlXPathCeilingFunction, vFpi)
GO(xmlXPathCmpNodes, iFpp)
GO(xmlXPathCompareValues, iFpii)
GO(xmlXPathCompile, pFp)
GO(xmlXPathCompiledEval, pFpp)
GO(xmlXPathCompiledEvalToBoolean, iFpp)
GO(xmlXPathConcatFunction, vFpi)
GO(xmlXPathContainsFunction, vFpi)
GO(xmlXPathContextSetCache, iFpiii)
GO(xmlXPathConvertBoolean, pFp)
GO(xmlXPathConvertNumber, pFp)
GO(xmlXPathConvertString, pFp)
GO(xmlXPathCountFunction, vFpi)
GO(xmlXPathCtxtCompile, pFpp)
GO(xmlXPathDebugDumpCompExpr, vFppi)
GO(xmlXPathDebugDumpObject, vFppi)
GO(xmlXPathDifference, pFpp)
GO(xmlXPathDistinct, pFp)
GO(xmlXPathDistinctSorted, pFp)
GO(xmlXPathDivValues, vFp)
GO(xmlXPathEqualValues, iFp)
GO(xmlXPathErr, vFpi)
GO(xmlXPatherror, vFppii)
GO(xmlXPathEval, pFpp)
GO(xmlXPathEvalExpr, vFp)
GO(xmlXPathEvalExpression, pFpp)
GO(xmlXPathEvalPredicate, pFpp)
GO(xmlXPathEvaluatePredicateResult, iFpp)
GO(xmlXPathFalseFunction, vFpi)
GO(xmlXPathFloorFunction, vFpi)
GO(xmlXPathFreeCompExpr, vFp)
GO(xmlXPathFreeContext, vFp)
GO(xmlXPathFreeNodeSet, vFp)
GO(xmlXPathFreeNodeSetList, vFp)
GO(xmlXPathFreeObject, vFp)
GO(xmlXPathFreeParserContext, vFp)
//GOM(xmlXPathFunctionLookup, BFEpp)
//GOM(xmlXPathFunctionLookupNS, BFEppp)
//GO(xmlXPathHasSameNodes, 
//GO(xmlXPathIdFunction, 
GO(xmlXPathInit, vFv)
GO(xmlXPathIntersection, pFpp)
GO(xmlXPathIsInf, iFd)
GO(xmlXPathIsNaN, iFd)
GO(xmlXPathIsNodeType, iFp)
GO(xmlXPathLangFunction, vFpi)
GO(xmlXPathLastFunction, vFpi)
GO(xmlXPathLeading, pFpp)
GO(xmlXPathLeadingSorted, pFpp)
GO(xmlXPathLocalNameFunction, vFpi)
GO(xmlXPathModValues, vFp)
GO(xmlXPathMultValues, vFp)
GO(xmlXPathNamespaceURIFunction, vFpi)
GO(xmlXPathNewBoolean, pFi)
GO(xmlXPathNewContext, pFp)
GO(xmlXPathNewCString, pFp)
GO(xmlXPathNewFloat, pFd)
GO(xmlXPathNewNodeSet, pFp)
GO(xmlXPathNewNodeSetList, pFp)
GO(xmlXPathNewParserContext, pFpp)
GO(xmlXPathNewString, pFp)
GO(xmlXPathNewValueTree, pFp)
GO(xmlXPathNextAncestor, pFpp)
GO(xmlXPathNextAncestorOrSelf, pFpp)
GO(xmlXPathNextAttribute, pFpp)
GO(xmlXPathNextChild, pFpp)
GO(xmlXPathNextDescendant, pFpp)
GO(xmlXPathNextDescendantOrSelf, pFpp)
GO(xmlXPathNextFollowing, pFpp)
GO(xmlXPathNextFollowingSibling, pFpp)
GO(xmlXPathNextNamespace, pFpp)
GO(xmlXPathNextParent, pFpp)
GO(xmlXPathNextPreceding, pFpp)
GO(xmlXPathNextPrecedingSibling, pFpp)
GO(xmlXPathNextSelf, pFpp)
GO(xmlXPathNodeEval, pFppp)
GO(xmlXPathNodeLeading, pFpp)
GO(xmlXPathNodeLeadingSorted, pFpp)
GO(xmlXPathNodeSetAdd, iFpp)
GO(xmlXPathNodeSetAddNs, iFppp)
GO(xmlXPathNodeSetAddUnique, iFpp)
GO(xmlXPathNodeSetContains, iFpp)
GO(xmlXPathNodeSetCreate, pFp)
GO(xmlXPathNodeSetDel, vFpp)
GO(xmlXPathNodeSetFreeNs, vFp)
GO(xmlXPathNodeSetMerge, pFpp)
GO(xmlXPathNodeSetRemove, vFpi)
GO(xmlXPathNodeSetSort, vFp)
GO(xmlXPathNodeTrailing, pFpp)
GO(xmlXPathNodeTrailingSorted, pFpp)
GO(xmlXPathNormalizeFunction, vFpi)
GO(xmlXPathNotEqualValues, iFp)
GO(xmlXPathNotFunction, vFpi)
GO(xmlXPathNsLookup, pFpp)
GO(xmlXPathNumberFunction, vFpi)
GO(xmlXPathObjectCopy, pFp)
GO(xmlXPathOrderDocElems, lFp)
GO(xmlXPathParseName, pFp)
GO(xmlXPathParseNCName, pFp)
GO(xmlXPathPopBoolean, iFp)
GO(xmlXPathPopExternal, pFp)
GO(xmlXPathPopNodeSet, pFp)
GO(xmlXPathPopNumber, dFp)
GO(xmlXPathPopString, pFp)
GO(xmlXPathPositionFunction, vFpi)
GO(xmlXPathRegisterAllFunctions, vFp)
GO(xmlXPathRegisteredFuncsCleanup, vFp)
GO(xmlXPathRegisteredNsCleanup,vFp)
GO(xmlXPathRegisteredVariablesCleanup, vFp)
GOM(xmlXPathRegisterFunc, iFEppp)
//GOM(xmlXPathRegisterFuncLookup, vFEpBp)
//GOM(xmlXPathRegisterFuncNS, iFEpppB)
GO(xmlXPathRegisterNs, iFppp)
GO(xmlXPathRegisterVariable, iFppp)
//GOM(xmlXPathRegisterVariableLookup, vFEpBp)
GO(xmlXPathRegisterVariableNS, iFpppp)
GO(xmlXPathRoot, vFp)
GO(xmlXPathRoundFunction, vFpi)
GO(xmlXPathSetContextNode, iFpp)
GO(xmlXPathStartsWithFunction, vFpi)
GO(xmlXPathStringEvalNumber, dFp)
GO(xmlXPathStringFunction, vFpi)
GO(xmlXPathStringLengthFunction, vFpi)
GO(xmlXPathSubstringAfterFunction, vFpi)
GO(xmlXPathSubstringBeforeFunction, vFpi)
GO(xmlXPathSubstringFunction, vFpi)
//GO(xmlXPathSubValues, 
GO(xmlXPathSumFunction, vFpi)
GO(xmlXPathTrailing, pFpp)
GO(xmlXPathTrailingSorted, pFpp)
GO(xmlXPathTranslateFunction, vFpi)
GO(xmlXPathTrueFunction, vFpi)
GO(xmlXPathValueFlipSign, vFp)
GO(xmlXPathVariableLookup, pFpp)
GO(xmlXPathVariableLookupNS, pFppp)
GO(xmlXPathWrapCString, pFp)
GO(xmlXPathWrapExternal, pFp)
GO(xmlXPathWrapNodeSet, pFp)
GO(xmlXPathWrapString, pFp)
//GO(xmlXPtrAdvanceNode, 
//GO(xmlXPtrBuildNodeList, 
//GO(xmlXPtrEval, 
//GO(xmlXPtrEvalRangePredicate, 
//GO(xmlXPtrFreeLocationSet, 
//GO(xmlXPtrLocationSetAdd, 
//GO(xmlXPtrLocationSetCreate, 
//GO(xmlXPtrLocationSetDel, 
//GO(xmlXPtrLocationSetMerge, 
//GO(xmlXPtrLocationSetRemove, 
//GO(xmlXPtrNewCollapsedRange, 
//GO(xmlXPtrNewContext, 
//GO(xmlXPtrNewLocationSetNodes, 
//GO(xmlXPtrNewLocationSetNodeSet, 
//GO(xmlXPtrNewRange, 
//GO(xmlXPtrNewRangeNodeObject, 
//GO(xmlXPtrNewRangeNodePoint, 
//GO(xmlXPtrNewRangeNodes, 
//GO(xmlXPtrNewRangePointNode, 
//GO(xmlXPtrNewRangePoints, 
//GO(xmlXPtrRangeToFunction, 
//GO(xmlXPtrWrapLocationSet, 

DATAM(xmlFree, sizeof(void*))
DATAM(xmlMalloc, sizeof(void*))
DATAM(xmlRealloc, sizeof(void*))
DATAM(xmlMemStrdup, sizeof(void*))

DATA(xmlIsDigitGroup, 2*sizeof(void*)+2*sizeof(int))
DATA(xmlStringText, sizeof(void*))
DATA(xmlStringTextNoenc, sizeof(void*))
DATA(xmlIsCombiningGroup, 2*sizeof(void*)+2*sizeof(int))
DATA(xmlIsExtenderGroup, 2*sizeof(void*)+2*sizeof(int))
DATA(xmlIsBaseCharGroup, 2*sizeof(void*)+2*sizeof(int))
